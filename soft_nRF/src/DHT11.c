#include "DHT11.h"

LOG_MODULE_REGISTER(dht11, LOG_LEVEL_INF);

static const struct device *dht_gpio = NULL;
int DHT11_corrected_temperature = 20;

int dht11_init(void){
    dht_gpio = DEVICE_DT_GET(DHT_PORT);
    if (!device_is_ready(dht_gpio)) {
        LOG_ERR("DHT11 GPIO not ready!");
        return -1;
    }
    LOG_INF("DHT11 initialized on pin %d", DHT_PIN);
    return 0;
}

int dht11_read(uint8_t data[5]){
    int i, j;
    uint32_t start;
    for (i = 0; i < 5; i++)
        data[i] = 0;
    gpio_pin_configure(dht_gpio, DHT_PIN, GPIO_OUTPUT);
    gpio_pin_set(dht_gpio, DHT_PIN, 0);
    k_msleep(20);
    gpio_pin_set(dht_gpio, DHT_PIN, 1);
    k_busy_wait(30);
    gpio_pin_configure(dht_gpio, DHT_PIN, GPIO_INPUT | GPIO_PULL_UP);

    start = k_cycle_get_32();
    while (gpio_pin_get(dht_gpio, DHT_PIN)){
        if (k_cyc_to_us_floor32(k_cycle_get_32() - start) > 100)
            return -1;
    }
    while (!gpio_pin_get(dht_gpio, DHT_PIN));
    while (gpio_pin_get(dht_gpio, DHT_PIN));

    for (i = 0; i < 40; i++){
        while (!gpio_pin_get(dht_gpio, DHT_PIN));
        start = k_cycle_get_32();
        while (gpio_pin_get(dht_gpio, DHT_PIN));
        uint32_t pulse = k_cyc_to_us_floor32(k_cycle_get_32() - start);
        j = i / 8;
        data[j] <<= 1;
        if (pulse > 50)
            data[j] |= 1;
    }
    if (((data[0] + data[1] + data[2] + data[3]) & 0xFF) != data[4])
        return -2;
    return 0;
}

// DHT11 work handler
void dht11_work_handler(struct k_work *work){
    ARG_UNUSED(work);
    uint8_t buf[5];
    int ret = dht11_read(buf);

    extern struct k_work_q sensor_wq;
    extern struct k_work_delayable dht11_periodic_work;

    if (ret == 0) {
        int humidity = buf[0];
        int temperature = buf[2];
        DHT11_corrected_temperature = temperature - 3;
        LOG_INF("Humidity: %d %% | Temp: %d °C", humidity, temperature);
    } else {
        LOG_WRN("Failed to read DHT11 (err %d)", ret);
    }
    k_work_schedule_for_queue(&sensor_wq, &dht11_periodic_work, K_MSEC(MEASURE_DHT11_PERIOD_MS));
}
