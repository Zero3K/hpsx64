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


#include "types.h"

#ifndef _VU_INSTRUCTION_
#define _VU_INSTRUCTION_

namespace Vu
{

	namespace Instruction
	{

		struct Format
		{
			union
			{
				struct
				{
					u32 Funct : 6;
					u32 Fd : 5;
					u32 Fs : 5;
					u32 Ft : 5;
					u32 dest : 4;
					u32 Opcode : 7;
				};
				
				struct
				{
					s32 Imm11 : 11;
					u32 filler0 : 10;
					u32 destw : 1;
					u32 destz : 1;
					u32 desty : 1;
					u32 destx : 1;
					u32 filler1 : 2;
					u32 T : 1;
					u32 D : 1;
					u32 M : 1;
					u32 E : 1;
					u32 I : 1;
				};
				
				struct
				{
					u32 Imm15_0 : 11;
					u32 filler6 : 5;
					u32 filler7 : 5;
					u32 Imm15_1 : 4;
					u32 filler2 : 7;
				};
				
				
				struct
				{
					u32 filler3 : 6;
					u32 id : 5;
					u32 is : 5;
					u32 it : 5;
					u32 fsf : 2;
					u32 ftf : 2;
					u32 filler4 : 7;
				};
				
				struct
				{
					u32 Imm24 : 24;
					u32 filler5 : 8;
				};
				
				struct
				{
					u32 filler8 : 6;
					s32 Imm5 : 5;
					u32 filler9 : 5;
					u32 filler10 : 5;
					u32 xyzw : 4;
					u32 filler12 : 7;
				};
				
				u32 Value;
			};
			
		};
		
		
		union Format2
		{
			struct
			{
				Format Lo;
				Format Hi;
			};
			
			struct
			{
				
				u32 Value2;
				
				union
				{
					struct
					{
						u32 Funct : 6;
						u32 Fd : 5;
						u32 Fs : 5;
						u32 Ft : 5;
						u32 dest : 4;
						u32 Opcode : 7;
					};
					
					struct
					{
						s32 Imm11 : 11;
						u32 filler0 : 10;
						u32 destw : 1;
						u32 destz : 1;
						u32 desty : 1;
						u32 destx : 1;
						u32 filler1 : 2;
						u32 T : 1;
						u32 D : 1;
						u32 M : 1;
						u32 E : 1;
						u32 I : 1;
					};
					
					struct
					{
						u32 Imm15_0 : 11;
						u32 filler6 : 5;
						u32 filler7 : 5;
						u32 Imm15_1 : 4;
						u32 filler2 : 7;
					};
					
					
					// commenting this out so there is not mixup in the transition to passing the 64-bit VU instruction
					/*
					struct
					{
						u32 filler3 : 6;
						u32 id : 5;
						u32 is : 5;
						u32 it : 5;
						u32 fsf : 2;
						u32 ftf : 2;
						u32 filler4 : 7;
					};
					*/
					
					struct
					{
						u32 Imm24 : 24;
						u32 filler5 : 8;
					};
					
					struct
					{
						u32 filler8 : 6;
						s32 Imm5 : 5;
						u32 filler9 : 5;
						u32 filler10 : 5;
						u32 xyzw : 4;
						u32 filler12 : 7;
					};
					
					u32 Value;
				};
			};
			
			u64 ValueLoHi;
		};
	
	};

};

#endif


