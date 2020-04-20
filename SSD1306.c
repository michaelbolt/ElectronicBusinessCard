/*
 * SSD1306.c
 *
 *  Created on: April 8, 2020
 *      Author: Michael Bolt
 */

#include "SSD1306.h"


/**************************************************************
 *  Global Variables  *****************************************
 **************************************************************/
static uint8_t  ssd1306_vram[SSD1306_ROWS / 8][SSD1306_COLUMNS] = {0};



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
    const uint8_t page = y >> 3;
    const uint8_t configMsg[] = {
        SSD1306_CMD_START,          // start commands
        SSD1306_SETPAGERANGE,       // set page range:
        page,                       //   y / 8
        page,                       //   y / 8
        SSD1306_SETCOLRANGE,        // set column range:
        x,                          //   x
        x                           //   x
    };
    if (i2c_tx(SSD1306_I2C_ADDRESS, configMsg, sizeof configMsg))   return 3;

    // draw pixel to VRAM
    if(value)   ssd1306_vram[page][x] |=   0x01 << (y & 0x07);
    else        ssd1306_vram[page][x] &= ~(0x01 << (y & 0x07));

    // draw updated VRAM page to screen
    const uint8_t dataMsg[] = {
        SSD1306_DATA_START,         // start data
        ssd1306_vram[page][x]       //   VRAM page
    };
    if (i2c_tx(SSD1306_I2C_ADDRESS, dataMsg, sizeof dataMsg))       return 4;

    // return successful
    return 0;
}



/*
 * ! Draw an 8x8 sprite at location (x,y)
 * !
 * ! \param x: x coordinate of bottom left pixel of sprite [0:SSD1306_COLUMNS-1]
 * ! \param y: y coordinate of bottom left pixel of sprite [0:SSD1306_ROWS-1]
 * ! \param *sprite: pointer to 8x8 sprite data (const uint8_t[8])
 * !
 * ! \return 0: successful, else error:
 * !         1: x value out of range
 * !         2: y value out of range
 * !         3: I2C error during configuration
 * !         4: I2C error during data transmission
 * !         5: dirty rectangle animation buffer was full
 * !
 * ! Draws an 8x8 sprite to on-chip VRAM, then the updated VRAM region to SSD1306
 */
uint16_t ssd1306_drawSprite(uint16_t x, uint16_t y, const uint8_t *const sprite) {
    // ensure pixel location is valid
    if (x >= SSD1306_COLUMNS)   return 1;
    if (y >= SSD1306_ROWS)      return 2;

    // add (x,y) coordinate to dirty rectangle animation buffer
    if (dirtyRect_write(x, y))  return 5;

    // determine column range: [x:x+7]
    uint8_t colStop = x + 7;
    if (colStop >= SSD1306_COLUMNS) {
        colStop = SSD1306_COLUMNS - 1;
    }

    // determine page range
    uint8_t pageStart = y >> 3, // y / 8 = starting page
            pageStop = y >> 3;  // y / 8 = stopping page, unless..
    if (y & 0x07)               // if y is not an integer multiple of 8,
        pageStop++;             //   two pages must be updated

    // update VRAM
    unsigned int i = colStop - x + 1,       // counter for iterating
                 pageOffset = y & 0x07;     // offset from bottom of page
    while (i!=0) {
        i--;                                              // decrement counter
        uint8_t lowerPage = sprite[i] << pageOffset;      // move sprite 'up'
        ssd1306_vram[pageStart][x+i] |= lowerPage;        // OR into VRAM
        if (pageStop < (SSD1306_ROWS / 8)) {                    // only update second page if valid
            uint8_t upperPage = sprite[i] >> (8-pageOffset);    // move sprite 'down'
            ssd1306_vram[pageStop][x+i] |= upperPage;           // OR into VRAM
        }
    }

    // send configuration message
    const uint8_t configMsg[] = {
        SSD1306_CMD_START,          // start commands
        SSD1306_SETPAGERANGE,       // set page range:
        pageStart,                  //   y / 8
        pageStop,                   //
        SSD1306_SETCOLRANGE,        // set column range:
        x,                          //   x
        colStop                     //   min(x+7, 127)
    };
    if (i2c_tx(SSD1306_I2C_ADDRESS, configMsg, sizeof configMsg))   return 3;

    // draw updated VRAM to screen
    uint8_t dataMsg[9] = {  // message can be a max of 9 bytes
        SSD1306_DATA_START  // start data
    };
    i = pageStart;
    while (i != pageStop + 1) {                 // loop over pages
        if (i < SSD1306_ROWS / 8) {                 // only if valid page
            unsigned int j = colStop - x + 1;           // local counter to
            while (j != 0) {                            // copy VRAM into dataMsg
                j--;
                dataMsg[j+1] = ssd1306_vram[i][x+j];
            }
            if (i2c_tx(SSD1306_I2C_ADDRESS, dataMsg, colStop-x+2))  return 4;
        }
        i++;
    }

    // return successful
    return 0;
}


/*****************************
 * Dirty Rectangle Animation *
 *****************************/
static DirtyRectangleBuffer dirtyRectBuff;

/*
 * ! write an (x,y) coordinate pair to the Dirty Rectangle buffer
 * !
 * ! \param x: x coordinate to write to buffer
 * ! \param y: y coordinate to write to buffer
 * !
 * ! \return 0 if successful, 1 if buffer was already full
 */
uint16_t dirtyRect_write(uint8_t x, uint8_t y) {
    // check if buffer is full
    if (dirtyRectBuff.length == (1 << DIRTY_RECT_BUFFER_POWER))
        return 1;
    // else write values, increment writeIndex & length
    dirtyRectBuff.coordinates[0][dirtyRectBuff.writeIndex] = x;
    dirtyRectBuff.coordinates[1][dirtyRectBuff.writeIndex] = y;
    dirtyRectBuff.writeIndex = (dirtyRectBuff.writeIndex + 1) & ((1 << DIRTY_RECT_BUFFER_POWER) - 1);
    dirtyRectBuff.length++;
    return 0;
}

/*
 * ! read an (x,y) coordinate pair from the Dirty Rectangle buffer
 * !
 * ! \param *x: reference to x value to store Dirty Rectangle buffer value
 * ! \param *y: reference to y value to store Dirty Rectangle buffer value
 * !
 * ! \return 0 if successful, 1 if buffer was already full
 */
uint16_t dirtyRect_read(uint8_t *x, uint8_t *y) {
    // check if buffer is empty
    if (!dirtyRectBuff.length)
        return 1;
    //else read values, increment readIndex, decrement length
    *x = dirtyRectBuff.coordinates[0][dirtyRectBuff.readIndex];
    *y = dirtyRectBuff.coordinates[1][dirtyRectBuff.readIndex];
    dirtyRectBuff.readIndex = (dirtyRectBuff.readIndex + 1) & ((1 << DIRTY_RECT_BUFFER_POWER) - 1);
    dirtyRectBuff.length--;
    return 0;
}

/*
 * clear all (x,y) sprite locations in VRAM that are listed in the Dirty Rectangle Buffer
 */
void dirtyRect_clearLastFrame(void) {
    // iterate over all (x,y) coordinate in buffer
    uint8_t x=0,
            y=0;
    while(!dirtyRect_read(&x, &y)) {
        // determine column range
        uint8_t colStop = x + 7;
        if (colStop >= SSD1306_COLUMNS) {
            colStop = SSD1306_COLUMNS - 1;
        }
        // determine page range
        uint8_t pageStart = y >> 3, // y / 8 = starting page
                pageStop = y >> 3;  // y / 8 = stopping page, unless..
        if (y & 0x07)               // if y is not an integer multiple of 8,
            pageStop++;             //   two pages must be updated
        // update VRAM
        unsigned int i = colStop - x + 1,       // counter for iterating
                     pageOffset = y & 0x07;     // offset from bottom of page
        while (i!=0) {
            i--;                                        // decrement counter
            uint8_t lowerPage = 0xFF << pageOffset;     // move box 'up'
            ssd1306_vram[pageStart][x+i] &= ~lowerPage; // clear from VRAM
            if (pageStop < (SSD1306_ROWS / 8)) {            // only update second page if valid
                uint8_t upperPage = 0xFF >> (8-pageOffset); // move box 'down'
                ssd1306_vram[pageStop][x+i] &= ~upperPage;  // clear from VRAM
            }
        }

    }
}












