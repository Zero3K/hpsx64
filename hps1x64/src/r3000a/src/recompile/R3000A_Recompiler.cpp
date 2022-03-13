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


#include "R3000A_Recompiler.h"
#include "ps1_system.h"

using namespace R3000A;



#ifdef PS2_COMPILE

//#define ENABLE_CPU_IDLE

/*

#define ENABLE_OPTIONAL_NEXTPC
*/

#endif


#define ENABLE_AUTO_BRANCH
#define ENABLE_CONNECT_ADJACENT_BLOCKS


#ifdef _DEBUG_VERSION_

#define INLINE_DEBUG_ENABLE

//#define INLINE_DEBUG_SPLIT


//#define INLINE_DEBUG_RECOMPILE
//#define INLINE_DEBUG_RECOMPILE2


#endif




#define ENABLE_R3000A_BRANCH_PENALTY




#define ENABLE_INLINE_STORE
#define ENABLE_INLINE_LOAD

#define USE_SHORT_SWL_CODE
#define USE_SHORT_SWR_CODE

#define USE_SHORT_LWL_CODE
#define USE_SHORT_LWR_CODE


#define EXECUTE_LOADS_IMMEDIATELY


#define ENABLE_MULTIPLY_LATENCY
#define ENABLE_DIVIDE_LATENCY



#define USE_NEW_LOAD_CODE
#define USE_NEW_STORE_CODE




#define USE_NEW_J_CODE
#define USE_NEW_JR_CODE
#define USE_NEW_JAL_CODE
#define USE_NEW_JALR_CODE


#define USE_NEW_BEQ_CODE
#define USE_NEW_BNE_CODE
#define USE_NEW_BLTZ_CODE
#define USE_NEW_BGTZ_CODE
#define USE_NEW_BLEZ_CODE
#define USE_NEW_BGEZ_CODE
#define USE_NEW_BLTZAL_CODE
#define USE_NEW_BGEZAL_CODE

#define USE_NEW_SYSCALL_CODE




#define USE_NEW_MULT_CODE
#define USE_NEW_MULTU_CODE
#define USE_NEW_DIV_CODE
#define USE_NEW_DIVU_CODE

#define USE_NEW_MFHI_CODE
#define USE_NEW_MFLO_CODE

#define USE_NEW_ADD_CODE
#define USE_NEW_ADDI_CODE
#define USE_NEW_SUB_CODE


#define USE_NEW_ADDU_CODE2
#define USE_NEW_SUBU_CODE2


#define USE_NEW_AND_CODE2
#define USE_NEW_OR_CODE2
#define USE_NEW_XOR_CODE2
#define USE_NEW_NOR_CODE2


#define USE_NEW_SLT_CODE2
#define USE_NEW_SLTU_CODE2


#define USE_NEW_ADDIU_CODE2
#define USE_NEW_ANDI_CODE2
#define USE_NEW_ORI_CODE2
#define USE_NEW_XORI_CODE2

#define USE_NEW_SLTI_CODE2
#define USE_NEW_SLTIU_CODE2


#define USE_NEW_LUI_CODE2


#define USE_NEW_SLL_CODE2
#define USE_NEW_SRL_CODE2
#define USE_NEW_SRA_CODE2

#define USE_NEW_SLLV_CODE2
#define USE_NEW_SRLV_CODE2
#define USE_NEW_SRAV_CODE2

#define USE_NEW_SB_CODE2
#define USE_NEW_SH_CODE2
#define USE_NEW_SW_CODE2
#define USE_NEW_SWR_CODE2
#define USE_NEW_SWL_CODE2
//#define USE_NEW_SWC2_CODE2



#define ALLOW_ENCODING_DELAYSLOT
#define ENCODE_ALL_POSSIBLE_DELAYSLOTS


#define CHECK_EVENT_AFTER_START


//#define ENABLE_SINGLE_STEP





// test pc arg pass, new methodology etc
//#define TEST_NEW_CODE


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


Debug::Log Recompiler::debug;

x64Encoder *Recompiler::e;
//ICache_Device *Recompiler::ICache;
Cpu *Recompiler::r;
s32 Recompiler::OpLevel;
u32 Recompiler::LocalPC;
u32 Recompiler::Local_LastModifiedReg;
u32 Recompiler::Local_NextPCModified;

u32 Recompiler::CurrentCount;

u32 Recompiler::isBranchDelaySlot;
u32 Recompiler::isLoadDelaySlot;

u32 Recompiler::bStopEncodingAfter;
u32 Recompiler::bStopEncodingBefore;

u32 Recompiler::RunCount;
u32 Recompiler::RunCount2;

//u32 Recompiler::Local_DelaySlot;
//u32 Recompiler::Local_DelayType;
//u32 Recompiler::Local_DelayCount;
//u32 Recompiler::Local_DelayCond;
//u32 Recompiler::Local_Condition;
Instruction::Format Recompiler::NextInst;

Recompiler::RDelaySlot Recompiler::RDelaySlots [ 2 ];
u32 Recompiler::DSIndex;
u32 Recompiler::RDelaySlots_Valid;

u64 Recompiler::MemCycles;

u64 Recompiler::LocalCycleCount;
u64 Recompiler::LocalCycleCount2;
u64 Recompiler::CacheBlock_CycleCount;

bool Recompiler::bIsBlockInICache;

u32 Recompiler::bResetCycleCount;


u32 Recompiler::CurrentBlock_StartAddress;
u32 Recompiler::NextBlock_StartAddress;

u32* Recompiler::pForwardBranchTargets;
u32 Recompiler::ForwardBranchIndex;

u8** Recompiler::pPrefix_CodeStart;
u8** Recompiler::pCodeStart;
u32* Recompiler::CycleCount;
u32* Recompiler::StartAddress;

u32 Recompiler::ulIndex_Mask;
u32 Recompiler::MaxStep;
u32 Recompiler::MaxStep_Shift;
u32 Recompiler::MaxStep_Mask;
u32 Recompiler::NumBlocks_Mask;

u32 Recompiler::StartBlockIndex;
u32 Recompiler::BlockIndex;



// multi-pass optimization vars //
u64 Recompiler::ullSrcRegBitmap;
u64 Recompiler::ullDstRegBitmap;

// the x64 registers that are currently allocated to MIPS registers
u64 Recompiler::ullTargetAlloc;

// the MIPS registers that are currently allocated to x64 registers
u64 Recompiler::ullSrcRegAlloc;

// the MIPS registers that are currently allocated to be constants
u64 Recompiler::ullSrcConstAlloc;

// the MIPS registers that have been modified and not written back yet
u64 Recompiler::ullSrcRegsModified;

// registers that are on the stack and need to be restored when done
u64 Recompiler::ullRegsOnStack;

// registers that are needed later on in the code
u64 Recompiler::ullNeededLater;

u64 Recompiler::ullSrcRegBitmaps [ 16 ];
u64 Recompiler::ullDstRegBitmaps [ 16 ];
u64 Recompiler::ullRegsStillNeeded [ 16 ];

// the actual data stored for the register, either a reg index or a constant value
u64 Recompiler::ullTargetData [ 32 ];


// lookup that indicates what register each target index corresponds to
const int Recompiler::iRegPriority [ 13 ] = { RAX, RDX, R8, R9, R10, R11, RBX, RSI, RDI, R12, R13, R14, R15 };

// lookup that indicates if the register requires you to save it on the stack before using it
// 0: no need to save on stack, 1: save and restore on stack
const int Recompiler::iRegStackSave [ 13 ] = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 };



u32 recompiler_r3000a_temp [ 4 ] __attribute__ ((aligned (16)));



// constructor
// NumberOfBlocks MUST be a power of 2, so 1 would mean 2, 2 would mean 4
Recompiler::Recompiler ( Cpu* R3000ACpu, u32 NumberOfBlocks, u32 BlockSize_PowerOfTwo, u32 MaxIStep_Shift )
{
#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create( "PS2_Recompiler_Log.txt" );
#endif

	
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
	pPrefix_CodeStart = new u8* [ NumBlocks * MaxStep ];
	
	
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
	
	//ICache = IC;
	r = R3000ACpu;
	
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
	memset ( pPrefix_CodeStart, 0x00, sizeof( u8* ) * NumBlocks * MaxStep );
	memset ( StartAddress, 0xff, sizeof( u32 ) * NumBlocks );
	memset ( pCodeStart, 0x00, sizeof( u8* ) * NumBlocks * MaxStep );
	memset ( CycleCount, 0x00, sizeof( u32 ) * NumBlocks * MaxStep );
	
	// reset invalidate arrays
	r->Bus->Reset_Invalidate ();
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
	}
	
	
	
	// will leave out all store instructions for now to play it safe //
	
#else
	if ( !ulInstruction ) return true;
#endif
	
	return false;
}


u32 Recompiler::CloseOpLevel ( u32 OptLevel, u32 Address )
{
	switch  ( OptLevel )
	{
		case 0:
			break;
			
		case 1:
			// write back last modified register if in load delay slot
			//if ( isLoadDelaySlot )
			//{
				e->MovMemImm32 ( (long*) &r->LastModifiedRegister, Local_LastModifiedReg );
			//}
			
			// write back "NextPC" if there was no SYSCALL
			if ( !Local_NextPCModified )
			{
				e->MovMemImm32 ( (long*) &r->NextPC, Address );
			}
			break;
			
		case 2:
			// write back last modified register if in load delay slot
			//if ( isLoadDelaySlot )
			//{
				e->MovMemImm32 ( (long*) &r->LastModifiedRegister, Local_LastModifiedReg );
			//}
			
			// write back "NextPC" if there was no SYSCALL
			if ( !Local_NextPCModified )
			{
				e->MovMemImm32 ( (long*) &r->NextPC, Address );
			}
			break;
	}

	return true;
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



u32* Recompiler::RGetPointer ( u32 Address )
{

	if ( ICache_Device::isCached ( Address ) )
	{
		// address is cached //
		
		// check if cache line has address
		if ( r->ICache.ICacheBlockSource [ ( Address >> 4 ) & 0xff ] == ( Address & 0x1ffffff0 ) )
		{
#ifdef VERBOSE_RECOMPILE
cout << "\nRGetPointer: CACHED";
#endif

			// address is already cached, so get pointer from cache
			return & r->ICache.ICacheData [ ( Address >> 2 ) & 0x3ff ];
		}
	}
	
#ifdef VERBOSE_RECOMPILE
cout << "\nRGetPointer: NON-CACHED";
#endif

	// address is NOT cache-able //
	
	if ( ( Address & 0x1fc00000 ) == 0x1fc00000 )
	{
		// cached bios region //
		return & r->Bus->BIOS.b32 [ ( Address & r->Bus->BIOS_Mask ) >> 2 ];
	}
	
	// cached ram region //
	return & r->Bus->MainMemory.b32 [ ( Address & r->Bus->MainMemory_Mask ) >> 2 ];
}


// returns the bitmap for the source registers for instruction
// if the instruction is not supported, then it will return -1ULL
u64 Recompiler::GetSourceRegs ( Instruction::Format i, u32 Address )
{
	/*
	if ( !i.Value )
	{
		return 0;
	}
	*/
	
	// "special"
	if ( !i.Opcode )
	{
		return ( ( 1ULL << i.Rs ) | ( 1ULL << i.Rt ) );
	}
	
	// regimm
	if ( i.Opcode == 1 )
	{
		// rs is source reg //
		
		return ( 1ULL << i.Rs );
	}
	
	// j, jal
	if ( i.Opcode <= 3 )
	{
		return 0;
	}
	
	// beq, bne, blez, bgtz
	if ( ( i.Opcode >> 3 ) == 0 )
	{
		return ( ( 1ULL << i.Rs ) | ( 1ULL << i.Rt ) );
	}
	
	// immediates
	if ( ( i.Opcode >> 3 ) == 1 )
	{
		return ( 1ULL << i.Rs );
	}
	
	// stores
	if ( ( i.Opcode >> 3 ) == 5 )
	{
		return 0;
	}
	
	// loads
	if ( ( i.Opcode >> 3 ) == 4 )
	{
		return ( 1ULL << i.Rs );
	}
	
	return -1ULL;
	
	/*
	// check for "special"
	if ( !i.Opcode )
	{
		// rs,rt are source regs //
		
		// not including syscall or break, but these shouldn't cause problems
		if ( ( i.Funct == 12 ) || ( i.Funct == 13 ) )
		{
			return 0;
		}
		
		//if ( ( i.Funct >> 3 ) == 0 )
		//{
		//	return -1;
		//}
		
		return ( ( 1ULL << i.Rs ) | ( 1ULL << i.Rt ) );
	}
	
	// stores are cleared to go (runs load delay before the store)
	if ( ( i.Opcode >> 3 ) == 5 )
	{
		return 0;
	}
	
	// check for regimm, immediates, loads
	if ( ( i.Opcode == 1 ) || ( ( i.Opcode >> 3 ) == 1 ) || ( ( i.Opcode >> 3 ) == 4 ) )
	{
		// rs is source reg //
		
		return ( 1ULL << i.Rs );
	}
	
	// any other instructions are not cleared to go
	return -1ULL;
	*/
}


// returns the bitmap for the destination registers for instruction
// if the instruction is not supported, then it will return -1ULL
u64 Recompiler::Get_DelaySlot_DestRegs ( Instruction::Format i )
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


/*
static u64 Recompiler::ReturnZero ( void )
{
	return 0;
}


static u64 Recompiler::ReturnOne ( void )
{
	return 1;
}


static u64 Recompiler::ReturnTwo ( void )
{
	return 2;
}
*/


// returns -1 if there was a problem, otherwise returns number of instructions recompiled
// level 2 recompiler
// returns the next address after level 2 block, returns -1 on error
u32 Recompiler::Recompile2 ( u32 ulBeginAddress )
{
	// the index used for looping
	int iIdx;
	int iRegIdx;
	
	// maximum number of level2 recompiled instructions in this run
	u32 ulMaxRun2;
	
	// the real number of instructions in the level 2 recompilation run
	u32 ulRealRun2;
	
	// the current address
	u32 ulAddress;
	
	Instruction::Format inst;
	u32* pSrcCodePtr;
	
	int iRet;
	
	u64 ullCombineBitmap;
	u64 ullCurAlloc;
	u64 ullBitmap;
	
	// this depends on the size of the cache blocks for the processor
	ulMaxRun2 = MaxStep - ( ( ulBeginAddress >> MaxStep_Shift ) & MaxStep_Mask );
	
	// first go through and get the source and destination registers //
	
	// start from the begin address
	ulAddress = ulBeginAddress;
	
	// get the pointer into the instructions
	// this should get pointer to the instruction
	pSrcCodePtr = RGetPointer ( ulAddress );
	
	// set oplevel to preprocess info
	OpLevel = -1;

#ifdef VERBOSE_RECOMPILE
	cout << "\nRecompile2: starting pass1.";
#endif
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\nRECOMPILE2: Starting pass1.";
#endif
	
	for ( iIdx = 0; iIdx < ulMaxRun2; iIdx++ )
	{
		// get the instruction
		inst.Value = *pSrcCodePtr++;
		
		// init src and dst reg bitmaps
		ullSrcRegBitmap = 0;
		ullDstRegBitmap = 0;
		
#ifdef VERBOSE_RECOMPILE
cout << "\n ASM2: " << Print::PrintInstruction ( inst.Value ).c_str ();
#endif
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\n ASM2: " << Print::PrintInstruction ( inst.Value ).c_str ();
#endif

		// get the source and dest register bitmaps
		iRet = Recompile ( inst, ulAddress );

#ifdef VERBOSE_RECOMPILE
cout << " iRet=" << dec << iRet;
#endif
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << " iRet=" << dec << iRet;
#endif
		
		if ( iRet < 0 ) break;

#ifdef VERBOSE_RECOMPILE
cout << " SrcBmp=" << hex << ullSrcRegBitmap;
cout << " DstBmp=" << hex << ullDstRegBitmap;
#endif
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << " SrcBmp=" << hex << ullSrcRegBitmap;
	debug << " DstBmp=" << hex << ullDstRegBitmap;
#endif
		
		// store the bitmap data for this instruction
		ullSrcRegBitmaps [ iIdx ] = ullSrcRegBitmap;
		ullDstRegBitmaps [ iIdx ] = ullDstRegBitmap;
	}
	
	// set the actual number of instructions to be recompiled at level 2
	ulRealRun2 = iIdx;
	
	// if fewer than 2 instructions can be recompiled, there's no benefit
	if ( ulRealRun2 < 2 )
	{
		// before returning, set oplevel back to what it was
		OpLevel = 2;
		
		return -1;
	}
	
	ullCombineBitmap = 0;
	
	// fill in the bitmap for registers that are needed
	for ( iIdx = ulRealRun2 - 1; iIdx >= 0; iIdx-- )
	{
		ullRegsStillNeeded [ iIdx ] = ullCombineBitmap;
		
		ullCombineBitmap |= ullSrcRegBitmaps [ iIdx ];
	}
	
	
	// start from the begin address
	ulAddress = ulBeginAddress;
	
	// get the pointer into the instructions
	// this should get pointer to the instruction
	pSrcCodePtr = RGetPointer ( ulAddress );
	
	// set oplevel to encode the actual instruction
	OpLevel = 2;
	
	// init bitmaps for allocation
	ullTargetAlloc = 0;
	ullSrcRegAlloc = 0;
	ullSrcConstAlloc = 0;
	
	// set r0 as a constant zero
	Alloc_Const ( 0, 0 );
	
	// now clear regs modified, including r0 modified bit
	ullSrcRegsModified = 0;
	
	// clear any regs on stack
	ullRegsOnStack = 0;


#ifdef VERBOSE_RECOMPILE
	cout << "\nRecompile2: starting pass2.";
#endif
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\nRecompile2: starting pass2.";
#endif
	
	// encode run //
	
	LocalCycleCount2 = LocalCycleCount;
	
	RunCount2 = 0;
	
	// encode the instructions ??
	for ( iIdx = 0; iIdx < ulRealRun2; iIdx++ )
	{
		// get the instruction
		inst.Value = *pSrcCodePtr++;
		
		// set the registers that are still needed later
		ullNeededLater = ullRegsStillNeeded [ iIdx ];
		
		// set the source and dest registers on source machine
		ullSrcRegBitmap = ullSrcRegBitmaps [ iIdx ];
		ullDstRegBitmap = ullDstRegBitmaps [ iIdx ];
		
#ifdef VERBOSE_RECOMPILE
cout << "\n ASM2: " << Print::PrintInstruction ( inst.Value ).c_str ();
#endif
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\n ASM2: " << Print::PrintInstruction ( inst.Value ).c_str ();
	debug << " ADDR:" << hex << ulAddress;
#endif

		// encode the instruction
		iRet = Recompile ( inst, ulAddress );

#ifdef VERBOSE_RECOMPILE
cout << " iRet=" << dec << iRet;
#endif
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << " iRet=" << dec << iRet;
#endif
		
		if ( iRet <= 0 )
		{
			break;
		}
		
		// automatically remove register allocations that are no longer needed (except r0)
		// *** todo ***
		ullCurAlloc = ( ullSrcRegAlloc ) & ~1;
		while ( ullCurAlloc & ~( ullNeededLater ) )
		{
			// get the next register that is no longer needed
			ullBitmap = ullCurAlloc & ~( ullNeededLater );
			ullBitmap &= -ullBitmap;
			
			// get its index
			iRegIdx = __builtin_ctz( ullBitmap );
			
			// remove it and remove from bitmap
			DisposeReg( iRegIdx );
			ullCurAlloc &= ~( 1ull << iRegIdx );
		}
		
		// update to the next address
		ulAddress += 4;
		
		// used by load/store/etc
		RunCount2++;
		
		// update cycles
		LocalCycleCount2 += MemCycles;
	}
	

	// if fewer than 2 instructions can be recompiled, there's no benefit
	if ( iIdx < 2 )
	{
		// before returning, set oplevel back to what it was
		OpLevel = 2;
		
		return -1;
	}

#ifdef VERBOSE_RECOMPILE
cout << "\n Writing back modified regs";
#endif
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\n Writing back modified regs";
#endif

	// write back all registers
	WriteBackModifiedRegs ();

#ifdef VERBOSE_RECOMPILE
cout << "\n Restoring regs from stack";
#endif
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\n Restoring regs from stack";
#endif
	
	// restore regs from stack
	RestoreRegsFromStack ();
	
	
	return iIdx;
}


// dispose the old register but reassign the new register to the same register on target device
// returns -1 on error, otherwise returns the register on target device
int Recompiler::RenameReg ( int iNewSrcRegIdx, int iOldSrcRegIdx )
{
	int iIdx;
	int iRegIdx;
	
	// if the same register, then return the register on target device
	if ( iNewSrcRegIdx == iOldSrcRegIdx )
	{
		// make sure it is a register
		if ( ullSrcRegAlloc & ( 1ull << iOldSrcRegIdx ) )
		{
			// if register has been renamed, then it has been modified ??
			ullSrcRegsModified |= ( 1ull << iNewSrcRegIdx );
			
			// return the actual register on target device
			iIdx = ullTargetData [ iOldSrcRegIdx ];
			return iRegPriority [ iIdx ];
		}
		else
		{
			// error ??
			return -1;
		}
	}
	
	// deallocate the old source register assigned to the target device register
	iIdx = DisposeReg ( iOldSrcRegIdx );
	
	// new source register is not a constant
	ullSrcConstAlloc &= ~( 1ull << iNewSrcRegIdx );
	
	// assign that register to the new source device register
	ullTargetData [ iNewSrcRegIdx ] = iIdx;
	ullSrcRegAlloc |= ( 1ull << iNewSrcRegIdx );
	ullTargetAlloc |= ( 1ull << iIdx );
	
	// if register has been renamed, then it has been modified ??
	ullSrcRegsModified |= ( 1ull << iNewSrcRegIdx );
	
	// return the actual register on the target device
	return iRegPriority [ iIdx ];
}



// returns -1 on error, returns id of register (not the actual register) on target device otherwise
int Recompiler::DisposeReg ( int iSrcRegIdx )
{
	int iRealRegIdx;
	
	// check that register id is valid
	if ( iSrcRegIdx < 0 )
	{
		return -1;
	}
	
	// check if register is allocated
	if ( ! isAlloc( iSrcRegIdx ) )
	{
		// register not allocated, so nothing to do
		return -1;
	}
	
	// check if register is modified (means needs write back)
	if ( ullSrcRegsModified & ( 1ull << iSrcRegIdx ) )
	{
		// check if constant or not
		if ( isConst( iSrcRegIdx ) )
		{
			// *** todo *** write back constant
			e->MovMemImm32 ( &r->GPR [ iSrcRegIdx ].s, (u32) ullTargetData [ iSrcRegIdx ] );
		}
		else
		{
			// *** todo *** write back register
			iRealRegIdx = iRegPriority [ ullTargetData [ iSrcRegIdx ] ];
			e->MovMemReg32 ( &r->GPR [ iSrcRegIdx ].s, iRealRegIdx );
		}
	}
	
	// clear modified bitmap
	ullSrcRegsModified &= ~( 1ull << iSrcRegIdx );
	
	// check if register
	if ( isReg( iSrcRegIdx ) )
	{
		// clear target bitmap
		ullTargetAlloc &= ~( 1ull << ( ullTargetData [ iSrcRegIdx ] ) );
	}
	
	// clear reg bitmap
	ullSrcRegAlloc &= ~( 1ull << iSrcRegIdx );
	
	// clear const bitmap
	ullSrcConstAlloc &= ~( 1ull << iSrcRegIdx );
	
	// return the old target register id
	return ullTargetData [ iSrcRegIdx ];
}

bool Recompiler::isAlloc ( int iSrcRegIdx )
{
	if ( ( ullSrcConstAlloc | ullSrcRegAlloc ) & ( 1ull << iSrcRegIdx ) )
	{
		return true;
	}
	
	return false;
}

bool Recompiler::isConst ( int iSrcRegIdx )
{
	// check if register is a constant
	if ( ullSrcConstAlloc & ( 1ull << iSrcRegIdx ) )
	{
		return true;
	}
	
	return false;
}

bool Recompiler::isReg ( int iSrcRegIdx )
{
	if ( ullSrcRegAlloc & ( 1ull << iSrcRegIdx ) )
	{
		return true;
	}
	
	return false;
}

bool Recompiler::isDisposable( int iSrcRegIdx )
{
	if ( ullNeededLater & ( 1ull << iSrcRegIdx ) )
	{
		return false;
	}
	else
	{
		return true;
	}
}


bool Recompiler::isBothAlloc ( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	
	if ( ( ( ullSrcConstAlloc | ullSrcRegAlloc ) & ullBitmap ) == ullBitmap )
	{
		return true;
	}
	
	return false;
}

bool Recompiler::isBothConst ( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	
	if ( ( ullSrcConstAlloc & ullBitmap ) == ullBitmap )
	{
		return true;
	}
	
	return false;
}

bool Recompiler::isBothReg ( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	
	if ( ( ullSrcRegAlloc & ullBitmap ) == ullBitmap )
	{
		return true;
	}
	
	return false;
}

bool Recompiler::isBothDisposable( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	
	if ( ! ( ullNeededLater & ullBitmap ) )
	{
		return false;
	}
	else
	{
		return true;
	}
}


// ---------------------------------------------

bool Recompiler::isEitherAlloc ( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	
	if ( ( ullSrcConstAlloc | ullSrcRegAlloc ) & ullBitmap )
	{
		return true;
	}
	
	return false;
}

bool Recompiler::isEitherConst ( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	
	if ( ullSrcConstAlloc & ullBitmap )
	{
		return true;
	}
	
	return false;
}

bool Recompiler::isEitherReg ( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	
	if ( ullSrcRegAlloc & ullBitmap )
	{
		return true;
	}
	
	return false;
}

bool Recompiler::isEitherDisposable( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	
	if ( ( ullNeededLater & ullBitmap ) == ullBitmap )
	{
		return false;
	}
	
	return true;
}



int Recompiler::SelectAlloc ( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	ullBitmap &= ( ullSrcRegAlloc | ullSrcConstAlloc );
	ullBitmap &= -ullBitmap;
	return __builtin_ctz( ullBitmap );
}

int Recompiler::SelectConst ( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	ullBitmap &= ullSrcConstAlloc;
	ullBitmap &= -ullBitmap;
	return __builtin_ctz( ullBitmap );
}

int Recompiler::SelectReg ( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	ullBitmap &= ullSrcRegAlloc;
	ullBitmap &= -ullBitmap;
	return __builtin_ctz( ullBitmap );
}

int Recompiler::SelectDisposable( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	ullBitmap &= ~ullNeededLater;
	ullBitmap &= -ullBitmap;
	return __builtin_ctz( ullBitmap );
}


int Recompiler::SelectNotAlloc ( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	ullBitmap &= ~( ullSrcRegAlloc | ullSrcConstAlloc );
	ullBitmap &= -ullBitmap;
	return __builtin_ctz( ullBitmap );
}

int Recompiler::SelectNotDisposable( int iSrcRegIdx1, int iSrcRegIdx2 )
{
	u64 ullBitmap;
	ullBitmap = ( 1ull << iSrcRegIdx1 ) | ( 1ull << iSrcRegIdx2 );
	ullBitmap &= ullNeededLater;
	ullBitmap &= -ullBitmap;
	return __builtin_ctz( ullBitmap );
}



// returns -1 if error, otherwise returns index of register on target platform
// iSrcRegIdx: index of register on source platform (for example, MIPS)
int Recompiler::Alloc_SrcReg ( int iSrcRegIdx )
{
	u64 ullBitmap;
	int iIdx, iRegIdx;
	
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\nSTART->Recompiler::Alloc_SrcReg";
#endif

	// make sure register id is valid
	if ( iSrcRegIdx < 0 )
	{
		return -1;
	}
	
	// check if register is a constant
	if ( ullSrcConstAlloc & ( 1ull << iSrcRegIdx ) )
	{
		return -1;
	}
	
	// check if register is already allocated
	if ( ullSrcRegAlloc & ( 1ull << iSrcRegIdx ) )
	{
		// return the index or register on the target platform
		//__builtin_ctz ();
		return iRegPriority [ ullTargetData [ iSrcRegIdx ] ];
	}
	
	// get the next available register on target //
	
	// there are currently 13 available registers, so make sure there is one available
	if ( ( ullTargetAlloc & 0x1fff ) == 0x1fff )
	{
		// no more registers available
		return -1;
	}
	
	// get the next target register available
	ullBitmap = ~ullTargetAlloc;
	ullBitmap &= -ullBitmap;
	
	// get the index it is allocated to on target
	// *** gcc specific code *** //
	iIdx = __builtin_ctz( ullBitmap );
	
	// set that register as allocated to a variable reg
	ullSrcRegAlloc |= ( 1ull << iSrcRegIdx );
	
	// .. not a constant
	ullSrcConstAlloc &= ~( 1ull << iSrcRegIdx );
	
	// and allocated on target
	ullTargetAlloc |= ullBitmap;
	
	// set the index it is allocated to on target
	ullTargetData [ iSrcRegIdx ] = iIdx;
	
	// get the actual register to use from the index
	iRegIdx = iRegPriority [ iIdx ];
	
	// if the register needs to be saved, push it onto stack unless it is already saved
	if ( iRegStackSave [ iIdx ] )
	{
		if ( ! ( ullRegsOnStack & ( 1ull << iIdx ) ) )
		{
			// push the register onto the stack //
			e->PushReg64( iRegIdx );
			
			// mark register as being on the stack
			ullRegsOnStack |= ( 1ull << iIdx );
		}
	}
	
	// load in the source register (only for Alloc_SrcReg)
	e->MovRegMem32 ( iRegIdx, & r->GPR [ iSrcRegIdx ].s );
	
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << " x64Reg#" << dec << iRegIdx << " -> " << " MIPSReg#" << dec << iSrcRegIdx;
#endif
	
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\nEND->Recompiler::Alloc_SrcReg";
	debug << " return=" << dec << iRegIdx;
#endif

	// done
	return iRegIdx;
}


int Recompiler::Alloc_DstReg ( int iSrcRegIdx )
{
	u64 ullBitmap;
	int iIdx, iRegIdx;
	
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\nSTART->Recompiler::Alloc_DstReg";
	debug << " ullTargetAlloc=" << hex << ullTargetAlloc;
#endif

	// make sure register id is valid
	if ( iSrcRegIdx < 0 )
	{
		return -1;
	}
	
	// check if register is a constant
	//if ( ullSrcConstAlloc & ( 1 << iSrcRegIdx ) )
	//{
	//	return -1;
	//}
	
	// check if register is already allocated
	if ( ullSrcRegAlloc & ( 1ull << iSrcRegIdx ) )
	{
		// register is already allocated as a register //
		
		// make sure register is set as being modified (might have been a source reg previously)
		ullSrcRegsModified |= ( 1ull << iSrcRegIdx );
		
		// return the index or register on the target platform
		//__builtin_ctz ();
		return iRegPriority [ ullTargetData [ iSrcRegIdx ] ];
	}
	
	// get the next available register on target //
	
	// there are currently 13 available registers, so make sure there is one available
	if ( ( ullTargetAlloc & 0x1fff ) == 0x1fff )
	{
		// no more registers available
		return -1;
	}
	
	// get the next target register available
	ullBitmap = ~ullTargetAlloc;
	ullBitmap &= -ullBitmap;
	
	// get the index it is allocated to on target
	// *** gcc specific code *** //
	iIdx = __builtin_ctz( ullBitmap );
	
	// set the register as modified, since it is a destination register
	ullSrcRegsModified |= ( 1ull << iSrcRegIdx );
	
	// set that register as allocated to a variable reg
	ullSrcRegAlloc |= ( 1ull << iSrcRegIdx );
	
	// .. not a constant
	ullSrcConstAlloc &= ~( 1ull << iSrcRegIdx );
	
	// and allocated on target
	ullTargetAlloc |= ullBitmap;
	
	// set the index it is allocated to on target
	ullTargetData [ iSrcRegIdx ] = iIdx;
		
	// get the actual register to use from the index
	iRegIdx = iRegPriority [ iIdx ];
	
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << " x64Reg#" << dec << iRegIdx << " -> " << "MIPSReg#" << iSrcRegIdx;
#endif

	// if the register needs to be saved, push it onto stack unless it is already saved
	if ( iRegStackSave [ iIdx ] )
	{
		if ( ! ( ullRegsOnStack & ( 1ull << iIdx ) ) )
		{
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << " SAVING-ONSTACK x64Reg#" << dec << iRegIdx;
#endif

			// push the register onto the stack //
			e->PushReg64( iRegIdx );
			
			// mark register as being on the stack
			ullRegsOnStack |= ( 1ull << iIdx );
		}
	}
	
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\nEND->Recompiler::Alloc_DstReg";
#endif

	// done
	return iRegIdx;
}


// returns -1 if error, otherwise returns 1
// iSrcRegIdx: index of register on source platform (for example, MIPS)
int Recompiler::Alloc_Const ( int iSrcRegIdx, u64 ullValue )
{
	u64 ullBitmap;
	int iIdx, iRegIdx;
	
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\nSTART->Recompiler::Alloc_Const";
#endif

	// if reg is allocated, then deallocate it
	// check if register is already allocated
	if ( ullSrcRegAlloc & ( 1ull << iSrcRegIdx ) )
	{
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << " Reg#" << dec << iSrcRegIdx << " already allocated as REGISTER. Deallocating.";
#endif
		// remove from allocation on target
		// get index of register on target
		iIdx = ullTargetData [ iSrcRegIdx ];
		
		// remove from target reg bitmap
		ullTargetAlloc &= ~( 1ull << iIdx );
		
		// clear the bit in source reg bitmap
		ullSrcRegAlloc &= ~( 1ull << iSrcRegIdx );
	}

#ifdef INLINE_DEBUG_RECOMPILE2
	debug << " Allocating Const#" << dec << iSrcRegIdx << " with value: " << hex << ullValue;
#endif
	
	// set register as a constant
	ullSrcConstAlloc |= ( 1ull << iSrcRegIdx );
	
	// set the constants as modified on source device (they need to be written back)
	ullSrcRegsModified |= ( 1ull << iSrcRegIdx );
	
	// set the value of the constant
	ullTargetData [ iSrcRegIdx ] = ullValue;

#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\nEND->Recompiler::Alloc_Const";
	debug << " return=" << dec << iSrcRegIdx;
#endif
	
	return iSrcRegIdx;
}

// write back any constants and registers that were modified in the run
void Recompiler::WriteBackModifiedRegs ()
{
	u64 ullBitmap;
	int iSrcRegIdx;
	int iRegIdx;
	
	u64 ullSrcRegsModified2;
	
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\nSTART->Recompiler::WriteBackModifiedRegs";
#endif
	
	ullSrcRegsModified2 = ullSrcRegsModified;
	
	// loop while there are source device registers (and/or constants) that need to be written back
	while ( ullSrcRegsModified2 )
	{
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\nullSrcRegsModified2=" << hex << ullSrcRegsModified2;
#endif

		ullBitmap = ullSrcRegsModified2 & -ullSrcRegsModified2;
		
		// get the next register on source device
		iSrcRegIdx = __builtin_ctz( ullBitmap );

#ifdef INLINE_DEBUG_RECOMPILE2
	debug << " iSrcRegIdx=" << dec << iSrcRegIdx;
#endif
		
		// check if it is a register or constant
		if ( ullSrcConstAlloc & ( 1ull << iSrcRegIdx ) )
		{
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << " Writing back Constant#" << dec << iSrcRegIdx;
#endif

			// write back constant //
			e->MovMemImm32 ( & r->GPR[ iSrcRegIdx ].s, (u32) ullTargetData [ iSrcRegIdx ] );
		}
		else if ( ullSrcRegAlloc & ( 1ull << iSrcRegIdx ) )
		{
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << " Writing back Register#" << dec << iSrcRegIdx;
#endif

			// write back register //
			iRegIdx = iRegPriority [ ullTargetData [ iSrcRegIdx ] ];
			e->MovMemReg32 ( & r->GPR [ iSrcRegIdx ].s, iRegIdx );
		}
		
		// remove from bitmaps
		ullSrcRegsModified2 &= ~ullBitmap;
		//ullSrcConstAlloc &= ~ullBitmap;
		//ullSrcRegAlloc &= ~ullBitmap;
	}
	
	//ullSrcRegsModified = 0;
	//ullSrcConstAlloc = 1;
	//ullSrcRegAlloc = 0;
	
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\nEND->Recompiler::WriteBackModifiedRegs";
#endif
}

// restore any registers that were saved on the stack to process the run
void Recompiler::RestoreRegsFromStack ()
{
	u64 ullBitmap;
	int iRegIdx;
	int iIdx;
	
	u64 ullRegsOnStack2;
	
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\nSTART->Recompiler::RestoreRegsFromStack";
#endif

	ullRegsOnStack2 = ullRegsOnStack;

	// loop while there are still registers on the stack
	while ( ullRegsOnStack2 )
	{
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\nullRegsOnStack2=" << hex << ullRegsOnStack2;
#endif

		// get the next register
		//ullBitmap = ullRegsOnStack2 & -ullRegsOnStack2;
		
		// get it's target device index
		//iRegIdx = __builtin_ctz( ullBitmap );
		iRegIdx = __builtin_clz( ullRegsOnStack2 );
		iRegIdx = 31 - iRegIdx;
		

		iIdx = iRegPriority [ iRegIdx ];

#ifdef INLINE_DEBUG_RECOMPILE2
	debug << " Restoring x64REG#" << dec << iIdx;
#endif

		// pop register from the stack
		e->PopReg64( iIdx );
		
		// remove register from bitmap
		//ullRegsOnStack2 &= ~ullBitmap;
		ullRegsOnStack2 &= ~( 1 << iRegIdx );
	}
	
	//ullRegsOnStack = 0;
	
#ifdef INLINE_DEBUG_RECOMPILE2
	debug << "\r\nEND->Recompiler::RestoreRegsFromStack";
#endif
}






// returns number of instructions that were recompiled
u32 Recompiler::Recompile ( u32 BeginAddress )
{
	u32 Address, Block;
	s32 ret, Cycles;
	Instruction::Format inst;
	s32 reti;
	
	//u32 StartBlockIndex, BlockIndex, SaveBlockIndex;
	
	// number of instructions in current run
	//u32 RunCount;
	
	int iFillIndex = 0;
	
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
#ifdef INLINE_DEBUG_RECOMPILE
	debug << "\r\nSTART->Recompiler::Recompile";
	debug << " BeginADDR=" << hex << BeginAddress;
#endif


	// need to first clear forward branch targets for the block
	memset ( pForwardBranchTargets, 0x00, sizeof( u32 ) * MaxStep );
	
	// initialize forward branch index
	// note: Will need a larger branch index table in the encoder object for larger code blocks than 128 instructions
	ForwardBranchIndex = c_ulForwardBranchIndex_Start;


	
	
	// set the encoder to use
	e = InstanceEncoder;
	
	// the starting address needs to be on a block boundary
	BeginAddress = ( BeginAddress >> ( 2 + MaxStep_Shift ) ) << ( 2 + MaxStep_Shift );
	
	// save the address?
	Address = BeginAddress;
	
	// set the start address for the current block so recompiler can access it
	CurrentBlock_StartAddress = BeginAddress;
	
	// set the start address for the next block also
	NextBlock_StartAddress = CurrentBlock_StartAddress + ( 1 << ( 2 + MaxStep_Shift ) );
	
	
	// get the block to encode in
	Block = ( BeginAddress >> ( 2 + MaxStep_Shift ) ) & NumBlocks_Mask;
	
	
	
	// start in code block
	e->StartCodeBlock ( Block );

	
	// set the start address for code block
	// address must actually match exactly. No mask
	StartAddress [ Block ] = BeginAddress;
	




	
	
	

	

	
	/////////////////////////////////////////////////////
	// note: multiply and divide require cycle count to be updated first
	// since they take more than one cycle to complete
	// same for mfhi and mflo, because they are interlocked
	// same for COP2 instructions
	// same for load and store
	// do the same for jumps and branches
	//////////////////////////////////////////////////////

	
	
	// get the starting block to store instruction addresses and cycle counts
	StartBlockIndex = ( BeginAddress >> 2 ) & ulIndex_Mask;
	BlockIndex = StartBlockIndex;

	// zero out code start addresses
	for ( i = 0; i < MaxStep; i++ )
	{
		pCodeStart [ StartBlockIndex + i ] = NULL;
	}
	
	
	// get the cycles per instruction
	if ( ICache_Device::isCached ( Address ) )
	{
		// address is cached //
		bIsBlockInICache = true;
		
		// one cycle to execute each instruction (unless reloading cache block)
		MemCycles = 1;
		//MemCycles = 0;
	}
	else
	{
		// address is NOT cache-able //
		bIsBlockInICache = false;
		
		// time to execute the instruction starts with the time to read it from memory
		if ( ( Address & 0x1fc00000 ) == 0x1fc00000 )
		{
			// bios region //
			MemCycles = DataBus::c_iBIOS_Read_Latency;
		}
		else
		{
			// ram region //
			MemCycles = DataBus::c_iRAM_Read_Latency;
		}
		
		// should be plus 1 like in the interpreter
		MemCycles += 1;
	}

#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiler: Starting starting";
#endif
#ifdef INLINE_DEBUG_RECOMPILE
	debug << "\r\nRecompiler: Starting starting";
#endif
	
	for( iFillIndex = 0; iFillIndex < MaxStep; iFillIndex++ )
	{

#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiler: iFillIndex=" << dec << iFillIndex << " MaxStep=" << MaxStep;
#endif
#ifdef INLINE_DEBUG_RECOMPILE
	debug << "\r\nRecompiler: iFillIndex=" << dec << iFillIndex << " MaxStep=" << MaxStep;
#endif

	if ( !pCodeStart [ StartBlockIndex + iFillIndex ] )
	{
#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiler: !pCodeStart";
#endif
#ifdef INLINE_DEBUG_RECOMPILE
	debug << "\r\nRecompiler: !pCodeStart";
#endif

	// need to keep track of cycles for run
	LocalCycleCount = 0;

	// need to know of any other jumps to return
	RetJumpCounter = 0;
	

	// instruction count for current run
	RunCount = 0;
	
	// current delay slot index
	DSIndex = 0;
	
	
	// save the address?
	//Address = BeginAddress;
	Address = BeginAddress + ( iFillIndex << 2 );
	
	// set the current optimization level
	OpLevel = OptimizeLevel;
	
	// haven't crossed any cache lines yet
	ulCacheLineCount = 0;
	
	// start cycles at zero
	Cycles = 0;
	
	
	
	// init count of recompiled instructions
	RecompileCount = 0;
	
	
	// want to stop at cache boundaries (would need extra code there anyways)
	// this is handled in loop now
	// set the maximum number of instructions to encode
	//MaxCount = MaxStep;
	MaxCount = MaxStep - iFillIndex;
	
	BlockIndex = StartBlockIndex + iFillIndex;
	
	// NextPC has not been modified yet
	Local_NextPCModified = false;
	
	// some instructions need to stop encoding either before or after the instruction, at least for now
	// if stopping before, it keeps the instruction if there is nothing before it in the run
	bStopEncodingAfter = false;
	bStopEncodingBefore = false;
	
	// don't reset the cycle count yet
	bResetCycleCount = false;


	
	// should set local last modified register to 255
	Local_LastModifiedReg = 255;
	
	reti = 1;
	
	
	// this should get pointer to the instruction
	pSrcCodePtr = RGetPointer ( Address );
	
#ifdef INLINE_DEBUG_RECOMPILE
	debug << " START-ADDR=" << hex << Address;
	debug << " MaxCount=" << dec << MaxCount;
#endif
	
#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiler: Starting loop";
#endif
#ifdef INLINE_DEBUG_RECOMPILE
	debug << "\r\nRecompiler: Starting loop";
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
	//for ( i = 0; i < MaxCount; i++ )
	for ( i = iFillIndex; i < MaxStep; i++ )
	{
#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiling: ADDR=" << hex << Address;
#endif
#ifdef INLINE_DEBUG_RECOMPILE
	debug << "\r\nRecompiling: ADDR=" << hex << Address;
	debug << " i=" << dec << i;
#endif


#ifdef INLINE_DEBUG_RECOMPILE
	debug << " START-INSTRUCTION-BLOCK";
#endif
		
		// start encoding a MIPS instruction
		e->StartInstructionBlock ();
		
#ifdef ENABLE_SINGLE_STEP
			LocalCycleCount = 0;
#endif

		// check if we are in a new icache block //
		// if region is cached and we are transitioning from previous block then need to check:
		// 1. check that block is in i-cache, and if so, continue
		// 2. if block is NOT in i-cache, then update Cycles,NextPC, and return
		// 3. don't put at beginning of block, so need RunCount >= 1 (will not be jumping between blocks)
		if ( bIsBlockInICache && ( ! ( Address & 0xf ) ) )
		{
#ifdef VERBOSE_RECOMPILE
cout << " END-CB";
#endif

			// new icache block //
			
			// for now just call it quits
			// later will add support for another block
			

			// check if address is cached in i-cache
			//if ( ICache_Device::isCached ( Address ) )
			//{
				// Address is cached //
				
				// update PC here for now
				
				// check if next cache-line is cached before executing it //
				
			if ( RunCount )
			{
					// Not the 1st intruction being encoded //
					
					// if address is cached, then check that address is in cache
					reti &= e->CmpMem32ImmX ( (long*) & r->ICache.ICacheBlockSource [ ( Address >> 4 ) & 0xff ], Address & 0x1ffffff0 );
				
#ifdef ENCODE_SINGLE_RUN_PER_BLOCK
					//reti &= e->Jmp_NE ( 0, 64 + ( i >> 2 ) );
					reti &= e->Jmp8_E ( 0, 0 );
					
#ifdef VERBOSE_RECOMPILE
cout << " Offset=" << hex << e->BranchOffset [ 64 + ( i >> 2 ) ];
#endif

#else
					// if not, then reload cache
					reti &= e->Jmp_NE ( 0, 12 + ulCacheLineCount );
#endif
				//}
				
				// cache-block is not loaded //
				
#ifdef UPDATE_BEFORE_RETURN
				// update NextPC
				// it should have already returned if NextPC was modified through other means so this should be ok
				e->MovMemImm32 ( (long*) & r->NextPC, Address );
				
				// update CycleCount
				//e->AddMem64ImmX ( (long long*) & r->CycleCount, CacheBlock_CycleCount - 1 );
				e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount );
#endif
				
				// done - return
				e->Ret ();
				
				// starting a new cache block
				//LocalCycleCount = MemCycles - 1;
				//CacheBlock_CycleCount = 0;
				
				// cache-block is loaded and can continue //
				ret &= e->SetJmpTarget8 ( 0 );

				// update the number cache or block lines crossed
				ulCacheLineCount++;
				
			} // end if ( RunCount )
				
			
			// update pointer based on whether data is in cache currently or not
			// this has to run whenever cache-line changes and instructions are cached, could even run every instruction
			pSrcCodePtr = RGetPointer ( Address );

		} // end if ( bIsBlockInICache && ( ! ( Address & 0xf ) ) )
			
		
#ifdef VERBOSE_RECOMPILE
cout << " INSTR#" << dec << i;
//cout << " LOC=" << hex << ((u64) e->Get_CodeBlock_CurrentPtr ());
cout << " CycleDiff=" << dec << LocalCycleCount;
#endif


		// set r0 to zero for now
		//e->MovMemImm32 ( &r->GPR [ 0 ].s, 0 );
		
		// in front of the instruction, set NextPC to the next instruction
		// do this at beginning of code block
		// NextPC = PC + 4
		//e->MovMemImm32 ( (long*) &r->NextPC, Address + 4 );
		
		// get the instruction
		//inst.Value = *((u32*) SrcCode);
		inst.Value = *pSrcCodePtr;
		
		
		// get the next instruction
		// note: this does not work if the next address is in a new cache block and the region is cached
		NextInst.Value = *(pSrcCodePtr + 1);
		
		// check if the address is cached
		// actually will need to check in the delay slot instruction if the delay slot is in the next cache block and if it is loaded or not
		//if ( ICache_Device::isCached ( Address ) )
		if ( bIsBlockInICache )
		{
			// address is in cached region //
			
			// check if next instruction is in same cache block or not
			if ( ! ( ( Address + 4 ) & 0xf ) )
			{
				// instruction is in another cache block
				
				//pNextCode = RGetPointer ( Address + 4 );
				//NextInst.Value = *pNextCode;
				
				NextInst.Value = -1;
			}

			// make sure instruction is not at end of code block
			//if ( ! ( ( Address + 4 ) & ( MaxStep_Mask << 2 ) ) )
			//{
			//	NextInst.Value = -1;
			//}
			
		}
		else
		{
			// not in cached region //
			
			// check if next instruction is in same cache block or not
			if ( ! ( ( Address + 4 ) & 0xf ) )
			{
				// instruction is in another cache block
				
				//pNextCode = RGetPointer ( Address + 4 );
				//NextInst.Value = *pNextCode;
				
				NextInst.Value = -1;
			}
			
			// still need to check against edge of block
			//if ( ! ( ( Address + 4 ) & ( MaxStep_Mask << 2 ) ) )
			//{
			//	// this can actually happen, so need to prevent optimizations there
			//	NextInst.Value = -1;
			//}
		}
		
		


#ifdef VERBOSE_RECOMPILE
cout << " OL=" << OpLevel;
#endif

		// check if a forward branch target needs to be set
		if ( pForwardBranchTargets [ BlockIndex & MaxStep_Mask ] )
		{
			// set the branch target
			e->SetJmpTarget ( pForwardBranchTargets [ BlockIndex & MaxStep_Mask ] );
		}
		
		// this is internal to recompiler and says where heading for instruction starts at
		//pPrefix_CodeStart [ BlockIndex & MaxStep_Mask ] = e->Get_CodeBlock_CurrentPtr ();
		pPrefix_CodeStart [ BlockIndex ] = (u8*) e->Get_CodeBlock_CurrentPtr ();
		
		// this can be changed by the instruction being recompiled to point to where the starting entry point should be for instruction instead of prefix
		pCodeStart [ BlockIndex ] = (u8*) e->Get_CodeBlock_CurrentPtr ();
		
#ifdef ENABLE_SINGLE_STEP

		CycleCount [ BlockIndex ] = 0;
		
#else
	
#ifdef ALLOW_COP2_CYCLE_RESET
		if ( ( inst.Value >> 26 ) == 0x12 )
		{
#ifdef VERBOSE_RECOMPILE
cout << " COP2";
#endif
			// COP2 instruction //
			CycleCount [ BlockIndex ] = LocalCycleCount;
		}
		else
#endif
		{
			// must add one to the cycle offset for starting point because the interpreter adds an extra cycle at the end of run
			//CycleCount [ BlockIndex ] = LocalCycleCount + 1;
			CycleCount [ BlockIndex ] = LocalCycleCount;
		}
		
#endif
		
		//EndAddress [ BlockIndex ] = -1;
		
		
		if ( inst.Value )
		{
		
		if ( OpLevel == 2 )
		{
			ret = Recompile2( Address );
			
			Local_NextPCModified = false;
			bStopEncodingBefore = false;
			bStopEncodingAfter = false;
			
#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiler: Recompile2 returns: " << dec << ret;
#endif
#ifdef INLINE_DEBUG_RECOMPILE
	debug << "\r\nRecompiler: Recompile2 returns: " << dec << ret;
#endif
		}
		else
		{
		// recompile the instruction
		ret = Recompile ( inst, Address );
		
#ifdef INLINE_DEBUG_RECOMPILE
	debug << "\r\nRecompiler: Recompile1 returns: " << dec << ret;
#endif

#ifdef VERBOSE_RECOMPILE
cout << " ret=" << ret;
//cout << " ENC0=" << hex << (((u64*) (pCodeStart [ BlockIndex ])) [ 0 ]);
//cout << " ENC1=" << hex << (((u64*) (pCodeStart [ BlockIndex ])) [ 1 ]);
cout << " ASM: " << Print::PrintInstruction ( inst.Value ).c_str ();
#endif
#ifdef INLINE_DEBUG_RECOMPILE
	debug << " ASM: " << Print::PrintInstruction ( inst.Value ).c_str ();
	debug << " ADDR:" << hex << Address;
#endif

		}
		
		}
		else
		{
			ret = 1;
		}	// end if ( inst.Value )
		

		if ( ret <= 0 )
		{
			// there was a problem, and recompiling is done
			
			// need to undo whatever we did for this instruction
			e->UndoInstructionBlock ();
			Local_NextPCModified = false;
			
//cout << "\nUndo: Address=" << hex << Address;
			
			// TODO: if no instructions have been encoded yet, then just try again with a lower optimization level
			if ( OpLevel > 0 )
			{
#ifdef INLINE_DEBUG_RECOMPILE
	debug << " REDO-INSTRUCTION-BLOCK";
#endif

				// could not encode the instruction at optimization level, so go down a level and try again
				OpLevel--;
				
				//Address -= 4;
				
				// at this point, this should be the last instruction since we had to go down an op level
				// this shouldn't be so, actually
				//MaxCount = 1;
				
				// here we need to reset and redo the instruction
				bStopEncodingBefore = false;
				bStopEncodingAfter = false;
				Local_NextPCModified = false;
				
				bResetCycleCount = false;
				
				// redo the instruction
				i--;
				continue;
			}
			else
			{
#ifdef INLINE_DEBUG_RECOMPILE
	debug << " BREAK-NO-REDO";
#endif
			
				//cout << "\nhps1x64: R3000A: Recompiler: Error: Unable to encode instruction.";
				
				// mark block as unable to recompile if there were no instructions recompiled at all
				//if ( !Cycles ) DoNotCache [ Block ] = 1;
				
				// done
				break;
			}
		}
		
		
		
			// if this is not the first instruction, then it can halt encoding before it
			if ( RunCount && bStopEncodingBefore )
			{
#ifdef VERBOSE_RECOMPILE
cout << " bStopEncodingBefore";
#endif
#ifdef INLINE_DEBUG_RECOMPILE
	debug << " bStopEncodingBefore";
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
				

				
#ifdef UPDATE_BEFORE_RETURN
				// check that NextPC was not modified
				// doesn't matter here except that RunCount>=1 so it is not first instruction in run, which is handled above
				// next pc was not modified because that will be handled differently now
				//if ( RunCount > 1 && !Local_NextPCModified )
				//{
					// update NextPC
					e->MovMemImm32 ( (long*) & r->NextPC, Address );
					
				//}
				
				// update CPU CycleCount
				e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles );
				//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles - ExeCycles );
#endif

#ifdef VERBOSE_RECOMPILE
cout << " RETURN";
#endif

				// return;
				reti &= e->x64EncodeReturn ();


				
				// set the current optimization level
				// note: don't do this here, because the optimization level might have been changed by current instruction
				//OpLevel = OptimizeLevel;
				
				// reset flags
				bStopEncodingBefore = false;
				bStopEncodingAfter = false;
				Local_NextPCModified = false;
				
				bResetCycleCount = false;
				
				// starting a new run
				RunCount = 0;
				
				// restart cycle count back to zero
				LocalCycleCount = 0;
				
				// clear delay slots
				//RDelaySlots [ 0 ].Value = 0;
				//RDelaySlots [ 1 ].Value = 0;
				
				//LocalCycleCount = MemCycles - 1;
				//CacheBlock_CycleCount = 0;
				
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
			} // end if ( RunCount && bStopEncodingBefore )

			
			
#ifdef INLINE_DEBUG_RECOMPILE
	debug << " END-INSTRUCTION-BLOCK";
#endif
			
			// instruction successfully encoded from MIPS into x64
			e->EndInstructionBlock ();
			
//cout << "\nCool: Address=" << hex << Address << " ret=" << dec << ret << " inst=" << hex << *pSrcCodePtr << " i=" << dec << i;

			// update number of instructions that have been recompiled
			//RecompileCount++;
			RecompileCount += ret;
			
			// update to next instruction
			//pSrcCodePtr++;
			pSrcCodePtr += ret;
			
			// add number of cycles encoded
			Cycles += ret;
			
			// update address
			//Address += 4;
			Address += ( ret << 2 );

			// update instruction count for run
			//RunCount++;
			RunCount += ret;
			
			// go to next block index
			//BlockIndex++;
			BlockIndex += ret;
			
			// update the cycles for run
			//LocalCycleCount += MemCycles;
			LocalCycleCount += ( MemCycles * ret );

			// reset the optimization level for next instruction
			OpLevel = OptimizeLevel;
			
			// need to update i also since some instructions might have been skipped over with OpLevel 2
			i += ( ret - 1 );
			
			
		// this has to happen here ?
		/*
		if ( bResetCycleCount )
		{
			// this isn't actually to reset the cycle count to zero, but rather to the cycles for the last instruction minus one
			// this is for instructions that wait for MulDiv unit to be ready or wait for COP2 to be ready
			LocalCycleCount = MemCycles - 1;
		}
		*/


#ifdef ENABLE_SINGLE_STEP
			LocalCycleCount = 0;
			bStopEncodingAfter = true;
#endif

		
		// if directed to stop encoding after the instruction, then do so
		if ( bStopEncodingAfter )
		{
#ifdef VERBOSE_RECOMPILE
cout << " bStopEncodingAfter";
#endif
#ifdef INLINE_DEBUG_RECOMPILE
	debug << " bStopEncodingAfter";
#endif

#ifdef ENCODE_SINGLE_RUN_PER_BLOCK


#ifdef UPDATE_BEFORE_RETURN
			// check that NextPC was not modified and that this is not an isolated instruction
			// actually just need to check if NextPC was modified by the encoded instruction
			if ( !Local_NextPCModified )
			{
				// update NextPC
				e->MovMemImm32 ( (long*) & r->NextPC, Address );
			}
			
			// update CycleCount
			e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles );
			//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles - ExeCycles );
#endif

				
#ifdef VERBOSE_RECOMPILE
cout << " RETURN";
#endif

			// return;
			reti &= e->x64EncodeReturn ();


			// set the current optimization level
			OpLevel = OptimizeLevel;
			
			// reset flags
			bStopEncodingBefore = false;
			bStopEncodingAfter = false;
			Local_NextPCModified = false;
			
			bResetCycleCount = false;
			
			// clear delay slots
			//RDelaySlots [ 0 ].Value = 0;
			//RDelaySlots [ 1 ].Value = 0;
			
			// starting a new run
			RunCount = 0;
			
			// restart cycle count to zero
			LocalCycleCount = 0;
			
			// cycle counts should start over
			//LocalCycleCount = MemCycles - 1;
			//CacheBlock_CycleCount = 0;
			
#else

				break;
#endif
		} // if ( bStopEncodingAfter )
			
			
			
		// reset flags
		bStopEncodingBefore = false;
		bStopEncodingAfter = false;
		Local_NextPCModified = false;
		
		bResetCycleCount = false;
			
	} // end for ( int i = 0; i < MaxStep; i++, Address += 4 )

#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiler: Done with loop";
#endif



#ifdef ENABLE_CONNECT_ADJACENT_BLOCKS

	// can't jump to the next block if NextPC is modified
	if ( !Local_NextPCModified )
	{
	// step 1: check if next block has been modified (if it is in main memory)
	if ( ( Address & 0x1fc00000 ) != 0x1fc00000 )
	{
		e->CmpMemImm8 ( (char*) & r->Bus->InvalidArray.b8 [ ( Address & Playstation1::DataBus::MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ], 0 );
		e->Jmp8_NE( 0, 0 );
	}
	
	// step 2: check if next block has the correct source address
	e->CmpMem32ImmX ( (long*) & StartAddress [ ( Block + 1 ) & NumBlocks_Mask ], Address );
	e->Jmp8_NE( 0, 1 );
	
	// step 3: if next block is cached, check that it is in i-cache
	if ( bIsBlockInICache )
	{
		// get the cache line that address should be at
		//u32 ICacheBlockIndex = ( Address >> 6 ) & 0x7f;
		u32 ICacheBlockIndex = ( Address >> 4 ) & 0xff;
		
		// make room for the way
		//ICacheBlockIndex <<= 1;

		// ICacheBlockSource [ ICacheBlockIndex ] == ( Address & 0x1ffffff0 )
		//e->CmpMem32ImmX ( & r->ICache.PFN [ ICacheBlockIndex ^ 1 ], ( Address & 0x1fffffc0 ) );
		e->CmpMem32ImmX ( (long*) & r->ICache.ICacheBlockSource [ ICacheBlockIndex ], ( Address & 0x1ffffff0 ) );
		e->Jmp8_NE ( 0, 3 );
		
		// make sure the cache line is valid and that the address is actually cached there
		// for ps2, must check way0 and way1 both
		//if ( PFN [ ICacheBlockIndex ] == ( Address & 0x1fffffc0 ) )
		//{
		//	//return & Data [ ICacheBlockIndex << 4 ];
		//	return & Data [ ( ICacheBlockIndex << 4 ) ^ ( ( Address >> 2 ) & 0xf ) ];
		//}
		
		//if ( PFN [ ICacheBlockIndex ^ 1 ] == ( Address & 0x1fffffc0 ) )
		//{
		//	//return & Data [ ( ICacheBlockIndex ^ 1 ) << 4 ];
		//	return & Data [ ( ( ICacheBlockIndex ^ 1 ) << 4 ) ^ ( ( Address >> 2 ) & 0xf ) ];
		//}
		
	}
	
	// step 0: make sure there are no pending events
	e->MovRegMem64 ( RAX, (long long*) & r->CycleCount );
	//e->AddReg64ImmX ( RAX, LocalCycleCount - ( MemCycles - 1 ) );
	e->AddReg64ImmX ( RAX, LocalCycleCount );
	
	
	// want check that there are no events pending //
	
	// get the current cycle count and compare with next event cycle
	// note: actually need to either offset the next event cycle and correct when done or
	// or need to offset the next even cycle into another variable and check against that one
#ifdef PS2_COMPILE
	e->CmpRegMem64 ( RAX, (long long*) & Playstation1::System::_SYSTEM->ullStopCycle );
#else
	e->CmpRegMem64 ( RAX, (long long*) & Playstation1::System::_SYSTEM->NextEvent_Cycle );
#endif
	
	// branch if current cycle is greater (or equal?) than next event cycle
	// changing this so that it branches if not returning
	// note: should probably be below or equal then jump, since the interpreter adds one to cycle
	//e->Jmp8_B ( 0, 0 );
	e->Jmp8_AE ( 0, 2 );
	//e->Jmp_AE ( 0, 0 );
	
	
	// since we have not reached the next event cycle, should write back the current system cycle
	// so that the correct cycle# gets seen when the store is executed
	// no need to update the CPU cycle count until either a branch/jump is encountered or returning
	// this way, there is no need to reset the current cycle number tally unless a branch/jump is encountered
	//e->DecReg64 ( RAX );
	e->MovMemReg64 ( (long long*) & Playstation1::System::_SYSTEM->CycleCount, RAX );
	
	
	// update cycle count
	//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount );
	e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
	
	e->MovMemImm32 ( (long*) & r->NextPC, Address );

	// step 4a: if all checks out, then jump to start of next block
	e->JMP ( e->Get_XCodeBlock_StartPtr ( ( Block + 1 ) & NumBlocks_Mask ) );
	
	// step 4b: if doesn't check out, then return
	if ( ( Address & 0x1fc00000 ) != 0x1fc00000 )
	{
	e->SetJmpTarget8 ( 0 );
	}
	e->SetJmpTarget8 ( 1 );
	e->SetJmpTarget8 ( 2 );
	if ( bIsBlockInICache )
	{
	e->SetJmpTarget8 ( 3 );
	}

	}	// end if ( !Local_NextPCModified )
	
#endif	// end ENABLE_CONNECT_ADJACENT_BLOCKS



#ifdef ENCODE_SINGLE_RUN_PER_BLOCK
	// at end of block need to return ok //
	
	// encode return if it has not already been encoded at end of block
	if ( RunCount )
	{
#ifdef INLINE_DEBUG_RECOMPILE
	debug << " END-RUN";
	debug << " START-PC=" << hex << BeginAddress;
#endif
	
#ifdef UPDATE_BEFORE_RETURN
		// check that NextPC was not modified and that this is not an isolated instruction
		if ( !Local_NextPCModified )
		{
#ifdef INLINE_DEBUG_RECOMPILE
	debug << " NEXT-PC=" << hex << Address;
#endif

			// update NextPC
			e->MovMemImm32 ( (long*) & r->NextPC, Address );
		}
		
		// update CycleCount
		// after update need to put in the minus MemCycles
		e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles );
		//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles - ExeCycles );
#endif

	} // end if ( RunCount )

#ifdef VERBOSE_RECOMPILE
cout << "\nulCacheLineCount=" << dec << ulCacheLineCount;
#endif


#ifdef VERBOSE_RECOMPILE
cout << "\nRecompiler: Encoding RETURN";
#endif

#ifdef INLINE_DEBUG_RECOMPILE
	debug << " RET";
#endif

	// return;
	reti &= e->x64EncodeReturn ();
	
#endif

	}
	
	}

	
	// done encoding block
	e->EndCodeBlock ();
	
	// address is now encoded
	
	
	if ( !reti )
	{
		cout << "\nRecompiler: Out of space in code block.";
	}

#ifdef INLINE_DEBUG_RECOMPILE
	debug << "\r\nEND->Recompiler::Recompile";
#endif

	
	return reti;
	//return RecompileCount;
}

/*
void Recompiler::Invalidate ( u32 Address, u32 Count )
{
	s32 StartBlock, StopBlock;
	
	u32 iBlock;
	
	
	// get the address to actually start checking from
	Address -= ( MaxStep - 1 );
	
	// update the count
	Count += ( MaxStep - 1 );
	
	// get the starting block
	iBlock = ( Address >> 2 ) & NumBlocks_Mask;
	
	while ( Count > 0 )
	{
		if ( ( StartAddress [ iBlock ] == Address ) && ( RunCount [ iBlock ] >= Count ) )
		{
			// invalidate the block
			StartAddress [ iBlock ] = 0xffffffff;
		}
		
		// update vars
		Address += 4;
		iBlock++;
		Count--;
	}
	
}
*/


// code generation //

// generate the instruction prefix to check for any pending events
// will also update NextPC,CycleCount (CPU) and return if there is an event
// will also update CycleCount (System) on a load or store
long Recompiler::Generate_Prefix_EventCheck ( u32 Address, bool bIsBranchOrJump )
{
	long ret;
	
	// get updated CycleCount value for CPU
	e->MovRegMem64 ( RAX, (long long*) & r->CycleCount );
	e->AddReg64ImmX ( RAX, LocalCycleCount );
	
	
	// want check that there are no events pending //
	
	// get the current cycle count and compare with next event cycle
	// note: actually need to either offset the next event cycle and correct when done or
	// or need to offset the next even cycle into another variable and check against that one
	e->CmpRegMem64 ( RAX, (long long*) & Playstation1::System::_SYSTEM->NextEvent_Cycle );
	
	// branch if current cycle is greater (or equal?) than next event cycle
	// changing this so that it branches if not returning
	//e->Jmp_A ( 0, 100 + RetJumpCounter++ );
	e->Jmp8_B ( 0, 0 );
	
	// update NextPC
	e->MovMemImm32 ( (long*) & r->NextPC, Address );
	
	// update CPU CycleCount
	e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
	
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
		ret = e->MovMemReg64 ( (long long*) & Playstation1::System::_SYSTEM->CycleCount, RAX );
	}

	return ret;
}



// BitTest should be 1 for SH, 3 for SW, 0 for SB, etc
long Recompiler::Generate_Normal_Store ( Instruction::Format i, u32 Address, u32 BitTest, void* StoreFunctionToCall )
{
	long ret;
	bool bPerformInlineStore;
	
	// part 1: first check for event //
	
#ifdef CHECK_EVENT_AFTER_START
	// get updated CycleCount value for CPU (the value as it would be after instruction executed)
	e->MovRegMem64 ( RAX, (long long*) & r->CycleCount );
	e->AddReg64ImmX ( RAX, LocalCycleCount - ( MemCycles - 1 ) );
	//e->AddReg64ImmX ( RAX, LocalCycleCount - MemCycles );
	
	
	// want check that there are no events pending //
	
	// get the current cycle count and compare with next event cycle
	// note: actually need to either offset the next event cycle and correct when done or
	// or need to offset the next even cycle into another variable and check against that one
#ifdef PS2_COMPILE
	e->CmpRegMem64 ( RAX, (long long*) & Playstation1::System::_SYSTEM->ullStopCycle );
#else
	e->CmpRegMem64 ( RAX, (long long*) & Playstation1::System::_SYSTEM->NextEvent_Cycle );
#endif
	
	// branch if current cycle is greater (or equal?) than next event cycle
	// changing this so that it branches if not returning
	// note: should probably be below or equal then jump, since the interpreter adds one to cycle
	//e->Jmp8_B ( 0, 0 );
	e->Jmp8_AE ( 0, 0 );
	//e->Jmp_AE ( 0, 0 );
	
	
	// since we have not reached the next event cycle, should write back the current system cycle
	// so that the correct cycle# gets seen when the store is executed
	// no need to update the CPU cycle count until either a branch/jump is encountered or returning
	// this way, there is no need to reset the current cycle number tally unless a branch/jump is encountered
	//e->DecReg64 ( RAX );
	e->MovMemReg64 ( (long long*) & Playstation1::System::_SYSTEM->CycleCount, RAX );
	
	// part 2: check for synchronous interrupt //
	
	// this is where the entry point should be if this is the first instruction in the run
	pCodeStart [ BlockIndex ] = (u8*) e->Get_CodeBlock_CurrentPtr ();
#endif
	
	// get the store address
	//u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	e->MovRegFromMem32 ( RDX, &r->GPR [ i.Base ].s );
	e->AddReg32ImmX ( RDX, i.sOffset );

	// check for synchronous interrupt
	
	// if there is a synchronous interrupt possible, then check for it
	if ( BitTest )
	{
		// if ( StoreAddress & 0x1 )
		e->TestReg32ImmX ( RDX, BitTest );

		// branch if zero
		//e->Jmp8_E ( 0, 0 );
		e->Jmp8_NE ( 0, 1 );
		
	}
	
	// part 3: execute the store //
			
	// get the value to store
	//e->MovRegFromMem32 ( RCX, &r->GPR [ i.Rt ].s );





	
#ifdef ENABLE_INLINE_STORE

	// exclusions for testing
	switch ( i.Opcode )
	{
		case OPSB:
		case OPSH:
		case OPSW:
		case OPSWC2:
		case OPSWL:
		case OPSWR:
			bPerformInlineStore = true;
			break;
		
			
		default:
			bPerformInlineStore = false;
			break;
	}
	
	if ( bPerformInlineStore )
	{
		

	// RCX has the address
	// RDX has the value to store
	
	// get the index into the device pointer array
	//e->MovRegReg32 ( RAX, RCX );
	e->MovRegReg32 ( RAX, RDX );
	e->ShrRegImm32 ( RAX, 22 );
	//e->AddRegReg32 ( RAX, RAX );
	e->ShlRegImm32 ( RAX, 2 );
	
	// get the pointer into the device pointer array
	//e->MovRegImm64 ( 9, (u64) & Playstation1::DataBus::LUT_DataBus_Write );
	e->LeaRegMem64 ( 9, & Playstation1::DataBus::LUT_DataBus_Write );
	
	
			// load the pointer into the device into RCX
			// save RCX into R9 first, though
			//e->MovRegReg32 ( 9, RCX );
			e->MovRegFromMem64 ( 10, 9, RAX, SCALE_EIGHT, 0 );
			
			// jump if zero
			//e->CmpReg64ImmX ( 10, 0 );
			e->OrRegReg64 ( 10, 10 );
	
	
	//e->Jmp_E ( 0, 0 );
	e->Jmp8_E ( 0, 4 );

	
	
	e->TestRegMem32 ( RDX, 9, RAX, SCALE_EIGHT, 24 );
	e->Jmp8_NE ( 0, 7 );
	
	
	// check isc
	//e->MovRegMem32 ( RAX, & r->CPR0.Status.Value );
	//e->BtRegImm32 ( RAX, 16 );
	e->BtMemImm32 ( (long*) & r->CPR0.Status.Value, 16 );
	e->Jmp8_B ( 0, 6 );
	//e->Jmp_B ( 0, 6 );
	
	
	// get pointer into invalidate array
	e->MovRegFromMem64 ( 11, 9, RAX, SCALE_EIGHT, 16 );

	// mask address
	//e->AndRegMem32 ( RCX, 9, RAX, SCALE_EIGHT, 8 );
	e->AndRegMem32 ( RDX, 9, RAX, SCALE_EIGHT, 8 );

	// get the latency
	e->MovRegFromMem32 ( 9, 9, RAX, SCALE_EIGHT, 12 );
	
	
	switch ( i.Opcode )
	{
		case OPSWC2:
			// get the value to store from COP1 register
			e->MovRegMem32 ( RCX, (long*) &r->COP2.CPR2.Regs [ i.Rt ] );
			break;
			
			
		case OPSWL:
			
#ifdef USE_SHORT_SWL_CODE
			e->MovRegMem32 ( RCX, &r->GPR [ i.Rt ].s );
			e->MovRegReg32 ( 8, RDX );
#else
			// temporarily save store address
			e->MovRegReg32 ( RCX, RDX );
			
			e->NotReg32 ( RCX );
			e->AndReg32ImmX ( RCX, 3 );
			e->ShlRegImm32 ( RCX, 3 );
			e->MovReg32ImmX ( RAX, -1 );
			e->ShrRegReg32 ( RAX );
			e->MovRegReg32 ( 8, RAX );
			e->MovRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
			e->ShrRegReg32 ( RAX );
			//e->MovRegReg32 ( RCX, RDX );
			//e->MovRegReg32 ( RDX, RAX );
			e->MovRegReg32 ( RCX, RAX );
#endif
		
			// clear bottom two bits of address
			//e->AndReg32ImmX ( RCX, ~0x3 );
			e->AndReg32ImmX ( RDX, ~0x3 );
			
			break;
			
		case OPSWR:
#ifdef USE_SHORT_SWR_CODE
			e->MovRegMem32 ( RCX, &r->GPR [ i.Rt ].s );
			e->MovRegReg32 ( 8, RDX );
#else
			// temporarily save store address
			//e->MovRegReg32 ( RDX, RCX );
			e->MovRegReg32 ( RCX, RDX );
			
			e->AndReg32ImmX ( RCX, 3 );
			e->ShlRegImm32 ( RCX, 3 );
			e->MovReg32ImmX ( RAX, -1 );
			e->ShlRegReg32 ( RAX );
			e->MovRegReg32 ( 8, RAX );
			e->MovRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
			e->ShlRegReg32 ( RAX );
			//e->MovRegReg32 ( RCX, RDX );
			//e->MovRegReg32 ( RDX, RAX );
			e->MovRegReg32 ( RCX, RAX );
#endif
		
			// clear bottom two bits of address
			//e->AndReg32ImmX ( RCX, ~0x3 );
			e->AndReg32ImmX ( RDX, ~0x3 );
			break;

		default:
			// get the value to store (32-bit)
			//e->MovRegMem32 ( RDX, &r->GPR [ i.Rt ].sw0 );
			e->MovRegMem32 ( RCX, &r->GPR [ i.Rt ].s );
			
			break;
			
	}

	
	
	
	
	// testing
	//e->MovMemReg32 ( & r->testvar [ 0 ], 11 );
	//e->MovMemReg32 ( & r->testvar [ 1 ], RCX );
	

	// testing
	//e->MovMemReg32 ( & r->testvar [ 2 ], RCX );
	//e->MovMemReg32 ( & r->testvar [ 3 ], RDX );
	
	// store the value
	switch ( i.Opcode )
	{
		case OPSB:
			//e->MovRegToMem8 ( RDX, 10, RCX, SCALE_NONE, 0 );
			e->MovRegToMem8 ( RCX, 10, RDX, SCALE_NONE, 0 );
			break;
		case OPSH:
			//e->MovRegToMem16 ( RDX, 10, RCX, SCALE_NONE, 0 );
			e->MovRegToMem16 ( RCX, 10, RDX, SCALE_NONE, 0 );
			break;
		case OPSWL:
#ifdef USE_SHORT_SWL_CODE
			e->MovRegFromMem32 ( RAX, 10, RDX, SCALE_NONE, -4 );
			e->MovRegToMem32 ( RCX, 10, 8, SCALE_NONE, -3 );
			e->MovRegToMem32 ( RAX, 10, RDX, SCALE_NONE, -4 );
			break;
#endif
		case OPSWR:
#ifdef USE_SHORT_SWR_CODE
			e->MovRegFromMem32 ( RAX, 10, RDX, SCALE_NONE, 4 );
			e->MovRegToMem32 ( RCX, 10, 8, SCALE_NONE, 0 );
			e->MovRegToMem32 ( RAX, 10, RDX, SCALE_NONE, 4 );
			break;
#else
			//e->AndRegReg32 ( RDX, 8 );
			e->AndRegReg32 ( RCX, 8 );
			e->NotReg32 ( 8 );
			//e->AndRegMem32 ( 8, 10, RCX, SCALE_NONE, 0 );
			e->AndRegMem32 ( 8, 10, RDX, SCALE_NONE, 0 );
			//e->OrRegReg32 ( RDX, 8 );
			e->OrRegReg32 ( RCX, 8 );
#endif
		case OPSW:
		case OPSWC2:
			//e->MovRegToMem32 ( RDX, 10, RCX, SCALE_NONE, 0 );
			e->MovRegToMem32 ( RCX, 10, RDX, SCALE_NONE, 0 );
			break;
	}
	
	// also need to invalidate cache
	//e->ShrRegImm32 ( RCX, 2 + r->Bus->c_iInvalidate_Shift );
	//e->MovMemImm8 ( 1, 11, RCX, SCALE_NONE, 0 );
	e->ShrRegImm32 ( RDX, 2 + r->Bus->c_iInvalidate_Shift );
	e->MovMemImm8 ( 1, 11, RDX, SCALE_NONE, 0 );
	
	
	// add additional latency
	//e->AddMem64ImmX ( (long long*) & r->CycleCount, 2 );
	e->AddMemReg64 ( (long long*) & r->CycleCount, 9 );

	// testing
	//e->MovRegMem32 ( RCX, & r->Bus->MainMemory.b32 [ ( /*Address*/ 0x588 & r->Bus->MainMemory_Mask ) >> 2 ] );
	//e->MovMemReg32 ( & r->testvar [ 4 ], RCX );
	
	e->Jmp8 ( 0, 5 );
	}
	else
	{
	e->Jmp8 ( 0, 4 );
	}

#else
	e->Jmp8 ( 0, 4 );
#endif


#ifdef CHECK_EVENT_AFTER_START
	//if ( !e->SetJmpTarget ( 0 ) )
	if ( !e->SetJmpTarget8 ( 0 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch0 too far!";
	}

	// update NextPC
	e->MovMemImm32 ( (long*) & r->NextPC, Address );
	
	// update CPU CycleCount
	//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
	e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles );
	//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles - ExeCycles );
	
	// done for now - return
	e->Ret ();
#endif

	if ( BitTest )
	{
	if ( !e->SetJmpTarget8 ( 1 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch1 too far!";
	}
	
		// update CycleCount, set PC, then jump to synchronous interrupt
		//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
		e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount );
		
		// set pc
		e->MovMemImm32 ( (long*) & r->PC, Address );
		
		//r->ProcessSynchronousInterrupt ( Cpu::EXC_ADES );
		e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_ADES> );
	}


	if ( !e->SetJmpTarget8 ( 4 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch4 too far!";
	}
#ifdef ENABLE_INLINE_STORE
	//if ( !e->SetJmpTarget ( 6 ) )
	if ( !e->SetJmpTarget8 ( 6 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch6 too far!";
	}
	if ( !e->SetJmpTarget8 ( 7 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch7 too far!";
	}
#endif


	switch ( i.Opcode )
	{
		case OPSWC2:
			// get the value to store from COP1 register
			e->MovRegMem32 ( RCX, (long*) &r->COP2.CPR2.Regs [ i.Rt ] );
			break;
			
		default:
			e->MovRegFromMem32 ( RCX, &r->GPR [ i.Rt ].s );
			break;
	}
	
	
	// call the function to store value //
	
#ifdef RESERVE_STACK_FRAME_FOR_CALL
	e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

	ret = e->Call ( StoreFunctionToCall );

#ifdef RESERVE_STACK_FRAME_FOR_CALL
	ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

	//e->Jmp8 ( 0, 2 );


	if ( !e->SetJmpTarget8 ( 2 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch2 too far!";
	}
	if ( !e->SetJmpTarget8 ( 5 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch5 too far!";
	}
	return ret;
}





long Recompiler::Generate_Normal_Store_L2 ( Instruction::Format i, u32 Address, u32 BitTest, u32 BaseAddress )
{
	long ret;
	bool bPerformInlineStore;
	
	u8* pMemoryDevice8;
	u32 ulMask;
	u32 ulLatency;
	u8* pInvalidateDevice8;
	u32 ulDeviceTestMask;
	u32 Dummy;
	
	u32 lConst;
	
	int Rt;
	
	u32 StoreAddress;




	// get the store address
	//u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	//e->MovRegFromMem32 ( RDX, &r->GPR [ i.Base ].s );
	//e->AddReg32ImmX ( RDX, i.sOffset );
	BaseAddress += ( (s32) i.sOffset );


	pMemoryDevice8 = (u8*) Playstation1::DataBus::LUT_DataBus_Write [ BaseAddress >> 22 ].pMemoryDevice;
	pInvalidateDevice8 = Playstation1::DataBus::LUT_DataBus_Write [ BaseAddress >> 22 ].pInvalidateDevice;
	ulMask = Playstation1::DataBus::LUT_DataBus_Write [ BaseAddress >> 22 ].ulMask;
	ulLatency = Playstation1::DataBus::LUT_DataBus_Write [ BaseAddress >> 22 ].ulLatency;
	ulDeviceTestMask = Playstation1::DataBus::LUT_DataBus_Write [ BaseAddress >> 22 ].ulDeviceTest;
	
	
	// check for synchronous interrupt
	
	// if there is a synchronous interrupt possible, then check for it
	if ( BaseAddress & BitTest )
	{
		return 0;
	}

	if ( !pMemoryDevice8 )
	{
		return 0;
	}
	
	if ( BaseAddress & ulDeviceTestMask )
	{
		return 0;
	}




	
	// part 1: first check for event //
	
#ifdef CHECK_EVENT_AFTER_START
	if ( RunCount2 )
	{
	// get updated CycleCount value for CPU (the value as it would be after instruction executed)
	e->MovRegMem64 ( RCX, (long long*) & r->CycleCount );
	e->AddReg64ImmX ( RCX, LocalCycleCount2 - ( MemCycles - 1 ) );
	//e->AddReg64ImmX ( RAX, LocalCycleCount - MemCycles );
	
	
	// want check that there are no events pending //
	
	// get the current cycle count and compare with next event cycle
	// note: actually need to either offset the next event cycle and correct when done or
	// or need to offset the next even cycle into another variable and check against that one
#ifdef PS2_COMPILE
	e->CmpRegMem64 ( RCX, (long long*) & Playstation1::System::_SYSTEM->ullStopCycle );
#else
	e->CmpRegMem64 ( RCX, (long long*) & Playstation1::System::_SYSTEM->NextEvent_Cycle );
#endif
	
	// branch if current cycle is greater (or equal?) than next event cycle
	// changing this so that it branches if not returning
	// note: should probably be below or equal then jump, since the interpreter adds one to cycle
	//e->Jmp8_B ( 0, 0 );
	e->Jmp8_AE ( 0, 0 );
	//e->Jmp_AE ( 0, 0 );
	
	
	// since we have not reached the next event cycle, should write back the current system cycle
	// so that the correct cycle# gets seen when the store is executed
	// no need to update the CPU cycle count until either a branch/jump is encountered or returning
	// this way, there is no need to reset the current cycle number tally unless a branch/jump is encountered
	//e->DecReg64 ( RAX );
	e->MovMemReg64 ( (long long*) & Playstation1::System::_SYSTEM->CycleCount, RCX );
	
	// part 2: check for synchronous interrupt //
	
	// this is where the entry point should be if this is the first instruction in the run
	//pCodeStart [ BlockIndex ] = e->Get_CodeBlock_CurrentPtr ();
	}
#endif
	

	
	// part 3: execute the store //
			

		
	// check isc
	//e->MovRegMem32 ( RAX, & r->CPR0.Status.Value );
	//e->BtRegImm32 ( RAX, 16 );
	e->BtMemImm32 ( (long*) & r->CPR0.Status.Value, 16 );
	//e->Jmp_B ( 0, 6 );
	e->Jmp8_AE ( 0, 6 );
	//e->Jmp8 ( 0, 6 );
	
	
	e->MovMemImm32 ( (long*) & r->ICache.ICacheBlockSource [ ( BaseAddress >> 4 ) & 0xff ], -1 );


	e->Jmp8 ( 0, 5 );


	//if ( !e->SetJmpTarget8 ( 7 ) )
	//{
	//	cout << "\nhps1x64: R3000A: Recompiler: short branch7 too far!";
	//}

#ifdef CHECK_EVENT_AFTER_START
	if ( RunCount2 )
	{
	//if ( !e->SetJmpTarget ( 0 ) )
	if ( !e->SetJmpTarget8 ( 0 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch0 too far!";
	}

	// update NextPC
	e->MovMemImm32 ( (long*) & r->NextPC, Address );
	
	// update CPU CycleCount
	//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
	e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount2 - MemCycles );
	//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles - ExeCycles );
	
	WriteBackModifiedRegs ();
	
	RestoreRegsFromStack ();
	
	// done for now - return
	e->Ret ();
	}
#endif



	//if ( !e->SetJmpTarget ( 6 ) )
	if ( !e->SetJmpTarget8 ( 6 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch6 too far!";
	}
	

	
	StoreAddress = BaseAddress & ulMask;

	
	
	
	switch ( i.Opcode )
	{
		case OPSWC2:
			// get the value to store from COP1 register
			e->MovRegMem32 ( RCX, (long*) &r->COP2.CPR2.Regs [ i.Rt ] );
			break;
			
			/*
		case OPSWL:
			
			e->MovRegMem32 ( RCX, &r->GPR [ i.Rt ].s );
			//e->MovRegReg32 ( 8, RDX );
		
			// clear bottom two bits of address
			//e->AndReg32ImmX ( RDX, ~0x3 );
			//StoreAddress &= ~0x3;
			
			break;
			
		case OPSWR:
			e->MovRegMem32 ( RCX, &r->GPR [ i.Rt ].s );
			//e->MovRegReg32 ( 8, RDX );
		
			// clear bottom two bits of address
			//e->AndReg32ImmX ( RDX, ~0x3 );
			//StoreAddress &= ~0x3;
			break;
			*/

		default:
			// get the value to store (32-bit)
			if ( isConst( i.Rt ) )
			{
				lConst = GetConst( i.Rt );
			}
			else
			{
				Rt = Alloc_SrcReg ( i.Rt );
			}
			//e->MovRegMem32 ( RCX, &r->GPR [ i.Rt ].s );
			
			break;
			
	}

	
	// store the value
	switch ( i.Opcode )
	{
		case OPSB:
			//e->MovRegToMem8 ( RDX, 10, RCX, SCALE_NONE, 0 );
			//e->MovRegToMem8 ( RCX, 10, RDX, SCALE_NONE, 0 );
			if ( isConst( i.Rt ) )
			{
				e->MovMemImm8 ( (char*) & pMemoryDevice8 [ StoreAddress ], lConst );
			}
			else
			{
				e->MovMemReg8 ( (char*) & pMemoryDevice8 [ StoreAddress ], Rt );
			}
			break;
		case OPSH:
			//e->MovRegToMem16 ( RDX, 10, RCX, SCALE_NONE, 0 );
			//e->MovRegToMem16 ( RCX, 10, RDX, SCALE_NONE, 0 );
			if ( isConst( i.Rt ) )
			{
				e->MovMemImm16 ( (short*) ( & pMemoryDevice8 [ StoreAddress ] ), lConst );
			}
			else
			{
				e->MovMemReg16 ( (short*) ( & pMemoryDevice8 [ StoreAddress ] ), Rt );
			}
			break;
		case OPSWL:
			if ( ( StoreAddress & 3 ) == 3 )
			{
				if ( isConst( i.Rt ) )
				{
					e->MovMemImm32( (long*) ( & pMemoryDevice8 [ StoreAddress & ~3 ] ), lConst );
				}
				else
				{
					e->MovMemReg32( (long*) ( & pMemoryDevice8 [ StoreAddress & ~3 ] ), Rt );
				}
			}
			else
			{
				e->MovRegMem32 ( RCX, (long*) ( & pMemoryDevice8 [ ( StoreAddress & ~3 ) - 4 ] ) );
				if ( isConst( i.Rt ) )
				{
					e->MovMemImm32 ( (long*) ( & pMemoryDevice8 [ StoreAddress - 3 ] ), lConst );
				}
				else
				{
					e->MovMemReg32 ( (long*) ( & pMemoryDevice8 [ StoreAddress - 3 ] ), Rt );
				}
				e->MovMemReg32 ( (long*) ( & pMemoryDevice8 [ ( StoreAddress & ~3 ) - 4 ] ), RCX );
			}
			break;
		case OPSWR:
			if ( ( StoreAddress & 3 ) == 0 )
			{
				if ( isConst( i.Rt ) )
				{
					e->MovMemImm32( (long*) ( & pMemoryDevice8 [ StoreAddress & ~3 ] ), lConst );
				}
				else
				{
					e->MovMemReg32( (long*) ( & pMemoryDevice8 [ StoreAddress & ~3 ] ), Rt );
				}
			}
			else
			{
				e->MovRegMem32 ( RCX, (long*) ( & pMemoryDevice8 [ ( StoreAddress & ~3 ) + 4 ] ) );
				if ( isConst( i.Rt ) )
				{
					e->MovMemImm32 ( (long*) ( & pMemoryDevice8 [ StoreAddress - 0 ] ), lConst );
				}
				else
				{
					e->MovMemReg32 ( (long*) ( & pMemoryDevice8 [ StoreAddress - 0 ] ), Rt );
				}
				e->MovMemReg32 ( (long*) ( & pMemoryDevice8 [ ( StoreAddress & ~3 ) + 4 ] ), RCX );
			}
			break;
		case OPSW:
		//case OPSWC2:
			//e->MovRegToMem32 ( RDX, 10, RCX, SCALE_NONE, 0 );
			//e->MovRegToMem32 ( RCX, 10, RDX, SCALE_NONE, 0 );
			if ( isConst( i.Rt ) )
			{
				e->MovMemImm32( (long*) ( & pMemoryDevice8 [ StoreAddress ] ), lConst );
			}
			else
			{
				e->MovMemReg32( (long*) ( & pMemoryDevice8 [ StoreAddress ] ), Rt );
			}
			break;
		case OPSWC2:
			e->MovMemReg32( (long*) ( & pMemoryDevice8 [ StoreAddress ] ), RCX );
			break;
	}
	
	// also need to invalidate cache
	//e->ShrRegImm32 ( RDX, 2 + r->Bus->c_iInvalidate_Shift );
	//e->MovMemImm8 ( 1, 11, RDX, SCALE_NONE, 0 );
	e->MovMemImm8 ( (char*) & pInvalidateDevice8 [ StoreAddress >> ( 2 + r->Bus->c_iInvalidate_Shift ) ], 1 );
	
	// add additional latency
	//e->AddMemReg64 ( (long long*) & r->CycleCount, 9 );
	e->AddMem64ImmX ( (long long*) & r->CycleCount, ulLatency );

	





	//if ( !e->SetJmpTarget8 ( 2 ) )
	//{
	//	cout << "\nhps1x64: R3000A: Recompiler: short branch2 too far!";
	//}
	if ( !e->SetJmpTarget8 ( 5 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch5 too far!";
	}
	
//cout << "\nRecompile L2 Store: ADDR=" << hex << Address << " BaseAddr=" << BaseAddress << " StoreAddr=" << StoreAddress << " Const=" << lConst << dec << " Rt=" << Rt << " i.Rt=" << i.Rt;
	
	return 1;
}



// BitTest should be 1 for LH, 3 for LW, 0 for LB, etc
// LoadFunctionImmediate is the load function to use for immediate loading of data
long Recompiler::Generate_Normal_Load ( Instruction::Format i, u32 Address, u32 BitTest, void* LoadFunctionToCall, void* LoadFunctionImmediate )
{
	long ret;
	bool bPerformInlineLoad;
	bool bNoConflict = false;

	
#ifdef CHECK_EVENT_AFTER_START
	// part 1: first check for event //
	
	// get updated CycleCount value for CPU
	e->MovRegMem64 ( RAX, (long long*) & r->CycleCount );
	e->AddReg64ImmX ( RAX, LocalCycleCount - ( MemCycles - 1 ) );
	//e->AddReg64ImmX ( RAX, LocalCycleCount - MemCycles );
	
	
	// want check that there are no events pending //
	
	// get the current cycle count and compare with next event cycle
	// note: actually need to either offset the next event cycle and correct when done or
	// or need to offset the next even cycle into another variable and check against that one
#ifdef PS2_COMPILE
	e->CmpRegMem64 ( RAX, (long long*) & Playstation1::System::_SYSTEM->ullStopCycle );
#else
	e->CmpRegMem64 ( RAX, (long long*) & Playstation1::System::_SYSTEM->NextEvent_Cycle );
#endif
	
	// branch if current cycle is greater (or equal?) than next event cycle
	// changing this so that it branches if not returning
	// note: should probably be below or equal then jump, since the interpreter adds one to cycle
	//e->Jmp8_B ( 0, 0 );
	e->Jmp8_AE ( 0, 0 );
	//e->Jmp_AE ( 0, 0 );
	
	
	// since we have not reached the next event cycle, should write back the current system cycle
	// so that the correct cycle# gets seen when the store is executed
	// no need to update the CPU cycle count until either a branch/jump is encountered or returning
	// this way, there is no need to reset the current cycle number tally unless a branch/jump is encountered
	e->MovMemReg64 ( (long long*) & Playstation1::System::_SYSTEM->CycleCount, RAX );
	
	// part 2: check for synchronous interrupt //
	
	// this is where the entry point should be if this is the first instruction in the run
	pCodeStart [ BlockIndex ] = (u8*) e->Get_CodeBlock_CurrentPtr ();
#endif
	
	// get the store address
	//u32 StoreAddress = r->GPR [ i.Base ].s + i.sOffset;
	//e->MovRegFromMem32 ( RDX, &r->GPR [ i.Base ].s );
	//e->AddReg32ImmX ( RDX, i.sOffset );
	e->MovRegFromMem32 ( RCX, &r->GPR [ i.Base ].s );
	e->AddReg32ImmX ( RCX, i.sOffset );

	// check for synchronous interrupt
	
	// if there is a synchronous interrupt possible, then check for it
	if ( BitTest )
	{
		// if ( StoreAddress & 0x1 )
		//e->TestReg32ImmX ( RDX, BitTest );
		e->TestReg32ImmX ( RCX, BitTest );

		// branch if zero
		//e->Jmp8_E ( 0, 0 );
		e->Jmp8_NE ( 0, 1 );
		//e->Jmp_NE ( 0, 1 );
		
	}
	
	// part 3: execute the load //
	
	// make sure that destination registers don't match
	// if so, cancel the load
	//if ( GetDestRegs ( NextInst ) & ( 1ULL << i.Rt ) )
	//{
	//	// cancelling load //
	//}
	//else
	{
		
		// for loads
		// 1. check that there are no events. If so, update Cycles,NextPC, then return
		// 2. check for synchronous interrupt
		// 3. check that there are no conflicts. If so, put load in delay slot, update Cycles,NextPC, then return
		// 4. encode load, then encode load delay slot
		// 5. if going across cache line and next line is not loaded, then put load in delay slot and return
		// 6. if it is a store in the delay slot, then can just process normally as if there is no delay slot and immediately load
		// 7. if at edge of code block, then put load in delay slot and return

		
#ifdef EXECUTE_LOADS_IMMEDIATELY
		// check for conflicts
		// if there is a conflict, then put in delay slot and return
		// no need to check for r0 as it is fixed
		if ( ( ( 1 << i.Rt ) & GetSourceRegs ( NextInst, Address + 4 ) & ~1 ) || ( NextInst.Value == -1 ) || ( ( 1 << i.Rt ) & Get_DelaySlot_DestRegs ( NextInst ) ) )
#endif
		{

			// conflict //
			
			// put the load in delay slot and return
			//e->MovRegImm32 ( 0, i.sOffset );
			//e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			//e->OrMem64ImmX ( &r->Status.Value, 1 );
			
			
		//e->MovRegToMem32 ( & r->DelaySlots [ 1 ].Data, RDX );
		e->MovRegToMem32 ( (long*) & r->DelaySlots [ 1 ].Data, RCX );
		e->MovMemImm32 ( (long*) & r->DelaySlots [ 1 ].Instruction.Value, i.Value );
		
			//e->MovReg64ImmX ( RAX, (u64) LoadFunctionToCall );
			e->LeaRegMem64 ( RAX, LoadFunctionToCall );
			e->MovMemReg64 ( (long long*) & r->DelaySlots [ 1 ].cb, RAX );
			
			e->MovMemImm32 ( (long*) & r->NextDelaySlotIndex, 0 );
			e->OrMem64ImmX ( (long long*) &r->Status.Value, 2 );
			ret = e->MovMemImm32 ( (long*) & r->LastModifiedRegister, 255 );


	e->MovMemImm32 ( (long*) & r->NextPC, Address + 4 );
	
	// update CPU CycleCount
	//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
	e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount );
	//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles - ExeCycles );
	
	// done for now - return
	e->Ret ();

			
			// return after putting the load into delay slot
			bStopEncodingAfter = true;
			
		}
#ifdef EXECUTE_LOADS_IMMEDIATELY
		else
		{
			// NO conflict //
			bNoConflict = true;



#ifdef ENABLE_INLINE_LOAD

	// exclusions for testing
	switch ( i.Opcode )
	{
		case OPLB:
		case OPLH:
		case OPLBU:
		case OPLHU:
		case OPLW:
		case OPLWC2:
		case OPLWL:
		case OPLWR:
			bPerformInlineLoad = true;
			break;
		
			
		default:
			bPerformInlineLoad = false;
			break;
	}
	
	if ( bPerformInlineLoad )
	{
		
	// check isc
	//e->MovRegMem32 ( RAX, & r->CPR0.Status.Value );
	//e->BtRegImm32 ( RAX, 16 );
	//e->Jmp8_B ( 0, 6 );

	// RDX has the address
	
	// get the index into the device pointer array
	e->MovRegReg32 ( RAX, RCX );
	//e->MovRegReg32 ( RAX, RDX );
	e->ShrRegImm32 ( RAX, 22 );
	//e->AddRegReg32 ( RAX, RAX );
	e->ShlRegImm32 ( RAX, 2 );
	
	// get the pointer into the device pointer array
	//e->MovRegImm64 ( 9, (u64) & Playstation1::DataBus::LUT_DataBus_Read );
	e->LeaRegMem64 ( 9, & Playstation1::DataBus::LUT_DataBus_Read );
	
	
			// load the pointer into the device into RCX
			// save RCX into R9 first, though
			//e->MovRegReg32 ( 9, RCX );
			e->MovRegFromMem64 ( 10, 9, RAX, SCALE_EIGHT, 0 );
			
			// jump if zero
			//e->CmpReg64ImmX ( 10, 0 );
			e->OrRegReg64 ( 10, 10 );
	
	
	//e->Jmp_E ( 0, 0 );
	e->Jmp8_E ( 0, 4 );

	
	//e->TestRegMem32 ( RDX, 9, RAX, SCALE_EIGHT, 24 );
	e->TestRegMem32 ( RCX, 9, RAX, SCALE_EIGHT, 24 );
	e->Jmp8_NE ( 0, 7 );
	
	
	if ( i.Rt || ( i.Opcode == OPLWC2 ) )
	{
		
	// get pointer into invalidate array
	//e->MovRegFromMem64 ( 11, 9, RAX, SCALE_EIGHT, 16 );

	// mask address
	e->AndRegMem32 ( RCX, 9, RAX, SCALE_EIGHT, 8 );
	//e->AndRegMem32 ( RDX, 9, RAX, SCALE_EIGHT, 8 );

	// get the latency
	e->MovRegFromMem32 ( 9, 9, RAX, SCALE_EIGHT, 12 );
	

	
	
	// store the value
	switch ( i.Opcode )
	{
		case OPLBU:
			//e->MovRegFromMem8 ( RAX, 10, RCX, SCALE_NONE, 0 );
			e->MovzxReg32Mem8 ( RAX, 10, RCX, SCALE_NONE, 0 );
			break;
		case OPLB:
			//e->MovRegFromMem8 ( RAX, 10, RCX, SCALE_NONE, 0 );
			e->MovsxReg32Mem8 ( RAX, 10, RCX, SCALE_NONE, 0 );
			break;
		//case OPLBU:
		//case OPLB:
		//	e->MovRegFromMem8 ( RAX, 10, RCX, SCALE_NONE, 0 );
		//	break;
		case OPLHU:
			//e->MovRegFromMem16 ( RAX, 10, RCX, SCALE_NONE, 0 );
			e->MovzxReg32Mem16 ( RAX, 10, RCX, SCALE_NONE, 0 );
			break;
		case OPLH:
			//e->MovRegFromMem16 ( RAX, 10, RCX, SCALE_NONE, 0 );
			e->MovsxReg32Mem16 ( RAX, 10, RCX, SCALE_NONE, 0 );
			break;
		//case OPLHU:
		//case OPLH:
		//	e->MovRegFromMem16 ( RAX, 10, RCX, SCALE_NONE, 0 );
		//	break;
		case OPLWL:
#ifdef USE_SHORT_LWL_CODE
			e->MovRegReg32 ( RAX, RCX );
			e->AndReg32ImmX ( RAX, ~3 );
			e->MovRegFromMem32 ( RAX, 10, RAX, SCALE_NONE, 0 );
			break;
#endif

		case OPLWR:
#ifdef USE_SHORT_LWR_CODE
			e->MovRegReg32 ( RAX, RCX );
			e->AndReg32ImmX ( RAX, ~3 );
			e->MovRegFromMem32 ( RAX, 10, RAX, SCALE_NONE, 0 );
			break;
#else
			//e->MovRegReg32 ( RCX, RDX );
			//e->AndReg32ImmX ( RDX, ~3 );
			e->MovRegReg32 ( RDX, RCX );
			e->AndReg32ImmX ( RCX, ~3 );
#endif
		case OPLW:
		case OPLWC2:
			//e->MovRegFromMem32 ( RAX, 10, RDX, SCALE_NONE, 0 );
			e->MovRegFromMem32 ( RAX, 10, RCX, SCALE_NONE, 0 );
			break;
	}
	
	
	switch ( i.Opcode )
	{
		case OPLWL:
#ifdef USE_SHORT_LWL_CODE
			break;
#endif

		case OPLWR:
#ifdef USE_SHORT_LWR_CODE
#else
			e->MovRegReg32 ( RCX, RDX );
#endif
			break;
	}
	
	// also need to invalidate cache
	//e->ShrRegImm32 ( RCX, 2 + r->Bus->c_iInvalidate_Shift );
	//e->MovMemImm8 ( 1, 11, RCX, SCALE_NONE, 0 );
	//e->ShrRegImm32 ( RDX, 2 + r->Bus->c_iInvalidate_Shift );
	//e->MovMemImm8 ( 1, 11, RDX, SCALE_NONE, 0 );
	
	} // end if ( i.Rt || ( i.Opcode == OPLWC2 ) )

	
	
	// add additional latency
	e->AddMemReg64 ( (long long*) & r->CycleCount, 9 );

	// testing
	//e->MovRegMem32 ( RCX, & r->Bus->MainMemory.b32 [ ( /*Address*/ 0x588 & r->Bus->MainMemory_Mask ) >> 2 ] );
	//e->MovMemReg32 ( & r->testvar [ 4 ], RCX );
	
	e->Jmp8 ( 0, 5 );
	
		
	} // end if ( bPerformInlineLoad )

		
#else
	e->Jmp8 ( 0, 4 );

#endif

	} // end if ( ( ( 1 << i.Rt ) & GetSourceRegs ( NextInst, Address + 4 ) & ~1 ) || ( NextInst.Value == -1 ) || ( ( 1 << i.Rt ) & Get_DelaySlot_DestRegs ( NextInst ) ) )

#endif

		
	
#ifdef CHECK_EVENT_AFTER_START
	//if ( !e->SetJmpTarget ( 0 ) )
	if ( !e->SetJmpTarget8 ( 0 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch0 too far!";
	}
	
	// update NextPC
	e->MovMemImm32 ( (long*) & r->NextPC, Address );
	
	// update CPU CycleCount
	//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
	e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles );
	//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles - ExeCycles );
	
	// done for now - return
	e->Ret ();
#endif

	
	if ( BitTest )
	{
	//if ( !e->SetJmpTarget ( 1 ) )
	if ( !e->SetJmpTarget8 ( 1 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch1 too far!";
	}
	
		// update CycleCount, set PC, then jump to synchronous interrupt
		//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
		e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount );
		
		// set pc
		e->MovMemImm32 ( (long*) & r->PC, Address );
		
		//r->ProcessSynchronousInterrupt ( Cpu::EXC_ADES );
		e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_ADEL> );
	}


	if ( bNoConflict )
	{
		
#ifdef ENABLE_INLINE_LOAD
	if ( !e->SetJmpTarget8 ( 4 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch4 too far!";
	}
	if ( !e->SetJmpTarget8 ( 7 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch7 too far!";
	}
#endif
			
		//e->MovRegToMem32 ( & r->DelaySlots [ 1 ].Data, RDX );
		//e->MovMemImm32 ( & r->DelaySlots [ 1 ].Instruction.Value, i.Value );
		
			// execute the load immediately
			//e->MovMemImm32 ( & r->NextDelaySlotIndex, 1 );
			//ret = e->MovMemImm32 ( & r->LastModifiedRegister, 255 );
			
			switch ( i.Opcode )
			{
				case OPLWL:
				case OPLWR:
					e->AndReg32ImmX ( RCX, ~3 );
					break;
			}
			
			
			// encode the load
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			//ret = e->Call ( LoadFunctionToCall );
			ret = e->Call ( LoadFunctionImmediate );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// reload address if needed
			switch ( i.Opcode )
			{
				case OPLWL:
				case OPLWR:
					e->MovRegFromMem32 ( RCX, &r->GPR [ i.Base ].s );
					e->AddReg32ImmX ( RCX, i.sOffset );
					break;
					
				// opcodes like lb and lh need to be sign extended! //
				case OPLB:
					e->Cbw();
				case OPLH:
					e->Cwde();
					break;
					
				case OPLBU:
					e->MovzxReg32Reg8 ( RAX, RAX );
					break;
					
				case OPLHU:
					e->MovzxReg32Reg16 ( RAX, RAX );
					break;
			}

			// add additional latency (for register operation)
			e->AddMem64ImmX ( (long long*) & r->CycleCount, 3 );


	
	
	//e->Jmp8 ( 0, 2 );
	

		
	if ( !e->SetJmpTarget8 ( 2 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch2 too far!";
	}
	if ( !e->SetJmpTarget8 ( 5 ) )
	{
		cout << "\nhps1x64: R3000A: Recompiler: short branch5 too far!";
	}
	
	//if ( i.Rt )
	if ( i.Rt || ( i.Opcode == OPLWC2 ) )
	{
		
	// write back to register
	switch ( i.Opcode )
	{
		case OPLBU:
			//e->AndReg32ImmX ( RAX, 0xff );
			e->MovMemReg32 ( & r->GPR [ i.Rt ].s, RAX );
			break;
			
		case OPLHU:
			//e->AndReg32ImmX ( RAX, 0xffff );
			e->MovMemReg32 ( & r->GPR [ i.Rt ].s, RAX );
			break;
			
		case OPLWC2:
			e->MovMemReg32 ( (long*) & r->COP2.CPR2.Regs [ i.Rt ], RAX );
			break;

		case OPLWL:
#ifdef USE_SHORT_LWL_CODE
			e->LeaRegMem64 ( RDX, & r->GPR [ i.Rt ].s );
			e->MovRegFromMem32 ( 8, RDX, NO_INDEX, SCALE_NONE, 4 );
			e->NotReg32 ( RCX );
			e->AndReg32ImmX ( RCX, 3 );
			e->MovRegToMem32 ( RAX, RDX, RCX, SCALE_NONE, 0 );
			e->MovRegToMem32 ( 8, RDX, NO_INDEX, SCALE_NONE, 4 );
#else
			//e->MovRegReg32 ( RCX, RDX );
			e->NotReg32 ( RCX );
			e->AndReg32ImmX ( RCX, 3 );
			e->ShlRegImm32 ( RCX, 3 );
			e->ShlRegReg32 ( RAX );
			e->MovRegReg32 ( RDX, RAX );
			e->MovReg32ImmX ( RAX, -1 );
			e->ShlRegReg32 ( RAX );
			e->NotReg32 ( RAX );
			e->AndRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
			e->OrRegReg32 ( RAX, RDX );
			ret = e->MovMemReg32 ( &r->GPR [ i.Rt ].s, RAX );
#endif
			break;
			
		case OPLWR:
#ifdef USE_SHORT_LWR_CODE
			e->LeaRegMem64 ( RDX, & r->GPR [ i.Rt ].s );
			e->MovRegFromMem32 ( 8, RDX, NO_INDEX, SCALE_NONE, -4 );
			e->AndReg32ImmX ( RCX, 3 );
			e->NegReg64 ( RCX );
			e->MovRegToMem32 ( RAX, RDX, RCX, SCALE_NONE, 0 );
			e->MovRegToMem32 ( 8, RDX, NO_INDEX, SCALE_NONE, -4 );
#else
			//e->MovRegReg32 ( RCX, RDX );
			e->AndReg32ImmX ( RCX, 3 );
			e->ShlRegImm32 ( RCX, 3 );
			e->ShrRegReg32 ( RAX );
			//e->Cdqe ();
			e->MovRegReg32 ( RDX, RAX );
			e->MovReg32ImmX ( RAX, -1 );
			e->ShrRegReg32 ( RAX );
			//e->Cdqe ();
			e->NotReg32 ( RAX );
			e->AndRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
			e->OrRegReg32 ( RAX, RDX );
			ret = e->MovMemReg32 ( &r->GPR [ i.Rt ].s, RAX );
#endif
			break;
			
		case OPLB:
			//e->Cbw ();
		case OPLH:
			//e->Cwde ();
		default:
			e->MovMemReg32 ( & r->GPR [ i.Rt ].s, RAX );
			break;
	}
	
	} // end if ( i.Rt )
	
	} // end if ( bNoConflict )
	
	}
		
	return ret;
}


long Recompiler::Generate_Normal_Branch ( Instruction::Format i, u32 Address, void* BranchFunctionToCall )
{
	u32 TargetAddress;
	u32 *pInst;
	bool bIdle;
	long ret;
	
	
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nStart";
#endif
	
	// get the target address
	switch ( i.Opcode )
	{
		case OPJR:
		//case OPJALR:
		
			// don't know what the address is since it is variable
			TargetAddress = 0;
			break;
			
		case OPJ:
		case OPJAL:
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nAddress=" << hex << Address << " JumpAddress=" << i.JumpAddress;
#endif
			TargetAddress = ( 0xf0000000 & Address ) | ( i.JumpAddress << 2 );
			break;
			
		// must be a branch
		default:
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nAddress=" << hex << Address << " sOffset=" << i.sImmediate;
#endif
			TargetAddress = 4 + Address + ( i.sImmediate << 2 );
			break;
	}
	
	
#ifdef CHECK_EVENT_AFTER_START
	// part 1: first check for event //
	
	// get updated CycleCount value for CPU
	e->MovRegMem64 ( RAX, (long long*) & r->CycleCount );
	e->AddReg64ImmX ( RAX, LocalCycleCount - ( MemCycles - 1 ) );
	//e->AddReg64ImmX ( RAX, LocalCycleCount - MemCycles );
	
	
	// want check that there are no events pending //
	
	// get the current cycle count and compare with next event cycle
	// note: actually need to either offset the next event cycle and correct when done or
	// or need to offset the next even cycle into another variable and check against that one
#ifdef PS2_COMPILE
	e->CmpRegMem64 ( RAX, (long long*) & Playstation1::System::_SYSTEM->ullStopCycle );
#else
	e->CmpRegMem64 ( RAX, (long long*) & Playstation1::System::_SYSTEM->NextEvent_Cycle );
#endif
	
	// branch if current cycle is greater (or equal?) than next event cycle
	// changing this so that it branches if not returning
	// note: should probably be below or equal then jump, since the interpreter adds one to cycle
	//e->Jmp8_B ( 0, 0 );
	//e->Jmp8_AE ( 0, 3 );
	e->Jmp_AE ( 0, 3 );
	

	// this is where the entry point should be if this is the first instruction in the run
	pCodeStart [ BlockIndex ] = (u8*) e->Get_CodeBlock_CurrentPtr ();
#endif

	// check for sychronous interrupt if applicable
	if ( i.Opcode == OPJR )
	{
		// get the address being jumped to
		e->MovRegMem32 ( RDX, & r->GPR [ i.Rs ].s );
		
		// if ( StoreAddress & 0x1 )
		//e->TestReg32ImmX ( RDX, BitTest );
		e->TestReg32ImmX ( RDX, 0x3 );

		// branch if zero
		//e->Jmp8_E ( 0, 0 );
		e->Jmp8_NE ( 0, 4 );
		
	}



	// check if branching or not
	switch ( i.Opcode )
	{
		case OPBEQ:
			if ( i.Rs != i.Rt )
			{
			if ( !i.Rs || !i.Rt )
			{
			e->CmpMem32ImmX ( & r->GPR [ i.Rs + i.Rt ].s, 0 );
			e->Jmp_NE ( 0, 0 );
			}
			else
			{
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].s );
			e->CmpMemReg32 ( & r->GPR [ i.Rt ].s, RCX );
			//e->Jmp8_NE ( 0, 0 );
			e->Jmp_NE ( 0, 0 );
			}
			}
			break;
			
		case OPBNE:
			if ( !i.Rs || !i.Rt )
			{
			e->CmpMem32ImmX ( & r->GPR [ i.Rs + i.Rt ].s, 0 );
			e->Jmp_E ( 0, 0 );
			}
			else
			{
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rs ].s );
			e->CmpMemReg32 ( & r->GPR [ i.Rt ].s, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp_E ( 0, 0 );
			}
			break;
			
		case OPBLEZ:
			e->CmpMem32ImmX ( & r->GPR [ i.Rs ].s, 0 );
			//e->Jmp8_G ( 0, 0 );
			e->Jmp_G ( 0, 0 );
			break;
			
		case OPBGTZ:
			e->CmpMem32ImmX ( & r->GPR [ i.Rs ].s, 0 );
			//e->Jmp8_LE ( 0, 0 );
			e->Jmp_LE ( 0, 0 );
			break;
			
		case OPBLTZ:
		//case OPBGEZ:
		//case OPBLTZAL:
		//case OPBGEZAL:
			switch ( i.Rt )
			{
				case RTBLTZ:
					e->CmpMem32ImmX ( & r->GPR [ i.Rs ].s, 0 );
					//e->Jmp8_GE ( 0, 0 );
					e->Jmp_GE ( 0, 0 );
					break;
				case RTBGEZ:
					e->CmpMem32ImmX ( & r->GPR [ i.Rs ].s, 0 );
					//e->Jmp8_L ( 0, 0 );
					e->Jmp_L ( 0, 0 );
					break;
			
				case RTBLTZAL:
					e->CmpMem32ImmX ( & r->GPR [ i.Rs ].s, 0 );
					e->MovMemImm32 ( & r->GPR [ 31 ].s, Address + 8 );
					//e->Jmp8_GE ( 0, 0 );
					e->Jmp_GE ( 0, 0 );
					break;
			
				case RTBGEZAL:
					e->CmpMem32ImmX ( & r->GPR [ i.Rs ].s, 0 );
					e->MovMemImm32 ( & r->GPR [ 31 ].s, Address + 8 );
					//e->Jmp8_L ( 0, 0 );
					e->Jmp_L ( 0, 0 );
					break;
			}
			
			break;
			
		case OPJAL:
			e->MovMemImm32 ( & r->GPR [ 31 ].s, Address + 8 );
			break;
			
		case OPJALR:
		
			if ( i.Funct == 9 )
			{
				// JALR //
				
				// make sure Rd is not r0
				if ( i.Rd )
				{
					// save return address in Rd
					e->MovMemImm32 ( & r->GPR [ i.Rd ].s, Address + 8 );
				}
			}
			
			break;
	}
	
	
	// branching //


#ifdef PS2_COMPILE
#ifdef ENABLE_CPU_IDLE
	// check for idle
	// make sure jump target is backwards
	if ( TargetAddress <= Address )
	{
		// make sure target address is in same cache block
		if ( ( TargetAddress >> 4 ) == ( Address >> 4 ) )
		{
			// make next address is in the same cache block
			if ( ( Address >> 4 ) == ( ( Address + 4 ) >> 4 ) )
			{
				// make sure next instruction is nop
				if ( !NextInst.Value )
				{
					bIdle = true;
					
					// make sure all instructions in range of jump are nop except for jump itself
					if ( TargetAddress != Address )
					{
						for ( u32 Addr = TargetAddress; Addr < Address; Addr += 4 )
						{
							pInst = RGetPointer ( Addr );
							
							if ( *pInst )
							{
								bIdle = false;
							}
							
							Addr += 4;
						}
					}
					
					// if idle, then set flag
					if ( bIdle )
					{
						// mark as idle
						e->MovMemImm32 ( & r->ulWaitingForInterrupt, 1 );
						
						//cout << "\nhps1x64: R3000A: Recompiler: CPU Idle detected. Address=" << hex << Address;
					}
				}
			}
		}
	}
#endif
#endif


	
	
#ifdef ALLOW_ENCODING_DELAYSLOT
	// check if target address is inside current block or not
	// for now, only check for only jumping backwards
	//if ( TargetAddress && TargetAddress >= CurrentBlock_StartAddress && TargetAddress < NextBlock_StartAddress && isBranchDelayOk ( NextInst.Value ) )
	if (
/*
#ifdef ENABLE_AUTO_BRANCH
		( TargetAddress && TargetAddress >= CurrentBlock_StartAddress && TargetAddress <= Address && isBranchDelayOk ( NextInst.Value, Address + 4 ) ) ||
#endif
*/
		isBranchDelayOk ( NextInst.Value, Address + 4 )
		)
	{
		// target can be reached with jump in same block //
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nisBranchDelayOk";
#endif
		
		// check if this is a cached region
		if ( bIsBlockInICache )
		{
			// region is cached //
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nbIsBlockInICache";
#endif
			
			// check if instruction in delay slot is in another cache block
			if ( ! ( ( Address + 4 ) & 0xf ) )
			{
				// instruction in delay slot is in next cache block //
				
				// check if cache block is loaded
				e->CmpMem32ImmX ( (long*) & r->ICache.ICacheBlockSource [ ( ( Address + 4 ) >> 4 ) & 0xff ], ( Address + 4 ) & 0x1ffffff0 );
				
				// jump if the cache line is not loaded
				//e->Jmp8_NE ( 0, 1 );
				e->Jmp_NE ( 0, 1 );
				
				// the cache-line is loaded //
				
			}

/*
#ifdef ENABLE_AUTO_BRANCH
			// check if that cache line is loaded we are jumping to
			e->CmpMem32ImmX ( & r->ICache.ICacheBlockSource [ ( TargetAddress >> 4 ) & 0xff ], TargetAddress & 0x1ffffff0 );
			
			e->Jmp8_E ( 0, 2 );
#endif
*/
		}

//#ifndef ENABLE_AUTO_BRANCH


		// check for processor waiting
		if ( ! NextInst.Value )
		{
			// make sure not jr //
			if ( i.Opcode != OPJR )
			{
				u8* pBranchedTo;
				u8* pBranch;
				
				// get the pointer being branched to
				pBranchedTo = pCodeStart [ ( TargetAddress >> 2 ) & ulIndex_Mask ];
				
				// get the pointer to the branch
				pBranch = pCodeStart [ ( Address >> 2 ) & ulIndex_Mask ];
				
				// check if equal
				if ( pBranchedTo == pBranch )
				{
					// update cycles
					e->MovMemImm32 ( (long*) & r->NextPC, TargetAddress );

					// update CycleCount
					// ***todo*** should be the cycles plus one instruction since branch into delay slot has already been executed
					//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount );
					e->MovRegMem64 ( RCX, (long long*) & r->CycleCount );
					e->AddReg64ImmX( RCX, LocalCycleCount );
					
					// check against the next event cycle
					e->CmpRegMem64 ( RCX, (long long*) & Playstation1::System::_SYSTEM->NextEvent_Cycle );
					e->CmovBRegMem64 ( RCX, (long long*) & Playstation1::System::_SYSTEM->NextEvent_Cycle );
					
					// store as the new cycle count
					e->MovMemReg64 ( (long long*) & r->CycleCount, RCX );
					
					// should be done
					e->Ret ();
					
				}
			}
		}



#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nbRecompile";
#endif
		// no auto branch, but cache line is loaded, so execute delay slot //
		// execute the instruction in delay slot
		// if it is in the same cache block or its cache block is loaded
		// note: if this is a mult/div then need to update the CPU cycles both before and after ?
		ret = Recompile ( NextInst, Address + 4 );
		
		if ( ret <= 0 )
		{
			cout << "\nR3000A: Recompiler: Error encoding branch in delay slot.";
		}


#ifdef ENABLE_AUTO_BRANCH
		if (
			// make sure not JR
			( i.Opcode != OPJR )
			
			// make sure target address is in same cache block for now
			// staying within same cache block means this can be done purely at re-compile time
			//&& ( ( TargetAddress >> 4 ) == ( Address >> 4 ) )
			
			// and also make sure we are jumping backwards
			//&& ( TargetAddress <= Address )
			
			//&& ( ( ( Address - TargetAddress ) >> 2 ) <= RunCount )
		)
		{

			if (
				// make sure target address is in same cache block for now
				// staying within same cache block means this can be done purely at re-compile time
				( ( TargetAddress >> 4 ) == ( Address >> 4 ) )
				
				// and also make sure we are jumping backwards
				&& ( TargetAddress <= Address )
			)
			{
		

			// update the cycle count before jumping
			// +MemCycles for the delay slot memory access cycles and +1 for the delay slot execute cycles -> but +1 is already included in memcycles
			//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + MemCycles - CycleCount [ ( TargetAddress >> 2 ) & ulIndex_Mask ] + 1 );
			//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + ( MemCycles << 1 ) - CycleCount [ ( TargetAddress >> 2 ) & ulIndex_Mask ] );
			//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + ( MemCycles << 1 ) + ( ExeCycles << 1 ) - CycleCount [ ( TargetAddress >> 2 ) & ulIndex_Mask ] );
			//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + ( MemCycles << 1 ) - CycleCount [ ( TargetAddress >> 2 ) & ulIndex_Mask ] );
			
			// *** todo *** check cycle count against next event
		// get updated CycleCount value for CPU (the value as it would be after instruction executed)
		e->MovRegMem64 ( RAX, (long long*) & r->CycleCount );
		e->AddReg64ImmX ( RAX, LocalCycleCount + ( MemCycles << 1 ) );
		//e->AddReg64ImmX ( RAX, LocalCycleCount - MemCycles );
		
		
		// want check that there are no events pending //
		
		// get the current cycle count and compare with next event cycle
		// note: actually need to either offset the next event cycle and correct when done or
		// or need to offset the next even cycle into another variable and check against that one
#ifdef PS2_COMPILE
		e->CmpRegMem64 ( RAX, (long long*) & Playstation1::System::_SYSTEM->ullStopCycle );
#else
		e->CmpRegMem64 ( RAX, (long long*) & Playstation1::System::_SYSTEM->NextEvent_Cycle );
#endif
	
		// branch if current cycle is greater (or equal?) than next event cycle
		// changing this so that it branches if not returning
		// note: should probably be below or equal then jump, since the interpreter adds one to cycle
		//e->Jmp8_B ( 0, 0 );
		e->Jmp8_AE ( 0, 9 );
		//e->Jmp_AE ( 0, 0 );
		
		
		// subtract cycle count for new address
		e->MovRegMem32 ( RCX, (long*) & CycleCount [ ( TargetAddress >> 2 ) & ulIndex_Mask ] );
		e->SubRegReg64 ( RAX, RCX );
		e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
		
		// since we have not reached the next event cycle, should write back the current system cycle
		// so that the correct cycle# gets seen when the store is executed
		// no need to update the CPU cycle count until either a branch/jump is encountered or returning
		// this way, there is no need to reset the current cycle number tally unless a branch/jump is encountered
		//e->DecReg64 ( RAX );
		//e->MovMemReg64 ( & Playstation1::System::_SYSTEM->CycleCount, RAX );
			
			
			e->MovMemImm32 ( (long*) & r->NextPC, TargetAddress );
				
			//ret = e->JMP ( pPrefix_CodeStart [ ( TargetAddress >> 2 ) & MaxStep_Mask ] );
			//ret = e->JMP ( pPrefix_CodeStart [ ( TargetAddress >> 2 ) & ulIndex_Mask ] );
			ret = e->JMP ( pCodeStart [ ( TargetAddress >> 2 ) & ulIndex_Mask ] );
			
			e->SetJmpTarget8 ( 9 );
			}
			else
			{
				// get the block to encode in
				u32 tBlock;
				tBlock = ( TargetAddress >> ( 2 + MaxStep_Shift ) ) & NumBlocks_Mask;
		
		
				// step 1: check if next block has been modified (if it is in main memory)
				if ( ( TargetAddress & 0x1fc00000 ) != 0x1fc00000 )
				{
					e->CmpMemImm8 ( (char*) & r->Bus->InvalidArray.b8 [ ( TargetAddress & Playstation1::DataBus::MainMemory_Mask ) >> ( 2 + r->Bus->c_iInvalidate_Shift ) ], 0 );
					e->Jmp8_NE( 0, 6 );
				}
				
				// step 2: check if next block has the correct source address
				e->CmpMem32ImmX ( (long*) & StartAddress [ ( tBlock ) & NumBlocks_Mask ], ( TargetAddress >> ( 2 + MaxStep_Shift ) ) << ( 2 + MaxStep_Shift ) );
				e->Jmp8_NE( 0, 7 );
				
				// step 3: if next block is cached, check that it is in i-cache
				if ( bIsBlockInICache )
				{
					// get the cache line that address should be at
					//u32 ICacheBlockIndex = ( Address >> 6 ) & 0x7f;
					u32 ICacheBlockIndex = ( TargetAddress >> 4 ) & 0xff;
					
					// make room for the way
					//ICacheBlockIndex <<= 1;

					// ICacheBlockSource [ ICacheBlockIndex ] == ( Address & 0x1ffffff0 )
					//e->CmpMem32ImmX ( & r->ICache.PFN [ ICacheBlockIndex ^ 1 ], ( Address & 0x1fffffc0 ) );
					e->CmpMem32ImmX ( (long*) & r->ICache.ICacheBlockSource [ ICacheBlockIndex ], ( TargetAddress & 0x1ffffff0 ) );
					e->Jmp8_NE ( 0, 8 );
				}
		
				e->MovRegMem64 ( RAX, (long long*) & r->CycleCount );
				
				// this isn't available at re-compile time
				e->MovRegMem32 ( RCX, (long*) & CycleCount [ ( TargetAddress >> 2 ) & ulIndex_Mask ] );


				e->MovMemImm32 ( (long*) & r->NextPC, TargetAddress );
				
				// update cycle count
				//e->AddReg64ImmX ( RAX, LocalCycleCount + ( MemCycles << 1 ) + ( ExeCycles << 1 ) );
				e->AddReg64ImmX ( RAX, LocalCycleCount + ( MemCycles << 1 ) );
				
				// compare with next event
#ifdef PS2_COMPILE
		e->CmpRegMem64 ( RAX, (long long*) & Playstation1::System::_SYSTEM->ullStopCycle );
#else
		e->CmpRegMem64 ( RAX, (long long*) & Playstation1::System::_SYSTEM->NextEvent_Cycle );
#endif
				
		e->Jmp8_AE ( 0, 9 );
		
				e->SubRegReg64 ( RAX, RCX );
	
				e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );

				// step 4a: if all checks out, then jump to start of next code
				//e->JmpMem64 ( (long long*) & pPrefix_CodeStart [ ( TargetAddress >> 2 ) & ulIndex_Mask ] );
				e->JmpMem64 ( (long long*) & pCodeStart [ ( TargetAddress >> 2 ) & ulIndex_Mask ] );
				
				// step 4b: if doesn't check out, then return
				if ( ( TargetAddress & 0x1fc00000 ) != 0x1fc00000 )
				{
				e->SetJmpTarget8 ( 6 );
				}
				e->SetJmpTarget8 ( 7 );
				if ( bIsBlockInICache )
				{
				e->SetJmpTarget8 ( 8 );
				}
			e->SetJmpTarget8 ( 9 );
				
				// return //
				// did not meet all criteria for open-auto-branch
		
			}
			
			//cout << "hps1x64: R3000A: recompiler: Address=" << hex << Address << " TargetAddress=" << TargetAddress;
			e->MovMemImm32 ( (long*) & r->NextPC, TargetAddress );
			//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + ( MemCycles ) + ( ExeCycles ) );
			e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + MemCycles );
			e->Ret();
		}
		else
		{
#endif

		
		// update CPU CycleCount
		// note: must include both branch and delay slot ? And also must subtract the cyclecount at address, then add one since CycleCount[] is minus one
		// so end up adding the MemCycles for the delay slot and the ExeCycles for the branch
#ifdef ENABLE_R3000A_BRANCH_PENALTY
		e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + MemCycles + r->c_ullLatency_PipelineRefill );
#else
		e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + MemCycles );
		//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + MemCycles + ExeCycles );
#endif
		
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nTargetAddress=" << hex << TargetAddress;
#endif
		
		if ( TargetAddress )
		{
			// update NextPC
			e->MovMemImm32 ( (long*) & r->NextPC, TargetAddress );
		}
		else
		{
			if ( i.Opcode == OPJR )
			{
				//e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].s );
				e->MovMemReg32 ( (long*) & r->NextPC, RDX );
			}
			else
			{
				// ???
				cout << "\nR3000A: Recompiler: Problem setting NextPC for branch after delay slot.";
			}
		}
		
		// done - return
		e->Ret ();

#ifdef ENABLE_AUTO_BRANCH
		}
#endif

//#endif

#ifdef CHECK_EVENT_AFTER_START
	//if ( !e->SetJmpTarget8 ( 3 ) )
	if ( !e->SetJmpTarget ( 3 ) )
	{
		cout << "\nR3000A: Recompiler: Short branch3 too far.";
	}

	// update NextPC
	e->MovMemImm32 ( (long*) & r->NextPC, Address );
	
	// update CPU CycleCount
	//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
	e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles );
	//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount - MemCycles - ExeCycles );
	
	// done for now - return
	e->Ret ();
#endif

	
	if ( i.Opcode == OPJR )
	{
		if ( !e->SetJmpTarget8 ( 4 ) )
		{
			cout << "\nR3000A: Recompiler: Short branch4 too far.";
		}
		
		// update CycleCount, set PC, then jump to synchronous interrupt
		//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
		e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount );
		
		// set pc
		e->MovMemImm32 ( (long*) & r->PC, Address );
		
		//r->ProcessSynchronousInterrupt ( Cpu::EXC_ADEL );
		e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_ADEL> );
	}
	
		
		// the cache-line is not loaded //
		
		// also jump here from above if needed
		//if ( !e->SetJmpTarget8 ( 1 ) )
		if ( !e->SetJmpTarget ( 1 ) )
		{
			cout << "\nR3000A: Recompiler: Short branch1 too far.";
		}
		
		
		// put branch into delay slot
		
		// first put in the target address
		switch ( i.Opcode )
		{
			case OPJR:
			//case OPJALR:
				e->MovMemReg32 ( (long*) & r->DelaySlots [ 1 ].Data, RDX );
				break;
				
			default:
				e->MovMemImm32 ( (long*) & r->DelaySlots [ 1 ].Data, TargetAddress );
				break;
		}
		
		// put in the instruction
		e->MovMemImm32 ( (long*) & r->DelaySlots [ 1 ].Instruction.Value, i.Value );
		
		e->MovReg64ImmX ( RAX, (u64) BranchFunctionToCall );
		e->MovMemReg64 ( (long long*) & r->DelaySlots [ 1 ].cb, RAX );
		
		e->MovMemImm32 ( (long*) & r->NextDelaySlotIndex, 0 );
		e->OrMem64ImmX ( (long long*) &r->Status.Value, 2 );
		
		// important? - must update LastPC? PC? when releasing to a branch delay slot?
		e->MovMemImm32 ( (long*) & r->PC, Address );

		// update NextPC,CycleCount
#ifdef UPDATE_BEFORE_RETURN
		// update NextPC
		// it should have already returned if NextPC was modified through other means so this should be ok
		// NextPC is Address+4 here because the current instruction was already executed
		e->MovMemImm32 ( (long*) & r->NextPC, Address + 4 );
				
		// update CycleCount
		// ***todo*** should be the cycles plus one instruction since branch into delay slot has already been executed
		e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount );
#endif
				
		// done - return
		ret = e->Ret ();
				
#ifdef ENABLE_AUTO_BRANCH
		// cache-block is loaded and can continue //
		e->SetJmpTarget8 ( 2 );
		
		
		// execute the instruction in delay slot
		// if it is in the same cache block or its cache block is loaded
		// note: if this is a mult/div then need to update the CPU cycles both before and after ?
		ret = Recompile ( NextInst, Address + 4 );
		
		// update CPU CycleCount
		// note: must include both branch and delay slot ? And also must subtract the cyclecount at address, then add one since CycleCount[] is minus one
		e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + ( MemCycles << 1 ) - CycleCount [ ( TargetAddress >> 2 ) & ulIndex_Mask ] + 1 );
		
		// check if jumping backwards or forwards
		if ( TargetAddress <= Address )
		{
			// backward branch //
			ret = e->JMP ( pCodeStart [ ( TargetAddress >> 2 ) & ulIndex_Mask ] );
		}
		else
		{
			// forward branch //
			e->Jmp ( 0, ForwardBranchIndex );
			
			// set the label for that line in code
			pForwardBranchTargets [ ( TargetAddress >> 2 ) & MaxStep_Mask ] = ForwardBranchIndex;
			
			// next time we'll use the next index
			ForwardBranchIndex++;
		}
#endif
	}
	else
#endif
	{
		// not directly branching to target address right now //
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nIntoDelaySlot";
#endif
		
		// put branch into delay slot
		//e->MovMemImm32 ( & r->DelaySlots [ 1 ].Data, TargetAddress );
		//e->MovMemImm32 ( & r->DelaySlots [ 1 ].Instruction.Value, i.Value );

		// first put in the target address
		switch ( i.Opcode )
		{
			case OPJR:
			//case OPJALR:
				e->MovMemReg32 ( (long*) & r->DelaySlots [ 1 ].Data, RDX );
				break;
				
			default:
				e->MovMemImm32 ( (long*) & r->DelaySlots [ 1 ].Data, TargetAddress );
				break;
		}
		
		// put in the instruction
		e->MovMemImm32 ( (long*) & r->DelaySlots [ 1 ].Instruction.Value, i.Value );

		
		e->MovReg64ImmX ( RAX, (u64) BranchFunctionToCall );
		e->MovMemReg64 ( (long long*) & r->DelaySlots [ 1 ].cb, RAX );
		
		e->MovMemImm32 ( (long*) & r->NextDelaySlotIndex, 0 );
		e->OrMem64ImmX ( (long long*) &r->Status.Value, 2 );
		
		// important? - must update LastPC? PC? when releasing to a branch delay slot?
		e->MovMemImm32 ( (long*) & r->PC, Address );

		// update NextPC,CycleCount
#ifdef UPDATE_BEFORE_RETURN
		// update NextPC
		// it should have already returned if NextPC was modified through other means so this should be ok
		// NextPC is Address+4 here because the current instruction was already executed
		e->MovMemImm32 ( (long*) & r->NextPC, Address + 4 );
				
		// update CycleCount
		// ***todo*** should be the cycles plus one instruction since branch into delay slot has already been executed
		e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount );
#endif
				
		// done - return
		ret = e->Ret ();
	}
	
	// if inside current block, then check if cache line is loaded
	// if cache line is loaded, then jump to it, otherwise put branch in delay slot and return
	
	// not branching //
	/*
	if ( !e->SetJmpTarget8 ( 0 ) )
	{
		cout << "\nR3000A: Recompiler: Short branch0 too far.";
	}
	*/
	
	if ( !e->SetJmpTarget ( 0 ) )
	{
		cout << "\nR3000A: Recompiler: Short branch0 too far.";
	}
	
#ifdef VERBOSE_NORMAL_BRANCH
cout << "\nEND";
#endif

	// done
	return ret;
}




// regular arithemetic //

// *** todo *** no need to save LastModifiedRegister unless instruction is KNOWN to be in a delay slot on run
long Recompiler::ADDU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "ADDU";
	static const void* c_vFunction = (const void*) Instruction::Execute::ADDU;
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	//r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u + r->GPR [ i.Rt ].u;
	//CHECK_DELAYSLOT ( i.Rd );
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( ( !i.Rs ) && ( !i.Rt ) )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( ( !i.Rs ) || ( !i.Rt ) )
				{
					if ( i.Rd != ( i.Rs | i.Rt ) )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs | i.Rt ].s );
						ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else if ( i.Rd == i.Rs )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->AddMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rt )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->AddMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rs == i.Rt )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->AddRegReg32 ( RAX, RAX );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->AddRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
#ifdef USE_NEW_ADDU_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( isBothConst( i.Rs, i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, GetConst( i.Rs ) + GetConst( i.Rt ) );
				}
				else if ( isEitherConst ( i.Rs, i.Rt ) )
				{
					// get const in Reg2
					Reg2 = SelectConst( i.Rs, i.Rt );
					
					// get the other register in Reg1
					if ( Reg2 == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
					// get the constnnt
					lConst = GetConst( Reg2 );
					
					if ( ( ( Reg1 == i.Rd ) && lConst ) || ( isAlloc( Reg1 ) && isDisposable( Reg1 ) ) )
					{
						if ( Reg1 == i.Rd )
						{
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
						}
						else
						{
							Rd = RenameReg( i.Rd, Reg1 );
						}
						
						e->AddReg32ImmX( Rd, lConst );
					}
					else
					{
						Rd = Alloc_DstReg( i.Rd );
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							e->LeaRegRegImm32( Rd, Rs, lConst );
						}
						else
						{
							e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
							e->AddReg32ImmX ( Rd, lConst );
						}
					}
				}
				else
				{
					// all registers, no constants //
					
					if ( ( i.Rd == i.Rs ) || ( i.Rd == i.Rt ) )
					{
						// 2 op, all registers //
						
						if ( i.Rd == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) || ( Reg1 == i.Rd ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							e->AddRegReg32( Rd, Rs );
						}
						else
						{
							// register is not alloc and is disposable //
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							e->AddRegMem32 ( Rd, & r->GPR [ Reg1 ].s );
						}
						
					}
					else
					{
						// 3 op, all different registers //
						
						if ( isBothAlloc( i.Rs, i.Rt ) || ( ! isEitherDisposable( i.Rs, i.Rt ) ) )
						{
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_SrcReg( i.Rt );
							
							if ( isEitherDisposable( i.Rs, i.Rt ) )
							{
								Reg1 = SelectDisposable( i.Rs, i.Rt );
								
								if ( Reg1 == i.Rs ) Reg2 = i.Rt; else Reg2 = i.Rs;
								
								Rs = Alloc_SrcReg( Reg2 );
								
								Rd = RenameReg( i.Rd, Reg1 );
								e->AddRegReg32( Rd, Rs );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->LeaRegRegReg32( Rd, Rs, Rt );
							}
						}
						else if ( isEitherAlloc( i.Rs, i.Rt ) )
						{
							Reg1 = SelectAlloc ( i.Rs, i.Rt );
							Reg2 = SelectNotAlloc( i.Rs, i.Rt );
							
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( isDisposable ( Reg1 ) )
							{
								Rd = RenameReg( i.Rd, Reg1 );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->MovRegReg32 ( Rd, Rs );
							}
							
							e->AddRegMem32( Rd, & r->GPR [ Reg2 ].s );
						}
						else
						{
							Rd = Alloc_DstReg( i.Rd );
							e->MovRegMem32( Rd, & r->GPR [ i.Rs ].s );
							e->AddRegMem32( Rd, & r->GPR [ i.Rt ].s );
						}
					}
				}
			}
				
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::SUBU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SUBU";
	static const void* c_vFunction = (const void*) Instruction::Execute::SUBU;
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	//r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u - r->GPR [ i.Rt ].u;
	//CHECK_DELAYSLOT ( i.Rd );
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( !i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
						ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else if ( i.Rd == i.Rs )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->SubMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( !i.Rs )
				{
					if ( i.Rd != i.Rt )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
						e->NegReg32 ( RAX );
						ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					}
					else
					{
						ret = e->NegMem32 ( &r->GPR [ i.Rd ].s );
					}
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->SubRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
#ifdef USE_NEW_SUBU_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					Alloc_Const ( i.Rd, 0 );
				}
				if ( isBothConst( i.Rs, i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, GetConst( i.Rs ) - GetConst( i.Rt ) );
				}
				else if ( isEitherConst ( i.Rs, i.Rt ) )
				{
					// get const in Reg2
					Reg2 = SelectConst( i.Rs, i.Rt );
					
					// get the other register in Reg1
					if ( Reg2 == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
					// get the constnnt
					lConst = GetConst( Reg2 );
					
					if ( ( Reg1 == i.Rd ) || ( isAlloc( Reg1 ) && isDisposable( Reg1 ) ) )
					{
						if ( Reg1 == i.Rd )
						{
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
						}
						else
						{
							Rd = RenameReg( i.Rd, Reg1 );
						}
						
						if( Reg1 == i.Rs )
						{
							e->SubReg32ImmX( Rd, lConst );
						}
						else
						{
							e->NegReg32( Rd );
							e->AddReg32ImmX( Rd, lConst );
						}
						
					}
					else
					{
						Rd = Alloc_DstReg( i.Rd );
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							
							if( Reg1 == i.Rs )
							{
								e->LeaRegRegImm32( Rd, Rs, -lConst );
							}
							else
							{
								e->MovRegReg32( Rd, Rs );
								e->NegReg32( Rd );
								e->AddReg32ImmX( Rd, lConst );
							}
						}
						else
						{
							e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
							
							if( Reg1 == i.Rs )
							{
								e->SubReg32ImmX( Rd, lConst );
							}
							else
							{
								e->NegReg32( Rd );
								e->AddReg32ImmX( Rd, lConst );
							}
						}
					}
				}
				else
				{
					// all registers, no constants //
					
					if ( ( i.Rd == i.Rs ) || ( i.Rd == i.Rt ) )
					{
						// 2 op, all registers //
						
						if ( i.Rd == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );

							if( i.Rd == i.Rs )
							{
								e->SubRegReg32( Rd, Rs );
							}
							else
							{
								e->NegReg32( Rd );
								e->AddRegReg32( Rd, Rs );
							}
						}
						else
						{
							// register is not alloc and is disposable //
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							
							if( i.Rd == i.Rs )
							{
								e->SubRegMem32( Rd, & r->GPR [ Reg1 ].s );
							}
							else
							{
								e->NegReg32( Rd );
								e->AddRegMem32( Rd, & r->GPR [ Reg1 ].s );
							}
						}
						
					}
					else
					{
						// 3 op, all different registers //
						
						if ( isBothAlloc( i.Rs, i.Rt ) || ( ! isEitherDisposable( i.Rs, i.Rt ) ) )
						{
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_SrcReg( i.Rt );
							
							if ( isEitherDisposable( i.Rs, i.Rt ) )
							{
								Reg1 = SelectDisposable( i.Rs, i.Rt );
								
								if ( Reg1 == i.Rs ) Reg2 = i.Rt; else Reg2 = i.Rs;
								
								Rs = Alloc_SrcReg( Reg2 );
								
								Rd = RenameReg( i.Rd, Reg1 );
								
								if ( Reg1 == i.Rs )
								{
									e->SubRegReg32 ( Rd, Rs );
								}
								else
								{
									e->NegReg32( Rd );
									e->AddRegReg32( Rd, Rs );
								}
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								
								e->MovRegReg32( Rd, Rs );
								e->SubRegReg32( Rd, Rt );
							}
						}
						else if ( isEitherAlloc( i.Rs, i.Rt ) )
						{
							Reg1 = SelectAlloc ( i.Rs, i.Rt );
							Reg2 = SelectNotAlloc( i.Rs, i.Rt );
							
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( isDisposable ( Reg1 ) )
							{
								Rd = RenameReg( i.Rd, Reg1 );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->MovRegReg32 ( Rd, Rs );
							}
							
							if ( Reg1 == i.Rs )
							{
								e->SubRegMem32( Rd, & r->GPR [ Reg2 ].s );
							}
							else
							{
								e->NegReg32( Rd );
								e->AddRegMem32( Rd, & r->GPR [ Reg2 ].s );
							}
						}
						else
						{
							Rd = Alloc_DstReg( i.Rd );
							e->MovRegMem32( Rd, & r->GPR [ i.Rs ].s );
							e->SubRegMem32( Rd, & r->GPR [ i.Rt ].s );
						}
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::AND ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "AND";
	static const void* c_vFunction = (const void*) Instruction::Execute::AND;
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( ( !i.Rs ) || ( !i.Rt ) )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( i.Rs == i.Rt )
				{
					if ( i.Rd != i.Rs )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
						ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else if ( i.Rd == i.Rs )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->AndMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rt )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->AndMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->AndRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				
				// note: last modified register can be cached locally in this case, and written back after all instructions encoded
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
#ifdef USE_NEW_AND_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( ( i.Rd == i.Rs ) && ( i.Rd == i.Rt ) )
				{
					// do nothing
				}
				else if ( isBothConst( i.Rs, i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, GetConst( i.Rs ) & GetConst( i.Rt ) );
				}
				else if ( isEitherConst ( i.Rs, i.Rt ) )
				{
					// get const in Reg2
					Reg2 = SelectConst( i.Rs, i.Rt );
					
					// get the other register in Reg1
					if ( Reg2 == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
					// get the constnnt
					lConst = GetConst( Reg2 );
					
					if ( !lConst )
					{
						Alloc_Const( i.Rd, 0 );
					}
					else
					{
						if ( ( Reg1 == i.Rd ) || ( isAlloc( Reg1 ) && isDisposable( Reg1 ) ) )
						{
							if ( Reg1 == i.Rd )
							{
								Rd = Alloc_SrcReg( i.Rd );
								Rd = Alloc_DstReg( i.Rd );
							}
							else
							{
								Rd = RenameReg( i.Rd, Reg1 );
							}
							
							e->AndReg32ImmX( Rd, lConst );
						}
						else
						{
							Rd = Alloc_DstReg( i.Rd );
							
							if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
							{
								Rs = Alloc_SrcReg( Reg1 );
								e->MovRegReg32( Rd, Rs );
								e->AndReg32ImmX( Rd, lConst );
							}
							else
							{
								e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
								e->AndReg32ImmX ( Rd, lConst );
							}
						}
					}
				}
				else
				{
					// all registers, no constants //
					
					if ( ( i.Rd == i.Rs ) || ( i.Rd == i.Rt ) )
					{
						// 2 op, all registers //
						
						if ( i.Rd == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							e->AndRegReg32( Rd, Rs );
						}
						else
						{
							// register is not alloc and is disposable //
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							e->AndRegMem32 ( Rd, & r->GPR [ Reg1 ].s );
						}
					}
					else
					{
						// 3 op, all different registers //
						
						if ( isBothAlloc( i.Rs, i.Rt ) || ( ! isEitherDisposable( i.Rs, i.Rt ) ) )
						{
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_SrcReg( i.Rt );
							
							if ( isEitherDisposable( i.Rs, i.Rt ) )
							{
								Reg1 = SelectDisposable( i.Rs, i.Rt );
								
								if ( Reg1 == i.Rs ) Reg2 = i.Rt; else Reg2 = i.Rs;
								
								Rs = Alloc_SrcReg( Reg2 );
								
								Rd = RenameReg( i.Rd, Reg1 );
								e->AndRegReg32( Rd, Rs );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->MovRegReg32( Rd, Rs );
								e->AndRegReg32( Rd, Rt );
							}
						}
						else if ( isEitherAlloc( i.Rs, i.Rt ) )
						{
							Reg1 = SelectAlloc ( i.Rs, i.Rt );
							Reg2 = SelectNotAlloc( i.Rs, i.Rt );
							
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( isDisposable ( Reg1 ) )
							{
								Rd = RenameReg( i.Rd, Reg1 );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->MovRegReg32 ( Rd, Rs );
							}
							
							e->AndRegMem32( Rd, & r->GPR [ Reg2 ].s );
						}
						else
						{
							Rd = Alloc_DstReg( i.Rd );
							e->MovRegMem32( Rd, & r->GPR [ i.Rs ].s );
							e->AndRegMem32( Rd, & r->GPR [ i.Rt ].s );
						}
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::OR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "OR";
	static const void* c_vFunction = (const void*) Instruction::Execute::OR;
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( ( !i.Rs ) && ( !i.Rt ) )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( ( !i.Rs ) || ( !i.Rt ) || ( i.Rs == i.Rt ) )
				{
					if ( i.Rd != ( i.Rs | i.Rt ) )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs | i.Rt ].s );
						ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else if ( i.Rd == i.Rs )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->OrMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rt )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->OrMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->OrRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
#ifdef USE_NEW_OR_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( ( i.Rd == i.Rs ) && ( i.Rd == i.Rt ) )
				{
					// do nothing
				}
				else if ( isBothConst( i.Rs, i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, GetConst( i.Rs ) | GetConst( i.Rt ) );
				}
				else if ( isEitherConst ( i.Rs, i.Rt ) )
				{
					// get const in Reg2
					Reg2 = SelectConst( i.Rs, i.Rt );
					
					// get the other register in Reg1
					if ( Reg2 == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
					// get the constnnt
					lConst = GetConst( Reg2 );
					
					if ( ( ( Reg1 == i.Rd ) && lConst ) || ( isAlloc( Reg1 ) && isDisposable( Reg1 ) ) )
					{
						if ( Reg1 == i.Rd )
						{
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
						}
						else
						{
							Rd = RenameReg( i.Rd, Reg1 );
						}
						
						e->OrReg32ImmX( Rd, lConst );
					}
					else
					{
						Rd = Alloc_DstReg( i.Rd );
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							e->MovRegReg32( Rd, Rs );
							e->OrReg32ImmX( Rd, lConst );
						}
						else
						{
							e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
							e->OrReg32ImmX ( Rd, lConst );
						}
					}
				}
				else
				{
					// all registers, no constants //
					
					if ( ( i.Rd == i.Rs ) || ( i.Rd == i.Rt ) )
					{
						// 2 op, all registers //
						
						if ( i.Rd == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							e->OrRegReg32( Rd, Rs );
						}
						else
						{
							// register is not alloc and is disposable //
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							e->OrRegMem32 ( Rd, & r->GPR [ Reg1 ].s );
						}
					}
					else
					{
						// 3 op, all different registers //
						
						if ( isBothAlloc( i.Rs, i.Rt ) || ( ! isEitherDisposable( i.Rs, i.Rt ) ) )
						{
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_SrcReg( i.Rt );
							
							if ( isEitherDisposable( i.Rs, i.Rt ) )
							{
								Reg1 = SelectDisposable( i.Rs, i.Rt );
								
								if ( Reg1 == i.Rs ) Reg2 = i.Rt; else Reg2 = i.Rs;
								
								Rs = Alloc_SrcReg( Reg2 );
								
								Rd = RenameReg( i.Rd, Reg1 );
								e->OrRegReg32( Rd, Rs );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->MovRegReg32( Rd, Rs );
								e->OrRegReg32( Rd, Rt );
							}
						}
						else if ( isEitherAlloc( i.Rs, i.Rt ) )
						{
							Reg1 = SelectAlloc ( i.Rs, i.Rt );
							Reg2 = SelectNotAlloc( i.Rs, i.Rt );
							
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( isDisposable ( Reg1 ) )
							{
								Rd = RenameReg( i.Rd, Reg1 );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->MovRegReg32 ( Rd, Rs );
							}
							
							e->OrRegMem32( Rd, & r->GPR [ Reg2 ].s );
						}
						else
						{
							Rd = Alloc_DstReg( i.Rd );
							e->MovRegMem32( Rd, & r->GPR [ i.Rs ].s );
							e->OrRegMem32( Rd, & r->GPR [ i.Rt ].s );
						}
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::XOR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "XOR";
	static const void* c_vFunction = (const void*) Instruction::Execute::XOR;
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( ( !i.Rs ) || ( !i.Rt ) )
				{
					if ( i.Rd != ( i.Rs | i.Rt ) )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs | i.Rt ].s );
						ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else if ( i.Rd == i.Rs )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->XorMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else if ( i.Rd == i.Rt )
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					ret = e->XorMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->XorRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
#ifdef USE_NEW_XOR_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					Alloc_Const ( i.Rd, 0 );
				}
				else if ( isBothConst( i.Rs, i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, GetConst( i.Rs ) ^ GetConst( i.Rt ) );
				}
				else if ( isEitherConst ( i.Rs, i.Rt ) )
				{
					// get const in Reg2
					Reg2 = SelectConst( i.Rs, i.Rt );
					
					// get the other register in Reg1
					if ( Reg2 == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
					// get the constnnt
					lConst = GetConst( Reg2 );
					
					if ( ( ( Reg1 == i.Rd ) && lConst ) || ( isAlloc( Reg1 ) && isDisposable( Reg1 ) ) )
					{
						if ( Reg1 == i.Rd )
						{
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
						}
						else
						{
							Rd = RenameReg( i.Rd, Reg1 );
						}
						
						e->XorReg32ImmX( Rd, lConst );
					}
					else
					{
						Rd = Alloc_DstReg( i.Rd );
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							e->MovRegReg32( Rd, Rs );
							e->XorReg32ImmX( Rd, lConst );
						}
						else
						{
							e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
							e->XorReg32ImmX ( Rd, lConst );
						}
					}
				}
				else
				{
					// all registers, no constants //
					
					if ( ( i.Rd == i.Rs ) || ( i.Rd == i.Rt ) )
					{
						// 2 op, all registers //
						
						if ( i.Rd == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							e->XorRegReg32( Rd, Rs );
						}
						else
						{
							// register is not alloc and is disposable //
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							e->XorRegMem32 ( Rd, & r->GPR [ Reg1 ].s );
						}
						
					}
					else
					{
						// 3 op, all different registers //
						
						if ( isBothAlloc( i.Rs, i.Rt ) || ( ! isEitherDisposable( i.Rs, i.Rt ) ) )
						{
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_SrcReg( i.Rt );
							
							if ( isEitherDisposable( i.Rs, i.Rt ) )
							{
								Reg1 = SelectDisposable( i.Rs, i.Rt );
								
								if ( Reg1 == i.Rs ) Reg2 = i.Rt; else Reg2 = i.Rs;
								
								Rs = Alloc_SrcReg( Reg2 );
								
								Rd = RenameReg( i.Rd, Reg1 );
								e->XorRegReg32( Rd, Rs );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->MovRegReg32( Rd, Rs );
								e->XorRegReg32( Rd, Rt );
							}
						}
						else if ( isEitherAlloc( i.Rs, i.Rt ) )
						{
							Reg1 = SelectAlloc ( i.Rs, i.Rt );
							Reg2 = SelectNotAlloc( i.Rs, i.Rt );
							
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( isDisposable ( Reg1 ) )
							{
								Rd = RenameReg( i.Rd, Reg1 );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->MovRegReg32 ( Rd, Rs );
							}
							
							e->XorRegMem32( Rd, & r->GPR [ Reg2 ].s );
						}
						else
						{
							Rd = Alloc_DstReg( i.Rd );
							e->MovRegMem32( Rd, & r->GPR [ i.Rs ].s );
							e->XorRegMem32( Rd, & r->GPR [ i.Rt ].s );
						}
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NOR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NOR";
	static const void* c_vFunction = (const void*) Instruction::Execute::NOR;
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( ( !i.Rs ) && ( !i.Rt ) )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, -1 );
				}
				else if ( ( !i.Rs ) || ( !i.Rt ) || ( i.Rs == i.Rt ) )
				{
					if ( i.Rd != ( i.Rs | i.Rt ) )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs | i.Rt ].s );
						e->NotReg32 ( RAX );
						ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					}
					else
					{
						ret = e->NotMem32 ( &r->GPR [ i.Rd ].s );
					}
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->OrRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->NotReg32 ( RAX );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
#ifdef USE_NEW_NOR_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( isBothConst( i.Rs, i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, ~( GetConst( i.Rs ) | GetConst( i.Rt ) ) );
				}
				else if ( isEitherConst ( i.Rs, i.Rt ) )
				{
					// get const in Reg2
					Reg2 = SelectConst( i.Rs, i.Rt );
					
					// get the other register in Reg1
					if ( Reg2 == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
					// get the constnnt
					lConst = GetConst( Reg2 );
					
					if ( ( Reg1 == i.Rd ) || ( isAlloc( Reg1 ) && isDisposable( Reg1 ) ) )
					{
						if ( Reg1 == i.Rd )
						{
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
						}
						else
						{
							Rd = RenameReg( i.Rd, Reg1 );
						}
						
						e->OrReg32ImmX( Rd, lConst );
						e->NotReg32( Rd );
					}
					else
					{
						Rd = Alloc_DstReg( i.Rd );
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							e->MovRegReg32( Rd, Rs );
							e->OrReg32ImmX( Rd, lConst );
							e->NotReg32( Rd );
						}
						else
						{
							e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
							e->OrReg32ImmX ( Rd, lConst );
							e->NotReg32( Rd );
						}
					}
				}
				else
				{
					// all registers, no constants //
					
					if ( ( i.Rd == i.Rs ) || ( i.Rd == i.Rt ) )
					{
						// 2 op, all registers //
						
						if ( i.Rd == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) || ( Reg1 == i.Rd ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							e->OrRegReg32( Rd, Rs );
							e->NotReg32( Rd );
						}
						else
						{
							// register is not alloc and is disposable //
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							e->OrRegMem32 ( Rd, & r->GPR [ Reg1 ].s );
							e->NotReg32( Rd );
						}
						
					}
					else
					{
						// 3 op, all different registers //
						
						if ( isBothAlloc( i.Rs, i.Rt ) || ( ! isEitherDisposable( i.Rs, i.Rt ) ) )
						{
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_SrcReg( i.Rt );
							
							if ( isEitherDisposable( i.Rs, i.Rt ) )
							{
								Reg1 = SelectDisposable( i.Rs, i.Rt );
								
								if ( Reg1 == i.Rs ) Reg2 = i.Rt; else Reg2 = i.Rs;
								
								Rs = Alloc_SrcReg( Reg2 );
								
								Rd = RenameReg( i.Rd, Reg1 );
								e->OrRegReg32( Rd, Rs );
								e->NotReg32( Rd );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->MovRegReg32( Rd, Rs );
								e->OrRegReg32( Rd, Rt );
								e->NotReg32( Rd );
							}
						}
						else if ( isEitherAlloc( i.Rs, i.Rt ) )
						{
							Reg1 = SelectAlloc ( i.Rs, i.Rt );
							Reg2 = SelectNotAlloc( i.Rs, i.Rt );
							
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( isDisposable ( Reg1 ) )
							{
								Rd = RenameReg( i.Rd, Reg1 );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->MovRegReg32 ( Rd, Rs );
							}
							
							e->OrRegMem32( Rd, & r->GPR [ Reg2 ].s );
							e->NotReg32( Rd );
						}
						else
						{
							Rd = Alloc_DstReg( i.Rd );
							e->MovRegMem32( Rd, & r->GPR [ i.Rs ].s );
							e->OrRegMem32( Rd, & r->GPR [ i.Rt ].s );
							e->NotReg32( Rd );
						}
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SLT";
	static const void* c_vFunction = (const void*) Instruction::Execute::SLT;
	
	//r->GPR [ i.Rd ].s = r->GPR [ i.Rs ].s < r->GPR [ i.Rt ].s ? 1 : 0;
	//CHECK_DELAYSLOT ( i.Rd );
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( !i.Rt )
				{
					// ***todo*** implement ShrMemImm32
					//if ( i.Rd == i.Rs )
					//{
					//	e->ShrMemImm32 ( &r->GPR [ i.Rs ].s, 31 );
					//}
					//else
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
						//e->Cdq ();
						//e->NegReg32 ( RDX );
						e->ShrRegImm32 ( RAX, 31 );
						//ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RDX );
						ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else
				{
					e->XorRegReg32 ( RCX, RCX );
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->CmpRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->Set_L ( RCX );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RCX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
#ifdef USE_NEW_SLT_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					Alloc_Const( i.Rd, 0 );
				}
				else if ( isBothConst( i.Rs, i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, ( ( (s32) GetConst( i.Rs ) ) < ( (s32) GetConst( i.Rt ) ) ) ? 1 : 0 );
				}
				else if ( isEitherConst ( i.Rs, i.Rt ) )
				{
					// get const in Reg2
					Reg2 = SelectConst( i.Rs, i.Rt );
					
					// get the other register in Reg1
					if ( Reg2 == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
					// get the constnnt
					lConst = GetConst( Reg2 );
					
					if ( ( Reg1 == i.Rd ) || ( isAlloc( Reg1 ) && isDisposable( Reg1 ) ) )
					{
						if ( Reg1 == i.Rd )
						{
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
						}
						else
						{
							Rd = RenameReg( i.Rd, Reg1 );
						}
						
						e->CmpReg32ImmX( Rd, lConst );
						
						if ( Reg1 == i.Rs )
						{
							e->Set_L( Rd );
						}
						else
						{
							e->Set_G( Rd );
						}
						
						e->AndReg32ImmX( Rd, 1 );
					}
					else
					{
						Rd = Alloc_DstReg( i.Rd );
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							
							e->CmpReg32ImmX( Rs, lConst );
							
							if ( Reg1 == i.Rs )
							{
								e->Set_L( Rd );
							}
							else
							{
								e->Set_G( Rd );
							}
							
							e->AndReg32ImmX( Rd, 1 );
						}
						else
						{
							e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
							e->CmpReg32ImmX ( Rd, lConst );
							
							if ( Reg1 == i.Rs )
							{
								e->Set_L( Rd );
							}
							else
							{
								e->Set_G( Rd );
							}
							
							e->AndReg32ImmX( Rd, 1 );
						}
					}
				}
				else
				{
					// all registers, no constants //
					
					if ( ( i.Rd == i.Rs ) || ( i.Rd == i.Rt ) )
					{
						// 2 op, all registers //
						
						if ( i.Rd == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							
							e->CmpRegReg32( Rd, Rs );
							
							if ( Reg1 == i.Rt )
							{
								e->Set_L( Rd );
							}
							else
							{
								e->Set_G( Rd );
							}
							
							e->AndReg32ImmX( Rd, 1 );
						}
						else
						{
							// register is not alloc and is disposable //
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							
							e->CmpRegMem32( Rd, & r->GPR [ Reg1 ].s );
							
							if ( Reg1 == i.Rt )
							{
								e->Set_L( Rd );
							}
							else
							{
								e->Set_G( Rd );
							}
							
							e->AndReg32ImmX( Rd, 1 );
						}
						
					}
					else
					{
						// 3 op, all different registers //
						
						if ( isBothAlloc( i.Rs, i.Rt ) || ( ! isEitherDisposable( i.Rs, i.Rt ) ) )
						{
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_SrcReg( i.Rt );
							
							if ( isEitherDisposable( i.Rs, i.Rt ) )
							{
								Reg2 = SelectDisposable( i.Rs, i.Rt );
								
								if ( Reg2 == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
								
								e->CmpRegReg32( Rs, Rt );
								
								Rd = RenameReg( i.Rd, Reg2 );
								
								e->Set_L ( Rd );
								e->AndReg32ImmX( Rd, 1 );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								
								e->XorRegReg32( Rd, Rd );
								e->CmpRegReg32( Rs, Rt );
								e->Set_L ( Rd );
							}
						}
						else if ( isEitherAlloc( i.Rs, i.Rt ) )
						{
							Reg1 = SelectAlloc ( i.Rs, i.Rt );
							Reg2 = SelectNotAlloc( i.Rs, i.Rt );
							
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( isDisposable ( Reg1 ) )
							{
								Rd = RenameReg( i.Rd, Reg1 );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								//e->MovRegReg32 ( Rd, Rs );
							}
							
							e->CmpRegMem32( Rs, & r->GPR [ Reg2 ].s );
							
							if ( Reg1 == i.Rs )
							{
								e->Set_L( Rd );
							}
							else
							{
								e->Set_G( Rd );
							}
							
							e->AndReg32ImmX( Rd, 1 );
						}
						else
						{
							Rd = Alloc_DstReg( i.Rd );
							e->MovRegMem32( Rd, & r->GPR [ i.Rs ].s );
							e->CmpRegMem32( Rd, & r->GPR [ i.Rt ].s );
							e->Set_L( Rd );
							e->AndReg32ImmX( Rd, 1 );
						}
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLTU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SLTU";
	static const void* c_vFunction = (const void*) Instruction::Execute::SLTU;
	
	//r->GPR [ i.Rd ].u = r->GPR [ i.Rs ].u < r->GPR [ i.Rt ].u ? 1 : 0;
	//CHECK_DELAYSLOT ( i.Rd );
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( ( !i.Rt ) || ( i.Rs == i.Rt ) )
				{
					e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else
				{
					e->XorRegReg32 ( RCX, RCX );
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->CmpRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
					//e->Set_B ( RCX );
					e->AdcRegReg32 ( RCX, RCX );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RCX );
				}
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
#ifdef USE_NEW_SLTU_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( i.Rs == i.Rt )
				{
					Alloc_Const( i.Rd, 0 );
				}
				else if ( isBothConst( i.Rs, i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, ( ( (u32) GetConst( i.Rs ) ) < ( (u32) GetConst( i.Rt ) ) ) ? 1 : 0 );
				}
				else if ( isEitherConst ( i.Rs, i.Rt ) )
				{
					// get const in Reg2
					Reg2 = SelectConst( i.Rs, i.Rt );
					
					// get the other register in Reg1
					if ( Reg2 == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
					// get the constnnt
					lConst = GetConst( Reg2 );
					
					if ( ( Reg1 == i.Rd ) || ( isAlloc( Reg1 ) && isDisposable( Reg1 ) ) )
					{
						if ( Reg1 == i.Rd )
						{
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
						}
						else
						{
							Rd = RenameReg( i.Rd, Reg1 );
						}
						
						e->CmpReg32ImmX( Rd, lConst );
						
						if ( Reg1 == i.Rs )
						{
							e->Set_B( Rd );
						}
						else
						{
							e->Set_A( Rd );
						}
						
						e->AndReg32ImmX( Rd, 1 );
					}
					else
					{
						Rd = Alloc_DstReg( i.Rd );
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							
							e->CmpReg32ImmX( Rs, lConst );
							
							if ( Reg1 == i.Rs )
							{
								e->Set_B( Rd );
							}
							else
							{
								e->Set_A( Rd );
							}
							
							e->AndReg32ImmX( Rd, 1 );
						}
						else
						{
							e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
							e->CmpReg32ImmX ( Rd, lConst );
							
							if ( Reg1 == i.Rs )
							{
								e->Set_B( Rd );
							}
							else
							{
								e->Set_A( Rd );
							}
							
							e->AndReg32ImmX( Rd, 1 );
						}
					}
				}
				else
				{
					// all registers, no constants //
					
					if ( ( i.Rd == i.Rs ) || ( i.Rd == i.Rt ) )
					{
						// 2 op, all registers //
						
						if ( i.Rd == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							
							e->CmpRegReg32( Rd, Rs );
							
							if ( Reg1 == i.Rt )
							{
								e->Set_B( Rd );
							}
							else
							{
								e->Set_A( Rd );
							}
							
							e->AndReg32ImmX( Rd, 1 );
						}
						else
						{
							// register is not alloc and is disposable //
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							
							e->CmpRegMem32( Rd, & r->GPR [ Reg1 ].s );
							
							if ( Reg1 == i.Rt )
							{
								e->Set_B( Rd );
							}
							else
							{
								e->Set_A( Rd );
							}
							
							e->AndReg32ImmX( Rd, 1 );
						}
						
					}
					else
					{
						// 3 op, all different registers //
						
						if ( isBothAlloc( i.Rs, i.Rt ) || ( ! isEitherDisposable( i.Rs, i.Rt ) ) )
						{
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_SrcReg( i.Rt );
							
							if ( isEitherDisposable( i.Rs, i.Rt ) )
							{
								Reg2 = SelectDisposable( i.Rs, i.Rt );
								
								if ( Reg2 == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
								
								e->CmpRegReg32( Rs, Rt );
								
								Rd = RenameReg( i.Rd, Reg2 );
								
								e->Set_B ( Rd );
								e->AndReg32ImmX( Rd, 1 );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								
								e->XorRegReg32( Rd, Rd );
								e->CmpRegReg32( Rs, Rt );
								e->Set_B ( Rd );
							}
						}
						else if ( isEitherAlloc( i.Rs, i.Rt ) )
						{
							Reg1 = SelectAlloc ( i.Rs, i.Rt );
							Reg2 = SelectNotAlloc( i.Rs, i.Rt );
							
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( isDisposable ( Reg1 ) )
							{
								Rd = RenameReg( i.Rd, Reg1 );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								//e->MovRegReg32 ( Rd, Rs );
							}
							
							e->CmpRegMem32( Rs, & r->GPR [ Reg2 ].s );
							
							if ( Reg1 == i.Rs )
							{
								e->Set_B( Rd );
							}
							else
							{
								e->Set_A( Rd );
							}
							
							e->AndReg32ImmX( Rd, 1 );
						}
						else
						{
							Rd = Alloc_DstReg( i.Rd );
							e->MovRegMem32( Rd, & r->GPR [ i.Rs ].s );
							e->CmpRegMem32( Rd, & r->GPR [ i.Rt ].s );
							e->Set_B( Rd );
							e->AndReg32ImmX( Rd, 1 );
						}
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}


////////////////////////////////////////////
// I-Type Instructions (non-interrupt)



long Recompiler::ADDIU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "ADDIU";
	static const void* c_vFunction = (const void*) Instruction::Execute::ADDIU;
	
	//r->GPR [ i.Rt ].s = r->GPR [ i.Rs ].s + i.sImmediate;
	//CHECK_DELAYSLOT ( i.Rt );
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs
			ullSrcRegBitmap |= ( 1ull << i.Rs );
			
			// destination register is Rt
			ullDstRegBitmap |= ( 1ull << i.Rt );
			break;
			
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				if ( !i.Rs )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rt ].s, i.sImmediate );
				}
				else if ( i.Rt == i.Rs )
				{
					if ( i.sImmediate )
					{
						ret = e->AddMem32ImmX ( &r->GPR [ i.Rt ].s, i.sImmediate );
					}
				}
				else if ( !i.sImmediate )
				{
					if ( i.Rt != i.Rs )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
						ret = e->MovMemReg32 ( &r->GPR [ i.Rt ].s, RAX );
					}
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					//e->AddRegImm32 ( RAX, i.sImmediate );
					e->AddReg32ImmX ( RAX, i.sImmediate );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rt );
			}
			Local_LastModifiedReg = i.Rt;
			break;
			
#ifdef USE_NEW_ADDIU_CODE2
		case 2:
			if ( i.Rt )
			{
				if ( isConst( i.Rs ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rt, GetConst( i.Rs ) + ( (s32) i.sImmediate ) );
				}
				else if ( ( i.Rt == i.Rs ) )
				{
					// only one source reg and it could be same as destination //
					
					if ( i.sImmediate )
					{
						Rt = Alloc_SrcReg( i.Rt );
						Rt = Alloc_DstReg( i.Rt );
					
						e->AddReg32ImmX( Rt, i.sImmediate );
					}
				}
				else
				{
					// all registers are different //
					
					if ( isAlloc( i.Rs ) || ( ! isDisposable( i.Rs ) ) )
					{
						if ( isDisposable( i.Rs ) )
						{
							// rs is disposable //
							
							Rt = RenameReg( i.Rt, i.Rs );
							e->AddReg32ImmX( Rt, i.sImmediate );
						}
						else
						{
							// rs is NOT disposable //
							
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_DstReg( i.Rt );
							
							if( i.sImmediate )
							{
								e->LeaRegRegImm32( Rt, Rs, i.sImmediate );
							}
							else
							{
								e->MovRegReg32( Rt, Rs );
							}
						}
					}
					else
					{
						// Rs is not allocated and is disposable //
						Rt = Alloc_DstReg( i.Rt );
						e->MovRegMem32( Rt, & r->GPR [ i.Rs ].s );
						e->AddReg32ImmX( Rt, i.sImmediate );
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nError encoding ADDIU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::ANDI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "ANDI";
	static const void* c_vFunction = (const void*) Instruction::Execute::ANDI;
	
	//r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u & i.uImmediate;
	//CHECK_DELAYSLOT ( i.Rt );
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs
			ullSrcRegBitmap |= ( 1ull << i.Rs );
			
			// destination register is Rt
			ullDstRegBitmap |= ( 1ull << i.Rt );
			break;
			
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				if ( ( !i.Rs ) || ( !i.uImmediate ) )
				{
					e->MovMemImm32 ( &r->GPR [ i.Rt ].s, 0 );
				}
				else if ( i.Rt == i.Rs )
				{
					e->AndMem32ImmX ( &r->GPR [ i.Rt ].s, i.uImmediate );
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					e->AndReg32ImmX ( RAX, i.uImmediate );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
				}
				
			}
			Local_LastModifiedReg = i.Rt;
			break;

#ifdef USE_NEW_ANDI_CODE2
		case 2:
			if ( i.Rt )
			{
				if ( isConst( i.Rs ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rt, GetConst( i.Rs ) & i.uImmediate );
				}
				else if ( !i.uImmediate )
				{
					Alloc_Const( i.Rt, 0 );
				}
				else if ( ( i.Rt == i.Rs ) )
				{
					// only one source reg and it could be same as destination //
					
					Rt = Alloc_SrcReg( i.Rt );
					Rt = Alloc_DstReg( i.Rt );
					e->AndReg32ImmX( Rt, i.uImmediate );
				}
				else
				{
					// all registers are different //
					
					if ( isAlloc( i.Rs ) || ( ! isDisposable( i.Rs ) ) )
					{
						if ( isDisposable( i.Rs ) )
						{
							// rs is disposable //
							
							Rt = RenameReg( i.Rt, i.Rs );
							e->AndReg32ImmX( Rt, i.uImmediate );
						}
						else
						{
							// rs is NOT disposable //
							
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_DstReg( i.Rt );
							
							e->MovRegReg32( Rt, Rs );
							e->AndReg32ImmX( Rt, i.uImmediate );
						}
					}
					else
					{
						// Rs is not allocated and is disposable //
						Rt = Alloc_DstReg( i.Rt );
						e->MovRegMem32( Rt, & r->GPR [ i.Rs ].s );
						e->AndReg32ImmX( Rt, i.uImmediate );
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nError encoding ADDIU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::ORI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "ORI";
	static const void* c_vFunction = (const void*) Instruction::Execute::ORI;
	
	//r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u | i.uImmediate;
	//CHECK_DELAYSLOT ( i.Rt );
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs
			ullSrcRegBitmap |= ( 1ull << i.Rs );
			
			// destination register is Rt
			ullDstRegBitmap |= ( 1ull << i.Rt );
			break;
			
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				if ( !i.Rs )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rt ].s, i.uImmediate );
				}
				else if ( i.Rt == i.Rs )
				{
					if ( i.uImmediate )
					{
						ret = e->OrMem32ImmX ( &r->GPR [ i.Rt ].s, i.uImmediate );
					}
				}
				else if ( !i.uImmediate )
				{
					if ( i.Rt != i.Rs )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
						ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
					}
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					//e->OrRegImm32 ( RAX, i.uImmediate );
					e->OrReg32ImmX ( RAX, i.uImmediate );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rt );
			}
			Local_LastModifiedReg = i.Rt;
			break;

#ifdef USE_NEW_ORI_CODE2
		case 2:
			if ( i.Rt )
			{
				if ( isConst( i.Rs ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rt, GetConst( i.Rs ) | i.uImmediate );
				}
				else if ( ( i.Rt == i.Rs ) )
				{
					// only one source reg and it could be same as destination //
					
					// only need to do anything if immediate is not zero
					if ( i.sImmediate )
					{
						Rt = Alloc_SrcReg( i.Rt );
						Rt = Alloc_DstReg( i.Rt );
						e->OrReg32ImmX( Rt, i.uImmediate );
					}
				}
				else
				{
					// all registers are different //
					
					if ( isAlloc( i.Rs ) || ( ! isDisposable( i.Rs ) ) )
					{
						if ( isDisposable( i.Rs ) )
						{
							// rs is disposable //
							
							Rt = RenameReg( i.Rt, i.Rs );
							e->OrReg32ImmX( Rt, i.uImmediate );
						}
						else
						{
							// rs is NOT disposable //
							
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_DstReg( i.Rt );
							
							e->MovRegReg32 ( Rt, Rs );
							e->OrReg32ImmX( Rt, i.uImmediate );
						}
					}
					else
					{
						// Rs is not allocated and is disposable //
						Rt = Alloc_DstReg( i.Rt );
						e->MovRegMem32( Rt, & r->GPR [ i.Rs ].s );
						e->OrReg32ImmX( Rt, i.uImmediate );
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nError encoding ADDIU instruction.\n";
		return -1;
	}
	
	return 1;
}

long Recompiler::XORI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "XORI";
	static const void* c_vFunction = (const void*) Instruction::Execute::XORI;
	
	//r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u ^ i.uImmediate;
	//CHECK_DELAYSLOT ( i.Rt );
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs
			ullSrcRegBitmap |= ( 1ull << i.Rs );
			
			// destination register is Rt
			ullDstRegBitmap |= ( 1ull << i.Rt );
			break;
			
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				if ( !i.Rs )
				{
					e->MovMemImm32 ( &r->GPR [ i.Rt ].s, i.uImmediate );
				}
				else if ( i.Rt == i.Rs )
				{
					if ( i.uImmediate )
					{
						e->XorMem32ImmX ( &r->GPR [ i.Rt ].s, i.uImmediate );
					}
				}
				else if ( !i.uImmediate )
				{
					if ( i.Rt != i.Rs )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
						ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
					}
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
					//e->XorRegImm32 ( RAX, i.uImmediate );
					e->XorReg32ImmX ( RAX, i.uImmediate );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
				}
				
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rt );
			}
			Local_LastModifiedReg = i.Rt;
			break;
			
#ifdef USE_NEW_XORI_CODE2
		case 2:
			if ( i.Rt )
			{
				if ( isConst( i.Rs ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rt, GetConst( i.Rs ) ^ i.uImmediate );
				}
				else if ( ( i.Rt == i.Rs ) )
				{
					// only one source reg and it could be same as destination //
					
					// only need to do anything if immediate is not zero
					if ( i.uImmediate )
					{
						Rt = Alloc_SrcReg( i.Rt );
						Rt = Alloc_DstReg( i.Rt );
						e->XorReg32ImmX( Rt, i.uImmediate );
					}
				}
				else
				{
					// all registers are different //
					
					if ( isAlloc( i.Rs ) || ( ! isDisposable( i.Rs ) ) )
					{
						if ( isDisposable( i.Rs ) )
						{
							// rs is disposable //
							
							Rt = RenameReg( i.Rt, i.Rs );
							e->XorReg32ImmX( Rt, i.uImmediate );
						}
						else
						{
							// rs is NOT disposable //
							
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_DstReg( i.Rt );
							
							e->MovRegReg32 ( Rt, Rs );
							e->XorReg32ImmX( Rt, i.uImmediate );
						}
					}
					else
					{
						// Rs is not allocated and is disposable //
						Rt = Alloc_DstReg( i.Rt );
						e->MovRegMem32( Rt, & r->GPR [ i.Rs ].s );
						e->XorReg32ImmX( Rt, i.uImmediate );
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nError encoding ADDIU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLTI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SLTI";
	static const void* c_vFunction = (const void*) Instruction::Execute::SLTI;
	
	//r->GPR [ i.Rt ].s = r->GPR [ i.Rs ].s < i.sImmediate ? 1 : 0;
	//CHECK_DELAYSLOT ( i.Rt );
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs
			ullSrcRegBitmap |= ( 1ull << i.Rs );
			
			// destination register is Rt
			ullDstRegBitmap |= ( 1ull << i.Rt );
			break;
			
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				if ( !i.sImmediate )
				{
					// ***todo*** implement ShrMemImm32
					//if ( i.Rt == i.Rs )
					//{
					//	e->ShrMemImm32 ( &r->GPR [ i.Rs ].s, 31 );
					//}
					//else
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
						//e->Cdq ();
						//e->NegReg32 ( RDX );
						e->ShrRegImm32 ( RAX, 31 );
						//ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RDX );
						ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
					}
				}
				else
				{
					e->XorRegReg32 ( RAX, RAX );
					//e->CmpMemImm32 ( &r->GPR [ i.Rs ].s, i.sImmediate );
					e->CmpMem32ImmX ( &r->GPR [ i.Rs ].s, i.sImmediate );
					e->Set_L ( RAX );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
				}
			}
			Local_LastModifiedReg = i.Rt;
			break;


#ifdef USE_NEW_SLTI_CODE2
		case 2:
			if ( i.Rt )
			{
				if ( isConst( i.Rs ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rt, ( ( (s32) GetConst( i.Rs ) ) < ( (s32) i.sImmediate ) ) ? 1 : 0 );
				}
				else if ( ( i.Rt == i.Rs ) )
				{
					// only one source reg and it could be same as destination //
					
					Rt = Alloc_SrcReg( i.Rt );
					Rt = Alloc_DstReg( i.Rt );
				
					e->CmpReg32ImmX( Rt, i.sImmediate );
					e->Set_L( Rt );
					e->AndReg32ImmX ( Rt, 1 );
				}
				else
				{
					// all registers are different //
					
					if ( isAlloc( i.Rs ) || ( ! isDisposable( i.Rs ) ) )
					{
						if ( isDisposable( i.Rs ) )
						{
							// rs is disposable //
							
							Rt = RenameReg( i.Rt, i.Rs );
							
							e->CmpReg32ImmX( Rt, i.sImmediate );
							e->Set_L( Rt );
							e->AndReg32ImmX ( Rt, 1 );
						}
						else
						{
							// rs is NOT disposable //
							
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_DstReg( i.Rt );
							
							e->XorRegReg32( Rt, Rt );
							e->CmpReg32ImmX( Rs, i.sImmediate );
							e->Set_L( Rt );
							//e->AndReg32ImmX ( Rt, 1 );
						}
					}
					else
					{
						// Rs is not allocated and is disposable //
						Rt = Alloc_DstReg( i.Rt );
						
						e->XorRegReg32 ( Rt, Rt );
						e->CmpMem32ImmX( & r->GPR [ i.Rs ].s, i.sImmediate );
						e->Set_L( Rt );
						//e->AndReg32ImmX ( Rt, 1 );
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLTIU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SLTIU";
	static const void* c_vFunction = (const void*) Instruction::Execute::SLTIU;
	
	//r->GPR [ i.Rt ].u = r->GPR [ i.Rs ].u < ((u32) ((s32) i.sImmediate)) ? 1 : 0;
	//CHECK_DELAYSLOT ( i.Rt );
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs
			ullSrcRegBitmap |= ( 1ull << i.Rs );
			
			// destination register is Rt
			ullDstRegBitmap |= ( 1ull << i.Rt );
			break;
			
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				if ( !i.sImmediate )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rt ].s, 0 );
				}
				else
				{
					e->XorRegReg32 ( RAX, RAX );
					//e->CmpMemImm32 ( &r->GPR [ i.Rs ].s, i.sImmediate );
					e->CmpMem32ImmX ( &r->GPR [ i.Rs ].s, i.sImmediate );
					//e->Set_B ( RAX );
					e->AdcRegReg32 ( RAX, RAX );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
				}
			}
			Local_LastModifiedReg = i.Rt;
			break;
			
#ifdef USE_NEW_SLTIU_CODE2
		case 2:
			if ( i.Rt )
			{
				if ( isConst( i.Rs ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rt, ( ( (u32) GetConst( i.Rs ) ) < ( (u32) ( (s32) i.sImmediate ) ) ) ? 1 : 0 );
				}
				else if ( !i.sImmediate )
				{
					Alloc_Const ( i.Rt, 0 );
				}
				else if ( ( i.Rt == i.Rs ) )
				{
					// only one source reg and it could be same as destination //
					
					Rt = Alloc_SrcReg( i.Rt );
					Rt = Alloc_DstReg( i.Rt );
				
					e->CmpReg32ImmX( Rt, i.sImmediate );
					e->Set_B( Rt );
					e->AndReg32ImmX ( Rt, 1 );
				}
				else
				{
					// all registers are different //
					
					if ( isAlloc( i.Rs ) || ( ! isDisposable( i.Rs ) ) )
					{
						if ( isDisposable( i.Rs ) )
						{
							// rs is disposable //
							
							Rt = RenameReg( i.Rt, i.Rs );
							
							e->CmpReg32ImmX( Rt, i.sImmediate );
							e->Set_B( Rt );
							e->AndReg32ImmX ( Rt, 1 );
						}
						else
						{
							// rs is NOT disposable //
							
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_DstReg( i.Rt );
							
							e->XorRegReg32 ( Rt, Rt );
							e->CmpReg32ImmX( Rs, i.sImmediate );
							e->Set_B( Rt );
							//e->AndReg32ImmX ( Rt, 1 );
						}
					}
					else
					{
						// Rs is not allocated and is disposable //
						Rt = Alloc_DstReg( i.Rt );
						
						e->XorRegReg32 ( Rt, Rt );
						e->CmpMem32ImmX( & r->GPR [ i.Rs ].s, i.sImmediate );
						e->Set_B( Rt );
						//e->AndReg32ImmX ( Rt, 1 );
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LUI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LUI";
	static const void* c_vFunction = (const void*) Instruction::Execute::LUI;
	
	//r->GPR [ i.Rt ].u = ( i.uImmediate << 16 );
	//CHECK_DELAYSLOT ( i.Rt );
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// there is no source register
			
			// destination register is Rt
			ullDstRegBitmap |= ( 1ull << i.Rt );
			
			// destination register is known to be constant
			//ullConstRegBitmap |= ( 1 << i.Rt );
			
			break;
			
		case 0:
			if ( i.Rt )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rt )
			{
				ret = e->MovMemImm32 ( &r->GPR [ i.Rt ].s, ( i.uImmediate << 16 ) );
				//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rt );
			}
			Local_LastModifiedReg = i.Rt;
			break;
			
#ifdef USE_NEW_LUI_CODE2
		case 2:
			if ( i.Rt )
			{
				Alloc_Const( i.Rt, ( i.uImmediate << 16 ) );
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LUI instruction.\n";
		return -1;
	}
	return 1;
}







//////////////////////////////////////////////////////////
// Shift instructions



long Recompiler::SLL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SLL";
	static const void* c_vFunction = (const void*) Instruction::Execute::SLL;
	
	//r->GPR [ i.Rd ].u = ( r->GPR [ i.Rt ].u << i.Shift );
	//CHECK_DELAYSLOT ( i.Rd );
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rt
			ullSrcRegBitmap |= ( 1 << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( !i.Shift )
				{
					if ( i.Rd != i.Rt )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
						ret = e->MovMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					//e->MovRegImm32 ( RCX, (u32) i.Shift );
					e->ShlRegImm32 ( RAX, (u32) i.Shift );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
				}
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
#ifdef USE_NEW_SLL_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( isConst( i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, GetConst( i.Rt ) << i.Shift );
				}
				else if ( ( i.Rd == i.Rt ) )
				{
					// only one source reg and it could be same as destination //
					
					// only need to do anything if immediate is not zero
					if ( i.Shift )
					{
						Rd = Alloc_SrcReg( i.Rd );
						Rd = Alloc_DstReg( i.Rd );
						e->ShlRegImm32( Rd, i.Shift );
					}
				}
				else
				{
					// all registers are different //
					
					if ( isAlloc( i.Rt ) || ( ! isDisposable( i.Rt ) ) )
					{
						if ( isDisposable( i.Rt ) )
						{
							// rs is disposable //
							
							Rd = RenameReg( i.Rd, i.Rt );
							e->ShlRegImm32( Rd, i.Shift );
						}
						else
						{
							// rs is NOT disposable //
							
							Rt = Alloc_SrcReg( i.Rt );
							Rd = Alloc_DstReg( i.Rd );
							
							e->MovRegReg32 ( Rd, Rt );
							e->ShlRegImm32( Rd, i.Shift );
						}
					}
					else
					{
						// Rs is not allocated and is disposable //
						Rd = Alloc_DstReg( i.Rd );
						e->MovRegMem32( Rd, & r->GPR [ i.Rt ].s );
						e->ShlRegImm32( Rd, i.Shift );
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SLL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SRL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SRL";
	static const void* c_vFunction = (const void*) Instruction::Execute::SRL;
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rt
			ullSrcRegBitmap |= ( 1 << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( !i.Shift )
				{
					if ( i.Rd != i.Rt )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
						ret = e->MovMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					//e->MovRegImm32 ( RCX, (u32) i.Shift );
					e->ShrRegImm32 ( RAX, (u32) i.Shift );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
				}
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
#ifdef USE_NEW_SRL_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( isConst( i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, ( (u32) GetConst( i.Rt ) ) >> i.Shift );
				}
				else if ( ( i.Rd == i.Rt ) )
				{
					// only one source reg and it could be same as destination //
					
					// only need to do anything if immediate is not zero
					if ( i.Shift )
					{
						Rd = Alloc_SrcReg( i.Rd );
						Rd = Alloc_DstReg( i.Rd );
						e->ShrRegImm32( Rd, i.Shift );
					}
				}
				else
				{
					// all registers are different //
					
					if ( isAlloc( i.Rt ) || ( ! isDisposable( i.Rt ) ) )
					{
						if ( isDisposable( i.Rt ) )
						{
							// rs is disposable //
							
							Rd = RenameReg( i.Rd, i.Rt );
							e->ShrRegImm32( Rd, i.Shift );
						}
						else
						{
							// rs is NOT disposable //
							
							Rt = Alloc_SrcReg( i.Rt );
							Rd = Alloc_DstReg( i.Rd );
							
							e->MovRegReg32 ( Rd, Rt );
							e->ShrRegImm32( Rd, i.Shift );
						}
					}
					else
					{
						// Rs is not allocated and is disposable //
						Rd = Alloc_DstReg( i.Rd );
						e->MovRegMem32( Rd, & r->GPR [ i.Rt ].s );
						e->ShrRegImm32( Rd, i.Shift );
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SRA ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SRA";
	static const void* c_vFunction = (const void*) Instruction::Execute::SRA;
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rt
			ullSrcRegBitmap |= ( 1 << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( !i.Shift )
				{
					if ( i.Rd != i.Rt )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
						ret = e->MovMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else
				{
					e->MovRegFromMem32 ( 0, &r->GPR [ i.Rt ].s );
					//e->MovRegImm32 ( RCX, (u32) i.Shift );
					e->SarRegImm32 ( 0, (u32) i.Shift );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, 0 );
					//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
				}
			}
			Local_LastModifiedReg = i.Rd;
			break;
			
#ifdef USE_NEW_SRA_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( isConst( i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, ( (s32) GetConst( i.Rt ) ) >> i.Shift );
				}
				else if ( ( i.Rd == i.Rt ) )
				{
					// only one source reg and it could be same as destination //
					
					// only need to do anything if immediate is not zero
					if ( i.Shift )
					{
						Rd = Alloc_SrcReg( i.Rd );
						Rd = Alloc_DstReg( i.Rd );
						e->SarRegImm32( Rd, i.Shift );
					}
				}
				else
				{
					// all registers are different //
					
					if ( isAlloc( i.Rt ) || ( ! isDisposable( i.Rt ) ) )
					{
						if ( isDisposable( i.Rt ) )
						{
							// rs is disposable //
							
							Rd = RenameReg( i.Rd, i.Rt );
							e->SarRegImm32( Rd, i.Shift );
						}
						else
						{
							// rs is NOT disposable //
							
							Rt = Alloc_SrcReg( i.Rt );
							Rd = Alloc_DstReg( i.Rd );
							
							e->MovRegReg32 ( Rd, Rt );
							e->SarRegImm32( Rd, i.Shift );
						}
					}
					else
					{
						// Rs is not allocated and is disposable //
						Rd = Alloc_DstReg( i.Rd );
						e->MovRegMem32( Rd, & r->GPR [ i.Rt ].s );
						e->SarRegImm32( Rd, i.Shift );
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SLLV ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SLLV";
	static const void* c_vFunction = (const void*) Instruction::Execute::SLLV;
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	//r->GPR [ i.Rd ].u = ( r->GPR [ i.Rt ].u << ( r->GPR [ i.Rs ].u & 0x1f ) );
	//CHECK_DELAYSLOT ( i.Rd );
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( !i.Rs )
				{
					if ( i.Rd != i.Rt )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
						ret = e->MovMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->MovRegFromMem32 ( RCX, &r->GPR [ i.Rs ].s );
					e->ShlRegReg32 ( RAX );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
				}
			}
			break;
			
#ifdef USE_NEW_SLLV_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( isBothConst( i.Rs, i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, GetConst( i.Rt ) << ( GetConst( i.Rs ) & 0x1f ) );
				}
				else if ( isEitherConst ( i.Rs, i.Rt ) )
				{
					// get const in Reg2
					Reg2 = SelectConst( i.Rs, i.Rt );
					
					// get the other register in Reg1
					if ( Reg2 == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
					// get the constnnt
					lConst = GetConst( Reg2 );
					
					if ( ( Reg1 == i.Rd ) || ( isAlloc( Reg1 ) && isDisposable( Reg1 ) ) )
					{
						if ( Reg1 == i.Rd )
						{
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
						}
						else
						{
							Rd = RenameReg( i.Rd, Reg1 );
						}
						
						if ( Reg2 == i.Rs )
						{
							//e->MovReg32ImmX( RCX, lConst & 0x1f );
							e->ShlRegImm32( Rd, lConst & 0x1f );
						}
						else
						{
							e->MovRegReg32( RCX, Rd );
							e->MovReg32ImmX( Rd, lConst );
							e->ShlRegReg32( Rd );
						}
						
					}
					else
					{
						Rd = Alloc_DstReg( i.Rd );
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( Reg1 == i.Rs )
							{
								e->MovRegReg32( RCX, Rs );
								e->MovReg32ImmX( Rd, lConst );
								e->ShlRegReg32( Rd );
							}
							else
							{
								//e->MovReg32ImmX( RCX, lConst );
								e->MovRegReg32( Rd, Rs );
								e->ShlRegImm32( Rd, lConst & 0x1f );
							}
							
						}
						else
						{
							if ( Reg1 == i.Rs )
							{
								e->MovRegMem32( RCX, & r->GPR [ Reg1 ].s );
								e->MovReg32ImmX( Rd, lConst );
								e->ShlRegReg32( Rd );
							}
							else
							{
								//e->MovReg32ImmX( RCX, lConst );
								e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
								e->ShlRegImm32( Rd, lConst & 0x1f );
							}
							
						}
					}
				}
				else
				{
					// all registers, no constants //
					
					if ( ( i.Rd == i.Rs ) || ( i.Rd == i.Rt ) )
					{
						// 2 op, all registers //
						
						if ( i.Rd == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							
							if ( Reg1 == i.Rs )
							{
								e->MovRegReg32( RCX, Rs );
							}
							else
							{
								e->MovRegReg32( RCX, Rd );
								e->MovRegReg32( Rd, Rs );
							}
							
							e->ShlRegReg32( Rd );
						}
						else
						{
							// register is not alloc and is disposable //
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							
							if ( Reg1 == i.Rs )
							{
								e->MovRegMem32( RCX, & r->GPR [ Reg1 ].s );
							}
							else
							{
								e->MovRegReg32( RCX, Rd );
								e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
							}
							
							e->ShlRegReg32( Rd );
						}
					}
					else
					{
						// 3 op, all different registers //
						
						if ( isBothAlloc( i.Rs, i.Rt ) || ( ! isEitherDisposable( i.Rs, i.Rt ) ) )
						{
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_SrcReg( i.Rt );
							
							if ( isEitherDisposable( i.Rs, i.Rt ) )
							{
								Reg1 = SelectDisposable( i.Rs, i.Rt );
								
								if ( Reg1 == i.Rs ) Reg2 = i.Rt; else Reg2 = i.Rs;
								
								Rs = Alloc_SrcReg( Reg2 );
								
								Rd = RenameReg( i.Rd, Reg1 );
								
								if ( Reg2 == i.Rs )
								{
									e->MovRegReg32( RCX, Rs );
								}
								else
								{
									e->MovRegReg32( RCX, Rd );
									e->MovRegReg32( Rd, Rs );
								}
								
								e->ShlRegReg32( Rd );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->MovRegReg32( RCX, Rs );
								e->MovRegReg32( Rd, Rt );
								e->ShlRegReg32( Rd );
							}
						}
						else if ( isEitherAlloc( i.Rs, i.Rt ) )
						{
							Reg1 = SelectAlloc ( i.Rs, i.Rt );
							Reg2 = SelectNotAlloc( i.Rs, i.Rt );
							
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( isDisposable ( Reg1 ) )
							{
								Rd = RenameReg( i.Rd, Reg1 );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								//e->MovRegReg32 ( Rd, Rs );
							}
							
							if ( Reg1 == i.Rs )
							{
								e->MovRegReg32( RCX, Rs );
								e->MovRegMem32( Rd, & r->GPR [ Reg2 ].s );
							}
							else
							{
								e->MovRegMem32( RCX, & r->GPR [ Reg2 ].s );
								e->MovRegReg32( Rd, Rs );
							}
							
							e->ShlRegReg32( Rd );
						}
						else
						{
							Rd = Alloc_DstReg( i.Rd );
							e->MovRegMem32( RCX, & r->GPR [ i.Rs ].s );
							e->MovRegMem32( Rd, & r->GPR [ i.Rt ].s );
							e->ShlRegReg32( Rd );
						}
					}
				}
			}
			
			break;
#endif

		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SRLV ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SRLV";
	static const void* c_vFunction = (const void*) Instruction::Execute::SRLV;
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( !i.Rs )
				{
					if ( i.Rd != i.Rt )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
						ret = e->MovMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->MovRegFromMem32 ( RCX, &r->GPR [ i.Rs ].s );
					e->ShrRegReg32 ( RAX );
					ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
				}
			}
			Local_LastModifiedReg = i.Rd;
			break;

#ifdef USE_NEW_SRLV_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( isBothConst( i.Rs, i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, ( (u32) GetConst( i.Rt ) ) >> ( GetConst( i.Rs ) & 0x1f ) );
				}
				else if ( isEitherConst ( i.Rs, i.Rt ) )
				{
					// get const in Reg2
					Reg2 = SelectConst( i.Rs, i.Rt );
					
					// get the other register in Reg1
					if ( Reg2 == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
					// get the constnnt
					lConst = GetConst( Reg2 );
					
					if ( ( Reg1 == i.Rd ) || ( isAlloc( Reg1 ) && isDisposable( Reg1 ) ) )
					{
						if ( Reg1 == i.Rd )
						{
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
						}
						else
						{
							Rd = RenameReg( i.Rd, Reg1 );
						}
						
						if ( Reg2 == i.Rs )
						{
							e->ShrRegImm32( Rd, lConst & 0x1f );
						}
						else
						{
							e->MovRegReg32( RCX, Rd );
							e->MovReg32ImmX( Rd, lConst );
							e->ShrRegReg32( Rd );
						}
						
					}
					else
					{
						Rd = Alloc_DstReg( i.Rd );
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( Reg1 == i.Rs )
							{
								e->MovRegReg32( RCX, Rs );
								e->MovReg32ImmX( Rd, lConst );
								e->ShrRegReg32( Rd );
							}
							else
							{
								e->MovRegReg32( Rd, Rs );
								e->ShrRegImm32( Rd, lConst & 0x1f );
							}
							
						}
						else
						{
							if ( Reg1 == i.Rs )
							{
								e->MovRegMem32( RCX, & r->GPR [ Reg1 ].s );
								e->MovReg32ImmX( Rd, lConst );
								e->ShrRegReg32( Rd );
							}
							else
							{
								e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
								e->ShrRegImm32( Rd, lConst & 0x1f );
							}
							
						}
					}
				}
				else
				{
					// all registers, no constants //
					
					if ( ( i.Rd == i.Rs ) || ( i.Rd == i.Rt ) )
					{
						// 2 op, all registers //
						
						if ( i.Rd == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							
							if ( Reg1 == i.Rs )
							{
								e->MovRegReg32( RCX, Rs );
							}
							else
							{
								e->MovRegReg32( RCX, Rd );
								e->MovRegReg32( Rd, Rs );
							}
							
							e->ShrRegReg32( Rd );
						}
						else
						{
							// register is not alloc and is disposable //
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							
							if ( Reg1 == i.Rs )
							{
								e->MovRegMem32( RCX, & r->GPR [ Reg1 ].s );
							}
							else
							{
								e->MovRegReg32( RCX, Rd );
								e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
							}
							
							e->ShrRegReg32( Rd );
						}
					}
					else
					{
						// 3 op, all different registers //
						
						if ( isBothAlloc( i.Rs, i.Rt ) || ( ! isEitherDisposable( i.Rs, i.Rt ) ) )
						{
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_SrcReg( i.Rt );
							
							if ( isEitherDisposable( i.Rs, i.Rt ) )
							{
								Reg1 = SelectDisposable( i.Rs, i.Rt );
								
								if ( Reg1 == i.Rs ) Reg2 = i.Rt; else Reg2 = i.Rs;
								
								Rs = Alloc_SrcReg( Reg2 );
								
								Rd = RenameReg( i.Rd, Reg1 );
								
								if ( Reg2 == i.Rs )
								{
									e->MovRegReg32( RCX, Rs );
								}
								else
								{
									e->MovRegReg32( RCX, Rd );
									e->MovRegReg32( Rd, Rs );
								}
								
								e->ShrRegReg32( Rd );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->MovRegReg32( RCX, Rs );
								e->MovRegReg32( Rd, Rt );
								e->ShrRegReg32( Rd );
							}
						}
						else if ( isEitherAlloc( i.Rs, i.Rt ) )
						{
							Reg1 = SelectAlloc ( i.Rs, i.Rt );
							Reg2 = SelectNotAlloc( i.Rs, i.Rt );
							
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( isDisposable ( Reg1 ) )
							{
								Rd = RenameReg( i.Rd, Reg1 );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								//e->MovRegReg32 ( Rd, Rs );
							}
							
							if ( Reg1 == i.Rs )
							{
								e->MovRegReg32( RCX, Rs );
								e->MovRegMem32( Rd, & r->GPR [ Reg2 ].s );
							}
							else
							{
								e->MovRegMem32( RCX, & r->GPR [ Reg2 ].s );
								e->MovRegReg32( Rd, Rs );
							}
							
							e->ShrRegReg32( Rd );
						}
						else
						{
							Rd = Alloc_DstReg( i.Rd );
							e->MovRegMem32( RCX, & r->GPR [ i.Rs ].s );
							e->MovRegMem32( Rd, & r->GPR [ i.Rt ].s );
							e->ShrRegReg32( Rd );
						}
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SRAV ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SRAV";
	static const void* c_vFunction = (const void*) Instruction::Execute::SRAV;
	
	int Rd, Rs, Rt;
	int Reg1, Reg2;
	s32 lConst;
	
	int ret = 1;
	
	
	// *testing*
	//bStopEncodingBefore = true;
	//bStopEncodingAfter = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			// destination register is Rd
			ullDstRegBitmap |= ( 1ull << i.Rd );
			break;
			
		case 0:
			if ( i.Rd )
			{
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

				// load arguments
				e->LoadImm32 ( RCX, i.Value );
				ret = e->Call ( (void*) c_vFunction );
				
#ifdef RESERVE_STACK_FRAME_FOR_CALL
				ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			}
			break;
			
		case 1:
			if ( i.Rd )
			{
				if ( !i.Rt )
				{
					ret = e->MovMemImm32 ( &r->GPR [ i.Rd ].s, 0 );
				}
				else if ( !i.Rs )
				{
					if ( i.Rd != i.Rt )
					{
						e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
						ret = e->MovMemReg32 ( &r->GPR [ i.Rd ].s, RAX );
					}
				}
				else
				{
					e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rt ].s );
					e->MovRegFromMem32 ( RCX, &r->GPR [ i.Rs ].s );
					e->SarRegReg32 ( RAX );
					e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
					//ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
				}
			}
			Local_LastModifiedReg = i.Rd;
			break;

#ifdef USE_NEW_SRAV_CODE2
		case 2:
			if ( i.Rd )
			{
				if ( isBothConst( i.Rs, i.Rt ) )
				{
					// both registers are constant //
					Alloc_Const ( i.Rd, ( (s32) GetConst( i.Rt ) ) >> ( GetConst( i.Rs ) & 0x1f ) );
				}
				else if ( isEitherConst ( i.Rs, i.Rt ) )
				{
					// get const in Reg2
					Reg2 = SelectConst( i.Rs, i.Rt );
					
					// get the other register in Reg1
					if ( Reg2 == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
					// get the constnnt
					lConst = GetConst( Reg2 );
					
					if ( ( Reg1 == i.Rd ) || ( isAlloc( Reg1 ) && isDisposable( Reg1 ) ) )
					{
						if ( Reg1 == i.Rd )
						{
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
						}
						else
						{
							Rd = RenameReg( i.Rd, Reg1 );
						}
						
						if ( Reg2 == i.Rs )
						{
							//e->MovReg32ImmX( RCX, lConst & 0x1f );
							e->SarRegImm32( Rd, lConst & 0x1f );
						}
						else
						{
							e->MovRegReg32( RCX, Rd );
							e->MovReg32ImmX( Rd, lConst );
							e->SarRegReg32( Rd );
						}
						
					}
					else
					{
						Rd = Alloc_DstReg( i.Rd );
						
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( Reg1 == i.Rs )
							{
								e->MovRegReg32( RCX, Rs );
								e->MovReg32ImmX( Rd, lConst );
								e->SarRegReg32( Rd );
							}
							else
							{
								//e->MovReg32ImmX( RCX, lConst );
								e->MovRegReg32( Rd, Rs );
								e->SarRegImm32( Rd, lConst & 0x1f );
							}
							
						}
						else
						{
							if ( Reg1 == i.Rs )
							{
								e->MovRegMem32( RCX, & r->GPR [ Reg1 ].s );
								e->MovReg32ImmX( Rd, lConst );
							}
							else
							{
								e->MovReg32ImmX( RCX, lConst );
								e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
							}
							
							e->SarRegReg32( Rd );
						}
					}
				}
				else
				{
					// all registers, no constants //
					
					if ( ( i.Rd == i.Rs ) || ( i.Rd == i.Rt ) )
					{
						// 2 op, all registers //
						
						if ( i.Rd == i.Rs ) Reg1 = i.Rt; else Reg1 = i.Rs;
					
						if ( isAlloc( Reg1 ) || ( !isDisposable( Reg1 ) ) )
						{
							Rs = Alloc_SrcReg( Reg1 );
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							
							if ( Reg1 == i.Rs )
							{
								e->MovRegReg32( RCX, Rs );
							}
							else
							{
								e->MovRegReg32( RCX, Rd );
								e->MovRegReg32( Rd, Rs );
							}
							
							e->SarRegReg32( Rd );
						}
						else
						{
							// register is not alloc and is disposable //
							Rd = Alloc_SrcReg( i.Rd );
							Rd = Alloc_DstReg( i.Rd );
							
							if ( Reg1 == i.Rs )
							{
								e->MovRegMem32( RCX, & r->GPR [ Reg1 ].s );
							}
							else
							{
								e->MovRegReg32( RCX, Rd );
								e->MovRegMem32( Rd, & r->GPR [ Reg1 ].s );
							}
							
							e->SarRegReg32( Rd );
						}
					}
					else
					{
						// 3 op, all different registers //
						
						if ( isBothAlloc( i.Rs, i.Rt ) || ( ! isEitherDisposable( i.Rs, i.Rt ) ) )
						{
							Rs = Alloc_SrcReg( i.Rs );
							Rt = Alloc_SrcReg( i.Rt );
							
							if ( isEitherDisposable( i.Rs, i.Rt ) )
							{
								Reg1 = SelectDisposable( i.Rs, i.Rt );
								
								if ( Reg1 == i.Rs ) Reg2 = i.Rt; else Reg2 = i.Rs;
								
								Rs = Alloc_SrcReg( Reg2 );
								
								Rd = RenameReg( i.Rd, Reg1 );
								
								if ( Reg2 == i.Rs )
								{
									e->MovRegReg32( RCX, Rs );
								}
								else
								{
									e->MovRegReg32( RCX, Rd );
									e->MovRegReg32( Rd, Rs );
								}
								
								e->SarRegReg32( Rd );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								e->MovRegReg32( RCX, Rs );
								e->MovRegReg32( Rd, Rt );
								e->SarRegReg32( Rd );
							}
						}
						else if ( isEitherAlloc( i.Rs, i.Rt ) )
						{
							Reg1 = SelectAlloc ( i.Rs, i.Rt );
							Reg2 = SelectNotAlloc( i.Rs, i.Rt );
							
							Rs = Alloc_SrcReg( Reg1 );
							
							if ( isDisposable ( Reg1 ) )
							{
								Rd = RenameReg( i.Rd, Reg1 );
							}
							else
							{
								Rd = Alloc_DstReg( i.Rd );
								//e->MovRegReg32 ( Rd, Rs );
							}
							
							if ( Reg1 == i.Rs )
							{
								e->MovRegReg32( RCX, Rs );
								e->MovRegMem32( Rd, & r->GPR [ Reg2 ].s );
							}
							else
							{
								e->MovRegMem32( RCX, & r->GPR [ Reg2 ].s );
								e->MovRegReg32( Rd, Rs );
							}
							
							e->SarRegReg32( Rd );
						}
						else
						{
							Rd = Alloc_DstReg( i.Rd );
							e->MovRegMem32( RCX, & r->GPR [ i.Rs ].s );
							e->MovRegMem32( Rd, & r->GPR [ i.Rt ].s );
							e->SarRegReg32( Rd );
						}
					}
				}
			}
			
			break;
#endif
			
		default:
			return -1;
			break;
	} // end switch ( OpLevel )
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDU instruction.\n";
		return -1;
	}
	return 1;
}


//----------------------------------------------------------------------------


////////////////////////////////////////////
// Jump/Branch Instructions



long Recompiler::J ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "J";
	static const void* c_vFunction = (const void*) Instruction::Execute::J;
	
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.cb = r->_cb_Jump;
	//r->Status.DelaySlot_Valid |= 0x1;
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			//bStopEncodingBefore = true;

			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( (long*) & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// load arguments
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_J_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPJ> );
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //J instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::JR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "JR";
	static const void* c_vFunction = (const void*) Instruction::Execute::JR;
	
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.cb = r->_cb_JumpRegister;
	//r->DelaySlot0.Data = r->GPR [ i.Rs ].u & ~3;
	//r->Status.DelaySlot_Valid |= 0x1;
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// has to stop encoding before AND after here due to the posible synchronous interrupt
			// so it has to have PC,LastPC updated (or could set it and move up the entry point)
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_JR_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPJR> );
#else
			return -1;
#endif

			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//e->MovRegFromMem32 ( 0, &r->GPR [ i.Rs ].s );
			//e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //JR instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::JAL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "JAL";
	static const void* c_vFunction = (const void*) Instruction::Execute::JAL;
	
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.cb = r->_cb_Jump;
	//r->Status.DelaySlot_Valid |= 0x1;
	//r->GPR [ 31 ].u = r->PC + 8;
	//CHECK_DELAYSLOT ( 31 );
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// there is no synchronous interrupt possible here, so only needs to stop encoding after
			bStopEncodingAfter = true;
			
			// also have to stop before because need an updated NextPC so you get the correct link value
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			break;
			
		case 1:
#ifdef USE_NEW_JAL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPJAL> );
#else
			return -1;
#endif

			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->MovMemImm32 ( &r->GPR [ 31 ].u, Address + 8 );
			//ret = e->MovMemImm32 ( &r->LastModifiedRegister, 31 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //JAL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::JALR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "JALR";
	static const void* c_vFunction = (const void*) Instruction::Execute::JALR;
	
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = r->GPR [ i.Rs ].u & ~3;
	//r->DelaySlot0.cb = r->_cb_JumpRegister;
	//r->Status.DelaySlot_Valid |= 0x1;
	//r->GPR [ i.Rd ].u = r->PC + 8;
	//CHECK_DELAYSLOT ( i.Rd );
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// has to stop encoding before AND after here due to the posible synchronous interrupt
			// so it has to have PC,LastPC updated (or could set it and move up the entry point)
			bStopEncodingBefore = true;
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// make sure Rd is not r0
			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				e->MovMemImm32 ( &r->GPR [ 0 ].s, 0 );
			}
			break;
			
		case 1:
#ifdef USE_NEW_JALR_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPJALR> );
#else
			return -1;
#endif
			
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//e->MovRegFromMem32 ( 0, &r->GPR [ i.Rs ].s );
			//e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//if ( i.Rd )
			//{
			//	e->MovMemImm32 ( &r->GPR [ i.Rd ].s, Address + 8 );
			//	ret = e->MovMemImm32 ( &r->LastModifiedRegister, i.Rd );
			//}
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //JALR instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BEQ ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BEQ";
	static const void* c_vFunction = (const void*) Instruction::Execute::BEQ;
	
	//if ( r->GPR [ i.Rs ].u == r->GPR [ i.Rt ].u )
	//{
	//	r->DelaySlot0.Instruction = i;
	//	r->DelaySlot0.cb = r->_cb_Branch;
	//	r->Status.DelaySlot_Valid |= 0x1;
	//}
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			bStopEncodingAfter = true;

			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( (long*) & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BEQ_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBEQ> );
#else
			return -1;
#endif
			
			//e->MovRegFromMem32 ( 0, &r->GPR [ i.Rs ].s );
			//e->CmpRegMem32 ( 0, &r->GPR [ i.Rt ].s );
			//e->Jmp8_NE ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BEQ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BNE ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BNE";
	static const void* c_vFunction = (const void*) Instruction::Execute::BNE;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			bStopEncodingAfter = true;
			
			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( (long*) & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BNE_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBNE> );
#else
			return -1;
#endif
			
			//e->MovRegFromMem32 ( 0, &r->GPR [ i.Rs ].s );
			//e->CmpRegMem32 ( 0, &r->GPR [ i.Rt ].s );
			//e->Jmp8_E ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BNE instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BLEZ ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BLEZ";
	static const void* c_vFunction = (const void*) Instruction::Execute::BLEZ;
	
	//if ( r->GPR [ i.Rs ].s <= 0 )
	//{
	//	// next instruction is in the branch delay slot
	//	r->DelaySlot0.Instruction = i;
	//	r->DelaySlot0.cb = r->_cb_Branch;
	//	r->Status.DelaySlot_Valid |= 0x1;
	//}
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			bStopEncodingAfter = true;
			
			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( (long*) & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BLEZ_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBLEZ> );
#else
			return -1;
#endif
			
			//e->CmpMemImm32 ( &r->GPR [ i.Rs ].s, 0 );
			//e->Jmp8_G ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BLEZ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BGTZ ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BGTZ";
	static const void* c_vFunction = (const void*) Instruction::Execute::BGTZ;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			bStopEncodingAfter = true;
			
			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( (long*) & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BGTZ_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBGTZ> );
#else
			return -1;
#endif
			
			//e->CmpMemImm32 ( &r->GPR [ i.Rs ].s, 0 );
			//e->Jmp8_LE ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BGTZ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BLTZ ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BLTZ";
	static const void* c_vFunction = (const void*) Instruction::Execute::BLTZ;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	//bStopEncodingAfter = true;
	
	// *testing*
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			bStopEncodingAfter = true;
			
			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( (long*) & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BLTZ_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBLTZ> );
#else
			return -1;
#endif
			
			//e->CmpMemImm32 ( &r->GPR [ i.Rs ].s, 0 );
			//e->Jmp8_GE ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BLTZ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BGEZ ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BGEZ";
	static const void* c_vFunction = (const void*) Instruction::Execute::BGEZ;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			bStopEncodingAfter = true;
			
			// *MUST* update PC if branch will return to delay slot (at least for now)
			e->MovMemImm32 ( (long*) & r->PC, Address );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BGEZ_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBGEZ> );
#else
			return -1;
#endif
			
			//e->CmpMemImm32 ( &r->GPR [ i.Rs ].s, 0 );
			//e->Jmp8_L ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//ret = e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BGEZ instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BLTZAL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BLTZAL";
	static const void* c_vFunction = (const void*) Instruction::Execute::BLTZAL;
	
	//if ( r->GPR [ i.Rs ].s < 0 )
	//{
	//	r->DelaySlot0.Instruction = i;
	//	r->DelaySlot0.cb = r->_cb_Branch;
	//	r->Status.DelaySlot_Valid |= 0x1;
	//}
	//r->GPR [ 31 ].u = r->PC + 8;
	//CHECK_DELAYSLOT ( 31 );
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// needs to also stop encoding before at level 0, because it requires an updated PC to link
			bStopEncodingBefore = true;
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BLTZAL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBLTZAL> );
#else
			return -1;
#endif
			
			//e->CmpMemImm32 ( &r->GPR [ i.Rs ].s, 0 );
			//e->Jmp8_GE ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			//e->MovMemImm32 ( &r->GPR [ 31 ].u, Address + 8 );
			//ret = e->MovMemImm32 ( &r->LastModifiedRegister, 31 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BLTZAL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BGEZAL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BGEZAL";
	static const void* c_vFunction = (const void*) Instruction::Execute::BGEZAL;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// needs to also stop encoding before at level 0, because it requires an updated PC to link
			bStopEncodingBefore = true;
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_BGEZAL_CODE
			ret = Generate_Normal_Branch ( i, Address, (void*) Cpu::ProcessBranchDelaySlot_t<OPBGEZAL> );
#else
			return -1;
#endif
			
			//e->CmpMemImm32 ( &r->GPR [ i.Rs ].s, 0 );
			//e->Jmp8_L ( 0, 0 );
			//e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			//e->OrMemImm64 ( &r->Status.Value, 1 );
			//e->SetJmpTarget8 ( 0 );
			//e->MovMemImm32 ( &r->GPR [ 31 ].u, Address + 8 );
			//ret = e->MovMemImm32 ( &r->LastModifiedRegister, 31 );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BGEZAL instruction.\n";
		return -1;
	}
	return 1;
}



/////////////////////////////////////////////////////////////
// Multiply/Divide Instructions

long Recompiler::MULT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MULT";
	static const void* c_vFunction = (const void*) Instruction::Execute::MULT;
	
	// if rs is between -0x800 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff or between -0x7ff and -0x100000, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	//{
	//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	//}
	//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
	//if ( r->GPR [ i.Rs ].s < 0x800 && r->GPR [ i.Rs ].s >= -0x800 )
	//{
	//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
	//}
	//else if ( r->GPR [ i.Rs ].s < 0x100000 && r->GPR [ i.Rs ].s >= -0x100000 )
	//{
	//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
	//}
	// multiply signed Lo,Hi = rs * rt
	//r->HiLo.sValue = ((s64) (r->GPR [ i.Rs ].s)) * ((s64) (r->GPR [ i.Rt ].s));
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// need to stop encoding before at level 0 to get an updated CycleCount
			// need to stop encoding after at level 0 because it updates CycleCount
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_MULT_CODE
			//bResetCycleCount = true;
			
			// calculate cycles mul/div unit will be busy for
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
			//if ( r->GPR [ i.Rs ].s < 0x800 && r->GPR [ i.Rs ].s >= -0x800 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
			//}
			//else if ( r->GPR [ i.Rs ].s < 0x100000 && r->GPR [ i.Rs ].s >= -0x100000 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
			//}
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].s );
			e->Cdq ();
			e->XorRegReg32 ( RAX, RDX );
			//e->MovRegReg32 ( RAX, RCX );
			//e->XorReg32ImmX ( RAX, -1 );
			//e->CmovSRegReg32 ( RAX, RCX );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Slow );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Med );
			e->CmpReg32ImmX ( RAX, 0x100000 );
			e->CmovBRegReg32 ( RCX, RDX );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Fast );
			e->CmpReg32ImmX ( RAX, 0x800 );
			e->CmovBRegReg32 ( RCX, RDX );
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, (long long*) & r->CycleCount );
			//e->MovRegMem64 ( RCX, (long long*) & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			e->AddRegReg64 ( RCX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			e->SubRegMem64 ( RAX, (long long*) & r->MulDiv_BusyUntil_Cycle );
			//e->CmovBRegReg64 ( RDX, RAX );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
			e->SubMemReg64 ( (long long*) & r->CycleCount, RDX );
			
			// store the correct busy until cycle back
			e->SubRegReg64 ( RCX, RDX );
			e->MovMemReg64 ( (long long*) & r->MulDiv_BusyUntil_Cycle, RCX );
			
			// do the multiply
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].s );
			e->ImulRegMem32 ( & r->GPR [ i.Rt ].s );
			e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			ret = e->MovMemReg32 ( & r->HiLo.sHi, RDX );
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MULT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::MULTU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MULTU";
	static const void* c_vFunction = (const void*) Instruction::Execute::MULTU;
	
	// if rs is between 0 and 0x7ff, then multiply takes 6 cycles
	static const int c_iMultiplyCycles_Fast = 6;
	
	// if rs is between 0x800 and 0xfffff, then multiply takes 9 cycles
	static const int c_iMultiplyCycles_Med = 9;
	
	// otherwise, multiply takes 13 cycles
	static const int c_iMultiplyCycles_Slow = 13;
	
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// need to stop encoding before at level 0 to get an updated CycleCount
			// need to stop encoding after at level 0 because it updates CycleCount
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_MULTU_CODE
			//bResetCycleCount = true;
			
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Slow;
			//if ( r->GPR [ i.Rs ].u < 0x800 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Fast;
			//}
			//else if ( r->GPR [ i.Rs ].u < 0x100000 )
			//{
			//	r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iMultiplyCycles_Med;
			//}
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].s );
			e->MovReg32ImmX ( RCX, c_iMultiplyCycles_Slow );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Med );
			e->CmpReg32ImmX ( RAX, 0x100000 );
			e->CmovBRegReg32 ( RCX, RDX );
			e->MovReg32ImmX ( RDX, c_iMultiplyCycles_Fast );
			e->CmpReg32ImmX ( RAX, 0x800 );
			e->CmovBRegReg32 ( RCX, RDX );
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, (long long*) & r->CycleCount );
			//e->MovRegMem64 ( RCX, (long long*) & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			e->AddRegReg64 ( RCX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			e->SubRegMem64 ( RAX, (long long*) & r->MulDiv_BusyUntil_Cycle );
			//e->CmovBRegReg64 ( RDX, RAX );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
			e->SubMemReg64 ( (long long*) & r->CycleCount, RDX );
			
			// store the correct busy until cycle back
			e->SubRegReg64 ( RCX, RDX );
			e->MovMemReg64 ( (long long*) & r->MulDiv_BusyUntil_Cycle, RCX );
			
			// do the multiply
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].s );
			e->MulRegMem32 ( & r->GPR [ i.Rt ].s );
			e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			e->MovMemReg32 ( & r->HiLo.sHi, RDX );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MULTU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DIV ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "DIV";
	static const void* c_vFunction = (const void*) Instruction::Execute::DIV;
	
	static const int c_iDivideCycles = 36;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
			
		case 1:
#ifdef USE_NEW_DIV_CODE
			//bResetCycleCount = true;
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, (long long*) & r->CycleCount );
			//e->MovRegMem64 ( RDX, (long long*) & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, (long long*) & r->MulDiv_BusyUntil_Cycle );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
			e->SubMemReg64 ( (long long*) & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_DIVIDE_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iDivideCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( (long long*) & r->MulDiv_BusyUntil_Cycle, RCX );
			
			// mult/div unit is busy now
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iDivideCycles;
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddReg64ImmX ( RAX, c_iDivideCycles );
			//e->MovMemReg64 ( (long long*) & r->MulDiv_BusyUntil_Cycle, RAX );
			
			// divide signed: Lo = rs / rt; Hi = rs % rt
			//if ( r->GPR [ i.Rt ].u != 0 )
			//{
			//	// if rs = 0x80000000 and rt = -1 then hi = 0 and lo = 0x80000000
			//	if ( r->GPR [ i.Rs ].u == 0x80000000 && r->GPR [ i.Rt ].s == -1 )
			//	{
			//		r->HiLo.uHi = 0;
			//		r->HiLo.uLo = 0x80000000;
			//	}
			//	else
			//	{
			//		r->HiLo.sLo = r->GPR [ i.Rs ].s / r->GPR [ i.Rt ].s;
			//		r->HiLo.sHi = r->GPR [ i.Rs ].s % r->GPR [ i.Rt ].s;
			//	}
			//}
			//else
			//{
			//	if ( r->GPR [ i.Rs ].s < 0 )
			//	{
			//		r->HiLo.sLo = 1;
			//	}
			//	else
			//	{
			//		r->HiLo.sLo = -1;
			//	}
			//	r->HiLo.uHi = r->GPR [ i.Rs ].u;
			//}
			//e->MovRegMem32 ( RCX, & r->GPR [ i.Rt ].s );
			//e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].s );
			e->MovsxdReg64Mem32 ( RCX, & r->GPR [ i.Rt ].s );
			e->MovsxdReg64Mem32 ( RAX, & r->GPR [ i.Rs ].s );
			
			//e->MovReg32ImmX ( RCX, -1 );
			//e->MovReg32ImmX ( RDX, 1 );
			//e->OrRegReg32 ( RAX, RAX );
			//e->CmovSRegReg32 ( RCX, RDX );
			//e->Cdq ();
			e->Cqo ();
			e->NotReg32 ( RDX );
			e->OrReg32ImmX ( RDX, 1 );
			
			//e->OrRegReg64 ( RCX, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->Jmp8_ECXZ ( 0, 0 );
			
			//e->MovRegReg64 ( RDX, RAX );
			//e->SarRegImm64 ( RDX, 63 );
			//e->Cqo ();
			e->Cdq ();
			//e->IdivRegReg64 ( RCX );
			e->IdivRegReg32 ( RCX );
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			//e->Jmp8 ( 0, 1 );
			e->XchgRegReg32( RAX, RDX );
			
			e->SetJmpTarget8 ( 0 );
			
			
			e->MovMemReg32 ( & r->HiLo.sHi, RAX );
			e->MovMemReg32 ( & r->HiLo.sLo, RDX );
			
			//e->SetJmpTarget8 ( 1 );
			
			// done //
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DIV instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DIVU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "DIVU";
	static const void* c_vFunction = (const void*) Instruction::Execute::DIVU;
	
	static const int c_iDivideCycles = 36;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			// actually needs to stop encoding before for now, because the correct current cycle count is needed
			bStopEncodingAfter = true;
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_DIVU_CODE
			//bResetCycleCount = true;
			
			// check if mul/div unit is in use
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, (long long*) & r->CycleCount );
			//e->MovRegMem64 ( RDX, (long long*) & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			e->MovRegReg64 ( RCX, RAX );
			e->SubRegMem64 ( RAX, (long long*) & r->MulDiv_BusyUntil_Cycle );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			//e->XorRegReg64 ( RDX, RDX );
			//e->SubRegReg64 ( RAX, RCX );
			//e->CmovBRegReg64 ( RDX, RAX );
			
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			// or, no need to subtract one if we don't count the cycle for the instruction (can subtract one when resetting the cycle count)
			//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
			e->SubMemReg64 ( (long long*) & r->CycleCount, RDX );
			
			
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddRegReg64 ( RDX, RAX );
			e->SubRegReg64 ( RCX, RDX );
			
#ifdef ENABLE_DIVIDE_LATENCY
			// add in the latency for multiply
			e->AddReg64ImmX ( RCX, c_iDivideCycles );
#endif

			// write back the new busy until cycle
			e->MovMemReg64 ( (long long*) & r->MulDiv_BusyUntil_Cycle, RCX );
			
			// mult/div unit is busy now
			//r->MulDiv_BusyUntil_Cycle = r->CycleCount + c_iDivideCycles;
			// add the REAL CycleCount (CPU) to the cycles for the multiply and store to the BusyUntil Cycle for Mul/Div unit
			//e->AddReg64ImmX ( RAX, c_iDivideCycles );
			//e->MovMemReg64 ( (long long*) & r->MulDiv_BusyUntil_Cycle, RAX );
			
			// divide unsigned: Lo = rs / rt; Hi = rs % rt
			//if ( r->GPR [ i.Rt ].u != 0 )
			//{
			//	r->HiLo.uLo = r->GPR [ i.Rs ].u / r->GPR [ i.Rt ].u;
			//	r->HiLo.uHi = r->GPR [ i.Rs ].u % r->GPR [ i.Rt ].u;
			//}
			//else
			//{
			//	r->HiLo.sLo = -1;
			//	r->HiLo.uHi = r->GPR [ i.Rs ].u;
			//}
			e->MovRegMem32 ( RCX, & r->GPR [ i.Rt ].s );
			e->MovRegMem32 ( RAX, & r->GPR [ i.Rs ].s );
			//e->OrRegReg64 ( RCX, RCX );
			//e->Jmp8_E ( 0, 0 );
			e->MovReg32ImmX ( RDX, -1 );
			e->Jmp8_ECXZ ( 0, 0 );
			
			e->XorRegReg32 ( RDX, RDX );
			e->DivRegReg32 ( RCX );
			//e->MovMemReg32 ( & r->HiLo.sLo, RAX );
			//e->MovMemReg32 ( & r->HiLo.sHi, RDX );
			//e->Jmp8 ( 0, 1 );
			e->XchgRegReg32 ( RAX, RDX );
			
			e->SetJmpTarget8 ( 0 );
			
			//e->MovMemImm32 ( & r->HiLo.sLo, -1 );
			e->MovMemReg32 ( & r->HiLo.sLo, RDX );
			e->MovMemReg32 ( & r->HiLo.sHi, RAX );
			
			//e->SetJmpTarget8 ( 1 );
			
			// done //
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DIVU instruction.\n";
		return -1;
	}
	return 1;
}



long Recompiler::MFHI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MFHI";
	static const void* c_vFunction = (const void*) Instruction::Execute::MFHI;
	
	//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
	//{
	//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
	//}
	//r->GPR [ i.Rd ].u = r->HiLo.uHi;
	//CHECK_DELAYSLOT ( i.Rd );
	
	int ret = 1;
	
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction (can update CycleCount)
			bStopEncodingAfter = true;
			
			// for now, stop encoding before, because an updated CycleCount is needed to determine if Mul/Div is done
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// make sure Rd is not r0
			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm32 ( &r->GPR [ 0 ].s, 0 );
			}
			break;
			
		case 1:
#ifdef USE_NEW_MFHI_CODE
			/*
			e->MovRegFromMem64 ( RAX, &r->CycleCount );
			e->CmpRegMem64 ( RAX, &r->MulDiv_BusyUntil_Cycle );
			e->CmovBRegMem64 ( RAX, &r->MulDiv_BusyUntil_Cycle );
			ret = e->MovRegToMem64 ( &r->CycleCount, RAX );
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->HiLo.uHi );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
			}
			Local_LastModifiedReg = i.Rd;
			*/
			
			// this instruction interlocks if multiply/divide unit is busy
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, (long long*) & r->CycleCount );
			//e->MovRegMem64 ( RCX, (long long*) & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			//e->XorRegReg64 ( RDX, RDX );
			e->SubRegMem64 ( RAX, (long long*) & r->MulDiv_BusyUntil_Cycle );
			//e->CmovBRegReg64 ( RDX, RAX );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
			e->SubMemReg64 ( (long long*) & r->CycleCount, RDX );
			
			// move from Hi register
			//r->GPR [ i.Rd ].u = r->HiLo.uHi;
			//CHECK_DELAYSLOT ( i.Rd );
			if ( i.Rd )
			{
				e->MovRegMem32 ( RAX, & r->HiLo.sHi );
				e->MovMemReg32 ( & r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MFHI instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::MFLO ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MFLO";
	static const void* c_vFunction = (const void*) Instruction::Execute::MFLO;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction (can update CycleCount)
			bStopEncodingAfter = true;
			
			// for now, stop encoding before, because an updated CycleCount is needed to determine if Mul/Div is done
			bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			// make sure Rd is not r0
			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm32 ( &r->GPR [ 0 ].s, 0 );
			}
			break;
			
		case 1:
#ifdef USE_NEW_MFLO_CODE
			/*
			e->MovRegFromMem64 ( RAX, &r->CycleCount );
			e->CmpRegMem64 ( RAX, &r->MulDiv_BusyUntil_Cycle );
			e->CmovBRegMem64 ( RAX, &r->MulDiv_BusyUntil_Cycle );
			ret = e->MovRegToMem64 ( &r->CycleCount, RAX );
			if ( i.Rd )
			{
				e->MovRegFromMem32 ( RAX, &r->HiLo.uLo );
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
			}
			Local_LastModifiedReg = i.Rd;
			*/
			
			// this instruction interlocks if multiply/divide unit is busy
			//if ( r->MulDiv_BusyUntil_Cycle > r->CycleCount )
			//{
			//	// for now, just add onto memory latency
			//	r->CycleCount = r->MulDiv_BusyUntil_Cycle;
			//}
			e->MovRegMem64 ( RAX, (long long*) & r->CycleCount );
			//e->MovRegMem64 ( RCX, (long long*) & r->MulDiv_BusyUntil_Cycle );
			e->AddReg64ImmX ( RAX, LocalCycleCount );
			
			//e->XorRegReg64 ( RDX, RDX );
			e->SubRegMem64 ( RAX, (long long*) & r->MulDiv_BusyUntil_Cycle );
			//e->CmovBRegReg64 ( RDX, RAX );
			e->Cqo ();
			e->AndRegReg64 ( RDX, RAX );
			
			// store cycle count minus one to the current cycle (because it adds one on return from recompiler for now)
			//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
			e->SubMemReg64 ( (long long*) & r->CycleCount, RDX );
			
			// move from Lo register
			//r->GPR [ i.Rd ].u = r->HiLo.uLo;
			//CHECK_DELAYSLOT ( i.Rd );
			if ( i.Rd )
			{
				e->MovRegMem32 ( RAX, & r->HiLo.sLo );
				e->MovMemReg32 ( & r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MFLO instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::MTHI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MTHI";
	static const void* c_vFunction = (const void*) Instruction::Execute::MTHI;
	
	//r->HiLo.uHi = r->GPR [ i.Rs ].u;
	
	// ***TODO*** should this sync with mul/div unit??
	
	int ret = 1;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
			ret = e->MovRegToMem32 ( &r->HiLo.sHi, RAX );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MTHI instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::MTLO ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MTLO";
	static const void* c_vFunction = (const void*) Instruction::Execute::MTLO;
	
	//r->HiLo.uLo = r->GPR [ i.Rs ].u;
	
	// ***TODO*** should this sync with mul/div unit??
	
	int ret = 1;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
			ret = e->MovRegToMem32 ( &r->HiLo.sLo, RAX );
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MTLO instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::RFE ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "RFE";
	static const void* c_vFunction = (const void*) Instruction::Execute::RFE;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	
	// *testing*
	bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //RFE instruction.\n";
		return -1;
	}
	return 1;
}




////////////////////////////////////////////////////////
// Instructions that can cause Synchronous Interrupts //
////////////////////////////////////////////////////////


long Recompiler::ADD ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "ADD";
	static const void* c_vFunction = (const void*) Instruction::Execute::ADD;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// go ahead and say that it modifies NextPC since it might, even if it does not this time around
			// but only if instruction is actually encoded
			Local_NextPCModified = true;
			
			// need to stop encoding before, because if it sync ints, that requires PC to be updated
			bStopEncodingBefore = true;
			
			// need to stop encoding after because if it sync ints, then it needs to "jump"
			bStopEncodingAfter = true;
			
			// update NextPC before executing instruction at level 0 since it may do a sync int
			e->MovMemImm32 ( (long*) &r->NextPC, Address + 4 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm32 ( &r->GPR [ 0 ].s, 0 );
			}
			
			break;
			
		case 1:
#ifdef USE_NEW_ADD_CODE
			e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
			e->AddRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
			
			if ( i.Rs && i.Rt )
			{
				// branch if not signed overflow
				e->Jmp8_NO ( 0, 0 );
				
				// update CycleCount, set PC, then jump to synchronous interrupt
				//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
				e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + MemCycles );
				//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount );
				
				// set pc
				e->MovMemImm32 ( (long*) & r->PC, Address );
				
				//r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
				e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_OV> );
				
				// continue processing store from here //
				e->SetJmpTarget8 ( 0 );
			}
			
			// check if destination is r0
			if ( i.Rd )
			{
				// store result if not signed overflow
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADD instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::ADDI ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "ADDI";
	static const void* c_vFunction = (const void*) Instruction::Execute::ADDI;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// go ahead and say that it modifies NextPC since it might, even if it does not this time around
			// but only if instruction is actually encoded
			Local_NextPCModified = true;
			
			// need to stop encoding before, because if it sync ints, that requires PC to be updated
			bStopEncodingBefore = true;
			
			// need to stop encoding after because if it sync ints, then it needs to "jump"
			bStopEncodingAfter = true;
			
			// update NextPC before executing instruction at level 0 since it may do a sync int
			e->MovMemImm32 ( (long*) &r->NextPC, Address + 4 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			if ( !i.Rt )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm32 ( &r->GPR [ 0 ].s, 0 );
			}
			
			break;
			
		case 1:
#ifdef USE_NEW_ADDI_CODE
			e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
			//e->AddRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
			e->AddReg32ImmX ( RAX, i.sImmediate );
			
			if ( i.Rs && i.sImmediate )
			{
				// branch if not signed overflow
				e->Jmp8_NO ( 0, 0 );
				
				// update CycleCount, set PC, then jump to synchronous interrupt
				//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
				e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + MemCycles );
				//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount );
				
				// set pc
				e->MovMemImm32 ( (long*) & r->PC, Address );
				
				//r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
				e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_OV> );
				
				// continue processing store from here //
				e->SetJmpTarget8 ( 0 );
			}
			
			// check if destination is r0
			if ( i.Rt )
			{
				// store result if not signed overflow
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rt ].s, RAX );
			}
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //ADDI instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SUB ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SUB";
	static const void* c_vFunction = (const void*) Instruction::Execute::SUB;
	
	int ret = 1;
	
	switch ( OpLevel )
	{
		case 0:
			// go ahead and say that it modifies NextPC since it might, even if it does not this time around
			// but only if instruction is actually encoded
			Local_NextPCModified = true;
			
			// need to stop encoding before, because if it sync ints, that requires PC to be updated
			bStopEncodingBefore = true;
			
			// need to stop encoding after because if it sync ints, then it needs to "jump"
			bStopEncodingAfter = true;
			
			// update NextPC before executing instruction at level 0 since it may do a sync int
			e->MovMemImm32 ( (long*) &r->NextPC, Address + 4 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			if ( !i.Rd )
			{
				// make sure r0 stays at zero
				ret = e->MovMemImm32 ( &r->GPR [ 0 ].s, 0 );
			}
			break;
			
		case 1:
#ifdef USE_NEW_SUB_CODE
			e->MovRegFromMem32 ( RAX, &r->GPR [ i.Rs ].s );
			e->SubRegMem32 ( RAX, &r->GPR [ i.Rt ].s );
			
			if ( i.Rt )
			{
				// branch if not signed overflow
				e->Jmp8_NO ( 0, 0 );
				
				// update CycleCount, set PC, then jump to synchronous interrupt
				//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
				e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + MemCycles );
				//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount );
				
				// set pc
				e->MovMemImm32 ( (long*) & r->PC, Address );
				
				//r->ProcessSynchronousInterrupt ( Cpu::EXC_OV );
				e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_OV> );
				
				// continue processing store from here //
				e->SetJmpTarget8 ( 0 );
			}
			
			// check if destination is r0
			if ( i.Rd )
			{
				// store result if not signed overflow
				ret = e->MovRegToMem32 ( &r->GPR [ i.Rd ].s, RAX );
			}
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SUB instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::SYSCALL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SYSCALL";
	static const void* c_vFunction = (const void*) Instruction::Execute::SYSCALL;
	
	int ret = 1;
	
	// stop encoding after since it is an unconditional synchronous interrupt
	bStopEncodingAfter = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	
	switch ( OpLevel )
	{
		case 0:
			
			// stop encoding before due to synchronous interrupt (needs updated PC,LastPC)
			bStopEncodingBefore = true;
			
			// update NextPC before executing instruction at level 0 since it may do a sync int
			// note: no need for this since it is an unconditional sync int and stopped encoding before
			//e->MovMemImm32 ( (long*) &r->NextPC, Address + 4 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
			
#ifdef USE_NEW_SYSCALL_CODE
			// update CycleCount, set PC, then jump to synchronous interrupt
#ifdef ENABLE_R3000A_BRANCH_PENALTY
			e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + MemCycles + r->c_ullLatency_PipelineRefill );
#else
			//e->MovMemReg64 ( (long long*) & r->CycleCount, RAX );
			e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount + MemCycles );
			//e->AddMem64ImmX ( (long long*) & r->CycleCount, LocalCycleCount );
#endif
			
			// set pc
			e->MovMemImm32 ( (long*) & r->PC, Address );
			
			//r->ProcessSynchronousInterrupt ( Cpu::EXC_SYSCALL );
			e->JMP ( (void*) Cpu::ProcessSynchronousInterrupt_t<Cpu::EXC_SYSCALL> );
#else
			return -1;
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SYSCALL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::BREAK ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "BREAK";
	static const void* c_vFunction = (const void*) Instruction::Execute::BREAK;
	
	int ret = 1;
	
	// stop encoding after since it is an unconditional synchronous interrupt
	bStopEncodingAfter = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			// also need to stop encoding before, because it needs both PC and LastPC updated first
			bStopEncodingBefore = true;
			
			// update NextPC before executing instruction at level 0 since it may do a sync int
			// note: no need for this since it is an unconditional sync int
			//e->MovMemImm32 ( (long*) &r->NextPC, Address + 4 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //BREAK instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::Invalid ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "Invalid";
	static const void* c_vFunction = (const void*) Instruction::Execute::Invalid;
	
	int ret = 1;
	
	// stop encoding after since it is an unconditional synchronous interrupt
	bStopEncodingAfter = true;
	
	// this instruction always has a synchronous interrupt
	Local_NextPCModified = true;
	
	switch ( OpLevel )
	{
		case 0:
			// also need to stop encoding before, because it needs both PC and LastPC updated first
			bStopEncodingBefore = true;
			
			// update NextPC before executing instruction at level 0 since it may do a sync int
			// note: no need for this since it is an unconditional sync int
			//e->MovMemImm32 ( (long*) &r->NextPC, Address + 4 );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //Invalid instruction.\n";
		return -1;
	}
	return 1;
}





long Recompiler::MFC0 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MFC0";
	static const void* c_vFunction = (const void*) Instruction::Execute::MFC0;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// should put something in delay slot, so return after this instruction at level 0
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MFC0 instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::MTC0 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MTC0";
	static const void* c_vFunction = (const void*) Instruction::Execute::MTC0;
	
	int ret = 1;
	
	// *testing*
	//bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case 0:
			// should put something in delay slot, so return after this instruction at level 0
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MTC0 instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::MFC2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MFC2";
	static const void* c_vFunction = (const void*) Instruction::Execute::MFC2;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	
	// probably needs an updated CycleCount, so need to stop encoding before too for now
	bStopEncodingBefore = true;
	
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MFC2 instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::MTC2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MTC2";
	static const void* c_vFunction = (const void*) Instruction::Execute::MTC2;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	
	// probably needs an updated CycleCount, so need to stop encoding before too for now
	bStopEncodingBefore = true;
	
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MTC2 instruction.\n";
		return -1;
	}
	return 1;
}




long Recompiler::CFC2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "CFC2";
	static const void* c_vFunction = (const void*) Instruction::Execute::CFC2;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	
	// probably needs an updated CycleCount, so need to stop encoding before too for now
	bStopEncodingBefore = true;
	
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CFC2 instruction.\n";
		return -1;
	}
	return 1;
}



long Recompiler::CTC2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "CTC2";
	static const void* c_vFunction = (const void*) Instruction::Execute::CTC2;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	
	// probably needs an updated CycleCount, so need to stop encoding before too for now
	bStopEncodingBefore = true;
	
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CTC2 instruction.\n";
		return -1;
	}
	return 1;
}




// Load/Store - will need to use address translation to get physical addresses when needed

//////////////////////////////////////////////////////////////////////////
// store instructions

// store instructions
long Recompiler::SB ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SB";
	static const void* c_vFunction = (const void*) Instruction::Execute::SB;
	
	int ret = 1;
	
/*
	// for now, stop encoding after this instruction
	// should be able to stop before, but continue after
#ifndef ENABLE_STORE_PREFIX
	bStopEncodingBefore = true;
#endif
#ifndef ENABLE_STORE_SUFFIX
	bStopEncodingAfter = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			break;
			
		case 0:
			// can just stop encoding before and ignore the prefix requirement at level 0 for testing
			bStopEncodingBefore = true;
			//bStopEncodingAfter = true;
		
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0x0, (void*) Instruction::Execute::SB_Recompiler );
#else
			return -1;
#endif

			break;
			
#ifdef USE_NEW_SB_CODE2
		case 2:
			if ( isConst ( i.Rs ) )
			{
				ret = Generate_Normal_Store_L2 ( i, Address, 0x0, GetConst( i.Rs ) );
				if ( !ret ) return -1;
			}
			else
			{
				return -1;
			}
			break;
#endif

		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SB instruction.\n";
		return -1;
	}
	return 1;
}





long Recompiler::SH ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SH";
	static const void* c_vFunction = (const void*) Instruction::Execute::SH;
	
	int ret = 1;
	
/*
	// for now, stop encoding after this instruction
#ifndef ENABLE_STORE_PREFIX
	bStopEncodingBefore = true;
#endif
#ifndef ENABLE_STORE_SUFFIX
	bStopEncodingAfter = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			break;
			
		case 0:	
			// can just stop encoding before and ignore the prefix requirement at level 0 for testing
			bStopEncodingBefore = true;
			//bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;

			
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0x1, (void*) Instruction::Execute::SH_Recompiler );
#else
			return -1;
#endif

			break;
			
#ifdef USE_NEW_SH_CODE2
		case 2:
			if ( isConst ( i.Rs ) )
			{
				ret = Generate_Normal_Store_L2 ( i, Address, 0x1, GetConst( i.Rs ) );
				if ( !ret ) return -1;
			}
			else
			{
				return -1;
			}
			break;
#endif

		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SH instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SW ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SW";
	static const void* c_vFunction = (const void*) Instruction::Execute::SW;
	
	int ret = 1;
	
/*
	// for now, stop encoding after this instruction
#ifndef ENABLE_STORE_PREFIX
	bStopEncodingBefore = true;
#endif
#ifndef ENABLE_STORE_SUFFIX
	bStopEncodingAfter = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			break;
			
		case 0:
			// can just stop encoding before and ignore the prefix requirement at level 0 for testing
			bStopEncodingBefore = true;
			//bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0x3, (void*) Instruction::Execute::SW_Recompiler );
#else
			return -1;
#endif

			break;
			
#ifdef USE_NEW_SW_CODE2
		case 2:
			if ( isConst ( i.Rs ) )
			{
				ret = Generate_Normal_Store_L2 ( i, Address, 0x3, GetConst( i.Rs ) );
				if ( !ret ) return -1;
			}
			else
			{
				return -1;
			}
			break;
#endif

		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SW instruction.\n";
		return -1;
	}
	return 1;
}


long Recompiler::SWC2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SWC2";
	static const void* c_vFunction = (const void*) Instruction::Execute::SWC2;
	
	int ret = 1;
	
	// for now, stop encoding after this instruction
	// and before
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			break;
			
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		// TODO: no case 1 ?? //
			
#ifdef USE_NEW_SWC2_CODE2
		case 2:
			if ( isConst ( i.Rs ) )
			{
				ret = Generate_Normal_Store_L2 ( i, Address, 0x3, GetConst( i.Rs ) );
				if ( !ret ) return -1;
			}
			else
			{
				return -1;
			}
			break;
#endif

		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SWC2 instruction.\n";
		return -1;
	}
	return 1;
}



long Recompiler::SWL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SWL";
	static const void* c_vFunction = (const void*) Instruction::Execute::SWL;
	
	int ret = 1;
	
/*
	// for now, stop encoding after this instruction
#ifndef ENABLE_STORE_PREFIX
	bStopEncodingBefore = true;
#endif
#ifndef ENABLE_STORE_SUFFIX
	bStopEncodingAfter = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			break;
			
		case 0:
			// can just stop encoding before and ignore the prefix requirement at level 0 for testing
			bStopEncodingBefore = true;
			//bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0x0, (void*) Instruction::Execute::SWL_Recompiler );
#else
			return -1;
#endif

			break;
			
#ifdef USE_NEW_SWL_CODE2
		case 2:
			if ( isConst ( i.Rs ) )
			{
				ret = Generate_Normal_Store_L2 ( i, Address, 0x0, GetConst( i.Rs ) );
				if ( !ret ) return -1;
			}
			else
			{
				return -1;
			}
			break;
#endif

		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SWL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SWR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SWR";
	static const void* c_vFunction = (const void*) Instruction::Execute::SWR;
	
	int ret = 1;
	
/*
	// for now, stop encoding after this instruction
#ifndef ENABLE_STORE_PREFIX
	bStopEncodingBefore = true;
#endif
#ifndef ENABLE_STORE_SUFFIX
	bStopEncodingAfter = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case -1:
			// source registers are Rs and Rt
			ullSrcRegBitmap |= ( 1ull << i.Rs ) | ( 1ull << i.Rt );
			
			break;
			
		case 0:
			// can just stop encoding before and ignore the prefix requirement at level 0 for testing
			bStopEncodingBefore = true;
			//bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
			
		case 1:
#ifdef USE_NEW_STORE_CODE
			ret = Generate_Normal_Store ( i, Address, 0x0, (void*) Instruction::Execute::SWR_Recompiler );
#else
			return -1;
#endif

			break;
			
#ifdef USE_NEW_SWR_CODE2
		case 2:
			if ( isConst ( i.Rs ) )
			{
				ret = Generate_Normal_Store_L2 ( i, Address, 0x0, GetConst( i.Rs ) );
				if ( !ret ) return -1;
			}
			else
			{
				return -1;
			}
			break;
#endif

		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SWR instruction.\n";
		return -1;
	}
	return 1;
}



/////////////////////////////////////////////////
// load instructions

// load instructions with delay slot
// *** todo *** it is also possible to this and just process load after load delay slot has executed - would still need previous load address before delay slot
// *** todo *** could also skip delay slot zero and put straight into delay slot 1 after next instruction, or just process load delay slot after next instruction
long Recompiler::LB ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LB";
	static const void* c_vFunction = (const void*) Instruction::Execute::LB;
	
	//LoadAddress = r->GPR [ i.Base ].s + i.sOffset;
	//r->DelaySlot0.Instruction = i;
	//r->DelaySlot0.Data = LoadAddress;
	//r->DelaySlot0.cb = LB_DelaySlot_Callback_Bus;
	//r->Status.DelaySlot_Valid |= 0x1;
	//r->LastModifiedRegister = 255;
	int ret = 1;

/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x0, (void*) Cpu::ProcessLoadDelaySlot_t<OPLB,0>, (void*) DataBus::Read_t<0xff> );
#else
			return -1;
#endif
			
			/*
			e->MovRegImm32 ( 0, i.sOffset );
			e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			e->OrMemImm64 ( &r->Status.Value, 1 );
			ret = e->MovMemImm32 ( &r->LastModifiedRegister, 255 );
			*/
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}






long Recompiler::LH ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LH";
	static const void* c_vFunction = (const void*) Instruction::Execute::LH;
	
	int ret = 1;

/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif

			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x1, (void*) Cpu::ProcessLoadDelaySlot_t<OPLH,0>, (void*) DataBus::Read_t<0xffff> );
#else
			return -1;
#endif
			
			/*
			e->MovRegImm32 ( 0, i.sOffset );
			e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			e->OrMemImm64 ( &r->Status.Value, 1 );
			ret = e->MovMemImm32 ( &r->LastModifiedRegister, 255 );
			*/
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}








long Recompiler::LW ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LW";
	static const void* c_vFunction = (const void*) Instruction::Execute::LW;
	
	int ret = 1;
	
/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x3, (void*) Cpu::ProcessLoadDelaySlot_t<OPLW,0>, (void*) DataBus::Read_t<0xffffffff> );
#else
			return -1;
#endif
			
			/*
			e->MovRegImm32 ( 0, i.sOffset );
			e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			e->OrMemImm64 ( &r->Status.Value, 1 );
			ret = e->MovMemImm32 ( &r->LastModifiedRegister, 255 );
			*/
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LBU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LBU";
	static const void* c_vFunction = (const void*) Instruction::Execute::LBU;
	
	int ret = 1;
	
/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x0, (void*) Cpu::ProcessLoadDelaySlot_t<OPLBU,0>, (void*) DataBus::Read_t<0xff> );
#else
			return -1;
#endif
			
			/*
			e->MovRegImm32 ( 0, i.sOffset );
			e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			e->OrMemImm64 ( &r->Status.Value, 1 );
			ret = e->MovMemImm32 ( &r->LastModifiedRegister, 255 );
			*/
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LBU instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LHU ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LHU";
	static const void* c_vFunction = (const void*) Instruction::Execute::LHU;
	
	int ret = 1;
	
/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x1, (void*) Cpu::ProcessLoadDelaySlot_t<OPLHU,0>, (void*) DataBus::Read_t<0xffff> );
#else
			return -1;
#endif
			
			/*
			e->MovRegImm32 ( 0, i.sOffset );
			e->AddRegMem32 ( 0, &r->GPR [ i.Base ].u );
			e->MovMemImm32 ( &r->DelaySlot0.Instruction.Value, i.Value );
			e->MovRegToMem32 ( &r->DelaySlot0.Data, 0 );
			e->OrMemImm64 ( &r->Status.Value, 1 );
			ret = e->MovMemImm32 ( &r->LastModifiedRegister, 255 );
			*/
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LWC2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LWC2";
	static const void* c_vFunction = (const void*) Instruction::Execute::LWC2;
	
	int ret = 1;
	
/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingBefore = true;
			bStopEncodingAfter = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x3, (void*) Cpu::ProcessLoadDelaySlot_t<OPLWC2,0>, (void*) DataBus::Read_t<0xffffffff> );
#else
			return -1;
#endif
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LWC2 instruction.\n";
		return -1;
	}
	return 1;
}




// load instructions without load-delay slot
long Recompiler::LWL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LWL";
	static const void* c_vFunction = (const void*) Instruction::Execute::LWL;
	
	int ret = 1;
	
/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x0, (void*) Cpu::ProcessLoadDelaySlot_t<OPLWL,0>, (void*) DataBus::Read_t<0xffffffff> );
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LB instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::LWR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "LWR";
	static const void* c_vFunction = (const void*) Instruction::Execute::LWR;
	
	int ret = 1;
	
/*
#ifndef ENABLE_LOAD_SUFFIX
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	//bStopEncodingBefore = true;
#endif
*/
	
	switch ( OpLevel )
	{
		case 0:
			// for now, stop encoding after this instruction
			bStopEncodingAfter = true;
			//bStopEncodingBefore = true;
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		case 1:
#ifdef USE_NEW_LOAD_CODE
			ret = Generate_Normal_Load ( i, Address, 0x0, (void*) Cpu::ProcessLoadDelaySlot_t<OPLWR,0>, (void*) DataBus::Read_t<0xffffffff> );
#else
			return -1;
#endif
			
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //LWR instruction.\n";
		return -1;
	}
	return 1;
}




















///////////////////////////
// GTE instructions

long Recompiler::COP2 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "COP2";
	static const void* c_vFunction = (const void*) Instruction::Execute::COP2;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //COP2 instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::RTPS ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "RTPS";
	static const void* c_vFunction = (const void*) Instruction::Execute::RTPS;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //RTPS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCLIP ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCLIP";
	static const void* c_vFunction = (const void*) Instruction::Execute::NCLIP;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCLIP instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::OP ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "OP";
	static const void* c_vFunction = (const void*) Instruction::Execute::OP;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //OP instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DPCS ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "DPCS";
	static const void* c_vFunction = (const void*) Instruction::Execute::DPCS;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DPCS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::INTPL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "INTPL";
	static const void* c_vFunction = (const void*) Instruction::Execute::INTPL;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //INTPL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::MVMVA ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "MVMVA";
	static const void* c_vFunction = (const void*) Instruction::Execute::MVMVA;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //MVMVA instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCDS ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCDS";
	static const void* c_vFunction = (const void*) Instruction::Execute::NCDS;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCDS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::CDP ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "CDP";
	static const void* c_vFunction = (const void*) Instruction::Execute::CDP;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CDP instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCDT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCDT";
	static const void* c_vFunction = (const void*) Instruction::Execute::NCDT;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCDT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCCS ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCCS";
	static const void* c_vFunction = (const void*) Instruction::Execute::NCCS;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCCS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::CC ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "CC";
	static const void* c_vFunction = (const void*) Instruction::Execute::CC;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //CC instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCS ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCS";
	static const void* c_vFunction = (const void*) Instruction::Execute::NCS;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCS instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCT";
	static const void* c_vFunction = (const void*) Instruction::Execute::NCT;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::SQR ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "SQR";
	static const void* c_vFunction = (const void*) Instruction::Execute::SQR;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //SQR instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DCPL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "DCPL";
	static const void* c_vFunction = (const void*) Instruction::Execute::DCPL;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DCPL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::DPCT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "DPCT";
	static const void* c_vFunction = (const void*) Instruction::Execute::DPCT;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //DPCT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::AVSZ3 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "AVSZ3";
	static const void* c_vFunction = (const void*) Instruction::Execute::AVSZ3;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //AVSZ3 instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::AVSZ4 ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "AVSZ4";
	static const void* c_vFunction = (const void*) Instruction::Execute::AVSZ4;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //AVSZ4 instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::RTPT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "RTPT";
	static const void* c_vFunction = (const void*) Instruction::Execute::RTPT;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //RTPT instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::GPF ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "GPF";
	static const void* c_vFunction = (const void*) Instruction::Execute::GPF;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //GPF instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::GPL ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "GPL";
	static const void* c_vFunction = (const void*) Instruction::Execute::GPL;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );
			
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //GPL instruction.\n";
		return -1;
	}
	return 1;
}

long Recompiler::NCCT ( Instruction::Format i, u32 Address )
{
	static const char* c_sName = "NCCT";
	static const void* c_vFunction = (const void*) Instruction::Execute::NCCT;
	
	int ret = 1;
	// for now, stop encoding after this instruction
	bStopEncodingAfter = true;
	bStopEncodingBefore = true;
	// to keep accurate cycle count, update minus 1 after the instruction
	bResetCycleCount = true;
	switch ( OpLevel )
	{
		case 0:
#ifdef RESERVE_STACK_FRAME_FOR_CALL
			e->SubReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			e->LoadImm32 ( RCX, i.Value );
			ret = e->Call ( (void*) c_vFunction );

#ifdef RESERVE_STACK_FRAME_FOR_CALL
			ret = e->AddReg64ImmX ( RSP, c_lSEH_StackSize );
#endif
			break;
			
		default:
			return -1;
			break;
	}
	
	if ( !ret )
	{
		cout << "\nR3000A: x64 Recompiler: Error encoding " << c_sName << " instruction.\n"; //NCCT instruction.\n";
		return -1;
	}
	return 1;
}



const Recompiler::Function Recompiler::FunctionList []
{
	// instructions on both R3000A and R5900
	Recompiler::Invalid,
	Recompiler::J, Recompiler::JAL, Recompiler::JR, Recompiler::JALR, Recompiler::BEQ, Recompiler::BNE, Recompiler::BGTZ, Recompiler::BGEZ,
	Recompiler::BLTZ, Recompiler::BLEZ, Recompiler::BGEZAL, Recompiler::BLTZAL, Recompiler::ADD, Recompiler::ADDI, Recompiler::ADDU, Recompiler::ADDIU,
	Recompiler::SUB, Recompiler::SUBU, Recompiler::MULT, Recompiler::MULTU, Recompiler::DIV, Recompiler::DIVU, Recompiler::AND, Recompiler::ANDI,
	Recompiler::OR, Recompiler::ORI, Recompiler::XOR, Recompiler::XORI, Recompiler::NOR, Recompiler::LUI, Recompiler::SLL, Recompiler::SRL,
	Recompiler::SRA, Recompiler::SLLV, Recompiler::SRLV, Recompiler::SRAV, Recompiler::SLT, Recompiler::SLTI, Recompiler::SLTU, Recompiler::SLTIU,
	Recompiler::LB, Recompiler::LBU, Recompiler::LH, Recompiler::LHU, Recompiler::LW, Recompiler::LWL, Recompiler::LWR, Recompiler::SB,
	Recompiler::SH, Recompiler::SW, Recompiler::SWL, Recompiler::SWR, Recompiler::MFHI, Recompiler::MTHI, Recompiler::MFLO, Recompiler::MTLO,
	Recompiler::MFC0, Recompiler::MTC0, Recompiler::CFC2, Recompiler::CTC2, Recompiler::SYSCALL, Recompiler::BREAK,
	
	// instructions on R3000A ONLY
	Recompiler::MFC2, Recompiler::MTC2, Recompiler::LWC2, Recompiler::SWC2, Recompiler::RFE,
	Recompiler::RTPS, Recompiler::RTPT, Recompiler::CC, Recompiler::CDP, Recompiler::DCPL, Recompiler::DPCS, Recompiler::DPCT, Recompiler::NCS,
	Recompiler::NCT, Recompiler::NCDS, Recompiler::NCDT, Recompiler::NCCS, Recompiler::NCCT, Recompiler::GPF, Recompiler::GPL, Recompiler::AVSZ3,
	Recompiler::AVSZ4, Recompiler::SQR, Recompiler::OP, Recompiler::NCLIP, Recompiler::INTPL, Recompiler::MVMVA
};
