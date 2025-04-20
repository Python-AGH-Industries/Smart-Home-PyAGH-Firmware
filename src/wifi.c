#include "../inc/wifi.h"
#include <zephyr/kernel.h>

K_THREAD_DEFINE(wifi, 2048, wifi_thread, NULL, NULL, NULL, 1, 0, 0);

void wifi_thread(void)
{
    while (true) {
        printk("Hello from wifi thread\n");
        k_msleep(1000);
    }
}