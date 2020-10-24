/*
 * hardware.c
 *
 *  Created on: May 4, 2020
 *      Author: Michael Bolt
 */

#include "hardware.h"

//*****************************************************************************
// Clock System Functions *****************************************************
//*****************************************************************************

/*
 * ! Set clock frequency to 16 MHz
 * ! Sets DCO (MCLK, SMCLK) to 16.777216 MHz (2^24)
 */
void clockSytem_init(void) {
    // Configure one FRAM wait state as required by the device datasheet for
    // operation beyond 8 MHz before configuring the Clock System
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Clock System setup
    __bis_SR_register(SCG0);                    // disable FLL
    CSCTL3 |= SELREF__REFOCLK;                  // set REFO as FLL reference source
    CSCTL0 = 0;                                 // clear DCO and MOD registers
    CSCTL1 &= !(DCORSEL_7);                     // clear DCO frequency select bits first
    // Follow TI's example for 16 MHz
    CSCTL1 |= DCORSEL_5;                        // set DCO = 16 MHz
    CSCTL2 = FLLD_0 + 487;                      // DCOCLKDIV = 16 MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                    // enable FLL
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));  // wait for FLL lock
}


//******************************************************************************
// Timer Functions *************************************************************
//******************************************************************************

/*
 * ! Initialize 16 Hz periodic interrupt
 * !
 * ! Configure Timer A0 CCR0 for 16 Hz interrupt
 */
void timer_init(void) {
    // enable 16 Hz timer interrupt with timerA CCR0 module
    TA0CTL   |= TACLR;        // clear timer logic
    TA0CTL   |= TASSEL__ACLK; // source = ACLK (32 kHz)
    TA0CCR0   = 2047;         // period = ((1/16) * 2^15) - 1 = 2047
    TA0CCTL0 |= CCIE;         // enable CCR0 interrupt
    TA0CTL   |= MC__UP;       // mode = UP
}


/*
 * ! Enter low power sleep mode until start of next frame
 */
inline void timer_sleep(void) {
    __bis_SR_register(GIE + LPM3_bits);
}


//************************
// Timer A0 CCR0 ISR *****
//************************
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    // return from LPM3, leave interrupts enabled
    __bic_SR_register_on_exit(LPM3_bits);
}



//******************************************************************************
// Button Functions ************************************************************
//******************************************************************************

// global variable to indicate if button is pressed
volatile uint16_t btnState = 0;


/*
 * ! Initialize buttons and debouncing interrupt
 * !
 * ! Configures Port2 buttons as pull-up resistor inputs
 * ! Configures 2.5 ms ISR for TimerA0 CCR1
 */
void buttons_init(void) {
    // configure Port2 pins for pushbutton inputs
    P2DIR &=  ~(BTN_UP | BTN_DOWN | BTN_SHOOT);     // input
    P2REN |=   (BTN_UP | BTN_DOWN | BTN_SHOOT);     // enable pull-up/down resistor
    P2OUT |=   (BTN_UP | BTN_DOWN | BTN_SHOOT);     // set to pull-up
    // configure 5 ms timer interrupt with timerA CCR1
    TA0CTL &= ~(MC_3);              // stop timer if running
    TA0CCR1 = (TA0R + 82) & 0x7FF;  // 2.5ms later (modulo 2048)
    TA0CCTL1 = CCIE;                // enable CCR1 interrupt and clear IFG
    TA0CTL |= MC__UP;               // return to UP mode
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


//************************
// Timer A0 CCRx ISR *****
//************************
#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A0_CCRx(void) {
    switch(__even_in_range(TA0IV, 0x1F))
    {
    case TA0IV_NONE:    break;  // Vector 0: None
    case TA0IV_TA0CCR1:         // Vector 2: CCR1 IFG - debouncing
        {
            // update TA0CCR1 to interrupt 2.5 ms later (modulo timer length)
            TA0CCR1 = (TA0R + 82) & 0x7FF;
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
        break;
    case TA0IV_TA0CCR2: break;  // Vector 4: CCR2 IFG
    case TA0IV_3:       break;  // Vector 6: reserved
    case TA0IV_4:       break;  // Vector 8: reserved
    case TA0IV_5:       break;  // Vector 10: reserved
    case TA0IV_6:       break;  // Vector 12: reserved
    case TA0IV_TA0IFG:  break;  // Vector 14: Timer reset IFG
    default:
        break;
    }
}


