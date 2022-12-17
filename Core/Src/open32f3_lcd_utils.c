#include "open32f3_lcd_utils.h"

int open32f3_lcd_delay(void *lcd_data, int ms) {
    HAL_Delay(ms);
    return 0;
}

int open32f3_lcd_reset(void *lcd_data) {
    // reset LCD
    HAL_GPIO_WritePin(OPEN32F3_LCD_RESET_PORT, OPEN32F3_LCD_RESET_PIN, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(OPEN32F3_LCD_RESET_PORT, OPEN32F3_LCD_RESET_PIN, GPIO_PIN_SET);
    return 0;
}

int app_lcd_write_register(void *user_data, uint16_t address, uint16_t value) {
	// реализаци�? запи�?и значени�? "value" в реги�?тр "address"
	OPEN32F3_LCD_CLEAR_CS();			//1
	OPEN32F3_LCD_CLEAR_RS();			//2/1
	OPEN32F3_LCD_DATA_WRITE(address);	//2/2
	OPEN32F3_LCD_CLEAR_WR();			//2/3
	OPEN32F3_LCD_SET_WR();				//2/4
	OPEN32F3_LCD_SET_RS();				//2/5
	OPEN32F3_LCD_DATA_WRITE(value);		//3/1
	OPEN32F3_LCD_CLEAR_WR();			//3/2
	OPEN32F3_LCD_SET_WR();				//3/3
	OPEN32F3_LCD_SET_CS();				//4
	return 0;
}

int app_lcd_write_words(void *user_data, uint16_t address, uint16_t *data,
		size_t size) {
	// реализаци�? запи�?и ма�?�?ива "data" размером "size" �?лементов в реги�?тр "address"
	OPEN32F3_LCD_CLEAR_CS();			//1
	OPEN32F3_LCD_CLEAR_RS();			//2/1
	OPEN32F3_LCD_DATA_WRITE(address);	//2/2
	OPEN32F3_LCD_CLEAR_WR();			//2/3
	OPEN32F3_LCD_SET_WR();				//2/4
	OPEN32F3_LCD_SET_RS();				//2/5
	while (size > 0) {
		OPEN32F3_LCD_DATA_WRITE(*data);	//3/1
		OPEN32F3_LCD_CLEAR_WR();		//3/2
		OPEN32F3_LCD_SET_WR();			//3/3
		size--;
		data++;
	}
	OPEN32F3_LCD_SET_CS();				//4
	return 0;
}

int app_lcd_read_reagister(void *user_data, uint16_t address, uint16_t *value) {
	// реализаци�? чтени�? значени�? из реги�?тра "address" в "value"
	OPEN32F3_LCD_CLEAR_CS();			//1
	OPEN32F3_LCD_CLEAR_RS();			//2/1
	OPEN32F3_LCD_DATA_WRITE(address);	//2/2
	OPEN32F3_LCD_CLEAR_WR();			//2/3
	OPEN32F3_LCD_SET_WR();				//2/4
	OPEN32F3_LCD_SET_RS();				//2/5
	OPEN32F3_LCD_DATA_SET_INPUT_MODE();	//3/1
	OPEN32F3_LCD_CLEAR_RD();			//3/2
	OPEN32F3_LCD_SET_RD();				//3/3
	*value = OPEN32F3_LCD_DATA_READ();			//3/4
	OPEN32F3_LCD_DATA_SET_OUTPUT_MODE();			//3/5
	OPEN32F3_LCD_SET_CS();				//4
	return 0;
}
