#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>

#define ADV_DURATION_MS 15000

int bluetooth_init(void);
void ble_alerte_temp_trigger(void);
void ble_alerte_temp_handler(struct k_work *work);
void ble_stop_alerte_temp_handler(struct k_work *work);

#endif