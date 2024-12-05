#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "esp_camera.h"

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

#define CONFIG_XCLK_FREQ 20000000

esp_err_t init_camera(pixformat_t pixel_format, framesize_t frame_size,
		      int jpeg_quality, size_t fb_count);

#endif // __CAMERA_H__
