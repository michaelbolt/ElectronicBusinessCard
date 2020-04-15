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
            __delay_cycles(5000);
        }
    }

    // draw a pattern on the left half
    for(x=0; x < SSD1306_COLUMNS/2; x += 2) {
        for(y = x&0x07; y < SSD1306_ROWS; y += 3) {
            ret = ssd1306_drawPixel(x, y, 1);
            __delay_cycles(5000);
        }
    }

    //draw a triangle
    for(x=70; x < 83; x++) {
        y = 25 + (x - 70);
        ret = ssd1306_drawPixel(x, y, 1);
        y = 31 - (x - 76);
        ret = ssd1306_drawPixel(x, y, 1);
        ret = ssd1306_drawPixel(x, 25, 1);
        __delay_cycles(15000);
    }

    // draw a circle
    unsigned int circle_x[] = {76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88};
    unsigned int circle_y1[] = {10, 13, 14, 15, 15, 15, 15, 15, 14, 13, 10};
    unsigned int circle_y2[] = {10,  7,  6,  5,  5,  5,  5,  5,  6,  7, 10};
    for(x=0; x < 11; x++) {
        ret = ssd1306_drawPixel(circle_x[x], circle_y1[x], 1);
        ret = ssd1306_drawPixel(circle_x[x], circle_y2[x], 1);
        __delay_cycles(10000);
    }

    // draw a smiley face
    unsigned int eye1[2][5] = {
        {100, 100, 100, 100, 100},
        {16, 17, 18, 19, 20}
    };
    unsigned int eye2[2][5] = {
        {110, 110, 110, 110, 110},
        {16, 17, 18, 19, 20}
    };
    unsigned int mouth[2][13] = {
        {100, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 110},
        {12, 11, 9, 9, 9, 9, 9, 9, 9, 9, 10, 11, 12}
    };
    for(x = 0; x < 5; x++) {
        ret = ssd1306_drawPixel(eye1[0][x], eye1[1][x], 1);
        ret = ssd1306_drawPixel(eye2[0][x], eye2[1][x], 1);
        __delay_cycles(10000);
    }
    for(x = 0; x < 13; x++) {
        ret = ssd1306_drawPixel(mouth[0][x], mouth[1][x], 1);
        __delay_cycles(10000);
    }


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
