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


#include <string>
#include <iostream>
#include <fstream>

using namespace std;


#ifndef _CONFIGFILE_H_

#define _CONFIGFILE_H_

namespace Config
{
	class PSXDiskUtility
	{
	public:
		// GetPSIDString
		// gets the format SLXX_YYY.ZZ string for the disk
		// string should be exactly 13 characters, or maybe 11 or 12
		// works for both PS1 and PS2
		// file must be initially closed
		static bool GetPSXIDString ( char* Output, const char* PSXFileName, int DiskSectorSize );

		static unsigned long GetLayer1Start ( char* Output, const char* PSXFileName, int DiskSectorSize );
		
		// returns TRUE if the file is a Playstation game CD, FALSE otherwise (meaning it might be a game DVD)
		static bool isDataCD ( const char* PSXFileName );
	};

	class File
	{
		// the maximum size of the config file. Does NOT need to be a power of two
		static const int c_iConfigFile_MaxSize = 32768;
		
		//static const char c_cDelimiter = '\n';
		//static const char c_cAssigner = '=';
		static const char* c_sDelimiter;
		static const char* c_sAssigner;
		
		// need the prefix and postfix since the variable value could contain the variable name
		static const char* c_sVarPrefix;
		static const char* c_sVarPostfix;
		
	public:
		char cData [ c_iConfigFile_MaxSize ];
		
		
		bool Load ( string NameOfFile );
		bool Save ( string NameOfFile );
		void Clear ();
		
		bool Get_Value32 ( string VarName, long& Value );
		bool Get_Value64 ( string VarName, long long& Value );
		bool Get_String ( string VarName, string& Value );
		void Set_Value32 ( string VarName, long Value );
		void Set_Value64 ( string VarName, long long Value );
		void Set_String ( string VarName, string Value );
		
		// constructor
		File ();
	};
};

#endif

