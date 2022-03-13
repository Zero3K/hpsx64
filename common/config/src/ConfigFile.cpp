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


#include "ConfigFile.h"
#include "StringUtils.h"
#include <cstring>

using namespace Config;
using namespace std;
using namespace Utilities::Strings;



const char* File::c_sDelimiter = "\n";
const char* File::c_sAssigner = "=";
const char* File::c_sVarPrefix = "[";
const char* File::c_sVarPostfix = "]";


File::File ()
{
	//Clear ();
}


void File::Clear ()
{
	memset ( this, 0, sizeof( Config::File ) );
	//cout << "\nClearing: Config file size=" << strlen( cData );
	//cout << "\nClearing: size of Config::File=" << sizeof( Config::File );
}


bool File::Load ( string NameOfFile )
{
	int size, result;
	ifstream *fFile;
	//FILE* fFile;
	
	//fFile = fopen ( NameOfFile.c_str(), "rb" );
	
	//if ( !fFile ) return false;
	
	fFile = new ifstream( NameOfFile.c_str()/*, ios::binary*/ );
	//result = fread ( cData, 1, size /*c_iConfigFile_MaxSize*/, fFile );
	
	if ( !fFile->is_open() || fFile->fail() ) return false;
	
	//fseek( fFile, 0, SEEK_END );
	//size = ftell( fFile );
	//fseek( fFile, 0, SEEK_SET );
	fFile->seekg ( 0, std::ifstream::end );
	size = fFile->tellg ();
	fFile->seekg ( 0, std::ifstream::beg );
	
	
	if ( !fFile->is_open() || fFile->fail() ) return false;
	//if ( !result ) return false;
	
	fFile->read ( cData, size );
	result = fFile->gcount();
	
	// terminate string
	cData [ result ] = 0;
	
	// close the file when done
	fFile->close ();
	//fclose ( fFile );
	
	cout << "\nLoading: Config file size=" << strlen( cData );
	
	delete fFile;
	
	return true;
}



bool File::Save ( string NameOfFile )
{
	ofstream *fFile;
	fFile = new ofstream( NameOfFile.c_str(), ios::trunc /*ios::binary*/ );
	
	if ( !fFile->is_open() || fFile->fail() ) return false;
	
	//fFile->write ( sData.c_str(), sData.size() );
	fFile->write ( cData, strlen(cData) );
	
	// close the file when done writing to it
	fFile->close ();
	
	cout << "\nSaving: Config file size=" << strlen( cData );
	
	delete fFile;
	
	return true;
}



bool File::Get_Value32 ( string VarName, long& Value )
{
	int x0, x1;
	string sVar;
	
	VarName = c_sVarPrefix + VarName + c_sVarPostfix;
	
	// find where variable is in string
	x0 = InStr ( cData, VarName );
	
	// if it is not there, then return false
	if ( x0 == string::npos ) return false;
	
	// look for assignment operator
	x0++;
	x0 = InStr ( cData, c_sAssigner, x0 );
	
	// look for delimiter
	x0++;
	x1 = InStr ( cData, c_sDelimiter, x0 );
	
	// get the string
	sVar = Mid ( cData, x0, x1 - x0 );
	
	if ( !isNumeric ( sVar ) ) return false;
	
	// convert to number
	Value = CLng ( sVar );
	
	// successful
	return true;
}

bool File::Get_Value64 ( string VarName, long long& Value )
{
	int x0, x1;
	string sVar;
	
	VarName = c_sVarPrefix + VarName + c_sVarPostfix;
	
	// find where variable is in string
	x0 = InStr ( cData, VarName );
	
	// if it is not there, then return false
	if ( x0 == string::npos ) return false;
	
	// look for assignment operator
	x0++;
	x0 = InStr ( cData, c_sAssigner, x0 );
	
	// look for delimiter
	x0++;
	x1 = InStr ( cData, c_sDelimiter, x0 );
	
	// get the string
	sVar = Mid ( cData, x0, x1 - x0 );
	
	if ( !isNumeric ( sVar ) ) return false;
	
	// convert to number
	Value = CLngLng ( sVar );
	
	// successful
	return true;
}


bool File::Get_String ( string VarName, string& Value )
{
	int x0, x1;
	string sVar;
	
	VarName = c_sVarPrefix + VarName + c_sVarPostfix;
	
	// find where variable is in string
	x0 = InStr ( cData, VarName );
	
	// if it is not there, then return false
	if ( x0 == string::npos ) return false;
	
	// look for assignment operator
	x0++;
	x0 = InStr ( cData, c_sAssigner, x0 );
	
	// look for delimiter
	x0++;
	x1 = InStr ( cData, c_sDelimiter, x0 );
	
	// get the string
	sVar = Mid ( cData, x0, x1 - x0 );
	
	//if ( !isNumeric ( sVar ) ) return false;
	
	// convert to string
	Value = sVar;
	
	// successful
	return true;
}


void File::Set_Value32 ( string VarName, long Value )
{
	VarName = c_sVarPrefix + VarName + c_sVarPostfix + c_sAssigner + CStr ( Value ) + c_sDelimiter;
	strcat ( cData, VarName.c_str () );
}

void File::Set_Value64 ( string VarName, long long Value )
{
	VarName = c_sVarPrefix + VarName + c_sVarPostfix + c_sAssigner + CStr ( Value ) + c_sDelimiter;
	strcat ( cData, VarName.c_str () );
}

void File::Set_String ( string VarName, string Value )
{
	VarName = c_sVarPrefix + VarName + c_sVarPostfix + c_sAssigner + Value + c_sDelimiter;
	strcat ( cData, VarName.c_str () );
}



unsigned long PSXDiskUtility::GetLayer1Start ( char* Output, const char* PSXFileName, int DiskSectorSize )
{
	int size, result;
	ifstream *fFile;
	unsigned char* cData = new unsigned char [ DiskSectorSize ];

	unsigned long layer1start;
	
	fFile = new ifstream( PSXFileName, ios_base::in | ios_base::binary );
	
	if ( !fFile->is_open() || fFile->fail() )
	{
		cout << "\n***ERROR*** GetLayer1Start: Problem opening file: " << PSXFileName;
		
		delete cData;
		delete fFile;
		
		return false;
	}
	
	fFile->seekg ( 0, std::ifstream::end );
	size = fFile->tellg ();

	// seek to sector 16
	//fFile->seekg ( 0, std::ifstream::beg );
	fFile->seekg ( 16 * 2048, std::ifstream::beg );
	
	//do
	if ( !fFile->eof () )
	{
	
		fFile->read ( (char*) cData, DiskSectorSize );

		layer1start = ( ((unsigned long) cData [ 80 ]) ) + ( ((unsigned long) cData [ 81 ]) << 8 ) + ( ((unsigned long) cData [ 82 ]) << 16 ) + ( ((unsigned long) cData [ 83 ]) << 24 );

		cout << "found layer1 start at:" << dec << layer1start;


		
		
		// find string with data
		/*
		for ( int i = 0; ( i + 12 ) < DiskSectorSize; i++ )
		{
			// SLXX_YYY.ZZ;1
			// 0 -> S, 1 -> L, 4 -> _, 8 -> ., 11 -> ;, 12 -> 1
			if (  cData [ i + 4 ] == '_' && cData [ i + 8 ] == '.' && cData [ i + 11 ] == ';' )
			{
				cout << "\nDisk ID=" << cData [ i ] << cData [ i + 1 ] << cData [ i + 2 ] << cData [ i + 3 ] << cData [ i + 4 ] << cData [ i + 5 ] << cData [ i + 6 ] << cData [ i + 7 ] << cData [ i + 8 ] << cData [ i + 9 ] << cData [ i + 10 ] << cData [ i + 11 ] << cData [ i + 12 ];
				//cout << "\nIndex=" << dec << i;
				
				cout << "\nid";
				
				for ( int j = 0; j < c_iOutputStringSize; j++ )
				{
					Output [ j ] = cData [ j + i ];
				}
				
				cout << "\ncopied";
				
				Output [ c_iOutputStringSize ] = 0;
				
				cout << "\nzero";
				
				fFile->close ();
				
				cout << "\nclosed";
				
				// done
				delete cData;
				
				delete fFile;
				
				return true;
			}
		}
		*/
		
	} //while ( !fFile->eof () );
	
	fFile->close ();
	
	delete cData;
	
	delete fFile;
	
	return layer1start;
}

bool PSXDiskUtility::GetPSXIDString ( char* Output, const char* PSXFileName, int DiskSectorSize )
{
	static const int c_iOutputStringSize = 11;

	int size, result;
	ifstream *fFile;
	char* cData = new char [ DiskSectorSize ];
	
	fFile = new ifstream( PSXFileName, ios_base::in | ios_base::binary );
	
	if ( !fFile->is_open() || fFile->fail() )
	{
		cout << "\n***ERROR*** GetPSIDString: Problem opening file: " << PSXFileName;
		
		delete cData;
		delete fFile;
		
		return false;
	}
	
	fFile->seekg ( 0, std::ifstream::end );
	size = fFile->tellg ();
	fFile->seekg ( 0, std::ifstream::beg );
	
	do
	{
	
		fFile->read ( cData, DiskSectorSize );
		
		// find string with data
		for ( int i = 0; ( i + 12 ) < DiskSectorSize; i++ )
		{
			// SLXX_YYY.ZZ;1
			// 0 -> S, 1 -> L, 4 -> _, 8 -> ., 11 -> ;, 12 -> 1
			if ( /* cData [ i ] == 'S' && cData [ i + 1 ] == 'L' && */ cData [ i + 4 ] == '_' && cData [ i + 8 ] == '.' && cData [ i + 11 ] == ';' /* && cData [ i + 12 ] == '1' */ )
			{
				cout << "\nDisk ID=" << cData [ i ] << cData [ i + 1 ] << cData [ i + 2 ] << cData [ i + 3 ] << cData [ i + 4 ] << cData [ i + 5 ] << cData [ i + 6 ] << cData [ i + 7 ] << cData [ i + 8 ] << cData [ i + 9 ] << cData [ i + 10 ] << cData [ i + 11 ] << cData [ i + 12 ];
				//cout << "\nIndex=" << dec << i;
				
				cout << "\nid";
				
				for ( int j = 0; j < c_iOutputStringSize; j++ )
				{
					Output [ j ] = cData [ j + i ];
				}
				
				cout << "\ncopied";
				
				Output [ c_iOutputStringSize ] = 0;
				
				cout << "\nzero";
				
				fFile->close ();
				
				cout << "\nclosed";
				
				// done
				delete cData;
				
				delete fFile;
				
				return true;
			}
		}
		
	} while ( !fFile->eof () );
	
	fFile->close ();
	
	delete cData;
	
	delete fFile;
	
	return true;
}


// this function determines if a PS2 Game disk image is a CD or not (if not, then might be a DVD disk)
bool PSXDiskUtility::isDataCD ( const char* PSXFileName )
{
	static const int c_iOutputStringSize = 11;

	int size, result;
	ifstream *fFile;
	//char* cData = new char [ DiskSectorSize ];
	unsigned long pData [ 12 ];
	
	bool bRet = false;
	
	fFile = new ifstream( PSXFileName, ios_base::in | ios_base::binary );
	
	if ( !fFile->is_open() || fFile->fail() )
	{
		cout << "\n***ERROR*** PSXDiskUtility::isDataCD: Problem opening file: " << PSXFileName;
		
		delete fFile;
		
		return false;
	}
	
	
	// read first 12 bytes of game disk
	fFile->read ( (char*) pData, 12 );
	
	//cout << "isDataCD: pData[0]=" << hex << pData [ 0 ] << " pData[1]=" << hex << pData [ 1 ] << " pData[2]=" << hex << pData [ 2 ];
	
	// check if the data disk pattern is in the first 12 bytes of game disk
	if ( ( pData [ 0 ] == 0xffffff00 ) && ( pData [ 1 ] == 0xffffffff ) && ( pData [ 2 ] == 0x00ffffff ) )
	{
		// game disk is a Playstation data disk //
		
		bRet = true;
		//return true;
	}
	
	
	
	fFile->close ();
	
	//delete cData;
	
	delete fFile;
	
	//return true;
	return bRet;
}



