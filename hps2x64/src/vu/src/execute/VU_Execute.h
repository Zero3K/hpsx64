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
#include "VU.h"
#include "VU_Instruction.h"
#include "VU_Lookup.h"

using namespace std;
using namespace Vu::Instruction;
using namespace Playstation2;


#ifndef _VU_EXECUTE_H_
#define _VU_EXECUTE_H_


// this should be enabled, but need to toggle for testing
#define ENABLE_VUFLAG_CLEAR

// this should be enabled to delay the flag update, which is how it appears to work on PS2
#define DELAY_FLAG_UPDATE


// will need this for accurate operation, and cycle accuracy is required
#define ENABLE_STALLS



//#define ENABLE_NEW_CLIP_BUFFER
//#define ENABLE_NEW_FLAG_BUFFER
#define ENABLE_SNAPSHOTS



namespace Vu
{

	namespace Instruction
	{
	
		class Execute
		{
		public:
		
			static Debug::Log debug;

			
			//VU *v;
			
			
			
			typedef void (*Function) ( VU *v, Instruction::Format i );
			

			// upper instruction type op
			typedef float (*OpType0) ( float fs, float ft, int index, unsigned short* StatusFlag, unsigned short* MACFlag );
			
			// upper instruction with accumulator type op
			typedef float (*OpType1) ( float dACC, float fd, float fs, float ft, int index, unsigned short* StatusFlag, unsigned short* MACFlag );
			
			// upper instruction type op store to ACC
			typedef float (*OpType0A) ( float fs, float ft, int index, unsigned short* StatusFlag, unsigned short* MACFlag );
			
			// upper instruction with accumulator type op w/ store to ACC
			typedef float (*OpType1A) ( float dACC, float fd, float fs, float ft, int index, unsigned short* StatusFlag, unsigned short* MACFlag );
			
			// lower floating point instruction (sqrt,rsqrt,div,etc) type op
			typedef float (*OpType2) ( float fs, float ft, unsigned short* StatusFlag );
			
			
			// clears the non-sticky non-divide/invalid status flags and all MAC flags
			inline static void ClearFlags ( VU *v )
			{
#ifdef ENABLE_VUFLAG_CLEAR

#ifdef DELAY_FLAG_UPDATE


#ifdef ENABLE_SNAPSHOTS
				// clear affected non-sticky status flags (O,U,S,Z)
				// note: status flag should only get cleared before the full SIMD instruction has executed
				v->vi [ 16 ].uLo &= ~0xf;
				
				// clear MAC flag - this should be the correct operation
				// note: this must only be cleared before execution of instruction
				v->vi [ 17 ].uLo = 0;
#else
				// clear affected non-sticky status flags (O,U,S,Z)
				// note: status flag should only get cleared before the full SIMD instruction has executed
				//v->FlagSave [ v->iFlagSave_Index ].StatusFlag &= ~0xf;
				// clear MAC flag - this should be the correct operation
				// note: this must only be cleared before execution of instruction
				//v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].Value = 0;
				
				// enable flag entry
				v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].FlagsAffected = 1;
				
				// clear MAC flag for pipeline stage
				v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].MACFlag = 0;
				
				// clear status flag for pipeline stage also (sticky bits are already set properly in the actual flag)
				v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].StatusFlag = 0;

				// *new* set cycle number change should take place at
				v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].ullBusyUntil_Cycle = v->CycleCount + VU::c_ullFloatPipeline_Cycles;

#endif	// end ENABLE_SNAPSHOTS

#else
				// clear affected non-sticky status flags (O,U,S,Z)
				// note: status flag should only get cleared before the full SIMD instruction has executed
				v->vi [ 16 ].uLo &= ~0xf;
				
				// clear MAC flag - this should be the correct operation
				// note: this must only be cleared before execution of instruction
				v->vi [ 17 ].uLo = 0;

#endif	// end DELAY_FLAG_UPDATE
				
#endif	// end ENABLE_VUFLAG_CLEAR

			}	// end inline static void ClearFlags ( VU *v )
			
			
			// takes the MAC flag results, and uses it to set the status flags
			// note: probably don't want to do this though for MADD/MSUB
			inline static void SetStatusFlags ( VU *v )
			{
				if ( v->vi [ 17 ].uLo & 0xf ) v->vi [ 16 ].uLo |= 0x41;
				if ( v->vi [ 17 ].uLo & 0xf0 ) v->vi [ 16 ].uLo |= 0x82;
				if ( v->vi [ 17 ].uLo & 0xf00 ) v->vi [ 16 ].uLo |= 0x104;
				if ( v->vi [ 17 ].uLo & 0xf000 ) v->vi [ 16 ].uLo |= 0x208;
			}
			
			
			// helper function for calling PS2 VU float functions
			inline static float OpFunc0_st ( OpType0 OpFunc0, VU *v, float fs, float ft, int index )
			{
#ifdef DELAY_FLAG_UPDATE

#ifdef ENABLE_SNAPSHOTS
				//return OpFunc0 ( fs, ft, index, &v->Temp_StatusFlag, &v->Temp_MacFlag );
				if ( !v->Status.SetStatus_Flag )
				{
					return OpFunc0 ( fs, ft, index, & v->vi [ 16 ].uLo, & v->vi [ 17 ].uLo );
				}
				
				return OpFunc0 ( fs, ft, index, & v->Temp_StatusFlag, & v->vi [ 17 ].uLo );
				
#else
				return OpFunc0 ( fs, ft, index, & v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].StatusFlag, & v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].MACFlag );
#endif

#else
				return OpFunc0 ( fs, ft, index, & v->vi [ 16 ].uLo, & v->vi [ 17 ].uLo );
#endif
			}
			
			inline static float OpFunc0_st ( OpType1 OpFunc0, VU *v, float ACC, float fd, float fs, float ft, int index )
			{
#ifdef DELAY_FLAG_UPDATE

#ifdef ENABLE_SNAPSHOTS
				//return OpFunc0 ( ACC, fd, fs, ft, index, &v->Temp_StatusFlag, &v->Temp_MacFlag );
				if ( !v->Status.SetStatus_Flag )
				{
					return OpFunc0 ( ACC, fd, fs, ft, index, & v->vi [ 16 ].uLo, & v->vi [ 17 ].uLo );
				}
				
				return OpFunc0 ( ACC, fd, fs, ft, index, & v->Temp_StatusFlag, & v->vi [ 17 ].uLo );
#else
				return OpFunc0 ( ACC, fd, fs, ft, index, & v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].StatusFlag, & v->FlagSave [ v->iFlagSave_Index & v->c_lFlag_Delay_Mask ].MACFlag );
#endif

#else
				return OpFunc0 ( ACC, fd, fs, ft, index, & v->vi [ 16 ].uLo, & v->vi [ 17 ].uLo );
#endif
			}
			
			
			inline static void VuUpperOp ( VU *v, Instruction::Format i, OpType0 OpFunc0 )
			{
				
#ifdef ENABLE_STALLS

				// set the source register(s)
				v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx, v->vf [ i.Ft ].fx, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy, v->vf [ i.Ft ].fy, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz, v->vf [ i.Ft ].fz, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw, v->vf [ i.Ft ].fw, 0 );
				}
				
				
				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}

			inline static void VuUpperOpI ( VU *v, Instruction::Format i, OpType0 OpFunc0 )
			{
				
#ifdef ENABLE_STALLS

	
				// set the source register(s)
				v->Set_SrcReg ( i.Value, i.Fs );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );

#endif

				// clear flags
				ClearFlags ( v );

				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx, v->vi [ 21 ].f, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy, v->vi [ 21 ].f, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz, v->vi [ 21 ].f, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw, v->vi [ 21 ].f, 0 );
				}
				
				
				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}

			inline static void VuUpperOpQ ( VU *v, Instruction::Format i, OpType0 OpFunc0 )
			{
				
#ifdef ENABLE_STALLS


				// set the source register(s)
				v->Set_SrcReg ( i.Value, i.Fs );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );

				
#ifdef ENABLE_NEW_QP_HANDLING
				// if div/unit is in use, check if it is done yet
				v->CheckQ ();
#endif

#endif

				// update q first
				//v->UpdateQ_Micro ();

				// clear flags
				ClearFlags ( v );

				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx, v->vi [ 22 ].f, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy, v->vi [ 22 ].f, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz, v->vi [ 22 ].f, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw, v->vi [ 22 ].f, 0 );
				}
				
				
				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}

			inline static void VuUpperOpX ( VU *v, Instruction::Format i, OpType0 OpFunc0 )
			{
				float t_fx;
				
#ifdef ENABLE_STALLS

				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

				// clear flags
				ClearFlags ( v );
				
				// get x component for the op
				t_fx = v->vf [ i.Ft ].fx;

				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx, t_fx, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy, t_fx, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz, t_fx, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw, t_fx, 0 );
				}
				

				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}

			inline static void VuUpperOpY ( VU *v, Instruction::Format i, OpType0 OpFunc0 )
			{
				float t_fy;
				
#ifdef ENABLE_STALLS

				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

				// clear flags
				ClearFlags ( v );

				// get y component for the op
				t_fy = v->vf [ i.Ft ].fy;
				
				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx, t_fy, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy, t_fy, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz, t_fy, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw, t_fy, 0 );
				}
				

				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}

			inline static void VuUpperOpZ ( VU *v, Instruction::Format i, OpType0 OpFunc0 )
			{
				float t_fz;
				
#ifdef ENABLE_STALLS

				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

				// clear flags
				ClearFlags ( v );

				// get z component for the op
				t_fz = v->vf [ i.Ft ].fz;
				
				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx, t_fz, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy, t_fz, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz, t_fz, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw, t_fz, 0 );
				}
				

				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}

			inline static void VuUpperOpW ( VU *v, Instruction::Format i, OpType0 OpFunc0 )
			{
				float t_fw;
				
#ifdef ENABLE_STALLS

				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

				// clear flags
				ClearFlags ( v );

				// get w component for the op
				t_fw = v->vf [ i.Ft ].fw;
				
				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx, t_fw, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy, t_fw, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz, t_fw, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw, t_fw, 0 );
				}
				

				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}

// -------------------------------------------

			
			inline static void VuUpperOp_A ( VU *v, Instruction::Format i, OpType0A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS

				// set the source register(s)
				v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
#endif

				// clear flags
				ClearFlags ( v );

				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx,  v->vf [ i.Ft ].fx, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy,  v->vf [ i.Ft ].fy, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz,  v->vf [ i.Ft ].fz, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw,  v->vf [ i.Ft ].fw, 0 );
				}
				
			}

			inline static void VuUpperOpI_A ( VU *v, Instruction::Format i, OpType0A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS

				// set the source register(s)
				v->Set_SrcReg ( i.Value, i.Fs );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
#endif

				// clear flags
				ClearFlags ( v );

				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx, v->vi [ 21 ].f, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy, v->vi [ 21 ].f, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz, v->vi [ 21 ].f, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw, v->vi [ 21 ].f, 0 );
				}
				
			}

			inline static void VuUpperOpQ_A ( VU *v, Instruction::Format i, OpType0A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS

				// set the source register(s)
				v->Set_SrcReg ( i.Value, i.Fs );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register


#ifdef ENABLE_NEW_QP_HANDLING
				// if div/unit is in use, check if it is done yet
				v->CheckQ ();
#endif
				
#endif

				// update q first
				//v->UpdateQ_Micro ();

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx, v->vi [ 22 ].f, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy, v->vi [ 22 ].f, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz, v->vi [ 22 ].f, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw, v->vi [ 22 ].f, 0 );
				}
				
			}

			inline static void VuUpperOpX_A ( VU *v, Instruction::Format i, OpType0A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS

				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx,  v->vf [ i.Ft ].fx, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy,  v->vf [ i.Ft ].fx, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz,  v->vf [ i.Ft ].fx, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw,  v->vf [ i.Ft ].fx, 0 );
				}
				
			}

			inline static void VuUpperOpY_A ( VU *v, Instruction::Format i, OpType0A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS

				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx,  v->vf [ i.Ft ].fy, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy,  v->vf [ i.Ft ].fy, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz,  v->vf [ i.Ft ].fy, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw,  v->vf [ i.Ft ].fy, 0 );
				}
				
			}

			inline static void VuUpperOpZ_A ( VU *v, Instruction::Format i, OpType0A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS

				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx,  v->vf [ i.Ft ].fz, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy,  v->vf [ i.Ft ].fz, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz,  v->vf [ i.Ft ].fz, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw,  v->vf [ i.Ft ].fz, 0 );
				}
				
			}

			inline static void VuUpperOpW_A ( VU *v, Instruction::Format i, OpType0A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS

				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx,  v->vf [ i.Ft ].fw, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy,  v->vf [ i.Ft ].fw, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz,  v->vf [ i.Ft ].fw, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw,  v->vf [ i.Ft ].fw, 0 );
				}
				
			}
			


			static const Function FunctionList [];
			
			inline static void ExecuteInstructionLO ( VU *v, Instruction::Format i ) { FunctionList [ Lookup::FindByInstructionLO ( i.Value ) ] ( v, i ); }
			inline static void ExecuteInstructionHI ( VU *v, Instruction::Format i ) { FunctionList [ Lookup::FindByInstructionHI ( i.Value ) ] ( v, i ); }
			
			
			
			static const char XyzwLUT [ 4 ];	// = { 'x', 'y', 'z', 'w' };
			static const char* BCType [ 4 ];// = { "F", "T", "FL", "TL" };

			
			
			// constructor
			//Execute ( Playstation2::VU *vv ) { v = vv; }
			
			// creates lookup table from list of entries
			static void Start ();
		
			static void Execute_LoadDelaySlot ( VU *v, Instruction::Format i );
			

			static void INVALID ( VU *v, Instruction::Format i );
			

			static void ADDBCX ( VU *v, Instruction::Format i );
			static void ADDBCY ( VU *v, Instruction::Format i );
			static void ADDBCZ ( VU *v, Instruction::Format i );
			static void ADDBCW ( VU *v, Instruction::Format i );
			
			static void SUBBCX ( VU *v, Instruction::Format i );
			static void SUBBCY ( VU *v, Instruction::Format i );
			static void SUBBCZ ( VU *v, Instruction::Format i );
			static void SUBBCW ( VU *v, Instruction::Format i );
			
			static void MADDBCX ( VU *v, Instruction::Format i );
			static void MADDBCY ( VU *v, Instruction::Format i );
			static void MADDBCZ ( VU *v, Instruction::Format i );
			static void MADDBCW ( VU *v, Instruction::Format i );
			
			static void MSUBBCX ( VU *v, Instruction::Format i );
			static void MSUBBCY ( VU *v, Instruction::Format i );
			static void MSUBBCZ ( VU *v, Instruction::Format i );
			static void MSUBBCW ( VU *v, Instruction::Format i );
			
			static void MAXBCX ( VU *v, Instruction::Format i );
			static void MAXBCY ( VU *v, Instruction::Format i );
			static void MAXBCZ ( VU *v, Instruction::Format i );
			static void MAXBCW ( VU *v, Instruction::Format i );
			
			static void MINIBCX ( VU *v, Instruction::Format i );
			static void MINIBCY ( VU *v, Instruction::Format i );
			static void MINIBCZ ( VU *v, Instruction::Format i );
			static void MINIBCW ( VU *v, Instruction::Format i );
			
			static void MULBCX ( VU *v, Instruction::Format i );
			static void MULBCY ( VU *v, Instruction::Format i );
			static void MULBCZ ( VU *v, Instruction::Format i );
			static void MULBCW ( VU *v, Instruction::Format i );
			
			static void MULq ( VU *v, Instruction::Format i );
			
			static void MAXi ( VU *v, Instruction::Format i );
			static void MULi ( VU *v, Instruction::Format i );
			static void MINIi ( VU *v, Instruction::Format i );
			static void ADDq ( VU *v, Instruction::Format i );
			static void MADDq ( VU *v, Instruction::Format i );
			static void ADDi ( VU *v, Instruction::Format i );
			static void MADDi ( VU *v, Instruction::Format i );
			static void OPMSUB ( VU *v, Instruction::Format i );
			static void SUBq ( VU *v, Instruction::Format i );
			static void MSUBq ( VU *v, Instruction::Format i );
			static void SUBi ( VU *v, Instruction::Format i );
			static void MSUBi ( VU *v, Instruction::Format i );
			static void ADD ( VU *v, Instruction::Format i );
			
			//static void ADDi ( VU *v, Instruction::Format i );
			//static void ADDq ( VU *v, Instruction::Format i );
			//static void ADDAi ( VU *v, Instruction::Format i );
			//static void ADDAq ( VU *v, Instruction::Format i );
			
			static void ADDABCX ( VU *v, Instruction::Format i );
			static void ADDABCY ( VU *v, Instruction::Format i );
			static void ADDABCZ ( VU *v, Instruction::Format i );
			static void ADDABCW ( VU *v, Instruction::Format i );
			
			static void MADD ( VU *v, Instruction::Format i );
			
			static void MUL ( VU *v, Instruction::Format i );
			static void MAX ( VU *v, Instruction::Format i );
			static void SUB ( VU *v, Instruction::Format i );
			static void MSUB ( VU *v, Instruction::Format i );
			static void OPMSUM ( VU *v, Instruction::Format i );
			static void MINI ( VU *v, Instruction::Format i );
			static void IADD ( VU *v, Instruction::Format i );
			static void ISUB ( VU *v, Instruction::Format i );
			static void IADDI ( VU *v, Instruction::Format i );
			static void IAND ( VU *v, Instruction::Format i );
			static void IOR ( VU *v, Instruction::Format i );
			//static void CALLMS ( VU *v, Instruction::Format i );
			//static void CALLMSR ( VU *v, Instruction::Format i );
			static void ITOF0 ( VU *v, Instruction::Format i );
			static void FTOI0 ( VU *v, Instruction::Format i );
			static void MULAq ( VU *v, Instruction::Format i );
			static void ADDAq ( VU *v, Instruction::Format i );
			static void SUBAq ( VU *v, Instruction::Format i );
			static void ADDA ( VU *v, Instruction::Format i );
			static void SUBA ( VU *v, Instruction::Format i );
			static void MOVE ( VU *v, Instruction::Format i );
			static void LQI ( VU *v, Instruction::Format i );
			static void DIV ( VU *v, Instruction::Format i );
			static void MTIR ( VU *v, Instruction::Format i );
			//static void RNEXT ( VU *v, Instruction::Format i );
			static void ITOF4 ( VU *v, Instruction::Format i );
			static void FTOI4 ( VU *v, Instruction::Format i );
			static void ABS ( VU *v, Instruction::Format i );
			static void MADDAq ( VU *v, Instruction::Format i );
			static void MSUBAq ( VU *v, Instruction::Format i );
			static void MADDA ( VU *v, Instruction::Format i );
			static void MSUBA ( VU *v, Instruction::Format i );
			//static void MR32 ( VU *v, Instruction::Format i );
			//static void SQI ( VU *v, Instruction::Format i );
			//static void SQRT ( VU *v, Instruction::Format i );
			//static void MFIR ( VU *v, Instruction::Format i );
			//static void RGET ( VU *v, Instruction::Format i );
			
			//static void ADDABCX ( VU *v, Instruction::Format i );
			//static void ADDABCY ( VU *v, Instruction::Format i );
			//static void ADDABCZ ( VU *v, Instruction::Format i );
			//static void ADDABCW ( VU *v, Instruction::Format i );
			
			static void SUBABCX ( VU *v, Instruction::Format i );
			static void SUBABCY ( VU *v, Instruction::Format i );
			static void SUBABCZ ( VU *v, Instruction::Format i );
			static void SUBABCW ( VU *v, Instruction::Format i );
			
			static void MADDABCX ( VU *v, Instruction::Format i );
			static void MADDABCY ( VU *v, Instruction::Format i );
			static void MADDABCZ ( VU *v, Instruction::Format i );
			static void MADDABCW ( VU *v, Instruction::Format i );
			
			static void MSUBABCX ( VU *v, Instruction::Format i );
			static void MSUBABCY ( VU *v, Instruction::Format i );
			static void MSUBABCZ ( VU *v, Instruction::Format i );
			static void MSUBABCW ( VU *v, Instruction::Format i );
			
			static void ITOF12 ( VU *v, Instruction::Format i );
			static void FTOI12 ( VU *v, Instruction::Format i );
			
			static void MULABCX ( VU *v, Instruction::Format i );
			static void MULABCY ( VU *v, Instruction::Format i );
			static void MULABCZ ( VU *v, Instruction::Format i );
			static void MULABCW ( VU *v, Instruction::Format i );
			
			static void MULAi ( VU *v, Instruction::Format i );
			static void ADDAi ( VU *v, Instruction::Format i );
			static void SUBAi ( VU *v, Instruction::Format i );
			static void MULA ( VU *v, Instruction::Format i );
			static void OPMULA ( VU *v, Instruction::Format i );
			//static void LQD ( VU *v, Instruction::Format i );
			//static void RSQRT ( VU *v, Instruction::Format i );
			//static void ILWR ( VU *v, Instruction::Format i );
			//static void RINIT ( VU *v, Instruction::Format i );
			static void ITOF15 ( VU *v, Instruction::Format i );
			static void FTOI15 ( VU *v, Instruction::Format i );
			static void CLIP ( VU *v, Instruction::Format i );
			static void MADDAi ( VU *v, Instruction::Format i );
			static void MSUBAi ( VU *v, Instruction::Format i );
			static void NOP ( VU *v, Instruction::Format i );
			//static void SQD ( VU *v, Instruction::Format i );


			// lower instructions

			
			static void LQ ( VU *v, Instruction::Format i );
			static void SQ ( VU *v, Instruction::Format i );
			static void ILW ( VU *v, Instruction::Format i );
			static void ISW ( VU *v, Instruction::Format i );
			static void IADDIU ( VU *v, Instruction::Format i );
			static void ISUBIU ( VU *v, Instruction::Format i );
			static void FCEQ ( VU *v, Instruction::Format i );
			static void FCSET ( VU *v, Instruction::Format i );
			static void FCAND ( VU *v, Instruction::Format i );
			static void FCOR ( VU *v, Instruction::Format i );
			static void FSEQ ( VU *v, Instruction::Format i );
			static void FSSET ( VU *v, Instruction::Format i );
			static void FSAND ( VU *v, Instruction::Format i );
			static void FSOR ( VU *v, Instruction::Format i );
			static void FMEQ ( VU *v, Instruction::Format i );
			static void FMAND ( VU *v, Instruction::Format i );
			static void FMOR ( VU *v, Instruction::Format i );
			static void FCGET ( VU *v, Instruction::Format i );
			static void B ( VU *v, Instruction::Format i );
			static void BAL ( VU *v, Instruction::Format i );
			static void JR ( VU *v, Instruction::Format i );
			static void JALR ( VU *v, Instruction::Format i );
			static void IBEQ ( VU *v, Instruction::Format i );
			static void IBNE ( VU *v, Instruction::Format i );
			static void IBLTZ ( VU *v, Instruction::Format i );
			static void IBGTZ ( VU *v, Instruction::Format i );
			static void IBLEZ ( VU *v, Instruction::Format i );
			static void IBGEZ ( VU *v, Instruction::Format i );
			
			//static void LowerOp ( VU *v, Instruction::Format i );
			//static void Lower60 ( VU *v, Instruction::Format i );
			//static void Lower61 ( VU *v, Instruction::Format i );
			//static void Lower62 ( VU *v, Instruction::Format i );
			//static void Lower63 ( VU *v, Instruction::Format i );
			
			//static void DIV ( VU *v, Instruction::Format i );
			//static void EATANxy ( VU *v, Instruction::Format i );
			//static void EATANxz ( VU *v, Instruction::Format i );
			//static void EATAN ( VU *v, Instruction::Format i );
			//static void IADD ( VU *v, Instruction::Format i );
			//static void ISUB ( VU *v, Instruction::Format i );
			//static void IADDI ( VU *v, Instruction::Format i );
			//static void IAND ( VU *v, Instruction::Format i );
			//static void IOR ( VU *v, Instruction::Format i );
			//static void MOVE ( VU *v, Instruction::Format i );
			//static void LQI ( VU *v, Instruction::Format i );
			//static void DIV ( VU *v, Instruction::Format i );
			//static void MTIR ( VU *v, Instruction::Format i );
			static void RNEXT ( VU *v, Instruction::Format i );
			static void MFP ( VU *v, Instruction::Format i );
			static void XTOP ( VU *v, Instruction::Format i );
			static void XGKICK ( VU *v, Instruction::Format i );

			static void MR32 ( VU *v, Instruction::Format i );
			static void SQI ( VU *v, Instruction::Format i );
			static void SQRT ( VU *v, Instruction::Format i );
			static void MFIR ( VU *v, Instruction::Format i );
			static void RGET ( VU *v, Instruction::Format i );
			
			static void XITOP ( VU *v, Instruction::Format i );
			static void ESADD ( VU *v, Instruction::Format i );
			static void EATANxy ( VU *v, Instruction::Format i );
			static void ESQRT ( VU *v, Instruction::Format i );
			static void ESIN ( VU *v, Instruction::Format i );
			static void ERSADD ( VU *v, Instruction::Format i );
			static void EATANxz ( VU *v, Instruction::Format i );
			static void ERSQRT ( VU *v, Instruction::Format i );
			static void EATAN ( VU *v, Instruction::Format i );
			static void LQD ( VU *v, Instruction::Format i );
			static void RSQRT ( VU *v, Instruction::Format i );
			static void ILWR ( VU *v, Instruction::Format i );
			static void RINIT ( VU *v, Instruction::Format i );
			static void ELENG ( VU *v, Instruction::Format i );
			static void ESUM ( VU *v, Instruction::Format i );
			static void ERCPR ( VU *v, Instruction::Format i );
			static void EEXP ( VU *v, Instruction::Format i );
			static void SQD ( VU *v, Instruction::Format i );
			static void WAITQ ( VU *v, Instruction::Format i );
			static void ISWR ( VU *v, Instruction::Format i );
			static void RXOR ( VU *v, Instruction::Format i );
			static void ERLENG ( VU *v, Instruction::Format i );
			static void WAITP ( VU *v, Instruction::Format i );
			
			// with the static-analysis, need some functions to be separate for calls from R5900
			static void VMR32 ( VU *v, Instruction::Format i );
			static void VMOVE ( VU *v, Instruction::Format i );

			static void VMTIR ( VU *v, Instruction::Format i );
			static void VIADD ( VU *v, Instruction::Format i );
			static void VISUB ( VU *v, Instruction::Format i );
			static void VIADDI ( VU *v, Instruction::Format i );
			static void VIAND ( VU *v, Instruction::Format i );
			static void VIOR ( VU *v, Instruction::Format i );

			static void VLQI ( VU *v, Instruction::Format i );
			static void VLQD ( VU *v, Instruction::Format i );
			static void VSQI ( VU *v, Instruction::Format i );
			static void VSQD ( VU *v, Instruction::Format i );


			
			inline static void VuUpperOp ( VU *v, Instruction::Format i, OpType1 OpFunc0 )
			{
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->vf [ i.Fd ].fx, v->vf [ i.Fs ].fx, v->vf [ i.Ft ].fx, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->vf [ i.Fd ].fy, v->vf [ i.Fs ].fy, v->vf [ i.Ft ].fy, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->vf [ i.Fd ].fz, v->vf [ i.Fs ].fz,  v->vf [ i.Ft ].fz, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->vf [ i.Fd ].fw, v->vf [ i.Fs ].fw,  v->vf [ i.Ft ].fw, 0 );
				}
				
				
				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}

			inline static void VuUpperOpI ( VU *v, Instruction::Format i, OpType1 OpFunc0 )
			{
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcReg ( i.Value, i.Fs );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->vf [ i.Fd ].fx, v->vf [ i.Fs ].fx, v->vi [ 21 ].f, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->vf [ i.Fd ].fy, v->vf [ i.Fs ].fy, v->vi [ 21 ].f, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->vf [ i.Fd ].fz, v->vf [ i.Fs ].fz, v->vi [ 21 ].f, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->vf [ i.Fd ].fw, v->vf [ i.Fs ].fw, v->vi [ 21 ].f, 0 );
				}
				

				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}

			inline static void VuUpperOpQ ( VU *v, Instruction::Format i, OpType1 OpFunc0 )
			{
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcReg ( i.Value, i.Fs );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );
				
#ifdef ENABLE_NEW_QP_HANDLING
				// if div/unit is in use, check if it is done yet
				v->CheckQ ();
#endif

#endif

				// update q first
				//v->UpdateQ_Micro ();

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->vf [ i.Fd ].fx, v->vf [ i.Fs ].fx, v->vi [ 22 ].f, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->vf [ i.Fd ].fy, v->vf [ i.Fs ].fy, v->vi [ 22 ].f, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->vf [ i.Fd ].fz, v->vf [ i.Fs ].fz, v->vi [ 22 ].f, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->vf [ i.Fd ].fw, v->vf [ i.Fs ].fw, v->vi [ 22 ].f, 0 );
				}
				

				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}

			inline static void VuUpperOpX ( VU *v, Instruction::Format i, OpType1 OpFunc0 )
			{
				float t_fx;
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

				// clear flags
				ClearFlags ( v );
				
				// get x component
				t_fx = v->vf [ i.Ft ].fx;
				
				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->vf [ i.Fd ].fx, v->vf [ i.Fs ].fx, t_fx, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->vf [ i.Fd ].fy, v->vf [ i.Fs ].fy, t_fx, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->vf [ i.Fd ].fz, v->vf [ i.Fs ].fz, t_fx, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->vf [ i.Fd ].fw, v->vf [ i.Fs ].fw, t_fx, 0 );
				}
				

				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}

			inline static void VuUpperOpY ( VU *v, Instruction::Format i, OpType1 OpFunc0 )
			{
				float t_fy;
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

				// clear flags
				ClearFlags ( v );
				
				// get y component
				t_fy = v->vf [ i.Ft ].fy;
				
				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->vf [ i.Fd ].fx, v->vf [ i.Fs ].fx, t_fy, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->vf [ i.Fd ].fy, v->vf [ i.Fs ].fy, t_fy, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->vf [ i.Fd ].fz, v->vf [ i.Fs ].fz, t_fy, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->vf [ i.Fd ].fw, v->vf [ i.Fs ].fw, t_fy, 0 );
				}
				

				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}

			inline static void VuUpperOpZ ( VU *v, Instruction::Format i, OpType1 OpFunc0 )
			{
				float t_fz;
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

				// clear flags
				ClearFlags ( v );
				
				// get z component
				t_fz = v->vf [ i.Ft ].fz;
				
				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->vf [ i.Fd ].fx, v->vf [ i.Fs ].fx, t_fz, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->vf [ i.Fd ].fy, v->vf [ i.Fs ].fy, t_fz, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->vf [ i.Fd ].fz, v->vf [ i.Fs ].fz, t_fz, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->vf [ i.Fd ].fw, v->vf [ i.Fs ].fw, t_fz, 0 );
				}
				

				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}

			inline static void VuUpperOpW ( VU *v, Instruction::Format i, OpType1 OpFunc0 )
			{
				float t_fw;
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

				// clear flags
				ClearFlags ( v );
				
				// get z component
				t_fw = v->vf [ i.Ft ].fw;
				
				if ( i.destx )
				{
					v->vf [ i.Fd ].fx = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->vf [ i.Fd ].fx, v->vf [ i.Fs ].fx, t_fw, 3 );
				}
				
				if ( i.desty )
				{
					v->vf [ i.Fd ].fy = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->vf [ i.Fd ].fy, v->vf [ i.Fs ].fy, t_fw, 2 );
				}
				
				if ( i.destz )
				{
					v->vf [ i.Fd ].fz = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->vf [ i.Fd ].fz, v->vf [ i.Fs ].fz, t_fw, 1 );
				}
				
				if ( i.destw )
				{
					v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->vf [ i.Fd ].fw, v->vf [ i.Fs ].fw, t_fw, 0 );
				}
				

				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}
			
// ------------------------------------------------------

			
			inline static void VuUpperOp_A ( VU *v, Instruction::Format i, OpType1A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->dACC [ 0 ].f, v->vf [ i.Fs ].fx,  v->vf [ i.Ft ].fx, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->dACC [ 1 ].f, v->vf [ i.Fs ].fy,  v->vf [ i.Ft ].fy, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->dACC [ 2 ].f, v->vf [ i.Fs ].fz,  v->vf [ i.Ft ].fz, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->dACC [ 3 ].f, v->vf [ i.Fs ].fw,  v->vf [ i.Ft ].fw, 0 );
				}
				
			}

			inline static void VuUpperOpI_A ( VU *v, Instruction::Format i, OpType1A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcReg ( i.Value, i.Fs );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->dACC [ 0 ].f, v->vf [ i.Fs ].fx, v->vi [ 21 ].f, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->dACC [ 1 ].f, v->vf [ i.Fs ].fy, v->vi [ 21 ].f, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->dACC [ 2 ].f, v->vf [ i.Fs ].fz, v->vi [ 21 ].f, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->dACC [ 3 ].f, v->vf [ i.Fs ].fw, v->vi [ 21 ].f, 0 );
				}
				
			}

			inline static void VuUpperOpQ_A ( VU *v, Instruction::Format i, OpType1A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcReg ( i.Value, i.Fs );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
				
#ifdef ENABLE_NEW_QP_HANDLING
				// if div/unit is in use, check if it is done yet
				v->CheckQ ();
#endif

#endif

				// update q first
				//v->UpdateQ_Micro ();

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->dACC [ 0 ].f, v->vf [ i.Fs ].fx, v->vi [ 22 ].f, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->dACC [ 1 ].f, v->vf [ i.Fs ].fy, v->vi [ 22 ].f, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->dACC [ 2 ].f, v->vf [ i.Fs ].fz, v->vi [ 22 ].f, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->dACC [ 3 ].f, v->vf [ i.Fs ].fw, v->vi [ 22 ].f, 0 );
				}
				
			}

			inline static void VuUpperOpX_A ( VU *v, Instruction::Format i, OpType1A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->dACC [ 0 ].f, v->vf [ i.Fs ].fx, v->vf [ i.Ft ].fx, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->dACC [ 1 ].f, v->vf [ i.Fs ].fy, v->vf [ i.Ft ].fx, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->dACC [ 2 ].f, v->vf [ i.Fs ].fz, v->vf [ i.Ft ].fx, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->dACC [ 3 ].f, v->vf [ i.Fs ].fw, v->vf [ i.Ft ].fx, 0 );
				}
				
			}

			inline static void VuUpperOpY_A ( VU *v, Instruction::Format i, OpType1A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->dACC [ 0 ].f, v->vf [ i.Fs ].fx, v->vf [ i.Ft ].fy, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->dACC [ 1 ].f, v->vf [ i.Fs ].fy, v->vf [ i.Ft ].fy, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->dACC [ 2 ].f, v->vf [ i.Fs ].fz, v->vf [ i.Ft ].fy, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->dACC [ 3 ].f, v->vf [ i.Fs ].fw, v->vf [ i.Ft ].fy, 0 );
				}
				
			}

			inline static void VuUpperOpZ_A ( VU *v, Instruction::Format i, OpType1A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->dACC [ 0 ].f, v->vf [ i.Fs ].fx, v->vf [ i.Ft ].fz, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->dACC [ 1 ].f, v->vf [ i.Fs ].fy, v->vf [ i.Ft ].fz, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->dACC [ 2 ].f, v->vf [ i.Fs ].fz, v->vf [ i.Ft ].fz, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->dACC [ 3 ].f, v->vf [ i.Fs ].fw, v->vf [ i.Ft ].fz, 0 );
				}
				
			}

			inline static void VuUpperOpW_A ( VU *v, Instruction::Format i, OpType1A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcRegsBC ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->dACC [ 0 ].f, v->vf [ i.Fs ].fx, v->vf [ i.Ft ].fw, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->dACC [ 1 ].f, v->vf [ i.Fs ].fy, v->vf [ i.Ft ].fw, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->dACC [ 2 ].f, v->vf [ i.Fs ].fz, v->vf [ i.Ft ].fw, 1 );
				}
				
				if ( i.destw )
				{
					v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->dACC [ 3 ].f, v->vf [ i.Fs ].fw, v->vf [ i.Ft ].fw, 0 );
				}
				
			}
			
			
			
			// OPMULA and OPMSUB //
			
			inline static void VuUpperOp_OPMULA ( VU *v, Instruction::Format i, OpType0A OpFunc0 )
			{
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// note: destination here is accumulator, so do not set a destination register
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					v->dACC [ 0 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fy,  v->vf [ i.Ft ].fz, 3 );
				}
				
				if ( i.desty )
				{
					v->dACC [ 1 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fz,  v->vf [ i.Ft ].fx, 2 );
				}
				
				if ( i.destz )
				{
					v->dACC [ 2 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fx,  v->vf [ i.Ft ].fy, 1 );
				}
				
				//if ( i.destw )
				//{
				//	v->dACC [ 3 ].f = OpFunc0_st ( OpFunc0, v, v->vf [ i.Fs ].fw,  v->vf [ i.Ft ].fw, 0 );
				//}
				
			}
			
			inline static void VuUpperOp_MSUB ( VU *v, Instruction::Format i, OpType1 OpFunc0 )
			{
				// must store result in intermediate variables before saving into registers
				//double fd_x, fd_y, fd_z;
				float fd_x, fd_y, fd_z;
				
#ifdef ENABLE_STALLS
				// set the source register(s)
				v->Set_SrcRegs ( i.Value, i.Fs, i.Ft );
				
				// make sure the source registers are available
				if ( v->TestStall () )
				{
#ifdef INLINE_DEBUG_STALLS
	debug << " STALL ";
#endif
					// FMAC pipeline stall //
					v->PipelineWait_FMAC ();
				}
				
				// todo: can set the MAC and STATUS flags registers as being modified also if needed later
				// set the destination register(s)
				// note: can only set this once the pipeline is not stalled since it modifies the pipeline stage bitmap
				v->Set_DestReg_Upper ( i.Value, i.Fd );
#endif

				// clear flags
				ClearFlags ( v );
				
				if ( i.destx )
				{
					fd_x = OpFunc0_st ( OpFunc0, v, v->dACC [ 0 ].f, v->vf [ i.Fd ].fx, v->vf [ i.Fs ].fy, v->vf [ i.Ft ].fz, 3 );
				}
				
				if ( i.desty )
				{
					fd_y = OpFunc0_st ( OpFunc0, v, v->dACC [ 1 ].f, v->vf [ i.Fd ].fy, v->vf [ i.Fs ].fz, v->vf [ i.Ft ].fx, 2 );
				}
				
				if ( i.destz )
				{
					fd_z = OpFunc0_st ( OpFunc0, v, v->dACC [ 2 ].f, v->vf [ i.Fd ].fz, v->vf [ i.Fs ].fx,  v->vf [ i.Ft ].fy, 1 );
				}
				
				//if ( i.destw )
				//{
				//	v->vf [ i.Fd ].fw = OpFunc0_st ( OpFunc0, v, v->dACC [ 3 ].f, v->vf [ i.Fd ].fw, v->vf [ i.Fs ].fw,  v->vf [ i.Ft ].fw, 0 );
				//}
				
				// now can store result without possibly corrupting registers
				v->vf [ i.Fd ].fx = fd_x;
				v->vf [ i.Fd ].fy = fd_y;
				v->vf [ i.Fd ].fz = fd_z;

				// the accompanying lower instruction can't modify the same register
				v->LastModifiedRegister = i.Fd;
				
			}
			

			
		};
		
	};
	
};


#endif


