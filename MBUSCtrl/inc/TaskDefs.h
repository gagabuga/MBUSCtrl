#pragma once

#ifndef _TASKSDEFS_h
#define _TASKSDEFS_h

/**************************************************************************
* definitions
**************************************************************************/
#define MAX_TOTAL_TASKS_IN_QUEUE    20


// Tasks queues
typedef enum {
	TSK_MAIN_TASK,
	TSK_PLAYER_TASK,
	TSK_MBUS_SEND_TASK,
	TSK_MAX_TASKS
} TTasks;


// Tasks
typedef enum {
	MT_IDDLE,
	MT_SCAN_DISKS,
	MT_READ_DIRECTORY,
	MT_UPDATE_SCREEN,
	MT_INIT_DIRECTORY_SCREEN,
	MT_INIT_GMS_SCREEN,
	MT_REDRAW_FILES_ONLY,
	MT_CF_EJECT,

	MT_CHECK_UNILINK,
	MT_UNILINK_PROCESS_RECV_DATA,
	MT_UNILINK_RESEARCH_PL,
	MT_UNILINK_SEARCH_PL,

	MT_REFRESH_SCREEN_COMPLETTE,

	MT_LAST_TASK
}TMainTask;



typedef enum {
	PT_VS_READ_FILE,
	PT_LAST_TASK,
} TPlayerTask;


#endif /* _TASKSDEFS_h */
