#ifndef UART_H
#define UART_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#define RECEIVE_BUFF_SIZE 4
#define RECEIVE_TIMEOUT 1000

int uart_init(void);

#endif
