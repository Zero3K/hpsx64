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





#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string.h>

#include <fenv.h>
#pragma STDC FENV_ACCESS ON

//#include <fstream>

#include "R5900.h"
#include "R5900_Execute.h"
#include "MipsOpcode.h"

// will fix this file later
#include "R5900_Print.h"
//#include "ps2_system.h"


// *** I'll put this here temporarily for debugging *** //
#include "PS2_GPU.h"
#include "VU.h"
#include "PS1_SPU2.h"


using namespace R5900;
using namespace R5900::Instruction;
using namespace Playstation2;

using namespace std;

//#include "GNUSignExtend_x64.h"
//using namespace x64SignExtend::Utilities;

// enable recompiler option
#define ENABLE_RECOMPILER_R5900

// enables the level-2 recompiler
//#define ENABLE_R5900_RECOMPILE2



#define ENABLE_BUS_EMULATION


// enables i-cache on R5900
// must be enabled here and in R5900_Execute.cpp
#define ENABLE_R5900_ICACHE

// do not interrupt for CPX 0/1/2/3 instructions
#define DISABLE_INTERRUPT_FOR_CPX_INSTRUCTIONS


// interrupt testing
//#define VERBOSE_MULTIPLE_INT_CAUSE


// this one goes to cout and is good for bug reports
#define INLINE_DEBUG_COUT

#ifdef _DEBUG_VERSION_

// build this with inline debugger
#define INLINE_DEBUG_ENABLE


//#define INLINE_DEBUG
//#define INLINE_DEBUG2
//#define INLINE_DEBUG_ASYNC_INT
//#define INLINE_DEBUG_SYNC_INT
//#define INLINE_DEBUG_UPDATE_INT
//#define INLINE_DEBUG_SYSCALL

//#define INLINE_DEBUG_LOAD
//#define INLINE_DEBUG_STORE

//#define COUT_DELAYSLOT_CANCEL


#endif


Playstation2::DataBus *Cpu::Bus;

namespace R5900
{
	

u64* Cpu::_SpuCycleCount;


u64* Cpu::_NextSystemEvent;


volatile Cpu::_DebugStatus Cpu::DebugStatus;
//volatile u32 Cpu::Debug_BreakPoint_Address;
//u32 Cpu::Debug_BreakPoint_Address;
//u32 Cpu::Debug_RAMDisplayStart;


Cpu *Cpu::Buffer::r;


u32* Cpu::_Debug_IntcStat;
u32* Cpu::_Debug_IntcMask;

u32* Cpu::_Intc_Stat;
u32* Cpu::_Intc_Mask;
u32* Cpu::_R5900_Status;


Cpu *Cpu::_CPU;


R5900::Recompiler* R5900::Cpu::rs;


bool Cpu::DebugWindow_Enabled;

#ifdef ENABLE_GUI_DEBUGGER
Debug::Log Cpu::debug;
Debug::Log Cpu::debugBios;
WindowClass::Window *Cpu::DebugWindow;
DebugValueList<u32> *Cpu::GPR_ValueList;
DebugValueList<u32> *Cpu::COP0_ValueList;
DebugValueList<float> *Cpu::COP2_CPCValueList;
DebugValueList<u32> *Cpu::COP2_CPRValueList;
Debug_DisassemblyViewer *Cpu::DisAsm_Window;
Debug_BreakpointWindow *Cpu::Breakpoint_Window;
Debug_MemoryViewer *Cpu::ScratchPad_Viewer;
Debug_BreakPoints *Cpu::Breakpoints;
#endif


u32 Cpu::TraceValue;







Cpu::Cpu ()
{
	cout << "Running R5900 constructor...\n";

/*
#ifdef INLINE_DEBUG_ENABLE
	// start the inline debugger
	debug.Create ( "R3000ALog.txt" );
	debugBios.Create ( "BiosCallsLog.txt" );
#endif


	// not yet debugging
	debug_enabled = false;

	// Connect CPU with System Bus
	//Bus = db;
	
	// set cpu for buffers
	LoadBuffer.ConnectDevices ( this );
	StoreBuffer.ConnectDevices ( this );
	
	// create execution unit to execute instructions
	//e = new Instruction::Execute ();
	Instruction::Execute::Execute ();
	
	// create an instance of ICache and connect with CPU
	// *note* this is no longer a pointer to the object
	//ICache = new ICache_Device ();
	
	// create an instance of COP2 co-processor and connect with CPU
	// *note* this is no longer a pointer to the object
	//COP2 = new COP2_Device ();
	
	// set as the current R3000A cpu object
	_CPU = this;
	
	// create object to handle breakpoints
	Breakpoints = new Debug_BreakPoints ();
	
	// reset the cpu object
	Reset ();
*/

}


void Cpu::ConnectDevices ( Playstation2::DataBus* db )
{
	Bus = db;
	//_SpuCycleCount = _SpuCC;
}


Cpu::~Cpu ( void )
{
	//if ( debug_enabled ) delete DebugWindow;
}


void Cpu::Start ()
{
	cout << "Running Cpu::Start...\n";
	
#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	// start the inline debugger
	debug.Create ( "R5900Log.txt" );
	
#ifdef INLINE_DEBUG_SPLIT_BIOS
	// put debug output into a separate file
	debugBios.SetSplit ( true );
	debugBios.SetCombine ( false );
#endif

	debugBios.Create ( "PS2SysCallsLog.txt" );
#endif

	// set float rounding mode
	//fesetround(FE_TOWARDZERO);

	
	// not yet debugging
	debug_enabled = false;
	
	// initialize/enable printing of instructions
	R5900::Instruction::Print::Start ();

	// Connect CPU with System Bus
	//Bus = db;
	
	// set cpu for buffers
	//LoadBuffer.ConnectDevices ( this );
	StoreBuffer.ConnectDevices ( this );
	
	// create execution unit to execute instructions
	//e = new Instruction::Execute ();
	//Instruction::Execute::Execute ( this );
	Instruction::Execute::r = this;
	
	// create an instance of ICache and connect with CPU
	// *note* this is no longer a pointer to the object
	//ICache = new ICache_Device ();
	
	// create an instance of COP2 co-processor and connect with CPU
	// *note* this is no longer a pointer to the object
	//COP2 = new COP2_Device ();
	
	// set as the current R3000A cpu object
	_CPU = this;
	
	// create object to handle breakpoints
#ifdef ENABLE_GUI_DEBUGGER
	//Breakpoints = new Debug_BreakPoints ( Bus->BIOS.b8, Bus->MainMemory.b8, DCache.b8 );
	Breakpoints = new Debug_BreakPoints ( Bus->BIOS.b8, Bus->MainMemory.b8, Bus->ScratchPad.b8 );
#endif

	
	// reset the cpu object
	Reset ();
	
	// start COP2 object
	//COP2.Start ();
	
	// start the instruction execution object
	Instruction::Execute::Start ();
	
#ifdef ENABLE_R5900_RECOMPILE2
	rs = new Recompiler ( this, 18 - Playstation2::DataBus::c_iInvalidate_Shift, Playstation2::DataBus::c_iInvalidate_Shift + 11, Playstation2::DataBus::c_iInvalidate_Shift );
	
	rs->SetOptimizationLevel ( 2 );
	//rs->SetOptimizationLevel ( 1 );
	//rs->SetOptimizationLevel ( 0 );
#else
	rs = new Recompiler ( this, 19 - Playstation2::DataBus::c_iInvalidate_Shift, Playstation2::DataBus::c_iInvalidate_Shift + 10, Playstation2::DataBus::c_iInvalidate_Shift );
	//rs = new Recompiler ( this, 19 - Playstation2::DataBus::c_iInvalidate_Shift, Playstation2::DataBus::c_iInvalidate_Shift + 11, Playstation2::DataBus::c_iInvalidate_Shift );
	
	rs->SetOptimizationLevel ( 1 );
#endif
	
	// enable recompiler by default
	bEnableRecompiler = true;
}

void Cpu::Reset ( void )
{
	u32 i;
	
	// zero object
	memset ( this, 0, sizeof( Cpu ) );

	// this is the start address for the program counter when a R5900 is reset
	PC = 0xbfc00000;
	
	// start in kernel mode?
	//CPR0.Status.KUc = 1;

	// must set this as already having been executed
	//CurInstExecuted = true;
	
	// set processor id implementation and revision for R3000A
	CPR0.PRId.Rev = c_ProcessorRevisionNumber;
	CPR0.PRId.Imp = c_ProcessorImplementationNumber;
	
	CPR0.Config.l = c_iConfigReg_StartValue;
	CPR0.Status.l = c_iStatusReg_StartValue;
	
	// fpu revision
	CPC1 [ 0 ] = c_lFCR0;
	
	// initialize the set bits for fpu status register 31
	CPC1 [ 31 ] = c_lFCR31_SetMask;
	
	// need to redo reset of COP2 - must call reset and not Start, cuz if you call start again then inline debugging does not work anymore
	//COP2.Reset ();
	
	// reset i-cache again
	ICache.Reset ();
	
	// reset dcache
	DCache.Reset ();
}


void Cpu::Refresh ( void )
{
	Refresh_AllBranchDelaySlots ();
}



void Cpu::InvalidateCache ( u32 Address )
{
	//ICache.Invalidate ( Address );
}



// ---------------------------------------------------------------------------------


// use for testing execution unit
void Cpu::Run ()
{
	u32 Index;
	u32 *pCacheLine32;
	u64 *pMemPtr64, *pCacheLine64;

	u64 ullChecksum64;
	
	/////////////////////////////////////////////////////////
	// Run DMA Controller first incase it needs to use bus //
	// Run Bus after so that it can clear busy status	   //
	/////////////////////////////////////////////////////////


	/////////////////////////////////
	// CPU components:
	// 1. I-Cache
	// 2. Instruction Execute Unit
	// 3. Load/Store Unit
	// 4. Delay Slot Unit
	// 5. Multiply/Divide Unit
	// 6. I-Cache Invalidate Unit
	// 7. COP2 Unit
	/////////////////////////////////


#ifdef INLINE_DEBUG
	debug << "\r\n->PC = " << hex << setw( 8 ) << PC << dec;
	debug << " Cycle#" << CycleCount;
#endif

	//if ( !( CycleCount & 1 ) )
	{
		
	//////////////////////
	// Load Instruction //
	//////////////////////
	
	/*
	
	// check if the current instruction has already been executed successfully
	//if ( CurInstExecuted )
	//{
		// current instruction has been executed, so we need to get a new one to execute
#ifdef INLINE_DEBUG
		debug << ";Fetch";
#endif

	*/

	
#ifdef ENABLE_R5900_ICACHE
			
		// load next instruction to be executed
		// step 0: check if instruction is in a cached location
		if ( ICache_Device::isCached ( PC ) )
		{
			// instruction is in a cached location //
			
#ifdef INLINE_DEBUG
			debug << ";isCached";
#endif

			// bus might be free //
			
			// check if there is a cache hit
			pCacheLine32 = ICache.isCacheHit ( PC );
			
			// check if there is a cache hit
			if ( !pCacheLine32 )
			{
				// cache miss //
				
#ifdef INLINE_DEBUG
				debug << ";MISS";
#endif

//#ifdef INLINE_DEBUG
//				debug << ";Miss State Enter";
//#endif
				
				////////////////////////////////////////////
				// check if we can access data bus
				


					// *** testing *** maybe i need to flush the store buffer before doing any loads
					//FlushStoreBuffer ();
					
					
					// step 3: there is a certain delay before i-cache is done loading
					//ICacheMiss_BusyCycles = ICacheMissCycles;
					//ICacheMiss_BusyUntil_Cycle = CycleCount + ICacheMissCycles;
					//BusyUntil_Cycle = CycleCount + ICacheMissCycles;
					
					// since it is pipelined, we can read all 4 instructions to refill cache all at once and then wait until setting cache as valid
					// I'll have bus decide how many cycles it is busy for
						
#ifdef INLINE_DEBUG
				debug << ";IREAD16";
#endif

					//u32* CacheLine;
					//CacheLine = ICache.GetCacheLinePtr ( PC & 0x1ffffff0 );
					//CacheLine [ 0 ] = Bus->Read ( ( PC & 0x1ffffff0 ) + 0 );
					//CacheLine [ 1 ] = Bus->Read ( ( PC & 0x1ffffff0 ) + 4 );
					//CacheLine [ 2 ] = Bus->Read ( ( PC & 0x1ffffff0 ) + 8 );
					//CacheLine [ 3 ] = Bus->Read ( ( PC & 0x1ffffff0 ) + 12 );
					pMemPtr64 = (u64*) Bus->GetIMemPtr ( PC & 0xffffffc0 );
					
#ifdef INLINE_DEBUG
				debug << " LATENCY=" << dec << Bus->GetLatency();
				debug << " Inst(Mem): ";
				for ( int i = 0; i < 16; i++ ) { debug << " " << R5900::Instruction::Print::PrintInstruction ( ((u32*)pMemPtr64) [ i ] ).c_str(); }
#endif
					
					ICache.ReloadCache ( PC, pMemPtr64 );

#ifdef INLINE_DEBUG
				debug << " Inst(ICache): ";
				pCacheLine32 = ICache.isCacheHit_Line ( PC );
				for ( int i = 0; i < 16; i++ ) { debug << " " << R5900::Instruction::Print::PrintInstruction ( pCacheLine32 [ i ] ).c_str(); }
#endif
					
					CycleCount += Bus->GetLatency ();
					
					// also add in the time to refill the cache line in addition to the latency
					CycleCount += c_ullCacheRefill_CycleTime;
					
					
#ifdef ENABLE_BUS_EMULATION
#ifdef INLINE_DEBUG
				debug << " BUSY-UNTIL=" << dec << Bus->BusyUntil_Cycle;
#endif
					//if ( Bus->GetLatency () < 20 )
					//{
					// now check if bus is free after memory device latency period
					if ( CycleCount < Bus->BusyUntil_Cycle )
					{
						// bus is not free yet, so must wait until it is free
						CycleCount = Bus->BusyUntil_Cycle;
					}
					
					// now set the time that the bus will be busy for
					// loading 4 quadwords, should take like 4 cycles?
					Bus->BusyUntil_Cycle = CycleCount + 4;
					//}
#endif
					
					
					// validate cache lines
					
//#ifdef INLINE_DEBUG
//					debug << ";ICache Validate;HIT";
//#endif

					//ICache.ValidateCacheLine ( PC );
				
					// reserve the bus for at least 4 cycles
					// no, reserve the bus for the number of cycles used
					//Bus->ReserveBus ( 4 );
					//Bus->ReserveBus_Latency ();
					
					// the cpu is busy until the bus is free
					//BusyUntil_Cycle = Bus->BusyUntil_Cycle;
					
					// skip the idle cycles
					//SkipIdleCycles ();
					
					// wait until the cpu has loaded the data and is ready to load instruction from cache
					//WaitForCpuReady1 ();
					
					// *** testing *** load the instruction from cache //
					//CurInst.Value = ICache.Read ( PC );
					CurInst.Value = ((u32*)pMemPtr64) [ ( PC >> 2 ) & 0xf ];
				//}
				
					if ( bEnableRecompiler )
					{
						
					if ( Bus->GetLatency () <= DataBus::c_iRAM_Read_Latency )
					{
						
					// check for data-modify if using recompiler //
					if ( Bus->InvalidArray.b8 [ ( ( PC & DataBus::MainMemory_Mask ) >> ( 2 + DataBus::c_iInvalidate_Shift ) ) & DataBus::c_iInvalidate_Mask ] )
					{
#ifdef VERBOSE_RECOMPILE
cout << "\nR3000A: CacheMiss: Recompile: PC=" << hex << PC;
#endif

						rs->Recompile ( PC );
						Bus->InvalidArray.b8 [ ( ( PC & DataBus::MainMemory_Mask ) >> ( 2 + DataBus::c_iInvalidate_Shift ) ) & DataBus::c_iInvalidate_Mask ] = 0;

					}
					
					} // if ( Bus->GetLatency () <= c_iRAM_Read_Latency )
					
					} // if ( bEnableRecompiler )
				
			}
			else
			{
				// cache hit //
				
#ifdef INLINE_DEBUG
					debug << ";HIT";
#endif

				//CurInst.Value = ICache.Read ( PC );
				CurInst.Value = *pCacheLine32;
			}
			
			
			/////////////
			// Execute //
			/////////////
			
			// execute instruction
			// process all CPU events

		}
		else
		{
			///////////////////////////////////////////////
			// instruction is not in cached location
			
#ifdef INLINE_DEBUG
			debug << ";!isCached";
#endif
			
			// attempt to load instruction from bus
			
				
			// step 2: if bus is not busy and cpu can access bus, and there are no pending load/stores...
			// then load instruction into current instruction
			/*
			if ( !Bus->isReady () )
			{
				// instruction could not be loaded from bus
				
#ifdef INLINE_DEBUG
				debug << ";!Bus->AccessOK";
#endif

				// cpu is ready when the bus is free
				BusyUntil_Cycle = Bus->BusyUntil_Cycle;

				// bus is busy, so skip idle cycles
				//SkipIdleCycles ();
				
				// wait until the bus is free for use
				WaitForBusReady1 ();
				
				//SkipIdleCpuCycles ();
				//CycleCount++;
				//return;
			}
			
			// handle load/store operations first

			// *** testing *** flush store buffer before loading anything?
			FlushStoreBuffer ();
			*/

#ifdef INLINE_DEBUG
			debug << ";IREAD1";
#endif

			// the bus is free and there are no pending load/store operations
			// Important: When reading from the bus, it has already been determined whether address is in I-Cache or not, so clear top 3 bits
			//CurInst.Value = Bus->Read ( PC & 0x1fffffff );
			CurInst.Value = Bus->Read_t<0xffffffff> ( PC );
			
			CycleCount += Bus->GetLatency ();


			if ( bEnableRecompiler )
			{
				
			if ( Bus->GetLatency () <= DataBus::c_iRAM_Read_Latency )
			{
				
			// check for data-modify if using recompiler //
			if ( Bus->InvalidArray.b8 [ ( ( PC & DataBus::MainMemory_Mask ) >> ( 2 + DataBus::c_iInvalidate_Shift ) ) & DataBus::c_iInvalidate_Mask ] )
			{
#ifdef VERBOSE_RECOMPILE
cout << "\nR3000A: CacheMiss: Recompile: PC=" << hex << PC;
#endif

				rs->Recompile ( PC );
				Bus->InvalidArray.b8 [ ( ( PC & DataBus::MainMemory_Mask ) >> ( 2 + DataBus::c_iInvalidate_Shift ) ) & DataBus::c_iInvalidate_Mask ] = 0;

			}
			
			} // if ( Bus->GetLatency () <= c_iRAM_Read_Latency )
			
			} // if ( bEnableRecompiler )


			
			// reserve bus for at least one cycle to load instruction
			// don't reserve bus yet
			// no, reserve bus for the number of cycles used
			//Bus->ReserveBus ( 1 );
			//Bus->ReserveBus ( c_InstructionLoad_Cycles );
			//Bus->ReserveBus_Latency ();
			
			// wait until instruction is loaded
			//BusyUntil_Cycle = Bus->BusyUntil_Cycle;
			//WaitForCpuReady1 ();
			
			// step 3: we can execute instruction right away since memory access is probably pipelined
			
			/////////////
			// Execute //
			/////////////
			
			// execute instruction
			// process all CPU events

		}
	
	//}
	

#else
	
	// load instruction
	//CurInst.Value = Bus->Read ( PC, 0xffffffff );
	CurInst.Value = Bus->Read_t<0xffffffff> ( PC );
	
#endif

#ifdef INLINE_DEBUG
	debug << " INST=" << dec << R5900::Instruction::Print::PrintInstruction ( CurInst.Value ).c_str();
#endif
	
	/////////////////////////
	// Execute Instruction //
	/////////////////////////
	
	
	// execute instruction
	NextPC = PC + 4;
	
#ifdef INLINE_DEBUG
	debug << ";Execute";
#endif

	///////////////////////////////////////////////////////////////////////////////////////////
	// R0 is always zero - must be cleared before any instruction is executed, not after
	GPR [ 0 ].uq0 = 0;
	GPR [ 0 ].uq1 = 0;
	
#ifdef ENABLE_RECOMPILER_R5900
	if ( !bEnableRecompiler )
	{
#endif

		// *note* could use rotate right, right shift, load...
		Instruction::Execute::ExecuteInstruction ( CurInst );
	
#ifdef ENABLE_RECOMPILER_R5900
	}
	else
	{
		if ( Status.Value
			/*
			//|| ( CurInst.Opcode == 0x1a || CurInst.Opcode == 0x1b || CurInst.Opcode == 0x1e || (CurInst.Opcode & 0x38) == 0x20 || (CurInst.Opcode & 0x38) == 0x30 )
			//|| ( CurInst.Opcode == 0x1f || (CurInst.Opcode & 0x38) == 0x28 || (CurInst.Opcode & 0x38) == 0x38 )
			//|| ( CurInst.Opcode == 0x9 )
			|| ( ( CurInst.Opcode < 0x1 ) && ( CurInst.Funct == 0xc ) )
			|| ( ( CurInst.Opcode < 0x1 ) && ( CurInst.Funct == 0x8 ) )
			|| ( ( CurInst.Opcode < 0x1 ) && ( CurInst.Funct == 0x9 ) )
			//|| ( ( CurInst.Opcode < 0x1 ) && ( CurInst.Funct < 0xa ) )
			*/
		)
		{
#ifdef INLINE_DEBUG
	debug << ";Interpret";
#endif

			Instruction::Execute::ExecuteInstruction ( CurInst );
		}
		else
		{
			
		/*
		//if ( bEnableRecompiler )
		//{
			
		// check for data-modify if using recompiler //
		if ( Bus->InvalidArray.b8 [ ( PC >> ( 2 + DataBus::c_iInvalidate_Shift ) ) & DataBus::c_iInvalidate_Mask ] )
		{

			rs->Recompile ( PC );
			Bus->InvalidArray.b8 [ ( PC >> ( 2 + DataBus::c_iInvalidate_Shift ) ) & DataBus::c_iInvalidate_Mask ] = 0;
		}
		
		//} // if ( bEnableRecompiler )
		*/


		// check that address block is encoded
		if ( ! rs->isRecompiled ( PC ) )
		{
#ifdef INLINE_DEBUG
	debug << ";NOT Recompiled";
#endif
			// address is NOT encoded //
			
			// recompile block
			rs->Recompile ( PC );
			
#ifdef INLINE_DEBUG2
	debug << "\r\nRecompiled: Execute: ADDR: " << hex << PC;
#endif
		}
		
#ifdef INLINE_DEBUG
	debug << "\r\nRecompiled: Execute: ADDR: " << hex << PC;
#endif


		// get the block index
		Index = rs->Get_Index ( PC );
		
		// offset cycles before the run, so that it updates to the correct value
		CycleCount -= rs->CycleCount [ Index ];

		// already checked that is was either in cache, or etc
		// execute from address
		( (func2) (rs->pCodeStart [ Index ]) ) ();
		
#ifdef INLINE_DEBUG
	debug << ";RecompilerReturned";
#endif

		} // end if ( Status.Value )
			
	}
#endif

#ifdef INLINE_DEBUG
	debug << ";ExeDone";
#endif
	
	
	///////////////////////////////////////////////////
	// Check if there is anything else going on
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// Check delay slots
	// need to do this before executing instruction because load delay slot might stall pipeline

	// check if anything is going on with the delay slots
	// *note* by doing this before execution of instruction, pipeline can be stalled as needed
	if ( Status.Value )
	{

		/////////////////////////////////////////////
		// check for anything in delay slots
		
#ifdef INLINE_DEBUG
		debug << ";DelaySlotValid";
#endif
		
		//if ( DelaySlot1.Value )
		if ( Status.DelaySlot_Valid & 1 )
		{
#ifdef INLINE_DEBUG
			debug << ";Delay1.Value";
#endif
			
			//ProcessBranchDelaySlot ();
			DelaySlots [ NextDelaySlotIndex ].cb ();
			DelaySlots [ NextDelaySlotIndex ].Value = 0;
		}

		///////////////////////////////////
		// move delay slot
		//DelaySlot1.Value = DelaySlot0.Value;
		//DelaySlot1.Value2 = DelaySlot0.Value2;
		//DelaySlot0.Value = 0;
		NextDelaySlotIndex ^= 1;

		//cout << hex << "\n" << DelaySlot1.Value << " " << DelaySlot1.Value2 << " " << DelaySlot0.Value << " " << DelaySlot0.Value2;
		
		///////////////////////////////////////////////////
		// Advance status bits for checking delay slot
		//Status.DelaySlot_Valid = ( Status.DelaySlot_Valid << 1 ) & 0x3;
		Status.DelaySlot_Valid >>= 1;
		
		//////////////////////////////////////////////
		// check for Asynchronous Interrupts
		// also make sure interrupts are enabled
		if ( Status.CheckInterrupt )
		{
			// make sure this is not a COP 0/1/2 instruction ??
#ifdef DISABLE_INTERRUPT_FOR_CPX_INSTRUCTIONS
			if ((CurInst.Opcode & 0x3c) != 0x10)
#endif
			{
				// interrupt has now been checked
				Status.CheckInterrupt = 0;

				if ((CPR0.Status.IE && CPR0.Status.EIE) && (!CPR0.Status.EXL) && (!CPR0.Status.ERL) && (CPR0.Status.l & CPR0.Cause.l & 0x8c00))
				{

#ifdef ENABLE_R5900_IDLE
					// not idle if interrupt hits
					ulIdle = 0;
#endif

					//if ( ( ( Bus->Read ( NextPC ) >> 24 ) & 0xfe ) != 0x4a )
					//{
						///////////////////////////////////////////////////
						// Advance status bits for checking delay slot
						//Status.DelaySlot_Valid = ( Status.DelaySlot_Valid << 1 ) & 0x3;

						// ***testing*** preserve interrupt when advancing delay slot?
						//Status.DelaySlot_Valid = ( Status.DelaySlot_Valid & 0xfc ) | ( ( Status.DelaySlot_Valid << 1 ) & 0x3 );

						// do the required stuff
						//ProcessRequiredCPUEvents ();
					CycleCount++;
					LastPC = PC;
					PC = NextPC;

					// interrupt has been triggered
					//Status.ExceptionType = Cpu::EXC_INT;	// *note* this is done in interrupt processing
					ProcessAsynchronousInterrupt();
					//cout << "\nStatus.Value=" << hex << Status.Value << dec << " CycleCount=" << CycleCount;


					return;
					//}
				}

			}	// end if ( CurInst.Opcode & 0xfc != 0x10 )
			
		}
		
		
		//ProcessRequiredCPUEvents ();
	}

	//////////////////////////////
	// Process Other CPU Events //
	//////////////////////////////
	
	/*
	if ( ( ( PC >> 24 ) == 0x18 ) || ( ( NextPC >> 24 ) == 0x18 ) )
	{
		cout << "\n**** FOUND ***** PC=" << hex << PC << " NextPC=" << NextPC;
		while ( 1 );
	}
	*/
	

	/////////////////////////////////////
	// Update Program Counter
	LastPC = PC;
	PC = NextPC;
	
	}
	
	
	//ProcessRequiredCPUEvents ();
	CycleCount++;
	


	//////////////////////////////////////////////
	// check for Asynchronous Interrupts
	/*
	if ( ( CPR0.Regs [ 13 ] & CPR0.Regs [ 12 ] & 0xff00 ) && ( CPR0.Regs [ 12 ] & 1 ) )
	{
		// interrupt has been triggered
		//Status.ExceptionType = Cpu::EXC_INT;	// *note* this is done in interrupt processing
		ProcessAsynchronousInterrupt ();
	}
	*/
}

// skips cycles when cpu is waiting for bus to free to load an instruction or after loading an instruction
void Cpu::WaitForBusReady1 ()
{
	/*
	if ( CycleCount < Bus->BusyUntil_Cycle )
	{
		while ( 1 )
		{
			if ( *_NextSystemEvent > CycleCount && *_NextSystemEvent < Bus->BusyUntil_Cycle )
			{
				CycleCount = *_NextSystemEvent;
				System::_SYSTEM->RunDevices ();
			}
			else
			{
				CycleCount = Bus->BusyUntil_Cycle;
				System::_SYSTEM->RunDevices ();
				return;
			}
		}
	}
	*/
}

// wait for the cpu before instruction execution
void Cpu::WaitForCpuReady1 ()
{
	/*
	if ( CycleCount < BusyUntil_Cycle )
	{
		while ( 1 )
		{
			if ( *_NextSystemEvent > CycleCount && *_NextSystemEvent < BusyUntil_Cycle )
			{
				CycleCount = *_NextSystemEvent;
				System::_SYSTEM->RunDevices ();
			}
			else
			{
				CycleCount = BusyUntil_Cycle;
				System::_SYSTEM->RunDevices ();
				return;
			}
		}
	}
	*/
}


void Cpu::SkipIdleCpuCycles ()
{
	/*
	while ( 1 )
	{
		if ( *_NextSystemEvent > CycleCount && *_NextSystemEvent < BusyUntil_Cycle )
		{
			CycleCount = *_NextSystemEvent;
			System::_SYSTEM->RunDevices ();
			if ( Bus->BusyUntil_Cycle > BusyUntil_Cycle ) BusyUntil_Cycle = Bus->BusyUntil_Cycle;
		}
		else
		{
			CycleCount = BusyUntil_Cycle;
			System::_SYSTEM->RunDevices ();
			if ( Bus->BusyUntil_Cycle > BusyUntil_Cycle )
			{
				BusyUntil_Cycle = Bus->BusyUntil_Cycle;
			}
			else
			{
				return;
			}
			
			//if ( CycleCount == BusyUntil_Cycle ) return;
		}
	}
	*/
}


void Cpu::ProcessRequiredCPUEvents ()
{
	/////////////////////////////////////////////
	// Events that are processed on all cycles //
	/////////////////////////////////////////////

	////////////////////////////////////////////////////////
	// Decrement with saturate busy counters
	// there's no need to do this on every cycle
	//MultiplyDivide_BusyCycles = x64Asm::Utilities::sdec32 ( MultiplyDivide_BusyCycles );
	//COP2.BusyCycles = x64Asm::Utilities::sdec32 ( COP2.BusyCycles );
	
	//////////////////////////////////////////
	// Check if we can access the bus

	/*
	//if ( Bus->BusyCycles ) return;		//!Bus->AccessOK ) return;
	if ( Bus->isReady () )
	{

		// need to process load/store stuff here - this stuff should get checked on all processor cycles
		if ( Status.LoadStore_Valid )
		{
				//ProcessExternalStore ();
				( StoreBuffer.Get_CB () ) ();
				
				StoreBuffer.InvalidateStore ();
				
				/////////////////////////////////////////////////////////////////////////
				// Only want to advance in buffer if we process store successfully
				StoreBuffer.Advance ();
			//}
		}
	}
	
	// update cycle count here??
	//CycleCount++;
	*/

}

// ------------------------------------------------------------------------------------



void Cpu::_cb_Branch ()
{
	DelaySlot *d = & ( _CPU->DelaySlots [ _CPU->NextDelaySlotIndex ] );
	_CPU->NextPC = _CPU->PC + ( d->Instruction.sImmediate << 2 );
}

void Cpu::_cb_Jump ()
{
	DelaySlot *d = & ( _CPU->DelaySlots [ _CPU->NextDelaySlotIndex ] );
	_CPU->NextPC = ( 0xf0000000 & _CPU->PC ) | ( d->Instruction.JumpAddress << 2 );
}

void Cpu::_cb_JumpRegister ()
{
	DelaySlot *d = & ( _CPU->DelaySlots [ _CPU->NextDelaySlotIndex ] );
	_CPU->NextPC = d->Data;
}


void Cpu::ProcessBranchDelaySlot ()
{
	Instruction::Format i;
	u32 Address;
	
	DelaySlot *d = & ( DelaySlots [ NextDelaySlotIndex ] );
	
	//i = DelaySlot1.Instruction;
	i = d->Instruction;
	
	switch ( i.Opcode )
	{
		case OPREGIMM:
		
			switch ( i.Rt )
			{
				case RTBLTZ:	// regimm
				case RTBGEZ:	// regimm
				case RTBLTZL:	// regimm
				case RTBGEZL:	// regimm
					NextPC = PC + ( i.sImmediate << 2 );
					break;

				case RTBLTZAL:	// regimm
				case RTBGEZAL:	// regimm
				case RTBLTZALL:	// regimm
				case RTBGEZALL:	// regimm
					// *** TESTING ***
					//if ( GPR [ 31 ].u != ( PC + 4 ) ) Cpu::DebugStatus.Stop = true;
					
					//GPR [ 31 ].u = PC + 4; // *note* this is already handled properly when instruction is first encountered
					NextPC = PC + ( i.sImmediate << 2 );
					break;
			}
			
			break;
		
		case OPSPECIAL:
		
			switch ( i.Funct )
			{
				case SPJR:
				case SPJALR:
					//NextPC = DelaySlot1.Data;
					NextPC = d->Data;
					break;
			}
			
			break;
			
	
		case OPJ:	// ok
			NextPC = ( 0xf0000000 & PC ) | ( i.JumpAddress << 2 );
			break;
			
			
		case OPJAL:	// ok
			// *** TESTING ***
			//if ( GPR [ 31 ].u != ( PC + 4 ) ) Cpu::DebugStatus.Stop = true;
			
			//GPR [ 31 ].u = PC + 4; // *note* this is already handled properly where it is supposed to be handled
			NextPC = ( 0xf0000000 & PC ) | ( i.JumpAddress << 2 );
			break;
			
		case OPBEQ:
		case OPBNE:
		case OPBLEZ:
		case OPBGTZ:
		case OPBEQL:
		case OPBNEL:
		case OPBLEZL:
		case OPBGTZL:
		case OPCOP0:
		case OPCOP1:
		case OPCOP2:
			NextPC = PC + ( i.sImmediate << 2 );
			break;
	}
	
	
#ifdef INLINE_DEBUG_COUT
	/*
	if ( NextPC == 0x000000a0 ) 
	{
		if ( GPR [ 9 ].u < ( sizeof(bioscallsA0) / sizeof(bioscallsA0[0]) ) )
		{
			Debug_CallStack_Address [ CallStackDepth ] = NextPC;
			Debug_CallStack_Function [ CallStackDepth ] = GPR [ 9 ].u;
			Debug_CallStack_ReturnAddress [ CallStackDepth ] = GPR [ 31 ].u;
			CallStackDepth++;
			CallStackDepth &= ( CallStack_Size - 1 );
			
	#ifdef INLINE_DEBUG_BIOS
			debugBios << "\r\nDepth: " << dec << CallStackDepth << ";BiosCallA0; " << hex << setw(8) << PC << " " << dec << CycleCount << " r31=" << hex << GPR [ 31 ].u << " " << bioscallsA0 [ GPR [ 9 ].u ].prototype;
	#endif
			
			switch ( GPR [ 9 ].u )
			{
				case 0x17:
					//int strcmp(const char *dst, const char *src)
	#ifdef INLINE_DEBUG_BIOS
					debugBios << "\r\ndst=" << hex << setw(8) << GPR [ 4 ].u << "; src=" << GPR [ 5 ].u;
	#endif
					break;
				
				case 0x25:
					//int toupper(int c)
	#ifdef INLINE_DEBUG_BIOS
					debugBios << "\r\nc=" << (char) GPR [ 4 ].u;
	#endif
					break;
					
				case 0x2a:
					//void *memcpy(void *dst, const void *src, size_t n)
	#ifdef INLINE_DEBUG_BIOS
					debugBios << "\r\ndst=" << hex << setw(8) << GPR [ 4 ].u << "; src=" << setw(8) << GPR [ 5 ].u << "; n=" << GPR [ 6 ].u;
	#endif
					break;
					
				case 0x3f:
					//int printf(const char *fmt, ...)
					
	#ifdef INLINE_DEBUG_BIOS
					debugBios << "\r\nfmt=" << hex << setw(8) << GPR [ 4 ].u << " string is: " << ( (const char*) (Bus->GetPointer( GPR [ 4 ].u )) );
					debugBios << " a1=" << GPR [ 5 ].u << " a2=" << GPR [ 6 ].u << " a3=" << GPR [ 7 ].u;
	#endif
					
					break;
			
				default:
	#ifdef INLINE_DEBUG_BIOS
					debugBios << "\r\nt1=" << hex << GPR [ 9 ].u << "; a0=" << hex << GPR [ 4 ].u << "; a1=" << GPR [ 5 ].u << "; a2=" << GPR [ 6 ].u << "; a3=" << GPR [ 7 ].u;
	#endif
					break;
			}
		}
		else
		{
	#ifdef INLINE_DEBUG_BIOS
			debugBios << "\r\nBiosCallA0; Unknown; t1 = " << hex << GPR [ 9 ].u;
	#endif
		}
	}
	else if ( NextPC == 0x000000b0 )
	{
		if ( GPR [ 9 ].u < ( sizeof(bioscallsB0) / sizeof(bioscallsB0[0]) ) )
		{
			Debug_CallStack_Address [ CallStackDepth ] = NextPC;
			Debug_CallStack_Function [ CallStackDepth ] = GPR [ 9 ].u;
			Debug_CallStack_ReturnAddress [ CallStackDepth ] = GPR [ 31 ].u;
			CallStackDepth++;
			CallStackDepth &= ( CallStack_Size - 1 );
			
	#ifdef INLINE_DEBUG_BIOS
			debugBios << "\r\nDepth: " << dec << CallStackDepth << ";BiosCallB0; " << hex << setw(8) << PC << " " << dec << CycleCount << " r31=" << hex << GPR [ 31 ].u << " " << bioscallsB0 [ GPR [ 9 ].u ].prototype;
	#endif
			
			switch ( GPR [ 9 ].u )
			{
				case 0x3d:
					//putchar(char ch)
	#ifdef INLINE_DEBUG_BIOS
					debugBios << "\r\nch=" << (char) GPR [ 4 ].u;
	#endif
					// if newline, then output cycle
					if ( GPR [ 4 ].u == '\n' )
					{
						// *** output cycle ***
						// disabling display of cycle count
						//out << " CycleCount=" << dec << CycleCount;
					}
					
					putchar ( GPR [ 4 ].u );
					
					break;
					
				case 0x3f:
					//puts(const char *s)
	#ifdef INLINE_DEBUG_BIOS
					debugBios << "\r\ns=" << hex << setw(8) << GPR [ 4 ].u << " string is: " << ( (const char*) (Bus->GetPointer( GPR [ 4 ].u )) );
	#endif
					
					puts ( (const char*) (Bus->GetPointer( GPR [ 4 ].u )) );
					
					// *** output cycle ***
					// disabling display of cycle count
					//cout << " CycleCount=" << dec << CycleCount << "\n";
					
					break;
							
				default:
	#ifdef INLINE_DEBUG_BIOS
					debugBios << "\r\nt1=" << hex << GPR [ 9 ].u << "; a0=" << hex << GPR [ 4 ].u << "; a1=" << GPR [ 5 ].u << "; a2=" << GPR [ 6 ].u << "; a3=" << GPR [ 7 ].u;
	#endif
					break;
			}
		}
		else
		{
	#ifdef INLINE_DEBUG_BIOS
			debugBios << "\r\nBiosCallB0; Unknown; t1 = " << hex << GPR [ 9 ].u;
	#endif
		}
	}
	else if ( NextPC == 0x000000c0 )
	{
		if ( GPR [ 9 ].u < ( sizeof(bioscallsC0) / sizeof(bioscallsC0[0]) ) )
		{
			Debug_CallStack_Address [ CallStackDepth ] = NextPC;
			Debug_CallStack_Function [ CallStackDepth ] = GPR [ 9 ].u;
			Debug_CallStack_ReturnAddress [ CallStackDepth ] = GPR [ 31 ].u;
			CallStackDepth++;
			CallStackDepth &= ( CallStack_Size - 1 );
			
	#ifdef INLINE_DEBUG_BIOS
			debugBios << "\r\nDepth: " << dec << CallStackDepth << ";BiosCallC0; " << hex << setw(8) << PC << " " << dec << CycleCount << " r31=" << hex << GPR [ 31 ].u << " " << bioscallsC0 [ GPR [ 9 ].u ].prototype;
			debugBios << "\r\nt1=" << hex << GPR [ 9 ].u << "; a0=" << hex << GPR [ 4 ].u << "; a1=" << GPR [ 5 ].u << "; a2=" << GPR [ 6 ].u << "; a3=" << GPR [ 7 ].u;
	#endif
		}
		else
		{
	#ifdef INLINE_DEBUG_BIOS
			debugBios << "\r\nBiosCallC0; Unknown; t1 = " << hex << GPR [ 9 ].u;
	#endif
		}
	}
	
	// also check if returning from a function on call stack
	if ( CallStackDepth > 0 )
	{
		if ( NextPC == Debug_CallStack_ReturnAddress [ CallStackDepth - 1 ] )
		{
			CallStackDepth--;
			// show that we are returning
			switch ( Debug_CallStack_Address [ CallStackDepth ] )
			{
				case 0xa0:
	#ifdef INLINE_DEBUG_BIOS
					debugBios << "\r\nreturn: " << bioscallsA0 [ Debug_CallStack_Function [ CallStackDepth ] ].prototype;
					debugBios << ";v0=" << hex << GPR [ 2 ].u << " " << hex << setw(8) << NextPC << " " << dec << CycleCount;
	#endif
					break;
				
				case 0xb0:
	#ifdef INLINE_DEBUG_BIOS
					debugBios << "\r\nreturn: " << bioscallsB0 [ Debug_CallStack_Function [ CallStackDepth ] ].prototype;
					debugBios << ";v0=" << hex << GPR [ 2 ].u << " " << hex << setw(8) << NextPC << " " << dec << CycleCount;
	#endif
					break;

				case 0xc0:
	#ifdef INLINE_DEBUG_BIOS
					debugBios << "\r\nreturn: " << bioscallsC0 [ Debug_CallStack_Function [ CallStackDepth ] ].prototype;
					debugBios << ";v0=" << hex << GPR [ 2 ].u << " " << hex << setw(8) << NextPC << " " << dec << CycleCount;
	#endif
					break;
			}
			
		}
	}
	*/
#endif
}




u32 Cpu::Read_MFC0 ( u32 Register )
{
	/////////////////////////////////////////////////////////////////////
	// Process storing to COP0 registers
	switch ( Register )
	{
		case 9:
#ifdef INLINE_DEBUG_STORE
			debug << ";Count" << ";Before=" << _CPU->CPR0.Regs [ 9 ] << ";Writing=" << hex << Value;
#endif

			_CPU->CPR0.Regs [ 9 ] = (u32) _CPU->CycleCount;
			return (u32) _CPU->CycleCount;
			break;
			
		default:
#ifdef INLINE_DEBUG_STORE
			debug << ";Other";
#endif
		
			return _CPU->CPR0.Regs [ Register ];
			break;
	}
}



void Cpu::Write_MTC0 ( u32 Register, u32 Value )
{
	/////////////////////////////////////////////////////////////////////
	// Process storing to COP0 registers
	switch ( Register )
	{
		case 1:
#ifdef INLINE_DEBUG_LOAD
			debug << ";Random";
#endif

			// Random register is read-only
			break;
			
		case 8:
#ifdef INLINE_DEBUG_LOAD
			debug << ";BadVAddr";
#endif

			// BadVAddr register is read-only
			break;
		
		case 12:
#ifdef INLINE_DEBUG_LOAD
			debug << ";Status" << ";Before=" << _CPU->CPR0.Regs [ 12 ] << ";Writing=" << hex << Value;
#endif
		
			///////////////////////////////////////////////////
			// Status register
			// bits 5-9,13-14,19-21,24-27 are always zero
			
			// *note* Asynchronous interrupt will be checked for in main processor loop
			_CPU->CPR0.Regs [ 12 ] = ( _CPU->CPR0.Regs [ 12 ] & StatusReg_WriteMask ) | ( Value & (~StatusReg_WriteMask) );
			
			
			//if ( CPR0.Status.SwC )
			//{
			//	cout << "\nhps1x64 ALERT: SwC -> it is invalidting I-Cache entries\n";
			//}
			
			
			// whenever interrupt related stuff is messed with, must update the other interrupt stuff
			_CPU->UpdateInterrupt ();
			
#ifdef INLINE_DEBUG_LOAD
			debug << ";After=" << hex << _CPU->CPR0.Regs [ 12 ];
#endif

			break;
			
		
								
		case 13:
#ifdef INLINE_DEBUG_LOAD
			debug << ";Cause";
#endif
		
			////////////////////////////////////////////////////
			// Cause register
			// ** note ** this register is READ-ONLY except for 2 software writable interrupt bits
			
			// software can't modify bits 0-1,7,10-31
			// software can only modify bits 8 and 9
			// *note* Asynchronous interrupt will be checked for in main processor loop
			//CPR0.Regs [ 13 ] = ( CPR0.Regs [ 13 ] & CauseReg_WriteMask ) | ( Value & (~CauseReg_WriteMask) );
			
			// whenever interrupt related stuff is messed with, must update the other interrupt stuff
			//UpdateInterrupt ();
			
			break;
			
			
		case 15:
#ifdef INLINE_DEBUG_LOAD
			debug << ";PRId";
#endif
							
			///////////////////////////////////////////////////
			// PRId register
			// this is register is READ-ONLY
			break;
		
		
		case 16:
			// config register - bits 3-11 and 14-15 and 19-31 are read-only
			break;
			
		default:
#ifdef INLINE_DEBUG_LOAD
			debug << ";Other";
#endif
		
			_CPU->CPR0.Regs [ Register ] = Value;
			break;
	}
}



u32 Cpu::Read_CFC1 ( u32 Register )
{
	//return CPC1 [ Register ];
	
	// for the first 16 registers return register #0
	if ( Register < 16 ) return c_lFCR0;
	
	// for the last 16 registers return register #31
	return CPC1 [ 31 ];
}


void Cpu::Write_CTC1 ( u32 Register, u32 Value )
{
	switch ( Register )
	{
		// read only
		case 0:
			break;
			
		case 31:
			// certain bits must be set or cleared
			//CPC1 [ Register ] = ( Value | c_lFCR31_SetMask ) & ~c_lFCR31_ClearMask;
			CPC1 [ 31 ] = ( CPC1 [ 31 ] & ~c_lFCR31_Mask ) | ( Value & c_lFCR31_Mask );
			break;
			
		default:
			CPC1 [ Register ] = Value;
			
			// don't know if it should be doing this
			cout << "\nhps2x64: ALERT: R5900: FPU: Writing to an unimplement register#" << dec << Register << " Value=" << hex << Value;
			break;
	}
}





/*
static void _cb_LB ()
{
	
	if ( isDCache ( _CPU->DelaySlot1.Data ) )
	{
		if ( _CPU->DelaySlot1.Instruction.Rt == _CPU->LastModifiedRegister ) return;
		_CPU->GPR [ _CPU->DelaySlot1.Instruction.Rt ].s = (s32) ((s8) (_CPU->DCache.b8 [ _CPU->DelaySlot1.Data & c_ScratchPadRam_Mask ]));
	}
	else
	{
		_CPU->FlushStoreBuffer ();
		
		// perform the load
		//ProcessExternalLoad ();
		Bus->ReserveBus ( c_CycleTime_Load );
		if ( _CPU->DelaySlot1.Instruction.Rt == _CPU->LastModifiedRegister ) return;
		
		_CPU->GPR [ _CPU->DelaySlot1.Instruction.Rt ].s = ( (s32) ((s32)(Bus->Read ( Address ) << 24)) >> 24 );
		
		// must wait for the loaded data to reach the cpu
		// cpu is only busy for as long as it takes to load the data
		_CPU->BusyUntil_Cycle = Bus->BusyUntil_Cycle;
		_CPU->WaitForCpuReady1 ();
	}
}

static void _cb_LBU ()
{
	if ( _CPU->DelaySlot1.Instruction.Rt == _CPU->LastModifiedRegister ) return;
	_CPU->GPR [ _CPU->DelaySlot1.Instruction.Rt ].u = (u32) _CPU->DCache.b8 [ _CPU->DelaySlot1.Data & c_ScratchPadRam_Mask ];
}

static void _cb_LH ()
{
	if ( _CPU->DelaySlot1.Instruction.Rt == _CPU->LastModifiedRegister ) return;
	_CPU->GPR [ _CPU->DelaySlot1.Instruction.Rt ].s = (s32) ((s16) (_CPU->DCache.b16 [ ( _CPU->DelaySlot1.Data & c_ScratchPadRam_Mask ) >> 1 ]));
}

static void _cb_LHU ()
{
	if ( _CPU->DelaySlot1.Instruction.Rt == _CPU->LastModifiedRegister ) return;
	_CPU->GPR [ _CPU->DelaySlot1.Instruction.Rt ].u = (u32) _CPU->DCache.b16 [ ( _CPU->DelaySlot1.Data & c_ScratchPadRam_Mask ) >> 1 ];
}

static void _cb_LW ()
{
	if ( _CPU->DelaySlot1.Instruction.Rt == _CPU->LastModifiedRegister ) return;
	_CPU->GPR [ _CPU->DelaySlot1.Instruction.Rt ].u = _CPU->DCache.b32 [ ( _CPU->DelaySlot1.Data & c_ScratchPadRam_Mask ) >> 2 ];
}

static void _cb_LWL ()
{
	if ( _CPU->DelaySlot1.Instruction.Rt == _CPU->LastModifiedRegister ) return;
}

static void _cb_LWR ()
{
	if ( _CPU->DelaySlot1.Instruction.Rt == _CPU->LastModifiedRegister ) return;
}

static void _cb_LWC2 ()
{
	_CPU->COP2.Write_MTC ( _CPU->DelaySlot1.Instruction.Rt, _CPU->DCache.b32 [ ( _CPU->DelaySlot1.Data & c_ScratchPadRam_Mask ) >> 2 ] );
}
*/


// returns false if load buffer is full
void Cpu::ProcessLoadDelaySlot ()
{
#ifdef INLINE_DEBUG_LOAD
		debug << "\r\nCPU::ProcessLoadDelaySlot " << hex << PC << " " << dec << CycleCount;
#endif

	/*
	Instruction::Format i;
	u32 LoadAddress;
	u32 Temp;

	static const u32 c_Mask = 0xffffffff;
	u32 Type, Offset;

	// get the instruction
	i = DelaySlot1.Instruction;
	
	
	// check if this is a load OP
	if ( i.Opcode >= 0x20 )
	{
		/////////////////////////
		// load instruction
		
		
#ifdef INLINE_DEBUG_LOAD
		debug << ";Load";
#endif
		
		
		// check if address is for data cache
		LoadAddress = DelaySlot1.Data;		//LoadDelaySlots.Get_AddressValue();
		
		// !!! Important !!!
		// there is no data cache, just scratch pad ram on PS1, so clear top 3 bits of address
		LoadAddress &= 0x1fffffff;
		
#ifdef INLINE_DEBUG_LOAD
		debug << ";LoadAddress=" << hex << LoadAddress;
#endif

		if ( isDCache ( LoadAddress ) )
		{
#ifdef INLINE_DEBUG_LOAD
			debug << ";D$";
#endif

			// load data from d-cache
			switch ( i.Opcode )
			{
				case OPLB:
				
					// if register was modified in delay slot, then cancel load
					if ( DelaySlot1.Instruction.Rt == LastModifiedRegister )
					{
#ifdef COUT_DELAYSLOT_CANCEL
						cout << "\nhps1x64 ALERT: LB Reg#" << dec << i.Rt << " was modified in load (from D$) delay slot @ Cycle#" << CycleCount << hex << " PC=" << PC;
#endif

						break;
					}
		
					GPR [ i.Rt ].s = (s32) ((s8) (DCache.b8 [ LoadAddress & ( c_ScratchPadRam_Size - 1 ) ]));
					break;
					
				case OPLH:
					if ( DelaySlot1.Instruction.Rt == LastModifiedRegister )
					{
#ifdef COUT_DELAYSLOT_CANCEL
						cout << "\nhps1x64 ALERT: LH Reg#" << dec << i.Rt << " was modified in load (from D$) delay slot @ Cycle#" << CycleCount << hex << " PC=" << PC;
#endif

						break;
					}
		
					GPR [ i.Rt ].s = (s32) ((s16) (DCache.b16 [ ( LoadAddress & ( c_ScratchPadRam_Size - 1 ) ) >> 1 ]));
					break;
					
				case OPLW:
					if ( DelaySlot1.Instruction.Rt == LastModifiedRegister )
					{
#ifdef COUT_DELAYSLOT_CANCEL
						cout << "\nhps1x64 ALERT: LW Reg#" << dec << i.Rt << " was modified in load (from D$) delay slot @ Cycle#" << CycleCount << hex << " PC=" << PC;
#endif

						break;
					}
		
					GPR [ i.Rt ].u = DCache.b32 [ ( LoadAddress & ( c_ScratchPadRam_Size - 1 ) ) >> 2 ];
					break;
					
				case OPLBU:
					if ( DelaySlot1.Instruction.Rt == LastModifiedRegister )
					{
#ifdef COUT_DELAYSLOT_CANCEL
						cout << "\nhps1x64 ALERT: LBU Reg#" << dec << i.Rt << " was modified in load (from D$) delay slot @ Cycle#" << CycleCount << hex << " PC=" << PC;
#endif

						break;
					}
		
					GPR [ i.Rt ].u = (u32) DCache.b8 [ LoadAddress & ( c_ScratchPadRam_Size - 1 ) ];
					break;
					
				case OPLHU:
					if ( DelaySlot1.Instruction.Rt == LastModifiedRegister )
					{
#ifdef COUT_DELAYSLOT_CANCEL
						cout << "\nhps1x64 ALERT: LHU Reg#" << dec << i.Rt << " was modified in load (from D$) delay slot @ Cycle#" << CycleCount << hex << " PC=" << PC;
#endif

						break;
					}
		
					GPR [ i.Rt ].u = (u32) DCache.b16 [ ( LoadAddress & ( c_ScratchPadRam_Size - 1 ) ) >> 1 ];
					break;
					
				// no LWL/LWR since this is done during first execution of instruction
				// actually, a PS1 seems to have an LWL/LWR delay slot, even though it is not supposed to
				case OPLWL:
					// don't break if LWL or LWR
					// no, actually this should happen
					if ( DelaySlot1.Instruction.Rt == LastModifiedRegister )
					{
#ifdef COUT_DELAYSLOT_CANCEL
						cout << "\nhps1x64 ALERT: LWL Reg#" << dec << i.Rt << " was modified in load (from D$) delay slot @ Cycle#" << CycleCount << hex << " PC=" << PC;
#endif

						break;
					}
					
					Type = 3 - ( LoadAddress & 3 );
					Offset = ( LoadAddress & ( c_ScratchPadRam_Size - 1 ) ) >> 2;
					GPR [ i.Rt ].u = ( DCache.b32 [ Offset ] << ( Type << 3 ) ) | ( GPR [ i.Rt ].u & ~( c_Mask << ( Type << 3 ) ) );
				
					break;
					
				case OPLWR:
					if ( DelaySlot1.Instruction.Rt == LastModifiedRegister )
					{
#ifdef COUT_DELAYSLOT_CANCEL
						cout << "\nhps1x64 ALERT: LWR Reg#" << dec << i.Rt << " was modified in load (from D$) delay slot @ Cycle#" << CycleCount << hex << " PC=" << PC;
#endif

						break;
					}
					
					Type = LoadAddress & 3;
					Offset = ( LoadAddress & ( c_ScratchPadRam_Size - 1 ) ) >> 2;
					GPR [ i.Rt ].u = ( DCache.b32 [ Offset ] >> ( Type << 3 ) ) | ( GPR [ i.Rt ].u & ~( c_Mask >> ( Type << 3 ) ) );
				
					break;
				
				case OPLWC2:
					//COP2.CPR2.Regs [ i.Rt ] = DCache.b32 [ ( LoadAddress & ( c_ScratchPadRam_Size - 1 ) ) >> 2 ];
					COP2.Write_MTC ( i.Rt, DCache.b32 [ ( LoadAddress & ( c_ScratchPadRam_Size - 1 ) ) >> 2 ] );
					
					// mark register as ready for use in COP2
					//COP2.CPRLoading_Bitmap &= ~( 1 << i.Rt );
					
					break;
			}
		}
		else
		{
		
#ifdef INLINE_DEBUG_LOAD
			debug << ";BUS";
#endif
			
			//WaitForBusReady1 ();
			
			// flush the store buffer
			FlushStoreBuffer ();
			
			// perform the load
			ProcessExternalLoad ();
			
			// must wait for the loaded data to reach the cpu
			// cpu is only busy for as long as it takes to load the data
			BusyUntil_Cycle = Bus->BusyUntil_Cycle;
			WaitForCpuReady1 ();
		}
		
#ifdef INLINE_DEBUG_LOAD
			debug << ";CLEARING_DELAY_SLOT1";
#endif

		// must do this in case I need to call this while executing an instruction (JAL,JALR,ETC)
		DelaySlot1.Value = 0;
		Status.DelaySlot_Valid &= ~2;
	}
	else if ( i.Opcode >= 0x10 )
	{
		//////////////////////////////////////////////////////
		// COP instruction
	
#ifdef INLINE_DEBUG_LOAD
		debug << ";COP";
#endif

		DelaySlot1.cb ();
		
		// CFC/CTC/MFC/MTC
		switch ( i.Rs )
		{
			case B1CFC2:
			case B1MFC0:
				// if register was modified in delay slot, then cancel load
				//if ( DelaySlot1.Instruction.Rt == LastModifiedRegister ) break;
				
				GPR [ i.Rt ].u = DelaySlot1.Data;
				break;

			case B1CTC2:
				
				COP2.Write_CTC ( i.Rd, DelaySlot1.Data );
				break;
				
			case B1MTC0:
			
				switch ( i.Opcode & 0x3 )
				{
					case 0:
#ifdef INLINE_DEBUG_LOAD
						debug << ";MTC0";
#endif
					
						Write_MTC0 ( i.Rd, DelaySlot1.Data );
						
						break;
						
					case 2:
					
						// MTC2

						/////////////////////////////////////////////////////////////////////
						// Process storing to COP2 registers
						COP2.Write_MTC ( i.Rd, DelaySlot1.Data );
						break;

				}
				
				break;
		}
		
		// must do this in case I need to call this while executing an instruction (JAL,JALR,ETC)
		DelaySlot1.Value = 0;
		Status.DelaySlot_Valid &= ~2;
	}
	*/
}



void Cpu::_cb_FC ()
{
	if ( _CPU->DelaySlot1.Instruction.Rt != _CPU->LastModifiedRegister )
	{
		_CPU->GPR [ _CPU->DelaySlot1.Instruction.Rt ].u = _CPU->DelaySlot1.Data;
	}
}

void Cpu::_cb_MTC0 ()
{
	_CPU->Write_MTC0 ( _CPU->DelaySlot1.Instruction.Rd, _CPU->DelaySlot1.Data );
}

void Cpu::_cb_MTC2 ()
{
	//_CPU->COP2.Write_MTC ( _CPU->DelaySlot1.Instruction.Rd, _CPU->DelaySlot1.Data );
}

void Cpu::_cb_CTC2 ()
{
	//_CPU->COP2.Write_CTC ( _CPU->DelaySlot1.Instruction.Rd, _CPU->DelaySlot1.Data );
}






bool Cpu::ProcessExternalLoad ()
{
#ifdef INLINE_DEBUG_LOAD
	debug << "\r\nCPU::ProcessExternalLoad; " << hex << PC << " " << dec << CycleCount;
#endif

	static const u32 c_CycleTime = 5;

	Instruction::Format i;
	u32 Address;
	u32 Value;
	u32 Temp;
	u32 Index;
	
	/////////////////////////////////////////
	// process load
	
	
	i = DelaySlot1.Instruction;
	Address = DelaySlot1.Data;
	
	// clear top 3 bits of address
	Address &= 0x1fffffff;
	
#ifdef INLINE_DEBUG_LOAD
	debug << "; Inst=" << hex << i.Value << "; Address=" << Address << "; Index=" << Index << "; NextIndex=" << NextIndex;
#endif

	// catch #1 - if register was modified in delay slot, then it gets overwritten later in the pipeline
	if ( i.Opcode < 50 )
	{
		// if register was modified in load delay slot, then cancel load
		if ( i.Rt == LastModifiedRegister )
		{
#ifdef COUT_DELAYSLOT_CANCEL
			cout << "\nhps1x64 ALERT: Reg#" << dec << i.Rt << " was modified in load (from bus) delay slot @ Cycle#" << CycleCount << hex << " PC=" << PC;
#endif
			
			//Bus->ReserveBus ( c_CycleTime );
			return 0;
		}
	}

	switch ( i.Opcode )
	{
		case OPLB:
#ifdef INLINE_DEBUG_LOAD
			debug << ";LB";
#endif
			//Value = Bus->Read ( Address );
			
			//GPR [ i.Rt ].s = ( (s32) ((s32)(Value << 24)) >> 24 );

			
			// bus is busy with something now
			//Bus->ReserveBus_Latency ();
			break;
		
		case OPLH:
#ifdef INLINE_DEBUG_LOAD
			debug << ";LH";
#endif
			//Value = Bus->Read ( Address );
			
			//GPR [ i.Rt ].s = ( (s32) ((s32)(Value << 16)) >> 16 );

			
			// bus is busy with something now
			//Bus->ReserveBus_Latency ();
			break;
		
		case OPLW:
#ifdef INLINE_DEBUG_LOAD
			debug << ";LW";
#endif
			//Value = Bus->Read ( Address );
			
			//GPR [ i.Rt ].u = Value;

			
			// bus is busy with something now
			//Bus->ReserveBus_Latency ();
			break;
		
		case OPLBU:
#ifdef INLINE_DEBUG_LOAD
			debug << ";LBU";
#endif
			//Value = Bus->Read ( Address );
			
			//GPR [ i.Rt ].u = ( Value & 0xff );

			
			// bus is busy with something now
			//Bus->ReserveBus_Latency ();
			break;
		
		case OPLHU:
#ifdef INLINE_DEBUG_LOAD
			debug << ";LHU";
#endif
			//Value = Bus->Read ( Address );
			
			//GPR [ i.Rt ].u = ( Value & 0xffff );

			
			// bus is busy with something now
			//Bus->ReserveBus_Latency ();
			break;
		
		case OPLWL:
#ifdef INLINE_DEBUG_LOAD
			debug << ";LWL";
#endif
			//Value = Bus->Read ( Address & ~3 );
			
			//Value <<= ( ( 3 - ( Address & 3 ) ) << 3 );
			//Temp = GPR [ i.Rt ].u;
			//Temp <<= ( ( ( Address & 3 ) + 1 ) << 3 );
			//if ( ( Address & 3 ) == 3 ) Temp = 0;
			//Temp >>= ( ( ( Address & 3 ) + 1 ) << 3 );
			//GPR [ i.Rt ].u = Value | Temp;

			
			// bus is busy with something now
			//Bus->ReserveBus_Latency ();
			break;
			
		case OPLWR:
#ifdef INLINE_DEBUG_LOAD
			debug << ";LWR";
#endif
			//Value = Bus->Read ( Address & ~3 );
			
			//Value >>= ( ( Address & 3 ) << 3 );
			//Temp = GPR [ i.Rt ].u;
			//Temp >>= ( ( 4 - ( Address & 3 ) ) << 3 );
			//if ( ( Address & 3 ) == 0 ) Temp = 0;
			//Temp <<= ( ( 4 - ( Address & 3 ) ) << 3 );
			//GPR [ i.Rt ].u = Value | Temp;
			
			
			// bus is busy with something now
			//Bus->ReserveBus_Latency ();
			break;
			
			
		case OPLWC2:
#ifdef INLINE_DEBUG_LOAD
			debug << ";LWC2";
#endif
			//Value = Bus->Read ( Address );
			
			//COP2.CPR2.Regs [ i.Rt ] = Value;
			//COP2.Write_MTC ( i.Rt, Value );

			// bus is busy with something now
			//Bus->ReserveBus_Latency ();
			break;
		
	}
	
#ifdef INLINE_DEBUG_LOAD
	debug << "; ValueLoaded=" << Value;
#endif

	return 0;
}


void Cpu::FlushStoreBuffer ()
{
	/*
	// need to wait until the bus is free before any load/stores can be performed
	if ( !Bus->isReady () )
	{
		WaitForBusReady1 ();
	}
	
	// bus is not in use, so flush the store buffer
	while ( Status.LoadStore_Valid )
	{
		//while ( StoreBuffer.isNextStore () )
		//{
			//ProcessExternalStore ();
			( StoreBuffer.Get_CB () ) ();
			
			
			StoreBuffer.InvalidateStore ();
			
			/////////////////////////////////////////////////////////////////////////
			// Only want to advance in buffer if we process store successfully
			StoreBuffer.Advance ();
		//}
	}
	*/
	
	// only need to wait for the bus again if a load is performed
}


void Cpu::ProcessExternalStore ()
{
#ifdef INLINE_DEBUG_STORE
	debug << "\r\nCPU::ProcessExternalStore " << hex << PC << " " << dec << CycleCount;
#endif

	static const u32 c_CycleTime = 1;
	
	( StoreBuffer.Get_CB () ) ();
	
	
	return;

/*
	Instruction::Format i;
	u32 Address;
	u32 Value;
	u32 Temp;
	u32 Index;
	
	
	/////////////////////////////////////////
	// process store
	
	
	i = StoreBuffer.Get_Inst ();
	Address = StoreBuffer.Get_Address ();	// & 0x1fffffff;
	Value = StoreBuffer.Get_Value ();
	
	
#ifdef INLINE_DEBUG_STORE
	debug << "; Inst=" << hex << i.Value << "; Address=" << Address << "; Value=" << Value << "; Index=" << Index << "; NextIndex=" << NextIndex;
#endif
	
	
	switch ( i.Opcode )
	{
		case OPSB:
#ifdef INLINE_DEBUG_STORE
			debug << ";SB";
#endif

			Bus->Write ( Value, Address, 0xff );
			
			// bus is busy with something now
			Bus->ReserveBus ( c_CycleTime );
			break;
		
		case OPSH:
#ifdef INLINE_DEBUG_STORE
			debug << ";SH";
#endif

			Bus->Write ( Value, Address, 0xffff );
			
			// bus is busy with something now
			Bus->ReserveBus ( c_CycleTime );
			break;
		
		case OPSW:
		case OPSWC2:
#ifdef INLINE_DEBUG_STORE
			debug << ";SW/SWC2";
#endif

			Bus->Write ( Value, Address, 0xffffffff );
			
			// bus is busy with something now
			Bus->ReserveBus ( c_CycleTime );
			break;
			
		case OPSWL:
#ifdef INLINE_DEBUG_STORE
			debug << ";SWL";
#endif

			Bus->Write ( Value >> ( ( 3 - ( Address & 3 ) ) << 3 ), Address & ~3, 0xffffffffUL >> ( ( 3 - ( Address & 3 ) ) << 3 ) );

			// bus is busy with something now
			Bus->ReserveBus ( c_CycleTime );
			break;
			
		case OPSWR:
#ifdef INLINE_DEBUG_STORE
			debug << ";SWR";
#endif

			Bus->Write ( Value << ( ( Address & 3 ) << 3 ), Address & ~3, 0xffffffffUL << ( ( Address & 3 ) << 3 ) );
			
			// bus is busy with something now
			Bus->ReserveBus ( c_CycleTime );
			break;
	}

#ifdef INLINE_DEBUG_STORE
	debug << "; ValueStored=" << Value;
#endif
*/

}



void Cpu::_cb_NONE ()
{
}

void Cpu::_cb_SB ()
{
	//Bus->Write ( _CPU->StoreBuffer.Get_Value (), _CPU->StoreBuffer.Get_Address (), 0xff );
	//Bus->ReserveBus ( c_CycleTime_Store );
}

void Cpu::_cb_SH ()
{
	//Bus->Write ( _CPU->StoreBuffer.Get_Value (), _CPU->StoreBuffer.Get_Address (), 0xffff );
	//Bus->ReserveBus ( c_CycleTime_Store );
}

void Cpu::_cb_SW ()
{
	//Bus->Write ( _CPU->StoreBuffer.Get_Value (), _CPU->StoreBuffer.Get_Address (), 0xffffffff );
	//Bus->ReserveBus ( c_CycleTime_Store );
}

void Cpu::_cb_SWL ()
{
	u32 Address, Value;
	//Address = _CPU->StoreBuffer.Get_Address ();
	//Value = _CPU->StoreBuffer.Get_Value ();
	//Bus->Write ( Value >> ( ( 3 - ( Address & 3 ) ) << 3 ), Address & ~3, 0xffffffffUL >> ( ( 3 - ( Address & 3 ) ) << 3 ) );
	//Bus->ReserveBus ( c_CycleTime_Store );
}

void Cpu::_cb_SWR ()
{
	//u32 Address, Value;
	//Address = _CPU->StoreBuffer.Get_Address ();
	//Value = _CPU->StoreBuffer.Get_Value ();
	//Bus->Write ( Value << ( ( Address & 3 ) << 3 ), Address & ~3, 0xffffffffUL << ( ( Address & 3 ) << 3 ) );
	//Bus->ReserveBus ( c_CycleTime_Store );
}


// this is the same as for SW
/*
static void Cpu::_cb_SWC2 ()
{
}
*/




Cpu::cbFunction Cpu::Refresh_BranchDelaySlot ( Instruction::Format i )
{
	//Instruction::Format i;
	//u32 Address;
	
	//DelaySlot *d = & ( DelaySlots [ NextDelaySlotIndex ] );
	//i = d->Instruction;
	
	switch ( i.Opcode )
	{
		case OPREGIMM:
		
			//switch ( i.Rt )
			//{
				//case RTBLTZ:	// regimm
				//case RTBGEZ:	// regimm
				//case RTBLTZL:	// regimm
				//case RTBGEZL:	// regimm
				//case RTBLTZAL:	// regimm
				//case RTBGEZAL:	// regimm
				//case RTBLTZALL:	// regimm
				//case RTBGEZALL:	// regimm
					//NextPC = PC + ( i.sImmediate << 2 );
					//break;
			//}
			
			return ProcessBranchDelaySlot_t<OPREGIMM>;
			break;
		
		case OPSPECIAL:
		
			//switch ( i.Funct )
			//{
				//case SPJR:
				//case SPJALR:
					//NextPC = d->Data;
					//break;
			//}
			
			return ProcessBranchDelaySlot_t<OPSPECIAL>;
			break;
			
	
		case OPJ:	// ok
		case OPJAL:	// ok
			//NextPC = ( 0xf0000000 & PC ) | ( i.JumpAddress << 2 );
			
			return ProcessBranchDelaySlot_t<OPJ>;
			break;
			
			
		case OPBEQ:
		case OPBNE:
		case OPBLEZ:
		case OPBGTZ:
		case OPBEQL:
		case OPBNEL:
		case OPBLEZL:
		case OPBGTZL:
		case OPCOP0:
		case OPCOP1:
		case OPCOP2:
			//NextPC = PC + ( i.sImmediate << 2 );
			
			return ProcessBranchDelaySlot_t<OPBEQ>;
			break;
	}

	return NULL;
}


void Cpu::Refresh_AllBranchDelaySlots ()
{
	DelaySlots [ 0 ].cb = Refresh_BranchDelaySlot ( DelaySlots [ 0 ].Instruction );
	DelaySlots [ 1 ].cb = Refresh_BranchDelaySlot ( DelaySlots [ 1 ].Instruction );
	
	// just in case I switch back
	DelaySlot0.cb = Refresh_BranchDelaySlot ( DelaySlot0.Instruction );
	DelaySlot1.cb = Refresh_BranchDelaySlot ( DelaySlot1.Instruction );
}



void Cpu::ProcessSynchronousInterrupt ( u32 ExceptionType )
{
	// *** todo *** Synchronous Interrupts can also be triggered by setting bits 8 or 9 (IP0 or IP1) of Cause register

	//////////////////////////////////////////////////////////////////////////////////
	// At this point:
	// PC points to instruction currently in the process of being executed
	// LastPC points to instruction before the one in the process of being executed
	// NextPC DOES NOT point to the next instruction to execute
	
#ifdef INLINE_DEBUG_SYNC_INT
	//if ( ExceptionType == EXC_SYSCALL )
	{
	debug << "\r\nSynchronous Interrupt PC=" << hex << PC << " NextPC=" << NextPC << " LastPC=" << LastPC << " Status=" << CPR0.Regs [ 12 ] << " Cause=" << CPR0.Regs [ 13 ];
	debug << "\r\nBranch Delay Instruction: " << R5900::Instruction::Print::PrintInstruction ( DelaySlot1.Instruction.Value ).c_str () << "  " << hex << DelaySlot1.Instruction.Value;
	}
#endif


	// step 3: shift left first 8 bits of status register by 2 and clear top 2 bits of byte
	//CPR0.Status.b0 = ( CPR0.Status.b0 << 2 ) & 0x3f;
	
	// step 4: set to kernel mode with interrupts disabled
	//CPR0.Status.IEc = 0;
	//CPR0.Status.KUc = 1;
	
	// R5900 step 1: switch to exception level 1/kernel mode (set Status.EXL=1)
	CPR0.Status.EXL = 1;
	
	// step 5: Store current address (has not been executed yet) into COP0 register "EPC" unless in branch delay slot
	// if in branch delay slot, then set field "BD" in Cause register and store address of previous instruction in COP0 register "EPC"
	// Branch Delay Slot 1 is correct since instruction is still in the process of being executed
	// probably only branches have delay slot on R5900
	//if ( DelaySlot1.Value && ( DelaySlot1.Instruction.Value < 0x40000000 ) )
	//if ( DelaySlot1.Value )
	if ( DelaySlots [ NextDelaySlotIndex ].Value )
	{
		// we are in branch delay slot - instruction in branch delay slot has not been executed yet, since it was "interrupted"
		CPR0.Cause.BD = 1;
		
		// this is actually not the previous instruction, but it is the previous instruction executed
		// this allows for branches in branch delay slots
		//CPR0.EPC = LastPC;
		CPR0.EPC = PC - 4;
		
		// no longer want to execute the branch that is in branch delay slot
		//DelaySlot1.Value = 0;
		DelaySlots [ NextDelaySlotIndex ].Value = 0;
		
		// ***testing*** may need to preserve the interrupts
		//Status.DelaySlot_Valid &= 0xfc;
	}
	else
	{
		// we are not in branch delay slot
		CPR0.Cause.BD = 0;
		
		// this is synchronous interrupt, so EPC gets set to the instruction that caused the interrupt
		CPR0.EPC = PC;
	}

	// step 6: Set correct interrupt pending bit in "IP" (Interrupt Pending) field of Cause register
	// actually, we need to send an interrupt acknowledge signal back to the interrupting device probably
	
	// step 7: set PC to interrupt vector address
	if ( CPR0.Status.BEV == 0 )
	{
		// check if tlb miss exception - skip for R3000A for now
		
		// set PC with interrupt vector address
		// kseg1 - 0xa000 0000 - 0xbfff ffff : translated to physical address by removing top three bits
		// this region is not cached
		//NextPC = c_GeneralInterruptVector;
		NextPC = c_CommonInterruptVector;
	}
	else
	{
		// check if tlb miss exception - skip for R3000A for now
		
		// set PC with interrupt vector address
		// kseg0 - 0x8000 0000 - 0x9fff ffff : translated to physical address by removing top bit
		// this region is cached
		//NextPC = c_BootInterruptVector;
		NextPC = c_CommonInterruptVector_BEV;
	}
	
	// step 8: set Cause register so that software can see the reason for the exception
	// we know that this is an synchronous interrupt
	//CPR0.Cause.ExcCode = Status.ExceptionType;
	CPR0.Cause.ExcCode = ExceptionType;
	
	// *** todo *** instruction needs to be passed above because lower 2 bits of opcode go into Cause Reg bits 28 and 29


	// step 9: continue execution
	
	// when returning from interrupt you should
	// Step 1: enable interrupts globally
	// Step 2: shift right first 8 bits of status register
	// Step 3: set ExceptionCode to EXC_Unknown
		

#ifdef INLINE_DEBUG_SYNC_INT
	debug << "\r\n->PC=" << hex << PC << " NextPC=" << NextPC << " LastPC=" << LastPC << " Status=" << CPR0.Regs [ 12 ] << " Cause=" << CPR0.Regs [ 13 ];
	debug << "\r\nBranch Delay Instruction: " << R5900::Instruction::Print::PrintInstruction ( DelaySlot1.Instruction.Value ).c_str () << "  " << hex << DelaySlot1.Instruction.Value;
	DebugNextERET = 1;
#endif

#ifdef INLINE_DEBUG_SYSCALL

	
	// for PS2, the function number being called via SYSCALL is put into the R3 register
	
	// testing
	if ( GPR [ 3 ].u == 122 )
	{
		DebugNextERET = 0;
		return;
	}
	
	DebugNextERET = 1;

	// toss this in possibly
	//debug << "\r\n->PC=" << hex << PC << " NextPC=" << NextPC << " LastPC=" << LastPC << " Status=" << CPR0.Regs [ 12 ] << " Cause=" << CPR0.Regs [ 13 ];
	//debug << "\r\nBranch Delay Instruction: " << R5900::Instruction::Print::PrintInstruction ( DelaySlot1.Instruction.Value ).c_str () << "  " << hex << DelaySlot1.Instruction.Value;
	
static const char* c_sSyscallFunctionNames [] = {	
   "0. RFU000_FullReset",
   "1 ResetEE",
   "2 SetGsCrt",
   "3 RFU003",
   "4 Exit",
   "5 RFU005",
   "6 LoadExecPS2",
   "7 ExecPS",
   "8 RFU008",
   "9 RFU009",
  "10 AddSbusIntcHandler",
  "11 RemoveSbusIntcHandler",
  "12 Interrupt2Iop",
  "13 SetVTLBRefillHandler",
  "14 SetVCommonHandler",
  "15 SetVInterruptHandler",
  "16 AddIntcHandler; 16 AddIntcHandler2",
  "17 RemoveIntcHandler",
  "18 AddDmacHandler; 18 AddDmacHandler2",
  "19 RemoveDmacHandler",
  "20 _EnableIntc",
  "21 _DisableIntc",
  "22 _EnableDmac",
  "23 _DisableDmac",
 "252 SetAlarm",
 "253 ReleaseAlarm",
 "-26 _iEnableIntc",
 "-27 _iDisableIntc",
 "-28 _iEnableDmac",
 "-29 _iDisableDmac",
 "-254 iSetAlarm",
 "-255 iReleaseAlarm",
  "32 CreateThread",
  "33 DeleteThread",
  "34 StartThread",
  "35 ExitThread",
  "36 ExitDeleteThread",
  "37 TerminateThread",
  "-38 iTerminateThread",
  "39 DisableDispatchThread",
  "40 EnableDispatchThread",
  "41 ChangeThreadPriority",
  "-42 iChangeThreadPriority",
  "43 RotateThreadReadyQueue",
  "-44 _iRotateThreadReadyQueue",
  "45 ReleaseWaitThread",
  "-46 iReleaseWaitThread",
  "47 GetThreadId",
  "48 ReferThreadStatus",
  "-49 iReferThreadStatus",
  "50 SleepThread",
  "51 WakeupThread",
  "-52 _iWakeupThread",
  "53 CancelWakeupThread",
  "-54 iCancelWakeupThread",
  "55 SuspendThread",
  "-56 _iSuspendThread",
  "57 ResumeThread",
  "-58 iResumeThread",
  "59 JoinThread",
  "60 RFU060",
  "61 RFU061",
  "62 EndOfHeap",
  "63 RFU063",
  "64 CreateSema",
  "65 DeleteSema",
  "66 SignalSema",
  "-67 iSignalSema",
  "68 WaitSema",
  "69 PollSema",
  "-70 iPollSema",
  "71 ReferSemaStatus",
  "-72 iReferSemaStatus>:",
  "73 RFU073",
  "74 SetOsdConfigParam",
  "75 GetOsdConfigParam",
  "76 GetGsHParam",
  "77 GetGsVParam",
  "78 SetGsHParam",
  "79 SetGsVParam",
  "80 RFU080_CreateEventFlag",
  "81 RFU081_DeleteEventFlag",
  "82 RFU082_SetEventFlag",
  "-83 RFU083_iSetEventFlag",
  "84 RFU084_ClearEventFlag",
  "-85 RFU085_iClearEventFlag",
  "86 RFU086_WaitEvnetFlag",
  "87 RFU087_PollEvnetFlag",
  "-88 RFU088_iPollEvnetFlag",
  "89 RFU089_ReferEventFlagStatus",
  "-90 RFU090_iReferEventFlagStatus",
  "91 RFU091",
  "92 EnableIntcHandler; -92 iEnableIntcHandler",
  "93 DisableIntcHandler; -93 iDisableIntcHandler",
  "94 EnableDmacHandler; -94 iEnableDmacHandler",
  "95 DisableDmacHandler; -95 iDisableDmacHandler",
  "96 KSeg0",
  "97 EnableCache",
  "98 DisableCache",
  "99 GetCop0",
 "100 FlushCache",
 "101 Unknown",
 "102 CpuConfig",
 "-103 iGetCop0",
 "-104 iFlushCache",
 "105 Unknown",
 "-106 iCpuConfig",
 "107 sceSifStopDma",
 "108 SetCPUTimerHandler",
 "109 SetCPUTimer",
 "110 SetOsdConfigParam2",
 "111 GetOsdConfigParam2",
 "112 GsGetIMR; -112 iGsGetIMR",
 "113 GsPutIMR; -113 iGsPutIMR",
 "114 SetPgifHandler",
 "115 SetVSyncFlag",
 "116 RFU116",
 "117 _print",
 "118 sceSifDmaStat; -118 isceSifDmaStat",
 "119 sceSifSetDma; -119 isceSifSetDma",
 "120 sceSifSetDChain; -120 isceSifSetDChain",
 "121 sceSifSetReg",
 "122 sceSifGetReg",
 "123 ExecOSD",
 "124 Deci2Call",
 "125 PSMode",
 "126 MachineType",
 "127 GetMemorySize" };
	
	if ( CPR0.Cause.ExcCode == EXC_SYSCALL )
	{
		u32 SyscallNumber;
		SyscallNumber = GPR [ 3 ].s;
		if ( GPR [ 3 ].s < 0 ) SyscallNumber = -GPR [ 3 ].s;
		if ( SyscallNumber != 124 )
		{
		debugBios << "\r\nSyscall; " << hex << setw(8) << PC << " " << dec << CycleCount << " Function: ";
		
		// output syscall function name
		if ( SyscallNumber < 128 )
		{
			debugBios << c_sSyscallFunctionNames [ SyscallNumber ];
		}
		else
		{
			debugBios << "Unknown";
		}
		
		debugBios << "; r3 (syscall number) = " << dec << GPR [ 3 ].s;
		debugBios << "; r1 = " << hex << GPR [ 1 ].u;
		debugBios << "; r2 = " << GPR [ 2 ].u;
		debugBios << "; r4 = " << GPR [ 4 ].u;
		debugBios << "; r5 = " << GPR [ 5 ].u;
		debugBios << "; r6 = " << GPR [ 6 ].u;
		}
		else
		{
			static u32 PollCount;
			
			// check reason for deci2call
			// call number is in a0
			switch ( GPR [ 4 ].u )
			{
				// open
				case 0x1:
					PollCount = 0;
					debugBios << "\r\nSyscall; " << hex << setw(8) << PC << " " << dec << CycleCount << " Function: deci2call->";
					debugBios << "open";
					break;
					
				// close
				case 0x2:
					PollCount = 0;
					debugBios << "\r\nSyscall; " << hex << setw(8) << PC << " " << dec << CycleCount << " Function: deci2call->";
					debugBios << "close";
					break;
					
				// reqsend
				case 0x3:
					PollCount = 0;
					debugBios << "\r\nSyscall; " << hex << setw(8) << PC << " " << dec << CycleCount << " Function: deci2call->";
					debugBios << "reqsend";
					break;
					
				// poll
				case 0x4:
					if ( !PollCount )
					{
					debugBios << "\r\nSyscall; " << hex << setw(8) << PC << " " << dec << CycleCount << " Function: deci2call->";
					debugBios << "poll";
					PollCount++;
					}
					break;
				
				// exrecv
				case 0x5:
					PollCount = 0;
					debugBios << "\r\nSyscall; " << hex << setw(8) << PC << " " << dec << CycleCount << " Function: deci2call->";
					debugBios << "exrecv";
					break;
				
				// exsend
				case 0x6:
					PollCount = 0;
					debugBios << "\r\nSyscall; " << hex << setw(8) << PC << " " << dec << CycleCount << " Function: deci2call->";
					debugBios << "exsend";
					break;
					
				// kputs
				case 0x10:
					PollCount = 0;
					debugBios << "kputs";
					break;
					
				// unknown
				default:
					debugBios << "unknown";
					break;
			}
		}
	}
#endif

}

void Cpu::ProcessAsynchronousInterrupt ()
{

	///////////////////////////////////////////////////////////////////
	// At this point:
	// PC points to instruction just executed
	// LastPC points to instruction before the one just executed
	// NextPC points to the next instruction to execute

#ifdef INLINE_DEBUG_ASYNC_INT
	debug << "\r\nASYNC-INT PC=" << hex << PC << " " << dec << CycleCount << hex << " NextPC=" << NextPC << " LastPC=" << LastPC << " Status=" << CPR0.Regs [ 12 ] << " Cause=" << CPR0.Regs [ 13 ];
	debug << hex << " ISTAT=" << *_Debug_IntcStat << " IMASK=" << *_Debug_IntcMask;
	//debug << "\r\nBranch Delay Instruction: " << R5900::Instruction::Print::PrintInstruction ( DelaySlot1.Instruction.Value ).c_str () << "  " << hex << DelaySlot1.Instruction.Value;
	//debug << "\r\nR29=" << _CPU->GPR [ 29 ].uw0 << " R31=" << _CPU->GPR [ 31 ].uw0;
	// << " 0x3201d0=" << Bus->MainMemory.b32 [ 0x3201d0 >> 2 ];
	DebugNextERET = 1;
#endif

#ifdef VERBOSE_MULTIPLE_INT_CAUSE
	// debugging - check for multiple interrupt reasons
	if ( ( CPR0.Regs [ 12 ] & CPR0.Regs [ 13 ] & 0xc00 ) == 0xc00 )
	{
		cout << "\n******************************************";
		cout << "\nhps2x64: R5900: multiple interrupt causes.\n";
		cout << "********************************************\n";
	}
#endif
	
	// step 1: make sure interrupts are enabled globally (Status register field "IEc" equal to 1) and not masked
	// and that external interrupts are enabled (Status register bit 10 equal to 1)
	// *** note *** already checked for this stuff
//	if ( ( CPR0.Status.IEc == 1 ) && ( CPR0.Regs [ 12 ] & CPR0.Regs [ 13 ] & 0x4 ) )
//	{
		// step 2: make sure corresponding interrupt bit is set (enabled) in 8-bit Status register field "Im" (Interrupt Mask)
		
		// step 3: shift left first 8 bits of status register by 2 and clear top 2 bits of byte
		//CPR0.Status.b0 = ( CPR0.Status.b0 << 2 ) & 0x3f;
		
		// step 4: set to kernel mode with interrupts disabled
		//CPR0.Status.IEc = 0;
		//CPR0.Status.KUc = 1;
		
		// R5900 step 1: switch to exception level 1/kernel mode (set Status.EXL=1)
		CPR0.Status.EXL = 1;

		// step 5: Store current address (has not been executed yet) into COP0 register "EPC" unless in branch delay slot
		// if in branch delay slot, then set field "BD" in Cause register and store address of previous instruction in COP0 register "EPC"
		// for R5900, the only instruction probably with delay slot is branch
		//if ( DelaySlot1.Value && ( DelaySlot1.Instruction.Value < 0x40000000 ) )
		//if ( DelaySlot1.Value )
		//if ( DelaySlots [ NextDelaySlotIndex ].Value )
		if ( Status.DelaySlot_Valid )
		{
#ifdef INLINE_DEBUG_ASYNC_INT
	//debug << "\r\nBranch Delay Instruction: " << R5900::Instruction::Print::PrintInstruction ( DelaySlot1.Instruction.Value ).c_str () << "  " << hex << DelaySlot1.Instruction.Value;
#endif

			// we are in branch delay slot - instruction in branch delay slot has not been executed yet, since it was "interrupted"
			CPR0.Cause.BD = 1;
			
			// this is actually not the previous instruction, but it is the previous instruction executed
			// this allows for branches in branch delay slots
			//CPR0.EPC = LastPC;
			// if we are in branch delay slot, then delay slot has branch at this point, so the instruction just executed is the branch
			//CPR0.EPC = PC;
			//CPR0.EPC = LastPC;
			CPR0.EPC = PC - 4;
			
			// no longer want to execute the branch that is in branch delay slot
			//DelaySlot1.Value = 0;
			DelaySlots [ NextDelaySlotIndex ].Value = 0;
			Status.DelaySlot_Valid = 0;
			
			// ***testing*** may need to preserve interrupts
			//Status.DelaySlot_Valid &= 0xfc;
		}
		else
		{
			// we are not in branch delay slot
			CPR0.Cause.BD = 0;
			
			// this is actually not the next instruction, but rather the next instruction to be executed
			//CPR0.EPC = NextPC;
			CPR0.EPC = PC;
		}

		// step 6: Set correct interrupt pending bit in "IP" (Interrupt Pending) field of Cause register
		// actually, we need to send an interrupt acknowledge signal back to the interrupting device probably
		
		// step 7: set PC to interrupt vector address
		if ( CPR0.Status.BEV == 0 )
		{
			// check if tlb miss exception - skip for R3000A for now
			
			// set PC with interrupt vector address
			// kseg0 - 0x8000 0000 - 0x9fff ffff : translated to physical address by removing top bit
			// this region is cached
			//PC = c_GeneralInterruptVector;
			PC = c_ExternalInterruptVector;
		}
		else
		{
			// check if tlb miss exception - skip for R3000A for now
			
			// set PC with interrupt vector address
			// kseg1 - 0xa000 0000 - 0xbfff ffff : translated to physical address by removing top three bits
			// this region is not cached
			//PC = c_BootInterruptVector;
			PC = c_ExternalInterruptVector_BEV;
		}
		
		// step 8: set Cause register so that software can see the reason for the exception
		// we know that this is an asynchronous interrupt
		CPR0.Cause.ExcCode = EXC_INT;
	
		// step 9: continue execution
		
		// when returning from interrupt you should
		// Step 1: enable interrupts globally
		// Step 2: shift right first 8 bits of status register
		// - this stuff is done automatically by RFE instruction
//	}

	// interrupt related stuff was modified, so update interrupts
	UpdateInterrupt ();


#ifdef INLINE_DEBUG_ASYNC_INT
	debug << "\r\n->PC=" << hex << PC << hex << " Status=" << CPR0.Regs [ 12 ] << " Cause=" << CPR0.Regs [ 13 ] << " EPC=" << CPR0.EPC << " BD=" << CPR0.Cause.BD;
	//debug << "\r\nBranch Delay Instruction: " << R5900::Instruction::Print::PrintInstruction ( DelaySlot1.Instruction.Value ).c_str () << "  " << hex << DelaySlot1.Instruction.Value;
#endif
}











void Cpu::SetPC ( u32 Value )
{
	PC = Value;
}



Cpu::Buffer::Buffer ()
{
	// zero object
	memset ( this, 0, sizeof( Buffer ) );
	
	//ReadIndex = 0;
	//WriteIndex = 0;

	// * not needed since this is cleared when processor starts in status
	// initialize the buffer
	//for ( u32 i = 0; i < 4; i++ )
	//{
		//Buf [ i ].isValid = false;
	//}
}


#ifdef ENABLE_GUI_DEBUGGER
void Cpu::DebugWindow_Enable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	static const char* COP0_Names [] = { "Index", "Random", "EntryLo0", "EntryLo1", "Context", "PageMask", "Wired", "Reserved",
								"BadVAddr", "Count", "EntryHi", "Compare", "Status", "Cause", "EPC", "PRId",
								"Config", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "BadPAddr",
								"Debug", "Perf", "Reserved", "Reserved", "TagLo", "TagHi", "ErrorEPC", "Reserved" };
								
	static constexpr char* DisAsm_Window_ColumnHeadings [] = { "Address", "@", ">", "Instruction", "Inst (hex)" };
								
	static constexpr char* FontName = "Courier New";
	static constexpr int FontSize = 6;
	
	static constexpr char* DebugWindow_Caption = "R5900 Debug Window";
	static constexpr int DebugWindow_X = 10;
	static constexpr int DebugWindow_Y = 10;
	static constexpr int DebugWindow_Width = 995;
	static constexpr int DebugWindow_Height = 420;
	
	static constexpr int GPRList_X = 0;
	static constexpr int GPRList_Y = 0;
	static constexpr int GPRList_Width = 190;
	static constexpr int GPRList_Height = 380;

	static constexpr int COP1List_X = GPRList_X + GPRList_Width;
	static constexpr int COP1List_Y = 0;
	static constexpr int COP1List_Width = 175;
	static constexpr int COP1List_Height = 300;
	
	static constexpr int COP2_CPCList_X = COP1List_X + COP1List_Width;
	static constexpr int COP2_CPCList_Y = 0;
	static constexpr int COP2_CPCList_Width = 175;
	static constexpr int COP2_CPCList_Height = 300;
	
	static constexpr int COP2_CPRList_X = COP2_CPCList_X + COP2_CPCList_Width;
	static constexpr int COP2_CPRList_Y = 0;
	static constexpr int COP2_CPRList_Width = 175;
	static constexpr int COP2_CPRList_Height = 300;
	
	static constexpr int DisAsm_X = COP2_CPRList_X + COP2_CPRList_Width;
	static constexpr int DisAsm_Y = 0;
	static constexpr int DisAsm_Width = 270;
	static constexpr int DisAsm_Height = 380;
	
	static constexpr int MemoryViewer_Columns = 8;
	static constexpr int MemoryViewer_X = GPRList_X + GPRList_Width;
	static constexpr int MemoryViewer_Y = 300;
	static constexpr int MemoryViewer_Width = 250;
	static constexpr int MemoryViewer_Height = 80;
	
	static constexpr int BkptViewer_X = MemoryViewer_X + MemoryViewer_Width;
	static constexpr int BkptViewer_Y = 300;
	static constexpr int BkptViewer_Width = 275;
	static constexpr int BkptViewer_Height = 80;
	
	int i;
	stringstream ss;
	
#ifdef INLINE_DEBUG
	debug << "\r\nStarting Cpu::DebugWindow_Enable";
#endif
	
	if ( !DebugWindow_Enabled )
	{
		// create the main debug window
		DebugWindow = new WindowClass::Window ();
		DebugWindow->Create ( DebugWindow_Caption, DebugWindow_X, DebugWindow_Y, DebugWindow_Width, DebugWindow_Height );
		DebugWindow->Set_Font ( DebugWindow->CreateFontObject ( FontSize, FontName ) );
		DebugWindow->DisableCloseButton ();
		
		// create "value lists"
		GPR_ValueList = new DebugValueList<u32> ();
		COP0_ValueList = new DebugValueList<u32> ();
		COP2_CPCValueList = new DebugValueList<float> ();
		COP2_CPRValueList = new DebugValueList<u32> ();
		
		// create the value lists
		GPR_ValueList->Create ( DebugWindow, GPRList_X, GPRList_Y, GPRList_Width, GPRList_Height );
		COP0_ValueList->Create ( DebugWindow, COP1List_X, COP1List_Y, COP1List_Width, COP1List_Height );
		COP2_CPCValueList->Create ( DebugWindow, COP2_CPCList_X, COP2_CPCList_Y, COP2_CPCList_Width, COP2_CPCList_Height );
		COP2_CPRValueList->Create ( DebugWindow, COP2_CPRList_X, COP2_CPRList_Y, COP2_CPRList_Width, COP2_CPRList_Height );
		
		GPR_ValueList->EnableVariableEdits ();
		COP0_ValueList->EnableVariableEdits ();
		COP2_CPCValueList->EnableVariableEdits ();
		COP2_CPRValueList->EnableVariableEdits ();
	
		// add variables into lists
		for ( i = 0; i < 32; i++ )
		{
			ss.str ("");
			ss << "R" << dec << i << "_x";
			GPR_ValueList->AddVariable ( ss.str().c_str(), &(_CPU->GPR [ i ].uw0) );
			ss.str ("");
			ss << "R" << dec << i << "_y";
			GPR_ValueList->AddVariable ( ss.str().c_str(), &(_CPU->GPR [ i ].uw1) );
			ss.str ("");
			ss << "R" << dec << i << "_z";
			GPR_ValueList->AddVariable ( ss.str().c_str(), &(_CPU->GPR [ i ].uw2) );
			ss.str ("");
			ss << "R" << dec << i << "_w";
			GPR_ValueList->AddVariable ( ss.str().c_str(), &(_CPU->GPR [ i ].uw3) );
			
			COP0_ValueList->AddVariable ( COP0_Names [ i ], &(_CPU->CPR0.Regs [ i ]) );

			// add FPU vars too
			ss.str ("");
			ss << "f" << dec << i;
			COP2_CPCValueList->AddVariable ( ss.str().c_str(), &(_CPU->CPR1 [ i ].f) );
			
			/*
			if ( i < 16 )
			{
				ss.str("");
				ss << "CPC2_" << dec << ( i << 1 );
				COP2_CPCValueList->AddVariable ( ss.str().c_str(), &(_CPU->COP2.CPC2.Regs [ i << 1 ]) );
				
				ss.str("");
				ss << "CPC2_" << dec << ( ( i << 1 ) + 1 );
				COP2_CPCValueList->AddVariable ( ss.str().c_str(), &(_CPU->COP2.CPC2.Regs [ ( i << 1 ) + 1 ]) );
			}
			else
			{
				ss.str("");
				ss << "CPR2_" << dec << ( ( i - 16 ) << 1 );
				COP2_CPRValueList->AddVariable ( ss.str().c_str(), &(_CPU->COP2.CPR2.Regs [ ( i - 16 ) << 1 ]) );
				
				ss.str("");
				ss << "CPR2_" << dec << ( ( ( i - 16 ) << 1 ) + 1 );
				COP2_CPRValueList->AddVariable ( ss.str().c_str(), &(_CPU->COP2.CPR2.Regs [ ( ( i - 16 ) << 1 ) + 1 ]) );
			}
			*/
		}
		
		// Don't forget to show the HI and LO registers
		//GPR_ValueList->AddVariable ( "LO", &(_CPU->HiLo.uLo) );
		GPR_ValueList->AddVariable ( "LOx", &(_CPU->LO.uw0) );
		GPR_ValueList->AddVariable ( "LOy", &(_CPU->LO.uw1) );
		GPR_ValueList->AddVariable ( "LOz", &(_CPU->LO.uw2) );
		GPR_ValueList->AddVariable ( "LOw", &(_CPU->LO.uw3) );
		//GPR_ValueList->AddVariable ( "HI", &(_CPU->HiLo.uHi) );
		GPR_ValueList->AddVariable ( "HIx", &(_CPU->HI.uw0) );
		GPR_ValueList->AddVariable ( "HIy", &(_CPU->HI.uw1) );
		GPR_ValueList->AddVariable ( "HIz", &(_CPU->HI.uw2) );
		GPR_ValueList->AddVariable ( "HIw", &(_CPU->HI.uw3) );
		
		// also add PC and CycleCount
		GPR_ValueList->AddVariable ( "PC", &(_CPU->PC) );
		GPR_ValueList->AddVariable ( "NextPC", &(_CPU->NextPC) );
		GPR_ValueList->AddVariable ( "LastPC", &(_CPU->LastPC) );
		GPR_ValueList->AddVariable ( "CycleLO", (u32*) &(_CPU->CycleCount) );
		
		GPR_ValueList->AddVariable ( "LastReadAddress", &(_CPU->Last_ReadAddress) );
		GPR_ValueList->AddVariable ( "LastWriteAddress", &(_CPU->Last_WriteAddress) );
		GPR_ValueList->AddVariable ( "LastReadWriteAddress", &(_CPU->Last_ReadWriteAddress) );
		
		// need to add in load delay slot values
		GPR_ValueList->AddVariable ( "D0_INST", &(_CPU->DelaySlot0.Instruction.Value) );
		GPR_ValueList->AddVariable ( "D0_VAL", &(_CPU->DelaySlot0.Data) );
		GPR_ValueList->AddVariable ( "D1_INST", &(_CPU->DelaySlot1.Instruction.Value) );
		GPR_ValueList->AddVariable ( "D1_VAL", &(_CPU->DelaySlot1.Data) );
		
		//GPR_ValueList->AddVariable ( "SPUCC", (u32*) _SpuCycleCount );
		
		GPR_ValueList->AddVariable ( "Trace", &TraceValue );
		
		GPR_ValueList->AddVariable ( "PCount", & Playstation2::GPU::_GPU->Primitive_Count );
		
		GPR_ValueList->AddVariable ( "a0", (u32*) & ( Bus->BIOS.b32 [ 0x251f80 >> 2 ] ) );
		GPR_ValueList->AddVariable ( "a1", (u32*) & ( Bus->MainMemory.b32 [ 0x1e290 >> 2 ] ) );
		GPR_ValueList->AddVariable ( "a2", (u32*) & ( Bus->MainMemory.b32 [ 0x1e294 >> 2 ] ) );
		GPR_ValueList->AddVariable ( "a3", (u32*) & ( Bus->MainMemory.b32 [ 0x1edc0 >> 2 ] ) );
		GPR_ValueList->AddVariable ( "a4", (u32*) & ( Bus->MainMemory.b32 [ 0x1edc4 >> 2 ] ) );
		//GPR_ValueList->AddVariable ( "a1", (u32*) & ( VU::_VU[1]->VuMem64 [ ( 0x56 << 1 ) + 1 ] ) );
		//GPR_ValueList->AddVariable ( "a0", &_CPU->testvar [ 0 ] );
		//GPR_ValueList->AddVariable ( "a1", &_CPU->testvar [ 1 ] );
		//GPR_ValueList->AddVariable ( "a2", &_CPU->testvar [ 2 ] );
		//GPR_ValueList->AddVariable ( "a3", &_CPU->testvar [ 3 ] );
		//GPR_ValueList->AddVariable ( "a4", &_CPU->testvar [ 4 ] );
		GPR_ValueList->AddVariable ( "a5", &_CPU->testvar [ 5 ] );
		GPR_ValueList->AddVariable ( "a6", &_CPU->testvar [ 6 ] );
		GPR_ValueList->AddVariable ( "a7", &_CPU->testvar [ 7 ] );
		
		GPR_ValueList->AddVariable ( "GNE", (u32*) &(Playstation2::GPU::_GPU->lVBlank) );

		// I can always move these next ones around if needed
		//GPR_ValueList->AddVariable ( "LDValid", &(_CPU->Status.LoadStore_Valid) );
		
		// add the loading bitmap
		//GPR_ValueList->AddVariable ( "LoadBitmap", &(_CPU->GPRLoading_Bitmap) );
		
		/*
		for ( i = 0; i < 4; i++ )
		{
			ss.str ("");
			ss << "LD" << i << "_INST";
			GPR_ValueList->AddVariable ( ss.str().c_str(), &(_CPU->LoadBuffer.Buf [ i ].Inst.Value) );
			ss.str ("");
			ss << "LD" << i << "_ADDR";
			GPR_ValueList->AddVariable ( ss.str().c_str(), &(_CPU->LoadBuffer.Buf [ i ].Address) );
			ss.str ("");
			ss << "LD" << i << "_VALUE";
			GPR_ValueList->AddVariable ( ss.str().c_str(), &(_CPU->LoadBuffer.Buf [ i ].Value) );
			ss.str ("");
			ss << "LD" << i << "_INDEX";
			GPR_ValueList->AddVariable ( ss.str().c_str(), &(_CPU->LoadBuffer.Buf [ i ].Index) );
		}
		*/

		/*
		for ( i = 0; i < 4; i++ )
		{
			ss.str ("");
			ss << "ST" << i << "_INST";
			GPR_ValueList->AddVariable ( ss.str().c_str(), &(_CPU->StoreBuffer.Buf [ i ].Inst.Value) );
			ss.str ("");
			ss << "ST" << i << "_ADDR";
			GPR_ValueList->AddVariable ( ss.str().c_str(), &(_CPU->StoreBuffer.Buf [ i ].Address) );
			ss.str ("");
			ss << "ST" << i << "_VALUE";
			GPR_ValueList->AddVariable ( ss.str().c_str(), &(_CPU->StoreBuffer.Buf [ i ].Value) );
			ss.str ("");
			//ss << "ST" << i << "_INDEX";
			//GPR_ValueList->AddVariable ( ss.str().c_str(), &(_CPU->StoreBuffer.Buf [ i ].Index) );
		}
		*/

		// temporary space
		//GPR_ValueList->AddVariable ( "0x1F8001A8", (u32*) &(_CPU->DCache.b8 [ 0x1A8 ]) );

		// create the disassembly window
		DisAsm_Window = new Debug_DisassemblyViewer ( Breakpoints );
		DisAsm_Window->Create ( DebugWindow, DisAsm_X, DisAsm_Y, DisAsm_Width, DisAsm_Height, (Debug_DisassemblyViewer::DisAsmFunction) R5900::Instruction::Print::PrintInstruction );
		
		DisAsm_Window->Add_MemoryDevice ( "RAM", Playstation2::DataBus::MainMemory_Start, Playstation2::DataBus::MainMemory_Size, (u8*) Bus->MainMemory.b8 );
		DisAsm_Window->Add_MemoryDevice ( "BIOS", Playstation2::DataBus::BIOS_Start, Playstation2::DataBus::BIOS_Size, (u8*) Bus->BIOS.b8 );
		
		DisAsm_Window->SetProgramCounter ( &_CPU->PC );
		
		
		// create window area for breakpoints
		Breakpoint_Window = new Debug_BreakpointWindow ( Breakpoints );
		Breakpoint_Window->Create ( DebugWindow, BkptViewer_X, BkptViewer_Y, BkptViewer_Width, BkptViewer_Height );
		
		// create the viewer for D-Cache scratch pad
		ScratchPad_Viewer = new Debug_MemoryViewer ();
		
		ScratchPad_Viewer->Create ( DebugWindow, MemoryViewer_X, MemoryViewer_Y, MemoryViewer_Width, MemoryViewer_Height, MemoryViewer_Columns );
		//ScratchPad_Viewer->Add_MemoryDevice ( "ScratchPad", c_ScratchPadRam_Addr, c_ScratchPadRam_Size, _CPU->DCache.b8 );
		ScratchPad_Viewer->Add_MemoryDevice ( "ScratchPad", Playstation2::DataBus::ScratchPad_Start, Playstation2::DataBus::ScratchPad_Size, (u8*) Bus->ScratchPad.b8 );
		
		
		// mark debug as enabled now
		DebugWindow_Enabled = true;
		
		// update the value lists
		DebugWindow_Update ();
	}
	
#endif
	
}

void Cpu::DebugWindow_Disable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	if ( DebugWindow_Enabled )
	{
		delete DebugWindow;
		delete GPR_ValueList;
		delete COP0_ValueList;
		delete COP2_CPCValueList;
		delete COP2_CPRValueList;

		delete DisAsm_Window;
		
		delete Breakpoint_Window;
		delete ScratchPad_Viewer;
		
		// disable debug window
		DebugWindow_Enabled = false;
	}
	
#endif

}


void Cpu::DebugWindow_Update ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	if ( DebugWindow_Enabled )
	{
		GPR_ValueList->Update();
		COP0_ValueList->Update();
		COP2_CPCValueList->Update();
		COP2_CPRValueList->Update();
		DisAsm_Window->GoTo_Address ( _CPU->PC );
		DisAsm_Window->Update ();
		Breakpoint_Window->Update ();
		ScratchPad_Viewer->Update ();
	}
	
#endif

}

#endif


}



