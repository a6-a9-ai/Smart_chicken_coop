#ifndef PIR_H
#define PIR_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define PIR_PORT                        DT_NODELABEL(gpio1)
#define PIR_PIN                         8
#define MEASURE_PIR_PERIOD_MS           2500

int pir_init(void);
int pir_read(void);

extern struct k_work_delayable pir_work;

#endif // PIR_H
