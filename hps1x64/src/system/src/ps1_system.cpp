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



#include "ps1_system.h"

#include "R3000ADebugPrint.h"

#include <sstream>
#include <fstream>
#include "WinApiHandler.h"


#ifdef PS2_COMPILE
#include "ps2_system.h"
#endif

//using namespace Playstation1;
using namespace std;

using namespace x64ThreadSafe::Utilities;
using namespace R3000A::Instruction;


//#define ENABLE_R3000A_IDLE



#define USE_SYSTEM_CYCLE


//#define ENABLE_FIXED_CYCLE_COUNT


#ifdef _DEBUG_VERSION_

// enable inline debugging
#define INLINE_DEBUG_ENABLE


//#define INLINE_DEBUG
//#define INLINE_DEBUG_DEVICE
//#define INLINE_DEBUG_NEXT_EVENT
//#define INLINE_DEBUG_RUN
//#define INLINE_DEBUG_RUN_DEVICES
//#define INLINE_DEBUG_MENU
//#define INLINE_DEBUG_BREAK_VALUECHANGE
//#define INLINE_DEBUG_BREAK_VALUEMATCH


#endif


Debug::Log Playstation1::System::debug;


u32 Playstation1::System::EventFunc_Count = 0;
funcVoid Playstation1::System::EventFunc [ Playstation1::System::c_iMaxEventFunctions ];


Playstation1::System *Playstation1::System::_SYSTEM;
u64 *Playstation1::System::_DebugCycleCount;

u32 Playstation1::System::debug_enabled;
WindowClass::Window *Playstation1::System::DebugWindow;
WindowClass::Window *Playstation1::System::FrameBuffer;
WindowClass::Window *Playstation1::System::SPU_DebugWindow;
WindowClass::Window *Playstation1::System::DMA_DebugWindow;
WindowClass::Window *Playstation1::System::COP2_DebugWindow;


u64* Playstation1::System::_PS2CycleCount;

/*
Playstation1::System::System ()
{
	//cout << "Running SYSTEM constructor...\n";
	printf ( "Running SYSTEM constructor...\n" );
}
*/



Playstation1::System::~System ()
{
	// write memory cards
	//_SIO.Store_MemoryCardFile ( "mcd0", 0 );
	//_SIO.Store_MemoryCardFile ( "mcd1", 1 );

	if ( debug_enabled )
	{
		delete DebugWindow;
		delete FrameBuffer;
	}
}

void Playstation1::System::Reset ()
{
	Debug_BreakPoint_Address = 0xffffffff;
	Debug_RAMDisplayStart = 0;
	
	Debug_CycleBreakPoint_Address = 0xffffffffffffffff;
	Debug_AddressBreakPoint_Address = 0xffffffff;
	
	//_BUS.Reset();
	//_CPU.Reset();
	//_DMA.Reset();
	//_CD.Reset();
	//_GPU.Reset();
	//_INTC.Reset();
	//_MDEC.Reset();
	//_PIO.Reset();
	//_SIO.Reset();
	//_SPU.Reset();
	//_TIMERS.Reset();
}


void Playstation1::System::Start ()
{
#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	// create inline debugger
	debug.Create ( "System_Log.txt" );
	//debug << "\r\nTesting\r\n";
#endif

	cout << "\n\nCalling System::Start\n\n";
	
	_SYSTEM = this;

	
	_SPU._NextSystemEvent = &NextEvent_Cycle;
	_CD._NextSystemEvent = &NextEvent_Cycle;
	_PIO._NextSystemEvent = &NextEvent_Cycle;
	_DMA._NextSystemEvent = &NextEvent_Cycle;
	_TIMERS._NextSystemEvent = &NextEvent_Cycle;
	_GPU._NextSystemEvent = &NextEvent_Cycle;
	_SIO._NextSystemEvent = &NextEvent_Cycle;
	_CPU._NextSystemEvent = &NextEvent_Cycle;
	
#ifdef PS2_COMPILE
	_CDVD._NextSystemEvent = &NextEvent_Cycle;
	_SPU2._NextSystemEvent = &NextEvent_Cycle;
	_USB._NextSystemEvent = &NextEvent_Cycle;
#endif


	// set pointers for debugger
	//_CPU.Breakpoints->RAM = _BUS.MainMemory.b8;
	//_CPU.Breakpoints->BIOS = _BUS.BIOS.b8;
	//_CPU.Breakpoints->DCACHE = _CPU.DCache.b8;
	
	// set pointer in databus for debugging
	_BUS.DebugPC = &(_CPU.PC);

#ifdef INLINE_DEBUG
	debug << "Setting pointer to bus for dma\r\n";
#endif


	// connect devices
	//_DMA.Set_Bus ( &_BUS );
	_CPU.ConnectDevices ( &_BUS, &_SPU.CycleCount );
	_DMA.ConnectDevices ( & _BUS, &_MDEC, & _GPU, &_CD, &_SPU /* , &_CPU */ );
	_TIMERS.ConnectDevices ( &_GPU );
	
	_BUS.ConnectDevices ( &_DMA, &_CD, &_GPU, &_INTC, &_MDEC, &_PIO, &_SIO, &_SPU, &_TIMERS
#ifdef PS2_COMPILE
		, &_USB
#endif
	);
	
	
	// connect debug info
	_CPU.ConnectDebugInfo ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value );
	
	// connect interrupts
	_DMA.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	_GPU.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	_SPU.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	_CD.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	_SIO.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	_PIO.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	_CPU.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 13 ] );
	_INTC.ConnectInterrupt ( & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );

	// testing master interrupt
	_TIMERS.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );

#ifdef PS2_COMPILE
	_CDVD.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	_SPU2.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	_USB.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
#endif

	
	_DMA._DebugPC = & (_CPU.PC);
	_GPU._DebugPC = & (_CPU.PC);
	_TIMERS._DebugPC = & (_CPU.PC);
	_INTC._DebugPC = & (_CPU.PC);
	_SPU._DebugPC = & (_CPU.PC);
	_CD._DebugPC = & (_CPU.PC);
	_SIO._DebugPC = & (_CPU.PC);
	_PIO._DebugPC = & (_CPU.PC);
	_MDEC._DebugPC = & (_CPU.PC);
	_BUS._DebugPC = & (_CPU.PC);
	
#ifdef PS2_COMPILE
	_CDVD._DebugPC = & (_CPU.PC);
	_SPU2._DebugPC = & (_CPU.PC);
	_USB._DebugPC = & (_CPU.PC);
#endif


	
#ifndef USE_SYSTEM_CYCLE

	_DMA._DebugCycleCount = & (_CPU.CycleCount);
	_GPU._DebugCycleCount = & (_CPU.CycleCount);
	_TIMERS._DebugCycleCount = & (_CPU.CycleCount);
	_INTC._DebugCycleCount = & (_CPU.CycleCount);
	_SPU._DebugCycleCount = & (_CPU.CycleCount);
	_CD._DebugCycleCount = & (_CPU.CycleCount);
	_SIO._DebugCycleCount = & (_CPU.CycleCount);
	_PIO._DebugCycleCount = & (_CPU.CycleCount);
	_MDEC._DebugCycleCount = & (_CPU.CycleCount);
	_BUS._DebugCycleCount = & (_CPU.CycleCount);
	//_DebugCycleCount = & (_CPU.CycleCount);
	
#ifdef PS2_COMPILE
	_CDVD._DebugCycleCount = & (_CPU.CycleCount);
	_SPU2._DebugCycleCount = & (_CPU.CycleCount);
	_USB._DebugCycleCount = & (_CPU.CycleCount);
#endif

#else

	_DMA._DebugCycleCount = & (CycleCount);
	_GPU._DebugCycleCount = & (CycleCount);
	_TIMERS._DebugCycleCount = & (CycleCount);
	_INTC._DebugCycleCount = & (CycleCount);
	_SPU._DebugCycleCount = & (CycleCount);
	_CD._DebugCycleCount = & (CycleCount);
	_SIO._DebugCycleCount = & (CycleCount);
	_PIO._DebugCycleCount = & (CycleCount);
	_MDEC._DebugCycleCount = & (CycleCount);
	_BUS._DebugCycleCount = & (CycleCount);
	//_DebugCycleCount = & (_CPU.CycleCount);
	
#ifdef PS2_COMPILE
	_CDVD._DebugCycleCount = & (CycleCount);
	_SPU2._DebugCycleCount = & (CycleCount);
	_USB._DebugCycleCount = & (CycleCount);
#endif

#endif
	
	_DMA._SystemCycleCount = & CycleCount;
	_GPU._SystemCycleCount = & CycleCount;
	_TIMERS._SystemCycleCount = & CycleCount;
	_INTC._SystemCycleCount = & CycleCount;
	_SPU._SystemCycleCount = & CycleCount;
	_CD._SystemCycleCount = & CycleCount;
	_SIO._SystemCycleCount = & CycleCount;
	_PIO._SystemCycleCount = & CycleCount;
	_MDEC._SystemCycleCount = & CycleCount;
	_BUS._SystemCycleCount = & CycleCount;
	
	// ***todo*** need to fix this sometime
	_DebugCycleCount = & CycleCount;
	
#ifdef PS2_COMPILE
	_CDVD._SystemCycleCount = & CycleCount;
	_SPU2._SystemCycleCount = & CycleCount;
	_USB._SystemCycleCount = & CycleCount;
#endif

	// also set for timer object
	Timer::_DebugPC = & (_CPU.PC);
	
#ifndef USE_SYSTEM_CYCLE
	Timer::_DebugCycleCount = & (_CPU.CycleCount);
#else
	Timer::_DebugCycleCount = & (CycleCount);
#endif

	Timer::_SystemCycleCount = & CycleCount;
	

	
	_DMA._NextEventIdx = & (NextEvent_Idx);
	_GPU._NextEventIdx = & (NextEvent_Idx);
	_TIMERS._NextEventIdx = & (NextEvent_Idx);
	//_INTC._NextEventIdx = & (NextEvent_Idx);
	_SPU._NextEventIdx = & (NextEvent_Idx);
	_CD._NextEventIdx = & (NextEvent_Idx);
	_SIO._NextEventIdx = & (NextEvent_Idx);
	_PIO._NextEventIdx = & (NextEvent_Idx);
	//_MDEC._NextEventIdx = & (NextEvent_Idx);
	//_BUS._NextEventIdx = & (NextEvent_Idx);
	
#ifdef PS2_COMPILE
	_CDVD._NextEventIdx = & (NextEvent_Idx);
	_SPU2._NextEventIdx = & (NextEvent_Idx);
	_USB._NextEventIdx = & (NextEvent_Idx);
#endif
	
	

	_CPU.Set_IntCallback ( Intc::UpdateInts );
	_DMA.Set_IntCallback ( Intc::UpdateInts );
	_GPU.Set_IntCallback ( Intc::UpdateInts );
	_TIMERS.Set_IntCallback ( Intc::UpdateInts );
	//_INTC.Set_IntCallback ( &  );
	_SPU.Set_IntCallback ( Intc::UpdateInts );
	_CD.Set_IntCallback ( Intc::UpdateInts );
	_SIO.Set_IntCallback ( Intc::UpdateInts );
	_PIO.Set_IntCallback ( Intc::UpdateInts );
	//_MDEC.Set_IntCallback ( Intc::UpdateInts );
	//_BUS.Set_IntCallback ( &  );
	
#ifdef PS2_COMPILE
	_CDVD.Set_IntCallback ( Intc::UpdateInts );
	_SPU2.Set_IntCallback ( Intc::UpdateInts );
	_USB.Set_IntCallback ( Intc::UpdateInts );
#endif

	


#ifdef PS2_COMPILE
	_INTC.Set_SBusF230 ( & Playstation2::SIF::_SIF->SIFRegs.F230 );
	
	_INTC._ulIdle = & _CPU.ulWaitingForInterrupt;
	
#endif


	
	_DMA._Intc_Stat = & _INTC.I_STAT_Reg.Value;
	_DMA._Intc_Mask = & _INTC.I_MASK_Reg.Value;
	_DMA._R3000a_Status = & _CPU.CPR0.Status.Value;
	
	_GPU.DebugCpuPC = & (_CPU.PC);

	//Reset ();

	// Start Objects //
	_TIMERS.Start ();
	_GPU.Start ();
	_CPU.Start ();
	_BUS.Start ();
	_DMA.Start ();
	_INTC.Start ();
	_SPU.Start ();
	_SIO.Start ();
	_PIO.Start ();
	_MDEC.Start ();
	_CD.Start ();
	
#ifdef PS2_COMPILE
	_CDVD.Start ();
	_SPU2.Start ();
	_USB.Start ();
#endif


	//_CPU.Set_EventCallback ( Register_CallbackFunction );
	_DMA.Set_EventCallback ( Register_CallbackFunction );
	_GPU.Set_EventCallback ( Register_CallbackFunction );
	_TIMERS.Set_EventCallback ( Register_CallbackFunction );
	//_INTC.Set_EventCallback ( &  );
	_SPU.Set_EventCallback ( Register_CallbackFunction );
	_CD.Set_EventCallback ( Register_CallbackFunction );
	_SIO.Set_EventCallback ( Register_CallbackFunction );
	_PIO.Set_EventCallback ( Register_CallbackFunction );
	//_MDEC.Set_EventCallback ( Register_CallbackFunction );
	//_BUS.Set_EventCallback ( &  );
	
#ifdef PS2_COMPILE
	_CDVD.Set_EventCallback ( Register_CallbackFunction );
	_SPU2.Set_EventCallback ( Register_CallbackFunction );
	_USB.Set_EventCallback ( Register_CallbackFunction );
#endif


	// set to run forever by default
	RunUntil_Cycle = -1LL;

	cout << "\nAfter starting PS2 system, PS1CD NextEvent=" << _CD.NextEvent_Cycle;
}


void Playstation1::System::Refresh ()
{
	_TIMERS.Refresh ();
	
	_CPU.Refresh ();

#ifdef PS2_COMPILE
	_SPU2.Refresh ();
#endif
}


void Playstation1::System::Test ( void )
{
	// transfer data from 0x1efb28 into frame buffer at x=0 y=400 96x84
	u32 Source, DestX, DestY, x, y, Index;
	
	//Source = 0x1efb28;
	
	// output contents of NVM file
	Index = 0;
	for ( y = 0; y < 32; y++ )
	{
		cout << "\n";
		for ( x = 0; x < 32; x++ )
		{
#ifdef PS2_COMPILE
			cout << hex << setw ( 2 ) << setfill ( '0' ) << (u32) (((u8*)_CDVD.NVM) [ Index++ ]);
#endif
		}
	}
	
	
	/*
	for ( DestY = 0; DestY < 180; DestY++ )
	{
		for ( DestX = 0; DestX < 640; DestX++ )
		{
			//_GPU.VRAM [ ( DestX + 0 ) + ( ( DestY + 400 ) << 10 ) ] = _BUS.MainMemory.b16 [ Source >> 1 ];
			_GPU.VRAM [ ( DestX + 0 ) + ( ( DestY + 0 ) << 10 ) ] = 0;
			
			Source += 2;
		}
	}
	
	// update the frame buffer window
	_GPU.DebugWindow_Update ();
	*/
}


void Playstation1::System::GetNextEventCycle ( void )
{
#ifdef INLINE_DEBUG_NEXT_EVENT
	debug << "\r\nNextEvents-" << dec <<  " CD=" << _CD.NextEvent_Cycle << " SPU=" << _SPU.NextEvent_Cycle << " DMA=" << _DMA.NextEvent_Cycle << " TIMERS=" << _TIMERS.NextEvent_Cycle << " SIO=" << _SIO.NextEvent_Cycle << " GPU=" << _GPU.NextEvent_Cycle;
#endif

	// initialize the next event cycle (this way I can later separate the components more later)
	NextEvent_Cycle = -1LL;
	
	/*
	if ( _CD.NextEvent_Cycle > *_DebugCycleCount && ( _CD.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _CD.NextEvent_Cycle;
	if ( _SPU.NextEvent_Cycle > *_DebugCycleCount && ( _SPU.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _SPU.NextEvent_Cycle;
	if ( _PIO.NextEvent_Cycle > *_DebugCycleCount && ( _PIO.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _PIO.NextEvent_Cycle;
	if ( _DMA.NextEvent_Cycle > *_DebugCycleCount && ( _DMA.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _DMA.NextEvent_Cycle;
	if ( _TIMERS.NextEvent_Cycle > *_DebugCycleCount && ( _TIMERS.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _TIMERS.NextEvent_Cycle;
	if ( _SIO.NextEvent_Cycle > *_DebugCycleCount && ( _SIO.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _SIO.NextEvent_Cycle;

	// the PS2 does not have a PS1 GPU, but would probably still need to update timers
	if ( _GPU.NextEvent_Cycle > *_DebugCycleCount && ( _GPU.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _GPU.NextEvent_Cycle;
	
#ifdef PS2_COMPILE
	if ( _CDVD.NextEvent_Cycle > *_DebugCycleCount && ( _CDVD.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _CDVD.NextEvent_Cycle;
	if ( _SPU2.NextEvent_Cycle > *_DebugCycleCount && ( _SPU2.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _SPU2.NextEvent_Cycle;
	if ( _USB.NextEvent_Cycle > *_DebugCycleCount && ( _USB.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _USB.NextEvent_Cycle;
#endif
	*/

	if ( _CD.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _CD.NextEvent_Cycle; NextEvent_Idx = _CD.NextEvent_Idx; }
	if ( _SPU.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _SPU.NextEvent_Cycle; NextEvent_Idx = _SPU.NextEvent_Idx; }
	if ( _PIO.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _PIO.NextEvent_Cycle; NextEvent_Idx = _PIO.NextEvent_Idx; }
	if ( _DMA.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _DMA.NextEvent_Cycle; NextEvent_Idx = _DMA.NextEvent_Idx; }
	if ( _TIMERS.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _TIMERS.NextEvent_Cycle; NextEvent_Idx = _TIMERS.NextEvent_Idx; }
	if ( _SIO.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _SIO.NextEvent_Cycle; NextEvent_Idx = _SIO.NextEvent_Idx; }

	// the PS2 does not have a PS1 GPU, but would probably still need to update timers
	if ( _GPU.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _GPU.NextEvent_Cycle; NextEvent_Idx = _GPU.NextEvent_Idx; }
	
#ifdef PS2_COMPILE
	if ( _CDVD.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _CDVD.NextEvent_Cycle; NextEvent_Idx = _CDVD.NextEvent_Idx; }
	if ( _SPU2.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _SPU2.NextEvent_Cycle; NextEvent_Idx = _SPU2.NextEvent_Idx; }
	if ( _USB.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _USB.NextEvent_Cycle; NextEvent_Idx = _USB.NextEvent_Idx; }
	
	//if ( ( Playstation2::System::_SYSTEM->NextEvent_Cycle >> 2 ) < NextEvent_Cycle ) { NextEvent_Cycle = ( Playstation2::System::_SYSTEM->NextEvent_Cycle >> 2 ); NextEvent_Idx = Playstation2::System::_SYSTEM->NextEvent_Idx2; }
	//if ( ( Playstation2::System::_SYSTEM->_CPU.CycleCount >> 2 ) < NextEvent_Cycle ) { NextEvent_Cycle = ( Playstation2::System::_SYSTEM->_CPU.CycleCount >> 2 ); NextEvent_Idx = Playstation2::System::_SYSTEM->NextEvent_Idx2; }
	ullStopCycle = NextEvent_Cycle;
	if ( ( Playstation2::System::_SYSTEM->NextEvent_Cycle >> 2 ) < ullStopCycle ) { ullStopCycle = ( Playstation2::System::_SYSTEM->NextEvent_Cycle >> 2 ); }
	if ( ( Playstation2::System::_SYSTEM->_CPU.CycleCount >> 2 ) < ullStopCycle ) { ullStopCycle = ( Playstation2::System::_SYSTEM->_CPU.CycleCount >> 2 ); }
	
#endif
}


void Playstation1::System::RunDevices ()
{
#ifdef INLINE_DEBUG_DEVICE
	debug << "\r\nPS1System::RunDevices; Cycle#" << dec << CycleCount;
	debug << " NextEvent_Idx=" << NextEvent_Idx;
	debug << " EventFunc [ NextEvent_Idx ]=" << hex << (u64) (EventFunc [ NextEvent_Idx ]);
#endif

/*
#ifdef INLINE_DEBUG_DEVICE
	debug << "; TIMERS";
#endif

		// *important* must do timer interrupt events first
		// this is because the GPU updates the timers, but you must catch the timer transition so you don't miss it
		// would probably make more sense for the timers to update themselves every so often, though
		_TIMERS.Run ();

#ifdef INLINE_DEBUG_DEVICE
	debug << "; GPU";
#endif

		// ps2 does not have a ps1 gpu, but would still need to update timers
		_GPU.Run ();


// only run CD and SPU1 events if not compiling for ps2 for now
#ifndef PS2_COMPILE

#ifdef INLINE_DEBUG_DEVICE
	debug << "; CD";
#endif

		_CD.Run ();

#ifdef INLINE_DEBUG_DEVICE
	debug << "; SPU";
#endif

		_SPU.Run ();
		
#endif



#ifdef INLINE_DEBUG_DEVICE
	debug << "; PIO";
#endif

		_PIO.Run ();
	
#ifdef INLINE_DEBUG_DEVICE
	debug << "; DMA";
#endif

		_DMA.Run ();
	
#ifdef INLINE_DEBUG_DEVICE
	debug << "; SIO";
#endif

		_SIO.Run ();


#ifdef PS2_COMPILE

#ifdef INLINE_DEBUG_DEVICE
	debug << "; CDVD";
#endif

		_CDVD.Run ();

#ifdef INLINE_DEBUG_DEVICE
	debug << "; SPU2";
#endif

		_SPU2.Run ();

#ifdef INLINE_DEBUG_DEVICE
	debug << "; USB";
#endif

		_USB.Run ();
		
#endif
*/

		EventFunc [ NextEvent_Idx ] ();

		// the cycle number for the next event
		GetNextEventCycle ();
		
#ifdef INLINE_DEBUG_DEVICE
	debug << " NextEvent_Cycle=" << NextEvent_Cycle;
#endif

}


void Playstation1::System::Run_Cycles ( u64 Cycles )
{
	Cycles += *_DebugCycleCount;
	
	do
	{
		Run ();
	}
	while ( *_DebugCycleCount < Cycles );
}





// must call this before the cycle count/bus is updated
// for ps2, this would also happen BEFORE a dma#10 transfer is executed
void Playstation1::System::RunEvents ()
{
	
	
#ifdef PS2_COMPILE
#ifdef ENABLE_R3000A_IDLE
	if ( _CPU.ulWaitingForInterrupt )
	{
		//if ( _CPU.bEnable_SkipIdleCycles )
		//{
			if ( _CPU.CycleCount < ( ( *_PS2CycleCount ) >> 2 ) )
			{
				_CPU.CycleCount = ( *_PS2CycleCount ) >> 2;
			}
		//}
	}
#endif
#endif
	
#ifdef ENABLE_FIXED_CYCLE_COUNT
	u64 tCycleCount;
	
	// no need to check if bus is busy here, since the CPU can just add memory latencies onto cycle timing
	// just in case for now
	// can be compiled with ps1 for testing or consistency, but MUST be compiled with ps2 probably
	
	
	// save the cycle to run events until
	tCycleCount = _CPU.CycleCount;
#endif
	
// if compiling for ps2, need this for now since transfer can come from SIF at any time
/*
#ifdef PS2_COMPILE
	// update the CPU cycle count based on bus activity
	// check if the bus is occupied
	//if ( _CPU.CycleCount < _BUS.BusyUntil_Cycle )
	if ( tCycleCount < _BUS.BusyUntil_Cycle )
	{
		//cout << "\nhps1x64 ALERT: CycleCount has not reached BUS BusyUntil_Cycle. Cycle#" << dec << _CPU.CycleCount << "\n";
		
		// if the bus is busy, on a PS1, the cpu waits
		//_CPU.CycleCount = _BUS.BusyUntil_Cycle;
		tCycleCount = _BUS.BusyUntil_Cycle;
	}
#endif
*/

	/*
	if ( NextEvent_Cycle <= tCycleCount )
	{
#ifdef INLINE_DEBUG_DEVICE
	debug << "\r\nPS1System::RunEvents";
	debug << " SystemCycle#" << dec << CycleCount;
	debug << " CPUCycle#" << dec << _CPU.CycleCount;
#endif
	}
	*/
	
	// only run components when they are busy with something
	// should run event for cycle before running cpu for cycle
#ifdef ENABLE_FIXED_CYCLE_COUNT
	while ( NextEvent_Cycle <= tCycleCount )
#else
	while ( NextEvent_Cycle <= _CPU.CycleCount )
#endif
	{
#ifdef INLINE_DEBUG_RUN_DEVICES
	debug << "\nRunDevicesLoop";
	debug << " System:NextEvent_Cycle=" << dec << NextEvent_Cycle;
	debug << " CPU:CycleCount=" << dec << _CPU.CycleCount;
#endif

		// save the cycle to run events until
		//tCycleCount = _CPU.CycleCount;

		//do
		//{
			// check if the next event is past where we can run system until
			// can be compiled with ps1 for testing or consistency, but MUST be compiled with ps2 probably
			
			// set the cycle count as the cycle the event that was skipped was supposed to happen at
//#ifndef USE_SYSTEM_CYCLE
//			_CPU.CycleCount = NextEvent_Cycle;
//#endif
			CycleCount = NextEvent_Cycle;
			
			RunDevices ();
				
			
			// check if the bus is busy (do this before running DMA device to reach current cycle first)
			// can't do this for ps2, because need to only run until specified cycle to synchronize with R5900
			//if ( _CPU.CycleCount < _BUS.BusyUntil_Cycle )
			/*
			if ( tCycleCount < _BUS.BusyUntil_Cycle )
			{
				// if the bus is busy, on a PS1, the cpu waits
				tCycleCount = _BUS.BusyUntil_Cycle;
				//_CPU.CycleCount = _BUS.BusyUntil_Cycle;
			}
			*/
		
		// } while ( NextEvent_Cycle <= tCycleCount );
		
		// restore the current cycle count for CPU
		//_CPU.CycleCount = tCycleCount;
	}

	// this must be here so that the current system cycle is at the cpu cycle
	//CycleCount = _CPU.CycleCount;
	// restore the current cycle count for CPU
	//_CPU.CycleCount = tCycleCount;
#ifdef ENABLE_FIXED_CYCLE_COUNT
	CycleCount = tCycleCount;
#else
	CycleCount = _CPU.CycleCount;
#endif
	
#ifdef INLINE_DEBUG_RUN_DEVICES
	debug << "\nRunDevicesLoop->DONE";
	debug << " System:NextEvent_Cycle=" << dec << NextEvent_Cycle;
	debug << " CPU:CycleCount=" << dec << _CPU.CycleCount;
#endif
}

void Playstation1::System::Run ()
{
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nPlaystation1::System::Run; CycleCount=" << dec << *_DebugCycleCount << " NextEvent_Cycle=" << NextEvent_Cycle;
#endif


	// run the events
	// need to do this before anything else in case the bus has suspended the cpu before it is run
	RunEvents ();


#ifdef INLINE_DEBUG_RUN
	debug << " CPU";
#endif


#ifdef PS2_COMPILE
#ifdef ENABLE_R3000A_IDLE
	if ( !_CPU.ulWaitingForInterrupt )
	{
#endif
#endif


	_CPU.Run ();
	

#ifdef PS2_COMPILE
#ifdef ENABLE_R3000A_IDLE
	}
#endif
#endif

}



///////////////////////////////////////////////////////
// Loads a test program into bios
// returns true on success, false otherwise
bool Playstation1::System::LoadTestProgramIntoBios ( const char* FilePath )
{
	//static const long BIOS_SIZE_IN_BYTES = 524288;

#ifdef INLINE_DEBUG
	debug << "\r\nEntered function: LoadTestProgram";
#endif

	u32 Code;
	u32 Address = 0;
	ifstream InputFile ( FilePath, ios::binary );
	
	if ( !InputFile )
	{
#ifdef INLINE_DEBUG
	debug << "->Error opening input file into memory/bios";
#endif

		cout << "Error opening test R3000A code.\n";
		return false;
	}


#ifdef INLINE_DEBUG
	debug << "; Reading input file into memory";
#endif

	// write entire program into memory
	//InputFile.read ( (char*) ( _BUS.BIOS.b32 ), BIOS_SIZE_IN_BYTES );
	InputFile.read ( (char*) ( _BUS.BIOS.b32 ), _BUS.BIOS_Size );
	
	InputFile.close();
	
#ifdef INLINE_DEBUG
	debug << "->Leaving function: LoadTestProgram";
#endif
	
	return true;
}




