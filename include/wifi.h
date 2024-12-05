#ifndef __WIFI_H_
#define __WIFI_H_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "local_mdns.h"
#include "server.h"
#include "camera.h"

void wifi_init_sta(void);

#endif // __WIFI_H_