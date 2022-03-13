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


#ifndef _WINDOWS_H_
#define _WINDOWS_H_
#include <windows.h>
#endif


#include "x64Encoder.h"
#include <iostream>

using namespace std;



#define FAST_ENCODE_VALUE16
#define FAST_ENCODE_VALUE32
#define FAST_ENCODE_VALUE64

// my awesome optimal assembly appears to run slow??
//#define ALLOC_PHYSICAL_MEM

// forces it to change memory protection when switching from execute to encode or vice versa
//#define CHANGE_MEM_PROTECT


x64Encoder::x64Encoder ( long SizePerCodeBlock_PowerOfTwo, long CountOfCodeBlocks )
{
	long i;
	long lSpaceToAllocate;
	long long Ret;
	
	// we're not ready yet
	isEncoding = true;
	isReadyForUse = false;
	
	lNumberOfCodeBlocks = CountOfCodeBlocks;
	//lCodeBlockSize = SizePerCodeBlock;
	lCodeBlockSize_PowerOfTwo = SizePerCodeBlock_PowerOfTwo;
	lCodeBlockSize = 1 << SizePerCodeBlock_PowerOfTwo;
	lCodeBlockSize_Mask = lCodeBlockSize - 1;
	
	// allocate space for hash table
	x64CodeHashTable = (long*) malloc( lNumberOfCodeBlocks * sizeof(long) );
	
	// allocate space for the end source cpu addresses table
	x64CodeSourceNextAddress = (long*) malloc( lNumberOfCodeBlocks * sizeof(long) );
	
	// allocate space for alternate stream
	AlternateStream = (char*) malloc( lCodeBlockSize );

	// make space for pre-allocation buffer
//	PreAllocationStream = (char*) malloc( SizePerCodeBlock );

	// make space for instruction stream buffer
//	InstructionStream = (char*) malloc( SizePerCodeBlock );

	// invalidate all code blocks
	for ( i = 0; i < lNumberOfCodeBlocks; i++ )
	{
		x64CodeHashTable [ i ] = INVALIDCODEBLOCK;
	}

	// get the amount of space we need to allocate
	lSpaceToAllocate = lCodeBlockSize * lNumberOfCodeBlocks;
	
	// need to allocate executable memory area

#ifdef ALLOC_PHYSICAL_MEM
	LiveCodeArea = new char [ lSpaceToAllocate ];
#else
	/* **** Start Of Platform Dependent Code ( *SOPDC* ) **** */
#ifdef CHANGE_MEM_PROTECT
	LiveCodeArea = (char*) VirtualAlloc( NULL, lSpaceToAllocate, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE );
#else
	LiveCodeArea = (char*) VirtualAlloc( NULL, lSpaceToAllocate, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );
#endif

	VirtualLock ( (LPVOID) LiveCodeArea, lSpaceToAllocate );
	//Ret = VirtualAlloc( (LPVOID) CodeBlocksStartAddress, lSpaceToAllocate, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
	/* **** End Of Platform Dependent Code **** */
#endif
	
	if ( !LiveCodeArea )
	{
		cout << "\nx64Encoder: There was an error allocating executable area.\n";
	}
	else
	{
		cout << "\nx64Encoder: Executable area allocated successfully. Address=" << hex << (unsigned long long) LiveCodeArea << "\n";
	}
	
	// set location of where code will be decoded to
	//LiveCodeArea = CodeBlocksStartAddress;
	
	if ( !x64CodeArea )
	{
		// there was an error during initialization
		isReadyForUse = false;
	}
	else
	{
		isReadyForUse = true;
	}

	// set the amount of space allocated
	x64TotalSpaceAllocated = lSpaceToAllocate;
	
	// clear stuff for jumping
	for ( i = 0; i < 256; i++ ) BranchOffset [ i ] = -1;
//	JmpOffset = -1;
//	BranchOffset = -1;
//	JmpOffsetEndCount = -1;
//	BranchOffsetEndCount = -1;

	
	// we're done with initializing and can use encoder when we need
	isEncoding = false;
}


x64Encoder::~x64Encoder ( void )
{
#ifdef ALLOC_PHYSICAL_MEM
	delete LiveCodeArea;
#else
	// ** Start Platform Dependent Code ** //
	VirtualUnlock ( (LPVOID) LiveCodeArea, lCodeBlockSize * lNumberOfCodeBlocks );
	VirtualFree( (LPVOID) LiveCodeArea, NULL, MEM_RELEASE );
	// ** End Platform Dependent Code ** //
#endif

	// deallocate space for hash table
	free( (void*) x64CodeHashTable );
	
	// deallocate space for the end source cpu addresses table
	free( (void*) x64CodeSourceNextAddress );
	
	// deallocate space for alternate stream
	free( (void*) AlternateStream );
}

bool x64Encoder::FlushCurrentCodeBlock ( void )
{
	return FlushCodeBlock ( x64CurrentCodeBlockIndex );
}

bool x64Encoder::FlushCodeBlock ( long IndexOfCodeBlock )
{
	long long CurrentCodeBlockAddress;
	
	CurrentCodeBlockAddress = ( ( (long long) x64CodeArea ) + ( (long long) ( IndexOfCodeBlock * lCodeBlockSize ) ) );

	return FlushICache ( CurrentCodeBlockAddress, lCodeBlockSize ); 
}

bool x64Encoder::FlushICache ( long long baseAddress, long NumberOfBytes )
{
	// ** Start Platform Dependent Code ** //
	if ( FlushInstructionCache( GetCurrentProcess (), (LPCVOID) baseAddress, NumberOfBytes ) == 0 ) return false;
	// ** End Platform Dependent Code ** //
	
	return true;
}

long x64Encoder::GetCurrentInstructionOffset ( void )
{
	return x64NextOffset;
}

void x64Encoder::SetCurrentInstructionOffset ( long offset )
{
	x64NextOffset = offset;
}

// get the size of a code block for encoder - should help with determining if there is more space available
long x64Encoder::GetCodeBlockSize ( void )
{
	return lCodeBlockSize;
}

// get the size of the current dynamic code being worked out - should help with determining if there is more space available
// this will only work when currently working in the alternate stream
long x64Encoder::GetAlternateStreamCurrentSize ( void )
{
	return x64NextOffset;
}

// start writing code to alternate stream
void x64Encoder::SwitchToAlternateStream ( void )
{
	// set dynamic code to be encoded into temporary location
	//x64CodeArea = AlternateStream;
	x64CodeArea = (char*) &ullAlternateStream;
	
	// reset position of offset into code area
	x64NextOffset = 0;
}

// start writing code to live code area
void x64Encoder::SwitchToLiveStream ( void )
{
	// store size of code in bytes that is in alternate stream
	lAlternateStreamSize  = x64NextOffset;

	// point current code area into current block of live stream
	x64CodeArea = LiveCodeArea;

	// get offset to current code block in live stream
	x64NextOffset = lCodeBlockSize * x64CurrentCodeBlockIndex;
}

// copy dynamic code from alternate stream to current position in live stream
void x64Encoder::CopyToLiveStream ( void )
{
	long lIndex;

	// just a straight byte copy. will optimize when we get there
	for ( lIndex = 0; lIndex < lAlternateStreamSize; lIndex++ )
	{
		x64CodeArea [ x64NextOffset++ ] = AlternateStream [ lIndex ];
	}
}


char* x64Encoder::Get_CodeBlock_StartPtr ()
{
	// get offset for start of code block
	return & x64CodeArea [ lCodeBlockSize * x64CurrentCodeBlockIndex ];
}

char* x64Encoder::Get_CodeBlock_EndPtr ()
{
	// get offset for end of code block
	return & x64CodeArea [ ( lCodeBlockSize * ( x64CurrentCodeBlockIndex + 1 ) ) - 1 ];
}

// get the start pointer for an arbitrary code block number
char* x64Encoder::Get_XCodeBlock_StartPtr ( long lCodeBlockIndex )
{
	// make sure code block number is valid
	if ( lCodeBlockIndex < lNumberOfCodeBlocks )
	{
		// get offset for start of code block specified
		return & x64CodeArea [ lCodeBlockSize * lCodeBlockIndex ];
	}
	
	// must be an invalid code block index
	return NULL;
}


char* x64Encoder::Get_CodeBlock_CurrentPtr ()
{
	return & x64CodeArea [ x64NextOffset ];
}



bool x64Encoder::StartCodeBlock ( long lCodeBlockIndex )
{

	//long lCodeBlockIndex;

#ifdef CHANGE_MEM_PROTECT	
	unsigned long ulOldProtect;

	// need the previous protection constants used
	ulOldProtect = PAGE_EXECUTE;
#endif
	
	// we have started encoding in an x64 code block
	isEncoding = true;
	
	
	// get the index for the code block to put x64 code in
	//lCodeBlockIndex = ( lSourceStartAddress & ( lNumberOfCodeBlocks - 1 ) );
	
	// set entry in hash table
	//x64CodeHashTable [ lCodeBlockIndex ] = lSourceStartAddress;
	
	// set the current source address
	//x64CurrentSourceAddress = lSourceStartAddress;
	
	// get offset for start of code block
	x64NextOffset = lCodeBlockSize * lCodeBlockIndex;
	
	// get offset for start of current instruction
	x64CurrentStartOffset = x64NextOffset;
	
	// also need to set the current code block index
	x64CurrentCodeBlockIndex = lCodeBlockIndex;
	
	// adding this so it works with older code
	x64CodeArea = LiveCodeArea;
	
#ifdef CHANGE_MEM_PROTECT	
	// change protection to write-only, or read/write
	if ( ! VirtualProtect( (LPVOID) & x64CodeArea [ x64NextOffset ], lCodeBlockSize, PAGE_READWRITE, (PDWORD) & ulOldProtect ) )
	{
		cout << "\nx64Encoder: VirtualProtect call failed at start of code block #" << dec << lCodeBlockIndex;
		return false;
	}
#endif
	
	// clear stuff for jumping
//	JmpOffset = -1;
//	BranchOffset = -1;
//	JmpOffsetEndCount = -1;
//	BranchOffsetEndCount = -1;

	// no problems here
	return true;
}



bool x64Encoder::EndCodeBlock ( void )
{
#ifdef CHANGE_MEM_PROTECT
	unsigned long ulOldProtect;
	
	ulOldProtect = PAGE_READWRITE;
#endif
	
	// set the source end address for the code block - this is actually the next instruction in source cpu memory
	x64CodeSourceNextAddress [ x64CurrentCodeBlockIndex ] = x64CurrentSourceAddress;

	
	// we have to flush the instruction cache for code block before we can use it
	// ??
	FlushCurrentCodeBlock ();

	
#ifdef CHANGE_MEM_PROTECT
	// change protection to something that allows execution
	// change protection to write-only, or read/write
	if ( ! VirtualProtect( (LPVOID) & x64CodeArea [ x64CurrentStartOffset ], lCodeBlockSize, PAGE_EXECUTE, (PDWORD) & ulOldProtect ) )
	{
		cout << "\nx64Encoder: VirtualProtect call failed at end of code block";
		return false;
	}
#endif
	
	// not currently encoding a code block
	isEncoding = false;

	return true;
}



long x64Encoder::x64CurrentCodeBlockSpaceRemaining ( void )
{
	long lNextCodeBlockOffset, lRemainingBytes;
	
	lNextCodeBlockOffset = lCodeBlockSize * ( x64CurrentCodeBlockIndex + 1 );
	
	lRemainingBytes = lNextCodeBlockOffset - x64NextOffset;
	
	return lRemainingBytes;
	
}



bool x64Encoder::x64InvalidateCodeBlock ( long lSourceAddress )
{
	long lCodeBlockIndex;
	
	// get the index for the code block to put x64 code in
	lCodeBlockIndex = ( lSourceAddress & ( lNumberOfCodeBlocks - 1 ) );
	
	// set entry in hash table
	x64CodeHashTable [ lCodeBlockIndex ] = INVALIDCODEBLOCK;
	
	return true;
}




bool x64Encoder::x64IsEncodedAndReady ( long lSourceAddress )
{
	long lCodeBlockIndex;
	
	// get the index for the code block to put x64 code in
	lCodeBlockIndex = ( lSourceAddress & ( lNumberOfCodeBlocks - 1 ) );
	
	// set entry in hash table
	if ( x64CodeHashTable [ lCodeBlockIndex ] == lSourceAddress )
	{
		return true;
	}
	
	return false;
}


// time sensitive function
// set to be inlined
//long long x64Encoder::ExecuteCodeBlock ( long lCodeBlockIndex )
//{
//	// function will return the address of the next instruction to execute
//	typedef long long (*asm_function) ( void );
//	
//	volatile asm_function x64Function;
//
//	//long lCodeBlockIndex;
//	
//	// get the index for the code block to put x64 code in
//	//lCodeBlockIndex = ( lSourceAddress & ( lNumberOfCodeBlocks - 1 ) );
//	
//	//x64Function = (asm_function) ( & ( x64CodeArea [ lCodeBlockIndex * lNumberOfCodeBlocks ] ) );
//	x64Function = (asm_function) ( & ( x64CodeArea [ lCodeBlockIndex << lCodeBlockSize_PowerOfTwo ] ) );
//
//	return x64Function ();
//}



bool x64Encoder::StartInstructionBlock ( void )
{
	x64CurrentSourceCpuInstructionStartOffset = x64NextOffset;
	
	return true;
}



bool x64Encoder::EndInstructionBlock ()
{
	// advance the source cpu instruction pointer
	//x64CurrentSourceAddress += lSourceCpuInstructionSize;
	
	return true;
}



long x64Encoder::x64GetBookmark8 ( void )
{
	long lBookmark;

	lBookmark = x64NextOffset;

	// also advance to where we will start to insert the next instruction
	x64NextOffset++;
	
	return lBookmark;
}


bool x64Encoder::x64SetBookmark8 ( long Bookmark, char value )
{
	x64CodeArea [ Bookmark ] = value;
	
	return true;
}



bool x64Encoder::UndoInstructionBlock ( void )
{
	x64NextOffset = x64CurrentSourceCpuInstructionStartOffset;
	
	return true;
}

bool x64Encoder::x64EncodeImmediate8 ( char Imm8 )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 1 ) return false;

	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = ( Imm8 );

	return true;
}

bool x64Encoder::x64EncodeImmediate16 ( short Imm16 )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 2 ) return false;

#ifdef FAST_ENCODE_VALUE16
	*((short*)(x64CodeArea+x64NextOffset)) = Imm16;
	x64NextOffset += 2;
#else
	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Imm16 & 0xff );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( ( Imm16 >> 8 ) & 0xff );
#endif
	
	return true;
}

bool x64Encoder::x64EncodeImmediate32 ( long Imm32 )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 4 ) return false;

#ifdef FAST_ENCODE_VALUE32
	*((long*)(x64CodeArea+x64NextOffset)) = Imm32;
	x64NextOffset += 4;
#else
	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Imm32 & 0xff );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( ( Imm32 >> 8 ) & 0xff );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( ( Imm32 >> 16 ) & 0xff );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( ( Imm32 >> 24 ) & 0xff );
#endif
	
	return true;
}

bool x64Encoder::x64EncodeImmediate64 ( long long Imm64 )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 8 ) return false;

#ifdef FAST_ENCODE_VALUE64
	*((long long*)(x64CodeArea+x64NextOffset)) = Imm64;
	x64NextOffset += 8;
#else
	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Imm64 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Imm64 >> 8 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Imm64 >> 16 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Imm64 >> 24 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Imm64 >> 32 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Imm64 >> 40 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Imm64 >> 48 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Imm64 >> 56 );
#endif
	
	return true;
}

inline bool x64Encoder::x64Encode16Bit ( void )
{
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

	x64CodeArea [ x64NextOffset++ ] = PREFIX_16BIT;
	
	return true;
}


inline bool x64Encoder::x64EncodePrefix ( char Prefix )
{
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

	x64CodeArea [ x64NextOffset++ ] = Prefix;
	
	return true;
}



bool x64Encoder::x64EncodeRexReg32 ( long DestReg_RM_Base, long SourceReg_Reg_Opcode )
{
	// check if any of the registers require rex opcode
	if ( DestReg_RM_Base > 7 || SourceReg_Reg_Opcode > 7 )
	{
		// make sure there is enough room for this
		if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

		// add rex opcode - we have to put the value in rex.r here since the register used in mov depends on the opcode
		x64CodeArea [ x64NextOffset++ ] = MAKE_REX( 0, SourceReg_Reg_Opcode, 0, DestReg_RM_Base );
	}
	
	return true;
}

bool x64Encoder::x64EncodeRexReg64 ( long DestReg_RM_Base, long SourceReg_Reg_Opcode )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

	x64CodeArea [ x64NextOffset++ ] = MAKE_REX( OPERAND_64BIT, SourceReg_Reg_Opcode, 0, DestReg_RM_Base );
	
	return true;
}


bool x64Encoder::x64EncodeMovImm64 ( long x64DestRegIndex, long long Immediate64 )
{
	x64EncodeRexReg64 ( x64DestRegIndex, 0 );
	
	x64EncodeOpcode ( X64BOP_MOV_IMM + ( x64DestRegIndex & 7 ) );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 8 ) return false;
	
#ifdef FAST_ENCODE_VALUE64
	*((long long*)(x64CodeArea+x64NextOffset)) = Immediate64;
	x64NextOffset += 8;
#else
	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate64 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate64 >> 8 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate64 >> 16 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate64 >> 24 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate64 >> 32 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate64 >> 40 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate64 >> 48 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate64 >> 56 );
#endif

	return true;
}


bool x64Encoder::x64EncodeMovImm32 ( long x64DestRegIndex, long Immediate32 )
{
	x64EncodeRexReg32 ( x64DestRegIndex, 0 );

	x64EncodeOpcode ( X64BOP_MOV_IMM + ( x64DestRegIndex & 7 ) );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 4 ) return false;
	
#ifdef FAST_ENCODE_VALUE32
	*((long*)(x64CodeArea+x64NextOffset)) = Immediate32;
	x64NextOffset += 4;
#else
	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate32 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate32 >> 8 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate32 >> 16 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate32 >> 24 );
#endif
	
	return true;
}

bool x64Encoder::x64EncodeMovImm16 ( long x64DestRegIndex, short Immediate16 )
{
	x64EncodeRexReg32 ( x64DestRegIndex, 0 );

	x64EncodeOpcode ( X64BOP_MOV_IMM + ( x64DestRegIndex & 7 ) );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 2 ) return false;
	
#ifdef FAST_ENCODE_VALUE16
	*((short*)(x64CodeArea+x64NextOffset)) = Immediate16;
	x64NextOffset += 2;
#else
	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate16 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate16 >> 8 );
#endif
	
	return true;
}

bool x64Encoder::x64EncodeMovImm8 ( long x64DestRegIndex, long Immediate8 )
{
	x64EncodeRexReg32 ( x64DestRegIndex, 0 );
	
	x64EncodeOpcode ( X64BOP_MOV_IMM8 + ( x64DestRegIndex & 7 ) );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;
	
	// now we need to drop in the immediate
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( Immediate8 );
	
	return true;
}


bool x64Encoder::x64Encode16 ( long x64InstOpcode )
{
	x64Encode16Bit ();
	return x64EncodeOpcode ( x64InstOpcode );
}

bool x64Encoder::x64Encode32 ( long x64InstOpcode )
{
	return x64EncodeOpcode ( x64InstOpcode );
}

bool x64Encoder::x64Encode64 ( long x64InstOpcode )
{
	x64CodeArea [ x64NextOffset++ ] = CREATE_REX( 1, 0, 0, 0 );
	return x64EncodeOpcode ( x64InstOpcode );
}


bool x64Encoder::x64EncodeReg16 ( long x64InstOpcode, long ModRMOpcode, long x64Reg )
{
	x64Encode16Bit ();
	return x64EncodeReg32 ( x64InstOpcode, ModRMOpcode, x64Reg );
}

bool x64Encoder::x64EncodeReg32 ( long x64InstOpcode, long ModRMOpcode, long x64Reg )
{
	x64EncodeRexReg32 ( x64Reg, 0 );
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

	// now we need to say what registers to use for instruction
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGREG( ModRMOpcode, x64Reg );
	
	return true;
}



bool x64Encoder::x64EncodeReg64 ( long x64InstOpcode, long ModRMOpcode, long x64Reg )
{
	x64EncodeRexReg64 ( x64Reg, 0 );
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

	// now we need to say what registers to use for instruction
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGREG( ModRMOpcode, x64Reg );
	
	return true;
}

bool x64Encoder::x64EncodeRegReg16 ( long x64InstOpcode, long x64DestReg_Reg_Opcode, long x64SourceReg_RM_Base )
{
	x64Encode16Bit ();
	return x64EncodeRegReg32 ( x64InstOpcode, x64DestReg_Reg_Opcode, x64SourceReg_RM_Base );
}


bool x64Encoder::x64EncodeRegReg32 ( long x64InstOpcode, long x64DestReg_Reg_Opcode, long x64SourceReg_RM_Base )
{
	// need to reverse source and dest since I used a certain form of regreg instructions
	x64EncodeRexReg32 ( x64SourceReg_RM_Base, x64DestReg_Reg_Opcode );
	//x64EncodeRexReg32 ( x64DestReg_Reg_Opcode, x64SourceReg_RM_Base );
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;
	
	// now we need to say what registers to use for instruction
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGREG( x64DestReg_Reg_Opcode, x64SourceReg_RM_Base );
	
	return true;
}


bool x64Encoder::x64EncodeRegReg64 ( long x64InstOpcode, long x64DestReg_Reg_Opcode, long x64SourceReg_RM_Base )
{
	// need to reverse source and dest since I used a certain form of regreg instructions
	x64EncodeRexReg64 ( x64SourceReg_RM_Base, x64DestReg_Reg_Opcode );
	//x64EncodeRexReg64 ( x64DestReg_Reg_Opcode, x64SourceReg_RM_Base );

	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

	// now we need to say what registers to use for instruction
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGREG( x64DestReg_Reg_Opcode, x64SourceReg_RM_Base );

	return true;
}



bool x64Encoder::x64EncodeReturn ( void )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

	x64CodeArea [ x64NextOffset++ ] = X64OP_RET;
	
	return true;
}




bool x64Encoder::x64EncodeReg16Imm16 ( long x64InstOpcode, long ModRMOpcode, long x64DestReg, short cImmediate16 )
{
	x64Encode16Bit ();

	x64EncodeRexReg32 ( x64DestReg, 0 );
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 3 ) return false;

	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGOP( REGREG, x64DestReg, ModRMOpcode );

#ifdef FAST_ENCODE_VALUE16
	*((short*)(x64CodeArea+x64NextOffset)) = cImmediate16;
	x64NextOffset += 2;
#else
	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate16 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate16 >> 8 );
#endif

	return true;
}


bool x64Encoder::x64EncodeReg32Imm32 ( long x64InstOpcode, long ModRMOpcode, long x64DestReg, long cImmediate32 )
{
	x64EncodeRexReg32 ( x64DestReg, 0 );
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 5 ) return false;

	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGOP( REGREG, x64DestReg, ModRMOpcode );

#ifdef FAST_ENCODE_VALUE32
	*((long*)(x64CodeArea+x64NextOffset)) = cImmediate32;
	x64NextOffset += 4;
#else
	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 >> 8 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 >> 16 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 >> 24 );
#endif

	return true;
}


bool x64Encoder::x64EncodeReg64Imm32 ( long x64InstOpcode, long ModRMOpcode, long x64DestReg, long cImmediate32 )
{
	x64EncodeRexReg64 ( x64DestReg, 0 );
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 5 ) return false;

	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGOP( REGREG, x64DestReg, ModRMOpcode );

#ifdef FAST_ENCODE_VALUE32
	*((long*)(x64CodeArea+x64NextOffset)) = cImmediate32;
	x64NextOffset += 4;
#else
	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 >> 8 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 >> 16 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 >> 24 );
#endif
	
	return true;
}


bool x64Encoder::x64EncodeAcc16Imm16 ( long x64InstOpcode, short cImmediate16 )
{
	x64Encode16Bit ();
	x64EncodeOpcode ( x64InstOpcode );
	
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 2 ) return false;

#ifdef FAST_ENCODE_VALUE16
	*((short*)(x64CodeArea+x64NextOffset)) = cImmediate16;
	x64NextOffset += 2;
#else
	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate16 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate16 >> 8 );
#endif

	return true;
}

bool x64Encoder::x64EncodeAcc32Imm32 ( long x64InstOpcode, long cImmediate32 )
{
	x64EncodeOpcode ( x64InstOpcode );
	
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 4 ) return false;

#ifdef FAST_ENCODE_VALUE32
	*((long*)(x64CodeArea+x64NextOffset)) = cImmediate32;
	x64NextOffset += 4;
#else
	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 >> 8 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 >> 16 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 >> 24 );
#endif

	return true;
}

bool x64Encoder::x64EncodeAcc64Imm32 ( long x64InstOpcode, long cImmediate32 )
{
	x64EncodeRexReg64 ( 0, 0 );
	x64EncodeOpcode ( x64InstOpcode );
	
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 4 ) return false;

#ifdef FAST_ENCODE_VALUE32
	*((long*)(x64CodeArea+x64NextOffset)) = cImmediate32;
	x64NextOffset += 4;
#else
	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 >> 8 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 >> 16 );
	x64CodeArea [ x64NextOffset++ ] = ( char ) ( cImmediate32 >> 24 );
#endif

	return true;
}





bool x64Encoder::x64EncodeRegMem16 ( long x64InstOpcode, long x64DestReg, long BaseAddressReg, long IndexReg, long Scale, long Offset )
{
	x64Encode16Bit ();
	return x64EncodeRegMem32 ( x64InstOpcode, x64DestReg, BaseAddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::x64EncodeRegMem32 ( long x64InstOpcode, long x64DestReg, long BaseAddressReg, long IndexReg, long Scale, long Offset )
{
	// check if any of the registers require rex opcode
	if ( x64DestReg > 7 || BaseAddressReg > 7 || IndexReg > 7 )
	{
		// make sure there is enough room for this
		if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

		// add rex opcode - we have to put the value in rex.r here since the register used in mov depends on the opcode
		x64CodeArea [ x64NextOffset++ ] = MAKE_REX( 0, x64DestReg, IndexReg, BaseAddressReg );
	}

	x64EncodeOpcode ( x64InstOpcode );
	
	x64EncodeMem ( x64DestReg, BaseAddressReg, IndexReg, Scale, Offset );

	return true;
}

bool x64Encoder::x64EncodeRegMem64 ( long x64InstOpcode, long x64DestReg, long BaseAddressReg, long IndexReg, long Scale, long Offset )
{
	// add rex opcode - we have to put the value in rex.r here since the register used in mov depends on the opcode
	x64CodeArea [ x64NextOffset++ ] = MAKE_REX( OPERAND_64BIT, x64DestReg, IndexReg, BaseAddressReg );
	
	x64EncodeOpcode ( x64InstOpcode );
	
	return x64EncodeMem ( x64DestReg, BaseAddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::x64EncodeMem16Imm8 ( long x64InstOpcode, long ModRMOpcode, long BaseAddressReg, long IndexReg, long Scale, long Offset, char Imm8 )
{
	x64Encode16Bit ();

	// check if any of the registers require rex opcode
	if ( BaseAddressReg > 7 || IndexReg > 7 )
	{
		// make sure there is enough room for this
		if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

		// add rex opcode - we have to put the value in rex.r here since the register used in mov depends on the opcode
		x64CodeArea [ x64NextOffset++ ] = MAKE_REX( 0, 0, IndexReg, BaseAddressReg );
	}

	x64EncodeOpcode ( x64InstOpcode );
	
	x64EncodeMem ( ModRMOpcode, BaseAddressReg, IndexReg, Scale, Offset );
	
	// encode immediate backwards
	return x64EncodeImmediate8 ( Imm8 );
}

bool x64Encoder::x64EncodeMem32Imm8 ( long x64InstOpcode, long ModRMOpcode, long BaseAddressReg, long IndexReg, long Scale, long Offset, char Imm8 )
{
	// check if any of the registers require rex opcode
	if ( BaseAddressReg > 7 || IndexReg > 7 )
	{
		// make sure there is enough room for this
		if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

		// add rex opcode - we have to put the value in rex.r here since the register used in mov depends on the opcode
		x64CodeArea [ x64NextOffset++ ] = MAKE_REX( 0, 0, IndexReg, BaseAddressReg );
	}

	x64EncodeOpcode ( x64InstOpcode );
	
	x64EncodeMem ( ModRMOpcode, BaseAddressReg, IndexReg, Scale, Offset );

	// encode immediate backwards
	return x64EncodeImmediate8 ( Imm8 );
}

bool x64Encoder::x64EncodeMem64Imm8 ( long x64InstOpcode, long ModRMOpcode, long BaseAddressReg, long IndexReg, long Scale, long Offset, char Imm8 )
{
	// add rex opcode - we have to put the value in rex.r here since the register used in mov depends on the opcode
	x64CodeArea [ x64NextOffset++ ] = MAKE_REX( OPERAND_64BIT, 0, IndexReg, BaseAddressReg );
	
	x64EncodeOpcode ( x64InstOpcode );
	
	x64EncodeMem ( ModRMOpcode, BaseAddressReg, IndexReg, Scale, Offset );

	// encode immediate backwards
	return x64EncodeImmediate8 ( Imm8 );
}



bool x64Encoder::x64EncodeMemImm8 ( long x64InstOpcode, long Mod, char Imm8, long BaseAddressReg, long IndexReg, long Scale, long Offset )
{
	// check if any of the registers require rex opcode
	if ( BaseAddressReg > 7 || IndexReg > 7 )
	{
		// make sure there is enough room for this
		if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

		// add rex opcode - we have to put the value in rex.r here since the register used in mov depends on the opcode
		x64CodeArea [ x64NextOffset++ ] = MAKE_REX( 0, 0, IndexReg, BaseAddressReg );
	}

	x64EncodeOpcode ( x64InstOpcode );
	
	x64EncodeMem ( Mod, BaseAddressReg, IndexReg, Scale, Offset );
	
	// encode immediate backwards
	return x64EncodeImmediate8 ( Imm8 );
}

bool x64Encoder::x64EncodeMemImm16 ( long x64InstOpcode, long Mod, short Imm16, long BaseAddressReg, long IndexReg, long Scale, long Offset )
{
	x64Encode16Bit ();

	// check if any of the registers require rex opcode
	if ( BaseAddressReg > 7 || IndexReg > 7 )
	{
		// make sure there is enough room for this
		if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

		// add rex opcode - we have to put the value in rex.r here since the register used in mov depends on the opcode
		x64CodeArea [ x64NextOffset++ ] = MAKE_REX( 0, 0, IndexReg, BaseAddressReg );
	}

	x64EncodeOpcode ( x64InstOpcode );
	
	x64EncodeMem ( Mod, BaseAddressReg, IndexReg, Scale, Offset );
	
	// encode immediate backwards
	return x64EncodeImmediate16 ( Imm16 );
}

bool x64Encoder::x64EncodeMemImm32 ( long x64InstOpcode, long Mod, long Imm32, long BaseAddressReg, long IndexReg, long Scale, long Offset )
{
	// check if any of the registers require rex opcode
	if ( BaseAddressReg > 7 || IndexReg > 7 )
	{
		// make sure there is enough room for this
		if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

		// add rex opcode - we have to put the value in rex.r here since the register used in mov depends on the opcode
		x64CodeArea [ x64NextOffset++ ] = MAKE_REX( 0, 0, IndexReg, BaseAddressReg );
	}

	x64EncodeOpcode ( x64InstOpcode );
	
	x64EncodeMem ( Mod, BaseAddressReg, IndexReg, Scale, Offset );

	// encode immediate backwards
	return x64EncodeImmediate32 ( Imm32 );
}

bool x64Encoder::x64EncodeMemImm64 ( long x64InstOpcode, long Imm32, long Mod, long BaseAddressReg, long IndexReg, long Scale, long Offset )
{
	// add rex opcode - we have to put the value in rex.r here since the register used in mov depends on the opcode
	x64CodeArea [ x64NextOffset++ ] = MAKE_REX( OPERAND_64BIT, 0, IndexReg, BaseAddressReg );
	
	x64EncodeOpcode ( x64InstOpcode );
	
	x64EncodeMem ( Mod, BaseAddressReg, IndexReg, Scale, Offset );

	// encode immediate backwards
	return x64EncodeImmediate32 ( Imm32 );
}

bool x64Encoder::x64EncodeRegMem32S ( long pp, long mmmmm, long avxInstOpcode, long avxDestSrcReg, long avxBaseReg, long avxIndexReg, long Scale, long Offset )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 3 ) return false;

	// add in vex 3-byte prefix
	x64CodeArea [ x64NextOffset++ ] = VEX_START_3BYTE;
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_MID( avxDestSrcReg, avxBaseReg, avxIndexReg, mmmmm);
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_END( 0, 0, 0, pp);

	x64EncodeOpcode ( avxInstOpcode );

	x64EncodeMem ( avxDestSrcReg, avxBaseReg, avxIndexReg, Scale, Offset );

	return true;
}

bool x64Encoder::x64EncodeRegMemV ( long L, long w, long pp, long mmmmm, long avxInstOpcode, long REG_R_Dest, long vvvv, long x64RM_B_Base, long x64IndexReg, long Scale, long Offset )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 3 ) return false;

	// add in vex 3-byte prefix
	x64CodeArea [ x64NextOffset++ ] = VEX_START_3BYTE;
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_MID( REG_R_Dest, x64IndexReg, x64RM_B_Base, mmmmm);
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_END( w, vvvv, L, pp);

	x64EncodeOpcode ( avxInstOpcode );

	x64EncodeMem ( REG_R_Dest, x64RM_B_Base, x64IndexReg, Scale, Offset );

	return true;
}

bool x64Encoder::x64EncodeRegMem256 ( long pp, long mmmmm, long avxInstOpcode, long avxDestSrcReg, long avxBaseReg, long avxIndexReg, long Scale, long Offset )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 3 ) return false;

	// add in vex 3-byte prefix
	x64CodeArea [ x64NextOffset++ ] = VEX_START_3BYTE;
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_MID( avxDestSrcReg, avxIndexReg, avxBaseReg, mmmmm);
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_END( 0, 0, VEX_256BIT, pp);

	x64EncodeOpcode ( avxInstOpcode );

	x64EncodeMem ( avxDestSrcReg, avxBaseReg, avxIndexReg, Scale, Offset );

	return true;
}

bool x64Encoder::x64EncodeReg16Imm8 ( long x64InstOpcode, long ModRMOpcode, long x64DestReg_RM_Base, char cImmediate8 )
{
	x64Encode16Bit ();
	
	x64EncodeRexReg32 ( x64DestReg_RM_Base, 0 );
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 2 ) return false;

	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGOP( REGREG, x64DestReg_RM_Base, ModRMOpcode );

	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = cImmediate8;

	return true;
}

bool x64Encoder::x64EncodeReg32Imm8 ( long x64InstOpcode, long ModRMOpcode, long x64DestReg_RM_Base, char cImmediate8 )
{
	x64EncodeRexReg32 ( x64DestReg_RM_Base, 0 );
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 2 ) return false;

	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGOP( REGREG, x64DestReg_RM_Base, ModRMOpcode );

	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = cImmediate8;

	return true;
}


bool x64Encoder::x64EncodeReg64Imm8 ( long x64InstOpcode, long ModRMOpcode, long x64DestReg_RM_Base, char cImmediate8 )
{
	x64EncodeRexReg64 ( x64DestReg_RM_Base, 0 );

	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 2 ) return false;

	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGOP( REGREG, x64DestReg_RM_Base, ModRMOpcode );

	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = cImmediate8;
	
	return true;
}

bool x64Encoder::x64EncodeRegVImm8 ( long L, long w, long pp, long mmmmm, long avxInstOpcode, long REG_R, long vvvv, long RM_B, char cImmediate8 )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 3 ) return false;

	// add in vex 3-byte prefix
	x64CodeArea [ x64NextOffset++ ] = VEX_START_3BYTE;
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_MID( REG_R, 0, RM_B, mmmmm );
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_END( w, vvvv, L, pp );
	
	x64EncodeOpcode ( avxInstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 2 ) return false;

	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGREG( REG_R, RM_B );

	// now we need to drop in the immediate, but backwards
	x64CodeArea [ x64NextOffset++ ] = cImmediate8;
	
	return true;
}

bool x64Encoder::x64EncodeRegMemVImm8 ( long L, long w, long pp, long mmmmm, long avxInstOpcode, long REG_R_Dest, long vvvv, long x64RM_B_Base, long x64IndexReg, long Scale, long Offset, char Imm8 )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 3 ) return false;

	// add in vex 3-byte prefix
	x64CodeArea [ x64NextOffset++ ] = VEX_START_3BYTE;
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_MID( REG_R_Dest, x64IndexReg, x64RM_B_Base, mmmmm);
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_END( w, vvvv, L, pp);

	x64EncodeOpcode ( avxInstOpcode );

	x64EncodeMem ( REG_R_Dest, x64RM_B_Base, x64IndexReg, Scale, Offset );

	if ( x64CurrentCodeBlockSpaceRemaining() < 1 ) return false;

	x64CodeArea [ x64NextOffset++ ] = Imm8;

	return true;
}


/*
bool x64Encoder::x64EncodeRegMemV ( long L, long w, long pp, long mmmmm, long avxInstOpcode, long avxDestReg, long avxSourceReg, long avxBaseReg, long avxIndexReg, long Scale, long Offset )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 3 ) return false;

	// add in vex 3-byte prefix
	x64CodeArea [ x64NextOffset++ ] = VEX_START_3BYTE;
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_MID( avxDestReg, avxIndexReg, 0, mmmmm);
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_END( w, avxSourceReg, L, pp);

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;
	
	x64EncodeOpcode ( avxInstOpcode );

	x64EncodeMem ( avxDestReg, avxBaseReg, avxIndexReg, Scale, Offset );

	return true;
}
*/

bool x64Encoder::x64EncodeRegRegV ( long L, long w, long pp, long mmmmm, long avxInstOpcode, long REG_R, long vvvv, long RM_B )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 3 ) return false;

	// add in vex 3-byte prefix
	x64CodeArea [ x64NextOffset++ ] = VEX_START_3BYTE;
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_MID( REG_R, 0, RM_B, mmmmm );
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_END( w, vvvv, L, pp);

	// encode the opcode
	x64EncodeOpcode ( avxInstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;
	
	// now we need to say what registers to use for instruction
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGREG( REG_R, RM_B );

	return true;
}

bool x64Encoder::x64EncodeRipOffsetV ( long L, long w, long pp, long mmmmm, long avxInstOpcode, long REG_R, long vvvv, char* DataAddress )
{
	long Offset;
	
	// will leave rex opcode off for now
	/*
	if ( bIsSourceReg )
	{
		x64EncodeRexReg32 ( 0, x64Reg );
	}
	else
	{
		x64EncodeRexReg32 ( x64Reg, 0 );
	}
	*/

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() < 3 ) return false;

	// add in vex 3-byte prefix
	x64CodeArea [ x64NextOffset++ ] = VEX_START_3BYTE;
	//x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_MID( REG_R, 0, RM_B, mmmmm );
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_MID( REG_R, 0, 0, mmmmm );
	x64CodeArea [ x64NextOffset++ ] = VEX_3BYTE_END( w, vvvv, L, pp);

	// encode the opcode
	x64EncodeOpcode ( avxInstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;
	
	// now we need to say what registers to use for instruction
	//x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGREG( REG_R, RM_B );

	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, REG_R, RMBASE_USINGRIP );
	
	// get offset to data
	Offset = (long) ( DataAddress - ( & ( x64CodeArea [ x64NextOffset + 4 ] ) ) );
	
	//cout << hex << "\nOffset=" << Offset << "\n";
	
	return x64EncodeImmediate32 ( Offset );
}







bool x64Encoder::x64EncodeRipOffset16 ( long x64InstOpcode, long x64DestReg, char* DataAddress, bool bIsSourceReg )
{
	x64Encode16Bit ();
	return x64EncodeRipOffset32 ( x64InstOpcode, x64DestReg, DataAddress, bIsSourceReg );
}


// added the "bToMem" argument because it turns out that this was only handling cases where register is the destination register
bool x64Encoder::x64EncodeRipOffset32 ( long x64InstOpcode, long x64Reg, char* DataAddress, bool bIsSourceReg )
{
	long Offset;
	
	if ( bIsSourceReg )
	{
		x64EncodeRexReg32 ( 0, x64Reg );
	}
	else
	{
		x64EncodeRexReg32 ( x64Reg, 0 );
	}
	

	x64EncodeOpcode ( x64InstOpcode );
	
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64Reg, RMBASE_USINGRIP );
	
	// get offset to data
	Offset = (long) ( DataAddress - ( & ( x64CodeArea [ x64NextOffset + 4 ] ) ) );
	
	//cout << hex << "\nOffset=" << Offset << "\n";
	
	return x64EncodeImmediate32 ( Offset );
}




bool x64Encoder::x64EncodeRipOffset64 ( long x64InstOpcode, long x64Reg, char* DataAddress, bool bIsSourceReg )
{
	long Offset;
	
	if ( bIsSourceReg )
	{
		x64EncodeRexReg64 ( 0, x64Reg );
	}
	else
	{
		x64EncodeRexReg64 ( x64Reg, 0 );
	}
	
	/*
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

	// add rex opcode - we have to put the value in rex.r here since the register used in mov depends on the opcode
	x64CodeArea [ x64NextOffset++ ] = MAKE_REX( OPERAND_64BIT, x64DestReg, 0, 0 );
	*/

	x64EncodeOpcode ( x64InstOpcode );
	
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64Reg, RMBASE_USINGRIP );
	
	// get offset to data
	Offset = (long) ( DataAddress - ( & ( x64CodeArea [ x64NextOffset + 4 ] ) ) );
	
	return x64EncodeImmediate32 ( Offset );
}


bool x64Encoder::x64EncodeRipOffsetImm8 ( long x64InstOpcode, long x64Reg, char* DataAddress, char Imm8, bool bIsSourceReg )
{
	long Offset;
	
	if ( bIsSourceReg )
	{
		x64EncodeRexReg32 ( 0, x64Reg );
	}
	else
	{
		x64EncodeRexReg32 ( x64Reg, 0 );
	}
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;
	
	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64Reg, RMBASE_USINGRIP );
	
	// get offset to data
	Offset = (long) ( DataAddress - ( & ( x64CodeArea [ x64NextOffset + 5 ] ) ) );
	
	//cout << hex << "\nOffset=" << Offset << "\n";
	
	x64EncodeImmediate32 ( Offset );
	return x64EncodeImmediate8 ( Imm8 );
	// needs to be fixed
	/*
	long Offset;
	
	x64EncodeRipOffset32 ( x64InstOpcode, x64DestReg, DataAddress );
	
	// get offset to data
	Offset = (long) ( DataAddress - ( & ( x64CodeArea [ x64NextOffset + 5 ] ) ) );
	
	x64EncodeImmediate32 ( Offset );
	return x64EncodeImmediate8 ( Imm8 );
	*/
}

bool x64Encoder::x64EncodeRipOffsetImm16 ( long x64InstOpcode, long x64Reg, char* DataAddress, short Imm16, bool bIsSourceReg )
{
	long Offset;
	
	x64Encode16Bit ();
	
	if ( bIsSourceReg )
	{
		x64EncodeRexReg32 ( 0, x64Reg );
	}
	else
	{
		x64EncodeRexReg32 ( x64Reg, 0 );
	}
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;
	
	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64Reg, RMBASE_USINGRIP );
	
	// get offset to data
	Offset = (long) ( DataAddress - ( & ( x64CodeArea [ x64NextOffset + 6 ] ) ) );
	
	//cout << hex << "\nOffset=" << Offset << "\n";
	
	x64EncodeImmediate32 ( Offset );
	return x64EncodeImmediate16 ( Imm16 );
}

bool x64Encoder::x64EncodeRipOffsetImm32 ( long x64InstOpcode, long x64Reg, char* DataAddress, long Imm32, bool bIsSourceReg )
{
	long Offset;
	
	if ( bIsSourceReg )
	{
		x64EncodeRexReg32 ( 0, x64Reg );
	}
	else
	{
		x64EncodeRexReg32 ( x64Reg, 0 );
	}
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;
	
	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64Reg, RMBASE_USINGRIP );
	
	// get offset to data
	Offset = (long) ( DataAddress - ( & ( x64CodeArea [ x64NextOffset + 8 ] ) ) );
	
	//cout << hex << "\nOffset=" << Offset << "\n";
	
	x64EncodeImmediate32 ( Offset );
	return x64EncodeImmediate32 ( Imm32 );
}

bool x64Encoder::x64EncodeRipOffsetImm64 ( long x64InstOpcode, long x64Reg, char* DataAddress, long Imm32, bool bIsSourceReg )
{
	long Offset;
	
	if ( bIsSourceReg )
	{
		x64EncodeRexReg64 ( 0, x64Reg );
	}
	else
	{
		x64EncodeRexReg64 ( x64Reg, 0 );
	}
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;
	
	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64Reg, RMBASE_USINGRIP );
	
	// get offset to data
	Offset = (long) ( DataAddress - ( & ( x64CodeArea [ x64NextOffset + 8 ] ) ) );
	
	//cout << hex << "\nOffset=" << Offset << "\n";
	
	x64EncodeImmediate32 ( Offset );
	return x64EncodeImmediate32 ( Imm32 );
}


// for instructions that take an IMM8 argument
bool x64Encoder::x64EncodeRipOffset16Imm8 ( long x64InstOpcode, long x64Reg, char* DataAddress, char Imm8, bool bIsSourceReg )
{
	long Offset;
	
	x64Encode16Bit ();
	
	if ( bIsSourceReg )
	{
		x64EncodeRexReg32 ( 0, x64Reg );
	}
	else
	{
		x64EncodeRexReg32 ( x64Reg, 0 );
	}
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;
	
	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64Reg, RMBASE_USINGRIP );
	
	// get offset to data
	Offset = (long) ( DataAddress - ( & ( x64CodeArea [ x64NextOffset + 5 ] ) ) );
	
	x64EncodeImmediate32 ( Offset );
	return x64EncodeImmediate8 ( Imm8 );
}


bool x64Encoder::x64EncodeRipOffset32Imm8 ( long x64InstOpcode, long x64Reg, char* DataAddress, char Imm8, bool bIsSourceReg )
{
	long Offset;
	
	if ( bIsSourceReg )
	{
		x64EncodeRexReg32 ( 0, x64Reg );
	}
	else
	{
		x64EncodeRexReg32 ( x64Reg, 0 );
	}
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;
	
	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64Reg, RMBASE_USINGRIP );
	
	// get offset to data
	Offset = (long) ( DataAddress - ( & ( x64CodeArea [ x64NextOffset + 5 ] ) ) );
	
	x64EncodeImmediate32 ( Offset );
	return x64EncodeImmediate8 ( Imm8 );
}

bool x64Encoder::x64EncodeRipOffset64Imm8 ( long x64InstOpcode, long x64Reg, char* DataAddress, char Imm8, bool bIsSourceReg )
{
	long Offset;
	
	if ( bIsSourceReg )
	{
		x64EncodeRexReg64 ( 0, x64Reg );
	}
	else
	{
		x64EncodeRexReg64 ( x64Reg, 0 );
	}
	
	x64EncodeOpcode ( x64InstOpcode );

	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;
	
	// add in modr/m
	x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64Reg, RMBASE_USINGRIP );
	
	// get offset to data
	Offset = (long) ( DataAddress - ( & ( x64CodeArea [ x64NextOffset + 5 ] ) ) );
	
	x64EncodeImmediate32 ( Offset );
	return x64EncodeImmediate8 ( Imm8 );
}



inline bool x64Encoder::x64EncodeMem ( long x64DestReg, long BaseAddressReg, long IndexReg, long Scale, long Offset )
{
	// check the size of the offset
	if ( Offset == 0 )
	{
	
		if ( IndexReg == NO_INDEX && ( BaseAddressReg & 0x7 ) != RMBASE_USINGSIB )
		{
			// make sure there is enough room for this
			if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

			// add in modr/m
			x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64DestReg, BaseAddressReg );
		}
		else
		{
			// make sure there is enough room for this
			if ( x64CurrentCodeBlockSpaceRemaining() < 2 ) return false;

			// add in modr/m
			x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64DestReg, RMBASE_USINGSIB );

			// add in the sib byte
			// with no offset I guess there is no sib byte
			x64CodeArea [ x64NextOffset++ ] = MAKE_SIB( Scale, IndexReg, BaseAddressReg );
		}

	}
	else if ( Offset <= 127 && Offset >= -128 )
	{
		// we can use a single byte for the displacement here
		
		if ( IndexReg == NO_INDEX && ( BaseAddressReg & 0x7 ) != RMBASE_USINGSIB )
		{
			// make sure there is enough room for this
			if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

			// add in modr/m
			x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_OFFSET8, x64DestReg, BaseAddressReg );
		}
		else
		{
			// make sure there is enough room for this
			if ( x64CurrentCodeBlockSpaceRemaining() < 2 ) return false;

			// add in modr/m
			x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_OFFSET8, x64DestReg, RMBASE_USINGSIB );

			// add in the sib byte
			// with no offset I guess there is no sib byte
			x64CodeArea [ x64NextOffset++ ] = MAKE_SIB( Scale, IndexReg, BaseAddressReg );
		}

		// make sure there is enough room for this
		if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

		// now we need to drop in the offset, but backwards
		x64CodeArea [ x64NextOffset++ ] = ( char ) ( Offset );

	}
	else
	{
		if ( IndexReg == NO_INDEX && ( BaseAddressReg & 0x7 ) != RMBASE_USINGSIB )
		{
			// make sure there is enough room for this
			if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

			// add in modr/m
			x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_OFFSET32, x64DestReg, BaseAddressReg );
		}
		else
		{
			// make sure there is enough room for this
			if ( x64CurrentCodeBlockSpaceRemaining() < 2 ) return false;

			// add in modr/m
			x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_OFFSET32, x64DestReg, RMBASE_USINGSIB );

			// add in the sib byte
			// with no offset I guess there is no sib byte
			x64CodeArea [ x64NextOffset++ ] = MAKE_SIB( Scale, IndexReg, BaseAddressReg );
		}

		// make sure there is enough room for this
		if ( x64CurrentCodeBlockSpaceRemaining() < 4 ) return false;

		// now we need to drop in the immediate, but backwards
		x64CodeArea [ x64NextOffset++ ] = ( char ) ( Offset );
		x64CodeArea [ x64NextOffset++ ] = ( char ) ( Offset >> 8 );
		x64CodeArea [ x64NextOffset++ ] = ( char ) ( Offset >> 16 );
		x64CodeArea [ x64NextOffset++ ] = ( char ) ( Offset >> 24 );

	}
	
	return true;
}


inline bool x64Encoder::x64EncodeOpcode ( long x64InstOpcode )
{
	// make sure there is enough room for this
	if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

	// add in opcode
	x64CodeArea [ x64NextOffset++ ] = GETOPCODE1( x64InstOpcode );
	
	// if there is another opcode, then add it in
	if ( GETOPCODE2( x64InstOpcode ) )
	{
		// make sure there is enough room for this
		if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

		// add in opcode
		x64CodeArea [ x64NextOffset++ ] = GETOPCODE2( x64InstOpcode );
		
		if ( GETOPCODE3( x64InstOpcode ) )
		{
			// make sure there is enough room for this
			if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

			// add in opcode
			x64CodeArea [ x64NextOffset++ ] = GETOPCODE3( x64InstOpcode );
			
			if ( GETOPCODE4( x64InstOpcode ) )
			{
				// make sure there is enough room for this
				if ( x64CurrentCodeBlockSpaceRemaining() == 0 ) return false;

				// add in opcode
				// note: needed to make 0xff represent 0x00 for the fourth byte in opcode for now
				if ( GETOPCODE4( x64InstOpcode ) == 0xff )
				{
					x64CodeArea [ x64NextOffset++ ] = 0;
				}
				else
				{
					x64CodeArea [ x64NextOffset++ ] = GETOPCODE4( x64InstOpcode );
				}
			}
		}
	}
	
	return true;
}




// **** x64 Instructions **** //


// ** popcnt ** //

bool x64Encoder::PopCnt32 ( long DestReg, long SrcReg )
{
	x64EncodePrefix ( X64OP1_POPCNT );
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP2_POPCNT, X64OP3_POPCNT ), DestReg, SrcReg );
}


// ** mov ** //

bool x64Encoder::MovRegReg16 ( long DestReg, long SrcReg )
{
	if ( DestReg == SrcReg ) return true;	// if both regs are the same, then no need to encode instruction
	x64Encode16Bit ();
	return x64EncodeRegReg32 ( X64OP_MOV, DestReg, SrcReg );
}

bool x64Encoder::MovRegReg32 ( long DestReg, long SrcReg )
{
	if ( DestReg == SrcReg ) return true;	// if both regs are the same, then no need to encode instruction
	return x64EncodeRegReg32 ( X64OP_MOV, DestReg, SrcReg );
}

bool x64Encoder::MovRegReg64 ( long DestReg, long SrcReg )
{
	if ( DestReg == SrcReg ) return true;	// if both regs are the same, then no need to encode instruction
	return x64EncodeRegReg64 ( X64OP_MOV, DestReg, SrcReg );
}

bool x64Encoder::MovRegImm8 ( long DestReg, char Imm8 )
{
	return x64EncodeMovImm8 ( DestReg, Imm8 );
}

bool x64Encoder::MovRegImm16 ( long DestReg, short Imm16 )
{
	x64Encode16Bit ();
	return x64EncodeMovImm16 ( DestReg, Imm16 );
}

bool x64Encoder::MovRegImm32 ( long DestReg, long Imm32 )
{
	return x64EncodeMovImm32 ( DestReg, Imm32 );
}

bool x64Encoder::MovRegImm64 ( long DestReg, long long Imm64 )
{
	return x64EncodeMovImm64 ( DestReg, Imm64 );
}

bool x64Encoder::MovReg64Imm32 ( long DestReg, long Imm32 )
{
	return x64EncodeReg64Imm32 ( X64OP_MOV_IMM, MODRM_MOV_IMM, DestReg, Imm32 );
}



bool x64Encoder::MovReg32ImmX ( long DestReg, long Imm32 )
{
	if ( !Imm32 )
	{
		return XorRegReg32 ( DestReg, DestReg );
	}
	
	return x64EncodeMovImm32 ( DestReg, Imm32 );
}

bool x64Encoder::MovReg64ImmX ( long DestReg, long long Imm64 )
{
	if ( !Imm64 )
	{
		return XorRegReg32 ( DestReg, DestReg );
	}
	
	if ( Imm64 <= 0xffffffffULL )
	{
		return MovRegImm32 ( DestReg, Imm64 );
	}
	
	if ( Imm64 >= -0x80000000LL && Imm64 <= 0x7fffffffLL )
	{
		return MovReg64Imm32 ( DestReg, Imm64 );
	}
	
	return x64EncodeMovImm64 ( DestReg, Imm64 );
}


bool x64Encoder::MovRegToMem8 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_MOV_TOMEM8, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovRegFromMem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_MOV_FROMMEM8, DestReg, AddressReg, IndexReg, Scale, Offset );
}


bool x64Encoder::MovRegToMem16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	// add in prefix to make this a 16-bit operation - this must come before REX prefix!
	x64CodeArea [ x64NextOffset++ ] = PREFIX_16BIT;

	return x64EncodeRegMem32 ( X64OP_MOV_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovRegFromMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	// add in prefix to make this a 16-bit operation - this must come before REX prefix!
	x64CodeArea [ x64NextOffset++ ] = PREFIX_16BIT;

	return x64EncodeRegMem32 ( X64OP_MOV_FROMMEM, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovRegToMem32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_MOV_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovRegFromMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_MOV_FROMMEM, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovRegToMem64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_MOV_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovRegFromMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_MOV_FROMMEM, DestReg, AddressReg, IndexReg, Scale, Offset );
}





// *** testing ***
bool x64Encoder::MovRegToMem8 ( char* Address, long SrcReg )
{
	return x64EncodeRipOffset32 ( X64OP_MOV_TOMEM8, SrcReg, (char*)Address, true );
}

bool x64Encoder::MovRegFromMem8 ( long DestReg, char* Address )
{
	return x64EncodeRipOffset32 ( X64OP_MOV_FROMMEM8, DestReg, (char*)Address, true );
}

bool x64Encoder::MovRegToMem16 ( short* Address, long SrcReg )
{
	return x64EncodeRipOffset16 ( X64OP_MOV_TOMEM, SrcReg, (char*) Address, true );
}

bool x64Encoder::MovRegFromMem16 ( long DestReg, short* Address )
{
	return x64EncodeRipOffset16 ( X64OP_MOV_FROMMEM, DestReg, (char*)Address, true );
}

bool x64Encoder::MovRegToMem32 ( long* Address, long SrcReg )
{
	return x64EncodeRipOffset32 ( X64OP_MOV_TOMEM, SrcReg, (char*)Address, true );
}

bool x64Encoder::MovRegFromMem32 ( long DestReg, long* Address )
{
	return x64EncodeRipOffset32 ( X64OP_MOV_FROMMEM, DestReg, (char*)Address, true );
}

bool x64Encoder::MovRegToMem64 ( long long* Address, long SrcReg )
{
	return x64EncodeRipOffset64 ( X64OP_MOV_TOMEM, SrcReg, (char*)Address, true );
}

bool x64Encoder::MovRegFromMem64 ( long DestReg, long long* Address )
{
	return x64EncodeRipOffset64 ( X64OP_MOV_FROMMEM, DestReg, (char*)Address, true );
}

bool x64Encoder::MovMemImm8 ( char* DestPtr, char Imm8 )
{
	return x64EncodeRipOffsetImm8 ( X64OP_MOV_IMM8, MODRM_MOV_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::MovMemImm16 ( short* DestPtr, short Imm16 )
{
	return x64EncodeRipOffsetImm16 ( X64OP_MOV_IMM, MODRM_MOV_IMM, (char*) DestPtr, Imm16 );
}

bool x64Encoder::MovMemImm32 ( long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm32 ( X64OP_MOV_IMM, MODRM_MOV_IMM, (char*) DestPtr, Imm32 );
}

bool x64Encoder::MovMemImm64 ( long long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm64 ( X64OP_MOV_IMM, MODRM_MOV_IMM, (char*) DestPtr, Imm32 );
}



bool x64Encoder::MovMemImm8 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm8 ( X64OP_MOV_IMM8, MODRM_MOV_IMM8, Imm8, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm16 ( X64OP_MOV_IMM, MODRM_MOV_IMM, Imm16, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm32 ( X64OP_MOV_IMM, MODRM_MOV_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm64 ( X64OP_MOV_IMM, MODRM_MOV_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AddRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( X64OP_ADD, DestReg, SrcReg );
}

bool x64Encoder::AddRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( X64OP_ADD, DestReg, SrcReg );
}

bool x64Encoder::AddRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( X64OP_ADD, DestReg, SrcReg );
}

bool x64Encoder::AddRegImm16 ( long DestReg, short Imm16 )
{
	return x64EncodeReg16Imm16 ( X64OP_ADD_IMM, MODRM_ADD_IMM, DestReg, Imm16 );
}

bool x64Encoder::AddRegImm32 ( long DestReg, long Imm32 )
{
	return x64EncodeReg32Imm32 ( X64OP_ADD_IMM, MODRM_ADD_IMM, DestReg, Imm32 );
}

bool x64Encoder::AddRegImm64 ( long DestReg, long Imm32 )
{
	return x64EncodeReg64Imm32 ( X64OP_ADD_IMM, MODRM_ADD_IMM, DestReg, Imm32 );
}


bool x64Encoder::AddRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_ADD, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AddRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_ADD, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AddRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_ADD, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AddMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_ADD_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AddMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_ADD_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AddMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_ADD_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AddMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm16 ( X64OP_ADD_IMM, MODRM_ADD_IMM, Imm16, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AddMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm32 ( X64OP_ADD_IMM, MODRM_ADD_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AddMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm64 ( X64OP_ADD_IMM, MODRM_ADD_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}




bool x64Encoder::AddRegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( X64OP_ADD, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::AddRegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( X64OP_ADD, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::AddRegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( X64OP_ADD, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::AddMemReg16 ( short* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset16 ( X64OP_ADD_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::AddMemReg32 ( long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset32 ( X64OP_ADD_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::AddMemReg64 ( long long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset64 ( X64OP_ADD_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::AddMemImm16 ( short* DestPtr, short Imm16 )
{
	return x64EncodeRipOffsetImm16 ( X64OP_ADD_IMM, MODRM_ADD_IMM, (char*) DestPtr, Imm16 );
}

bool x64Encoder::AddMemImm32 ( long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm32 ( X64OP_ADD_IMM, MODRM_ADD_IMM, (char*) DestPtr, Imm32 );
}

bool x64Encoder::AddMemImm64 ( long long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm64 ( X64OP_ADD_IMM, MODRM_ADD_IMM, (char*) DestPtr, Imm32 );
}



bool x64Encoder::AddAcc16Imm16 ( short Imm16 )
{
	return x64EncodeAcc16Imm16 ( X64OP_ADD_IMMA, Imm16 );
}

bool x64Encoder::AddAcc32Imm32 ( long Imm32 )
{
	return x64EncodeAcc32Imm32 ( X64OP_ADD_IMMA, Imm32 );
}

bool x64Encoder::AddAcc64Imm32 ( long Imm32 )
{
	return x64EncodeAcc64Imm32 ( X64OP_ADD_IMMA, Imm32 );
}


bool x64Encoder::AddReg16Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg16Imm8 ( X64OP_ADD_IMM8, MODRM_ADD_IMM8, DestReg, Imm8 );
}

bool x64Encoder::AddReg32Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg32Imm8 ( X64OP_ADD_IMM8, MODRM_ADD_IMM8, DestReg, Imm8 );
}

bool x64Encoder::AddReg64Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg64Imm8 ( X64OP_ADD_IMM8, MODRM_ADD_IMM8, DestReg, Imm8 );
}

bool x64Encoder::AddMem16Imm8 ( short* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset16Imm8 ( X64OP_ADD_IMM8, MODRM_ADD_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::AddMem32Imm8 ( long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset32Imm8 ( X64OP_ADD_IMM8, MODRM_ADD_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::AddMem64Imm8 ( long long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset64Imm8 ( X64OP_ADD_IMM8, MODRM_ADD_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::AddReg16ImmX ( long DestReg, short ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX == 1 )
	{
		return IncReg16 ( DestReg );
	}

	if ( ImmX == -1 )
	{
		return DecReg16 ( DestReg );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return AddReg16Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return AddRegImm16 ( DestReg, ImmX );
	}
	
	return AddAcc16Imm16 ( ImmX );
}

bool x64Encoder::AddReg32ImmX ( long DestReg, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX == 1 )
	{
		return IncReg32 ( DestReg );
	}

	if ( ImmX == -1 )
	{
		return DecReg32 ( DestReg );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return AddReg32Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return AddRegImm32 ( DestReg, ImmX );
	}
	
	return AddAcc32Imm32 ( ImmX );
}

bool x64Encoder::AddReg64ImmX ( long DestReg, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX == 1 )
	{
		return IncReg64 ( DestReg );
	}

	if ( ImmX == -1 )
	{
		return DecReg64 ( DestReg );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return AddReg64Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return AddRegImm64 ( DestReg, ImmX );
	}
	
	return AddAcc64Imm32 ( ImmX );
}


bool x64Encoder::AddMem16ImmX ( short* DestPtr, short ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX == 1 )
	{
		return IncMem16 ( DestPtr );
	}

	if ( ImmX == -1 )
	{
		return DecMem16 ( DestPtr );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset16Imm8 ( X64OP_ADD_IMM8, MODRM_ADD_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm16 ( X64OP_ADD_IMM, MODRM_ADD_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::AddMem32ImmX ( long* DestPtr, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX == 1 )
	{
		return IncMem32 ( DestPtr );
	}

	if ( ImmX == -1 )
	{
		return DecMem32 ( DestPtr );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset32Imm8 ( X64OP_ADD_IMM8, MODRM_ADD_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm32 ( X64OP_ADD_IMM, MODRM_ADD_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::AddMem64ImmX ( long long* DestPtr, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX == 1 )
	{
		return IncMem64 ( DestPtr );
	}

	if ( ImmX == -1 )
	{
		return DecMem64 ( DestPtr );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset64Imm8 ( X64OP_ADD_IMM8, MODRM_ADD_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm64 ( X64OP_ADD_IMM, MODRM_ADD_IMM, (char*) DestPtr, ImmX );
}






// ---------------------------


bool x64Encoder::AdcRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( X64OP_ADC, DestReg, SrcReg );
}

bool x64Encoder::AdcRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( X64OP_ADC, DestReg, SrcReg );
}

bool x64Encoder::AdcRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( X64OP_ADC, DestReg, SrcReg );
}

bool x64Encoder::AdcRegImm16 ( long DestReg, short Imm16 )
{
	return x64EncodeReg16Imm16 ( X64OP_ADC_IMM, MODRM_ADC_IMM, DestReg, Imm16 );
}

bool x64Encoder::AdcRegImm32 ( long DestReg, long Imm32 )
{
	return x64EncodeReg32Imm32 ( X64OP_ADC_IMM, MODRM_ADC_IMM, DestReg, Imm32 );
}

bool x64Encoder::AdcRegImm64 ( long DestReg, long Imm32 )
{
	return x64EncodeReg64Imm32 ( X64OP_ADC_IMM, MODRM_ADC_IMM, DestReg, Imm32 );
}


bool x64Encoder::AdcRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_ADC, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AdcRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_ADC, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AdcRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_ADC, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AdcMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_ADC_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AdcMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_ADC_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AdcMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_ADC_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AdcMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm16 ( X64OP_ADC_IMM, MODRM_ADC_IMM, Imm16, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AdcMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm32 ( X64OP_ADC_IMM, MODRM_ADC_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AdcMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm64 ( X64OP_ADC_IMM, MODRM_ADC_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}




bool x64Encoder::AdcRegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( X64OP_ADC, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::AdcRegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( X64OP_ADC, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::AdcRegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( X64OP_ADC, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::AdcMemReg16 ( short* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset16 ( X64OP_ADC_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::AdcMemReg32 ( long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset32 ( X64OP_ADC_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::AdcMemReg64 ( long long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset64 ( X64OP_ADC_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::AdcMemImm16 ( short* DestPtr, short Imm16 )
{
	return x64EncodeRipOffsetImm16 ( X64OP_ADC_IMM, MODRM_ADC_IMM, (char*) DestPtr, Imm16 );
}

bool x64Encoder::AdcMemImm32 ( long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm32 ( X64OP_ADC_IMM, MODRM_ADC_IMM, (char*) DestPtr, Imm32 );
}

bool x64Encoder::AdcMemImm64 ( long long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm64 ( X64OP_ADC_IMM, MODRM_ADC_IMM, (char*) DestPtr, Imm32 );
}



bool x64Encoder::AdcAcc16Imm16 ( short Imm16 )
{
	return x64EncodeAcc16Imm16 ( X64OP_ADC_IMMA, Imm16 );
}

bool x64Encoder::AdcAcc32Imm32 ( long Imm32 )
{
	return x64EncodeAcc32Imm32 ( X64OP_ADC_IMMA, Imm32 );
}

bool x64Encoder::AdcAcc64Imm32 ( long Imm32 )
{
	return x64EncodeAcc64Imm32 ( X64OP_ADC_IMMA, Imm32 );
}


bool x64Encoder::AdcReg16Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg16Imm8 ( X64OP_ADC_IMM8, MODRM_ADC_IMM8, DestReg, Imm8 );
}

bool x64Encoder::AdcReg32Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg32Imm8 ( X64OP_ADC_IMM8, MODRM_ADC_IMM8, DestReg, Imm8 );
}

bool x64Encoder::AdcReg64Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg64Imm8 ( X64OP_ADC_IMM8, MODRM_ADC_IMM8, DestReg, Imm8 );
}

bool x64Encoder::AdcMem16Imm8 ( short* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset16Imm8 ( X64OP_ADC_IMM8, MODRM_ADC_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::AdcMem32Imm8 ( long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset32Imm8 ( X64OP_ADC_IMM8, MODRM_ADC_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::AdcMem64Imm8 ( long long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset64Imm8 ( X64OP_ADC_IMM8, MODRM_ADC_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::AdcReg16ImmX ( long DestReg, short ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return AdcReg16Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return AdcRegImm16 ( DestReg, ImmX );
	}
	
	return AdcAcc16Imm16 ( ImmX );
}

bool x64Encoder::AdcReg32ImmX ( long DestReg, long ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return AdcReg32Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return AdcRegImm32 ( DestReg, ImmX );
	}
	
	return AdcAcc32Imm32 ( ImmX );
}

bool x64Encoder::AdcReg64ImmX ( long DestReg, long ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return AdcReg64Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return AdcRegImm64 ( DestReg, ImmX );
	}
	
	return AdcAcc64Imm32 ( ImmX );
}


bool x64Encoder::AdcMem16ImmX ( short* DestPtr, short ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset16Imm8 ( X64OP_ADC_IMM8, MODRM_ADC_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm16 ( X64OP_ADC_IMM, MODRM_ADC_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::AdcMem32ImmX ( long* DestPtr, long ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset32Imm8 ( X64OP_ADC_IMM8, MODRM_ADC_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm32 ( X64OP_ADC_IMM, MODRM_ADC_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::AdcMem64ImmX ( long long* DestPtr, long ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset64Imm8 ( X64OP_ADC_IMM8, MODRM_ADC_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm64 ( X64OP_ADC_IMM, MODRM_ADC_IMM, (char*) DestPtr, ImmX );
}








// --------------------------------












bool x64Encoder::BsrRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP_BSR_OP1, X64OP_BSR_OP2 ), DestReg, SrcReg );
}

bool x64Encoder::BsrRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP_BSR_OP1, X64OP_BSR_OP2 ), DestReg, SrcReg );
}

bool x64Encoder::BsrRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP_BSR_OP1, X64OP_BSR_OP2 ), DestReg, SrcReg );
}

bool x64Encoder::BsrRegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( MAKE2OPCODE( X64OP_BSR_OP1, X64OP_BSR_OP2 ), DestReg, (char*) SrcPtr );
}

bool x64Encoder::BsrRegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP_BSR_OP1, X64OP_BSR_OP2 ), DestReg, (char*) SrcPtr );
}

bool x64Encoder::BsrRegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP_BSR_OP1, X64OP_BSR_OP2 ), DestReg, (char*) SrcPtr );
}



bool x64Encoder::AndRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( X64OP_AND, DestReg, SrcReg );
}

bool x64Encoder::AndRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( X64OP_AND, DestReg, SrcReg );
}

bool x64Encoder::AndRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( X64OP_AND, DestReg, SrcReg );
}

bool x64Encoder::AndRegImm16 ( long DestReg, short Imm16 )
{
	return x64EncodeReg16Imm16 ( X64OP_AND_IMM, MODRM_AND_IMM, DestReg, Imm16 );
}

bool x64Encoder::AndRegImm32 ( long DestReg, long Imm32 )
{
	return x64EncodeReg32Imm32 ( X64OP_AND_IMM, MODRM_AND_IMM, DestReg, Imm32 );
}

bool x64Encoder::AndRegImm64 ( long DestReg, long Imm32 )
{
	return x64EncodeReg64Imm32 ( X64OP_AND_IMM, MODRM_AND_IMM, DestReg, Imm32 );
}

bool x64Encoder::AndRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_AND, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AndRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_AND, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AndRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_AND, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AndMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_AND_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AndMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_AND_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AndMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_AND_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AndMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm16 ( X64OP_AND_IMM, MODRM_AND_IMM, Imm16, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AndMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm32 ( X64OP_AND_IMM, MODRM_AND_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::AndMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm64 ( X64OP_AND_IMM, MODRM_AND_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}


bool x64Encoder::AndRegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( X64OP_AND, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::AndRegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( X64OP_AND, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::AndRegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( X64OP_AND, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::AndMemReg16 ( short* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset16 ( X64OP_AND_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::AndMemReg32 ( long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset32 ( X64OP_AND_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::AndMemReg64 ( long long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset64 ( X64OP_AND_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::AndMemImm16 ( short* DestPtr, short Imm16 )
{
	return x64EncodeRipOffsetImm16 ( X64OP_AND_IMM, MODRM_AND_IMM, (char*) DestPtr, Imm16 );
}

bool x64Encoder::AndMemImm32 ( long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm32 ( X64OP_AND_IMM, MODRM_AND_IMM, (char*) DestPtr, Imm32 );
}

bool x64Encoder::AndMemImm64 ( long long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm64 ( X64OP_AND_IMM, MODRM_AND_IMM, (char*) DestPtr, Imm32 );
}



bool x64Encoder::AndAcc16Imm16 ( short Imm16 )
{
	return x64EncodeAcc16Imm16 ( X64OP_AND_IMMA, Imm16 );
}

bool x64Encoder::AndAcc32Imm32 ( long Imm32 )
{
	return x64EncodeAcc32Imm32 ( X64OP_AND_IMMA, Imm32 );
}

bool x64Encoder::AndAcc64Imm32 ( long Imm32 )
{
	return x64EncodeAcc64Imm32 ( X64OP_AND_IMMA, Imm32 );
}


bool x64Encoder::AndReg16Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg16Imm8 ( X64OP_AND_IMM8, MODRM_AND_IMM8, DestReg, Imm8 );
}

bool x64Encoder::AndReg32Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg32Imm8 ( X64OP_AND_IMM8, MODRM_AND_IMM8, DestReg, Imm8 );
}

bool x64Encoder::AndReg64Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg64Imm8 ( X64OP_AND_IMM8, MODRM_AND_IMM8, DestReg, Imm8 );
}

bool x64Encoder::AndMem16Imm8 ( short* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset16Imm8 ( X64OP_AND_IMM8, MODRM_AND_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::AndMem32Imm8 ( long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset32Imm8 ( X64OP_AND_IMM8, MODRM_AND_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::AndMem64Imm8 ( long long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset64Imm8 ( X64OP_AND_IMM8, MODRM_AND_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::AndReg16ImmX ( long DestReg, short ImmX )
{
	if ( !ImmX )
	{
		return XorRegReg16 ( DestReg, DestReg );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return AndReg16Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return AndRegImm16 ( DestReg, ImmX );
	}
	
	return AndAcc16Imm16 ( ImmX );
}

bool x64Encoder::AndReg32ImmX ( long DestReg, long ImmX )
{
	if ( !ImmX )
	{
		return XorRegReg32 ( DestReg, DestReg );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return AndReg32Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return AndRegImm32 ( DestReg, ImmX );
	}
	
	return AndAcc32Imm32 ( ImmX );
}

bool x64Encoder::AndReg64ImmX ( long DestReg, long ImmX )
{
	if ( !ImmX )
	{
		return XorRegReg64 ( DestReg, DestReg );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return AndReg64Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return AndRegImm64 ( DestReg, ImmX );
	}
	
	return AndAcc64Imm32 ( ImmX );
}


bool x64Encoder::AndMem16ImmX ( short* DestPtr, short ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset16Imm8 ( X64OP_AND_IMM8, MODRM_AND_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm16 ( X64OP_AND_IMM, MODRM_AND_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::AndMem32ImmX ( long* DestPtr, long ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset32Imm8 ( X64OP_AND_IMM8, MODRM_AND_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm32 ( X64OP_AND_IMM, MODRM_AND_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::AndMem64ImmX ( long long* DestPtr, long ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset64Imm8 ( X64OP_AND_IMM8, MODRM_AND_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm64 ( X64OP_AND_IMM, MODRM_AND_IMM, (char*) DestPtr, ImmX );
}




bool x64Encoder::BtRegReg16 ( long Reg, long BitSelectReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE ( X64OP1_BT, X64OP2_BT ), BitSelectReg, Reg );
}

bool x64Encoder::BtRegReg32 ( long Reg, long BitSelectReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE ( X64OP1_BT, X64OP2_BT ), BitSelectReg, Reg );
}

bool x64Encoder::BtRegReg64 ( long Reg, long BitSelectReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE ( X64OP1_BT, X64OP2_BT ), BitSelectReg, Reg );
}

bool x64Encoder::BtRegImm16 ( long Reg, char Imm8 )
{
	return x64EncodeReg16Imm8 ( MAKE2OPCODE ( X64OP1_BT_IMM, X64OP2_BT_IMM ), MODRM_BT_IMM, Reg, Imm8 );
}

bool x64Encoder::BtRegImm32 ( long Reg, char Imm8 )
{
	return x64EncodeReg32Imm8 ( MAKE2OPCODE ( X64OP1_BT_IMM, X64OP2_BT_IMM ), MODRM_BT_IMM, Reg, Imm8 );
}

bool x64Encoder::BtRegImm64 ( long Reg, char Imm8 )
{
	return x64EncodeReg64Imm8 ( MAKE2OPCODE ( X64OP1_BT_IMM, X64OP2_BT_IMM ), MODRM_BT_IMM, Reg, Imm8 );
}


bool x64Encoder::BtMemImm16 ( short* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset16Imm8 ( MAKE2OPCODE ( X64OP1_BT_IMM, X64OP2_BT_IMM ), MODRM_BT_IMM, (char*) DestPtr, Imm8 );
}

bool x64Encoder::BtMemImm32 ( long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset32Imm8 ( MAKE2OPCODE ( X64OP1_BT_IMM, X64OP2_BT_IMM ), MODRM_BT_IMM, (char*) DestPtr, Imm8 );
}

bool x64Encoder::BtMemImm64 ( long long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset64Imm8 ( MAKE2OPCODE ( X64OP1_BT_IMM, X64OP2_BT_IMM ), MODRM_BT_IMM, (char*) DestPtr, Imm8 );
}



bool x64Encoder::BtMemReg16 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( MAKE2OPCODE ( X64OP1_BT, X64OP2_BT ), BitSelectReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::BtMemReg32 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE ( X64OP1_BT, X64OP2_BT ), BitSelectReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::BtMemReg64 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( MAKE2OPCODE ( X64OP1_BT, X64OP2_BT ), BitSelectReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::BtMemImm16 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMem16Imm8 ( MAKE2OPCODE ( X64OP1_BT_IMM, X64OP2_BT_IMM ), MODRM_BT_IMM, AddressReg, IndexReg, Scale, Offset, Imm8 );
}

bool x64Encoder::BtMemImm32 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMem32Imm8 ( MAKE2OPCODE ( X64OP1_BT_IMM, X64OP2_BT_IMM ), MODRM_BT_IMM, AddressReg, IndexReg, Scale, Offset, Imm8 );
}

bool x64Encoder::BtMemImm64 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMem64Imm8 ( MAKE2OPCODE ( X64OP1_BT_IMM, X64OP2_BT_IMM ), MODRM_BT_IMM, AddressReg, IndexReg, Scale, Offset, Imm8 );
}

// btc

bool x64Encoder::BtcRegReg16 ( long Reg, long BitSelectReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE ( X64OP1_BTC, X64OP2_BTC ), BitSelectReg, Reg );
}

bool x64Encoder::BtcRegReg32 ( long Reg, long BitSelectReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE ( X64OP1_BTC, X64OP2_BTC ), BitSelectReg, Reg );
}

bool x64Encoder::BtcRegReg64 ( long Reg, long BitSelectReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE ( X64OP1_BTC, X64OP2_BTC ), BitSelectReg, Reg );
}

bool x64Encoder::BtcRegImm16 ( long Reg, char Imm8 )
{
	return x64EncodeReg16Imm8 ( MAKE2OPCODE ( X64OP1_BTC_IMM, X64OP2_BTC_IMM ), MODRM_BTC_IMM, Reg, Imm8 );
}

bool x64Encoder::BtcRegImm32 ( long Reg, char Imm8 )
{
	return x64EncodeReg32Imm8 ( MAKE2OPCODE ( X64OP1_BTC_IMM, X64OP2_BTC_IMM ), MODRM_BTC_IMM, Reg, Imm8 );
}

bool x64Encoder::BtcRegImm64 ( long Reg, char Imm8 )
{
	return x64EncodeReg64Imm8 ( MAKE2OPCODE ( X64OP1_BTC_IMM, X64OP2_BTC_IMM ), MODRM_BTC_IMM, Reg, Imm8 );
}

bool x64Encoder::BtcMemReg16 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( MAKE2OPCODE ( X64OP1_BTC, X64OP2_BTC ), BitSelectReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::BtcMemReg32 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE ( X64OP1_BTC, X64OP2_BTC ), BitSelectReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::BtcMemReg64 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( MAKE2OPCODE ( X64OP1_BTC, X64OP2_BTC ), BitSelectReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::BtcMemImm16 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMem16Imm8 ( MAKE2OPCODE ( X64OP1_BTC_IMM, X64OP2_BTC_IMM ), MODRM_BTC_IMM, AddressReg, IndexReg, Scale, Offset, Imm8 );
}

bool x64Encoder::BtcMemImm32 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMem32Imm8 ( MAKE2OPCODE ( X64OP1_BTC_IMM, X64OP2_BTC_IMM ), MODRM_BTC_IMM, AddressReg, IndexReg, Scale, Offset, Imm8 );
}

bool x64Encoder::BtcMemImm64 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMem64Imm8 ( MAKE2OPCODE ( X64OP1_BTC_IMM, X64OP2_BTC_IMM ), MODRM_BTC_IMM, AddressReg, IndexReg, Scale, Offset, Imm8 );
}

// btr

bool x64Encoder::BtrRegReg16 ( long Reg, long BitSelectReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE ( X64OP1_BTR, X64OP2_BTR ), BitSelectReg, Reg );
}

bool x64Encoder::BtrRegReg32 ( long Reg, long BitSelectReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE ( X64OP1_BTR, X64OP2_BTR ), BitSelectReg, Reg );
}

bool x64Encoder::BtrRegReg64 ( long Reg, long BitSelectReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE ( X64OP1_BTR, X64OP2_BTR ), BitSelectReg, Reg );
}

bool x64Encoder::BtrRegImm16 ( long Reg, char Imm8 )
{
	return x64EncodeReg16Imm8 ( MAKE2OPCODE ( X64OP1_BTR_IMM, X64OP2_BTR_IMM ), MODRM_BTR_IMM, Reg, Imm8 );
}

bool x64Encoder::BtrRegImm32 ( long Reg, char Imm8 )
{
	return x64EncodeReg32Imm8 ( MAKE2OPCODE ( X64OP1_BTR_IMM, X64OP2_BTR_IMM ), MODRM_BTR_IMM, Reg, Imm8 );
}

bool x64Encoder::BtrRegImm64 ( long Reg, char Imm8 )
{
	return x64EncodeReg64Imm8 ( MAKE2OPCODE ( X64OP1_BTR_IMM, X64OP2_BTR_IMM ), MODRM_BTR_IMM, Reg, Imm8 );
}

bool x64Encoder::BtrMemReg16 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( MAKE2OPCODE ( X64OP1_BTR, X64OP2_BTR ), BitSelectReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::BtrMemReg32 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE ( X64OP1_BTR, X64OP2_BTR ), BitSelectReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::BtrMemReg64 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( MAKE2OPCODE ( X64OP1_BTR, X64OP2_BTR ), BitSelectReg, AddressReg, IndexReg, Scale, Offset );
}


bool x64Encoder::BtrMem32Imm ( long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffsetImm8 ( MAKE2OPCODE ( X64OP1_BTR_IMM, X64OP2_BTR_IMM ), MODRM_BTR_IMM, (char*) DestPtr, Imm8 );
}

bool x64Encoder::BtrMemImm16 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMem16Imm8 ( MAKE2OPCODE ( X64OP1_BTR_IMM, X64OP2_BTR_IMM ), MODRM_BTR_IMM, AddressReg, IndexReg, Scale, Offset, Imm8 );
}

bool x64Encoder::BtrMemImm32 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMem32Imm8 ( MAKE2OPCODE ( X64OP1_BTR_IMM, X64OP2_BTR_IMM ), MODRM_BTR_IMM, AddressReg, IndexReg, Scale, Offset, Imm8 );
}

bool x64Encoder::BtrMemImm64 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMem64Imm8 ( MAKE2OPCODE ( X64OP1_BTR_IMM, X64OP2_BTR_IMM ), MODRM_BTR_IMM, AddressReg, IndexReg, Scale, Offset, Imm8 );
}

// bts

bool x64Encoder::BtsRegReg16 ( long Reg, long BitSelectReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE ( X64OP1_BTS, X64OP2_BTS ), BitSelectReg, Reg );
}

bool x64Encoder::BtsRegReg32 ( long Reg, long BitSelectReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE ( X64OP1_BTS, X64OP2_BTS ), BitSelectReg, Reg );
}

bool x64Encoder::BtsRegReg64 ( long Reg, long BitSelectReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE ( X64OP1_BTS, X64OP2_BTS ), BitSelectReg, Reg );
}

bool x64Encoder::BtsRegImm16 ( long Reg, char Imm8 )
{
	return x64EncodeReg16Imm8 ( MAKE2OPCODE ( X64OP1_BTS_IMM, X64OP2_BTS_IMM ), MODRM_BTS_IMM, Reg, Imm8 );
}

bool x64Encoder::BtsRegImm32 ( long Reg, char Imm8 )
{
	return x64EncodeReg32Imm8 ( MAKE2OPCODE ( X64OP1_BTS_IMM, X64OP2_BTS_IMM ), MODRM_BTS_IMM, Reg, Imm8 );
}

bool x64Encoder::BtsRegImm64 ( long Reg, char Imm8 )
{
	return x64EncodeReg64Imm8 ( MAKE2OPCODE ( X64OP1_BTS_IMM, X64OP2_BTS_IMM ), MODRM_BTS_IMM, Reg, Imm8 );
}

bool x64Encoder::BtsMemReg16 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( MAKE2OPCODE ( X64OP1_BTS, X64OP2_BTS ), BitSelectReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::BtsMemReg32 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE ( X64OP1_BTS, X64OP2_BTS ), BitSelectReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::BtsMemReg64 ( long BitSelectReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( MAKE2OPCODE ( X64OP1_BTS, X64OP2_BTS ), BitSelectReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::BtsMemImm16 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMem16Imm8 ( MAKE2OPCODE ( X64OP1_BTS_IMM, X64OP2_BTS_IMM ), MODRM_BTS_IMM, AddressReg, IndexReg, Scale, Offset, Imm8 );
}

bool x64Encoder::BtsMemImm32 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMem32Imm8 ( MAKE2OPCODE ( X64OP1_BTS_IMM, X64OP2_BTS_IMM ), MODRM_BTS_IMM, AddressReg, IndexReg, Scale, Offset, Imm8 );
}

bool x64Encoder::BtsMemImm64 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMem64Imm8 ( MAKE2OPCODE ( X64OP1_BTS_IMM, X64OP2_BTS_IMM ), MODRM_BTS_IMM, AddressReg, IndexReg, Scale, Offset, Imm8 );
}



bool x64Encoder::Cbw ( void )
{
	return x64Encode16 ( X64OP_CBW );
}

bool x64Encoder::Cwde ( void )
{
	return x64Encode32 ( X64OP_CWDE );
}

bool x64Encoder::Cdqe ( void )
{
	return x64Encode64 ( X64OP_CDQE );
}



bool x64Encoder::Cwd ( void )
{
	return x64Encode16 ( X64OP_CWD );
}

bool x64Encoder::Cdq ( void )
{
	return x64Encode32 ( X64OP_CDQ );
}

bool x64Encoder::Cqo ( void )
{
	return x64Encode64 ( X64OP_CQO );
}



bool x64Encoder::CmovERegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_CMOVE, X64OP2_CMOVE ), DestReg, SrcReg );
}

bool x64Encoder::CmovNERegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_CMOVNE, X64OP2_CMOVNE ), DestReg, SrcReg );
}

bool x64Encoder::CmovBRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_CMOVB, X64OP2_CMOVB ), DestReg, SrcReg );
}

bool x64Encoder::CmovBERegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_CMOVBE, X64OP2_CMOVBE ), DestReg, SrcReg );
}

bool x64Encoder::CmovARegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_CMOVA, X64OP2_CMOVA ), DestReg, SrcReg );
}

bool x64Encoder::CmovAERegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_CMOVAE, X64OP2_CMOVAE ), DestReg, SrcReg );
}

bool x64Encoder::CmovLRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_CMOVL, X64OP2_CMOVL ), DestReg, SrcReg );
}

bool x64Encoder::CmovERegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CMOVE, X64OP2_CMOVE ), DestReg, SrcReg );
}

bool x64Encoder::CmovNERegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CMOVNE, X64OP2_CMOVNE ), DestReg, SrcReg );
}

bool x64Encoder::CmovBRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CMOVB, X64OP2_CMOVB ), DestReg, SrcReg );
}

bool x64Encoder::CmovBERegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CMOVBE, X64OP2_CMOVBE ), DestReg, SrcReg );
}

bool x64Encoder::CmovARegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CMOVA, X64OP2_CMOVA ), DestReg, SrcReg );
}

bool x64Encoder::CmovAERegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CMOVAE, X64OP2_CMOVAE ), DestReg, SrcReg );
}


bool x64Encoder::CmovLRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CMOVL, X64OP2_CMOVL ), DestReg, SrcReg );
}

bool x64Encoder::CmovERegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_CMOVE, X64OP2_CMOVE ), DestReg, SrcReg );
}

bool x64Encoder::CmovNERegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_CMOVNE, X64OP2_CMOVNE ), DestReg, SrcReg );
}

bool x64Encoder::CmovBRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_CMOVB, X64OP2_CMOVB ), DestReg, SrcReg );
}

bool x64Encoder::CmovBERegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_CMOVBE, X64OP2_CMOVBE ), DestReg, SrcReg );
}

bool x64Encoder::CmovARegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_CMOVA, X64OP2_CMOVA ), DestReg, SrcReg );
}

bool x64Encoder::CmovAERegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_CMOVAE, X64OP2_CMOVAE ), DestReg, SrcReg );
}


bool x64Encoder::CmovLRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_CMOVL, X64OP2_CMOVL ), DestReg, SrcReg );
}


bool x64Encoder::CmovERegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( MAKE2OPCODE( X64OP1_CMOVE, X64OP2_CMOVE ), DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmovERegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_CMOVE, X64OP2_CMOVE ), DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmovERegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( MAKE2OPCODE( X64OP1_CMOVE, X64OP2_CMOVE ), DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmovNERegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( MAKE2OPCODE( X64OP1_CMOVNE, X64OP2_CMOVNE ), DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmovNERegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_CMOVNE, X64OP2_CMOVNE ), DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmovNERegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( MAKE2OPCODE( X64OP1_CMOVNE, X64OP2_CMOVNE ), DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmovBRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( MAKE2OPCODE( X64OP1_CMOVB, X64OP2_CMOVB ), DestReg, AddressReg, IndexReg, Scale, Offset );
}



bool x64Encoder::CmovERegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( MAKE2OPCODE( X64OP1_CMOVE, X64OP2_CMOVE ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovNERegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( MAKE2OPCODE( X64OP1_CMOVNE, X64OP2_CMOVNE ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovBRegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( MAKE2OPCODE( X64OP1_CMOVB, X64OP2_CMOVB ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovBERegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( MAKE2OPCODE( X64OP1_CMOVBE, X64OP2_CMOVBE ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovARegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( MAKE2OPCODE( X64OP1_CMOVA, X64OP2_CMOVA ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovAERegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( MAKE2OPCODE( X64OP1_CMOVAE, X64OP2_CMOVAE ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovERegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_CMOVE, X64OP2_CMOVE ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovNERegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_CMOVNE, X64OP2_CMOVNE ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovBRegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_CMOVB, X64OP2_CMOVB ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovBERegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_CMOVBE, X64OP2_CMOVBE ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovARegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_CMOVA, X64OP2_CMOVA ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovAERegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_CMOVAE, X64OP2_CMOVAE ), DestReg, (char*) SrcPtr, true );
}


bool x64Encoder::CmovERegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_CMOVE, X64OP2_CMOVE ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovNERegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_CMOVNE, X64OP2_CMOVNE ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovBRegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_CMOVB, X64OP2_CMOVB ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovBERegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_CMOVBE, X64OP2_CMOVBE ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovARegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_CMOVA, X64OP2_CMOVA ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovAERegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_CMOVAE, X64OP2_CMOVAE ), DestReg, (char*) SrcPtr, true );
}



bool x64Encoder::CmovLERegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_CMOVLE, X64OP2_CMOVLE ), DestReg, SrcReg );
}

bool x64Encoder::CmovGRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_CMOVG, X64OP2_CMOVG ), DestReg, SrcReg );
}

bool x64Encoder::CmovGERegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_CMOVGE, X64OP2_CMOVGE ), DestReg, SrcReg );
}

bool x64Encoder::CmovLERegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CMOVLE, X64OP2_CMOVLE ), DestReg, SrcReg );
}


bool x64Encoder::CmovGRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CMOVG, X64OP2_CMOVG ), DestReg, SrcReg );
}

bool x64Encoder::CmovGERegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CMOVGE, X64OP2_CMOVGE ), DestReg, SrcReg );
}

bool x64Encoder::CmovLERegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_CMOVLE, X64OP2_CMOVLE ), DestReg, SrcReg );
}

bool x64Encoder::CmovGRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_CMOVG, X64OP2_CMOVG ), DestReg, SrcReg );
}

bool x64Encoder::CmovGERegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_CMOVGE, X64OP2_CMOVGE ), DestReg, SrcReg );
}


bool x64Encoder::CmovSRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_CMOVS, X64OP2_CMOVS ), DestReg, SrcReg );
}

bool x64Encoder::CmovSRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CMOVS, X64OP2_CMOVS ), DestReg, SrcReg );
}

bool x64Encoder::CmovSRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_CMOVS, X64OP2_CMOVS ), DestReg, SrcReg );
}

bool x64Encoder::CmovSRegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( MAKE2OPCODE( X64OP1_CMOVS, X64OP2_CMOVS ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovSRegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_CMOVS, X64OP2_CMOVS ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovSRegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_CMOVS, X64OP2_CMOVS ), DestReg, (char*) SrcPtr, true );
}



bool x64Encoder::CmovNSRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_CMOVNS, X64OP2_CMOVNS ), DestReg, SrcReg );
}

bool x64Encoder::CmovNSRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CMOVNS, X64OP2_CMOVNS ), DestReg, SrcReg );
}

bool x64Encoder::CmovNSRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_CMOVNS, X64OP2_CMOVNS ), DestReg, SrcReg );
}

bool x64Encoder::CmovNSRegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( MAKE2OPCODE( X64OP1_CMOVNS, X64OP2_CMOVNS ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovNSRegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_CMOVNS, X64OP2_CMOVNS ), DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmovNSRegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_CMOVNS, X64OP2_CMOVNS ), DestReg, (char*) SrcPtr, true );
}






bool x64Encoder::CmpRegReg16 ( long SrcReg1, long SrcReg2 )
{
	return x64EncodeRegReg16 ( X64OP_CMP, SrcReg1, SrcReg2 );
}

bool x64Encoder::CmpRegReg32 ( long SrcReg1, long SrcReg2 )
{
	return x64EncodeRegReg32 ( X64OP_CMP, SrcReg1, SrcReg2 );
}

bool x64Encoder::CmpRegReg64 ( long SrcReg1, long SrcReg2 )
{
	return x64EncodeRegReg64 ( X64OP_CMP, SrcReg1, SrcReg2 );
}

bool x64Encoder::CmpRegImm16 ( long SrcReg, short Imm16 )
{
	return x64EncodeReg16Imm16 ( X64OP_CMP_IMM, MODRM_CMP_IMM, SrcReg, Imm16 );
}

bool x64Encoder::CmpRegImm32 ( long SrcReg, long Imm32 )
{
	return x64EncodeReg32Imm32 ( X64OP_CMP_IMM, MODRM_CMP_IMM, SrcReg, Imm32 );
}

bool x64Encoder::CmpRegImm64 ( long SrcReg, long Imm32 )
{
	return x64EncodeReg64Imm32 ( X64OP_CMP_IMM, MODRM_CMP_IMM, SrcReg, Imm32 );
}

bool x64Encoder::CmpRegMem8 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_CMP8, SrcReg, AddressReg, IndexReg, Scale, Offset );
}
bool x64Encoder::CmpRegMem16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_CMP, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmpRegMem32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_CMP, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmpRegMem64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_CMP, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmpMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_CMP_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmpMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_CMP_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmpMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_CMP_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmpMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm16 ( X64OP_CMP_IMM, MODRM_CMP_IMM, Imm16, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmpMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm32 ( X64OP_CMP_IMM, MODRM_CMP_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::CmpMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm64 ( X64OP_CMP_IMM, MODRM_CMP_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}




bool x64Encoder::CmpRegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( X64OP_CMP, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmpRegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( X64OP_CMP, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmpRegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( X64OP_CMP, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::CmpMemReg16 ( short* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset16 ( X64OP_CMP_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::CmpMemReg32 ( long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset32 ( X64OP_CMP_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::CmpMemReg64 ( long long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset64 ( X64OP_CMP_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::CmpMemImm8 ( char* DestPtr, char Imm8 )
{
	return x64EncodeRipOffsetImm8 ( X64OP_CMP_IMM8, MODRM_CMP_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::CmpMemImm16 ( short* DestPtr, short Imm16 )
{
	return x64EncodeRipOffsetImm16 ( X64OP_CMP_IMM, MODRM_CMP_IMM, (char*) DestPtr, Imm16 );
}

bool x64Encoder::CmpMemImm32 ( long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm32 ( X64OP_CMP_IMM, MODRM_CMP_IMM, (char*) DestPtr, Imm32 );
}

bool x64Encoder::CmpMemImm64 ( long long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm64 ( X64OP_CMP_IMM, MODRM_CMP_IMM, (char*) DestPtr, Imm32 );
}



bool x64Encoder::CmpAcc16Imm16 ( short Imm16 )
{
	return x64EncodeAcc16Imm16 ( X64OP_CMP_IMMA, Imm16 );
}

bool x64Encoder::CmpAcc32Imm32 ( long Imm32 )
{
	return x64EncodeAcc32Imm32 ( X64OP_CMP_IMMA, Imm32 );
}

bool x64Encoder::CmpAcc64Imm32 ( long Imm32 )
{
	return x64EncodeAcc64Imm32 ( X64OP_CMP_IMMA, Imm32 );
}


bool x64Encoder::CmpReg16Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg16Imm8 ( X64OP_CMP_IMM8, MODRM_CMP_IMM8, DestReg, Imm8 );
}

bool x64Encoder::CmpReg32Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg32Imm8 ( X64OP_CMP_IMM8, MODRM_CMP_IMM8, DestReg, Imm8 );
}

bool x64Encoder::CmpReg64Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg64Imm8 ( X64OP_CMP_IMM8, MODRM_CMP_IMM8, DestReg, Imm8 );
}

bool x64Encoder::CmpMem16Imm8 ( short* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset16Imm8 ( X64OP_CMP_IMM8, MODRM_CMP_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::CmpMem32Imm8 ( long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset32Imm8 ( X64OP_CMP_IMM8, MODRM_CMP_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::CmpMem64Imm8 ( long long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset64Imm8 ( X64OP_CMP_IMM8, MODRM_CMP_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::CmpReg16ImmX ( long DestReg, short ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return CmpReg16Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return CmpRegImm16 ( DestReg, ImmX );
	}
	
	return CmpAcc16Imm16 ( ImmX );
}

bool x64Encoder::CmpReg32ImmX ( long DestReg, long ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return CmpReg32Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return CmpRegImm32 ( DestReg, ImmX );
	}
	
	return CmpAcc32Imm32 ( ImmX );
}

bool x64Encoder::CmpReg64ImmX ( long DestReg, long ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return CmpReg64Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return CmpRegImm64 ( DestReg, ImmX );
	}
	
	return CmpAcc64Imm32 ( ImmX );
}


bool x64Encoder::CmpMem16ImmX ( short* DestPtr, short ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset16Imm8 ( X64OP_CMP_IMM8, MODRM_CMP_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm16 ( X64OP_CMP_IMM, MODRM_CMP_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::CmpMem32ImmX ( long* DestPtr, long ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset32Imm8 ( X64OP_CMP_IMM8, MODRM_CMP_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm32 ( X64OP_CMP_IMM, MODRM_CMP_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::CmpMem64ImmX ( long long* DestPtr, long ImmX )
{
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset64Imm8 ( X64OP_CMP_IMM8, MODRM_CMP_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm64 ( X64OP_CMP_IMM, MODRM_CMP_IMM, (char*) DestPtr, ImmX );
}






bool x64Encoder::DivRegReg16 ( long SrcReg )
{
	return x64EncodeReg16 ( X64OP_DIV, MODRM_DIV, SrcReg );
}

bool x64Encoder::DivRegReg32 ( long SrcReg )
{
	return x64EncodeReg32 ( X64OP_DIV, MODRM_DIV, SrcReg );
}

bool x64Encoder::DivRegReg64 ( long SrcReg )
{
	return x64EncodeReg64 ( X64OP_DIV, MODRM_DIV, SrcReg );
}

bool x64Encoder::DivRegMem16 ( short* SrcPtr )
{
	return x64EncodeRipOffset16 ( X64OP_DIV, MODRM_DIV, (char*) SrcPtr );
}

bool x64Encoder::DivRegMem32 ( long* SrcPtr )
{
	return x64EncodeRipOffset32 ( X64OP_DIV, MODRM_DIV, (char*) SrcPtr );
}

bool x64Encoder::DivRegMem64 ( long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( X64OP_DIV, MODRM_DIV, (char*) SrcPtr );
}



bool x64Encoder::DivRegMem16 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_DIV, MODRM_DIV, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::DivRegMem32 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_DIV, MODRM_DIV, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::DivRegMem64 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_DIV, MODRM_DIV, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::IdivRegReg16 ( long SrcReg )
{
	return x64EncodeReg16 ( X64OP_IDIV, MODRM_IDIV, SrcReg );
}

bool x64Encoder::IdivRegReg32 ( long SrcReg )
{
	return x64EncodeReg32 ( X64OP_IDIV, MODRM_IDIV, SrcReg );
}

bool x64Encoder::IdivRegReg64 ( long SrcReg )
{
	return x64EncodeReg64 ( X64OP_IDIV, MODRM_IDIV, SrcReg );
}

bool x64Encoder::IdivRegMem16 ( short* SrcPtr )
{
	return x64EncodeRipOffset16 ( X64OP_IDIV, MODRM_IDIV, (char*) SrcPtr );
}

bool x64Encoder::IdivRegMem32 ( long* SrcPtr )
{
	return x64EncodeRipOffset32 ( X64OP_IDIV, MODRM_IDIV, (char*) SrcPtr );
}

bool x64Encoder::IdivRegMem64 ( long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( X64OP_IDIV, MODRM_IDIV, (char*) SrcPtr );
}


bool x64Encoder::IdivRegMem16 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_IDIV, MODRM_IDIV, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::IdivRegMem32 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_IDIV, MODRM_IDIV, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::IdivRegMem64 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_IDIV, MODRM_IDIV, AddressReg, IndexReg, Scale, Offset );
}


bool x64Encoder::ImulRegReg16 ( long SrcReg )
{
	return x64EncodeReg16 ( X64OP_IMUL, MODRM_IMUL, SrcReg );
}

bool x64Encoder::ImulRegReg32 ( long SrcReg )
{
	return x64EncodeReg32 ( X64OP_IMUL, MODRM_IMUL, SrcReg );
}

bool x64Encoder::ImulRegReg64 ( long SrcReg )
{
	return x64EncodeReg64 ( X64OP_IMUL, MODRM_IMUL, SrcReg );
}

bool x64Encoder::ImulRegMem16 ( short* SrcPtr )
{
	return x64EncodeRipOffset16 ( X64OP_IMUL, MODRM_IMUL, (char*) SrcPtr );
}

bool x64Encoder::ImulRegMem32 ( long* SrcPtr )
{
	return x64EncodeRipOffset32 ( X64OP_IMUL, MODRM_IMUL, (char*) SrcPtr );
}

bool x64Encoder::ImulRegMem64 ( long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( X64OP_IMUL, MODRM_IMUL, (char*) SrcPtr );
}


bool x64Encoder::ImulRegMem16 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_IMUL, MODRM_IMUL, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::ImulRegMem32 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_IMUL, MODRM_IMUL, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::ImulRegMem64 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_IMUL, MODRM_IMUL, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::LeaRegRegReg16 ( long DestReg, long Src1Reg, long Src2Reg )
{
	return x64EncodeRegMem16 ( X64OP_LEA, DestReg, Src1Reg, Src2Reg, SCALE_NONE, 0 );
}

bool x64Encoder::LeaRegRegReg32 ( long DestReg, long Src1Reg, long Src2Reg )
{
	return x64EncodeRegMem32 ( X64OP_LEA, DestReg, Src1Reg, Src2Reg, SCALE_NONE, 0 );
}

bool x64Encoder::LeaRegRegReg64 ( long DestReg, long Src1Reg, long Src2Reg )
{
	return x64EncodeRegMem64 ( X64OP_LEA, DestReg, Src1Reg, Src2Reg, SCALE_NONE, 0 );
}

bool x64Encoder::LeaRegMem64 ( long DestReg, void* SrcPtr )
{
	return x64EncodeRipOffset64 ( X64OP_LEA, DestReg, (char*) SrcPtr, true );
}


bool x64Encoder::LeaRegRegImm16 ( long DestReg, long SrcReg, long Imm16 )
{
	return x64EncodeRegMem16 ( X64OP_LEA, DestReg, SrcReg, NO_INDEX, SCALE_NONE, Imm16 );
}

bool x64Encoder::LeaRegRegImm32 ( long DestReg, long SrcReg, long Imm32 )
{
	return x64EncodeRegMem32 ( X64OP_LEA, DestReg, SrcReg, NO_INDEX, SCALE_NONE, Imm32 );
}

bool x64Encoder::LeaRegRegImm64 ( long DestReg, long SrcReg, long Imm32 )
{
	return x64EncodeRegMem64 ( X64OP_LEA, DestReg, SrcReg, NO_INDEX, SCALE_NONE, Imm32 );
}




bool x64Encoder::MovsxReg16Reg8 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_MOVSXB, X64OP2_MOVSXB ), DestReg, SrcReg );
}

bool x64Encoder::MovsxReg32Reg8 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_MOVSXB, X64OP2_MOVSXB ), DestReg, SrcReg );
}

bool x64Encoder::MovsxReg64Reg8 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_MOVSXB, X64OP2_MOVSXB ), DestReg, SrcReg );
}





//

bool x64Encoder::MovsxReg32Reg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_MOVSXH, X64OP2_MOVSXH ), DestReg, SrcReg );
}

bool x64Encoder::MovsxReg64Reg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_MOVSXH, X64OP2_MOVSXH ), DestReg, SrcReg );
}



bool x64Encoder::MovsxReg16Mem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( MAKE2OPCODE( X64OP1_MOVSXB, X64OP2_MOVSXB ), DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovsxReg32Mem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_MOVSXB, X64OP2_MOVSXB ), DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovsxReg64Mem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( MAKE2OPCODE( X64OP1_MOVSXB, X64OP2_MOVSXB ), DestReg, AddressReg, IndexReg, Scale, Offset );
}




bool x64Encoder::MovsxReg16Mem8 ( long DestReg, char* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_MOVSXB, X64OP2_MOVSXB ), DestReg, (char*)SrcPtr, true );
}

bool x64Encoder::MovsxReg32Mem8 ( long DestReg, char* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_MOVSXB, X64OP2_MOVSXB ), DestReg, (char*)SrcPtr, true );
}

bool x64Encoder::MovsxReg64Mem8 ( long DestReg, char* SrcPtr )
{
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_MOVSXB, X64OP2_MOVSXB ), DestReg, (char*)SrcPtr, true );
}

bool x64Encoder::MovsxReg32Mem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_MOVSXH, X64OP2_MOVSXH ), DestReg, (char*)SrcPtr, true );
}

bool x64Encoder::MovsxReg64Mem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_MOVSXH, X64OP2_MOVSXH ), DestReg, (char*)SrcPtr, true );
}






bool x64Encoder::MovsxdReg64Reg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( X64OP_MOVSXD, DestReg, SrcReg );
}

bool x64Encoder::MovsxdReg64Mem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset64 ( X64OP_MOVSXD, DestReg, (char*) SrcPtr, true );
}







bool x64Encoder::MovsxReg32Mem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_MOVSXH, X64OP2_MOVSXH ), DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovsxReg64Mem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( MAKE2OPCODE( X64OP1_MOVSXH, X64OP2_MOVSXH ), DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovsxdReg64Mem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_MOVSXD, DestReg, AddressReg, IndexReg, Scale, Offset );
}



// -----------------------------------------

bool x64Encoder::MovzxReg16Reg8 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( MAKE2OPCODE( X64OP1_MOVZXB, X64OP2_MOVZXB ), DestReg, SrcReg );
}

bool x64Encoder::MovzxReg32Reg8 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_MOVZXB, X64OP2_MOVZXB ), DestReg, SrcReg );
}

bool x64Encoder::MovzxReg64Reg8 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_MOVZXB, X64OP2_MOVZXB ), DestReg, SrcReg );
}



bool x64Encoder::MovzxReg16Mem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( MAKE2OPCODE( X64OP1_MOVZXB, X64OP2_MOVZXB ), DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovzxReg32Mem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_MOVZXB, X64OP2_MOVZXB ), DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovzxReg64Mem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( MAKE2OPCODE( X64OP1_MOVZXB, X64OP2_MOVZXB ), DestReg, AddressReg, IndexReg, Scale, Offset );
}



bool x64Encoder::MovzxReg32Reg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_MOVZXH, X64OP2_MOVZXH ), DestReg, SrcReg );
}

bool x64Encoder::MovzxReg64Reg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_MOVZXH, X64OP2_MOVZXH ), DestReg, SrcReg );
}



bool x64Encoder::MovzxReg32Mem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_MOVZXH, X64OP2_MOVZXH ), DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MovzxReg64Mem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( MAKE2OPCODE( X64OP1_MOVZXH, X64OP2_MOVZXH ), DestReg, AddressReg, IndexReg, Scale, Offset );
}





bool x64Encoder::MovzxReg16Mem8 ( long DestReg, char* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_MOVZXB, X64OP2_MOVZXB ), DestReg, (char*)SrcPtr, true );
}

bool x64Encoder::MovzxReg32Mem8 ( long DestReg, char* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_MOVZXB, X64OP2_MOVZXB ), DestReg, (char*)SrcPtr, true );
}

bool x64Encoder::MovzxReg64Mem8 ( long DestReg, char* SrcPtr )
{
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_MOVZXB, X64OP2_MOVZXB ), DestReg, (char*)SrcPtr, true );
}

bool x64Encoder::MovzxReg32Mem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_MOVZXH, X64OP2_MOVZXH ), DestReg, (char*)SrcPtr, true );
}

bool x64Encoder::MovzxReg64Mem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_MOVZXH, X64OP2_MOVZXH ), DestReg, (char*)SrcPtr, true );
}






bool x64Encoder::MulRegReg16 ( long SrcReg )
{
	return x64EncodeReg16 ( X64OP_MUL, MODRM_MUL, SrcReg );
}

bool x64Encoder::MulRegReg32 ( long SrcReg )
{
	return x64EncodeReg32 ( X64OP_MUL, MODRM_MUL, SrcReg );
}

bool x64Encoder::MulRegReg64 ( long SrcReg )
{
	return x64EncodeReg64 ( X64OP_MUL, MODRM_MUL, SrcReg );
}

bool x64Encoder::MulRegMem16 ( short* SrcPtr )
{
	return x64EncodeRipOffset16 ( X64OP_MUL, MODRM_MUL, (char*) SrcPtr );
}

bool x64Encoder::MulRegMem32 ( long* SrcPtr )
{
	return x64EncodeRipOffset32 ( X64OP_MUL, MODRM_MUL, (char*) SrcPtr );
}

bool x64Encoder::MulRegMem64 ( long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( X64OP_MUL, MODRM_MUL, (char*) SrcPtr );
}


bool x64Encoder::MulRegMem16 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_MUL, MODRM_MUL, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MulRegMem32 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_MUL, MODRM_MUL, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::MulRegMem64 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_MUL, MODRM_MUL, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::NegReg16 ( long DestReg )
{
	return x64EncodeReg16 ( X64OP_NEG, MODRM_NEG, DestReg );
}

bool x64Encoder::NegReg32 ( long DestReg )
{
	return x64EncodeReg32 ( X64OP_NEG, MODRM_NEG, DestReg );
}

bool x64Encoder::NegReg64 ( long DestReg )
{
	return x64EncodeReg64 ( X64OP_NEG, MODRM_NEG, DestReg );
}

bool x64Encoder::NegMem16 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_NEG, MODRM_NEG, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::NegMem32 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_NEG, MODRM_NEG, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::NegMem64 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_NEG, MODRM_NEG, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::NegMem16 ( short* DestPtr )
{
	return x64EncodeRipOffset16 ( X64OP_NEG, MODRM_NEG, (char*) DestPtr );
}

bool x64Encoder::NegMem32 ( long* DestPtr )
{
	return x64EncodeRipOffset32 ( X64OP_NEG, MODRM_NEG, (char*) DestPtr );
}

bool x64Encoder::NegMem64 ( long long* DestPtr )
{
	return x64EncodeRipOffset64 ( X64OP_NEG, MODRM_NEG, (char*) DestPtr );
}




bool x64Encoder::NotReg16 ( long DestReg )
{
	return x64EncodeReg16 ( X64OP_NOT, MODRM_NOT, DestReg );
}

bool x64Encoder::NotReg32 ( long DestReg )
{
	return x64EncodeReg32 ( X64OP_NOT, MODRM_NOT, DestReg );
}

bool x64Encoder::NotReg64 ( long DestReg )
{
	return x64EncodeReg64 ( X64OP_NOT, MODRM_NOT, DestReg );
}

bool x64Encoder::NotMem16 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_NOT, MODRM_NOT, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::NotMem32 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_NOT, MODRM_NOT, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::NotMem64 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_NOT, MODRM_NOT, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::NotMem16 ( short* DestPtr )
{
	return x64EncodeRipOffset16 ( X64OP_NOT, MODRM_NOT, (char*) DestPtr );
}

bool x64Encoder::NotMem32 ( long* DestPtr )
{
	return x64EncodeRipOffset32 ( X64OP_NOT, MODRM_NOT, (char*) DestPtr );
}

bool x64Encoder::NotMem64 ( long long* DestPtr )
{
	return x64EncodeRipOffset64 ( X64OP_NOT, MODRM_NOT, (char*) DestPtr );
}




bool x64Encoder::IncReg16 ( long DestReg )
{
	return x64EncodeReg16 ( X64OP_INC, MODRM_INC, DestReg );
}

bool x64Encoder::IncReg32 ( long DestReg )
{
	return x64EncodeReg32 ( X64OP_INC, MODRM_INC, DestReg );
}

bool x64Encoder::IncReg64 ( long DestReg )
{
	return x64EncodeReg64 ( X64OP_INC, MODRM_INC, DestReg );
}

bool x64Encoder::IncMem16 ( short* DestPtr )
{
	return x64EncodeRipOffset16 ( X64OP_INC, MODRM_INC, (char*) DestPtr );
}

bool x64Encoder::IncMem32 ( long* DestPtr )
{
	return x64EncodeRipOffset32 ( X64OP_INC, MODRM_INC, (char*) DestPtr );
}

bool x64Encoder::IncMem64 ( long long* DestPtr )
{
	return x64EncodeRipOffset64 ( X64OP_INC, MODRM_INC, (char*) DestPtr );
}



bool x64Encoder::DecReg16 ( long DestReg )
{
	return x64EncodeReg16 ( X64OP_DEC, MODRM_DEC, DestReg );
}

bool x64Encoder::DecReg32 ( long DestReg )
{
	return x64EncodeReg32 ( X64OP_DEC, MODRM_DEC, DestReg );
}

bool x64Encoder::DecReg64 ( long DestReg )
{
	return x64EncodeReg64 ( X64OP_DEC, MODRM_DEC, DestReg );
}

bool x64Encoder::DecMem16 ( short* DestPtr )
{
	return x64EncodeRipOffset16 ( X64OP_DEC, MODRM_DEC, (char*) DestPtr );
}

bool x64Encoder::DecMem32 ( long* DestPtr )
{
	return x64EncodeRipOffset32 ( X64OP_DEC, MODRM_DEC, (char*) DestPtr );
}

bool x64Encoder::DecMem64 ( long long* DestPtr )
{
	return x64EncodeRipOffset64 ( X64OP_DEC, MODRM_DEC, (char*) DestPtr );
}




bool x64Encoder::OrRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( X64OP_OR, DestReg, SrcReg );
}

bool x64Encoder::OrRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( X64OP_OR, DestReg, SrcReg );
}

bool x64Encoder::OrRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( X64OP_OR, DestReg, SrcReg );
}

bool x64Encoder::OrRegImm16 ( long DestReg, short Imm16 )
{
	return x64EncodeReg16Imm16 ( X64OP_OR_IMM, MODRM_OR_IMM, DestReg, Imm16 );
}

bool x64Encoder::OrRegImm32 ( long DestReg, long Imm32 )
{
	return x64EncodeReg32Imm32 ( X64OP_OR_IMM, MODRM_OR_IMM, DestReg, Imm32 );
}

bool x64Encoder::OrRegImm64 ( long DestReg, long Imm32 )
{
	return x64EncodeReg64Imm32 ( X64OP_OR_IMM, MODRM_OR_IMM, DestReg, Imm32 );
}

bool x64Encoder::OrRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_OR, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::OrRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_OR, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::OrRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_OR, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::OrMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_OR_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::OrMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_OR_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::OrMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_OR_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::OrMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm16 ( X64OP_OR_IMM, MODRM_OR_IMM, Imm16, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::OrMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm32 ( X64OP_OR_IMM, MODRM_OR_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::OrMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm64 ( X64OP_OR_IMM, MODRM_OR_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::OrRegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( X64OP_OR, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::OrRegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( X64OP_OR, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::OrRegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( X64OP_OR, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::OrMemReg8 ( char* DestPtr, int SrcReg )
{
	return x64EncodeRipOffset32 ( X64OP_OR_TOMEM8, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::OrMemReg16 ( short* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset16 ( X64OP_OR_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::OrMemReg32 ( long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset32 ( X64OP_OR_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::OrMemReg64 ( long long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset64 ( X64OP_OR_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::OrMemImm16 ( short* DestPtr, short Imm16 )
{
	return x64EncodeRipOffsetImm16 ( X64OP_OR_IMM, MODRM_OR_IMM, (char*) DestPtr, Imm16 );
}

bool x64Encoder::OrMemImm32 ( long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm32 ( X64OP_OR_IMM, MODRM_OR_IMM, (char*) DestPtr, Imm32 );
}

bool x64Encoder::OrMemImm64 ( long long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm64 ( X64OP_OR_IMM, MODRM_OR_IMM, (char*) DestPtr, Imm32 );
}


bool x64Encoder::OrAcc16Imm16 ( short Imm16 )
{
	return x64EncodeAcc16Imm16 ( X64OP_OR_IMMA, Imm16 );
}

bool x64Encoder::OrAcc32Imm32 ( long Imm32 )
{
	return x64EncodeAcc32Imm32 ( X64OP_OR_IMMA, Imm32 );
}

bool x64Encoder::OrAcc64Imm32 ( long Imm32 )
{
	return x64EncodeAcc64Imm32 ( X64OP_OR_IMMA, Imm32 );
}


bool x64Encoder::OrReg16Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg16Imm8 ( X64OP_OR_IMM8, MODRM_OR_IMM8, DestReg, Imm8 );
}

bool x64Encoder::OrReg32Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg32Imm8 ( X64OP_OR_IMM8, MODRM_OR_IMM8, DestReg, Imm8 );
}

bool x64Encoder::OrReg64Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg64Imm8 ( X64OP_OR_IMM8, MODRM_OR_IMM8, DestReg, Imm8 );
}

bool x64Encoder::OrMem16Imm8 ( short* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset16Imm8 ( X64OP_OR_IMM8, MODRM_OR_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::OrMem32Imm8 ( long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset32Imm8 ( X64OP_OR_IMM8, MODRM_OR_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::OrMem64Imm8 ( long long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset64Imm8 ( X64OP_OR_IMM8, MODRM_OR_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::OrReg16ImmX ( long DestReg, short ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return OrReg16Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return OrRegImm16 ( DestReg, ImmX );
	}
	
	return OrAcc16Imm16 ( ImmX );
}

bool x64Encoder::OrReg32ImmX ( long DestReg, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return OrReg32Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return OrRegImm32 ( DestReg, ImmX );
	}
	
	return OrAcc32Imm32 ( ImmX );
}

bool x64Encoder::OrReg64ImmX ( long DestReg, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return OrReg64Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return OrRegImm64 ( DestReg, ImmX );
	}
	
	return OrAcc64Imm32 ( ImmX );
}


bool x64Encoder::OrMem16ImmX ( short* DestPtr, short ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset16Imm8 ( X64OP_OR_IMM8, MODRM_OR_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm16 ( X64OP_OR_IMM, MODRM_OR_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::OrMem32ImmX ( long* DestPtr, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset32Imm8 ( X64OP_OR_IMM8, MODRM_OR_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm32 ( X64OP_OR_IMM, MODRM_OR_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::OrMem64ImmX ( long long* DestPtr, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset64Imm8 ( X64OP_OR_IMM8, MODRM_OR_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm64 ( X64OP_OR_IMM, MODRM_OR_IMM, (char*) DestPtr, ImmX );
}








bool x64Encoder::PopReg64 ( long SrcReg )
{
	x64EncodeRexReg64 ( 0, SrcReg );
	return x64EncodeOpcode ( X64BOP_POP + ( SrcReg & 7 ) );
}
	
bool x64Encoder::PushReg64 ( long SrcReg )
{
	x64EncodeRexReg64 ( 0, SrcReg );
	return x64EncodeOpcode ( X64BOP_PUSH + ( SrcReg & 7 ) );
}

bool x64Encoder::PushImm8 ( char Imm8 )
{
	x64EncodeOpcode ( X64OP_PUSH_IMM8 );

	if ( x64CurrentCodeBlockSpaceRemaining () <= 0 ) return false;
	
	x64CodeArea [ x64NextOffset++ ] = Imm8;
	
	return true;
}

bool x64Encoder::PushImm16 ( short Imm16 )
{
	x64Encode16Bit ();
	x64EncodeOpcode ( X64OP_PUSH_IMM );
	return x64EncodeImmediate16 ( Imm16 );
}

bool x64Encoder::PushImm32 ( long Imm32 )
{
	x64EncodeOpcode ( X64OP_PUSH_IMM );
	return x64EncodeImmediate32 ( Imm32 );
}

bool x64Encoder::Ret ( void )
{
	return x64EncodeReturn ();
}

// seta - set if unsigned above
bool x64Encoder::Set_A ( int DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETA, X64OP2_SETA ), 0, DestReg );
}

bool x64Encoder::Set_A ( void* DestPtr8 )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_SETA, X64OP2_SETA ), 0, (char*) DestPtr8 );
}

bool x64Encoder::Set_A ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_SETA, X64OP2_SETA ), 0, AddressReg, IndexReg, Scale, Offset );
}

// setae - set if unsigned above or equal
bool x64Encoder::Set_AE ( int DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETAE, X64OP2_SETAE ), 0, DestReg );
}

bool x64Encoder::Set_AE ( void* DestPtr8 )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_SETAE, X64OP2_SETAE ), 0, (char*) DestPtr8 );
}

bool x64Encoder::Set_AE ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_SETAE, X64OP2_SETAE ), 0, AddressReg, IndexReg, Scale, Offset );
}

// setb - set if unsigned below
bool x64Encoder::Set_B ( int DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETB, X64OP2_SETB ), 0, DestReg );
}

bool x64Encoder::Set_B ( void* DestPtr8 )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_SETB, X64OP2_SETB ), 0, (char*) DestPtr8 );
}

bool x64Encoder::Set_B ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_SETB, X64OP2_SETB ), 0, AddressReg, IndexReg, Scale, Offset );
}

// setbe - set if unsigned below or equal
bool x64Encoder::Set_BE ( int DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETBE, X64OP2_SETBE ), 0, DestReg );
}

bool x64Encoder::Set_BE ( void* DestPtr8 )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_SETBE, X64OP2_SETBE ), 0, (char*) DestPtr8 );
}

bool x64Encoder::Set_BE ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_SETBE, X64OP2_SETBE ), 0, AddressReg, IndexReg, Scale, Offset );
}

// sete - set if equal
bool x64Encoder::Set_E ( int DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETE, X64OP2_SETE ), 0, DestReg );
}

bool x64Encoder::Set_E ( void* DestPtr8 )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_SETE, X64OP2_SETE ), 0, (char*) DestPtr8 );
}

bool x64Encoder::Set_E ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_SETE, X64OP2_SETE ), 0, AddressReg, IndexReg, Scale, Offset );
}

// setne - set if equal
bool x64Encoder::Set_NE ( int DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETNE, X64OP2_SETNE ), 0, DestReg );
}

bool x64Encoder::Set_NE ( void* DestPtr8 )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_SETNE, X64OP2_SETNE ), 0, (char*) DestPtr8 );
}

bool x64Encoder::Set_NE ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_SETNE, X64OP2_SETNE ), 0, AddressReg, IndexReg, Scale, Offset );
}

// signed versions

// setg - set if signed greater
bool x64Encoder::Set_G ( int DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETG, X64OP2_SETG ), 0, DestReg );
}

bool x64Encoder::Set_G ( void* DestPtr8 )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_SETG, X64OP2_SETG ), 0, (char*) DestPtr8 );
}

bool x64Encoder::Set_G ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_SETG, X64OP2_SETG ), 0, AddressReg, IndexReg, Scale, Offset );
}

// setge - set if signed greater
bool x64Encoder::Set_GE ( int DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETGE, X64OP2_SETGE ), 0, DestReg );
}

bool x64Encoder::Set_GE ( void* DestPtr8 )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_SETGE, X64OP2_SETGE ), 0, (char*) DestPtr8 );
}

bool x64Encoder::Set_GE ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_SETGE, X64OP2_SETGE ), 0, AddressReg, IndexReg, Scale, Offset );
}

// setl - set if signed greater
bool x64Encoder::Set_L ( int DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETL, X64OP2_SETL ), 0, DestReg );
}

bool x64Encoder::Set_L ( void* DestPtr8 )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_SETL, X64OP2_SETL ), 0, (char*) DestPtr8 );
}

bool x64Encoder::Set_L ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_SETL, X64OP2_SETL ), 0, AddressReg, IndexReg, Scale, Offset );
}

// setle - set if signed greater
bool x64Encoder::Set_LE ( int DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETLE, X64OP2_SETLE ), 0, DestReg );
}

bool x64Encoder::Set_LE ( void* DestPtr8 )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_SETLE, X64OP2_SETLE ), 0, (char*) DestPtr8 );
}

bool x64Encoder::Set_LE ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_SETLE, X64OP2_SETLE ), 0, AddressReg, IndexReg, Scale, Offset );
}


// sets - set if signed
bool x64Encoder::Set_S ( int DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETS, X64OP2_SETS ), 0, DestReg );
}

bool x64Encoder::Set_S ( void* DestPtr8 )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_SETS, X64OP2_SETS ), 0, (char*) DestPtr8 );
}

bool x64Encoder::Set_S ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_SETS, X64OP2_SETS ), 0, AddressReg, IndexReg, Scale, Offset );
}


// setpo - set if parity odd
bool x64Encoder::Set_PO ( int DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETPO, X64OP2_SETPO ), 0, DestReg );
}

bool x64Encoder::Set_PO ( void* DestPtr8 )
{
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_SETPO, X64OP2_SETPO ), 0, (char*) DestPtr8 );
}

bool x64Encoder::Set_PO ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_SETPO, X64OP2_SETPO ), 0, AddressReg, IndexReg, Scale, Offset );
}


bool x64Encoder::Setb ( long DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETB, X64OP2_SETB ), 0, DestReg );
}

bool x64Encoder::Setl ( long DestReg )
{
	return x64EncodeReg32 ( MAKE2OPCODE( X64OP1_SETL, X64OP2_SETL ), 0, DestReg );
}

bool x64Encoder::Setl ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( MAKE2OPCODE( X64OP1_SETL, X64OP2_SETL ), 0, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::SarRegReg16 ( long DestReg )
{
	return x64EncodeReg16 ( X64OP_SAR, MODRM_SAR, DestReg );
}

bool x64Encoder::SarRegReg32 ( long DestReg )
{
	return x64EncodeReg32 ( X64OP_SAR, MODRM_SAR, DestReg );
}

bool x64Encoder::SarRegReg64 ( long DestReg )
{
	return x64EncodeReg64 ( X64OP_SAR, MODRM_SAR, DestReg );
}

bool x64Encoder::SarRegImm16 ( long DestReg, char Imm8 )
{
	if ( !Imm8 ) return true;
	
	return x64EncodeReg16Imm8 ( X64OP_SAR_IMM, MODRM_SAR_IMM, DestReg, Imm8 );
}

bool x64Encoder::SarRegImm32 ( long DestReg, char Imm8 )
{
	if ( !Imm8 ) return true;
	
	return x64EncodeReg32Imm8 ( X64OP_SAR_IMM, MODRM_SAR_IMM, DestReg, Imm8 );
}

bool x64Encoder::SarRegImm64 ( long DestReg, char Imm8 )
{
	if ( !Imm8 ) return true;
	
	return x64EncodeReg64Imm8 ( X64OP_SAR_IMM, MODRM_SAR_IMM, DestReg, Imm8 );
}

bool x64Encoder::SarMemReg32 ( long* DestPtr )
{
	return x64EncodeRipOffset32 ( X64OP_SAR, MODRM_SAR, (char*) DestPtr );
}


bool x64Encoder::ShlRegReg16 ( long DestReg )
{
	return x64EncodeReg16 ( X64OP_SHL, MODRM_SHL, DestReg );
}

bool x64Encoder::ShlRegReg32 ( long DestReg )
{
	return x64EncodeReg32 ( X64OP_SHL, MODRM_SHL, DestReg );
}

bool x64Encoder::ShlRegReg64 ( long DestReg )
{
	return x64EncodeReg64 ( X64OP_SHL, MODRM_SHL, DestReg );
}

bool x64Encoder::ShlRegImm16 ( long DestReg, char Imm8 )
{
	if ( !Imm8 ) return true;
	
	return x64EncodeReg16Imm8 ( X64OP_SHL_IMM, MODRM_SHL_IMM, DestReg, Imm8 );
}

bool x64Encoder::ShlRegImm32 ( long DestReg, char Imm8 )
{
	if ( !Imm8 ) return true;
	
	return x64EncodeReg32Imm8 ( X64OP_SHL_IMM, MODRM_SHL_IMM, DestReg, Imm8 );
}

bool x64Encoder::ShlRegImm64 ( long DestReg, char Imm8 )
{
	if ( !Imm8 ) return true;
	
	return x64EncodeReg64Imm8 ( X64OP_SHL_IMM, MODRM_SHL_IMM, DestReg, Imm8 );
}

bool x64Encoder::ShlMemReg32 ( long* DestPtr )
{
	return x64EncodeRipOffset32 ( X64OP_SHL, MODRM_SHL, (char*) DestPtr );
}


bool x64Encoder::ShlMemReg16 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_SHL, MODRM_SHL, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::ShlMemReg32 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_SHL, MODRM_SHL, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::ShlMemReg64 ( long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_SHL, MODRM_SHL, AddressReg, IndexReg, Scale, Offset );
}

//bool x64Encoder::ShlMemImm16 ( long AddressReg, long IndexReg, long Scale, long Offset, char Imm8 )
//{
//}

//bool x64Encoder::ShlMemImm32 ( long AddressReg, long IndexReg, long Scale, long Offset, char Imm8 )
//{
//}

//bool x64Encoder::ShlMemImm64 ( long AddressReg, long IndexReg, long Scale, long Offset, char Imm8 )
//{
//}







bool x64Encoder::ShrRegReg16 ( long DestReg )
{
	return x64EncodeReg16 ( X64OP_SHR, MODRM_SHR, DestReg );
}

bool x64Encoder::ShrRegReg32 ( long DestReg )
{
	return x64EncodeReg32 ( X64OP_SHR, MODRM_SHR, DestReg );
}

bool x64Encoder::ShrRegReg64 ( long DestReg )
{
	return x64EncodeReg64 ( X64OP_SHR, MODRM_SHR, DestReg );
}

bool x64Encoder::ShrRegImm16 ( long DestReg, char Imm8 )
{
	if ( !Imm8 ) return true;
	
	return x64EncodeReg16Imm8 ( X64OP_SHR_IMM, MODRM_SHR_IMM, DestReg, Imm8 );
}

bool x64Encoder::ShrRegImm32 ( long DestReg, char Imm8 )
{
	if ( !Imm8 ) return true;
	
	return x64EncodeReg32Imm8 ( X64OP_SHR_IMM, MODRM_SHR_IMM, DestReg, Imm8 );
}

bool x64Encoder::ShrRegImm64 ( long DestReg, char Imm8 )
{
	if ( !Imm8 ) return true;
	
	return x64EncodeReg64Imm8 ( X64OP_SHR_IMM, MODRM_SHR_IMM, DestReg, Imm8 );
}


bool x64Encoder::ShrMemReg32 ( long* DestPtr )
{
	return x64EncodeRipOffset32 ( X64OP_SHR, MODRM_SHR, (char*) DestPtr );
}





bool x64Encoder::SubRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( X64OP_SUB, DestReg, SrcReg );
}

bool x64Encoder::SubRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( X64OP_SUB, DestReg, SrcReg );
}

bool x64Encoder::SubRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( X64OP_SUB, DestReg, SrcReg );
}

bool x64Encoder::SubRegImm16 ( long DestReg, short Imm16 )
{
	return x64EncodeReg16Imm16 ( X64OP_SUB_IMM, MODRM_SUB_IMM, DestReg, Imm16 );
}

bool x64Encoder::SubRegImm32 ( long DestReg, long Imm32 )
{
	return x64EncodeReg32Imm32 ( X64OP_SUB_IMM, MODRM_SUB_IMM, DestReg, Imm32 );
}

bool x64Encoder::SubRegImm64 ( long DestReg, long Imm32 )
{
	return x64EncodeReg64Imm32 ( X64OP_SUB_IMM, MODRM_SUB_IMM, DestReg, Imm32 );
}

bool x64Encoder::SubRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_SUB, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::SubRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_SUB, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::SubRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_SUB, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::SubMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_SUB_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::SubMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_SUB_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::SubMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_SUB_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::SubMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm16 ( X64OP_SUB_IMM, MODRM_SUB_IMM, Imm16, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::SubMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm32 ( X64OP_SUB_IMM, MODRM_SUB_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::SubMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm64 ( X64OP_SUB_IMM, MODRM_SUB_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}



bool x64Encoder::SubRegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( X64OP_SUB, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::SubRegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( X64OP_SUB, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::SubRegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( X64OP_SUB, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::SubMemReg16 ( short* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset16 ( X64OP_SUB_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::SubMemReg32 ( long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset32 ( X64OP_SUB_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::SubMemReg64 ( long long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset64 ( X64OP_SUB_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::SubMemImm16 ( short* DestPtr, short Imm16 )
{
	return x64EncodeRipOffsetImm16 ( X64OP_SUB_IMM, MODRM_SUB_IMM, (char*) DestPtr, Imm16 );
}

bool x64Encoder::SubMemImm32 ( long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm32 ( X64OP_SUB_IMM, MODRM_SUB_IMM, (char*) DestPtr, Imm32 );
}

bool x64Encoder::SubMemImm64 ( long long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm64 ( X64OP_SUB_IMM, MODRM_SUB_IMM, (char*) DestPtr, Imm32 );
}


bool x64Encoder::SubAcc16Imm16 ( short Imm16 )
{
	return x64EncodeAcc16Imm16 ( X64OP_SUB_IMMA, Imm16 );
}

bool x64Encoder::SubAcc32Imm32 ( long Imm32 )
{
	return x64EncodeAcc32Imm32 ( X64OP_SUB_IMMA, Imm32 );
}

bool x64Encoder::SubAcc64Imm32 ( long Imm32 )
{
	return x64EncodeAcc64Imm32 ( X64OP_SUB_IMMA, Imm32 );
}


bool x64Encoder::SubReg16Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg16Imm8 ( X64OP_SUB_IMM8, MODRM_SUB_IMM8, DestReg, Imm8 );
}

bool x64Encoder::SubReg32Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg32Imm8 ( X64OP_SUB_IMM8, MODRM_SUB_IMM8, DestReg, Imm8 );
}

bool x64Encoder::SubReg64Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg64Imm8 ( X64OP_SUB_IMM8, MODRM_SUB_IMM8, DestReg, Imm8 );
}

bool x64Encoder::SubMem16Imm8 ( short* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset16Imm8 ( X64OP_SUB_IMM8, MODRM_SUB_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::SubMem32Imm8 ( long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset32Imm8 ( X64OP_SUB_IMM8, MODRM_SUB_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::SubMem64Imm8 ( long long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset64Imm8 ( X64OP_SUB_IMM8, MODRM_SUB_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::SubReg16ImmX ( long DestReg, short ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX == 1 )
	{
		return DecReg16 ( DestReg );
	}

	if ( ImmX == -1 )
	{
		return IncReg16 ( DestReg );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return SubReg16Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return SubRegImm16 ( DestReg, ImmX );
	}
	
	return SubAcc16Imm16 ( ImmX );
}

bool x64Encoder::SubReg32ImmX ( long DestReg, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX == 1 )
	{
		return DecReg32 ( DestReg );
	}

	if ( ImmX == -1 )
	{
		return IncReg32 ( DestReg );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return SubReg32Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return SubRegImm32 ( DestReg, ImmX );
	}
	
	return SubAcc32Imm32 ( ImmX );
}

bool x64Encoder::SubReg64ImmX ( long DestReg, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX == 1 )
	{
		return DecReg64 ( DestReg );
	}

	if ( ImmX == -1 )
	{
		return IncReg64 ( DestReg );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return SubReg64Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return SubRegImm64 ( DestReg, ImmX );
	}
	
	return SubAcc64Imm32 ( ImmX );
}


bool x64Encoder::SubMem16ImmX ( short* DestPtr, short ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX == 1 )
	{
		return DecMem16 ( DestPtr );
	}

	if ( ImmX == -1 )
	{
		return IncMem16 ( DestPtr );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset16Imm8 ( X64OP_SUB_IMM8, MODRM_SUB_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm16 ( X64OP_SUB_IMM, MODRM_SUB_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::SubMem32ImmX ( long* DestPtr, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX == 1 )
	{
		return DecMem32 ( DestPtr );
	}

	if ( ImmX == -1 )
	{
		return IncMem32 ( DestPtr );
	}

	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset32Imm8 ( X64OP_SUB_IMM8, MODRM_SUB_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm32 ( X64OP_SUB_IMM, MODRM_SUB_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::SubMem64ImmX ( long long* DestPtr, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX == 1 )
	{
		return DecMem64 ( DestPtr );
	}

	if ( ImmX == -1 )
	{
		return IncMem64 ( DestPtr );
	}
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset64Imm8 ( X64OP_SUB_IMM8, MODRM_SUB_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm64 ( X64OP_SUB_IMM, MODRM_SUB_IMM, (char*) DestPtr, ImmX );
}




bool x64Encoder::TestRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( X64OP_TEST_TOMEM, DestReg, SrcReg );
}

bool x64Encoder::TestRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( X64OP_TEST_TOMEM, DestReg, SrcReg );
}

bool x64Encoder::TestRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( X64OP_TEST_TOMEM, DestReg, SrcReg );
}

bool x64Encoder::TestRegImm16 ( long DestReg, short Imm16 )
{
	return x64EncodeReg16Imm16 ( X64OP_TEST_IMM, MODRM_TEST_IMM, DestReg, Imm16 );
}

bool x64Encoder::TestRegImm32 ( long DestReg, long Imm32 )
{
	return x64EncodeReg32Imm32 ( X64OP_TEST_IMM, MODRM_TEST_IMM, DestReg, Imm32 );
}

bool x64Encoder::TestRegImm64 ( long DestReg, long Imm32 )
{
	return x64EncodeReg64Imm32 ( X64OP_TEST_IMM, MODRM_TEST_IMM, DestReg, Imm32 );
}

bool x64Encoder::TestMemImm16 ( short* DestPtr, short Imm16 )
{
	return x64EncodeRipOffsetImm16 ( X64OP_TEST_IMM, MODRM_TEST_IMM, (char*) DestPtr, Imm16 );
}

bool x64Encoder::TestMemImm32 ( long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm32 ( X64OP_TEST_IMM, MODRM_TEST_IMM, (char*) DestPtr, Imm32 );
}

bool x64Encoder::TestMemImm64 ( long long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm64 ( X64OP_TEST_IMM, MODRM_TEST_IMM, (char*) DestPtr, Imm32 );
}


bool x64Encoder::TestRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_TEST_TOMEM, DestReg, AddressReg, IndexReg, Scale, Offset );
}
bool x64Encoder::TestRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_TEST_TOMEM, DestReg, AddressReg, IndexReg, Scale, Offset );
}
bool x64Encoder::TestRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_TEST_TOMEM, DestReg, AddressReg, IndexReg, Scale, Offset );
}


bool x64Encoder::TestAcc16Imm16 ( short Imm16 )
{
	return x64EncodeAcc16Imm16 ( X64OP_TEST_IMMA, Imm16 );
}

bool x64Encoder::TestAcc32Imm32 ( long Imm32 )
{
	return x64EncodeAcc32Imm32 ( X64OP_TEST_IMMA, Imm32 );
}

bool x64Encoder::TestAcc64Imm32 ( long Imm32 )
{
	return x64EncodeAcc64Imm32 ( X64OP_TEST_IMMA, Imm32 );
}


bool x64Encoder::TestReg16Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg16Imm8 ( X64OP_TEST_IMM8, MODRM_TEST_IMM8, DestReg, Imm8 );
}

bool x64Encoder::TestReg32Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg32Imm8 ( X64OP_TEST_IMM8, MODRM_TEST_IMM8, DestReg, Imm8 );
}

bool x64Encoder::TestReg64Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg64Imm8 ( X64OP_TEST_IMM8, MODRM_TEST_IMM8, DestReg, Imm8 );
}

bool x64Encoder::TestMem16Imm8 ( short* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset16Imm8 ( X64OP_TEST_IMM8, MODRM_TEST_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::TestMem32Imm8 ( long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset32Imm8 ( X64OP_TEST_IMM8, MODRM_TEST_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::TestMem64Imm8 ( long long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset64Imm8 ( X64OP_TEST_IMM8, MODRM_TEST_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::TestReg16ImmX ( long DestReg, short ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return TestReg16Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return TestRegImm16 ( DestReg, ImmX );
	}
	
	return TestAcc16Imm16 ( ImmX );
}

bool x64Encoder::TestReg32ImmX ( long DestReg, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return TestReg32Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return TestRegImm32 ( DestReg, ImmX );
	}
	
	return TestAcc32Imm32 ( ImmX );
}

bool x64Encoder::TestReg64ImmX ( long DestReg, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return TestReg64Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return TestRegImm64 ( DestReg, ImmX );
	}
	
	return TestAcc64Imm32 ( ImmX );
}


bool x64Encoder::TestMem16ImmX ( short* DestPtr, short ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset16Imm8 ( X64OP_TEST_IMM8, MODRM_TEST_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm16 ( X64OP_TEST_IMM, MODRM_TEST_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::TestMem32ImmX ( long* DestPtr, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset32Imm8 ( X64OP_TEST_IMM8, MODRM_TEST_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm32 ( X64OP_TEST_IMM, MODRM_TEST_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::TestMem64ImmX ( long long* DestPtr, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset64Imm8 ( X64OP_TEST_IMM8, MODRM_TEST_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm64 ( X64OP_TEST_IMM, MODRM_TEST_IMM, (char*) DestPtr, ImmX );
}





bool x64Encoder::XchgRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( X64OP_XCHG, DestReg, SrcReg );
}

bool x64Encoder::XchgRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( X64OP_XCHG, DestReg, SrcReg );
}

bool x64Encoder::XchgRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( X64OP_XCHG, DestReg, SrcReg );
}


bool x64Encoder::XorRegReg16 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg16 ( X64OP_XOR, DestReg, SrcReg );
}

bool x64Encoder::XorRegReg32 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg32 ( X64OP_XOR, DestReg, SrcReg );
}

bool x64Encoder::XorRegReg64 ( long DestReg, long SrcReg )
{
	return x64EncodeRegReg64 ( X64OP_XOR, DestReg, SrcReg );
}

bool x64Encoder::XorRegImm16 ( long DestReg, short Imm16 )
{
	return x64EncodeReg16Imm16 ( X64OP_XOR_IMM, MODRM_XOR_IMM, DestReg, Imm16 );
}

bool x64Encoder::XorRegImm32 ( long DestReg, long Imm32 )
{
	return x64EncodeReg32Imm32 ( X64OP_XOR_IMM, MODRM_XOR_IMM, DestReg, Imm32 );
}

bool x64Encoder::XorRegImm64 ( long DestReg, long Imm32 )
{
	return x64EncodeReg64Imm32 ( X64OP_XOR_IMM, MODRM_XOR_IMM, DestReg, Imm32 );
}

bool x64Encoder::XorRegMem8 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_XOR8, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::XorRegMem16 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_XOR, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::XorRegMem32 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_XOR, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::XorRegMem64 ( long DestReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_XOR, DestReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::XorMemReg16 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem16 ( X64OP_XOR_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::XorMemReg32 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem32 ( X64OP_XOR_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::XorMemReg64 ( long SrcReg, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMem64 ( X64OP_XOR_TOMEM, SrcReg, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::XorMemImm8 ( char Imm8, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm8 ( X64OP_XOR8_IMM8, MODRM_XOR8_IMM8, Imm8, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::XorMemImm16 ( short Imm16, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm16 ( X64OP_XOR_IMM, MODRM_XOR_IMM, Imm16, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::XorMemImm32 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm32 ( X64OP_XOR_IMM, MODRM_XOR_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}

bool x64Encoder::XorMemImm64 ( long Imm32, long AddressReg, long IndexReg, long Scale, long Offset )
{
	return x64EncodeMemImm64 ( X64OP_XOR_IMM, MODRM_XOR_IMM, Imm32, AddressReg, IndexReg, Scale, Offset );
}



bool x64Encoder::XorRegMem16 ( long DestReg, short* SrcPtr )
{
	return x64EncodeRipOffset16 ( X64OP_XOR, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::XorRegMem32 ( long DestReg, long* SrcPtr )
{
	return x64EncodeRipOffset32 ( X64OP_XOR, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::XorRegMem64 ( long DestReg, long long* SrcPtr )
{
	return x64EncodeRipOffset64 ( X64OP_XOR, DestReg, (char*) SrcPtr, true );
}

bool x64Encoder::XorMemReg16 ( short* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset16 ( X64OP_XOR_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::XorMemReg32 ( long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset32 ( X64OP_XOR_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::XorMemReg64 ( long long* DestPtr, long SrcReg )
{
	return x64EncodeRipOffset64 ( X64OP_XOR_TOMEM, SrcReg, (char*) DestPtr, true );
}

bool x64Encoder::XorMemImm16 ( short* DestPtr, short Imm16 )
{
	return x64EncodeRipOffsetImm16 ( X64OP_XOR_IMM, MODRM_XOR_IMM, (char*) DestPtr, Imm16 );
}

bool x64Encoder::XorMemImm32 ( long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm32 ( X64OP_XOR_IMM, MODRM_XOR_IMM, (char*) DestPtr, Imm32 );
}

bool x64Encoder::XorMemImm64 ( long long* DestPtr, long Imm32 )
{
	return x64EncodeRipOffsetImm64 ( X64OP_XOR_IMM, MODRM_XOR_IMM, (char*) DestPtr, Imm32 );
}


bool x64Encoder::XorAcc16Imm16 ( short Imm16 )
{
	return x64EncodeAcc16Imm16 ( X64OP_XOR_IMMA, Imm16 );
}

bool x64Encoder::XorAcc32Imm32 ( long Imm32 )
{
	return x64EncodeAcc32Imm32 ( X64OP_XOR_IMMA, Imm32 );
}

bool x64Encoder::XorAcc64Imm32 ( long Imm32 )
{
	return x64EncodeAcc64Imm32 ( X64OP_XOR_IMMA, Imm32 );
}


bool x64Encoder::XorReg16Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg16Imm8 ( X64OP_XOR_IMM8, MODRM_XOR_IMM8, DestReg, Imm8 );
}

bool x64Encoder::XorReg32Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg32Imm8 ( X64OP_XOR_IMM8, MODRM_XOR_IMM8, DestReg, Imm8 );
}

bool x64Encoder::XorReg64Imm8 ( long DestReg, char Imm8 )
{
	return x64EncodeReg64Imm8 ( X64OP_XOR_IMM8, MODRM_XOR_IMM8, DestReg, Imm8 );
}

bool x64Encoder::XorMem16Imm8 ( short* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset16Imm8 ( X64OP_XOR_IMM8, MODRM_XOR_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::XorMem32Imm8 ( long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset32Imm8 ( X64OP_XOR_IMM8, MODRM_XOR_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::XorMem64Imm8 ( long long* DestPtr, char Imm8 )
{
	return x64EncodeRipOffset64Imm8 ( X64OP_XOR_IMM8, MODRM_XOR_IMM8, (char*) DestPtr, Imm8 );
}

bool x64Encoder::XorReg16ImmX ( long DestReg, short ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return XorReg16Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return XorRegImm16 ( DestReg, ImmX );
	}
	
	return XorAcc16Imm16 ( ImmX );
}

bool x64Encoder::XorReg32ImmX ( long DestReg, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return XorReg32Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return XorRegImm32 ( DestReg, ImmX );
	}
	
	return XorAcc32Imm32 ( ImmX );
}

bool x64Encoder::XorReg64ImmX ( long DestReg, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return XorReg64Imm8 ( DestReg, ImmX );
	}
	
	if ( DestReg )
	{
		return XorRegImm64 ( DestReg, ImmX );
	}
	
	return XorAcc64Imm32 ( ImmX );
}


bool x64Encoder::XorMem16ImmX ( short* DestPtr, short ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset16Imm8 ( X64OP_XOR_IMM8, MODRM_XOR_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm16 ( X64OP_XOR_IMM, MODRM_XOR_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::XorMem32ImmX ( long* DestPtr, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset32Imm8 ( X64OP_XOR_IMM8, MODRM_XOR_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm32 ( X64OP_XOR_IMM, MODRM_XOR_IMM, (char*) DestPtr, ImmX );
}

bool x64Encoder::XorMem64ImmX ( long long* DestPtr, long ImmX )
{
	if ( !ImmX ) return true;
	
	if ( ImmX >= -128 && ImmX <= 127 )
	{
		return x64EncodeRipOffset64Imm8 ( X64OP_XOR_IMM8, MODRM_XOR_IMM8, (char*) DestPtr, ImmX );
	}
	
	return x64EncodeRipOffsetImm64 ( X64OP_XOR_IMM, MODRM_XOR_IMM, (char*) DestPtr, ImmX );
}






// ** sse instruction definitions ** //

bool x64Encoder::movhlps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, VEX_PREFIXNONE, VEX_PREFIX0F, AVXOP_MOVHLPS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::movlhps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, VEX_PREFIXNONE, VEX_PREFIX0F, AVXOP_MOVLHPS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pinsrb ( long sseDestReg, long sseSrcReg1, long x64SrcReg2, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, 0, VEX_PREFIX66, VEX_PREFIX0F3A, AVXOP_PINSRB, sseDestReg, sseSrcReg1, x64SrcReg2, Imm8 );
}

bool x64Encoder::pinsrw ( long sseDestReg, long sseSrcReg1, long x64SrcReg2, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PINSRW, sseDestReg, sseSrcReg1, x64SrcReg2, Imm8 );
}

bool x64Encoder::pinsrd ( long sseDestReg, long sseSrcReg1, long x64SrcReg2, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, 0, VEX_PREFIX66, VEX_PREFIX0F3A, AVXOP_PINSRD, sseDestReg, sseSrcReg1, x64SrcReg2, Imm8 );
}

bool x64Encoder::pinsrq ( long sseDestReg, long sseSrcReg1, long x64SrcReg2, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, OPERAND_64BIT, VEX_PREFIX66, VEX_PREFIX0F3A, AVXOP_PINSRQ, sseDestReg, sseSrcReg1, x64SrcReg2, Imm8 );
}

bool x64Encoder::pextrb( long x64DestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, 0, VEX_PREFIX66, VEX_PREFIX0F3A, AVXOP_PEXTRB, sseSrcReg, 0, x64DestReg, Imm8 );
}

bool x64Encoder::pextrw( long x64DestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, 0, VEX_PREFIX66, VEX_PREFIX0F3A, AVXOP_PEXTRW, sseSrcReg, 0, x64DestReg, Imm8 );
}

bool x64Encoder::pextrd( long x64DestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, 0, VEX_PREFIX66, VEX_PREFIX0F3A, AVXOP_PEXTRD, sseSrcReg, 0, x64DestReg, Imm8 );
}

bool x64Encoder::pextrq( long x64DestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, OPERAND_64BIT, VEX_PREFIX66, VEX_PREFIX0F3A, AVXOP_PEXTRQ, sseSrcReg, 0, x64DestReg, Imm8 );
}
/*
bool x64Encoder::blendvps ( long sseDestReg, long sseSrcReg1, long x64SrcReg2, long x64SrcReg3 )
{
	return x64EncodeRegVImm8 ( 0, 0, VEX_PREFIX66, VEX_PREFIX0F3A, AVXOP_BLENDVPS, sseDestReg, sseSrcReg1, sseSrcReg2, ( x64SrcReg3 << 4 ) );
}
*/
bool x64Encoder::pblendw ( long sseDestReg, long sseSrcReg1, long sseSrcReg2, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, 0, VEX_PREFIX66, VEX_PREFIX0F3A, AVXOP_PBLENDW, sseDestReg, sseSrcReg1, sseSrcReg2, Imm8 );
}

bool x64Encoder::pblendwregregimm ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	x64EncodePrefix ( 0x66 );
	x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x3a, 0x0e ), sseDestReg, sseSrcReg );
	return x64EncodeImmediate8 ( Imm8 );
}

bool x64Encoder::pblendwregmemimm ( long sseDestReg, void* SrcPtr, char Imm8 )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32Imm8 ( MAKE3OPCODE( 0x0f, 0x3a, 0x0e ), sseDestReg, (char*) SrcPtr, Imm8 );
	//return x64EncodeImmediate8 ( Imm8 );
}

bool x64Encoder::pblendvbregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE4OPCODE( 0x66, 0x0f, 0x38, 0x10 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pblendvbregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE4OPCODE( 0x66, 0x0f, 0x38, 0x10 ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::movaps128 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIXNONE, VEX_PREFIX0F, AVXOP_MOVAPS_FROMMEM, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::movaps_to_mem128 ( void* DestPtr, long sseSrcReg )
{
	//return x64EncodeRegMemV ( VEX_128BIT, 0, VEX_PREFIXNONE, VEX_PREFIX0F, AVXOP_MOVAPS_TOMEM, sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
	//return x64EncodeRegMem32 ( MAKE2OPCODE ( 0x0f, AVXOP_MOVAPS_TOMEM ), sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, AVXOP_MOVAPS_TOMEM ), sseSrcReg, (char*) DestPtr );
}

bool x64Encoder::movaps_from_mem128 ( long sseDestReg, void* SrcPtr )
{
	//return x64EncodeRegMemV ( VEX_128BIT, 0, VEX_PREFIXNONE, VEX_PREFIX0F, AVXOP_MOVAPS_TOMEM, sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
	//return x64EncodeRegMem32 ( MAKE2OPCODE ( 0x0f, AVXOP_MOVAPS_TOMEM ), sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, AVXOP_MOVAPS_FROMMEM ), sseDestReg, (char*) SrcPtr );
}





bool x64Encoder::movdqa_regreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x6f ), sseDestReg, sseSrcReg );
}


bool x64Encoder::movdqa_memreg ( void* DestPtr, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x7f ), sseSrcReg, (char*) DestPtr );
}

bool x64Encoder::movdqa_regmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x6f ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::movdqa_to_mem128 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	//return x64EncodeRegMemV ( VEX_128BIT, 0, VEX_PREFIXNONE, VEX_PREFIX0F, AVXOP_MOVAPS_TOMEM, sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegMem32 ( MAKE2OPCODE ( 0x0f, 0x7f ), sseSrcReg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::movdqa_from_mem128 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	//return x64EncodeRegMemV ( VEX_128BIT, 0, VEX_PREFIXNONE, VEX_PREFIX0F, AVXOP_MOVAPS_FROMMEM, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegMem32 ( MAKE2OPCODE ( 0x0f, 0x6f ), sseDestReg, x64BaseReg, x64IndexReg, Scale, Offset );
}




bool x64Encoder::movdqu_regreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0xf3 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x6f ), sseDestReg, sseSrcReg );
}


bool x64Encoder::movdqu_memreg ( void* DestPtr, long sseSrcReg )
{
	x64EncodePrefix ( 0xf3 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x7f ), sseSrcReg, (char*) DestPtr );
}

bool x64Encoder::movdqu_regmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0xf3 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x6f ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::movdqu_to_mem128 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	//return x64EncodeRegMemV ( VEX_128BIT, 0, VEX_PREFIXNONE, VEX_PREFIX0F, AVXOP_MOVAPS_TOMEM, sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
	x64EncodePrefix ( 0xf3 );
	return x64EncodeRegMem32 ( MAKE2OPCODE ( 0x0f, 0x7f ), sseSrcReg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::movdqu_from_mem128 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	//return x64EncodeRegMemV ( VEX_128BIT, 0, VEX_PREFIXNONE, VEX_PREFIX0F, AVXOP_MOVAPS_FROMMEM, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
	x64EncodePrefix ( 0xf3 );
	return x64EncodeRegMem32 ( MAKE2OPCODE ( 0x0f, 0x6f ), sseDestReg, x64BaseReg, x64IndexReg, Scale, Offset );
}








bool x64Encoder::movaps_to_mem128 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	//return x64EncodeRegMemV ( VEX_128BIT, 0, VEX_PREFIXNONE, VEX_PREFIX0F, AVXOP_MOVAPS_TOMEM, sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
	return x64EncodeRegMem32 ( MAKE2OPCODE ( 0x0f, AVXOP_MOVAPS_TOMEM ), sseSrcReg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::movaps_from_mem128 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	//return x64EncodeRegMemV ( VEX_128BIT, 0, VEX_PREFIXNONE, VEX_PREFIX0F, AVXOP_MOVAPS_FROMMEM, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
	return x64EncodeRegMem32 ( MAKE2OPCODE ( 0x0f, AVXOP_MOVAPS_FROMMEM ), sseDestReg, x64BaseReg, x64IndexReg, Scale, Offset );
}



bool x64Encoder::vmovdqa_regreg128 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_MOVDQA_FROMMEM, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vmovdqa_regmem128 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_128BIT, 0, PP_VMOVDQA, MMMMM_VMOVDQA, AVXOP_MOVDQA_FROMMEM, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vmovdqa_memreg128 ( void* DestPtr, long sseSrcReg )
{
	return x64EncodeRipOffsetV ( VEX_128BIT, 0, PP_VMOVDQA, MMMMM_VMOVDQA, AVXOP_MOVDQA_TOMEM, sseSrcReg, 0, (char*) DestPtr );
}

bool x64Encoder::vmovdqa_memreg128 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( VEX_128BIT, 0, PP_VMOVDQA, MMMMM_VMOVDQA, AVXOP_MOVDQA_TOMEM, sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vmovdqa_regmem128 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( VEX_128BIT, 0, PP_VMOVDQA, MMMMM_VMOVDQA, AVXOP_MOVDQA_TOMEM, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}


bool x64Encoder::vmovdqa_regreg256 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_256BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_MOVDQA_FROMMEM, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vmovdqa_regmem256 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_256BIT, 0, PP_VMOVDQA, MMMMM_VMOVDQA, AVXOP_MOVDQA_FROMMEM, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vmovdqa_memreg256 ( void* DestPtr, long sseSrcReg )
{
	return x64EncodeRipOffsetV ( VEX_256BIT, 0, PP_VMOVDQA, MMMMM_VMOVDQA, AVXOP_MOVDQA_TOMEM, sseSrcReg, 0, (char*) DestPtr );
}

bool x64Encoder::vmovdqa_memreg256 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( VEX_256BIT, 0, PP_VMOVDQA, MMMMM_VMOVDQA, AVXOP_MOVDQA_TOMEM, sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vmovdqa_regmem256 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( VEX_256BIT, 0, PP_VMOVDQA, MMMMM_VMOVDQA, AVXOP_MOVDQA_TOMEM, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}





/*
bool x64Encoder::movdqa_to_mem128 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_MOVDQA_TOMEM, sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::movdqa_from_mem128 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_MOVDQA_FROMMEM, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::movdqa256 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_256BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_MOVDQA_FROMMEM, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::movdqa_to_mem256 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( VEX_256BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_MOVDQA_TOMEM, sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::movdqa_from_mem256 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( VEX_256BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_MOVDQA_FROMMEM, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}
*/

// movss
bool x64Encoder::movss_to_mem128 ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( VEX_128BIT, 0, VEX_PREFIXF3, VEX_PREFIX0F, AVXOP_MOVSS_TOMEM, sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::movss_from_mem128 ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( VEX_128BIT, 0, VEX_PREFIXF3, VEX_PREFIX0F, AVXOP_MOVSS_FROMMEM, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}





bool x64Encoder::vpabsb_regreg128 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, PP_VPABS, MMMMM_VPABS, AVXOP_PABSB, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vpabsw_regreg128 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, PP_VPABS, MMMMM_VPABS, AVXOP_PABSW, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vpabsd_regreg128 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, PP_VPABS, MMMMM_VPABS, AVXOP_PABSD, sseDestReg, 0, sseSrcReg );
}


bool x64Encoder::vpabsb_regmem128 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_128BIT, 0, PP_VPABS, MMMMM_VPABS, AVXOP_PABSB, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vpabsw_regmem128 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_128BIT, 0, PP_VPABS, MMMMM_VPABS, AVXOP_PABSW, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vpabsd_regmem128 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_128BIT, 0, PP_VPABS, MMMMM_VPABS, AVXOP_PABSD, sseDestReg, 0, (char*) SrcPtr );
}






bool x64Encoder::pabswregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x38, 0x1d ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pabswregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x0f, 0x38, 0x1d ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::pabsdregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x38, 0x1e ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pabsdregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x0f, 0x38, 0x1e ), sseDestReg, (char*) SrcPtr );
}

	
bool x64Encoder::paddb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PADDB, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::paddw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PADDW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::paddd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PADDD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}


bool x64Encoder::paddbregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xfc ), sseDestReg, sseSrcReg );
}

bool x64Encoder::paddbregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xfc ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::paddwregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xfd ), sseDestReg, sseSrcReg );
}

bool x64Encoder::paddwregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xfd ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::padddregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xfe ), sseDestReg, sseSrcReg );
}



bool x64Encoder::padddregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xfe ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::paddqregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xd4 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::paddqregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xd4 ), sseDestReg, (char*) SrcPtr );
}



bool x64Encoder::paddusb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PADDUSB, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::paddusw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PADDUSW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::paddsb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PADDSB, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::paddsw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PADDSW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::paddsbregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xec ), sseDestReg, sseSrcReg );
}

bool x64Encoder::paddsbregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xec ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::paddswregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xed ), sseDestReg, sseSrcReg );
}

bool x64Encoder::paddswregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xed ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::paddusbregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xdc ), sseDestReg, sseSrcReg );
}

bool x64Encoder::paddusbregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xdc ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::padduswregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xdd ), sseDestReg, sseSrcReg );
}

bool x64Encoder::padduswregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xdd ), sseDestReg, (char*) SrcPtr );
}




bool x64Encoder::pand ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PAND, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pandregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xdb ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pandregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xdb ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::pandn ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PANDN, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pandnregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xdf ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pandnregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xdf ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::pcmpeqb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PCMPEQB, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pcmpeqw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PCMPEQW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pcmpeqd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PCMPEQD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pcmpeqbregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x74 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pcmpeqbregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x74 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::pcmpeqwregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x75 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pcmpeqwregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x75 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::pcmpeqdregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x76 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pcmpeqdregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x76 ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::pcmpgtb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PCMPGTB, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pcmpgtw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PCMPGTW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pcmpgtd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PCMPGTD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pcmpgtbregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x64 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pcmpgtbregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x64 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::pcmpgtwregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x65 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pcmpgtwregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x65 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::pcmpgtdregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x66 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pcmpgtdregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x66 ), sseDestReg, (char*) SrcPtr );
}


	
bool x64Encoder::pmaxsw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PMAXSW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pmaxswregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xee ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmaxswregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xee ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::pmaxsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F38, AVXOP_PMAXSD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pmaxsdregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x38, 0x3d ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmaxsdregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x0f, 0x38, 0x3d ), sseDestReg, (char*) SrcPtr );
}
	
	// pmin - get minimum of signed integers
bool x64Encoder::pminsw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PMINSW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pminswregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xea ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pminswregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xea ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::pminsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F38, AVXOP_PMINSD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pminsdregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x38, 0x39 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pminsdregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x0f, 0x38, 0x39 ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::pmovsxdq ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F38, AVXOP_PMOVSXDQ, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::pmovsxdqregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x38, 0x25 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmovsxdqregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x0f, 0x38, 0x25 ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::por ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_POR, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::porregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xeb ), sseDestReg, sseSrcReg );
}

bool x64Encoder::porregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xeb ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::packusdw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F38, AVXOP_PACKUSDW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::packuswb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PACKUSWB, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::packuswbregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x67 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::packuswbregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x67 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::packusdwregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x38, 0x2b ), sseDestReg, sseSrcReg );
}

bool x64Encoder::packusdwregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x0f, 0x38, 0x2b ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::packsswbregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x63 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::packsswbregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x63 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::packssdwregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x6b ), sseDestReg, sseSrcReg );
}

bool x64Encoder::packssdwregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x6b ), sseDestReg, (char*) SrcPtr );
}







bool x64Encoder::vpmovzxbw_regreg128 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXBW, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vpmovzxbd_regreg128 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXBD, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vpmovzxbq_regreg128 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXBQ, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vpmovzxwd_regreg128 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXWD, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vpmovzxwq_regreg128 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXWQ, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vpmovzxdq_regreg128 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXDQ, sseDestReg, 0, sseSrcReg );
}


bool x64Encoder::vpmovzxbw_regmem128 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_128BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXBW, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vpmovzxbd_regmem128 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_128BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXBD, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vpmovzxbq_regmem128 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_128BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXBQ, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vpmovzxwd_regmem128 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_128BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXWD, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vpmovzxwq_regmem128 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_128BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXWQ, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vpmovzxdq_regmem128 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_128BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXDQ, sseDestReg, 0, (char*) SrcPtr );
}




bool x64Encoder::vpmovzxbw_regreg256 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_256BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXBW, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vpmovzxbd_regreg256 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_256BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXBD, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vpmovzxbq_regreg256 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_256BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXBQ, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vpmovzxwd_regreg256 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_256BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXWD, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vpmovzxwq_regreg256 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_256BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXWQ, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::vpmovzxdq_regreg256 ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( VEX_256BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXDQ, sseDestReg, 0, sseSrcReg );
}


bool x64Encoder::vpmovzxbw_regmem256 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_256BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXBW, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vpmovzxbd_regmem256 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_256BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXBD, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vpmovzxbq_regmem256 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_256BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXBQ, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vpmovzxwd_regmem256 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_256BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXWD, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vpmovzxwq_regmem256 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_256BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXWQ, sseDestReg, 0, (char*) SrcPtr );
}

bool x64Encoder::vpmovzxdq_regmem256 ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffsetV ( VEX_256BIT, 0, PP_VPMOVZX, MMMMM_VPMOVZX, AVXOP_PMOVZXDQ, sseDestReg, 0, (char*) SrcPtr );
}








bool x64Encoder::pmovzxbwregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x38, 0x30 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmovzxbwregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x0f, 0x38, 0x30 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::pmovzxbdregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x38, 0x31 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmovzxbdregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x0f, 0x38, 0x31 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::pmovzxbqregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x38, 0x32 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmovzxbqregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x0f, 0x38, 0x32 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::pmovzxwdregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x38, 0x33 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmovzxwdregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x0f, 0x38, 0x33 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::pmovzxwqregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x38, 0x34 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmovzxwqregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x0f, 0x38, 0x34 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::pmovzxdqregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x38, 0x35 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmovzxdqregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x0f, 0x38, 0x35 ), sseDestReg, (char*) SrcPtr );
}






bool x64Encoder::pmuludq ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, PP_PMULUDQ, MMMMM_PMULUDQ, AVXOP_PMULUDQ, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pshufb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F38, AVXOP_PSHUFB, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pshufd ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSHUFD, sseDestReg, 0, sseSrcReg, Imm8 );
}

bool x64Encoder::pshufbregreg ( long sseDestReg, long sseSrcReg )
{
	// note: fourth byte in opcode is actually zero, but I didn't expect this when I started the project
	return x64EncodeRegReg32 ( MAKE4OPCODE( 0x66, 0x0f, 0x38, 0xff ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pshufbregmem ( long sseDestReg, void* SrcPtr )
{
	// note: fourth byte in opcode is actually zero, but I didn't expect this when I started the project
	return x64EncodeRipOffset32 ( MAKE4OPCODE( 0x66, 0x0f, 0x38, 0xff ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::pshufdregregimm ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	x64EncodePrefix ( 0x66 );
	x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x70 ), sseDestReg, sseSrcReg );
	return x64EncodeImmediate8 ( Imm8 );
}

bool x64Encoder::pshufdregmemimm ( long sseDestReg, void* SrcPtr, char Imm8 )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32Imm8 ( MAKE2OPCODE( 0x0f, 0x70 ), sseDestReg, (char*) SrcPtr, Imm8 );
	//return x64EncodeImmediate8 ( Imm8 );
}




bool x64Encoder::pshufhw ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( VEX_128BIT, 0, VEX_PREFIXF3, VEX_PREFIX0F, AVXOP_PSHUFHW, sseDestReg, 0, sseSrcReg, Imm8 );
}


bool x64Encoder::pshufhwregregimm ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	x64EncodePrefix ( 0xf3 );
	x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x70 ), sseDestReg, sseSrcReg );
	return x64EncodeImmediate8 ( Imm8 );
}

bool x64Encoder::pshufhwregmemimm ( long sseDestReg, void* SrcPtr, char Imm8 )
{
	x64EncodePrefix ( 0xf3 );
	return x64EncodeRipOffset32Imm8 ( MAKE2OPCODE( 0x0f, 0x70 ), sseDestReg, (char*) SrcPtr, Imm8 );
	//return x64EncodeImmediate8 ( Imm8 );
}


bool x64Encoder::pshuflw ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( VEX_128BIT, 0, VEX_PREFIXF2, VEX_PREFIX0F, AVXOP_PSHUFLW, sseDestReg, 0, sseSrcReg, Imm8 );
}

bool x64Encoder::pshuflwregregimm ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	x64EncodePrefix ( 0xf2 );
	x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x70 ), sseDestReg, sseSrcReg );
	return x64EncodeImmediate8 ( Imm8 );
}

bool x64Encoder::pshuflwregmemimm ( long sseDestReg, void* SrcPtr, char Imm8 )
{
	x64EncodePrefix ( 0xf2 );
	return x64EncodeRipOffset32Imm8 ( MAKE2OPCODE( 0x0f, 0x70 ), sseDestReg, (char*) SrcPtr, Imm8 );
	//return x64EncodeImmediate8 ( Imm8 );
}

bool x64Encoder::pslldq ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSLLDQ, MODRM_PSLLDQ, sseDestReg, sseSrcReg, Imm8 );
}

bool x64Encoder::psllw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSLLW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::psllw_imm ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSLLW_IMM, MODRM_PSLLW_IMM, sseDestReg, sseSrcReg, Imm8 );
}

bool x64Encoder::pslld ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSLLD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pslld_imm ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSLLD_IMM, MODRM_PSLLD_IMM, sseDestReg, sseSrcReg, Imm8 );
}

bool x64Encoder::psllwregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf1 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psllwregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf1 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::psllwregimm ( long sseDestReg, char Imm8 )
{
	//return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf1 ), sseDestReg, sseSrcReg );
	return x64EncodeReg32Imm8 ( MAKE3OPCODE( 0x66, 0x0f, 0x71 ), 0x6, sseDestReg, Imm8 );
}

bool x64Encoder::pslldregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf2 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pslldregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf2 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::pslldregimm ( long sseDestReg, char Imm8 )
{
	//return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf1 ), sseDestReg, sseSrcReg );
	return x64EncodeReg32Imm8 ( MAKE3OPCODE( 0x66, 0x0f, 0x72 ), 0x6, sseDestReg, Imm8 );
}

bool x64Encoder::psllqregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xf3 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psllqregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xf3 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::psllqregimm ( long sseDestReg, char Imm8 )
{
	//return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf1 ), sseDestReg, sseSrcReg );
	x64EncodePrefix ( 0x66 );
	return x64EncodeReg32Imm8 ( MAKE2OPCODE( 0x0f, 0x73 ), 0x6, sseDestReg, Imm8 );
}




	
bool x64Encoder::psraw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSRAW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::psraw_imm ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSRAW_IMM, MODRM_PSRAW_IMM, sseDestReg, sseSrcReg, Imm8 );
}

bool x64Encoder::psrad ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSRAD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::psrad_imm ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSRAD_IMM, MODRM_PSRAD_IMM, sseDestReg, sseSrcReg, Imm8 );
}


bool x64Encoder::psrawregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xe1 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psrawregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xe1 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::psrawregimm ( long sseDestReg, char Imm8 )
{
	//return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf1 ), sseDestReg, sseSrcReg );
	return x64EncodeReg32Imm8 ( MAKE3OPCODE( 0x66, 0x0f, 0x71 ), 0x4, sseDestReg, Imm8 );
}

bool x64Encoder::psradregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xe2 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psradregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xe2 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::psradregimm ( long sseDestReg, char Imm8 )
{
	//return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf1 ), sseDestReg, sseSrcReg );
	return x64EncodeReg32Imm8 ( MAKE3OPCODE( 0x66, 0x0f, 0x72 ), 0x4, sseDestReg, Imm8 );
}




	
bool x64Encoder::psrldq ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSRLDQ_IMM, MODRM_PSRLDQ_IMM, sseDestReg, sseSrcReg, Imm8 );
}

bool x64Encoder::psrlw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSRLW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::psrlw_imm ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSRLW_IMM, MODRM_PSRLW_IMM, sseDestReg, sseSrcReg, Imm8 );
}

bool x64Encoder::psrld ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSRLD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::psrld_imm ( long sseDestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSRLD_IMM, MODRM_PSRLD_IMM, sseDestReg, sseSrcReg, Imm8 );
}


bool x64Encoder::psrlwregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xd1 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psrlwregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xd1 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::psrlwregimm ( long sseDestReg, char Imm8 )
{
	//return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf1 ), sseDestReg, sseSrcReg );
	return x64EncodeReg32Imm8 ( MAKE3OPCODE( 0x66, 0x0f, 0x71 ), 0x2, sseDestReg, Imm8 );
}

bool x64Encoder::psrldregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xd2 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psrldregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xd2 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::psrldregimm ( long sseDestReg, char Imm8 )
{
	//return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf1 ), sseDestReg, sseSrcReg );
	return x64EncodeReg32Imm8 ( MAKE3OPCODE( 0x66, 0x0f, 0x72 ), 0x2, sseDestReg, Imm8 );
}

bool x64Encoder::psrlqregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xd3 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psrlqregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xd3 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::psrlqregimm ( long sseDestReg, char Imm8 )
{
	//return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf1 ), sseDestReg, sseSrcReg );
	return x64EncodeReg32Imm8 ( MAKE3OPCODE( 0x66, 0x0f, 0x73 ), 0x2, sseDestReg, Imm8 );
}



	
bool x64Encoder::psubb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSUBB, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::psubw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSUBW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::psubd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSUBD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}


bool x64Encoder::psubbregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf8 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psubbregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf8 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::psubwregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf9 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psubwregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf9 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::psubdregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xfa ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psubdregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xfa ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::psubqregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xfb ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psubqregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xfb ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::psubusb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSUBUSB, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::psubusw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSUBUSW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::psubsb ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSUBSB, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::psubsw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PSUBSW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::psubsbregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xe8 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psubsbregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xe8 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::psubswregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xe9 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psubswregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xe9 ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::psubusbregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xd8 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psubusbregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xd8 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::psubuswregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xd9 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::psubuswregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xd9 ), sseDestReg, (char*) SrcPtr );
}




bool x64Encoder::punpckhbw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PUNPCKHBW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::punpckhwd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PUNPCKHWD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::punpckhdq ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PUNPCKHDQ, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::punpckhqdq ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PUNPCKHQDQ, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}


bool x64Encoder::punpckhbwregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0x68 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::punpckhbwregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0x68 ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::punpckhwdregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0x69 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::punpckhwdregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0x69 ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::punpckhdqregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0x6a ), sseDestReg, sseSrcReg );
}

bool x64Encoder::punpckhdqregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0x6a ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::punpckhqdqregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0x6d ), sseDestReg, sseSrcReg );
}

bool x64Encoder::punpckhqdqregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0x6d ), sseDestReg, (char*) SrcPtr );
}






bool x64Encoder::punpcklbw ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PUNPCKLBW, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::punpcklwd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PUNPCKLWD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::punpckldq ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PUNPCKLDQ, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::punpcklqdq ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PUNPCKLQDQ, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}


bool x64Encoder::punpcklbwregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0x60 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::punpcklbwregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0x60 ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::punpcklwdregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0x61 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::punpcklwdregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0x61 ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::punpckldqregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0x62 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::punpckldqregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0x62 ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::punpcklqdqregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0x6c ), sseDestReg, sseSrcReg );
}

bool x64Encoder::punpcklqdqregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0x6c ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::pxor ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( VEX_128BIT, 0, VEX_PREFIX66, VEX_PREFIX0F, AVXOP_PXOR, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::pxorregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0xef ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pxorregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0xef ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::ptestregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x0f, 0x38, 0x17 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::ptestregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x0f, 0x38, 0x17 ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::pmuldqregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE4OPCODE( 0x66, 0x0f, 0x38, 0x28 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmuldqregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE4OPCODE( 0x66, 0x0f, 0x38, 0x28 ), sseDestReg, (char*) SrcPtr );
}



bool x64Encoder::pmuludqregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf4 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmuludqregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf4 ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::pmulldregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE4OPCODE( 0x66, 0x0f, 0x38, 0x40 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmulldregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE4OPCODE( 0x66, 0x0f, 0x38, 0x40 ), sseDestReg, (char*) SrcPtr );
}



bool x64Encoder::pmullwregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xd5 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmullwregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xd5 ), sseDestReg, (char*) SrcPtr );
}



bool x64Encoder::pmulhwregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xe5 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmulhwregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xe5 ), sseDestReg, (char*) SrcPtr );
}



bool x64Encoder::pmaddwdregreg ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf5 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::pmaddwdregmem ( long sseDestReg, void* SrcPtr )
{
	return x64EncodeRipOffset32 ( MAKE3OPCODE( 0x66, 0x0f, 0xf5 ), sseDestReg, (char*) SrcPtr );
}




// sse floating point instructions

// *** SSE instructions *** //

bool x64Encoder::movd_to_sse ( long sseDestReg, long x64SrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, X64OP1_MOVD_FROMMEM, X64OP2_MOVD_FROMMEM ), sseDestReg, x64SrcReg );
}

bool x64Encoder::movd_from_sse ( long x64DestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0x66, X64OP1_MOVD_TOMEM, X64OP2_MOVD_TOMEM ), x64DestReg, sseSrcReg );
}

bool x64Encoder::movq_to_sse ( long sseDestReg, long x64SrcReg )
{
	x64CodeArea [ x64NextOffset++ ] = 0x66;
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_MOVQ_FROMMEM, X64OP2_MOVQ_FROMMEM ), sseDestReg, x64SrcReg );
}

bool x64Encoder::movq_from_sse ( long x64DestReg, long sseSrcReg )
{
	x64CodeArea [ x64NextOffset++ ] = 0x66;
	return x64EncodeRegReg64 ( MAKE2OPCODE( X64OP1_MOVQ_TOMEM, X64OP2_MOVQ_TOMEM ), x64DestReg, sseSrcReg );
}


bool x64Encoder::movd_regmem ( long sseDestReg, long* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_MOVD_FROMMEM, X64OP2_MOVD_FROMMEM ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::movd_memreg ( long* DstPtr, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_MOVD_TOMEM, X64OP2_MOVD_TOMEM ), sseSrcReg, (char*) DstPtr );
}

bool x64Encoder::movq_regmem ( long sseDestReg, long* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_MOVQ_FROMMEM, X64OP2_MOVQ_FROMMEM ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::movq_memreg ( long* DstPtr, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset64 ( MAKE2OPCODE( X64OP1_MOVQ_TOMEM, X64OP2_MOVQ_TOMEM ), sseSrcReg, (char*) DstPtr );
}

/*
bool x64Encoder::movd_regmem ( long sseDestReg, long* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x6e ), sseDestReg, (char*) SrcPtr );
}

bool x64Encoder::movq_regmem ( long sseDestReg, long long* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset64 ( MAKE2OPCODE( 0x0f, 0x6e ), sseDestReg, (char*) SrcPtr );
}
*/


// *** SSE floating point instructions *** //

bool x64Encoder::addsd ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0xf2, X64OP1_ADDSD, X64OP2_ADDSD ), sseDestReg, sseSrcReg );
}

bool x64Encoder::subsd ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0xf2, X64OP1_SUBSD, X64OP2_SUBSD ), sseDestReg, sseSrcReg );
}

bool x64Encoder::mulsd ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0xf2, X64OP1_MULSD, X64OP2_MULSD ), sseDestReg, sseSrcReg );
}

bool x64Encoder::divsd ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0xf2, X64OP1_DIVSD, X64OP2_DIVSD ), sseDestReg, sseSrcReg );
}

bool x64Encoder::sqrtsd ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0xf2, X64OP1_SQRTSD, X64OP2_SQRTSD ), sseDestReg, sseSrcReg );
}


bool x64Encoder::addpdregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x58 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::addpdregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x58 ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::mulpdregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, 0x59 ), sseDestReg, sseSrcReg );
}

bool x64Encoder::mulpdregmem ( long sseDestReg, void* SrcPtr )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( 0x0f, 0x59 ), sseDestReg, (char*) SrcPtr );
}


bool x64Encoder::cvttss2si ( long x64DestReg, long sseSrcReg )
{
	return x64EncodeRegReg32 ( MAKE3OPCODE( 0xf3, X64OP1_CVTTSS2SI, X64OP2_CVTTSS2SI ), x64DestReg, sseSrcReg );
}

bool x64Encoder::cvttps2dq_regreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0xf3 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CVTTPS2DQ, X64OP2_CVTTPS2DQ ), sseDestReg, sseSrcReg );
}


bool x64Encoder::cvtsi2sd ( long sseDestReg, long x64SrcReg )
{
	x64EncodePrefix ( 0xf2 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CVTSI2SD, X64OP2_CVTSI2SD ), sseDestReg, x64SrcReg );
}

bool x64Encoder::cvtdq2pd_regreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0xf3 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_CVTDQ2PD, X64OP2_CVTDQ2PD ), sseDestReg, sseSrcReg );
}


bool x64Encoder::movddup_regreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0xf2 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_MOVDDUP, X64OP2_MOVDDUP ), sseDestReg, sseSrcReg );
}

bool x64Encoder::movddup_regmem ( long sseDestReg, long long* SrcPtr )
{
	x64EncodePrefix ( 0xf2 );
	return x64EncodeRipOffset32 ( MAKE2OPCODE( X64OP1_MOVDDUP, X64OP2_MOVDDUP ), sseDestReg, (char*) SrcPtr );
}




// *** AVX instructions ***//



bool x64Encoder::blendvps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, long sseSrc3Reg )
{
	return x64EncodeRegVImm8 ( 0, 0, PP_BLENDPS, MMMMM_BLENDPS, AVXOP_BLENDPS, sseDestReg, sseSrc1Reg, sseSrc2Reg,  ( sseSrc3Reg << 4 ) );
}

bool x64Encoder::blendvpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, long sseSrc3Reg )
{
	return x64EncodeRegVImm8 ( 1, 0, PP_BLENDPD, MMMMM_BLENDPD, AVXOP_BLENDPD, sseDestReg, sseSrc1Reg, sseSrc2Reg,  ( sseSrc3Reg << 4 ) );
}

bool x64Encoder::blendps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, 0, PP_BLENDPS, MMMMM_BLENDPS, AVXOP_BLENDPS, sseDestReg, sseSrc1Reg, sseSrc2Reg, Imm8 );
}

bool x64Encoder::blendpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 1, 0, PP_BLENDPD, MMMMM_BLENDPD, AVXOP_BLENDPD, sseDestReg, sseSrc1Reg, sseSrc2Reg, Imm8 );
}

bool x64Encoder::blendvps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, long sseSrc3Reg )
{
	return x64EncodeRegMemVImm8 ( 0, 0, PP_BLENDPS, MMMMM_BLENDPS, AVXOP_BLENDPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, ( sseSrc3Reg << 4 ) );
}

bool x64Encoder::blendvpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, long sseSrc3Reg )
{
	return x64EncodeRegMemVImm8 ( 1, 0, PP_BLENDPD, MMMMM_BLENDPD, AVXOP_BLENDPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, ( sseSrc3Reg << 4 ) );
}

bool x64Encoder::blendps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 )
{
	return x64EncodeRegMemVImm8 ( 0, 0, PP_BLENDPS, MMMMM_BLENDPS, AVXOP_BLENDPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, Imm8 );
}

bool x64Encoder::blendpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 )
{
	return x64EncodeRegMemVImm8 ( 1, 0, PP_BLENDPD, MMMMM_BLENDPD, AVXOP_BLENDPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, Imm8 );
}



bool x64Encoder::vaddps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_ADDPS, MMMMM_ADDPS, AVXOP_ADDPS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vaddpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_ADDPD, MMMMM_ADDPD, AVXOP_ADDPD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vaddss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_ADDSS, MMMMM_ADDSS, AVXOP_ADDSS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vaddsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_ADDSD, MMMMM_ADDSD, AVXOP_ADDSD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vaddps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_ADDPS, MMMMM_ADDPS, AVXOP_ADDPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vaddpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_ADDPD, MMMMM_ADDPD, AVXOP_ADDPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vaddss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_ADDSS, MMMMM_ADDSS, AVXOP_ADDSS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vaddsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_ADDSD, MMMMM_ADDSD, AVXOP_ADDSD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::andps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_ANDPS, MMMMM_ANDPS, AVXOP_ANDPS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::andpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_ANDPD, MMMMM_ANDPD, AVXOP_ANDPD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::andps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_ANDPS, MMMMM_ANDPS, AVXOP_ANDPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::andpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_ANDPD, MMMMM_ANDPD, AVXOP_ANDPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::andnps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_ANDNPS, MMMMM_ANDNPS, AVXOP_ANDNPS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::andnpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_ANDNPD, MMMMM_ANDNPD, AVXOP_ANDNPD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::andnps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_ANDNPS, MMMMM_ANDNPS, AVXOP_ANDNPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::andnpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_ANDNPD, MMMMM_ANDNPD, AVXOP_ANDNPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}






bool x64Encoder::cvtdq2pd ( long sseDestReg, long sseSrc1Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_CVTDQ2PD, MMMMM_CVTDQ2PD, AVXOP_CVTDQ2PD, sseDestReg, 0, sseSrc1Reg );
}

bool x64Encoder::cvtdq2ps ( long sseDestReg, long sseSrc1Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_CVTDQ2PS, MMMMM_CVTDQ2PS, AVXOP_CVTDQ2PS, sseDestReg, 0, sseSrc1Reg );
}

bool x64Encoder::cvtpd2dq ( long sseDestReg, long sseSrc1Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_CVTPD2DQ, MMMMM_CVTPD2DQ, AVXOP_CVTPD2DQ, sseDestReg, 0, sseSrc1Reg );
}

bool x64Encoder::cvtps2dq ( long sseDestReg, long sseSrc1Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_CVTPS2DQ, MMMMM_CVTPS2DQ, AVXOP_CVTPS2DQ, sseDestReg, 0, sseSrc1Reg );
}

bool x64Encoder::cvtps2pd ( long sseDestReg, long sseSrc1Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_CVTPS2PD, MMMMM_CVTPS2PD, AVXOP_CVTPS2PD, sseDestReg, 0, sseSrc1Reg );
}

bool x64Encoder::cvtpd2ps ( long sseDestReg, long sseSrc1Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_CVTPD2PS, MMMMM_CVTPD2PS, AVXOP_CVTPD2PS, sseDestReg, 0, sseSrc1Reg );
}


bool x64Encoder::cvtdq2pd ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_CVTDQ2PD, MMMMM_CVTDQ2PD, AVXOP_CVTDQ2PD, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::cvtdq2ps ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_CVTDQ2PS, MMMMM_CVTDQ2PS, AVXOP_CVTDQ2PS, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::cvtpd2dq ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_CVTPD2DQ, MMMMM_CVTPD2DQ, AVXOP_CVTPD2DQ, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::cvtps2dq ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_CVTPS2DQ, MMMMM_CVTPS2DQ, AVXOP_CVTPS2DQ, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::cvtps2pd ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_CVTPS2PD, MMMMM_CVTPS2PD, AVXOP_CVTPS2PD, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::cvtpd2ps ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_CVTPD2PS, MMMMM_CVTPD2PS, AVXOP_CVTPD2PS, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}



bool x64Encoder::vdivps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_DIVPS, MMMMM_DIVPS, AVXOP_DIVPS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vdivpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_DIVPD, MMMMM_DIVPD, AVXOP_DIVPD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vdivss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_DIVSS, MMMMM_DIVSS, AVXOP_DIVSS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vdivsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_DIVSD, MMMMM_DIVSD, AVXOP_DIVSD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vdivps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_DIVPS, MMMMM_DIVPS, AVXOP_DIVPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vdivpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_DIVPD, MMMMM_DIVPD, AVXOP_DIVPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vdivss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_DIVSS, MMMMM_DIVSS, AVXOP_DIVSS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vdivsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_DIVSD, MMMMM_DIVSD, AVXOP_DIVSD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::maxps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_MAXPS, MMMMM_MAXPS, AVXOP_MAXPS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::maxpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_MAXPD, MMMMM_MAXPD, AVXOP_MAXPD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::maxss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_MAXSS, MMMMM_MAXSS, AVXOP_MAXSS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::maxsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_MAXSD, MMMMM_MAXSD, AVXOP_MAXSD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::maxps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_MAXPS, MMMMM_MAXPS, AVXOP_MAXPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::maxpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_MAXPD, MMMMM_MAXPD, AVXOP_MAXPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::maxss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_MAXSS, MMMMM_MAXSS, AVXOP_MAXSS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::maxsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_MAXSD, MMMMM_MAXSD, AVXOP_MAXSD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::minps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_MINPS, MMMMM_MINPS, AVXOP_MINPS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::minpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_MINPD, MMMMM_MINPD, AVXOP_MINPD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::minss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_MINSS, MMMMM_MINSS, AVXOP_MINSS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::minsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_MINSD, MMMMM_MINSD, AVXOP_MINSD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::minps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_MINPS, MMMMM_MINPS, AVXOP_MINPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::minpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_MINPD, MMMMM_MINPD, AVXOP_MINPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::minss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_MINSS, MMMMM_MINSS, AVXOP_MINSS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::minsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_MINSD, MMMMM_MINSD, AVXOP_MINSD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vmulps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_MULPS, MMMMM_MULPS, AVXOP_MULPS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vmulpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_MULPD, MMMMM_MULPD, AVXOP_MULPD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vmulss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_MULSS, MMMMM_MULSS, AVXOP_MULSS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vmulsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_MULSD, MMMMM_MULSD, AVXOP_MULSD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vmulps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_MULPS, MMMMM_MULPS, AVXOP_MULPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vmulpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_MULPD, MMMMM_MULPD, AVXOP_MULPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vmulss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_MULSS, MMMMM_MULSS, AVXOP_MULSS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vmulsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_MULSD, MMMMM_MULSD, AVXOP_MULSD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}



bool x64Encoder::orps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_ORPS, MMMMM_ORPS, AVXOP_ORPS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::orpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_ORPD, MMMMM_ORPD, AVXOP_ORPD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::orps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_ORPS, MMMMM_ORPS, AVXOP_ORPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::orpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_ORPD, MMMMM_ORPD, AVXOP_ORPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}


bool x64Encoder::shufps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, 0, PP_SHUFPS, MMMMM_SHUFPS, AVXOP_SHUFPS, sseDestReg, sseSrc1Reg, sseSrc2Reg, Imm8 );
}

bool x64Encoder::shufpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 1, 0, PP_SHUFPD, MMMMM_SHUFPD, AVXOP_SHUFPD, sseDestReg, sseSrc1Reg, sseSrc2Reg, Imm8 );
}

bool x64Encoder::shufps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 )
{
	return x64EncodeRegMemVImm8 ( 0, 0, PP_SHUFPS, MMMMM_SHUFPS, AVXOP_SHUFPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, Imm8 );
}

bool x64Encoder::shufpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 )
{
	return x64EncodeRegMemVImm8 ( 1, 0, PP_SHUFPD, MMMMM_SHUFPD, AVXOP_SHUFPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, Imm8 );
}




bool x64Encoder::vsqrtps ( long sseDestReg, long sseSrc1Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_SQRTPS, MMMMM_SQRTPS, AVXOP_SQRTPS, sseDestReg, 0, sseSrc1Reg );
}

bool x64Encoder::vsqrtpd ( long sseDestReg, long sseSrc1Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_SQRTPD, MMMMM_SQRTPD, AVXOP_SQRTPD, sseDestReg, 0, sseSrc1Reg );
}

bool x64Encoder::vsqrtss ( long sseDestReg, long sseSrc1Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_SQRTSS, MMMMM_SQRTSS, AVXOP_SQRTSS, sseDestReg, 0, sseSrc1Reg );
}

bool x64Encoder::vsqrtsd ( long sseDestReg, long sseSrc1Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_SQRTSD, MMMMM_SQRTSD, AVXOP_SQRTSD, sseDestReg, 0, sseSrc1Reg );
}


bool x64Encoder::vsqrtps ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_SQRTPS, MMMMM_SQRTPS, AVXOP_SQRTPS, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vsqrtpd ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_SQRTPD, MMMMM_SQRTPD, AVXOP_SQRTPD, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vsqrtss ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_SQRTSS, MMMMM_SQRTSS, AVXOP_SQRTSS, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vsqrtsd ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_SQRTSD, MMMMM_SQRTSD, AVXOP_SQRTSD, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}


bool x64Encoder::subpsregreg ( long sseDestReg, long sseSrcReg )
{
	//x64EncodePrefix ( 0xf2 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_SUBPS, X64OP2_SUBPS ), sseDestReg, sseSrcReg );
}

bool x64Encoder::subpdregreg ( long sseDestReg, long sseSrcReg )
{
	x64EncodePrefix ( 0x66 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( X64OP1_SUBPD, X64OP2_SUBPD ), sseDestReg, sseSrcReg );
}


bool x64Encoder::vsubps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_SUBPS, MMMMM_SUBPS, AVXOP_SUBPS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vsubpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_SUBPD, MMMMM_SUBPD, AVXOP_SUBPD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vsubss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_SUBSS, MMMMM_SUBSS, AVXOP_SUBSS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vsubsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_SUBSD, MMMMM_SUBSD, AVXOP_SUBSD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::vsubps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_SUBPS, MMMMM_SUBPS, AVXOP_SUBPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vsubpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_SUBPD, MMMMM_SUBPD, AVXOP_SUBPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vsubss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_SUBSS, MMMMM_SUBSS, AVXOP_SUBSS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vsubsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_SUBSD, MMMMM_SUBSD, AVXOP_SUBSD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}












bool x64Encoder::xorps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 0, 0, PP_XORPS, MMMMM_XORPS, AVXOP_XORPS, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::xorpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return x64EncodeRegRegV ( 1, 0, PP_XORPD, MMMMM_XORPD, AVXOP_XORPD, sseDestReg, sseSrc1Reg, sseSrc2Reg );
}

bool x64Encoder::xorps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_XORPS, MMMMM_XORPS, AVXOP_XORPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::xorpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_XORPD, MMMMM_XORPD, AVXOP_XORPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset );
}











bool x64Encoder::cmpps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, 0, PP_CMPPS, MMMMM_CMPPS, AVXOP_CMPPS, sseDestReg, sseSrc1Reg, sseSrc2Reg, Imm8 );
}

bool x64Encoder::cmppd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 1, 0, PP_CMPPD, MMMMM_CMPPD, AVXOP_CMPPD, sseDestReg, sseSrc1Reg, sseSrc2Reg, Imm8 );
}

bool x64Encoder::cmpss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, 0, PP_CMPSS, MMMMM_CMPSS, AVXOP_CMPSS, sseDestReg, sseSrc1Reg, sseSrc2Reg, Imm8 );
}

bool x64Encoder::cmpsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 1, 0, PP_CMPSD, MMMMM_CMPSD, AVXOP_CMPSD, sseDestReg, sseSrc1Reg, sseSrc2Reg, Imm8 );
}




bool x64Encoder::cmpeqps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpps ( sseDestReg, sseSrc1Reg, sseSrc2Reg, EQ_OQ );
}

bool x64Encoder::cmpltps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpps ( sseDestReg, sseSrc1Reg, sseSrc2Reg, LT_OQ );
}

bool x64Encoder::cmpgtps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpps ( sseDestReg, sseSrc1Reg, sseSrc2Reg, GT_OQ );
}

bool x64Encoder::cmpleps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpps ( sseDestReg, sseSrc1Reg, sseSrc2Reg, LE_OQ );
}

bool x64Encoder::cmpgeps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpps ( sseDestReg, sseSrc1Reg, sseSrc2Reg, GE_OQ );
}

bool x64Encoder::cmpunordps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpps ( sseDestReg, sseSrc1Reg, sseSrc2Reg, UNORD_Q );
}

bool x64Encoder::cmpordps ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpps ( sseDestReg, sseSrc1Reg, sseSrc2Reg, ORD_Q );
}

bool x64Encoder::cmpeqpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmppd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, EQ_OQ );
}

bool x64Encoder::cmpltpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmppd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, LT_OQ );
}

bool x64Encoder::cmpgtpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmppd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, GT_OQ );
}

bool x64Encoder::cmplepd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmppd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, LE_OQ );
}

bool x64Encoder::cmpgepd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmppd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, GE_OQ );
}

bool x64Encoder::cmpunordpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmppd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, UNORD_Q );
}

bool x64Encoder::cmpordpd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmppd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, ORD_Q );
}


bool x64Encoder::cmpeqss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpss ( sseDestReg, sseSrc1Reg, sseSrc2Reg, EQ_OQ );
}

bool x64Encoder::cmpltss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpss ( sseDestReg, sseSrc1Reg, sseSrc2Reg, LT_OQ );
}

bool x64Encoder::cmpgtss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpss ( sseDestReg, sseSrc1Reg, sseSrc2Reg, GT_OQ );
}

bool x64Encoder::cmpless ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpss ( sseDestReg, sseSrc1Reg, sseSrc2Reg, LE_OQ );
}

bool x64Encoder::cmpgess ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpss ( sseDestReg, sseSrc1Reg, sseSrc2Reg, GE_OQ );
}

bool x64Encoder::cmpunordss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpss ( sseDestReg, sseSrc1Reg, sseSrc2Reg, UNORD_Q );
}

bool x64Encoder::cmpordss ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpss ( sseDestReg, sseSrc1Reg, sseSrc2Reg, ORD_Q );
}


bool x64Encoder::cmpeqsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, EQ_OQ );
}

bool x64Encoder::cmpltsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, LT_OQ );
}

bool x64Encoder::cmpgtsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, GT_OQ );
}

bool x64Encoder::cmplesd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, LE_OQ );
}

bool x64Encoder::cmpgesd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, GE_OQ );
}

bool x64Encoder::cmpunordsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, UNORD_Q );
}

bool x64Encoder::cmpordsd ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, sseSrc2Reg, ORD_Q );
}








bool x64Encoder::cmpps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 )
{
	return x64EncodeRegMemVImm8 ( 0, 0, PP_CMPPS, MMMMM_CMPPS, AVXOP_CMPPS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, Imm8 );
}

bool x64Encoder::cmppd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 )
{
	return x64EncodeRegMemVImm8 ( 1, 0, PP_CMPPD, MMMMM_CMPPD, AVXOP_CMPPD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, Imm8 );
}

bool x64Encoder::cmpss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 )
{
	return x64EncodeRegMemVImm8 ( 0, 0, PP_CMPSS, MMMMM_CMPSS, AVXOP_CMPSS, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, Imm8 );
}

bool x64Encoder::cmpsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, char Imm8 )
{
	return x64EncodeRegMemVImm8 ( 1, 0, PP_CMPSD, MMMMM_CMPSD, AVXOP_CMPSD, sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, Imm8 );
}



bool x64Encoder::cmpeqps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpps ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, EQ_OQ );
}

bool x64Encoder::cmpltps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpps ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, LT_OQ );
}

bool x64Encoder::cmpgtps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpps ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, GT_OQ );
}

bool x64Encoder::cmpleps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpps ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, LE_OQ );
}

bool x64Encoder::cmpgeps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpps ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, GE_OQ );
}

bool x64Encoder::cmpunordps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpps ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, UNORD_Q );
}

bool x64Encoder::cmpordps ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpps ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, ORD_Q );
}


bool x64Encoder::cmpeqpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmppd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, EQ_OQ );
}

bool x64Encoder::cmpltpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmppd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, LT_OQ );
}

bool x64Encoder::cmpgtpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmppd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, GT_OQ );
}

bool x64Encoder::cmplepd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmppd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, LE_OQ );
}

bool x64Encoder::cmpgepd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmppd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, GE_OQ );
}

bool x64Encoder::cmpunordpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmppd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, UNORD_Q );
}

bool x64Encoder::cmpordpd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmppd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, ORD_Q );
}


bool x64Encoder::cmpeqss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpss ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, EQ_OQ );
}

bool x64Encoder::cmpltss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpss ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, LT_OQ );
}

bool x64Encoder::cmpgtss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpss ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, GT_OQ );
}

bool x64Encoder::cmpless ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpss ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, LE_OQ );
}

bool x64Encoder::cmpgess ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpss ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, GE_OQ );
}

bool x64Encoder::cmpunordss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpss ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, UNORD_Q );
}

bool x64Encoder::cmpordss ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpss ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, ORD_Q );
}


bool x64Encoder::cmpeqsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, EQ_OQ );
}

bool x64Encoder::cmpltsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, LT_OQ );
}

bool x64Encoder::cmpgtsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, GT_OQ );
}

bool x64Encoder::cmplesd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, LE_OQ );
}

bool x64Encoder::cmpgesd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, GE_OQ );
}

bool x64Encoder::cmpunordsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, UNORD_Q );
}

bool x64Encoder::cmpordsd ( long sseDestReg, long sseSrc1Reg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return cmpsd ( sseDestReg, sseSrc1Reg, x64BaseReg, x64IndexReg, Scale, Offset, ORD_Q );
}





// movaps
bool x64Encoder::movaps ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( 0, 0, PP_MOVAPS_FROMMEM, MMMMM_MOVAPS_FROMMEM, AVXOP_MOVAPS_FROMMEM, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::movapd ( long sseDestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( 1, 0, PP_MOVAPD_FROMMEM, MMMMM_MOVAPD_FROMMEM, AVXOP_MOVAPD_FROMMEM, sseDestReg, 0, sseSrcReg );
}

bool x64Encoder::movapstomem ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_MOVAPS_TOMEM, MMMMM_MOVAPS_TOMEM, AVXOP_MOVAPS_TOMEM, sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::movapsfrommem ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_MOVAPS_FROMMEM, MMMMM_MOVAPS_FROMMEM, AVXOP_MOVAPS_FROMMEM, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::movapdtomem ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_MOVAPD_TOMEM, MMMMM_MOVAPD_TOMEM, AVXOP_MOVAPD_TOMEM, sseSrcReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::movapdfrommem ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_MOVAPD_FROMMEM, MMMMM_MOVAPD_FROMMEM, AVXOP_MOVAPD_FROMMEM, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

// vbroadcastss
bool x64Encoder::vbroadcastss ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 0, 0, PP_VBROADCASTSS, MMMMM_VBROADCASTSS, AVXOP_VBROADCASTSS, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vbroadcastsd ( long sseDestReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset )
{
	return x64EncodeRegMemV ( 1, 0, PP_VBROADCASTSD, MMMMM_VBROADCASTSD, AVXOP_VBROADCASTSD, sseDestReg, 0, x64BaseReg, x64IndexReg, Scale, Offset );
}


bool x64Encoder::vmaskmovpstomem ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, long sseMaskReg )
{
	return x64EncodeRegMemV ( 0, 0, PP_VMASKMOVPS_TOMEM, MMMMM_VMASKMOVPS_TOMEM, AVXOP_VMASKMOVPS_TOMEM, sseSrcReg, sseMaskReg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::vmaskmovpdtomem ( long sseSrcReg, long x64BaseReg, long x64IndexReg, long Scale, long Offset, long sseMaskReg )
{
	return x64EncodeRegMemV ( 1, 0, PP_VMASKMOVPD_TOMEM, MMMMM_VMASKMOVPD_TOMEM, AVXOP_VMASKMOVPD_TOMEM, sseSrcReg, sseMaskReg, x64BaseReg, x64IndexReg, Scale, Offset );
}

bool x64Encoder::extractps ( long x64DestReg, long sseSrcReg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 0, 0, PP_EXTRACTPS, MMMMM_EXTRACTPS, AVXOP_EXTRACTPS, sseSrcReg, 0, x64DestReg, Imm8 );
}

bool x64Encoder::movmskps256 ( long x64DestReg, long sseSrcReg )
{
	return x64EncodeRegRegV ( 1, 0, PP_MOVMSKPS, MMMMM_MOVMSKPS, AVXOP_MOVMSKPS, x64DestReg, 0, sseSrcReg );
}


bool x64Encoder::movmskpsregreg ( long x64DestReg, long sseSrcReg )
{
	//x64EncodePrefix ( 0xf2 );
	return x64EncodeRegReg32 ( MAKE2OPCODE( 0x0f, AVXOP_MOVMSKPS ), x64DestReg, sseSrcReg );
}


bool x64Encoder::vperm2f128 ( long sseDestReg, long sseSrc1Reg, long sseSrc2Reg, char Imm8 )
{
	return x64EncodeRegVImm8 ( 1, 0, PP_VPERM2F128, MMMMM_VPERM2F128, AVXOP_VPERM2F128, sseDestReg, sseSrc1Reg, sseSrc2Reg, Imm8 );
}



bool x64Encoder::JmpReg64 ( long SrcReg )
{
	return x64EncodeReg32 ( 0xff, 0x4, SrcReg );
}

bool x64Encoder::JmpMem64 ( long long* SrcPtr )
{
	return x64EncodeRipOffset32 ( 0xff, 0x4, (char*) SrcPtr );
}

// these are used to make a short term hop while encoding x64 instructions
bool x64Encoder::Jmp ( long Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JMP );
	result = x64EncodeImmediate32 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp_NE ( long Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( MAKE2OPCODE ( X64OP1_JMP_NE, X64OP2_JMP_NE ) );
	result = x64EncodeImmediate32 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp_E ( long Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( MAKE2OPCODE ( X64OP1_JMP_E, X64OP2_JMP_E ) );
	result = x64EncodeImmediate32 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp_L ( long Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( MAKE2OPCODE ( X64OP1_JMP_L, X64OP2_JMP_L ) );
	result = x64EncodeImmediate32 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp_LE ( long Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( MAKE2OPCODE ( X64OP1_JMP_LE, X64OP2_JMP_LE ) );
	result = x64EncodeImmediate32 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp_G ( long Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( MAKE2OPCODE ( X64OP1_JMP_G, X64OP2_JMP_G ) );
	result = x64EncodeImmediate32 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp_GE ( long Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( MAKE2OPCODE ( X64OP1_JMP_GE, X64OP2_JMP_GE ) );
	result = x64EncodeImmediate32 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp_B ( long Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( MAKE2OPCODE ( X64OP1_JMP_B, X64OP2_JMP_B ) );
	result = x64EncodeImmediate32 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp_BE ( long Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( MAKE2OPCODE ( X64OP1_JMP_BE, X64OP2_JMP_BE ) );
	result = x64EncodeImmediate32 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp_A ( long Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( MAKE2OPCODE ( X64OP1_JMP_A, X64OP2_JMP_A ) );
	result = x64EncodeImmediate32 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp_AE ( long Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( MAKE2OPCODE ( X64OP1_JMP_AE, X64OP2_JMP_AE ) );
	result = x64EncodeImmediate32 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}



// jump short
bool x64Encoder::Jmp8 ( char Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JMP8 );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}


// jump short if unsigned above (carry flag=0 and zero flag=0)
bool x64Encoder::Jmp8_E ( char Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JE );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

// jump short if unsigned above (carry flag=0 and zero flag=0)
bool x64Encoder::Jmp8_NE ( char Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JNE );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

// jump short if unsigned above (carry flag=0 and zero flag=0)
bool x64Encoder::Jmp8_A ( char Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JA );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}
	
// jump short if unsigned above (carry flag=0 and zero flag=0)
bool x64Encoder::Jmp8_AE ( char Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JAE );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

// jump short if unsigned above (carry flag=0 and zero flag=0)
bool x64Encoder::Jmp8_B ( char Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JB );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

// jump short if unsigned above (carry flag=0 and zero flag=0)
bool x64Encoder::Jmp8_BE ( char Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JBE );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}


bool x64Encoder::Jmp8_G ( char Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JG );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp8_GE ( char Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JGE );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp8_L ( char Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JL );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp8_LE ( char Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JLE );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}


bool x64Encoder::Jmp8_NO ( char Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JNO );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}


bool x64Encoder::Jmp8_CXZ ( char Offset, unsigned long Label )
{
	bool result;
	x64Encode16 ( X64OP_JCX );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp8_ECXZ ( char Offset, unsigned long Label )
{
	bool result;
	x64EncodeOpcode ( X64OP_JECX );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}

bool x64Encoder::Jmp8_RCXZ ( char Offset, unsigned long Label )
{
	bool result;
	x64Encode64 ( X64OP_JRCX );
	result = x64EncodeImmediate8 ( Offset );
	BranchOffset [ Label ] = x64NextOffset;
	return result;
}



// these are used to set the target address of jump once you find out what it is
bool x64Encoder::SetJmpTarget ( unsigned long Label )
{
	long NextOffsetSave, BranchTargetOffset;
	
	if ( BranchOffset [ Label ] == -1 ) return true;
	
	NextOffsetSave = x64NextOffset;
	
//	JmpTargetOffset = x64NextOffset - JmpOffset;
	BranchTargetOffset = x64NextOffset - BranchOffset [ Label ];
/*	
	// set offset for unconditional jump
	x64NextOffset = JmpOffset - 4;
	if ( JmpOffset > 0 ) x64EncodeImmediate32 ( JmpTargetOffset );
*/
	// set offset for conditional jump
	x64NextOffset = BranchOffset [ Label ] - 4;
	x64EncodeImmediate32 ( BranchTargetOffset );
	
	// restore next offset
	x64NextOffset = NextOffsetSave;
	
	// clear stuff for jumping
//	JmpOffset = -1;
	BranchOffset [ Label ] = -1;

	return true;
}

bool x64Encoder::SetJmpTarget8 ( unsigned long Label )
{
	long NextOffsetSave, BranchTargetOffset;
	
	if ( BranchOffset [ Label ] == -1 ) return true;
	
	NextOffsetSave = x64NextOffset;
	
	BranchTargetOffset = x64NextOffset - BranchOffset [ Label ];
	
	// set offset for conditional jump
	x64NextOffset = BranchOffset [ Label ] - 1;
	x64EncodeImmediate8 ( (char) ( BranchTargetOffset & 0xff ) );
	
	// restore next offset
	x64NextOffset = NextOffsetSave;
	
	// make sure the offset is not greater than signed 8-bits
	if ( BranchTargetOffset < -128 || BranchTargetOffset > 127 ) return false;
	
	BranchOffset [ Label ] = -1;

	return true;
}




// self-optimizing unconditional jump
bool x64Encoder::JMP ( void* Target )
{
	long Offset;
	
	// try short jump first //

	// get offset to data
	Offset = (long) ( ( (char*) Target ) - ( & ( x64CodeArea [ x64NextOffset + 2 ] ) ) );
	
	if ( Offset >= -128 && Offset <= 127 )
	{
		x64EncodeOpcode ( X64OP_JMP8 );
		return x64EncodeImmediate8 ( Offset );
	}
	
	// short jump not possible, so do the long jump //
	
	Offset = (long) ( ( (char*) Target ) - ( & ( x64CodeArea [ x64NextOffset + 5 ] ) ) );
	x64EncodeOpcode ( X64OP_JMP );
	return x64EncodeImmediate32 ( Offset );
}

// self-optimizing jump on not-equal
bool x64Encoder::JMP_E ( void* Target )
{
	long Offset;
	
	// try short jump first //

	// get offset to data
	Offset = (long) ( ( (char*) Target ) - ( & ( x64CodeArea [ x64NextOffset + 2 ] ) ) );
	
	if ( Offset >= -128 && Offset <= 127 )
	{
		x64EncodeOpcode ( X64OP_JE );
		return x64EncodeImmediate8 ( Offset );
	}
	
	// short jump not possible, so do the long jump //
	
	Offset = (long) ( ( (char*) Target ) - ( & ( x64CodeArea [ x64NextOffset + 6 ] ) ) );
	x64EncodeOpcode ( MAKE2OPCODE ( X64OP1_JMP_E, X64OP2_JMP_E ) );
	return x64EncodeImmediate32 ( Offset );
}


// self-optimizing jump on not-equal
bool x64Encoder::JMP_NE ( void* Target )
{
	long Offset;
	
	// try short jump first //

	// get offset to data
	Offset = (long) ( ( (char*) Target ) - ( & ( x64CodeArea [ x64NextOffset + 2 ] ) ) );
	
	if ( Offset >= -128 && Offset <= 127 )
	{
		x64EncodeOpcode ( X64OP_JNE );
		return x64EncodeImmediate8 ( Offset );
	}
	
	// short jump not possible, so do the long jump //
	
	Offset = (long) ( ( (char*) Target ) - ( & ( x64CodeArea [ x64NextOffset + 6 ] ) ) );
	x64EncodeOpcode ( MAKE2OPCODE ( X64OP1_JMP_NE, X64OP2_JMP_NE ) );
	return x64EncodeImmediate32 ( Offset );
}

bool x64Encoder::Jmp ( void* Target )
{
	long Offset;
	
	//x64EncodeRexReg32 ( x64DestReg, 0 );
	x64EncodeOpcode ( X64OP_JMP );

	// add in modr/m
	//x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64DestReg, RMBASE_USINGRIP );
	
	// get offset to data
	Offset = (long) ( ( (char*) Target ) - ( & ( x64CodeArea [ x64NextOffset + 4 ] ) ) );
	
	//cout << hex << "\nOffset=" << Offset << " Code Address=" << (unsigned long long) & ( x64CodeArea [ x64NextOffset + 4 ] ) << "\n";
	
	return x64EncodeImmediate32 ( Offset );
}

bool x64Encoder::Jmp_B ( void* Target )
{
	long Offset;
	
	//x64EncodeOpcode ( X64OP_JMP );
	x64EncodeOpcode ( MAKE2OPCODE ( X64OP1_JMP_B, X64OP2_JMP_B ) );

	// add in modr/m
	//x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64DestReg, RMBASE_USINGRIP );
	
	// get offset to data
	Offset = (long) ( ( (char*) Target ) - ( & ( x64CodeArea [ x64NextOffset + 4 ] ) ) );
	
	//cout << hex << "\nOffset=" << Offset << " Code Address=" << (unsigned long long) & ( x64CodeArea [ x64NextOffset + 4 ] ) << "\n";
	
	return x64EncodeImmediate32 ( Offset );
}


bool x64Encoder::Call ( const void* Target )
{
	long Offset;
	
	//x64EncodeRexReg32 ( x64DestReg, 0 );
	x64EncodeOpcode ( X64OP_CALL );

	// add in modr/m
	//x64CodeArea [ x64NextOffset++ ] = MAKE_MODRMREGMEM( REGMEM_NOOFFSET, x64DestReg, RMBASE_USINGRIP );
	
	// get offset to data
	Offset = (long) ( ( (char*) Target ) - ( & ( x64CodeArea [ x64NextOffset + 4 ] ) ) );
	
	//cout << hex << "\nOffset=" << Offset << " Code Address=" << (unsigned long long) & ( x64CodeArea [ x64NextOffset + 4 ] ) << "\n";
	
	return x64EncodeImmediate32 ( Offset );
}


