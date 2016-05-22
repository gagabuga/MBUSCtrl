// swtimer.h
#pragma once

#ifndef _SWTIMER_h
#define _SWTIMER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "../inc/types.h"


/**************************************************************************
* definitions
**************************************************************************/


typedef struct {
 u32 TimerVar;
 u08 Flag;
 u08 Enabled;
} TSWTimer;

/**************************************************************************
* declarations
**************************************************************************/

//!!!GLOBAL u08 SWdtReset;
//!!!GLOBAL u32 SWdtTimer;
extern u08 SWdtReset;
extern u32 SWdtTimer;

/**************************************************************************
* function prototypes
**************************************************************************/

void SWTInit(void);

void SWTLoad(u08 bTimer, u32 dwTime);

u08 SWTEnabled(u08 bTimer);

u08 SWTFlag(u08 bTimer);

void SWTClear(u08 bTimer);

void SWTStop(u08 bTimer);

void SWTResume(u08 bTimer);

void SWTTrigger(void);

void ProcessSWTimers(void);

u32 inline GetTickCount(void);

void inline SetTickCount(u32 nTicks);

//!!!#undef GLOBAL



#endif /* _SWTIMER_h */

