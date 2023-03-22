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



#ifndef _PS1_INTC_H_
#define _PS1_INTC_H_

#include "types.h"
#include "Debug.h"
#include "DebugValueList.h"


#ifdef PS2_COMPILE

#include "PS2_SIF.h"

#endif


namespace Playstation1
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
		
		static u32* _R3000A_Status_12;
		static u32* _R3000A_Cause_13;
		static u64* _ProcStatus;
		
		static u32* _SBus_F230;
		
		inline static void Set_SBusF230 ( u32* _SBus_F230_in_ ) { _SBus_F230 = _SBus_F230_in_; }

		// cycle that the next event will happen at for this device
		// this will not actually be used here
		u64 NextEvent_Cycle;
		
		inline void ConnectInterrupt ( u32* _R3000A_Status, u32* _R3000A_Cause, u64* _ProcStat )
		{
			_R3000A_Cause_13 = _R3000A_Cause;
			_R3000A_Status_12 = _R3000A_Status;
			_ProcStatus = _ProcStat;
		}

		static u32 Read ( u32 Address );
		static void Write ( u32 Address, u32 Data, u32 Mask );

		/////////////////////////////////////////////////////////////////
		// has to interrupt the processor when there is an interrupt
		u32 Run ( u32 Interrupt );
		
		// updates interrupts
		static void UpdateInts ();
		

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////


		// master interrupt line
		// interrupt is edge triggered, so don't use this
		//u32 I_MASTER_Reg;
		// need a master interrupt register ??
		u32 uiIntcMaster;



		// Interrupt Acknowledge register - r/w
		static const u32 I_STAT = 0x1f801070;
		
		union I_STAT_Format
		{
			struct
			{
				// bit 0 - interrupt acknowledge bit for vblank interrupt - 1 = on
				// to acknowledge an interrupt, write a zero to the bit, and a 1 to all other bits
				// so to acknowledge a VBlank interrupt, you would write 0xffff fffe
				// if bit is high, it means that interrupt has been signalled
				
				// bit 0
				u32 VBlankInterruptAcknowledge : 1;
				
				// bit 1
				// for PS2 this might be connected to the SBUS/SIF interrupt
				u32 GPUInterruptAcknowledge : 1;
				
				// bit 2
				u32 CDROMInterruptAcknowledge : 1;
				
				// bit 3
				u32 DMAInterruptAcknowledge : 1;
				
				// bit 4
				u32 Timer0InterruptAcknowledge : 1;
				
				// bit 5
				u32 Timer1InterruptAcknowledge : 1;
				
				// bit 6
				u32 Timer2InterruptAcknowledge : 1;
				
				// bit 7
				u32 ControllerInterruptAcknowledge : 1;
				
				// bit 8
				u32 SIOInterruptAcknowledge : 1;
				
				// bit 9
				u32 SPUInterruptAcknowledge : 1;
				
				// bit 10
				u32 PIOInterruptAcknowledge : 1;
				
#ifdef PS2_COMPILE

				// bit 11 - VBlank End
				u32 Unknown0 : 1;
				
				// bit 12 - DVD?
				u32 Unknown1 : 1;
				
				// dev 9 @ bit 13 ??
				u32 Dev9InterruptAcknowledge : 1;
				
				// timer 3 @ bit 14
				u32 Timer3InterruptAcknowledge : 1;
				
				// timer 4 @ bit 15
				u32 Timer4InterruptAcknowledge : 1;
				
				// timer 5 @ bit 16
				u32 Timer5InterruptAcknowledge : 1;
				
				// sio2 bit 17
				u32 SIO2InterruptAcknowledge : 1;
				
				// bit 18 - HTR0?
				// bit 19 - HTR1?
				// bit 20 - HTR2?
				// bit 21 - HTR3?
				
				// usb bit 22 ??
				// bit 22 - USB
				
				// bit 23 - EXTR
				
				// firmware bit 24?
				// bit 24 - ILINK
				
				// bit 25 - FDMA
				
#endif

			};
			
			u32 Value;
		};
		
		I_STAT_Format I_STAT_Reg;
	
		// Interrupt Mask Register - r/w
		static const u32 I_MASK = 0x1f801074;
		
		union I_MASK_Format
		{
			struct
			{
				// bit 0 - interrupt mask bit for vblank interrupt - 1 = interrupt is enabled, 0 = interrupt is disabled
				// write 1 to allow the interrupt, write 0 to disable the interrupt
				u32 VBlankInterruptMask : 1;
				
				// for PS2 this might be connected to the SBUS/SIF interrupt
				// for PS2 this is SBUS interrupt mask
				u32 GPUInterruptMask : 1;
				
				u32 CDROMInterruptMask : 1;
				u32 DMAInterruptMask : 1;
				u32 Timer0InterruptMask : 1;
				u32 Timer1InterruptMask : 1;
				u32 Timer2InterruptMask : 1;
				
				// SIO 0
				u32 ControllerInterruptMask : 1;
				
				// SIO 1
				u32 SIOInterruptMask : 1;
				u32 SPUInterruptMask : 1;
				u32 PIOInterruptMask : 1;

#ifdef PS2_COMPILE

				// bit 11 - External VBlank
				u32 Unknown0 : 1;
				
				// bit 12 - DVD?? maybe something with the tray ? or reading?
				u32 Unknown1 : 1;
				
				// dev 9 @ bit 13 ??
				u32 Dev9InterruptMask : 1;
				
				// timer 3 @ bit 14
				u32 Timer3InterruptMask : 1;
				
				// timer 4 @ bit 15
				u32 Timer4InterruptMask : 1;
				
				// timer 5 @ bit 16
				u32 Timer5InterruptMask : 1;
				
				// sio2 bit 17
				// SIO 2
				u32 SIO2InterruptMask : 1;
				
				// bit 18 - HTR0?
				// bit 19 - HTR1?
				// bit 20 - HTR2?
				// bit 21 - HTR3?
				
				// usb bit 22 ??
				// bit 22 - USB
				
				// bit 23 - EXTR
				
				// firmware bit 24?
				// bit 24 - ILINK
				
				// bit 25 - FDMA
				
#endif

			};
			
			u32 Value;
		};

		I_MASK_Format I_MASK_Reg;




#ifdef PS2_COMPILE
		static const u32 I_CTRL = 0x1f801078;
		u32 I_CTRL_Reg;

		// previous sbus interrupt state to test for transition from 0->1
		u32 ulPrevSbus;
		
		static u32* _ulIdle;
#endif
		
		// constructor
		//Intc ();
		
		// enables interrupts whose bits are set
		void SetInterrupt ( u32 Interrupt );
		
		// clears interrupts whose bits are set
		void ClearInterrupt ( u32 Interrupt );

		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		static u64* _SystemCycleCount;
		
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

