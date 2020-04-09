/*
 * SSD1306.c
 *
 *  Created on: April 8, 2020
 *      Author: Michael Bolt
 */

#include "SSD1306.h"



/*
 * Initialize the SSD1306 display module
 *
 * \return 0 if I2C communication was successful, otherwise the number
 *         of bytes left in the I2C transmission buffer when the
 *         exchange failed will be returned
 */
uint16_t ssd1306_init(void){
    const uint8_t instructions[] = {
        SSD1306_CMD_START,              // start commands
        SSD1306_SETDISPLAY_OFF,         // turn off display
        SSD1306_SETDISPLAYCLOCKDIV,     // set clock:
        0x80,                           //   Fosc = 8, divide ratio = 0+1
        SSD1306_SETMULTIPLEX,           // display multiplexer:
        (SSD1306_ROWS - 1),             //   number of display rows
        SSD1306_VERTICALOFFSET,         // display vertical offset:
        0,                              //   no offset
        SSD1306_SETSTARTLINE | 0x00,    // RAM start line 0
        SSD1306_SETCHARGEPUMP,          // charge pump:
        0x14,                           //   charge pump ON (0x10 for OFF)
        SSD1306_SETADDRESSMODE,         // addressing mode:
        0x00,                           //   horizontal mode
        SSD1306_COLSCAN_DESCENDING,     // flip columns
        SSD1306_COMSCAN_ASCENDING,      // don't flip rows (pages)
        SSD1306_SETCOMPINS,             // set COM pins
        0x02,                           //   sequential pin mode
        SSD1306_SETCONTRAST,            // set contrast
        0x00,                           //   minimal contrast
        SSD1306_SETPRECHARGE,           // set precharge period
        0xF1,                           //   phase1 = 15, phase2 = 1
        SSD1306_SETVCOMLEVEL,           // set VCOMH deselect level
        0x40,                           //   ????? (0,2,3)
        SSD1306_ENTIREDISPLAY_OFF,      // use RAM contents for display
        SSD1306_SETINVERT_OFF,          // no inversion
        SSD1306_SCROLL_DEACTIVATE,      // no scrolling
        SSD1306_SETDISPLAY_ON,          // turn on display (normal mode)
    };
    // send list of commands
    return i2c_tx(SSD1306_I2C_ADDRESS, instructions, sizeof instructions);
}



/*
 * ! Draw a single pixel to the display
 * !
 * ! \param x: x coordinate of pixel to write to [0:SSD1306_COLUMNS-1]
 * ! \param y: y coordinate of pixel to write to [0:SSD1306_ROWS-1]
 * ! \param value: value to write to the pixel [0:1]
 * !
 * ! \return 0 if successful, error code if failed:
 * !         1: x value out of range
 * !         2: y value out of range
 * !         3: I2C error during configuration
 * !         4: I2C error during data transmission
 */
uint16_t ssd1306_drawPixel(uint16_t x, uint16_t y, uint8_t value){
    // ensure pixel location is valid
    if (x >= SSD1306_COLUMNS)   return 1;
    if (y >= SSD1306_ROWS)      return 2;
    // send configuration message
    uint8_t configMsg[] = {
        SSD1306_CMD_START,           // start commands
        SSD1306_SETPAGERANGE,        // set page range:
        y >> 3,                      //   y / 8
        y >> 3,                      //   y / 8
        SSD1306_SETCOLRANGE,         // set column range:
        x,                           //   x
        x                            //   x
    };
    if (i2c_tx(SSD1306_I2C_ADDRESS, configMsg, sizeof configMsg))   return 3;
    // send pixel to be drawn
    uint8_t dataMsg[] = {
        SSD1306_DATA_START,         // start data
        value << (y & 0x07)         //   y % 8
    };
    if (i2c_tx(SSD1306_I2C_ADDRESS, dataMsg, sizeof dataMsg))       return 4;
    return 0;
}
