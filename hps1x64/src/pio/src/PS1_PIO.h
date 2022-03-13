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


#ifndef _PS1_PIO_H_
#define _PS1_PIO_H_

#include "types.h"
#include "Debug.h"

#include "PS1_Intc.h"

namespace Playstation1
{

	class PIO
	{
	
		static Debug::Log debug;
		static PIO *_PIO;
	
	public:
	
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the dma registers start at
		static const long Regs_Start = 0x1f801040;
		
		// where the dma registers end at
		static const long Regs_End = 0x1f80104e;
	
		// distance between numbered groups of registers
		static const long Reg_Size = 0x4;
		
		// index of next event
		u32 NextEvent_Idx;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;
		
		static u32 Read ( u32 Address );
		static void Write ( u32 Address, u32 Data, u32 Mask );
		void DMA_Read ( u32* Data, int ByteReadCount );
		void DMA_Write ( u32* Data, int ByteWriteCount );
		
		void Start ();
		
		void Reset ();
		
		//u32 Run ();
		void Run ();

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////

		// PIO data
		static const long PIO_DATA = 0x1f801040;
		
		// PIO status
		static const long PIO_STAT = 0x1f801044;
		
		// PIO mode
		static const long PIO_MODE = 0x1f801048;
		
		// PIO control
		static const long PIO_CTRL = 0x1f80104a;
		
		// PIO baud
		static const long PIO_BAUD = 0x1f80104e;
		
		// constructor
		PIO ();

		
		static void sRun () { _PIO->Run (); }
		static void Set_EventCallback ( funcVoid2 UpdateEvent_CB ) { _PIO->NextEvent_Idx = UpdateEvent_CB ( sRun ); };

		
		// for interrupt call back
		static funcVoid UpdateInterrupts;
		static void Set_IntCallback ( funcVoid UpdateInt_CB ) { UpdateInterrupts = UpdateInt_CB; };
		
		
		
		static const u32 c_InterruptBit = 10;
		
		//static u32* _Intc_Master;
		static u32* _Intc_Stat;
		static u32* _Intc_Mask;
		static u32* _R3000A_Status_12;
		static u32* _R3000A_Cause_13;
		static u64* _ProcStatus;
		
		inline void ConnectInterrupt ( u32* _IStat, u32* _IMask, u32* _R3000A_Status, u32* _R3000A_Cause, u64* _ProcStat )
		{
			_Intc_Stat = _IStat;
			_Intc_Mask = _IMask;
			//_Intc_Master = _IMaster;
			_R3000A_Cause_13 = _R3000A_Cause;
			_R3000A_Status_12 = _R3000A_Status;
			_ProcStatus = _ProcStat;
		}
		
		
		inline void SetInterrupt ()
		{
			//*_Intc_Master |= ( 1 << c_InterruptBit );
			*_Intc_Stat |= ( 1 << c_InterruptBit );
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}
		
		inline void ClearInterrupt ()
		{
			//*_Intc_Master &= ~( 1 << c_InterruptBit );
			*_Intc_Stat &= ~( 1 << c_InterruptBit );
			
			UpdateInterrupts ();
			
			/*
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}
		
		static u64* _NextSystemEvent;
		
		// set offset to next event
		inline void Set_NextEvent ( u64 Cycles )
		{
			NextEvent_Cycle = Cycles + *_DebugCycleCount;
			//if ( NextEvent_Cycle > *_SystemCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_SystemCycleCount ) ) *_NextSystemEvent = NextEvent_Cycle;
			if ( NextEvent_Cycle < *_NextSystemEvent )
			{
				*_NextSystemEvent = NextEvent_Cycle;
				*_NextEventIdx = NextEvent_Idx;
			}
		}
		
		// set cycle# of next event
		inline void Set_NextEventCycle ( u64 Cycle )
		{
			NextEvent_Cycle = Cycle;
			if ( NextEvent_Cycle < *_NextSystemEvent )
			{
				*_NextSystemEvent = NextEvent_Cycle;
				*_NextEventIdx = NextEvent_Idx;
			}
		}
		
		///////////////////////////////////
		// Debug
		static u32 *_DebugPC;
		static u64 *_DebugCycleCount;
		static u64 *_SystemCycleCount;
		static u32 *_NextEventIdx;
		
		// I'll make this variable standard on all objects
		s64 BusyCycles;
		
		
		
		
	};
	
};

#endif

