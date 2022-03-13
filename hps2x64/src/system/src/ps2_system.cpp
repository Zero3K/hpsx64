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



#include "ps2_system.h"

#include "R5900_Print.h"

#include <sstream>
#include <fstream>

#ifdef ENABLE_GUI_DEBUGGER
#include "WinApiHandler.h"
#endif


//using namespace Playstation2;
using namespace std;

//using namespace x64ThreadSafe::Utilities;
using namespace R5900::Instruction;



//#define ENABLE_R3000A_IDLE
//#define ENABLE_R5900_IDLE



#define USE_SYSTEM_CYCLE


#ifdef _DEBUG_VERSION_

// enable inline debugging
#define INLINE_DEBUG_ENABLE

/*
#define INLINE_DEBUG
#define INLINE_DEBUG_RUN
#define INLINE_DEBUG_RUNEVENTS
#define INLINE_DEBUG_DEVICE
//#define INLINE_DEBUG_NEXTEVENT
//#define INLINE_DEBUG_MENU
//#define INLINE_DEBUG_BREAK_VALUECHANGE
//#define INLINE_DEBUG_BREAK_VALUEMATCH
*/

#endif


u32 Playstation2::System::EventFunc_Count = 0;
funcVoid Playstation2::System::EventFunc [ Playstation2::System::c_iMaxEventFunctions ];


#ifdef ENABLE_GUI_DEBUGGER
Debug::Log Playstation2::System::debug;
#endif


Playstation2::System *Playstation2::System::_SYSTEM;
u64 *Playstation2::System::_DebugCycleCount;


#ifdef ENABLE_GUI_DEBUGGER
u32 Playstation2::System::debug_enabled;
WindowClass::Window *Playstation2::System::DebugWindow;
WindowClass::Window *Playstation2::System::FrameBuffer;
WindowClass::Window *Playstation2::System::SPU_DebugWindow;
WindowClass::Window *Playstation2::System::DMA_DebugWindow;
WindowClass::Window *Playstation2::System::COP2_DebugWindow;
#endif


Playstation2::System::System ()
{
	cout << "Running SYSTEM constructor...\n";
	
/*
#ifdef INLINE_DEBUG_ENABLE
	// create inline debugger
	debug.Create ( "System_Log.txt" );
#endif

	
	_SYSTEM = this;
	
	// set pointer in databus for debugging
	_BUS.DebugPC = &(_CPU.PC);

#ifdef INLINE_DEBUG
	debug << "Setting pointer to bus for dma\r\n";
#endif

	// connect devices
	//_DMA.Set_Bus ( &_BUS );
	_CPU.ConnectDevices ( &_BUS );
	_BUS.ConnectDevices ( &_DMA, &_CD, &_GPU, &_INTC, &_MDEC, &_PIO, &_SIO, &_SPU, &_TIMERS );
	_DMA.ConnectDevices ( & _BUS, & _GPU );
	_TIMERS.ConnectDevices ( &_GPU );
	
	// connect debug info
	_CPU.ConnectDebugInfo ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value );
	
	_DMA._DebugPC = & (_CPU.PC);
	_GPU._DebugPC = & (_CPU.PC);
	_TIMERS._DebugPC = & (_CPU.PC);
	_INTC._DebugPC = & (_CPU.PC);
	_SPU._DebugPC = & (_CPU.PC);
	_CD._DebugPC = & (_CPU.PC);
	_SIO._DebugPC = & (_CPU.PC);
	_PIO._DebugPC = & (_CPU.PC);
	
	_DMA._DebugCycleCount = & (_CPU.CycleCount);
	_GPU._DebugCycleCount = & (_CPU.CycleCount);
	_TIMERS._DebugCycleCount = & (_CPU.CycleCount);
	_INTC._DebugCycleCount = & (_CPU.CycleCount);
	_SPU._DebugCycleCount = & (_CPU.CycleCount);
	_CD._DebugCycleCount = & (_CPU.CycleCount);
	_SIO._DebugCycleCount = & (_CPU.CycleCount);
	_PIO._DebugCycleCount = & (_CPU.CycleCount);
	
	
	_GPU.DebugCpuPC = & (_CPU.PC);
	
	Reset ();
	*/
	
	cout << "Exiting SYSTEM constructor...\n";
}




Playstation2::System::~System ()
{
	// write memory cards
	//_SIO.Store_MemoryCardFile ( "mcd0", 0 );
	//_SIO.Store_MemoryCardFile ( "mcd1", 1 );

#ifdef ENABLE_GUI_DEBUGGER
	if ( debug_enabled )
	{
		delete DebugWindow;
		delete FrameBuffer;
	}
#endif
}

void Playstation2::System::Reset ()
{
	Debug_BreakPoint_Address = 0xffffffff;
	Debug_RAMDisplayStart = 0;
	
	Debug_CycleBreakPoint_Address = 0xffffffffffffffff;
	Debug_AddressBreakPoint_Address = 0xffffffff;
	
	_BUS.Reset();
	_CPU.Reset();
	_DMA.Reset();
	_GPU.Reset();
	_INTC.Reset();
	//_MDEC.Reset();
	//_PIO.Reset();
	_SIF.Reset();
	_SIO.Reset();
	_TIMERS.Reset();
	
	_VU0.Reset ();
	_VU1.Reset ();
	
	_IPU.Reset ();
}


void Playstation2::System::Start ()
{
#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	// create inline debugger
	debug.Create ( "PS2System_Log.txt" );
#endif

	cout << "Running Playstation2::System::Start...\n";
	
	
	_SYSTEM = this;

	cout << "Setting next event pointers...\n";
	
	//_PIO._NextSystemEvent = &NextEvent_Cycle;
	_DMA._NextSystemEvent = &NextEvent_Cycle;
	_TIMERS._NextSystemEvent = &NextEvent_Cycle;
	_GPU._NextSystemEvent = &NextEvent_Cycle;
	_SIO._NextSystemEvent = &NextEvent_Cycle;
	_CPU._NextSystemEvent = &NextEvent_Cycle;
	_SIF._NextSystemEvent = &NextEvent_Cycle;
	_IPU._NextSystemEvent = &NextEvent_Cycle;

	VU::_NextSystemEvent = &NextEvent_Cycle;
	
	// set pointers for debugger
	//_CPU.Breakpoints->RAM = _BUS.MainMemory.b8;
	//_CPU.Breakpoints->BIOS = _BUS.BIOS.b8;
	//_CPU.Breakpoints->DCACHE = _CPU.DCache.b8;
	
	// set pointer in databus for debugging
	_BUS._DebugPC = &(_CPU.PC);

#ifdef INLINE_DEBUG
	debug << "Setting pointer to bus for dma\r\n";
#endif

	cout << "Setting external signals...\n";
	
	// connect external signals
	
	// timers need a signal from GPU to know when hblank/vblank etc gets hit on PS2
	_TIMERS.ConnectExternalSignal_GPU ( & _GPU.llScanlineStart, & _GPU.llNextScanlineStart, & _GPU.llHBlankStart, & _GPU.lScanline, & _GPU.lNextScanline, & _GPU.lVBlank, & _GPU.lMaxScanline );
	
#ifdef USE_SYSTEM_CYCLE
	_TIMERS.ConnectExternalSignal_Clock ( & CycleCount );
#else
	// this should actually be the clock for the bus
	_TIMERS.ConnectExternalSignal_Clock ( & _CPU.CycleCount );
#endif
	

	// connect devices
	//_DMA.Set_Bus ( &_BUS );
	_CPU.ConnectDevices ( &_BUS );
	_DMA.ConnectDevices ( & _SIF.SIFRegs.F240, &_CPU.CPCOND0 );
	//_BUS.ConnectDevices ( &_DMA, &_CD, &_GPU, &_INTC, &_MDEC, &_PIO, &_SIO, &_SPU, &_TIMERS );
	//_TIMERS.ConnectDevices ( &_GPU );
	
	// connect debug info
	_CPU.ConnectDebugInfo ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value );
	
	// connect interrupts
	_DMA.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	_GPU.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	_TIMERS.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	//_SIO.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	//_PIO.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	_IPU.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	_CPU.ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 13 ] );
	_INTC.ConnectInterrupt ( & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	
	VU::ConnectInterrupt ( & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );
	
	_SIF.ConnectInterrupt ( & _PS1SYSTEM._INTC.I_STAT_Reg.Value, & _PS1SYSTEM._INTC.I_MASK_Reg.Value, & _INTC.I_STAT_Reg.Value, & _INTC.I_MASK_Reg.Value, & _PS1SYSTEM._CPU.CPR0.Regs [ 12 ], & _PS1SYSTEM._CPU.CPR0.Regs [ 13 ], & _PS1SYSTEM._CPU.Status.Value, & _CPU.CPR0.Regs [ 12 ], & _CPU.CPR0.Regs [ 13 ], &_CPU.Status.Value );

	DataBus::Connect_VuMem ( _VU0.VU0.MicroMem64, _VU0.VU0.VuMem64, _VU1.VU1.MicroMem64, _VU1.VU1.VuMem64 );
	
	_DMA._DebugPC = & (_CPU.PC);
	_GPU._DebugPC = & (_CPU.PC);
	_TIMERS._DebugPC = & (_CPU.PC);
	_INTC._DebugPC = & (_CPU.PC);
	_SIF._DebugPC = & (_CPU.PC);
	_SIO._DebugPC = & (_CPU.PC);
	_IPU._DebugPC = & (_CPU.PC);
	//_PIO._DebugPC = & (_CPU.PC);
	//_MDEC._DebugPC = & (_CPU.PC);
	_BUS._DebugPC = & (_CPU.PC);
	VU::_DebugPC = & (_CPU.PC);

#ifdef USE_SYSTEM_CYCLE
	_DMA._DebugCycleCount = & (CycleCount);
	_GPU._DebugCycleCount = & (CycleCount);
	_TIMERS._DebugCycleCount = & (CycleCount);
	_INTC._DebugCycleCount = & (CycleCount);
	_IPU._DebugCycleCount = & (CycleCount);
	_SIF._DebugCycleCount = & (CycleCount);
	_SIO._DebugCycleCount = & (CycleCount);
	//_PIO._DebugCycleCount = & (CycleCount);
	//_MDEC._DebugCycleCount = & (CycleCount);
	_BUS._DebugCycleCount = & (CycleCount);
	_DebugCycleCount = & (CycleCount);
	VU::_DebugCycleCount = & (CycleCount);
#else
	_DMA._DebugCycleCount = & (_CPU.CycleCount);
	_GPU._DebugCycleCount = & (_CPU.CycleCount);
	_TIMERS._DebugCycleCount = & (_CPU.CycleCount);
	_INTC._DebugCycleCount = & (_CPU.CycleCount);
	_IPU._DebugCycleCount = & (_CPU.CycleCount);
	_SIF._DebugCycleCount = & (_CPU.CycleCount);
	_SIO._DebugCycleCount = & (_CPU.CycleCount);
	//_PIO._DebugCycleCount = & (_CPU.CycleCount);
	//_MDEC._DebugCycleCount = & (_CPU.CycleCount);
	_BUS._DebugCycleCount = & (_CPU.CycleCount);
	_DebugCycleCount = & (_CPU.CycleCount);
	VU::_DebugCycleCount = & (_CPU.CycleCount);
#endif


	_DMA._NextEventIdx = & (NextEvent_Idx);
	_GPU._NextEventIdx = & (NextEvent_Idx);
	_TIMERS._NextEventIdx = & (NextEvent_Idx);
	//_INTC._NextEventIdx = & (NextEvent_Idx);
	_SIF._NextEventIdx = & (NextEvent_Idx);
	//_SIO._NextEventIdx = & (NextEvent_Idx);
	_IPU._NextEventIdx = & (NextEvent_Idx);
	//_PIO._NextEventIdx = & (NextEvent_Idx);
	//_BUS._NextEventIdx = & (NextEvent_Idx);
	VU::_NextEventIdx = & (NextEvent_Idx);


	
	//_DMA._Intc_Stat = & _INTC.I_STAT_Reg.Value;
	//_DMA._Intc_Mask = & _INTC.I_MASK_Reg.Value;
	//_DMA._R5900_Status = & _CPU.CPR0.Status.Value;
	
	//_GPU.DebugCpuPC = & (_CPU.PC);

	//Reset ();

	
	// Start Objects //
	_CPU.Start ();
	_TIMERS.Start ();
	_GPU.Start ();
	_DMA.Start ();
	_INTC.Start ();
	_SIF.Start ();
	_SIO.Start ();
	_IPU.Start ();
	//_PIO.Start ();
	//_MDEC.Start ();
	_VU0.Start ();
	_VU1.Start ();
	
	// put this last so that it can reference static pointers in the other objects for now
	_BUS.Start ();
	
#ifndef EE_ONLY_COMPILE
	// playstation 1 (iop/sbus)
	_PS1SYSTEM.Reset ();
	_PS1SYSTEM.Start ();
#endif


	_DMA.Set_EventCallback ( Register_CallbackFunction );
	_GPU.Set_EventCallback ( Register_CallbackFunction );
	_TIMERS.Set_EventCallback ( Register_CallbackFunction );
	//_INTC.Set_EventCallback ( &  );
	//_SIO.Set_EventCallback ( Register_CallbackFunction );
	_SIF.Set_EventCallback ( Register_CallbackFunction );
	_IPU.Set_EventCallback ( Register_CallbackFunction );

	
	_VU0.Set_EventCallback ( Register_CallbackFunction );
	_VU1.Set_EventCallback ( Register_CallbackFunction );
	
	
	_PS1SYSTEM.Set_EventCallback ( Register_CallbackFunction );
	
	_PS1SYSTEM._PS2CycleCount = & _CPU.CycleCount;
	
	_INTC._ulIdle = & _CPU.ulIdle;
	
	cout << "\nAfter starting PS2 system, PS1CD NextEvent=" << _PS1SYSTEM._CD.NextEvent_Cycle;

}


void Playstation2::System::Refresh ()
{
	_TIMERS.Refresh ();
	_CPU.Refresh ();
	_GPU.Refresh ();
	_PS1SYSTEM.Refresh ();
}


void Playstation2::System::Test ( void )
{
	/*
	// transfer data from 0x1efb28 into frame buffer at x=0 y=400 96x84
	u32 Source, DestX, DestY;
	
	//Source = 0x1efb28;
	
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


void Playstation2::System::GetNextEventCycle ( void )
{
#ifdef INLINE_DEBUG_NEXTEVENT
	debug << "\r\nPlaystation2::System::GetNextEventCycle; CycleCount=" << dec << _CPU.CycleCount;
	debug << " (before) NextEvent_Cycle=" << NextEvent_Cycle << " " << NextEvent_Idx;
	debug << " DMA=" << _DMA.NextEvent_Cycle << " " << _DMA.NextEvent_Idx;
	debug << " TIMERS=" << _TIMERS.NextEvent_Cycle << " " << _TIMERS.NextEvent_Idx;
	debug << " GPU=" << _GPU.NextEvent_Cycle << " " << _GPU.NextEvent_Idx;
	debug << " IPU=" << _IPU.NextEvent_Cycle << " " << _IPU.NextEvent_Idx;
#endif

	// initialize the next event cycle (this way I can later separate the components more later)
	NextEvent_Cycle = -1LL;
	
	/*
	//if ( _PIO.NextEvent_Cycle > *_DebugCycleCount && ( _PIO.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _PIO.NextEvent_Cycle;
	if ( _DMA.NextEvent_Cycle > *_DebugCycleCount && ( _DMA.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _DMA.NextEvent_Cycle;
	if ( _TIMERS.NextEvent_Cycle > *_DebugCycleCount && ( _TIMERS.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _TIMERS.NextEvent_Cycle;
	if ( _GPU.NextEvent_Cycle > *_DebugCycleCount && ( _GPU.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _GPU.NextEvent_Cycle;
	//if ( _SIO.NextEvent_Cycle > *_DebugCycleCount && ( _SIO.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _SIO.NextEvent_Cycle;
	if ( _SIF.NextEvent_Cycle > *_DebugCycleCount && ( _SIF.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _SIF.NextEvent_Cycle;
	if ( _IPU.NextEvent_Cycle > *_DebugCycleCount && ( _IPU.NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) ) NextEvent_Cycle = _IPU.NextEvent_Cycle;
	*/

	
	if ( _DMA.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _DMA.NextEvent_Cycle; NextEvent_Idx = _DMA.NextEvent_Idx; }
	if ( _TIMERS.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _TIMERS.NextEvent_Cycle; NextEvent_Idx = _TIMERS.NextEvent_Idx; }
	if ( _GPU.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _GPU.NextEvent_Cycle; NextEvent_Idx = _GPU.NextEvent_Idx; }
	if ( _SIF.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _SIF.NextEvent_Cycle; NextEvent_Idx = _SIF.NextEvent_Idx; }
	if ( _IPU.NextEvent_Cycle < NextEvent_Cycle ) { NextEvent_Cycle = _IPU.NextEvent_Cycle; NextEvent_Idx = _IPU.NextEvent_Idx; }


	if ( _VU0.VU0.CycleCount < NextEvent_Cycle ) { NextEvent_Cycle = _VU0.VU0.CycleCount; NextEvent_Idx = _VU0.VU0.NextEvent_Idx; }

	// for vu#1, using eCycleCount now due to multi-threading
	//if ( _VU1.VU1.CycleCount < NextEvent_Cycle ) { NextEvent_Cycle = _VU1.VU1.CycleCount; NextEvent_Idx = _VU1.VU1.NextEvent_Idx; }
	if ( _VU1.VU1.eCycleCount < NextEvent_Cycle ) { NextEvent_Cycle = _VU1.VU1.eCycleCount; NextEvent_Idx = _VU1.VU1.NextEvent_Idx; }
	
	
	if ( ( _PS1SYSTEM.NextEvent_Cycle << 2 ) < NextEvent_Cycle ) { NextEvent_Cycle = ( _PS1SYSTEM.NextEvent_Cycle << 2 ); NextEvent_Idx = _PS1SYSTEM.NextEvent_Idx2; }
	
#ifdef ENABLE_R3000A_IDLE
	if ( !_PS1SYSTEM._CPU.ulWaitingForInterrupt )
	{
		//if ( _PS1SYSTEM._CPU.bEnable_SkipIdleCycles )
		//{

#endif

	if ( ( _PS1SYSTEM._CPU.CycleCount << 2 ) < NextEvent_Cycle ) { NextEvent_Cycle = ( _PS1SYSTEM._CPU.CycleCount << 2 ); NextEvent_Idx = _PS1SYSTEM.NextEvent_Idx2; }
	
#ifdef ENABLE_R3000A_IDLE
		//}
	}
#endif


	
#ifdef INLINE_DEBUG_NEXTEVENT
	debug << " (after) NextEvent_Cycle=" << NextEvent_Cycle;
#endif
}


void Playstation2::System::RunDevices ()
{
#ifdef INLINE_DEBUG_DEVICE
	debug << "\r\nPlaystation2::System::RunDevices; CycleCount=" << dec << _CPU.CycleCount;
	debug << " NextEvent_Idx=" << dec << NextEvent_Idx;
	//debug << "\r\nPS1CD NextEvent=" << dec << _PS1SYSTEM._CD.NextEvent_Cycle;
#endif

/*
#ifdef INLINE_DEBUG_DEVICE
	debug << "; TIMERS";
	debug << " GNE=" << _GPU.NextEvent_Cycle;
#endif

		// *important* must do timer interrupt events first
		// this is because the GPU updates the timers, but you must catch the timer transition so you don't miss it
		// would probably make more sense for the timers to update themselves every so often, though
		_TIMERS.Run ();
		
#ifdef INLINE_DEBUG_DEVICE
	debug << "; GPU";
	debug << " GNE=" << _GPU.NextEvent_Cycle;
#endif

		_GPU.Run ();



//#ifdef INLINE_DEBUG_DEVICE
//	debug << "; PIO";
//#endif

		//_PIO.Run ();
	
	
//#ifdef INLINE_DEBUG_DEVICE
//	debug << "; SIO";
//#endif

		//_SIO.Run ();


#ifdef INLINE_DEBUG_DEVICE
	debug << "; DMA";
	debug << " GNE=" << _GPU.NextEvent_Cycle;
#endif

		_DMA.Run ();

#ifdef INLINE_DEBUG_DEVICE
	debug << "; IPU";
	debug << " GNE=" << _GPU.NextEvent_Cycle;
#endif

		// this has to run after dma for now, since it might initiate transfers in run function
		_IPU.Run ();


#ifdef INLINE_DEBUG_DEVICE
	debug << "; SIF";
	debug << " GNE=" << _GPU.NextEvent_Cycle;
#endif

		_SIF.Run ();
*/

		
		EventFunc [ NextEvent_Idx ] ();

		// the cycle number for the next event
		GetNextEventCycle ();
}



// must call this before the cycle count/bus is updated
// for ps2, this would also happen BEFORE a dma#10 transfer is executed
void Playstation2::System::RunEvents ()
{
#ifdef INLINE_DEBUG_RUNEVENTS
	debug << "\r\nRunEvents CpuCycle#" << dec << _CPU.CycleCount << " SystemCycle#" << dec << CycleCount << " NextEventCycle#" << dec << NextEvent_Cycle;
#endif

	/*
	u64 tCycleCount;
	
	// no need to check if bus is busy here, since the CPU can just add memory latencies onto cycle timing
	// just in case for now
	// can be compiled with ps1 for testing or consistency, but MUST be compiled with ps2 probably
	
	// save the cycle to run events until
	tCycleCount = _CPU.CycleCount;
	
// if compiling for ps2, need this for now since transfer can come from SIF at any time
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

#ifdef ENABLE_R5900_IDLE
	if ( _CPU.ulIdle )
	{
		if ( _CPU.bEnable_SkipIdleCycles )
		{
			if ( _CPU.CycleCount < NextEvent_Cycle )
			{
				_CPU.CycleCount = NextEvent_Cycle;
			}
		}
	}
#endif

	
	// only run components when they are busy with something
	// should run event for cycle before running cpu for cycle
	//while ( NextEvent_Cycle <= tCycleCount )
	while ( NextEvent_Cycle <= _CPU.CycleCount )
	{
#ifdef INLINE_DEBUG_RUN
	debug << " RunDevices";
	debug << " NextEvent_Cycle=" << dec << NextEvent_Cycle;
	debug << " _CPU.CycleCount=" << dec << _CPU.CycleCount;
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
				
			
			/*
			// check if the bus is busy (do this before running DMA device to reach current cycle first)
			// can't do this for ps2, because need to only run until specified cycle to synchronize with R5900
			//if ( _CPU.CycleCount < _BUS.BusyUntil_Cycle )
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
	//CycleCount = tCycleCount;
	CycleCount = _CPU.CycleCount;
	
}



void Playstation2::System::Run_Cycles ( u64 Cycles )
{
	Cycles += *_DebugCycleCount;
	
	do
	{
		Run ();
	}
	while ( *_DebugCycleCount < Cycles );
}


void Playstation2::System::Run ()
{
#ifdef INLINE_DEBUG
	debug << "\r\nPlaystation2::System::Run; CycleCount=" << dec << *_DebugCycleCount;
	debug << " NextEvent_Cycle=" << NextEvent_Cycle;
#endif

	// only run components when they are busy with something
	//if ( NextEvent_Cycle == *_DebugCycleCount )
	//{
	//	RunDevices ();
	//}
	
	
	RunEvents ();
	

#ifdef INLINE_DEBUG
	debug << "; CPU";
#endif

	_CPU.Run ();


	/*
#ifndef EE_ONLY_COMPILE
	// run the ps1 also - runs at 1/4 the ps2 bus speed
	//if ( ( ( *_DebugCycleCount ) >> 2 ) > ( *_PS1SYSTEM._DebugCycleCount ) ) _PS1SYSTEM.Run ( ( *_DebugCycleCount ) >> 2 );

#ifdef SKIP_PS1_ON_IDLE
	if ( ( ( *_DebugCycleCount ) >> 2 ) > _PS1SYSTEM._CPU.BusyUntil_Cycle )
	{
#endif

		while ( ( ( *_DebugCycleCount ) >> 2 ) > ( *_PS1SYSTEM._DebugCycleCount ) ) _PS1SYSTEM.Run ();
		
#ifdef SKIP_PS1_ON_IDLE
	}
	else
	{
		// update the cycles for ps1 system and its cpu and continue
		_PS1SYSTEM.CycleCount = ( ( *_DebugCycleCount ) >> 2 );
		_PS1SYSTEM._CPU.CycleCount = ( ( *_DebugCycleCount ) >> 2 );
	}
#endif

#endif
	*/


	/*
#ifdef INLINE_DEBUG
	debug << "; VU0";
#endif

	// vu0 is running
	//if ( ( *_DebugCycleCount ) > ( _VU0.VU0.CycleCount ) ) _VU0.VU0.Run ();
	while ( ( *_DebugCycleCount ) > ( _VU0.VU0.CycleCount ) ) _VU0.VU0.Run ();
	
#ifdef INLINE_DEBUG
	debug << "; VU1";
#endif

	//if ( ( *_DebugCycleCount ) > ( _VU1.VU1.CycleCount ) ) _VU1.VU1.Run ();
	while ( ( *_DebugCycleCount ) > ( _VU1.VU1.CycleCount ) ) _VU1.VU1.Run ();
	*/
	
	
	// testing
	//if ( _GPU.GPURegs0.DISPFB2.FBW != 10 )
	//{
	//	debug << "\r\nPS2Cycle#" << dec << _CPU.CycleCount << " PS1Cycle#" << _PS1SYSTEM._CPU.CycleCount;
	//}

}



///////////////////////////////////////////////////////
// Loads a test program into bios
// returns true on success, false otherwise
bool Playstation2::System::LoadTestProgramIntoBios ( const char* FilePath )
{
	static const long BIOS_SIZE_IN_BYTES = 4194304;

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
	InputFile.read ( (char*) ( _BUS.BIOS.b32 ), _BUS.BIOS_Size );

	
#ifndef EE_ONLY_COMPILE

	// copy into PS1 bios area too
	for ( int i = 0; i < ( 0x400000 / 4 ); i++ )
	{
		_PS1SYSTEM._BUS.BIOS.b32 [ i ] = _BUS.BIOS.b32 [ i ];
	}
	
#endif

	
	InputFile.close();
	
#ifdef INLINE_DEBUG
	debug << "->Leaving function: LoadTestProgram";
#endif
	
	return true;
}




