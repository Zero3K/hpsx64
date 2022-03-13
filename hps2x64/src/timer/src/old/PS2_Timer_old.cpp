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


#include "types.h"

#include "PS2_Timer.h"
#include <math.h>

#ifdef _DEBUG_VERSION_

// enable debugging
#define INLINE_DEBUG_ENABLE

//#define INLINE_DEBUG_READ
//#define INLINE_DEBUG_WRITE
//#define INLINE_DEBUG_CALIBRATE
//#define INLINE_DEBUG_EVENT
//#define INLINE_DEBUG_RUN

//#define INLINE_DEBUG_RUN_VBLANK
//#define INLINE_DEBUG

#endif


using namespace Playstation1;

#define CONVERT_TO_FIXED1PT63( x )	( ( (u64) ( ( x ) * ( ( (u64) 1 ) << 63 ) ) ) + ( 1 << 8 ) )

//const char* TimerNameList [ 4 ] = { "Timer0_Log.txt", "Timer1_Log.txt", "Timer2_Log.txt", "Timer3_Log.txt" };


u32* Timers::_DebugPC;
u64* Timers::_DebugCycleCount;

u64* Timers::_NextSystemEvent;

u32* Timers::_Intc_Stat;
u32* Timers::_Intc_Mask;
//u32* Timers::_Intc_Master;
u32* Timers::_R3000A_Status_12;
u32* Timers::_R3000A_Cause_13;
u64* Timers::_ProcStatus;


int Timer::Count = 0;
Debug::Log Timers::debug;

Timers *Timers::_TIMERS;


Playstation1::GPU *Timers::g;


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
	Number = Count++;
	//t = ts;
	Reset ();
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

void Timers::ConnectDevices ( Playstation1::GPU* _g )
{
	g = _g;
}


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
	debug.Create ( "Timers_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering Timers::Start";
#endif


	Reset ();
	
	_TIMERS = this;
	
	// calibrate the timers
	CalibrateTimer ( 0 );
	CalibrateTimer ( 1 );
	CalibrateTimer ( 2 );
	
	// set the timer values
	//SetTimerValue ( 0, 0 );
	//SetTimerValue ( 1, 0 );
	//SetTimerValue ( 2, 0 );
	SetValue ( 0, 0 );
	SetValue ( 1, 0 );
	SetValue ( 2, 0 );
	
	// get the next event for each timer (should be none here)
	//GetNextEvent ( 0 );
	//GetNextEvent ( 1 );
	//GetNextEvent ( 2 );
	
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

	if ( NextEvent_Cycle != *_DebugCycleCount ) return;
	
	// event triggered //
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nTimers::Run";
#endif
	
	for ( TimerNumber = 0; TimerNumber < c_iNumberOfChannels; TimerNumber++ )
	{

		// check if it is a timer event
		if ( TheTimers [ TimerNumber ].NextEvent_Cycle == NextEvent_Cycle )
		{
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nInterrupt; Timer#" << TimerNumber << " Cycle#" << dec << *_DebugCycleCount;
#endif

			// make sure value is target - ONLY COMPARE 16-BITS!!
			//if ( ( TimerValue == TimerTarget && TheTimers [ TimerNumber ].MODE.IrqOnTarget ) || ( TimerValue == 0xffff && TheTimers [ TimerNumber ].MODE.IrqOnOverflow ) )
			//{
				// *** TODO *** still need to check which interrupt is enabled.. int on target or int on overflow
				// *** TODO *** also need to check if is a one-shot or repeat interrupt
				
				// check if this is a repeated interrupt or single interrupt on first signal
				if ( TheTimers [ TimerNumber ].MODE.IrqMode_Repeat || ( !TheTimers [ TimerNumber ].MODE.IrqMode_Repeat && !TheTimers [ TimerNumber ].IRQ_Counter ) )
				{
					// check if timer is in pulse mode or toggle mode
					if ( TheTimers [ TimerNumber ].MODE.IrqMode_Toggle )
					{
						// timer is in toggle mode //
						
#ifdef INLINE_DEBUG_RUN
	debug << "; TOGGLE";
#endif

						// toggle irq
						TheTimers [ TimerNumber ].MODE.IrqRequest ^= 1;
					}
					else
					{
						// timer is in pulse mode //
						
#ifdef INLINE_DEBUG_RUN
	debug << "; PULSE";
#endif

						// clear irq temporarily
						TheTimers [ TimerNumber ].MODE.IrqRequest = 0;
					}
					
					// check if irq is requested (IrqRequest==0)
					if ( !TheTimers [ TimerNumber ].MODE.IrqRequest )
					{
						// IRQ requested //
				
#ifdef INLINE_DEBUG_RUN
	debug << "; INT Timer#" << TimerNumber;
#endif

						// generate a timer0 interrupt
						SetInterrupt ( TimerNumber );
						
						// update count of interrupts for timer
						TheTimers [ TimerNumber ].IRQ_Counter++;
					}
					
					// if timer is in pulse mode, then set irq request back for now
					if ( !TheTimers [ TimerNumber ].MODE.IrqMode_Toggle )
					{
						// timer is in pulse mode //
						
						TheTimers [ TimerNumber ].MODE.IrqRequest = 1;
					}
				}
			//}
/*
#ifdef INLINE_DEBUG_RUN
			else
			{
				// there is an event, but there is not interrupt
				debug << " NoInterrupt TimerValue=" << dec << ( TheTimers [ TimerNumber ].COUNT.Count & 0xffff ) << " TimerTarget=" << TimerTarget;
			}
#endif
*/

			// update the timer - need to do this to get the correct next event cycle distance from the current cycle
			UpdateTimer ( TimerNumber );
			
			// get next event for timer
			Get_NextEvent ( TimerNumber, g->NextEvent_Cycle );
			
#ifdef INLINE_DEBUG_RUN
	debug << "; NextEvent_Cycle=" << dec << TheTimers [ TimerNumber ].NextEvent_Cycle;
#endif

		}
		
	}

}




static u32 Timers::Read ( u32 Address )
{
#ifdef INLINE_DEBUG_READ
	debug << "\r\nTimers::Read CycleCount=" << dec << *_DebugCycleCount << " Address=" << hex << Address;
#endif

	u32 TimerNumber, Output;
	
	// get the timer number
	TimerNumber = ( Address >> 4 ) & 0x3;
	
	switch ( Address & 0xf )
	{
		case 0:	// READ: COUNT
		
			// update timer //
			_TIMERS->UpdateTimer ( TimerNumber );
			
			// get the current value of the timer //
			//_TIMERS->TheTimers [ TimerNumber ].COUNT.Value = _TIMERS->GetTimerValue ( TimerNumber );
			_TIMERS->TheTimers [ TimerNumber ].COUNT.Value = _TIMERS->TheTimers [ TimerNumber ].StartValue;
			
#ifdef INLINE_DEBUG_READ
			debug << "; T" << dec << TimerNumber << "_COUNT = " << hex << _TIMERS->TheTimers [ TimerNumber ].COUNT.Value;
			debug << dec << " TicksPerCycle=" << _TIMERS->TheTimers [ TimerNumber ].dTicksPerCycle;
			debug << dec << " CyclePerTick=" << _TIMERS->TheTimers [ TimerNumber ].dCyclesPerTick;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
			debug << dec << " OffsetCycles=" << _TIMERS->TheTimers [ TimerNumber ].dOffsetCycles;
#endif

			return _TIMERS->TheTimers [ TimerNumber ].COUNT.Value;
			break;
			
		case 4:	// READ: MODE
#ifdef INLINE_DEBUG_READ
			debug << "; T" << dec << TimerNumber << "_MODE = " << hex << _TIMERS->TheTimers [ TimerNumber ].MODE.Value;
			debug << dec << " TicksPerCycle=" << _TIMERS->TheTimers [ TimerNumber ].dTicksPerCycle;
			debug << dec << " CyclePerTick=" << _TIMERS->TheTimers [ TimerNumber ].dCyclesPerTick;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
			debug << dec << " OffsetCycles=" << _TIMERS->TheTimers [ TimerNumber ].dOffsetCycles;
#endif

			// update timer before reading mode, to update previous flags (reached target/overflow flags, etc.) for reading
			_TIMERS->UpdateTimer ( TimerNumber );
		
			// get the value of MODE register
			//return _TIMERS->TheTimers [ TimerNumber ].MODE.Value;
			Output = _TIMERS->TheTimers [ TimerNumber ].MODE.Value;
			
			// from Nocash PSX Specifications
			// bits 11 and 12 get reset after reading MODE register
			_TIMERS->TheTimers [ TimerNumber ].MODE.Value &= ~( ( 1 << 11 ) | ( 1 << 12 ) );
			
			return Output;
			
			break;
			
		case 8:	// READ: COMP
#ifdef INLINE_DEBUG_READ
			debug << "; T" << dec << TimerNumber << "_COMP = " << hex << _TIMERS->TheTimers [ TimerNumber ].COMP.Value;
			debug << dec << " TicksPerCycle=" << _TIMERS->TheTimers [ TimerNumber ].dTicksPerCycle;
			debug << dec << " CyclePerTick=" << _TIMERS->TheTimers [ TimerNumber ].dCyclesPerTick;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
			debug << dec << " OffsetCycles=" << _TIMERS->TheTimers [ TimerNumber ].dOffsetCycles;
#endif
		
			// get the value of COMP register
			return _TIMERS->TheTimers [ TimerNumber ].COMP.Value;
			break;
			
		default:
#ifdef INLINE_DEBUG_READ
			debug << "; Invalid";
#endif
		
			// invalid TIMER Register
			cout << "\nhps1x64 ALERT: Unknown TIMER READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
			break;
	}
}


static void Timers::Write ( u32 Address, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nTimers::Write CycleCount=" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data;
#endif

	u32 TimerNumber;

	// *** testing *** check if mask is a word write
	if ( Mask != 0xffffffff && Mask != 0xffff )
	{
		cout << "\nhps1x64 ALERT: Timers::Write Mask=" << hex << Mask;
	}
	
	// *** TESTING ***
	// only need 16-bits
	Data &= 0xffff;
	
	// get the timer number
	TimerNumber = ( Address >> 4 ) & 0x3;
	
	switch ( Address & 0xf )
	{
		case 0:	// WRITE: COUNT
		
			/*
			// write new timer value
			_TIMERS->TheTimers [ TimerNumber ].COUNT.Value = Data;
			
			// set timer start value
			// recalibrate timer
			_TIMERS->SetTimerValue ( TimerNumber, Data );
			*/
			
			// write the new timer value //
			_TIMERS->SetValue ( TimerNumber, Data );
			
#ifdef INLINE_DEBUG_WRITE
			debug << "; T" << dec << TimerNumber << "_COUNT = " << hex << _TIMERS->TheTimers [ TimerNumber ].COUNT.Value;
			debug << dec << " TicksPerCycle=" << _TIMERS->TheTimers [ TimerNumber ].dTicksPerCycle;
			debug << dec << " CyclePerTick=" << _TIMERS->TheTimers [ TimerNumber ].dCyclesPerTick;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
			debug << dec << " OffsetCycles=" << _TIMERS->TheTimers [ TimerNumber ].dOffsetCycles;
#endif

			break;
			
		case 4:	// WRITE: MODE
		
			/*
			// update timer using previous MODE value
			_TIMERS->UpdateTimer ( TimerNumber );

			// write new mode value
			_TIMERS->TheTimers [ TimerNumber ].MODE.Value = Data;
			
			// from Nocash PSX Specifications
			// writing to mode register sets bit 10 (InterruptRequest=No=1)
			_TIMERS->TheTimers [ TimerNumber ].MODE.Value |= ( 1 << 10 );
			
			// calibrate timer
			_TIMERS->CalibrateTimer ( TimerNumber );

			// from Nocash PSX Specifications
			// writing to the mode register clears timer to zero
			_TIMERS->TheTimers [ TimerNumber ].COUNT.Value = 0;
			_TIMERS->SetTimerValue ( TimerNumber, 0 );
			
			// reset irq counter
			_TIMERS->TheTimers [ TimerNumber ].IRQ_Counter = 0;
			*/
			
			// write new mode value
			_TIMERS->SetMode ( TimerNumber, Data );

			
#ifdef INLINE_DEBUG_WRITE
			debug << "; T" << dec << TimerNumber << "_MODE = " << hex << _TIMERS->TheTimers [ TimerNumber ].MODE.Value;
			debug << dec << " TicksPerCycle=" << _TIMERS->TheTimers [ TimerNumber ].dTicksPerCycle;
			debug << dec << " CyclePerTick=" << _TIMERS->TheTimers [ TimerNumber ].dCyclesPerTick;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
			debug << dec << " OffsetCycles=" << _TIMERS->TheTimers [ TimerNumber ].dOffsetCycles;
#endif

			break;
			
		case 8:	// WRITE: COMP
		
			/*
			// update timer using previous compare value
			_TIMERS->UpdateTimer ( TimerNumber );

			// write new compare value
			_TIMERS->TheTimers [ TimerNumber ].COMP.Value = Data;
			
			// *** todo *** is it needed to recalibrate timer and where interrupt should occur??
			
			// need to get the next event as it might have changed since compare value was changed
			_TIMERS->GetNextEvent ( TimerNumber );
			*/
			
			_TIMERS->SetComp ( TimerNumber, Data );
			
#ifdef INLINE_DEBUG_WRITE
			debug << "; T" << dec << TimerNumber << "_COMP = " << hex << _TIMERS->TheTimers [ TimerNumber ].COMP.Value;
			debug << dec << " TicksPerCycle=" << _TIMERS->TheTimers [ TimerNumber ].dTicksPerCycle;
			debug << dec << " CyclePerTick=" << _TIMERS->TheTimers [ TimerNumber ].dCyclesPerTick;
			debug << dec << " StartValue=" << _TIMERS->TheTimers [ TimerNumber ].StartValue;
			debug << dec << " StartCycle=" << _TIMERS->TheTimers [ TimerNumber ].StartCycle;
			debug << dec << " OffsetCycles=" << _TIMERS->TheTimers [ TimerNumber ].dOffsetCycles;
#endif

			break;
			
		default:
#ifdef INLINE_DEBUG_WRITE
			debug << "; Invalid";
#endif
		
			// invalid TIMER Register
			cout << "\nhps1x64 ALERT: Unknown TIMER WRITE @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data << "\n";
			break;

	}
}





// update what cycle the next event is at for this device
void Timers::SetNextEventCh_Cycle ( u64 Cycle, u32 Channel )
{
	TheTimers [ Channel ].NextEvent_Cycle = Cycle;
	
	Update_NextEventCycle ();
}

void Timers::SetNextEventCh ( u64 Cycle, u32 Channel )
{
	TheTimers [ Channel ].NextEvent_Cycle = Cycle + *_DebugCycleCount;
	
	Update_NextEventCycle ();
}

void Timers::Update_NextEventCycle ()
{
	// first need to initialize the next event cycle to an actual cycle number that currently exists
	NextEvent_Cycle = TheTimers [ 0 ].NextEvent_Cycle;
	
	for ( int i = 0; i < NumberOfChannels; i++ )
	{
		if ( TheTimers [ i ].NextEvent_Cycle > *_DebugCycleCount && ( TheTimers [ i ].NextEvent_Cycle < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) )
		{
			// the next event is the next event for device
			NextEvent_Cycle = TheTimers [ i ].NextEvent_Cycle;
		}
	}

	if ( NextEvent_Cycle > *_DebugCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_DebugCycleCount ) ) *_NextSystemEvent = NextEvent_Cycle;
}


u32 Timers::GetTimerTarget ( u32 TimerNumber )
{
	u32 TimerTarget;
	
	// get timer target
	if ( TheTimers [ TimerNumber ].MODE.Tar )
	{
		// count to value in target register //
		TimerTarget = TheTimers [ TimerNumber ].COMP.Compare;
	}
	else
	{
		// count to 0xffff //
		TimerTarget = 0xffff;
	}
	
	return TimerTarget;
}

u32 Timers::GetTimerValue ( u32 TimerNumber )
{
	u64 TimerValue, TimerTarget;
	
	/*
	if ( *_DebugCycleCount >= TheTimers [ TimerNumber ].StartCycle )
	{
	*/
	
		// timer has already been started //
		
		// get the number of ticks since timer was started
		TimerValue = TheTimers [ TimerNumber ].StartValue + ( (u64) ( ( ( (double) ( *_DebugCycleCount - TheTimers [ TimerNumber ].StartCycle ) ) + TheTimers [ TimerNumber ].dOffsetCycles ) * TheTimers [ TimerNumber ].dTicksPerCycle ) );
		
		// get the timer target value
		TimerTarget = GetTimerTarget ( TimerNumber );
		
		// perform modulo if target is not zero
		// actually can be zero since then it does modulo 1
		//if ( TimerTarget )
		//{
			// counts to target including target value
			//TimerValue %= TimerTarget;
			TimerValue %= ( TimerTarget + 1 );
		//}
	
	/*
	}
	else
	{
		TimerValue = TheTimers [ TimerNumber ].StartValue;
	}
	*/
	
	return TimerValue;
}


// get number of ticks until you reach the next tick
u32 Timers::Get_NextIntTick ( int TimerNumber, u32 lStartTick )
{
	u32 lIntTick_Target, lIntTick_Overflow, lIntTick, lCompTick;
	u32 lCompare;
	
	// get the start tick //
	//lStartTick = TheTimers [ TimerNumber ].StartValue & 0xffff;
	
	// get compare value //
	lCompare = (u32) TheTimers [ TimerNumber ].COMP.Compare;
	
	// get the compare tick //
	
	// check if counting to target //
	if ( TheTimers [ TimerNumber ].MODE.CountToTarget )
	{
		// count to target //
		lCompTick = ( (u32) TheTimers [ TimerNumber ].COMP.Compare );
	}
	else
	{
		// count to overflow //
		lCompTick = ( (u32) 0xffff );
	}
	
	// get the next int tick //
	
	// set int tick to max
	lIntTick_Target = 0xffffffff;
	
	// check if irq is on target
	if ( TheTimers [ TimerNumber ].MODE.IrqOnTarget )
	{
		// counter can always reach target //
		lIntTick_Target = ( (u32) TheTimers [ TimerNumber ].COMP.Compare );
		
		// check if start tick is on or after int tick, then add compare value to the end tick plus one
		if ( lStartTick >= lIntTick_Target )
		{
			lIntTick_Target += lCompTick + 1;
		}
	}
	
	// set int tick to max
	lIntTick_Overflow = 0xffffffff;
	
	// check if irq is on overflow
	if ( TheTimers [ TimerNumber ].MODE.IrqOnOverflow )
	{
		// make sure counter can reach overflow
		if ( lCompTick == 0xffff )
		{
			lIntTick_Overflow = lCompTick;
			
			// check if start tick is on or after end tick, then add compare value to the end tick plus one
			if ( lStartTick >= lIntTick_Overflow )
			{
				lIntTick_Overflow += lCompTick + 1;
			}
		}
	}
	
	// check which int tick comes next //
	if ( lIntTick_Target < lIntTick_Overflow ) lIntTick = lIntTick_Target; else lIntTick = lIntTick_Overflow;
	
	return lIntTick;
}


double Timers::Get_OffsetCycles ( int TimerNumber, u64 lStartCycle )
{
	double dOffsetCycles;

	// calculate the offset cycles //
	
	dOffsetCycles = 0.0L;
	if ( TheTimers [ TimerNumber ].MODE.ClockSource & 1 )
	{
		// NOT cycle counter //
		
		// get offset cycles (since gpu runs at a different frequency than CPU) //
		// check if timer 0 or 1 //
		switch ( TimerNumber )
		{
			case 0:
				// pixel counter //
				dOffsetCycles = g->GetCycles_SinceLastPixel ( (double) lStartCycle );
				break;
			
			case 1:
				// hblank counter //
				dOffsetCycles = g->GetCycles_SinceLastHBlank ( (double) lStartCycle );
				break;
				
			case 2:
				// timer 2 - cycle counter //
				break;
		}
	}
	
	if ( TheTimers [ TimerNumber ].MODE.ClockSource & 2 )
	{
		// check if timer 2 //
		if ( TimerNumber == 2 )
		{
			// one-eigth cycle timer //
			
			// get offset cycles //
			dOffsetCycles = ( (double) ( lStartCycle & 7 ) );
		}
	}
	
	return dOffsetCycles;
}


u64 Timers::Get_FreeRunNextIntCycle ( int TimerNumber, u32 lStartValue, u64 lStartCycle )
{
	double dOffsetCycles;
	u32 lIntValue;
	u64 lIntCycle;
	
	// get next int tick //
	lIntValue = Get_NextIntTick ( TimerNumber, lStartValue );
	
	// calculate the offset cycles //
	dOffsetCycles = Get_OffsetCycles ( TimerNumber, lStartCycle );
	
	// get cycle at which the next interrupt is at for timer //
	
	// set the cycle next interrupt will take place at
	lIntCycle = lStartCycle + (u64) ceil ( ( ( (double) ( lIntValue - lStartValue ) ) * TheTimers [ TimerNumber ].dCyclesPerTick ) - dOffsetCycles );
	
	return lIntCycle;
}


// gets the next event for timer.. checks up to "ThroughCycle"
// *important* don't forget to re-check for the next event whenever timer/mode/comp gets set/updated
void Timers::Get_NextEvent ( int TimerNumber, u64 ThroughCycle )
{
	bool bBlank;
	double dOffsetCycles;
	u64 lStartCycle, lEndCycle, lCurrentCycle, lBlankCycle;
	u64 lIntCycle;
	u32 lStartValue, lEndValue;
	u32 lIntValue;
	
	// set the final cycle //
	lCurrentCycle = ThroughCycle;
	
	// set start cycle //
	lStartCycle = TheTimers [ TimerNumber ].StartCycle;
	
	// get start tick //
	lStartValue = TheTimers [ TimerNumber ].StartValue & 0xffff;
	
	// if in free run mode, or this is timer 2, then can calculate like free run
	if ( ( !TheTimers [ TimerNumber ].MODE.RunMode ) || ( TimerNumber == 2 ) )
	{
		// timer is in free run //
	
		// get the cycle of the next interrupt for free run timer //
		lIntCycle = Get_FreeRunNextIntCycle ( TimerNumber, lStartValue, lStartCycle );
		
		// set the cycle for the next event for timer
		SetNextEventCh_Cycle ( lIntCycle, TimerNumber );
		
		return;
	}
	else
	{
		// Timer 0 or 1 in sync mode //
		
		// note: don't wrap around timer so you can see where it hits a particular tick //
		
		while ( lStartCycle < lCurrentCycle )
		{
		
			// check if in blanking area //
			
			// check if timer 0 or 1 //
			if ( !TimerNumber )
			{
				// pixel counter //
				bBlank = g->isHBlank ( (double) lStartCycle );
			}
			else
			{
				// hblank counter //
				bBlank = g->isVBlank ( (double) lStartCycle );
			}
		
			// check if in blank //
			if ( bBlank )
			{
				// Blank //
				
				// get next end cycle //
				
				// check if this is timer 0 or 1
				if ( !TimerNumber )
				{
					// Timer 0 //
					
					// jump to next scanline
					lEndCycle = lStartCycle + (u64) ceil ( g->GetCycles_ToNextScanlineStart ( (double) lStartCycle ) );
				}
				else
				{
					// Timer 1 //
					
					// jump to next field start
					lEndCycle = lStartCycle + (u64) ceil ( g->GetCycles_ToNextFieldStart ( (double) lStartCycle ) );
				}
				
				// check the sync mode //
				// but here, we were already in blanking area, so count for modes except zero
				// both sync modes 0 and 3 are paused during blank, so only count for modes 1 and 2
				if ( TheTimers[ TimerNumber ].MODE.SyncMode == 1 || TheTimers[ TimerNumber ].MODE.SyncMode == 2 )
				{
					// calculate the offset cycles //
					dOffsetCycles = Get_OffsetCycles ( TimerNumber, lStartCycle );
					
					// get next clock tick //
					lEndValue = lStartValue + (u64) ( ( ((double) ( lEndCycle - lStartCycle )) + dOffsetCycles ) * TheTimers [ TimerNumber ].dTicksPerCycle );
					
					// get next int tick //
					// have to get this again each loop
					lIntValue = Get_NextIntTick ( TimerNumber, lStartValue );
					
					// check if the next end tick is past the next int tick
					// must include the int value when comparing
					if ( lEndValue >= lIntValue )
					{
						// the interrupt is somewhere in here //
						lIntCycle = lStartCycle + (u64) ceil ( ( ( (double) ( lIntValue - lStartValue ) ) * TheTimers [ TimerNumber ].dCyclesPerTick ) - dOffsetCycles );
						
						// set cycle for next interrupt
						SetNextEventCh_Cycle ( lIntCycle, TimerNumber );
						
						// done
						return;
					}
					
					// set the next tick
					lStartValue = lEndValue;
				}
			}
			else
			{
				// not in Blank //
				
				// get blank cycle and offset cycles //
				
				// check if this is timer 0 or 1
				if ( !TimerNumber )
				{
					// Timer 0 //
					// get the cycle at the next hblank
					lBlankCycle = lStartCycle + (u64) ceil ( g->GetCycles_ToNextHBlank ( (double) lStartCycle ) );
				}
				else
				{
					// Timer 1 //
					// get the cycle at the next vblank
					lBlankCycle = lStartCycle + (u64) ceil ( g->GetCycles_ToNextVBlank ( (double) lStartCycle ) );
				}
				
				// set as the end cycle //
				lEndCycle = lBlankCycle;
				
				// if not mode 3 and not mode 2, then update clock //
				// sync modes 2 and 3 are paused outside of blank //
				if ( TheTimers[ TimerNumber ].MODE.SyncMode < 2 )
				{
					// calculate the offset cycles //
					dOffsetCycles = Get_OffsetCycles ( TimerNumber, lStartCycle );
					
					// get next clock tick //
					lEndValue = lStartValue + (u64) ( ( ((double) ( lEndCycle - lStartCycle )) + dOffsetCycles ) * TheTimers [ TimerNumber ].dTicksPerCycle );
					
					// get next int tick //
					// have to get this again each loop
					lIntValue = Get_NextIntTick ( TimerNumber, lStartValue );
					
					// check if the next end tick is past the next int tick
					// must include the int value when comparing
					if ( lEndValue >= lIntValue )
					{
						// the interrupt is somewhere in here //
						lIntCycle = lStartCycle + (u64) ceil ( ( ( (double) ( lIntValue - lStartValue ) ) * TheTimers [ TimerNumber ].dCyclesPerTick ) - dOffsetCycles );
						
						// set cycle of next interrupt for timer
						SetNextEventCh_Cycle ( lIntCycle, TimerNumber );
						
						// done
						return;
					}
					
					// set the next tick
					lStartValue = lEndValue;
				}
				
				// if the blanking area was just hit, then process //
				// when just reached blank, so for mode 0 do nothing, for mode 1 reset to zero, mode 2 reset to zero, mode 3 switch to free run
				if ( lEndCycle == lBlankCycle )
				{
					// just reached blanking area //
					
					switch ( TheTimers[ TimerNumber ].MODE.SyncMode )
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
							//TheTimers [ TimerNumber ].MODE.RunMode = 0;
							
							// update the start cycle for timer
							lStartCycle = lEndCycle;
							
							// calculate the rest of the cycles //
							lIntCycle = Get_FreeRunNextIntCycle ( TimerNumber, lStartValue, lStartCycle );
							
							// set cycle of next interrupt for timer
							SetNextEventCh_Cycle ( lIntCycle, TimerNumber );
							
							// done
							return;
							
							break;
					}
				
				}
			}
			
			// update cycle we are at //
			lStartCycle = lEndCycle;
		}
	}
	
	// unable to find next cycle timer should interrupt at for now
	SetNextEventCh_Cycle ( 0, TimerNumber );
}


// must update the timer before calling this or it does not work
void Timers::GetNextEvent ( u32 TimerNumber )
{
#ifdef INLINE_DEBUG_EVENT
	debug << "\r\nTimers::GetNextEvent Timer#" << dec << TimerNumber;
#endif

	u32 TimerValue, TimerTarget;
	s32 TicksToGo;
	u64 NumberOfCycles;
	u64 CyclesToGo;
	
	double dCyclesToOverflow;
	
		
	// check if timer has interrupts enabled
	if ( TheTimers [ TimerNumber ].MODE.Iq1 && TheTimers [ TimerNumber ].MODE.Iq2 )
	{
#ifdef INLINE_DEBUG_EVENT
	debug << " InterruptIsEnabled";
#endif

		// interrupt IS enabled for timer //
	
		// if the timer is stopped (Timer2) then skip
		if ( TheTimers [ TimerNumber ].dTicksPerCycle == 0.0L )
		{
#ifdef INLINE_DEBUG_EVENT
	debug << " TimerStopped";
#endif

			// Timer is stopped //
			
			// no interrupt
			//TheTimers [ TimerNumber ].NextEvent_Cycle = 0xffffffffffffffff;
			//SetNextEventCh_Cycle ( 0xffffffffffffffff, TimerNumber );
			SetNextEventCh_Cycle ( 0, TimerNumber );
		}
		else
		{
#ifdef INLINE_DEBUG_EVENT
	debug << " TimerRunning";
#endif

			// Timer is running //
			
			// get timer value
			TimerValue = GetTimerValue ( TimerNumber ) & 0xffff;
			
			// get timer target
			TimerTarget = GetTimerTarget ( TimerNumber );
			
			// check if value is at target
			if ( TimerValue == TimerTarget )
			{
				// get number of ticks between timer and target
				TicksToGo = TimerTarget + 1;
			}
			else
			{
				// get number of ticks between timer and target
				TicksToGo = TimerTarget - TimerValue;
			}
			
			// if ticks is negative, this means that timer value is past target already
			// can this happen?
			/*
			if ( TicksToGo < 0 )
			{
				// should be the target plus the distance to 0xffff
				TicksToGo = ( 0xffff - TimerValue ) + TimerTarget;
			}
			*/
			
			// find out the number of cycles before the number of ticks is reached
			// the offset keeps it all cycle accurate
			CyclesToGo = (u64) ceil( ( ((double)TicksToGo) * TheTimers [ TimerNumber ].dCyclesPerTick ) - TheTimers [ TimerNumber ].dOffsetCycles );
			
			// add that to the current cycle count to get when the next event for this timer happens
			//TheTimers [ TimerNumber ].NextEvent_Cycle = CyclesToGo + *_DebugCycleCount;
			SetNextEventCh ( CyclesToGo, TimerNumber );
			
#ifdef INLINE_DEBUG_EVENT
	debug << " NextEvent_Cycle(channel)=" << dec << TheTimers [ TimerNumber ].NextEvent_Cycle << " NextTimerEvent=" << NextEvent_Cycle;
	debug << " TimerValue=" << TimerValue << " TimerTarget=" << TimerTarget << " TicksToGo=" << TicksToGo << " CyclesToGo=" << CyclesToGo;
	debug << " dOffsetCycles=" << TheTimers [ TimerNumber ].dOffsetCycles << " dCyclesPerTick=" << TheTimers [ TimerNumber ].dCyclesPerTick << " CycleCount=" << *_DebugCycleCount;
#endif
		}
	}
	else
	{
#ifdef INLINE_DEBUG_EVENT
	debug << " InterruptNotEnabled";
#endif

		// interrupt is NOT enabled for timer //
		
		// clear next event for timer
		//TheTimers [ TimerNumber ].NextEvent_Cycle = 0xffffffffffffffff;
		//SetNextEventCh_Cycle ( 0xffffffffffffffff, TimerNumber );
		SetNextEventCh_Cycle ( 0, TimerNumber );
	}
}


/*
void Timers::UpdateTimer ( u32 TimerNumber )
{
	// check if timer is in sync mode or free run //
	if ( !TheTimers [ TimerNumber ].MODE.RunMode )
	{
		// timer is in free run //
		
		// need to add the offset cycles since hblank for example happens every scanline, but only at a certain point on the scanline
		TheTimers [ TimerNumber ].StartValue += (u64) ( ( ( (double) ( *_DebugCycleCount - TheTimers [ TimerNumber ].StartCycle ) ) + TheTimers [ TimerNumber ].dOffsetCycles ) * TheTimers [ TimerNumber ].dTicksPerCycle );
		TheTimers [ TimerNumber ].StartCycle = *_DebugCycleCount;
		
		// *** TODO *** IF TIMER GETS UPDATED, THEN YOU STILL NEED TO UPDATE OFFSET CYCLES
		// UPDATE OFFSET WHENEVER UPDATING STARTCYCLE
		ReCalibrateTimer ( TimerNumber );
	}
	else
	{
		// timer is in sync mode //
		
		// check if this is timer 2
		if ( TimerNumber == 2 )
		{
			// this is timer 2 //
			switch ( TheTimers[ TimerNumber ].MODE.SyncMode )
			{
				case 0:
				case 3:
					// stop timer //
					TheTimers [ TimerNumber ].StartCycle = *_DebugCycleCount;
					
					break;
					
				case 1:
				case 2:
					// timer is in free run mode //
					
					// need to add the offset cycles since hblank for example happens every scanline, but only at a certain point on the scanline
					TheTimers [ TimerNumber ].StartValue += (u64) ( ( ( (double) ( *_DebugCycleCount - TheTimers [ TimerNumber ].StartCycle ) ) + TheTimers [ TimerNumber ].dOffsetCycles ) * TheTimers [ TimerNumber ].dTicksPerCycle );
					TheTimers [ TimerNumber ].StartCycle = *_DebugCycleCount;
					
					// *** TODO *** IF TIMER GETS UPDATED, THEN YOU STILL NEED TO UPDATE OFFSET CYCLES
					// UPDATE OFFSET WHENEVER UPDATING STARTCYCLE
					ReCalibrateTimer ( TimerNumber );
	
					break;
			}
		}
		else
		{
			if ( TimerNumber == 0 )
			{
				// pixel clock //
				
				switch ( TheTimers[ TimerNumber ].MODE.SyncMode )
				{
					case 0:
						// do not count during blanking //
						
						cout << "\nhps1x64 Error: Timer#" << TimerNumber << " is in sync mode " << TheTimers[ TimerNumber ].MODE.SyncMode << " - Not yet implemented";
						
						break;
						
					case 1:
						// reset counter to zero at start of blanking //
						
						cout << "\nhps1x64 Error: Timer#" << TimerNumber << " is in sync mode " << TheTimers[ TimerNumber ].MODE.SyncMode << " - Not yet implemented";
						
						break;
						
					case 2:
						// reset counter to zero at blanking and do not count outside of blanking //
						
						cout << "\nhps1x64 Error: Timer#" << TimerNumber << " is in sync mode " << TheTimers[ TimerNumber ].MODE.SyncMode << " - Not yet implemented";
						
						break;
						
					case 3:
						// pause until start of next blanking, then switch to free run //
						
						cout << "\nhps1x64 Error: Timer#" << TimerNumber << " is in sync mode " << TheTimers[ TimerNumber ].MODE.SyncMode << " - Not yet implemented";
						
						break;
				}
			}
			else if ( TimerNumber == 1 )
			{
				// horizontal clock //
				
				switch ( TheTimers[ TimerNumber ].MODE.SyncMode )
				{
					case 0:
						// do not count during blanking //
						
						cout << "\nhps1x64 Error: Timer#" << TimerNumber << " is in sync mode " << TheTimers[ TimerNumber ].MODE.SyncMode << " - Not yet implemented";
						
						break;
						
					case 1:
						// reset counter to zero at start of blanking //
						
						cout << "\nhps1x64 Error: Timer#" << TimerNumber << " is in sync mode " << TheTimers[ TimerNumber ].MODE.SyncMode << " - Not yet implemented";
						
						break;
						
					case 2:
						// reset counter to zero at blanking and do not count outside of blanking //
						
						cout << "\nhps1x64 Error: Timer#" << TimerNumber << " is in sync mode " << TheTimers[ TimerNumber ].MODE.SyncMode << " - Not yet implemented";
						
						break;
						
					case 3:
						// pause until start of next blanking, then switch to free run //
						
						cout << "\nhps1x64 Error: Timer#" << TimerNumber << " is in sync mode " << TheTimers[ TimerNumber ].MODE.SyncMode << " - Not yet implemented";
						
						break;
				}
			}
		}
	}
	
}
*/

/*
// calculation should include the start cycle but not the end cycle?
u64 Timers::Count_Ticks ( double StartCycle, double EndCycle, double CyclesSinceLastTick, double TicksPerCycle )
{
	double NumberOfCycles;
	
	// calculate the number of cycles with offset
	NumberOfCycles = EndCycle - StartCycle + CyclesSinceLastTick;
	
	// get the number of ticks
	return NumberOfCycles * TicksPerCycle;
}
*/

void Timers::UpdateTimer_Wrap ( int TimerNumber )
{
	// check if reached/exceeded target //
	
	// check if counting to target //
	if ( TheTimers [ TimerNumber ].MODE.CountToTarget )
	{
		if ( TheTimers [ TimerNumber ].StartValue >= ( (u32) TheTimers [ TimerNumber ].COMP.Compare ) )
		{
			// target was reached
			TheTimers [ TimerNumber ].MODE.TargetReached = 1;
		}
		
		// this one is for strictly greater than
		if ( TheTimers [ TimerNumber ].StartValue > ( (u32) TheTimers [ TimerNumber ].COMP.Compare ) )
		{
			// only count to target, including target //
			TheTimers [ TimerNumber ].StartValue %= ( ( (u32) TheTimers [ TimerNumber ].COMP.Compare ) + 1 );
		}
	}
	
	// check if overflow was reached //
	if ( TheTimers [ TimerNumber ].StartValue >= 0xffff )
	{
		TheTimers [ TimerNumber ].MODE.OverflowReached = 1;
		
		// this one is for strictly greater than
		if ( TheTimers [ TimerNumber ].StartValue > 0xffff )
		{
			// only count to overflow, including overflow //
			TheTimers [ TimerNumber ].StartValue %= ( 0xffff + 1 );
		}
	}
}


void Timers::Update_FreeRunTimer ( int TimerNumber )
{
	double dOffsetCycles;
	u64 lStartCycle, lEndCycle, lCurrentCycle, lBlankCycle;
	
	// timer is in free run //
	
	// get start cycle //
	lStartCycle = TheTimers [ TimerNumber ].StartCycle;
	
	// get end cycle //
	lEndCycle = *_DebugCycleCount;
	
	// calculate the offset cycles //
	dOffsetCycles = Get_OffsetCycles ( TimerNumber, lStartCycle );
	
	// update ticks //
	
	// need to add the offset cycles since hblank for example happens every scanline, but only at a certain point on the scanline
	TheTimers [ TimerNumber ].StartValue += (u64) ( ( ( (double) ( lEndCycle - lStartCycle ) ) + dOffsetCycles ) * TheTimers [ TimerNumber ].dTicksPerCycle );
	
	TheTimers [ TimerNumber ].StartCycle = lEndCycle;
}


// the only way to properly simulate a PS1 timer it appears at this point is to update it per scanline... hblank is probably best
// update per scanline
void Timers::UpdateTimer ( u32 TimerNumber )
{
	bool bBlank;
	double dOffsetCycles;
	u64 lStartCycle, lEndCycle, lCurrentCycle, lBlankCycle;
	
	// note: timer should be updated before reading value/mode. Should also be updated periodically to maintain a good speed. per scanline or per frame is probably good.
	
	// if in free run mode, or this is timer 2, then can calculate like free run
	if ( ( !TheTimers [ TimerNumber ].MODE.RunMode ) || ( TimerNumber == 2 ) )
	{
		// timer is in free run //
		Update_FreeRunTimer ( TimerNumber );
	}
	else
	{
		// Timer 0 or 1 in sync mode //
		
		// get the current cycle //
		lCurrentCycle = *_DebugCycleCount;
		
		// get the start cycle //
		lStartCycle = TheTimers [ TimerNumber ].StartCycle;
		
		while ( lStartCycle < lCurrentCycle )
		{
		
			// check if in blanking area //
			
			// check if timer 0 or 1 //
			if ( !TimerNumber )
			{
				// pixel counter //
				bBlank = g->isHBlank ( (double) lStartCycle );
			}
			else
			{
				// hblank counter //
				bBlank = g->isVBlank ( (double) lStartCycle );
			}
		
			// check if in blank //
			if ( bBlank )
			{
				// Blank //
				
				// get next end cycle //
				
				// check if this is timer 0 or 1
				if ( !TimerNumber )
				{
					// Timer 0 //
					
					// jump to next scanline
					lEndCycle = lStartCycle + (u64) ceil ( g->GetCycles_ToNextScanlineStart ( (double) lStartCycle ) );
				}
				else
				{
					// Timer 1 //
					
					// jump to next field start
					lEndCycle = lStartCycle + (u64) ceil ( g->GetCycles_ToNextFieldStart ( (double) lStartCycle ) );
				}
				
				
				// check if passed the current cycle
				if ( lEndCycle > lCurrentCycle )
				{
					// do not pass the current cycle //
					lEndCycle = lCurrentCycle;
				}
				
				// check the sync mode //
				// but here, we were already in blanking area, so count for modes except zero
				// both sync modes 0 and 3 are paused during hblank, so only count for modes 1 and 2
				if ( TheTimers[ TimerNumber ].MODE.SyncMode == 1 || TheTimers[ TimerNumber ].MODE.SyncMode == 2 )
				{
					// calculate the offset cycles //
					dOffsetCycles = Get_OffsetCycles ( TimerNumber, lStartCycle );
					
					// update clock ticks //
					TheTimers [ TimerNumber ].StartValue += (u64) ( ( ((double) ( lEndCycle - lStartCycle )) + dOffsetCycles ) * TheTimers [ TimerNumber ].dTicksPerCycle );
				}
			}
			else
			{
				// not in Blank //
				
				// get blank cycle and offset cycles //
				
				// check if this is timer 0 or 1
				if ( !TimerNumber )
				{
					// Timer 0 //
					// get the cycle at the next hblank
					lBlankCycle = lStartCycle + (u64) ceil ( g->GetCycles_ToNextHBlank ( (double) lStartCycle ) );
				}
				else
				{
					// Timer 1 //
					// get the cycle at the next vblank
					lBlankCycle = lStartCycle + (u64) ceil ( g->GetCycles_ToNextVBlank ( (double) lStartCycle ) );
				}
				
				// set as the end cycle //
				lEndCycle = lBlankCycle;
				
				// check if passed the current cycle
				if ( lEndCycle > lCurrentCycle )
				{
					// do not pass the current cycle //
					lEndCycle = lCurrentCycle;
				}
				
				// if not mode 3 and not mode 2, then update clock //
				// sync modes 2 and 3 are paused outside of blank //
				if ( TheTimers[ TimerNumber ].MODE.SyncMode < 2 )
				{
					// get the offset cycles //
					dOffsetCycles = Get_OffsetCycles ( TimerNumber, lStartCycle );
					
					// update clock ticks //
					TheTimers [ TimerNumber ].StartValue += (u64) ( ( ((double) ( lEndCycle - lStartCycle )) + dOffsetCycles ) * TheTimers [ TimerNumber ].dTicksPerCycle );
				}
				
				// if the blanking area was just hit, then process //
				// when just reached blank, so for mode 0 do nothing, for mode 1 reset to zero, mode 2 reset to zero, mode 3 switch to free run
				if ( lEndCycle == lBlankCycle )
				{
					switch ( TheTimers[ TimerNumber ].MODE.SyncMode )
					{
						case 0:
							// pause counter during blank //
							
							// just reached blank and will pause in the code, so do nothing here //
							break;
							
						case 1:
						case 2:
						
							// reset counter to zero at blank //
							TheTimers [ TimerNumber ].StartValue = 0;
							break;
							
						case 3:
						
							// switch to free run at blank //
							TheTimers [ TimerNumber ].MODE.RunMode = 0;
							
							// update the start cycle for timer
							TheTimers [ TimerNumber ].StartCycle = lEndCycle;
							
							// calculate the rest of the cycles //
							Update_FreeRunTimer ( TimerNumber );
							
							// set the end cycle //
							lEndCycle = TheTimers [ TimerNumber ].StartCycle;
							
							break;
					}
				
				}
			}
			
			// update cycle we are at //
			lStartCycle = lEndCycle;
		}
		
		// set the start cycle as the current cycle //
		TheTimers [ TimerNumber ].StartCycle = *_DebugCycleCount;
	}
	
	// wrap timer value around and update associated flags
	UpdateTimer_Wrap ( TimerNumber );
}







void Timers::SetTimerValue ( u32 TimerNumber, u32 TimerValue )
{
	// need to add the offset cycles since hblank for example happens every scanline, but only at a certain point on the scanline
	//TheTimers [ TimerNumber ].StartValue += (u64) ( ( ( (double) ( *_DebugCycleCount - TheTimers [ TimerNumber ].StartCycle ) ) + TheTimers [ TimerNumber ].dOffsetCycles ) * TheTimers [ TimerNumber ].dTicksPerCycle );
	TheTimers [ TimerNumber ].StartValue = TimerValue;
	TheTimers [ TimerNumber ].StartCycle = *_DebugCycleCount;
	
	// IF TIMER GETS UPDATED, THEN YOU STILL NEED TO UPDATE OFFSET CYCLES
	// UPDATE OFFSET WHENEVER UPDATING STARTCYCLE
	// this call will also update next event
	ReCalibrateTimer ( TimerNumber );
}


// updates ticks per cycle and offset cycles from last tick
void Timers::CalibrateTimer ( u32 TimerNumber )
{
#ifdef INLINE_DEBUG_CALIBRATE
	debug << "\r\nTimers::CalibrateTimer Timer#" << dec << TimerNumber;
#endif

	// check timer number
	switch ( TimerNumber )
	{
		case 0:
		
			// pixel clock //
			
			// check the clock source
			if ( TheTimers [ TimerNumber ].MODE.ClockSource & 1 )
			{
				// dot clock //
				
				// get cycles since the last pixel and put into offset
				// set offset
				TheTimers [ TimerNumber ].dOffsetCycles = g->GetCycles_SinceLastPixel ();
				
				// set cycles per pixel
				TheTimers [ TimerNumber ].dCyclesPerTick = g->dCyclesPerPixel;
				TheTimers [ TimerNumber ].dTicksPerCycle = 1.0L / g->dCyclesPerPixel;
			}
			else
			{
				// system clock //
				
				// get cycles since the last pixel and put into offset
				// set offset
				TheTimers [ TimerNumber ].dOffsetCycles = 0.0L;
				
				// set cycles per pixel
				TheTimers [ TimerNumber ].dCyclesPerTick = 1.0L;
				TheTimers [ TimerNumber ].dTicksPerCycle = 1.0L;
			}
			
			break;
			
		case 1:
		
			// hblank clock //
			
			// check the clock source
			if ( TheTimers [ TimerNumber ].MODE.ClockSource & 1 )
			{
				// hblank clock //
				
				// get cycles since the last hblank and put into offset
				// set offset
				TheTimers [ TimerNumber ].dOffsetCycles = g->GetCycles_SinceLastHBlank ();
				
				// set cycles per hblank
				TheTimers [ TimerNumber ].dCyclesPerTick = g->dCyclesPerScanline;
				TheTimers [ TimerNumber ].dTicksPerCycle = 1.0L / g->dCyclesPerScanline;
			}
			else
			{
				// system clock //
				
				// get cycles since the last pixel and put into offset
				// set offset
				TheTimers [ TimerNumber ].dOffsetCycles = 0.0L;
				
				// set cycles per pixel
				TheTimers [ TimerNumber ].dCyclesPerTick = 1.0L;
				TheTimers [ TimerNumber ].dTicksPerCycle = 1.0L;
			}
			
			
			break;
			
		case 2:
		
			// 1/8 system clock //
			
			// check if timer is in sync mode 0 or 3
			if ( TheTimers [ TimerNumber ].MODE.RunMode && ( TheTimers [ TimerNumber ].MODE.SyncMode == 0 || TheTimers [ TimerNumber ].MODE.SyncMode == 3 ) )
			{
				// timer 2 is stopped //
				TheTimers [ TimerNumber ].dTicksPerCycle = 0.0L;
				TheTimers [ TimerNumber ].dCyclesPerTick = 0.0L;
				TheTimers [ TimerNumber ].dOffsetCycles = 0.0L;
			}
			else
			{
				// timer 2 is in free run //
				
				// check the clock source
				if ( TheTimers [ TimerNumber ].MODE.ClockSource & 2 )
				{
					// 1/8 system clock //
					
					TheTimers [ TimerNumber ].dTicksPerCycle = 1.0L / 8.0L;
					TheTimers [ TimerNumber ].dCyclesPerTick = 8.0L;
					
					// get cycles since last tick
					TheTimers [ TimerNumber ].dOffsetCycles = (double) ( *_DebugCycleCount & 7 );
				}
				else
				{
					// system clock //
					
					// get cycles since the last pixel and put into offset
					// set offset
					TheTimers [ TimerNumber ].dOffsetCycles = 0.0L;
					
					// set cycles per pixel
					TheTimers [ TimerNumber ].dCyclesPerTick = 1.0L;
					TheTimers [ TimerNumber ].dTicksPerCycle = 1.0L;
				}
			}
			
			break;
	}

	// if calibration changed, then next event for timer might change
	GetNextEvent ( TimerNumber );
	
}


// sets just the offset cycles from last tick
// called whenever a full calibration is not needed but the startvalue/startcycle gets changed
void Timers::ReCalibrateTimer ( u32 TimerNumber )
{
	// need to update the timer before you calibrate it
	// but updating timer should be done explicitly
	//UpdateTimer ( TimerNumber );

	// check timer number
	switch ( TimerNumber )
	{
		case 0:
		
			// pixel clock //
			
			// check the clock source
			if ( TheTimers [ TimerNumber ].MODE.ClockSource & 1 )
			{
				// dot clock //
				
				// get cycles since the last pixel and put into offset
				// set offset
				TheTimers [ TimerNumber ].dOffsetCycles = g->GetCycles_SinceLastPixel ();
			}
			else
			{
				// system clock //
				
				TheTimers [ TimerNumber ].dOffsetCycles = 0.0L;
			}
			
			break;
			
		case 1:
		
			// hblank clock //
			
			// check the clock source
			if ( TheTimers [ TimerNumber ].MODE.ClockSource & 1 )
			{
				// hblank clock //
				
				// get cycles since the last hblank and put into offset
				// set offset
				TheTimers [ TimerNumber ].dOffsetCycles = g->GetCycles_SinceLastHBlank ();
			}
			else
			{
				// system clock //
				
				TheTimers [ TimerNumber ].dOffsetCycles = 0.0L;
			}
			
			
			break;
			
		case 2:
		
			// 1/8 system clock //
			
			// check if timer is in sync mode 0 or 3
			if ( TheTimers [ TimerNumber ].MODE.RunMode && ( TheTimers [ TimerNumber ].MODE.SyncMode == 0 || TheTimers [ TimerNumber ].MODE.SyncMode == 3 ) )
			{
				// timer 2 is stopped //
				TheTimers [ TimerNumber ].dOffsetCycles = 0.0L;
			}
			else
			{
				// timer 2 is in free run //
				
				// check the clock source
				if ( TheTimers [ TimerNumber ].MODE.ClockSource & 2 )
				{
					// 1/8 system clock //
					
					// get cycles since last tick
					TheTimers [ TimerNumber ].dOffsetCycles = (double) ( *_DebugCycleCount & 7 );
				}
				else
				{
					// system clock //
					
					TheTimers [ TimerNumber ].dOffsetCycles = 0.0L;
				}
			}
			
			break;
	}
	
	// make sure we have correct cycle at which next event occurs for this timer
	GetNextEvent ( TimerNumber );

}



void Timers::SetMode( u32 TimerNumber, u32 Data )
{
	//u64 ThroughCycle;
	
	// update timer using previous MODE value
	UpdateTimer ( TimerNumber );

	// write new mode value
	TheTimers [ TimerNumber ].MODE.Value = Data;
	
	// from Nocash PSX Specifications
	// writing to mode register sets bit 10 (InterruptRequest=No=1)
	TheTimers [ TimerNumber ].MODE.Value |= ( 1 << 10 );
	
	// calibrate timer
	CalibrateTimer ( TimerNumber );

	// from Nocash PSX Specifications
	// writing to the mode register clears timer to zero
	TheTimers [ TimerNumber ].COUNT.Value = 0;
	//SetTimerValue ( TimerNumber, 0 );
	TheTimers [ TimerNumber ].StartValue = 0;
	TheTimers [ TimerNumber ].StartCycle = *_DebugCycleCount;
	
	// reset irq counter
	TheTimers [ TimerNumber ].IRQ_Counter = 0;
	
	// check for timer events up until next scanline transition
	//ThroughCycle = ( *_DebugCycleCount ) + (u64) ceil ( g->GetCycles_ToNextScanlineStart ( (double) ( *_DebugCycleCount ) ) );
	
	// get next interrupt event for timer
	Get_NextEvent ( TimerNumber, g->NextEvent_Cycle );
}


void Timers::SetValue ( u32 TimerNumber, u32 Data )
{
	//u64 ThroughCycle;
	
	// write new timer value
	TheTimers [ TimerNumber ].COUNT.Value = Data;

	TheTimers [ TimerNumber ].StartValue = TheTimers [ TimerNumber ].COUNT.Value;
	TheTimers [ TimerNumber ].StartCycle = *_DebugCycleCount;
	
	// check for timer events up until next scanline transition
	//ThroughCycle = ( *_DebugCycleCount ) + (u64) ceil ( g->GetCycles_ToNextScanlineStart ( (double) ( *_DebugCycleCount ) ) );
	
	// get next interrupt event for timer
	Get_NextEvent ( TimerNumber, g->NextEvent_Cycle );

	// set timer start value
	// recalibrate timer
	//SetTimerValue ( TimerNumber, Data );
	
	// if the set value is equal or greater than compare value, alert //
	
	if ( Data > TheTimers [ TimerNumber ].COMP.Compare )
	{
		cout << "\nhps1x64 ALERT: TIMER#" << TimerNumber << " is being manually set greater than compare value.\n";
	}
	
	//if ( Data == TheTimers [ TimerNumber ].COMP.Compare )
	//{
	//	cout << "\nhps1x64 ALERT: TIMER#" << TimerNumber << " is being manually set equal to compare value.\n";
	//}
}


void Timers::SetComp ( u32 TimerNumber, u32 Data )
{
	//u64 ThroughCycle;
	
	// update timer using previous compare value
	UpdateTimer ( TimerNumber );

	// write new compare value
	TheTimers [ TimerNumber ].COMP.Value = Data;
	
	// *** todo *** is it needed to recalibrate timer and where interrupt should occur??
	
	// check for timer events up until next scanline transition
	//ThroughCycle = ( *_DebugCycleCount ) + (u64) ceil ( g->GetCycles_ToNextScanlineStart ( (double) ( *_DebugCycleCount ) ) );
	
	// get next interrupt event for timer
	Get_NextEvent ( TimerNumber, g->NextEvent_Cycle );
}


u32 GetMode ( u32 TimerNumber )
{
}




////////////// Debugging ///////////////////////////




static void Timers::DebugWindow_Enable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	static const char* DebugWindow_Caption = "PS1 Timer Debug Window";
	static const int DebugWindow_X = 10;
	static const int DebugWindow_Y = 10;
	static const int DebugWindow_Width = 200;
	static const int DebugWindow_Height = 200;
	
	static const int TimerList_X = 0;
	static const int TimerList_Y = 0;
	static const int TimerList_Width = 150;
	static const int TimerList_Height = 180;
	
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
		Timer_ValueList->AddVariable ( "T0_MODE", & ( _TIMERS->TheTimers [ 0 ].MODE.Value ) );
		Timer_ValueList->AddVariable ( "T0_COMP", & ( _TIMERS->TheTimers [ 0 ].COMP.Value ) );
		Timer_ValueList->AddVariable ( "T1_COUNT", & ( _TIMERS->TheTimers [ 1 ].COUNT.Value ) );
		Timer_ValueList->AddVariable ( "T1_MODE", & ( _TIMERS->TheTimers [ 1 ].MODE.Value ) );
		Timer_ValueList->AddVariable ( "T1_COMP", & ( _TIMERS->TheTimers [ 1 ].COMP.Value ) );
		Timer_ValueList->AddVariable ( "T2_COUNT", & ( _TIMERS->TheTimers [ 2 ].COUNT.Value ) );
		Timer_ValueList->AddVariable ( "T2_MODE", & ( _TIMERS->TheTimers [ 2 ].MODE.Value ) );
		Timer_ValueList->AddVariable ( "T2_COMP", & ( _TIMERS->TheTimers [ 2 ].COMP.Value ) );
		
		// mark debug as enabled now
		DebugWindow_Enabled = true;
		
		// update the value lists
		DebugWindow_Update ();
	}
	
#endif

}

static void Timers::DebugWindow_Disable ()
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

static void Timers::DebugWindow_Update ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;
	
	if ( DebugWindow_Enabled )
	{
		Timer_ValueList->Update();
	}
	
#endif

}



