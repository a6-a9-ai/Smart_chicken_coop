#include "fan_control.h"
#include "DHT11.h"

LOG_MODULE_REGISTER(fan_control, LOG_LEVEL_INF);

#define FAN_PWM_NODE DT_ALIAS(pwm_led0)

static const struct pwm_dt_spec fan_pwm = PWM_DT_SPEC_GET(FAN_PWM_NODE);
#define PWM_PERIOD_USEC 40    // Frequency of PWM = 25 kHz
#define FAN_THRESHOLD_TEMP 23
#define FAN_UPDATE_INTERVAL_MS 10000

extern struct k_work_q sensor_wq;

// Fan_control work handler
static void fan_control_work_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(fan_control_work, fan_control_work_handler);

static void fan_control_work_handler(struct k_work *work){
    uint32_t duty_cycle = 0;
    if (DHT11_corrected_temperature > FAN_THRESHOLD_TEMP) {
        duty_cycle = PWM_PERIOD_USEC;
        LOG_INF("Temp = %d°C → Fan ON (100%% duty)", DHT11_corrected_temperature);
    } else {
        duty_cycle = 0;
        LOG_INF("Temp = %d°C → Fan OFF", DHT11_corrected_temperature);
    }

    int ret = pwm_set_dt(&fan_pwm, PWM_USEC(PWM_PERIOD_USEC), PWM_USEC(duty_cycle));
    if (ret < 0) {
        LOG_ERR("pwm_set_dt() failed: %d", ret);
    }

    // Fan_control scheduling in workqueues
    k_work_schedule_for_queue(&sensor_wq, &fan_control_work, K_MSEC(FAN_UPDATE_INTERVAL_MS));
}
// Definition of fan_control initialisation
int fan_control_init(void){
    if (!device_is_ready(fan_pwm.dev)){
        LOG_ERR("PWM device not ready");
        return -1;
    }
    LOG_INF("Fan PWM initialized (threshold = %d°C)", FAN_THRESHOLD_TEMP);

    // Fist scheduling of dht11_periodic_work
    k_work_schedule_for_queue(&sensor_wq, &fan_control_work, K_NO_WAIT);
    return 0;
}
