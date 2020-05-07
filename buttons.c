/*
 * buttons.c
 *
 *  Created on: May 4, 2020
 *      Author: Michael Bolt
 */

#include "buttons.h"


// global variable to indicate if button is pressed
volatile uint16_t btnState = 0;

/*
 * ! Initialize buttons and debouncing interrupts
 * !
 * ! Configures P2.[0:2] as pull-up resistor inputs
 * ! Configures 5 ms ISR for TimerA0
 */
void buttons_init(void) {
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
}


/*
 * ! Read a button
 * !
 * ! \param button: which button to read (BTN_UP, BTN_DOWN, BTN_SHOOT)
 * !
 * ! Returns the current debounced state of the chosen button
 */
inline uint16_t readButton(uint16_t button) {
    return btnState & button;
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
    btnState = 0x00FF ^ temp;
}
