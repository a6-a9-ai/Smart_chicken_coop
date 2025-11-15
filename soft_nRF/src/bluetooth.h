#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <zephyr/kernel.h>

void ble_alerte_temp_trigger(void);
static void ble_alerte_temp_handler(struct k_work *work);


#endif