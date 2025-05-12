#include "../inc/wifi.h"
#include "../inc/sensors.h"

#include <zephyr/kernel.h>
#include <zephyr/types.h>
#include <zephyr/random/random.h>

#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/socket.h>

#include <net/wifi_credentials.h>
#include <net/mqtt_helper.h>

#include <zephyr/logging/log.h>

K_THREAD_DEFINE(wifi, WIFI_STACK_SIZE, wifi_thread, NULL, NULL, NULL, 1, 0, 0);
LOG_MODULE_REGISTER(wifi_logger);

static struct net_mgmt_event_callback mgmt_cb;
static uint8_t client_id[sizeof(CONFIG_BOARD) + 11];

static bool connected;
static K_SEM_DEFINE(run_app, 0, 1);
static K_SEM_DEFINE(mqtt_connected, 0, 1);

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
            mqtt_helper_disconnect();
            connected = false;
            k_sem_reset(&run_app);
        }
        return;
    }
}

static int publish(uint8_t* data, size_t len)
{
    int err;

    struct mqtt_publish_param mqtt_param;

    mqtt_param.message.payload.data = data;
    mqtt_param.message.payload.len = len;
    mqtt_param.message.topic.qos = MQTT_QOS_0_AT_MOST_ONCE;
    mqtt_param.message_id = mqtt_helper_msg_id_get();
    mqtt_param.message.topic.topic.utf8 = SENSOR_PUBLISH_TOPIC;
    mqtt_param.message.topic.topic.size = sizeof(SENSOR_PUBLISH_TOPIC) - 1,
    mqtt_param.dup_flag = 0;
    mqtt_param.retain_flag = 0;

    err = mqtt_helper_publish(&mqtt_param);
    if (err) {
        LOG_WRN("Failed to send payload, err: %d", err);
        return err;
    }

    LOG_INF("Published %d bytes on topic: \"%.*s\"", 
        len,
        mqtt_param.message.topic.topic.size,
        mqtt_param.message.topic.topic.utf8
    );

    return 0;
}

static void on_mqtt_connack(enum mqtt_conn_return_code return_code, bool session_present)
{
    if (return_code == MQTT_CONNECTION_ACCEPTED) {
        LOG_INF("Connected to MQTT broker");
        LOG_INF("Hostname: %s", BROKER_HOSTNAME);
		LOG_INF("Client ID: %s", (char *)client_id);
		LOG_INF("Port: %d", CONFIG_MQTT_HELPER_PORT);
		LOG_INF("TLS: %s", IS_ENABLED(CONFIG_MQTT_LIB_TLS) ? "Yes" : "No");
        k_sem_give(&mqtt_connected);
    } else {
        LOG_WRN("Connection to broker not established, err = %d", return_code);
    }
}

static void on_mqtt_disconnect(int result)
{
    LOG_INF("Disconnect from the broker, result = %d", result);
    k_sem_reset(&mqtt_connected);
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
        k_sem_give(&run_app);

        while (k_sem_count_get(&run_app)) {
            struct mqtt_helper_cfg config = {
                .cb = {
                    .on_connack = on_mqtt_connack,
                    .on_disconnect = on_mqtt_disconnect
                }
            };

            int err = mqtt_helper_init(&config);
            if (err) {
                LOG_ERR("Failed to initialize MQTT helper, error: %d", err);
                k_sleep(K_SECONDS(1));
                continue;
            }

            uint32_t id = sys_rand32_get();
            snprintf(client_id, sizeof(client_id), "%s-%010u", CONFIG_BOARD, id);

            struct mqtt_helper_conn_params conn_params = {
                .hostname.ptr = BROKER_HOSTNAME,
                .hostname.size = sizeof(BROKER_HOSTNAME) - 1,
                .device_id.ptr = (char *)client_id,
                .device_id.size = sizeof(client_id) - 1
            };

            err = mqtt_helper_connect(&conn_params);
            if (err) {
                LOG_ERR("Failed to connect to MQTT, error code: %d", err);
                k_sleep(K_SECONDS(1));
                continue;
            }

            k_sem_take(&mqtt_connected, K_FOREVER);
            k_sem_give(&mqtt_connected);

            while (k_sem_count_get(&mqtt_connected)) {
                struct sensor_data data = {
                    .userId = 1,
                    .readingId = sys_rand32_get(),
                    .temp = {
                        100 + sys_rand16_get() % 210,
                        200 + sys_rand16_get() % 160,
                        300 + sys_rand16_get() % 110
                    },
                    .humidity = {
                        300 + sys_rand16_get() % 101,
                        320 + sys_rand16_get() % 100,
                        100 + sys_rand16_get() % 200
                    },
                    .pressure = {
                        900000 + sys_rand32_get() % 200000,
                        1000000 + sys_rand32_get() % 100000,
                        1000999 + sys_rand32_get() % 50000
                    },
                    .light = {
                        sys_rand32_get() % 50000,
                        70000 + sys_rand32_get() % 10000,
                        15000 + sys_rand32_get() % 40000
                    }
                };

                publish((uint8_t*)&data, sizeof(data));

                k_sleep(K_SECONDS(1));
            }
        }
        LOG_INF("App stoped");
    }
}
