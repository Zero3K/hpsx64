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



#ifndef _PS2_SIO_H_
#define _PS2_SIO_H_

#include "types.h"
#include "Debug.h"

//#include "WinJoy.h"

//#include "PS1_Intc.h"

//#include <stdio.h>
//#include <sys/stat.h>


namespace Playstation2
{

	class SIO
	{
	
		static Debug::Log debug;
		
	public:
	
		static SIO *_SIO;
		
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the dma registers start at
		static const long Regs_Start = 0x1000f100;
		
		// where the dma registers end at
		static const long Regs_End = 0x1000f1c0;
	
		// distance between numbered groups of registers for dma
		static const long Reg_Size = 0x4;
		
		
		// index for the next event
		u32 NextEvent_Idx;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;
		
		void GetNextEvent ();
		void SetNextEvent ( u64 Cycle );
		void Set_NextEventCycle ( u64 Cycle );
		void Update_NextEventCycle ();
		
		static u64 Read ( u32 Address, u64 Mask );
		static void Write ( u32 Address, u64 Data, u64 Mask );
		
		void Reset ();
		
		void Start ();
		
		// returns interrupt signal
		void Run ();
		
		

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////

		// SIO LCR - Line Control Register
		static const long SIO_LCR = 0x1000f100;
		
		// SIO LSR - Line Status Register
		static const long SIO_LSR = 0x1000f110;
		
		// SIO IER - Interrupt Enable Register
		static const long SIO_IER = 0x1000f120;
		
		// SIO ISR - Interrupt Status Register
		static const long SIO_ISR = 0x1000f130;
		
		// SIO FCR - FIFO Control Register
		static const long SIO_FCR = 0x1000f140;

		// SIO BGR - Baud Rate Control Register
		static const long SIO_BGR = 0x1000f150;
		
		// SIO TX FIFO - Transmit FIFO Register
		static const long SIO_TXFIFO = 0x1000f180;
		
		// SIO RX FIFO - Receive FIFO Register
		static const long SIO_RXFIFO = 0x1000f1c0;
		
		
		// constants for SIO FCR
		
		// FIFO Reset Enable
		static const long SIO_FCR_FRSTE = 0x1;
		
		// RX FIFO Reset
		static const long SIO_FCR_RFRST = 0x2;
		
		// TX FIFO Reset
		static const long SIO_FCR_TFRST = 0x4;
		
		

		// constructor
		SIO ();
		
		// Debug
		static u32 *_DebugPC;
		static u64 *_DebugCycleCount;
		static u32* _NextEventIdx;
		
		
		
		
		

		void Update_PreScaler ();
		void Update_WaitCycles ();

		
		static void sRun () { _SIO->Run (); }
		static void Set_EventCallback ( funcVoid2 UpdateEvent_CB ) { _SIO->NextEvent_Idx = UpdateEvent_CB ( sRun ); };
		
		

		static u64* _NextSystemEvent;
		

		static const u32 c_InterruptBit_SIO = 12;
		
		
		static u32* _Intc_Stat;
		static u32* _Intc_Mask;
		static u32* _R5900_Status_12;
		static u32* _R5900_Cause_13;
		static u64* _ProcStatus;
		
		inline void ConnectInterrupt ( u32* _IStat, u32* _IMask, u32* _R5900_Status, u32* _R5900_Cause, u64* _ProcStat )
		{
			_Intc_Stat = _IStat;
			_Intc_Mask = _IMask;
			_R5900_Cause_13 = _R5900_Cause;
			_R5900_Status_12 = _R5900_Status;
			_ProcStatus = _ProcStat;
		}
		
		
		
		
		inline void SetInterrupt_SIO ()
		{
			//*_Intc_Stat |= ( 1 << c_InterruptBit_SIO );
			//if ( *_Intc_Stat & *_Intc_Mask ) *_R5900_Cause_13 |= ( 1 << 10 );
			
			//if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
		}
		
		inline void ClearInterrupt_SIO ()
		{
			//*_Intc_Stat &= ~( 1 << c_InterruptBit_SIO );
			//if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R5900_Cause_13 &= ~( 1 << 10 );
			
			//if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
		}
		
		
		
	};
	
};




#endif

