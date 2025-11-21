#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>

#define ADV_DURATION_MS 60000

int bluetooth_init(void);
void ble_update_bit_trigger(int bit_number, int bit_value);
void ble_launch_alerte_handler(struct k_work *work);
void ble_stop_alerte_handler(struct k_work *work);

#endif