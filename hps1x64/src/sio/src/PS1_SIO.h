/*
	Copyright (C) 2012-2030

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef _PS1_SIO_H_
#define _PS1_SIO_H_

#include "types.h"
#include "Debug.h"

#include "WinJoy.h"
#include "DJoySticks.h"

#include "PS1_Intc.h"

#include <stdio.h>
#include <sys/stat.h>

#ifdef PS2_COMPILE

#include "PS2_SIF.h"

#endif



#define ENABLE_SBUS_INT_SIO


namespace Playstation1
{

	class SIO
	{
	
		static Debug::Log debug;
		
	public:
	
		static SIO *_SIO;
		
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the sio registers start at
		static const long Regs_Start = 0x1f801050;
		
		// where the sio registers end at
		static const long Regs_End = 0x1f80105e;
	
		// distance between numbered groups of registers for sio
		static const long Reg_Size = 0x4;
		
		static const int c_iPadCycles = 32 * 13;
		static const int c_iCardCycles = 32 * 9;
		
		
		// index of next event
		u32 NextEvent_Idx;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;
		
		void GetNextEvent ();
		
		// set offset to next event
		void SetNextEvent ( u64 Cycle );
		
		// set cycle# of next event
		void Set_NextEventCycle ( u64 Cycle );
		
		// update events
		void Update_NextEventCycle ();
		
		static u32 Read ( u32 Address );
		static void Write ( u32 Address, u32 Data, u32 Mask );
		
		void Set_CTRL0 ( u32 Data );
		
#ifdef PS2_COMPILE
		// sector size for a ps2 memory card (in bytes?)
		static const int c_iPS2_MemCard_SectorSize = 512;
		
		// it looks like there are 16 sectors in a block ??
		static const int c_iPS2_MemCard_SectorsPerBlock = 16;

		// playstation 2 memory cards are 8MB each
		//static const int c_iPS2_MemoryCard_Size = 8388608;
		//static const int c_iPS2_MemCard_SectorSize = 512 + 16;
		static const int c_iPS2_MemoryCard_Size = ( 512 + 16 ) * 16384;
		u8 PS2MemoryCard0 [ c_iPS2_MemoryCard_Size ];
		u8 PS2MemoryCard1 [ c_iPS2_MemoryCard_Size ];

		void Load_PS2MemoryCardFile ( string FileName, int MemoryCard_Slot );
		void Store_PS2MemoryCardFile ( string FileName, int MemoryCard_Slot );

		// for live testing with 0, looks like 16, 16, 16 works for card, and 512,512 for pad

		static const int c_iSIO2_EventCycles_FastCard = 64;	//16;
		static const int c_iSIO2_EventCycles_MedCard = 256;	//16;	//1024;
		static const int c_iSIO2_EventCycles_SlowCard = 1024;	//32;	//16;	//16384;
		
		// a value of like 64 does not work well, but larger values like maybe 2048 seem to give better results???
		static const int c_iSIO2_EventCycles_Pad = 512;	//1500;
		
		static const int c_iSIO2_EventCycles_MultiTap = 512;	//512;
		
		// PS2 IOP clock is 1.088 times the PS1 clock, so 1.088*13=14, 14*2=28
		static const int c_iSIO2_PadPacketSizeMult = 32*128;
		static const int c_iSIO2_CardPacketSizeMult = 8;
		static const int c_iSIO2_PacketSizeMult = 16;
		
		u32 PadTransferCount, MultiTapTransferCount, CardSlowTransferCount, CardMedTransferCount, CardFastTransferCount;
		
		u32 lLastCommandType;
		//enum {
		//static const int LC_NONE = 0;
		// for PAD, seems to like ranges of 32*4 - 32*?, with 32*13 solving some problems at 32*5 ??
		// setting PAD to high like 32*32 or 32*64 causes issues
		static const int LC_PAD = 32 * 13;	//32*13;
		static const int LC_MULTITAP = 32*13;	//32*5;	//32;
		static const int LC_REMOTE = 32*13;	//32*5;	//32;
		static const int LC_CARD_FAST = 8;
		static const int LC_CARD_MED = 16;
		static const int LC_CARD_SLOW = 32*1;
		//	};
		
		u64 SIO2Multiplier;
		u64 TotalPacketCount;
		
		u64 CommandTime;

		static u32 DMA_ReadBlock ( u32* pMemoryPtr, u32 Address, u32 WordCount );	//( u8* Data, int ByteCount );
		static u32 DMA_WriteBlock ( u32* pMemoryPtr, u32 Address, u32 WordCount );	//( u8* Data, int ByteCount );
		
		void Set_CTRL2 ( u32 Data );
		
		u64 NextSIO2_Cycle;
#endif
		
		void Reset ();
		
		void Start ();
		
		// returns interrupt signal
		void Run ();
		
		void Handle_ControlPadInput ();
		

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////

		// SIO0 data
		static const long SIO0_DATA = 0x1f801040;
		
		// SIO0 status
		static const long SIO0_STAT = 0x1f801044;
		
		// SIO0 mode
		static const long SIO0_MODE = 0x1f801048;
		
		// SIO0 control
		static const long SIO0_CTRL = 0x1f80104a;
		
		// SIO0 baud
		static const long SIO0_BAUD = 0x1f80104e;

		
		// SIO1 data
		static const long SIO1_DATA = 0x1f801050;
		
		// SIO1 status
		static const long SIO1_STAT = 0x1f801054;
		
		// SIO1 mode
		static const long SIO1_MODE = 0x1f801058;
		
		// SIO1 control
		static const long SIO1_CTRL = 0x1f80105a;
		
		// SIO1 baud
		static const long SIO1_BAUD = 0x1f80105e;
		
		
#ifdef PS2_COMPILE

		// ps2 memory card
		// average read/write speed: 130 kbits/sec

		// PS2 Memory Card format //
		// I don't know anything about this. This is just ripped from "PlayStation 2 Memory Card File System" by "Ross Ridge"
		struct PS2_MC
		{
			// default value: "Sony PS2 Memory Card Format "
			char magic [ 28 ];
			
			// default value: "Version 1.2" ??
			char version [ 12 ];
			
			// default value: 512
			u16 page_len;
			
			// default value: 2
			u16 pages_per_cluster;
			
			// default value: 16
			u16 pages_per_block;
			
			// default value: 0xff00
			u16 unk0;
			
			// default value: 8192
			u32 clusters_per_card;
			
			// default value: 41
			u32 alloc_offset;
			
			// default value: 8135
			u32 alloc_end;
			
			// default value: 0
			u32 rootdir_cluster;
			
			// default value: 1023
			u32 backup_block1;
			
			// default value: 1022
			u32 backup_block2;
			
			// default value: 8 ??
			u32 ifc_list [ 32 ];
			
			// default value: -1 ??
			u32 bad_block_list [ 32 ];
			
			// default value: 2
			u8 card_type;
			
			// default value: 0x52
			u8 card_flags;
		};

		// SIO2 Registers //
		
		// SIO2 FIFO IN
		static const long SIO2_FIFO_IN = 0x1f808260;
		
		// SIO2 FIFO OUT
		// this one is read-only, so is only output from FIFO
		static const long SIO2_FIFO_OUT = 0x1f808264;
		
		// SIO2 CTRL
		static const long SIO2_CTRL = 0x1f808268;
		
		// SIO2 RECV1
		static const long SIO2_RECV1 = 0x1f80826c;
		
		// SIO2 RECV2
		static const long SIO2_RECV2 = 0x1f808270;
		
		// SIO2 RECV3
		static const long SIO2_RECV3 = 0x1f808274;
		
		// SIO2 8278
		static const long SIO2_8278 = 0x1f808278;
		
		// SIO2 827C
		static const long SIO2_827C = 0x1f80827c;
		
		// SIO2 INTR
		static const long SIO2_INTR = 0x1f808280;
		
		u32 SIO2_CTRL_Reg;
		
		u32 SIO2_8278_Reg;
		u32 SIO2_827c_Reg;
		u32 SIO2_INTR_Reg;
		
		u32 SendIndex;
		u32 ReceiveIndex;
		u32 SendSize;
		u32 CommandPort;
		u32 CommandLength;
		u32 PacketSize;
		u32 TransferSize;
		
		u32 InputCount;
		
		//u32 InputIndex;
		//u8 InputBuffer [ 2048 ];
		
		u32 OutputReadIndex, OutputWriteIndex;
		//u8 OutputBuffer [ 16384 ];
		
		static const int c_iSIO2OutputBuffer_Size = 16384;
		static const int c_iSIO2OutputBuffer_Mask = c_iSIO2OutputBuffer_Size - 1;
		
		u8 SIO2_OutputBuffer [ c_iSIO2OutputBuffer_Size ];
		u32 SIO2_OutputIndex;
		u32 isDmaTransfer;
		
		// ???
		u32 recvVal1, recvVal2, recvVal3;
		u32 recvVal1ret;
		
		u32 Array0 [ 16 ], Array1 [ 8 ];	//, Array2 [ 4 ];
		
		bool SIO2_InputReady, SIO2_OutputReady;

		static u64 SIO2in_DMA_Ready ();
		static u64 SIO2out_DMA_Ready ();

		void SIO2_FifoInput ( u8 Data );
		u8 SIO2_FifoOutput ();
		
		bool SIO2_InterruptPending;
		u32 StartOfInput;
		u32 ResponseSize;
		
		u32 MemoryCardSector;
		u32 MemoryCardIndex;
		u32 MemCardTransferSize;
		u32 LastCommand;
		
		// check if this is correct to check that data was sent/received correctly
		u8 MemoryCardXorValue;
		
		u64 SIO2_Cycles;
		
		// the memory card command terminator can be changed
		u8 cTerminator;
		
		// this should be called before sending a new command to SIO2
		inline void StartNewCommand () { StartOfInput = true; }

		static void PS2_FormatMemCard ( PS2_MC* mc );
#endif

		// constructor
		//SIO ();

		
		void Execute ();

		
		// Debug
		static u32 *_DebugPC;
		static u64 *_DebugCycleCount;
		static u64 *_SystemCycleCount;
		static u32 *_NextEventIdx;
		
		static Joysticks Joy;
		static DJoySticks DJoy;
		
		u32 DATA0, STAT0, MODE0, CTRL0, BAUD0;
		u32 DATA1, STAT1, MODE1, CTRL1, BAUD1;
		u32 WaitCycles0, WaitCycles1;
		u32 BusyCycles0, BusyCycles1;
		u32 PreScaler0, PreScaler1;
		u32 DataIn, DataOut;
		u32 isDataIn, isDataOut;
		u32 ControlPad_State, MemoryCard_State;
		u32 Command;
		//u8 Pad_Output_Buf [ 20 ], Card_Output_Buf [ 2048 ];
		u8 Output_Buf [ 2048 ];
		//u32 SizeOf_Pad_Output_Buf, SizeOf_Card_Output_Buf;
		u32 SizeOf_Output_Buf;
		
		u32 Interrupt;
		u32 BusyCycles;
		
		// the first byte sent at start of command
		u8 CurrentDevice;
		
		// current state counter
		u32 State;
		
		// any pad should be able to map to any port on the system
		u32 PortMapping [ 4 ];
		
		// joystick defaults
		static const u32 Key_X_default = 0x1, Key_O_default = 0x2, Key_Triangle_default = 0x3, Key_Square_default = 0x0,
				Key_Start_default = 9, Key_Select_default = 8, Key_L1_default = 0x4, Key_L2_default = 0x6, Key_L3_default = 10,
				Key_R1_default = 0x5, Key_R2_default = 0x7, Key_R3_default = 11;
		
		// joystick configuration - bitmaps that say what key to use on joystick
		// might make these static
		static const int c_iMaxNumPlayers = 2;
		static s32 Key_X [ c_iMaxNumPlayers ],
		Key_O [ c_iMaxNumPlayers ],
		Key_Triangle [ c_iMaxNumPlayers ],
		Key_Square [ c_iMaxNumPlayers ],
		Key_Start [ c_iMaxNumPlayers ],
		Key_Select [ c_iMaxNumPlayers ],
		Key_R1 [ c_iMaxNumPlayers ],
		Key_R2 [ c_iMaxNumPlayers ],
		Key_R3 [ c_iMaxNumPlayers ],
		Key_L1 [ c_iMaxNumPlayers ],
		Key_L2 [ c_iMaxNumPlayers ],
		Key_L3 [ c_iMaxNumPlayers ];
		
		enum { AXIS_X = 0, AXIS_Y, AXIS_Z, AXIS_R, AXIS_U, AXIS_V };
		static s32 LeftAnalog_X  [ c_iMaxNumPlayers ],
		LeftAnalog_Y  [ c_iMaxNumPlayers ],
		RightAnalog_X  [ c_iMaxNumPlayers ],
		RightAnalog_Y  [ c_iMaxNumPlayers ];
		
		//u64 POV_Up, POV_Down, POV_Right, POV_Left;

		// type of control pad for each controller
		s32 ControlPad_Type [ 2 ];
		enum {
		PADTYPE_DIGITAL = 0,
		PADTYPE_ANALOG,
#ifdef PS2_COMPILE
		PADTYPE_DUALSHOCK2
#endif
		};
		
		u8 DataIn_Buffer [ 2048 ];
		
		u8 XOR_code;
		
		// rumble values
		u8 aa [ 2 ], bb [ 2 ], cc [ 2 ], dd [ 2 ], ee [ 2 ], ff [ 2 ];
		u8 lMotorSmall [ 2 ], lMotorLarge [ 2 ];
		
		u32 DigitalID_ExtraHalfwordCount [ 2 ];
		
		// playstation 1 memory cards are 128KB each
		static const int c_iMemoryCard_Size = 131072;
		u32 MemoryCard_Offset;
		u8 MemoryCard0 [ 131072 ];
		u8 MemoryCard1 [ 131072 ];
		
		// a memory card can be connected or disconnected
		s32 MemoryCard_ConnectionState [ 2 ];
		enum { MCD_CONNECTED = 0, MCD_DISCONNECTED };
		
		static void Create_MemoryCardFile ( const char* FileName );
		void Load_MemoryCardFile ( string FileName, int MemoryCard_Slot );
		void Store_MemoryCardFile ( string FileName, int MemoryCard_Slot );
		

		void Update_PreScaler ();
		void Update_WaitCycles ();

		
		// need to know if a pad is in config mode or not
		u32 isConfigMode [ 2 ];
		
		
		// Command 0x42 - read pad/vibrate
		void Command_0x42 ( bool ForceAnalogOutput = false );
		
		
		// SIO0_TX_DATA (W) - 0x1f801040
		// bits 0-7 : Data to be sent
		// bits 8-31 : Not used
		
		struct SIO0_DATA_WFORMAT
		{
			// bits 0-7
			u32 Data : 8;
			
			// bits 8-31
			u32 NotUsed : 24;
		};
		
		// SIO0_TX_DATA (R) - 0x1f801040
		// usually only has 1 byte in fifo, so preview data is typically useless
		// bits 0-7 : Received data
		// bits 8-31 : Preview data
		
		struct SIO0_DATA_RFORMAT
		{
			// bits 0-7
			u32 Data : 8;
			
			// bits 8-15
			u32 Preview1 : 8;
			
			// bits 16-23
			u32 Preview2 : 8;
			
			// bits 24-31
			u32 Preview3 : 8;
		};
		
		
		// SIO0_STAT (R) - 0x1f801044
		// read-only
		
		struct SIO0_STAT_RFORMAT
		{
			// bit 0 - tx ready flag 1 (1: ready/started)
			u32 TxReady_Started : 1;
			
			// bit 1 - rx fifo not empty (0: empty; 1: not empty)
			u32 RxFIFO_NotEmpty : 1;
			
			// bit 2 - tx ready flag 2 (1: ready/finished)
			u32 TxReady_Finished : 1;
			
			// bit 3 - rx parity error (0: no; 1: error)
			u32 RxParity_Error : 1;
			
			// bits 4-6 - unknown (always zero)
			u32 Zero0 : 3;
			
			// bit 7 - ack input level (0: high; 1: low)
			u32 AckInputLevel : 1;
			
			// bit 8 - unknown (zero)
			u32 Zero1 : 1;
			
			// bit 9 - interrupt request (0: none; 1: IRQ7)
			u32 InterruptRequest : 1;
			
			// bit 10 - unknown (always zero)
			u32 Zero2 : 1;
			
			// bits 11-31 - baudrate timer (21bit timer, decrementing at 33MHz)
			u32 BaudrateTimer : 21;
		};

		
		// SIO0_MODE (R/W) (usually 0x000d meaning 8bit,no parity,MUL1) - 0x1f801048
		
		struct SIO0_MODE_RWFORMAT
		{
			// bits 0-1 : baudrate reload factor (0: MUL1; 1: MUL1; 2: MUL16; 3: MUL64)
			u32 PreScaler : 2;
			
			// bits 2-3 : character length (0: 5bits; 1: 6bits; 2: 7bits; 3: 8bits)
			u32 CharLength : 2;
			
			// bit 4 : parity enable (0: No; 1: Enable)
			u32 ParityEnable : 1;
			
			// bit 5 : parity type (0: even; 1: odd)
			u32 ParityType : 1;
			
			// bit 6-7 : unknown (always zero)
			u32 Zero0 : 2;
			
			// bit 8 : destroy received data (0: normal; 1: force data=0xff)
			u32 DestroyRcvdData : 1;
			
			// bits 9-15 : unknown (always zero)
			u32 Zero1 : 7;
		};
		
		
		// SIO0_CTRL (R/W) (usually 0x1003,0x3003,0x0000) - 0x1f80104a
		
		struct SIO0_CTRL_RWFORMAT
		{
			// bit 0 - tx enable (0: disable; 1: enable)
			u32 TxEnable : 1;
			
			// bit 1 - port output (0: high; 1: low/select)
			u32 PortSelectEnable : 1;
			
			// bit 2 - rx enable (0: normal, when portselectenable=low; 1: force enable once)
			u32 RxEnable : 1;
			
			// bit 3 - unknown (r/w)
			u32 Unknown0 : 1;
			
			// bit 4 - acknowledge (0: no change; 1: reset stat bits 3,9)
			u32 Acknowledge : 1;
			
			// bit 5 - unknown (r/w)
			u32 Unknown1 : 1;
			
			// bit 6 - reset (0: no change; 1: reset most sio0 registers to zero)
			u32 Reset : 1;
			
			// bit 7 - not used (always zero)
			u32 Zero0 : 1;
			
			// bits 8-9 - rx interrupt mode (0-3: IRQ when rx fifo contains 1,2,4,8 bytes)
			u32 RxInterruptMode : 2;
			
			// bit 10 - tx interrupt enable (0: disable; 1: enable) - when stat bit 0 or 2 ready
			u32 TxInterruptEnable : 1;
			
			// bit 11 - rx interrupt enable (0: disable; 1: enable) - when n bytes in rx fifo
			u32 RxInterruptEnable : 1;
			
			// bit 12 - ack interrupt enable (0: disable; 1: enable) - when stat bit 7 ack=low
			// like a "more-data-request" interrupt - looks like bios waits, then acknowledges previous interrupts, then reads.. then waits for this..
			// otherwise it times out
			u32 AckInterruptEnable : 1;
			
			// bit 13 - desired slot number (0: slot 0; 1: slot 1) (set to low when bit 1=1)
			u32 Slot : 1;
			
			// bits 14-15 - not used (always zero)
			u32 Zero1 : 2;
		};
		
		
		// SIO0_BAUD (R/W) (usually 0x0088 which means 250KHz when prescaler=MUL1) - 0x1f80104e
		// bits 0-15 : baudrate reload value for decrementing baudrate timer
		
		
		// Status Flags
		enum {
			STAT_TX_RDY = 0x0001,
			STAT_RX_RDY = 0x0002,
			STAT_TX_EMPTY = 0x0004,
			STAT_PARITY_ERR = 0x0008,
			STAT_RX_OVERRUN = 0x0010,
			STAT_FRAMING_ERR = 0x0020,
			STAT_SYNC_DETECT = 0x0040,
			STAT_DSR = 0x0080,
			STAT_CTS = 0x0100,
			STAT_IRQ = 0x0200
		};
		
		// when writing 0x2 to ctrl register, it means that pad0, and 0x2002 means pad1
		// 0x2/0x2002 also means memory card 0/1
		
		// command 0x01 means to start pad when written to data register
		// command 0x81 means to start memory card when written to data register

		// Control Flags when reading
		enum {
			CTRL_TX_PERM = 0x0001,
			CTRL_DTR = 0x0002,
			CTRL_RX_PERM = 0x0004,
			CTRL_BREAK = 0x0008,
			
			// mame says that this is an interrupt acknowledge
			//CTRL_RESET_ERR = 0x0010,
			CTRL_IACK = 0x0010,
			
			CTRL_RTS = 0x0020,
			CTRL_SIO_RESET = 0x0040,
			
			CTRL_TX_IENA = 0x400,
			CTRL_RX_IENA = 0x800,
			CTRL_DSR_IENA = 0x1000,
			
			CTRL_DTR2 = 0x2000
		};
		

		
		static void sRun () { _SIO->Run (); }
		static void Set_EventCallback ( funcVoid2 UpdateEvent_CB ) { _SIO->NextEvent_Idx = UpdateEvent_CB ( sRun ); };
		

		static u64* _NextSystemEvent;
		

		// for interrupt call back
		static funcVoid UpdateInterrupts;
		static void Set_IntCallback ( funcVoid UpdateInt_CB ) { UpdateInterrupts = UpdateInt_CB; };
		
		
		static const u32 c_InterruptBit_Controller = 7;
		static const u32 c_InterruptBit_SIO = 8;
		static const u32 c_InterruptBit_SIO2 = 17;
		//static const u32 c_InterruptBit_SIO2_test = 0x80;
		
		
		static u32* _Intc_Stat;
		static u32* _Intc_Mask;
		static u32* _R3000A_Status_12;
		static u32* _R3000A_Cause_13;
		static u64* _ProcStatus;
		
		inline void ConnectInterrupt ( u32* _IStat, u32* _IMask, u32* _R3000A_Status, u32* _R3000A_Cause, u64* _ProcStat )
		{
			_Intc_Stat = _IStat;
			_Intc_Mask = _IMask;
			_R3000A_Cause_13 = _R3000A_Cause;
			_R3000A_Status_12 = _R3000A_Status;
			_ProcStatus = _ProcStat;
		}
		
		
		inline void SetInterrupt_Controller ()
		{
			*_Intc_Stat |= ( 1 << c_InterruptBit_Controller );
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}
		
		inline void ClearInterrupt_Controller ()
		{
			*_Intc_Stat &= ~( 1 << c_InterruptBit_Controller );
			
			UpdateInterrupts ();
			
			/*
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}
		
		
		inline void SetInterrupt_SIO ()
		{
			*_Intc_Stat |= ( 1 << c_InterruptBit_SIO );
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}
		
		inline void ClearInterrupt_SIO ()
		{
			*_Intc_Stat &= ~( 1 << c_InterruptBit_SIO );
			
			UpdateInterrupts ();
			
			/*
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}


#ifdef PS2_COMPILE

		inline void SetInterrupt_SIO2 ()
		{
			*_Intc_Stat |= ( 1 << c_InterruptBit_SIO2 );	// | c_InterruptBit_SIO2_test;
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			
#ifdef ENABLE_SBUS_INT_SIO
			if ( *_Intc_Stat & Playstation2::SIF::_SIF->lSBUS_F230 )
			{
				// ??
				Playstation2::SIF::SetInterrupt_EE_SIF ();
			}
#endif
			*/

		}
		
		inline void ClearInterrupt_SIO2 ()
		{
			*_Intc_Stat &= ~( 1 << c_InterruptBit_SIO2 );
			
			UpdateInterrupts ();
			
			/*
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}
		
#endif
		
	};
	
};


void CreateMcd(const char *mcd);


#endif

