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

    // sprites
    const uint8_t playerShip[4][8] = {
        {0x89, 0xD3, 0xFF, 0xFF, 0x7E, 0x5A, 0x66, 0x3C},
        {0x91, 0xD3, 0xFF, 0xFF, 0x7E, 0x5A, 0x66, 0x3C},
        {0x91, 0xCB, 0xFF, 0xFF, 0x7E, 0x5A, 0x66, 0x3C},
        {0x89, 0xCB, 0xFF, 0xFF, 0x7E, 0x5A, 0x66, 0x3C}
        };
    const uint8_t playerLaser[4][8] = {
        {0x00, 0x08, 0x18, 0x10, 0x18, 0x18, 0x18, 0x18},
        {0x08, 0x18, 0x10, 0x10, 0x18, 0x18, 0x18, 0x18},
        {0x18, 0x10, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18},
        {0x10, 0x00, 0x08, 0x18, 0x18, 0x18, 0x18, 0x18}
        };

    uint16_t keyFrame = 0;  // which sprite frame to draw
    int16_t y1 = 0,         // y position of ship 1
            y2 = 0,         // y position of ship 2
            s2 = 0;         // y speed of ship 2

    // game loop
    while(1) {
        display_frameStart();   //
        // playerController() version 1: speed based
        if (readButton(BTN_UP))         y1 += 2;
        else if (readButton(BTN_DOWN))  y1 -= 2;
        if (y1 > 24)        y1 = 24;
        else if (y1 < 0)    y1 = 0;
        display_drawSprite(8, y1, playerShip[keyFrame]);

        //playerController() version 2: acceleration based
        if (readButton(BTN_UP))         s2++;   // increase speed upward
        else if (readButton(BTN_DOWN))  s2--;   // increase speed downward
        else {                                  // decrease speed (friction)
            if (s2 > 0)         s2--;
            else if (s2 < 0)    s2++;
        }
        if (s2 > 3)         s2 = 3;     // limit max speed
        else if(s2 < -3)    s2 = -3;    // (terminal velocity)
        y2 += s2;                       // update position
        if (y2 > 24)        y2 = 24;    // wrap position
        else if (y2 < 0)    y2 = 0;     // wrap position
        display_drawSprite(32, y2, playerShip[keyFrame]);

        // update which frame to draw
        keyFrame++;
        keyFrame &= 0x03;

        // render the screen
        display_drawFrame();

        // SHOOT button = red LED
        if(readButton(BTN_SHOOT))   P1OUT |=  BIT0;
        else                        P1OUT &= ~BIT0;

        // sleep until next frame
        timer_sleep();
    }


}



