#ifndef __UART_H__
#define __UART_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"

void init_uart(void);
int send_data(const char *log_name, const char *data, size_t len);
int recv_data(const char *log_name, uint8_t *buf, size_t buf_len);

#endif // __UART_H__
