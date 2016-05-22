#include "../inc/types.h"
#include "../inc/swtimer.h"
#include "../inc/swtimerDefs.h"

/**************************************************************************
* definitions
**************************************************************************/


/**************************************************************************
* declarations
**************************************************************************/

static TSWTimer SWTimerVar[SWT_MAX_TIMER];
static u32      dwTimerTicks;
u08 SWdtReset;
u32 SWdtTimer;


/**************************************************************************
* function prototypes
**************************************************************************/


/**************************************************************************
* implementation
**************************************************************************/

u32 inline GetTickCount(void)
{
  return(dwTimerTicks);
}

void inline SetTickCount(u32 nTicks)
{
  dwTimerTicks = nTicks;
}

void SWTInit(void)
{
 u08 i;
 for (i = 0; i < SWT_MAX_TIMER; i++)
 {
   SWTClear(i);
 }
 dwTimerTicks = 0;

}

void SWTLoad(u08 bTimer, u32 dwTime)
{
 /*
 if (bTimer < SWT_MAX_TIMER)
 {
 */
   SWTimerVar[bTimer].TimerVar = dwTime;
   SWTimerVar[bTimer].Flag     = 0;
   SWTimerVar[bTimer].Enabled  = 1;
 //}
}

u08 SWTEnabled(u08 bTimer)
{
  /*
  if (bTimer < SWT_MAX_TIMER)
  {
  */
   return(SWTimerVar[bTimer].Enabled);
   /*
  }
  else
   return(0);
  */
}

u08 SWTFlag(u08 bTimer)
{
  /*
  if (bTimer < SWT_MAX_TIMER)
  {
  */
   return(SWTimerVar[bTimer].Flag);
  /*
  }
  else
   return(1);
  */
}

void SWTClear(u08 bTimer)
{
  /*
  if (bTimer < SWT_MAX_TIMER)
  {
  */
   SWTimerVar[bTimer].Enabled  = 0;
   SWTimerVar[bTimer].Flag     = 0;
   SWTimerVar[bTimer].TimerVar = 0;
  //}
}

void SWTStop(u08 bTimer)
{

  if (bTimer < SWT_MAX_TIMER)
  {
   SWTimerVar[bTimer].Enabled  = 0;
  }

}

void SWTResume(u08 bTimer)
{
  if (bTimer < SWT_MAX_TIMER)
  {
   SWTimerVar[bTimer].Enabled  = 1;
  }
}

void SWTTrigger(void)
{
 u08 i;
 dwTimerTicks++;

 for (i = 0; i < SWT_MAX_TIMER; i++)
 {
   if (SWTimerVar[i].Enabled)
   {
     if (SWTimerVar[i].TimerVar > 0)
     {
       SWTimerVar[i].TimerVar--;
     }
     else
     {
      SWTimerVar[i].Flag     = 1;
      SWTimerVar[i].Enabled  = 0;
     }
   }
 }
}

