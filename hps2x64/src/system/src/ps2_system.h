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


#ifndef _PS2_SYSTEM_H_
#define _PS2_SYSTEM_H_

#include "PS2_Dma.h"
#include "PS2_Gpu.h"
#include "PS2_Intc.h"
//#include "PS2_MDEC.h"
//#include "PS2_PIO.h"
//#include "PS2_SIO.h"
#include "PS2_Timer.h"
#include "R5900.h"
#include "PS2_SIF.h"
#include "VU.h"
#include "PS2_IPU.h"


#ifndef EE_ONLY_COMPILE

#include "ps1_system.h"

#endif


#ifdef ENABLE_GUI_DEBUGGER
#include "Debug.h"
#endif



//using namespace Playstation2;
//using namespace R5900;


#define USE_SYSTEM_CYCLE


namespace Playstation2
{
	class System
	{

	public:
	
#ifdef ENABLE_GUI_DEBUGGER
		static Debug::Log debug;
#endif
		
		///////////////////////////////////////////////////////////////////////////////////////////////////
		// pointer to itself - I'll probably only be creating 1 instance of this object
		static System *_SYSTEM;
		
		
		static u64 *_DebugCycleCount;
		
		
		// multi-threading stuff for system
		
		// says which devices have ran for the current cycle
		// bit is 1 if device has ran, and 0 if it has not ran for cycle
		static volatile u64 Ran;
		
		enum {
			RAN_R3000A = 0,
			RAN_PS1_DMA0,
			RAN_PS1_DMA1,
			RAN_PS1_DMA2,
			RAN_PS1_DMA3,
			RAN_PS1_DMA4,
			RAN_PS1_DMA5,
			RAN_PS1_DMA6,
			RAN_PS1_TIMER0,
			RAN_PS1_TIMER1,
			RAN_PS1_TIMER2,
			RAN_PS1_TIMER3,
			RAN_PS1_SPU,
			RAN_PS1_CD,
			RAN_PS1_GPU,
			RAN_PS1_INTC,
			RAN_PS1_MDEC,
			RAN_PS1_PIO,
			RAN_PS1_SIO,
			
			RAN_R5900,
			RAN_PS2_DMA0,
			RAN_PS2_DMA1,
			RAN_PS2_DMA2,
			RAN_PS2_DMA3,
			RAN_PS2_DMA4,
			RAN_PS2_DMA5,
			RAN_PS2_DMA6,
			RAN_PS2_DMA7,
			RAN_PS2_DMA8,
			RAN_PS2_DMA9,
			RAN_PS2_TIMER0,
			RAN_PS2_TIMER1,
			RAN_PS2_TIMER2,
			RAN_PS2_TIMER3,
			RAN_PS2_SPU0,
			RAN_PS2_SPU1,
			RAN_PS2_DVDCD,
			RAN_PS2_GPU,
			RAN_PS2_INTC,
			RAN_PS2_MDEC,
			RAN_PS2_PIO,
			RAN_PS2_SIO
		};
		
		// this can't be static
		u32 bEnableVsync;
		
		// need to store the path to the last bios file
		// so that I know where to store NVM file to ?
		char Last_NVM_Path [ 2048 ];
		
		
		// index of next event
		u32 NextEvent_Idx;
		
		// says what cycle the next non-CPU event is scheduled to run at
		// needs to be calculated for the next time whenever non-cpu events get run
		u64 NextEvent_Cycle;
		
		u64 CycleCount;
		
		// the cycle to exit at
		u64 NextExit_Cycle;
		
		// get the cycle for the next event
		void GetNextEventCycle ( void );
		

		//////////////////////////////////////////
		// Debug Status
		union _DebugStatus
		{
			struct
			{
				u32 Stop : 1;
				u32 StepCycle : 1;
				u32 StepInstruction : 1;
				u32 Done : 1;
				u32 OutputCode : 1;
				u32 SaveBIOSToFile : 1;
				u32 SaveRAMToFile : 1;
				u32 SetBreakPoint : 1;
				
				u32 SetCycleBreakPoint : 1;
				u32 SetAddressBreakPoint : 1;
				u32 SetValue : 1;
				
				u32 SetMemoryStart : 1;
				
				u32 SaveState : 1;
				u32 LoadState : 1;
				
				u32 LoadBios : 1;
			};
			

			u32 Value;
		};

		volatile _DebugStatus DebugStatus;
		u32 Debug_BreakPoint_Address;	//0xbfc0022c;
		
		u32 Debug_RAMDisplayStart;
		
		u64 Debug_CycleBreakPoint_Address;
		u32 Debug_AddressBreakPoint_Address;
		u32 Debug_Value;
		
		u32 Debug_Address;
		u32 Debug_NumberOfInstructions;
		
#ifdef ENABLE_GUI_DEBUGGER
		static u32 debug_enabled;
		static WindowClass::Window *DebugWindow;
		static WindowClass::Window *FrameBuffer;
		static WindowClass::Window *SPU_DebugWindow;
		static WindowClass::Window *DMA_DebugWindow;
		static WindowClass::Window *COP2_DebugWindow;
#endif

		///////////////////////////////////////
		// System Components
		
		DataBus _BUS;
		R5900::Cpu _CPU;
		Dma _DMA;
		GPU _GPU;
		Intc _INTC;
		SIF _SIF;
		//MDEC _MDEC;
		//PIO _PIO;
		SIO _SIO;
		Timers _TIMERS;
		VU0 _VU0;
		VU1 _VU1;
		IPU _IPU;

		
#ifndef EE_ONLY_COMPILE

		Playstation1::System _PS1SYSTEM;
		
#endif

		
		// constructor
		System ();
		
		// destructor
		~System ();
		
		void Start ();
		
		// refresh system object (sets non-static pointers, like when initializing object or after loading a save state)
		void Refresh ();
		
		
		void Test ();

		// run the devices on the bus except for cpu for current cycle
		void RunDevices ();
		
		// execute any pending events
		void RunEvents ();

		// run one cycle for entire playstation 2 system
		void Run ();
		
		// run certain number of cycles
		void Run_Cycles ( u64 Cycles );
		
		// run a test program until it terminates with "break" instruction
		int RunTestProgram ();

		// reset the playstation 1 system
		void Reset ();
		
		
		static const int c_iMaxEventFunctions = 256;
		static u32 EventFunc_Count;
		static funcVoid EventFunc [ c_iMaxEventFunctions ];

		// returns the index of the call back function (can't use pointers)
		static u32 Register_CallbackFunction ( funcVoid CallbackFunction )
		{
			EventFunc [ EventFunc_Count++ ] = CallbackFunction;
			return EventFunc_Count - 1;
		}

		
		// opens debug window and prepares for writing
		void EnableDebugging ( void );

		// can only do these after enabling debugging
		void UpdateDebugWindow ( void );
		void UpdateFrameBuffer ( void );
		
		// loads a test program into bios - used for preliminary testing
		bool LoadTestProgramIntoBios ( const char* FilePath );
		
		//////////////////////////////////////////////////////////
		// need to be able to create save/load state files
		void SaveState ( string FilePath = "" );
		void LoadState ( string FilePath = "" );
		
		void LoadBIOS ( string FilePath = "" );
		
		// need to get debug info for debugging
		static char* GetDebugInfo ();
		
	};
}

#endif


