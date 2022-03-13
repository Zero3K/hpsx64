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


#ifndef _R5900_EXECUTE_H_

#define _R5900_EXECUTE_H_

#include "R5900.h"
#include "R5900_Instruction.h"
#include "R5900_Lookup.h"
#include "types.h"


namespace R5900
{

	class Cpu;

	namespace Instruction
	{
	
		class Execute
		{
		public:
		
			static Cpu *r;
		
			// constructor - creates lookup table from list of entries
			Execute ( Cpu* pCpu );
			
			static void Start ();
		
			// returns true if the instruction has executed its last cycle
			// r - pointer to the Cpu object to execute the instruction on
			// i - the instruction to execute
			// CycleToExecute - the cycle to execute for the instruction. To execute first cycle, this should be zero
			typedef void (*Function) ( Instruction::Format i );
			
			//static Entry<Function> Entries [];
			//static Entry<Function> BranchLoadEntries1 [];	// this is modified to include stuff for branch/load delay slots
			
			//static const int c_iOpcode_MaxValue = 64;
			//static const int c_iRs_MaxValue = 32;
			//static const int c_iRt_MaxValue = 32;
			//static const int c_iShiftAmount_MaxValue = 32;
			//static const int c_iFunct_MaxValue = 64;

			// this should come out to 2^27, or rather 134217728
			//static const u32 c_iLookupTable_Size = c_iOpcode_MaxValue * c_iRs_MaxValue * c_iRt_MaxValue * c_iShiftAmount_MaxValue * c_iFunct_MaxValue;
			//static const u32 c_iLookupTable_Mask = c_iLookupTable_Size - 1;
			
			// this actually has to be a 4D array, but I'll make it one array			
			// use Opcode, Rs, Rt, and Funct to lookup value
			//static u16 LookupTable [ c_iLookupTable_Size ];

			static const Function FunctionList [];
			
			inline static void ExecuteInstruction ( Instruction::Format i ) { FunctionList [ Lookup::FindByInstruction ( i.Value ) ] ( i ); }

			
			static u64* handle_cached_load ( u32 LoadAddress, u32 BaseRegister );
			static void handle_uncached_load ( u32 BaseRegister );
			static u64* handle_cached_store ( u32 StoreAddress );
			static void handle_uncached_store ();
			static u64* handle_cached_load_blocking ( u32 LoadAddress );
			static void handle_uncached_load_blocking ();
			static u64* handle_cached_store_blocking ( u32 StoreAddress );
			static void handle_uncached_store_blocking ();


		
			static void SignalSynchronousInterrupt ( u32 ExceptionType );

			static void Invalid ( Instruction::Format i );
			
			
			// *** R3000A Instructions *** //
			
			static void ADD ( Instruction::Format i );
			static void ADDU ( Instruction::Format i );
			static void SUB ( Instruction::Format i );
			static void SUBU ( Instruction::Format i );
			static void AND ( Instruction::Format i );
			static void OR ( Instruction::Format i );
			static void XOR ( Instruction::Format i );
			static void NOR ( Instruction::Format i );
			static void MULT ( Instruction::Format i );
			static void MULTU ( Instruction::Format i );
			static void DIV ( Instruction::Format i );
			static void DIVU ( Instruction::Format i );
			static void SLT ( Instruction::Format i );
			
			static void ADDI ( Instruction::Format i );
			static void ADDIU ( Instruction::Format i );
			static void ANDI ( Instruction::Format i );
			static void ORI ( Instruction::Format i );
			static void XORI ( Instruction::Format i );
			
			static void SLL ( Instruction::Format i );
			static void SRL ( Instruction::Format i );
			static void SRA ( Instruction::Format i );
			static void SLLV ( Instruction::Format i );
			static void SRLV ( Instruction::Format i );
			static void SRAV ( Instruction::Format i );
			
			static void SLTU ( Instruction::Format i );
			static void SLTI ( Instruction::Format i );
			static void SLTIU ( Instruction::Format i );
			
			static void J ( Instruction::Format i );
			static void JAL ( Instruction::Format i );
			static void BEQ ( Instruction::Format i );
			static void BNE ( Instruction::Format i );
			static void BLEZ ( Instruction::Format i );
			static void BGTZ ( Instruction::Format i );
			static void JR ( Instruction::Format i );
			static void JALR ( Instruction::Format i );
			static void BLTZ ( Instruction::Format i );
			static void BGEZ ( Instruction::Format i );
			static void BLTZAL ( Instruction::Format i );
			static void BGEZAL ( Instruction::Format i );
			
			static void LUI ( Instruction::Format i );
			static void MFHI ( Instruction::Format i );
			static void MTHI ( Instruction::Format i );
			static void MFLO ( Instruction::Format i );
			static void MTLO ( Instruction::Format i );

			static void MFC0 ( Instruction::Format i );
			static void MTC0 ( Instruction::Format i );
			
			static void CFC2_I ( Instruction::Format i );
			static void CTC2_I ( Instruction::Format i );
			static void CFC2_NI ( Instruction::Format i );
			static void CTC2_NI ( Instruction::Format i );
			
			static void LB ( Instruction::Format i );
			static void LH ( Instruction::Format i );
			static void LW ( Instruction::Format i );
			static void LBU ( Instruction::Format i );
			static void LHU ( Instruction::Format i );
			static void LWL ( Instruction::Format i );
			static void LWR ( Instruction::Format i );
			static void SB ( Instruction::Format i );
			static void SH ( Instruction::Format i );
			static void SW ( Instruction::Format i );
			static void SWL ( Instruction::Format i );
			static void SWR ( Instruction::Format i );
			
			static void SYSCALL ( Instruction::Format i );
			static void BREAK ( Instruction::Format i );
			
			static void COP2 ( Instruction::Format i );
			
			
			// these instructions are R3000A only //
			/*
			static void RFE ( Instruction::Format i );
			static void LWC2 ( Instruction::Format i );
			static void SWC2 ( Instruction::Format i );
			static void MFC2 ( Instruction::Format i );
			static void MTC2 ( Instruction::Format i );
			static void RTPS ( Instruction::Format i );
			static void NCLIP ( Instruction::Format i );
			static void OP ( Instruction::Format i );
			static void DPCS ( Instruction::Format i );
			static void INTPL ( Instruction::Format i );
			static void MVMVA ( Instruction::Format i );
			static void NCDS ( Instruction::Format i );
			static void CDP ( Instruction::Format i );
			static void NCDT ( Instruction::Format i );
			static void NCCS ( Instruction::Format i );
			static void CC ( Instruction::Format i );
			static void NCS ( Instruction::Format i );
			static void NCT ( Instruction::Format i );
			static void SQR ( Instruction::Format i );
			static void DCPL ( Instruction::Format i );
			static void DPCT ( Instruction::Format i );
			static void AVSZ3 ( Instruction::Format i );
			static void AVSZ4 ( Instruction::Format i );
			static void RTPT ( Instruction::Format i );
			static void GPF ( Instruction::Format i );
			static void GPL ( Instruction::Format i );
			static void NCCT ( Instruction::Format i );
			*/
			
			
			// *** R5900 Instructions *** //
			
			static void BC0T ( Instruction::Format i );
			static void BC0TL ( Instruction::Format i );
			static void BC0F ( Instruction::Format i );
			static void BC0FL ( Instruction::Format i );
			static void BC1T ( Instruction::Format i );
			static void BC1TL ( Instruction::Format i );
			static void BC1F ( Instruction::Format i );
			static void BC1FL ( Instruction::Format i );
			static void BC2T ( Instruction::Format i );
			static void BC2TL ( Instruction::Format i );
			static void BC2F ( Instruction::Format i );
			static void BC2FL ( Instruction::Format i );
			

			static void CFC0 ( Instruction::Format i );
			static void CTC0 ( Instruction::Format i );
			static void EI ( Instruction::Format i );
			static void DI ( Instruction::Format i );
			
			static void SD ( Instruction::Format i );
			static void LD ( Instruction::Format i );
			static void LWU ( Instruction::Format i );
			static void SDL ( Instruction::Format i );
			static void SDR ( Instruction::Format i );
			static void LDL ( Instruction::Format i );
			static void LDR ( Instruction::Format i );
			static void LQ ( Instruction::Format i );
			static void SQ ( Instruction::Format i );
			
			
			// arithemetic instructions //
			static void DADD ( Instruction::Format i );
			static void DADDI ( Instruction::Format i );
			static void DADDU ( Instruction::Format i );
			static void DADDIU ( Instruction::Format i );
			static void DSUB ( Instruction::Format i );
			static void DSUBU ( Instruction::Format i );
			static void DSLL ( Instruction::Format i );
			static void DSLL32 ( Instruction::Format i );
			static void DSLLV ( Instruction::Format i );
			static void DSRA ( Instruction::Format i );
			static void DSRA32 ( Instruction::Format i );
			static void DSRAV ( Instruction::Format i );
			static void DSRL ( Instruction::Format i );
			static void DSRL32 ( Instruction::Format i );
			static void DSRLV ( Instruction::Format i );
			
			

			static void MFC1 ( Instruction::Format i );
			static void CFC1 ( Instruction::Format i );
			static void MTC1 ( Instruction::Format i );
			static void CTC1 ( Instruction::Format i );
			
			static void BEQL ( Instruction::Format i );
			static void BNEL ( Instruction::Format i );
			static void BGEZL ( Instruction::Format i );
			static void BLEZL ( Instruction::Format i );
			static void BGTZL ( Instruction::Format i );
			static void BLTZL ( Instruction::Format i );
			static void BLTZALL ( Instruction::Format i );
			static void BGEZALL ( Instruction::Format i );
			
			static void CACHE ( Instruction::Format i );
			static void PREF ( Instruction::Format i );
			
			static void TGEI ( Instruction::Format i );
			static void TGEIU ( Instruction::Format i );
			static void TLTI ( Instruction::Format i );
			static void TLTIU ( Instruction::Format i );
			static void TEQI ( Instruction::Format i );
			static void TNEI ( Instruction::Format i );
			static void TGE ( Instruction::Format i );
			static void TGEU ( Instruction::Format i );
			static void TLT ( Instruction::Format i );
			static void TLTU ( Instruction::Format i );
			static void TEQ ( Instruction::Format i );
			static void TNE ( Instruction::Format i );
			
			static void MOVCI ( Instruction::Format i );
			static void MOVZ ( Instruction::Format i );
			static void MOVN ( Instruction::Format i );
			static void SYNC ( Instruction::Format i );
			
			static void MFHI1 ( Instruction::Format i );
			static void MTHI1 ( Instruction::Format i );
			static void MFLO1 ( Instruction::Format i );
			static void MTLO1 ( Instruction::Format i );
			static void MULT1 ( Instruction::Format i );
			static void MULTU1 ( Instruction::Format i );
			static void DIV1 ( Instruction::Format i );
			static void DIVU1 ( Instruction::Format i );
			static void MADD ( Instruction::Format i );
			static void MADD1 ( Instruction::Format i );
			static void MADDU ( Instruction::Format i );
			static void MADDU1 ( Instruction::Format i );
			
			static void MFSA ( Instruction::Format i );
			static void MTSA ( Instruction::Format i );
			static void MTSAB ( Instruction::Format i );
			static void MTSAH ( Instruction::Format i );
			
			static void TLBR ( Instruction::Format i );
			static void TLBWI ( Instruction::Format i );
			static void TLBWR ( Instruction::Format i );
			static void TLBP ( Instruction::Format i );
			
			static void ERET ( Instruction::Format i );
			static void DERET ( Instruction::Format i );
			static void WAIT ( Instruction::Format i );
			
			
			// Parallel instructions (SIMD) //
			static void PABSH ( Instruction::Format i );
			static void PABSW ( Instruction::Format i );
			static void PADDB ( Instruction::Format i );
			static void PADDH ( Instruction::Format i );
			static void PADDW ( Instruction::Format i );
			static void PADDSB ( Instruction::Format i );
			static void PADDSH ( Instruction::Format i );
			static void PADDSW ( Instruction::Format i );
			static void PADDUB ( Instruction::Format i );
			static void PADDUH ( Instruction::Format i );
			static void PADDUW ( Instruction::Format i );
			static void PADSBH ( Instruction::Format i );
			static void PAND ( Instruction::Format i );
			static void POR ( Instruction::Format i );
			static void PXOR ( Instruction::Format i );
			static void PNOR ( Instruction::Format i );
			static void PCEQB ( Instruction::Format i );
			static void PCEQH ( Instruction::Format i );
			static void PCEQW ( Instruction::Format i );
			static void PCGTB ( Instruction::Format i );
			static void PCGTH ( Instruction::Format i );
			static void PCGTW ( Instruction::Format i );
			static void PCPYH ( Instruction::Format i );
			static void PCPYLD ( Instruction::Format i );
			static void PCPYUD ( Instruction::Format i );
			static void PDIVBW ( Instruction::Format i );
			static void PDIVUW ( Instruction::Format i );
			static void PDIVW ( Instruction::Format i );
			static void PEXCH ( Instruction::Format i );
			static void PEXCW ( Instruction::Format i );
			static void PEXEH ( Instruction::Format i );
			static void PEXEW ( Instruction::Format i );
			static void PEXT5 ( Instruction::Format i );
			static void PEXTLB ( Instruction::Format i );
			static void PEXTLH ( Instruction::Format i );
			static void PEXTLW ( Instruction::Format i );
			static void PEXTUB ( Instruction::Format i );
			static void PEXTUH ( Instruction::Format i );
			static void PEXTUW ( Instruction::Format i );
			static void PHMADH ( Instruction::Format i );
			static void PHMSBH ( Instruction::Format i );
			static void PINTEH ( Instruction::Format i );
			static void PINTH ( Instruction::Format i );
			static void PLZCW ( Instruction::Format i );
			static void PMADDH ( Instruction::Format i );
			static void PMADDW ( Instruction::Format i );
			static void PMADDUW ( Instruction::Format i );
			static void PMAXH ( Instruction::Format i );
			static void PMAXW ( Instruction::Format i );
			static void PMINH ( Instruction::Format i );
			static void PMINW ( Instruction::Format i );
			static void PMFHI ( Instruction::Format i );
			static void PMFLO ( Instruction::Format i );
			static void PMTHI ( Instruction::Format i );
			static void PMTLO ( Instruction::Format i );
			static void PMFHL_LH ( Instruction::Format i );
			static void PMFHL_SH ( Instruction::Format i );
			static void PMFHL_LW ( Instruction::Format i );
			static void PMFHL_UW ( Instruction::Format i );
			static void PMFHL_SLW ( Instruction::Format i );
			static void PMTHL_LW ( Instruction::Format i );
			static void PMSUBH ( Instruction::Format i );
			static void PMSUBW ( Instruction::Format i );
			static void PMULTH ( Instruction::Format i );
			static void PMULTW ( Instruction::Format i );
			static void PMULTUW ( Instruction::Format i );
			static void PPAC5 ( Instruction::Format i );
			static void PPACB ( Instruction::Format i );
			static void PPACH ( Instruction::Format i );
			static void PPACW ( Instruction::Format i );
			static void PREVH ( Instruction::Format i );
			static void PROT3W ( Instruction::Format i );
			static void PSLLH ( Instruction::Format i );
			static void PSLLVW ( Instruction::Format i );
			static void PSLLW ( Instruction::Format i );
			static void PSRAH ( Instruction::Format i );
			static void PSRAW ( Instruction::Format i );
			static void PSRAVW ( Instruction::Format i );
			static void PSRLH ( Instruction::Format i );
			static void PSRLW ( Instruction::Format i );
			static void PSRLVW ( Instruction::Format i );
			static void PSUBB ( Instruction::Format i );
			static void PSUBH ( Instruction::Format i );
			static void PSUBW ( Instruction::Format i );
			static void PSUBSB ( Instruction::Format i );
			static void PSUBSH ( Instruction::Format i );
			static void PSUBSW ( Instruction::Format i );
			static void PSUBUB ( Instruction::Format i );
			static void PSUBUH ( Instruction::Format i );
			static void PSUBUW ( Instruction::Format i );
			static void QFSRV ( Instruction::Format i );
			

			// floating point instructions //

			static void LWC1 ( Instruction::Format i );
			static void SWC1 ( Instruction::Format i );
			
			static void ABS_S ( Instruction::Format i );
			static void ADD_S ( Instruction::Format i );
			static void ADDA_S ( Instruction::Format i );
			static void C_EQ_S ( Instruction::Format i );
			static void C_F_S ( Instruction::Format i );
			static void C_LE_S ( Instruction::Format i );
			static void C_LT_S ( Instruction::Format i );
			static void CVT_S_W ( Instruction::Format i );
			static void CVT_W_S ( Instruction::Format i );
			static void DIV_S ( Instruction::Format i );
			static void MADD_S ( Instruction::Format i );
			static void MADDA_S ( Instruction::Format i );
			static void MAX_S ( Instruction::Format i );
			static void MIN_S ( Instruction::Format i );
			static void MOV_S ( Instruction::Format i );
			static void MSUB_S ( Instruction::Format i );
			static void MSUBA_S ( Instruction::Format i );
			static void MUL_S ( Instruction::Format i );
			static void MULA_S ( Instruction::Format i );
			static void NEG_S ( Instruction::Format i );
			static void RSQRT_S ( Instruction::Format i );
			static void SQRT_S ( Instruction::Format i );
			static void SUB_S ( Instruction::Format i );
			static void SUBA_S ( Instruction::Format i );
			

			// PS2 COP2 instructions //

			static void LQC2 ( Instruction::Format i );
			static void SQC2 ( Instruction::Format i );
			static void QMFC2_NI ( Instruction::Format i );
			static void QMTC2_NI ( Instruction::Format i );
			static void QMFC2_I ( Instruction::Format i );
			static void QMTC2_I ( Instruction::Format i );
			
			
			static void VABS ( Instruction::Format i );
			
			static void VADD ( Instruction::Format i );
			static void VADDi ( Instruction::Format i );
			static void VADDq ( Instruction::Format i );
			static void VADDBCX ( Instruction::Format i );
			static void VADDBCY ( Instruction::Format i );
			static void VADDBCZ ( Instruction::Format i );
			static void VADDBCW ( Instruction::Format i );
			
			static void VSUB ( Instruction::Format i );
			static void VSUBi ( Instruction::Format i );
			static void VSUBq ( Instruction::Format i );
			static void VSUBBCX ( Instruction::Format i );
			static void VSUBBCY ( Instruction::Format i );
			static void VSUBBCZ ( Instruction::Format i );
			static void VSUBBCW ( Instruction::Format i );
			
			static void VMUL ( Instruction::Format i );
			static void VMULi ( Instruction::Format i );
			static void VMULq ( Instruction::Format i );
			static void VMULBCX ( Instruction::Format i );
			static void VMULBCY ( Instruction::Format i );
			static void VMULBCZ ( Instruction::Format i );
			static void VMULBCW ( Instruction::Format i );
			
			static void VMADD ( Instruction::Format i );
			static void VMADDi ( Instruction::Format i );
			static void VMADDq ( Instruction::Format i );
			static void VMADDBCX ( Instruction::Format i );
			static void VMADDBCY ( Instruction::Format i );
			static void VMADDBCZ ( Instruction::Format i );
			static void VMADDBCW ( Instruction::Format i );
			
			static void VMSUB ( Instruction::Format i );
			static void VMSUBi ( Instruction::Format i );
			static void VMSUBq ( Instruction::Format i );
			static void VMSUBBCX ( Instruction::Format i );
			static void VMSUBBCY ( Instruction::Format i );
			static void VMSUBBCZ ( Instruction::Format i );
			static void VMSUBBCW ( Instruction::Format i );
			
			static void VMAX ( Instruction::Format i );
			static void VMAXi ( Instruction::Format i );
			static void VMAXBCX ( Instruction::Format i );
			static void VMAXBCY ( Instruction::Format i );
			static void VMAXBCZ ( Instruction::Format i );
			static void VMAXBCW ( Instruction::Format i );
			
			static void VMINI ( Instruction::Format i );
			static void VMINIi ( Instruction::Format i );
			static void VMINIBCX ( Instruction::Format i );
			static void VMINIBCY ( Instruction::Format i );
			static void VMINIBCZ ( Instruction::Format i );
			static void VMINIBCW ( Instruction::Format i );
			
			static void VDIV ( Instruction::Format i );
			
			static void VADDA ( Instruction::Format i );
			static void VADDAi ( Instruction::Format i );
			static void VADDAq ( Instruction::Format i );
			static void VADDABCX ( Instruction::Format i );
			static void VADDABCY ( Instruction::Format i );
			static void VADDABCZ ( Instruction::Format i );
			static void VADDABCW ( Instruction::Format i );
			
			static void VSUBA ( Instruction::Format i );
			static void VSUBAi ( Instruction::Format i );
			static void VSUBAq ( Instruction::Format i );
			static void VSUBABCX ( Instruction::Format i );
			static void VSUBABCY ( Instruction::Format i );
			static void VSUBABCZ ( Instruction::Format i );
			static void VSUBABCW ( Instruction::Format i );
			
			static void VMULA ( Instruction::Format i );
			static void VMULAi ( Instruction::Format i );
			static void VMULAq ( Instruction::Format i );
			static void VMULABCX ( Instruction::Format i );
			static void VMULABCY ( Instruction::Format i );
			static void VMULABCZ ( Instruction::Format i );
			static void VMULABCW ( Instruction::Format i );
			
			static void VMADDA ( Instruction::Format i );
			static void VMADDAi ( Instruction::Format i );
			static void VMADDAq ( Instruction::Format i );
			static void VMADDABCX ( Instruction::Format i );
			static void VMADDABCY ( Instruction::Format i );
			static void VMADDABCZ ( Instruction::Format i );
			static void VMADDABCW ( Instruction::Format i );
			
			static void VMSUBA ( Instruction::Format i );
			static void VMSUBAi ( Instruction::Format i );
			static void VMSUBAq ( Instruction::Format i );
			static void VMSUBABCX ( Instruction::Format i );
			static void VMSUBABCY ( Instruction::Format i );
			static void VMSUBABCZ ( Instruction::Format i );
			static void VMSUBABCW ( Instruction::Format i );
			
			static void VOPMULA ( Instruction::Format i );
			static void VOPMSUB ( Instruction::Format i );

			static void VNOP ( Instruction::Format i );
			static void VCLIP ( Instruction::Format i );
			static void VSQRT ( Instruction::Format i );
			static void VRSQRT ( Instruction::Format i );
			static void VMR32 ( Instruction::Format i );
			static void VRINIT ( Instruction::Format i );
			static void VRGET ( Instruction::Format i );
			static void VRNEXT ( Instruction::Format i );
			static void VRXOR ( Instruction::Format i );
			static void VMOVE ( Instruction::Format i );
			static void VMFIR ( Instruction::Format i );
			static void VMTIR ( Instruction::Format i );
			static void VLQD ( Instruction::Format i );
			static void VLQI ( Instruction::Format i );
			static void VSQD ( Instruction::Format i );
			static void VSQI ( Instruction::Format i );
			static void VWAITQ ( Instruction::Format i );
			
			static void VFTOI0 ( Instruction::Format i );
			static void VFTOI4 ( Instruction::Format i );
			static void VFTOI12 ( Instruction::Format i );
			static void VFTOI15 ( Instruction::Format i );
			
			static void VITOF0 ( Instruction::Format i );
			static void VITOF4 ( Instruction::Format i );
			static void VITOF12 ( Instruction::Format i );
			static void VITOF15 ( Instruction::Format i );
			
			static void VIADD ( Instruction::Format i );
			static void VISUB ( Instruction::Format i );
			static void VIADDI ( Instruction::Format i );
			static void VIAND ( Instruction::Format i );
			static void VIOR ( Instruction::Format i );
			static void VILWR ( Instruction::Format i );
			static void VISWR ( Instruction::Format i );
			static void VCALLMS ( Instruction::Format i );
			static void VCALLMSR ( Instruction::Format i );


			typedef void (*Callback_Function) ( Cpu* r );
			
			static void TRUE_DelaySlot_Callback ( Cpu* r );
			static void Jump_DelaySlot_Callback ( Cpu* r );
			static void JumpRegister_DelaySlot_Callback ( Cpu* r );
			static void Branch_DelaySlot_Callback ( Cpu* r );
			static void FC_Callback ( Cpu* r );
			static void MTC0_Callback ( Cpu* r );
			static void MTC2_Callback ( Cpu* r );
			static void CTC2_Callback ( Cpu* r );
			static void LB_DelaySlot_Callback_Bus ( Cpu* r );
			static void LH_DelaySlot_Callback_Bus ( Cpu* r );
			static void LW_DelaySlot_Callback_Bus ( Cpu* r );
			static void LBU_DelaySlot_Callback_Bus ( Cpu* r );
			static void LHU_DelaySlot_Callback_Bus ( Cpu* r );
			static void LWC2_DelaySlot_Callback_Bus ( Cpu* r );
			static void LB_DelaySlot_Callback_DCache ( Cpu* r );
			static void LH_DelaySlot_Callback_DCache ( Cpu* r );
			static void LW_DelaySlot_Callback_DCache ( Cpu* r );
			static void LBU_DelaySlot_Callback_DCache ( Cpu* r );
			static void LHU_DelaySlot_Callback_DCache ( Cpu* r );
			static void LWC2_DelaySlot_Callback_DCache ( Cpu* r );
			static void LB_Callback_Bus ( Cpu* r );
			static void LH_Callback_Bus ( Cpu* r );
			static void LW_Callback_Bus ( Cpu* r );
			static void LBU_Callback_Bus ( Cpu* r );
			static void LHU_Callback_Bus ( Cpu* r );
			static void LWL_Callback_Bus ( Cpu* r );
			static void LWR_Callback_Bus ( Cpu* r );
			static void LWC2_Callback_Bus ( Cpu* r );
			static void SB_Callback_Bus ( Cpu* r );
			static void SH_Callback_Bus ( Cpu* r );
			static void SW_Callback_Bus ( Cpu* r );
			static void SBU_Callback_Bus ( Cpu* r );
			static void SHU_Callback_Bus ( Cpu* r );
			static void SWL_Callback_Bus ( Cpu* r );
			static void SWR_Callback_Bus ( Cpu* r );
			static void SWC2_Callback_Bus ( Cpu* r );

			
		private:
		
#ifdef _DEBUG_VERSION_
			static Debug::Log debug;
#endif
			
		};
		
	};
	
};

#endif

