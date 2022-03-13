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



#include "PS2_SIO.h"
//#include "WinApiHandler.h"

using namespace Playstation2;




#ifdef _DEBUG_VERSION_

#define INLINE_DEBUG_ENABLE
//#define INLINE_DEBUG
#define INLINE_DEBUG_CONSOLE

//#define INLINE_DEBUG_WRITE
//#define INLINE_DEBUG_READ
//#define INLINE_DEBUG_RUN

#endif


u32 *SIO::_DebugPC;
u64 *SIO::_DebugCycleCount;
u32* SIO::_NextEventIdx;

u32* SIO::_Intc_Stat;
u32* SIO::_Intc_Mask;
u32* SIO::_R5900_Status_12;
u32* SIO::_R5900_Cause_13;
u64* SIO::_ProcStatus;


u64* SIO::_NextSystemEvent;


Debug::Log SIO::debug;

SIO *SIO::_SIO;




SIO::SIO ()
{
	cout << "Running SIO constructor...\n";

/*
#ifdef INLINE_DEBUG_ENABLE
	debug.Create( "PS2_SIO_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering SIO::SIO constructor";
#endif


	Reset ();

	
#ifdef INLINE_DEBUG
	debug << "->Exiting SIO::SIO constructor";
#endif
*/

}


void SIO::Start ()
{
	cout << "Running SIO::Start...\n";

#ifdef INLINE_DEBUG_ENABLE
	debug.Create( "PS2_SIO_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering SIO::Start";
#endif


	Reset ();
	
	_SIO = this;

	
	// clear events
	Set_NextEventCycle ( -1ULL );
	
#ifdef INLINE_DEBUG
	debug << "->Exiting SIO::Start";
#endif
}


void SIO::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( SIO ) );
}





void SIO::Run ()
{


	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;

#ifdef INLINE_DEBUG_RUN
	debug << "\r\n\r\nSIO::Run; CycleCount=" << dec << *_DebugCycleCount;
#endif



}







u64 SIO::Read ( u32 Address, u64 Mask )
{
#ifdef INLINE_DEBUG_READ
	debug << "\r\nSIO::Read " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address;
#endif

	u32 Output;

	switch ( Address )
	{
		case SIO_LCR:
#ifdef INLINE_DEBUG_READ
	debug << "; LCR";
#endif

			Output = 0;
			break;
			
		case SIO_LSR:
#ifdef INLINE_DEBUG_READ
	debug << "; LSR";
#endif

			Output = 0;
			break;
			
		case SIO_IER:
#ifdef INLINE_DEBUG_READ
	debug << "; IER";
#endif


			Output = 0;
			break;

		case SIO_ISR:
#ifdef INLINE_DEBUG_READ
	debug << "; ISR";
#endif

			Output = 0;
			break;
			
		case SIO_FCR:
#ifdef INLINE_DEBUG_READ
	debug << "; FCR";
#endif

			Output = 0;
			break;

		case SIO_BGR:
#ifdef INLINE_DEBUG_READ
	debug << "; BGR";
#endif

			Output = 0;
			break;
			
		case SIO_TXFIFO:
#ifdef INLINE_DEBUG_READ
	debug << "; TXFIFO";
#endif

			// incoming read from STAT
			Output = 0;
			break;
			
		case SIO_RXFIFO:
#ifdef INLINE_DEBUG_READ
	debug << "; RXFIFO";
#endif

			// incoming read from MODE
			Output = 0;
			break;

			
		default:
#ifdef INLINE_DEBUG_READ
			debug << "; Invalid";
#endif
		
			// invalid SIO Register
			cout << "\nhps2x64 ALERT: Unknown SIO READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
			break;
	};
	
#ifdef INLINE_DEBUG_READ
	debug << "; Output =" << hex << Output;
#endif

	return Output;
}




void SIO::Write ( u32 Address, u64 Data, u64 Mask )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nSIO::Write " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << "; Data=" << Data;
#endif

	
	switch ( Address )
	{
		case SIO_LCR:
#ifdef INLINE_DEBUG_WRITE
	debug << "; LCR";
#endif

			
			break;
			
		case SIO_LSR:
#ifdef INLINE_DEBUG_WRITE
	debug << "; LSR";
#endif

			
			break;
			
		case SIO_IER:
#ifdef INLINE_DEBUG_WRITE
	debug << "; IER";
#endif

			
			
			break;

		case SIO_ISR:
#ifdef INLINE_DEBUG_WRITE
	debug << "; ISR";
#endif

			
			break;
			
		case SIO_FCR:
#ifdef INLINE_DEBUG_WRITE
	debug << "; FCR";
#endif

			
			break;

		case SIO_BGR:
#ifdef INLINE_DEBUG_WRITE
	debug << "; BGR";
#endif

			break;
			
		case SIO_TXFIFO:
#ifdef INLINE_DEBUG_WRITE
	debug << "; TXFIFO";
#endif

#ifdef INLINE_DEBUG_CONSOLE
	if ( ( Data & 0xff ) == '\n' )
	{
		debug << "\r\n";
	}
	else
	{
		debug << (char) ( Data & 0xff );
	}
#endif

			// the sio on the TX79/EE/PS2 isn't connected to anything, unless you solder some wires...
			// but it appears that debug info gets output via this port...
			cout << (char) ( Data & 0xff );
			break;
			
		case SIO_RXFIFO:
#ifdef INLINE_DEBUG_WRITE
	debug << "; RXFIFO";
#endif

			break;

			
		default:
#ifdef INLINE_DEBUG_WRITE
			debug << "; Invalid";
#endif
		
			// invalid SIO Register
			cout << "\nhps2x64 ALERT: Unknown SIO WRITE @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data << "\n";
			break;
	};
	
}






void SIO::Update_NextEventCycle ()
{
	//if ( NextEvent_Cycle > *_DebugCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_DebugCycleCount ) ) *_NextSystemEvent = NextEvent_Cycle;
	if ( NextEvent_Cycle < *_NextSystemEvent )
	{
		*_NextSystemEvent = NextEvent_Cycle;
		*_NextEventIdx = NextEvent_Idx;
	}
}


void SIO::SetNextEvent ( u64 Cycle )
{
	NextEvent_Cycle = Cycle + *_DebugCycleCount;
	
	Update_NextEventCycle ();
}

void SIO::Set_NextEventCycle ( u64 Cycle )
{
	NextEvent_Cycle = Cycle;
	
	Update_NextEventCycle ();
}


//void SIO::GetNextEvent ()
//{
//	SetNextEvent ( WaitCycles0 );
//}






