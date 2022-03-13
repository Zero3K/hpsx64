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


#ifndef _WINFILE_H_
#define _WINFILE_H_

#include <string>
#include <windows.h>

using namespace std;

namespace WinApi
{

	class File
	{
		HANDLE hFile;
		
	public:

		// need this for asynchronous reading
		OVERLAPPED Overlapped;
		
		// need this for a 64-bit seek
		unsigned long DistanceHigh;
	
		// pass "OPEN_EXISTING" to the Creation flag to open an existing file
		// pass "CREATE_ALWAYS" to create a new file
		// for asynchronous reading use CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
		// and for callback use static VOID CALLBACK ReadFileBuffer::StaticCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped); 
		bool Create ( string fileName, unsigned long Creation = CREATE_ALWAYS, unsigned long Access = GENERIC_WRITE, unsigned long Attributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH );
		bool CreateSync ( string fileName, unsigned long Creation = OPEN_EXISTING, unsigned long Access = GENERIC_READ, unsigned long Attributes = FILE_ATTRIBUTE_NORMAL );
		bool CreateAsync ( string fileName, unsigned long Creation = OPEN_EXISTING, unsigned long Access = GENERIC_READ, unsigned long Attributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED );
		
		unsigned long WriteString ( string Data );
		
		unsigned long Read ( char* DataOut, unsigned long BytesToRead );
		bool ReadSync ( char* DataOut, unsigned long BytesToRead, unsigned long long SeekPosition );
		bool ReadAsync ( char* DataOut, unsigned long BytesToRead, unsigned long long SeekPosition, void* Callback_Function );
		
		bool WaitAsync ();
		
		bool Seek ( unsigned long long offset );
		
		bool Close ();
		
		long long Size ();
		
	};
	
}

#endif
