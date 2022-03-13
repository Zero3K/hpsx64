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
#include "VU_Recompiler.h"
//#include "ps2_system.h"
#include "VU_Print.h"
//#include "PS2DataBus.h"

using namespace Playstation2;
using namespace Vu;



//#define DISABLE_LQD_VU0
//#define DISABLE_LQI_VU0
//#define DISABLE_LQ_VU0
//#define DISABLE_SQD_VU0
//#define DISABLE_SQI_VU0
//#define DISABLE_SQ_VU0
//#define DISABLE_ILW_VU0
//#define DISABLE_ILWR_VU0
//#define DISABLE_ISW_VU0
//#define DISABLE_ISWR_VU0





//#define ENABLE_VFLAGS


#define ENABLE_RECOMPILER_BITMAP		
#define ENABLE_SETDSTBITMAP
#define RECOMPILE_SETDSTBITMAP
#define ENABLE_RECOMPILER_ADVANCE_CYCLE



#define OPTIMIZE_RO_MULTIPLY_MUL
#define OPTIMIZE_RO_MULTIPLY_MADD

#define USE_NEW_VMUL_CODE
#define USE_NEW_VMADD_CODE



#define ENABLE_INTDELAYSLOT_SQI
#define ENABLE_INTDELAYSLOT_SQD
#define ENABLE_INTDELAYSLOT_LQI
#define ENABLE_INTDELAYSLOT_LQD


//#define ENABLE_TEST_DEBUG_FTOI4


//#define ENABLE_DSTBITMAP_BEFORE_DELAYSLOT


#define ENABLE_BITMAP_NOP


#define ENABLE_BITMAP_ABS

#define ENABLE_BITMAP_MAX
#define ENABLE_BITMAP_MAXi
#define ENABLE_BITMAP_MAXX
#define ENABLE_BITMAP_MAXY
#define ENABLE_BITMAP_MAXZ
#define ENABLE_BITMAP_MAXW

#define ENABLE_BITMAP_MINI
#define ENABLE_BITMAP_MINIi
#define ENABLE_BITMAP_MINIX
#define ENABLE_BITMAP_MINIY
#define ENABLE_BITMAP_MINIZ
#define ENABLE_BITMAP_MINIW


// problem
#define ENABLE_BITMAP_FTOI0
#define ENABLE_BITMAP_FTOI4


#define ENABLE_BITMAP_FTOI12
#define ENABLE_BITMAP_FTOI15


#define ENABLE_BITMAP_ITOF0
#define ENABLE_BITMAP_ITOF4
#define ENABLE_BITMAP_ITOF12
#define ENABLE_BITMAP_ITOF15



#define ENABLE_BITMAP_MOVE
#define ENABLE_BITMAP_MR32

#define ENABLE_BITMAP_MFP



#define ENABLE_BITMAP_IADD
#define ENABLE_BITMAP_IADDI
#define ENABLE_BITMAP_IADDIU
#define ENABLE_BITMAP_IAND
#define ENABLE_BITMAP_IOR
#define ENABLE_BITMAP_ISUB
#define ENABLE_BITMAP_ISUBIU



#define ENABLE_BITMAP_LQ
#define ENABLE_BITMAP_LQI
#define ENABLE_BITMAP_LQD
#define ENABLE_BITMAP_ILW
#define ENABLE_BITMAP_ILWR
#define ENABLE_BITMAP_SQ
#define ENABLE_BITMAP_SQI
#define ENABLE_BITMAP_SQD
#define ENABLE_BITMAP_ISW
#define ENABLE_BITMAP_ISWR


#define ENABLE_BITMAP_MFIR
#define ENABLE_BITMAP_MTIR



#define ENABLE_BITMAP_DIV
#define ENABLE_BITMAP_RSQRT
#define ENABLE_BITMAP_SQRT


#define ENABLE_BITMAP_XGKICK



#define ENABLE_BITMAP_RGET
#define ENABLE_BITMAP_RINIT
#define ENABLE_BITMAP_RNEXT
#define ENABLE_BITMAP_RXOR



#define ENABLE_BITMAP_EATAN
#define ENABLE_BITMAP_EATANxy
#define ENABLE_BITMAP_EATANxz
#define ENABLE_BITMAP_EEXP
#define ENABLE_BITMAP_ELENG
#define ENABLE_BITMAP_ERCPR
#define ENABLE_BITMAP_ERLENG
#define ENABLE_BITMAP_ERSADD
#define ENABLE_BITMAP_ERSQRT
#define ENABLE_BITMAP_ESADD
#define ENABLE_BITMAP_ESIN
#define ENABLE_BITMAP_ESQRT
#define ENABLE_BITMAP_ESUM




#define ENABLE_BITMAP_CLIP


// problem
#define ENABLE_BITMAP_ADD
#define ENABLE_BITMAP_ADDi

#define ENABLE_BITMAP_ADDq
#define ENABLE_BITMAP_ADDX
#define ENABLE_BITMAP_ADDY

// problem
#define ENABLE_BITMAP_ADDZ
#define ENABLE_BITMAP_ADDW



#define ENABLE_BITMAP_ADDA
#define ENABLE_BITMAP_ADDAi
#define ENABLE_BITMAP_ADDAq
#define ENABLE_BITMAP_ADDAX
#define ENABLE_BITMAP_ADDAY
#define ENABLE_BITMAP_ADDAZ
#define ENABLE_BITMAP_ADDAW


// problem
#define ENABLE_BITMAP_SUB

#define ENABLE_BITMAP_SUBi
#define ENABLE_BITMAP_SUBq
#define ENABLE_BITMAP_SUBX
#define ENABLE_BITMAP_SUBY
#define ENABLE_BITMAP_SUBZ
#define ENABLE_BITMAP_SUBW


#define ENABLE_BITMAP_SUBA
#define ENABLE_BITMAP_SUBAi
#define ENABLE_BITMAP_SUBAq
#define ENABLE_BITMAP_SUBAX
#define ENABLE_BITMAP_SUBAY
#define ENABLE_BITMAP_SUBAZ
#define ENABLE_BITMAP_SUBAW



#define ENABLE_BITMAP_MUL

// problem
#define ENABLE_BITMAP_MULi

#define ENABLE_BITMAP_MULq

// problem
#define ENABLE_BITMAP_MULX

#define ENABLE_BITMAP_MULY
#define ENABLE_BITMAP_MULZ
#define ENABLE_BITMAP_MULW


#define ENABLE_BITMAP_MULA
#define ENABLE_BITMAP_MULAi
#define ENABLE_BITMAP_MULAq
#define ENABLE_BITMAP_MULAX
#define ENABLE_BITMAP_MULAY
#define ENABLE_BITMAP_MULAZ
#define ENABLE_BITMAP_MULAW



#define ENABLE_BITMAP_MADD
#define ENABLE_BITMAP_MADDi
#define ENABLE_BITMAP_MADDq
#define ENABLE_BITMAP_MADDX
#define ENABLE_BITMAP_MADDY
#define ENABLE_BITMAP_MADDZ
#define ENABLE_BITMAP_MADDW



#define ENABLE_BITMAP_MADDA
#define ENABLE_BITMAP_MADDAi
#define ENABLE_BITMAP_MADDAq
#define ENABLE_BITMAP_MADDAX
#define ENABLE_BITMAP_MADDAY
#define ENABLE_BITMAP_MADDAZ
#define ENABLE_BITMAP_MADDAW


#define ENABLE_BITMAP_MSUB
#define ENABLE_BITMAP_MSUBi
#define ENABLE_BITMAP_MSUBq
#define ENABLE_BITMAP_MSUBX
#define ENABLE_BITMAP_MSUBY
#define ENABLE_BITMAP_MSUBZ
#define ENABLE_BITMAP_MSUBW


#define ENABLE_BITMAP_MSUBA
#define ENABLE_BITMAP_MSUBAi
#define ENABLE_BITMAP_MSUBAq
#define ENABLE_BITMAP_MSUBAX
#define ENABLE_BITMAP_MSUBAY
#define ENABLE_BITMAP_MSUBAZ
#define ENABLE_BITMAP_MSUBAW


#define ENABLE_BITMAP_OPMSUB
#define ENABLE_BITMAP_OPMULA




// -----------------------------


#define USE_NEW_NOP_RECOMPILE

#define USE_NEW_ABS_RECOMPILE

#define USE_NEW_MAX_RECOMPILE
#define USE_NEW_MIN_RECOMPILE


#define USE_NEW_FTOI0_RECOMPILE
#define USE_NEW_FTOI4_RECOMPILE
#define USE_NEW_FTOI12_RECOMPILE
#define USE_NEW_FTOI15_RECOMPILE


#define USE_NEW_ITOF0_RECOMPILE
#define USE_NEW_ITOF4_RECOMPILE
#define USE_NEW_ITOF12_RECOMPILE
#define USE_NEW_ITOF15_RECOMPILE



#define USE_NEW_MOVE_RECOMPILE
#define USE_NEW_MR32_RECOMPILE

#define USE_NEW_MFP_RECOMPILE


#define USE_NEW_IADD_RECOMPILE
#define USE_NEW_IADDI_RECOMPILE
#define USE_NEW_IAND_RECOMPILE
#define USE_NEW_IOR_RECOMPILE
#define USE_NEW_ISUB_RECOMPILE

#define USE_NEW_IADDIU_RECOMPILE
#define USE_NEW_ISUBIU_RECOMPILE


// these deal with the integer registers that have delay type slots
// needs work
#define USE_NEW_MFIR_RECOMPILE
#define USE_NEW_MTIR_RECOMPILE


// MACRO MODE R5900 ONLY
//#define USE_NEW_CFC2_NI_RECOMPILE
//#define USE_NEW_CTC2_NI_RECOMPILE
//#define USE_NEW_QMFC2_NI_RECOMPILE
//#define USE_NEW_QMTC2_NI_RECOMPILE


// needs work
#define USE_NEW_DIV_RECOMPILE
#define USE_NEW_RSQRT_RECOMPILE
#define USE_NEW_SQRT_RECOMPILE
#define USE_NEW_WAITQ_RECOMPILE



#define USE_NEW_CLIP_RECOMPILE



#define USE_NEW_ADD_RECOMPILE
#define USE_NEW_ADDi_RECOMPILE
#define USE_NEW_ADDq_RECOMPILE
#define USE_NEW_ADDX_RECOMPILE
#define USE_NEW_ADDY_RECOMPILE
#define USE_NEW_ADDZ_RECOMPILE
#define USE_NEW_ADDW_RECOMPILE



#define USE_NEW_ADDA_RECOMPILE
#define USE_NEW_ADDAi_RECOMPILE
#define USE_NEW_ADDAq_RECOMPILE
#define USE_NEW_ADDAX_RECOMPILE
#define USE_NEW_ADDAY_RECOMPILE
#define USE_NEW_ADDAZ_RECOMPILE
#define USE_NEW_ADDAW_RECOMPILE



#define USE_NEW_SUB_RECOMPILE
#define USE_NEW_SUBi_RECOMPILE
#define USE_NEW_SUBq_RECOMPILE
#define USE_NEW_SUBX_RECOMPILE
#define USE_NEW_SUBY_RECOMPILE
#define USE_NEW_SUBZ_RECOMPILE
#define USE_NEW_SUBW_RECOMPILE

#define USE_NEW_SUBA_RECOMPILE
#define USE_NEW_SUBAi_RECOMPILE
#define USE_NEW_SUBAq_RECOMPILE
#define USE_NEW_SUBAX_RECOMPILE
#define USE_NEW_SUBAY_RECOMPILE
#define USE_NEW_SUBAZ_RECOMPILE
#define USE_NEW_SUBAW_RECOMPILE



#define USE_NEW_MUL_RECOMPILE
#define USE_NEW_MULi_RECOMPILE
#define USE_NEW_MULq_RECOMPILE
#define USE_NEW_MULX_RECOMPILE
#define USE_NEW_MULY_RECOMPILE
#define USE_NEW_MULZ_RECOMPILE
#define USE_NEW_MULW_RECOMPILE


#define USE_NEW_MULA_RECOMPILE
#define USE_NEW_MULAi_RECOMPILE
#define USE_NEW_MULAq_RECOMPILE
#define USE_NEW_MULAX_RECOMPILE
#define USE_NEW_MULAY_RECOMPILE
#define USE_NEW_MULAZ_RECOMPILE
#define USE_NEW_MULAW_RECOMPILE



#define USE_NEW_MADD_RECOMPILE
#define USE_NEW_MADDi_RECOMPILE
#define USE_NEW_MADDq_RECOMPILE
#define USE_NEW_MADDX_RECOMPILE
#define USE_NEW_MADDY_RECOMPILE
#define USE_NEW_MADDZ_RECOMPILE

// ??
#define USE_NEW_MADDW_RECOMPILE



#define USE_NEW_MADDA_RECOMPILE
#define USE_NEW_MADDAi_RECOMPILE
#define USE_NEW_MADDAq_RECOMPILE
#define USE_NEW_MADDAX_RECOMPILE
#define USE_NEW_MADDAY_RECOMPILE
#define USE_NEW_MADDAZ_RECOMPILE
#define USE_NEW_MADDAW_RECOMPILE


#define USE_NEW_MSUB_RECOMPILE
#define USE_NEW_MSUBi_RECOMPILE
#define USE_NEW_MSUBq_RECOMPILE
#define USE_NEW_MSUBX_RECOMPILE
#define USE_NEW_MSUBY_RECOMPILE
#define USE_NEW_MSUBZ_RECOMPILE
#define USE_NEW_MSUBW_RECOMPILE


#define USE_NEW_MSUBA_RECOMPILE
#define USE_NEW_MSUBAi_RECOMPILE
#define USE_NEW_MSUBAq_RECOMPILE
#define USE_NEW_MSUBAX_RECOMPILE
#define USE_NEW_MSUBAY_RECOMPILE
#define USE_NEW_MSUBAZ_RECOMPILE
#define USE_NEW_MSUBAW_RECOMPILE


#define USE_NEW_OPMSUB_RECOMPILE
#define USE_NEW_OPMULA_RECOMPILE



#define USE_NEW_RXOR_RECOMPILE
#define USE_NEW_RGET_RECOMPILE
#define USE_NEW_RINIT_RECOMPILE
#define USE_NEW_RNEXT_RECOMPILE

#define USE_NEW_XTOP_RECOMPILE
#define USE_NEW_XITOP_RECOMPILE
#define USE_NEW_FCSET_RECOMPILE
#define USE_NEW_FSSET_RECOMPILE



#define USE_NEW_FCGET_RECOMPILE
#define USE_NEW_FCAND_RECOMPILE
#define USE_NEW_FCEQ_RECOMPILE
#define USE_NEW_FCOR_RECOMPILE


#define USE_NEW_FMAND_RECOMPILE
#define USE_NEW_FMEQ_RECOMPILE
#define USE_NEW_FMOR_RECOMPILE
#define USE_NEW_FSAND_RECOMPILE
#define USE_NEW_FSEQ_RECOMPILE
#define USE_NEW_FSOR_RECOMPILE



#define USE_NEW_LQ_RECOMPILE
#define USE_NEW_LQI_RECOMPILE
#define USE_NEW_LQD_RECOMPILE


#define USE_NEW_ILW_RECOMPILE
#define USE_NEW_ILWR_RECOMPILE


#define USE_NEW_SQ_RECOMPILE
#define USE_NEW_SQI_RECOMPILE
#define USE_NEW_SQD_RECOMPILE

#define USE_NEW_ISW_RECOMPILE
#define USE_NEW_ISWR_RECOMPILE


#define USE_NEW_B_RECOMPILE
#define USE_NEW_BAL_RECOMPILE
#define USE_NEW_JALR_RECOMPILE
#define USE_NEW_JR_RECOMPILE

#define USE_NEW_IBEQ_RECOMPILE
#define USE_NEW_IBNE_RECOMPILE
#define USE_NEW_IBLTZ_RECOMPILE
#define USE_NEW_IBLEZ_RECOMPILE
#define USE_NEW_IBGTZ_RECOMPILE
#define USE_NEW_IBGEZ_RECOMPILE



#define CHECK_EVENT_AFTER_START



#define ENABLE_BRANCH_DELAY_RECOMPILE
//#define ENABLE_EBIT_RECOMPILE
#define ENABLE_MBIT_RECOMPILE


#define ENABLE_NOP_HI
#define ENABLE_NOP_LO



// *** remove this when done testing ***
//#define ENABLE_SINGLE_STEP
//#define ENABLE_SINGLE_STEP_BEFORE



#define CACHE_NOT_IMPLEMENTED


// test pc arg pass, new methodology etc
//#define TEST_NEW_RECOMPILE


// check that instructions in cached-region were not modified since last recompile
//#define CHECK_CACHED_INSTRUCTIONS


//#define USE_MEMORYPTR_FOR_CACHED_REGION


//#define USE_GETPTR_FOR_CACHED_REGION


// theoretically, anything in BIOS is read-only
//#define DONT_CHECK_BIOS_INSTRUCTIONS



//#define INCLUDE_ICACHE_RELOAD




//#define ALWAYS_USE_MEMORYPTR_FOR_ENCODING


#define ENCODE_SINGLE_RUN_PER_BLOCK


#define UPDATE_BEFORE_RETURN


// crashes unless you do this ?? Compiler dependent?
#define RESERVE_STACK_FRAME_FOR_CALL


//#define ENABLE_AUTONOMOUS_BRANCH_U
//#define ENABLE_AUTONOMOUS_BRANCH_C


//#define VERBOSE_RECOMPILE
//#define VERBOSE_RECOMPILE_MBIT


Vu::Recompiler *Vu::Recompiler::_REC;



/*
u32 Recompiler::v->SetStatus_Flag;
u32 Recompiler::v->SetClip_Flag;

Vu::Instruction::Format Recompiler::instLO;
Vu::Instruction::Format Recompiler::instHI;
Vu::Instruction::Format Recompiler::v->NextInstLO;


x64Encoder *Recompiler::e;
//VU *Vu::Recompiler::r;
s32 Vu::Recompiler::v->OpLevel;
u32 Vu::Recompiler::LocalPC;
u32 Vu::Recompiler::Local_LastModifiedReg;
u32 Vu::Recompiler::Local_NextPCModified;

u32 Vu::Recompiler::CurrentCount;

u32 Vu::Recompiler::isBranchDelaySlot;
u32 Vu::Recompiler::isLoadDelaySlot;

u32 Vu::Recompiler::v->bStopEncodingAfter;
u32 Vu::Recompiler::v->bStopEncodingBefore;


//u32 Recompiler::Local_DelaySlot;
//u32 Recompiler::Local_DelayType;
//u32 Recompiler::Local_DelayCount;
//u32 Recompiler::Local_DelayCond;
//u32 Recompiler::Local_Condition;
Vu::Instruction::Format Recompiler::NextInst;

//Recompiler::RDelaySlot Recompiler::RDelaySlots [ 2 ];
//u32 Recompiler::DSIndex;
//u32 Recompiler::RDelaySlots_Valid;

u64 Recompiler::MemCycles;

u64 Recompiler::LocalCycleCount;
u64 Recompiler::CacheBlock_CycleCount;

bool Recompiler::bIsBlockInICache;

u32 Recompiler::bResetCycleCount;


u32 Recompiler::CurrentBlock_StartAddress;
u32 Recompiler::NextBlock_StartAddress;

//static u32* Recompiler::pForwardBranchTargets;
u32 Recompiler::ForwardBranchIndex;

//static u8** Recompiler::pPrefix_CodeStart;
//static u8** Recompiler::pCodeStart;
//static u32* Recompiler::CycleCount;

//static u32 Recompiler::ulIndex_Mask;
//static u32 Recompiler::MaxStep;
//static u32 Recompiler::MaxStep_Shift;
//static u32 Recompiler::MaxStep_Mask;

u32 Recompiler::StartBlockIndex;
u32 Recompiler::BlockIndex;

u32 Recompiler::v->Status_BranchDelay;
u32 Recompiler::v->Status_BranchConditional;
Vu::Instruction::Format Recompiler::v->Status_BranchInstruction;

u32 Recompiler::Status_EBit;
*/

// lookup table for op shuffles
u8 _op_shuffle_lut_1 [ 16 ] = {
0x00, // 0x0000
0x00, // 0x0001
0x55, // 0x0010
0x50, // 0x0011

0xa0, // 0x0100
0xa0, // 0x0101
0xa5, // 0x0110
0x00, // 0x0111

0xf0, // 0x1000
0xf0, // 0x1001
0xf5, // 0x1010
0x00, // 0x1011

0xfa, // 0x1100
0x00, // 0x1101
0x00, // 0x1110
0x00, // 0x1111
};

u8 _op_shuffle_lut_2 [ 16 ] = {
0x00, // 0x0000
0x0d, // 0x0001
0x0d, // 0x0010
0x0d, // 0x0011

0x30, // 0x0100
0x31, // 0x0101
0x34, // 0x0110
0x00, // 0x0111

0xc0, // 0x1000
0xc1, // 0x1001
0xc4, // 0x1010
0x00, // 0x1011

0xd0, // 0x1100
0x00, // 0x1101
0x00, // 0x1110
0x00, // 0x1111
};

u8 _op_add_shuffle_lut_2 [ 16 ] = {
0x00, // 0x0000
0x0a, // 0x0001
0x0a, // 0x0010
0x08, // 0x0011

0x20, // 0x0100
0x20, // 0x0101
0x20, // 0x0110
0x00, // 0x0111

0x80, // 0x1000
0x80, // 0x1001
0x80, // 0x1010
0x00, // 0x1011

0x80, // 0x1100
0x00, // 0x1101
0x00, // 0x1110
0x00, // 0x1111
};


// constructor
// NumberOfBlocks MUST be a power of 2, so 1 would mean 2, 2 would mean 4
Recompiler::Recompiler ( VU* v, u32 NumberOfBlocks, u32 BlockSize_PowerOfTwo, u32 MaxIStep_Shift )
{
	
	BlockSize = 1 << BlockSize_PowerOfTwo;
	
	MaxStep_Shift = MaxIStep_Shift;
	MaxStep = 1 << MaxIStep_Shift;
	MaxStep_Mask = MaxStep - 1;
	
	NumBlocks = 1 << NumberOfBlocks;
	NumBlocks_Mask = NumBlocks - 1;
	
	// need a mask for referencing each encoded instruction
	ulIndex_Mask = 1 << ( NumberOfBlocks + MaxIStep_Shift );
	ulIndex_Mask -= 1;
	
	// allocate variables
	//StartAddress = new u32 [ NumBlocks ];
	//RunCount = new u8 [ NumBlocks ];
	//MaxCycles = new u64 [ NumBlocks ];
	//Instructions = new u32 [ NumBlocks * MaxStep ];
	
	// only need to compare the starting address of the entire block
	StartAddress = new u32 [ NumBlocks ];
	pCodeStart = new u8* [ NumBlocks * MaxStep ];
	CycleCount = new u32 [ NumBlocks * MaxStep ];
	//EndAddress = new u32 [ NumBlocks * MaxStep ];
	
	pForwardBranchTargets = new u32 [ MaxStep ];
	
	// used internally by recompiler (in case it branches to a load/store or another branch, etc, then need to go to prefix instead)
	pPrefix_CodeStart = new u8* [ MaxStep ];
	
	
	// create the encoder
	//e = new x64Encoder ( BlockSize_PowerOfTwo, NumBlocks );
	InstanceEncoder = new x64Encoder ( BlockSize_PowerOfTwo, NumBlocks );
	
	e = InstanceEncoder;
	
	/*
	// set the "alternate stream" with the code to clear a block
	// which should just simply return 1 (meaning to recompile the block)
	e->SwitchToAlternateStream ();
	e->MovReg32ImmX ( RAX, 1 );
	e->Ret ();
	
	// we're done in the "alternate stream"
	e->SwitchToLiveStream ();
	
	// I'd like to know the size of the code
	cout << "\nSize of alternate stream in bytes: " << dec << e->lAlternateStreamSize << " Alt Stream=" << hex << e->ullAlternateStream;
	
	// reset all the blocks
	//for ( int i = 0; i < NumBlocks; i++ ) InitBlock ( i );
	for ( int i = 0; i < NumBlocks; i++ ) e->Emit_AltStreamToBlock8 ( i );
	*/
	
	//cout << "\nAfter clear, live code stream=" << hex << (((u64*)e->LiveCodeArea) [ 0 ]);
	//cout << "\n#2=" << hex << ((u64*)e->LiveCodeArea) [ ( ( (NumBlocks-1) & e->lCodeBlockSize_Mask ) << e->lCodeBlockSize_PowerOfTwo ) >> 3 ];
	//cout << " offset=" << hex << ( ( ( 2 & e->lCodeBlockSize_Mask ) << e->lCodeBlockSize_PowerOfTwo ) >> 3 );
	
	// testing
	//pCodeStart [ 0x27e4 >> 2 ] = 0x5373b36;
	
	//r = R5900Cpu;
	
	Reset ();
}


// destructor
Recompiler::~Recompiler ()
{
	delete e;
	
	delete StartAddress;
	
	delete pPrefix_CodeStart;
	delete pCodeStart;
	delete CycleCount;
	delete pForwardBranchTargets;
	
	// delete the variables that were allocated
	/*
	delete StartAddress;
	delete RunCount;
	delete MaxCycles;
	delete Instructions;
	*/
}


void Recompiler::Reset ()
{
	//memset ( this, 0, sizeof( Recompiler ) );	
	// initialize the address and instruction so it is known that it does not refer to anything
	memset ( pForwardBranchTargets, 0x00, sizeof( u32 ) * MaxStep );
	memset ( pPrefix_CodeStart, 0x00, sizeof( u8* ) * MaxStep );
	memset ( StartAddress, 0xff, sizeof( u32 ) * NumBlocks );
	memset ( pCodeStart, 0x00, sizeof( u8* ) * NumBlocks * MaxStep );
	memset ( CycleCount, 0x00, sizeof( u32 ) * NumBlocks * MaxStep );
	
#ifdef ENABLE_ICACHE
	// reset invalidate arrays
	r->Bus->Reset_Invalidate ();
#endif
}


/** 
 * @fn static void Calc_Checksum( VU *v )
 * @brief calculate checksum for source of recompiled code and store into "ullChecksum"
 * @param v is a pointer into the VU object state that holds the source code that was recompiled
 * @return 64-bit checksum calculated from source vu code mem at the current point in time
 */
u64 Recompiler::Calc_Checksum( VU *v )
{
	u32* pSrcPtr32;
	u64 ullAddress;
	u64 ullCode;
	u64 ullCurChecksum;

	u64 ullLoopCount;

	// get pointer into the vu memory starting from beginning
	pSrcPtr32 = RGetPointer ( v, 0 );

	// init checksum
	ullCurChecksum = 0;

	// determine if this is vu0 or vu1
	// get the size of vu code mem based vu number etc
	ullLoopCount = v->ulVuMem_Size >> 2;

	// calculate the check sum
	for ( ullAddress = 0; ullAddress < ullLoopCount; ullAddress++ )
	{
		// get the source code value
		ullCode = (u64) ( *pSrcPtr32++ );

		// multiply by the address
		ullCode *= ( ullAddress + 1 );

		ullCurChecksum += ullCode;
	}

	// go ahead and return check sum
	return ullCurChecksum;
}


// returns 1 if it is ok to have instruction in branch delay slot when recompiling, zero otherwise
bool Recompiler::isBranchDelayOk ( u32 ulInstruction, u32 Address )
{
#ifdef ENCODE_ALL_POSSIBLE_DELAYSLOTS

	u32 ulOpcode, ulFunction;
	
	ulOpcode = ulInstruction >> 26;
	
	
	// second row starting at second column is ok
	if ( ulOpcode >= 0x9 && ulOpcode <= 0xf )
	{

		// constant instructions that will never interrupt //
		//cout << "\nAddress=" << hex << Address << " Opcode=" << ulOpcode;
		return true;
	}
	
	
	
	// for R5900, DADDIU is ok //
	if ( ulOpcode == 0x19 )
	{
		return true;
	}
	
	
	
	// check special instructions
	if ( !ulOpcode )
	{
		ulFunction = ulInstruction & 0x3f;
		
		// first row is mostly ok
		if ( ( ( ulFunction >> 3 ) == 0 ) && ( ulFunction != 1 ) && ( ulFunction != 5 ) )
		{
			return true;
		}
		
		// row 4 is ok except for column 0 and column 2
		if ( ( ulFunction >> 3 ) == 4 && ulFunction != 0x20 && ulFunction != 0x22 )
		{
			return true;
		}
		
		// in row 5 for R3000A columns 2 and 3 are ok
		if ( ulFunction == 0x2a || ulFunction == 0x2b )
		{
			return true;
		}
		
		
		// in row 5 for R5900 columns 5 and 7 are ok //
		if ( ulFunction == 0x2d || ulFunction == 0x2f )
		{
			return true;
		}
		
		
		// for R5900, on row 2 MOVZ and MOVN are ok //
		if ( ulFunction == 0xa || ulFunction == 0xb )
		{
			return true;
		}
		
		
		// for R5900, on row 3 DSLLV, DSRLV, DSRAV or ok //
		if ( ulFunction == 0x14 || ulFunction == 0x16 || ulFunction == 0x17 )
		{
			return true;
		}
		
		// in last row for R5900, all is ok except columns 1 and 5 //
		if ( ( ulFunction >> 3 ) == 7 && ulFunction != 0x39 && ulFunction != 0x3d )
		{
			return true;
		}
		
	}
	
	// will leave out all store instructions for now to play it safe //
	
#else
	if ( !ulInstruction ) return true;
#endif
	
	return false;
}




// force pipeline to wait for the Q register
//void VU::PipelineWaitQ ()
//{
//	PipelineWaitCycle ( QBusyUntil_Cycle );
//	if ( QBusyUntil_Cycle != -1LL )
//	{
//		SetQ ();
//	}
//}

void Recompiler::PipelineWaitQ ( VU* v )
{
	if ( v->CycleCount < v->QBusyUntil_Cycle )
	{
		v->PipelineWaitCycle ( v->QBusyUntil_Cycle );
	}
	
	if ( v->QBusyUntil_Cycle != -1LL )
	{
		v->SetQ ();
	}
}


//inline u64 TestStall ()
//{
//	return TestBitmap ( Pipeline_Bitmap, SrcRegs_Bitmap );
//}

//inline u64 TestStall_INT ()
//{
//	// test integer pipeline for a stall (like if the integer register is not loaded yet)
//	return Int_Pipeline_Bitmap & Int_SrcRegs_Bitmap;
//}


void Recompiler::TestStall ( VU* v )
{
	if ( v->TestStall () )
	{
		// FMAC pipeline stall //
		v->PipelineWait_FMAC ();
	}
}


void Recompiler::TestStall_INT ( VU* v )
{
	if ( v->TestStall_INT () )
	{
		// Integer pipeline stall //
		v->PipelineWait_INT ();
	}
}


bool Recompiler::isNopHi ( Vu::Instruction::Format i )
{
#ifdef ENABLE_NOP_HI
	// check for NOP instruction
	if ( ( i.Funct == 0x3f ) && ( i.Fd == 0xb ) )
	{
		return true;
	}
#endif
	
	return false;
}


bool Recompiler::isNopLo ( Vu::Instruction::Format i )
{
#ifdef ENABLE_NOP_LO
	// check for mov with no xyzw
	if ( i.Opcode == 0x40 )
	{
		if ( ( i.Funct == 0x3c ) && ( i.Fd == 0xc ) )
		{
			// mov instruction //
			
			// if no xyzw is getting moved or if destination is zero register then is NOP
			if ( !i.xyzw || !i.Ft )
			{
				return true;
			}
		}
	}
#endif
	
	return false;
}



void Recompiler::SetDstBitmap_rec ( x64Encoder *e, VU* v, u64 b0, u64 b1, u64 i0 )
{
	//v->FlagSave [ v->iFlagSave_Index & VU::c_lFlag_Delay_Mask ].Bitmap.b0 = b0;
	//v->FlagSave [ v->iFlagSave_Index & VU::c_lFlag_Delay_Mask ].Bitmap.b1 = b1;
	//v->FlagSave [ v->iFlagSave_Index & VU::c_lFlag_Delay_Mask ].Int_Bitmap = i0;
	//v->Pipeline_Bitmap.b0 |= b0;
	//v->Pipeline_Bitmap.b1 |= b1;

	if ( b0 | b1 | i0 )
	{
	e->MovRegMem32 ( RAX, (long*) & v->iFlagSave_Index );
	//e->MovRegImm64 ( RCX, (u64) & v->FlagSave );
	e->LeaRegMem64 ( RCX, & v->FlagSave );
	//e->IncReg32 ( RAX );
	e->AndReg32ImmX ( RAX, VU::c_lFlag_Delay_Mask );
	e->ShlRegImm32 ( RAX, 2 );
	}
	
	/*
	if ( i0 )
	{
	e->MovRegImm64 ( RDX, i0 );
	e->MovRegToMem64 ( RDX, RCX, RAX, SCALE_EIGHT, 8 );
	}
	*/
	
	if ( i0 & 0xffffffffULL )
	{
		e->MovMemImm32 ( (u32) ( i0 & 0xffffffffULL ), RCX, RAX, SCALE_EIGHT, 8 );
	}

	if ( i0 >> 32 )
	{
		e->MovMemImm32 ( (u32) ( i0 >> 32 ), RCX, RAX, SCALE_EIGHT, 12 );
		
		// if an integer register is in process (like if it is loading), then need to add into pipeline
		e->OrMemImm32 ( ( ( (s32*) & v->Int_Pipeline_Bitmap ) + 1 ), (s32) ( i0 >> 32 ) );
	}
	
	if ( b0 | b1 )
	{
		_REC->VectorConstants[ _REC->CountOfVConsts ].uq0 = b0;
		_REC->VectorConstants[ _REC->CountOfVConsts ].uq1 = b1;
		
		e->movdqa_regmem ( RAX, & _REC->VectorConstants[ _REC->CountOfVConsts ].uq0 );
		e->movdqa_to_mem128 ( RAX, RCX, RAX, SCALE_EIGHT, 16 );
		
		// also need to add to pipeline bitmap
		e->porregmem ( RAX, & v->Pipeline_Bitmap.b0 );
		e->movdqa_memreg ( & v->Pipeline_Bitmap.b0, RAX );

		_REC->CountOfVConsts++;
	}
	
	/*
	if ( b0 )
	{
	e->MovRegImm64 ( RDX, b0 );
	e->MovRegToMem64 ( RDX, RCX, RAX, SCALE_EIGHT, 16 );
	e->OrMemReg64 ( & v->Pipeline_Bitmap.b0, RDX );
	}
	*/

	/*
	if ( b0 & 0xffffffffULL )
	{
		e->MovMemImm32 ( (u32) ( b0 & 0xffffffffULL ), RCX, RAX, SCALE_EIGHT, 16 );
	}

	if ( b0 >> 32 )
	{
		e->MovMemImm32 ( (u32) ( b0 >> 32 ), RCX, RAX, SCALE_EIGHT, 20 );
	}
	*/
	
	/*
	if ( b1 )
	{
	e->MovRegImm64 ( RDX, b1 );
	e->MovRegToMem64 ( RDX, RCX, RAX, SCALE_EIGHT, 16 + 8 );
	e->OrMemReg64 ( & v->Pipeline_Bitmap.b1, RDX );
	}
	*/
	
	/*
	if ( b1 & 0xffffffffULL )
	{
		e->MovMemImm32 ( (u32) ( b1 & 0xffffffffULL ), RCX, RAX, SCALE_EIGHT, 24 );
	}

	if ( b1 >> 32 )
	{
		e->MovMemImm32 ( (u32) ( b1 >> 32 ), RCX, RAX, SCALE_EIGHT, 28 );
	}
	*/
}


void Recompiler::SetDstBitmap ( VU* v, u64 b0, u64 b1, u64 i0 )
{
	v->FlagSave [ v->iFlagSave_Index & VU::c_lFlag_Delay_Mask ].Bitmap.b0 = b0;
	v->FlagSave [ v->iFlagSave_Index & VU::c_lFlag_Delay_Mask ].Bitmap.b1 = b1;
	v->FlagSave [ v->iFlagSave_Index & VU::c_lFlag_Delay_Mask ].Int_Bitmap = i0;
	
	v->Pipeline_Bitmap.b0 |= b0;
	v->Pipeline_Bitmap.b1 |= b1;
	
}



//inline void UpdateQ ()
//{
//	if ( CycleCount >= QBusyUntil_Cycle )
//	{
//		// set the q register
//		SetQ ();
//	}
//}

//inline void UpdateP ()
//{
//	if ( CycleCount == PBusyUntil_Cycle )
//	{
//		// set the p register
//		SetP ();
//	}
//}


//inline void SetQ ()
//{
//	// set the Q register
//	vi [ REG_Q ].s = NextQ.l;
//	
//	// clear non-sticky div unit flags
//	vi [ REG_STATUSFLAG ].uLo &= ~0x30;
//	
//	// set flags
//	vi [ REG_STATUSFLAG ].uLo |= NextQ_Flag;
//	
//	// don't set the Q register again until div unit is used again
//	// should clear to zero to indicate last event happened far in the past
//	QBusyUntil_Cycle = -1LL;
//	//QBusyUntil_Cycle = 0LL;
//}

//inline void SetP ()
//{
//	vi [ REG_P ].s = NextP.l;
//	
//	// should set this to zero to indicate it happened far in the past
//	//PBusyUntil_Cycle = -1LL;
//	PBusyUntil_Cycle = 0LL;
//}



void Recompiler::Test_FTOI4 ( VU* v, Vu::Instruction::Format i )
{
//#if defined INLINE_DEBUG_MADDW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
//	VU::debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
//	VU::debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
//	VU::debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
//	VU::debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
//#endif

	//VuUpperOpW ( v, i, PS2_Float_Madd );
	
	if ( ( v->test1_result1.sw0 != v->test2_result1.sw0 )
	|| ( v->test1_result1.sw1 != v->test2_result1.sw1 )
	|| ( v->test1_result1.sw2 != v->test2_result1.sw2 )
	|| ( v->test1_result1.sw3 != v->test2_result1.sw3 ) )
	{
		VU::debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
		VU::debug << " test1_src1= x=" << hex << v->test1_src1.fx << " y=" << v->test1_src1.fy << " z=" << v->test1_src1.fz << " w=" << v->test1_src1.fw;
		VU::debug << " test1_src1(hex)= x=" << hex << v->test1_src1.sw0 << " y=" << v->test1_src1.sw1 << " z=" << v->test1_src1.sw2 << " w=" << v->test1_src1.sw3;
		VU::debug << " test1_result1= x=" << hex << v->test1_result1.sw0 << " y=" << v->test1_result1.sw1 << " z=" << v->test1_result1.sw2 << " w=" << v->test1_result1.sw3;
		VU::debug << " test2_result1= x=" << hex << v->test2_result1.sw0 << " y=" << v->test2_result1.sw1 << " z=" << v->test2_result1.sw2 << " w=" << v->test2_result1.sw3;
	}
	
}


int Recompiler::Prefix_MADDW ( VU* v, Vu::Instruction::Format i )
{
//#if defined INLINE_DEBUG_MADDW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	VU::debug << "\r\n" << hex << "VU#" << v->Number << " " << setw( 8 ) << v->PC << " " << dec << v->CycleCount << " " << Print::PrintInstructionHI ( i.Value ).c_str () << "; " << hex << i.Value;
	VU::debug << " Fs= x=" << hex << v->vf [ i.Fs ].fx << " y=" << v->vf [ i.Fs ].fy << " z=" << v->vf [ i.Fs ].fz << " w=" << v->vf [ i.Fs ].fw;
	VU::debug << " Ft= x=" << hex << v->vf [ i.Ft ].fx << " y=" << v->vf [ i.Ft ].fy << " z=" << v->vf [ i.Ft ].fz << " w=" << v->vf [ i.Ft ].fw;
	VU::debug << " ACC= x=" << v->dACC [ 0 ].f << " y=" << v->dACC [ 1 ].f << " z=" << v->dACC [ 2 ].f << " w=" << v->dACC [ 3 ].f;
//#endif

	//VuUpperOpW ( v, i, PS2_Float_Madd );

	return true;
}


int Recompiler::Postfix_MADDW ( VU* v, Vu::Instruction::Format i )
{

//#if defined INLINE_DEBUG_MADDW || defined INLINE_DEBUG_VU || defined INLINE_DEBUG_FPU	// || defined INLINE_DEBUG_UNIMPLEMENTED
	VU::debug << " Output: Fd=" << " vfx=" << hex << v->vf [ i.Fd ].fx << " vfy=" << v->vf [ i.Fd ].fy << " vfz=" << v->vf [ i.Fd ].fz << " vfw=" << v->vf [ i.Fd ].fw;
	//VU::debug << " MAC=" << v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].MACFlag << " STATF=" << v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].StatusFlag;
	VU::debug << " MAC=" << v->vi [ VU::REG_MACFLAG ].uLo << " STATF=" << v->vi [ VU::REG_STATUSFLAG ].uLo;
//#endif

	return true;
}


void Recompiler::AdvanceCycle_rec ( x64Encoder *e, VU* v )
{
#ifdef VERBOSE_RECOMPILE
cout << " AdvanceCycle_rec";
cout << " VU#" << v->Number;
cout << " eOffset=" << dec << e->x64NextOffset;
#endif

	e->MovRegMem64 ( RAX, (long long*) & v->CycleCount );
	e->IncReg64 ( RAX );

	//if ( CycleCount >= QBusyUntil_Cycle )
	e->CmpRegMem64 ( RAX, (long long*) & v->QBusyUntil_Cycle );
	e->Jmp8_B ( 0, 0 );

#ifdef VERBOSE_RECOMPILE
cout << " jump1";
#endif
	
	//QBusyUntil_Cycle = -1LL;
	e->MovMemImm64 ( (long long*) & v->QBusyUntil_Cycle, -1 );

#ifdef VERBOSE_RECOMPILE
cout << " checkpoint1";
#endif
	
	//vi [ REG_Q ].s = NextQ.l;
	e->MovRegMem32 ( RCX, & v->NextQ.l );
	e->MovMemReg32 ( & v->vi [ VU::REG_Q ].s, RCX );

#ifdef VERBOSE_RECOMPILE
cout << " checkpoint2";
#endif
	
	//vi [ REG_STATUSFLAG ].uLo &= ~0x30;
	e->MovRegMem32 ( RDX, & v->vi [ VU::REG_STATUSFLAG ].s );
	e->AndReg32ImmX ( RDX, ~0x30 );

#ifdef VERBOSE_RECOMPILE
cout << " checkpoint3";
#endif
	
	//vi [ REG_STATUSFLAG ].uLo |= NextQ_Flag;
	e->OrRegMem16 ( RDX, (short*) & v->NextQ_Flag );

#ifdef VERBOSE_RECOMPILE
cout << " checkpoint4";
#endif
	
	e->MovMemReg32 ( & v->vi [ VU::REG_STATUSFLAG ].s, RDX );
	
	e->SetJmpTarget8 ( 0 );

#ifdef VERBOSE_RECOMPILE
cout << " land1";
#endif
	
	// write back CycleCount (for now)
	e->MovMemReg64 ( (long long*) & v->CycleCount, RAX );
	
	//v->iFlagSave_Index++;
	//int FlagIndex = ( v->iFlagSave_Index ) & VU::c_lFlag_Delay_Mask;
	e->MovRegMem32 ( RAX, (long*) & v->iFlagSave_Index );
	e->LeaRegMem64 ( RCX, & v->FlagSave );
	e->IncReg32 ( RAX );
	e->MovMemReg32 ( (long*) & v->iFlagSave_Index, RAX );
	e->AndReg32ImmX ( RAX, VU::c_lFlag_Delay_Mask );

	
#ifdef ENABLE_VFLAGS
	e->LeaRegMem64 ( RDX, & v->vFlagSave );
	e->AddRegReg32 ( RAX, RAX );
	e->movdqa_regmem ( RAX, & v->CurrentVFlags.Value0 );
	e->movdqa_to_mem128 ( RAX, RDX, RAX, SCALE_EIGHT, 0 );
	e->AddRegReg32 ( RAX, RAX );
#else
	e->ShlRegImm32 ( RAX, 2 );
	//v->FlagSave [ FlagIndex ].StatusFlag = v->vi [ VU::REG_STATUSFLAG ].u;
	e->MovRegMem32 ( RDX, & v->vi [ VU::REG_STATUSFLAG ].s );
	e->MovRegToMem16 ( RDX, RCX, RAX, SCALE_EIGHT, 0 );

	//v->FlagSave [ FlagIndex ].MACFlag = v->vi [ VU::REG_MACFLAG ].u;
	e->MovRegMem32 ( RDX, & v->vi [ VU::REG_MACFLAG ].s );
	e->MovRegToMem16 ( RDX, RCX, RAX, SCALE_EIGHT, 2 );
#endif
	
	//v->FlagSave [ FlagIndex ].ClipFlag = v->vi [ VU::REG_CLIPFLAG ].u;
	e->MovRegMem32 ( RDX, & v->vi [ VU::REG_CLIPFLAG ].s );
	e->MovRegToMem32 ( RDX, RCX, RAX, SCALE_EIGHT, 4 );
	
	//v->RemovePipeline ();
	e->movdqa_from_mem128 ( RAX, RCX, RAX, SCALE_EIGHT, 16 );
	e->pandnregmem ( RAX, & v->Pipeline_Bitmap );
	e->movdqa_memreg ( & v->Pipeline_Bitmap, RAX );
	
	//VU::ClearBitmap ( v->FlagSave [ FlagIndex ].Bitmap );
	e->pxorregreg ( RAX, RAX );
	e->movdqa_to_mem128 ( RAX, RCX, RAX, SCALE_EIGHT, 16 );
	
	//v->Int_Pipeline_Bitmap &= ~v->FlagSave [ FlagIndex ].Int_Bitmap;
	e->MovRegFromMem64 ( RDX, RCX, RAX, SCALE_EIGHT, 8 );
	e->NotReg64 ( RDX );
	e->AndMemReg64 ( (long long*) & v->Int_Pipeline_Bitmap, RDX );
	
	//v->FlagSave [ FlagIndex ].Int_Bitmap = 0;
	e->MovMemImm64 ( 0, RCX, RAX, SCALE_EIGHT, 8 );
	
#ifdef VERBOSE_RECOMPILE
cout << "->AdvanceCycle_rec_DONE";
#endif
}


void Recompiler::AdvanceCycle ( VU* v )
{
	v->CycleCount++;

	// update q and p registers here for now
	v->UpdateQ ();
	//v->UpdateP ();
	
	v->iFlagSave_Index++;
	
	// set the flags
	int FlagIndex = ( v->iFlagSave_Index ) & VU::c_lFlag_Delay_Mask;

//#ifdef ENABLE_SNAPSHOTS
#ifdef ENABLE_VFLAGS
	v->vFlagSave [ FlagIndex ].Value0 = v->CurrentVFlags.Value0;
	v->vFlagSave [ FlagIndex ].Value1 = v->CurrentVFlags.Value1;
#else
	v->FlagSave [ FlagIndex ].StatusFlag = v->vi [ VU::REG_STATUSFLAG ].u;
	v->FlagSave [ FlagIndex ].MACFlag = v->vi [ VU::REG_MACFLAG ].u;
#endif

	v->FlagSave [ FlagIndex ].ClipFlag = v->vi [ VU::REG_CLIPFLAG ].u;

//#ifdef ENABLE_STALLS
	// for now, will process bitmap on every instruction
	
	// remove bitmap from pipeline
	//RemoveBitmap ( Pipeline_Bitmap, FlagSave [ FlagIndex ].Bitmap );
	v->RemovePipeline ();
	
	// need to clear the bitmap for the entry for now
	VU::ClearBitmap ( v->FlagSave [ FlagIndex ].Bitmap );
	
	// remove from MACRO pipeline also
	v->Int_Pipeline_Bitmap &= ~v->FlagSave [ FlagIndex ].Int_Bitmap;
	v->FlagSave [ FlagIndex ].Int_Bitmap = 0;
//#endif

//#ifdef INLINE_DEBUG_ADVANCE_CYCLE
//	VU::debug << "\r\nhps2x64: VU#" << dec << v->Number << ": ADVANCE_CYCLE_REC: P0=" << hex << v->Pipeline_Bitmap.b0 << " P1=" << v->Pipeline_Bitmap.b1 << " S0=" << v->SrcRegs_Bitmap.b0 << " S1=" << v->SrcRegs_Bitmap.b1 << " PC=" << v->PC << " Cycle#" << dec << v->CycleCount << "\r\n";
//	VU::debug << " fi=" << (( v->iFlagSave_Index - 4 ) & v->c_lFlag_Delay_Mask);
//	VU::debug << " fb0= " << hex << v->FlagSave [ 0 ].Bitmap.b0 << " " << v->FlagSave [ 0 ].Bitmap.b1;
//	VU::debug << " fb1= " << v->FlagSave [ 1 ].Bitmap.b0 << " " << v->FlagSave [ 1 ].Bitmap.b1;
//	VU::debug << " fb2= " << v->FlagSave [ 2 ].Bitmap.b0 << " " << v->FlagSave [ 2 ].Bitmap.b1;
//	VU::debug << " fb3= " << v->FlagSave [ 3 ].Bitmap.b0 << " " << v->FlagSave [ 3 ].Bitmap.b1;
//#endif

}



long Recompiler::ProcessBranch ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{

#ifdef ENABLE_PIPELINE_CLEAR_ON_BRANCH
	// clear the pipeline when branching?
	//***todo***
	//ClearFullPipeline ();
	
	// cycle time to refill the pipeline ?
	//***todo***
	//CycleCount += 6;
#endif

	switch ( i.Opcode )
	{
		// B
		case 0x20:
			
		// BAL
		case 0x21:
	
		// IBEQ
		case 0x28:
		
		// IBGEZ
		case 0x2f:
		
		// IBGTZ
		case 0x2d:
		
		// IBLEZ
		case 0x2e:
		
		// IBLTZ
		case 0x2c:
		
		// IBNE
		case 0x29:
		
			e->MovMemImm32 ( (long*) & v->NextPC, ( Address + ( i.Imm11 << 3 ) ) & v->ulVuMem_Mask );
			
			break;
			
			
		// JALR
		case 0x25:
		
		// JR
		case 0x24:
		
			// it must be multiplying the register address by 8 before jumping to it
			//NextPC = d->Data << 3;
			e->MovRegMem32 ( RAX, (long*) & v->Recompiler_BranchDelayAddress );
			e->ShlRegImm32 ( RAX, 3 );
			
			// make sure that address is not outside range (no memory management on VU)
			//NextPC &= ulVuMem_Mask;
			e->AndReg32ImmX ( RAX, v->ulVuMem_Mask );
			e->MovMemReg32 ( (long*) & v->NextPC, RAX );
			
			break;
	}
	
	// return
	return e->Ret ();
}


// if block has no recompiled code, then it should return 1 (meaning to recompile the instruction(s))
u32 Recompiler::InitBlock ( u32 Block )
{
	// set the encoder to use
	e = InstanceEncoder;

	// start encoding in block
	e->StartCodeBlock ( Block );
	
	// set return value 1 (return value for X64 goes in register A)
	e->LoadImm32 ( RAX, 1 );
	
	// return
	e->x64EncodeReturn ();
	
	// done encoding in block
	e->EndCodeBlock ();

	return true;
}


/** 
 * @fn static u32* RGetPointer ( VU *v, u32 Address )
 * @brief return a point into PROGRAM memory for vu at address
 * @param v is a pointer into the VU object state that you want the pointer for
 * @param Address is the address into vu program memory for that vu to get the pointer for
 * @return pointer into specified vu memory for the specified Address
 */

u32* Recompiler::RGetPointer ( VU *v, u32 Address )
{
#ifdef VERBOSE_RECOMPILE
cout << "\nRGetPointer: NON-CACHED";
#endif

	return (u32*) & v->MicroMem64 [ ( Address & v->ulVuMem_Mask ) >> 3 ];
}



/*
static u64 Recompiler::GetFSourceRegsHI_LoXYZW ( Vu::Instruction::Format i )
{
	u64 ullTemp;
	
	if ( ( i.Funct >> 2 ) == 0xf )
	{
		switch ( i.Fd )
		{
			// ft is the destination only on ABS/FTOI/ITOF
			case 0x4:
			case 0x5:
			case 0x7:
				if ( i.Fs < 16 )
				{
					return ( ( ( (u64) i.xyzw ) << ( i.Fs << 2 ) ) & ~0xfULL );
				}
				
				return 0;
				break;
				
			default:
				//return ( ( 1 << i.Fs ) | ( 1 << i.Ft ) );
				break;
		}
	}
	
	ullTemp = 0;
	if ( i.Fs < 16 )
	{
		ullTemp |= ( ( (u64) i.xyzw ) << ( i.Fs << 2 ) );
	}

	if ( i.Ft < 16 )
	{
		ullTemp |= ( ( (u64) i.xyzw ) << ( i.Ft << 2 ) );
	}
	
	// otherwise, for the HI instructions both fs and ft should be source registers
	return ullTemp & ~0xfULL;
}



static u64 Recompiler::GetFSourceRegsHI_HiXYZW ( Vu::Instruction::Format i )
{
	u64 ullTemp;
	
	if ( ( i.Funct >> 2 ) == 0xf )
	{
		switch ( i.Fd )
		{
			// ft is the destination only on ABS/FTOI/ITOF
			case 0x4:
			case 0x5:
			case 0x7:
				if ( i.Fs >= 16 )
				{
					return ( ( ( (u64) i.xyzw ) << ( ( i.Fs - 16 ) << 2 ) ) );
				}
				
				return 0;
				break;
				
			default:
				//return ( ( 1 << i.Fs ) | ( 1 << i.Ft ) );
				break;
		}
	}
	
	ullTemp = 0;
	if ( i.Fs >= 16 )
	{
		ullTemp |= ( ( (u64) i.xyzw ) << ( ( i.Fs - 16 ) << 2 ) );
	}

	if ( i.Ft >= 16 )
	{
		ullTemp |= ( ( (u64) i.xyzw ) << ( ( i.Ft - 16 ) << 2 ) );
	}
	
	// otherwise, for the HI instructions both fs and ft should be source registers
	return ullTemp;
}



static u64 Recompiler::GetFSourceRegsLO_LoXYZW ( Vu::Instruction::Format i )
{
	u64 ullTemp;
	
	switch ( i.Opcode )
	{
		// lower op
		case 0x40:
			switch ( i.Fd )
			{
				// fs source: MOVE,MFP,MR32,MTIR,RINIT,RXOR,SQD,SQI
				case 0xc:
				case 0xd:
				case 0xf:
				case 0x10:
					if ( i.Fs < 16 )
					{
						return ( ( ( (u64) i.xyzw ) << ( i.Fs << 2 ) ) & ~0xfULL );
					}
					
					return 0;
					break;
				
				// fs and ft source: DIV,E,RSQRT,SQRT
				case 0xe:
				case 0x1c:
				case 0x1d:
				case 0x1e:
				case 0x1f:
					ullTemp = 0;
					if ( i.Fs < 16 )
					{
						ullTemp |= ( ( (u64) i.xyzw ) << ( i.Fs << 2 ) );
					}

					if ( i.Ft < 16 )
					{
						ullTemp |= ( ( (u64) i.xyzw ) << ( i.Ft << 2 ) );
					}
					
					// otherwise, for the HI instructions both fs and ft should be source registers
					return ullTemp & ~0xfULL;
					break;
			}
			break;
			
		//sq - fs as source
		case 0x01:
			if ( i.Fs < 16 )
			{
				return ( ( ( (u64) i.xyzw ) << ( i.Fs << 2 ) ) & ~0xfULL );
			}
			
			return 0;
			break;
		
	}
	
	return 0;
}



static u64 Recompiler::GetFSourceRegsLO_HiXYZW ( Vu::Instruction::Format i )
{
	u64 ullTemp;
	
	switch ( i.Opcode )
	{
		// lower op
		case 0x40:
			switch ( i.Fd )
			{
				// fs source: MOVE,MFP,MR32,MTIR,RINIT,RXOR,SQD,SQI
				case 0xc:
				case 0xd:
				case 0xf:
				case 0x10:
					if ( i.Fs >= 16 )
					{
						return ( ( ( (u64) i.xyzw ) << ( ( i.Fs - 16 ) << 2 ) ) & ~0xfULL );
					}
					
					return 0;
					break;
				
				// fs and ft source: DIV,E,RSQRT,SQRT
				case 0xe:
				case 0x1c:
				case 0x1d:
				case 0x1e:
				case 0x1f:
					ullTemp = 0;
					if ( i.Fs >= 16 )
					{
						ullTemp |= ( ( (u64) i.xyzw ) << ( ( i.Fs - 16 ) << 2 ) );
					}

					if ( i.Ft >= 16 )
					{
						ullTemp |= ( ( (u64) i.xyzw ) << ( ( i.Ft - 16 ) << 2 ) );
					}
					
					// otherwise, for the HI instructions both fs and ft should be source registers
					return ullTemp & ~0xfULL;
					break;
			}
			break;
			
		//sq - fs as source
		case 0x01:
			if ( i.Fs >= 16 )
			{
				return ( ( ( (u64) i.xyzw ) << ( ( i.Fs - 16 ) << 2 ) ) & ~0xfULL );
			}
			
			return 0;
			break;
		
	}
	
	return 0;
}




// get the destination registers for hi instruction
static u64 Recompiler::GetFDestRegsHI_LoXYZW ( Vu::Instruction::Format i )
{
	if ( ( i.Funct >> 2 ) == 0xf )
	{
		switch ( i.Fd )
		{
			// ft is the destination only on ABS/FTOI/ITOF
			case 0x4:
			case 0x5:
			case 0x7:
				if ( i.Ft < 16 )
				{
					return ( ( (u64) i.xyzw ) << ( i.Ft << 2 ) ) & ~0xfULL;
				}
				
				return 0;
				break;
				
			// otherwise, the destination is the accumulator
			default:
				return 0;
				break;
		}
	}
	
	
	if ( i.Fd < 16 )
	{
		return ( ( (u64) i.xyzw ) << ( i.Fd << 2 ) ) & ~0xfULL;
	}
	
	// otherwise, for the HI instructions fd should be destination register
	return 0;
}



static u64 Recompiler::GetFDestRegsHI_HiXYZW ( Vu::Instruction::Format i )
{
	if ( ( i.Funct >> 2 ) == 0xf )
	{
		switch ( i.Fd )
		{
			// ft is the destination only on ABS/FTOI/ITOF
			case 0x4:
			case 0x5:
			case 0x7:
				if ( i.Ft >= 16 )
				{
					return ( ( (u64) i.xyzw ) << ( ( i.Ft - 16 ) << 2 ) ) & ~0xfULL;
				}
				
				return 0;
				break;
				
			// otherwise, the destination is the accumulator
			default:
				return 0;
				break;
		}
	}
	
	
	if ( i.Fd >= 16 )
	{
		return ( ( (u64) i.xyzw ) << ( ( i.Fd - 16 ) << 2 ) ) & ~0xfULL;
	}
	
	// otherwise, for the HI instructions fd should be destination register
	return 0;
}



// returns the bitmap for the source registers for instruction
// if the instruction is not supported, then it will return -1ULL
static u64 Recompiler::GetFSourceRegsHI ( Vu::Instruction::Format i )
{
	if ( ( i.Funct >> 2 ) == 0xf )
	{
		switch ( i.Fd )
		{
			// ft is the destination only on ABS/FTOI/ITOF
			case 0x4:
			case 0x5:
			case 0x7:
				return ( 1 << i.Fs );
				break;
				
			default:
				return ( ( 1 << i.Fs ) | ( 1 << i.Ft ) );
				break;
		}
	}
	
	
	// otherwise, for the HI instructions both fs and ft should be source registers
	return ( ( 1 << i.Fs ) | ( 1 << i.Ft ) );
}




// get the destination registers for hi instruction
static u64 Recompiler::GetFDestRegsHI ( Vu::Instruction::Format i )
{
	if ( ( i.Funct >> 2 ) == 0xf )
	{
		switch ( i.Fd )
		{
			// ft is the destination only on ABS/FTOI/ITOF
			case 0x4:
			case 0x5:
			case 0x7:
				return ( 1 << i.Ft );
				break;
				
			// otherwise, the destination is the accumulator
			default:
				return 0;
				break;
		}
	}
	
	
	// otherwise, for the HI instructions fd should be destination register
	return ( 1 << i.Fd );
}



static u64 Recompiler::GetFSourceRegsLO ( Vu::Instruction::Format i )
{
	switch ( i.Opcode )
	{
		// lower op
		case 0x40:
			switch ( i.Fd )
			{
				// fs source: MOVE,MR32,MTIR,RINIT,RXOR,SQD,SQI
				
				// MOVE,MR32
				case 0xc:
				
				// RINIT, RXOR
				case 0x10:
				
					return ( 1 << i.Fs );
					break;

					
				//MTIR
				case 0xf:
					if ( i.Funct == 0x3c )
					{
						return ( 1 << i.Fs );
					}
					
					break;

					
				// SQD, SQI
				case 0xd:
					if ( ( i.Funct == 0x3f ) || ( i.Funct == 0x3d ) )
					{
						return ( 1 << i.Fs );
					}
					
					break;
					
				// fs and ft source: DIV,E,RSQRT,SQRT
				case 0xe:
				case 0x1c:
				case 0x1d:
				case 0x1e:
				case 0x1f:
					return ( ( 1 << i.Fs ) | ( 1 << i.Ft ) );
					break;
			}
			break;
			
		//sq - fs as source
		case 0x01:
			return ( 1 << i.Fs );
			break;
		
	}
	
	return 0;
}


static u64 Recompiler::GetFDestRegsLO ( Vu::Instruction::Format i )
{
	switch ( i.Opcode )
	{
		// lower op
		case 0x40:
			switch ( i.Fd )
			{
				// ft dest: MOVE,MR32
				case 0xc:
				
				// MTIR
				//case 0xf:
				
				// MFP
				case 0x19:
				
				// MFIR
				//case 0xf:
				
				// RINIT, RXOR
				//case 0x10:
				
					return ( 1 << i.Ft );
					break;


				//MFIR
				case 0xf:
					if ( i.Funct == 0x3d )
					{
						return ( 1 << i.Ft );
					}
					
					break;

					
				// LQD, LQI
				case 0xd:
					if ( ( i.Funct == 0x3c ) || ( i.Funct == 0x3e ) )
					{
						return ( 1 << i.Ft );
					}
					
					break;
					
				// fs and ft source: DIV,E,RSQRT,SQRT
				case 0xe:
				case 0x1c:
				case 0x1d:
				case 0x1e:
				case 0x1f:
					//return ( ( 1 << i.Fs ) | ( 1 << i.Ft ) );
					return 0;
					break;
			}
			
			return 0;
			break;
			
		//lq - ft as dest
		case 0x00:
			return ( 1 << i.Ft );
			break;
		
	}
	
	return 0;
}



static u64 Recompiler::GetISourceRegsLO ( Vu::Instruction::Format i )
{
	switch ( i.Opcode )
	{
	}
}

static u64 Recompiler::GetIDestRegsLO ( Vu::Instruction::Format i )
{
	switch ( i.Opcode )
	{
	}
}
*/



// returns the bitmap for the destination registers for instruction
// if the instruction is not supported, then it will return -1ULL
u64 Recompiler::Get_DelaySlot_DestRegs ( Vu::Instruction::Format i )
{
	
	/*
	// check for "special"
	if ( !i.Opcode )
	{
		// rd is dest reg //
		
		// not including syscall or break, but these shouldn't cause problems
		if ( ( i.Funct == 12 ) || ( i.Funct == 13 ) )
		{
			return 0;
		}
		
		
		return ( 1ULL << i.Rd );
	}
	
	// check for regimm
	if ( ( i.Opcode == 1 ) )
	{
		// rd is dest reg //
		
		if ( i.Rt >= 16 )
		{
			return ( 1ULL << 31 );
		}
	}
	
	// check for jal
	if ( i.Opcode == 3 )
	{
		return ( 1ULL << 31 );
	}
	
	// immediates
	if ( ( i.Opcode >> 3 ) == 1 )
	{
		return ( 1ULL << i.Rt );
	}
	*/
	
	// loads
	/*
	if ( ( i.Opcode >> 3 ) == 4 )
	{
		// rt is dest reg //
		
		return ( 1ULL << i.Rt );
	}
	*/
	
	
	
	// any other instructions are not cleared to go
	return 0;
}










long Recompiler::Generate_VABSp ( x64Encoder *e, VU* v, Vu::Instruction::Format i )
{
	long ret;
	
	ret = 1;
	
	if ( i.Ft && i.xyzw )
	{
		if ( !i.Fs )
		{
			e->movdqa_regmem ( RCX, & v->vf [ i.Fs ].sw0 );
			
			if ( i.xyzw != 0xf )
			{
				//e->movdqa_regmem ( RAX, & v->vf [ i.Ft ].sw0 );
				//e->pblendwregregimm ( RCX, RAX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
				e->pblendwregmemimm ( RCX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			}
			
			ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RCX );
		}
		else
		{
			e->movdqa_regmem ( RCX, & v->vf [ i.Fs ].sw0 );
			
			//if ( i.xyzw != 0xf )
			//{
			//	e->movdqa_regmem ( RAX, & v->vf [ i.Ft ].sw0 );
			//}
			
			//e->pslldregimm ( RCX, 1 );
			e->padddregreg ( RCX, RCX );
			e->psrldregimm ( RCX, 1 );
			
			if ( i.xyzw != 0xf )
			{
				//e->pblendwregregimm ( RCX, RAX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
				e->pblendwregmemimm ( RCX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			}
			
			ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RCX );
		}
	}
	
	return ret;
}




long Recompiler::Generate_VMAXp ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u32 *pFt, u32 FtComponent )
{
	//lfs = ( lfs >= 0 ) ? lfs : ~( lfs & 0x7fffffff );
	//lft = ( lft >= 0 ) ? lft : ~( lft & 0x7fffffff );
	// compare as integer and return original value?
	//fResult = ( ( lfs > lft ) ? fs : ft );
	long ret;
	
	ret = 1;
	
	
	if ( i.Fd && i.xyzw )
	{
		e->movdqa_regmem ( RBX, & v->vf [ i.Fs ].sw0 );
		
		if ( !pFt )
		{
			e->movdqa_regmem ( RCX, & v->vf [ i.Ft ].sw0 );
		}
		else
		{
			e->movd_regmem( RCX, (long*) pFt );
		}
		
		//if ( i.xyzw != 0xf )
		//{
		//	e->movdqa_regmem ( 5, & v->vf [ i.Fd ].sw0 );
		//}
		
		/*
		e->movdqa_regreg ( RDX, RBX );
		e->movdqa_regreg ( 4, RBX );
		e->pslldregimm ( RDX, 1 );
		e->psrldregimm ( RDX, 1 );
		e->psradregimm ( 4, 31 );
		e->pxorregreg ( RDX, 4 );
		*/
		e->movdqa_regreg ( RDX, RBX );
		//e->movdqa_regreg ( 4, RBX );
		//e->pslldregimm ( RDX, 1 );
		e->psradregimm ( RDX, 31 );
		e->psrldregimm ( RDX, 1 );
		e->pxorregreg ( RDX, RBX );
		
		if ( pFt )
		{
			// need to "broadcast" the value in sse ??
			e->pshufdregregimm ( RCX, RCX, 0 );
		}
		
		/*
		e->movdqa_regreg ( RAX, RCX );
		e->movdqa_regreg ( 4, RCX );
		e->pslldregimm ( RAX, 1 );
		e->psrldregimm ( RAX, 1 );
		e->psradregimm ( 4, 31 );
		e->pxorregreg ( RAX, 4 );
		*/
		e->movdqa_regreg ( RAX, RCX );
		//e->movdqa_regreg ( 4, RCX );
		//e->pslldregimm ( RAX, 1 );
		e->psradregimm ( RAX, 31 );
		e->psrldregimm ( RAX, 1 );
		e->pxorregreg ( RAX, RCX );
		
		e->pcmpgtdregreg ( RAX, RDX );
		e->pblendvbregreg ( RBX, RCX );
		
		if ( i.xyzw != 0xf )
		{
			//e->pblendwregregimm ( RBX, 5, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			e->pblendwregmemimm ( RBX, & v->vf [ i.Fd ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
		}
		
		ret = e->movdqa_memreg ( & v->vf [ i.Fd ].sw0, RBX );
	}
	
	
	return ret;
}



long Recompiler::Generate_VMINp ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u32 *pFt, u32 FtComponent )
{
	//lfs = ( lfs >= 0 ) ? lfs : ~( lfs & 0x7fffffff );
	//lft = ( lft >= 0 ) ? lft : ~( lft & 0x7fffffff );
	// compare as integer and return original value?
	//fResult = ( ( lfs > lft ) ? fs : ft );
	long ret;
	
	ret = 1;
	
	if ( i.Fd && i.xyzw )
	{
		e->movdqa_regmem ( RBX, & v->vf [ i.Fs ].sw0 );
		
		if ( !pFt )
		{
			e->movdqa_regmem ( RCX, & v->vf [ i.Ft ].sw0 );
		}
		else
		{
			e->movd_regmem( RCX, (long*) pFt );
		}
		
		//if ( i.xyzw != 0xf )
		//{
		//	e->movdqa_regmem ( 5, & v->vf [ i.Fd ].sw0 );
		//}
		
		/*
		e->movdqa_regreg ( RAX, RBX );
		e->movdqa_regreg ( 4, RBX );
		e->pslldregimm ( RAX, 1 );
		e->psrldregimm ( RAX, 1 );
		e->psradregimm ( 4, 31 );
		e->pxorregreg ( RAX, 4 );
		*/
		e->movdqa_regreg ( RAX, RBX );
		//e->movdqa_regreg ( 4, RBX );
		//e->pslldregimm ( RAX, 1 );
		e->psradregimm ( RAX, 31 );
		e->psrldregimm ( RAX, 1 );
		e->pxorregreg ( RAX, RBX );
		
		
		
		/*
		e->movdqa_regreg ( RAX, RBX );
		e->psradregimm ( RAX, 31 );
		e->psrldregimm ( RAX, 1 );
		e->pxorregreg ( RBX, RAX );
		*/

		if ( pFt )
		{
			// need to "broadcast" the value in sse ??
			e->pshufdregregimm ( RCX, RCX, 0 );
		}
		
		/*
		e->movdqa_regreg ( RDX, RCX );
		e->movdqa_regreg ( 4, RCX );
		e->pslldregimm ( RDX, 1 );
		e->psrldregimm ( RDX, 1 );
		e->psradregimm ( 4, 31 );
		e->pxorregreg ( RDX, 4 );
		*/
		e->movdqa_regreg ( RDX, RCX );
		//e->movdqa_regreg ( 4, RCX );
		//e->pslldregimm ( RDX, 1 );
		e->psradregimm ( RDX, 31 );
		e->psrldregimm ( RDX, 1 );
		e->pxorregreg ( RDX, RCX );
		
		/*
		e->movdqa_regreg ( RAX, RCX );
		e->psradregimm ( RAX, 31 );
		e->psrldregimm ( RAX, 1 );
		e->pxorregreg ( RCX, RAX );
		
		e->pminsdregreg ( RBX, RCX );
		
		e->movdqa_regreg ( RAX, RBX );
		e->psradregimm ( RAX, 31 );
		e->psrldregimm ( RAX, 1 );
		e->pxorregreg ( RBX, RAX );
		*/
		
		e->pcmpgtdregreg ( RAX, RDX );
		e->pblendvbregreg ( RBX, RCX );
		
		if ( i.xyzw != 0xf )
		{
			//e->pblendwregregimm ( RBX, 5, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			e->pblendwregmemimm ( RBX, & v->vf [ i.Fd ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
		}
		
		ret = e->movdqa_memreg ( & v->vf [ i.Fd ].sw0, RBX );
	}
	
	
	return ret;
}



long Recompiler::Generate_VFTOIXp_test ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u32 IX )
{
	long ret;
	
	ret = 1;

	if ( i.Ft && i.xyzw )
	{
		//e->movdqa_regmem ( RBX, & v->vf [ i.Fs ].sw0 );
		e->movdqa_regmem ( RBX, & v->test1_src1.sw0 );
		

		if ( IX )
		{
			e->MovRegImm32 ( RAX, IX << 23 );
			e->movd_to_sse ( RCX, RAX );
			e->pshufdregregimm ( RCX, RCX, 0 );
			e->padddregreg ( RCX, RBX );
		}
		else
		{
			//e->MovRegReg32 ( RCX, RAX );
			e->movdqa_regreg ( RCX, RBX );
		}
		
		// move the registers now to floating point unit
		//e->movd_to_sse ( RAX, RCX );
		
		// convert single precision to signed 
		//e->cvttss2si ( RCX, RAX );
		e->cvttps2dq_regreg ( RCX, RCX );
		
		//e->Cdq ();
		//e->AndReg32ImmX ( RAX, 0x7f800000 );
		//e->CmovERegReg32 ( RDX, RAX );
		
		
		// compare exponent of magnitude and maximize if needed
		//e->CmpReg32ImmX ( RAX, 0x4e800000 - ( IX << 23 ) );
		//e->MovReg32ImmX ( RAX, 0x7fffffff );
		//e->CmovLERegReg32 ( RAX, RCX );
		//e->ShlRegImm32 ( RDX, 31 );
		//e->OrRegReg32 ( RAX, RDX );
		e->MovRegImm32 ( RAX, 0x4e800000 - ( IX << 23 ) - 1 );
		e->movd_to_sse ( RDX, RAX );
		e->pshufdregregimm ( RDX, RDX, 0 );
		e->pcmpeqbregreg ( RAX, RAX );
		e->psrldregimm ( RAX, 1 );
		
		e->movdqa_regreg ( 5, RAX );
		
		e->pandregreg ( RAX, RBX );
		
		e->psrldregimm ( RBX, 31 );
		e->padddregreg ( RBX, 5 );

		//if ( i.xyzw != 0xf )
		//{
		//	e->movdqa_regmem ( 5, & v->vf [ i.Ft ].sw0 );
		//}
		
		e->pcmpgtdregreg ( RAX, RDX );
		
		e->pblendvbregreg ( RCX, RBX );
		
		if ( i.xyzw != 0xf )
		{
			//e->pblendwregregimm ( RCX, 5, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			e->pblendwregmemimm ( RCX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
		}
		
		// set result
		//ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RCX );
		ret = e->movdqa_memreg ( & v->test2_result1.sw0, RCX );
	}

	return ret;
}



long Recompiler::Generate_VFTOIXp ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u32 IX )
{
	long ret;
	
	ret = 1;

	if ( i.Ft && i.xyzw )
	{
		//e->MovRegMem32 ( RAX, ( &v->vf [ i.Fs ].sw0 ) + FsComponent );
		e->movdqa_regmem ( RBX, & v->vf [ i.Fs ].sw0 );
		

		if ( IX )
		{
			e->MovRegImm32 ( RAX, IX << 23 );
			e->movd_to_sse ( RCX, RAX );
			e->pshufdregregimm ( RCX, RCX, 0 );
			e->padddregreg ( RCX, RBX );
		}
		else
		{
			//e->MovRegReg32 ( RCX, RAX );
			e->movdqa_regreg ( RCX, RBX );
		}
		
		// move the registers now to floating point unit
		//e->movd_to_sse ( RAX, RCX );
		
		// convert single precision to signed 
		//e->cvttss2si ( RCX, RAX );
		e->cvttps2dq_regreg ( RCX, RCX );
		
		//e->Cdq ();
		//e->AndReg32ImmX ( RAX, 0x7f800000 );
		//e->CmovERegReg32 ( RDX, RAX );
		
		
		// compare exponent of magnitude and maximize if needed
		//e->CmpReg32ImmX ( RAX, 0x4e800000 - ( IX << 23 ) );
		//e->MovReg32ImmX ( RAX, 0x7fffffff );
		//e->CmovLERegReg32 ( RAX, RCX );
		//e->ShlRegImm32 ( RDX, 31 );
		//e->OrRegReg32 ( RAX, RDX );
		e->MovRegImm32 ( RAX, 0x4f000000 - ( IX << 23 ) - 1 );
		e->movd_to_sse ( RDX, RAX );
		e->pshufdregregimm ( RDX, RDX, 0 );
		e->pcmpeqbregreg ( RAX, RAX );
		e->psrldregimm ( RAX, 1 );
		
		e->movdqa_regreg ( 5, RAX );
		
		e->pandregreg ( RAX, RBX );
		
		e->psrldregimm ( RBX, 31 );
		e->padddregreg ( RBX, 5 );

		//if ( i.xyzw != 0xf )
		//{
		//	e->movdqa_regmem ( 5, & v->vf [ i.Ft ].sw0 );
		//}
		
		e->pcmpgtdregreg ( RAX, RDX );
		
		e->pblendvbregreg ( RCX, RBX );
		
		if ( i.xyzw != 0xf )
		{
			//e->pblendwregregimm ( RCX, 5, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			e->pblendwregmemimm ( RCX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
		}
		
		// set result
		//ret = e->MovMemReg32 ( ( & v->vf [ i.Ft ].sw0 ) + FtComponent, RAX );
		ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RCX );
	}

	return ret;
}


long Recompiler::Generate_VITOFXp ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u64 FX )
{
	long ret;
	
	ret = 1;

	if ( i.Ft && i.xyzw )
	{
		//e->MovRegMem32 ( RAX, ( &v->vf [ i.Fs ].sw0 ) + FsComponent );
		e->movdqa_regmem ( RBX, & v->vf [ i.Fs ].sw0 );
		
		// convert single precision to signed 
		//e->cvtsi2sd ( RAX, RAX );
		//e->movq_from_sse ( RAX, RAX );
		e->cvtdq2pd ( RCX, RBX );
				
				
		//e->MovReg64ImmX ( RCX, ( 896 << 23 ) + ( FX << 23 ) );
		//e->Cqo();
		e->MovReg64ImmX ( RAX, ( 896ull << 23 ) + ( FX << 23 ) );
		e->movq_to_sse ( RDX, RAX );
		e->movddup_regreg ( RDX, RDX );
		
		//e->ShrRegImm64 ( RAX, 29 );
		//e->CmovERegReg64 ( RCX, RDX );
		//e->SubRegReg64 ( RAX, RCX );
		e->movdqa_regreg ( 4, RCX );
		e->psrlqregimm ( 4, 63 );
		e->pslldregimm ( 4, 31 );
		e->psrlqregimm ( RCX, 29 );
		e->psubqregreg ( RCX, RDX );
		e->porregreg ( RCX, 4 );
		
		e->pshufdregregimm ( 5, RBX, ( 3 << 2 ) | ( 2 << 0 ) );
		e->cvtdq2pd ( 5, 5 );

		e->movdqa_regreg ( RAX, 5 );
		e->psrlqregimm ( RAX, 63 );
		e->pslldregimm ( RAX, 31 );
		e->psrlqregimm ( 5, 29 );
		e->psubqregreg ( 5, RDX );
		e->porregreg ( 5, RAX );
		
		// combine RCX (bottom) and 5 (top)
		e->pshufdregregimm ( RCX, RCX, ( 2 << 2 ) | ( 0 << 0 ) );
		e->pshufdregregimm ( RDX, 5, ( 2 << 6 ) | ( 0 << 4 ) );
		e->pblendwregregimm ( RCX, RDX, 0xf0 );
		
		// load destination register
		//if ( i.xyzw != 0xf )
		//{
		//	e->movdqa_regmem ( 5, & v->vf [ i.Ft ].sw0 );
		//}
		
		// clear zeros
		e->pxorregreg ( RAX, RAX );
		e->pcmpeqdregreg ( RAX, RBX );
		e->pandnregreg ( RAX, RCX );
		
		// select result
		if ( i.xyzw != 0xf )
		{
			//e->pblendwregregimm ( RAX, 5, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			e->pblendwregmemimm ( RAX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
		}
		
		// store result
		ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RAX );
		
		//e->ShlRegImm32 ( RDX, 31 );
		//e->OrRegReg32 ( RAX, RDX );
				
		// set result
		//ret = e->MovMemReg32 ( ( &v->vf [ i.Ft ].sw0 ) + FtComponent, RAX );
	}

	return ret;
}



long Recompiler::Generate_VMOVEp ( x64Encoder *e, VU* v, Vu::Instruction::Format i )
{
	long ret;
	
	ret = 1;

	if ( i.Ft && i.xyzw )
	{
		e->movdqa_regmem ( RCX, & v->vf [ i.Fs ].sw0 );
		
		if ( i.xyzw != 0xf )
		{
			//e->movdqa_regmem ( RAX, & v->vf [ i.Ft ].sw0 );
			//e->pblendwregregimm ( RCX, RAX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			e->pblendwregmemimm ( RCX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
		}
		
		ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RCX );
	}

	return ret;
}


long Recompiler::Generate_VMR32p ( x64Encoder *e, VU* v, Vu::Instruction::Format i )
{
	long ret;
	
	ret = 1;

	if ( i.Ft && i.xyzw )
	{
		e->movdqa_regmem ( RCX, & v->vf [ i.Fs ].sw0 );
		
		//if ( i.xyzw != 0xf )
		//{
		//	e->movdqa_regmem ( RAX, & v->vf [ i.Ft ].sw0 );
		//}
		
		e->pshufdregregimm ( RCX, RCX, ( 0 << 6 ) | ( 3 << 4 ) | ( 2 << 2 ) | ( 1 << 0 ) );
		
		if ( i.xyzw != 0xf )
		{
			//e->pblendwregregimm ( RCX, RAX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			e->pblendwregmemimm ( RCX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
		}
		
		ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RCX );
	}

	return ret;
}



long Recompiler::Generate_VMFIRp ( x64Encoder *e, VU* v, Vu::Instruction::Format i )
{
	long ret;
	
	ret = 1;

	if ( i.Ft && i.xyzw )
	{
		// flush ps2 float to zero
		if ( !( i.is & 0xf ) )
		{
			//e->MovMemImm32 ( ( & v->vf [ i.Ft ].sw0 ) + FtComponent, 0 );
			
			//if ( i.xyzw != 0xf )
			//{
			//	e->movdqa_regmem ( RAX, & v->vf [ i.Ft ].sw0 );
			//}
			
			e->pxorregreg ( RCX, RCX );
			
			if ( i.xyzw != 0xf )
			{
				//e->pblendwregregimm ( RCX, RAX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
				e->pblendwregmemimm ( RCX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			}
			
			ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RCX );
		}
		else
		{
			// flush ps2 float to zero
			e->MovRegMem32 ( RAX, ( &v->vi [ i.is & 0xf ].s ) );
			
			//if ( i.xyzw != 0xf )
			//{
			//	e->movdqa_regmem ( RAX, & v->vf [ i.Ft ].sw0 );
			//}
			
			// sign-extend from 16-bit to 32-bit
			e->Cwde();
			
			e->movd_to_sse ( RCX, RAX );
			e->pshufdregregimm ( RCX, RCX, 0 );
			
			if ( i.xyzw != 0xf )
			{
				//e->pblendwregregimm ( RCX, RAX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
				e->pblendwregmemimm ( RCX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			}
			
			// set result
			//ret = e->MovMemReg32 ( ( &v->vf [ i.Ft ].sw0 ) + FtComponent, RAX );
			ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RCX );
		}
		
	}

	return ret;
}




long Recompiler::Generate_VMTIRp ( x64Encoder *e, VU* v, Vu::Instruction::Format i )
{
	long ret;
	
	ret = 1;


	if ( ( i.it & 0xf ) )
	{
		//v->Set_IntDelaySlot ( i.it & 0xf, (u16) v->vf [ i.Fs ].vsw [ i.fsf ] );

		// flush ps2 float to zero
		if ( ( !i.Fs ) && ( i.fsf < 3 ) )
		{
			//e->MovMemImm32 ( ( & v->vf [ i.Ft ].sw0 ) + FtComponent, 0 );
			ret = e->MovMemImm32 ( & v->vi [ i.it & 0xf ].s, 0 );
		}
		else
		{
			// flush ps2 float to zero
			e->MovRegMem32 ( RAX, & v->vf [ i.Fs ].vsw [ i.fsf ] );
			e->AndReg32ImmX ( RAX, 0xffff );
			
			// set result
			ret = e->MovMemReg32 ( & v->vi [ i.it & 0xf ].s, RAX );
		}
		
	}


	return ret;
}


// set bSub to 1 for subtraction
long Recompiler::Generate_VADDp ( x64Encoder *e, VU* v, u32 bSub, Vu::Instruction::Format i, u32 FtComponent, void *pFd, u32 *pFt )
{
	static const u64 c_lUpperBound = ( 24LL << 32 ) | ( 24LL );
	static const u64 c_lLowerBound = ( 0xffffffe8LL << 32 ) | ( 0xffffffe8LL );
	static const u64 c_lUFTest = ( 0x800000LL << 32 ) | ( 0x800000LL );
	long ret;
	
	ret = 1;


	if ( i.xyzw )
	{
		// load source regs
		// but they need to be shuffled into reverse on load
		//e->movdqa_regmem ( RAX, & v->vf [ i.Fs ].sw0 );
		
		if ( pFt )
		{
			e->movd_regmem ( RCX, (long*) pFt );
			e->pshufdregregimm ( RCX, RCX, ( FtComponent << 6 ) | ( FtComponent << 4 ) | ( FtComponent << 2 ) | ( FtComponent << 0 ) );
		}
		else
		{
			//e->movdqa_regmem ( RCX, & v->vf [ i.Ft ].sw0 );
			
			if ( FtComponent < 4 )
			{
				//e->pshufdregregimm ( RCX, RCX, ( FtComponent << 6 ) | ( FtComponent << 4 ) | ( FtComponent << 2 ) | ( FtComponent << 0 ) );
				e->pshufdregmemimm ( RCX, & v->vf [ i.Ft ].sw0, ( FtComponent << 6 ) | ( FtComponent << 4 ) | ( FtComponent << 2 ) | ( FtComponent << 0 ) );
			}
			else
			{
				//e->pshufdregregimm ( RCX, RCX, ( 0 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | ( 3 << 0 ) );
				e->pshufdregmemimm ( RCX, & v->vf [ i.Ft ].sw0, ( 0 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | ( 3 << 0 ) );
			}
		}
		
		
		//e->pshufdregregimm ( RAX, RAX, ( 0 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | ( 3 << 0 ) );
		e->pshufdregmemimm ( RAX, & v->vf [ i.Fs ].sw0, ( 0 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | ( 3 << 0 ) );
		
		//if ( FtComponent < 4 )
		//{
		//	e->pshufdregregimm ( RCX, RCX, ( FtComponent << 6 ) | ( FtComponent << 4 ) | ( FtComponent << 2 ) | ( FtComponent << 0 ) );
		//}
		//else
		//{
		//	e->pshufdregregimm ( RCX, RCX, ( 0 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | ( 3 << 0 ) );
		//}
		
		// get exponents
		e->movdqa_regreg ( RDX, RAX );
		e->pslldregimm ( RDX, 1 );
		e->psrldregimm ( RDX, 24 );
		e->movdqa_regreg ( 4, RCX );
		e->pslldregimm ( 4, 1 );
		e->psrldregimm ( 4, 24 );

		
		// debug
		//e->movdqa_memreg ( & v0, RDX );

		// debug
		//e->movdqa_memreg ( & v1, 4 );

		// clear zero exponents ??
		e->pxorregreg ( RBX, RBX );
		e->pcmpeqdregreg ( RBX, RDX );
		e->psrldregimm ( RBX, 1 );
		e->pandnregreg ( RBX, RAX );
		e->movdqa_regreg ( RAX, RBX );
		e->pxorregreg ( RBX, RBX );
		e->pcmpeqdregreg ( RBX, 4 );
		e->psrldregimm ( RBX, 1 );
		e->pandnregreg ( RBX, RCX );
		e->movdqa_regreg ( RCX, RBX );
		
		
		// get difference
		e->psubdregreg ( RDX, 4 );
		
		
		// if positive 24 or greater then zero Ft
		e->movddup_regmem ( 5, (long long*) &c_lUpperBound );
		e->pcmpgtdregreg ( 5, RDX );
		e->pandregreg ( RCX, 5 );


		// if negative 24 or less, then zero Fs
		e->movddup_regmem ( 5, (long long*) &c_lLowerBound );
		e->pcmpgtdregreg ( RDX, 5 );
		e->pandregreg ( RAX, RDX );

		if ( ( __builtin_popcount ( i.xyzw ) < 3 ) )
		{
			e->pshufdregregimm ( RDX, RAX, _op_shuffle_lut_1 [ i.xyzw ] );

				//e->movdqa_regreg ( RDX, RAX );
				//e->movdqa_regreg ( 4, RAX );
				e->movdqa_regreg ( 4, RDX );
				e->psllqregimm ( RDX, 33 );
				e->psrlqregimm ( RDX, 1 + 11 - 8 );
				e->psrldregimm ( 4, 31 );
				e->psllqregimm ( 4, 63 );
				e->porregreg ( RDX, 4 );
				
			e->pshufdregregimm ( 5, RCX, _op_shuffle_lut_1 [ i.xyzw ] );
				
				//e->movdqa_regreg ( 5, RCX );
				//e->movdqa_regreg ( 4, RCX );
				e->movdqa_regreg ( 4, 5 );
				e->psllqregimm ( 5, 33 );
				e->psrlqregimm ( 5, 1 + 11 - 8 );
				e->psrldregimm ( 4, 31 );
				e->psllqregimm ( 4, 63 );
				e->porregreg ( 5, 4 );
				
				
				if ( bSub )
				{
					// subtract (round1)
					e->subpdregreg ( RDX, 5 );
				}
				else
				{
					// add (round1)
					e->addpdregreg ( RDX, 5 );
				}
				
				
				// merge result (round1) without sign into RAX, and sign into RCX
				e->movdqa_regreg ( 4, RDX );
				e->psrlqregimm ( 4, 63 );
				e->pslldregimm ( 4, 31 );
				
				e->psrlqregimm ( RDX, 29 );
				
				//e->pblendwregregimm ( RAX, RDX, 0x33 );
				//e->movdqa_regreg ( RBX, 4 );
			
			e->pshufdregregimm ( RAX, RDX, _op_add_shuffle_lut_2 [ i.xyzw ] );
			e->pshufdregregimm ( RBX, 4, _op_add_shuffle_lut_2 [ i.xyzw ] );
		}
		else
		{
			if ( i.xyzw & 0x5 )
			{
				// convert to double (round1)
				e->movdqa_regreg ( RDX, RAX );
				e->movdqa_regreg ( 4, RAX );
				e->psllqregimm ( RDX, 33 );
				e->psrlqregimm ( RDX, 1 + 11 - 8 );
				e->psrldregimm ( 4, 31 );
				e->psllqregimm ( 4, 63 );
				e->porregreg ( RDX, 4 );
				
				
				e->movdqa_regreg ( 5, RCX );
				e->movdqa_regreg ( 4, RCX );
				e->psllqregimm ( 5, 33 );
				e->psrlqregimm ( 5, 1 + 11 - 8 );
				e->psrldregimm ( 4, 31 );
				e->psllqregimm ( 4, 63 );
				e->porregreg ( 5, 4 );
				
				
				if ( bSub )
				{
					// subtract (round1)
					e->subpdregreg ( RDX, 5 );
				}
				else
				{
					// add (round1)
					e->addpdregreg ( RDX, 5 );
				}
				
				
				// merge result (round1) without sign into RAX, and sign into RCX
				e->movdqa_regreg ( 4, RDX );
				e->psrlqregimm ( 4, 63 );
				e->pslldregimm ( 4, 31 );
				//e->psllqregimm ( RDX, 4 );
				e->psrlqregimm ( RDX, 29 );
				e->pblendwregregimm ( RAX, RDX, 0x33 );
				e->movdqa_regreg ( RBX, 4 );
			}
			
			if ( i.xyzw & 0xa )
			{
				// convert to double (round2)
				e->movdqa_regreg ( RDX, RAX );
				e->movdqa_regreg ( 4, RAX );
				e->psrlqregimm ( RDX, 32 );
				e->psllqregimm ( RDX, 33 );
				e->psrlqregimm ( RDX, 1 + 11 - 8 );
				e->psrlqregimm ( 4, 63 );
				e->psllqregimm ( 4, 63 );
				e->porregreg ( RDX, 4 );

				
				e->movdqa_regreg ( 5, RCX );
				e->movdqa_regreg ( 4, RCX );
				e->psrlqregimm ( 5, 32 );
				e->psllqregimm ( 5, 33 );
				e->psrlqregimm ( 5, 1 + 11 - 8 );
				e->psrlqregimm ( 4, 63 );
				e->psllqregimm ( 4, 63 );
				e->porregreg ( 5, 4 );


				// debug
				//e->movdqa_memreg ( & v4, RAX );
				//e->movdqa_memreg ( & v5, RCX );
				
				if ( bSub )
				{
					// subtract (round2)
					e->subpdregreg ( RDX, 5 );
				}
				else
				{
					// add (round2)
					e->addpdregreg ( RDX, 5 );
				}
				
				// merge result (round2) without sign into RAX, and sign into RCX
				e->movdqa_regreg ( 4, RDX );
				e->psrlqregimm ( 4, 63 );
				e->psllqregimm ( 4, 63 );
				e->psllqregimm ( RDX, 3 );
				//e->psrlqregimm ( RDX, 1 );
				e->pblendwregregimm ( RAX, RDX, 0xcc );
				e->pblendwregregimm ( RBX, 4, 0xcc );
			}
		}
		
		// pull overflow flags
		e->movmskpsregreg ( RAX, RAX );
		
		// if overflow, then maximize result
		e->movdqa_regreg ( RCX, RAX );
		e->psradregimm ( RCX, 31 );
		//e->psrldregimm ( RCX, 1 );
		e->porregreg ( RAX, RCX );
		e->pslldregimm ( RAX, 1 );
		e->psrldregimm ( RAX, 1 );

		// debug
		//e->movdqa_memreg ( & v1, RCX );

		// test for zero (also zero flag on underflow)
		e->pxorregreg ( RCX, RCX );
		e->pcmpeqdregreg ( RCX, RAX );

		// test for underflow
		e->movddup_regmem ( RDX, (long long*) &c_lUFTest );
		e->pcmpgtdregreg ( RDX, RAX );
		
		// if underflow, then clear result (but keep sign ??)
		e->movdqa_regreg ( 4, RDX );
		e->pandnregreg ( 4, RAX );
		
		
		// combine result with sign now
		e->pslldregimm ( 4, 1 );
		e->psrldregimm ( 4, 1 );
		e->porregreg ( 4, RBX );

		// not underflow if it is zero
		// puts underflow in RAX
		e->movdqa_regreg ( RAX, RCX );
		e->pandnregreg( RAX, RDX );
		
		
		
		// clear sign flag if signed zero ?? unless underflow ??
		// but test for underflow before testing for zero
		// so and not zero, then and underflow
		e->movdqa_regreg ( RDX, RCX );
		e->pandnregreg ( RDX, RBX );
		//e->pandregreg ( RDX, RAX );
		
		// pull sign flags
		// RDX = sign flag
		e->movmskpsregreg ( RCX, RDX );
		
		// zero flag is also set when underflow
		// RAX = underflow flag, RCX = zero flag
		e->porregreg ( RCX, RAX );
		
		// pull zero flags
		e->movmskpsregreg ( RDX, RCX );
		
		// pull underflow flags
		// RAX = underflow flag
		e->movmskpsregreg ( 8, RAX );
		
		// set result
		//if ( i.xyzw != 0xf )
		//{
		//	if ( pFd )
		//	{
		//		e->movdqa_regmem ( 5, pFd );
		//	}
		//	else
		//	{
		//		e->movdqa_regmem ( 5, & v->vf [ i.Fd ].sw0 );
		//	}
		//}
		
		e->pshufdregregimm ( 4, 4, ( 0 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | ( 3 << 0 ) );
		
		if ( i.xyzw != 0xf )
		{
			//e->pblendwregregimm ( 4, 5, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			if ( pFd )
			{
				//e->movdqa_regmem ( 5, pFd );
				e->pblendwregmemimm ( 4, pFd, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			}
			else
			{
				//e->movdqa_regmem ( 5, & v->vf [ i.Fd ].sw0 );
				e->pblendwregmemimm ( 4, & v->vf [ i.Fd ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			}
		}
		
		if ( pFd )
		{
			e->movdqa_memreg ( pFd, 4 );
		}
		else
		{
			if ( i.Fd )
			{
				e->movdqa_memreg ( & v->vf [ i.Fd ].sw0, 4 );
			}
		}
		

		// clear flags for vu units that do not operate
		if ( i.xyzw != 0xf )
		{
			e->AndReg32ImmX ( RAX, ( i.Value >> 21 ) & 0xf );
			e->AndReg32ImmX ( RCX, ( i.Value >> 21 ) & 0xf );
			e->AndReg32ImmX ( RDX, ( i.Value >> 21 ) & 0xf );
			e->AndReg32ImmX ( 8, ( i.Value >> 21 ) & 0xf );
		}

#ifdef ENABLE_VFLAGS
		e->MovMemReg8 ( & v->CurrentVFlags.vZeroFlag, RDX );
		e->MovMemReg8 ( & v->CurrentVFlags.vUnderflowFlag, 8 );
		e->MovMemReg8 ( & v->CurrentVFlags.vSignFlag, RCX );
		e->MovMemReg8 ( & v->CurrentVFlags.vOverflowFlag, RAX );
		if ( !v->SetStatus_Flag )
		{
		ret = e->MovRegMem32 ( RAX, & v->CurrentVFlags.Flags );
		e->MovMemReg32 ( & v->CurrentVFlags.StatusFlags, RAX );
		ret = e->OrMemReg32 ( & v->CurrentVFlags.StickyFlags, RAX );
		}
#else
		// combine MAC flags
		e->ShlRegImm32 ( RCX, 4 );
		e->ShlRegImm32 ( 8, 8 );
		e->ShlRegImm32 ( RAX, 12 );
		
		e->OrRegReg32 ( RDX, RAX );
		e->OrRegReg32 ( RDX, RCX );
		e->OrRegReg32 ( RDX, 8 );
		
		// set MAC flags (RAX)
		ret = e->MovMemReg32 ( &v->vi [ VU::REG_MACFLAG ].s, RDX );
		
		// set status flags
		// RAX=overflow, RCX=sign, RDX=zero, 8=underflow
		
		// check if the lower instruction set stat flag already (there's only like one instruction that does this)
		if ( !v->SetStatus_Flag )
		{
			// restore RDX
			e->AndReg32ImmX ( RDX, 0xf );
			
			// set overflow status flag
			e->NegReg32 ( RAX );
			e->AdcRegReg32 ( RAX, RAX );
			
			// set underflow status flag
			e->NegReg32 ( 8 );
			e->AdcRegReg32 ( RAX, RAX );
			
			// set sign status flag
			e->NegReg32 ( RCX );
			e->AdcRegReg32 ( RAX, RAX );
			
			// set zero status flag
			e->NegReg32 ( RDX );
			e->AdcRegReg32 ( RAX, RAX );
			
			e->MovRegMem32 ( RDX, &v->vi [ VU::REG_STATUSFLAG ].s );
			e->AndReg32ImmX ( RAX, 0xf );
			e->MovRegReg32 ( RCX, RAX );
			e->ShlRegImm32 ( RAX, 6 );
			e->OrRegReg32 ( RAX, RCX );
			e->AndReg32ImmX ( RDX, ~0xf );
			e->OrRegReg32 ( RAX, RDX );
			ret = e->MovMemReg32 ( &v->vi [ VU::REG_STATUSFLAG ].s, RAX );
			
		}
#endif
		
	}
	
	
	return ret;
}



long Recompiler::Generate_VMULp ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u32 FtComponentp, void *pFd, u32 *pFt, u32 FsComponentp )
{
	static const unsigned long long c_llMinExpDbl = ( 896LL << 52 );
	static const unsigned long long c_llExpMask = ( 0xffLL << 23 ) | ( 0xffLL << ( 23 + 32 ) );
	static const u64 c_lUFTest = ( 0x800000LL << 32 ) | ( 0x800000LL );
	
	long ret;
	
	ret = 1;


	if ( i.xyzw )
	{
		
		// load source regs
		// but they need to be shuffled into reverse on load
		//e->movdqa_regmem ( RAX, & v->vf [ i.Fs ].sw0 );
		
		
		if ( pFt )
		{
			e->movd_regmem ( RCX, (long*) pFt );
			e->pshufdregregimm ( RCX, RCX, FtComponentp );
		}
		else
		{
			//e->movdqa_regmem ( RCX, & v->vf [ i.Ft ].sw0 );
			e->pshufdregmemimm ( RCX, & v->vf [ i.Ft ].sw0, FtComponentp );
		}
		
		//e->pshufdregregimm ( RAX, RAX, FsComponentp );
		e->pshufdregmemimm ( RAX, & v->vf [ i.Fs ].sw0, FsComponentp );
		
		//e->pshufdregregimm ( RCX, RCX, FtComponentp );

#ifdef OPTIMIZE_RO_MULTIPLY_MUL
		// if fs or ft is r0 then the computation is easier/quicker
		if ( !pFt && ( ( ! i.Fs ) || ( ! i.Ft ) ) )
		{
			// if fs is not the r0 register then it has the value we want in RAX
			// and the r0 value is in RCX
			if ( i.Fs )
			{
				//e->movdqa_regreg ( RBX, RCX );
				e->movdqa_regreg ( RDX, RCX );
				e->movdqa_regreg ( 4, RAX );
			}
			else
			{
				// otherwise RAX/Fs has the r0 value
				//e->movdqa_regreg ( RBX, RAX );
				e->movdqa_regreg ( RDX, RAX );
				e->movdqa_regreg ( 4, RCX );
			}
			
			
			// check which values are zero/non-zero in r0
			e->pxorregreg ( RBX, RBX );
			e->pcmpeqdregreg ( RDX, RBX );
			
			// check if the operand has any zeros and flush
			//e->movdqa_regreg ( 4, RCX );
			e->pslldregimm ( 4, 1 );
			e->psrldregimm ( 4, 24 );
			e->pcmpeqdregreg ( RBX, 4 );
			
			// combine zero flags
			e->porregreg ( RDX, RBX );
			
			
			// get zero flags
			e->movmskpsregreg ( RDX, RDX );
			
			// get multiply result
			e->psrldregimm ( RDX, 1 );
			if ( i.Fs )
			{
				// multiply r0 with fs/rax
				e->pandnregreg ( RDX, RAX );
			}
			else
			{
				// multiply r0 with ft/rcx
				e->pandnregreg ( RDX, RCX );
			}
			
			// get sign flags
			e->movmskpsregreg ( RCX, RDX );
			
			// set result
			if ( i.xyzw != 0xf )
			{
				if ( pFd )
				{
					e->movdqa_regmem ( 5, pFd );
				}
				else
				{
					e->movdqa_regmem ( 5, & v->vf [ i.Fd ].sw0 );
				}
			}
			
			e->pshufdregregimm ( RDX, RDX, ( 0 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | ( 3 << 0 ) );
			
			if ( i.xyzw != 0xf )
			{
				e->pblendwregregimm ( RDX, 5, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			}
			
			if ( pFd )
			{
				e->movdqa_memreg ( pFd, RDX );
			}
			else
			{
				if ( i.Fd )
				{
					e->movdqa_memreg ( & v->vf [ i.Fd ].sw0, RDX );
				}
			}
			
			// clear flags for vu units that do not operate
			if ( i.xyzw != 0xf )
			{
				e->AndReg32ImmX ( RCX, ( i.Value >> 21 ) & 0xf );
				e->AndReg32ImmX ( RDX, ( i.Value >> 21 ) & 0xf );
			}
			
#ifdef ENABLE_SET_SIGN_FLAG_ON_ZERO
			// clear sign flags where result is zero
			e->NotReg32 ( RDX );
			e->AndRegReg32 ( RCX, RDX );
			e->NotReg32 ( RDX );
#endif

#ifdef ENABLE_VFLAGS
			e->MovMemImm64 ( & v->CurrentVFlags.Value0, 0 );
			e->MovMemReg8 ( & v->CurrentVFlags.vZeroFlag, RDX );
			//e->MovMemReg8 ( & v->CurrentVFlags.vUnderflowFlag, 8 );
			e->MovMemReg8 ( & v->CurrentVFlags.vSignFlag, RCX );
			//e->MovMemReg8 ( & v->CurrentVFlags.vOverflowFlag, RAX );
			if ( !v->SetStatus_Flag )
			{
			e->MovRegMem32 ( RAX, & v->CurrentVFlags.Flags );
			e->MovMemReg32 ( & v->CurrentVFlags.StatusFlags, RAX );
			ret = e->OrMemReg32 ( & v->CurrentVFlags.StickyFlags, RAX );
			}
#else

			// combine MAC flags
			e->ShlRegImm32 ( RCX, 4 );
			
			//e->OrRegReg32 ( RDX, RCX );
			e->LeaRegRegReg32 ( RAX, RCX, RDX );
			
			// set MAC flags (RAX)
			//ret = e->MovMemReg32 ( &v->vi [ VU::REG_MACFLAG ].s, RDX );
			ret = e->MovMemReg32 ( &v->vi [ VU::REG_MACFLAG ].s, RAX );
			
			
			// set status flags
			// RAX=overflow, RCX=sign, RDX=zero, 8=underflow
			
			// check if the lower instruction set stat flag already (there's only like one instruction that does this)
			if ( !v->SetStatus_Flag )
			{
				// restore RDX
				//e->AndReg32ImmX ( RDX, 0xf );
				e->XorRegReg32 ( RAX, RAX );
				
				// set sign status flag
				e->NegReg32 ( RCX );
				e->AdcRegReg32 ( RAX, RAX );
				
				// set zero status flag
				e->NegReg32 ( RDX );
				e->AdcRegReg32 ( RAX, RAX );
				
				e->MovRegMem32 ( RDX, &v->vi [ VU::REG_STATUSFLAG ].s );
				//e->AndReg32ImmX ( RAX, 0x3 );
				e->MovRegReg32 ( RCX, RAX );
				e->ShlRegImm32 ( RAX, 6 );
				e->OrRegReg32 ( RAX, RCX );
				e->AndReg32ImmX ( RDX, ~0xf );
				e->OrRegReg32 ( RAX, RDX );
				ret = e->MovMemReg32 ( &v->vi [ VU::REG_STATUSFLAG ].s, RAX );
				
			}
#endif
			
			return ret;
		}
#endif


#ifdef USE_NEW_VMUL_CODE

		// get the initial exponent
		//es = ( fs >> 23 ) & 0xff;
		e->movdqa_regreg( RDX, RAX );
		e->pslldregimm ( RDX, 1 );
		e->psrldregimm ( RDX, 24 );
		
		// check for zero
		e->pxorregreg ( RBX, RBX );
		e->pcmpeqdregreg ( RBX, RDX );
		
		//et = ( ft >> 23 ) & 0xff;
		e->movdqa_regreg( 4, RCX );
		e->pslldregimm ( 4, 1 );
		e->psrldregimm ( 4, 24 );
		
		// check for zero
		// puts what should be zero in r5
		e->pxorregreg ( 5, 5 );
		e->pcmpeqdregreg ( 5, 4 );
		e->porregreg ( 5, RBX );
		
		//ed = es + et - 127;
		e->pcmpeqbregreg ( RBX, RBX );
		e->padddregreg ( 4, RDX );
		e->psrldregimm ( RBX, 25 );
		e->psubdregreg ( 4, RBX );
		
		
		// add hidden bit
		//ms |= 0x00800000;
		//mt |= 0x00800000;
		e->pslldregimm ( RBX, 31 );


		// get sign into RDX
		// sign is always a xor
		//fd = ( fs ^ ft ) & -0x80000000;
		e->movdqa_regreg ( RDX, RAX );
		e->pxorregreg ( RDX, RCX );
		e->pandregreg ( RDX, RBX );
		
		
		
		// get the mantissa and add the hidden bit
		//ms = fs & 0x007fffff;
		//mt = ft & 0x007fffff;
		e->pslldregimm ( RAX, 8 );
		e->pslldregimm ( RCX, 8 );
		e->porregreg ( RAX, RBX );
		e->porregreg ( RCX, RBX );
		
		
		//e->movdqa_memreg ( & v0, RAX );
		//e->movdqa_memreg ( & v1, RCX );
		
		if ( __builtin_popcount ( i.xyzw ) < 3 )
		{
			e->pshufdregregimm ( RBX, RAX, _op_shuffle_lut_1 [ i.xyzw ] );
			e->pshufdregregimm ( RCX, RCX, _op_shuffle_lut_1 [ i.xyzw ] );
			e->pmuludqregreg ( RBX, RCX );
			e->pshufdregregimm ( RBX, RBX, _op_shuffle_lut_2 [ i.xyzw ] );
		}
		else
		{
			// do the multiply
			//md = ms * mt;
			if ( i.xyzw & 0x5 )
			{
				e->movdqa_regreg ( RBX, RAX );
				e->pmuludqregreg ( RBX, RCX );
				e->psrlqregimm ( RBX, 32 );
			}
			
			if ( i.xyzw & 0xa )
			{
				e->psrlqregimm ( RAX, 32 );
				e->psrlqregimm ( RCX, 32 );
				e->pmuludqregreg ( RAX, RCX );
				e->pblendwregregimm ( RBX, RAX, 0xcc );
			}
		}

		//e->movdqa_memreg ( & v2, RAX );
		//e->movdqa_memreg ( & v3, RBX );

		
		// first shift right by the 23
		// shift and combine
		
		// get bit 47
		//ext = md >> 47;
		e->movdqa_regreg ( RAX, RBX );
		e->psradregimm ( RAX, 31 );
		
		// get the result
		//md >>= ( 23 + ext );
		e->movdqa_regreg ( RCX, RBX );
		e->psrldregimm ( RCX, 1 );
		e->pblendvbregreg ( RBX, RCX );
		
		
		//e->movdqa_memreg ( & v4, RAX );
		//e->movdqa_memreg ( & v5, RBX );
		
		// remove the hidden bit
		//md &= 0x7fffff;
		e->pslldregimm ( RBX, 2 );
		e->psrldregimm ( RBX, 9 );
		
		// update exponent
		//ed += ext;
		e->psubdregreg ( 4, RAX );
		
		// check if result is zero
		e->pxorregreg ( RAX, RAX );
		e->pcmpeqdregreg ( RAX, 4 );
		e->porregreg ( 5, RAX );
		
		// only under flow if not zero!
		e->movdqa_regreg ( RAX, 5 );
		e->pandnregreg ( RAX, 4 );
		
		// get exponent underflow into RCX
		e->movmskpsregreg ( 8, RAX );
		
		// set to zero for under flow
		//e->movdqa_regreg ( RAX, 4 );
		e->psradregimm ( RAX, 31 );
		e->porregreg ( 5, RAX );
		
		// get exponent overflow into R8 (*note* remove the underflow from the bits, can only have one or the other)
		e->pslldregimm ( 4, 23 );
		
		// only if not under flow
		e->movdqa_regreg ( RAX, 5 );
		e->pandnregreg ( RAX, 4 );
		
		// pull overflow flags
		e->movmskpsregreg ( RAX, RAX );
		
		
		// check if ( es == 0 ) || ( et == 0 ) || ( ed <= 0 )
		// puts zero exponent mask in RDX
		// puts exponent underflow in RBX
		
		// add in the exponent
		e->porregreg ( RBX, 4 );
		
		// maximize on overflow
		e->psradregimm ( RAX, 31 );
		e->porregreg ( RBX, RAX );
		
		// clear sign
		e->pslldregimm ( RBX, 1 );
		e->psrldregimm ( RBX, 1 );
		
		// get the sign flags into RAX
		e->movmskpsregreg ( RCX, RDX );
		
		// get exponent zero flags into RDX
		e->movmskpsregreg ( RDX, 5 );
		
		// clear the zero results
		e->pandnregreg ( 5, RBX );
		
		// add in the sign (sign is in RDX)
		e->porregreg ( RDX, 5 );

#else
		// get signs into RBX
		e->movdqa_regreg ( RBX, RAX );
		e->pxorregreg ( RBX, RCX );
		
		// clear signs
		e->pslldregimm ( RAX, 1 );
		e->psrldregimm ( RAX, 1 );
		e->pslldregimm ( RCX, 1 );
		e->psrldregimm ( RCX, 1 );
		
		// clear zero exponents
		e->pcmpeqbregreg ( RDX, RDX );
		e->psrldregimm ( RDX, 9 );
		e->movdqa_regreg ( 5, RAX );
		e->pcmpgtdregreg ( 5, RDX );
		e->pandregreg ( RAX, 5 );
		e->pandregreg ( RCX, 5 );
		e->movdqa_regreg ( 4, RCX );
		e->pcmpgtdregreg ( 4, RDX );
		e->pandregreg ( RAX, 4 );
		e->pandregreg ( RCX, 4 );
		
		// get the non-zero results with logical and
		e->pandregreg ( 4, 5 );
		
		// load constant into R5
		e->movddup_regmem ( 5, & c_llMinExpDbl );
		
		// stuff into RBX with the signs for now
		//e->psrldregimm ( 4, 1 );
		//e->porregreg ( RBX, 4 );
		
		// get the non-zero flags
		e->movmskpsregreg ( 8, 4 );

		
		

		
		if ( i.xyzw & 0x5 )
		{
			// convert to double (round1)
			e->movdqa_regreg ( RDX, RAX );
			//e->movdqa_regreg ( 4, RAX );
			e->psllqregimm ( RDX, 33 );
			e->psrlqregimm ( RDX, 1 + 11 - 8 );
			//e->psrldregimm ( 4, 31 );
			//e->psllqregimm ( 4, 63 );
			//e->porregreg ( RDX, 4 );
			
			
			e->movdqa_regreg ( 4, RCX );
			//e->movdqa_regreg ( 5, RCX );
			e->psllqregimm ( 4, 33 );
			e->psrlqregimm ( 4, 1 + 11 - 8 );
			//e->psrldregimm ( 5, 31 );
			//e->psllqregimm ( 5, 63 );
			//e->porregreg ( 4, 5 );




			// add to one arg
			e->paddqregreg ( RDX, 5 );


			
			// multiply (round1)
			e->mulpdregreg ( RDX, 4 );

			

			
			// merge result (round1) without sign into RAX, and sign into RCX
			//e->movdqa_regreg ( 4, RDX );
			//e->psrlqregimm ( 4, 63 );
			//e->pslldregimm ( 4, 31 );
			e->psrlqregimm ( RDX, 29 );
			e->pblendwregregimm ( RAX, RDX, 0x33 );
			//e->movdqa_regreg ( RBX, 4 );
		}
		
		if ( i.xyzw & 0xa )
		{
			// convert to double (round2)
			e->movdqa_regreg ( RDX, RAX );
			//e->movdqa_regreg ( 4, RAX );
			e->psrlqregimm ( RDX, 32 );
			//e->psllqregimm ( RDX, 33 );
			//e->psrlqregimm ( RDX, 1 + 11 - 8 );
			e->psllqregimm ( RDX, 29 );
			//e->psrlqregimm ( 4, 63 );
			//e->psllqregimm ( 4, 63 );
			//e->porregreg ( RDX, 4 );

			
			e->movdqa_regreg ( 4, RCX );
			//e->movdqa_regreg ( 5, RCX );
			e->psrlqregimm ( 4, 32 );
			//e->psllqregimm ( 4, 33 );
			//e->psrlqregimm ( 4, 1 + 11 - 8 );
			e->psllqregimm ( 4, 29 );
			//e->psrlqregimm ( 5, 63 );
			//e->psllqregimm ( 5, 63 );
			//e->porregreg ( 4, 5 );

			
			// debug
			//e->movdqa_memreg ( & v4, RAX );
			//e->movdqa_memreg ( & v5, RCX );
			
			// add to one arg
			e->paddqregreg ( RDX, 5 );
			
			// multiply (round2)
			e->mulpdregreg ( RDX, 4 );

			

			
			// merge result (round2) without sign into RAX, and sign into RCX
			//e->movdqa_regreg ( 4, RDX );
			//e->psrlqregimm ( 4, 63 );
			//e->psllqregimm ( 4, 63 );
			e->psllqregimm ( RDX, 3 );
			//e->psrlqregimm ( RDX, 1 );
			e->pblendwregregimm ( RAX, RDX, 0xcc );
			//e->pblendwregregimm ( RBX, 4, 0xcc );
		}
		
		// debug
		//e->movdqa_memreg ( & v0, RDX );
		
		// debug
		//e->movdqa_memreg ( & v1, 4 );

		
		// pull overflow flags
		e->movmskpsregreg ( RAX, RAX );
		
		// if overflow, then maximize result
		e->movdqa_regreg ( RCX, RAX );
		e->psradregimm ( RCX, 31 );
		//e->psrldregimm ( RCX, 1 );
		e->porregreg ( RAX, RCX );
		e->pslldregimm ( RAX, 1 );
		e->psrldregimm ( RAX, 1 );

		// debug
		//e->movdqa_memreg ( & v1, RCX );

		// test for zero (also zero flag on underflow)
		//e->pxorregreg ( RCX, RCX );
		//e->pcmpeqdregreg ( RCX, RAX );

		// test for zero
		// puts zero flag in RDX
		e->movddup_regmem ( RDX, &c_lUFTest );
		e->pcmpgtdregreg ( RDX, RAX );
		
		// pull zero flags
		e->movmskpsregreg ( RDX, RDX );

		
		// pull sign flags
		e->movmskpsregreg ( RCX, RBX );
		
		
		// pull non-zero op flags
		//e->movdqa_regreg ( 4, RBX );
		//e->pslldregimm ( 4, 31 );
		//e->movmskpsregreg ( 8, 4 );
		
		
		// and non-zero op flags with zero flags
		e->AndRegReg32 ( 8, RDX );
		
		// clear sign flags where result is zero but not overflow
		e->MovRegReg32 ( 9, 8 );
		e->NotReg32 ( 9 );
		e->AndRegReg32 ( 9, RDX );
		e->NotReg32 ( 9 );
		e->AndRegReg32 ( RCX, 9 );
		
		// check zero flag against non-zero ops
		// zero flag is now in RDX
		// puts underflow flag in R4
		//e->movdqa_regreg ( 4, RBX );
		//e->pslldregimm ( 4, 31 );
		//e->pandregreg ( 4, RDX );
		
		// if zero/underflow, then clear result (but keep sign ??)
		// puts result in RDX
		//e->movdqa_regreg ( 4, RDX );
		e->pandnregreg ( RDX, RAX );
		
		
		// combine result with sign now
		//e->pslldregimm ( RDX, 1 );
		//e->psrldregimm ( RDX, 1 );
		e->psrldregimm ( RBX, 31 );
		e->pslldregimm ( RBX, 31 );
		e->porregreg ( RDX, RBX );
		
#endif


		// set result
		if ( i.xyzw != 0xf )
		{
			if ( pFd )
			{
				e->movdqa_regmem ( 5, pFd );
			}
			else
			{
				e->movdqa_regmem ( 5, & v->vf [ i.Fd ].sw0 );
			}
		}
		
		e->pshufdregregimm ( RDX, RDX, ( 0 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | ( 3 << 0 ) );
		
		if ( i.xyzw != 0xf )
		{
			e->pblendwregregimm ( RDX, 5, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
		}
		
		if ( pFd )
		{
			e->movdqa_memreg ( pFd, RDX );
		}
		else
		{
			if ( i.Fd )
			{
				e->movdqa_memreg ( & v->vf [ i.Fd ].sw0, RDX );
			}
		}

		
		// clear flags for vu units that do not operate
		if ( i.xyzw != 0xf )
		{
			e->AndReg32ImmX ( RAX, ( i.Value >> 21 ) & 0xf );
			e->AndReg32ImmX ( RCX, ( i.Value >> 21 ) & 0xf );
			e->AndReg32ImmX ( RDX, ( i.Value >> 21 ) & 0xf );
			e->AndReg32ImmX ( 8, ( i.Value >> 21 ) & 0xf );
		}

#ifdef ENABLE_VFLAGS
		e->MovMemReg8 ( & v->CurrentVFlags.vZeroFlag, RDX );
		e->MovMemReg8 ( & v->CurrentVFlags.vUnderflowFlag, 8 );
		e->MovMemReg8 ( & v->CurrentVFlags.vSignFlag, RCX );
		e->MovMemReg8 ( & v->CurrentVFlags.vOverflowFlag, RAX );
		if ( !v->SetStatus_Flag )
		{
		e->MovRegMem32 ( RAX, & v->CurrentVFlags.Flags );
		e->MovMemReg32 ( & v->CurrentVFlags.StatusFlags, RAX );
		ret = e->OrMemReg32 ( & v->CurrentVFlags.StickyFlags, RAX );
		}
#else
		// combine MAC flags
		e->ShlRegImm32 ( RCX, 4 );
		e->ShlRegImm32 ( 8, 8 );
		e->ShlRegImm32 ( RAX, 12 );
		
		e->OrRegReg32 ( RDX, RAX );
		e->OrRegReg32 ( RDX, RCX );
		e->OrRegReg32 ( RDX, 8 );
		
		// set MAC flags (RAX)
		ret = e->MovMemReg32 ( &v->vi [ VU::REG_MACFLAG ].s, RDX );
		
		
		// set status flags
		// RAX=overflow, RCX=sign, RDX=zero, 8=underflow
		
		// check if the lower instruction set stat flag already (there's only like one instruction that does this)
		if ( !v->SetStatus_Flag )
		{
			// restore RDX
			e->AndReg32ImmX ( RDX, 0xf );
			
			// set overflow status flag
			e->NegReg32 ( RAX );
			e->AdcRegReg32 ( RAX, RAX );
			
			// set underflow status flag
			e->NegReg32 ( 8 );
			e->AdcRegReg32 ( RAX, RAX );
			
			// set sign status flag
			e->NegReg32 ( RCX );
			e->AdcRegReg32 ( RAX, RAX );
			
			// set zero status flag
			e->NegReg32 ( RDX );
			e->AdcRegReg32 ( RAX, RAX );
			
			e->MovRegMem32 ( RDX, &v->vi [ VU::REG_STATUSFLAG ].s );
			e->AndReg32ImmX ( RAX, 0xf );
			e->MovRegReg32 ( RCX, RAX );
			e->ShlRegImm32 ( RAX, 6 );
			e->OrRegReg32 ( RAX, RCX );
			e->AndReg32ImmX ( RDX, ~0xf );
			e->OrRegReg32 ( RAX, RDX );
			ret = e->MovMemReg32 ( &v->vi [ VU::REG_STATUSFLAG ].s, RAX );
			
		}
#endif
	}
	
	return ret;
}



long Recompiler::Generate_VMADDp ( x64Encoder *e, VU* v, u32 bSub, Vu::Instruction::Format i, u32 FtComponentp, void *pFd, u32 *pFt, u32 FsComponentp )
{
	static const unsigned long long c_llMinExpDbl = ( 896LL << 52 );
	static const unsigned long long c_llExpMask = ( 0xffLL << 23 ) | ( 0xffLL << ( 23 + 32 ) );
	static const u64 c_lUFTest = ( 0x800000LL << 32 ) | ( 0x800000LL );
	
	static const u64 c_lUpperBound = ( 24LL << 32 ) | ( 24LL );
	static const u64 c_lLowerBound = ( 0xffffffe8LL << 32 ) | ( 0xffffffe8LL );
	
	long ret;
	
	ret = 1;


	if ( i.xyzw )
	{
		// load source regs
		// but they need to be shuffled into reverse on load
		//e->movdqa_regmem ( RAX, & v->vf [ i.Fs ].sw0 );
		
		if ( pFt )
		{
			e->movd_regmem ( RCX, (long*) pFt );
			e->pshufdregregimm ( RCX, RCX, FtComponentp );
		}
		else
		{
			//e->movdqa_regmem ( RCX, & v->vf [ i.Ft ].sw0 );
			e->pshufdregmemimm ( RCX, & v->vf [ i.Ft ].sw0, FtComponentp );
		}
		
		//e->pshufdregregimm ( RAX, RAX, FsComponentp );
		e->pshufdregmemimm ( RAX, & v->vf [ i.Fs ].sw0, FsComponentp );
		
		//e->pshufdregregimm ( RCX, RCX, FtComponentp );
		

#ifdef OPTIMIZE_RO_MULTIPLY_MADD
		// if fs or ft is r0 then the computation is easier/quicker
		if ( !pFt && ( ( ! i.Fs ) || ( ! i.Ft ) ) )
		{
			// if fs is not the r0 register then it has the value we want in RAX
			// and the r0 value is in RCX
			if ( i.Fs )
			{
				//e->movdqa_regreg ( RBX, RCX );
				e->movdqa_regreg ( RDX, RCX );
				e->movdqa_regreg ( 4, RAX );
			}
			else
			{
				// otherwise RAX/Fs has the r0 value
				//e->movdqa_regreg ( RBX, RAX );
				e->movdqa_regreg ( RDX, RAX );
				e->movdqa_regreg ( 4, RCX );
			}
			
			
			// check which values are zero/non-zero in r0
			e->pxorregreg ( RBX, RBX );
			e->pcmpeqdregreg ( RDX, RBX );
			
			// check if the operand has any zeros and flush
			//e->movdqa_regreg ( 4, RCX );
			e->pslldregimm ( 4, 1 );
			e->psrldregimm ( 4, 24 );
			e->pcmpeqdregreg ( RBX, 4 );
			
			// combine zero flags
			e->porregreg ( RDX, RBX );
			
			
			// get zero flags
			//e->movmskpsregreg ( RDX, RDX );
			
			// get multiply result
			e->psrldregimm ( RDX, 1 );
			if ( i.Fs )
			{
				// multiply r0 with fs/rax
				e->pandnregreg ( RDX, RAX );
			}
			else
			{
				// multiply r0 with ft/rcx
				e->pandnregreg ( RDX, RCX );
			}
			
			// get sign flags
			//e->movmskpsregreg ( RCX, RDX );
			
			
			// this line has a multi-purpose
			e->pcmpeqbregreg ( 4, 4 );
			
			// if MSUB and not MADD, then toggle sign
			if ( bSub )
			{
				e->pcmpeqbregreg ( RBX, RBX );
				e->pslldregimm ( RBX, 31 );
				e->pxorregreg ( RDX, RBX );
			}
			
			
			// combine result with sign now
			//e->psrldregimm ( RBX, 31 );
			//e->pslldregimm ( RBX, 31 );
			//e->porregreg ( RDX, RBX );

			
			// get multiply underflow sticky status flag
			e->XorRegReg32 ( 11, 11 );
			
			// no overflow
			e->pxorregreg ( RAX, RAX );

		}
		else
#endif
		{

#ifdef USE_NEW_VMADD_CODE

		// get the initial exponent
		//es = ( fs >> 23 ) & 0xff;
		e->movdqa_regreg( RDX, RAX );
		e->pslldregimm ( RDX, 1 );
		e->psrldregimm ( RDX, 24 );
		
		// check for zero
		e->pxorregreg ( RBX, RBX );
		e->pcmpeqdregreg ( RBX, RDX );
		
		//et = ( ft >> 23 ) & 0xff;
		e->movdqa_regreg( 4, RCX );
		e->pslldregimm ( 4, 1 );
		e->psrldregimm ( 4, 24 );
		
		// check for zero
		// puts what should be zero in r5
		e->pxorregreg ( 5, 5 );
		e->pcmpeqdregreg ( 5, 4 );
		e->porregreg ( 5, RBX );
		
		//ed = es + et - 127;
		e->pcmpeqbregreg ( RBX, RBX );
		e->padddregreg ( 4, RDX );
		e->psrldregimm ( RBX, 25 );
		e->psubdregreg ( 4, RBX );
		
		
		// add hidden bit
		//ms |= 0x00800000;
		//mt |= 0x00800000;
		e->pslldregimm ( RBX, 31 );


		// get sign into RDX
		// sign is always a xor
		//fd = ( fs ^ ft ) & -0x80000000;
		e->movdqa_regreg ( RDX, RAX );
		e->pxorregreg ( RDX, RCX );
		
		// if doing MSUB, then also need to toggle the sign
		if ( bSub )
		{
			e->pxorregreg ( RDX, RBX );
		}
		
		e->pandregreg ( RDX, RBX );
		
		
		
		// get the mantissa and add the hidden bit
		//ms = fs & 0x007fffff;
		//mt = ft & 0x007fffff;
		e->pslldregimm ( RAX, 8 );
		e->pslldregimm ( RCX, 8 );
		e->porregreg ( RAX, RBX );
		e->porregreg ( RCX, RBX );
		
		
		//e->movdqa_memreg ( & v0, RAX );
		//e->movdqa_memreg ( & v1, RCX );
		
		if ( __builtin_popcount ( i.xyzw ) < 3 )
		{
			e->pshufdregregimm ( RBX, RAX, _op_shuffle_lut_1 [ i.xyzw ] );
			e->pshufdregregimm ( RCX, RCX, _op_shuffle_lut_1 [ i.xyzw ] );
			e->pmuludqregreg ( RBX, RCX );
			e->pshufdregregimm ( RBX, RBX, _op_shuffle_lut_2 [ i.xyzw ] );
		}
		else
		{
			// do the multiply
			//md = ms * mt;
			if ( i.xyzw & 0x5 )
			{
				e->movdqa_regreg ( RBX, RAX );
				e->pmuludqregreg ( RBX, RCX );
				e->psrlqregimm ( RBX, 32 );
			}
			
			if ( i.xyzw & 0xa )
			{
				e->psrlqregimm ( RAX, 32 );
				e->psrlqregimm ( RCX, 32 );
				e->pmuludqregreg ( RAX, RCX );
				e->pblendwregregimm ( RBX, RAX, 0xcc );
			}
		}

		//e->movdqa_memreg ( & v2, RAX );
		//e->movdqa_memreg ( & v3, RBX );

		
		// first shift right by the 23
		// shift and combine
		
		// get bit 47
		//ext = md >> 47;
		e->movdqa_regreg ( RAX, RBX );
		e->psradregimm ( RAX, 31 );
		
		// get the result
		//md >>= ( 23 + ext );
		e->movdqa_regreg ( RCX, RBX );
		e->psrldregimm ( RCX, 1 );
		e->pblendvbregreg ( RBX, RCX );
		
		
		//e->movdqa_memreg ( & v4, RAX );
		//e->movdqa_memreg ( & v5, RBX );
		
		// remove the hidden bit
		//md &= 0x7fffff;
		e->pslldregimm ( RBX, 2 );
		e->psrldregimm ( RBX, 9 );
		
		// update exponent
		//ed += ext;
		e->psubdregreg ( 4, RAX );
		
		// check if result is zero
		e->pxorregreg ( RAX, RAX );
		e->pcmpeqdregreg ( RAX, 4 );
		e->porregreg ( 5, RAX );

		// only under flow if not zero!
		e->movdqa_regreg ( RAX, 5 );
		e->pandnregreg ( RAX, 4 );
		
		// get exponent underflow into RCX
		e->movmskpsregreg ( 11, RAX );
		
		// set to zero for under flow
		//e->movdqa_regreg ( RAX, 4 );
		e->psradregimm ( RAX, 31 );
		e->porregreg ( 5, RAX );
		
		// get exponent overflow into R8 (*note* remove the underflow from the bits, can only have one or the other)
		e->pslldregimm ( 4, 23 );
		
		// only if not under flow
		e->movdqa_regreg ( RAX, 5 );
		e->pandnregreg ( RAX, 4 );
		
		// pull overflow flags
		//e->movmskpsregreg ( RAX, RAX );
		
		
		// check if ( es == 0 ) || ( et == 0 ) || ( ed <= 0 )
		// puts zero exponent mask in RDX
		// puts exponent underflow in RBX
		
		// add in the exponent
		e->porregreg ( RBX, 4 );
		
		// maximize on overflow
		e->psradregimm ( RAX, 31 );
		e->porregreg ( RBX, RAX );
		
		// clear sign
		e->pslldregimm ( RBX, 1 );
		e->psrldregimm ( RBX, 1 );
		
		// get the sign flags into RAX
		//e->movmskpsregreg ( RCX, RDX );
		
		// get exponent zero flags into RDX
		//e->movmskpsregreg ( RDX, 5 );
		
		// clear the zero results
		e->pandnregreg ( 5, RBX );
		
		// add in the sign (sign is in RDX)
		e->porregreg ( RDX, 5 );
		

		// this line has a multi-purpose (MADD only)
		e->pcmpeqbregreg ( 4, 4 );
#else


		// get signs into RBX
		e->movdqa_regreg ( RBX, RAX );
		e->pxorregreg ( RBX, RCX );
		
		// clear signs
		e->pslldregimm ( RAX, 1 );
		e->psrldregimm ( RAX, 1 );
		e->pslldregimm ( RCX, 1 );
		e->psrldregimm ( RCX, 1 );
		
		// clear zero exponents
		e->pcmpeqbregreg ( RDX, RDX );
		e->psrldregimm ( RDX, 9 );
		e->movdqa_regreg ( 5, RAX );
		e->pcmpgtdregreg ( 5, RDX );
		e->pandregreg ( RAX, 5 );
		e->pandregreg ( RCX, 5 );
		e->movdqa_regreg ( 4, RCX );
		e->pcmpgtdregreg ( 4, RDX );
		e->pandregreg ( RAX, 4 );
		e->pandregreg ( RCX, 4 );
		
		// get the non-zero results with logical and
		e->pandregreg ( 4, 5 );
		
		// load constant into R5
		e->movddup_regmem ( 5, & c_llMinExpDbl );
		
		// stuff into RBX with the signs for now
		//e->psrldregimm ( 4, 1 );
		//e->porregreg ( RBX, 4 );
		
		// get the non-zero flags
		e->movmskpsregreg ( 10, 4 );



		

		
		if ( i.xyzw & 0x5 )
		{
			// convert to double (round1)
			e->movdqa_regreg ( RDX, RAX );
			//e->movdqa_regreg ( 4, RAX );
			e->psllqregimm ( RDX, 33 );
			e->psrlqregimm ( RDX, 1 + 11 - 8 );
			//e->psrldregimm ( 4, 31 );
			//e->psllqregimm ( 4, 63 );
			//e->porregreg ( RDX, 4 );
			
			
			e->movdqa_regreg ( 4, RCX );
			//e->movdqa_regreg ( 5, RCX );
			e->psllqregimm ( 4, 33 );
			e->psrlqregimm ( 4, 1 + 11 - 8 );
			//e->psrldregimm ( 5, 31 );
			//e->psllqregimm ( 5, 63 );
			//e->porregreg ( 4, 5 );




			// add to one arg
			e->paddqregreg ( RDX, 5 );


			
			// multiply (round1)
			e->mulpdregreg ( RDX, 4 );

			

			
			// merge result (round1) without sign into RAX, and sign into RCX
			//e->movdqa_regreg ( 4, RDX );
			//e->psrlqregimm ( 4, 63 );
			//e->pslldregimm ( 4, 31 );
			e->psrlqregimm ( RDX, 29 );
			e->pblendwregregimm ( RAX, RDX, 0x33 );
			//e->movdqa_regreg ( RBX, 4 );
		}
		
		if ( i.xyzw & 0xa )
		{
			// convert to double (round2)
			e->movdqa_regreg ( RDX, RAX );
			//e->movdqa_regreg ( 4, RAX );
			e->psrlqregimm ( RDX, 32 );
			//e->psllqregimm ( RDX, 33 );
			//e->psrlqregimm ( RDX, 1 + 11 - 8 );
			e->psllqregimm ( RDX, 29 );
			//e->psrlqregimm ( 4, 63 );
			//e->psllqregimm ( 4, 63 );
			//e->porregreg ( RDX, 4 );

			
			e->movdqa_regreg ( 4, RCX );
			//e->movdqa_regreg ( 5, RCX );
			e->psrlqregimm ( 4, 32 );
			//e->psllqregimm ( 4, 33 );
			//e->psrlqregimm ( 4, 1 + 11 - 8 );
			e->psllqregimm ( 4, 29 );
			//e->psrlqregimm ( 5, 63 );
			//e->psllqregimm ( 5, 63 );
			//e->porregreg ( 4, 5 );

			
			// debug
			//e->movdqa_memreg ( & v4, RAX );
			//e->movdqa_memreg ( & v5, RCX );
			
			// add to one arg
			e->paddqregreg ( RDX, 5 );
			
			// multiply (round2)
			e->mulpdregreg ( RDX, 4 );

			

			
			// merge result (round2) without sign into RAX, and sign into RCX
			//e->movdqa_regreg ( 4, RDX );
			//e->psrlqregimm ( 4, 63 );
			//e->psllqregimm ( 4, 63 );
			e->psllqregimm ( RDX, 3 );
			//e->psrlqregimm ( RDX, 1 );
			e->pblendwregregimm ( RAX, RDX, 0xcc );
			//e->pblendwregregimm ( RBX, 4, 0xcc );
		}
		
		// debug
		//e->movdqa_memreg ( & v0, RDX );
		
		// debug
		//e->movdqa_memreg ( & v1, 4 );

		
		// pull overflow flags
		
		// if overflow, then maximize result
		// puts multiply overflow flag into RAX
		// puts result into after overflow check into RCX
		e->movdqa_regreg ( RCX, RAX );
		e->psradregimm ( RAX, 31 );
		
		// save overflow flags for multiply result
		//e->movdqa_memreg ( vOverflow, RCX );
		
		//e->psrldregimm ( RCX, 1 );
		e->porregreg ( RCX, RAX );
		e->pslldregimm ( RCX, 1 );
		e->psrldregimm ( RCX, 1 );

		// debug
		//e->movdqa_memreg ( & v1, RCX );

		// test for zero (also zero flag on underflow)
		//e->pxorregreg ( RCX, RCX );
		//e->pcmpeqdregreg ( RCX, RAX );

		// test for zero
		// puts zero flag in RDX
		e->movddup_regmem ( RDX, &c_lUFTest );
		e->pcmpgtdregreg ( RDX, RCX );


		// save zero flags into R11
		e->movmskpsregreg ( 11, RDX );
		
		
		// check zero flag against non-zero ops
		// zero flag is now in RDX
		// puts underflow flag in R4
		//e->movdqa_regreg ( 4, RBX );
		//e->pslldregimm ( 4, 31 );
		//e->pandregreg ( 4, RDX );
		
		// if zero/underflow, then clear result (but keep sign ??)
		// puts result in RDX
		//e->movdqa_regreg ( 4, RDX );
		e->pandnregreg ( RDX, RCX );
		
		
		// if result should be non-zero but is zero, then underflow
		// underflow = nonzero and zero
		// this puts underflow in RAX
		//e->pandregreg ( RCX, 4 );
		
		// save flags for multiply underflow into R11
		//e->movdqa_memreg ( & vUnderflow, RCX );
		//e->movmskpsregreg ( 11, RCX );
		
		// this line has a multi-purpose
		e->pcmpeqbregreg ( 4, 4 );
		
		// if MSUB and not MADD, then toggle sign
		if ( bSub )
		{
			e->pxorregreg ( RBX, 4 );
		}
		
		
		// combine result with sign now
		//e->pslldregimm ( RDX, 1 );
		//e->psrldregimm ( RDX, 1 );
		e->psrldregimm ( RBX, 31 );
		e->pslldregimm ( RBX, 31 );
		e->porregreg ( RDX, RBX );

		
		
		
		// get multiply underflow sticky status flag
		e->AndRegReg32 ( 11, 10 );
		
#endif
		}
		


		// -----------------------------ADD ---------------------------


		// load source regs
		// but they need to be shuffled into reverse on load
		//e->movdqa_regmem ( RAX, & va );
		//e->movdqa_regmem ( RAX, RDX );
		
		


		
		//e->movdqa_regmem ( RCX, &v->dACC[ 0 ].l );
		//e->pshufdregregimm ( RCX, RCX, ( 0 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | ( 3 << 0 ) );
		e->pshufdregmemimm ( RCX, &v->dACC[ 0 ].l, ( 0 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | ( 3 << 0 ) );
		
		// check for multiply overflow
		e->pblendvbregreg ( RCX, RDX );

		
		// debug
		//e->movdqa_memreg ( & v0, RAX );
		
		// debug
		//e->movdqa_memreg ( & v1, RCX );


		
		// check for accumulator +/-max
		
		// the multi-purpose line from above
		//e->pcmpeqbregreg ( 4, 4 );
		
		e->psrldregimm ( 4, 1 );
		e->movdqa_regreg ( RAX, 4 );
		e->pandregreg ( 4, RCX );
		e->pcmpeqdregreg ( RAX, 4 );
		e->pblendvbregreg ( RDX, RCX );
		
		
		
		
		// get exponents
		
		// no need to move into RDX in this case because it was here previously
		e->movdqa_regreg ( RAX, RDX );
		e->pslldregimm ( RDX, 1 );
		e->psrldregimm ( RDX, 24 );
		e->movdqa_regreg ( 4, RCX );
		e->pslldregimm ( 4, 1 );
		e->psrldregimm ( 4, 24 );

		


		// clear zero exponents ??
		/*
		e->pxorregreg ( RBX, RBX );
		e->pcmpeqdregreg ( RBX, RDX );
		e->psrldregimm ( RBX, 1 );
		e->pandnregreg ( RBX, RAX );
		e->movdqa_regreg ( RAX, RBX );
		e->pxorregreg ( RBX, RBX );
		e->pcmpeqdregreg ( RBX, 4 );
		e->psrldregimm ( RBX, 1 );
		e->pandnregreg ( RBX, RCX );
		e->movdqa_regreg ( RCX, RBX );
		*/
		
		
		// get difference
		e->psubdregreg ( RDX, 4 );
		
		
		// if positive 24 or greater then zero Ft
		e->movddup_regmem ( 5, (long long*) &c_lUpperBound );
		e->pcmpgtdregreg ( 5, RDX );
		e->pandregreg ( RCX, 5 );


		// if negative 24 or less, then zero Fs
		e->movddup_regmem ( 5, (long long*) &c_lLowerBound );
		e->pcmpgtdregreg ( RDX, 5 );
		e->pandregreg ( RAX, RDX );

		
		if ( ( __builtin_popcount ( i.xyzw ) < 3 ) )
		{
			e->pshufdregregimm ( RDX, RAX, _op_shuffle_lut_1 [ i.xyzw ] );

				//e->movdqa_regreg ( RDX, RAX );
				//e->movdqa_regreg ( 4, RAX );
				e->movdqa_regreg ( 4, RDX );
				e->psllqregimm ( RDX, 33 );
				e->psrlqregimm ( RDX, 1 + 11 - 8 );
				e->psrldregimm ( 4, 31 );
				e->psllqregimm ( 4, 63 );
				e->porregreg ( RDX, 4 );
				
			e->pshufdregregimm ( 5, RCX, _op_shuffle_lut_1 [ i.xyzw ] );
				
				//e->movdqa_regreg ( 5, RCX );
				//e->movdqa_regreg ( 4, RCX );
				e->movdqa_regreg ( 4, 5 );
				e->psllqregimm ( 5, 33 );
				e->psrlqregimm ( 5, 1 + 11 - 8 );
				e->psrldregimm ( 4, 31 );
				e->psllqregimm ( 4, 63 );
				e->porregreg ( 5, 4 );
				
				
				// add (round1)
				e->addpdregreg ( RDX, 5 );
				
				
				// merge result (round1) without sign into RAX, and sign into RCX
				e->movdqa_regreg ( 4, RDX );
				e->psrlqregimm ( 4, 63 );
				e->pslldregimm ( 4, 31 );
				
				e->psrlqregimm ( RDX, 29 );
				
				//e->pblendwregregimm ( RAX, RDX, 0x33 );
				//e->movdqa_regreg ( RBX, 4 );
			
			e->pshufdregregimm ( RAX, RDX, _op_add_shuffle_lut_2 [ i.xyzw ] );
			e->pshufdregregimm ( RBX, 4, _op_add_shuffle_lut_2 [ i.xyzw ] );
		}
		else
		{
			if ( i.xyzw & 0x5 )
			{
				// convert to double (round1)
				e->movdqa_regreg ( RDX, RAX );
				e->movdqa_regreg ( 4, RAX );
				e->psllqregimm ( RDX, 33 );
				e->psrlqregimm ( RDX, 1 + 11 - 8 );
				e->psrldregimm ( 4, 31 );
				e->psllqregimm ( 4, 63 );
				e->porregreg ( RDX, 4 );
				
				
				e->movdqa_regreg ( 5, RCX );
				e->movdqa_regreg ( 4, RCX );
				e->psllqregimm ( 5, 33 );
				e->psrlqregimm ( 5, 1 + 11 - 8 );
				e->psrldregimm ( 4, 31 );
				e->psllqregimm ( 4, 63 );
				e->porregreg ( 5, 4 );
				
				
				// add (round1)
				e->addpdregreg ( RDX, 5 );
				
				
				// merge result (round1) without sign into RAX, and sign into RCX
				e->movdqa_regreg ( 4, RDX );
				e->psrlqregimm ( 4, 63 );
				e->pslldregimm ( 4, 31 );
				//e->psllqregimm ( RDX, 4 );
				e->psrlqregimm ( RDX, 29 );
				e->pblendwregregimm ( RAX, RDX, 0x33 );
				e->movdqa_regreg ( RBX, 4 );
			}
			
			if ( i.xyzw & 0xa )
			{
				// convert to double (round2)
				e->movdqa_regreg ( RDX, RAX );
				e->movdqa_regreg ( 4, RAX );
				e->psrlqregimm ( RDX, 32 );
				e->psllqregimm ( RDX, 33 );
				e->psrlqregimm ( RDX, 1 + 11 - 8 );
				e->psrlqregimm ( 4, 63 );
				e->psllqregimm ( 4, 63 );
				e->porregreg ( RDX, 4 );

				
				e->movdqa_regreg ( 5, RCX );
				e->movdqa_regreg ( 4, RCX );
				e->psrlqregimm ( 5, 32 );
				e->psllqregimm ( 5, 33 );
				e->psrlqregimm ( 5, 1 + 11 - 8 );
				e->psrlqregimm ( 4, 63 );
				e->psllqregimm ( 4, 63 );
				e->porregreg ( 5, 4 );


				// debug
				//e->movdqa_memreg ( & v4, RAX );
				//e->movdqa_memreg ( & v5, RCX );
				
				
				// add (round2)
				e->addpdregreg ( RDX, 5 );

				
				// merge result (round2) without sign into RAX, and sign into RCX
				e->movdqa_regreg ( 4, RDX );
				e->psrlqregimm ( 4, 63 );
				e->psllqregimm ( 4, 63 );
				e->psllqregimm ( RDX, 3 );
				//e->psrlqregimm ( RDX, 1 );
				
				// want to merge into RDX in this case since pblendvb is coming up
				e->pblendwregregimm ( RAX, RDX, 0xcc );
				
				
				e->pblendwregregimm ( RBX, 4, 0xcc );
			}
		}
		
		
		// pull overflow flags
		e->movmskpsregreg ( RAX, RAX );
		
		// if overflow, then maximize result
		e->movdqa_regreg ( RCX, RAX );
		e->psradregimm ( RCX, 31 );
		//e->psrldregimm ( RCX, 1 );
		e->porregreg ( RAX, RCX );
		e->pslldregimm ( RAX, 1 );
		e->psrldregimm ( RAX, 1 );

		// debug
		//e->movdqa_memreg ( & v1, RCX );

		// test for zero (also zero flag on underflow)
		e->pxorregreg ( RCX, RCX );
		e->pcmpeqdregreg ( RCX, RAX );

		// test for underflow
		e->movddup_regmem ( RDX, (long long*) &c_lUFTest );
		e->pcmpgtdregreg ( RDX, RAX );
		
		// if underflow, then clear result (but keep sign ??)
		e->movdqa_regreg ( 4, RDX );
		e->pandnregreg ( 4, RAX );
		
		
		// combine result with sign now
		e->pslldregimm ( 4, 1 );
		e->psrldregimm ( 4, 1 );
		e->porregreg ( 4, RBX );

		// not underflow if it is zero
		// puts underflow in RAX
		e->movdqa_regreg ( RAX, RCX );
		e->pandnregreg( RAX, RDX );
		
		
		
		// clear sign flag if signed zero ?? unless underflow ??
		// but test for underflow before testing for zero
		// so and not zero, then and underflow
		e->movdqa_regreg ( RDX, RCX );
		e->pandnregreg ( RDX, RBX );
		//e->pandregreg ( RDX, RAX );
		
		// pull sign flags
		// RDX = sign flag
		e->movmskpsregreg ( RCX, RDX );
		
		// zero flag is also set when underflow
		// RAX = underflow flag, RCX = zero flag
		e->porregreg ( RCX, RAX );
		
		// pull zero flags
		e->movmskpsregreg ( RDX, RCX );
		
		// pull underflow flags
		// RAX = underflow flag
		e->movmskpsregreg ( 8, RAX );
		
		// set result
		if ( i.xyzw != 0xf )
		{
			if ( pFd )
			{
				e->movdqa_regmem ( 5, pFd );
			}
			else
			{
				e->movdqa_regmem ( 5, & v->vf [ i.Fd ].sw0 );
			}
		}
		
		e->pshufdregregimm ( 4, 4, 0x1b );	//FdComponentp );
		
		if ( i.xyzw != 0xf )
		{
			e->pblendwregregimm ( 4, 5, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
		}
		
		if ( pFd )
		{
			e->movdqa_memreg ( pFd, 4 );
		}
		else
		{
			if ( i.Fd )
			{
				e->movdqa_memreg ( & v->vf [ i.Fd ].sw0, 4 );
			}
		}

		
		// clear flags for vu units that do not operate
		if ( i.xyzw != 0xf )
		{
			e->AndReg32ImmX ( RAX, ( i.Value >> 21 ) & 0xf );
			e->AndReg32ImmX ( RCX, ( i.Value >> 21 ) & 0xf );
			e->AndReg32ImmX ( RDX, ( i.Value >> 21 ) & 0xf );
			e->AndReg32ImmX ( 8, ( i.Value >> 21 ) & 0xf );
		
			// this is for madd/msub
			e->AndReg32ImmX ( 11, ( i.Value >> 21 ) & 0xf );
		}

#ifdef ENABLE_VFLAGS
		e->MovMemReg8 ( & v->CurrentVFlags.vZeroFlag, RDX );
		e->MovMemReg8 ( & v->CurrentVFlags.vUnderflowFlag, 8 );
		e->MovMemReg8 ( & v->CurrentVFlags.vSignFlag, RCX );
		e->MovMemReg8 ( & v->CurrentVFlags.vOverflowFlag, RAX );
		if ( !v->SetStatus_Flag )
		{
		e->MovRegMem32 ( RAX, & v->CurrentVFlags.Flags );
		e->MovMemReg32 ( & v->CurrentVFlags.StatusFlags, RAX );
		e->OrMemReg32 ( & v->CurrentVFlags.StickyFlags, RAX );
		
		// in addition, for MADD there is the sticky underflow flag
		ret = e->OrMemReg8 ( & v->CurrentVFlags.vUnderflowFlag_Sticky, 11 );
		}
#else
		// combine MAC flags
		e->ShlRegImm32 ( RCX, 4 );
		e->ShlRegImm32 ( 8, 8 );
		e->ShlRegImm32 ( RAX, 12 );
		
		e->OrRegReg32 ( RDX, RAX );
		e->OrRegReg32 ( RDX, RCX );
		e->OrRegReg32 ( RDX, 8 );
		
		// set MAC flags (RAX)
		ret = e->MovMemReg32 ( &v->vi [ VU::REG_MACFLAG ].s, RDX );
		
		// set status flags
		// RAX=overflow, RCX=sign, RDX=zero, 8=underflow
		
		// check if the lower instruction set stat flag already (there's only like one instruction that does this)
		if ( !v->SetStatus_Flag )
		{
			// restore RDX
			e->AndReg32ImmX ( RDX, 0xf );
			
			// set overflow status flag
			e->NegReg32 ( RAX );
			e->AdcRegReg32 ( RAX, RAX );
			
			// set underflow status flag
			e->NegReg32 ( 8 );
			e->AdcRegReg32 ( RAX, RAX );
			
			// set sign status flag
			e->NegReg32 ( RCX );
			e->AdcRegReg32 ( RAX, RAX );
			
			// set zero status flag
			e->NegReg32 ( RDX );
			e->AdcRegReg32 ( RAX, RAX );
			
			e->MovRegMem32 ( RDX, &v->vi [ VU::REG_STATUSFLAG ].s );
			e->AndReg32ImmX ( RAX, 0xf );
			e->MovRegReg32 ( RCX, RAX );
			e->ShlRegImm32 ( RAX, 6 );
			e->OrRegReg32 ( RAX, RCX );
			
			// for madd/msub
			e->NegReg32 ( 11 );
			e->AdcRegReg32 ( RCX, RCX );
			e->AndReg32ImmX ( RCX, 1 );
			e->ShlRegImm32 ( RCX, 8 );
			e->OrRegReg32 ( RAX, RCX );
			
			e->AndReg32ImmX ( RDX, ~0xf );
			e->OrRegReg32 ( RAX, RDX );
			ret = e->MovMemReg32 ( &v->vi [ VU::REG_STATUSFLAG ].s, RAX );
			
		}
#endif
	}
	
	return ret;
}





// returns number of instructions that were recompiled
u32 Recompiler::Recompile ( VU* v, u32 BeginAddress )
{
	u32 Address, Block;
	s32 ret, Cycles;
	s32 reti;
	
	s32 retLo, retHi;
	s32 OpLvlSave;

	//Vu::Instruction::Format inst;
	//Vu::Instruction::Format instLO;
	//Vu::Instruction::Format instHI;
	//Vu::Instruction::Format v->NextInstLO;
	
	//u32 StartBlockIndex, BlockIndex, SaveBlockIndex;
	
	// number of instructions in current run
	u32 RunCount;
	
	u32 RecompileCount;
	u32 MaxCount;
	
	//u32 ProjectedMaxCount;
	
	//u32 Snapshot_Address [ 4 ];
	//u32 Snapshot_RecompileCount [ 4 ];
	
	//static u64 MemCycles;
	u32 SetCycles;
	
	//u32* pInstrPtr;
	
	u32* pSrcCodePtr;
	u32* pNextCode;
	
	//u32* pCmpCodePtr;
	//u32* pSaveCodePtr;
	//u32* pSaveCmpPtr;
	
	u32 SaveReg0;
	u32 ulCacheLineCount;
	
	//u64 LocalCycleCount, CacheBlock_CycleCount;
	
	int RetJumpCounter;
	
	//char* ReturnFromCacheReload;
	
	int i;
	
	unsigned long First_LastModifiedReg;
	
	s32 MaxBlocks;
	
	u32 NextAddress;
	
#ifdef VERBOSE_RECOMPILE
cout << "\nrecompile: starting recompile.";
#endif

	// set current recompiler as the one doing the recompiling
	_REC = this;

	// need to first clear forward branch targets for the block
	memset ( pForwardBranchTargets, 0x00, sizeof( u32 ) * MaxStep );
	
	// initialize forward branch index
	// note: Will need a larger branch index table in the encoder object for larger code blocks than 128 instructions
	ForwardBranchIndex = c_ulForwardBranchIndex_Start;


	// mask address
	// don't do this
	//StartAddress &= c_iAddress_Mask;
	
	
	// set the encoder to use
	e = InstanceEncoder;
	
	// the starting address needs to be on a block boundary
	BeginAddress = ( BeginAddress >> ( 3 + MaxStep_Shift ) ) << ( 3 + MaxStep_Shift );
	
	// save the address?
	Address = BeginAddress;
	
	// set the start address for the current block so recompiler can access it
	CurrentBlock_StartAddress = BeginAddress;
	
	// set the start address for the next block also
	NextBlock_StartAddress = CurrentBlock_StartAddress + ( 1 << ( 3 + MaxStep_Shift ) );
	
	// set the current optimization level
	v->OpLevel = v->OptimizeLevel;
	
	// get the block to encode in
	// new formula
	Block = ( BeginAddress >> ( 3 + MaxStep_Shift ) ) & NumBlocks_Mask;
	
	
	
	// start in code block
	e->StartCodeBlock ( Block );
	
	// set the start address for code block
	// address must actually match exactly. No mask
	StartAddress [ Block ] = BeginAddress;
	
	// set the instruction
	//Instructions [ Block ] = *((u32*) SrcCode);
	//pInstrPtr = & ( Instructions [ Block << MaxStep_Shift ] );
	
	
	// start cycles at zero
	Cycles = 0;
	
	// start PC
	//LocalPC = r->PC;
	
	
	// init count of recompiled instructions
	RecompileCount = 0;
	
	
	// want to stop at cache boundaries (would need extra code there anyways)
	// this is handled in loop now
	//MaxCount = MaxStep - ( ( Address >> 2 ) & MaxStep_Mask );
	//if ( MaxCount <= 0 ) MaxCount = 1;
	// set the maximum number of instructions to encode
	MaxCount = MaxStep;
	
	
	// NextPC has not been modified yet
	Local_NextPCModified = false;
	
	// some instructions need to stop encoding either before or after the instruction, at least for now
	// if stopping before, it keeps the instruction if there is nothing before it in the run
	v->bStopEncodingAfter = false;
	v->bStopEncodingBefore = false;
	
	// don't reset the cycle count yet
	bResetCycleCount = false;


	
	// should set local last modified register to 255
	Local_LastModifiedReg = 255;
	
	reti = 1;
	
	
	

	// clear delay slot
	//RDelaySlots [ 0 ].Value = 0;
	//RDelaySlots [ 1 ].Value = 0;

	// clear delay slot valid bits
	//RDelaySlots_Valid = 0;
	

	
	/////////////////////////////////////////////////////
	// note: multiply and divide require cycle count to be updated first
	// since they take more than one cycle to complete
	// same for mfhi and mflo, because they are interlocked
	// same for COP2 instructions
	// same for load and store
	// do the same for jumps and branches
	//////////////////////////////////////////////////////

	
	
	// get the starting block to store instruction addresses and cycle counts
	StartBlockIndex = ( Address >> 3 ) & ulIndex_Mask;
	BlockIndex = StartBlockIndex;

	// instruction count for current run
	RunCount = 0;
	
	// current delay slot index
	//DSIndex = 0;
	
	
	
	
	// this should get pointer to the instruction
	pSrcCodePtr = RGetPointer ( v, Address );
	
	
	
	// one cycle to fetch each instruction
	MemCycles = 1;


	// need to keep track of cycles for run
	//LocalCycleCount = MemCycles - 1;
	//CacheBlock_CycleCount = 0;
	LocalCycleCount = 0;

	// need to know of any other jumps to return
	RetJumpCounter = 0;
	
	
	CountOfVConsts = 0;
	
	// no branch delay slots yet
	v->Status_BranchDelay = 0;
	
	// no e-bit delay slots yet
	Status_EBit = 0;
	
#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiler: Starting loop";
#endif

	// for loads
	// 1. check that there are no events. If so, update Cycles,NextPC, then return
	// 2. check for synchronous interrupt
	// 3. check that there are no conflicts. If so, put load in delay slot, update Cycles,NextPC, then return
	// 4. encode load, then encode load delay slot
	// 5. if going across cache line and next line is not loaded, then put load in delay slot and return
	// 6. if it is a store in the delay slot, then can just process normally as if there is no delay slot and immediately load
	
	// for stores
	// 1. check that there are no events. If so, update Cycles,NextPC, then return
	// 2. check for synchronous interrupt
	// 3. encode store
	
	// for jumps/branches
	// 1. check that there are no events. If so, update Cycles,NextPC, then return
	// 2. check for synchronous interrupt (for jumps that might have them)
	// 3. check that there are no loads,stores,branches,delay slots, in the delay slot. If so, put branch/jump in delay slot, update Cycles,NextPC, then return
	// 4. encode jump/branch then encode delay slot
	// 5. if branching backwards within same block, if cached then make sure cache-block is loaded and then jump, implement forward jumps later?
	// 6. if not branching within same block or forward jumping before implementation, then update Cycles,NextPC, then return
	// 7. if going across cache blocks and next block not loaded, then put in delay slot and return
	
	// other delay slot instructions
	// 1. check that there are no conflicts with delay slot. If so, update Cycles,NextPC, then return
	// 2. encode instruction then encode delay slot
	// 3. if going across cache blocks and next block not loaded, then put in delay slot and return
	
	// finding source registers
	// special instructions can use rs,rt as source registers
	// stores use rs,rt as source registers
	// immediates and loads use only rs as source register

	//for ( int i = 0; i < MaxStep; i++, Address += 4 )
	for ( i = 0; i < MaxCount; i++ )
	{
#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiling: ADDR=" << hex << Address;
#endif
		
		
		// start encoding a MIPS instruction
		e->StartInstructionBlock ();

		
#ifdef ENABLE_SINGLE_STEP
				
			v->bStopEncodingAfter = true;
#endif


		// mark the checkpoint in case encoding of instruction does not go well
		LastVConstCount = CountOfVConsts;

#ifdef VERBOSE_RECOMPILE
cout << " RunCount=" << dec << RunCount;
#endif

		// the VUs run cycle by cycle, so if putting more than one instruction in a run will need to advance cycle
		// needs to be in transition to next instruction but not after starting point from main loop
		if ( RunCount )
		{
#ifdef VERBOSE_RECOMPILE
cout << " AdvanceCycle";
#endif
			
			//static void AdvanceCycle ( VU* v )
			
#ifdef ENABLE_RECOMPILER_ADVANCE_CYCLE

			AdvanceCycle_rec ( e, v );
			
#else
	
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LeaRegMem64 ( RCX, v );
			e->Call ( (void*) AdvanceCycle );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

#endif

#ifdef VERBOSE_RECOMPILE
cout << "->AdvanceCycle_DONE";
#endif

		}
			
		
#ifdef VERBOSE_RECOMPILE
cout << " INSTR#" << dec << i;
//cout << " LOC=" << hex << ((u64) e->Get_CodeBlock_CurrentPtr ());
cout << " CycleDiff=" << dec << LocalCycleCount;
#endif


		// set r0 to zero for now
		//e->MovMemImm32 ( &r->GPR [ 0 ].u, 0 );
		
		// in front of the instruction, set NextPC to the next instruction
		// do this at beginning of code block
		// NextPC = PC + 4
		//e->MovMemImm32 ( &r->NextPC, Address + 4 );
		
		// get the instruction
		//inst.Value = *((u32*) SrcCode);
		instLO.Value = *(pSrcCodePtr + 0);
		instHI.Value = *(pSrcCodePtr + 1);
		
		
		// get the next instruction
		// note: this does not work if the next address is in a new cache block and the region is cached
		v->NextInstLO.Value = *(pSrcCodePtr + 2);

		
		{
			// not in cached region //
			
			// still need to check against edge of block
			if ( ! ( ( Address + 8 ) & ( MaxStep_Mask << 3 ) ) )
			{
				// this can actually happen, so need to prevent optimizations there
				NextInst.Value = -1;
			}
		}
		
		


#ifdef VERBOSE_RECOMPILE
cout << " OL=" << v->OpLevel;
#endif

		// check if a forward branch target needs to be set
		if ( pForwardBranchTargets [ BlockIndex & MaxStep_Mask ] )
		{
			// set the branch target
			e->SetJmpTarget ( pForwardBranchTargets [ BlockIndex & MaxStep_Mask ] );
		}
		
		// this is internal to recompiler and says where heading for instruction starts at
		pPrefix_CodeStart [ BlockIndex & MaxStep_Mask ] = (u8*) e->Get_CodeBlock_CurrentPtr ();
		
		// this can be changed by the instruction being recompiled to point to where the starting entry point should be for instruction instead of prefix
		pCodeStart [ BlockIndex ] = (u8*) e->Get_CodeBlock_CurrentPtr ();
		
	
			// must add one to the cycle offset for starting point because the interpreter adds an extra cycle at the end of run
			//CycleCount [ BlockIndex ] = LocalCycleCount + 1;
			CycleCount [ BlockIndex ] = LocalCycleCount;
		
		
		//EndAddress [ BlockIndex ] = -1;
		



	if ( instHI.E )
	{
#ifdef INLINE_DEBUG
	debug << "; ***E-BIT SET***";
#endif

		switch ( v->OpLevel )
		{
				
#ifdef ENABLE_EBIT_RECOMPILE
			case 1:
				Status_EBit = 2;
				
				e->MovMemImm32 ( & v->Recompiler_EnableEBitDelay, 1 );
				break;
#endif

			default:
				// delay slot after e-bit
				v->bStopEncodingAfter = true;

				//Status.EBitDelaySlot_Valid |= 0x2;
				e->OrMemImm32 ( (long*) & v->Status.ValueHi, 0x2 );
				
				Status_EBit = 0;
				break;

		}
	}
	
	
#ifdef ENABLE_MBIT_RECOMPILE
	// M-bit must be VU0 only
	if ( !v->Number )
	{
		if ( instHI.M )
		{
#ifdef VERBOSE_RECOMPILE_MBIT
			// for now should alert
			cout << "\nhps2x64: VU0: NOTICE: M-bit set encountered during recompile!\n";
#endif
			
			// this should hopefully do the trick
			v->bStopEncodingAfter = true;
		}
	}
#endif
	
	
#ifdef VERBOSE_RECOMPILE_DBIT
	// alert if d or t is set
	//if ( CurInstHI.D )
	if ( instHI.D )
	{
		// register #28 is the FBRST register
		// the de bit says if the d-bit is enabled or not
		// de0 register looks to be bit 2
		// de1 register looks to be bit 10
		if ( !v->Number )
		{
			// check de0
			//if ( vi [ 28 ].u & ( 1 << 2 ) )
			//{
				cout << "\nhps2x64: ALERT: VU#" << v->Number << " D-bit is set! de0=" << hex << v->vi [ 28 ].u << "\n";
			//}
		}
		else
		{
			// check de1
			//if ( vi [ 28 ].u & ( 1 << 10 ) )
			//{
				cout << "\nhps2x64: ALERT: VU#" << v->Number << " D-bit is set! de1=" << hex << v->vi [ 28 ].u << "\n";
			//}
		}
	}
#endif

	
#ifdef VERBOSE_RECOMPILE_TBIT
	//if ( CurInstHI.T )
	if ( instHI.T )
	{
		cout << "\nhps2x64: ALERT: VU#" << v->Number << " T-bit is set!\n";
	}
#endif
	
	
	// execute HI instruction first ??
	
	// make sure the return values are set in case instruction is skipped
	retHi = 1;
	retLo = 1;
	
	// lower instruction has not set stat or clip flag
	v->SetStatus_Flag = 0;
	v->SetClip_Flag = 0;
	
	// check if Immediate or End of execution bit is set
	if ( instHI.I )
	{
		// lower instruction contains an immediate value //
		
		// first need to wait for source (and destination??) registers //

#ifdef ENABLE_RECOMPILER_BITMAP		
		// clear the bitmaps for source and destination registers
		Clear_FSrcReg ( v );
		Clear_ISrcReg ( v );
		Clear_DstReg ( v );
		
		// get source (and destination??) registers
		OpLvlSave = v->OpLevel;
		v->OpLevel = -1;
		Vu::Recompiler::RecompileHI ( InstanceEncoder, v, instHI, Address );
		v->OpLevel = OpLvlSave;
		
		// set source (and destination??) registers
		if ( ( v->FSrcBitmap.b0 & ~0xfULL ) | v->FSrcBitmap.b1 )
		{
			//e->MovRegImm64 ( RAX, v->FSrcBitmap.b0 );
			//e->MovMemReg64 ( & v->SrcRegs_Bitmap.b0, RAX );
			//e->MovRegImm64 ( RAX, v->FSrcBitmap.b1 );
			//e->MovMemReg64 ( & v->SrcRegs_Bitmap.b1, RAX );
			
			VectorConstants[ CountOfVConsts ].uq0 = v->FSrcBitmap.b0;
			VectorConstants[ CountOfVConsts ].uq1 = v->FSrcBitmap.b1;
			
			e->movdqa_regmem ( RAX, & VectorConstants[ CountOfVConsts ].uq0 );
			
			// inc has to come after the constant is used
			CountOfVConsts++;
			
			e->ptestregmem ( RAX, & v->Pipeline_Bitmap );
			e->Jmp8_E ( 0, 0 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->movdqa_memreg ( & v->SrcRegs_Bitmap.b0, RAX );
			
			e->LeaRegMem64 ( RCX, v );
			e->Call ( (void*) TestStall );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->SetJmpTarget8 ( 0 );
		}

		if ( ( v->ISrcBitmap >> 32 ) & ~1 )
		{
			e->TestMemImm32 ( ( (s32*) (long long*) & v->Int_Pipeline_Bitmap ) + 1, ( v->ISrcBitmap >> 32 ) );
			e->Jmp8_E ( 0, 0 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->MovRegImm64 ( RAX, v->ISrcBitmap );
			e->MovMemReg64 ( (long long*) & v->Int_SrcRegs_Bitmap, RAX );
			
			e->LeaRegMem64 ( RCX, v );
			e->Call ( (void*) TestStall_INT );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->SetJmpTarget8 ( 0 );
		}
#endif

		
		// start encoding a MIPS instruction
		e->StartInstructionBlock ();
		
		if ( !isNopHi ( instHI ) )
		{
			// *important* MUST execute the HI instruction BEFORE storing the immediate
			//Instruction::Execute::ExecuteInstructionHI ( this, CurInstHI );
			//Instruction::Execute::ExecuteInstructionHI ( this, instHI );
			retHi = Vu::Recompiler::RecompileHI ( InstanceEncoder, v, instHI, Address );
		}
#ifdef VERBOSE_RECOMPILE
		else
		{
cout << " NOP-HI";
		}
#endif
		
		// load immediate regiser with LO instruction
		//vi [ 21 ].u = CurInstLO.Value;
		//vi [ 21 ].u = CurInstLOHI.Lo.Value;
		ret = e->MovMemImm32 ( & v->vi [ 21 ].s, instLO.Value );
		

#ifdef ENABLE_SETDSTBITMAP

		// set destination bitmaps (*todo*)
		
#ifdef RECOMPILE_SETDSTBITMAP

		SetDstBitmap_rec ( e, v, v->FDstBitmap.b0, v->FDstBitmap.b1, v->IDstBitmap );
#else
		
#ifdef RESERVE_STACK_FRAME_FOR_CALL
		e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

		//static void Recompiler::SetDstBitmap ( VU* v, u64 b0, u64 b1, u64 i0 )
		e->LeaRegMem64 ( RCX, v );
		e->MovRegImm64 ( RDX, v->FDstBitmap.b0 );
		e->MovRegImm64 ( R8, v->FDstBitmap.b1 );
		e->MovRegImm64 ( R9, v->IDstBitmap );
		
		e->Call ( (void*) SetDstBitmap );
		
#ifdef RESERVE_STACK_FRAME_FOR_CALL
		ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

#endif

#endif
	}
	else
	{
		// execute lo/hi instruction normally //
		// unsure of order
		
		
		// execute LO instruction since it is an instruction rather than an immediate value
		//Instruction::Execute::ExecuteInstructionLO ( this, CurInstLO );
		//Instruction::Execute::ExecuteInstructionLO ( this, CurInstLOHI.Lo );
		
		// execute HI instruction
		//Instruction::Execute::ExecuteInstructionHI ( this, CurInstHI );
		//Instruction::Execute::ExecuteInstructionHI ( this, CurInstLOHI.Hi );

#ifdef ENABLE_RECOMPILER_BITMAP		
		// clear the bitmaps for source and destination registers
		Clear_FSrcReg ( v );
		Clear_ISrcReg ( v );
		Clear_DstReg ( v );
		
		// get source (and destination??) registers
		OpLvlSave = v->OpLevel;
		v->OpLevel = -1;
		Vu::Recompiler::RecompileLO ( InstanceEncoder, v, instLO, Address );
		Vu::Recompiler::RecompileHI ( InstanceEncoder, v, instHI, Address );
		v->OpLevel = OpLvlSave;
		
		// set source (and destination??) registers
		if ( ( v->FSrcBitmap.b0 & ~0xfULL ) | v->FSrcBitmap.b1 )
		{
			//e->MovRegImm64 ( RAX, v->FSrcBitmap.b0 );
			//e->MovMemReg64 ( & v->SrcRegs_Bitmap.b0, RAX );
			//e->MovRegImm64 ( RAX, v->FSrcBitmap.b1 );
			//e->MovMemReg64 ( & v->SrcRegs_Bitmap.b1, RAX );
			
			VectorConstants[ CountOfVConsts ].uq0 = v->FSrcBitmap.b0;
			VectorConstants[ CountOfVConsts ].uq1 = v->FSrcBitmap.b1;
			
			e->movdqa_regmem ( RAX, & VectorConstants[ CountOfVConsts ].uq0 );
			
			// inc has to come after the constant is used
			CountOfVConsts++;
			
			e->ptestregmem ( RAX, & v->Pipeline_Bitmap );
			e->Jmp8_E ( 0, 0 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->movdqa_memreg ( & v->SrcRegs_Bitmap.b0, RAX );
			
			e->LeaRegMem64 ( RCX, v );
			e->Call ( (void*) TestStall );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->SetJmpTarget8 ( 0 );
		}

		if ( ( v->ISrcBitmap >> 32 ) & ~1 )
		{
			e->TestMemImm32 ( ( (s32*) (long long*) & v->Int_Pipeline_Bitmap ) + 1, ( v->ISrcBitmap >> 32 ) );
			e->Jmp8_E ( 0, 0 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->MovRegImm64 ( RAX, v->ISrcBitmap );
			e->MovMemReg64 ( (long long*) & v->Int_SrcRegs_Bitmap, RAX );

			e->LeaRegMem64 ( RCX, v );
			e->Call ( (void*) TestStall_INT );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->SetJmpTarget8 ( 0 );

		}
#endif
		
		// recompile the instruction
		
		if ( !isNopLo ( instLO ) )
		{
			retLo = Vu::Recompiler::RecompileLO ( InstanceEncoder, v, instLO, Address );
			
		}
#ifdef VERBOSE_RECOMPILE
		else
		{
cout << " NOP-LO";
		}
#endif
		
		if ( !isNopHi ( instHI ) )
		{
			retHi = Vu::Recompiler::RecompileHI ( InstanceEncoder, v, instHI, Address );
			
		}
#ifdef VERBOSE_RECOMPILE
		else
		{
cout << " NOP-HI";
		}
#endif


#ifdef ENABLE_SETDSTBITMAP

		// set the destination bitmaps

#ifdef RECOMPILE_SETDSTBITMAP

		SetDstBitmap_rec ( e, v, v->FDstBitmap.b0, v->FDstBitmap.b1, v->IDstBitmap );
#else
		
#ifdef RESERVE_STACK_FRAME_FOR_CALL
		e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

		//static void Recompiler::SetDstBitmap ( VU* v, u64 b0, u64 b1, u64 i0 )
		e->LeaRegMem64 ( RCX, v );
		e->MovRegImm64 ( RDX, v->FDstBitmap.b0 );
		e->MovRegImm64 ( R8, v->FDstBitmap.b1 );
		e->MovRegImm64 ( R9, v->IDstBitmap );
		
		e->Call ( (void*) SetDstBitmap );
		
#ifdef RESERVE_STACK_FRAME_FOR_CALL
		ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

#endif

#endif
	}



		

		
#ifdef VERBOSE_RECOMPILE
cout << " retLo=" << retLo;
cout << " retHi=" << retHi;
//cout << " ENC0=" << hex << (((u64*) (pCodeStart [ BlockIndex ])) [ 0 ]);
//cout << " ENC1=" << hex << (((u64*) (pCodeStart [ BlockIndex ])) [ 1 ]);
cout << " ASM-LO: " << Vu::Instruction::Print::PrintInstructionLO ( instLO.Value ).c_str ();
//cout << " IDX-LO: " << dec << Vu::Instruction::Lookup::FindByInstructionLO ( instLO.Value );
cout << " ASM-HI: " << Vu::Instruction::Print::PrintInstructionHI ( instHI.Value ).c_str ();
//cout << " IDX-HI: " << dec << Vu::Instruction::Lookup::FindByInstructionHI ( instHI.Value );
#endif

		//if ( ret <= 0 )
		if ( ( retLo <= 0 ) || ( retHi <= 0 ) )
		{
			// there was a problem, and recompiling is done
			
			// need to undo whatever we did for this instruction
			e->UndoInstructionBlock ();
			Local_NextPCModified = false;
			
//cout << "\nUndo: Address=" << hex << Address;
			
			// TODO: if no instructions have been encoded yet, then just try again with a lower optimization level
			if ( v->OpLevel > 0 )
			{
//cout << "\nNext Op Level down";

				// could not encode the instruction at optimization level, so go down a level and try again
				v->OpLevel--;
				
				//Address -= 4;
				
				// at this point, this should be the last instruction since we had to go down an op level
				// this shouldn't be so, actually
				//MaxCount = 1;
				
				// here we need to reset and redo the instruction
				v->bStopEncodingBefore = false;
				v->bStopEncodingAfter = false;
				Local_NextPCModified = false;
				
				bResetCycleCount = false;
				
				// also backup vector constant generation
				CountOfVConsts = LastVConstCount;
				
				// redo the instruction
				i--;
				continue;
			}
			else
			{
			
				cout << "\nhps2x64: VU#" << v->Number << ": Recompiler: Error: Unable to encode instruction. PC=" << hex << Address << " retLO=" << dec << retLo << " retHI=" << retHi;
				cout << "\n ASM-LO: " << Vu::Instruction::Print::PrintInstructionLO ( instLO.Value ).c_str ();
				cout << "\n ASM-HI: " << Vu::Instruction::Print::PrintInstructionHI ( instHI.Value ).c_str ();
				
				// mark block as unable to recompile if there were no instructions recompiled at all
				//if ( !Cycles ) DoNotCache [ Block ] = 1;
				
				// done
				break;
			}
		}
		
#ifdef ENABLE_SINGLE_STEP_BEFORE
			if ( !v->OpLevel )
			{
				v->bStopEncodingBefore = true;
			}
#endif
		
		
			// if this is not the first instruction, then it can halt encoding before it
			if ( RunCount && v->bStopEncodingBefore )
			{
#ifdef VERBOSE_RECOMPILE
cout << " v->bStopEncodingBefore";
#endif

#ifdef ENCODE_SINGLE_RUN_PER_BLOCK
				// first need to take back the instruction just encoded
				e->UndoInstructionBlock ();

				// check if we are in a new icache block //
				//if ( ! ( Address & 0xf ) )
				//{
				//	// in a new cache block, so must also clear branch to take back the instruction completely
				//	e->BranchOffset [ 64 + ( i >> 2 ) ] = -1;
				//}
				
				
				if ( Status_EBit == 1 )
				{
					//Status.EBitDelaySlot_Valid |= 0x2;
					e->OrMemImm32 ( (long*) & v->Status.ValueHi, 0x2 );
				}

				
#ifdef UPDATE_BEFORE_RETURN
				// run count has not been updated yet for the instruction to stop encoding before
				if ( RunCount > 1 )
				{
					// check that NextPC was not modified
					// doesn't matter here except that RunCount>=1 so it is not first instruction in run, which is handled above
					// next pc was not modified because that will be handled differently now
					//if ( RunCount > 1 && !Local_NextPCModified )
					//{
						// update NextPC
						e->MovMemImm32 ( (long*) & v->NextPC, Address );
						
					//}
					
					// update CPU CycleCount
					// *note* for VU cycle count is already updated
					//e->AddMem64ImmX ( & v->CycleCount, LocalCycleCount - MemCycles );
				}
#endif

#ifdef VERBOSE_RECOMPILE
cout << " RETURN";
#endif

				// return;
				reti &= e->x64EncodeReturn ();


				
				// set the current optimization level
				// note: don't do this here, because the optimization level might have been changed by current instruction
				//v->OpLevel = v->OptimizeLevel;
				
				// reset flags
				v->bStopEncodingBefore = false;
				v->bStopEncodingAfter = false;
				Local_NextPCModified = false;
				
				bResetCycleCount = false;
				
				// starting a new run
				RunCount = 0;
				
				// restart cycle count back to zero
				LocalCycleCount = 0;
				
				
				// also backup vector constant generation
				CountOfVConsts = LastVConstCount;
				
				
				// clear delay slots
				//RDelaySlots [ 0 ].Value = 0;
				//RDelaySlots [ 1 ].Value = 0;
				v->Status_BranchDelay = 0;

				// clear e-bit delay slot
				Status_EBit = 0;
				
				//LocalCycleCount = MemCycles - 1;
				
				// need to redo this instruction at this address
				// since I needed to insert the code to stop the block at this point
				i--;
				continue;
#else

				// do not encode instruction and done encoding
				e->UndoInstructionBlock ();
				Local_NextPCModified = false;
				break;
#endif
			} // end if ( RunCount && v->bStopEncodingBefore )

				
#ifdef ENABLE_EBIT_RECOMPILE
			// put in e-bit delay slot here
			if ( Status_EBit )
			{
				Status_EBit--;
				
				if ( !Status_EBit )
				{
						// check if branch is taken
						e->MovRegMem32 ( RCX, & v->Recompiler_EnableEBitDelay );
						e->Jmp8_ECXZ ( 0, 0 );
						
					e->OrMemImm32 ( (long*) & v->Status.ValueHi, 0x1 );

					if ( RunCount > 1 )
					{
						// there is more than one instruction in run //
						
						// check that NextPC was not modified and that this is not an isolated instruction
						// actually just need to check if NextPC was modified by the encoded instruction
						if ( !Local_NextPCModified )
						{
							// update NextPC
							e->MovMemImm32 ( (long*) & v->NextPC, Address + 8 );
						}
						
					}
					
					// return
					e->Ret ();
					
						e->SetJmpTarget8 ( 0 );
						
					// if unconditional then start new run
					//v->bStopEncodingAfter = true;
				}
			}
#endif


#ifdef ENABLE_BRANCH_DELAY_RECOMPILE
			// put in branch delay slot here
			if ( v->Status_BranchDelay )
			{
				v->Status_BranchDelay--;
				
				if ( !v->Status_BranchDelay )
				{
					// check if conditional branch
					//if ( v->Status_BranchConditional )
					//{
						// check if branch is taken
						e->MovRegMem32 ( RCX, (long*) & v->Recompiler_EnableBranchDelay );
						e->Jmp8_ECXZ ( 0, 0 );
					//}
					
					ProcessBranch ( e, v, v->Status_BranchInstruction, Address );
					
					//if ( v->Status_BranchConditional )
					//{
						e->SetJmpTarget8 ( 0 );
					//}
					//else
					//{
					//	// if unconditional then start new run
					//	v->bStopEncodingAfter = true;
					//}
				}
			}
#endif
			
			// instruction successfully encoded from MIPS into x64
			e->EndInstructionBlock ();
			
//cout << "\nCool: Address=" << hex << Address << " ret=" << dec << ret << " inst=" << hex << *pSrcCodePtr << " i=" << dec << i;

			// update number of instructions that have been recompiled
			RecompileCount++;
			
			// update to next instruction
			//SrcCode += 4;
			// *pInstrPtr++ = *pSrcCodePtr++;
			//pSrcCodePtr++;
			pSrcCodePtr += 2;
			
			// add number of cycles encoded
			Cycles += ret;
			
			// update address
			//Address += 4;
			Address += 8;

			// update instruction count for run
			RunCount++;
			
			// go to next block index
			BlockIndex++;
			
			// update the cycles for run
			LocalCycleCount += MemCycles;


			// reset the optimization level for next instruction
			v->OpLevel = v->OptimizeLevel;
			
			


		
		// if directed to stop encoding after the instruction, then do so
		if ( v->bStopEncodingAfter )
		{
#ifdef VERBOSE_RECOMPILE
cout << " v->bStopEncodingAfter";
#endif

#ifdef ENCODE_SINGLE_RUN_PER_BLOCK

			if ( Status_EBit == 1 )
			{
				//Status.EBitDelaySlot_Valid |= 0x2;
				e->OrMemImm32 ( (long*) & v->Status.ValueHi, 0x2 );
			}


#ifdef UPDATE_BEFORE_RETURN
			// run count has already been updated at this point, but still on instruction#1
			if ( RunCount > 1 )
			{
				// there is more than one instruction in run //
				
				// check that NextPC was not modified and that this is not an isolated instruction
				// actually just need to check if NextPC was modified by the encoded instruction
				if ( !Local_NextPCModified )
				{
					// update NextPC
					e->MovMemImm32 ( (long*) & v->NextPC, Address );
				}
				
				// update CycleCount
				// *note* for VU cycle count is already updated
				//e->AddMem64ImmX ( & v->CycleCount, LocalCycleCount - MemCycles );
			}
#endif

				
#ifdef VERBOSE_RECOMPILE
cout << " RETURN";
#endif

			// return;
			reti &= e->x64EncodeReturn ();


			// set the current optimization level
			v->OpLevel = v->OptimizeLevel;
			
			// reset flags
			v->bStopEncodingBefore = false;
			v->bStopEncodingAfter = false;
			Local_NextPCModified = false;
			
			bResetCycleCount = false;
			
			// clear delay slots
			//RDelaySlots [ 0 ].Value = 0;
			//RDelaySlots [ 1 ].Value = 0;
			v->Status_BranchDelay = 0;
			
			// starting a new run
			RunCount = 0;
			
			// restart cycle count to zero
			LocalCycleCount = 0;


			// clear e-bit delay slot
			Status_EBit = 0;

			
			// cycle counts should start over
			//LocalCycleCount = MemCycles - 1;
			//CacheBlock_CycleCount = 0;
			
#else

				break;
#endif
		} // if ( v->bStopEncodingAfter )
			
			
			
		// reset flags
		v->bStopEncodingBefore = false;
		v->bStopEncodingAfter = false;
		Local_NextPCModified = false;
		
		bResetCycleCount = false;
		

	} // end for ( int i = 0; i < MaxStep; i++, Address += 4 )

#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiler: Done with loop";
#endif




#ifdef ENCODE_SINGLE_RUN_PER_BLOCK
	// at end of block need to return ok //
	
	// encode return if it has not already been encoded at end of block
	if ( RunCount )
	{
	
#ifdef UPDATE_BEFORE_RETURN
		// check that NextPC was not modified and that this is not an isolated instruction
		if ( !Local_NextPCModified )
		{
			// update NextPC
			e->MovMemImm32 ( (long*) & v->NextPC, Address );
		}
		
		// update CycleCount
		// after update need to put in the minus MemCycles
		// *note* for VU cycle count is already updated
		//e->AddMem64ImmX ( & v->CycleCount, LocalCycleCount - MemCycles );
#endif

	} // end if ( RunCount )

#ifdef VERBOSE_RECOMPILE
cout << "\nulCacheLineCount=" << dec << ulCacheLineCount;
#endif


#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiler: Encoding RETURN";
#endif


	// return;
	reti &= e->x64EncodeReturn ();
	
#endif


	
	// done encoding block
	e->EndCodeBlock ();
	
	// address is now encoded
	
	
	if ( !reti )
	{
		cout << "\nRecompiler: Out of space in code block.";
	}

#ifdef VERBOSE_RECOMPILE
//cout << "\n(when all done)TEST0=" << hex << (((u64*) (pCodeStart [ 0x27e4 >> 2 ])) [ 0 ]);
//cout << " TEST1=" << hex << (((u64*) (pCodeStart [ 0x27e4 >> 2 ])) [ 1 ]);
#endif

	
	return reti;
	//return RecompileCount;
}



// code generation //

// generate the instruction prefix to check for any pending events
// will also update NextPC,CycleCount (CPU) and return if there is an event
// will also update CycleCount (System) on a load or store
/*
long Recompiler::Generate_Prefix_EventCheck ( u32 Address, bool bIsBranchOrJump )
{
	long ret;
	
	// get updated CycleCount value for CPU
	e->MovRegMem64 ( RAX, & r->CycleCount );
	e->AddReg64ImmX ( RAX, LocalCycleCount );
	
	
	// want check that there are no events pending //
	
	// get the current cycle count and compare with next event cycle
	// note: actually need to either offset the next event cycle and correct when done or
	// or need to offset the next even cycle into another variable and check against that one
	e->CmpRegMem64 ( RAX, & Playstation1::System::_SYSTEM->NextEvent_Cycle );
	
	// branch if current cycle is greater (or equal?) than next event cycle
	// changing this so that it branches if not returning
	//e->Jmp_A ( 0, 100 + RetJumpCounter++ );
	e->Jmp8_B ( 0, 0 );
	
	// update NextPC
	e->MovMemImm32 ( & r->NextPC, Address );
	
	// update CPU CycleCount
	e->MovMemReg64 ( & r->CycleCount, RAX );
	
	// done for now - return
	ret = e->Ret ();
	
	// jump to here to continue execution in code block
	e->SetJmpTarget8 ( 0 );
	
	// if it is a branch or a jump, then no need to update the System CycleCount
	if ( !bIsBranchOrJump )
	{
		// since we have not reached the next event cycle, should write back the current system cycle
		// so that the correct cycle# gets seen when the store is executed
		// no need to update the CPU cycle count until either a branch/jump is encountered or returning
		// this way, there is no need to reset the current cycle number tally unless a branch/jump is encountered
		ret = e->MovMemReg64 ( & Playstation1::System::_SYSTEM->CycleCount, RAX );
	}

	return ret;
}
*/



long Recompiler::INVALID ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "INVALID";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::INVALID;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ABS ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ABS";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ABS;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ABS
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Ft );
			Add_FDstReg ( v, i.Value, i.Ft );
			break;
#endif

			
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
#ifdef USE_NEW_ABS_RECOMPILE
		case 1:
			Generate_VABSp ( e, v, i );

			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}




long Recompiler::ADD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADD";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADD;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADD
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ADD_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ADDi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADDi";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADDi;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADDi
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ADDi_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i, 0, NULL, &v->vi [ VU::REG_I ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ADDq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADDq";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADDq;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADDq
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
#ifdef USE_NEW_ADDq_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i, 0, NULL, &v->vi [ VU::REG_Q ].u );
			break;
#endif

		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ADDBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADDBCX";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADDBCX;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADDX
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ADDX_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i, 0 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ADDBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADDBCY";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADDBCY;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADDY
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ADDY_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i, 1 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ADDBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADDBCZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADDBCZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADDZ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ADDZ_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i, 2 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ADDBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADDBCW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADDBCW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADDW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ADDW_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i, 3 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ADDA ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADDA";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADDA;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADDA
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ADDA_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i, -1, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ADDAi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADDAi";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADDAi;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADDAi
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ADDAi_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i, 0, &v->dACC[ 0 ].l, &v->vi [ VU::REG_I ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ADDAq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADDAq";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADDAq;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADDAq
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ADDAq_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i, 0, &v->dACC[ 0 ].l, &v->vi [ VU::REG_Q ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ADDABCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADDABCX";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADDABCX;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADDAX
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ADDAX_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i, 0, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ADDABCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADDABCY";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADDABCY;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADDAY
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ADDAY_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i, 1, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ADDABCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADDABCZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADDABCZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADDAZ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ADDAZ_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i, 2, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ADDABCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ADDABCW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ADDABCW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ADDAW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ADDAW_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 0, i, 3, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}










long Recompiler::SUB ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUB";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUB;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUB
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUB_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SUBi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUBi";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUBi;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUBi
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUBi_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i, 0, NULL, &v->vi [ VU::REG_I ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SUBq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUBq";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUBq;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUBq
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUBq_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i, 0, NULL, &v->vi [ VU::REG_Q ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SUBBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUBBCX";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUBBCX;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUBX
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUBX_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i, 0 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SUBBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUBBCY";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUBBCY;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUBY
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUBY_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i, 1 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SUBBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUBBCZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUBBCZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUBZ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUBZ_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i, 2 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SUBBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUBBCW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUBBCW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUBW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUBW_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i, 3 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SUBA ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUBA";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUBA;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUBA
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUBA_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i, -1, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SUBAi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUBAi";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUBAi;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUBAi
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUBAi_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i, 0, &v->dACC[ 0 ].l, &v->vi [ VU::REG_I ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SUBAq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUBAq";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUBAq;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUBAq
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUBAq_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i, 0, &v->dACC[ 0 ].l, &v->vi [ VU::REG_Q ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SUBABCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUBABCX";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUBABCX;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUBAX
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUBAX_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i, 0, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SUBABCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUBABCY";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUBABCY;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUBAY
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUBAY_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i, 1, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SUBABCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUBABCZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUBABCZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUBAZ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUBAZ_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i, 2, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SUBABCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SUBABCW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SUBABCW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SUBAW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SUBAW_RECOMPILE
		case 1:
			ret = Generate_VADDp ( e, v, 1, i, 3, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::MUL ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MUL";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MUL;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MUL
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MUL_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MULi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MULi";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MULi;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MULi
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MULi_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i, 0, NULL, &v->vi [ VU::REG_I ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MULq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MULq";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MULq;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MULq
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MULq_RECOMPILE
		case 1:
		

		
			ret = Generate_VMULp ( e, v, i, 0, NULL, &v->vi [ VU::REG_Q ].u );

			
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MULBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MULBCX";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MULBCX;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MULX
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MULX_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i, 0x00 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MULBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MULBCY";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MULBCY;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MULY
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MULY_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i, 0x55 );	//1 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MULBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MULBCZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MULBCZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MULZ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MULZ_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i, 0xaa );	//2 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MULBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MULBCW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MULBCW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MULW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MULW_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i, 0xff );	//3 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MULA ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MULA";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MULA;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MULA
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MULA_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i, 0x1b, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MULAi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MULAi";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MULAi;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MULAi
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MULAi_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i, 0, &v->dACC[ 0 ].l, &v->vi [ VU::REG_I ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MULAq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MULAq";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MULAq;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MULAq
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MULAq_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i, 0, &v->dACC[ 0 ].l, &v->vi [ VU::REG_Q ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MULABCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MULABCX";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MULABCX;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MULAX
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MULAX_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i, 0x00, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MULABCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MULABCY";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MULABCY;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MULAY
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MULAY_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i, 0x55, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MULABCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MULABCZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MULABCZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MULAZ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MULAZ_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i, 0xaa, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MULABCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MULABCW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MULABCW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MULAW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MULAW_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i, 0xff, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}








long Recompiler::MADD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADD";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADD;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADD
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADD_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 0, i );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MADDi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADDi";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADDi;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADDi
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADDi_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 0, i, 0, NULL, &v->vi [ VU::REG_I ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MADDq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADDq";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADDq;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADDq
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADDq_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 0, i, 0, NULL, &v->vi [ VU::REG_Q ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MADDBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADDBCX";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADDBCX;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADDX
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADDX_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 0, i, 0x00 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MADDBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADDBCY";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADDBCY;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADDY
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADDY_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 0, i, 0x55 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MADDBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADDBCZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADDBCZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADDZ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADDZ_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 0, i, 0xaa );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MADDBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADDBCW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADDBCW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADDW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADDW_RECOMPILE
		case 1:
		

		
			ret =  Generate_VMADDp ( e, v, 0, i, 0xff );
			


			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MADDA ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADDA";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADDA;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADDA
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADDA_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 0, i, 0x1b, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MADDAi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADDAi";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADDAi;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADDAi
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADDAi_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 0, i, 0, &v->dACC[ 0 ].l, &v->vi [ VU::REG_I ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MADDAq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADDAq";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADDAq;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADDAq
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADDAq_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 0, i, 0, &v->dACC[ 0 ].l, &v->vi [ VU::REG_Q ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MADDABCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADDABCX";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADDABCX;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADDAX
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADDAX_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 0, i, 0x00, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MADDABCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADDABCY";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADDABCY;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADDAY
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADDAY_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 0, i, 0x55, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MADDABCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADDABCZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADDABCZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADDAZ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADDAZ_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 0, i, 0xaa, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MADDABCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MADDABCW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MADDABCW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MADDAW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MADDAW_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 0, i, 0xff, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::MSUB ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUB";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUB;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUB
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUB_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MSUBi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUBi";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUBi;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUBi
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUBi_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i, 0, NULL, &v->vi [ VU::REG_I ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MSUBq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUBq";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUBq;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUBq
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUBq_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i, 0, NULL, &v->vi [ VU::REG_Q ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MSUBBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUBBCX";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUBBCX;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUBX
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUBX_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i, 0x00 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MSUBBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUBBCY";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUBBCY;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUBY
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUBY_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i, 0x55 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MSUBBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUBBCZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUBBCZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUBZ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUBZ_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i, 0xaa );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MSUBBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUBBCW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUBBCW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUBW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUBW_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i, 0xff );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MSUBA ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUBA";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUBA;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUBA
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUBA_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i, 0x1b, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MSUBAi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUBAi";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUBAi;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUBAi
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUBAi_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i, 0, &v->dACC[ 0 ].l, &v->vi [ VU::REG_I ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MSUBAq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUBAq";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUBAq;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUBAq
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUBAq_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i, 0, &v->dACC[ 0 ].l, &v->vi [ VU::REG_Q ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MSUBABCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUBABCX";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUBABCX;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUBAX
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUBAX_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i, 0x00, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MSUBABCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUBABCY";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUBABCY;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUBAY
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUBAY_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i, 0x55, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MSUBABCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUBABCZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUBABCZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUBAZ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUBAZ_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i, 0xaa, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MSUBABCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MSUBABCW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MSUBABCW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MSUBAW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MSUBAW_RECOMPILE
		case 1:
			ret =  Generate_VMADDp ( e, v, 1, i, 0xff, &v->dACC[ 0 ].l );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}




long Recompiler::MAX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MAX";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MAX;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MAX
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MAX_RECOMPILE
		case 1:
			ret = Generate_VMAXp ( e, v, i );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MAXi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MAXi";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MAXi;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MAXi
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MAX_RECOMPILE
		case 1:
			ret = Generate_VMAXp ( e, v, i, &v->vi [ VU::REG_I ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::MAXBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MAXBCX";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MAXBCX;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MAXX
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MAX_RECOMPILE
		case 1:
			ret = Generate_VMAXp ( e, v, i, &v->vf [ i.Ft ].uw0 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MAXBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MAXBCY";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MAXBCY;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MAXY
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MAX_RECOMPILE
		case 1:
			ret = Generate_VMAXp ( e, v, i, &v->vf [ i.Ft ].uw1 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MAXBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MAXBCZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MAXBCZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MAXZ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MAX_RECOMPILE
		case 1:
			ret = Generate_VMAXp ( e, v, i, &v->vf [ i.Ft ].uw2 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MAXBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MAXBCW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MAXBCW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MAXW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MAX_RECOMPILE
		case 1:
			ret = Generate_VMAXp ( e, v, i, &v->vf [ i.Ft ].uw3 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}





long Recompiler::MINI ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MINI";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MINI;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MINI
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MIN_RECOMPILE
		case 1:
			ret = Generate_VMINp ( e, v, i );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MINIi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MINIi";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MINIi;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MINIi
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MIN_RECOMPILE
		case 1:
			ret = Generate_VMINp ( e, v, i, &v->vi [ VU::REG_I ].u );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::MINIBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MINIBCX";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MINIBCX;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MINIX
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MIN_RECOMPILE
		case 1:
			ret = Generate_VMINp ( e, v, i, &v->vf [ i.Ft ].uw0 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MINIBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MINIBCY";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MINIBCY;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MINIY
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MIN_RECOMPILE
		case 1:
			ret = Generate_VMINp ( e, v, i, &v->vf [ i.Ft ].uw1 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MINIBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MINIBCZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MINIBCZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MINIZ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MIN_RECOMPILE
		case 1:
			ret = Generate_VMINp ( e, v, i, &v->vf [ i.Ft ].uw2 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MINIBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MINIBCW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MINIBCW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MINIW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcRegBC ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MIN_RECOMPILE
		case 1:
			ret = Generate_VMINp ( e, v, i, &v->vf [ i.Ft ].uw3 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}







long Recompiler::FTOI0 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FTOI0";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FTOI0;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_FTOI0
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Ft );
			Add_FDstReg ( v, i.Value, i.Ft );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FTOI0_RECOMPILE
		case 1:
		
			Generate_VFTOIXp ( e, v, i, 0 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::FTOI4 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FTOI4";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FTOI4;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_FTOI4
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Ft );
			Add_FDstReg ( v, i.Value, i.Ft );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );

#ifdef ENABLE_TEST_DEBUG_FTOI4
			Generate_VFTOIXp_test ( e, v, i, 4 );

			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( Test_FTOI4 );
#endif
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FTOI4_RECOMPILE
		case 1:
			Generate_VFTOIXp ( e, v, i, 4 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::FTOI12 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FTOI12";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FTOI12;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_FTOI12
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Ft );
			Add_FDstReg ( v, i.Value, i.Ft );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FTOI12_RECOMPILE
		case 1:
			Generate_VFTOIXp ( e, v, i, 12 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::FTOI15 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FTOI15";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FTOI15;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_FTOI15
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Ft );
			Add_FDstReg ( v, i.Value, i.Ft );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FTOI15_RECOMPILE
		case 1:
			Generate_VFTOIXp ( e, v, i, 15 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ITOF0 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ITOF0";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ITOF0;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ITOF0
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Ft );
			Add_FDstReg ( v, i.Value, i.Ft );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ITOF0_RECOMPILE
		case 1:
			Generate_VITOFXp ( e, v, i, 0 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ITOF4 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ITOF4";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ITOF4;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ITOF4
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Ft );
			Add_FDstReg ( v, i.Value, i.Ft );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ITOF4_RECOMPILE
		case 1:
			Generate_VITOFXp ( e, v, i, 4 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ITOF12 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ITOF12";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ITOF12;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ITOF12
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Ft );
			Add_FDstReg ( v, i.Value, i.Ft );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ITOF12_RECOMPILE
		case 1:
			Generate_VITOFXp ( e, v, i, 12 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ITOF15 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ITOF15";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ITOF15;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ITOF15
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			//v->Set_DestReg_Upper ( i.Value, i.Ft );
			Add_FDstReg ( v, i.Value, i.Ft );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ITOF15_RECOMPILE
		case 1:
			Generate_VITOFXp ( e, v, i, 15 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::NOP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "NOP";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::NOP;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
#ifdef USE_NEW_NOP_RECOMPILE
		case 1:
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::OPMULA ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "OPMULA";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::OPMULA;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_OPMULA
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_OPMULA_RECOMPILE
		case 1:
			ret = Generate_VMULp ( e, v, i, 0x84, &v->dACC[ 0 ].l, NULL, 0x60 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::OPMSUB ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "OPMSUB";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::OPMSUB;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_OPMSUB
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			//v->Set_DestReg_Upper ( i.Value, i.Fd );
			Add_FDstReg ( v, i.Value, i.Fd );
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_OPMSUB_RECOMPILE
		case 1:
			//static long R5900::Recompiler::Generate_VMADDp ( u32 bSub, R5900::Instruction::Format i, u32 FtComponentp, void *pFd, u32 *pFt, u32 FsComponentp, u32 FdComponentp )
			ret =  Generate_VMADDp ( e, v, 1, i, 0x84, NULL, NULL, 0x60 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::CLIP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "CLIP";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::CLIP;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_CLIP
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Ft );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_CLIP_RECOMPILE
		case 1:
		
			// only set clip flag if not set by lower instruction
			if ( !v->SetClip_Flag )
			{
				// load clip flag
				e->MovRegMem32 ( RAX, &v->vi [ VU::REG_CLIPFLAG ].s );
				
				// flush ps2 float to zero
				e->movdqa_regmem ( RBX, &v->vf [ i.Ft ].sw0 );
				
				if ( !i.Fs )
				{
					e->pxorregreg ( RAX, RAX );
				}
				else if ( i.Fs == i.Ft )
				{
					e->movdqa_regreg ( RAX, RBX );
				}
				else
				{
					e->movdqa_regmem ( RAX, &v->vf [ i.Fs ].sw0 );
				}
				
				// get w from ft
				e->pshufdregregimm ( RBX, RBX, 0xff );
				
				
				// get +w into RBX
				e->pslldregimm ( RBX, 1 );
				e->psrldregimm ( RBX, 1 );
				
				// get -w into RCX
				e->pcmpeqbregreg ( RCX, RCX );
				e->movdqa_regreg ( RDX, RCX );
				e->pxorregreg ( RCX, RBX );
				//e->psubdregreg ( RCX, RDX );
				
				// get x,y from fs into RDX
				e->pshufdregregimm ( RDX, RAX, ( 1 << 6 ) | ( 1 << 4 ) | ( 0 << 2 ) | ( 0 << 0 ) );
				e->movdqa_regreg ( 4, RDX );
				e->psradregimm ( 4, 31 );
				//e->pslldregimm ( RDX, 1 );
				//e->psrldregimm ( RDX, 1 );
				e->psrldregimm ( 4, 1 );
				e->pxorregreg ( RDX, 4 );
				//e->psubdregreg ( RDX, 4 );
				
				// get greater than +w into R4 and less than -w into R5
				e->movdqa_regreg ( 4, RDX );
				e->pcmpgtdregreg ( 4, RBX );
				e->movdqa_regreg ( 5, RCX );
				e->pcmpgtdregreg ( 5, RDX );
				
				// get x and y flags into R4
				e->pblendwregregimm ( 4, 5, 0xcc );
				
				
				// get z from fs into RAX
				e->pshufdregregimm ( RAX, RAX, ( 2 << 6 ) | ( 2 << 4 ) | ( 2 << 2 ) | ( 2 << 0 ) );
				e->movdqa_regreg ( 5, RAX );
				e->psradregimm ( 5, 31 );
				//e->pslldregimm ( RAX, 1 );
				//e->psrldregimm ( RAX, 1 );
				e->psrldregimm ( 5, 1 );
				e->pxorregreg ( RAX, 5 );
				//e->psubdregreg ( RAX, 5 );
				
				// get greater than into RAX and less than into RCX
				e->pcmpgtdregreg ( RCX, RAX );
				e->pcmpgtdregreg ( RAX, RBX );
				
				// get z flags into RAX
				e->pblendwregregimm ( RAX, RCX, 0xcc );
				
				// pull flags
				e->movmskpsregreg ( RCX, 4 );
				e->movmskpsregreg ( RDX, RAX );
				
				// combine flags
				e->ShlRegImm32 ( RDX, 4 );
				e->OrRegReg32 ( RCX, RDX );
				e->AndReg32ImmX ( RCX, 0x3f );
				
				// combine into rest of the clipping flags
				e->ShlRegImm32 ( RAX, 6 );
				e->OrRegReg32 ( RAX, RCX );
				e->AndReg32ImmX ( RAX, 0x00ffffff );
				
				// write back to clipping flag
				e->MovMemReg32 ( &v->vi [ VU::REG_CLIPFLAG ].s, RAX );
				
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}





// lower instructions

long Recompiler::DIV ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "DIV";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::DIV;
	
	static const u64 c_CycleTime = 7;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_DIV
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegBC ( i.ftf, i.Ft );
			//v->Add_SrcRegBC ( i.fsf, i.Fs );
			Add_FSrcRegBC ( v, i.fsf, i.Fs );
			Add_FSrcRegBC ( v, i.ftf, i.Ft );
			
			break;
#endif

		case 0:
			// ***testing***
			//v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

			// ***TODO*** when div affects flags it should also affect all snapshots of the flags too
			// ***TODO*** at what point does div affect flag register? immediately or later ?
#ifdef USE_NEW_DIV_RECOMPILE
		case 1:
		
			// check if QBusyUntil_Cycle is -1
			e->MovRegMem64 ( RAX, (long long*) & v->QBusyUntil_Cycle );
			e->CmpReg64ImmX ( RAX, -1 );
			e->Jmp8_E ( 0, 0 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LeaRegMem64 ( RCX, v );
			ret = e->Call ( (void*) PipelineWaitQ );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->SetJmpTarget8 ( 0 );



			
			// now can do the DIV //
			
			// clear bits 16 and 17 in the flag register first
			//e->AndMem32ImmX ( &VU0::_VU0->vi [ VU::REG_STATUSFLAG ].u, ~0x00000030 );		// r->CPC1 [ 31 ], ~0x00030000 );
			
			// flush ps2 float to zero
			e->MovRegMem32 ( RAX, &v->vf [ i.Ft ].vsw [ i.ftf ] );
			e->XorRegReg32 ( 11, 11 );
			e->MovReg32ImmX ( 8, 0x00000c30 );
			e->MovReg64ImmX ( RCX, 896ULL << 23 );
			e->Cdq ();
			e->AndReg32ImmX ( RAX, 0x7fffffff );
			//e->LeaRegRegReg64 ( RDX, RAX, RCX );
			e->AddRegReg64 ( RCX, RAX );
			//e->TestReg32ImmX ( RAX, 0x7f800000 );
			e->AndReg32ImmX ( RAX, 0x7f800000 );
			e->CmovNERegReg32 ( 8, 11 );
			e->CmovNERegReg64 ( RAX, RCX );
			e->ShlRegImm64 ( RAX, 29 );
			e->movq_to_sse ( RCX, RAX );
			
			
			e->MovRegMem32 ( RAX, &v->vf [ i.Fs ].vsw [ i.fsf ] );
			//e->MovReg64ImmX ( RCX, 896ULL << 23 );
			//e->XorRegReg32 ( 8, RAX );
			e->XorRegReg32 ( RDX, RAX );
			e->AndReg32ImmX ( RAX, 0x7fffffff );
			//e->LeaRegRegReg64 ( RDX, RAX, RCX );
			//e->AddRegReg64 ( RCX, RAX );
			//e->AndReg32ImmX ( RAX, 0x7f800000 );
			e->TestReg32ImmX ( RAX, 0x7f800000 );
			e->MovReg32ImmX ( 9, 0x00000820 );
			e->MovReg32ImmX ( 10, 0x00000410 );
			e->CmovERegReg32 ( 9, 10 );
			e->CmovERegReg32 ( RAX, 11 );
			e->ShlRegImm64 ( RAX, 29 );
			e->movq_to_sse ( RAX, RAX );

			
			// get sign in R8
			e->AndReg32ImmX ( RDX, 0x80000000 );
			
			// set flags
			e->AndRegReg32 ( 8, 9 );
			//e->OrMemReg32 ( &VU0::_VU0->vi [ VU::REG_STATUSFLAG ].u, 8 );
			e->MovMemReg16 ( (short*) &v->NextQ_Flag, 8 );
			
			// perform div
			e->divsd ( RAX, RCX );


			// get result
			e->movq_from_sse ( RAX, RAX );

			
			// shift back down without sign
			e->ShrRegImm64 ( RAX, 29 );
			
			
			// clear on underflow or zero
			e->TestReg32ImmX ( RAX, 0xff800000 );
			e->CmovERegReg32 ( RAX, 11 );
			
			
			// set to max on overflow
			e->MovReg32ImmX ( RCX, 0x7fffffff );
			e->CmovSRegReg32 ( RAX, RCX );
			
			// or if any flags are set
			e->OrRegReg32 ( 8, 8 );
			e->CmovNERegReg32 ( RAX, RCX );
			
			// set sign
			e->OrRegReg32 ( RAX, RDX );
			

			// store result
			//e->MovMemReg32 ( &VU0::_VU0->vi [ VU::REG_Q ].u, RAX );		// &r->CPR1 [ i.Fd ].u, RAX );
			e->MovMemReg32 ( &v->NextQ.l, RAX );
			
			// set time to process
			e->MovRegMem64 ( RAX, (long long*) & v->CycleCount );
			e->AddReg64ImmX ( RAX, c_CycleTime );
			e->MovMemReg64 ( (long long*) & v->QBusyUntil_Cycle, RAX );

			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::IADD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "IADD";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::IADD;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_IADD
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcRegs ( ( i.is & 0xf ) + 32, ( i.it & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.it & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// delay slot issue
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_IADD_RECOMPILE
		case 1:
			// check for a conditional branch that might be affected by integer destination register
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.id & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.id & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}
			
			if ( i.id & 0xf )
			{
				if ( ( !( i.is & 0xf ) ) && ( !( i.it & 0xf ) ) )
				{
					e->MovMemImm16 ( (s16*) &v->vi [ i.id & 0xf ].u, 0 );
				}
				else if ( ( !( i.is & 0xf ) ) || ( !( i.it & 0xf ) ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ ( i.is & 0xf ) + ( i.it & 0xf ) ].u );
					e->MovMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else if ( ( i.id & 0xf ) == ( i.is & 0xf ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.it & 0xf ].s );
					e->AddMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else if ( ( i.id & 0xf ) == ( i.it & 0xf ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.is & 0xf ].s );
					e->AddMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else if ( ( i.is & 0xf ) == ( i.it & 0xf ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.is & 0xf ].s );
					e->AddRegReg16 ( RAX, RAX );
					
					e->MovMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.is & 0xf ].s );
					e->AddRegMem16 ( RAX, (s16*) &v->vi [ i.it & 0xf ].s );
					e->MovMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
			}
			
			break;
#endif

			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::IADDI ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "IADDI";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::IADDI;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_IADDI
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// delay slot issue
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_IADDI_RECOMPILE
		case 1:
			// check for a conditional branch that might be affected by integer destination register
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.it & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.it & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}
			
			if ( i.it & 0xf )
			{
				
				if ( !( i.is & 0xf ) )
				{
					e->MovMemImm16 ( (s16*) &v->vi [ i.it & 0xf ].s, ( (s16) i.Imm5 ) );
				}
				else if ( i.it == i.is )
				{
					e->AddMemImm16 ( (s16*) &v->vi [ i.it & 0xf ].s, ( (s16) i.Imm5 ) );
				}
				else
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.is & 0xf ].s );
					e->AddRegImm16 ( RAX, ( (s16) i.Imm5 ) );
					e->MovMemReg16 ( (s16*) &v->vi [ i.it & 0xf ].s, RAX );
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::IAND ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "IAND";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::IAND;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_IAND
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcRegs ( ( i.is & 0xf ) + 32, ( i.it & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.it & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// delay slot issue
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_IAND_RECOMPILE
		case 1:
			// check for a conditional branch that might be affected by integer destination register
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.id & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.id & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}
			
			if ( i.id & 0xf )
			{
				if ( ( !( i.is & 0xf ) ) || ( !( i.it & 0xf ) ) )
				{
					e->MovMemImm16 ( (s16*) &v->vi [ i.id & 0xf ].u, 0 );
				}
				else if ( ( i.is & 0xf ) == ( i.it & 0xf ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.is & 0xf ].s );
					e->MovMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else if ( ( i.id & 0xf ) == ( i.is & 0xf ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.it & 0xf ].s );
					e->AndMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else if ( ( i.id & 0xf ) == ( i.it & 0xf ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.is & 0xf ].s );
					e->AndMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.is & 0xf ].s );
					e->AndRegMem16 ( RAX, (s16*) &v->vi [ i.it & 0xf ].s );
					e->MovMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::IOR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "IOR";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::IOR;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_IOR
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcRegs ( ( i.is & 0xf ) + 32, ( i.it & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.it & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// delay slot issue
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_IOR_RECOMPILE
		case 1:
			// check for a conditional branch that might be affected by integer destination register
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.id & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.id & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}
			
			if ( i.id & 0xf )
			{
				if ( ( !( i.is & 0xf ) ) && ( !( i.it & 0xf ) ) )
				{
					e->MovMemImm16 ( (s16*) &v->vi [ i.id & 0xf ].u, 0 );
				}
				else if ( ( !( i.is & 0xf ) ) || ( !( i.it & 0xf ) ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ ( i.is & 0xf ) + ( i.it & 0xf ) ].u );
					e->MovMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else if ( ( i.is & 0xf ) == ( i.it & 0xf ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.is & 0xf ].s );
					e->MovMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else if ( ( i.id & 0xf ) == ( i.is & 0xf ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.it & 0xf ].s );
					e->OrMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else if ( ( i.id & 0xf ) == ( i.it & 0xf ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.is & 0xf ].s );
					e->OrMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.is & 0xf ].s );
					e->OrRegMem16 ( RAX, (s16*) &v->vi [ i.it & 0xf ].s );
					e->MovMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ISUB ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ISUB";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ISUB;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ISUB
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcRegs ( ( i.is & 0xf ) + 32, ( i.it & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.it & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// delay slot issue
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ISUB_RECOMPILE
		case 1:
			// check for a conditional branch that might be affected by integer destination register
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.id & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.id & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}
			
			if ( i.id & 0xf )
			{
				if ( ( !( i.is & 0xf ) ) && ( !( i.it & 0xf ) ) )
				{
					e->MovMemImm16 ( (s16*) &v->vi [ i.id & 0xf ].u, 0 );
				}
				else if ( ( i.is & 0xf ) == ( i.it & 0xf ) )
				{
					e->MovMemImm16 ( (s16*) &v->vi [ i.id & 0xf ].u, 0 );
				}
				else if ( !( i.it & 0xf ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.is & 0xf ].s );
					e->MovMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else if ( !( i.is & 0xf ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.it & 0xf ].s );
					e->NegReg16 ( RAX );
					e->MovMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else if ( ( i.id & 0xf ) == ( i.is & 0xf ) )
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.it & 0xf ].s );
					e->SubMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
				else
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.is & 0xf ].s );
					e->SubRegMem16 ( RAX, (s16*) &v->vi [ i.it & 0xf ].s );
					
					e->MovMemReg16 ( (s16*) &v->vi [ i.id & 0xf ].u, RAX );
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}


long Recompiler::ISUBIU ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ISUBIU";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ISUBIU;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ISUBIU
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// integer math at level 0 has delay slot
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ISUBIU_RECOMPILE
		case 1:
			// check for a conditional branch that might be affected by integer destination register
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.it & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.it & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}

			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}
			
			if ( i.it & 0xf )
			{
				
				if ( !( i.is & 0xf ) )
				{
					e->MovMemImm16 ( (s16*) &v->vi [ i.it & 0xf ].s, -( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) );
				}
				else if ( i.it == i.is )
				{
					e->SubMemImm16 ( (s16*) &v->vi [ i.it & 0xf ].s, ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) );
				}
				else
				{
					e->MovRegMem16 ( RAX, (s16*) &v->vi [ i.is & 0xf ].s );
					e->SubRegImm16 ( RAX, ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) );
					e->MovMemReg16 ( (s16*) &v->vi [ i.it & 0xf ].s, RAX );
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}




long Recompiler::ILWR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ILWR";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ILWR;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ILWR
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
	
			if ( i.it & 0xf )
			{
				//v->Set_DestReg_Lower ( ( i.it & 0xf ) + 32 );
				Add_IDstReg ( v, ( i.it & 0xf ) + 32 );
			}
			
			break;
#endif

		case 0:
			// load at level 0 has delay slot
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ILWR_RECOMPILE
		case 1:
		
#ifdef DISABLE_ILWR_VU0
			// not doing VU#0 for now since it has more involved with load/store
			if ( !v->Number )
			{
				return -1;
			}
#endif
		
			// check for a conditional branch that might be affected by integer destination register
			/*
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.it & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.it & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}
			*/
			
			if ( i.it )
			{
				//LoadAddress = ( v->vi [ i.is & 0xf ].sLo + i.Imm11 ) << 2;
				e->MovRegMem32 ( RAX, & v->vi [ i.is & 0xf ].s );
				
				
				//pVuMem32 = v->GetMemPtr ( LoadAddress );
				//return & ( VuMem32 [ Address32 & ( c_ulVuMem1_Mask >> 2 ) ] );
				//e->MovRegImm64 ( RCX, (u64) & v->VuMem32 [ 0 ] );
				e->LeaRegMem64 ( RCX, & v->VuMem32 [ 0 ] );

				// special code for VU#0
				if ( !v->Number )
				{
					// check if Address & 0xf000 == 0x4000
					e->MovRegReg32 ( RDX, RAX );
					e->AndReg32ImmX ( RDX, 0xf000 >> 4 );
					e->CmpReg32ImmX ( RDX, 0x4000 >> 4 );
					
					// here it will be loading/storing from/to the registers for VU#1
					e->LeaRegMem64 ( RDX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
					e->CmovERegReg64 ( RCX, RDX );
					
					// ***TODO*** check if storing to TPC
				}
				
				if ( !v->Number )
				{
				e->AndReg32ImmX ( RAX, VU::c_ulVuMem0_Mask >> 4 );
				}
				else
				{
				e->AndReg32ImmX ( RAX, VU::c_ulVuMem1_Mask >> 4 );
				}
				e->AddRegReg32 ( RAX, RAX );
				
				switch( i.xyzw )
				{
					case 8:
						e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
						break;
						
					case 4:
						e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 4 );
						break;
						
					case 2:
						e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 8 );
						break;
						
					case 1:
						e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 12 );
						break;
						
					default:
						cout << "\nVU: Recompiler: ALERT: ILWR with illegal xyzw=" << hex << i.xyzw << "\n";
						break;
				}
				
				//if ( i.xyzw != 0xf )
				//{
				//	e->pblendwregregimm ( RAX, RCX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
				//}
				//ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RAX );
				
				ret = e->MovMemReg32 ( & v->vi [ i.it & 0xf ].s, RAX );
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ISWR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ISWR";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ISWR;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ISWR
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.it & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// ***testing***
			//v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ISWR_RECOMPILE
		case 1:
		
#ifdef DISABLE_ISWR_VU0
			// not doing VU#0 for now since it has more involved with load/store
			if ( !v->Number )
			{
				return -1;
			}
#endif
		
				
			//LoadAddress = ( v->vi [ i.is & 0xf ].sLo + i.Imm11 ) << 2;
			e->MovRegMem32 ( RAX, & v->vi [ i.is & 0xf ].s );
			e->movd_regmem ( RAX, & v->vi [ i.it & 0xf ].s );
			
			//e->movdqa_regmem ( RAX, & v->vf [ i.Fs ].sw0 );
			
			//pVuMem32 = v->GetMemPtr ( LoadAddress );
			//return & ( VuMem32 [ Address32 & ( c_ulVuMem1_Mask >> 2 ) ] );
			//e->MovRegImm64 ( RCX, (u64) & v->VuMem32 [ 0 ] );
			e->LeaRegMem64 ( RCX, & v->VuMem32 [ 0 ] );

			// special code for VU#0
			if ( !v->Number )
			{
#ifdef ALL_VU0_UPPER_ADDRS_ACCESS_VU1
				e->AndReg32ImmX ( RAX, 0x7fff );
#endif

				// check if Address & 0xf000 == 0x4000
				e->MovRegReg32 ( RDX, RAX );
				e->AndReg32ImmX ( RDX, 0xf000 >> 4 );
				e->CmpReg32ImmX ( RDX, 0x4000 >> 4 );
				e->Jmp8_NE ( 0, 1 );
				
				// here it will be loading/storing from/to the registers for VU#1
				//e->LeaRegMem64 ( RDX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
				//e->CmovERegReg64 ( RCX, RDX );
				e->LeaRegMem64 ( RCX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
				
				// ***TODO*** check if storing to TPC
				e->CmpReg32ImmX ( RAX, 0x43a );
				e->Jmp8_NE ( 0, 0 );
				
				//VU1::_VU1->Running = 1;
				e->MovMemImm32 ( (long*) & VU1::_VU1->Running, 1 );
				
				//VU1::_VU1->CycleCount = *VU::_DebugCycleCount + 1;
				e->MovRegMem64 ( RDX, (long long*) VU::_DebugCycleCount );
				
				// set VBSx in VPU STAT to 1 (running)
				//VU0::_VU0->vi [ 29 ].uLo |= ( 1 << ( 1 << 3 ) );
				e->OrMemImm32 ( & VU0::_VU0->vi [ 29 ].s, ( 1 << ( 1 << 3 ) ) );
				
				// also set VIFx STAT to indicate program is running
				//VU1::_VU1->VifRegs.STAT.VEW = 1;
				e->OrMemImm32 ( (long*) & VU1::_VU1->VifRegs.STAT.Value, ( 1 << 2 ) );
				
				// finish handling the new cycle count
				e->IncReg64 ( RDX );
				e->MovMemReg64 ( (long long*) & VU1::_VU1->CycleCount, RDX );
				
				e->SetJmpTarget8 ( 0 );
				
				// mask address for accessing registers
				e->AndReg32ImmX ( RAX, 0x3f );
				
				e->SetJmpTarget8 ( 1 );
			}
			
			if ( !v->Number )
			{
			e->AndReg32ImmX ( RAX, VU::c_ulVuMem0_Mask >> 4 );
			}
			else
			{
			e->AndReg32ImmX ( RAX, VU::c_ulVuMem1_Mask >> 4 );
			}
			e->AddRegReg32 ( RAX, RAX );

			if ( i.xyzw != 0xf )
			{
				e->movdqa_from_mem128 ( RCX, RCX, RAX, SCALE_EIGHT, 0 );
			}
			
			e->pmovzxwdregreg ( RAX, RAX );
			e->pshufdregregimm ( RAX, RAX, 0 );
			//e->pslldregimm ( RAX, 16 );
			//e->psrldregimm ( RAX, 16 );
			
			if ( i.xyzw != 0xf )
			{
				//e->movdqa_from_mem128 ( RCX, RCX, RAX, SCALE_EIGHT, 0 );
				e->pblendwregregimm ( RAX, RCX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			}
			
			//ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RAX );
			ret = e->movdqa_to_mem128 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::LQD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "LQD";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::LQD;
	
	int ret = 1;
	VU::Bitmap128 bmTemp;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_LQD
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			
#ifdef ENABLE_DSTBITMAP_BEFORE_DELAYSLOT
			// destination for move instruction needs to be set only if move is made
			Add_FDstReg ( v, i.Value, i.Ft );
#endif
			
			break;
#endif

		case 0:
			// load at level 0 has delay slot
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_LQD_RECOMPILE
		case 1:
		
#ifdef DISABLE_LQD_VU0
			// not doing VU#0 for now since it has more involved with load/store
			if ( !v->Number )
			{
				return -1;
			}
#endif
		
#ifdef ENABLE_INTDELAYSLOT_LQD
			// check for a conditional branch that might be affected by integer destination register
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.is & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.is & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}
#endif

			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}
			
			// check if instruction should be cancelled (if it writes to same reg as upper instruction)
			if ( !( ( 1 << i.Ft ) & v->IDstBitmap ) )
			{
				// check if destination for move is source for upper instruction
				VU::ClearBitmap ( bmTemp );
				VU::AddBitmap ( bmTemp, i.xyzw, i.Ft );
				if ( !VU::TestBitmap ( bmTemp, v->FSrcBitmap ) )
				{
					if ( i.Ft )
					{
						// add destination register to bitmap at end
						Add_FDstReg ( v, i.Value, i.Ft );
						
						//LoadAddress = ( v->vi [ i.is & 0xf ].sLo + i.Imm11 ) << 2;
						e->MovRegMem32 ( RAX, & v->vi [ i.is & 0xf ].s );
						
						
						//if ( i.xyzw != 0xf )
						//{
						//	e->movdqa_regmem ( RCX, & v->vf [ i.Ft ].sw0 );
						//}
						
						//pVuMem32 = v->GetMemPtr ( LoadAddress );
						//return & ( VuMem32 [ Address32 & ( c_ulVuMem1_Mask >> 2 ) ] );
						//e->MovRegImm64 ( RCX, (u64) & v->VuMem32 [ 0 ] );
						e->LeaRegMem64 ( RCX, & v->VuMem32 [ 0 ] );
						
						// special code for VU#0
						if ( !v->Number )
						{
							// check if Address & 0xf000 == 0x4000
							e->MovRegReg32 ( RDX, RAX );
							e->AndReg32ImmX ( RDX, 0xf000 >> 4 );
							e->CmpReg32ImmX ( RDX, 0x4000 >> 4 );
							
							// here it will be loading/storing from/to the registers for VU#1
							e->LeaRegMem64 ( RDX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
							e->CmovERegReg64 ( RCX, RDX );
							
						}
						
						
						e->DecReg16 ( RAX );
						
						// post-inc
						if ( i.is & 0xf )
						{
							e->MovMemReg16 ( & v->vi [ i.is & 0xf ].sLo, RAX );
						}
						
						if ( !v->Number )
						{
							e->AndReg32ImmX ( RAX, VU::c_ulVuMem0_Mask >> 4 );
						}
						else
						{
							e->AndReg32ImmX ( RAX, VU::c_ulVuMem1_Mask >> 4 );
						}
						e->AddRegReg32 ( RAX, RAX );
						
						//e->MovRegFromMem32 ( RDX, RCX, RAX, SCALE_EIGHT, 0 );
						e->movdqa_from_mem128 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
						
						if ( i.xyzw != 0xf )
						{
							//e->pblendwregregimm ( RAX, RCX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
							e->pblendwregmemimm ( RAX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
						}
						
						ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RAX );
					}
					else
					{
						if ( i.is & 0xf )
						{
						e->MovRegMem32 ( RAX, & v->vi [ i.is & 0xf ].s );
						e->DecReg16 ( RAX );
						e->MovMemReg16 ( & v->vi [ i.is & 0xf ].sLo, RAX );
						}
					}
				}
				else
				{
					return -1;
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::LQI ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "LQI";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::LQI;
	
	int ret = 1;
	VU::Bitmap128 bmTemp;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_LQI
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			
#ifdef ENABLE_DSTBITMAP_BEFORE_DELAYSLOT
			// destination for move instruction needs to be set only if move is made
			Add_FDstReg ( v, i.Value, i.Ft );
#endif
			
			break;
#endif

		case 0:
			// load at level 0 has delay slot
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_LQI_RECOMPILE
		case 1:
		
#ifdef DISABLE_LQI_VU0
			// not doing VU#0 for now since it has more involved with load/store
			if ( !v->Number )
			{
				return -1;
			}
#endif
		
#ifdef ENABLE_INTDELAYSLOT_LQI
			// check for a conditional branch that might be affected by integer destination register
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.is & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.is & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}
#endif

			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}
			
			// check if instruction should be cancelled (if it writes to same reg as upper instruction)
			if ( !( ( 1 << i.Ft ) & v->IDstBitmap ) )
			{
				// check if destination for move is source for upper instruction
				VU::ClearBitmap ( bmTemp );
				VU::AddBitmap ( bmTemp, i.xyzw, i.Ft );
				if ( !VU::TestBitmap ( bmTemp, v->FSrcBitmap ) )
				{
					if ( i.Ft )
					{
						// add destination register to bitmap at end
						Add_FDstReg ( v, i.Value, i.Ft );
						
						//LoadAddress = ( v->vi [ i.is & 0xf ].sLo + i.Imm11 ) << 2;
						e->MovRegMem32 ( RAX, & v->vi [ i.is & 0xf ].s );
						
						
						//if ( i.xyzw != 0xf )
						//{
						//	e->movdqa_regmem ( RCX, & v->vf [ i.Ft ].sw0 );
						//}

						//pVuMem32 = v->GetMemPtr ( LoadAddress );
						//return & ( VuMem32 [ Address32 & ( c_ulVuMem1_Mask >> 2 ) ] );
						//e->MovRegImm64 ( RCX, (u64) & v->VuMem32 [ 0 ] );
						e->LeaRegMem64 ( RCX, & v->VuMem32 [ 0 ] );
						
						// special code for VU#0
						if ( !v->Number )
						{
							// check if Address & 0xf000 == 0x4000
							e->MovRegReg32 ( RDX, RAX );
							e->AndReg32ImmX ( RDX, 0xf000 >> 4 );
							e->CmpReg32ImmX ( RDX, 0x4000 >> 4 );
							
							// here it will be loading/storing from/to the registers for VU#1
							e->LeaRegMem64 ( RDX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
							e->CmovERegReg64 ( RCX, RDX );
							
						}

						
						// post-inc
						if ( i.is & 0xf )
						{
						e->LeaRegRegImm32 ( RDX, RAX, 1 );
						e->MovMemReg16 ( & v->vi [ i.is & 0xf ].sLo, RDX );
						}
						
						if ( !v->Number )
						{
							e->AndReg32ImmX ( RAX, VU::c_ulVuMem0_Mask >> 4 );
						}
						else
						{
							e->AndReg32ImmX ( RAX, VU::c_ulVuMem1_Mask >> 4 );
						}
						e->AddRegReg32 ( RAX, RAX );
						
						//e->MovRegFromMem32 ( RDX, RCX, RAX, SCALE_EIGHT, 0 );
						e->movdqa_from_mem128 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
						
						if ( i.xyzw != 0xf )
						{
							//e->pblendwregregimm ( RAX, RCX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
							e->pblendwregmemimm ( RAX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
						}
						
						ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RAX );
					}
					else
					{
						if ( i.is & 0xf )
						{
						e->MovRegMem32 ( RAX, & v->vi [ i.is & 0xf ].s );
						e->IncReg32 ( RAX );
						e->MovMemReg16 ( & v->vi [ i.is & 0xf ].sLo, RAX );
						}
					}
				}
				else
				{
					return -1;
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}







long Recompiler::MFIR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MFIR";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MFIR;
	
	int ret = 1;
	VU::Bitmap128 bmTemp;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MFIR
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			
#ifdef ENABLE_DSTBITMAP_BEFORE_DELAYSLOT
			// destination for move instruction needs to be set only if move is made
			Add_FDstReg ( v, i.Value, i.Ft );
#endif
			
			break;
#endif

		case 0:
			// delay slot issue
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;


#ifdef USE_NEW_MFIR_RECOMPILE
		case 1:
			// check if instruction should be cancelled (if it writes to same reg as upper instruction)
			if ( !( ( 1 << i.Ft ) & v->IDstBitmap ) )
			{
				// check if destination for move is source for upper instruction
				VU::ClearBitmap ( bmTemp );
				VU::AddBitmap ( bmTemp, i.xyzw, i.Ft );
				if ( !VU::TestBitmap ( bmTemp, v->FSrcBitmap ) )
				{
					// add destination register to bitmap at end
					Add_FDstReg ( v, i.Value, i.Ft );
					
					ret = Generate_VMFIRp ( e, v, i );
				}
				else
				{
					return -1;
				}
			}
			
			break;
#endif


			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MTIR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MTIR";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MTIR;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MTIR
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_FSrcRegBC ( v, i.fsf, i.Fs );
			
			//Add_IDstReg ( v, ( i.it & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// integer register destination
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MTIR_RECOMPILE
		case 1:
			// check for a conditional branch that might be affected by integer destination register
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.it & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.it & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}

			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}


			ret = Generate_VMTIRp ( e, v, i );
			

			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MOVE ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MOVE";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MOVE;
	
	int ret = 1;
	VU::Bitmap128 bmTemp;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MOVE
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
#ifdef ENABLE_DSTBITMAP_BEFORE_DELAYSLOT
			// destination for move instruction needs to be set only if move is made
			//v->Set_DestReg_Upper ( i.Value, i.Ft );
			Add_FDstReg ( v, i.Value, i.Ft );
#endif

			break;
#endif

		case 0:
			// delay slot issue
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MOVE_RECOMPILE
		case 1:
			// check if instruction should be cancelled (if it writes to same reg as upper instruction)
			if ( !( ( 1 << i.Ft ) & v->IDstBitmap ) )
			{
				// check if destination for move is source for upper instruction
				// todo: what if destination and source for move are the same ?? //
				VU::ClearBitmap ( bmTemp );
				VU::AddBitmap ( bmTemp, i.xyzw, i.Ft );
				if ( !VU::TestBitmap ( bmTemp, v->FSrcBitmap ) )
				{
					// add destination register to bitmap at end
					Add_FDstReg ( v, i.Value, i.Ft );
					
					ret = Generate_VMOVEp ( e, v, i );
				}
				else
				{
					return -1;
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MR32 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MR32";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MR32;
	
	int ret = 1;
	VU::Bitmap128 bmTemp;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_MR32
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcReg ( i.Value, i.Fs );
			//Add_FSrcReg ( v, ( ( i.Value << 1 ) & ( 0xe << 21 ) ) | ( ( i.Value >> 3 ) & ( 1 << 21 ) ), i.Fs );
			Add_FSrcReg ( v, ( ( i.Value >> 1 ) & ( 0x7 << 21 ) ) | ( ( i.Value << 3 ) & ( 0x8 << 21 ) ), i.Fs );
			
#ifdef ENABLE_DSTBITMAP_BEFORE_DELAYSLOT
			// destination for move instruction needs to be set only if move is made
			//v->Set_DestReg_Upper ( i.Value, i.Ft );
			Add_FDstReg ( v, i.Value, i.Ft );
#endif

			break;
#endif

		case 0:
			// delay slot issue
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;


#ifdef USE_NEW_MR32_RECOMPILE
		case 1:
			// check if instruction should be cancelled (if it writes to same reg as upper instruction)
			if ( !( ( 1 << i.Ft ) & v->IDstBitmap ) )
			{
				// check if destination for move is source for upper instruction
				// todo: what if destination and source for move are the same ?? //
				VU::ClearBitmap ( bmTemp );
				VU::AddBitmap ( bmTemp, i.xyzw, i.Ft );
				if ( !VU::TestBitmap ( bmTemp, v->FSrcBitmap ) )
				{
					// add destination register to bitmap at end
					Add_FDstReg ( v, i.Value, i.Ft );
					
					ret = Generate_VMR32p ( e, v, i );
				}
				else
				{
					return -1;
				}
			}
			break;
#endif

			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::RGET ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "RGET";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::RGET;
	
	int ret = 1;
	VU::Bitmap128 bmTemp;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_RGET
		// get source and destination register(s) bitmap
		case -1:
		
#ifdef ENABLE_DSTBITMAP_BEFORE_DELAYSLOT
			// destination for move instruction needs to be set only if move is made
			//v->Set_DestReg_Upper ( i.Value, i.Ft );
			Add_FDstReg ( v, i.Value, i.Ft );
#endif
			
			break;
#endif

		case 0:
			// delay slot issue
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
#ifdef USE_NEW_RGET_RECOMPILE
		case 1:
			// check if instruction should be cancelled (if it writes to same reg as upper instruction)
			if ( !( ( 1 << i.Ft ) & v->IDstBitmap ) )
			{
				// check if destination for move is source for upper instruction
				VU::ClearBitmap ( bmTemp );
				VU::AddBitmap ( bmTemp, i.xyzw, i.Ft );
				if ( !VU::TestBitmap ( bmTemp, v->FSrcBitmap ) )
				{
					// add destination register to bitmap at end
					Add_FDstReg ( v, i.Value, i.Ft );
					
					//ret = Generate_VMOVEp ( v, i );
					if ( i.Ft && i.xyzw )
					{
						//e->MovRegMem64 ( RAX, (long long*) & v->CycleCount );
						//e->CmpRegMem64 ( RAX, (long long*) & v->PBusyUntil_Cycle );
						
						
						// get new P register value if needed
						e->movd_regmem ( RCX, & v->vi [ VU::REG_R ].s );
						//e->MovRegMem32 ( RAX, ( &v->vi [ VU::REG_R ].s ) );
						//e->CmovAERegMem32 ( RAX, & v->NextP.l );
						//e->MovMemReg32 ( & v->vi [ VU::REG_P ].s, RAX );
						
						//if ( i.xyzw != 0xf )
						//{
						//	e->movdqa_regmem ( RAX, & v->vf [ i.Ft ].sw0 );
						//}
						
						// sign-extend from 16-bit to 32-bit
						//e->Cwde();
						
						//e->movd_to_sse ( RCX, RAX );
						e->pshufdregregimm ( RCX, RCX, 0 );
						
						if ( i.xyzw != 0xf )
						{
							//e->pblendwregregimm ( RCX, RAX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
							e->pblendwregmemimm ( RCX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
						}
						
						// set result
						//ret = e->MovMemReg32 ( ( &v->vf [ i.Ft ].sw0 ) + FtComponent, RAX );
						ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RCX );
					}
				}
				else
				{
					return -1;
				}
			}
			
			break;
#endif

		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::RINIT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "RINIT";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::RINIT;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_RINIT
		// get source and destination register(s) bitmap
		case -1:
			//v->Add_SrcRegBC ( i.fsf, i.Fs );
			Add_FSrcRegBC ( v, i.fsf, i.Fs );
			
			break;
#endif

		case 0:
			// ***testing***
			//v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
#ifdef USE_NEW_RINIT_RECOMPILE
		case 1:
			e->MovRegMem32 ( RAX, &v->vf [ i.Fs ].vsw [ i.fsf ] );
			//e->XorRegMem32 ( RAX, &v->vi [ VU::REG_R ].s );
			e->AndReg32ImmX ( RAX, 0x7fffff );
			e->OrReg32ImmX ( RAX, ( 0x7f << 23 ) );
			e->MovMemReg32 ( &v->vi [ VU::REG_R ].s, RAX );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::RNEXT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "RNEXT";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::RNEXT;
	
	static const unsigned long c_ulRandMask = 0x7ffb18;
	
	int ret = 1;
	VU::Bitmap128 bmTemp;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_RNEXT
		// get source and destination register(s) bitmap
		case -1:
		
#ifdef ENABLE_DSTBITMAP_BEFORE_DELAYSLOT
			// destination for move instruction needs to be set only if move is made
			//v->Set_DestReg_Upper ( i.Value, i.Ft );
			Add_FDstReg ( v, i.Value, i.Ft );
#endif
			
			break;
#endif

		case 0:
			// delay slot issue
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_RNEXT_RECOMPILE
		case 1:
			// check if instruction should be cancelled (if it writes to same reg as upper instruction)
			if ( !( ( 1 << i.Ft ) & v->IDstBitmap ) )
			{
				// check if destination for move is source for upper instruction
				VU::ClearBitmap ( bmTemp );
				VU::AddBitmap ( bmTemp, i.xyzw, i.Ft );
				if ( !VU::TestBitmap ( bmTemp, v->FSrcBitmap ) )
				{
					// add destination register to bitmap at end
					Add_FDstReg ( v, i.Value, i.Ft );
					
					//ret = Generate_VMOVEp ( v, i );
					if ( i.Ft && i.xyzw )
					{
						//e->MovRegMem64 ( RAX, (long long*) & v->CycleCount );
						//e->CmpRegMem64 ( RAX, (long long*) & v->PBusyUntil_Cycle );
						
						
						// get new P register value if needed
						//e->movd_regmem ( RCX, & v->vi [ VU::REG_R ].s );
						e->MovRegMem32 ( RAX, ( &v->vi [ VU::REG_R ].s ) );
						
						e->MovRegReg32 ( RCX, RAX );
						e->AndReg32ImmX( RAX, c_ulRandMask );
						//e->Set_PO ( RCX );
						e->PopCnt32 ( RAX, RAX );
						e->AndReg32ImmX ( RAX, 1 );
						e->AddRegReg32 ( RCX, RCX );
						e->OrRegReg32 ( RAX, RCX );
						
						e->AndReg32ImmX ( RAX, 0x7fffff );
						e->OrReg32ImmX ( RAX, ( 0x7f << 23 ) );
						e->MovMemReg32 ( &v->vi [ VU::REG_R ].s, RAX );
						
						//if ( i.xyzw != 0xf )
						//{
						//	e->movdqa_regmem ( RAX, & v->vf [ i.Ft ].sw0 );
						//}
						
						// sign-extend from 16-bit to 32-bit
						//e->Cwde();
						
						e->movd_to_sse ( RCX, RAX );
						e->pshufdregregimm ( RCX, RCX, 0 );
						
						if ( i.xyzw != 0xf )
						{
							//e->pblendwregregimm ( RCX, RAX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
							e->pblendwregmemimm ( RCX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
						}
						
						// set result
						//ret = e->MovMemReg32 ( ( &v->vf [ i.Ft ].sw0 ) + FtComponent, RAX );
						ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RCX );
					}
				}
				else
				{
					return -1;
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::RXOR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "RXOR";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::RXOR;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_RXOR
		// get source and destination register(s) bitmap
		case -1:
			//v->Add_SrcRegBC ( i.fsf, i.Fs );
			Add_FSrcRegBC ( v, i.fsf, i.Fs );
			
			break;
#endif

		case 0:
			// ***testing***
			//v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_RXOR_RECOMPILE
		case 1:
			e->MovRegMem32 ( RAX, &v->vf [ i.Fs ].vsw [ i.fsf ] );
			e->XorRegMem32 ( RAX, &v->vi [ VU::REG_R ].s );
			e->AndReg32ImmX ( RAX, 0x7fffff );
			e->OrReg32ImmX ( RAX, ( 0x7f << 23 ) );
			e->MovMemReg32 ( &v->vi [ VU::REG_R ].s, RAX );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::RSQRT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "RSQRT";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::RSQRT;
	
	static const u64 c_CycleTime = 13;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_RSQRT
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegBC ( i.ftf, i.Ft );
			//v->Add_SrcRegBC ( i.fsf, i.Fs );
			Add_FSrcRegBC ( v, i.fsf, i.Fs );
			Add_FSrcRegBC ( v, i.ftf, i.Ft );
			
			break;
#endif

		case 0:
			// ***testing***
			//v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;


#ifdef USE_NEW_RSQRT_RECOMPILE
		case 1:
		
			// check if QBusyUntil_Cycle is -1
			e->MovRegMem64 ( RAX, (long long*) & v->QBusyUntil_Cycle );
			e->CmpReg64ImmX ( RAX, -1 );
			e->Jmp8_E ( 0, 0 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LeaRegMem64 ( RCX, v );
			ret = e->Call ( (void*) PipelineWaitQ );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->SetJmpTarget8 ( 0 );
			
		
			// clear bits 14 and 15 in the flag register first
			//e->AndMem32ImmX ( &VU0::_VU0->vi [ VU::REG_STATUSFLAG ].u, ~0x00000030 );
			
			// flush ps2 float to zero
			e->MovRegMem32 ( RAX, &v->vf [ i.Ft ].vsw [ i.ftf ] );
			e->XorRegReg32 ( 11, 11 );
			e->MovReg64ImmX ( RCX, 896ULL << 23 );
			
			// get flags
			e->Cdq();
			e->AndReg32ImmX ( RDX, 0x00410 );
			
			
			e->AndReg32ImmX ( RAX, 0x7fffffff );
			//e->LeaRegRegReg64 ( 8, RAX, RCX );
			e->AddRegReg64 ( RCX, RAX );
			e->AndReg32ImmX ( RAX, 0x7f800000 );
			e->MovReg32ImmX ( 8, 0x00820 );
			e->CmovNERegReg32 ( 8, RDX );
			e->CmovNERegReg64 ( RAX, RCX );
			e->ShlRegImm64 ( RAX, 29 );
			
			
			// set flags
			//e->OrMemReg32 ( &VU0::_VU0->vi [ VU::REG_STATUSFLAG ].u, 8 );
			e->MovMemReg16 ( (short*) &v->NextQ_Flag, 8 );
			
			
			// move the registers now to floating point unit
			e->movq_to_sse ( RAX, RAX );
			//e->movq_to_sse ( RCX, RDX );
			
			
			// sqrt
			e->sqrtsd ( RAX, RAX );
			e->movq_from_sse ( RAX, RAX );
			
			// ??
			e->AddReg64ImmX ( RAX, 0x10000000 );
			e->AndReg64ImmX ( RAX, ~0x1fffffff );
			

			e->movq_to_sse ( RCX, RAX );


			e->MovRegMem32 ( RAX, &v->vf [ i.Fs ].vsw [ i.fsf ] );
			//e->MovRegReg32 ( RCX, RAX );
			e->Cdq ();
			e->AndReg32ImmX ( RAX, 0x7fffffff );
			//e->LeaRegRegReg64 ( RDX, RAX, RCX );
			e->TestReg32ImmX ( RAX, 0x7f800000 );
			e->CmovERegReg64 ( RAX, 11 );
			//e->ShrRegImm32 ( 10, 31 );
			//e->ShlRegImm64 ( 10, 63 );
			e->ShlRegImm64 ( RAX, 29 );
			//e->OrRegReg64 ( RAX, 10 );
			e->movq_to_sse ( RAX, RAX );

			
			// divide
			e->divsd ( RAX, RCX );
			
			
			// get result
			e->movq_from_sse ( RAX, RAX );
			
			
			// shift back down without sign
			e->ShrRegImm64 ( RAX, 29 );
			
			// subtract exponent
			//e->XorRegReg32 ( 10, 10 );
			//e->MovRegReg32 ( RDX, RAX );
			//e->AndReg64ImmX ( RAX, ~0x007fffff );
			//e->SubRegReg64 ( RAX, RCX );
			e->TestReg32ImmX ( RAX, 0xff800000 );
			
			// clear on underflow or zero
			//e->CmovLERegReg32 ( RAX, 10 );
			//e->CmovLERegReg32 ( RDX, 10 );
			e->CmovERegReg32 ( RAX, 11 );
			
			
			// set to max on overflow
			e->MovReg32ImmX ( RCX, 0x7fffffff );
			//e->OrRegReg32 ( RDX, RDX );
			e->CmovSRegReg32 ( RAX, RCX );
			
			
			// or if any flags are set indicating denominator is zero
			e->AndReg32ImmX ( 8, 0x00020 );
			e->CmovNERegReg32 ( RAX, RCX );

			
			// set sign
			e->AndReg32ImmX ( RDX, 0x80000000 );
			e->OrRegReg32 ( RAX, RDX );
			

			// store result
			//e->MovMemReg32 ( &VU0::_VU0->vi [ VU::REG_Q ].u, RAX );		// &r->CPR1 [ i.Fd ].u, RAX );
			e->MovMemReg32 ( &v->NextQ.l, RAX );
			
			// set time to process
			e->MovRegMem64 ( RAX, (long long*) & v->CycleCount );
			e->AddReg64ImmX ( RAX, c_CycleTime );
			e->MovMemReg64 ( (long long*) & v->QBusyUntil_Cycle, RAX );
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SQRT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SQRT";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SQRT;
	
	static const u64 c_CycleTime = 7;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SQRT
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegBC ( i.ftf, i.Ft );
			Add_FSrcRegBC ( v, i.ftf, i.Ft );
			
			break;
#endif

		case 0:
			// ***testing***
			//v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;


#ifdef USE_NEW_SQRT_RECOMPILE
		case 1:
			// check if QBusyUntil_Cycle is -1
			e->MovRegMem64 ( RAX, (long long*) & v->QBusyUntil_Cycle );
			e->CmpReg64ImmX ( RAX, -1 );
			e->Jmp8_E ( 0, 0 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LeaRegMem64 ( RCX, v );
			ret = e->Call ( (void*) PipelineWaitQ );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->SetJmpTarget8 ( 0 );
			
			// clear bits 14 and 15 in the flag register first
			//e->AndMem32ImmX ( &VU0::_VU0->vi [ VU::REG_STATUSFLAG ].u, ~0x00000030 );
			
			// flush ps2 float to zero
			e->MovRegMem32 ( RAX, &v->vf [ i.Ft ].vsw [ i.ftf ] );
			e->MovReg64ImmX ( RCX, 896ULL << 23 );
			
			// get flags
			e->Cdq();
			e->AndReg32ImmX ( RDX, 0x00410 );
			
			
			e->AndReg32ImmX ( RAX, 0x7fffffff );
			e->LeaRegRegReg64 ( 8, RAX, RCX );
			e->AndReg32ImmX ( RAX, 0x7f800000 );
			e->CmovERegReg32 ( RDX, RAX );
			e->CmovNERegReg64 ( RAX, 8 );
			e->ShlRegImm64 ( RAX, 29 );
			
			
			// set flags
			//e->OrMemReg32 ( &VU0::_VU0->vi [ VU::REG_STATUSFLAG ].u, RDX );
			e->MovMemReg16 ( (short*) &v->NextQ_Flag, RDX );
			
			
			// move the registers now to floating point unit
			e->movq_to_sse ( RAX, RAX );
			//e->movq_to_sse ( RCX, RDX );
			
			
			// sqrt
			e->sqrtsd ( RAX, RAX );
			e->movq_from_sse ( RAX, RAX );
			
			// ??
			e->AddReg64ImmX ( RAX, 0x10000000 );
			
			
			
			// shift back down without sign
			e->ShrRegImm64 ( RAX, 29 );
			
			// if zero, then clear RCX
			e->CmovERegReg64 ( RCX, RAX );
			
			// subtract exponent
			e->SubRegReg64 ( RAX, RCX );
			
			
			// set result
			//ret = e->MovMemReg32 ( &VU0::_VU0->vi [ VU::REG_Q ].u, RAX );
			e->MovMemReg32 ( &v->NextQ.l, RAX );
			
			// set time to process
			e->MovRegMem64 ( RAX, (long long*) & v->CycleCount );
			e->AddReg64ImmX ( RAX, c_CycleTime );
			e->MovMemReg64 ( (long long*) & v->QBusyUntil_Cycle, RAX );
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}





long Recompiler::SQD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SQD";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SQD;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SQD
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_ISrcReg ( v, ( i.it & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// store post/pre inc/dec currently implemented with delay slot
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SQD_RECOMPILE
		case 1:
		
#ifdef DISABLE_SQD_VU0
			// not doing VU#0 for now since it has more involved with load/store
			if ( !v->Number )
			{
				return -1;
			}
#endif
		
#ifdef ENABLE_INTDELAYSLOT_SQD
			// check for a conditional branch that might be affected by integer destination register
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.it & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.it & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}
#endif

			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}
			
			//LoadAddress = ( v->vi [ i.is & 0xf ].sLo + i.Imm11 ) << 2;
			e->MovRegMem32 ( RAX, & v->vi [ i.it & 0xf ].s );
			
			
			e->movdqa_regmem ( RAX, & v->vf [ i.Fs ].sw0 );
			
			//pVuMem32 = v->GetMemPtr ( LoadAddress );
			//return & ( VuMem32 [ Address32 & ( c_ulVuMem1_Mask >> 2 ) ] );
			//e->MovRegImm64 ( RCX, (u64) & v->VuMem32 [ 0 ] );
			e->LeaRegMem64 ( RCX, & v->VuMem32 [ 0 ] );

			// special code for VU#0
			if ( !v->Number )
			{
#ifdef ALL_VU0_UPPER_ADDRS_ACCESS_VU1
				e->AndReg32ImmX ( RAX, 0x7fff );
#endif

				// check if Address & 0xf000 == 0x4000
				e->MovRegReg32 ( RDX, RAX );
				e->AndReg32ImmX ( RDX, 0xf000 >> 4 );
				e->CmpReg32ImmX ( RDX, 0x4000 >> 4 );
				e->Jmp8_NE ( 0, 1 );
				
				// here it will be loading/storing from/to the registers for VU#1
				//e->LeaRegMem64 ( RDX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
				//e->CmovERegReg64 ( RCX, RDX );
				e->LeaRegMem64 ( RCX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
				
				// ***TODO*** check if storing to TPC
				e->CmpReg32ImmX ( RAX, 0x43a );
				e->Jmp8_NE ( 0, 0 );
				
				//VU1::_VU1->Running = 1;
				e->MovMemImm32 ( (long*) & VU1::_VU1->Running, 1 );
				
				//VU1::_VU1->CycleCount = *VU::_DebugCycleCount + 1;
				e->MovRegMem64 ( RDX, (long long*) VU::_DebugCycleCount );
				
				// set VBSx in VPU STAT to 1 (running)
				//VU0::_VU0->vi [ 29 ].uLo |= ( 1 << ( 1 << 3 ) );
				e->OrMemImm32 ( & VU0::_VU0->vi [ 29 ].s, ( 1 << ( 1 << 3 ) ) );
				
				// also set VIFx STAT to indicate program is running
				//VU1::_VU1->VifRegs.STAT.VEW = 1;
				e->OrMemImm32 ( (long*) & VU1::_VU1->VifRegs.STAT.Value, ( 1 << 2 ) );
				
				// finish handling the new cycle count
				e->IncReg64 ( RDX );
				e->MovMemReg64 ( (long long*) & VU1::_VU1->CycleCount, RDX );
				
				e->SetJmpTarget8 ( 0 );
				
				// mask address for accessing registers
				e->AndReg32ImmX ( RAX, 0x3f );
				
				e->SetJmpTarget8 ( 1 );
			}
			
			// post-inc
			e->DecReg32 ( RAX );
			
			if ( i.it & 0xf )
			{
			e->MovMemReg16 ( & v->vi [ i.it & 0xf ].sLo, RAX );
			}
			
			if ( !v->Number )
			{
				e->AndReg32ImmX ( RAX, VU::c_ulVuMem0_Mask >> 4 );
			}
			else
			{
				e->AndReg32ImmX ( RAX, VU::c_ulVuMem1_Mask >> 4 );
			}
			e->AddRegReg32 ( RAX, RAX );
			
			
			if ( i.xyzw != 0xf )
			{
				e->movdqa_from_mem128 ( RCX, RCX, RAX, SCALE_EIGHT, 0 );
				e->pblendwregregimm ( RAX, RCX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			}
			
			ret = e->movdqa_to_mem128 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}


//static void test_value ( u64 value )
//{
//	cout << "\nvalue(dec)=" << dec << value << " value(hex)=" << hex << value;
//}

long Recompiler::SQI ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SQI";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SQI;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SQI
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_ISrcReg ( v, ( i.it & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// store post/pre inc/dec currently implemented with delay slot
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SQI_RECOMPILE
		case 1:
		
#ifdef DISABLE_SQI_VU0
			// not doing VU#0 for now since it has more involved with load/store
			if ( !v->Number )
			{
				return -1;
			}
#endif
		
#ifdef ENABLE_INTDELAYSLOT_SQI
			// check for a conditional branch that might be affected by integer destination register
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.it & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.it & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}
#endif
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}
			
			//LoadAddress = ( v->vi [ i.is & 0xf ].sLo + i.Imm11 ) << 2;
			e->MovRegMem32 ( RAX, & v->vi [ i.it & 0xf ].s );
			
			e->movdqa_regmem ( RAX, & v->vf [ i.Fs ].sw0 );

			//pVuMem32 = v->GetMemPtr ( LoadAddress );
			//return & ( VuMem32 [ Address32 & ( c_ulVuMem1_Mask >> 2 ) ] );
			//e->MovRegImm64 ( RCX, (u64) & v->VuMem32 [ 0 ] );
			e->LeaRegMem64 ( RCX, & v->VuMem32 [ 0 ] );
			
			// special code for VU#0
			if ( !v->Number )
			{
#ifdef ALL_VU0_UPPER_ADDRS_ACCESS_VU1
				e->AndReg32ImmX ( RAX, 0x7fff );
#endif

				// check if Address & 0xf000 == 0x4000
				e->MovRegReg32 ( RDX, RAX );
				e->AndReg32ImmX ( RDX, 0xf000 >> 4 );
				e->CmpReg32ImmX ( RDX, 0x4000 >> 4 );
				e->Jmp8_NE ( 0, 1 );
				
				// here it will be loading/storing from/to the registers for VU#1
				//e->LeaRegMem64 ( RDX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
				//e->CmovERegReg64 ( RCX, RDX );
				e->LeaRegMem64 ( RCX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
				
				// ***TODO*** check if storing to TPC
				e->CmpReg32ImmX ( RAX, 0x43a );
				e->Jmp8_NE ( 0, 0 );
				
				//VU1::_VU1->Running = 1;
				e->MovMemImm32 ( (long*) & VU1::_VU1->Running, 1 );
				
				//VU1::_VU1->CycleCount = *VU::_DebugCycleCount + 1;
				e->MovRegMem64 ( RDX, (long long*) VU::_DebugCycleCount );
				
				// set VBSx in VPU STAT to 1 (running)
				//VU0::_VU0->vi [ 29 ].uLo |= ( 1 << ( 1 << 3 ) );
				e->OrMemImm32 ( & VU0::_VU0->vi [ 29 ].s, ( 1 << ( 1 << 3 ) ) );
				
				// also set VIFx STAT to indicate program is running
				//VU1::_VU1->VifRegs.STAT.VEW = 1;
				e->OrMemImm32 ( (long*) & VU1::_VU1->VifRegs.STAT.Value, ( 1 << 2 ) );
				
				// finish handling the new cycle count
				e->IncReg64 ( RDX );
				e->MovMemReg64 ( (long long*) & VU1::_VU1->CycleCount, RDX );
				
				e->SetJmpTarget8 ( 0 );
				
				// mask address for accessing registers
				e->AndReg32ImmX ( RAX, 0x3f );
				
				e->SetJmpTarget8 ( 1 );
			}
			
			
			// post-inc
			if ( i.it & 0xf )
			{
			e->LeaRegRegImm32 ( RDX, RAX, 1 );
			e->MovMemReg16 ( & v->vi [ i.it & 0xf ].sLo, RDX );
			}
			
			if ( !v->Number )
			{
				e->AndReg32ImmX ( RAX, VU::c_ulVuMem0_Mask >> 4 );
			}
			else
			{
				e->AndReg32ImmX ( RAX, VU::c_ulVuMem1_Mask >> 4 );
			}
			e->AddRegReg32 ( RAX, RAX );
			
			
			if ( i.xyzw != 0xf )
			{
				e->movdqa_from_mem128 ( RCX, RCX, RAX, SCALE_EIGHT, 0 );
				e->pblendwregregimm ( RAX, RCX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			}
			
			ret = e->movdqa_to_mem128 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::WAITQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "WAITQ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::WAITQ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// ***testing***
			//v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_WAITQ_RECOMPILE
		case 1:
		
			// check if QBusyUntil_Cycle is -1
			e->MovRegMem64 ( RAX, (long long*) & v->QBusyUntil_Cycle );
			e->CmpReg64ImmX ( RAX, -1 );
			e->Jmp8_E ( 0, 0 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LeaRegMem64 ( RCX, v );
			ret = e->Call ( (void*) PipelineWaitQ );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->SetJmpTarget8 ( 0 );

			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::B ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "B";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::B;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// branches/jumps need updated PC at level 0
			v->bStopEncodingBefore = true;
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_B_RECOMPILE
		case 1:
			// ***TODO*** check if next instruction is branch
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			//if ( v->Status_BranchDelay )
			if ( v->NextInstLO.Value & 0x40000000 )
			{
				//cout << "\nhps2x64: ALERT: VU: BRANCH IN BRANCH DELAY SLOT";
				return -1;
			}

			// check if encoding stops after this instruction (since the static delay slot is after it)
			if ( v->bStopEncodingAfter )
			{
				return -1;
			}

			
			e->MovMemImm32 ( (long*) & v->Recompiler_EnableBranchDelay, 1 );
			
			// check for branch delay
			v->Status_BranchDelay = 2;
			
			// not a conditional branch
			v->Status_BranchConditional = 0;
			
			// need to know what type of branch
			v->Status_BranchInstruction = i;
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::BAL ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "BAL";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::BAL;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// branches/jumps need updated PC at level 0
			v->bStopEncodingBefore = true;
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_BAL_RECOMPILE
		case 1:
			// ***TODO*** check if next instruction is branch
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			//if ( v->Status_BranchDelay )
			if ( v->NextInstLO.Value & 0x40000000 )
			{
				//cout << "\nhps2x64: ALERT: VU: BRANCH IN BRANCH DELAY SLOT";
				return -1;
			}

			// check if encoding stops after this instruction (since the static delay slot is after it)
			if ( v->bStopEncodingAfter )
			{
				return -1;
			}

			
			e->MovMemImm32 ( (long*) & v->Recompiler_EnableBranchDelay, 1 );
			
			// v->vi [ i.it ].uLo = ( v->PC + 16 ) >> 3;
			e->MovMemImm32 ( & v->vi [ i.it & 0xf ].s, ( Address + 16 ) >> 3 );
			
			// check for branch delay
			v->Status_BranchDelay = 2;
			
			// not a conditional branch
			v->Status_BranchConditional = 0;
			
			// need to know what type of branch
			v->Status_BranchInstruction = i;
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::JALR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "JALR";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::JALR;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// branches/jumps need updated PC at level 0
			v->bStopEncodingBefore = true;
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_JALR_RECOMPILE
		case 1:
			// ***TODO*** check if next instruction is branch
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			//if ( v->Status_BranchDelay )
			if ( v->NextInstLO.Value & 0x40000000 )
			{
				//cout << "\nhps2x64: ALERT: VU: BRANCH IN BRANCH DELAY SLOT";
				return -1;
			}

			// check if encoding stops after this instruction (since the static delay slot is after it)
			if ( v->bStopEncodingAfter )
			{
				return -1;
			}

			
			e->MovMemImm32 ( (long*) & v->Recompiler_EnableBranchDelay, 1 );
			
			// d->Data = v->vi [ i.is ].uLo;
			// *note* must do this first incase is and it registers are the same.. then it overwrites!
			e->MovRegMem32 ( RAX, & v->vi [ i.is & 0xf ].s );
			e->MovMemReg32 ( (long*) & v->Recompiler_BranchDelayAddress, RAX );
			
			// v->vi [ i.it ].uLo = ( v->PC + 16 ) >> 3;
			e->MovMemImm32 ( & v->vi [ i.it & 0xf ].s, ( Address + 16 ) >> 3 );
			
			// check for branch delay
			v->Status_BranchDelay = 2;
			
			// not a conditional branch
			v->Status_BranchConditional = 0;
			
			// need to know what type of branch
			v->Status_BranchInstruction = i;
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::JR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "JR";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::JR;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// branches/jumps need updated PC at level 0
			v->bStopEncodingBefore = true;
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_JR_RECOMPILE
		case 1:
			// ***TODO*** check if next instruction is branch
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			//if ( v->Status_BranchDelay )
			if ( v->NextInstLO.Value & 0x40000000 )
			{
				//cout << "\nhps2x64: ALERT: VU: BRANCH IN BRANCH DELAY SLOT";
				return -1;
			}

			// check if encoding stops after this instruction (since the static delay slot is after it)
			if ( v->bStopEncodingAfter )
			{
				return -1;
			}

			
			e->MovMemImm32 ( (long*) & v->Recompiler_EnableBranchDelay, 1 );
			
			// d->Data = v->vi [ i.is ].uLo;
			e->MovRegMem32 ( RAX, & v->vi [ i.is & 0xf ].s );
			e->MovMemReg32 ( (long*) & v->Recompiler_BranchDelayAddress, RAX );
			
			// check for branch delay
			v->Status_BranchDelay = 2;
			
			// not a conditional branch
			v->Status_BranchConditional = 0;
			
			// need to know what type of branch
			v->Status_BranchInstruction = i;
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}




long Recompiler::IBEQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "IBEQ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::IBEQ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// branches/jumps need updated PC at level 0
			v->bStopEncodingBefore = true;
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_IBEQ_RECOMPILE
		case 1:
			// ***TODO*** check if next instruction is branch
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			//if ( v->Status_BranchDelay )
			if ( v->NextInstLO.Value & 0x40000000 )
			{
				//cout << "\nhps2x64: ALERT: VU: BRANCH IN BRANCH DELAY SLOT";
				return -1;
			}

			// check if encoding stops after this instruction (since the static delay slot is after it)
			if ( v->bStopEncodingAfter )
			{
				return -1;
			}

			
			//if ( v->vi [ i.it ].uLo == v->vi [ i.is ].uLo )
			e->MovRegMem16 ( RAX, & v->vi [ i.it & 0xf ].sLo );
			e->MovRegMem16 ( RCX, & v->vi [ i.is & 0xf ].sLo );
			e->XorRegReg32 ( RDX, RDX );
			e->CmpRegReg16 ( RAX, RCX );
			e->Set_E ( RDX );
			
			e->MovMemReg32 ( (long*) & v->Recompiler_EnableBranchDelay, RDX );
			
			
			// check for branch delay
			v->Status_BranchDelay = 2;
			
			// not a conditional branch
			v->Status_BranchConditional = 1;
			
			// need to know what type of branch
			v->Status_BranchInstruction = i;
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::IBGEZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "IBGEZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::IBGEZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// branches/jumps need updated PC at level 0
			v->bStopEncodingBefore = true;
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_IBGEZ_RECOMPILE
		case 1:
			// ***TODO*** check if next instruction is branch
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			//if ( v->Status_BranchDelay )
			if ( v->NextInstLO.Value & 0x40000000 )
			{
				//cout << "\nhps2x64: ALERT: VU: BRANCH IN BRANCH DELAY SLOT";
				return -1;
			}

			// check if encoding stops after this instruction (since the static delay slot is after it)
			if ( v->bStopEncodingAfter )
			{
				return -1;
			}

			
			//if ( v->vi [ i.it ].uLo == v->vi [ i.is ].uLo )
			e->MovRegMem16 ( RAX, & v->vi [ i.is & 0xf ].sLo );
			e->XorRegReg32 ( RDX, RDX );
			e->CmpRegImm16 ( RAX, 0 );
			e->Set_GE ( RDX );
			
			e->MovMemReg32 ( (long*) & v->Recompiler_EnableBranchDelay, RDX );
			
			
			// check for branch delay
			v->Status_BranchDelay = 2;
			
			// not a conditional branch
			v->Status_BranchConditional = 1;
			
			// need to know what type of branch
			v->Status_BranchInstruction = i;
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::IBGTZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "IBGTZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::IBGTZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// branches/jumps need updated PC at level 0
			v->bStopEncodingBefore = true;
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_IBGTZ_RECOMPILE
		case 1:
			// ***TODO*** check if next instruction is branch
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			//if ( v->Status_BranchDelay )
			if ( v->NextInstLO.Value & 0x40000000 )
			{
				//cout << "\nhps2x64: ALERT: VU: BRANCH IN BRANCH DELAY SLOT";
				return -1;
			}

			// check if encoding stops after this instruction (since the static delay slot is after it)
			if ( v->bStopEncodingAfter )
			{
				return -1;
			}

			
			//if ( v->vi [ i.it ].uLo == v->vi [ i.is ].uLo )
			e->MovRegMem16 ( RAX, & v->vi [ i.is & 0xf ].sLo );
			e->XorRegReg32 ( RDX, RDX );
			e->CmpRegImm16 ( RAX, 0 );
			e->Set_G ( RDX );
			
			e->MovMemReg32 ( (long*) & v->Recompiler_EnableBranchDelay, RDX );
			
			
			// check for branch delay
			v->Status_BranchDelay = 2;
			
			// not a conditional branch
			v->Status_BranchConditional = 1;
			
			// need to know what type of branch
			v->Status_BranchInstruction = i;
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::IBLEZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "IBLEZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::IBLEZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// branches/jumps need updated PC at level 0
			v->bStopEncodingBefore = true;
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_IBLEZ_RECOMPILE
		case 1:
			// ***TODO*** check if next instruction is branch
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			//if ( v->Status_BranchDelay )
			if ( v->NextInstLO.Value & 0x40000000 )
			{
				//cout << "\nhps2x64: ALERT: VU: BRANCH IN BRANCH DELAY SLOT";
				return -1;
			}
			
			// check if encoding stops after this instruction (since the static delay slot is after it)
			if ( v->bStopEncodingAfter )
			{
				return -1;
			}

			
			//if ( v->vi [ i.it ].uLo == v->vi [ i.is ].uLo )
			e->MovRegMem16 ( RAX, & v->vi [ i.is & 0xf ].sLo );
			e->XorRegReg32 ( RDX, RDX );
			e->CmpRegImm16 ( RAX, 0 );
			e->Set_LE ( RDX );
			
			e->MovMemReg32 ( (long*) & v->Recompiler_EnableBranchDelay, RDX );
			
			
			// check for branch delay
			v->Status_BranchDelay = 2;
			
			// not a conditional branch
			v->Status_BranchConditional = 1;
			
			// need to know what type of branch
			v->Status_BranchInstruction = i;
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::IBLTZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "IBLTZ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::IBLTZ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// branches/jumps need updated PC at level 0
			v->bStopEncodingBefore = true;
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_IBLTZ_RECOMPILE
		case 1:
			// ***TODO*** check if next instruction is branch
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			//if ( v->Status_BranchDelay )
			if ( v->NextInstLO.Value & 0x40000000 )
			{
				//cout << "\nhps2x64: ALERT: VU: BRANCH IN BRANCH DELAY SLOT";
				return -1;
			}

			// check if encoding stops after this instruction (since the static delay slot is after it)
			if ( v->bStopEncodingAfter )
			{
				return -1;
			}

			
			//if ( v->vi [ i.it ].uLo == v->vi [ i.is ].uLo )
			e->MovRegMem16 ( RAX, & v->vi [ i.is & 0xf ].sLo );
			e->XorRegReg32 ( RDX, RDX );
			e->CmpRegImm16 ( RAX, 0 );
			e->Set_L ( RDX );
			
			e->MovMemReg32 ( (long*) & v->Recompiler_EnableBranchDelay, RDX );
			
			
			// check for branch delay
			v->Status_BranchDelay = 2;
			
			// not a conditional branch
			v->Status_BranchConditional = 1;
			
			// need to know what type of branch
			v->Status_BranchInstruction = i;
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::IBNE ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "IBNE";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::IBNE;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// branches/jumps need updated PC at level 0
			v->bStopEncodingBefore = true;
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_IBNE_RECOMPILE
		case 1:
			// ***TODO*** check if next instruction is branch
			
			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			//if ( v->Status_BranchDelay )
			if ( v->NextInstLO.Value & 0x40000000 )
			{
				//cout << "\nhps2x64: ALERT: VU: BRANCH IN BRANCH DELAY SLOT";
				return -1;
			}
			
			
			// check if encoding stops after this instruction (since the static delay slot is after it)
			if ( v->bStopEncodingAfter )
			{
				return -1;
			}

			
			//if ( v->vi [ i.it ].uLo == v->vi [ i.is ].uLo )
			e->MovRegMem16 ( RAX, & v->vi [ i.it & 0xf ].sLo );
			e->MovRegMem16 ( RCX, & v->vi [ i.is & 0xf ].sLo );
			e->XorRegReg32 ( RDX, RDX );
			e->CmpRegReg16 ( RAX, RCX );
			e->Set_NE ( RDX );
			
			e->MovMemReg32 ( (long*) & v->Recompiler_EnableBranchDelay, RDX );
			
			
			// check for branch delay
			v->Status_BranchDelay = 2;
			
			// not a conditional branch
			v->Status_BranchConditional = 1;
			
			// need to know what type of branch
			v->Status_BranchInstruction = i;
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::FCAND ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FCAND";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FCAND;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FCAND_RECOMPILE
		case 1:
			e->MovRegMem32 ( RAX, (long*) & v->iFlagSave_Index );
			//e->MovRegImm64 ( RCX, (u64) & v->FlagSave );
			e->LeaRegMem64 ( RCX, & v->FlagSave );
			e->IncReg32 ( RAX );
			e->AndReg32ImmX ( RAX, VU::c_lFlag_Delay_Mask );
			e->ShlRegImm32 ( RAX, 2 );
			e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 4 );
			
			e->XorRegReg32 ( RCX, RCX );
			e->AndReg32ImmX ( RAX, i.Imm24 );
			e->Set_NE ( RCX );
			
			// store to 16-bits or 32-bits ??
			e->MovMemReg32 ( & v->vi [ 1 ].s, RCX );
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::FCEQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FCEQ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FCEQ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
#ifdef USE_NEW_FCEQ_RECOMPILE
		case 1:
			e->MovRegMem32 ( RAX, (long*) & v->iFlagSave_Index );
			//e->MovRegImm64 ( RCX, (u64) & v->FlagSave );
			e->LeaRegMem64 ( RCX, & v->FlagSave );
			e->IncReg32 ( RAX );
			e->AndReg32ImmX ( RAX, VU::c_lFlag_Delay_Mask );
			e->ShlRegImm32 ( RAX, 2 );
			e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 4 );
			
			e->XorRegReg32 ( RCX, RCX );
			e->XorReg32ImmX ( RAX, i.Imm24 );
			e->AndReg32ImmX ( RAX, 0x00ffffff );
			e->Set_E ( RCX );
			
			// store to 16-bits or 32-bits ??
			e->MovMemReg32 ( & v->vi [ 1 ].s, RCX );
			
			break;
#endif

		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::FCGET ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FCGET";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FCGET;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
#ifdef USE_NEW_FCGET_RECOMPILE
		case 1:
			if ( i.it & 0xf )
			{
				e->MovRegMem32 ( RAX, (long*) & v->iFlagSave_Index );
				//e->MovRegImm64 ( RCX, (u64) & v->FlagSave );
				e->LeaRegMem64 ( RCX, & v->FlagSave );
				e->IncReg32 ( RAX );
				e->AndReg32ImmX ( RAX, VU::c_lFlag_Delay_Mask );
				e->ShlRegImm32 ( RAX, 2 );
				e->MovRegFromMem32 ( RDX, RCX, RAX, SCALE_EIGHT, 4 );
				
				e->AndReg32ImmX ( RDX, 0xfff );
				e->MovMemReg16 ( & v->vi [ i.it & 0xf ].sLo, RDX );
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::FCOR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FCOR";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FCOR;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FCOR_RECOMPILE
		case 1:
			e->MovRegMem32 ( RAX, (long*) & v->iFlagSave_Index );
			//e->MovRegImm64 ( RCX, (u64) & v->FlagSave );
			e->LeaRegMem64 ( RCX, & v->FlagSave );
			e->IncReg32 ( RAX );
			e->AndReg32ImmX ( RAX, VU::c_lFlag_Delay_Mask );
			e->ShlRegImm32 ( RAX, 2 );
			e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 4 );
			
			e->XorRegReg32 ( RCX, RCX );
			e->OrReg32ImmX ( RAX, i.Imm24 );
			e->AndReg32ImmX ( RAX, 0x00ffffff );
			e->CmpReg32ImmX ( RAX, 0xffffff );
			e->Set_E ( RCX );
			
			// store to 16-bits or 32-bits ??
			e->MovMemReg32 ( & v->vi [ 1 ].s, RCX );
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::FCSET ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FCSET";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FCSET;
	
	int ret = 1;
	
	// sets clip flag in lower instruction
	v->SetClip_Flag = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FCSET_RECOMPILE
		case 1:
			ret = e->MovMemImm32 ( &v->vi [ VU::REG_CLIPFLAG ].s, i.Imm24 );
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}




long Recompiler::FMAND ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FMAND";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FMAND;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FMAND_RECOMPILE
		case 1:
			if ( i.it & 0xf )
			{
				if ( ! ( i.is & 0xf ) )
				{
					e->MovMemImm32 ( & v->vi [ i.it & 0xf ].s, 0 );
				}
				else
				{
					e->MovRegMem32 ( RAX, (long*) & v->iFlagSave_Index );
					
#ifdef ENABLE_VFLAGS
					e->LeaRegMem64 ( RCX, & v->vFlagSave );
#else
					e->LeaRegMem64 ( RCX, & v->FlagSave );
#endif
					
					e->IncReg32 ( RAX );
					e->AndReg32ImmX ( RAX, VU::c_lFlag_Delay_Mask );
					
#ifdef ENABLE_VFLAGS
					e->AddRegReg32 ( RAX, RAX );
					e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
					e->MovRegReg32 ( RCX, RAX );
					e->ShrRegImm32 ( RAX, 16 );
					e->ShlRegImm32 ( RAX, 4 );
					e->OrRegReg32 ( RAX, RCX );
					e->AndReg32ImmX ( RAX, 0xffff );
#else
					e->ShlRegImm32 ( RAX, 2 );
					e->MovRegFromMem16 ( RAX, RCX, RAX, SCALE_EIGHT, 2 );
#endif
					
					e->MovRegMem32 ( RCX, & v->vi [ i.is & 0xf ].s );
					e->AndRegReg32 ( RAX, RCX );
					
					// store to 16-bits or 32-bits ??
					ret = e->MovMemReg32 ( & v->vi [ i.it & 0xf ].s, RAX );
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )

	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::FMEQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FMEQ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FMEQ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FMEQ_RECOMPILE
		case 1:
			if ( i.it & 0xf )
			{
				e->MovRegMem32 ( RAX, (long*) & v->iFlagSave_Index );
				
#ifdef ENABLE_VFLAGS
					e->LeaRegMem64 ( RCX, & v->vFlagSave );
#else
					e->LeaRegMem64 ( RCX, & v->FlagSave );
#endif
				
				e->IncReg32 ( RAX );
				e->AndReg32ImmX ( RAX, VU::c_lFlag_Delay_Mask );
				
#ifdef ENABLE_VFLAGS
					e->AddRegReg32 ( RAX, RAX );
					e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
					e->MovRegReg32 ( RCX, RAX );
					e->ShrRegImm32 ( RAX, 16 );
					e->ShlRegImm32 ( RAX, 4 );
					e->OrRegReg32 ( RAX, RCX );
					e->AndReg32ImmX ( RAX, 0xffff );
#else
					e->ShlRegImm32 ( RAX, 2 );
					e->MovRegFromMem16 ( RAX, RCX, RAX, SCALE_EIGHT, 2 );
#endif
				
				if ( ! ( i.is & 0xf ) )
				{
					e->XorRegReg32 ( RCX, RCX );
				}
				else
				{
					e->MovRegMem16 ( RCX, & v->vi [ i.is & 0xf ].sLo );
				}
				
				e->XorRegReg32 ( RDX, RDX );
				e->CmpRegReg16 ( RAX, RCX );
				e->Set_E ( RDX );
				
				// store to 16-bits or 32-bits ??
				ret = e->MovMemReg32 ( & v->vi [ i.it & 0xf ].s, RDX );
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::FMOR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FMOR";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FMOR;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FMOR_RECOMPILE
		case 1:
			if ( i.it & 0xf )
			{
				e->MovRegMem32 ( RAX, (long*) & v->iFlagSave_Index );
				
#ifdef ENABLE_VFLAGS
					e->LeaRegMem64 ( RCX, & v->vFlagSave );
#else
					e->LeaRegMem64 ( RCX, & v->FlagSave );
#endif
				
				e->IncReg32 ( RAX );
				e->AndReg32ImmX ( RAX, VU::c_lFlag_Delay_Mask );
				
#ifdef ENABLE_VFLAGS
					e->AddRegReg32 ( RAX, RAX );
					e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
					e->MovRegReg32 ( RCX, RAX );
					e->ShrRegImm32 ( RAX, 16 );
					e->ShlRegImm32 ( RAX, 4 );
					e->OrRegReg32 ( RAX, RCX );
					e->AndReg32ImmX ( RAX, 0xffff );
#else
					e->ShlRegImm32 ( RAX, 2 );
					e->MovRegFromMem16 ( RAX, RCX, RAX, SCALE_EIGHT, 2 );
#endif
				
				if ( ( i.is & 0xf ) )
				{
					e->MovRegMem16 ( RCX, & v->vi [ i.is & 0xf ].sLo );
					e->OrRegReg16 ( RAX, RCX );
				}
				
				// store to 16-bits or 32-bits ??
				ret = e->MovMemReg32 ( & v->vi [ i.it & 0xf ].s, RAX );
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::FSAND ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FSAND";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FSAND;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FSAND_RECOMPILE
		case 1:
			if ( i.it & 0xf )
			{
				e->MovRegMem32 ( RAX, (long*) & v->iFlagSave_Index );
				e->LeaRegMem64 ( RCX, & v->FlagSave );
				e->IncReg32 ( RAX );
				e->AndReg32ImmX ( RAX, VU::c_lFlag_Delay_Mask );
				e->ShlRegImm32 ( RAX, 2 );
				e->MovRegFromMem16 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
				
				e->AndReg32ImmX ( RAX, ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfff ) );
				
				// store to 16-bits or 32-bits ??
				ret = e->MovMemReg32 ( & v->vi [ i.it & 0xf ].s, RAX );
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::FSEQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FSEQ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FSEQ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FSEQ_RECOMPILE
		case 1:
			if ( i.it & 0xf )
			{
				e->MovRegMem32 ( RAX, (long*) & v->iFlagSave_Index );
				e->LeaRegMem64 ( RCX, & v->FlagSave );
				e->IncReg32 ( RAX );
				e->AndReg32ImmX ( RAX, VU::c_lFlag_Delay_Mask );
				e->ShlRegImm32 ( RAX, 2 );
				e->MovRegFromMem16 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
				
				e->XorRegReg32 ( RCX, RCX );
				//e->CmpRegImm16 ( RAX, ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfff ) );
				e->XorReg16ImmX ( RAX, ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfff ) );
				e->AndReg16ImmX ( RAX, 0xfff );
				e->Set_E ( RCX );
				
				// store to 16-bits or 32-bits ??
				ret = e->MovMemReg32 ( & v->vi [ i.it & 0xf ].s, RCX );
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::FSOR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FSOR";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FSOR;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FSOR_RECOMPILE
		case 1:
			if ( i.it & 0xf )
			{
				e->MovRegMem32 ( RAX, (long*) & v->iFlagSave_Index );
				e->LeaRegMem64 ( RCX, & v->FlagSave );
				e->IncReg32 ( RAX );
				e->AndReg32ImmX ( RAX, VU::c_lFlag_Delay_Mask );
				e->ShlRegImm32 ( RAX, 2 );
				e->MovRegFromMem16 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
				
				if ( ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfff ) )
				{
					e->OrReg32ImmX ( RAX, ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfff ) );
				}
				
				e->AndReg32ImmX ( RAX, 0x00000fff );
				
				// store to 16-bits or 32-bits ??
				ret = e->MovMemReg32 ( & v->vi [ i.it & 0xf ].s, RAX );
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::FSSET ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "FSSET";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::FSSET;
	
	int ret = 1;
	
	// sets stat flag in lower instruction
	v->SetStatus_Flag = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_FSSET_RECOMPILE
		case 1:
			e->MovRegMem32 ( RAX, & v->vi [ VU::REG_STATUSFLAG ].s );
			e->AndReg32ImmX ( RAX, 0x3f );
			e->OrReg32ImmX ( RAX, ( ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) & 0xfc0 ) );
			e->MovMemReg32 ( & v->vi [ VU::REG_STATUSFLAG ].s, RAX );
			break;
#endif

			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::IADDIU ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "IADDIU";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::IADDIU;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_IADDIU
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// integer math at level 0 has delay slot
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_IADDIU_RECOMPILE
		case 1:
			// check for a conditional branch that might be affected by integer destination register
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.it & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.it & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}

			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}
			
			if ( i.it & 0xf )
			{
				if ( !( i.is & 0xf ) )
				{
					e->MovMemImm16 ( &v->vi [ i.it & 0xf ].sLo, ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) );
				}
				else if ( i.it == i.is )
				{
					e->AddMemImm16 ( &v->vi [ i.it & 0xf ].sLo, ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) );
				}
				else
				{
					e->MovRegMem32 ( RAX, &v->vi [ i.is & 0xf ].s );
					e->AddRegImm16 ( RAX, ( ( i.Imm15_1 << 11 ) | ( i.Imm15_0 ) ) );
					e->MovMemReg32 ( &v->vi [ i.it & 0xf ].s, RAX );
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ILW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ILW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ILW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ILW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			
			if ( i.it & 0xf )
			{
				Add_IDstReg ( v, ( i.it & 0xf ) + 32 );
			}
			
			break;
#endif

		case 0:
			// load at level 0 has delay slot
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ILW_RECOMPILE
		case 1:
		
#ifdef DISABLE_ILW_VU0
			// not doing VU#0 for now since it has more involved with load/store
			if ( !v->Number )
			{
				return -1;
			}
#endif
		
			// check for a conditional branch that might be affected by integer destination register
			/*
			if ( ( v->NextInstLO.Opcode & 0x28 ) == 0x28 )
			{
				if ( ( ( i.it & 0xf ) == ( v->NextInstLO.it & 0xf ) ) || ( ( i.it & 0xf ) == ( v->NextInstLO.is & 0xf ) ) )
				{
					return -1;
				}
			}

			// make sure we are not in a known branch delay slot
			// E-Bit delay slot wouldn't matter, just branch delay
			if ( v->Status_BranchDelay )
			{
				return -1;
			}
			*/
			
			if ( i.it )
			{
				//LoadAddress = ( v->vi [ i.is & 0xf ].sLo + i.Imm11 ) << 2;
				e->MovRegMem32 ( RAX, & v->vi [ i.is & 0xf ].s );
				
				
				//pVuMem32 = v->GetMemPtr ( LoadAddress );
				//return & ( VuMem32 [ Address32 & ( c_ulVuMem1_Mask >> 2 ) ] );
				//e->MovRegImm64 ( RCX, (u64) & v->VuMem32 [ 0 ] );
				e->LeaRegMem64 ( RCX, & v->VuMem32 [ 0 ] );
				
				e->AddReg32ImmX ( RAX, (s32) i.Imm11 );
				
				
				// special code for VU#0
				if ( !v->Number )
				{
					// check if Address & 0xf000 == 0x4000
					e->MovRegReg32 ( RDX, RAX );
					e->AndReg32ImmX ( RDX, 0xf000 >> 4 );
					e->CmpReg32ImmX ( RDX, 0x4000 >> 4 );
					
					// here it will be loading/storing from/to the registers for VU#1
					e->LeaRegMem64 ( RDX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
					e->CmovERegReg64 ( RCX, RDX );
					
					// ***TODO*** check if storing to TPC
				}
				
				
				if ( !v->Number )
				{
				e->AndReg32ImmX ( RAX, VU::c_ulVuMem0_Mask >> 4 );
				}
				else
				{
				e->AndReg32ImmX ( RAX, VU::c_ulVuMem1_Mask >> 4 );
				}
				e->AddRegReg32 ( RAX, RAX );
				
				switch( i.xyzw )
				{
					case 8:
						e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
						break;
						
					case 4:
						e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 4 );
						break;
						
					case 2:
						e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 8 );
						break;
						
					case 1:
						e->MovRegFromMem32 ( RAX, RCX, RAX, SCALE_EIGHT, 12 );
						break;
						
					default:
						cout << "\nVU: Recompiler: ALERT: ILWR with illegal xyzw=" << hex << i.xyzw << "\n";
						break;
				}
				
				
				ret = e->MovMemReg32 ( & v->vi [ i.it & 0xf ].s, RAX );
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::ISW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ISW";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ISW;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ISW
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.it & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// ***testing***
			//v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_ISW_RECOMPILE
		case 1:
		
#ifdef DISABLE_ISW_VU0
			// not doing VU#0 for now since it has more involved with load/store
			if ( !v->Number )
			{
				return -1;
			}
#endif
		
				
			//LoadAddress = ( v->vi [ i.is & 0xf ].sLo + i.Imm11 ) << 2;
			e->MovRegMem32 ( RAX, & v->vi [ i.is & 0xf ].s );
			e->movd_regmem ( RAX, & v->vi [ i.it & 0xf ].s );
			
			//e->movdqa_regmem ( RAX, & v->vf [ i.Fs ].sw0 );
			
			//pVuMem32 = v->GetMemPtr ( LoadAddress );
			//return & ( VuMem32 [ Address32 & ( c_ulVuMem1_Mask >> 2 ) ] );
			//e->MovRegImm64 ( RCX, (u64) & v->VuMem32 [ 0 ] );
			e->LeaRegMem64 ( RCX, & v->VuMem32 [ 0 ] );
			
			e->AddReg32ImmX ( RAX, (s32) i.Imm11 );
			
			// special code for VU#0
			if ( !v->Number )
			{
#ifdef ALL_VU0_UPPER_ADDRS_ACCESS_VU1
				e->AndReg32ImmX ( RAX, 0x7fff );
#endif

				// check if Address & 0xf000 == 0x4000
				e->MovRegReg32 ( RDX, RAX );
				e->AndReg32ImmX ( RDX, 0xf000 >> 4 );
				e->CmpReg32ImmX ( RDX, 0x4000 >> 4 );
				e->Jmp8_NE ( 0, 1 );
				
				// here it will be loading/storing from/to the registers for VU#1
				//e->LeaRegMem64 ( RDX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
				//e->CmovERegReg64 ( RCX, RDX );
				e->LeaRegMem64 ( RCX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
				
				// ***TODO*** check if storing to TPC
				e->CmpReg32ImmX ( RAX, 0x43a );
				e->Jmp8_NE ( 0, 0 );
				
				//VU1::_VU1->Running = 1;
				e->MovMemImm32 ( (long*) & VU1::_VU1->Running, 1 );
				
				//VU1::_VU1->CycleCount = *VU::_DebugCycleCount + 1;
				e->MovRegMem64 ( RDX, (long long*) VU::_DebugCycleCount );
				
				// set VBSx in VPU STAT to 1 (running)
				//VU0::_VU0->vi [ 29 ].uLo |= ( 1 << ( 1 << 3 ) );
				e->OrMemImm32 ( & VU0::_VU0->vi [ 29 ].s, ( 1 << ( 1 << 3 ) ) );
				
				// also set VIFx STAT to indicate program is running
				//VU1::_VU1->VifRegs.STAT.VEW = 1;
				e->OrMemImm32 ( (long*) & VU1::_VU1->VifRegs.STAT.Value, ( 1 << 2 ) );
				
				// finish handling the new cycle count
				e->IncReg64 ( RDX );
				e->MovMemReg64 ( (long long*) & VU1::_VU1->CycleCount, RDX );
				
				e->SetJmpTarget8 ( 0 );
				
				// mask address for accessing registers
				e->AndReg32ImmX ( RAX, 0x3f );
				
				e->SetJmpTarget8 ( 1 );
			}
			
			if ( !v->Number )
			{
			e->AndReg32ImmX ( RAX, VU::c_ulVuMem0_Mask >> 4 );
			}
			else
			{
			e->AndReg32ImmX ( RAX, VU::c_ulVuMem1_Mask >> 4 );
			}
			e->AddRegReg32 ( RAX, RAX );

			if ( i.xyzw != 0xf )
			{
				e->movdqa_from_mem128 ( RCX, RCX, RAX, SCALE_EIGHT, 0 );
			}
			
			e->pmovzxwdregreg ( RAX, RAX );
			e->pshufdregregimm ( RAX, RAX, 0 );
			//e->pslldregimm ( RAX, 16 );
			//e->psrldregimm ( RAX, 16 );
			
			if ( i.xyzw != 0xf )
			{
				//e->movdqa_from_mem128 ( RCX, RCX, RAX, SCALE_EIGHT, 0 );
				e->pblendwregregimm ( RAX, RCX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			}
			
			//ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RAX );
			ret = e->movdqa_to_mem128 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::LQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "LQ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::LQ;
	
	int ret = 1;
	VU::Bitmap128 bmTemp;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_LQ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			
#ifdef ENABLE_DSTBITMAP_BEFORE_DELAYSLOT
			// destination for move instruction needs to be set only if move is made
			Add_FDstReg ( v, i.Value, i.Ft );
#endif
			
			break;
#endif

		case 0:
			// load at level 0 has delay slot
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_LQ_RECOMPILE
		case 1:
		
#ifdef DISABLE_LQ_VU0
			// not doing VU#0 for now since it has more involved with load/store
			if ( !v->Number )
			{
				return -1;
			}
#endif
		
			// check if instruction should be cancelled (if it writes to same reg as upper instruction)
			if ( !( ( 1 << i.Ft ) & v->IDstBitmap ) )
			{
				// check if destination for move is source for upper instruction
				VU::ClearBitmap ( bmTemp );
				VU::AddBitmap ( bmTemp, i.xyzw, i.Ft );
				if ( !VU::TestBitmap ( bmTemp, v->FSrcBitmap ) )
				{
					if ( i.Ft )
					{
						// add destination register to bitmap at end
						Add_FDstReg ( v, i.Value, i.Ft );
						
						//LoadAddress = ( v->vi [ i.is & 0xf ].sLo + i.Imm11 ) << 2;
						e->MovRegMem32 ( RAX, & v->vi [ i.is & 0xf ].s );
						
						//if ( i.xyzw != 0xf )
						//{
						//	e->movdqa_regmem ( RCX, & v->vf [ i.Ft ].sw0 );
						//}

						e->AddReg32ImmX ( RAX, (s32) i.Imm11 );
						
						//pVuMem32 = v->GetMemPtr ( LoadAddress );
						//return & ( VuMem32 [ Address32 & ( c_ulVuMem1_Mask >> 2 ) ] );
						//e->MovRegImm64 ( RCX, (u64) & v->VuMem32 [ 0 ] );
						e->LeaRegMem64 ( RCX, & v->VuMem32 [ 0 ] );
						
						// special code for VU#0
						if ( !v->Number )
						{
							// check if Address & 0xf000 == 0x4000
							e->MovRegReg32 ( RDX, RAX );
							e->AndReg32ImmX ( RDX, 0xf000 >> 4 );
							e->CmpReg32ImmX ( RDX, 0x4000 >> 4 );
							
							// here it will be loading/storing from/to the registers for VU#1
							e->LeaRegMem64 ( RDX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
							e->CmovERegReg64 ( RCX, RDX );
							
						}
						
						
						if ( !v->Number )
						{
							e->AndReg32ImmX ( RAX, VU::c_ulVuMem0_Mask >> 4 );
						}
						else
						{
							e->AndReg32ImmX ( RAX, VU::c_ulVuMem1_Mask >> 4 );
						}
						e->AddRegReg32 ( RAX, RAX );
						
						//e->MovRegFromMem32 ( RDX, RCX, RAX, SCALE_EIGHT, 0 );
						e->movdqa_from_mem128 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
						
						if ( i.xyzw != 0xf )
						{
							//e->pblendwregregimm ( RAX, RCX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
							e->pblendwregmemimm ( RAX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
						}
						
						ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RAX );
					}
				}
				else
				{
					return -1;
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::SQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "SQ";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::SQ;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_SQ
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcReg ( ( i.is & 0xf ) + 32 );
			Add_FSrcReg ( v, i.Value, i.Fs );
			Add_ISrcReg ( v, ( i.it & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// ***testing***
			//v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_SQ_RECOMPILE
		case 1:
		
#ifdef DISABLE_SQ_VU0
			// not doing VU#0 for now since it has more involved with load/store
			if ( !v->Number )
			{
				return -1;
			}
#endif
				
			//LoadAddress = ( v->vi [ i.is & 0xf ].sLo + i.Imm11 ) << 2;
			e->MovRegMem32 ( RAX, & v->vi [ i.it & 0xf ].s );
			
			e->movdqa_regmem ( RAX, & v->vf [ i.Fs ].sw0 );
			
			//pVuMem32 = v->GetMemPtr ( LoadAddress );
			//return & ( VuMem32 [ Address32 & ( c_ulVuMem1_Mask >> 2 ) ] );
			//e->MovRegImm64 ( RCX, (u64) & v->VuMem32 [ 0 ] );
			e->LeaRegMem64 ( RCX, & v->VuMem32 [ 0 ] );
			
			e->AddReg32ImmX ( RAX, (s32) i.Imm11 );
			
			// special code for VU#0
			if ( !v->Number )
			{
#ifdef ALL_VU0_UPPER_ADDRS_ACCESS_VU1
				e->AndReg32ImmX ( RAX, 0x7fff );
#endif

				// check if Address & 0xf000 == 0x4000
				e->MovRegReg32 ( RDX, RAX );
				e->AndReg32ImmX ( RDX, 0xf000 >> 4 );
				e->CmpReg32ImmX ( RDX, 0x4000 >> 4 );
				e->Jmp8_NE ( 0, 1 );
				
				// here it will be loading/storing from/to the registers for VU#1
				//e->LeaRegMem64 ( RDX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
				//e->CmovERegReg64 ( RCX, RDX );
				e->LeaRegMem64 ( RCX, & VU::_VU [ 1 ]->vf [ 0 ].sw0 );
				
				// ***TODO*** check if storing to TPC
				e->CmpReg32ImmX ( RAX, 0x43a );
				e->Jmp8_NE ( 0, 0 );
				
				//VU1::_VU1->Running = 1;
				e->MovMemImm32 ( (long*) & VU1::_VU1->Running, 1 );
				
				//VU1::_VU1->CycleCount = *VU::_DebugCycleCount + 1;
				e->MovRegMem64 ( RDX, (long long*) VU::_DebugCycleCount );
				
				// set VBSx in VPU STAT to 1 (running)
				//VU0::_VU0->vi [ 29 ].uLo |= ( 1 << ( 1 << 3 ) );
				e->OrMemImm32 ( & VU0::_VU0->vi [ 29 ].s, ( 1 << ( 1 << 3 ) ) );
				
				// also set VIFx STAT to indicate program is running
				//VU1::_VU1->VifRegs.STAT.VEW = 1;
				e->OrMemImm32 ( (long*) & VU1::_VU1->VifRegs.STAT.Value, ( 1 << 2 ) );
				
				// finish handling the new cycle count
				e->IncReg64 ( RDX );
				e->MovMemReg64 ( (long long*) & VU1::_VU1->CycleCount, RDX );
				
				e->SetJmpTarget8 ( 0 );
				
				// mask address for accessing registers
				e->AndReg32ImmX ( RAX, 0x3f );
				
				e->SetJmpTarget8 ( 1 );
			}
			
			if ( !v->Number )
			{
				e->AndReg32ImmX ( RAX, VU::c_ulVuMem0_Mask >> 4 );
			}
			else
			{
				e->AndReg32ImmX ( RAX, VU::c_ulVuMem1_Mask >> 4 );
			}
			e->AddRegReg32 ( RAX, RAX );
			
			
			if ( i.xyzw != 0xf )
			{
				e->movdqa_from_mem128 ( RCX, RCX, RAX, SCALE_EIGHT, 0 );
				e->pblendwregregimm ( RAX, RCX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
			}
			
			ret = e->movdqa_to_mem128 ( RAX, RCX, RAX, SCALE_EIGHT, 0 );
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::MFP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "MFP";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::MFP;
	
	int ret = 1;
	VU::Bitmap128 bmTemp;
	
	switch ( v->OpLevel )
	{
		case 0:
			// delay slot issue
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_MFP_RECOMPILE
		case 1:
			// check if instruction should be cancelled (if it writes to same reg as upper instruction)
			if ( !( ( 1 << i.Ft ) & v->IDstBitmap ) )
			{
				// check if destination for move is source for upper instruction
				VU::ClearBitmap ( bmTemp );
				VU::AddBitmap ( bmTemp, i.xyzw, i.Ft );
				if ( !VU::TestBitmap ( bmTemp, v->FSrcBitmap ) )
				{
					// add destination register to bitmap at end
					Add_FDstReg ( v, i.Value, i.Ft );
					
					//ret = Generate_VMOVEp ( v, i );
					if ( i.Ft && i.xyzw )
					{
						e->MovRegMem64 ( RAX, (long long*) & v->CycleCount );
						e->CmpRegMem64 ( RAX, (long long*) & v->PBusyUntil_Cycle );
						
						
						// get new P register value if needed
						//e->movd_regmem ( RCX, & v->vi [ VU::REG_P ].s );
						e->MovRegMem32 ( RAX, ( &v->vi [ VU::REG_P ].s ) );
						e->CmovAERegMem32 ( RAX, & v->NextP.l );
						e->MovMemReg32 ( & v->vi [ VU::REG_P ].s, RAX );
						
						//if ( i.xyzw != 0xf )
						//{
						//	e->movdqa_regmem ( RAX, & v->vf [ i.Ft ].sw0 );
						//}
						
						// sign-extend from 16-bit to 32-bit
						//e->Cwde();
						
						e->movd_to_sse ( RCX, RAX );
						e->pshufdregregimm ( RCX, RCX, 0 );
						
						if ( i.xyzw != 0xf )
						{
							//e->pblendwregregimm ( RCX, RAX, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
							e->pblendwregmemimm ( RCX, & v->vf [ i.Ft ].sw0, ~( ( i.destx * 0x03 ) | ( i.desty * 0x0c ) | ( i.destz * 0x30 ) | ( i.destw * 0xc0 ) ) );
						}
						
						// set result
						//ret = e->MovMemReg32 ( ( &v->vf [ i.Ft ].sw0 ) + FtComponent, RAX );
						ret = e->movdqa_memreg ( & v->vf [ i.Ft ].sw0, RCX );
					}
				}
				else
				{
					return -1;
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::WAITP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "WAITP";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::WAITP;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// ***testing***
			//v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::XGKICK ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "XGKICK";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::XGKICK;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_XGKICK
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_Int_SrcRegs ( ( i.is & 0xf ) + 32, ( i.it & 0xf ) + 32 );
			Add_ISrcReg ( v, ( i.is & 0xf ) + 32 );
			
			break;
#endif

		case 0:
			// xgkick currently at level 0 has delay slot
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::XITOP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "XITOP";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::XITOP;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// integer register destination
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;

#ifdef USE_NEW_XITOP_RECOMPILE
		case 1:
			if ( i.it & 0xf )
			{
				e->MovRegMem32 ( RAX, (long*) &v->iVifRegs.ITOP );
				
				if ( !v->Number )
				{
					e->AndReg32ImmX ( RAX, 0xff );
				}
				else
				{
					e->AndReg32ImmX ( RAX, 0x3ff );
				}
				
				// store to 16-bit or full 32-bit ??
				e->MovMemReg16 ( &v->vi [ i.it & 0xf ].sLo, RAX );
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::XTOP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "XTOP";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::XTOP;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
		case 0:
			// integer register destination
			v->bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
#ifdef USE_NEW_XTOP_RECOMPILE
		case 1:
			if ( i.it & 0xf )
			{
				e->MovRegMem32 ( RAX, (long*) &v->iVifRegs.TOP );
				e->AndReg32ImmX ( RAX, 0x3ff );
				
				// store to 16-bit or full 32-bit ??
				e->MovMemReg16 ( &v->vi [ i.it & 0xf ].sLo, RAX );
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






// external unit


long Recompiler::EATAN ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "EATAN";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::EATAN;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_EATAN
		// get source and destination register(s) bitmap
		case -1:
			//v->Add_SrcRegBC ( i.fsf, i.Fs );
			Add_FSrcRegBC ( v, i.fsf, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::EATANxy ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "EATANxy";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::EATANxy;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_EATANxy
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::EATANxz ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "EATANxz";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::EATANxz;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_EATANxz
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::EEXP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "EEXP";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::EEXP;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_EEXP
		// get source and destination register(s) bitmap
		case -1:
			//v->Add_SrcRegBC ( i.fsf, i.Fs );
			Add_FSrcRegBC ( v, i.fsf, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ELENG ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ELENG";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ELENG;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ELENG
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ERCPR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ERCPR";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ERCPR;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ERCPR
		// get source and destination register(s) bitmap
		case -1:
			//v->Add_SrcRegBC ( i.fsf, i.Fs );
			Add_FSrcRegBC ( v, i.fsf, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ERLENG ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ERLENG";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ERLENG;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ERLENG
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ERSADD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ERSADD";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ERSADD;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ERSADD
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}






long Recompiler::ERSQRT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ERSQRT";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ERSQRT;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ERSQRT
		// get source and destination register(s) bitmap
		case -1:
			//v->Add_SrcRegBC ( i.fsf, i.Fs );
			Add_FSrcRegBC ( v, i.fsf, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ESADD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ESADD";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ESADD;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ESADD
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ESIN ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ESIN";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ESIN;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ESIN
		// get source and destination register(s) bitmap
		case -1:
			//v->Add_SrcRegBC ( i.fsf, i.Fs );
			Add_FSrcRegBC ( v, i.fsf, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ESQRT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ESQRT";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ESQRT;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ESQRT
		// get source and destination register(s) bitmap
		case -1:
			//v->Add_SrcRegBC ( i.fsf, i.Fs );
			Add_FSrcRegBC ( v, i.fsf, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}



long Recompiler::ESUM ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address )
{
	static constexpr char *c_sName = "ESUM";
	static const void *c_vFunction = (const void*) Vu::Instruction::Execute::ESUM;
	
	int ret = 1;
	
	switch ( v->OpLevel )
	{
#ifdef ENABLE_BITMAP_ESUM
		// get source and destination register(s) bitmap
		case -1:
			//v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
			Add_FSrcReg ( v, i.Value, i.Fs );
			
			break;
#endif

		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LeaRegMem64 ( RCX, v ); e->LoadImm32 ( RDX, i.Value );
			ret = e->Call ( c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			
			break;
			
		default:
			return -1;
			break;
			
	} // end switch ( v->OpLevel )
	
	if ( !ret )
	{
		cout << "\nx64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}








const Vu::Recompiler::Function Vu::Recompiler::FunctionList []
{
	Vu::Recompiler::INVALID,
	
	// VU macro mode instructions //
	
	//Vu::Recompiler::COP2
	//Vu::Recompiler::QMFC2_NI, Vu::Recompiler::QMFC2_I, Vu::Recompiler::QMTC2_NI, Vu::Recompiler::QMTC2_I, Vu::Recompiler::LQC2, Vu::Recompiler::SQC2,
	//Vu::Recompiler::CALLMS, Vu::Recompiler::CALLMSR,
	
	// upper instructions //
	
	// 24
	Vu::Recompiler::ABS,
	Vu::Recompiler::ADD, Vu::Recompiler::ADDi, Vu::Recompiler::ADDq, Vu::Recompiler::ADDBCX, Vu::Recompiler::ADDBCY, Vu::Recompiler::ADDBCZ, Vu::Recompiler::ADDBCW,
	Vu::Recompiler::ADDA, Vu::Recompiler::ADDAi, Vu::Recompiler::ADDAq, Vu::Recompiler::ADDABCX, Vu::Recompiler::ADDABCY, Vu::Recompiler::ADDABCZ, Vu::Recompiler::ADDABCW,
	Vu::Recompiler::CLIP,
	Vu::Recompiler::FTOI0, Vu::Recompiler::FTOI4, Vu::Recompiler::FTOI12, Vu::Recompiler::FTOI15,
	Vu::Recompiler::ITOF0, Vu::Recompiler::ITOF4, Vu::Recompiler::ITOF12, Vu::Recompiler::ITOF15,
	
	// 26
	Vu::Recompiler::MADD, Vu::Recompiler::MADDi, Vu::Recompiler::MADDq, Vu::Recompiler::MADDBCX, Vu::Recompiler::MADDBCY, Vu::Recompiler::MADDBCZ, Vu::Recompiler::MADDBCW,
	Vu::Recompiler::MADDA, Vu::Recompiler::MADDAi, Vu::Recompiler::MADDAq, Vu::Recompiler::MADDABCX, Vu::Recompiler::MADDABCY, Vu::Recompiler::MADDABCZ, Vu::Recompiler::MADDABCW,
	Vu::Recompiler::MAX, Vu::Recompiler::MAXi, Vu::Recompiler::MAXBCX, Vu::Recompiler::MAXBCY, Vu::Recompiler::MAXBCZ, Vu::Recompiler::MAXBCW,
	Vu::Recompiler::MINI, Vu::Recompiler::MINIi, Vu::Recompiler::MINIBCX, Vu::Recompiler::MINIBCY, Vu::Recompiler::MINIBCZ, Vu::Recompiler::MINIBCW,
	
	Vu::Recompiler::MSUB, Vu::Recompiler::MSUBi, Vu::Recompiler::MSUBq, Vu::Recompiler::MSUBBCX, Vu::Recompiler::MSUBBCY, Vu::Recompiler::MSUBBCZ, Vu::Recompiler::MSUBBCW,
	Vu::Recompiler::MSUBA, Vu::Recompiler::MSUBAi, Vu::Recompiler::MSUBAq, Vu::Recompiler::MSUBABCX, Vu::Recompiler::MSUBABCY, Vu::Recompiler::MSUBABCZ, Vu::Recompiler::MSUBABCW,
	Vu::Recompiler::MUL, Vu::Recompiler::MULi, Vu::Recompiler::MULq, Vu::Recompiler::MULBCX, Vu::Recompiler::MULBCY, Vu::Recompiler::MULBCZ, Vu::Recompiler::MULBCW,
	Vu::Recompiler::MULA, Vu::Recompiler::MULAi, Vu::Recompiler::MULAq, Vu::Recompiler::MULABCX, Vu::Recompiler::MULABCY, Vu::Recompiler::MULABCZ, Vu::Recompiler::MULABCW,
	Vu::Recompiler::NOP, Vu::Recompiler::OPMSUB, Vu::Recompiler::OPMULA,
	Vu::Recompiler::SUB, Vu::Recompiler::SUBi, Vu::Recompiler::SUBq, Vu::Recompiler::SUBBCX, Vu::Recompiler::SUBBCY, Vu::Recompiler::SUBBCZ, Vu::Recompiler::SUBBCW,
	Vu::Recompiler::SUBA, Vu::Recompiler::SUBAi, Vu::Recompiler::SUBAq, Vu::Recompiler::SUBABCX, Vu::Recompiler::SUBABCY, Vu::Recompiler::SUBABCZ, Vu::Recompiler::SUBABCW,
	
	// lower instructions //
	
	Vu::Recompiler::DIV,
	Vu::Recompiler::IADD, Vu::Recompiler::IADDI, Vu::Recompiler::IAND,
	Vu::Recompiler::ILWR,
	Vu::Recompiler::IOR, Vu::Recompiler::ISUB,
	Vu::Recompiler::ISWR,
	Vu::Recompiler::LQD, Vu::Recompiler::LQI,
	Vu::Recompiler::MFIR, Vu::Recompiler::MOVE, Vu::Recompiler::MR32, Vu::Recompiler::MTIR,
	Vu::Recompiler::RGET, Vu::Recompiler::RINIT, Vu::Recompiler::RNEXT,
	Vu::Recompiler::RSQRT,
	Vu::Recompiler::RXOR,
	Vu::Recompiler::SQD, Vu::Recompiler::SQI,
	Vu::Recompiler::SQRT,
	Vu::Recompiler::WAITQ,

	// instructions not in macro mode //
	
	Vu::Recompiler::B, Vu::Recompiler::BAL,
	Vu::Recompiler::FCAND, Vu::Recompiler::FCEQ, Vu::Recompiler::FCGET, Vu::Recompiler::FCOR, Vu::Recompiler::FCSET,
	Vu::Recompiler::FMAND, Vu::Recompiler::FMEQ, Vu::Recompiler::FMOR,
	Vu::Recompiler::FSAND, Vu::Recompiler::FSEQ, Vu::Recompiler::FSOR, Vu::Recompiler::FSSET,
	Vu::Recompiler::IADDIU,
	Vu::Recompiler::IBEQ, Vu::Recompiler::IBGEZ, Vu::Recompiler::IBGTZ, Vu::Recompiler::IBLEZ, Vu::Recompiler::IBLTZ, Vu::Recompiler::IBNE,
	Vu::Recompiler::ILW,
	Vu::Recompiler::ISUBIU, Vu::Recompiler::ISW,
	Vu::Recompiler::JALR, Vu::Recompiler::JR,
	Vu::Recompiler::LQ,
	Vu::Recompiler::MFP,
	Vu::Recompiler::SQ,
	Vu::Recompiler::WAITP,
	Vu::Recompiler::XGKICK, Vu::Recompiler::XITOP, Vu::Recompiler::XTOP,

	// External Unit //

	Vu::Recompiler::EATAN, Vu::Recompiler::EATANxy, Vu::Recompiler::EATANxz, Vu::Recompiler::EEXP, Vu::Recompiler::ELENG, Vu::Recompiler::ERCPR, Vu::Recompiler::ERLENG, Vu::Recompiler::ERSADD,
	Vu::Recompiler::ERSQRT, Vu::Recompiler::ESADD, Vu::Recompiler::ESIN, Vu::Recompiler::ESQRT, Vu::Recompiler::ESUM
};
