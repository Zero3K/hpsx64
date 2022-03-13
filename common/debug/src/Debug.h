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


#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "WinFile.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>


//FILE * fopen ( const char * filename, const char * mode );
#include <stdio.h>

// define to force using c standard library instead of c++
#define USE_PURE_C_ONLY

#define USE_CFILE
//#define DEBUG_COMBINE
//#define COMBINE_ONLY


#define DEBUG_OUT(LogObj,Str,...) if ( LogObj.bEnableSplit ) { fprintf ( LogObj.fOutput, Str, __VA_ARGS__ ); } \
									if ( ! LogObj.bDisableCombine ) { fprintf ( LogObj.fOutputCombined, Str, __VA_ARGS__ ); }

using namespace std;

using namespace WinApi;

namespace Debug
{
	class Log
	{
	public:
		typedef std::ios_base& (*FunctionIOS) ( std::ios_base& );
		typedef ostream& (*FunctionOSTREAM) ( ostream& );
		//typedef smanip (*FunctionMANIP) ( int );
		
#ifdef USE_CFILE
		static int iInstance;
		static int iInstanceC;
		ofstream cOutput;
		static ofstream cOutputCombined;
#endif

		FILE* fOutput;
		static FILE* fOutputCombined;

		bool bDisableCombine;
		bool bEnableSplit;
		
		static const char *c_sCombinedFile;
	
		string OutputFileName;
		//ofstream OutputFile;
		stringstream ss;
		
		File OutputFile;
		
		// construtor
		Log();

		// destructor
		~Log();
		
		// set to true to combine debug info into a master file, set to false to disable combining into a master file
		inline void SetCombine ( bool bCombine ) { bDisableCombine = !bCombine; }
		inline void SetSplit ( bool bSplit ) { bEnableSplit = bSplit; }
		
		bool Create ( char* LogFileName );
		bool CreateDebugFile ( char* LogFileName );
		
	/*
		ostream& operator<< (ostream& out, char c );
		ostream& operator<< (ostream& out, signed char c );
		ostream& operator<< (ostream& out, unsigned char c );
		 
		ostream& operator<< (ostream& out, const char* s );
		ostream& operator<< (ostream& out, const signed char* s );
		ostream& operator<< (ostream& out, const unsigned char* s );
	*/
		
		//template<typename T>
		//Log& operator<< ( T s );
		template<typename T>
		Log& operator<< ( T s )
		{
#ifdef USE_CFILE

			if ( bEnableSplit )
			{
			
//#ifndef COMBINE_ONLY
			cOutput << s;
			cOutput.flush ();
//#endif

			}
			
			if ( !bDisableCombine )
			{
			
//#ifdef DEBUG_COMBINE
			cOutputCombined << s;
			cOutputCombined.flush ();
//#endif

			}

#else
			ss << s;
			OutputFile.WriteString ( ss.str().c_str () );
			ss.str ( "" );
#endif

			return *this;
		}

		/*
		Log& operator<< ( char s );
		Log& operator<< ( signed char s );
		Log& operator<< ( unsigned char s );
		Log& operator<< ( const char* s );
		Log& operator<< ( const signed char* s );
		Log& operator<< ( const unsigned char* s );
		Log& operator<< ( int s );
		Log& operator<< ( long s );
		Log& operator<< ( long long s );
		Log& operator<< ( unsigned long s );
		Log& operator<< ( unsigned long long s );
		Log& operator<< ( float s );
		Log& operator<< ( double s );
		Log& operator<< ( FunctionIOS s );
		Log& operator<< ( FunctionOSTREAM s );
		Log& operator<< ( _Setw s );
		*/
		
	};


}





#endif


