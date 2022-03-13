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



#ifndef _VUUNIT_H_

#define _VUUNIT_H_


#include "WinApiHandler.h"

#include "typedefs.h"

//#include "VuEncoder.h"
//#include "x64Encoder.h"

using namespace std;



//struct VuFloatReg
//{
//	FloatLong x;
//	FloatLong y;
//	FloatLong z;
//	FloatLong w;
//};

namespace Vu
{

	class Cpu
	{

	public:

		// this does not have to be saved with system state
		volatile long object_locked;

		bool debug_enabled;
		
		static const long InstructionSize = 8;

		// A pointer to the debug window
		WindowClass::Window* DebugWindow;
		
		x64Encoder* x;
		VuEncoder* r;
		
		long VuUnitNumber;
		

		FloatLong4 Vf [ 32 ], ACC;

		// vu flags
		DoubleLong4 MACFlagOverflow, MACFlagUnderflow, MACFlagSign, MACFlagZero;
		DoubleLong4 StickyFlagDivide, StickyFlagInvalid, StickyFlagOverflow, StickyFlagUnderflow, StickyFlagSign, StickyFlagZero;
		FloatLong4 StatusFlagDivide, StatusFlagInvalid, StatusFlagOverflow, StatusFlagUnderflow, StatusFlagSign, StatusFlagZero;

		short Vi [ 32 ];

		FloatLong4 I, P, Q, R;

		long MACFlags, StickyFlags, StatusFlags, ClippingFlag;
		
		long PC;

		volatile long CycleCount;
		long CycleTarget;
		
		volatile long Stop;

		char ALIGN16 Mem [ 16384 ];	// this is the Vu Data memory
		char ALIGN16 MicroMem [ 16384 ];	// MicroMem is the Vu instruction memory
		


		// constructor
		Cpu ( void );
		
		// destructor
		~Cpu ( void );
		
		// create
		bool Create ( long VuUnitNum );

		
		// opens debug window and prepares for writing
		void EnableDebugging ( void );

		// can only do this after enabling debugging
		void UpdateDebugWindow ( void );

		void SetProcessorSpeed ( unsigned long long frequency );

		bool Start ( void );
		
		void Execute ( long NumberOfCycles );

		void EncodeInstructions ( unsigned long long ProgramCounter, unsigned long long NumberOfInstructions );

		bool Halt ( void );

		// resets the data for the processor
		void Reset ( void );
		
		// allow you to single step instructions
		long Step ( void );
		
		// signals an interrupt to occur at the next possible moment
		void SignalInterrupt ( long Interrupt );
		
	private:

		bool isDecoded ( void );
		long DecodeInstructions ( long NumberOfCyclesToDecode );
		void ExecuteInstructions ( void );

	};

}


#endif




