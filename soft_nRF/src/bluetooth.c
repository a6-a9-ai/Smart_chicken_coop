#include "bluetooth.h"

LOG_MODULE_REGISTER(bluetooth, LOG_LEVEL_INF);

extern struct k_work_q critical_wq;
static bool ble_adv_running = false;

static const struct bt_data ble_alerte_data[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, strlen(CONFIG_BT_DEVICE_NAME)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, 0, sizeof(int)),
};

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

static void ble_launch_alerte_handler(struct k_work *work){
    ble_send_alert(1, 0, 1);
}
//  BLE works definitions
K_WORK_DEFINE(ble_launch_alerte_work, ble_launch_alerte_handler);
K_WORK_DELAYABLE_DEFINE(ble_stop_alerte_work, ble_stop_alerte_handler);


void ble_stop_alerte_handler(struct k_work *work){
    bt_le_adv_stop();
    ble_adv_running = false;
    LOG_INF("Advertising stopped.");

}

struct ble_mfg_data_t {
    uint16_t company_id;
    uint8_t payload;
};

static struct ble_mfg_data_t mfg_data = {
    .company_id = 0xFFFF,
    .payload = 0x00,
};

static struct bt_data adv_data[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE,
            CONFIG_BT_DEVICE_NAME,
            sizeof(CONFIG_BT_DEVICE_NAME) - 1),
    BT_DATA(BT_DATA_MANUFACTURER_DATA,
            &mfg_data,
            sizeof(mfg_data)),
};

static uint8_t current_temp = 0;
static uint8_t current_gaz = 0;
static uint8_t current_attack = 0;

void ble_send_alert(uint8_t temp, uint8_t gaz, uint8_t attack){
    int err;

    // Update static state
    current_temp = temp;
    current_gaz = gaz;
    current_attack = attack;

    mfg_data.payload = 0;
    mfg_data.payload |= (current_temp   & 0x01) << 0;
    mfg_data.payload |= (current_gaz    & 0x01) << 1;
    mfg_data.payload |= (current_attack & 0x01) << 2;

    if (ble_adv_running) {
        bt_le_adv_stop();
    }
    err = bt_le_adv_start(&adv_param, adv_data, ARRAY_SIZE(adv_data), NULL, 0);
    if (err) {
        LOG_ERR("BLE adv start failed (%d)", err);
        return;
    }

    ble_adv_running = true;
    LOG_INF("BLE alert sent: temp=%d gaz=%d attack=%d (0x%02X)", current_temp, current_gaz, current_attack, mfg_data.payload);
}

void ble_update_bit_trigger(uint8_t bit, uint8_t value) {
    switch (bit) {
        case 0: // BLE_PAQUET_TEMP_BIT
            current_temp = value;
            break;
        case 1: // BLE_PAQUET_GAS_BIT
            current_gaz = value;
            break;
        case 2: // BLE_PAQUET_ATTACK_BIT
            current_attack = value;
            break;
        default:
            LOG_WRN("Unknown BLE bit: %d", bit);
            return;
    }
    ble_send_alert(current_temp, current_gaz, current_attack);
}


