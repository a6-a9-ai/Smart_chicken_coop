#include "DHT11.h"
#include "fan_control.h"

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
struct k_work_q critical_workqueue;

// Definition of dht11_periodic_work
K_WORK_DELAYABLE_DEFINE(dht11_periodic_work, dht11_work_handler);

// Definition of critical_task (work)
void critical_task_handler(struct k_work *work){
    //LOG_INF("Critical_task executed");
}
K_WORK_DEFINE(critical_task, critical_task_handler);

int main(void){
    if (dht11_init() < 0){
        LOG_ERR("DHT11 init failed!");
        return -1;
    }
    // Workqueues initialisation
    k_work_queue_start(&sensor_wq, sensor_stack_area,
                       K_THREAD_STACK_SIZEOF(sensor_stack_area),
                       SENSOR_WQ_PRIORITY, NULL);
    k_work_queue_start(&critical_workqueue, critical_stack_area,
                       K_THREAD_STACK_SIZEOF(critical_stack_area),
                       CRITICAL_WQ_PRIORITY, NULL);
    LOG_INF("Workqueues started (sensor=%d, critical=%d)", SENSOR_WQ_PRIORITY, CRITICAL_WQ_PRIORITY);

    if (fan_control_init() < 0){
        LOG_ERR("Fan control init failed!");
    }
    // ! Exemples of scheduling and submition of tasks in critical_workqueue and sensor_wq
    k_work_schedule_for_queue(&sensor_wq, &dht11_periodic_work, K_SECONDS(3)); // Scheduling dht11_periodic_work in 3 seconds
    k_work_submit_to_queue(&critical_workqueue, &critical_task); //Execution of critical_task using critical workque (exemple)

    while(1){
        k_sleep(K_SECONDS(10));
    }
}
