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


#ifndef _R3000A_EXECUTE_H_

#define _R3000A_EXECUTE_H_

#include "R3000A.h"
#include "R3000A_Instruction.h"
#include "R3000A_Lookup.h"
#include "types.h"


namespace R3000A
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

			// this actually has to be a 4D array, but I'll make it one array			
			// use Opcode, Rs, Rt, and Funct to lookup value
			//static Function LookupTable [ 64 * 32 * 32 * 64 ];
		
			//static void SignalSynchronousInterrupt ( u32 ExceptionType );
			
			static const Function FunctionList [];
			
			inline static void ExecuteInstruction ( Instruction::Format i ) { FunctionList [ Lookup::FindByInstruction ( i.Value ) ] ( i ); }

			static void Invalid ( Instruction::Format i );

			static void J ( Instruction::Format i );
			static void JAL ( Instruction::Format i );
			static void BEQ ( Instruction::Format i );
			static void BNE ( Instruction::Format i );
			static void BLEZ ( Instruction::Format i );
			static void BGTZ ( Instruction::Format i );
			static void ADDI ( Instruction::Format i );
			static void ADDIU ( Instruction::Format i );
			static void SLTI ( Instruction::Format i );
			static void SLTIU ( Instruction::Format i );
			static void ANDI ( Instruction::Format i );
			static void ORI ( Instruction::Format i );
			static void XORI ( Instruction::Format i );
			static void LUI ( Instruction::Format i );
			static void LB ( Instruction::Format i );
			static void LH ( Instruction::Format i );
			static void LWL ( Instruction::Format i );
			static void LW ( Instruction::Format i );
			static void LBU ( Instruction::Format i );
			static void LHU ( Instruction::Format i );
			
			static void LWR ( Instruction::Format i );
			static void SB ( Instruction::Format i );
			static void SH ( Instruction::Format i );
			static void SWL ( Instruction::Format i );
			static void SW ( Instruction::Format i );
			static void SWR ( Instruction::Format i );
			static void LWC2 ( Instruction::Format i );
			static void SWC2 ( Instruction::Format i );
			static void SLL ( Instruction::Format i );
			static void SRL ( Instruction::Format i );
			static void SRA ( Instruction::Format i );
			static void SLLV ( Instruction::Format i );
			static void SRLV ( Instruction::Format i );
			static void SRAV ( Instruction::Format i );
			static void JR ( Instruction::Format i );
			static void JALR ( Instruction::Format i );
			static void SYSCALL ( Instruction::Format i );
			static void BREAK ( Instruction::Format i );
			static void MFHI ( Instruction::Format i );
			static void MTHI ( Instruction::Format i );

			static void MFLO ( Instruction::Format i );
			static void MTLO ( Instruction::Format i );
			static void MULT ( Instruction::Format i );
			static void MULTU ( Instruction::Format i );
			static void DIV ( Instruction::Format i );
			static void DIVU ( Instruction::Format i );
			static void ADD ( Instruction::Format i );
			static void ADDU ( Instruction::Format i );
			static void SUB ( Instruction::Format i );
			static void SUBU ( Instruction::Format i );
			static void AND ( Instruction::Format i );
			static void OR ( Instruction::Format i );
			static void XOR ( Instruction::Format i );
			static void NOR ( Instruction::Format i );
			static void SLT ( Instruction::Format i );
			static void SLTU ( Instruction::Format i );
			static void BLTZ ( Instruction::Format i );
			static void BGEZ ( Instruction::Format i );
			static void BLTZAL ( Instruction::Format i );
			static void BGEZAL ( Instruction::Format i );

			static void MFC0 ( Instruction::Format i );
			static void MTC0 ( Instruction::Format i );
			static void RFE ( Instruction::Format i );
			static void MFC2 ( Instruction::Format i );
			static void CFC2 ( Instruction::Format i );
			static void MTC2 ( Instruction::Format i );
			static void CTC2 ( Instruction::Format i );
			static void COP2 ( Instruction::Format i );
			
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

			
			typedef void (*Callback_Function) ( R3000A::Cpu* r );


			// the recompiler handles half of the loads/stores, and the function handles the other half
			static void SB_Recompiler ( u32 StoreValue, u32 StoreAddress );
			static void SH_Recompiler ( u32 StoreValue, u32 StoreAddress );
			static void SW_Recompiler ( u32 StoreValue, u32 StoreAddress );
			static void SWL_Recompiler ( u32 StoreValue, u32 StoreAddress );
			static void SWR_Recompiler ( u32 StoreValue, u32 StoreAddress );
			static void SWC2_Recompiler ( u32 StoreValue, u32 StoreAddress );
			
			/*
			static void LB_Recompiler ( u32 StoreValue, u32 LoadAddress );
			static void LH_Recompiler ( u32 StoreValue, u32 LoadAddress );
			static void LBU_Recompiler ( u32 StoreValue, u32 LoadAddress );
			static void LHU_Recompiler ( u32 StoreValue, u32 LoadAddress );
			static void LW_Recompiler ( u32 StoreValue, u32 LoadAddress );
			static void LWL_Recompiler ( u32 StoreValue, u32 LoadAddress );
			static void LWR_Recompiler ( u32 StoreValue, u32 LoadAddress );
			static void LWC2_Recompiler ( u32 StoreValue, u32 LoadAddress );
			*/
			
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
		
			static Debug::Log debug;
			
		};
		
	};
	
};

#endif

