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



#include "PS1_SIO.h"
#include "WinApiHandler.h"

#ifdef PS2_COMPILE
#include "PS1_Dma.h"
#endif

using namespace Playstation1;


//#define PS2_FORCE_DIGITAL

//#define VERBOSE_DEBUG_SIO2_RUN


//#define VERBOSE_COMM


//#define ENABLE_DEFAULT_COMMAND_41

//#define ENABLE_CLEAR_CONFIG_MODE


// disable command 0x43 (controller read AND escape) when in config mode
#define DISABLE_COMMAND43_IN_CONFIG_MODE



// for testing a disconnected state
#define CONNECTION_VALUE 0x1100

#ifdef _DEBUG_VERSION_

#define INLINE_DEBUG_ENABLE
//#define INLINE_DEBUG

//#define INLINE_DEBUG_SPLIT

/*
#define INLINE_DEBUG_WRITE
#define INLINE_DEBUG_READ
#define INLINE_DEBUG_RUN
//#define INLINE_DEBUG_RUN_SIO2_TEST
#define INLINE_DEBUG_SIO2_DMA_READ
#define INLINE_DEBUG_SIO2_DMA_WRITE
//#define INLINE_DEBUG_UNKNOWN
#define INLINE_DEBUG_SIO2_RUN


#define INLINE_DEBUG_PAD
#define INLINE_DEBUG_RUN_MCD

#define INLINE_DEBUG_INPUT
#define INLINE_DEBUG_DATAOUT
#define INLINE_DEBUG_COMMAND
//#define INLINE_DEBUG_BREAK
//#define INLINE_DEBUG_OVERRUN


#define INLINE_DEBUG_CONFIGMODE
#define INLINE_DEBUG_SETMODE
#define INLINE_DEBUG_QUERYMODE
#define INLINE_DEBUG_QUERYACT
#define INLINE_DEBUG_QUERYCOMB
#define INLINE_DEBUG_QUERYMODE
#define INLINE_DEBUG_VIBRATION

#define INLINE_DEBUG_SIO2_DMA_READ_CONTENTS
#define INLINE_DEBUG_SIO2_DMA_WRITE_CONTENTS

//#define INLINE_DEBUG_PS2_MULTITAP
//#define INLINE_DEBUG_PS2_PAD

#define INLINE_DEBUG_CTRL2
*/


#endif



#define ENABLE_DIRECT_INPUT



funcVoid SIO::UpdateInterrupts;

u32 *SIO::_DebugPC;
u64 *SIO::_DebugCycleCount;
u64 *SIO::_SystemCycleCount;
u32* SIO::_NextEventIdx;

u32* SIO::_Intc_Stat;
u32* SIO::_Intc_Mask;
//u32* SIO::_Intc_Master;
u32* SIO::_R3000A_Status_12;
u32* SIO::_R3000A_Cause_13;
u64* SIO::_ProcStatus;


u64* SIO::_NextSystemEvent;


Debug::Log SIO::debug;

SIO *SIO::_SIO;


Joysticks SIO::Joy;
DJoySticks SIO::DJoy;


s32 SIO::Key_X [ SIO::c_iMaxNumPlayers ],
SIO::Key_O [ SIO::c_iMaxNumPlayers ],
SIO::Key_Triangle [ SIO::c_iMaxNumPlayers ],
SIO::Key_Square [ SIO::c_iMaxNumPlayers ],
SIO::Key_Start [ SIO::c_iMaxNumPlayers ],
SIO::Key_Select [ SIO::c_iMaxNumPlayers ],
SIO::Key_R1 [ SIO::c_iMaxNumPlayers ],
SIO::Key_R2 [ SIO::c_iMaxNumPlayers ],
SIO::Key_R3 [ SIO::c_iMaxNumPlayers ],
SIO::Key_L1 [ SIO::c_iMaxNumPlayers ],
SIO::Key_L2 [ SIO::c_iMaxNumPlayers ],
SIO::Key_L3 [ SIO::c_iMaxNumPlayers ];

s32 SIO::LeftAnalog_X [ SIO::c_iMaxNumPlayers ],
SIO::LeftAnalog_Y [ SIO::c_iMaxNumPlayers ],
SIO::RightAnalog_X [ SIO::c_iMaxNumPlayers ],
SIO::RightAnalog_Y [ SIO::c_iMaxNumPlayers ];


// these arrays are ripped from dfinput plugin //

static u8 stdpar[2][8] = {
	{0xFF, 0x5A, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80},
	{0xFF, 0x5A, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80}
};

static u8 unk46[2][8] = {
	{0xFF, 0x5A, 0x00, 0x00, 0x01, 0x02, 0x00, 0x0A},
	{0xFF, 0x5A, 0x00, 0x00, 0x01, 0x02, 0x00, 0x0A}
};

static const u8 unk47 [8] = { 0xf3, 0x5a, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00 };

static const u8 unk48 [8] = { 0xf3, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00 };


static u8 unk4c[2][8] = {
	{0xF3, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xF3, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static u8 unk4d[2][8] = { 
	{0xF3, 0x5A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xF3, 0x5A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

static u8 stdcfg[2][8]   = { 
	{0xFF, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFF, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static u8 stdmode[2][8]  = { 
	{0xFF, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFF, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static u8 stdmodel[2][8] = { 
	{0xFF,
	 0x5A,
	 0x01, // 03 - dualshock2, 01 - dualshock
	 0x02, // number of modes
	 0x01, // current mode: 01 - analog, 00 - digital
	 0x02,
	 0x01,
	 0x00},
	{0xFF, 
	 0x5A,
	 0x01, // 03 - dualshock2, 01 - dualshock
	 0x02, // number of modes
	 0x01, // current mode: 01 - analog, 00 - digital
	 0x02,
	 0x01,
	 0x00}
};


// use this for the led state (command 0x45)
// index 4 is where you return the led state (0x00 for off and 0x01 for on)
static const u8 led_state [8] = { 0xf3, 0x5a, 0x01, 0x02, 0x00, 0x02, 0x01, 0x00 };

// use this one for commands 0x40, 0x41, 0x49, 0x4a, 0x4b, 0x4e, 0x4f
static const u8 comzero [8]  = { 0xf3, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };



// ripped from dev9ghz pcsx2 plugin //

static unsigned char xor_table[256]={
 0x00, 0x87, 0x96, 0x11, 0xA5, 0x22, 0x33, 0xB4, 0xB4, 0x33, 0x22, 0xA5, 0x11, 0x96, 0x87, 0x00,
 0xC3, 0x44, 0x55, 0xD2, 0x66, 0xE1, 0xF0, 0x77, 0x77, 0xF0, 0xE1, 0x66, 0xD2, 0x55, 0x44, 0xC3,
 0xD2, 0x55, 0x44, 0xC3, 0x77, 0xF0, 0xE1, 0x66, 0x66, 0xE1, 0xF0, 0x77, 0xC3, 0x44, 0x55, 0xD2,
 0x11, 0x96, 0x87, 0x00, 0xB4, 0x33, 0x22, 0xA5, 0xA5, 0x22, 0x33, 0xB4, 0x00, 0x87, 0x96, 0x11,
 0xE1, 0x66, 0x77, 0xF0, 0x44, 0xC3, 0xD2, 0x55, 0x55, 0xD2, 0xC3, 0x44, 0xF0, 0x77, 0x66, 0xE1,
 0x22, 0xA5, 0xB4, 0x33, 0x87, 0x00, 0x11, 0x96, 0x96, 0x11, 0x00, 0x87, 0x33, 0xB4, 0xA5, 0x22,
 0x33, 0xB4, 0xA5, 0x22, 0x96, 0x11, 0x00, 0x87, 0x87, 0x00, 0x11, 0x96, 0x22, 0xA5, 0xB4, 0x33,
 0xF0, 0x77, 0x66, 0xE1, 0x55, 0xD2, 0xC3, 0x44, 0x44, 0xC3, 0xD2, 0x55, 0xE1, 0x66, 0x77, 0xF0,
 0xF0, 0x77, 0x66, 0xE1, 0x55, 0xD2, 0xC3, 0x44, 0x44, 0xC3, 0xD2, 0x55, 0xE1, 0x66, 0x77, 0xF0,
 0x33, 0xB4, 0xA5, 0x22, 0x96, 0x11, 0x00, 0x87, 0x87, 0x00, 0x11, 0x96, 0x22, 0xA5, 0xB4, 0x33,
 0x22, 0xA5, 0xB4, 0x33, 0x87, 0x00, 0x11, 0x96, 0x96, 0x11, 0x00, 0x87, 0x33, 0xB4, 0xA5, 0x22,
 0xE1, 0x66, 0x77, 0xF0, 0x44, 0xC3, 0xD2, 0x55, 0x55, 0xD2, 0xC3, 0x44, 0xF0, 0x77, 0x66, 0xE1,
 0x11, 0x96, 0x87, 0x00, 0xB4, 0x33, 0x22, 0xA5, 0xA5, 0x22, 0x33, 0xB4, 0x00, 0x87, 0x96, 0x11,
 0xD2, 0x55, 0x44, 0xC3, 0x77, 0xF0, 0xE1, 0x66, 0x66, 0xE1, 0xF0, 0x77, 0xC3, 0x44, 0x55, 0xD2,
 0xC3, 0x44, 0x55, 0xD2, 0x66, 0xE1, 0xF0, 0x77, 0x77, 0xF0, 0xE1, 0x66, 0xD2, 0x55, 0x44, 0xC3,
 0x00, 0x87, 0x96, 0x11, 0xA5, 0x22, 0x33, 0xB4, 0xB4, 0x33, 0x22, 0xA5, 0x11, 0x96, 0x87, 0x00};

// buffer should have 128 elements, returns 3 byte xor code in "xor"
//static void xfromman_call20_calculateXors(unsigned char buffer[128], unsigned char xor[4]){
static void xfromman_call20_calculateXors(u8* buffer, u8* xor_result){
	register unsigned char a=0, b=0, c=0, i;

	for (i=0; i<128; i++){
		a ^= xor_table[buffer[i]];
		if (xor_table[buffer[i]] & 0x80){
			b ^= ~i;
			c ^=  i;
		}
	}

	xor_result[0]=(~a) & 0x77;
	xor_result[1]=(~b) & 0x7F;
	xor_result[2]=(~c) & 0x7F;
}


/*
SIO::SIO ()
{
	cout << "Running SIO constructor...\n";
}
*/


void SIO::Start ()
{
	cout << "Running SIO::Start...\n";

#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create( "SIO_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering SIO::Start";
#endif


	Reset ();
	
	_SIO = this;

	STAT0 = STAT_TX_RDY | STAT_TX_EMPTY;
	STAT1 = STAT_TX_RDY | STAT_TX_EMPTY;
	
#ifndef ENABLE_DIRECT_INPUT
	Joy.InitJoysticks ();
#endif
	
	Key_X [ 0 ] = Key_X_default;
	Key_O [ 0 ] = Key_O_default;
	Key_Triangle [ 0 ] = Key_Triangle_default;
	Key_Square [ 0 ] = Key_Square_default;
	Key_Start [ 0 ] = Key_Start_default;
	Key_Select [ 0 ] = Key_Select_default;
	Key_R1 [ 0 ] = Key_R1_default;
	Key_R2 [ 0 ] = Key_R2_default;
	Key_R3 [ 0 ] = Key_R3_default;
	Key_L1 [ 0 ] = Key_L1_default;
	Key_L2 [ 0 ] = Key_L2_default;
	Key_L3 [ 0 ] = Key_L3_default;
	
	LeftAnalog_X [ 0 ] = AXIS_X;
	LeftAnalog_Y [ 0 ] = AXIS_Y;
	RightAnalog_X [ 0 ] = AXIS_R;
	RightAnalog_Y [ 0 ] = AXIS_Z;

#ifdef PS2_COMPILE
	// if the pressure sensitive buttons have not been enabled yet, then it should be just regular ps1 analog?
	//ControlPad_Type [ 0 ] = PADTYPE_DUALSHOCK2;
	//ControlPad_Type [ 1 ] = PADTYPE_DUALSHOCK2;
	ControlPad_Type [ 0 ] = PADTYPE_ANALOG;
	ControlPad_Type [ 1 ] = PADTYPE_ANALOG;
#else
	// the default should be to set both controllers to analog mode
	ControlPad_Type [ 0 ] = PADTYPE_ANALOG;
	ControlPad_Type [ 1 ] = PADTYPE_ANALOG;
#endif


	// initialize next event to far into future
	Set_NextEventCycle ( -1ULL );

	
	// output size of structure
	//cout << " SIO Class Size=" << sizeof ( Playstation1::SIO );
	
	// *** testing *** connect mcd0 but not mcd1
	//MemoryCard_ConnectionState [ 0 ] = MCD_CONNECTED;
	//MemoryCard_ConnectionState [ 1 ] = MCD_DISCONNECTED;
	
	
	// initially, controller0 is connected to port0 on the system and controller1 maps to port1
	PortMapping [ 0 ] = 0;
	PortMapping [ 1 ] = -1;
	
	// for some other time...
	PortMapping [ 2 ] = 2;
	PortMapping [ 3 ] = 3;

#ifdef PS2_COMPILE

	recvVal1 = 0x1D100;

#endif	


#ifdef INLINE_DEBUG
	debug << "->Exiting SIO::Start";
#endif
}


void SIO::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( SIO ) );

	lMotorSmall [ 0 ] = 0xff;
	lMotorLarge [ 0 ] = 0xff;
	lMotorSmall [ 1 ] = 0xff;
	lMotorLarge [ 1 ] = 0xff;
	
	aa [ 0 ] = 0xff;
	bb [ 0 ] = 0xff;
	cc [ 0 ] = 0xff;
	dd [ 0 ] = 0xff;
	ee [ 0 ] = 0xff;
	ff [ 0 ] = 0xff;
	aa [ 1 ] = 0xff;
	bb [ 1 ] = 0xff;
	cc [ 1 ] = 0xff;
	dd [ 1 ] = 0xff;
	ee [ 1 ] = 0xff;
	ff [ 1 ] = 0xff;
	
#ifdef PS2_COMPILE
	// must be the starting terminator??
	cTerminator = 0x55;
	
	// intialize memory cards for PS2 ??
	//PS2_FormatMemCard ( (PS2_MC*) PS2MemoryCard0 );
	//PS2_FormatMemCard ( (PS2_MC*) PS2MemoryCard1 );
	memset ( PS2MemoryCard0, 0xff, sizeof( PS2MemoryCard0 ) );
	memset ( PS2MemoryCard1, 0xff, sizeof( PS2MemoryCard1 ) );
	
	// calculate xor codes for both entire cards
	
	for ( int i = 0; i < 16384; i++ )
	{
		// recalculate the xor code for page
		memset ( & PS2MemoryCard0 [ ( i * ( 512 + 16 ) ) + 512 + 0 ], 0, 16 );
		xfromman_call20_calculateXors( & PS2MemoryCard0 [ ( i * ( 512 + 16 ) ) + 0 ], & PS2MemoryCard0 [ ( i * ( 512 + 16 ) ) + 512 + 0 ] );
		xfromman_call20_calculateXors( & PS2MemoryCard0 [ ( i * ( 512 + 16 ) ) + 128 ], & PS2MemoryCard0 [ ( i * ( 512 + 16 ) ) + 512 + 3 ] );
		xfromman_call20_calculateXors( & PS2MemoryCard0 [ ( i * ( 512 + 16 ) ) + 256 ], & PS2MemoryCard0 [ ( i * ( 512 + 16 ) ) + 512 + 6 ] );
		xfromman_call20_calculateXors( & PS2MemoryCard0 [ ( i * ( 512 + 16 ) ) + 384 ], & PS2MemoryCard0 [ ( i * ( 512 + 16 ) ) + 512 + 9 ] );
	}

	for ( int i = 0; i < 16384; i++ )
	{
		// recalculate the xor code for page
		memset ( & PS2MemoryCard1 [ ( i * ( 512 + 16 ) ) + 512 + 0 ], 0, 16 );
		xfromman_call20_calculateXors( & PS2MemoryCard1 [ ( i * ( 512 + 16 ) ) + 0 ], & PS2MemoryCard1 [ ( i * ( 512 + 16 ) ) + 512 + 0 ] );
		xfromman_call20_calculateXors( & PS2MemoryCard1 [ ( i * ( 512 + 16 ) ) + 128 ], & PS2MemoryCard1 [ ( i * ( 512 + 16 ) ) + 512 + 3 ] );
		xfromman_call20_calculateXors( & PS2MemoryCard1 [ ( i * ( 512 + 16 ) ) + 256 ], & PS2MemoryCard1 [ ( i * ( 512 + 16 ) ) + 512 + 6 ] );
		xfromman_call20_calculateXors( & PS2MemoryCard1 [ ( i * ( 512 + 16 ) ) + 384 ], & PS2MemoryCard1 [ ( i * ( 512 + 16 ) ) + 512 + 9 ] );
	}
	
#endif

}

#ifdef PS2_COMPILE

static constexpr char* c_sMagic = "Sony PS2 Memory Card Format ";
static constexpr char* c_sVersion = "1.2.0.0";
void SIO::PS2_FormatMemCard ( PS2_MC* mc )
{
	int i;
	
	for ( i = 0; i < 28; i++ )
	{
		mc -> magic [ i ] = c_sMagic [ i ];
	}
	
	for ( i = 0; i < 12; i++ )
	{
		mc -> version [ i ] = 0;
	}
	
	for ( i = 0; i < 7; i++ )
	{
		mc -> version [ i ] = c_sVersion [ i ];
	}
	
	mc -> page_len = 512;
	mc -> pages_per_cluster = 2;
	mc -> pages_per_block = 16;
	mc -> unk0 = 0xff00;
	mc -> page_len = 512;
	mc -> clusters_per_card = 8192;
	mc -> alloc_offset = 41;
	mc -> alloc_end = 8135;
	mc -> rootdir_cluster = 0;
	mc -> backup_block1 = 1023;
	mc -> backup_block2 = 1022;
	
	for ( i = 0; i < 32; i++ )
	{
		mc -> ifc_list [ i ] = 8;
	}
	
	for ( i = 0; i < 32; i++ )
	{
		mc -> bad_block_list [ i ] = -1;
	}
	
	mc -> card_type = 2;
	mc -> card_flags = 0x52;
}

#endif


// returns request for interrupt pad/card, or zero
// returns true if requesting interrupt
// also gives number of cycles before next event
void SIO::Execute ()
{
	static const u8 ControlPad_Digital_Output [] = { 0xff, 0x41, 0x5a };
	static const u8 ControlPad_AnalogGreen_Output [] = { 0xff, 0x53, 0x5a };
	static const u8 ControlPad_AnalogRed_Output [] = { 0xff, 0x73, 0x5a };
#ifdef PS2_COMPILE
	static const u8 ControlPad_DualShock2_Output [] = { 0xff, 0x79, 0x5a };
#endif
	
	static const u8 MemoryCard_Header_Output [] = { 0xff, 0x00, 0x5a, 0x5d, 0x00 };
	
	static u8* MemCardPtr;
	
	u32 i;

	Interrupt = false;
	BusyCycles = 0;

#ifdef INLINE_DEBUG_RUN
	debug << "\r\n\r\nSIO::Run; CycleCount=" << dec << *_DebugCycleCount << "; Pad_State=" << ControlPad_State << "; MCD_State=" << MemoryCard_State << hex << "; isDataIn=" << isDataIn << "; DataIn=" << DataIn;
	debug << "\r\n; STAT0=" << STAT0 << "; CTRL0=" << CTRL0 << "; MODE0=" << MODE0 << "; BAUD0=" << BAUD0 << "; isDataOut=" << isDataOut << "; DigitalID_ExtraHalfwordCount=" << DigitalID_ExtraHalfwordCount [ ( CTRL0 >> 13 ) & 1 ] << "\r\n";
#endif


#ifdef PS2_COMPILE

	if ( State )
	{
		SIO2Multiplier = c_iSIO2_PadPacketSizeMult;
		
		// determine what the current device is
		switch ( CurrentDevice )
		{
			case 0x61:
				CardSlowTransferCount++;
				lLastCommandType = LC_REMOTE;
				
				// output zeroes for now
				CommandTime = c_iSIO2_EventCycles_Pad;
				Output_Buf [ SizeOf_Output_Buf ] = 0xff;
				break;
				
			case 0x21:
				MultiTapTransferCount++;
				lLastCommandType = LC_MULTITAP;
				
				CommandTime = c_iSIO2_EventCycles_MultiTap;
				break;
				
			default:
				PadTransferCount++;
				lLastCommandType = LC_PAD;
				
				CommandTime = c_iSIO2_EventCycles_Pad;
				cout << "hps1x64: SIO: CurrentDevice not known. Device=" << hex << CurrentDevice;
				break;
		}
		
		return;
	}
	
#endif


	if ( ControlPad_State )
	{

#ifdef PS2_COMPILE
	// assume that control pad is present on ps2 for now
	//recvVal1 = 0x1100;
	recvVal1 = CONNECTION_VALUE;

		PadTransferCount++;
		lLastCommandType = LC_PAD;
		
		CommandTime = c_iSIO2_EventCycles_Pad;
		SIO2Multiplier = c_iSIO2_PadPacketSizeMult;
#endif
		
		if ( /*(!isDataOut) &&*/ ( ControlPad_State & 1 ) )
		{
			// handling the DataIn right now
			//isDataIn = false;
			
			// store input and go to next control pad state
			//DataIn_Buffer [ ControlPad_State++ ] = DataIn;
			
			// data has just been read, so it is time for a transfer
			//STAT0 &= ~( STAT_TX_RDY | STAT_TX_EMPTY );
			STAT0 |= STAT_TX_RDY | STAT_TX_EMPTY;
			
			// *** TESTING *** transfer has not started as it is complete at this point
			//STAT0 &= ~( STAT_TX_EMPTY );
			
			// *** TESTING *** mark receive as ready
			//STAT0 |= ( STAT_RX_RDY );
			
			// trigger interrupt here if either tx or dsr interrupt is enabled for now
			if ( ( CTRL0 & CTRL_RX_IENA ) || ( CTRL0 & CTRL_DSR_IENA ) )
			{
#ifdef INLINE_DEBUG_RUN
	debug << "; INT";
	debug << "\r\n(before) _Intc_Stat=" << hex << *_Intc_Stat << " _R3000A_Cause_13=" << *_R3000A_Cause_13 << " _R3000A_Status_12=" << *_R3000A_Status_12 << " _ProcStatus=" << *_ProcStatus;
#endif

				// signal that data is ready to be tranferred
				//SetInterrupt_Controller ();
				Interrupt = true;
				
				// just sent irq
				STAT0 |= STAT_IRQ;

#ifdef INLINE_DEBUG_RUN
	debug << "\r\n(after) _Intc_Stat=" << hex << *_Intc_Stat << " _R3000A_Cause_13=" << *_R3000A_Cause_13 << " _R3000A_Status_12=" << *_R3000A_Status_12 << " _ProcStatus=" << *_ProcStatus;
#endif
			}
			
			// go to next control pad state
			ControlPad_State++;
			
			return;
		}

		if ( isDataIn && !( ControlPad_State & 1 ) )
		{
			// data has now been transferred, so transfer buffer is empty
			// but this is already done when data is read
			//STAT0 &= ~( STAT_TX_RDY | STAT_TX_EMPTY );

			/*
			if ( !isDataIn )
			{
				// did not receive data -> request more data via interrupt
#ifdef INLINE_DEBUG_RUN
	debug << "; INT->DID NOT RECEIVE DATA";
#endif

				// signal that data is ready to be tranferred
				SetInterrupt_Controller ();
				
				// just sent irq
				STAT0 |= STAT_IRQ;
				
				// did not receive data
				return;
			}
			*/
			
			// save the input data for now
			// *** TODO *** handle commands sent
			DataIn_Buffer [ ( ControlPad_State >> 1 ) ] = DataIn;
			
#ifdef INLINE_DEBUG_INPUT
//	if ( Command == 0x43 )
//	{
//		debug << " " << hex << (u32) DataIn;
//	}
#endif
			
			// we have now handled the input
			isDataIn = false;
			
			if ( ControlPad_State == 2 )
			{
#ifdef INLINE_DEBUG_COMMAND
	debug << "\r\nCommand=" << hex << (u32) DataIn;
#endif

				// *** TODO *** COMMANDS: 0x42, 0x43, 0x44, 0x45, 0x46, 0x4c, 0x4d

				// set command
				Command = DataIn;

				// if digital pad, then only allow command 0x42
				/*
				if ( ControlPad_Type [ ( CTRL0 >> 13 ) & 1 ] == PADTYPE_DIGITAL )
				{
					cout << "\nCommand=" << hex << (u32)Command << ". Changing to 0x42. Cycle#" << dec << *_DebugCycleCount << "\n";
					Command = 0x42;
				}
				*/

				//cout << "\nCommand=" << hex << (u32)Command << ". Changing to 0x42. Cycle#" << dec << *_DebugCycleCount << "\n";
#ifdef VERBOSE_COMM
	cout << "\nhps1x64: SIO: Pad Command=" << hex << Command;
#endif

				// get the command
				switch ( Command /*DataIn*/ )
				{
					// Command 0x40 - set vref param ?
					case 0x40:
					
						for ( int i = 0; i < 8; i++ ) Output_Buf [ 1 + i ] = comzero [ i ];
						SizeOf_Output_Buf += 6;

						Output_Buf [ 1 + 2 ] = 0;
						Output_Buf [ 1 + 3 ] = 0;
						Output_Buf [ 1 + 4 ] = 2;
						Output_Buf [ 1 + 5 ] = 0;
						Output_Buf [ 1 + 6 ] = 0;
						
						// must end with 0x5a ?
						Output_Buf [ 1 + 7 ] = 0x5a;
						break;
						
					// Command 0x41 - query button mask?
					case 0x41:
//cout << "\nPS1:SIO:Command0x41\n";
						for ( int i = 0; i < 8; i++ ) Output_Buf [ 1 + i ] = comzero [ i ];
						SizeOf_Output_Buf += 6;
						
						// NOTE: in digital mode, this returns all zeroes
						// also only returns result if in config mode
						if ( ( ControlPad_Type [ ( CTRL0 >> 13 ) & 1 ] != PADTYPE_DIGITAL ) && ( isConfigMode [ ( CTRL0 >> 13 ) & 1 ] ) )
						{
							Output_Buf [ 1 + 2 ] = 0xff;
							Output_Buf [ 1 + 3 ] = 0xff;
							Output_Buf [ 1 + 4 ] = 3;
							Output_Buf [ 1 + 5 ] = 0;
						}
						
						
						// set xx to zero ?
						Output_Buf [ 1 + 6 ] = 0;
						
						// must end with 0x5a ?
						Output_Buf [ 1 + 7 ] = 0x5a;
						break;
						
					// Command 0x42 - read data and vibrate?
					case 0x42:
					

						// set the current command
						Command = 0x42;
						
#ifdef ENABLE_CLEAR_CONFIG_MODE
						// clear config mode
						isConfigMode [ ( CTRL0 >> 13 ) & 1 ] = 0;
						
						// send keys
						Command_0x42 ();
#else
						
						if ( isConfigMode [ ( CTRL0 >> 13 ) & 1 ] )
						{
							// config mode - force analog output //
							Command_0x42 ( true );
						}
						else
						{
							Command_0x42 ();
						}
#endif

#ifdef PS2_COMPILE
						// control pad is present
						//recvVal1 = 0x1100;
						recvVal1 = CONNECTION_VALUE;
#endif

						break;
						
					// Command 0x43 - enter config mode
					case 0x43:
#ifdef INLINE_DEBUG_CONFIGMODE
	debug << "\r\nConfigMode";
#endif

						Command = 0x43;
						
						
#ifdef DISABLE_COMMAND43_IN_CONFIG_MODE
						if ( isConfigMode [ ( CTRL0 >> 13 ) & 1 ] )
						{
							// config mode //
							
							// in config mode this returns 0xf3 0x5a 0x00 0x00...(rest zeros)
							for ( int i = 0; i < 8; i++ ) Output_Buf [ 1 + i ] = comzero [ i ];
							SizeOf_Output_Buf += 6;
							/*
							if ( ControlPad_Type [ 0 ] == PADTYPE_DIGITAL )
							{
								// digital controller //
								SizeOf_Output_Buf += 2;
							}
							else
							{
								// analog controller //
								SizeOf_Output_Buf += 6;
							}
							*/
						}
						else
#endif
						{
							// normal mode //
							
							// in normal mode, this returns the keys
							Command_0x42 ();
						}
						
						
						/*
						if ( stdcfg [ 0 ] [ 3 ] == 0xff )
						{
							// should output 0xf3 instead of control pad type indentifier
							Output_Buf [ 1 ] = 0xf3;
						}
						
						for ( int i = 2; i < 8; i++ ) Output_Buf [ 1 + i ] = stdcfg [ 0 ] [ i ];
						SizeOf_Output_Buf += 6;
						*/

						break;
						
					// Command 0x44 - set mode and lock
					case 0x44:
#ifdef INLINE_DEBUG_SETMODE
	debug << "\r\nSetMode";
#endif

						Command = 0x44;
						
						//cout << "\n***hps2x64: UNIMPLEMENTED PAD COMMAND? Command=" << hex << ((u32)Command) << "***\n";
						
						// sends 0x01 0x44 0x00 val sel 0x00...(rest zeros)
						// reply 0xf3 0x5a 0x00...(rest zeros)
						// where if sel is 0x02 then val is the new led state (0x00 for off and 0x01 for on)
						for ( int i = 0; i < 8; i++ ) Output_Buf [ 1 + i ] = comzero [ i ];
						SizeOf_Output_Buf += 6;

						break;
						
					// Command 0x45 - query model and mode
					case 0x45:
#ifdef INLINE_DEBUG_QUERYMODE
	debug << "\r\nQueryMode";
#endif

						Command = 0x45;

						// sends 0x01 0x45 0x00 0x00...(rest zeros)
						// reply HiZ 0xf3 0x5a 0x01 0x02 val 0x02 0x01 0x00
						// where val = 0x00 for off and 0x01 for on
						
						//for ( int i = 2; i < 8; i++ ) Output_Buf [ 1 + i ] = stdmodel [ 0 ] [ i ];
						for ( int i = 0; i < 8; i++ ) Output_Buf [ 1 + i ] = led_state [ i ];
						
						// put in the led state at index 4 in led_state array
						//if ( ControlPad_Type [ 0 ] == PADTYPE_DIGITAL )
						switch ( ControlPad_Type [ ( CTRL0 >> 13 ) & 1 ] )
						{
							case PADTYPE_DIGITAL:
							
#ifdef PS2_COMPILE
								// ps2 dual shock 2 controller
								Output_Buf [ 1 + 2 ] = 0x03;
#else
								// ps2 dual shock 1 controller
								Output_Buf [ 1 + 2 ] = 0x01;
#endif

								// LED on analog pad is off //
								Output_Buf [ 1 + 4 ] = 0x00;
								break;

							case PADTYPE_ANALOG:
							
#ifdef PS2_COMPILE
								// ps2 dual shock 2 controller
								Output_Buf [ 1 + 2 ] = 0x03;
#else
								// ps2 dual shock 1 controller
								Output_Buf [ 1 + 2 ] = 0x01;
#endif
								
								// LED on analog pad is on //
								Output_Buf [ 1 + 4 ] = 0x01;
								break;
								
#ifdef PS2_COMPILE
							case PADTYPE_DUALSHOCK2:
								// this must be a PS2 dual shock 2 controller //
								
								// ps2 dual shock 2 controller
								Output_Buf [ 1 + 2 ] = 0x03;
								
								// led is off ?? or on ?? //
								Output_Buf [ 1 + 4 ] = 0x01;
								break;
#endif
								
							default:
								cout << "\nhps1x64: ERROR: CONTROLPAD/SIO: Invalid Pad Type: " << dec << ControlPad_Type [ ( CTRL0 >> 13 ) & 1 ];
								break;
						}
						
						// *** testing *** set led off and digital controller
						/*
						if ( ! ( ( CTRL0 >> 13 ) & 1 ) )
						{
							// digital pad
							Output_Buf [ 3 ] = 0;
							
							// max 1 mode
							Output_Buf [ 4 ] = 1;
							
							// current mode digital
							Output_Buf [ 4 ] = 0;
						}
						*/
						
						// should output 0xf3 instead of control pad type indentifier
						//Output_Buf [ 1 ] = 0xf3;
						
						SizeOf_Output_Buf += 6;

						break;
						
					// Command 0x46 - query act??
					case 0x46:
#ifdef INLINE_DEBUG_QUERYACT
	debug << "\r\nQueryAct";
#endif

						Command = 0x46;

						// sends 0x01 0x46 0x00 xx 0x00...(rest zeros)
						// reply HiZ  0xf3 0x5a 0x00 0x00 yy yy yy yy
						// if xx=0x00 then yy= 0x01,0x02,0x00,0x0a, if xx=0x01 then yy= 0x01,0x01,0x01,0x14
				
						//cout << "\n***hps2x64: UNIMPLEMENTED PAD COMMAND? Command=" << hex << ((u32)Command) << "***\n";
						
						for ( int i = 0; i < 8; i++ ) Output_Buf [ 1 + i ] = comzero [ i ];
						SizeOf_Output_Buf += 6;
						
						/*
						for ( int i = 2; i < 8; i++ ) Output_Buf [ 1 + i ] = unk46 [ 0 ] [ i ];
						
						// should output 0xf3 instead of control pad type indentifier
						Output_Buf [ 1 ] = 0xf3;
						
						SizeOf_Output_Buf += 6;
						*/
						
						break;
						
						
					// Command 0x47 - query comb??
					case 0x47:
#ifdef INLINE_DEBUG_QUERYCOMB
	debug << "\r\nQueryComb";
#endif

						Command = 0x47;

						for ( int i = 0; i < 8; i++ ) Output_Buf [ 1 + i ] = unk47 [ i ];
						
						// should output 0xf3 instead of control pad type indentifier
						//Output_Buf [ 1 ] = 0xf3;
						
						SizeOf_Output_Buf += 6;
						
						break;
						
						
					case 0x48:
					
						for ( int i = 0; i < 8; i++ ) Output_Buf [ 1 + i ] = unk48 [ i ];
						SizeOf_Output_Buf += 6;
						break;
						
						
					// Command 0x4c - query mode??
					case 0x4c:
#ifdef INLINE_DEBUG_QUERYMODE
	debug << "\r\nQueryModeUnk";
#endif
						Command = 0x4c;
						
						//cout << "\n***hps2x64: UNIMPLEMENTED PAD COMMAND? Command=" << hex << ((u32)Command) << "***\n";
						
						for ( int i = 0; i < 8; i++ ) Output_Buf [ 1 + i ] = comzero [ i ];
						SizeOf_Output_Buf += 6;
						
						/*
						for ( int i = 2; i < 8; i++ ) Output_Buf [ 1 + i ] = unk4c [ 0 ] [ i ];
						
						// should output 0xf3 instead of control pad type indentifier
						Output_Buf [ 1 ] = 0xf3;
						
						SizeOf_Output_Buf += 6;
						*/

						break;
						
					// Command 0x4d - vibration toggle
					case 0x4d:
#ifdef INLINE_DEBUG_VIBRATION
	debug << "\r\nVibration";
#endif

						Command = 0x4d;
						
						for ( int i = 0; i < 8; i++ ) Output_Buf [ 1 + i ] = unk4d [ 0 ] [ i ];
						
/*						
#ifdef PS2_COMPILE
						Output_Buf [ 3 ] = 0xff;
						Output_Buf [ 4 ] = 0xff;
						Output_Buf [ 5 ] = 0xff;
						Output_Buf [ 6 ] = 0xff;
						Output_Buf [ 7 ] = 0xff;
						Output_Buf [ 8 ] = 0xff;
#else
*/

						// output the old values
						Output_Buf [ 3 ] = lMotorSmall [ ( CTRL0 >> 13 ) & 1 ];
						Output_Buf [ 4 ] = lMotorLarge [ ( CTRL0 >> 13 ) & 1 ];
						
						//Output_Buf [ 3 ] = aa [ ( CTRL0 >> 13 ) & 1 ];
						//Output_Buf [ 4 ] = bb [ ( CTRL0 >> 13 ) & 1 ];
						Output_Buf [ 5 ] = cc [ ( CTRL0 >> 13 ) & 1 ];
						Output_Buf [ 6 ] = dd [ ( CTRL0 >> 13 ) & 1 ];
						Output_Buf [ 7 ] = ee [ ( CTRL0 >> 13 ) & 1 ];
						Output_Buf [ 8 ] = ff [ ( CTRL0 >> 13 ) & 1 ];
//#endif
						
						
						SizeOf_Output_Buf += 6;
						
						// should output 0xf3 instead of control pad type indentifier
						//Output_Buf [ 1 ] = 0xf3;
						
						
						break;

						
					// set DS2 native mode
					// set button info ?
					case 0x4f:
					
						for ( int i = 0; i < 8; i++ ) Output_Buf [ 1 + i ] = comzero [ i ];
						SizeOf_Output_Buf += 6;
						
						// must end with 0x5a ?
						Output_Buf [ 1 + 7 ] = 0x5a;
						
#ifdef PS2_COMPILE
						// set controller as a dual shock 2 controller
						ControlPad_Type [ ( CTRL0 >> 13 ) & 1 ] = PADTYPE_DUALSHOCK2;
#endif
						break;
						
					// use this one for commands 0x40, 0x41, 0x49, 0x4a, 0x4b, 0x4e, 0x4f //
					
					// set vref param
					//case 0x40:
					
					// query ds2 analog mode
					//case 0x41:
					
					case 0x49:
					case 0x4a:
					case 0x4b:
					case 0x4e:
					
					// set DS2 native mode
					// set button info ?
					//case 0x4f:
					
						//cout << "\n***hps2x64: UNIMPLEMENTED PAD COMMAND? Command=" << hex << ((u32)Command) << "***\n";
						
						for ( int i = 0; i < 8; i++ ) Output_Buf [ 1 + i ] = comzero [ i ];
						SizeOf_Output_Buf += 6;
						break;

					default:
						cout << "\nhps1x64: ***ALERT***: SIO: Unknown control pad command=" << hex << Command << "\n";
						break;
				}
			}
			
			if ( ControlPad_State == 6 )
			{
				switch ( Command )
				{
					case 0x43:
					
						// this is checking the third value written as in sequence... 0x01 0x43 0x00 xx 0x00
						if ( DataIn_Buffer [ 3 ] == 0 )
						{
							// enter NORMAL mode //
							
							// just do pad1 for now
							//isConfigMode [ 0 ] = false;
							isConfigMode [ ( CTRL0 >> 13 ) & 1 ] = false;
							
							//stdcfg [ 0 ] [ 2 ] = 0;
							//stdcfg [ 0 ] [ 3 ] = 0;
							
							// ***testing*** changed this back for ps2 testing
							//Output_Buf [ 3 ] = 0xff;
							//Output_Buf [ 4 ] = 0xff;
						}
						else if ( DataIn_Buffer [ 3 ] == 1 )
						{
							// enter CONFIG mode //
							
							// just do pad1 for now
							//isConfigMode [ 0 ] = true;
							isConfigMode [ ( CTRL0 >> 13 ) & 1 ] = true;
							
							// for digital pad 2 & 3 should be zero, for analog should be 0xff
							// note: actually this is the keys being returned...
							//stdcfg [ 0 ] [ 2 ] = (u8) 0xff;
							//stdcfg [ 0 ] [ 3 ] = (u8) 0xff;
							
							// ***testing*** changed this back for ps2 testing
							//Output_Buf [ 3 ] = 0xff;
							//Output_Buf [ 4 ] = 0xff;
						}
						
						break;

						
					case 0x46:
					
						switch ( DataIn_Buffer [ 3 ] )
						{
							// default
							case 0:
								//unk46 [ 0 ] [ 5 ] = 0x02;
								//unk46 [ 0 ] [ 6 ] = 0x00;
								//unk46 [ 0 ] [ 7 ] = 0x0A;
								
								Output_Buf [ 5 ] = 0x01;
								Output_Buf [ 6 ] = 0x02;
								Output_Buf [ 7 ] = 0x00;
								Output_Buf [ 8 ] = 0x0a;
								
								break;

							// Param std conf change
							case 1:
								//unk46 [ 0 ] [ 5 ] = 0x01;
								//unk46 [ 0 ] [ 6 ] = 0x01;
								//unk46 [ 0 ] [ 7 ] = 0x14;
								
								Output_Buf [ 5 ] = 0x01;
								Output_Buf [ 6 ] = 0x01;
								Output_Buf [ 7 ] = 0x01;
								Output_Buf [ 8 ] = 0x14;
								
								break;
						}
						
						break;

						
					case 0x4c:
					
						switch ( DataIn_Buffer [ 3 ] )
						{
							case 0:
								// set mode to digital
								// element 5 should be zero if digital pad, 1 for analog pad
								//unk4c [ 0 ] [ 5 ] = 0;
								
								// for digital pad, output byte 6 is 0x4 for 0x4c command
								Output_Buf [ 6 ] = 0x04;
								
								break;
						
							case 1:
								// set mode to digital
								// element 5 should be zero if digital pad, 1 for analog pad
								//unk4c [ 0 ] [ 5 ] = 1;
								
								// for analog pad, output byte 6 is 0x7 for 0x4c command
								Output_Buf [ 6 ] = 0x07;
								
								break;
						}
						
						break;

				}
			}
			
			if ( ControlPad_State == 10 )
			{
				switch ( Command )
				{
					case 0x44:
					
						//cout << "\n***PadState0x44," << hex << "isConfigMode=" << isConfigMode [ ( CTRL0 >> 13 ) & 1 ] << ",pad#" << (( CTRL0 >> 13 ) & 1) << "," << (u32) DataIn_Buffer [ 3 ] << "," << (u32) DataIn_Buffer [ 4 ];
						// set led state //
						
#ifdef INLINE_DEBUG_SETMODE
	debug << "\r\nCommand=0x44: Pad#" << dec << (u32) ( ( CTRL0 >> 13 ) & 1 ) << " val=" << (u32) DataIn_Buffer [ 3 ] << " sel=" << (u32) DataIn_Buffer [ 4 ];
#endif
						
						// has to be in config mode
						if ( isConfigMode [ ( CTRL0 >> 13 ) & 1 ] )
						{

					
/*
							switch ( DataIn_Buffer [ 4 ] )
							{
								case 2:
#ifdef PS2_COMPILE
								// also if it is 3 ??
								case 3:
#endif
*/

									switch ( DataIn_Buffer [ 3 ] )
									{
										case 0:
#ifdef INLINE_DEBUG_SETMODE
	debug << "\r\nCommand 0x44: Set to DIGITAL";
#endif

											//ControlPad_Type [ ( CTRL0 >> 13 ) & 1 ] = PADTYPE_DUALSHOCK2;
											ControlPad_Type [ ( CTRL0 >> 13 ) & 1 ] = PADTYPE_DIGITAL;
											break;

										case 1:
#ifdef INLINE_DEBUG_SETMODE
	debug << "\r\nCommand 0x44: Set to ANALOG";
#endif

											//ControlPad_Type [ ( CTRL0 >> 13 ) & 1 ] = PADTYPE_DUALSHOCK2;
											ControlPad_Type [ ( CTRL0 >> 13 ) & 1 ] = PADTYPE_ANALOG;
											break;
									
										default:
#ifdef INLINE_DEBUG_SETMODE
	debug << "\r\nCommand=0x44: UNKNOWN COMBINATION2. Pad#" << dec << (u32) ( ( CTRL0 >> 13 ) & 1 ) << " val=" << (u32) DataIn_Buffer [ 3 ] << " sel=" << (u32) DataIn_Buffer [ 4 ];
#endif

											break;
									}


/*
									break;

									default:
#ifdef INLINE_DEBUG_SETMODE
	debug << "\r\nCommand=0x44: UNKNOWN COMBINATION1. Pad#" << dec << (u32) ( ( CTRL0 >> 13 ) & 1 ) << " val=" << (u32) DataIn_Buffer [ 3 ] << " sel=" << (u32) DataIn_Buffer [ 4 ];
#endif

										break;
										
							}	// end switch ( DataIn_Buffer [ 4 ] )
*/
						
						}	// end if ( isConfigMode [ ( CTRL0 >> 13 ) & 1 ] )
						
						/*
						if ( Output_Buf [ 4 ] == 2 )
						{
							// apply new led state //
							ControlPad_Type [ ( CTRL0 >> 13 ) & 1 ] = Output_Buf [ 3 ];
						}
						*/
					
						break;
						
					default:
						break;
						
				}
			}
			
			
			switch ( Command )
			{
				case 0x4d:
				
					switch ( ControlPad_State )
					{
						case 6:
							aa [ ( CTRL0 >> 13 ) & 1 ] = DataIn_Buffer [ 3 ];
							
							if ( DataIn_Buffer [ 3 ] == 0 ) lMotorSmall [ ( CTRL0 >> 13 ) & 1 ] = 0;
							if ( DataIn_Buffer [ 3 ] == 1 ) lMotorLarge [ ( CTRL0 >> 13 ) & 1 ] = 0;
							
							break;
							
						case 8:
							bb [ ( CTRL0 >> 13 ) & 1 ] = DataIn_Buffer [ 4 ];
							
							if ( DataIn_Buffer [ 4 ] == 0 ) lMotorSmall [ ( CTRL0 >> 13 ) & 1 ] = 1;
							if ( DataIn_Buffer [ 4 ] == 1 ) lMotorLarge [ ( CTRL0 >> 13 ) & 1 ] = 1;
							
							break;
							
						case 10:
							cc [ ( CTRL0 >> 13 ) & 1 ] = DataIn_Buffer [ 5 ];
							
							
							if ( DataIn_Buffer [ 5 ] == 0 ) lMotorSmall [ ( CTRL0 >> 13 ) & 1 ] = 2;
							if ( DataIn_Buffer [ 5 ] == 1 ) lMotorLarge [ ( CTRL0 >> 13 ) & 1 ] = 2;
							
							// ***todo*** if bits 1-7 of cc are zero, then transfer one extra halfword
							// also adds one to digital id (not analog id)
							DigitalID_ExtraHalfwordCount [ ( CTRL0 >> 13 ) & 1 ] = 0;
							if ( ! ( cc [ ( CTRL0 >> 13 ) & 1 ] & 0xfe ) ) DigitalID_ExtraHalfwordCount [ ( CTRL0 >> 13 ) & 1 ]++;
							break;
							
						case 12:
							dd [ ( CTRL0 >> 13 ) & 1 ] = DataIn_Buffer [ 6 ];
							
							if ( DataIn_Buffer [ 6 ] == 0 ) lMotorSmall [ ( CTRL0 >> 13 ) & 1 ] = 3;
							if ( DataIn_Buffer [ 6 ] == 1 ) lMotorLarge [ ( CTRL0 >> 13 ) & 1 ] = 3;
							
							break;
							
						case 14:
							ee [ ( CTRL0 >> 13 ) & 1 ] = DataIn_Buffer [ 7 ];
							
							
							if ( DataIn_Buffer [ 7 ] == 0 ) lMotorSmall [ ( CTRL0 >> 13 ) & 1 ] = 4;
							if ( DataIn_Buffer [ 7 ] == 1 ) lMotorLarge [ ( CTRL0 >> 13 ) & 1 ] = 4;
							
							// ***todo*** if bits 1-7 of ee are zero, then transfer one extra halfword
							// also adds one to digital id (not analog id)
							if ( ! ( ee [ ( CTRL0 >> 13 ) & 1 ] & 0xfe ) ) DigitalID_ExtraHalfwordCount [ ( CTRL0 >> 13 ) & 1 ]++;
							
							break;
							
						case 16:
							/*
							aa [ ( CTRL0 >> 13 ) & 1 ] = DataIn_Buffer [ 3 ];
							bb [ ( CTRL0 >> 13 ) & 1 ] = DataIn_Buffer [ 4 ];
							cc [ ( CTRL0 >> 13 ) & 1 ] = DataIn_Buffer [ 5 ];
							dd [ ( CTRL0 >> 13 ) & 1 ] = DataIn_Buffer [ 6 ];
							ee [ ( CTRL0 >> 13 ) & 1 ] = DataIn_Buffer [ 7 ];
							*/
							
							ff [ ( CTRL0 >> 13 ) & 1 ] = DataIn_Buffer [ 8 ];
							
							if ( DataIn_Buffer [ 8 ] == 0 ) lMotorSmall [ ( CTRL0 >> 13 ) & 1 ] = 5;
							if ( DataIn_Buffer [ 8 ] == 1 ) lMotorLarge [ ( CTRL0 >> 13 ) & 1 ] = 5;
							
							/*
							// ***todo*** if bits 1-7 of cc are zero, then transfer one extra halfword
							// also adds one to digital id (not analog id)
							DigitalID_ExtraHalfwordCount [ ( CTRL0 >> 13 ) & 1 ] = 0;
							if ( ! ( cc [ ( CTRL0 >> 13 ) & 1 ] & 0xfe ) ) DigitalID_ExtraHalfwordCount [ ( CTRL0 >> 13 ) & 1 ]++;
							
							// ***todo*** if bits 1-7 of ee are zero, then transfer one extra halfword
							// also adds one to digital id (not analog id)
							if ( ! ( ee [ ( CTRL0 >> 13 ) & 1 ] & 0xfe ) ) DigitalID_ExtraHalfwordCount [ ( CTRL0 >> 13 ) & 1 ]++;
							*/
							
							break;
							
						default:
							break;
							
					}
					
					break;
					
				default:
					break;
			}
					
			// send data out
			if ( ( ControlPad_State >> 1 ) < SizeOf_Output_Buf )
			{
				DataOut = Output_Buf [ ControlPad_State >> 1 ];
				
#ifdef INLINE_DEBUG_DATAOUT
		//debug << " #" << hex << ( ControlPad_State >> 1 ) << " In=" << (u32) DataIn_Buffer [ ControlPad_State >> 1 ] << " Out=" << (u32) Output_Buf [ ControlPad_State >> 1 ];
		debug << " " << (u32) Output_Buf [ ControlPad_State >> 1 ];
#endif

			}
			else
			{
				// I'll just output zeros if it keeps reading past end of the buffer
				DataOut = 0;
				
				// *** TODO *** need to set overrun bit if it keeps reading past end of buffer also
#ifdef INLINE_DEBUG_OVERRUN
	debug << "\r\nOverrun";
#endif

			}
			
			// data has been received from device and is ready for reading
			isDataOut = true;
				

			// go to next control pad state
			ControlPad_State++;
				
			// data has been received and is ready to be read
			STAT0 |= STAT_RX_RDY;
			
			// only interrupt if the rx interrupt is enabled here
			if ( CTRL0 & CTRL_TX_IENA )
			{
#ifdef INLINE_DEBUG_RUN
	debug << "; INT";
	debug << "\r\n(before) _Intc_Stat=" << hex << *_Intc_Stat << " _R3000A_Cause_13=" << *_R3000A_Cause_13 << " _R3000A_Status_12=" << *_R3000A_Status_12 << " _ProcStatus=" << *_ProcStatus;
#endif

				// signal that data is ready to be read
				//SetInterrupt_Controller ();
				Interrupt = true;
				
				// just sent irq
				STAT0 |= STAT_IRQ;
				
#ifdef INLINE_DEBUG_RUN
	debug << "\r\n(after) _Intc_Stat=" << hex << *_Intc_Stat << " _R3000A_Cause_13=" << *_R3000A_Cause_13 << " _R3000A_Status_12=" << *_R3000A_Status_12 << " _ProcStatus=" << *_ProcStatus;
#endif
			}
			
#ifdef INLINE_DEBUG_RUN
	debug << "; isDataOut=" << isDataOut << "; DataOut=" << DataOut;
#endif

			//return;
		}
		
		// set next event to happen when ps1 receives data from control pad
		//SetNextEvent ( c_iPadCycles );
		BusyCycles = c_iPadCycles;
		
		return;
	}



	if ( MemoryCard_State )
	{
//#ifdef INLINE_DEBUG_RUN_MCD
//	debug << "\r\n\r\nSIO::Run; CycleCount=" << dec << *_DebugCycleCount << "; Pad_State=" << ControlPad_State << "; MCD_State=" << MemoryCard_State << hex << "; isDataIn=" << isDataIn << "; DataIn=" << DataIn;
//	debug << "\r\n; STAT0=" << STAT0 << "; CTRL0=" << CTRL0 << "; MODE0=" << MODE0 << "; BAUD0=" << BAUD0 << "; isDataOut=" << isDataOut << "\r\n";
//#endif

#ifdef INLINE_DEBUG_RUN
	debug << "; ->MemoryCard_State; DataIn_Buffer [ 1 ]=" << (u32) DataIn_Buffer [ 1 ];
#endif

#ifdef PS2_COMPILE
		CardSlowTransferCount++;
		
		SIO2Multiplier = c_iSIO2_CardPacketSizeMult;
#endif
		
		if ( /*(!isDataOut) &&*/ ( MemoryCard_State & 1 ) )
		{
#ifdef INLINE_DEBUG_RUN
	debug << "; ->MemoryCard_State&1";
#endif

			// data has just been read, so it is time for a transfer
			STAT0 |= STAT_TX_RDY | STAT_TX_EMPTY;
			
			// data is also ready to be read
			//STAT0 |= STAT_RX_RDY;
			
			// trigger interrupt here if either tx or dsr interrupt is enabled for now
			if ( ( CTRL0 & CTRL_RX_IENA ) || ( CTRL0 & CTRL_DSR_IENA ) )
			{
#ifdef INLINE_DEBUG_RUN
	debug << "; INT";
#endif

				// signal that data is ready to be tranferred
				//SetInterrupt_Controller ();
				Interrupt = true;
				
				// just sent irq
				STAT0 |= STAT_IRQ;
			}
			
			// go to next control pad state
			MemoryCard_State++;
			
			return;
		}
		

		if ( isDataIn && !( MemoryCard_State & 1 ) )
		{
#ifdef INLINE_DEBUG_RUN
	debug << "; ->isDataIn&&!MemoryCard_State&1";
#endif

			// data has now been transferred, so transfer buffer is empty
			// but this is already done when data is read
			//STAT0 &= ~( STAT_TX_RDY | STAT_TX_EMPTY );
			
			
			// save the input data for now
			DataIn_Buffer [ ( MemoryCard_State >> 1 ) ] = DataIn;
			
			// we have now handled the input
			isDataIn = false;
			
#ifdef PS2_COMPILE
			CommandTime = c_iSIO2_EventCycles_SlowCard;
#endif
			
			// get the command
			switch ( DataIn_Buffer [ 1 ] )
			{
				// Command 0x57 - write data to memory card
				case 0x57:

#ifdef INLINE_DEBUG_RUN
	debug << "; ->MemoryCard WRITE";
#endif

#ifdef PS2_COMPILE

					lLastCommandType = LC_CARD_SLOW;
					
					if ( ( MemoryCard_State >> 1 ) < 137 )
					{
#ifdef INLINE_DEBUG_RUN
	debug << "; ->Blank";
#endif

						// for now, for PS2, just return 0xff with terminator
						Output_Buf [ MemoryCard_State >> 1 ] = 0xff;
					}
					
					if ( ( MemoryCard_State >> 1 ) >= ( ResponseSize - 2 ) )
					{
#ifdef INLINE_DEBUG_RUN
	debug << "; ->Terminator";
#endif

						// put in PS2 style terminator
						Output_Buf [ ResponseSize - 2 ] = '+';
						Output_Buf [ ResponseSize - 1 ] = cTerminator;
					}
					
#else

					// check which memory card this is for
					switch ( CTRL0 & 0x2002 )
					{
						// this means memory card 0
						case 0x2:
#ifdef INLINE_DEBUG_RUN
	debug << "; ->Card0";
#endif
						
							// when (MemoryCard_State>>1) == 135, then we need to fill in 0x5c, 0x5d, 0x47
							if ( ( MemoryCard_State >> 1 ) == 1 )
							{
#ifdef INLINE_DEBUG_RUN
	debug << "; ->Terminator";
#endif
								// the output data from the memory card is always good
								// this is just the end of the data from memory card in a write sequence that says all is good
								// so that means 138 bytes total get output from memory card during a write sequence
								Output_Buf [ 135 ] = 0x5c;
								Output_Buf [ 136 ] = 0x5d;
								Output_Buf [ 137 ] = 0x47;
								SizeOf_Output_Buf = 138;
							}
							
							// need to get the address to write to, then shift left two and write directly
							// high part of address is in byte 4
							// low part of address is in byte 5
							// when (MemoryCard_State>>1) == 135, then we need to fill in 0x5c, 0x5d, 0x47
							if ( ( MemoryCard_State >> 1 ) == 5 )
							{
#ifdef INLINE_DEBUG_RUN
	debug << "; ->GetAddress";
#endif

								MemoryCard_Offset = (u32) ( ( ( (u32) DataIn_Buffer [ 4 ] ) << 8 ) | (u32) DataIn_Buffer [ 5 ] ) << 7;
							}
							
							if ( ( ( MemoryCard_State >> 1 ) >= 5 ) && ( ( MemoryCard_State >> 1 ) < 135 ) )
							{
#ifdef INLINE_DEBUG_RUN
	debug << "; ->Output";
#endif
								// *** TODO *** make sure this is outputting the right data from memory card
								Output_Buf [ MemoryCard_State >> 1 ] = DataIn_Buffer [ ( MemoryCard_State >> 1 ) - 1 ];
							}
							
							if ( ( ( MemoryCard_State >> 1 ) >= 6 ) && ( ( MemoryCard_State >> 1 ) < 134 ) )
							{
#ifdef INLINE_DEBUG_RUN
	debug << "; ->ToCard";
#endif
								// this data gets output to the memory card
								MemoryCard0 [ MemoryCard_Offset++ ] = DataIn;
							}
							
							break;
							
						// this means memory card 1
						case 0x2002:
#ifdef INLINE_DEBUG_RUN
	debug << "; ->Card1";
#endif
						
							// when (MemoryCard_State>>1) == 135, then we need to fill in 0x5c, 0x5d, 0x47
							if ( ( MemoryCard_State >> 1 ) == 1 )
							{
#ifdef INLINE_DEBUG_RUN
	debug << "; ->Terminator";
#endif

								// need to put in the rest of the heading
							
								// the output data from the memory card is always good
								// this is just the end of the data from memory card in a write sequence that says all is good
								// so that means 138 bytes total get output from memory card during a write sequence
								Output_Buf [ 135 ] = 0x5c;
								Output_Buf [ 136 ] = 0x5d;
								Output_Buf [ 137 ] = 0x47;
								SizeOf_Output_Buf = 138;
							}
							
							// need to get the address to write to, then shift left two and write directly
							// high part of address is in byte 4
							// low part of address is in byte 5
							// when (MemoryCard_State>>1) == 135, then we need to fill in 0x5c, 0x5d, 0x47
							if ( ( MemoryCard_State >> 1 ) == 5 )
							{
#ifdef INLINE_DEBUG_RUN
	debug << "; ->GetAddress";
#endif

								MemoryCard_Offset = (u32) ( ( ( (u32) DataIn_Buffer [ 4 ] ) << 8 ) | (u32) DataIn_Buffer [ 5 ] ) << 7;
							}
							
							if ( ( ( MemoryCard_State >> 1 ) >= 5 ) && ( ( MemoryCard_State >> 1 ) < 135 ) )
							{
#ifdef INLINE_DEBUG_RUN
	debug << "; ->Output";
#endif

								// *** TODO *** make sure this is outputting the right data from memory card
								Output_Buf [ MemoryCard_State >> 1 ] = DataIn_Buffer [ ( MemoryCard_State >> 1 ) - 1 ];
							}
							
							if ( ( ( MemoryCard_State >> 1 ) >= 6 ) && ( ( MemoryCard_State >> 1 ) < 134 ) )
							{
#ifdef INLINE_DEBUG_RUN
	debug << "; ->ToCard";
#endif

								// this data gets output to the memory card
								MemoryCard1 [ MemoryCard_Offset++ ] = DataIn;
							}
							
							break;
						
					}
#endif
					
					break;
					
				// Command 0x52 - read data from memory card
				case 0x52:
				
#ifdef INLINE_DEBUG_RUN
	debug << "; ->MemoryCard READ";
#endif

#ifdef PS2_COMPILE

					lLastCommandType = LC_CARD_SLOW;
					
					if ( ( MemoryCard_State >> 1 ) < 137 )
					{
						// for now, for PS2, just return 0xff with terminator
						Output_Buf [ MemoryCard_State >> 1 ] = 0xff;
					}
					
					if ( ( MemoryCard_State >> 1 ) >= ( ResponseSize - 2 ) )
					{
						// put in PS2 style terminator
						Output_Buf [ ResponseSize - 2 ] = '+';
						Output_Buf [ ResponseSize - 1 ] = cTerminator;
					}
					
#else

					// check which memory card this is for
					switch ( CTRL0 & 0x2002 )
					{
						// this means memory card 0
						case 0x2:
						
							// when (MemoryCard_State>>1) == 139, then we need to fill in 0x47
							if ( ( MemoryCard_State >> 1 ) == 1 )
							{
								// put in the rest of the header now that we know it is reading from memory card
								Output_Buf [ 5 ] = 0x00;
								Output_Buf [ 6 ] = 0x5c;
								Output_Buf [ 7 ] = 0x5d;
								
							
								// the output data from the memory card is always good
								// this is just the end of the data from memory card in a write sequence that says all is good
								// so that means 140 bytes total get output from memory card during a read sequence
								Output_Buf [ 139 ] = 0x47;
								SizeOf_Output_Buf = 140;
							}
							
							if ( ( MemoryCard_State >> 1 ) == 8 )
							{
								// need to clear the XOR code
								XOR_code = 0;
								
								// we also need to read back address
								Output_Buf [ 8 ] = DataIn_Buffer [ 4 ];
								Output_Buf [ 9 ] = DataIn_Buffer [ 5 ];
							}
							
							
							// need to get the address to read from, then shift left two and read directly
							// high part of address is in byte 4
							// low part of address is in byte 5
							if ( ( MemoryCard_State >> 1 ) == 5 )
							{
								MemoryCard_Offset = (u32) ( ( ( (u32) DataIn_Buffer [ 4 ] ) << 8 ) | (u32) DataIn_Buffer [ 5 ] ) << 7;
							}
							
							if ( ( ( MemoryCard_State >> 1 ) >= 10 ) && ( ( MemoryCard_State >> 1 ) < 138 ) )
							{
								// read data from memory card - don't do all at once since this is cycle by cycle and will be multi threaded
								Output_Buf [ MemoryCard_State >> 1 ] = MemoryCard0 [ MemoryCard_Offset++ ];
							}
							
							// also need to create the xor code
							if ( ( ( MemoryCard_State >> 1 ) >= 8 ) && ( ( MemoryCard_State >> 1 ) < 138 ) )
							{
								XOR_code ^= Output_Buf [ MemoryCard_State >> 1 ];
							}
							
							// need to put in the xor code
							if ( ( MemoryCard_State >> 1 ) == 138 )
							{
								Output_Buf [ 138 ] = XOR_code;
							}
							
							break;
							
						// this means memory card 1
						case 0x2002:
						
							// when (MemoryCard_State>>1) == 139, then we need to fill in 0x47
							if ( ( MemoryCard_State >> 1 ) == 1 )
							{
								// put in the rest of the header now that we know it is reading from memory card
								Output_Buf [ 5 ] = 0x00;
								Output_Buf [ 6 ] = 0x5c;
								Output_Buf [ 7 ] = 0x5d;
								
							
								// the output data from the memory card is always good
								// this is just the end of the data from memory card in a write sequence that says all is good
								// so that means 140 bytes total get output from memory card during a read sequence
								Output_Buf [ 139 ] = 0x47;
								SizeOf_Output_Buf = 140;
							}
							
							if ( ( MemoryCard_State >> 1 ) == 6 )
							{
								// we also need to read back address
								Output_Buf [ 8 ] = DataIn_Buffer [ 4 ];
								Output_Buf [ 9 ] = DataIn_Buffer [ 5 ];
							}
							
							// need to get the address to read from, then shift left two and read directly
							// high part of address is in byte 4
							// low part of address is in byte 5
							if ( ( MemoryCard_State >> 1 ) == 5 )
							{
								MemoryCard_Offset = (u32) ( ( ( (u32) DataIn_Buffer [ 4 ] ) << 8 ) | (u32) DataIn_Buffer [ 5 ] ) << 7;
							}
							
							if ( ( ( MemoryCard_State >> 1 ) >= 10 ) && ( ( MemoryCard_State >> 1 ) < 138 ) )
							{
								// read data from memory card - don't do all at once since this is cycle by cycle and will be multi threaded
								Output_Buf [ MemoryCard_State >> 1 ] = MemoryCard1 [ MemoryCard_Offset++ ];
							}
							
							// need to also put in the XOR code
							if ( ( MemoryCard_State >> 1 ) == 138 )
							{
								// will make XOR code zero for now
								Output_Buf [ MemoryCard_State >> 1 ] = 0;
							}
							
							break;
						
					}
					
#endif
					
					break;


#ifdef PS2_COMPILE

				// reset memory card
				case 0x11:
				case 0x12:
					if ( ( MemoryCard_State >> 1 ) == 1 )
					{
//#ifdef INLINE_DEBUG_RUN_SIO2
//	debug << "; SIO2 MEMORY CARD RESET";
//#endif
//#ifdef INLINE_DEBUG_RUN_SIO2
//	debug << "; RESETTING; PacketSize=" << dec << PacketSize;
//#endif
if ( ( MemoryCard_State >> 1 ) == 1 )
{
#ifdef INLINE_DEBUG_SIO2_RUN
	debug << " (MEMCARDRESET) PacketSize=" << dec << PacketSize << " ";
#endif
}

						lLastCommandType = LC_CARD_SLOW;
						
						SizeOf_Output_Buf = 4;

						recvVal3 = 0x8c;
						
						//for ( int i = 0; i < 2048; i++ )
						//{
						//	Output_Buf [ i ] = 0xff;
						//}
						
						Output_Buf [ 0 ] = 0xff;
						Output_Buf [ 1 ] = 0xff;
						
						Output_Buf [ 2 ] = '+';
						Output_Buf [ 3 ] = cTerminator;
						
					}
					
					break;

				// SET SECTOR commands
				
				// set sector erase
				case 0x21:
				
				// set sector write
				case 0x22:
				
				// set sector read
				case 0x23:
				
//#ifdef INLINE_DEBUG_RUN_SIO2
//	debug << "; SIO2 MEMORY CARD SET SECTOR";
//	debug << "; PacketSize=" << dec << PacketSize;
//#endif
if ( ( MemoryCard_State >> 1 ) == 1 )
{
#ifdef INLINE_DEBUG_SIO2_RUN
	debug << " (MEMCARDSETSECTOR) PacketSize=" << dec << PacketSize << " ";
#endif
#ifdef VERBOSE_DEBUG_SIO2_RUN
	cout << "\nMEMCARDSETSECTOR ";
#endif
}

					lLastCommandType = LC_CARD_SLOW;
					
					SizeOf_Output_Buf = 9;
					recvVal3 = 0x8c;
					
					Output_Buf [ MemoryCard_State >> 1 ] = 0xff;
					
					
					// get the sector
					MemoryCardSector = ( (u32) DataIn_Buffer [ 2 ] ) | ( ( (u32) DataIn_Buffer [ 3 ] ) << 8 ) | ( ( (u32) DataIn_Buffer [ 4 ] ) << 16 ) | ( ( (u32) DataIn_Buffer [ 5 ] ) << 24 );
					
					// sometimes it will try to set past the last sector, so mask it
					MemoryCardSector &= 0x3fff;

					// start memory card index at the sector ??
					//MemoryCardIndex = MemoryCardSector << 9;
					MemoryCardIndex = MemoryCardSector * ( 512 + 16 );

if ( ( MemoryCard_State >> 1 ) == 6 )
{
#ifdef VERBOSE_DEBUG_SIO2_RUN
// testing only
cout << "SIO2 Testing: SetSector: " << dec << MemoryCardSector;
#endif
}
					
					// get the xor value to confirm that correct data was received
					MemoryCardXorValue = DataIn_Buffer [ 6 ];
					
					Output_Buf [ 7 ] = '+';
					Output_Buf [ 8 ] = cTerminator;
					
					break;

				case 0x24:
					// ???
					lLastCommandType = LC_CARD_SLOW;
					break;
					
				case 0x25:
					// ???
					lLastCommandType = LC_CARD_SLOW;
					break;
				
				// get memory card info
				case 0x26:
if ( ( MemoryCard_State >> 1 ) == 1 )
{
#ifdef INLINE_DEBUG_SIO2_RUN
	debug << " (MEMCARDGETINFO) ";
#endif
}

					lLastCommandType = LC_CARD_SLOW;
					
					//static const mc_command_0x26_tag mc_sizeinfo_8mb= {'+', 512, 16, 0x4000, 0x52, 0x5A};
					Output_Buf [ MemoryCard_State >> 1 ] = 0xff;
					
					// put in the terminator
					Output_Buf [ 12 ] = cTerminator;
					
					// put in the flags
					Output_Buf [ 2 ] = '+';
					
					// put in the sector size
					Output_Buf [ 3 ] = (u8) ( 512 );
					Output_Buf [ 4 ] = (u8) ( 512 >> 8 );
					
					// put in the count of sectors in the erase block
					Output_Buf [ 5 ] = (u8) ( 16 );
					Output_Buf [ 6 ] = (u8) ( 16 >> 8 );
					
					// total number of sectors in the card
					Output_Buf [ 7 ] = (u8) ( 0x4000 );
					Output_Buf [ 8 ] = (u8) ( 0x4000 >> 8 );
					Output_Buf [ 9 ] = (u8) ( 0 );
					Output_Buf [ 10 ] = (u8) ( 0 );
					
					// xor checksum
					Output_Buf [ 11 ] = 0x52;
					
					// terminator
					Output_Buf [ 12 ] = cTerminator;
					
					SizeOf_Output_Buf = 13;
					recvVal3 = 0x83;
					break;
					
				// set terminator
				case 0x27:
if ( ( MemoryCard_State >> 1 ) == 1 )
{
#ifdef INLINE_DEBUG_SIO2_RUN
	debug << " (MEMCARDSETTER) ";
#endif
}

					// a fast command ??
					CommandTime = c_iSIO2_EventCycles_FastCard;
					lLastCommandType = LC_CARD_FAST;
					
					//memset8<0xff>(sio.buf);
					Output_Buf [ MemoryCard_State >> 1 ] = 0xff;
					
					//sio.terminator = value;
					cTerminator = DataIn_Buffer [ 2 ];
					
					Output_Buf [ 3 ] = '+';
					
					//sio.buf[4] = value;
					Output_Buf [ 4 ] = cTerminator;
					
					SizeOf_Output_Buf = 5;
					recvVal3 = 0x8b;
					break;
				
				// get terminator
				case 0x28:
if ( ( MemoryCard_State >> 1 ) == 1 )
{
#ifdef INLINE_DEBUG_SIO2_RUN
	debug << " (MEMCARDGETTER) ";
#endif
}

					// a fast command ??
					CommandTime = c_iSIO2_EventCycles_MedCard;
					lLastCommandType = LC_CARD_MED;
					
					//memset8<0xff>(sio.buf);
					Output_Buf [ MemoryCard_State >> 1 ] = 0xff;
					
					Output_Buf [ 2 ] = '+';
					Output_Buf [ 3 ] = cTerminator;
					
					//sio.buf[4] = value;
					Output_Buf [ 4 ] = 0x55;
					
					SizeOf_Output_Buf = 5;
					recvVal3 = 0x8b;
					break;

				// this is already covered from ps1
				//case 0x52:
				//	break;
					
				//case 0x57:
				//	break;

				// WRITE
				case 0x42:
				
					//lLastCommandType = LC_CARD_SLOW;
					lLastCommandType = LC_CARD_FAST;
					
					Output_Buf [ 0 ] = 0xff;
					Output_Buf [ 1 ] = 0xff;
					Output_Buf [ 2 ] = 0xff;
					
					// ***TODO*** this will need fixing later
					if ( ( MemoryCard_State >> 1 ) == 2 )
					{
#ifdef INLINE_DEBUG_SIO2_RUN
	debug << " (MEMCARDWRITE) PacketSize=" << dec << PacketSize << " ";
#endif
#ifdef VERBOSE_DEBUG_SIO2_RUN
	cout << "\nMEMCARDWRITE PacketSize=" << dec << PacketSize << " ";
#endif

						// set that the last command issued was read
						LastCommand = 0x42;
						
						MemCardTransferSize = (u32) DataIn_Buffer [ 2 ];
						
						SizeOf_Output_Buf = MemCardTransferSize + 6;
						
						Output_Buf [ 3 ] = '+';
						
						// make data from mem card zero for now
						// later use "MemoryCardSector" to load from memory card at specified sector
						//for ( int i = 0; i < MemCardTransferSize; i++ )
						//{
						//	Output_Buf [ 4 + i ] = 0;
						//}
						
						// put in the xor code of just the data
						//Output_Buf [ SizeOf_Output_Buf - 2 ] = 0;
						
						// put in the terminator
						Output_Buf [ SizeOf_Output_Buf - 1 ] = cTerminator;
						
						// set the size of packet incoming
						//PacketSize = SizeOf_Output_Buf;
						
						// initialize the xor value
						MemoryCardXorValue = 0;
						
// testing only
//cout << "SIO2 Testing: Write: " << dec << " TransferSize=" << MemCardTransferSize << " OutputSize=" << SizeOf_Output_Buf;

					}
					
					// repeat the input starting at index 3 (or index 4??)
					if ( ( ( MemoryCard_State >> 1 ) >= 4 ) && ( ( MemoryCard_State >> 1 ) < ( SizeOf_Output_Buf - 2 ) ) )
					{
						Output_Buf [ MemoryCard_State >> 1 ] = DataIn_Buffer [ ( MemoryCard_State >> 1 ) - 1 ];
						
						// send the data to the memory card //
						
						// determine which memory card to send data to
						switch ( ( CTRL0 >> 13 ) & 1 )
						{
							case 0:
								MemCardPtr = PS2MemoryCard0;
								break;
								
							case 1:
								MemCardPtr = PS2MemoryCard1;
								break;
						}
						
						// Make sure we have not reached the end of data
						if ( ( MemoryCard_State >> 1 ) < ( SizeOf_Output_Buf - 2 ) )
						{
							// write data into memory card
							//MemCardPtr [ MemoryCardIndex ] = DataIn_Buffer [ MemoryCard_State >> 1 ];
							MemCardPtr [ MemoryCardIndex ] = Output_Buf [ MemoryCard_State >> 1 ];
							MemoryCardIndex++;
							
							// if the memory card index goes past the max, then wrap around
							if ( MemoryCardIndex >= c_iPS2_MemoryCard_Size )
							{
#ifdef VERBOSE_MEMCARD_WRAP
								// also alert that there was a problem
								cout << "\nhps1x64: SIO2: ALERT: Memcard wraparound to prevent crash.";
#endif
								
								MemoryCardIndex = 0;
							}
							
							// update the xor value
							//MemoryCardXorValue ^= DataIn_Buffer [ MemoryCard_State >> 1 ];
							MemoryCardXorValue ^= Output_Buf [ MemoryCard_State >> 1 ];
						}
					}
					
					if ( ( MemoryCard_State >> 1 ) == ( SizeOf_Output_Buf - 2 ) )
					{
						// put in the xor code.. using zero for now
						// ***TODO***
						//Output_Buf [ SizeOf_Output_Buf - 2 ] = 0;
						Output_Buf [ SizeOf_Output_Buf - 2 ] = MemoryCardXorValue;
						
						// probably need to update to the next sector ??
						//MemoryCardSector++;
						
						// put in the terminator
						//Output_Buf [ SizeOf_Output_Buf - 1 ] = cTerminator;
					}
					
					break;


				// PS2 MEMORY CARD READ
				case 0x43:
				
					//lLastCommandType = LC_CARD_SLOW;
					lLastCommandType = LC_CARD_FAST;
					
					Output_Buf [ 0 ] = 0xff;
					Output_Buf [ 1 ] = 0xff;
					Output_Buf [ 2 ] = 0xff;
					
					if ( ( MemoryCard_State >> 1 ) == 2 )
					{
#ifdef INLINE_DEBUG_SIO2_RUN
	debug << " (MEMCARDREAD) PacketSize=" << dec << PacketSize << " ";
#endif
#ifdef VERBOSE_DEBUG_SIO2_RUN
	cout << "\nMEMCARDREAD PacketSize=" << dec << PacketSize << " ";
#endif

						// set that the last command issued was read
						LastCommand = 0x43;
						
						MemCardTransferSize = (u32) DataIn_Buffer [ 2 ];
						
						SizeOf_Output_Buf = MemCardTransferSize + 6;
						
						Output_Buf [ 3 ] = '+';
						
						// make data from mem card zero for now
						// later use "MemoryCardSector" to load from memory card at specified sector
						//for ( int i = 0; i < MemCardTransferSize; i++ )
						//{
						//	// ***TODO*** output memory card data
						//	Output_Buf [ 4 + i ] = 0xff;	//0;
						//}
						
						// put in the xor code of just the data
						// ***TODO*** get xor code of memory card data (just xor ONLY the data together)
						Output_Buf [ SizeOf_Output_Buf - 2 ] = 0;
						
						// put in the terminator
						Output_Buf [ SizeOf_Output_Buf - 1 ] = cTerminator;
						
						// initialize the xor value
						MemoryCardXorValue = 0;
						
						// set the size of packet incoming
						//PacketSize = SizeOf_Output_Buf;
						
// testing only
//cout << "SIO2 Testing: Read: " << dec << " TransferSize=" << MemCardTransferSize << " OutputSize=" << SizeOf_Output_Buf;
					}
					
					// read the memory card data starting at index 3 (or index 4??)
					if ( ( MemoryCard_State >> 1 ) >= 4 )
					{
						//Output_Buf [ MemoryCard_State >> 1 ] = DataIn_Buffer [ MemoryCard_State >> 1 ];
						
						// send the data to the memory card //
						
						// determine which memory card to send data to
						switch ( ( CTRL0 >> 13 ) & 1 )
						{
							case 0:
								MemCardPtr = PS2MemoryCard0;
								break;
								
							case 1:
								MemCardPtr = PS2MemoryCard1;
								break;
						}
						
						// Make sure we have not reached the end of data
						if ( ( MemoryCard_State >> 1 ) < ( SizeOf_Output_Buf - 2 ) )
						{
							// write data into memory card
							Output_Buf [ MemoryCard_State >> 1 ] = MemCardPtr [ MemoryCardIndex ];
							MemoryCardIndex++;
							
							// if the memory card index goes past the max, then wrap around
							if ( MemoryCardIndex >= c_iPS2_MemoryCard_Size )
							{
#ifdef VERBOSE_MEMCARD_WRAP
								// also alert that there was a problem
								cout << "\nhps1x64: SIO2: ALERT: Memcard wraparound to prevent crash.";
#endif
								
								MemoryCardIndex = 0;
							}
							
							// update the xor value
							MemoryCardXorValue ^= Output_Buf [ MemoryCard_State >> 1 ];
						}
					}
					
					if ( ( MemoryCard_State >> 1 ) == ( SizeOf_Output_Buf - 2 ) )
					{
						// put in the xor code.. using zero for now
						// ***TODO***
						//Output_Buf [ SizeOf_Output_Buf - 2 ] = 0;
						Output_Buf [ SizeOf_Output_Buf - 2 ] = MemoryCardXorValue;
						
						// probably need to update to the next sector ??
						//MemoryCardSector++;
						
						// put in the terminator
						//Output_Buf [ SizeOf_Output_Buf - 1 ] = cTerminator;
					}
					
					break;
					
				// COMMIT
				case 0x81:

//#ifdef INLINE_DEBUG_RUN_SIO2
//	debug << "; SIO2 MEMORY CARD COMMIT";
//	debug << "; PacketSize=" << dec << PacketSize;
//#endif
if ( ( MemoryCard_State >> 1 ) == 1 )
{
#ifdef INLINE_DEBUG_SIO2_RUN
	debug << " (MEMCARDCOMMIT) ";
#endif
#ifdef VERBOSE_DEBUG_SIO2_RUN
	cout << "\nMEMCARDCOMMIT ";
#endif
}

					lLastCommandType = LC_CARD_SLOW;
					
					SizeOf_Output_Buf = 9;

					recvVal3 = 0x8c;
					
					Output_Buf [ MemoryCard_State >> 1 ] = 0xff;
					
					Output_Buf [ 2 ] = '+';
					Output_Buf [ 3 ] = cTerminator;
					
					if ( LastCommand == 0x42 )
					{
						// the last command was write
						recvVal1 = 0x1600;
						
						// in this case, need to update xor code at end of page //
						
						// get the page number
						// ??
						
						/*
						// determine which memory card to send data to
						switch ( ( CTRL0 >> 13 ) & 1 )
						{
							case 0:
								MemCardPtr = PS2MemoryCard0;
								break;
								
							case 1:
								MemCardPtr = PS2MemoryCard1;
								break;
						}
						
						// recalculate the xor code for page
						xfromman_call20_calculateXors( MemCardPtr [ MemoryCardSector * ( 512 + 16 ) + 0 ], MemCardPtr [ ( MemoryCardSector * ( 512 + 16 ) ) + 512 + 0 ] );
						xfromman_call20_calculateXors( MemCardPtr [ MemoryCardSector * ( 512 + 16 ) + 128 ], MemCardPtr [ ( MemoryCardSector * ( 512 + 16 ) ) + 512 + 3 ] );
						xfromman_call20_calculateXors( MemCardPtr [ MemoryCardSector * ( 512 + 16 ) + 256 ], MemCardPtr [ ( MemoryCardSector * ( 512 + 16 ) ) + 512 + 6 ] );
						xfromman_call20_calculateXors( MemCardPtr [ MemoryCardSector * ( 512 + 16 ) + 384 ], MemCardPtr [ ( MemoryCardSector * ( 512 + 16 ) ) + 512 + 9 ] );
						*/
						
					}
					
					if ( LastCommand == 0x43 )
					{
						// the last command was a read
						recvVal1 = 0x1700;
					}
				
					/*
					if ( ( ( LastCommand == 0x42 ) || ( LastCommand == 0x43 ) ) && ( ( MemoryCard_State >> 1 ) == 1 ) )
					{
						// determine which memory card to send data to
						switch ( ( CTRL0 >> 13 ) & 1 )
						{
							case 0:
								MemCardPtr = PS2MemoryCard0;
								break;
								
							case 1:
								MemCardPtr = PS2MemoryCard1;
								break;
						}
						
						// recalculate the xor code for page
						memset ( & MemCardPtr [ ( MemoryCardSector * ( 512 + 16 ) ) + 512 + 0 ], 0, 16 );
						xfromman_call20_calculateXors( & ( MemCardPtr [ MemoryCardSector * ( 512 + 16 ) + 0 ] ), & ( MemCardPtr [ ( MemoryCardSector * ( 512 + 16 ) ) + 512 + 0 ] ) );
						xfromman_call20_calculateXors( & ( MemCardPtr [ MemoryCardSector * ( 512 + 16 ) + 128 ] ), & ( MemCardPtr [ ( MemoryCardSector * ( 512 + 16 ) ) + 512 + 3 ] ) );
						xfromman_call20_calculateXors( & ( MemCardPtr [ MemoryCardSector * ( 512 + 16 ) + 256 ] ), & ( MemCardPtr [ ( MemoryCardSector * ( 512 + 16 ) ) + 512 + 6 ] ) );
						xfromman_call20_calculateXors( & ( MemCardPtr [ MemoryCardSector * ( 512 + 16 ) + 384 ] ), & ( MemCardPtr [ ( MemoryCardSector * ( 512 + 16 ) ) + 512 + 9 ] ) );
					}
					*/
					
					break;
					
					
				// ps2 memory card erase block
				case 0x82:
if ( ( MemoryCard_State >> 1 ) == 1 )
{
#ifdef INLINE_DEBUG_SIO2_RUN
	debug << " (ERASEBLOCK) ";
#endif
#ifdef VERBOSE_DEBUG_SIO2_RUN
	cout << "\nERASEBLOCK ";
#endif
}
				
					Output_Buf [ MemoryCard_State >> 1 ] = 0xff;
					Output_Buf [ 2 ] = '+';
					Output_Buf [ 3 ] = cTerminator;
					
					if ( ( MemoryCard_State >> 1 ) == 1 )
					{
						
						lLastCommandType = LC_CARD_SLOW;
						
						SizeOf_Output_Buf = 4;
						
						// erase the block here ?? //
						// determine which memory card to send data to
						switch ( ( CTRL0 >> 13 ) & 1 )
						{
							case 0:
								MemCardPtr = PS2MemoryCard0;
								break;
								
							case 1:
								MemCardPtr = PS2MemoryCard1;
								break;
						}
						
						
						// check if block is being erased from start of block ?
						if ( MemoryCardSector & ( c_iPS2_MemCard_SectorsPerBlock - 1 ) )
						{
							// trying to erase block from outside the starting point for a block
							cout << "\nhps1x64: SIO: ERROR?: PS2 Memory Card: Trying to erase block from outside start of block.";
						}
						else
						{
							// erase execute..
							// how many sectors per block ??
							for ( int i = 0; i < c_iPS2_MemCard_SectorsPerBlock; i++ )
							{
								memset ( & ( MemCardPtr [ ( MemoryCardSector + i ) * ( 512 + 16 ) + 0 ] ), 0xff, 512+16 );
								
								// recalculate the xor code for page
								memset ( & MemCardPtr [ ( ( MemoryCardSector + i ) * ( 512 + 16 ) ) + 512 + 0 ], 0, 16 );
								xfromman_call20_calculateXors( & ( MemCardPtr [ ( MemoryCardSector + i ) * ( 512 + 16 ) + 0 ] ), & ( MemCardPtr [ ( ( MemoryCardSector + i ) * ( 512 + 16 ) ) + 512 + 0 ] ) );
								xfromman_call20_calculateXors( & ( MemCardPtr [ ( MemoryCardSector + i ) * ( 512 + 16 ) + 128 ] ), & ( MemCardPtr [ ( ( MemoryCardSector + i ) * ( 512 + 16 ) ) + 512 + 3 ] ) );
								xfromman_call20_calculateXors( & ( MemCardPtr [ ( MemoryCardSector + i ) * ( 512 + 16 ) + 256 ] ), & ( MemCardPtr [ ( ( MemoryCardSector + i ) * ( 512 + 16 ) ) + 512 + 6 ] ) );
								xfromman_call20_calculateXors( & ( MemCardPtr [ ( MemoryCardSector + i ) * ( 512 + 16 ) + 384 ] ), & ( MemCardPtr [ ( ( MemoryCardSector + i ) * ( 512 + 16 ) ) + 512 + 9 ] ) );
							}
						}

					}
					
					break;
					
					
				// ???
				case 0xbf:
					lLastCommandType = LC_CARD_SLOW;
					
					//memset8<0xff>(sio.buf);
					Output_Buf [ MemoryCard_State >> 1 ] = 0xff;
					
					Output_Buf [ 3 ] = '+';
					
					//sio.buf[4] = value;
					Output_Buf [ 4 ] = cTerminator;
					
					SizeOf_Output_Buf = 5;
					recvVal3 = 0x8b;
					break;
					
					
				// card authentication checks
				case 0xf0:
if ( ( MemoryCard_State >> 1 ) == 2 )
{
#ifdef INLINE_DEBUG_SIO2_RUN
	debug << " (MEMCARDAUTH) ";
#endif
}
				
#ifdef INLINE_DEBUG_RUN_SIO2_TEST
	debug << "\r\n; SIO2 MEMORY CARD AUTH CHECK";
	debug << "; DataIn_Buffer[2]=" << (u32) DataIn_Buffer [ 2 ];
#endif

					// these commands appear to be fast?
					CommandTime = c_iSIO2_EventCycles_FastCard;
					lLastCommandType = LC_CARD_FAST;

					switch(	DataIn_Buffer [ 2 ] )
					{
						case  1:
						case  2:
						case  4:
						case 15:
						case 17:
						case 19:
#ifdef INLINE_DEBUG_RUN_SIO2_TEST
	debug << "->CHECK1\r\n";
#endif

							//sio.bufcount=13;
							//memset8<0xff>(sio.buf);
							Output_Buf [ MemoryCard_State >> 1 ] = 0xff;
							
							//sio.buf[12] = 0; // Xor value of data from index 4 to 11
							Output_Buf [ 12 ] = 0;
							
							//sio.buf[3]='+';
							Output_Buf [ 3 ] = '+';
							
							//sio.buf[13] = sio.terminator;
							Output_Buf [ 13 ] = cTerminator;
							
							SizeOf_Output_Buf = 14;
							break;
							
						case  6:
						case  7:
						case 11:
#ifdef INLINE_DEBUG_RUN_SIO2_TEST
	debug << "->CHECK2\r\n";
#endif

							//sio.bufcount=13;
							//memset8<0xff>(sio.buf);
							Output_Buf [ MemoryCard_State >> 1 ] = 0xff;
							
							//sio.buf[12]='+';
							Output_Buf [ 12 ] = '+';
							
							//sio.buf[13] = sio.terminator;
							Output_Buf [ 13 ] = cTerminator;
							
							SizeOf_Output_Buf = 14;
							break;
							
						default:
#ifdef INLINE_DEBUG_RUN_SIO2_TEST
	debug << "->CHECK3\r\n";
#endif

							//sio.bufcount=4;
							//memset8<0xff>(sio.buf);
							Output_Buf [ MemoryCard_State >> 1 ] = 0xff;
							
							//sio.buf[3]='+';
							Output_Buf [ 3 ] = '+';
							
							//sio.buf[4] = sio.terminator;
							Output_Buf [ 4 ] = cTerminator;
							
							SizeOf_Output_Buf = 5;
							break;
					}
					
					break;
					
				// ???
				case 0xf1:
				case 0xf2:
					CommandTime = c_iSIO2_EventCycles_FastCard;
					lLastCommandType = LC_CARD_FAST;
					break;
					
				case 0xf3:
				case 0xf7:
				
					CommandTime = c_iSIO2_EventCycles_FastCard;
					lLastCommandType = LC_CARD_FAST;
					
					if ( ( MemoryCard_State >> 1 ) == 1 )
					{
//#ifdef INLINE_DEBUG_RUN_SIO2
//	debug << "; SIO2 MEMORY CARD UNKNOWN COMMAND";
//	debug << "; PacketSize=" << dec << PacketSize;
//#endif
#ifdef INLINE_DEBUG_SIO2_RUN
	debug << " (MEMCARDUNK) ";
#endif

						SizeOf_Output_Buf = 5;

						recvVal3 = 5;
						
						//for ( int i = 0; i < 2048; i++ )
						//{
						//	Output_Buf [ i ] = 0xff;
						//}
						Output_Buf [ 0 ] = 0xff;
						Output_Buf [ 1 ] = 0xff;
						Output_Buf [ 2 ] = 0xff;
						
						Output_Buf [ 3 ] = '+';
						Output_Buf [ 4 ] = cTerminator;
						
					}
					
					break;

#endif
					
			}
			
			// send data out
			if ( ( MemoryCard_State >> 1 ) < SizeOf_Output_Buf )
			{
				DataOut = Output_Buf [ MemoryCard_State >> 1 ];
			}
			else
			{
				// I'll just output zeros if it keeps reading past end of the buffer for now
				DataOut = 0;
				
				// *** TODO *** also need to set overrun bit when reads are past end of buffer
			}
			
			// data has been received from device and is ready for reading
			isDataOut = true;
				

			// go to next control pad state
			MemoryCard_State++;
				
			// data has been received and is ready to be read
			STAT0 |= STAT_RX_RDY;
			
			// only interrupt if the rx interrupt is enabled here
			if ( ( CTRL0 & CTRL_TX_IENA ) /*|| ( CTRL0 & CTRL_DSR_IENA )*/ )
			{
#ifdef INLINE_DEBUG_RUN
	debug << "; INT";
#endif

				// signal that data is ready to be read
				//SetInterrupt_Controller ();
				Interrupt = true;
				
				// just sent irq
				STAT0 |= STAT_IRQ;
			}
			
#ifdef INLINE_DEBUG_RUN_MCD
	debug << "; isDataOut=" << isDataOut << "; DataOut=" << DataOut;
#endif

			//return;
		}
		
		// ***TESTING*** set next event to happen when ps1 receives data from memory card
		//SetNextEvent ( c_iCardCycles );
		BusyCycles = c_iCardCycles;
		
		return;
	}
	

	// check for input data
	if ( isDataIn )
	{

//#ifdef INLINE_DEBUG_RUN
//	debug << "\r\n\r\nSIO::Run; CycleCount=" << dec << *_DebugCycleCount << "; Pad_State=" << ControlPad_State << "; MCD_State=" << MemoryCard_State << hex << "; isDataIn=" << isDataIn << "; DataIn=" << DataIn;
//	debug << "\r\n; STAT0=" << STAT0 << "; CTRL0=" << CTRL0 << "; MODE0=" << MODE0 << "; BAUD0=" << BAUD0 << "; isDataOut=" << isDataOut << "\r\n";
//#endif

			// no devices have been selected yet
			
			// no commands issued yet
			Command = 0;
			
			switch ( DataIn )
			{
				case 1:
#ifdef INLINE_DEBUG_RUN
	debug << "; Selecting Control Pad";
#endif
#ifdef INLINE_DEBUG_PS2_PAD
	debug << "\r\nPS2 PAD\r\n";
#endif
					
					// handling the DataIn right now
					isDataIn = false;
					
					
					// store input and go to next control pad state
					DataIn_Buffer [ ControlPad_State ] = DataIn;
					
					
					// selected the control pad, so set the start of output data based on type of controller
					// *** TODO *** determine type of controller - assuming digital for now
					switch ( ControlPad_Type [ ( CTRL0 >> 13 ) & 1 ] )
					{
						case PADTYPE_DIGITAL:
							for ( i = 0; i < sizeof(ControlPad_Digital_Output); i++ ) Output_Buf [ i ] = ControlPad_Digital_Output [ i ];
							SizeOf_Output_Buf = sizeof(ControlPad_Digital_Output);
							
							// if rumble enabled, then response length and digital id can increase
							Output_Buf [ 1 ] += DigitalID_ExtraHalfwordCount [ ( CTRL0 >> 13 ) & 1 ];
							break;
							
						case PADTYPE_ANALOG:
							for ( i = 0; i < sizeof(ControlPad_AnalogRed_Output); i++ ) Output_Buf [ i ] = ControlPad_AnalogRed_Output [ i ];
							SizeOf_Output_Buf = sizeof(ControlPad_AnalogRed_Output);
							break;

#ifdef PS2_COMPILE
						case PADTYPE_DUALSHOCK2:
							for ( i = 0; i < sizeof(ControlPad_DualShock2_Output); i++ ) Output_Buf [ i ] = ControlPad_DualShock2_Output [ i ];
							SizeOf_Output_Buf = sizeof(ControlPad_DualShock2_Output);
							break;
#endif

						default:
							cout << "\nhps1x64: ERROR: PAD/SIO: Invalid control pad type: " << dec << ControlPad_Type [ ( CTRL0 >> 13 ) & 1 ];
							break;
							
					} // end switch ( ControlPad_Type [ ( CTRL0 >> 13 ) & 1 ] )
					
					// exception - if in config mode, the pad ID should be 0xf3
					// unless command is 0x42 ??
					if ( isConfigMode [ ( CTRL0 >> 13 ) & 1 ] )
					{
						Output_Buf [ 1 ] = 0xf3;
					}
						

					DataOut = Output_Buf [ ControlPad_State++ ];
					isDataOut = true;
				
					
					// transfer is probably ready before read is ready
					STAT0 |= STAT_RX_RDY;
					
					// transfer is done
					STAT0 &= ~( STAT_TX_RDY );
					STAT0 |= ( STAT_TX_EMPTY );
					
					// only interrupt if the rx interrupt is enabled here
					if ( CTRL0 & CTRL_TX_IENA )
					{
#ifdef INLINE_DEBUG_RUN
	debug << "; INT";
	debug << "\r\n(before) _Intc_Stat=" << hex << *_Intc_Stat << " _R3000A_Cause_13=" << *_R3000A_Cause_13 << " _R3000A_Status_12=" << *_R3000A_Status_12 << " _ProcStatus=" << *_ProcStatus;
#endif

						// signal that data is ready to be read
						//SetInterrupt_Controller ();
						Interrupt = true;
						
						// just sent irq
						STAT0 |= STAT_IRQ;
						
#ifdef INLINE_DEBUG_RUN
	debug << "\r\n(after) _Intc_Stat=" << hex << *_Intc_Stat << " _R3000A_Cause_13=" << *_R3000A_Cause_13 << " _R3000A_Status_12=" << *_R3000A_Status_12 << " _ProcStatus=" << *_ProcStatus;
#endif
					}
					
#ifdef INLINE_DEBUG_RUN
	debug << "; isDataOut=" << isDataOut << "; DataOut=" << DataOut;
#endif

					// ***TESTING*** set next event to happen when ps1 receives data from memory card
					//SetNextEvent ( c_iPadCycles );
					BusyCycles = c_iPadCycles;
					
					break;
					
				
				case 0x81:
#ifdef INLINE_DEBUG_RUN
	debug << "; Selecting Memory Card";
#endif
					// wants to communicate with memory card
					//MemoryCard_State++;
					
					// handling the DataIn right now
					isDataIn = false;
					
					
					// store input and go to next control pad state
					DataIn_Buffer [ MemoryCard_State ] = DataIn;
					
					
					// selected the control pad, so set the start of output data based on type of controller
					// *** TODO *** determine type of controller - assuming digital for now
					for ( i = 0; i < sizeof(MemoryCard_Header_Output); i++ ) Output_Buf [ i ] = MemoryCard_Header_Output [ i ];
					SizeOf_Output_Buf = sizeof(MemoryCard_Header_Output);
					
					DataOut = Output_Buf [ MemoryCard_State++ ];
					isDataOut = true;
				
					// check which memory card this is for
					switch ( CTRL0 & 0x2002 )
					{
						// this means memory card 0
						case 0x2:
						
							// check if card is connected
							if ( MemoryCard_ConnectionState [ 0 ] == MCD_DISCONNECTED )
							{
								MemoryCard_State = 0;
								isDataOut = false;
								return;
							}
							
							break;
							
						// this means memory card 1
						case 0x2002:
						
							// check if card is connected
							if ( MemoryCard_ConnectionState [ 1 ] == MCD_DISCONNECTED )
							{
								MemoryCard_State = 0;
								isDataOut = false;
								return;
							}
							
							break;
					}
					
					// transfer is probably ready before read is ready
					// actually read is ready first
					// actually, probably has not got the data back from the memory card yet
					STAT0 |= STAT_RX_RDY;
					
					// transfer is done
					STAT0 &= ~( STAT_TX_RDY );
					STAT0 |= ( STAT_TX_EMPTY );
					
					// only interrupt if the rx interrupt is enabled here
					if ( ( CTRL0 & CTRL_TX_IENA ) /*|| ( CTRL0 & CTRL_DSR_IENA )*/ )
					{
#ifdef INLINE_DEBUG_RUN
	debug << "; INT";
#endif

						// signal that data is ready to be read
						//SetInterrupt_Controller ();
						Interrupt = true;
						
						// just sent irq
						STAT0 |= STAT_IRQ;
					}
					
#ifdef INLINE_DEBUG_RUN
	debug << "; isDataOut=" << isDataOut << "; DataOut=" << DataOut;
#endif

					// ***TESTING*** set next event to happen when ps1 receives data from memory card
					//SetNextEvent ( c_iCardCycles );
					BusyCycles = c_iCardCycles;
					
#ifdef PS2_COMPILE
					//recvVal1 = 0x1100;
					recvVal1 = CONNECTION_VALUE;
#endif
					
					break;
					
#ifdef PS2_COMPILE

				// checking for Multi tap or something
				case 0x21:
#ifdef INLINE_DEBUG_PS2_MULTITAP
	debug << "\r\nPS2 MULTITAP\r\n";
#endif

					MultiTapTransferCount++;
					lLastCommandType = LC_MULTITAP;
					
					CommandTime = c_iSIO2_EventCycles_MultiTap;

					// no multi tap support yet
					recvVal1 = 0x1d100;
					
					SizeOf_Output_Buf = 6;
					Output_Buf [ 0 ] = 0xff;
					Output_Buf [ 1 ] = 0xff;
					Output_Buf [ 2 ] = 0x2b;
					Output_Buf [ 3 ] = 0x5a;
					Output_Buf [ 4 ] = 0x2b;
					Output_Buf [ 5 ] = 0x5a;
					
					CurrentDevice = 0x21;
					
					// go to next control pad state
					//ControlPad_State++;
					State = 1;
					break;
				
				// checking for a remote control or something
				case 0x61:
					lLastCommandType = LC_REMOTE;
					
					// output response byte
					SizeOf_Output_Buf = 9;
					
					// make the first one zero, and the rest 0xff
					Output_Buf [ 0 ] = 0;
					Output_Buf [ 1 ] = 0xff;
					Output_Buf [ 2 ] = 0xff;
					Output_Buf [ 3 ] = 0xff;
					Output_Buf [ 4 ] = 0xff;
					Output_Buf [ 5 ] = 0xff;
					Output_Buf [ 6 ] = 0xff;
					Output_Buf [ 7 ] = 0xff;
					Output_Buf [ 8 ] = 0xff;
					Output_Buf [ 9 ] = 0xff;
					Output_Buf [ 10 ] = 0xff;
					
					
					CurrentDevice = 0x61;
					State = 1;
					
					//recvVal1 = 0x1100;
					recvVal1 = CONNECTION_VALUE;
					
					break;
					
					
				default:
#ifdef INLINE_DEBUG_UNKNOWN
	debug << "\r\nSIO COMMAND UNKNOWN. Command=" << hex << DataIn;
#endif
					cout << "SIO COMMAND UNKNOWN. Command=" << hex << DataIn;
					break;
#endif
			}
//		}
	}
	
		
	return;
	
	// *** TEMPORARY PLACEHOLDER ***
	//if ( ! ( STAT0 & STAT_TX_EMPTY ) )
	//{
		//STAT0 |= STAT_RX_RDY | STAT_TX_RDY | STAT_TX_EMPTY;
	//}

}


// returns interrupt signal
void SIO::Run ()
{
	//static const u8 ControlPad_Digital_Output [] = { 0xff, 0x41, 0x5a };
	//static const u8 ControlPad_AnalogGreen_Output [] = { 0xff, 0x53, 0x5a };
	//static const u8 ControlPad_AnalogRed_Output [] = { 0xff, 0x73, 0x5a };
	
	//static const u8 MemoryCard_Header_Output [] = { 0xff, 0x00, 0x5a, 0x5d, 0x00 };
	
	//u32 i;
	//u32 Cycles;
	//bool bRet;

	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;


#ifdef PS2_COMPILE	
	// check if this is an SIO2 event for PS2
	if ( NextEvent_Cycle == NextSIO2_Cycle )
	{
#ifdef INLINE_DEBUG_SIO2_RUN
	debug << "\r\nSIO::Run; SIO2 Interrupt; Cycle#" << dec << *_DebugCycleCount << "\r\n";
	debug << "; SIO2_Cycles=" << dec << SIO2_Cycles;
	//debug << "; PadTransferCount=" << dec << PadTransferCount;
	debug << "; PacketSize=" << dec << PacketSize;
	debug << "; Multiplier=" << dec << SIO2Multiplier;
#endif

		// SIO2 event //
		
		SetInterrupt_SIO2 ();
		
		// clear next event cycle and NextSIO2_Cycle for future use
		//NextEvent_Cycle = -1LL;
		NextSIO2_Cycle = -1LL;
		Set_NextEventCycle ( -1ULL );
		
		// set recvval 1 ??
		recvVal1ret = recvVal1;
		
		// update the dma state since external conditions have changed
		Playstation1::Dma::_DMA->Update_ActiveChannel ();
		
		/*
		// after checking/sending interrupt check for dma transfer
		if ( Playstation1::Dma::_DMA->isEnabledAndActive ( 12 ) )
		{
			// attempt to perform dma transfer
			//Playstation1::Dma::_DMA->DMA12_Run ();
			Playstation1::Dma::_DMA->Transfer ( 12 );
		}
		*/
		
		return;
	}
#endif

	// clear out the next event cycle for now, event is being handled
	Set_NextEventCycle ( -1ULL );
	
	Execute ();
	
	if ( Interrupt )
	{
		// interrupt was requested
		SetInterrupt_Controller ();
	}
	

	if ( BusyCycles )
	{
		// device is busy, check back after device is no longer busy
		// next event was requested
		SetNextEvent ( BusyCycles );
	}
}



void SIO::Command_0x42 ( bool ForceAnalogOutput )
{
	u32 PortMap = -1;
	u32 PadNum = 0;
	
	// check which controller this is for
	switch ( CTRL0 & 0x2002 )
	{
		case 0x2:
			PortMap = PortMapping [ 0 ];
			PadNum = 0;
			break;
			
		case 0x2002:
			PortMap = PortMapping [ 1 ];
			PadNum = 1;
			break;
	}
	
	// if that controller port is not connected, then treat as nothing connected
	if ( PortMap >= DJoy.gameControllerStates.size() )
	{
		PortMap = -1;
	}
	
	// read the joystick data
	switch ( PortMap )
	{
		case 0:
			// read joystick
#ifdef ENABLE_DIRECT_INPUT
			DJoy.Update ( 0 );
#else
			Joy.ReadJoystick ( 0 );
#endif
			break;
			
		case 1:
			// read joystick
#ifdef ENABLE_DIRECT_INPUT
			DJoy.Update ( 1 );
#else
			Joy.ReadJoystick ( 1 );
#endif
			break;
	}


			// this is the first set of keys for a digital controller - bits 2 and 4 are always high
			Output_Buf [ 3 ] = 0xff;
			
			// this is the second set of keys for a digital controller
			Output_Buf [ 4 ] = 0xff;
			
			// pretend pad is analog
			Output_Buf [ 5 ] = 0x80;
			Output_Buf [ 6 ] = 0x80;
			Output_Buf [ 7 ] = 0x80;
			Output_Buf [ 8 ] = 0x80;
			
#ifdef PS2_COMPILE
			Output_Buf [ 9 ] = 0;
			Output_Buf [ 10 ] = 0;
			Output_Buf [ 11 ] = 0;
			Output_Buf [ 12 ] = 0;
			Output_Buf [ 13 ] = 0;
			Output_Buf [ 14 ] = 0;
			Output_Buf [ 15 ] = 0;
			Output_Buf [ 16 ] = 0;
			Output_Buf [ 17 ] = 0;
			Output_Buf [ 18 ] = 0;
			Output_Buf [ 19 ] = 0;
			Output_Buf [ 20 ] = 0;
#endif


	
	// process the joystick data
	switch ( PortMap )
	{
		case 0:
		case 1:
			// *** TODO *** check if pad is digital/analog/etc
			// assuming digital pad for now
			
			
			// this is the first set of keys for a digital controller - bits 2 and 4 are always high
			Output_Buf [ 3 ] = 0xff;
			
			// this is the second set of keys for a digital controller
			Output_Buf [ 4 ] = 0xff;
			
#ifdef ENABLE_DIRECT_INPUT
			if ( DJoy.gameControllerStates[PortMap].rgdwPOV[0] > 0 && DJoy.gameControllerStates[PortMap].rgdwPOV[0] < 18000 )
			{
				// right arrow is down on joystick
				Output_Buf [ 3 ] ^= 0x20;
			}
			
			if ( DJoy.gameControllerStates[PortMap].rgdwPOV[0] > 9000 && DJoy.gameControllerStates[PortMap].rgdwPOV[0] < 27000 )
			{
				// down arrow is down on joystick
				Output_Buf [ 3 ] ^= 0x40;
			}
			
			if ( DJoy.gameControllerStates[PortMap].rgdwPOV[0] > 18000 && DJoy.gameControllerStates[PortMap].rgdwPOV[0] <= 36000 )
			{
				// left arrow is down on joystick
				Output_Buf [ 3 ] ^= 0x80;
			}
			
			if ( ( DJoy.gameControllerStates[PortMap].rgdwPOV[0] < 9000 || DJoy.gameControllerStates[PortMap].rgdwPOV[0] > 27000 ) && DJoy.gameControllerStates[PortMap].rgdwPOV[0] <= 36000 && DJoy.gameControllerStates[PortMap].rgdwPOV[0] >= 0 )
			{
				// up arrow is down on joystick
				Output_Buf [ 3 ] ^= 0x10;
			}
#else
			if ( Joy.joyinfo[PortMap].dwPOV > 0 && Joy.joyinfo[PortMap].dwPOV < 18000 )
			{
				// right arrow is down on joystick
				Output_Buf [ 3 ] ^= 0x20;
			}
			
			if ( Joy.joyinfo[PortMap].dwPOV > 9000 && Joy.joyinfo[PortMap].dwPOV < 27000 )
			{
				// down arrow is down on joystick
				Output_Buf [ 3 ] ^= 0x40;
			}
			
			if ( Joy.joyinfo[PortMap].dwPOV > 18000 && Joy.joyinfo[PortMap].dwPOV <= 36000 )
			{
				// left arrow is down on joystick
				Output_Buf [ 3 ] ^= 0x80;
			}
			
			if ( ( Joy.joyinfo[PortMap].dwPOV < 9000 || Joy.joyinfo[PortMap].dwPOV > 27000 ) && Joy.joyinfo[PortMap].dwPOV <= 36000 && Joy.joyinfo[PortMap].dwPOV >= 0 )
			{
				// up arrow is down on joystick
				Output_Buf [ 3 ] ^= 0x10;
			}
#endif

#ifdef ENABLE_DIRECT_INPUT
			if ( DJoy.gameControllerStates[PortMap].rgbButtons[ Key_Start [ PadNum ] ] )
			{
				// start key //
				Output_Buf [ 3 ] ^= 0x08;
			}
			
			//if ( Joy.joyinfo[PortMap].dwButtons & Key_Select [ PadNum ] )
			if ( DJoy.gameControllerStates[PortMap].rgbButtons[ Key_Select [ PadNum ] ] )
			{
				// select key //
				Output_Buf [ 3 ] ^= 0x01;
			}
			
			//if ( Joy.joyinfo[PortMap].dwButtons & Key_X [ PadNum ] )
			if ( DJoy.gameControllerStates[PortMap].rgbButtons[ Key_X [ PadNum ] ] )
			{
				// x key //
				Output_Buf [ 4 ] ^= 0x40;
			}
			
			//if ( Joy.joyinfo[PortMap].dwButtons & Key_O [ PadNum ] )
			if ( DJoy.gameControllerStates[PortMap].rgbButtons[ Key_O [ PadNum ] ] )
			{
				// o key //
				Output_Buf [ 4 ] ^= 0x20;
			}
			
			//if ( Joy.joyinfo[PortMap].dwButtons & Key_Square [ PadNum ] )
			if ( DJoy.gameControllerStates[PortMap].rgbButtons[ Key_Square [ PadNum ] ] )
			{
				// square key //
				Output_Buf [ 4 ] ^= 0x80;
			}
			
			//if ( Joy.joyinfo[PortMap].dwButtons & Key_Triangle [ PadNum ] )
			if ( DJoy.gameControllerStates[PortMap].rgbButtons[ Key_Triangle [ PadNum ] ] )
			{
				// triangle key //
				Output_Buf [ 4 ] ^= 0x10;
			}
			
			//if ( Joy.joyinfo[PortMap].dwButtons & Key_L1 [ PadNum ] )
			if ( DJoy.gameControllerStates[PortMap].rgbButtons[ Key_L1 [ PadNum ] ] )
			{
				// L1 key //
				Output_Buf [ 4 ] ^= 0x4;
			}
			
			//if ( Joy.joyinfo[PortMap].dwButtons & Key_L2 [ PadNum ] )
			if ( DJoy.gameControllerStates[PortMap].rgbButtons[ Key_L2 [ PadNum ] ] )
			{
				// L2 key //
				Output_Buf [ 4 ] ^= 0x1;
			}
			
			//if ( Joy.joyinfo[PortMap].dwButtons & Key_R1 [ PadNum ] )
			if ( DJoy.gameControllerStates[PortMap].rgbButtons[ Key_R1 [ PadNum ] ] )
			{
				// R1 key //
				Output_Buf [ 4 ] ^= 0x8;
			}
			
			//if ( Joy.joyinfo[PortMap].dwButtons & Key_R2 [ PadNum ] )
			if ( DJoy.gameControllerStates[PortMap].rgbButtons[ Key_R2 [ PadNum ] ] )
			{
				// R2 key //
				Output_Buf [ 4 ] ^= 0x2;
			}
			
			//if ( Joy.joyinfo[PortMap].dwButtons & Key_L3 [ PadNum ] )
			if ( DJoy.gameControllerStates[PortMap].rgbButtons[ Key_L3 [ PadNum ] ] )
			{
				// L3 key //
				Output_Buf [ 3 ] ^= 0x2;
			}
			
			//if ( Joy.joyinfo[PortMap].dwButtons & Key_R3 [ PadNum ] )
			if ( DJoy.gameControllerStates[PortMap].rgbButtons[ Key_R3 [ PadNum ] ] )
			{
				// R3 key //
				Output_Buf [ 3 ] ^= 0x4;
			}
#else
			if ( Joy.joyinfo[PortMap].dwButtons & Key_Start [ PadNum ] )
			{
				// start key //
				Output_Buf [ 3 ] ^= 0x08;
			}
			
			if ( Joy.joyinfo[PortMap].dwButtons & Key_Select [ PadNum ] )
			{
				// select key //
				Output_Buf [ 3 ] ^= 0x01;
			}
			
			if ( Joy.joyinfo[PortMap].dwButtons & Key_X [ PadNum ] )
			{
				// x key //
				Output_Buf [ 4 ] ^= 0x40;
			}
			
			if ( Joy.joyinfo[PortMap].dwButtons & Key_O [ PadNum ] )
			{
				// o key //
				Output_Buf [ 4 ] ^= 0x20;
			}
			
			if ( Joy.joyinfo[PortMap].dwButtons & Key_Square [ PadNum ] )
			{
				// square key //
				Output_Buf [ 4 ] ^= 0x80;
			}
			
			if ( Joy.joyinfo[PortMap].dwButtons & Key_Triangle [ PadNum ] )
			{
				// triangle key //
				Output_Buf [ 4 ] ^= 0x10;
			}
			
			if ( Joy.joyinfo[PortMap].dwButtons & Key_L1 [ PadNum ] )
			{
				// L1 key //
				Output_Buf [ 4 ] ^= 0x4;
			}
			
			if ( Joy.joyinfo[PortMap].dwButtons & Key_L2 [ PadNum ] )
			{
				// L2 key //
				Output_Buf [ 4 ] ^= 0x1;
			}
			
			if ( Joy.joyinfo[PortMap].dwButtons & Key_R1 [ PadNum ] )
			{
				// R1 key //
				Output_Buf [ 4 ] ^= 0x8;
			}
			
			if ( Joy.joyinfo[PortMap].dwButtons & Key_R2 [ PadNum ] )
			{
				// R2 key //
				Output_Buf [ 4 ] ^= 0x2;
			}
			
			if ( Joy.joyinfo[PortMap].dwButtons & Key_L3 [ PadNum ] )
			{
				// L3 key //
				Output_Buf [ 3 ] ^= 0x2;
			}
			
			if ( Joy.joyinfo[PortMap].dwButtons & Key_R3 [ PadNum ] )
			{
				// R3 key //
				Output_Buf [ 3 ] ^= 0x4;
			}
#endif

#ifdef ENABLE_DIRECT_INPUT
			long *pData = (long*) & SIO::DJoy.gameControllerStates[PortMap];

			if ( pData [ RightAnalog_X [ PadNum ] ] )
			{
				Output_Buf [ 5 ] = pData [ RightAnalog_X [ PadNum ] ] - 1;
			}
			else
			{
				Output_Buf [ 5 ] = 0x80;
			}
			
			if ( pData [ RightAnalog_Y [ PadNum ] ] )
			{
				Output_Buf [ 6 ] = pData [ RightAnalog_Y [ PadNum ] ] - 1;
			}
			else
			{
				Output_Buf [ 6 ] = 0x80;
			}
			
			if ( pData [ LeftAnalog_X [ PadNum ] ] )
			{
				Output_Buf [ 7 ] = pData [ LeftAnalog_X [ PadNum ] ] - 1;
			}
			else
			{
				Output_Buf [ 7 ] = 0x80;
			}
			

			if ( pData [ LeftAnalog_Y [ PadNum ] ] )
			{
				Output_Buf [ 8 ] = pData [ LeftAnalog_Y [ PadNum ] ] - 1;
			}
			else
			{
				Output_Buf [ 8 ] = 0x80;
			}
#else
			// right analog stick - x-axis
			if ( /*Joy.joyinfo.dwRpos*/ ((u32*)(&Joy.joyinfo[PadNum].dwXpos)) [ RightAnalog_X [ PadNum ] ] == 0x7fff )
			{
				Output_Buf [ 5 ] = 0x80;
			}
			else
			{
				Output_Buf [ 5 ] = ( /*Joy.joyinfo.dwRpos*/ ((u32*)(&Joy.joyinfo[PadNum].dwXpos)) [ RightAnalog_X [ PadNum ] ] >> 8 );
			}
			
			// right analog stick - y-axis
			if ( /*Joy.joyinfo.dwZpos*/ ((u32*)(&Joy.joyinfo[PadNum].dwXpos)) [ RightAnalog_Y [ PadNum ] ] == 0x7fff )
			{
				Output_Buf [ 6 ] = 0x80;
			}
			else
			{
				Output_Buf [ 6 ] = ( /*Joy.joyinfo.dwZpos*/ ((u32*)(&Joy.joyinfo[PadNum].dwXpos)) [ RightAnalog_Y [ PadNum ] ] >> 8 );
			}
			
			// left analog stick - x-axis
			if ( /*Joy.joyinfo.dwXpos*/ ((u32*)(&Joy.joyinfo[PadNum].dwXpos)) [ LeftAnalog_X [ PadNum ] ] == 0x7fff )
			{
				Output_Buf [ 7 ] = 0x80;
			}
			else
			{
				Output_Buf [ 7 ] = ( /*Joy.joyinfo.dwXpos*/ ((u32*)(&Joy.joyinfo[PadNum].dwXpos)) [ LeftAnalog_X [ PadNum ] ] >> 8 );
			}
			
			// left analog stick - y-axis
			if ( /*Joy.joyinfo.dwYpos*/ ((u32*)(&Joy.joyinfo[PadNum].dwXpos)) [ LeftAnalog_Y [ PadNum ] ] == 0x7fff )
			{
				Output_Buf [ 8 ] = 0x80;
			}
			else
			{
				Output_Buf [ 8 ] = ( /*Joy.joyinfo.dwYpos*/ ((u32*)(&Joy.joyinfo[PadNum].dwXpos)) [ LeftAnalog_Y [ PadNum ] ] >> 8 );
			}
#endif
			
#ifdef PS2_COMPILE
			// put in data for Dual Shock 2 controller (PS2-only) //

			// put in analog info for digital joypad //
			// looks like it goes right,left,up,down
			
			// right arrow
			Output_Buf [ 9 ] = 0;
			//if ( Output_Buf [ 3 ] & 0x20 )
			if ( ! ( Output_Buf [ 3 ] & 0x20 ) )
			{
				Output_Buf [ 9 ] = 0xff;
			}
			
			// left arrow
			Output_Buf [ 10 ] = 0;
			//if ( Output_Buf [ 3 ] & 0x80 )
			if ( ! ( Output_Buf [ 3 ] & 0x80 ) )
			{
				Output_Buf [ 10 ] = 0xff;
			}

			// up arrow
			Output_Buf [ 11 ] = 0;
			//if ( Output_Buf [ 3 ] & 0x10 )
			if ( ! ( Output_Buf [ 3 ] & 0x10 ) )
			{
				Output_Buf [ 11 ] = 0xff;
			}
			
			// down arrow
			Output_Buf [ 12 ] = 0;
			//if ( Output_Buf [ 3 ] & 0x40 )
			if ( ! ( Output_Buf [ 3 ] & 0x40 ) )
			{
				Output_Buf [ 12 ] = 0xff;
			}
			
			// put in data for the buttons //
			// looks like it goes triangle,circle,cross,square,l1,r1,l2,r2
			
			// triangle
			Output_Buf [ 13 ] = 0;
			//if ( Output_Buf [ 4 ] & 0x10 )
			if ( ! ( Output_Buf [ 4 ] & 0x10 ) )
			{
				Output_Buf [ 13 ] = 0xff;
			}
			
			// circle
			Output_Buf [ 14 ] = 0;
			//if ( Output_Buf [ 4 ] & 0x20 )
			if ( ! ( Output_Buf [ 4 ] & 0x20 ) )
			{
				Output_Buf [ 14 ] = 0xff;
			}
			
			// cross
			Output_Buf [ 15 ] = 0;
			//if ( Output_Buf [ 4 ] & 0x40 )
			if ( ! ( Output_Buf [ 4 ] & 0x40 ) )
			{
				Output_Buf [ 15 ] = 0xff;
			}
			
			// square
			Output_Buf [ 16 ] = 0;
			//if ( Output_Buf [ 4 ] & 0x80 )
			if ( ! ( Output_Buf [ 4 ] & 0x80 ) )
			{
				Output_Buf [ 16 ] = 0xff;
			}
			
			// L1
			Output_Buf [ 17 ] = 0;
			//if ( Output_Buf [ 4 ] & 0x04 )
			if ( ! ( Output_Buf [ 4 ] & 0x04 ) )
			{
				Output_Buf [ 17 ] = 0xff;
			}
			
			// R1
			Output_Buf [ 18 ] = 0;
			//if ( Output_Buf [ 4 ] & 0x08 )
			if ( ! ( Output_Buf [ 4 ] & 0x08 ) )
			{
				Output_Buf [ 18 ] = 0xff;
			}
			
			// L2
			Output_Buf [ 19 ] = 0;
			//if ( Output_Buf [ 4 ] & 0x01 )
			if ( ! ( Output_Buf [ 4 ] & 0x01 ) )
			{
				Output_Buf [ 19 ] = 0xff;
			}
			
			// R2
			Output_Buf [ 20 ] = 0;
			//if ( Output_Buf [ 4 ] & 0x02 )
			if ( ! ( Output_Buf [ 4 ] & 0x02 ) )
			{
				Output_Buf [ 20 ] = 0xff;
			}
#endif			

			break;




		/*
			// update amount of data to output to include the two bytes just above
			// note: if in config mode and analog pad, then analog output data is forced
			// actually, analog output is always forced in config mode
			//if ( ControlPad_Type [ 0 ] == PADTYPE_DIGITAL && !ForceAnalogOutput )
			if ( ControlPad_Type [ PadNum ] == PADTYPE_DIGITAL && !ForceAnalogOutput )
			{
				// digital controller //
				SizeOf_Output_Buf += 2 + ( DigitalID_ExtraHalfwordCount [ ( CTRL0 >> 13 ) & 1 ] << 1 );
			}
			else
#ifdef PS2_COMPILE
			//if ( ( ControlPad_Type [ 0 ] == PADTYPE_ANALOG ) || ( ControlPad_Type [ 0 ] == PADTYPE_DIGITAL && ForceAnalogOutput ) )
			//if ( ( ControlPad_Type [ 0 ] == PADTYPE_ANALOG ) || ( ForceAnalogOutput ) )
			if ( ( ControlPad_Type [ PadNum ] == PADTYPE_ANALOG ) || ( ForceAnalogOutput ) )
#endif
			{
				// analog controller //
				SizeOf_Output_Buf += 6;
			}
#ifdef PS2_COMPILE
			else
			{
				// ps2 dual shock 2 analog controller //
				SizeOf_Output_Buf += 18;
			}
#endif
			
			break;
			
		default:
			// *** TODO *** check if pad is digital/analog/etc
			// assuming digital pad for now
			
			
			
			break;
		*/
	}
	
			// update amount of data to output to include the two bytes just above
			//if ( ControlPad_Type [ 1 ] == PADTYPE_DIGITAL && !ForceAnalogOutput )
			if ( ControlPad_Type [ PadNum ] == PADTYPE_DIGITAL && !ForceAnalogOutput )
			{
				// digital controller //
				SizeOf_Output_Buf += 2 + ( DigitalID_ExtraHalfwordCount [ ( CTRL0 >> 13 ) & 1 ] << 1 );
			}
			else
#ifdef PS2_COMPILE
			//if ( ( ControlPad_Type [ 1 ] == PADTYPE_ANALOG ) || ( ControlPad_Type [ 1 ] == PADTYPE_DIGITAL && ForceAnalogOutput ) )
			if ( ( ControlPad_Type [ PadNum ] == PADTYPE_ANALOG ) || ( ControlPad_Type [ PadNum ] == PADTYPE_DIGITAL && ForceAnalogOutput ) )
#endif
			{
				// analog controller //
				SizeOf_Output_Buf += 6;
			}
#ifdef PS2_COMPILE
			else
			{
				// ps2 dual shock 2 analog controller //
				SizeOf_Output_Buf += 18;
			}
#endif
	
}




u32 SIO::Read ( u32 Address )
{
#ifdef INLINE_DEBUG_READ
	debug << "\r\nSIO::Read " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address;
#endif

	u32 Output;

	switch ( Address )
	{
		case SIO0_DATA:
#ifdef INLINE_DEBUG_READ
	debug << "; DATA0";
#endif
			// incoming read from DATA
			//Output = DATA0;
			
			
			// if there is data waiting to be output, then return it, otherwise return 0xff
			Output = _SIO->DataOut;
			_SIO->isDataOut = false;
			
			// receive is no longer ready - cuz data is being read (***todo*** account for more stuff in read buffer)
			// mame does this and sets DataOut = 0xff
			_SIO->STAT0 &= ~STAT_RX_RDY;
			_SIO->DataOut = 0xff;

			
			/*
			if ( _SIO->ControlPad_State )
			{
				// set an event to occur
				//_SIO->GetNextEvent ();
				_SIO->SetNextEvent ( c_iPadCycles );
			}
			*/
			
			break;
			
		case SIO0_STAT:
#ifdef INLINE_DEBUG_READ
	debug << "; STAT0";
#endif

			// incoming read from STAT
			Output = _SIO->STAT0;
			break;
			
		case SIO0_MODE:
#ifdef INLINE_DEBUG_READ
	debug << "; MODE0";
#endif

			// incoming read from MODE
			Output = _SIO->MODE0;
			break;

		case SIO0_CTRL:
#ifdef INLINE_DEBUG_READ
	debug << "; CTRL0";
#endif

			// incoming read from CTRL
			Output = _SIO->CTRL0;
			break;
			
		case SIO0_BAUD:
#ifdef INLINE_DEBUG_READ
	debug << "; BAUD0";
#endif

			// incoming read from BAUD
			Output = _SIO->BAUD0;
			
			break;

		case SIO1_DATA:
#ifdef INLINE_DEBUG_READ
	debug << "; DATA1";
#endif
			// incoming read from DATA
			Output = _SIO->DATA1;
			break;
			
		case SIO1_STAT:
#ifdef INLINE_DEBUG_READ
	debug << "; STAT1";
#endif

			// incoming read from STAT
			Output = _SIO->STAT1;
			break;
			
		case SIO1_MODE:
#ifdef INLINE_DEBUG_READ
	debug << "; MODE1";
#endif

			// incoming read from MODE
			Output = _SIO->MODE1;
			break;

		case SIO1_CTRL:
#ifdef INLINE_DEBUG_READ
	debug << "; CTRL1";
#endif

			// incoming read from CTRL
			Output = _SIO->CTRL1;
			break;
			
		case SIO1_BAUD:
#ifdef INLINE_DEBUG_READ
	debug << "; BAUD1";
#endif

			// incoming read from BAUD
			Output = _SIO->BAUD1;
			break;
			

#ifdef PS2_COMPILE

		// 0x1f808260
		case SIO2_FIFO_IN:
#ifdef INLINE_DEBUG_READ
			debug << "; SIO2_FIFO_IN";
#endif

			// do nothing - this is the input to input fifo

			break;

		// 0x1f808264
		case SIO2_FIFO_OUT:
#ifdef INLINE_DEBUG_READ
			debug << "; SIO2_FIFO_OUT";
#endif

			Output = _SIO->SIO2_FifoOutput ();
			
			break;

		// 0x1f808268
		case SIO2_CTRL:
#ifdef INLINE_DEBUG_READ
			debug << "; SIO2_CTRL";
#endif

			Output = _SIO->SIO2_CTRL_Reg;

			break;

		// 0x1f80826c
		case SIO2_RECV1:
#ifdef INLINE_DEBUG_READ
			debug << "; SIO2_RECV1";
			if ( _SIO->recvVal1ret != _SIO->recvVal1 )
			{
				debug << "; ***EARLY***";
			}
#endif

			//Output = _SIO->recvVal1;
			_SIO->recvVal1ret = _SIO->recvVal1;
			
			// ??
			//_SIO->SetNextEvent ( -1LL );
			_SIO->Set_NextEventCycle ( -1LL );
			
			Output = _SIO->recvVal1ret;

			break;

		// 0x1f808270
		case SIO2_RECV2:
#ifdef INLINE_DEBUG_READ
			debug << "; SIO2_RECV2";
#endif

			// ????
			Output = 0xf;

			break;

		// 0x1f808274
		case SIO2_RECV3:
#ifdef INLINE_DEBUG_READ
			debug << "; SIO2_RECV3";
#endif

			//if(sio2.packet.recvVal3 == 0x8C || sio2.packet.recvVal3 == 0x8b ||
			//	sio2.packet.recvVal3 == 0x83)
			if ( _SIO->recvVal3 == 0x8c || _SIO->recvVal3 == 0x8b || _SIO->recvVal3 == 0x83 )
			{
				//PAD_LOG("Reading Recv3 = %x",sio2.packet.recvVal3);

				//sio.packetsize = sio2.packet.recvVal3;
				_SIO->PacketSize = _SIO->recvVal3;
				
				//sio2.packet.recvVal3 = 0; // Reset
				_SIO->recvVal3 = 0;
				
				//return sio.packetsize;
				Output = _SIO->PacketSize;
			}
			else
			{
				//PAD_LOG("Reading Recv3 = %x",sio.packetsize << 16);

				//return sio.packetsize << 16;
				//Output = _SIO->PacketSize << 16;
				//Output = _SIO->TransferSize << 16;
				//Output = _SIO->ResponseSize << 16;
				Output = _SIO->SIO2_OutputIndex << 16;
			}
			
			break;

		// 0x1f808278
		case SIO2_8278:
#ifdef INLINE_DEBUG_READ
			debug << "; SIO2_8278";
#endif

			Output = _SIO->SIO2_8278_Reg;

			break;

		// 0x1f80827c
		case SIO2_827C:
#ifdef INLINE_DEBUG_READ
			debug << "; SIO2_827c";
#endif

			Output = _SIO->SIO2_827c_Reg;

			break;

		// 0x1f808280
		case SIO2_INTR:
#ifdef INLINE_DEBUG_READ
			debug << "; SIO2_INTR";
#endif

			Output = _SIO->SIO2_INTR_Reg;

			break;

#endif
			

			
		default:
#ifdef PS2_COMPILE

			if ( Address >= 0x1f808200 && Address < 0x1f808290 )
			{
#ifdef INLINE_DEBUG_READ_SIO2
	debug << "; SIO2";
#endif

				if ( Address < 0x1f808240 )
				{
#ifdef INLINE_DEBUG_READ_SIO2
	debug << "; Array0";
#endif

					Output = _SIO->Array0 [ ( Address >> 2 ) & 0xf ];
				}
				else if ( Address < 0x1f808260 )
				{
#ifdef INLINE_DEBUG_READ_SIO2
	debug << "; Array1";
#endif

					Output = _SIO->Array1 [ ( Address >> 2 ) & 0x7 ];
				}

				break;
			}
			
#endif


#ifdef INLINE_DEBUG_READ
			debug << "; Invalid";
#endif

			// invalid SIO Register
			cout << "\nhps1x64 ALERT: Unknown SIO READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
			break;
	};
	
#ifdef INLINE_DEBUG_READ
	debug << "; Output =" << hex << Output;
#endif

	return Output;
}



void SIO::Set_CTRL0 ( u32 Data )
{
	// incoming write to CTRL
	CTRL0 = Data & 0xffff;
	
	if ( ( CTRL0 & CTRL_SIO_RESET ) || !CTRL0 )
	{
		// reset command issued
		// the two commented out lines were how mame was handling this
		//STAT0 |= STAT_TX_RDY | STAT_TX_EMPTY;
		//STAT0 &= ~( STAT_RX_RDY | STAT_RX_OVERRUN | STAT_IRQ );
		ControlPad_State = 0;
		MemoryCard_State = 0;
		STAT0 &= ~( STAT_RX_RDY | STAT_RX_OVERRUN | STAT_IRQ );
		STAT0 |= STAT_TX_RDY | STAT_TX_EMPTY;
		
		State = 0;
		CurrentDevice = 0;
	}
	
	if ( CTRL0 & CTRL_IACK )
	{
		// acknowledging interrupt
		STAT0 &= ~STAT_IRQ;
		CTRL0 &= ~CTRL_IACK;
	}
	
	if ( CTRL0 & CTRL_BREAK )
	{
#ifdef INLINE_DEBUG_BREAK
	debug << "\r\nBreak";
#endif

	}
}



void SIO::Write ( u32 Address, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nSIO::Write " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << "; Data=" << Data;
#endif

	// *** testing *** check if mask is a word write
	//if ( Mask != 0xff && Mask != 0xffff )
	//{
	//	cout << "\nhps1x64 ALERT: SIO::Write Mask=" << hex << Mask << " Address=" << Address << " Data=" << Data;
	//}
	
	
	// could be writing a byte or a halfword, or even possibly word? so need to mask
	Data &= Mask;
	
	switch ( Address )
	{
		case SIO0_DATA:
#ifdef INLINE_DEBUG_WRITE
	debug << "; DATA0";
#endif

			// incoming write to DATA
			_SIO->DATA0 = Data;
			
			
			// set the input data
			_SIO->DataIn = Data;
			_SIO->isDataIn = true;
			
			// just wrote data so response is not ready yet
			//STAT0 &= ~STAT_RX_RDY;
			
			// transfer buffer is not empty since we just filled it
			// not ready for transfer since it is in progress
			// mame does this too
			_SIO->STAT0 &= ~( STAT_TX_RDY | STAT_TX_EMPTY );
			
			// transfer is started
			_SIO->STAT0 |= ( STAT_TX_RDY );

			// check if both counters are zero
			if ( !_SIO->ControlPad_State && !_SIO->MemoryCard_State )
			{
				// command being sent //
				
				if ( Data == 0x1 )
				{
					// wants to send control pad data //
					_SIO->SetNextEvent ( c_iPadCycles );
				}
				else if ( Data == 0x81 )
				{
					// wants to send memory card data //
					_SIO->SetNextEvent ( c_iCardCycles );
				}
				else
				{
					// unknown command //
					_SIO->SetNextEvent ( c_iPadCycles );
				}
			}
			else if ( _SIO->ControlPad_State )
			{
				// control pad data being sent //
				_SIO->SetNextEvent ( c_iPadCycles );
			}
			else
			{
				// memory card data being sent //
				_SIO->SetNextEvent ( c_iCardCycles );
			}
			
			// data is not ready to be read yet
			//isDataOut = false;
			
			// now we need to set an event to occur
			//_SIO->GetNextEvent ();
			
			break;
			
		case SIO0_STAT:
#ifdef INLINE_DEBUG_WRITE
	debug << "; STAT0";
#endif

			// incoming write to STAT
			
			// can't write to STAT register
			//STAT0 = Data;
			
			break;
			
		case SIO0_MODE:
#ifdef INLINE_DEBUG_WRITE
	debug << "; MODE0";
#endif

			// incoming write to MODE
			_SIO->MODE0 = Data & 0xffff;

			// the prescaler info is in the mode register lowest 2 bits
			_SIO->Update_PreScaler ();
			
			// need to update the wait cycles again since prescaler has changed
			_SIO->Update_WaitCycles ();
			
			
			break;

		case SIO0_CTRL:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CTRL0";
#endif

			_SIO->Set_CTRL0 ( Data );
			
			break;
			
		case SIO0_BAUD:
#ifdef INLINE_DEBUG_WRITE
	debug << "; BAUD0";
#endif

			// incoming write to BAUD
			_SIO->BAUD0 = Data;

			// set the speed at which serial bus ticks
			_SIO->Update_WaitCycles ();
			
			break;

		case SIO1_DATA:
#ifdef INLINE_DEBUG_WRITE
	debug << "; DATA1";
#endif

			// incoming write to DATA
			_SIO->DATA1 = Data;
			break;
			
		case SIO1_STAT:
#ifdef INLINE_DEBUG_WRITE
	debug << "; STAT1";
#endif

			// incoming write to STAT
			_SIO->STAT1 = Data;
			break;
			
		case SIO1_MODE:
#ifdef INLINE_DEBUG_WRITE
	debug << "; MODE1";
#endif

			// incoming write to MODE
			_SIO->MODE1 = Data;
			break;

		case SIO1_CTRL:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CTRL1";
#endif

			// incoming write to CTRL
			_SIO->CTRL1 = Data;
			break;
			
		case SIO1_BAUD:
#ifdef INLINE_DEBUG_WRITE
	debug << "; BAUD1";
#endif

			// incoming write to BAUD
			_SIO->BAUD1 = Data;
			break;


#ifdef PS2_COMPILE

		// 0x1f808260
		case SIO2_FIFO_IN:
#ifdef INLINE_DEBUG_WRITE
			debug << "; SIO2_FIFO_IN";
#endif

			// this is where the input data to SIO gets written to on a PS2
			// looks like it takes data from registers in 0x1f808200-0x1f80823c and uses that for SIO0 CTRL values per command
			
			//_SIO->InputBuffer [ _SIO->InputIndex++ ] = Data;
			
			_SIO->SIO2_FifoInput ( Data );

			break;

		// 0x1f808264
		case SIO2_FIFO_OUT:
#ifdef INLINE_DEBUG_WRITE
			debug << "; SIO2_FIFO_OUT";
#endif

			// do nothing - this is where the output data from SIO gets read from on a PS2

			break;

		// 0x1f808268
		case SIO2_CTRL:
#ifdef INLINE_DEBUG_WRITE
			debug << "; SIO2_CTRL";
#endif

			_SIO->Set_CTRL2 ( Data );

			break;

		// 0x1f80826c
		case SIO2_RECV1:
#ifdef INLINE_DEBUG_WRITE
			debug << "; SIO2_RECV1";
#endif


			break;

		// 0x1f808270
		case SIO2_RECV2:
#ifdef INLINE_DEBUG_WRITE
			debug << "; SIO2_RECV2";
#endif

			
			break;

		// 0x1f808274
		case SIO2_RECV3:
#ifdef INLINE_DEBUG_WRITE
			debug << "; SIO2_RECV3";
#endif


			break;

		// 0x1f808278
		case SIO2_8278:
#ifdef INLINE_DEBUG_WRITE
			debug << "; SIO2_8278";
#endif

			_SIO->SIO2_8278_Reg = Data;

			break;

		// 0x1f80827c
		case SIO2_827C:
#ifdef INLINE_DEBUG_WRITE
			debug << "; SIO2_827c";
#endif

			_SIO->SIO2_827c_Reg = Data;

			break;

		// 0x1f808280
		case SIO2_INTR:
#ifdef INLINE_DEBUG_WRITE
			debug << "; SIO2_INTR";
#endif

			_SIO->SIO2_INTR_Reg = Data;

			break;

#endif

			
		default:
#ifdef PS2_COMPILE

			if ( Address >= 0x1f808200 && Address < 0x1f808290 )
			{
#ifdef INLINE_DEBUG_WRITE_SIO2
	debug << "; SIO2";
#endif

				if ( Address < 0x1f808240 )
				{
#ifdef INLINE_DEBUG_WRITE_SIO2
	debug << "; Array0";
#endif

					_SIO->Array0 [ ( Address >> 2 ) & 0xf ] = Data;
				}
				else if ( Address < 0x1f808260 )
				{
#ifdef INLINE_DEBUG_WRITE_SIO2
	debug << "; Array1";
#endif

					_SIO->Array1 [ ( Address >> 2 ) & 0x7 ] = Data;
				}

				break;
			}
#endif

#ifdef INLINE_DEBUG_WRITE
			debug << "; Invalid";
#endif
		
			// invalid SIO Register
			cout << "\nhps1x64 ALERT: Unknown SIO WRITE @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data << "\n";
			break;
	};
	
}

/*
void SIO::SetInterrupt_Controller ()
{
	//*_Intc_Stat |= ( 1 << c_InterruptBit_Controller ) & *_Intc_Mask;
}
*/


#ifdef PS2_COMPILE

// dma#11 ready function
u64 SIO::SIO2in_DMA_Ready ()
{
	return _SIO->SIO2_InputReady;
}

// dma#12 ready function
u64 SIO::SIO2out_DMA_Ready ()
{
	return _SIO->SIO2_OutputReady;
}


void SIO::SIO2_FifoInput ( u8 Data )
{
	if ( !InputCount )
	{
		// no longer the start of input
		StartOfInput = false;
		
		// should probably set size of command input here
		//sio2.cmdlength=(sio2.packet.sendArray3[sio2.cmdport] >> 8) & 0x1FF;
		PacketSize = ( Array0 [ CommandPort ] >> 8 ) & 0x1ff;
		ResponseSize = PacketSize;
		
		// calc cycles based on packet size??
		//SIO2_Cycles += PacketSize;
		
		// also set CTRL0
		
		// initialize the count of input data
		InputCount = 0;
		
		//sioWriteCtrl16(SIO_RESET);
		// ** reset SIO **
		Set_CTRL0 ( 0 );
		
		// set CTRL0
		Set_CTRL0 ( ( ( Array0 [ CommandPort ] & 1 ) << 13 ) | 2 );
		
		// clear the read index for the output data here
		//OutputReadIndex = 0;
		
		// update command port ??
		CommandPort++;
	}

	// set as the data in
	DataIn = Data;
	isDataIn = true;
	
	// execute the SIO with the data that was set
	Execute ();
	//SIO2_Cycles += BusyCycles;
	
	// execute fifo without datain
	isDataIn = false;
	Execute ();
	//SIO2_Cycles += BusyCycles;
	
	//SIO2_OutputBuffer [ SIO2_OutputIndex++ ] = Output_Buf [ InputCount ];
	
	// input the data
	InputCount++;
	
	// need to calculate the amount of time needed to process the data when it is done transferring
	if ( InputCount == PacketSize )
	{
		SIO2_Cycles += (u64) ( PacketSize * lLastCommandType );
	}
	
	// dma transfers will control the "InputCount"
	if ( !isDmaTransfer )
	{
		if ( InputCount >= PacketSize )
		{
			for ( int i = 0; i < PacketSize; i++ )
			{
				if ( SIO2_OutputIndex < c_iSIO2OutputBuffer_Size )
				{
					SIO2_OutputBuffer [ SIO2_OutputIndex++ ] = Output_Buf [ i ];
				}
				else
				{
					cout << "\nhps1x64: ERROR: SIO2_OutputIndex >= c_iSIO2OutputBuffer_Size";
				}
			}
			
			InputCount = 0;
		}
		
	}
}


u8 SIO::SIO2_FifoOutput ()
{
	u8 Output;
	
	if ( !OutputReadIndex )
	{
		// first check if this is a memory card data output
		//if ( MemoryCard_State )
		//{
		//	Output_Buf [ SizeOf_Output_Buf - 1 ] = cTerminator;
		//	Output_Buf [ SizeOf_Output_Buf - 2 ] = '+';
		//}
	}
	
	//if ( OutputReadIndex < SizeOf_Output_Buf )
	if ( OutputReadIndex < SIO2_OutputIndex )
	{
		// pull data from the SIO2 output buffer for now
		//Output = Output_Buf [ OutputReadIndex++ ];
		Output = SIO2_OutputBuffer [ OutputReadIndex++ ];
	}
	else
	{
		Output = 0;
		//Output = 0xff;
	}
	
	return Output;
}


// reads a block of data from the device via DMA
//void SIO::DMA_ReadBlock ( u8* Data, int ByteCount )
u32 SIO::DMA_ReadBlock ( u32* pMemoryPtr, u32 Address, u32 WordCount )
{
#ifdef INLINE_DEBUG_SIO2_DMA_READ
	debug << "\r\nDMA_ReadBlock";
	debug << "; TransferSize=" << dec << WordCount;
	debug << " OutputReadIndex=" << dec << _SIO->OutputReadIndex;
	debug << " SIO2_OutputIndex=" << dec << _SIO->SIO2_OutputIndex;
	debug << " PacketSize=" << dec << _SIO->PacketSize;
	debug << "; Output=";
#endif

	u8 *Data;
	u32 ByteCount;
	
	Data = (u8*) & ( pMemoryPtr [ Address >> 2 ] );

	_SIO->isDmaTransfer = true;
	
	ByteCount = WordCount << 2;

	for ( int i = 0; i < ByteCount; i++ )
	//for ( int i = 0; i < TransferSize; i++ )
	{
		//Data [ i ] = SIO2_FifoOutput ();
		Data [ i ] = _SIO->SIO2_OutputBuffer [ _SIO->OutputReadIndex++ ];
		
#ifdef INLINE_DEBUG_SIO2_DMA_READ_CONTENTS
	debug << hex << setw(2) << (u32) Data [ i ] << " ";
#endif
	}
	
	// ??interrupt??
	//SetInterrupt_SIO2 ();
	_SIO->isDmaTransfer = false;
	
	return WordCount;
}

// writes a block of data to the device via DMA
//void SIO::DMA_WriteBlock ( u8* Data, int ByteCount )
u32 SIO::DMA_WriteBlock ( u32* pMemoryPtr, u32 Address, u32 WordCount )
{
	u8 *Data;
	u32 ByteCount;
	
	Data = (u8*) & ( pMemoryPtr [ Address >> 2 ] );
	
#ifdef INLINE_DEBUG_SIO2_DMA_WRITE
	debug << "\r\nDMA_WriteBlock";
	debug << "; TransferSize=" << dec << WordCount;
	debug << " OutputReadIndex=" << dec << _SIO->OutputReadIndex;
	debug << " SIO2_OutputIndex=" << dec << _SIO->SIO2_OutputIndex;
	debug << " PacketSize=" << dec << _SIO->PacketSize;
	debug << "; Input=";
	
#ifdef INLINE_DEBUG_SIO2_DMA_WRITE_CONTENTS
	for ( int i = 0; i < WordCount; i++ )
	{
		debug << hex << setw(2) << (u32) Data [ i ] << " ";
	}
#endif

#endif

	int i;

	_SIO->isDmaTransfer = true;
	
	_SIO->InputCount = 0;
	
	ByteCount = WordCount << 2;

	for ( i = 0; i < ByteCount; i++ )
	//for ( int i = 0; i < TransferSize; i++ )
	{
		_SIO->SIO2_FifoInput ( Data [ i ] );
		
	}
	
#ifdef INLINE_DEBUG_SIO2_DMA_WRITE
	debug << "; Output=";
#endif

	for ( i = 0; i < ByteCount; i++ )
	{
		if ( i < _SIO->PacketSize )
		{
		
			if ( _SIO->SIO2_OutputIndex < c_iSIO2OutputBuffer_Size )
			{
				_SIO->SIO2_OutputBuffer [ _SIO->SIO2_OutputIndex++ ] = _SIO->Output_Buf [ i ];
			}
			else
			{
				cout << "\nhps1x64: ERROR: SIO2: DMA_WriteBlock1: SIO2_OutputIndex >= c_iSIO2OutputBuffer_Size";
			}
			
#ifdef INLINE_DEBUG_SIO2_DMA_WRITE_CONTENTS
	debug << hex << setw(2) << (u32) _SIO->Output_Buf [ i ] << " ";
#endif
		}
		else
		{
			if ( _SIO->SIO2_OutputIndex < c_iSIO2OutputBuffer_Size )
			{
				_SIO->SIO2_OutputBuffer [ _SIO->SIO2_OutputIndex++ ] = 0;
			}
			else
			{
				cout << "\nhps1x64: ERROR: SIO2: DMA_WriteBlock2: SIO2_OutputIndex >= c_iSIO2OutputBuffer_Size";
			}
		}
	}
	
	_SIO->isDmaTransfer = false;
	
	return WordCount;
}




void SIO::Set_CTRL2 ( u32 Data )
{
#ifdef INLINE_DEBUG_CTRL2
	debug << "; Set_CTRL2";
	debug << " (before) CTRL2=" << hex << SIO2_CTRL_Reg;
#endif

	// I don't know anything about this SIO2 interface, so this is copied/adapted from pcsx2 for now
	//sio2.ctrl=value;
	SIO2_CTRL_Reg = Data;
	
	// this is for the entire packet whether sending or receiving
	OutputReadIndex = 0;
	
	//if (sio2.ctrl & 1){	//recv packet
	if ( SIO2_CTRL_Reg & 1 )
	{
#ifdef INLINE_DEBUG_CTRL2
	debug << "; ReceivePacket";
	debug << "; SIO2_Cycles=" << dec << SIO2_Cycles;
#endif

		// this means it wants to send a packet out from SIO2

		//iopIntcIrq( 17 );
		// ** interrupt **
		//SetInterrupt_SIO2 ();
		// only interrupt if bits 8 or 9 are set??
		//if ( SIO2_CTRL_Reg & 0x300 )
		//{
			// maybe divide by two??
			//SIO2_Cycles >>= 2;
			//SIO2_Cycles = c_iSIO2_EventCycles;
			//SIO2_Cycles = ( PadTransferCount * c_iSIO2_PadPacketSizeMult ) + ( MultiTapTransferCount  * c_iSIO2_PadPacketSizeMult ) + ( CardSlowTransferCount * c_iSIO2_CardPacketSizeMult );
			//SIO2_Cycles = CommandTime;
			//if ( !SIO2_Cycles ) SIO2_Cycles = CommandTime;
			//SIO2_Cycles *= SIO2Multiplier;
			SetNextEvent ( SIO2_Cycles );
			//SetNextEvent ( c_iSIO2_EventCycles );
			NextSIO2_Cycle = NextEvent_Cycle;
			
			lLastCommandType = 0;
			PadTransferCount = 0;
			MultiTapTransferCount = 0;
			CardSlowTransferCount = 0;
		//}
		
		//SBUS
		//sio2.recvIndex=0;
		//ReceiveIndex = 0;
		
		//sio2.ctrl &= ~1;
		SIO2_CTRL_Reg &= ~1;
		
		SIO2_OutputReady = true;
		SIO2_InputReady = false;
		
		// for testing, for now, act like the data is cool
		// 0x1100 means ok, 0x1d100 means not ok
		//recvVal1 = 0x1100;
		
		//if ( Playstation1::Dma::_DMA->isEnabledAndActive ( 12 ) )
		//{
		//	// attempt to perform dma transfer
		//	Playstation1::Dma::_DMA->DMA12_Run ();
		//}
	}
	else
	{
		// send packet
		
#ifdef INLINE_DEBUG_CTRL2
	debug << "; SendPacket";
	debug << "; PacketSize=" << dec << ( ( Array0 [ CommandPort ] >> 8 ) & 0x1ff );
#endif

		// this means it wants to accept a packet into SIO2
	
		//clean up
		//sio2.packet.sendSize=0;	//reset size
		//SendSize = 0;
		OutputWriteIndex = 0;
		
		//sio2.cmdport=0;
		CommandPort = 0;
		
		//sio2.cmdlength=0;
		CommandLength = 0;
		
		// set as start of input
		StartOfInput = true;
		
		// clear the cycles for the command being sent
		SIO2_Cycles = 0;
		
		
		SIO2_OutputReady = false;
		SIO2_InputReady = true;
		
		// this should probably happen too
		//recvVal1 = 0;
		recvVal1ret = 0;
		recvVal2 = 0;
		recvVal3 = 0;
		
		// set intput index to zero
		InputCount = 0;
		
		// need to know the size of sio2 buffer, starts at zero
		SIO2_OutputIndex = 0;
		
		// update the dma state since external conditions have changed
		Playstation1::Dma::_DMA->Update_ActiveChannel ();
		
		/*
		if ( Playstation1::Dma::_DMA->isEnabledAndActive ( 11 ) )
		{
			// attempt to perform dma transfer
			//Playstation1::Dma::_DMA->DMA11_Run ();
			Playstation1::Dma::_DMA->Transfer ( 11 );
		}
		*/
	}
	
//#ifdef INLINE_DEBUG_CTRL2
//	debug << " (after) CTRL2=" << SIO2_CTRL_Reg;
//#endif
}

#endif



void SIO::Update_NextEventCycle ()
{
	//if ( NextEvent_Cycle > *_SystemCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_SystemCycleCount ) ) *_NextSystemEvent = NextEvent_Cycle;
	if ( NextEvent_Cycle < *_NextSystemEvent )
	{
		*_NextSystemEvent = NextEvent_Cycle;
		*_NextEventIdx = NextEvent_Idx;
	}
}


void SIO::SetNextEvent ( u64 Cycles )
{
	NextEvent_Cycle = Cycles + *_DebugCycleCount;
	
	Update_NextEventCycle ();
}

void SIO::Set_NextEventCycle ( u64 Cycle )
{
	NextEvent_Cycle = Cycle;
	
	Update_NextEventCycle ();
}

void SIO::GetNextEvent ()
{
	SetNextEvent ( WaitCycles0 );
}






void SIO::Update_PreScaler ()
{
	// set prescaler
	switch ( MODE0 & 3 )
	{
		case 0:
			PreScaler0 = 0;
			break;
			
		case 1:
			PreScaler0 = 1;
			break;
			
		case 2:
			PreScaler0 = 16;
			break;
			
		case 3:
			PreScaler0 = 64;
			break;
	}
	
	switch ( MODE1 & 3 )
	{
		case 0:
			PreScaler1 = 0;
			break;
			
		case 1:
			PreScaler1 = 1;
			break;
			
		case 2:
			PreScaler1 = 16;
			break;
			
		case 3:
			PreScaler1 = 64;
			break;
	}
}


void SIO::Update_WaitCycles ()
{
	u32 Temp;
	
	// update speed of sio = bus speed / ( prescaler * ( baud << 16 ) )
	Temp = ( PreScaler0 * ( BAUD0 << 16 ) );
	if ( Temp ) WaitCycles0 = ( 33868800 / Temp ) * 8; else WaitCycles0 = 0;
	
	Temp = ( PreScaler1 * ( BAUD1 << 16 ) );
	if ( Temp ) WaitCycles1 = ( 33868800 / Temp ) * 8; else WaitCycles1 = 0;
	
	// *** TESTING *** //
	//WaitCycles0 = 100;
	//WaitCycles1 = 100;
}


void SIO::Load_MemoryCardFile ( string FileName, int MemoryCard_Slot )
{
	FILE *f;
	
	f = fopen(FileName.c_str(), "rb");
	if (f == NULL)
		return;
	
	if ( MemoryCard_Slot == 0 )
	{
		fread ( MemoryCard0, 1, c_iMemoryCard_Size, f );
	}
	else
	{
		fread ( MemoryCard1, 1, c_iMemoryCard_Size, f );
	}
	
	fclose (f);
}

void SIO::Store_MemoryCardFile ( string FileName, int MemoryCard_Slot )
{
	FILE *f;
	
	f = fopen(FileName.c_str(), "wb");
	if (f == NULL)
		return;
	
	if ( MemoryCard_Slot == 0 )
	{
		fwrite ( MemoryCard0, 1, c_iMemoryCard_Size, f );
	}
	else
	{
		fwrite ( MemoryCard1, 1, c_iMemoryCard_Size, f );
	}
	
	fclose (f);
}



#ifdef PS2_COMPILE

void SIO::Load_PS2MemoryCardFile ( string FileName, int MemoryCard_Slot )
{
	FILE *f;
	
	f = fopen(FileName.c_str(), "rb");
	if (f == NULL)
		return;
	
	if ( MemoryCard_Slot == 0 )
	{
		fread ( PS2MemoryCard0, 1, c_iPS2_MemoryCard_Size, f );
	}
	else
	{
		fread ( PS2MemoryCard1, 1, c_iPS2_MemoryCard_Size, f );
	}
	
	fclose (f);
}

void SIO::Store_PS2MemoryCardFile ( string FileName, int MemoryCard_Slot )
{
	FILE *f;
	
	f = fopen(FileName.c_str(), "wb");
	if (f == NULL)
		return;
	
	if ( MemoryCard_Slot == 0 )
	{
		fwrite ( PS2MemoryCard0, 1, c_iPS2_MemoryCard_Size, f );
	}
	else
	{
		fwrite ( PS2MemoryCard1, 1, c_iPS2_MemoryCard_Size, f );
	}
	
	fclose (f);
}

#endif



void SIO::Create_MemoryCardFile ( const char* FileName )
{
	CreateMcd ( FileName );
}


// *** this stuff below here is from pcsx *** //

#define MCD_SIZE	(1024 * 8 * 16)

void CreateMcd(const char *mcd) {
	FILE *f;
	struct stat buf;
	int s = MCD_SIZE;
	int i = 0, j;

	f = fopen(mcd, "wb");
	if (f == NULL)
		return;

	if (stat(mcd, &buf) != -1) {
		if ((buf.st_size == MCD_SIZE + 3904) || strstr(mcd, ".gme")) {
			s = s + 3904;
			fputc('1', f);
			s--;
			fputc('2', f);
			s--;
			fputc('3', f);
			s--;
			fputc('-', f);
			s--;
			fputc('4', f);
			s--;
			fputc('5', f);
			s--;
			fputc('6', f);
			s--;
			fputc('-', f);
			s--;
			fputc('S', f);
			s--;
			fputc('T', f);
			s--;
			fputc('D', f);
			s--;
			for (i = 0; i < 7; i++) {
				fputc(0, f);
				s--;
			}
			fputc(1, f);
			s--;
			fputc(0, f);
			s--;
			fputc(1, f);
			s--;
			fputc('M', f);
			s--;
			fputc('Q', f);
			s--;
			for (i = 0; i < 14; i++) {
				fputc(0xa0, f);
				s--;
			}
			fputc(0, f);
			s--;
			fputc(0xff, f);
			while (s-- > (MCD_SIZE + 1))
				fputc(0, f);
		} else if ((buf.st_size == MCD_SIZE + 64) || strstr(mcd, ".mem") || strstr(mcd, ".vgs")) {
			s = s + 64;
			fputc('V', f);
			s--;
			fputc('g', f);
			s--;
			fputc('s', f);
			s--;
			fputc('M', f);
			s--;
			for (i = 0; i < 3; i++) {
				fputc(1, f);
				s--;
				fputc(0, f);
				s--;
				fputc(0, f);
				s--;
				fputc(0, f);
				s--;
			}
			fputc(0, f);
			s--;
			fputc(2, f);
			while (s-- > (MCD_SIZE + 1))
				fputc(0, f);
		}
	}
	fputc('M', f);
	s--;
	fputc('C', f);
	s--;
	while (s-- > (MCD_SIZE - 127))
		fputc(0, f);
	fputc(0xe, f);
	s--;

	for (i = 0; i < 15; i++) { // 15 blocks
		fputc(0xa0, f);
		s--;
		fputc(0x00, f);
		s--;
		fputc(0x00, f);
		s--;
		fputc(0x00, f);
		s--;
		fputc(0x00, f);
		s--;
		fputc(0x00, f);
		s--;
		fputc(0x00, f);
		s--;
		fputc(0x00, f);
		s--;
		fputc(0xff, f);
		s--;
		fputc(0xff, f);
		s--;
		for (j = 0; j < 117; j++) {
			fputc(0x00, f);
			s--;
		}
		fputc(0xa0, f);
		s--;
	}

	for (i = 0; i < 20; i++) {
		fputc(0xff, f);
		s--;
		fputc(0xff, f);
		s--;
		fputc(0xff, f);
		s--;
		fputc(0xff, f);
		s--;
		fputc(0x00, f);
		s--;
		fputc(0x00, f);
		s--;
		fputc(0x00, f);
		s--;
		fputc(0x00, f);
		s--;
		fputc(0xff, f);
		s--;
		fputc(0xff, f);
		s--;
		for (j = 0; j < 118; j++) {
			fputc(0x00, f);
			s--;
		}
	}

	while ((s--) >= 0)
		fputc(0, f);

	fclose(f);
}




