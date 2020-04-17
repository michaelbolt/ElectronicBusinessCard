/*
 * main.c
 *
 *  Created on: April 9, 2020
 *      Author: Michael Bolt
 */

#include <msp430.h>
#include <stdint.h>
#include "i2c.h"
#include "ssd1306.h"

void main(void) {

    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    // small sanity test of SSD1306 initialization & pixel drawing
    i2c_init();                                 // initialize I2C interface
    volatile uint16_t ret = 100;                // value to capture returns
    ret = ssd1306_init();                       // initialize SSD1306 chip
    __delay_cycles(10000);                      // spin for a bit

    // clear in the screen
    uint16_t x,y;
    for(x=0; x < SSD1306_COLUMNS; x++){
        for(y=0; y <SSD1306_ROWS; y+= 8) {
            ret = ssd1306_drawPixel(x, y, 0);
        }
    }

    // draw checkerboard sprites in a few places to test
    const uint8_t checkerBoard[8] = {
        0xFF, 0xAB, 0xD5, 0xAB, 0xD5, 0xAB, 0xD5, 0xFF,
    };
    // first column
    ret = ssd1306_drawSprite(8, 0, checkerBoard);
    ret = ssd1306_drawSprite(0, 8, checkerBoard);
    ret = ssd1306_drawSprite(8, 16, checkerBoard);
    ret = ssd1306_drawSprite(0, 24, checkerBoard);
    // offset in y
    ret = ssd1306_drawSprite(24, 4, checkerBoard);
    ret = ssd1306_drawSprite(32, 12, checkerBoard);
    ret = ssd1306_drawSprite(24, 20, checkerBoard);
    // offset in x
    ret = ssd1306_drawSprite(52, 0, checkerBoard);
    ret = ssd1306_drawSprite(60, 8, checkerBoard);
    ret = ssd1306_drawSprite(52, 16, checkerBoard);
    ret = ssd1306_drawSprite(60, 24, checkerBoard);
    // off edge y
    ret = ssd1306_drawSprite(96, 28, checkerBoard);
    // off edge x
    ret = ssd1306_drawSprite(124, 0, checkerBoard);
    // off edge x & y
    ret = ssd1306_drawSprite(124, 28, checkerBoard);

    // blink red LED to show we are done
    P1DIR |= 0x01;                          // Set P1.0 to output direction
    for(;;) {
        volatile unsigned int i;            // volatile to prevent optimization
        P1OUT ^= 0x01;                      // Toggle P1.0 using exclusive-OR
        i = 10000;                          // SW Delay
        do i--;
        while(i != 0);
    }
}
