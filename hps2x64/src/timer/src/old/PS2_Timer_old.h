/*
	Copyright (C) 2012-2016

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


#ifndef _PS2_TIMER_H_
#define _PS2_TIMER_H_


#include "types.h"

#include "Debug.h"

#include "PS1_Intc.h"
#include "PS1_Gpu.h"

#include "DebugValueList.h"


namespace Playstation2
{


	class Timer
	{
	
		//Debug::Log debug;
	
	public:
	
		static int Count;
	
		u32 Number;
		
		union COUNT_Format
		{
			struct
			{
				// this is the current counter value
				// bits 0-15
				u16 Count;
				
				// this is garbage data
				// bits 16-31
				u16 Garbage;
			};
			
			u32 Value;
		};
		
		// the counter value - read only
		COUNT_Format COUNT;

		union MODE_Format
		{
			struct
			{
				// bits 0-1: Clock select
				// 00: Bus clk (147.456+MHZ); 01: 1/16 Bus clk; 10: 1/256 Bus clk; 11: External clk (h-blank)
				u32 ClkSelect : 2;
				
				// bit 2: Enable gate function
				// 0: Gate function disabled; 1: Gate function enabled
				u32 Gate : 1;
				
				// bit 3: select gate
				// 0: h-blank (disabled when ClkSelect is 11); 1: v-blank
				u32 GateSelect : 1;
				
				// bit 4-5: Gate mode
				// 00: Counts while gate signal low; 01: resets and starts counting at gate signal rising edge
				// 10: Resets and starts counting at gate signal falling edge; 11: resets and starts couting at both edges of gate signal
				u32 GateMode : 2;
				
				// bit 6: Zero Return
				// 0: The counter ignores the compare value; 1: counter is reset when it reaches compare value
				u32 CompareEnable : 1;
				
				// bit 7: Counter enable
				// 0: Stops the counter; 1: starts/restarts counter
				u32 CounterEnable : 1;
				
				// bit 8: interrupt on target reached
				// 0: disable interrupt on target reached; 1: interrupt is generated when target is reached
				u32 IrqOnTarget : 1;
				
				// bit 9: interrupt on overflow reached
				// 0: disable interrupt on overflow reached; 1: interrupt is generated when overflow is reached
				u32 IrqOnOverflow : 1;
				
				// bit 10: interrupt on target reached
				// 0: target interrupt not generated; 1: target interrupt generated
				// clear flag by writing 1
				u32 IrqOnTarget_Generated : 1;
				
				// bit 11: interrupt on overflow reached
				// 0: overflow interrupt not generated; 1: overflow interrupt generated
				// clear flag by writing 1
				u32 IrqOnOverflow_Generated : 1;
			};
			
			u32 Value;
		};
		
		// the counter mode - r/w
		MODE_Format MODE;

		
		union COMP_Format
		{
			struct
			{
				// this is the current compare value - bits 0-15
				u16 Compare;
				
				// this is garbage data - bits 16-31
				u16 Garbage;
			};
			
			u32 Value;
		};

		
		// the counter compare value - read and write
		COMP_Format COMP;
		
		// also need to know more stuff about the timer
		
		// timer status - like free run, wait till next hblank, etc
		enum { TIMER_FREERUN = 0, TIMER_SYNC_RUN, TIMER_SYNC_WAIT, TIMER_SYNC_STOPPED };
		u32 TimerStatus;
		
		u64 IRQ_Counter;
		
		// value the timer started at with the cycle number it started at that value
		u64 StartValue;
		u64 StartCycle;
		
		// need to know when the next blanking starts for a timer in sync mode
		u64 NextBlankingStart_Cycle;
		
		// cycles to add before dividing
		double dOffsetCycles;
		
		// these two are in 32.32 fixed point
		// these actually need to be double-precision floating point
		double dCyclesPerTick;
		double dTicksPerCycle;
		
		// this is when the next known event or interrupt happens
		u64 NextEvent_Cycle;
		
		// need this to get sync timer values
		//Timers* t;
		
		// constructor - pass timer number 0-3, and ref to parent object
		Timer ();
		
		// reset the timer - use when creating timer or resetting system
		void Reset ();

		// run clock edge for execute and write
		//Intc::I_STAT_Format Run ();
		
	};

	class Timers
	{
	
		static Debug::Log debug;
	
	public:
		
		static Timers *_TIMERS;
		
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the timer registers start at
		static const u32 Regs_Start = 0x10000000;
		
		// where the timer registers end at
		static const u32 Regs_End = 0x10001830;
	
		// distance between numbered groups of registers
		static const u32 Reg_Size = 0x10;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;


		static const int NumberOfChannels = 4;
		static const int c_iNumberOfChannels = 4;

		
		// need to pack lots of info into the structure for debugging and read/write of hardware registers
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
		
		

		// these allow you to read and write registers and allow the device to act on the read/write event
		static u32 Read ( u32 Address, u32 Mask );
		static void Write ( u32 Address, u32 Data, u32 Mask );

		// update what cycle the next event is at for this device
		void Update_NextEventCycle ();
		void SetNextEventCh_Cycle ( u64 Cycle, u32 Channel );
		void SetNextEventCh ( u64 Cycle, u32 Channel );
		
		void SetTimer ( u32 TimerNumber, u32 TimerValue );
		u32 GetTimerTarget ( u32 TimerNumber );
		u32 GetTimerValue ( u32 TimerNumber );
		void GetNextEvent ( u32 TimerNumber );

		
		u32 Get_NextIntTick ( int TimerNumber, u32 lStartTick );
		double Get_OffsetCycles ( int TimerNumber, u64 lStartCycle );
		u64 Get_FreeRunNextIntCycle ( int TimerNumber, u32 lStartValue, u64 lStartCycle );
		void Get_NextEvent ( int TimerNumber, u64 ThroughCycle );
		
		
		void UpdateTimer ( u32 TimerNumber );
		void Update_FreeRunTimer ( int TimerNumber );
		void UpdateTimer_Wrap ( int TimerNumber );
		
		
		
		
		void CalibrateTimer ( u32 TimerNumber );
		void ReCalibrateTimer ( u32 TimerNumber );
		void SetTimerValue ( u32 TimerNumber, u32 TimerValue );
		
		void SetMode ( u32 TimerNumber, u32 Data );
		void SetValue ( u32 TimerNumber, u32 Data );
		void SetComp ( u32 TimerNumber, u32 Data );
		
		u32 GetMode ( u32 TimerNumber );
		

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////

		// the timer count - read only
		static const u32 COUNT_Base = 0x10000000;
		
		
		// the timer mode - setting this to zero resets a timer - read and write
		static const u32 MODE_Base = 0x10000010;
		
		
		// the timer compare/target value - read and write
		static const u32 COMP_Base = 0x10000020;

		// the timer hold value
		static const u32 HOLD_Base = 0x10000030;
		
		
		static const u32 TIMER0_COUNT = 0x10000000;
		static const u32 TIMER0_MODE = 0x10000010;
		static const u32 TIMER0_COMP = 0x10000020;
		static const u32 TIMER0_HOLD = 0x10000030;

		static const u32 TIMER1_COUNT = 0x10000800;
		static const u32 TIMER1_MODE = 0x10000810;
		static const u32 TIMER1_COMP = 0x10000820;
		static const u32 TIMER1_HOLD = 0x10000830;

		static const u32 TIMER2_COUNT = 0x10001000;
		static const u32 TIMER2_MODE = 0x10001010;
		static const u32 TIMER2_COMP = 0x10001020;

		static const u32 TIMER3_COUNT = 0x10001800;
		static const u32 TIMER3_MODE = 0x10001810;
		static const u32 TIMER3_COMP = 0x10001820;

		
		
		//////////////////////////////////////////
		// Timers are synchronized to:			//
		// Timer 0 - Pixel clock				//
		// Timer 1 - Horizontal Retrace			//
		// Timer 2 - 1/8 System Clock			//
		// Timer 3 - Vertical Retrace			//
		//////////////////////////////////////////

		static Playstation1::GPU *g;
		
		
		/*
		u32 CycleCounter;
		
		// high-accuracy fixed point counters in 1.63 fixed point format
		u64 OneEighthSystemTimer;
		u64 PixelTimer;
		u64 HSyncTimer;
		u64 VSyncTimer;
		
		static const u64 OneEighthSystemTimer_INC = ( ( (u64) 1 ) << 60 );
		u64 PixelTimer_INC;
		u64 HSyncTimer_INC;
		u64 VSyncTimer_INC;
		
		// the actual timers
		Timer Timer0;
		Timer Timer1;
		Timer Timer2;
		Timer Timer3;
		
		GPU::GPU_CTRL_Read_Format GPU_Status_Save;
		
		Intc::I_STAT_Format Interrupt_Status;
		
		// busy cycles
		u32 BusyCycles_VBlank;
		*/
		
		
		Timer TheTimers [ c_iNumberOfChannels ];
		
		
		// Constructor
		Timers ();
		
		void Reset ();
		
		void Start ();
		
		void ConnectDevices ( GPU *_g );

		// run for a clock cycle
		// returns interrupt status - whether to interrupt or not
		// needs GPU Status READ register
		// returns interrupt status for INTC
		void Run ();
		
		static u64* _NextSystemEvent;
		
		static const u32 c_InterruptBit = 4;
		
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
		
		inline void SetInterrupt ( const u32 TimerNumber )
		{
			//*_Intc_Master |= ( 1 << ( c_InterruptBit + TimerNumber ) );
			*_Intc_Stat |= ( 1 << ( c_InterruptBit + TimerNumber ) );
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
		}
		
		inline void ClearInterrupt ( const u32 TimerNumber )
		{
			//*_Intc_Master &= ~( 1 << ( c_InterruptBit + TimerNumber ) );
			*_Intc_Stat &= ~( 1 << ( c_InterruptBit + TimerNumber ) );
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
		}

		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;

		// object debug stuff
		// Enable/Disable debug window for object
		// Windows API specific code is in here
		static bool DebugWindow_Enabled;
		static WindowClass::Window *DebugWindow;
		static DebugValueList<u32> *Timer_ValueList;
		static void DebugWindow_Enable ();
		static void DebugWindow_Disable ();
		static void DebugWindow_Update ();
	};
	
};


#endif

