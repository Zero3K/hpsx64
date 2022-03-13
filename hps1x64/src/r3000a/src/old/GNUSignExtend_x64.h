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


namespace x64SignExtend
{

	namespace Utilities
	{
		// use these for modifying variables that are thread sensitive
		
		inline short SignExtend8To16 ( char Op2 )
		{
			short Out;
			asm volatile ( "movsx %1, %0\n"
							: "+q" (Out)	// output to register a
							: "q" (Op2)	// input registers
							//: "0"	// clobbered register is same as output, so don't specify
							);
			return Out;
		}
		
		inline long SignExtend8To32 ( char Op2 )
		{
			long Out;
			asm volatile ( "movsx %1, %0\n"
							: "+q" (Out)	// output to register a
							: "q" (Op2)	// input registers
							//: "0"	// clobbered register is same as output, so don't specify
							);
			return Out;
		}
		
		inline long SignExtend16To32 ( short Op2 )
		{
			long Out;
			asm volatile ( "movsx %1, %0\n"
							: "+q" (Out)	// output to register a
							: "q" (Op2)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
			return Out;
		}

		inline long long SignExtend16To64 ( short Op2 )
		{
			long long Out;
			asm volatile ( "movsx %1, %0\n"
							: "+q" (Out)	// output to register a
							: "q" (Op2)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
			return Out;
		}

		inline long long SignExtend32To64 ( long Op2 )
		{
			long long Out;
			asm volatile ( "movsxd %1, %0\n"
							: "+q" (Out)	// output to register a
							: "q" (Op2)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
			return Out;
		}
		
	
	}

}

