#include "server/camera_handlers.h"

#define TAG "CAMERA_HANDLERS"
#define PART_BOUNDARY "123456789000000000000987654321"

static const char *_STREAM_CONTENT_TYPE =
	"multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART =
	"Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

esp_err_t stream_jpg(httpd_req_t *req) {
	camera_fb_t *fb = NULL;
	esp_err_t res = ESP_OK;
	size_t _jpg_buf_len;
	uint8_t *_jpg_buf;
	char *part_buf[64];
	static int64_t last_frame = 0;

	if (!last_frame) {
		last_frame = esp_timer_get_time();
	}

	res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
	if (res != ESP_OK) {
		return res;
	}

	while (true) {
		fb = esp_camera_fb_get();
		if (!fb) {
			ESP_LOGE(TAG, "Camera capture failed");
			res = ESP_FAIL;
			break;
		}

		_jpg_buf_len = fb->len;
		_jpg_buf = fb->buf;

		if (res == ESP_OK) {
			res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY,
						    strlen(_STREAM_BOUNDARY));
		}

		if (res == ESP_OK) {
			size_t hlen = snprintf((char *)part_buf, 64,
					       _STREAM_PART, _jpg_buf_len);
			res = httpd_resp_send_chunk(req, (const char *)part_buf,
						    hlen);
		}

		if (res == ESP_OK) {
			res = httpd_resp_send_chunk(req, (const char *)_jpg_buf,
						    _jpg_buf_len);
		}

		esp_camera_fb_return(fb);

		if (res != ESP_OK) {
			break;
		}

		int64_t fr_end = esp_timer_get_time();
		int64_t frame_time = fr_end - last_frame;
		last_frame = fr_end;
		frame_time /= 1000;
		trigger_async_send(1000.0 / (uint32_t)frame_time);
		ESP_LOGI(TAG, "MJPG: %luKB %lums (%.1ffps)",
			 (uint32_t)(_jpg_buf_len / 1024), (uint32_t)frame_time,
			 1000.0 / (uint32_t)frame_time);
	}

	last_frame = 0;
	return res;
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

esp_err_t reload_camera_resolution(httpd_req_t *req) {
	char buffer[64];
	size_t total_len = req->content_len;
	int received = 0;

	if (total_len > sizeof(buffer)) {
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
				    "Content length too large");
		return ESP_FAIL;
	}

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

esp_err_t reload_camera_quality(httpd_req_t *req) {
	char buffer[64];
	size_t total_len = req->content_len;
	int received = 0;

	if (total_len > sizeof(buffer)) {
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
				    "Content length too large");
		return ESP_FAIL;
	}

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
		httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
		return ESP_FAIL;
	}

	cJSON *q_field = cJSON_GetObjectItem(json, "q");
	if (cJSON_IsNumber(q_field)) {
		int q = q_field->valueint;

		if (q < 11 || q > 63) {
			cJSON_Delete(json);
			httpd_resp_send_err(
				req, HTTPD_400_BAD_REQUEST,
				"Field is not a valid resolution type");
			return ESP_FAIL;
		}

		// TODO: map to actual resolution in the log statement
		int reload_result = change_camera_jpeg_quality(q);
		if (reload_result != 0) {
			ESP_LOGE(TAG, "Could not reload camera config");
			cJSON_Delete(json);
			httpd_resp_send_err(req,
					    HTTPD_500_INTERNAL_SERVER_ERROR,
					    "Could not reload camera config");
			return ESP_FAIL;
		}
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

esp_err_t toggle_flash_handler(httpd_req_t *req) {
	esp_err_t res = toggle_camera_flash();
	if (res != ESP_OK) {
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
				    "Toggle flash ERROR");
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "Toggle flash OK");
	httpd_resp_sendstr(req, "Toggle flash OK");
	return ESP_OK;
}