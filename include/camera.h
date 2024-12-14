#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "esp_camera.h"
#include "esp_log.h"

esp_err_t init_camera();
int change_camera_resolution(framesize_t frame_size);
int change_camera_jpeg_quality(int quality);

#endif // __CAMERA_H__
