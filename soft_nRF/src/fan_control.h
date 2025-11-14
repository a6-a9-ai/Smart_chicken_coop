#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include <stdint.h>

int fan_control_init(void);

#endif