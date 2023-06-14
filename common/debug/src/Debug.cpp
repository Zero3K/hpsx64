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


#include "Debug.h"


// define this to output debug information using ofstream instead of platform API
#define USE_CFILE

// define this to ALSO output debug information into a combined file
//#define DEBUG_COMBINE

// define this to combine ALL debug information into one sequential file ONLY
// MUST also define DEBUG_COMBINE with this for it to work
//#define COMBINE_ONLY

// debug to standard output
//#define VERBOSE_DEBUG


using namespace Debug;


const char *Log::c_sCombinedFile = "CombinedMaster.txt";

int Log::iInstance = 0;
int Log::iInstanceC = 0;
ofstream Log::cOutputCombined;

FILE* Log::fOutputCombined;


Log::Log()
{
}

Log::~Log()
{
#ifdef USE_CFILE

	if ( bEnableSplit )
	{

//#ifndef COMBINE_ONLY
	if ( cOutput.is_open () )
	{
		cOutput.close ();
	}
//#endif

	}
	
	
	if ( !bDisableCombine )
	{
	
//#ifdef DEBUG_COMBINE
	if ( cOutputCombined.is_open () )
	{
		cOutputCombined.close ();
	}
//#endif

	}

#else
	OutputFile.Close ();
#endif
}

bool Log::Create ( const char* LogFileName )
{
	OutputFileName = LogFileName;

#ifdef USE_CFILE

	if ( bEnableSplit )
	{
	cOutput.open ( LogFileName );
	}
	
//#ifdef DEBUG_COMBINE
	if ( !iInstance )
	{
		cOutputCombined.open ( c_sCombinedFile );
	}
//#endif

	iInstance++;
#else
	//OutputFile.open ( LogFileName );
	OutputFile.Create ( LogFileName );
	//OutputFile.close ();
	ss.str("");
#endif


	return true;
}

// this function is the pure C version (C++ libraries not playing well with recompiler)
bool Log::CreateDebugFile (const char* LogFileName )
{
#ifdef VERBOSE_DEBUG
	printf ( "\nStart->Log::CreateDebugFile iInstanceC= %i", iInstanceC );
#endif

	if ( !iInstanceC )
	{
		fOutputCombined = fopen ( c_sCombinedFile, "w" );
		
		if ( !fOutputCombined )
		{
			printf ( "\nError: Log::CreateDebugFile: Unable to create combined log file: %s", c_sCombinedFile );
		}
	}
	
	iInstanceC++;
	
	if ( bEnableSplit )
	{
		fOutput = fopen ( LogFileName, "w" );
		
		if ( !fOutput )
		{
			printf ( "\nError: Log::CreateDebugFile: Unable to create log file: %s", LogFileName );
		}
	}
	
#ifdef VERBOSE_DEBUG
	printf ( "\nEnd->Log::CreateDebugFile iInstanceC= %i", iInstanceC );
#endif

	return true;
}


