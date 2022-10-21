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




#include "R5900_Execute.h"
#include "R5900_Lookup.h"

#include "VU.h"
#include "VU_Execute.h"
#include "PS2Float.h"

#include "R5900_Print.h"

#include "GeneralUtilities.h"

#include <iostream>
#include <iomanip>

using namespace GeneralUtilities;
using namespace Playstation2;
using namespace PS2Float;


//#define VERBOSE_MSCAL
//#define VERBOSE_VCALLMS
//#define VERBOSE_VCALLMSR

#define ENABLE_MACROMODE_CHECKVU0
#define ENABLE_STALLS


// enables i-cache on R5900
// must be enabled here and in R5900.cpp
#define ENABLE_R5900_ICACHE

// enables d-cache on R5900
// must be enabled here and in R5900_Recompiler.cpp, R5900_DCache.h
// note: sometimes this is required
// ***todo*** does not work perfectly yet
//#define ENABLE_R5900_DCACHE


#ifdef ENABLE_R5900_DCACHE

//#define DCACHE_FORCE_MEMPTR

//#define DCACHE_READ_MEMORY
//#define DCACHE_WRITE_MEMORY

//#define DCACHE_ALWAYS_MARK_DIRTY


#define ENABLE_DCACHE_TIMING_LOAD
#define ENABLE_DCACHE_TIMING_STORE

#define ENABLE_BUS_SIMULATION_CACHE_LOAD
#define ENABLE_BUS_SIMULATION_CACHE_STORE


#define ENABLE_R5900_DCACHE_SB
#define ENABLE_R5900_DCACHE_SH
#define ENABLE_R5900_DCACHE_SW

#define ENABLE_R5900_DCACHE_SWL
#define ENABLE_R5900_DCACHE_SWR
#define ENABLE_R5900_DCACHE_SWC1

#define ENABLE_R5900_DCACHE_SD
#define ENABLE_R5900_DCACHE_SDL
#define ENABLE_R5900_DCACHE_SDR
#define ENABLE_R5900_DCACHE_SQ
#define ENABLE_R5900_DCACHE_SQC2


#define ENABLE_R5900_DCACHE_LB
#define ENABLE_R5900_DCACHE_LBU
#define ENABLE_R5900_DCACHE_LH
#define ENABLE_R5900_DCACHE_LHU
#define ENABLE_R5900_DCACHE_LW
#define ENABLE_R5900_DCACHE_LWC1
#define ENABLE_R5900_DCACHE_LWU
#define ENABLE_R5900_DCACHE_LWL
#define ENABLE_R5900_DCACHE_LWR
#define ENABLE_R5900_DCACHE_LD
#define ENABLE_R5900_DCACHE_LDL
#define ENABLE_R5900_DCACHE_LDR
#define ENABLE_R5900_DCACHE_LQ
#define ENABLE_R5900_DCACHE_LQC2

#endif	// end #ifdef ENABLE_R5900_DCACHE


// some early branch prediction simulation
#define ENABLE_R5900_BRANCH_PREDICTION


// skip around while waiting for vu0 rather than executing every single cycle#
#define ENABLE_VU0_SKIP_WAIT


//#define UPDATE_INTERRUPTS_EI
//#define UPDATE_INTERRUPTS_DI



#ifdef _DEBUG_VERSION_

// enable debug
#define INLINE_DEBUG_ENABLE

//#define INLINE_DEBUG_SPLIT

//#define INLINE_DEBUG_SUB_S
//#define INLINE_DEBUG_SUBA_S

//#define INLINE_DEBUG_ADD_S
//#define INLINE_DEBUG_ADDA_S
//#define INLINE_DEBUG_FPU
//#define INLINE_DEBUG_VU0

//#define INLINE_DEBUG_PREF

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

/*
#define INLINE_DEBUG_LBU
#define INLINE_DEBUG_LB
#define INLINE_DEBUG_SB
#define INLINE_DEBUG_LH
#define INLINE_DEBUG_LHU
#define INLINE_DEBUG_SH
#define INLINE_DEBUG_LW
#define INLINE_DEBUG_SW
#define INLINE_DEBUG_LWU

#define INLINE_DEBUG_LD
#define INLINE_DEBUG_SD
#define INLINE_DEBUG_LQ
#define INLINE_DEBUG_SQ
#define INLINE_DEBUG_SWL
#define INLINE_DEBUG_LWL
#define INLINE_DEBUG_SWR
#define INLINE_DEBUG_LWR
#define INLINE_DEBUG_SDL
#define INLINE_DEBUG_LDL
#define INLINE_DEBUG_SDR
#define INLINE_DEBUG_LDR
#define INLINE_DEBUG_LQC2
#define INLINE_DEBUG_SQC2
#define INLINE_DEBUG_LWC1
#define INLINE_DEBUG_SWC1
*/

//#define INLINE_DEBUG_CACHE

//#define INLINE_DEBUG_EI
//#define INLINE_DEBUG_DI
//#define INLINE_DEBUG_SYNC


//#define INLINE_DEBUG_SYSCALL
//#define INLINE_DEBUG_RFE
//#define INLINE_DEBUG_ADD
//#define INLINE_DEBUG_R5900
//#define INLINE_DEBUG_DCACHE
#define INLINE_DEBUG_BREAK
#define INLINE_DEBUG_INVALID
#define INLINE_DEBUG_UNIMPLEMENTED
//#define INLINE_DEBUG_ERET
#define INLINE_DEBUG_INTEGER_VECTOR
//#define INLINE_DEBUG_VU0
//#define INLINE_DEBUG_VUEXECUTE
//#define INLINE_DEBUG_FPU
//#define INLINE_DEBUG_PADDSW
//#define INLINE_DEBUG_QFSRV
//#define INLINE_DEBUG_MFSA
//#define INLINE_DEBUG_MTSA
//#define INLINE_DEBUG_MTSAB
//#define INLINE_DEBUG_MTSAH


/*
#define INLINE_DEBUG_PABSH
#define INLINE_DEBUG_PABSW
#define INLINE_DEBUG_PADDB
#define INLINE_DEBUG_PADDH
#define INLINE_DEBUG_PADDW
#define INLINE_DEBUG_PADSBH
#define INLINE_DEBUG_PAND


#define INLINE_DEBUG_PCPYH
#define INLINE_DEBUG_PCPYLD
#define INLINE_DEBUG_PCPYUD


#define INLINE_DEBUG_PEXCH
#define INLINE_DEBUG_PEXCW
#define INLINE_DEBUG_PEXEH
#define INLINE_DEBUG_PEXEW
#define INLINE_DEBUG_PEXT5
#define INLINE_DEBUG_PEXTLB
#define INLINE_DEBUG_PEXTLH
#define INLINE_DEBUG_PEXTLW
#define INLINE_DEBUG_PEXTUB
#define INLINE_DEBUG_PEXTUH
#define INLINE_DEBUG_PEXTUW


#define INLINE_DEBUG_PCEQB
#define INLINE_DEBUG_PCEQH
#define INLINE_DEBUG_PCEQW
#define INLINE_DEBUG_PCGTB
#define INLINE_DEBUG_PCGTH
#define INLINE_DEBUG_PCGTW
#define INLINE_DEBUG_PMAXH
#define INLINE_DEBUG_PMAXW
#define INLINE_DEBUG_PMINH
#define INLINE_DEBUG_PMINW

*/

#define INLINE_DEBUG_PDIVBW
#define INLINE_DEBUG_PDIVUW
#define INLINE_DEBUG_PDIVW

//#define INLINE_DEBUG_MTC0
//#define INLINE_DEBUG_MFC0
//#define INLINE_DEBUG_CTC0
//#define INLINE_DEBUG_CFC0
//#define INLINE_DEBUG_MFC1
//#define INLINE_DEBUG_MTC1
//#define INLINE_DEBUG_CFC1
//#define INLINE_DEBUG_CTC1


//#define INLINE_DEBUG_QMTC2_I
//#define INLINE_DEBUG_QMFC2_I
//#define INLINE_DEBUG_QMTC2_NI
//#define INLINE_DEBUG_QMFC2_NI
//#define INLINE_DEBUG_CTC2
//#define INLINE_DEBUG_CFC2
//#define INLINE_DEBUG_COP2


//#define INLINE_DEBUG_DIV_S
//#define INLINE_DEBUG_ADD_S
//#define INLINE_DEBUG_SUB_S
//#define INLINE_DEBUG_MUL_S
//#define INLINE_DEBUG_ADDA_S
//#define INLINE_DEBUG_SUBA_S
//#define INLINE_DEBUG_MULA_S
//#define INLINE_DEBUG_SQRT_S
//#define INLINE_DEBUG_RSQRT_S
//#define INLINE_DEBUG_MADD_S
//#define INLINE_DEBUG_MSUB_S
//#define INLINE_DEBUG_MADDA_S
//#define INLINE_DEBUG_MSUBA_S
//#define INLINE_DEBUG_CVT_S_W
//#define INLINE_DEBUG_CVT_W_S
//#define INLINE_DEBUG_MIN_S
//#define INLINE_DEBUG_MAX_S

//#define INLINE_DEBUG_BLTZL


#define INLINE_DEBUG_STORE_UNALIGNED
#define INLINE_DEBUG_BREAK_EXCEPTION

//#define COUT_USERMODE_LOAD
//#define COUT_USERMODE_STORE
//#define COUT_FC
//#define COUT_SWC

//#define INLINE_DEBUG_TRACE

#endif






using namespace std;

// this area deals with the execution of instructions on the R5900
using namespace R5900;
using namespace R5900::Instruction;



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
//#define PROCESS_LOADDELAY_BEFORESTORE


namespace R5900
{

namespace Instruction
{


// static vars //
Cpu *Execute::r;



void Execute::Start ()
{
#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create ( "R5900_Execute_Log.txt" );
#endif

	cout << "\nRunning R5900::Execute::Start\n";

	// this function currently takes a long time to execute
	Lookup::Start ();
}


// *** R3000A Instructions *** //


////////////////////////////////////////////////
// R-Type Instructions (non-interrupt)


// regular arithemetic
void Execute::ADDU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDU || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// add without overflow exception: rd = rs + rt
	r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rs ].u + r->GPR [ i.Rt ].u );
	
#if defined INLINE_DEBUG_ADDU || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SUBU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBU || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// subtract without overflow exception: rd = rs - rt
	r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rs ].u - r->GPR [ i.Rt ].u );
	
#if defined INLINE_DEBUG_SUBU || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::AND ( Instruction::Format i )
{
#if defined INLINE_DEBUG_AND || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// logical AND: rd = rs & rt
	r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u & r->GPR [ i.Rt ].u;
	
#if defined INLINE_DEBUG_AND || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::OR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_OR || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// logical OR: rd = rs | rt
	r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u | r->GPR [ i.Rt ].u;
	
#if defined INLINE_DEBUG_OR || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::XOR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_XOR || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// logical XOR: rd = rs ^ rt
	r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u ^ r->GPR [ i.Rt ].u;
	
#if defined INLINE_DEBUG_XOR || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::NOR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_NOR || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// logical NOR: rd = ~(rs | rt)
	r->GPR [ i.Rd ].u = ~( r->GPR [ i.Rs ].u | r->GPR [ i.Rt ].u );
	
#if defined INLINE_DEBUG_NOR || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SLT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SLT || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// set less than signed: rd = rs < rt ? 1 : 0
	r->GPR [ i.Rd ].s = r->GPR [ i.Rs ].s < r->GPR [ i.Rt ].s ? 1 : 0;
	
#if defined INLINE_DEBUG_SLT || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SLTU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SLTU || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// set less than signed: rd = rs < rt ? 1 : 0
	r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u < r->GPR [ i.Rt ].u ? 1 : 0;
	
#if defined INLINE_DEBUG_SLTU || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}


////////////////////////////////////////////
// I-Type Instructions (non-interrupt)



void Execute::ADDIU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDIU || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif

	// *note* immediate value is sign-extended before the addition is performed

	// add immedeate without overflow exception: rt = rs + immediate
	r->GPR [ i.Rt ].s = (s32) ( r->GPR [ i.Rs ].s + i.sImmediate );
	
#if defined INLINE_DEBUG_ADDIU || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::ANDI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ANDI || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// Logical AND zero-extended immedeate: rt = rs & immediate
	r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u & ( (u64) i.uImmediate );
	
#if defined INLINE_DEBUG_ANDI || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::ORI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ORI || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// Logical OR zero-extended immedeate: rt = rs | immediate
	r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u | ( (u64) i.uImmediate );
	
#if defined INLINE_DEBUG_ORI || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::XORI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_XORI || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// Logical XOR zero-extended immedeate: rt = rs & immediate
	r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u ^ ( (u64) i.uImmediate );
	
#if defined INLINE_DEBUG_XORI || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::SLTI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SLTI || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// Set less than sign-extended immedeate: rt = rs < immediate ? 1 : 0
	r->GPR [ i.Rt ].s = r->GPR [ i.Rs ].s < ( (s64) i.sImmediate ) ? 1 : 0;
	
#if defined INLINE_DEBUG_SLTI || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::SLTIU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SLTIU || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// *note* Rs is still sign-extended here, but the comparison is an unsigned one

	// Set if unsigned less than sign-extended immedeate: rt = rs < immediate ? 1 : 0
	r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u < ((u64) ((s64) i.sImmediate)) ? 1 : 0;
	
#if defined INLINE_DEBUG_SLTIU || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::LUI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LUI || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif
	
	// Load upper immediate
	r->GPR [ i.Rt ].s = ( ( (s64) i.sImmediate ) << 16 );
	
#if defined INLINE_DEBUG_LUI || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}





void Execute::MFHI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MFHI || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: HI = " << r->HI.u;
#endif
	
	// this instruction interlocks if multiply/divide unit is busy
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	
	// move from Hi register
	r->GPR [ i.Rd ].u = r->HI.u;	//r->HiLo.uHi;
	
#if defined INLINE_DEBUG_MFHI || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::MFLO ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MFLO || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: LO = " << r->LO.u;
#endif
	
	// this instruction interlocks if multiply/divide unit is busy
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	
	// move from Lo register
	r->GPR [ i.Rd ].u = r->LO.u;	//r->HiLo.uLo;
	
#if defined INLINE_DEBUG_MFLO || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}




void Execute::MTHI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MTHI || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// there is no MTHI/MTLO delay slot
	//r->HiLo.uHi = r->GPR [ i.Rs ].u;
	r->HI.u = r->GPR [ i.Rs ].u;
}

void Execute::MTLO ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MTLO || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// there is no MTHI/MTLO delay slot
	//r->HiLo.uLo = r->GPR [ i.Rs ].u;
	r->LO.u = r->GPR [ i.Rs ].u;
}


//////////////////////////////////////////////////////////
// Shift instructions



void Execute::SLL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SLL || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// shift left logical: rd = rt << shift
	//r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rt ].u << i.Shift );
	r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rt ].uw0 << i.Shift );
	
#if defined INLINE_DEBUG_SLL || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SRL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SRL || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// shift right logical: rd = rt >> shift
	//r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rt ].u >> i.Shift );
	r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rt ].uw0 >> i.Shift );
	
#if defined INLINE_DEBUG_SRL || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SRA ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SRA || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// shift right arithmetic: rd = rt >> shift
	//r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rt ].s >> i.Shift );
	r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rt ].sw0 >> i.Shift );
	
#if defined INLINE_DEBUG_SRA || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SLLV ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SLLV || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// shift left logical variable: rd = rt << rs
	//r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rt ].u << ( r->GPR [ i.Rs ].u & 0x1f ) );
	r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rt ].uw0 << ( r->GPR [ i.Rs ].uw0 & 0x1f ) );
	
#if defined INLINE_DEBUG_SLLV || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SRLV ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SRLV || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// shift right logical variable: rd = rt >> rs
	//r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rt ].u >> ( r->GPR [ i.Rs ].u & 0x1f ) );
	r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rt ].uw0 >> ( r->GPR [ i.Rs ].uw0 & 0x1f ) );
	
#if defined INLINE_DEBUG_SRLV || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::SRAV ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SRAV || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// shift right arithmetic variable: rd = rt >> rs
	//r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rt ].s >> ( r->GPR [ i.Rs ].u & 0x1f ) );
	r->GPR [ i.Rd ].s = (s32) ( r->GPR [ i.Rt ].sw0 >> ( r->GPR [ i.Rs ].uw0 & 0x1f ) );
	
#if defined INLINE_DEBUG_SRAV || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}


/////////////////////////////////////////////////////////////
// Multiply/Divide Instructions


void Execute::MULT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MULT || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}

	// on PS2, only need to calculate cycles for multiply if it is running at full clock speed?
	/*
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
	*/
	
	// multiply signed Lo,Hi = rs * rt
	//r->HiLo.sValue = ((s64) (r->GPR [ i.Rs ].s)) * ((s64) (r->GPR [ i.Rt ].s));
	s64 temp;
	temp = ( (s64) ( r->GPR [ i.Rs ].sw0 ) ) * ( (s64) ( r->GPR [ i.Rt ].sw0 ) );
	r->LO.s = (s32) temp;
	r->HI.s = (s32) ( temp >> 32 );
	
	// R5900 can additionally write to register
	if ( i.Rd )
	{
		r->GPR [ i.Rd ].sq0 = (s32) temp;
	}

#if defined INLINE_DEBUG_MULT || defined INLINE_DEBUG_R5900
	debug << "; Output: LO=" << r->LO.s << "; HI=" << r->HI.s << "; rd=" << r->GPR [ i.Rd ].s;
#endif
}

void Execute::MULTU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MULTU || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// if rs is between 0 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	
	// on PS2, only need to calculate cycles for multiply if it is running at full clock speed?
	/*
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
	*/

	// multiply unsigned Lo,Hi = rs * rt
	//r->HiLo.uValue = ((u64) (r->GPR [ i.Rs ].u)) * ((u64) (r->GPR [ i.Rt ].u));
	u64 temp;
	temp = ( (u64) ( r->GPR [ i.Rs ].uw0 ) ) * ( (u64) ( r->GPR [ i.Rt ].uw0 ) );
	r->LO.s = (s32) temp;
	r->HI.s = (s32) ( temp >> 32 );
	
	// R5900 can additionally write to register
	if ( i.Rd )
	{
		r->GPR [ i.Rd ].sq0 = (s32) temp;
	}
	
#if defined INLINE_DEBUG_MULTU || defined INLINE_DEBUG_R5900
	debug << "; Output: LO=" << r->LO.s << "; HI=" << r->HI.s << "; rd=" << r->GPR [ i.Rd ].s;
#endif
}

void Execute::DIV ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DIV || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// 37 cycles
	// divide by 2 here since r5900 is currently only running at bus speed for testing
	static const int c_iDivideCycles = 37 / 2;

	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	
	// mult/div unit is busy now
	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iDivideCycles;

	// divide signed: Lo = rs / rt; Hi = rs % rt
	//if ( r->GPR [ i.Rt ].u != 0 )
	if ( r->GPR [ i.Rt ].uw0 != 0 )
	{
		// if rs = 0x80000000 and rt = -1 then hi = 0 and lo = 0x80000000
		if ( r->GPR [ i.Rs ].uw0 == 0x80000000 && r->GPR [ i.Rt ].sw0 == -1 )
		{
			// *todo* check if all this stuff is sign extended during testing
			//r->HiLo.uHi = 0;
			//r->HiLo.uLo = 0x80000000;
			r->HI.s = 0;
			r->LO.s = (s32) 0x80000000;
		}
		else
		{
			// *todo* check during testing if signs are switched on modulus result when sign of dividend and divisor are different
			//r->HiLo.sLo = r->GPR [ i.Rs ].s / r->GPR [ i.Rt ].s;
			//r->HiLo.sHi = r->GPR [ i.Rs ].s % r->GPR [ i.Rt ].s;
			r->LO.s = r->GPR [ i.Rs ].sw0 / r->GPR [ i.Rt ].sw0;
			r->HI.s = r->GPR [ i.Rs ].sw0 % r->GPR [ i.Rt ].sw0;
		}
	}
	else
	{
		if ( r->GPR [ i.Rs ].s < 0 )
		{
			//r->HiLo.sLo = 1;
			r->LO.s = 1;
		}
		else
		{
			//r->HiLo.sLo = -1;
			r->LO.s = -1;
		}
		
		//r->HiLo.uHi = r->GPR [ i.Rs ].u;
		r->HI.s = r->GPR [ i.Rs ].sw0;
	}
	
#if defined INLINE_DEBUG_DIV || defined INLINE_DEBUG_R5900
	debug << "; Output: LO = " << r->LO.s << "; HI = " << r->HI.s;
#endif
}

void Execute::DIVU ( Instruction::Format i )
{
	// 37 cycles
	// divide by 2 here since r5900 is currently only running at bus speed for testing
	static const int c_iDivideCycles = 37 / 2;
	
#if defined INLINE_DEBUG_DIVU || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	
	// mult/div unit is busy now
	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iDivideCycles;

	// divide unsigned: Lo = rs / rt; Hi = rs % rt
	//if ( r->GPR [ i.Rt ].u != 0 )
	if ( r->GPR [ i.Rt ].uw0 != 0 )
	{
		//r->HiLo.uLo = r->GPR [ i.Rs ].u / r->GPR [ i.Rt ].u;
		//r->HiLo.uHi = r->GPR [ i.Rs ].u % r->GPR [ i.Rt ].u;
		r->LO.s = (s32) ( r->GPR [ i.Rs ].uw0 / r->GPR [ i.Rt ].uw0 );
		r->HI.s = (s32) ( r->GPR [ i.Rs ].uw0 % r->GPR [ i.Rt ].uw0 );
	}
	else
	{
		//r->HiLo.sLo = -1;
		//r->HiLo.uHi = r->GPR [ i.Rs ].u;
		r->LO.s = -1;
		r->HI.s = r->GPR [ i.Rs ].sw0;
	}
	
#if defined INLINE_DEBUG_DIVU || defined INLINE_DEBUG_R5900
	debug << "; Output: LO = " << r->LO.s << "; HI = " << r->HI.s;
#endif
}



////////////////////////////////////////////
// Jump/Branch Instructions



void Execute::J ( Instruction::Format i )
{
#if defined INLINE_DEBUG_J || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif
	
	// next instruction is in the branch delay slot
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.cb = r->_cb_Jump;
	d->Instruction = i;
	//d->cb = r->_cb_Jump;
	d->cb = r->ProcessBranchDelaySlot_t<OPJ>;
	r->Status.DelaySlot_Valid |= 0x2;
}

void Execute::JR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_JR || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
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
	
	// next instruction is in the branch delay slot
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	//d->cb = r->_cb_JumpRegister;
	d->cb = r->ProcessBranchDelaySlot_t<OPJR>;

	// *** todo *** check if address exception should be generated if lower 2-bits of jump address are not zero
	// will clear out lower two bits of address for now
	d->Data = r->GPR [ i.Rs ].u & ~3;
	
	r->Status.DelaySlot_Valid |= 0x2;
}

void Execute::JAL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_JAL || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	
	// next instruction is in the branch delay slot
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	//d->cb = r->_cb_Jump;
	d->cb = r->ProcessBranchDelaySlot_t<OPJAL>;
	r->Status.DelaySlot_Valid |= 0x2;

	// *** note *** this is tricky because return address gets stored to r31 after execution of load delay slot but before next instruction
	///////////////////////////////////////////////////////////////
	// Store return address when instruction is executed in r31
	r->GPR [ 31 ].u = r->PC + 8;
	
#if defined INLINE_DEBUG_JAL || defined INLINE_DEBUG_R5900
	debug << "; Output: r31 = " << r->GPR [ 31 ].u;
#endif
}

void Execute::JALR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_JALR || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
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
	
	// next instruction is in the branch delay slot
	//r->DelaySlot0.Instruction = i;
	
	// *** todo *** check if address exception should be generated if lower 2-bits of jump address are not zero
	// will clear out lower two bits of address for now
	Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
	d->Instruction = i;
	d->Data = r->GPR [ i.Rs ].u & ~3;
	//d->cb = r->_cb_JumpRegister;
	d->cb = r->ProcessBranchDelaySlot_t<OPJALR>;
	
	r->Status.DelaySlot_Valid |= 0x2;

	///////////////////////////////////////////////////////////////
	// Store return address when instruction is executed in Rd
	// *note* this must happen AFTER the stuff above
	r->GPR [ i.Rd ].u = r->PC + 8;
	
#if defined INLINE_DEBUG_JALR || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::BEQ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BEQ || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	if ( r->GPR [ i.Rs ].u == r->GPR [ i.Rt ].u )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBEQ>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BEQ || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BNE ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BNE || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	if ( r->GPR [ i.Rs ].u != r->GPR [ i.Rt ].u )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBNE>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BNE || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BLEZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BLEZ || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	if ( r->GPR [ i.Rs ].s <= 0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBLEZ>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BLEZ || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BGTZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BGTZ || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	if ( r->GPR [ i.Rs ].s > 0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBGTZ>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BGTZ || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BLTZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BLTZ || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	if ( r->GPR [ i.Rs ].s < 0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBLTZ>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BLTZ || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BGEZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BGEZ || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	if ( r->GPR [ i.Rs ].s >= 0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBGEZ>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BGEZ || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BLTZAL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BLTZAL || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	if ( r->GPR [ i.Rs ].s < 0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBLTZAL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BLTZAL || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
	
	////////////////////////////////////////////////////////////////////////
	// Store return address when instruction is executed in r31
	// for this instruction this happens whether branch is taken or not
	// *note* this must happen AFTER comparison check
	r->GPR [ 31 ].u = r->PC + 8;
}

void Execute::BGEZAL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BGEZAL || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif
	
	if ( r->GPR [ i.Rs ].s >= 0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBGEZAL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BGEZAL || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}

	////////////////////////////////////////////////////////////////////////
	// Store return address when instruction is executed in r31
	// for this instruction this happens whether branch is taken or not
	// *note* this must happen AFTER comparison check
	r->GPR [ 31 ].u = r->PC + 8;
}



/*
void Execute::RFE ( Instruction::Format i )
{
#if defined INLINE_DEBUG_RFE || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
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

#if defined INLINE_DEBUG_RFE || defined INLINE_DEBUG_R5900
	debug << "\r\n(after) _Intc_Stat=" << hex << *r->_Intc_Stat << " _Intc_Mask=" << *r->_Intc_Mask << " _R3000A_Status=" << r->CPR0.Regs [ 12 ] << " _R3000A_Cause=" << r->CPR0.Regs [ 13 ] << " _ProcStatus=" << r->Status.Value << " CycleCount=" << dec << r->CycleCount;
#endif
}
*/



////////////////////////////////////////////////////////
// Instructions that can cause Synchronous Interrupts //
////////////////////////////////////////////////////////


void Execute::ADD ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ADD || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	s32 temp;
	
	temp = r->GPR [ i.Rs ].sw0 + r->GPR [ i.Rt ].sw0;
	
	// if the carry outs of bits 30 and 31 differ, then it's signed overflow
	//if ( ( temp < -2147483648LL ) || ( temp > 2147483647LL ) )
	//if( (INT32)( ~( m_r[ INS_RS( m_op ) ] ^ m_r[ INS_RT( m_op ) ] ) & ( m_r[ INS_RS( m_op ) ] ^ result ) ) < 0 )
	//if ( (s32)( ~( r->GPR [ i.Rs ].s ^ r->GPR [ i.Rt ].s ) & ( r->GPR [ i.Rs ].s ^ temp ) ) < 0 )
	//if ( temp < -0x80000000LL || temp > 0x7fffffffLL )
	if ( ( ( ~( r->GPR [ i.Rs ].sw0 ^ r->GPR [ i.Rt ].sw0 ) ) & ( r->GPR [ i.Rs ].sw0 ^ temp ) ) < 0 )
	{
		// overflow
		cout << "\nhps2x64: Execute::ADD generated an overflow exception @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << "\n";
		r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
		
#if defined INLINE_DEBUG_ADD || defined INLINE_DEBUG_R5900
		debug << ";  INT";
#endif
	}
	else
	{
		// it's cool - we can do the add and store the result to register
		// sign-extend for R5900
		r->GPR [ i.Rd ].s = temp;
	}
	
#if defined INLINE_DEBUG_ADD || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::ADDI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDI || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif

	s32 temp;
	
	//temp = ( (s64) r->GPR [ i.Rs ].sw0 ) + ( (s64) i.sImmediate );
	temp = r->GPR [ i.Rs ].sw0 + ( (s32) i.sImmediate );
	
	// if the carry outs of bits 30 and 31 differ, then it's signed overflow
	//if ( ( temp < -2147483648LL ) || ( temp > 2147483647LL ) )
	//if( (INT32)( ~( m_r[ INS_RS( m_op ) ] ^ m_r[ INS_RT( m_op ) ] ) & ( m_r[ INS_RS( m_op ) ] ^ result ) ) < 0 )
	//if ( (s32)( ~( r->GPR [ i.Rs ].s ^ ( (s32) i.sImmediate ) ) & ( r->GPR [ i.Rs ].s ^ temp ) ) < 0 )
	//if ( temp < -0x80000000LL || temp > 0x7fffffffLL )
	if ( ( ( ~( r->GPR [ i.Rs ].sw0 ^ ( (s32) i.sImmediate ) ) ) & ( r->GPR [ i.Rs ].sw0 ^ temp ) ) < 0 )
	{
		// overflow
		cout << "\nhps2x64: Execute::ADDI generated an overflow exception @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << "\n";
		r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
		
#if defined INLINE_DEBUG_ADDI || defined INLINE_DEBUG_R5900
		debug << ";  INT";
#endif
	}
	else
	{
		// it's cool - we can do the addi and store the result to register
		r->GPR [ i.Rt ].s = temp;
	}
	
#if defined INLINE_DEBUG_ADDI || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::SUB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SUB || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif
	
	s32 temp;
	
	temp = r->GPR [ i.Rs ].sw0 - r->GPR [ i.Rt ].sw0;
	
	// if the carry outs of bits 30 and 31 differ, then it's signed overflow
	//if ( ( temp < -2147483648LL ) || ( temp > 2147483647LL ) )
	//if( (INT32)( ( m_r[ INS_RS( m_op ) ] ^ m_r[ INS_RT( m_op ) ] ) & ( m_r[ INS_RS( m_op ) ] ^ result ) ) < 0 )
	//if ( (s32) ( ( r->GPR [ i.Rs ].s ^ r->GPR [ i.Rt ].s ) & ( r->GPR [ i.Rs ].s ^ temp ) ) < 0 )
	//if ( temp < -0x80000000LL || temp > 0x7fffffffLL )
	if ( ( ( r->GPR [ i.Rs ].sw0 ^ r->GPR [ i.Rt ].sw0 ) & ( r->GPR [ i.Rs ].sw0 ^ temp ) ) < 0 )
	{
		// overflow
		cout << "\nhps2x64: Execute::SUB generated an overflow exception @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << "\n";
		r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
		
#if defined INLINE_DEBUG_SUB || defined INLINE_DEBUG_R5900
		debug << ";  INT";
#endif
	}
	else
	{
		// it's cool - we can do the sub and store the result to register
		r->GPR [ i.Rd ].s = temp;
	}
	
#if defined INLINE_DEBUG_SUB || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}




void Execute::SYSCALL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SYSCALL || defined INLINE_DEBUG_R5900
	debug << "\r\nBefore:" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "\r\n" << hex << "Status=" << r->CPR0.Regs [ 12 ] << " Cause=" << r->CPR0.Regs [ 13 ] << " a0=" << r->GPR [ 4 ].u << " a1=" << r->GPR [ 5 ].u;
	debug << " r1=" << r->GPR [ 1 ].u;
	debug << " r2=" << r->GPR [ 2 ].u;
	debug << " r3=" << r->GPR [ 3 ].u;
#endif
	
#ifdef ENABLE_R5900_BRANCH_PREDICTION
	r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif


	r->ProcessSynchronousInterrupt ( Cpu::EXC_SYSCALL );


#if defined INLINE_DEBUG_SYSCALL || defined INLINE_DEBUG_R5900
	debug << "\r\nAfter:" << hex << setw( 8 ) << r->PC << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << i.Value;
	debug << "\r\n" << hex << "Status=" << r->CPR0.Regs [ 12 ] << " Cause=" << r->CPR0.Regs [ 13 ] << " a0=" << r->GPR [ 4 ].u << " a1=" << r->GPR [ 5 ].u;
	debug << " r1=" << r->GPR [ 1 ].u;
	debug << " r2=" << r->GPR [ 2 ].u;
	debug << " r3=" << r->GPR [ 3 ].u;
#endif
}

void Execute::BREAK ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BREAK || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif
#if defined INLINE_DEBUG_BREAK_EXCEPTION
	debug << "\r\nhps2x64: Execute::BREAK generated an exception @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << "\r\n";
#endif
	
	cout << "\nhps2x64: Execute::BREAK generated an exception @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << "\n";

#ifdef ENABLE_R5900_BRANCH_PREDICTION
	r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif

	r->ProcessSynchronousInterrupt ( Cpu::EXC_BP );
	
	// say to stop if we are debugging
	Cpu::DebugStatus.Stop = true;
	Cpu::DebugStatus.Done = true;
}

void Execute::Invalid ( Instruction::Format i )
{
#if defined INLINE_DEBUG_INVALID || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	cout << "\nhps2x64 NOTE: Invalid Instruction @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << " Instruction=" << i.Value << " LastPC=" << r->LastPC << "\n";
	r->ProcessSynchronousInterrupt ( Cpu::EXC_RI );
}





void Execute::MFC0 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MFC0 || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: CPR[0,rd] = " << r->CPR0.Regs [ i.Rd ];
#endif

	// MFCz rt, rd
	// 1 instruction delay?
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = r->CPR0.Regs [ i.Rd ];
	//r->DelaySlot0.cb = r->_cb_FC;
	//r->Status.DelaySlot_Valid |= 0x1;
	
	// no delay slot on R5900?
	r->GPR [ i.Rt ].sq0 = (s32) r->Read_MFC0 ( i.Rd );	//r->CPR0.Regs [ i.Rd ];
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->CPR0.Regs [ i.Rd ] )
#endif
}


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


void Execute::MTC0 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MTC0 || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// MTCz rt, rd
	// 1 instruction delay?
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = r->GPR [ i.Rt ].u;
	//r->DelaySlot0.cb = r->_cb_MTC0;
	//r->Status.DelaySlot_Valid |= 0x1;
	
	// no delay slot on R5900?
	//r->CPR0.Regs [ i.Rd ] = r->GPR [ i.Rt ].sw0;
	r->Write_MTC0 ( i.Rd, r->GPR [ i.Rt ].sw0 );
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].sw0 )
#endif
}


void Execute::MTC0_Callback ( Cpu* r )
{
	r->Write_MTC0 ( r->DelaySlot1.Instruction.Rd, r->DelaySlot1.Data );
	
	// clear delay slot
	r->DelaySlot1.Value = 0;
}




void Execute::MTC2_Callback ( Cpu* r )
{
	//r->COP2.Write_MTC ( r->DelaySlot1.Instruction.Rd, r->DelaySlot1.Data );
	
	// clear delay slot
	r->DelaySlot1.Value = 0;
}



void Execute::CFC2_I ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CFC2 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: CPC[2,rd] = " << VU0::_VU0->vi [ i.Rd ].u;
#endif
	
	// CFCz rt, rd
	// no delay slot on R5900?
	
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
	{
		//r->GPR [ i.Rt ].sq0 = (s32) ( r->CPC2 [ i.Rd ] & 0xffff );
		//r->GPR [ i.Rt ].uq0 = VU0::_VU0->vi [ i.Rd ].u;
		r->GPR [ i.Rt ].sq0 = (s32) VU0::_VU0->Read_CFC ( i.Rd );
	}
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( VU0::_VU0->vi [ i.Rd ].u )
#endif
}



void Execute::CTC2_I ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CTC2 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// CTCz rt, rd
	// no delay slot on R5900?
	
	// ***todo*** this instruction can have interlock, so need to check I bit
	
	// needs to stop if either micro-subroutine is running or M-bit is set
	if ( VU0::_VU0->VifRegs.STAT.VEW || VU0::_VU0->CurInstHI.M )
	{
		// vu#0 is running //
		// or m-bit is set
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
	{
		//r->CPC2 [ i.Rd ] = ( r->GPR [ i.Rt ].sw0 & 0xffff );
		//VU0::_VU0->vi [ i.Rd ].u = r->GPR [ i.Rt ].uw0;
		VU0::_VU0->Write_CTC ( i.Rd, r->GPR [ i.Rt ].uw0 );
	}

#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].uw0 )
#endif
}


void Execute::CFC2_NI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CFC2 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: CPC[2,rd] = " << VU0::_VU0->vi [ i.Rd ].u;
#endif
	
	// CFCz rt, rd
	// no delay slot on R5900?
	//r->GPR [ i.Rt ].sq0 = (s32) ( r->CPC2 [ i.Rd ] & 0xffff );
	//r->GPR [ i.Rt ].uq0 = VU0::_VU0->vi [ i.Rd ].u;
	r->GPR [ i.Rt ].sq0 = (s32) VU0::_VU0->Read_CFC ( i.Rd );
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( VU0::_VU0->vi [ i.Rd ].u )
#endif
}



void Execute::CTC2_NI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CTC2 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// CTCz rt, rd
	// no delay slot on R5900?
	
	// ***todo*** this instruction can have interlock, so need to check I bit
	
	//r->CPC2 [ i.Rd ] = ( r->GPR [ i.Rt ].sw0 & 0xffff );
	//VU0::_VU0->vi [ i.Rd ].u = r->GPR [ i.Rt ].uw0;
	VU0::_VU0->Write_CTC ( i.Rd, r->GPR [ i.Rt ].uw0 );

#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].uw0 )
#endif
}


void Execute::CTC2_Callback ( Cpu* r )
{
	//r->COP2.Write_CTC ( r->DelaySlot1.Instruction.Rd, r->DelaySlot1.Data );
	
	// clear delay slot
	r->DelaySlot1.Value = 0;
}



// Load/Store - will need to use address translation to get physical addresses when needed

//////////////////////////////////////////////////////////////////////////
// store instructions

// store instructions
void Execute::SB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SB || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif
	
	u64 *pMemPtr64;
	
	// SB rt, offset(base)
	
	// step 1: check if storing to data cache
	//u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	u32 StoreAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
	// clear top 3 bits since there is no data cache for caching stores
	// don't clear top 3 bits since scratchpad is at 0x70000000
	//StoreAddress &= 0x1fffffff;


#ifdef ENABLE_R5900_DCACHE_SB
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( StoreAddress ) )
	{
		pMemPtr64 = handle_cached_store ( StoreAddress );
		
		// store data to cache-line
		((s8*)pMemPtr64) [ StoreAddress & 0x3f ] = r->GPR [ i.Rt ].sb0;
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->Bus->Write_t<0xff> ( StoreAddress, r->GPR [ i.Rt ].uq0 );
		
		handle_uncached_store ();
	}

#else

	
	// ***todo*** perform store of byte
	//r->Bus->Write ( StoreAddress, r->GPR [ i.Rt ].uq0, 0xff );
	r->Bus->Write_t<0xff> ( StoreAddress, r->GPR [ i.Rt ].uq0 );
	
	
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
#if defined INLINE_DEBUG_SH || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif
	
	u64 *pMemPtr64;
	
	// SH rt, offset(base)
	
	// step 1: check if storing to data cache
	//u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	u32 StoreAddress = r->GPR [ i.Base ].sw0 + i.sOffset;

	// *** testing *** alert if load is from unaligned address
	if ( StoreAddress & 0x1 )
	{
#if defined INLINE_DEBUG_STORE_UNALIGNED
		debug << "\r\nhps2x64 ALERT: StoreAddress is unaligned for SH @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
#endif

		cout << "\nhps2x64 ALERT: StoreAddress is unaligned for SH @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADES );
		return;
	}
	
	// clear top 3 bits since there is no data cache for caching stores
	// don't clear top 3 bits since scratchpad is at 0x70000000
	//StoreAddress &= 0x1fffffff;
	
#ifdef ENABLE_R5900_DCACHE_SH
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( StoreAddress ) )
	{
		pMemPtr64 = handle_cached_store ( StoreAddress );
		
		// store data to cache-line
		((s16*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 1 ] = r->GPR [ i.Rt ].sh0;
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->Bus->Write_t<0xffff> ( StoreAddress, r->GPR [ i.Rt ].uq0 );
		
		handle_uncached_store ();
	}

#else
	
	// ***todo*** perform store of halfword
	//r->Bus->Write ( StoreAddress, r->GPR [ i.Rt ].uq0, 0xffff );
	r->Bus->Write_t<0xffff> ( StoreAddress, r->GPR [ i.Rt ].uq0 );
	
	

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
#if defined INLINE_DEBUG_SW || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif
	
	// SW rt, offset(base)
	u32 StoreAddress;
	u64 *pMemPtr64;
	
	// check if storing to data cache
	//StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	StoreAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
	// *** testing *** alert if load is from unaligned address
	if ( StoreAddress & 0x3 )
	{
#if defined INLINE_DEBUG_STORE_UNALIGNED
		debug << "\r\nhps2x64 ALERT: StoreAddress is unaligned for SW @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
#endif

		cout << "\nhps2x64 ALERT: StoreAddress is unaligned for SW @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADES );
		return;
	}
	
	// clear top 3 bits since there is no data cache for caching stores
	// don't clear top 3 bits since scratchpad is at 0x70000000
	//StoreAddress &= 0x1fffffff;
	
#ifdef ENABLE_R5900_DCACHE_SW
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( StoreAddress ) )
	{
		pMemPtr64 = handle_cached_store ( StoreAddress );

		// store data to cache-line
		((s32*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 2 ] = r->GPR [ i.Rt ].sw0;
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->Bus->Write_t<0xffffffff> ( StoreAddress, r->GPR [ i.Rt ].uq0 );
		
		handle_uncached_store ();
	}

#else
	
	// ***todo*** perform store of word
	//r->Bus->Write ( StoreAddress, r->GPR [ i.Rt ].uq0, 0xffffffffULL );
	r->Bus->Write_t<0xffffffff> ( StoreAddress, r->GPR [ i.Rt ].uq0 );
	
	

	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
	
#endif
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].uw0 )
#endif
}



void Execute::SWL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SWL || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif

	static const u32 c_Mask = 0xffffffff;
	u32 Type, Offset;
	u32 Value1, Value2;
	u64 *pMemPtr64;

	// SWL rt, offset(base)
	
	// step 1: check if storing to data cache
	//u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	u32 StoreAddress = r->GPR [ i.Base ].sw0 + i.sOffset;

	// clear top 3 bits since there is no data cache for caching stores
	//StoreAddress &= 0x1fffffff;
	
#ifdef ENABLE_R5900_DCACHE_SWL
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( StoreAddress ) )
	{
		pMemPtr64 = handle_cached_store ( StoreAddress );

		// store data to cache-line
		Value1 = r->GPR [ i.Rt ].uw0 >> ( ( 3 - ( StoreAddress & 3 ) ) << 3 );
		Value2 = ((s32*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 2 ] & ~( 0xffffffffULL >> ( ( 3 - ( StoreAddress & 3 ) ) << 3 ) );
		((s32*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 2 ] = Value1 | Value2;
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->Bus->Write ( StoreAddress & ~3, r->GPR [ i.Rt ].uw0 >> ( ( 3 - ( StoreAddress & 3 ) ) << 3 ), 0xffffffffULL >> ( ( 3 - ( StoreAddress & 3 ) ) << 3 ) );
		
		handle_uncached_store ();
	}

#else
	
	// ***todo*** perform store SWL
	r->Bus->Write ( StoreAddress & ~3, r->GPR [ i.Rt ].uw0 >> ( ( 3 - ( StoreAddress & 3 ) ) << 3 ), 0xffffffffULL >> ( ( 3 - ( StoreAddress & 3 ) ) << 3 ) );
	

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
#if defined INLINE_DEBUG_SWR || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif

	static const u32 c_Mask = 0xffffffff;
	u32 Type, Offset;
	u32 Value1, Value2;
	u64 *pMemPtr64;
	
	// SWR rt, offset(base)
	
	// step 1: check if storing to data cache
	//u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	u32 StoreAddress = r->GPR [ i.Base ].sw0 + i.sOffset;

	// clear top 3 bits since there is no data cache for caching stores
	//StoreAddress &= 0x1fffffff;
	
#ifdef ENABLE_R5900_DCACHE_SWR
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( StoreAddress ) )
	{
		pMemPtr64 = handle_cached_store ( StoreAddress );

		// store data to cache-line
		Value1 = r->GPR [ i.Rt ].uw0 << ( ( StoreAddress & 3 ) << 3 );
		Value2 = ((s32*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 2 ] & ~( 0xffffffffULL << ( ( StoreAddress & 3 ) << 3 ) );
		((s32*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 2 ] = Value1 | Value2;
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->Bus->Write ( StoreAddress & ~3, r->GPR [ i.Rt ].uw0 << ( ( StoreAddress & 3 ) << 3 ), 0xffffffffULL << ( ( StoreAddress & 3 ) << 3 ) );
		
		handle_uncached_store ();
	}

#else
	
	// ***todo*** perform store SWR
	r->Bus->Write ( StoreAddress & ~3, r->GPR [ i.Rt ].uw0 << ( ( StoreAddress & 3 ) << 3 ), 0xffffffffULL << ( ( StoreAddress & 3 ) << 3 ) );
	
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].u )
#endif
}

u64* Execute::handle_cached_load ( u32 LoadAddress, u32 BaseRegister )
{
	u32 ulIndex;
	u32 PFN;
	u64 Latency;
	u64 *pMemPtr64;
	
#ifdef INLINE_DEBUG_DCACHE
	debug << "+$";
#endif

#ifdef DCACHE_FORCE_MEMPTR
		pMemPtr64 = (u64*) r->Bus->GetIMemPtr( LoadAddress & 0x1fffffc0 );
#else
		// determine if it is a cache-hit
		ulIndex = r->DCache.isCacheHit_Index ( LoadAddress );
		
		if ( ulIndex == -1 )
		{
#ifdef INLINE_DEBUG_DCACHE
	debug << "+MISS";
#endif
			// cache-miss -> reload cache-line //
			
			// get the next index to be replaced
			ulIndex = r->DCache.Get_NextIndex ( LoadAddress );

#ifdef ENABLE_DCACHE_TIMING_LOAD
			// check if cpu is already accessing bus -> blocking load
			if ( r->CycleCount < r->LoadFromBus_BusyUntilCycle )
			{
				// blocking load //
				
#ifdef ENABLE_BUS_SIMULATION_CACHE_LOAD
				r->CycleCount = r->LoadFromBus_BusyUntilCycle + r->Bus->c_iRAM_Read_Latency;
				if ( r->CycleCount < r->Bus->BusyUntil_Cycle ) r->CycleCount = r->Bus->BusyUntil_Cycle;
				r->CycleCount += r->c_ullCacheRefill_CycleTime;
				r->Bus->BusyUntil_Cycle = r->CycleCount;
#else
				// this is a different cache-line or the other access is from a register/etc
				// otherwise, it would appear to be a cache-hit
				r->CycleCount = r->LoadFromBus_BusyUntilCycle + r->Bus->c_iRAM_Read_Latency + r->c_ullCacheRefill_CycleTime;
#endif
			}
			else
			{
				// for GPR, this is non-blocking load //
#ifdef ENABLE_BUS_SIMULATION_CACHE_LOAD
				r->LoadFromBus_BusyUntilCycle = r->CycleCount + r->Bus->c_iRAM_Read_Latency;
				if ( r->LoadFromBus_BusyUntilCycle < r->Bus->BusyUntil_Cycle ) r->LoadFromBus_BusyUntilCycle = r->Bus->BusyUntil_Cycle;
				r->LoadFromBus_BusyUntilCycle += r->c_ullCacheRefill_CycleTime;
				r->Bus->BusyUntil_Cycle = r->LoadFromBus_BusyUntilCycle;
#else
				r->LoadFromBus_BusyUntilCycle = r->CycleCount + r->Bus->c_iRAM_Read_Latency + r->c_ullCacheRefill_CycleTime;
#endif
				
				// cache-line is busy loading from bus during that time too
				r->RefillDCache_BusyUntilCycle [ ulIndex ] = r->LoadFromBus_BusyUntilCycle;
				
#ifdef ENABLE_GPR_REGISTER_TIMING_LOAD
				// also need to know how long before data available for register
				// the extra code involved here doesn't appear to work well
				r->ullReg_BusyUntilCycle [ BaseRegister ] = r->LoadFromBus_BusyUntilCycle;
#endif
			}
#endif
			
			// get a pointer into memory
			
			// reload cache-line
			
			
			// if cache is dirty and valid, then write back cache-line
			if ( r->DCache.Get_Dirty( ulIndex ) && r->DCache.Get_Valid( ulIndex ) )
			{
#ifdef INLINE_DEBUG_DCACHE
	debug << "+WB";
#endif
				// *todo?* add the time needed to write-back modified cache-line (at least 4 cpu cycles)
				
				// get pointer into memory for data being replaced
				PFN = r->DCache.Get_PFN ( ulIndex );
				//pMemPtr64 = (u64*) r->Bus->GetIMemPtr ( PFN );
				pMemPtr64 = (u64*) & r->Bus->MainMemory.b8 [ PFN & r->Bus->MainMemory_Mask ];
				
//#ifdef ENABLE_INVALID_ARRAY
				r->Bus->InvalidArray.b8 [ ( PFN & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
//#endif

				// write-back cache line
				r->DCache.WriteBackCache( ulIndex, pMemPtr64 );
				
			}
			
			// get pointer into memory for data to load into cache
			pMemPtr64 = (u64*) r->Bus->GetIMemPtr( LoadAddress & 0x1fffffc0 );
			
			// load data from cache-line or from memory
			r->DCache.ReloadCache ( LoadAddress, pMemPtr64 );
		}
		else
		{
#ifdef INLINE_DEBUG_DCACHE
	debug << "+HIT";
#endif
			// cache-hit -> check if data is ready and load data from cache-line //
			
#ifdef ENABLE_DCACHE_TIMING_LOAD
			// check time that cache-line is busy -> if busy, then actually a cache-miss and blocking??
			if ( r->CycleCount < r->RefillDCache_BusyUntilCycle [ ulIndex ] )
			{
				// wait until the cache-line is filled from memory
				r->CycleCount = r->RefillDCache_BusyUntilCycle [ ulIndex ];
			}
#endif

#ifdef DCACHE_READ_MEMORY
			pMemPtr64 = (u64*) r->Bus->GetIMemPtr( LoadAddress & 0x1fffffc0 );
#else
			// load data from the cache-line (not from memory)
			pMemPtr64 = (u64*) r->DCache.Get_CacheLinePtr( ulIndex );
#endif
		}
#endif


#ifdef DCACHE_ALWAYS_MARK_DIRTY
		r->DCache.Set_Dirty( ulIndex );
#endif

	return pMemPtr64;
}

// call this after the load, because it uses Bus->GetLatency()
void Execute::handle_uncached_load ( u32 BaseRegister )
{
#ifdef INLINE_DEBUG_DCACHE
	debug << "-$";
#endif

#ifdef ENABLE_DCACHE_TIMING_LOAD
		// check if cpu is already accessing bus -> blocking load
		if ( r->CycleCount < r->LoadFromBus_BusyUntilCycle )
		{
			r->CycleCount = r->LoadFromBus_BusyUntilCycle + r->Bus->GetLatency();
		}
		else
		{
			// set time that CPU will be accessing bus -> any other accesses are blocking ??
			r->LoadFromBus_BusyUntilCycle = r->CycleCount + r->Bus->GetLatency();
			
#ifdef ENABLE_GPR_REGISTER_TIMING
			// also need to know how long before data available for register
			// the extra code involved here doesn't appear to work well
			r->ullReg_BusyUntilCycle [ BaseRegister ] = r->LoadFromBus_BusyUntilCycle;
#endif
		}
#endif
}

u64* Execute::handle_cached_store ( u32 StoreAddress )
{
	u32 ulIndex;
	u32 PFN;
	u64 Latency;
	u64 *pMemPtr64;
	
#ifdef INLINE_DEBUG_DCACHE
	debug << "+$";
#endif

#ifdef DCACHE_FORCE_MEMPTR
		pMemPtr64 = (u64*) r->Bus->GetIMemPtr( StoreAddress & 0x1fffffc0 );
		r->Bus->InvalidArray.b8 [ ( StoreAddress & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
#else
		// determine if it is a cache-hit
		ulIndex = r->DCache.isCacheHit_Index ( StoreAddress );
		
		if ( ulIndex == -1 )
		{
#ifdef INLINE_DEBUG_DCACHE
	debug << "+MISS";
#endif
			// cache-miss -> reload cache-line //
			
			// get the next index to be replaced
			ulIndex = r->DCache.Get_NextIndex ( StoreAddress );
			
#ifdef ENABLE_DCACHE_TIMING_STORE
			// check if cpu is already accessing bus -> blocking load
			if ( r->CycleCount < r->LoadFromBus_BusyUntilCycle )
			{
				// blocking load //
				
				// this is a different cache-line or the other access is from a register/etc
				// otherwise, it would appear to be a cache-hit
#ifdef ENABLE_BUS_SIMULATION_CACHE_STORE
				r->CycleCount = r->LoadFromBus_BusyUntilCycle + r->Bus->c_iRAM_Read_Latency;
				if ( r->CycleCount < r->Bus->BusyUntil_Cycle ) r->CycleCount = r->Bus->BusyUntil_Cycle;
				r->CycleCount += r->c_ullCacheRefill_CycleTime;
				r->Bus->BusyUntil_Cycle = r->CycleCount;
#else
				r->CycleCount = r->LoadFromBus_BusyUntilCycle + r->Bus->c_iRAM_Read_Latency + r->c_ullCacheRefill_CycleTime;
#endif
			}
			else
			{
				// for GPR, this is non-blocking load //
#ifdef ENABLE_BUS_SIMULATION_CACHE_STORE
				r->LoadFromBus_BusyUntilCycle = r->CycleCount + r->Bus->c_iRAM_Read_Latency;
				if ( r->LoadFromBus_BusyUntilCycle < r->Bus->BusyUntil_Cycle ) r->LoadFromBus_BusyUntilCycle = r->Bus->BusyUntil_Cycle;
				r->LoadFromBus_BusyUntilCycle += r->c_ullCacheRefill_CycleTime;
				r->Bus->BusyUntil_Cycle = r->LoadFromBus_BusyUntilCycle;
#else
				r->LoadFromBus_BusyUntilCycle = r->CycleCount + r->Bus->c_iRAM_Read_Latency + r->c_ullCacheRefill_CycleTime;
#endif
				
				// cache-line is busy loading from bus during that time too
				r->RefillDCache_BusyUntilCycle [ ulIndex ] = r->LoadFromBus_BusyUntilCycle;
				
#ifdef ENABLE_GPR_REGISTER_TIMING_STORE
				// also need to know how long before data available for register
				// but... no register is waiting to be accessed on a store
				r->ullReg_BusyUntilCycle [ BaseRegister ] = r->LoadFromBus_BusyUntilCycle;
#endif
			}
#endif
			
			// get a pointer into memory
			
			// reload cache-line
			
			// if cache is dirty and valid, then write back cache-line
			if ( r->DCache.Get_Dirty( ulIndex ) && r->DCache.Get_Valid( ulIndex ) )
			{
#ifdef INLINE_DEBUG_DCACHE
	debug << "+WB";
#endif
				// *todo?* add the time needed to write-back modified cache-line (at least 4 cpu cycles)
				
				// get pointer into memory for data being replaced
				PFN = r->DCache.Get_PFN ( ulIndex );
				//pMemPtr64 = (u64*) r->Bus->GetIMemPtr ( PFN );
				//pMemPtr64 = (u64*) & r->Bus->MainMemory.b8 [ PFN & 0x01ffffff ];
				pMemPtr64 = (u64*) & r->Bus->MainMemory.b8 [ ( PFN & r->Bus->MainMemory_Mask ) & 0x1fffffc0 ];
				
//#ifdef ENABLE_INVALID_ARRAY
				r->Bus->InvalidArray.b8 [ ( PFN & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
//#endif

				// write-back cache line
				r->DCache.WriteBackCache( ulIndex, pMemPtr64 );
			}
			
			// get pointer into memory for data to load into cache
			pMemPtr64 = (u64*) r->Bus->GetIMemPtr( StoreAddress & 0x1fffffc0 );
			
			// load data from cache-line or from memory
			r->DCache.ReloadCache ( StoreAddress, pMemPtr64 );
		}
		else
		{
#ifdef INLINE_DEBUG_DCACHE
	debug << "+HIT";
#endif
			// cache-hit -> check if data is ready and load data from cache-line //
			
#ifdef ENABLE_DCACHE_TIMING_STORE
			// check time that cache-line is busy -> if busy, then actually a cache-miss and blocking??
			if ( r->CycleCount < r->RefillDCache_BusyUntilCycle [ ulIndex ] )
			{
				// wait until the cache-line is filled from memory
				r->CycleCount = r->RefillDCache_BusyUntilCycle [ ulIndex ];
			}
#endif
			
		}
		
		// cache-line is being modified, so it is dirty/modified
		r->DCache.Set_Dirty( ulIndex );
		
		//r->Bus->InvalidArray.b8 [ ( StoreAddress & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
		
#ifdef DCACHE_WRITE_MEMORY
		pMemPtr64 = (u64*) r->Bus->GetIMemPtr( StoreAddress & 0x1fffffc0 );
#else
		// load data from the cache-line (not from memory)
		pMemPtr64 = (u64*) r->DCache.Get_CacheLinePtr( ulIndex );
#endif

#endif

	return pMemPtr64;
}

void Execute::handle_uncached_store ()
{
#ifdef INLINE_DEBUG_DCACHE
	debug << "-$";
#endif
	
#ifdef ENABLE_DCACHE_TIMING_STORE
		// check if cpu is already accessing bus -> blocking load
		// this is wrong.. there is a store buffer
		//if ( r->CycleCount < r->LoadFromBus_BusyUntilCycle )
		//{
		//	r->CycleCount = r->LoadFromBus_BusyUntilCycle + r->Bus->GetLatency();
		//}
		//else
		//{
		//	// set time that CPU will be accessing bus -> any other accesses are blocking ??
		//	r->LoadFromBus_BusyUntilCycle = r->CycleCount + r->Bus->GetLatency();
		//	
		//	// also need to know how long before data available for register
		//	// but.. no register is waiting to be accessed when storing
		//	//r->ullReg_BusyUntilCycle [ BaseRegister ] = r->LoadFromBus_BusyUntilCycle;
		//}
#endif
}



u64* Execute::handle_cached_load_blocking ( u32 LoadAddress )
{
	u32 ulIndex;
	u32 PFN;
	u64 Latency;
	u64 *pMemPtr64;
	
#ifdef INLINE_DEBUG_DCACHE
	debug << "+$";
#endif

#ifdef DCACHE_FORCE_MEMPTR
		pMemPtr64 = (u64*) r->Bus->GetIMemPtr( LoadAddress & 0x1fffffc0 );
#else
		// determine if it is a cache-hit
		ulIndex = r->DCache.isCacheHit_Index ( LoadAddress );
		
		if ( ulIndex == -1 )
		{
#ifdef INLINE_DEBUG_DCACHE
	debug << "+MISS";
#endif
			// cache-miss -> reload cache-line //
			
			// always a blocking load ? //
			
			// get the next index to be replaced
			ulIndex = r->DCache.Get_NextIndex ( LoadAddress );

#ifdef ENABLE_DCACHE_TIMING_LOAD
			// check if cpu is already accessing bus -> blocking load
			if ( r->CycleCount < r->LoadFromBus_BusyUntilCycle )
			{
				// blocking load //
				
				// this is a different cache-line or the other access is from a register/etc
				// otherwise, it would appear to be a cache-hit
				r->CycleCount = r->LoadFromBus_BusyUntilCycle;
			}
			
			// blocking load
#ifdef ENABLE_BUS_SIMULATION_CACHE_LOAD
			r->CycleCount += r->Bus->c_iRAM_Read_Latency;
			if ( r->CycleCount < r->Bus->BusyUntil_Cycle ) r->CycleCount = r->Bus->BusyUntil_Cycle;
			r->CycleCount += r->c_ullCacheRefill_CycleTime;
			r->Bus->BusyUntil_Cycle = r->CycleCount;
#else
			r->CycleCount += r->Bus->c_iRAM_Read_Latency + r->c_ullCacheRefill_CycleTime;
#endif

#endif
			// get a pointer into memory
			
			// reload cache-line
			
			// if cache is dirty and valid, then write back cache-line
			if ( r->DCache.Get_Dirty( ulIndex ) && r->DCache.Get_Valid( ulIndex ) )
			{
#ifdef INLINE_DEBUG_DCACHE
	debug << "+WB";
#endif
				// *todo?* add the time needed to write-back modified cache-line (at least 4 cpu cycles)
				
				// get pointer into memory for data being replaced
				PFN = r->DCache.Get_PFN ( ulIndex );
				//pMemPtr64 = (u64*) r->Bus->GetIMemPtr ( PFN );
				pMemPtr64 = (u64*) & r->Bus->MainMemory.b8 [ PFN & 0x01ffffff ];
				
//#ifdef ENABLE_INVALID_ARRAY
				r->Bus->InvalidArray.b8 [ ( PFN & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
//#endif

				// write-back cache line
				r->DCache.WriteBackCache( ulIndex, pMemPtr64 );
			}
			
			// get pointer into memory for data to load into cache
			pMemPtr64 = (u64*) r->Bus->GetIMemPtr( LoadAddress & 0x1fffffc0 );
			
			// load data from cache-line or from memory
			r->DCache.ReloadCache ( LoadAddress, pMemPtr64 );
		}
		else
		{
#ifdef INLINE_DEBUG_DCACHE
	debug << "+HIT";
#endif
			// cache-hit -> check if data is ready and load data from cache-line //
			
#ifdef ENABLE_DCACHE_TIMING_LOAD
			// check time that cache-line is busy -> if busy, then actually a cache-miss and blocking??
			if ( r->CycleCount < r->RefillDCache_BusyUntilCycle [ ulIndex ] )
			{
				// wait until the cache-line is filled from memory
				r->CycleCount = r->RefillDCache_BusyUntilCycle [ ulIndex ];
			}
#endif
			
#ifdef DCACHE_READ_MEMORY
			pMemPtr64 = (u64*) r->Bus->GetIMemPtr( LoadAddress & 0x1fffffc0 );
#else
			// load data from the cache-line (not from memory)
			pMemPtr64 = (u64*) r->DCache.Get_CacheLinePtr( ulIndex );
#endif
		}
#endif

#ifdef DCACHE_ALWAYS_MARK_DIRTY
		r->DCache.Set_Dirty( ulIndex );
#endif

	return pMemPtr64;
}

void Execute::handle_uncached_load_blocking ()
{
#ifdef INLINE_DEBUG_DCACHE
	debug << "-$";
#endif

#ifdef ENABLE_DCACHE_TIMING_LOAD
		// check if cpu is already accessing bus -> blocking load
		if ( r->CycleCount < r->LoadFromBus_BusyUntilCycle )
		{
			r->CycleCount = r->LoadFromBus_BusyUntilCycle;
		}
		
		// blocking load
		r->CycleCount += r->Bus->GetLatency();
#endif
}


u64* Execute::handle_cached_store_blocking ( u32 StoreAddress )
{
	u32 ulIndex;
	u32 PFN;
	u64 Latency;
	u64 *pMemPtr64;
	
#ifdef INLINE_DEBUG_DCACHE
	debug << "+$";
#endif

#ifdef DCACHE_FORCE_MEMPTR
		pMemPtr64 = (u64*) r->Bus->GetIMemPtr( StoreAddress & 0x1fffffc0 );
		r->Bus->InvalidArray.b8 [ ( StoreAddress & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
#else
		// determine if it is a cache-hit
		ulIndex = r->DCache.isCacheHit_Index ( StoreAddress );
		
		if ( ulIndex == -1 )
		{
#ifdef INLINE_DEBUG_DCACHE
	debug << "+MISS";
#endif
			// cache-miss -> reload cache-line //
			
			// get the next index to be replaced
			ulIndex = r->DCache.Get_NextIndex ( StoreAddress );
			
			// always a blocking load ? //
			
#ifdef ENABLE_DCACHE_TIMING_STORE
			// check if cpu is already accessing bus -> blocking load
			if ( r->CycleCount < r->LoadFromBus_BusyUntilCycle )
			{
				// blocking load //
				
				// this is a different cache-line or the other access is from a register/etc
				// otherwise, it would appear to be a cache-hit
				r->CycleCount = r->LoadFromBus_BusyUntilCycle;
			}
			
			// blocking load
#ifdef ENABLE_BUS_SIMULATION_CACHE_STORE
			r->CycleCount += r->Bus->c_iRAM_Read_Latency;
			if ( r->CycleCount < r->Bus->BusyUntil_Cycle ) r->CycleCount = r->Bus->BusyUntil_Cycle;
			r->CycleCount += r->c_ullCacheRefill_CycleTime;
			r->Bus->BusyUntil_Cycle = r->CycleCount;
#else
			r->CycleCount += r->Bus->c_iRAM_Read_Latency + r->c_ullCacheRefill_CycleTime;
#endif

#endif	// ENABLE_DCACHE_TIMING_STORE
			
			// get a pointer into memory
			
			// reload cache-line
			
			// if cache is dirty and valid, then write back cache-line
			if ( r->DCache.Get_Dirty( ulIndex ) && r->DCache.Get_Valid( ulIndex ) )
			{
#ifdef INLINE_DEBUG_DCACHE
	debug << "+WB";
#endif
				// *todo?* add the time needed to write-back modified cache-line (at least 4 cpu cycles)
				
				// get pointer into memory for data being replaced
				PFN = r->DCache.Get_PFN ( ulIndex );
				//pMemPtr64 = (u64*) r->Bus->GetIMemPtr ( PFN );
				pMemPtr64 = (u64*) & r->Bus->MainMemory.b8 [ PFN & 0x01ffffff ];
				
//#ifdef ENABLE_INVALID_ARRAY
				r->Bus->InvalidArray.b8 [ ( PFN & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
//#endif

				// write-back cache line
				r->DCache.WriteBackCache( ulIndex, pMemPtr64 );
			}
			
			// get pointer into memory for data to load into cache
			pMemPtr64 = (u64*) r->Bus->GetIMemPtr( StoreAddress & 0x1fffffc0 );
			
			// load data from cache-line or from memory
			r->DCache.ReloadCache ( StoreAddress, pMemPtr64 );
		}
		else
		{
#ifdef INLINE_DEBUG_DCACHE
	debug << "+HIT";
#endif
			// cache-hit -> check if data is ready and load data from cache-line //
			
#ifdef ENABLE_DCACHE_TIMING_STORE
			// check time that cache-line is busy -> if busy, then actually a cache-miss and blocking??
			if ( r->CycleCount < r->RefillDCache_BusyUntilCycle [ ulIndex ] )
			{
				// wait until the cache-line is filled from memory
				r->CycleCount = r->RefillDCache_BusyUntilCycle [ ulIndex ];
			}
#endif
			
		}
		
		// cache-line is being modified, so it is dirty/modified
		r->DCache.Set_Dirty( ulIndex );
		
		//r->Bus->InvalidArray.b8 [ ( StoreAddress & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
		
#ifdef DCACHE_WRITE_MEMORY
		pMemPtr64 = (u64*) r->Bus->GetIMemPtr( StoreAddress & 0x1fffffc0 );
#else
		// load data from the cache-line (not from memory)
		pMemPtr64 = (u64*) r->DCache.Get_CacheLinePtr( ulIndex );
#endif

#endif

	return pMemPtr64;
}


void Execute::handle_uncached_store_blocking ()
{
#ifdef INLINE_DEBUG_DCACHE
	debug << "-$";
#endif

#ifdef ENABLE_DCACHE_TIMING_STORE
		// check if cpu is already accessing bus -> blocking load
		// this is wrong.. there's a store buffer
		//if ( r->CycleCount < r->LoadFromBus_BusyUntilCycle )
		//{
		//	r->CycleCount = r->LoadFromBus_BusyUntilCycle;
		//}
		
		// blocking load
		//r->CycleCount += r->Bus->GetLatency();
#endif
}




/////////////////////////////////////////////////
// load instructions

// load instructions with delay slot
void Execute::LB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LB || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// signed load byte from memory
	// LB rt, offset(base)
	
	u32 LoadAddress;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
#ifdef ENABLE_R5900_DCACHE_LB
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load ( LoadAddress, i.Rt );
		
		// load from data cache into register
		r->GPR [ i.Rt ].sq0 = ((s8*)pMemPtr64) [ LoadAddress & 0x3f ];
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->GPR [ i.Rt ].sq0 = (s8) r->Bus->Read_t<0xff> ( LoadAddress );
		
		// call this after the load, because it uses Bus->GetLatency()
		handle_uncached_load ( i.Rt );
	}

#else

	// ***todo*** perform signed load of byte
	//r->GPR [ i.Rt ].sq0 = (s8) r->Bus->Read ( LoadAddress, 0xff );
	r->GPR [ i.Rt ].sq0 = (s8) r->Bus->Read_t<0xff> ( LoadAddress );
	
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;

#endif
	
#if defined INLINE_DEBUG_LB || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].sq0;
#endif
}






void Execute::LH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LH || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// LH rt, offset(base)
	
	u32 LoadAddress;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
	// *** testing *** alert if load is from unaligned address
	if ( LoadAddress & 0x1 )
	{
		cout << "\nhps2x64 ALERT: LoadAddress is unaligned for LH @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
		return;
	}
	
#ifdef ENABLE_R5900_DCACHE_LH
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load ( LoadAddress, i.Rt );
		
		// load from data cache into register
		r->GPR [ i.Rt ].sq0 = ((s16*)pMemPtr64) [ ( LoadAddress & 0x3f ) >> 1 ];
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->GPR [ i.Rt ].sq0 = (s16) r->Bus->Read_t<0xffff> ( LoadAddress );
		
		handle_uncached_load ( i.Rt );
	}

#else
	
	// ***todo*** perform signed load of halfword
	//r->GPR [ i.Rt ].sq0 = (s16) r->Bus->Read ( LoadAddress, 0xffff );
	r->GPR [ i.Rt ].sq0 = (s16) r->Bus->Read_t<0xffff> ( LoadAddress );
	
	
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
	
#endif
	
#if defined INLINE_DEBUG_LH || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].sq0;
#endif
}








void Execute::LW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LW || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// LW rt, offset(base)
	
	u32 LoadAddress;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
	// *** testing *** alert if load is from unaligned address
	if ( LoadAddress & 0x3 )
	{
		cout << "\nhps2x64 ALERT: LoadAddress is unaligned for LW @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		
#if defined INLINE_DEBUG_LW || defined INLINE_DEBUG_R5900
		debug << "\r\nhps2x64 ALERT: LoadAddress is unaligned for LW @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\r\n";
#endif

		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
		return;
	}
	
#ifdef ENABLE_R5900_DCACHE_LW
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load ( LoadAddress, i.Rt );
		
		// load from data cache into register
		r->GPR [ i.Rt ].sq0 = ((s32*)pMemPtr64) [ ( LoadAddress & 0x3f ) >> 2 ];
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->GPR [ i.Rt ].sq0 = (s32) r->Bus->Read_t<0xffffffff> ( LoadAddress );

		handle_uncached_load ( i.Rt );
	}

#else
	
	// ***todo*** perform signed load of word
	//r->GPR [ i.Rt ].sq0 = (s32) r->Bus->Read ( LoadAddress, 0xffffffffULL );
	r->GPR [ i.Rt ].sq0 = (s32) r->Bus->Read_t<0xffffffff> ( LoadAddress );
	
	
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
	
#endif
	
#if defined INLINE_DEBUG_LW || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].sq0;
#endif
}

void Execute::LBU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LBU || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// LBU rt, offset(base)
	
	u32 LoadAddress;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
#ifdef ENABLE_R5900_DCACHE_LBU
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load ( LoadAddress, i.Rt );
		
		// load from data cache into register
		r->GPR [ i.Rt ].uq0 = ((u8*)pMemPtr64) [ LoadAddress & 0x3f ];
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->GPR [ i.Rt ].uq0 = (u8) r->Bus->Read_t<0xff> ( LoadAddress );

		handle_uncached_load ( i.Rt );
	}

#else

	// ***todo*** perform load of unsigned byte
	//r->GPR [ i.Rt ].uq0 = (u8) r->Bus->Read ( LoadAddress, 0xff );
	r->GPR [ i.Rt ].uq0 = (u8) r->Bus->Read_t<0xff> ( LoadAddress );
	
	
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
	
#endif
	
#if defined INLINE_DEBUG_LBU || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].sq0;
#endif
}

void Execute::LHU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LHU || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// LHU rt, offset(base)
	
	u32 LoadAddress;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
	// *** testing *** alert if load is from unaligned address
	if ( LoadAddress & 0x1 )
	{
		cout << "\nhps2x64 ALERT: LoadAddress is unaligned for LHU @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
		return;
	}
	
#ifdef ENABLE_R5900_DCACHE_LHU
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load ( LoadAddress, i.Rt );
		
		// load from data cache into register
		r->GPR [ i.Rt ].uq0 = ((u16*)pMemPtr64) [ ( LoadAddress & 0x3f ) >> 1 ];
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->GPR [ i.Rt ].uq0 = (u16) r->Bus->Read_t<0xffff> ( LoadAddress );
		
		handle_uncached_load ( i.Rt );
	}

#else
	
	// ***todo*** perform unsigned load of halfword
	//r->GPR [ i.Rt ].uq0 = (u16) r->Bus->Read ( LoadAddress, 0xffff );
	r->GPR [ i.Rt ].uq0 = (u16) r->Bus->Read_t<0xffff> ( LoadAddress );
	
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
	
#endif
	
#if defined INLINE_DEBUG_LHU || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].sq0;
#endif
}




// load instructions without load-delay slot
void Execute::LWL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LWL || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif

	// LWL rt, offset(base)
	
	u32 LoadAddress;
	u32 Value, Temp;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
#ifdef ENABLE_R5900_DCACHE_LWL
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load ( LoadAddress, i.Rt );
		
		// load from data cache into register
		Value = ((s32*)pMemPtr64) [ ( LoadAddress & 0x3f ) >> 2 ];
		
		Value <<= ( ( 3 - ( LoadAddress & 3 ) ) << 3 );
		Temp = r->GPR [ i.Rt ].uw0;
		Temp <<= ( ( ( LoadAddress & 3 ) + 1 ) << 3 );
		if ( ( LoadAddress & 3 ) == 3 ) Temp = 0;
		Temp >>= ( ( ( LoadAddress & 3 ) + 1 ) << 3 );
		r->GPR [ i.Rt ].sq0 = (s32) ( Value | Temp );
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		//r->GPR [ i.Rt ].sq0 = (s32) r->Bus->Read_t<0xffffffff> ( LoadAddress );
		Value = r->Bus->Read_t<0xffffffffULL> ( LoadAddress & ~3 );
		
		Value <<= ( ( 3 - ( LoadAddress & 3 ) ) << 3 );
		Temp = r->GPR [ i.Rt ].uw0;
		Temp <<= ( ( ( LoadAddress & 3 ) + 1 ) << 3 );
		if ( ( LoadAddress & 3 ) == 3 ) Temp = 0;
		Temp >>= ( ( ( LoadAddress & 3 ) + 1 ) << 3 );
		r->GPR [ i.Rt ].sq0 = (s32) ( Value | Temp );
		
		handle_uncached_load ( i.Rt );
	}

#else
	
	// ***todo*** perform load LWL
	//Value = r->Bus->Read ( LoadAddress & ~3, 0xffffffffULL );
	Value = r->Bus->Read_t<0xffffffffULL> ( LoadAddress & ~3 );
	
	Value <<= ( ( 3 - ( LoadAddress & 3 ) ) << 3 );
	Temp = r->GPR [ i.Rt ].uw0;
	Temp <<= ( ( ( LoadAddress & 3 ) + 1 ) << 3 );
	if ( ( LoadAddress & 3 ) == 3 ) Temp = 0;
	Temp >>= ( ( ( LoadAddress & 3 ) + 1 ) << 3 );
	r->GPR [ i.Rt ].sq0 = (s32) ( Value | Temp );
	
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
#endif
	
#if defined INLINE_DEBUG_LWL || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].sq0;
#endif
}

void Execute::LWR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LWR || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif

	// LWR rt, offset(base)
	
	u32 LoadAddress;
	u32 Value, Temp;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
#ifdef ENABLE_R5900_DCACHE_LWR
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load ( LoadAddress, i.Rt );
		
		// load from data cache into register
		Value = ((s32*)pMemPtr64) [ ( LoadAddress & 0x3f ) >> 2 ];
		
		Value >>= ( ( LoadAddress & 3 ) << 3 );
		Temp = r->GPR [ i.Rt ].uw0;
		Temp >>= ( ( 4 - ( LoadAddress & 3 ) ) << 3 );
		if ( ( LoadAddress & 3 ) == 0 ) Temp = 0;
		Temp <<= ( ( 4 - ( LoadAddress & 3 ) ) << 3 );
		
		// note: LWR is only sign extended when the full memory value is loaded, which is when ( LoadAddress & 3 ) == 0
		if ( LoadAddress & 3 )
		{
			// NOT sign extended //
			r->GPR [ i.Rt ].uw0 = Value | Temp;
		}
		else
		{
			// sign extended //
			r->GPR [ i.Rt ].sq0 = (s32) ( Value | Temp );
		}
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		Value = r->Bus->Read_t<0xffffffffULL> ( LoadAddress & ~3 );
		
		Value >>= ( ( LoadAddress & 3 ) << 3 );
		Temp = r->GPR [ i.Rt ].uw0;
		Temp >>= ( ( 4 - ( LoadAddress & 3 ) ) << 3 );
		if ( ( LoadAddress & 3 ) == 0 ) Temp = 0;
		Temp <<= ( ( 4 - ( LoadAddress & 3 ) ) << 3 );
		
		// note: LWR is only sign extended when the full memory value is loaded, which is when ( LoadAddress & 3 ) == 0
		if ( LoadAddress & 3 )
		{
			// NOT sign extended //
			r->GPR [ i.Rt ].uw0 = Value | Temp;
		}
		else
		{
			// sign extended //
			r->GPR [ i.Rt ].sq0 = (s32) ( Value | Temp );
		}
		
		handle_uncached_load ( i.Rt );
	}

#else
	
	// ***todo*** perform load LWR
	//Value = r->Bus->Read ( LoadAddress & ~3, 0xffffffffULL );
	Value = r->Bus->Read_t<0xffffffffULL> ( LoadAddress & ~3 );
	
	Value >>= ( ( LoadAddress & 3 ) << 3 );
	Temp = r->GPR [ i.Rt ].uw0;
	Temp >>= ( ( 4 - ( LoadAddress & 3 ) ) << 3 );
	if ( ( LoadAddress & 3 ) == 0 ) Temp = 0;
	Temp <<= ( ( 4 - ( LoadAddress & 3 ) ) << 3 );
	
	// note: LWR is only sign extended when the full memory value is loaded, which is when ( LoadAddress & 3 ) == 0
	if ( LoadAddress & 3 )
	{
		// NOT sign extended //
		r->GPR [ i.Rt ].uw0 = Value | Temp;
	}
	else
	{
		// sign extended //
		r->GPR [ i.Rt ].sq0 = (s32) ( Value | Temp );
	}
	
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
#endif
	
#if defined INLINE_DEBUG_LWR || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].sq0;
#endif
}






//// ***** R5900 INSTRUCTIONS ***** ////

// arithemetic instructions //

void Execute::DADD ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DADD || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	s64 temp;
	
	temp = r->GPR [ i.Rs ].s + r->GPR [ i.Rt ].s;

	// ***todo*** implement overflow exception
	// note: this is similar to something seen in MAME and adapted from something in pcsx2
	if ( ( ( ~( r->GPR [ i.Rs ].s ^ r->GPR [ i.Rt ].s ) ) & ( r->GPR [ i.Rs ].s ^ temp ) ) < 0 )
	{
		// overflow
		cout << "\nhps2x64: Execute::DADD generated an overflow exception @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << "\n";
		r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
		
#if defined INLINE_DEBUG_DADD || defined INLINE_DEBUG_R5900
		debug << ";  INT";
#endif
	}
	else
	{
		// it's cool - we can do the add and store the result to register
		// sign-extend for R5900
		r->GPR [ i.Rd ].s = temp;
	}
	
	// add WITH overflow exception: rd = rs + rt
	//r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u + r->GPR [ i.Rt ].u;
	
#if defined INLINE_DEBUG_DADD || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::DADDI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DADDI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	s64 temp;
	
	// ***todo*** implement overflow exception
	temp = r->GPR [ i.Rs ].s + ( (s64) i.sImmediate );
	
	// ***todo*** implement overflow exception
	// note: this is similar to something seen in MAME and adapted from something in pcsx2
	if ( ( ( ~( r->GPR [ i.Rs ].s ^ ( (s64) i.sImmediate ) ) ) & ( r->GPR [ i.Rs ].s ^ temp ) ) < 0 )
	{
		// overflow
		cout << "\nhps2x64: Execute::DADDI generated an overflow exception @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << "\n";
		r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
		
#if defined INLINE_DEBUG_DADDI || defined INLINE_DEBUG_R5900
		debug << ";  INT";
#endif
	}
	else
	{
		// it's cool - we can do the add and store the result to register
		// sign-extend for R5900
		r->GPR [ i.Rt ].s = temp;
	}
	
	// add immedeate WITH overflow exception: rt = rs + immediate
	//r->GPR [ i.Rt ].s = r->GPR [ i.Rs ].s + ( (s64) i.sImmediate );
	
#if defined INLINE_DEBUG_DADD || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::DADDU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DADDU || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// add without overflow exception: rd = rs + rt
	r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u + r->GPR [ i.Rt ].u;
	
#if defined INLINE_DEBUG_DADDU || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::DADDIU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DADDIU || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif

	// *note* immediate value is sign-extended before the addition is performed

	// add immedeate without overflow exception: rt = rs + immediate
	r->GPR [ i.Rt ].s = r->GPR [ i.Rs ].s + ( (s64) i.sImmediate );
	
#if defined INLINE_DEBUG_DADDIU || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].u;
#endif
}

void Execute::DSUB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DSUB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	s64 temp;

	temp = r->GPR [ i.Rs ].s - r->GPR [ i.Rt ].s;
	
	// ***todo*** implement overflow exception
	if ( ( ( r->GPR [ i.Rs ].s ^ r->GPR [ i.Rt ].s ) & ( r->GPR [ i.Rs ].s ^ temp ) ) < 0 )
	{
		// overflow
		cout << "\nhps2x64: Execute::DSUB generated an overflow exception @ Cycle#" << dec << r->CycleCount << " PC=" << hex << r->PC << "\n";
		r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
		
#if defined INLINE_DEBUG_DSUB || defined INLINE_DEBUG_R5900
		debug << ";  INT";
#endif
	}
	else
	{
		// it's cool - we can do the sub and store the result to register
		r->GPR [ i.Rd ].s = temp;
	}
	
	// subtract WITH overflow exception: rd = rs - rt
	//r->GPR [ i.Rd ].s = r->GPR [ i.Rs ].u - r->GPR [ i.Rt ].u;
	
#if defined INLINE_DEBUG_DSUB || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::DSUBU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DSUBU || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// subtract without overflow exception: rd = rs - rt
	r->GPR [ i.Rd ].s = r->GPR [ i.Rs ].u - r->GPR [ i.Rt ].u;
	
#if defined INLINE_DEBUG_DSUBU || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::DSLL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DSLL || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// shift left logical: rd = rt << shift
	r->GPR [ i.Rd ].u = ( r->GPR [ i.Rt ].u << i.Shift );
	
#if defined INLINE_DEBUG_DSLL || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::DSLL32 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DSLL32 || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// shift left logical: rd = rt << shift
	r->GPR [ i.Rd ].u = ( r->GPR [ i.Rt ].u << ( i.Shift + 32 ) );
	
#if defined INLINE_DEBUG_DSLL32 || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::DSLLV ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DSLLV || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// shift left logical variable: rd = rt << rs
	r->GPR [ i.Rd ].s = r->GPR [ i.Rt ].u << ( r->GPR [ i.Rs ].u & 0x3f );
	
#if defined INLINE_DEBUG_DSLLV || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::DSRA ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DSRA || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// shift right arithmetic: rd = rt >> shift
	r->GPR [ i.Rd ].s = ( r->GPR [ i.Rt ].s >> i.Shift );
	
#if defined INLINE_DEBUG_DSRA || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::DSRA32 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DSRA32 || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// shift right arithmetic: rd = rt >> shift
	r->GPR [ i.Rd ].s = ( r->GPR [ i.Rt ].s >> ( i.Shift + 32 ) );
	
#if defined INLINE_DEBUG_DSRA32 || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::DSRAV ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DSRAV || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// shift right arithmetic variable: rd = rt >> rs
	r->GPR [ i.Rd ].s = ( r->GPR [ i.Rt ].s >> ( r->GPR [ i.Rs ].u & 0x3f ) );
	
#if defined INLINE_DEBUG_DSRAV || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::DSRL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DSRL || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// shift right logical: rd = rt >> shift
	r->GPR [ i.Rd ].s = ( r->GPR [ i.Rt ].u >> i.Shift );
	
#if defined INLINE_DEBUG_DSRL || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::DSRL32 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DSRL32 || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u;
#endif
	
	// shift right logical: rd = rt >> shift
	r->GPR [ i.Rd ].s = ( r->GPR [ i.Rt ].u >> ( i.Shift + 32 ) );
	
#if defined INLINE_DEBUG_DSRL32 || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::DSRLV ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DSRLV || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; rs = " << r->GPR [ i.Rs ].u;
#endif
	
	// shift right logical variable: rd = rt >> rs
	r->GPR [ i.Rd ].s = ( r->GPR [ i.Rt ].u >> ( r->GPR [ i.Rs ].u & 0x3f ) );
	
#if defined INLINE_DEBUG_DSRLV || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}


void Execute::MULT1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MULT1 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	
	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}

	// cycle times are different on ps2 ??
	/*
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
	*/
	
	// multiply signed Lo,Hi = rs * rt
	//r->HiLo.sValue = ((s64) (r->GPR [ i.Rs ].s)) * ((s64) (r->GPR [ i.Rt ].s));
	s64 temp;
	temp = ( (s64) ( r->GPR [ i.Rs ].sw0 ) ) * ( (s64) ( r->GPR [ i.Rt ].sw0 ) );
	r->LO.sq1 = (s32) temp;
	r->HI.sq1 = (s32) ( temp >> 32 );
	
	// R5900 can additionally write to register
	if ( i.Rd )
	{
		r->GPR [ i.Rd ].sq0 = (s32) temp;
	}

#if defined INLINE_DEBUG_MULT1 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "; Output: LO1=" << r->LO.sq1 << "; HI1=" << r->HI.sq1 << "; rd=" << r->GPR [ i.Rd ].sq0;
#endif

}

void Execute::MULTU1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MULTU1 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// if rs is between 0 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	// cycle times are different on ps2 ??
	/*
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
	*/

	// multiply unsigned Lo,Hi = rs * rt
	//r->HiLo.uValue = ((u64) (r->GPR [ i.Rs ].u)) * ((u64) (r->GPR [ i.Rt ].u));
	u64 temp;
	temp = ( (u64) ( r->GPR [ i.Rs ].uw0 ) ) * ( (u64) ( r->GPR [ i.Rt ].uw0 ) );
	r->LO.sq1 = (s32) temp;
	r->HI.sq1 = (s32) ( temp >> 32 );
	
	// R5900 can additionally write to register
	if ( i.Rd )
	{
		r->GPR [ i.Rd ].sq0 = (s32) temp;
	}
	
#if defined INLINE_DEBUG_MULTU1 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "; Output: LO1=" << r->LO.sq1 << "; HI1=" << r->HI.sq1 << "; rd=" << r->GPR [ i.Rd ].sq0;
#endif

}

void Execute::DIV1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DIV1 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	
	// 37 cycles
	// divide by two until r5900 implementation is running at proper speed
	static const int c_iDivideCycles = 37 / 2;

	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	// mult/div unit is busy now
	r->MulDiv_BusyUntil_Cycle1 = r->CycleCount + c_iDivideCycles;

	// divide signed: Lo = rs / rt; Hi = rs % rt
	//if ( r->GPR [ i.Rt ].u != 0 )
	if ( r->GPR [ i.Rt ].uw0 != 0 )
	{
		// if rs = 0x80000000 and rt = -1 then hi = 0 and lo = 0x80000000
		if ( r->GPR [ i.Rs ].uw0 == 0x80000000 && r->GPR [ i.Rt ].sw0 == -1 )
		{
			// *todo* check if all this stuff is sign extended during testing
			//r->HiLo.uHi = 0;
			//r->HiLo.uLo = 0x80000000;
			r->HI.sq1 = 0;
			r->LO.sq1 = (s32) 0x80000000;
		}
		else
		{
			// *todo* check during testing if signs are switched on modulus result when sign of dividend and divisor are different
			//r->HiLo.sLo = r->GPR [ i.Rs ].s / r->GPR [ i.Rt ].s;
			//r->HiLo.sHi = r->GPR [ i.Rs ].s % r->GPR [ i.Rt ].s;
			r->LO.sq1 = r->GPR [ i.Rs ].sw0 / r->GPR [ i.Rt ].sw0;
			r->HI.sq1 = r->GPR [ i.Rs ].sw0 % r->GPR [ i.Rt ].sw0;
		}
	}
	else
	{
		if ( r->GPR [ i.Rs ].s < 0 )
		{
			//r->HiLo.sLo = 1;
			r->LO.sq1 = 1;
		}
		else
		{
			//r->HiLo.sLo = -1;
			r->LO.sq1 = -1;
		}
		
		//r->HiLo.uHi = r->GPR [ i.Rs ].u;
		r->HI.sq1 = r->GPR [ i.Rs ].sw0;
	}
	
#if defined INLINE_DEBUG_DIV1 || defined INLINE_DEBUG_R5900
	debug << "; Output: LO1 = " << r->LO.sq1 << "; HI1 = " << r->HI.sq1;
#endif
}

void Execute::DIVU1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DIVU1 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	// 37 cycles
	// divide by two until r5900 implementation is running at proper speed
	static const int c_iDivideCycles = 37 / 2;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	// mult/div unit is busy now
	r->MulDiv_BusyUntil_Cycle1 = r->CycleCount + c_iDivideCycles;

	// divide unsigned: Lo = rs / rt; Hi = rs % rt
	//if ( r->GPR [ i.Rt ].u != 0 )
	if ( r->GPR [ i.Rt ].uw0 != 0 )
	{
		//r->HiLo.uLo = r->GPR [ i.Rs ].u / r->GPR [ i.Rt ].u;
		//r->HiLo.uHi = r->GPR [ i.Rs ].u % r->GPR [ i.Rt ].u;
		r->LO.sq1 = (s32) ( r->GPR [ i.Rs ].uw0 / r->GPR [ i.Rt ].uw0 );
		r->HI.sq1 = (s32) ( r->GPR [ i.Rs ].uw0 % r->GPR [ i.Rt ].uw0 );
	}
	else
	{
		//r->HiLo.sLo = -1;
		//r->HiLo.uHi = r->GPR [ i.Rs ].u;
		r->LO.sq1 = -1;
		r->HI.sq1 = r->GPR [ i.Rs ].sw0;
	}
	
#if defined INLINE_DEBUG_DIVU1 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "; Output: LO1 = " << r->LO.sq1 << "; HI1 = " << r->HI.sq1;
#endif
}

void Execute::MADD ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MADD || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}

	/*
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
	*/
	
	// multiply signed Lo,Hi = rs * rt
	//r->HiLo.sValue = ((s64) (r->GPR [ i.Rs ].s)) * ((s64) (r->GPR [ i.Rt ].s));
	s64 temp, lltemp2;
	temp = ( (s64) ( r->GPR [ i.Rs ].sw0 ) ) * ( (s64) ( r->GPR [ i.Rt ].sw0 ) );
	
	// also add in hi,lo
	lltemp2 = ( (u64) r->LO.uw0 ) | ( ( (u64) r->HI.uw0 ) << 32 );
	temp += lltemp2;
	
	r->LO.sq0 = (s32) temp;
	r->HI.sq0 = (s32) ( temp >> 32 );
	
	// R5900 can additionally write to register
	if ( i.Rd )
	{
		r->GPR [ i.Rd ].sq0 = (s32) temp;
	}
}

void Execute::MADD1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MADD1 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}

	/*
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
	*/
	
	// multiply signed Lo,Hi = rs * rt
	//r->HiLo.sValue = ((s64) (r->GPR [ i.Rs ].s)) * ((s64) (r->GPR [ i.Rt ].s));
	s64 temp, lltemp2;
	temp = ( (s64) ( r->GPR [ i.Rs ].sw0 ) ) * ( (s64) ( r->GPR [ i.Rt ].sw0 ) );
	
	// also add in hi,lo
	lltemp2 = ( (u64) r->LO.uw2 ) | ( ( (u64) r->HI.uw2 ) << 32 );
	temp += lltemp2;
	
	r->LO.sq1 = (s32) temp;
	r->HI.sq1 = (s32) ( temp >> 32 );
	
	// R5900 can additionally write to register
	if ( i.Rd )
	{
		r->GPR [ i.Rd ].sq0 = (s32) temp;
	}
}

void Execute::MADDU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDU || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// if rs is between 0 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	
	/*
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
	*/

	// multiply unsigned Lo,Hi = rs * rt
	//r->HiLo.uValue = ((u64) (r->GPR [ i.Rs ].u)) * ((u64) (r->GPR [ i.Rt ].u));
	u64 temp, ulltemp2;
	temp = ( (u64) ( r->GPR [ i.Rs ].uw0 ) ) * ( (u64) ( r->GPR [ i.Rt ].uw0 ) );
	
	// also add in hi,lo
	ulltemp2 = ( (u64) r->LO.uw0 ) | ( ( (u64) r->HI.uw0 ) << 32 );
	temp += ulltemp2;
	
	r->LO.sq0 = (s32) temp;
	r->HI.sq0 = (s32) ( temp >> 32 );
	
	// R5900 can additionally write to register
	if ( i.Rd )
	{
		r->GPR [ i.Rd ].sq0 = (s32) temp;
	}
	
#if defined INLINE_DEBUG_MULTU || defined INLINE_DEBUG_R5900
	debug << "; Output: LO=" << r->LO.s << "; HI=" << r->HI.s << "; rd=" << r->GPR [ i.Rd ].s;
#endif
}

void Execute::MADDU1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDU1 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// if rs is between 0 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	// constant multiply cycles for PS2
	static const int c_iMultiplyCycles = 4 / 2;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	/*
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
	*/

	// multiply unsigned Lo,Hi = rs * rt
	//r->HiLo.uValue = ((u64) (r->GPR [ i.Rs ].u)) * ((u64) (r->GPR [ i.Rt ].u));
	u64 temp, ulltemp2;
	temp = ( (u64) ( r->GPR [ i.Rs ].uw0 ) ) * ( (u64) ( r->GPR [ i.Rt ].uw0 ) );
	
	// also add in hi,lo
	ulltemp2 = ( (u64) r->LO.uw2 ) | ( ( (u64) r->HI.uw2 ) << 32 );
	temp += ulltemp2;
	
	r->LO.sq1 = (s32) temp;
	r->HI.sq1 = (s32) ( temp >> 32 );
	
	// R5900 can additionally write to register
	if ( i.Rd )
	{
		r->GPR [ i.Rd ].sq0 = (s32) temp;
	}
	
#if defined INLINE_DEBUG_MULTU || defined INLINE_DEBUG_R5900
	debug << "; Output: LO=" << r->LO.s << "; HI=" << r->HI.s << "; rd=" << r->GPR [ i.Rd ].s;
#endif
}



// Load/Store instructions //

void Execute::SD ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SD || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif
	
	// SW rt, offset(base)
	u32 StoreAddress;
	u64 *pMemPtr64;
	
	// check if storing to data cache
	//StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	StoreAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
	// *** testing *** alert if load is from unaligned address
	if ( StoreAddress & 0x7 )
	{
#if defined INLINE_DEBUG_STORE_UNALIGNED
		debug << "\r\nhps2x64 ALERT: StoreAddress is unaligned for SD @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
#endif

		cout << "\nhps2x64 ALERT: StoreAddress is unaligned for SD @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADES );
		return;
	}
	
	// clear top 3 bits since there is no data cache for caching stores
	// don't clear top 3 bits since scratchpad is at 0x70000000
	//StoreAddress &= 0x1fffffff;

#ifdef ENABLE_R5900_DCACHE_SD
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( StoreAddress ) )
	{
		pMemPtr64 = handle_cached_store ( StoreAddress );
		
		// store data to cache-line
		((s64*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 3 ] = r->GPR [ i.Rt ].sq0;
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->Bus->Write_t<0xffffffffffffffffULL> ( StoreAddress, r->GPR [ i.Rt ].uq0 );

		handle_uncached_store ();
	}

#else
	
	// ***todo*** perform store of word
	//r->Bus->Write ( StoreAddress, r->GPR [ i.Rt ].uq0, 0xffffffffffffffffULL );
	r->Bus->Write_t<0xffffffffffffffffULL> ( StoreAddress, r->GPR [ i.Rt ].uq0 );
	
	

	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
	
#endif

#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].uw0 )
#endif
}

void Execute::LD ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LD || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// LW rt, offset(base)
	
	u32 LoadAddress;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
	// *** testing *** alert if load is from unaligned address
	if ( LoadAddress & 0x7 )
	{
		cout << "\nhps2x64 ALERT: LoadAddress is unaligned for LD @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
		return;
	}
	
#ifdef ENABLE_R5900_DCACHE_LD
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load ( LoadAddress, i.Rt );
		
		// load from data cache into register
		r->GPR [ i.Rt ].uq0 = ((u64*)pMemPtr64) [ ( LoadAddress & 0x3f ) >> 3 ];
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->GPR [ i.Rt ].uq0 = (u64) r->Bus->Read_t<0xffffffffffffffffULL> ( LoadAddress );
		
		handle_uncached_load ( i.Rt );
	}

#else
	
	// ***todo*** perform signed load of word
	//r->GPR [ i.Rt ].uq0 = r->Bus->Read ( LoadAddress, 0xffffffffffffffffULL );
	r->GPR [ i.Rt ].uq0 = r->Bus->Read_t<0xffffffffffffffffULL> ( LoadAddress );
	
	
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
	
#endif

#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].uw0 )
#endif

#if defined INLINE_DEBUG_LD || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].sq0;
#endif
}

void Execute::LWU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LWU || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif
	
	// LW rt, offset(base)
	
	u32 LoadAddress;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
	// *** testing *** alert if load is from unaligned address
	if ( LoadAddress & 0x3 )
	{
		cout << "\nhps2x64 ALERT: LoadAddress is unaligned for LW @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
		return;
	}
	
#ifdef ENABLE_R5900_DCACHE_LWU
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load ( LoadAddress, i.Rt );
		
		// load from data cache into register
		r->GPR [ i.Rt ].uq0 = ((u32*)pMemPtr64) [ ( LoadAddress & 0x3f ) >> 2 ];
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->GPR [ i.Rt ].uq0 = (u32) r->Bus->Read_t<0xffffffff> ( LoadAddress );

		handle_uncached_load ( i.Rt );
	}

#else
	
	// ***todo*** perform signed load of word
	//r->GPR [ i.Rt ].uq0 = (u32) r->Bus->Read ( LoadAddress, 0xffffffffULL );
	r->GPR [ i.Rt ].uq0 = (u32) r->Bus->Read_t<0xffffffff> ( LoadAddress );
	
	
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
	
#endif

#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].uw0 )
#endif
	
#if defined INLINE_DEBUG_LWU || defined INLINE_DEBUG_R5900
	debug << "; Output: rt = " << r->GPR [ i.Rt ].sq0;
#endif
}

void Execute::SDL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SDL || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif

	static const u32 c_Mask = 0xffffffff;
	u64 Type, Offset;
	u64 Value1, Value2;
	u64 *pMemPtr64;

	// SDL rt, offset(base)
	
	// step 1: check if storing to data cache
	//u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	u32 StoreAddress = r->GPR [ i.Base ].sw0 + i.sOffset;

	// clear top 3 bits since there is no data cache for caching stores
	//StoreAddress &= 0x1fffffff;
	
#ifdef ENABLE_R5900_DCACHE_SDL
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( StoreAddress ) )
	{
		pMemPtr64 = handle_cached_store ( StoreAddress );

		// store data to cache-line
		Value1 = r->GPR [ i.Rt ].uq0 >> ( ( 7 - ( StoreAddress & 7 ) ) << 3 );
		Value2 = ((s64*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 3 ] & ~( 0xffffffffffffffffULL >> ( ( 7 - ( StoreAddress & 7 ) ) << 3 ) );
		((s64*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 3 ] = Value1 | Value2;
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->Bus->Write ( StoreAddress & ~7, r->GPR [ i.Rt ].uq0 >> ( ( 7 - ( StoreAddress & 7 ) ) << 3 ), 0xffffffffffffffffULL >> ( ( 7 - ( StoreAddress & 7 ) ) << 3 ) );
		
		handle_uncached_store ();
	}

#else
	
	// ***todo*** perform store SDL
	r->Bus->Write ( StoreAddress & ~7, r->GPR [ i.Rt ].uq0 >> ( ( 7 - ( StoreAddress & 7 ) ) << 3 ), 0xffffffffffffffffULL >> ( ( 7 - ( StoreAddress & 7 ) ) << 3 ) );
	

	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif

#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].uw0 )
#endif
}

void Execute::SDR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SDR || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].u << "; base = " << r->GPR [ i.Base ].u;
#endif

	static const u32 c_Mask = 0xffffffff;
	u32 Type, Offset;
	u64 Value1, Value2;
	u64 *pMemPtr64;
	
	// SDR rt, offset(base)
	
	// step 1: check if storing to data cache
	//u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	u32 StoreAddress = r->GPR [ i.Base ].sw0 + i.sOffset;

	// clear top 3 bits since there is no data cache for caching stores
	//StoreAddress &= 0x1fffffff;
	
#ifdef ENABLE_R5900_DCACHE_SDR
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( StoreAddress ) )
	{
		pMemPtr64 = handle_cached_store ( StoreAddress );
		
		// store data to cache-line
		Value1 = r->GPR [ i.Rt ].uq0 << ( ( StoreAddress & 7 ) << 3 );
		Value2 = ((s64*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 3 ] & ~( 0xffffffffffffffffULL << ( ( StoreAddress & 7 ) << 3 ) );
		((s64*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 3 ] = Value1 | Value2;
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->Bus->Write ( StoreAddress & ~7, r->GPR [ i.Rt ].uq0 << ( ( StoreAddress & 7 ) << 3 ), 0xffffffffffffffffULL << ( ( StoreAddress & 7 ) << 3 ) );
		
		handle_uncached_store ();
	}

#else
	
	// ***todo*** perform store SWR
	r->Bus->Write ( StoreAddress & ~7, r->GPR [ i.Rt ].uq0 << ( ( StoreAddress & 7 ) << 3 ), 0xffffffffffffffffULL << ( ( StoreAddress & 7 ) << 3 ) );
	
	
	
	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
	
#endif
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].uw0 )
#endif
}

void Execute::LDL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LDL || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif

	// LDL rt, offset(base)
	
	u32 LoadAddress;
	u64 Value, Temp;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;

#ifdef ENABLE_R5900_DCACHE_LDL
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load ( LoadAddress, i.Rt );
		
		// load from data cache into register
		Value = ((s64*)pMemPtr64) [ ( LoadAddress & 0x3f ) >> 3 ];
		
		Value <<= ( ( 7 - ( LoadAddress & 7 ) ) << 3 );
		Temp = r->GPR [ i.Rt ].uq0;
		Temp <<= ( ( ( LoadAddress & 7 ) + 1 ) << 3 );
		if ( ( LoadAddress & 7 ) == 7 ) Temp = 0;
		Temp >>= ( ( ( LoadAddress & 7 ) + 1 ) << 3 );
		r->GPR [ i.Rt ].sq0 = Value | Temp;
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		Value = r->Bus->Read_t<0xffffffffffffffffULL> ( LoadAddress & ~7 );
		
		Value <<= ( ( 7 - ( LoadAddress & 7 ) ) << 3 );
		Temp = r->GPR [ i.Rt ].uq0;
		Temp <<= ( ( ( LoadAddress & 7 ) + 1 ) << 3 );
		if ( ( LoadAddress & 7 ) == 7 ) Temp = 0;
		Temp >>= ( ( ( LoadAddress & 7 ) + 1 ) << 3 );
		r->GPR [ i.Rt ].sq0 = Value | Temp;
		
		handle_uncached_load ( i.Rt );
	}

#else

	// ***todo*** perform load LDL
	//Value = r->Bus->Read ( LoadAddress & ~7, 0xffffffffffffffffULL );
	Value = r->Bus->Read_t<0xffffffffffffffffULL> ( LoadAddress & ~7 );
	
	Value <<= ( ( 7 - ( LoadAddress & 7 ) ) << 3 );
	Temp = r->GPR [ i.Rt ].uq0;
	Temp <<= ( ( ( LoadAddress & 7 ) + 1 ) << 3 );
	if ( ( LoadAddress & 7 ) == 7 ) Temp = 0;
	Temp >>= ( ( ( LoadAddress & 7 ) + 1 ) << 3 );
	r->GPR [ i.Rt ].sq0 = Value | Temp;
	
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
	
#endif

#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].uw0 )
#endif
	
#if defined INLINE_DEBUG_LDL || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "; Output: rt = " << r->GPR [ i.Rt ].sq0;
#endif
}

void Execute::LDR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LDR || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif

	// LDR rt, offset(base)
	
	u32 LoadAddress;
	u64 Value, Temp;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
#ifdef ENABLE_R5900_DCACHE_LDR
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load ( LoadAddress, i.Rt );
		
		// load from data cache into register
		Value = ((s64*)pMemPtr64) [ ( LoadAddress & 0x3f ) >> 3 ];
		
		Value >>= ( ( LoadAddress & 7 ) << 3 );
		Temp = r->GPR [ i.Rt ].uq0;
		Temp >>= ( ( 8 - ( LoadAddress & 7 ) ) << 3 );
		if ( ( LoadAddress & 7 ) == 0 ) Temp = 0;
		Temp <<= ( ( 8 - ( LoadAddress & 7 ) ) << 3 );
		r->GPR [ i.Rt ].sq0 = Value | Temp;
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		Value = r->Bus->Read_t<0xffffffffffffffffULL> ( LoadAddress & ~7 );
		
		Value >>= ( ( LoadAddress & 7 ) << 3 );
		Temp = r->GPR [ i.Rt ].uq0;
		Temp >>= ( ( 8 - ( LoadAddress & 7 ) ) << 3 );
		if ( ( LoadAddress & 7 ) == 0 ) Temp = 0;
		Temp <<= ( ( 8 - ( LoadAddress & 7 ) ) << 3 );
		r->GPR [ i.Rt ].sq0 = Value | Temp;
		
		handle_uncached_load ( i.Rt );
	}

#else
	
	// ***todo*** perform load LWR
	//Value = r->Bus->Read ( LoadAddress & ~7, 0xffffffffffffffffULL );
	Value = r->Bus->Read_t<0xffffffffffffffffULL> ( LoadAddress & ~7 );
	
	Value >>= ( ( LoadAddress & 7 ) << 3 );
	Temp = r->GPR [ i.Rt ].uq0;
	Temp >>= ( ( 8 - ( LoadAddress & 7 ) ) << 3 );
	if ( ( LoadAddress & 7 ) == 0 ) Temp = 0;
	Temp <<= ( ( 8 - ( LoadAddress & 7 ) ) << 3 );
	r->GPR [ i.Rt ].sq0 = Value | Temp;
	
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
#endif

#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].uw0 )
#endif
	
#if defined INLINE_DEBUG_LDR || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "; Output: rt = " << r->GPR [ i.Rt ].sq0;
#endif
}

void Execute::LQ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LQ || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif

	// LQ rt, offset(base)
	
	u32 LoadAddress;
	u64* Data;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
#if defined INLINE_DEBUG_LQ || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " LoadAddress=" << LoadAddress;
#endif

	// *** testing *** alert if load is from unaligned address
	// this one actually does NOT have any address errors due to alignment, as it pretends the bottom 4-bits are zero on R5900
	//if ( LoadAddress & 0xf )
	//{
	//	cout << "\nhps2x64 ALERT: LoadAddress is unaligned for LW @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
	//	
	//	// *** testing ***
	//	r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
	//	return;
	//}
	
	// bottom four bits of Address are cleared
	LoadAddress &= ~0xf;

	
#ifdef ENABLE_R5900_DCACHE_LQ
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load ( LoadAddress, i.Rt );
		
		// load from data cache into register
		Data = & ((u64*)pMemPtr64) [ ( LoadAddress & 0x3f ) >> 3 ];
		
		r->GPR [ i.Rt ].uLo = Data [ 0 ];
		r->GPR [ i.Rt ].uHi = Data [ 1 ];
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		Data = (u64*) r->Bus->Read_t<0> ( LoadAddress );
		
		r->GPR [ i.Rt ].uLo = Data [ 0 ];
		r->GPR [ i.Rt ].uHi = Data [ 1 ];
		
		handle_uncached_load ( i.Rt );
	}

#else
	
	
	//Data = (u64*) r->Bus->Read ( LoadAddress, 0 );
	Data = (u64*) r->Bus->Read_t<0> ( LoadAddress );
	
	r->GPR [ i.Rt ].uLo = Data [ 0 ];
	r->GPR [ i.Rt ].uHi = Data [ 1 ];
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
	
#endif

#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].uw0 )
#endif

#if defined INLINE_DEBUG_LQ || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "; Output: rt = " << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif
}

void Execute::SQ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SQ || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1 << "; base = " << r->GPR [ i.Base ].u;
#endif

	// SQ rt, offset(base)
	u32 StoreAddress;
	u64 *pMemPtr64;
	u64 *Data;
	
	// check if storing to data cache
	//StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	StoreAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
	// clear top 3 bits since there is no data cache for caching stores
	// don't clear top 3 bits since scratchpad is at 0x70000000
	//StoreAddress &= 0x1fffffff;
	
	// bottom four bits of Address are cleared
	StoreAddress &= ~0xf;

#ifdef ENABLE_R5900_DCACHE_SQ
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( StoreAddress ) )
	{
		pMemPtr64 = handle_cached_store ( StoreAddress );

		// load from data cache into register
		Data = & ((u64*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 3 ];
		
		Data [ 0 ] = r->GPR [ i.Rt ].uLo;
		Data [ 1 ] = r->GPR [ i.Rt ].uHi;
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->Bus->Write_t<0> ( StoreAddress, (u64) & ( r->GPR [ i.Rt ].uw0 ) );
		
		handle_uncached_store ();
	}

#else
	
	// ***todo*** perform store of word
	// *note* probably want to pass a pointer to hi part, since that is in lower area of memory
	//r->Bus->Write ( StoreAddress, & ( r->GPR [ i.Rt ].uw0 ), 0 );
	r->Bus->Write_t<0> ( StoreAddress, (u64) & ( r->GPR [ i.Rt ].uw0 ) );
	
	

	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
	
#endif
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].uw0 )
#endif
}


void Execute::MOVZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MOVZ || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rd=" << r->GPR [ i.Rd ].uq0 << "; rs=" << r->GPR [ i.Rs ].uq0 << "; rt=" << r->GPR [ i.Rt ].uq0;
#endif

	// movz rd, rs, rt
	// if ( rt == 0 ) rd = rs
	
	if ( !r->GPR [ i.Rt ].uq0 )
	{
		r->GPR [ i.Rd ].uq0 = r->GPR [ i.Rs ].uq0;
	}
	
#if defined INLINE_DEBUG_MOVZ || defined INLINE_DEBUG_R5900
	debug << hex << "; Output: rd=" << r->GPR [ i.Rd ].uq0;
#endif
}

void Execute::MOVN ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MOVN || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rd=" << r->GPR [ i.Rd ].uq0 << "; rs=" << r->GPR [ i.Rs ].uq0 << "; rt=" << r->GPR [ i.Rt ].uq0;
#endif

	// movn rd, rs, rt
	// if ( rt != 0 ) rd = rs

	if ( r->GPR [ i.Rt ].uq0 )
	{
		r->GPR [ i.Rd ].uq0 = r->GPR [ i.Rs ].uq0;
	}
	
#if defined INLINE_DEBUG_MOVN || defined INLINE_DEBUG_R5900
	debug << hex << "; Output: rd=" << r->GPR [ i.Rd ].uq0;
#endif
}


void Execute::MFHI1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MFHI1 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: HI1 = " << r->HI.uq1;
#endif
	
	// this instruction interlocks if multiply/divide unit is busy
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	// move from Hi register
	r->GPR [ i.Rd ].uq0 = r->HI.uq1;
	
#if defined INLINE_DEBUG_MFHI || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::MTHI1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MTHI1 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	r->HI.uq1 = r->GPR [ i.Rs ].uq0;
}

void Execute::MFLO1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MFLO1 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: LO1 = " << r->LO.uq1;
#endif

	
	// this instruction interlocks if multiply/divide unit is busy
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	// move from Lo register
	r->GPR [ i.Rd ].uq0 = r->LO.uq1;
	
#if defined INLINE_DEBUG_MFLO || defined INLINE_DEBUG_R5900
	debug << "; Output: rd = " << r->GPR [ i.Rd ].u;
#endif
}

void Execute::MTLO1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MTLO1 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	r->LO.uq1 = r->GPR [ i.Rs ].uq0;
}



void Execute::MFSA ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MFSA || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: SA = " << r->SA;
#endif

	// MFSA rd
	// only operates in instruction pipeline 0
	r->GPR [ i.Rd ].uq0 = r->SA;

#if defined INLINE_DEBUG_MFSA || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "; Output: rd = " << r->GPR [ i.Rd ].uq0;
#endif
}

void Execute::MTSA ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MTSA || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].uq0;
#endif

	// MTSA rs
	// only operates in instruction pipeline 0
	// note: according to ps2autotests should also logical AND with 0xf
	//r->SA = r->GPR [ i.Rs ].uq0;
	r->SA = r->GPR [ i.Rs ].uq0 & 0xf;
	
#if defined INLINE_DEBUG_MTSA || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "; Output: SA = " << r->SA;
#endif
}

void Execute::MTSAB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MTSAB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].uq0;
#endif

	// MTSAB rs, immediate
	// only operates in instruction pipeline 0
	// or does this not shift left 3 ??
	//r->SA = ( ( ( r->GPR [ i.Rs ].uw0 ) ^ ( i.uImmediate ) ) & 0xf ) << 3;
	r->SA = ( ( ( r->GPR [ i.Rs ].uw0 ) ^ ( i.uImmediate ) ) & 0xf );
	
#if defined INLINE_DEBUG_MTSAB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "; Output: SA = " << r->SA;
#endif
}

void Execute::MTSAH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MTSAH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].uq0;
#endif

	// MTSAH rs, immediate
	// only operates in instruction pipeline 0
	// or does this shift left only one instead ??
	//r->SA = ( ( ( r->GPR [ i.Rs ].uw0 ) ^ ( i.uImmediate ) ) & 0x7 ) << 4;
	r->SA = ( ( ( r->GPR [ i.Rs ].uw0 ) ^ ( i.uImmediate ) ) & 0x7 ) << 1;
	
#if defined INLINE_DEBUG_MTSAH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "; Output: SA = " << r->SA;
#endif
}




// Branch instructions //

void Execute::BEQL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BEQL || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	if ( r->GPR [ i.Rs ].u == r->GPR [ i.Rt ].u )
	{
		// taking branch (after delay slot of course) //
		
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBEQL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BEQ || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}

void Execute::BNEL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BNEL || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u << "; rt = " << r->GPR [ i.Rt ].u;
#endif

	if ( r->GPR [ i.Rs ].u != r->GPR [ i.Rt ].u )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBNEL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BNE || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}

void Execute::BGEZL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BGEZL || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif

	if ( r->GPR [ i.Rs ].s >= 0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBGEZL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BGEZ || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}

void Execute::BLEZL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BLEZL || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif

	if ( r->GPR [ i.Rs ].s <= 0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBLEZL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BLEZ || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}

void Execute::BGTZL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BGTZL || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif

	if ( r->GPR [ i.Rs ].s > 0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBGTZL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BGTZ || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}

void Execute::BLTZL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BLTZL || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif

	if ( r->GPR [ i.Rs ].s < 0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBLTZL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BLTZ || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}



void Execute::BLTZALL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BLTZALL || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif

	if ( r->GPR [ i.Rs ].s < 0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBLTZALL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BLTZAL || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
	
	////////////////////////////////////////////////////////////////////////
	// Store return address when instruction is executed in r31
	// for this instruction this happens whether branch is taken or not
	// *note* this must happen AFTER comparison check
	r->GPR [ 31 ].u = r->PC + 8;
}

void Execute::BGEZALL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BGEZALL || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rs = " << r->GPR [ i.Rs ].u;
#endif

	if ( r->GPR [ i.Rs ].s >= 0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBGEZALL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BGEZAL || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}

	////////////////////////////////////////////////////////////////////////
	// Store return address when instruction is executed in r31
	// for this instruction this happens whether branch is taken or not
	// *note* this must happen AFTER comparison check
	r->GPR [ 31 ].u = r->PC + 8;
}




void Execute::BC0T ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BC0T || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; CPCOND0=" << hex << r->CPCOND0;
#endif

	if ( r->CPCOND0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBC0T>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BC0T || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BC0TL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BC0TL || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; CPCOND0=" << hex << r->CPCOND0;
#endif

	if ( r->CPCOND0 )
	{
		// taking branch (after delay slot of course) //
		
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBC0TL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BC0TL || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}

void Execute::BC0F ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BC0F || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; CPCOND0=" << hex << r->CPCOND0;
#endif

	if ( !r->CPCOND0 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBC0F>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BC0F || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BC0FL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BC0FL || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; CPCOND0=" << hex << r->CPCOND0;
#endif

	if ( !r->CPCOND0 )
	{
		// taking branch (after delay slot of course) //
		
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBC0FL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BC0FL || defined INLINE_DEBUG_R5900
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}

void Execute::BC1T ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BC1T || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: FCR31 = " << r->CPC1 [ 31 ];
#endif

	if ( r->CPC1 [ 31 ] & 0x00800000 )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBC1T>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BC1T || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BC1TL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BC1TL || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: FCR31 = " << r->CPC1 [ 31 ];
#endif

	if ( r->CPC1 [ 31 ] & 0x00800000 )
	{
		// taking branch (after delay slot of course) //
		
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBC1TL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BC1TL || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}

void Execute::BC1F ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BC1F || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: FCR31 = " << r->CPC1 [ 31 ];
#endif

	if ( ! ( r->CPC1 [ 31 ] & 0x00800000 ) )
	{
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBC1F>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BC1F || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
		debug << ";  WILL TAKE";
#endif
	}
}

void Execute::BC1FL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BC1FL || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: FCR31 = " << r->CPC1 [ 31 ];
#endif

	if ( ! ( r->CPC1 [ 31 ] & 0x00800000 ) )
	{
		// taking branch (after delay slot of course) //
		
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBC1FL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BC1FL || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}

void Execute::BC2T ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BC2T || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// branch if vu1 is running
	if ( VU1::_VU1->Running )
	{
		// taking branch (after delay slot of course) //
		
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBC2T>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BC2T || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}

void Execute::BC2TL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BC2TL || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// branch if vu1 is running
	if ( VU1::_VU1->Running )
	{
		// taking branch (after delay slot of course) //
		
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBC2TL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BC2TL || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}

void Execute::BC2F ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BC2F || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// branch if vu1 is NOT running
	if ( !VU1::_VU1->Running )
	{
		// taking branch (after delay slot of course) //
		
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBC2F>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BC2F || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}

void Execute::BC2FL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_BC2FL || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// branch if vu1 is NOT running
	if ( !VU1::_VU1->Running )
	{
		// taking branch (after delay slot of course) //
		
		// next instruction is in the branch delay slot
		Cpu::DelaySlot *d = & ( r->DelaySlots [ r->NextDelaySlotIndex ^ 1 ] );
		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		d->cb = r->ProcessBranchDelaySlot_t<OPBC2FL>;
		r->Status.DelaySlot_Valid |= 0x2;
		
#if defined INLINE_DEBUG_BC2FL || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
		debug << ";  WILL TAKE";
#endif
	}
	else
	{
		// NOT branching (and skips next instruction) //
		
		// skip next instruction
		r->NextPC = r->PC + 8;
		
		// *todo* add an additional cycle for the skipped instruction
		//CycleCount++;
	}
}






void Execute::TGEI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TGEI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// trap if rs >= signed imm
	if ( r->GPR [ i.Rs ].sq0 >= (s64) i.sImmediate )
	{
		r->ProcessSynchronousInterrupt ( Cpu::EXC_TRAP );
		
#ifdef ENABLE_R5900_BRANCH_PREDICTION
		r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif
	}
}

void Execute::TGEIU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TGEIU || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// trap if rs >= unsigned imm
	if ( r->GPR [ i.Rs ].uq0 >= (u64) i.uImmediate )
	{
		r->ProcessSynchronousInterrupt ( Cpu::EXC_TRAP );
		
#ifdef ENABLE_R5900_BRANCH_PREDICTION
		r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif
	}
}

void Execute::TLTI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TLTI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// trap if rs < signed imm
	if ( r->GPR [ i.Rs ].sq0 < (s64) i.sImmediate )
	{
		r->ProcessSynchronousInterrupt ( Cpu::EXC_TRAP );
		
#ifdef ENABLE_R5900_BRANCH_PREDICTION
		r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif
	}
}

void Execute::TLTIU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TLTIU || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// trap if rs < unsigned imm
	if ( r->GPR [ i.Rs ].uq0 < (u64) i.uImmediate )
	{
		r->ProcessSynchronousInterrupt ( Cpu::EXC_TRAP );
		
#ifdef ENABLE_R5900_BRANCH_PREDICTION
		r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif
	}
}

void Execute::TEQI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TEQI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// trap if rs == signed imm
	if ( r->GPR [ i.Rs ].sq0 == (s64) i.sImmediate )
	{
		r->ProcessSynchronousInterrupt ( Cpu::EXC_TRAP );
		
#ifdef ENABLE_R5900_BRANCH_PREDICTION
		r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif
	}
}

void Execute::TNEI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TNEI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// trap if rs != signed imm
	if ( r->GPR [ i.Rs ].sq0 != (s64) i.sImmediate )
	{
		r->ProcessSynchronousInterrupt ( Cpu::EXC_TRAP );
		
#ifdef ENABLE_R5900_BRANCH_PREDICTION
		r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif
	}
}


void Execute::TGE ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TGE || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// trap if rs >= rt
	if ( r->GPR [ i.Rs ].sq0 >= r->GPR [ i.Rt ].sq0 )
	{
		r->ProcessSynchronousInterrupt ( Cpu::EXC_TRAP );
		
#ifdef ENABLE_R5900_BRANCH_PREDICTION
		r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif
	}
}

void Execute::TGEU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TGEU || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// trap if rs >= rt
	if ( r->GPR [ i.Rs ].uq0 >= r->GPR [ i.Rt ].uq0 )
	{
		r->ProcessSynchronousInterrupt ( Cpu::EXC_TRAP );
		
#ifdef ENABLE_R5900_BRANCH_PREDICTION
		r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif
	}
}

void Execute::TLT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TLT || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// trap if rs < rt
	if ( r->GPR [ i.Rs ].sq0 < r->GPR [ i.Rt ].sq0 )
	{
		r->ProcessSynchronousInterrupt ( Cpu::EXC_TRAP );
		
#ifdef ENABLE_R5900_BRANCH_PREDICTION
		r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif
	}
}

void Execute::TLTU ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TLTU || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// trap if rs < rt
	if ( r->GPR [ i.Rs ].uq0 < r->GPR [ i.Rt ].uq0 )
	{
		r->ProcessSynchronousInterrupt ( Cpu::EXC_TRAP );
		
#ifdef ENABLE_R5900_BRANCH_PREDICTION
		r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif
	}
}

void Execute::TEQ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TEQ || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// trap if rs == rt
	if ( r->GPR [ i.Rs ].uq0 == r->GPR [ i.Rt ].uq0 )
	{
		r->ProcessSynchronousInterrupt ( Cpu::EXC_TRAP );
		
#ifdef ENABLE_R5900_BRANCH_PREDICTION
		r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif
	}
}

void Execute::TNE ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TNE || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// trap if rs != rt
	if ( r->GPR [ i.Rs ].uq0 != r->GPR [ i.Rt ].uq0 )
	{
		r->ProcessSynchronousInterrupt ( Cpu::EXC_TRAP );
		
#ifdef ENABLE_R5900_BRANCH_PREDICTION
		r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif
	}
}


















// * R5900 Parallel (SIMD) instructions * //


void Execute::PADSBH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PADSBH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// first four halfwords subtract, next four add
	r->GPR [ i.Rd ].uh0 = r->GPR [ i.Rs ].uh0 - r->GPR [ i.Rt ].uh0;
	r->GPR [ i.Rd ].uh1 = r->GPR [ i.Rs ].uh1 - r->GPR [ i.Rt ].uh1;
	r->GPR [ i.Rd ].uh2 = r->GPR [ i.Rs ].uh2 - r->GPR [ i.Rt ].uh2;
	r->GPR [ i.Rd ].uh3 = r->GPR [ i.Rs ].uh3 - r->GPR [ i.Rt ].uh3;
	r->GPR [ i.Rd ].uh4 = r->GPR [ i.Rs ].uh4 + r->GPR [ i.Rt ].uh4;
	r->GPR [ i.Rd ].uh5 = r->GPR [ i.Rs ].uh5 + r->GPR [ i.Rt ].uh5;
	r->GPR [ i.Rd ].uh6 = r->GPR [ i.Rs ].uh6 + r->GPR [ i.Rt ].uh6;
	r->GPR [ i.Rd ].uh7 = r->GPR [ i.Rs ].uh7 + r->GPR [ i.Rt ].uh7;
	
#if defined INLINE_DEBUG_PADSBH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PABSH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PABSH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sh0 = ( r->GPR [ i.Rt ].sh0 >= 0 ) ? r->GPR [ i.Rt ].sh0 : -r->GPR [ i.Rt ].sh0;
	r->GPR [ i.Rd ].sh1 = ( r->GPR [ i.Rt ].sh1 >= 0 ) ? r->GPR [ i.Rt ].sh1 : -r->GPR [ i.Rt ].sh1;
	r->GPR [ i.Rd ].sh2 = ( r->GPR [ i.Rt ].sh2 >= 0 ) ? r->GPR [ i.Rt ].sh2 : -r->GPR [ i.Rt ].sh2;
	r->GPR [ i.Rd ].sh3 = ( r->GPR [ i.Rt ].sh3 >= 0 ) ? r->GPR [ i.Rt ].sh3 : -r->GPR [ i.Rt ].sh3;
	r->GPR [ i.Rd ].sh4 = ( r->GPR [ i.Rt ].sh4 >= 0 ) ? r->GPR [ i.Rt ].sh4 : -r->GPR [ i.Rt ].sh4;
	r->GPR [ i.Rd ].sh5 = ( r->GPR [ i.Rt ].sh5 >= 0 ) ? r->GPR [ i.Rt ].sh5 : -r->GPR [ i.Rt ].sh5;
	r->GPR [ i.Rd ].sh6 = ( r->GPR [ i.Rt ].sh6 >= 0 ) ? r->GPR [ i.Rt ].sh6 : -r->GPR [ i.Rt ].sh6;
	r->GPR [ i.Rd ].sh7 = ( r->GPR [ i.Rt ].sh7 >= 0 ) ? r->GPR [ i.Rt ].sh7 : -r->GPR [ i.Rt ].sh7;
	
#if defined INLINE_DEBUG_PABSH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PABSW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PABSW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sw0 = ( r->GPR [ i.Rt ].sw0 >= 0 ) ? r->GPR [ i.Rt ].sw0 : -r->GPR [ i.Rt ].sw0;
	r->GPR [ i.Rd ].sw1 = ( r->GPR [ i.Rt ].sw1 >= 0 ) ? r->GPR [ i.Rt ].sw1 : -r->GPR [ i.Rt ].sw1;
	r->GPR [ i.Rd ].sw2 = ( r->GPR [ i.Rt ].sw2 >= 0 ) ? r->GPR [ i.Rt ].sw2 : -r->GPR [ i.Rt ].sw2;
	r->GPR [ i.Rd ].sw3 = ( r->GPR [ i.Rt ].sw3 >= 0 ) ? r->GPR [ i.Rt ].sw3 : -r->GPR [ i.Rt ].sw3;
	
#if defined INLINE_DEBUG_PABSW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PAND ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PAND || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1 << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].uq0 = r->GPR [ i.Rs ].uq0 & r->GPR [ i.Rt ].uq0;
	r->GPR [ i.Rd ].uq1 = r->GPR [ i.Rs ].uq1 & r->GPR [ i.Rt ].uq1;
	
#if defined INLINE_DEBUG_PAND || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PXOR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PXOR || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1 << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].uq0 = r->GPR [ i.Rs ].uq0 ^ r->GPR [ i.Rt ].uq0;
	r->GPR [ i.Rd ].uq1 = r->GPR [ i.Rs ].uq1 ^ r->GPR [ i.Rt ].uq1;
	
#if defined INLINE_DEBUG_PXOR || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::POR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_POR || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1 << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].uq0 = r->GPR [ i.Rs ].uq0 | r->GPR [ i.Rt ].uq0;
	r->GPR [ i.Rd ].uq1 = r->GPR [ i.Rs ].uq1 | r->GPR [ i.Rt ].uq1;
	
#if defined INLINE_DEBUG_POR || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PNOR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PNOR || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1 << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].uq0 = ~( r->GPR [ i.Rs ].uq0 | r->GPR [ i.Rt ].uq0 );
	r->GPR [ i.Rd ].uq1 = ~( r->GPR [ i.Rs ].uq1 | r->GPR [ i.Rt ].uq1 );
	
#if defined INLINE_DEBUG_PNOR || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}


void Execute::PLZCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PLZCW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
#endif

	if ( r->GPR [ i.Rs ].sw0 < 0 )
	{
		// if negative, it is actually a leading one count
		r->GPR [ i.Rd ].sw0 = CountLeadingZeros32 ( ~r->GPR [ i.Rs ].sw0 ) - 1;
	}
	else
	{
		// is positive, so do a leading zero count //
		r->GPR [ i.Rd ].sw0 = CountLeadingZeros32 ( r->GPR [ i.Rs ].sw0 ) - 1;
	}
	
	if ( r->GPR [ i.Rs ].sw1 < 0 )
	{
		// if negative, it is actually a leading one count
		r->GPR [ i.Rd ].sw1 = CountLeadingZeros32 ( ~r->GPR [ i.Rs ].sw1 ) - 1;
	}
	else
	{
		// is positive, so do a leading zero count //
		r->GPR [ i.Rd ].sw1 = CountLeadingZeros32 ( r->GPR [ i.Rs ].sw1 ) - 1;
	}
	
#if defined INLINE_DEBUG_PLZCW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}


void Execute::PMFHL_LH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMFHL_LH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " LO=" << r->LO.uq0 << " " << r->LO.uq1;
	debug << hex << " HI=" << r->HI.uq0 << " " << r->HI.uq1;
#endif

	r->GPR [ i.Rd ].uh0 = r->LO.uh0;
	r->GPR [ i.Rd ].uh1 = r->LO.uh2;
	r->GPR [ i.Rd ].uh2 = r->HI.uh0;
	r->GPR [ i.Rd ].uh3 = r->HI.uh2;
	r->GPR [ i.Rd ].uh4 = r->LO.uh4;
	r->GPR [ i.Rd ].uh5 = r->LO.uh6;
	r->GPR [ i.Rd ].uh6 = r->HI.uh4;
	r->GPR [ i.Rd ].uh7 = r->HI.uh6;
	
#if defined INLINE_DEBUG_PMFHL_LH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PMFHL_LW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMFHL_LW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " LO=" << r->LO.uq0 << " " << r->LO.uq1;
	debug << hex << " HI=" << r->HI.uq0 << " " << r->HI.uq1;
#endif

	r->GPR [ i.Rd ].uw0 = r->LO.uw0;
	r->GPR [ i.Rd ].uw1 = r->HI.uw0;
	r->GPR [ i.Rd ].uw2 = r->LO.uw2;
	r->GPR [ i.Rd ].uw3 = r->HI.uw2;
	
#if defined INLINE_DEBUG_PMFHL_LW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PMFHL_UW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMFHL_UW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " LO=" << r->LO.uq0 << " " << r->LO.uq1;
	debug << hex << " HI=" << r->HI.uq0 << " " << r->HI.uq1;
#endif

	r->GPR [ i.Rd ].uw0 = r->LO.uw1;
	r->GPR [ i.Rd ].uw1 = r->HI.uw1;
	r->GPR [ i.Rd ].uw2 = r->LO.uw3;
	r->GPR [ i.Rd ].uw3 = r->HI.uw3;
	
#if defined INLINE_DEBUG_PMFHL_UW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PMTHL_LW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMTHL_LW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
#endif

	r->LO.sw0 = r->GPR [ i.Rs ].sw0;
	r->LO.sw2 = r->GPR [ i.Rs ].sw2;
	r->HI.sw0 = r->GPR [ i.Rs ].sw1;
	r->HI.sw2 = r->GPR [ i.Rs ].sw3;
	
#if defined INLINE_DEBUG_PMTHL_LW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " LO=" << r->LO.uq0 << " " << r->LO.uq1 << " HI=" << r->HI.uq0 << " " << r->HI.uq1;
#endif
}


void Execute::PMFHL_SH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMFHL_SH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PMFHL_SH";
	
	r->GPR [ i.Rd ].sh0 = ( ( r->LO.sw0 > 0x7fff ) ? 0x7fff : ( ( r->LO.sw0 < -0x8000 ) ? 0x8000 : r->LO.sw0 ) );
	r->GPR [ i.Rd ].sh1 = ( ( r->LO.sw1 > 0x7fff ) ? 0x7fff : ( ( r->LO.sw1 < -0x8000 ) ? 0x8000 : r->LO.sw1 ) );
	r->GPR [ i.Rd ].sh2 = ( ( r->HI.sw0 > 0x7fff ) ? 0x7fff : ( ( r->HI.sw0 < -0x8000 ) ? 0x8000 : r->HI.sw0 ) );
	r->GPR [ i.Rd ].sh3 = ( ( r->HI.sw1 > 0x7fff ) ? 0x7fff : ( ( r->HI.sw1 < -0x8000 ) ? 0x8000 : r->HI.sw1 ) );
	r->GPR [ i.Rd ].sh4 = ( ( r->LO.sw2 > 0x7fff ) ? 0x7fff : ( ( r->LO.sw2 < -0x8000 ) ? 0x8000 : r->LO.sw2 ) );
	r->GPR [ i.Rd ].sh5 = ( ( r->LO.sw3 > 0x7fff ) ? 0x7fff : ( ( r->LO.sw3 < -0x8000 ) ? 0x8000 : r->LO.sw3 ) );
	r->GPR [ i.Rd ].sh6 = ( ( r->HI.sw2 > 0x7fff ) ? 0x7fff : ( ( r->HI.sw2 < -0x8000 ) ? 0x8000 : r->HI.sw2 ) );
	r->GPR [ i.Rd ].sh7 = ( ( r->HI.sw3 > 0x7fff ) ? 0x7fff : ( ( r->HI.sw3 < -0x8000 ) ? 0x8000 : r->HI.sw3 ) );
}


void Execute::PMFHL_SLW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMFHL_SLW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PMFHL_SLW";
	
	s64 sTemp64;
	
	sTemp64 = r->LO.uw0;
	sTemp64 |= ( r->HI.uq0 << 32 );
	r->GPR [ i.Rd ].sq0 = ( ( sTemp64 >= 0x7fffffffLL ) ? 0x7fffffffLL : ( ( sTemp64 <= -0x80000000LL ) ? -0x80000000LL : r->LO.sw0 ) );
	
	sTemp64 = r->LO.uw2;
	sTemp64 |= ( r->HI.uq1 << 32 );
	r->GPR [ i.Rd ].sq1 = ( ( sTemp64 >= 0x7fffffffLL ) ? 0x7fffffffLL : ( ( sTemp64 <= -0x80000000LL ) ? -0x80000000LL : r->LO.sw2 ) );
}



void Execute::PSLLH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSLLH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].uh0 = r->GPR [ i.Rt ].uh0 << i.Shift;
	r->GPR [ i.Rd ].uh1 = r->GPR [ i.Rt ].uh1 << i.Shift;
	r->GPR [ i.Rd ].uh2 = r->GPR [ i.Rt ].uh2 << i.Shift;
	r->GPR [ i.Rd ].uh3 = r->GPR [ i.Rt ].uh3 << i.Shift;
	r->GPR [ i.Rd ].uh4 = r->GPR [ i.Rt ].uh4 << i.Shift;
	r->GPR [ i.Rd ].uh5 = r->GPR [ i.Rt ].uh5 << i.Shift;
	r->GPR [ i.Rd ].uh6 = r->GPR [ i.Rt ].uh6 << i.Shift;
	r->GPR [ i.Rd ].uh7 = r->GPR [ i.Rt ].uh7 << i.Shift;
	
#if defined INLINE_DEBUG_PSLLH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PSLLW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSLLW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].ux = r->GPR [ i.Rt ].ux << i.Shift;
	r->GPR [ i.Rd ].uy = r->GPR [ i.Rt ].uy << i.Shift;
	r->GPR [ i.Rd ].uz = r->GPR [ i.Rt ].uz << i.Shift;
	r->GPR [ i.Rd ].uw = r->GPR [ i.Rt ].uw << i.Shift;
	
#if defined INLINE_DEBUG_PSLLW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PSRLH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSRLH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].uh0 = r->GPR [ i.Rt ].uh0 >> i.Shift;
	r->GPR [ i.Rd ].uh1 = r->GPR [ i.Rt ].uh1 >> i.Shift;
	r->GPR [ i.Rd ].uh2 = r->GPR [ i.Rt ].uh2 >> i.Shift;
	r->GPR [ i.Rd ].uh3 = r->GPR [ i.Rt ].uh3 >> i.Shift;
	r->GPR [ i.Rd ].uh4 = r->GPR [ i.Rt ].uh4 >> i.Shift;
	r->GPR [ i.Rd ].uh5 = r->GPR [ i.Rt ].uh5 >> i.Shift;
	r->GPR [ i.Rd ].uh6 = r->GPR [ i.Rt ].uh6 >> i.Shift;
	r->GPR [ i.Rd ].uh7 = r->GPR [ i.Rt ].uh7 >> i.Shift;
	
#if defined INLINE_DEBUG_PSRLH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PSRLW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSRLW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].ux = r->GPR [ i.Rt ].ux >> i.Shift;
	r->GPR [ i.Rd ].uy = r->GPR [ i.Rt ].uy >> i.Shift;
	r->GPR [ i.Rd ].uz = r->GPR [ i.Rt ].uz >> i.Shift;
	r->GPR [ i.Rd ].uw = r->GPR [ i.Rt ].uw >> i.Shift;
	
#if defined INLINE_DEBUG_PSRLW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}


void Execute::PSRAH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSRAH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sh0 = r->GPR [ i.Rt ].sh0 >> i.Shift;
	r->GPR [ i.Rd ].sh1 = r->GPR [ i.Rt ].sh1 >> i.Shift;
	r->GPR [ i.Rd ].sh2 = r->GPR [ i.Rt ].sh2 >> i.Shift;
	r->GPR [ i.Rd ].sh3 = r->GPR [ i.Rt ].sh3 >> i.Shift;
	r->GPR [ i.Rd ].sh4 = r->GPR [ i.Rt ].sh4 >> i.Shift;
	r->GPR [ i.Rd ].sh5 = r->GPR [ i.Rt ].sh5 >> i.Shift;
	r->GPR [ i.Rd ].sh6 = r->GPR [ i.Rt ].sh6 >> i.Shift;
	r->GPR [ i.Rd ].sh7 = r->GPR [ i.Rt ].sh7 >> i.Shift;
	
#if defined INLINE_DEBUG_PSRAH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PSRAW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSRAW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sx = r->GPR [ i.Rt ].sx >> i.Shift;
	r->GPR [ i.Rd ].sy = r->GPR [ i.Rt ].sy >> i.Shift;
	r->GPR [ i.Rd ].sz = r->GPR [ i.Rt ].sz >> i.Shift;
	r->GPR [ i.Rd ].sw = r->GPR [ i.Rt ].sw >> i.Shift;
	
#if defined INLINE_DEBUG_PSRAW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PSLLVW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSLLVW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sq0 = (s32) ( r->GPR [ i.Rt ].uw0 << ( r->GPR [ i.Rs ].uw0 & 0x1f ) );
	r->GPR [ i.Rd ].sq1 = (s32) ( r->GPR [ i.Rt ].uw2 << ( r->GPR [ i.Rs ].uw2 & 0x1f ) );
	
#if defined INLINE_DEBUG_PSLLVW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PSRLVW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSRLVW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sq0 = (s32) ( r->GPR [ i.Rt ].uw0 >> ( r->GPR [ i.Rs ].uw0 & 0x1f ) );
	r->GPR [ i.Rd ].sq1 = (s32) ( r->GPR [ i.Rt ].uw2 >> ( r->GPR [ i.Rs ].uw2 & 0x1f ) );
	
#if defined INLINE_DEBUG_PSRLVW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PSRAVW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSRAVW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sq0 = (s32) ( r->GPR [ i.Rt ].sw0 >> ( r->GPR [ i.Rs ].sw0 & 0x1f ) );
	r->GPR [ i.Rd ].sq1 = (s32) ( r->GPR [ i.Rt ].sw2 >> ( r->GPR [ i.Rs ].sw2 & 0x1f ) );
	
#if defined INLINE_DEBUG_PSRAVW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}


void Execute::PADDB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PADDB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].ub0 = r->GPR [ i.Rs ].ub0 + r->GPR [ i.Rt ].ub0;
	r->GPR [ i.Rd ].ub1 = r->GPR [ i.Rs ].ub1 + r->GPR [ i.Rt ].ub1;
	r->GPR [ i.Rd ].ub2 = r->GPR [ i.Rs ].ub2 + r->GPR [ i.Rt ].ub2;
	r->GPR [ i.Rd ].ub3 = r->GPR [ i.Rs ].ub3 + r->GPR [ i.Rt ].ub3;
	r->GPR [ i.Rd ].ub4 = r->GPR [ i.Rs ].ub4 + r->GPR [ i.Rt ].ub4;
	r->GPR [ i.Rd ].ub5 = r->GPR [ i.Rs ].ub5 + r->GPR [ i.Rt ].ub5;
	r->GPR [ i.Rd ].ub6 = r->GPR [ i.Rs ].ub6 + r->GPR [ i.Rt ].ub6;
	r->GPR [ i.Rd ].ub7 = r->GPR [ i.Rs ].ub7 + r->GPR [ i.Rt ].ub7;
	r->GPR [ i.Rd ].ub8 = r->GPR [ i.Rs ].ub8 + r->GPR [ i.Rt ].ub8;
	r->GPR [ i.Rd ].ub9 = r->GPR [ i.Rs ].ub9 + r->GPR [ i.Rt ].ub9;
	r->GPR [ i.Rd ].ub10 = r->GPR [ i.Rs ].ub10 + r->GPR [ i.Rt ].ub10;
	r->GPR [ i.Rd ].ub11 = r->GPR [ i.Rs ].ub11 + r->GPR [ i.Rt ].ub11;
	r->GPR [ i.Rd ].ub12 = r->GPR [ i.Rs ].ub12 + r->GPR [ i.Rt ].ub12;
	r->GPR [ i.Rd ].ub13 = r->GPR [ i.Rs ].ub13 + r->GPR [ i.Rt ].ub13;
	r->GPR [ i.Rd ].ub14 = r->GPR [ i.Rs ].ub14 + r->GPR [ i.Rt ].ub14;
	r->GPR [ i.Rd ].ub15 = r->GPR [ i.Rs ].ub15 + r->GPR [ i.Rt ].ub15;
	
#if defined INLINE_DEBUG_PADDB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PADDH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PADDH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].uh0 = r->GPR [ i.Rs ].uh0 + r->GPR [ i.Rt ].uh0;
	r->GPR [ i.Rd ].uh1 = r->GPR [ i.Rs ].uh1 + r->GPR [ i.Rt ].uh1;
	r->GPR [ i.Rd ].uh2 = r->GPR [ i.Rs ].uh2 + r->GPR [ i.Rt ].uh2;
	r->GPR [ i.Rd ].uh3 = r->GPR [ i.Rs ].uh3 + r->GPR [ i.Rt ].uh3;
	r->GPR [ i.Rd ].uh4 = r->GPR [ i.Rs ].uh4 + r->GPR [ i.Rt ].uh4;
	r->GPR [ i.Rd ].uh5 = r->GPR [ i.Rs ].uh5 + r->GPR [ i.Rt ].uh5;
	r->GPR [ i.Rd ].uh6 = r->GPR [ i.Rs ].uh6 + r->GPR [ i.Rt ].uh6;
	r->GPR [ i.Rd ].uh7 = r->GPR [ i.Rs ].uh7 + r->GPR [ i.Rt ].uh7;
	
#if defined INLINE_DEBUG_PADDH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PADDW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PADDW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].ux = r->GPR [ i.Rs ].ux + r->GPR [ i.Rt ].ux;
	r->GPR [ i.Rd ].uy = r->GPR [ i.Rs ].uy + r->GPR [ i.Rt ].uy;
	r->GPR [ i.Rd ].uz = r->GPR [ i.Rs ].uz + r->GPR [ i.Rt ].uz;
	r->GPR [ i.Rd ].uw = r->GPR [ i.Rs ].uw + r->GPR [ i.Rt ].uw;
	
#if defined INLINE_DEBUG_PADDW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PSUBB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSUBB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].ub0 = r->GPR [ i.Rs ].ub0 - r->GPR [ i.Rt ].ub0;
	r->GPR [ i.Rd ].ub1 = r->GPR [ i.Rs ].ub1 - r->GPR [ i.Rt ].ub1;
	r->GPR [ i.Rd ].ub2 = r->GPR [ i.Rs ].ub2 - r->GPR [ i.Rt ].ub2;
	r->GPR [ i.Rd ].ub3 = r->GPR [ i.Rs ].ub3 - r->GPR [ i.Rt ].ub3;
	r->GPR [ i.Rd ].ub4 = r->GPR [ i.Rs ].ub4 - r->GPR [ i.Rt ].ub4;
	r->GPR [ i.Rd ].ub5 = r->GPR [ i.Rs ].ub5 - r->GPR [ i.Rt ].ub5;
	r->GPR [ i.Rd ].ub6 = r->GPR [ i.Rs ].ub6 - r->GPR [ i.Rt ].ub6;
	r->GPR [ i.Rd ].ub7 = r->GPR [ i.Rs ].ub7 - r->GPR [ i.Rt ].ub7;
	r->GPR [ i.Rd ].ub8 = r->GPR [ i.Rs ].ub8 - r->GPR [ i.Rt ].ub8;
	r->GPR [ i.Rd ].ub9 = r->GPR [ i.Rs ].ub9 - r->GPR [ i.Rt ].ub9;
	r->GPR [ i.Rd ].ub10 = r->GPR [ i.Rs ].ub10 - r->GPR [ i.Rt ].ub10;
	r->GPR [ i.Rd ].ub11 = r->GPR [ i.Rs ].ub11 - r->GPR [ i.Rt ].ub11;
	r->GPR [ i.Rd ].ub12 = r->GPR [ i.Rs ].ub12 - r->GPR [ i.Rt ].ub12;
	r->GPR [ i.Rd ].ub13 = r->GPR [ i.Rs ].ub13 - r->GPR [ i.Rt ].ub13;
	r->GPR [ i.Rd ].ub14 = r->GPR [ i.Rs ].ub14 - r->GPR [ i.Rt ].ub14;
	r->GPR [ i.Rd ].ub15 = r->GPR [ i.Rs ].ub15 - r->GPR [ i.Rt ].ub15;
	
#if defined INLINE_DEBUG_PSUBB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PSUBH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSUBH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].uh0 = r->GPR [ i.Rs ].uh0 - r->GPR [ i.Rt ].uh0;
	r->GPR [ i.Rd ].uh1 = r->GPR [ i.Rs ].uh1 - r->GPR [ i.Rt ].uh1;
	r->GPR [ i.Rd ].uh2 = r->GPR [ i.Rs ].uh2 - r->GPR [ i.Rt ].uh2;
	r->GPR [ i.Rd ].uh3 = r->GPR [ i.Rs ].uh3 - r->GPR [ i.Rt ].uh3;
	r->GPR [ i.Rd ].uh4 = r->GPR [ i.Rs ].uh4 - r->GPR [ i.Rt ].uh4;
	r->GPR [ i.Rd ].uh5 = r->GPR [ i.Rs ].uh5 - r->GPR [ i.Rt ].uh5;
	r->GPR [ i.Rd ].uh6 = r->GPR [ i.Rs ].uh6 - r->GPR [ i.Rt ].uh6;
	r->GPR [ i.Rd ].uh7 = r->GPR [ i.Rs ].uh7 - r->GPR [ i.Rt ].uh7;
	
#if defined INLINE_DEBUG_PSUBH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PSUBW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSUBW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].ux = r->GPR [ i.Rs ].ux - r->GPR [ i.Rt ].ux;
	r->GPR [ i.Rd ].uy = r->GPR [ i.Rs ].uy - r->GPR [ i.Rt ].uy;
	r->GPR [ i.Rd ].uz = r->GPR [ i.Rs ].uz - r->GPR [ i.Rt ].uz;
	r->GPR [ i.Rd ].uw = r->GPR [ i.Rs ].uw - r->GPR [ i.Rt ].uw;
	
#if defined INLINE_DEBUG_PSUBW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}


void Execute::PADDSB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PADDSB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PADDSB";
	
	s32 sResult32;
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb0 ) + ( (s32) r->GPR [ i.Rt ].sb0 );
	r->GPR [ i.Rd ].sb0 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb1 ) + ( (s32) r->GPR [ i.Rt ].sb1 );
	r->GPR [ i.Rd ].sb1 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb2 ) + ( (s32) r->GPR [ i.Rt ].sb2 );
	r->GPR [ i.Rd ].sb2 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb3 ) + ( (s32) r->GPR [ i.Rt ].sb3 );
	r->GPR [ i.Rd ].sb3 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb4 ) + ( (s32) r->GPR [ i.Rt ].sb4 );
	r->GPR [ i.Rd ].sb4 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb5 ) + ( (s32) r->GPR [ i.Rt ].sb5 );
	r->GPR [ i.Rd ].sb5 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb6 ) + ( (s32) r->GPR [ i.Rt ].sb6 );
	r->GPR [ i.Rd ].sb6 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb7 ) + ( (s32) r->GPR [ i.Rt ].sb7 );
	r->GPR [ i.Rd ].sb7 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb8 ) + ( (s32) r->GPR [ i.Rt ].sb8 );
	r->GPR [ i.Rd ].sb8 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb9 ) + ( (s32) r->GPR [ i.Rt ].sb9 );
	r->GPR [ i.Rd ].sb9 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb10 ) + ( (s32) r->GPR [ i.Rt ].sb10 );
	r->GPR [ i.Rd ].sb10 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb11 ) + ( (s32) r->GPR [ i.Rt ].sb11 );
	r->GPR [ i.Rd ].sb11 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb12 ) + ( (s32) r->GPR [ i.Rt ].sb12 );
	r->GPR [ i.Rd ].sb12 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb13 ) + ( (s32) r->GPR [ i.Rt ].sb13 );
	r->GPR [ i.Rd ].sb13 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb14 ) + ( (s32) r->GPR [ i.Rt ].sb14 );
	r->GPR [ i.Rd ].sb14 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb15 ) + ( (s32) r->GPR [ i.Rt ].sb15 );
	r->GPR [ i.Rd ].sb15 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
}

void Execute::PADDSH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PADDSH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PADDSH";
	
	s32 sResult32;
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh0 ) + ( (s32) r->GPR [ i.Rt ].sh0 );
	r->GPR [ i.Rd ].sh0 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh1 ) + ( (s32) r->GPR [ i.Rt ].sh1 );
	r->GPR [ i.Rd ].sh1 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh2 ) + ( (s32) r->GPR [ i.Rt ].sh2 );
	r->GPR [ i.Rd ].sh2 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh3 ) + ( (s32) r->GPR [ i.Rt ].sh3 );
	r->GPR [ i.Rd ].sh3 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh4 ) + ( (s32) r->GPR [ i.Rt ].sh4 );
	r->GPR [ i.Rd ].sh4 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh5 ) + ( (s32) r->GPR [ i.Rt ].sh5 );
	r->GPR [ i.Rd ].sh5 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh6 ) + ( (s32) r->GPR [ i.Rt ].sh6 );
	r->GPR [ i.Rd ].sh6 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh7 ) + ( (s32) r->GPR [ i.Rt ].sh7 );
	r->GPR [ i.Rd ].sh7 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
}

void Execute::PADDSW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PADDSW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PADDSW";
	
	s64 sResult64;
	
	sResult64 = ( (s64) r->GPR [ i.Rs ].sw0 ) + ( (s64) r->GPR [ i.Rt ].sw0 );
	r->GPR [ i.Rd ].sw0 = ( ( sResult64 > 0x7fffffffLL ) ? 0x7fffffff : ( ( sResult64 < -0x80000000LL ) ? -0x80000000 : sResult64 ) );
	
	sResult64 = ( (s64) r->GPR [ i.Rs ].sw1 ) + ( (s64) r->GPR [ i.Rt ].sw1 );
	r->GPR [ i.Rd ].sw1 = ( ( sResult64 > 0x7fffffffLL ) ? 0x7fffffff : ( ( sResult64 < -0x80000000LL ) ? -0x80000000 : sResult64 ) );
	
	sResult64 = ( (s64) r->GPR [ i.Rs ].sw2 ) + ( (s64) r->GPR [ i.Rt ].sw2 );
	r->GPR [ i.Rd ].sw2 = ( ( sResult64 > 0x7fffffffLL ) ? 0x7fffffff : ( ( sResult64 < -0x80000000LL ) ? -0x80000000 : sResult64 ) );
	
	sResult64 = ( (s64) r->GPR [ i.Rs ].sw3 ) + ( (s64) r->GPR [ i.Rt ].sw3 );
	r->GPR [ i.Rd ].sw3 = ( ( sResult64 > 0x7fffffffLL ) ? 0x7fffffff : ( ( sResult64 < -0x80000000LL ) ? -0x80000000 : sResult64 ) );
	
#if defined INLINE_DEBUG_PADDSW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}


void Execute::PSUBSB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSUBSB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PSUBSB";
	
	s32 sResult32;
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb0 ) - ( (s32) r->GPR [ i.Rt ].sb0 );
	r->GPR [ i.Rd ].sb0 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb1 ) - ( (s32) r->GPR [ i.Rt ].sb1 );
	r->GPR [ i.Rd ].sb1 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb2 ) - ( (s32) r->GPR [ i.Rt ].sb2 );
	r->GPR [ i.Rd ].sb2 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb3 ) - ( (s32) r->GPR [ i.Rt ].sb3 );
	r->GPR [ i.Rd ].sb3 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb4 ) - ( (s32) r->GPR [ i.Rt ].sb4 );
	r->GPR [ i.Rd ].sb4 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb5 ) - ( (s32) r->GPR [ i.Rt ].sb5 );
	r->GPR [ i.Rd ].sb5 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb6 ) - ( (s32) r->GPR [ i.Rt ].sb6 );
	r->GPR [ i.Rd ].sb6 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb7 ) - ( (s32) r->GPR [ i.Rt ].sb7 );
	r->GPR [ i.Rd ].sb7 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb8 ) - ( (s32) r->GPR [ i.Rt ].sb8 );
	r->GPR [ i.Rd ].sb8 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb9 ) - ( (s32) r->GPR [ i.Rt ].sb9 );
	r->GPR [ i.Rd ].sb9 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb10 ) - ( (s32) r->GPR [ i.Rt ].sb10 );
	r->GPR [ i.Rd ].sb10 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb11 ) - ( (s32) r->GPR [ i.Rt ].sb11 );
	r->GPR [ i.Rd ].sb11 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb12 ) - ( (s32) r->GPR [ i.Rt ].sb12 );
	r->GPR [ i.Rd ].sb12 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb13 ) - ( (s32) r->GPR [ i.Rt ].sb13 );
	r->GPR [ i.Rd ].sb13 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb14 ) - ( (s32) r->GPR [ i.Rt ].sb14 );
	r->GPR [ i.Rd ].sb14 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sb15 ) - ( (s32) r->GPR [ i.Rt ].sb15 );
	r->GPR [ i.Rd ].sb15 = ( ( sResult32 > 0x7f ) ? 0x7f : ( ( sResult32 < -0x80 ) ? -0x80 : sResult32 ) );
}


void Execute::PSUBSH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSUBSH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PSUBSH";
	
	s32 sResult32;
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh0 ) - ( (s32) r->GPR [ i.Rt ].sh0 );
	r->GPR [ i.Rd ].sh0 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh1 ) - ( (s32) r->GPR [ i.Rt ].sh1 );
	r->GPR [ i.Rd ].sh1 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh2 ) - ( (s32) r->GPR [ i.Rt ].sh2 );
	r->GPR [ i.Rd ].sh2 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh3 ) - ( (s32) r->GPR [ i.Rt ].sh3 );
	r->GPR [ i.Rd ].sh3 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh4 ) - ( (s32) r->GPR [ i.Rt ].sh4 );
	r->GPR [ i.Rd ].sh4 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh5 ) - ( (s32) r->GPR [ i.Rt ].sh5 );
	r->GPR [ i.Rd ].sh5 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh6 ) - ( (s32) r->GPR [ i.Rt ].sh6 );
	r->GPR [ i.Rd ].sh6 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
	
	sResult32 = ( (s32) r->GPR [ i.Rs ].sh7 ) - ( (s32) r->GPR [ i.Rt ].sh7 );
	r->GPR [ i.Rd ].sh7 = ( ( sResult32 > 0x7fff ) ? 0x7fff : ( ( sResult32 < -0x8000 ) ? -0x8000 : sResult32 ) );
}


void Execute::PSUBSW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSUBSW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PSUBSW";
	
	s64 sResult64;
	
	sResult64 = ( (s64) r->GPR [ i.Rs ].sw0 ) - ( (s64) r->GPR [ i.Rt ].sw0 );
	r->GPR [ i.Rd ].sw0 = ( ( sResult64 > 0x7fffffffLL ) ? 0x7fffffff : ( ( sResult64 < -0x80000000LL ) ? -0x80000000 : sResult64 ) );
	
	sResult64 = ( (s64) r->GPR [ i.Rs ].sw1 ) - ( (s64) r->GPR [ i.Rt ].sw1 );
	r->GPR [ i.Rd ].sw1 = ( ( sResult64 > 0x7fffffffLL ) ? 0x7fffffff : ( ( sResult64 < -0x80000000LL ) ? -0x80000000 : sResult64 ) );
	
	sResult64 = ( (s64) r->GPR [ i.Rs ].sw2 ) - ( (s64) r->GPR [ i.Rt ].sw2 );
	r->GPR [ i.Rd ].sw2 = ( ( sResult64 > 0x7fffffffLL ) ? 0x7fffffff : ( ( sResult64 < -0x80000000LL ) ? -0x80000000 : sResult64 ) );
	
	sResult64 = ( (s64) r->GPR [ i.Rs ].sw3 ) - ( (s64) r->GPR [ i.Rt ].sw3 );
	r->GPR [ i.Rd ].sw3 = ( ( sResult64 > 0x7fffffffLL ) ? 0x7fffffff : ( ( sResult64 < -0x80000000LL ) ? -0x80000000 : sResult64 ) );
}


void Execute::PADDUB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PADDUB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].ub0 = ( ( ( (u64) r->GPR [ i.Rs ].ub0 ) + ( (u64) r->GPR [ i.Rt ].ub0 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub0 + r->GPR [ i.Rt ].ub0 ) );
	r->GPR [ i.Rd ].ub1 = ( ( ( (u64) r->GPR [ i.Rs ].ub1 ) + ( (u64) r->GPR [ i.Rt ].ub1 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub1 + r->GPR [ i.Rt ].ub1 ) );
	r->GPR [ i.Rd ].ub2 = ( ( ( (u64) r->GPR [ i.Rs ].ub2 ) + ( (u64) r->GPR [ i.Rt ].ub2 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub2 + r->GPR [ i.Rt ].ub2 ) );
	r->GPR [ i.Rd ].ub3 = ( ( ( (u64) r->GPR [ i.Rs ].ub3 ) + ( (u64) r->GPR [ i.Rt ].ub3 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub3 + r->GPR [ i.Rt ].ub3 ) );
	r->GPR [ i.Rd ].ub4 = ( ( ( (u64) r->GPR [ i.Rs ].ub4 ) + ( (u64) r->GPR [ i.Rt ].ub4 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub4 + r->GPR [ i.Rt ].ub4 ) );
	r->GPR [ i.Rd ].ub5 = ( ( ( (u64) r->GPR [ i.Rs ].ub5 ) + ( (u64) r->GPR [ i.Rt ].ub5 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub5 + r->GPR [ i.Rt ].ub5 ) );
	r->GPR [ i.Rd ].ub6 = ( ( ( (u64) r->GPR [ i.Rs ].ub6 ) + ( (u64) r->GPR [ i.Rt ].ub6 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub6 + r->GPR [ i.Rt ].ub6 ) );
	r->GPR [ i.Rd ].ub7 = ( ( ( (u64) r->GPR [ i.Rs ].ub7 ) + ( (u64) r->GPR [ i.Rt ].ub7 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub7 + r->GPR [ i.Rt ].ub7 ) );
	r->GPR [ i.Rd ].ub8 = ( ( ( (u64) r->GPR [ i.Rs ].ub8 ) + ( (u64) r->GPR [ i.Rt ].ub8 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub8 + r->GPR [ i.Rt ].ub8 ) );
	r->GPR [ i.Rd ].ub9 = ( ( ( (u64) r->GPR [ i.Rs ].ub9 ) + ( (u64) r->GPR [ i.Rt ].ub9 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub9 + r->GPR [ i.Rt ].ub9 ) );
	r->GPR [ i.Rd ].ub10 = ( ( ( (u64) r->GPR [ i.Rs ].ub10 ) + ( (u64) r->GPR [ i.Rt ].ub10 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub10 + r->GPR [ i.Rt ].ub10 ) );
	r->GPR [ i.Rd ].ub11 = ( ( ( (u64) r->GPR [ i.Rs ].ub11 ) + ( (u64) r->GPR [ i.Rt ].ub11 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub11 + r->GPR [ i.Rt ].ub11 ) );
	r->GPR [ i.Rd ].ub12 = ( ( ( (u64) r->GPR [ i.Rs ].ub12 ) + ( (u64) r->GPR [ i.Rt ].ub12 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub12 + r->GPR [ i.Rt ].ub12 ) );
	r->GPR [ i.Rd ].ub13 = ( ( ( (u64) r->GPR [ i.Rs ].ub13 ) + ( (u64) r->GPR [ i.Rt ].ub13 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub13 + r->GPR [ i.Rt ].ub13 ) );
	r->GPR [ i.Rd ].ub14 = ( ( ( (u64) r->GPR [ i.Rs ].ub14 ) + ( (u64) r->GPR [ i.Rt ].ub14 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub14 + r->GPR [ i.Rt ].ub14 ) );
	r->GPR [ i.Rd ].ub15 = ( ( ( (u64) r->GPR [ i.Rs ].ub15 ) + ( (u64) r->GPR [ i.Rt ].ub15 ) ) > 0xffULL ? 0xff : ( r->GPR [ i.Rs ].ub15 + r->GPR [ i.Rt ].ub15 ) );
	
#if defined INLINE_DEBUG_PADDUB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PADDUH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PADDUH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].uh0 = ( ( ( (u64) r->GPR [ i.Rs ].uh0 ) + ( (u64) r->GPR [ i.Rt ].uh0 ) ) > 0xffffULL ? 0xffff : ( r->GPR [ i.Rs ].uh0 + r->GPR [ i.Rt ].uh0 ) );
	r->GPR [ i.Rd ].uh1 = ( ( ( (u64) r->GPR [ i.Rs ].uh1 ) + ( (u64) r->GPR [ i.Rt ].uh1 ) ) > 0xffffULL ? 0xffff : ( r->GPR [ i.Rs ].uh1 + r->GPR [ i.Rt ].uh1 ) );
	r->GPR [ i.Rd ].uh2 = ( ( ( (u64) r->GPR [ i.Rs ].uh2 ) + ( (u64) r->GPR [ i.Rt ].uh2 ) ) > 0xffffULL ? 0xffff : ( r->GPR [ i.Rs ].uh2 + r->GPR [ i.Rt ].uh2 ) );
	r->GPR [ i.Rd ].uh3 = ( ( ( (u64) r->GPR [ i.Rs ].uh3 ) + ( (u64) r->GPR [ i.Rt ].uh3 ) ) > 0xffffULL ? 0xffff : ( r->GPR [ i.Rs ].uh3 + r->GPR [ i.Rt ].uh3 ) );
	r->GPR [ i.Rd ].uh4 = ( ( ( (u64) r->GPR [ i.Rs ].uh4 ) + ( (u64) r->GPR [ i.Rt ].uh4 ) ) > 0xffffULL ? 0xffff : ( r->GPR [ i.Rs ].uh4 + r->GPR [ i.Rt ].uh4 ) );
	r->GPR [ i.Rd ].uh5 = ( ( ( (u64) r->GPR [ i.Rs ].uh5 ) + ( (u64) r->GPR [ i.Rt ].uh5 ) ) > 0xffffULL ? 0xffff : ( r->GPR [ i.Rs ].uh5 + r->GPR [ i.Rt ].uh5 ) );
	r->GPR [ i.Rd ].uh6 = ( ( ( (u64) r->GPR [ i.Rs ].uh6 ) + ( (u64) r->GPR [ i.Rt ].uh6 ) ) > 0xffffULL ? 0xffff : ( r->GPR [ i.Rs ].uh6 + r->GPR [ i.Rt ].uh6 ) );
	r->GPR [ i.Rd ].uh7 = ( ( ( (u64) r->GPR [ i.Rs ].uh7 ) + ( (u64) r->GPR [ i.Rt ].uh7 ) ) > 0xffffULL ? 0xffff : ( r->GPR [ i.Rs ].uh7 + r->GPR [ i.Rt ].uh7 ) );
	
#if defined INLINE_DEBUG_PADDUH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PADDUW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PADDUW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// rd = rs + rt
	r->GPR [ i.Rd ].uw0 = ( ( ( (u64) r->GPR [ i.Rs ].uw0 ) + ( (u64) r->GPR [ i.Rt ].uw0 ) ) > 0xffffffffULL ? 0xffffffff : ( r->GPR [ i.Rs ].uw0 + r->GPR [ i.Rt ].uw0 ) );
	r->GPR [ i.Rd ].uw1 = ( ( ( (u64) r->GPR [ i.Rs ].uw1 ) + ( (u64) r->GPR [ i.Rt ].uw1 ) ) > 0xffffffffULL ? 0xffffffff : ( r->GPR [ i.Rs ].uw1 + r->GPR [ i.Rt ].uw1 ) );
	r->GPR [ i.Rd ].uw2 = ( ( ( (u64) r->GPR [ i.Rs ].uw2 ) + ( (u64) r->GPR [ i.Rt ].uw2 ) ) > 0xffffffffULL ? 0xffffffff : ( r->GPR [ i.Rs ].uw2 + r->GPR [ i.Rt ].uw2 ) );
	r->GPR [ i.Rd ].uw3 = ( ( ( (u64) r->GPR [ i.Rs ].uw3 ) + ( (u64) r->GPR [ i.Rt ].uw3 ) ) > 0xffffffffULL ? 0xffffffff : ( r->GPR [ i.Rs ].uw3 + r->GPR [ i.Rt ].uw3 ) );
	
#if defined INLINE_DEBUG_PADDUW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}


void Execute::PSUBUB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSUBUB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PSUBUB";
	
	u32 uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub0 ) - ( (u32) r->GPR [ i.Rt ].ub0 );
	r->GPR [ i.Rd ].ub0 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub1 ) - ( (u32) r->GPR [ i.Rt ].ub1 );
	r->GPR [ i.Rd ].ub1 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub2 ) - ( (u32) r->GPR [ i.Rt ].ub2 );
	r->GPR [ i.Rd ].ub2 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub3 ) - ( (u32) r->GPR [ i.Rt ].ub3 );
	r->GPR [ i.Rd ].ub3 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub4 ) - ( (u32) r->GPR [ i.Rt ].ub4 );
	r->GPR [ i.Rd ].ub4 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub5 ) - ( (u32) r->GPR [ i.Rt ].ub5 );
	r->GPR [ i.Rd ].ub5 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub6 ) - ( (u32) r->GPR [ i.Rt ].ub6 );
	r->GPR [ i.Rd ].ub6 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub7 ) - ( (u32) r->GPR [ i.Rt ].ub7 );
	r->GPR [ i.Rd ].ub7 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub8 ) - ( (u32) r->GPR [ i.Rt ].ub8 );
	r->GPR [ i.Rd ].ub8 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub9 ) - ( (u32) r->GPR [ i.Rt ].ub9 );
	r->GPR [ i.Rd ].ub9 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub10 ) - ( (u32) r->GPR [ i.Rt ].ub10 );
	r->GPR [ i.Rd ].ub10 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub11 ) - ( (u32) r->GPR [ i.Rt ].ub11 );
	r->GPR [ i.Rd ].ub11 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub12 ) - ( (u32) r->GPR [ i.Rt ].ub12 );
	r->GPR [ i.Rd ].ub12 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub13 ) - ( (u32) r->GPR [ i.Rt ].ub13 );
	r->GPR [ i.Rd ].ub13 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub14 ) - ( (u32) r->GPR [ i.Rt ].ub14 );
	r->GPR [ i.Rd ].ub14 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].ub15 ) - ( (u32) r->GPR [ i.Rt ].ub15 );
	r->GPR [ i.Rd ].ub15 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;

}

void Execute::PSUBUH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSUBUH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PSUBUH";
	
	u32 uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].uh0 ) - ( (u32) r->GPR [ i.Rt ].uh0 );
	r->GPR [ i.Rd ].uh0 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].uh1 ) - ( (u32) r->GPR [ i.Rt ].uh1 );
	r->GPR [ i.Rd ].uh1 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].uh2 ) - ( (u32) r->GPR [ i.Rt ].uh2 );
	r->GPR [ i.Rd ].uh2 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].uh3 ) - ( (u32) r->GPR [ i.Rt ].uh3 );
	r->GPR [ i.Rd ].uh3 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].uh4 ) - ( (u32) r->GPR [ i.Rt ].uh4 );
	r->GPR [ i.Rd ].uh4 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].uh5 ) - ( (u32) r->GPR [ i.Rt ].uh5 );
	r->GPR [ i.Rd ].uh5 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].uh6 ) - ( (u32) r->GPR [ i.Rt ].uh6 );
	r->GPR [ i.Rd ].uh6 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
	
	uResult32 = ( (u32) r->GPR [ i.Rs ].uh7 ) - ( (u32) r->GPR [ i.Rt ].uh7 );
	r->GPR [ i.Rd ].uh7 = ( ( (s32) uResult32 ) < 0 ) ? 0 : uResult32;
}

void Execute::PSUBUW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PSUBUW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PSUBUW";
	
	u64 uResult64;
	
	uResult64 = ( (u64) r->GPR [ i.Rs ].uw0 ) - ( (u64) r->GPR [ i.Rt ].uw0 );
	r->GPR [ i.Rd ].uw0 = ( ( (s64) uResult64 ) < 0 ) ? 0 : uResult64;
	
	uResult64 = ( (u64) r->GPR [ i.Rs ].uw1 ) - ( (u64) r->GPR [ i.Rt ].uw1 );
	r->GPR [ i.Rd ].uw1 = ( ( (s64) uResult64 ) < 0 ) ? 0 : uResult64;
	
	uResult64 = ( (u64) r->GPR [ i.Rs ].uw2 ) - ( (u64) r->GPR [ i.Rt ].uw2 );
	r->GPR [ i.Rd ].uw2 = ( ( (s64) uResult64 ) < 0 ) ? 0 : uResult64;
	
	uResult64 = ( (u64) r->GPR [ i.Rs ].uw3 ) - ( (u64) r->GPR [ i.Rt ].uw3 );
	r->GPR [ i.Rd ].uw3 = ( ( (s64) uResult64 ) < 0 ) ? 0 : uResult64;
}



void Execute::PMAXH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMAXH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sh0 = ( r->GPR [ i.Rs ].sh0 > r->GPR [ i.Rt ].sh0 ) ? r->GPR [ i.Rs ].sh0 : r->GPR [ i.Rt ].sh0;
	r->GPR [ i.Rd ].sh1 = ( r->GPR [ i.Rs ].sh1 > r->GPR [ i.Rt ].sh1 ) ? r->GPR [ i.Rs ].sh1 : r->GPR [ i.Rt ].sh1;
	r->GPR [ i.Rd ].sh2 = ( r->GPR [ i.Rs ].sh2 > r->GPR [ i.Rt ].sh2 ) ? r->GPR [ i.Rs ].sh2 : r->GPR [ i.Rt ].sh2;
	r->GPR [ i.Rd ].sh3 = ( r->GPR [ i.Rs ].sh3 > r->GPR [ i.Rt ].sh3 ) ? r->GPR [ i.Rs ].sh3 : r->GPR [ i.Rt ].sh3;
	r->GPR [ i.Rd ].sh4 = ( r->GPR [ i.Rs ].sh4 > r->GPR [ i.Rt ].sh4 ) ? r->GPR [ i.Rs ].sh4 : r->GPR [ i.Rt ].sh4;
	r->GPR [ i.Rd ].sh5 = ( r->GPR [ i.Rs ].sh5 > r->GPR [ i.Rt ].sh5 ) ? r->GPR [ i.Rs ].sh5 : r->GPR [ i.Rt ].sh5;
	r->GPR [ i.Rd ].sh6 = ( r->GPR [ i.Rs ].sh6 > r->GPR [ i.Rt ].sh6 ) ? r->GPR [ i.Rs ].sh6 : r->GPR [ i.Rt ].sh6;
	r->GPR [ i.Rd ].sh7 = ( r->GPR [ i.Rs ].sh7 > r->GPR [ i.Rt ].sh7 ) ? r->GPR [ i.Rs ].sh7 : r->GPR [ i.Rt ].sh7;
	
#if defined INLINE_DEBUG_PMAXH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PMAXW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMAXW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sw0 = ( r->GPR [ i.Rs ].sw0 > r->GPR [ i.Rt ].sw0 ) ? r->GPR [ i.Rs ].sw0 : r->GPR [ i.Rt ].sw0;
	r->GPR [ i.Rd ].sw1 = ( r->GPR [ i.Rs ].sw1 > r->GPR [ i.Rt ].sw1 ) ? r->GPR [ i.Rs ].sw1 : r->GPR [ i.Rt ].sw1;
	r->GPR [ i.Rd ].sw2 = ( r->GPR [ i.Rs ].sw2 > r->GPR [ i.Rt ].sw2 ) ? r->GPR [ i.Rs ].sw2 : r->GPR [ i.Rt ].sw2;
	r->GPR [ i.Rd ].sw3 = ( r->GPR [ i.Rs ].sw3 > r->GPR [ i.Rt ].sw3 ) ? r->GPR [ i.Rs ].sw3 : r->GPR [ i.Rt ].sw3;
	
#if defined INLINE_DEBUG_PMAXW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PMINH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMINH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sh0 = ( r->GPR [ i.Rs ].sh0 < r->GPR [ i.Rt ].sh0 ) ? r->GPR [ i.Rs ].sh0 : r->GPR [ i.Rt ].sh0;
	r->GPR [ i.Rd ].sh1 = ( r->GPR [ i.Rs ].sh1 < r->GPR [ i.Rt ].sh1 ) ? r->GPR [ i.Rs ].sh1 : r->GPR [ i.Rt ].sh1;
	r->GPR [ i.Rd ].sh2 = ( r->GPR [ i.Rs ].sh2 < r->GPR [ i.Rt ].sh2 ) ? r->GPR [ i.Rs ].sh2 : r->GPR [ i.Rt ].sh2;
	r->GPR [ i.Rd ].sh3 = ( r->GPR [ i.Rs ].sh3 < r->GPR [ i.Rt ].sh3 ) ? r->GPR [ i.Rs ].sh3 : r->GPR [ i.Rt ].sh3;
	r->GPR [ i.Rd ].sh4 = ( r->GPR [ i.Rs ].sh4 < r->GPR [ i.Rt ].sh4 ) ? r->GPR [ i.Rs ].sh4 : r->GPR [ i.Rt ].sh4;
	r->GPR [ i.Rd ].sh5 = ( r->GPR [ i.Rs ].sh5 < r->GPR [ i.Rt ].sh5 ) ? r->GPR [ i.Rs ].sh5 : r->GPR [ i.Rt ].sh5;
	r->GPR [ i.Rd ].sh6 = ( r->GPR [ i.Rs ].sh6 < r->GPR [ i.Rt ].sh6 ) ? r->GPR [ i.Rs ].sh6 : r->GPR [ i.Rt ].sh6;
	r->GPR [ i.Rd ].sh7 = ( r->GPR [ i.Rs ].sh7 < r->GPR [ i.Rt ].sh7 ) ? r->GPR [ i.Rs ].sh7 : r->GPR [ i.Rt ].sh7;
	
#if defined INLINE_DEBUG_PMINH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PMINW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMINW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sw0 = ( r->GPR [ i.Rs ].sw0 < r->GPR [ i.Rt ].sw0 ) ? r->GPR [ i.Rs ].sw0 : r->GPR [ i.Rt ].sw0;
	r->GPR [ i.Rd ].sw1 = ( r->GPR [ i.Rs ].sw1 < r->GPR [ i.Rt ].sw1 ) ? r->GPR [ i.Rs ].sw1 : r->GPR [ i.Rt ].sw1;
	r->GPR [ i.Rd ].sw2 = ( r->GPR [ i.Rs ].sw2 < r->GPR [ i.Rt ].sw2 ) ? r->GPR [ i.Rs ].sw2 : r->GPR [ i.Rt ].sw2;
	r->GPR [ i.Rd ].sw3 = ( r->GPR [ i.Rs ].sw3 < r->GPR [ i.Rt ].sw3 ) ? r->GPR [ i.Rs ].sw3 : r->GPR [ i.Rt ].sw3;
	
#if defined INLINE_DEBUG_PMINW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}




void Execute::PPACB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PPACB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// note: must use this order
	r->GPR [ i.Rd ].ub1 = r->GPR [ i.Rt ].ub2;
	r->GPR [ i.Rd ].ub3 = r->GPR [ i.Rt ].ub6;
	r->GPR [ i.Rd ].ub5 = r->GPR [ i.Rt ].ub10;
	r->GPR [ i.Rd ].ub7 = r->GPR [ i.Rt ].ub14;
	r->GPR [ i.Rd ].ub9 = r->GPR [ i.Rs ].ub2;
	r->GPR [ i.Rd ].ub11 = r->GPR [ i.Rs ].ub6;
	r->GPR [ i.Rd ].ub13 = r->GPR [ i.Rs ].ub10;
	r->GPR [ i.Rd ].ub15 = r->GPR [ i.Rs ].ub14;
	
	r->GPR [ i.Rd ].ub2 = r->GPR [ i.Rt ].ub4;
	r->GPR [ i.Rd ].ub6 = r->GPR [ i.Rt ].ub12;
	r->GPR [ i.Rd ].ub10 = r->GPR [ i.Rs ].ub4;
	r->GPR [ i.Rd ].ub14 = r->GPR [ i.Rs ].ub12;
	
	r->GPR [ i.Rd ].ub12 = r->GPR [ i.Rs ].ub8;
	r->GPR [ i.Rd ].ub4 = r->GPR [ i.Rt ].ub8;
	r->GPR [ i.Rd ].ub8 = r->GPR [ i.Rs ].ub0;
	r->GPR [ i.Rd ].ub0 = r->GPR [ i.Rt ].ub0;
	
	
#if defined INLINE_DEBUG_PPACB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PPACH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PPACH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// note: must use this order
	
	// write into odd indexes first
	r->GPR [ i.Rd ].uh1 = r->GPR [ i.Rt ].uh2;
	r->GPR [ i.Rd ].uh3 = r->GPR [ i.Rt ].uh6;
	r->GPR [ i.Rd ].uh5 = r->GPR [ i.Rs ].uh2;
	r->GPR [ i.Rd ].uh7 = r->GPR [ i.Rs ].uh6;
	
	// h2 and h6 have both been written from both rs and rt
	r->GPR [ i.Rd ].uh2 = r->GPR [ i.Rt ].uh4;
	r->GPR [ i.Rd ].uh6 = r->GPR [ i.Rs ].uh4;
	
	// can only store into h0 LAST
	r->GPR [ i.Rd ].uh4 = r->GPR [ i.Rs ].uh0;
	r->GPR [ i.Rd ].uh0 = r->GPR [ i.Rt ].uh0;
	
#if defined INLINE_DEBUG_PPACH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PPACW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PPACW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// note: must use this order
	
	// write into odd indexes first
	r->GPR [ i.Rd ].uw1 = r->GPR [ i.Rt ].uw2;
	r->GPR [ i.Rd ].uw3 = r->GPR [ i.Rs ].uw2;
	
	// can only store into h0 LAST
	r->GPR [ i.Rd ].uw2 = r->GPR [ i.Rs ].uw0;
	r->GPR [ i.Rd ].uw0 = r->GPR [ i.Rt ].uw0;
	
#if defined INLINE_DEBUG_PPACW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}


void Execute::PEXT5 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PEXT5 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].uq0 = ( ( r->GPR [ i.Rt ].uq0 & 0x1f0000001fULL ) << 3 ) | ( ( r->GPR [ i.Rt ].uq0 & 0x3e0000003e0ULL ) << 6 ) | ( ( r->GPR [ i.Rt ].uq0 & 0x7c0000007c00ULL ) << 9 ) | ( ( r->GPR [ i.Rt ].uq0 & 0x800000008000ULL ) << 16 );
	r->GPR [ i.Rd ].uq1 = ( ( r->GPR [ i.Rt ].uq1 & 0x1f0000001fULL ) << 3 ) | ( ( r->GPR [ i.Rt ].uq1 & 0x3e0000003e0ULL ) << 6 ) | ( ( r->GPR [ i.Rt ].uq1 & 0x7c0000007c00ULL ) << 9 ) | ( ( r->GPR [ i.Rt ].uq1 & 0x800000008000ULL ) << 16 );
	
#if defined INLINE_DEBUG_PEXT5 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PPAC5 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PPAC5 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	u64 temp;
	temp = ( ( r->GPR [ i.Rt ].uq0 & 0xf8000000f8ULL ) >> 3 ) | ( ( r->GPR [ i.Rt ].uq0 & 0xf8000000f800ULL ) >> 6 ) | ( ( r->GPR [ i.Rt ].uq0 & 0xf8000000f80000ULL ) >> 9 ) | ( ( r->GPR [ i.Rt ].uq0 & 0x8000000080000000ULL ) >> 16 );
	r->GPR [ i.Rd ].uh0 = (u16) temp;
	r->GPR [ i.Rd ].uh2 = (u16) ( temp >> 32 );
	temp = ( ( r->GPR [ i.Rt ].uq1 & 0xf8000000f8ULL ) >> 3 ) | ( ( r->GPR [ i.Rt ].uq1 & 0xf8000000f800ULL ) >> 6 ) | ( ( r->GPR [ i.Rt ].uq1 & 0xf8000000f80000ULL ) >> 9 ) | ( ( r->GPR [ i.Rt ].uq1 & 0x8000000080000000ULL ) >> 16 );
	r->GPR [ i.Rd ].uh4 = (u16) temp;
	r->GPR [ i.Rd ].uh6 = (u16) ( temp >> 32 );
	
	// the halfwords 1,3,5,7 are set to zero
	r->GPR [ i.Rd ].uh1 = 0;
	r->GPR [ i.Rd ].uh3 = 0;
	r->GPR [ i.Rd ].uh5 = 0;
	r->GPR [ i.Rd ].uh7 = 0;
	
#if defined INLINE_DEBUG_PPAC5 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}


void Execute::PCGTB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PCGTB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sb0 = ( r->GPR [ i.Rs ].sb0 > r->GPR [ i.Rt ].sb0 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb1 = ( r->GPR [ i.Rs ].sb1 > r->GPR [ i.Rt ].sb1 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb2 = ( r->GPR [ i.Rs ].sb2 > r->GPR [ i.Rt ].sb2 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb3 = ( r->GPR [ i.Rs ].sb3 > r->GPR [ i.Rt ].sb3 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb4 = ( r->GPR [ i.Rs ].sb4 > r->GPR [ i.Rt ].sb4 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb5 = ( r->GPR [ i.Rs ].sb5 > r->GPR [ i.Rt ].sb5 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb6 = ( r->GPR [ i.Rs ].sb6 > r->GPR [ i.Rt ].sb6 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb7 = ( r->GPR [ i.Rs ].sb7 > r->GPR [ i.Rt ].sb7 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb8 = ( r->GPR [ i.Rs ].sb8 > r->GPR [ i.Rt ].sb8 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb9 = ( r->GPR [ i.Rs ].sb9 > r->GPR [ i.Rt ].sb9 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb10 = ( r->GPR [ i.Rs ].sb10 > r->GPR [ i.Rt ].sb10 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb11 = ( r->GPR [ i.Rs ].sb11 > r->GPR [ i.Rt ].sb11 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb12 = ( r->GPR [ i.Rs ].sb12 > r->GPR [ i.Rt ].sb12 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb13 = ( r->GPR [ i.Rs ].sb13 > r->GPR [ i.Rt ].sb13 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb14 = ( r->GPR [ i.Rs ].sb14 > r->GPR [ i.Rt ].sb14 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb15 = ( r->GPR [ i.Rs ].sb15 > r->GPR [ i.Rt ].sb15 ) ? 0xff : 0;
	
#if defined INLINE_DEBUG_PCGTB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PCGTH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PCGTH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sh0 = ( r->GPR [ i.Rs ].sh0 > r->GPR [ i.Rt ].sh0 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh1 = ( r->GPR [ i.Rs ].sh1 > r->GPR [ i.Rt ].sh1 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh2 = ( r->GPR [ i.Rs ].sh2 > r->GPR [ i.Rt ].sh2 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh3 = ( r->GPR [ i.Rs ].sh3 > r->GPR [ i.Rt ].sh3 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh4 = ( r->GPR [ i.Rs ].sh4 > r->GPR [ i.Rt ].sh4 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh5 = ( r->GPR [ i.Rs ].sh5 > r->GPR [ i.Rt ].sh5 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh6 = ( r->GPR [ i.Rs ].sh6 > r->GPR [ i.Rt ].sh6 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh7 = ( r->GPR [ i.Rs ].sh7 > r->GPR [ i.Rt ].sh7 ) ? 0xffff : 0;
	
#if defined INLINE_DEBUG_PCGTH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PCGTW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PCGTW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sw0 = ( r->GPR [ i.Rs ].sw0 > r->GPR [ i.Rt ].sw0 ) ? 0xffffffff : 0;
	r->GPR [ i.Rd ].sw1 = ( r->GPR [ i.Rs ].sw1 > r->GPR [ i.Rt ].sw1 ) ? 0xffffffff : 0;
	r->GPR [ i.Rd ].sw2 = ( r->GPR [ i.Rs ].sw2 > r->GPR [ i.Rt ].sw2 ) ? 0xffffffff : 0;
	r->GPR [ i.Rd ].sw3 = ( r->GPR [ i.Rs ].sw3 > r->GPR [ i.Rt ].sw3 ) ? 0xffffffff : 0;
	
#if defined INLINE_DEBUG_PCGTW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}


void Execute::PCEQB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PCEQB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sb0 = ( r->GPR [ i.Rs ].sb0 == r->GPR [ i.Rt ].sb0 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb1 = ( r->GPR [ i.Rs ].sb1 == r->GPR [ i.Rt ].sb1 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb2 = ( r->GPR [ i.Rs ].sb2 == r->GPR [ i.Rt ].sb2 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb3 = ( r->GPR [ i.Rs ].sb3 == r->GPR [ i.Rt ].sb3 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb4 = ( r->GPR [ i.Rs ].sb4 == r->GPR [ i.Rt ].sb4 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb5 = ( r->GPR [ i.Rs ].sb5 == r->GPR [ i.Rt ].sb5 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb6 = ( r->GPR [ i.Rs ].sb6 == r->GPR [ i.Rt ].sb6 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb7 = ( r->GPR [ i.Rs ].sb7 == r->GPR [ i.Rt ].sb7 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb8 = ( r->GPR [ i.Rs ].sb8 == r->GPR [ i.Rt ].sb8 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb9 = ( r->GPR [ i.Rs ].sb9 == r->GPR [ i.Rt ].sb9 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb10 = ( r->GPR [ i.Rs ].sb10 == r->GPR [ i.Rt ].sb10 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb11 = ( r->GPR [ i.Rs ].sb11 == r->GPR [ i.Rt ].sb11 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb12 = ( r->GPR [ i.Rs ].sb12 == r->GPR [ i.Rt ].sb12 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb13 = ( r->GPR [ i.Rs ].sb13 == r->GPR [ i.Rt ].sb13 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb14 = ( r->GPR [ i.Rs ].sb14 == r->GPR [ i.Rt ].sb14 ) ? 0xff : 0;
	r->GPR [ i.Rd ].sb15 = ( r->GPR [ i.Rs ].sb15 == r->GPR [ i.Rt ].sb15 ) ? 0xff : 0;
	
#if defined INLINE_DEBUG_PCEQB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PCEQH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PCEQH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sh0 = ( r->GPR [ i.Rs ].sh0 == r->GPR [ i.Rt ].sh0 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh1 = ( r->GPR [ i.Rs ].sh1 == r->GPR [ i.Rt ].sh1 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh2 = ( r->GPR [ i.Rs ].sh2 == r->GPR [ i.Rt ].sh2 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh3 = ( r->GPR [ i.Rs ].sh3 == r->GPR [ i.Rt ].sh3 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh4 = ( r->GPR [ i.Rs ].sh4 == r->GPR [ i.Rt ].sh4 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh5 = ( r->GPR [ i.Rs ].sh5 == r->GPR [ i.Rt ].sh5 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh6 = ( r->GPR [ i.Rs ].sh6 == r->GPR [ i.Rt ].sh6 ) ? 0xffff : 0;
	r->GPR [ i.Rd ].sh7 = ( r->GPR [ i.Rs ].sh7 == r->GPR [ i.Rt ].sh7 ) ? 0xffff : 0;
	
#if defined INLINE_DEBUG_PCEQH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}


void Execute::PCEQW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PCEQW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].sw0 = ( r->GPR [ i.Rs ].sw0 == r->GPR [ i.Rt ].sw0 ) ? 0xffffffff : 0;
	r->GPR [ i.Rd ].sw1 = ( r->GPR [ i.Rs ].sw1 == r->GPR [ i.Rt ].sw1 ) ? 0xffffffff : 0;
	r->GPR [ i.Rd ].sw2 = ( r->GPR [ i.Rs ].sw2 == r->GPR [ i.Rt ].sw2 ) ? 0xffffffff : 0;
	r->GPR [ i.Rd ].sw3 = ( r->GPR [ i.Rs ].sw3 == r->GPR [ i.Rt ].sw3 ) ? 0xffffffff : 0;
	
#if defined INLINE_DEBUG_PCEQW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}




void Execute::PEXTLB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PEXTLB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// note: must do these in reverse order or else data can get overwritten
	r->GPR [ i.Rd ].uh7 = ( ( ( (u16) r->GPR [ i.Rs ].ub7 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub7 ) );
	r->GPR [ i.Rd ].uh6 = ( ( ( (u16) r->GPR [ i.Rs ].ub6 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub6 ) );
	r->GPR [ i.Rd ].uh5 = ( ( ( (u16) r->GPR [ i.Rs ].ub5 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub5 ) );
	r->GPR [ i.Rd ].uh4 = ( ( ( (u16) r->GPR [ i.Rs ].ub4 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub4 ) );
	r->GPR [ i.Rd ].uh3 = ( ( ( (u16) r->GPR [ i.Rs ].ub3 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub3 ) );
	r->GPR [ i.Rd ].uh2 = ( ( ( (u16) r->GPR [ i.Rs ].ub2 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub2 ) );
	r->GPR [ i.Rd ].uh1 = ( ( ( (u16) r->GPR [ i.Rs ].ub1 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub1 ) );
	r->GPR [ i.Rd ].uh0 = ( ( ( (u16) r->GPR [ i.Rs ].ub0 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub0 ) );
	
#if defined INLINE_DEBUG_PEXTLB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PEXTLH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PEXTLH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// note: must do these in reverse order or else data can get overwritten
	r->GPR [ i.Rd ].uw3 = ( ( ( (u32) r->GPR [ i.Rs ].uh3 ) << 16 ) | ( (u32) r->GPR [ i.Rt ].uh3 ) );
	r->GPR [ i.Rd ].uw2 = ( ( ( (u32) r->GPR [ i.Rs ].uh2 ) << 16 ) | ( (u32) r->GPR [ i.Rt ].uh2 ) );
	r->GPR [ i.Rd ].uw1 = ( ( ( (u32) r->GPR [ i.Rs ].uh1 ) << 16 ) | ( (u32) r->GPR [ i.Rt ].uh1 ) );
	r->GPR [ i.Rd ].uw0 = ( ( ( (u32) r->GPR [ i.Rs ].uh0 ) << 16 ) | ( (u32) r->GPR [ i.Rt ].uh0 ) );
	
#if defined INLINE_DEBUG_PEXTLH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PEXTLW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PEXTLW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// PEXTLW rd, rs, rt
	// note: must do uq1 first or else data can get overwritten
	r->GPR [ i.Rd ].uq1 = ( ( ( (u64) r->GPR [ i.Rs ].uw1 ) << 32 ) | ( (u64) r->GPR [ i.Rt ].uw1 ) );
	r->GPR [ i.Rd ].uq0 = ( ( ( (u64) r->GPR [ i.Rs ].uw0 ) << 32 ) | ( (u64) r->GPR [ i.Rt ].uw0 ) );
	
#if defined INLINE_DEBUG_PEXTLW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PEXTUB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PEXTUB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].uh0 = ( ( ( (u16) r->GPR [ i.Rs ].ub8 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub8 ) );
	r->GPR [ i.Rd ].uh1 = ( ( ( (u16) r->GPR [ i.Rs ].ub9 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub9 ) );
	r->GPR [ i.Rd ].uh2 = ( ( ( (u16) r->GPR [ i.Rs ].ub10 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub10 ) );
	r->GPR [ i.Rd ].uh3 = ( ( ( (u16) r->GPR [ i.Rs ].ub11 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub11 ) );
	r->GPR [ i.Rd ].uh4 = ( ( ( (u16) r->GPR [ i.Rs ].ub12 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub12 ) );
	r->GPR [ i.Rd ].uh5 = ( ( ( (u16) r->GPR [ i.Rs ].ub13 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub13 ) );
	r->GPR [ i.Rd ].uh6 = ( ( ( (u16) r->GPR [ i.Rs ].ub14 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub14 ) );
	r->GPR [ i.Rd ].uh7 = ( ( ( (u16) r->GPR [ i.Rs ].ub15 ) << 8 ) | ( (u16) r->GPR [ i.Rt ].ub15 ) );
	
#if defined INLINE_DEBUG_PEXTUB || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PEXTUH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PEXTUH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].uw0 = ( ( ( (u32) r->GPR [ i.Rs ].uh4 ) << 16 ) | ( (u32) r->GPR [ i.Rt ].uh4 ) );
	r->GPR [ i.Rd ].uw1 = ( ( ( (u32) r->GPR [ i.Rs ].uh5 ) << 16 ) | ( (u32) r->GPR [ i.Rt ].uh5 ) );
	r->GPR [ i.Rd ].uw2 = ( ( ( (u32) r->GPR [ i.Rs ].uh6 ) << 16 ) | ( (u32) r->GPR [ i.Rt ].uh6 ) );
	r->GPR [ i.Rd ].uw3 = ( ( ( (u32) r->GPR [ i.Rs ].uh7 ) << 16 ) | ( (u32) r->GPR [ i.Rt ].uh7 ) );
	
#if defined INLINE_DEBUG_PEXTUH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PEXTUW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PEXTUW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	r->GPR [ i.Rd ].uq0 = ( ( ( (u64) r->GPR [ i.Rs ].uw2 ) << 32 ) | ( (u64) r->GPR [ i.Rt ].uw2 ) );
	r->GPR [ i.Rd ].uq1 = ( ( ( (u64) r->GPR [ i.Rs ].uw3 ) << 32 ) | ( (u64) r->GPR [ i.Rt ].uw3 ) );
	
#if defined INLINE_DEBUG_PEXTUW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}








void Execute::PMFLO ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMFLO || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " LO=" << r->LO.uq0 << " " << r->LO.uq1;
#endif

	r->GPR [ i.Rd ].uq0 = r->LO.uq0;
	r->GPR [ i.Rd ].uq1 = r->LO.uq1;
	
#if defined INLINE_DEBUG_PMFLO || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PMFHI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMFHI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " HI=" << r->HI.uq0 << " " << r->HI.uq1;
#endif

	r->GPR [ i.Rd ].uq0 = r->HI.uq0;
	r->GPR [ i.Rd ].uq1 = r->HI.uq1;
	
#if defined INLINE_DEBUG_PMFHI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}


void Execute::PINTH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PINTH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	u16 temp;

	// note: must use this order/method
	r->GPR [ i.Rd ].uh0 = r->GPR [ i.Rt ].uh0;
	r->GPR [ i.Rd ].uh7 = r->GPR [ i.Rs ].uh7;
	
	temp = r->GPR [ i.Rs ].uh4;
	r->GPR [ i.Rd ].uh4 = r->GPR [ i.Rt ].uh2;
	r->GPR [ i.Rd ].uh2 = r->GPR [ i.Rt ].uh1;
	r->GPR [ i.Rd ].uh1 = temp;
	
	temp = r->GPR [ i.Rs ].uh5;
	r->GPR [ i.Rd ].uh5 = r->GPR [ i.Rs ].uh6;
	r->GPR [ i.Rd ].uh6 = r->GPR [ i.Rt ].uh3;
	r->GPR [ i.Rd ].uh3 = temp;
	
#if defined INLINE_DEBUG_PINTH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PINTEH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PINTEH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// note: these must be done in reverse order or else
	r->GPR [ i.Rd ].uh7 = r->GPR [ i.Rs ].uh6;
	r->GPR [ i.Rd ].uh6 = r->GPR [ i.Rt ].uh6;
	r->GPR [ i.Rd ].uh5 = r->GPR [ i.Rs ].uh4;
	r->GPR [ i.Rd ].uh4 = r->GPR [ i.Rt ].uh4;
	r->GPR [ i.Rd ].uh3 = r->GPR [ i.Rs ].uh2;
	r->GPR [ i.Rd ].uh2 = r->GPR [ i.Rt ].uh2;
	r->GPR [ i.Rd ].uh1 = r->GPR [ i.Rs ].uh0;
	r->GPR [ i.Rd ].uh0 = r->GPR [ i.Rt ].uh0;
	
#if defined INLINE_DEBUG_PINTEH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}



// multimedia multiply //


void Execute::PMADDH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMADDH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PMADDH";

	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	r->LO.sw0 += ( (s32) r->GPR [ i.Rs ].sh0 ) * ( (s32) r->GPR [ i.Rt ].sh0 );
	r->LO.sw1 += ( (s32) r->GPR [ i.Rs ].sh1 ) * ( (s32) r->GPR [ i.Rt ].sh1 );
	r->HI.sw0 += ( (s32) r->GPR [ i.Rs ].sh2 ) * ( (s32) r->GPR [ i.Rt ].sh2 );
	r->HI.sw1 += ( (s32) r->GPR [ i.Rs ].sh3 ) * ( (s32) r->GPR [ i.Rt ].sh3 );
	r->LO.sw2 += ( (s32) r->GPR [ i.Rs ].sh4 ) * ( (s32) r->GPR [ i.Rt ].sh4 );
	r->LO.sw3 += ( (s32) r->GPR [ i.Rs ].sh5 ) * ( (s32) r->GPR [ i.Rt ].sh5 );
	r->HI.sw2 += ( (s32) r->GPR [ i.Rs ].sh6 ) * ( (s32) r->GPR [ i.Rt ].sh6 );
	r->HI.sw3 += ( (s32) r->GPR [ i.Rs ].sh7 ) * ( (s32) r->GPR [ i.Rt ].sh7 );
	
	r->GPR [ i.Rd ].sw0 = r->LO.sw0;
	r->GPR [ i.Rd ].sw1 = r->HI.sw0;
	r->GPR [ i.Rd ].sw2 = r->LO.sw2;
	r->GPR [ i.Rd ].sw3 = r->HI.sw2;
}

void Execute::PMADDW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMADDW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PMADDW";
	
	s64 sTemp64;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	sTemp64 = ( (u64) r->LO.uw0 );
	sTemp64 |= ( r->HI.uq0 << 32 );
	sTemp64 += ( (s64) r->GPR [ i.Rs ].sw0 ) * ( (s64) r->GPR [ i.Rt ].sw0 );
	r->LO.sq0 = (s32) sTemp64;
	r->HI.sq0 = ( sTemp64 >> 32 );
	r->GPR [ i.Rd ].sq0 = sTemp64;
	
	sTemp64 = ( (u64) r->LO.uw2 );
	sTemp64 |= ( r->HI.uq1 << 32 );
	sTemp64 += ( (s64) r->GPR [ i.Rs ].sw2 ) * ( (s64) r->GPR [ i.Rt ].sw2 );
	r->LO.sq1 = (s32) sTemp64;
	r->HI.sq1 = ( sTemp64 >> 32 );
	r->GPR [ i.Rd ].sq1 = sTemp64;
}

void Execute::PMADDUW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMADDUW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PMADDUW";
	
	u64 uTemp64;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	uTemp64 = ( (u64) r->LO.uw0 );
	uTemp64 |= ( r->HI.uq0 << 32 );
	uTemp64 += ( (u64) r->GPR [ i.Rs ].uw0 ) * ( (u64) r->GPR [ i.Rt ].uw0 );
	r->LO.sq0 = (s32) uTemp64;
	r->HI.sq0 = (s32) ( uTemp64 >> 32 );
	r->GPR [ i.Rd ].uq0 = uTemp64;
	
	uTemp64 = ( (u64) r->LO.uw2 );
	uTemp64 |= ( r->HI.uq1 << 32 );
	uTemp64 += ( (u64) r->GPR [ i.Rs ].uw2 ) * ( (u64) r->GPR [ i.Rt ].uw2 );
	r->LO.sq1 = (s32) uTemp64;
	r->HI.sq1 = (s32) ( uTemp64 >> 32 );
	r->GPR [ i.Rd ].uq1 = uTemp64;
}


void Execute::PMSUBH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMSUBH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PMSUBH";
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	r->LO.sw0 -= ( (s32) r->GPR [ i.Rs ].sh0 ) * ( (s32) r->GPR [ i.Rt ].sh0 );
	r->LO.sw1 -= ( (s32) r->GPR [ i.Rs ].sh1 ) * ( (s32) r->GPR [ i.Rt ].sh1 );
	r->HI.sw0 -= ( (s32) r->GPR [ i.Rs ].sh2 ) * ( (s32) r->GPR [ i.Rt ].sh2 );
	r->HI.sw1 -= ( (s32) r->GPR [ i.Rs ].sh3 ) * ( (s32) r->GPR [ i.Rt ].sh3 );
	r->LO.sw2 -= ( (s32) r->GPR [ i.Rs ].sh4 ) * ( (s32) r->GPR [ i.Rt ].sh4 );
	r->LO.sw3 -= ( (s32) r->GPR [ i.Rs ].sh5 ) * ( (s32) r->GPR [ i.Rt ].sh5 );
	r->HI.sw2 -= ( (s32) r->GPR [ i.Rs ].sh6 ) * ( (s32) r->GPR [ i.Rt ].sh6 );
	r->HI.sw3 -= ( (s32) r->GPR [ i.Rs ].sh7 ) * ( (s32) r->GPR [ i.Rt ].sh7 );
	
	r->GPR [ i.Rd ].sw0 = r->LO.sw0;
	r->GPR [ i.Rd ].sw1 = r->HI.sw0;
	r->GPR [ i.Rd ].sw2 = r->LO.sw2;
	r->GPR [ i.Rd ].sw3 = r->HI.sw2;
}



void Execute::PMSUBW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMSUBW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PMSUBW";
	
	s64 sTemp64;
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	sTemp64 = ( (u64) r->LO.uw0 );
	sTemp64 |= ( r->HI.uq0 << 32 );
	sTemp64 -= ( (s64) r->GPR [ i.Rs ].sw0 ) * ( (s64) r->GPR [ i.Rt ].sw0 );
	r->LO.sq0 = (s32) sTemp64;
	r->HI.sq0 = ( sTemp64 >> 32 );
	r->GPR [ i.Rd ].sq0 = sTemp64;
	
	sTemp64 = ( (u64) r->LO.uw2 );
	sTemp64 |= ( r->HI.uq1 << 32 );
	sTemp64 -= ( (s64) r->GPR [ i.Rs ].sw2 ) * ( (s64) r->GPR [ i.Rt ].sw2 );
	r->LO.sq1 = (s32) sTemp64;
	r->HI.sq1 = ( sTemp64 >> 32 );
	r->GPR [ i.Rd ].sq1 = sTemp64;
}

void Execute::PMULTH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMULTH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	// rd = 0, 2, 4, 6
	// lo = 0, 1, 4, 5
	// hi = 2, 3, 6, 7
	
	r->GPR [ i.Rd ].sw0 = ( (s32) r->GPR [ i.Rs ].sh0 ) * ( (s32) r->GPR [ i.Rt ].sh0 );
	r->GPR [ i.Rd ].sw1 = ( (s32) r->GPR [ i.Rs ].sh2 ) * ( (s32) r->GPR [ i.Rt ].sh2 );
	r->GPR [ i.Rd ].sw2 = ( (s32) r->GPR [ i.Rs ].sh4 ) * ( (s32) r->GPR [ i.Rt ].sh4 );
	r->GPR [ i.Rd ].sw3 = ( (s32) r->GPR [ i.Rs ].sh6 ) * ( (s32) r->GPR [ i.Rt ].sh6 );
	
	r->LO.sw0 = r->GPR [ i.Rd ].sw0;
	r->LO.sw1 = ( (s32) r->GPR [ i.Rs ].sh1 ) * ( (s32) r->GPR [ i.Rt ].sh1 );
	r->LO.sw2 = r->GPR [ i.Rd ].sw2;
	r->LO.sw3 = ( (s32) r->GPR [ i.Rs ].sh5 ) * ( (s32) r->GPR [ i.Rt ].sh5 );
	
	r->HI.sw0 = r->GPR [ i.Rd ].sw1;
	r->HI.sw1 = ( (s32) r->GPR [ i.Rs ].sh3 ) * ( (s32) r->GPR [ i.Rt ].sh3 );
	r->HI.sw2 = r->GPR [ i.Rd ].sw3;
	r->HI.sw3 = ( (s32) r->GPR [ i.Rs ].sh7 ) * ( (s32) r->GPR [ i.Rt ].sh7 );
	
#if defined INLINE_DEBUG_PMULTH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PMULTW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMULTW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	r->GPR [ i.Rd ].sq0 = ( (s64) r->GPR [ i.Rs ].sw0 ) * ( (s64) r->GPR [ i.Rt ].sw0 );
	r->GPR [ i.Rd ].sq1 = ( (s64) r->GPR [ i.Rs ].sw2 ) * ( (s64) r->GPR [ i.Rt ].sw2 );
	
	r->LO.sq0 = (s32) ( r->GPR [ i.Rd ].sq0 );
	r->LO.sq1 = (s32) ( r->GPR [ i.Rd ].sq1 );
	r->HI.sq0 = ( r->GPR [ i.Rd ].sq0 >> 32 );
	r->HI.sq1 = ( r->GPR [ i.Rd ].sq1 >> 32 );
	
#if defined INLINE_DEBUG_PMULTW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PMULTUW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMULTUW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	r->GPR [ i.Rd ].uq0 = ( (u64) r->GPR [ i.Rs ].uw0 ) * ( (u64) r->GPR [ i.Rt ].uw0 );
	r->GPR [ i.Rd ].uq1 = ( (u64) r->GPR [ i.Rs ].uw2 ) * ( (u64) r->GPR [ i.Rt ].uw2 );
	
	r->LO.sq0 = (s32) ( r->GPR [ i.Rd ].sq0 );
	r->LO.sq1 = (s32) ( r->GPR [ i.Rd ].sq1 );
	r->HI.sq0 = ( r->GPR [ i.Rd ].sq0 >> 32 );
	r->HI.sq1 = ( r->GPR [ i.Rd ].sq1 >> 32 );
	
#if defined INLINE_DEBUG_PMULTUW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}






void Execute::PHMADH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PHMADH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PHMADH";
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	r->LO.sw0 = ( ( (s32) r->GPR [ i.Rs ].sh1 ) * ( (s32) r->GPR [ i.Rt ].sh1 ) ) + ( ( (s32) r->GPR [ i.Rs ].sh0 ) * ( (s32) r->GPR [ i.Rt ].sh0 ) );
	r->HI.sw0 = ( ( (s32) r->GPR [ i.Rs ].sh3 ) * ( (s32) r->GPR [ i.Rt ].sh3 ) ) + ( ( (s32) r->GPR [ i.Rs ].sh2 ) * ( (s32) r->GPR [ i.Rt ].sh2 ) );
	r->LO.sw2 = ( ( (s32) r->GPR [ i.Rs ].sh5 ) * ( (s32) r->GPR [ i.Rt ].sh5 ) ) + ( ( (s32) r->GPR [ i.Rs ].sh4 ) * ( (s32) r->GPR [ i.Rt ].sh4 ) );
	r->HI.sw2 = ( ( (s32) r->GPR [ i.Rs ].sh7 ) * ( (s32) r->GPR [ i.Rt ].sh7 ) ) + ( ( (s32) r->GPR [ i.Rs ].sh6 ) * ( (s32) r->GPR [ i.Rt ].sh6 ) );
	
	r->GPR [ i.Rd ].sw0 = r->LO.sw0;
	r->GPR [ i.Rd ].sw1 = r->HI.sw0;
	r->GPR [ i.Rd ].sw2 = r->LO.sw2;
	r->GPR [ i.Rd ].sw3 = r->HI.sw2;
}



void Execute::PHMSBH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PHMSBH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: PHMSBH";
	
	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	r->LO.sw0 = ( ( (s32) r->GPR [ i.Rs ].sh1 ) * ( (s32) r->GPR [ i.Rt ].sh1 ) ) - ( ( (s32) r->GPR [ i.Rs ].sh0 ) * ( (s32) r->GPR [ i.Rt ].sh0 ) );
	r->HI.sw0 = ( ( (s32) r->GPR [ i.Rs ].sh3 ) * ( (s32) r->GPR [ i.Rt ].sh3 ) ) - ( ( (s32) r->GPR [ i.Rs ].sh2 ) * ( (s32) r->GPR [ i.Rt ].sh2 ) );
	r->LO.sw2 = ( ( (s32) r->GPR [ i.Rs ].sh5 ) * ( (s32) r->GPR [ i.Rt ].sh5 ) ) - ( ( (s32) r->GPR [ i.Rs ].sh4 ) * ( (s32) r->GPR [ i.Rt ].sh4 ) );
	r->HI.sw2 = ( ( (s32) r->GPR [ i.Rs ].sh7 ) * ( (s32) r->GPR [ i.Rt ].sh7 ) ) - ( ( (s32) r->GPR [ i.Rs ].sh6 ) * ( (s32) r->GPR [ i.Rt ].sh6 ) );
	
	r->GPR [ i.Rd ].sw0 = r->LO.sw0;
	r->GPR [ i.Rd ].sw1 = r->HI.sw0;
	r->GPR [ i.Rd ].sw2 = r->LO.sw2;
	r->GPR [ i.Rd ].sw3 = r->HI.sw2;
}



// multimedia divide //

void Execute::PDIVW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PDIVW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// 37 cycles
	// divide by two until r5900 is running at proper speed
	static const int c_iDivideCycles = 37 / 2;

	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	// set until when mul/div unit will be in use for each pipeline
	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iDivideCycles;
	r->MulDiv_BusyUntil_Cycle1 = r->MulDiv_BusyUntil_Cycle;
	
	r->LO.sq0 = (s32) ( r->GPR [ i.Rs ].sw0 / r->GPR [ i.Rt ].sw0 );
	r->LO.sq1 = (s32) ( r->GPR [ i.Rs ].sw2 / r->GPR [ i.Rt ].sw2 );
	
	r->HI.sq0 = (s32) ( r->GPR [ i.Rs ].sw0 % r->GPR [ i.Rt ].sw0 );
	r->HI.sq1 = (s32) ( r->GPR [ i.Rs ].sw2 % r->GPR [ i.Rt ].sw2 );
	
#if defined INLINE_DEBUG_PDIVW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " LO=" << r->LO.uq0 << " " << r->LO.uq1 << " HI=" << r->HI.uq0 << " " << r->HI.uq1;
#endif
}

void Execute::PDIVUW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PDIVUW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// 37 cycles
	// divide by two until r5900 is running at proper speed
	static const int c_iDivideCycles = 37 / 2;

	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	// set until when mul/div unit will be in use for each pipeline
	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iDivideCycles;
	r->MulDiv_BusyUntil_Cycle1 = r->MulDiv_BusyUntil_Cycle;
	
	r->LO.sq0 = (s32) ( r->GPR [ i.Rs ].uw0 / r->GPR [ i.Rt ].uw0 );
	r->LO.sq1 = (s32) ( r->GPR [ i.Rs ].uw2 / r->GPR [ i.Rt ].uw2 );
	
	r->HI.sq0 = (s32) ( r->GPR [ i.Rs ].uw0 % r->GPR [ i.Rt ].uw0 );
	r->HI.sq1 = (s32) ( r->GPR [ i.Rs ].uw2 % r->GPR [ i.Rt ].uw2 );
	
#if defined INLINE_DEBUG_PDIVUW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " LO=" << r->LO.uq0 << " " << r->LO.uq1 << " HI=" << r->HI.uq0 << " " << r->HI.uq1;
#endif
}

void Execute::PDIVBW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PDIVBW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// 37 cycles
	// divide by two until r5900 is running at proper speed
	static const int c_iDivideCycles = 37 / 2;

	// check if mul/div unit is in use
	if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	}
	if ( r->MulDiv_BusyUntil_Cycle1 > r->CycleCount )
	{
		r->CycleCount = r->MulDiv_BusyUntil_Cycle1;
	}
	
	// set until when mul/div unit will be in use for each pipeline
	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iDivideCycles;
	r->MulDiv_BusyUntil_Cycle1 = r->MulDiv_BusyUntil_Cycle;
	
	r->LO.sw0 = ( r->GPR [ i.Rs ].sw0 / ( (s32) r->GPR [ i.Rt ].sh0 ) );
	r->LO.sw1 = ( r->GPR [ i.Rs ].sw1 / ( (s32) r->GPR [ i.Rt ].sh0 ) );
	r->LO.sw2 = ( r->GPR [ i.Rs ].sw2 / ( (s32) r->GPR [ i.Rt ].sh0 ) );
	r->LO.sw3 = ( r->GPR [ i.Rs ].sw3 / ( (s32) r->GPR [ i.Rt ].sh0 ) );
	
	r->HI.sw0 = (s16) ( r->GPR [ i.Rs ].sw0 % ( (s32) r->GPR [ i.Rt ].sh0 ) );
	r->HI.sw1 = (s16) ( r->GPR [ i.Rs ].sw1 % ( (s32) r->GPR [ i.Rt ].sh0 ) );
	r->HI.sw2 = (s16) ( r->GPR [ i.Rs ].sw2 % ( (s32) r->GPR [ i.Rt ].sh0 ) );
	r->HI.sw3 = (s16) ( r->GPR [ i.Rs ].sw3 % ( (s32) r->GPR [ i.Rt ].sh0 ) );
	
#if defined INLINE_DEBUG_PDIVBW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " LO=" << r->LO.uq0 << " " << r->LO.uq1 << " HI=" << r->HI.uq0 << " " << r->HI.uq1;
#endif
}




void Execute::PREVH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PREVH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif	

	u16 temp;

	// note: must use this order/method
	temp = r->GPR [ i.Rt ].uh3;
	r->GPR [ i.Rd ].uh3 = r->GPR [ i.Rt ].uh0;
	r->GPR [ i.Rd ].uh0 = temp;
	
	temp = r->GPR [ i.Rt ].uh2;
	r->GPR [ i.Rd ].uh2 = r->GPR [ i.Rt ].uh1;
	r->GPR [ i.Rd ].uh1 = temp;
	
	temp = r->GPR [ i.Rt ].uh7;
	r->GPR [ i.Rd ].uh7 = r->GPR [ i.Rt ].uh4;
	r->GPR [ i.Rd ].uh4 = temp;
	
	temp = r->GPR [ i.Rt ].uh6;
	r->GPR [ i.Rd ].uh6 = r->GPR [ i.Rt ].uh5;
	r->GPR [ i.Rd ].uh5 = temp;
	
#if defined INLINE_DEBUG_PREVH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}




void Execute::PEXEH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PEXEH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	u16 temp;

	// note: must use this order/method
	r->GPR [ i.Rd ].uh1 = r->GPR [ i.Rt ].uh1;
	r->GPR [ i.Rd ].uh3 = r->GPR [ i.Rt ].uh3;
	r->GPR [ i.Rd ].uh5 = r->GPR [ i.Rt ].uh5;
	r->GPR [ i.Rd ].uh7 = r->GPR [ i.Rt ].uh7;
	
	temp = r->GPR [ i.Rt ].uh2;
	r->GPR [ i.Rd ].uh2 = r->GPR [ i.Rt ].uh0;
	r->GPR [ i.Rd ].uh0 = temp;
	
	temp = r->GPR [ i.Rt ].uh6;
	r->GPR [ i.Rd ].uh6 = r->GPR [ i.Rt ].uh4;
	r->GPR [ i.Rd ].uh4 = temp;
	
#if defined INLINE_DEBUG_PEXEH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PEXEW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PEXEW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	u32 temp;

	// note: must use this order/method
	r->GPR [ i.Rd ].uw1 = r->GPR [ i.Rt ].uw1;
	r->GPR [ i.Rd ].uw3 = r->GPR [ i.Rt ].uw3;
	
	temp = r->GPR [ i.Rt ].uw2;
	r->GPR [ i.Rd ].uw2 = r->GPR [ i.Rt ].uw0;
	r->GPR [ i.Rd ].uw0 = temp;
	
#if defined INLINE_DEBUG_PEXEW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PROT3W ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PROT3W || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	u32 temp;

	// note: must use this order/method
	r->GPR [ i.Rd ].uw3 = r->GPR [ i.Rt ].uw3;
	
	temp = r->GPR [ i.Rt ].uw2;
	r->GPR [ i.Rd ].uw2 = r->GPR [ i.Rt ].uw0;
	r->GPR [ i.Rd ].uw0 = r->GPR [ i.Rt ].uw1;
	r->GPR [ i.Rd ].uw1 = temp;
	
#if defined INLINE_DEBUG_PROT3W || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}



void Execute::PMTHI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMTHI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif

	// note: PTMHI instruction uses Rs, not Rd
	r->HI.uq0 = r->GPR [ i.Rs ].uq0;
	r->HI.uq1 = r->GPR [ i.Rs ].uq1;
	
#if defined INLINE_DEBUG_PMTHI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " HI=" << r->HI.uq0 << " " << r->HI.uq1;
#endif
}

void Execute::PMTLO ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PMTLO || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif

	// note: PTMLO instruction uses Rs, not Rd
	r->LO.uq0 = r->GPR [ i.Rs ].uq0;
	r->LO.uq1 = r->GPR [ i.Rs ].uq1;
	
#if defined INLINE_DEBUG_PMTLO || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " LO=" << r->LO.uq0 << " " << r->LO.uq1;
#endif
}



void Execute::PCPYLD ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PCPYLD || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// PCPYLD rd, rs, rt
	// note: must use this order
	r->GPR [ i.Rd ].uq1 = r->GPR [ i.Rs ].uq0;
	r->GPR [ i.Rd ].uq0 = r->GPR [ i.Rt ].uq0;
	
#if defined INLINE_DEBUG_PCPYLD || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PCPYUD ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PCPYUD || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// PCPYUD rd, rs, rt
	// note: must use this order
	r->GPR [ i.Rd ].uq0 = r->GPR [ i.Rs ].uq1;
	r->GPR [ i.Rd ].uq1 = r->GPR [ i.Rt ].uq1;
	
#if defined INLINE_DEBUG_PCPYUD || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PCPYH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PCPYH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	// note: must use this order
	r->GPR [ i.Rd ].uh7 = r->GPR [ i.Rt ].uh4;
	r->GPR [ i.Rd ].uh6 = r->GPR [ i.Rt ].uh4;
	r->GPR [ i.Rd ].uh5 = r->GPR [ i.Rt ].uh4;
	r->GPR [ i.Rd ].uh4 = r->GPR [ i.Rt ].uh4;
	
	r->GPR [ i.Rd ].uh3 = r->GPR [ i.Rt ].uh0;
	r->GPR [ i.Rd ].uh2 = r->GPR [ i.Rt ].uh0;
	r->GPR [ i.Rd ].uh1 = r->GPR [ i.Rt ].uh0;
	r->GPR [ i.Rd ].uh0 = r->GPR [ i.Rt ].uh0;
	
#if defined INLINE_DEBUG_PCPYH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}


void Execute::PEXCH ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PEXCH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	u16 temp;

	// note: must use this order/method
	r->GPR [ i.Rd ].uh0 = r->GPR [ i.Rt ].uh0;
	r->GPR [ i.Rd ].uh3 = r->GPR [ i.Rt ].uh3;
	
	temp = r->GPR [ i.Rt ].uh2;
	r->GPR [ i.Rd ].uh2 = r->GPR [ i.Rt ].uh1;
	r->GPR [ i.Rd ].uh1 = temp;
	
	r->GPR [ i.Rd ].uh4 = r->GPR [ i.Rt ].uh4;
	r->GPR [ i.Rd ].uh7 = r->GPR [ i.Rt ].uh7;
	
	temp = r->GPR [ i.Rt ].uh6;
	r->GPR [ i.Rd ].uh6 = r->GPR [ i.Rt ].uh5;
	r->GPR [ i.Rd ].uh5 = temp;
	
#if defined INLINE_DEBUG_PEXCH || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::PEXCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PEXCW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
#endif

	u32 temp;

	// note: must use this order/method
	r->GPR [ i.Rd ].uw0 = r->GPR [ i.Rt ].uw0;
	r->GPR [ i.Rd ].uw3 = r->GPR [ i.Rt ].uw3;
	
	temp = r->GPR [ i.Rt ].uw2;
	r->GPR [ i.Rd ].uw2 = r->GPR [ i.Rt ].uw1;
	r->GPR [ i.Rd ].uw1 = temp;
	
#if defined INLINE_DEBUG_PEXCW || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}

void Execute::QFSRV ( Instruction::Format i )
{
#if defined INLINE_DEBUG_QFSRV || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Rs=" << r->GPR [ i.Rs ].uq0 << " " << r->GPR [ i.Rs ].uq1;
	debug << hex << " Rt=" << r->GPR [ i.Rt ].uq0 << " " << r->GPR [ i.Rt ].uq1;
	debug << hex << " SA=" << r->SA;
#endif

	unsigned long ulRdIndex, ulSrcIndex, ulShiftAmount, ulShiftLeft, ulShiftRight;
	unsigned long long Rs0, Rt0, Rs1, Rt1;

	//cout << "\nhps2x64: ERROR: R5900: Instruction not implemented: QFSRV";
	
	// QFSRV rd, rs, rt
	
	// get the shift amount
	ulShiftAmount = r->SA & 0xf;
	
	// need the number of bits to shift, so number of bytes to shift times 8
	ulShiftAmount <<= 3;
	ulShiftRight = ulShiftAmount & 63;
	ulShiftLeft = 64 - ulShiftRight;
	
	// intialize where to copy into RD
	//ulRdIndex = 0;
	
	// these could potentially get overwritten
	Rs0 = r->GPR [ i.Rs ].uq0;
	Rs1 = r->GPR [ i.Rs ].uq1;
	Rt0 = r->GPR [ i.Rt ].uq0;
	Rt1 = r->GPR [ i.Rt ].uq1;
	
	if ( ulShiftAmount < 64 )
	{
		//r->GPR [ i.Rd ].uq0 = r->GPR [ i.Rt ].uq0 >> ulShiftRight;
		//r->GPR [ i.Rd ].uq1 = r->GPR [ i.Rt ].uq1 >> ulShiftRight;
		r->GPR [ i.Rd ].uq0 = Rt0 >> ulShiftRight;
		r->GPR [ i.Rd ].uq1 = Rt1 >> ulShiftRight;
		
		// if ulShiftLeft is 64, well.. you can't shift by 64 because it is same as shift by zero?
		if ( ulShiftRight )
		{
			//r->GPR [ i.Rd ].uq0 |= r->GPR [ i.Rt ].uq1 << ulShiftLeft;
			//r->GPR [ i.Rd ].uq1 |= r->GPR [ i.Rs ].uq0 << ulShiftLeft;
			r->GPR [ i.Rd ].uq0 |= Rt1 << ulShiftLeft;
			r->GPR [ i.Rd ].uq1 |= Rs0 << ulShiftLeft;
		}
	}
	else
	{
		//r->GPR [ i.Rd ].uq0 = r->GPR [ i.Rt ].uq1 >> ulShiftRight;
		//r->GPR [ i.Rd ].uq1 = r->GPR [ i.Rs ].uq0 >> ulShiftRight;
		r->GPR [ i.Rd ].uq0 = Rt1 >> ulShiftRight;
		r->GPR [ i.Rd ].uq1 = Rs0 >> ulShiftRight;
		
		// if ulShiftLeft is 64, well.. you can't shift by 64 because it is same as shift by zero?
		if ( ulShiftRight )
		{
			//r->GPR [ i.Rd ].uq0 |= r->GPR [ i.Rs ].uq0 << ulShiftLeft;
			//r->GPR [ i.Rd ].uq1 |= r->GPR [ i.Rs ].uq1 << ulShiftLeft;
			r->GPR [ i.Rd ].uq0 |= Rs0 << ulShiftLeft;
			r->GPR [ i.Rd ].uq1 |= Rs1 << ulShiftLeft;
		}
	}
	
	/*
	// copy RT into RD starting at index specified in SA
	for ( ulSrcIndex = ulShiftAmount; ulSrcIndex < 16; ulSrcIndex++ )
	{
		r->GPR [ i.Rd ].vub [ ulRdIndex++ ] = r->GPR [ i.Rt ].vub [ ulSrcIndex ];
	}
	
	// copy RS into RD taking over where RT left off
	for ( ulSrcIndex = 0; ulSrcIndex < ulShiftAmount; ulSrcIndex++ )
	{
		r->GPR [ i.Rd ].vub [ ulRdIndex++ ] = r->GPR [ i.Rs ].vub [ ulSrcIndex ];
	}
	*/
	
#if defined INLINE_DEBUG_QFSRV || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_INTEGER_VECTOR // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " Rd=" << r->GPR [ i.Rd ].uq0 << " " << r->GPR [ i.Rd ].uq1;
#endif
}





// * R5900 COP0 instructions * //


void Execute::EI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_EI || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	if ( r->CPR0.Status.EDI || r->CPR0.Status.EXL || r->CPR0.Status.ERL || !r->CPR0.Status.KSU )
	{
		r->CPR0.Status.EIE = 1;

#ifdef UPDATE_INTERRUPTS_EI
		// interrupt status changed
		r->UpdateInterrupt ();
#endif
	}

#if defined INLINE_DEBUG_EI || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " ISTAT=" << *r->_Debug_IntcStat << " IMASK=" << *r->_Debug_IntcMask;
	debug << " EIPROCSTAT=" << (r->Status.Value & 1);
#endif
}

void Execute::DI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DI || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	if ( r->CPR0.Status.EDI || r->CPR0.Status.EXL || r->CPR0.Status.ERL || !r->CPR0.Status.KSU )
	{
		r->CPR0.Status.EIE = 0;

#ifdef UPDATE_INTERRUPTS_DI
		// interrupt status changed
		r->UpdateInterrupt ();
#endif
	}

}

void Execute::CFC0 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CFC0 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	r->GPR [ i.Rt ].sq0 = (s32) r->CPC0 [ i.Rd ];

}

void Execute::CTC0 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CTC0 || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	r->CPC0 [ i.Rd ] = r->GPR [ i.Rt ].sw0;
}




void Execute::SYNC ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SYNC || defined INLINE_DEBUG_R5900
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

		// interrupt status changed
		r->UpdateInterrupt ();
}


void Execute::CACHE ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " op=" << hex << i.Rt;
#endif

	u32 ulIndex, PFN;

	u32 VirtualAddress;
	u64 *pMemPtr64;
	u64 *pCacheData;
	
	VirtualAddress = r->GPR [ i.Base ].sw0 + i.sOffset;

	switch ( i.Rt )
	{
#ifdef ENABLE_R5900_ICACHE
		// IXIN - index invalidate - bit 0 has way to invalidate and bits 6-12 have index to invalidate
		case 0x7:
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " IXIN";
#endif
			// get index
			ulIndex = ( VirtualAddress >> 6 ) & 0x7f;
			ulIndex <<= 1;
			ulIndex |= ( VirtualAddress & 1 );
			
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " IDX=" << hex << ulIndex;
#endif

			// get the physical address
			PFN = r->ICache.Get_PFN ( ulIndex );
			
			// if not bios, then invalidate recompiler cache also
			/*
			if ( ( PFN & 0x1fc00000 ) != 0x1fc00000 )
			{
//#ifdef ENABLE_INVALID_ARRAY
				r->Bus->InvalidArray.b8 [ ( PFN & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
//#endif
			}
			*/
			
			r->ICache.InvalidateIndex ( VirtualAddress );
			break;
			
		// IHIN - hit invalidate i-cache (invalidate if hit)
		case 0xb:
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " IHIN";
#endif
			r->ICache.InvalidateHit ( VirtualAddress );
			
			// if not bios, then invalidate recompiler cache also
			/*
			if ( ( VirtualAddress & 0x1fc00000 ) != 0x1fc00000 )
			{
//#ifdef ENABLE_INVALID_ARRAY
				r->Bus->InvalidArray.b8 [ ( VirtualAddress & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
//#endif
			}
			*/
			break;
			
		// IFL - fill i-cache line (prefetch?)
		case 0xe:
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " IFL";
#endif

			// ***todo*** also needs to clear cache line in recompiler if the block was modified
			/*
			pMemPtr64 = (u64*) r->Bus->GetIMemPtr ( VirtualAddress & 0x1fffffc0 );
			
			if ( pMemPtr64 )
			{
				r->ICache.ReloadCache ( VirtualAddress, pMemPtr64 );
			}
			*/
			
			//r->CycleCount += r->Bus->GetLatency ();
			break;
#endif

#ifdef ENABLE_R5900_DCACHE
		// DXLTG - load data cache tag into taglo using index/way
		case 0x10:
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " DXLTG";
#endif
			// get index
			ulIndex = ( VirtualAddress >> 6 ) & 0x3f;
			ulIndex <<= 1;
			ulIndex |= ( VirtualAddress & 1 );
			
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " IDX=" << hex << ulIndex;
#endif

			// ***TODO*** implement LOCK bit
			
			PFN = 0;
			
			if ( r->DCache.Get_Valid ( ulIndex ) )
			{
				PFN = r->DCache.Get_PFN ( ulIndex ) & 0xfffff000;
				PFN |= ( 1 << 5 );
				
				if ( r->DCache.Get_Dirty ( ulIndex ) )
				{
					PFN |= ( 1 << 6 );
				}
				
				if ( r->DCache.Get_LRF ( ulIndex ) )
				{
					PFN |= ( 1 << 4 );
				}
				
				if ( r->DCache.Get_Lock ( ulIndex ) )
				{
					PFN |= ( 1 << 3 );
				}
			}
			
			r->CPR0.TagLo = PFN;
			
			break;


		// DHIN - invalidate data cache entry if address matches PFN and valid is set, then clears lock/dirty/valid to zero
		case 0x1a:
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " DHIN";
#endif
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " ADDR=" << hex << VirtualAddress;
#endif

			// make sure address is cached ?
			if ( !r->DCache.isCached( VirtualAddress ) )
			{
				break;
			}

			r->DCache.InvalidateHit ( VirtualAddress );
			
			
			break;

			
		// DHWBIN - like DHIN but does write-back if dirty bit is set
		case 0x18:
		// DHIN - invalidate data cache entry if address matches PFN and valid is set, then clears lock/dirty/valid to zero
		//case 0x1a:
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " DHWBIN";
#endif
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " ADDR=" << hex << VirtualAddress;
#endif

			// make sure address is cached ?
			//if ( ( VirtualAddress & 0x70000000 ) == 0x70000000 )
			if ( !r->DCache.isCached( VirtualAddress ) )
			{
				//cout << "\nhps2x64: R5900: DHWBIN/DHIN on uncached address:" << hex << VirtualAddress;
				break;
			}

			ulIndex = r->DCache.isCacheHit_Index ( VirtualAddress );
			
			if ( ulIndex != -1 )
			{
				//PFN = r->DCache.Get_PFN ( ulIndex );
				//r->Bus->InvalidArray.b8 [ ( PFN & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
				
				if ( r->DCache.Get_Dirty ( ulIndex ) )
				{
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " DIRTY";
#endif
					// get the physical address
					PFN = r->DCache.Get_PFN ( ulIndex );
					
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " PFN=" << hex << PFN;
#endif

					// get pointer into cache line
					//pCacheData = (u64*) r->DCache.Get_CacheLinePtr ( ulIndex );
					
					// get memory pointer
					pMemPtr64 = (u64*) r->Bus->GetIMemPtr ( PFN & 0x1fffffc0 );
					
					// write-back dirty cache line
					r->DCache.WriteBackCache ( ulIndex, pMemPtr64 );
					
					// if not bios, then invalidate recompiler cache also
//#ifdef ENABLE_INVALID_ARRAY
					r->Bus->InvalidArray.b8 [ ( PFN & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
//#endif
				}
				
				// invalidate cache entry
				r->DCache.InvalidateHit ( VirtualAddress );
			}
			
			
			break;
			
		// DHWOIN - data cache write-back if dirty bit is set, but does not invalidate data cache intry
		case 0x1c:
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " DHWOIN";
#endif

#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " ADDR=" << hex << VirtualAddress;
#endif

			// make sure address is cached ?
			//if ( ( VirtualAddress & 0x70000000 ) == 0x70000000 )
			if ( !r->DCache.isCached( VirtualAddress ) )
			{
				//cout << "\nhps2x64: R5900: DHWOIN on uncached address:" << hex << VirtualAddress;
				break;
			}
			
			ulIndex = r->DCache.isCacheHit_Index ( VirtualAddress );
			
			if ( ulIndex != -1 )
			{
				//PFN = r->DCache.Get_PFN ( ulIndex );
				//r->Bus->InvalidArray.b8 [ ( PFN & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
				
				if ( r->DCache.Get_Dirty ( ulIndex ) )
				{
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " DIRTY";
#endif

					// get the physical address
					PFN = r->DCache.Get_PFN ( ulIndex );

#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " PFN=" << hex << PFN;
#endif

					// get pointer into cache line
					//pCacheData = (u64*) r->DCache.Get_CacheLinePtr ( ulIndex );
					
					// get memory pointer
					pMemPtr64 = (u64*) r->Bus->GetIMemPtr ( PFN & 0x1fffffc0 );
					
					// write-back dirty cache line
					r->DCache.WriteBackCache ( ulIndex, pMemPtr64 );
					
					// if not bios, then invalidate recompiler cache also
//#ifdef ENABLE_INVALID_ARRAY
					r->Bus->InvalidArray.b8 [ ( PFN & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
//#endif
				}
				
			}
			
			
			break;


		// DXIN - invalidate data cache entry by index/way - clear lock/dirty/valid to zero
		case 0x16:
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " DXIN";
#endif
			// get index
			ulIndex = ( VirtualAddress >> 6 ) & 0x3f;
			ulIndex <<= 1;
			ulIndex |= ( VirtualAddress & 1 );
			
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " IDX=" << hex << ulIndex;

			if ( r->DCache.Get_Valid ( ulIndex ) )
			{
	debug << " VALID";

				// get the physical address
				PFN = r->DCache.Get_PFN ( ulIndex );

	debug << " PFN=" << hex << PFN;
				if ( r->DCache.Get_Dirty ( ulIndex ) )
				{
	debug << " DIRTY";

				}
			}
#endif

			r->DCache.InvalidateIndex ( VirtualAddress );
			
			break;

			
		
			
		// DXWBIN - invalidate data cache entry with write-back by index/way - clear lock/dirty/valid to zero
		case 0x14:
		// DXIN - invalidate data cache entry by index/way - clear lock/dirty/valid to zero
		//case 0x16:
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " DXWBIN";
#endif
			// get index
			ulIndex = ( VirtualAddress >> 6 ) & 0x3f;
			ulIndex <<= 1;
			ulIndex |= ( VirtualAddress & 1 );
			
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " IDX=" << hex << ulIndex;
#endif

			// write-back if dirty
			if ( r->DCache.Get_Valid ( ulIndex ) )
			{
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " VALID";
#endif

				//PFN = r->DCache.Get_PFN ( ulIndex );
				//r->Bus->InvalidArray.b8 [ ( PFN & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
				
				if ( r->DCache.Get_Dirty ( ulIndex ) )
				{
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " DIRTY";
#endif

					// get the physical address
					PFN = r->DCache.Get_PFN ( ulIndex );

#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << " PFN=" << hex << PFN;
#endif

					// get pointer into cache line
					//pCacheData = (u64*) r->DCache.Get_CacheLinePtr ( ulIndex );
					
					// get memory pointer
					pMemPtr64 = (u64*) r->Bus->GetIMemPtr ( PFN & 0x1fffffc0 );
					
					// write-back dirty cache line
					r->DCache.WriteBackCache ( ulIndex, pMemPtr64 );
					
					// if not bios, then invalidate recompiler cache also
//#ifdef ENABLE_INVALID_ARRAY
					r->Bus->InvalidArray.b8 [ ( PFN & r->Bus->MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ] = 1;
//#endif
				}
			}
			
			r->DCache.InvalidateIndex ( VirtualAddress );
			
			break;
			
#endif

		default:
#if defined INLINE_DEBUG_CACHE || defined INLINE_DEBUG_R5900
	debug << "\r\nhps2x64: R5900: CACHE: unimplemented cache op=" << hex << i.Rt;
#endif
#ifdef VERBOSE_CACHE_INVALID
			cout << "\nhps2x64: R5900: CACHE: unimplemented cache op=" << hex << i.Rt;
#endif
			break;
	}


}

void Execute::PREF ( Instruction::Format i )
{
#if defined INLINE_DEBUG_PREF || defined INLINE_DEBUG_R5900 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

}

void Execute::TLBR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TLBR || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

}

void Execute::TLBWI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TLBWI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

}

void Execute::TLBWR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TLBWR || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

}

void Execute::TLBP ( Instruction::Format i )
{
#if defined INLINE_DEBUG_TLBP || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

}

void Execute::ERET ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ERET || defined INLINE_DEBUG_R5900
	if ( r->DebugNextERET )
	{
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	}
#endif

	if ( r->CPR0.Status.ERL )
	{
		r->NextPC = r->CPR0.ErrorEPC;
		r->CPR0.Status.ERL = 0;
		
#if defined INLINE_DEBUG_ERET || defined INLINE_DEBUG_R5900
	if ( r->DebugNextERET )
	{
	debug << " Status.ERL=1";
	debug << " ErrorEPC=" << r->CPR0.ErrorEPC;
	debug << " NextPC=" << r->NextPC;
	//debug << "\r\nR29=" << r->GPR [ 29 ].uw0 << " R31=" << r->GPR [ 31 ].uw0 << " 0x3201d0=" << r->Bus->MainMemory.b32 [ 0x3201d0 >> 2 ];
	//r->DebugNextERET = 0;
	}
#endif
	}
	else
	{
		r->NextPC = r->CPR0.EPC;
		r->CPR0.Status.EXL = 0;
		
#if defined INLINE_DEBUG_ERET || defined INLINE_DEBUG_R5900
	if ( r->DebugNextERET )
	{
	debug << " Status.ERL=0";
	debug << " EPC=" << r->CPR0.EPC;
	debug << " NextPC=" << r->NextPC;
	//debug << "\r\nR29=" << r->GPR [ 29 ].uw0 << " R31=" << r->GPR [ 31 ].uw0 << " 0x3201d0=" << r->Bus->MainMemory.b32 [ 0x3201d0 >> 2 ];
	//r->DebugNextERET = 0;
	}
#endif
	}
	
	// interrupt status changed
	r->UpdateInterrupt ();
	
#ifdef ENABLE_R5900_BRANCH_PREDICTION
	r->CycleCount += r->c_ullLatency_BranchMisPredict;
#endif


//#define VERBOSE_UNHANDLED_INT
#ifdef VERBOSE_UNHANDLED_INT
	// check for an interrupt after eret
	if ( r->Status.Value & 1 )
	{
		cout << "\nhps2x64: ALERT: R5900: Interrupt unhandled before ERET. DATA=" << hex << (r->CPR0.Regs [ 13 ] & 0x8c00);
	}
#endif

#if defined INLINE_DEBUG_ERET || defined INLINE_DEBUG_R5900
	if ( r->DebugNextERET )
	{
	debug << hex << " ISTAT=" << *r->_Debug_IntcStat << " IMASK=" << *r->_Debug_IntcMask;
	debug << " ERETPROCSTAT=" << (r->Status.Value & 1);
	r->DebugNextERET = 0;
	}
#endif
}

// R5900 does not have RFE instruction //
//void Execute::RFE ( Instruction::Format i )
//{
//	strInstString << "RFE";
//	AddInstArgs ( strInstString, instruction, FTRFE );
//}


void Execute::DERET ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DERET || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

}

void Execute::WAIT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_WAIT || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

}





// * COP1 (floating point) instructions * //


void Execute::MFC1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MFC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << r->CPR1 [ i.Fs ].u;
#endif

	// no delay slot on R5900?
	r->GPR [ i.Rt ].sq0 = (s32) r->CPR1 [ i.Rd ].s;
	
#if defined INLINE_DEBUG_MFC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << " Output: Rt=" << r->GPR [ i.Rt ].sq0;
#endif
}

void Execute::MTC1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MTC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Rt=" << r->GPR [ i.Rt ].sw0;
#endif

	// no delay slot on R5900?
	r->CPR1 [ i.Rd ].s = r->GPR [ i.Rt ].sw0;
	
#if defined INLINE_DEBUG_MTC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << " Output: Fs=" << r->CPR1 [ i.Fs ].u;
#endif
}

void Execute::CFC1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CFC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " CFs=" << r->CPC1 [ i.Rd ];
#endif

	// no delay slot on R5900?
	r->GPR [ i.Rt ].sq0 = (s32) r->Read_CFC1 ( i.Rd );
	
#if defined INLINE_DEBUG_CFC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << " Output: Rt=" << r->GPR [ i.Rt ].sq0;
#endif
}

void Execute::CTC1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CTC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Rt=" << r->GPR [ i.Rt ].sw0;
#endif

	// no delay slot on R5900?
	//r->CPC1 [ i.Rd ] = r->GPR [ i.Rt ].sw0;
	r->Write_CTC1 ( i.Rd, r->GPR [ i.Rt ].sw0 );
	
#if defined INLINE_DEBUG_CTC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << " Output: CFs=" << r->CPC1 [ i.Rd ];
#endif
}


void Execute::LWC1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LWC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: base = " << r->GPR [ i.Base ].u;
#endif

	// LWC1 rt, offset(base)
	
	u32 LoadAddress;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
#if defined INLINE_DEBUG_LWC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << " LoadAddress=" << LoadAddress;
#endif

	// *** testing *** alert if load is from unaligned address
	if ( LoadAddress & 0x3 )
	{
		cout << "\nhps2x64 ALERT: LoadAddress is unaligned for LWC1 @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
		return;
	}
	
#ifdef ENABLE_R5900_DCACHE_LWC1
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load_blocking ( LoadAddress );
		
		// load from data cache into register
		r->CPR1 [ i.Rt ].s = ((s32*)pMemPtr64) [ ( LoadAddress & 0x3f ) >> 2 ];
	}
	else
	{
#ifdef INLINE_DEBUG_DCACHE
	debug << "-$";
#endif
		// address not cached //
		
		// load the register from bus
		r->CPR1 [ i.Rt ].s = (s32) r->Bus->Read_t<0xffffffff> ( LoadAddress );
		
		handle_uncached_load_blocking ();
	}

#else
	
	// ***todo*** perform signed load of word
	//r->CPR1 [ i.Rt ].s = (s32) r->Bus->Read ( LoadAddress, 0xffffffffULL );
	r->CPR1 [ i.Rt ].s = (s32) r->Bus->Read_t<0xffffffff> ( LoadAddress );
	
	
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
	
#endif

#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->GPR [ i.Rt ].uw0 )
#endif
	
#if defined INLINE_DEBUG_LWC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << " Output: C1=" << r->CPR1 [ i.Rt ].u << " " << r->CPR1 [ i.Rt ].f;
#endif
}

void Execute::SWC1 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SWC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "; Input: rt = " << r->CPR1 [ i.Rt ].u << " " << r->CPR1 [ i.Rt ].f << "; base = " << r->GPR [ i.Base ].u;
#endif

	// SWC1 rt, offset(base)
	u32 StoreAddress;
	u64 *pMemPtr64;
	
	// check if storing to data cache
	//StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	StoreAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
#if defined INLINE_DEBUG_SWC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << " StoreAddress=" << StoreAddress;
#endif

	// *** testing *** alert if load is from unaligned address
	if ( StoreAddress & 0x3 )
	{
		cout << "\nhps2x64 ALERT: StoreAddress is unaligned for SWC1 @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
		
#if defined INLINE_DEBUG_SWC1 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
		debug << "\r\nhps2x64 ALERT: StoreAddress is unaligned for SWC1 @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\r\n";
#endif

		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADES );
		return;
	}
	
	// clear top 3 bits since there is no data cache for caching stores
	// don't clear top 3 bits since scratchpad is at 0x70000000
	//StoreAddress &= 0x1fffffff;
	
#ifdef ENABLE_R5900_DCACHE_SWC1
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( StoreAddress ) )
	{
		pMemPtr64 = handle_cached_store_blocking ( StoreAddress );
		
		// load from data cache into register
		((s32*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 2 ] = r->CPR1 [ i.Rt ].s;
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->Bus->Write_t<0xffffffff> ( StoreAddress, r->CPR1 [ i.Rt ].u );
		
		handle_uncached_store_blocking ();
	}

#else

	// ***todo*** perform store of word
	//r->Bus->Write ( StoreAddress, r->CPR1 [ i.Rt ].u, 0xffffffffULL );
	r->Bus->Write_t<0xffffffff> ( StoreAddress, r->CPR1 [ i.Rt ].u );
	
	

	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
#endif
	
#ifdef INLINE_DEBUG_TRACE
	TRACE_VALUE ( r->CPR1 [ i.Rt ].u )
#endif
}





void Execute::ABS_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ABS_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
#endif

	r->CPR1 [ i.Fd ].u = r->CPR1 [ i.Fs ].u & 0x7fffffff;
	
	// flags affected:
	// clears flags o,u (bits 14,15)
	r->CPC1 [ 31 ] &= ~0x0000c000;

#if defined INLINE_DEBUG_ABS_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
#endif
}


void Execute::ADD_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ADD_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
#endif

	//u32 OverflowFlag, UnderflowFlag, OverflowSticky, UnderflowSticky, DummyFlag, FlagSet;
	u16 StatusFlag = 0, DummyMACFlag;
	u32 FlagSet;

	// fd = fs + ft
	r->CPR1 [ i.Fd ].f = PS2_Float_Add ( r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
										
	// clear bits 14, 15 in flag register
	r->CPC1 [ 31 ] &= ~0x0000c000;
	
	// get flags to set
	FlagSet = StatusFlag & 0xc;
	FlagSet = ( FlagSet << 12 ) | ( FlagSet << 1 );

	// overflow flag is bit 15
	// underflow flag is bit 14
	// sticky overflow flag is bit 4
	// sticky underflow flag is bit 3
	r->CPC1 [ 31 ] |= FlagSet;

#if defined INLINE_DEBUG_ADD_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
	debug << " FlagSet=" << hex << FlagSet;
#endif
}


void Execute::ADDA_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDA_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
#endif

	//u32 OverflowFlag, UnderflowFlag, OverflowSticky, UnderflowSticky, DummyFlag, FlagSet;
	u16 StatusFlag = 0, DummyMACFlag;
	u32 FlagSet;
	
	// ACC = fs + ft
	//r->dACC.f = PS2_Float_AddA ( r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	r->dACC.f = PS2_Float_Add ( r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	
	// clear bits 14, 15 in flag register
	r->CPC1 [ 31 ] &= ~0x0000c000;
	
	// get flags to set
	FlagSet = StatusFlag & 0xc;
	FlagSet = ( FlagSet << 12 ) | ( FlagSet << 1 );

	// overflow flag is bit 15
	// underflow flag is bit 14
	// sticky overflow flag is bit 4
	// sticky underflow flag is bit 3
	r->CPC1 [ 31 ] |= FlagSet;

#if defined INLINE_DEBUG_ADDA_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC=" << hex << r->dACC.l << " " << r->dACC.f;
	debug << " FlagSet=" << hex << FlagSet;
#endif
}



void Execute::CVT_S_W ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CVT_S_W || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
#endif

	// fd = FLOAT ( fs )
	r->CPR1 [ i.Fd ].f = (float) r->CPR1 [ i.Fs ].s;
	
#if defined INLINE_DEBUG_CVT_S_W || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
#endif
}


void Execute::SUB_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SUB_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
	float fs, ft;
	FloatLong fl;
	fs = r->CPR1[i.Fs].f;
	ft = r->CPR1[i.Ft].f;
	fl.f = fs - ft;
#endif

	//u32 OverflowFlag, UnderflowFlag, OverflowSticky, UnderflowSticky, DummyFlag, FlagSet;
	u16 StatusFlag = 0, DummyMACFlag;
	u32 FlagSet;
	
	// fd = fs - ft
	r->CPR1 [ i.Fd ].f = PS2_Float_Sub ( r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
										
	// clear bits 14, 15 in flag register
	r->CPC1 [ 31 ] &= ~0x0000c000;
	
	// get flags to set
	FlagSet = StatusFlag & 0xc;
	FlagSet = ( FlagSet << 12 ) | ( FlagSet << 1 );

	// overflow flag is bit 15
	// underflow flag is bit 14
	// sticky overflow flag is bit 4
	// sticky underflow flag is bit 3
	r->CPC1 [ 31 ] |= FlagSet;

#if defined INLINE_DEBUG_SUB_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
	debug << " FlagSet=" << hex << FlagSet;
	if (fl.l != r->CPR1[i.Fd].s)
	{
		debug << " DIFF=" << hex << fl.l << " " << fl.f;
	}
	if (std::abs(fl.l- r->CPR1[i.Fd].s) > 1)
	{
		debug << " MAJOR";
	}
	if (FlagSet)
	{
		debug << " OVFUND";
	}
#endif
}

void Execute::MUL_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MUL_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
#endif

	//u32 OverflowFlag, UnderflowFlag, OverflowSticky, UnderflowSticky, DummyFlag, FlagSet;
	u16 StatusFlag = 0, DummyMACFlag;
	u32 FlagSet;
	
	// fd = fs * ft
	//r->CPR1 [ i.Fd ].f = r->CPR1 [ i.Fs ].f * r->CPR1 [ i.Ft ].f;
	r->CPR1 [ i.Fd ].f = PS2_Float_Mul ( r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	
	// clear bits 14, 15 in flag register
	r->CPC1 [ 31 ] &= ~0x0000c000;
	
	// get flags to set
	FlagSet = StatusFlag & 0xc;
	FlagSet = ( FlagSet << 12 ) | ( FlagSet << 1 );

	// overflow flag is bit 15
	// underflow flag is bit 14
	// sticky overflow flag is bit 4
	// sticky underflow flag is bit 3
	r->CPC1 [ 31 ] |= FlagSet;

#if defined INLINE_DEBUG_MUL_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
	debug << " FlagSet=" << hex << FlagSet;
#endif
}

void Execute::MULA_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MULA_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
	debug << " ACC=" << hex << r->dACC.l << " " << r->dACC.f;
#endif

	//u32 OverflowFlag, UnderflowFlag, OverflowSticky, UnderflowSticky, DummyFlag, FlagSet;
	u16 StatusFlag = 0, DummyMACFlag;
	u32 FlagSet;
	
	// ACC = fs * ft
	//r->dACC.f = PS2_Float_MulA ( r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	r->dACC.f = PS2_Float_Mul ( r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	
	// clear bits 14, 15 in flag register
	r->CPC1 [ 31 ] &= ~0x0000c000;
	
	// get flags to set
	FlagSet = StatusFlag & 0xc;
	FlagSet = ( FlagSet << 12 ) | ( FlagSet << 1 );

	// overflow flag is bit 15
	// underflow flag is bit 14
	// sticky overflow flag is bit 4
	// sticky underflow flag is bit 3
	r->CPC1 [ 31 ] |= FlagSet;

#if defined INLINE_DEBUG_MULA_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC=" << hex << r->dACC.l << " " << r->dACC.f;
	debug << " FlagSet=" << hex << FlagSet;
#endif
}


void Execute::DIV_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_DIV_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
#endif

	u16 StatusFlag = 0;
	u32 FlagSet;

	// fd = fs / ft
	//r->CPR1 [ i.Fd ].f = r->CPR1 [ i.Fs ].f / r->CPR1 [ i.Ft ].f;
	r->CPR1 [ i.Fd ].f = PS2_Float_Div ( r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, & StatusFlag );
	
	// clear non-sticky invalid/divide flags (bits 16,17)
	r->CPC1 [ 31 ] &= ~0x00030000;
	
	// ***TODO*** set flags
	// sticky divide flag is bit 5
	// sticky invalid flag is bit 6
	// divide flag is bit 16
	// invalid flag is bit 17
	
	// swap bits..
	FlagSet = ( StatusFlag & 0x10 ) | ( ( StatusFlag & 0x20 ) >> 2 );
	FlagSet = ( FlagSet << 13 ) | ( FlagSet << 2 );
	
	// set flags
	r->CPC1 [ 31 ] |= FlagSet;

#if defined INLINE_DEBUG_DIV_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
	debug << " FlagSet=" << hex << FlagSet;
#endif
}

void Execute::SQRT_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SQRT_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
#endif

	u16 StatusFlag = 0;
	u32 FlagSet;

	// fd = sqrt ( ft )
	r->CPR1 [ i.Fd ].f = PS2_Float_Sqrt ( r->CPR1 [ i.Ft ].f, & StatusFlag );
	
	// clear the affected non-sticky flags (bits 16,17)
	r->CPC1 [ 31 ] &= ~0x00030000;
	
	// ***TODO*** set flags
	// get invalid flag
	FlagSet = StatusFlag & 0x10;
	FlagSet = ( FlagSet << 13 ) | ( FlagSet << 2 );
	
	// set the flags
	r->CPC1 [ 31 ] |= FlagSet;
	
#if defined INLINE_DEBUG_SQRT_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
	debug << " FlagSet=" << hex << FlagSet;
#endif
}


void Execute::RSQRT_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_RSQRT_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
#endif

	u16 StatusFlag = 0;
	u32 FlagSet;
	
	r->CPR1 [ i.Fd ].f = PS2_Float_RSqrt ( r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, & StatusFlag );
	
	// clear the affected non-sticky flags (bits 16,17)
	r->CPC1 [ 31 ] &= ~0x00030000;
	
	// ***todo*** flags
	// swap bits..
	FlagSet = ( StatusFlag & 0x10 ) | ( ( StatusFlag & 0x20 ) >> 2 );
	FlagSet = ( FlagSet << 13 ) | ( FlagSet << 2 );
	
	// set flags
	r->CPC1 [ 31 ] |= FlagSet;
	
#if defined INLINE_DEBUG_RSQRT_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
	debug << " FlagSet=" << hex << FlagSet;
#endif
}




void Execute::MOV_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MOV_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
#endif

	r->CPR1 [ i.Fd ].u = r->CPR1 [ i.Fs ].u;
	
	// flags affected: none

#if defined INLINE_DEBUG_MOV_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
#endif
}

void Execute::NEG_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_NEG_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
#endif

	r->CPR1 [ i.Fd ].u = r->CPR1 [ i.Fs ].u ^ 0x80000000;
	
	// flags affected:
	// clears flags o,u (bits 14,15)
	r->CPC1 [ 31 ] &= ~0x0000c000;

#if defined INLINE_DEBUG_NEG_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
#endif
}



void Execute::SUBA_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBA_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
	debug << " ACC=" << hex << r->dACC.l << " " << r->dACC.f;
#endif

	//u32 OverflowFlag, UnderflowFlag, OverflowSticky, UnderflowSticky, DummyFlag, FlagSet;
	u16 StatusFlag = 0, DummyMACFlag;
	u32 FlagSet;
	
	//r->dACC.f = PS2_Float_SubA ( r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	r->dACC.f = PS2_Float_Sub ( r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	
	// clear bits 14, 15 in flag register
	r->CPC1 [ 31 ] &= ~0x0000c000;
	
	// get flags to set
	FlagSet = StatusFlag & 0xc;
	FlagSet = ( FlagSet << 12 ) | ( FlagSet << 1 );

	// overflow flag is bit 15
	// underflow flag is bit 14
	// sticky overflow flag is bit 4
	// sticky underflow flag is bit 3
	r->CPC1 [ 31 ] |= FlagSet;
	
#if defined INLINE_DEBUG_SUBA_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " ACC=" << hex << r->dACC.l << " " << r->dACC.f;
	debug << " FlagSet=" << hex << FlagSet;
#endif
}

void Execute::MADD_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MADD_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
	debug << " ACC=" << hex << r->dACC.l << " " << r->dACC.f;
#endif

	//u32 OverflowFlag, UnderflowFlag, OverflowSticky, UnderflowSticky, DummyFlag, FlagSet;
	u16 StatusFlag = 0, DummyMACFlag;
	u32 FlagSet;
	u32 StickyFlagSet;
	
	float fTemp;

	// fd = fs * ft + ACC
	//fTemp = r->CPR1 [ i.Fs ].f * r->CPR1 [ i.Ft ].f;
	//r->CPR1 [ i.Fd ].f = fTemp + r->ACC.f;
	//r->CPR1 [ i.Fd ].f = PS2_Float_Madd ( r->dACC.f, r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	r->CPR1 [ i.Fd ].f = PS2_Float_Madd ( r->dACC.f, r->CPR1 [ i.Fd ].f, r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	
	// clear bits 14, 15 in flag register
	r->CPC1 [ 31 ] &= ~0x0000c000;
	
	// get flags to set
	//FlagSet = StatusFlag & 0xc;
	//FlagSet = ( FlagSet << 12 ) | ( FlagSet << 1 );
	
	// get sticky flags
	StickyFlagSet = ( StatusFlag & 0x300 ) >> 5;
	
	// get stat flags
	FlagSet = ( StatusFlag & 0xc ) << 12;
	
	// include sticky flags
	FlagSet |= StickyFlagSet;

	// overflow flag is bit 15
	// underflow flag is bit 14
	// sticky overflow flag is bit 4
	// sticky underflow flag is bit 3
	r->CPC1 [ 31 ] |= FlagSet;

#if defined INLINE_DEBUG_MADD_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
	debug << " FlagSet=" << hex << FlagSet;
#endif
}

void Execute::MSUB_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUB_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
	debug << " ACC=" << hex << r->dACC.l << " " << r->dACC.f;
#endif

	//u32 OverflowFlag, UnderflowFlag, OverflowSticky, UnderflowSticky, DummyFlag, FlagSet;
	u16 StatusFlag = 0, DummyMACFlag;
	u32 FlagSet;
	u32 StickyFlagSet;
	
	//r->CPR1 [ i.Fd ].f = PS2_Float_Msub ( r->dACC.f, r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	r->CPR1 [ i.Fd ].f = PS2_Float_Msub ( r->dACC.f, r->CPR1 [ i.Fd ].f, r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	
	// clear bits 14, 15 in flag register
	r->CPC1 [ 31 ] &= ~0x0000c000;
	
	// get flags to set
	//FlagSet = StatusFlag & 0xc;
	//FlagSet = ( FlagSet << 12 ) | ( FlagSet << 1 );

	// get sticky flags
	StickyFlagSet = ( StatusFlag & 0x300 ) >> 5;
	
	// get stat flags
	FlagSet = ( StatusFlag & 0xc ) << 12;
	
	// include sticky flags
	FlagSet |= StickyFlagSet;
	
	// overflow flag is bit 15
	// underflow flag is bit 14
	// sticky overflow flag is bit 4
	// sticky underflow flag is bit 3
	r->CPC1 [ 31 ] |= FlagSet;
	
#if defined INLINE_DEBUG_MSUB_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
	debug << " FlagSet=" << hex << FlagSet;
#endif
}

void Execute::MSUBA_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBA_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
	debug << " ACC=" << hex << r->dACC.l << " " << r->dACC.f;
#endif

	//u32 OverflowFlag, UnderflowFlag, OverflowSticky, UnderflowSticky, DummyFlag, FlagSet;
	u16 StatusFlag = 0, DummyMACFlag;
	u32 FlagSet;
	u32 StickyFlagSet;
	
	//r->dACC.f = PS2_Float_Msub ( r->dACC.f, r->CPR1 [ i.Fd ].f, r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	r->dACC.f = PS2_Float_Msub ( r->dACC.f, r->dACC.f, r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	
	// clear bits 14, 15 in flag register
	r->CPC1 [ 31 ] &= ~0x0000c000;
	
	// get flags to set
	//FlagSet = StatusFlag & 0xc;
	//FlagSet = ( FlagSet << 12 ) | ( FlagSet << 1 );

	// get sticky flags
	StickyFlagSet = ( StatusFlag & 0x300 ) >> 5;
	
	// get stat flags
	FlagSet = ( StatusFlag & 0xc ) << 12;
	
	// include sticky flags
	FlagSet |= StickyFlagSet;
	
	// overflow flag is bit 15
	// underflow flag is bit 14
	// sticky overflow flag is bit 4
	// sticky underflow flag is bit 3
	r->CPC1 [ 31 ] |= FlagSet;
	
#if defined INLINE_DEBUG_MSUBA_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " ACC=" << hex << r->dACC.l << " " << r->dACC.f;
	debug << " FlagSet=" << hex << FlagSet;
#endif
}

void Execute::MADDA_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDA_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
	debug << " ACC=" << hex << r->dACC.l << " " << r->dACC.f;
#endif

	//u32 OverflowFlag, UnderflowFlag, OverflowSticky, UnderflowSticky, DummyFlag, FlagSet;
	u16 StatusFlag = 0, DummyMACFlag;
	u32 FlagSet;
	u32 StickyFlagSet;
	
	//r->dACC.f = PS2_Float_Madd ( r->dACC.f, r->CPR1 [ i.Fd ].f, r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
	r->dACC.f = PS2_Float_Madd ( r->dACC.f, r->dACC.f, r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f, 0, & StatusFlag, & DummyMACFlag );
										
	// clear bits 14, 15 in flag register
	r->CPC1 [ 31 ] &= ~0x0000c000;
	
	// get flags to set
	//FlagSet = StatusFlag & 0xc;
	//FlagSet = ( FlagSet << 12 ) | ( FlagSet << 1 );

	// get sticky flags
	StickyFlagSet = ( StatusFlag & 0x300 ) >> 5;
	
	// get stat flags
	FlagSet = ( StatusFlag & 0xc ) << 12;
	
	// include sticky flags
	FlagSet |= StickyFlagSet;
	
	// overflow flag is bit 15
	// underflow flag is bit 14
	// sticky overflow flag is bit 4
	// sticky underflow flag is bit 3
	r->CPC1 [ 31 ] |= FlagSet;
	
#if defined INLINE_DEBUG_MADDA_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " ACC=" << hex << r->dACC.l << " " << r->dACC.f;
	debug << " FlagSet=" << hex << FlagSet;
#endif
}

void Execute::CVT_W_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_CVT_W_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
#endif

	// fd = INT ( fs )
	/*
	if ( ( r->CPR1 [ i.Fs ].u & 0x7f800000 ) <= 0x4e800000 )
	{
		r->CPR1 [ i.Fd ].s = (s32) r->CPR1 [ i.Fs ].f;
	}
	else if ( r->CPR1 [ i.Fs ].u & 0x80000000 )
	{
		// set to negative integer max
		r->CPR1 [ i.Fd ].u = 0x80000000;
	}
	else
	{
		// set to positive integer max
		r->CPR1 [ i.Fd ].u = 0x7fffffff;
	}
	*/
	
	r->CPR1 [ i.Fd ].u = PS2_Float_ToInteger ( r->CPR1 [ i.Fs ].f );
	
#if defined INLINE_DEBUG_CVT_W_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
#endif
}

void Execute::MAX_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MAX_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
#endif

	// fd = MAX ( fs, ft )
	//r->CPR1 [ i.Fd ].f = ( ( r->CPR1 [ i.Fs ].f >= r->CPR1 [ i.Ft ].f ) ? r->CPR1 [ i.Fs ].f : r->CPR1 [ i.Ft ].f );
	r->CPR1 [ i.Fd ].f = PS2_Float_Max ( r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f );

	// flags affected:
	// clears flags o,u (bits 14,15)
	r->CPC1 [ 31 ] &= ~0x0000c000;
	
#if defined INLINE_DEBUG_MAX_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
#endif
}

void Execute::MIN_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_MIN_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
#endif

	// fd = MIN ( fs, ft )
	//r->CPR1 [ i.Fd ].f = ( ( r->CPR1 [ i.Fs ].f <= r->CPR1 [ i.Ft ].f ) ? r->CPR1 [ i.Fs ].f : r->CPR1 [ i.Ft ].f );
	r->CPR1 [ i.Fd ].f = PS2_Float_Min ( r->CPR1 [ i.Fs ].f, r->CPR1 [ i.Ft ].f );

	// flags affected:
	// clears flags o,u (bits 14,15)
	r->CPC1 [ 31 ] &= ~0x0000c000;
	
#if defined INLINE_DEBUG_MIN_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << hex << r->CPR1 [ i.Fd ].u << " " << r->CPR1 [ i.Fd ].f;
#endif
}

void Execute::C_F_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_C_F_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// clears bit 23 in FCR31
	r->CPC1 [ 31 ] &= ~0x00800000;
	
#if defined INLINE_DEBUG_C_F_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: FCR31=" << hex << r->CPC1 [ 31 ];
#endif
}

void Execute::C_EQ_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_C_EQ_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
#endif

	float fs, ft;
	long lfs, lft;
	
	fs = r->CPR1 [ i.Fs ].f;
	ft = r->CPR1 [ i.Ft ].f;

	//PS2Float::ClampValue2_f ( fs, ft );
	lfs = PS2Float::FlushConvertToComparableInt_f ( fs );
	lft = PS2Float::FlushConvertToComparableInt_f ( ft );
	
	//if ( fs == ft )
	if ( lfs == lft )
	{
#if defined INLINE_DEBUG_C_EQ_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << " EQUAL";
#endif

		r->CPC1 [ 31 ] |= 0x00800000;
	}
	else
	{
#if defined INLINE_DEBUG_C_EQ_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << " NOTEQUAL";
#endif

		r->CPC1 [ 31 ] &= ~0x00800000;
	}
	
#if defined INLINE_DEBUG_C_EQ_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: FCR31=" << hex << r->CPC1 [ 31 ];
#endif
}

void Execute::C_LT_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_C_LT_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
#endif

	float fs, ft;
	long lfs, lft;
	
	fs = r->CPR1 [ i.Fs ].f;
	ft = r->CPR1 [ i.Ft ].f;

	//PS2Float::ClampValue2_f ( fs, ft );
	
	// flush denormals to zero here, but don't clamp, and also make integer
	lfs = PS2Float::FlushConvertToComparableInt_f ( fs );
	lft = PS2Float::FlushConvertToComparableInt_f ( ft );
	
	// Cond = fs < ft
	// ***todo*** handle non-ieee values for this too
	// compare as integer ??
	//if ( fs < ft )
	if ( lfs < lft )
	{
#if defined INLINE_DEBUG_C_LT_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << " LESS";
#endif

		r->CPC1 [ 31 ] |= 0x00800000;
	}
	else
	{
#if defined INLINE_DEBUG_C_LT_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << " NOTLESS";
#endif

		r->CPC1 [ 31 ] &= ~0x00800000;
	}
	
#if defined INLINE_DEBUG_C_LT_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: FCR31=" << hex << r->CPC1 [ 31 ];
#endif
}

void Execute::C_LE_S ( Instruction::Format i )
{
#if defined INLINE_DEBUG_C_LE_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << r->CPR1 [ i.Fs ].u << " " << r->CPR1 [ i.Fs ].f;
	debug << " Ft=" << hex << r->CPR1 [ i.Ft ].u << " " << r->CPR1 [ i.Ft ].f;
#endif

	float fs, ft;
	long lfs, lft;
	
	fs = r->CPR1 [ i.Fs ].f;
	ft = r->CPR1 [ i.Ft ].f;

	//PS2Float::ClampValue2_f ( fs, ft );
	
	
	// flush denormals to zero here, but don't clamp, and also make integer
	lfs = PS2Float::FlushConvertToComparableInt_f ( fs );
	lft = PS2Float::FlushConvertToComparableInt_f ( ft );
	
	// compare as integer ??
	//if ( fs <= ft )
	if ( lfs <= lft )
	{
#if defined INLINE_DEBUG_C_LE_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << " LESSEQ";
#endif

		r->CPC1 [ 31 ] |= 0x00800000;
	}
	else
	{
#if defined INLINE_DEBUG_C_LE_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU
	debug << " NOTLESSEQ";
#endif

		r->CPC1 [ 31 ] &= ~0x00800000;
	}
	
#if defined INLINE_DEBUG_C_LE_S || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: FCR31=" << hex << r->CPC1 [ 31 ];
#endif
}



// * COP2 (VU0) instrutions * //



// PS2 has LQC2/SQC2 instead of LWC2/SWC2 //
void Execute::LQC2 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_LQC2 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Base=" << r->GPR [ i.Base ].sw0;
#endif

	// LQC2 ft, offset(base)
	
	u32 LoadAddress;
	u64* Data;
	u64 *pMemPtr64;
	
	// set load to happen after delay slot
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	LoadAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
#if defined INLINE_DEBUG_LQC2 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " LoadAddress=" << LoadAddress;
#endif

	// *** testing *** alert if load is from unaligned address
	if ( LoadAddress & 0xf )
	{
#if defined INLINE_DEBUG_LQC2 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
		debug << " UNALIGNED";
#endif

		cout << "\nhps2x64 ALERT: LoadAddress is unaligned for LQC2 @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << LoadAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
		return;
	}
	
	// bottom four bits of Address are cleared
	//LoadAddress &= ~0xf;
	
#ifdef ENABLE_R5900_DCACHE_LQC2
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( LoadAddress ) )
	{
		pMemPtr64 = handle_cached_load_blocking ( LoadAddress );
		
		// load from data cache into register
		Data = & ((u64*)pMemPtr64) [ ( LoadAddress & 0x3f ) >> 3 ];
		 
		VU0::_VU0->vf [ i.Ft ].uq0 = Data [ 0 ];
		VU0::_VU0->vf [ i.Ft ].uq1 = Data [ 1 ];
	}
	else
	{
		// address not cached //
		
		
		// load the register from bus
		Data = (u64*) r->Bus->Read_t<0> ( LoadAddress );
		
		VU0::_VU0->vf [ i.Ft ].uq0 = Data [ 0 ];
		VU0::_VU0->vf [ i.Ft ].uq1 = Data [ 1 ];
		
		handle_uncached_load_blocking ();
	}

#else
	
	
	// ***todo*** perform signed load of word
	//Data = (u64*) r->Bus->Read ( LoadAddress, 0 );
	//r->GPR [ i.Rt ].uLo = Data [ 1 ];
	//r->GPR [ i.Rt ].uHi = Data [ 0 ];
	//Data = (u64*) r->Bus->Read ( LoadAddress, 0 );
	Data = (u64*) r->Bus->Read_t<0> ( LoadAddress );

	VU0::_VU0->vf [ i.Ft ].uq0 = Data [ 0 ];
	VU0::_VU0->vf [ i.Ft ].uq1 = Data [ 1 ];
	
	
	// used for debugging
	r->Last_ReadAddress = LoadAddress;
	r->Last_ReadWriteAddress = LoadAddress;
	
#endif
	
#if defined INLINE_DEBUG_LQC2 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output:" << " ft=" << VU0::_VU0->vf [ i.Ft ].sq0 << " " << VU0::_VU0->vf [ i.Ft ].sq1;
#endif
}

void Execute::SQC2 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_SQC2 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Base=" << r->GPR [ i.Base ].sw0;
	debug << " ft=" << VU0::_VU0->vf [ i.Ft ].sq0 << " " << VU0::_VU0->vf [ i.Ft ].sq1;
#endif

	// SQC2 ft, offset(base)
	u32 StoreAddress;
	u64 *pMemPtr64;
	u64 *Data;
	
	// check if storing to data cache
	//StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	StoreAddress = r->GPR [ i.Base ].sw0 + i.sOffset;
	
#if defined INLINE_DEBUG_SQC2 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " StoreAddress=" << StoreAddress;
#endif

	// *** testing *** alert if load is from unaligned address
	if ( StoreAddress & 0xf )
	{
#if defined INLINE_DEBUG_SQC2 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
		debug << " UNALIGNED";
#endif

		cout << "\nhps2x64 ALERT: StoreAddress is unaligned for SQC2 @ cycle=" << dec << r->CycleCount << " PC=" << hex << r->PC << " Address=" << StoreAddress << "\n";
		
		// *** testing ***
		r->ProcessSynchronousInterrupt ( Cpu::EXC_ADES );
		return;
	}
	
	// clear top 3 bits since there is no data cache for caching stores
	// don't clear top 3 bits since scratchpad is at 0x70000000
	//StoreAddress &= 0x1fffffff;
	
	// bottom four bits of Address are cleared
	//StoreAddress &= ~0xf;
	
#ifdef ENABLE_R5900_DCACHE_SQC2
	
	// check if data is in cache before reaching out to the bus
	if ( r->DCache.isCached ( StoreAddress ) )
	{
		pMemPtr64 = handle_cached_store_blocking ( StoreAddress );

		// load from data cache into register
		Data = & ((u64*)pMemPtr64) [ ( StoreAddress & 0x3f ) >> 3 ];
		
		Data [ 0 ] = VU0::_VU0->vf [ i.Ft ].uq0;
		Data [ 1 ] = VU0::_VU0->vf [ i.Ft ].uq1;
	}
	else
	{
		// address not cached //
		
		// load the register from bus
		r->Bus->Write_t<0> ( StoreAddress, (u64) & ( VU0::_VU0->vf [ i.Ft ].uw0 ) );
		
		handle_uncached_store_blocking ();
	}

#else

	
	// ***todo*** perform store of word
	// *note* probably want to pass a pointer to hi part, since that is in lower area of memory
	//r->Bus->Write ( StoreAddress, & ( VU0::_VU0->vf [ i.Ft ].uw0 ), 0 );
	r->Bus->Write_t<0> ( StoreAddress, (u64) & ( VU0::_VU0->vf [ i.Ft ].uw0 ) );
	

	// used for debugging
	r->Last_WriteAddress = StoreAddress;
	r->Last_ReadWriteAddress = StoreAddress;
	
#endif

}


void Execute::QMFC2_NI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_QMFC2_NI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Rd/Fs=" << VU0::_VU0->vf [ i.Rd ].sq0 << " " << VU0::_VU0->vf [ i.Rd ].sq1;
#endif

	// QMFC2.NI rt, fd -> fd is actually rd
	// NI -> no interlocking. does not wait for previous VCALLMS to complete

	//r->GPR [ i.Rt ].sq0 = r->CPR2 [ i.Fd ].sq0;
	//r->GPR [ i.Rt ].sq1 = r->CPR2 [ i.Fd ].sq1;
	
	r->GPR [ i.Rt ].sq0 = VU0::_VU0->vf [ i.Rd ].sq0;
	r->GPR [ i.Rt ].sq1 = VU0::_VU0->vf [ i.Rd ].sq1;
	
#if defined INLINE_DEBUG_QMFC2_NI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output:" << " Rt=" << r->GPR [ i.Rt ].sq0 << " " << r->GPR [ i.Rt ].sq1;
#endif
}

void Execute::QMFC2_I ( Instruction::Format i )
{
#if defined INLINE_DEBUG_QMFC2_I || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Rd/Fs=" << VU0::_VU0->vf [ i.Rd ].sq0 << " " << VU0::_VU0->vf [ i.Rd ].sq1;
#endif

	// QMFC2.I rt, fd
	// I -> interlocking. waits for previous VCALLMS to complete
	// ***todo*** implement interlocking

	//r->GPR [ i.Rt ].sq0 = r->CPR2 [ i.Fd ].sq0;
	//r->GPR [ i.Rt ].sq1 = r->CPR2 [ i.Fd ].sq1;
	
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
	{
		r->GPR [ i.Rt ].sq0 = VU0::_VU0->vf [ i.Rd ].sq0;
		r->GPR [ i.Rt ].sq1 = VU0::_VU0->vf [ i.Rd ].sq1;
	}
	
#if defined INLINE_DEBUG_QMFC2_I || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output:" << " Rt=" << r->GPR [ i.Rt ].sq0 << " " << r->GPR [ i.Rt ].sq1;
#endif
}

void Execute::QMTC2_NI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_QMTC2_NI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Rt=" << r->GPR [ i.Rt ].uw0 << " " << r->GPR [ i.Rt ].uw1 << " " << r->GPR [ i.Rt ].uw2 << " " << r->GPR [ i.Rt ].uw3;
#endif

	// QMTC2.NI rt, fd
	// NI -> no interlocking. does not wait for previous VCALLMS to complete

	//r->CPR2 [ i.Rt ].sq0 = r->GPR [ i.Fd ].sq0;
	//r->CPR2 [ i.Rt ].sq1 = r->GPR [ i.Fd ].sq1;
	VU0::_VU0->vf [ i.Rd ].sq0 = r->GPR [ i.Rt ].sq0;
	VU0::_VU0->vf [ i.Rd ].sq1 = r->GPR [ i.Rt ].sq1;
	
#if defined INLINE_DEBUG_QMTC2_NI || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output:" << " Rd/Fs=" << VU0::_VU0->vf [ i.Rd ].sq0 << " " << VU0::_VU0->vf [ i.Rd ].sq1;
#endif
}

void Execute::QMTC2_I ( Instruction::Format i )
{
#if defined INLINE_DEBUG_QMTC2_I || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 //	 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Rt=" << r->GPR [ i.Rt ].uw0 << " " << r->GPR [ i.Rt ].uw1 << " " << r->GPR [ i.Rt ].uw2 << " " << r->GPR [ i.Rt ].uw3;
#endif

	// QMTC2.I rt, fd
	// I -> interlocking. waits for previous VCALLMS to complete
	// ***todo*** implement interlocking

	//r->CPR2 [ i.Rt ].sq0 = r->GPR [ i.Rd ].sq0;
	//r->CPR2 [ i.Rt ].sq1 = r->GPR [ i.Rd ].sq1;
	
	// for now, just ignore the interlock
	// all it does is stop the write if m bit is set
	// needs to stop if either micro-subroutine is running or M-bit is set
	if ( VU0::_VU0->VifRegs.STAT.VEW || VU0::_VU0->CurInstHI.M )
	{
		// vu#0 is running //
		// or m-bit is set
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
	{
		VU0::_VU0->vf [ i.Rd ].sq0 = r->GPR [ i.Rt ].sq0;
		VU0::_VU0->vf [ i.Rd ].sq1 = r->GPR [ i.Rt ].sq1;
	}
	
#if defined INLINE_DEBUG_QMTC2_I || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output:" << " Rd/Fs=" << VU0::_VU0->vf [ i.Rd ].sq0 << " " << VU0::_VU0->vf [ i.Rd ].sq1;
#endif
}


void Execute::COP2 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_COP2 || defined INLINE_DEBUG_R5900 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	cout << "\nhps2x64: ALERT: 'COP2' Instruction ?? \n";

}




// VABS //

void Execute::VABS ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VABS || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ABS ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
	// does not affect any flags
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}

}


// VADD //

void Execute::VADD ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADD || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADD ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VADDi ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADDi || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADDi ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VADDq ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADDq || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADDq ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VADDBCX ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADDBCX || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADDBCX ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VADDBCY ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADDBCY || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADDBCY ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VADDBCZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADDBCZ || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADDBCZ ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VADDBCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADDBCW || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADDBCW ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}


// VADDA //

void Execute::VADDA ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADDA || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADDA ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VADDAi ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADDAi || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADDAi ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VADDAq ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADDAq || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADDAq ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VADDABCX ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADDABCX || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADDABCX ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VADDABCY ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADDABCY || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADDABCY ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VADDABCZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADDABCZ || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADDABCZ ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VADDABCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VADDABCW || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::ADDABCW ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}





// VSUB //

void Execute::VSUB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUB || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUB ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VSUBi ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUBi || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUBi ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VSUBq ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUBq || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUBq ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VSUBBCX ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUBBCX || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUBBCX ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VSUBBCY ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUBBCY || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUBBCY ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VSUBBCZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUBBCZ || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUBBCZ ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VSUBBCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUBBCW || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUBBCW ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}




// VMADD //

void Execute::VMADD ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADD || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADD ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMADDi ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADDi || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADDi ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMADDq ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADDq || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADDq ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMADDBCX ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADDBCX || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADDBCX ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMADDBCY ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADDBCY || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADDBCY ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMADDBCZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADDBCZ || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADDBCZ ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMADDBCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADDBCW || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADDBCW ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}




// VMSUB //

void Execute::VMSUB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUB || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUB ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMSUBi ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUBi || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUBi ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMSUBq ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUBq || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUBq ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMSUBBCX ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUBBCX || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUBBCX ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMSUBBCY ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUBBCY || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUBBCY ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMSUBBCZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUBBCZ || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUBBCZ ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMSUBBCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUBBCW || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUBBCW ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}




// VMAX //

void Execute::VMAX ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMAX || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MAX ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VMAXi ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMAXi || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MAXi ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VMAXBCX ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMAXBCX || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MAXBCX ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VMAXBCY ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMAXBCY || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MAXBCY ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VMAXBCZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMAXBCZ || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MAXBCZ ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VMAXBCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMAXBCW || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
	Vu::Instruction::Execute::MAXBCW ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}




// VMINI //

void Execute::VMINI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMINI || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MINI ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VMINIi ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMINi || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MINIi ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VMINIBCX ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMINIBCX || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MINIBCX ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VMINIBCY ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMINIBCY || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MINIBCY ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VMINIBCZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMINIBCZ || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MINIBCZ ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VMINIBCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMINIBCW || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MINIBCW ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}




// VMUL //

void Execute::VMUL ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMUL || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MUL ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMULi ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMULi || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MULi ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMULq ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMULq || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MULq ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMULBCX ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMULBCX || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MULBCX ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMULBCY ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMULBCY || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MULBCY ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMULBCZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMULBCZ || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MULBCZ ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMULBCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMULBCW || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MULBCW ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}






void Execute::VOPMSUB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VOPMSUB || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::OPMSUB ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VIADD ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VIADD || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::VIADD ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VISUB ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VISUB || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::VISUB ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VIADDI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VIADDI || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::VIADDI ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VIAND ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VIAND || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::VIAND ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VIOR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VIOR || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::VIOR ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}



// VCALLMS //

void Execute::VCALLMS ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VCALLMS || defined INLINE_DEBUG_VU0 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64 ALERT: R5900/VU0: *** VCALLMS *** unimplemented";
	
	if ( VU0::_VU0->Running || VU0::_VU0->VifRegs.STAT.VPS )
	{
		// a VU0 program is in progress already //
					
//#ifdef INLINE_DEBUG_VUCOM
//	debug << " (VIFSTOP)";
//#endif

#if defined INLINE_DEBUG_VCALLMS || defined INLINE_DEBUG_VU0 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\nR5900/VU0: VCALLMS while VU0 is already running!";
#endif

#ifdef VERBOSE_VCALLMS
		cout << "\nhps2x64 ALERT: R5900/VU0: VCALLMS while VU0 is already running!";
#endif

		// tell vu0 not to reset instr count since it is going to continue execution
		VU0::_VU0->bContinueVU0 = 1;

		// this is probably supposed to wait until vu0 is done and then execute
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// when vu is not running, cycle# is just -1
		if ( VU0::_VU0->Running )
		{

		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
		}
#endif

		return;
	}
					

	// set PC = IMM * 8
	VU0::_VU0->PC = i.Imm15 << 3;
	
	// need to set next pc too since this could get called in the middle of VU Main CPU loop
	VU0::_VU0->NextPC = VU0::_VU0->PC;
	
	// VU0 is now running
	VU0::_VU0->StartVU ();

#ifdef VERBOSE_MSCAL
	// debugging
	cout << "\nhps2x64: VU#0" << ": VCALLMS";
	cout << " StartPC=" << hex << VU0::_VU0->PC;
#endif

#ifdef INLINE_DEBUG_VUEXECUTE
	Vu::Instruction::Execute::debug << "\r\n*** VCALLMS";
	Vu::Instruction::Execute::debug << " VU#0";		// << Number;
	Vu::Instruction::Execute::debug << " StartPC=" << hex << VU0::_VU0->PC;
	//Vu::Instruction::Execute::debug << " VifCode=" << hex << VifCode.Value;
	Vu::Instruction::Execute::debug << " ***";
#endif

}

void Execute::VCALLMSR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VCALLMSR || defined INLINE_DEBUG_VU0 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64 ALERT: R5900/VU0: *** VCALLMSR *** unimplemented";
	
	if ( VU0::_VU0->Running || VU0::_VU0->VifRegs.STAT.VPS )
	{
		// a VU0 program is in progress already //
					
//#ifdef INLINE_DEBUG_VUCOM
//	debug << " (VIFSTOP)";
//#endif

#if defined INLINE_DEBUG_VCALLMS || defined INLINE_DEBUG_VU0 || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\nR5900/VU0: VCALLMS while VU0 is already running!";
#endif

#ifdef VERBOSE_VCALLMSR
		cout << "\nhps2x64 ALERT: R5900/VU0: VCALLMSR while VU0 is already running!";
#endif

		// tell vu0 not to reset instr count since it is going to continue execution
		VU0::_VU0->bContinueVU0 = 1;

		// this is probably supposed to wait until vu0 is done and then execute
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// when vu is not running, cycle# is just -1
		if ( VU0::_VU0->Running )
		{
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
		}
#endif

		return;
	}
					

	// set PC = IMM * 8
	VU0::_VU0->PC = VU0::_VU0->vi [ 27 ].uLo << 3;
	
	// need to set next pc too since this could get called in the middle of VU Main CPU loop
	VU0::_VU0->NextPC = VU0::_VU0->PC;
	
	// VU0 is now running
	VU0::_VU0->StartVU ();
	

#ifdef VERBOSE_MSCAL
	// debugging
	cout << "\nhps2x64: VU#0" << ": VCALLMSR";
	cout << " StartPC=" << hex << VU0::_VU0->PC;
#endif

#ifdef INLINE_DEBUG_VUEXECUTE
	Vu::Instruction::Execute::debug << "\r\n*** VCALLMSR";
	Vu::Instruction::Execute::debug << " VU#0";		// << Number;
	Vu::Instruction::Execute::debug << " StartPC=" << hex << VU0::_VU0->PC;
	//Vu::Instruction::Execute::debug << " VifCode=" << hex << VifCode.Value;
	Vu::Instruction::Execute::debug << " ***";
#endif
	
}


// VFTOI //

void Execute::VFTOI0 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VFTOI0 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::FTOI0 ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VFTOI4 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VFTOI4 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::FTOI4 ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VFTOI12 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VFTOI12 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::FTOI12 ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VFTOI15 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VFTOI15 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::FTOI15 ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}


// VITOF //

void Execute::VITOF0 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VITOF0 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::ITOF0 ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VITOF4 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VITOF4 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::ITOF4 ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VITOF12 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VITOF12 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::ITOF12 ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VITOF15 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VITOF15 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::ITOF15 ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}





void Execute::VMOVE ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMOVE || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::VMOVE ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VLQI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VLQI || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::VLQI ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VDIV ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VDIV || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::DIV ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
	// in macro mode, set Q immediately for now
	VU0::_VU0->SetQ ();
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMTIR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMTIR || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::VMTIR ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VRNEXT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VRNEXT || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::RNEXT ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VMR32 ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMR32 || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::VMR32 ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VSQI ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSQI || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::VSQI ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VSQRT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSQRT || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::SQRT ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
	// in macro mode, set Q immediately for now
	VU0::_VU0->SetQ ();
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMFIR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMFIR || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::MFIR ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VRGET ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VRGET || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::RGET ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}




// VSUBA //

void Execute::VSUBA ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUBA || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUBA ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VSUBAi ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUBAi || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUBAi ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VSUBAq ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUBAq || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUBAq ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VSUBABCX ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUBABCX || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUBABCX ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VSUBABCY ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUBABCY || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUBABCY ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VSUBABCZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUBABCZ || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUBABCZ ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VSUBABCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSUBABCW || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::SUBABCW ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}



// VMADDA //

void Execute::VMADDA ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADDA || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADDA ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMADDAi ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADDAi || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADDAi ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMADDAq ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADDAq || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADDAq ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMADDABCX ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADDABCX || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADDABCX ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMADDABCY ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADDABCY || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADDABCY ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMADDABCZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADDABCZ || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADDABCZ ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMADDABCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMADDABCW || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MADDABCW ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}




// VMSUBA //

void Execute::VMSUBA ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUBA || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUBA ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMSUBAi ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUBAi || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUBAi ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMSUBAq ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUBAq || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUBAq ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMSUBABCX ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUBABCX || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUBABCX ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMSUBABCY ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUBABCY || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUBABCY ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMSUBABCZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUBABCZ || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUBABCZ ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMSUBABCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMSUBABCW || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MSUBABCW ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}





// VMULA //

void Execute::VMULA ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMULA || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MULA ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMULAi ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMULAi || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MULAi ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMULAq ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMULAq || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MULAq ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMULABCX ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMULABCX || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MULABCX ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMULABCY ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMULABCY || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MULABCY ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMULABCZ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMULABCZ || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MULABCZ ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VMULABCW ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VMULABCW || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::MULABCW ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}





void Execute::VOPMULA ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VOPMULA || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::OPMULA ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VLQD ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VLQD || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::VLQD ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VRSQRT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VRSQRT || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::RSQRT ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
	// in macro mode, set Q immediately for now
	VU0::_VU0->SetQ ();
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VILWR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VILWR || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::ILWR ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VRINIT ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VRINIT || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::RINIT ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VCLIP ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VCLIP || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	Vu::Instruction::Execute::CLIP ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_NEW_CLIP_BUFFER
	// in macro mode, update the clip flag immediately
	VU0::_VU0->Get_CFBuffer_Force ( 3 );
#endif
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#else
	// update flags immediately for now
	VU0::_VU0->SetCurrentFlags ();
#endif
	}
}

void Execute::VNOP ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VNOP || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
		
#ifdef ENABLE_VU0_SKIP_WAIT
		// do a more passive wait
		if ( r->CycleCount < VU0::_VU0->CycleCount )
		{
			r->CycleCount = VU0::_VU0->CycleCount;
		}
#endif
	}
	else
#endif
	{
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VSQD ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VSQD || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::VSQD ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VWAITQ ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VWAITQ || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::WAITQ ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VISWR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VISWR || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::ISWR ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}

void Execute::VRXOR ( Instruction::Format i )
{
#if defined INLINE_DEBUG_VRXOR || defined INLINE_DEBUG_VU0 // || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << setw( 8 ) << r->PC << " " << dec << r->CycleCount << " " << Print::PrintInstruction ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_MACROMODE_CHECKVU0
	if ( VU0::_VU0->VifRegs.STAT.VEW )
	{
		// vu#0 is running //
		
		// don't go anywhere until it is done for now
		r->NextPC = r->PC;
	}
	else
#endif
	{
	i.Value = 0x80000000 | ( i.Value & 0x01ffffff );
	Vu::Instruction::Execute::RXOR ( VU0::_VU0, (Vu::Instruction::Format&) i );
	
#ifdef ENABLE_STALLS
	// update pipeline in macro mode ??
	VU0::_VU0->MacroMode_AdvanceCycle ( i.Value );
#endif
	}
}





const Execute::Function Execute::FunctionList []
{
	// instructions on both R3000A and R5900
	// 1 + 56 + 6 = 63 instructions //
	Execute::Invalid,
	Execute::J, Execute::JAL, Execute::JR, Execute::JALR, Execute::BEQ, Execute::BNE, Execute::BGTZ, Execute::BGEZ,
	Execute::BLTZ, Execute::BLEZ, Execute::BGEZAL, Execute::BLTZAL, Execute::ADD, Execute::ADDI, Execute::ADDU, Execute::ADDIU,
	Execute::SUB, Execute::SUBU, Execute::MULT, Execute::MULTU, Execute::DIV, Execute::DIVU, Execute::AND, Execute::ANDI,
	Execute::OR, Execute::ORI, Execute::XOR, Execute::XORI, Execute::NOR, Execute::LUI, Execute::SLL, Execute::SRL,
	Execute::SRA, Execute::SLLV, Execute::SRLV, Execute::SRAV, Execute::SLT, Execute::SLTI, Execute::SLTU, Execute::SLTIU,
	Execute::LB, Execute::LBU, Execute::LH, Execute::LHU, Execute::LW, Execute::LWL, Execute::LWR, Execute::SB,
	Execute::SH, Execute::SW, Execute::SWL, Execute::SWR, Execute::MFHI, Execute::MTHI, Execute::MFLO, Execute::MTLO,
	Execute::MFC0, Execute::MTC0,
	Execute::CFC2_I, Execute::CTC2_I, Execute::CFC2_NI, Execute::CTC2_NI,
	Execute::SYSCALL, Execute::BREAK,
	
	// instructions on R3000A ONLY
	//Execute::MFC2, Execute::MTC2, Execute::LWC2, Execute::SWC2, Execute::RFE,
	//Execute::RTPS, Execute::RTPT, Execute::CC, Execute::CDP, Execute::DCPL, Execute::DPCS, Execute::DPCT, Execute::NCS,
	//Execute::NCT, Execute::NCDS, Execute::NCDT, Execute::NCCS, Execute::NCCT, Execute::GPF, Execute::GPL, Execute::AVSZ3,
	//Execute::AVSZ4, Execute::SQR, Execute::OP, Execute::NCLIP, Execute::INTPL, Execute::MVMVA
	
	// instructions on R5900 ONLY
	// (24*8) + 4 + 6 = 192 + 10 = 202 instructions //
	Execute::BEQL, Execute::BNEL, Execute::BGEZL, Execute::BGTZL, Execute::BLEZL, Execute::BLTZL, Execute::BGEZALL, Execute::BLTZALL,
	Execute::DADD, Execute::DADDI, Execute::DADDU, Execute::DADDIU, Execute::DSUB, Execute::DSUBU, Execute::DSLL, Execute::DSLL32,
	Execute::DSLLV, Execute::DSRA, Execute::DSRA32, Execute::DSRAV, Execute::DSRL, Execute::DSRL32, Execute::DSRLV, Execute::LD,
	Execute::LDL, Execute::LDR, Execute::LWU, Execute::LQ, Execute::PREF, Execute::SD, Execute::SDL, Execute::SDR,
	Execute::SQ, Execute::TEQ, Execute::TEQI, Execute::TNE, Execute::TNEI, Execute::TGE, Execute::TGEI, Execute::TGEU,
	Execute::TGEIU, Execute::TLT, Execute::TLTI, Execute::TLTU, Execute::TLTIU, Execute::MOVN, Execute::MOVZ, Execute::MULT1,
	Execute::MULTU1, Execute::DIV1, Execute::DIVU1, Execute::MADD, Execute::MADD1, Execute::MADDU, Execute::MADDU1, Execute::MFHI1,
	Execute::MTHI1, Execute::MFLO1, Execute::MTLO1, Execute::MFSA, Execute::MTSA, Execute::MTSAB, Execute::MTSAH,
	Execute::PABSH, Execute::PABSW, Execute::PADDB, Execute::PADDH, Execute::PADDW, Execute::PADDSB, Execute::PADDSH, Execute::PADDSW,
	Execute::PADDUB, Execute::PADDUH, Execute::PADDUW, Execute::PADSBH, Execute::PAND, Execute::POR, Execute::PXOR, Execute::PNOR,
	Execute::PCEQB, Execute::PCEQH, Execute::PCEQW, Execute::PCGTB, Execute::PCGTH, Execute::PCGTW, Execute::PCPYH, Execute::PCPYLD,
	Execute::PCPYUD, Execute::PDIVBW, Execute::PDIVUW, Execute::PDIVW, Execute::PEXCH, Execute::PEXCW, Execute::PEXEH, Execute::PEXEW,
	Execute::PEXT5, Execute::PEXTLB, Execute::PEXTLH, Execute::PEXTLW, Execute::PEXTUB, Execute::PEXTUH, Execute::PEXTUW, Execute::PHMADH,
	Execute::PHMSBH, Execute::PINTEH, Execute::PINTH, Execute::PLZCW, Execute::PMADDH, Execute::PMADDW, Execute::PMADDUW, Execute::PMAXH,
	Execute::PMAXW, Execute::PMINH, Execute::PMINW, Execute::PMFHI, Execute::PMFLO, Execute::PMTHI, Execute::PMTLO, Execute::PMFHL_LH,
	Execute::PMFHL_SH, Execute::PMFHL_LW, Execute::PMFHL_UW, Execute::PMFHL_SLW, Execute::PMTHL_LW, Execute::PMSUBH, Execute::PMSUBW, Execute::PMULTH,
	Execute::PMULTW, Execute::PMULTUW, Execute::PPAC5, Execute::PPACB, Execute::PPACH, Execute::PPACW, Execute::PREVH, Execute::PROT3W,
	Execute::PSLLH, Execute::PSLLVW, Execute::PSLLW, Execute::PSRAH, Execute::PSRAW, Execute::PSRAVW, Execute::PSRLH, Execute::PSRLW,
	Execute::PSRLVW, Execute::PSUBB, Execute::PSUBH, Execute::PSUBW, Execute::PSUBSB, Execute::PSUBSH, Execute::PSUBSW, Execute::PSUBUB,
	Execute::PSUBUH, Execute::PSUBUW,
	Execute::QFSRV, Execute::SYNC,
	
	Execute::DI, Execute::EI, Execute::ERET, Execute::CACHE, Execute::TLBP, Execute::TLBR, Execute::TLBWI, Execute::TLBWR,
	Execute::CFC0, Execute::CTC0,
	
	Execute::BC0T, Execute::BC0TL, Execute::BC0F, Execute::BC0FL, Execute::BC1T, Execute::BC1TL, Execute::BC1F, Execute::BC1FL,
	Execute::BC2T, Execute::BC2TL, Execute::BC2F, Execute::BC2FL,
	
	Execute::LWC1, Execute::SWC1, Execute::MFC1, Execute::MTC1, Execute::CFC1, Execute::CTC1,
	Execute::ABS_S, Execute::ADD_S, Execute::ADDA_S, Execute::C_EQ_S, Execute::C_F_S, Execute::C_LE_S, Execute::C_LT_S, Execute::CVT_S_W,
	Execute::CVT_W_S, Execute::DIV_S, Execute::MADD_S, Execute::MADDA_S, Execute::MAX_S, Execute::MIN_S, Execute::MOV_S, Execute::MSUB_S,
	Execute::MSUBA_S, Execute::MUL_S, Execute::MULA_S, Execute::NEG_S, Execute::RSQRT_S, Execute::SQRT_S, Execute::SUB_S, Execute::SUBA_S,
	
	// VU macro mode instructions
	Execute::QMFC2_NI, Execute::QMFC2_I, Execute::QMTC2_NI, Execute::QMTC2_I, Execute::LQC2, Execute::SQC2,
	
	Execute::VABS,
	Execute::VADD, Execute::VADDi, Execute::VADDq, Execute::VADDBCX, Execute::VADDBCY, Execute::VADDBCZ, Execute::VADDBCW,
	Execute::VADDA, Execute::VADDAi, Execute::VADDAq, Execute::VADDABCX, Execute::VADDABCY, Execute::VADDABCZ, Execute::VADDABCW,
	Execute::VCALLMS, Execute::VCALLMSR, Execute::VCLIP, Execute::VDIV,
	Execute::VFTOI0, Execute::VFTOI4, Execute::VFTOI12, Execute::VFTOI15,
	Execute::VIADD, Execute::VIADDI, Execute::VIAND, Execute::VILWR, Execute::VIOR, Execute::VISUB, Execute::VISWR,
	Execute::VITOF0, Execute::VITOF4, Execute::VITOF12, Execute::VITOF15,
	Execute::VLQD, Execute::VLQI,
	
	Execute::VMADD, Execute::VMADDi, Execute::VMADDq, Execute::VMADDBCX, Execute::VMADDBCY, Execute::VMADDBCZ, Execute::VMADDBCW,
	Execute::VMADDA, Execute::VMADDAi, Execute::VMADDAq, Execute::VMADDABCX, Execute::VMADDABCY, Execute::VMADDABCZ, Execute::VMADDABCW,
	Execute::VMAX, Execute::VMAXi, Execute::VMAXBCX, Execute::VMAXBCY, Execute::VMAXBCZ, Execute::VMAXBCW,
	Execute::VMFIR,
	Execute::VMINI, Execute::VMINIi, Execute::VMINIBCX, Execute::VMINIBCY, Execute::VMINIBCZ, Execute::VMINIBCW,
	Execute::VMOVE, Execute::VMR32,
	
	Execute::VMSUB, Execute::VMSUBi, Execute::VMSUBq, Execute::VMSUBBCX, Execute::VMSUBBCY, Execute::VMSUBBCZ, Execute::VMSUBBCW,
	Execute::VMSUBA, Execute::VMSUBAi, Execute::VMSUBAq, Execute::VMSUBABCX, Execute::VMSUBABCY, Execute::VMSUBABCZ, Execute::VMSUBABCW,
	Execute::VMTIR,
	Execute::VMUL, Execute::VMULi, Execute::VMULq, Execute::VMULBCX, Execute::VMULBCY, Execute::VMULBCZ, Execute::VMULBCW,
	Execute::VMULA, Execute::VMULAi, Execute::VMULAq, Execute::VMULABCX, Execute::VMULABCY, Execute::VMULABCZ, Execute::VMULABCW,
	Execute::VNOP, Execute::VOPMSUB, Execute::VOPMULA, Execute::VRGET, Execute::VRINIT, Execute::VRNEXT, Execute::VRSQRT, Execute::VRXOR,
	Execute::VSQD, Execute::VSQI, Execute::VSQRT,
	Execute::VSUB, Execute::VSUBi, Execute::VSUBq, Execute::VSUBBCX, Execute::VSUBBCY, Execute::VSUBBCZ, Execute::VSUBBCW,
	Execute::VSUBA, Execute::VSUBAi, Execute::VSUBAq, Execute::VSUBABCX, Execute::VSUBABCY, Execute::VSUBABCZ, Execute::VSUBABCW,
	Execute::VWAITQ,
	Execute::COP2
};





#ifdef _DEBUG_VERSION_
Debug::Log Execute::debug;
#endif





// generates the lookup table
Execute::Execute ( Cpu* pCpu )
{
	r = pCpu;
}



}

}



