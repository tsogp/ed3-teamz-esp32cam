#ifndef __CAMERA_HANDLERS_H__
#define __CAMERA_HANDLERS_H__

#include <esp_err.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_http_server.h>
#include <esp_camera.h>
#include <cJSON.h>

#include "camera.h"
#include "server/ws_handlers.h"

esp_err_t stream_jpg(httpd_req_t *req);
esp_err_t reload_camera_resolution(httpd_req_t *req);
esp_err_t reload_camera_quality(httpd_req_t *req);
esp_err_t toggle_flash_handler(httpd_req_t *req);

#endif // __CAMERA_HANDLERS_H__
