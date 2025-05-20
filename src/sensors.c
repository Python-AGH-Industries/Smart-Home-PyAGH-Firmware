#include "../inc/sensors.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>

K_THREAD_DEFINE(sensors, 2048, sensors_thread, NULL, NULL, NULL, 1, 0, 0);

static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);

static float lps25hb_read(struct sensor_data *data)
{
    int ret;
    uint8_t wake_up_data[] = {LPS25HB_CTRL_REG1, LPS25HB_WAKE_UP_25HZ};
    ret = i2c_write_dt(&dev_i2c, wake_up_data, sizeof(wake_up_data));
    
    if (ret < 0) {
        printk("Failed to activate sensor, error = %d\n", ret);
        return -1.0f;
    }

    uint8_t w_temp_data[] = {LPS25HB_TEMP_OUT_L | LPS25HB_READ_NEXT};
    uint8_t w_pres_data[] = {LPS25HB_PRESS_OUT_XL | LPS25HB_READ_NEXT};

    int16_t raw_temp = 0;
    int32_t raw_pres = 0;

    ret = i2c_write_read_dt(
        &dev_i2c,
        w_temp_data,
        sizeof(w_temp_data),
        (uint8_t*)&raw_temp,
        sizeof(raw_temp)
    );

    if (ret < 0) {
        printk("Temperature read failed, error = %d\n", ret);
        return -1.0f;
    }

    ret = i2c_write_read_dt(
        &dev_i2c,
        w_pres_data,
        sizeof(w_pres_data),
        (uint8_t*)&raw_pres,
        3
    );

    if (ret < 0) {
        printk("Pressure read failed, error = %d\n", ret);
        return -1.0f;
    }

    uint16_t temperature_scaled = 425 + raw_temp / 48.0f;
    uint32_t pressure_scaled = raw_pres * 1000.0f / 4096.0f;

    data->temp[0] = temperature_scaled;
    data->temp[1] = temperature_scaled + sys_rand16_get() % 20 + 10;
    data->temp[2] = temperature_scaled - sys_rand16_get() % 20 - 20;
    data->pressure[0] = pressure_scaled;
    data->pressure[1] = pressure_scaled + sys_rand32_get() % 2000 + 1000;
    data->pressure[2] = pressure_scaled + sys_rand32_get() % 2000 - 1000;
}

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
        struct sensor_data data;
        lps25hb_read(&data);
        printk(
            "T1 = %.1f C, T2 = %.1f C, T3 = %.1f C\nP1 = %.1f hPa, P2 = %.1f hPa, P3 = %.1f hPa\n\n",
            data.temp[0] / 10.0f,
            data.temp[1] / 10.0f,
            data.temp[2] / 10.0f,
            data.pressure[0] / 1000.0f,
            data.pressure[1] / 1000.0f,
            data.pressure[2] / 1000.0f
        );
        k_msleep(1000);
    }
}

