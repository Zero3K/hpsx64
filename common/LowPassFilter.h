
//#include <iostream>

//using namespace std;

#ifndef _LOWPASSFILTER_H_

#define _LOWPASSFILTER_H_

namespace Emulator
{
	namespace Audio
	{
	
		template <const int NumOfFilters, const int BufferSize_PowerOfTwo>
		class LowPassFilter
		{
			unsigned long long WriteIndex;
			
			// these are in 1.15.16 fixed point format
			long Filter [ NumOfFilters ];
			
			long CircularBuffer [ BufferSize_PowerOfTwo ];
			
			static const unsigned long c_iMask = BufferSize_PowerOfTwo - 1;
			
		public:
			
			void Reset ()
			{
				WriteIndex = 0;
				for ( int i = 0; i < BufferSize_PowerOfTwo; i++ ) CircularBuffer [ i ] = 0;
			}
			
			LowPassFilter ()
			{
				Reset ();
			}
			
			void SetFilter ( long* Coefficients )
			{
				for ( int i = 0; i < NumOfFilters; i++ ) Filter [ i ] = Coefficients [ i ];
			}
			
			long ApplyFilter ( long Sample )
			{
				long long i, j;
				long long Output = 0;
				
				CircularBuffer [ WriteIndex++ & c_iMask ] = Sample;
				
				for ( i = 0, j = WriteIndex - NumOfFilters; i < BufferSize_PowerOfTwo; i++, j++ )
				{
					Output += ( ((long long) CircularBuffer [ j & c_iMask ]) * ((long long) Filter [ i ]) );
				}
				
				Output >>= 16;
				
				return Output;
			}
		};
		
		
	}
}

#endif




