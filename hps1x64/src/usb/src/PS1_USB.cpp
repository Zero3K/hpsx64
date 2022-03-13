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


#include "PS1_USB.h"

using namespace Playstation1;


Debug::Log USB::debug;

USB *USB::_USB;


u64* USB::_NextSystemEvent;


#define VERBOSE_USB_INT
#define VERBOSE_USB_READ
#define VERBOSE_USB_WRITE

#ifdef _DEBUG_VERSION_

//#define INLINE_DEBUG_ENABLE
//#define INLINE_DEBUG_SPLIT

//#define INLINE_DEBUG
//#define INLINE_DEBUG_READ
//#define INLINE_DEBUG_WRITE
#define INLINE_DEBUG_RUN

#endif


funcVoid USB::UpdateInterrupts;

u32* USB::_DebugPC;
u64* USB::_DebugCycleCount;
u64* USB::_SystemCycleCount;
u32* USB::_NextEventIdx;

u32* USB::_Intc_Stat;
u32* USB::_Intc_Mask;
//u32* USB::_Intc_Master;
u32* USB::_R3000A_Status_12;
u32* USB::_R3000A_Cause_13;
u64* USB::_ProcStatus;


USB::USB ()
{
}


void USB::Start ()
{
	cout << "Running USB::Start...\n";
	
#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create( "USB_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering USB::Start";
#endif

	Reset ();
	
	_USB = this;
	
	
	// initialize events to far in the future
	Set_NextEventCycle ( -1ULL );
	

#ifdef INLINE_DEBUG
	debug << "->Exiting USB::Start";
#endif
}


void USB::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( USB ) );
}


void USB::Run ()
{
	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;
	
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nUSB::Run " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << "; BusyCycles=" << BusyCycles;
#endif

	
#ifdef INLINE_DEBUG_RUN
	debug << "; Signalling Interrupt";
#endif

#ifdef VERBOSE_USB_INT
	// ALERT
	cout << "\nhps1x64: USB: INTERRUPT. PC=" << hex << *_DebugPC << " Cycle=" << dec << *_DebugCycleCount << "\n";
#endif

	// time of next event after this one is not known
	//Set_NextEvent ( 0xffffffffffffffffULL );
	Set_NextEventCycle ( -1ULL );

	// signal interrupt
	SetInterrupt ();
}



u32 USB::Read ( u32 Address )
{
#ifdef INLINE_DEBUG_READ
	debug << "\r\nUSB::Read " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address;
#endif

	u32 Output;
	
	// interrupt when there is more data ready from USB
	//_USB->BusyCycles = 16;
	//_USB->NextEvent_Cycle = *_DebugCycleCount + 16;
	//_USB->Set_NextEvent ( 16 );
	
	Output = 0;

	/*
	if ( ( Address & 0x1fffffff ) == 0x1f80160c )
	{
		Output = 0xffffffff;
	}

	if ( ( Address & 0x1fffffff ) == 0x1f801610 )
	{
		Output = 0xffffffff;
	}
	*/

#ifdef VERBOSE_USB_READ
	// ALERT
	cout << "\nhps1x64: USB: READ. PC=" << hex << *_DebugPC << " Address=" << Address << " Cycle=" << dec << *_DebugCycleCount << " Output=" << hex << Output << "\n";
#endif
	
	return Output;
	
#ifdef INLINE_DEBUG_READ
	debug << "; Output =" << dec << Output;
#endif

}

void USB::Write ( u32 Address, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nUSB::Write " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << "; Data=" << Data;
#endif

#ifdef VERBOSE_USB_WRITE
	// ALERT
	cout << "\nhps1x64: USB: WRITE. PC=" << hex << *_DebugPC << " Address=" << Address << " Cycle=" << dec << *_DebugCycleCount << " Input=" << hex << Data << "\n";
#endif
	
	/*
	if ( ( Address & 0x1fffffff ) == 0x1f801610 )
	{
		if ( Data )
		{
			_USB->Set_NextEvent ( c_iIntDelay );
		}
	}
	*/

}

/*
void USB::DMA_Read ( u32* Data, int ByteReadCount )
{
#ifdef INLINE_DEBUG
	debug << "\r\nUSB::DMA_Read";
#endif

	Data [ 0 ] = 0;
}

void USB::DMA_Write ( u32* Data, int ByteWriteCount )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nUSB::DMA_Write; Data = " << Data [ 0 ];
#endif
}
*/


