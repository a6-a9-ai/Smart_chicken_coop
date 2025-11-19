#include "DHT11.h"
#include "bluetooth.h"

LOG_MODULE_REGISTER(dht11, LOG_LEVEL_INF);

#define FAN_PWM_NODE                DT_ALIAS(pwm_led0)
static const struct pwm_dt_spec fan_pwm = PWM_DT_SPEC_GET(FAN_PWM_NODE);

static const struct device *dht_gpio = NULL;
int DHT11_corrected_temperature = 20;
uint32_t duty_cycle;
bool previous_fan_pwm = true;

extern struct k_work_q sensor_wq;
extern struct k_work_q critical_wq;

K_WORK_DEFINE(fan_control_work, fan_control_work_handler);

void fan_control_work_handler(struct k_work *work){
    int ret = pwm_set_dt(&fan_pwm, PWM_USEC(PWM_PERIOD_USEC), PWM_USEC(duty_cycle));
    if (ret < 0) {
        LOG_ERR("pwm_set_dt() failed: %d", ret);
    } else {
        LOG_INF("Fan PWM updated: %s", duty_cycle ? "ON (100%)" : "OFF (0%)");
    }
}

int fan_control_init(void){
    if (!device_is_ready(fan_pwm.dev)) {
        LOG_ERR("PWM device not ready");
        return -1;
    }
    LOG_INF("Fan PWM initialized");
    return 0;
}

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
void dht11_periodic_work_handler(struct k_work *work){
    uint8_t buf[5];
    int ret = dht11_read(buf);
    extern struct k_work_delayable dht11_periodic_work;

    if (ret == 0) {
        int humidity = buf[0];
        int temperature = buf[2];
        DHT11_corrected_temperature = temperature - 3;
        LOG_INF("Humidity: %d %% | Temp: %d °C", humidity, DHT11_corrected_temperature);

        if (DHT11_corrected_temperature > FAN_THRESHOLD_TEMP && !previous_fan_pwm){
            ble_alerte_temp_trigger();                          // Lancer alerte BLE via critical_wq
            duty_cycle = PWM_PERIOD_USEC;
            k_work_submit_to_queue(&critical_wq, &fan_control_work);
            previous_fan_pwm = true;

        } else if (DHT11_corrected_temperature < FAN_THRESHOLD_TEMP && previous_fan_pwm) {
            duty_cycle = 0;
            k_work_submit_to_queue(&critical_wq, &fan_control_work);
            previous_fan_pwm = false;
        }
    } else {
        LOG_WRN("Failed to read DHT11 (err %d)", ret);
    }
    k_work_schedule_for_queue(&sensor_wq, &dht11_periodic_work, K_MSEC(MEASURE_DHT11_PERIOD_MS));
}
