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

    // fill in the screen
    uint16_t x,y;
    for(x=0; x < SSD1306_COLUMNS; x++){
        for(y=0; y <SSD1306_ROWS; y++) {
            ret = ssd1306_drawPixel(x, y, 1);
            __delay_cycles(5000);
        }
    }

    // clear every somethingth pixel
    for(x=1; x < SSD1306_COLUMNS; x += 2) {
        for(y = x&0x07; y < SSD1306_ROWS; y += 3) {
            ret = ssd1306_drawPixel(x, y, 0);
            __delay_cycles(5000);
        }
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
