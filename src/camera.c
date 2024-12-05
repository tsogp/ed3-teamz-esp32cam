#include "camera.h"

esp_err_t init_camera(pixformat_t pixel_format, framesize_t frame_size,
		      int jpeg_quality, size_t fb_count) {
	camera_config_t camera_config = { 
                        .pin_pwdn = CAM_PIN_PWDN,
					  .pin_reset = CAM_PIN_RESET,
					  .pin_xclk = CAM_PIN_XCLK,
					  .pin_sccb_sda = CAM_PIN_SIOD,
					  .pin_sccb_scl = CAM_PIN_SIOC,

					  .pin_d7 = CAM_PIN_D7,
					  .pin_d6 = CAM_PIN_D6,
					  .pin_d5 = CAM_PIN_D5,
					  .pin_d4 = CAM_PIN_D4,
					  .pin_d3 = CAM_PIN_D3,
					  .pin_d2 = CAM_PIN_D2,
					  .pin_d1 = CAM_PIN_D1,
					  .pin_d0 = CAM_PIN_D0,
					  .pin_vsync = CAM_PIN_VSYNC,
					  .pin_href = CAM_PIN_HREF,
					  .pin_pclk = CAM_PIN_PCLK,

					  .xclk_freq_hz = CONFIG_XCLK_FREQ,
					  .ledc_timer = LEDC_TIMER_0,
					  .ledc_channel = LEDC_CHANNEL_0,

					  .pixel_format = pixel_format,
					  .frame_size = frame_size,

					  .jpeg_quality = jpeg_quality,
					  .fb_count = fb_count,
                      .fb_location = CAMERA_FB_IN_DRAM,
					  .grab_mode = CAMERA_GRAB_WHEN_EMPTY };

	esp_err_t err = esp_camera_init(&camera_config);
	if (err != ESP_OK) {
		return err;
	}

	return ESP_OK;
}