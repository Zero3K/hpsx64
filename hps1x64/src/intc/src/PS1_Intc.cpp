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



#include "PS1_Intc.h"


// testing
#include "PS1_Dma.h"


using namespace Playstation1;



//#define ENABLE_R3000A_IDLE




#define ENABLE_INTC_CTRL
#define ENABLE_SBUS_INT


#ifdef _DEBUG_VERSION_

// enable debugging
#define INLINE_DEBUG_ENABLE

/*
#define INLINE_DEBUG_WRITE
//#define INLINE_DEBUG_READ

#define INLINE_DEBUG_UPDATE
*/

#endif


u32* Intc::_SBus_F230;


u32* Intc::_DebugPC;
u64* Intc::_DebugCycleCount;
u64* Intc::_SystemCycleCount;

u32* Intc::_R3000A_Status_12;
u32* Intc::_R3000A_Cause_13;
u64* Intc::_ProcStatus;



Debug::Log Intc::debug;


Intc *Intc::_INTC;

bool Intc::DebugWindow_Enabled;
WindowClass::Window *Intc::DebugWindow;
DebugValueList<u32> *Intc::ValueList;


#ifdef PS2_COMPILE
u32* Intc::_ulIdle;
#endif

/*
Intc::Intc ()
{
	cout << "Running INTC constructor...\n";
}
*/

void Intc::Start ()
{
	cout << "Running INTC constructor...\n";
	
#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create ( "INTC_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering INTC Constructor";
#endif

	Reset ();
	
	_INTC = this;

#ifdef INLINE_DEBUG
	debug << "->Exiting INTC Constructor";
#endif
}


void Intc::Reset ()
{
#ifdef INLINE_DEBUG
	debug << "\r\nEntering Intc::Reset";
#endif

	// zero object
	memset ( this, 0, sizeof( Intc ) );
	

#ifdef INLINE_DEBUG
	debug << "->Exiting Intc::Reset";
#endif
}

u32 Intc::Read ( u32 Address )
{
#ifdef INLINE_DEBUG_READ
	debug << "\r\nIntc::Read; " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif

	u32 Output;

	// Read interrupt controller register value
	switch ( Address )
	{
		case I_STAT:
#ifdef INLINE_DEBUG_READ
			debug << "; I_STAT = " << hex << _INTC->I_STAT_Reg.Value;
#endif

			Output = _INTC->I_STAT_Reg.Value;
			break;
			
		case I_MASK:
#ifdef INLINE_DEBUG_READ
			debug << "; I_MASK = " << hex << _INTC->I_MASK_Reg.Value;
#endif

			Output = _INTC->I_MASK_Reg.Value;
			break;

#ifdef PS2_COMPILE

		case I_CTRL:
#ifdef INLINE_DEBUG_READ
			debug << "; I_CTRL = " << hex << _INTC->I_CTRL_Reg;
#endif

			Output = _INTC->I_CTRL_Reg;
			_INTC->I_CTRL_Reg = 0;
			
#ifdef ENABLE_INTC_CTRL
			_INTC->UpdateInts ();
#endif
			
			break;
			
#endif
			
		default:
			cout << "hps1x64 WARNING: READ from unknown INTC Register @ Cycle#" << dec << *_DebugCycleCount << " PC=" << hex << *_DebugPC << " Address=" << Address;
			break;
	}
	
	return Output;
	
}



void Intc::Write ( u32 Address, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nIntc::Write; " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
#endif

	// make sure the address is in the correct range
	//if ( Address < Regs_Start || Address > Regs_End ) return;
	
	// apply write mask here for now
	Data &= Mask;
	
	// handle writing of value to interrupt controller register
	switch ( Address )
	{
		case I_STAT:
#ifdef INLINE_DEBUG_WRITE
			debug << "; (Before) I_STAT = " << hex << _INTC->I_STAT_Reg.Value;
#endif
		
			// logical AND with value
			//I_STAT_Reg.Value &= Data;
			//I_STAT_Reg.Value = ( I_STAT_Reg.Value & Data & I_MASK_Reg.Value );	//n_irqdata = ( n_irqdata & ~mem_mask ) | ( n_irqdata & n_irqmask & data );
			_INTC->I_STAT_Reg.Value &= Data;	// ( I_STAT_Reg.Value & Data & I_MASK_Reg.Value );
			
			_INTC->UpdateInts ();
			
#ifdef INLINE_DEBUG_WRITE
			debug << "; (After) I_STAT = " << hex << _INTC->I_STAT_Reg.Value;
#endif

			break;
		
		case I_MASK:
#ifdef INLINE_DEBUG_WRITE
			debug << "; (Before) I_MASK = " << hex << _INTC->I_MASK_Reg.Value;
#endif
		
			// set value
			_INTC->I_MASK_Reg.Value = Data;	//n_irqmask = ( n_irqmask & ~mem_mask ) | data;
			
			_INTC->UpdateInts ();
			
#ifdef INLINE_DEBUG_WRITE
			debug << "; (After) I_MASK = " << hex << _INTC->I_MASK_Reg.Value;
#endif

			break;


#ifdef PS2_COMPILE

		case I_CTRL:
#ifdef INLINE_DEBUG_WRITE
			debug << "; (Before) I_CTRL = " << hex << _INTC->I_CTRL_Reg;
#endif
		
			// set value
			_INTC->I_CTRL_Reg = Data;
			
#ifdef ENABLE_INTC_CTRL
			_INTC->UpdateInts ();
#endif
			
#ifdef INLINE_DEBUG_WRITE
			debug << "; (After) I_CTRL = " << hex << _INTC->I_CTRL_Reg;
#endif

			break;
			
#endif
			
			
		default:
			cout << "hps1x64 WARNING: WRITE to unknown INTC Register @ Cycle#" << dec << *_DebugCycleCount << " PC=" << hex << *_DebugPC << " Address=" << Address;
			break;

	}
	
}

void Intc::SetInterrupt ( u32 Interrupt )
{
	I_STAT_Reg.Value |= Interrupt;
}

void Intc::ClearInterrupt ( u32 Interrupt )
{
	I_STAT_Reg.Value &= ~Interrupt;
}


// *** TODO *** need to make this more of an "UPDATE" function that runs less often
u32 Intc::Run ( u32 Interrupt )
{
	u32 SignalingInts, Temp;

	/*
	// *** testing *** vblank interrupts look like they are queued
	// check what interrupts are already signalling
	if ( I_STAT_Reg.Value & Interrupt & 1 )
	{
		QueuedInterrupts = 1;
	}
	
	// check if queued vblank interrupt is cleared
	if ( !( I_STAT_Reg.Value & 1 ) && ( QueuedInterrupts ) )
	{
		I_STAT_Reg.Value |= 1;
		QueuedInterrupts = 0;
	}
	*/

	I_STAT_Reg.Value |= Interrupt;

	if ( I_STAT_Reg.Value & I_MASK_Reg.Value ) return ( 1 << 10 );

	return 0;
}


void Intc::UpdateInts ()
{
#ifdef INLINE_DEBUG_UPDATE
	debug << "\r\nIntc::UpdateInts; " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount;
	debug << " IMASK=" << hex << _INTC->I_MASK_Reg.Value;
	debug << " ISTAT=" << hex << _INTC->I_STAT_Reg.Value;
#endif

	// need to get all the interrupt statuses
	
	// master -> stat first
	//_INTC->I_STAT_Reg.Value |= _INTC->uiIntcMaster;


	// check if dma interrupt is cleared
	/*
	if ( !( _INTC->I_STAT_Reg.Value & 0x8 ) )
	{
		// check if dma is interrupting
		if ( Playstation1::Dma::_DMA->DMARegs0.ICR.Value & 0x80000000 )
		{
#ifdef INLINE_DEBUG_UPDATE
	debug << "\r\n***PS1 INTC DMA BIT CLEAR WITH DMA INTERRUPTING***";
	debug << " ICR=" << hex << Playstation1::Dma::_DMA->DMARegs0.ICR.Value;
	//debug << " ISTAT=" << hex << _INTC->I_STAT_Reg.Value;
	//debug << " IMASK=" << hex << _INTC->I_MASK_Reg.Value;
#endif

			// shouldn't happen??
			cout << "\n*************************************************************************";
			cout << "\nhps1x64: ALERT: INTC PS1 DMA bit is clear while PS1 DMA is interrupting.\n";
			cout << "*************************************************************************\n";

			// put the bit back in
			_INTC->I_STAT_Reg.Value |= 0x8;
		}
	}
	*/


	// if interrupts are still set, then set them back when cleared
	if ( ( _INTC->I_STAT_Reg.Value & _INTC->I_MASK_Reg.Value )
#ifdef PS2_COMPILE
#ifdef ENABLE_INTC_CTRL
		&& ( _INTC->I_CTRL_Reg & 1 )
#endif
#endif
	)
	{
#ifdef INLINE_DEBUG_WRITE
	debug << " INT-SET";
#endif

		*_R3000A_Cause_13 |= ( 1 << 10 );
	}
	else
	{
#ifdef INLINE_DEBUG_WRITE
	debug << " INT-CLEAR";
#endif

		*_R3000A_Cause_13 &= ~( 1 << 10 );
	}
	
	if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) )
	{
#ifdef PS2_COMPILE
#ifdef ENABLE_R3000A_IDLE
		// cpu not idle if there is an interrupt
		*_ulIdle = 0;
#endif
#endif

#ifdef INLINE_DEBUG_WRITE
	debug << " CPU-INT-READY";
#endif

		*_ProcStatus |= ( 1 << 20 );
	}
	/*
	else
	{
#ifdef INLINE_DEBUG_WRITE
	debug << " CPU-INT-NOTREADY";
#endif

		*_ProcStatus &= ~( 1 << 20 );
	}
	*/



	
#ifdef PS2_COMPILE
#ifdef ENABLE_SBUS_INT
	// this should probably only happen on transition from 0->1
	// or it is supposed to happen on the falling edge, from 1->0 ?? or maybe the signal goes directly and it is the 0->1 transition ?
	u32 ulNewSbus = _INTC->I_STAT_Reg.Value & *_SBus_F230;
	//if ( ( _INTC->I_STAT_Reg.Value & _INTC->I_MASK_Reg.Value )
	//if (_INTC->ulPrevSbus && !ulNewSbus)
	if ( ulNewSbus && !_INTC->ulPrevSbus )
	{
		// ??
		Playstation2::SIF::SetInterrupt_EE_SIF ();
	}
	_INTC->ulPrevSbus = ulNewSbus;
#endif
#endif

}



void Intc::DebugWindow_Enable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	static constexpr char* DebugWindow_Caption = "PS1 INTC Debug Window";
	static constexpr int DebugWindow_X = 10;
	static constexpr int DebugWindow_Y = 10;
	static constexpr int DebugWindow_Width = 200;
	static constexpr int DebugWindow_Height = 200;
	
	static constexpr int List_X = 0;
	static constexpr int List_Y = 0;
	static constexpr int List_Width = 150;
	static constexpr int List_Height = 180;
	
	int i;
	stringstream ss;
	
	if ( !DebugWindow_Enabled )
	{
		// create the main debug window
		DebugWindow = new WindowClass::Window ();
		DebugWindow->Create ( DebugWindow_Caption, DebugWindow_X, DebugWindow_Y, DebugWindow_Width, DebugWindow_Height );
		DebugWindow->DisableCloseButton ();
		
		// create "value lists"
		ValueList = new DebugValueList<u32> ();
		ValueList->Create ( DebugWindow, List_X, List_Y, List_Width, List_Height, true, false );
		
		ValueList->AddVariable ( "IMASK", & _INTC->I_MASK_Reg.Value );
		ValueList->AddVariable ( "ISTAT", & _INTC->I_STAT_Reg.Value );
		
#ifdef PS2_COMPILE
		ValueList->AddVariable ( "ICTRL", & _INTC->I_CTRL_Reg );
#endif

		// mark debug as enabled now
		DebugWindow_Enabled = true;
		
		// update the value lists
		DebugWindow_Update ();
	}
	
#endif

}

void Intc::DebugWindow_Disable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;
	
	if ( DebugWindow_Enabled )
	{
		delete DebugWindow;
		delete ValueList;
	
		// disable debug window
		DebugWindow_Enabled = false;
	}
	
#endif

}

void Intc::DebugWindow_Update ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;
	
	if ( DebugWindow_Enabled )
	{
		ValueList->Update();
	}
	
#endif

}




