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


// ICache object designed for R3000A object
// designed for potential accuracy, but not 100% accurate

#ifndef _R3000A_ICACHE_H_
#define _R3000A_ICACHE_H_

//#define INLINE_DEBUG

#include "types.h"

#include "Debug.h"


//#define USE_VALID_BITS


namespace R3000A
{

	class ICache_Device
	{
	
	public:

		// size of i-cache in bytes
		static const u32 ICache_Size = 4096;
		static const u32 ICache_Mask = ICache_Size - 1;
		
		// the number of cache lines
		static const u32 ICacheLineCount = ICache_Size / 16;

#ifdef USE_VALID_BITS
		// marks if a cache line is valid or not
		bool isValid [ ICacheLineCount ];
#endif

		// the data in ICache
		// has 1024 entries
		ALIGN16 u32 ICacheData [ ICache_Size / sizeof(u32) ];
		
		// the block address for the blocks of data in ICache
		// 16 bytes per cache line
		// 256 Cache Blocks, so this has 256 entries in the array
		ALIGN16 u32 ICacheBlockSource [ ICache_Size / 16 ];
		
		// constructor
		// invalidates all cache entries
		ICache_Device ( void ) { Reset (); }
		
		void Reset ()
		{
#ifdef USE_VALID_BITS
			memset ( this, 0, sizeof( ICache_Device ) );
#else
			memset ( this, -1, sizeof( ICache_Device ) );
#endif
		}

		
		// destructor
		//~ICache ( void );

		// checks if address is in a cached location
		inline static bool isCached ( u32 Address )
		{
			// only should check the first and third most significant bits of address to see if it is in a cached region
			return ( ( Address & 0xa0000000 ) != 0xa0000000 );
		}
		
		inline u32 GetCacheLine ( u32 Address )
		{
			return ( Address >> 4 ) & 0xff;
		}
		
		inline u32 GetCacheLineStart ( u32 Address )
		{
			return ( GetCacheLine ( Address ) << 2 );
		}
		
		// check if address results in cache miss
		// returns 0: cache hit; 1: cache miss
		inline u32 isCacheHit ( u32 Address )
		{
			// get the cache line that address should be at
			u32 ICacheBlockIndex = ( Address >> 4 ) & 0xff;
			
			// make sure the cache line is valid and that the address is actually cached there
			if ( ( ICacheBlockSource [ ICacheBlockIndex ] == ( Address & 0x1ffffff0 ) )
#ifdef USE_VALID_BITS
			&& ( isValid [ ICacheBlockIndex ] )
#endif
			)
			{
				return 1;
			}
			
			return 0;
		}
		
		// read address from cache
		// assumes that the cache line is valid - should always check if address is a cache hit first with isCacheHit
		inline u32 Read ( u32 Address )
		{
			return ICacheData [ ( Address >> 2 ) & 0x3ff ];
		}
		
		// load line into cache for block starting at address
		// Data points to a 4 element array of 32-bit values
		inline u32* GetCacheLinePtr ( u32 Address )
		{
			// get the index of start of cache line
			u32 ICacheDataStartIndex = ( ( Address & 0x1ffffff0 ) >> 2 ) & 0x3ff;

			// load data into cache
			return &(ICacheData [ ICacheDataStartIndex ]);
		}
		
		// gets a pointer to the address in cache
		inline u32* GetCachePtr ( u32 Address )
		{
			// get the index of start of cache line
			//u32 ICacheDataStartIndex = ( ( Address ) >> 2 ) & ICache_Mask;

			// get pointer into cache
			return &(ICacheData [ ( ( Address ) >> 2 ) & ICache_Mask ]);
		}
		
		inline void ValidateCacheLine ( u32 Address )
		{
			// get the cache line that address should be at
			u32 ICacheBlockIndex = ( Address >> 4 ) & 0xff;
			
			// set the source address for start of cache line
			ICacheBlockSource [ ICacheBlockIndex ] = Address & 0x1ffffff0;
			
#ifdef USE_VALID_BITS
			// mark cache line as valid
			isValid [ ICacheBlockIndex ] = true;
#endif
		}
		
		// invalidate address from cache if it is cached
		inline void Invalidate ( u32 Address )
		{
			if ( isCacheHit ( Address ) )
			{
#ifdef USE_VALID_BITS
				// invalidate cache line
				isValid [ ( Address >> 4 ) & 0xff ] = false;
#else
				ICacheBlockSource [ ( Address >> 4 ) & 0xff ] = -1;
#endif
			}
		}

		
		// invalidate address from cache if it is cached
		inline void InvalidateDirect ( u32 Address )
		{
			// address will be the address to the actual cache line, like 0x0,0x10,0x20,...,0xfe0,0xff0
			// get the cache line that address should be at
			
#ifdef USE_VALID_BITS
			// invalidate cache line
			isValid [ ( Address >> 4 ) & 0xff ] = false;
#else
			ICacheBlockSource [ ( Address >> 4 ) & 0xff ] = -1;
#endif
		}
		
		
	
	};
}

#endif

