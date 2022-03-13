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


#ifndef _PS2_TIMER_H_
#define _PS2_TIMER_H_


#include "types.h"

#include "Debug.h"

//#include "PS2_Intc.h"
//#include "PS2_Gpu.h"

#include "DebugValueList.h"


//#define INLINE_DEBUG_NEXTEVENT_T

// with the new ps2 timer code, this should probably be enabled?
#define USE_NEW_TIMER_CODE

// enables templates for ps2 timers (speed increase, but not good for debugging)
//#define USE_TEMPLATES_PS2_TIMER


// unsure if compare interrupt should get triggered on the same cycle and overflow interrupt is triggered
// note: this is also in the Code/CPP file
//#define DISABLE_COMPAREINT_AFTEROVF


namespace Playstation2
{

	

	class Timer
	{
	
		//Debug::Log debug;
	
	public:
	
		// maximum readable timer value
		static const long c_iMaxTimerValue = 0xffff;
		
		// the internal timer value at overflow (before getting reset to zero)
		static const long c_iTimerValueAtOverflow = 0x10000;
		
		static unsigned long long *_llCycleCount;
		static unsigned long long *_llScanlineStart, *_llNextScanlineStart, *_llHBlankStart;
		static unsigned long *_lScanline, *_lNextScanline, *_lVBlank_Y, *_lRasterYMax;
	
		// timers 0 and 1 get a signal from the GPU for pixel/hblank
		//static GPU *g;
		
		
		static int Count;
		
		
	
		int Index;
		u32 TimerNumber;
		
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
				// 10: Resets and starts counting at gate signal falling edge; 11: resets and starts counting at both edges of gate signal
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
		
		
		union HOLD_Format
		{
			struct
			{
				// this is the current compare value - bits 0-15
				u16 Hold;
				
				// this is garbage data - bits 16-31
				u16 Garbage;
			};
			
			u32 Value;
		};
		
		HOLD_Format HOLD;
		
		
		// also need to know more stuff about the timer
		
		// timer status - like free run, wait till next hblank, etc
		//enum { TIMER_FREERUN = 0, TIMER_SYNC_RUN, TIMER_SYNC_WAIT, TIMER_SYNC_STOPPED };
		//u32 TimerStatus;
		
		u64 IRQ_Counter;
		
		// value the timer started at with the cycle number it started at that value
		u64 StartValue;
		u64 StartCycle;
		
		static const int c_iNumberOfChannels = 4;
		static Timer* TimerPtrs [ c_iNumberOfChannels ];
		
		// need to know when the next blanking starts for a timer in sync mode
		//u64 NextBlankingStart_Cycle;
		
		// cycles to add before dividing
		//double dOffsetCycles;
		
		// these two are in 32.32 fixed point
		// these actually need to be double-precision floating point
		//double dCyclesPerTick;
		//double dTicksPerCycle;
		
		// this is when the next known event or interrupt happens
		u64 NextEvent_Cycle;
		
		
		typedef void (*UpdateFn) ( Timer* t );
		typedef void (*GetNextEventFn) ( Timer* t );

		typedef void (*UpdateFn_Sync) ( Timer* t );
		typedef void (*GetNextEventFn_Sync) ( Timer* t );
		
		// *note* these would have to be reloaded whenever restoring a save state
		UpdateFn cbUpdate;
		GetNextEventFn cbGetNextEvent;
		UpdateFn_Sync cbUpdate_Sync;
		GetNextEventFn_Sync cbGetNextEvent_Sync;
		
		// ***todo*** here would need to set the call back functions above
		void AfterSaveStateLoad ();
		
		// sets the call back functions
		void SetCB ();
		
		u32 EventType;
		enum { TIMER_COMPARE_INT, TIMER_OVERFLOW_INT, TIMER_COMPARE, TIMER_OVERFLOW };
		
		// constructor - pass timer number 0-3, and ref to parent object
		Timer ();
		
		// reset the timer - use when creating timer or resetting system
		void Reset ();

		// "TimerIndex" just specifies the index of the timer in the array of timers for the collection of timers it is in
		// for standalone PS1, "TimerNumber" is 0 for pixel timer, 1 for HBlank Timer, 2 for Cycle timer
		void Set_TimerNumber ( int TimerIndex, u32 TimerNumber );
		
		// sets/calculates timer interval
		void CalibrateTimer ();
		
		
		//inline static u64 Get_NextIntCycle_Clock ( Timer* t, u64 llStartCycle, long lStartTick, long lNextIntTick );
		
		// this updates the timer value
		//template<const int MODE_CounterEnable, const int MODE_ClkSelect, const int MODE_Gate, const int MODE_GateSelect, const int MODE_GateMode>
		//static void UpdateTimer ( Timer *t );
		
		// this gets the next event cycle for the timer, but may possibly only calculate up until "ThroughCycle"
		//template<const int MODE_CounterEnable, const int MODE_ClkSelect, const int MODE_Gate, const int MODE_GateSelect, const int MODE_GateMode>
		//static void Get_NextEvent ( Timer *t );
		
		inline long GetValue () { cbUpdate (this); return StartValue; }
		inline long GetMode () { return MODE.Value; }
		inline long GetComp () { return COMP.Value; }
		
		void SetMode ( u32 Data );
		void SetValue ( u32 Data );
		void SetComp ( u32 Data );
		
		// update what cycle the next event is at for this device
		void SetNextEventCh_Cycle ( u64 Cycle );
		void SetNextEventCh ( u64 Cycle );
		
		u64 Get_NextIntTick ( u64 lStartValue );
		u64 CountTicks ( u64 lBeginCycle, u64 lEndCycle );
		u64 Get_InterruptCycle ( u64 lBeginCycle, u64 lStartTick, u64 lTargetTick );
		void UpdateTimer ();
		void Get_NextEvent ();


template<const u32 MODE_CounterEnable, const u32 MODE_ClkSelect, const u32 MODE_Gate>
static void Get_NextEvent_Scanline_Sync_t ( Timer* pt )
{
	if ( MODE_CounterEnable && ( MODE_Gate || MODE_ClkSelect ) )
	{
//#ifdef USE_TIMER_CALLBACKS
		pt->cbGetNextEvent ( pt );
//#else
//		TheTimers [ TimerNumber ]->Get_NextEvent_Scanline ();
//#endif
	}
}

template<const u32 MODE_CounterEnable, const u32 MODE_ClkSelect, const u32 MODE_Gate>
static void UpdateTimer_Scanline_Sync_t ( Timer* pt )
{
	if ( MODE_CounterEnable && ( MODE_Gate || MODE_ClkSelect ) )
	{
//#ifdef USE_TIMER_CALLBACKS
		pt->cbUpdate ( pt );
//#else
//		TheTimers [ TimerNumber ]->UpdateTimer_Scanline ();
//#endif
	}
}
		
		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;

		
	private:
		
		// gets the next tick at which interrupt occurs for specified timer (does not wrap tick value)
		//u32 Get_NextIntTick ( u32 lStartTick );
		
		//double Get_OffsetCycles ( u64 lStartCycle );
		//u64 Get_FreeRunNextIntCycle ( u32 lStartValue, u64 lStartCycle );
		
		//void Update_FreeRunTimer ();
		//void UpdateTimer_Wrap ();
		
		template<const int MODE_CounterEnable, const int MODE_ClkSelect, const int MODE_Gate, const int MODE_GateSelect, const int MODE_GateMode>
		inline void CalibrateTimer5 ();
		template<const int MODE_CounterEnable, const int MODE_ClkSelect, const int MODE_Gate, const int MODE_GateSelect>
		inline void CalibrateTimer4 ();
		template<const int MODE_CounterEnable, const int MODE_ClkSelect, const int MODE_Gate>
		inline void CalibrateTimer3 ();
		template<const int MODE_CounterEnable, const int MODE_ClkSelect>
		inline void CalibrateTimer2 ();
		template<const int MODE_CounterEnable>
		inline void CalibrateTimer1 ();



	public:


#ifdef USE_TEMPLATES_PS2_TIMER

template<const u32 MODE_CompareEnable, const u32 MODE_IrqOnTarget, const u32 MODE_IrqOnOverflow>
inline u64 Get_NextIntTick_t ( u64 lStartValue )
{
	const u64 OVERFLOW_MASK = 0xffffULL;
	
	u64 lIntTick_Target, lIntTick_Overflow, lIntTick;
	u64 lCompareValue;	// , lOverflowValue;
	u64 lWrapValue;
	
	// get the start tick //
	//lStartTick = StartValue & 0xffff;
	
	// initialize value at which interrupt occurs to be max
	// not needed for now
	//lIntTick = 0xffffffffffffffffULL;

/*
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
*/


	// get compare value //
	lCompareValue = (u32) COMP.Compare;
	
	
	// get the compare tick //
	
	// check if counting to target //
	if ( MODE_CompareEnable && ( lStartValue <= lCompareValue ) )
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
	
#ifdef DISABLE_COMPARE_INT_ON_ZERO
		// note: can't interrupt on compare value if it is zero
		// note: looks like it does interrupt on compare value if it is zero?
		if ( lCompareValue )
		{
#endif

			// counter can always reach target //
			//lIntTick_Target = ( (u32) COMP.Compare );
			lIntTick_Target = lCompareValue;
			
			// check if start tick is on or after int tick, then add compare value to the end tick plus one
			if ( lStartValue >= lCompareValue )
			{
				// should only be plus one if the target is NOT excluded
				lIntTick_Target += lWrapValue;
			}
			
#ifdef DISABLE_COMPARE_INT_ON_ZERO
		} // end if ( lCompareValue )
#endif

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



// counts the clock tics between two cycle numbers ignoring sync modes	
template<const u32 MODE_ClkSelect>
static inline u64 CountTicks_t ( u64 lBeginCycle, u64 lEndCycle )
{
	const u64 c_ullCycleClock_Mask = ( 1 << ( MODE_ClkSelect << 2 ) ) - 1;
	const u64 c_ullCycleClock_Shift = MODE_ClkSelect << 2;
	
	switch ( MODE_ClkSelect )
	{
		case 0:
		case 1:
		case 2:
			// count cycles //
			return ( ( lEndCycle & ~c_ullCycleClock_Mask ) - ( lBeginCycle & ~c_ullCycleClock_Mask ) ) >> c_ullCycleClock_Shift;
			break;
			
		case 3:
			// count hsyncs //
			if ( ( lBeginCycle < *_llHBlankStart ) && ( lEndCycle >= *_llHBlankStart ) )
			{
				return 1;
			}
			
			return 0;
			
			break;
	}
	
}


// get the cycles to get from one tick to another
template<const u32 MODE_ClkSelect>
static inline u64 Get_InterruptCycle_t ( u64 lBeginCycle, u64 lStartTick, u64 lTargetTick )
{
	const u64 c_ullCycleClock_Mask = ( 1 << ( MODE_ClkSelect << 2 ) ) - 1;
	const u64 c_ullCycleClock_Shift = MODE_ClkSelect << 2;
	
	u64 lReturnCycles;
	u64 lTemp;
	double dTemp;
	
	// the target should come after the starting point
	if ( lTargetTick <= lStartTick ) return -1LL;
	
	// check for clock source
	switch ( MODE_ClkSelect )
	{
		case 0:
		case 1:
		case 2:
			// system clock //
			//return lBeginCycle + ( lTargetTick - lStartTick );
			return lBeginCycle + ( ( ( lTargetTick - lStartTick ) << c_ullCycleClock_Shift ) - ( lBeginCycle & c_ullCycleClock_Mask ) );
			break;
			
		case 3:
			// hblank clock //
			if ( lTargetTick != ( lStartTick + 1 ) )
			{
				// interrupt happens at some unknown point in the future
				return -1LL;
			}
			
			// make sure the starting cycle is before hblank on the scanline
			if ( lBeginCycle >= *_llHBlankStart ) return -1LL;
			
			// interrupt should trigger when hblank happens
			return *_llHBlankStart;
			
			break;
	}

}


template<const u32 MODE_CounterEnable, const u32 MODE_ClkSelect, const u32 MODE_GateSelect, const u32 MODE_Gate, const u32 MODE_GateMode, const u32 MODE_CompareEnable, const u32 MODE_IrqOnTarget, const u32 MODE_IrqOnOverflow>
static void UpdateTimer_t ( Timer *pt )
{
	const u64 OVERFLOW_MASK = 0xffffULL;

	bool bBlank;
	double dOffsetCycles;
	u64 lStartCycle, lEndCycle, lCurrentCycle;
	u64 lBlankStartCycle, lBlankEndCycle;
	u64 lInitialValue, lCompareValue, lWrapValue;	// , lOverflowValue;
	
	//Timer* pt;
	
	// note: timer should be updated before reading value/mode. Should also be updated periodically to maintain a good speed. per scanline or per frame is probably good.

	//pt = TimerPtrs [ TIMERNUMBER ];
	
	// if timer is not enabled or otherwise frozen then do not count
	if ( !MODE_CounterEnable )
	{
		// timer is paused //
		pt->StartCycle = lCurrentCycle;
		return;
	}
	
	// get the value you started at for timer
	lInitialValue = pt->StartValue;

	// get the current cycle //
	lCurrentCycle = *_DebugCycleCount;
	
	// if in free run mode, or this is timer 2, then can calculate like free run
	if ( !MODE_Gate )
	{
		// timer is in free run //
		//Update_FreeRunTimer ();
		pt->StartValue += CountTicks_t<MODE_ClkSelect>( pt->StartCycle, lCurrentCycle );
		
		pt->StartCycle = lCurrentCycle;
	}
	else
	{
		// Timer 0 or 1 in sync mode //
		
		
		// get the start cycle //
		//lStartCycle = StartCycle;
		
		// initialize next blanking area start cycle
		lBlankStartCycle = 0;
		lBlankEndCycle = -1LL;
		
		//while ( lStartCycle < lCurrentCycle )
		//{
		
		
		
			// check if in blanking area //
			
			// check if timer 0 or 1 //
			// note: possibly check for timer 3 if ps2 ??
			//if ( !TIMERNUMBER )
			if ( !MODE_GateSelect )
			{
				// pixel counter //
				//if ( pt->StartCycle < *_llHBlankStart )
				//{
					// set the cycle hblank start at
					lBlankStartCycle = *_llHBlankStart;
				//}
				
				lBlankEndCycle = *_llNextScanlineStart;
			}
			else
			{
				// hblank counter //
				//bBlank = g->isVBlank ( (double) lStartCycle );
				
				// set the cycle that vblank starts at if known, otherwise use max value
				if ( ( *_lNextScanline & ~1 ) == *_lVBlank_Y )
				{
					// vblank starts at the next scanline, so set as the start of the blanking area, otherwise set to max if not in vblank
					lBlankStartCycle = *_llNextScanlineStart;
				}
				else if ( ( *_lScanline & ~1 ) < *_lVBlank_Y )
				{
					// otherwise not in blanking area, so set to max
					lBlankStartCycle = -1LL;
				}
				// otherwise, must be in blanking area, but need to know if next scanline exits blanking area
				else if ( ( *_lNextScanline & ~1 ) < *_lVBlank_Y )
				{
					// next scanline is the end of blanking area
					lBlankEndCycle = *_llNextScanlineStart;
				}
				
			}


			if ( pt->StartCycle < lBlankStartCycle )
			{
				// not in Blank //
				
				// get blank cycle and offset cycles //
				
				
				// set where blanking area starts as the end cycle //
				lEndCycle = lBlankStartCycle;
				
				// check if passed the current cycle
				if ( lEndCycle > lCurrentCycle )
				{
					// do not pass the current cycle //
					lEndCycle = lCurrentCycle;
				}
				
				
				switch ( MODE_GateMode )
				{
					case 0:
					case 1:
					case 2:
					case 3:
						// counts outside of blanking area //
						pt->StartValue += CountTicks_t<MODE_ClkSelect>( pt->StartCycle, lCurrentCycle );
						break;
				}
				
				
				// now we are at either current cycle or start of blanking area
				pt->StartCycle = lEndCycle;
				
				
				// if the blanking area was just hit, then process //
				// when just reached blank, so for mode 0 do nothing, for mode 1 reset to zero, mode 2 reset to zero, mode 3 switch to free run
				if ( lEndCycle == lBlankStartCycle )
				{
					// gate edge (rising) //
					switch ( MODE_GateMode )
					{
						case 0:
						case 2:
							// do nothing at rising edge of gate signal //
							// pause counter during blank //
							
							// just reached blank and will pause in the code, so do nothing here //
							break;
							
						case 1:
						case 3:
							// reset at rising edge of gate signal //
							// reset counter to zero at blank //
							pt->StartValue = 0;
							break;
							
					} // end switch ( MODE.SyncMode )
				
				} // end if ( lEndCycle == lBlankCycle )
				
				
			} // end if ( !bBlank )

			
			// check if in blank //
			if ( pt->StartCycle >= lBlankStartCycle )
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
				
				switch ( MODE_GateMode )
				{
					case 0:
						// paused during blanking area //
						break;
						
					case 1:
					case 2:
					case 3:
						// counts during blanking area //
						pt->StartValue += CountTicks_t<MODE_ClkSelect>( pt->StartCycle, lCurrentCycle );
						break;
				}
				
			} // end if ( StartCycle >= lBlankCycle )
			
			
			// update cycle we are at //
			//lStartCycle = lEndCycle;
			
		//} // end while ( lStartCycle < lCurrentCycle )
		
		// set the start cycle as the current cycle //
		//StartCycle = *_DebugCycleCount;
		pt->StartCycle = lCurrentCycle;

		// if the blanking area was just hit, then process //
		// when just reached blank, so for mode 0 do nothing, for mode 1 reset to zero, mode 2 reset to zero, mode 3 switch to free run
		if ( lCurrentCycle == lBlankEndCycle )
		{
			// gate edge (falling) //
			switch ( MODE_GateMode )
			{
				case 0:
				case 1:
					// do nothing at falling edge of gate signal //
					// pause counter during blank //
					
					// just reached blank and will pause in the code, so do nothing here //
					break;
					
				case 2:
				case 3:
					// reset counter at falling edge of gate signal //
					// reset counter to zero at blank //
					pt->StartValue = 0;
					break;
					
			} // end switch ( MODE.SyncMode )
		
		} // end if ( lEndCycle == lBlankCycle )
		
	} // end if ( ( !MODE.RunMode ) || ( TimerNumber == 2 ) || ( TimerNumber > 3 ) )
	
	

	lCompareValue = ( (u64) pt->COMP.Compare );
	//lOverflowValue = 0xffffULL;
	

	// get the wrap value
	if ( MODE_CompareEnable && ( lInitialValue <= lCompareValue ) )
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
		// note: can only interrupt on compare if it is not zero
		if ( lCompareValue ) lCompareValue += lWrapValue;
	}
	
	// before wrapping value around, set the "value reached" flags

//#ifdef SET_REACHED_ONLY_ON_INT
	if ( MODE_IrqOnTarget )
	{
//#endif

	// if target value was reached, then set the flag that says so
	if ( lInitialValue < lCompareValue && pt->StartValue >= lCompareValue )
	{

		// *note* it's also possible that this might be only set if an interrupt occurs at target ??
		pt->MODE.IrqOnTarget_Generated = 1;
		
	}

//#ifdef SET_REACHED_ONLY_ON_INT
	}
//#endif
	
	
	if ( !MODE_CompareEnable || ( lInitialValue > lCompareValue ) )
	{
		// if overflow value was reached, then set the flag that says so
		if ( pt->StartValue > OVERFLOW_MASK )
		{
//#ifdef SET_REACHED_ONLY_ON_INT
			if ( MODE_IrqOnOverflow )
			{
//#endif

			// *note* it's also possible that this might be only set if an interrupt occurs at overflow ??
			pt->MODE.IrqOnOverflow_Generated = 1;

#ifndef DISABLE_COMPAREINT_AFTEROVF
				// check for additional irq on target ??
				if ( MODE_IrqOnTarget )
				{
					if ( !pt->COMP.Compare )
					{
//#ifdef INLINE_DEBUG_UPDATE_INT
//	Timers::debug << " COMPAREINTAFTEROVF";
//#endif

						pt->MODE.IrqOnTarget_Generated = 1;
					}
				}
#endif
			
//#ifdef SET_REACHED_ONLY_ON_INT
			}
//#endif

			// can only do this part when counting to target, since it resets the wrap value
			if ( MODE_CompareEnable )
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
	

	if ( MODE_CompareEnable )
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



template<const u32 MODE_CounterEnable, const u32 MODE_ClkSelect, const u32 MODE_GateSelect, const u32 MODE_Gate, const u32 MODE_GateMode, const u32 MODE_CompareEnable, const u32 MODE_IrqOnTarget, const u32 MODE_IrqOnOverflow>
static void Timer::Get_NextEvent_t ( Timer *pt )
{
	const u64 OVERFLOW_MASK = 0xffffULL;
	
	bool bBlank;
	double dOffsetCycles;
	u64 lStartCycle, lEndCycle, lCurrentCycle;
	u64 lBlankStartCycle, lBlankEndCycle;
	u64 lIntCycle;
	u32 lStartValue, lEndValue;
	//u32 lIntValue;
	u32 lTargetValue;
	
	//Timer* pt;
	
	//pt = TimerPtrs [ TIMERNUMBER ];
#ifdef INLINE_DEBUG_NEXTEVENT_T
	Timers::debug << "\r\nGet_NextEvent_t";
#endif
	
	// if there are no interrupts, then no need for any events
	if ( ( !MODE_IrqOnTarget && !MODE_IrqOnOverflow ) || ( !MODE_CounterEnable ) )
	{
		pt->SetNextEventCh_Cycle ( -1LL );
		return;
	}
	
	
	
	// set the final cycle //
	//lCurrentCycle = g->llNextScanlineStart;
	
	// set start cycle //
	lStartCycle = pt->StartCycle;

#ifdef INLINE_DEBUG_NEXTEVENT_T
	Timers::debug << " lStartCycle=" << dec << lStartCycle;
#endif

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

#ifdef INLINE_DEBUG_NEXTEVENT_T
	Timers::debug << " lStartValue=" << dec << lStartValue;
#endif

	// get value where next event should happen
	lTargetValue = pt->Get_NextIntTick_t<MODE_CompareEnable,MODE_IrqOnTarget,MODE_IrqOnOverflow>( lStartValue );
	
#ifdef INLINE_DEBUG_NEXTEVENT_T
	Timers::debug << " lTargetValue=" << dec << lTargetValue;
#endif

	// if in free run mode, or this is timer 2, then can calculate like free run
	if ( !MODE_Gate )
	{
		// timer is in free run //
	
			// get the cycle of the next interrupt for free run timer //
			//lIntCycle = Get_FreeRunNextIntCycle ( lStartValue, lStartCycle );
			lIntCycle = Get_InterruptCycle_t<MODE_ClkSelect>( lStartCycle, lStartValue, lTargetValue );
			
#ifdef INLINE_DEBUG_NEXTEVENT_T
	Timers::debug << " lIntCycle=" << dec << lIntCycle;
#endif

			// set the cycle for the next event for timer
			pt->SetNextEventCh_Cycle ( lIntCycle );
			
			return;
		
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
		lCurrentCycle = *_llNextScanlineStart;
		
		
		// initialize blank cycle
		lBlankStartCycle = 0;
		
		//while ( lStartCycle < lCurrentCycle )
		//{
		
			// check if in blanking area //
			
			// check if timer 0 or 1 //
			// check if in blanking area //
			
			// check if timer 0 or 1 //
			// note: possibly check for timer 3 if ps2 ??
			if ( !MODE_GateSelect )
			{
				// pixel counter //
				//bBlank = g->isHBlank ( (double) lStartCycle );
				//if ( lStartCycle < *_llHBlankStart )
				//{
					// set the cycle hblank start at
					lBlankStartCycle = *_llHBlankStart;
				//}
				
				lBlankEndCycle = *_llNextScanlineStart;
			}
			else
			{
				// hblank counter //
				//bBlank = g->isVBlank ( (double) lStartCycle );
				
				// set the cycle that vblank starts at if known, otherwise use max value
				if ( ( *_lNextScanline & ~1 ) == *_lVBlank_Y )
				{
					// vblank starts at the next scanline, so set as the start of the blanking area, otherwise set to max if not in vblank
					lBlankStartCycle = *_llNextScanlineStart;
				}
				else if ( ( *_lScanline & ~1 ) < *_lVBlank_Y )
				{
					// otherwise not in blanking area, so set to max
					lBlankStartCycle = -1LL;
				}
				else if ( ( *_lNextScanline & ~1 ) < *_lVBlank_Y )
				{
					// otherwise not in blanking area, so set to max
					lBlankEndCycle = *_llNextScanlineStart;
				}
			}

			

			if ( lStartCycle < lBlankStartCycle )
			{
				// not in Blank //
				
				// get blank cycle and offset cycles //
				
				
				// set where blanking area starts as the end cycle //
				lEndCycle = lBlankStartCycle;
				
				
				// check if passed the end of scanline
				if ( lEndCycle > lCurrentCycle )
				{
					// do not pass the current cycle //
					lEndCycle = lCurrentCycle;
				}
				
				
				// get the end value
				switch ( MODE_GateMode )
				{
					case 0:
					case 1:
					case 2:
					case 3:
						// counts outside of blanking area //
						lIntCycle = Get_InterruptCycle_t<MODE_ClkSelect>( lStartCycle, lStartValue, lTargetValue );
						
						// check if the target value has been passed before (or on transition to?) the blanking area
						if ( lIntCycle <= lBlankStartCycle )
						{
							// set an event for when interrupt occurs
							//lIntCycle = Get_InterruptCycle_Scanline ( lStartCycle, lStartValue, lTargetValue );
							pt->SetNextEventCh_Cycle ( lIntCycle );
							
							return;
						}
						
						// update timer value
						lStartValue += CountTicks_t <MODE_ClkSelect>( lStartCycle, lEndCycle );
						
						//lStartValue = lEndValue;
						
						break;
				}
				
				
				
				
				// set new value for timer
				//lStartValue = lEndValue;
				
				// now we are at either current cycle or start of blanking area
				lStartCycle = lEndCycle;

				
				// if the blanking area was just hit, then process //
				// when just reached blank, so for mode 0 do nothing, for mode 1 reset to zero, mode 2 reset to zero, mode 3 switch to free run
				if ( lEndCycle == lBlankStartCycle )
				{
					// gate edge (rising) //
					switch ( MODE_GateMode )
					{
						case 0:
						case 2:
							// pause counter during blank //
							
							// just reached blank and will pause in the code, so do nothing here //
							break;
							
						case 1:
						case 3:
						
							// reset counter to zero at blank //
							lStartValue = 0;
							break;
							
						
							
					} // end switch ( MODE.SyncMode )
				
				} // end if ( lEndCycle == lBlankCycle )
				
				
			} // end if ( !bBlank )

			
			// check if in blank //
			if ( lStartCycle >= lBlankStartCycle )
			{
				// Blank //
				
				// get next end cycle //
				
				switch ( MODE_GateMode )
				{
					case 0:
						// paused during blanking area //
						
						break;
						
					case 1:
					case 2:
					case 3:
					
						// counts during blanking area //
						
						//StartValue += CountTicks_Scanline ( lStartCycle, lEndCycle );
						
						lIntCycle = Get_InterruptCycle_t<MODE_ClkSelect>( lStartCycle, lStartValue, lTargetValue );
						
						// set the cycle for the next event for timer
						pt->SetNextEventCh_Cycle ( lIntCycle );
						
						return;
						
						break;
						
				} // end switch ( MODE.SyncMode )
				
				// if the blanking area was just hit, then process //
				// when just reached blank, so for mode 0 do nothing, for mode 1 reset to zero, mode 2 reset to zero, mode 3 switch to free run
				/*
				if ( lEndCycle == lBlankEndCycle )
				{
					// gate edge (falling) //
					switch ( MODE_GateMode )
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
						
							
							
							break;
							
					} // end switch ( MODE.SyncMode )
				
				} // end if ( lEndCycle == lBlankCycle )
				*/
				
			} // end if ( bBlank )
			
			// update cycle we are at //
			//lStartCycle = lEndCycle;
		//}
	}
	
	// unable to find next cycle timer should interrupt at for now
	pt->SetNextEventCh_Cycle ( -1LL );
}





//template<const u32 MODE_CounterEnable, const u32 MODE_ClkSelect, const u32 MODE_GateSelect, const u32 MODE_Gate, const u32 MODE_GateMode, const u32 MODE_CompareEnable, const u32 MODE_IrqOnTarget, const u32 MODE_IrqOnOverflow>


template<const u32 MODE_CounterEnable, const u32 MODE_ClkSelect, const u32 MODE_GateSelect, const u32 MODE_Gate, const u32 MODE_GateMode, const u32 MODE_CompareEnable, const u32 MODE_IrqOnTarget>
inline void CalibrateTimer7_t ()
{
	switch ( MODE.IrqOnOverflow )
	{
		case 0:
			cbUpdate = UpdateTimer_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,MODE_Gate,MODE_GateMode,MODE_CompareEnable,MODE_IrqOnTarget,0>;
			cbGetNextEvent = Get_NextEvent_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,MODE_Gate,MODE_GateMode,MODE_CompareEnable,MODE_IrqOnTarget,0>;
			break;
			
		case 1:
			cbUpdate = UpdateTimer_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,MODE_Gate,MODE_GateMode,MODE_CompareEnable,MODE_IrqOnTarget,1>;
			cbGetNextEvent = Get_NextEvent_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,MODE_Gate,MODE_GateMode,MODE_CompareEnable,MODE_IrqOnTarget,1>;
			break;
	}
	
	cbUpdate_Sync = UpdateTimer_Scanline_Sync_t<MODE_CounterEnable, ( ( MODE_ClkSelect == 3 ) ? 1 : 0 ), MODE_Gate>;
	cbGetNextEvent_Sync = Get_NextEvent_Scanline_Sync_t<MODE_CounterEnable, ( ( MODE_ClkSelect == 3 ) ? 1 : 0 ), MODE_Gate>;
}



template<const u32 MODE_CounterEnable, const u32 MODE_ClkSelect, const u32 MODE_GateSelect, const u32 MODE_Gate, const u32 MODE_GateMode, const u32 MODE_CompareEnable>
inline void CalibrateTimer6_t ()
{
	switch ( MODE.IrqOnTarget )
	{
		case 0:
			CalibrateTimer7_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,MODE_Gate,MODE_GateMode,MODE_CompareEnable,0>();
			break;
			
		case 1:
			CalibrateTimer7_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,MODE_Gate,MODE_GateMode,MODE_CompareEnable,1>();
			break;
	}
}



template<const u32 MODE_CounterEnable, const u32 MODE_ClkSelect, const u32 MODE_GateSelect, const u32 MODE_Gate, const u32 MODE_GateMode>
inline void CalibrateTimer5_t ()
{
	switch ( MODE.CompareEnable )
	{
		case 0:
			CalibrateTimer6_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,MODE_Gate,MODE_GateMode,0>();
			break;
			
		case 1:
			CalibrateTimer6_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,MODE_Gate,MODE_GateMode,1>();
			break;
	}
}


template<const u32 MODE_CounterEnable, const u32 MODE_ClkSelect, const u32 MODE_GateSelect, const u32 MODE_Gate>
inline void CalibrateTimer4_t ()
{
	switch ( MODE.GateMode )
	{
		case 0:
			CalibrateTimer5_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,MODE_Gate,0>();
			break;
			
		case 1:
			CalibrateTimer5_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,MODE_Gate,1>();
			break;
			
		case 2:
			CalibrateTimer5_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,MODE_Gate,2>();
			break;
			
		case 3:
			CalibrateTimer5_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,MODE_Gate,3>();
			break;
	}
}



template<const u32 MODE_CounterEnable, const u32 MODE_ClkSelect, const u32 MODE_GateSelect>
inline void CalibrateTimer3_t ()
{
	switch ( MODE.Gate )
	{
		case 0:
			CalibrateTimer4_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,0>();
			break;
			
		case 1:
			CalibrateTimer4_t<MODE_CounterEnable,MODE_ClkSelect,MODE_GateSelect,1>();
			break;
	}
}


template<const u32 MODE_CounterEnable, const u32 MODE_ClkSelect>
inline void CalibrateTimer2_t ()
{
	switch ( MODE.GateSelect )
	{
		case 0:
			// h-blank gate //
			
			// make sure counter is not h-blank
			if ( MODE_ClkSelect != 3 )
			{
				CalibrateTimer3_t<MODE_CounterEnable,MODE_ClkSelect,0>();
			}
			else
			{
				CalibrateTimer5_t<MODE_CounterEnable,MODE_ClkSelect,0,0,0>();
			}
			
			break;
			
		case 1:
			// v-blank gate //
			CalibrateTimer3_t<MODE_CounterEnable,MODE_ClkSelect,1>();
			break;
	}
}


template<const u32 MODE_CounterEnable>
inline void CalibrateTimer1_t ()
{
	switch ( MODE.ClkSelect )
	{
		case 0:
			CalibrateTimer2_t<MODE_CounterEnable,0>();
			break;
			
		case 1:
			CalibrateTimer2_t<MODE_CounterEnable,1>();
			break;
			
		case 2:
			CalibrateTimer2_t<MODE_CounterEnable,2>();
			break;
			
		case 3:
			CalibrateTimer2_t<MODE_CounterEnable,3>();
			break;
	}
}

void CalibrateTimer_t ()
{
	switch ( MODE.CounterEnable )
	{
		case 0:
			cbUpdate = UpdateTimer_t<0,0,0,0,0,0,0,0>;
			cbGetNextEvent = Get_NextEvent_t<0,0,0,0,0,0,0,0>;
			cbUpdate_Sync = UpdateTimer_Scanline_Sync_t<0,0,0>;
			cbGetNextEvent_Sync = Get_NextEvent_Scanline_Sync_t<0,0,0>;
			break;
			
		case 1:
			CalibrateTimer1_t<1>();
			break;
	}
}

#endif



	
		
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
		static const u32 Regs_Start = 0x10000000;
		
		// where the timer registers end at
		static const u32 Regs_End = 0x10001830;
	
		// distance between numbered groups of registers
		static const u32 Reg_Size = 0x10;
		
		// index for the next event
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
		
		
		static const u32 c_iNumberOfChannels = 4;
		
		Timer TheTimers [ c_iNumberOfChannels ];
		

		// these allow you to read and write registers and allow the device to act on the read/write event
		static u64 Read ( u32 Address, u64 Mask );
		static void Write ( u32 Address, u64 Data, u64 Mask );

		
		// update what cycle the next event is at for this device
		void Update_NextEventCycle ();

		
		// this gets the next event cycle for the specified timer
		inline void Get_NextEvent ( int TimerNumber )
		{
#ifdef USE_TEMPLATES_PS2_TIMER
			TheTimers [ TimerNumber ].cbGetNextEvent ( & ( TheTimers [ TimerNumber ] ) );
#else
			TheTimers [ TimerNumber ].Get_NextEvent ();
#endif
		}
		
		inline void Get_NextEvent_Sync ( int TimerNumber )
		{
#ifdef USE_TEMPLATES_PS2_TIMER
			TheTimers [ TimerNumber ].cbGetNextEvent_Sync ( & ( TheTimers [ TimerNumber ] ) );
#else
			TheTimers [ TimerNumber ].Get_NextEvent ();
#endif
		}
		
		
		// this updates the specified timer value
		inline void UpdateTimer ( u32 TimerNumber )
		{
#ifdef USE_TEMPLATES_PS2_TIMER
			TheTimers [ TimerNumber ].cbUpdate ( & ( TheTimers [ TimerNumber ] ) );
#else
			TheTimers [ TimerNumber ].UpdateTimer ();
#endif
		}
		
		inline void UpdateTimer_Sync ( u32 TimerNumber )
		{
#ifdef USE_TEMPLATES_PS2_TIMER
			TheTimers [ TimerNumber ].cbUpdate_Sync ( & ( TheTimers [ TimerNumber ] ) );
#else
			TheTimers [ TimerNumber ].UpdateTimer ();
#endif
		}
		

		
		inline void CalibrateTimer ( u32 TimerNumber )
		{
#ifdef USE_TEMPLATES_PS2_TIMER
			TheTimers [ TimerNumber ].CalibrateTimer_t ();
//#else
//			TheTimers [ TimerNumber ].CalibrateTimer ();
#endif
		}
		
		
		inline long GetValue ( u32 TimerNumber ) { return TheTimers [ TimerNumber ].GetValue (); }
		inline long GetMode ( u32 TimerNumber ) { return TheTimers [ TimerNumber ].GetMode (); }
		inline long GetComp ( u32 TimerNumber ) { return TheTimers [ TimerNumber ].GetComp (); }
		
		inline void SetMode ( u32 TimerNumber, u32 Data ) { TheTimers [ TimerNumber ].SetMode ( Data ); }
		inline void SetValue ( u32 TimerNumber, u32 Data ) { TheTimers [ TimerNumber ].SetValue ( Data ); }
		inline void SetComp ( u32 TimerNumber, u32 Data ) { TheTimers [ TimerNumber ].SetComp ( Data ); }
		
		

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
		// Timer X - System/Horizontal Retrace	//
		//////////////////////////////////////////

		// timers 0 and 1 get a signal from the GPU for pixel/hblank
		//static GPU *g;
		
		// this should be called on SBUS interrupt to set the timer hold values
		static void Perform_TimerHold ();
		
		
		// Constructor
		Timers ();
		
		void Reset ();
		
		void Start ();
		
		// use this to set non-static pointers for object
		// also call after loading a save state
		void Refresh ();
		
		// *** IMPORTANT *** this MUST be called after loading a save state due to new timer structure
		// re-calibrate all timers - call this after loading in a save state, since the timers have pointers to some timer functions
		void ReCalibrateAll ();
		
		inline static void ConnectExternalSignal_GPU ( u64 *ScanlineStart, u64 *NextScanlineStart, u64 *HBlankStart, u32 *Scanline, u32 *NextScanline, u32 *VBlank_Y, u32 *RasterYMax )
		{
			Timer::_llScanlineStart = ScanlineStart;
			Timer::_llNextScanlineStart = NextScanlineStart;
			Timer::_llHBlankStart = HBlankStart;
			Timer::_lScanline = Scanline;
			Timer::_lNextScanline = NextScanline;
			Timer::_lVBlank_Y = VBlank_Y;
			Timer::_lRasterYMax = RasterYMax;
		}
		
		inline static void ConnectExternalSignal_Clock ( u64* CycleCount )
		{
			Timer::_llCycleCount = CycleCount;
			Timer::_DebugCycleCount = CycleCount;
			Timers::_DebugCycleCount = CycleCount;
		}
		
		
		
		//void ConnectDevices ( GPU *_g );

		// run for a clock cycle
		// returns interrupt status - whether to interrupt or not
		// needs GPU Status READ register
		// returns interrupt status for INTC
		void Run ();
		
		
		static void sRun () { _TIMERS->Run (); }
		static void Set_EventCallback ( funcVoid2 UpdateEvent_CB ) { _TIMERS->NextEvent_Idx = UpdateEvent_CB ( sRun ); };
		
		
		static u64* _NextSystemEvent;
		
		static const u32 c_InterruptBit = 9;
		
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
		
		inline void SetInterrupt ( const u32 TimerNumber )
		{
			*_Intc_Stat |= ( 1 << ( c_InterruptBit + TimerNumber ) );
			if ( *_Intc_Stat & *_Intc_Mask ) *_R5900_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 0 );
		}
		
		// goes through INTC which is a latch, so no clearing of interrupts from here
		inline void ClearInterrupt ( const u32 TimerNumber )
		{
			//*_Intc_Stat &= ~( 1 << ( c_InterruptBit + TimerNumber ) );
			//if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R5900_Cause_13 &= ~( 1 << 10 );
			
			//if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
		}

		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		static u32* _NextEventIdx;

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


/*
// get cycle that next interrupt would happen at if the timer is counting in clock cycles
inline static u64 Timer::Get_NextIntCycle_Clock ( Timer* t, u64 llStartCycle, long lStartTick, long lNextIntTick )
{
	long lTicksToGo;
	u64 llCycleOffset, llCyclesToGo, llIntCycle;
	
	lTicksToGo = lNextIntTick - lStartTick;
	llCycleOffset = llStartCycle & ( ( 1 << ( t->MODE.ClkSelect << 2 ) ) - 1 );
	llCyclesToGo = ( lTicksToGo << ( t->MODE.ClkSelect << 2 ) ) - llCycleOffset;
	llIntCycle = llStartCycle + llCyclesToGo;
	
	return llIntCycle;
}





// gets the next event for timer.. checks up to "ThroughCycle"
// *important* don't forget to re-check for the next event whenever timer/mode/comp gets set/updated
template<const int MODE_CounterEnable, const int MODE_ClkSelect, const int MODE_Gate, const int MODE_GateSelect, const int MODE_GateMode>
static void Timer::Get_NextEvent ( Timer* t )
{
	long CompareValue;
	long MaxTick, IntTick, NextIntTick, TicksToGo, StartTick;
	long long llCycleOffset, llCyclesToGo, llIntCycle, llStartCycle;
	
	// event is only hit if interrupt is enabled or timer is enabled
	if ( ( !t->MODE.IrqOnTarget && !t->MODE.IrqOnOverflow ) || !MODE_CounterEnable )
	{
		// no events for timer needed //
		t->SetNextEventCh_Cycle ( -1LL );
		return;
	}
	
	// init next int tick
	IntTick = -1;
	
	// get compare value
	CompareValue = t->COMP.Value;
	
	// get maximum tick value
	if ( t->MODE.CompareEnable )
	{
		MaxTick = t->COMP.Value;
	}
	else
	{
		MaxTick = 0xffff;
	}
	
	// check if next tick for interrupt is the compare value //
	
	if ( t->MODE.IrqOnTarget )
	{
		if ( ( t->StartValue < CompareValue ) || !t->MODE.IrqOnOverflow )
		{
			// next timer interrupt is on compare value
			IntTick = CompareValue;
			
			//if ( t->StartValue >= CompareValue )
			//{
			//	// timer needs to wrap around first //
			//	NextIntTick += 0x10000;
			//}
		}
		
	}
	
	// check if next tick for interrupt is on overflow
	if ( t->MODE.IrqOnOverflow )
	{
		// first check if overflow can be reached
		if ( MaxTick == 0xffff )
		{
			// overflow can be reached //
			
			// check if timer value is greater or equal to compare value or there is no compare interrupt
			if ( ( t->StartValue >= CompareValue ) || !t->MODE.IrqOnTarget )
			{
				// next interrupt occurs at overflow //
				IntTick = c_iTimerValueAtOverflow;
			}
		}
	}
	
	// double check that there is an interrupt in the future
	if ( IntTick < 0 )
	{
		// no events for timer needed //
		t->SetNextEventCh_Cycle ( -1LL );
		return;
	}
	
	// get current tick
	StartTick = t->StartValue;
	
	if ( StartTick < IntTick ) NextIntTick = IntTick; else NextIntTick = IntTick + 0x10000;
	
	// get number of ticks to go until event
	TicksToGo = NextIntTick - StartTick;
	
	// set start cycle
	llStartCycle = t->StartCycle;
	
	// check if counting clock cycles or hblanks
	if ( MODE_ClkSelect <= 2 )
	{
		// counting clock cycles //
		
		// check if gate is enabled
		if ( MODE_Gate )
		{
			// gate enabled //
			
			// check if gate is hblank or vblank
			if ( !MODE_GateSelect )
			{
				// gate hblank //
				
				// check gate mode
				switch ( MODE_GateMode )
				{
					case 0:
						// counts while not in blank //
						
						// check if start cycle is before hblank
						if ( llStartCycle < *_llHBlankStart )
						{
							// check if interrupt happens before hblank //
							
							// check if counting 1/16, 1/256, etc
							llIntCycle = Get_NextIntCycle_Clock ( t, llStartCycle, StartTick, NextIntTick );
							
							if ( llIntCycle <= *_llHBlankStart )
							{
								// set interrupt event
								t->SetNextEventCh_Cycle ( llIntCycle );
								return;
							}
						}
						
						break;
						
					case 1:
					
						// resets and starts counting at start of blank //
						
						// check if interrupt occurs before hblank //
						if ( llStartCycle < *_llHBlankStart )
						{
							// count cycles up until hblank //
							
							// check if counting 1/16, 1/256, etc
							llIntCycle = Get_NextIntCycle_Clock ( t, llStartCycle, StartTick, NextIntTick );
							
							if ( llIntCycle < *_llHBlankStart )
							{
								// set interrupt event
								t->SetNextEventCh_Cycle ( llIntCycle );
								return;
							}
							
							llStartCycle = *_llHBlankStart;
							StartTick = 0;
							
							// start tick changed, so recalculate NextIntTick
							if ( StartTick < IntTick ) NextIntTick = IntTick; else NextIntTick = IntTick + 0x10000;
						}
						
						// check if interrupt occurs after hblank //
						llIntCycle = Get_NextIntCycle_Clock ( t, llStartCycle, StartTick, NextIntTick );
						
						// if interrupt is not on scanline or at very beginning of next, then disregard
						if ( llIntCycle <= *_llNextScanlineStart )
						{
							// set interrupt event
							t->SetNextEventCh_Cycle ( llIntCycle );
						}
						
						break;
						
					case 2:
					
						// resets and starts counting on transition out of blank //
						
						// get cycle that interrupt occurs at
						llIntCycle = Get_NextIntCycle_Clock ( t, llStartCycle, StartTick, NextIntTick );
						
						if ( llIntCycle < *_llNextScanlineStart )
						{
							// set interrupt event
							t->SetNextEventCh_Cycle ( llIntCycle );
						}
						
						break;
						
					case 3:
						// resets and starts counting at start of blank and on transition out of blank //
						
						// check if interrupt occurs before hblank //
						if ( llStartCycle < *_llHBlankStart )
						{
							// count cycles up until hblank //
							
							// check if counting 1/16, 1/256, etc
							llIntCycle = Get_NextIntCycle_Clock ( t, llStartCycle, StartTick, NextIntTick );
							
							if ( llIntCycle < *_llHBlankStart )
							{
								// set interrupt event
								t->SetNextEventCh_Cycle ( llIntCycle );
								return;
							}
							
							llStartCycle = *_llHBlankStart;
							StartTick = 0;
							
							// start tick changed, so recalculate NextIntTick
							if ( StartTick < IntTick ) NextIntTick = IntTick; else NextIntTick = IntTick + 0x10000;
						}
						
						// check if interrupt occurs after hblank //
						llIntCycle = Get_NextIntCycle_Clock ( t, llStartCycle, StartTick, NextIntTick );
						
						// if interrupt is not on scanline or at very beginning of next, then disregard.. no, its cool
						if ( llIntCycle < *_llNextScanlineStart )
						{
							// set interrupt event
							t->SetNextEventCh_Cycle ( llIntCycle );
						}
						
						break;
				}
			}
			else
			{
				// gate vblank //
				
				// check gate mode
				switch ( MODE_GateMode )
				{
					case 0:
						// counts while not in blank //
						
						// check if scanline is NOT in vblank
						if ( *_lScanline < *_lVBlank_Y )
						{
							// get cycle that interrupt occurs at
							llIntCycle = Get_NextIntCycle_Clock ( t, llStartCycle, StartTick, NextIntTick );
							
							if ( llIntCycle <= *_llNextScanlineStart )
							{
								// set interrupt event
								t->SetNextEventCh_Cycle ( llIntCycle );
							}
						}
						
						break;
						
					case 1:
						// resets and starts counting at start of blank //
						
						// get cycle that interrupt occurs at
						llIntCycle = Get_NextIntCycle_Clock ( t, llStartCycle, StartTick, NextIntTick );
						
						// check if current scanline is next to blank
						if ( ( *_lScanline + 2 ) >= *_lVBlank_Y )
						{
							// counter resets at start of next scanline
							if ( llIntCycle < *_llNextScanlineStart )
							{
								// set interrupt event
								t->SetNextEventCh_Cycle ( llIntCycle );
							}
						}
						else
						{
							// counter does not reset at start of next scanline
							if ( llIntCycle <= *_llNextScanlineStart )
							{
								// set interrupt event
								t->SetNextEventCh_Cycle ( llIntCycle );
							}
						}
						
						break;
						
					case 2:
						// resets and starts counting on transition out of blank //
						
						// get cycle that interrupt occurs at
						llIntCycle = Get_NextIntCycle_Clock ( t, llStartCycle, StartTick, NextIntTick );
						
						// check if current scanline is next to last
						if ( ( *_lScanline + 2 ) >= *_lRasterYMax )
						{
							// check if current cycle is at next scanline start
							if ( llIntCycle < *_llNextScanlineStart )
							{
								// set interrupt event
								t->SetNextEventCh_Cycle ( llIntCycle );
							}
						}
						else
						{
							if ( llIntCycle <= *_llNextScanlineStart )
							{
								// set interrupt event
								t->SetNextEventCh_Cycle ( llIntCycle );
							}
						}
						
						break;
						
					case 3:
						// resets and starts counting at start of blank and on transition out of blank //
						
						// check if current scanline is next to blank
						if ( ( *_lScanline + 2 ) >= *_lVBlank_Y )
						{
							// check if current cycle is at next scanline start
							if ( *_llCycleCount >= *_llNextScanlineStart )
							{
								// reset counter
								t->StartValue = 0;
								t->StartCycle = *_llCycleCount;
							}
						}
						
						// check if current scanline is next to last
						if ( ( *_lScanline + 2 ) >= *_lRasterYMax )
						{
							// check if current cycle is at next scanline start
							if ( *_llCycleCount >= *_llNextScanlineStart )
							{
								// reset counter
								t->StartValue = 0;
								t->StartCycle = *_llCycleCount;
							}
						}
						
						
						break;
				}
			}
		}
		else
		{
			// gate is disabled //
			
			// get cycle that interrupt occurs at
			llIntCycle = Get_NextIntCycle_Clock ( t, llStartCycle, StartTick, NextIntTick );
			
			if ( llIntCycle <= *_llNextScanlineStart )
			{
				// set interrupt event
				t->SetNextEventCh_Cycle ( llIntCycle );
			}
			
		}
	}
	else
	{
		// counting hblanks //
		
		// check that interrupt is possible on scanline
		if ( ( StartTick + 1 ) != NextIntTick )
		{
			// interrupt not possible on scanline //
			t->SetNextEventCh_Cycle ( -1LL );
			return;
		}
		
		// check if gate function is enabled for vblank (hblank is disabled since we are counting hblanks)
		// check if gate is enabled
		if ( MODE_Gate && MODE_GateSelect )
		{
			// gate enabled (vblank) //
			
			// gate vblank //
			
			// check gate mode
			switch ( MODE_GateMode )
			{
				case 0:
					// counts while not in vblank //
					
					// check if scanline is NOT in vblank
					if ( *_lScanline < *_lVBlank_Y )
					{
						// count cycles
						if ( llStartCycle < *_llHBlankStart )
						{
							// interrupt happens at hblank on scanline
							t->SetNextEventCh_Cycle ( *_llHBlankStart );
							//t->StartCycle++;
						}
					}
					
					// update cycle
					//t->StartCycle = *_llCycleCount;
					
					break;
					
				case 1:
					// resets and starts counting at start of blank //
				case 2:
					// resets and starts counting on transition out of blank //
				case 3:
					// resets and starts counting at start of blank and on transition out of blank //
					
					// count cycles
					if ( llStartCycle < *_llHBlankStart )
					{
						// interrupt happens at hblank on scanline
						t->SetNextEventCh_Cycle ( *_llHBlankStart );
						//t->StartCycle++;
					}
					
					//t->StartCycle = *_llCycleCount;
					
					break;
			}
		}
		else
		{
			// gate is disabled //
			
			// interrupt happens at hblank
			if ( llStartCycle < *_llHBlankStart )
			{
				// interrupt happens at hblank on scanline
				t->SetNextEventCh_Cycle ( *_llHBlankStart );
				//t->StartCycle++;
			}
			
			// update timer cycle
			//t->StartCycle = *_llCycleCount;
		}
	}
	
	// unable to find next cycle timer should interrupt at for now
	//SetNextEventCh_Cycle ( 0 );
}











// the only way to properly simulate a PS2 timer it appears at this point is to update it per scanline... scanline start is probably best
// update per scanline
// needs MODE_CounterEnable, MODE_ClkSelect, MODE_Gate, MODE_GateSelect, MODE_GateMode
template<const int MODE_CounterEnable, const int MODE_ClkSelect, const int MODE_Gate, const int MODE_GateSelect, const int MODE_GateMode>
static void Timer::UpdateTimer ( Timer* t )
{
	long CompareValue;
	
	// if timer is not enabled, then set cycle and done
	if ( !MODE_CounterEnable )
	{
		t->StartCycle = *_llCycleCount;
		return;
	}
	
	// check if counting clock cycles or hblanks
	if ( MODE_ClkSelect <= 2 )
	{
		// counting clock cycles //
		
		// check if gate is enabled
		if ( MODE_Gate )
		{
			// gate enabled //
			
			// check if gate is hblank or vblank
			if ( !MODE_GateSelect )
			{
				// gate hblank //
				
				// check gate mode
				switch ( MODE_GateMode )
				{
					case 0:
						// counts while not in blank //
						
						// check if start cycle is before hblank
						if ( t->StartCycle < *_llHBlankStart )
						{
							// count cycles up until hblank //
							
							// check if counting 1/16, 1/256, etc
							// update timer
							t->StartValue += ( *_llHBlankStart - t->StartCycle + ( t->StartCycle & ( ( 1 << ( MODE_ClkSelect << 2 ) ) - 1 ) ) ) >> ( MODE_ClkSelect << 2 );
						}
						
						// update cycle
						t->StartCycle = *_llCycleCount;
						
						break;
						
					case 1:
						// resets and starts counting at start of blank //
						
						// check if current cycle is at or after hblank
						if ( *_llCycleCount >= *_llHBlankStart )
						{
							t->StartValue = 0;
							t->StartCycle = *_llHBlankStart;
						}
						
						// count cycles //
						t->StartValue += ( *_llCycleCount - t->StartCycle + ( t->StartCycle & ( ( 1 << ( MODE_ClkSelect << 2 ) ) - 1 ) ) ) >> ( MODE_ClkSelect << 2 );
						
						// update cycle
						t->StartCycle = *_llCycleCount;
						
						break;
						
					case 2:
					
						// resets and starts counting on transition out of blank //
						
						// check if current cycle is at or after the start of next scanline
						if ( *_llCycleCount >= *_llNextScanlineStart )
						{
							// reset counter
							t->StartValue = 0;
							t->StartCycle = *_llCycleCount;
						}
						else
						{
							// count cycles
							t->StartValue += ( *_llCycleCount - t->StartCycle + ( t->StartCycle & ( ( 1 << ( MODE_ClkSelect << 2 ) ) - 1 ) ) ) >> ( MODE_ClkSelect << 2 );
						}
						
						break;
						
					case 3:
						// resets and starts counting at start of blank and on transition out of blank //
						
						// check if current cycle is at or after the start of hblank
						if ( *_llCycleCount >= *_llHBlankStart )
						{
							t->StartValue = 0;
							t->StartCycle = *_llHBlankStart;
						}
						
						// check if current cycle is at or after the start of next scanline
						if ( *_llCycleCount >= *_llNextScanlineStart )
						{
							// reset counter
							t->StartValue = 0;
							t->StartCycle = *_llCycleCount;
						}
						else
						{
							// count cycles
							t->StartValue += ( *_llCycleCount - t->StartCycle + ( t->StartCycle & ( ( 1 << ( MODE_ClkSelect << 2 ) ) - 1 ) ) ) >> ( MODE_ClkSelect << 2 );
							t->StartCycle = *_llCycleCount;
						}
						
						break;
				}
			}
			else
			{
				// gate vblank //
				
				// check gate mode
				switch ( MODE_GateMode )
				{
					case 0:
						// counts while not in blank //
						
						// check if scanline is NOT in vblank
						if ( *_lScanline < *_lVBlank_Y )
						{
							// count cycles
							t->StartValue += ( *_llCycleCount - t->StartCycle + ( t->StartCycle & ( ( 1 << ( MODE_ClkSelect << 2 ) ) - 1 ) ) ) >> ( MODE_ClkSelect << 2 );
						}
						
						// update cycle
						t->StartCycle = *_llCycleCount;
						
						break;
						
					case 1:
						// resets and starts counting at start of blank //
						
						// check if current scanline is next to blank
						if ( ( *_lScanline + 2 ) >= *_lVBlank_Y )
						{
							// check if current cycle is at next scanline start
							if ( *_llCycleCount >= *_llNextScanlineStart )
							{
								// reset counter
								t->StartValue = 0;
								t->StartCycle = *_llCycleCount;
							}
						}
						
						// count cycles
						t->StartValue += ( *_llCycleCount - t->StartCycle + ( t->StartCycle & ( ( 1 << ( MODE_ClkSelect << 2 ) ) - 1 ) ) ) >> ( MODE_ClkSelect << 2 );
						t->StartCycle = *_llCycleCount;
							
						break;
						
					case 2:
						// resets and starts counting on transition out of blank //
						
						// check if current scanline is next to last
						if ( ( *_lScanline + 2 ) >= *_lRasterYMax )
						{
							// check if current cycle is at next scanline start
							if ( *_llCycleCount >= *_llNextScanlineStart )
							{
								// reset counter
								t->StartValue = 0;
								t->StartCycle = *_llCycleCount;
							}
						}
						
						// count cycles
						t->StartValue += ( *_llCycleCount - t->StartCycle + ( t->StartCycle & ( ( 1 << ( MODE_ClkSelect << 2 ) ) - 1 ) ) ) >> ( MODE_ClkSelect << 2 );
						t->StartCycle = *_llCycleCount;
						
						break;
						
					case 3:
						// resets and starts counting at start of blank and on transition out of blank //
						
						// check if current scanline is next to blank
						if ( ( *_lScanline + 2 ) >= *_lVBlank_Y )
						{
							// check if current cycle is at next scanline start
							if ( *_llCycleCount >= *_llNextScanlineStart )
							{
								// reset counter
								t->StartValue = 0;
								t->StartCycle = *_llCycleCount;
							}
						}
						
						// check if current scanline is next to last
						if ( ( *_lScanline + 2 ) >= *_lRasterYMax )
						{
							// check if current cycle is at next scanline start
							if ( *_llCycleCount >= *_llNextScanlineStart )
							{
								// reset counter
								t->StartValue = 0;
								t->StartCycle = *_llCycleCount;
							}
						}
						
						// count cycles
						t->StartValue += ( *_llCycleCount - t->StartCycle + ( t->StartCycle & ( ( 1 << ( MODE_ClkSelect << 2 ) ) - 1 ) ) ) >> ( MODE_ClkSelect << 2 );
						t->StartCycle = *_llCycleCount;
						
						break;
				}
			}
		}
		else
		{
			// gate is disabled //
			
			// update timer value
			t->StartValue += ( *_llCycleCount - t->StartCycle + ( t->StartCycle & ( ( 1 << ( MODE_ClkSelect << 2 ) ) - 1 ) ) ) >> ( MODE_ClkSelect << 2 );
			
			// update timer cycle
			t->StartCycle = *_llCycleCount;
		}
		
		// get compare value
		if ( t->MODE.CompareEnable )
		{
			CompareValue = t->COMP.Value;
		}
		else
		{
			CompareValue = 0xffff;
		}
		
		// wrap timer //
		if ( t->StartValue > CompareValue )
		{
			t->StartValue -= ( CompareValue + 1 );
			
			// check if that was not enough wrapping
			if ( t->StartValue > CompareValue )
			{
				t->StartValue %= ( CompareValue + 1 );
			}
		}
	}
	else
	{
		// counting hblanks //
		
		// check if gate function is enabled for vblank (hblank is disabled since we are counting hblanks)
		// check if gate is enabled
		if ( MODE_Gate && MODE_GateSelect )
		{
			// gate enabled (vblank) //
			
			// gate vblank //
			
			// check gate mode
			switch ( MODE_GateMode )
			{
				case 0:
					// counts while not in vblank //
					
					// check if scanline is NOT in vblank
					if ( *_lScanline < *_lVBlank_Y )
					{
						// count cycles
						if ( t->StartCycle < *_llHBlankStart )
						{
							if ( *_llCycleCount >= *_llHBlankStart )
							{
								// passed hblank
								t->StartValue++;
							}
						}
					}
					
					// update cycle
					t->StartCycle = *_llCycleCount;
					
					break;
					
				case 1:
					// resets and starts counting at start of blank //
					
					// check if current scanline is next to blank
					if ( ( *_lScanline + 2 ) >= *_lVBlank_Y )
					{
						// check if current cycle is at next scanline start
						if ( *_llCycleCount >= *_llNextScanlineStart )
						{
							// reset counter
							t->StartValue = 0;
							t->StartCycle = *_llCycleCount;
						}
					}
					
					// count cycles
					if ( t->StartCycle < *_llHBlankStart )
					{
						if ( *_llCycleCount >= *_llHBlankStart )
						{
							// passed hblank
							t->StartValue++;
						}
					}
					
					t->StartCycle = *_llCycleCount;
					
					break;
					
				case 2:
					// resets and starts counting on transition out of blank //
					
					// check if current scanline is next to last
					if ( ( *_lScanline + 2 ) >= *_lRasterYMax )
					{
						// check if current cycle is at next scanline start
						if ( *_llCycleCount >= *_llNextScanlineStart )
						{
							// reset counter
							t->StartValue = 0;
							t->StartCycle = *_llCycleCount;
						}
					}
					
					// count cycles
					if ( t->StartCycle < *_llHBlankStart )
					{
						if ( *_llCycleCount >= *_llHBlankStart )
						{
							// passed hblank
							t->StartValue++;
						}
					}
					
					t->StartCycle = *_llCycleCount;
					
					break;
					
				case 3:
					// resets and starts counting at start of blank and on transition out of blank //
					
					// check if current scanline is next to blank
					if ( ( *_lScanline + 2 ) >= *_lVBlank_Y )
					{
						// check if current cycle is at next scanline start
						if ( *_llCycleCount >= *_llNextScanlineStart )
						{
							// reset counter
							t->StartValue = 0;
							t->StartCycle = *_llCycleCount;
						}
					}
					
					// check if current scanline is next to last
					if ( ( *_lScanline + 2 ) >= *_lRasterYMax )
					{
						// check if current cycle is at next scanline start
						if ( *_llCycleCount >= *_llNextScanlineStart )
						{
							// reset counter
							t->StartValue = 0;
							t->StartCycle = *_llCycleCount;
						}
					}
					
					// count cycles
					if ( t->StartCycle < *_llHBlankStart )
					{
						if ( *_llCycleCount >= *_llHBlankStart )
						{
							// passed hblank
							t->StartValue++;
						}
					}
					
					t->StartCycle = *_llCycleCount;
					
					break;
			}
		}
		else
		{
			// gate is disabled //
			
			// update timer value
			if ( t->StartCycle < *_llHBlankStart )
			{
				if ( *_llCycleCount >= *_llHBlankStart )
				{
					// passed hblank
					t->StartValue++;
				}
			}
			
			// update timer cycle
			t->StartCycle = *_llCycleCount;
		}
		
		
		// get compare value
		if ( t->MODE.CompareEnable )
		{
			CompareValue = t->COMP.Value;
		}
		else
		{
			CompareValue = 0xffff;
		}
		
		// wrap timer //
		if ( t->StartValue > CompareValue )
		{
			t->StartValue -= ( CompareValue + 1 );
		}
	}
	
	// timer has been updated at this point //

}


template<const int MODE_CounterEnable, const int MODE_ClkSelect, const int MODE_Gate, const int MODE_GateSelect, const int MODE_GateMode>
inline void Timer::CalibrateTimer5 ()
{
	cbUpdate = UpdateTimer <MODE_CounterEnable,MODE_ClkSelect,MODE_Gate,MODE_GateSelect,MODE_GateMode>;
	cbGetNextEvent = Get_NextEvent <MODE_CounterEnable,MODE_ClkSelect,MODE_Gate,MODE_GateSelect,MODE_GateMode>;
}


template<const int MODE_CounterEnable, const int MODE_ClkSelect, const int MODE_Gate, const int MODE_GateSelect>
inline void Timer::CalibrateTimer4 ()
{
	switch ( MODE.ClkSelect )
	{
		case 0:
			CalibrateTimer5 <MODE_CounterEnable,MODE_ClkSelect,MODE_Gate,MODE_GateSelect,0> ();
			break;
			
		case 1:
			CalibrateTimer5 <MODE_CounterEnable,MODE_ClkSelect,MODE_Gate,MODE_GateSelect,1> ();
			break;
			
		case 2:
			CalibrateTimer5 <MODE_CounterEnable,MODE_ClkSelect,MODE_Gate,MODE_GateSelect,2> ();
			break;
			
		case 3:
			CalibrateTimer5 <MODE_CounterEnable,MODE_ClkSelect,MODE_Gate,MODE_GateSelect,3> ();
			break;
	}
}


template<const int MODE_CounterEnable, const int MODE_ClkSelect, const int MODE_Gate>
inline void Timer::CalibrateTimer3 ()
{
	switch ( MODE.GateSelect )
	{
		case 0:
			CalibrateTimer4 <MODE_CounterEnable,MODE_ClkSelect,MODE_Gate,0> ();
			break;
			
		case 1:
			CalibrateTimer4 <MODE_CounterEnable,MODE_ClkSelect,MODE_Gate,1> ();
			break;
	}
}


template<const int MODE_CounterEnable, const int MODE_ClkSelect>
inline void Timer::CalibrateTimer2 ()
{
	switch ( MODE.Gate )
	{
		case 0:
			CalibrateTimer3 <MODE_CounterEnable,MODE_ClkSelect,0> ();
			break;
			
		case 1:
			CalibrateTimer3 <MODE_CounterEnable,MODE_ClkSelect,1> ();
			break;
	}
}

template<const int MODE_CounterEnable>
inline void Timer::CalibrateTimer1 ()
{
	switch ( MODE.ClkSelect )
	{
		case 0:
			CalibrateTimer2 <MODE_CounterEnable,0> ();
			break;
			
		case 1:
			CalibrateTimer2 <MODE_CounterEnable,1> ();
			break;
			
		case 2:
			CalibrateTimer2 <MODE_CounterEnable,2> ();
			break;
			
		case 3:
			CalibrateTimer2 <MODE_CounterEnable,3> ();
			break;
	}
}
*/





//---------------------------------------------------------------------------




	
};



#endif

