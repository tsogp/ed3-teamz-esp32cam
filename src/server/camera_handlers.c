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

		if (req == NULL) {
			ESP_LOGE(TAG, "break");
			continue;
		}

		if (res == ESP_OK) {
			res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY,
						    strlen(_STREAM_BOUNDARY));
		} else {
			ESP_LOGE(TAG, "1chunk error %s", esp_err_to_name(res));
		}

		if (res == ESP_OK) {
			size_t hlen = snprintf((char *)part_buf, 64,
					       _STREAM_PART, _jpg_buf_len);
			res = httpd_resp_send_chunk(req, (const char *)part_buf,
						    hlen);
		} else {
			ESP_LOGE(TAG, "2chunk error %s", esp_err_to_name(res));
		}

		if (res == ESP_OK) {
			res = httpd_resp_send_chunk(req, (const char *)_jpg_buf,
						    _jpg_buf_len);
		} else {
			ESP_LOGE(TAG, "3chunk error %s", esp_err_to_name(res));
		}

		esp_camera_fb_return(fb);

		if (res != ESP_OK) {
            ESP_LOGE(TAG, "Error while streaming, closing connection.");
            // Close the current connection
            int sockfd = httpd_req_to_sockfd(req);
            if (sockfd >= 0) {
                ESP_LOGI(TAG, "Closing socket: %d", sockfd);
                httpd_sess_trigger_close(req->handle, sockfd);
            }

            // Optionally, handle a reconnection or wait for the next client request
            ESP_LOGI(TAG, "Waiting for a new client request...");
            break;  // Break the loop after closing the connection
        }

		int64_t fr_end = esp_timer_get_time();
		int64_t frame_time = fr_end - last_frame;
		last_frame = fr_end;
		frame_time /= 1000;
		trigger_async_send(1000.0 / (uint32_t)frame_time);
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

esp_err_t reload_camera_config(httpd_req_t *req) {
	char buffer[32];
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

	bool resolution_updated = false;
	bool quality_updated = false;

	// Handle resolution field
	cJSON *rs_field = cJSON_GetObjectItem(json, "rs");
	if (rs_field) {
		if (cJSON_IsNumber(rs_field)) {
			int rs = rs_field->valueint;

			if (rs < 0 || rs > 15) {
				ESP_LOGE(TAG, "Invalid resolution value");
				cJSON_Delete(json);
				httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
						    "Invalid resolution value");
				return ESP_FAIL;
			}

			if (change_camera_resolution((framesize_t)rs) != 0) {
				ESP_LOGE(TAG, "Failed to update resolution");
				cJSON_Delete(json);
				httpd_resp_send_err(
					req, HTTPD_500_INTERNAL_SERVER_ERROR,
					"Failed to update resolution");
				return ESP_FAIL;
			}
			resolution_updated = true;
		} else {
			ESP_LOGE(TAG, "Resolution field is not a valid number");
			cJSON_Delete(json);
			httpd_resp_send_err(
				req, HTTPD_400_BAD_REQUEST,
				"Resolution field is not a valid number");
			return ESP_FAIL;
		}
	}

	// Handle quality field
	cJSON *q_field = cJSON_GetObjectItem(json, "q");
	if (q_field) {
		if (cJSON_IsNumber(q_field)) {
			int q = q_field->valueint;

			if (q < 11 || q > 63) {
				ESP_LOGE(TAG, "Invalid quality value");
				cJSON_Delete(json);
				httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
						    "Invalid quality value");
				return ESP_FAIL;
			}

			if (change_camera_jpeg_quality(q) != 0) {
				ESP_LOGE(TAG, "Failed to update quality");
				cJSON_Delete(json);
				httpd_resp_send_err(
					req, HTTPD_500_INTERNAL_SERVER_ERROR,
					"Failed to update quality");
				return ESP_FAIL;
			}
			quality_updated = true;
		} else {
			ESP_LOGE(TAG, "Quality field is not a valid number");
			cJSON_Delete(json);
			httpd_resp_send_err(
				req, HTTPD_400_BAD_REQUEST,
				"Quality field is not a valid number");
			return ESP_FAIL;
		}
	}

	cJSON_Delete(json);

	if (!resolution_updated && !quality_updated) {
		httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
				    "Provide 'q' or 'rs to update.");
		return ESP_FAIL;
	}

	httpd_resp_sendstr(req, "Success");
	return ESP_OK;
}