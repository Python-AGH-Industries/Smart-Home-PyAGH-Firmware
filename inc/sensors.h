#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

#define USER_MAX_SENSORS 3

struct sensor_data {
    uint16_t temp[USER_MAX_SENSORS];
    uint16_t humidity[USER_MAX_SENSORS];
    uint16_t pressure[USER_MAX_SENSORS];
    uint16_t light[USER_MAX_SENSORS];
};

void sensors_thread(void);

#endif /* SENSORS_H */