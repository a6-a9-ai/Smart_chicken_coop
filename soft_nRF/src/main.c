/*******************************************************************************/
/*::::'######::'##::::'##:'####::'######::'##:::'##:'########:'##::: ##::::::::*/
/*:::'##... ##: ##:::: ##:. ##::'##... ##: ##::'##:: ##.....:: ###:: ##::::::::*/
/*::: ##:::..:: ##:::: ##:: ##:: ##:::..:: ##:'##::: ##::::::: ####: ##::::::::*/
/*::: ##::::::: #########:: ##:: ##::::::: #####:::: ######::: ## ## ##::::::::*/
/*::: ##::::::: ##.... ##:: ##:: ##::::::: ##. ##::: ##...:::: ##. ####::::::::*/
/*::: ##::: ##: ##:::: ##:: ##:: ##::: ##: ##:. ##:: ##::::::: ##:. ###::::::::*/
/*:::. ######:: ##:::: ##:'####:. ######:: ##::. ##: ########: ##::. ##::::::::*/
/*::::......:::..:::::..::....:::......:::..::::..::........::..::::..:::::::::*/
/*::::'######:::'#######:::'#######::'########:::::main.c::::::::::::::::::::::*/
/*:::'##... ##:'##.... ##:'##.... ##: ##.... ##::::::::::::::::::::::::::::::::*/
/*::: ##:::..:: ##:::: ##: ##:::: ##: ##:::: ##::::Author: a6a9aia:::::::::::::*/
/*::: ##::::::: ##:::: ##: ##:::: ##: ########:::::<a5a8ahaiac@proton.me>::::::*/
/*::: ##::::::: ##:::: ##: ##:::: ##: ##.....::::::::::::::::::::::::::::::::::*/
/*::: ##::: ##: ##:::: ##: ##:::: ##: ##:::::::::::Created: 2025/09/01:::::::::*/
/*:::. ######::. #######::. #######:: ##:::::::::::Updated: 2026/01/10:::::::::*/
/*::::......::::.......::::.......:::..::::::::::::::::::::::::::::::::::::::::*/
/*******************************************************************************/

#include "DHT11.h"
#include "pir.h"
#include "s30.h"
//#include "SCD30.h"
#include "bluetooth.h"
#include "uart.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// Sensor workqueue (periodic)
#define SENSOR_WQ_STACK_SIZE                    1024
#define SENSOR_WQ_PRIORITY                      5
K_THREAD_STACK_DEFINE(sensor_stack_area, SENSOR_WQ_STACK_SIZE);
struct k_work_q sensor_wq;

// Critical workqueue
#define CRITICAL_WQ_STACK_SIZE                  1024
#define CRITICAL_WQ_PRIORITY                    -4
K_THREAD_STACK_DEFINE(critical_stack_area, CRITICAL_WQ_STACK_SIZE);
struct k_work_q critical_wq;

// Definition of dht11_periodic_work
K_WORK_DELAYABLE_DEFINE(dht11_periodic_work, dht11_periodic_work_handler);
K_WORK_DELAYABLE_DEFINE(s30_periodic_work, s30_periodic_work_handler);
//K_WORK_DELAYABLE_DEFINE(scd30_periodic_work, scd30_periodic_work_handler);

int main(void){
    if (bluetooth_init() < 0){
        LOG_ERR("Bluetooth init failed");
    }
    if (dht11_init() < 0){
        LOG_ERR("DHT11 init failed!");
    }
    if (pir_init() < 0){
        LOG_ERR("PIR init failed!");
    }
    if (s30_init() < 0){
        LOG_ERR("S30 init failed!");
    }
    //if (scd30_init() < 0){
    //    LOG_ERR("SCD30 init failed!");
    //} else {
    //    k_work_schedule_for_queue(&sensor_wq, &scd30_periodic_work, K_SECONDS(4));
    //}

    // Workqueues initialisation
    k_work_queue_start(&sensor_wq, sensor_stack_area,
                       K_THREAD_STACK_SIZEOF(sensor_stack_area),
                       SENSOR_WQ_PRIORITY, NULL);
    k_work_queue_start(&critical_wq, critical_stack_area,
                       K_THREAD_STACK_SIZEOF(critical_stack_area),
                       CRITICAL_WQ_PRIORITY, NULL);
    LOG_INF("Workqueues started (sensor=%d, critical=%d)", SENSOR_WQ_PRIORITY, CRITICAL_WQ_PRIORITY);

    if (uart_init() < 0){
        LOG_ERR("UART init failed!");
    } else {
        LOG_INF("UART initialized successfully");
    }

    if (fan_control_init() < 0){
        LOG_ERR("Fan control init failed!");
    }
    // Scheduling periodic_sensors_works
    k_work_schedule_for_queue(&sensor_wq, &dht11_periodic_work, K_SECONDS(3));
    k_work_schedule_for_queue(&sensor_wq, &s30_periodic_work, K_SECONDS(7));
    k_work_schedule_for_queue(&sensor_wq, &pir_work, K_MSEC(MEASURE_PIR_PERIOD_MS));

    while(1){
        k_sleep(K_SECONDS(10));
    }
}
