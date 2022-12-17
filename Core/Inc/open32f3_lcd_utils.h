#ifndef OPEN32F3_LCD_UTILS_H
#define OPEN32F3_LCD_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f3xx_hal.h"

// helper macros with ports and pins
#define OPEN32F3_LCD_CONTROL_PORT GPIOB
#define OPEN32F3_LCD_DATA_PORT GPIOD
#define OPEN32F3_LCD_CS_PIN GPIO_PIN_8
#define OPEN32F3_LCD_RS_PIN GPIO_PIN_9
#define OPEN32F3_LCD_WR_PIN GPIO_PIN_10
#define OPEN32F3_LCD_RD_PIN GPIO_PIN_11
#define OPEN32F3_LCD_RESET_PIN GPIO_PIN_11
#define OPEN32F3_LCD_RESET_PORT GPIOC
#define OPEN32F3_LCD_BACKLIGHT_PIN GPIO_PIN_10
#define OPEN32F3_LCD_BACKLIGHT_PORT GPIOC
#define OPEN32F3_LCD_PORT_MODER_INPUT 0x00000000
#define OPEN32F3_LCD_PORT_MODER_OUTPUT 0x55555555

// enable lcd bus
#define OPEN32F3_LCD_CLEAR_CS() OPEN32F3_LCD_CONTROL_PORT->BRR |= OPEN32F3_LCD_CS_PIN
// disable lcd bus
#define OPEN32F3_LCD_SET_CS() OPEN32F3_LCD_CONTROL_PORT->BSRR |= OPEN32F3_LCD_CS_PIN
// switch to index mode
#define OPEN32F3_LCD_CLEAR_RS() OPEN32F3_LCD_CONTROL_PORT->BRR |= OPEN32F3_LCD_RS_PIN
// switch to data mode
#define OPEN32F3_LCD_SET_RS() OPEN32F3_LCD_CONTROL_PORT->BSRR |= OPEN32F3_LCD_RS_PIN
// start write signal
#define OPEN32F3_LCD_CLEAR_WR() OPEN32F3_LCD_CONTROL_PORT->BRR |= OPEN32F3_LCD_WR_PIN
// stop write signal
#define OPEN32F3_LCD_SET_WR() OPEN32F3_LCD_CONTROL_PORT->BSRR |= OPEN32F3_LCD_WR_PIN
// start read signal
#define OPEN32F3_LCD_CLEAR_RD() OPEN32F3_LCD_CONTROL_PORT->BRR |= OPEN32F3_LCD_RD_PIN
// stop read signal
#define OPEN32F3_LCD_SET_RD() OPEN32F3_LCD_CONTROL_PORT->BSRR |= OPEN32F3_LCD_RD_PIN
// read data from data bus
#define OPEN32F3_LCD_DATA_READ() OPEN32F3_LCD_DATA_PORT->IDR
// write data from data bus
#define OPEN32F3_LCD_DATA_WRITE(value) OPEN32F3_LCD_DATA_PORT->ODR = value
// switch data bus pins to input mode
#define OPEN32F3_LCD_DATA_SET_INPUT_MODE() OPEN32F3_LCD_DATA_PORT->MODER = OPEN32F3_LCD_PORT_MODER_INPUT
// switch data bus pins to output mode
#define OPEN32F3_LCD_DATA_SET_OUTPUT_MODE() OPEN32F3_LCD_DATA_PORT->MODER = OPEN32F3_LCD_PORT_MODER_OUTPUT

/**
 * LCD driver delay callback.
 */
int open32f3_lcd_delay(void *lcd_data, int ms);

/**
 * LCD driver reset callback.
 */
int open32f3_lcd_reset(void *lcd_data);

int app_lcd_write_register(void *user_data, uint16_t address, uint16_t value);

int app_lcd_write_words(void *user_data, uint16_t address, uint16_t *data, size_t size);

int app_lcd_read_reagister(void *user_data, uint16_t address, uint16_t *value);

#ifdef __cplusplus
}
#endif

#endif // OPEN32F3_LCD_UTILS_H
