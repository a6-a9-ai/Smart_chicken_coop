#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "bluetooth.h"

int main(void){
    printk("Starting Scanner Application\n");
    if (scanner_init() < 0){
        printk("Failed to initialize scanner\n");
        return 0;
    }
    while (1){
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
