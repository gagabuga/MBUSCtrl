#include "../inc/MBus.h"
#include "../inc/swtimerDefs.h"
#include "../inc/swtimer.h"
#include "../inc/TaskDefs.h"

/**************************************************************************
* definitions
**************************************************************************/

// Transmit states
typedef enum {
	MBTS_START,
	MBTS_LOW_0,
	MBTS_LOW_1,
	MBTS_END,
	MBTS_MAX_STATES
} TLineTxStates;

// Data for Transmit process
typedef struct {
	u08 bTxState;                             // State of Transmit process
	u08 bBitsCount;                           // Total count of bits in a message
	u08 bBitsArray[MBUS_MAX_PACKET_LEN];    // Message to send
	u08 bCurBit;                              // Current bit to send
	u08 bSending;                             // Transmit porocess is runing
} TMBusTx;


/**************************************************************************
* declarations
**************************************************************************/

TMBusTx MBTProcess;                         // Transmit process data

TMBusRx MBRProcess;                         // Receive process data

TMBusStatus MBusStatus;                     // States of M-Bus driver and some info to send to HU

u08 bDontSendSpinUp = 0;                    // Flag of sending Spin Up cmd

											/**************************************************************************
											* function prototypes
											**************************************************************************/


											/**************************************************************************
											* implementation
											**************************************************************************/

void MBus_SetPinged(u08 OnOff)
{
	MBusStatus.bPinged = OnOff;
}

void MBus_AcceptUnilinkCmds(u08 OnOff)
{
	MBusStatus.bAcceptUnilinkCmds = OnOff;
}

TMBusStatus * MBus_GetMBusStatus(void)
{
	return(&MBusStatus);
}

TMBusRx * MBus_GetRxProcess(void)
{
	return(&MBRProcess);
}


/*
*   Initialization of M-Bus driver
*/
void MBus_Init(void)
{
	memset((u08 *)&MBusStatus, 0, sizeof(TMBusStatus));
	MBusStatus.ePlayState = PS_STOPPED;                               // Player is stopped
	MBusStatus.bAcceptUnilinkCmds = 1;
	memset((u08 *)&MBTProcess, 0, sizeof(TMBusTx));
	memset((u08 *)&MBRProcess, 0, sizeof(TMBusRx));
	__asm__("nop");
	cbi(MBUS_DIR, MBUS_ACC_PIN); _NOP();                           // Set Acc pin to input
	cbi(MBUS_PORT, MBUS_ACC_PIN); _NOP();                           // Set Acc pullup resitor off

																	// Check, is there a M-Bus data line here
	sbi(MBUS_DIR, MBUS_RX_PIN); _NOP();                            // quck to output and 0 than back to input
	cbi(MBUS_PORT, MBUS_RX_PIN); _NOP();
	cbi(MBUS_DIR, MBUS_RX_PIN); _NOP();


	if (MBUS_PINS & (1 << MBUS_RX_PIN))                            // Is M-Bus dataline there?
	{
		MBusStatus.bMBusPresent = 1;                                    // Yes
	}
	sbi(MBUS_PORT, MBUS_RX_PIN);                                    // switch PullUp resistor on

																	// Set receive interrupt trigger
	MBUS_RX_EICR &= ~MBUS_RX_INT_MSK;                                 // Clear previous settings
	MBUS_RX_EICR |= MBUS_RX_INT_FAL;                                 // set Falling edge

	EIMSK |= (1 << MBUS_RX_INT);                                      // Enable receive interrupt

	MBus_StartReceiving();                                            // Enable receiving

	//!!!TIMSK &= ~MBUS_TX_INT_EN;                                         // disable transfer timer, but set it to fast start
	MBUS_TX_TCCR = MBUS_TX_PRESCALE_1;                               // prescale clk/1
	MBUS_TX_TCNT = 0xFF;                                             // fast rollover

	DEBUG_OUTP("MBus Initialized\n");

	SWTLoad(SWT_MBUS_ANSWER_TIMER, 1);   // Load answer timer, to enable answering mechanism

}

// Send a Power On ping to HU
void MBus_CheckMBus(void)
{
	if (!MBusStatus.bPinged)
	{
		// AddTask( TSK_MBUS_SEND_TASK, MBUS_CMD_POWER_ON_PING );
	}
}

// M-Bus Present Flag
u08 MBus_Present(void)
{
	return(MBusStatus.bMBusPresent);
}

// Send process runing
u08 MBus_IsSending(void)
{
	return MBTProcess.bSending;
}

/*
*   Receive interrupt
*/
SIGNAL(MBUS_RX_SIG_INT)
{
	u08 register bTimerVal;                                             // Timer value storage
	if (MBRProcess.bCanReceive)
	{
		switch (MBRProcess.bBitRecState)                                // Where we are in a receive process
		{
		default:                                                        // everything starts here
		case  MBR_WAITING:                                              // we are waiting for a falling enge
			if (MBTProcess.bSending == 0)                               // do we sending now? so we can to hear our messages.
			{
				MBRProcess.bBitCounter = 0;                                 // reset bitcounter
				MBRProcess.bBitRecState++;                                  // go to the next state
			}
		case MBR_FALLING:                                               // It is a falling edge

			MBUS_RX_TCNT = 0;                                      // resetting timer register
			MBUS_RX_TIMER_TCCR = MBUS_RX_PRESCALE;                       // Starting timer

			MBUS_RX_EICR &= ~MBUS_RX_INT_MSK;                             // Clear previous settings
			MBUS_RX_EICR |= MBUS_RX_INT_RIS;                             // set Rising edge
			MBRProcess.bBitRecState++;                                    // go to the next state
			break;

		case MBR_RISING:                                                // here we have a rising edge
			bTimerVal = MBUS_RX_TCNT;
			// clearing a place for received bit
			MBRProcess.bBitsArray[MBRProcess.bBitCounter / 8] &= ~(1 << (7 - (MBRProcess.bBitCounter % 8)));
			if (                                                          // is the Low level time in a margins for logig one
				(bTimerVal >= (MBUS_TCNT_LOW_ONE - MBUS_TCNT_TOLERANCE)) &&
				(bTimerVal <= (MBUS_TCNT_LOW_ONE + MBUS_TCNT_TOLERANCE))
				)                                                          // its a one
			{                                                             // Set a bit
				MBRProcess.bBitsArray[MBRProcess.bBitCounter / 8] |= (1 << (7 - (MBRProcess.bBitCounter % 8)));
			}

			MBRProcess.bBitCounter++;                                     // count that bit

			MBUS_RX_EICR &= ~MBUS_RX_INT_MSK;                             // Clear previous settings
			MBUS_RX_EICR |= MBUS_RX_INT_FAL;                             // set Falling edge
			MBRProcess.bBitRecState = MBR_FALLING;                        // waiting for a falling edge
																		  // load a timer, to end a packet on timeout
			SWTLoad(SWT_MBUS_PACKET_TIMER, SWT_MBUS_PACKET_TIMER_VAL);
			break;

		case MBR_DONT_RECEIVE:                                          // here we are not receiving
			break;
		}
		// just in case if state overrolled
		if (MBRProcess.bBitRecState >= MBR_MAX_STATES)
		{
			MBRProcess.bBitRecState = MBR_FALLING;
		}
		// check if there too much bits vsout timeouts are coming
		if (MBRProcess.bBitCounter >= (MBUS_MAX_PACKET_LEN * 8))
		{
			MBRProcess.bBitRecState = MBR_DONT_RECEIVE;                     // stop receiving
			MBRProcess.bReceived = 1;                                    // process the data
		}
	}
}

// Enable receive process
void MBus_StartReceiving(void)
{
	MBRProcess.bBitRecState = MBR_WAITING;
	MBRProcess.bCanReceive = 1;

}

/*
* Called on Receive timeout, sets a flag, that data received, and stops receive process
*/
void MBus_ReceivedPacket(void)
{
	MBUS_RX_TIMER_TCCR = 0;                                           // Stop receiving timer
	MBRProcess.bBitRecState = MBR_DONT_RECEIVE;                       // Set receive state to not-receive-state
	MBRProcess.bReceived = 1;                                         // Set flag of received data
	SWTLoad(SWT_MBUS_ANSWER_TIMER, SWT_MBUS_ANSWER_AFTER_REQUEST_TIMER_VAL);
}

/*
*   Transmitt timer interrupt
*/
SIGNAL(MBUS_TX_SIG_INT)
{
	switch (MBTProcess.bTxState)                                    // where we are in transmit process
	{
	case MBTS_START:                                                // in the beginning
		if (MBTProcess.bSending)                                    // are we sending
		{
			if (MBTProcess.bCurBit < MBTProcess.bBitsCount)           // it there something to send
			{                                                           // it is a one
				if (MBTProcess.bBitsArray[MBTProcess.bCurBit / 8] & (1 << (7 - (MBTProcess.bCurBit % 8))))
				{
					MBUS_TX_TCCR = MBUS_TX_PRESCALE_256;
					MBUS_TX_TCNT = MBUS_TX_TCNT_LOW_ONE;                    // Set Timer to Low One time
					MBTProcess.bTxState = MBTS_LOW_1;                       // go to next One-state
				}
				else                                                      // its a zerro
				{
					MBUS_TX_TCCR = MBUS_TX_PRESCALE_256;
					MBUS_TX_TCNT = MBUS_TX_TCNT_LOW_ZERRO;                  // Set Timer to Low Zerro
					MBTProcess.bTxState = MBTS_LOW_0;                       // go to next zerro-state
				}
				MBUS_TX_PIN_LOW();                                        // set data pin low
				MBTProcess.bCurBit++;                                     // calc next bit
			}
			else                                                        // we have everything sended
			{
				MBUS_TX_PIN_HIGH();                                       // Data pin to High
				MBUS_SEND_OFF();                                          // switch pins to receive mode (pin to input, RX interrupt on)
				MBTProcess.bTxState = MBTS_START;                       // Reset transmit state
				MBTProcess.bSending = 0;                                // we are not sending any more
				MBTProcess.bCurBit = 0;                                // reset current bit and 
				MBTProcess.bBitsCount = 0;                                // bits count
				TIMSK &= ~MBUS_TX_INT_EN;                  // disable transmit timer but set it to fast triggering
				MBUS_TX_TCCR = 0; // MBUS_TX_PRESCALE_1;               // prescaller to clk/1
				MBUS_TX_TCNT = 0xFF;                             // fast rollover
																 // Load a sw timer to minimum delay between two packets
				SWTLoad(SWT_MBUS_ANSWER_TIMER, SWT_MBUS_ANSWER_BETWEEN_PACKETS_TIMER_VAL);
			}
		}
		break;

	case MBTS_LOW_0:                                                // Zerro time is off
		MBUS_TX_TCCR = MBUS_TX_PRESCALE_256;
		MBUS_TX_TCNT = MBUS_TX_TCNT_HIGH_ZERRO;                       // Set Timer to the next bit time
		MBUS_TX_PIN_HIGH();                                           // Set pin high
		MBTProcess.bTxState = MBTS_START;                             // go to Bit-Start state
		break;

	case MBTS_LOW_1:                                                // One time is off
		MBUS_TX_TCCR = MBUS_TX_PRESCALE_256;
		MBUS_TX_TCNT = MBUS_TX_TCNT_HIGH_ONE;                         // Set Timer to the next bit time
		MBUS_TX_PIN_HIGH();                                           // Set pin high
		MBTProcess.bTxState = MBTS_START;                             // go to Bit-Start state
		break;

	default:                                                        // we must not get here at all, just in case...
	case MBTS_END:
		MBTProcess.bTxState = MBTS_START;
		break;
	}
}

/*
*  Start transmitt process
*/
void MBus_SendPacket(void)
{
	if (MBTProcess.bBitsCount)                                      // Is there somethign to send
	{
#ifdef MBUS_SEND_DEBUG
		u08 i;
		u08 bNibbles;
		bNibbles = MBTProcess.bBitsCount / 4 + ((MBTProcess.bBitsCount % 4) == 0 ? 0 : 1);
		DEBUG_OUTP(PSTR("MBus S: |"));
		for (i = 0; i < bNibbles; i++)
		{
			DEBUG_OUTP(PSTR("%X"), ((MBTProcess.bBitsArray[i / 2] >> (4 - (4 * (i % 2)))) & 0x0F));
		}
		DEBUG_OUTP(PSTR("|\n"));
#endif

		MBTProcess.bSending = 1;                                        // we are sending!
		MBUS_SEND_ON();                                                 // switch Pins to send mode (pin to output, RX interrupt off)
		MBTProcess.bTxState = MBTS_START;                               // Set transmit state to beginn of bit
		MBTProcess.bCurBit = 0;
		MBUS_TX_TCCR = MBUS_TX_PRESCALE_1;                           // set prescaller to clk/256
		MBUS_TX_TCNT = 0xFF;                                           // timer triggered sofort
		TIMSK |= MBUS_TX_INT_EN;                                        // enable transmit timer
																		/*
																		while ( MBTProcess.bSending == 1 )
																		{
																		wdt_reset();
																		}
																		*/
	}
}


/*
*  Processing received data here
*/
void MBus_Process(void)
{
	u08 bCrc;
	u08 nTaskDone;
	u08 bCurrentTask;
	static u08 bOldStatus = 0xff;


	if (MBRProcess.bReceived)                                       // is there something received
	{
		MBRProcess.bBitCounter -= 4;                                    // do not count crc nibble
																		// calc crc 
		bCrc = MBus_MakeCheckSumm(MBRProcess.bBitCounter, MBRProcess.bBitsArray);
		// check it
		if (((MBRProcess.bBitsArray[MBRProcess.bBitCounter / 8] >> (4 - (MBRProcess.bBitCounter % 8))) & 0x0F) == bCrc)
		{                                                               // packet is valid
			MBusStatus.bMBusPresent = 1;

#ifdef MBUS_RECEIVE_DEBUG
			u08 i;
			u08 bNibbles;
			bNibbles = (MBRProcess.bBitCounter + 4) / 4 + (((MBRProcess.bBitCounter + 4) % 4) == 0 ? 0 : 1);
			DEBUG_OUTP(PSTR("MBus R: |"));
			for (i = 0; i < bNibbles; i++)
			{
				DEBUG_OUTP(PSTR("%X"), ((MBRProcess.bBitsArray[i / 2] >> (4 - (4 * (i % 2)))) & 0x0F));
			}
			DEBUG_OUTP(PSTR("|\n"));
#endif

			// is it our address
			if (((MBRProcess.bBitsArray[MBUS_BYTE_DST] >> MBUS_BIT_DST) & 0x0F) == MBUS_OUR_ADDRESS)
			{
				MBusStatus.bPinged = 1;                                     // we are pinged, dont send PowerOn ping any more

																			// which command is that
				switch ((MBRProcess.bBitsArray[MBUS_BYTE_CMD] >> MBUS_BIT_CMD) & 0x0F)
				{
				case MBUS_CMD_PING:                                       // 0x8
					DEBUG_OUTP(PSTR("Ping\n"));
					AddTask(TSK_MBUS_SEND_TASK, MBUS_CMD_PING);           // Send ping answer
																		  // We must fast answer here (actually, not fast, it is almost the same time)
																		  // SWTLoad(SWT_MBUS_ANSWER_TIMER, SWT_MBUS_ANSWER_FAST_TIMER_VAL);
					MBusStatus.bPinged = 1;
					MBusStatus.bMBusPresent = 1;
					break;

				case MBUS_CMD_WAKEUP:                                     // 0x9
#ifdef MBUS_RECEIVE_DEBUG
																		  //  DEBUG_OUTP( PSTR("Wakeup cmd\n") );
#endif
																		  // Send some cmds to Head Unit
					AddTask(TSK_MBUS_SEND_TASK, MBUS_STATUS_ANSWER);
					AddTask(TSK_MBUS_SEND_TASK, MBUS_STATUS1_ANSWER);
					AddTask(TSK_MBUS_SEND_TASK, MBUS_CMD_CDINFO);
					AddTask(TSK_MBUS_SEND_TASK, MBUS_CMD_PLAY_STATUS);
					// and a answer timeout
					// SWTLoad(SWT_MBUS_ANSWER_TIMER, SWT_MBUS_ANSWER_SLOW_TIMER_VAL);
					break;

				case MBUS_CMD_CONTROL:                                    // 0x1
					switch ((MBRProcess.bBitsArray[MBUS_BYTE_SUB_CMD] >> MBUS_BIT_SUB_CMD) & 0x0F)
					{
					case MBUS_CMD_CTRL_PLAY:                              // 0x1
					{
						u08 bPlayCtrl = (((MBRProcess.bBitsArray[MBUS_BYTE_SUB_DATA0] >> MBUS_BIT_SUB_DATA0) & 0x0F) << 4) |
							(((MBRProcess.bBitsArray[MBUS_BYTE_SUB_DATA1] >> MBUS_BIT_SUB_DATA1) & 0x0F));
						switch (bPlayCtrl)
						{
						case MBUS_CMD_CTRL_PLAY_PLAY:                     // 0x01
#ifdef MBUS_RECEIVE_DEBUG
																		  //  DEBUG_OUTP( PSTR("Play cmd\n") );
#endif
							MBusStatus.ePlayState = PS_PREPARING;
							// Some cmds to HU
							AddTask(TSK_MBUS_SEND_TASK, MBUS_CMD_PLAY_STATUS);
							AddTask(TSK_MBUS_SEND_TASK, MBUS_CMD_CDINFO);
							AddTask(TSK_MBUS_SEND_TASK, MBUS_MAX_CMDS + UART_CMD_PLAY); // cmd to unilink, to start a playback
							MBusStatus.bAcceptUnilinkCmds = 0;

							//SWTLoad(SWT_MBUS_ANSWER_TIMER, SWT_MBUS_ANSWER_SLOW_TIMER_VAL);
							SetMainMode(MM_PLAYER);
							bDontSendSpinUp = 1;
							break;

						case MBUS_CMD_CTRL_PLAY_STOP:                     // 0x60
#ifdef MBUS_RECEIVE_DEBUG
																		  //  DEBUG_OUTP( PSTR("Stop cmd\n") );
#endif
							MBusStatus.ePlayState = PS_STOPPED;
							//SWTLoad(SWT_MBUS_ANSWER_TIMER, SWT_MBUS_ANSWER_SLOW_TIMER_VAL);
							AddTask(TSK_MBUS_SEND_TASK, MBUS_CMD_PLAY_STATUS);

							MBusStatus.bAcceptUnilinkCmds = 0;
							if (!MBus_UnilinkCmd(UART_CMD_STOP))
							{
								AddTask(TSK_MBUS_SEND_TASK, MBUS_MAX_CMDS + UART_CMD_STOP); // cmd to unilink, to stop playback
							}
							// cmd to HU as acknowledge

							break;

						case MBUS_CMD_CTRL_PLAY_FF:                       // 0x04
#ifdef MBUS_RECEIVE_DEBUG
																		  //  DEBUG_OUTP( PSTR("FF cmd\n") );
#endif                                          // it will be next track
							MBusStatus.bAcceptUnilinkCmds = 0;
							// AddTask( TSK_MBUS_SEND_TASK, MBUS_CMD_SPINUP_STATUS );
							// MBusStatus.ePlayState = PS_PREPARING;
							AddTask(TSK_MBUS_SEND_TASK, MBUS_MAX_CMDS + UART_CMD_NEXT_TRK);
							bDontSendSpinUp = 1;
							SWTClear(SWT_MBUS_CMD_DELAY_TIMER);
							break;

						case MBUS_CMD_CTRL_PLAY_REW:                      // 0x08
#ifdef MBUS_RECEIVE_DEBUG
																		  //  DEBUG_OUTP( PSTR("Rew cmd\n") );
#endif                                          // it will be prev track
							MBusStatus.bAcceptUnilinkCmds = 0;
							// AddTask( TSK_MBUS_SEND_TASK, MBUS_CMD_SPINUP_STATUS );
							// MBusStatus.ePlayState = PS_PREPARING;
							AddTask(TSK_MBUS_SEND_TASK, MBUS_MAX_CMDS + UART_CMD_PREV_TRK);
							bDontSendSpinUp = 1;
							SWTClear(SWT_MBUS_CMD_DELAY_TIMER);
							break;
						}
					}
					break;
					case MBUS_CMD_CTRL_SEEK:                              // here are Next Track, Prev Track, Next Disk, Prev Disk cmds are calced out
																		  //if ( !SWTEnabled(SWT_MBUS_CMD_DELAY_TIMER) )        // do not accep any furser seek cmd about 16 seconds
					{
						//SWTLoad( SWT_MBUS_CMD_DELAY_TIMER, SWT_MBUS_CMD_DELAY_TIMER_VAL );
						MBus_ProcessSeekCmd();                            // Process seek cmd
																		  // SWTLoad(SWT_MBUS_ANSWER_TIMER, SWT_MBUS_ANSWER_SLOW_TIMER_VAL);
						bDontSendSpinUp = 1;
					}
					break;
					}
					break;

				default:
#ifdef MBUS_RECEIVE_DEBUG
					DEBUG_OUTP(PSTR("Unk cmd, Ack\n"));
#endif
					AddTask(TSK_MBUS_SEND_TASK, MBUS_CMD_ACK);
					break;
				}
			}
		}
		else
		{
#ifdef MBUS_RECEIVE_DEBUG
			// DEBUG_OUTP( PSTR("MBus - Bad CRC\n") );
#endif
		}
		MBRProcess.bReceived = 0;                                    // all cmds processed
		MBRProcess.bBitRecState = MBR_WAITING;                          // waiting for next one
	}

	if (MBusStatus.bAcceptUnilinkCmds == 0)
	{
		// MBRProcess.bCanReceive = 0;
	}

	if (MBusStatus.ePlayState == PS_PLAYING)                        // do we in playing state
	{
		if (bOldStatus == PS_PREPARING)                               // Playing state was just setted
		{                                                               // Send some info to HU about disk configuration
																		// SWTLoad(SWT_MBUS_ANSWER_TIMER, SWT_MBUS_ANSWER_SLOW_TIMER_VAL);
			AddTask(TSK_MBUS_SEND_TASK, MBUS_STATUS_ANSWER);
			AddTask(TSK_MBUS_SEND_TASK, MBUS_CMD_SPINUP_STATUS);
			//AddTask( TSK_MBUS_SEND_TASK, MBUS_PAUSE_CMD );
			AddTask(TSK_MBUS_SEND_TASK, MBUS_CMD_CDINFO);
			//AddTask( TSK_MBUS_SEND_TASK, MBUS_PAUSE_CMD );
			// AddTask( TSK_MBUS_SEND_TASK, MBUS_CMD_CDINFO );  // Sometimes Headunit dont get it. Send it twice.
		}

		if (SWTFlag(SWT_MBUS_PLAY_STATUS_TIMER))                      // is there a time to send a Playing status to HU
		{                                                               // send it
			SWTLoad(SWT_MBUS_PLAY_STATUS_TIMER, SWT_MBUS_PLAY_STATUS_TIMER_VAL);
			AddTask(TSK_MBUS_SEND_TASK, MBUS_CMD_PLAY_STATUS);
		}
		else                                                            // Timer is not yet startet, but we are playing
		{
			if (!SWTEnabled(SWT_MBUS_PLAY_STATUS_TIMER))
			{
				SWTLoad(SWT_MBUS_PLAY_STATUS_TIMER, SWT_MBUS_PLAY_STATUS_TIMER_VAL);                   // start it, and send state now
			}
		}
	}
	bOldStatus = MBusStatus.ePlayState;

	// Send Tasks here
	nTaskDone = 0;
	bCurrentTask = GetNextTask(TSK_MBUS_SEND_TASK);                   // get next send task

	if (bCurrentTask > 0)                                           // is there something to send
	{
		if (bCurrentTask > MBUS_MAX_CMDS)                             // is it cmd to unilink
		{
			if (MBus_UnilinkCmd(bCurrentTask - MBUS_MAX_CMDS))         // send it
			{                                                             // is it sended
				nTaskDone = 1;                                              // erase that task
			}
		}
		else
		{
			/*
			if (bCurrentTask == MBUS_PAUSE_CMD )
			{
			//SWTLoad( SWT_MBUS_ANSWER_TIMER, SWT_MBUS_ANSWER_PAUSE_TIMER_VAL );
			nTaskDone = 1;                                                // erase that task
			}
			*/
			if (MBus_PreparePacket(bCurrentTask))                   // it is cmd to HU
			{                                                               // is it prepared (answer timeout)

				MBus_SendPacket();                                            // send it and
				nTaskDone = 1;                                                // erase that task
			}
		}
	}

	if (nTaskDone)                                                     // if task is done
		DoneTask(TSK_MBUS_SEND_TASK);                                    // delete it from the queue

																		 // End of Send Tasks

																		 // Check power down event
#ifndef NO_SLEEP
	if (MBus_Present())                                             // Only if MBus is present
	{
		if ((MBUS_PINS & (1 << MBUS_ACC_PIN)) != 0)               // Is ACC On
		{
			gFlags.PowerDown = 0;                                         // do not sleep
		}

		if (SWTFlag(SWT_MBUS_POWER_OFF_TIMER))                      // Deboncing of ACC Pin timer
		{
			SWTClear(SWT_MBUS_POWER_OFF_TIMER);
			if ((MBUS_PINS & (1 << MBUS_ACC_PIN)) == 0)             // Check it again
			{
				gFlags.PowerDown = 1;                                       // ACC is still off - sleeping.
			}
		}
		else if ((MBUS_PINS & (1 << MBUS_ACC_PIN)) == 0)          // If ACC Off
		{
			if (!SWTEnabled(SWT_MBUS_POWER_OFF_TIMER))                // Start Debouncing Timer, if not yet stated
			{
				SWTLoad(SWT_MBUS_POWER_OFF_TIMER, SWT_MBUS_POWER_OFF_TIMER_VAL);
			}
		}
	}
#endif
	// End of Check power down event

}

/*
*  Processing Seek Cmd
*/
void MBus_ProcessSeekCmd(void)
{
	// Calc Requested Disk
	u08 bReqDisk = ((MBRProcess.bBitsArray[MBUS_BYTE_SUB_DATA0] >> MBUS_BIT_SUB_DATA0) & 0x0F);
	// Calc Requested Track
	u08 bReqTrack = (((MBRProcess.bBitsArray[MBUS_BYTE_SUB_DATA1] >> MBUS_BIT_SUB_DATA1) & 0x0F) * 10) +
		((MBRProcess.bBitsArray[MBUS_BYTE_SUB_DATA2] >> MBUS_BIT_SUB_DATA2) & 0x0F);
	// Calc current played disk
	u08 bCurDisk = (((MBusStatus.bDiskNum  % MBUS_MAX_DISKS) != 0 ? (MBusStatus.bDiskNum  % MBUS_MAX_DISKS) : MBUS_MAX_DISKS));
	// Calc current played track
	u08 bCurTrack = (((MBusStatus.bTrackNum % 99) != 0 ? (MBusStatus.bTrackNum % 99) : 99));

#ifdef MBUS_RECEIVE_DEBUG
	// DEBUG_OUTP( PSTR("MBus - Set %u disk, %u track, Cur Disk: %u, Cur Track: %u\n"), bReqDisk, bReqTrack, bCurDisk, bCurTrack );
#endif

	MBusStatus.bAcceptUnilinkCmds = 0;                                // Do not accept any cmds from Unilink modul for change preiod
	if (bReqTrack != 0)                                             // Is it request to set track
	{
		//MBusStatus.ePlayState = PS_PREPARING;                         // We are preparing

		AddTask(TSK_MBUS_SEND_TASK, MBUS_CMD_SPINUP_STATUS);
		u08 dMaxTracks = GetFileCount() > 99 ? 99 : GetFileCount();   // from 1 to n  not from 0!!

																	  // define direction of change
		if (
			((bCurTrack == dMaxTracks) && (bReqTrack == 1)) ||
			(
			((bReqTrack  > bCurTrack) && (bCurTrack  < dMaxTracks)) &&
				(!((bReqTrack == dMaxTracks) && (bCurTrack == 1)))
				)
			)
		{                                                             // Next Track
#ifdef MBUS_RECEIVE_DEBUG
																	  // DEBUG_OUTP( PSTR("MBus - Next track\n"));
#endif
																	  // Send it to unilink
			AddTask(TSK_MBUS_SEND_TASK, MBUS_MAX_CMDS + UART_CMD_NEXT_TRK);
		}
		else                                                          // Prev Track
		{
			if (bReqTrack == bCurTrack)
			{
#ifdef MBUS_RECEIVE_DEBUG
				// DEBUG_OUTP( PSTR("MBus - Cur track from begin\n"));
#endif
				// Send it to unilink
				AddTask(TSK_MBUS_SEND_TASK, MBUS_MAX_CMDS + UART_CMD_CURRENT_TRK);
			}
			else
			{
#ifdef MBUS_RECEIVE_DEBUG
				// DEBUG_OUTP( PSTR("MBus - Prev track\n"));
#endif
				// Send it to unilink
				AddTask(TSK_MBUS_SEND_TASK, MBUS_MAX_CMDS + UART_CMD_PREV_TRK);
			}
		}
	}
	else                                                              // request to set disk
	{
		MBusStatus.ePlayState = PS_PREPARING;                         // We are preparing
		ClearAllTasks(TSK_MBUS_SEND_TASK);                          // Clear all sendings to HU
		AddTask(TSK_MBUS_SEND_TASK, MBUS_STATUS_ANSWER);
		//AddTask( TSK_MBUS_SEND_TASK, MBUS_PAUSE_CMD );
		// define direction of change
		if (
			((bCurDisk == MBUS_MAX_DISKS) && (bReqDisk == 1)) ||  // 6 -> 1
			(
			((bReqDisk  > bCurDisk) && (bCurDisk  < MBUS_MAX_DISKS)) &&      // 1 -> 2 -> 3 -> 4 -> 5 -> 6
				(!((bReqDisk == MBUS_MAX_DISKS) && (bCurDisk == 1)))
				)
			)
		{                                                             // Next disk

			if (MBusStatus.bDiskNum < MBUS_MAX_DISKS)                   // Calc it now for cmd to HU
				MBusStatus.bDiskNum++;
			else
				MBusStatus.bDiskNum = 1;

#ifdef MBUS_RECEIVE_DEBUG
			// DEBUG_OUTP( PSTR("MBus - Next disk: %u\n"), MBusStatus.bDiskNum);
#endif
			// Send it to unilink
			AddTask(TSK_MBUS_SEND_TASK, MBUS_MAX_CMDS + UART_CMD_NEXT_DSC);
		}
		else                                                          // Prev Disk
		{
			if (MBusStatus.bDiskNum > 1)                                // Calc it now for cmd to HU
				MBusStatus.bDiskNum--;
			else
				MBusStatus.bDiskNum = MBUS_MAX_DISKS;

#ifdef MBUS_RECEIVE_DEBUG
			// DEBUG_OUTP( PSTR("MBus - Prev disk %u\n"), MBusStatus.bDiskNum);
#endif
			// Send it to unilink
			AddTask(TSK_MBUS_SEND_TASK, MBUS_MAX_CMDS + UART_CMD_PREV_DSC);
		}
	}
}

/*
*  Preparing a send packet with right data
*/
u08 MBus_PreparePacket(u08 bCmd)
{
	u08 bBitsCount = 0;
	u08 bCrc;
	u08 bRes = 0;
	if (MBTProcess.bSending == 0 && SWTFlag(SWT_MBUS_ANSWER_TIMER))               // we are not sended, and answer delay is timeouted
	{
		memset((u08 *)&(MBTProcess.bBitsArray[0]), 0, MBUS_MAX_PACKET_LEN);         // reset data buffer
		MBTProcess.bBitsArray[MBUS_BYTE_DST] |= MBUS_HEAD_ADDRESS << MBUS_BIT_DST;    // set dst address
		MBTProcess.bBitsArray[MBUS_BYTE_CMD] |= bCmd << MBUS_BIT_CMD;                 // set cmd

																					  // Calc Disk and Track number for HU
		u08 bCurDisk = BCD(((MBusStatus.bDiskNum  % MBUS_MAX_DISKS) != 0 ? (MBusStatus.bDiskNum  % MBUS_MAX_DISKS) : MBUS_MAX_DISKS)); // + 1 ???? sink about it
		u08 bCurTrack = BCD(((MBusStatus.bTrackNum % 99) != 0 ? (MBusStatus.bTrackNum % 99) : 99));

		switch (bCmd)                                                             // check, what data to send
		{
		case MBUS_CMD_POWER_ON_PING:
			MBTProcess.bBitsArray[MBUS_BYTE_DST] = MBUS_HEAD_ADDRESS << MBUS_BIT_DST;    // set dst address
			MBTProcess.bBitsArray[MBUS_BYTE_CMD] |= 0x8 << MBUS_BIT_CMD;                  // set cmd
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 1] = 0x10;
			bBitsCount = 12;                                                        // Packet data lengt
			break;
		case MBUS_CMD_PING:                                                       // 0x8 for ping send nothing                                        
			MBTProcess.bBitsArray[MBUS_BYTE_DST] = MBUS_HEAD_ADDRESS << MBUS_BIT_DST;    // set dst address
			MBTProcess.bBitsArray[MBUS_BYTE_CMD] |= 0x8 << MBUS_BIT_CMD;                  // set cmd
			bBitsCount = 8;                                                         // Packet data lengt
			break;

		case MBUS_CMD_ACK:                                                        // 0x2 -> 0xF for acknowledge 
			MBTProcess.bBitsArray[MBUS_BYTE_CMD] = 0xF << MBUS_BIT_CMD;
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 3] = 0x10;
			bBitsCount = 8 + 20;
			break;

		case MBUS_CMD_CDINFO:                                                     // 0xC  Cd Info
			MBTProcess.bBitsArray[MBUS_BYTE_CDINFO_DISK_N] |= bCurDisk << MBUS_BIT_CDINFO_DISK_N;

			if (MBusStatus.ePlayState == PS_PLAYING)                             // do we in playback
			{                  // from 1 to n  not from 0 !
				u08 bcdMaxTracks = BCD((MBusStatus.bMaxTracks > 99 ? 99 : MBusStatus.bMaxTracks));
				MBTProcess.bBitsArray[MBUS_BYTE_CDINFO_1st_TRACK_H] |= 0 << MBUS_BIT_CDINFO_1st_TRACK_H;   // First track is allways 1
				MBTProcess.bBitsArray[MBUS_BYTE_CDINFO_1st_TRACK_L] |= 1 << MBUS_BIT_CDINFO_1st_TRACK_L;
				MBTProcess.bBitsArray[MBUS_BYTE_CDINFO_Lst_TRACK_H] |= (bcdMaxTracks >> 4) << MBUS_BIT_CDINFO_Lst_TRACK_H;   // Last Track
				MBTProcess.bBitsArray[MBUS_BYTE_CDINFO_Lst_TRACK_L] |= (bcdMaxTracks & 0x0F) << MBUS_BIT_CDINFO_Lst_TRACK_L;
				MBTProcess.bBitsArray[MBUS_BYTE_CDINFO_MINUTES_H] |= 9 << MBUS_BIT_CDINFO_MINUTES_H;     // Max Time is allways 99:99
				MBTProcess.bBitsArray[MBUS_BYTE_CDINFO_MINUTES_L] |= 9 << MBUS_BIT_CDINFO_MINUTES_L;
				MBTProcess.bBitsArray[MBUS_BYTE_CDINFO_SECUNDES_H] |= 9 << MBUS_BIT_CDINFO_SECUNDES_H;
				MBTProcess.bBitsArray[MBUS_BYTE_CDINFO_SECUNDES_L] |= 9 << MBUS_BIT_CDINFO_SECUNDES_L;
				MBTProcess.bBitsArray[MBUS_BYTE_CDINFO_CD_IN] |= 15 << MBUS_BIT_CDINFO_CD_IN;         // CD is allways insterted
			}
			bBitsCount = 8 + 40;
			break;

		case MBUS_WAKEUP_ANSWER:                                                  // 0xF
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 1] = 0x00;
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 2] = 0x56;
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 3] = 0x68;
			bBitsCount = 8 + 24;
			break;

		case MBUS_STATUS_ANSWER:                                                  // 0xB          
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 1] = 0x90 | bCurDisk;
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 2] = 0;//(bCurTrack << 4) | (bCurTrack & 0x0F);
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 3] = (MBusStatus.ePlayState == PS_PREPARING ? 1 : 0) << 4;
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 4] = 0x00;
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 5] = (MBusStatus.ePlayState == PS_PREPARING ? 1 : 0xC) << 4;;
			bBitsCount = 8 + 36;
			break;

		case MBUS_STATUS1_ANSWER:                                                 // 0xD
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 1] = 0x12;                      // wich disks are present
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 2] = 0x34;                      // will be on on the display ???
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 3] = 0x56;
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 4] = 0x00;
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 5] = 0x00;
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 6] = 0x00;
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 7] = 0xFC;
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 8] = 0x00;
			bBitsCount = 8 + 60;
			break;

		case MBUS_CMD_PLAY_STATUS:                                                // 0x9 PlayBack info
		{
			switch (MBusStatus.ePlayState)
			{
			case PS_PLAYING:                                                      // we are playing
			{
				u08 bCurMin = BCD(MBusStatus.bMinutes);
				u08 bCurSec = BCD(MBusStatus.bSeconds);
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 1] |= 0x40 | ((bCurTrack & 0xF0) >> 4);
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 2] |= ((bCurTrack & 0x0F) << 4) | ((bCurDisk & 0xF0) >> 4);
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 3] |= ((bCurDisk & 0x0F) << 4) | ((bCurMin & 0xF0) >> 4);
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 4] |= ((bCurMin & 0x0F) << 4) | ((bCurSec & 0xF0) >> 4);
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 5] |= ((bCurSec & 0x0F) << 4) | (MBusStatus.eRepeatMode);
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 6] |= ((MBusStatus.ePlayState == PS_MIX ? 2 : (MBusStatus.ePlayState == PS_SCAN ? 8 : 0)) << 4);
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 7] |= ((MBusStatus.ePlayState == PS_PLAYING ? 1 : (MBusStatus.ePlayState == PS_PAUSED ? 2 : (MBusStatus.ePlayState == PS_STOPPED ? 8 : 0))) << 4); // 
			}
			break;
			case PS_PREPARING:                                                    // we are searching something to play
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 1] |= 0x50 | ((bCurTrack & 0xF0) >> 4);
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 2] |= ((bCurTrack & 0x0F) << 4) | ((bCurDisk & 0xF0) >> 4);
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 3] |= bCurDisk << 4;
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 7] = 0xC0;
				break;
			case PS_STOPPED:
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 1] = 0x00;
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 3] = bCurDisk << 4;
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 7] = 0xC0;
				break;
			default:                                                              // in other cases
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 3] |= bCurDisk << 4;
				MBTProcess.bBitsArray[MBUS_BYTE_CMD + 7] = 0xC0;
				break;
			}
			bBitsCount = 52 + 8;
		}
		break;
		case MBUS_CMD_SPINUP_STATUS:                                              // We are spinning up disks ;-), i.e. loading Play Lists
			MBTProcess.bBitsArray[MBUS_BYTE_DST] = MBUS_HEAD_ADDRESS << MBUS_BIT_DST;    // set dst address
			MBTProcess.bBitsArray[MBUS_BYTE_CMD] |= 0x9 << MBUS_BIT_CMD;                  // set cmd
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 1] |= 0x50 | ((bCurTrack & 0xF0) >> 4);
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 2] |= ((bCurTrack & 0x0F) << 4) | ((bCurDisk & 0xF0) >> 4);
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 3] |= bCurDisk << 4;
			MBTProcess.bBitsArray[MBUS_BYTE_CMD + 7] = 0xC0;
			bBitsCount = 52 + 8;
			break;

		}

		if (bBitsCount)                                                             // is there data to send
		{
			SWTClear(SWT_MBUS_ANSWER_TIMER);                                            // clear answer timer, not to answer one more time

			bCrc = MBus_MakeCheckSumm(bBitsCount, MBTProcess.bBitsArray);             // calc crc for that packet
																					  // store crc
			MBTProcess.bBitsArray[bBitsCount / 8] |= (bCrc << (4 - (bBitsCount % 8)));
			bBitsCount += 4;                                                            // add crc lenght
			MBTProcess.bBitsCount = bBitsCount;                                         // store packet length for Transmit process
		}

		bRes = 1;                                                                     // Data preparing is done
	}
	return(bRes);
}

/*
*  Calc crc for the packet
*/
u08 MBus_MakeCheckSumm(u08 bBitCnt, u08 * bBitsArray)
{
	u08 i;
	u08 bNibbles;
	u08 bCheckSum = 0;
	bNibbles = bBitCnt / 4 + ((bBitCnt % 4) == 0 ? 0 : 1);

	for (i = 0; i < bNibbles; i++)
	{
		bCheckSum ^= ((bBitsArray[i / 2] >> (4 - (4 * (i % 2)))) & 0x0F);
	}
	bCheckSum = (bCheckSum + 1) & 0x0F;
	return(bCheckSum);
}

/*
*  Compose a cmd to unilink module
*/
u08 MBus_UnilinkCmd(u08 bCmd)
{
	u08 bRes = 0;
	Tucb * pCmdBuf = (Tucb *)UnilinkGetRecBuffer();         // get a buffer

	if (Unilink_GetReceiveState() == UR_SYNC_1)           // Is buffer blocked
	{
		Unilink_StopReceive();                                // Block it
		pCmdBuf->Cmd = bCmd;                                  // store cmd

		switch (bCmd)                                       // sotre a data for some cmds
		{
		case UART_CMD_STATUS:
			pCmdBuf->Data.ULStatus.Val.Bits.playing = !(MBusStatus.ePlayState == PS_STOPPED);
			pCmdBuf->Data.ULStatus.Val.Bits.ShowMode = UL_SHM_PLAY_INFO;
			break;
		case UART_CMD_PLAY:
			break;
		case UART_CMD_STOP:
			break;
		}
		// here we do not need to make crcs and other things, thus we are know, what is there
		AddTask(TSK_MAIN_TASK, MT_UNILINK_PROCESS_RECV_DATA); // Start to process unilink data - May be direct call of UnilinkProcessReceivedData is better ????
		bRes = 1;                                             // Cmd is sended and already processed
	}
	return(bRes);
}

/*
*   process cmd from unilink module
*/
void MBus_ProcessUnilinkCmd(u08 * pBuff)
{
	static u08 bOldTrackNum = 1;

	Tucb * pCmdBuf = (Tucb *)pBuff;
#ifdef MBUS_RECEIVE_UNILINK_CMD_DEBUG
	// DEBUG_OUTP( PSTR("MBus - Unilink Cmd %u\n"), pCmdBuf->Cmd );
#endif
	if (MBusStatus.bAcceptUnilinkCmds)
	{
		MBRProcess.bCanReceive = 1;

		switch (pCmdBuf->Cmd)
		{
		case UART_CMD_STATUS:                                 // Status request
															  // AddTask( TSK_MBUS_SEND_TASK, MBUS_MAX_CMDS + UART_CMD_STATUS );   // Send cmd to unilink
			break;
		case UART_CMD_SET_TIME:                               // Playback info from player, store it in M-Bus module

#ifdef MBUS_RECEIVE_UNILINK_CMD_DEBUG
															  // DEBUG_OUTP( PSTR("MBus - Unilink Cmd %u. Playing %u, ExtStatus %u\n"), pCmdBuf->Cmd, UnilinkGetPlayingState(), UnilinkGetExtStatus() );
#endif
			if (UnilinkGetPlayingState())                                   // do we in a Play mode
			{
				MBusStatus.ePlayState = PS_PLAYING;                             // Set flag of it
				if (UnilinkGetExtStatus() == UXS_LOAD_PLAYLIST)
				{
					MBusStatus.ePlayState = PS_PREPARING;                         // Set flag of it
				}
			}
			else
			{
				MBusStatus.ePlayState = PS_STOPPED;                             // no, we do not playing
			}


			// Store Plaback info
			MBusStatus.bDiskNum = pCmdBuf->Data.TimeInfo.bDiskNum;

			MBusStatus.bTrackNum = pCmdBuf->Data.TimeInfo.bTrackNum;

			MBusStatus.bMinutes = pCmdBuf->Data.TimeInfo.bMin;
			MBusStatus.bSeconds = pCmdBuf->Data.TimeInfo.bSec;
			MBusStatus.bMaxDisks = PL_GetPlayListCount();
			MBusStatus.bMaxTracks = GetFileCount();

			break;
		}
		/*
		if ( bOldTrackNum != MBusStatus.bTrackNum )
		{
		if( !bDontSendSpinUp)
		{
		AddTask( TSK_MBUS_SEND_TASK, MBUS_CMD_SPINUP_STATUS );
		}
		bDontSendSpinUp = 0;
		}
		*/
	}
	bOldTrackNum = MBusStatus.bTrackNum;
}

/*
*  Sens Stop cmd to unilink modul and sets MBus internal state of it
*/
void MBus_StopPlayBack(void)
{
	MBusStatus.ePlayState = PS_STOPPED;
	AddTask(TSK_MBUS_SEND_TASK, MBUS_MAX_CMDS + UART_CMD_STOP); // cmd to unilink, to stop playback
}



