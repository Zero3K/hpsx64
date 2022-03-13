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



#ifndef _GENERICDATAPORT_H_
#define _GENERICDATAPORT_H_

#include "types.h"

namespace Data
{

	namespace InOut
	{

		class Port
		{
		public:
			// this says whether data has been placed in the In/Out port or not
			u32 Enable;
			
			// says whether data is available yet
			
			// this is the command, like if you are transfering word, byte, etc.
			u32 Command;
			enum { CMD_LOAD_BYTE, CMD_LOAD_HALFWORD, CMD_LOAD_WORD, CMD_LOAD_BURST4, CMD_LOAD_INSTRUCTION, CMD_LOAD_LWL, CMD_LOAD_LWR,
				CMD_STORE_BYTE, CMD_STORE_HALFWORD, CMD_STORE_WORD, CMD_STORE_BURST4, CMD_STORE_SWL, CMD_STORE_SWR };
			
			// this is the address line for the data, which is where it is going to or coming from
			u32 SourceAddress;
			u32 DestinationAddress;
			
			// this is the actual data that is being transfered
			u32 Data;
			
			// any note about transfer
			u32 Note;

			// constructor
			Port ()
			{
				Enable = false;
			}
		};
		
	};
	
};

#endif

