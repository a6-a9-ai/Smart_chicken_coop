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
void ble_send_alert(uint8_t temp, uint8_t gaz, uint8_t attack);

// Custom function to update specific alert bits
void ble_update_bit_trigger(uint8_t bit, uint8_t value);

void ble_stop_alerte_handler(struct k_work *work);

#endif
