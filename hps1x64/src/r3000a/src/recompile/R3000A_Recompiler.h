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


#include "R3000A.h"
#include "R3000A_Lookup.h"
#include "x64Encoder.h"


#ifndef _R3000A_RECOMPILER_H_
#define _R3000A_RECOMPILER_H_

namespace R3000A
{
	// will probably create two recompilers for each processor. one for single stepping and one for multi-stepping
	class Recompiler
	{
	public:
		static Debug::Log debug;
		
		// number of code cache slots total
		// but the number of blocks should be variable
		//static const int c_iNumBlocks = 1024;
		//static const u32 c_iNumBlocks_Mask = c_iNumBlocks - 1;
		u32 NumBlocks;
		static u32 NumBlocks_Mask;
		//static const int c_iBlockSize_Shift = 4;
		//static const int c_iBlockSize_Mask = ( 1 << c_iBlockSize_Shift ) - 1;
		
		static const u32 c_iAddress_Mask = 0x1fffffff;
		
		u32 TotalCodeCache_Size;	// = c_iNumBlocks * ( 1 << c_iBlockSize_Shift );
		
		u32 BlockSize;
		u32 BlockSize_Shift;
		u32 BlockSize_Mask;
		
		// maximum number of instructions that can be encoded/recompiled in a run
		static u32 MaxStep;
		static u32 MaxStep_Shift;
		static u32 MaxStep_Mask;
		
		// zero if not in delay slot, otherwise has the instruction
		//static u32 Local_DelaySlot;
		//static u32 Local_DelayType;
		//static u32 Local_DelayCount;
		//static u32 Local_DelayCond;
		
		// 0 means unconitional branch, 1 means check condition before branching
		//static u32 Local_Condition;
		
		
		// the amount of stack space required for SEH
		static const long c_lSEH_StackSize = 40;
		
		
		union RDelaySlot
		{
			struct
			{
				// instruction in delay slot
				u32 Code;
				
				// type of instruction with delay slot
				// 0 - branch (BEQ,BNE,etc), 1 - jump (J,JAL)
				u16 Type;
				
				// branch condition
				// 0 - unconditional, 1 - conditional
				u16 Condition;
			};
			
			u64 Value;
		};
		
		static RDelaySlot RDelaySlots [ 2 ];
		static u32 DSIndex;
		
		static u32 RDelaySlots_Valid;
		
		inline void ClearDelaySlots ( void ) { RDelaySlots [ 0 ].Value = 0; RDelaySlots [ 1 ].Value = 0; }
		
		// need something to use to keep track of dynamic delay slot stuff
		u32 RecompilerDelaySlot_Flags;
		
		
		// the next instruction in the cache block if there is one
		static Instruction::Format NextInst;
		
		static u64 MemCycles;
		static const u64 ExeCycles = 1;

		static u64 LocalCycleCount;
		static u64 LocalCycleCount2;
		static u64 CacheBlock_CycleCount;
		
		// also need to know if the addresses in the block are cached or not
		static bool bIsBlockInICache;

		
		// the PC while recompiling
		static u32 LocalPC;
		
		// the optimization level
		// 0 means no optimization at all, anything higher means to optimize
		static s32 OpLevel;
		u32 OptimizeLevel;
		
		// the current enabled encoder
		static x64Encoder *e;
		//static ICache_Device *ICache;
		static Cpu *r;
		
		// the encoder for this particular instance
		x64Encoder *InstanceEncoder;
		
		// the maximum number of cache blocks it can encode across
		s32 MaxCacheBlocks;

		static u32 RunCount;
		static u32 RunCount2;
		
		
		// multi-pass optimization vars //
		static u64 ullSrcRegBitmap;
		static u64 ullDstRegBitmap;
		
		// the x64 registers that are currently allocated to MIPS registers
		static u64 ullTargetAlloc;
		
		// the MIPS registers that are currently allocated to x64 registers
		static u64 ullSrcRegAlloc;
		
		// the MIPS registers that are currently allocated to be constants
		static u64 ullSrcConstAlloc;
		
		// the MIPS registers that have been modified and not written back yet
		static u64 ullSrcRegsModified;
		
		// registers that are on the stack and need to be restored when done
		static u64 ullRegsOnStack;
		
		// registers that are needed later on in the code
		static u64 ullNeededLater;
		
		static u64 ullSrcRegBitmaps [ 16 ];
		static u64 ullDstRegBitmaps [ 16 ];
		static u64 ullRegsStillNeeded [ 16 ];
		
		// the actual data stored for the register, either a reg index or a constant value
		static u64 ullTargetData [ 32 ];
		
		// lookup that indicates what register each target index corresponds to
		static const int iRegPriority [ 13 ];
		
		// lookup that indicates if the register requires you to save it on the stack before using it
		// 0: no need to save on stack, 1: save and restore on stack
		static const int iRegStackSave [ 13 ];
		
		// returns false on error, returns true otherwise
		//static bool Remove_SrcReg ( int iSrcRegIdx );
		
		static bool isAlloc ( int iSrcRegIdx );
		static bool isConst ( int iSrcRegIdx );
		static bool isReg ( int iSrcRegIdx );
		static bool isDisposable( int iSrcRegIdx );
		
		static bool isBothAlloc ( int iSrcRegIdx1, int iSrcRegIdx2 );
		static bool isBothConst ( int iSrcRegIdx1, int iSrcRegIdx2 );
		static bool isBothReg ( int iSrcRegIdx1, int iSrcRegIdx2 );
		static bool isBothDisposable( int iSrcRegIdx1, int iSrcRegIdx2 );

		static bool isEitherAlloc ( int iSrcRegIdx1, int iSrcRegIdx2 );
		static bool isEitherConst ( int iSrcRegIdx1, int iSrcRegIdx2 );
		static bool isEitherReg ( int iSrcRegIdx1, int iSrcRegIdx2 );
		static bool isEitherDisposable( int iSrcRegIdx1, int iSrcRegIdx2 );

		static int SelectAlloc ( int iSrcRegIdx1, int iSrcRegIdx2 );
		static int SelectConst ( int iSrcRegIdx1, int iSrcRegIdx2 );
		static int SelectReg ( int iSrcRegIdx1, int iSrcRegIdx2 );
		static int SelectDisposable( int iSrcRegIdx1, int iSrcRegIdx2 );

		static int SelectNotAlloc ( int iSrcRegIdx1, int iSrcRegIdx2 );
		static int SelectNotDisposable( int iSrcRegIdx1, int iSrcRegIdx2 );
		
		static inline u64 GetConst ( int iSrcRegIdx ) { return ullTargetData [ iSrcRegIdx ]; }
		
		// gets the register on the target device using the register on the source device
		static inline int GetReg ( int iSrcRegIdx ) { return iRegPriority [ ullTargetData [ iSrcRegIdx ] ]; }
		
		// returns -1 if error, otherwise returns index of register on target platform
		// iSrcRegIdx: index of register on source platform (for example, MIPS)
		static int Alloc_SrcReg ( int iSrcRegIdx );
		static int Alloc_DstReg ( int iSrcRegIdx );

		// returns -1 if error, otherwise returns index of register on target platform
		// iSrcRegIdx: index of register on source platform (for example, MIPS)
		static int Alloc_Const ( int iSrcRegIdx, u64 ullValue );
		
		static int DisposeReg ( int iSrcRegIdx );
		static int RenameReg ( int iNewSrcRegIdx, int iOldSrcRegIdx );
		static void WriteBackModifiedRegs ();
		static void RestoreRegsFromStack ();

		
		inline void Set_MaxCacheBlocks ( s32 Value ) { MaxCacheBlocks = Value; }
		
		// constructor
		// block size must be power of two, multiplier shift value
		// so for BlockSize_PowerOfTwo, for a block size of 4 pass 2, block size of 8 pass 3, etc
		// for MaxStep, use 0 for single stepping, 1 for stepping until end of 1 cache block, 2 for stepping until end of 2 cache blocks, etc
		// no, for MaxStep, it should be the maximum number of instructions to step
		// NumberOfBlocks MUST be a power of 2, where 1 means 2, 2 means 4, etc
		Recompiler ( Cpu* R3000ACpu, u32 NumberOfBlocks, u32 BlockSize_PowerOfTwo, u32 MaxIStep );
		
		// destructor
		~Recompiler ();
		
		
		void Reset ();	// { memset ( this, 0, sizeof( Recompiler ) ); }

		
		static bool isBranchDelayOk ( u32 ulInstruction, u32 Address );

		
		static u32* RGetPointer ( u32 Address );
		
		
		// set the optimization level
		inline void SetOptimizationLevel ( u32 Level ) { OptimizeLevel = Level; }
		
		// accessors
		//inline u32 Get_DoNotCache ( u32 Address ) { return DoNotCache [ ( Address >> 2 ) & NumBlocks_Mask ]; }
		//inline u32 Get_CacheMissCount ( u32 Address ) { return CacheMissCount [ ( Address >> 2 ) & NumBlocks_Mask ]; }
		//inline u32 Get_StartAddress ( u32 Address ) { return StartAddress [ ( Address >> 2 ) & NumBlocks_Mask ]; }
		//inline u32 Get_LastAddress ( u32 Address ) { return LastAddress [ ( Address >> 2 ) & NumBlocks_Mask ]; }
		//inline u32 Get_RunCount ( u32 Address ) { return RunCount [ ( Address >> 2 ) & NumBlocks_Mask ]; }
		//inline u32 Get_MaxCycles ( u32 Address ) { return MaxCycles [ ( Address >> 2 ) & NumBlocks_Mask ]; }
		
		// returns a pointer to the instructions cached starting at Address
		// this function assumes that this instance of recompiler only has one instruction per run
		//inline u32* Get_InstructionsPtr ( u32 Address ) { return & ( Instructions [ ( ( Address >> 2 ) & NumBlocks_Mask ) << MaxStep_Shift ] ); }
		
		// returns a pointer to the start addresses cached starting at Address
		// this function assumes that this instance of recompiler only has one instruction per run
		//inline u32* Get_AddressPtr ( u32 Address ) { return & ( StartAddress [ ( ( Address >> 2 ) & NumBlocks_Mask ) ] ); }
		
		// check that Address is not -1
		// returns 1 if address is ok, returns 0 if it is not ok
		//inline bool isBlockValid ( u32 Address ) { return ( StartAddress [ ( Address >> 2 ) & NumBlocks_Mask ] != 0xffffffff ); }
		
		// check that Address matches StartAddress for the block
		// returns 1 if address is ok, returns 0 if it is not ok
		//inline bool isStartAddressMatch ( u32 Address ) { return ( Address == StartAddress [ ( Address >> 2 ) & NumBlocks_Mask ] ); }
		
		// this function assumes that this instance of recompiler only has one instruction per run
		// returns 1 if the instruction is cached for address, 0 otherwise
		//inline bool isCached ( u32 Address, u32 Instruction ) { return ( Address == StartAddress [ ( Address >> 2 ) & NumBlocks_Mask ] ) && ( Instruction == Instructions [ ( Address >> 2 ) & NumBlocks_Mask ] ); }
		
		inline bool isRecompiled ( u32 Address ) { return ( StartAddress [ ( Address >> ( 2 + MaxStep_Shift ) ) & NumBlocks_Mask ] == ( Address & ~( ( 1 << ( 2 + MaxStep_Shift ) ) - 1 ) ) ); }
		
		
		inline u64 Execute ( u32 Address ) { return InstanceEncoder->ExecuteCodeBlock ( ( Address >> 2 ) & NumBlocks_Mask ); }
		
		
		// initializes a code block that is not being used yet
		u32 InitBlock ( u32 Block );
		
		
		// get a bitmap for the source registers used by the specified instruction
		static u64 GetSourceRegs ( Instruction::Format i, u32 Address );
		static u64 Get_DelaySlot_DestRegs ( Instruction::Format i );
		
		static long Generate_Normal_Store ( Instruction::Format i, u32 Address, u32 BitTest, void* StoreFunctionToCall );
		static long Generate_Normal_Load ( Instruction::Format i, u32 Address, u32 BitTest, void* LoadFunctionToCall, void* LoadFunctionImmediate );
		static long Generate_Normal_Branch ( Instruction::Format i, u32 Address, void* BranchFunctionToCall );
		
		static long Generate_Normal_Store_L2 ( Instruction::Format i, u32 Address, u32 BitTest, u32 BaseAddress );
		
		
		// says if block points to an R3000A instruction that should NOT be EVER cached
		// *note* this should be dynamically allocated
		//u8* DoNotCache;	// [ c_iNumBlocks ];
		
		// number of times a cache miss was encountered while recompiling for this block
		// *note* this should be dynamically allocated
		//u32* CacheMissCount;	// [ c_iNumBlocks ];
		
		// code cache block not invalidated
		// *note* this should be dynamically allocated
		//u8 isValid [ c_iNumBlocks ];
		
		// start address for instruction encodings (inclusive)
		// *note* this should be dynamically allocated
		static u32* StartAddress;	// [ c_iNumBlocks ];
		
		
		// last address that instruction encoding is valid for (inclusive)
		// *note* this should be dynamically allocated
		// It actually makes things MUCH simpler if you store the number of instructions recompiled instead of the last address
		//u32* LastAddress;	// [ c_iNumBlocks ];
		//u8* RunCount;
		
		//u32* Instructions;
		
		// pointer to where the prefix for the instruction starts at (used only internally by recompiler)
		static u8** pPrefix_CodeStart;
		
		// where the actual instruction starts at in code block
		static u8** pCodeStart;
		
		// the number of offset cycles from this instruction in the code block
		static u32* CycleCount;
		
		
		// list of branch targets when jumping forwards in code block
		static u32* pForwardBranchTargets;
		static const int c_ulForwardBranchIndex_Start = 5;
		static u32 ForwardBranchIndex;
		
		static u32 StartBlockIndex;
		static u32 BlockIndex;
		
		// not needed
		//u32* EndAddress;
		
		
		long Generate_Prefix_EventCheck ( u32 Address, bool bIsBranchOrJump );
		
		
		
		// need to know what address current block starts at
		static u32 CurrentBlock_StartAddress;
		
		// also need to know what address the next block starts at
		static u32 NextBlock_StartAddress;
		
		
		// max number of cycles that instruction encoding could use up if executed
		// need to know this to single step when there are interrupts in between
		// code block is not valid when this is zero
		// *note* this should be dynamically allocated
		//u64* MaxCycles;	// [ c_iNumBlocks ];
		
		static u32 Local_LastModifiedReg;
		static u32 Local_NextPCModified;
		
		static u32 CurrentCount;
		
		static u32 isBranchDelaySlot;
		static u32 isLoadDelaySlot;
		
		static u32 bStopEncodingAfter;
		static u32 bStopEncodingBefore;
		
		// set the local cycle count to reset (start from zero) for the next cycle
		static u32 bResetCycleCount;
		
		inline void ResetFlags ( void ) { bStopEncodingBefore = false; bStopEncodingAfter = false; Local_NextPCModified = false; }

		
		
		// recompiler function
		// returns -1 if the instruction cannot be recompiled, otherwise returns the maximum number of cycles the instruction uses
		typedef long (*Function) ( Instruction::Format Instruction, u32 Address );

		// *** todo *** do not recompile more than one instruction if currently in a branch delay slot or load delay slot!!
		u32 Recompile ( u32 StartAddress );
		u32 Recompile2 ( u32 StartAddress );
		//void Invalidate ( u32 Address );
		void Invalidate ( u32 Address, u32 Count );
		
		u32 CloseOpLevel ( u32 OptLevel, u32 Address );
		
		
		static u32 ulIndex_Mask;
		
		inline u32 Get_Block ( u32 Address ) { return ( Address >> ( 2 + MaxStep_Shift ) ) & NumBlocks_Mask; }
		inline u32 Get_Index ( u32 Address ) { return ( Address >> 2 ) & ulIndex_Mask; }
		
		//void Recompile ( u32 Instruction );
		
			static const Function FunctionList [];
			
		// used by object to recompile an instruction into a code block
		// returns -1 if the instruction cannot be recompiled
		// returns 0 if the instruction was recompiled, but MUST start a new block for the next instruction (because it is guaranteed in a delay slot)
		// returns 1 if successful and can continue recompiling
		inline static long Recompile ( Instruction::Format i, u32 Address ) { return FunctionList [ Instruction::Lookup::FindByInstruction ( i.Value ) ] ( i, Address ); }
		
		
		inline void Run ( u32 Address ) { InstanceEncoder->ExecuteCodeBlock ( ( Address >> 2 ) & NumBlocks_Mask ); }

			static long Invalid ( Instruction::Format i, u32 Address );

			static long J ( Instruction::Format i, u32 Address );
			static long JAL ( Instruction::Format i, u32 Address );
			static long BEQ ( Instruction::Format i, u32 Address );
			static long BNE ( Instruction::Format i, u32 Address );
			static long BLEZ ( Instruction::Format i, u32 Address );
			static long BGTZ ( Instruction::Format i, u32 Address );
			static long ADDI ( Instruction::Format i, u32 Address );
			static long ADDIU ( Instruction::Format i, u32 Address );
			static long SLTI ( Instruction::Format i, u32 Address );
			static long SLTIU ( Instruction::Format i, u32 Address );
			static long ANDI ( Instruction::Format i, u32 Address );
			static long ORI ( Instruction::Format i, u32 Address );
			static long XORI ( Instruction::Format i, u32 Address );
			static long LUI ( Instruction::Format i, u32 Address );
			static long LB ( Instruction::Format i, u32 Address );
			static long LH ( Instruction::Format i, u32 Address );
			static long LWL ( Instruction::Format i, u32 Address );
			static long LW ( Instruction::Format i, u32 Address );
			static long LBU ( Instruction::Format i, u32 Address );
			static long LHU ( Instruction::Format i, u32 Address );
			
			static long LWR ( Instruction::Format i, u32 Address );
			static long SB ( Instruction::Format i, u32 Address );
			static long SH ( Instruction::Format i, u32 Address );
			static long SWL ( Instruction::Format i, u32 Address );
			static long SW ( Instruction::Format i, u32 Address );
			static long SWR ( Instruction::Format i, u32 Address );
			static long LWC2 ( Instruction::Format i, u32 Address );
			static long SWC2 ( Instruction::Format i, u32 Address );
			static long SLL ( Instruction::Format i, u32 Address );
			static long SRL ( Instruction::Format i, u32 Address );
			static long SRA ( Instruction::Format i, u32 Address );
			static long SLLV ( Instruction::Format i, u32 Address );
			static long SRLV ( Instruction::Format i, u32 Address );
			static long SRAV ( Instruction::Format i, u32 Address );
			static long JR ( Instruction::Format i, u32 Address );
			static long JALR ( Instruction::Format i, u32 Address );
			static long SYSCALL ( Instruction::Format i, u32 Address );
			static long BREAK ( Instruction::Format i, u32 Address );
			static long MFHI ( Instruction::Format i, u32 Address );
			static long MTHI ( Instruction::Format i, u32 Address );

			static long MFLO ( Instruction::Format i, u32 Address );
			static long MTLO ( Instruction::Format i, u32 Address );
			static long MULT ( Instruction::Format i, u32 Address );
			static long MULTU ( Instruction::Format i, u32 Address );
			static long DIV ( Instruction::Format i, u32 Address );
			static long DIVU ( Instruction::Format i, u32 Address );
			static long ADD ( Instruction::Format i, u32 Address );
			static long ADDU ( Instruction::Format i, u32 Address );
			static long SUB ( Instruction::Format i, u32 Address );
			static long SUBU ( Instruction::Format i, u32 Address );
			static long AND ( Instruction::Format i, u32 Address );
			static long OR ( Instruction::Format i, u32 Address );
			static long XOR ( Instruction::Format i, u32 Address );
			static long NOR ( Instruction::Format i, u32 Address );
			static long SLT ( Instruction::Format i, u32 Address );
			static long SLTU ( Instruction::Format i, u32 Address );
			static long BLTZ ( Instruction::Format i, u32 Address );
			static long BGEZ ( Instruction::Format i, u32 Address );
			static long BLTZAL ( Instruction::Format i, u32 Address );
			static long BGEZAL ( Instruction::Format i, u32 Address );

			static long MFC0 ( Instruction::Format i, u32 Address );
			static long MTC0 ( Instruction::Format i, u32 Address );
			static long RFE ( Instruction::Format i, u32 Address );
			static long MFC2 ( Instruction::Format i, u32 Address );
			static long CFC2 ( Instruction::Format i, u32 Address );
			static long MTC2 ( Instruction::Format i, u32 Address );
			static long CTC2 ( Instruction::Format i, u32 Address );
			static long COP2 ( Instruction::Format i, u32 Address );
			
			static long RTPS ( Instruction::Format i, u32 Address );
			static long NCLIP ( Instruction::Format i, u32 Address );
			static long OP ( Instruction::Format i, u32 Address );
			static long DPCS ( Instruction::Format i, u32 Address );
			static long INTPL ( Instruction::Format i, u32 Address );
			static long MVMVA ( Instruction::Format i, u32 Address );
			static long NCDS ( Instruction::Format i, u32 Address );
			static long CDP ( Instruction::Format i, u32 Address );
			static long NCDT ( Instruction::Format i, u32 Address );
			static long NCCS ( Instruction::Format i, u32 Address );
			static long CC ( Instruction::Format i, u32 Address );
			static long NCS ( Instruction::Format i, u32 Address );
			static long NCT ( Instruction::Format i, u32 Address );
			static long SQR ( Instruction::Format i, u32 Address );
			static long DCPL ( Instruction::Format i, u32 Address );
			static long DPCT ( Instruction::Format i, u32 Address );
			static long AVSZ3 ( Instruction::Format i, u32 Address );
			static long AVSZ4 ( Instruction::Format i, u32 Address );
			static long RTPT ( Instruction::Format i, u32 Address );
			static long GPF ( Instruction::Format i, u32 Address );
			static long GPL ( Instruction::Format i, u32 Address );
			static long NCCT ( Instruction::Format i, u32 Address );

	};
};

#endif
