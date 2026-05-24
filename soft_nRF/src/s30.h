#ifndef S30_H
#define S30_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <stdint.h>

#define MEASURE_S30_PERIOD_MS         5000

// SCD30 I2C commands
#define SCD30_CMD_TRIGGER_MEAS 0x0010
#define SCD30_CMD_DATA_READY   0x0202
#define SCD30_CMD_READ_MEAS    0x0300

// Functions
int s30_init(void);
void s30_periodic_work_handler(struct k_work *work);

// CRC-8 calculation helper
uint8_t scd30_calculate_crc(const uint8_t *data, size_t len);

#endif // S30_H
