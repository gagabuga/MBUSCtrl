#include "../inc/const.h"
#include "../inc/interrupts.h"
#include "../inc/swtimer.h"

/**************************************************************************
* implementation
**************************************************************************/

void  Int_InitTimer()
{
 outp(0, TCCR1A);
 outp(4, TCCR1B);		// prescaler /256  tPeriod = 64 uS
 outp(TI1_H, TCNT1H);	// load counter value hi
 outp(TI1_L, TCNT1L);	// load counter value lo
 sbi(TIMSK1, TOIE1);   //!!!Проверить настроку маски
}

// timer 1 overflows every 1 mS
SIGNAL(SIG_OVERFLOW1)		
{
 outp(TI1_H, TCNT1H);	// reload timer 
 outp(TI1_L, TCNT1L);	// reload timer	

 if (SWdtReset)
 {
   SWdtReset = 0;
   SWdtTimer = SWDT_VALUE;
 }
 if (SWdtTimer)
 {
  SWdtTimer--;
  //!!! wdt_reset(); //Что делает эта функция?
 }

 SWTTrigger();

 //if ( !MBus_IsSending() )
 {
   //!!!ButtonRotaion(); //Что делает эта функция?

   ProcessSWTimers();
 }
}





