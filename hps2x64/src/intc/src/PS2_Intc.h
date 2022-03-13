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



#ifndef _PS2_INTC_H_
#define _PS2_INTC_H_

#include "types.h"
#include "Debug.h"
#include "DebugValueList.h"

namespace Playstation2
{

	class Intc
	{
		static Debug::Log debug;
	
	public:
	
		static Intc *_INTC;
		
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the dma registers start at
		static const u32 Regs_Start = 0x1f801070;
		
		// where the dma registers end at
		static const u32 Regs_End = 0x1f801074;
	
		// distance between numbered groups of registers for dma
		static const u32 Reg_Size = 0x4;
		
		struct HW_Register
		{
			bool ReadOK;
			bool WriteOK;
			bool Unknown;
			char* Name;
			u32 Address;
			u32 SizeInBytes;
			u32* DataPtr;
		};
		
		HW_Register Registers [ Regs_End - Regs_Start + Reg_Size ];
		
		void Start ();
		
		void Reset ();
		
		static u32* _R5900_Status_12;
		static u32* _R5900_Cause_13;
		static u64* _ProcStatus;
		
		static u32* _ulIdle;

		// cycle that the next event will happen at for this device
		// this will not actually be used here
		u64 NextEvent_Cycle;
		
		
		
		inline void ConnectInterrupt ( u32* _R5900_Status, u32* _R5900_Cause, u64* _ProcStat )
		{
			_R5900_Cause_13 = _R5900_Cause;
			_R5900_Status_12 = _R5900_Status;
			_ProcStatus = _ProcStat;
		}

		static u64 Read ( u32 Address, u64 Mask );
		static void Write ( u32 Address, u64 Data, u64 Mask );

		/////////////////////////////////////////////////////////////////
		// has to interrupt the processor when there is an interrupt
		u32 Run ( u32 Interrupt );
		
		// updates interrupts
		void UpdateInts ();
		
		// master interrupt line
		// interrupt is edge triggered, so don't use this
		//u32 I_MASTER_Reg;

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////

		// Interrupt Acknowledge register - r/w
		static const u32 I_STAT = 0x1000f000;
		
		union I_STAT_Format
		{
			struct
			{
				// bit 0 - interrupt acknowledge bit for vblank interrupt - 1 = on
				// to acknowledge an interrupt, write a zero to the bit, and a 1 to all other bits
				// so to acknowledge a VBlank interrupt, you would write 0xffff fffe
				// if bit is high, it means that interrupt has been signalled
				
				// bit 0
				u32 GPUInterruptAcknowledge : 1;
				
				// bit 1
				u32 SBUSInterruptAcknowledge : 1;
				
				// bit 2
				u32 VBlankStartInterruptAcknowledge : 1;
				
				// bit 3
				u32 VBlankEndInterruptAcknowledge : 1;
				
				// bit 4
				u32 VIF0InterruptAcknowledge : 1;
				
				// bit 5
				u32 VIF1InterruptAcknowledge : 1;
				
				// bit 6
				u32 VU0InterruptAcknowledge : 1;
				
				// bit 7
				u32 VU1InterruptAcknowledge : 1;
				
				// bit 8
				u32 IPUInterruptAcknowledge : 1;
				
				// bit 9
				u32 TIMER0InterruptAcknowledge : 1;
				
				// bit 10
				u32 TIMER1InterruptAcknowledge : 1;
				
				// bit 11
				u32 TIMER2InterruptAcknowledge : 1;
				
				// bit 12
				u32 TIMER3InterruptAcknowledge : 1;
				
				// bit 13
				u32 SFIFOInterruptAcknowledge : 1;
				
				// bit 14
				u32 VU0WTDGInterruptAcknowledge : 1;
			};
			
			u32 Value;
		};
		
		I_STAT_Format I_STAT_Reg;
	
		// Interrupt Mask Register - r/w
		static const u32 I_MASK = 0x1000f010;
		
		union I_MASK_Format
		{
			struct
			{
				// bit 0 - interrupt mask bit for vblank interrupt - 1 = interrupt is enabled, 0 = interrupt is disabled
				// write 1 to allow the interrupt, write 0 to disable the interrupt
				
				// bit 0
				u32 GPUInterruptMask : 1;
				
				// bit 1
				u32 SBUSInterruptMask : 1;
				
				// bit 2
				u32 VBlankStartInterruptMask : 1;
				
				// bit 3
				u32 VBlankEndInterruptMask : 1;
				
				// bit 4
				u32 VIF0InterruptMask : 1;
				
				// bit 5
				u32 VIF1InterruptMask : 1;
				
				// bit 6
				u32 VU0InterruptMask : 1;
				
				// bit 7
				u32 VU1InterruptMask : 1;
				
				// bit 8
				u32 IPUInterruptMask : 1;
				
				// bit 9
				u32 TIMER0InterruptMask : 1;
				
				// bit 10
				u32 TIMER1InterruptMask : 1;
				
				// bit 11
				u32 TIMER2InterruptMask : 1;
				
				// bit 12
				u32 TIMER3InterruptMask : 1;
				
				// bit 13
				u32 SFIFOInterruptMask : 1;
				
				// bit 14
				u32 VU0WTDGInterruptMask : 1;
			};
			
			u32 Value;
		};

		I_MASK_Format I_MASK_Reg;
		
		// constructor
		Intc ();
		
		// enables interrupts whose bits are set
		void SetInterrupt ( u32 Interrupt );
		
		// clears interrupts whose bits are set
		void ClearInterrupt ( u32 Interrupt );

		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		
		// object debug stuff
		// Enable/Disable debug window for object
		// Windows API specific code is in here
		static bool DebugWindow_Enabled;
		static WindowClass::Window *DebugWindow;
		static DebugValueList<u32> *ValueList;
		static void DebugWindow_Enable ();
		static void DebugWindow_Disable ();
		static void DebugWindow_Update ();

	};
	
};

#endif

