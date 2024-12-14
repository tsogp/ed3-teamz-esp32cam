#include "camera.h"

// TODO: investigate jpeg_quality, fb_count, grab_mode
static camera_config_t camera_config = { .pin_pwdn = CAM_PIN_PWDN,
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

					 .pixel_format = PIXFORMAT_JPEG,

					 .jpeg_quality = 60,
					 .fb_count = 1,
					 .fb_location = CAMERA_FB_IN_DRAM,
					 .grab_mode = CAMERA_GRAB_WHEN_EMPTY };

// TODO: Load default values from NVS
static framesize_t camera_resolution = FRAMESIZE_VGA;

static bool inject_new_camera_resolution(const framesize_t fsz) {
	bool res_changed = false;
	if (camera_resolution != fsz) {
		camera_resolution = fsz;
		res_changed = false;
	}

	return res_changed;
}

esp_err_t init_camera() {
	camera_config.frame_size = camera_resolution;

	esp_err_t err = esp_camera_init(&camera_config);
	if (err != ESP_OK) {
		return err;
	}

	return ESP_OK;
}

// TODO: fix into cleaner implementation
esp_err_t reinit_camera(framesize_t frame_size) {
	sensor_t *sensor = esp_camera_sensor_get();

	ESP_LOGW("CAMERA", "got framesize %d", frame_size);

	sensor->set_framesize(sensor, frame_size);
	// vTaskDelay(pdMS_TO_TICKS(100));
	// sensor->set_aec2(sensor, false);
	// sensor->set_exposure_ctrl(sensor, false);
	// vTaskDelay(pdMS_TO_TICKS(200));

	// flush
	// for (int i = 0; i < camera_config.fb_count; i++) {
	// 	int retries = 3;
	// 	while (--retries) {
	// 		camera_fb_t *fb = esp_camera_fb_get();
	// 		if (fb) {
	// 			esp_camera_fb_return(fb);
	// 			retries = 0;
	// 		} else {
	// 			ESP_LOGW(
	// 				"CAMERA",
	// 				"camera returned null fb while flushing %d",
	// 				i);
	// 			vTaskDelay(pdMS_TO_TICKS(500));
	// 		}
	// 	}
	// }

	return ESP_OK;
}