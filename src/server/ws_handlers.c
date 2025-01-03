#include "server/ws_handlers.h"

#define TAG "WS_HANDLERS"
#define WS_BUFFER_SIZE 64

struct async_resp_arg {
	httpd_handle_t hd;
	int fd;
};

static struct async_resp_arg ws_cfg = { .hd = NULL, .fd = -1 };

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

	httpd_ws_send_frame_async(hd, fd, &ws_pkt);
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

	esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,
			 "httpd_ws_recv_frame failed to get frame len with %d",
			 ret);
		return ret;
	} else if (ws_pkt.len > WS_BUFFER_SIZE + 1) {
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

		send_data("UART_CONTROLS", buffer, strlen(buffer));
	}

	return ESP_OK;
}