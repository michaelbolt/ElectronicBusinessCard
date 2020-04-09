//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  Texas Instruments, Inc
//  July 2013
//***************************************************************************************

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
    uint16_t x,y;                               // iterate over every pixel
    for(x=0; x < SSD1306_COLUMNS; x++){
        for(y=0; y <SSD1306_ROWS; y++) {
            ret = ssd1306_drawPixel(x, y, 1);   // fill every pixel
            __delay_cycles(10000);              // slow down so we can see the drawing
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
