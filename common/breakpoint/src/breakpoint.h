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


#ifndef _BREAKPOINT_H_
#define _BREAKPOINT_H_

#include "StringUtils.h"
#include "DebugValueList.h"

class Debug_BreakPoints
{

	static const int MaxNumberOfBreakPoints = 8;
	
	// checks if string is a conditional
	static bool isConditional ( string st );

	class BreakPoint
	{
	public:
	
		static const int MaxNumberOfConditions = 8;
		
		int ComparisonType[ MaxNumberOfConditions ], LogicType[ MaxNumberOfConditions ];
		
		enum { COMPARE_EQUAL = 0, COMPARE_NOTEQUAL, COMPARE_LESS, COMPARE_LESSOREQUAL, COMPARE_GREATER, COMPARE_GREATEROREQUAL, COMPARE_BETWEEN, COMPARE_CHANGE };
		enum { LOGIC_NONE = 0, LOGIC_AND, LOGIC_OR };

		
		bool isLastBreakPoint;
		
		// number of times breakpoint has been reached
		int Count;
		
		// name of the breakpoint
		string Name;
		
		// breakpoint condition string
		string Condition;
		
		// reason for breaking to display when breakpoint is reached
		string Reason;

		// for a logical OR condition, you could just add another breakpoint
		int NumberOfConditions;
		
		// need to know if we should only compare physical address (clear top 3 bits)
		u64 CompareMask [ MaxNumberOfConditions ];
		
		s32* ValueToCompare [ MaxNumberOfConditions ];
		//s32 ValueGreaterOrEqualTo [ MaxNumberOfConditions ];
		//s32 ValueLesserOrEqualTo [ MaxNumberOfConditions ];
		s64 Arg0 [ MaxNumberOfConditions ];
		s64 Arg1 [ MaxNumberOfConditions ];

		
	
		// constructor
		BreakPoint () { NumberOfConditions = 0; }
	
		
		// returns true if breakpoint has been reached
		bool Check_BreakPoint ();
	};

	
	BreakPoint ListOfBreakPoints [ MaxNumberOfBreakPoints ];

public:

	// needs RAM, BIOS, and D-Cache pointers
	u8 *RAM, *BIOS, *DCACHE;
	
	int NumberOfBreakPoints;
	
	// the index of the last breakpoint that was triggered
	int Last_BreakPointIndex;
	
	// constructor
	Debug_BreakPoints ( u8* BIOS_PTR, u8* RAM_PTR, u8* DCACHE_PTR ) { NumberOfBreakPoints = 0; Clear_LastBreakPoint (); BIOS = BIOS_PTR; RAM = RAM_PTR; DCACHE = DCACHE_PTR; }

	// returns index of breakpoint added, returns -1 on error
	int Add_BreakPoint ( string Condition, string Name = "", string Reason = "" );
	
	bool Remove_BreakPoint ( int index );
	
	// set the last breakpoint that was triggered
	void Set_LastBreakPoint ( int index );
	
	// clear the last breakpoint that was triggered
	void Clear_LastBreakPoint ();
	
	int Get_LastBreakPoint ();
	
	inline int Count () { return NumberOfBreakPoints; }
	
	string Get_BreakPoint_Name ( int index );
	string Get_BreakPoint_Reason ( int index );
	string Get_BreakPoint_Condition ( int index );
	
	// checks if a breakpoint has been reached and returns its index if it has been
	// returns -1 if no breakpoint was reached
	int Check_IfBreakPointReached ();
	
	// checks if there is a primary breakpoint at address - the primary breakpoint will just be the first one for now
	// if found, returns the index of the breakpoint. Otherwise returns -1
	int isPrimaryBreakPoint ( u32 Address );
};


#endif

