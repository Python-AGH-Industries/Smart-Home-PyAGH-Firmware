#ifndef WIFI_H
#define WIFI_H

#define EVENT_MASK (NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED)

#define SENSOR_PUBLISH_TOPIC "pyagh/smarthome/sensors/data"

#define BROKER_HOSTNAME "broker.hivemq.com"

#define WIFI_STACK_SIZE 2048

void wifi_thread(void);

#endif /* WIFI_H */