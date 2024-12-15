#ifndef __WS_HANDLERS_H__
#define __WS_HANDLERS_H__

#include <esp_err.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <cJSON.h>

esp_err_t controls_data_handler(httpd_req_t *req);
esp_err_t trigger_async_send(double framerate);

#endif // __WS_HANDLERS_H__
