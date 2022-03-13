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



#ifndef _X64ENCODER_H_

#define _X64ENCODER_H_


#include "x64Instructions.h"



#define INVALIDCODEBLOCK		0xffffffff

// some x64 instructions have more than 1 opcode
#define MAKE2OPCODE(opcode1,opcode2)			( ( ( opcode2 ) << 8 ) | ( opcode1 ) )
#define MAKE3OPCODE(opcode1,opcode2,opcode3)	( ( ( opcode3 ) << 16 ) | ( ( opcode2 ) << 8 ) | ( opcode1 ) )
#define MAKE4OPCODE(opcode1,opcode2,opcode3,opcode4)	( ( ( opcode4 ) << 24 ) | ( ( opcode3 ) << 16 ) | ( ( opcode2 ) << 8 ) | ( opcode1 ) )
#define GETOPCODE1(multiopcode)					( ( multiopcode ) & 0xff )
#define GETOPCODE2(multiopcode)					( ( ( multiopcode ) >> 8 ) & 0xff )
#define GETOPCODE3(multiopcode)					( ( ( multiopcode ) >> 16 ) & 0xff )
#define GETOPCODE4(multiopcode)					( ( ( multiopcode ) >> 24 ) & 0xff )


// this not only encodes from a source cpu to a target cpu but also holds the code and runs it
// lNumberOfCodeBlocks must be a power of 2
// the total amount allocated to the x64 code area is lCodeBlockSize*lNumberOfCodeBlocks*2
// lCodeBlockSize is in bytes

class x64Encoder
{

public:
	
	// need to know what code is where
	// this contains the full source address (no, I'll use address/4 or address/8) and the index of the x64 block in the array of x64 code
	// you look up by using the lower bits of the source address
	long* x64CodeHashTable;
	
	// we also need to know the source cpu address that points to next instruction after block - not next instruction to execute, just the next one
	long* x64CodeSourceNextAddress;

	// this is going to point to the current code area
	char* x64CodeArea;
	
	// this is going to be where all the live cached x64 code is at - no need for this after testing
	char* LiveCodeArea;
	
	// I'll need a work area before copying all dynamic code into actual code block
	char* AlternateStream;

	long lCodeBlockSize_PowerOfTwo, lCodeBlockSize, lCodeBlockSize_Mask;
	long lNumberOfCodeBlocks;
	
	// I need the index for the current code block
	long x64CurrentCodeBlockIndex;
	
	// the total amount of space allocated for code area
	long x64TotalSpaceAllocated;

	// need to know the address where to start putting the next x64 instruction
	long x64NextOffset;
	
	// need to know the address of the start of the current x64 instruction being encoded
	long x64CurrentStartOffset;
	
	// need to know offset first x64 byte of current source cpu instruction being encoded
	long x64CurrentSourceCpuInstructionStartOffset;
	
	// need to know the Source Cpu Address that is currently being worked on
	long x64CurrentSourceAddress;
	
	// need to keep track of the source cpu instruction size
	long x64SourceCpuInstructionSize;

	// need to know if we are currently encoding or if we're ready to execute
	bool isEncoding;
	
	// we also ned to know that it was set up successfully
	bool isReadyForUse;
	
	// the size of code in bytes in alternate stream
	long lAlternateStreamSize;
	
	// holds the code for alternate stream
	unsigned long long ullAlternateStream;

	// no need for this stuff after testing - modern cpus can handle this stuff without any tricks needed
	// we can get more control over pre-loading of data by separating pre-allocation and instructions into different streams
	// I don't know the effect on performance, so I'll add in some way to control operation of this
//	long PreAllocationBitmap;	// just set the bits for registers, then if a bit is already set copy the streams (PreAlloc then Inst), reset, repeat
//	long PreAllocationStreamSize;
//	long InstructionStreamSize;
//	char* PreAllocationStream;
//	char* InstructionStream;
	
	// constructor
	// CountOfCodeBlocks must be a power of 2, OR ELSE!!!!
	x64Encoder ( long SizePerCodeBlock_PowerOfTwo, long CountOfCodeBlocks );
	
	// destructor
	~x64Encoder ( void );
	
	// flush current code block
	bool FlushCurrentCodeBlock ( void );
	
	// flush dynamic code block - must do this once after creation before you can use it
	bool FlushCodeBlock ( long IndexOfCodeBlock );
	
	// flush instruction cache line for current process - have to use this with dynamic code or else
	bool FlushICache ( long long baseAddress, long NumberOfBytes );

	inline void Emit_AltStreamToBlock8 ( long lCodeBlockIndex )
	{
		((unsigned long long*)LiveCodeArea) [ ( lCodeBlockIndex << lCodeBlockSize_PowerOfTwo ) >> 3 ] = ullAlternateStream;
	}


	inline void Emit_AltStreamToBlock4 ( long lCodeBlockIndex )
	{
		((unsigned long*)LiveCodeArea) [ ( lCodeBlockIndex << lCodeBlockSize_PowerOfTwo ) >> 2 ] = ullAlternateStream;
	}

	
	// get/set current instruction offset
	long GetCurrentInstructionOffset ( void );
	void SetCurrentInstructionOffset ( long offset );

	// get the size of a code block for encoder - should help with determining if there is more space available
	long GetCodeBlockSize ( void );
	
	// get the size of the current dynamic code being worked out - should help with determining if there is more space available
	// this will only work when currently working in the alternate stream
	long GetAlternateStreamCurrentSize ( void );
	
	// start writing code to alternate stream
	void SwitchToAlternateStream ( void );
	
	// start writing code to live code area
	void SwitchToLiveStream ( void );
	
	// copy dynamic code from alternate stream to current position in live stream
	void CopyToLiveStream ( void );

	// call this before beginning an x64 code block
	bool StartCodeBlock ( long lCodeBlockIndex );

	// call this when you are done with the code block
	bool EndCodeBlock ( void );

	// gets a pointer to the start of current code block
	char* Get_CodeBlock_StartPtr ();
	
	// gets a pointer to the end of current code block (where there SHOULD be return instruction)
	char* Get_CodeBlock_EndPtr ();

	// get the start pointer for an arbitrary code block number
	char* Get_XCodeBlock_StartPtr ( long lCodeBlockIndex );
	
	// get the current pointer in code block
	char* Get_CodeBlock_CurrentPtr ();

	// call this before starting to encode a single instruction from source cpu
	bool StartInstructionBlock ( void );

	// call this when done encoding a single instructin from source cpu
	bool EndInstructionBlock ( void );

	// takes you to the start of the current source cpu instruction you were encoding - cuz you may need to backtrack
	bool UndoInstructionBlock ( void );
	
	// this returns the amount of space remaining in the current code block
	long x64CurrentCodeBlockSpaceRemaining ( void );
	
	// invalidate a code block that has been overwritten in source cpu memory or is otherwise no longer valid
	bool x64InvalidateCodeBlock ( long lSourceAddress );

	// check if code is already encoded and ready to run
	bool x64IsEncodedAndReady ( long lSourceAddress );

	// executes a code block and returns address of the next instruction
	//long long ExecuteCodeBlock ( long lSourceAddress );
	inline long long ExecuteCodeBlock ( long lSourceAddress )
	{
		// function will return the address of the next instruction to execute
		typedef long long (*asm_function) ( void );
		
		volatile asm_function x64Function;

		//long lCodeBlockIndex;
		
		// get the index for the code block to put x64 code in
		//lCodeBlockIndex = ( lSourceAddress & ( lNumberOfCodeBlocks - 1 ) );
		
		//x64Function = (asm_function) ( & ( x64CodeArea [ lCodeBlockIndex * lNumberOfCodeBlocks ] ) );
		x64Function = (asm_function) ( & ( x64CodeArea [ lSourceAddress << lCodeBlockSize_PowerOfTwo ] ) );

		return x64Function ();
	}



	// **** Functions For Encoding of x64 Instructions **** //
	
	// encode an instruction with no register arguments
	bool x64Encode16 ( long x64InstOpcode );
	bool x64Encode32 ( long x64InstOpcode );
	bool x64Encode64 ( long x64InstOpcode );
	
	// encode a move immediate instruction using fewest bytes possible
	bool x64EncodeMovImm64 ( long x64DestRegIndex, long long Immediate64 );
	bool x64EncodeMovImm32 ( long x64DestRegIndex, long Immediate32 );
	bool x64EncodeMovImm16 ( long x64DestRegIndex, short Immediate16 );
	bool x64EncodeMovImm8 ( long x64DestRegIndex, long Immediate8 );

	// encode a single register x64 instruction into code block
	bool x64EncodeReg32 ( long x64InstOpcode, long ModRMOpcode, long x64Reg );
	bool x64EncodeReg64 ( long x64InstOpcode, long ModRMOpcode, long x64Reg );
	
	// encode a register-register x64 instruction into code block
	bool x64EncodeRegReg16 ( long x64InstOpcode, long x64DestReg_Reg_Opcode, long x64SourceReg_RM_Base );
	bool x64EncodeRegReg32 ( long x64InstOpcode, long x64DestReg_Reg_Opcode, long x64SourceReg_RM_Base );
	bool x64EncodeRegReg64 ( long x64InstOpcode, long x64DestReg_Reg_Opcode, long x64SourceReg_RM_Base );

	// encode a x64 Imm8 instruction into code block
	bool x64EncodeReg16Imm8 ( long x64InstOpcode, long ModRMOpcode, long x64DestReg_RM_Base, char cImmediate8 );
	bool x64EncodeReg32Imm8 ( long x64InstOpcode, long ModRMOpcode, long x64DestReg_RM_Base, char cImmediate8 );
	bool x64EncodeReg64Imm8 ( long x64InstOpcode, long ModRMOpcode, long x64DestReg_RM_Base, char cImmediate8 );
	
	// encode x64 Imm8 instruction but with memory
	bool x64EncodeMem16Imm8 ( long x64InstOpcode, long ModRMOpcode, long BaseAddressReg, long IndexReg, long Scale, long Offset, char cImmediate8 );
	bool x64EncodeMem32Imm8 ( long x64InstOpcode, long ModRMOpcode, long BaseAddressReg, long IndexReg, long Scale, long Offset, char cImmediate8 );
	bool x64EncodeMem64Imm8 ( long x64InstOpcode, long ModRMOpcode, long BaseAddressReg, long IndexReg, long Scale, long Offset, char cImmediate8 );

	
	// encode 16-bit immediate x64 instruction
	bool x64EncodeReg16Imm16 ( long x64InstOpcode, long ModRMOpcode, long x64DestReg, short cImmediate16 );

	// encode a reg-immediate 32 bit x64 instruction into code block
	bool x64EncodeReg32Imm32 ( long x64InstOpcode, long ModRMOpcode, long x64DestReg, long cImmediate32 );

	// encode a reg-immediate 64 bit x64 instruction into code block
	bool x64EncodeReg64Imm32 ( long x64InstOpcode, long ModRMOpcode, long x64DestReg, long cImmediate32 );

	// encode for operation on accumulator
	bool x64EncodeAcc16Imm16 ( long x64InstOpcode, short cImmediate16 );
	bool x64EncodeAcc32Imm32 ( long x64InstOpcode, long cImmediate32 );
	bool x64EncodeAcc64Imm32 ( long x64InstOpcode, long cImmediate32 );

	
	
	// *** These functions work with memory accesses ***
	// ** NOTE ** Do not use sp/esp/rsp or bp/ebp/rbp with these functions until I read the x64 docs more thoroughly!!!

	// encode an x64 instruction that addresses memory
	bool x64EncodeRegMem16 ( long x64InstOpcode, long x64DestReg, long BaseAddressReg, long IndexReg, long Scale, long Offset );
	bool x64EncodeRegMem32 ( long x64InstOpcode, long x64DestReg, long BaseAddressReg, long IndexReg, long Scale, long Offset );
	bool x64EncodeRegMem64 ( long x64InstOpcode, long x64DestReg, long BaseAddressReg, long IndexReg, long Scale, long Offset );

	bool x64EncodeMemImm8 ( long x64InstOpcode, long Mod, char Imm8, long BaseAddressReg, long IndexReg, long Scale, long Offset );
	bool x64EncodeMemImm16 ( long x64InstOpcode, long Mod, short Imm16, long BaseAddressReg, long IndexReg, long Scale, long Offset );
	bool x64EncodeMemImm32 ( long x64InstOpcode, long Mod, long Imm32, long BaseAddressReg, long IndexReg, long Scale, long Offset );
	bool x64EncodeMemImm64 ( long x64InstOpcode, long Mod, long Imm32, long BaseAddressReg, long IndexReg, long Scale, long Offset );
	
	// these are for encoding just immediate values
	bool x64EncodeImmediate8 ( char Imm8 );
	bool x64EncodeImmediate16 ( short Imm16 );
	bool x64EncodeImmediate32 ( long Imm32 );
	bool x64EncodeImmediate64 ( long long Imm64 );

	// encode a return instruction
	bool x64EncodeReturn ( void );
	
	
	
	// *** testing *** encode RIP-offset addressing
	bool x64EncodeRipOffset16 ( long x64InstOpcode, long x64Reg, char* DataAddress, bool bIsSourceReg = false );
	bool x64EncodeRipOffset32 ( long x64InstOpcode, long x64Reg, char* DataAddress, bool bIsSourceReg = false );
	bool x64EncodeRipOffset64 ( long x64InstOpcode, long x64Reg, char* DataAddress, bool bIsSourceReg = false );
	bool x64EncodeRipOffsetImm8 ( long x64InstOpcode, long x64Reg, char* DataAddress, char Imm8, bool bIsSourceReg = false );
	bool x64EncodeRipOffsetImm16 ( long x64InstOpcode, long x64Reg, char* DataAddress, short Imm16, bool bIsSourceReg = false );
	bool x64EncodeRipOffsetImm32 ( long x64InstOpcode, long x64Reg, char* DataAddress, long Imm32, bool bIsSourceReg = false );
	bool x64EncodeRipOffsetImm64 ( long x64InstOpcode, long x64Reg, char* DataAddress, long Imm32, bool bIsSourceReg = false );
	
	bool x64EncodeRipOffset16Imm8 ( long x64InstOpcode, long x64Reg, char* DataAddress, char Imm8, bool bIsSourceReg = false );
	bool x64EncodeRipOffset32Imm8 ( long x64InstOpcode, long x64Reg, char* DataAddress, char Imm8, bool bIsSourceReg = false );
	bool x64EncodeRipOffset64Imm8 ( long x64InstOpcode, long x64Reg, char* DataAddress, char Imm8, bool bIsSourceReg = false );
	
	
	
	// ** Encoding vector/sse instructions ** //

	// encode an avx instruction (64-bit version) with an immediate byte on the end
	bool x64EncodeRegVImm8 ( long L, long w, long pp, long mmmmm, long avxInstOpcode, long REG_R, long vvvv, long RM_B, char cImmediate8 );
	bool x64EncodeRegMemVImm8 ( long L, long w, long pp, long mmmmm, long avxInstOpcode, long REG_R_Dest, long vvvv, long x64RM_B_Base, long x64IndexReg, long Scale, long Offset, char Imm8 );
	
	// encode avx instruction for a single 32-bit load store to/from memory
	bool x64EncodeRegMem32S ( long pp, long mmmmm, long avxInstOpcode, long avxDestSrcReg, long avxBaseReg, long avxIndexReg, long Scale, long Offset );
	bool x64EncodeRegMemV ( long L, long w, long pp, long mmmmm, long avxInstOpcode, long REG_R_Dest, long vvvv, long x64RM_B_Base, long x64IndexReg, long Scale, long Offset );
	bool x64EncodeRegMem256 ( long pp, long mmmmm, long avxInstOpcode, long avxDestSrcReg, long avxBaseReg, long avxIndexReg, long Scale, long Offset );

	// encode an avx instruction (64-bit version) that is just register-register
	bool x64EncodeRegRegV ( long L, long w, long pp, long mmmmm, long avxInstOpcode, long REG_R, long vvvv, long RM_B );

	bool x64EncodeRipOffsetV ( long L, long w, long pp, long mmmmm, long avxInstOpcode, long REG_R, long vvvv, char* DataAddress );
	
	// **** x64 Instuctions **** //
	
	// ** General Purpose Register Instructions ** //
	
	
	// * jump/branch instructions * //
	
	static const int c_iNumOfBranchOffsets = 32768;
	static const int c_iNumOfBranchOffsets_Mask = c_iNumOfBranchOffsets - 1;
	
//	long JmpOffset [ 256 ];
	long BranchOffset [ c_iNumOfBranchOffsets ];
/*
	long JmpOffsetEndCount;
	long JmpOffset_End [ 512 ];

	long BranchOffsetEndCount;
	long BranchOffset_End [ 512 ];
*/

	// jump to address in 64-bit register
	bool JmpReg64 ( long x64Reg );
	bool JmpMem64 ( long long* SrcPtr );

	// these are used to make a short term hop while encoding x64 instructions
	bool Jmp ( long Offset, unsigned long Label );
	bool Jmp_NE ( long Offset, unsigned long Label );
	bool Jmp_E ( long Offset, unsigned long Label );
	bool Jmp_L ( long Offset, unsigned long Label );
	bool Jmp_LE ( long Offset, unsigned long Label );
	bool Jmp_G ( long Offset, unsigned long Label );
	bool Jmp_GE ( long Offset, unsigned long Label );
	bool Jmp_B ( long Offset, unsigned long Label );
	bool Jmp_BE ( long Offset, unsigned long Label );
	bool Jmp_A ( long Offset, unsigned long Label );
	bool Jmp_AE ( long Offset, unsigned long Label );
	
	
	// jump short
	bool Jmp8 ( char Offset, unsigned long Label );
	
	// jump short if equal
	bool Jmp8_E ( char Offset, unsigned long Label );
	
	// jump short if not equal
	bool Jmp8_NE ( char Offset, unsigned long Label );
	
	// jump short if unsigned above (carry flag=0 and zero flag=0)
	bool Jmp8_A ( char Offset, unsigned long Label );

	// jump short if unsigned above or equal (carry flag=0)
	bool Jmp8_AE ( char Offset, unsigned long Label );
	
	// jump short if unsigned below (carry flag=1)
	bool Jmp8_B ( char Offset, unsigned long Label );

	// jump short if unsigned below or equal (carry flag=1 or zero flag=1)
	bool Jmp8_BE ( char Offset, unsigned long Label );

	bool Jmp8_G ( char Offset, unsigned long Label );
	bool Jmp8_GE ( char Offset, unsigned long Label );
	bool Jmp8_L ( char Offset, unsigned long Label );
	bool Jmp8_LE ( char Offset, unsigned long Label );
	
	// jump short if not overflow
	bool Jmp8_NO ( char Offset, unsigned long Label );

	// jump if CX/ECX/RCX is zero
	bool Jmp8_CXZ ( char Offset, unsigned long Label );
	bool Jmp8_ECXZ ( char Offset, unsigned long Label );
	bool Jmp8_RCXZ ( char Offset, unsigned long Label );

	

	// these are used to set the target address of jump once you find out what it is
	bool SetJmpTarget ( unsigned long Label );
	
	// set jump target address for a short jump
	bool SetJmpTarget8 ( unsigned long Label );
	
	// self-optimizing unconditional jump
	bool JMP ( void* Target );
	
	// self-optimizing jump on equal
	bool JMP_E ( void* Target );
	
	// self-optimizing jump on not-equal
	bool JMP_NE ( void* Target );
	
	
	// absolute jump with 32-bit rip-offset
	bool Jmp ( void* Target );
	bool Jmp_B ( void* Target );
	
	
	// absolute call with 32-bit rip-offset
	bool Call ( const void* Target );

//	bool SetJmpTarget_End ( void );

	// mov
	bool MovRegReg16 ( long DestReg, long SrcReg );
	bool MovRegReg32 ( long DestReg, long SrcReg );
	bool MovRegReg64 ( long DestReg, long SrcReg );
	bool MovRegImm8 ( long DestReg, char Imm8 );
	bool MovRegImm16 ( long DestReg, short Imm16 );
	bool MovRegImm32 ( long DestReg, long Imm32 );
	bool MovRegImm64 ( long DestReg, long long Imm64 );
	bool MovReg64Imm32 ( long DestReg, long Imm32 );
	bool MovRegToMem8 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovRegFromMem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovRegToMem16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovRegFromMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovRegToMem32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovRegFromMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovRegToMem64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovRegFromMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	
	bool MovMemImm8 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );

	// self-optimizing instructions
	bool MovReg16ImmX ( long DestReg, short Imm16 );
	bool MovReg32ImmX ( long DestReg, long Imm32 );
	bool MovReg64ImmX ( long DestReg, long long Imm64 );

	
	// *** testing ***
	bool MovRegToMem8 ( char* Address, long SrcReg );
	bool MovRegFromMem8 ( long DestReg, char* Address );
	bool MovRegToMem16 ( short* Address, long SrcReg );
	bool MovRegFromMem16 ( long DestReg, short* Address );
	bool MovRegToMem32 ( long* Address, long SrcReg );
	bool MovRegFromMem32 ( long DestReg, long* Address );
	bool MovRegToMem64 ( long long* Address, long SrcReg );
	bool MovRegFromMem64 ( long DestReg, long long* Address );
	
	inline bool MovMemReg8 ( char* Address, long SrcReg ) { return MovRegToMem8 ( Address, SrcReg ); }
	inline bool MovRegMem8 ( long DestReg, char* Address ) { return MovRegFromMem8 ( DestReg, Address ); }
	inline bool MovMemReg16 ( short* Address, long SrcReg ) { return MovRegToMem16 ( Address, SrcReg ); }
	inline bool MovRegMem16 ( long DestReg, short* Address ) { return MovRegFromMem16 ( DestReg, Address ); }
	inline bool MovMemReg32 ( long* Address, long SrcReg ) { return MovRegToMem32 ( Address, SrcReg ); }
	inline bool MovRegMem32 ( long DestReg, long* Address ) { return MovRegFromMem32 ( DestReg, Address ); }
	inline bool MovMemReg64 ( long long* Address, long SrcReg ) { return MovRegToMem64 ( Address, SrcReg ); }
	inline bool MovRegMem64 ( long DestReg, long long* Address ) { return MovRegFromMem64 ( DestReg, Address ); }

	bool MovMemImm8 ( char* DestPtr, char Imm8 );
	bool MovMemImm16 ( short* DestPtr, short Imm16 );
	bool MovMemImm32 ( long* DestPtr, long Imm32 );
	bool MovMemImm64 ( long long* DestPtr, long Imm32 );

	// load variables from memory //
	inline bool LoadMem8 ( long DstReg, void* MemoryAddress ) { return MovRegFromMem8 ( DstReg, (char*) MemoryAddress ); }
	inline bool LoadMem16 ( long DstReg, void* MemoryAddress ) { return MovRegFromMem16 ( DstReg, (short*) MemoryAddress ); }
	inline bool LoadMem32 ( long DstReg, void* MemoryAddress ) { return MovRegFromMem32 ( DstReg, (long*) MemoryAddress ); }
	inline bool LoadMem64 ( long DstReg, void* MemoryAddress ) { return MovRegFromMem64 ( DstReg, (long long*) MemoryAddress ); }
	
	// store variables to memory //
	inline bool StoreMem8 ( void* MemoryAddress, long SrcReg ) { return MovRegToMem8 ( (char*) MemoryAddress, SrcReg ); }
	inline bool StoreMem16 ( void* MemoryAddress, long SrcReg ) { return MovRegToMem16 ( (short*) MemoryAddress, SrcReg ); }
	inline bool StoreMem32 ( void* MemoryAddress, long SrcReg ) { return MovRegToMem32 ( (long*) MemoryAddress, SrcReg ); }
	inline bool StoreMem64 ( void* MemoryAddress, long SrcReg ) { return MovRegToMem64 ( (long long*) MemoryAddress, SrcReg ); }
	
	// set register to constant //
	//inline bool LoadImm8 ( long DstReg, char Imm8 ) { return MovRegImm8 ( DstReg, Imm8 ); }
	inline bool LoadImm16 ( long DstReg, short Imm16 ) { return MovRegImm16 ( DstReg, Imm16 ); }
	inline bool LoadImm32 ( long DstReg, long Imm32 ) { return MovRegImm32 ( DstReg, Imm32 ); }
	inline bool LoadImm64 ( long DstReg, long long Imm64 ) { return MovRegImm64 ( DstReg, Imm64 ); }

	// set memory to constant //
	//inline bool StoreImm8 ( void* MemoryAddress, char Imm8 ) { return MovMemImm8 ( (char*) MemoryAddress, Imm8 ); }
	inline bool StoreImm16 ( void* MemoryAddress, short Imm16 ) { return MovMemImm16 ( (short*) MemoryAddress, Imm16 ); }
	inline bool StoreImm32 ( void* MemoryAddress, long Imm32 ) { return MovMemImm32 ( (long*) MemoryAddress, Imm32 ); }
	inline bool StoreImm64 ( void* MemoryAddress, long Imm32 ) { return MovMemImm64 ( (long long*) MemoryAddress, Imm32 ); }
	
	
//	bool MovRegToMem32S ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
//	bool MovRegFromMem32S ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );

	// add
	bool AddRegReg16 ( long DestReg, long SrcReg );
	bool AddRegReg32 ( long DestReg, long SrcReg );
	bool AddRegReg64 ( long DestReg, long SrcReg );
	bool AddRegImm16 ( long DestReg, short Imm16 );
	bool AddRegImm32 ( long DestReg, long Imm32 );
	bool AddRegImm64 ( long DestReg, long Imm32 );
	
	bool AddRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AddRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AddRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AddMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AddMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AddMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AddMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AddMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AddMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );

	bool AddRegMem16 ( long DestReg, short* SrcPtr );
	bool AddRegMem32 ( long DestReg, long* SrcPtr );
	bool AddRegMem64 ( long DestReg, long long* SrcPtr );
	bool AddMemReg16 ( short* DestPtr, long SrcReg );
	bool AddMemReg32 ( long* DestPtr, long SrcReg );
	bool AddMemReg64 ( long long* DestPtr, long SrcReg );
	bool AddMemImm16 ( short* DestPtr, short Imm16 );
	bool AddMemImm32 ( long* DestPtr, long Imm32 );
	bool AddMemImm64 ( long long* DestPtr, long Imm32 );
	

	// 8-bit immediate
	bool AddReg16Imm8 ( long DestReg, char Imm8 );
	bool AddReg32Imm8 ( long DestReg, char Imm8 );
	bool AddReg64Imm8 ( long DestReg, char Imm8 );

	bool AddMem16Imm8 ( short* DestPtr, char Imm8 );
	bool AddMem32Imm8 ( long* DestPtr, char Imm8 );
	bool AddMem64Imm8 ( long long* DestPtr, char Imm8 );
	
	// add with accumulator
	bool AddAcc16Imm16 ( short Imm16 );
	bool AddAcc32Imm32 ( long Imm32 );
	bool AddAcc64Imm32 ( long Imm32 );
	
	// self-optimizing versions
	bool AddReg16ImmX ( long DestReg, short ImmX );
	bool AddReg32ImmX ( long DestReg, long ImmX );
	bool AddReg64ImmX ( long DestReg, long ImmX );
	
	// self-optimizing
	bool AddMem16ImmX ( short* DestPtr, short ImmX );
	bool AddMem32ImmX ( long* DestPtr, long ImmX );
	bool AddMem64ImmX ( long long* DestPtr, long ImmX );
	
	
	// adc - add with carry
	
	bool AdcRegReg16 ( long DestReg, long SrcReg );
	bool AdcRegReg32 ( long DestReg, long SrcReg );
	bool AdcRegReg64 ( long DestReg, long SrcReg );
	bool AdcRegImm16 ( long DestReg, short Imm16 );
	bool AdcRegImm32 ( long DestReg, long Imm32 );
	bool AdcRegImm64 ( long DestReg, long Imm32 );
	
	bool AdcRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AdcRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AdcRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AdcMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AdcMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AdcMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AdcMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AdcMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AdcMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );

	bool AdcRegMem16 ( long DestReg, short* SrcPtr );
	bool AdcRegMem32 ( long DestReg, long* SrcPtr );
	bool AdcRegMem64 ( long DestReg, long long* SrcPtr );
	bool AdcMemReg16 ( short* DestPtr, long SrcReg );
	bool AdcMemReg32 ( long* DestPtr, long SrcReg );
	bool AdcMemReg64 ( long long* DestPtr, long SrcReg );
	bool AdcMemImm16 ( short* DestPtr, short Imm16 );
	bool AdcMemImm32 ( long* DestPtr, long Imm32 );
	bool AdcMemImm64 ( long long* DestPtr, long Imm32 );
	

	// 8-bit immediate
	bool AdcReg16Imm8 ( long DestReg, char Imm8 );
	bool AdcReg32Imm8 ( long DestReg, char Imm8 );
	bool AdcReg64Imm8 ( long DestReg, char Imm8 );

	bool AdcMem16Imm8 ( short* DestPtr, char Imm8 );
	bool AdcMem32Imm8 ( long* DestPtr, char Imm8 );
	bool AdcMem64Imm8 ( long long* DestPtr, char Imm8 );
	
	// add with accumulator
	bool AdcAcc16Imm16 ( short Imm16 );
	bool AdcAcc32Imm32 ( long Imm32 );
	bool AdcAcc64Imm32 ( long Imm32 );
	
	// self-optimizing versions
	bool AdcReg16ImmX ( long DestReg, short ImmX );
	bool AdcReg32ImmX ( long DestReg, long ImmX );
	bool AdcReg64ImmX ( long DestReg, long ImmX );
	
	// self-optimizing
	bool AdcMem16ImmX ( short* DestPtr, short ImmX );
	bool AdcMem32ImmX ( long* DestPtr, long ImmX );
	bool AdcMem64ImmX ( long long* DestPtr, long ImmX );
	
	
	// and
	bool AndRegReg16 ( long DestReg, long SrcReg );
	bool AndRegReg32 ( long DestReg, long SrcReg );
	bool AndRegReg64 ( long DestReg, long SrcReg );
	bool AndRegImm16 ( long DestReg, short Imm16 );
	bool AndRegImm32 ( long DestReg, long Imm32 );
	bool AndRegImm64 ( long DestReg, long Imm32 );

	bool AndRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AndRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AndRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AndMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AndMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AndMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AndMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AndMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );
	bool AndMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );
	
	bool AndRegMem16 ( long DestReg, short* SrcPtr );
	bool AndRegMem32 ( long DestReg, long* SrcPtr );
	bool AndRegMem64 ( long DestReg, long long* SrcPtr );
	bool AndMemReg16 ( short* DestPtr, long SrcReg );
	bool AndMemReg32 ( long* DestPtr, long SrcReg );
	bool AndMemReg64 ( long long* DestPtr, long SrcReg );
	bool AndMemImm16 ( short* DestPtr, short Imm16 );
	bool AndMemImm32 ( long* DestPtr, long Imm32 );
	bool AndMemImm64 ( long long* DestPtr, long Imm32 );

	// 8-bit immediate
	bool AndReg16Imm8 ( long DestReg, char Imm8 );
	bool AndReg32Imm8 ( long DestReg, char Imm8 );
	bool AndReg64Imm8 ( long DestReg, char Imm8 );

	bool AndMem16Imm8 ( short* DestPtr, char Imm8 );
	bool AndMem32Imm8 ( long* DestPtr, char Imm8 );
	bool AndMem64Imm8 ( long long* DestPtr, char Imm8 );
	
	// add with accumulator
	bool AndAcc16Imm16 ( short Imm16 );
	bool AndAcc32Imm32 ( long Imm32 );
	bool AndAcc64Imm32 ( long Imm32 );
	
	// self-optimizing versions
	bool AndReg16ImmX ( long DestReg, short ImmX );
	bool AndReg32ImmX ( long DestReg, long ImmX );
	bool AndReg64ImmX ( long DestReg, long ImmX );
	
	// self-optimizing
	bool AndMem16ImmX ( short* DestPtr, short ImmX );
	bool AndMem32ImmX ( long* DestPtr, long ImmX );
	bool AndMem64ImmX ( long long* DestPtr, long ImmX );
	
	
	// bsr - bit scan reverse - leading zero count
	bool BsrRegReg16 ( long DestReg, long SrcReg );
	bool BsrRegReg32 ( long DestReg, long SrcReg );
	bool BsrRegReg64 ( long DestReg, long SrcReg );
	
	bool BsrRegMem16 ( long DestReg, short* SrcPtr );
	bool BsrRegMem32 ( long DestReg, long* SrcPtr );
	bool BsrRegMem64 ( long DestReg, long long* SrcPtr );
	
	
	// bt - bit test - stores specified bit of value in carry flag
	bool BtRegReg16 ( long Reg, long BitSelectReg );
	bool BtRegReg32 ( long Reg, long BitSelectReg );
	bool BtRegReg64 ( long Reg, long BitSelectReg );
	bool BtRegImm16 ( long Reg, char Imm8 );
	bool BtRegImm32 ( long Reg, char Imm8 );
	bool BtRegImm64 ( long Reg, char Imm8 );
	
	// with memory
	bool BtMemImm16 ( short* DestPtr, char Imm8 );
	bool BtMemImm32 ( long* DestPtr, char Imm8 );
	bool BtMemImm64 ( long long* DestPtr, char Imm8 );
	bool BtMemReg16 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtMemReg32 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtMemReg64 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtMemImm16 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtMemImm32 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtMemImm64 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );

	// btc - bit test and compliment
	bool BtcRegReg16 ( long Reg, long BitSelectReg );
	bool BtcRegReg32 ( long Reg, long BitSelectReg );
	bool BtcRegReg64 ( long Reg, long BitSelectReg );
	bool BtcRegImm16 ( long Reg, char Imm8 );
	bool BtcRegImm32 ( long Reg, char Imm8 );
	bool BtcRegImm64 ( long Reg, char Imm8 );
	
	// with memory
	bool BtcMemReg16 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtcMemReg32 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtcMemReg64 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtcMemImm16 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtcMemImm32 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtcMemImm64 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );
	
	// btr - bit test and reset
	bool BtrRegReg16 ( long Reg, long BitSelectReg );
	bool BtrRegReg32 ( long Reg, long BitSelectReg );
	bool BtrRegReg64 ( long Reg, long BitSelectReg );
	bool BtrRegImm16 ( long Reg, char Imm8 );
	bool BtrRegImm32 ( long Reg, char Imm8 );
	bool BtrRegImm64 ( long Reg, char Imm8 );
	
	// with memory
	bool BtrMemReg16 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtrMemReg32 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtrMemReg64 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtrMemImm16 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtrMemImm32 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtrMemImm64 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );

	bool BtrMem32Imm ( long* DestPtr, char Imm8 );
	
	// bts - bit test and set
	bool BtsRegReg16 ( long Reg, long BitSelectReg );
	bool BtsRegReg32 ( long Reg, long BitSelectReg );
	bool BtsRegReg64 ( long Reg, long BitSelectReg );
	bool BtsRegImm16 ( long Reg, char Imm8 );
	bool BtsRegImm32 ( long Reg, char Imm8 );
	bool BtsRegImm64 ( long Reg, char Imm8 );

	// with memory
	bool BtsMemReg16 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtsMemReg32 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtsMemReg64 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtsMemImm16 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtsMemImm32 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );
	bool BtsMemImm64 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );
	
	// cbw, cwde, cdqe - sign extention for RAX - from byte to word, word to double word, or double word to quadword respectively
	bool Cbw ( void );
	bool Cwde ( void );
	bool Cdqe ( void );
	
	// cwd, cdq, cqo - sign extension RAX into RAX:RDX
	bool Cwd ( void );
	bool Cdq ( void );
	bool Cqo ( void );

	// cmov - conditional mov
	bool CmovERegReg16 ( long DestReg, long SrcReg );
	bool CmovNERegReg16 ( long DestReg, long SrcReg );
	bool CmovBRegReg16 ( long DestReg, long SrcReg );
	bool CmovBERegReg16 ( long DestReg, long SrcReg );
	bool CmovARegReg16 ( long DestReg, long SrcReg );
	bool CmovAERegReg16 ( long DestReg, long SrcReg );
	bool CmovERegReg32 ( long DestReg, long SrcReg );
	bool CmovNERegReg32 ( long DestReg, long SrcReg );
	bool CmovBRegReg32 ( long DestReg, long SrcReg );
	bool CmovBERegReg32 ( long DestReg, long SrcReg );
	bool CmovARegReg32 ( long DestReg, long SrcReg );
	bool CmovAERegReg32 ( long DestReg, long SrcReg );
	bool CmovERegReg64 ( long DestReg, long SrcReg );
	bool CmovNERegReg64 ( long DestReg, long SrcReg );
	bool CmovBRegReg64 ( long DestReg, long SrcReg );
	bool CmovBERegReg64 ( long DestReg, long SrcReg );
	bool CmovARegReg64 ( long DestReg, long SrcReg );
	bool CmovAERegReg64 ( long DestReg, long SrcReg );
	
	
	// with memory
	bool CmovERegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmovNERegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmovERegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmovNERegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmovERegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmovNERegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmovBRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );

	bool CmovERegMem16 ( long DestReg, short* Mem16 );
	bool CmovNERegMem16 ( long DestReg, short* Mem16 );
	bool CmovBRegMem16 ( long DestReg, short* Mem16 );
	bool CmovBERegMem16 ( long DestReg, short* Mem16 );
	bool CmovARegMem16 ( long DestReg, short* Mem16 );
	bool CmovAERegMem16 ( long DestReg, short* Mem16 );
	bool CmovERegMem32 ( long DestReg, long* Mem32 );
	bool CmovNERegMem32 ( long DestReg, long* Mem32 );
	bool CmovBRegMem32 ( long DestReg, long* Mem32 );
	bool CmovBERegMem32 ( long DestReg, long* Mem32 );
	bool CmovARegMem32 ( long DestReg, long* Mem32 );
	bool CmovAERegMem32 ( long DestReg, long* Mem32 );
	bool CmovERegMem64 ( long DestReg, long long* Mem64 );
	bool CmovNERegMem64 ( long DestReg, long long* Mem64 );
	bool CmovBRegMem64 ( long DestReg, long long* Mem64 );
	bool CmovBERegMem64 ( long DestReg, long long* Mem64 );
	bool CmovARegMem64 ( long DestReg, long long* Mem64 );
	bool CmovAERegMem64 ( long DestReg, long long* Mem64 );

	
	// signed conditional mov
	
	bool CmovLRegReg16 ( long DestReg, long SrcReg );
	bool CmovLERegReg16 ( long DestReg, long SrcReg );
	bool CmovGRegReg16 ( long DestReg, long SrcReg );
	bool CmovGERegReg16 ( long DestReg, long SrcReg );
	bool CmovLRegReg32 ( long DestReg, long SrcReg );
	bool CmovLERegReg32 ( long DestReg, long SrcReg );
	bool CmovGRegReg32 ( long DestReg, long SrcReg );
	bool CmovGERegReg32 ( long DestReg, long SrcReg );
	bool CmovLRegReg64 ( long DestReg, long SrcReg );
	bool CmovLERegReg64 ( long DestReg, long SrcReg );
	bool CmovGRegReg64 ( long DestReg, long SrcReg );
	bool CmovGERegReg64 ( long DestReg, long SrcReg );

	bool CmovSRegReg16 ( long DestReg, long SrcReg );
	bool CmovSRegReg32 ( long DestReg, long SrcReg );
	bool CmovSRegReg64 ( long DestReg, long SrcReg );
	
	bool CmovSRegMem16 ( long DestReg, short* Mem16 );
	bool CmovSRegMem32 ( long DestReg, long* Mem32 );
	bool CmovSRegMem64 ( long DestReg, long long* Mem64 );

	bool CmovNSRegReg16 ( long DestReg, long SrcReg );
	bool CmovNSRegReg32 ( long DestReg, long SrcReg );
	bool CmovNSRegReg64 ( long DestReg, long SrcReg );
	
	bool CmovNSRegMem16 ( long DestReg, short* Mem16 );
	bool CmovNSRegMem32 ( long DestReg, long* Mem32 );
	bool CmovNSRegMem64 ( long DestReg, long long* Mem64 );
	

	// cmp - compare two values
	bool CmpRegReg16 ( long SrcReg1, long SrcReg2 );
	bool CmpRegReg32 ( long SrcReg1, long SrcReg2 );
	bool CmpRegReg64 ( long SrcReg1, long SrcReg2 );
	bool CmpRegImm16 ( long SrcReg, short Imm16 );
	bool CmpRegImm32 ( long SrcReg, long Imm32 );
	bool CmpRegImm64 ( long SrcReg, long Imm32 );

	bool CmpRegMem8 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmpRegMem16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmpRegMem32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmpRegMem64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmpMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmpMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmpMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmpMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmpMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );
	bool CmpMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );

	bool CmpRegMem16 ( long DestReg, short* SrcPtr );
	bool CmpRegMem32 ( long DestReg, long* SrcPtr );
	bool CmpRegMem64 ( long DestReg, long long* SrcPtr );
	bool CmpMemReg16 ( short* DestPtr, long SrcReg );
	bool CmpMemReg32 ( long* DestPtr, long SrcReg );
	bool CmpMemReg64 ( long long* DestPtr, long SrcReg );
	bool CmpMemImm8 ( char* DestPtr, char Imm8 );
	bool CmpMemImm16 ( short* DestPtr, short Imm16 );
	bool CmpMemImm32 ( long* DestPtr, long Imm32 );
	bool CmpMemImm64 ( long long* DestPtr, long Imm32 );

	// 8-bit immediate
	bool CmpReg16Imm8 ( long DestReg, char Imm8 );
	bool CmpReg32Imm8 ( long DestReg, char Imm8 );
	bool CmpReg64Imm8 ( long DestReg, char Imm8 );

	bool CmpMem16Imm8 ( short* DestPtr, char Imm8 );
	bool CmpMem32Imm8 ( long* DestPtr, char Imm8 );
	bool CmpMem64Imm8 ( long long* DestPtr, char Imm8 );
	
	// add with accumulator
	bool CmpAcc16Imm16 ( short Imm16 );
	bool CmpAcc32Imm32 ( long Imm32 );
	bool CmpAcc64Imm32 ( long Imm32 );
	
	// self-optimizing versions
	bool CmpReg16ImmX ( long DestReg, short ImmX );
	bool CmpReg32ImmX ( long DestReg, long ImmX );
	bool CmpReg64ImmX ( long DestReg, long ImmX );
	
	// self-optimizing
	bool CmpMem16ImmX ( short* DestPtr, short ImmX );
	bool CmpMem32ImmX ( long* DestPtr, long ImmX );
	bool CmpMem64ImmX ( long long* DestPtr, long ImmX );
	
	
	// div - unsigned divide - divides d:a by register, puts quotient in a, remainder in d
	bool DivRegReg16 ( long SrcReg );
	bool DivRegReg32 ( long SrcReg );
	bool DivRegReg64 ( long SrcReg );

	bool DivRegMem16 ( short* SrcPtr );
	bool DivRegMem32 ( long* SrcPtr );
	bool DivRegMem64 ( long long* SrcPtr );

	bool DivRegMem16 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool DivRegMem32 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool DivRegMem64 ( long AddressReg, long IndexReg, long Scale, long Offset );

	// idiv - signed divide
	bool IdivRegReg16 ( long SrcReg );
	bool IdivRegReg32 ( long SrcReg );
	bool IdivRegReg64 ( long SrcReg );

	bool IdivRegMem16 ( short* SrcPtr );
	bool IdivRegMem32 ( long* SrcPtr );
	bool IdivRegMem64 ( long long* SrcPtr );

	bool IdivRegMem16 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool IdivRegMem32 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool IdivRegMem64 ( long AddressReg, long IndexReg, long Scale, long Offset );
	
	// imul - signed multiply
	bool ImulRegReg16 ( long SrcReg );
	bool ImulRegReg32 ( long SrcReg );
	bool ImulRegReg64 ( long SrcReg );

	bool ImulRegMem16 ( short* SrcPtr );
	bool ImulRegMem32 ( long* SrcPtr );
	bool ImulRegMem64 ( long long* SrcPtr );

	bool ImulRegMem16 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool ImulRegMem32 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool ImulRegMem64 ( long AddressReg, long IndexReg, long Scale, long Offset );

	// lea - can be used as 3 operand add instruction
	bool LeaRegRegReg16 ( long DestReg, long Src1Reg, long Src2Reg );
	bool LeaRegRegReg32 ( long DestReg, long Src1Reg, long Src2Reg );
	bool LeaRegRegReg64 ( long DestReg, long Src1Reg, long Src2Reg );
	bool LeaRegRegImm16 ( long DestReg, long SrcReg, long Imm16 );
	bool LeaRegRegImm32 ( long DestReg, long SrcReg, long Imm32 );
	bool LeaRegRegImm64 ( long DestReg, long SrcReg, long Imm32 );

	// loads the address into register using a 4-byte offset
	//bool LeaRegMem32 ( long DestReg, void* SrcPtr );
	bool LeaRegMem64 ( long DestReg, void* SrcPtr );

	
	// movsx - move with sign extension
	bool MovsxReg16Reg8 ( long DestReg, long SrcReg );
	bool MovsxReg32Reg8 ( long DestReg, long SrcReg );
	bool MovsxReg64Reg8 ( long DestReg, long SrcReg );

	bool MovsxReg16Mem8 ( long DestReg, char* SrcPtr );
	bool MovsxReg32Mem8 ( long DestReg, char* SrcPtr );
	bool MovsxReg64Mem8 ( long DestReg, char* SrcPtr );

	bool MovsxReg16Mem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovsxReg32Mem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovsxReg64Mem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );

	bool MovsxReg32Reg16 ( long DestReg, long SrcReg );
	bool MovsxReg64Reg16 ( long DestReg, long SrcReg );
	
	bool MovsxReg32Mem16 ( long DestReg, short* SrcPtr );
	bool MovsxReg64Mem16 ( long DestReg, short* SrcPtr );
	
	
	bool MovsxdReg64Reg32 ( long DestReg, long SrcReg );

	bool MovsxdReg64Mem32 ( long DestReg, long* SrcPtr );
	
	bool MovsxReg32Mem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovsxReg64Mem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovsxdReg64Mem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );

	
	// movzx - move with zero extension
	bool MovzxReg16Reg8 ( long DestReg, long SrcReg );
	bool MovzxReg32Reg8 ( long DestReg, long SrcReg );
	bool MovzxReg64Reg8 ( long DestReg, long SrcReg );

	bool MovzxReg16Mem8 ( long DestReg, char* SrcPtr );
	bool MovzxReg32Mem8 ( long DestReg, char* SrcPtr );
	bool MovzxReg64Mem8 ( long DestReg, char* SrcPtr );

	bool MovzxReg16Mem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovzxReg32Mem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovzxReg64Mem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );

	bool MovzxReg32Reg16 ( long DestReg, long SrcReg );
	bool MovzxReg64Reg16 ( long DestReg, long SrcReg );

	bool MovzxReg32Mem16 ( long DestReg, short* SrcPtr );
	bool MovzxReg64Mem16 ( long DestReg, short* SrcPtr );

	bool MovzxReg32Mem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool MovzxReg64Mem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	
	
	// mul - unsigned multiply
	bool MulRegReg16 ( long SrcReg );
	bool MulRegReg32 ( long SrcReg );
	bool MulRegReg64 ( long SrcReg );
	
	bool MulRegMem16 ( short* SrcPtr );
	bool MulRegMem32 ( long* SrcPtr );
	bool MulRegMem64 ( long long* SrcPtr );
	
	bool MulRegMem16 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool MulRegMem32 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool MulRegMem64 ( long AddressReg, long IndexReg, long Scale, long Offset );

	// neg
	bool NegReg16 ( long DestReg );
	bool NegReg32 ( long DestReg );
	bool NegReg64 ( long DestReg );
	
	bool NegMem16 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool NegMem32 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool NegMem64 ( long AddressReg, long IndexReg, long Scale, long Offset );

	bool NegMem16 ( short* DestPtr );
	bool NegMem32 ( long* DestPtr );
	bool NegMem64 ( long long* DestPtr );

	// not 
	bool NotReg16 ( long DestReg );
	bool NotReg32 ( long DestReg );
	bool NotReg64 ( long DestReg );

	bool NotMem16 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool NotMem32 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool NotMem64 ( long AddressReg, long IndexReg, long Scale, long Offset );

	bool NotMem16 ( short* DestPtr );
	bool NotMem32 ( long* DestPtr );
	bool NotMem64 ( long long* DestPtr );
	
	
	// inc
	bool IncReg16 ( long DestReg );
	bool IncReg32 ( long DestReg );
	bool IncReg64 ( long DestReg );

	bool IncMem16 ( short* DestPtr );
	bool IncMem32 ( long* DestPtr );
	bool IncMem64 ( long long* DestPtr );
	
	
	// dec
	bool DecReg16 ( long DestReg );
	bool DecReg32 ( long DestReg );
	bool DecReg64 ( long DestReg );

	bool DecMem16 ( short* DestPtr );
	bool DecMem32 ( long* DestPtr );
	bool DecMem64 ( long long* DestPtr );

	
	// or
	bool OrRegReg16 ( long DestReg, long SrcReg );
	bool OrRegReg32 ( long DestReg, long SrcReg );
	bool OrRegReg64 ( long DestReg, long SrcReg );
	bool OrRegImm16 ( long DestReg, short Imm16 );
	bool OrRegImm32 ( long DestReg, long Imm32 );
	bool OrRegImm64 ( long DestReg, long Imm32 );

	bool OrRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool OrRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool OrRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool OrMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool OrMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool OrMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool OrMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset );
	bool OrMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );
	bool OrMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );

	bool OrRegMem16 ( long DestReg, short* SrcPtr );
	bool OrRegMem32 ( long DestReg, long* SrcPtr );
	bool OrRegMem64 ( long DestReg, long long* SrcPtr );
	bool OrMemReg8 ( char* DestPtr, int SrcReg );
	bool OrMemReg16 ( short* DestPtr, long SrcReg );
	bool OrMemReg32 ( long* DestPtr, long SrcReg );
	bool OrMemReg64 ( long long* DestPtr, long SrcReg );
	bool OrMemImm16 ( short* DestPtr, short Imm16 );
	bool OrMemImm32 ( long* DestPtr, long Imm32 );
	bool OrMemImm64 ( long long* DestPtr, long Imm32 );

	// 8-bit immediate
	bool OrReg16Imm8 ( long DestReg, char Imm8 );
	bool OrReg32Imm8 ( long DestReg, char Imm8 );
	bool OrReg64Imm8 ( long DestReg, char Imm8 );

	bool OrMem16Imm8 ( short* DestPtr, char Imm8 );
	bool OrMem32Imm8 ( long* DestPtr, char Imm8 );
	bool OrMem64Imm8 ( long long* DestPtr, char Imm8 );
	
	// add with accumulator
	bool OrAcc16Imm16 ( short Imm16 );
	bool OrAcc32Imm32 ( long Imm32 );
	bool OrAcc64Imm32 ( long Imm32 );
	
	// self-optimizing versions
	bool OrReg16ImmX ( long DestReg, short ImmX );
	bool OrReg32ImmX ( long DestReg, long ImmX );
	bool OrReg64ImmX ( long DestReg, long ImmX );
	
	// self-optimizing
	bool OrMem16ImmX ( short* DestPtr, short ImmX );
	bool OrMem32ImmX ( long* DestPtr, long ImmX );
	bool OrMem64ImmX ( long long* DestPtr, long ImmX );

	
	// popcnt - population count
	bool PopCnt32 ( long DestReg, long SrcReg );

	
	// pop - pop register from stack
	bool PopReg16 ( long DestReg );
	bool PopReg32 ( long DestReg );
	bool PopReg64 ( long DestReg );
	
	// push - push register onto stack
	bool PushReg16 ( long SrcReg );
	bool PushReg32 ( long SrcReg );
	bool PushReg64 ( long SrcReg );

	bool PushImm8 ( char Imm8 );
	bool PushImm16 ( short Imm16 );
	bool PushImm32 ( long Imm32 );

	// ret
	bool Ret ( void );
	
	// set - set byte instructions
	
	// unsigned versions
	
	// seta - set if unsigned above
	bool Set_A ( int DestReg );
	bool Set_A ( void* DestPtr8 );
	bool Set_A ( long AddressReg, long IndexReg, long Scale, long Offset );
	
	// setae - set if unsigned above or equal
	bool Set_AE ( int DestReg );
	bool Set_AE ( void* DestPtr8 );
	bool Set_AE ( long AddressReg, long IndexReg, long Scale, long Offset );
	
	// setb - set if unsigned below
	bool Set_B ( int DestReg );
	bool Set_B ( void* DestPtr8 );
	bool Set_B ( long AddressReg, long IndexReg, long Scale, long Offset );
	
	// setbe - set if unsigned below or equal
	bool Set_BE ( int DestReg );
	bool Set_BE ( void* DestPtr8 );
	bool Set_BE ( long AddressReg, long IndexReg, long Scale, long Offset );
	
	// sete - set if equal
	bool Set_E ( int DestReg );
	bool Set_E ( void* DestPtr8 );
	bool Set_E ( long AddressReg, long IndexReg, long Scale, long Offset );
	
	// setne - set if NOT equal
	bool Set_NE ( int DestReg );
	bool Set_NE ( void* DestPtr8 );
	bool Set_NE ( long AddressReg, long IndexReg, long Scale, long Offset );
	
	// signed versions

	// setg - set if signed greater
	bool Set_G ( int DestReg );
	bool Set_G ( void* DestPtr8 );
	bool Set_G ( long AddressReg, long IndexReg, long Scale, long Offset );

	// setge - set if signed greater
	bool Set_GE ( int DestReg );
	bool Set_GE ( void* DestPtr8 );
	bool Set_GE ( long AddressReg, long IndexReg, long Scale, long Offset );

	// setl - set if signed greater
	bool Set_L ( int DestReg );
	bool Set_L ( void* DestPtr8 );
	bool Set_L ( long AddressReg, long IndexReg, long Scale, long Offset );

	// setle - set if signed greater
	bool Set_LE ( int DestReg );
	bool Set_LE ( void* DestPtr8 );
	bool Set_LE ( long AddressReg, long IndexReg, long Scale, long Offset );

	// sets - set if signed
	bool Set_S ( int DestReg );
	bool Set_S ( void* DestPtr8 );
	bool Set_S ( long AddressReg, long IndexReg, long Scale, long Offset );
	
	// setpo - set if parity odd
	bool Set_PO ( int DestReg );
	bool Set_PO ( void* DestPtr8 );
	bool Set_PO ( long AddressReg, long IndexReg, long Scale, long Offset );
	
	
	// setb - set if unsigned below
	bool Setb ( long DestReg );
	
	// setl - set if unsigned less than
	bool Setl ( long DestReg );
	bool Setl ( long AddressReg, long IndexReg, long Scale, long Offset );

	// shl - shift left logical - these shift by the value in register c or by an immediate 8-bit value
	bool ShlRegReg16 ( long DestReg );
	bool ShlRegReg32 ( long DestReg );
	bool ShlRegReg64 ( long DestReg );
	bool ShlRegImm16 ( long DestReg, char Imm8 );
	bool ShlRegImm32 ( long DestReg, char Imm8 );
	bool ShlRegImm64 ( long DestReg, char Imm8 );
	
	bool ShlMemReg32 ( long* DestPtr );
	
	// with memory
	bool ShlMemReg16 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool ShlMemReg32 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool ShlMemReg64 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool ShlMemImm16 ( long AddressReg, long IndexReg, long Scale, long Offset, char Imm8 );
	bool ShlMemImm32 ( long AddressReg, long IndexReg, long Scale, long Offset, char Imm8 );
	bool ShlMemImm64 ( long AddressReg, long IndexReg, long Scale, long Offset, char Imm8 );

	// sar - shift right arithemetic - these shift by the value in register c or by an immediate 8-bit value
	bool SarRegReg16 ( long DestReg );
	bool SarRegReg32 ( long DestReg );
	bool SarRegReg64 ( long DestReg );
	bool SarRegImm16 ( long DestReg, char Imm8 );
	bool SarRegImm32 ( long DestReg, char Imm8 );
	bool SarRegImm64 ( long DestReg, char Imm8 );

	bool SarMemReg32 ( long* DestPtr );
	
	// with memory
	bool SarMemReg16 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool SarMemReg32 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool SarMemReg64 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool SarMemImm16 ( long AddressReg, long IndexReg, long Scale, long Offset, char Imm8 );
	bool SarMemImm32 ( long AddressReg, long IndexReg, long Scale, long Offset, char Imm8 );
	bool SarMemImm64 ( long AddressReg, long IndexReg, long Scale, long Offset, char Imm8 );

	// shr - shift right logical - these shift by the value in register c or by an immediate 8-bit value
	bool ShrRegReg16 ( long DestReg );
	bool ShrRegReg32 ( long DestReg );
	bool ShrRegReg64 ( long DestReg );
	bool ShrRegImm16 ( long DestReg, char Imm8 );
	bool ShrRegImm32 ( long DestReg, char Imm8 );
	bool ShrRegImm64 ( long DestReg, char Imm8 );

	bool ShrMemReg32 ( long* DestPtr );
	
	// ***todo***
	//bool ShrMemImm32 ( long* DestPtr, char Imm8 );
	//bool ShrMemImm32 ( long* DestPtr, char Imm8 );
	
	// with memory
	bool ShrMemReg16 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool ShrMemReg32 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool ShrMemReg64 ( long AddressReg, long IndexReg, long Scale, long Offset );
	bool ShrMemImm16 ( long AddressReg, long IndexReg, long Scale, long Offset, char Imm8 );
	bool ShrMemImm32 ( long AddressReg, long IndexReg, long Scale, long Offset, char Imm8 );
	bool ShrMemImm64 ( long AddressReg, long IndexReg, long Scale, long Offset, char Imm8 );

	// sub
	bool SubRegReg16 ( long DestReg, long SrcReg );
	bool SubRegReg32 ( long DestReg, long SrcReg );
	bool SubRegReg64 ( long DestReg, long SrcReg );
	bool SubRegImm16 ( long DestReg, short Imm16 );
	bool SubRegImm32 ( long DestReg, long Imm32 );
	bool SubRegImm64 ( long DestReg, long Imm32 );

	bool SubRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool SubRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool SubRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool SubMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool SubMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool SubMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool SubMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset );
	bool SubMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );
	bool SubMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );

	bool SubRegMem16 ( long DestReg, short* SrcPtr );
	bool SubRegMem32 ( long DestReg, long* SrcPtr );
	bool SubRegMem64 ( long DestReg, long long* SrcPtr );
	bool SubMemReg16 ( short* DestPtr, long SrcReg );
	bool SubMemReg32 ( long* DestPtr, long SrcReg );
	bool SubMemReg64 ( long long* DestPtr, long SrcReg );
	bool SubMemImm16 ( short* DestPtr, short Imm16 );
	bool SubMemImm32 ( long* DestPtr, long Imm32 );
	bool SubMemImm64 ( long long* DestPtr, long Imm32 );

	// 8-bit immediate
	bool SubReg16Imm8 ( long DestReg, char Imm8 );
	bool SubReg32Imm8 ( long DestReg, char Imm8 );
	bool SubReg64Imm8 ( long DestReg, char Imm8 );

	bool SubMem16Imm8 ( short* DestPtr, char Imm8 );
	bool SubMem32Imm8 ( long* DestPtr, char Imm8 );
	bool SubMem64Imm8 ( long long* DestPtr, char Imm8 );
	
	// add with accumulator
	bool SubAcc16Imm16 ( short Imm16 );
	bool SubAcc32Imm32 ( long Imm32 );
	bool SubAcc64Imm32 ( long Imm32 );
	
	// self-optimizing versions
	bool SubReg16ImmX ( long DestReg, short ImmX );
	bool SubReg32ImmX ( long DestReg, long ImmX );
	bool SubReg64ImmX ( long DestReg, long ImmX );
	
	// self-optimizing
	bool SubMem16ImmX ( short* DestPtr, short ImmX );
	bool SubMem32ImmX ( long* DestPtr, long ImmX );
	bool SubMem64ImmX ( long long* DestPtr, long ImmX );

	
	// test - perform and without writing result and just setting the flags

	bool TestRegReg16 ( long DestReg, long SrcReg );
	bool TestRegReg32 ( long DestReg, long SrcReg );
	bool TestRegReg64 ( long DestReg, long SrcReg );
	
	bool TestRegImm16 ( long DestReg, short Imm16 );
	bool TestRegImm32 ( long DestReg, long Imm32 );
	bool TestRegImm64 ( long DestReg, long Imm32 );
	
	bool TestMemReg16 ( short* DestPtr, long SrcReg );
	bool TestMemReg32 ( long* DestPtr, long SrcReg );
	bool TestMemReg64 ( long long* DestPtr, long SrcReg );

	bool TestMemImm16 ( short* DestPtr, short Imm16 );
	bool TestMemImm32 ( long* DestPtr, long Imm32 );
	bool TestMemImm64 ( long long* DestPtr, long Imm32 );

	bool TestRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool TestRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool TestRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );

	
	// 8-bit immediate
	bool TestReg16Imm8 ( long DestReg, char Imm8 );
	bool TestReg32Imm8 ( long DestReg, char Imm8 );
	bool TestReg64Imm8 ( long DestReg, char Imm8 );

	bool TestMem16Imm8 ( short* DestPtr, char Imm8 );
	bool TestMem32Imm8 ( long* DestPtr, char Imm8 );
	bool TestMem64Imm8 ( long long* DestPtr, char Imm8 );
	
	// test with accumulator
	bool TestAcc16Imm16 ( short Imm16 );
	bool TestAcc32Imm32 ( long Imm32 );
	bool TestAcc64Imm32 ( long Imm32 );
	
	// self-optimizing versions
	bool TestReg16ImmX ( long DestReg, short ImmX );
	bool TestReg32ImmX ( long DestReg, long ImmX );
	bool TestReg64ImmX ( long DestReg, long ImmX );
	
	// self-optimizing
	bool TestMem16ImmX ( short* DestPtr, short ImmX );
	bool TestMem32ImmX ( long* DestPtr, long ImmX );
	bool TestMem64ImmX ( long long* DestPtr, long ImmX );
	
	
	// xchg - exchange register with register or memory
	bool XchgRegReg16 ( long DestReg, long SrcReg );
	bool XchgRegReg32 ( long DestReg, long SrcReg );
	bool XchgRegReg64 ( long DestReg, long SrcReg );

	bool XchgRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool XchgRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool XchgRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );

	// xor
	bool XorRegReg16 ( long DestReg, long SrcReg );
	bool XorRegReg32 ( long DestReg, long SrcReg );
	bool XorRegReg64 ( long DestReg, long SrcReg );
	bool XorRegImm16 ( long DestReg, short Imm16 );
	bool XorRegImm32 ( long DestReg, long Imm32 );
	bool XorRegImm64 ( long DestReg, long Imm32 );

	bool XorRegMem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool XorRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool XorRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool XorRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool XorMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool XorMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool XorMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset );
	bool XorMemImm8 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset );
	bool XorMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset );
	bool XorMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );
	bool XorMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset );

	bool XorRegMem16 ( long DestReg, short* SrcPtr );
	bool XorRegMem32 ( long DestReg, long* SrcPtr );
	bool XorRegMem64 ( long DestReg, long long* SrcPtr );
	bool XorMemReg16 ( short* DestPtr, long SrcReg );
	bool XorMemReg32 ( long* DestPtr, long SrcReg );
	bool XorMemReg64 ( long long* DestPtr, long SrcReg );
	bool XorMemImm16 ( short* DestPtr, short Imm16 );
	bool XorMemImm32 ( long* DestPtr, long Imm32 );
	bool XorMemImm64 ( long long* DestPtr, long Imm32 );

	// 8-bit immediate
	bool XorReg16Imm8 ( long DestReg, char Imm8 );
	bool XorReg32Imm8 ( long DestReg, char Imm8 );
	bool XorReg64Imm8 ( long DestReg, char Imm8 );

	bool XorMem16Imm8 ( short* DestPtr, char Imm8 );
	bool XorMem32Imm8 ( long* DestPtr, char Imm8 );
	bool XorMem64Imm8 ( long long* DestPtr, char Imm8 );
	
	// add with accumulator
	bool XorAcc16Imm16 ( short Imm16 );
	bool XorAcc32Imm32 ( long Imm32 );
	bool XorAcc64Imm32 ( long Imm32 );
	
	// self-optimizing versions
	bool XorReg16ImmX ( long DestReg, short ImmX );
	bool XorReg32ImmX ( long DestReg, long ImmX );
	bool XorReg64ImmX ( long DestReg, long ImmX );
	
	// self-optimizing
	bool XorMem16ImmX ( short* DestPtr, short ImmX );
	bool XorMem32ImmX ( long* DestPtr, long ImmX );
	bool XorMem64ImmX ( long long* DestPtr, long ImmX );
	
	
	// ** SSE/AVX Register Instructions ** //
	
	// movhlps - combine high quadwords of xmm registers, 3rd operand on bottom
	bool movhlps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	// movlhps - combine low quadwords of xmm registers, 3rd operand on top
	bool movlhps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	// pinsr
	bool pinsrb ( long sseDestReg, long sseSrcReg1, long x64SrcReg2, char Imm8 );
	bool pinsrw ( long sseDestReg, long sseSrcReg1, long x64SrcReg2, char Imm8 );
	bool pinsrd ( long sseDestReg, long sseSrcReg1, long x64SrcReg2, char Imm8 );
	bool pinsrq ( long sseDestReg, long sseSrcReg1, long x64SrcReg2, char Imm8 );

	// pextr
	bool pextrb ( long x64DestReg, long sseSrcReg, char Imm8 );
	bool pextrw ( long x64DestReg, long sseSrcReg, char Imm8 );
	bool pextrd ( long x64DestReg, long sseSrcReg, char Imm8 );
	bool pextrq ( long x64DestReg, long sseSrcReg, char Imm8 );
	
	// blendvps
//	bool blendvps ( long sseDestReg, long sseSrcReg1, long x64SrcReg2, long x64SrcReg3 );
	
	// pblendw
	bool pblendw ( long sseDestReg, long sseSrcReg1, long x64SrcReg2, char Imm8 );
	
	bool pblendwregregimm ( long sseDestReg, long sseSrcReg, char Imm8 );
	bool pblendwregmemimm ( long sseDestReg, void* SrcPtr, char Imm8 );
	
	// pblendvb
	bool pblendvbregreg ( long sseDestReg, long sseSrcReg );
	bool pblendvbregmem ( long sseDestReg, void* SrcPtr );
	
	// movaps
	bool movaps128 ( long sseDestReg, long sseSrcReg );
	bool movaps_to_mem128 ( void* DestPtr, long sseSrcReg );
	bool movaps_from_mem128 ( long sseDestReg, void* SrcPtr );
	bool movaps_to_mem128 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool movaps_from_mem128 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	

	// movdqa
	bool vmovdqa_regreg128 ( long sseDestReg, long sseSrcReg );
	bool vmovdqa_regmem128 ( long sseDestReg, void* SrcPtr );
	bool vmovdqa_memreg128 ( void* DestPtr, long sseSrcReg );
	bool vmovdqa_memreg128 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vmovdqa_regmem128 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	
	bool vmovdqa_regreg256 ( long sseDestReg, long sseSrcReg );
	bool vmovdqa_regmem256 ( long sseDestReg, void* SrcPtr );
	bool vmovdqa_memreg256 ( void* DestPtr, long sseSrcReg );
	bool vmovdqa_memreg256 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vmovdqa_regmem256 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	
	//bool movdqa_to_mem128 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	//bool movdqa_from_mem128 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	//bool movdqa256 ( long sseDestReg, long sseSrcReg );
	//bool movdqa_to_mem256 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	//bool movdqa_from_mem256 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	
	bool movdqa_regreg ( long sseDestReg, long sseSrcReg );
	bool movdqa_memreg ( void* DestPtr, long sseSrcReg );
	bool movdqa_regmem ( long sseDestReg, void* SrcPtr );
	bool movdqa_to_mem128 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool movdqa_from_mem128 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	
	
	bool movdqu_regreg ( long sseDestReg, long sseSrcReg );
	bool movdqu_memreg ( void* DestPtr, long sseSrcReg );
	bool movdqu_regmem ( long sseDestReg, void* SrcPtr );
	bool movdqu_to_mem128 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool movdqu_from_mem128 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	
	// move 32-bit value into lower part of xmm register (zero upper bits)
	//bool movd_regmem ( long sseDestReg, long* SrcPtr );

	// move 64-bit value into lower part of xmm register (zero upper bits)
	//bool movq_regmem ( long sseDestReg, long long* SrcPtr );
	
	// movss - move single 32-bit value from memory or to memory
	bool movss_to_mem128 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool movss_from_mem128 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );

	// pabs
	bool vpabsb_regreg128 ( long sseDestReg, long sseSrcReg );
	bool vpabsw_regreg128 ( long sseDestReg, long sseSrcReg );
	bool vpabsd_regreg128 ( long sseDestReg, long sseSrcReg );
	
	bool vpabsb_regmem128 ( long sseDestReg, void* SrcPtr );
	bool vpabsw_regmem128 ( long sseDestReg, void* SrcPtr );
	bool vpabsd_regmem128 ( long sseDestReg, void* SrcPtr );

	bool pabswregreg ( long sseDestReg, long sseSrcReg );
	bool pabswregmem ( long sseDestReg, void* SrcPtr );
	bool pabsdregreg ( long sseDestReg, long sseSrcReg );
	bool pabsdregmem ( long sseDestReg, void* SrcPtr );

	
	// packus
	bool packusdw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool packuswb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool packuswbregreg ( long sseDestReg, long sseSrcReg );
	bool packuswbregmem ( long sseDestReg, void* SrcPtr );
	bool packusdwregreg ( long sseDestReg, long sseSrcReg );
	bool packusdwregmem ( long sseDestReg, void* SrcPtr );
	
	bool packsswbregreg ( long sseDestReg, long sseSrcReg );
	bool packsswbregmem ( long sseDestReg, void* SrcPtr );
	bool packssdwregreg ( long sseDestReg, long sseSrcReg );
	bool packssdwregmem ( long sseDestReg, void* SrcPtr );
	
	// pmovzx
	bool vpmovzxbw_regreg128 ( long sseDestReg, long sseSrcReg );
	bool vpmovzxbw_regmem128 ( long sseDestReg, void* SrcPtr );
	bool vpmovzxbd_regreg128 ( long sseDestReg, long sseSrcReg );
	bool vpmovzxbd_regmem128 ( long sseDestReg, void* SrcPtr );
	bool vpmovzxbq_regreg128 ( long sseDestReg, long sseSrcReg );
	bool vpmovzxbq_regmem128 ( long sseDestReg, void* SrcPtr );
	bool vpmovzxwd_regreg128 ( long sseDestReg, long sseSrcReg );
	bool vpmovzxwd_regmem128 ( long sseDestReg, void* SrcPtr );
	bool vpmovzxwq_regreg128 ( long sseDestReg, long sseSrcReg );
	bool vpmovzxwq_regmem128 ( long sseDestReg, void* SrcPtr );
	bool vpmovzxdq_regreg128 ( long sseDestReg, long sseSrcReg );
	bool vpmovzxdq_regmem128 ( long sseDestReg, void* SrcPtr );

	bool vpmovzxbw_regreg256 ( long sseDestReg, long sseSrcReg );
	bool vpmovzxbw_regmem256 ( long sseDestReg, void* SrcPtr );
	bool vpmovzxbd_regreg256 ( long sseDestReg, long sseSrcReg );
	bool vpmovzxbd_regmem256 ( long sseDestReg, void* SrcPtr );
	bool vpmovzxbq_regreg256 ( long sseDestReg, long sseSrcReg );
	bool vpmovzxbq_regmem256 ( long sseDestReg, void* SrcPtr );
	bool vpmovzxwd_regreg256 ( long sseDestReg, long sseSrcReg );
	bool vpmovzxwd_regmem256 ( long sseDestReg, void* SrcPtr );
	bool vpmovzxwq_regreg256 ( long sseDestReg, long sseSrcReg );
	bool vpmovzxwq_regmem256 ( long sseDestReg, void* SrcPtr );
	bool vpmovzxdq_regreg256 ( long sseDestReg, long sseSrcReg );
	bool vpmovzxdq_regmem256 ( long sseDestReg, void* SrcPtr );
	
	
	
	
	bool pmovzxbwregreg ( long sseDestReg, long sseSrcReg );
	bool pmovzxbwregmem ( long sseDestReg, void* SrcPtr );
	bool pmovzxbdregreg ( long sseDestReg, long sseSrcReg );
	bool pmovzxbdregmem ( long sseDestReg, void* SrcPtr );
	bool pmovzxbqregreg ( long sseDestReg, long sseSrcReg );
	bool pmovzxbqregmem ( long sseDestReg, void* SrcPtr );
	bool pmovzxwdregreg ( long sseDestReg, long sseSrcReg );
	bool pmovzxwdregmem ( long sseDestReg, void* SrcPtr );
	bool pmovzxwqregreg ( long sseDestReg, long sseSrcReg );
	bool pmovzxwqregmem ( long sseDestReg, void* SrcPtr );
	bool pmovzxdqregreg ( long sseDestReg, long sseSrcReg );
	bool pmovzxdqregmem ( long sseDestReg, void* SrcPtr );
	

	// padd
	bool paddb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool paddw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool paddd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool paddbregreg ( long sseDestReg, long sseSrcReg );
	bool paddwregreg ( long sseDestReg, long sseSrcReg );
	bool padddregreg ( long sseDestReg, long sseSrcReg );
	bool paddqregreg ( long sseDestReg, long sseSrcReg );

	bool paddbregmem ( long sseDestReg, void* SrcPtr );
	bool paddwregmem ( long sseDestReg, void* SrcPtr );
	bool padddregmem ( long sseDestReg, void* SrcPtr );
	bool paddqregmem ( long sseDestReg, void* SrcPtr );
	
	// paddus
	bool paddusb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool paddusw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool paddusbregreg ( long sseDestReg, long sseSrcReg );
	bool paddusbregmem ( long sseDestReg, void* SrcPtr );
	bool padduswregreg ( long sseDestReg, long sseSrcReg );
	bool padduswregmem ( long sseDestReg, void* SrcPtr );
	
	// padds - add packed signed values with saturation
	bool paddsb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool paddsw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	bool paddsbregreg ( long sseDestReg, long sseSrcReg );
	bool paddsbregmem ( long sseDestReg, void* SrcPtr );
	bool paddswregreg ( long sseDestReg, long sseSrcReg );
	bool paddswregmem ( long sseDestReg, void* SrcPtr );
	
	// pand
	bool pand ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	bool pandregreg ( long sseDestReg, long sseSrcReg );
	bool pandregmem ( long sseDestReg, void* SrcPtr );
	
	// pandn
	bool pandn ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool pandnregreg ( long sseDestReg, long sseSrcReg );
	bool pandnregmem ( long sseDestReg, void* SrcPtr );

	// todo
	
	// pcmpeq - compare packed integers - sets destination to all 1s if equal, sets to all 0s otherwise
	bool pcmpeqb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool pcmpeqw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool pcmpeqd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool pcmpeqbregreg ( long sseDestReg, long sseSrcReg );
	bool pcmpeqwregreg ( long sseDestReg, long sseSrcReg );
	bool pcmpeqdregreg ( long sseDestReg, long sseSrcReg );

	bool pcmpeqbregmem ( long sseDestReg, void* SrcPtr );
	bool pcmpeqwregmem ( long sseDestReg, void* SrcPtr );
	bool pcmpeqdregmem ( long sseDestReg, void* SrcPtr );

	// pcmpgt - compare packed signed integers for greater than
	bool pcmpgtb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool pcmpgtw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool pcmpgtd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	bool pcmpgtbregreg ( long sseDestReg, long sseSrcReg );
	bool pcmpgtwregreg ( long sseDestReg, long sseSrcReg );
	bool pcmpgtdregreg ( long sseDestReg, long sseSrcReg );

	bool pcmpgtbregmem ( long sseDestReg, void* SrcPtr );
	bool pcmpgtwregmem ( long sseDestReg, void* SrcPtr );
	bool pcmpgtdregmem ( long sseDestReg, void* SrcPtr );
	
	// pmax - get maximum of signed integers
	bool pmaxsw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool pmaxsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool pmaxswregreg ( long sseDestReg, long sseSrcReg );
	bool pmaxsdregreg ( long sseDestReg, long sseSrcReg );

	bool pmaxswregmem ( long sseDestReg, void* SrcPtr );
	bool pmaxsdregmem ( long sseDestReg, void* SrcPtr );
	
	// pmin - get minimum of signed integers
	bool pminsw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool pminsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool pminswregreg ( long sseDestReg, long sseSrcReg );
	bool pminsdregreg ( long sseDestReg, long sseSrcReg );

	bool pminswregmem ( long sseDestReg, void* SrcPtr );
	bool pminsdregmem ( long sseDestReg, void* SrcPtr );
	
	// pmovsxdq - move and sign extend lower 2 32-bit packed integers into 2 64-bit packed integers
	bool pmovsxdq ( long sseDestReg, long sseSrcReg );
	
	bool pmovsxdqregreg ( long sseDestReg, long sseSrcReg );
	bool pmovsxdqregmem ( long sseDestReg, void* SrcPtr );
	
	// pmuludq
	bool pmuludq ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	// por - packed logical or of integers
	bool por ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool porregreg ( long sseDestReg, long sseSrcReg );
	bool porregmem ( long sseDestReg, void* SrcPtr );
	
	// pshuf - shuffle packed values
	bool pshufb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool pshufd ( long sseDestReg, long sseSrcReg, char Imm8 );

	bool pshufbregreg ( long sseDestReg, long sseSrcReg );
	bool pshufbregmem ( long sseDestReg, void* SrcPtr );
	
	bool pshufdregregimm ( long sseDestReg, long sseSrcReg, char Imm8 );
	
	// ***todo*** this instruction crashes for some reason at the moment ***needs fixing***
	bool pshufdregmemimm ( long sseDestReg, void* SrcPtr, char Imm8 );
	
	// pshufhw - packed shuffle high words
	bool pshufhw ( long sseDestReg, long sseSrcReg, char Imm8 );

	bool pshufhwregregimm ( long sseDestReg, long sseSrcReg, char Imm8 );
	
	// ***todo*** this instruction crashes for some reason at the moment ***needs fixing***
	bool pshufhwregmemimm ( long sseDestReg, void* SrcPtr, char Imm8 );
	
	// pshuflw - packed shuffle low words
	bool pshuflw ( long sseDestReg, long sseSrcReg, char Imm8 );

	bool pshuflwregregimm ( long sseDestReg, long sseSrcReg, char Imm8 );
	bool pshuflwregmemimm ( long sseDestReg, void* SrcPtr, char Imm8 );
	
	// psll - packed shift logical left integers
	bool pslldq ( long sseDestReg, long sseSrcReg, char Imm8 );
	bool psllw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool psllw_imm ( long sseDestReg, long sseSrcReg, char Imm8 );
	bool pslld ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool pslld_imm ( long sseDestReg, long sseSrcReg, char Imm8 );
	
	bool psllwregreg ( long sseDestReg, long sseSrcReg );
	bool psllwregmem ( long sseDestReg, void* SrcPtr );
	bool psllwregimm ( long sseDestReg, char Imm8 );

	bool pslldregreg ( long sseDestReg, long sseSrcReg );
	bool pslldregmem ( long sseDestReg, void* SrcPtr );
	bool pslldregimm ( long sseDestReg, char Imm8 );
	
	bool psllqregreg ( long sseDestReg, long sseSrcReg );
	bool psllqregmem ( long sseDestReg, void* SrcPtr );
	bool psllqregimm ( long sseDestReg, char Imm8 );

	// psra - packed shift arithemetic right integers
	bool psraw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool psraw_imm ( long sseDestReg, long sseSrcReg, char Imm8 );
	bool psrad ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool psrad_imm ( long sseDestReg, long sseSrcReg, char Imm8 );
	
	bool psrawregreg ( long sseDestReg, long sseSrcReg );
	bool psrawregmem ( long sseDestReg, void* SrcPtr );
	bool psrawregimm ( long sseDestReg, char Imm8 );

	bool psradregreg ( long sseDestReg, long sseSrcReg );
	bool psradregmem ( long sseDestReg, void* SrcPtr );
	bool psradregimm ( long sseDestReg, char Imm8 );
	
	// psrl - packed shift logical right integers
	bool psrldq ( long sseDestReg, long sseSrcReg, char Imm8 );
	bool psrlw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool psrlw_imm ( long sseDestReg, long sseSrcReg, char Imm8 );
	bool psrld ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool psrld_imm ( long sseDestReg, long sseSrcReg, char Imm8 );
	
	bool psrlwregreg ( long sseDestReg, long sseSrcReg );
	bool psrlwregmem ( long sseDestReg, void* SrcPtr );
	bool psrlwregimm ( long sseDestReg, char Imm8 );

	bool psrldregreg ( long sseDestReg, long sseSrcReg );
	bool psrldregmem ( long sseDestReg, void* SrcPtr );
	bool psrldregimm ( long sseDestReg, char Imm8 );
	
	bool psrlqregreg ( long sseDestReg, long sseSrcReg );
	bool psrlqregmem ( long sseDestReg, void* SrcPtr );
	bool psrlqregimm ( long sseDestReg, char Imm8 );
	
	// psub - subtraction of packed integers
	bool psubb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool psubw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool psubd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool psubbregreg ( long sseDestReg, long sseSrcReg );
	bool psubwregreg ( long sseDestReg, long sseSrcReg );
	bool psubdregreg ( long sseDestReg, long sseSrcReg );
	bool psubqregreg ( long sseDestReg, long sseSrcReg );

	bool psubbregmem ( long sseDestReg, void* SrcPtr );
	bool psubwregmem ( long sseDestReg, void* SrcPtr );
	bool psubdregmem ( long sseDestReg, void* SrcPtr );
	bool psubqregmem ( long sseDestReg, void* SrcPtr );
	
	// psubs - subtract packed integers with signed saturation
	bool psubsb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool psubsw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	// psubus - subtract packed integers with unsigned saturation
	bool psubusb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool psubusw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool psubsbregreg ( long sseDestReg, long sseSrcReg );
	bool psubsbregmem ( long sseDestReg, void* SrcPtr );
	bool psubswregreg ( long sseDestReg, long sseSrcReg );
	bool psubswregmem ( long sseDestReg, void* SrcPtr );
	
	bool psubusbregreg ( long sseDestReg, long sseSrcReg );
	bool psubusbregmem ( long sseDestReg, void* SrcPtr );
	bool psubuswregreg ( long sseDestReg, long sseSrcReg );
	bool psubuswregmem ( long sseDestReg, void* SrcPtr );
	
	// punpckh - unpack from high - first source on bottom
	bool punpckhbw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool punpckhwd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool punpckhdq ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool punpckhqdq ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool punpckhbwregreg ( long sseDestReg, long sseSrcReg );
	bool punpckhbwregmem ( long sseDestReg, void* SrcPtr );
	
	bool punpckhwdregreg ( long sseDestReg, long sseSrcReg );
	bool punpckhwdregmem ( long sseDestReg, void* SrcPtr );

	bool punpckhdqregreg ( long sseDestReg, long sseSrcReg );
	bool punpckhdqregmem ( long sseDestReg, void* SrcPtr );
	
	bool punpckhqdqregreg ( long sseDestReg, long sseSrcReg );
	bool punpckhqdqregmem ( long sseDestReg, void* SrcPtr );
	
	// punpckl - unpack from low - first source on bottom
	bool punpcklbw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool punpcklwd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool punpckldq ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool punpcklqdq ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool punpcklbwregreg ( long sseDestReg, long sseSrcReg );
	bool punpcklbwregmem ( long sseDestReg, void* SrcPtr );
	
	bool punpcklwdregreg ( long sseDestReg, long sseSrcReg );
	bool punpcklwdregmem ( long sseDestReg, void* SrcPtr );

	bool punpckldqregreg ( long sseDestReg, long sseSrcReg );
	bool punpckldqregmem ( long sseDestReg, void* SrcPtr );
	
	bool punpcklqdqregreg ( long sseDestReg, long sseSrcReg );
	bool punpcklqdqregmem ( long sseDestReg, void* SrcPtr );

	// pxor
	bool pxor ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	bool pxorregreg ( long sseDestReg, long sseSrcReg );
	bool pxorregmem ( long sseDestReg, void* SrcPtr );
	
	
	// ptest
	bool ptestregreg ( long sseDestReg, long sseSrcReg );
	bool ptestregmem ( long sseDestReg, void* SrcPtr );
	
	
	// pmuldq - multiplies 2 signed 32-bit values (index 0 and 2) and stores the results as two 64-bit values
	bool pmuldqregreg ( long sseDestReg, long sseSrcReg );
	bool pmuldqregmem ( long sseDestReg, void* SrcPtr );
	
	// pmuludq - unsigned version of pmuldq
	bool pmuludqregreg ( long sseDestReg, long sseSrcReg );
	bool pmuludqregmem ( long sseDestReg, void* SrcPtr );
	
	// pmulld - multiplies 4 signed 32-bit values and stores low result
	bool pmulldregreg ( long sseDestReg, long sseSrcReg );
	bool pmulldregmem ( long sseDestReg, void* SrcPtr );
	
	
	// pmullw - multiply packed signed 16-bit halfwords and store low result
	bool pmullwregreg ( long sseDestReg, long sseSrcReg );
	bool pmullwregmem ( long sseDestReg, void* SrcPtr );
	
	// pmulhw - multiply packed signed 16-bit halfwords and store high result
	bool pmulhwregreg ( long sseDestReg, long sseSrcReg );
	bool pmulhwregmem ( long sseDestReg, void* SrcPtr );
	
	
	// pmaddwd - mulitply halfwords and add adjacent results
	bool pmaddwdregreg ( long sseDestReg, long sseSrcReg );
	bool pmaddwdregmem ( long sseDestReg, void* SrcPtr );
	
	
	//** SSE Floating Point Instructions **//
	
	// extractps
	bool extractps ( long x64DestReg, long sseSrcReg, char Imm8 );
	
	// movmskps
	bool movmskps256 ( long x64DestReg, long sseSrcReg );
	
	bool movmskpsregreg ( long x64DestReg, long sseSrcReg );

	
	// movaps
	bool movaps ( long sseDestReg, long sseSrcReg );
	bool movapd ( long sseDestReg, long sseSrcReg );
	bool movapstomem ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool movapsfrommem ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool movapdtomem ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool movapdfrommem ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	

	// vbroadcastss
	bool vbroadcastss ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vbroadcastsd ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	
	// vmaskmov
	bool vmaskmovpstomem ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, long sseMaskReg );
	bool vmaskmovpdtomem ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, long sseMaskReg );

	// blendvp
	bool blendvps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, long sseSrc3Reg );
	bool blendvpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, long sseSrc3Reg );
	bool blendps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 );
	bool blendpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 );

	bool blendvps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, long sseSrc3Reg );
	bool blendvpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, long sseSrc3Reg );
	bool blendps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 );
	bool blendpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 );
	
	
	// *** SSE instructions *** //
	
	bool movd_to_sse ( long sseDestReg, long x64SrcReg );
	bool movd_from_sse ( long x64DestReg, long sseSrcReg );
	
	bool movq_to_sse ( long sseDestReg, long x64SrcReg );
	bool movq_from_sse ( long x64DestReg, long sseSrcReg );

	bool movd_regmem ( long sseDestReg, long* SrcPtr );
	bool movd_memreg ( long* DstPtr, long sseSrcReg );
	bool movq_regmem ( long sseDestReg, long* SrcPtr );
	bool movq_memreg ( long* DstPtr, long sseSrcReg );


	
	// move and duplicate double floating point values
	bool movddup_regreg ( long sseDestReg, long sseSrcReg );
	bool movddup_regmem ( long sseDestReg, long long* SrcPtr );
	
	

	
	// *** SSE floating point instructions *** //
	
	bool addsd ( long sseDestReg, long sseSrcReg );
	bool subsd ( long sseDestReg, long sseSrcReg );
	bool mulsd ( long sseDestReg, long sseSrcReg );
	bool divsd ( long sseDestReg, long sseSrcReg );
	bool sqrtsd ( long sseDestReg, long sseSrcReg );
	
	bool addpdregreg ( long sseDestReg, long sseSrcReg );
	bool addpdregmem ( long sseDestReg, void* SrcPtr );

	bool mulpdregreg ( long sseDestReg, long sseSrcReg );
	bool mulpdregmem ( long sseDestReg, void* SrcPtr );
	
	// convert with truncation scalar single precision floating point value to general purpose register signed doubleword
	bool cvttss2si ( long x64DestReg, long sseSrcReg );
	bool cvttps2dq_regreg ( long sseDestReg, long sseSrcReg );
	
	// convert doubleword integer to scalar double precision floating point value
	bool cvtsi2sd ( long sseDestReg, long x64SrcReg );
	bool cvtdq2pd_regreg ( long sseDestReg, long sseSrcReg );
	
	
	// *** AVX floating point instructions *** //
	
	// addp
	bool vaddps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool vaddpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool vaddss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool vaddsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool vaddps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vaddpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vaddss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vaddsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );

	// andp
	bool andps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool andpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	bool andps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool andpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );

	// andnp
	bool andnps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool andnpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	bool andnps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool andnpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );

	// cmpxxp
	bool cmpps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 );
	bool cmppd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 );
	bool cmpss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 );
	bool cmpsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 );
	bool cmpeqps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpeqpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpeqss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpeqsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpltps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpltpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpltss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpltsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpgtps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpgtpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpgtss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpgtsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpleps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmplepd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpless ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmplesd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpgeps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpgepd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpgess ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpgesd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpunordps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpunordpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpunordss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpunordsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpordps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpordpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpordss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool cmpordsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	bool cmpps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 );
	bool cmppd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 );
	bool cmpss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 );
	bool cmpsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 );
	bool cmpeqps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpeqpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpeqss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpeqsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpltps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpltpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpltss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpltsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpgtps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpgtpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpgtss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpgtsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpleps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmplepd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpless ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmplesd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpgeps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpgepd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpgess ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpgesd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpunordps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpunordpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpunordss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpunordsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpordps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpordpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpordss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cmpordsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	
	// cvtp
	bool cvtdq2pd ( long sseDestReg, long sseSrc1Reg );
	bool cvtdq2ps ( long sseDestReg, long sseSrc1Reg );
	bool cvtpd2dq ( long sseDestReg, long sseSrc1Reg );
	bool cvtps2dq ( long sseDestReg, long sseSrc1Reg );
	bool cvtps2pd ( long sseDestReg, long sseSrc1Reg );
	bool cvtpd2ps ( long sseDestReg, long sseSrc1Reg );
	
	bool cvtdq2pd ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cvtdq2ps ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cvtpd2dq ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cvtps2dq ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cvtps2pd ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool cvtpd2ps ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );

	// divp
	bool vdivps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool vdivpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool vdivss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool vdivsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	bool vdivps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vdivpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vdivss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vdivsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );

	//maxp
	bool maxps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool maxpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool maxss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool maxsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	bool maxps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool maxpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool maxss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool maxsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );

	// minp
	bool minps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool minpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool minss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool minsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	bool minps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool minpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool minss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool minsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );

	// mulp
	bool vmulps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool vmulpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool vmulss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool vmulsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	bool vmulps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vmulpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vmulss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vmulsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	
	// orp
	bool orps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool orpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );

	bool orps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool orpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );

	// shufp
	bool shufps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 );
	bool shufpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 );

	bool shufps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 );
	bool shufpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 );
	
	// rsqrtp - I don't think this is used
	
	// sqrtp
	bool vsqrtps ( long sseDestReg, long sseSrc1Reg );
	bool vsqrtpd ( long sseDestReg, long sseSrc1Reg );
	bool vsqrtss ( long sseDestReg, long sseSrc1Reg );
	bool vsqrtsd ( long sseDestReg, long sseSrc1Reg );
	
	bool vsqrtps ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vsqrtpd ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vsqrtss ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vsqrtsd ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	
	// subp
	
	bool subpsregreg ( long sseDestReg, long sseSrcReg );
	bool subpdregreg ( long sseDestReg, long sseSrcReg );
	
	bool vsubps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool vsubpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool vsubss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool vsubsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	bool vsubps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vsubpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vsubss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool vsubsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	
	// vperm2f128
	bool vperm2f128 ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 );
	
	// xorp
	bool xorps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	bool xorpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg );
	
	bool xorps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	bool xorpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset );
	
	
	// **** bookmarking **** //
	
	// if you have a jump and don't know how far you'll need to jump, you'll need to store the jump amount later
	// this will return a bookmark for the current byte and advance to the next byte so you can insert the next instruction
	long x64GetBookmark8 ( void );
	
	// this is where you can write to where you put the bookmark - this will be used for short jumps
	bool x64SetBookmark8 ( long Bookmark, char value );

private:

	bool x64EncodeReg16 ( long x64InstOpcode, long ModRMOpcode, long x64Reg );

	// general encode of sib and immediates and stuff for register-memory instructions
	inline bool x64EncodeMem ( long x64DestReg, long BaseAddressReg, long IndexReg, long Scale, long Offset );
	
	inline bool x64Encode16Bit ( void );
	inline bool x64EncodePrefix ( char Prefix );
	
	bool x64EncodeRexReg32 ( long DestReg_RM_Base, long SourceReg_Reg_Opcode );
	bool x64EncodeRexReg64 ( long DestReg_RM_Base, long SourceReg_Reg_Opcode );

	// general encoding of opcode(s)
	inline bool x64EncodeOpcode ( long x64InstOpcode );
};




#endif


