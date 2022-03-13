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



#ifndef _R5900DEBUGPRINT_H_
#define _R5900DEBUGPRINT_H_

#include <sstream>
#include <string>


#include "types.h"
#include "MipsOpcode.h"
#include "R5900_Instruction.h"
#include "R5900_Lookup.h"


using namespace std;


namespace R5900
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
			
			static string PrintInstruction ( long instruction );	// { FunctionList [ Lookup::FindByInstruction ( instruction ) ] ( s, instruction ); }
			
			
			
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

		

			static void Invalid ( stringstream &s, long instruction );
			

			// *** R3000A Instructions *** //
			
			static void ADD ( stringstream &s, long instruction );
			static void ADDU ( stringstream &s, long instruction );
			static void SUB ( stringstream &s, long instruction );
			static void SUBU ( stringstream &s, long instruction );
			static void AND ( stringstream &s, long instruction );
			static void OR ( stringstream &s, long instruction );
			static void XOR ( stringstream &s, long instruction );
			static void NOR ( stringstream &s, long instruction );
			static void MULT ( stringstream &s, long instruction );
			static void MULTU ( stringstream &s, long instruction );
			static void DIV ( stringstream &s, long instruction );
			static void DIVU ( stringstream &s, long instruction );
			static void SLT ( stringstream &s, long instruction );
			
			static void ADDI ( stringstream &s, long instruction );
			static void ADDIU ( stringstream &s, long instruction );
			static void ANDI ( stringstream &s, long instruction );
			static void ORI ( stringstream &s, long instruction );
			static void XORI ( stringstream &s, long instruction );
			
			static void SLL ( stringstream &s, long instruction );
			static void SRL ( stringstream &s, long instruction );
			static void SRA ( stringstream &s, long instruction );
			static void SLLV ( stringstream &s, long instruction );
			static void SRLV ( stringstream &s, long instruction );
			static void SRAV ( stringstream &s, long instruction );
			
			static void SLTU ( stringstream &s, long instruction );
			static void SLTI ( stringstream &s, long instruction );
			static void SLTIU ( stringstream &s, long instruction );
			
			static void J ( stringstream &s, long instruction );
			static void JAL ( stringstream &s, long instruction );
			static void BEQ ( stringstream &s, long instruction );
			static void BNE ( stringstream &s, long instruction );
			static void BLEZ ( stringstream &s, long instruction );
			static void BGTZ ( stringstream &s, long instruction );
			static void JR ( stringstream &s, long instruction );
			static void JALR ( stringstream &s, long instruction );
			static void BLTZ ( stringstream &s, long instruction );
			static void BGEZ ( stringstream &s, long instruction );
			static void BLTZAL ( stringstream &s, long instruction );
			static void BGEZAL ( stringstream &s, long instruction );
			
			static void LUI ( stringstream &s, long instruction );
			static void MFHI ( stringstream &s, long instruction );
			static void MTHI ( stringstream &s, long instruction );
			static void MFLO ( stringstream &s, long instruction );
			static void MTLO ( stringstream &s, long instruction );

			static void MFC0 ( stringstream &s, long instruction );
			static void MTC0 ( stringstream &s, long instruction );
			
			static void CFC2_I ( stringstream &s, long instruction );
			static void CTC2_I ( stringstream &s, long instruction );
			static void CFC2_NI ( stringstream &s, long instruction );
			static void CTC2_NI ( stringstream &s, long instruction );
			
			static void LB ( stringstream &s, long instruction );
			static void LH ( stringstream &s, long instruction );
			static void LW ( stringstream &s, long instruction );
			static void LBU ( stringstream &s, long instruction );
			static void LHU ( stringstream &s, long instruction );
			static void LWL ( stringstream &s, long instruction );
			static void LWR ( stringstream &s, long instruction );
			static void SB ( stringstream &s, long instruction );
			static void SH ( stringstream &s, long instruction );
			static void SW ( stringstream &s, long instruction );
			static void SWL ( stringstream &s, long instruction );
			static void SWR ( stringstream &s, long instruction );
			
			static void SYSCALL ( stringstream &s, long instruction );
			static void BREAK ( stringstream &s, long instruction );
			
			static void COP2 ( stringstream &s, long instruction );
			
			
			// these instructions are R3000A only //
			/*
			static void RFE ( stringstream &s, long instruction );
			static void LWC2 ( stringstream &s, long instruction );
			static void SWC2 ( stringstream &s, long instruction );
			static void MFC2 ( stringstream &s, long instruction );
			static void MTC2 ( stringstream &s, long instruction );
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
			*/
			
			
			// *** R5900 Instructions *** //
			
			static void BC0T ( stringstream &strInstString, long instruction );
			static void BC0TL ( stringstream &strInstString, long instruction );
			static void BC0F ( stringstream &strInstString, long instruction );
			static void BC0FL ( stringstream &strInstString, long instruction );
			static void BC1T ( stringstream &strInstString, long instruction );
			static void BC1TL ( stringstream &strInstString, long instruction );
			static void BC1F ( stringstream &strInstString, long instruction );
			static void BC1FL ( stringstream &strInstString, long instruction );
			static void BC2T ( stringstream &strInstString, long instruction );
			static void BC2TL ( stringstream &strInstString, long instruction );
			static void BC2F ( stringstream &strInstString, long instruction );
			static void BC2FL ( stringstream &strInstString, long instruction );
			

			static void CFC0 ( stringstream &strInstString, long instruction );
			static void CTC0 ( stringstream &strInstString, long instruction );
			static void EI ( stringstream &strInstString, long instruction );
			static void DI ( stringstream &strInstString, long instruction );
			
			static void SD ( stringstream &strInstString, long instruction );
			static void LD ( stringstream &strInstString, long instruction );
			static void LWU ( stringstream &strInstString, long instruction );
			static void SDL ( stringstream &strInstString, long instruction );
			static void SDR ( stringstream &strInstString, long instruction );
			static void LDL ( stringstream &strInstString, long instruction );
			static void LDR ( stringstream &strInstString, long instruction );
			static void LQ ( stringstream &strInstString, long instruction );
			static void SQ ( stringstream &strInstString, long instruction );
			
			
			// arithemetic instructions //
			static void DADD ( stringstream &strInstString, long instruction );
			static void DADDI ( stringstream &strInstString, long instruction );
			static void DADDU ( stringstream &strInstString, long instruction );
			static void DADDIU ( stringstream &strInstString, long instruction );
			static void DSUB ( stringstream &strInstString, long instruction );
			static void DSUBU ( stringstream &strInstString, long instruction );
			static void DSLL ( stringstream &strInstString, long instruction );
			static void DSLL32 ( stringstream &strInstString, long instruction );
			static void DSLLV ( stringstream &strInstString, long instruction );
			static void DSRA ( stringstream &strInstString, long instruction );
			static void DSRA32 ( stringstream &strInstString, long instruction );
			static void DSRAV ( stringstream &strInstString, long instruction );
			static void DSRL ( stringstream &strInstString, long instruction );
			static void DSRL32 ( stringstream &strInstString, long instruction );
			static void DSRLV ( stringstream &strInstString, long instruction );
			
			

			static void MFC1 ( stringstream &s, long instruction );
			static void CFC1 ( stringstream &s, long instruction );
			static void MTC1 ( stringstream &s, long instruction );
			static void CTC1 ( stringstream &s, long instruction );
			
			static void BEQL ( stringstream &strInstString, long instruction );
			static void BNEL ( stringstream &strInstString, long instruction );
			static void BGEZL ( stringstream &strInstString, long instruction );
			static void BLEZL ( stringstream &strInstString, long instruction );
			static void BGTZL ( stringstream &strInstString, long instruction );
			static void BLTZL ( stringstream &strInstString, long instruction );
			static void BLTZALL ( stringstream &strInstString, long instruction );
			static void BGEZALL ( stringstream &strInstString, long instruction );
			
			static void CACHE ( stringstream &strInstString, long instruction );
			static void PREF ( stringstream &strInstString, long instruction );
			
			static void TGEI ( stringstream &strInstString, long instruction );
			static void TGEIU ( stringstream &strInstString, long instruction );
			static void TLTI ( stringstream &strInstString, long instruction );
			static void TLTIU ( stringstream &strInstString, long instruction );
			static void TEQI ( stringstream &strInstString, long instruction );
			static void TNEI ( stringstream &strInstString, long instruction );
			static void TGE ( stringstream &strInstString, long instruction );
			static void TGEU ( stringstream &strInstString, long instruction );
			static void TLT ( stringstream &strInstString, long instruction );
			static void TLTU ( stringstream &strInstString, long instruction );
			static void TEQ ( stringstream &strInstString, long instruction );
			static void TNE ( stringstream &strInstString, long instruction );
			
			static void MOVCI ( stringstream &strInstString, long instruction );
			static void MOVZ ( stringstream &strInstString, long instruction );
			static void MOVN ( stringstream &strInstString, long instruction );
			static void SYNC ( stringstream &strInstString, long instruction );
			
			static void MFHI1 ( stringstream &strInstString, long instruction );
			static void MTHI1 ( stringstream &strInstString, long instruction );
			static void MFLO1 ( stringstream &strInstString, long instruction );
			static void MTLO1 ( stringstream &strInstString, long instruction );
			static void MULT1 ( stringstream &strInstString, long instruction );
			static void MULTU1 ( stringstream &strInstString, long instruction );
			static void DIV1 ( stringstream &strInstString, long instruction );
			static void DIVU1 ( stringstream &strInstString, long instruction );
			static void MADD ( stringstream &strInstString, long instruction );
			static void MADD1 ( stringstream &strInstString, long instruction );
			static void MADDU ( stringstream &strInstString, long instruction );
			static void MADDU1 ( stringstream &strInstString, long instruction );
			
			static void MFSA ( stringstream &strInstString, long instruction );
			static void MTSA ( stringstream &strInstString, long instruction );
			static void MTSAB ( stringstream &strInstString, long instruction );
			static void MTSAH ( stringstream &strInstString, long instruction );
			
			static void TLBR ( stringstream &strInstString, long instruction );
			static void TLBWI ( stringstream &strInstString, long instruction );
			static void TLBWR ( stringstream &strInstString, long instruction );
			static void TLBP ( stringstream &strInstString, long instruction );
			
			static void ERET ( stringstream &strInstString, long instruction );
			static void DERET ( stringstream &strInstString, long instruction );
			static void WAIT ( stringstream &strInstString, long instruction );
			
			
			// Parallel instructions (SIMD) //
			static void PABSH ( stringstream &strInstString, long instruction );
			static void PABSW ( stringstream &strInstString, long instruction );
			static void PADDB ( stringstream &strInstString, long instruction );
			static void PADDH ( stringstream &strInstString, long instruction );
			static void PADDW ( stringstream &strInstString, long instruction );
			static void PADDSB ( stringstream &strInstString, long instruction );
			static void PADDSH ( stringstream &strInstString, long instruction );
			static void PADDSW ( stringstream &strInstString, long instruction );
			static void PADDUB ( stringstream &strInstString, long instruction );
			static void PADDUH ( stringstream &strInstString, long instruction );
			static void PADDUW ( stringstream &strInstString, long instruction );
			static void PADSBH ( stringstream &strInstString, long instruction );
			static void PAND ( stringstream &strInstString, long instruction );
			static void POR ( stringstream &strInstString, long instruction );
			static void PXOR ( stringstream &strInstString, long instruction );
			static void PNOR ( stringstream &strInstString, long instruction );
			static void PCEQB ( stringstream &strInstString, long instruction );
			static void PCEQH ( stringstream &strInstString, long instruction );
			static void PCEQW ( stringstream &strInstString, long instruction );
			static void PCGTB ( stringstream &strInstString, long instruction );
			static void PCGTH ( stringstream &strInstString, long instruction );
			static void PCGTW ( stringstream &strInstString, long instruction );
			static void PCPYH ( stringstream &strInstString, long instruction );
			static void PCPYLD ( stringstream &strInstString, long instruction );
			static void PCPYUD ( stringstream &strInstString, long instruction );
			static void PDIVBW ( stringstream &strInstString, long instruction );
			static void PDIVUW ( stringstream &strInstString, long instruction );
			static void PDIVW ( stringstream &strInstString, long instruction );
			static void PEXCH ( stringstream &strInstString, long instruction );
			static void PEXCW ( stringstream &strInstString, long instruction );
			static void PEXEH ( stringstream &strInstString, long instruction );
			static void PEXEW ( stringstream &strInstString, long instruction );
			static void PEXT5 ( stringstream &strInstString, long instruction );
			static void PEXTLB ( stringstream &strInstString, long instruction );
			static void PEXTLH ( stringstream &strInstString, long instruction );
			static void PEXTLW ( stringstream &strInstString, long instruction );
			static void PEXTUB ( stringstream &strInstString, long instruction );
			static void PEXTUH ( stringstream &strInstString, long instruction );
			static void PEXTUW ( stringstream &strInstString, long instruction );
			static void PHMADH ( stringstream &strInstString, long instruction );
			static void PHMSBH ( stringstream &strInstString, long instruction );
			static void PINTEH ( stringstream &strInstString, long instruction );
			static void PINTH ( stringstream &strInstString, long instruction );
			static void PLZCW ( stringstream &strInstString, long instruction );
			static void PMADDH ( stringstream &strInstString, long instruction );
			static void PMADDW ( stringstream &strInstString, long instruction );
			static void PMADDUW ( stringstream &strInstString, long instruction );
			static void PMAXH ( stringstream &strInstString, long instruction );
			static void PMAXW ( stringstream &strInstString, long instruction );
			static void PMINH ( stringstream &strInstString, long instruction );
			static void PMINW ( stringstream &strInstString, long instruction );
			static void PMFHI ( stringstream &strInstString, long instruction );
			static void PMFLO ( stringstream &strInstString, long instruction );
			static void PMTHI ( stringstream &strInstString, long instruction );
			static void PMTLO ( stringstream &strInstString, long instruction );
			static void PMFHL_LH ( stringstream &strInstString, long instruction );
			static void PMFHL_SH ( stringstream &strInstString, long instruction );
			static void PMFHL_LW ( stringstream &strInstString, long instruction );
			static void PMFHL_UW ( stringstream &strInstString, long instruction );
			static void PMFHL_SLW ( stringstream &strInstString, long instruction );
			static void PMTHL_LW ( stringstream &strInstString, long instruction );
			static void PMSUBH ( stringstream &strInstString, long instruction );
			static void PMSUBW ( stringstream &strInstString, long instruction );
			static void PMULTH ( stringstream &strInstString, long instruction );
			static void PMULTW ( stringstream &strInstString, long instruction );
			static void PMULTUW ( stringstream &strInstString, long instruction );
			static void PPAC5 ( stringstream &strInstString, long instruction );
			static void PPACB ( stringstream &strInstString, long instruction );
			static void PPACH ( stringstream &strInstString, long instruction );
			static void PPACW ( stringstream &strInstString, long instruction );
			static void PREVH ( stringstream &strInstString, long instruction );
			static void PROT3W ( stringstream &strInstString, long instruction );
			static void PSLLH ( stringstream &strInstString, long instruction );
			static void PSLLVW ( stringstream &strInstString, long instruction );
			static void PSLLW ( stringstream &strInstString, long instruction );
			static void PSRAH ( stringstream &strInstString, long instruction );
			static void PSRAW ( stringstream &strInstString, long instruction );
			static void PSRAVW ( stringstream &strInstString, long instruction );
			static void PSRLH ( stringstream &strInstString, long instruction );
			static void PSRLW ( stringstream &strInstString, long instruction );
			static void PSRLVW ( stringstream &strInstString, long instruction );
			static void PSUBB ( stringstream &strInstString, long instruction );
			static void PSUBH ( stringstream &strInstString, long instruction );
			static void PSUBW ( stringstream &strInstString, long instruction );
			static void PSUBSB ( stringstream &strInstString, long instruction );
			static void PSUBSH ( stringstream &strInstString, long instruction );
			static void PSUBSW ( stringstream &strInstString, long instruction );
			static void PSUBUB ( stringstream &strInstString, long instruction );
			static void PSUBUH ( stringstream &strInstString, long instruction );
			static void PSUBUW ( stringstream &strInstString, long instruction );
			static void QFSRV ( stringstream &strInstString, long instruction );
			

			// floating point instructions //

			static void LWC1 ( stringstream &s, long instruction );
			static void SWC1 ( stringstream &s, long instruction );
			
			static void ABS_S ( stringstream &strInstString, long instruction );
			static void ADD_S ( stringstream &strInstString, long instruction );
			static void ADDA_S ( stringstream &strInstString, long instruction );
			static void C_EQ_S ( stringstream &strInstString, long instruction );
			static void C_F_S ( stringstream &strInstString, long instruction );
			static void C_LE_S ( stringstream &strInstString, long instruction );
			static void C_LT_S ( stringstream &strInstString, long instruction );
			static void CVT_S_W ( stringstream &strInstString, long instruction );
			static void CVT_W_S ( stringstream &strInstString, long instruction );
			static void DIV_S ( stringstream &strInstString, long instruction );
			static void MADD_S ( stringstream &strInstString, long instruction );
			static void MADDA_S ( stringstream &strInstString, long instruction );
			static void MAX_S ( stringstream &strInstString, long instruction );
			static void MIN_S ( stringstream &strInstString, long instruction );
			static void MOV_S ( stringstream &strInstString, long instruction );
			static void MSUB_S ( stringstream &strInstString, long instruction );
			static void MSUBA_S ( stringstream &strInstString, long instruction );
			static void MUL_S ( stringstream &strInstString, long instruction );
			static void MULA_S ( stringstream &strInstString, long instruction );
			static void NEG_S ( stringstream &strInstString, long instruction );
			static void RSQRT_S ( stringstream &strInstString, long instruction );
			static void SQRT_S ( stringstream &strInstString, long instruction );
			static void SUB_S ( stringstream &strInstString, long instruction );
			static void SUBA_S ( stringstream &strInstString, long instruction );
			

			// PS2 COP2 instructions //

			static void LQC2 ( stringstream &strInstString, long instruction );
			static void SQC2 ( stringstream &strInstString, long instruction );
			static void QMFC2_NI ( stringstream &strInstString, long instruction );
			static void QMTC2_NI ( stringstream &strInstString, long instruction );
			static void QMFC2_I ( stringstream &strInstString, long instruction );
			static void QMTC2_I ( stringstream &strInstString, long instruction );
			
			
			static void VADD ( stringstream &strInstString, long instruction );
			static void VADDi ( stringstream &strInstString, long instruction );
			static void VADDq ( stringstream &strInstString, long instruction );
			static void VADDBCX ( stringstream &strInstString, long instruction );
			static void VADDBCY ( stringstream &strInstString, long instruction );
			static void VADDBCZ ( stringstream &strInstString, long instruction );
			static void VADDBCW ( stringstream &strInstString, long instruction );
			
			static void VSUB ( stringstream &strInstString, long instruction );
			static void VSUBi ( stringstream &strInstString, long instruction );
			static void VSUBq ( stringstream &strInstString, long instruction );
			static void VSUBBCX ( stringstream &strInstString, long instruction );
			static void VSUBBCY ( stringstream &strInstString, long instruction );
			static void VSUBBCZ ( stringstream &strInstString, long instruction );
			static void VSUBBCW ( stringstream &strInstString, long instruction );
			
			static void VMUL ( stringstream &strInstString, long instruction );
			static void VMULi ( stringstream &strInstString, long instruction );
			static void VMULq ( stringstream &strInstString, long instruction );
			static void VMULBCX ( stringstream &strInstString, long instruction );
			static void VMULBCY ( stringstream &strInstString, long instruction );
			static void VMULBCZ ( stringstream &strInstString, long instruction );
			static void VMULBCW ( stringstream &strInstString, long instruction );
			
			static void VMADD ( stringstream &strInstString, long instruction );
			static void VMADDi ( stringstream &strInstString, long instruction );
			static void VMADDq ( stringstream &strInstString, long instruction );
			static void VMADDBCX ( stringstream &strInstString, long instruction );
			static void VMADDBCY ( stringstream &strInstString, long instruction );
			static void VMADDBCZ ( stringstream &strInstString, long instruction );
			static void VMADDBCW ( stringstream &strInstString, long instruction );
			
			static void VMSUB ( stringstream &strInstString, long instruction );
			static void VMSUBi ( stringstream &strInstString, long instruction );
			static void VMSUBq ( stringstream &strInstString, long instruction );
			static void VMSUBBCX ( stringstream &strInstString, long instruction );
			static void VMSUBBCY ( stringstream &strInstString, long instruction );
			static void VMSUBBCZ ( stringstream &strInstString, long instruction );
			static void VMSUBBCW ( stringstream &strInstString, long instruction );
			
			static void VMAX ( stringstream &strInstString, long instruction );
			static void VMAXi ( stringstream &strInstString, long instruction );
			static void VMAXBCX ( stringstream &strInstString, long instruction );
			static void VMAXBCY ( stringstream &strInstString, long instruction );
			static void VMAXBCZ ( stringstream &strInstString, long instruction );
			static void VMAXBCW ( stringstream &strInstString, long instruction );
			
			static void VMINI ( stringstream &strInstString, long instruction );
			static void VMINIi ( stringstream &strInstString, long instruction );
			static void VMINIBCX ( stringstream &strInstString, long instruction );
			static void VMINIBCY ( stringstream &strInstString, long instruction );
			static void VMINIBCZ ( stringstream &strInstString, long instruction );
			static void VMINIBCW ( stringstream &strInstString, long instruction );
			
			static void VDIV ( stringstream &strInstString, long instruction );
			
			static void VADDA ( stringstream &strInstString, long instruction );
			static void VADDAi ( stringstream &strInstString, long instruction );
			static void VADDAq ( stringstream &strInstString, long instruction );
			static void VADDABCX ( stringstream &strInstString, long instruction );
			static void VADDABCY ( stringstream &strInstString, long instruction );
			static void VADDABCZ ( stringstream &strInstString, long instruction );
			static void VADDABCW ( stringstream &strInstString, long instruction );
			
			static void VSUBA ( stringstream &strInstString, long instruction );
			static void VSUBAi ( stringstream &strInstString, long instruction );
			static void VSUBAq ( stringstream &strInstString, long instruction );
			static void VSUBABCX ( stringstream &strInstString, long instruction );
			static void VSUBABCY ( stringstream &strInstString, long instruction );
			static void VSUBABCZ ( stringstream &strInstString, long instruction );
			static void VSUBABCW ( stringstream &strInstString, long instruction );
			
			static void VMULA ( stringstream &strInstString, long instruction );
			static void VMULAi ( stringstream &strInstString, long instruction );
			static void VMULAq ( stringstream &strInstString, long instruction );
			static void VMULABCX ( stringstream &strInstString, long instruction );
			static void VMULABCY ( stringstream &strInstString, long instruction );
			static void VMULABCZ ( stringstream &strInstString, long instruction );
			static void VMULABCW ( stringstream &strInstString, long instruction );
			
			static void VMADDA ( stringstream &strInstString, long instruction );
			static void VMADDAi ( stringstream &strInstString, long instruction );
			static void VMADDAq ( stringstream &strInstString, long instruction );
			static void VMADDABCX ( stringstream &strInstString, long instruction );
			static void VMADDABCY ( stringstream &strInstString, long instruction );
			static void VMADDABCZ ( stringstream &strInstString, long instruction );
			static void VMADDABCW ( stringstream &strInstString, long instruction );
			
			static void VMSUBA ( stringstream &strInstString, long instruction );
			static void VMSUBAi ( stringstream &strInstString, long instruction );
			static void VMSUBAq ( stringstream &strInstString, long instruction );
			static void VMSUBABCX ( stringstream &strInstString, long instruction );
			static void VMSUBABCY ( stringstream &strInstString, long instruction );
			static void VMSUBABCZ ( stringstream &strInstString, long instruction );
			static void VMSUBABCW ( stringstream &strInstString, long instruction );
			
			static void VOPMULA ( stringstream &strInstString, long instruction );
			static void VOPMSUB ( stringstream &strInstString, long instruction );

			static void VNOP ( stringstream &strInstString, long instruction );
			static void VABS ( stringstream &strInstString, long instruction );
			static void VCLIP ( stringstream &strInstString, long instruction );
			static void VSQRT ( stringstream &strInstString, long instruction );
			static void VRSQRT ( stringstream &strInstString, long instruction );
			static void VMR32 ( stringstream &strInstString, long instruction );
			static void VRINIT ( stringstream &strInstString, long instruction );
			static void VRGET ( stringstream &strInstString, long instruction );
			static void VRNEXT ( stringstream &strInstString, long instruction );
			static void VRXOR ( stringstream &strInstString, long instruction );
			static void VMOVE ( stringstream &strInstString, long instruction );
			static void VMFIR ( stringstream &strInstString, long instruction );
			static void VMTIR ( stringstream &strInstString, long instruction );
			static void VLQD ( stringstream &strInstString, long instruction );
			static void VLQI ( stringstream &strInstString, long instruction );
			static void VSQD ( stringstream &strInstString, long instruction );
			static void VSQI ( stringstream &strInstString, long instruction );
			static void VWAITQ ( stringstream &strInstString, long instruction );
			
			static void VFTOI0 ( stringstream &strInstString, long instruction );
			static void VFTOI4 ( stringstream &strInstString, long instruction );
			static void VFTOI12 ( stringstream &strInstString, long instruction );
			static void VFTOI15 ( stringstream &strInstString, long instruction );
			
			static void VITOF0 ( stringstream &strInstString, long instruction );
			static void VITOF4 ( stringstream &strInstString, long instruction );
			static void VITOF12 ( stringstream &strInstString, long instruction );
			static void VITOF15 ( stringstream &strInstString, long instruction );
			
			static void VIADD ( stringstream &strInstString, long instruction );
			static void VISUB ( stringstream &strInstString, long instruction );
			static void VIADDI ( stringstream &strInstString, long instruction );
			static void VIAND ( stringstream &strInstString, long instruction );
			static void VIOR ( stringstream &strInstString, long instruction );
			static void VILWR ( stringstream &strInstString, long instruction );
			static void VISWR ( stringstream &strInstString, long instruction );
			static void VCALLMS ( stringstream &strInstString, long instruction );
			static void VCALLMSR ( stringstream &strInstString, long instruction );
			

			
		};
		
	}



	/*
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


