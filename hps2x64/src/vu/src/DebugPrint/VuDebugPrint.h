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



#include <sstream>
#include <string>


#include "MipsOpcode.h"


namespace VuDebugPrint
{
	using namespace std;

	typedef void (*Function) ( stringstream &strInstString, long instruction );
	typedef void (*FunctionVLIW) ( stringstream &strInstString, unsigned long long instruction );




	const char* BCType [ 4 ] = { "F", "T", "FL", "TL" };

	void PrintInstruction ( stringstream &strInstString, unsigned long long instruction );
	void PrintUpperInstruction ( stringstream &strInstString, long instruction );
	void PrintLowerInstruction ( stringstream &strInstString, long instruction );

	void Normal ( stringstream &strInstString, unsigned long long instruction );
	void IImmediate ( stringstream &strInstString, unsigned long long instruction );
	void EBit ( stringstream &strInstString, unsigned long long instruction );
	void MInterlock ( stringstream &strInstString, unsigned long long instruction );
	void TDebug ( stringstream &strInstString, unsigned long long instruction );
	void DDebug ( stringstream &strInstString, unsigned long long instruction );


	// list of instructions
	// UPPER
	enum
	{
		IDX_NOP, IDX_ABS, IDX_CLIP, IDX_ITOF0, IDX_FTOI0, IDX_ITOF4, IDX_FTOI4, IDX_ITOF12,
		IDX_FTOI12, IDX_ITOF15, IDX_FTOI15,
		IDX_MAXBCX, IDX_MAXBCY, IDX_MAXBCZ, IDX_MAXBCW, IDX_MINIBCX, IDX_MINIBCY, IDX_MINIBCZ, IDX_MINIBCW,
		IDX_ADDBCX, IDX_ADDBCY, IDX_ADDBCZ, IDX_ADDBCW, IDX_SUBBCX, IDX_SUBBCY, IDX_SUBBCZ, IDX_SUBBCW,
		IDX_MULBCX, IDX_MULBCY, IDX_MULBCZ, IDX_MULBCW, IDX_MADDBCX, IDX_MADDBCY, IDX_MADDBCZ, IDX_MADDBCW,
		IDX_MSUBBCX, IDX_MSUBBCY, IDX_MSUBBCZ, IDX_MSUBBCW,
		IDX_ADDABCX, IDX_ADDABCY, IDX_ADDABCZ, IDX_ADDABCW, IDX_SUBABCX, IDX_SUBABCY, IDX_SUBABCZ, IDX_SUBABCW,
		IDX_MULABCX, IDX_MULABCY, IDX_MULABCZ, IDX_MULABCW, IDX_MADDABCX, IDX_MADDABCY, IDX_MADDABCZ, IDX_MADDABCW,
		IDX_MSUBABCX, IDX_MSUBABCY, IDX_MSUBABCZ, IDX_MSUBABCW,
		IDX_MAX, IDX_MAXI, IDX_MINI, IDX_MINII, IDX_ADD, IDX_ADDI, IDX_ADDQ, IDX_SUB,
		IDX_SUBI, IDX_SUBQ, IDX_MUL, IDX_MULI, IDX_MULQ, IDX_MADD, IDX_MADDI, IDX_MADDQ,
		IDX_MSUB, IDX_MSUBI, IDX_MSUBQ,
		IDX_ADDA, IDX_ADDAI, IDX_ADDAQ, IDX_SUBA, IDX_SUBAI, IDX_SUBAQ, IDX_MULA, IDX_MULAI,
		IDX_MULAQ, IDX_MADDA, IDX_MADDAI, IDX_MADDAQ, IDX_MSUBA, IDX_MSUBAI, IDX_MSUBAQ,
		IDX_OPMULA, IDX_OPMSUM, IDX_OPMSUB
	};

	// list of instructions
	// LOWER
	enum
	{
		IDX_MOVE, IDX_MR32, IDX_MFIR, IDX_MTIR,
		IDX_FCEQ, IDX_FCGET, IDX_FCSET, IDX_FCAND, IDX_FCOR, IDX_FMEQ, IDX_FMAND, IDX_FMOR,
		IDX_FSEQ, IDX_FSSET, IDX_FSAND, IDX_FSOR,
		IDX_B, IDX_BAL, IDX_JR, IDX_JALR, IDX_IBEQ, IDX_IBNE, IDX_IBLTZ, IDX_IBGTZ,
		IDX_IBLEZ, IDX_IBGEZ,
		IDX_IADD, IDX_IADDI, IDX_IADDIU, IDX_ISUB, IDX_ISUBIU, IDX_IAND, IDX_IOR,
		IDX_ILW, IDX_ILWR, IDX_ISW, IDX_ISWR, IDX_LQ, IDX_LQD, IDX_LQI, IDX_SQ,
		IDX_SQD, IDX_SQI,
		IDX_WAITQ, IDX_DIV, IDX_SQRT, IDX_RSQRT,
		IDX_RINIT, IDX_RGET, IDX_RNEXT, IDX_RXOR,
		IDX_XITOP,
		IDX_XTOP, IDX_XGKICK,
		IDX_WAITP, IDX_MFP, IDX_ESUM, IDX_ERCPR, IDX_ESADD, IDX_ERSADD, IDX_EEXP, IDX_ELENG,
		IDX_ERLENG, IDX_ESQRT, IDX_ERSQRT, IDX_ESIN, IDX_EATAN, IDX_EATAN_XY, IDX_EATAN_XZ
	};

	// *** Functions for encoding decoded PS2 Mips64 instructions *** //

	void NOP ( stringstream &strInstString, long instruction );
	void ABS ( stringstream &strInstString, long instruction );
	void CLIP ( stringstream &strInstString, long instruction );
	void ITOF0 ( stringstream &strInstString, long instruction );
	void FTOI0 ( stringstream &strInstString, long instruction );
	void ITOF4 ( stringstream &strInstString, long instruction );
	void FTOI4 ( stringstream &strInstString, long instruction );
	void ITOF12 ( stringstream &strInstString, long instruction );
	void FTOI12 ( stringstream &strInstString, long instruction );
	void ITOF15 ( stringstream &strInstString, long instruction );
	void FTOI15 ( stringstream &strInstString, long instruction );
	
	void MAXBCX ( stringstream &strInstString, long instruction );
	void MAXBCY ( stringstream &strInstString, long instruction );
	void MAXBCZ ( stringstream &strInstString, long instruction );
	void MAXBCW ( stringstream &strInstString, long instruction );
	void MINIBCX ( stringstream &strInstString, long instruction );
	void MINIBCY ( stringstream &strInstString, long instruction );
	void MINIBCZ ( stringstream &strInstString, long instruction );
	void MINIBCW ( stringstream &strInstString, long instruction );
	void ADDBCX ( stringstream &strInstString, long instruction );
	void ADDBCY ( stringstream &strInstString, long instruction );
	void ADDBCZ ( stringstream &strInstString, long instruction );
	void ADDBCW ( stringstream &strInstString, long instruction );
	void SUBBCX ( stringstream &strInstString, long instruction );
	void SUBBCY ( stringstream &strInstString, long instruction );
	void SUBBCZ ( stringstream &strInstString, long instruction );
	void SUBBCW ( stringstream &strInstString, long instruction );
	void MULBCX ( stringstream &strInstString, long instruction );
	void MULBCY ( stringstream &strInstString, long instruction );
	void MULBCZ ( stringstream &strInstString, long instruction );
	void MULBCW ( stringstream &strInstString, long instruction );
	void MADDBCX ( stringstream &strInstString, long instruction );
	void MADDBCY ( stringstream &strInstString, long instruction );
	void MADDBCZ ( stringstream &strInstString, long instruction );
	void MADDBCW ( stringstream &strInstString, long instruction );
	void MSUBBCX ( stringstream &strInstString, long instruction );
	void MSUBBCY ( stringstream &strInstString, long instruction );
	void MSUBBCZ ( stringstream &strInstString, long instruction );
	void MSUBBCW ( stringstream &strInstString, long instruction );

	
	void ADDABCX ( stringstream &strInstString, long instruction );
	void ADDABCY ( stringstream &strInstString, long instruction );
	void ADDABCZ ( stringstream &strInstString, long instruction );
	void ADDABCW ( stringstream &strInstString, long instruction );
	void SUBABCX ( stringstream &strInstString, long instruction );
	void SUBABCY ( stringstream &strInstString, long instruction );
	void SUBABCZ ( stringstream &strInstString, long instruction );
	void SUBABCW ( stringstream &strInstString, long instruction );
	void MULABCX ( stringstream &strInstString, long instruction );
	void MULABCY ( stringstream &strInstString, long instruction );
	void MULABCZ ( stringstream &strInstString, long instruction );
	void MULABCW ( stringstream &strInstString, long instruction );
	void MADDABCX ( stringstream &strInstString, long instruction );
	void MADDABCY ( stringstream &strInstString, long instruction );
	void MADDABCZ ( stringstream &strInstString, long instruction );
	void MADDABCW ( stringstream &strInstString, long instruction );
	void MSUBABCX ( stringstream &strInstString, long instruction );
	void MSUBABCY ( stringstream &strInstString, long instruction );
	void MSUBABCZ ( stringstream &strInstString, long instruction );
	void MSUBABCW ( stringstream &strInstString, long instruction );
	
	void MAX ( stringstream &strInstString, long instruction );
	void MAXI ( stringstream &strInstString, long instruction );
	void MINI ( stringstream &strInstString, long instruction );
	void MINII ( stringstream &strInstString, long instruction );
	void ADD ( stringstream &strInstString, long instruction );
	void ADDI ( stringstream &strInstString, long instruction );
	void ADDQ ( stringstream &strInstString, long instruction );
	void SUB ( stringstream &strInstString, long instruction );
	void SUBI ( stringstream &strInstString, long instruction );
	void SUBQ ( stringstream &strInstString, long instruction );
	void MUL ( stringstream &strInstString, long instruction );
	void MULI ( stringstream &strInstString, long instruction );
	void MULQ ( stringstream &strInstString, long instruction );
	void MADD ( stringstream &strInstString, long instruction );
	void MADDI ( stringstream &strInstString, long instruction );
	void MADDQ ( stringstream &strInstString, long instruction );
	void MSUB ( stringstream &strInstString, long instruction );
	void MSUBI ( stringstream &strInstString, long instruction );
	void MSUBQ ( stringstream &strInstString, long instruction );
	
	void ADDA ( stringstream &strInstString, long instruction );
	void ADDAI ( stringstream &strInstString, long instruction );
	void ADDAQ ( stringstream &strInstString, long instruction );
	void SUBA ( stringstream &strInstString, long instruction );
	void SUBAI ( stringstream &strInstString, long instruction );
	void SUBAQ ( stringstream &strInstString, long instruction );
	void MULA ( stringstream &strInstString, long instruction );
	void MULAI ( stringstream &strInstString, long instruction );
	void MULAQ ( stringstream &strInstString, long instruction );
	void MADDA ( stringstream &strInstString, long instruction );
	void MADDAI ( stringstream &strInstString, long instruction );
	void MADDAQ ( stringstream &strInstString, long instruction );
	void MSUBA ( stringstream &strInstString, long instruction );
	void MSUBAI ( stringstream &strInstString, long instruction );
	void MSUBAQ ( stringstream &strInstString, long instruction );


	
	void OPMULA ( stringstream &strInstString, long instruction );
	void OPMSUM ( stringstream &strInstString, long instruction );
	void OPMSUB ( stringstream &strInstString, long instruction );

	// lower instructions



	void MOVE ( stringstream &strInstString, long instruction );
	void MR32 ( stringstream &strInstString, long instruction );
	void MFIR ( stringstream &strInstString, long instruction );
	void MTIR ( stringstream &strInstString, long instruction );
	
	void FCEQ ( stringstream &strInstString, long instruction );
	void FCGET ( stringstream &strInstString, long instruction );
	void FCSET ( stringstream &strInstString, long instruction );
	void FCAND ( stringstream &strInstString, long instruction );
	void FCOR ( stringstream &strInstString, long instruction );
	void FMEQ ( stringstream &strInstString, long instruction );
	void FMAND ( stringstream &strInstString, long instruction );
	void FMOR ( stringstream &strInstString, long instruction );
	void FSEQ ( stringstream &strInstString, long instruction );
	void FSSET ( stringstream &strInstString, long instruction );
	void FSAND ( stringstream &strInstString, long instruction );
	void FSOR ( stringstream &strInstString, long instruction );
	
	void B ( stringstream &strInstString, long instruction );
	void BAL ( stringstream &strInstString, long instruction );
	void JR ( stringstream &strInstString, long instruction );
	void JALR ( stringstream &strInstString, long instruction );
	void IBEQ ( stringstream &strInstString, long instruction );
	void IBNE ( stringstream &strInstString, long instruction );
	void IBLTZ ( stringstream &strInstString, long instruction );
	void IBGTZ ( stringstream &strInstString, long instruction );
	void IBLEZ ( stringstream &strInstString, long instruction );
	void IBGEZ ( stringstream &strInstString, long instruction );

	
	void IADD ( stringstream &strInstString, long instruction );
	void IADDI ( stringstream &strInstString, long instruction );
	void IADDIU ( stringstream &strInstString, long instruction );
	void ISUB ( stringstream &strInstString, long instruction );
	void ISUBIU ( stringstream &strInstString, long instruction );
	void IAND ( stringstream &strInstString, long instruction );
	void IOR ( stringstream &strInstString, long instruction );

	void ILW ( stringstream &strInstString, long instruction );
	void ILWR ( stringstream &strInstString, long instruction );
	void ISW ( stringstream &strInstString, long instruction );
	void ISWR ( stringstream &strInstString, long instruction );
	void LQ ( stringstream &strInstString, long instruction );
	void LQD ( stringstream &strInstString, long instruction );
	void LQI ( stringstream &strInstString, long instruction );
	void SQ ( stringstream &strInstString, long instruction );
	void SQD ( stringstream &strInstString, long instruction );
	void SQI ( stringstream &strInstString, long instruction );

	void WAITQ ( stringstream &strInstString, long instruction );
	void DIV ( stringstream &strInstString, long instruction );
	void SQRT ( stringstream &strInstString, long instruction );
	void RSQRT ( stringstream &strInstString, long instruction );

	void RINIT ( stringstream &strInstString, long instruction );
	void RGET ( stringstream &strInstString, long instruction );
	void RNEXT ( stringstream &strInstString, long instruction );
	void RXOR ( stringstream &strInstString, long instruction );
	
	void XITOP ( stringstream &strInstString, long instruction );
	
	// VU1 instructions only? //
	
	void XTOP ( stringstream &strInstString, long instruction );
	void XGKICK ( stringstream &strInstString, long instruction );
	
	void WAITP ( stringstream &strInstString, long instruction );
	void MFP ( stringstream &strInstString, long instruction );
	void ESUM ( stringstream &strInstString, long instruction );
	void ERCPR ( stringstream &strInstString, long instruction );
	void ESADD ( stringstream &strInstString, long instruction );
	void ERSADD ( stringstream &strInstString, long instruction );
	void EEXP ( stringstream &strInstString, long instruction );
	void ELENG ( stringstream &strInstString, long instruction );
	void ERLENG ( stringstream &strInstString, long instruction );
	void ESQRT ( stringstream &strInstString, long instruction );
	void ERSQRT ( stringstream &strInstString, long instruction );
	void ESIN ( stringstream &strInstString, long instruction );
	void EATAN ( stringstream &strInstString, long instruction );
	void EATANxy ( stringstream &strInstString, long instruction );
	void EATANxz ( stringstream &strInstString, long instruction );


	const char XyzwLUT [ 4 ] = { 'x', 'y', 'z', 'w' };

	void AddVuDestArgs ( stringstream &strVuArgs, long Instruction );

	void AddInstArgs ( stringstream &strMipsArgs, long Instruction, long InstFormat );

	
	
}













