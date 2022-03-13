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


// DCache object designed for R5900 object
// designed for potential accuracy, but not 100% accurate

#ifndef _R5900_DCACHE_H_
#define _R5900_DCACHE_H_

//#define INLINE_DEBUG


#define ENABLE_SIMD_DCACHE_SSE2


// enables d-cache on R5900
//#define ENABLE_R5900_DCACHE


#ifdef ENABLE_R5900_DCACHE

#define ENABLE_DCACHE_DATA_READ
#define ENABLE_DCACHE_DATA_WRITE

#endif


#include <emmintrin.h>

#include "types.h"

#include "Debug.h"

namespace R5900
{

	class DCache_Device
	{
	
	public:

		// size of i-cache in bytes
		// ICache on R5900 is 16KB
		static constexpr u32 DCache_Size = 8192;
		
		// 64 bytes per way on R5900 cache
		static constexpr u32 DCache_Line_Size = 64;
		
		// the number of cache lines
		static constexpr u32 DCacheLineCount = DCache_Size / DCache_Line_Size;

		// marks if a cache line is valid or not
		u8 Valid [ DCacheLineCount ] __attribute__ ((aligned (32)));
		
		// flips when data is loaded
		u8 LRF [ DCacheLineCount ] __attribute__ ((aligned (32)));

		// marks if a cache line is dirty/modified or not
		u8 Dirty [ DCacheLineCount ] __attribute__ ((aligned (32)));

		// marks if a cache line is locked or not
		u8 Lock [ DCacheLineCount ] __attribute__ ((aligned (32)));
		
		// the block address for the blocks of data in DCache
		// 16 bytes per cache line
		// 256 Cache Blocks, so this has 256 entries in the array
		u32 PFN [ DCacheLineCount ] __attribute__ ((aligned (32)));
		
		// the data in ICache
		// has 1024 entries
		u32 Data [ DCache_Size / sizeof(u32) ] __attribute__ ((aligned (32)));
		
		// constructor
		// invalidates all cache entries
		DCache_Device ( void ) { Reset (); }
		
		void Reset ()
		{
			memset ( this, 0, sizeof( DCache_Device ) );
			memset ( PFN, -1, DCacheLineCount * sizeof( u32 ) );
		}

		
		// destructor
		//~DCache ( void );

		inline u8 Get_Dirty ( u32 ulIndex )
		{
			return Dirty [ ulIndex ];
		}
		
		inline u8 Get_Valid ( u32 ulIndex )
		{
			return Valid [ ulIndex ];
		}
		
		inline u8 Get_LRF ( u32 ulIndex )
		{
			return LRF [ ulIndex ];
		}
		
		inline u8 Get_Lock ( u32 ulIndex )
		{
			return Lock [ ulIndex ];
		}
		
		inline u32 Get_PFN ( u32 ulIndex )
		{
			return PFN [ ulIndex ];
		}
		
		inline u32* Get_CacheLinePtr ( u32 ulIndex )
		{
			return & Data [ ulIndex << 4 ];
		}

		inline void Set_Dirty ( u32 ulIndex )
		{
			Dirty [ ulIndex ] = 1;
		}
		
		// checks if address is in a cached location
		// returns: 0 - uncached; 1 - cached
		inline static bool isCached ( u32 Address )
		{
			// only should check the first and third most significant bits of address to see if it is in a cached region
			// *todo* for ps2, need to use some other logic to determine if region is cached or not
			//return ( ( Address & 0xa0000000 ) != 0xa0000000 );
			
			// 3 is uncached accelerated; 0,8,9 is cached; rest are uncached
			switch ( Address >> 28 )
			{
				case 0:
				case 8:
				case 9:
					return 1;
					break;
			}
			
			return 0;
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
		// for ps2, needs to return a pointer to the cache line!
		inline u32* isCacheHit ( u32 Address )
		{
			// get the cache line that address should be at
			u32 DCacheBlockIndex = ( Address >> 6 ) & 0x3f;
			
			// make room for the way
			DCacheBlockIndex <<= 1;
			
			// make sure the cache line is valid and that the address is actually cached there
			// for ps2, must check way0 and way1 both
			if ( PFN [ DCacheBlockIndex ] == ( Address & 0x1fffffc0 ) )
			{
				//return & Data [ DCacheBlockIndex << 4 ];
				return & Data [ ( DCacheBlockIndex << 4 ) ^ ( ( Address >> 2 ) & 0xf ) ];
			}
			
			if ( PFN [ DCacheBlockIndex ^ 1 ] == ( Address & 0x1fffffc0 ) )
			{
				//return & Data [ ( DCacheBlockIndex ^ 1 ) << 4 ];
				return & Data [ ( ( DCacheBlockIndex ^ 1 ) << 4 ) ^ ( ( Address >> 2 ) & 0xf ) ];
			}
			
			return NULL;
		}
		

		// same as "isCacheHit" except returns entire cache line
		inline u32* isCacheHit_Line ( u32 Address )
		{
			// get the cache line that address should be at
			u32 DCacheBlockIndex = ( Address >> 6 ) & 0x3f;
			
			// make room for the way
			DCacheBlockIndex <<= 1;
			
			// make sure the cache line is valid and that the address is actually cached there
			// for ps2, must check way0 and way1 both
			if ( PFN [ DCacheBlockIndex ] == ( Address & 0x1fffffc0 ) )
			{
				//return & Data [ DCacheBlockIndex << 4 ];
				return & Data [ ( DCacheBlockIndex << 4 ) ];
			}
			
			if ( PFN [ DCacheBlockIndex ^ 1 ] == ( Address & 0x1fffffc0 ) )
			{
				//return & Data [ ( DCacheBlockIndex ^ 1 ) << 4 ];
				return & Data [ ( ( DCacheBlockIndex ^ 1 ) << 4 ) ];
			}
			
			return NULL;
		}
		
		// returns index on cache hit
		inline u32 isCacheHit_Index ( u32 Address )
		{
			// get the cache line that address should be at
			u32 DCacheBlockIndex = ( Address >> 6 ) & 0x3f;
			
			// make room for the way
			DCacheBlockIndex <<= 1;
			
			// make sure the cache line is valid and that the address is actually cached there
			// for ps2, must check way0 and way1 both
			if ( ( PFN [ DCacheBlockIndex ] == ( Address & 0x1fffffc0 ) ) && Valid [ DCacheBlockIndex ] )
			{
				return DCacheBlockIndex;
			}
			
			if ( ( PFN [ DCacheBlockIndex ^ 1 ] == ( Address & 0x1fffffc0 ) ) && Valid [ DCacheBlockIndex ^ 1 ] )
			{
				return ( DCacheBlockIndex ^ 1 );
			}
			
			return -1;
		}
		
		// get the next index to be replaced on reload of cache-line
		inline u32 Get_NextIndex ( u32 Address )
		{
			u32 ulLRF;
			
			// get the cache line that address should be at
			u32 DCacheBlockIndex = ( Address >> 6 ) & 0x3f;
			
			// make room for the way
			DCacheBlockIndex <<= 1;
			
			// check that both ways are valid
			if ( ((u16*)Valid) [ DCacheBlockIndex >> 1 ] == 0x0101 )
			{
				// get lrf
				ulLRF = LRF [ DCacheBlockIndex ] ^ LRF [ DCacheBlockIndex ^ 1 ];
				
				// put correct way into index
				DCacheBlockIndex ^= ulLRF;
			}
			else
			{
				// store to the way that is invalid //
				if ( Valid [ DCacheBlockIndex ] )
				{
					// way0 is valid, so way1 must not be valid //
					DCacheBlockIndex ^= 1;
				}
			}
			
			return DCacheBlockIndex;
		}
		
		inline void ReloadCache ( u32 Address, u64* pMemData )
		{
			u32 ulLRF;
			
			u64* pCacheLine;
			
			// get the cache line that address should be at
			u32 DCacheBlockIndex = ( Address >> 6 ) & 0x3f;
			
			// make room for the way
			DCacheBlockIndex <<= 1;
			
			// check that both ways are valid
			if ( ((u16*)Valid) [ DCacheBlockIndex >> 1 ] == 0x0101 )
			{
				// get lrf
				ulLRF = LRF [ DCacheBlockIndex ] ^ LRF [ DCacheBlockIndex ^ 1 ];
				
				// put correct way into index
				DCacheBlockIndex ^= ulLRF;
			}
			else
			{
				// store to the way that is invalid //
				if ( Valid [ DCacheBlockIndex ] )
				{
					// way0 is valid, so way1 must not be valid //
					DCacheBlockIndex ^= 1;
				}
			}
			
			// get pointer to cache line
			pCacheLine = (u64*) ( & Data [ DCacheBlockIndex << 4 ] );

#ifdef ENABLE_DCACHE_DATA_READ

#ifdef ENABLE_SIMD_DCACHE_SSE2
			_mm_store_si128 ((__m128i*) ( pCacheLine + 0 ), _mm_load_si128 ((__m128i const*) ( pMemData + 0 )));
			_mm_store_si128 ((__m128i*) ( pCacheLine + 2 ), _mm_load_si128 ((__m128i const*) ( pMemData + 2 )));
			_mm_store_si128 ((__m128i*) ( pCacheLine + 4 ), _mm_load_si128 ((__m128i const*) ( pMemData + 4 )));
			_mm_store_si128 ((__m128i*) ( pCacheLine + 6 ), _mm_load_si128 ((__m128i const*) ( pMemData + 6 )));
#else
			for ( int i = 0; i < 8; i++ )
			{
				pCacheLine [ i ] = pMemData [ i ];
			}
#endif

#endif
			
			// set as valid
			Valid [ DCacheBlockIndex ] = 1;
			
			// set PFN
			PFN [ DCacheBlockIndex ] = Address & 0x1fffffc0;
			
			// toggle lrf
			LRF [ DCacheBlockIndex ] ^= 1;
			
			// clear dirty
			Dirty [ DCacheBlockIndex ] = 0;
			
			// clear lock
			Lock [ DCacheBlockIndex ] = 0;
		}



		inline void WriteBackCache ( u32 ulIndex, u64* pMemData )
		{
			u64* pCacheLine;
			
			// clear the dirty bit since it is being written back
			Dirty [ ulIndex ] = 0;
			
			// get pointer to cache line
			pCacheLine = (u64*) ( & Data [ ulIndex << 4 ] );

#ifdef ENABLE_DCACHE_DATA_WRITE

#ifdef ENABLE_SIMD_DCACHE_SSE2
			_mm_store_si128 ((__m128i*) ( pMemData + 0 ), _mm_load_si128 ((__m128i const*) ( pCacheLine + 0 )));
			_mm_store_si128 ((__m128i*) ( pMemData + 2 ), _mm_load_si128 ((__m128i const*) ( pCacheLine + 2 )));
			_mm_store_si128 ((__m128i*) ( pMemData + 4 ), _mm_load_si128 ((__m128i const*) ( pCacheLine + 4 )));
			_mm_store_si128 ((__m128i*) ( pMemData + 6 ), _mm_load_si128 ((__m128i const*) ( pCacheLine + 6 )));
#else
			for ( int i = 0; i < 8; i++ )
			{
				pMemData [ i ] = pCacheLine [ i ];
			}
#endif

#endif
			
		}


		// return index to cache line if valid - for write back
		inline u32 InvalidateIndex ( u32 Address )
		{
			u32 ulLRF;
			u32 ulIndex;
			
			ulLRF = Address & 1;
			ulIndex = ( ( Address >> 6 ) & 0x3f ) << 1;
			ulIndex ^= ulLRF;
			
			if ( Valid [ ulIndex ] )
			{
				Valid [ ulIndex ] = 0;
				PFN [ ulIndex ] = -1;
				
				Dirty [ ulIndex ] = 0;
				Lock [ ulIndex ] = 0;
				
				return ulIndex;
			}
			
			return -1;
		}
		
		
		// return index to cache line if valid - for write back
		inline u32 InvalidateHit ( u32 Address )
		{
			u32 ulIndex;
			
			// get the cache line that address should be at
			ulIndex = ( ( Address >> 6 ) & 0x3f ) << 1;
			
			Address &= 0x1fffffc0;
			
			// make sure the cache line is valid and that the address is actually cached there
			// for ps2, must check way0 and way1 both
			if ( PFN [ ulIndex ] == Address )
			{
				Valid [ ulIndex ] = 0;
				PFN [ ulIndex ] = -1;
				
				Dirty [ ulIndex ] = 0;
				Lock [ ulIndex ] = 0;
				
				return ulIndex;
			}
			
			// check the other way
			ulIndex ^= 1;
			
			if ( PFN [ ulIndex ] == Address )
			{
				Valid [ ulIndex ] = 0;
				PFN [ ulIndex ] = -1;
				
				Dirty [ ulIndex ] = 0;
				Lock [ ulIndex ] = 0;
				
				return ulIndex;
			}
			
			return -1;
		}
		
		
		/*
		// read address from cache
		// assumes that the cache line is valid - should always check if address is a cache hit first with isCacheHit
		inline u32 Read ( u32 Address )
		{
			return Data [ ( Address >> 2 ) & 0x3ff ];
		}
		
		// load line into cache for block starting at address
		// Data points to a 4 element array of 32-bit values
		inline u32* GetCacheLinePtr ( u32 Address )
		{
			// get the index of start of cache line
			u32 ICacheDataStartIndex = ( ( Address & 0x1ffffff0 ) >> 2 ) & 0x3ff;

			// load data into cache
			return &(Data [ ICacheDataStartIndex ]);
		}
		
		inline void ValidateCacheLine ( u32 Address )
		{
			// get the cache line that address should be at
			u32 DCacheBlockIndex = ( Address >> 4 ) & 0xff;
			
			// set the source address for start of cache line
			Data [ DCacheBlockIndex ] = Address & 0x1ffffff0;
			
			// mark cache line as valid
			Valid [ DCacheBlockIndex ] = true;
		}
		
		// invalidate address from cache if it is cached
		inline void Invalidate ( u32 Address )
		{
			if ( isCacheHit ( Address ) )
			{
				// invalidate cache line
				Valid [ ( Address >> 4 ) & 0xff ] = false;
			}
		}

		
		// invalidate address from cache if it is cached
		inline void InvalidateDirect ( u32 Address )
		{
			// address will be the address to the actual cache line, like 0x0,0x10,0x20,...,0xfe0,0xff0
			// get the cache line that address should be at
			
			// invalidate cache line
			Valid [ ( Address >> 4 ) & 0xff ] = false;
		}
		*/
		
		
	
	};
}

#endif

