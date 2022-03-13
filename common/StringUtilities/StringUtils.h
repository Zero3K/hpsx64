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


#ifndef _STRINGUTILS_H_
#define _STRINGUTILS_H_


#include <algorithm>
#include <string>
#include <sstream>
#include <vector>

#include <iostream>


namespace Utilities
{

	namespace Strings
	{
using namespace std;

		// find location of one string within another string
		// returns std::npos when string is not found
		int InStr ( string StringToSearch, string StringToFind, int StartPos = 0 );
		
		// find location of one string within another string starting from end of string
		int InStrRev ( string StringToSearch, string StringToFind );

		// convert a string into lower case
		string LCase ( string StringToConvert );
		
		// convert a string into upper case
		string UCase ( string StringToConvert );

		// remove extra spaces from right of string
		string RTrim ( string StringToConvert );

		// remove extra spaces from left of string
		string LTrim ( string StringToConvert );

		// remove extra spaces anywhere in string
		string Trim ( string StringToConvert );

		// get text from the left of a string
		string Left ( string StringToConvert, int NumberOfChars = 1 );
		
		// get text from the middle of a string
		string Mid ( string StringToConvert, int StartPos, int NumberOfChars = string::npos );

		// get text from the right of a string
		string Right ( string StringToConvert, int NumberOfChars );
		
		// replace text in a string
		string Replace ( string StringToConvert, string TextToFind, string ReplaceText );
		
		// get length of string
		int Len ( string StringToSearch );
		
		bool isNumeric ( string StringToConvert );
		
		long CLng ( string StringToConvert );
		long long CLngLng ( string StringToConvert );
		
		
		string CStr ( long long Value );
		string CStr_Hex ( long long Value );
		
		
		vector<string> Split ( string StringToConvert, string Delimiter );
		
		vector<string> Tokenate ( string StringToConvert );
		
		// stuff for working with full file paths //
		string GetPath ( string sFullFilePath );
		string GetExtension ( string sFullFilePath );
		string GetFileAndExtension ( string sFullFilePath );
		string GetFile ( string sFullFilePath );
	}
}

#endif


