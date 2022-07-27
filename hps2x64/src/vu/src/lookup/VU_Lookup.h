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

#ifndef _VU_LOOKUP_H_

#define _VU_LOOKUP_H_

#include "types.h"
#include "StringUtils.h"
#include "VU_Instruction.h"

#include <iostream>
using namespace std;

using namespace Utilities::Strings;

namespace Vu
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
			//u8 Rs;
			//u8 Rt;
			//u8 Rd;
			u8 Fd;
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
				IDX_INVALID,
				
				// VU macro mode instructions //
				
				//IDX_COP2
				//IDX_QMFC2_NI, IDX_QMFC2_I, IDX_QMTC2_NI, IDX_QMTC2_I, IDX_LQC2, IDX_SQC2,
				//IDX_CALLMS, IDX_CALLMSR,
				
				// upper instructions //
				
				IDX_ABS,
				IDX_ADD, IDX_ADDi, IDX_ADDq, IDX_ADDBCX, IDX_ADDBCY, IDX_ADDBCZ, IDX_ADDBCW,
				IDX_ADDA, IDX_ADDAi, IDX_ADDAq, IDX_ADDABCX, IDX_ADDABCY, IDX_ADDABCZ, IDX_ADDABCW,
				IDX_CLIP,
				IDX_FTOI0, IDX_FTOI4, IDX_FTOI12, IDX_FTOI15,
				IDX_ITOF0, IDX_ITOF4, IDX_ITOF12, IDX_ITOF15,
				
				IDX_MADD, IDX_MADDi, IDX_MADDq, IDX_MADDBCX, IDX_MADDBCY, IDX_MADDBCZ, IDX_MADDBCW,
				IDX_MADDA, IDX_MADDAi, IDX_MADDAq, IDX_MADDABCX, IDX_MADDABCY, IDX_MADDABCZ, IDX_MADDABCW,
				IDX_MAX, IDX_MAXi, IDX_MAXBCX, IDX_MAXBCY, IDX_MAXBCZ, IDX_MAXBCW,
				IDX_MINI, IDX_MINIi, IDX_MINIBCX, IDX_MINIBCY, IDX_MINIBCZ, IDX_MINIBCW,
				
				IDX_MSUB, IDX_MSUBi, IDX_MSUBq, IDX_MSUBBCX, IDX_MSUBBCY, IDX_MSUBBCZ, IDX_MSUBBCW,
				IDX_MSUBA, IDX_MSUBAi, IDX_MSUBAq, IDX_MSUBABCX, IDX_MSUBABCY, IDX_MSUBABCZ, IDX_MSUBABCW,
				IDX_MUL, IDX_MULi, IDX_MULq, IDX_MULBCX, IDX_MULBCY, IDX_MULBCZ, IDX_MULBCW,
				IDX_MULA, IDX_MULAi, IDX_MULAq, IDX_MULABCX, IDX_MULABCY, IDX_MULABCZ, IDX_MULABCW,
				IDX_NOP, IDX_OPMSUB, IDX_OPMULA,
				IDX_SUB, IDX_SUBi, IDX_SUBq, IDX_SUBBCX, IDX_SUBBCY, IDX_SUBBCZ, IDX_SUBBCW,
				IDX_SUBA, IDX_SUBAi, IDX_SUBAq, IDX_SUBABCX, IDX_SUBABCY, IDX_SUBABCZ, IDX_SUBABCW,
				
				// lower instructions //
				
				IDX_DIV,
				IDX_IADD, IDX_IADDI, IDX_IAND,
				IDX_ILWR,
				IDX_IOR, IDX_ISUB,
				IDX_ISWR,
				IDX_LQD, IDX_LQI,
				IDX_MFIR, IDX_MOVE, IDX_MR32, IDX_MTIR,
				IDX_RGET, IDX_RINIT, IDX_RNEXT,
				IDX_RSQRT,
				IDX_RXOR,
				IDX_SQD, IDX_SQI,
				IDX_SQRT,
				IDX_WAITQ,

				// instructions not in macro mode //
				
				IDX_B, IDX_BAL,
				IDX_FCAND, IDX_FCEQ, IDX_FCGET, IDX_FCOR, IDX_FCSET,
				IDX_FMAND, IDX_FMEQ, IDX_FMOR,
				IDX_FSAND, IDX_FSEQ, IDX_FSOR, IDX_FSSET,
				IDX_IADDIU,
				IDX_IBEQ, IDX_IBGEZ, IDX_IBGTZ, IDX_IBLEZ, IDX_IBLTZ, IDX_IBNE,
				IDX_ILW,
				IDX_ISUBIU, IDX_ISW,
				IDX_JALR, IDX_JR,
				IDX_LQ,
				IDX_MFP,
				IDX_SQ,
				IDX_WAITP,
				IDX_XGKICK, IDX_XITOP, IDX_XTOP,

				// External Unit //

				IDX_EATAN, IDX_EATANXY, IDX_EATANXZ, IDX_EEXP, IDX_ELENG, IDX_ERCPR, IDX_ERLENG, IDX_ERSADD,
				IDX_ERSQRT, IDX_ESADD, IDX_ESIN, IDX_ESQRT, IDX_ESUM
			};
			
			static const int c_iOpcode_MaxValue = 128;
			//static const int c_iRs_MaxValue = 32;
			//static const int c_iRt_MaxValue = 32;
			static const int c_iFd_MaxValue = 32;
			//static const int c_iShiftAmount_MaxValue = 32;
			static const int c_iFunct_MaxValue = 64;
			
			// instruction definitions list
			static const Entry EntriesLO [];
			static const Entry EntriesHI [];
			
			
			
			
			static const u32 c_iLookupTableLO_Size = c_iOpcode_MaxValue * c_iFd_MaxValue * c_iFunct_MaxValue;
			static const u32 c_iLookupTableLO_Mask = c_iLookupTableLO_Size - 1;
			
			static const u32 c_iLookupTableHI_Size = c_iFd_MaxValue * c_iFunct_MaxValue;
			static const u32 c_iLookupTableHI_Mask = c_iLookupTableHI_Size - 1;

			
			static bool c_bObjectInitialized;
			
			
			// this actually has to be a 4D array, but I'll make it one array			
			// use Opcode, Rs, Rt, Shift, and Funct to lookup value
			alignas(32) static u8 LookupTableHI [ c_iLookupTableHI_Size ];
			alignas(32) static u8 LookupTableLO [ c_iLookupTableLO_Size ];
			
		public:
			// call this before doing anything
			static void Start ();
			
			inline static u8 FindByInstructionLO ( u32 instruction ) { return LookupTableLO [ ( ( instruction >> 25 ) | ( instruction << 7 ) ) & c_iLookupTableLO_Mask ]; }
			inline static u8 FindByInstructionHI ( u32 instruction ) { return LookupTableHI [ instruction & c_iLookupTableHI_Mask ]; }
			
			// returns -1 on error
			static int FindByNameLO ( string NameOfInstruction );
			static int FindByNameHI ( string NameOfInstruction );
		};
	};
	
};


#endif

