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


#ifndef _GNUTHREADING_X64_H_
#define _GNUTHREADING_X64_H_


#define GCC_OR_CLANG_COMPILER (defined(__GNUC__) || defined(__clang__))


#include <intrin.h>


namespace x64ThreadSafe
{

	namespace Utilities
	{
		// use these for modifying variables that are thread sensitive

#if GCC_OR_CLANG_COMPILER

		inline void PAUSE ()
		{
			asm volatile ( "pause\n" );
		}
		
		inline void Store_Fence ()
		{
			asm volatile ( "sfence\n" );
		}
		
		/*
		inline void Lock_OR32 ( long& Value, long Op2 )
		{
			asm volatile ( "lock or %1, %0\n"
							: "+m" (Value)	// output to register a
							: "q" (Op2)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
		}
		
		inline void Lock_OR32_Imm ( long& Value, const long Op2 )
		{
			asm volatile ( "lock or %1, %0\n"
							: "+m" (Value)	// output to register a
							: "e" (Op2)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
		}

		inline void Lock_OR64 ( long long& Value, long long Op2 )
		{
			asm volatile ( "lock or %1, %0\n"
							: "+m" (Value)	// output to register a
							: "q" (Op2)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
		}
		
		// you can only do this with a 32-bit constant
		inline void Lock_OR64_Imm ( long long& Value, const long Op2 )
		{
			asm volatile ( "lock or %1, %0\n"
							: "+m" (Value)	// output to register a
							: "e" (Op2)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
		}

		inline void Lock_AND32 ( long& Value, long Op2 )
		{
			asm volatile ( "lock and %1, %0\n"
							: "+m" (Value)	// output to register a
							: "q" (Op2)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
		}

		inline void Lock_XOR32 ( long& Value, long Op2 )
		{
			asm volatile ( "lock xor %1, %0\n"
							: "+m" (Value)	// output to register a
							: "q" (Op2)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
		}
	
		inline void Lock_BitSet32 ( long& Value, const int BitToSet )
		{
			asm volatile ( "lock bts %1, %0\n"
							: "+m" (Value)	// output to register a
							: "I" (BitToSet)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
		}
		
		inline void Lock_BitSet64 ( long long& Value, const int BitToSet )
		{
			asm volatile ( "lock bts %1, %0\n"
							: "+m" (Value)	// output to register a
							: "J" (BitToSet)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
		}
		*/
		
		inline long Lock_Exchange32 ( long& Value, long Op2 )
		{
			long Out;
			asm volatile ( "mov %2, %1\n"
							"xchg %0, %1\n"
							: "+m" (Value), "=q" (Out)	// output to register a
							: "q" (Op2)	// input registers
							//: "0"	// clobbered register is same as output, so don't specify
							);
			return Out;
		}
		
		inline long Lock_ExchangeAdd32 ( long& Value, long Op2 )
		{
			long Out;
			asm volatile ( "mov %2, %1\n"
							"lock xadd %1, %0\n"
							: "+m" (Value), "=q" (Out)	// output to register a
							: "q" (Op2)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
			return Out;
		}

		inline long long Lock_Exchange64 ( long long& Value, long long Op2 )
		{
			long long Out;
			asm volatile ( "mov %2, %1\n"
							"xchg %0, %1\n"
							: "+m" (Value), "=q" (Out)	// output to register a
							: "q" (Op2)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
			return Out;
		}
		
		inline long long Lock_ExchangeAdd64 ( long long& Value, long long Op2 )
		{
			long long Out;
			asm volatile ( "mov %2, %1\n"
							"lock xadd %1, %0\n"
							: "+m" (Value), "=q" (Out)	// output to register a
							: "q" (Op2)	// input registers
							//: "eax"	// clobbered register is same as output, so don't specify
							);
			return Out;
		}

#else


		inline void PAUSE()
		{
		}

		inline void Store_Fence()
		{
		}

		inline void Lock_OR32(long& Value, long Op2)
		{
		}

		inline void Lock_OR32_Imm(long& Value, const long Op2)
		{
		}

		inline void Lock_OR64(long long& Value, long long Op2)
		{
		}

		// you can only do this with a 32-bit constant
		inline void Lock_OR64_Imm(long long& Value, const long Op2)
		{
		}

		inline void Lock_AND32(long& Value, long Op2)
		{
		}

		inline void Lock_XOR32(long& Value, long Op2)
		{
		}

		inline void Lock_BitSet32(long& Value, const int BitToSet)
		{
		}

		inline void Lock_BitSet64(long long& Value, const int BitToSet)
		{
		}

		inline long Lock_Exchange32(long& Value, long Op2)
		{
			long Out;
			Out = _InterlockedExchange(&Value, Op2);
			return Out;
		}

		inline long Lock_ExchangeAdd32(long& Value, long Op2)
		{
			long Out;
			Out = _InterlockedExchangeAdd(&Value, Op2);
			return Out;
		}

		inline long long Lock_Exchange64(long long& Value, long long Op2)
		{
			long long Out;
			Out = _InterlockedExchange64(&Value, Op2);
			return Out;
		}

		inline long long Lock_ExchangeAdd64(long long& Value, long long Op2)
		{
			long long Out;
			Out = _InterlockedExchangeAdd64(&Value, Op2);
			return Out;
		}


#endif
	
	}	// end namespace Utilities

}	// end namespace x64ThreadSafe

#endif

