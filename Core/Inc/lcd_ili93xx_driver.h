#ifndef LCD_ILI93XX_DRIVER
#define LCD_ILI93XX_DRIVER

#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Common ILI9325 LCD driver.
 *
 * Driver usage:
 *
 * 1. Create and clear driver:
 *
 * @code{.cpp}
 * lcd_ili93xx_driver_t lcd_driver;
 * lcd_ili93xx_init_clear(&lcd_driver);
 * @endcode
 *
 * 2. Attach callbacks to handler:
 *
 * @code{.cpp}
 * // set custom data. If don't need any data, assign NULL.
 * lcd_driver.user_data = my_data_ptr;
 * // Set delay callback (it's needed for LCD initialization)
 * lcd_driver.delay = my_delay;
 * // Set callback that resets LCD.
 * lcd_driver.reset = my_lcd_reset_callback;
 * // Set callback that can write a single value to register.
 * lcd_driver.write_reg = my_lcd_write_reg;
 * // Set callback that can read a single value from register.
 * lcd_driver.read_reg = my_lcd_read_reg;
 * // Set callback that can write multiple word to register.
 * lcd_driver.write_words = my_lcd_write_words;
 * @endcode
 *
 * 3. Initialize driver:
 *
 * @code{.cpp}
 * int res = lcd_ili93xx_init(&lcd_driver);
 * if (!res) {
 *     // Driver initialization error.
 *     // Show error somehow and stop application
 *     // ...
 * }
 * @endcode
 *
 * 4. Use driver for some operations.
 *
 * @code{.cpp}
 * lcd_ili93xx_fill_area_color(&lcd_driver, 0, 0, 240 - 1, 320 - 1, LCD_ILI93XX_COLOR_GREEN);
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
     * Delay callback.
     *
     * @param lcd_data custom data
     * @param ms expected delay in milliseconds
     * @return zero on success, otherwise non-zero value
     */
    int (*delay)(void *user_data, int ms);

    /**
     * Hardware LCD reset callback.
     *
     * @param lcd_data custom data
     * @return zero on success, otherwise non-zero value
     */
    int (*reset)(void *user_data);

    /**
     * Write LCD register callback.
     *
     * @param lcd_data custom data
     * @param address register address
     * @param value register value
     * @return zero on success, otherwise non-zero value
     */
    int (*write_reg)(void *user_data, uint16_t address, uint16_t value);

    /**
     * Write LCD register callback.
     *
     * @param lcd_data custom data
     * @param address register address
     * @param value register value address
     * @return zero on success, otherwise non-zero value
     */
    int (*read_reg)(void *user_data, uint16_t address, uint16_t *value);

    /**
     * Write words to LCD register callback.
     *
     * @param lcd_data custom data
     * @param address register address
     * @param data array with words
     * @param size data array size
     * @return zero on success, otherwise non-zero value
     */
    int (*write_words)(void *user_data, uint16_t address, uint16_t *data, size_t size);

    // private variables
    int _is_initialized;
    int _model;
    int16_t _width;
    int16_t _height;
} lcd_ili93xx_driver_t;

/**
 * Clear lcd_ili93xx_driver_t driver struct.
 *
 * @param driver
 * @return zero on success, otherwise non-zero value
 */
int lcd_ili93xx_init_clear(lcd_ili93xx_driver_t *driver);

/**
 * Initialize LCD.
 *
 * @param driver
 * @return zero on success, otherwise non-zero value
 */
int lcd_ili93xx_init(lcd_ili93xx_driver_t *driver);

/**
 * Get LCD width.
 *
 * @param driver
 * @param width
 * @return zero on success, otherwise non-zero value
 */
int lcd_ili93xx_get_width(lcd_ili93xx_driver_t *driver, int16_t *width);

/**
 * Get LCD height.
 *
 * @param driver
 * @param height
 * @return zero on success, otherwise non-zero value
 */
int lcd_ili93xx_get_height(lcd_ili93xx_driver_t *driver, int16_t *height);

/**
 * Fill specified area with a color.
 *
 * @param driver
 * @param x1 left rectangle coordinate (inclusive)
 * @param y1 upper rectangle coordinate (inclusive)
 * @param x2 right rectangle coordinate (inclusive)
 * @param y2 bottom rectangle coordinate (inclusive)
 * @param color rectangle color
 * @return zero on success, otherwise non-zero value
 */
int lcd_ili93xx_fill_area_color(lcd_ili93xx_driver_t *driver, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

/**
 * Fill specified area.
 *
 * @param driver
 * @param x1 left rectangle coordinate (inclusive)
 * @param y1 upper rectangle coordinate (inclusive)
 * @param x2 right rectangle coordinate (inclusive)
 * @param y2 bottom rectangle coordinate (inclusive)
 * @param color 2d array with rectangle pixels
 * @return zero on success, otherwise non-zero value
 */
int lcd_ili93xx_fill_area(lcd_ili93xx_driver_t *driver, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t *colors_data);

/**
 * Helper function to convert RGB color to RGB565 one.
 *
 * @param red red component from 0 to 255
 * @param green green component from 0 to 255
 * @param blue blue component from 0 to 255
 * @return RGB565 color
 */
inline uint16_t lcd_ili93xx_rgb565_convert(uint8_t red, uint8_t green, uint8_t blue) {
    return (((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3));
}

/**
 * Helper macros to convert RGB color to RGB565 one.
 *
 * @param red red component from 0 to 255
 * @param green green component from 0 to 255
 * @param blue blue component from 0 to 255
 * @return RGB565 color
 */
#define LCD_ILI93XX_RGB565_CONVERT_M(red, green, blue) (((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3))

/**
 * Color constants.
 */
enum lcd_ili93xx_colors {
    LCD_ILI93XX_COLOR_BLACK = LCD_ILI93XX_RGB565_CONVERT_M(0x00, 0x00, 0x00),
    LCD_ILI93XX_COLOR_MAROON = LCD_ILI93XX_RGB565_CONVERT_M(0x80, 0x00, 0x00),
    LCD_ILI93XX_COLOR_GREEN = LCD_ILI93XX_RGB565_CONVERT_M(0x00, 0x80, 0x00),
    LCD_ILI93XX_COLOR_OLIVE = LCD_ILI93XX_RGB565_CONVERT_M(0x80, 0x80, 0x00),
    LCD_ILI93XX_COLOR_NAVY = LCD_ILI93XX_RGB565_CONVERT_M(0x00, 0x00, 0x80),
    LCD_ILI93XX_COLOR_PURPLE = LCD_ILI93XX_RGB565_CONVERT_M(0x80, 0x00, 0x80),
    LCD_ILI93XX_COLOR_TEAL = LCD_ILI93XX_RGB565_CONVERT_M(0x00, 0x80, 0x80),
    LCD_ILI93XX_COLOR_SILVER = LCD_ILI93XX_RGB565_CONVERT_M(0xC0, 0xC0, 0xC0),
    LCD_ILI93XX_COLOR_GRAY = LCD_ILI93XX_RGB565_CONVERT_M(0x80, 0x80, 0x80),
    LCD_ILI93XX_COLOR_RED = LCD_ILI93XX_RGB565_CONVERT_M(0xFF, 0x00, 0x00),
    LCD_ILI93XX_COLOR_LIME = LCD_ILI93XX_RGB565_CONVERT_M(0xFF, 0xFF, 0x00),
    LCD_ILI93XX_COLOR_BLUE = LCD_ILI93XX_RGB565_CONVERT_M(0x00, 0x00, 0xFF),
    LCD_ILI93XX_COLOR_FUCHSIA = LCD_ILI93XX_RGB565_CONVERT_M(0xFF, 0x00, 0xFF),
    LCD_ILI93XX_COLOR_AQUA = LCD_ILI93XX_RGB565_CONVERT_M(0x00, 0xFF, 0xFF),
    LCD_ILI93XX_COLOR_WHITE = LCD_ILI93XX_RGB565_CONVERT_M(0xFF, 0xFF, 0xFF)
};

#ifdef __cplusplus
}
#endif

#endif
