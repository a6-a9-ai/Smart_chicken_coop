#include "pir.h"

LOG_MODULE_REGISTER(pir, LOG_LEVEL_INF);

static const struct device *pir_gpio = NULL;
extern struct k_work_q sensor_wq;
void pir_work_handler(struct k_work *work);

K_WORK_DELAYABLE_DEFINE(pir_work, pir_work_handler);

int pir_init(void){
    int ret;
    pir_gpio = DEVICE_DT_GET(PIR_PORT);
    if (!device_is_ready(pir_gpio)){
        LOG_ERR("Error: PIR Sensor GPIO device is not ready");
        return -ENODEV;
    }
    ret = gpio_pin_configure(pir_gpio, PIR_PIN, GPIO_INPUT | GPIO_PULL_DOWN);
    if (ret != 0){
        LOG_ERR("Error %d: failed to configure PIR pin %d", ret, PIR_PIN);
        return ret;
    }

    LOG_INF("PIR sensor initialized on pin %d (Polling Mode)", PIR_PIN);
    return 0;
}

int pir_read(void){
    int val = gpio_pin_get(pir_gpio, PIR_PIN);
    if (val < 0){
        LOG_ERR("Error reading PIR sensor: %d", val);
    }
    return val;
}

void pir_work_handler(struct k_work *work){
    int motion = pir_read();
    if (motion > 0){
        LOG_INF("mouvement detecte : %d", motion);
    } else if (motion == 0){
        LOG_INF("pas de mouvement : %d", motion);
    }
    k_work_schedule_for_queue(&sensor_wq, &pir_work, K_MSEC(MEASURE_PIR_PERIOD_MS));
}
