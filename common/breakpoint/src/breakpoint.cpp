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


#include "breakpoint.h"

using namespace Utilities::Strings;




bool Debug_BreakPoints::isConditional ( string st )
{
	if ( st == "==" )
	{
		return true;
	}
	else if ( st == "!=" )
	{
		return true;
	}
	else if ( st == "<" )
	{
		return true;
	}
	else if ( st == "<=" )
	{
		return true;
	}
	else if ( st == ">" )
	{
		return true;
	}
	else if ( st == ">=" )
	{
		return true;
	}
	else if ( st == "<>" )
	{
		return true;
	}
	
	return false;
}

string Debug_BreakPoints::Get_BreakPoint_Name ( int index )
{
	return ListOfBreakPoints [ index ].Name;
}

string Debug_BreakPoints::Get_BreakPoint_Reason ( int index )
{
	return ListOfBreakPoints [ index ].Reason;
}

string Debug_BreakPoints::Get_BreakPoint_Condition ( int index )
{
	return ListOfBreakPoints [ index ].Condition;
}

bool Debug_BreakPoints::Remove_BreakPoint ( int index )
{
	int i;
	
	if ( ((unsigned int) index) >= ((unsigned int) NumberOfBreakPoints) ) return false;
	
	for ( i = index + 1; i < NumberOfBreakPoints; i++ )
	{
		ListOfBreakPoints [ i - 1 ] = ListOfBreakPoints [ i ];
	}
	
	NumberOfBreakPoints--;
	return true;
}


int Debug_BreakPoints::Add_BreakPoint ( string Condition, string Name, string Reason )
{
	int i;
	vector<string> vst;
	u32* Var;
	string st;
	s64 Value;
	
	// create a new breakpoint
	BreakPoint b;
		
	int CurrentCondition = 0;
	
	// make sure we did not run out of breakpoints
	if ( NumberOfBreakPoints > MaxNumberOfBreakPoints ) return -1;
	
	// remove any spaces in string
	Condition = Replace ( Condition, " ", "" );
	
	// set the condition for the breakpoint
	b.Condition = Condition;
	
	// this is not the last breakpoint hit yet
	b.isLastBreakPoint = false;
	
	// get string tokens
	vst = Tokenate ( Condition );
	
	// the first condition is always set to no logic
	b.LogicType [ 0 ] = BreakPoint::LOGIC_NONE;
	
	// loop through tokens
	for ( i = 0; i < vst.size(); i++ )
	{
		CurrentCondition = b.NumberOfConditions;
		
		for ( ; i < vst.size (); i++ )
		{
			// make sure it is not a number
			// edit: could be an address
			//if ( isNumeric ( vst [ i ] ) ) return -1;
			
			// make sure it is not a conditional
			if ( isConditional ( vst [ i ] ) ) return -1;
			
			// we need to check for a double underscore, which would mean we should only compare physical address
			// also need to check for asterisk, which indicates a memory address
			if ( InStr ( vst [ i ], "__" ) != string::npos )
			{
				// need to compare physical address only (like for PC breakpoint, Memory breakpoint, etc)
				b.CompareMask [ CurrentCondition ] = 0x1fffffff;
				
				// get rid of the double underscore
				vst [ i ] = Replace ( vst [ i ], "__", "" );
			}
			else
			{
				b.CompareMask [ CurrentCondition ] = 0xffffffffffffffffULL;
			}
			
			// check if token is variable
			Var = DebugValueList<u32>::FindVariableGlobal ( vst [ i ] );
			
			//cout << "\nToken=" << vst [ i ] << "\n";
			
			if ( !Var )
			{
				//cout << "\nNot Var. vst [ i ]=" << vst [ i ] << "\n";
				
				// check if it is a number
				if ( isNumeric ( vst [ i ] ) )
				{
					//cout << "\nisNumeric\n";
					
					Value = CLngLng ( vst [ i ] );
					
					// get physical address
					Value &= 0x1fffffff;
					
					//cout << "\nValue=" << Value << "\n";
					
					// check if value is physical ram address
					if ( ( ( Value >> 20 ) & 0x1fc ) == 0x1fc )
					{
						// should point into BIOS
						if ( BIOS )
						{
							Var = (u32*) & ( BIOS [ Value & 0x7ffff ] );
							b.CompareMask [ CurrentCondition ] = 0xffffffff;
						}
					}
					else if ( ( ( Value >> 20 ) & 0x1fc ) == 0x1f8 )
					{
						// should point into D-Cache
						if ( DCACHE )
						{
							Var = (u32*) & ( DCACHE [ Value & 0x3ff ] );
							b.CompareMask [ CurrentCondition ] = 0xffffffff;
						}
					}
					else if ( ( ( Value >> 20 ) & 0x1fc ) == 0x000 )
					{
						// should point into RAM
						if ( RAM )
						{
							Var = (u32*) & ( RAM [ Value & 0x1fffff ] );
							b.CompareMask [ CurrentCondition ] = 0xffffffff;
						}
					}
				}
			}
			
			if ( !Var ) return -1;
			
			// if it is a variable, then put pointer into breakpoint
			if ( Var )
			{
				
				// found variable in condition string
				b.ValueToCompare [ CurrentCondition ] = (s32*) Var;
				break;
			}
		}
		
		// get next token, check if comparator
		for ( i = i + 1; i < vst.size (); i++ )
		{
			// make sure it is not a variable
			if ( DebugValueList<u32>::FindVariableGlobal ( vst [ i ] ) ) return -1;
			
			// make sure it is not a number
			if ( isNumeric ( vst [ i ] ) ) return -1;
			
			// make sure it is a conditional
			if ( isConditional ( vst [ i ] ) )
			{

				st = vst [ i ];
				break;
			}
			else
			{
				return -1;
			}
		}
		
		// get next token, check that is value and store value
		for ( i = i + 1; i < vst.size (); i++ )
		{
			// make sure it is not a variable
			if ( DebugValueList<u32>::FindVariableGlobal ( vst [ i ] ) ) return -1;
			
			// make sure it is not a conditional
			if ( isConditional ( vst [ i ] ) ) return -1;
			
			// make sure it is a number
			if ( isNumeric ( vst [ i ] ) )
			{
				
				Value = CLngLng ( vst [ i ] );
				
				if ( st == "==" )
				{
					b.ComparisonType [ CurrentCondition ] = BreakPoint::COMPARE_EQUAL;
					b.Arg0 [ CurrentCondition ] = Value & b.CompareMask [ CurrentCondition ];
					//b.ValueGreaterOrEqualTo [ CurrentCondition ] = Value;
					//b.ValueLesserOrEqualTo [ CurrentCondition ] = Value;
				}
				else if ( st == "!=" )
				{
					// if ValueLesserOrEqual is less than ValueGreaterOrEqual, then it will be a special case for "does not equal"
					b.ComparisonType [ CurrentCondition ] = BreakPoint::COMPARE_NOTEQUAL;
					b.Arg0 [ CurrentCondition ] = Value & b.CompareMask [ CurrentCondition ];
					//b.ValueGreaterOrEqualTo [ CurrentCondition ] = Value + 1;
					//b.ValueLesserOrEqualTo [ CurrentCondition ] = Value - 1;
				}
				else if ( st == "<" )
				{
					b.ComparisonType [ CurrentCondition ] = BreakPoint::COMPARE_LESS;
					b.Arg0 [ CurrentCondition ] = Value & b.CompareMask [ CurrentCondition ];
					//b.ValueGreaterOrEqualTo [ CurrentCondition ] = 0;
					//b.ValueLesserOrEqualTo [ CurrentCondition ] = Value - 1;
				}
				else if ( st == "<=" )
				{
					b.ComparisonType [ CurrentCondition ] = BreakPoint::COMPARE_LESSOREQUAL;
					b.Arg0 [ CurrentCondition ] = Value & b.CompareMask [ CurrentCondition ];
					//b.ValueGreaterOrEqualTo [ CurrentCondition ] = 0;
					//b.ValueLesserOrEqualTo [ CurrentCondition ] = Value;
				}
				else if ( st == ">" )
				{
					b.ComparisonType [ CurrentCondition ] = BreakPoint::COMPARE_GREATER;
					b.Arg0 [ CurrentCondition ] = Value & b.CompareMask [ CurrentCondition ];
					//b.ValueGreaterOrEqualTo [ CurrentCondition ] = Value + 1;
					//b.ValueLesserOrEqualTo [ CurrentCondition ] = 0x7fffffff;
				}
				else if ( st == ">=" )
				{
					b.ComparisonType [ CurrentCondition ] = BreakPoint::COMPARE_GREATEROREQUAL;
					b.Arg0 [ CurrentCondition ] = Value & b.CompareMask [ CurrentCondition ];
					//b.ValueGreaterOrEqualTo [ CurrentCondition ] = Value;
					//b.ValueLesserOrEqualTo [ CurrentCondition ] = 0x7fffffff;
				}
				else if ( st == "<>" )
				{
					// this will indicate a value change breakpoint - will just put a zero or could even be any number after it
					b.ComparisonType [ CurrentCondition ] = BreakPoint::COMPARE_CHANGE;
					b.Arg0 [ CurrentCondition ] = Value & b.CompareMask [ CurrentCondition ];
					//b.ValueGreaterOrEqualTo [ CurrentCondition ] = Value;
					//b.ValueLesserOrEqualTo [ CurrentCondition ] = 0x7fffffff;
				}
				
				break;
			}
			else
			{
				return -1;
			}
		}
		
		// get next token, make sure is logical AND
		for ( i = i + 1; i < vst.size (); i++ )
		{
			// make sure it is not a variable
			if ( DebugValueList<u32>::FindVariableGlobal ( vst [ i ] ) ) return -1;
			
			// make sure it is not a conditional
			if ( isConditional ( vst [ i ] ) ) return -1;
			
			// make sure it is not a number
			if ( isNumeric ( vst [ i ] ) ) return -1;
			
			// check for logical AND
			if ( vst [ i ] == "&&" )
			{
				// need to set the logic to be performed at the next condition
				b.LogicType [ CurrentCondition + 1 ] = BreakPoint::LOGIC_AND;
				break;
			}
			else if ( vst [ i ] == "||" )	// check for logical OR
			{
				// need to set the logic to be performed at the next condition
				b.LogicType [ CurrentCondition + 1 ] = BreakPoint::LOGIC_OR;
				break;
			}
			else if ( vst [ i ] == "&!" )	// check for SIZE specifier
			{
				cout << "\nFound size specifier=" << vst [ i ];
				
				// the next token should be a number
				i++;
				
				// make sure it is not a variable
				if ( DebugValueList<u32>::FindVariableGlobal ( vst [ i ] ) ) return -1;
				
				// make sure it is not a conditional
				if ( isConditional ( vst [ i ] ) ) return -1;
				
				// make sure it is a number
				if ( isNumeric ( vst [ i ] ) )
				{
					// change this to a between comparison
					b.ComparisonType [ CurrentCondition ] = BreakPoint::COMPARE_BETWEEN;
					
					// add number minus one to lesser or equal condition
					b.Arg1 [ CurrentCondition ] = ( b.Arg0 [ CurrentCondition ] + CLngLng( vst [ i ] ) - 1 ) & b.CompareMask [ CurrentCondition ];
				}
				
				// for loop again to find logical and condition if it is there
			}
			else
			{
				return -1;
			}
		}
		
		// put in the next condition for breakpoint
		b.NumberOfConditions++;
		
		if ( b.NumberOfConditions >= BreakPoint::MaxNumberOfConditions ) return -1;
	}
	
	
	// add the breakpoint
	i = NumberOfBreakPoints;
	ListOfBreakPoints [ NumberOfBreakPoints++ ] = b;
	ListOfBreakPoints [ i ].Name = Name;
	ListOfBreakPoints [ i ].Reason = Reason;
	
	return i;
}

bool Debug_BreakPoints::BreakPoint::Check_BreakPoint ()
{
	int i;
	u32 _CurrentCondition, _TempCondition;
	s64 _ValueCompare;
	static const unsigned long Mask_VirtualAddress = 0x1fffffff;
	
	//bool bConditionReached = true;
	
	if ( !NumberOfConditions ) return false;
	
	for ( i = 0; i < NumberOfConditions; i++ )
	{
		_ValueCompare = (s64) (*ValueToCompare [ i ] & CompareMask [ i ]);
		
		switch ( ComparisonType [ i ] )
		{
			case COMPARE_EQUAL:
				_TempCondition = _ValueCompare == Arg0 [ i ];
				break;
				
			case COMPARE_NOTEQUAL:
				_TempCondition = _ValueCompare != Arg0 [ i ];
				break;
				
			case COMPARE_LESS:
				_TempCondition = _ValueCompare < Arg0 [ i ];
				break;
				
			case COMPARE_LESSOREQUAL:
				_TempCondition = _ValueCompare <= Arg0 [ i ];
				break;
				
			case COMPARE_GREATER:
				_TempCondition = _ValueCompare > Arg0 [ i ];
				break;
				
			case COMPARE_GREATEROREQUAL:
				_TempCondition = _ValueCompare >= Arg0 [ i ];
				break;
				
			case COMPARE_BETWEEN:
				_TempCondition = _ValueCompare >= Arg0 [ i ] && _ValueCompare <= Arg1 [ i ];
				break;
				
			case COMPARE_CHANGE:
				_TempCondition = _ValueCompare != Arg0 [ i ];
				
				// *note* because of this, it will only be possible to check for breakpoints once
				if ( _TempCondition ) Arg0 [ i ] = _ValueCompare;
				break;
		};
		
		switch ( LogicType [ i ] )
		{
			case LOGIC_NONE:
				_CurrentCondition = _TempCondition;
				break;
			
			case LOGIC_AND:
				_CurrentCondition = _CurrentCondition && _TempCondition;
				break;
				
			case LOGIC_OR:
				_CurrentCondition = _CurrentCondition || _TempCondition;
				break;
		};
		
		/*
		// check if virtual address
		if ( isVirtualAddress [ i ] )
		{
			// check if condition has not been reached
			if ( ValueGreaterOrEqualTo [ i ] > ValueLesserOrEqualTo [ i ] )
			{
				// special case for "does not equal"
				if ( ( *(ValueToCompare [ i ]) & Mask_VirtualAddress ) == ( ( ValueLesserOrEqualTo [ i ] + 1 ) & Mask_VirtualAddress ) )
				{
					bConditionReached = false;
					break;
				}
			}
			else
			{
				if ( !( ( *(ValueToCompare [ i ]) & Mask_VirtualAddress ) >= ( ValueGreaterOrEqualTo [ i ] & Mask_VirtualAddress ) && ( *(ValueToCompare [ i ]) & Mask_VirtualAddress ) <= ( ValueLesserOrEqualTo [ i ] & Mask_VirtualAddress ) ) )
				{
					bConditionReached = false;
					break;
				}
			}
		}
		else
		{
			// check if condition has not been reached
			if ( ValueGreaterOrEqualTo [ i ] > ValueLesserOrEqualTo [ i ] )
			{
				// special case for "does not equal"
				if ( *(ValueToCompare [ i ]) == ( ValueLesserOrEqualTo [ i ] + 1 ) )
				{
					bConditionReached = false;
					break;
				}
			}
			else
			{
				if ( !( *(ValueToCompare [ i ]) >= ValueGreaterOrEqualTo [ i ] && *(ValueToCompare [ i ]) <= ValueLesserOrEqualTo [ i ] ) )
				{
					bConditionReached = false;
					break;
				}
			}
		}
		*/
	}
	
	return _CurrentCondition;
	//return bConditionReached;
}


int Debug_BreakPoints::Check_IfBreakPointReached ()
{
	int i;
	
	for ( i = 0; i < NumberOfBreakPoints; i++ )
	{
		if ( ListOfBreakPoints [ i ].Check_BreakPoint () )
		{
			ListOfBreakPoints [ i ].isLastBreakPoint = true;
			return i;
		}
	}
	
	return -1;
}


int Debug_BreakPoints::isPrimaryBreakPoint ( u32 Address )
{
	int i;
	
	for ( i = 0; i < NumberOfBreakPoints; i++ )
	{
		//if ( ( ListOfBreakPoints [ i ].ValueGreaterOrEqualTo [ 0 ] & 0x1fffffff ) == ( Address & 0x1fffffff ) )
		if ( ListOfBreakPoints [ i ].Arg0 [ 0 ] == ( Address & 0x1fffffff ) )
		{
			return i;
		}
	}
	
	return -1;
}



// set the last breakpoint that was triggered
void Debug_BreakPoints::Set_LastBreakPoint ( int index )
{
	ListOfBreakPoints [ index ].isLastBreakPoint = true;
	//Last_BreakPointIndex = index;
}

// clear the last breakpoint that was triggered
void Debug_BreakPoints::Clear_LastBreakPoint ()
{
	//Last_BreakPointIndex = -1;
	int i;
	
	for ( i = 0; i < NumberOfBreakPoints; i++ )
	{
		ListOfBreakPoints [ i ].isLastBreakPoint = false;
	}
}

int Debug_BreakPoints::Get_LastBreakPoint ()
{
	//return Last_BreakPointIndex;
	int i;
	
	for ( i = 0; i < NumberOfBreakPoints; i++ )
	{
		if ( ListOfBreakPoints [ i ].isLastBreakPoint ) return i;
	}
	
	return -1;
}





