// MBus.h

#ifndef _MBUS_h
#define _MBUS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "../inc/const.h"
#include "../inc/types.h"


// Some debug switches
#define MBUS_SEND_DEBUG
#define MBUS_RECEIVE_DEBUG
//#define MBUS_RECEIVE_UNILINK_CMD_DEBUG

/**************************************************************************
* definitions
**************************************************************************/

// MBus Interface ports and pins
#define MBUS_PORT     PORTD
#define MBUS_DIR      DDRD
#define MBUS_PINS     PIND
#define MBUS_RX_PIN   PD1
#define MBUS_ACC_PIN  PD5

// switch ports to sending mode on or off
#define MBUS_SEND_ON()    sbi(MBUS_DIR, MBUS_RX_PIN); EIMSK &= ~(1 << MBUS_RX_INT);
#define MBUS_SEND_OFF()   cbi(MBUS_DIR, MBUS_RX_PIN); EIMSK |=  (1 << MBUS_RX_INT); EIFR |= (1 << MBUS_RX_INT);

// depend on hardware realisation - switchs TX pin low or high
#define MBUS_TX_PIN_LOW()  cbi(MBUS_PORT, MBUS_RX_PIN)
#define MBUS_TX_PIN_HIGH() sbi(MBUS_PORT, MBUS_RX_PIN)

// RX Interrupt defines
#define MBUS_RX_INT       INT1
#define MBUS_RX_EICR      EICRA
#define MBUS_RX_INT_FAL     ( 1 << ISC11 )
#define MBUS_RX_INT_RIS   ( ( 1 << ISC11 ) | ( 1 << ISC10 ) )
#define MBUS_RX_INT_MSK   ( ( 1 << ISC11 ) | ( 1 << ISC10 ) )
#define MBUS_RX_SIG_INT   SIG_INTERRUPT1

// RX Timer defines
#define MBUS_RX_TIMER_TCCR    TCCR2
#define MBUS_RX_PRESCALE      (1 << CS22)               // prescaler for Receiving process 256
#define MBUS_RX_TIMER_EN      (1 << TOIE2)
#define MBUS_RX_TCNT          TCNT2

// TX Timer defines
#define MBUS_TX_INT_EN   (1 << TOIE0)  //Timer/Counter0 Overflow Interrupt Enable - разрешение прерывани€ по переполнению 0-го таймера
#define MBUS_TX_SIG_INT  SIG_OVERFLOW0  //прерывание по переполнению счетчика
#define MBUS_TX_TCNT     TCNT0  //счетный регистр. ¬ зависимости от режима работы содержимое этого регистра инкрементируетс€
#define MBUS_TX_TCCR     TCCR0  //регистр управлени€. «адает режим работы таймера “0.

#define MBUS_TX_PRESCALE_256 ((1 << CS02) | 1 << CS01)  // prescaler for normal operation
#define MBUS_TX_PRESCALE_1   (1 << CS00)                // minimum prescaler to force interrupt

#define MBUS_MAX_PACKET_LEN           15

// Send timings
#define MBUS_TCNT_TOLERANCE         ( 0x20 )
#define MBUS_TCNT_BIT_TIME          0xBD  // 2.9 - 3.0 ms  complete bit

#define MBUS_TCNT_LOW_ONE           0x74  // 1.7 - 1.9 ms  one-time
#define MBUS_TCNT_LOW_ZERRO         0x22  // 0.5 - 0.6 ms  zerro-time

#define MBUS_TCNT_HIGH_ONE          ( MBUS_TCNT_BIT_TIME - MBUS_TCNT_LOW_ONE )
#define MBUS_TCNT_HIGH_ZERRO        ( MBUS_TCNT_BIT_TIME - MBUS_TCNT_LOW_ZERRO )

#define MBUS_TCNT_PACKET_DELAY      ( 0xAF)

#define MBUS_TX_TCNT_BIT_TIME       ( 0xFF - MBUS_TCNT_BIT_TIME     )

#define MBUS_TX_TCNT_LOW_ONE        ( 0xFF - MBUS_TCNT_LOW_ONE      )
#define MBUS_TX_TCNT_LOW_ZERRO      ( 0xFF - MBUS_TCNT_LOW_ZERRO    )

#define MBUS_TX_TCNT_HIGH_ONE       ( 0xFF - MBUS_TCNT_HIGH_ONE     )
#define MBUS_TX_TCNT_HIGH_ZERRO     ( 0xFF - MBUS_TCNT_HIGH_ZERRO   )

#define MBUS_TX_TCNT_PACKET_DELAY   ( 0xFF - MBUS_TCNT_PACKET_DELAY )

// Receive states
typedef enum {
	MBR_WAITING,
	MBR_FALLING,
	MBR_RISING,
	MBR_DONT_RECEIVE,
	MBR_MAX_STATES
} TLineRecStates;

// Data for receive process
typedef struct {
	u08 bCanReceive;
	u08 bBitCounter;                          // Count of bit received
	u08 bBitsArray[MBUS_MAX_PACKET_LEN];    // Received Message
	u08 bBitRecState;                         // Current receive state
	u08 bReceived;                            // Flag that message is received
} TMBusRx;


// MBUS Protocol defines

#define MBUS_HEAD_ADDRESS             0xD
#define MBUS_OUR_ADDRESS              0x5

#define MBUS_CMD_ACK                  0x2
#define MBUS_CMD_WAKEUP               0x9

typedef enum {
	MBUS_CMD_CONTROL = 0x1,
	MBUS_CMD_POWER_ON_PING = 0x7,
	MBUS_CMD_PING = 0x8,
	MBUS_CMD_PLAY_STATUS = 0x9,
	MBUS_CMD_SPINUP_STATUS = 0xA,

	MBUS_STATUS_ANSWER = 0xB,
	MBUS_CMD_CDINFO = 0xC,
	MBUS_STATUS1_ANSWER = 0xD,
	MBUS_WAKEUP_ANSWER = 0xF,

	MBUS_PAUSE_CMD = 0x10,
	MBUS_MAX_CMDS = 0x20
} TMbusCmds;


#define  MBUS_CMD_CTRL_PLAY           0x1
#define    MBUS_CMD_CTRL_PLAY_PLAY    0x01
#define    MBUS_CMD_CTRL_PLAY_PAUSE   0x02  // ?
#define    MBUS_CMD_CTRL_PLAY_FF      0x04
#define    MBUS_CMD_CTRL_PLAY_REW     0x08
#define    MBUS_CMD_CTRL_PLAY_STOP    0x40

#define  MBUS_CMD_CTRL_SEEK           0x3

#define MBUS_MAX_DISKS                6

// Byte and bit positions
#define MBUS_BYTE_DST                 0
#define MBUS_BIT_DST                  4

#define MBUS_BYTE_CMD                 0
#define MBUS_BIT_CMD                  0

#define MBUS_BYTE_SUB_CMD             1
#define MBUS_BIT_SUB_CMD              4

#define MBUS_BYTE_SUB_DATA0           1
#define MBUS_BIT_SUB_DATA0            0

#define MBUS_BYTE_SUB_DATA1           2
#define MBUS_BIT_SUB_DATA1            4

#define MBUS_BYTE_SUB_DATA2           2
#define MBUS_BIT_SUB_DATA2            0

// Cd Info cmd
#define MBUS_BYTE_CDINFO_DISK_N       1
#define MBUS_BIT_CDINFO_DISK_N        4

#define MBUS_BYTE_CDINFO_1st_TRACK_H  1
#define MBUS_BIT_CDINFO_1st_TRACK_H   0

#define MBUS_BYTE_CDINFO_1st_TRACK_L  2
#define MBUS_BIT_CDINFO_1st_TRACK_L   4
#define MBUS_BYTE_CDINFO_Lst_TRACK_H  2
#define MBUS_BIT_CDINFO_Lst_TRACK_H   0

#define MBUS_BYTE_CDINFO_Lst_TRACK_L  3
#define MBUS_BIT_CDINFO_Lst_TRACK_L   4

#define MBUS_BYTE_CDINFO_MINUTES_H    3
#define MBUS_BIT_CDINFO_MINUTES_H     0

#define MBUS_BYTE_CDINFO_MINUTES_L    4
#define MBUS_BIT_CDINFO_MINUTES_L     4

#define MBUS_BYTE_CDINFO_SECUNDES_H   4
#define MBUS_BIT_CDINFO_SECUNDES_H    0

#define MBUS_BYTE_CDINFO_SECUNDES_L   5
#define MBUS_BIT_CDINFO_SECUNDES_L    4

#define MBUS_BYTE_CDINFO_CD_IN        5
#define MBUS_BIT_CDINFO_CD_IN         0

// Wake Up answer
// Just send DF005668

// this defines are not used because of bit order in GCC, but idee was good
/////// Data for Control cmd
typedef struct {
	u08 Action : 8;
	u08 Reserved : 4;
} TControlPlay;
typedef struct {
	u08 Reserved1 : 4;
	u08 TrakNo : 8;
	u08 Reserved2 : 4;
} TControlSeek;
typedef struct {
	u08 Reserved1 : 1;
	u08 Repeat : 1;
	u08 Reserved2 : 4;
	u08 Random : 1;
	u16 Reserved3 : 9;
} TControlConfig;

typedef struct {
	u08 SubCmd : 4;
	union
	{
		TControlPlay  PlayCmd; // 16
		TControlSeek  SeekCmd; // 20
		TControlConfig Config; // 20
	};
} TDataControl;   // 16 - 20


				  /////// Data for Hello cmd
typedef struct {
	u08 bHello : 4;
} TDataHello;         // 4 bits

					  /////// Data for Status cmd
typedef struct {        // aaaa-bbbb-bbbb-cccc-dddd-eeee-eeee-ffff-ffff-gggg-hhhh-iiii-jjjj
	u08 PlayState : 4;   // aaaa is 0000 (stopped), 0110 (fast forwarding), 0100 (playing), 0101 (seeking), 0111 (rewinding)
	u08 Track : 8;   // bbbb-bbbb is a BCD encoding of the current track
	u08 CDNumber : 8;   // cccc-dddd is 0000-0001 (maybe CD number in a changer?)
	u08 Minutes : 8;   // eeee-eeee is a BCD encoding of the current position minutes
	u08 Seconds : 8;   // ffff-ffff is a BCD encoding of the current position seconds
	u08 RepeatState : 4;   // gggg is 0100 when repeat is on, 0000 otherwise
	u08 RandomState : 4;   // hhhh is 0010 when random is on, 0000 otherwise
	u08 Reserved1 : 4;   // iiii is 0000
	u08 Reserved2 : 4;   // jjjj is [0001|1100]
} TDataStatus;        // 52 bits



typedef struct {
	u08 bSrc : 4;
	u08 bCmd : 4;
	union
	{
		TDataControl Control;
		TDataStatus  Status;

	} Data;
} TMBusPacket;


#define MBusSetDiskNum(a)     MBusStatus.bDiskNum    = a
#define MBusSetTrackNum(a)    MBusStatus.bTrackNum   = a
#define MBusSetMinutes(a)     MBusStatus.bMinutes    = a
#define MBusSetSeconds(a)     MBusStatus.bSeconds    = a
#define MBusSetMaxDisks(a)    MBusStatus.bMaxDisks   = a
#define MBusSetMaxTracks(a)   MBusStatus.bMaxTracks  = a
#define MBusSetMaxMinutes(a)  MBusStatus.bMaxMinutes = a
#define MBusSetMaxSecundes(a) MBusStatus.bMaxSeconds = a
#define MBusSetPlayState(a)   MBusStatus.ePlayState  = a
#define MBusSetRepeatMode(a)  MBusStatus.eRepeatMode = a

#define MBusGetDiskNum()      MBusStatus.bDiskNum
#define MBusGetTrackNum()     MBusStatus.bTrackNum  
#define MBusGetMinutes()      MBusStatus.bMinutes   
#define MBusGetSeconds()      MBusStatus.bSeconds   
#define MBusGetMaxDisks()     MBusStatus.bMaxDisks  
#define MBusGetMaxTracks()    MBusStatus.bMaxTracks 
#define MBusGetMaxMinutes()   MBusStatus.bMaxMinutes
#define MBusGetMaxSecundes()  MBusStatus.bMaxSeconds
#define MBusGetPlayState(a)   MBusStatus.ePlayState
#define MBusGetRepeatMode(a)  MBusStatus.eRepeatMode


// MBus Driver internal state storages
typedef enum {
	PS_PLAYING,
	PS_PAUSED,
	PS_STOPPED,
	PS_FF,
	PS_REW,
	PS_MIX,
	PS_SCAN,
	PS_PREPARING,
	PS_MAX_STATES
} TPlayState;

typedef enum {
	RM_NORMAL = 0,
	RM_REPEAT_ONE = 4,
	RM_REPEAT_ALL = 8,
	RM_MAX_MODES
} TRepeatMode;


typedef struct {
	u08 bMBusPresent;
	TPlayState ePlayState;
	TRepeatMode eRepeatMode;
	u08 bDiskNum;
	u08 bTrackNum;
	u08 bMinutes;
	u08 bSeconds;
	u08 bMaxDisks;
	u08 bMaxTracks;
	u08 bMaxMinutes;
	u08 bMaxSeconds;
	u08 bAcceptUnilinkCmds;
	u08 bPinged;
} TMBusStatus;

/**************************************************************************
* declarations
**************************************************************************/


/**************************************************************************
* function prototypes
**************************************************************************/

void MBus_AcceptUnilinkCmds(u08 OnOff);

TMBusStatus * MBus_GetMBusStatus(void);

TMBusRx * MBus_GetRxProcess(void);

void MBus_Init(void);

void MBus_CheckMBus(void);

u08 MBus_Present(void);

void MBus_ReceivedPacket(void);

void MBus_StartReceiving(void);

void MBus_Process(void);

u08 MBus_IsSending(void);

void MBus_SendPacket(void);

u08 MBus_PreparePacket(u08 bCmd);

u08 MBus_MakeCheckSumm(u08 bBitCnt, u08 * bBitsArray);

u08 MBus_UnilinkCmd(u08 bCmd);

void MBus_ProcessUnilinkCmd(u08 * pBuff);

void MBus_ProcessSeekCmd(void);

void MBus_StopPlayBack(void);


void MBus_SetPinged(u08 OnOff);



#endif /* _MBUS_h */

