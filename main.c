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
#include <math.h>
#include <stdio.h>

// global variable to indicate if button is pressed
volatile uint8_t btnState = 0;

// main
void main(void) {

    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    // i2c init
    i2c_init();
    ssd1306_init();
    // configure P1.0 as output for LED
    P1DIR |=  BIT0 | BIT1;
    P1OUT &= ~(BIT0 | BIT1);
    // configure P2.[0:2] for a pushbutton input
    P2DIR &=  ~(BIT0 | BIT1 | BIT2); // input
    P2REN |=   (BIT0 | BIT1 | BIT2); // enable pull-up/down resistor
    P2OUT |=   (BIT0 | BIT1 | BIT2); // set to pull-up
    // enable 5 ms timer interrupt with timerA module
    TA0CTL   |= TACLR;        // clear timer logic
    TA0CTL   |= TASSEL__ACLK; // source = ACLK (32 kHz)
    TA0CCR0   = 164;          // period = 164 / 2^15 = 5 ms
    TA0CCTL0 |= CCIE;         // enable CCR0 interrupt
    TA0CTL   |= MC__UP;       // mode = UP

    // enable interrupts
    __bis_SR_register(GIE);

    const uint8_t arrow[8] = {0x18,0x18,0x18,0x18,0x99,0x5A,0x3C,0x18};
    uint16_t x = 0;

    while(1) {
        // UP button = red LED
        if(btnState & BIT2) P1OUT |=  BIT0;
        else                P1OUT &= ~BIT0;
        // DOWN button = green LED
        if(btnState & BIT1) P1OUT |=  BIT1;
        else                P1OUT &= ~BIT1;
        // SHOOT button = sprite!
        if(btnState & BIT0) {
            display_frameStart();
            display_drawSprite(x, 12, arrow);
            display_drawFrame();
            x++;
            x &= 127;
        }
        __delay_cycles(10000);
    }
}

//******************************************************************************
// Timer A0 CCR0 ISR ***********************************************************
//******************************************************************************
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    // static state buffer and index retain values between interrupts
    static uint8_t state[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    static uint16_t index = 0;
    // add pushbutton samples to state buffer
    state[index++] = P2IN;
    // wrap index value to length of state buffer (modulo 8)
    index &= 0x07;
    // OR entire state buffer to check for 1's
    uint16_t i = 0;
    uint8_t temp = 0;
    for(i = 0; i < 8; temp |= state[i++]);
    // store stable button presses as 1's
    btnState = 0xFF ^ temp;
    // breakpoint here to check operation
    __no_operation();
}


