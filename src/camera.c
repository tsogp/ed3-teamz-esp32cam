#include "camera.h"

#define TAG "CAMERA"

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

#define FLASH_PIN 4

// TODO: Load default values from NVS
static framesize_t camera_resolution = FRAMESIZE_VGA;
static int jpeg_quality = 30;

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

					 .fb_count = 2,
					 .fb_location = CAMERA_FB_IN_DRAM,
					 .grab_mode = CAMERA_GRAB_WHEN_EMPTY };

esp_err_t init_camera() {
	gpio_config_t io_conf = { .pin_bit_mask = (1ULL << FLASH_PIN),
				  .mode = GPIO_MODE_OUTPUT,
				  .pull_up_en = GPIO_PULLUP_DISABLE,
				  .pull_down_en = GPIO_PULLDOWN_DISABLE,
				  .intr_type = GPIO_INTR_DISABLE };
	esp_err_t err = gpio_config(&io_conf);
	if (err != ESP_OK) {
		return err;
	}

	camera_config.frame_size = camera_resolution;
	camera_config.jpeg_quality = jpeg_quality;

	err = esp_camera_init(&camera_config);
	if (err != ESP_OK) {
		return err;
	}

	return ESP_OK;
}

// TODO: fix into cleaner implementation
int change_camera_resolution(framesize_t frame_size) {
	// frame_size is bigger then FRAMESIZE_UXGA,
	// which is the maximum quality supported for OX2640
	if (frame_size > 15) {
		return 1;
	} else if (frame_size == camera_resolution) {
		return 2;
	}

	sensor_t *sensor = esp_camera_sensor_get();
	int ret = sensor->set_framesize(sensor, frame_size);
	vTaskDelay(pdMS_TO_TICKS(100));
	camera_resolution = frame_size;
	return ret;
}

int change_camera_jpeg_quality(int quality) {
	// Only qualities between 10 and 63 are
	// supported in OX2640
	if (quality < 10 || quality > 63) {
		return 1;
	} else if (quality == jpeg_quality) {
		return 2;
	}

	sensor_t *sensor = esp_camera_sensor_get();
	int ret = sensor->set_quality(sensor, quality);
	vTaskDelay(pdMS_TO_TICKS(100));
	jpeg_quality = quality;
	return ret;
}

esp_err_t toggle_camera_flash() {
	int cr = gpio_get_level(FLASH_PIN);
	ESP_LOGI(TAG, "%d", (cr + 1) % 2);
	esp_err_t ret = gpio_set_level(FLASH_PIN, (cr + 1) % 2);
	vTaskDelay(pdMS_TO_TICKS(100));
	return ret;
}