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


#include "MipsOpcode.h"

#include "R5900Assembler.h"

void R5900::Assembler::SetAddress ( unsigned long *StartAddress )
{
	NextAddress = StartAddress;
}

void R5900::Assembler::NOP ( void )
{
	*NextAddress++ = 0;
}

void R5900::Assembler::ADDIU ( long rt, long rs, long immediate )
{
	*NextAddress++ = SET_OPCODE ( OPADDIU ) + SET_RT ( rt ) + SET_RS ( rs ) + SET_IMMED ( immediate );
}

void R5900::Assembler::ADDU ( long rd, long rs, long rt )
{
	*NextAddress++ = SET_OPCODE ( OPADDU ) + SET_RD( rd ) + SET_RS ( rs ) + SET_RT ( rt ) + SET_SPECIAL ( SPADDU );
}

void R5900::Assembler::AND ( long rd, long rs, long rt )
{
	*NextAddress++ = SET_OPCODE ( OPAND ) + SET_RD( rd ) + SET_RS ( rs ) + SET_RT ( rt ) + SET_SPECIAL ( SPAND );
}

void R5900::Assembler::ANDI ( long rt, long rs, long immediate )
{
	*NextAddress++ = SET_OPCODE ( OPANDI ) + SET_RT ( rt ) + SET_RS ( rs ) + SET_IMMED ( immediate );
}

void R5900::Assembler::BEQ ( long rs, long rt, long offset )
{
	*NextAddress++ = SET_OPCODE ( OPBEQ ) + SET_RS ( rs ) + SET_RT ( rt ) + SET_IMMED ( offset );
}

void R5900::Assembler::BEQL ( long rs, long rt, long offset )
{
	*NextAddress++ = SET_OPCODE ( OPBEQL ) + SET_RS ( rs ) + SET_RT ( rt ) + SET_IMMED ( offset );
}

void R5900::Assembler::LB ( long rt, long base, long offset )
{
	*NextAddress++ = SET_OPCODE ( OPLB ) + SET_RT ( rt ) + SET_BASE ( base ) + SET_IMMED ( offset );
}

void R5900::Assembler::SB ( long rt, long base, long offset )
{
	*NextAddress++ = SET_OPCODE ( OPSB ) + SET_RT ( rt ) + SET_BASE ( base ) + SET_IMMED ( offset );
}




