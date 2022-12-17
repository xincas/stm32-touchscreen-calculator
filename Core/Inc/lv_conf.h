/**
 * Minimal LVGL 7.10 configuration.
 *
 * Note: if some sections/options are omitted, then default values from "lv_conf_internal.h" are used.
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
 Graphical settings
 *====================*/

/* Color depth:
 * - 1:  1 byte per pixel
 * - 8:  RGB332
 * - 16: RGB565
 * - 32: ARGB8888
 */
#define LV_COLOR_DEPTH 16

/* Type of coordinates. Should be `int16_t` (or `int32_t` for extreme cases) */
typedef int16_t lv_coord_t;

/*=========================
 Memory manager settings
 *=========================*/

/*
 * Memory configuration:
 * 1: use custom malloc/free. By default it's standard C malloc/free
 * 0: use the built-in `lv_mem_alloc` and `lv_mem_free`
 */
#define LV_MEM_CUSTOM      1

/*=======================
 Input device settings
 *=======================*/

/*==================
 * Feature usage
 *==================*/

/* Types of user data fields */
typedef void *lv_anim_user_data_t;
typedef void *lv_group_user_data_t;
typedef void *lv_fs_drv_user_data_t;

/*  1: Add a `user_data` to drivers and objects*/
#define LV_USE_USER_DATA 1

/*========================
 * Image decoder and cache
 *========================*/

/*Declare the type of the user data of image decoder  */
typedef void *lv_img_decoder_user_data_t;

/*=====================
 *  Compiler settings
 *====================*/

/*===================
 *  HAL settings
 *==================*/

/* Use STM32 HAL tick tick source. */
#define LV_TICK_CUSTOM     1
#if LV_TICK_CUSTOM == 1
#define LV_TICK_CUSTOM_INCLUDE "stm32f3xx_hal.h"         /*Header for the system time function*/
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (HAL_GetTick())     /*Expression evaluating to current system time in ms*/
#endif   /*LV_TICK_CUSTOM*/

/* Types of user data fields */
typedef void *lv_disp_drv_user_data_t;
typedef void *lv_indev_drv_user_data_t;

/*================
 * Log settings
 *===============*/

/*1: Enable the log module*/
#define LV_USE_LOG      1
#if LV_USE_LOG
/* How important log should be added:
 * LV_LOG_LEVEL_TRACE       A lot of logs to give detailed information
 * LV_LOG_LEVEL_INFO        Log important events
 * LV_LOG_LEVEL_WARN        Log if something unwanted happened but didn't cause a problem
 * LV_LOG_LEVEL_ERROR       Only critical issue, when the system may fail
 * LV_LOG_LEVEL_NONE        Do not log anything
 */
#define LV_LOG_LEVEL LV_LOG_LEVEL_INFO

/* 1: Print the log with 'printf';
 * 0: user need to register a callback with `lv_log_register_print_cb`*/
#define LV_LOG_PRINTF 1
#endif  /*LV_USE_LOG*/

/*=================
 * Debug settings
 *================*/

/* If Debug is enabled LittelvGL validates the parameters of the functions.
 * If an invalid parameter is found an error log message is printed and
 * the MCU halts at the error. (`LV_USE_LOG` should be enabled)
 * If you are debugging the MCU you can pause
 * the debugger to see exactly where the issue is.
 *
 * The behavior of asserts can be overwritten by redefining them here.
 * E.g. #define LV_ASSERT_MEM(p)  <my_assert_code>
 */
#define LV_USE_DEBUG 1

/*  Check if the parameter is NULL. (Quite fast) */
#define LV_USE_ASSERT_NULL  1

/*Checks is the memory is successfully allocated or no. (Quite fast) */
#define LV_USE_ASSERT_MEM       1

/* Check the integrity of `lv_mem` after critical operations. (Slow) */
#define LV_USE_ASSERT_MEM_INTEGRITY       0

/* Check the strings.
 * Search for NULL, very long strings, invalid characters, and unnatural repetitions. (Slow)
 * If disabled `LV_USE_ASSERT_NULL` will be performed instead (if it's enabled) */
#define LV_USE_ASSERT_STR 1

/* Check NULL, the object's type and existence (e.g. not deleted). (Quite slow)
 * If disabled `LV_USE_ASSERT_NULL` will be performed instead (if it's enabled) */
#define LV_USE_ASSERT_OBJ 1

/* Check if the styles are properly initialized. (Fast) */
#define LV_USE_ASSERT_STYLE 1

/*==================
 *    FONT USAGE
 *===================*/

/* Declare the type of the user data of fonts (can be e.g. `void *`, `int`, `struct`) */
typedef void *lv_font_user_data_t;

/*================
 *  THEME USAGE
 *================*/

/*=================
 *  Text settings
 *=================*/

/*===================
 *  LV_OBJ SETTINGS
 *==================*/

/* Declare the type of the user data of object (can be e.g. `void *`, `int`, `struct`) */
typedef void *lv_obj_user_data_t;

/*==================
 *  LV OBJ X USAGE
 *================*/

/*==================
 * Non-user section
 *==================*/

/*--END OF LV_CONF_H--*/

#endif /*LV_CONF_H*/
