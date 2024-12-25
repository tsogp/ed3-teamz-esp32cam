#include "uart.h"

#define BUF_SIZE 256

#define TXD_PIN (GPIO_NUM_1)
#define RXD_PIN (GPIO_NUM_3)

void init_uart(void) {
	const uart_config_t uart_config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_DEFAULT,
	};

	ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
	ESP_ERROR_CHECK(
		uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0));
	ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, TXD_PIN, RXD_PIN,
				     UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

int send_data(const char *log_name, const char *data, size_t len) {
	const int txBytes = uart_write_bytes(UART_NUM_0, data, len);
	return txBytes;
}

int recv_data(const char *log_name, uint8_t *buf, size_t buf_len) {
	ESP_ERROR_CHECK(
		uart_get_buffered_data_len(UART_NUM_0, (size_t *)&buf_len));
	int len = uart_read_bytes(UART_NUM_0, buf, buf_len, 100);
	if (len) {
		buf[len] = '\0';
	}
	return len;
}
