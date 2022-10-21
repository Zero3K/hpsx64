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


//#include "types.h"

#include "PS2_Timer.h"
//#include <math.h>
//#include "Reciprocal.h"

// unsure if compare interrupt should get triggered on the same cycle and overflow interrupt is triggered
// note: this is also in the Header file
//#define DISABLE_COMPAREINT_AFTEROVF

//#define USE_NEW_TIMER_CODE

// this disables compare interrupts when the compare value is zero
//#define DISABLE_COMPARE_INT_ON_ZERO




#ifdef _DEBUG_VERSION_

// enable debugging
#define INLINE_DEBUG_ENABLE

/*
#define INLINE_DEBUG_READ
#define INLINE_DEBUG_WRITE
#define INLINE_DEBUG_RUN

//#define INLINE_DEBUG_UPDATE
//#define INLINE_DEBUG_UPDATE_INT
//#define INLINE_DEBUG_NEXTEVENT_T
//#define INLINE_DEBUG_NEXTINTTICK


//#define INLINE_DEBUG_CALIBRATE
//#define INLINE_DEBUG_EVENT

//#define INLINE_DEBUG_RUN_VBLANK
//#define INLINE_DEBUG
*/

#endif


using namespace Playstation2;
//using namespace Math::Reciprocal;

//#define CONVERT_TO_FIXED1PT63( x )	( ( (u64) ( ( x ) * ( ( (u64) 1 ) << 63 ) ) ) + ( 1 << 8 ) )

//const char* TimerNameList [ 4 ] = { "Timer0_Log.txt", "Timer1_Log.txt", "Timer2_Log.txt", "Timer3_Log.txt" };


unsigned long long *Timer::_llCycleCount;
unsigned long long *Timer::_llScanlineStart, *Timer::_llNextScanlineStart, *Timer::_llHBlankStart;
unsigned long *Timer::_lScanline, *Timer::_lNextScanline, *Timer::_lVBlank_Y, *Timer::_lRasterYMax;

u32* Timer::_DebugPC;
u64* Timer::_DebugCycleCount;

Timer* Timer::TimerPtrs [ c_iNumberOfChannels ];


u32* Timers::_DebugPC;
u64* Timers::_DebugCycleCount;
u32* Timers::_NextEventIdx;


u64* Timers::_NextSystemEvent;

u32* Timers::_Intc_Stat;
u32* Timers::_Intc_Mask;
//u32* Timers::_Intc_Master;
u32* Timers::_R5900_Status_12;
u32* Timers::_R5900_Cause_13;
u64* Timers::_ProcStatus;


int Timer::Count = 0;
Debug::Log Timers::debug;

Timers *Timers::_TIMERS;



//GPU *Timer::g;
//GPU *Timers::g;


bool Timers::DebugWindow_Enabled;
WindowClass::Window *Timers::DebugWindow;
DebugValueList<u32> *Timers::Timer_ValueList;


Timer::Timer ()
{
/*
	debug.Create ( TimerNameList [ Index ] );

#ifdef INLINE_DEBUG
	debug << "\r\nEntering Timer" << Index << " constructor";
#endif
*/

	Reset ();
	
	//Number = Count++;
	
/*	
#ifdef INLINE_DEBUG
	debug << "->Exiting Timer" << Number << " constructor";
#endif
*/
}

void Timer::Reset ()
{
/*
#ifdef INLINE_DEBUG
	debug << "\r\nEntering Timer" << Number << "::Reset";
#endif
*/

	// zero object
	memset ( this, 0, sizeof( Timer ) );
	
/*	
#ifdef INLINE_DEBUG
	debug << "->Exiting Timer" << Number << "::Reset";
#endif
*/
}


void Timer::Set_TimerNumber ( int lTimerIndex, u32 lTimerNumber )
{
	Index = lTimerIndex;
	TimerNumber = lTimerNumber;
}



/*
void Timers::ConnectDevices ( Playstation1::GPU* _g )
{
	g = _g;
	Timer::g = _g;
}
*/


Timers::Timers ()
{
	cout << "Running Timers constructor...\n";

/*
#ifdef INLINE_DEBUG_ENABLE
	debug.Create ( "Timers_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering Timers constructor";
#endif


	Reset ();
	
	_TIMERS = this;
	
#ifdef INLINE_DEBUG
	debug << "->Exiting Timers constructor";
#endif
*/

}



void Timers::Start ()
{
	cout << "Running Timers::Start...\n";

#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create ( "PS2_Timers_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering Timers::Start";
#endif


	Reset ();
	
	_TIMERS = this;

	
	cout << "\nSetting timer numbers";
	
	/*
	Timer::TimerPtrs [ 0 ] = & ( TheTimers [ 0 ] );
	Timer::TimerPtrs [ 1 ] = & ( TheTimers [ 1 ] );
	Timer::TimerPtrs [ 2 ] = & ( TheTimers [ 2 ] );
	Timer::TimerPtrs [ 3 ] = & ( TheTimers [ 3 ] );
	*/
	
	// set timer ptrs
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		Timer::TimerPtrs [ i ] = & ( TheTimers [ i ] );
	}
	
	
	/*
	TheTimers [ 0 ].Set_TimerNumber ( 0, 0 );
	TheTimers [ 1 ].Set_TimerNumber ( 1, 1 );
	TheTimers [ 2 ].Set_TimerNumber ( 2, 2 );
	TheTimers [ 3 ].Set_TimerNumber ( 3, 3 );
	*/

	// set the timer numbers
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		TheTimers [ i ].Set_TimerNumber ( i, i );
	}
	
	
	cout << "\nCalibrating timers";
	
	/*
	CalibrateTimer ( 0 );
	CalibrateTimer ( 1 );
	CalibrateTimer ( 2 );
	CalibrateTimer ( 3 );
	*/
	
	// calibrate the timers
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		CalibrateTimer ( i );
	}
	
	
	cout << "\nClearing timer values";
	
	/*
	//SetTimerValue ( 0, 0 );
	//SetTimerValue ( 1, 0 );
	//SetTimerValue ( 2, 0 );
	SetValue ( 0, 0 );
	SetValue ( 1, 0 );
	SetValue ( 2, 0 );
	SetValue ( 3, 0 );
	*/
	
	// set the timer values
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		SetValue ( i, 0 );
	}
	
	
	// clear events
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		TheTimers [ i ].SetNextEventCh_Cycle ( -1ULL );
	}

	
	// get the next event for each timer (should be none here)
	//GetNextEvent ( 0 );
	//GetNextEvent ( 1 );
	//GetNextEvent ( 2 );



	
	cout << "->Exiting Timers::Start";
	
#ifdef INLINE_DEBUG
	debug << "->Exiting Timers::Start";
#endif
}


void Timers::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( Timers ) );
}


void Timers::Run ()
{
	// *** TODO *** need to account for when irq is set on both target and 0xffff/Overflow
	// *** TODO *** one shot mode would only trigger the first one in that case also

	//u32 TimerTarget;
	//u32 TimerValue;
	
	u32 TimerNumber;

	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;
	
	// event triggered //
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nPS2::Timers::Run";
#endif
	
	for ( TimerNumber = 0; TimerNumber < c_iNumberOfChannels; TimerNumber++ )
	{

		// check if it is a timer event
		//if ( TheTimers [ TimerNumber ].NextEvent_Cycle == NextEvent_Cycle )
		if ( TheTimers [ TimerNumber ].NextEvent_Cycle == *_DebugCycleCount )
		{
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nInterrupt; Timer#" << TimerNumber << " Cycle#" << dec << *_DebugCycleCount;
#endif

			// event for timer is being handled, so clear events for timer
			TheTimers [ TimerNumber ].SetNextEventCh_Cycle ( -1ULL );
			

			// first update timer to get current value //

#ifdef USE_TEMPLATES_PS2_TIMER
			// update the timer
			TheTimers [ TimerNumber ].cbUpdate ( & ( TheTimers [ TimerNumber ] ) );
#else
			TheTimers [ TimerNumber ].UpdateTimer ();
#endif
			
#ifndef USE_NEW_TIMER_CODE

			// if timer value is zero, then it is overflow, so mark that first //
			
			if ( ( TheTimers [ TimerNumber ].MODE.IrqOnOverflow ) && !( TheTimers [ TimerNumber ].StartValue & 0xffff ) )
			{
				//if ( !( TheTimers [ TimerNumber ].StartValue & 0xffff ) )
				{
					// generate a timer interrupt for an overflow event
					SetInterrupt ( TimerNumber );
					
					// set the overflow interrupt generated flag
					TheTimers [ TimerNumber ].MODE.IrqOnOverflow_Generated = 1;
				}
			}
			
			// check if it is a compare interrupt (can also happen on overflow, or not?) //

			
			if ( TheTimers [ TimerNumber ].MODE.IrqOnTarget )
			{
#ifdef DISABLE_COMPAREINT_AFTEROVF
				if ( TheTimers [ TimerNumber ].COMP.Compare || !TheTimers [ TimerNumber ].MODE.IrqOnOverflow )
				{
#endif

					if ( ( TheTimers [ TimerNumber ].StartValue & 0xffff ) == TheTimers [ TimerNumber ].COMP.Compare )
					{
						// generate a timer interrupt for a target event
						SetInterrupt ( TimerNumber );
						
						// set the target interrupt generated flag
						TheTimers [ TimerNumber ].MODE.IrqOnTarget_Generated = 1;
					}
					
#ifdef DISABLE_COMPAREINT_AFTEROVF
				}
#endif
			}

#else

			// generate a timer interrupt for an event
			SetInterrupt ( TimerNumber );

#endif
			
			// finally, get the cycle number that next event happens at //

#ifdef USE_TEMPLATES_PS2_TIMER
			// get the cycle number for the next event after this one
			TheTimers [ TimerNumber ].cbGetNextEvent ( & ( TheTimers [ TimerNumber ] ) );
#else
			TheTimers [ TimerNumber ].Get_NextEvent ();
#endif
			
			/*
			// check what the event is thats supposed to happen
			switch ( TheTimers [ TimerNumber ].EventType )
			{
				case Timer::TIMER_COMPARE_INT:
				
					// generate a timer interrupt for a compare event
					SetInterrupt ( TimerNumber );
					
					// set the compare interrupt generated flag
					TheTimers [ TimerNumber ].MODE.IrqOnTarget_Generated = 1;
					
					// update the timer
					TheTimers [ TimerNumber ].cbUpdate ( & ( TheTimers [ TimerNumber ] ) );
					
					// get the cycle number for the next event after this one
					TheTimers [ TimerNumber ].cbGetNextEvent ( & ( TheTimers [ TimerNumber ] ) );
					
					break;
					
				case Timer::TIMER_OVERFLOW_INT:
				
					// *** TODO *** check if also set to interrupt on compare with a compare value of zero
				
					// generate a timer interrupt for an overflow event
					SetInterrupt ( TimerNumber );
					
					// set the overflow interrupt generated flag
					TheTimers [ TimerNumber ].MODE.IrqOnOverflow_Generated = 1;
					
					// update the timer
					TheTimers [ TimerNumber ].cbUpdate ( & ( TheTimers [ TimerNumber ] ) );
					
					// get the cycle number for the next event after this one
					TheTimers [ TimerNumber ].cbGetNextEvent ( & ( TheTimers [ TimerNumber ] ) );
					
					break;
					
			}
			*/

			
#ifdef INLINE_DEBUG_RUN
	debug << "; NextEvent_Cycle=" << dec << TheTimers [ TimerNumber ].NextEvent_Cycle;
#endif

		}
		
	}

}




u64 Timers::Read ( u32 Address, u64 Mask )
{
#ifdef INLINE_DEBUG_READ
	debug << "\r\nTimers::Read CycleCount=" << dec << *_DebugCycleCount << " Address=" << hex << Address;
#endif

	u32 TimerNumber, Output;
	
	// get the timer number (fixed for ps2)
	TimerNumber = ( Address >> 11 ) & 3;
	
	// switch fixed for ps2
	switch ( ( Address >> 4 ) & 0xf )
	{
		case 0:	// READ: COUNT
		
			// update timer //
			_TIMERS->UpdateTimer ( TimerNumber );
			
			// get the current value of the timer //
			//_TIMERS->TheTimers [ TimerNumber ].COUNT.Value = _TIMERS->GetTimerValue ( TimerNumber );
			_TIMERS->TheTimers [ TimerNumber ].COUNT.Value = _TIMERS->TheTimers [ TimerNumber ].StartValue;
			
#ifdef INLINE_DEBUG_READ
			//if ( TimerNumber == 3 )
			//{
			debug << "; T" << dec << TimerNumber << "_COUNT = " << hex << _TIMERS->TheTimers [ TimerNumber ].COUNT.Value;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
			//}
#endif

			return _TIMERS->TheTimers [ TimerNumber ].COUNT.Value;
			break;
			
		case 1:	// READ: MODE
#ifdef INLINE_DEBUG_READ
			//if ( TimerNumber == 3 )
			//{
			debug << "; T" << dec << TimerNumber << "_MODE = " << hex << _TIMERS->TheTimers [ TimerNumber ].MODE.Value;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
			//}
#endif

			// update timer before reading mode, to update previous flags (reached target/overflow flags, etc.) for reading
			// for PS2, the "reached" flags only update on overflow, so no need to update when reading mode
			// actually, the reached flags should update only if interrupt is enabled
			_TIMERS->UpdateTimer ( TimerNumber );
		
			// get the value of MODE register
			//return _TIMERS->TheTimers [ TimerNumber ].MODE.Value;
			Output = _TIMERS->TheTimers [ TimerNumber ].MODE.Value;
			
			// from Nocash PSX Specifications
			// bits 11 and 12 get reset after reading MODE register
			// not for PS2
			//_TIMERS->TheTimers [ TimerNumber ].MODE.Value &= ~( ( 1 << 11 ) | ( 1 << 12 ) );
			
			
			return Output;
			
			break;
			
		case 2:	// READ: COMP
#ifdef INLINE_DEBUG_READ
			//if ( TimerNumber == 3 )
			//{
			debug << "; T" << dec << TimerNumber << "_COMP = " << hex << _TIMERS->TheTimers [ TimerNumber ].COMP.Value;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
			//}
#endif
		
			// get the value of COMP register
			return _TIMERS->TheTimers [ TimerNumber ].COMP.Value;
			break;
			
		case 3:	// READ: HOLD
#ifdef INLINE_DEBUG_READ
			//if ( TimerNumber == 3 )
			//{
			debug << "; T" << dec << TimerNumber << "_HOLD = " << hex << _TIMERS->TheTimers [ TimerNumber ].HOLD.Value;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
			//}
#endif
		
			// only for timers 0 and 1
			if ( TimerNumber <= 1 )
			{
				return _TIMERS->TheTimers [ TimerNumber ].HOLD.Value;
			}
			
			// otherwise
			// invalid TIMER Register
			cout << "\nhps2x64 ALERT: Unknown TIMER READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
			
			break;
			
		default:
#ifdef INLINE_DEBUG_READ
			debug << "; Invalid";
#endif
		
			// invalid TIMER Register
			cout << "\nhps2x64 ALERT: Unknown TIMER READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
			break;
	}

	return 0;
}


void Timers::Write ( u32 Address, u64 Data, u64 Mask )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nTimers::Write CycleCount=" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data;
#endif

	u32 TimerNumber;

	// *** testing *** check if mask is a word write
	//if ( Mask != 0xffffffff && Mask != 0xffff )
	//{
	//	cout << "\nhps2x64 ALERT: Timers::Write Mask=" << hex << Mask;
	//}
	
	// *** TESTING ***
	// only need 16-bits
	//Data &= 0xffff;
	
	// apply write mask here for now
	Data &= Mask;
	
	// get the timer number (fixed for PS2)
	TimerNumber = ( Address >> 11 ) & 3;
	
	switch ( ( Address >> 4 ) & 0xf )
	{
		case 0:	// WRITE: COUNT
		
			// write the new timer value //
			_TIMERS->SetValue ( TimerNumber, Data );
			
#ifdef INLINE_DEBUG_WRITE
			debug << "; T" << dec << TimerNumber << "_COUNT = " << hex << _TIMERS->TheTimers [ TimerNumber ].COUNT.Value;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
#endif

			break;
			
		case 1:	// WRITE: MODE
		
			// write new mode value
			_TIMERS->SetMode ( TimerNumber, Data );

			// clear irq on compare/overflow reached flags when a 1 is written to them (bits 10 and 11)
			// but need to do this before getting the next event
			//_TIMERS->TheTimers [ TimerNumber ].MODE.Value &= ~( Data & 0xc00 );
			
#ifdef INLINE_DEBUG_WRITE
			debug << "; T" << dec << TimerNumber << "_MODE = " << hex << _TIMERS->TheTimers [ TimerNumber ].MODE.Value;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
#endif

			break;
			
		case 2:	// WRITE: COMP
		
			_TIMERS->SetComp ( TimerNumber, Data );
			
#ifdef INLINE_DEBUG_WRITE
			debug << "; T" << dec << TimerNumber << "_COMP = " << hex << _TIMERS->TheTimers [ TimerNumber ].COMP.Value;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
#endif

			break;
			
			
		case 3:	// WRITE: HOLD
		
			// only for timers 0 and 1
			if ( TimerNumber <= 1 )
			{
				_TIMERS->TheTimers [ TimerNumber ].HOLD.Value = Data;
			}
			else
			{
				// otherwise
				// invalid TIMER Register
				cout << "\nhps2x64 ALERT: Unknown TIMER WRITE @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data << "\n";
			}
			
#ifdef INLINE_DEBUG_WRITE
			debug << "; T" << dec << TimerNumber << "_COMP = " << hex << _TIMERS->TheTimers [ TimerNumber ].HOLD.Value;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
#endif

			break;
			
		default:
#ifdef INLINE_DEBUG_WRITE
			debug << "; Invalid";
#endif
		
			// invalid TIMER Register
			cout << "\nhps2x64 ALERT: Unknown TIMER WRITE @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data << "\n";
			break;

	}
}





// update what cycle the next event is at for this device
void Timer::SetNextEventCh_Cycle ( u64 Cycle )
{
	NextEvent_Cycle = Cycle;
	
	Timers::_TIMERS->Update_NextEventCycle ();
}

void Timer::SetNextEventCh ( u64 Cycle )
{
	NextEvent_Cycle = Cycle + *_DebugCycleCount;
	
	Timers::_TIMERS->Update_NextEventCycle ();
}


void Timers::Update_NextEventCycle ()
{
	// first need to initialize the next event cycle to an actual cycle number that currently exists
	//NextEvent_Cycle = TheTimers [ 0 ].NextEvent_Cycle;
	NextEvent_Cycle = -1LL;
	
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		//if ( TheTimers [ i ].NextEvent_Cycle > *_DebugCycleCount && ( TheTimers [ i ].NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) )
		if ( TheTimers [ i ].NextEvent_Cycle < NextEvent_Cycle )
		{
			// the next event is the next event for device
			NextEvent_Cycle = TheTimers [ i ].NextEvent_Cycle;
		}
	}

	//if ( NextEvent_Cycle > *_DebugCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_DebugCycleCount ) )
	if ( NextEvent_Cycle < *_NextSystemEvent )
	{
		*_NextSystemEvent = NextEvent_Cycle;
		*_NextEventIdx = NextEvent_Idx;
	}
}





u64 Timer::Get_NextIntTick ( u64 lStartValue )
{
	const u64 OVERFLOW_MASK = 0xffffULL;
	
	u64 lIntTick_Target, lIntTick_Overflow, lIntTick;
	u64 lCompareValue;	// , lOverflowValue;
	u64 lWrapValue;
	

#ifdef INLINE_DEBUG_NEXTINTTICK
	Timers::debug << "\r\nTimer::Get_NextIntTick";
#endif

	// get compare value //
	lCompareValue = (u32) COMP.Compare;
	
#ifdef INLINE_DEBUG_NEXTINTTICK
	Timers::debug << " lStartValue=" << hex << lStartValue;
	Timers::debug << " lCompareValue=" << hex << lCompareValue;
#endif
	
	// get the compare tick //
	
	// check if counting to target //
	if ( MODE.CompareEnable && ( lStartValue <= lCompareValue ) )
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
	
#ifdef INLINE_DEBUG_NEXTINTTICK
	Timers::debug << " lWrapValue=" << hex << lWrapValue;
#endif

	// get the next int tick //
	
	// set int tick to max
	//lIntTick_Target = 0xffffffff;
	//lIntTick_Target = 0x100000000ULL;
	lIntTick_Target = -1ull;
	
	// check if irq is on target
	if ( MODE.IrqOnTarget && !MODE.IrqOnTarget_Generated )
	{
		// note: can't interrupt on compare value if it is zero (or can you??)
#ifdef DISABLE_COMPARE_INT_ON_ZERO
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

#ifdef INLINE_DEBUG_NEXTINTTICK
	Timers::debug << " lIntTick_Target=" << hex << lIntTick_Target;
#endif
	
	// set int tick to max
	//lIntTick_Overflow = 0xffffffff;
	//lIntTick_Overflow = 0x100000000ULL;
	lIntTick_Overflow = -1ull;
	
	// check if irq is on overflow
	if ( MODE.IrqOnOverflow && !MODE.IrqOnOverflow_Generated )
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
	
#ifdef INLINE_DEBUG_NEXTINTTICK
	Timers::debug << " lIntTick_Overflow=" << hex << lIntTick_Overflow;
#endif

	lIntTick = -1ull;

	// check which int tick comes next //
	if ( ( lIntTick_Target < lIntTick_Overflow ) && ( lIntTick_Target != -1ull ) )
	{
		//NextIntType = INT_TYPE_TARGET;
		lIntTick = lIntTick_Target;
	}
	else if ( lIntTick_Overflow != -1ull )
	{
		//NextIntType = INT_TYPE_OVERFLOW;
		lIntTick = lIntTick_Overflow;
	}
	
#ifdef INLINE_DEBUG_NEXTINTTICK
	Timers::debug << " lIntTick=" << hex << lIntTick;
#endif

	return lIntTick;
}



// counts the clock tics between two cycle numbers ignoring sync modes	
u64 Timer::CountTicks ( u64 lBeginCycle, u64 lEndCycle )
{
	u64 c_ullCycleClock_Mask = ( 1 << ( MODE.ClkSelect << 2 ) ) - 1;
	u64 c_ullCycleClock_Shift = MODE.ClkSelect << 2;
	
	switch ( MODE.ClkSelect )
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
u64 Timer::Get_InterruptCycle ( u64 lBeginCycle, u64 lStartTick, u64 lTargetTick )
{
	u64 c_ullCycleClock_Mask = ( 1 << ( MODE.ClkSelect << 2 ) ) - 1;
	u64 c_ullCycleClock_Shift = MODE.ClkSelect << 2;
	
	u64 lReturnCycles;
	u64 lTemp;
	double dTemp;
	
	// the target should come after the starting point
	if ( lTargetTick <= lStartTick ) return -1LL;
	
	// check for clock source
	switch ( MODE.ClkSelect )
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


void Timer::UpdateTimer ()
{
	const u64 OVERFLOW_MASK = 0xffffULL;

	bool bBlank;
	double dOffsetCycles;
	u64 lStartCycle, lEndCycle, lCurrentCycle;
	u64 lBlankStartCycle, lBlankEndCycle;
	u64 lInitialValue, lCompareValue, lWrapValue;	// , lOverflowValue;
	
#ifdef INLINE_DEBUG_UPDATE
	Timers::debug << "\r\nTimer::UpdateTimer";
#endif
	
	//Timer* pt;
	
	// note: timer should be updated before reading value/mode. Should also be updated periodically to maintain a good speed. per scanline or per frame is probably good.

	//pt = TimerPtrs [ TIMERNUMBER ];

	// get the value you started at for timer
	lInitialValue = StartValue;

	// get the current cycle //
	lCurrentCycle = *_DebugCycleCount;


	// if timer is not enabled or otherwise frozen then do not count
	if ( !MODE.CounterEnable )
	{
#ifdef INLINE_DEBUG_UPDATE
	Timers::debug << " PAUSED";
#endif

		// timer is paused //
		StartCycle = lCurrentCycle;
		return;
	}
	
	
	// if in free run mode, or this is timer 2, then can calculate like free run
	if ( !MODE.Gate )
	{
#ifdef INLINE_DEBUG_UPDATE
	Timers::debug << " FREERUN";
#endif

		// timer is in free run //
		//Update_FreeRunTimer ();
		StartValue += CountTicks ( StartCycle, lCurrentCycle );
		
		StartCycle = lCurrentCycle;
	}
	else
	{
#ifdef INLINE_DEBUG_UPDATE
	Timers::debug << " SYNC";
#endif

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
			if ( !MODE.GateSelect )
			{
#ifdef INLINE_DEBUG_UPDATE
	Timers::debug << " HBLANKGATE";
#endif

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
#ifdef INLINE_DEBUG_UPDATE
	Timers::debug << " VBLANKGATE";
#endif

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


			if ( StartCycle < lBlankStartCycle )
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
				
				
				switch ( MODE.GateMode )
				{
					case 0:
					case 1:
					case 2:
					case 3:
						// counts outside of blanking area //
						StartValue += CountTicks ( StartCycle, lCurrentCycle );
						break;
				}
				
				
				// now we are at either current cycle or start of blanking area
				StartCycle = lEndCycle;
				
				
				// if the blanking area was just hit, then process //
				// when just reached blank, so for mode 0 do nothing, for mode 1 reset to zero, mode 2 reset to zero, mode 3 switch to free run
				if ( lEndCycle == lBlankStartCycle )
				{
					// gate edge (rising) //
					switch ( MODE.GateMode )
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
							StartValue = 0;
							break;
							
					} // end switch ( MODE.SyncMode )
				
				} // end if ( lEndCycle == lBlankCycle )
				
				
			} // end if ( !bBlank )

			
			// check if in blank //
			if ( StartCycle >= lBlankStartCycle )
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
				
				switch ( MODE.GateMode )
				{
					case 0:
						// paused during blanking area //
						break;
						
					case 1:
					case 2:
					case 3:
						// counts during blanking area //
						StartValue += CountTicks ( StartCycle, lCurrentCycle );
						break;
				}
				
			} // end if ( StartCycle >= lBlankCycle )
			
			
			// update cycle we are at //
			//lStartCycle = lEndCycle;
			
		//} // end while ( lStartCycle < lCurrentCycle )
		
		// set the start cycle as the current cycle //
		//StartCycle = *_DebugCycleCount;
		StartCycle = lCurrentCycle;

		// if the blanking area was just hit, then process //
		// when just reached blank, so for mode 0 do nothing, for mode 1 reset to zero, mode 2 reset to zero, mode 3 switch to free run
		if ( lCurrentCycle == lBlankEndCycle )
		{
			// gate edge (falling) //
			switch ( MODE.GateMode )
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
					StartValue = 0;
					break;
					
			} // end switch ( MODE.SyncMode )
		
		} // end if ( lEndCycle == lBlankCycle )
		
	} // end if ( ( !MODE.RunMode ) || ( TimerNumber == 2 ) || ( TimerNumber > 3 ) )
	
	

	lCompareValue = ( (u64) COMP.Compare );
	//lOverflowValue = 0xffffULL;
	

	// get the wrap value
	if ( MODE.CompareEnable && ( lInitialValue <= lCompareValue ) )
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
		//if ( lCompareValue ) lCompareValue += lWrapValue;
		// but also want a compare interrupt on zero if interrupt overflow not set
		lCompareValue += lWrapValue;
	}
	
	// before wrapping value around, set the "value reached" flags

//#ifdef SET_REACHED_ONLY_ON_INT

	// only does irq if the interrupt generated bit is cleared
	if ( MODE.IrqOnTarget )
	{
//#endif

	// if target value was reached, then set the flag that says so
	if ( lInitialValue < lCompareValue && StartValue >= lCompareValue )
	{

#ifdef DISABLE_COMPAREINT_AFTEROVF
		// only set target interrupt if overflow interrupt didn't happen first
		if ( ( lCompareValue & OVERFLOW_MASK ) || ( !MODE.IrqOnOverflow ) )
#endif
		{

#ifdef INLINE_DEBUG_UPDATE_INT
	Timers::debug << " COMPAREINT";
#endif

		// *note* it's also possible that this might be only set if an interrupt occurs at target ??
		MODE.IrqOnTarget_Generated = 1;

		}
		
	}

//#ifdef SET_REACHED_ONLY_ON_INT
	}
//#endif
	
	
	if ( !MODE.CompareEnable || ( lInitialValue > lCompareValue ) )
	{
		// if overflow value was reached, then set the flag that says so
		if ( StartValue > OVERFLOW_MASK )
		{
//#ifdef SET_REACHED_ONLY_ON_INT
			if ( MODE.IrqOnOverflow )
			{
//#endif
#ifdef INLINE_DEBUG_UPDATE_INT
	Timers::debug << " OVFINT";
#endif

			// *note* it's also possible that this might be only set if an interrupt occurs at overflow ??
			MODE.IrqOnOverflow_Generated = 1;


/*
#ifndef DISABLE_COMPAREINT_AFTEROVF
				// check for additional irq on target ??
				if ( MODE.IrqOnTarget )
				{
					if ( !COMP.Compare )
					{
#ifdef INLINE_DEBUG_UPDATE_INT
	Timers::debug << " COMPAREINTAFTEROVF";
#endif

						MODE.IrqOnTarget_Generated = 1;
					}
				}
#endif
*/


//#ifdef SET_REACHED_ONLY_ON_INT
			}
//#endif

			// can only do this part when counting to target, since it resets the wrap value
			if ( MODE.CompareEnable )
			{
				// here should wrap against overflow
				// but the timer could be anywhere, so must first subtract
				//pt->StartValue &= lOverflowValue;
				//StartValue -= ( OVERFLOW_MASK + 1 );
				StartValue &= OVERFLOW_MASK;
				
				// now the wrap value is the compare value plus one
				lWrapValue = lCompareValue + 1;
			}
		}
	}
	
	
	

	if ( MODE.CompareEnable )
	{
		// otherwise lInitialValue < lWrapValue
		if ( StartValue >= lWrapValue )
		{
			// here should wrap against whatever it should be counting to
			StartValue %= lWrapValue;
		}
	}
	else
	{
		// wrap against overflow
		StartValue &= OVERFLOW_MASK;
	}
}



void Timer::Get_NextEvent ()
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
	if ( ( !MODE.IrqOnTarget && !MODE.IrqOnOverflow ) || ( !MODE.CounterEnable ) )
	{
		SetNextEventCh_Cycle ( -1LL );
		return;
	}
	

	
	
	// set the final cycle //
	//lCurrentCycle = g->llNextScanlineStart;
	
	// set start cycle //
	lStartCycle = StartCycle;

#ifdef INLINE_DEBUG_NEXTEVENT_T
	Timers::debug << " lStartCycle=" << dec << lStartCycle;
#endif


	lStartValue = StartValue & OVERFLOW_MASK;

#ifdef INLINE_DEBUG_NEXTEVENT_T
	Timers::debug << " lStartValue=" << dec << lStartValue;
#endif

	// get value where next event should happen
	lTargetValue = Get_NextIntTick ( lStartValue );
	
#ifdef INLINE_DEBUG_NEXTEVENT_T
	Timers::debug << " lTargetValue=" << dec << lTargetValue;
#endif

	// if in free run mode, or this is timer 2, then can calculate like free run
	if ( !MODE.Gate )
	{
		// timer is in free run //
	
			// get the cycle of the next interrupt for free run timer //
			//lIntCycle = Get_FreeRunNextIntCycle ( lStartValue, lStartCycle );
			lIntCycle = Get_InterruptCycle ( lStartCycle, lStartValue, lTargetValue );
			
#ifdef INLINE_DEBUG_NEXTEVENT_T
	Timers::debug << " lIntCycle=" << dec << lIntCycle;
#endif

			// set the cycle for the next event for timer
			SetNextEventCh_Cycle ( lIntCycle );
			
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
			if ( !MODE.GateSelect )
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
				switch ( MODE.GateMode )
				{
					case 0:
					case 1:
					case 2:
					case 3:
						// counts outside of blanking area //
						lIntCycle = Get_InterruptCycle ( lStartCycle, lStartValue, lTargetValue );
						
						// check if the target value has been passed before (or on transition to?) the blanking area
						if ( lIntCycle <= lBlankStartCycle )
						{
							// set an event for when interrupt occurs
							//lIntCycle = Get_InterruptCycle_Scanline ( lStartCycle, lStartValue, lTargetValue );
							SetNextEventCh_Cycle ( lIntCycle );
							
							return;
						}
						
						// update timer value
						lStartValue += CountTicks ( lStartCycle, lEndCycle );
						
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
					switch ( MODE.GateMode )
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
				
				switch ( MODE.GateMode )
				{
					case 0:
						// paused during blanking area //
						
						break;
						
					case 1:
					case 2:
					case 3:
					
						// counts during blanking area //
						
						//StartValue += CountTicks_Scanline ( lStartCycle, lEndCycle );
						
						lIntCycle = Get_InterruptCycle ( lStartCycle, lStartValue, lTargetValue );
						
						// set the cycle for the next event for timer
						SetNextEventCh_Cycle ( lIntCycle );
						
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
	SetNextEventCh_Cycle ( -1LL );
}










void Timer::SetMode( u32 Data )
{
	//u64 ThroughCycle;
	
#ifdef USE_TEMPLATES_PS2_TIMER
	// update timer using previous MODE value
	cbUpdate ( this );
#else
	UpdateTimer ();
#endif


	// from Nocash PSX Specifications
	// writing to the mode register clears timer to zero (but ONLY on PS1 timers, apparently not on PS2 timers??)
	//if ( ( Data ^ MODE.Value ) & 0x3ff )
	//if ( Data & 0x80 )
	if ( ( Data ^ MODE.Value ) & 0x80 )
	{
		COUNT.Value = 0;
		StartValue = 0;
		StartCycle = *_DebugCycleCount;
	}
	
	
	// bits 10 and 11 acknowledge interrupts
	// handled elsewhere
	//MODE.Value &= ~( Data & 0xc00 );

	// write new mode value (lower 10 bits)
	//MODE.Value = Data;
	MODE.Value = ( MODE.Value & 0xc00 ) | ( Data & 0x3ff );


	// clear irq on compare/overflow reached flags when a 1 is written to them (bits 10 and 11)
	MODE.Value &= ~( Data & 0xc00 );



	// from Nocash PSX Specifications
	// writing to mode register sets bit 10 (InterruptRequest=No=1)
	//MODE.Value |= ( 1 << 10 );
	

	
	// reset irq counter
	//IRQ_Counter = 0;
	
	
	// calibrate timer
#ifdef USE_TEMPLATES_PS2_TIMER
	CalibrateTimer_t ();
//#else
//	CalibrateTimer ();
#endif
	
#ifdef USE_TEMPLATES_PS2_TIMER
	// get next interrupt event for timer
	cbGetNextEvent ( this );
#else
	Get_NextEvent ();
#endif

}


void Timer::SetValue ( u32 Data )
{
	//u64 ThroughCycle;
	
#ifdef USE_TEMPLATES_PS2_TIMER
	// update timer using previous MODE value
	cbUpdate ( this );
#else
	UpdateTimer ();
#endif

	
	// write new timer value
	COUNT.Value = Data;

	
	StartValue = COUNT.Count;
	StartCycle = *_DebugCycleCount;
	
	
#ifdef USE_TEMPLATES_PS2_TIMER
	// get next interrupt event for timer
	cbGetNextEvent ( this );
#else
	Get_NextEvent ();
#endif

	// set timer start value
	// recalibrate timer
	//SetTimerValue ( TimerNumber, Data );
	
	// if the set value is equal or greater than compare value, alert //
	
	if ( Data > COMP.Compare )
	{
		cout << "\nhps2x64 ALERT: TIMER#" << TimerNumber << " is being manually set greater than compare value.\n";
	}
	
	//if ( Data == COMP.Compare )
	//{
	//	cout << "\nhps1x64 ALERT: TIMER#" << TimerNumber << " is being manually set equal to compare value.\n";
	//}
}


void Timer::SetComp ( u32 Data )
{
	//u64 ThroughCycle;
	
#ifdef USE_TEMPLATES_PS2_TIMER
	// update timer using previous compare value
	cbUpdate ( this );
#else
	UpdateTimer ();
#endif

	// write new compare value
	COMP.Value = Data;
	
	// *** todo *** is it needed to recalibrate timer and where interrupt should occur??
	
#ifdef USE_TEMPLATES_PS2_TIMER
	// get next interrupt event for timer
	cbGetNextEvent ( this );
#else
	Get_NextEvent ();
#endif

}


void Timers::Refresh ()
{
	ReCalibrateAll ();
}


void Timers::ReCalibrateAll ()
{
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		CalibrateTimer ( i );
	}
}


void Timers::Perform_TimerHold ()
{
	// update first two timers //
	_TIMERS->UpdateTimer ( 0 );
	_TIMERS->UpdateTimer ( 1 );
	
	// get the current value of the timer //
	//_TIMERS->TheTimers [ 0 ].COUNT.Value = _TIMERS->TheTimers [ 0 ].StartValue;
	//_TIMERS->TheTimers [ 1 ].COUNT.Value = _TIMERS->TheTimers [ 1 ].StartValue;
	
	// set the hold registers //
	_TIMERS->TheTimers [ 0 ].HOLD.Value = _TIMERS->TheTimers [ 0 ].StartValue;
	_TIMERS->TheTimers [ 1 ].HOLD.Value = _TIMERS->TheTimers [ 1 ].StartValue;
}




////////////// Debugging ///////////////////////////




void Timers::DebugWindow_Enable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	static constexpr char* DebugWindow_Caption = "PS2 Timer Debug Window";
	static constexpr int DebugWindow_X = 10;
	static constexpr int DebugWindow_Y = 10;
	static constexpr int DebugWindow_Width = 200;
	static constexpr int DebugWindow_Height = 200;
	
	static constexpr int TimerList_X = 0;
	static constexpr int TimerList_Y = 0;
	static constexpr int TimerList_Width = 150;
	static constexpr int TimerList_Height = 180;
	
	int i;
	stringstream ss;
	
	if ( !DebugWindow_Enabled )
	{
		// create the main debug window
		DebugWindow = new WindowClass::Window ();
		DebugWindow->Create ( DebugWindow_Caption, DebugWindow_X, DebugWindow_Y, DebugWindow_Width, DebugWindow_Height );
		DebugWindow->DisableCloseButton ();
		
		// create "value lists"
		Timer_ValueList = new DebugValueList<u32> ();
		Timer_ValueList->Create ( DebugWindow, TimerList_X, TimerList_Y, TimerList_Width, TimerList_Height, true, false );
		
		
		Timer_ValueList->AddVariable ( "T0_COUNT", & ( _TIMERS->TheTimers [ 0 ].COUNT.Value ) );
		Timer_ValueList->AddVariable ( "T0_COUNT2", (u32*) & ( _TIMERS->TheTimers [ 0 ].StartValue ) );
		Timer_ValueList->AddVariable ( "T0_MODE", & ( _TIMERS->TheTimers [ 0 ].MODE.Value ) );
		Timer_ValueList->AddVariable ( "T0_COMP", & ( _TIMERS->TheTimers [ 0 ].COMP.Value ) );
		Timer_ValueList->AddVariable ( "T1_COUNT", & ( _TIMERS->TheTimers [ 1 ].COUNT.Value ) );
		Timer_ValueList->AddVariable ( "T1_COUNT2", (u32*) & ( _TIMERS->TheTimers [ 1 ].StartValue ) );
		Timer_ValueList->AddVariable ( "T1_MODE", & ( _TIMERS->TheTimers [ 1 ].MODE.Value ) );
		Timer_ValueList->AddVariable ( "T1_COMP", & ( _TIMERS->TheTimers [ 1 ].COMP.Value ) );
		Timer_ValueList->AddVariable ( "T2_COUNT", & ( _TIMERS->TheTimers [ 2 ].COUNT.Value ) );
		Timer_ValueList->AddVariable ( "T2_COUNT2", (u32*) & ( _TIMERS->TheTimers [ 2 ].StartValue ) );
		Timer_ValueList->AddVariable ( "T2_MODE", & ( _TIMERS->TheTimers [ 2 ].MODE.Value ) );
		Timer_ValueList->AddVariable ( "T2_COMP", & ( _TIMERS->TheTimers [ 2 ].COMP.Value ) );
		Timer_ValueList->AddVariable ( "T3_COUNT", & ( _TIMERS->TheTimers [ 3 ].COUNT.Value ) );
		Timer_ValueList->AddVariable ( "T3_COUNT2", (u32*) & ( _TIMERS->TheTimers [ 3 ].StartValue ) );
		Timer_ValueList->AddVariable ( "T3_MODE", & ( _TIMERS->TheTimers [ 3 ].MODE.Value ) );
		Timer_ValueList->AddVariable ( "T3_COMP", & ( _TIMERS->TheTimers [ 3 ].COMP.Value ) );
		
		// mark debug as enabled now
		DebugWindow_Enabled = true;
		
		// update the value lists
		DebugWindow_Update ();
	}
	
#endif

}

void Timers::DebugWindow_Disable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;
	
	if ( DebugWindow_Enabled )
	{
		delete DebugWindow;
		delete Timer_ValueList;
	
		// disable debug window
		DebugWindow_Enabled = false;
	}
	
#endif

}

void Timers::DebugWindow_Update ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;
	
	if ( DebugWindow_Enabled )
	{
		Timer_ValueList->Update();
	}
	
#endif

}



