#ifndef DHT11_H
#define DHT11_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <stdint.h>

#define DHT_PORT                        DT_NODELABEL(gpio0)
#define DHT_PIN                         4
#define MEASURE_DHT11_PERIOD_MS         5000

#define FAN_THRESHOLD_TEMP_SUP          20
#define FAN_THRESHOLD_TEMP_INF          -5
#define PWM_PERIOD_USEC                 40

#define BLE_PAQUET_TEMP_BIT             0
#define BLE_PAQUET_GAS_BIT              1
#define BLE_PAQUET_ATTACK_BIT           2

//Functions
int dht11_init(void);
int fan_control_init(void);
int dht11_read(uint8_t data[5]);
void dht11_periodic_work_handler(struct k_work *work);
void fan_control_work_handler(struct k_work *work);

#endif
