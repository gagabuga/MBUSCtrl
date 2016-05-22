#pragma once

#ifndef _CONST_h
#define _CONST_h


//#define NO_SLEEP

#define VERSION_H 1
#define VERSION_L 0


/**************************************************************************
* definitions
**************************************************************************/

//******************************************************************************** 
// timer interrupt
//******************************************************************************** 
// CPU clock is 16.000 MHz
// prescaler set to 256 uS
// Timer 1 set to 1 ms
#define TI1_H	0xFF
#define TI1_L	0xC2


// prescaler set to 8
// Timer 2 set to 18 uS
//#define TI2_L   0xF6 //0xEC // 0xF9

// Timer 2 set to 18 uS
#define TI2_L_18us   0xEE 

// Timer 2 set to 38 uS
#define TI2_L_38us   0xE3 

// Timer 2 set to 26 uS
#define TI2_L_26us   0xEE



#define IDDLE_WAIT_TIME 100         // 100 * 100 mS

#define SWDT_VALUE      1500L 

#define SWT_KBD_TIMER_VAL   50L      // 50 mS

#define SWT_PWM_TIMER_VAL   2L

#define inp(p) p
#define outp(v,p) p = v
#define sbi(p, pi) p |= _BV(pi)
#define cbi(p,pi) p &= ~_BV(pi)

#define BV  _BV
//
// system constants
//
#define F_CPU	16000000L		// CPU clock frequency

//#define DDR(x) ((x)-1)    	// address of data direction register of port x 
//#define PIN(x) ((x)-2)    	// address of input register of port x 

//
// some max values
//
#define MAX_U16  65535
#define MAX_S16  32767

#define NO_FILE "No file          "



#endif /* _CONST_h */
