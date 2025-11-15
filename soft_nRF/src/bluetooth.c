#include "bluetooth.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(bluetooth, LOG_LEVEL_INF);

extern struct k_work_q critical_workqueue;

//  ble_alerte_temp definition
K_WORK_DEFINE(BLE_alerte_temp, ble_alerte_temp_handler);

static void ble_alerte_temp_handler(struct k_work *work){
    LOG_INF("alerte lancee");
}

void ble_alerte_temp_trigger(void){
    k_work_submit_to_queue(&critical_workqueue, &BLE_alerte_temp);
}
