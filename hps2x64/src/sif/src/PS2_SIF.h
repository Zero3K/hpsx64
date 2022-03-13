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



#ifndef _PS2_SIF_H_
#define _PS2_SIF_H_

#include "types.h"
#include "Debug.h"

#include "PS2_Timer.h"

//#include "PS1_Intc.h"

//#include <stdio.h>
//#include <sys/stat.h>





namespace Playstation2
{

	class SIF
	{
	
		static Debug::Log debug;
		
	public:
	
		static SIF *_SIF;
		
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the dma registers start at
		static const long Regs_Start = 0x1000f200;
		
		// where the dma registers end at
		static const long Regs_End = 0x1000f260;
	
		// distance between numbered groups of registers for dma
		static const long Reg_Size = 0x10;
		
		static const char* Reg_Names_EE [ 7 ];
		static const char* Reg_Names_IOP [ 7 ];
		
		// ?? interrupt interval ??
		// specified in EE BUS cycles
		static const u64 c_llIntInterval = 0x80000000ULL;
		
		static const int c_iDebugLines = 20;
		int DebugCount;
		
		// index for the next event
		u32 NextEvent_Idx;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;
		
		void GetNextEvent ();
		void SetNextEvent ( u64 Cycle );
		void Set_NextEventCycle ( u64 Cycle );
		void Update_NextEventCycle ();
		
		static u64 EE_Read ( u32 Address, u64 Mask );
		static void EE_Write ( u32 Address, u64 Data, u64 Mask );
		static u32 IOP_Read ( u32 Address );
		static void IOP_Write ( u32 Address, u32 Data, u32 Mask );
		
		
		// dma reading/writing from/to peripheral //
		// readblock functions should return amount of data read
		
		static void EE_DMA_ReadBlock ();
		
		// returns the number of quadwords written to SIF by EE
		static u32 EE_DMA_WriteBlock ( u64* Data, u32 Count );
		
		static u32 IOP_DMA_ReadBlock ( u32 *pMemory, u32 Address, u32 WordCount );
		
		// returns 1 if transfer is complete on ps2 side, zero otherwise
		static u32 IOP_DMA_WriteBlock ( u32 *pMemory, u32 Address, u32 WordCount );

		
		static void SuspendTransfer_IOP ( int channel );
		static void SuspendTransfer_EE ( int channel );
		
		
		// updates the active channel on the ps1 side
		static void Update_ActiveChannel ();

		
		// used on the IOP side to determine when SIF is ready
		static u64 EE_DMA_In_Ready ();
		static u64 EE_DMA_Out_Ready ();
		
		// used on the EE side to determine when SIF is ready
		static bool IOP_DMA_In_Ready ();
		static bool IOP_DMA_Out_Ready ();
		
		static u64 EE_To_IOP_DMA_Ready ();
		static u64 IOP_To_EE_DMA_Ready ();

		static bool EE_To_IOP_DMA_Ready_FromEE ();
		static bool IOP_To_EE_DMA_Ready_FromEE ();
		
		static void Check_TransferToIOP ();
		static void Check_TransferFromIOP ();
		
		void Reset ();
		
		void Start ();

		void Run ();
		
		

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////

		//u32 lSBUS_F200, lSBUS_F210, lSBUS_F220, lSBUS_F230, lSBUS_F240, lSBUS_F260;
		
		union RegData
		{
			struct
			{
				u32 F200;
				u32 F210;
				u32 F220;
				u32 F230;
				u32 F240;
				u32 F250;
				u32 F260;
				u32 F270;
				u32 F280;
				u32 F290;
				u32 F2A0;
				u32 F2B0;
				u32 F2C0;
				u32 F2D0;
				u32 F2E0;
				u32 F2F0;
			};
			
			u32 Regs [ 0x10 ];
		};
		
		RegData SIFRegs;
		static RegData *pRegData;
		
		static const int c_iMaxBufferSize_Bytes = 128;
		static const int c_iMaxBufferSizeQWC = 8;
		
		u32 ulBufferDirection;	enum { BUFFER_NONE = 0, BUFFER_SIF0_IOP_TO_EE, BUFFER_SIF1_EE_TO_IOP };
		u32 ulBufferSizeQWC;
		u64 ullBuffer [ c_iMaxBufferSizeQWC * 2 ];
		
		// the SIF is busy until the transfer completes
		
		// in units of IOP Bus cycles (1/4 EE BUS cycles)
		u64 IOP_BusyUntil_Cycle;
		
		// in units of EE Bus cycles
		u64 EE_BusyUntil_Cycle;
		
		// probably need to have the entire thing busy
		u64 BusyUntil_Cycle;
		
		// this allows the busy until cycles to be set for the SIF interface between PS1 and PS2
		// IOP_BusyUntil sets in units of IOP cycles, and EE_BusyUntil sets in units of EE cycles
		inline void IOP_BusyUntil ( u64 Cycle ) { BusyUntil_Cycle = ( Cycle << 2 ); }
		inline void EE_BusyUntil ( u64 Cycle ) { BusyUntil_Cycle = Cycle; }
		
		// EE SIF Registers //
		
		// SIF EE Data Out/IOP Data In (transfers data from EE->IOP)
		static const long EESIF_F200 = 0x1000f200;
		
		// SIF EE Data In/IOP Data Out (transfers data from IOP->EE)
		static const long EESIF_F210 = 0x1000f210;
		
		// SIF EE Flags Out - bits get set from EE side on write and cleared from IOP side on write
		static const long EESIF_F220 = 0x1000f220;
		
		// SIF IOP Flags Out - bits get set from IOP side on write and cleared from EE side on write
		static const long EESIF_F230 = 0x1000f230;

		// SIF CTRL - Control Register
		static const long EESIF_CTRL = 0x1000f240;
		
		// SIF F260 - F260 Register
		static const long EESIF_F260 = 0x1000f260;
		
		
		// IOP SIF Registers //

		// SIF EE Data Out/IOP Data In (transfers data from EE->IOP)
		// EE side buffer address for SIF transfers
		static const long IOPSIF_F200 = 0x1d000000;
		
		// SIF EE Data In/IOP Data Out (transfers data from IOP->EE)
		// IOP side buffer address for SIF transfers
		static const long IOPSIF_F210 = 0x1d000010;
		
		// SIF EE Flags Out - bits get set from EE side on write and cleared from IOP side on write
		static const long IOPSIF_F220 = 0x1d000020;
		
		// SIF IOP Flags Out - bits get set from IOP side on write and cleared from EE side on write
		static const long IOPSIF_F230 = 0x1d000030;

		// SIF CTRL - Control Register
		static const long IOPSIF_CTRL = 0x1d000040;
		
		// SIF F260 - F260 Register
		static const long IOPSIF_F260 = 0x1d000060;
		


		// constructor
		SIF ();
		
		// Debug
		static u32 *_DebugPC;
		static u64 *_DebugCycleCount;
		static u32* _NextEventIdx;
		
		
		static void sRun () { _SIF->Run (); }
		static void Set_EventCallback ( funcVoid2 UpdateEvent_CB ) { _SIF->NextEvent_Idx = UpdateEvent_CB ( sRun ); };
		
		

		static u64* _NextSystemEvent;
		
		// these are the bits to set in INTC STAT for SIF interrupt on IOP/EE
		// ***todo*** figure out sif interrupt bit for IOP
		static const u32 c_IOP_InterruptBit_SIF = 11;
		static const u32 c_EE_InterruptBit_SIF = 1;
		
		// these are the bits to set in Cause Register for SIF interrupt on IOP/EE
		// ***todo*** figure out which cause bit gets set for EE
		static const u32 c_IOP_CauseBit_SIF = 10;
		static const u32 c_EE_CauseBit_SIF = 10;
		
		// these are the bits to set in ProcStatus for SIF interrupt on IOP/EE
		static const u32 c_IOP_ProcStatusBit_SIF = 20;
		static const u32 c_EE_ProcStatusBit_SIF = 0;
		
		
		static u32* _R3000A_Intc_Stat;
		static u32* _R3000A_Intc_Mask;
		static u32* _R3000A_Status_12;
		static u32* _R3000A_Cause_13;
		static u64* _R3000A_ProcStatus;
		static u32* _R5900_Intc_Stat;
		static u32* _R5900_Intc_Mask;
		static u32* _R5900_Status_12;
		static u32* _R5900_Cause_13;
		static u64* _R5900_ProcStatus;
		
		inline void ConnectInterrupt ( u32* _R3000A_IStat, u32* _R3000A_IMask, u32* _R5900_IStat, u32* _R5900_IMask, u32* _R3000A_Status, u32* _R3000A_Cause, u64* _R3000A_ProcStat, u32* _R5900_Status, u32* _R5900_Cause, u64* _R5900_ProcStat )
		{
			_R3000A_Intc_Stat = _R3000A_IStat;
			_R3000A_Intc_Mask = _R3000A_IMask;
			_R3000A_Cause_13 = _R3000A_Cause;
			_R3000A_Status_12 = _R3000A_Status;
			_R3000A_ProcStatus = _R3000A_ProcStat;
			_R5900_Intc_Stat = _R5900_IStat;
			_R5900_Intc_Mask = _R5900_IMask;
			_R5900_Cause_13 = _R5900_Cause;
			_R5900_Status_12 = _R5900_Status;
			_R5900_ProcStatus = _R5900_ProcStat;
		}
		
		
		
		
		inline static void SetInterrupt_IOP_SIF ()
		{
			*_R3000A_Intc_Stat |= ( 1 << c_IOP_InterruptBit_SIF );
			if ( *_R3000A_Intc_Stat & *_R3000A_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << c_IOP_CauseBit_SIF );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_R3000A_ProcStatus |= ( 1 << c_IOP_ProcStatusBit_SIF );
		}
		
		inline static void SetInterrupt_EE_SIF ()
		{
			*_R5900_Intc_Stat |= ( 1 << c_EE_InterruptBit_SIF );
			if ( *_R5900_Intc_Stat & *_R5900_Intc_Mask ) *_R5900_Cause_13 |= ( 1 << c_EE_CauseBit_SIF );
			
			if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0x8c00 ) && ( *_R5900_Status_12 & 1 ) && ( *_R5900_Status_12 & 0x10000 ) && !( *_R5900_Status_12 & 0x6 )  )
			{
				*_R5900_ProcStatus |= ( 1 << c_EE_ProcStatusBit_SIF );
			}
			
			// ***TODO*** trigger of SBUS interrupt also records ps2 timer values into hold registers (timers 0/1 only?)
			Timers::Perform_TimerHold ();
		}
		
		
		//inline void ClearInterrupt_SIO ()
		//{
			//*_Intc_Stat &= ~( 1 << c_InterruptBit_SIO );
			//if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R5900_Cause_13 &= ~( 1 << 10 );
			
			//if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
		//}
		
		
		
	};
	
};




#endif

