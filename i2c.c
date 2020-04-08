/*
 * i2c.h
 *
 *  Created on: April 8, 2020
 *      Author: Michael Bolt
 */

#include "i2c.h"

/**************************************************************
 *  Global Variables  *****************************************
 **************************************************************/
static uint8_t              i2cTransmitBuffer[I2C_TX_BUFFER_SIZE] = {0};
static volatile uint16_t    i2cTransmitCounter = 0;
static uint16_t             i2cTransmitIndex = 0;



/*
 * ! Initialize I2C interface
 * !
 * ! Follows procedure outlined on p. 629 of the MSP430FR2xx
 * ! Family User Guide
 */
void i2c_init(void) {
    // 1. set UCSWRST to disable the eUSCI_B module
    UCB0CTLW0   =   UCSWRST;        // enable SW reset
    // 2. Initialize all eUSCI_B registers with UCSWRST = 1
    UCB0CTLW0   |=  UCMODE_3 |      // I2C mode
                    UCMST |         // Master mode
                    UCSSEL__SMCLK | // CLK source: SMCLK (1 MHz default)
                    UCSYNC;         // Synchronous mode enable
    UCB0BRW     =   3;              // SCL = SMCLK / 3 ~= 350 kHz
    // 3. Configure ports:
    //   MSP430FR2433 datasheet p. 55
    //      P1.2 = SDA (DIR=X, SEL=01)
    //      P1.3 = SCL (DIR=X, SEL=01)
    P1SEL0      |=  (BIT2 | BIT3);
    P1SEL1      &= ~(BIT2 | BIT3);
    // 4. Clear UCSWRST to release the eUSCI_B for operation
    UCB0CTLW0   &= ~(UCSWRST);
    // 5. Enable interrupts
    UCB0IE      |=  UCNACKIE;       // enable NACK interrupt
}



/*
 * ! Transmit data through I2C interface
 * !
 * ! \param deviceAddress: 7-bit I2C slave device address
 * ! \param *data: pointer to the byte array of data to write
 * ! \param count: number of bytes to transmit
 * !
 * ! \return number of bytes left in I2C transmit buffer; 0 indicates
 * !         success, any other value indicates failure
 * !
 * ! Copies count bytes from *data to a transmission buffer and begins
 * ! interrupt based transmission through the I2C interface
 */
uint16_t i2c_tx(uint8_t deviceAddress, const uint8_t *data, uint16_t count) {
    // prepare data for transmission
    i2cTransmitCounter = count;         // there are count bytes to transmit
    i2cTransmitIndex = 0;               // start from byte 0
    do {                                // copy data into transmit buffer
        count--;
        i2cTransmitBuffer[count] = data[count];
    } while (count);



    // configure and begin I2C transmission
    UCB0I2CSA    = deviceAddress;   // set slave address
    UCB0IFG     &= ~UCTXIFG;        // clear any pending interrupts
    UCB0IE      |=  UCTXIE;         // enable TX interrupt
    UCB0CTLW0   |=  UCTR |          // I2C TX mode
                    UCTXSTT;        // send START condition

    // enter LPM0 until transmission is complete
    __bis_SR_register(GIE + LPM0_bits);

    // return the number of bytes left in the transmit buffer:
    //   0  if transmission succeeded,
    //  >0  if transmission failed
    return i2cTransmitCounter;
}





/**************************************************************
 * I2C Interrupt Vector ***************************************
 **************************************************************/
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void) {
    switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
    {
      case USCI_NONE:          break;           // Vector 0: No interrupts
      case USCI_I2C_UCALIFG:   break;           // Vector 2: ALIFG
      case USCI_I2C_UCNACKIFG:                  // Vector 4: NACKIFG
        __bic_SR_register_on_exit(CPUOFF);          //exit LPM0
        break;
      case USCI_I2C_UCSTTIFG:  break;           // Vector 6: STTIFG
      case USCI_I2C_UCSTPIFG:  break;           // Vector 8: STPIFG
      case USCI_I2C_UCRXIFG3:  break;           // Vector 10: RXIFG3
      case USCI_I2C_UCTXIFG3:  break;           // Vector 12: TXIFG3
      case USCI_I2C_UCRXIFG2:  break;           // Vector 14: RXIFG2
      case USCI_I2C_UCTXIFG2:  break;           // Vector 16: TXIFG2
      case USCI_I2C_UCRXIFG1:  break;           // Vector 18: RXIFG1
      case USCI_I2C_UCTXIFG1:  break;           // Vector 20: TXIFG1
      case USCI_I2C_UCRXIFG0:  break;           // Vector 22: RXIFG0
      case USCI_I2C_UCTXIFG0:                   // Vector 24: TXIFG0
          if (i2cTransmitCounter) {                 //if more to transmit:
            UCB0TXBUF = i2cTransmitBuffer[i2cTransmitIndex++];  //transmit byte and increment index
            i2cTransmitCounter--;                               //decrement counter
          }
          else {                                    //if done with transmission
            UCB0CTLW0 |= UCTXSTP;                       // Send stop condition
            UCB0IE &= ~UCTXIE;                          // disable TX interrupt
            __bic_SR_register_on_exit(CPUOFF);          // Exit LPM0
          }
          break;
      default:
          break;
    }
}


