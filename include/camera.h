#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "esp_camera.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>

esp_err_t init_camera();
int change_camera_resolution(framesize_t frame_size);
int change_camera_jpeg_quality(int quality);
esp_err_t toggle_camera_flash();

#endif // __CAMERA_H__
