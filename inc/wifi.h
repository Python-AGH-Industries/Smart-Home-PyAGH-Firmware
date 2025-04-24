#ifndef WIFI_H
#define WIFI_H

#define CONFIG_MQTT_BROKER_HOSTNAME "mqtt.hivemq.com"
#define CONFIG_MQTT_PUB_TOPIC "pyagh/smarthome/firmware/sensors"

void wifi_thread(void);

#endif /* WIFI_H */