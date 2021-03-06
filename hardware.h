/*
 * hardware.h
 *
 *  Created on: May 4, 2020
 *      Author: Michael Bolt
 */

#ifndef HARDWARE_H_
#define HARDWARE_H_


#include <msp430.h>
#include <stdint.h>

/*************************
 * Clock System Function *
 *************************/
void clockSytem_init(void);

/*******************
 * Timer Functions *
 *******************/
void timer_init(void);
inline void timer_sleep(void);


/********************
 * Button Functions *
 ********************/
// button definitions
#define BTN_UP      BIT2
#define BTN_DOWN    BIT1
#define BTN_SHOOT   BIT0
// functions
void buttons_init(void);
inline uint16_t readButton(uint16_t button);


#endif /* BUTTONS_H_ */
