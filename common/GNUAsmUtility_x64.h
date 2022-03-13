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


#ifndef _GNUASMUTILITY_X64_H_
#define _GNUASMUTILITY_X64_H_

namespace x64Asm
{

	namespace Utilities
	{
		// use these for modifying variables that are thread sensitive
		
		
		inline unsigned long POPCNT ( long Op1 )
		{
			unsigned long ReturnValue;
			asm volatile ( "popcnt %1, %0\n"
							: "=q" (ReturnValue)	// output to register a
							: "q" (Op1)	// input registers
							//: "0"	// clobbered register is same as output, so don't specify
							);
			return ReturnValue;
		}
		
		inline unsigned long BSR ( long Op1 )
		{
			unsigned long ReturnValue;
			asm volatile ( "bsr %1, %0\n"
							: "+q" (ReturnValue)	// output to register a
							: "q" (Op1)	// input registers
							//: "0"	// clobbered register is same as output, so don't specify
							);
			return ReturnValue;
		}

		/////////////////////////////////////////////
		// saturating decrement
		inline long sdec32 ( long Op1 )
		{
			asm volatile ( "sub $1, %0\n"
							"adc $0, %0\n"
							: "+q" (Op1)	// output to register a
							//: "q" (Op2)	// input registers
							//: "0"	// clobbered register is same as output, so don't specify
							);
			return Op1;
		}
		
		//////////////////////////////////////////////////
		// exchange
		static inline void Exchange32 ( unsigned long &Op1, unsigned long &Op2 )
		{
			asm volatile ( "xchg %0, %1\n"
							: "+q" (Op1), "+q" (Op2)	// output to register a
							//: "q" (Op2)	// input registers
							//: "0"	// clobbered register is same as output, so don't specify
							);
		}
	
		static inline void Exchange32 ( signed long &Op1, signed long &Op2 )
		{
			asm volatile ( "xchg %0, %1\n"
							: "+q" (Op1), "+q" (Op2)	// output to register a
							//: "q" (Op2)	// input registers
							//: "0"	// clobbered register is same as output, so don't specify
							);
		}
		
		static inline void Exchange64 ( long long &Op1, long long &Op2 )
		{
			asm volatile ( "xchg %0, %1\n"
							: "+q" (Op1), "+q" (Op2)	// output to register a
							//: "q" (Op2)	// input registers
							//: "0"	// clobbered register is same as output, so don't specify
							);
		}
	}

}

#endif

