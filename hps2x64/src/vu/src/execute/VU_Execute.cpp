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


#include "VU_Execute.h"
#include "VU_Print.h"
#include "PS2Float.h"
#include "PS2_GPU.h"

//#include <cmath>
#include <stdlib.h>

using namespace std;
using namespace Vu::Instruction;
using namespace PS2Float;


#ifdef _DEBUG_VERSION_
Debug::Log Execute::debug;
#endif


//#define ENABLE_TEST_DEBUG_FTOI4



//#define ENABLE_XGKICK_WAIT




// will need this for accurate operation, and cycle accuracy is required

#define ENABLE_STALLS
#define ENABLE_STALLS_INT

// these are for if not doing execution reorder
/*
#define ENABLE_STALLS_MOVE
#define ENABLE_STALLS_MR32

#define ENABLE_STALLS_LQ
#define ENABLE_STALLS_LQD
#define ENABLE_STALLS_LQI
#define ENABLE_STALLS_RGET
#define ENABLE_STALLS_RNEXT
#define ENABLE_STALLS_MFIR
#define ENABLE_STALLS_MFP
*/

/*
#define ENABLE_INTDELAYSLOT

#define ENABLE_INTDELAYSLOT_CALC


#define ENABLE_INTDELAYSLOT_ILW_BEFORE
#define ENABLE_INTDELAYSLOT_ILWR_BEFORE

#define ENABLE_INTDELAYSLOT_ISW
#define ENABLE_INTDELAYSLOT_ISWR


#define ENABLE_INTDELAYSLOT_LQD_BEFORE
#define ENABLE_INTDELAYSLOT_LQI_BEFORE
#define ENABLE_INTDELAYSLOT_SQD_BEFORE
#define ENABLE_INTDELAYSLOT_SQI_BEFORE
*/

//#define ENABLE_INTDELAYSLOT_LQD_AFTER
//#define ENABLE_INTDELAYSLOT_LQI_AFTER
//#define ENABLE_INTDELAYSLOT_SQD_AFTER
//#define ENABLE_INTDELAYSLOT_SQI_AFTER




#define PERFORM_CLIP_AS_INTEGER



#define ENABLE_SNAPSHOTS


//#define USE_NEW_RECOMPILE2

#define USE_NEW_RECOMPILE2_INTCALC

#define USE_NEW_RECOMPILE2_MOVE
#define USE_NEW_RECOMPILE2_MR32

#define USE_NEW_RECOMPILE2_LQI
#define USE_NEW_RECOMPILE2_LQD
#define USE_NEW_RECOMPILE2_SQI
#define USE_NEW_RECOMPILE2_SQD



// enable debugging

#ifdef _DEBUG_VERSION_

#define INLINE_DEBUG_ENABLE

//#define INLINE_DEBUG_SPLIT


//#define INLINE_DEBUG_STALLS

//#define INLINE_DEBUG_XGKICK
#define INLINE_DEBUG_INVALID

//#define INLINE_DEBUG_ADD
//#define INLINE_DEBUG_IAND

#define INLINE_DEBUG_SUB
#define INLINE_DEBUG_SUBI
#define INLINE_DEBUG_SUBQ
#define INLINE_DEBUG_SUBBCX
#define INLINE_DEBUG_SUBBCY
#define INLINE_DEBUG_SUBBCW
#define INLINE_DEBUG_SUBBCZ
#define INLINE_DEBUG_SUBA
#define INLINE_DEBUG_SUBAI
#define INLINE_DEBUG_SUBAQ
#define INLINE_DEBUG_SUBAX
#define INLINE_DEBUG_SUBAY
#define INLINE_DEBUG_SUBAW
#define INLINE_DEBUG_SUBAZ


/*
#define INLINE_DEBUG_ADD
#define INLINE_DEBUG_ADDI
#define INLINE_DEBUG_ADDQ
#define INLINE_DEBUG_ADDBCX
#define INLINE_DEBUG_ADDBCY
#define INLINE_DEBUG_ADDBCW
#define INLINE_DEBUG_ADDBCZ
#define INLINE_DEBUG_ADDA
#define INLINE_DEBUG_ADDAI
#define INLINE_DEBUG_ADDAQ
#define INLINE_DEBUG_ADDAX
#define INLINE_DEBUG_ADDAY
#define INLINE_DEBUG_ADDAW
#define INLINE_DEBUG_ADDAZ
*/

/*
#define INLINE_DEBUG_MADD
#define INLINE_DEBUG_MADDI
#define INLINE_DEBUG_MADDQ
#define INLINE_DEBUG_MADDX
#define INLINE_DEBUG_MADDY
#define INLINE_DEBUG_MADDZ
#define INLINE_DEBUG_MADDW

#define INLINE_DEBUG_MADDA
#define INLINE_DEBUG_MADDAI
#define INLINE_DEBUG_MADDAQ
#define INLINE_DEBUG_MADDAX
#define INLINE_DEBUG_MADDAY
#define INLINE_DEBUG_MADDAZ
#define INLINE_DEBUG_MADDAW
*/

//#define INLINE_DEBUG_MFIR
//#define INLINE_DEBUG_MTIR

//#define INLINE_DEBUG_RINIT
//#define INLINE_DEBUG_RGET
//#define INLINE_DEBUG_RNEXT
//#define INLINE_DEBUG_LD

//#define INLINE_DEBUG_VU
//#define INLINE_DEBUG_UNIMPLEMENTED
//#define INLINE_DEBUG_EXT


#endif




const char Execute::XyzwLUT [ 4 ] = { 'x', 'y', 'z', 'w' };
const char* Execute::BCType [ 4 ] = { "F", "T", "FL", "TL" };






void Execute::Start ()
{
	// make sure the lookup object has started (note: this can take a LONG time for R5900 currently)
	Lookup::Start ();
	
#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create ( "PS2_VU_Execute_Log.txt" );
#endif
}



void Execute::INVALID ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_INVALID || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << "INVALID" << "; " << hex << i.Value;
	debug << " vfx=" << hex << v->vf [ i.Fs ].fx << " vfy=" << v->vf [ i.Fs ].fy << " vfz=" << v->vf [ i.Fs ].fz << " vfw=" << v->vf [ i.Fs ].fw;
#endif

	cout << "\nhps2x64: ERROR: VU: Invalid instruction encountered. VU#" << v->Number << " PC=" << hex << v->PC << " Code=" << i.Value << " Cycle#" << dec << v->CycleCount;
}



//// *** UPPER instructions *** ////


// ABS //

void Execute::ABS ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ABS || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " vfx=" << hex << v->vf [ i.Fs ].fx << " vfy=" << v->vf [ i.Fs ].fy << " vfz=" << v->vf [ i.Fs ].fz << " vfw=" << v->vf [ i.Fs ].fw;
#endif

#ifdef ENABLE_STALLS

	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Ft );

#endif

	if ( i.destx )
	{
		v->vf [ i.Ft ].uw0 = v->vf [ i.Fs ].uw0 & ~0x80000000;
	}
	
	if ( i.desty )
	{
		v->vf [ i.Ft ].uw1 = v->vf [ i.Fs ].uw1 & ~0x80000000;
	}
	
	if ( i.destz )
	{
		v->vf [ i.Ft ].uw2 = v->vf [ i.Fs ].uw2 & ~0x80000000;
	}
	
	if ( i.destw )
	{
		v->vf [ i.Ft ].uw3 = v->vf [ i.Fs ].uw3 & ~0x80000000;
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Ft;
	
	// flags affected: none

#if defined INLINE_DEBUG_ABS || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << hex << v->vf [ i.Ft ].fx << " vfy=" << v->vf [ i.Ft ].fy << " vfz=" << v->vf [ i.Ft ].fz << " vfw=" << v->vf [ i.Ft ].fw;
#endif
}





// ADD //

void Execute::ADD ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	// fd = fs + ft
	VuUpperOp ( v, i, PS2_Float_Add );
	
	// flags affected: zero, sign, [ overflow, underflow ]

#if defined INLINE_DEBUG_ADD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::ADDi ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " I=" << hex << v->vi [ 21 ].f << " " << v->vi [ 21 ].u;
#endif

	// fd = fs + I
	
	VuUpperOpI ( v, i, PS2_Float_Add );
	
	// flags affected: zero, sign, [ overflow, underflow ]

#if defined INLINE_DEBUG_ADDI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::ADDq ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Q=" << hex << v->vi [ 22 ].f << " " << v->vi [ 22 ].u;
#endif

	// fd = fs + Q
	
	VuUpperOpQ ( v, i, PS2_Float_Add );
	
	// flags affected: zero, sign, [ overflow, underflow ]

#if defined INLINE_DEBUG_ADDQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::ADDBCX ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDBCX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	// fd = fs + ftbc
	
	VuUpperOpX ( v, i, PS2_Float_Add );

	// flags affected: zero, sign, [ overflow, underflow ]

#if defined INLINE_DEBUG_ADDBCX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::ADDBCY ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDBCY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	// fd = fs + ftbc
	
	VuUpperOpY ( v, i, PS2_Float_Add );
	
	// flags affected: zero, sign, [ overflow, underflow ]

#if defined INLINE_DEBUG_ADDBCY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::ADDBCZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDBCZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	// fd = fs + ftbc
	
	VuUpperOpZ ( v, i, PS2_Float_Add );
	
	// flags affected: zero, sign, [ overflow, underflow ]

#if defined INLINE_DEBUG_ADDBCZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::ADDBCW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDBCW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	// fd = fs + ftbc
	VuUpperOpW ( v, i, PS2_Float_Add );
	
	// flags affected: zero, sign, [ overflow, underflow ]

#if defined INLINE_DEBUG_ADDBCW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}




// SUB //

void Execute::SUB ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUB || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	// fd = fs - ft
	VuUpperOp ( v, i, PS2_Float_Sub );
	
	// flags affected: zero, sign, [ overflow, underflow ]

#if defined INLINE_DEBUG_SUB || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::SUBi ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " I=" << hex << v->vi [ 21 ].f << " " << v->vi [ 21 ].u;
#endif

	VuUpperOpI ( v, i, PS2_Float_Sub );
	
#if defined INLINE_DEBUG_SUBI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::SUBq ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Q=" << hex << v->vi [ 22 ].f << " " << v->vi [ 22 ].u;
#endif

	VuUpperOpQ ( v, i, PS2_Float_Sub );
	
#if defined INLINE_DEBUG_SUBQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::SUBBCX ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpX ( v, i, PS2_Float_Sub );
	
#if defined INLINE_DEBUG_SUBX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::SUBBCY ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpY ( v, i, PS2_Float_Sub );
	
#if defined INLINE_DEBUG_SUBY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::SUBBCZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpZ ( v, i, PS2_Float_Sub );
	
#if defined INLINE_DEBUG_SUBZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::SUBBCW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpW ( v, i, PS2_Float_Sub );
	
#if defined INLINE_DEBUG_SUBW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}




// MADD //

void Execute::MADD ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOp ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MADDi ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " I=" << hex << v->vi [ 21 ].f << " " << v->vi [ 21 ].u;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpI ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADDI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MADDq ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Q=" << hex << v->vi [ 22 ].f << " " << v->vi [ 22 ].u;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpQ ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADDQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MADDBCX ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpX ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADDX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MADDBCY ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpY ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADDY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << std::fixed << std::setprecision(0) << v->vi [ VU::REG_STATUSFLAG ].uLo << " STATF=" << v->vi [ VU::REG_MACFLAG ].uLo;
#endif
}

void Execute::MADDBCZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	//debug << " Fs(hex)= x=" << hex << v->vf [ i.Fs ].ux << " y=" << v->vf [ i.Fs ].uy << " z=" << v->vf [ i.Fs ].uz << " w=" << v->vf [ i.Fs ].uw;
	//debug << " Ft(hex)= x=" << hex << v->vf [ i.Ft ].ux << " y=" << v->vf [ i.Ft ].uy << " z=" << v->vf [ i.Ft ].uz << " w=" << v->vf [ i.Ft ].uw;
	//debug << " ACC(hex)= x=" << v->dACC [ 0 ].l << " y=" << v->dACC [ 1 ].l << " z=" << v->dACC [ 2 ].l << " w=" << v->dACC [ 3 ].l;
#endif

	VuUpperOpZ ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADDZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	//debug << " Output(hex): Fd=" << " vfx=" << hex << v->vf [ i.Fd ].ux << " vfy=" << v->vf [ i.Fd ].uy << " vfz=" << v->vf [ i.Fd ].uz << " vfw=" << v->vf [ i.Fd ].uw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MADDBCW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpW ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADDW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}



// MSUB //

void Execute::MSUB ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUB || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOp ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUB || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MSUBi ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " I=" << hex << v->vi [ 21 ].f << " " << v->vi [ 21 ].u;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpI ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUBI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MSUBq ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Q=" << hex << v->vi [ 22 ].f << " " << v->vi [ 22 ].u;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpQ ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUBQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MSUBBCX ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpX ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUBX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MSUBBCY ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpY ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUBY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MSUBBCZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpZ ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUBZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MSUBBCW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpW ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUBW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}



// MUL //

void Execute::MUL ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MUL || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOp ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MUL || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MULi ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MULI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " I=" << hex << v->vi [ 21 ].f << " " << v->vi [ 21 ].u;
#endif

	VuUpperOpI ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MULI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MULq ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MULQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Q=" << hex << v->vi [ 22 ].f << " " << v->vi [ 22 ].u;
#endif

	VuUpperOpQ ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MULQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MULBCX ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MULX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpX ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MULX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MULBCY ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MULY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpY ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MULY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MULBCZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MULZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpZ ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MULZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MULBCW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MULW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpW ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MULW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}



// MAX //

void Execute::MAX ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MAX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
	

	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

	if ( i.destx )
	{
		v->vf [ i.Fd ].fx = PS2_Float_Max ( v->vf [ i.Fs ].fx, v->vf [ i.Ft ].fx );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Fd ].fy = PS2_Float_Max ( v->vf [ i.Fs ].fy, v->vf [ i.Ft ].fy );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Fd ].fz = PS2_Float_Max ( v->vf [ i.Fs ].fz, v->vf [ i.Ft ].fz );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Fd ].fw = PS2_Float_Max ( v->vf [ i.Fs ].fw, v->vf [ i.Ft ].fw );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Fd;
	
#if defined INLINE_DEBUG_MAX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
#endif
}

void Execute::MAXi ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MAXI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " I=" << hex << v->vi [ 21 ].f << " " << v->vi [ 21 ].u;
#endif

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

	if ( i.destx )
	{
		v->vf [ i.Fd ].fx = PS2_Float_Max ( v->vf [ i.Fs ].fx, v->vi [ 21 ].f );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Fd ].fy = PS2_Float_Max ( v->vf [ i.Fs ].fy, v->vi [ 21 ].f );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Fd ].fz = PS2_Float_Max ( v->vf [ i.Fs ].fz, v->vi [ 21 ].f );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Fd ].fw = PS2_Float_Max ( v->vf [ i.Fs ].fw, v->vi [ 21 ].f );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Fd;
	
#if defined INLINE_DEBUG_MAXI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
#endif
}

void Execute::MAXBCX ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MAXX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	float fx;

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

	// don't want to overwrite and propagate...
	fx = v->vf [ i.Ft ].fx;

	if ( i.destx )
	{
		v->vf [ i.Fd ].fx = PS2_Float_Max ( v->vf [ i.Fs ].fx, fx );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Fd ].fy = PS2_Float_Max ( v->vf [ i.Fs ].fy, fx );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Fd ].fz = PS2_Float_Max ( v->vf [ i.Fs ].fz, fx );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Fd ].fw = PS2_Float_Max ( v->vf [ i.Fs ].fw, fx );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Fd;
	
#if defined INLINE_DEBUG_MAXX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
#endif
}

void Execute::MAXBCY ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MAXY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	float fy;

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

	fy = v->vf [ i.Ft ].fy;

	if ( i.destx )
	{
		v->vf [ i.Fd ].fx = PS2_Float_Max ( v->vf [ i.Fs ].fx, fy );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Fd ].fy = PS2_Float_Max ( v->vf [ i.Fs ].fy, fy );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Fd ].fz = PS2_Float_Max ( v->vf [ i.Fs ].fz, fy );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Fd ].fw = PS2_Float_Max ( v->vf [ i.Fs ].fw, fy );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Fd;
	
#if defined INLINE_DEBUG_MAXY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
#endif
}

void Execute::MAXBCZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MAXZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	float fz;

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

	fz = v->vf [ i.Ft ].fz;

	if ( i.destx )
	{
		v->vf [ i.Fd ].fx = PS2_Float_Max ( v->vf [ i.Fs ].fx, fz );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Fd ].fy = PS2_Float_Max ( v->vf [ i.Fs ].fy, fz );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Fd ].fz = PS2_Float_Max ( v->vf [ i.Fs ].fz, fz );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Fd ].fw = PS2_Float_Max ( v->vf [ i.Fs ].fw, fz );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Fd;
	
#if defined INLINE_DEBUG_MAXZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
#endif
}

void Execute::MAXBCW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MAXW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	float fw;

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

	fw = v->vf [ i.Ft ].fw;

	if ( i.destx )
	{
		v->vf [ i.Fd ].fx = PS2_Float_Max ( v->vf [ i.Fs ].fx, fw );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Fd ].fy = PS2_Float_Max ( v->vf [ i.Fs ].fy, fw );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Fd ].fz = PS2_Float_Max ( v->vf [ i.Fs ].fz, fw );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Fd ].fw = PS2_Float_Max ( v->vf [ i.Fs ].fw, fw );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Fd;
	
#if defined INLINE_DEBUG_MAXW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
#endif
}



// MINI //

void Execute::MINI ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MINI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

	if ( i.destx )
	{
		v->vf [ i.Fd ].fx = PS2_Float_Min ( v->vf [ i.Fs ].fx, v->vf [ i.Ft ].fx );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Fd ].fy = PS2_Float_Min ( v->vf [ i.Fs ].fy, v->vf [ i.Ft ].fy );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Fd ].fz = PS2_Float_Min ( v->vf [ i.Fs ].fz, v->vf [ i.Ft ].fz );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Fd ].fw = PS2_Float_Min ( v->vf [ i.Fs ].fw, v->vf [ i.Ft ].fw );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Fd;
	
#if defined INLINE_DEBUG_MINI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
#endif
}

void Execute::MINIi ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MINII || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " I=" << hex << v->vi [ 21 ].f << " " << v->vi [ 21 ].u;
#endif

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

	if ( i.destx )
	{
		v->vf [ i.Fd ].fx = PS2_Float_Min ( v->vf [ i.Fs ].fx, v->vi [ 21 ].f );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Fd ].fy = PS2_Float_Min ( v->vf [ i.Fs ].fy, v->vi [ 21 ].f );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Fd ].fz = PS2_Float_Min ( v->vf [ i.Fs ].fz, v->vi [ 21 ].f );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Fd ].fw = PS2_Float_Min ( v->vf [ i.Fs ].fw, v->vi [ 21 ].f );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Fd;
	
#if defined INLINE_DEBUG_MINII || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
#endif
}

void Execute::MINIBCX ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MINIX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	float fx;

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

	fx = v->vf [ i.Ft ].fx;

	if ( i.destx )
	{
		v->vf [ i.Fd ].fx = PS2_Float_Min ( v->vf [ i.Fs ].fx, fx );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Fd ].fy = PS2_Float_Min ( v->vf [ i.Fs ].fy, fx );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Fd ].fz = PS2_Float_Min ( v->vf [ i.Fs ].fz, fx );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Fd ].fw = PS2_Float_Min ( v->vf [ i.Fs ].fw, fx );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Fd;
	
#if defined INLINE_DEBUG_MINIX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
#endif
}

void Execute::MINIBCY ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MINIY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	float fy;

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

	fy = v->vf [ i.Ft ].fy;

	if ( i.destx )
	{
		v->vf [ i.Fd ].fx = PS2_Float_Min ( v->vf [ i.Fs ].fx, fy );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Fd ].fy = PS2_Float_Min ( v->vf [ i.Fs ].fy, fy );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Fd ].fz = PS2_Float_Min ( v->vf [ i.Fs ].fz, fy );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Fd ].fw = PS2_Float_Min ( v->vf [ i.Fs ].fw, fy );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Fd;
	
#if defined INLINE_DEBUG_MINIY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
#endif
}

void Execute::MINIBCZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MINIZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	float fz;

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

	fz = v->vf [ i.Ft ].fz;

	if ( i.destx )
	{
		v->vf [ i.Fd ].fx = PS2_Float_Min ( v->vf [ i.Fs ].fx, fz );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Fd ].fy = PS2_Float_Min ( v->vf [ i.Fs ].fy, fz );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Fd ].fz = PS2_Float_Min ( v->vf [ i.Fs ].fz, fz );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Fd ].fw = PS2_Float_Min ( v->vf [ i.Fs ].fw, fz );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Fd;
	
#if defined INLINE_DEBUG_MINIZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
#endif
}

void Execute::MINIBCW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MINIW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	float fw;

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

	fw = v->vf [ i.Ft ].fw;

	if ( i.destx )
	{
		v->vf [ i.Fd ].fx = PS2_Float_Min ( v->vf [ i.Fs ].fx, fw );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Fd ].fy = PS2_Float_Min ( v->vf [ i.Fs ].fy, fw );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Fd ].fz = PS2_Float_Min ( v->vf [ i.Fs ].fz, fw );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Fd ].fw = PS2_Float_Min ( v->vf [ i.Fs ].fw, fw );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Fd;
	
#if defined INLINE_DEBUG_MINIW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
#endif
}








// ITOF //

void Execute::ITOF0 ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ITOF0 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " vfx=" << dec << v->vf [ i.Fs ].sx << " vfy=" << v->vf [ i.Fs ].sy << " vfz=" << v->vf [ i.Fs ].sz << " vfw=" << v->vf [ i.Fs ].sw;
#endif

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif

	if ( i.destx )
	{
		v->vf [ i.Ft ].fx = (float) v->vf [ i.Fs ].sw0;
	}
	
	if ( i.desty )
	{
		v->vf [ i.Ft ].fy = (float) v->vf [ i.Fs ].sw1;
	}
	
	if ( i.destz )
	{
		v->vf [ i.Ft ].fz = (float) v->vf [ i.Fs ].sw2;
	}
	
	if ( i.destw )
	{
		v->vf [ i.Ft ].fw = (float) v->vf [ i.Fs ].sw3;
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Ft;
	
	// flags affected: none

#if defined INLINE_DEBUG_ITOF0 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << dec << v->vf [ i.Ft ].fx << " vfy=" << v->vf [ i.Ft ].fy << " vfz=" << v->vf [ i.Ft ].fz << " vfw=" << v->vf [ i.Ft ].fw;
#endif
}

void Execute::FTOI0 ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FTOI0 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " vfx=" << dec << v->vf [ i.Fs ].fx << " vfy=" << v->vf [ i.Fs ].fy << " vfz=" << v->vf [ i.Fs ].fz << " vfw=" << v->vf [ i.Fs ].fw;
#endif

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif

	if ( i.destx )
	{
		v->vf [ i.Ft ].sw0 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fx );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Ft ].sw1 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fy );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Ft ].sw2 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fz );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Ft ].sw3 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fw );
	}
	
	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Ft;
	
	// flags affected: none

#if defined INLINE_DEBUG_FTOI0 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << dec << v->vf [ i.Ft ].sx << " vfy=" << v->vf [ i.Ft ].sy << " vfz=" << v->vf [ i.Ft ].sz << " vfw=" << v->vf [ i.Ft ].sw;
#endif
}

void Execute::ITOF4 ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ITOF4 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " vfx=" << dec << v->vf [ i.Fs ].sx << " vfy=" << v->vf [ i.Fs ].sy << " vfz=" << v->vf [ i.Fs ].sz << " vfw=" << v->vf [ i.Fs ].sw;
#endif

	static const float c_fMultiplier = 1.0f / 16.0f;

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif

	if ( i.destx )
	{
		v->vf [ i.Ft ].fx = ( (float) v->vf [ i.Fs ].sw0 ) * c_fMultiplier;
	}
	
	if ( i.desty )
	{
		v->vf [ i.Ft ].fy = ( (float) v->vf [ i.Fs ].sw1 ) * c_fMultiplier;
	}
	
	if ( i.destz )
	{
		v->vf [ i.Ft ].fz = ( (float) v->vf [ i.Fs ].sw2 ) * c_fMultiplier;
	}
	
	if ( i.destw )
	{
		v->vf [ i.Ft ].fw = ( (float) v->vf [ i.Fs ].sw3 ) * c_fMultiplier;
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Ft;
	
	// flags affected: none
	
#if defined INLINE_DEBUG_ITOF4 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << dec << v->vf [ i.Ft ].fx << " vfy=" << v->vf [ i.Ft ].fy << " vfz=" << v->vf [ i.Ft ].fz << " vfw=" << v->vf [ i.Ft ].fw;
#endif
}

void Execute::FTOI4 ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FTOI4 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " vfx=" << dec << v->vf [ i.Fs ].fx << " vfy=" << v->vf [ i.Fs ].fy << " vfz=" << v->vf [ i.Fs ].fz << " vfw=" << v->vf [ i.Fs ].fw;
#endif

	static const float c_fMultiplier = 16.0f;
	
#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif


	if ( i.destx )
	{
		v->vf [ i.Ft ].sw0 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fx * c_fMultiplier );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Ft ].sw1 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fy * c_fMultiplier );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Ft ].sw2 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fz * c_fMultiplier );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Ft ].sw3 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fw * c_fMultiplier );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Ft;

	// flags affected: none

#if defined INLINE_DEBUG_FTOI4 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << dec << v->vf [ i.Ft ].sx << " vfy=" << v->vf [ i.Ft ].sy << " vfz=" << v->vf [ i.Ft ].sz << " vfw=" << v->vf [ i.Ft ].sw;
#endif
}

void Execute::ITOF12 ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ITOF12 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " vfx=" << dec << v->vf [ i.Fs ].sx << " vfy=" << v->vf [ i.Fs ].sy << " vfz=" << v->vf [ i.Fs ].sz << " vfw=" << v->vf [ i.Fs ].sw;
#endif

	static const float c_fMultiplier = 1.0f / 4096.0f;

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif

	if ( i.destx )
	{
		v->vf [ i.Ft ].fx = ( (float) v->vf [ i.Fs ].sw0 ) * c_fMultiplier;
	}
	
	if ( i.desty )
	{
		v->vf [ i.Ft ].fy = ( (float) v->vf [ i.Fs ].sw1 ) * c_fMultiplier;
	}
	
	if ( i.destz )
	{
		v->vf [ i.Ft ].fz = ( (float) v->vf [ i.Fs ].sw2 ) * c_fMultiplier;
	}
	
	if ( i.destw )
	{
		v->vf [ i.Ft ].fw = ( (float) v->vf [ i.Fs ].sw3 ) * c_fMultiplier;
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Ft;
	
	// flags affected: none
	
#if defined INLINE_DEBUG_ITOF12 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << dec << v->vf [ i.Ft ].fx << " vfy=" << v->vf [ i.Ft ].fy << " vfz=" << v->vf [ i.Ft ].fz << " vfw=" << v->vf [ i.Ft ].fw;
#endif
}

void Execute::FTOI12 ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FTOI12 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " vfx=" << dec << v->vf [ i.Fs ].fx << " vfy=" << v->vf [ i.Fs ].fy << " vfz=" << v->vf [ i.Fs ].fz << " vfw=" << v->vf [ i.Fs ].fw;
#endif

	static const float c_fMultiplier = 4096.0f;
	
#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif

	if ( i.destx )
	{
		v->vf [ i.Ft ].sw0 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fx * c_fMultiplier );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Ft ].sw1 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fy * c_fMultiplier );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Ft ].sw2 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fz * c_fMultiplier );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Ft ].sw3 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fw * c_fMultiplier );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Ft;
	
	// flags affected: none

#if defined INLINE_DEBUG_FTOI12 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << dec << v->vf [ i.Ft ].sx << " vfy=" << v->vf [ i.Ft ].sy << " vfz=" << v->vf [ i.Ft ].sz << " vfw=" << v->vf [ i.Ft ].sw;
#endif
}

void Execute::ITOF15 ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ITOF15 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " vfx=" << dec << v->vf [ i.Fs ].sx << " vfy=" << v->vf [ i.Fs ].sy << " vfz=" << v->vf [ i.Fs ].sz << " vfw=" << v->vf [ i.Fs ].sw;
#endif

	static const float c_fMultiplier = 1.0f / 32768.0f;

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif

	if ( i.destx )
	{
		v->vf [ i.Ft ].fx = ( (float) v->vf [ i.Fs ].sw0 ) * c_fMultiplier;
	}
	
	if ( i.desty )
	{
		v->vf [ i.Ft ].fy = ( (float) v->vf [ i.Fs ].sw1 ) * c_fMultiplier;
	}
	
	if ( i.destz )
	{
		v->vf [ i.Ft ].fz = ( (float) v->vf [ i.Fs ].sw2 ) * c_fMultiplier;
	}
	
	if ( i.destw )
	{
		v->vf [ i.Ft ].fw = ( (float) v->vf [ i.Fs ].sw3 ) * c_fMultiplier;
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Ft;
	
	// flags affected: none
	
#if defined INLINE_DEBUG_ITOF15 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << dec << v->vf [ i.Ft ].fx << " vfy=" << v->vf [ i.Ft ].fy << " vfz=" << v->vf [ i.Ft ].fz << " vfw=" << v->vf [ i.Ft ].fw;
#endif
}

void Execute::FTOI15 ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FTOI15 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " vfx=" << dec << v->vf [ i.Fs ].fx << " vfy=" << v->vf [ i.Fs ].fy << " vfz=" << v->vf [ i.Fs ].fz << " vfw=" << v->vf [ i.Fs ].fw;
#endif

	static const float c_fMultiplier = 32768.0f;
	
#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif

	if ( i.destx )
	{
		v->vf [ i.Ft ].sw0 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fx * c_fMultiplier );
	}
	
	if ( i.desty )
	{
		v->vf [ i.Ft ].sw1 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fy * c_fMultiplier );
	}
	
	if ( i.destz )
	{
		v->vf [ i.Ft ].sw2 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fz * c_fMultiplier );
	}
	
	if ( i.destw )
	{
		v->vf [ i.Ft ].sw3 = PS2_Float_ToInteger ( v->vf [ i.Fs ].fw * c_fMultiplier );
	}

	// the accompanying lower instruction can't modify the same register
	v->LastModifiedRegister = i.Ft;
	
	// flags affected: none

#if defined INLINE_DEBUG_FTOI15 || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << dec << v->vf [ i.Ft ].sx << " vfy=" << v->vf [ i.Ft ].sy << " vfz=" << v->vf [ i.Ft ].sz << " vfw=" << v->vf [ i.Ft ].sw;
#endif
}





// ADDA //

void Execute::ADDA ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDA || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	// fd = fs + ft
	VuUpperOp_A ( v, i, PS2_Float_Add );
	
#if defined INLINE_DEBUG_ADDA || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::ADDAi ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDAI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " I=" << hex << v->vi [ 21 ].f << " " << v->vi [ 21 ].u;
#endif

	VuUpperOpI_A ( v, i, PS2_Float_Add );
	
#if defined INLINE_DEBUG_ADDAI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::ADDAq ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDAQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Q=" << hex << v->vi [ 22 ].f << " " << v->vi [ 22 ].u;
#endif

	VuUpperOpQ_A ( v, i, PS2_Float_Add );
	
#if defined INLINE_DEBUG_ADDAQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::ADDABCX ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDAX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpX_A ( v, i, PS2_Float_Add );
	
#if defined INLINE_DEBUG_ADDAX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::ADDABCY ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDAY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpY_A ( v, i, PS2_Float_Add );
	
#if defined INLINE_DEBUG_ADDAY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::ADDABCZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDAZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpZ_A ( v, i, PS2_Float_Add );
	
#if defined INLINE_DEBUG_ADDAZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::ADDABCW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ADDAW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpW_A ( v, i, PS2_Float_Add );
	
#if defined INLINE_DEBUG_ADDAW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}




// SUBA //

void Execute::SUBA ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBA || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOp_A ( v, i, PS2_Float_Sub );
	
#if defined INLINE_DEBUG_SUBA || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::SUBAi ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBAI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " I=" << hex << v->vi [ 21 ].f << " " << v->vi [ 21 ].u;
#endif

	VuUpperOpI_A ( v, i, PS2_Float_Sub );
	
#if defined INLINE_DEBUG_SUBAI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::SUBAq ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBAQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Q=" << hex << v->vi [ 22 ].f << " " << v->vi [ 22 ].u;
#endif

	VuUpperOpQ_A ( v, i, PS2_Float_Sub );
	
#if defined INLINE_DEBUG_SUBAQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::SUBABCX ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBAX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpX_A ( v, i, PS2_Float_Sub );
	
#if defined INLINE_DEBUG_SUBAX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::SUBABCY ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBAY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpY_A ( v, i, PS2_Float_Sub );
	
#if defined INLINE_DEBUG_SUBAY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::SUBABCZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBAZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpZ_A ( v, i, PS2_Float_Sub );
	
#if defined INLINE_DEBUG_SUBAZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::SUBABCW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SUBAW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpW_A ( v, i, PS2_Float_Sub );
	
#if defined INLINE_DEBUG_SUBAW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}



// MADDA //

void Execute::MADDA ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDA || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOp_A ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADDA || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MADDAi ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDAI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " I=" << hex << v->vi [ 21 ].f << " " << v->vi [ 21 ].u;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpI_A ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADDAI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MADDAq ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDAQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Q=" << hex << v->vi [ 22 ].f << " " << v->vi [ 22 ].u;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpQ_A ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADDAQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MADDABCX ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDAX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpX_A ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADDAX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MADDABCY ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDAY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpY_A ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADDAY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MADDABCZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDAZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpZ_A ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADDAZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MADDABCW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MADDAW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpW_A ( v, i, PS2_Float_Madd );
	
#if defined INLINE_DEBUG_MADDAW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}


// MSUBA //

void Execute::MSUBA ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBA || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOp_A ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUBA || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MSUBAi ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBAI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " I=" << hex << v->vi [ 21 ].f << " " << v->vi [ 21 ].u;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpI_A ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUBAI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MSUBAq ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBAQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Q=" << hex << v->vi [ 22 ].f << " " << v->vi [ 22 ].u;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpQ_A ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUBAQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MSUBABCX ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBAX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpX_A ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUBAX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MSUBABCY ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBAY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpY_A ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUBAY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MSUBABCZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBAZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpZ_A ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUBAZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MSUBABCW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MSUBAW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	VuUpperOpW_A ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_MSUBAW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}



// MULA //

void Execute::MULA ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MULA || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOp_A ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MULA || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MULAi ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MULAI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " I=" << hex << v->vi [ 21 ].f << " " << v->vi [ 21 ].u;
#endif

	VuUpperOpI_A ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MULAI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MULAq ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MULAQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Q=" << hex << v->vi [ 22 ].f << " " << v->vi [ 22 ].u;
#endif

	VuUpperOpQ_A ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MULAQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MULABCX ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MULAX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpX_A ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MULAX || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MULABCY ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MULAY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpY_A ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MULAY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MULABCZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MULAZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpZ_A ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MULAZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::MULABCW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MULAW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	VuUpperOpW_A ( v, i, PS2_Float_Mul );
	
#if defined INLINE_DEBUG_MULAW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}







// other upper instructions //

void Execute::CLIP ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_CLIP || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << "Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

#ifdef PERFORM_CLIP_AS_INTEGER
	long lx, ly, lz, lw, lw_plus, lw_minus;
#else
	FloatLong fw_plus, fw_minus, fw;
	float fx, fy, fz;
#endif



	
#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
#endif


#ifdef PERFORM_CLIP_AS_INTEGER
	lx = v->vf [ i.Fs ].sx;
	ly = v->vf [ i.Fs ].sy;
	lz = v->vf [ i.Fs ].sz;
	lw = v->vf [ i.Ft ].sw;
	
	lw_plus = ( lw & 0x7fffffff );
	
	lx = ( lx >> 31 ) ^ ( lx & 0x7fffffff );
	ly = ( ly >> 31 ) ^ ( ly & 0x7fffffff );
	lz = ( lz >> 31 ) ^ ( lz & 0x7fffffff );
	
	lw_minus = lw_plus ^ 0xffffffff;
	
	v->ClippingFlag.Value = 0;
	if ( lx > lw_plus ) v->ClippingFlag.x_plus0 = 1; else if ( lx < lw_minus ) v->ClippingFlag.x_minus0 = 1;
	if ( ly > lw_plus ) v->ClippingFlag.y_plus0 = 1; else if ( ly < lw_minus ) v->ClippingFlag.y_minus0 = 1;
	if ( lz > lw_plus ) v->ClippingFlag.z_plus0 = 1; else if ( lz < lw_minus ) v->ClippingFlag.z_minus0 = 1;
	
#else
	fx = v->vf [ i.Fs ].fx;
	fy = v->vf [ i.Fs ].fy;
	fz = v->vf [ i.Fs ].fz;
	fw.f = v->vf [ i.Ft ].fw;
	
	ClampValue_f ( fx );
	ClampValue_f ( fy );
	ClampValue_f ( fz );
	ClampValue_f ( fw.f );

	fw_plus.l = fw.l & 0x7fffffff;
	fw_minus.l = fw.l | 0x80000000;
	
	
	
	// read clipping flag
	//v->ClippingFlag.Value = v->vi [ 18 ].u;
	//v->ClippingFlag.Value <<= 6;
	v->ClippingFlag.Value = 0;
	
	//if ( v->vf [ i.Fs ].fx > fw_plus.f ) v->ClippingFlag.x_plus0 = 1; else if ( v->vf [ i.Fs ].fx < fw_minus.f ) v->ClippingFlag.x_minus0 = 1;
	//if ( v->vf [ i.Fs ].fy > fw_plus.f ) v->ClippingFlag.y_plus0 = 1; else if ( v->vf [ i.Fs ].fy < fw_minus.f ) v->ClippingFlag.y_minus0 = 1;
	//if ( v->vf [ i.Fs ].fz > fw_plus.f ) v->ClippingFlag.z_plus0 = 1; else if ( v->vf [ i.Fs ].fz < fw_minus.f ) v->ClippingFlag.z_minus0 = 1;
	
	if ( fx > fw_plus.f ) v->ClippingFlag.x_plus0 = 1; else if ( fx < fw_minus.f ) v->ClippingFlag.x_minus0 = 1;
	if ( fy > fw_plus.f ) v->ClippingFlag.y_plus0 = 1; else if ( fy < fw_minus.f ) v->ClippingFlag.y_minus0 = 1;
	if ( fz > fw_plus.f ) v->ClippingFlag.z_plus0 = 1; else if ( fz < fw_minus.f ) v->ClippingFlag.z_minus0 = 1;
#endif
	


	//if ( !v->Status.SetClip_Flag )
	//{
		v->vi [ VU::REG_CLIPFLAG ].u = ( ( v->vi [ VU::REG_CLIPFLAG ].u << 6 ) | ( v->ClippingFlag.Value & 0x3f ) ) & 0xffffff;
	//}



#if defined INLINE_DEBUG_CLIP || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " ClippingFlag=" << hex << v->ClippingFlag.Value;
#endif
}



void Execute::OPMULA ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_OPMULA || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << "Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
#endif

	// ACC_x = fs_y * ft_z
	// ACC_y = fs_z * ft_x
	// ACC_z = fs_x * ft_y
	VuUpperOp_OPMULA ( v, i, PS2_Float_Mul );

#if defined INLINE_DEBUG_OPMULA || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}

void Execute::OPMSUB ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_OPMSUB || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << "Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << "Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
#endif

	
	// fd_x = ACC_x - fs_y * ft_z
	// fd_y = ACC_y - fs_x * ft_z
	// fd_z = ACC_z - fs_x * ft_y
	VuUpperOp_MSUB ( v, i, PS2_Float_Msub );
	
#if defined INLINE_DEBUG_OPMULA || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
#endif
}



void Execute::NOP ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_NOP || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
#endif


}






//// *** LOWER instructions *** ////




// branch/jump instructions //

void Execute::B ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_B || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

		// next instruction is in the branch delay slot
		VU::DelaySlot *d = & ( v->DelaySlots [ v->NextDelaySlotIndex ^ 1 ] );
		v->Status.DelaySlot_Valid |= 0x2;


		d->Instruction = i;
		//d->cb = r->_cb_Branch;
}

void Execute::BAL ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_BAL || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

// here we probably don't want the link register overwritten
// but can toggle execution of the int delay slot here for testing
#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif


		// next instruction is in the branch delay slot
		VU::DelaySlot *d = & ( v->DelaySlots [ v->NextDelaySlotIndex ^ 1 ] );
		v->Status.DelaySlot_Valid |= 0x2;


		d->Instruction = i;
		//d->cb = r->_cb_Branch;
		
		// should probably store updated program counter divided by 8 it looks like, unless the PC is already divided by 8??
		//v->vi [ i.it ].uLo = v->PC + 16;
		v->vi [ i.it ].uLo = ( v->PC + 16 ) >> 3;
}

void Execute::IBEQ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IBEQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " it=" << v->vi [ i.it ].uLo << " is=" << v->vi [ i.is ].uLo;
#endif

	if ( v->vi [ i.it ].uLo == v->vi [ i.is ].uLo )
	{
#if defined INLINE_DEBUG_IBEQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " BRANCH";
#endif


		// next instruction is in the branch delay slot
		VU::DelaySlot *d = & ( v->DelaySlots [ v->NextDelaySlotIndex ^ 1 ] );
		v->Status.DelaySlot_Valid |= 0x2;


		d->Instruction = i;
		//d->cb = r->_cb_Branch;
	}
}

void Execute::IBNE ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IBNE || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " it=" << v->vi [ i.it ].uLo << " is=" << v->vi [ i.is ].uLo;
#endif

	if ( v->vi [ i.it ].uLo != v->vi [ i.is ].uLo )
	{
#if defined INLINE_DEBUG_IBNE || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " BRANCH";
#endif

		// next instruction is in the branch delay slot
		VU::DelaySlot *d = & ( v->DelaySlots [ v->NextDelaySlotIndex ^ 1 ] );
		v->Status.DelaySlot_Valid |= 0x2;


		d->Instruction = i;
		//d->cb = r->_cb_Branch;

	}
}

void Execute::IBLTZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IBLTZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << v->vi [ i.is ].uLo;
#endif

	if ( v->vi [ i.is ].sLo < 0 )
	{
#if defined INLINE_DEBUG_IBLTZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " BRANCH";
#endif

		// next instruction is in the branch delay slot
		VU::DelaySlot *d = & ( v->DelaySlots [ v->NextDelaySlotIndex ^ 1 ] );
		v->Status.DelaySlot_Valid |= 0x2;


		d->Instruction = i;
		//d->cb = r->_cb_Branch;
	}
}

void Execute::IBGTZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IBGTZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << v->vi [ i.is ].uLo;
#endif

	if ( v->vi [ i.is ].sLo > 0 )
	{
#if defined INLINE_DEBUG_IBGTZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " BRANCH";
#endif

		// next instruction is in the branch delay slot
		VU::DelaySlot *d = & ( v->DelaySlots [ v->NextDelaySlotIndex ^ 1 ] );
		v->Status.DelaySlot_Valid |= 0x2;

		d->Instruction = i;
		//d->cb = r->_cb_Branch;
	}
}

void Execute::IBLEZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IBLEZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << v->vi [ i.is ].uLo;
#endif

	if ( v->vi [ i.is ].sLo <= 0 )
	{
#if defined INLINE_DEBUG_IBLEZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " BRANCH";
#endif

		// next instruction is in the branch delay slot
		VU::DelaySlot *d = & ( v->DelaySlots [ v->NextDelaySlotIndex ^ 1 ] );
		v->Status.DelaySlot_Valid |= 0x2;

		d->Instruction = i;
		//d->cb = r->_cb_Branch;
	}
}

void Execute::IBGEZ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IBGEZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << v->vi [ i.is ].uLo;
#endif

	if ( v->vi [ i.is ].sLo >= 0 )
	{
#if defined INLINE_DEBUG_IBGEZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " BRANCH";
#endif

		// next instruction is in the branch delay slot
		VU::DelaySlot *d = & ( v->DelaySlots [ v->NextDelaySlotIndex ^ 1 ] );
		v->Status.DelaySlot_Valid |= 0x2;


		d->Instruction = i;
		//d->cb = r->_cb_Branch;
	}
}

void Execute::JR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_JR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " is(hex)=" << v->vi [ i.is ].uLo;
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif
	
	// next instruction is in the branch delay slot
	VU::DelaySlot *d = & ( v->DelaySlots [ v->NextDelaySlotIndex ^ 1 ] );
	v->Status.DelaySlot_Valid |= 0x2;

	d->Instruction = i;
	//d->cb = r->_cb_JumpRegister;

	// *** todo *** check if address exception should be generated if lower 3-bits of jump address are not zero
	// will clear out lower three bits of address for now
	//d->Data = v->vi [ i.is ].uLo & ~7;
	d->Data = v->vi [ i.is & 0xf ].uLo;
	

}

void Execute::JALR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_JALR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " is(hex)=" << v->vi [ i.is ].uLo;
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif
	
	// next instruction is in the branch delay slot
	VU::DelaySlot *d = & ( v->DelaySlots [ v->NextDelaySlotIndex ^ 1 ] );
	v->Status.DelaySlot_Valid |= 0x2;


	d->Instruction = i;
	//d->cb = r->_cb_JumpRegister;

	// *** todo *** check if address exception should be generated if lower 3-bits of jump address are not zero
	// will clear out lower three bits of address for now
	//d->Data = v->vi [ i.is ].uLo & ~7;
	d->Data = v->vi [ i.is & 0xf ].uLo;
	
	
	//v->vi [ i.it ].uLo = v->PC + 16;
	v->vi [ i.it & 0xf ].uLo = ( v->PC + 16 ) >> 3;
	
#if defined INLINE_DEBUG_JALR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output: it(hex)=" << v->vi [ i.it ].uLo;
#endif
}









// FC/FM/FS instructions //

void Execute::FCEQ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FCEQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " CF=" << v->vi [ 18 ].u;
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif


#ifdef ENABLE_SNAPSHOTS
	v->vi [ 1 ].u = ! ( ( v->FlagSave [ ( v->iFlagSave_Index + 1 ) & v->c_lFlag_Delay_Mask ].ClipFlag ^ i.Imm24 ) & 0xffffff );
#else
	//v->vi [ 1 ].u = ( ( ( v->vi [ 18 ].u & 0xffffff ) == i.Imm24 ) ? 1 : 0 );
	v->vi [ 1 ].u = ! ( ( v->vi [ 18 ].u ^ i.Imm24 ) & 0xffffff );
#endif

	
#if defined INLINE_DEBUG_FCEQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " vi1=" << v->vi [ 1 ].u;
#endif
}

void Execute::FCAND ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FCAND || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " CF=" << v->vi [ 18 ].u;
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif


#ifdef ENABLE_SNAPSHOTS
	v->vi [ 1 ].u = ( ( v->FlagSave [ ( v->iFlagSave_Index + 1 ) & v->c_lFlag_Delay_Mask ].ClipFlag & i.Imm24 ) ? 1 : 0 );
#else
	v->vi [ 1 ].u = ( ( v->vi [ 18 ].u & i.Imm24 ) ? 1 : 0 );
#endif

	
#if defined INLINE_DEBUG_FCAND || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " vi1=" << v->vi [ 1 ].u;
#endif
}

void Execute::FCOR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FCOR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " CF=" << v->vi [ 18 ].u;
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif


#ifdef ENABLE_SNAPSHOTS
	v->vi [ 1 ].u = ( ( ( ( v->FlagSave [ ( v->iFlagSave_Index + 1 ) & v->c_lFlag_Delay_Mask ].ClipFlag & 0xffffff ) | i.Imm24 ) == 0xffffff ) ? 1 : 0 );
#else
	v->vi [ 1 ].u = ( ( ( ( v->vi [ 18 ].u & 0xffffff ) | i.Imm24 ) == 0xffffff ) ? 1 : 0 );
#endif


#if defined INLINE_DEBUG_FCOR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " vi1=" << v->vi [ 1 ].u;
#endif
}

void Execute::FCGET ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FCGET || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " CF=" << v->vi [ 18 ].u;
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif


#ifdef ENABLE_SNAPSHOTS
	v->vi [ i.it & 0xf ].uLo = v->FlagSave [ ( v->iFlagSave_Index + 1 ) & v->c_lFlag_Delay_Mask ].ClipFlag & 0xfff;
#else
	v->vi [ i.it ].uLo = v->vi [ 18 ].u & 0xfff;
#endif

	
#if defined INLINE_DEBUG_FCGET || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " it=" << v->vi [ i.it ].u;
#endif
}

void Execute::FCSET ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FCSET || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//v->vi [ 18 ].u = i.Imm24;


#ifdef ENABLE_SNAPSHOTS
	v->vi [ 18 ].u = i.Imm24;
	
	// also need to inform that we set the clip flag (in case the upper instruction is CLIP)
	v->Status.SetClip_Flag = 1;
#else

	//v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].FlagsAffected = VU::RF_SET_CLIP;
	//v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].ClippingFlag = i.Imm24;
	v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].FlagsAffected_Lower = VU::RF_SET_CLIP;
	v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].FlagsSet_Lower = i.Imm24;

#endif

}

void Execute::FMEQ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FMEQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].u << " MAC=" << v->vi [ 17 ].u;
#endif

#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcReg ( i.is + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: no need to set destination register since instruction is on the integer pipeline
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif


#ifdef ENABLE_SNAPSHOTS
	v->vi [ i.it & 0xf ].u = ! ( ( v->FlagSave [ ( v->iFlagSave_Index + 1 ) & v->c_lFlag_Delay_Mask ].MACFlag ^ v->vi [ i.is & 0xf ].u ) & 0xffff );
#else
	//v->vi [ i.it ].u = ~( v->vi [ i.is ].u ^ v->vi [ 17 ].u );
	v->vi [ i.it ].u = ! ( ( v->vi [ i.is ].u ^ v->vi [ 17 ].u ) & 0xffff );
#endif

	
#if defined INLINE_DEBUG_FMEQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " it=" << v->vi [ i.it ].u;
#endif
}

void Execute::FMAND ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FMAND || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].u << " MAC=" << v->vi [ 17 ].u;
#endif

#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcReg ( i.is + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: no need to set destination register since instruction is on the integer pipeline
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif


#ifdef ENABLE_SNAPSHOTS
	v->vi [ i.it & 0xf ].u = v->FlagSave [ ( v->iFlagSave_Index + 1 ) & v->c_lFlag_Delay_Mask ].MACFlag & v->vi [ i.is & 0xf ].u;
#else
	v->vi [ i.it ].u = v->vi [ i.is ].u & v->vi [ 17 ].u;
#endif


#if defined INLINE_DEBUG_FMAND || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " it=" << v->vi [ i.it ].u;
#endif
}

void Execute::FMOR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FMOR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].u << " MAC=" << v->vi [ 17 ].u;
#endif

#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcReg ( i.is + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: no need to set destination register since instruction is on the integer pipeline
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif


#ifdef ENABLE_SNAPSHOTS
	v->vi [ i.it & 0xf ].u = v->FlagSave [ ( v->iFlagSave_Index + 1 ) & v->c_lFlag_Delay_Mask ].MACFlag | v->vi [ i.is & 0xf ].u;
#else
	v->vi [ i.it ].u = v->vi [ i.is ].u | v->vi [ 17 ].u;
#endif

	
#if defined INLINE_DEBUG_FMOR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " it=" << v->vi [ i.it ].u;
#endif
}

void Execute::FSEQ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FSEQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " STAT=" << v->vi [ 16 ].u;
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif


#ifdef ENABLE_NEW_QP_HANDLING
	// this affects the status flag also
	v->CheckQ ();
#endif


#ifdef ENABLE_SNAPSHOTS
	v->vi [ i.it & 0xf ].u = ! ( ( v->FlagSave [ ( v->iFlagSave_Index + 1 ) & v->c_lFlag_Delay_Mask ].StatusFlag ^ ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) ) & 0xfff );
#else
	//v->vi [ i.it ].u = ~( v->vi [ 16 ].u ^ ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfff ) );
	//v->vi [ i.it ].u = ( ( ( v->vi [ 16 ].u & 0xfff ) == ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfff ) ) ? 1 : 0 );
	v->vi [ i.it ].u = ! ( ( v->vi [ 16 ].u ^ ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) ) & 0xfff );
#endif


#if defined INLINE_DEBUG_FSEQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " it=" << v->vi [ i.it ].u;
#endif
}

void Execute::FSSET ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FSSET || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//v->vi [ 16 ].u = ( v->vi [ 16 ].u & 0x3f ) | ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfc0 );


#ifdef ENABLE_SNAPSHOTS
	v->vi [ 16 ].u = ( v->vi [ 16 ].u & 0x3f ) | ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfc0 );
	
	// also need to inform that we set the clip flag (in case the upper instruction is CLIP)
	v->Status.SetStatus_Flag = 1;
#else
	
	//v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].FlagsAffected = VU::RF_SET_STICKY;
	//v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].StatusFlag = ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfc0 );
	v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].FlagsAffected_Lower = VU::RF_SET_STICKY;
	v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].FlagsSet_Lower = ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfc0 );

#endif

}

void Execute::FSAND ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FSAND || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " STAT=" << v->vi [ 16 ].u;
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif


#ifdef ENABLE_NEW_QP_HANDLING
	// this affects the status flag also
	v->CheckQ ();
#endif


#ifdef ENABLE_SNAPSHOTS
	v->vi [ i.it ].u = v->FlagSave [ ( v->iFlagSave_Index + 1 ) & v->c_lFlag_Delay_Mask ].StatusFlag & ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfff );
#else
	v->vi [ i.it ].u = v->vi [ 16 ].u & ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfff );
#endif

	
#if defined INLINE_DEBUG_FSAND || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " it=" << v->vi [ i.it ].u;
#endif
}

void Execute::FSOR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_FSOR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " STAT=" << v->vi [ 16 ].u;
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif


#ifdef ENABLE_NEW_QP_HANDLING
	// this affects the status flag also
	v->CheckQ ();
#endif


#ifdef ENABLE_SNAPSHOTS
	v->vi [ i.it ].u = v->FlagSave [ ( v->iFlagSave_Index + 1 ) & v->c_lFlag_Delay_Mask ].StatusFlag | ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfff );
#else
	v->vi [ i.it ].u = v->vi [ 16 ].u | ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfff );
#endif


#if defined INLINE_DEBUG_FSOR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " it=" << v->vi [ i.it ].u;
#endif
}



// Integer math //


void Execute::IADD ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IADD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].uLo << " it=" << v->vi [ i.it ].uLo;
#endif

#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcRegs ( ( i.is & 0xf ) + 32, ( i.it & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: no need to set destination register since instruction is on the integer pipeline
#endif

	// id = is + it

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

#ifdef USE_NEW_RECOMPILE2_INTCALC

	// check if int calc result needs to be output to delay slot or not
	if ( v->pLUT_StaticInfo [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 10 ) )
	{
#if defined INLINE_DEBUG_IADD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << ">INT-DELAY-SLOT";
#endif
		v->Set_IntDelaySlot ( i.id & 0xf, v->vi [ i.is & 0xf ].uLo + v->vi [ i.it & 0xf ].uLo );
	}
	else
	{
		v->vi [ i.id & 0xf ].u = v->vi [ i.is & 0xf ].uLo + v->vi [ i.it & 0xf ].uLo;
	}

#else

#ifdef ENABLE_INTDELAYSLOT_CALC

	v->Set_IntDelaySlot ( i.id & 0xf, v->vi [ i.is & 0xf ].uLo + v->vi [ i.it & 0xf ].uLo );

#else
	v->vi [ i.id ].u = v->vi [ i.is ].uLo + v->vi [ i.it ].uLo;
#endif

#endif
	
#if defined INLINE_DEBUG_IADD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: id=" << hex << v->vi [ i.id ].uLo;
#endif
}

void Execute::VIADD ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IADD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].uLo << " it=" << v->vi [ i.it ].uLo;
#endif

		v->vi [ i.id & 0xf ].u = v->vi [ i.is & 0xf ].uLo + v->vi [ i.it & 0xf ].uLo;

#if defined INLINE_DEBUG_IADD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: id=" << hex << v->vi [ i.id ].uLo;
#endif
}


void Execute::IADDI ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IADDI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].uLo;
#endif

#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: no need to set destination register since instruction is on the integer pipeline
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	// it = is + Imm5
#ifdef USE_NEW_RECOMPILE2_INTCALC

	// check if int calc result needs to be output to delay slot or not
	if ( v->pLUT_StaticInfo [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 10 ) )
	{
#if defined INLINE_DEBUG_IADDI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << ">INT-DELAY-SLOT";
#endif
		v->Set_IntDelaySlot ( i.it & 0xf, v->vi [ i.is & 0xf ].uLo + ( (s32) i.Imm5 ) );
	}
	else
	{
		v->vi [ i.it & 0xf ].u = v->vi [ i.is & 0xf ].uLo + ( (s32) i.Imm5 );
	}

#else

#ifdef ENABLE_INTDELAYSLOT_CALC
	
	// *TODO* ?? adding with an s32 could put a 32-bit value in i.it ??
	v->Set_IntDelaySlot ( i.it & 0xf, v->vi [ i.is & 0xf ].uLo + ( (s32) i.Imm5 ) );
	//v->Set_IntDelaySlot ( i.it & 0xf, ( v->vi [ i.is & 0xf ].uLo + ( (s16) i.Imm5 ) ) | ( v->vi [ i.is & 0xf ].u & 0xffff0000 ) );

#else
	v->vi [ i.it ].u = v->vi [ i.is ].uLo + ( (s32) i.Imm5 );
#endif

#endif
	
#if defined INLINE_DEBUG_IADDI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: it=" << hex << v->vi [ i.it ].uLo;
#endif
}

void Execute::VIADDI ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IADDI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].uLo;
#endif

		v->vi [ i.it & 0xf ].u = v->vi [ i.is & 0xf ].uLo + ( (s32) i.Imm5 );

#if defined INLINE_DEBUG_IADDI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: it=" << hex << v->vi [ i.it ].uLo;
#endif
}


void Execute::IADDIU ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IADDIU || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].uLo;
#endif

#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: no need to set destination register since instruction is on the integer pipeline
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	// it = is + Imm15
#ifdef USE_NEW_RECOMPILE2_INTCALC

	// check if int calc result needs to be output to delay slot or not
	if ( v->pLUT_StaticInfo [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 10 ) )
	{
#if defined INLINE_DEBUG_IADDIU || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << ">INT-DELAY-SLOT";
#endif
		v->Set_IntDelaySlot ( i.it & 0xf, v->vi [ i.is & 0xf ].uLo + ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) );
	}
	else
	{
		v->vi [ i.it & 0xf ].u = v->vi [ i.is & 0xf ].uLo + ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) );
	}

#else

#ifdef ENABLE_INTDELAYSLOT_CALC
	
	v->Set_IntDelaySlot ( i.it & 0xf, v->vi [ i.is & 0xf ].uLo + ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) );
	//v->Set_IntDelaySlot ( i.it & 0xf, ( v->vi [ i.is & 0xf ].uLo + ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) ) | ( v->vi [ i.is & 0xf ].u & 0xffff0000 ) );

#else
	v->vi [ i.it ].u = v->vi [ i.is ].uLo + ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) );
#endif

#endif
	
#if defined INLINE_DEBUG_IADDIU || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: it=" << hex << v->vi [ i.it ].uLo;
#endif
}

void Execute::ISUB ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ISUB || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].uLo << " it=" << v->vi [ i.it ].uLo;
#endif

#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcRegs ( ( i.is & 0xf ) + 32, ( i.it & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: no need to set destination register since instruction is on the integer pipeline
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	// id = is - it
#ifdef USE_NEW_RECOMPILE2_INTCALC

	// check if int calc result needs to be output to delay slot or not
	if ( v->pLUT_StaticInfo [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 10 ) )
	{
#if defined INLINE_DEBUG_ISUB || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << ">INT-DELAY-SLOT";
#endif
		v->Set_IntDelaySlot ( i.id & 0xf, v->vi [ i.is & 0xf ].uLo - v->vi [ i.it & 0xf ].uLo );
	}
	else
	{
		v->vi [ i.id & 0xf ].u = v->vi [ i.is & 0xf ].uLo - v->vi [ i.it & 0xf ].uLo;
	}

#else

#ifdef ENABLE_INTDELAYSLOT_CALC
	
	v->Set_IntDelaySlot ( i.id & 0xf, v->vi [ i.is & 0xf ].uLo - v->vi [ i.it & 0xf ].uLo );

#else
	v->vi [ i.id ].u = v->vi [ i.is ].uLo - v->vi [ i.it ].uLo;
#endif

#endif
	
#if defined INLINE_DEBUG_ISUB || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: id=" << hex << v->vi [ i.id ].uLo;
#endif
}

void Execute::VISUB ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ISUB || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].uLo << " it=" << v->vi [ i.it ].uLo;
#endif

		v->vi [ i.id & 0xf ].u = v->vi [ i.is & 0xf ].uLo - v->vi [ i.it & 0xf ].uLo;

#if defined INLINE_DEBUG_ISUB || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: id=" << hex << v->vi [ i.id ].uLo;
#endif
}

void Execute::ISUBIU ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ISUBIU || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].uLo;
#endif

#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: no need to set destination register since instruction is on the integer pipeline
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	// it = is - Imm15
#ifdef USE_NEW_RECOMPILE2_INTCALC

	// check if int calc result needs to be output to delay slot or not
	if ( v->pLUT_StaticInfo [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 10 ) )
	{
#if defined INLINE_DEBUG_ISUBIU || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << ">INT-DELAY-SLOT";
#endif
		v->Set_IntDelaySlot ( i.it & 0xf, v->vi [ i.is & 0xf ].uLo - ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) );
	}
	else
	{
		v->vi [ i.it & 0xf ].u = v->vi [ i.is & 0xf ].uLo - ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) );
	}

#else

#ifdef ENABLE_INTDELAYSLOT_CALC
	
	v->Set_IntDelaySlot ( i.it & 0xf, v->vi [ i.is & 0xf ].uLo - ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) );
	//v->Set_IntDelaySlot ( i.it & 0xf, ( v->vi [ i.is & 0xf ].uLo - ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) ) | ( v->vi [ i.is & 0xf ].u & 0xffff0000 ) );

#else
	v->vi [ i.it ].u = v->vi [ i.is ].uLo - ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) );
#endif

#endif
	
#if defined INLINE_DEBUG_ISUBIU || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: it=" << hex << v->vi [ i.it ].uLo;
#endif
}


void Execute::IAND ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IAND || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].uLo << " it=" << v->vi [ i.it ].uLo;
#endif

#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcRegs ( ( i.is & 0xf ) + 32, ( i.it & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: no need to set destination register since instruction is on the integer pipeline
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	// id = is & it
#ifdef USE_NEW_RECOMPILE2_INTCALC

	// check if int calc result needs to be output to delay slot or not
	if ( v->pLUT_StaticInfo [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 10 ) )
	{
#if defined INLINE_DEBUG_IAND || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << ">INT-DELAY-SLOT";
#endif
		v->Set_IntDelaySlot ( i.id & 0xf, v->vi [ i.is & 0xf ].uLo & v->vi [ i.it & 0xf ].uLo );
	}
	else
	{
		v->vi [ i.id & 0xf ].u = v->vi [ i.is & 0xf ].uLo & v->vi [ i.it & 0xf ].uLo;
	}

#else

#ifdef ENABLE_INTDELAYSLOT_CALC
	
	v->Set_IntDelaySlot ( i.id & 0xf, v->vi [ i.is & 0xf ].uLo & v->vi [ i.it & 0xf ].uLo );

#else
	v->vi [ i.id ].u = v->vi [ i.is ].uLo & v->vi [ i.it ].uLo;
#endif

#endif
	
#if defined INLINE_DEBUG_IAND || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: id=" << hex << v->vi [ i.id ].uLo;
#endif
}

void Execute::VIAND ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IAND || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].uLo << " it=" << v->vi [ i.it ].uLo;
#endif

		v->vi [ i.id & 0xf ].u = v->vi [ i.is & 0xf ].uLo & v->vi [ i.it & 0xf ].uLo;

#if defined INLINE_DEBUG_IAND || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: id=" << hex << v->vi [ i.id ].uLo;
#endif
}

void Execute::IOR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IOR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].uLo << " it=" << v->vi [ i.it ].uLo;
#endif

#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcRegs ( ( i.is & 0xf ) + 32, ( i.it & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: no need to set destination register since instruction is on the integer pipeline
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	// id = is | it
#ifdef USE_NEW_RECOMPILE2_INTCALC

	// check if int calc result needs to be output to delay slot or not
	if ( v->pLUT_StaticInfo [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 10 ) )
	{
#if defined INLINE_DEBUG_IOR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << ">INT-DELAY-SLOT";
#endif
		v->Set_IntDelaySlot ( i.id & 0xf, v->vi [ i.is & 0xf ].uLo | v->vi [ i.it & 0xf ].uLo );
	}
	else
	{
		v->vi [ i.id & 0xf ].u = v->vi [ i.is & 0xf ].uLo | v->vi [ i.it & 0xf ].uLo;
	}

#else

#ifdef ENABLE_INTDELAYSLOT_CALC
	
	v->Set_IntDelaySlot ( i.id & 0xf, v->vi [ i.is & 0xf ].uLo | v->vi [ i.it & 0xf ].uLo );

#else
	v->vi [ i.id ].u = v->vi [ i.is ].uLo | v->vi [ i.it ].uLo;
#endif

#endif
	
#if defined INLINE_DEBUG_IOR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: id=" << hex << v->vi [ i.id ].uLo;
#endif
}


void Execute::VIOR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_IOR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " is=" << hex << v->vi [ i.is ].uLo << " it=" << v->vi [ i.it ].uLo;
#endif

		v->vi [ i.id & 0xf ].u = v->vi [ i.is & 0xf ].uLo | v->vi [ i.it & 0xf ].uLo;

#if defined INLINE_DEBUG_IOR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: id=" << hex << v->vi [ i.id ].uLo;
#endif
}



// STORE (to integer) instructions //

void Execute::ISWR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ISWR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Base=" << v->vi [ i.is ].uLo << " it=" << v->vi [ i.it ].uLo;
#endif

	// ISWR itdest, (is)
	// do Imm11 x16
	
	u32 StoreAddress;
	u32* pVuMem32;
	
#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcRegs ( ( i.is & 0xf ) + 32, ( i.it & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: don't want to set destination register until after upper instruction is executed!!!
#endif

#ifdef ENABLE_INTDELAYSLOT_ISWR
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	StoreAddress = v->vi [ i.is & 0xf ].uLo << 2;
	
	pVuMem32 = v->GetMemPtr ( StoreAddress );
	
	if ( i.destx ) pVuMem32 [ 0 ] = v->vi [ i.it & 0xf ].uLo;
	if ( i.desty ) pVuMem32 [ 1 ] = v->vi [ i.it & 0xf ].uLo;
	if ( i.destz ) pVuMem32 [ 2 ] = v->vi [ i.it & 0xf ].uLo;
	if ( i.destw ) pVuMem32 [ 3 ] = v->vi [ i.it & 0xf ].uLo;
	
	// if writing to TPC, then is should also start VU#1
	if ( !v->Number )
	{
		if ( ( StoreAddress << 2 ) == 0x43a0 )
		{
			if ( i.destx )
			{
				VU1::_VU1->PC = v->vf [ i.Fs ].uw0;
				VU1::_VU1->StartVU ();
			}
		}
	}
	
#if defined INLINE_DEBUG_ISWR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " SA=" << v->vi [ i.is ].uLo;
#endif
}


void Execute::ISW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ISW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Base=" << v->vi [ i.is ].uLo << " it=" << v->vi [ i.it ].uLo;
#endif

	// ISW itdest, Imm11(is)
	// do Imm11 x16
	
	u32 StoreAddress;
	u32* pVuMem32;
	
#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcRegs ( ( i.is & 0xf ) + 32, ( i.it & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: don't want to set destination register until after upper instruction is executed!!!
#endif

#ifdef ENABLE_INTDELAYSLOT_ISW
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	StoreAddress = ( v->vi [ i.is & 0xf ].sLo + i.Imm11 ) << 2;
	
	pVuMem32 = v->GetMemPtr ( StoreAddress );
	
	if ( i.destx ) pVuMem32 [ 0 ] = v->vi [ i.it & 0xf ].uLo;
	if ( i.desty ) pVuMem32 [ 1 ] = v->vi [ i.it & 0xf ].uLo;
	if ( i.destz ) pVuMem32 [ 2 ] = v->vi [ i.it & 0xf ].uLo;
	if ( i.destw ) pVuMem32 [ 3 ] = v->vi [ i.it & 0xf ].uLo;

	// if writing to TPC, then is should also start VU#1
	if ( !v->Number )
	{
		if ( ( StoreAddress << 2 ) == 0x43a0 )
		{
			if ( i.destx )
			{
				VU1::_VU1->PC = v->vf [ i.Fs ].uw0;
				VU1::_VU1->StartVU ();
			}
		}
	}
	
#if defined INLINE_DEBUG_ISW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " SA=" << ( v->vi [ i.is ].sLo + i.Imm11 );
#endif
}


// STORE (to float) instructions //

void Execute::SQ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " it=" << v->vi [ i.it ].uLo << " fs=" << v->vf [ i.Fs ].uw0 << " " << v->vf [ i.Fs ].uw1 << " " << v->vf [ i.Fs ].uw2 << " " << v->vf [ i.Fs ].uw3;
#endif

	// SQ fsdest, Imm11(it)
	// do Imm11 x16
	
	u32 StoreAddress;
	u32* pVuMem32;
	
#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
#endif
	
#ifdef ENABLE_STALLS_INT
	// set the source integer register
	v->Set_Int_SrcReg ( ( i.it & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	StoreAddress = ( v->vi [ i.it & 0xf ].sLo + i.Imm11 ) << 2;
	
	pVuMem32 = v->GetMemPtr ( StoreAddress );
	
	if ( i.destx ) pVuMem32 [ 0 ] = v->vf [ i.Fs ].uw0;
	if ( i.desty ) pVuMem32 [ 1 ] = v->vf [ i.Fs ].uw1;
	if ( i.destz ) pVuMem32 [ 2 ] = v->vf [ i.Fs ].uw2;
	if ( i.destw ) pVuMem32 [ 3 ] = v->vf [ i.Fs ].uw3;

	// if writing to TPC, then is should also start VU#1
	if ( !v->Number )
	{
		if ( ( StoreAddress << 2 ) == 0x43a0 )
		{
			if ( i.destx )
			{
				VU1::_VU1->PC = v->vf [ i.Fs ].uw0;
				VU1::_VU1->StartVU ();
			}
		}
	}
	
#if defined INLINE_DEBUG_SQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " SA=" << ( v->vi [ i.it ].sLo + i.Imm11 );
#endif
}


void Execute::SQD ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SQD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " it=" << v->vi [ i.it ].uLo << " fs=" << v->vf [ i.Fs ].uw0 << " " << v->vf [ i.Fs ].uw1 << " " << v->vf [ i.Fs ].uw2 << " " << v->vf [ i.Fs ].uw3;
#endif

	// SQD fsdest, (--it)
	// do Imm11 x16
	
	u32 StoreAddress;
	u32* pVuMem32;
	
#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
#endif

#ifdef ENABLE_STALLS_INT
	// set the source integer register
	v->Set_Int_SrcReg ( ( i.it & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
#endif

	// pre-decrement
#ifdef ENABLE_INTDELAYSLOT_SQD_BEFORE
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

#ifdef USE_NEW_RECOMPILE2_SQD

	// check if int calc result needs to be output to delay slot or not
	if ( v->pLUT_StaticInfo [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 10 ) )
	{
		v->Set_IntDelaySlot ( i.it & 0xf, v->vi [ i.it & 0xf ].uLo - 1 );
		StoreAddress = ( v->vi [ i.it & 0xf ].uLo - 1 ) << 2;
	}
	else
	{
		v->vi [ i.it & 0xf ].uLo--;
		StoreAddress = v->vi [ i.it & 0xf ].uLo << 2;
	}

#else

#ifdef ENABLE_INTDELAYSLOT_SQD_AFTER
	v->Set_IntDelaySlot ( i.it & 0xf, v->vi [ i.it & 0xf ].uLo - 1 );
	StoreAddress = ( v->vi [ i.it & 0xf ].uLo - 1 ) << 2;

#else
	v->vi [ i.it & 0xf ].uLo--;
	
	StoreAddress = v->vi [ i.it & 0xf ].uLo << 2;
#endif

#endif
	
	pVuMem32 = v->GetMemPtr ( StoreAddress );
	
	if ( i.destx ) pVuMem32 [ 0 ] = v->vf [ i.Fs ].uw0;
	if ( i.desty ) pVuMem32 [ 1 ] = v->vf [ i.Fs ].uw1;
	if ( i.destz ) pVuMem32 [ 2 ] = v->vf [ i.Fs ].uw2;
	if ( i.destw ) pVuMem32 [ 3 ] = v->vf [ i.Fs ].uw3;

	// if writing to TPC, then is should also start VU#1
	if ( !v->Number )
	{
		if ( ( StoreAddress << 2 ) == 0x43a0 )
		{
			if ( i.destx )
			{
				VU1::_VU1->PC = v->vf [ i.Fs ].uw0;
				VU1::_VU1->StartVU ();
			}
		}
	}
	
#if defined INLINE_DEBUG_SQD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " SA=" << (v->vi [ i.it ].uLo-1);
#endif
}

void Execute::VSQD ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SQD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " it=" << v->vi [ i.it ].uLo << " fs=" << v->vf [ i.Fs ].uw0 << " " << v->vf [ i.Fs ].uw1 << " " << v->vf [ i.Fs ].uw2 << " " << v->vf [ i.Fs ].uw3;
#endif

	// SQD fsdest, (--it)
	// do Imm11 x16
	
	u32 StoreAddress;
	u32* pVuMem32;

	v->vi [ i.it & 0xf ].uLo--;
	StoreAddress = v->vi [ i.it & 0xf ].uLo << 2;

	pVuMem32 = v->GetMemPtr ( StoreAddress );
	
	if ( i.destx ) pVuMem32 [ 0 ] = v->vf [ i.Fs ].uw0;
	if ( i.desty ) pVuMem32 [ 1 ] = v->vf [ i.Fs ].uw1;
	if ( i.destz ) pVuMem32 [ 2 ] = v->vf [ i.Fs ].uw2;
	if ( i.destw ) pVuMem32 [ 3 ] = v->vf [ i.Fs ].uw3;

	// if writing to TPC, then is should also start VU#1
	if ( !v->Number )
	{
		if ( ( StoreAddress << 2 ) == 0x43a0 )
		{
			if ( i.destx )
			{
				VU1::_VU1->PC = v->vf [ i.Fs ].uw0;
				VU1::_VU1->StartVU ();
			}
		}
	}
	
#if defined INLINE_DEBUG_SQD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " SA=" << (v->vi [ i.it ].uLo-1);
#endif
}

void Execute::SQI ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SQI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " it=" << v->vi [ i.it ].uLo << " fs(hex)=" << v->vf [ i.Fs ].uw0 << " " << v->vf [ i.Fs ].uw1 << " " << v->vf [ i.Fs ].uw2 << " " << v->vf [ i.Fs ].uw3;
	debug << dec << " fs(dec)=" << v->vf [ i.Fs ].fx << " " << v->vf [ i.Fs ].fy << " " << v->vf [ i.Fs ].fz << " " << v->vf [ i.Fs ].fw;
#endif

	// SQD fsdest, (it++)
	// do Imm11 x16
	
	u32 StoreAddress;
	u32* pVuMem32;
	
#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
#endif
	
#ifdef ENABLE_STALLS_INT
	// set the source integer register
	v->Set_Int_SrcReg ( ( i.it & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
#endif

#ifdef ENABLE_INTDELAYSLOT_SQI_BEFORE
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	StoreAddress = v->vi [ i.it & 0xf ].uLo << 2;
	
	pVuMem32 = v->GetMemPtr ( StoreAddress );
	
	if ( i.destx ) pVuMem32 [ 0 ] = v->vf [ i.Fs ].uw0;
	if ( i.desty ) pVuMem32 [ 1 ] = v->vf [ i.Fs ].uw1;
	if ( i.destz ) pVuMem32 [ 2 ] = v->vf [ i.Fs ].uw2;
	if ( i.destw ) pVuMem32 [ 3 ] = v->vf [ i.Fs ].uw3;

	// if writing to TPC, then is should also start VU#1
	if ( !v->Number )
	{
		if ( ( StoreAddress << 2 ) == 0x43a0 )
		{
			if ( i.destx )
			{
				VU1::_VU1->PC = v->vf [ i.Fs ].uw0;
				VU1::_VU1->StartVU ();
			}
			
		}
	}

#ifdef USE_NEW_RECOMPILE2_SQI

	// check if int calc result needs to be output to delay slot or not
	if ( v->pLUT_StaticInfo [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 10 ) )
	{
		v->Set_IntDelaySlot ( i.it & 0xf, v->vi [ i.it & 0xf ].uLo + 1 );
	}
	else
	{
		v->vi [ i.it & 0xf ].uLo++;
	}

#else

	// post-increment
#ifdef ENABLE_INTDELAYSLOT_SQI_AFTER
	
	v->Set_IntDelaySlot ( i.it & 0xf, v->vi [ i.it & 0xf ].uLo + 1 );

#else
	v->vi [ i.it & 0xf ].uLo++;
#endif

#endif

#if defined INLINE_DEBUG_SQI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " SA=" << v->vi [ i.it ].uLo;
#endif
}

void Execute::VSQI ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SQI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " it=" << v->vi [ i.it ].uLo << " fs(hex)=" << v->vf [ i.Fs ].uw0 << " " << v->vf [ i.Fs ].uw1 << " " << v->vf [ i.Fs ].uw2 << " " << v->vf [ i.Fs ].uw3;
	debug << dec << " fs(dec)=" << v->vf [ i.Fs ].fx << " " << v->vf [ i.Fs ].fy << " " << v->vf [ i.Fs ].fz << " " << v->vf [ i.Fs ].fw;
#endif

	// SQD fsdest, (it++)
	// do Imm11 x16
	
	u32 StoreAddress;
	u32* pVuMem32;

	StoreAddress = v->vi [ i.it & 0xf ].uLo << 2;
	
	pVuMem32 = v->GetMemPtr ( StoreAddress );
	
	if ( i.destx ) pVuMem32 [ 0 ] = v->vf [ i.Fs ].uw0;
	if ( i.desty ) pVuMem32 [ 1 ] = v->vf [ i.Fs ].uw1;
	if ( i.destz ) pVuMem32 [ 2 ] = v->vf [ i.Fs ].uw2;
	if ( i.destw ) pVuMem32 [ 3 ] = v->vf [ i.Fs ].uw3;

	// if writing to TPC, then is should also start VU#1
	if ( !v->Number )
	{
		if ( ( StoreAddress << 2 ) == 0x43a0 )
		{
			if ( i.destx )
			{
				VU1::_VU1->PC = v->vf [ i.Fs ].uw0;
				VU1::_VU1->StartVU ();
			}
			
		}
	}

	v->vi [ i.it & 0xf ].uLo++;

#if defined INLINE_DEBUG_SQI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " SA=" << v->vi [ i.it ].uLo;
#endif
}


// LOAD (to integer) instructions //

void Execute::ILWR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ILWR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Base=" << v->vi [ i.is ].uLo;
#endif

	// ILWR itdest, (is)
	// do Imm11 x16
	
	u32 LoadAddress;
	u32* pVuMem32;

#ifdef ENABLE_STALLS_INT

	// TODO: check if the source register is ready for use or if a stall is needed
	
	// set the source integer register
	v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}

	// integer register load takes 4 cycles
	// integer register is +32 in the bitmap
	v->Set_DestReg_Lower ( ( i.it & 0xf ) + 32 );
#endif

#ifdef ENABLE_INTDELAYSLOT_ILWR_BEFORE
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif
	

	LoadAddress = v->vi [ i.is & 0xf ].uLo << 2;
	
	pVuMem32 = v->GetMemPtr ( LoadAddress );
	
	if ( i.destx ) v->vi [ i.it ].uLo = pVuMem32 [ 0 ];
	if ( i.desty ) v->vi [ i.it ].uLo = pVuMem32 [ 1 ];
	if ( i.destz ) v->vi [ i.it ].uLo = pVuMem32 [ 2 ];
	if ( i.destw ) v->vi [ i.it ].uLo = pVuMem32 [ 3 ];


#if defined INLINE_DEBUG_ILWR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " LA=" << v->vi [ i.is ].uLo;
	debug << " Output:" << " it=" << v->vi [ i.it ].uLo;
#endif
}


void Execute::ILW ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ILW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " Base=" << v->vi [ i.is ].uLo;
#endif

	// ILW itdest, Imm11(is)
	// do Imm11 x16
	
	u32 LoadAddress;
	u32* pVuMem32;
	
#ifdef ENABLE_STALLS_INT

	// TODO: check if the source register is ready for use or if a stall is needed
	
	// set the source integer register
	v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}

	// integer register load takes 4 cycles
	// integer register is +32 in the bitmap
	v->Set_DestReg_Lower ( ( i.it & 0xf ) + 32 );
#endif

#ifdef ENABLE_INTDELAYSLOT_ILW_BEFORE
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif
	
	LoadAddress = ( v->vi [ i.is & 0xf ].sLo + i.Imm11 ) << 2;
	
	pVuMem32 = v->GetMemPtr ( LoadAddress );
	
	if ( i.destx ) v->vi [ i.it ].uLo = pVuMem32 [ 0 ];
	if ( i.desty ) v->vi [ i.it ].uLo = pVuMem32 [ 1 ];
	if ( i.destz ) v->vi [ i.it ].uLo = pVuMem32 [ 2 ];
	if ( i.destw ) v->vi [ i.it ].uLo = pVuMem32 [ 3 ];
	
#if defined INLINE_DEBUG_ILW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " LA=" << ( v->vi [ i.is ].sLo + i.Imm11 );
	debug << " Output:" << " it=" << v->vi [ i.it ].uLo;
#endif
}


// LOAD (to float) instructions //

void Execute::Execute_LoadDelaySlot ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_LD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n(LoadMoveDelaySlot)" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// disable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 0;

	// only perform transfer if the upper instruction does not write to same register as lower instruction
	// also must check the field, because it only cancels if it writes to the same field as upper instruction?? - no, incorrect
	//if ( i.Ft != v->LastModifiedRegister )
	if ( ! ( v->FlagSave [ v->iFlagSave_Index & VU::c_lFlag_Delay_Mask ].Int_Bitmap & ( 1 << i.Ft ) ) )
	{

		if ( i.destx ) v->vf [ i.Ft ].uw0 = v->LoadMoveDelayReg.uw0;
		if ( i.desty ) v->vf [ i.Ft ].uw1 = v->LoadMoveDelayReg.uw1;
		if ( i.destz ) v->vf [ i.Ft ].uw2 = v->LoadMoveDelayReg.uw2;
		if ( i.destw ) v->vf [ i.Ft ].uw3 = v->LoadMoveDelayReg.uw3;
		
#ifdef ENABLE_STALLS

	// must set this AFTER checking if instruction should be cancelled
	
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif
	}
	else
	{
#if defined INLINE_DEBUG_LD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
		debug << " CANCELLED ";
#endif
	}


	// must clear this absolute last
	//v->UpperDest_Bitmap = 0;

#if defined INLINE_DEBUG_LD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " ft(hex)=" << v->vf [ i.Ft ].uw0 << " " << v->vf [ i.Ft ].uw1 << " " << v->vf [ i.Ft ].uw2 << " " << v->vf [ i.Ft ].uw3;
	debug << dec << " ft(dec)=" << v->vf [ i.Ft ].fx << " " << v->vf [ i.Ft ].fy << " " << v->vf [ i.Ft ].fz << " " << v->vf [ i.Ft ].fw;
#endif
}

void Execute::LQ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_LQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " (PRE-DELAY)";
	debug << hex << " is=" << v->vi [ i.is ].uLo;
	debug << hex << " LA=" << ( v->vi [ i.is ].sLo + i.Imm11 );
#endif

	// LQ ftdest, Imm11(is)
	// do Imm11 x16
	
	u32 LoadAddress;
	u32* pVuMem32;
	
#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: don't want to set destination register until after upper instruction is executed!!!
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	LoadAddress = ( v->vi [ i.is & 0xf ].sLo + i.Imm11 ) << 2;
	
#if defined INLINE_DEBUG_LQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " (POST-DELAY)";
	debug << hex << " is=" << v->vi [ i.is ].uLo;
	debug << hex << " LA=" << ( v->vi [ i.is ].sLo + i.Imm11 );
#endif

	pVuMem32 = v->GetMemPtr ( LoadAddress );

	
#ifdef ENABLE_STALLS_LQ
	if ( i.destx ) v->LoadMoveDelayReg.uw0 = pVuMem32 [ 0 ];
	if ( i.desty ) v->LoadMoveDelayReg.uw1 = pVuMem32 [ 1 ];
	if ( i.destz ) v->LoadMoveDelayReg.uw2 = pVuMem32 [ 2 ];
	if ( i.destw ) v->LoadMoveDelayReg.uw3 = pVuMem32 [ 3 ];
	
	// enable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 1;
	
	// put the instruction in the delay slot (for recompiler since it would not be there)
	v->CurInstLOHI.Lo.Value = i.Value;
	
	// clear last modified register to detect if it should be cancelled
	v->LastModifiedRegister = 0;

	// TODO: this should only happen AFTER upper instruction is executed probably!!!
	if (i.destx) v->vf[i.Ft].uw0 = pVuMem32[0];
	if (i.desty) v->vf[i.Ft].uw1 = pVuMem32[1];
	if (i.destz) v->vf[i.Ft].uw2 = pVuMem32[2];
	if (i.destw) v->vf[i.Ft].uw3 = pVuMem32[3];

#else
	// TODO: this should only happen AFTER upper instruction is executed probably!!!
	if ( i.destx ) v->vf [ i.Ft ].uw0 = pVuMem32 [ 0 ];
	if ( i.desty ) v->vf [ i.Ft ].uw1 = pVuMem32 [ 1 ];
	if ( i.destz ) v->vf [ i.Ft ].uw2 = pVuMem32 [ 2 ];
	if ( i.destw ) v->vf [ i.Ft ].uw3 = pVuMem32 [ 3 ];
	v->Set_DestReg_Upper(i.Value, i.Ft);
#endif
	
#if defined INLINE_DEBUG_LQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " ft(hex)=" << v->vf [ i.Ft ].uw0 << " " << v->vf [ i.Ft ].uw1 << " " << v->vf [ i.Ft ].uw2 << " " << v->vf [ i.Ft ].uw3;
	debug << dec << " ft(dec)=" << v->vf [ i.Ft ].fx << " " << v->vf [ i.Ft ].fy << " " << v->vf [ i.Ft ].fz << " " << v->vf [ i.Ft ].fw;
#endif
}

void Execute::LQD ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_LQD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " is=" << v->vi [ i.is ].uLo;
	debug << hex << " LA=" << (v->vi [ i.is ].uLo-1);
#endif

	// LQD ftdest, (--is)
	// do Imm11 x16
	
	u32 LoadAddress;
	u32* pVuMem32;
	
#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: don't want to set destination register until after upper instruction is executed!!!
#endif

	// pre-decrement
#ifdef ENABLE_INTDELAYSLOT_LQD_BEFORE
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

#ifdef USE_NEW_RECOMPILE2_LQD

	// check if int calc result needs to be output to delay slot or not
	if ( v->pLUT_StaticInfo [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 10 ) )
	{
		v->Set_IntDelaySlot ( i.is & 0xf, v->vi [ i.is & 0xf ].uLo - 1 );
		LoadAddress = ( v->vi [ i.is & 0xf ].uLo - 1 ) << 2;
	}
	else
	{
		v->vi [ i.is & 0xf ].uLo--;
		LoadAddress = v->vi [ i.is & 0xf ].uLo << 2;
	}

#else

#ifdef ENABLE_INTDELAYSLOT_LQD_AFTER
	v->Set_IntDelaySlot ( i.is & 0xf, v->vi [ i.is & 0xf ].uLo - 1 );
	LoadAddress = ( v->vi [ i.is & 0xf ].uLo - 1 ) << 2;
#else
	v->vi [ i.is & 0xf ].uLo--;
	
	LoadAddress = v->vi [ i.is & 0xf ].uLo << 2;
#endif

#endif
	
	pVuMem32 = v->GetMemPtr ( LoadAddress );
	
#ifdef ENABLE_STALLS_LQD
	if ( i.destx ) v->LoadMoveDelayReg.uw0 = pVuMem32 [ 0 ];
	if ( i.desty ) v->LoadMoveDelayReg.uw1 = pVuMem32 [ 1 ];
	if ( i.destz ) v->LoadMoveDelayReg.uw2 = pVuMem32 [ 2 ];
	if ( i.destw ) v->LoadMoveDelayReg.uw3 = pVuMem32 [ 3 ];
	
	// enable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 1;
	
	// put the instruction in the delay slot (for recompiler since it would not be there)
	v->CurInstLOHI.Lo.Value = i.Value;
	
	// clear last modified register to detect if it should be cancelled
	v->LastModifiedRegister = 0;
#else
	if ( i.destx ) v->vf [ i.Ft ].uw0 = pVuMem32 [ 0 ];
	if ( i.desty ) v->vf [ i.Ft ].uw1 = pVuMem32 [ 1 ];
	if ( i.destz ) v->vf [ i.Ft ].uw2 = pVuMem32 [ 2 ];
	if ( i.destw ) v->vf [ i.Ft ].uw3 = pVuMem32 [ 3 ];
	v->Set_DestReg_Upper(i.Value, i.Ft);
#endif
	
#if defined INLINE_DEBUG_LQD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " ft=" << v->vf [ i.Ft ].uw0 << " " << v->vf [ i.Ft ].uw1 << " " << v->vf [ i.Ft ].uw2 << " " << v->vf [ i.Ft ].uw3;
#endif
}

void Execute::VLQD ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_LQD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " is=" << v->vi [ i.is ].uLo;
	debug << hex << " LA=" << (v->vi [ i.is ].uLo-1);
#endif

	// LQD ftdest, (--is)
	// do Imm11 x16
	
	u32 LoadAddress;
	u32* pVuMem32;

	v->vi [ i.is & 0xf ].uLo--;
	LoadAddress = v->vi [ i.is & 0xf ].uLo << 2;

	pVuMem32 = v->GetMemPtr ( LoadAddress );

	if ( i.destx ) v->vf [ i.Ft ].uw0 = pVuMem32 [ 0 ];
	if ( i.desty ) v->vf [ i.Ft ].uw1 = pVuMem32 [ 1 ];
	if ( i.destz ) v->vf [ i.Ft ].uw2 = pVuMem32 [ 2 ];
	if ( i.destw ) v->vf [ i.Ft ].uw3 = pVuMem32 [ 3 ];
	
#if defined INLINE_DEBUG_LQD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " ft=" << v->vf [ i.Ft ].uw0 << " " << v->vf [ i.Ft ].uw1 << " " << v->vf [ i.Ft ].uw2 << " " << v->vf [ i.Ft ].uw3;
#endif
}


void Execute::LQI ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_LQI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " is=" << v->vi [ i.is ].uLo;
	debug << hex << " LA=" << v->vi [ i.is ].uLo;
#endif

	// LQI ftdest, (is++)
	// do Imm11 x16
	
	u32 LoadAddress;
	u32* pVuMem32;
	
#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: don't want to set destination register until after upper instruction is executed!!!
#endif

#ifdef ENABLE_INTDELAYSLOT_LQI_BEFORE
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	LoadAddress = v->vi [ i.is & 0xf ].uLo << 2;
	
	pVuMem32 = v->GetMemPtr ( LoadAddress );
	
#ifdef ENABLE_STALLS_LQI
	if ( i.destx ) v->LoadMoveDelayReg.uw0 = pVuMem32 [ 0 ];
	if ( i.desty ) v->LoadMoveDelayReg.uw1 = pVuMem32 [ 1 ];
	if ( i.destz ) v->LoadMoveDelayReg.uw2 = pVuMem32 [ 2 ];
	if ( i.destw ) v->LoadMoveDelayReg.uw3 = pVuMem32 [ 3 ];
	
	// enable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 1;
	
	// put the instruction in the delay slot (for recompiler since it would not be there)
	v->CurInstLOHI.Lo.Value = i.Value;
	
	// clear last modified register to detect if it should be cancelled
	v->LastModifiedRegister = 0;
#else
	if ( i.destx ) v->vf [ i.Ft ].uw0 = pVuMem32 [ 0 ];
	if ( i.desty ) v->vf [ i.Ft ].uw1 = pVuMem32 [ 1 ];
	if ( i.destz ) v->vf [ i.Ft ].uw2 = pVuMem32 [ 2 ];
	if ( i.destw ) v->vf [ i.Ft ].uw3 = pVuMem32 [ 3 ];
	v->Set_DestReg_Upper(i.Value, i.Ft);
#endif


#ifdef USE_NEW_RECOMPILE2_LQI

	// check if int calc result needs to be output to delay slot or not
	if ( v->pLUT_StaticInfo [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 10 ) )
	{
		v->Set_IntDelaySlot ( i.is & 0xf, v->vi [ i.is & 0xf ].uLo + 1 );
	}
	else
	{
		v->vi [ i.is & 0xf ].uLo++;
	}

#else

	// post-increment
#ifdef ENABLE_INTDELAYSLOT_LQI_AFTER
	
	v->Set_IntDelaySlot ( i.is & 0xf, v->vi [ i.is & 0xf ].uLo + 1 );
#else
	v->vi [ i.is & 0xf ].uLo++;
#endif

#endif
	
#if defined INLINE_DEBUG_LQI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " ft(hex)=" << v->vf [ i.Ft ].uw0 << " " << v->vf [ i.Ft ].uw1 << " " << v->vf [ i.Ft ].uw2 << " " << v->vf [ i.Ft ].uw3;
	debug << dec << " ft(dec)=" << v->vf [ i.Ft ].fx << " " << v->vf [ i.Ft ].fy << " " << v->vf [ i.Ft ].fz << " " << v->vf [ i.Ft ].fw;
#endif
}

void Execute::VLQI ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_LQI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " is=" << v->vi [ i.is ].uLo;
	debug << hex << " LA=" << v->vi [ i.is ].uLo;
#endif

	// LQI ftdest, (is++)
	// do Imm11 x16
	
	u32 LoadAddress;
	u32* pVuMem32;

	LoadAddress = v->vi [ i.is & 0xf ].uLo << 2;
	
	pVuMem32 = v->GetMemPtr ( LoadAddress );

	if ( i.destx ) v->vf [ i.Ft ].uw0 = pVuMem32 [ 0 ];
	if ( i.desty ) v->vf [ i.Ft ].uw1 = pVuMem32 [ 1 ];
	if ( i.destz ) v->vf [ i.Ft ].uw2 = pVuMem32 [ 2 ];
	if ( i.destw ) v->vf [ i.Ft ].uw3 = pVuMem32 [ 3 ];

	v->vi [ i.is & 0xf ].uLo++;

#if defined INLINE_DEBUG_LQI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " Output:" << " ft(hex)=" << v->vf [ i.Ft ].uw0 << " " << v->vf [ i.Ft ].uw1 << " " << v->vf [ i.Ft ].uw2 << " " << v->vf [ i.Ft ].uw3;
	debug << dec << " ft(dec)=" << v->vf [ i.Ft ].fx << " " << v->vf [ i.Ft ].fy << " " << v->vf [ i.Ft ].fz << " " << v->vf [ i.Ft ].fw;
#endif
}








// MOVE (to float) instructions //


void Execute::MFP ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MFP || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " vfx=" << hex << v->vf [ i.Fs ].fx << " vfy=" << v->vf [ i.Fs ].fy << " vfz=" << v->vf [ i.Fs ].fz << " vfw=" << v->vf [ i.Fs ].fw;
	debug << " P=" << hex << v->vi [ VU::REG_P ].u << " " << dec << v->vi [ VU::REG_P ].f;
	debug << hex << " NextP=" << v->NextP.l << " (float)" << v->NextP.f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif

	// dest ft = P
	
	// the MFP instruction does NOT wait until the EFU unit is done executing
	// also need to check timing of EFU instructions
	/*
#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	// note: must do this AFTER upper instruction is executed if it does not cancel the instruction
	//v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif
	*/
	
#ifdef ENABLE_STALLS_MFP
	// need to make sure P register is updated properly first
	v->UpdateP ();
	
	if ( i.destx ) v->LoadMoveDelayReg.uw0 = v->vi [ VU::REG_P ].u;
	if ( i.desty ) v->LoadMoveDelayReg.uw1 = v->vi [ VU::REG_P ].u;
	if ( i.destz ) v->LoadMoveDelayReg.uw2 = v->vi [ VU::REG_P ].u;
	if ( i.destw ) v->LoadMoveDelayReg.uw3 = v->vi [ VU::REG_P ].u;
	
	// enable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 1;
	
	// put the instruction in the delay slot (for recompiler since it would not be there)
	v->CurInstLOHI.Lo.Value = i.Value;
	
	// clear last modified register to detect if it should be cancelled
	v->LastModifiedRegister = 0;
#else

	// update p
	v->UpdateP_Micro ();

	// TODO: this should only store to the float registers AFTER upper instruction has been executed probably!!!
	// but only if the instruction does not get cancelled by upper instruction
	if ( i.destx ) v->vf [ i.Ft ].uw0 = v->vi [ VU::REG_P ].u;
	if ( i.desty ) v->vf [ i.Ft ].uw1 = v->vi [ VU::REG_P ].u;
	if ( i.destz ) v->vf [ i.Ft ].uw2 = v->vi [ VU::REG_P ].u;
	if ( i.destw ) v->vf [ i.Ft ].uw3 = v->vi [ VU::REG_P ].u;
	v->Set_DestReg_Upper(i.Value, i.Ft);
#endif
	
	// flags affected: none

#if defined INLINE_DEBUG_MFP || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << hex << v->vf [ i.Ft ].fx << " vfy=" << v->vf [ i.Ft ].fy << " vfz=" << v->vf [ i.Ft ].fz << " vfw=" << v->vf [ i.Ft ].fw;
#endif
}

void Execute::MOVE ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MOVE || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	if ( i.Fs || i.Ft )
	debug << " vfx=" << hex << v->vf [ i.Fs ].fx << " vfy=" << v->vf [ i.Fs ].fy << " vfz=" << v->vf [ i.Fs ].fz << " vfw=" << v->vf [ i.Fs ].fw;
#endif

#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( i.Value, i.Fs );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	// note: must do this AFTER upper instruction is executed if it does not cancel the instruction
	//v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif


	// dest ft = fs

#ifdef USE_NEW_RECOMPILE2_MOVE

	if ( v->pStaticInfo[v->Number] [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 5 ) )
	{
	if ( i.destx ) v->LoadMoveDelayReg.uw0 = v->vf [ i.Fs ].uw0; else v->LoadMoveDelayReg.uw0 = v->vf [ i.Ft ].uw0;
	if ( i.desty ) v->LoadMoveDelayReg.uw1 = v->vf [ i.Fs ].uw1; else v->LoadMoveDelayReg.uw1 = v->vf [ i.Ft ].uw1;
	if ( i.destz ) v->LoadMoveDelayReg.uw2 = v->vf [ i.Fs ].uw2; else v->LoadMoveDelayReg.uw2 = v->vf [ i.Ft ].uw2;
	if ( i.destw ) v->LoadMoveDelayReg.uw3 = v->vf [ i.Fs ].uw3; else v->LoadMoveDelayReg.uw3 = v->vf [ i.Ft ].uw3;

	// enable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 1;

	// put the instruction in the delay slot (for recompiler since it would not be there)
	v->CurInstLOHI.Lo.Value = i.Value;
	}
	else
	{
	if ( i.destx ) v->vf [ i.Ft ].uw0 = v->vf [ i.Fs ].uw0;
	if ( i.desty ) v->vf [ i.Ft ].uw1 = v->vf [ i.Fs ].uw1;
	if ( i.destz ) v->vf [ i.Ft ].uw2 = v->vf [ i.Fs ].uw2;
	if ( i.destw ) v->vf [ i.Ft ].uw3 = v->vf [ i.Fs ].uw3;
	v->Set_DestReg_Upper(i.Value, i.Ft);
	}

#else

#ifdef ENABLE_STALLS_MOVE
	if ( i.destx ) v->LoadMoveDelayReg.uw0 = v->vf [ i.Fs ].uw0; else v->LoadMoveDelayReg.uw0 = v->vf [ i.Ft ].uw0;
	if ( i.desty ) v->LoadMoveDelayReg.uw1 = v->vf [ i.Fs ].uw1; else v->LoadMoveDelayReg.uw1 = v->vf [ i.Ft ].uw1;
	if ( i.destz ) v->LoadMoveDelayReg.uw2 = v->vf [ i.Fs ].uw2; else v->LoadMoveDelayReg.uw2 = v->vf [ i.Ft ].uw2;
	if ( i.destw ) v->LoadMoveDelayReg.uw3 = v->vf [ i.Fs ].uw3; else v->LoadMoveDelayReg.uw3 = v->vf [ i.Ft ].uw3;
	
	// enable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 1;
	
	// put the instruction in the delay slot (for recompiler since it would not be there)
	v->CurInstLOHI.Lo.Value = i.Value;
	
	// clear last modified register to detect if it should be cancelled
	v->LastModifiedRegister = 0;
#else
	// TODO: this needs to write the register into a temporary register and execute the instruction after upper instruction
	// but only if the instruction does not get cancelled by upper instruction
	if ( i.destx ) v->vf [ i.Ft ].uw0 = v->vf [ i.Fs ].uw0;
	if ( i.desty ) v->vf [ i.Ft ].uw1 = v->vf [ i.Fs ].uw1;
	if ( i.destz ) v->vf [ i.Ft ].uw2 = v->vf [ i.Fs ].uw2;
	if ( i.destw ) v->vf [ i.Ft ].uw3 = v->vf [ i.Fs ].uw3;
#endif

#endif
	
	// flags affected: none

#if defined INLINE_DEBUG_MOVE || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	if ( i.Fs || i.Ft )
	debug << " Output: Ft=" << " vfx=" << hex << v->vf [ i.Ft ].fx << " vfy=" << v->vf [ i.Ft ].fy << " vfz=" << v->vf [ i.Ft ].fz << " vfw=" << v->vf [ i.Ft ].fw;
#endif
}


void Execute::VMOVE ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MOVE || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	if ( i.Fs || i.Ft )
	debug << " vfx=" << hex << v->vf [ i.Fs ].fx << " vfy=" << v->vf [ i.Fs ].fy << " vfz=" << v->vf [ i.Fs ].fz << " vfw=" << v->vf [ i.Fs ].fw;
#endif

	if ( i.destx ) v->LoadMoveDelayReg.uw0 = v->vf [ i.Fs ].uw0; else v->LoadMoveDelayReg.uw0 = v->vf [ i.Ft ].uw0;
	if ( i.desty ) v->LoadMoveDelayReg.uw1 = v->vf [ i.Fs ].uw1; else v->LoadMoveDelayReg.uw1 = v->vf [ i.Ft ].uw1;
	if ( i.destz ) v->LoadMoveDelayReg.uw2 = v->vf [ i.Fs ].uw2; else v->LoadMoveDelayReg.uw2 = v->vf [ i.Ft ].uw2;
	if ( i.destw ) v->LoadMoveDelayReg.uw3 = v->vf [ i.Fs ].uw3; else v->LoadMoveDelayReg.uw3 = v->vf [ i.Ft ].uw3;
	
	// enable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 1;
	
	// put the instruction in the delay slot (for recompiler since it would not be there)
	v->CurInstLOHI.Lo.Value = i.Value;
	
	// clear last modified register to detect if it should be cancelled
	v->LastModifiedRegister = 0;


	// flags affected: none

#if defined INLINE_DEBUG_MOVE || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	if ( i.Fs || i.Ft )
	debug << " Output: Ft=" << " vfx=" << hex << v->vf [ i.Ft ].fx << " vfy=" << v->vf [ i.Ft ].fy << " vfz=" << v->vf [ i.Ft ].fz << " vfw=" << v->vf [ i.Ft ].fw;
#endif
}

void Execute::MR32 ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MOVE || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " vfx=" << hex << v->vf [ i.Fs ].fx << " vfy=" << v->vf [ i.Fs ].fy << " vfz=" << v->vf [ i.Fs ].fz << " vfw=" << v->vf [ i.Fs ].fw;
#endif

	// dest ft = fs
	
	u32 temp;
	
#ifdef ENABLE_STALLS
	// set the source register(s)
	v->Set_SrcReg ( ( ( i.Value << 1 ) & ( 0xe << 21 ) ) | ( ( i.Value >> 3 ) & ( 1 << 21 ) ), i.Fs );
	//v->Set_SrcReg ( ( ( i.Value >> 1 ) & ( 0x7 << 21 ) ) | ( ( i.Value << 3 ) & ( 0x8 << 21 ) ), i.Fs );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	// note: this should only happen AFTER the upper instruction has executed
	//v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif

	// TODO: this should only store to the float registers AFTER upper instruction has been executed probably!!!
	// but only if the instruction does not get cancelled by upper instruction
	
	// must do this or data can get overwritten
	temp = v->vf [ i.Fs ].ux;

#ifdef USE_NEW_RECOMPILE2_MR32

	if ( v->pStaticInfo[v->Number] [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 5 ) )
	{
	if ( i.destx ) v->LoadMoveDelayReg.uw0 = v->vf [ i.Fs ].uy; else v->LoadMoveDelayReg.uw0 = v->vf [ i.Ft ].uw0;
	if ( i.desty ) v->LoadMoveDelayReg.uw1 = v->vf [ i.Fs ].uz; else v->LoadMoveDelayReg.uw1 = v->vf [ i.Ft ].uw1;
	if ( i.destz ) v->LoadMoveDelayReg.uw2 = v->vf [ i.Fs ].uw; else v->LoadMoveDelayReg.uw2 = v->vf [ i.Ft ].uw2;
	if ( i.destw ) v->LoadMoveDelayReg.uw3 = temp; else v->LoadMoveDelayReg.uw3 = v->vf [ i.Ft ].uw3;

	// enable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 1;

	// put the instruction in the delay slot (for recompiler since it would not be there)
	v->CurInstLOHI.Lo.Value = i.Value;
	}
	else
	{
	if ( i.destx ) v->vf [ i.Ft ].ux = v->vf [ i.Fs ].uy;
	if ( i.desty ) v->vf [ i.Ft ].uy = v->vf [ i.Fs ].uz;
	if ( i.destz ) v->vf [ i.Ft ].uz = v->vf [ i.Fs ].uw;
	if ( i.destw ) v->vf [ i.Ft ].uw = temp;
	v->Set_DestReg_Upper(i.Value, i.Ft);
	}

#else

#ifdef ENABLE_STALLS_MR32
	if ( i.destx ) v->LoadMoveDelayReg.uw0 = v->vf [ i.Fs ].uy; else v->LoadMoveDelayReg.uw0 = v->vf [ i.Ft ].uw0;
	if ( i.desty ) v->LoadMoveDelayReg.uw1 = v->vf [ i.Fs ].uz; else v->LoadMoveDelayReg.uw1 = v->vf [ i.Ft ].uw1;
	if ( i.destz ) v->LoadMoveDelayReg.uw2 = v->vf [ i.Fs ].uw; else v->LoadMoveDelayReg.uw2 = v->vf [ i.Ft ].uw2;
	if ( i.destw ) v->LoadMoveDelayReg.uw3 = temp; else v->LoadMoveDelayReg.uw3 = v->vf [ i.Ft ].uw3;
	
	// enable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 1;
	
	// put the instruction in the delay slot (for recompiler since it would not be there)
	v->CurInstLOHI.Lo.Value = i.Value;
	
	// clear last modified register to detect if it should be cancelled
	v->LastModifiedRegister = 0;
#else
	if ( i.destx ) v->vf [ i.Ft ].ux = v->vf [ i.Fs ].uy;
	if ( i.desty ) v->vf [ i.Ft ].uy = v->vf [ i.Fs ].uz;
	if ( i.destz ) v->vf [ i.Ft ].uz = v->vf [ i.Fs ].uw;
	if ( i.destw ) v->vf [ i.Ft ].uw = temp;
#endif

#endif
	
	// flags affected: none

#if defined INLINE_DEBUG_MOVE || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << hex << v->vf [ i.Ft ].fx << " vfy=" << v->vf [ i.Ft ].fy << " vfz=" << v->vf [ i.Ft ].fz << " vfw=" << v->vf [ i.Ft ].fw;
#endif
}

void Execute::VMR32 ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MOVE || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " vfx=" << hex << v->vf [ i.Fs ].fx << " vfy=" << v->vf [ i.Fs ].fy << " vfz=" << v->vf [ i.Fs ].fz << " vfw=" << v->vf [ i.Fs ].fw;
#endif

	u32 temp;

	// must do this or data can get overwritten
	temp = v->vf [ i.Fs ].ux;

	if ( i.destx ) v->LoadMoveDelayReg.uw0 = v->vf [ i.Fs ].uy; else v->LoadMoveDelayReg.uw0 = v->vf [ i.Ft ].uw0;
	if ( i.desty ) v->LoadMoveDelayReg.uw1 = v->vf [ i.Fs ].uz; else v->LoadMoveDelayReg.uw1 = v->vf [ i.Ft ].uw1;
	if ( i.destz ) v->LoadMoveDelayReg.uw2 = v->vf [ i.Fs ].uw; else v->LoadMoveDelayReg.uw2 = v->vf [ i.Ft ].uw2;
	if ( i.destw ) v->LoadMoveDelayReg.uw3 = temp; else v->LoadMoveDelayReg.uw3 = v->vf [ i.Ft ].uw3;
	
	// enable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 1;
	
	// put the instruction in the delay slot (for recompiler since it would not be there)
	v->CurInstLOHI.Lo.Value = i.Value;
	
	// clear last modified register to detect if it should be cancelled
	v->LastModifiedRegister = 0;


	// flags affected: none

#if defined INLINE_DEBUG_MOVE || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << hex << v->vf [ i.Ft ].fx << " vfy=" << v->vf [ i.Ft ].fy << " vfz=" << v->vf [ i.Ft ].fz << " vfw=" << v->vf [ i.Ft ].fw;
#endif
}

void Execute::MFIR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MFIR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " vi=" << hex << v->vi [ i.is ].uLo;
#endif

	// dest ft = is
	
	// TODO: this should only store to the float registers AFTER upper instruction has been executed probably!!!
	// but only if the instruction does not get cancelled by upper instruction


#ifdef ENABLE_STALLS
	// set the source integer register
	v->Set_Int_SrcReg ( i.is + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
#endif


#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	
#ifdef ENABLE_STALLS_MFIR
	if ( i.destx ) v->LoadMoveDelayReg.sw0 = (s32) v->vi [ i.is ].sLo;
	if ( i.desty ) v->LoadMoveDelayReg.sw1 = (s32) v->vi [ i.is ].sLo;
	if ( i.destz ) v->LoadMoveDelayReg.sw2 = (s32) v->vi [ i.is ].sLo;
	if ( i.destw ) v->LoadMoveDelayReg.sw3 = (s32) v->vi [ i.is ].sLo;
	
	// enable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 1;
	
	// put the instruction in the delay slot (for recompiler since it would not be there)
	v->CurInstLOHI.Lo.Value = i.Value;
	
	// clear last modified register to detect if it should be cancelled
	v->LastModifiedRegister = 0;
#else
	// TODO: destination register should only be set AFTER upper instruction has executed
	if ( i.destx ) v->vf [ i.Ft ].sw0 = (s32) v->vi [ i.is ].sLo;
	if ( i.desty ) v->vf [ i.Ft ].sw1 = (s32) v->vi [ i.is ].sLo;
	if ( i.destz ) v->vf [ i.Ft ].sw2 = (s32) v->vi [ i.is ].sLo;
	if ( i.destw ) v->vf [ i.Ft ].sw3 = (s32) v->vi [ i.is ].sLo;
	v->Set_DestReg_Upper(i.Value, i.Ft);
#endif
	
	// flags affected: none

#if defined INLINE_DEBUG_MFIR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << hex << v->vf [ i.Ft ].sw0 << " vfy=" << v->vf [ i.Ft ].sw1 << " vfz=" << v->vf [ i.Ft ].sw2 << " vfw=" << v->vf [ i.Ft ].sw3;
#endif
}


// MOVE (to integer) instructions //

void Execute::MTIR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MTIR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << v->vf [ i.Fs ].vuw [ i.fsf ];
#endif

	// fsf it = fs

#ifdef ENABLE_STALLS
	// set the source register(s)
	//v->Set_SrcReg ( i.Value, i.Fs );
	v->Set_SrcRegBC ( i.Value, i.Fs );
	
	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
#endif

	
	// todo: determine if integer register can be used by branch immediately or if you need int delay slot
#ifdef USE_NEW_RECOMPILE2_INTCALC

	// check if int calc result needs to be output to delay slot or not
	if ( v->pLUT_StaticInfo [ ( v->PC & v->ulVuMem_Mask ) >> 3 ] & ( 1 << 10 ) )
	{
#if defined INLINE_DEBUG_IADDI || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT
	debug << ">INT-DELAY-SLOT";
#endif
		v->Set_IntDelaySlot ( i.it & 0xf, (u16) v->vf [ i.Fs ].vuw [ i.fsf ] );
	}
	else
	{
		v->vi [ i.it & 0xf ].uLo = (u16) v->vf [ i.Fs ].vuw [ i.fsf ];
	}

#else

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
	
	v->Set_IntDelaySlot ( i.it & 0xf, (u16) v->vf [ i.Fs ].vuw [ i.fsf ] );
#else
	// note: this should be ok, since this is a lower instruction and integer instructions are lower instructions
	// note: this instruction happens immediately, with NO stall possible
	v->vi [ i.it ].uLo = (u16) v->vf [ i.Fs ].vuw [ i.fsf ];
#endif

#endif

	// flags affected: none

#if defined INLINE_DEBUG_MTIR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: it=" << hex << v->vi [ i.it ].uLo;
#endif
}

void Execute::VMTIR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_MTIR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs=" << hex << v->vf [ i.Fs ].vuw [ i.fsf ];
#endif

		v->vi [ i.it & 0xf ].uLo = (u16) v->vf [ i.Fs ].vuw [ i.fsf ];

#if defined INLINE_DEBUG_MTIR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: it=" << hex << v->vi [ i.it ].uLo;
#endif
}



// Random Number instructions //

void Execute::RGET ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_RGET || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " R=" << hex << v->vi [ VU::REG_R ].u;
#endif


#ifdef ENABLE_STALLS
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	// destination register actually gets set after the move
	//v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif

#ifdef ENABLE_STALLS_RGET
	if ( i.destx ) v->LoadMoveDelayReg.uw0 = v->vi [ VU::REG_R ].u;
	if ( i.desty ) v->LoadMoveDelayReg.uw1 = v->vi [ VU::REG_R ].u;
	if ( i.destz ) v->LoadMoveDelayReg.uw2 = v->vi [ VU::REG_R ].u;
	if ( i.destw ) v->LoadMoveDelayReg.uw3 = v->vi [ VU::REG_R ].u;
	
	// enable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 1;
	
	// put the instruction in the delay slot (for recompiler since it would not be there)
	v->CurInstLOHI.Lo.Value = i.Value;
	
	// clear last modified register to detect if it should be cancelled
	v->LastModifiedRegister = 0;
#else
	// TODO: destination register should only be set AFTER upper instruction has executed
	if ( i.destx ) v->vf [ i.Ft ].uw0 = v->vi [ VU::REG_R ].u;
	if ( i.desty ) v->vf [ i.Ft ].uw1 = v->vi [ VU::REG_R ].u;
	if ( i.destz ) v->vf [ i.Ft ].uw2 = v->vi [ VU::REG_R ].u;
	if ( i.destw ) v->vf [ i.Ft ].uw3 = v->vi [ VU::REG_R ].u;
	v->Set_DestReg_Upper(i.Value, i.Ft);
#endif

	
	// flags affected: none

#if defined INLINE_DEBUG_RGET || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << hex << v->vf [ i.Ft ].fx << " vfy=" << v->vf [ i.Ft ].fy << " vfz=" << v->vf [ i.Ft ].fz << " vfw=" << v->vf [ i.Ft ].fw;
#endif
}

void Execute::RNEXT ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_RNEXT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	//cout << "\nhps2x64: ERROR: VU: Instruction not implemented: RNEXT";

#ifdef ENABLE_STALLS
	
	// todo: can set the MAC and STATUS flags registers as being modified also if needed later
	// set the destination register(s)
	// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
	// destination register actually gets set after the move
	//v->Set_DestReg_Upper ( i.Value, i.Ft );
#endif

	static const unsigned long c_ulRandMask = 0x7ffb18;

	unsigned long bit;
	unsigned long reg;
	
	reg = v->vi [ VU::REG_R ].u;
	//bit = __builtin_popcount ( reg & c_ulRandMask ) & 1;
	bit = popcnt32(reg & c_ulRandMask) & 1;
	v->vi [ VU::REG_R ].u = ( 0x7f << 23 ) | ( ( reg << 1 ) & 0x007fffff ) | bit;
	
	
#ifdef ENABLE_STALLS_RNEXT
	if ( i.destx ) v->LoadMoveDelayReg.uw0 = v->vi [ VU::REG_R ].u;
	if ( i.desty ) v->LoadMoveDelayReg.uw1 = v->vi [ VU::REG_R ].u;
	if ( i.destz ) v->LoadMoveDelayReg.uw2 = v->vi [ VU::REG_R ].u;
	if ( i.destw ) v->LoadMoveDelayReg.uw3 = v->vi [ VU::REG_R ].u;
	
	// enable the quick delay slot
	v->Status.EnableLoadMoveDelaySlot = 1;
	
	// put the instruction in the delay slot (for recompiler since it would not be there)
	v->CurInstLOHI.Lo.Value = i.Value;
	
	// clear last modified register to detect if it should be cancelled
	v->LastModifiedRegister = 0;
#else
	// TODO: destination register should only be set AFTER upper instruction has executed
	if ( i.destx ) v->vf [ i.Ft ].uw0 = v->vi [ VU::REG_R ].u;
	if ( i.desty ) v->vf [ i.Ft ].uw1 = v->vi [ VU::REG_R ].u;
	if ( i.destz ) v->vf [ i.Ft ].uw2 = v->vi [ VU::REG_R ].u;
	if ( i.destw ) v->vf [ i.Ft ].uw3 = v->vi [ VU::REG_R ].u;
	v->Set_DestReg_Upper(i.Value, i.Ft);
#endif

	
#if defined INLINE_DEBUG_RNEXT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: Ft=" << " vfx=" << hex << v->vf [ i.Ft ].fx << " vfy=" << v->vf [ i.Ft ].fy << " vfz=" << v->vf [ i.Ft ].fz << " vfw=" << v->vf [ i.Ft ].fw;
	debug << " R=" << hex << v->vi [ VU::REG_R ].u;
#endif
}

void Execute::RINIT ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_RINIT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fsfsf=" << hex << v->vf [ i.Fs ].vuw [ i.fsf ];
#endif

	//cout << "\nhps2x64: ERROR: VU: Instruction not implemented: RINIT";
	
	v->vi [ VU::REG_R ].u = ( 0x7f << 23 ) | ( v->vf [ i.Fs ].vuw [ i.fsf ] & 0x7fffff );
	
	// seed random number generator for now
	//srand ( v->vi [ VU::REG_R ].u );

#if defined INLINE_DEBUG_RINIT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: R=" << hex << v->vi [ VU::REG_R ].u;
#endif
}

void Execute::RXOR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_RXOR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fsfsf=" << hex << v->vf [ i.Fs ].vuw [ i.fsf ];
	debug << " R=" << hex << v->vi [ VU::REG_R ].u;
#endif

	//cout << "\nhps2x64: ERROR: VU: Instruction not implemented: RXOR";
	
	v->vi [ VU::REG_R ].u = ( 0x7f << 23 ) | ( ( v->vf [ i.Fs ].vuw [ i.fsf ] ^ v->vi [ VU::REG_R ].u ) & 0x7fffff );
	
#if defined INLINE_DEBUG_RXOR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " Output: R=" << hex << v->vi [ VU::REG_R ].u;
#endif
}










// X instructions //

void Execute::XGKICK ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_XGKICK || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

#ifdef ENABLE_STALLS_INT

	// set the source integer register
	v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
	
	// make sure the source registers are available
	if ( v->TestStall_INT () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " INT-STALL ";
#endif
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
	
	// note: don't want to set destination register until after upper instruction is executed!!!
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

#if defined INLINE_DEBUG_XGKICK || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " vi=" << hex << v->vi [ i.is ].uLo;
#endif

#ifdef ENABLE_XGKICK_WAIT
	// if another path is in progress, then don't start transfer yet
	if ( ( GPU::_GPU->GIFRegs.STAT.APATH ) && ( GPU::_GPU->GIFRegs.STAT.APATH != 1 ) )
	{
		// make sure path 3 is not in an interruptible image transfer
		if ( ( GPU::_GPU->GIFRegs.STAT.APATH != 3 ) || ( !GPU::_GPU->GIFRegs.STAT.IMT ) || ( GPU::_GPU->GIFTag0 [ 3 ].FLG != 2 ) )
		{
			// path 1 in queue
			GPU::_GPU->GIFRegs.STAT.P1Q = 1;
			
			// waiting
			v->Status.XgKick_Wait = 1;
			
			// wait until transfer via the other path is complete
			//v->NextPC = v->PC;
			
			// ??
			v->XgKick_Address = v->vi [ i.is & 0xf ].uLo;
			return;
		}
	}
#endif

#ifdef ENABLE_XGKICK_TIMING

	if ( v->bXgKickActive )
	{
		v->Flush_XgKick ();
	}

	// clear the path 1 count before start writing block
	GPU::_GPU->ulTransferCount [ 1 ] = 0;

	v->ullXgKickCycle = v->CycleCount;
	v->bXgKickActive = 1;

#else

	// if there is an xgkick instruction in progress, then need to complete it immediately
	if ( v->Status.XgKickDelay_Valid )
	{
		// looks like this is only supposed to write one 128-bit value to PATH1
		// no, this actually writes an entire gif packet to path1
		// the address should only be a maximum of 10-bits, so must mask
		//GPU::Path1_WriteBlock ( v->VuMem64, v->XgKick_Address & 0x3ff );
		v->Execute_XgKick ();
		
		// the previous xgkick is done with
		//v->Status.XgKickDelay_Valid = 0;
	}
	
	// now transferring via path 1
	// but, not if running from another thread or putting data into a buffer
	// probably best to comment this one out for now
	if ( !GPU::_GPU->GIFRegs.STAT.APATH )
	{
		GPU::_GPU->GIFRegs.STAT.APATH = 1;
	}
	else if ( GPU::_GPU->GIFRegs.STAT.APATH )
	{
#ifdef ENABLE_APATH_ALERTS
		//if ( GPU::_GPU->GIFRegs.STAT.APATH == 1 )
		//{
		//	cout << "\nALERT: PATH1 running while PATH1 is already running\n";
		//}
		//else
		if ( GPU::_GPU->GIFRegs.STAT.APATH == 2 )
		{
			cout << "\nALERT: PATH1 running while PATH2 is already running\n";
		}
		else if ( GPU::_GPU->GIFRegs.STAT.APATH == 3 )
		{
			if ( ( !GPU::_GPU->GIFRegs.STAT.IMT ) || ( GPU::_GPU->GIFTag0 [ 3 ].FLG != 2 ) )
			{
				cout << "\nALERT: PATH1 running while PATH3 is already running (IMT=" << GPU::_GPU->GIFRegs.STAT.IMT << " FLG=" << GPU::_GPU->GIFTag0 [ 3 ].FLG << ")\n";
			}
		}
#endif
	}

	// need to execute xgkick instruction, but it doesn't execute completely immediately and vu keeps going
	// so for now will delay execution for an instruction or two
	v->Status.XgKickDelay_Valid = 0x2;
#endif

	// no longer waiting
	v->Status.XgKick_Wait = 0;
	
	// path 1 no longer in queue
	GPU::_GPU->GIFRegs.STAT.P1Q = 0;

	v->XgKick_Address = v->vi [ i.is & 0xf ].uLo;
}


void Execute::XTOP ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_XTOP || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " TOP(hex)=" << v->iVifRegs.TOP;
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	// it = TOP
	// this instruction is VU1 only
	v->vi [ i.it & 0xf ].uLo = v->iVifRegs.TOP & 0x3ff;

#if defined INLINE_DEBUG_XTOP || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " it(hex)=" << v->vi [ i.it ].uLo;
#endif
}

void Execute::XITOP ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_XITOP || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << hex << " ITOP(hex)=" << v->iVifRegs.ITOP;
#endif

#ifdef ENABLE_INTDELAYSLOT
	// execute int delay slot immediately
	v->Execute_IntDelaySlot ();
#endif

	if ( !v->Number )
	{
		// VU0 //
		v->vi [ i.it & 0xf ].uLo = v->iVifRegs.ITOP & 0xff;
	}
	else
	{
		// VU1 //
		v->vi [ i.it & 0xf ].uLo = v->iVifRegs.ITOP & 0x3ff;
	}
	
#if defined INLINE_DEBUG_XITOP || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " it(hex)=" << v->vi [ i.it ].uLo;
#endif
}





// WAIT instructions //

void Execute::WAITP ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_WAITP || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// TODO: still need to update cycle count
	
#ifdef ENABLE_STALLS

#ifdef ENABLE_NEW_QP_HANDLING
	// if unit is in use, wait until it is free
	v->WaitP ();
#else
	//if ( v->CycleCount < ( v->PBusyUntil_Cycle ) )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// div/rsqrt/sqrt unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until efu unit is done with what it is doing
		v->PipelineWaitP ();
	}
	v->SetP ();
#endif

#endif

#ifdef USE_NEW_RECOMPILE2_WAITP
	// wait p
	v->WaitP_Micro ();
#endif

}

void Execute::WAITQ ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_WAITQ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
#endif

	// TODO: still need to update cycle count
	
#ifdef ENABLE_STALLS

#ifdef ENABLE_NEW_QP_HANDLING
	// if div/unit is in use, wait until it is free
	v->WaitQ ();
#else
	// check if Q has already been set properly
	if ( v->QBusyUntil_Cycle != -1LL )
	{
		if ( v->CycleCount < v->QBusyUntil_Cycle )
		{
			// div/rsqrt/sqrt unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
			
			// wait until div unit is done with what it is doing
			v->PipelineWaitQ ();
		}
	}
	
#endif

#endif

#ifdef USE_NEW_RECOMPILE2_WAITQ
	// wait q
	v->WaitQ_Micro ();
#endif

}



// lower float math //

void Execute::DIV ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_DIV || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << dec << " fs=" << ( (float&) v->vf [ i.Fs ].vuw [ i.fsf ] );
	debug << dec << " ft=" << ( (float&) v->vf [ i.Ft ].vuw [ i.ftf ] );
#endif

	// todo: update cycle count when div has to wait for a previous div (where Q=NextQ)

	static const u64 c_CycleTime = 7;

	float fs;
	float ft;
	
#ifdef ENABLE_STALLS
	// TODO: need to wait for source register(s) to be ready
	v->Set_SrcRegBC ( i.ftf, i.Ft );
	v->Add_SrcRegBC ( i.fsf, i.Fs );

	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}


#ifdef ENABLE_NEW_QP_HANDLING
	// if div/unit is in use, wait until it is free
	v->WaitQ ();
#else
	if ( v->QBusyUntil_Cycle != -1LL )
	{
	// if the div/rsqrt/sqrt unit is still running, then need to wait until it finishes first
	if ( v->CycleCount < v->QBusyUntil_Cycle )
	{
		// div/rsqrt/sqrt unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " Q-STALL ";
#endif
		
		// wait until div unit is done with what it is doing
		v->PipelineWaitQ ();
	}
	}
#endif

#endif
	
#ifdef USE_NEW_RECOMPILE2_WAITQ
	// wait q
	v->WaitQ_Micro ();
#endif
	
	fs = (float&) v->vf [ i.Fs ].vuw [ i.fsf ];
	ft = (float&) v->vf [ i.Ft ].vuw [ i.ftf ];
	
	//v->vi [ 22 ].f = PS2_Float_Div ( fs, ft, & v->vi [ 16 ].sLo );
	v->NextQ.f = PS2_Float_Div ( fs, ft, & v->NextQ_Flag );
	v->QBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_DIV || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << dec << " Q=" << v->vi [ 22 ].f;
	debug << dec << " NextQ=" << v->NextQ.f;
#endif
}

void Execute::RSQRT ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_RSQRT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << dec << " fs=" << ( (float&) v->vf [ i.Fs ].vuw [ i.fsf ] );
	debug << dec << " ft=" << ( (float&) v->vf [ i.Ft ].vuw [ i.ftf ] );
#endif

	static const u64 c_CycleTime = 13;

	float fs;
	float ft;
	
#ifdef ENABLE_STALLS
	// TODO: need to wait for source register(s) to be ready
	v->Set_SrcRegBC ( i.ftf, i.Ft );
	v->Add_SrcRegBC ( i.fsf, i.Fs );

	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
#ifdef ENABLE_NEW_QP_HANDLING
	// if div/unit is in use, wait until it is free
	v->WaitQ ();
#else
	if ( v->QBusyUntil_Cycle != -1LL )
	{
	// if the div/rsqrt/sqrt unit is still running, then need to wait until it finishes first
	if ( ( (s64)v->CycleCount ) < ( (s64)v->QBusyUntil_Cycle ) )
	{
		// div/rsqrt/sqrt unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until div unit is done with what it is doing
		v->PipelineWaitQ ();
	}
	}
#endif

#endif
	
#ifdef USE_NEW_RECOMPILE2_WAITQ
	// wait q
	v->WaitQ_Micro ();
#endif
	
	fs = (float&) v->vf [ i.Fs ].vuw [ i.fsf ];
	ft = (float&) v->vf [ i.Ft ].vuw [ i.ftf ];
	
	//v->vi [ 22 ].f = PS2_Float_RSqrt ( fs, ft, & v->vi [ 16 ].sLo );
	v->NextQ.f = PS2_Float_RSqrt ( fs, ft, & v->NextQ_Flag );
	v->QBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_RSQRT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << dec << " Q=" << v->vi [ 22 ].f;
	debug << dec << " NextQ=" << v->NextQ.f;
#endif
}

void Execute::SQRT ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_SQRT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << dec << " ft=" << ( (float&) v->vf [ i.Ft ].vuw [ i.ftf ] );
#endif

	static const u64 c_CycleTime = 7;

	float ft;
	
#ifdef ENABLE_STALLS
	// TODO: need to wait for source register(s) to be ready
	v->Set_SrcRegBC ( i.ftf, i.Ft );

	// make sure the source registers are available
	if ( v->TestStall () )
	{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
	
#ifdef ENABLE_NEW_QP_HANDLING
	// if div/unit is in use, wait until it is free
	v->WaitQ ();
#else
	if ( v->QBusyUntil_Cycle != -1LL )
	{
	// if the div/rsqrt/sqrt unit is still running, then need to wait until it finishes first
	if ( ( (s64) v->CycleCount ) < ( (s64) v->QBusyUntil_Cycle ) )
	{
		// div/rsqrt/sqrt unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until div unit is done with what it is doing
		v->PipelineWaitQ ();
	}
	}
#endif

#endif
	
#ifdef USE_NEW_RECOMPILE2_WAITQ
	// wait q
	v->WaitQ_Micro ();
#endif
	
	ft = (float&) v->vf [ i.Ft ].vuw [ i.ftf ];
	
	//v->vi [ 22 ].f = PS2_Float_Sqrt ( ft, & v->vi [ 16 ].sLo );
	v->NextQ.f = PS2_Float_Sqrt ( ft, & v->NextQ_Flag );
	v->QBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_SQRT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_INT	// || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << dec << " Q=" << v->vi [ 22 ].f;
	debug << dec << " NextQ=" << v->NextQ.f;
#endif
}




// External unit //

void Execute::EATAN ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_EATAN || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << hex << " (before)NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif

	// 53/54 cycles
	static const u64 c_CycleTime = 53;	//54;
	
	// I'll make c0 = pi/4
	static const long c_fC0 = 0x3f490fdb;
	
	static const long c_fC1 = 0x3f7ffff5;
	static const long c_fC2 = 0xbeaaa61c;
	static const long c_fC3 = 0x3e4c40a6;
	static const long c_fC4 = 0xbe0e6c63;
	static const long c_fC5 = 0x3dc577df;
	static const long c_fC6 = 0xbd6501c4;
	static const long c_fC7 = 0x3cb31652;
	static const long c_fC8 = 0xbb84d7e7;

	u16 NoFlags;
	
	float fs;
	float fy1, fy2, fy3, fy4, fy5;
	float ft, ft2, ft3, ft5, ft7, ft9, ft11, ft13, ft15;

	
	//cout << "\nhps2x64: ERROR: VU: Instruction not implemented: EATAN";


#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	//if ( v->PBusyUntil_Cycle )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	
	v->SetP ();
#endif

#ifdef USE_NEW_RECOMPILE2_WAITP
	// wait p
	v->WaitP_Micro ();
#endif

	fs = (float&) v->vf [ i.Fs ].vuw [ i.fsf ];
	
	// get (x-1)/(x+1)
	ft = PS2_Float_Div ( PS2_Float_Sub ( fs, 1.0f, 0, & NoFlags, & NoFlags ), PS2_Float_Add ( fs, 1.0f, 0, & NoFlags, & NoFlags ), & NoFlags );
	
	ft2 = PS2_Float_Mul ( ft, ft, 0, & NoFlags, & NoFlags );
	ft3 = PS2_Float_Mul ( ft, ft2, 0, & NoFlags, & NoFlags );
	ft5 = PS2_Float_Mul ( ft3, ft2, 0, & NoFlags, & NoFlags );
	ft7 = PS2_Float_Mul ( ft5, ft2, 0, & NoFlags, & NoFlags );
	ft9 = PS2_Float_Mul ( ft7, ft2, 0, & NoFlags, & NoFlags );
	ft11 = PS2_Float_Mul ( ft9, ft2, 0, & NoFlags, & NoFlags );
	ft13 = PS2_Float_Mul ( ft11, ft2, 0, & NoFlags, & NoFlags );
	ft15 = PS2_Float_Mul ( ft13, ft2, 0, & NoFlags, & NoFlags );
	
	// (pi/4) + ( c1 * ft + c2 * ft^3 + c3 * ft^5 + c4 * ft^7 + c5 * ft^9 + c6 * ft^11 + c7 * ft^13 + c8 * ft^15 )
	fy1 = PS2_Float_Add ( PS2_Float_Mul ( (float&) c_fC1, ft, 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( (float&) c_fC2, ft3, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	fy2 = PS2_Float_Add ( PS2_Float_Mul ( (float&) c_fC3, ft5, 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( (float&) c_fC4, ft7, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	fy3 = PS2_Float_Add ( PS2_Float_Mul ( (float&) c_fC5, ft9, 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( (float&) c_fC6, ft11, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	fy4 = PS2_Float_Add ( PS2_Float_Mul ( (float&) c_fC7, ft13, 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( (float&) c_fC8, ft15, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	fy5 = PS2_Float_Add ( PS2_Float_Add ( fy1, fy2, 0, & NoFlags, & NoFlags ), PS2_Float_Add ( fy3, fy4, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	v->NextP.f = PS2_Float_Add ( fy5, (float&) c_fC0, 0, & NoFlags, & NoFlags );
	v->PBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_EATAN || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " (after) NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
#endif
}

void Execute::EATANxy ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_EATANXY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << hex << " (before)NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif

	// 53/54 cycles
	static const u64 c_CycleTime = 53;
	
	// I'll make c0 = pi/4
	static const long c_fC0 = 0x3f490fdb;
	
	static const long c_fC1 = 0x3f7ffff5;
	static const long c_fC2 = 0xbeaaa61c;
	static const long c_fC3 = 0x3e4c40a6;
	static const long c_fC4 = 0xbe0e6c63;
	static const long c_fC5 = 0x3dc577df;
	static const long c_fC6 = 0xbd6501c4;
	static const long c_fC7 = 0x3cb31652;
	static const long c_fC8 = 0xbb84d7e7;

	u16 NoFlags;
	
	float fx, fy;
	float fy1, fy2, fy3, fy4, fy5;
	float ft, ft2, ft3, ft5, ft7, ft9, ft11, ft13, ft15;

	
	//cout << "\nhps2x64: ERROR: VU: Instruction not implemented: EATANxy";


#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	//if ( v->PBusyUntil_Cycle )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	v->SetP ();
#endif

#ifdef USE_NEW_RECOMPILE2_WAITP
	// wait p
	v->WaitP_Micro ();
#endif

	fx = v->vf [ i.Fs ].fx;
	fy = v->vf [ i.Fs ].fy;
	
	// get (y-1)/(x+y)
	ft = PS2_Float_Div ( PS2_Float_Sub ( fy, fx, 0, & NoFlags, & NoFlags ), PS2_Float_Add ( fx, fy, 0, & NoFlags, & NoFlags ), & NoFlags );
	
	ft2 = PS2_Float_Mul ( ft, ft, 0, & NoFlags, & NoFlags );
	ft3 = PS2_Float_Mul ( ft, ft2, 0, & NoFlags, & NoFlags );
	ft5 = PS2_Float_Mul ( ft3, ft2, 0, & NoFlags, & NoFlags );
	ft7 = PS2_Float_Mul ( ft5, ft2, 0, & NoFlags, & NoFlags );
	ft9 = PS2_Float_Mul ( ft7, ft2, 0, & NoFlags, & NoFlags );
	ft11 = PS2_Float_Mul ( ft9, ft2, 0, & NoFlags, & NoFlags );
	ft13 = PS2_Float_Mul ( ft11, ft2, 0, & NoFlags, & NoFlags );
	ft15 = PS2_Float_Mul ( ft13, ft2, 0, & NoFlags, & NoFlags );
	
	// (pi/4) + ( c1 * ft + c2 * ft^3 + c3 * ft^5 + c4 * ft^7 + c5 * ft^9 + c6 * ft^11 + c7 * ft^13 + c8 * ft^15 )
	fy1 = PS2_Float_Add ( PS2_Float_Mul ( (float&) c_fC1, ft, 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( (float&) c_fC2, ft3, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	fy2 = PS2_Float_Add ( PS2_Float_Mul ( (float&) c_fC3, ft5, 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( (float&) c_fC4, ft7, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	fy3 = PS2_Float_Add ( PS2_Float_Mul ( (float&) c_fC5, ft9, 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( (float&) c_fC6, ft11, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	fy4 = PS2_Float_Add ( PS2_Float_Mul ( (float&) c_fC7, ft13, 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( (float&) c_fC8, ft15, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	fy5 = PS2_Float_Add ( PS2_Float_Add ( fy1, fy2, 0, & NoFlags, & NoFlags ), PS2_Float_Add ( fy3, fy4, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	v->NextP.f = PS2_Float_Add ( fy5, (float&) c_fC0, 0, & NoFlags, & NoFlags );
	v->PBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_EATANXY || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " (after) NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
#endif
}

void Execute::EATANxz ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_EATANXZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << hex << " (before)NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif

	// 53/54 cycles
	static const u64 c_CycleTime = 53;
	
	// I'll make c0 = pi/4
	static const long c_fC0 = 0x3f490fdb;
	
	static const long c_fC1 = 0x3f7ffff5;
	static const long c_fC2 = 0xbeaaa61c;
	static const long c_fC3 = 0x3e4c40a6;
	static const long c_fC4 = 0xbe0e6c63;
	static const long c_fC5 = 0x3dc577df;
	static const long c_fC6 = 0xbd6501c4;
	static const long c_fC7 = 0x3cb31652;
	static const long c_fC8 = 0xbb84d7e7;

	u16 NoFlags;
	
	float fx, fz;
	float fy1, fy2, fy3, fy4, fy5;
	float ft, ft2, ft3, ft5, ft7, ft9, ft11, ft13, ft15;

	
	//cout << "\nhps2x64: ERROR: VU: Instruction not implemented: EATANxz";


#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	//if ( v->PBusyUntil_Cycle )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	v->SetP ();
#endif

#ifdef USE_NEW_RECOMPILE2_WAITP
	// wait p
	v->WaitP_Micro ();
#endif

	fx = v->vf [ i.Fs ].fx;
	fz = v->vf [ i.Fs ].fz;
	
	// get (z-x)/(x+z)
	ft = PS2_Float_Div ( PS2_Float_Sub ( fz, fx, 0, & NoFlags, & NoFlags ), PS2_Float_Add ( fx, fz, 0, & NoFlags, & NoFlags ), & NoFlags );
	
	ft2 = PS2_Float_Mul ( ft, ft, 0, & NoFlags, & NoFlags );
	ft3 = PS2_Float_Mul ( ft, ft2, 0, & NoFlags, & NoFlags );
	ft5 = PS2_Float_Mul ( ft3, ft2, 0, & NoFlags, & NoFlags );
	ft7 = PS2_Float_Mul ( ft5, ft2, 0, & NoFlags, & NoFlags );
	ft9 = PS2_Float_Mul ( ft7, ft2, 0, & NoFlags, & NoFlags );
	ft11 = PS2_Float_Mul ( ft9, ft2, 0, & NoFlags, & NoFlags );
	ft13 = PS2_Float_Mul ( ft11, ft2, 0, & NoFlags, & NoFlags );
	ft15 = PS2_Float_Mul ( ft13, ft2, 0, & NoFlags, & NoFlags );
	
	// (pi/4) + ( c1 * ft + c2 * ft^3 + c3 * ft^5 + c4 * ft^7 + c5 * ft^9 + c6 * ft^11 + c7 * ft^13 + c8 * ft^15 )
	fy1 = PS2_Float_Add ( PS2_Float_Mul ( (float&) c_fC1, ft, 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( (float&) c_fC2, ft3, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	fy2 = PS2_Float_Add ( PS2_Float_Mul ( (float&) c_fC3, ft5, 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( (float&) c_fC4, ft7, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	fy3 = PS2_Float_Add ( PS2_Float_Mul ( (float&) c_fC5, ft9, 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( (float&) c_fC6, ft11, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	fy4 = PS2_Float_Add ( PS2_Float_Mul ( (float&) c_fC7, ft13, 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( (float&) c_fC8, ft15, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	fy5 = PS2_Float_Add ( PS2_Float_Add ( fy1, fy2, 0, & NoFlags, & NoFlags ), PS2_Float_Add ( fy3, fy4, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	v->NextP.f = PS2_Float_Add ( fy5, (float&) c_fC0, 0, & NoFlags, & NoFlags );
	v->PBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_EATANXZ || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " (after) NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
#endif
}

void Execute::EEXP ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_EEXP || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << hex << " (before)NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif

	// 43/44 cycles
	static const u64 c_CycleTime = 43;	//44;
	
	//static const float c_fC0 = (const float&) ((const long&) 0x3f800000);
	static const long c_fC1 = 0x3e7fffa8;
	static const long c_fC2 = 0x3d0007f4;
	static const long c_fC3 = 0x3b29d3ff;
	static const long c_fC4 = 0x3933e553;
	static const long c_fC5 = 0x36b63510;
	static const long c_fC6 = 0x353961ac;

	u16 NoFlags;
	
	float fs;
	float fx2, fx3, fx4, fx5, fx6;
	float fy, fy2, fy4;
	float ft1, ft2, ft3, ft4, ft5, ft6;
	float fc12, fc34, fc56, fc1234, fc123456;

	
	//cout << "\nhps2x64: ERROR: VU: Instruction not implemented: EEXP";


#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	//if ( v->PBusyUntil_Cycle )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	v->SetP ();
#endif

#ifdef USE_NEW_RECOMPILE2_WAITP
	// wait p
	v->WaitP_Micro ();
#endif

	fs = (float&) v->vf [ i.Fs ].vuw [ i.fsf ];
	
	fx2 = PS2_Float_Mul ( fs, fs, 0, & NoFlags, & NoFlags );
	fx3 = PS2_Float_Mul ( fs, fx2, 0, & NoFlags, & NoFlags );
	fx4 = PS2_Float_Mul ( fs, fx3, 0, & NoFlags, & NoFlags );
	fx5 = PS2_Float_Mul ( fs, fx4, 0, & NoFlags, & NoFlags );
	fx6 = PS2_Float_Mul ( fs, fx5, 0, & NoFlags, & NoFlags );
	
	// 1 / ( ( 1 + c1 * x + c2 * x^2 + c3 * x^3 + c4 * x^4 + c5 * x^5 + c6 * x^6 ) ^ 4 )
	// *TODO*: should use fast floating point ?
	//fy = 1.0f + ( c_fC1 * fs ) + ( c_fC2 * fx2 ) + ( c_fC3 * fx3 ) + ( c_fC4 * fx4 ) + ( c_fC5 * fx5 ) + ( c_fC6 * fx6 );

	ft1 = PS2_Float_Mul ( (float&) c_fC1, fs, 0, & NoFlags, & NoFlags );
	ft2 = PS2_Float_Mul ( (float&) c_fC2, fx2, 0, & NoFlags, & NoFlags );
	ft3 = PS2_Float_Mul ( (float&) c_fC3, fx3, 0, & NoFlags, & NoFlags );
	ft4 = PS2_Float_Mul ( (float&) c_fC4, fx4, 0, & NoFlags, & NoFlags );
	ft5 = PS2_Float_Mul ( (float&) c_fC5, fx5, 0, & NoFlags, & NoFlags );
	ft6 = PS2_Float_Mul ( (float&) c_fC6, fx6, 0, & NoFlags, & NoFlags );
	
	fc12 = PS2_Float_Add ( ft1, ft2, 0, & NoFlags, & NoFlags );
	fc34 = PS2_Float_Add ( ft3, ft4, 0, & NoFlags, & NoFlags );
	fc56 = PS2_Float_Add ( ft5, ft6, 0, & NoFlags, & NoFlags );
	fc1234 = PS2_Float_Add ( fc12, fc34, 0, & NoFlags, & NoFlags );
	fc123456 = PS2_Float_Add ( fc1234, fc56, 0, & NoFlags, & NoFlags );
	fy = PS2_Float_Add ( 1.0f, fc123456, 0, & NoFlags, & NoFlags );
	
	fy2 = PS2_Float_Mul ( fy, fy, 0, & NoFlags, & NoFlags );
	fy4 = PS2_Float_Mul ( fy2, fy2, 0, & NoFlags, & NoFlags );
	v->NextP.f = PS2_Float_Div ( 1.0f, fy4, & NoFlags );
	v->PBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_EEXP || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " (after) NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
#endif
}

void Execute::ESIN ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ESIN || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << hex << " (before)NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif

	// 28/29 cycles
	static const u64 c_CycleTime = 28;	//29;
	
	static const long c_fC1 = 0x3f800000;
	static const long c_fC2 = 0xbe2aaaa4;
	static const long c_fC3 = 0x3c08873e;
	static const long c_fC4 = 0xb94fb21f;
	static const long c_fC5 = 0x362e9c14;

	u16 NoFlags;
	
	float fs;
	float fx2, fx3, fx5, fx7, fx9;
	float fc1, fc2, fc3, fc4, fc5, fc12, fc34, fc1234, fy;

	
	//cout << "\nhps2x64: ERROR: VU: Instruction not implemented: ESIN";


#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	//if ( v->PBusyUntil_Cycle )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	v->SetP ();
#endif

#ifdef USE_NEW_RECOMPILE2_WAITP
	// wait p
	v->WaitP_Micro ();
#endif

	fs = (float&) v->vf [ i.Fs ].vuw [ i.fsf ];
	
	fx2 = PS2_Float_Mul ( fs, fs, 0, & NoFlags, & NoFlags );
	fx3 = PS2_Float_Mul ( fs, fx2, 0, & NoFlags, & NoFlags );
	fx5 = PS2_Float_Mul ( fx3, fx2, 0, & NoFlags, & NoFlags );
	fx7 = PS2_Float_Mul ( fx5, fx2, 0, & NoFlags, & NoFlags );
	fx9 = PS2_Float_Mul ( fx7, fx2, 0, & NoFlags, & NoFlags );
	
	fc1 = PS2_Float_Mul ( (float&) c_fC1, fs, 0, & NoFlags, & NoFlags );
	fc2 = PS2_Float_Mul ( (float&) c_fC2, fx3, 0, & NoFlags, & NoFlags );
	fc3 = PS2_Float_Mul ( (float&) c_fC3, fx5, 0, & NoFlags, & NoFlags );
	fc4 = PS2_Float_Mul ( (float&) c_fC4, fx7, 0, & NoFlags, & NoFlags );
	fc5 = PS2_Float_Mul ( (float&) c_fC5, fx9, 0, & NoFlags, & NoFlags );
	
	fc12 = PS2_Float_Add ( fc1, fc2, 0, & NoFlags, & NoFlags );
	fc34 = PS2_Float_Add ( fc3, fc4, 0, & NoFlags, & NoFlags );
	fc1234 = PS2_Float_Add ( fc12, fc34, 0, & NoFlags, & NoFlags );
	fy = PS2_Float_Add ( fc1234, fc5, 0, & NoFlags, & NoFlags );
	
	// c1 * x + c2 * x^3 + c3 * x^5 + c4 * x^7 + c5 * x^9
	// *TODO*: should use fast floating point ?
	//v->NextP.f = ( c_fC1 * fs ) + ( c_fC2 * fx3 ) + ( c_fC3 * fx5 ) + ( c_fC4 * fx7 ) + ( c_fC5 * fx9 );
	v->NextP.f = fy;
	v->PBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_ESIN || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " (after) NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
#endif
}

void Execute::ERSQRT ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ERSQRT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " i.Fs=" << dec << i.Fs << " i.fsf=" << i.fsf << " vf[i.Fs]=" << hex << v->vf [ i.Fs ].vuw [ i.fsf ];
	debug << hex << " (before)NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif

	//cout << "\nhps2x64: ERROR: VU: Instruction not implemented: ERSQRT";
	
	// 17/18 cycles
	static const u64 c_CycleTime = 17;

	u16 NoFlags;
	float fs;
	
#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	//if ( v->PBusyUntil_Cycle )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	v->SetP ();
#endif
	
#ifdef USE_NEW_RECOMPILE2_WAITP
	// wait p
	v->WaitP_Micro ();
#endif

	fs = (float&) v->vf [ i.Fs ].vuw [ i.fsf ];

#if defined INLINE_DEBUG_ERSQRT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " (float)" << fs;
#endif
	
	v->NextP.f = PS2_Float_RSqrt ( 1.0f, fs, & NoFlags );
	v->PBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_ERSQRT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " (after) NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
#endif
}

void Execute::ERCPR ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ERCPR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " i.Fs=" << dec << i.Fs << " i.fsf=" << i.fsf << " vf[i.Fs]=" << hex << v->vf [ i.Fs ].vuw [ i.fsf ];
	debug << hex << " (before)NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif

	// 11/12 cycles
	static const u64 c_CycleTime = 11;

	u16 NoFlags;
	float fs;
	
	//cout << "\nhps2x64: ERROR: VU: Instruction not implemented: ERCPR";
	
#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	//if ( v->PBusyUntil_Cycle )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	v->SetP ();
#endif

#ifdef USE_NEW_RECOMPILE2_WAITP
	// wait p
	v->WaitP_Micro ();
#endif

	fs = (float&) v->vf [ i.Fs ].vuw [ i.fsf ];
	
#if defined INLINE_DEBUG_ERCPR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " fs= (float)" << fs;
#endif

	v->NextP.f = PS2_Float_Div ( 1.0f, fs, & NoFlags );

	v->PBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_ERCPR || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " (after) NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
#endif
}


void Execute::ESQRT ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ESQRT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " i.Fs=" << dec << i.Fs << " i.fsf=" << i.fsf << " vf[i.Fs]=" << hex << v->vf [ i.Fs ].vuw [ i.fsf ];
	debug << hex << " (before)NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif

	
	// 11/12 cycles
	static const u64 c_CycleTime = 11;

	u16 NoFlags;
	float fs;
	
#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	//if ( v->PBusyUntil_Cycle )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	v->SetP ();
#endif

#ifdef USE_NEW_RECOMPILE2_WAITP
	// wait p
	v->WaitP_Micro ();
#endif

	fs = (float&) v->vf [ i.Fs ].vuw [ i.fsf ];
	
#if defined INLINE_DEBUG_ESQRT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << " (float)" << fs;
#endif

	v->NextP.f = PS2_Float_Sqrt ( fs, & NoFlags );
	v->PBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_ESQRT || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " (after) NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
#endif
}

void Execute::ESADD ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ESADD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << hex << " (before)NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif


	// 10/11 cycles
	static const u64 c_CycleTime = 10;

	u16 NoFlags;
	
#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	//if ( v->PBusyUntil_Cycle )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	v->SetP ();
#endif
	
	// wait p
	v->WaitP_Micro ();

	v->NextP.f = PS2_Float_Add ( PS2_Float_Add ( PS2_Float_Mul ( v->vf [ i.Fs ].fx, v->vf [ i.Fs ].fx, 0, & NoFlags, & NoFlags ) , PS2_Float_Mul ( v->vf [ i.Fs ].fy, v->vf [ i.Fs ].fy, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( v->vf [ i.Fs ].fz, v->vf [ i.Fs ].fz, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	v->PBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_ESADD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " (after) NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
#endif
}

void Execute::ERSADD ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ERSADD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << hex << " (before)NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif

	
	// 17/18 cycles
	static const u64 c_CycleTime = 17;

	u16 NoFlags;
	//float fs = (float&) v->vf [ i.Fs ].vuw [ i.fsf ];
	
#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	//if ( v->PBusyUntil_Cycle )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	v->SetP ();
#endif
	
	// wait p
	v->WaitP_Micro ();

	v->NextP.f = PS2_Float_Div ( 1.0f, PS2_Float_Add ( PS2_Float_Add ( PS2_Float_Mul ( v->vf [ i.Fs ].fx, v->vf [ i.Fs ].fx, 0, & NoFlags, & NoFlags ) , PS2_Float_Mul ( v->vf [ i.Fs ].fy, v->vf [ i.Fs ].fy, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( v->vf [ i.Fs ].fz, v->vf [ i.Fs ].fz, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags ), & NoFlags );

	v->PBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_ERSADD || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " (after) NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
#endif
}


void Execute::ESUM ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ESUM || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << hex << " (before)NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif

	
	// 11/12 cycles
	static const u64 c_CycleTime = 11;

	u16 NoFlags;
	
#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	//if ( v->PBusyUntil_Cycle )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	v->SetP ();
#endif
	
	// wait p
	v->WaitP_Micro ();

	v->NextP.f = PS2_Float_Add ( PS2_Float_Add ( v->vf [ i.Fs ].fx, v->vf [ i.Fs ].fy, 0, & NoFlags, & NoFlags ), PS2_Float_Add ( v->vf [ i.Fs ].fz, v->vf [ i.Fs ].fw, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags );
	v->PBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_ESUM || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " (after) NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
#endif
}


void Execute::ELENG ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ELENG || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << hex << " (before)NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif

	
	// 17/18 cycles
	static const u64 c_CycleTime = 17;

	u16 NoFlags;
	
#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	//if ( v->PBusyUntil_Cycle )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	v->SetP ();
#endif
	
	// wait p
	v->WaitP_Micro ();

	v->NextP.f = PS2_Float_Sqrt ( PS2_Float_Add ( PS2_Float_Add ( PS2_Float_Mul ( v->vf [ i.Fs ].fx, v->vf [ i.Fs ].fx, 0, & NoFlags, & NoFlags ) , PS2_Float_Mul ( v->vf [ i.Fs ].fy, v->vf [ i.Fs ].fy, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( v->vf [ i.Fs ].fz, v->vf [ i.Fs ].fz, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags ), & NoFlags );
	v->PBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_ELENG || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " (after) NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
#endif
}


void Execute::ERLENG ( VU *v, Instruction::Format i )
{
#if defined INLINE_DEBUG_ERLENG || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionLO ( i.Value ).c_str () << "; " << hex << i.Value;
	debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	debug << hex << " (before)NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
	debug << dec << " PBusyUntil=" << v->PBusyUntil_Cycle;
#endif

	
	// 23/24 cycles
	static const u64 c_CycleTime = 23;

	u16 NoFlags;
	
#ifdef ENABLE_STALLS
	// if the EFU unit is still running, then need to wait until it finishes first
	//if ( v->PBusyUntil_Cycle )
	if ( ( (s64)v->CycleCount ) < ( (s64)v->PBusyUntil_Cycle ) )
	{
		// EFU unit is already busy //
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
		
		// wait until EFU unit is done with what it is doing
		v->PipelineWaitP ();
	}
	v->SetP ();
#endif
	
	// wait p
	v->WaitP_Micro ();

	v->NextP.f = PS2_Float_RSqrt ( 1.0f, PS2_Float_Add ( PS2_Float_Add ( PS2_Float_Mul ( v->vf [ i.Fs ].fx, v->vf [ i.Fs ].fx, 0, & NoFlags, & NoFlags ) , PS2_Float_Mul ( v->vf [ i.Fs ].fy, v->vf [ i.Fs ].fy, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags ), PS2_Float_Mul ( v->vf [ i.Fs ].fz, v->vf [ i.Fs ].fz, 0, & NoFlags, & NoFlags ), 0, & NoFlags, & NoFlags ), & NoFlags );
	v->PBusyUntil_Cycle = v->CycleCount + c_CycleTime;
	
#if defined INLINE_DEBUG_ERLENG || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_EXT || defined INLINE_DEBUG_UNIMPLEMENTED
	debug << hex << " (after) NextP=" << v->NextP.l << " (float)" << v->NextP.f << " CurrentP=" << v->vi [ VU::REG_P ].u << " (float)" << v->vi [ VU::REG_P ].f;
#endif
}










const Execute::Function Execute::FunctionList []
{
	Execute::INVALID,
	
	// VU macro mode instructions //
	
	//Execute::COP2
	//Execute::QMFC2_NI, Execute::QMFC2_I, Execute::QMTC2_NI, Execute::QMTC2_I, Execute::LQC2, Execute::SQC2,
	//Execute::CALLMS, Execute::CALLMSR,
	
	// upper instructions //
	
	Execute::ABS,
	Execute::ADD, Execute::ADDi, Execute::ADDq, Execute::ADDBCX, Execute::ADDBCY, Execute::ADDBCZ, Execute::ADDBCW,
	Execute::ADDA, Execute::ADDAi, Execute::ADDAq, Execute::ADDABCX, Execute::ADDABCY, Execute::ADDABCZ, Execute::ADDABCW,
	Execute::CLIP,
	Execute::FTOI0, Execute::FTOI4, Execute::FTOI12, Execute::FTOI15,
	Execute::ITOF0, Execute::ITOF4, Execute::ITOF12, Execute::ITOF15,
	
	Execute::MADD, Execute::MADDi, Execute::MADDq, Execute::MADDBCX, Execute::MADDBCY, Execute::MADDBCZ, Execute::MADDBCW,
	Execute::MADDA, Execute::MADDAi, Execute::MADDAq, Execute::MADDABCX, Execute::MADDABCY, Execute::MADDABCZ, Execute::MADDABCW,
	Execute::MAX, Execute::MAXi, Execute::MAXBCX, Execute::MAXBCY, Execute::MAXBCZ, Execute::MAXBCW,
	Execute::MINI, Execute::MINIi, Execute::MINIBCX, Execute::MINIBCY, Execute::MINIBCZ, Execute::MINIBCW,
	
	Execute::MSUB, Execute::MSUBi, Execute::MSUBq, Execute::MSUBBCX, Execute::MSUBBCY, Execute::MSUBBCZ, Execute::MSUBBCW,
	Execute::MSUBA, Execute::MSUBAi, Execute::MSUBAq, Execute::MSUBABCX, Execute::MSUBABCY, Execute::MSUBABCZ, Execute::MSUBABCW,
	Execute::MUL, Execute::MULi, Execute::MULq, Execute::MULBCX, Execute::MULBCY, Execute::MULBCZ, Execute::MULBCW,
	Execute::MULA, Execute::MULAi, Execute::MULAq, Execute::MULABCX, Execute::MULABCY, Execute::MULABCZ, Execute::MULABCW,
	Execute::NOP, Execute::OPMSUB, Execute::OPMULA,
	Execute::SUB, Execute::SUBi, Execute::SUBq, Execute::SUBBCX, Execute::SUBBCY, Execute::SUBBCZ, Execute::SUBBCW,
	Execute::SUBA, Execute::SUBAi, Execute::SUBAq, Execute::SUBABCX, Execute::SUBABCY, Execute::SUBABCZ, Execute::SUBABCW,
	
	// lower instructions //
	
	Execute::DIV,
	Execute::IADD, Execute::IADDI, Execute::IAND,
	Execute::ILWR,
	Execute::IOR, Execute::ISUB,
	Execute::ISWR,
	Execute::LQD, Execute::LQI,
	Execute::MFIR, Execute::MOVE, Execute::MR32, Execute::MTIR,
	Execute::RGET, Execute::RINIT, Execute::RNEXT,
	Execute::RSQRT,
	Execute::RXOR,
	Execute::SQD, Execute::SQI,
	Execute::SQRT,
	Execute::WAITQ,

	// instructions not in macro mode //
	
	Execute::B, Execute::BAL,
	Execute::FCAND, Execute::FCEQ, Execute::FCGET, Execute::FCOR, Execute::FCSET,
	Execute::FMAND, Execute::FMEQ, Execute::FMOR,
	Execute::FSAND, Execute::FSEQ, Execute::FSOR, Execute::FSSET,
	Execute::IADDIU,
	Execute::IBEQ, Execute::IBGEZ, Execute::IBGTZ, Execute::IBLEZ, Execute::IBLTZ, Execute::IBNE,
	Execute::ILW,
	Execute::ISUBIU, Execute::ISW,
	Execute::JALR, Execute::JR,
	Execute::LQ,
	Execute::MFP,
	Execute::SQ,
	Execute::WAITP,
	Execute::XGKICK, Execute::XITOP, Execute::XTOP,

	// External Unit //

	Execute::EATAN, Execute::EATANxy, Execute::EATANxz, Execute::EEXP, Execute::ELENG, Execute::ERCPR, Execute::ERLENG, Execute::ERSADD,
	Execute::ERSQRT, Execute::ESADD, Execute::ESIN, Execute::ESQRT, Execute::ESUM
};





