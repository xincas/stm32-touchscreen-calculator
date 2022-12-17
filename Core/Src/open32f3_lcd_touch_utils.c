#include "open32f3_lcd_touch_utils.h"

#define XPT2046_CONV_VAL(byte_0, byte_1) (byte_0 << 5 | byte_1 >> 3)
#define XPT2046_CMD_DEFAULT_POWER_MODE 0x00

int lcd_xpt2046_init_clear(lcd_xpt2046_driver_t *driver) {
    driver->user_data = NULL;
    driver->communication_cb = NULL;
    driver->_is_initialized = 0;
    return 0;
}

int lcd_xpt2046_init(lcd_xpt2046_driver_t *driver) {
    int err = 0;

    if (driver->_is_initialized) {
        return -2;
    }

    if (driver->communication_cb == NULL) {
        return -1;
    }

    // try to measure x, z1 and z2 to check interface
    uint8_t in_buf[7] = {
    XPT2046_CMD_MEASURE_X | XPT2046_CMD_DEFAULT_POWER_MODE, 0x00,
    XPT2046_CMD_MEASURE_Z1 | XPT2046_CMD_DEFAULT_POWER_MODE, 0x00,
    XPT2046_CMD_MEASURE_Z2 | XPT2046_CMD_DEFAULT_POWER_MODE, 0x00, 0x00 };
    uint8_t out_buf[7] = { 0x00 };
    err = driver->communication_cb(driver->user_data, in_buf, out_buf, 7);
    if (err) {
        return err;
    }
    uint16_t x = XPT2046_CONV_VAL(out_buf[1], out_buf[2]);
    uint16_t z1 = XPT2046_CONV_VAL(out_buf[3], out_buf[4]);
    uint16_t z2 = XPT2046_CONV_VAL(out_buf[5], out_buf[6]);
    if (x == z1 && z1 == z2) {
        // something goes wrong
        return -1;
    }

    driver->_is_initialized = 1;
    return 0;
}

int lcd_xpt2046_measure(lcd_xpt2046_driver_t *driver, uint8_t measure_command, int *result) {
    int err;

    if (!driver->_is_initialized) {
        return -1;
    }

    // measure resistance
    uint8_t in_buf[3] = { measure_command | XPT2046_CMD_DEFAULT_POWER_MODE, 0x00, 0x00 };
    uint8_t out_buf[3] = { 0x00 };
    err = driver->communication_cb(driver->user_data, in_buf, out_buf, 3);
    if (err) {
        return err;
    }

    *result = XPT2046_CONV_VAL(out_buf[1], out_buf[2]);
    return 0;
}
