#ifndef __SERVER_H__
#define __SERVER_H__

#include "esp_http_server.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "camera.h"
#include "cJSON.h"

void start_webserver(void);
void stop_webserver(void);

#endif // __SERVER_H__
