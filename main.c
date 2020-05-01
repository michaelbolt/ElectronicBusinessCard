/*
 * main.c
 *
 *  Created on: April 9, 2020
 *      Author: Michael Bolt
 */

#include <msp430.h>

volatile unsigned int btnCount = 0;

void main(void) {

    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    // configure P1.0 as output for LED
    P1DIR |=  BIT0;
    P1OUT &= ~BIT0;
    // configure P2.0 for a pushbutton interrupt
    P2DIR &=  BIT0; // input
    P2REN |=  BIT0; // enable pull-up/down resistor
    P2OUT |=  BIT0; // set to pull-up
    P2IES |=  BIT0; // interrupt on high-low transition
    P2IFG &= ~BIT0; // make sure interrupt flag is cleared
    P2IE  |=  BIT0; // enable interrupt

    // enter low power mode to wait for interrupts
    __bis_SR_register(GIE + LPM0_bits);
}

//******************************************************************************
// Button Interrupts *************************************************************
//******************************************************************************
#pragma vector = PORT2_VECTOR
__interrupt void P2_ISR(void)
{
  switch(__even_in_range(P2IV, 0x1F))
  {
    case P2IV_NONE:    break;        // Vector 0: No interrupts
    case P2IV_P2IFG0:                // Vector 2: P2.0
        P1OUT ^= BIT0;      // toggle LED
        btnCount++;         // increment counter
        __no_operation();   // set breakpoint here to check btnCount
        break;
    case P2IV_P2IFG1:  break;        // Vector 4: P2.1
    case P2IV_P2IFG2:  break;        // Vector 6: P2.2
    case P2IV_P2IFG3:  break;        // Vector 8: P2.3
    case P2IV_P2IFG4:  break;        // Vector 10: P2.4
    case P2IV_P2IFG5:  break;        // Vector 12: P2.5
    case P2IV_P2IFG6:  break;        // Vector 14: P2.6
    case P2IV_P2IFG7:  break;        // Vector 16: P2.7
    default:
        break;
  }
}
