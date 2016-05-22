#include "../inc/TaskDefs.h"
#include "../inc/tasks.h"

/**************************************************************************
* declarations
**************************************************************************/

BYTE volatile bFirstTask[TSK_MAX_TASKS];
BYTE volatile bLastTask[TSK_MAX_TASKS];
TTaskElement volatile TasksQueue[TSK_MAX_TASKS * MAX_TOTAL_TASKS_IN_QUEUE];

static BYTE bSemafor = 0;

/**************************************************************************
* implementation
**************************************************************************/
void InitTasks(void)
{
	memset((BYTE *)bFirstTask, 0, TSK_MAX_TASKS);
	memset((BYTE *)bLastTask, 0, TSK_MAX_TASKS);
	memset((BYTE *)TasksQueue, 0, MAX_TOTAL_TASKS_IN_QUEUE * TSK_MAX_TASKS);
}

/**************************************************************************
* implementation
**************************************************************************/


void ClearAllTasks(BYTE TaskQueueNumber)
{
	// if(TaskQueueNumber < TSK_MAX_TASKS)
	{
		bLastTask[TaskQueueNumber] = 0;
		bFirstTask[TaskQueueNumber] = 0;
	}
}

BYTE GetFreeTaskPlace(BYTE TaskQueueNumber)
{
	BYTE ReturnVal;
	// if(TaskQueueNumber < TSK_MAX_TASKS)
	{
		if (bFirstTask[TaskQueueNumber] == bLastTask[TaskQueueNumber])
			ReturnVal = MAX_TOTAL_TASKS_IN_QUEUE - 1;
		else if (bFirstTask[TaskQueueNumber] < bLastTask[TaskQueueNumber])
		{
			ReturnVal = ((MAX_TOTAL_TASKS_IN_QUEUE - 1) - (bLastTask[TaskQueueNumber] - bFirstTask[TaskQueueNumber]));
		}
		else
			ReturnVal = (bFirstTask[TaskQueueNumber] - bLastTask[TaskQueueNumber] - 1);
	}
	/*
	else
	ReturnVal = 0;
	*/
	return(ReturnVal);
}


// Returned next Task in TaskQueueNumber TaskQueue
BYTE GetNextTask(BYTE TaskQueueNumber)
{
	BYTE RetVal = 0;
	// if(TaskQueueNumber < TSK_MAX_TASKS)
	{
		if (GetFreeTaskPlace(TaskQueueNumber) < (MAX_TOTAL_TASKS_IN_QUEUE - 1))
		{
			RetVal = TasksQueue[TaskQueueNumber * MAX_TOTAL_TASKS_IN_QUEUE + bFirstTask[TaskQueueNumber]].bTask;
		}
	}
	return(RetVal);
}

// Add Task in TaskQueueNumber TaskQueue
BYTE AddTask(BYTE TaskQueueNumber, BYTE TaskID)
{
	BYTE fProcessed = 0;
	if (!bSemafor)
	{
		bSemafor = 1;
		// if(TaskQueueNumber < TSK_MAX_TASKS)
		{
			if (GetFreeTaskPlace(TaskQueueNumber) != 0)
			{
				TasksQueue[TaskQueueNumber * MAX_TOTAL_TASKS_IN_QUEUE + bLastTask[TaskQueueNumber]].bTask = TaskID;
				bLastTask[TaskQueueNumber]++;
				if (bLastTask[TaskQueueNumber] >= MAX_TOTAL_TASKS_IN_QUEUE)
					bLastTask[TaskQueueNumber] = 0;

				fProcessed = 1;
			}
		}
		bSemafor = 0;
	}
	return(fProcessed);
}

// Erased Task from Queue
void DoneTask(BYTE TaskQueueNumber)
{
	bSemafor = 1;
	// if(TaskQueueNumber < TSK_MAX_TASKS)
	{
		if (GetFreeTaskPlace(TaskQueueNumber) < (MAX_TOTAL_TASKS_IN_QUEUE - 1))
		{
			bFirstTask[TaskQueueNumber]++;
			if (bFirstTask[TaskQueueNumber] >= MAX_TOTAL_TASKS_IN_QUEUE)
				bFirstTask[TaskQueueNumber] = 0;
		}
	}
	bSemafor = 0;
}







