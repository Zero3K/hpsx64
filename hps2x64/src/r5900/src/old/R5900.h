/*
	Copyright (C) 2012-2016

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



#ifndef _R5900_H_

#define _R5900_H_


#include <iostream>
#include <sstream>

#ifdef ENABLE_DEBUGGER_GUI
#include "WinApiHandler.h"
#endif

#include "types.h"

//#include "R5900Encoder.h"
//#include "x64Encoder.h"

using namespace std;




union Mips128GPR
{
	long Reg32 [ 4 ];
	long long Reg64 [ 2 ];
};


union Reg128
{
	struct
	{
		u64 q0;
		u64 q1;
	};
	
	struct
	{
		u32 d0;
		u32 d1;
		u32 d2;
		u32 d3;
	};
	
	struct
	{
		u16 h0;
		u16 h1;
		u16 h2;
		u16 h3;
		u16 h4;
		u16 h5;
		u16 h6;
		u16 h7;
	};
	
	struct
	{
		u8 b0;
		u8 b1;
		u8 b2;
		u8 b3;
		u8 b4;
		u8 b5;
		u8 b6;
		u8 b7;
		u8 b8;
		u8 b9;
		u8 b10;
		u8 b11;
		u8 b12;
		u8 b13;
		u8 b14;
		u8 b15;
	};
	
	u64 q [ 2 ];
	u32 d [ 4 ];
	u16 h [ 8 ];
	u8 b [ 16 ];
};


namespace R5900
{

	struct EntryLo0_Reg
	{
		union
		{
			unsigned long l;

			struct
			{
				// bit 0 - Global - r/w
				unsigned long G : 1;
				
				// bit 1 - Valid - set to 1 if the TLB entry is enabled - r/w
				unsigned long V : 1;
				
				// bit 2 - Dirty - set to 1 if writable - r/w
				unsigned long D : 1;
				
				// bits 3-5 - TLB page coherency - 2: uncached, 3: Cached, 7: uncached accelerated - r/w
				unsigned long C : 3;
				
				// bits 6-25 - Page frame number (the upper bits of the physical address) - r/w
				unsigned long PFN : 20;
				
				// bits 26-30 - zero
				unsigned long zero : 5;
				
				// bit 31 - Memory type - 0: Main Memory, 1: Scratchpad RAM - r/w - only for EntryLo0
				unsigned long S : 1;
			};

		};
	};
	
	struct EntryLo1_Reg
	{
		union
		{
			unsigned long l;
			
			struct
			{
				// bit 0 - Global - r/w
				unsigned long G : 1;
				
				// bit 1 - Valid - set to 1 if the TLB entry is enabled - r/w
				unsigned long V : 1;
				
				// bit 2 - Dirty - set to 1 if writable - r/w
				unsigned long D : 1;
				
				// bits 3-5 - TLB page coherency - 2: uncached, 3: Cached, 7: uncached accelerated - r/w
				unsigned long C : 3;
				
				// bits 6-25 - Page frame number (the upper bits of the physical address) - r/w
				unsigned long PFN : 20;
			};
		};
	};

	struct Context_Reg
	{
		union
		{
			unsigned long l;
			
			struct
			{
				// bits 0-3 - zero
				unsigned long zero : 4;
				
				// bits 4-22 - Virtual page address that caused TLB miss
				unsigned long BadVPN2 : 19;
				
				// bits 23-31 - Page table address
				unsigned long PTEBase : 9;
			};
		};
	};
	
	struct PageMask_Reg
	{
		union
		{
			unsigned long l;
			
			struct
			{
				// bits 0-12 - zero
				unsigned long zero : 13;
				
				// bits 13-24 - Page size comparison mask - 0: 4KB, 3: 16KB, 15: 64KB, 63: 256KB, 65536: 1MB, 256K-1: 4MB, 1M-1: 16MB - r/w
				// start value:
				unsigned long MASK : 12;
			};
		};
	};

	struct EntryHi_Reg
	{
		union
		{
			unsigned long l;
			
			struct
			{
				// bits 0-7 - ASID - Address space ID - r/w
				// start value:
				unsigned long ASID : 8;
				
				// bits 8-12 - zero
				unsigned long zero : 5;
				
				// bits 13-31 - VPN2 - Virtual page number divided by two - r/w
				// start value:
				unsigned long VPN2 : 19;
			};
		};
	};

	struct Status_Reg
	{
		union
		{
			unsigned long l;
			
			struct
			{
				// bit 0 - Interrupt enable
				unsigned long IE : 1;
				
				// bit 1 - Exception level
				unsigned long EXL : 1;
				
				// bit 2 - Error level
				unsigned long ERL : 1;
				
				// bits 3-4 - Operation mode - 0: Kernel mode, 1: Supervisor mode, 2: User mode - r/w - start value:
				unsigned long KSU : 2;
				
				// bits 5-9 - zero
				unsigned long zero0 : 5;
				
				// bits 10-11,15 - Interrupt mask - 0: disables interrupts, 1: enables interrupts - r/w
				unsigned long IM : 2;

				// bit 12 - Bus error mask - 0: signals a bus error, 1: masks a bus error - r/w
				unsigned long BEM : 1;
				
				// bits 13-14 - zero
				unsigned long zero1 : 2;
				
				// bit 15 - Interrupt mask (con't)
				unsigned long IM7 : 1;
				
				// bit 16 - Enable IE - 0: disables all interrupts regardless of IE, 1: enables IE
				unsigned long EIE : 1;
				
				// bit 17 - "EDI" - EI/DI instruction enable - 0: enabled only in kernel mode, 1: enabled in all modes - r/w
				unsigned long EnableDisableI : 1;
				
				// bit 18 - "CH" - Status of the most recent cache instruction for the data cache - 0: miss, 1: hit - r/w
				unsigned long CacheHistory : 1;
				
				// bits 19-21 - zero
				unsigned long zero2 : 3;
				
				// bit 22 - Controls address of the TLB Refill or general exception vectors - r/w
				// start value: 1
				unsigned long BEV : 1;
				
				// bit 23 - Controls address of performance counter and the debug exception vectors - r/w
				unsigned long DEV : 1;
				
				// bits 24-27 - zero
				unsigned long zero3 : 4;
				
				// bits 28-31 - Coprocessor Usable - 0: Unusable, 1: Usable - r/w
				unsigned long CU : 4;
			};
		};
	};

	struct Cause_Reg
	{
		union
		{
			unsigned long l;
			
			struct
			{
				// bits 0-1 - zero
				unsigned long zero0 : 2;
				
				// bits 2-6 - Exception code - 0: Interrupt, 1: TLB Modified, 2: TLB Refill, 3: TLB Refill (store), 4: Address Error, 5: Address Error (store)
				// 6: Bus Error (instruction), 7: Bus Error (data), 8: System Call, 9: Breakpoint, 10: Reserved instruction, 11: Coprocessor Unusable
				// 12: Overflow, 13: Trap
				unsigned long ExcCode : 5;
				
				// bits 7-9 - zero
				unsigned long zero1 : 3;
				
				// bit 10 - Set when Int[1] interrupt is pending
				unsigned long IP2 : 1;
				
				// bit 11 - Set when Int[0] interrupt is pending
				unsigned long IP3 : 1;
				
				// bits 12-14 - zero
				unsigned long zero2 : 3;
				
				// bit 15 - Set when a timer interrupt is pending
				unsigned long IP7 : 1;
				
				// bits 16-18 - Exception codes for level 2 exceptions - 0: Reset, 1: NMI, 2: Performance Counter, 3: Debug
				unsigned long EXC2 : 3;
				
				// bits 19-27 - zero
				unsigned long zero3 : 9;
				
				// bits 28-29 - Coprocessor number when a coprocessor unusable exception is taken
				unsigned long CE : 2;
				
				// bit 30 - Set when level 2 exception occurs in a branch delay slot
				unsigned long BD2 : 1;
				
				// bit 31 - Set when a level 1 exception occurs in a branch delay slot
				unsigned long BD : 1;
			};
		};
	};

	struct PRId_Reg
	{
		union
		{
			unsigned long l;
			
			struct
			{
				// bits 0-7 - Revision number - read only
				// start value: revision number -> set to zero
				unsigned long Rev : 8;
				
				// bits 8-15 - Implementation number - read only
				// start value: 0x2e
				unsigned long Imp : 8;
			};
		};
	};

	struct Config_Reg
	{
		union
		{
			unsigned long l;
			
			struct
			{
				// bits 0-2 - kseg0 cache mode - 0: cached w/o write-back or -allocate, 2: Uncached, 3: Cached, 7: Uncached accelerated
				// start value: undefined
				unsigned long K0 : 3;
				
				// bits 3-5 - zero
				unsigned long zero0 : 3;
				
				// bits 6-8 - Data cache size - read only - 0: 4KB, 1: 8KB, 2: 16KB
				// start value: 1
				unsigned long DC : 3;
				
				// bits 9-11 - Instruction cache size - read only - 0: 4KB, 1: 8KB, 2: 16KB
				// start value: 2
				unsigned long IC : 3;
				
				// bit 12 - Setting this bit to 1 enables branch prediction
				// start value: 0
				unsigned long BPE : 1;
				
				// bit 13 - Setting this bit to 1 enables non-blocking load
				// start value: 0
				unsigned long NBE : 1;
				
				// bits 14-15 - zero
				unsigned long zero1 : 2;
				
				// bit 16 - Setting this bit to 1 enables the data cache
				// start value: 0
				unsigned long DCE : 1;
				
				// bit 17 - Setting this bit to 1 enables the instruction cache
				// start value: 0
				unsigned long ICE : 1;
				
				// bit 18 - Setting this bit to 1 enables the pipeline parallel issue
				// start value: 0
				unsigned long DIE : 1;
				
				// bits 19-27 - zero
				unsigned long zero2 : 9;
				
				// bits 28-30 - Bus clock ratio - 0: processor clock frequency divided by 2
				// start value: 0
				unsigned long EC : 3;
			};
		};
	};

	struct CPR0_Regs
	{
	
		union
		{

			// there are 32 COP0 control registers
			unsigned long Regs [ 32 ];
			
			struct
			{
			
				// Register #0 - "Index"
				// Desc: Index that specifies TLB entry for reading or writing
				// Index - bits 0-5 - r/w
				// start value: undefined
				unsigned long Index;
				
				// Register #1 - "Random" - Read Only
				// Desc: Index that specifies TLB entry for the TLBWR instruction
				// Purpose: MMU
				// Random - bits 0-5 - read only
				// start value: undefined
				unsigned long Random;
				
				// Register #2 - "EntryLo0" - Lower part of the TLB entry
				EntryLo0_Reg EntryLo0;
				
				// Register #3 - "EntryLo1" - Lower part of the TLB entry
				EntryLo1_Reg EntryLo1;
				
				// Register #4 - "Context" - TLB miss handling information
				Context_Reg Context;
				
				// Register #5 - "PageMask" - Page size comparison mask
				PageMask_Reg PageMask;
				
				// Register #6 - "Wired" - The number of wired TLB entries
				// bits 0-5 - Wired
				unsigned long Wired;
				
				// Register #7 - Reserved
				unsigned long Reserved0;
				
				// Register #8 - "BadVAddr" - Virtual address that causes an error - bits 0-31
				unsigned long BadVAddr;
				
				// Register #9 - "Count" - Timer count value - incremented every clock cycle - r/w
				unsigned long Count;
				
				// Register #10 - "EntryHi" - upper parts of a TLB entry
				EntryHi_Reg EntryHi;
				
				// Register #11 - "Compare" - Timer stable value - when the "Count" register reaches this value an interrupt occurs - r/w
				unsigned long Compare;
				
				// Register #12 - "Status" - COP0 Status
				Status_Reg Status;
				
				// Register #13 - "Cause" - Cause of the most recent exception
				Cause_Reg Cause;
				
				// Register #14 - "EPC" - address that is to resume after an exception has been serviced
				unsigned long EPC;
				
				// Register #15 - "PRId" - Processor revision
				PRId_Reg PRId;

				// Register #16 - "Config" - Processor configuration
				Config_Reg Config;
				
				// Registers #17-#22 - Reserved
				unsigned long Reserved1;
				unsigned long Reserved2;
				unsigned long Reserved3;
				unsigned long Reserved4;
				unsigned long Reserved5;
				unsigned long Reserved6;
				
				// Register #23 - "BadPAddr" - Physical address that caused an error - lower 4 bits are always zero
				unsigned long BadPAddr;
			};
			
		};

	};


	class Cpu
	{

	public:

		// this does not have to be saved with system state
		volatile long object_locked;

		bool debug_enabled;
		
		static const long InstructionSize = 4;

		// A pointer to the debug window
		WindowClass::Window* DebugWindow;

		x64Encoder* x;
		R5900Encoder* r;
		
		// assuming branch uses 10 half cycles
		static const unsigned long BranchHalfCycles = 10;
		
		// assuming a normal instruction uses 1 half cycle
		static const unsigned long NormalHalfCycles = 1;

		Mips128GPR ALIGN16 GPR [ 32 ]; // must have 16 byte alignment

		Mips128GPR ALIGN16 Hi;	// Hi0 corresponds to the normal "Hi" register
		Mips128GPR ALIGN16 Lo;	// Lo0 corresponds to the normal "Lo" register

		FloatLong FPR [ 32 ];	// these registers can be accessed through move (MFC1/MTC1) and load (LWC1/SWC1) instructions
		FloatLong ACC;
		
		// Implementation and Revision Register (FCR[0]) (Read-only)
		// bits 16-31: fixed at zero
		// bits 8-15: Implementation Number - start value: 0x2e
		// bits 0-7: Revision Number - fix at zero
		// ---------------------------------
		// Control/Status Rgister (FCR[31]) 
		// bits 0-2: fix at binary 001
		// bits 3-6: Sticky Flags - initial value: all zero
		// bit 3 - Sticky Underflow flag
		// bit 4 - Sticky Overflow flag
		// bit 5 - Sticky Divide flag
		// bit 6 - Sticky Invalid flag
		// bits 7-13: fix at zero
		// bits 14-17: Cause flags - initial value: all zero
		// bit 14 - Underflow flag
		// bit 15 - Overflow flag
		// bit 16 - Divide flag
		// bit 17 - Invalid flag
		// bits 18-22: fix at zero
		// bit 23 - Condition bit - set when result of floating point compare operation is true and clear when it is false
		// bit 24 - Fix at 1
		// bits 25-31 - Fix at zero
		long FCR [ 32 ];	// COP1 control registers - FCR[0] is the implementation/revision register, and FCR[31] is the control/status register

		// PCCR/PCR0/PCR1 are all mapped to CPR0[25] and are accessed with special instructions: MFPC rt,0/1 MFPS rt,0 MTPC rt,0/1 MTPS rt,0
		// Performance Counter Control Register (PCCR) - controls the function of the performance counter
		// bit 0 - fix at zero
		// bit 1 - EXL0 - set to enable PCR0 counting in level 1 exception handler
		// bit 2 - K0 - set to enable PCR0 counting in kernel mode (except in exception handler )
		// bit 3 - S0 - set to enable PCR0 counting in supervisor mode
		// bit 4 - U0 - set to enable PCR0 counting in user mode
		// bits 5-9: Event counted by PCR0 - this depends on counter number (PCR0/PCR1) as well as this value
		// bit 10 - fix at zero
		// bits 11-15: same as bits 1-9, but for PCR1
		// bit 31 - Couter enable - enables counting and exception generation for PCR0 and PCR1 - initial value: zero
		// -----------------------------------------
		// Counter Registers (PCR0/PCR1) - each can independently count one event
		// bits 0-30: Counter value
		// bit 31 - Counter Overflow flag
		long PCCR, PCR [ 2 ];

	//	long CPR0 [ 32 ];	// COP0 control registers
		CPR0_Regs CPR0;
		long CPR2 [ 32 ];	// COP2 control registers

		long SA;		// shift amount register for 128-bit shifts

		unsigned long PC;	// program counter
		volatile unsigned long DecodePC;	// address of instruction currently being decoded by recompiler

		// making this a long so recompiler knows that its 32-bits
		// bit 0 is set when instruction following a branch is being taken in branch delay slot
		volatile long isInBranchDelaySlot;
		volatile long isBranchTaken;
		unsigned long NextPC;	// next value of program counter after executing a branch delay slot
		
		// we need to be able to save branch delay slot information in case there is a trap or some other crazy instruction in the branch delay slot
		// this is where it should be saved when an exception is taken
		volatile long isInBranchDelaySlotSave;
		volatile long isBranchTakenSave;
		volatile long NextPCSave;
		
		// used for testing braches
		volatile long BranchTest;

		// this says whether or not a trap is taken
		// bit 0 is set when a trap needs to be taken by current trap instruction, cleared otherwise
		volatile long isTrapTaken;
		volatile long NextTrapPC;	// the vector address to use for the trap

		// this can pass the target cycles without hurting accuracy since anything over is just extra cycles
		volatile long CycleCount;	// will count the cycles
		long CycleTarget;	// this is the target number of cycles for the processor to execute
		
		unsigned long long frequency;
		
		volatile long Stop;

		// processor must keep track of whether or not it is in a branch delay slot dynamically
		volatile long BranchDelaySlot;

		volatile long LoadDelaySlot;

		// constructor
		Cpu ( void );
		
		// destructor
		~Cpu ( void );
		
		// create
		bool Create ( void );

		// reset processor
		void Reset ( void );

		void Run ( long NumberOfCycles );

		// this should only be used for testing
		void Step ( long NumberOfInstructions );

		// opens debug window and prepares for writing
		void EnableDebugging ( void );

		// can only do this after enabling debugging
		void UpdateDebugWindow ( void );

		// signals an interrupt to occur at the next possible moment
		void SignalInterrupt ( long Interrupt );

	#define V	((char*) PS2SystemState.MainMemory)
	#define W	((char*) PS2SystemState.Bios)

		static char* PS2MemoryMapLUT [ 1024 ];
		static const long PS2MemoryMapMask [ 1024 ];
		
		// as much as the upper 20 bits of the address can be used to specify what device is being referenced by an address
		unsigned long long TranslationAddressTable [ 1048576 ];
		unsigned long TranslationMaskTable [ 1048576 ];

	private:

		// determines if the next instruction has been decoded already
		bool isDecoded ( void );
		
		// decodes instructions until a branch is reached that is not the first instruction decoded or we reach a certain number of cycles decoded
		long DecodeCycles ( long NumberOfCyclesToDecode );
		
		// this is for stepping through instructions
		long DecodeInstruction ( void );
		
		// executes instructions until CycleTarget is reached
		void Execute ( void );

		
		void InstructionFooter ( long NumberOfCyclesDecoded, long instruction, long PreviousInstruction );
		void CodeBlockFooter ( long NumberOfCyclesDecoded );

	};

}

#endif



