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


#include "TokenCollection.h"

#include <iostream>
using namespace std;

//char input [ 256 ];

using namespace DataStructures;


TokenCollection* TokenCollection::Split ( string Str, string Delimiter )
{
	long CurrentPos = 0, LastPos = 0;
	
	while ( LastPos != string::npos )
	{
		LastPos = Str.find( Delimiter, CurrentPos );
		
		Tokens.push_back ( Str.substr ( CurrentPos, LastPos - CurrentPos ) );
		
		CurrentPos = LastPos + Delimiter.size();
	}

	return this;
}

// treats any non-printing characters as white space and delimiter
TokenCollection* TokenCollection::SplitTokens ( string Str )
{
	long LastPos = 1;
	
	while ( LastPos != string::npos )
	{
		LastPos = Str.find_first_not_of ( " \n\r\t" );

		if ( LastPos != string::npos )
		{
			Str = Str.erase ( 0, LastPos );
			
			LastPos = Str.find_first_of ( " \n\r\t" );
			
			Tokens.push_back ( Str.substr ( 0, LastPos ) );
			
			Str = Str.erase ( 0, LastPos );
			
		}
		
	}

	return this;
}


TokenCollection::~TokenCollection ()
{
	// delete all the strings in the vector - pop_back also calls their destructor
	while ( !Tokens.empty() ) Tokens.pop_back();
}



