#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/net/buf.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/drivers/gpio.h>

int scanner_init(void);

#endif
