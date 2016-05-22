// tasks.h

#ifndef _TASKS_h
#define _TASKS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "../inc/types.h"


/**************************************************************************
* definitions
**************************************************************************/

extern BYTE volatile bFirstTask[TSK_MAX_TASKS];
extern BYTE volatile bLastTask[TSK_MAX_TASKS];

typedef struct {
	BYTE bTask;
} TTaskElement;



/**************************************************************************
* declarations
**************************************************************************/

/**************************************************************************
* implementation
**************************************************************************/

/**************************************************************************
* function prototypes
**************************************************************************/

void InitTasks(void);
void ClearAllTasks(BYTE TaskQueueNumber);
BYTE GetNextTask(BYTE TaskQueueNumber);
BYTE AddTask(BYTE TaskQueueNumber, BYTE TaskID);
void DoneTask(BYTE TaskQueueNumber);


#endif /* _TASKS_h */

