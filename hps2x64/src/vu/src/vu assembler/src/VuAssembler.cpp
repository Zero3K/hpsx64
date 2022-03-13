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


#include "MipsOpcode.h"

#include "VuAssembler.h"

void Vu::Assembler::SetAddress ( unsigned long *StartAddress )
{
	NextAddress = StartAddress;
}

void Vu::Assembler::ADDIU ( long rs, long rt, short offset )
{
	*NextAddress++ = SET_OPCODE ( OPADDIU ) + SET_RS ( rs ) + SET_RT ( rt ) + SET_IMMED ( offset );
}

