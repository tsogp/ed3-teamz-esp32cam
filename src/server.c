#include "server.h"
#include <server/keep_alive.h>

#define TAG "SERVER"

static httpd_handle_t server = NULL;

#define ASYNC_WORKER_TASK_PRIORITY 15
#define CONFIG_EXAMPLE_MAX_ASYNC_REQUESTS 2
#define ASYNC_WORKER_TASK_STACK_SIZE 8192

static QueueHandle_t request_queue;
static SemaphoreHandle_t worker_ready_count;
static TaskHandle_t worker_handles[CONFIG_EXAMPLE_MAX_ASYNC_REQUESTS];
typedef esp_err_t (*httpd_req_handler_t)(httpd_req_t *req);

typedef struct {
	httpd_req_t *req;
	httpd_req_handler_t handler;
} httpd_async_req_t;

struct async_resp_arg {
	httpd_handle_t hd;
	int fd;
};

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

	int ticks = 0;

	if (xSemaphoreTake(worker_ready_count, ticks) == false) {
		ESP_LOGE(TAG, "No workers are available");
		httpd_req_async_handler_complete(copy); // cleanup
		return ESP_FAIL;
	}

	if (xQueueSend(request_queue, &async_req, pdMS_TO_TICKS(100)) ==
	    false) {
		ESP_LOGE(TAG, "worker queue is full");
		httpd_req_async_handler_complete(copy); // cleanup
		return ESP_FAIL;
	}

	return ESP_OK;
}

static void worker_task(void *p) {
	ESP_LOGI(TAG, "starting async req task worker");

	while (true) {
		xSemaphoreGive(worker_ready_count);

		httpd_async_req_t async_req;
		if (xQueueReceive(request_queue, &async_req, portMAX_DELAY)) {
			ESP_LOGI(TAG, "invoking %s", async_req.req->uri);

			async_req.handler(async_req.req);

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
				    ASYNC_WORKER_TASK_STACK_SIZE, // stack size
				    (void *)0, // argument
				    ASYNC_WORKER_TASK_PRIORITY, // priority
				    &worker_handles[i]);

		if (!success) {
			ESP_LOGE(TAG, "Failed to start asyncReqWorker");
			continue;
		}
	}
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

esp_err_t options_handler(httpd_req_t *req) {
	httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Methods",
			   "GET, POST, OPTIONS");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "*");
	httpd_resp_set_hdr(req, "Access-Control-Max-Age", "86400");
	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}

static void send_ping(void *arg) {
	struct async_resp_arg *resp_arg = arg;
	httpd_handle_t hd = resp_arg->hd;
	int fd = resp_arg->fd;
	httpd_ws_frame_t ws_pkt;
	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	ws_pkt.payload = NULL;
	ws_pkt.len = 0;
	ws_pkt.type = HTTPD_WS_TYPE_PING;

	httpd_ws_send_frame_async(hd, fd, &ws_pkt);
	free(resp_arg);
}

esp_err_t wss_open_fd(httpd_handle_t hd, int sockfd) {
	ESP_LOGI(TAG, "New client connected %d", sockfd);
	wss_keep_alive_t h = httpd_get_global_user_ctx(hd);
	return wss_keep_alive_add_client(h, sockfd);
}

void wss_close_fd(httpd_handle_t hd, int sockfd) {
	ESP_LOGI(TAG, "Client disconnected %d", sockfd);
	wss_keep_alive_t h = httpd_get_global_user_ctx(hd);
	wss_keep_alive_remove_client(h, sockfd);
	close(sockfd);
}

bool client_not_alive_cb(wss_keep_alive_t h, int fd) {
	ESP_LOGE(TAG, "Client not alive, closing fd %d", fd);
	httpd_sess_trigger_close(wss_keep_alive_get_user_ctx(h), fd);
	return true;
}

bool check_client_alive_cb(wss_keep_alive_t h, int fd) {
	ESP_LOGD(TAG, "Checking if client (fd=%d) is alive", fd);
	struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
	assert(resp_arg != NULL && "here failed");
	resp_arg->hd = wss_keep_alive_get_user_ctx(h);
	resp_arg->fd = fd;

	if (httpd_queue_work(resp_arg->hd, send_ping, resp_arg) == ESP_OK) {
		return true;
	}
	return false;
}

void start_webserver(void) {
	wss_keep_alive_config_t keep_alive_config = KEEP_ALIVE_CONFIG_DEFAULT();
	keep_alive_config.max_clients = 2;
	keep_alive_config.client_not_alive_cb = client_not_alive_cb;
	keep_alive_config.check_client_alive_cb = check_client_alive_cb;
	wss_keep_alive_t keep_alive = wss_keep_alive_start(&keep_alive_config);

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.max_open_sockets = CONFIG_EXAMPLE_MAX_ASYNC_REQUESTS + 1;
	config.global_user_ctx = keep_alive;
	config.open_fn = wss_open_fd;
	config.close_fn = wss_close_fd;

	static httpd_uri_t video_streamer = { .uri = "/",
					      .method = HTTP_GET,
					      .handler =
						      stream_jpg_httpd_handler,
					      .user_ctx = NULL };

	static httpd_uri_t camera_config_reloader = {
		.uri = "/reload-camera-config",
		.method = HTTP_POST,
		.handler = reload_camera_config,
		.user_ctx = NULL
	};

	static httpd_uri_t controls_handler = { .uri = "/controls",
						.method = HTTP_GET,
						.handler =
							controls_data_handler,
						.user_ctx = NULL,
						.is_websocket = true };

	static httpd_uri_t options_uri = { .uri = "/",
					   .method = HTTP_OPTIONS,
					   .handler = options_handler,
					   .user_ctx = NULL };

	if (httpd_start(&server, &config) == ESP_OK) {
		httpd_register_uri_handler(server, &options_uri);
		httpd_register_uri_handler(server, &video_streamer);
		httpd_register_uri_handler(server, &camera_config_reloader);
		httpd_register_uri_handler(server, &controls_handler);
	}
}

void stop_webserver(void) {
	if (server) {
		httpd_stop(server);
	}
}