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



#ifndef _R3000ADEBUGPRINT_H_
#define _R3000ADEBUGPRINT_H_

#include <sstream>
#include <string>


//#include "R3000A.h"
#include "R3000A_Instruction.h"
#include "R3000A_Lookup.h"
#include "types.h"

#include "MipsOpcode.h"

using namespace std;


namespace R3000A
{

	namespace Instruction
	{
	
		class Print
		{
		public:
			//using namespace std;

			typedef void (*Function) ( stringstream &s, long instruction );
			
			static const Function FunctionList [];

			// this gets the instruction text
			static string PrintInstruction ( long instruction );

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
			
			static void AddInstArgs ( stringstream &strMipsArgs, long Instruction, long InstFormat );
			static void AddVuDestArgs ( stringstream &strVuArgs, long Instruction );

		

			static void Invalid ( stringstream &s, long instruction );

			static void J ( stringstream &s, long instruction );
			static void JAL ( stringstream &s, long instruction );
			static void BEQ ( stringstream &s, long instruction );
			static void BNE ( stringstream &s, long instruction );
			static void BLEZ ( stringstream &s, long instruction );
			static void BGTZ ( stringstream &s, long instruction );
			static void ADDI ( stringstream &s, long instruction );
			static void ADDIU ( stringstream &s, long instruction );
			static void SLTI ( stringstream &s, long instruction );
			static void SLTIU ( stringstream &s, long instruction );
			static void ANDI ( stringstream &s, long instruction );
			static void ORI ( stringstream &s, long instruction );
			static void XORI ( stringstream &s, long instruction );
			static void LUI ( stringstream &s, long instruction );
			static void LB ( stringstream &s, long instruction );
			static void LH ( stringstream &s, long instruction );
			static void LWL ( stringstream &s, long instruction );
			static void LW ( stringstream &s, long instruction );
			static void LBU ( stringstream &s, long instruction );
			static void LHU ( stringstream &s, long instruction );
			
			static void LWR ( stringstream &s, long instruction );
			static void SB ( stringstream &s, long instruction );
			static void SH ( stringstream &s, long instruction );
			static void SWL ( stringstream &s, long instruction );
			static void SW ( stringstream &s, long instruction );
			static void SWR ( stringstream &s, long instruction );
			static void LWC2 ( stringstream &s, long instruction );
			static void SWC2 ( stringstream &s, long instruction );
			static void SLL ( stringstream &s, long instruction );
			static void SRL ( stringstream &s, long instruction );
			static void SRA ( stringstream &s, long instruction );
			static void SLLV ( stringstream &s, long instruction );
			static void SRLV ( stringstream &s, long instruction );
			static void SRAV ( stringstream &s, long instruction );
			static void JR ( stringstream &s, long instruction );
			static void JALR ( stringstream &s, long instruction );
			static void SYSCALL ( stringstream &s, long instruction );
			static void BREAK ( stringstream &s, long instruction );
			static void MFHI ( stringstream &s, long instruction );
			static void MTHI ( stringstream &s, long instruction );

			static void MFLO ( stringstream &s, long instruction );
			static void MTLO ( stringstream &s, long instruction );
			static void MULT ( stringstream &s, long instruction );
			static void MULTU ( stringstream &s, long instruction );
			static void DIV ( stringstream &s, long instruction );
			static void DIVU ( stringstream &s, long instruction );
			static void ADD ( stringstream &s, long instruction );
			static void ADDU ( stringstream &s, long instruction );
			static void SUB ( stringstream &s, long instruction );
			static void SUBU ( stringstream &s, long instruction );
			static void AND ( stringstream &s, long instruction );
			static void OR ( stringstream &s, long instruction );
			static void XOR ( stringstream &s, long instruction );
			static void NOR ( stringstream &s, long instruction );
			static void SLT ( stringstream &s, long instruction );
			static void SLTU ( stringstream &s, long instruction );
			static void BLTZ ( stringstream &s, long instruction );
			static void BGEZ ( stringstream &s, long instruction );
			static void BLTZAL ( stringstream &s, long instruction );
			static void BGEZAL ( stringstream &s, long instruction );

			static void MFC0 ( stringstream &s, long instruction );
			static void MTC0 ( stringstream &s, long instruction );
			static void RFE ( stringstream &s, long instruction );
			static void MFC2 ( stringstream &s, long instruction );
			static void CFC2 ( stringstream &s, long instruction );
			static void MTC2 ( stringstream &s, long instruction );
			static void CTC2 ( stringstream &s, long instruction );
			static void COP2 ( stringstream &s, long instruction );
			
			static void RTPS ( stringstream &s, long instruction );
			static void NCLIP ( stringstream &s, long instruction );
			static void OP ( stringstream &s, long instruction );
			static void DPCS ( stringstream &s, long instruction );
			static void INTPL ( stringstream &s, long instruction );
			static void MVMVA ( stringstream &s, long instruction );
			static void NCDS ( stringstream &s, long instruction );
			static void CDP ( stringstream &s, long instruction );
			static void NCDT ( stringstream &s, long instruction );
			static void NCCS ( stringstream &s, long instruction );
			static void CC ( stringstream &s, long instruction );
			static void NCS ( stringstream &s, long instruction );
			static void NCT ( stringstream &s, long instruction );
			static void SQR ( stringstream &s, long instruction );
			static void DCPL ( stringstream &s, long instruction );
			static void DPCT ( stringstream &s, long instruction );
			static void AVSZ3 ( stringstream &s, long instruction );
			static void AVSZ4 ( stringstream &s, long instruction );
			static void RTPT ( stringstream &s, long instruction );
			static void GPF ( stringstream &s, long instruction );
			static void GPL ( stringstream &s, long instruction );
			static void NCCT ( stringstream &s, long instruction );

		};
		
	}

/*
	void BC0 ( stringstream &strInstString, long instruction );
	void BC1 ( stringstream &strInstString, long instruction );
	void BC2 ( stringstream &strInstString, long instruction );
	void CO ( stringstream &strInstString, long instruction );
	void MF0 ( stringstream &strInstString, long instruction );
	void MT0 ( stringstream &strInstString, long instruction );
	void iEI ( stringstream &strInstString, long instruction );
	void iDI ( stringstream &strInstString, long instruction );
	void SD ( stringstream &strInstString, long instruction );
	void SW ( stringstream &strInstString, long instruction );
	void LW ( stringstream &strInstString, long instruction );
	void LD ( stringstream &strInstString, long instruction );
	void LHU ( stringstream &strInstString, long instruction );
	void LWR ( stringstream &strInstString, long instruction );
	void LWU ( stringstream &strInstString, long instruction );
	void SB ( stringstream &strInstString, long instruction );
	void SDL ( stringstream &strInstString, long instruction );
	void SDR ( stringstream &strInstString, long instruction );
	void SH ( stringstream &strInstString, long instruction );
	void SWL ( stringstream &strInstString, long instruction );
	void SWR ( stringstream &strInstString, long instruction );
	void LB ( stringstream &strInstString, long instruction );
	void LBU ( stringstream &strInstString, long instruction );
	void LDL ( stringstream &strInstString, long instruction );
	void LDR ( stringstream &strInstString, long instruction );
	void LH ( stringstream &strInstString, long instruction );
	void LWL ( stringstream &strInstString, long instruction );
	void LQ ( stringstream &strInstString, long instruction );
	void SQ ( stringstream &strInstString, long instruction );
	void SRL ( stringstream &strInstString, long instruction );

	void RFE ( stringstream &strInstString, long instruction );

	void Special ( stringstream &strInstString, long instruction );
	void Regimm ( stringstream &strInstString, long instruction );
	void J ( stringstream &strInstString, long instruction );
	void JAL ( stringstream &strInstString, long instruction );
	void BEQ ( stringstream &strInstString, long instruction );
	void BNE ( stringstream &strInstString, long instruction );
	void BLEZ ( stringstream &strInstString, long instruction );
	void BGTZ ( stringstream &strInstString, long instruction );
	void ADDI ( stringstream &strInstString, long instruction );
	void ADDIU ( stringstream &strInstString, long instruction );
	void SLTI ( stringstream &strInstString, long instruction );
	void SLTIU ( stringstream &strInstString, long instruction );
	void ANDI ( stringstream &strInstString, long instruction );
	void ORI ( stringstream &strInstString, long instruction );
	void XORI ( stringstream &strInstString, long instruction );
	void LUI ( stringstream &strInstString, long instruction );
	void COP0 ( stringstream &strInstString, long instruction );
	void COP2 ( stringstream &strInstString, long instruction );
	void None ( stringstream &strInstString, long instruction );
	void BEQL ( stringstream &strInstString, long instruction );
	void BNEL ( stringstream &strInstString, long instruction );
	void BLEZL ( stringstream &strInstString, long instruction );
	void BGTZL ( stringstream &strInstString, long instruction );
	void CACHE ( stringstream &strInstString, long instruction );
	void PREF ( stringstream &strInstString, long instruction );
	void LQC2 ( stringstream &strInstString, long instruction );
	void SQC2 ( stringstream &strInstString, long instruction );
	void BLTZ ( stringstream &strInstString, long instruction );
	void BGEZ ( stringstream &strInstString, long instruction );
	void BLTZL ( stringstream &strInstString, long instruction );
	void TGEI ( stringstream &strInstString, long instruction );
	void TGEIU ( stringstream &strInstString, long instruction );
	void TLTI ( stringstream &strInstString, long instruction );
	void TLTIU ( stringstream &strInstString, long instruction );
	void TEQI ( stringstream &strInstString, long instruction );
	void TNEI ( stringstream &strInstString, long instruction );
	void BLTZAL ( stringstream &strInstString, long instruction );
	void BGEZAL ( stringstream &strInstString, long instruction );
	void BLTZALL ( stringstream &strInstString, long instruction );
	void BGEZALL ( stringstream &strInstString, long instruction );
	void SLL ( stringstream &strInstString, long instruction );
	void MOVCI ( stringstream &strInstString, long instruction );
	void SRA ( stringstream &strInstString, long instruction );
	void SLLV ( stringstream &strInstString, long instruction );
	void SRLV ( stringstream &strInstString, long instruction );
	void SRAV ( stringstream &strInstString, long instruction );
	void JR ( stringstream &strInstString, long instruction );
	void JALR ( stringstream &strInstString, long instruction );
	void MOVZ ( stringstream &strInstString, long instruction );
	void MOVN ( stringstream &strInstString, long instruction );
	void SYSCALL ( stringstream &strInstString, long instruction );
	void BREAK ( stringstream &strInstString, long instruction );
	void SYNC ( stringstream &strInstString, long instruction );
	void MFHI ( stringstream &strInstString, long instruction );
	void MTHI ( stringstream &strInstString, long instruction );
	void MFLO ( stringstream &strInstString, long instruction );
	void MTLO ( stringstream &strInstString, long instruction );
	void MULT ( stringstream &strInstString, long instruction );
	void MULTU ( stringstream &strInstString, long instruction );
	void DIV ( stringstream &strInstString, long instruction );
	void DIVU ( stringstream &strInstString, long instruction );
	void ADD ( stringstream &strInstString, long instruction );
	void ADDU ( stringstream &strInstString, long instruction );
	void SUB ( stringstream &strInstString, long instruction );
	void SUBU ( stringstream &strInstString, long instruction );
	void AND ( stringstream &strInstString, long instruction );
	void OR ( stringstream &strInstString, long instruction );
	void XOR ( stringstream &strInstString, long instruction );
	void NOR ( stringstream &strInstString, long instruction );
	void MFSA ( stringstream &strInstString, long instruction );
	void MTSA ( stringstream &strInstString, long instruction );
	void SLT ( stringstream &strInstString, long instruction );
	void SLTU ( stringstream &strInstString, long instruction );
	void TGE ( stringstream &strInstString, long instruction );
	void TGEU ( stringstream &strInstString, long instruction );
	void TLT ( stringstream &strInstString, long instruction );
	void TLTU ( stringstream &strInstString, long instruction );
	void TEQ ( stringstream &strInstString, long instruction );
	void TNE ( stringstream &strInstString, long instruction );
	void TLBR ( stringstream &strInstString, long instruction );
	void TLBWI ( stringstream &strInstString, long instruction );
	void TLBWR ( stringstream &strInstString, long instruction );
	void TLBP ( stringstream &strInstString, long instruction );
	void ERET ( stringstream &strInstString, long instruction );
	void DERET ( stringstream &strInstString, long instruction );
	void WAIT ( stringstream &strInstString, long instruction );
	void MFC2 ( stringstream &strInstString, long instruction );
	void CFC2 ( stringstream &strInstString, long instruction );
	void CTC2 ( stringstream &strInstString, long instruction );
	void MTC2 ( stringstream &strInstString, long instruction );
	void LWC2 ( stringstream &strInstString, long instruction );
	void SWC2 ( stringstream &strInstString, long instruction );
	void Cop2BC ( stringstream &strInstString, long instruction );
	void S ( stringstream &strInstString, long instruction );
	void MFC2 ( stringstream &strInstString, long instruction );
	void MTC2 ( stringstream &strInstString, long instruction );

	const char XyzwLUT [ 4 ] = { 'x', 'y', 'z', 'w' };

	void AddVuDestArgs ( stringstream &strVuArgs, long Instruction );

	void AddInstArgs ( stringstream &strMipsArgs, long Instruction, long InstFormat );

	// to decode instruction, just look up the opcode here first, then call the function in the array which either formulates x64 code or looks up in other arrays
	// array complete
	const Function AllOpcodes [ 64 ] = { Special, Regimm, J, JAL, BEQ, BNE, BLEZ, BGTZ,	// 0-7
										ADDI, ADDIU, SLTI, SLTIU, ANDI, ORI, XORI, LUI,		// 8-15
										COP0, None, COP2, None, BEQL, BNEL, BLEZL, BGTZL,			// 16-23
										None, None, None, None, None, None, None, None,	// 24-31
										LB, LH, LWL, LW, LBU, LHU, LWR, LWU,	// 32-39
										SB, SH, SWL, SW, None, None, SWR, CACHE,	// 40-47
										None, None, LWC2, PREF, None, None, None, None,	//48-55
										None, None, SWC2, None, None, None, None, None };	//56-63
													
													
	// this is where you look up "REGIMM" codes in B2 slot
	// array complete
	const Function AllRegimmB2Codes [ 32 ] = { BLTZ, BGEZ, BLTZL, None, None, None, None, None, //0-7
													TGEI, TGEIU, TLTI, TLTIU, TEQI, None, TNEI, None, //8-15
													BLTZAL, BGEZAL, BLTZALL, BGEZALL, None, None, None, None, //16-23
													None, None, None, None, None, None, None, None }; //24-31

													

	// this one has only Playstation 2 special codes
	const Function AllPS2SpecialInsts [ 64 ] = {	SLL, MOVCI, SRL, SRA, SLLV, None, SRLV, SRAV, //0-7
													JR, JALR, MOVZ, MOVN, SYSCALL, BREAK, None, SYNC, //8-15
													MFHI, MTHI, MFLO, MTLO, None, None, None, None, //16-23
													MULT, MULTU, DIV, DIVU, None, None, None, None, //24-31
													ADD, ADDU, SUB, SUBU, AND, OR, XOR, NOR, //32-39
													MFSA, MTSA, SLT, SLTU, None, None, None, None, //40-47
													TGE, TGEU, TLT, TLTU, TEQ, None, TNE, None, //48-55
													None, None, None, None, None, None, None, None }; //56-63







	// this is where you look up "COP0" instructions - lookup using "Special" slot
	// array complete
	const Function AllCop0Insts [ 64 ] = { None, TLBR, TLBWI, None, None, None, TLBWR, None, //0-7
													TLBP, None, None, None, None, None, None, None, //8-15
													None, None, None, None, None, None, None, None, //16-23
													ERET, None, None, None, None, None, None, None, //24-31
													None, None, None, None, None, None, None, None, //32-39
													None, None, None, None, None, None, None, None, //40-47
													None, None, None, None, None, None, None, None, //48-55
													None, None, None, None, None, None, None, None }; //56-63
													
	const Function AllCop0FmtCodes [ 32 ] = { MF0, None, None, None, MT0, None, None, None, //0-7
													BC0, None, None, None, None, None, None, None, //8-15
													CO, CO, CO, CO, CO, CO, CO, CO, //16-23
													CO, CO, CO, CO, CO, CO, CO, CO }; //24-31

	const Function AllC0Insts [ 64 ] = { None, TLBR, TLBWI, None, None, None, TLBWR, None, //0-7
													TLBP, None, None, None, None, None, None, None, //8-15
													RFE, None, None, None, None, None, None, None, //16-23
													ERET, None, None, None, None, None, None, None, //24-31
													None, None, None, None, None, None, None, None, //32-39
													None, None, None, None, None, None, None, None, //40-47
													None, None, None, None, None, None, None, None, //48-55
													iEI, iDI, None, None, None, None, None, None }; //56-63

													

	const Function AllCop2FmtCodes [ 32 ] = { MFC2, None, CFC2, None, MTC2, None, CTC2, None, //0-7
													BC2, None, None, None, None, None, None, None, //8-15
													None, None, None, None, None, None, None, None, //16-23
													None, None, None, None, None, None, None, None }; //24-31








	// LUTs for VU Macro mode instructions
													
*/

}



#endif


