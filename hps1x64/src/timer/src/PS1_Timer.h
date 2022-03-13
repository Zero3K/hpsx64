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


#ifndef _PS1_TIMER_H_
#define _PS1_TIMER_H_


#include "types.h"

#include "Debug.h"

//#include "PS1_Intc.h"
#include "PS1_Gpu.h"

#include "DebugValueList.h"


#ifdef PS2_COMPILE
#include "PS2_SIF.h"
#endif


#define ENABLE_SBUS_INT_TIMER


// probably not a good idea to do this on PS2, since the templates are needed more for GPU probably
#ifndef PS2_COMPILE

// this should be specified with a compile flag
// enable this to enable using templates for ps1 timers (speed increase, but not good for debugging)
//#define USE_TEMPLATES_PS1_TIMER

#endif


#ifdef USE_TEMPLATES_PS1_TIMER

// the callbacks are due to the templates
#define USE_TIMER_CALLBACKS

#endif


// unsure if the "value reached" flags are actually "interrupt generated" flags or not
//#define SET_REACHED_ONLY_ON_INT

//#define ENABLE_FORCE_FREERUN

namespace Playstation1
{

	

	class Timer
	{
	
		//Debug::Log debug;
	
	public:
	
		// timers 0 and 1 get a signal from the GPU for pixel/hblank
		static GPU *g;
		
		
		static int Count;
	
		int Index;
		u32 TimerNumber;
		
		static Timer* TimerPtrs [ 6 ];
		
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
				// 0 - timer running; 1 - timer stopped (can only be 1 for timer 2)
				// bit 0
				// from Nocash PSX Specification
				// 0: free run; 1: synchronize via bits 1-2
				u32 RunMode : 1;

				// bits 1-2
				// from Nocash PSX Specification
				// Synchronization Mode
				// counter 0: 0-do not count during hblank; 1-reset counter to 0 at hblank;
				// 2-reset to 0 at hblank and do not count outside of hblank; 3-pause until next hblank, then switch to free run
				// counter 1: like counter 0, but vblank instead of hblank
				// counter 2: 0 or 3-stop counter at current value; 1 or 2-free run
				u32 SyncMode : 2;
				
				// 0 - count to 0xffff (including that value); 1 - count to value in target/compare register (including that value)
				// bit 3
				u32 CountToTarget : 1;
				
				// set along with Iq2 for an IRQ when target is reached in counter
				// bit 4
				// 1: Interrupt when counter equals target
				u32 IrqOnTarget : 1;
				
				// bit 5
				// 1: Interrupt when counter equals 0xffff
				u32 IrqOnOverflow : 1;
				
				// set along with Iq1 for an IRQ when target is reached in counter
				// bit 6
				// 0: Irq only once; 1: Irq repeatedly
				u32 IrqMode_Repeat : 1;

				// bit 7
				// 0: pulse mode; 1: toggle mode
				u32 IrqMode_Toggle : 1;
				
				// bits 8-9
				// counter 0: 0 or 2-system clock; 1 or 3-dot clock
				// counter 1: 0 or 2-system clock; 1 or 3-hblank
				// counter 2: 0 or 1-system clock; 2 or 3-system clock/8
				u32 ClockSource : 2;
				
				// bit 10 - Interrupt Request
				// 0: yes; 1: no
				u32 IrqRequest : 1;
				
				// bit 11 - Target reached
				// 0: no; 1: yes
				u32 TargetReached : 1;
				
				// bit 12 - 0xffff reached
				// 0: no; 1: yes
				u32 OverflowReached : 1;

#ifdef PS2_COMPILE

				// bits 13-14 - Clock div for timers 4 and 5 ??
				// 0: count cycles, 1: count every 8 cycles, 2: count every 16 cycles, 3: count every 256 cycles
				u32 ClockDiv : 2;
				
				// bit 15 - zero
				u32 zero0 : 1;
				
#else

				// bits 13-15 - zero
				u32 zero0 : 3;
				
#endif
				
				// garbage data - bits 16-31
				u32 Garbage0 : 16;
			};
			
		
			struct
			{
				// 0 - timer running; 1 - timer stopped (can only be 1 for timer 2)
				// bit 0
				u32 En : 1;

				// bits 1-2
				u32 Zero2 : 2;
				
				// 0 - count to 0xffff; 1 - count to value in target/compare register
				// bit 3
				u32 Tar : 1;
				
				// set along with Iq2 for an IRQ when target is reached in counter
				// bit 4
				u32 Iq1 : 1;
				
				// bit 5
				u32 Zero1 : 1;
				
				// set along with Iq1 for an IRQ when target is reached in counter
				// bit 6
				u32 Iq2 : 1;

				// bit 7
				u32 Zero0 : 1;
				
				// 0 - System Clock (if Div is also zero); 1 - Pixel Clock (timer 0), Horizontal Retrace (timer 1)
				// bit 8
				u32 Clc : 1;
				
				// 0 - System Clock (if Clc is also zero); 1 - 1/8 System Clock (timer 2)
				// bit 9
				u32 Div : 1;
				
				// garbage data - bits 10-31
				u32 Garbage : 22;
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


		typedef void (*UpdateFn) ( void );
		typedef void (*GetNextEventFn) ( void );
		
		// *note* these would have to be reloaded whenever restoring a save state (or possibly not?)
		UpdateFn cbUpdate;
		GetNextEventFn cbGetNextEvent;
		UpdateFn cbUpdate_Sync;
		GetNextEventFn cbGetNextEvent_Sync;

		u32 Force_FreeRun;
		
		// also need to know more stuff about the timer
		
		// timer status - like free run, wait till next hblank, etc
		//enum { TIMER_FREERUN = 0, TIMER_SYNC_RUN, TIMER_SYNC_WAIT, TIMER_SYNC_STOPPED };
		//u32 TimerStatus;
		
		u64 IRQ_Counter;
		
		// value the timer started at with the cycle number it started at that value
		u64 StartValue;
		u64 StartCycle;
		
		u32 NextIntType;
		enum { INT_TYPE_TARGET = 0, INT_TYPE_OVERFLOW = 1 };
		
		// need to know when the next blanking starts for a timer in sync mode
		//u64 NextBlankingStart_Cycle;
		
		// cycles to add before dividing
		double dOffsetCycles;
		
		// these two are in 32.32 fixed point
		// these actually need to be double-precision floating point
		double dCyclesPerTick;
		double dTicksPerCycle;
		
		// this is when the next known event or interrupt happens
		u64 NextEvent_Cycle;
		
		
		// constructor - pass timer number 0-3, and ref to parent object
		Timer ();
		
		// reset the timer - use when creating timer or resetting system
		void Reset ();

		// "TimerIndex" just specifies the index of the timer in the array of timers for the collection of timers it is in
		// for standalone PS1, "TimerNumber" is 0 for pixel timer, 1 for HBlank Timer, 2 for Cycle timer
		void Set_TimerNumber ( int TimerIndex, u32 TimerNumber );
		
		// sets/calculates timer interval
		void CalibrateTimer ();
		
		// this updates the timer value
		void UpdateTimer ();
		
		// this gets the next event cycle for the timer, but may possibly only calculate up until "ThroughCycle"
		void Get_NextEvent ( u64 ThroughCycle );
		
		
		void SetMode ( u32 Data );
		void SetValue ( u32 Data );
		void SetComp ( u32 Data );
		
		
		
		// new stuff
		u64 CountTicks_Scanline ( u64 lBeginCycle, u64 lEndCycle );
		void UpdateTimer_Scanline ();
		void Get_NextEvent_Scanline ();
		u64 Get_InterruptCycle_Scanline ( u64 lBeginCycle, u64 lStartTick, u64 lTargetTick );
		u64 Get_Wrap_Value ();
		
		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		static u64* _SystemCycleCount;

		
	public:
		// update what cycle the next event is at for this device
		void SetNextEventCh_Cycle ( u64 Cycle );
		void SetNextEventCh ( u64 Cycle );
		
		// gets the next tick at which interrupt occurs for specified timer (does not wrap tick value)
		u64 Get_NextIntTick ( u64 lStartTick );
		
		double Get_OffsetCycles ( u64 lStartCycle );
		u64 Get_FreeRunNextIntCycle ( u32 lStartValue, u64 lStartCycle );
		
		void Update_FreeRunTimer ();
		void UpdateTimer_Wrap ();
		
		
public:



template<const u32 TIMERNUMBER, const u32 MODE_CountToTarget, const u32 MODE_IrqOnTarget, const u32 MODE_IrqOnOverflow>
inline u64 Get_NextIntTick_t ( u64 lStartValue )
{
	const u64 OVERFLOW_MASK = ( TIMERNUMBER < 3 ) ? 0xffffULL : 0xffffffffULL;
	
	u64 lIntTick_Target, lIntTick_Overflow, lIntTick;
	u64 lCompareValue;	// , lOverflowValue;
	u64 lWrapValue;
	
	// get the start tick //
	//lStartTick = StartValue & 0xffff;
	
	// initialize value at which interrupt occurs to be max
	// not needed for now
	//lIntTick = 0xffffffffffffffffULL;
	
#ifdef PS2_COMPILE
	if ( TIMERNUMBER < 3 )
	{
#endif

	// get compare value //
	lCompareValue = (u32) COMP.Compare;
	//lOverflowValue = 0x10000ULL;
	
#ifdef PS2_COMPILE
	}
	else
	{
		// get compare value //
		lCompareValue = (u32) COMP.Value;
		//lOverflowValue = 0x100000000ULL;
	}
#endif
	
	// get the compare tick //
	
	// check if counting to target //
	if ( MODE_CountToTarget && ( lStartValue <= lCompareValue ) )
	{
		// count to target //
		//lCompTick = ( (u32) COMP.Compare );
		lWrapValue = lCompareValue + 1;
	}
	else
	{
		// count to overflow //
		//lCompTick = ( (u32) 0xffff );
		lWrapValue = OVERFLOW_MASK + 1;
	}
	
	// get the next int tick //
	
	// set int tick to max
	//lIntTick_Target = 0xffffffff;
	//lIntTick_Target = 0x100000000ULL;
	lIntTick_Target = 0xffffffffffffffffULL;
	
	// check if irq is on target
	if ( MODE_IrqOnTarget )
	{
		// note: can only interrupt on compare if it is non-zero
		if ( lCompareValue )
		{
			// counter can always reach target //
			//lIntTick_Target = ( (u32) COMP.Compare );
			lIntTick_Target = lCompareValue;
			
			// check if start tick is on or after int tick, then add compare value to the end tick plus one
			if ( lStartValue >= lCompareValue )
			{
				// should only be plus one if the target is NOT excluded
				lIntTick_Target += lWrapValue;
			}
		}
	}
	
	// set int tick to max
	//lIntTick_Overflow = 0xffffffff;
	//lIntTick_Overflow = 0x100000000ULL;
	lIntTick_Overflow = 0xffffffffffffffffULL;
	
	// check if irq is on overflow
	if ( MODE_IrqOnOverflow )
	{
		// make sure counter can reach overflow
		//if ( lCompTick == 0xffff )
		//if ( lCompTick == lOverflow )
		if ( lWrapValue > OVERFLOW_MASK )
		{
			//lIntTick_Overflow = lCompTick;
			lIntTick_Overflow = ( OVERFLOW_MASK + 1 );
			
			// check if start tick is on or after end tick, then add compare value to the end tick plus one
			// this condition should never be true
			//if ( lStartTick >= lIntTick_Overflow )
			//{
			//	lIntTick_Overflow += lWrapValue;
			//}
		}
	}
	
	// check which int tick comes next //
	if ( lIntTick_Target < lIntTick_Overflow )
	{
		//NextIntType = INT_TYPE_TARGET;
		lIntTick = lIntTick_Target;
	}
	else
	{
		//NextIntType = INT_TYPE_OVERFLOW;
		lIntTick = lIntTick_Overflow;
	}
	
	return lIntTick;
}



template<const u32 TIMERNUMBER, const u32 MODE_ClockSource
#ifdef PS2_COMPILE
, const u32 MODE_ClockDiv
#endif
>
static inline u64 CountTicks_Scanline_t ( u64 lBeginCycle, u64 lEndCycle )
{
	switch ( TIMERNUMBER )
	{
		case 0:
			// check for clock source
			switch ( MODE_ClockSource )
			{
				case 0:
				//case 2:
					// system clock //
					return lEndCycle - lBeginCycle;
					break;
					
				case 1:
				//case 3:
					// dot clock //
					return ( (u64) ( ( (double) lBeginCycle ) * g->dPixelsPerCycle ) ) - ( (u64) ( ( (double) lEndCycle ) * g->dPixelsPerCycle ) );
					break;
			}
			
			break;
			
		case 1:
#ifdef PS2_COMPILE
		case 3:
#endif
			// *** note: might need to fix access to hblank start for ps2 compile?
			switch ( MODE_ClockSource )
			{
				case 0:
				//case 2:
					// system clock //
					return lEndCycle - lBeginCycle;
					break;
					
				case 1:
				//case 3:
					// hblank clock //
					if ( ( lBeginCycle < g->llHBlankStart ) && ( lEndCycle >= g->llHBlankStart ) )
					{
						return 1;
					}
					
					return 0;
					
					break;
			}
			
			break;
			
		case 2:
			switch ( MODE_ClockSource )
			{
				case 0:
				//case 1:
					// system clock //
					return lEndCycle - lBeginCycle;
					break;
					
				//case 2:
				//case 3:
				case 1:
					// system/8 clock //
					
					return ( ( lEndCycle & ~7 ) - ( lBeginCycle & ~7 ) ) >> 3;
					break;
			}
			
			break;
			
#ifdef PS2_COMPILE
			
		case 4:
		case 5:
			switch ( MODE_ClockDiv )
			{
				case 0:
					// cycle counter //
					return lEndCycle - lBeginCycle;
					break;
					
				case 1:
					// 1/8 cycle counter //
					return ( ( lEndCycle & ~0x7 ) - ( lBeginCycle & ~0x7 ) ) >> 3;
					break;
					
				case 2:
					// 1/16 cycle counter //
					return ( ( lEndCycle & ~0xf ) - ( lBeginCycle & ~0xf ) ) >> 4;
					break;
					
				case 3:
					// 1/256 cycle counter //
					return ( ( lEndCycle & ~0xff ) - ( lBeginCycle & ~0xff ) ) >> 8;
					break;
			}
			
			break;
#endif
	}
}


// get the cycles to get from one tick to another
template<const u32 TIMERNUMBER, const u32 MODE_ClockSource
#ifdef PS2_COMPILE
, const u32 MODE_ClockDiv
#endif
>
static inline u64 Get_InterruptCycle_Scanline_t ( u64 lBeginCycle, u64 lStartTick, u64 lTargetTick )
{
	u64 lReturnCycles;
	u64 lTemp;
	double dTemp;
	
	// the target should come after the starting point
	if ( lTargetTick <= lStartTick ) return -1LL;
	
	switch ( TIMERNUMBER )
	{
		case 0:
			// check for clock source
			switch ( MODE_ClockSource )
			{
				case 0:
				//case 2:
					// system clock //
					return lBeginCycle + ( lTargetTick - lStartTick );
					break;
					
				case 1:
				//case 3:
					// dot clock //
					//return ( (u64) ( ( ( (double) lBeginCycle ) - g->dScanlineCycle ) * dPixelsPerCycle ) ) - ( (u64) ( ( ( (double) lEndCycle ) - g->dScanlineCycle ) * dPixelsPerCycle ) );
					
					// get cycles since last pixel
					
					// ***todo*** can later update pixel timer calculations to get the difference before double floating point multiply
					
					// get start pixel
					dTemp = ( (double) lBeginCycle ) * g->dPixelsPerCycle;
					
					// get where start pixel starts at
					dTemp = ( (double) ( (u64) dTemp ) ) * g->dCyclesPerPixel;
					
					// get the difference from the start cycle
					dTemp = ( (double) lBeginCycle ) - dTemp;
					
					// get number of cycles until interrupt
					dTemp = ( ( lTargetTick - lStartTick ) * g->dCyclesPerPixel ) - dTemp;
					
					lReturnCycles = (u64) dTemp;
					
					if ( dTemp - ( (double) lReturnCycles ) > 0.0L ) lReturnCycles++;
					
					return lBeginCycle + lReturnCycles;
					
					break;
			}
			
			break;
			
		case 1:
#ifdef PS2_COMPILE
		case 3:
#endif
			// *** note: might need to fix access to hblank start for ps2 compile?
			switch ( MODE_ClockSource )
			{
				case 0:
				//case 2:
					// system clock //
					return lBeginCycle + ( lTargetTick - lStartTick );
					break;
					
				case 1:
				//case 3:
					// hblank clock //
					if ( lTargetTick != ( lStartTick + 1 ) )
					{
						// interrupt happens at some unknown point in the future
						return -1LL;
					}
					
					// make sure the starting cycle is before hblank on the scanline
					if ( lBeginCycle >= g->llHBlankStart ) return -1LL;
					
					// interrupt should trigger when hblank happens
					return g->llHBlankStart;
					
					break;
			}
			
			break;
			
		case 2:
			switch ( MODE_ClockSource )
			{
				case 0:
				//case 1:
					// system clock //
					return lBeginCycle + ( lTargetTick - lStartTick );
					break;
					
				//case 2:
				//case 3:
				case 1:
					// system/8 clock //
					
					return lBeginCycle + ( ( ( lTargetTick - lStartTick ) << 3 ) - ( lBeginCycle & 0x7 ) );
					break;
			}
			
			break;
			
#ifdef PS2_COMPILE
			
		case 4:
		case 5:
			switch ( MODE_ClockDiv )
			{
				case 0:
					// cycle counter //
					return lBeginCycle + ( lTargetTick - lStartTick );
					break;
					
				case 1:
					// 1/8 cycle counter //
					return lBeginCycle + ( ( ( lTargetTick - lStartTick ) << 3 ) - ( lBeginCycle & 0x7 ) );
					break;
					
				case 2:
					// 1/16 cycle counter //
					return lBeginCycle + ( ( ( lTargetTick - lStartTick ) << 4 ) - ( lBeginCycle & 0xf ) );
					break;
					
				case 3:
					// 1/256 cycle counter //
					return lBeginCycle + ( ( ( lTargetTick - lStartTick ) << 8 ) - ( lBeginCycle & 0xff ) );
					break;
			}
			
			break;
#endif
	}
}




template<const u32 TIMERNUMBER, const u32 MODE_ClockSource, const u32 MODE_RunMode, const u32 MODE_SyncMode, const u32 MODE_CountToTarget, const u32 MODE_IrqOnTarget, const u32 MODE_IrqOnOverflow
#ifdef PS2_COMPILE
, const u32 MODE_ClockDiv
#endif
>
static void UpdateTimer_Scanline_t ()
{
	const u64 OVERFLOW_MASK = ( TIMERNUMBER < 3 ) ? 0xffffULL : 0xffffffffULL;

	bool bBlank;
	double dOffsetCycles;
	u64 lStartCycle, lEndCycle, lCurrentCycle, lBlankCycle;
	u64 lInitialValue, lCompareValue, lWrapValue;	// , lOverflowValue;
	
	Timer* pt;
	
	// note: timer should be updated before reading value/mode. Should also be updated periodically to maintain a good speed. per scanline or per frame is probably good.

	pt = TimerPtrs [ TIMERNUMBER ];
	
	
	// get the value you started at for timer
	lInitialValue = pt->StartValue;

	// get the current cycle //
	lCurrentCycle = *_DebugCycleCount;
	
	// if in free run mode, or this is timer 2, then can calculate like free run
	if ( ( !MODE_RunMode ) || ( TIMERNUMBER == 2 )
#ifdef PS2_COMPILE
		|| ( TIMERNUMBER > 3 )
#endif
	)
	{
		// if this is timer 2 and sync mode is set and sync mode is 0 or 3, then  do not count
		if ( MODE_RunMode && ( TIMERNUMBER == 2 ) && ( ( MODE_SyncMode == 0 ) || ( MODE_SyncMode == 3 ) ) )
		{
			// timer 2 is paused //
			pt->StartCycle = lCurrentCycle;
		}
		else
		{
			// timer is in free run //
			//Update_FreeRunTimer ();
			pt->StartValue += CountTicks_Scanline_t<TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( pt->StartCycle, lCurrentCycle );
			
			pt->StartCycle = lCurrentCycle;
		}
	}
	else
	{
		// Timer 0 or 1 in sync mode //
		
		
		// get the start cycle //
		//lStartCycle = StartCycle;
		
		// initialize next blanking area start cycle
		lBlankCycle = 0;
		
		//while ( lStartCycle < lCurrentCycle )
		//{
		
		
		
			// check if in blanking area //
			
			// check if timer 0 or 1 //
			// note: possibly check for timer 3 if ps2 ??
			if ( !TIMERNUMBER )
			{
				// pixel counter //
				//bBlank = g->isHBlank ( (double) lStartCycle );
				if ( pt->StartCycle < g->llHBlankStart )
				{
					// set the cycle hblank start at
					lBlankCycle = g->llHBlankStart;
				}
			}
			else
			{
				// hblank counter //
				//bBlank = g->isVBlank ( (double) lStartCycle );
				
				// set the cycle that vblank starts at if known, otherwise use max value
				if ( ( g->lNextScanline & ~1 ) == g->VBlank_Y )
				{
					// vblank starts at the next scanline, so set as the start of the blanking area, otherwise set to max if not in vblank
					lBlankCycle = g->llNextScanlineStart;
				}
				else if ( ( g->lScanline & ~1 ) < g->VBlank_Y )
				{
					// otherwise not in blanking area, so set to max
					lBlankCycle = -1LL;
				}
			}


			if ( pt->StartCycle < lBlankCycle )
			{
				// not in Blank //
				
				// get blank cycle and offset cycles //
				
				
				// set where blanking area starts as the end cycle //
				lEndCycle = lBlankCycle;
				
				// check if passed the current cycle
				if ( lEndCycle > lCurrentCycle )
				{
					// do not pass the current cycle //
					lEndCycle = lCurrentCycle;
				}
				
				
				switch ( MODE_SyncMode )
				{
					case 0:
					case 1:
						// counts outside of blanking area //
						pt->StartValue += CountTicks_Scanline_t<TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( pt->StartCycle, lEndCycle );
						break;
						
					case 2:
					case 3:
						// paused outside of blanking area //
#ifdef ENABLE_FORCE_FREERUN
							if ( pt->Force_FreeRun )
							{
							
							pt->StartValue += CountTicks_Scanline_t<TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( pt->StartCycle, lEndCycle );
							//pt->StartCycle = lCurrentCycle;
							
							// no more blanking areas to worry about because it is in free run now, so set to max
							//lBlankCycle = -1LL;
							
							}
#endif
						break;
				}
				
				
				// now we are at either current cycle or start of blanking area
				pt->StartCycle = lEndCycle;
				
				
				// if the blanking area was just hit, then process //
				// when just reached blank, so for mode 0 do nothing, for mode 1 reset to zero, mode 2 reset to zero, mode 3 switch to free run
				if ( lEndCycle == lBlankCycle )
				{
					switch ( MODE_SyncMode )
					{
						case 0:
							// pause counter during blank //
							
							// just reached blank and will pause in the code, so do nothing here //
							break;
							
						case 1:
						case 2:
						
							// reset counter to zero at blank //
							pt->StartValue = 0;
							break;
							
						case 3:
						
							// switch to free run at blank //
#ifdef ENABLE_FORCE_FREERUN
							if ( !pt->Force_FreeRun )
							{
								pt->Force_FreeRun = 1;
#else
							pt->MODE.RunMode = 0;
#endif
							
							pt->StartValue += CountTicks_Scanline_t<TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( pt->StartCycle, lCurrentCycle );
							pt->StartCycle = lCurrentCycle;
							
							// no more blanking areas to worry about because it is in free run now, so set to max
							lBlankCycle = -1LL;
							
#ifdef ENABLE_FORCE_FREERUN
							}
#endif

							// update the start cycle for timer
							//StartCycle = lEndCycle;
							
							// calculate the rest of the cycles //
							//Update_FreeRunTimer ();
							
							// set the end cycle //
							//lEndCycle = StartCycle;
							
							break;
							
					} // end switch ( MODE.SyncMode )
				
				} // end if ( lEndCycle == lBlankCycle )
				
				
			} // end if ( !bBlank )

			
			// check if in blank //
			if ( pt->StartCycle >= lBlankCycle )
			{
				// Blank //
				
				// get next end cycle //
				
				
				// if in blanking area, then process until end of scanline or until current cycle is reached
				//lEndCycle = g->llNextScanlineStart;
				//lEndCycle = lCurrentCycle;
				
				// check if passed the current cycle
				//if ( lEndCycle > lCurrentCycle )
				//{
				//	// do not pass the current cycle //
				//	lEndCycle = lCurrentCycle;
				//}
				
				// check the sync mode //
				// but here, we were already in blanking area, so count for modes except zero
				// both sync modes 0 and 3 are paused during blank, so only count for modes 1 and 2
				/*
				if ( MODE.SyncMode == 1 || MODE.SyncMode == 2 )
				{
					// calculate the offset cycles //
					dOffsetCycles = Get_OffsetCycles ( lStartCycle );
					
					// update clock ticks //
					StartValue += (u64) ( ( ((double) ( lEndCycle - lStartCycle )) + dOffsetCycles ) * dTicksPerCycle );
				}
				*/
				
				switch ( MODE_SyncMode )
				{
					case 0:
					case 3:
						// paused during blanking area //
#ifdef ENABLE_FORCE_FREERUN
							if ( pt->Force_FreeRun )
							{
							
							pt->StartValue += CountTicks_Scanline_t<TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( pt->StartCycle, lCurrentCycle );
							//pt->StartCycle = lCurrentCycle;
							
							// no more blanking areas to worry about because it is in free run now, so set to max
							//lBlankCycle = -1LL;
							
							}
#endif
						break;
						
					case 1:
					case 2:
						// counts during blanking area //
						pt->StartValue += CountTicks_Scanline_t<TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( pt->StartCycle, lCurrentCycle );
						break;
				}
				
			} // end if ( StartCycle >= lBlankCycle )
			
			
			// update cycle we are at //
			//lStartCycle = lEndCycle;
			
		//} // end while ( lStartCycle < lCurrentCycle )
		
		// set the start cycle as the current cycle //
		//StartCycle = *_DebugCycleCount;
		pt->StartCycle = lCurrentCycle;
		
	} // end if ( ( !MODE.RunMode ) || ( TimerNumber == 2 ) || ( TimerNumber > 3 ) )
	
	
#ifdef PS2_COMPILE
	if ( TIMERNUMBER < 3 )
	{
#endif

	lCompareValue = ( (u64) pt->COMP.Compare );
	//lOverflowValue = 0xffffULL;
	
#ifdef PS2_COMPILE
	}
	else
	{
		lCompareValue = ( (u64) pt->COMP.Value );
		//lOverflowValue = 0xffffffffULL;
		
	}
#endif

	// get the wrap value
	if ( MODE_CountToTarget && ( lInitialValue <= lCompareValue ) )
	{
		// count to target //
		//lCompTick = ( (u32) COMP.Compare );
		lWrapValue = lCompareValue + 1;
	}
	else
	{
		// count to overflow //
		//lCompTick = ( (u32) 0xffff );
		lWrapValue = OVERFLOW_MASK + 1;
	}
	

	if ( lInitialValue >= lCompareValue )
	{
		// note: it's not possible to reach or interrupt on compare value if it is zero since it could only do this on reset
		if ( lCompareValue ) lCompareValue += lWrapValue;
	}
	
	// before wrapping value around, set the "value reached" flags

	// if target value was reached, then set the flag that says so
	if ( lInitialValue < lCompareValue && pt->StartValue >= lCompareValue )
	{
#ifdef SET_REACHED_ONLY_ON_INT
		if ( MODE_IrqOnTarget )
		{
#endif

		// *note* it's also possible that this might be only set if an interrupt occurs at target ??
		pt->MODE.TargetReached = 1;
		
#ifdef SET_REACHED_ONLY_ON_INT
		}
#endif
	}
	
	if ( !MODE_CountToTarget || ( lInitialValue > lCompareValue ) )
	{
		// if overflow value was reached, then set the flag that says so
		if ( pt->StartValue > OVERFLOW_MASK )
		{
#ifdef SET_REACHED_ONLY_ON_INT
			if ( MODE_IrqOnOverflow )
			{
#endif

			// *note* it's also possible that this might be only set if an interrupt occurs at overflow ??
			pt->MODE.OverflowReached = 1;
			
#ifdef SET_REACHED_ONLY_ON_INT
			}
#endif

			// can only do this part when counting to target, since it resets the wrap value
			if ( MODE_CountToTarget )
			{
				// here should wrap against overflow
				// but the timer could be anywhere, so must first subtract
				//pt->StartValue &= lOverflowValue;
				pt->StartValue -= ( OVERFLOW_MASK + 1 );
				
				// now the wrap value is the compare value plus one
				lWrapValue = lCompareValue + 1;
			}
		}
	}
	
	
	// wrap timer value around and update associated flags
	//pt->UpdateTimer_Wrap ();
	
	// must actually hit the compare value to wrap
	// so if timer was set greater than compare value when counting to target then wrap against overflow first
	/*
	if ( MODE_CountToTarget && ( lInitialValue > lCompareValue ) )
	{
		if ( pt->StartValue > OVERFLOW_MASK )
		{
			// here should wrap against overflow
			// but the timer could be anywhere, so must first subtract
			//pt->StartValue &= lOverflowValue;
			pt->StartValue -= ( OVERFLOW_MASK + 1 );
			
			// now the wrap value is the compare value plus one
			lWrapValue = lCompareValue + 1;
		}
	}
	*/
	

	if ( MODE_CountToTarget )
	{
		// otherwise lInitialValue < lWrapValue
		if ( pt->StartValue >= lWrapValue )
		{
			// here should wrap against whatever it should be counting to
			pt->StartValue %= lWrapValue;
		}
	}
	else
	{
		// wrap against overflow
		pt->StartValue &= OVERFLOW_MASK;
	}
}


template<const u32 TIMERNUMBER, const u32 MODE_ClockSource, const u32 MODE_RunMode, const u32 MODE_SyncMode, const u32 MODE_CountToTarget, const u32 MODE_IrqOnTarget, const u32 MODE_IrqOnOverflow
#ifdef PS2_COMPILE
, const u32 MODE_ClockDiv
#endif
>
static void Get_NextEvent_Scanline_t ()
{
	const u64 OVERFLOW_MASK = ( TIMERNUMBER < 3 ) ? 0xffffULL : 0xffffffffULL;
	
	bool bBlank;
	double dOffsetCycles;
	u64 lStartCycle, lEndCycle, lCurrentCycle, lBlankCycle;
	u64 lIntCycle;
	u32 lStartValue, lEndValue;
	//u32 lIntValue;
	u32 lTargetValue;
	
	Timer* pt;
	
	pt = TimerPtrs [ TIMERNUMBER ];
	
	
	// if there are no interrupts, then no need for any events
	if ( ( !MODE_IrqOnTarget && !MODE_IrqOnOverflow ) || ( ( TIMERNUMBER == 2 ) && ( MODE_RunMode ) && ( ( MODE_SyncMode == 0 ) || ( MODE_SyncMode == 3 ) ) ) )
	{
		pt->SetNextEventCh_Cycle ( -1LL );
		return;
	}
	
	
	
	// set the final cycle //
	//lCurrentCycle = g->llNextScanlineStart;
	
	// set start cycle //
	lStartCycle = pt->StartCycle;

/*
#ifdef PS2_COMPILE
	if ( TIMERNUMBER < 3 )
	{
#endif
	// get start tick //
	lStartValue = pt->StartValue & 0xffffULL;
	
#ifdef PS2_COMPILE
	}
	else
	{
		lStartValue = pt->StartValue & 0xffffffffULL;
	}
#endif
*/

	lStartValue = pt->StartValue & OVERFLOW_MASK;

	// get value where next event should happen
	lTargetValue = pt->Get_NextIntTick_t<TIMERNUMBER,MODE_CountToTarget,MODE_IrqOnTarget,MODE_IrqOnOverflow>( lStartValue );
	
	// if in free run mode, or this is timer 2, then can calculate like free run
	if ( ( !MODE_RunMode ) || ( TIMERNUMBER == 2 )
#ifdef PS2_COMPILE
		|| ( TIMERNUMBER > 3 )
#endif
	)
	{
		// timer is in free run //
	
		// if this is timer 2 and sync mode is set and sync mode is 0 or 3, then do not count
		if ( MODE_RunMode && ( TIMERNUMBER == 2 ) && ( ( MODE_SyncMode == 0 ) || ( MODE_SyncMode == 3 ) ) )
		{
			// timer 2 is paused //
			
			//SetNextEventCh_Cycle ( -1LL );
		}
		else
		{
			// get the cycle of the next interrupt for free run timer //
			//lIntCycle = Get_FreeRunNextIntCycle ( lStartValue, lStartCycle );
			lIntCycle = Get_InterruptCycle_Scanline_t<TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( lStartCycle, lStartValue, lTargetValue );
			
			// set the cycle for the next event for timer
			pt->SetNextEventCh_Cycle ( lIntCycle );
			
			return;
		}
		
	}
	else
	{
		// Timer 0 or 1 in sync mode //
		
		// note: don't wrap around timer so you can see where it hits a particular tick //
		
		// first determine if in blanking area
		// if not in blanking area, then determine if target is reached before blanking area start, and if not discard result
		// perform action at blanking area start (reset timer, switch to free run, etc)
		// determine if target is reached on rest of scanline (in blanking area)
		
		
		// initialize cycle number for interrupt
		//lIntCycle = -1LL;
		
		
		// act like the current cycle is at end of scanline
		lCurrentCycle = g->llNextScanlineStart;
		
		
		// initialize blank cycle
		lBlankCycle = 0;
		
		//while ( lStartCycle < lCurrentCycle )
		//{
		
			// check if in blanking area //
			
			// check if timer 0 or 1 //
			// check if in blanking area //
			
			// check if timer 0 or 1 //
			// note: possibly check for timer 3 if ps2 ??
			if ( !TIMERNUMBER )
			{
				// pixel counter //
				//bBlank = g->isHBlank ( (double) lStartCycle );
				if ( lStartCycle < g->llHBlankStart )
				{
					// set the cycle hblank start at
					lBlankCycle = g->llHBlankStart;
				}
			}
			else
			{
				// hblank counter //
				//bBlank = g->isVBlank ( (double) lStartCycle );
				
				// set the cycle that vblank starts at if known, otherwise use max value
				if ( ( g->lNextScanline & ~1 ) == g->VBlank_Y )
				{
					// vblank starts at the next scanline, so set as the start of the blanking area, otherwise set to max if not in vblank
					lBlankCycle = g->llNextScanlineStart;
				}
				else if ( ( g->lScanline & ~1 ) < g->VBlank_Y )
				{
					// otherwise not in blanking area, so set to max
					lBlankCycle = -1LL;
				}
			}

			

			if ( lStartCycle < lBlankCycle )
			{
				// not in Blank //
				
				// get blank cycle and offset cycles //
				
				
				// set where blanking area starts as the end cycle //
				lEndCycle = lBlankCycle;
				
				
				// check if passed the end of scanline
				if ( lEndCycle > lCurrentCycle )
				{
					// do not pass the current cycle //
					lEndCycle = lCurrentCycle;
				}
				
				
				// get the end value
				switch ( MODE_SyncMode )
				{
					case 0:
					case 1:
						// counts outside of blanking area //
						lIntCycle = Get_InterruptCycle_Scanline_t<TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( lStartCycle, lStartValue, lTargetValue );
						
						// check if the target value has been passed before (or on transition to?) the blanking area
						if ( lIntCycle <= lBlankCycle )
						{
							// set an event for when interrupt occurs
							//lIntCycle = Get_InterruptCycle_Scanline ( lStartCycle, lStartValue, lTargetValue );
							pt->SetNextEventCh_Cycle ( lIntCycle );
							
							return;
						}
						
						// update timer value
						lStartValue += CountTicks_Scanline_t <TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( lStartCycle, lEndCycle );
						
						//lStartValue = lEndValue;
						
						break;
						
					case 2:
					case 3:
						// paused outside of blanking area //
						//lEndValue = lStartValue;
						
#ifdef ENABLE_FORCE_FREERUN
						if ( pt->Force_FreeRun )
						{
						lIntCycle = Get_InterruptCycle_Scanline_t<TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( lStartCycle, lStartValue, lTargetValue );
						
						// check if the target value has been passed before (or on transition to?) the blanking area
						if ( lIntCycle <= lBlankCycle )
						{
							// set an event for when interrupt occurs
							//lIntCycle = Get_InterruptCycle_Scanline ( lStartCycle, lStartValue, lTargetValue );
							pt->SetNextEventCh_Cycle ( lIntCycle );
							
							return;
						}
						
						// update timer value
						lStartValue += CountTicks_Scanline_t <TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( lStartCycle, lEndCycle );
						}
#endif

						break;
				}
				
				
				
				
				// set new value for timer
				//lStartValue = lEndValue;
				
				// now we are at either current cycle or start of blanking area
				lStartCycle = lEndCycle;

				
				// if the blanking area was just hit, then process //
				// when just reached blank, so for mode 0 do nothing, for mode 1 reset to zero, mode 2 reset to zero, mode 3 switch to free run
				if ( lEndCycle == lBlankCycle )
				{
					switch ( MODE_SyncMode )
					{
						case 0:
							// pause counter during blank //
							
							// just reached blank and will pause in the code, so do nothing here //
							break;
							
						case 1:
						case 2:
						
							// reset counter to zero at blank //
							lStartValue = 0;
							break;
							
						case 3:
						
							// switch to free run at blank //
							//MODE.RunMode = 0;
							
							//lStartValue += CountTicks_Scanline ( lStartCycle, lCurrentCycle );
							//StartCycle = lCurrentCycle;
							lIntCycle = Get_InterruptCycle_Scanline_t<TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( lStartCycle, lStartValue, lTargetValue );
							
							pt->SetNextEventCh_Cycle ( lIntCycle );
							
							return;
							
							// no more blanking areas to worry about because it is in free run now, so set to max
							//lBlankCycle = -1LL;
							
							
							break;
							
					} // end switch ( MODE.SyncMode )
				
				} // end if ( lEndCycle == lBlankCycle )
				
				
			} // end if ( !bBlank )

			
			// check if in blank //
			if ( lStartCycle >= lBlankCycle )
			{
				// Blank //
				
				// get next end cycle //
				
				switch ( MODE_SyncMode )
				{
					case 0:
					case 3:
						// paused during blanking area //

#ifdef ENABLE_FORCE_FREERUN
						if ( pt->Force_FreeRun )
						{
						lIntCycle = Get_InterruptCycle_Scanline_t<TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( lStartCycle, lStartValue, lTargetValue );
						
						// set the cycle for the next event for timer
						pt->SetNextEventCh_Cycle ( lIntCycle );
						
						return;
						}
#endif
						break;
						
					case 1:
					case 2:
					
						// counts during blanking area //
						
						//StartValue += CountTicks_Scanline ( lStartCycle, lEndCycle );
						
						lIntCycle = Get_InterruptCycle_Scanline_t<TIMERNUMBER,MODE_ClockSource
#ifdef PS2_COMPILE
			,MODE_ClockDiv
#endif
			>( lStartCycle, lStartValue, lTargetValue );
						
						// set the cycle for the next event for timer
						pt->SetNextEventCh_Cycle ( lIntCycle );
						
						return;
						
						break;
						
				} // end switch ( MODE.SyncMode )
				
			} // end if ( bBlank )
			
			// update cycle we are at //
			//lStartCycle = lEndCycle;
		//}
	}
	
	// unable to find next cycle timer should interrupt at for now
	pt->SetNextEventCh_Cycle ( -1LL );
}


template<const u32 TIMERNUMBER, const u32 CLOCKSOURCE, const u32 RUNMODE>
static void Get_NextEvent_Scanline_Sync_t ()
{
	if ( RUNMODE || ( ( TIMERNUMBER == 1 || TIMERNUMBER == 3 ) && ( CLOCKSOURCE & 1 ) ) )
	{
#ifdef USE_TIMER_CALLBACKS
		TimerPtrs [ TIMERNUMBER ]->cbGetNextEvent ();
#else
		TimerPtrs [ TIMERNUMBER ]->Get_NextEvent_Scanline ();
#endif
	}
}
		
template<const u32 TIMERNUMBER, const u32 CLOCKSOURCE, const u32 RUNMODE>
static void UpdateTimer_Scanline_Sync_t ()
{
	if ( RUNMODE || ( ( TIMERNUMBER == 1 || TIMERNUMBER == 3 ) && ( CLOCKSOURCE & 1 ) ) )
	{
#ifdef USE_TIMER_CALLBACKS
		TimerPtrs [ TIMERNUMBER ]->cbUpdate ();
#else
		TimerPtrs [ TIMERNUMBER ]->UpdateTimer_Scanline ();
#endif
	}
}


#ifdef USE_TEMPLATES_PS1_TIMER

#ifdef PS2_COMPILE
template<const u32 TIMERNUMBER, const u32 MODE_ClockSource, const u32 MODE_RunMode, const u32 MODE_SyncMode, const u32 MODE_CountToTarget, const u32 MODE_IrqOnTarget, const u32 MODE_IrqOnOverflow>
inline void CalibrateTimer_Scanline_t_7 ()
{
	switch ( MODE.ClockDiv )
	{
		case 0:
			cbUpdate = UpdateTimer_Scanline_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,MODE_IrqOnOverflow,0>;
			cbGetNextEvent = Get_NextEvent_Scanline_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,MODE_IrqOnOverflow,0>;
			break;
			
		case 1:
			cbUpdate = UpdateTimer_Scanline_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,MODE_IrqOnOverflow,1>;
			cbGetNextEvent = Get_NextEvent_Scanline_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,MODE_IrqOnOverflow,1>;
			break;
			
		case 2:
			cbUpdate = UpdateTimer_Scanline_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,MODE_IrqOnOverflow,2>;
			cbGetNextEvent = Get_NextEvent_Scanline_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,MODE_IrqOnOverflow,2>;
			break;
			
		case 3:
			cbUpdate = UpdateTimer_Scanline_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,MODE_IrqOnOverflow,3>;
			cbGetNextEvent = Get_NextEvent_Scanline_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,MODE_IrqOnOverflow,3>;
			break;
	}
}
#endif


template<const u32 TIMERNUMBER, const u32 MODE_ClockSource, const u32 MODE_RunMode, const u32 MODE_SyncMode, const u32 MODE_CountToTarget, const u32 MODE_IrqOnTarget>
inline void CalibrateTimer_Scanline_t_6 ()
{
	switch ( MODE.IrqOnOverflow )
	{
		case 0:
#ifdef PS2_COMPILE
			CalibrateTimer_Scanline_t_7<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,0>();
#else
			cbUpdate = UpdateTimer_Scanline_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,0>;
			cbGetNextEvent = Get_NextEvent_Scanline_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,0>;
#endif
			break;
			
		case 1:
#ifdef PS2_COMPILE
			CalibrateTimer_Scanline_t_7<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,1>();
#else
			cbUpdate = UpdateTimer_Scanline_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,1>;
			cbGetNextEvent = Get_NextEvent_Scanline_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,MODE_IrqOnTarget,1>;
#endif
			break;
			
	}
	
	cbUpdate_Sync = UpdateTimer_Scanline_Sync_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode>;
	cbGetNextEvent_Sync = Get_NextEvent_Scanline_Sync_t<TIMERNUMBER,MODE_ClockSource,MODE_RunMode>;
}



template<const u32 TIMERNUMBER, const u32 MODE_ClockSource, const u32 MODE_RunMode, const u32 MODE_SyncMode, const u32 MODE_CountToTarget>
inline void CalibrateTimer_Scanline_t_5 ()
{
	switch ( MODE.IrqOnTarget )
	{
		case 0:
			CalibrateTimer_Scanline_t_6<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,0>();
			break;
			
		case 1:
			CalibrateTimer_Scanline_t_6<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,MODE_CountToTarget,1>();
			break;
	}
}



template<const u32 TIMERNUMBER, const u32 MODE_ClockSource, const u32 MODE_RunMode, const u32 MODE_SyncMode>
inline void CalibrateTimer_Scanline_t_4 ()
{
	switch ( MODE.CountToTarget )
	{
		case 0:
			CalibrateTimer_Scanline_t_5<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,0>();
			break;
			
		case 1:
			CalibrateTimer_Scanline_t_5<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,MODE_SyncMode,1>();
			break;
	}
}



template<const u32 TIMERNUMBER, const u32 MODE_ClockSource, const u32 MODE_RunMode>
inline void CalibrateTimer_Scanline_t_3 ()
{
	switch ( MODE.SyncMode )
	{
		case 0:
			CalibrateTimer_Scanline_t_4<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,0>();
			break;
			
		case 1:
			CalibrateTimer_Scanline_t_4<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,1>();
			break;
			
		case 2:
			CalibrateTimer_Scanline_t_4<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,2>();
			break;
			
		case 3:
			CalibrateTimer_Scanline_t_4<TIMERNUMBER,MODE_ClockSource,MODE_RunMode,3>();
			break;
	}
}



template<const u32 TIMERNUMBER, const u32 MODE_ClockSource>
inline void CalibrateTimer_Scanline_t_2 ()
{
	switch ( MODE.RunMode )
	{
		case 0:
			//CalibrateTimer_Scanline_t_3<TIMERNUMBER,MODE_ClockSource,0>();
			CalibrateTimer_Scanline_t_4<TIMERNUMBER,MODE_ClockSource,0,0>();
			break;
			
		case 1:
			CalibrateTimer_Scanline_t_3<TIMERNUMBER,MODE_ClockSource,1>();
			break;
	}
}


template<const u32 TIMERNUMBER>
inline void CalibrateTimer_Scanline_t_1 ()
{
	//switch ( MODE.ClockSource )
	switch ( ( ( TIMERNUMBER == 2 ) ? ( MODE.ClockSource >> 1 ) : MODE.ClockSource ) & 1 )
	{
		case 0:
			CalibrateTimer_Scanline_t_2<TIMERNUMBER,0>();
			break;
			
		case 1:
			CalibrateTimer_Scanline_t_2<TIMERNUMBER,1>();
			break;
			
		//case 2:
		//	CalibrateTimer_Scanline_t_2<TIMERNUMBER,2>();
		//	break;
			
		//case 3:
		//	CalibrateTimer_Scanline_t_2<TIMERNUMBER,3>();
		//	break;
	}
}

void CalibrateTimer_Scanline_t ()
{
	switch ( TimerNumber )
	{
		case 0:
			CalibrateTimer_Scanline_t_1<0>();
			break;
			
		case 1:
			CalibrateTimer_Scanline_t_1<1>();
			break;
			
		case 2:
			CalibrateTimer_Scanline_t_1<2>();
			break;
			
#ifdef PS2_COMPILE
		case 3:
			CalibrateTimer_Scanline_t_1<3>();
			break;
			
		case 4:
			//CalibrateTimer_Scanline_t_1<4>();
			CalibrateTimer_Scanline_t_4<4,0,0,0> ();
			break;
			
		case 5:
			//CalibrateTimer_Scanline_t_1<5>();
			CalibrateTimer_Scanline_t_4<5,0,0,0> ();
			break;
#endif
	}
}

#endif	// USE_TEMPLATES_PS1_TIMER



	};
	
	
	

	class Timers
	{
	
	public:
		
		static Debug::Log debug;
		
		static Timers *_TIMERS;
		
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the timer registers start at
		static const u32 Regs_Start = 0x1f801100;
		
		// where the timer registers end at
		static const u32 Regs_End = 0x1f801128;
	
		// distance between numbered groups of registers
		static const u32 Reg_Size = 0x10;
		
		
		// index of next event
		u32 NextEvent_Idx;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;



		
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
		
		
#ifdef PS2_COMPILE
		static const u32 c_iNumberOfChannels = 6;
#else
		static const u32 c_iNumberOfChannels = 3;
#endif
		
		Timer TheTimers [ c_iNumberOfChannels ];
		

		// these allow you to read and write registers and allow the device to act on the read/write event
		static u32 Read ( u32 Address );
		static void Write ( u32 Address, u32 Data, u32 Mask );

		
		// update what cycle the next event is at for this device
		void Update_NextEventCycle ();

		
		// this gets the next event cycle for the specified timer
		inline void Get_NextEvent ( int TimerNumber, u64 ThroughCycle ) { TheTimers [ TimerNumber ].Get_NextEvent ( ThroughCycle ); }
		
		
		// this updates the specified timer value
		inline void UpdateTimer ( u32 TimerNumber ) { TheTimers [ TimerNumber ].UpdateTimer (); }

		// for when using scanline timers
		inline void Get_NextEvent_Scanline ( int TimerNumber )
		{
#ifdef USE_TIMER_CALLBACKS
			TheTimers [ TimerNumber ].cbGetNextEvent ();
#else
			TheTimers [ TimerNumber ].Get_NextEvent_Scanline ();
#endif
		}
		
		inline void UpdateTimer_Scanline ( u32 TimerNumber )
		{
#ifdef USE_TIMER_CALLBACKS
			TheTimers [ TimerNumber ].cbUpdate ();
#else
			TheTimers [ TimerNumber ].UpdateTimer_Scanline ();
#endif
		}

		// to only enable update of scanline timers when a sync mode applies
		inline void Get_NextEvent_Scanline_Sync ( int TimerNumber )
		{
#ifdef USE_TIMER_CALLBACKS
			TheTimers [ TimerNumber ].cbGetNextEvent_Sync ();
#else
			TheTimers [ TimerNumber ].Get_NextEvent_Scanline ();
#endif
		}
		
		inline void UpdateTimer_Scanline_Sync ( u32 TimerNumber )
		{
#ifdef USE_TIMER_CALLBACKS
			TheTimers [ TimerNumber ].cbUpdate_Sync ();
#else
			TheTimers [ TimerNumber ].UpdateTimer_Scanline ();
#endif
		}
		


#ifdef USE_TEMPLATES_PS1_TIMER
		inline void CalibrateTimer_Scanline ( u32 TimerNumber ) { TheTimers [ TimerNumber ].CalibrateTimer_Scanline_t (); }
#endif
		
		inline void CalibrateTimer ( u32 TimerNumber ) { TheTimers [ TimerNumber ].CalibrateTimer (); }
		
		
		inline void SetMode ( u32 TimerNumber, u32 Data ) { TheTimers [ TimerNumber ].SetMode ( Data ); }
		inline void SetValue ( u32 TimerNumber, u32 Data ) { TheTimers [ TimerNumber ].SetValue ( Data ); }
		inline void SetComp ( u32 TimerNumber, u32 Data ) { TheTimers [ TimerNumber ].SetComp ( Data ); }
		
		

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////

		// the timer count - read only
		static const u32 COUNT_Base = 0x1f801100;
		
		
		// the timer mode - setting this to zero resets a timer - read and write
		static const u32 MODE_Base = 0x1f801104;
		
		
		// the timer compare/target value - read and write
		static const u32 COMP_Base = 0x1f801108;
		
		
		static const u32 TIMER0_COUNT = 0x1f801100;
		static const u32 TIMER0_MODE = 0x1f801104;
		static const u32 TIMER0_COMP = 0x1f801108;

		static const u32 TIMER1_COUNT = 0x1f801110;
		static const u32 TIMER1_MODE = 0x1f801114;
		static const u32 TIMER1_COMP = 0x1f801118;

		static const u32 TIMER2_COUNT = 0x1f801120;
		static const u32 TIMER2_MODE = 0x1f801124;
		static const u32 TIMER2_COMP = 0x1f801128;
		
#ifdef PS2_COMPILE

		static const u32 TIMER3_COUNT = 0x1f801480;
		static const u32 TIMER3_MODE = 0x1f801484;
		static const u32 TIMER3_COMP = 0x1f801488;

		static const u32 TIMER4_COUNT = 0x1f801490;
		static const u32 TIMER4_MODE = 0x1f801494;
		static const u32 TIMER4_COMP = 0x1f801498;

		static const u32 TIMER5_COUNT = 0x1f8014a0;
		static const u32 TIMER5_MODE = 0x1f8014a4;
		static const u32 TIMER5_COMP = 0x1f8014a8;
		
#endif
		
		//////////////////////////////////////////
		// Timers are synchronized to:			//
		// Timer 0 - Pixel clock				//
		// Timer 1 - Horizontal Retrace			//
		// Timer 2 - 1/8 System Clock			//
		// Timer 3 - Vertical Retrace			//
		//////////////////////////////////////////

		// timers 0 and 1 get a signal from the GPU for pixel/hblank
		static GPU *g;
		
		
		
		
		// Constructor
		//Timers ();
		
		void Reset ();
		
		void Start ();
		
		void ConnectDevices ( GPU *_g );

		void Refresh ();
		void ReCalibrateAll ();
		
		
		// run for a clock cycle
		// returns interrupt status - whether to interrupt or not
		// needs GPU Status READ register
		// returns interrupt status for INTC
		void Run ();


		static void sRun () { _TIMERS->Run (); }
		static void Set_EventCallback ( funcVoid2 UpdateEvent_CB ) { _TIMERS->NextEvent_Idx = UpdateEvent_CB ( sRun ); };

		
		static u64* _NextSystemEvent;


		// for interrupt call back
		static funcVoid UpdateInterrupts;
		static void Set_IntCallback ( funcVoid UpdateInt_CB ) { UpdateInterrupts = UpdateInt_CB; };

		
		static const u32 c_InterruptBit = 4;
		static const u32 c_InterruptBit2 = 14;
		
		static u32* _Intc_Master;
		static u32* _Intc_Stat;
		static u32* _Intc_Mask;
		static u32* _R3000A_Status_12;
		static u32* _R3000A_Cause_13;
		static u64* _ProcStatus;
		
		inline void ConnectInterrupt ( u32* _IStat, u32* _IMask, u32* _R3000A_Status, u32* _R3000A_Cause, u64* _ProcStat )
		{
			_Intc_Stat = _IStat;
			_Intc_Mask = _IMask;
			// _Intc_Master = _IMaster;
			_R3000A_Cause_13 = _R3000A_Cause;
			_R3000A_Status_12 = _R3000A_Status;
			_ProcStatus = _ProcStat;
		}
		
		inline void SetInterrupt ( const u32 TimerNumber )
		{
#ifdef PS2_COMPILE
			if ( TimerNumber < 3 )
			{
#endif

			*_Intc_Stat |= ( 1 << ( c_InterruptBit + TimerNumber ) );

			// *_Intc_Master |= ( 1 << ( c_InterruptBit + TimerNumber ) );
			
#ifdef PS2_COMPILE
			}
			else
			{
				*_Intc_Stat |= ( 1 << ( c_InterruptBit2 + ( TimerNumber - 3 ) ) );

				// *_Intc_Master |= ( 1 << ( c_InterruptBit2 + ( TimerNumber - 3 ) ) );
			}
#endif

			UpdateInterrupts ();

			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			
#ifdef PS2_COMPILE
#ifdef ENABLE_SBUS_INT_TIMER
			if ( *_Intc_Stat & Playstation2::SIF::_SIF->lSBUS_F230 )
			{
				// ??
				Playstation2::SIF::SetInterrupt_EE_SIF ();
			}
#endif
#endif
			*/
		}
		
		inline void ClearInterrupt ( const u32 TimerNumber )
		{
#ifdef PS2_COMPILE
			if ( TimerNumber < 3 )
			{
#endif
			// maybe this also needs to be cleared on INTC ??
			// *_Intc_Stat &= ~( 1 << ( c_InterruptBit + TimerNumber ) );

			*_Intc_Master &= ~( 1 << ( c_InterruptBit + TimerNumber ) );

#ifdef PS2_COMPILE
			}
			else
			{
				// *_Intc_Stat &= ~( 1 << ( c_InterruptBit2 + ( TimerNumber - 3 ) ) );

				*_Intc_Master &= ~( 1 << ( c_InterruptBit2 + ( TimerNumber - 3 ) ) );
			}
#endif

			UpdateInterrupts ();
			
			/*
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}

		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		static u64* _SystemCycleCount;
		static u32 *_NextEventIdx;

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

