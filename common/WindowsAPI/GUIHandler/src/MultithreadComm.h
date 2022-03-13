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


// quick inter-thread communication made simple

extern "C" long ThreadSafeExchange( long lValueToExchange, volatile long* WhereToExchangeValue );

// This buffer size must be a power of 2
#define MCBUFFERSIZE	32

#define LOCKED			1
#define UNLOCKED		0

template <typename T>
class MultithreadComm
{

	volatile long lReadIndex, lWriteIndex;
	volatile long lReadLock, lWriteLock;

	T tCircularBuffer [ MCBUFFERSIZE ];

	public:
	
		//constructor
		MultithreadComm ( void )
		{
			lReadIndex = 0;
			lWriteIndex = 0;
			lReadLock = 0;
			lWriteLock = 0;
		}

		// use this to check that data is ready to be read
		bool isDataAvailable ( void );
		
		// use this to make sure that there is enough space in circular buffer to write data
		bool isOkToWrite ( void );
		
		// this will make program wait if there is no data available to read
		// mutiple threads can read at the same time
		T Read ( void );

		// this will make program wait if there is no free space available in circular buffer to write
		// multiple threads can write at the same time
		void Write ( T tValue );
		
};

template <typename T>
bool MultithreadComm<T>::isDataAvailable ( void )
{
	return ( lReadIndex != lWriteIndex );
}


template <typename T>
bool MultithreadComm<T>::isOkToWrite ( void )
{
	return ( ( ( lWriteIndex + 1 ) & ( MCBUFFERSIZE - 1 ) ) != lReadIndex );
}


template <typename T>
T MultithreadComm<T>::Read ( void )
{
	T tData;
	long lIndex;
	
	// wait until read is not locked by another thread
	while ( ThreadSafeExchange ( LOCKED, & lReadLock ) == LOCKED );

	// wait until there is data available to read
	while ( lReadIndex == lWriteIndex );
	
	// read the data
	tData = tCircularBuffer [ lReadIndex ];
	
	// update read index IN A THREAD-SAFE WAY
	lIndex = ( lReadIndex + 1 ) & ( MCBUFFERSIZE - 1 );

	// THIS PART NEEDS TO BE THREAD-SAFE
	ThreadSafeExchange ( lIndex, & lReadIndex );
	
	// unlock read so other threads can read data
	ThreadSafeExchange ( UNLOCKED, & lReadLock );
	
	return tData;
}


template <typename T>
void MultithreadComm<T>::Write ( T tValue )
{
	long lIndex;

	// wait until write is not locked by another thread
	while ( ThreadSafeExchange ( LOCKED, & lWriteLock ) == LOCKED );

	// wait until there is data available to read
	while ( ( ( lWriteIndex + 1 ) % MCBUFFERSIZE ) == lReadIndex );
	
	// read the data
	tCircularBuffer [ lWriteIndex ] = tValue;
	
	// update write index IN A THREAD-SAFE WAY
	lIndex = ( lWriteIndex + 1 ) & ( MCBUFFERSIZE - 1 );

	// THIS PART NEEDS TO BE THREAD-SAFE
	ThreadSafeExchange ( lIndex, & lWriteIndex );

	// unlock write so other threads can read data
	ThreadSafeExchange ( UNLOCKED, & lWriteLock );
	
	return;

}

