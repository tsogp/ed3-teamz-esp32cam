#include "server/ws_handlers.h"

#define TAG "WS_HANDLERS"
#define WS_BUFFER_SIZE 128

#define WS_KEEPALIVE_INTERVAL_MS 10000
#define WS_PONG_TIMEOUT_MS 5000

static TimerHandle_t ws_keepalive_timer;
static bool pong_received = true;
static const char* ping_msg = "ping";

struct async_resp_arg {
	httpd_handle_t hd;
	int fd;
};

static struct async_resp_arg ws_cfg = { .hd = NULL, .fd = -1 };

static esp_err_t send_ping() {
	httpd_ws_frame_t ws_pkt = {
		.type = HTTPD_WS_TYPE_TEXT,
		.payload = (uint8_t*)ping_msg,
		.len = 4,
	};

	esp_err_t ret = ESP_OK;
	if (ws_cfg.hd != NULL && ws_cfg.fd != -1) {
		ret = httpd_ws_send_frame_async(ws_cfg.hd, ws_cfg.fd, &ws_pkt);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG, "1Failed to send frame: %s; fd: %d", esp_err_to_name(ret), ws_cfg.fd);
		}
	}

	return ret;
}

static void stop_ws_keepalive() {
	if (ws_keepalive_timer != NULL) {
		xTimerStop(ws_keepalive_timer, 0);
		xTimerDelete(ws_keepalive_timer, 0);
		ws_keepalive_timer = NULL;
		ESP_LOGI(TAG, "Keep-alive timer stopped");
	}
}

static void ws_keepalive_task(struct tmrTimerControl *arg) {
	if (!ws_cfg.hd || ws_cfg.fd == -1) {
		return;
	}

	if (!pong_received) {
		ESP_LOGW(TAG, "Pong not received. Closing connection.");
		stop_ws_keepalive();
		httpd_sess_trigger_close(ws_cfg.hd, ws_cfg.fd);
		ws_cfg.hd = NULL;
		ws_cfg.fd = -1;
		return;
	}

	pong_received = false;
	send_ping();
}

static TimerCallbackFunction_t timer_cb = ws_keepalive_task;

static esp_err_t ws_pong_handler(httpd_req_t *req, httpd_ws_frame_t *frame) {
	pong_received = true;
	ESP_LOGI(TAG, "Pong received");
	return ESP_OK;
}

static void start_ws_keepalive() {
	ws_keepalive_timer = xTimerCreate(
		"ws_keepalive", pdMS_TO_TICKS(WS_KEEPALIVE_INTERVAL_MS), pdTRUE,
		NULL, timer_cb);

	if (ws_keepalive_timer != NULL) {
		send_ping();
		xTimerStart(ws_keepalive_timer, 0);
	}
}

esp_err_t ws_disconnection_handler(httpd_handle_t hd, httpd_err_code_t err) {
	ESP_LOGI(TAG, "WebSocket connection closed");
	ws_cfg.hd = NULL;
	ws_cfg.fd = -1;
	stop_ws_keepalive(); // Stop the keep-alive timer

	return ESP_OK;
}

static void ws_framerate_send(void *arg) {
	double *fps_arg = (double *)arg;
	httpd_handle_t hd = ws_cfg.hd;
	int fd = ws_cfg.fd;
	httpd_ws_frame_t ws_pkt = { 0 };

	char fps_buf[8];
	snprintf(fps_buf, 8, "%.1f", *fps_arg);

	ws_pkt.payload = (uint8_t *)fps_buf;
	ws_pkt.len = strlen(fps_buf);
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;

	if (hd != NULL && fd != -1) {
		esp_err_t ret = httpd_ws_send_frame_async(hd, fd, &ws_pkt);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG, "2Failed to send frame: %s, fd: %d", esp_err_to_name(ret), fd);
		}
	}

	free(fps_arg);
}

esp_err_t trigger_async_send(double framerate) {
	if (!ws_cfg.hd || ws_cfg.fd == -1) {
		return ESP_FAIL;
	}

	double *fps_args = malloc(sizeof(double));
	*fps_args = framerate;
	if (fps_args == NULL) {
		ESP_LOGE(TAG, "Memory allocation failed for fps_args");
		return ESP_ERR_NO_MEM;
	}

	esp_err_t ret =
		httpd_queue_work(ws_cfg.hd, ws_framerate_send, fps_args);
	if (ret != ESP_OK) {
		free(fps_args);
	}

	return ret;
}

esp_err_t controls_data_handler(httpd_req_t *req) {
	if (req->method == HTTP_GET || req->method == HTTP_OPTIONS) {
		httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
		httpd_resp_set_hdr(req, "Access-Control-Allow-Methods",
				   "GET, OPTIONS");

		ESP_LOGI(TAG, "Handshake done.");
		return ESP_OK;
	}

	httpd_ws_frame_t ws_pkt = { .type = HTTPD_WS_TYPE_TEXT };

	ws_cfg.hd = req->handle;
	ws_cfg.fd = httpd_req_to_sockfd(req);

	if (ws_keepalive_timer == NULL) {
		start_ws_keepalive();
	}

	esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,
			 "httpd_ws_recv_frame failed to get frame len with %d",
			 ret);
		return ret;
	}

	if (ws_pkt.len > WS_BUFFER_SIZE + 1) {
		ESP_LOGE(TAG, "Packet len %d tooo big", ret);
		return ESP_FAIL;
	}

	char buffer[WS_BUFFER_SIZE];
	if (ws_pkt.len) {
		ws_pkt.payload = (uint8_t *)buffer;
		ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d",
				 ret);
			return ret;
		}

		if (ws_pkt.len < WS_BUFFER_SIZE) {
			buffer[ws_pkt.len] = '\0';
		} else {
			ESP_LOGW(TAG, "Received data exceeds buffer size");
		}

		if (ws_pkt.len == 4 && !strcmp(buffer, "pong")) {
			return ws_pong_handler(req, &ws_pkt);
		}

		send_data("UART_CONTROLS", buffer, strlen(buffer));
	}

	return ESP_OK;
}