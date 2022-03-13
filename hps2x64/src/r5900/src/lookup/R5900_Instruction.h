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

#ifndef _R5900_INSTRUCTION_
#define _R5900_INSTRUCTION_

namespace R5900
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
					u32 Shift : 5;
					u32 Rd : 5;
					u32 Rt : 5;
					u32 Rs : 5;
					u32 Opcode : 6;
				};
				
				
				
				struct
				{
					s32 sOffset : 16;
					u32 filler0 : 5;
					u32 Base : 5;
					u32 filler1 : 6;
				};
				
				struct
				{
					u32 uImmediate : 16;
					u32 filler2 : 16;
				};
				
				struct
				{
					s32 sImmediate : 16;
					s32 filler3 : 16;
				};

				struct
				{
					u32 JumpAddress : 26;
					u32 filler4 : 6;
				};
				
				struct
				{
					// bits 0-9 - lower part of GTE instruction
					u32 filler5 : 10;
					
					// bit 10 - 0: no negative limit; 1: limit negative results to zero
					u32 lm : 1;
					
					// bits 11-12
					u32 filler6 : 2;
					
					// bits 13-14 - 0: add translation vector (TR); 1: add back color vector (BK); 2: add far color vector (FC); 3: add no vector
					u32 cv : 2;
					
					// bits 15-16 - 0: V0 source vector (short); 1: V1 source vector (short); 2: V2 source vector (short); 3: IR source vector (long)
					u32 v : 2;
					
					// bits 17-18 - 0: multiply with rotation matrix; 1: multiply with light matrix; 2: multiply with color matrix; 3: -
					u32 mx : 2;
					
					// bit 19 - 0: normal calculation; 1: calculations on data shifted 12 bits to the left in the IR regs
					u32 sf : 1;
					
					// bits 20-31
					u32 filler7 : 12;
				};
				
				struct
				{
					u32 filler8 : 6;
					u32 Fd : 5;
					u32 Fs : 5;
					u32 Ft : 5;
					u32 SWD : 5;
					u32 filler9 : 6;
				};
				
				
				// for VCALLMS
				struct
				{
					u32 filler10 : 6;
					u32 Imm15 : 15;
					u32 filler11: 11;
				};
				
				// for VU0 macro mode
				struct
				{
					u32 filler12 : 6;
					u32 id : 5;
					u32 is : 5;
					u32 it : 5;
					u32 fsf : 2;
					u32 ftf : 2;
					u32 filler13 : 7;
				};
				
				struct
				{
					u32 filler14 : 6;
					s32 Imm5 : 5;
					u32 filler15 : 5;
					u32 filler16 : 5;
					u32 xyzw : 4;
					u32 filler18 : 7;
				};
				
				struct
				{
					s32 Imm11 : 11;
					u32 filler19 : 10;
					u32 destw : 1;
					u32 destz : 1;
					u32 desty : 1;
					u32 destx : 1;
					u32 filler20 : 2;
					u32 T : 1;
					u32 D : 1;
					u32 M : 1;
					u32 E : 1;
					u32 I : 1;
				};
				
				
				u32 Value;
			};
			
		};
	
	};

};

#endif


