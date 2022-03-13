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


#include "PS1_PIO.h"

using namespace Playstation1;


Debug::Log PIO::debug;

PIO *PIO::_PIO;


u64* PIO::_NextSystemEvent;



#ifdef _DEBUG_VERSION_

//#define INLINE_DEBUG_ENABLE
//#define INLINE_DEBUG
//#define INLINE_DEBUG_READ
//#define INLINE_DEBUG_WRITE
//#define INLINE_DEBUG_RUN

#endif


funcVoid PIO::UpdateInterrupts;

u32* PIO::_DebugPC;
u64* PIO::_DebugCycleCount;
u64* PIO::_SystemCycleCount;
u32* PIO::_NextEventIdx;

u32* PIO::_Intc_Stat;
u32* PIO::_Intc_Mask;
//u32* PIO::_Intc_Master;
u32* PIO::_R3000A_Status_12;
u32* PIO::_R3000A_Cause_13;
u64* PIO::_ProcStatus;


PIO::PIO ()
{
}


void PIO::Start ()
{
	cout << "Running PIO::Start...\n";
	
#ifdef INLINE_DEBUG_ENABLE
	debug.Create( "PIO_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering PIO::Start";
#endif

	Reset ();
	
	_PIO = this;
	
	
	// start next event as far in the future
	Set_NextEventCycle ( -1ULL );

#ifdef INLINE_DEBUG
	debug << "->Exiting PIO::Start";
#endif
}


void PIO::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( PIO ) );
}

// returns interrupt;
void PIO::Run ()
{
	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;
	
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nPIO::Run " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << "; BusyCycles=" << BusyCycles;
#endif

	
#ifdef INLINE_DEBUG_RUN
	debug << "; Signalling Interrupt";
#endif

	// time of next event after this one is not known
	//Set_NextEvent ( 0xffffffffffffffffULL );
	Set_NextEventCycle ( -1ULL );

	// signal interrupt
	SetInterrupt ();
}



u32 PIO::Read ( u32 Address )
{
#ifdef INLINE_DEBUG_READ
	debug << "\r\nPIO::Read " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address;
#endif

	u32 Output;
	
	// interrupt when there is more data ready from PIO
	//_PIO->BusyCycles = 16;
	//_PIO->NextEvent_Cycle = *_DebugCycleCount + 16;
	_PIO->Set_NextEvent ( 16 );
	
	Output = 0;

	return 0;
	
#ifdef INLINE_DEBUG_READ
	debug << "; Output =" << dec << Output;
#endif

}

void PIO::Write ( u32 Address, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nPIO::Write " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << "; Data=" << Data;
#endif

}

void PIO::DMA_Read ( u32* Data, int ByteReadCount )
{
#ifdef INLINE_DEBUG
	debug << "\r\nPIO::DMA_Read";
#endif

	Data [ 0 ] = 0;
}

void PIO::DMA_Write ( u32* Data, int ByteWriteCount )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nPIO::DMA_Write; Data = " << Data [ 0 ];
#endif
}



