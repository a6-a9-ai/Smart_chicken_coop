#include "bluetooth.h"
#define COMPANY_ID_TEST 0xFFFF

struct ble_mfg_data_t{
    uint16_t company_id;
    uint8_t payload;
} __packed;

static void device_found(const bt_addr_le_t *addr,
                        int8_t rssi,
                        uint8_t type,
                        struct net_buf_simple *ad){

    char addr_str[BT_ADDR_LE_STR_LEN];
    struct ble_mfg_data_t *mfg_data;
    char dev_name[33];

    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    
    memset(dev_name, 0, sizeof(dev_name));
    struct net_buf_simple_state name_state;
    net_buf_simple_save(ad, &name_state);
    
    while (ad->len > 1){
        uint8_t len = net_buf_simple_pull_u8(ad);
        if (len == 0) break;
        if (len > ad->len) break;
        uint8_t type = net_buf_simple_pull_u8(ad);
        if (type == BT_DATA_NAME_COMPLETE || type == BT_DATA_NAME_SHORTENED){
            uint8_t name_len = len - 1;
            if (name_len > 32) name_len = 32;
            memcpy(dev_name, ad->data, name_len);
            dev_name[name_len] = '\0';
            break; 
        }
        net_buf_simple_pull(ad, len - 1);
    }
    net_buf_simple_restore(ad, &name_state);
    if (dev_name[0] == 0){
        strcpy(dev_name, "(Unknown)");
    }

    printk("[DEVICE] %s (RSSI %d) Name: %s\n", addr_str, rssi, dev_name);

    struct net_buf_simple_state state;
    net_buf_simple_save(ad, &state);

    while (ad->len > 1){
        uint8_t len = net_buf_simple_pull_u8(ad);
        uint8_t ad_type;

        if (len == 0){
            break;
        }
        if (len > ad->len){
            break;
        }

        ad_type = net_buf_simple_pull_u8(ad);
        
        if (strncmp(dev_name, "smartChicken", 12) == 0){
            if ((len +1) == sizeof(struct ble_mfg_data_t)){
                mfg_data = (struct ble_mfg_data_t *)ad->data;
                {
                     uint16_t found_company_id = sys_le16_to_cpu(mfg_data->company_id);
                     uint8_t payload = mfg_data->payload;
                     uint8_t temp = (payload >> 0) & 0x01;
                     uint8_t gas  = (payload >> 1) & 0x01;
                     uint8_t attack = (payload >> 2) & 0x01;

                     printk("   -> [TARGET MATCH BY SIZE] %s (RSSI %d) Name: %s\n", addr_str, rssi, dev_name);
                     printk("   -> Company: 0x%04X (Expected: 0x%04X)\n", found_company_id, COMPANY_ID_TEST);
                     printk("   -> Status: Temp=%d, Gas=%d, Attack=%d\n", temp, gas, attack);
                }
            }
        }
        net_buf_simple_pull(ad, len - 1);
    }
    net_buf_simple_restore(ad, &state);
}

int scanner_init(void)
{
    struct bt_le_scan_param scan_param = {
        .type       = BT_LE_SCAN_TYPE_PASSIVE,
        .options    = BT_LE_SCAN_OPT_NONE,
        .interval   = 0x0150,
        .window     = 0x0030,
    };
    int err;

    err = bt_enable(NULL);
    if (err){
        printk("Bluetooth init failed (err %d)\n", err);
        return err;
    }

    printk("Bluetooth initialized\n");

    err = bt_le_scan_start(&scan_param, device_found);
    if (err){
        printk("Starting scanning failed (err %d)\n", err);
        return err;
    }

    printk("Scanning successfully started\n");
    return 0;
}
