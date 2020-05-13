/*
 * main.c
 *
 *  Created on: April 9, 2020
 *      Author: Michael Bolt
 */

#include <msp430.h>
#include <stdint.h>
#include "hardware.h"
#include "i2c.h"
#include "ssd1306.h"


// main
void main(void) {

    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    timer_init();
    buttons_init();
    i2c_init();
    ssd1306_init();
    // configure P1.[0,1] as output for LED
    P1DIR |=   BIT0 | BIT1;
    P1OUT &= ~(BIT0 | BIT1);

    // enable interrupts
    __bis_SR_register(GIE);

    // sprite to draw for shoot button
    const uint8_t arrow[8] = {0x18,0x18,0x18,0x18,0x99,0x5A,0x3C,0x18};
    uint16_t x = 0;

    // game loop
    while(1) {
        // UP button = red LED
        if(readButton(BTN_UP))      P1OUT |=  BIT0;
        else                        P1OUT &= ~BIT0;
        // DOWN button = green LED
        if(readButton(BTN_DOWN))    P1OUT |=  BIT1;
        else                        P1OUT &= ~BIT1;
        // SHOOT button = move arrow sprite to the right
        if(readButton(BTN_SHOOT)) {
            display_frameStart();
            display_drawSprite(x, 12, arrow);
            display_drawFrame();
            x++;
            x &= 127;
        }
        // sleep until next frame
        timer_sleep();
    }


}



