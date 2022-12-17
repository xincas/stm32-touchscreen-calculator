#ifndef LCD_UTILS_OPEN32F3_LCD_TOUCH_UTILS_H
#define LCD_UTILS_OPEN32F3_LCD_TOUCH_UTILS_H

#include "stddef.h"
#include "stdint.h"
#include "stm32f3xx_hal.h"
#include "lvgl.h"
#include "main.h"


#ifdef __cplusplus
extern "C" {
#endif

#define XPT2046_CMD_MEASURE_Z1 0xB0
#define XPT2046_CMD_MEASURE_Z2 0xC0
#define XPT2046_CMD_MEASURE_Y 0x90
#define XPT2046_CMD_MEASURE_X 0xD0

/**
 * Simple XPT2046 LCD touchpad driver to measure resistive touchpad X,Y,Z1 and Z2 position.
 *
 * Driver usage:
 *
 * 1. Create and clear driver:
 *
 * @code{.cpp}
 * lcd_xpt2046_driver_t touch_driver;
 * lcd_xpt2046_init_clear(&touch_driver);
 * @endcode
 *
 * 2. Attach SPI callback.
 *
 * @code{.cpp}
 * // Set custom data. If you don't need any data, assign NULL.
 * touch_driver.user_data = my_data_ptr;
 * // Set communication callback
 * touch_driver.communication_cb = my_communication_cb;
 * @endcode
 *
 * 3. Initialize driver:
 *
 * @code{.cpp}
 * int res = lcd_xpt2046_init(&touch_driver);
 * if (!res) {
 *     // Driver initialization error.
 *     // Show error somehow and stop application
 *     // ...
 * }
 * @endcode
 *
 * 4. Use driver to measure touch resistance.
 *
 * @code{.cpp}
 * int x_value;
 * int y_value;
 * int z1_value;
 * int z2_value;
 *
 * lcd_xpt2046_measure(&touch_driver, XPT2046_CMD_X_MEASURE, &x_value);
 * lcd_xpt2046_measure(&touch_driver, XPT2046_CMD_y_MEASURE, &y_value);
 * lcd_xpt2046_measure(&touch_driver, XPT2046_CMD_Z1_MEASURE, &z1_value);
 * lcd_xpt2046_measure(&touch_driver, XPT2046_CMD_Z2_MEASURE, &z2_value);
 *
 * ...
 * @endcode
 *
 */
typedef struct {
    /**
     * custom data.
     */
    void *user_data;
    /**
     * Touch sensor communication callback.
     *
     * @param lcd_data custom data
     * @param out_buf buffer with data to transfer
     * @param in_buf buffer with data to receive
     * @param size buffers length
     * @return zero on success, otherwise non-zero value
     */
    int (*communication_cb)(void *user_data, uint8_t *out_buf, uint8_t *in_buf, size_t size);

    // private variables
    int _is_initialized;
} lcd_xpt2046_driver_t;

/**
 * Clear lcd_xpt2046_driver_t driver struct.
 *
 * @param driver
 * @return zero on success, otherwise non-zero value
 */
int lcd_xpt2046_init_clear(lcd_xpt2046_driver_t *driver);

/**
 * Initialize LCD touch controller.
 *
 * @param driver
 * @return zero on success, otherwise non-zero value
 */
int lcd_xpt2046_init(lcd_xpt2046_driver_t *driver);

/**
 * Measure resistance.
 *
 * @param driver
 * @param measure_command - measure command XPT2046_CMD_<TYPE>_MEASURE
 * @param result - measured value
 * @return zero on success, otherwise non-zero value
 */
int lcd_xpt2046_measure(lcd_xpt2046_driver_t *driver, uint8_t measure_command, int *result);


#ifdef __cplusplus
}
#endif

#endif /* LCD_UTILS_OPEN32F3_LCD_TOUCH_UTILS_H */
