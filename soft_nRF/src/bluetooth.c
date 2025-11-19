#include "bluetooth.h"

LOG_MODULE_REGISTER(bluetooth, LOG_LEVEL_INF);

extern struct k_work_q critical_wq;

// Advertising data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, 12),
};

// Advertising parameters
static struct bt_le_adv_param adv_param = {
    .id = BT_ID_DEFAULT,
    .sid = 0,
    .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
    .interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
    .options = BT_LE_ADV_OPT_CONNECTABLE,
};

int bluetooth_init(void){
    return bt_enable(NULL);
}

//  ble works definitions
K_WORK_DEFINE(BLE_alerte_temp_work, ble_alerte_temp_handler);
K_WORK_DELAYABLE_DEFINE(BLE_stop_alerte_temp_work, ble_stop_alerte_temp_handler);

void ble_alerte_temp_handler(struct k_work *work){
    int err;
    err = bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        LOG_ERR("Advertising start failed (err %d)", err);
    }
    LOG_INF("Advertising started for %d ms", ADV_DURATION_MS);
    k_work_schedule_for_queue(&critical_wq, &BLE_stop_alerte_temp_work, K_MSEC(ADV_DURATION_MS));
}

void ble_alerte_temp_trigger(void){
    k_work_submit_to_queue(&critical_wq, &BLE_alerte_temp_work);
}

void ble_stop_alerte_temp_handler(struct k_work *work){
    bt_le_adv_stop();
    LOG_INF("Advertising stopped.");
}




