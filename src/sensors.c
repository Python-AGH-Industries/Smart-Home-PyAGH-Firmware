#include "../inc/sensors.h"
#include <zephyr/kernel.h>

K_THREAD_DEFINE(sensors, 2048, sensors_thread, NULL, NULL, NULL, 1, 0, 0);

void sensors_thread(void)
{
    while (true) {
        printk("Hello from sensor thread\n");
        k_msleep(1000);
    }
}

