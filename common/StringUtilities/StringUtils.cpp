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


#include "StringUtils.h"


namespace Utilities
{

	namespace Strings
	{
		// find location of one string within another string
		int InStr ( string StringToSearch, string StringToFind, int StartPos )
		{
			return StringToSearch.find( StringToFind, StartPos );
		}
		
		// find location of one string within another string starting from end of string
		int InStrRev ( string StringToSearch, string StringToFind )
		{
			return StringToSearch.rfind( StringToFind );
		}

		// convert a string into lower case
		string LCase ( string StringToConvert )
		{
			std::transform( StringToConvert.begin(), StringToConvert.end(), StringToConvert.begin(), ::tolower);
			return StringToConvert;
		}
		
		// convert a string into upper case
		string UCase ( string StringToConvert )
		{
			std::transform( StringToConvert.begin(), StringToConvert.end(), StringToConvert.begin(), ::toupper);
			return StringToConvert;
		}

		// remove extra spaces from right of string
		string RTrim ( string StringToConvert )
		{
			int i;
			i = StringToConvert.find_last_not_of ( " \n\r\t" );
			if ( i == string::npos ) return "";
			if ( i == ( StringToConvert.size() - 1 ) ) return StringToConvert;
			return StringToConvert.erase ( i + 1 );
		}

		// remove extra spaces from left of string
		string LTrim ( string StringToConvert )
		{
			return StringToConvert.erase ( 0, StringToConvert.find_first_not_of ( " \n\r\t" ) );
		}

		// remove extra spaces anywhere in string
		string Trim ( string StringToConvert )
		{
			return LTrim ( RTrim ( StringToConvert ) );
		}

		// get text from the left of a string
		string Left ( string StringToConvert, int NumberOfChars )
		{
			return StringToConvert.substr ( 0, NumberOfChars );
		}
		
		// get text from the middle of a string
		// Note: StartPos starts from zero
		string Mid ( string StringToConvert, int StartPos, int NumberOfChars )
		{
			return StringToConvert.substr ( StartPos, NumberOfChars );
		}

		// get text from the right of a string
		string Right ( string StringToConvert, int NumberOfChars )
		{
			return StringToConvert.substr ( StringToConvert.length() - NumberOfChars );
		}
		
		// replace text in a string
		string Replace ( string StringToConvert, string TextToFind, string ReplaceText )
		{
			int CurrentPos = 0;
			
			while ( CurrentPos != string::npos )
			{
				CurrentPos = StringToConvert.find( TextToFind );

				if ( CurrentPos != string::npos ) StringToConvert = StringToConvert.replace ( CurrentPos, TextToFind.length(), ReplaceText );
			}
			
			return StringToConvert;
		}
		
		int Len ( string StringToSearch )
		{
			return StringToSearch.length();
		}
		
		bool isNumeric ( string StringToConvert )
		{
			int i;
			unsigned long long output;
			string st;
			
			st = LCase( Trim ( StringToConvert ) );
			
			i = InStr ( st, "0x" );
			
			//cout << "\ni=" << i;
			
			st = Replace ( st, "0x", "" );
			
			//cout << "\nst=" << st.c_str();
			
			istringstream iss ( st.c_str () );
			
			if ( !i )
			{
				//cout << "\nhexadecimal string=" << iss.str().c_str();
				
				// string is in hexadecimal
				if ( (iss >> hex >> output).fail() ) return false;
			}
			else
			{
				//cout << "\ndecimal string=" << iss.str().c_str();
				
				// string is in decimal
				if ( (iss >> dec >> output).fail() ) return false;
			}
			
			return true;
		}
		
		long CLng ( string StringToConvert )
		{
			int i;
			unsigned long output;
			string st;
			
			st = LCase( Trim ( StringToConvert ) );
			
			i = InStr ( st, "0x" );
			
			st = Replace ( st, "0x", "" );
			
			istringstream iss ( st.c_str () );
			
			if ( !i )
			{
				// string is in hexadecimal
				if ( (iss >> hex >> output).fail() ) output = 0;
			}
			else
			{
				// string is in decimal
				if ( (iss >> dec >> output).fail() ) output = 0;
			}
			
			return output;
		}
		
		long long CLngLng ( string StringToConvert )
		{
			int i;
			unsigned long long output;
			string st;
			
			st = LCase( Trim ( StringToConvert ) );
			
			i = InStr ( st, "0x" );
			
			st = Replace ( st, "0x", "" );
			
			istringstream iss ( st.c_str () );
			
			if ( !i )
			{
				// string is in hexadecimal
				if ( (iss >> hex >> output).fail() ) output = 0;
			}
			else
			{
				// string is in decimal
				if ( (iss >> dec >> output).fail() ) output = 0;
			}
			
			return output;
		}
		
		
		string CStr ( long long Value )
		{
			stringstream ss;
			ss << dec << Value;
			return ss.str ();
		}
		
		string CStr_Hex ( long long Value )
		{
			stringstream ss;
			ss << hex << Value;
			return ss.str ();
		}
		
		
		
		vector<string> Split ( string StringToConvert, string Delimiter )
		{
			int CurrentPos = 0, NextPos = 0;
			vector<string> vst;
			string st;
			
			if ( !Delimiter.size() ) return vst;
			
			while ( NextPos != string::npos )
			{
				//cout << "\nCurrentPos=" << CurrentPos;
			
				// find delimiter
				NextPos = StringToConvert.find ( Delimiter, CurrentPos );
				
				//cout << " NextPos=" << NextPos;
				
				if ( NextPos == string::npos )
				{
					// get token
					st = StringToConvert.substr ( CurrentPos, string::npos );
				}
				else
				{
					// get token
					st = StringToConvert.substr ( CurrentPos, NextPos - CurrentPos );
				}
				
				//cout << "\nToken=" << st.c_str();
				
				// add token into vector
				vst.push_back ( st );
				
				// set new position in string
				CurrentPos = NextPos + Delimiter.size ();
			}
			
			return vst;
		}
		
		vector<string> Tokenate ( string StringToConvert )
		{
			static const char* Separators = " \n\r\t()=!<>&|";
		
			int CurrentPos = 0, NextPos = 0;
			vector<string> vst;
			string st;

			while ( NextPos != string::npos )
			{
				// get end of token
				NextPos = StringToConvert.find_first_not_of ( Separators, CurrentPos );
				
				if ( CurrentPos == NextPos ) NextPos = StringToConvert.find_first_of ( Separators, CurrentPos );

				if ( NextPos == string::npos )
				{
					// get token
					st = StringToConvert.substr ( CurrentPos, string::npos );
				}
				else
				{
					// get token
					st = StringToConvert.substr ( CurrentPos, NextPos - CurrentPos );
				}
				
				// add token into vector
				vst.push_back ( st );
				
				// set new position in string
				CurrentPos = NextPos;
			}
			
			return vst;
		}
		
		string GetPath ( string sFullFilePath )
		{
			return Left ( sFullFilePath, InStrRev ( sFullFilePath, "\\" ) + 1 );
		}

		// includes the dot
		string GetExtension ( string sFullFilePath )
		{
			return Mid ( sFullFilePath, InStrRev ( sFullFilePath, "." ) );
		}
		
		string GetFileAndExtension ( string sFullFilePath )
		{
			return Mid ( sFullFilePath, InStrRev ( sFullFilePath, "\\" ) + 1 );
		}
		
		string GetFile ( string sFullFilePath )
		{
			return Mid ( sFullFilePath, InStrRev ( sFullFilePath, "\\" ) + 1, InStrRev ( sFullFilePath, "." ) - InStrRev ( sFullFilePath, "\\" ) - 1 );
		}

	}
}


