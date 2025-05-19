#include "../inc/sensors.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

K_THREAD_DEFINE(sensors, 2048, sensors_thread, NULL, NULL, NULL, 1, 0, 0);

static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);

void sensors_thread(void)
{
    int ret;
    if (!device_is_ready(dev_i2c.bus)) {
        printk("I2C bus %s is not ready!\n", dev_i2c.bus->name);
        return;
    }

    uint8_t who_am_i_value = 0;
    uint8_t who_am_i_addr = LPS25HB_WHO_AM_I;

    ret = i2c_write_read_dt(
        &dev_i2c,
        &who_am_i_addr,
        sizeof(who_am_i_addr),
        &who_am_i_value,
        sizeof(who_am_i_value)
    );

    if (ret < 0) {
        printk("I2C read from WHO_AM_I register failed with error %d\n", ret);
        return;
    }

    if (who_am_i_value != LPS25HB_WHO_AM_I_EXPECTED) {
        printk("WHO_AM_I does not match, value read %d\n", who_am_i_value);
        return;
    }

    printk("I2C communication with LPS25HB established\n");

    while (true) {
        printk("Hello from sensor thread\n");
        k_msleep(1000);
    }
}

