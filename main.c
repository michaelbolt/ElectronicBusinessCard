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

    // small sanity test of SSD1306
    i2c_init();                                 // initialize I2C interface
    volatile uint16_t ret = 100;                // value to capture returns
    ret = ssd1306_init();                       // initialize SSD1306 chip
    __delay_cycles(10000);                      // spin for a bit
    // clear display
    unsigned int x=0, y=0;
    for(x=0; x < SSD1306_COLUMNS; x++) {
        for(y=0; y < SSD1306_ROWS; y+= 8) {
            ssd1306_drawPixel(x, y, 0);
        }
    }

    // draw ship sprites in a few places
    const uint8_t ship[4][8] = {
        {0x14, 0x24, 0x3C, 0xFF, 0xFF, 0xFF, 0xA5, 0x99},
        {0x24, 0x28, 0x3C, 0xFF, 0xFF, 0xFF, 0xA5, 0x99},
        {0x28, 0x18, 0x3C, 0xFF, 0xFF, 0xFF, 0xA5, 0x99},
        {0x18, 0x14, 0x3C, 0xFF, 0xFF, 0xFF, 0xA5, 0x99}
        };
    unsigned int frame = 0;
    x = 10;
    y = 10;



    // blink red LED to show we are done
    P1DIR |= 0x01;                          // Set P1.0 to output direction
    for(;;) {
        volatile unsigned int i;            // volatile to prevent optimization
        P1OUT ^= 0x01;                      // Toggle P1.0 using exclusive-OR
        i = 10000;                          // SW Delay
        do i--;
        while(i != 0);
        // test spaceship animation
        dirtyRect_clearLastFrame();
        ssd1306_drawSprite(x, y, ship[frame]);
        frame++;
        frame &= 0x03;
        x += 2;
        x &= 127;
    }
}
