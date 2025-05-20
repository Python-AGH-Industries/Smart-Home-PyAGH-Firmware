#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include <zephyr/sys/__assert.h>

#define USER_MAX_SENSORS 3

/* LPS25HB I2C sensor */
#define I2C_NODE DT_NODELABEL(lps25hb)

#define LPS25HB_ADDR 0xBA
#define LPS25HB_WHO_AM_I 0x0F
#define LPS25HB_CTRL_REG1 0x20
#define LPS25HB_CTRL_REG2 0x21
#define LPS25HB_CTRL_REG3 0x22
#define LPS25HB_CTRL_REG4 0x23
#define LPS25HB_PRESS_OUT_XL 0x28
#define LPS25HB_PRESS_OUT_L 0x29
#define LPS25HB_PRESS_OUT_H 0x2A
#define LPS25HB_TEMP_OUT_L 0x2B
#define LPS25HB_TEMP_OUT_H 0x2C

#define LPS25HB_WHO_AM_I_EXPECTED 0xBD
#define LPS25HB_WAKE_UP_25HZ 0xC0
#define LPS25HB_READ_NEXT 0x80

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