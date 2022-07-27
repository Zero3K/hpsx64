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

#ifndef _R5900_LOOKUP_H_

#define _R5900_LOOKUP_H_

#include "types.h"
#include "StringUtils.h"
#include "R5900_Instruction.h"

#include <iostream>
using namespace std;

using namespace Utilities::Strings;

namespace R5900
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
			//u8 Rd;
			u8 Shift;
			u8 Funct;
			#define DOES_NOT_MATTER 0xffffffff
			#define ANY_VALUE 0xff
			
			// this is the function to call to perform operation with instruction (like execute, print, etc)
			//TypeOfFunction FunctionToCall;
			u16 InstructionIndex;
		};

		// class for looking up the instruction index to find out what instruction is which
		class Lookup
		{
			// list of instructions
			enum
			{
				// instructions on both R3000A and R5900
				// 1 + 56 + 6 = 63 instructions //
				IDX_Invalid = 0,
				IDX_J, IDX_JAL, IDX_JR, IDX_JALR, IDX_BEQ, IDX_BNE, IDX_BGTZ, IDX_BGEZ,
				IDX_BLTZ, IDX_BLEZ, IDX_BGEZAL, IDX_BLTZAL, IDX_ADD, IDX_ADDI, IDX_ADDU, IDX_ADDIU,
				IDX_SUB, IDX_SUBU, IDX_MULT, IDX_MULTU, IDX_DIV, IDX_DIVU, IDX_AND, IDX_ANDI,
				IDX_OR, IDX_ORI, IDX_XOR, IDX_XORI, IDX_NOR, IDX_LUI, IDX_SLL, IDX_SRL,
				IDX_SRA, IDX_SLLV, IDX_SRLV, IDX_SRAV, IDX_SLT, IDX_SLTI, IDX_SLTU, IDX_SLTIU,
				IDX_LB, IDX_LBU, IDX_LH, IDX_LHU, IDX_LW, IDX_LWL, IDX_LWR, IDX_SB,
				IDX_SH, IDX_SW, IDX_SWL, IDX_SWR, IDX_MFHI, IDX_MTHI, IDX_MFLO, IDX_MTLO,
				IDX_MFC0, IDX_MTC0,
				IDX_CFC2_I, IDX_CTC2_I, IDX_CFC2_NI, IDX_CTC2_NI,
				IDX_SYSCALL, IDX_BREAK,
				
				// instructions on R3000A ONLY
				//IDX_MFC2, IDX_MTC2, IDX_LWC2, IDX_SWC2, IDX_RFE,
				//IDX_RTPS, IDX_RTPT, IDX_CC, IDX_CDP, IDX_DCPL, IDX_DPCS, IDX_DPCT, IDX_NCS,
				//IDX_NCT, IDX_NCDS, IDX_NCDT, IDX_NCCS, IDX_NCCT, IDX_GPF, IDX_GPL, IDX_AVSZ3,
				//IDX_AVSZ4, IDX_SQR, IDX_OP, IDX_NCLIP, IDX_INTPL, IDX_MVMVA
				
				// instructions on R5900 ONLY
				// (24*8) + 4 + 6 = 192 + 10 = 202 instructions //
				IDX_BEQL, IDX_BNEL, IDX_BGEZL, IDX_BGTZL, IDX_BLEZL, IDX_BLTZL, IDX_BGEZALL, IDX_BLTZALL,
				IDX_DADD, IDX_DADDI, IDX_DADDU, IDX_DADDIU, IDX_DSUB, IDX_DSUBU, IDX_DSLL, IDX_DSLL32,
				IDX_DSLLV, IDX_DSRA, IDX_DSRA32, IDX_DSRAV, IDX_DSRL, IDX_DSRL32, IDX_DSRLV, IDX_LD,
				IDX_LDL, IDX_LDR, IDX_LWU, IDX_LQ, IDX_PREF, IDX_SD, IDX_SDL, IDX_SDR,
				IDX_SQ, IDX_TEQ, IDX_TEQI, IDX_TNE, IDX_TNEI, IDX_TGE, IDX_TGEI, IDX_TGEU,
				IDX_TGEIU, IDX_TLT, IDX_TLTI, IDX_TLTU, IDX_TLTIU, IDX_MOVN, IDX_MOVZ, IDX_MULT1,
				IDX_MULTU1, IDX_DIV1, IDX_DIVU1, IDX_MADD, IDX_MADD1, IDX_MADDU, IDX_MADDU1, IDX_MFHI1,
				IDX_MTHI1, IDX_MFLO1, IDX_MTLO1, IDX_MFSA, IDX_MTSA, IDX_MTSAB, IDX_MTSAH,
				IDX_PABSH, IDX_PABSW, IDX_PADDB, IDX_PADDH, IDX_PADDW, IDX_PADDSB, IDX_PADDSH, IDX_PADDSW,
				IDX_PADDUB, IDX_PADDUH, IDX_PADDUW, IDX_PADSBH, IDX_PAND, IDX_POR, IDX_PXOR, IDX_PNOR,
				IDX_PCEQB, IDX_PCEQH, IDX_PCEQW, IDX_PCGTB, IDX_PCGTH, IDX_PCGTW, IDX_PCPYH, IDX_PCPYLD,
				IDX_PCPYUD, IDX_PDIVBW, IDX_PDIVUW, IDX_PDIVW, IDX_PEXCH, IDX_PEXCW, IDX_PEXEH, IDX_PEXEW,
				IDX_PEXT5, IDX_PEXTLB, IDX_PEXTLH, IDX_PEXTLW, IDX_PEXTUB, IDX_PEXTUH, IDX_PEXTUW, IDX_PHMADH,
				IDX_PHMSBH, IDX_PINTEH, IDX_PINTH, IDX_PLZCW, IDX_PMADDH, IDX_PMADDW, IDX_PMADDUW, IDX_PMAXH,
				IDX_PMAXW, IDX_PMINH, IDX_PMINW, IDX_PMFHI, IDX_PMFLO, IDX_PMTHI, IDX_PMTLO, IDX_PMFHL_LH,
				IDX_PMFHL_SH, IDX_PMFHL_LW, IDX_PMFHL_UW, IDX_PMFHL_SLW, IDX_PMTHL_LW, IDX_PMSUBH, IDX_PMSUBW, IDX_PMULTH,
				IDX_PMULTW, IDX_PMULTUW, IDX_PPAC5, IDX_PPACB, IDX_PPACH, IDX_PPACW, IDX_PREVH, IDX_PROT3W,
				IDX_PSLLH, IDX_PSLLVW, IDX_PSLLW, IDX_PSRAH, IDX_PSRAW, IDX_PSRAVW, IDX_PSRLH, IDX_PSRLW,
				IDX_PSRLVW, IDX_PSUBB, IDX_PSUBH, IDX_PSUBW, IDX_PSUBSB, IDX_PSUBSH, IDX_PSUBSW, IDX_PSUBUB,
				IDX_PSUBUH, IDX_PSUBUW,
				IDX_QFSRV, IDX_SYNC,
				
				IDX_DI, IDX_EI, IDX_ERET, IDX_CACHE, IDX_TLBP, IDX_TLBR, IDX_TLBWI, IDX_TLBWR,
				IDX_CFC0, IDX_CTC0,
				
				IDX_BC0T, IDX_BC0TL, IDX_BC0F, IDX_BC0FL, IDX_BC1T, IDX_BC1TL, IDX_BC1F, IDX_BC1FL,
				IDX_BC2T, IDX_BC2TL, IDX_BC2F, IDX_BC2FL,
				
				// COP1 floating point instructions
				IDX_LWC1, IDX_SWC1, IDX_MFC1, IDX_MTC1, IDX_CFC1, IDX_CTC1,
				IDX_ABS_S, IDX_ADD_S, IDX_ADDA_S, IDX_C_EQ_S, IDX_C_F_S, IDX_C_LE_S, IDX_C_LT_S, IDX_CVT_S_W,
				IDX_CVT_W_S, IDX_DIV_S, IDX_MADD_S, IDX_MADDA_S, IDX_MAX_S, IDX_MIN_S, IDX_MOV_S, IDX_MSUB_S,
				IDX_MSUBA_S, IDX_MUL_S, IDX_MULA_S, IDX_NEG_S, IDX_RSQRT_S, IDX_SQRT_S, IDX_SUB_S, IDX_SUBA_S,
				
				// VU macro mode instructions
				IDX_QMFC2_NI, IDX_QMFC2_I, IDX_QMTC2_NI, IDX_QMTC2_I, IDX_LQC2, IDX_SQC2,
				
				IDX_VABS,
				IDX_VADD, IDX_VADDi, IDX_VADDq, IDX_VADDBCX, IDX_VADDBCY, IDX_VADDBCZ, IDX_VADDBCW,
				IDX_VADDA, IDX_VADDAi, IDX_VADDAq, IDX_VADDABCX, IDX_VADDABCY, IDX_VADDABCZ, IDX_VADDABCW,
				IDX_VCALLMS, IDX_VCALLMSR, IDX_VCLIP, IDX_VDIV,
				IDX_VFTOI0, IDX_VFTOI4, IDX_VFTOI12, IDX_VFTOI15,
				IDX_VIADD, IDX_VIADDI, IDX_VIAND, IDX_VILWR, IDX_VIOR, IDX_VISUB, IDX_VISWR,
				IDX_VITOF0, IDX_VITOF4, IDX_VITOF12, IDX_VITOF15,
				IDX_VLQD, IDX_VLQI,
				
				IDX_VMADD, IDX_VMADDi, IDX_VMADDq, IDX_VMADDBCX, IDX_VMADDBCY, IDX_VMADDBCZ, IDX_VMADDBCW,
				IDX_VMADDA, IDX_VMADDAi, IDX_VMADDAq, IDX_VMADDABCX, IDX_VMADDABCY, IDX_VMADDABCZ, IDX_VMADDABCW,
				IDX_VMAX, IDX_VMAXi, IDX_VMAXBCX, IDX_VMAXBCY, IDX_VMAXBCZ, IDX_VMAXBCW,
				IDX_VMFIR,
				IDX_VMINI, IDX_VMINIi, IDX_VMINIBCX, IDX_VMINIBCY, IDX_VMINIBCZ, IDX_VMINIBCW,
				IDX_VMOVE, IDX_VMR32,
				
				IDX_VMSUB, IDX_VMSUBi, IDX_VMSUBq, IDX_VMSUBBCX, IDX_VMSUBBCY, IDX_VMSUBBCZ, IDX_VMSUBBCW,
				IDX_VMSUBA, IDX_VMSUBAi, IDX_VMSUBAq, IDX_VMSUBABCX, IDX_VMSUBABCY, IDX_VMSUBABCZ, IDX_VMSUBABCW,
				IDX_VMTIR,
				IDX_VMUL, IDX_VMULi, IDX_VMULq, IDX_VMULBCX, IDX_VMULBCY, IDX_VMULBCZ, IDX_VMULBCW,
				IDX_VMULA, IDX_VMULAi, IDX_VMULAq, IDX_VMULABCX, IDX_VMULABCY, IDX_VMULABCZ, IDX_VMULABCW,
				IDX_VNOP, IDX_VOPMSUB, IDX_VOPMULA, IDX_VRGET, IDX_VRINIT, IDX_VRNEXT, IDX_VRSQRT, IDX_VRXOR,
				IDX_VSQD, IDX_VSQI, IDX_VSQRT,
				IDX_VSUB, IDX_VSUBi, IDX_VSUBq, IDX_VSUBBCX, IDX_VSUBBCY, IDX_VSUBBCZ, IDX_VSUBBCW,
				IDX_VSUBA, IDX_VSUBAi, IDX_VSUBAq, IDX_VSUBABCX, IDX_VSUBABCY, IDX_VSUBABCZ, IDX_VSUBABCW,
				IDX_VWAITQ,
				IDX_COP2
			};
			
			static const int c_iOpcode_MaxValue = 64;
			static const int c_iRs_MaxValue = 32;
			static const int c_iRt_MaxValue = 32;
			static const int c_iRd_MaxValue = 32;
			static const int c_iShiftAmount_MaxValue = 32;
			static const int c_iFunct_MaxValue = 64;
			
			// instruction definitions list
			static const Entry Entries [];
			
			
			
			
			static const u32 c_iLookupTable_Size = c_iOpcode_MaxValue * c_iRs_MaxValue * c_iRt_MaxValue * c_iShiftAmount_MaxValue * c_iFunct_MaxValue;
			static const u32 c_iLookupTable_Mask = c_iLookupTable_Size - 1;

			
			static bool c_bObjectInitialized;
			
			
			// this actually has to be a 4D array, but I'll make it one array			
			// use Opcode, Rs, Rt, Shift, and Funct to lookup value
			alignas(32) static u16 LookupTable [ c_iLookupTable_Size ];
			
		public:
			// call this before doing anything
			static void Start ();
			
			inline static u16 FindByInstruction ( u32 instruction ) { return LookupTable [ ( ( instruction << 16 ) | ( instruction >> 16 ) ) & c_iLookupTable_Mask ]; }
			
			// returns -1 on error
			static int FindByName ( string NameOfInstruction );
		};
	};
	
};


#endif

