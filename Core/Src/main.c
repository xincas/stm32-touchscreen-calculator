/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtc.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd_ili93xx_driver.h"
#include "open32f3_lcd_utils.h"
#include "open32f3_lcd_touch_utils.h"

#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "math.h"

#include "lvgl.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

}

typedef struct {
	lcd_ili93xx_driver_t *lcd_driver;
	lcd_xpt2046_driver_t *touch_driver;

	// Данные для привязки LVGL к дисплею
	size_t buf_size;
	lv_color_t *buf;
	lv_disp_buf_t disp_buf;
	lv_disp_drv_t disp_driver;
	// Объект дисплея LVGL
	lv_disp_t *disp;

	// Данные для привязки LVGL к touch сенсору
	lv_indev_drv_t indev_drv;
	lv_indev_t *indev;
	int16_t last_x_position;
	int16_t last_y_position;

// Любые пользовательские данные для пересчета данных touch сенсора в координаты дисплея.
// Если у вас нет подобных структур, можно удалить данное поле.
//    void *app_touch_data;
} app_open32f3_lvgl_driver_t;

/**
 * Колбек для передачи отрисованного интерфейса на дисплей.
 *
 * LVGL сам вызовет его, когда ему понадобиться передать данные на дисплей.
 */
static void app_open32f3_lvgl_display_flush_cb(lv_disp_drv_t *disp_drv,
		const lv_area_t *area, lv_color_t *color_p) {
	// получить app_open32f3_lvgl_driver_t объект
	app_open32f3_lvgl_driver_t *lvgl_data = disp_drv->user_data;
	// передать изображение на экран
	lcd_ili93xx_fill_area(lvgl_data->lcd_driver, area->x1, area->y1, area->x2,
			area->y2, (uint16_t*) color_p);
	// оповестить LVGL, что передача изображения на экран закончена
	lv_disp_flush_ready(disp_drv);
}

int calculate_touch_position(int x1, int x2, int x3) {
	return (x1 + x2 + x3) / 3;
}
/**
 * Колбек для получения текущих координат касания от сенсорного дисплей.
 *
 * LVGL сам вызовет его, когда ему понадобиться считать данные ввода.
 */
static bool app_open32f3_lvgl_display_touch_sensor_read_cb(
		lv_indev_drv_t *disp_drv, lv_indev_data_t *data) {
	// получить app_open32f3_lvgl_driver_t объект
	app_open32f3_lvgl_driver_t *lvgl_data = disp_drv->user_data;

	int touch_x = 0;
	int touch_y = 0;
	int touch_flag = 0;
	lcd_xpt2046_driver_t *touch_driver = lvgl_data->touch_driver;

	int x1, y1, z11, z21, x2, y2, z12, z22, x3, y3, z13, z23,
		Z1disp = 0, Z2disp = 0;

	lcd_xpt2046_measure(touch_driver, XPT2046_CMD_MEASURE_X, &x1);
	lcd_xpt2046_measure(touch_driver, XPT2046_CMD_MEASURE_Y, &y1);
	lcd_xpt2046_measure(touch_driver, XPT2046_CMD_MEASURE_Z1, &z11);
	lcd_xpt2046_measure(touch_driver, XPT2046_CMD_MEASURE_Z2, &z21);
	lcd_xpt2046_measure(touch_driver, XPT2046_CMD_MEASURE_X, &x2);
	lcd_xpt2046_measure(touch_driver, XPT2046_CMD_MEASURE_Y, &y2);
	lcd_xpt2046_measure(touch_driver, XPT2046_CMD_MEASURE_Z1, &z12);
	lcd_xpt2046_measure(touch_driver, XPT2046_CMD_MEASURE_Z2, &z22);
	lcd_xpt2046_measure(touch_driver, XPT2046_CMD_MEASURE_X, &x3);
	lcd_xpt2046_measure(touch_driver, XPT2046_CMD_MEASURE_Y, &y3);
	lcd_xpt2046_measure(touch_driver, XPT2046_CMD_MEASURE_Z1, &z13);
	lcd_xpt2046_measure(touch_driver, XPT2046_CMD_MEASURE_Z2, &z23);

	Z1disp = calculate_touch_position(z11, z12, z13);
	Z2disp = calculate_touch_position(z21, z22, z23);

	if (abs(Z1disp - Z2disp) < 3800) {				//ЕСТЬ

		touch_x = calculate_touch_position(x1, x2, x3);
		touch_y = calculate_touch_position(y1, y2, y3);
		touch_x = 0.062 * touch_x ;
		touch_y = 0.084 * touch_y ;
		if (touch_x <= 0) {
			touch_x = 0;
		}
		if (touch_x >= 240 ) {
			touch_x = 240 ;
		}
		if (touch_y <= 0) {
			touch_y = 0;
		}
		if (touch_y >= 320 ) {
			touch_y = 320 ;
		}
		touch_flag = 1;
	}

	// обработка логики устройства ввода в соответствии с LVGL
	if (touch_flag) {
		lvgl_data->last_x_position = touch_x;
		lvgl_data->last_y_position = touch_y;
	}
	data->point.x = lvgl_data->last_x_position;
	data->point.y = lvgl_data->last_y_position;
	if (touch_flag) {
		// касание дисплея обнаружено
		data->state = LV_INDEV_STATE_PR;
	} else {
		// касание дисплея не обнаружено
		data->state = LV_INDEV_STATE_REL;
	}
	return false;
}

/**
 * Функция инициализации дисплея для LVGL.
 *
 * @param lcd_driver инициализированный драйвер дисплея
 * @param touch_driver инициализированный драйвер touch сенсора
 * @param lvgl_data неинициализированные объект app_open32f3_lvgl_driver_t
 * @return 0 в случае успеха, иначе не нулевое значение
 */
int app_open32f3_lvgl_init(app_open32f3_lvgl_driver_t *lvgl_data,
		lcd_ili93xx_driver_t *lcd_driver, lcd_xpt2046_driver_t *touch_driver) {
	int err;

	//
	// Шаг 1. Базовая инициализация LVGL
	//
	lv_init();

	// базовая инициализация lvgl_data
	lvgl_data->lcd_driver = lcd_driver;
	lvgl_data->touch_driver = touch_driver;

	//
	// Шаг 2. Подключение дисплея
	//

	// получить ширину и высоту дисплея
	int16_t width;
	int16_t height;
	err = lcd_ili93xx_get_width(lcd_driver, &width);
	if (err) {
		return err;
	}
	err = lcd_ili93xx_get_height(lcd_driver, &height);
	if (err) {
		return err;
	}

	// Шаг 2.1. �?нициализация буфера отрисовки кадра
	lvgl_data->buf_size = width * 20;
	lvgl_data->buf = malloc(sizeof(lv_color_t) * lvgl_data->buf_size);
	lv_disp_buf_init(&lvgl_data->disp_buf, lvgl_data->buf, NULL, lvgl_data->buf_size);
	// Шаг 2.2. Регистрация дисплея
	lv_disp_drv_init(&lvgl_data->disp_driver);
	lvgl_data->disp_driver.hor_res = width;
	lvgl_data->disp_driver.ver_res = height;
	lvgl_data->disp_driver.buffer = &lvgl_data->disp_buf;
	lvgl_data->disp_driver.user_data = lvgl_data;
	lvgl_data->disp_driver.flush_cb = app_open32f3_lvgl_display_flush_cb;
	lvgl_data->disp = lv_disp_drv_register(&lvgl_data->disp_driver);
	if (lvgl_data->disp == NULL) {
		return -1;
	}

	//
	// Шаг 3. �?нициализация устройства ввода
	//
	lv_indev_drv_init(&lvgl_data->indev_drv);
	// Будем использовать устройство тип "POINTER"
	lvgl_data->indev_drv.type = LV_INDEV_TYPE_POINTER;
	lvgl_data->last_x_position = 0;
	lvgl_data->last_y_position = 0;
	lvgl_data->indev_drv.user_data = lvgl_data;
	lvgl_data->indev_drv.disp = lvgl_data->disp;
	
	// Регистрация устройства ввода
	lvgl_data->indev_drv.read_cb =
			app_open32f3_lvgl_display_touch_sensor_read_cb;
	lvgl_data->indev = lv_indev_drv_register(&lvgl_data->indev_drv);
	if (lvgl_data->indev == NULL) {
		return -1;
	}

	return 0;
}

static const char * btnm_map[] = {"AC", "MRC", "M-", "M+", "\n",
								  "7", "8", "9", "+", "\n",
                                  "4", "5", "6", "-", "\n",
								  "1", "2", "3", "*", "\n",
								  "0", ".", "=", "/", ""
                                 };

typedef struct {
	lv_style_t btn_style;
	lv_style_t kbd_style;
	lv_style_t inp_style;

	lv_obj_t* text_main;
	lv_obj_t* text_history;
	lv_obj_t* text_mem;
	lv_obj_t* btnm;

} app_scene_t;

app_scene_t app_scene;

static const short int MAX_LENGTH = 24;
static const float MIN_SCIENTIFIC_NUMBER = 1000000000.f;
//static const uint32_t MEM_ADDR = 0x08031F9C;

static short FLAGS = 0;

typedef enum {
	PLUS = 43,
	MINUS = 45,
	MUL = 42,
	DIV = 47,
	NONE = 0
} e_function;

typedef enum {
	SQ_FUN = 0,
	SQ_EQ,
	MEM_SAVE,
	ERR
} e_flags;

float read_mem()
{
	uint32_t value = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);
	float fvalue = .0f;
	memcpy(&fvalue, &value, sizeof fvalue);
	//memcpy(&fvalue, MEM_ADDR, sizeof fvalue);
	return fvalue;
}

void write_mem(float num)
{
	HAL_PWR_EnableBkUpAccess();

	uint32_t fbits = 0;
	memcpy(&fbits, &num, sizeof fbits);
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, fbits);

	HAL_PWR_DisableBkUpAccess();

	/*HAL_FLASH_Unlock();

	FLASH_EraseInitTypeDef ef;
	HAL_StatusTypeDef stat;
	ef.TypeErase = FLASH_TYPEERASE_PAGES;
	ef.PageAddress = MEM_ADDR;
	ef.NbPages = 1;
	uint32_t tmp;
	HAL_FLASHEx_Erase(&ef, &tmp);

	stat = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, MEM_ADDR, fbits);

	if (stat != HAL_OK)
	{
		set_flag(ERR, true);
		lv_textarea_set_text(app_scene.text_main, "ERROR: Write memory fail!");
	}

	HAL_FLASH_Lock();*/
}

bool is_char_contain(char ch, const char* str)
{
	for (int i = 0; i < strlen(str); i++)
		if (str[i] == ch)
			return true;
	return false;
}

const char* save_display(const char *text_on_disp, e_function fun) {
	char f[2] = " \0";
	f[0] = (char) fun;

	//lv_textarea_set_text(app_scene.text_main, "");
	if (strlen(text_on_disp) == 0)
	{
		char res[5] = "0.0 \0";
		res[3] = (char) fun;
		lv_textarea_set_text(app_scene.text_history, res);
		return "0";
	}
	else
	{
		lv_textarea_set_text(app_scene.text_history, strcat(strdup(text_on_disp), f));
		return text_on_disp;
	}

}

bool float_equal(float a, float b, float epsilon)
{
	return fabs(a - b) < epsilon;
}

void format_result(char* buf, float res)
{
	if (res < MIN_SCIENTIFIC_NUMBER)
		sprintf(buf, "%.5f", res);
	else
		sprintf(buf, "%G", res);
}

static inline short get_flag(e_flags bit)
{
	return (FLAGS >> bit) & 0b1;
}

void set_flag(e_flags bit, bool value)
{
	if (value)
		FLAGS |= 0b1 << bit;
	else
		FLAGS &= ~(0b1 << bit);
}

const char* eval(const char* left, const char* right, e_function fun)
{
	float left_f = atof(left);
	float right_f = atof(right);
	char buf[MAX_LENGTH];
	switch(fun)
	{
		case PLUS:
			format_result(buf, left_f + right_f);
			break;
		case MINUS:
			format_result(buf, left_f - right_f);
			break;
		case MUL:
			format_result(buf, left_f * right_f);
			break;
		case DIV:
			if (float_equal(0.f, right_f, 0.00000001f))
			{
				set_flag(ERR, true);
				strcpy(buf, "ERROR: Division by 0");
			}
			else
				format_result(buf, left_f / right_f);
			break;
		case NONE:
			strcpy(buf, right);
			break;
	}

	return strdup(buf);
}

void all_clear(e_function* fun, char **first_arg, char** text_main) {
	//free(*first_arg);
	//free(*text_main);
	lv_textarea_set_text(app_scene.text_main, "");
	lv_textarea_set_text(app_scene.text_history, "");
	//(*first_arg) = "";
	(*fun) = NONE;
}

void fun_execute(const char** first_arg, const  char* text_on_disp, e_function* fun, e_function current_operation)
{
	if (get_flag(SQ_FUN))
	{
		text_on_disp = eval(*first_arg, text_on_disp, *fun);
	}
	set_flag(SQ_FUN, true);
	set_flag(SQ_EQ, false);
	*fun = current_operation;
	*first_arg = save_display(text_on_disp, *fun);
	lv_textarea_set_text(app_scene.text_main, "");
}

void memory_operation(const char* text_on_disp, e_function operation)
{
	if (!get_flag(MEM_SAVE))
	{
		if (operation == PLUS)
			write_mem(atof(text_on_disp));
		else
			write_mem(-atof(text_on_disp));
		lv_label_set_text(app_scene.text_mem, "M");
		set_flag(MEM_SAVE, true);
	}
	else
	{
		if (operation == PLUS)
			write_mem(read_mem() + atof(text_on_disp));
		else
			write_mem(read_mem() - atof(text_on_disp));

		if (float_equal(read_mem(), 0.f, 0.0000001))
		{
			lv_label_set_text(app_scene.text_mem, "");
			set_flag(MEM_SAVE, false);
		}
	}
}

static void keyboard_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        uint16_t id = lv_btnmatrix_get_active_btn(obj);
        static uint16_t prev_id = 0;
        static char* first_arg;
        // 0 bit - sequence of functions
        // 1 bit - sequence of equality
        // 2 bit - memory first save
        // 3 bit - MRC toggle
        // 4 bit - error state
        static e_function fun = NONE;
        const char* txt = lv_btnmatrix_get_active_btn_text(obj);
        const char* text_on_disp = strdup(lv_textarea_get_text(app_scene.text_main));
        //float mem_value;
        //float dsp_value;
        /*
         * 	AC_0	MRC_1	M-_2	M+_3
         * 	7_4		8_5		9_6		+_7
         * 	4_8		5_9		6_10	-_11
         * 	1_12	2_13	3_14	*_15
         * 	0_16	._17	=_18	/_19
         *
         */
        if (get_flag(ERR))
        {
        	set_flag(ERR, false);
			all_clear(&fun, &first_arg, &text_on_disp);
        }

        switch (id)
        {
        	case 0: // AC
        		all_clear(&fun, &first_arg, &text_on_disp);
        		break;
        	case 1: // MRC
        		if (get_flag(MEM_SAVE) && prev_id == id)
        		{
        			set_flag(MEM_SAVE, false);
        			write_mem(.0f);
        			lv_label_set_text(app_scene.text_mem, "");
        		}
        		else if (get_flag(MEM_SAVE))
        		{
        			//mem_value = read_mem();
        			//format_result(text_on_disp, mem_value);
        			format_result(text_on_disp, read_mem());
        			lv_textarea_set_text(app_scene.text_main, text_on_disp);
        		}
        		break;
        	case 2: // M-
        		memory_operation(text_on_disp, MINUS);
        		break;
        	case 3: // M+
        		memory_operation(text_on_disp, PLUS);
        		break;

        	case 7: // +
        		fun_execute(&first_arg, text_on_disp, &fun, PLUS);
        		break;
        	case 11: // -
        		fun_execute(&first_arg, text_on_disp, &fun, MINUS);
        		break;
        	case 15: // *
        		fun_execute(&first_arg, text_on_disp, &fun, MUL);
        		break;
        	case 19: // /
        		fun_execute(&first_arg, text_on_disp, &fun, DIV);
        		break;

        	case 17: // .
        		if (strlen(text_on_disp) == 0)
        			lv_textarea_set_text(app_scene.text_main, "0.");
        		else if (!is_char_contain('.', text_on_disp))
        			lv_textarea_add_text(app_scene.text_main, txt);
        	    break;

        	case 18: // =
        		if (get_flag(SQ_FUN))
        		{
        			lv_textarea_set_text(app_scene.text_main, eval(first_arg, text_on_disp, fun));
        			if (fun != MUL)
						first_arg = strdup(text_on_disp);
        			save_display(first_arg, fun);
            		set_flag(SQ_FUN, false);
            		set_flag(SQ_EQ, true);
        		}
        		else if (get_flag(SQ_EQ))
				{
        			if (fun == MUL)
        				lv_textarea_set_text(app_scene.text_main, eval(first_arg, text_on_disp, fun));
        			else
        				lv_textarea_set_text(app_scene.text_main, eval(text_on_disp, first_arg, fun));
				}
        		break;

        	case 4:
        	case 5:
        	case 6:
        	case 8:
        	case 9:
        	case 10:
        	case 12:
        	case 13:
        	case 14:
        	case 16:
        		lv_textarea_add_text(app_scene.text_main, txt);
        		break;
        }
        prev_id = id;
        //free(text_on_disp);
    }
}

/**
 * Создание и настройка виджетов интерфейса
 *
 * @param app_scene неинициализированный объект app_scene_t
 * @param scr объект экрана
 * @return 0 в случае успеха, иначе не нулевое значение
 */
int app_scene_init(app_scene_t *app_scene, lv_obj_t *screen) {

	//docs.lvgl.io/7.11/overview/style.html
	lv_obj_set_style_local_bg_color(screen, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_PURPLE);
	lv_style_init(&app_scene->btn_style);
	lv_style_init(&app_scene->kbd_style);
	lv_style_init(&app_scene->inp_style);

	// Стили кнопок клавиатуры
	lv_style_set_bg_color(&app_scene->btn_style, LV_STATE_DEFAULT, lv_color_hex(0xf777f7));
	lv_style_set_bg_color(&app_scene->btn_style, LV_STATE_PRESSED, lv_color_hex(0xedcaed));
	lv_style_set_text_font(&app_scene->btn_style, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);
	lv_style_set_text_font(&app_scene->btn_style, LV_STATE_PRESSED, LV_THEME_DEFAULT_FONT_TITLE);

	// Стиль клавиатуры
	lv_style_set_bg_color(&app_scene->kbd_style, LV_STATE_DEFAULT, LV_COLOR_PURPLE);
	lv_style_set_bg_color(&app_scene->kbd_style, LV_STATE_PRESSED, LV_COLOR_PURPLE);
	lv_style_set_bg_color(&app_scene->kbd_style, LV_STATE_FOCUSED, LV_COLOR_PURPLE);
	lv_style_set_border_width(&app_scene->kbd_style, LV_STATE_DEFAULT, 0);
	lv_style_set_border_width(&app_scene->kbd_style, LV_STATE_FOCUSED, 0);

	// Стиль клавиатуры
	lv_style_set_bg_color(&app_scene->inp_style, LV_STATE_DEFAULT, lv_color_hex(0xf777f7));
	lv_style_set_bg_color(&app_scene->inp_style, LV_STATE_PRESSED, lv_color_hex(0xf777f7));
	lv_style_set_bg_color(&app_scene->inp_style, LV_STATE_FOCUSED, lv_color_hex(0xf777f7));
	lv_style_set_border_width(&app_scene->inp_style, LV_STATE_DEFAULT, 0);
	lv_style_set_border_width(&app_scene->inp_style, LV_STATE_FOCUSED, 0);
	lv_style_set_clip_corner(&app_scene->inp_style, LV_STATE_DEFAULT, false);


	//https://docs.lvgl.io/7.11/widgets/btnmatrix.html
	// �?нициализация кнопок
	app_scene->btnm = lv_btnmatrix_create(screen, NULL);
    lv_btnmatrix_set_map(app_scene->btnm, btnm_map);
    lv_obj_set_size(app_scene->btnm, 240, 240);
    lv_obj_add_style(app_scene->btnm, LV_BTNMATRIX_PART_BTN, &app_scene->btn_style);
    lv_obj_add_style(app_scene->btnm, LV_BTNMATRIX_PART_BG, &app_scene->kbd_style);
    lv_obj_align(app_scene->btnm, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_obj_set_event_cb(app_scene->btnm, keyboard_event_handler);

    //https://docs.lvgl.io/master/widgets/textarea.html
    // �?нициализация текста
    app_scene->text_history = lv_textarea_create(screen, NULL);
    lv_textarea_set_one_line(app_scene->text_history, true);
    lv_obj_set_size(app_scene->text_history, 230, 15);
    lv_textarea_set_text(app_scene->text_history, "");
    lv_textarea_set_cursor_hidden(app_scene->text_history, true);
    lv_obj_align(app_scene->text_history, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    lv_obj_add_style(app_scene->text_history, LV_TEXTAREA_PART_BG, &app_scene->inp_style);
    lv_textarea_set_text_align(app_scene->text_history, LV_LABEL_ALIGN_RIGHT);

    app_scene->text_main = lv_textarea_create(screen, NULL);
    lv_textarea_set_max_length(app_scene->text_main, MAX_LENGTH);
    lv_textarea_set_one_line(app_scene->text_main, true);
    lv_obj_set_size(app_scene->text_main, 230, 30);
    lv_textarea_set_text(app_scene->text_main, "");
    lv_textarea_set_cursor_hidden(app_scene->text_main, true);
    lv_obj_align(app_scene->text_main, app_scene->text_history, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_add_style(app_scene->text_main, LV_TEXTAREA_PART_BG, &app_scene->inp_style);
    lv_textarea_set_text_align(app_scene->text_main, LV_LABEL_ALIGN_RIGHT);

    //Проверка памяти
    float mem_val = read_mem();

    app_scene->text_mem = lv_label_create(screen, NULL);
    lv_label_set_recolor(app_scene->text_mem, true);
    lv_label_set_align(app_scene->text_mem, LV_LABEL_ALIGN_LEFT);
    lv_obj_set_width(app_scene->text_mem, 50);
    lv_obj_align(app_scene->text_mem, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    if (!float_equal(0.f, mem_val, 0.0000001f))
    {
    	set_flag(MEM_SAVE, true);
    	lv_label_set_text(app_scene->text_mem, "M");
    }
    else
    {
    	lv_label_set_text(app_scene->text_mem, "");
    }

	return 0;
}

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/**
 * GNUC specific stdout/stderr hooks.
 *
 * @note: other compilers have other stdout/stderr hooks.
 */
#ifndef __GNUC__
#error Unknown compilator
#else
/*#include <unistd.h>
int _write(int fd, const void *buf, size_t count) {
	int res = 0;
	if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
		// write data to UART
		HAL_StatusTypeDef hal_res = HAL_UART_Transmit(&huart2, (uint8_t*) buf,
				count, HAL_MAX_DELAY);
		res = hal_res == HAL_OK ? count : -1;
	} else {
		res = -1;
	}
	return res;
}*/
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static int app_lcd_touch_callback(void *user_data, uint8_t *out_buf,
		uint8_t *in_buf, size_t size) {
	// у�?тановить CS линию в 0 дл�? начала приема/передачи
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	// передать/прин�?ть данные
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, out_buf, in_buf,
			size, HAL_MAX_DELAY);
	// у�?тановить CS линию в 1 дл�? конца приема/передачи
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	return status == HAL_OK ? 0 : -1;
}

void init_lcd_driver(lcd_ili93xx_driver_t *lcd_driver) {
	lcd_ili93xx_init_clear(&*lcd_driver);
	lcd_driver->user_data = NULL;
	lcd_driver->read_reg = app_lcd_read_reagister;
	lcd_driver->write_reg = app_lcd_write_register;
	lcd_driver->write_words = app_lcd_write_words;
	lcd_driver->reset = open32f3_lcd_reset;
	lcd_driver->delay = open32f3_lcd_delay;
	int err = lcd_ili93xx_init(&*lcd_driver);
	if (err) {
		printf("ERROR: DRIVER %i!\n", err);
	}
}

void init_touch_driver(lcd_xpt2046_driver_t *touch_driver) {
	lcd_xpt2046_init_clear(&*touch_driver);
	touch_driver->user_data = NULL;
	touch_driver->communication_cb = app_lcd_touch_callback;
	int err = lcd_xpt2046_init(&*touch_driver);
	if (err) {
		printf("ERROR: TOUCH DRIVER!\n");
	}
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

	lcd_ili93xx_driver_t lcd_driver;
	init_lcd_driver(&lcd_driver);

	lcd_xpt2046_driver_t touch_driver;
	init_touch_driver(&touch_driver);

	app_open32f3_lvgl_driver_t lvgl_driver;
	int err = app_open32f3_lvgl_init(&lvgl_driver, &lcd_driver, &touch_driver);
	if (err) {
		printf("ERROR: TOUCH DRIVER!\n");
	}

	lv_obj_t *screen = lv_disp_get_scr_act(lvgl_driver.disp);
	err = app_scene_init(&app_scene, screen);
	if (err) {
		printf("ERROR: Creating & initialization scene!\n");
	}

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  lv_task_handler();
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
		/* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
