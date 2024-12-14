#include "server.h"

#define TAG "SERVER"

static httpd_handle_t server = NULL;

#define ASYNC_WORKER_TASK_PRIORITY 5
#define CONFIG_EXAMPLE_MAX_ASYNC_REQUESTS 4
#define ASYNC_WORKER_TASK_STACK_SIZE 2048

static QueueHandle_t request_queue;

// Track the number of free workers at any given time
static SemaphoreHandle_t worker_ready_count;

// Each worker has its own thread
static TaskHandle_t worker_handles[CONFIG_EXAMPLE_MAX_ASYNC_REQUESTS];

typedef esp_err_t (*httpd_req_handler_t)(httpd_req_t *req);

typedef struct {
	httpd_req_t *req;
	httpd_req_handler_t handler;
} httpd_async_req_t;

// queue an HTTP req to the worker queue
static esp_err_t queue_request(httpd_req_t *req, httpd_req_handler_t handler) {
	ESP_LOGI(TAG, "got a queue request");
	// must create a copy of the request that we own
	httpd_req_t *copy = NULL;
	esp_err_t err = httpd_req_async_handler_begin(req, &copy);
	if (err != ESP_OK) {
		return err;
	}

	httpd_async_req_t async_req = {
		.req = copy,
		.handler = handler,
	};

	// How should we handle resource exhaustion?
	// In this example, we immediately respond with an
	// http error if no workers are available.
	int ticks = 0;

	// counting semaphore: if success, we know 1 or
	// more asyncReqTaskWorkers are available.
	if (xSemaphoreTake(worker_ready_count, ticks) == false) {
		ESP_LOGE(TAG, "No workers are available");
		httpd_req_async_handler_complete(copy); // cleanup
		return ESP_FAIL;
	}

	// Since worker_ready_count > 0 the queue should already have space.
	// But lets wait up to 100ms just to be safe.
	if (xQueueSend(request_queue, &async_req, pdMS_TO_TICKS(100)) ==
	    false) {
		ESP_LOGE(TAG, "worker queue is full");
		httpd_req_async_handler_complete(copy); // cleanup
		return ESP_FAIL;
	}

	return ESP_OK;
}

// each worker thread loops forever, processing requests
static void worker_task(void *p) {
	ESP_LOGI(TAG, "starting async req task worker");

	while (true) {
		// counting semaphore - this signals that a worker
		// is ready to accept work
		xSemaphoreGive(worker_ready_count);

		// wait for a request
		httpd_async_req_t async_req;
		if (xQueueReceive(request_queue, &async_req, portMAX_DELAY)) {
			ESP_LOGI(TAG, "invoking %s", async_req.req->uri);

			// call the handler
			async_req.handler(async_req.req);

			// Inform the server that it can purge the socket used for
			// this request, if needed.
			if (httpd_req_async_handler_complete(async_req.req) !=
			    ESP_OK) {
				ESP_LOGE(TAG, "failed to complete async req");
			}
		}
	}

	ESP_LOGW(TAG, "worker stopped");
	vTaskDelete(NULL);
}

// start worker threads
void start_workers(void) {
	// counting semaphore keeps track of available workers
	worker_ready_count = xSemaphoreCreateCounting(
		CONFIG_EXAMPLE_MAX_ASYNC_REQUESTS, // Max Count
		0); // Initial Count
	if (worker_ready_count == NULL) {
		ESP_LOGE(TAG, "Failed to create workers counting Semaphore");
		return;
	}

	request_queue = xQueueCreate(CONFIG_EXAMPLE_MAX_ASYNC_REQUESTS,
				     sizeof(httpd_async_req_t));
	if (request_queue == NULL) {
		ESP_LOGE(TAG, "Failed to create request queue");
		vSemaphoreDelete(worker_ready_count);
		return;
	}

	// start worker tasks
	for (int i = 0; i < CONFIG_EXAMPLE_MAX_ASYNC_REQUESTS; i++) {
		bool success =
			xTaskCreate(worker_task, "async_req_worker",
				    8192, // stack size
				    (void *)0, // argument
				    ASYNC_WORKER_TASK_PRIORITY, // priority
				    &worker_handles[i]);

		if (!success) {
			ESP_LOGE(TAG, "Failed to start asyncReqWorker");
			continue;
		}
	}
}

static esp_err_t post_body_recv_error_check(httpd_req_t *req, int ret) {
	if (ret <= 0) {
		if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
			httpd_resp_send_408(req);
		}

		return ESP_FAIL;
	}

	return ESP_OK;
}

static esp_err_t stream_jpg_httpd_handler(httpd_req_t *req) {
	if (queue_request(req, stream_jpg) == ESP_OK) {
		return ESP_OK;
	} else {
		httpd_resp_set_status(req, "503 Busy");
		httpd_resp_sendstr(
			req, "<div> no workers available. server busy.</div>");
		return ESP_OK;
	}
}

static esp_err_t reload_camera_config_httpd_handler(httpd_req_t *req) {
	char buffer[256];
	size_t total_len = req->content_len;
	int received = 0;

	if (total_len > sizeof(buffer)) {
		ESP_LOGE(TAG, "Content length too large");
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
				    "Content length too large");
		return ESP_FAIL;
	}

	ESP_LOGE(TAG, "Starting receiving new camera config");

	while (received < total_len) {
		int ret = httpd_req_recv(req, buffer + received,
					 total_len - received);
		esp_err_t post_recv_ok = post_body_recv_error_check(req, ret);
		if (post_recv_ok != ESP_OK) {
			return post_recv_ok;
		}
		received += ret;
	}

	// Ensure the buffer is null-terminated for safe JSON parsing
	buffer[total_len] = '\0';

	cJSON *json = cJSON_Parse(buffer);
	if (!json) {
		ESP_LOGE(TAG, "Invalid JSON");
		httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
		return ESP_FAIL;
	}

	cJSON *rs_field = cJSON_GetObjectItem(json, "rs");
	if (cJSON_IsNumber(rs_field)) {
		int rs = rs_field->valueint;

		if (rs < 0 || rs > 15) {
			ESP_LOGE(TAG, "Not a valid resolution");
			cJSON_Delete(json);
			httpd_resp_send_err(
				req, HTTPD_400_BAD_REQUEST,
				"Field is not a valid resolution type");
			return ESP_FAIL;
		}

		// TODO: map to actual resolution in the log statement
		int reload_result = change_camera_resolution((framesize_t)rs);
		if (reload_result != 0) {
			ESP_LOGE(TAG, "Could not reload camera config");
			cJSON_Delete(json);
			httpd_resp_send_err(req,
					    HTTPD_500_INTERNAL_SERVER_ERROR,
					    "Could not reload camera config");
			return ESP_FAIL;
		}
		ESP_LOGI(TAG, "Changed camera resolution to %d", rs);
	} else {
		ESP_LOGE(TAG, "Not a valid number");
		cJSON_Delete(json);
		httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
				    "Field is not a valid number");
		return ESP_FAIL;
	}

	cJSON_Delete(json);
	httpd_resp_sendstr(req, "Resolution changed successfully");
	return ESP_OK;
}

// static esp_err_t reload_camera_config_httpd_handler(httpd_req_t *req) {
// 	if (queue_request(req, reload_camera_config) == ESP_OK) {
// 		return ESP_OK;
// 	} else {
// 		httpd_resp_set_status(req, "503 Busy");
// 		httpd_resp_sendstr(
// 			req, "<div> no workers available. server busy.</div>");
// 		return ESP_OK;
// 	}
// }

esp_err_t options_handler(httpd_req_t *req) {
	httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Methods",
			   "GET, POST, OPTIONS");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "*");
	httpd_resp_set_hdr(req, "Access-Control-Max-Age", "86400");
	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}

void start_webserver(void) {
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	config.max_open_sockets = CONFIG_EXAMPLE_MAX_ASYNC_REQUESTS + 1;

	static httpd_uri_t video_streamer = { .uri = "/",
					      .method = HTTP_GET,
					      .handler =
						      stream_jpg_httpd_handler,
					      .user_ctx = NULL };

	static httpd_uri_t camera_config_reloader = {
		.uri = "/reload-camera-config",
		.method = HTTP_POST,
		.handler = reload_camera_config_httpd_handler,
		.user_ctx = NULL
	};

	static httpd_uri_t options_uri = { .uri = "/",
					   .method = HTTP_OPTIONS,
					   .handler = options_handler,
					   .user_ctx = NULL };

	if (httpd_start(&server, &config) == ESP_OK) {
		httpd_register_uri_handler(server, &options_uri);
		httpd_register_uri_handler(server, &video_streamer);
		httpd_register_uri_handler(server, &camera_config_reloader);
	}
}

void stop_webserver(void) {
	if (server) {
		httpd_stop(server);
	}
}