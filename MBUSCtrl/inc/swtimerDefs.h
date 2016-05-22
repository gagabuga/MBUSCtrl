#pragma once
#ifndef _SWTIMERDEFS_h
#define _SWTIMERDEFS_h


/**************************************************************************
* definitions
**************************************************************************/

// Times
#define SWT_UPDATEPLAYER_TIMER_VAL                    300
#define SWT_UPDATESCREEN_TIMER_VAL                    500
#define SWT_UPDATESCREEN_TIMER_VAL_UNILINK_MODE       1000
#define SWT_UPDATESCREEN_TIMER_VAL_POWERDOWN_MODE     2500

#define SWT_UPDATESCREEN_TIMER_VAL_PLAYER_RUNNING     150

#define SWT_CHECK_UNILINK_TIMER_VAL                   3000
#define SWT_UNILINK_EDIT_NUM_TIMER_VAL                3000
#define SWT_UNILINK_SHOW_EDIT_STAT_TIMER_VAL          3000

#define SWT_BACKLIGT_TIMER_VAL                        15000
#define SWT_MBUS_PACKET_TIMER_VAL                     6
#define SWT_MBUS_PLAY_STATUS_TIMER_VAL                1000

#define SWT_MBUS_ANSWER_PAUSE_TIMER_VAL               1


// with MBUS DEBUGS - 15 ms
#define SWT_MBUS_ANSWER_BETWEEN_PACKETS_TIMER_VAL     20 // 25 // 95
//#define SWT_MBUS_ANSWER_FAST_TIMER_VAL                15 // 75 // 80
#define SWT_MBUS_ANSWER_AFTER_REQUEST_TIMER_VAL       20 // 25 // 80

#define SWT_MBUS_CMD_DELAY_TIMER_VAL                  16000
#define SWT_MBUS_POWER_OFF_TIMER_VAL                  5000

#define SWT_BACK_TO_BEGIN_TIMER_VAL                   3000
#define SWT_CF_EJECT_DEBOUNCE_TIMER_VAL               300

// MBus R: |5110127| MBus S: |D9501040000000C9| 76
//MBus R: |E94010201280001B|  30 - 115200 bit/s  14400 byte  - 1s
//                                                 30          x
// Timers
typedef enum {
	SWT_MAIN_TIMER,
	SWT_BUTTON_ROTATION_TIMER,
	SWT_LONG_BUTTON_TIMER,
	SWT_DOUBLE_BUTTON_TIMER,
	SWT_UPDATESCREEN_TIMER,
	SWT_UPDATEPLAYER_TIMER,

	SWT_MBUS_PACKET_TIMER,
	SWT_CHECK_UNILINK_TIMER,
	SWT_UNILINK_SYNC_TIMER,
	SWT_UNILINK_EDIT_NUM_TIMER,
	SWT_UNILINK_SHOW_EDIT_STAT_TIMER,

	SWT_BACKLIGT_TIMER,

	SWT_MBUS_PLAY_STATUS_TIMER,
	SWT_MBUS_ANSWER_TIMER,
	SWT_MBUS_CMD_DELAY_TIMER,
	SWT_MBUS_POWER_OFF_TIMER,

	SWT_BACK_TO_BEGIN_TIMER,
	SWT_CF_EJECT_DEBOUNCE_TIMER,

	SWT_MAX_TIMER
} TTimers;

/**************************************************************************
* declarations
**************************************************************************/


/**************************************************************************
* function prototypes
**************************************************************************/


/**************************************************************************
* implementation
**************************************************************************/







#endif /* _SWTIMERDEFS_h */ 

