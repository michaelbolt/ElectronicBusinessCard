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
#include "game.h"


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

    // clear screen
    {
        uint16_t r = 0,
                 c = 0;
        const uint8_t blank[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        for (r = 0; r < SSD1306_COLUMNS; r += 8) {
            display_frameStart();
            for (c = 0; c < SSD1306_ROWS; c += 8) {
                display_drawSprite(r, c, blank);
            }
            display_drawFrame();
        }
    }

    gameInit();
    // game loop
    while(1) {
        display_frameStart();

        playerController();
        drawPlayer();

        // render the screen
        display_drawFrame();

        // SHOOT button = red LED
        if(readButton(BTN_SHOOT))   P1OUT |=  BIT0;
        else                        P1OUT &= ~BIT0;

        // sleep until next frame
        timer_sleep();
    }


}



