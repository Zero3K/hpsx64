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


#ifndef MULTITHREAD_H_
#define MULTITHREAD_H_



#include "GNUThreading_x64.h"


#include <windows.h>


namespace Api
{

	class Thread
	{
	public:
		HANDLE ThreadHandle;
		int ThreadId;

		volatile long ThreadStarted;

		// this is what the start function must look like for multi-threading in windows
		typedef int (*StartFunction) ( void* _Param );

		// this creates the thread object and starts the thread
		Thread ( StartFunction st, void* _Param, bool WaitForStart = true );
		~Thread ();

		//void Create ( StartFunction st, void* Param );

		// suspends thread for specified number of milliseconds
		void SleepThread ( int Milliseconds );

		// suspends execution of thread until you call "Resume"
		void Suspend ();

		// resume execution of a suspended thread
		void Resume ();

		// exits the thread
		void Exit ( int ExitCode );

		// attach input from another thread to this one
		// returns zero if failed
		// fails if the there is no input queue for the thread that is attaching to this one
		// thread gets an input queue when it calls any user or gdi functions
		int Attach ( int ThreadToAttach );

		// this waits for the thread to complete
		// return true if it does not complete, returns false when it completes
		int Join ( int TimeOut = INFINITE );

		// kill the thread - terminate it - dangerous function
		//void Kill ();

		LPVOID Param;
		StartFunction Start;
	private:
		static void _StartThread ( Thread* _Param );

	};

}



#endif /* MULTITHREAD_H_ */
