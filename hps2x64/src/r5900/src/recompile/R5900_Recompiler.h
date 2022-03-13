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


#include "R5900.h"
#include "R5900_Lookup.h"
#include "x64Encoder.h"


#ifndef _R5900_RECOMPILER_H_
#define _R5900_RECOMPILER_H_

namespace R5900
{
	
	class Cpu;
	
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
		static R5900::Instruction::Format NextInst;
		
		// cycles to load instruction from memory/cache
		static u64 MemCycles;
		
		// cycles to start execution of instruction (should be 1)
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
		static R5900::Cpu *r;
		
		// the encoder for this particular instance
		x64Encoder *InstanceEncoder;
		
		// the maximum number of cache blocks it can encode across
		s32 MaxCacheBlocks;

		static u32 RunCount;
		static u32 RunCount2;

		// a bitmap that shows the usable registers when allocating registers in level 2 compiler
		// set bits need to start from position 0 and need to be contiguous. 0x1f is ok but not 0x12 for example
		static const u32 c_ulUsableRegsBitmap = 0x1fff;	//0x1fff;
		
		// keep of what registers might be in load/store pipeline
		static u64 ullLSRegs;
		
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
		static bool isLarge ( u64 ullValue );
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
		
		
		static bool Check_StaticDependencyOk ( R5900::Instruction::Format i );
		static unsigned long long GetGPR_SrcRegs ( R5900::Instruction::Format i );
		static unsigned long long GetCop1_SrcRegs ( R5900::Instruction::Format i0 );

		
		inline void Set_MaxCacheBlocks ( s32 Value ) { MaxCacheBlocks = Value; }
		
		// constructor
		// block size must be power of two, multiplier shift value
		// so for BlockSize_PowerOfTwo, for a block size of 4 pass 2, block size of 8 pass 3, etc
		// for MaxStep, use 0 for single stepping, 1 for stepping until end of 1 cache block, 2 for stepping until end of 2 cache blocks, etc
		// no, for MaxStep, it should be the maximum number of instructions to step
		// NumberOfBlocks MUST be a power of 2, where 1 means 2, 2 means 4, etc
		Recompiler ( R5900::Cpu* R5900Cpu, u32 NumberOfBlocks, u32 BlockSize_PowerOfTwo, u32 MaxIStep );
		
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
		static u64 GetSourceRegs ( R5900::Instruction::Format i, u32 Address );
		static u64 Get_DelaySlot_DestRegs ( R5900::Instruction::Format i );
		
		static long Generate_Normal_Store ( R5900::Instruction::Format i, u32 Address, u32 BitTest, void* StoreFunctionToCall );
		static long Generate_Normal_Store_L2 ( Instruction::Format i, u32 Address, u32 BitTest, u32 BaseAddress );
		static long Generate_Normal_Load ( R5900::Instruction::Format i, u32 Address, u32 BitTest, void* LoadFunctionToCall );
		static long Generate_Normal_Load_L2 ( Instruction::Format i, u32 Address, u32 BitTest, u32 BaseAddress );
		static long Generate_Normal_Branch ( R5900::Instruction::Format i, u32 Address, void* BranchFunctionToCall );
		static long Generate_Normal_Trap ( R5900::Instruction::Format i, u32 Address );

		// combined load/store
		static int Get_CombinedLoadCount ( R5900::Instruction::Format i, long Address, R5900::Instruction::Format* pInstructionList );
		static long Get_CombinedLoadMaxWidthMask ( int iLoadCount, R5900::Instruction::Format* pLoadList );
		static long Generate_Combined_Load ( R5900::Instruction::Format i, u32 Address, u32 BitTest, void* LoadFunctionToCall, int iLoadCount, R5900::Instruction::Format* pLoadList );

		static int Get_CombinedStoreCount ( R5900::Instruction::Format i, long Address, R5900::Instruction::Format* pInstructionList );
		static long Get_CombinedStoreMaxWidthMask ( int iLoadCount, R5900::Instruction::Format* pLoadList );
		static long Generate_Combined_Store ( R5900::Instruction::Format i, u32 Address, u32 BitTest, void* StoreFunctionToCall, int iLoadCount, R5900::Instruction::Format* pLoadList );

		static long Generate_VABSp ( R5900::Instruction::Format i );
		static long Generate_VMAXp ( R5900::Instruction::Format i, u32 *pFt = NULL, u32 FtComponent = 4 );
		static long Generate_VMINp ( R5900::Instruction::Format i, u32 *pFt = NULL, u32 FtComponent = 4 );
		static long Generate_VFTOIXp ( R5900::Instruction::Format i, u32 IX );
		static long Generate_VITOFXp ( R5900::Instruction::Format i, u64 FX );
		static long Generate_VMOVEp ( R5900::Instruction::Format i );
		static long Generate_VMR32p ( R5900::Instruction::Format i );
		static long Generate_VMFIRp ( R5900::Instruction::Format i );
		static long Generate_VMTIRp ( R5900::Instruction::Format i );
		static long Generate_VADDp ( u32 bSub, R5900::Instruction::Format i, u32 FtComponent = 4, void *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMULp ( R5900::Instruction::Format i, u32 FtComponentp = 0x1b, void *pFd = NULL, u32 *pFt = NULL, u32 FsComponentp = 0x1b );
		static long Generate_VMADDp ( u32 bSub, R5900::Instruction::Format i, u32 FtComponentp = 0x1b, void *pFd = NULL, u32 *pFt = NULL, u32 FsComponentp = 0x1b );
		
		
		static long Generate_VABS ( R5900::Instruction::Format i, u32 Address, u32 Component );
		static long Generate_VMAX ( R5900::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFt = NULL );
		static long Generate_VMIN ( R5900::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFt = NULL );
		static long Generate_VFTOI0 ( R5900::Instruction::Format i, u32 Address, u32 FtComponent, u32 FsComponent );
		static long Generate_VFTOIX ( R5900::Instruction::Format i, u32 Address, u32 FtComponent, u32 FsComponent, u32 IX );
		static long Generate_VITOFX ( R5900::Instruction::Format i, u32 Address, u32 FtComponent, u32 FsComponent, u64 FX );
		static long Generate_VMOVE ( R5900::Instruction::Format i, u32 Address, u32 FtComponent, u32 FsComponent );
		
		static long Generate_VMR32_Load ( R5900::Instruction::Format i, u32 Address, u32 FsComponent );
		static long Generate_VMR32_Store ( R5900::Instruction::Format i, u32 Address, u32 FtComponent );
		
		static long Generate_VMFIR ( R5900::Instruction::Format i, u32 Address, u32 FtComponent );
		static long Generate_VADD ( R5900::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VSUB ( R5900::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMUL ( R5900::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMADD ( R5900::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMSUB ( R5900::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		
		
		static long Generate_VPrefix ( u32 Address );
		
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
		static u32* LastOffset;
		
		
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

#ifdef ENABLE_R5900_CHECKSUM
		// 64-bit checksum of the source - final check to determine if recompile is really needed
		static u64* pChecksum64;
#endif
		
		u64 Calc_Checksum( u32 StartAddress );


		
		// list of branch targets when jumping forwards in code block
		static u32* pForwardBranchTargets;
		static const int c_ulForwardBranchIndex_Start = 100;
		static u32 ForwardBranchIndex;

		static void FJMP ( long AddressFrom, long AddressTo );
		static void Set_FJMPs ( long AddressTo );



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
		typedef long (*Function) ( R5900::Instruction::Format Instruction, u32 Address );

		// *** todo *** do not recompile more than one instruction if currently in a branch delay slot or load delay slot!!
		u32 Recompile ( u32 StartAddress );
		u32 Recompile2 ( u32 ulBeginAddress );
		
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
		inline static long Recompile ( R5900::Instruction::Format i, u32 Address ) { return R5900::Recompiler::FunctionList [ R5900::Instruction::Lookup::FindByInstruction ( i.Value ) ] ( i, Address ); }
		
		
		inline void Run ( u32 Address ) { InstanceEncoder->ExecuteCodeBlock ( ( Address >> 2 ) & NumBlocks_Mask ); }

			static long Invalid ( R5900::Instruction::Format i, u32 Address );

			static long J ( R5900::Instruction::Format i, u32 Address );
			static long JAL ( R5900::Instruction::Format i, u32 Address );
			static long BEQ ( R5900::Instruction::Format i, u32 Address );
			static long BNE ( R5900::Instruction::Format i, u32 Address );
			static long BLEZ ( R5900::Instruction::Format i, u32 Address );
			static long BGTZ ( R5900::Instruction::Format i, u32 Address );
			static long ADDI ( R5900::Instruction::Format i, u32 Address );
			static long ADDIU ( R5900::Instruction::Format i, u32 Address );
			static long SLTI ( R5900::Instruction::Format i, u32 Address );
			static long SLTIU ( R5900::Instruction::Format i, u32 Address );
			static long ANDI ( R5900::Instruction::Format i, u32 Address );
			static long ORI ( R5900::Instruction::Format i, u32 Address );
			static long XORI ( R5900::Instruction::Format i, u32 Address );
			static long LUI ( R5900::Instruction::Format i, u32 Address );
			static long LB ( R5900::Instruction::Format i, u32 Address );
			static long LH ( R5900::Instruction::Format i, u32 Address );
			static long LWL ( R5900::Instruction::Format i, u32 Address );
			static long LW ( R5900::Instruction::Format i, u32 Address );
			static long LBU ( R5900::Instruction::Format i, u32 Address );
			static long LHU ( R5900::Instruction::Format i, u32 Address );
			
			static long LWR ( R5900::Instruction::Format i, u32 Address );
			static long SB ( R5900::Instruction::Format i, u32 Address );
			static long SH ( R5900::Instruction::Format i, u32 Address );
			static long SWL ( R5900::Instruction::Format i, u32 Address );
			static long SW ( R5900::Instruction::Format i, u32 Address );
			static long SWR ( R5900::Instruction::Format i, u32 Address );
			static long SLL ( R5900::Instruction::Format i, u32 Address );
			static long SRL ( R5900::Instruction::Format i, u32 Address );
			static long SRA ( R5900::Instruction::Format i, u32 Address );
			static long SLLV ( R5900::Instruction::Format i, u32 Address );
			static long SRLV ( R5900::Instruction::Format i, u32 Address );
			static long SRAV ( R5900::Instruction::Format i, u32 Address );
			static long JR ( R5900::Instruction::Format i, u32 Address );
			static long JALR ( R5900::Instruction::Format i, u32 Address );
			static long SYSCALL ( R5900::Instruction::Format i, u32 Address );
			static long BREAK ( R5900::Instruction::Format i, u32 Address );
			static long MFHI ( R5900::Instruction::Format i, u32 Address );
			static long MTHI ( R5900::Instruction::Format i, u32 Address );

			static long MFLO ( R5900::Instruction::Format i, u32 Address );
			static long MTLO ( R5900::Instruction::Format i, u32 Address );
			static long MULT ( R5900::Instruction::Format i, u32 Address );
			static long MULTU ( R5900::Instruction::Format i, u32 Address );
			static long DIV ( R5900::Instruction::Format i, u32 Address );
			static long DIVU ( R5900::Instruction::Format i, u32 Address );
			static long ADD ( R5900::Instruction::Format i, u32 Address );
			static long ADDU ( R5900::Instruction::Format i, u32 Address );
			static long SUB ( R5900::Instruction::Format i, u32 Address );
			static long SUBU ( R5900::Instruction::Format i, u32 Address );
			static long AND ( R5900::Instruction::Format i, u32 Address );
			static long OR ( R5900::Instruction::Format i, u32 Address );
			static long XOR ( R5900::Instruction::Format i, u32 Address );
			static long NOR ( R5900::Instruction::Format i, u32 Address );
			static long SLT ( R5900::Instruction::Format i, u32 Address );
			static long SLTU ( R5900::Instruction::Format i, u32 Address );
			static long BLTZ ( R5900::Instruction::Format i, u32 Address );
			static long BGEZ ( R5900::Instruction::Format i, u32 Address );
			static long BLTZAL ( R5900::Instruction::Format i, u32 Address );
			static long BGEZAL ( R5900::Instruction::Format i, u32 Address );

			static long MFC0 ( R5900::Instruction::Format i, u32 Address );
			static long MTC0 ( R5900::Instruction::Format i, u32 Address );
			
			static long CFC2_I ( R5900::Instruction::Format i, u32 Address );
			static long CTC2_I ( R5900::Instruction::Format i, u32 Address );
			static long CFC2_NI ( R5900::Instruction::Format i, u32 Address );
			static long CTC2_NI ( R5900::Instruction::Format i, u32 Address );
			
			/*
			static long MFC2 ( R5900::Instruction::Format i, u32 Address );
			static long MTC2 ( R5900::Instruction::Format i, u32 Address );
			static long LWC2 ( R5900::Instruction::Format i, u32 Address );
			static long SWC2 ( R5900::Instruction::Format i, u32 Address );
			static long RFE ( R5900::Instruction::Format i, u32 Address );
			static long RTPS ( R5900::Instruction::Format i, u32 Address );
			static long NCLIP ( R5900::Instruction::Format i, u32 Address );
			static long OP ( R5900::Instruction::Format i, u32 Address );
			static long DPCS ( R5900::Instruction::Format i, u32 Address );
			static long INTPL ( R5900::Instruction::Format i, u32 Address );
			static long MVMVA ( R5900::Instruction::Format i, u32 Address );
			static long NCDS ( R5900::Instruction::Format i, u32 Address );
			static long CDP ( R5900::Instruction::Format i, u32 Address );
			static long NCDT ( R5900::Instruction::Format i, u32 Address );
			static long NCCS ( R5900::Instruction::Format i, u32 Address );
			static long CC ( R5900::Instruction::Format i, u32 Address );
			static long NCS ( R5900::Instruction::Format i, u32 Address );
			static long NCT ( R5900::Instruction::Format i, u32 Address );
			static long SQR ( R5900::Instruction::Format i, u32 Address );
			static long DCPL ( R5900::Instruction::Format i, u32 Address );
			static long DPCT ( R5900::Instruction::Format i, u32 Address );
			static long AVSZ3 ( R5900::Instruction::Format i, u32 Address );
			static long AVSZ4 ( R5900::Instruction::Format i, u32 Address );
			static long RTPT ( R5900::Instruction::Format i, u32 Address );
			static long GPF ( R5900::Instruction::Format i, u32 Address );
			static long GPL ( R5900::Instruction::Format i, u32 Address );
			static long NCCT ( R5900::Instruction::Format i, u32 Address );
			*/

			static long COP2 ( R5900::Instruction::Format i, u32 Address );
			
			
			// *** R5900 Instructions *** //
			
			static long BC0T ( R5900::Instruction::Format i, u32 Address );
			static long BC0TL ( R5900::Instruction::Format i, u32 Address );
			static long BC0F ( R5900::Instruction::Format i, u32 Address );
			static long BC0FL ( R5900::Instruction::Format i, u32 Address );
			static long BC1T ( R5900::Instruction::Format i, u32 Address );
			static long BC1TL ( R5900::Instruction::Format i, u32 Address );
			static long BC1F ( R5900::Instruction::Format i, u32 Address );
			static long BC1FL ( R5900::Instruction::Format i, u32 Address );
			static long BC2T ( R5900::Instruction::Format i, u32 Address );
			static long BC2TL ( R5900::Instruction::Format i, u32 Address );
			static long BC2F ( R5900::Instruction::Format i, u32 Address );
			static long BC2FL ( R5900::Instruction::Format i, u32 Address );
			

			static long CFC0 ( R5900::Instruction::Format i, u32 Address );
			static long CTC0 ( R5900::Instruction::Format i, u32 Address );
			static long EI ( R5900::Instruction::Format i, u32 Address );
			static long DI ( R5900::Instruction::Format i, u32 Address );
			
			static long SD ( R5900::Instruction::Format i, u32 Address );
			static long LD ( R5900::Instruction::Format i, u32 Address );
			static long LWU ( R5900::Instruction::Format i, u32 Address );
			static long SDL ( R5900::Instruction::Format i, u32 Address );
			static long SDR ( R5900::Instruction::Format i, u32 Address );
			static long LDL ( R5900::Instruction::Format i, u32 Address );
			static long LDR ( R5900::Instruction::Format i, u32 Address );
			static long LQ ( R5900::Instruction::Format i, u32 Address );
			static long SQ ( R5900::Instruction::Format i, u32 Address );
			
			
			// arithemetic instructions //
			static long DADD ( R5900::Instruction::Format i, u32 Address );
			static long DADDI ( R5900::Instruction::Format i, u32 Address );
			static long DADDU ( R5900::Instruction::Format i, u32 Address );
			static long DADDIU ( R5900::Instruction::Format i, u32 Address );
			static long DSUB ( R5900::Instruction::Format i, u32 Address );
			static long DSUBU ( R5900::Instruction::Format i, u32 Address );
			static long DSLL ( R5900::Instruction::Format i, u32 Address );
			static long DSLL32 ( R5900::Instruction::Format i, u32 Address );
			static long DSLLV ( R5900::Instruction::Format i, u32 Address );
			static long DSRA ( R5900::Instruction::Format i, u32 Address );
			static long DSRA32 ( R5900::Instruction::Format i, u32 Address );
			static long DSRAV ( R5900::Instruction::Format i, u32 Address );
			static long DSRL ( R5900::Instruction::Format i, u32 Address );
			static long DSRL32 ( R5900::Instruction::Format i, u32 Address );
			static long DSRLV ( R5900::Instruction::Format i, u32 Address );
			
			

			static long MFC1 ( R5900::Instruction::Format i, u32 Address );
			static long CFC1 ( R5900::Instruction::Format i, u32 Address );
			static long MTC1 ( R5900::Instruction::Format i, u32 Address );
			static long CTC1 ( R5900::Instruction::Format i, u32 Address );
			
			static long BEQL ( R5900::Instruction::Format i, u32 Address );
			static long BNEL ( R5900::Instruction::Format i, u32 Address );
			static long BGEZL ( R5900::Instruction::Format i, u32 Address );
			static long BLEZL ( R5900::Instruction::Format i, u32 Address );
			static long BGTZL ( R5900::Instruction::Format i, u32 Address );
			static long BLTZL ( R5900::Instruction::Format i, u32 Address );
			static long BLTZALL ( R5900::Instruction::Format i, u32 Address );
			static long BGEZALL ( R5900::Instruction::Format i, u32 Address );
			
			static long CACHE ( R5900::Instruction::Format i, u32 Address );
			static long PREF ( R5900::Instruction::Format i, u32 Address );
			
			static long TGEI ( R5900::Instruction::Format i, u32 Address );
			static long TGEIU ( R5900::Instruction::Format i, u32 Address );
			static long TLTI ( R5900::Instruction::Format i, u32 Address );
			static long TLTIU ( R5900::Instruction::Format i, u32 Address );
			static long TEQI ( R5900::Instruction::Format i, u32 Address );
			static long TNEI ( R5900::Instruction::Format i, u32 Address );
			static long TGE ( R5900::Instruction::Format i, u32 Address );
			static long TGEU ( R5900::Instruction::Format i, u32 Address );
			static long TLT ( R5900::Instruction::Format i, u32 Address );
			static long TLTU ( R5900::Instruction::Format i, u32 Address );
			static long TEQ ( R5900::Instruction::Format i, u32 Address );
			static long TNE ( R5900::Instruction::Format i, u32 Address );
			
			static long MOVCI ( R5900::Instruction::Format i, u32 Address );
			static long MOVZ ( R5900::Instruction::Format i, u32 Address );
			static long MOVN ( R5900::Instruction::Format i, u32 Address );
			static long SYNC ( R5900::Instruction::Format i, u32 Address );
			
			static long MFHI1 ( R5900::Instruction::Format i, u32 Address );
			static long MTHI1 ( R5900::Instruction::Format i, u32 Address );
			static long MFLO1 ( R5900::Instruction::Format i, u32 Address );
			static long MTLO1 ( R5900::Instruction::Format i, u32 Address );
			static long MULT1 ( R5900::Instruction::Format i, u32 Address );
			static long MULTU1 ( R5900::Instruction::Format i, u32 Address );
			static long DIV1 ( R5900::Instruction::Format i, u32 Address );
			static long DIVU1 ( R5900::Instruction::Format i, u32 Address );
			static long MADD ( R5900::Instruction::Format i, u32 Address );
			static long MADD1 ( R5900::Instruction::Format i, u32 Address );
			static long MADDU ( R5900::Instruction::Format i, u32 Address );
			static long MADDU1 ( R5900::Instruction::Format i, u32 Address );
			
			static long MFSA ( R5900::Instruction::Format i, u32 Address );
			static long MTSA ( R5900::Instruction::Format i, u32 Address );
			static long MTSAB ( R5900::Instruction::Format i, u32 Address );
			static long MTSAH ( R5900::Instruction::Format i, u32 Address );
			
			static long TLBR ( R5900::Instruction::Format i, u32 Address );
			static long TLBWI ( R5900::Instruction::Format i, u32 Address );
			static long TLBWR ( R5900::Instruction::Format i, u32 Address );
			static long TLBP ( R5900::Instruction::Format i, u32 Address );
			
			static long ERET ( R5900::Instruction::Format i, u32 Address );
			static long DERET ( R5900::Instruction::Format i, u32 Address );
			static long WAIT ( R5900::Instruction::Format i, u32 Address );
			
			
			// Parallel instructions (SIMD) //
			static long PABSH ( R5900::Instruction::Format i, u32 Address );
			static long PABSW ( R5900::Instruction::Format i, u32 Address );
			static long PADDB ( R5900::Instruction::Format i, u32 Address );
			static long PADDH ( R5900::Instruction::Format i, u32 Address );
			static long PADDW ( R5900::Instruction::Format i, u32 Address );
			static long PADDSB ( R5900::Instruction::Format i, u32 Address );
			static long PADDSH ( R5900::Instruction::Format i, u32 Address );
			static long PADDSW ( R5900::Instruction::Format i, u32 Address );
			static long PADDUB ( R5900::Instruction::Format i, u32 Address );
			static long PADDUH ( R5900::Instruction::Format i, u32 Address );
			static long PADDUW ( R5900::Instruction::Format i, u32 Address );
			static long PADSBH ( R5900::Instruction::Format i, u32 Address );
			static long PAND ( R5900::Instruction::Format i, u32 Address );
			static long POR ( R5900::Instruction::Format i, u32 Address );
			static long PXOR ( R5900::Instruction::Format i, u32 Address );
			static long PNOR ( R5900::Instruction::Format i, u32 Address );
			static long PCEQB ( R5900::Instruction::Format i, u32 Address );
			static long PCEQH ( R5900::Instruction::Format i, u32 Address );
			static long PCEQW ( R5900::Instruction::Format i, u32 Address );
			static long PCGTB ( R5900::Instruction::Format i, u32 Address );
			static long PCGTH ( R5900::Instruction::Format i, u32 Address );
			static long PCGTW ( R5900::Instruction::Format i, u32 Address );
			static long PCPYH ( R5900::Instruction::Format i, u32 Address );
			static long PCPYLD ( R5900::Instruction::Format i, u32 Address );
			static long PCPYUD ( R5900::Instruction::Format i, u32 Address );
			static long PDIVBW ( R5900::Instruction::Format i, u32 Address );
			static long PDIVUW ( R5900::Instruction::Format i, u32 Address );
			static long PDIVW ( R5900::Instruction::Format i, u32 Address );
			static long PEXCH ( R5900::Instruction::Format i, u32 Address );
			static long PEXCW ( R5900::Instruction::Format i, u32 Address );
			static long PEXEH ( R5900::Instruction::Format i, u32 Address );
			static long PEXEW ( R5900::Instruction::Format i, u32 Address );
			static long PEXT5 ( R5900::Instruction::Format i, u32 Address );
			static long PEXTLB ( R5900::Instruction::Format i, u32 Address );
			static long PEXTLH ( R5900::Instruction::Format i, u32 Address );
			static long PEXTLW ( R5900::Instruction::Format i, u32 Address );
			static long PEXTUB ( R5900::Instruction::Format i, u32 Address );
			static long PEXTUH ( R5900::Instruction::Format i, u32 Address );
			static long PEXTUW ( R5900::Instruction::Format i, u32 Address );
			static long PHMADH ( R5900::Instruction::Format i, u32 Address );
			static long PHMSBH ( R5900::Instruction::Format i, u32 Address );
			static long PINTEH ( R5900::Instruction::Format i, u32 Address );
			static long PINTH ( R5900::Instruction::Format i, u32 Address );
			static long PLZCW ( R5900::Instruction::Format i, u32 Address );
			static long PMADDH ( R5900::Instruction::Format i, u32 Address );
			static long PMADDW ( R5900::Instruction::Format i, u32 Address );
			static long PMADDUW ( R5900::Instruction::Format i, u32 Address );
			static long PMAXH ( R5900::Instruction::Format i, u32 Address );
			static long PMAXW ( R5900::Instruction::Format i, u32 Address );
			static long PMINH ( R5900::Instruction::Format i, u32 Address );
			static long PMINW ( R5900::Instruction::Format i, u32 Address );
			static long PMFHI ( R5900::Instruction::Format i, u32 Address );
			static long PMFLO ( R5900::Instruction::Format i, u32 Address );
			static long PMTHI ( R5900::Instruction::Format i, u32 Address );
			static long PMTLO ( R5900::Instruction::Format i, u32 Address );
			static long PMFHL_LH ( R5900::Instruction::Format i, u32 Address );
			static long PMFHL_SH ( R5900::Instruction::Format i, u32 Address );
			static long PMFHL_LW ( R5900::Instruction::Format i, u32 Address );
			static long PMFHL_UW ( R5900::Instruction::Format i, u32 Address );
			static long PMFHL_SLW ( R5900::Instruction::Format i, u32 Address );
			static long PMTHL_LW ( R5900::Instruction::Format i, u32 Address );
			static long PMSUBH ( R5900::Instruction::Format i, u32 Address );
			static long PMSUBW ( R5900::Instruction::Format i, u32 Address );
			static long PMULTH ( R5900::Instruction::Format i, u32 Address );
			static long PMULTW ( R5900::Instruction::Format i, u32 Address );
			static long PMULTUW ( R5900::Instruction::Format i, u32 Address );
			static long PPAC5 ( R5900::Instruction::Format i, u32 Address );
			static long PPACB ( R5900::Instruction::Format i, u32 Address );
			static long PPACH ( R5900::Instruction::Format i, u32 Address );
			static long PPACW ( R5900::Instruction::Format i, u32 Address );
			static long PREVH ( R5900::Instruction::Format i, u32 Address );
			static long PROT3W ( R5900::Instruction::Format i, u32 Address );
			static long PSLLH ( R5900::Instruction::Format i, u32 Address );
			static long PSLLVW ( R5900::Instruction::Format i, u32 Address );
			static long PSLLW ( R5900::Instruction::Format i, u32 Address );
			static long PSRAH ( R5900::Instruction::Format i, u32 Address );
			static long PSRAW ( R5900::Instruction::Format i, u32 Address );
			static long PSRAVW ( R5900::Instruction::Format i, u32 Address );
			static long PSRLH ( R5900::Instruction::Format i, u32 Address );
			static long PSRLW ( R5900::Instruction::Format i, u32 Address );
			static long PSRLVW ( R5900::Instruction::Format i, u32 Address );
			static long PSUBB ( R5900::Instruction::Format i, u32 Address );
			static long PSUBH ( R5900::Instruction::Format i, u32 Address );
			static long PSUBW ( R5900::Instruction::Format i, u32 Address );
			static long PSUBSB ( R5900::Instruction::Format i, u32 Address );
			static long PSUBSH ( R5900::Instruction::Format i, u32 Address );
			static long PSUBSW ( R5900::Instruction::Format i, u32 Address );
			static long PSUBUB ( R5900::Instruction::Format i, u32 Address );
			static long PSUBUH ( R5900::Instruction::Format i, u32 Address );
			static long PSUBUW ( R5900::Instruction::Format i, u32 Address );
			static long QFSRV ( R5900::Instruction::Format i, u32 Address );
			

			// floating point instructions //

			static long LWC1 ( R5900::Instruction::Format i, u32 Address );
			static long SWC1 ( R5900::Instruction::Format i, u32 Address );
			
			static long ABS_S ( R5900::Instruction::Format i, u32 Address );
			static long ADD_S ( R5900::Instruction::Format i, u32 Address );
			static long ADDA_S ( R5900::Instruction::Format i, u32 Address );
			static long C_EQ_S ( R5900::Instruction::Format i, u32 Address );
			static long C_F_S ( R5900::Instruction::Format i, u32 Address );
			static long C_LE_S ( R5900::Instruction::Format i, u32 Address );
			static long C_LT_S ( R5900::Instruction::Format i, u32 Address );
			static long CVT_S_W ( R5900::Instruction::Format i, u32 Address );
			static long CVT_W_S ( R5900::Instruction::Format i, u32 Address );
			static long DIV_S ( R5900::Instruction::Format i, u32 Address );
			static long MADD_S ( R5900::Instruction::Format i, u32 Address );
			static long MADDA_S ( R5900::Instruction::Format i, u32 Address );
			static long MAX_S ( R5900::Instruction::Format i, u32 Address );
			static long MIN_S ( R5900::Instruction::Format i, u32 Address );
			static long MOV_S ( R5900::Instruction::Format i, u32 Address );
			static long MSUB_S ( R5900::Instruction::Format i, u32 Address );
			static long MSUBA_S ( R5900::Instruction::Format i, u32 Address );
			static long MUL_S ( R5900::Instruction::Format i, u32 Address );
			static long MULA_S ( R5900::Instruction::Format i, u32 Address );
			static long NEG_S ( R5900::Instruction::Format i, u32 Address );
			static long RSQRT_S ( R5900::Instruction::Format i, u32 Address );
			static long SQRT_S ( R5900::Instruction::Format i, u32 Address );
			static long SUB_S ( R5900::Instruction::Format i, u32 Address );
			static long SUBA_S ( R5900::Instruction::Format i, u32 Address );
			

			// PS2 COP2 instructions //

			static long LQC2 ( R5900::Instruction::Format i, u32 Address );
			static long SQC2 ( R5900::Instruction::Format i, u32 Address );
			static long QMFC2_NI ( R5900::Instruction::Format i, u32 Address );
			static long QMTC2_NI ( R5900::Instruction::Format i, u32 Address );
			static long QMFC2_I ( R5900::Instruction::Format i, u32 Address );
			static long QMTC2_I ( R5900::Instruction::Format i, u32 Address );
			
			
			static long VABS ( R5900::Instruction::Format i, u32 Address );
			
			static long VADD ( R5900::Instruction::Format i, u32 Address );
			static long VADDi ( R5900::Instruction::Format i, u32 Address );
			static long VADDq ( R5900::Instruction::Format i, u32 Address );
			static long VADDBCX ( R5900::Instruction::Format i, u32 Address );
			static long VADDBCY ( R5900::Instruction::Format i, u32 Address );
			static long VADDBCZ ( R5900::Instruction::Format i, u32 Address );
			static long VADDBCW ( R5900::Instruction::Format i, u32 Address );
			
			static long VSUB ( R5900::Instruction::Format i, u32 Address );
			static long VSUBi ( R5900::Instruction::Format i, u32 Address );
			static long VSUBq ( R5900::Instruction::Format i, u32 Address );
			static long VSUBBCX ( R5900::Instruction::Format i, u32 Address );
			static long VSUBBCY ( R5900::Instruction::Format i, u32 Address );
			static long VSUBBCZ ( R5900::Instruction::Format i, u32 Address );
			static long VSUBBCW ( R5900::Instruction::Format i, u32 Address );
			
			static long VMUL ( R5900::Instruction::Format i, u32 Address );
			static long VMULi ( R5900::Instruction::Format i, u32 Address );
			static long VMULq ( R5900::Instruction::Format i, u32 Address );
			static long VMULBCX ( R5900::Instruction::Format i, u32 Address );
			static long VMULBCY ( R5900::Instruction::Format i, u32 Address );
			static long VMULBCZ ( R5900::Instruction::Format i, u32 Address );
			static long VMULBCW ( R5900::Instruction::Format i, u32 Address );
			
			static long VMADD ( R5900::Instruction::Format i, u32 Address );
			static long VMADDi ( R5900::Instruction::Format i, u32 Address );
			static long VMADDq ( R5900::Instruction::Format i, u32 Address );
			static long VMADDBCX ( R5900::Instruction::Format i, u32 Address );
			static long VMADDBCY ( R5900::Instruction::Format i, u32 Address );
			static long VMADDBCZ ( R5900::Instruction::Format i, u32 Address );
			static long VMADDBCW ( R5900::Instruction::Format i, u32 Address );
			
			static long VMSUB ( R5900::Instruction::Format i, u32 Address );
			static long VMSUBi ( R5900::Instruction::Format i, u32 Address );
			static long VMSUBq ( R5900::Instruction::Format i, u32 Address );
			static long VMSUBBCX ( R5900::Instruction::Format i, u32 Address );
			static long VMSUBBCY ( R5900::Instruction::Format i, u32 Address );
			static long VMSUBBCZ ( R5900::Instruction::Format i, u32 Address );
			static long VMSUBBCW ( R5900::Instruction::Format i, u32 Address );
			
			static long VMAX ( R5900::Instruction::Format i, u32 Address );
			static long VMAXi ( R5900::Instruction::Format i, u32 Address );
			static long VMAXBCX ( R5900::Instruction::Format i, u32 Address );
			static long VMAXBCY ( R5900::Instruction::Format i, u32 Address );
			static long VMAXBCZ ( R5900::Instruction::Format i, u32 Address );
			static long VMAXBCW ( R5900::Instruction::Format i, u32 Address );
			
			static long VMINI ( R5900::Instruction::Format i, u32 Address );
			static long VMINIi ( R5900::Instruction::Format i, u32 Address );
			static long VMINIBCX ( R5900::Instruction::Format i, u32 Address );
			static long VMINIBCY ( R5900::Instruction::Format i, u32 Address );
			static long VMINIBCZ ( R5900::Instruction::Format i, u32 Address );
			static long VMINIBCW ( R5900::Instruction::Format i, u32 Address );
			
			static long VDIV ( R5900::Instruction::Format i, u32 Address );
			
			static long VADDA ( R5900::Instruction::Format i, u32 Address );
			static long VADDAi ( R5900::Instruction::Format i, u32 Address );
			static long VADDAq ( R5900::Instruction::Format i, u32 Address );
			static long VADDABCX ( R5900::Instruction::Format i, u32 Address );
			static long VADDABCY ( R5900::Instruction::Format i, u32 Address );
			static long VADDABCZ ( R5900::Instruction::Format i, u32 Address );
			static long VADDABCW ( R5900::Instruction::Format i, u32 Address );
			
			static long VSUBA ( R5900::Instruction::Format i, u32 Address );
			static long VSUBAi ( R5900::Instruction::Format i, u32 Address );
			static long VSUBAq ( R5900::Instruction::Format i, u32 Address );
			static long VSUBABCX ( R5900::Instruction::Format i, u32 Address );
			static long VSUBABCY ( R5900::Instruction::Format i, u32 Address );
			static long VSUBABCZ ( R5900::Instruction::Format i, u32 Address );
			static long VSUBABCW ( R5900::Instruction::Format i, u32 Address );
			
			static long VMULA ( R5900::Instruction::Format i, u32 Address );
			static long VMULAi ( R5900::Instruction::Format i, u32 Address );
			static long VMULAq ( R5900::Instruction::Format i, u32 Address );
			static long VMULABCX ( R5900::Instruction::Format i, u32 Address );
			static long VMULABCY ( R5900::Instruction::Format i, u32 Address );
			static long VMULABCZ ( R5900::Instruction::Format i, u32 Address );
			static long VMULABCW ( R5900::Instruction::Format i, u32 Address );
			
			static long VMADDA ( R5900::Instruction::Format i, u32 Address );
			static long VMADDAi ( R5900::Instruction::Format i, u32 Address );
			static long VMADDAq ( R5900::Instruction::Format i, u32 Address );
			static long VMADDABCX ( R5900::Instruction::Format i, u32 Address );
			static long VMADDABCY ( R5900::Instruction::Format i, u32 Address );
			static long VMADDABCZ ( R5900::Instruction::Format i, u32 Address );
			static long VMADDABCW ( R5900::Instruction::Format i, u32 Address );
			
			static long VMSUBA ( R5900::Instruction::Format i, u32 Address );
			static long VMSUBAi ( R5900::Instruction::Format i, u32 Address );
			static long VMSUBAq ( R5900::Instruction::Format i, u32 Address );
			static long VMSUBABCX ( R5900::Instruction::Format i, u32 Address );
			static long VMSUBABCY ( R5900::Instruction::Format i, u32 Address );
			static long VMSUBABCZ ( R5900::Instruction::Format i, u32 Address );
			static long VMSUBABCW ( R5900::Instruction::Format i, u32 Address );
			
			static long VOPMULA ( R5900::Instruction::Format i, u32 Address );
			static long VOPMSUB ( R5900::Instruction::Format i, u32 Address );

			static long VNOP ( R5900::Instruction::Format i, u32 Address );
			static long VCLIP ( R5900::Instruction::Format i, u32 Address );
			static long VSQRT ( R5900::Instruction::Format i, u32 Address );
			static long VRSQRT ( R5900::Instruction::Format i, u32 Address );
			static long VMR32 ( R5900::Instruction::Format i, u32 Address );
			static long VRINIT ( R5900::Instruction::Format i, u32 Address );
			static long VRGET ( R5900::Instruction::Format i, u32 Address );
			static long VRNEXT ( R5900::Instruction::Format i, u32 Address );
			static long VRXOR ( R5900::Instruction::Format i, u32 Address );
			static long VMOVE ( R5900::Instruction::Format i, u32 Address );
			static long VMFIR ( R5900::Instruction::Format i, u32 Address );
			static long VMTIR ( R5900::Instruction::Format i, u32 Address );
			static long VLQD ( R5900::Instruction::Format i, u32 Address );
			static long VLQI ( R5900::Instruction::Format i, u32 Address );
			static long VSQD ( R5900::Instruction::Format i, u32 Address );
			static long VSQI ( R5900::Instruction::Format i, u32 Address );
			static long VWAITQ ( R5900::Instruction::Format i, u32 Address );
			
			static long VFTOI0 ( R5900::Instruction::Format i, u32 Address );
			static long VFTOI4 ( R5900::Instruction::Format i, u32 Address );
			static long VFTOI12 ( R5900::Instruction::Format i, u32 Address );
			static long VFTOI15 ( R5900::Instruction::Format i, u32 Address );
			
			static long VITOF0 ( R5900::Instruction::Format i, u32 Address );
			static long VITOF4 ( R5900::Instruction::Format i, u32 Address );
			static long VITOF12 ( R5900::Instruction::Format i, u32 Address );
			static long VITOF15 ( R5900::Instruction::Format i, u32 Address );
			
			static long VIADD ( R5900::Instruction::Format i, u32 Address );
			static long VISUB ( R5900::Instruction::Format i, u32 Address );
			static long VIADDI ( R5900::Instruction::Format i, u32 Address );
			static long VIAND ( R5900::Instruction::Format i, u32 Address );
			static long VIOR ( R5900::Instruction::Format i, u32 Address );
			static long VILWR ( R5900::Instruction::Format i, u32 Address );
			static long VISWR ( R5900::Instruction::Format i, u32 Address );
			static long VCALLMS ( R5900::Instruction::Format i, u32 Address );
			static long VCALLMSR ( R5900::Instruction::Format i, u32 Address );
	};
};

#endif
