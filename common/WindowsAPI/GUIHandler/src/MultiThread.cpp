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



#include "MultiThread.h"
using namespace x64ThreadSafe::Utilities;

Api::Thread::Thread ( StartFunction st, void* _Param, bool WaitForStart )
{
	// thread has not started yet
	ThreadStarted = 0;

	// set the start function
	Start = st;

	// set the parameter to start function
	Param = (LPVOID) _Param;

	ThreadHandle = CreateThread ( NULL, NULL, (LPTHREAD_START_ROUTINE) _StartThread, (LPVOID) this, NULL, NULL );

	// we also need the id of the thread, which is different from the handle
	ThreadId = GetThreadId ( ThreadHandle );

	if ( WaitForStart )
	{
	// we need to wait until thread has started
	while ( ThreadStarted == 0 );
	}
}


Api::Thread::~Thread ()
{
	CloseHandle ( ThreadHandle );
}

void Api::Thread::_StartThread ( Thread* _Param )
{
	int Result;

	// we need to make sure this is a GUI thread for now
	//IsGUIThread ( true );
	IsGUIThread ( false );

	// signal that thread has started
	//ThreadSafeExchange ( 1, &_Param->ThreadStarted );
	Lock_Exchange32 ( (long&)_Param->ThreadStarted, 1 );

	Result = _Param->Start ( _Param->Param );
	
	// signal that thread is now stopping
	Lock_Exchange32 ( (long&)_Param->ThreadStarted, 0 );

	// set the return value from thread that finished
	_Param->Exit ( Result );
}

int Api::Thread::Join ( int TimeOut )
{
	return WaitForSingleObject( ThreadHandle, TimeOut );
}

void Api::Thread::SleepThread ( int Milliseconds )
{
	Sleep ( (DWORD) Milliseconds );
}

void Api::Thread::Suspend ()
{
	SuspendThread ( ThreadHandle );
}

void Api::Thread::Resume ()
{
	ResumeThread ( ThreadHandle );
}

void Api::Thread::Exit ( int ExitCode )
{
	ExitThread ( (DWORD) ExitCode );
}

int Api::Thread::Attach ( int ThreadIdToAttach )
{
	return AttachThreadInput ( ThreadIdToAttach, ThreadId, true );
}


// dangerous function
//void Api::Thread::Kill ()
//{
//}




