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



#include "PS2_Intc.h"

using namespace Playstation2;


#ifdef _DEBUG_VERSION_

// enable debugging
#define INLINE_DEBUG_ENABLE

/*
#define INLINE_DEBUG_WRITE
#define INLINE_DEBUG_READ

#define INLINE_DEBUG_UPDATE
*/

#endif


u32* Intc::_DebugPC;
u64* Intc::_DebugCycleCount;

u32* Intc::_R5900_Status_12;
u32* Intc::_R5900_Cause_13;
u64* Intc::_ProcStatus;


u32* Intc::_ulIdle;



Debug::Log Intc::debug;


Intc *Intc::_INTC;

bool Intc::DebugWindow_Enabled;
WindowClass::Window *Intc::DebugWindow;
DebugValueList<u32> *Intc::ValueList;


Intc::Intc ()
{
	cout << "Running INTC constructor...\n";


}


void Intc::Start ()
{
	cout << "Running INTC constructor...\n";
	
#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create ( "PS2_INTC_Log.txt" );
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

u64 Intc::Read ( u32 Address, u64 Mask )
{
#ifdef INLINE_DEBUG_READ
	debug << "\r\nIntc::Read; " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif

	// Read interrupt controller register value
	switch ( Address )
	{
		case I_STAT:
#ifdef INLINE_DEBUG_READ
			debug << "; I_STAT = " << hex << _INTC->I_STAT_Reg.Value;
#endif

			return _INTC->I_STAT_Reg.Value;
			break;
			
		case I_MASK:
#ifdef INLINE_DEBUG_READ
			debug << "; I_MASK = " << hex << _INTC->I_MASK_Reg.Value;
#endif

			return _INTC->I_MASK_Reg.Value;
			break;

			
		default:
			cout << "hps2x64 WARNING: READ from unknown INTC Register @ Cycle#" << dec << *_DebugCycleCount << " PC=" << hex << *_DebugPC << " Address=" << Address;
			break;
	}
	
	return 0;
}



void Intc::Write ( u32 Address, u64 Data, u64 Mask )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nIntc::Write; " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
#endif

	// make sure the address is in the correct range
	//if ( Address < Regs_Start || Address > Regs_End ) return;
	
	// apply write mask here for now
	Data &= Mask;
	
	// apply data offset ??
	
	// handle writing of value to interrupt controller register
	switch ( Address )
	{
		case I_STAT:
#ifdef INLINE_DEBUG_WRITE
			debug << "; (Before) I_STAT = " << hex << _INTC->I_STAT_Reg.Value;
#endif
		
			// logical AND with value
			// this is different from PS1 here, because writing a one is used to reset/clear the flag
			_INTC->I_STAT_Reg.Value &= ~Data;
			
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
			// this is different from PS1, because here writing a one reverses (XOR) that bit
			_INTC->I_MASK_Reg.Value ^= Data;
			
			_INTC->UpdateInts ();
			
#ifdef INLINE_DEBUG_WRITE
			debug << "; (After) I_MASK = " << hex << _INTC->I_MASK_Reg.Value;
#endif

			break;


			
			
		default:
			cout << "hps2x64 WARNING: WRITE to unknown INTC Register @ Cycle#" << dec << *_DebugCycleCount << " PC=" << hex << *_DebugPC << " Address=" << Address;
			break;

	}
	
}

void Intc::SetInterrupt ( u32 Interrupt )
{
	I_STAT_Reg.Value |= Interrupt;
}

// this shouldn't be used, since intc latches interrupts so they can only be cleared by R5900
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
	debug << "\r\nIntc2::UpdateInts " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount;
	debug << " IMASK=" << hex << _INTC->I_MASK_Reg.Value;
	debug << " ISTAT=" << hex << _INTC->I_STAT_Reg.Value;
#endif

	// need to get all the interrupt statuses
	
	// set value
	//_INTC->I_STAT_Reg.Value |= I_MASTER_Reg;	//n_irqmask = ( n_irqmask & ~mem_mask ) | data;
	
	// update I_STAT
	//_INTC->I_STAT_Reg.Value &= _INTC->I_MASK_Reg.Value;
	
	// if interrupts are still set, then set them back when cleared
	if ( _INTC->I_STAT_Reg.Value & _INTC->I_MASK_Reg.Value )
	{
#ifdef INLINE_DEBUG_UPDATE
	debug << " INT-SET";
#endif

		*_R5900_Cause_13 |= ( 1 << 10 );
	}
	else
	{
#ifdef INLINE_DEBUG_UPDATE
	debug << " INT-CLEAR";
#endif

		*_R5900_Cause_13 &= ~( 1 << 10 );
	}
	
	// for ps2, need to set bit 0 to trigger interrupt on CPU
	if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0x8c00 ) && ( *_R5900_Status_12 & 1 ) && ( *_R5900_Status_12 & 0x10000 ) && !( *_R5900_Status_12 & 0x6 ) )
	{
#ifdef INLINE_DEBUG_UPDATE
	debug << " CPU-INT-READY";
#endif

		// interrupt wakes up cpu from idle
		*_ulIdle = 0;
		
		*_ProcStatus |= ( 1 << 0 );
	}
	/*
	else
	{
#ifdef INLINE_DEBUG_UPDATE
	debug << " CPU-INT-NOTREADY";
#endif

		*_ProcStatus &= ~( 1 << 0 );
	}
	*/
}



void Intc::DebugWindow_Enable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	static const char* DebugWindow_Caption = "PS2 INTC Debug Window";
	static const int DebugWindow_X = 10;
	static const int DebugWindow_Y = 10;
	static const int DebugWindow_Width = 200;
	static const int DebugWindow_Height = 200;
	
	static const int List_X = 0;
	static const int List_Y = 0;
	static const int List_Width = 150;
	static const int List_Height = 180;
	
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




