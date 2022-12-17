#include "lcd_ili93xx_driver.h"
#include "stdbool.h"
#include "stdlib.h"

#define ILI93XX_LCD_WIDTH 240
#define ILI93XX_LCD_HEIGTH 320

/* LCD models */
#define ILI93XX_LCD_MODEL_ILI9320 0 /* 0x9320 */
#define ILI93XX_LCD_MODEL_ILI9325 1 /* 0x9325 */
#define ILI93XX_LCD_MODEL_ILI9328 2 /* 0x9328 */
#define ILI93XX_LCD_MODEL_ILI9331 3 /* 0x9331 */

// helper register addresses
#define ILI93XX_LCD_REGISTER_GRAM_X_ADDR 0x0020
#define ILI93XX_LCD_REGISTER_GRAM_Y_ADDR 0x0021
#define ILI93XX_LCD_REGISTER_GRAM_WRITE_DATA 0x0022

#define ILI93XX_LCD_REGISTER_GRAM_X_ADDR_START 0x0050
#define ILI93XX_LCD_REGISTER_GRAM_X_ADDR_END 0x0051
#define ILI93XX_LCD_REGISTER_GRAM_Y_ADDR_START 0x0052
#define ILI93XX_LCD_REGISTER_GRAM_Y_ADDR_END 0x0053

int lcd_ili93xx_init_clear(lcd_ili93xx_driver_t *driver) {
    driver->user_data = NULL;
    driver->reset = NULL;
    driver->read_reg = NULL;
    driver->write_reg = NULL;
    driver->write_words = NULL;
    driver->_height = ILI93XX_LCD_HEIGTH;
    driver->_width = ILI93XX_LCD_WIDTH;
    driver->_is_initialized = 0;
    driver->_model = -1;
    return 0;
}

static int ili93xx_lcd_test_write_reg(lcd_ili93xx_driver_t *driver, uint16_t test_reg_addr, uint16_t test_vals[2]) {
    uint16_t act_val = 0;
    for (int i = 0; i < 2; i++) {
        driver->write_reg(driver->user_data, test_reg_addr, test_vals[i]);
        driver->read_reg(driver->user_data, test_reg_addr, &act_val);
        if (act_val != test_vals[i]) {
            return -4;
        }
    }
    return 0;
}

int lcd_ili93xx_init(lcd_ili93xx_driver_t *driver) {
    int err;

    if (driver->_is_initialized) {
        return -2;
    }

    if (driver->reset == NULL) {
        return -1;
    }
    if (driver->read_reg == NULL) {
        return -1;
    }
    if (driver->write_reg == NULL) {
        return -1;
    }
    if (driver->write_words == NULL) {
        return -1;
    }

    // reset LCD
    driver->reset(driver->user_data);

    // read device code
    uint16_t device_code = 0;
    driver->read_reg(driver->user_data, 0x0000, &device_code);
    device_code = 0x9325;

    if (device_code == 0x9325 || device_code == 0x9328) {
        driver->_model = ILI93XX_LCD_MODEL_ILI9325;
        // test write operation
//        uint16_t test_vals[2] = { 0x0077, 0x0000 };
//        if ((err = ili93xx_lcd_test_write_reg(driver, 0x0020, test_vals))) {
//            return err;
//        }

        driver->write_reg(driver->user_data, 0xE5, 0x78F0); /* set SRAM internal timing */
        driver->write_reg(driver->user_data, 0x01, 0x0100); /* set Driver Output Control */
        driver->write_reg(driver->user_data, 0x02, 0x0700); /* set 1 line inversion */
        driver->write_reg(driver->user_data, 0x03, 0x1030); /* set GRAM write direction and BGR=1 */
        driver->write_reg(driver->user_data, 0x04, 0x0000); /* Resize register */
        driver->write_reg(driver->user_data, 0x08, 0x0202); /* set the back porch and front porch. Note: with an original value 0x0207 an image trembles */
        driver->write_reg(driver->user_data, 0x09, 0x0000); /* set non-display area refresh cycle ISC[3:0] */
        driver->write_reg(driver->user_data, 0x0A, 0x0000); /* FMARK function */
        driver->write_reg(driver->user_data, 0x0C, 0x0000); /* RGB interface setting */
        driver->write_reg(driver->user_data, 0x0D, 0x0000); /* Frame marker Position */
        driver->write_reg(driver->user_data, 0x0F, 0x0000); /* RGB interface polarity */
        /*************Power On sequence ****************/
        driver->write_reg(driver->user_data, 0x10, 0x0000); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
        driver->write_reg(driver->user_data, 0x11, 0x0007); /* DC1[2:0], DC0[2:0], VC[2:0] */
        driver->write_reg(driver->user_data, 0x12, 0x0000); /* VREG1OUT voltage */
        driver->write_reg(driver->user_data, 0x13, 0x0000); /* VDV[4:0] for VCOM amplitude */
        driver->write_reg(driver->user_data, 0x07, 0x0001);
        driver->delay(driver->user_data, 200);
        /* Dis-charge capacitor power voltage */
        driver->write_reg(driver->user_data, 0x10, 0x1090); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
        driver->write_reg(driver->user_data, 0x11, 0x0227); /* Set DC1[2:0], DC0[2:0], VC[2:0] */
        driver->delay(driver->user_data, 50); /* Delay 50ms */
        driver->write_reg(driver->user_data, 0x12, 0x001F);
        driver->delay(driver->user_data, 50); /* Delay 50ms */
        driver->write_reg(driver->user_data, 0x13, 0x1500); /* VDV[4:0] for VCOM amplitude */
        driver->write_reg(driver->user_data, 0x29, 0x0027); /* 04 VCM[5:0] for VCOMH */
        driver->write_reg(driver->user_data, 0x2B, 0x000D); /* Set Frame Rate */
        driver->delay(driver->user_data, 50); /* Delay 50ms */
        driver->write_reg(driver->user_data, 0x20, 0x0000); /* GRAM horizontal Address */
        driver->write_reg(driver->user_data, 0x21, 0x0000); /* GRAM Vertical Address */
        /* ----------- Adjust the Gamma Curve ---------- */
        driver->write_reg(driver->user_data, 0x30, 0x0000);
        driver->write_reg(driver->user_data, 0x31, 0x0707);
        driver->write_reg(driver->user_data, 0x32, 0x0307);
        driver->write_reg(driver->user_data, 0x35, 0x0200);
        driver->write_reg(driver->user_data, 0x36, 0x0008);
        driver->write_reg(driver->user_data, 0x37, 0x0004);
        driver->write_reg(driver->user_data, 0x38, 0x0000);
        driver->write_reg(driver->user_data, 0x39, 0x0707);
        driver->write_reg(driver->user_data, 0x3C, 0x0002);
        driver->write_reg(driver->user_data, 0x3D, 0x1D04);
        /* ------------------ Set GRAM area --------------- */
        driver->write_reg(driver->user_data, 0x50, 0x0000); /* Horizontal GRAM Start Address */
        driver->write_reg(driver->user_data, 0x51, 0x00EF); /* Horizontal GRAM End Address */
        driver->write_reg(driver->user_data, 0x52, 0x0000); /* Vertical GRAM Start Address */
        driver->write_reg(driver->user_data, 0x53, 0x013F); /* Vertical GRAM Start Address */
        driver->write_reg(driver->user_data, 0x60, 0xA700); /* Gate Scan Line */
        driver->write_reg(driver->user_data, 0x61, 0x0001); /* NDL,VLE, REV */
        driver->write_reg(driver->user_data, 0x6A, 0x0000); /* set scrolling line */
        /* -------------- Partial Display Control --------- */
        driver->write_reg(driver->user_data, 0x80, 0x0000);
        driver->write_reg(driver->user_data, 0x81, 0x0000);
        driver->write_reg(driver->user_data, 0x82, 0x0000);
        driver->write_reg(driver->user_data, 0x83, 0x0000);
        driver->write_reg(driver->user_data, 0x84, 0x0000);
        driver->write_reg(driver->user_data, 0x85, 0x0000);
        /* -------------- Panel Control ------------------- */
        driver->write_reg(driver->user_data, 0x90, 0x0010);
        driver->write_reg(driver->user_data, 0x92, 0x0600);
        driver->write_reg(driver->user_data, 0x07, 0x0133); /* 262K color and display ON */
    } else if (device_code == 0x9320 || device_code == 0x9300) {
        driver->_model = ILI93XX_LCD_MODEL_ILI9320;
        // test write operation
        uint16_t test_vals[2] = { 0x0077, 0x0000 };
        if ((err = ili93xx_lcd_test_write_reg(driver, 0x0020, test_vals))) {
            return err;
        }

        driver->write_reg(driver->user_data, 0x00, 0x0000);
        driver->write_reg(driver->user_data, 0x01, 0x0100); /* Driver Output Contral */
        driver->write_reg(driver->user_data, 0x02, 0x0700); /* LCD Driver Waveform Contral */
        driver->write_reg(driver->user_data, 0x03, 0x1018); /* Entry Mode Set */

        driver->write_reg(driver->user_data, 0x04, 0x0000); /* Scalling Contral */
        driver->write_reg(driver->user_data, 0x08, 0x0202); /* Display Contral */
        driver->write_reg(driver->user_data, 0x09, 0x0000); /* Display Contral 3.(0x0000) */
        driver->write_reg(driver->user_data, 0x0a, 0x0000); /* Frame Cycle Contal.(0x0000) */
        driver->write_reg(driver->user_data, 0x0c, (1 << 0)); /* Extern Display Interface Contral */
        driver->write_reg(driver->user_data, 0x0d, 0x0000); /* Frame Maker Position */
        driver->write_reg(driver->user_data, 0x0f, 0x0000); /* Extern Display Interface Contral 2. */

        driver->delay(driver->user_data, 100); /* delay 100 ms */
        driver->write_reg(driver->user_data, 0x07, 0x0101); /* Display Contral */
        driver->delay(driver->user_data, 100); /* delay 100 ms */

        driver->write_reg(driver->user_data, 0x10, (1 << 12) | (0 << 8) | (1 << 7) | (1 << 6) | (0 << 4)); /* Power Control 1.(0x16b0)	*/
        driver->write_reg(driver->user_data, 0x11, 0x0007); /* Power Control 2 */
        driver->write_reg(driver->user_data, 0x12, (1 << 8) | (1 << 4) | (0 << 0)); /* Power Control 3.(0x0138)	*/
        driver->write_reg(driver->user_data, 0x13, 0x0b00); /* Power Control 4 */
        driver->write_reg(driver->user_data, 0x29, 0x0000); /* Power Control 7 */

        driver->write_reg(driver->user_data, 0x2b, (1 << 14) | (1 << 4));

        driver->write_reg(driver->user_data, 0x50, 0); /* Set X Start */
        driver->write_reg(driver->user_data, 0x51, 239); /* Set X End */
        driver->write_reg(driver->user_data, 0x52, 0); /* Set Y Start */
        driver->write_reg(driver->user_data, 0x53, 319); /* Set Y End */

        driver->write_reg(driver->user_data, 0x60, 0x2700); /* Driver Output Control */
        driver->write_reg(driver->user_data, 0x61, 0x0001); /* Driver Output Control */
        driver->write_reg(driver->user_data, 0x6a, 0x0000); /* Vertical Srcoll Control */

        driver->write_reg(driver->user_data, 0x80, 0x0000); /* Display Position? Partial Display 1 */
        driver->write_reg(driver->user_data, 0x81, 0x0000); /* RAM Address Start? Partial Display 1 */
        driver->write_reg(driver->user_data, 0x82, 0x0000); /* RAM Address End-Partial Display 1 */
        driver->write_reg(driver->user_data, 0x83, 0x0000); /* Displsy Position? Partial Display 2 */
        driver->write_reg(driver->user_data, 0x84, 0x0000); /* RAM Address Start? Partial Display 2 */
        driver->write_reg(driver->user_data, 0x85, 0x0000); /* RAM Address End? Partial Display 2 */

        driver->write_reg(driver->user_data, 0x90, (0 << 7) | (16 << 0)); /* Frame Cycle Contral.(0x0013)	*/
        driver->write_reg(driver->user_data, 0x92, 0x0000); /* Panel Interface Contral 2.(0x0000) */
        driver->write_reg(driver->user_data, 0x93, 0x0001); /* Panel Interface Contral 3. */
        driver->write_reg(driver->user_data, 0x95, 0x0110); /* Frame Cycle Contral.(0x0110)	*/
        driver->write_reg(driver->user_data, 0x97, (0 << 8));
        driver->write_reg(driver->user_data, 0x98, 0x0000); /* Frame Cycle Contral */

        driver->write_reg(driver->user_data, 0x07, 0x0173);
    } else if (device_code == 0x9331) {
        driver->_model = ILI93XX_LCD_MODEL_ILI9331;
        driver->write_reg(driver->user_data, 0x00E7, 0x1014);
        driver->write_reg(driver->user_data, 0x0001, 0x0100); /* set SS and SM bit */
        driver->write_reg(driver->user_data, 0x0002, 0x0200); /* set 1 line inversion */
        driver->write_reg(driver->user_data, 0x0003, 0x1030); /* set GRAM write direction and BGR=1 */
        driver->write_reg(driver->user_data, 0x0008, 0x0202); /* set the back porch and front porch */
        driver->write_reg(driver->user_data, 0x0009, 0x0000); /* set non-display area refresh cycle ISC[3:0] */
        driver->write_reg(driver->user_data, 0x000A, 0x0000); /* FMARK function */
        driver->write_reg(driver->user_data, 0x000C, 0x0000); /* RGB interface setting */
        driver->write_reg(driver->user_data, 0x000D, 0x0000); /* Frame marker Position */
        driver->write_reg(driver->user_data, 0x000F, 0x0000); /* RGB interface polarity */
        /* Power On sequence */
        driver->write_reg(driver->user_data, 0x0010, 0x0000); /* SAP, BT[3:0], AP, DSTB, SLP, STB	*/
        driver->write_reg(driver->user_data, 0x0011, 0x0007); /* DC1[2:0], DC0[2:0], VC[2:0] */
        driver->write_reg(driver->user_data, 0x0012, 0x0000); /* VREG1OUT voltage	*/
        driver->write_reg(driver->user_data, 0x0013, 0x0000); /* VDV[4:0] for VCOM amplitude */
        driver->delay(driver->user_data, 200); /* delay 200 ms */
        driver->write_reg(driver->user_data, 0x0010, 0x1690); /* SAP, BT[3:0], AP, DSTB, SLP, STB	*/
        driver->write_reg(driver->user_data, 0x0011, 0x0227); /* DC1[2:0], DC0[2:0], VC[2:0] */
        driver->delay(driver->user_data, 50); /* delay 50 ms */
        driver->write_reg(driver->user_data, 0x0012, 0x000C); /* Internal reference voltage= Vci	*/
        driver->delay(driver->user_data, 50); /* delay 50 ms */
        driver->write_reg(driver->user_data, 0x0013, 0x0800); /* Set VDV[4:0] for VCOM amplitude */
        driver->write_reg(driver->user_data, 0x0029, 0x0011); /* Set VCM[5:0] for VCOMH */
        driver->write_reg(driver->user_data, 0x002B, 0x000B); /* Set Frame Rate */
        driver->delay(driver->user_data, 50); /* delay 50 ms */
        driver->write_reg(driver->user_data, 0x0020, 0x0000); /* GRAM horizontal Address */
        driver->write_reg(driver->user_data, 0x0021, 0x0000); /* GRAM Vertical Address */
        /* Adjust the Gamma Curve */
        driver->write_reg(driver->user_data, 0x0030, 0x0000);
        driver->write_reg(driver->user_data, 0x0031, 0x0106);
        driver->write_reg(driver->user_data, 0x0032, 0x0000);
        driver->write_reg(driver->user_data, 0x0035, 0x0204);
        driver->write_reg(driver->user_data, 0x0036, 0x160A);
        driver->write_reg(driver->user_data, 0x0037, 0x0707);
        driver->write_reg(driver->user_data, 0x0038, 0x0106);
        driver->write_reg(driver->user_data, 0x0039, 0x0707);
        driver->write_reg(driver->user_data, 0x003C, 0x0402);
        driver->write_reg(driver->user_data, 0x003D, 0x0C0F);
        /* Set GRAM area */
        driver->write_reg(driver->user_data, 0x0050, 0x0000); /* Horizontal GRAM Start Address */
        driver->write_reg(driver->user_data, 0x0051, 0x00EF); /* Horizontal GRAM End Address */
        driver->write_reg(driver->user_data, 0x0052, 0x0000); /* Vertical GRAM Start Address */
        driver->write_reg(driver->user_data, 0x0053, 0x013F); /* Vertical GRAM Start Address */
        driver->write_reg(driver->user_data, 0x0060, 0x2700); /* Gate Scan Line */
        driver->write_reg(driver->user_data, 0x0061, 0x0001); /*  NDL,VLE, REV */
        driver->write_reg(driver->user_data, 0x006A, 0x0000); /* set scrolling line */
        /* Partial Display Control */
        driver->write_reg(driver->user_data, 0x0080, 0x0000);
        driver->write_reg(driver->user_data, 0x0081, 0x0000);
        driver->write_reg(driver->user_data, 0x0082, 0x0000);
        driver->write_reg(driver->user_data, 0x0083, 0x0000);
        driver->write_reg(driver->user_data, 0x0084, 0x0000);
        driver->write_reg(driver->user_data, 0x0085, 0x0000);
        /* Panel Control */
        driver->write_reg(driver->user_data, 0x0090, 0x0010);
        driver->write_reg(driver->user_data, 0x0092, 0x0600);
        driver->write_reg(driver->user_data, 0x0007, 0x0021);
        driver->delay(driver->user_data, 50); /* delay 50 ms */
        driver->write_reg(driver->user_data, 0x0007, 0x0061);
        driver->delay(driver->user_data, 50); /* delay 50 ms */
        driver->write_reg(driver->user_data, 0x0007, 0x0133); /* 262K color and display ON */
    } else {
        return -3;
    }

    driver->_is_initialized = true;

    return 0;
}

int lcd_ili93xx_get_height(lcd_ili93xx_driver_t *driver, int16_t *height) {
    *height = driver->_height;
    return 0;
}

int lcd_ili93xx_get_width(lcd_ili93xx_driver_t *driver, int16_t *width) {
    *width = driver->_width;
    return 0;
}

static int lcd_ili93xx_prepare_area(lcd_ili93xx_driver_t *driver, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    // set window
    if (x1 > x2) {
        int16_t x_tmp = x1;
        x1 = x2;
        x2 = x_tmp;
    }
    if (y1 > y2) {
        int16_t y_tmp = y1;
        y1 = y2;
        y2 = y_tmp;
    }
    // set gram coordinates
    driver->write_reg(driver->user_data, ILI93XX_LCD_REGISTER_GRAM_X_ADDR, x1);
    driver->write_reg(driver->user_data, ILI93XX_LCD_REGISTER_GRAM_Y_ADDR, y1);
    driver->write_reg(driver->user_data, ILI93XX_LCD_REGISTER_GRAM_X_ADDR_START, x1);
    driver->write_reg(driver->user_data, ILI93XX_LCD_REGISTER_GRAM_X_ADDR_END, x2);
    driver->write_reg(driver->user_data, ILI93XX_LCD_REGISTER_GRAM_Y_ADDR_START, y1);
    driver->write_reg(driver->user_data, ILI93XX_LCD_REGISTER_GRAM_Y_ADDR_END, y2);

    return (x2 - x1 + 1) * (y2 - y1 + 1);
}

int lcd_ili93xx_fill_area_color(lcd_ili93xx_driver_t *driver, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    // prepare area to draw
    int total_pixes = lcd_ili93xx_prepare_area(driver, x1, y1, x2, y2);

    const size_t max_buff_size = 128;
    size_t buff_size = total_pixes < max_buff_size ? total_pixes : max_buff_size;
    uint16_t buff[buff_size];
    for (size_t i = 0; i < buff_size; i++) {
        buff[i] = color;
    }

    size_t i = total_pixes;
    while (i > 0) {
        size_t pixels_to_update = i >= buff_size ? buff_size : i;
        // write pixels
        driver->write_words(driver->user_data, ILI93XX_LCD_REGISTER_GRAM_WRITE_DATA, buff, pixels_to_update);
        i -= pixels_to_update;
    }
    return 0;
}

int lcd_ili93xx_fill_area(lcd_ili93xx_driver_t *driver, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t *colors_data) {
    // prepare area to draw
    int total_pixes = lcd_ili93xx_prepare_area(driver, x1, y1, x2, y2);
    // flush pixels
    driver->write_words(driver->user_data, ILI93XX_LCD_REGISTER_GRAM_WRITE_DATA, colors_data, total_pixes);

    return 0;
}
