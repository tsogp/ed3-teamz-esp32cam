#ifndef __SERVER_H__
#define __SERVER_H__

#include <esp_http_server.h>
#include <esp_camera.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "camera.h"

#include "server/camera_handlers.h"
#include "server/ws_handlers.h"

void start_webserver(void);
void stop_webserver(void);
void start_workers(void);

#endif // __SERVER_H__
