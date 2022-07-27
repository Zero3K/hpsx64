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

#ifndef _R3000A_LOOKUP_H_

#define _R3000A_LOOKUP_H_

#include "types.h"
#include "R3000A_Instruction.h"

#include <iostream>
using namespace std;

namespace R3000A
{

	namespace Instruction
	{

		// this is an entry in the list of instructons
		//template<class TypeOfFunction>
		class Entry
		{
		public:
		
			// this is the name of the instruction
			char* Name;

			// this says how instruction is defined
			u8 Opcode;
			u8 Rs;
			u8 Rt;
			u8 Funct;
			#define DOES_NOT_MATTER 0xffffffff
			#define ANY_VALUE 0xff
			
			// this is the function to call to perform operation with instruction (like execute, print, etc)
			//TypeOfFunction FunctionToCall;
			u8 InstructionIndex;
		};
		
		// class for looking up the instruction index to find out what instruction is which
		class Lookup
		{
			// list of instructions
			enum
			{
				// instructions on both R3000A and R5900
				IDX_INVALID = 0,
				IDX_J, IDX_JAL, IDX_JR, IDX_JALR, IDX_BEQ, IDX_BNE, IDX_BGTZ, IDX_BGEZ,
				IDX_BLTZ, IDX_BLEZ, IDX_BGEZAL, IDX_BLTZAL, IDX_ADD, IDX_ADDI, IDX_ADDU, IDX_ADDIU,
				IDX_SUB, IDX_SUBU, IDX_MULT, IDX_MULTU, IDX_DIV, IDX_DIVU, IDX_AND, IDX_ANDI,
				IDX_OR, IDX_ORI, IDX_XOR, IDX_XORI, IDX_NOR, IDX_LUI, IDX_SLL, IDX_SRL,
				IDX_SRA, IDX_SLLV, IDX_SRLV, IDX_SRAV, IDX_SLT, IDX_SLTI, IDX_SLTU, IDX_SLTIU,
				IDX_LB, IDX_LBU, IDX_LH, IDX_LHU, IDX_LW, IDX_LWL, IDX_LWR, IDX_SB,
				IDX_SH, IDX_SW, IDX_SWL, IDX_SWR, IDX_MFHI, IDX_MTHI, IDX_MFLO, IDX_MTLO,
				IDX_MFC0, IDX_MTC0, IDX_CFC2, IDX_CTC2, IDX_SYSCALL, IDX_BREAK,
				
				// instructions on R3000A ONLY
				IDX_MFC2, IDX_MTC2, IDX_LWC2, IDX_SWC2, IDX_RFE,
				IDX_RTPS, IDX_RTPT, IDX_CC, IDX_CDP, IDX_DCPL, IDX_DPCS, IDX_DPCT, IDX_NCS,
				IDX_NCT, IDX_NCDS, IDX_NCDT, IDX_NCCS, IDX_NCCT, IDX_GPF, IDX_GPL, IDX_AVSZ3,
				IDX_AVSZ4, IDX_SQR, IDX_OP, IDX_NCLIP, IDX_INTPL, IDX_MVMVA
			};
			
			static const int c_iOpcode_MaxValue = 64;
			static const int c_iRs_MaxValue = 32;
			static const int c_iRt_MaxValue = 32;
			static const int c_iRd_MaxValue = 32;
			static const int c_iShiftAmount_MaxValue = 32;
			static const int c_iFunct_MaxValue = 64;
			
			// instruction definitions list
			static Entry Entries [];
			
			static const u32 c_iLookupTable_Size = c_iOpcode_MaxValue * c_iRs_MaxValue * c_iRt_MaxValue * c_iFunct_MaxValue;
			static const u32 c_iLookupTable_Mask = c_iLookupTable_Size - 1;
			
			static bool c_bObjectInitialized;
			
			// this actually has to be a 4D array, but I'll make it one array			
			// use Opcode, Rs, Rt, and Funct to lookup value
			alignas(32) static u8 LookupTable [ c_iLookupTable_Size ];
			
		public:
			// call this before doing anything
			static void Start ();
			
			inline static u8 FindByInstruction ( u32 instruction ) { return LookupTable [ ( ( instruction << 16 ) | ( instruction >> 16 ) ) & c_iLookupTable_Mask ]; };
		};

	};
	
};


#endif

