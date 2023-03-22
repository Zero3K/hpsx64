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


///////////////////////////////////////////////////////////////////////////////
// *** TODO *** Fix LWL/LWR/SWL/SWR to use masked loading/storing
// *** TODO *** make sure COP2 is ready before processing SWC2/LWC2


#include "R3000A_Execute.h"
#include "R3000A_Lookup.h"

#include "R3000ADebugPrint.h"

#include <iostream>
#include <iomanip>



// if disabling store buffer, then must comment it out in all these files:
// R3000A.h
// R3000A.cpp
// R3000A_Execute.cpp
//#define ENABLE_STORE_BUFFER

// enable penalty for refilling the pipeline after a branch
#define ENABLE_R3000A_BRANCH_PENALTY


// enabling these shows the last read and write address in the debug window
#define ENABLE_DEBUG_LOAD
#define ENABLE_DEBUG_STORE



#ifdef _DEBUG_VERSION_

// enable debug
#define INLINE_DEBUG_ENABLE

/*
#define INLINE_DEBUG_MULT
#define INLINE_DEBUG_MULTU
#define INLINE_DEBUG_DIV
#define INLINE_DEBUG_DIVU
#define INLINE_DEBUG_MTHI
#define INLINE_DEBUG_MTLO
#define INLINE_DEBUG_MFHI
#define INLINE_DEBUG_MFLO
*/


//#define INLINE_DEBUG_SB
//#define INLINE_DEBUG_SH
//#define INLINE_DEBUG_SW
//#define INLINE_DEBUG_SWL
//#define INLINE_DEBUG_LWL
//#define INLINE_DEBUG_SWR
//#define INLINE_DEBUG_LWR
//#define INLINE_DEBUG_SYSCALL
//#define INLINE_DEBUG_RFE
//#define INLINE_DEBUG_R3000A
#define INLINE_DEBUG_BREAK
#define INLINE_DEBUG_INVALID
#define INLINE_DEBUG_LOAD_UNALIGNED
#define INLINE_DEBUG_STORE_UNALIGNED


//#define INLINE_DEBUG_LWC2
//#define INLINE_DEBUG_SWC2
//#define INLINE_DEBUG_MTC2
//#define INLINE_DEBUG_MFC2
//#define INLINE_DEBUG_CTC2
//#define INLINE_DEBUG_CFC2
//#define INLINE_DEBUG_COP2



//#define COUT_USERMODE_LOAD
//#define COUT_USERMODE_STORE
//#define COUT_FC
//#define COUT_SWC

#define INLINE_DEBUG_TRACE

#endif


using namespace std;

// this area deals with the execution of instructions on the R3000A
using namespace R3000A;
using namespace R3000A::Instruction;

// static vars //
Cpu *Execute::r;


// if a register is loading after load-delay slot, then we need to stall the pipeline until it finishes loading from memory
// if register is loading but is stored to, then we should clear loading flag for register and cancel load
//#define CHECK_LOADING(arg)	/*if ( r->GPRLoading_Bitmap & ( arg ) ) { r->BusyUntil_Cycle = r->Bus->BusyUntil_Cycle; return false; }*/

//#define CHECK_LOADING_COP2(arg)	/*if ( r->COP2.CPRLoading_Bitmap & ( arg ) ) { r->BusyUntil_Cycle = r->Bus->BusyUntil_Cycle; return false; }*/

// we can also cancel the loading of a register if it gets written to before load is complete
//#define CANCEL_LOADING(arg) ( r->GPRLoading_Bitmap &= ~( arg ) )

// if modified register is in delay slot, then kill the load from delay slot
#define CHECK_DELAYSLOT(ModifiedRegister) r->LastModifiedRegister = ModifiedRegister;

#define TRACE_VALUE(ValTr) r->TraceValue = ValTr;

//#define PROCESS_LOADDELAY_BEFORELOAD
#define PROCESS_LOADDELAY_BEFORESTORE

////////////////////////////////////////////////
// R-Type Instructions (non-interrupt)


// regular arithemetic
void Execute::ADDU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDU || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// add without overflow exception: rd = rs + rt
	r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u + r->GPR [ i.Rt ].u;
	
	// if the register that gets modified is in load delay slot, then cancel load
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_ADDU || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SUBU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBU || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// subtract without overflow exception: rd = rs - rt
	r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u - r->GPR [ i.Rt ].u;
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_SUBU || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::AND ( Instruction::Format i )
{
#if defined INLINE_DEBUG_AND || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// logical AND: rd = rs & rt
	r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u & r->GPR [ i.Rt ].u;
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_AND || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::OR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_OR || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// logical OR: rd = rs | rt
	r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u | r->GPR [ i.Rt ].u;
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_OR || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::XOR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_XOR || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// logical XOR: rd = rs ^ rt
	r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u ^ r->GPR [ i.Rt ].u;
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_XOR || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::NOR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_NOR || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// logical NOR: rd = ~(rs | rt)
	r->GPR [ i.Rd ].u = ~( r->GPR [ i.Rs ].u | r->GPR [ i.Rt ].u );
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_NOR || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SLT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SLT || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// set less than signed: rd = rs < rt ? 1 : 0
	r->GPR [ i.Rd ].s = r->GPR [ i.Rs ].s < r->GPR [ i.Rt ].s ? 1 : 0;
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_SLT || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SLTU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SLTU || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// set less than signed: rd = rs < rt ? 1 : 0
	r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u < r->GPR [ i.Rt ].u ? 1 : 0;
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_SLTU || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}


////////////////////////////////////////////
// I-Type Instructions (non-interrupt)



void Execute::ADDIU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDIU || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif

	// *note* immediate value is sign-extended before the addition is performed

	// add immedeate without overflow exception: rt = rs + immediate
	r->GPR [ i.Rt ].s = r->GPR [ i.Rs ].s + i.sImmediate;
	
	CHECK_DELAYSLOT ( i.Rt );
	
#if defined INLINE_DEBUG_ADDIU || defined INLINE_DEBUG_R3000A
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::ANDI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ANDI || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// Logical AND zero-extended immedeate: rt = rs & immediate
	r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u & i.uImmediate;
	
	CHECK_DELAYSLOT ( i.Rt );
	
#if defined INLINE_DEBUG_ANDI || defined INLINE_DEBUG_R3000A
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::ORI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ORI || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// Logical OR zero-extended immedeate: rt = rs | immediate
	r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u | i.uImmediate;
	
	CHECK_DELAYSLOT ( i.Rt );
	
#if defined INLINE_DEBUG_ORI || defined INLINE_DEBUG_R3000A
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::XORI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_XORI || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// Logical XOR zero-extended immedeate: rt = rs & immediate
	r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u ^ i.uImmediate;
	
	CHECK_DELAYSLOT ( i.Rt );
	
#if defined INLINE_DEBUG_XORI || defined INLINE_DEBUG_R3000A
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::SLTI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SLTI || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// Set less than sign-extended immedeate: rt = rs < immediate ? 1 : 0
	r->GPR [ i.Rt ].s = r->GPR [ i.Rs ].s < i.sImmediate ? 1 : 0;
	
	CHECK_DELAYSLOT ( i.Rt );
	
#if defined INLINE_DEBUG_SLTI || defined INLINE_DEBUG_R3000A
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::SLTIU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SLTIU || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// *note* Rs is still sign-extended here, but the comparison is an unsigned one

	// Set if unsigned less than sign-extended immedeate: rt = rs < immediate ? 1 : 0
	r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u < ((u32) ((s32) i.sImmediate)) ? 1 : 0;
	
	CHECK_DELAYSLOT ( i.Rt );
	
#if defined INLINE_DEBUG_SLTIU || defined INLINE_DEBUG_R3000A
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::LUI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LUI || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif
	
	// Load upper immediate
	r->GPR [ i.Rt ].u = ( i.uImmediate << 16 );
	
	CHECK_DELAYSLOT ( i.Rt );
	
#if defined INLINE_DEBUG_LUI || defined INLINE_DEBUG_R3000A
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}





void Execute::MFHI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MFHI || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: HI = " << r->HiLo.uHi;
#endif
	
	// this instruction interlocks if multiply/divide unit is busy
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += r->MulDiv_BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// determine when multiply/divide unit is busy until
		r->BusyUntil_Cycle = r->MulDiv_BusyUntil_Cycle;
		
		// wait until the multiply/divide unit is no longer busy
		r->WaitForCpuReady1 ();
		*/
	}
	
	// move from Hi register
	r->GPR [ i.Rd ].u = r->HiLo.uHi;
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_MFHI || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::MFLO ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MFLO || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: LO = " << r->HiLo.uLo;
#endif
	
	// this instruction interlocks if multiply/divide unit is busy
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += r->MulDiv_BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// determine when multiply/divide unit is busy until
		r->BusyUntil_Cycle = r->MulDiv_BusyUntil_Cycle;
		
		// wait until the multiply/divide unit is no longer busy
		r->WaitForCpuReady1 ();
		*/
	}
	
	// move from Lo register
	r->GPR [ i.Rd ].u = r->HiLo.uLo;
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_MFLO || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}




void Execute::MTHI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MTHI || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// ***TODO*** should this sync with mul/div unit??
	
	// there is no MTHI/MTLO delay slot
	r->HiLo.uHi = r->GPR [ i.Rs ].u;
}

void Execute::MTLO ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MTLO || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// ***TODO*** should this sync with mul/div unit??
	
	// there is no MTHI/MTLO delay slot
	r->HiLo.uLo = r->GPR [ i.Rs ].u;
}


//////////////////////////////////////////////////////////
// Shift instructions



void Execute::SLL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SLL || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// shift left logical: rd = rt << shift
	r->GPR [ i.Rd ].u = ( r->GPR [ i.Rt ].u << i.Shift );
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_SLL || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SRL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SRL || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// shift right logical: rd = rt >> shift
	r->GPR [ i.Rd ].u = ( r->GPR [ i.Rt ].u >> i.Shift );
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_SRL || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SRA ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SRA || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// shift right arithmetic: rd = rt >> shift
	r->GPR [ i.Rd ].s = ( r->GPR [ i.Rt ].s >> i.Shift );
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_SRA || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SLLV ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SLLV || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// shift left logical variable: rd = rt << rs
	r->GPR [ i.Rd ].u = ( r->GPR [ i.Rt ].u << ( r->GPR [ i.Rs ].u & 0x1f ) );
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_SLLV || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SRLV ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SRLV || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// shift right logical variable: rd = rt >> rs
	r->GPR [ i.Rd ].u = ( r->GPR [ i.Rt ].u >> ( r->GPR [ i.Rs ].u & 0x1f ) );
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_SRLV || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SRAV ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SRAV || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// shift right arithmetic variable: rd = rt >> rs
	r->GPR [ i.Rd ].s = ( r->GPR [ i.Rt ].s >> ( r->GPR [ i.Rs ].u & 0x1f ) );
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_SRAV || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}


/////////////////////////////////////////////////////////////
// Multiply/Divide Instructions


void Execute::MULT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MULT || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += r->MulDiv_BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// determine when multiply/divide unit is busy until
		r->BusyUntil_Cycle = r->MulDiv_BusyUntil_Cycle;
		
		// wait until the multiply/divide unit is no longer busy
		r->WaitForCpuReady1 ();
		*/
	}

	// calculate cycles mul/div unit will be busy for
	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
	if ( r->GPR [ i.Rs ].s < 0x800 && r->GPR [ i.Rs ].s >= -0x800 )
	{
		r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
	}
	else if ( r->GPR [ i.Rs ].s < 0x100000 && r->GPR [ i.Rs ].s >= -0x100000 )
	{
		r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
	}
	
	// multiply signed Lo,Hi = rs * rt
	r->HiLo.sValue = ((s64) (r->GPR [ i.Rs ].s)) * ((s64) (r->GPR [ i.Rt ].s));

#if defined INLINE_DEBUG_MULT || defined INLINE_DEBUG_R3000A
	debug << "; Output: LO = " << r->HiLo.uLo << "; HI = " << r->HiLo.uHi;
#endif
}

void Execute::MULTU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MULTU || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// if rs is between 0 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += r->MulDiv_BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// determine when multiply/divide unit is busy until
		r->BusyUntil_Cycle = r->MulDiv_BusyUntil_Cycle;
		
		// wait until the multiply/divide unit is no longer busy
		r->WaitForCpuReady1 ();
		*/
	}
	
	// calculate cycles mul/div unit will be busy for
	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
	if ( r->GPR [ i.Rs ].u < 0x800 )
	{
		r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
	}
	else if ( r->GPR [ i.Rs ].u < 0x100000 )
	{
		r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
	}

	// multiply unsigned Lo,Hi = rs * rt
	r->HiLo.uValue = ((u64) (r->GPR [ i.Rs ].u)) * ((u64) (r->GPR [ i.Rt ].u));
	
#if defined INLINE_DEBUG_MULTU || defined INLINE_DEBUG_R3000A
	debug << "; Output: LO = " << r->HiLo.uLo << "; HI = " << r->HiLo.uHi;
#endif
}

void Execute::DIV ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DIV || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// 36 cycles
	static const int c_iDivideCycles = 36;

	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += r->MulDiv_BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// determine when multiply/divide unit is busy until
		r->BusyUntil_Cycle = r->MulDiv_BusyUntil_Cycle;
		
		// wait until the multiply/divide unit is no longer busy
		r->WaitForCpuReady1 ();
		*/
	}
	
	// mult/div unit is busy now
	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iDivideCycles;

	// divide signed: Lo = rs / rt; Hi = rs % rt
	if ( r->GPR [ i.Rt ].u != 0 )
	{
		// if rs = 0x80000000 and rt = -1 then hi = 0 and lo = 0x80000000
		r->HiLo.sLo = r->GPR [ i.Rs ].s / r->GPR [ i.Rt ].s;
		r->HiLo.sHi = r->GPR [ i.Rs ].s % r->GPR [ i.Rt ].s;
	}
	else
	{
		/*
		if ( r->GPR [ i.Rs ].s < 0 )
		{
			r->HiLo.sLo = 1;
		}
		else
		{
			r->HiLo.sLo = -1;
		}
		*/

		r->HiLo.sLo = (~(r->GPR[i.Rs].s >> 31)) | 1;
		
		r->HiLo.uHi = r->GPR [ i.Rs ].u;
	}
	
#if defined INLINE_DEBUG_DIV || defined INLINE_DEBUG_R3000A
	debug << "; Output: LO = " << r->HiLo.uLo << "; HI = " << r->HiLo.uHi;
#endif
}

void Execute::DIVU ( Instruction::Format i )
{
	// 36 cycles
	static const int c_iDivideCycles = 36;
	
#if defined INLINE_DEBUG_DIVU || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += r->MulDiv_BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// determine when multiply/divide unit is busy until
		r->BusyUntil_Cycle = r->MulDiv_BusyUntil_Cycle;
		
		// wait until the multiply/divide unit is no longer busy
		r->WaitForCpuReady1 ();
		*/
	}
	
	// mult/div unit is busy now
	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iDivideCycles;

	// divide unsigned: Lo = rs / rt; Hi = rs % rt
	if ( r->GPR [ i.Rt ].u != 0 )
	{
		r->HiLo.uLo = r->GPR [ i.Rs ].u / r->GPR [ i.Rt ].u;
		r->HiLo.uHi = r->GPR [ i.Rs ].u % r->GPR [ i.Rt ].u;
	}
	else
	{
		r->HiLo.sLo = -1;
		r->HiLo.uHi = r->GPR [ i.Rs ].u;
	}
	
#if defined INLINE_DEBUG_DIVU || defined INLINE_DEBUG_R3000A
	debug << "; Output: LO = " << r->HiLo.uLo << "; HI = " << r->HiLo.uHi;
#endif
}



////////////////////////////////////////////
// Jump/Branch Instructions



void Execute::J ( Instruction::Format i )
{
#if defined INLINE_DEBUG_J || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif
	
	// next instruction is in the branch delay slot
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.cb = Cpu::ProcessBranchDelaySlot_t<OPJ>;
	//r->Status.DelaySlot_Valid |= 0x1;
	
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->cb = Cpu::ProcessBranchDelaySlot_t<OPJ>;
	r->Status.DelaySlot_Valid |= 2;
}

void Execute::JR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_JR || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// generates an address exception if two low order bits of Rs are not zero


	// check for address exception??
	if ( r->GPR [ i.Rs ].u & 3 )
	{
		// if lower 2-bits of register are not zero, fire address exception
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
		return;
	}
	
	/*
	// next instruction is in the branch delay slot
	r->DelaySlot0.Instruction = i;
	r->DelaySlot0.cb = Cpu::ProcessBranchDelaySlot_t<OPJR>;

	// *** todo *** check if address exception should be generated if lower 2-bits of jump address are not zero
	// will clear out lower two bits of address for now
	r->DelaySlot0.Data = r->GPR [ i.Rs ].u & ~3;
	
	r->Status.DelaySlot_Valid |= 0x1;
	*/
	
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->cb = Cpu::ProcessBranchDelaySlot_t<OPJR>;
	
	// no need for he and-not 3, since if the lower bits are set it interrupts
	//d->Data = r->GPR [ i.Rs ].u & ~3;
	d->Data = r->GPR [ i.Rs ].u;
	
	r->Status.DelaySlot_Valid |= 2;
}

void Execute::JAL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_JAL || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	
	// next instruction is in the branch delay slot
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.cb = Cpu::ProcessBranchDelaySlot_t<OPJAL>;
	//r->Status.DelaySlot_Valid |= 0x1;

	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->cb = Cpu::ProcessBranchDelaySlot_t<OPJAL>;
	r->Status.DelaySlot_Valid |= 2;
	
	// *** note *** this is tricky because return address gets stored to r31 after execution of load delay slot but before next instruction
	///////////////////////////////////////////////////////////////
	// Store return address when instruction is executed in r31
	r->GPR [ 31 ].u = r->PC + 8;
	
	CHECK_DELAYSLOT ( 31 );
	
#if defined INLINE_DEBUG_JAL || defined INLINE_DEBUG_R3000A
	debug << "; Output: r31 = " << r->GPR [ 31 ].u;
#endif
}

void Execute::JALR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_JALR || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// JALR rd, rs


	// check for address exception??
	if ( r->GPR [ i.Rs ].u & 3 )
	{
		// if lower 2-bits of register are not zero, fire address exception
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
		
		return;
	}
	
	/*
	// next instruction is in the branch delay slot
	r->DelaySlot0.Instruction = i;
	
	// *** todo *** check if address exception should be generated if lower 2-bits of jump address are not zero
	// will clear out lower two bits of address for now
	r->DelaySlot0.Data = r->GPR [ i.Rs ].u & ~3;
	r->DelaySlot0.cb = Cpu::ProcessBranchDelaySlot_t<OPJALR>;
	
	r->Status.DelaySlot_Valid |= 0x1;
	*/

	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->cb = Cpu::ProcessBranchDelaySlot_t<OPJALR>;
	
	// no need for the and-not 3, because if the lower bits are set it interrupts
	//d->Data = r->GPR [ i.Rs ].u & ~3;
	d->Data = r->GPR [ i.Rs ].u;
	
	r->Status.DelaySlot_Valid |= 2;
	
	///////////////////////////////////////////////////////////////
	// Store return address when instruction is executed in Rd
	// *note* this must happen AFTER the stuff above
	r->GPR [ i.Rd ].u = r->PC + 8;
	
	CHECK_DELAYSLOT ( i.Rd );
	
#if defined INLINE_DEBUG_JALR || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::BEQ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BEQ || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	if ( r->GPR [ i.Rs ].u == r->GPR [ i.Rt ].u )
	{
		// next instruction is in the branch delay slot
		//r->DelaySlot0.Instruction = i;
		//r->DelaySlot0.cb = Cpu::ProcessBranchDelaySlot_t<OPBEQ>;
		//r->Status.DelaySlot_Valid |= 0x1;
		
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		d->cb = Cpu::ProcessBranchDelaySlot_t<OPBEQ>;
		r->Status.DelaySlot_Valid |= 2;
		
#if defined INLINE_DEBUG_BEQ || defined INLINE_DEBUG_R3000A
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BNE ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BNE || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	if ( r->GPR [ i.Rs ].u != r->GPR [ i.Rt ].u )
	{
		// next instruction is in the branch delay slot
		//r->DelaySlot0.Instruction = i;
		//r->DelaySlot0.cb = Cpu::ProcessBranchDelaySlot_t<OPBNE>;
		//r->Status.DelaySlot_Valid |= 0x1;
		
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		d->cb = Cpu::ProcessBranchDelaySlot_t<OPBNE>;
		r->Status.DelaySlot_Valid |= 2;
		
#if defined INLINE_DEBUG_BNE || defined INLINE_DEBUG_R3000A
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BLEZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BLEZ || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	if ( r->GPR [ i.Rs ].s <= 0 )
	{
		// next instruction is in the branch delay slot
		//r->DelaySlot0.Instruction = i;
		//r->DelaySlot0.cb = Cpu::ProcessBranchDelaySlot_t<OPBLEZ>;
		//r->Status.DelaySlot_Valid |= 0x1;
		
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		d->cb = Cpu::ProcessBranchDelaySlot_t<OPBLEZ>;
		r->Status.DelaySlot_Valid |= 2;
		
#if defined INLINE_DEBUG_BLEZ || defined INLINE_DEBUG_R3000A
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BGTZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BGTZ || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	if ( r->GPR [ i.Rs ].s > 0 )
	{
		// next instruction is in the branch delay slot
		//r->DelaySlot0.Instruction = i;
		//r->DelaySlot0.cb = Cpu::ProcessBranchDelaySlot_t<OPBGTZ>;
		//r->Status.DelaySlot_Valid |= 0x1;
		
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		d->cb = Cpu::ProcessBranchDelaySlot_t<OPBGTZ>;
		r->Status.DelaySlot_Valid |= 2;
		
#if defined INLINE_DEBUG_BGTZ || defined INLINE_DEBUG_R3000A
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BLTZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BLTZ || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	if ( r->GPR [ i.Rs ].s < 0 )
	{
		// next instruction is in the branch delay slot
		//r->DelaySlot0.Instruction = i;
		//r->DelaySlot0.cb = Cpu::ProcessBranchDelaySlot_t<OPBLTZ>;
		//r->Status.DelaySlot_Valid |= 0x1;
		
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		d->cb = Cpu::ProcessBranchDelaySlot_t<OPBLTZ>;
		r->Status.DelaySlot_Valid |= 2;
		
#if defined INLINE_DEBUG_BLTZ || defined INLINE_DEBUG_R3000A
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BGEZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BGEZ || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	if ( r->GPR [ i.Rs ].s >= 0 )
	{
		// next instruction is in the branch delay slot
		//r->DelaySlot0.Instruction = i;
		//r->DelaySlot0.cb = Cpu::ProcessBranchDelaySlot_t<OPBGEZ>;
		//r->Status.DelaySlot_Valid |= 0x1;
		
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		d->cb = Cpu::ProcessBranchDelaySlot_t<OPBGEZ>;
		r->Status.DelaySlot_Valid |= 2;
		
#if defined INLINE_DEBUG_BGEZ || defined INLINE_DEBUG_R3000A
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BLTZAL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BLTZAL || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	if ( r->GPR [ i.Rs ].s < 0 )
	{
		// next instruction is in the branch delay slot
		//r->DelaySlot0.Instruction = i;
		//r->DelaySlot0.cb = Cpu::ProcessBranchDelaySlot_t<OPBLTZAL>;
		//r->Status.DelaySlot_Valid |= 0x1;
		
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		d->cb = Cpu::ProcessBranchDelaySlot_t<OPBLTZAL>;
		r->Status.DelaySlot_Valid |= 2;
		
#if defined INLINE_DEBUG_BLTZAL || defined INLINE_DEBUG_R3000A
		debug << ";  WILL TAKE";
#endif
	}
	
	////////////////////////////////////////////////////////////////////////
	// Store return address when instruction is executed in r31
	// for this instruction this happens whether branch is taken or not
	// *note* this must happen AFTER comparison check
	r->GPR [ 31 ].u = r->PC + 8;
	
	CHECK_DELAYSLOT ( 31 );
}

void Execute::BGEZAL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BGEZAL || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	if ( r->GPR [ i.Rs ].s >= 0 )
	{
		// next instruction is in the branch delay slot
		//r->DelaySlot0.Instruction = i;
		//r->DelaySlot0.cb = Cpu::ProcessBranchDelaySlot_t<OPBGEZAL>;
		//r->Status.DelaySlot_Valid |= 0x1;
		
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		d->cb = Cpu::ProcessBranchDelaySlot_t<OPBGEZAL>;
		r->Status.DelaySlot_Valid |= 2;
		
#if defined INLINE_DEBUG_BGEZAL || defined INLINE_DEBUG_R3000A
		debug << ";  WILL TAKE";
#endif
	}

	////////////////////////////////////////////////////////////////////////
	// Store return address when instruction is executed in r31
	// for this instruction this happens whether branch is taken or not
	// *note* this must happen AFTER comparison check
	r->GPR [ 31 ].u = r->PC + 8;
	
	CHECK_DELAYSLOT ( 31 );
}




void Execute::RFE ( Instruction::Format i )
{
#if defined INLINE_DEBUG_RFE || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "\r\nReturning From: ";
	if ( r->CPR0.Cause.ExcCode == 0 ) debug << "ASYNC Interrupt"; else if ( r->CPR0.Cause.ExcCode == 8 ) debug << "Syscall"; else debug << "Other";
	debug << "\r\nReturning To: " << hex << setw ( 8 ) << r->GPR [ 26 ].u << "; EPC=" << r->CPR0.EPC;
	debug << "\r\nUpdateInterrupt;(before) _Intc_Stat=" << hex << *r->_Intc_Stat << " _Intc_Mask=" << *r->_Intc_Mask << " _R3000A_Status=" << r->CPR0.Regs [ 12 ] << " _R3000A_Cause=" << r->CPR0.Regs [ 13 ] << " _ProcStatus=" << r->Status.Value;
#endif
	
	// restore user/kernel status register bits
	// bits 7-8 should stay zero, and bits 5-6 should stay the same
	r->CPR0.Status.b0 = ( r->CPR0.Status.b0 & 0x30 ) | ( ( r->CPR0.Status.b0 >> 2 ) & 0xf );
	
	// check if interrupts should be re-enabled or cleared ?
	
	// whenever interrupt related stuff is messed with, must update the other interrupt stuff
	r->UpdateInterrupt ();

#if defined INLINE_DEBUG_RFE || defined INLINE_DEBUG_R3000A
	debug << "\r\n(after) _Intc_Stat=" << hex << *r->_Intc_Stat << " _Intc_Mask=" << *r->_Intc_Mask << " _R3000A_Status=" << r->CPR0.Regs [ 12 ] << " _R3000A_Cause=" << r->CPR0.Regs [ 13 ] << " _ProcStatus=" << r->Status.Value << " CycleCount=" << dec << r->CycleCount;
#endif
}




////////////////////////////////////////////////////////
// Instructions that can cause Synchronous Interrupts //
////////////////////////////////////////////////////////


void Execute::ADD ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ADD || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	s32 temp;
	
	//temp = ( (s64) r->GPR [ i.Rs ].s ) + ( (s64) r->GPR [ i.Rt ].s );
	temp = r->GPR [ i.Rs ].s + r->GPR [ i.Rt ].s;
	
	// if the carry outs of bits 30 and 31 differ, then it's signed overflow
	//if ( ( temp < -2147483648LL ) || ( temp > 2147483647LL ) )
	//if( (INT32)( ~( m_r[ INS_RS( m_op ) ] ^ m_r[ INS_RT( m_op ) ] ) & ( m_r[ INS_RS( m_op ) ] ^ result ) ) < 0 )
	//if ( (s32)( ~( r->GPR [ i.Rs ].s ^ r->GPR [ i.Rt ].s ) & ( r->GPR [ i.Rs ].s ^ temp ) ) < 0 )
	//if ( temp < -0x80000000LL || temp > 0x7fffffffLL )
	if ( ( ( ~( r->GPR [ i.Rs ].s ^ r->GPR [ i.Rt ].s ) ) & ( r->GPR [ i.Rs ].s ^ temp ) ) < 0 )
	{
		// overflow
		cout << "\nhps1x64: Execute::ADD generated an overflow exception @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << "\n";
		r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
		
#if defined INLINE_DEBUG_ADD || defined INLINE_DEBUG_R3000A
		debug << ";  INT";
#endif
	}
	else
	{
		// it's cool - we can do the add and store the result to register
		r->GPR [ i.Rd ].s = temp;
		
		CHECK_DELAYSLOT ( i.Rd );
	}
	
#if defined INLINE_DEBUG_ADD || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::ADDI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDI || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif

	s32 temp;
	
	//temp = ( (s64) r->GPR [ i.Rs ].s ) + ( (s64) i.sImmediate );
	temp = r->GPR [ i.Rs ].s + ( (s32) i.sImmediate );
	
	// if the carry outs of bits 30 and 31 differ, then it's signed overflow
	//if ( ( temp < -2147483648LL ) || ( temp > 2147483647LL ) )
	//if( (INT32)( ~( m_r[ INS_RS( m_op ) ] ^ m_r[ INS_RT( m_op ) ] ) & ( m_r[ INS_RS( m_op ) ] ^ result ) ) < 0 )
	//if ( (s32)( ~( r->GPR [ i.Rs ].s ^ ( (s32) i.sImmediate ) ) & ( r->GPR [ i.Rs ].s ^ temp ) ) < 0 )
	//if ( temp < -0x80000000LL || temp > 0x7fffffffLL )
	if ( ( ( ~( r->GPR [ i.Rs ].s ^ ( (s32) i.sImmediate ) ) ) & ( r->GPR [ i.Rs ].s ^ temp ) ) < 0 )
	{
		// overflow
		cout << "\nhps1x64: Execute::ADDI generated an overflow exception @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << "\n";
		r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
		
#if defined INLINE_DEBUG_ADDI || defined INLINE_DEBUG_R3000A
		debug << ";  INT";
#endif
	}
	else
	{
		// it's cool - we can do the addi and store the result to register
		r->GPR [ i.Rt ].s = temp;
		
		CHECK_DELAYSLOT ( i.Rt );
	}
	
#if defined INLINE_DEBUG_ADDI || defined INLINE_DEBUG_R3000A
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::SUB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SUB || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	s64 temp;
	
	temp = ( (s64) r->GPR [ i.Rs ].s ) - ( (s64) r->GPR [ i.Rt ].s );
	
	// if the carry outs of bits 30 and 31 differ, then it's signed overflow
	//if ( ( temp < -2147483648LL ) || ( temp > 2147483647LL ) )
	//if( (INT32)( ( m_r[ INS_RS( m_op ) ] ^ m_r[ INS_RT( m_op ) ] ) & ( m_r[ INS_RS( m_op ) ] ^ result ) ) < 0 )
	//if ( (s32) ( ( r->GPR [ i.Rs ].s ^ r->GPR [ i.Rt ].s ) & ( r->GPR [ i.Rs ].s ^ temp ) ) < 0 )
	if ( temp < -0x80000000LL || temp > 0x7fffffffLL )
	{
		// overflow
		cout << "\nhps1x64: Execute::SUB generated an overflow exception @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << "\n";
		r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
		
#if defined INLINE_DEBUG_SUB || defined INLINE_DEBUG_R3000A
		debug << ";  INT";
#endif
	}
	else
	{
		// it's cool - we can do the sub and store the result to register
		r->GPR [ i.Rd ].s = temp;
		
		CHECK_DELAYSLOT ( i.Rd );
	}
	
#if defined INLINE_DEBUG_SUB || defined INLINE_DEBUG_R3000A
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}




void Execute::SYSCALL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SYSCALL || defined INLINE_DEBUG_R3000A
	debug << "\r\nBefore:" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "\r\n" << hex << "Status=" << r->CPR0.Regs [ 12 ] << " Cause=" << r->CPR0.Regs [ 13 ] << " a0=" << r->GPR [ 4 ].u;
#endif
	
	r->ProcessSynchronousInterrupt ( Cpu::EXC_SYSCALL );
	
#ifdef ENABLE_R3000A_BRANCH_PENALTY
	r->CycleCount += r->c_ullLatency_PipelineRefill;
#endif

#if defined INLINE_DEBUG_SYSCALL || defined INLINE_DEBUG_R3000A
	debug << "\r\nAfter:" << hex << setw( 8 ) << r->PC << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "\r\n" << hex << "Status=" << r->CPR0.Regs [ 12 ] << " Cause=" << r->CPR0.Regs [ 13 ] << " a0=" << r->GPR [ 4 ].u;
#endif
}

void Execute::BREAK ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BREAK || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif
	
	cout << "\nhps1x64: Execute::BREAK generated an exception @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << "\n";
	r->ProcessSynchronousInterrupt ( Cpu::EXC_BP );
	
#ifdef ENABLE_R3000A_BRANCH_PENALTY
	r->CycleCount += r->c_ullLatency_PipelineRefill;
#endif

	// say to stop if we are debugging
	Cpu::DebugStatus.Stop = true;
	Cpu::DebugStatus.Done = true;
}

void Execute::Invalid ( Instruction::Format i )
{
#if defined INLINE_DEBUG_INVALID || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << " INVALID R3000A INSTRUCTION";
#endif

	cout << "\nhps1x64 NOTE: Invalid Instruction @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << " Instruction=" << i.Value << " LastPC=" << r->LastPC << "\n";
	r->ProcessSynchronousInterrupt ( Cpu::EXC_RI );
}





void Execute::MFC0 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MFC0 || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: CPR[0,rd] = " << r->CPR0.Regs [ i.Rd ];
#endif

	// MFCz rt, rd
	// 1 instruction delay?
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = r->CPR0.Regs [ i.Rd ];
	//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPMFC0,RSMFC0>;
	//r->Status.DelaySlot_Valid |= 0x1;
	
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->Data = r->CPR0.Regs [ i.Rd ];
	d->cb = Cpu::ProcessLoadDelaySlot_t<OPMFC0,RSMFC0>;
	r->Status.DelaySlot_Valid |= 2;
		
	// important note: if this regiter is modified in delay slot, then it IS cancelled
	r->LastModifiedRegister = 255;
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->DelaySlot0.Data )
#endif
}

/*
void Execute::FC_Callback ( Cpu* r )
{
	if ( r->DelaySlot1.Instruction.Rt == r->LastModifiedRegister )
	{
#ifdef COUT_FC
		cout << "\nhps1x64 ALERT: Reg#" << dec << r->DelaySlot1.Instruction.Rt << " was modified in MFC/CFC delay slot @ Cycle#" << r->CycleCount << hex << " PC=" << r->PC << "\n";
#endif

	}
	
	r->GPR [ r->DelaySlot1.Instruction.Rt ].u = r->DelaySlot1.Data;
	
	// clear delay slot
	r->DelaySlot1.Value = 0;
}
*/


void Execute::MTC0 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MTC0 || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// MTCz rt, rd
	// 1 instruction delay?
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = r->GPR [ i.Rt ].u;
	//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPMTC0,RSMTC0>;
	//r->Status.DelaySlot_Valid |= 0x1;

	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->Data = r->GPR [ i.Rt ].u;
	d->cb = Cpu::ProcessLoadDelaySlot_t<OPMTC0,RSMTC0>;
	r->Status.DelaySlot_Valid |= 2;
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->DelaySlot0.Data )
#endif
}


/*
void Execute::MTC0_Callback ( Cpu* r )
{
	r->Write_MTC0 ( r->DelaySlot1.Instruction.Rd, r->DelaySlot1.Data );
	
	// clear delay slot
	r->DelaySlot1.Value = 0;
}
*/


void Execute::MFC2 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MFC2 || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: CPR[2,rd] = " << r->COP2.CPR2.Regs [ i.Rd ];
#endif

	/////////////////////////////////////////////////////
	// If COP2 is busy then we need to stall pipeline
	if ( r->CycleCount < r->COP2.BusyUntil_Cycle )
	{
		// COP2 is busy //
		
		/*
		// determine what cycle CPU has to wait until before it can continue
		r->BusyUntil_Cycle = r->COP2.BusyUntil_Cycle;
		
		// wait until COP2/CPU is ready
		r->WaitForCpuReady1 ();
		*/
		
		// for now, just set cycle count
		r->CycleCount = r->COP2.BusyUntil_Cycle;
	}
	
	// MFCz rt, rd
	// 1 instruction delay
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = r->COP2.Read_MFC ( i.Rd );
	//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPMFC2,RSMFC2>;
	//r->Status.DelaySlot_Valid |= 0x1;
	
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->Data = r->COP2.Read_MFC ( i.Rd );
	d->cb = Cpu::ProcessLoadDelaySlot_t<OPMFC2,RSMFC2>;
	r->Status.DelaySlot_Valid |= 2;
	
	// important note: if this regiter is modified in delay slot, then it IS cancelled
	r->LastModifiedRegister = 255;
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->DelaySlot0.Data )
#endif
}




void Execute::MTC2 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MTC2 || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	/////////////////////////////////////////////////////
	// If COP2 is busy then we need to stall pipeline
	if ( r->CycleCount < r->COP2.BusyUntil_Cycle )
	{
		// COP2 is busy //
		
		/*
		// determine what cycle CPU has to wait until before it can continue
		r->BusyUntil_Cycle = r->COP2.BusyUntil_Cycle;
		
		// wait until COP2/CPU is ready
		r->WaitForCpuReady1 ();
		*/
		
		// for now, just set cycle count
		r->CycleCount = r->COP2.BusyUntil_Cycle;
	}
	
	// MTCz rt, rd
	// 1 instruction delay
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = r->GPR [ i.Rt ].u;
	//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPMTC2,RSMTC2>;
	//r->Status.DelaySlot_Valid |= 0x1;
	
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->Data = r->GPR [ i.Rt ].u;
	d->cb = Cpu::ProcessLoadDelaySlot_t<OPMTC2,RSMTC2>;
	r->Status.DelaySlot_Valid |= 2;
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->DelaySlot0.Data )
#endif
}


/*
void Execute::MTC2_Callback ( Cpu* r )
{
	r->COP2.Write_MTC ( r->DelaySlot1.Instruction.Rd, r->DelaySlot1.Data );
	
	// clear delay slot
	r->DelaySlot1.Value = 0;
}
*/


void Execute::CFC2 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CFC2 || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: CPC[2,rd] = " << r->COP2.CPC2.Regs [ i.Rd ];
#endif
	
	/////////////////////////////////////////////////////
	// If COP2 is busy then we need to stall pipeline
	if ( r->CycleCount < r->COP2.BusyUntil_Cycle )
	{
		// COP2 is busy //
		
		/*
		// determine what cycle CPU has to wait until before it can continue
		r->BusyUntil_Cycle = r->COP2.BusyUntil_Cycle;
		
		// wait until COP2/CPU is ready
		r->WaitForCpuReady1 ();
		*/
		
		// for now, just set cycle count
		r->CycleCount = r->COP2.BusyUntil_Cycle;
	}
	
	// CFCz rt, rd
	// 1 instruction delay
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = r->COP2.Read_CFC ( i.Rd );
	//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPCFC2,RSCFC2>;
	//r->Status.DelaySlot_Valid |= 0x1;
	
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->Data = r->COP2.Read_CFC ( i.Rd );
	d->cb = Cpu::ProcessLoadDelaySlot_t<OPCFC2,RSCFC2>;
	r->Status.DelaySlot_Valid |= 2;
	
	// important note: if this regiter is modified in delay slot, then it IS cancelled
	r->LastModifiedRegister = 255;
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].u )
#endif
}



void Execute::CTC2 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CTC2 || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	/////////////////////////////////////////////////////
	// If COP2 is busy then we need to stall pipeline
	if ( r->CycleCount < r->COP2.BusyUntil_Cycle )
	{
		// COP2 is busy //
		
		/*
		// determine what cycle CPU has to wait until before it can continue
		r->BusyUntil_Cycle = r->COP2.BusyUntil_Cycle;
		
		// wait until COP2/CPU is ready
		r->WaitForCpuReady1 ();
		*/
		
		// for now, just set cycle count
		r->CycleCount = r->COP2.BusyUntil_Cycle;
	}
	
	// CTCz rt, rd
	// 1 instruction delay
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = r->GPR [ i.Rt ].u;
	//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPCTC2,RSCTC2>;
	//r->Status.DelaySlot_Valid |= 0x1;
	
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->Data = r->GPR [ i.Rt ].u;
	d->cb = Cpu::ProcessLoadDelaySlot_t<OPCTC2,RSCTC2>;
	r->Status.DelaySlot_Valid |= 2;
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].u )
#endif
}


/*
void Execute::CTC2_Callback ( Cpu* r )
{
	r->COP2.Write_CTC ( r->DelaySlot1.Instruction.Rd, r->DelaySlot1.Data );
	
	// clear delay slot
	r->DelaySlot1.Value = 0;
}
*/


// Load/Store - will need to use address translation to get physical addresses when needed

//////////////////////////////////////////////////////////////////////////
// store instructions

// store instructions
void Execute::SB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SB || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif
	
	// SB rt, offset(base)

#ifdef PROCESS_LOADDELAY_BEFORESTORE
	// *note* load delay slot runs before actual value is put in line to be stored
	r->ProcessLoadDelaySlot ();
#endif
	
	// step 1: check if storing to data cache
	u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	
	// clear top 3 bits since there is no data cache for caching stores
	StoreAddress &= 0x1fffffff;
	
	if ( r->CPR0.Status.IsC )	// *todo* - I think we need to check if DCache is isolated too - unsure
	{
#if defined INLINE_DEBUG_SB || defined INLINE_DEBUG_R3000A
		debug << "; IsC";
#endif

		//cout << "\nhps1x64 ALERT: IsC -> Cache is isolated.\n";
		// *** todo *** also this only clears cache line if this writes byte or half word - or maybe all stores invalidate??
		// important note: appears that this clears the instruction cache regardless of whether cache is swapped or not
		r->ICache.InvalidateDirect ( StoreAddress );
		
		// invalidate memory for recompiler
		//r->Bus->InvalidArray.b8 [ StoreAddress >> ( 2 + DataBus::c_iInvalidate_Shift ) ] = 1;
	}
	else
	{
#ifdef COUT_USERMODE_STORE
		// check if in user mode
		if ( !r->CPR0.Status.KUc )
		{
			// make sure that top 3 bits of address are clear
			if ( StoreAddress & 0xe0000000 )
			{
				cout << "\nhps1x64 ALERT: Invalid store for USER mode. SB. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
			}
		}
#endif
	
		if ( r->isDCache ( StoreAddress ) )
		{
#if defined INLINE_DEBUG_SB || defined INLINE_DEBUG_R3000A
			debug << "; D$";
#endif

			// step 3: if storing to data cache and lines are not reversed, then store to data cache
			r->DCache.b8 [ StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ] = (u8) r->GPR [ i.Rt ].u;
		}
		else
		{
#ifdef ENABLE_STORE_BUFFER
			if ( ( StoreAddress & 0x1d000000 ) == 0x1d000000 )
			{
			
			// step 4: otherwise, add entry into store buffer. if store buffer is full, then stall pipeline
			if ( r->StoreBuffer.isFullStore() )
			{
#if defined INLINE_DEBUG_SB || defined INLINE_DEBUG_R3000A
				debug << "; STALL->SBUF FULL";
#endif

				// store buffer is full //
				
				// flush store buffer
				r->FlushStoreBuffer ();
				
				// store buffer was full, so add another 3 cycles onto that (special case or something)
				//r->Bus->ReserveBus ( 3 );
				//Cpu::MemoryLatency += Cpu::c_CycleTime_StoreBuffer_Full;
				
				/*
				// determine how long before stores finish
				r->BusyUntil_Cycle = r->Bus->BusyUntil_Cycle;
				
				// wait for stores to finish
				r->WaitForCpuReady1 ();
				*/
			}
			
#if defined INLINE_DEBUG_SB || defined INLINE_DEBUG_R3000A
			debug << "; TO SBUF";
#endif

			// add entry into store buffer
			//r->StoreBuffer.Add_Store ( i, r->_cb_SB );
			r->StoreBuffer.Add_Store ( i, r->ProcessExternalStore_t<OPSB> );
			
			}
			else
			{
#endif

				//r->ProcessExternalStore_t<OPSB> ( r->GPR [ i.Rt ].u, StoreAddress );
				//r->ProcessExternalStore ( i, r->GPR [ i.Rt ].u, StoreAddress );
				DataBus::Write_t<0xff> ( r->GPR [ i.Rt ].u, StoreAddress );
				r->CycleCount += Cpu::c_CycleTime_Store;
				
#ifdef ENABLE_STORE_BUFFER
			}
#endif
			
		}
	}
	
#ifdef ENABLE_DEBUG_STORE
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].u )
#endif
}





void Execute::SH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SH || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif
	
	// SH rt, offset(base)
	
#ifdef PROCESS_LOADDELAY_BEFORESTORE
	// * note * load delay slot runs before actual value is put in line to be stored
	r->ProcessLoadDelaySlot ();
#endif
	
	// step 1: check if storing to data cache
	u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;

	// *** testing *** alert if load is from unaligned address
	if ( StoreAddress & 0x1 )
	{
#if defined INLINE_DEBUG_STORE_UNALIGNED
		debug << "\r\nhps1x64 ALERT: StoreAddress is unaligned for SH @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\r\n";
#endif

		cout << "\nhps1x64 ALERT: StoreAddress is unaligned for SH @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADES );
		return;
	}
	
	// clear top 3 bits since there is no data cache for caching stores
	StoreAddress &= 0x1fffffff;
	
	
	if ( r->CPR0.Status.IsC )	// *todo* - I think we need to check if DCache is isolated too - unsure
	{
#if defined INLINE_DEBUG_SH || defined INLINE_DEBUG_R3000A
		debug << "; IsC";
#endif

		//cout << "\nhps1x64 ALERT: IsC -> Cache is isolated.\n";
		// *** todo *** also this only clears cache line if this writes byte or half word - or maybe all stores invalidate??
		r->ICache.InvalidateDirect ( StoreAddress );
		
		// invalidate memory for recompiler
		//r->Bus->InvalidArray.b8 [ StoreAddress >> ( 2 + DataBus::c_iInvalidate_Shift ) ] = 1;
	}
	else
	{
#ifdef COUT_USERMODE_STORE
		// check if in user mode
		if ( !r->CPR0.Status.KUc )
		{
			// make sure that top 3 bits of address are clear
			if ( StoreAddress & 0xe0000000 )
			{
				cout << "\nhps1x64 ALERT: Invalid store for USER mode. SH. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
			}
		}
#endif
	
		if ( r->isDCache ( StoreAddress ) )
		{
#if defined INLINE_DEBUG_SH || defined INLINE_DEBUG_R3000A
			debug << "; D$";
#endif

			// step 3: if storing to data cache and lines are not reversed, then store to data cache
			r->DCache.b16 [ ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 1 ] = (u16) r->GPR [ i.Rt ].u;
		}
		else
		{
#ifdef ENABLE_STORE_BUFFER
			if ( ( StoreAddress & 0x1d000000 ) == 0x1d000000 )
			{

			// step 4: otherwise, add entry into store buffer. if store buffer is full, then stall pipeline
			if ( r->StoreBuffer.isFullStore() )
			{
#if defined INLINE_DEBUG_SH || defined INLINE_DEBUG_R3000A
				debug << "; STALL->SBUF FULL";
#endif

				// store buffer is full //
				
				// flush store buffer
				r->FlushStoreBuffer ();
				
				// store buffer was full, so add another 3 cycles onto that (special case or something)
				//r->Bus->ReserveBus ( 3 );
				//Cpu::MemoryLatency += Cpu::c_CycleTime_StoreBuffer_Full;
				
				/*
				// determine how long before stores finish
				r->BusyUntil_Cycle = r->Bus->BusyUntil_Cycle;
				
				// wait for stores to finish
				r->WaitForCpuReady1 ();
				*/
			}
			
#if defined INLINE_DEBUG_SH || defined INLINE_DEBUG_R3000A
			debug << "; TO SBUF";
#endif

			// add entry into store buffer
			//r->StoreBuffer.Add_Store ( i, r->_cb_SH );
			r->StoreBuffer.Add_Store ( i, r->ProcessExternalStore_t<OPSH> );

			}
			else
			{
#endif

				//r->ProcessExternalStore_t<OPSH> ( r->GPR [ i.Rt ].u, StoreAddress );
				//r->ProcessExternalStore ( i, r->GPR [ i.Rt ].u, StoreAddress );
				DataBus::Write_t<0xffff> ( r->GPR [ i.Rt ].u, StoreAddress );
				r->CycleCount += Cpu::c_CycleTime_Store;
				
#ifdef ENABLE_STORE_BUFFER
			}
#endif

		}
	}

#ifdef ENABLE_DEBUG_STORE
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].u )
#endif
}

void Execute::SW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SW || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif
	
	// SW rt, offset(base)
	u32 StoreAddress;
	
#ifdef PROCESS_LOADDELAY_BEFORESTORE
	// * note * load delay slot runs before actual value is put in line to be stored
	r->ProcessLoadDelaySlot ();
#endif
	
	// check if storing to data cache
	StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	
	// *** testing *** alert if load is from unaligned address
	if ( StoreAddress & 0x3 )
	{
#if defined INLINE_DEBUG_STORE_UNALIGNED
		debug << "\r\nhps1x64 ALERT: StoreAddress is unaligned for SW @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\r\n";
#endif

		cout << "\nhps1x64 ALERT: StoreAddress is unaligned for SW @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADES );
		return;
	}
	
	// clear top 3 bits since there is no data cache for caching stores
	StoreAddress &= 0x1fffffff;
	
	
	if ( r->CPR0.Status.IsC )	// *todo* - I think we need to check if DCache is isolated too - unsure
	{
#if defined INLINE_DEBUG_SW || defined INLINE_DEBUG_R3000A
		debug << "; IsC";
#endif

		//cout << "\nhps1x64 ALERT: IsC -> Cache is isolated.\n";
		// *** todo *** also this only clears cache line if this writes byte or half word - or maybe all stores invalidate??
		r->ICache.InvalidateDirect ( StoreAddress );
		
		// invalidate memory for recompiler
		//r->Bus->InvalidArray.b8 [ StoreAddress >> ( 2 + DataBus::c_iInvalidate_Shift ) ] = 1;
	}
	else
	{
#ifdef COUT_USERMODE_STORE
		// check if in user mode
		if ( !r->CPR0.Status.KUc )
		{
			// make sure that top 3 bits of address are clear
			if ( StoreAddress & 0xe0000000 )
			{
				cout << "\nhps1x64 ALERT: Invalid store for USER mode. SW. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
			}
		}
#endif
	
		if ( r->isDCache ( StoreAddress ) )
		{
#if defined INLINE_DEBUG_SW || defined INLINE_DEBUG_R3000A
			debug << "; D$";
#endif

			// step 3: if storing to data cache and lines are not reversed, then store to data cache
			r->DCache.b32 [ ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 2 ] = r->GPR [ i.Rt ].u;
		}
		else
		{
#ifdef ENABLE_STORE_BUFFER
			if ( ( StoreAddress & 0x1d000000 ) == 0x1d000000 )
			{

			// step 4: otherwise, add entry into store buffer. if store buffer is full, then stall pipeline
			if ( r->StoreBuffer.isFullStore() )
			{
#if defined INLINE_DEBUG_SW || defined INLINE_DEBUG_R3000A
				debug << "; STALL->SBUF FULL";
#endif

				// store buffer is full //
				
				// flush store buffer
				r->FlushStoreBuffer ();
				
				// store buffer was full, so add another 3 cycles onto that (special case or something)
				//r->Bus->ReserveBus ( 3 );
				//Cpu::MemoryLatency += Cpu::c_CycleTime_StoreBuffer_Full;
				
				/*
				// determine how long before stores finish
				r->BusyUntil_Cycle = r->Bus->BusyUntil_Cycle;
				
				// wait for stores to finish
				r->WaitForCpuReady1 ();
				*/
			}
			
#if defined INLINE_DEBUG_SW || defined INLINE_DEBUG_R3000A
			debug << "; TO SBUF";
#endif

			// add entry into store buffer
			//r->StoreBuffer.Add_Store ( i, r->_cb_SW );
			r->StoreBuffer.Add_Store ( i, r->ProcessExternalStore_t<OPSW> );
			}
			else
			{
#endif

				//r->ProcessExternalStore_t<OPSW> ( r->GPR [ i.Rt ].u, StoreAddress );
				//r->ProcessExternalStore ( i, r->GPR [ i.Rt ].u, StoreAddress );
				DataBus::Write_t<0xffffffff> ( r->GPR [ i.Rt ].u, StoreAddress );
				r->CycleCount += Cpu::c_CycleTime_Store;
				
#ifdef ENABLE_STORE_BUFFER
			}
#endif

		}
	}

#ifdef ENABLE_DEBUG_STORE
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].u )
#endif
}


void Execute::SWC2 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SWC2 || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: CPR[2,rt] = " << r->COP2.CPR2.Regs [ i.Rt ] << "; base = " << r->GPR [ i.Base ].u;
#endif
	
	// SWC2 rt, offset(base)
	
	// *** TODO *** WAIT FOR COP2 TO BE READY AND IDLE
	
#ifdef PROCESS_LOADDELAY_BEFORESTORE
	// * note * load delay slot runs before actual value is put in line to be stored
	r->ProcessLoadDelaySlot ();
#endif
	
	u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	
	// *** testing *** alert if load is from unaligned address
	if ( StoreAddress & 0x3 )
	{
#if defined INLINE_DEBUG_STORE_UNALIGNED
		debug << "\r\nhps1x64 ALERT: StoreAddress is unaligned for SWC2 @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\r\n";
#endif

		cout << "\nhps1x64 ALERT: StoreAddress is unaligned for SWC2 @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADES );
		return;
	}
	
	
	// *** TODO *** this is where it should be made sure that COP2 is ready and idle
	
	
	// clear top 3 bits since there is no data cache for caching stores
	StoreAddress &= 0x1fffffff;

	// step 2: if storing to data cache, check if I$ and D$ are reversed. If so, invalidate cache line
	// * note * - this is probably incorrect but will use as a placeholder
	if ( r->CPR0.Status.IsC )	// *todo* - I think we need to check if DCache is isolated too
	{
#if defined INLINE_DEBUG_SWC2 || defined INLINE_DEBUG_R3000A
	debug << " IsC";
#endif

		//cout << "\nhps1x64 ALERT: IsC -> Cache is isolated.\n";
		r->ICache.InvalidateDirect ( StoreAddress );
		
		// invalidate memory for recompiler
		//r->Bus->InvalidArray.b8 [ StoreAddress >> ( 2 + DataBus::c_iInvalidate_Shift ) ] = 1;
	}
	else
	{
#ifdef COUT_USERMODE_STORE
		// check if in user mode
		if ( !r->CPR0.Status.KUc )
		{
			// make sure that top 3 bits of address are clear
			if ( StoreAddress & 0xe0000000 )
			{
				cout << "\nhps1x64 ALERT: Invalid store for USER mode. SWC2. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
			}
		}
#endif
		
		// step 1: check if storing to data cache
		if ( r->isDCache ( StoreAddress ) )
		{
#if defined INLINE_DEBUG_SWC2 || defined INLINE_DEBUG_R3000A
	debug << " DCache";
#endif

			// step 3: if storing to data cache and lines are not reversed, then store to data cache
			r->DCache.b32 [ ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 2 ] = r->COP2.Read_MFC ( i.Rt ) /*r->COP2.CPR2.Regs [ i.Rt ]*/;
		}
		else
		{
#ifdef ENABLE_STORE_BUFFER
			if ( ( StoreAddress & 0x1d000000 ) == 0x1d000000 )
			{

			// step 4: otherwise, add entry into store buffer. if store buffer is full, then stall pipeline
			if ( r->StoreBuffer.isFullStore() )
			{
#if defined INLINE_DEBUG_SWC2 || defined INLINE_DEBUG_R3000A
			debug << "; STALL->SBUF FULL";
#endif

				// store buffer is full //
				
				// flush store buffer
				r->FlushStoreBuffer ();
				
				// store buffer was full, so add another 3 cycles onto that (special case or something)
				//r->Bus->ReserveBus ( 3 );
				//Cpu::MemoryLatency += Cpu::c_CycleTime_StoreBuffer_Full;
				
				/*
				// determine how long before stores finish
				r->BusyUntil_Cycle = r->Bus->BusyUntil_Cycle;
				
				// wait for stores to finish
				r->WaitForCpuReady1 ();
				*/
			}
			
#if defined INLINE_DEBUG_SWC2 || defined INLINE_DEBUG_R3000A
	debug << " STORE";
#endif


			// add entry into store buffer
			//r->StoreBuffer.Add_StoreFromCOP2 ( i, r->_cb_SW );
			r->StoreBuffer.Add_StoreFromCOP2 ( i, r->ProcessExternalStore_t<OPSW> );

			}
			else
			{
#endif

				//r->ProcessExternalStore_t<OPSW> ( r->COP2.Read_MFC ( i.Rt ), StoreAddress );
				//r->ProcessExternalStore ( i, r->COP2.Read_MFC ( i.Rt ), StoreAddress );
				DataBus::Write_t<0xffffffff> ( r->COP2.Read_MFC ( i.Rt ), StoreAddress );
				r->CycleCount += Cpu::c_CycleTime_Store;
				
#ifdef ENABLE_STORE_BUFFER
			}
#endif

		}
	}

#ifdef ENABLE_DEBUG_STORE
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->COP2.Read_MFC ( i.Rt ) )
#endif
}



void Execute::SWL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SWL || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif

	static const u32 c_Mask = 0xffffffff;
	u32 Type, Offset;

	// SWL rt, offset(base)
	
#ifdef PROCESS_LOADDELAY_BEFORESTORE
	// * note * load delay slot runs before actual value is put in line to be stored
	r->ProcessLoadDelaySlot ();
#endif
	
	// step 1: check if storing to data cache
	u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;

	// clear top 3 bits since there is no data cache for caching stores
	StoreAddress &= 0x1fffffff;
	
	if ( r->CPR0.Status.IsC )	// *todo* - I think we need to check if DCache is isolated too - unsure
	{
#if defined INLINE_DEBUG_SWL || defined INLINE_DEBUG_R3000A
		debug << "; IsC";
#endif

		//cout << "\nhps1x64 ALERT: IsC -> Cache is isolated.\n";
		// *** todo *** also this only clears cache line if this writes byte or half word - or maybe all stores invalidate??
		r->ICache.InvalidateDirect ( StoreAddress );
		
		// invalidate memory for recompiler
		//r->Bus->InvalidArray.b8 [ StoreAddress >> ( 2 + DataBus::c_iInvalidate_Shift ) ] = 1;
	}
	else
	{
#ifdef COUT_USERMODE_STORE
		// check if in user mode
		if ( !r->CPR0.Status.KUc )
		{
			// make sure that top 3 bits of address are clear
			if ( StoreAddress & 0xe0000000 )
			{
				cout << "\nhps1x64 ALERT: Invalid store for USER mode. SWL. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
			}
		}
#endif
	
		if ( r->isDCache ( StoreAddress ) )
		{
#if defined INLINE_DEBUG_SWL || defined INLINE_DEBUG_R3000A
			debug << "; D$";
#endif

			// step 3: if storing to data cache and lines are not reversed, then store to data cache
			// store left stores 4-offset number of bytes
			// shift register right by 3-type bytes, and mask is 0xffffffff shifted right by the same amount
			Type = 3 - ( StoreAddress & 3 );
			Offset = ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 2;
			r->DCache.b32 [ Offset ] = ( r->GPR [ i.Rt ].u >> ( Type << 3 ) ) | ( r->DCache.b32 [ Offset ] & ~( c_Mask >> ( Type << 3 ) ) );
		}
		else
		{
#ifdef ENABLE_STORE_BUFFER
			if ( ( StoreAddress & 0x1d000000 ) == 0x1d000000 )
			{

			// step 4: otherwise, add entry into store buffer. if store buffer is full, then stall pipeline
			if ( r->StoreBuffer.isFullStore() )
			{
#if defined INLINE_DEBUG_SWL || defined INLINE_DEBUG_R3000A
				debug << "; STALL->SBUF FULL";
#endif

				// store buffer is full //
				
				// flush store buffer
				r->FlushStoreBuffer ();
				
				// store buffer was full, so add another 3 cycles onto that (special case or something)
				//r->Bus->ReserveBus ( 3 );
				//Cpu::MemoryLatency += Cpu::c_CycleTime_StoreBuffer_Full;
				
				/*
				// determine how long before stores finish
				r->BusyUntil_Cycle = r->Bus->BusyUntil_Cycle;
				
				// wait for stores to finish
				r->WaitForCpuReady1 ();
				*/
			}
			
#if defined INLINE_DEBUG_SWL || defined INLINE_DEBUG_R3000A
			debug << "; TO SBUF";
#endif

			// add entry into store buffer
			//r->StoreBuffer.Add_Store ( i, r->_cb_SWL );
			r->StoreBuffer.Add_Store ( i, r->ProcessExternalStore_t<OPSWL> );
			}
			else
			{
#endif

				//r->ProcessExternalStore_t<OPSWL> ( r->GPR [ i.Rt ].u, StoreAddress );
				//r->ProcessExternalStore ( i, r->GPR [ i.Rt ].u, StoreAddress );
				DataBus::Write ( r->GPR [ i.Rt ].u >> ( ( 3 - ( StoreAddress & 3 ) ) << 3 ), StoreAddress & ~3, 0xffffffffUL >> ( ( 3 - ( StoreAddress & 3 ) ) << 3 ) );
				r->CycleCount += Cpu::c_CycleTime_Store;
				
#ifdef ENABLE_STORE_BUFFER
			}
#endif
		}
	}

#ifdef ENABLE_DEBUG_STORE
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].u )
#endif
}

void Execute::SWR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SWR || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif

	static const u32 c_Mask = 0xffffffff;
	u32 Type, Offset;
	
	// SWR rt, offset(base)
	
#ifdef PROCESS_LOADDELAY_BEFORESTORE
	// * note * load delay slot runs before actual value is put in line to be stored
	r->ProcessLoadDelaySlot ();
#endif
	
	// step 1: check if storing to data cache
	u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;

	// clear top 3 bits since there is no data cache for caching stores
	StoreAddress &= 0x1fffffff;
	
	if ( r->CPR0.Status.IsC )	// *todo* - I think we need to check if DCache is isolated too - unsure
	{
#if defined INLINE_DEBUG_SWR || defined INLINE_DEBUG_R3000A
		debug << "; IsC";
#endif

		//cout << "\nhps1x64 ALERT: IsC -> Cache is isolated.\n";
		// *** todo *** also this only clears cache line if this writes byte or half word - or maybe all stores invalidate??
		r->ICache.InvalidateDirect ( StoreAddress );
		
		// invalidate memory for recompiler
		//r->Bus->InvalidArray.b8 [ StoreAddress >> ( 2 + DataBus::c_iInvalidate_Shift ) ] = 1;
	}
	else
	{
#ifdef COUT_USERMODE_STORE
		// check if in user mode
		if ( !r->CPR0.Status.KUc )
		{
			// make sure that top 3 bits of address are clear
			if ( StoreAddress & 0xe0000000 )
			{
				cout << "\nhps1x64 ALERT: Invalid store for USER mode. SWR. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
			}
		}
#endif
	
		if ( r->isDCache ( StoreAddress ) )
		{
#if defined INLINE_DEBUG_SWR || defined INLINE_DEBUG_R3000A
			debug << "; D$";
#endif

			// step 3: if storing to data cache and lines are not reversed, then store to data cache
			// store right
			// mask and register are shifted left by type bytes
			Type = StoreAddress & 3;
			Offset = ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 2;
			r->DCache.b32 [ Offset ] = ( r->GPR [ i.Rt ].u << ( Type << 3 ) ) | ( r->DCache.b32 [ Offset ] & ~( c_Mask << ( Type << 3 ) ) );
		}
		else
		{
#ifdef ENABLE_STORE_BUFFER
			if ( ( StoreAddress & 0x1d000000 ) == 0x1d000000 )
			{

			// step 4: otherwise, add entry into store buffer. if store buffer is full, then stall pipeline
			if ( r->StoreBuffer.isFullStore() )
			{
#if defined INLINE_DEBUG_SWR || defined INLINE_DEBUG_R3000A
				debug << "; STALL->SBUF FULL";
#endif

				// store buffer is full //
				
				// flush store buffer
				r->FlushStoreBuffer ();
				
				// store buffer was full, so add another 3 cycles onto that (special case or something)
				//r->Bus->ReserveBus ( 3 );
				//Cpu::MemoryLatency += Cpu::c_CycleTime_StoreBuffer_Full;
				
				/*
				// determine how long before stores finish
				r->BusyUntil_Cycle = r->Bus->BusyUntil_Cycle;
				
				// wait for stores to finish
				r->WaitForCpuReady1 ();
				*/
			}
			
#if defined INLINE_DEBUG_SWR || defined INLINE_DEBUG_R3000A
			debug << "; TO SBUF";
#endif

			// add entry into store buffer
			//r->StoreBuffer.Add_Store ( i, r->_cb_SWR );
			r->StoreBuffer.Add_Store ( i, r->ProcessExternalStore_t<OPSWR> );
			}
			else
			{
#endif

				//r->ProcessExternalStore_t<OPSWR> ( r->GPR [ i.Rt ].u, StoreAddress );
				//r->ProcessExternalStore ( i, r->GPR [ i.Rt ].u, StoreAddress );
				DataBus::Write ( r->GPR [ i.Rt ].u << ( ( StoreAddress & 3 ) << 3 ), StoreAddress & ~3, 0xffffffffUL << ( ( StoreAddress & 3 ) << 3 ) );
				r->CycleCount += Cpu::c_CycleTime_Store;
				
#ifdef ENABLE_STORE_BUFFER
			}
#endif
		}
	}
	
#ifdef ENABLE_DEBUG_STORE
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].u )
#endif
}



/////////////////////////////////////////////////
// load instructions

// load instructions with delay slot
void Execute::LB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LB || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// signed load byte from memory
	// LB rt, offset(base)
	
	u32 LoadAddress;
	
#ifdef PROCESS_LOADDELAY_BEFORELOAD
	// ***testing*** load delay slot probably runs before actual value is put in line to be loaded
	r->ProcessLoadDelaySlot ();
#endif
	
	// set load to happen after delay slot
	LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	
#ifdef COUT_USERMODE_LOAD
	// check if in user mode
	if ( !r->CPR0.Status.KUc )
	{
		// make sure that top 3 bits of address are clear
		if ( LoadAddress & 0xe0000000 )
		{
			cout << "\nhps1x64 ALERT: Invalid store for USER mode. LB. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		}
	}
#endif
	
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = LoadAddress;
	//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPLB,0>;
	//r->Status.DelaySlot_Valid |= 0x1;
	
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->Data = LoadAddress;
	d->cb = Cpu::ProcessLoadDelaySlot_t<OPLB,0>;
	r->Status.DelaySlot_Valid |= 2;
	
	// clear the last modified register so we can see what register was modified in load delay slot
	r->LastModifiedRegister = 255;
	
#ifdef ENABLE_DEBUG_LOAD
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
#endif
}






void Execute::LH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LH || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// LH rt, offset(base)
	
	u32 LoadAddress;
	
#ifdef PROCESS_LOADDELAY_BEFORELOAD
	// ***testing*** load delay slot probably runs before actual value is put in line to be loaded
	r->ProcessLoadDelaySlot ();
#endif
	
	// set load to happen after delay slot
	LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	
#ifdef COUT_USERMODE_LOAD
	// check if in user mode
	if ( !r->CPR0.Status.KUc )
	{
		// make sure that top 3 bits of address are clear
		if ( LoadAddress & 0xe0000000 )
		{
			cout << "\nhps1x64 ALERT: Invalid store for USER mode. LH. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		}
	}
#endif
	
	// *** testing *** alert if load is from unaligned address
	if ( LoadAddress & 0x1 )
	{
#if defined INLINE_DEBUG_LOAD_UNALIGNED
		debug << "\r\nhps1x64 ALERT: LoadAddress is unaligned for LH @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\r\n";
#endif

		cout << "\nhps1x64 ALERT: LoadAddress is unaligned for LH @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
	}
	else
	{
		//r->DelaySlot0.Instruction = i;
		//r->DelaySlot0.Data = LoadAddress;
		//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPLH,0>;
		//r->Status.DelaySlot_Valid |= 0x1;
		
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		d->Data = LoadAddress;
		d->cb = Cpu::ProcessLoadDelaySlot_t<OPLH,0>;
		r->Status.DelaySlot_Valid |= 2;
		
		// clear the last modified register so we can see what register was modified in load delay slot
		r->LastModifiedRegister = 255;
		
#ifdef ENABLE_DEBUG_LOAD
		// used for debugging
		r->Last_ReadAddress = LoadAddress;
		r->Last_ReadWriteAddress = LoadAddress;
#endif
	}
}








void Execute::LW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LW || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// LW rt, offset(base)
	
	u32 LoadAddress;
	
#ifdef PROCESS_LOADDELAY_BEFORELOAD
	// ***testing*** load delay slot probably runs before actual value is put in line to be loaded
	r->ProcessLoadDelaySlot ();
#endif
	
	// set load to happen after delay slot
	LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	
#ifdef COUT_USERMODE_LOAD
	// check if in user mode
	if ( !r->CPR0.Status.KUc )
	{
		// make sure that top 3 bits of address are clear
		if ( LoadAddress & 0xe0000000 )
		{
			cout << "\nhps1x64 ALERT: Invalid store for USER mode. LW. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		}
	}
#endif
	
	// *** testing *** alert if load is from unaligned address
	if ( LoadAddress & 0x3 )
	{
#if defined INLINE_DEBUG_LOAD_UNALIGNED
		debug << "\r\nhps1x64 ALERT: LoadAddress is unaligned for LW @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\r\n";
#endif

		cout << "\nhps1x64 ALERT: LoadAddress is unaligned for LW @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
	}
	else
	{
		//r->DelaySlot0.Instruction = i;
		//r->DelaySlot0.Data = LoadAddress;
		//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPLW,0>;
		//r->Status.DelaySlot_Valid |= 0x1;
		
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		d->Data = LoadAddress;
		d->cb = Cpu::ProcessLoadDelaySlot_t<OPLW,0>;
		r->Status.DelaySlot_Valid |= 2;
		
		// clear the last modified register so we can see what register was modified in load delay slot
		r->LastModifiedRegister = 255;
		
#ifdef ENABLE_DEBUG_LOAD
		// used for debugging
		r->Last_ReadAddress = LoadAddress;
		r->Last_ReadWriteAddress = LoadAddress;
#endif
	}
}

void Execute::LBU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LBU || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// LBU rt, offset(base)
	
	u32 LoadAddress;
	
#ifdef PROCESS_LOADDELAY_BEFORELOAD
	// ***testing*** load delay slot probably runs before actual value is put in line to be loaded
	r->ProcessLoadDelaySlot ();
#endif
	
	// set load to happen after delay slot
	LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	
#ifdef COUT_USERMODE_LOAD
	// check if in user mode
	if ( !r->CPR0.Status.KUc )
	{
		// make sure that top 3 bits of address are clear
		if ( LoadAddress & 0xe0000000 )
		{
			cout << "\nhps1x64 ALERT: Invalid store for USER mode. LBU. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		}
	}
#endif
	
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = LoadAddress;
	//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPLBU,0>;
	//r->Status.DelaySlot_Valid |= 0x1;
	
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->Data = LoadAddress;
	d->cb = Cpu::ProcessLoadDelaySlot_t<OPLBU,0>;
	r->Status.DelaySlot_Valid |= 2;
	
	// clear the last modified register so we can see what register was modified in load delay slot
	r->LastModifiedRegister = 255;
	
#ifdef ENABLE_DEBUG_LOAD
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
#endif
}

void Execute::LHU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LHU || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// LHU rt, offset(base)
	
	u32 LoadAddress;
	
#ifdef PROCESS_LOADDELAY_BEFORELOAD
	// ***testing*** load delay slot probably runs before actual value is put in line to be loaded
	r->ProcessLoadDelaySlot ();
#endif
	
	// set load to happen after delay slot
	LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	
#ifdef COUT_USERMODE_LOAD
	// check if in user mode
	if ( !r->CPR0.Status.KUc )
	{
		// make sure that top 3 bits of address are clear
		if ( LoadAddress & 0xe0000000 )
		{
			cout << "\nhps1x64 ALERT: Invalid store for USER mode. LHU. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		}
	}
#endif
	
	// *** testing *** alert if load is from unaligned address
	if ( LoadAddress & 0x1 )
	{
#if defined INLINE_DEBUG_LOAD_UNALIGNED
		debug << "\r\nhps1x64 ALERT: LoadAddress is unaligned for LHU @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\r\n";
#endif

		cout << "\nhps1x64 ALERT: LoadAddress is unaligned for LHU @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
	}
	else
	{
		//r->DelaySlot0.Instruction = i;
		//r->DelaySlot0.Data = LoadAddress;
		//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPLHU,0>;
		//r->Status.DelaySlot_Valid |= 0x1;
		
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		d->Data = LoadAddress;
		d->cb = Cpu::ProcessLoadDelaySlot_t<OPLHU,0>;
		r->Status.DelaySlot_Valid |= 2;
		
		// clear the last modified register so we can see what register was modified in load delay slot
		r->LastModifiedRegister = 255;
		
#ifdef ENABLE_DEBUG_LOAD
		// used for debugging
		r->Last_ReadAddress = LoadAddress;
		r->Last_ReadWriteAddress = LoadAddress;
#endif
	}
}

void Execute::LWC2 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LWC2 || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// LWC2 rt, offset(base)
	
	// *** TODO *** WAIT FOR COP2 TO BE READY AND IDLE
	
	u32 LoadAddress;
	
#ifdef PROCESS_LOADDELAY_BEFORELOAD
	// ***testing*** load delay slot probably runs before actual value is put in line to be loaded
	r->ProcessLoadDelaySlot ();
#endif
	
	// add entry into load delay slot
	LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	
#ifdef COUT_USERMODE_LOAD
	// check if in user mode
	if ( !r->CPR0.Status.KUc )
	{
		// make sure that top 3 bits of address are clear
		if ( LoadAddress & 0xe0000000 )
		{
			cout << "\nhps1x64 ALERT: Invalid store for USER mode. LWC2. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		}
	}
#endif
	
	// *** testing *** alert if load is from unaligned address
	if ( LoadAddress & 0x3 )
	{
#if defined INLINE_DEBUG_LOAD_UNALIGNED
		debug << "\r\nhps1x64 ALERT: LoadAddress is unaligned for LWC2 @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\r\n";
#endif

		cout << "\nhps1x64 ALERT: LoadAddress is unaligned for LWC2 @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
	}
	else
	{
		//r->DelaySlot0.Instruction = i;
		//r->DelaySlot0.Data = LoadAddress;
		//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPLWC2,0>;
		//r->Status.DelaySlot_Valid |= 0x1;
		
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		d->Data = LoadAddress;
		d->cb = Cpu::ProcessLoadDelaySlot_t<OPLWC2,0>;
		r->Status.DelaySlot_Valid |= 2;
		
#ifdef ENABLE_DEBUG_LOAD
		// used for debugging
		r->Last_ReadAddress = LoadAddress;
		r->Last_ReadWriteAddress = LoadAddress;
#endif
	}
}




// load instructions without load-delay slot
void Execute::LWL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LWL || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif

	// LWL rt, offset(base)
	
	u32 LoadAddress;
	
#if defined INLINE_DEBUG_LWL || defined INLINE_DEBUG_R3000A
			debug << "; CHECK";
#endif

#ifdef PROCESS_LOADDELAY_BEFORELOAD
	// ***testing*** load delay slot probably runs before actual value is put in line to be loaded
	r->ProcessLoadDelaySlot ();
#endif
	
#if defined INLINE_DEBUG_LWL || defined INLINE_DEBUG_R3000A
			debug << "; CHECKOK";
#endif

	// set load to happen after delay slot
	LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	
#ifdef COUT_USERMODE_LOAD
	// check if in user mode
	if ( !r->CPR0.Status.KUc )
	{
		// make sure that top 3 bits of address are clear
		if ( LoadAddress & 0xe0000000 )
		{
			cout << "\nhps1x64 ALERT: Invalid store for USER mode. LWL. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		}
	}
#endif
	
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = LoadAddress;
	//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPLWL,0>;
	//r->Status.DelaySlot_Valid |= 0x1;
	
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->Data = LoadAddress;
	d->cb = Cpu::ProcessLoadDelaySlot_t<OPLWL,0>;
	r->Status.DelaySlot_Valid |= 2;
	
	// clear the last modified register so we can see what register was modified in load delay slot
	r->LastModifiedRegister = 255;
	
#ifdef ENABLE_DEBUG_LOAD
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
#endif
}

void Execute::LWR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LWR || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif

	// LWR rt, offset(base)
	
	u32 LoadAddress;
	
#ifdef PROCESS_LOADDELAY_BEFORELOAD
	// ***testing*** load delay slot probably runs before actual value is put in line to be loaded
	r->ProcessLoadDelaySlot ();
#endif
	
	// set load to happen after delay slot
	LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	
#ifdef COUT_USERMODE_LOAD
	// check if in user mode
	if ( !r->CPR0.Status.KUc )
	{
		// make sure that top 3 bits of address are clear
		if ( LoadAddress & 0xe0000000 )
		{
			cout << "\nhps1x64 ALERT: Invalid store for USER mode. LWR. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		}
	}
#endif
	
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = LoadAddress;
	//r->DelaySlot0.cb = Cpu::ProcessLoadDelaySlot_t<OPLWR,0>;
	//r->Status.DelaySlot_Valid |= 0x1;
	
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->Data = LoadAddress;
	d->cb = Cpu::ProcessLoadDelaySlot_t<OPLWR,0>;
	r->Status.DelaySlot_Valid |= 2;
	
	// clear the last modified register so we can see what register was modified in load delay slot
	r->LastModifiedRegister = 255;
	
#ifdef ENABLE_DEBUG_LOAD
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
#endif
}




















///////////////////////////
// GTE instructions

void Execute::COP2 ( Instruction::Format i ) {}

void Execute::RTPS ( Instruction::Format i )
{
#if defined INLINE_DEBUG_RTPS || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.RTPS ( r, i );
}

void Execute::NCLIP ( Instruction::Format i )
{
#if defined INLINE_DEBUG_NCLIP || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.NCLIP ( r, i );
}

void Execute::OP ( Instruction::Format i )
{
#if defined INLINE_DEBUG_OP || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.OP ( r, i );
}

void Execute::DPCS ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DPCS || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.DPCS ( r, i );
}

void Execute::INTPL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_INTPL || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.INTPL ( r, i );
}

void Execute::MVMVA ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MVMVA || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.MVMVA ( r, i );
}

void Execute::NCDS ( Instruction::Format i )
{
#if defined INLINE_DEBUG_NCDS || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.NCDS ( r, i );
}

void Execute::CDP ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CDP || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.CDP ( r, i );
}

void Execute::NCDT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_NCDT || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.NCDT ( r, i );
}

void Execute::NCCS ( Instruction::Format i )
{
#if defined INLINE_DEBUG_NCCS || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.NCCS ( r, i );
}

void Execute::CC ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CC || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.CC ( r, i );
}

void Execute::NCS ( Instruction::Format i )
{
#if defined INLINE_DEBUG_NCS || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.NCS ( r, i );
}

void Execute::NCT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_NCT || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.NCT ( r, i );
}

void Execute::SQR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SQR || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.SQR ( r, i );
}

void Execute::DCPL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DCPL || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.DCPL ( r, i );
}

void Execute::DPCT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DPCT || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.DPCT ( r, i );
}

void Execute::AVSZ3 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_AVSZ3 || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.AVSZ3 ( r, i );
}

void Execute::AVSZ4 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_AVSZ4 || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.AVSZ4 ( r, i );
}

void Execute::RTPT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_RTPT || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.RTPT ( r, i );
}

void Execute::GPF ( Instruction::Format i )
{
#if defined INLINE_DEBUG_GPF || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.GPF ( r, i );
}

void Execute::GPL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_GPL || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.GPL ( r, i );
}

void Execute::NCCT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_NCCT || defined INLINE_DEBUG_COP2
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
#endif

	// ***testing*** load delay slot probably runs before COP2 starts executing an instruction
	r->ProcessLoadDelaySlot ();
	
	r->COP2.NCCT ( r, i );
}



const Execute::Function Execute::FunctionList []
{
	// instructions on both R3000A and R5900
	Execute::Invalid,
	Execute::J, Execute::JAL, Execute::JR, Execute::JALR, Execute::BEQ, Execute::BNE, Execute::BGTZ, Execute::BGEZ,
	Execute::BLTZ, Execute::BLEZ, Execute::BGEZAL, Execute::BLTZAL, Execute::ADD, Execute::ADDI, Execute::ADDU, Execute::ADDIU,
	Execute::SUB, Execute::SUBU, Execute::MULT, Execute::MULTU, Execute::DIV, Execute::DIVU, Execute::AND, Execute::ANDI,
	Execute::OR, Execute::ORI, Execute::XOR, Execute::XORI, Execute::NOR, Execute::LUI, Execute::SLL, Execute::SRL,
	Execute::SRA, Execute::SLLV, Execute::SRLV, Execute::SRAV, Execute::SLT, Execute::SLTI, Execute::SLTU, Execute::SLTIU,
	Execute::LB, Execute::LBU, Execute::LH, Execute::LHU, Execute::LW, Execute::LWL, Execute::LWR, Execute::SB,
	Execute::SH, Execute::SW, Execute::SWL, Execute::SWR, Execute::MFHI, Execute::MTHI, Execute::MFLO, Execute::MTLO,
	Execute::MFC0, Execute::MTC0, Execute::CFC2, Execute::CTC2, Execute::SYSCALL, Execute::BREAK,
	
	// instructions on R3000A ONLY
	Execute::MFC2, Execute::MTC2, Execute::LWC2, Execute::SWC2, Execute::RFE,
	Execute::RTPS, Execute::RTPT, Execute::CC, Execute::CDP, Execute::DCPL, Execute::DPCS, Execute::DPCT, Execute::NCS,
	Execute::NCT, Execute::NCDS, Execute::NCDT, Execute::NCCS, Execute::NCCT, Execute::GPF, Execute::GPL, Execute::AVSZ3,
	Execute::AVSZ4, Execute::SQR, Execute::OP, Execute::NCLIP, Execute::INTPL, Execute::MVMVA
};


Debug::Log Execute::debug;




void Execute::Start ()
{
#ifdef INLINE_DEBUG_ENABLE	

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create ( "R3000A_Execute_Log.txt" );
#endif

	Lookup::Start ();
}


// generates the lookup table
Execute::Execute ( Cpu* pCpu )
{
	r = pCpu;
}



void Execute::SB_Recompiler ( u32 StoreValue, u32 StoreAddress )
{
#if defined INLINE_DEBUG_SB || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " SB_Recompiler";
	debug << "; Input: rt = " << StoreValue << "; base = " << StoreAddress;
#endif
	
	// SB rt, offset(base)
	
	// clear top 3 bits since there is no data cache for caching stores
	StoreAddress &= 0x1fffffff;
	
	
	if ( r->CPR0.Status.IsC )	// *todo* - I think we need to check if DCache is isolated too - unsure
	{
#if defined INLINE_DEBUG_SB || defined INLINE_DEBUG_R3000A
		debug << "; IsC";
#endif

		//cout << "\nhps1x64 ALERT: IsC -> Cache is isolated.\n";
		// *** todo *** also this only clears cache line if this writes byte or half word - or maybe all stores invalidate??
		r->ICache.InvalidateDirect ( StoreAddress );
		
		// invalidate memory for recompiler
		//r->Bus->InvalidArray.b8 [ StoreAddress >> ( 2 + DataBus::c_iInvalidate_Shift ) ] = 1;
	}
	else
	{
#ifdef COUT_USERMODE_STORE
		// check if in user mode
		if ( !r->CPR0.Status.KUc )
		{
			// make sure that top 3 bits of address are clear
			if ( StoreAddress & 0xe0000000 )
			{
				cout << "\nhps1x64 ALERT: Invalid store for USER mode. SH. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
			}
		}
#endif
	
		if ( r->isDCache ( StoreAddress ) )
		{
#if defined INLINE_DEBUG_SB || defined INLINE_DEBUG_R3000A
			debug << "; D$";
#endif

			// step 3: if storing to data cache and lines are not reversed, then store to data cache
			//r->DCache.b16 [ ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 1 ] = (u16) r->GPR [ i.Rt ].u;
			r->DCache.b8 [ ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) ] = (u8) StoreValue;
		}
		else
		{

				//DataBus::Write_t<0xffff> ( r->GPR [ i.Rt ].u, StoreAddress );
				DataBus::Write_t<0xff> ( StoreValue, StoreAddress );
				r->CycleCount += Cpu::c_CycleTime_Store;

		}
	}

#ifdef ENABLE_DEBUG_STORE
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	//TRACE_VALUE ( r->GPR [ i.Rt ].u )
	TRACE_VALUE ( StoreValue )
#endif
}


void Execute::SH_Recompiler ( u32 StoreValue, u32 StoreAddress )
{
#if defined INLINE_DEBUG_SH || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " SH_Recompiler";
	debug << "; Input: rt = " << StoreValue << "; base = " << StoreAddress;
#endif
	
	// SH rt, offset(base)
	
	// clear top 3 bits since there is no data cache for caching stores
	StoreAddress &= 0x1fffffff;
	
	
	if ( r->CPR0.Status.IsC )	// *todo* - I think we need to check if DCache is isolated too - unsure
	{
#if defined INLINE_DEBUG_SH || defined INLINE_DEBUG_R3000A
		debug << "; IsC";
#endif

		//cout << "\nhps1x64 ALERT: IsC -> Cache is isolated.\n";
		// *** todo *** also this only clears cache line if this writes byte or half word - or maybe all stores invalidate??
		r->ICache.InvalidateDirect ( StoreAddress );
		
		// invalidate memory for recompiler
		//r->Bus->InvalidArray.b8 [ StoreAddress >> ( 2 + DataBus::c_iInvalidate_Shift ) ] = 1;
	}
	else
	{
#ifdef COUT_USERMODE_STORE
		// check if in user mode
		if ( !r->CPR0.Status.KUc )
		{
			// make sure that top 3 bits of address are clear
			if ( StoreAddress & 0xe0000000 )
			{
				cout << "\nhps1x64 ALERT: Invalid store for USER mode. SH. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
			}
		}
#endif
	
		if ( r->isDCache ( StoreAddress ) )
		{
#if defined INLINE_DEBUG_SH || defined INLINE_DEBUG_R3000A
			debug << "; D$";
#endif

			// step 3: if storing to data cache and lines are not reversed, then store to data cache
			//r->DCache.b16 [ ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 1 ] = (u16) r->GPR [ i.Rt ].u;
			r->DCache.b16 [ ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 1 ] = (u16) StoreValue;
		}
		else
		{

				//DataBus::Write_t<0xffff> ( r->GPR [ i.Rt ].u, StoreAddress );
				DataBus::Write_t<0xffff> ( StoreValue, StoreAddress );
				r->CycleCount += Cpu::c_CycleTime_Store;

		}
	}

#ifdef ENABLE_DEBUG_STORE
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	//TRACE_VALUE ( r->GPR [ i.Rt ].u )
	TRACE_VALUE ( StoreValue )
#endif
}


void Execute::SW_Recompiler ( u32 StoreValue, u32 StoreAddress )
{
#if defined INLINE_DEBUG_SW || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " SW_Recompiler";
	debug << "; Input: rt = " << StoreValue << "; base = " << StoreAddress;
#endif
	
	// SW rt, offset(base)
	
	// clear top 3 bits since there is no data cache for caching stores
	StoreAddress &= 0x1fffffff;
	
	
	if ( r->CPR0.Status.IsC )	// *todo* - I think we need to check if DCache is isolated too - unsure
	{
#if defined INLINE_DEBUG_SW || defined INLINE_DEBUG_R3000A
		debug << "; IsC";
#endif

		//cout << "\nhps1x64 ALERT: IsC -> Cache is isolated.\n";
		// *** todo *** also this only clears cache line if this writes byte or half word - or maybe all stores invalidate??
		r->ICache.InvalidateDirect ( StoreAddress );
		
		// invalidate memory for recompiler
		//r->Bus->InvalidArray.b8 [ StoreAddress >> ( 2 + DataBus::c_iInvalidate_Shift ) ] = 1;
	}
	else
	{
#ifdef COUT_USERMODE_STORE
		// check if in user mode
		if ( !r->CPR0.Status.KUc )
		{
			// make sure that top 3 bits of address are clear
			if ( StoreAddress & 0xe0000000 )
			{
				cout << "\nhps1x64 ALERT: Invalid store for USER mode. SH. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
			}
		}
#endif
	
		if ( r->isDCache ( StoreAddress ) )
		{
#if defined INLINE_DEBUG_SW || defined INLINE_DEBUG_R3000A
			debug << "; D$";
#endif

			// step 3: if storing to data cache and lines are not reversed, then store to data cache
			//r->DCache.b16 [ ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 1 ] = (u16) r->GPR [ i.Rt ].u;
			r->DCache.b32 [ ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 2 ] = (u32) StoreValue;
		}
		else
		{

				//DataBus::Write_t<0xffff> ( r->GPR [ i.Rt ].u, StoreAddress );
				DataBus::Write_t<0xffffffff> ( StoreValue, StoreAddress );
				r->CycleCount += Cpu::c_CycleTime_Store;

		}
	}

#ifdef ENABLE_DEBUG_STORE
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	//TRACE_VALUE ( r->GPR [ i.Rt ].u )
	TRACE_VALUE ( StoreValue )
#endif
}


void Execute::SWL_Recompiler ( u32 StoreValue, u32 StoreAddress )
{
#if defined INLINE_DEBUG_SWL || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " SWL_Recompiler";
	debug << "; Input: rt = " << StoreValue << "; base = " << StoreAddress;
#endif
	
	// SWL rt, offset(base)
	
	static const u32 c_Mask = 0xffffffff;
	u32 Type, Offset;
	
	// clear top 3 bits since there is no data cache for caching stores
	StoreAddress &= 0x1fffffff;
	
	
	if ( r->CPR0.Status.IsC )	// *todo* - I think we need to check if DCache is isolated too - unsure
	{
#if defined INLINE_DEBUG_SWL || defined INLINE_DEBUG_R3000A
		debug << "; IsC";
#endif

		//cout << "\nhps1x64 ALERT: IsC -> Cache is isolated.\n";
		// *** todo *** also this only clears cache line if this writes byte or half word - or maybe all stores invalidate??
		r->ICache.InvalidateDirect ( StoreAddress );
		
		// invalidate memory for recompiler
		//r->Bus->InvalidArray.b8 [ StoreAddress >> ( 2 + DataBus::c_iInvalidate_Shift ) ] = 1;
	}
	else
	{
#ifdef COUT_USERMODE_STORE
		// check if in user mode
		if ( !r->CPR0.Status.KUc )
		{
			// make sure that top 3 bits of address are clear
			if ( StoreAddress & 0xe0000000 )
			{
				cout << "\nhps1x64 ALERT: Invalid store for USER mode. SH. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
			}
		}
#endif
	
		if ( r->isDCache ( StoreAddress ) )
		{
#if defined INLINE_DEBUG_SWL || defined INLINE_DEBUG_R3000A
			debug << "; D$";
#endif

			// step 3: if storing to data cache and lines are not reversed, then store to data cache
			//r->DCache.b16 [ ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 1 ] = (u16) r->GPR [ i.Rt ].u;
			Type = 3 - ( StoreAddress & 3 );
			Offset = ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 2;
			r->DCache.b32 [ Offset ] = ( StoreValue >> ( Type << 3 ) ) | ( r->DCache.b32 [ Offset ] & ~( c_Mask >> ( Type << 3 ) ) );
		}
		else
		{

				//DataBus::Write_t<0xffff> ( r->GPR [ i.Rt ].u, StoreAddress );
				DataBus::Write ( StoreValue >> ( ( 3 - ( StoreAddress & 3 ) ) << 3 ), StoreAddress & ~3, 0xffffffffUL >> ( ( 3 - ( StoreAddress & 3 ) ) << 3 ) );
				r->CycleCount += Cpu::c_CycleTime_Store;

		}
	}

#ifdef ENABLE_DEBUG_STORE
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	//TRACE_VALUE ( r->GPR [ i.Rt ].u )
	TRACE_VALUE ( StoreValue )
#endif
}


void Execute::SWR_Recompiler ( u32 StoreValue, u32 StoreAddress )
{
#if defined INLINE_DEBUG_SWR || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " SWR_Recompiler";
	debug << "; Input: rt = " << StoreValue << "; base = " << StoreAddress;
#endif
	
	// SWR rt, offset(base)
	
	static const u32 c_Mask = 0xffffffff;
	u32 Type, Offset;
	
	// clear top 3 bits since there is no data cache for caching stores
	StoreAddress &= 0x1fffffff;
	
	
	if ( r->CPR0.Status.IsC )	// *todo* - I think we need to check if DCache is isolated too - unsure
	{
#if defined INLINE_DEBUG_SWR || defined INLINE_DEBUG_R3000A
		debug << "; IsC";
#endif

		//cout << "\nhps1x64 ALERT: IsC -> Cache is isolated.\n";
		// *** todo *** also this only clears cache line if this writes byte or half word - or maybe all stores invalidate??
		r->ICache.InvalidateDirect ( StoreAddress );
		
		// invalidate memory for recompiler
		//r->Bus->InvalidArray.b8 [ StoreAddress >> ( 2 + DataBus::c_iInvalidate_Shift ) ] = 1;
	}
	else
	{
#ifdef COUT_USERMODE_STORE
		// check if in user mode
		if ( !r->CPR0.Status.KUc )
		{
			// make sure that top 3 bits of address are clear
			if ( StoreAddress & 0xe0000000 )
			{
				cout << "\nhps1x64 ALERT: Invalid store for USER mode. SH. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
			}
		}
#endif
	
		if ( r->isDCache ( StoreAddress ) )
		{
#if defined INLINE_DEBUG_SWR || defined INLINE_DEBUG_R3000A
			debug << "; D$";
#endif

			// step 3: if storing to data cache and lines are not reversed, then store to data cache
			//r->DCache.b16 [ ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 1 ] = (u16) r->GPR [ i.Rt ].u;
			Type = StoreAddress & 3;
			Offset = ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 2;
			r->DCache.b32 [ Offset ] = ( StoreValue << ( Type << 3 ) ) | ( r->DCache.b32 [ Offset ] & ~( c_Mask << ( Type << 3 ) ) );
		}
		else
		{

				//DataBus::Write_t<0xffff> ( r->GPR [ i.Rt ].u, StoreAddress );
				DataBus::Write ( StoreValue << ( ( StoreAddress & 3 ) << 3 ), StoreAddress & ~3, 0xffffffffUL << ( ( StoreAddress & 3 ) << 3 ) );
				r->CycleCount += Cpu::c_CycleTime_Store;

		}
	}

#ifdef ENABLE_DEBUG_STORE
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	//TRACE_VALUE ( r->GPR [ i.Rt ].u )
	TRACE_VALUE ( StoreValue )
#endif
}


void Execute::SWC2_Recompiler ( u32 StoreValue, u32 StoreAddress )
{
#if defined INLINE_DEBUG_SWC2 || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " SWC2_Recompiler";
	debug << "; Input: rt = " << StoreValue << "; base = " << StoreAddress;
#endif
	
	// SWC2 rt, offset(base)
	
	// clear top 3 bits since there is no data cache for caching stores
	StoreAddress &= 0x1fffffff;
	
	
	if ( r->CPR0.Status.IsC )	// *todo* - I think we need to check if DCache is isolated too - unsure
	{
#if defined INLINE_DEBUG_SWC2 || defined INLINE_DEBUG_R3000A
		debug << "; IsC";
#endif

		//cout << "\nhps1x64 ALERT: IsC -> Cache is isolated.\n";
		// *** todo *** also this only clears cache line if this writes byte or half word - or maybe all stores invalidate??
		r->ICache.InvalidateDirect ( StoreAddress );
		
		// invalidate memory for recompiler
		//r->Bus->InvalidArray.b8 [ StoreAddress >> ( 2 + DataBus::c_iInvalidate_Shift ) ] = 1;
	}
	else
	{
#ifdef COUT_USERMODE_STORE
		// check if in user mode
		if ( !r->CPR0.Status.KUc )
		{
			// make sure that top 3 bits of address are clear
			if ( StoreAddress & 0xe0000000 )
			{
				cout << "\nhps1x64 ALERT: Invalid store for USER mode. SH. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
			}
		}
#endif
	
		if ( r->isDCache ( StoreAddress ) )
		{
#if defined INLINE_DEBUG_SWC2 || defined INLINE_DEBUG_R3000A
			debug << "; D$";
#endif

			// step 3: if storing to data cache and lines are not reversed, then store to data cache
			//r->DCache.b16 [ ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 1 ] = (u16) r->GPR [ i.Rt ].u;
			r->DCache.b32 [ ( StoreAddress & ( Cpu::c_ScratchPadRam_Size - 1 ) ) >> 2 ] = (u32) StoreValue;
		}
		else
		{

				//DataBus::Write_t<0xffff> ( r->GPR [ i.Rt ].u, StoreAddress );
				DataBus::Write_t<0xffffffff> ( StoreValue, StoreAddress );
				r->CycleCount += Cpu::c_CycleTime_Store;

		}
	}

#ifdef ENABLE_DEBUG_STORE
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	//TRACE_VALUE ( r->GPR [ i.Rt ].u )
	TRACE_VALUE ( StoreValue )
#endif
}

/*
void Execute::LW_Recompiler ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LW || defined INLINE_DEBUG_R3000A
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << hex << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// LW rt, offset(base)
	
	u32 LoadAddress;
	
	
	// set load to happen after delay slot
	LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	
#ifdef COUT_USERMODE_LOAD
	// check if in user mode
	if ( !r->CPR0.Status.KUc )
	{
		// make sure that top 3 bits of address are clear
		if ( LoadAddress & 0xe0000000 )
		{
			cout << "\nhps1x64 ALERT: Invalid store for USER mode. LW. Cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		}
	}
#endif
	
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		d->Data = LoadAddress;
		d->cb = Cpu::ProcessLoadDelaySlot_t<OPLW,0>;
		r->Status.DelaySlot_Valid |= 2;
		
		// clear the last modified register so we can see what register was modified in load delay slot
		r->LastModifiedRegister = 255;
		
#ifdef ENABLE_DEBUG_LOAD
		// used for debugging
		r->Last_ReadAddress = LoadAddress;
		r->Last_ReadWriteAddress = LoadAddress;
#endif
}
*/



