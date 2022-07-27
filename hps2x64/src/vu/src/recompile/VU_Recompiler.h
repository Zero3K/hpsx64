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



#ifndef _VU_RECOMPILER_H_
#define _VU_RECOMPILER_H_


#include "VU.h"
#include "VU_Lookup.h"
#include "x64Encoder.h"


//class Playstation2::VU;

using namespace Vu::Instruction;

namespace Playstation2
{
	class VU;
}

namespace Vu
{
	
	//using namespace Playstation2;
	//class Playstation2::VU;
	
	// will probably create two recompilers for each processor. one for single stepping and one for multi-stepping
	class Recompiler
	{
	public:

		static Recompiler *_REC;

		// number of code cache slots total
		// but the number of blocks should be variable
		//static const int c_iNumBlocks = 1024;
		//static const u32 c_iNumBlocks_Mask = c_iNumBlocks - 1;
		u32 NumBlocks;
		u32 NumBlocks_Mask;
		//static const int c_iBlockSize_Shift = 4;
		//static const int c_iBlockSize_Mask = ( 1 << c_iBlockSize_Shift ) - 1;
		
		static const u32 c_iAddress_Mask = 0x1fffffff;
		
		u32 TotalCodeCache_Size;	// = c_iNumBlocks * ( 1 << c_iBlockSize_Shift );
		
		u32 BlockSize;
		u32 BlockSize_Shift;
		u32 BlockSize_Mask;
		
		// maximum number of instructions that can be encoded/recompiled in a run
		u32 MaxStep;
		u32 MaxStep_Shift;
		u32 MaxStep_Mask;
		
		
		
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
		
		RDelaySlot RDelaySlots [ 2 ];
		u32 DSIndex;
		
		u32 RDelaySlots_Valid;
		
		inline void ClearDelaySlots ( void ) { RDelaySlots [ 0 ].Value = 0; RDelaySlots [ 1 ].Value = 0; }
		
		// need something to use to keep track of dynamic delay slot stuff
		u32 RecompilerDelaySlot_Flags;
		
		
		// the next instruction in the cache block if there is one
		Vu::Instruction::Format NextInst;
		
		u64 MemCycles;

		u64 LocalCycleCount;
		u64 CacheBlock_CycleCount;
		
		// also need to know if the addresses in the block are cached or not
		bool bIsBlockInICache;

		
		// the PC while recompiling
		u32 LocalPC;


		// need to know the checksum for code that is recompiled for caching
		u64 ullChecksum;

		// need to calculate the checksum
		static u64 Calc_Checksum( VU *v );


		// static analysis data //
		
		// up to 2k instructions (16k bytes / 8 bytes per instruction)
		// up to 64 possibilities per instruction (instr count,q/p wait1,q/p wait2)
		// 8-bits/1-byte per possibility
		//u8 LUT_StaticDelay [ 2048 * 256 ];
		//u8 LUT_StaticDelay [ 2048 * 64 ];

		// also need to know where to pull the flags from if instruction needs them
		//u8 LUT_StaticFlags [ 2048 * 4 ];

		// analysis flags
		// bit 0 - upper/lower register hazard (calc wait/delay unless zero, then no need to calc wait/delay)
		// bit 1 - output mac flag registers to history buffer at instruction end
		// bit 2 - output stat flag registers to history buffer at instruction end
		// bit 3 - output clip flag registers to history buffer at instruction end
		// bit 4 - both upper and lower instruction output to mac/stat or clip flag or register, ignore lower instruction
		// bit 5 - complete move instruction at end (store from temporary location, unless cancelled)
		// bit 6 - conditional branch instruction
		// bit 7 - unconditional branch instruction
		// bit 8 - conditional branch delay slot (branch after instruction if instr# not zero)
		// bit 9 - unconditional branch delay slot (branch after instruction if instr# not zero)
		// bit 10 - output int to current delay4 slot (conditional branch next that uses previous int dest reg)
		// bit 11 - load int from previous delay4 slot if instr# count set after conditional branch at end
		// bit 12 - lower register hazard detected
		// bit 13 - experimental int delay4 slot detected (load/store with inc/dec with inc/dec reg used later)
		// bit 14/15 - last conflict count (for float or int load conflict/wait)
		// bit 16 - bit0 has a simple hazard, not a complex one
		// bit 17 - e-bit delay slot
		// bit 18 - output CycleCount to history buffer at instruction end
		// bit 19 - update q (NOT waitq) before upper instruction
		// bit 20 - reverse order of lower/upper execution (execute upper first, then lower)
		// bit 21 - mac flag check
		// bit 22 - stat flag check
		// bit 23 - clip flag check
		// bit 24 - output mac flag data
		// bit 25 - output stat flag data
		// bit 26 - output clip flag data
		// bit 27 - implicit waitq
		// bit 28 - implicit waitp
		// bit 29 - implicit updatep (not waitp) (for mfp instruction)
		// bit 30 - xgkick delay slot

		// todo: *note* don't forget to make sure q/p set at very end of vu process run (unless a vu0 continue)
		u32 LUT_StaticInfo [ 2048 ];

		// perform static analysis
		void StaticAnalysis ( VU *v );


		bool isBranch ( Vu::Instruction::Format64 i );
		bool isConditionalBranch ( Vu::Instruction::Format64 i );
		bool isUnconditionalBranch ( Vu::Instruction::Format64 i );
		bool isMacFlagCheck ( Vu::Instruction::Format64 i );
		bool isStatFlagCheck ( Vu::Instruction::Format64 i );
		bool isClipFlagCheck ( Vu::Instruction::Format64 i );
		bool isStatFlagSetLo ( Vu::Instruction::Format64 i );
		bool isClipFlagSetLo ( Vu::Instruction::Format64 i );
		bool isLowerImmediate ( Vu::Instruction::Format64 i );
		bool isClipFlagSetHi ( Vu::Instruction::Format64 i );
		bool isMacStatFlagSetHi ( Vu::Instruction::Format64 i );

		u64 getSrcRegMapHi ( Vu::Instruction::Format64 i );
		u64 getDstRegMapHi ( Vu::Instruction::Format64 i );
		void getDstFieldMapHi ( VU::Bitmap128 &Bm, Vu::Instruction::Format64 i );
		void getSrcFieldMapHi ( VU::Bitmap128 &Bm, Vu::Instruction::Format64 i );
		void getDstFieldMapLo ( VU::Bitmap128 &Bm, Vu::Instruction::Format64 i );
		void getSrcFieldMapLo ( VU::Bitmap128 &Bm, Vu::Instruction::Format64 i );
		u64 getDstRegMapLo ( Vu::Instruction::Format64 i );
		u64 getSrcRegMapLo ( Vu::Instruction::Format64 i );

		bool isMoveToFloatLo ( Vu::Instruction::Format64 i );
		bool isMoveToFloatFromFloatLo ( Vu::Instruction::Format64 i );

		bool isIntLoad ( Vu::Instruction::Format64 i );

		bool isFloatLoadStore ( Vu::Instruction::Format64 i );

		bool isQWait ( Vu::Instruction::Format64 i );
		bool isPWait ( Vu::Instruction::Format64 i );

		bool isQUpdate ( Vu::Instruction::Format64 i );
		bool isPUpdate ( Vu::Instruction::Format64 i );

		bool isXgKick ( Vu::Instruction::Format64 i );
		bool isIntCalc ( Vu::Instruction::Format64 i );


		static bool Perform_GetMacFlag ( x64Encoder *e, VU* v, u32 Address );
		static bool Perform_GetStatFlag ( x64Encoder *e, VU* v, u32 Address );
		static bool Perform_GetClipFlag ( x64Encoder *e, VU* v, u32 Address );

		static bool Perform_WaitQ ( x64Encoder *e, VU* v, u32 Address );
		static bool Perform_WaitP ( x64Encoder *e, VU* v, u32 Address );

		static bool Perform_UpdateQ ( x64Encoder *e, VU* v, u32 Address );
		static bool Perform_UpdateP ( x64Encoder *e, VU* v, u32 Address );

		// the optimization level
		// 0 means no optimization at all, anything higher means to optimize
		//s32 OpLevel;
		//u32 OptimizeLevel;
		
		// the current enabled encoder
		x64Encoder *e;
		//static ICache_Device *ICache;
		//static VU::Cpu *r;
		
		// the encoder for this particular instance
		x64Encoder *InstanceEncoder;
		
		// bitmap for branch delay slot
		//u32 Status_BranchDelay;
		//u32 Status_BranchConditional;
		//Vu::Instruction::Format Status_BranchInstruction;
		
		u32 Status_EBit;
		
		// the maximum number of cache blocks it can encode across
		s32 MaxCacheBlocks;
		
		
		//u32 SetStatus_Flag;
		//u32 SetClip_Flag;

		Vu::Instruction::Format instLO;
		Vu::Instruction::Format instHI;
		

		// vector constants for the recompiler
		// note: this should be part of the recompiler, since the constants apply to that recompiler instance
		u32 CountOfVConsts;
		u32 LastVConstCount;
		alignas(64) Reg128 VectorConstants [ 4096 ];


		static void AdvanceCycle ( VU* v );
		static void AdvanceCycle_rec ( x64Encoder *e, VU* v );
		
		
		inline void Set_MaxCacheBlocks ( s32 Value ) { MaxCacheBlocks = Value; }
		
		// constructor
		// block size must be power of two, multiplier shift value
		// so for BlockSize_PowerOfTwo, for a block size of 4 pass 2, block size of 8 pass 3, etc
		// for MaxStep, use 0 for single stepping, 1 for stepping until end of 1 cache block, 2 for stepping until end of 2 cache blocks, etc
		// no, for MaxStep, it should be the maximum number of instructions to step
		// NumberOfBlocks MUST be a power of 2, where 1 means 2, 2 means 4, etc
		Recompiler ( VU* v, u32 NumberOfBlocks, u32 BlockSize_PowerOfTwo, u32 MaxIStep );
		
		// destructor
		~Recompiler ();
		
		
		void Reset ();	// { memset ( this, 0, sizeof( Recompiler ) ); }

		
		static bool isBranchDelayOk ( u32 ulInstruction, u32 Address );

		
		static u32* RGetPointer ( VU *v, u32 Address );
		
		
		// set the optimization level
		inline void SetOptimizationLevel ( VU* v, u32 Level ) { v->OptimizeLevel = Level; }
		
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


		static int Prefix_MADDW ( VU* v, Vu::Instruction::Format i );
		static int Postfix_MADDW ( VU* v, Vu::Instruction::Format i );
		
		
		// initializes a code block that is not being used yet
		u32 InitBlock ( u32 Block );
		
		
		inline static void Clear_FSrcReg ( VU* v ) { VU::ClearBitmap ( v->FSrcBitmap ); }
		inline static void Add_FSrcReg ( VU* v, u32 i, u32 SrcReg ) { if ( SrcReg ) { VU::AddBitmap ( v->FSrcBitmap, ( i >> 21 ) & 0xf, SrcReg ); } }
		inline static void Add_FSrcRegBC ( VU* v, u32 i, u32 SrcReg ) { if ( SrcReg ) { VU::AddBitmap ( v->FSrcBitmap, 0x8 >> ( i & 0x3 ), SrcReg ); } }

		inline static void Clear_ISrcReg ( VU* v ) { v->ISrcBitmap = 0; }
		inline static void Add_ISrcReg ( VU* v, u32 SrcReg ) { if ( SrcReg & 31 ) { v->ISrcBitmap |= ( 1ull << ( SrcReg + 0 ) ); } }

		inline static void Clear_DstReg ( VU* v ) { VU::ClearBitmap ( v->FDstBitmap ); v->IDstBitmap = 0; }
		inline static void Add_FDstReg ( VU* v, u32 i, u32 DstReg ) { if ( DstReg ) { VU::AddBitmap ( v->FDstBitmap, ( i >> 21 ) & 0xf, DstReg ); v->IDstBitmap |= ( 1ULL << DstReg ); } }
		inline static void Add_IDstReg ( VU* v, u32 DstReg ) { if ( DstReg & 31 ) { v->IDstBitmap |= ( 1ULL << ( DstReg + 0 ) ); } }
		
		static void PipelineWaitQ ( VU* v );
		
		static void TestStall ( VU* v );
		static void TestStall_INT ( VU* v );

		static void SetDstBitmap ( VU* v, u64 b0, u64 b1, u64 i0 );

		// recompilation of SetDstBitmap
		static void SetDstBitmap_rec ( x64Encoder *e, VU* v, u64 b0, u64 b1, u64 i0 );

		
		long ProcessBranch ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u32 Address );

		

		static u64 GetFSourceRegsHI_LoXYZW ( Vu::Instruction::Format i );
		static u64 GetFSourceRegsHI_HiXYZW ( Vu::Instruction::Format i );
		static u64 GetFSourceRegsLO_LoXYZW ( Vu::Instruction::Format i );
		static u64 GetFSourceRegsLO_HiXYZW ( Vu::Instruction::Format i );

		static u64 GetFDestRegsHI_LoXYZW ( Vu::Instruction::Format i );
		static u64 GetFDestRegsHI_HiXYZW ( Vu::Instruction::Format i );
		
		// get a bitmap for the source registers used by the specified instruction
		static u64 GetFSourceRegsHI ( Vu::Instruction::Format i );
		static u64 GetFSourceRegsLO ( Vu::Instruction::Format i );
		static u64 GetFDestRegsHI ( Vu::Instruction::Format i );
		static u64 Get_DelaySlot_DestRegs ( Vu::Instruction::Format i );
		
		//static long Generate_Normal_Store ( R5900::Instruction::Format i, u32 Address, u32 BitTest, void* StoreFunctionToCall );
		//static long Generate_Normal_Load ( R5900::Instruction::Format i, u32 Address, u32 BitTest, void* LoadFunctionToCall );
		//static long Generate_Normal_Branch ( R5900::Instruction::Format i, u32 Address, void* BranchFunctionToCall );
		//static long Generate_Normal_Trap ( R5900::Instruction::Format i, u32 Address );

		static long Generate_VABSp ( x64Encoder *e, VU* v, Vu::Instruction::Format i );
		static long Generate_VMAXp ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u32 *pFt = NULL, u32 FtComponent = 4 );
		static long Generate_VMINp ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u32 *pFt = NULL, u32 FtComponent = 4 );
		static long Generate_VFTOIXp ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u32 IX );
		static long Generate_VITOFXp ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u64 FX );
		static long Generate_VMOVEp ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u32 Address );
		static long Generate_VMR32p ( x64Encoder *e, VU* v, Vu::Instruction::Format i );
		static long Generate_VMFIRp ( x64Encoder *e, VU* v, Vu::Instruction::Format i );
		static long Generate_VMTIRp ( x64Encoder *e, VU* v, Vu::Instruction::Format i );
		static long Generate_VADDp ( x64Encoder *e, VU* v, u32 bSub, Vu::Instruction::Format i, u32 FtComponent = 4, void *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMULp ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u32 FtComponentp = 0x1b, void *pFd = NULL, u32 *pFt = NULL, u32 FsComponentp = 0x1b );
		static long Generate_VMADDp ( x64Encoder *e, VU* v, u32 bSub, Vu::Instruction::Format i, u32 FtComponentp = 0x1b, void *pFd = NULL, u32 *pFt = NULL, u32 FsComponentp = 0x1b );

		// testing
		static long Generate_VFTOIXp_test ( x64Encoder *e, VU* v, Vu::Instruction::Format i, u32 IX );
		static void Test_FTOI4 ( VU* v, Vu::Instruction::Format i );

		/*
		static long Generate_VABSp ( Vu::Instruction::Format i );
		static long Generate_VMAXp ( Vu::Instruction::Format i, u32 *pFt = NULL, u32 FtComponent = 4 );
		static long Generate_VMINp ( Vu::Instruction::Format i, u32 *pFt = NULL, u32 FtComponent = 4 );
		static long Generate_VFTOIXp ( Vu::Instruction::Format i, u32 IX );
		static long Generate_VITOFXp ( Vu::Instruction::Format i, u32 FX );
		static long Generate_VMOVEp ( Vu::Instruction::Format i );
		static long Generate_VMR32p ( Vu::Instruction::Format i );
		static long Generate_VMFIRp ( Vu::Instruction::Format i );
		static long Generate_VMTIRp ( Vu::Instruction::Format i );
		static long Generate_VADDp ( u32 bSub, Vu::Instruction::Format i, u32 FtComponent = 4, void *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMULp ( Vu::Instruction::Format i, u32 FtComponentp = 0x1b, void *pFd = NULL, u32 *pFt = NULL, u32 FsComponentp = 0x1b );
		static long Generate_VMADDp ( u32 bSub, Vu::Instruction::Format i, u32 FtComponentp = 0x1b, void *pFd = NULL, u32 *pFt = NULL, u32 FsComponentp = 0x1b );
		
		
		static long Generate_VABS ( Vu::Instruction::Format i, u32 Address, u32 Component );
		static long Generate_VMAX ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFt = NULL );
		static long Generate_VMIN ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFt = NULL );
		static long Generate_VFTOI0 ( Vu::Instruction::Format i, u32 Address, u32 FtComponent, u32 FsComponent );
		static long Generate_VFTOIX ( Vu::Instruction::Format i, u32 Address, u32 FtComponent, u32 FsComponent, u32 IX );
		static long Generate_VITOFX ( Vu::Instruction::Format i, u32 Address, u32 FtComponent, u32 FsComponent, u32 FX );
		static long Generate_VMOVE ( Vu::Instruction::Format i, u32 Address, u32 FtComponent, u32 FsComponent );
		
		static long Generate_VMR32_Load ( Vu::Instruction::Format i, u32 Address, u32 FsComponent );
		static long Generate_VMR32_Store ( Vu::Instruction::Format i, u32 Address, u32 FtComponent );
		
		static long Generate_VMFIR ( Vu::Instruction::Format i, u32 Address, u32 FtComponent );
		static long Generate_VADD ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VSUB ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMUL ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMADD ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		static long Generate_VMSUB ( Vu::Instruction::Format i, u32 Address, u32 FdComponent, u32 FsComponent, u32 FtComponent, u32 *pFd = NULL, u32 *pFt = NULL );
		*/
		
		
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
		u32* StartAddress;	// [ c_iNumBlocks ];
		
		
		// last address that instruction encoding is valid for (inclusive)
		// *note* this should be dynamically allocated
		// It actually makes things MUCH simpler if you store the number of instructions recompiled instead of the last address
		//u32* LastAddress;	// [ c_iNumBlocks ];
		//u8* RunCount;
		
		//u32* Instructions;
		
		// pointer to where the prefix for the instruction starts at (used only internally by recompiler)
		u8** pPrefix_CodeStart;
		
		// where the actual instruction starts at in code block
		u8** pCodeStart;
		
		// the number of offset cycles from this instruction in the code block
		u32* CycleCount;
		
		
		// list of branch targets when jumping forwards in code block
		u32* pForwardBranchTargets;
		
		static const int c_ulForwardBranchIndex_Start = 5;
		u32 ForwardBranchIndex;
		
		u32 StartBlockIndex;
		u32 BlockIndex;
		
		// not needed
		//u32* EndAddress;
		
		
		long Generate_Prefix_EventCheck ( u32 Address, bool bIsBranchOrJump );
		
		
		
		// need to know what address current block starts at
		u32 CurrentBlock_StartAddress;
		
		// also need to know what address the next block starts at
		u32 NextBlock_StartAddress;
		
		
		// max number of cycles that instruction encoding could use up if executed
		// need to know this to single step when there are interrupts in between
		// code block is not valid when this is zero
		// *note* this should be dynamically allocated
		//u64* MaxCycles;	// [ c_iNumBlocks ];
		
		u32 Local_LastModifiedReg;
		u32 Local_NextPCModified;
		
		u32 CurrentCount;
		
		u32 isBranchDelaySlot;
		u32 isLoadDelaySlot;
		
		//u32 bStopEncodingAfter;
		//u32 bStopEncodingBefore;
		
		// set the local cycle count to reset (start from zero) for the next cycle
		u32 bResetCycleCount;
		
		//inline void ResetFlags ( void ) { bStopEncodingBefore = false; bStopEncodingAfter = false; Local_NextPCModified = false; }

		
		
		// recompiler function
		// returns -1 if the instruction cannot be recompiled, otherwise returns the maximum number of cycles the instruction uses
		typedef long (*Function) ( x64Encoder *e, VU *v, Vu::Instruction::Format Instruction, u32 Address );

		// *** todo *** do not recompile more than one instruction if currently in a branch delay slot or load delay slot!!
		u32 Recompile ( VU* v, u32 StartAddress );

		// new recompile code using static analysis data
		u32 Recompile2 ( VU* v, u32 StartAddress );


		//void Invalidate ( u32 Address );
		void Invalidate ( u32 Address, u32 Count );
		
		u32 CloseOpLevel ( u32 OptLevel, u32 Address );

		bool isNopHi ( Vu::Instruction::Format i );
		bool isNopLo ( Vu::Instruction::Format i );
		
		
		u32 ulIndex_Mask;
		
		inline u32 Get_Block ( u32 Address ) { return ( Address >> ( 3 + MaxStep_Shift ) ) & NumBlocks_Mask; }
		inline u32 Get_Index ( u32 Address ) { return ( Address >> 3 ) & ulIndex_Mask; }

		
		//void Recompile ( u32 Instruction );
		
			static const Function FunctionList [];
			
		// used by object to recompile an instruction into a code block
		// returns -1 if the instruction cannot be recompiled
		// returns 0 if the instruction was recompiled, but MUST start a new block for the next instruction (because it is guaranteed in a delay slot)
		// returns 1 if successful and can continue recompiling
		inline static long RecompileHI ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address ) { return Vu::Recompiler::FunctionList [ Vu::Instruction::Lookup::FindByInstructionHI ( i.Value ) ] ( e, v, i, Address ); }
		inline static long RecompileLO ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address ) { return Vu::Recompiler::FunctionList [ Vu::Instruction::Lookup::FindByInstructionLO ( i.Value ) ] ( e, v, i, Address ); }
		
		
		inline void Run ( u32 Address ) { InstanceEncoder->ExecuteCodeBlock ( ( Address >> 2 ) & NumBlocks_Mask ); }


			static long INVALID ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			

			static long ADDBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long SUBBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MADDBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MSUBBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MAXBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MAXBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MAXBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MAXBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MINIBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MINIBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MINIBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MINIBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MULBCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULBCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULBCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULBCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MULq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MAXi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MINIi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long OPMSUB ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			//static long ADDi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ADDq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ADDAi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ADDAq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long ADDABCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDABCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDABCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDABCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MADD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MUL ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MAX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUB ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUB ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long OPMSUM ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MINI ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long IADD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ISUB ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long IADDI ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long IAND ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long IOR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long CALLMS ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long CALLMSR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ITOF0 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FTOI0 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULAq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDAq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBAq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDA ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBA ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MOVE ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long LQI ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long DIV ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MTIR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long RNEXT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ITOF4 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FTOI4 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ABS ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDAq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBAq ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDA ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBA ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long MR32 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long SQI ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long SQRT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long MFIR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long RGET ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			//static long ADDABCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ADDABCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ADDABCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ADDABCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long SUBABCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBABCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBABCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBABCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MADDABCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDABCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDABCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDABCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MSUBABCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBABCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBABCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBABCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long ITOF12 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FTOI12 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MULABCX ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULABCY ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULABCZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULABCW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long MULAi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ADDAi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SUBAi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MULA ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long OPMULA ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long LQD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long RSQRT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ILWR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long RINIT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ITOF15 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FTOI15 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long CLIP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MADDAi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MSUBAi ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long NOP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long SQD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );


			// lower instructions

			
			static long LQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ILW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ISW ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long IADDIU ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ISUBIU ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FCEQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FCSET ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FCAND ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FCOR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FSEQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FSSET ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FSAND ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FSOR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FMEQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FMAND ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FMOR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long FCGET ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long B ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long BAL ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long JR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long JALR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long IBEQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long IBNE ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long IBLTZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long IBGTZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long IBLEZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long IBGEZ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			
			//static long DIV ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long EATANxy ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long EATANxz ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long EATAN ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long IADD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long ISUB ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long IADDI ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long IAND ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long IOR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long MOVE ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long LQI ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long DIV ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			//static long MTIR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long RNEXT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MFP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long XTOP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long XGKICK ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );

			static long MR32 ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SQI ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SQRT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long MFIR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long RGET ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			static long XITOP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ESADD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long EATANxy ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ESQRT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ESIN ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ERSADD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long EATANxz ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ERSQRT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long EATAN ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long LQD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long RSQRT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ILWR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long RINIT ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ELENG ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ESUM ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ERCPR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long EEXP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long SQD ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long WAITQ ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ISWR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long RXOR ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long ERLENG ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			static long WAITP ( x64Encoder *e, VU *v, Vu::Instruction::Format i, u32 Address );
			
			
	};
};

//};

#endif
