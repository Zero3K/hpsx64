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


#include "WinFile.h"

#include <iostream>
using namespace std;
using namespace WinApi;

bool File::Create ( string fileName, unsigned long Creation, unsigned long Access, unsigned long Attributes )
{
	hFile = CreateFile ( fileName.c_str (), Access, NULL, NULL, Creation, Attributes, NULL );
	if ( hFile == INVALID_HANDLE_VALUE ) return false;
	return true;
}


bool File::CreateSync ( string fileName, unsigned long Creation, unsigned long Access, unsigned long Attributes )
{
	hFile = CreateFile ( fileName.c_str (), Access, NULL, NULL, Creation, Attributes, NULL );
	if ( hFile == INVALID_HANDLE_VALUE ) return false;
	return true;
}


bool File::CreateAsync ( string fileName, unsigned long Creation, unsigned long Access, unsigned long Attributes )
{
	hFile = CreateFile ( fileName.c_str (), Access, NULL, NULL, Creation, Attributes, NULL );
	if ( hFile == INVALID_HANDLE_VALUE ) return false;
	return true;
}


unsigned long File::WriteString ( string Data )
{
	unsigned long Count;
	WriteFile ( hFile, Data.c_str(), Data.size(), &Count, NULL );
	return Count;
}

unsigned long File::Read ( char* DataOut, unsigned long BytesToRead )
{
	unsigned long Count;
	ReadFile ( hFile, DataOut, BytesToRead, &Count, NULL );
	return Count;
}


bool File::ReadSync ( char* DataOut, unsigned long BytesToRead, unsigned long long SeekPosition )
{
	unsigned long Count;
	Seek ( SeekPosition );
	ReadFile ( hFile, DataOut, BytesToRead, &Count, NULL );
	return Count;
}


//typedef VOID (WINAPI *LPOVERLAPPED_COMPLETION_ROUTINE)(
//    _In_     DWORD dwErrorCode,
//    _In_     DWORD dwNumberOfBytesTransfered,
//    _Inout_  LPOVERLAPPED lpOverlapped
//);


bool File::ReadAsync ( char* DataOut, unsigned long BytesToRead, unsigned long long SeekPosition, void* Callback_Function )
{
	bool Ret;
	
	Overlapped.Offset = (DWORD) SeekPosition;
	Overlapped.OffsetHigh = (DWORD) ( SeekPosition >> 32 );
	
	// set the hEvent to NULL so that it signals the file handle when done
	Overlapped.hEvent = NULL;
	
	//cout << "\nlpOverlapped set successfully";
	
	Ret = ReadFileEx ( hFile, DataOut, BytesToRead, (LPOVERLAPPED) &Overlapped, (LPOVERLAPPED_COMPLETION_ROUTINE) Callback_Function );

	// notify console of any errors
	if ( !Ret )
	{
		cout << "\nFile::ReadAsync Error; Return value zero. ReadFileEx returned an error.\n";
	}
	
	//cout << "\nReadFileEx called successfully";
	
	return Ret;
}

// returns true if operation completed successfully
bool File::WaitAsync()
{
	//unsigned long ulBytesRead;
	//return GetOverlappedResultEx( hFile, &Overlapped, (LPDWORD) &ulBytesRead, 1, true );
	WaitForSingleObjectEx( hFile, INFINITE, true );
	return true;
}

bool File::Seek ( unsigned long long offset )
{
	unsigned long DistanceLow;
	unsigned long long CompareDistance;

	//including offset high
	DistanceHigh = (LONG) ( offset >> 32 );
	
	// set the movemethod to NULL which should be the same as "FILE_BEGIN"
	//if ( SetFilePointer( hFile, (unsigned long) offset, (PLONG) &DistanceHigh, NULL ) != offset ) return false;
	DistanceLow = SetFilePointer( hFile, (unsigned long) offset, (PLONG) &DistanceHigh, NULL );
	
	CompareDistance = ( ((unsigned long long)DistanceHigh) << 32 ) | ((unsigned long long)DistanceLow);
	
	if ( CompareDistance != offset ) return false;
	
	return true;
}


bool File::Close ()
{
	BOOL Result;
	Result = CloseHandle ( hFile );
	if ( !Result ) return false;
	return true;
}


long long File::Size ()
{
	long long SizeOfFile;
	int ret;
	
	ret = GetFileSizeEx( hFile, (PLARGE_INTEGER) &SizeOfFile );
	
	return SizeOfFile;
}


