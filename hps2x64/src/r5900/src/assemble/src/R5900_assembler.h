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

#ifndef _R5900_ASSEMBLER_

#define _R5900_ASSEMBLER_


#ifdef INLINE_DEBUG
#include "Debug.h"
#endif



#include "TokenCollection.h"

#include "MipsOpcode.h"

#include "types.h"

#include <fstream>
#include <algorithm>


#include <string>
#include <sstream>
#include <vector>

using namespace std;

namespace R5900
{

	namespace Utilities
	{
	
		static const char* CommentString = ";";
		static const char* RegisterString = "%";
		static const char* ImmedString = "$";
		static const char* LabelString = ">";

		// convert string into number returning false on error
		template <class T>
		bool from_string (T& t, 
						 const std::string& s, 
						 std::ios_base& (*f)(std::ios_base&))
		{
		  std::istringstream iss(s);
		  return !(iss >> f >> t).fail();
		}

	
		class Assembler
		{
		
#ifdef INLINE_DEBUG
			static Debug::Log debug;
#endif
			
		public:
		
			ofstream* OutputFile;
			ifstream* InputFile;
			
			typedef bool (*Function) ( long &Code, string sInst, long CurrentAddress );

			static const Function FunctionList [];
			
			//static R5900::Instruction::Entry<Function> Entries [];
			
			class LabelEntry
			{
			public:
				LabelEntry ( string _Name, u32 _Address ) { Name = _Name; Address = _Address; }
			
				string Name;
				long Address;
			};
			
			static vector<LabelEntry*> Labels;
			
			// constructor
			Assembler ();
			
			// destructor
			~Assembler ();
			
			bool SetInputFile ( string InputFileName );
			bool SetOutputFile ( string OutputFileName );
			
			void Pass1 ();
			
			void Pass2 ();

		//private:
			// check if a line in source file has an instruction, label, etc.
			static int hasInstruction ( string Line );
			static bool hasLabel ( string Line );
			static bool hasComment ( string Line );
			static bool hasDirective ( string Line );

			// get instruction parts
			static bool GetRegister ( long& Result, string Line, long Instance );
			static bool GetImmed ( long& Result, string Line, long Instance );
			static bool GetLabel ( long& Result, string Line, long Instance );
			
			// removes parts of line
			
			// comments will start with a semicolon
			static string removeComment ( string Line );
			
			// labels will end with a colon
			static string removeLabel ( string Line );
			
			inline static bool Format1 ( long &rt, long &rs, long &immediate, string sInst );
			inline static bool Format2 ( long &rd, long &rs, long &rt, string sInst );
			inline static bool Format3 ( long &rs, long &rt, string sInst );
			inline static bool Format4 ( long &rs, long &immediate, string sInst );
			inline static bool Format5 ( long &rs, string sInst );

			inline static bool Format19 ( long &offset, string sInst );
			
			static bool Invalid ( u32& Code, string sInst, long CurrentAddress );
			
			static bool ADDIU ( u32& Code, string sInst, long CurrentAddress );
			static bool ANDI ( u32& Code, string sInst, long CurrentAddress );
			static bool ORI ( u32& Code, string sInst, long CurrentAddress );
			static bool SLTI ( u32& Code, string sInst, long CurrentAddress );
			static bool SLTIU ( u32& Code, string sInst, long CurrentAddress );
			static bool XORI ( u32& Code, string sInst, long CurrentAddress );

			static bool ADDI ( u32& Code, string sInst, long CurrentAddress );

			static bool ADDU ( u32& Code, string sInst, long CurrentAddress );
			static bool AND ( u32& Code, string sInst, long CurrentAddress );
			static bool OR ( u32& Code, string sInst, long CurrentAddress );
			static bool NOR ( u32& Code, string sInst, long CurrentAddress );
			static bool SLT ( u32& Code, string sInst, long CurrentAddress );
			static bool SLTU ( u32& Code, string sInst, long CurrentAddress );
			static bool SUBU ( u32& Code, string sInst, long CurrentAddress );
			static bool XOR ( u32& Code, string sInst, long CurrentAddress );
			
			static bool ADD ( u32& Code, string sInst, long CurrentAddress );
			static bool SUB ( u32& Code, string sInst, long CurrentAddress );

			static bool DIV ( u32& Code, string sInst, long CurrentAddress );
			static bool DIVU ( u32& Code, string sInst, long CurrentAddress );
			
			static bool MULT ( u32& Code, string sInst, long CurrentAddress );
			static bool MULTU ( u32& Code, string sInst, long CurrentAddress );
			
			static bool SLL ( u32& Code, string sInst, long CurrentAddress );
			static bool SRA ( u32& Code, string sInst, long CurrentAddress );
			static bool SRL ( u32& Code, string sInst, long CurrentAddress );
			
			static bool SLLV ( u32& Code, string sInst, long CurrentAddress );
			static bool SRAV ( u32& Code, string sInst, long CurrentAddress );
			static bool SRLV ( u32& Code, string sInst, long CurrentAddress );
			
			static bool J ( u32& Code, string sInst, long CurrentAddress );
			static bool JR ( u32& Code, string sInst, long CurrentAddress );
			static bool JAL ( u32& Code, string sInst, long CurrentAddress );
			static bool JALR ( u32& Code, string sInst, long CurrentAddress );
		
			static bool BEQ ( u32& Code, string sInst, long CurrentAddress );
			static bool BNE ( u32& Code, string sInst, long CurrentAddress );
			static bool BGEZ ( u32& Code, string sInst, long CurrentAddress );
			static bool BGTZ ( u32& Code, string sInst, long CurrentAddress );
			static bool BLEZ ( u32& Code, string sInst, long CurrentAddress );
			static bool BLTZ ( u32& Code, string sInst, long CurrentAddress );

			static bool BGEZAL ( u32& Code, string sInst, long CurrentAddress );
			static bool BLTZAL ( u32& Code, string sInst, long CurrentAddress );
			
			static bool LUI ( u32& Code, string sInst, long CurrentAddress );

			static bool MFHI ( u32& Code, string sInst, long CurrentAddress );
			static bool MFLO ( u32& Code, string sInst, long CurrentAddress );
			static bool MTHI ( u32& Code, string sInst, long CurrentAddress );
			static bool MTLO ( u32& Code, string sInst, long CurrentAddress );

			static bool SYSCALL ( u32& Code, string sInst, long CurrentAddress );
			static bool RFE ( u32& Code, string sInst, long CurrentAddress );
			static bool BREAK ( u32& Code, string sInst, long CurrentAddress );

			static bool MFC0 ( u32& Code, string sInst, long CurrentAddress );
			static bool MTC0 ( u32& Code, string sInst, long CurrentAddress );
			static bool MFC2 ( u32& Code, string sInst, long CurrentAddress );
			static bool MTC2 ( u32& Code, string sInst, long CurrentAddress );
			static bool CFC2 ( u32& Code, string sInst, long CurrentAddress );
			static bool CTC2 ( u32& Code, string sInst, long CurrentAddress );
			
			//static bool COP2 ( u32& Code, string sInst, long CurrentAddress );
			
			static bool LB ( u32& Code, string sInst, long CurrentAddress );
			static bool LH ( u32& Code, string sInst, long CurrentAddress );
			static bool LWL ( u32& Code, string sInst, long CurrentAddress );
			static bool LW ( u32& Code, string sInst, long CurrentAddress );
			static bool LBU ( u32& Code, string sInst, long CurrentAddress );
			static bool LHU ( u32& Code, string sInst, long CurrentAddress );
			static bool LWR ( u32& Code, string sInst, long CurrentAddress );
			static bool SB ( u32& Code, string sInst, long CurrentAddress );
			static bool SH ( u32& Code, string sInst, long CurrentAddress );
			static bool SWL ( u32& Code, string sInst, long CurrentAddress );
			static bool SW ( u32& Code, string sInst, long CurrentAddress );
			static bool SWR ( u32& Code, string sInst, long CurrentAddress );
			
			//static bool LWC2 ( u32& Code, string sInst, long CurrentAddress );
			//static bool SWC2 ( u32& Code, string sInst, long CurrentAddress );
			
			
			// *** R5900 Instructions *** //
			
			static bool BC0T ( u32& Code, string sInst, long CurrentAddress );
			static bool BC0TL ( u32& Code, string sInst, long CurrentAddress );
			static bool BC0F ( u32& Code, string sInst, long CurrentAddress );
			static bool BC0FL ( u32& Code, string sInst, long CurrentAddress );
			static bool BC1T ( u32& Code, string sInst, long CurrentAddress );
			static bool BC1TL ( u32& Code, string sInst, long CurrentAddress );
			static bool BC1F ( u32& Code, string sInst, long CurrentAddress );
			static bool BC1FL ( u32& Code, string sInst, long CurrentAddress );
			static bool BC2T ( u32& Code, string sInst, long CurrentAddress );
			static bool BC2TL ( u32& Code, string sInst, long CurrentAddress );
			static bool BC2F ( u32& Code, string sInst, long CurrentAddress );
			static bool BC2FL ( u32& Code, string sInst, long CurrentAddress );
			

			static bool CFC0 ( u32& Code, string sInst, long CurrentAddress );
			static bool CTC0 ( u32& Code, string sInst, long CurrentAddress );
			static bool EI ( u32& Code, string sInst, long CurrentAddress );
			static bool DI ( u32& Code, string sInst, long CurrentAddress );
			
			static bool SD ( u32& Code, string sInst, long CurrentAddress );
			static bool LD ( u32& Code, string sInst, long CurrentAddress );
			static bool LWU ( u32& Code, string sInst, long CurrentAddress );
			static bool SDL ( u32& Code, string sInst, long CurrentAddress );
			static bool SDR ( u32& Code, string sInst, long CurrentAddress );
			static bool LDL ( u32& Code, string sInst, long CurrentAddress );
			static bool LDR ( u32& Code, string sInst, long CurrentAddress );
			static bool LQ ( u32& Code, string sInst, long CurrentAddress );
			static bool SQ ( u32& Code, string sInst, long CurrentAddress );
			
			
			// arithemetic instructions //
			static bool DADD ( u32& Code, string sInst, long CurrentAddress );
			static bool DADDI ( u32& Code, string sInst, long CurrentAddress );
			static bool DADDU ( u32& Code, string sInst, long CurrentAddress );
			static bool DADDIU ( u32& Code, string sInst, long CurrentAddress );
			static bool DSUB ( u32& Code, string sInst, long CurrentAddress );
			static bool DSUBU ( u32& Code, string sInst, long CurrentAddress );
			static bool DSLL ( u32& Code, string sInst, long CurrentAddress );
			static bool DSLL32 ( u32& Code, string sInst, long CurrentAddress );
			static bool DSLLV ( u32& Code, string sInst, long CurrentAddress );
			static bool DSRA ( u32& Code, string sInst, long CurrentAddress );
			static bool DSRA32 ( u32& Code, string sInst, long CurrentAddress );
			static bool DSRAV ( u32& Code, string sInst, long CurrentAddress );
			static bool DSRL ( u32& Code, string sInst, long CurrentAddress );
			static bool DSRL32 ( u32& Code, string sInst, long CurrentAddress );
			static bool DSRLV ( u32& Code, string sInst, long CurrentAddress );
			
			

			static bool MFC1 ( u32& Code, string sInst, long CurrentAddress );
			static bool CFC1 ( u32& Code, string sInst, long CurrentAddress );
			static bool MTC1 ( u32& Code, string sInst, long CurrentAddress );
			static bool CTC1 ( u32& Code, string sInst, long CurrentAddress );
			
			static bool BEQL ( u32& Code, string sInst, long CurrentAddress );
			static bool BNEL ( u32& Code, string sInst, long CurrentAddress );
			static bool BGEZL ( u32& Code, string sInst, long CurrentAddress );
			static bool BLEZL ( u32& Code, string sInst, long CurrentAddress );
			static bool BGTZL ( u32& Code, string sInst, long CurrentAddress );
			static bool BLTZL ( u32& Code, string sInst, long CurrentAddress );
			static bool BLTZALL ( u32& Code, string sInst, long CurrentAddress );
			static bool BGEZALL ( u32& Code, string sInst, long CurrentAddress );
			
			static bool CACHE ( u32& Code, string sInst, long CurrentAddress );
			static bool PREF ( u32& Code, string sInst, long CurrentAddress );
			
			static bool TGEI ( u32& Code, string sInst, long CurrentAddress );
			static bool TGEIU ( u32& Code, string sInst, long CurrentAddress );
			static bool TLTI ( u32& Code, string sInst, long CurrentAddress );
			static bool TLTIU ( u32& Code, string sInst, long CurrentAddress );
			static bool TEQI ( u32& Code, string sInst, long CurrentAddress );
			static bool TNEI ( u32& Code, string sInst, long CurrentAddress );
			static bool TGE ( u32& Code, string sInst, long CurrentAddress );
			static bool TGEU ( u32& Code, string sInst, long CurrentAddress );
			static bool TLT ( u32& Code, string sInst, long CurrentAddress );
			static bool TLTU ( u32& Code, string sInst, long CurrentAddress );
			static bool TEQ ( u32& Code, string sInst, long CurrentAddress );
			static bool TNE ( u32& Code, string sInst, long CurrentAddress );
			
			static bool MOVCI ( u32& Code, string sInst, long CurrentAddress );
			static bool MOVZ ( u32& Code, string sInst, long CurrentAddress );
			static bool MOVN ( u32& Code, string sInst, long CurrentAddress );
			static bool SYNC ( u32& Code, string sInst, long CurrentAddress );
			
			static bool MFHI1 ( u32& Code, string sInst, long CurrentAddress );
			static bool MTHI1 ( u32& Code, string sInst, long CurrentAddress );
			static bool MFLO1 ( u32& Code, string sInst, long CurrentAddress );
			static bool MTLO1 ( u32& Code, string sInst, long CurrentAddress );
			static bool MULT1 ( u32& Code, string sInst, long CurrentAddress );
			static bool MULTU1 ( u32& Code, string sInst, long CurrentAddress );
			static bool DIV1 ( u32& Code, string sInst, long CurrentAddress );
			static bool DIVU1 ( u32& Code, string sInst, long CurrentAddress );
			static bool MADD ( u32& Code, string sInst, long CurrentAddress );
			static bool MADD1 ( u32& Code, string sInst, long CurrentAddress );
			static bool MADDU ( u32& Code, string sInst, long CurrentAddress );
			static bool MADDU1 ( u32& Code, string sInst, long CurrentAddress );
			
			static bool MFSA ( u32& Code, string sInst, long CurrentAddress );
			static bool MTSA ( u32& Code, string sInst, long CurrentAddress );
			static bool MTSAB ( u32& Code, string sInst, long CurrentAddress );
			static bool MTSAH ( u32& Code, string sInst, long CurrentAddress );
			
			static bool TLBR ( u32& Code, string sInst, long CurrentAddress );
			static bool TLBWI ( u32& Code, string sInst, long CurrentAddress );
			static bool TLBWR ( u32& Code, string sInst, long CurrentAddress );
			static bool TLBP ( u32& Code, string sInst, long CurrentAddress );
			
			static bool ERET ( u32& Code, string sInst, long CurrentAddress );
			static bool DERET ( u32& Code, string sInst, long CurrentAddress );
			static bool WAIT ( u32& Code, string sInst, long CurrentAddress );
			
			
			// Parallel instructions (SIMD) //
			static bool PABSH ( u32& Code, string sInst, long CurrentAddress );
			static bool PABSW ( u32& Code, string sInst, long CurrentAddress );
			static bool PADDB ( u32& Code, string sInst, long CurrentAddress );
			static bool PADDH ( u32& Code, string sInst, long CurrentAddress );
			static bool PADDW ( u32& Code, string sInst, long CurrentAddress );
			static bool PADDSB ( u32& Code, string sInst, long CurrentAddress );
			static bool PADDSH ( u32& Code, string sInst, long CurrentAddress );
			static bool PADDSW ( u32& Code, string sInst, long CurrentAddress );
			static bool PADDUB ( u32& Code, string sInst, long CurrentAddress );
			static bool PADDUH ( u32& Code, string sInst, long CurrentAddress );
			static bool PADDUW ( u32& Code, string sInst, long CurrentAddress );
			static bool PADSBH ( u32& Code, string sInst, long CurrentAddress );
			static bool PAND ( u32& Code, string sInst, long CurrentAddress );
			static bool POR ( u32& Code, string sInst, long CurrentAddress );
			static bool PXOR ( u32& Code, string sInst, long CurrentAddress );
			static bool PNOR ( u32& Code, string sInst, long CurrentAddress );
			static bool PCEQB ( u32& Code, string sInst, long CurrentAddress );
			static bool PCEQH ( u32& Code, string sInst, long CurrentAddress );
			static bool PCEQW ( u32& Code, string sInst, long CurrentAddress );
			static bool PCGTB ( u32& Code, string sInst, long CurrentAddress );
			static bool PCGTH ( u32& Code, string sInst, long CurrentAddress );
			static bool PCGTW ( u32& Code, string sInst, long CurrentAddress );
			static bool PCPYH ( u32& Code, string sInst, long CurrentAddress );
			static bool PCPYLD ( u32& Code, string sInst, long CurrentAddress );
			static bool PCPYUD ( u32& Code, string sInst, long CurrentAddress );
			static bool PDIVBW ( u32& Code, string sInst, long CurrentAddress );
			static bool PDIVUW ( u32& Code, string sInst, long CurrentAddress );
			static bool PDIVW ( u32& Code, string sInst, long CurrentAddress );
			static bool PEXCH ( u32& Code, string sInst, long CurrentAddress );
			static bool PEXCW ( u32& Code, string sInst, long CurrentAddress );
			static bool PEXEH ( u32& Code, string sInst, long CurrentAddress );
			static bool PEXEW ( u32& Code, string sInst, long CurrentAddress );
			static bool PEXT5 ( u32& Code, string sInst, long CurrentAddress );
			static bool PEXTLB ( u32& Code, string sInst, long CurrentAddress );
			static bool PEXTLH ( u32& Code, string sInst, long CurrentAddress );
			static bool PEXTLW ( u32& Code, string sInst, long CurrentAddress );
			static bool PEXTUB ( u32& Code, string sInst, long CurrentAddress );
			static bool PEXTUH ( u32& Code, string sInst, long CurrentAddress );
			static bool PEXTUW ( u32& Code, string sInst, long CurrentAddress );
			static bool PHMADH ( u32& Code, string sInst, long CurrentAddress );
			static bool PHMSBH ( u32& Code, string sInst, long CurrentAddress );
			static bool PINTEH ( u32& Code, string sInst, long CurrentAddress );
			static bool PINTH ( u32& Code, string sInst, long CurrentAddress );
			static bool PLZCW ( u32& Code, string sInst, long CurrentAddress );
			static bool PMADDH ( u32& Code, string sInst, long CurrentAddress );
			static bool PMADDW ( u32& Code, string sInst, long CurrentAddress );
			static bool PMADDUW ( u32& Code, string sInst, long CurrentAddress );
			static bool PMAXH ( u32& Code, string sInst, long CurrentAddress );
			static bool PMAXW ( u32& Code, string sInst, long CurrentAddress );
			static bool PMINH ( u32& Code, string sInst, long CurrentAddress );
			static bool PMINW ( u32& Code, string sInst, long CurrentAddress );
			static bool PMFHI ( u32& Code, string sInst, long CurrentAddress );
			static bool PMFLO ( u32& Code, string sInst, long CurrentAddress );
			static bool PMTHI ( u32& Code, string sInst, long CurrentAddress );
			static bool PMTLO ( u32& Code, string sInst, long CurrentAddress );
			static bool PMFHL_LH ( u32& Code, string sInst, long CurrentAddress );
			static bool PMFHL_SH ( u32& Code, string sInst, long CurrentAddress );
			static bool PMFHL_LW ( u32& Code, string sInst, long CurrentAddress );
			static bool PMFHL_UW ( u32& Code, string sInst, long CurrentAddress );
			static bool PMFHL_SLW ( u32& Code, string sInst, long CurrentAddress );
			static bool PMTHL_LW ( u32& Code, string sInst, long CurrentAddress );
			static bool PMSUBH ( u32& Code, string sInst, long CurrentAddress );
			static bool PMSUBW ( u32& Code, string sInst, long CurrentAddress );
			static bool PMULTH ( u32& Code, string sInst, long CurrentAddress );
			static bool PMULTW ( u32& Code, string sInst, long CurrentAddress );
			static bool PMULTUW ( u32& Code, string sInst, long CurrentAddress );
			static bool PPAC5 ( u32& Code, string sInst, long CurrentAddress );
			static bool PPACB ( u32& Code, string sInst, long CurrentAddress );
			static bool PPACH ( u32& Code, string sInst, long CurrentAddress );
			static bool PPACW ( u32& Code, string sInst, long CurrentAddress );
			static bool PREVH ( u32& Code, string sInst, long CurrentAddress );
			static bool PROT3W ( u32& Code, string sInst, long CurrentAddress );
			static bool PSLLH ( u32& Code, string sInst, long CurrentAddress );
			static bool PSLLVW ( u32& Code, string sInst, long CurrentAddress );
			static bool PSLLW ( u32& Code, string sInst, long CurrentAddress );
			static bool PSRAH ( u32& Code, string sInst, long CurrentAddress );
			static bool PSRAW ( u32& Code, string sInst, long CurrentAddress );
			static bool PSRAVW ( u32& Code, string sInst, long CurrentAddress );
			static bool PSRLH ( u32& Code, string sInst, long CurrentAddress );
			static bool PSRLW ( u32& Code, string sInst, long CurrentAddress );
			static bool PSRLVW ( u32& Code, string sInst, long CurrentAddress );
			static bool PSUBB ( u32& Code, string sInst, long CurrentAddress );
			static bool PSUBH ( u32& Code, string sInst, long CurrentAddress );
			static bool PSUBW ( u32& Code, string sInst, long CurrentAddress );
			static bool PSUBSB ( u32& Code, string sInst, long CurrentAddress );
			static bool PSUBSH ( u32& Code, string sInst, long CurrentAddress );
			static bool PSUBSW ( u32& Code, string sInst, long CurrentAddress );
			static bool PSUBUB ( u32& Code, string sInst, long CurrentAddress );
			static bool PSUBUH ( u32& Code, string sInst, long CurrentAddress );
			static bool PSUBUW ( u32& Code, string sInst, long CurrentAddress );
			static bool QFSRV ( u32& Code, string sInst, long CurrentAddress );
			

			// floating point instructions //

			static bool LWC1 ( u32& Code, string sInst, long CurrentAddress );
			static bool SWC1 ( u32& Code, string sInst, long CurrentAddress );
			
			static bool ABS_S ( u32& Code, string sInst, long CurrentAddress );
			static bool ADD_S ( u32& Code, string sInst, long CurrentAddress );
			static bool ADDA_S ( u32& Code, string sInst, long CurrentAddress );
			static bool C_EQ_S ( u32& Code, string sInst, long CurrentAddress );
			static bool C_F_S ( u32& Code, string sInst, long CurrentAddress );
			static bool C_LE_S ( u32& Code, string sInst, long CurrentAddress );
			static bool C_LT_S ( u32& Code, string sInst, long CurrentAddress );
			static bool CVT_S_W ( u32& Code, string sInst, long CurrentAddress );
			static bool CVT_W_S ( u32& Code, string sInst, long CurrentAddress );
			static bool DIV_S ( u32& Code, string sInst, long CurrentAddress );
			static bool MADD_S ( u32& Code, string sInst, long CurrentAddress );
			static bool MADDA_S ( u32& Code, string sInst, long CurrentAddress );
			static bool MAX_S ( u32& Code, string sInst, long CurrentAddress );
			static bool MIN_S ( u32& Code, string sInst, long CurrentAddress );
			static bool MOV_S ( u32& Code, string sInst, long CurrentAddress );
			static bool MSUB_S ( u32& Code, string sInst, long CurrentAddress );
			static bool MSUBA_S ( u32& Code, string sInst, long CurrentAddress );
			static bool MUL_S ( u32& Code, string sInst, long CurrentAddress );
			static bool MULA_S ( u32& Code, string sInst, long CurrentAddress );
			static bool NEG_S ( u32& Code, string sInst, long CurrentAddress );
			static bool RSQRT_S ( u32& Code, string sInst, long CurrentAddress );
			static bool SQRT_S ( u32& Code, string sInst, long CurrentAddress );
			static bool SUB_S ( u32& Code, string sInst, long CurrentAddress );
			static bool SUBA_S ( u32& Code, string sInst, long CurrentAddress );
			

			// PS2 COP2 instructions //

			static bool LQC2 ( u32& Code, string sInst, long CurrentAddress );
			static bool SQC2 ( u32& Code, string sInst, long CurrentAddress );
			static bool QMFC2_NI ( u32& Code, string sInst, long CurrentAddress );
			static bool QMTC2_NI ( u32& Code, string sInst, long CurrentAddress );
			static bool QMFC2_I ( u32& Code, string sInst, long CurrentAddress );
			static bool QMTC2_I ( u32& Code, string sInst, long CurrentAddress );
			
			
			static bool COP2 ( u32& Code, string sInst, long CurrentAddress );
			
			static bool VADDBCX ( u32& Code, string sInst, long CurrentAddress );
			static bool VADDBCY ( u32& Code, string sInst, long CurrentAddress );
			static bool VADDBCZ ( u32& Code, string sInst, long CurrentAddress );
			static bool VADDBCW ( u32& Code, string sInst, long CurrentAddress );
			static bool VSUBBCX ( u32& Code, string sInst, long CurrentAddress );
			static bool VSUBBCY ( u32& Code, string sInst, long CurrentAddress );
			static bool VSUBBCZ ( u32& Code, string sInst, long CurrentAddress );
			static bool VSUBBCW ( u32& Code, string sInst, long CurrentAddress );
			static bool VMULBCX ( u32& Code, string sInst, long CurrentAddress );
			static bool VMULBCY ( u32& Code, string sInst, long CurrentAddress );
			static bool VMULBCZ ( u32& Code, string sInst, long CurrentAddress );
			static bool VMULBCW ( u32& Code, string sInst, long CurrentAddress );
			static bool VMADDBCX ( u32& Code, string sInst, long CurrentAddress );
			static bool VMADDBCY ( u32& Code, string sInst, long CurrentAddress );
			static bool VMADDBCZ ( u32& Code, string sInst, long CurrentAddress );
			static bool VMADDBCW ( u32& Code, string sInst, long CurrentAddress );
			static bool VMSUBBCX ( u32& Code, string sInst, long CurrentAddress );
			static bool VMSUBBCY ( u32& Code, string sInst, long CurrentAddress );
			static bool VMSUBBCZ ( u32& Code, string sInst, long CurrentAddress );
			static bool VMSUBBCW ( u32& Code, string sInst, long CurrentAddress );
			static bool VMAXBCX ( u32& Code, string sInst, long CurrentAddress );
			static bool VMAXBCY ( u32& Code, string sInst, long CurrentAddress );
			static bool VMAXBCZ ( u32& Code, string sInst, long CurrentAddress );
			static bool VMAXBCW ( u32& Code, string sInst, long CurrentAddress );
			static bool VMINIBCX ( u32& Code, string sInst, long CurrentAddress );
			static bool VMINIBCY ( u32& Code, string sInst, long CurrentAddress );
			static bool VMINIBCZ ( u32& Code, string sInst, long CurrentAddress );
			static bool VMINIBCW ( u32& Code, string sInst, long CurrentAddress );
			
			static bool VADD ( u32& Code, string sInst, long CurrentAddress );
			static bool VADDI ( u32& Code, string sInst, long CurrentAddress );
			static bool VADDQ ( u32& Code, string sInst, long CurrentAddress );
			static bool VSUB ( u32& Code, string sInst, long CurrentAddress );
			static bool VSUBI ( u32& Code, string sInst, long CurrentAddress );
			static bool VSUBQ ( u32& Code, string sInst, long CurrentAddress );
			static bool VMUL ( u32& Code, string sInst, long CurrentAddress );
			static bool VMULI ( u32& Code, string sInst, long CurrentAddress );
			static bool VMULQ ( u32& Code, string sInst, long CurrentAddress );
			static bool VMAX ( u32& Code, string sInst, long CurrentAddress );
			static bool VMAXI ( u32& Code, string sInst, long CurrentAddress );
			static bool VMINI ( u32& Code, string sInst, long CurrentAddress );
			static bool VMINII ( u32& Code, string sInst, long CurrentAddress );
			static bool VMADD ( u32& Code, string sInst, long CurrentAddress );
			static bool VMADDI ( u32& Code, string sInst, long CurrentAddress );
			static bool VMADDQ ( u32& Code, string sInst, long CurrentAddress );
			static bool VMSUB ( u32& Code, string sInst, long CurrentAddress );
			static bool VMSUBI ( u32& Code, string sInst, long CurrentAddress );
			static bool VMSUBQ ( u32& Code, string sInst, long CurrentAddress );
			static bool VDIV ( u32& Code, string sInst, long CurrentAddress );
			
			static bool VADDA ( u32& Code, string sInst, long CurrentAddress );
			static bool VADDAI ( u32& Code, string sInst, long CurrentAddress );
			static bool VADDAQ ( u32& Code, string sInst, long CurrentAddress );
			static bool VSUBA ( u32& Code, string sInst, long CurrentAddress );
			static bool VADDABC ( u32& Code, string sInst, long CurrentAddress );
			static bool VSUBAI ( u32& Code, string sInst, long CurrentAddress );
			static bool VSUBAQ ( u32& Code, string sInst, long CurrentAddress );
			static bool VSUBABC ( u32& Code, string sInst, long CurrentAddress );
			static bool VMULA ( u32& Code, string sInst, long CurrentAddress );
			static bool VMULAI ( u32& Code, string sInst, long CurrentAddress );
			static bool VMULAQ ( u32& Code, string sInst, long CurrentAddress );
			static bool VMULABC ( u32& Code, string sInst, long CurrentAddress );
			static bool VMADDA ( u32& Code, string sInst, long CurrentAddress );
			static bool VMADDAI ( u32& Code, string sInst, long CurrentAddress );
			static bool VMADDAQ ( u32& Code, string sInst, long CurrentAddress );
			static bool VMADDABC ( u32& Code, string sInst, long CurrentAddress );
			static bool VMSUBA ( u32& Code, string sInst, long CurrentAddress );
			static bool VMSUBAI ( u32& Code, string sInst, long CurrentAddress );
			static bool VMSUBAQ ( u32& Code, string sInst, long CurrentAddress );
			static bool VMSUBABC ( u32& Code, string sInst, long CurrentAddress );
			
			static bool VOPMULA ( u32& Code, string sInst, long CurrentAddress );
			static bool VOPMSUM ( u32& Code, string sInst, long CurrentAddress );
			static bool VOPMSUB ( u32& Code, string sInst, long CurrentAddress );

			static bool VNOP ( u32& Code, string sInst, long CurrentAddress );
			static bool VABS ( u32& Code, string sInst, long CurrentAddress );
			static bool VCLIP ( u32& Code, string sInst, long CurrentAddress );
			static bool VSQRT ( u32& Code, string sInst, long CurrentAddress );
			static bool VRSQRT ( u32& Code, string sInst, long CurrentAddress );
			static bool VMR32 ( u32& Code, string sInst, long CurrentAddress );
			static bool VRINIT ( u32& Code, string sInst, long CurrentAddress );
			static bool VRGET ( u32& Code, string sInst, long CurrentAddress );
			static bool VRNEXT ( u32& Code, string sInst, long CurrentAddress );
			static bool VRXOR ( u32& Code, string sInst, long CurrentAddress );
			static bool VMOVE ( u32& Code, string sInst, long CurrentAddress );
			static bool VMFIR ( u32& Code, string sInst, long CurrentAddress );
			static bool VMTIR ( u32& Code, string sInst, long CurrentAddress );
			static bool VLQD ( u32& Code, string sInst, long CurrentAddress );
			static bool VLQI ( u32& Code, string sInst, long CurrentAddress );
			static bool VSQD ( u32& Code, string sInst, long CurrentAddress );
			static bool VSQI ( u32& Code, string sInst, long CurrentAddress );
			static bool VWAITQ ( u32& Code, string sInst, long CurrentAddress );
			
			static bool VFTOI0 ( u32& Code, string sInst, long CurrentAddress );
			static bool VITOF0 ( u32& Code, string sInst, long CurrentAddress );
			static bool VFTOI4 ( u32& Code, string sInst, long CurrentAddress );
			static bool VITOF4 ( u32& Code, string sInst, long CurrentAddress );
			static bool VFTOI12 ( u32& Code, string sInst, long CurrentAddress );
			static bool VITOF12 ( u32& Code, string sInst, long CurrentAddress );
			static bool VFTOI15 ( u32& Code, string sInst, long CurrentAddress );
			static bool VITOF15 ( u32& Code, string sInst, long CurrentAddress );
			
			static bool VIADD ( u32& Code, string sInst, long CurrentAddress );
			static bool VISUB ( u32& Code, string sInst, long CurrentAddress );
			static bool VIADDI ( u32& Code, string sInst, long CurrentAddress );
			static bool VIAND ( u32& Code, string sInst, long CurrentAddress );
			static bool VIOR ( u32& Code, string sInst, long CurrentAddress );
			static bool VILWR ( u32& Code, string sInst, long CurrentAddress );
			static bool VISWR ( u32& Code, string sInst, long CurrentAddress );
			static bool VCALLMS ( u32& Code, string sInst, long CurrentAddress );
			static bool VCALLMSR ( u32& Code, string sInst, long CurrentAddress );
			
		};
		
	}
}

#endif


