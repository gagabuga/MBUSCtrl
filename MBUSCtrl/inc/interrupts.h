// interrupts.h

#ifndef _INTERRUPTS_h
#define _INTERRUPTS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

/**************************************************************************
* function prototypes
**************************************************************************/

void  Int_InitTimer(void);

#endif /* _INTERRUPTS_h */

