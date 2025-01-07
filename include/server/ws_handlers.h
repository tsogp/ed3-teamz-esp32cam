#ifndef __WS_HANDLERS_H__
#define __WS_HANDLERS_H__

#include <esp_err.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <cJSON.h>
#include "uart.h"

esp_err_t controls_data_handler(httpd_req_t *req);
esp_err_t trigger_async_send(double framerate);
esp_err_t ws_disconnection_handler(httpd_handle_t hd, httpd_err_code_t err);

#endif // __WS_HANDLERS_H__
