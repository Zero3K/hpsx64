
namespace GeneralUtilities
{

	// clamp from 0-ClampValue, signed 32-bit
	// note: ClampValue MUST be (2^x)-1
	inline static unsigned long SignedClamp32 ( long n, const long ClampValue )
	{
		long a = ClampValue;
		a -= n;
		a >>= 31;
		a |= n;
		n >>= 31;
		n = ~n;
		n &= a;
		return n & ClampValue;
	}

	// clamp from 0-ClampValue, signed 64-bit
	// note: ClampValue MUST be (2^x)-1
	inline static unsigned long long SignedClamp64 ( long long n, const long long ClampValue )
	{
		long long a = ClampValue;
		a -= n;
		a >>= 63;
		a |= n;
		n >>= 63;
		n = ~n;
		n &= a;
		return n & ClampValue;
	}


	// clamp from 0-ClampValue, unsigned 32-bit
	// note: ClampValue MUST be (2^x)-1
	inline static unsigned long UnSignedClamp32 ( unsigned long n, const unsigned long ClampValue )
	{
		long a = ClampValue;
		a -= n;
		a >>= 31;
		a |= n;
		return a & ClampValue;
	}


	// clamp from 0-ClampValue, unsigned 64-bit
	// note: ClampValue MUST be (2^x)-1
	inline static unsigned long long UnSignedClamp64 ( unsigned long long n, const unsigned long long ClampValue )
	{
		long long a = ClampValue;
		a -= n;
		a >>= 63;
		a |= n;
		return a & ClampValue;
	}

	
	// source: http://graphics.stanford.edu/~seander/bithacks.html
	// usage: to sign extend a signed 5 bit value for example use
	// int r = signextend<signed int,5>(x);  // sign extend 5 bit number x to r
	template <typename T, unsigned B>
	inline T signextend(const T x)
	{
	  struct {T x:B;} s;
	  return s.x = x;
	}


	inline static int PopulationCount32(unsigned long v)
	{
		v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
		v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
		return ((v + (v >> 4) & 0xF0F0F0F) * 0x01010101) >> 24; // count
	}
	
	inline static int PopulationCount64(unsigned long long w)
	{
		w -= (w >> 1) & 0x5555555555555555ULL;
		w = (w & 0x3333333333333333ULL) + ((w >> 2) & 0x3333333333333333ULL);
		w = (w + (w >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
		return int((w * 0x0101010101010101ULL) >> 56);
	}
	
	
	// derived from: http://graphics.stanford.edu/~seander/bithacks.html
	/*
	inline static int CountTrailingZeros32_2 ( unsigned long v )
	{
		//unsigned int v;            // find the number of trailing zeros in v
		long r, t;                     // the result goes here
		float f = (float)(v & -v); // cast the least significant bit in v to a float
		r = (*(unsigned long *)&f >> 23) - 0x7f;
		t = r >> 31;
		r = ( ( t | r ) & 0x1f ) + ( t & 1 );	// make sure it returns 32 instead of -127 when v=0
		return r;
	}
	*/

	
	// source: http://www.hackersdelight.org/hdcodetxt/nlz.c.txt
	inline static int CountLeadingZeros32 (unsigned long k)
	{
		union {
		  unsigned long asInt;
		  float asFloat;
		};

		k = k & ~(k >> 1);           // Fix problem with rounding.
		asFloat = (float)k + 0.5f;
		return 158 - (asInt >> 23);
	}
	
	// note: this only works for little endian machines
	inline static int CountLeadingZeros64 (unsigned long long k)
	{
		union {
		  unsigned long long asInt;
		  double asDouble;
		};

		k &= ~(k >> 1);
		asDouble = (double)k + 0.5;
		return 1086 - (asInt >> 52);
	}

	
	inline static int CountTrailingZeros32 ( unsigned long v )
	{
		union {
		  unsigned long asInt;
		  float asFloat;
		};

		v = ~v & ( v - 1 );

		v = v & ~(v >> 1);           // Fix problem with rounding.
		asFloat = (float)v + 0.5f;
		return (asInt >> 23) - 126;
	}

	// note: this only works on little endian machines
	inline static int CountTrailingZeros64 ( unsigned long long v )
	{
		union {
		  unsigned long long asInt;
		  double asDouble;
		};

		v = ~v & ( v - 1 );

		v &= ~(v >> 1);
		asDouble = (double)v + 0.5;
		return (asInt >> 52) - 1022;
	}
	
	// use this one for a quick trailing zero count for 16-bit value?
	inline static int CountTrailingZeros16 ( unsigned long v )
	{
		union {
		  unsigned long asInt;
		  float asFloat;
		};
		
		asFloat = (float)v;
		return (asInt >> 23) - 127;
	}

}

