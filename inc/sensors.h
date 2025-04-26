#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include <zephyr/sys/__assert.h>

#define USER_MAX_SENSORS 3

struct __attribute__((packed)) sensor_data {
    uint16_t userId;
    uint32_t readingId;
    uint16_t temp[USER_MAX_SENSORS];
    uint16_t humidity[USER_MAX_SENSORS];
    uint32_t pressure[USER_MAX_SENSORS];
    uint32_t light[USER_MAX_SENSORS];
};

BUILD_ASSERT(sizeof(struct sensor_data) == 42, "Struct size mismatch");

void sensors_thread(void);

#endif /* SENSORS_H */