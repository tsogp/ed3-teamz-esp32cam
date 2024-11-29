#include "server.h"

static httpd_handle_t server = NULL;

esp_err_t get_handler(httpd_req_t *req) {
	const char resp[] = "lol";
	httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

httpd_uri_t default_get = { .uri = "/",
			    .method = HTTP_GET,
			    .handler = get_handler,
			    .user_ctx = NULL };

void start_webserver(void) {
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	if (httpd_start(&server, &config) == ESP_OK) {
		httpd_register_uri_handler(server, &default_get);
	}
}

void stop_webserver(void) {
	if (server) {
		httpd_stop(server);
	}
}