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


#include "types.h"
#include "MipsOpcode.h"
#include "VU_Instruction.h"
#include "VU_Lookup.h"


#ifndef _VU_PRINT_H_
#define _VU_PRINT_H_


namespace Vu
{

	namespace Instruction
	{
	
		class Print
		{
		public:
			//using namespace std;

			typedef void (*Function) ( stringstream &s, long instruction );

			// this gets the instruction text
			//static string PrintInstruction ( long instruction );

			static const Function FunctionList [];
			
			static string PrintInstructionLO ( long instruction );
			static string PrintInstructionHI ( long instruction );
			
			
			
			static const char XyzwLUT [ 4 ];	// = { 'x', 'y', 'z', 'w' };
			static const char* BCType [ 4 ];// = { "F", "T", "FL", "TL" };

			// constructor
			//Print ();
			
			// creates lookup table from list of entries
			static void Start ();
		
			// returns true if the instruction has executed its last cycle
			// r - pointer to the Cpu object to execute the instruction on
			// i - the instruction to execute
			// CycleToExecute - the cycle to execute for the instruction. To execute first cycle, this should be zero
			//typedef bool (*Function) ( R3000A::Cpu* r, Instruction::Format i );
			
			//static Entry<Function> Entries [];

			// this actually has to be a 4D array, but I'll make it one array			
			// use Opcode, Rs, Rt, and Funct to lookup value
			//static Function LookupTable [ 64 * 32 * 32 * 64 ];
			//static Function LookupTable [];
			
			static void AddInstArgs ( stringstream &strMipsArgs, long Instruction, long InstFormat );
			static void AddVuDestArgs ( stringstream &strVuArgs, long Instruction );

		

			static void INVALID ( stringstream &s, long instruction );
			

			static void ADDBCX ( stringstream &strInstString, long instruction );
			static void ADDBCY ( stringstream &strInstString, long instruction );
			static void ADDBCZ ( stringstream &strInstString, long instruction );
			static void ADDBCW ( stringstream &strInstString, long instruction );
			
			static void SUBBCX ( stringstream &strInstString, long instruction );
			static void SUBBCY ( stringstream &strInstString, long instruction );
			static void SUBBCZ ( stringstream &strInstString, long instruction );
			static void SUBBCW ( stringstream &strInstString, long instruction );
			
			static void MADDBCX ( stringstream &strInstString, long instruction );
			static void MADDBCY ( stringstream &strInstString, long instruction );
			static void MADDBCZ ( stringstream &strInstString, long instruction );
			static void MADDBCW ( stringstream &strInstString, long instruction );
			
			static void MSUBBCX ( stringstream &strInstString, long instruction );
			static void MSUBBCY ( stringstream &strInstString, long instruction );
			static void MSUBBCZ ( stringstream &strInstString, long instruction );
			static void MSUBBCW ( stringstream &strInstString, long instruction );
			
			static void MAXBCX ( stringstream &strInstString, long instruction );
			static void MAXBCY ( stringstream &strInstString, long instruction );
			static void MAXBCZ ( stringstream &strInstString, long instruction );
			static void MAXBCW ( stringstream &strInstString, long instruction );
			
			static void MINIBCX ( stringstream &strInstString, long instruction );
			static void MINIBCY ( stringstream &strInstString, long instruction );
			static void MINIBCZ ( stringstream &strInstString, long instruction );
			static void MINIBCW ( stringstream &strInstString, long instruction );
			
			static void MULBCX ( stringstream &strInstString, long instruction );
			static void MULBCY ( stringstream &strInstString, long instruction );
			static void MULBCZ ( stringstream &strInstString, long instruction );
			static void MULBCW ( stringstream &strInstString, long instruction );
			
			static void MULq ( stringstream &strInstString, long instruction );
			
			static void MAXi ( stringstream &strInstString, long instruction );
			static void MULi ( stringstream &strInstString, long instruction );
			static void MINIi ( stringstream &strInstString, long instruction );
			static void ADDq ( stringstream &strInstString, long instruction );
			static void MADDq ( stringstream &strInstString, long instruction );
			static void ADDi ( stringstream &strInstString, long instruction );
			static void MADDi ( stringstream &strInstString, long instruction );
			static void OPMSUB ( stringstream &strInstString, long instruction );
			static void SUBq ( stringstream &strInstString, long instruction );
			static void MSUBq ( stringstream &strInstString, long instruction );
			static void SUBi ( stringstream &strInstString, long instruction );
			static void MSUBi ( stringstream &strInstString, long instruction );
			static void ADD ( stringstream &strInstString, long instruction );
			
			//static void ADDi ( stringstream &strInstString, long instruction );
			//static void ADDq ( stringstream &strInstString, long instruction );
			//static void ADDAi ( stringstream &strInstString, long instruction );
			//static void ADDAq ( stringstream &strInstString, long instruction );
			
			static void ADDABCX ( stringstream &strInstString, long instruction );
			static void ADDABCY ( stringstream &strInstString, long instruction );
			static void ADDABCZ ( stringstream &strInstString, long instruction );
			static void ADDABCW ( stringstream &strInstString, long instruction );
			
			static void MADD ( stringstream &strInstString, long instruction );
			
			static void MUL ( stringstream &strInstString, long instruction );
			static void MAX ( stringstream &strInstString, long instruction );
			static void SUB ( stringstream &strInstString, long instruction );
			static void MSUB ( stringstream &strInstString, long instruction );
			static void OPMSUM ( stringstream &strInstString, long instruction );
			static void MINI ( stringstream &strInstString, long instruction );
			static void IADD ( stringstream &strInstString, long instruction );
			static void ISUB ( stringstream &strInstString, long instruction );
			static void IADDI ( stringstream &strInstString, long instruction );
			static void IAND ( stringstream &strInstString, long instruction );
			static void IOR ( stringstream &strInstString, long instruction );
			//static void CALLMS ( stringstream &strInstString, long instruction );
			//static void CALLMSR ( stringstream &strInstString, long instruction );
			static void ITOF0 ( stringstream &strInstString, long instruction );
			static void FTOI0 ( stringstream &strInstString, long instruction );
			static void MULAq ( stringstream &strInstString, long instruction );
			static void ADDAq ( stringstream &strInstString, long instruction );
			static void SUBAq ( stringstream &strInstString, long instruction );
			static void ADDA ( stringstream &strInstString, long instruction );
			static void SUBA ( stringstream &strInstString, long instruction );
			static void MOVE ( stringstream &strInstString, long instruction );
			static void LQI ( stringstream &strInstString, long instruction );
			static void DIV ( stringstream &strInstString, long instruction );
			static void MTIR ( stringstream &strInstString, long instruction );
			//static void RNEXT ( stringstream &strInstString, long instruction );
			static void ITOF4 ( stringstream &strInstString, long instruction );
			static void FTOI4 ( stringstream &strInstString, long instruction );
			static void ABS ( stringstream &strInstString, long instruction );
			static void MADDAq ( stringstream &strInstString, long instruction );
			static void MSUBAq ( stringstream &strInstString, long instruction );
			static void MADDA ( stringstream &strInstString, long instruction );
			static void MSUBA ( stringstream &strInstString, long instruction );
			//static void MR32 ( stringstream &strInstString, long instruction );
			//static void SQI ( stringstream &strInstString, long instruction );
			//static void SQRT ( stringstream &strInstString, long instruction );
			//static void MFIR ( stringstream &strInstString, long instruction );
			//static void RGET ( stringstream &strInstString, long instruction );
			
			//static void ADDABCX ( stringstream &strInstString, long instruction );
			//static void ADDABCY ( stringstream &strInstString, long instruction );
			//static void ADDABCZ ( stringstream &strInstString, long instruction );
			//static void ADDABCW ( stringstream &strInstString, long instruction );
			
			static void SUBABCX ( stringstream &strInstString, long instruction );
			static void SUBABCY ( stringstream &strInstString, long instruction );
			static void SUBABCZ ( stringstream &strInstString, long instruction );
			static void SUBABCW ( stringstream &strInstString, long instruction );
			
			static void MADDABCX ( stringstream &strInstString, long instruction );
			static void MADDABCY ( stringstream &strInstString, long instruction );
			static void MADDABCZ ( stringstream &strInstString, long instruction );
			static void MADDABCW ( stringstream &strInstString, long instruction );
			
			static void MSUBABCX ( stringstream &strInstString, long instruction );
			static void MSUBABCY ( stringstream &strInstString, long instruction );
			static void MSUBABCZ ( stringstream &strInstString, long instruction );
			static void MSUBABCW ( stringstream &strInstString, long instruction );
			
			static void ITOF12 ( stringstream &strInstString, long instruction );
			static void FTOI12 ( stringstream &strInstString, long instruction );
			
			static void MULABCX ( stringstream &strInstString, long instruction );
			static void MULABCY ( stringstream &strInstString, long instruction );
			static void MULABCZ ( stringstream &strInstString, long instruction );
			static void MULABCW ( stringstream &strInstString, long instruction );
			
			static void MULAi ( stringstream &strInstString, long instruction );
			static void ADDAi ( stringstream &strInstString, long instruction );
			static void SUBAi ( stringstream &strInstString, long instruction );
			static void MULA ( stringstream &strInstString, long instruction );
			static void OPMULA ( stringstream &strInstString, long instruction );
			//static void LQD ( stringstream &strInstString, long instruction );
			//static void RSQRT ( stringstream &strInstString, long instruction );
			//static void ILWR ( stringstream &strInstString, long instruction );
			//static void RINIT ( stringstream &strInstString, long instruction );
			static void ITOF15 ( stringstream &strInstString, long instruction );
			static void FTOI15 ( stringstream &strInstString, long instruction );
			static void CLIP ( stringstream &strInstString, long instruction );
			static void MADDAi ( stringstream &strInstString, long instruction );
			static void MSUBAi ( stringstream &strInstString, long instruction );
			static void NOP ( stringstream &strInstString, long instruction );
			//static void SQD ( stringstream &strInstString, long instruction );


			// lower instructions

			
			static void LQ ( stringstream &strInstString, long instruction );
			static void SQ ( stringstream &strInstString, long instruction );
			static void ILW ( stringstream &strInstString, long instruction );
			static void ISW ( stringstream &strInstString, long instruction );
			static void IADDIU ( stringstream &strInstString, long instruction );
			static void ISUBIU ( stringstream &strInstString, long instruction );
			static void FCEQ ( stringstream &strInstString, long instruction );
			static void FCSET ( stringstream &strInstString, long instruction );
			static void FCAND ( stringstream &strInstString, long instruction );
			static void FCOR ( stringstream &strInstString, long instruction );
			static void FSEQ ( stringstream &strInstString, long instruction );
			static void FSSET ( stringstream &strInstString, long instruction );
			static void FSAND ( stringstream &strInstString, long instruction );
			static void FSOR ( stringstream &strInstString, long instruction );
			static void FMEQ ( stringstream &strInstString, long instruction );
			static void FMAND ( stringstream &strInstString, long instruction );
			static void FMOR ( stringstream &strInstString, long instruction );
			static void FCGET ( stringstream &strInstString, long instruction );
			static void B ( stringstream &strInstString, long instruction );
			static void BAL ( stringstream &strInstString, long instruction );
			static void JR ( stringstream &strInstString, long instruction );
			static void JALR ( stringstream &strInstString, long instruction );
			static void IBEQ ( stringstream &strInstString, long instruction );
			static void IBNE ( stringstream &strInstString, long instruction );
			static void IBLTZ ( stringstream &strInstString, long instruction );
			static void IBGTZ ( stringstream &strInstString, long instruction );
			static void IBLEZ ( stringstream &strInstString, long instruction );
			static void IBGEZ ( stringstream &strInstString, long instruction );
			
			//static void LowerOp ( stringstream &strInstString, long instruction );
			//static void Lower60 ( stringstream &strInstString, long instruction );
			//static void Lower61 ( stringstream &strInstString, long instruction );
			//static void Lower62 ( stringstream &strInstString, long instruction );
			//static void Lower63 ( stringstream &strInstString, long instruction );
			
			//static void DIV ( stringstream &strInstString, long instruction );
			//static void EATANxy ( stringstream &strInstString, long instruction );
			//static void EATANxz ( stringstream &strInstString, long instruction );
			//static void EATAN ( stringstream &strInstString, long instruction );
			//static void IADD ( stringstream &strInstString, long instruction );
			//static void ISUB ( stringstream &strInstString, long instruction );
			//static void IADDI ( stringstream &strInstString, long instruction );
			//static void IAND ( stringstream &strInstString, long instruction );
			//static void IOR ( stringstream &strInstString, long instruction );
			//static void MOVE ( stringstream &strInstString, long instruction );
			//static void LQI ( stringstream &strInstString, long instruction );
			//static void DIV ( stringstream &strInstString, long instruction );
			//static void MTIR ( stringstream &strInstString, long instruction );
			static void RNEXT ( stringstream &strInstString, long instruction );
			static void MFP ( stringstream &strInstString, long instruction );
			static void XTOP ( stringstream &strInstString, long instruction );
			static void XGKICK ( stringstream &strInstString, long instruction );

			static void MR32 ( stringstream &strInstString, long instruction );
			static void SQI ( stringstream &strInstString, long instruction );
			static void SQRT ( stringstream &strInstString, long instruction );
			static void MFIR ( stringstream &strInstString, long instruction );
			static void RGET ( stringstream &strInstString, long instruction );
			
			static void XITOP ( stringstream &strInstString, long instruction );
			static void ESADD ( stringstream &strInstString, long instruction );
			static void EATANxy ( stringstream &strInstString, long instruction );
			static void ESQRT ( stringstream &strInstString, long instruction );
			static void ESIN ( stringstream &strInstString, long instruction );
			static void ERSADD ( stringstream &strInstString, long instruction );
			static void EATANxz ( stringstream &strInstString, long instruction );
			static void ERSQRT ( stringstream &strInstString, long instruction );
			static void EATAN ( stringstream &strInstString, long instruction );
			static void LQD ( stringstream &strInstString, long instruction );
			static void RSQRT ( stringstream &strInstString, long instruction );
			static void ILWR ( stringstream &strInstString, long instruction );
			static void RINIT ( stringstream &strInstString, long instruction );
			static void ELENG ( stringstream &strInstString, long instruction );
			static void ESUM ( stringstream &strInstString, long instruction );
			static void ERCPR ( stringstream &strInstString, long instruction );
			static void EEXP ( stringstream &strInstString, long instruction );
			static void SQD ( stringstream &strInstString, long instruction );
			static void WAITQ ( stringstream &strInstString, long instruction );
			static void ISWR ( stringstream &strInstString, long instruction );
			static void RXOR ( stringstream &strInstString, long instruction );
			static void ERLENG ( stringstream &strInstString, long instruction );
			static void WAITP ( stringstream &strInstString, long instruction );
			
			
		};
		
	};
	
};


#endif


