#include "../inc/wifi.h"
#include <zephyr/kernel.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/wifi_mgmt.h>
#include <net/wifi_credentials.h>
#include <zephyr/logging/log.h>

K_THREAD_DEFINE(wifi, 2048, wifi_thread, NULL, NULL, NULL, 1, 0, 0);
LOG_MODULE_REGISTER(wifi_logger);

#define EVENT_MASK (NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED)

static struct net_mgmt_event_callback mgmt_cb;

static bool connected;
static K_SEM_DEFINE(run_app, 0, 1);

static void net_mgmt_event_handler(struct net_mgmt_event_callback *cb, 
                                uint32_t mgmt_event, struct net_if *iface)
{
    if ((mgmt_event & EVENT_MASK) != mgmt_event) {
        return;
    }

    if (mgmt_event == NET_EVENT_L4_CONNECTED) {
        LOG_INF("Network connected");
        connected = true;
        k_sem_give(&run_app);
        return;
    }

    if (mgmt_event == NET_EVENT_L4_DISCONNECTED) {
        if (connected == false) {
            LOG_INF("Waiting for wifi to be connected");
        } else {
            LOG_INF("Network disconnected");
            connected = false;
        }
        k_sem_reset(&run_app);
        return;
    }
}

void wifi_thread(void)
{
    LOG_INF("Initializing Wi-Fi driver");
    k_sleep(K_SECONDS(1));

    net_mgmt_init_event_callback(&mgmt_cb, net_mgmt_event_handler, EVENT_MASK);
    net_mgmt_add_event_callback(&mgmt_cb);

    #ifdef CONFIG_WIFI_CREDENTIALS_STATIC
    struct net_if *iface = net_if_get_first_wifi();
    if (iface == NULL) {
        LOG_ERR("Returned network interface is NULL");
        return;
    }
    #endif /* CONFIG_WIFI_CREDENTIALS_STATIC */

    while (true) {
        k_sem_take(&run_app, K_FOREVER);
        while (k_sem_count_get(&run_app) == 0) {
            LOG_INF("Wi-Fi is ready");
            k_sleep(K_SECONDS(1));
        }
        LOG_INF("App stoped");
    }
}
