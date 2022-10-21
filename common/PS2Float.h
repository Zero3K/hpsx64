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


#include "types.h"
#include <math.h>

#include <iostream>
using namespace std;


#ifndef PSFLOAT_H

#define PSFLOAT_H






#define ENABLE_FLAG_CHECK
//#define USE_DOUBLE
//#define CALC_WITH_INF


// use double-precision to implement PS2 floating point
//#define USE_DOUBLE_MATH_ADD
//#define USE_DOUBLE_MATH_SUB
//#define USE_DOUBLE_MATH_MUL
//#define USE_DOUBLE_MATH_MADD
//#define USE_DOUBLE_MATH_MSUB
//#define USE_DOUBLE_MATH_DIV
//#define USE_DOUBLE_MATH_SQRT
//#define USE_DOUBLE_MATH_RSQRT



// use integer math when more simplistic
//#define USE_INTEGER_MATH
#define USE_INTEGER_MATH_ADD
#define USE_INTEGER_MATH_SUB
#define USE_INTEGER_MATH_MUL
#define USE_INTEGER_MATH_MADD
#define USE_INTEGER_MATH_MSUB


// this is for testing negation of the msub multiply result
#define NEGATE_MSUB_MULTIPLY_RESULT


#define ENABLE_FLAG_CHECK_MADD


//#define IGNORE_DENORMAL_MAXMIN
#define COMPUTE_MINMAX_AS_INTEGER


// chops off last bit before multiply of one of the multiply operands
//#define ENABLE_MUL_PRECISION_CHOP


// should signed zero set the signed flag too, or just the zero flag ??
#define DISABLE_SIGN_FLAG_WHEN_ZERO


// on multiply underflow store the accumulator or previous value for MADD/MSUB ??
//#define ENABLE_PREVIOUS_VALUE_ON_MULTIPY_UNDERFLOW


// output of these defines should be LONG LONG
#define CvtLongLongToDouble(ll) ( ( (ll) & 0x7f800000 ) ? ( ( ( (( ( (ll) >> 23 ) & 0xff ) + 896) ) << 52 ) | ( ( (ll) & 0x7fffff ) << 29 ) | ( ( (ll) & 0x80000000ULL ) << 32 ) /* | ( 0x1fffffffULL ) */ ) : ( ( (ll) & 0x80000000 ) << 32 ) )
#define CvtPS2FloatToDouble(f) ( CvtLongLongToDouble( ((long long&)f) ) )
#define CvtLongToDouble(l) ( CvtLongLongToDouble( ((long long)l) ) )

// output of these defines should be LONG
//#define CvtLongLongToPS2Float(ll) ( ( ll & 0x7fffffffffffffffLL ) ? ( ( ( ( ( (ll) >> 52 ) & 0x7ff ) - 896 ) << 23 ) | ( ( (ll) >> 29 ) & 0x7fffff ) | ( ( (ll) >> 32 ) & 0x80000000 ) ) : ( ( (ll) >> 32 ) & 0x80000000 ) )
#define CvtLongLongToPS2Float(ll) ( ( ll & 0x7ff0000000000000ULL ) ? ( ( ( ( ( (ll) >> 52 ) & 0x7ff ) - 896 ) << 23 ) | ( ( (ll) >> 29 ) & 0x7fffff ) | ( ( (ll) >> 32 ) & 0x80000000 ) ) : ( ( (ll) >> 32 ) & 0x80000000 ) )
#define CvtDoubleToPS2Float(d) ( CvtLongLongToPS2Float( ((unsigned long long&)d) ) )

// these are absolute max values and represent a PS2 overflow
// so if result is greater OR equal to these, then you have an overflow
#define PosMaxLongLongPS2 ((896LL+256LL)<<52)
#define NegMaxLongLongPS2 ( ((896LL+256LL)<<52) | ( 1LL << 63 ) )

// these are absolute min values and represent zero before calculation and underflow after calculation
// so if result is strictly below these (but NOT equal to), then you have an underflow
#define PosMinLongLongPS2 ((896LL+1LL)<<52)
#define NegMinLongLongPS2 ( ((896LL+1LL)<<52) | ( 1LL << 63 ) )


namespace PS2Float
{
	//static const long long c_llPS2DoubleMax = ( 1151ULL << 52 );
	static const long long c_llPS2DoubleMax = ( 1152ULL << 52 );
	static const long long c_llPS2DoubleMin = ( 897ULL << 52 );
	//static const long long c_llPS2DoubleMin = ( 1ULL << 63 ) | ( 1151ULL << 52 );
	static const long long c_llDoubleINF = ( 0x7ffULL << 52 );
	static const long long c_llDoubleAbsMask = -( 1LL << 63 );
	static const long long c_llDoubleSignMask = -( 1LL << 63 );
	

	static const long c_lFloatINF = ( 0xff << 23 );
	static const long c_lFloat_SignExpMask = ( 0x1ff << 23 );
	static const long c_lFloat_SignMask = 0x80000000;
	static const long c_lFloat_ExpMask = ( 0xff << 23 );
	static const long c_lFloat_MantissaMask = 0x7fffff;
	
	static const long c_lFloat_ValidMax = 0x7f7fffff;
	
	// difference between max ps2 float and next value down
	static const long c_lFloatMaxDiff = 0x73800000;
	
	
	// maximum positive 64-bit integer
	static const long long c_llMaxPosInt64 = 0x7fffffffffffffffLL;
	
	// maximum positive 32-bit integer
	static const long c_lMaxPosInt32 = 0x7fffffff;
	
	
	
	static const long long c_llPS2DoubleMaxVal = CvtLongToDouble ( c_lMaxPosInt32 );
	
	
	
	// these 3 functions are from: http://cottonvibes.blogspot.com/2010/07/testing-for-nan-and-infinity-values.html
	//inline bool isNaN(float x) { return x != x; }
	//inline bool isInf(float x) { return fabs(x) == numeric_limits<float>::infinity(); }
	static inline bool isNaNorInf(float x) { return ((long&)x & 0x7fffffff) >= 0x7f800000; }
	//inline bool isNaN_d (double x) { return x != x; }
	//inline bool isInf_d (double x) { return fabs(x) == numeric_limits<float>::infinity(); }
	static inline bool isNaNorInf_d (double x) { return ((long long&)x & 0x7fffffffffffffffULL) >= 0x7ff0000000000000ULL; }
	
	
	inline static void FlushPS2DoubleToZero ( double& d )
	{
		//static const long long c_ullMinPS2Value = PosMinLongLongPS2;

#ifdef USE_INTEGER_MATH
		long long ll;
		
		ll = (long long&) d;
		
		// if less than the min value, then flush to zero
		if ( ( ll & c_llMaxPosInt64 ) < c_llPS2DoubleMin ) ll &= c_llDoubleSignMask;
		
		d = (double&) ll;
#else
		// if less than the min value, then flush to zero
		if ( d < ( (double&) c_llPS2DoubleMin ) ) d = 0.0L;
#endif
	}
	
	
	// forces double to be within the range ps2 max and min
	inline static void PS2DblMaxMin ( double& d )
	{
		long long ll;
		
		ll = (long long&) d;
		
		// if less than the min value, then flush to zero
		if ( ( ll & c_llMaxPosInt64 ) < c_llPS2DoubleMin ) ll &= c_llDoubleSignMask;
		
		// greater than max value, then set to +/-max
		else if ( ( ll & c_llMaxPosInt64 ) >= c_llPS2DoubleMax ) ll = ( ll & c_llDoubleSignMask ) | c_llPS2DoubleMaxVal;
		
		d = (double&) ll;
	}


	inline static void SetFlagsOnResultDF_d ( DoubleLong& dResult, DoubleLong dlACC, DoubleLong dProd, DoubleLong dSum, double fd, int index, unsigned short* StatusFlag, unsigned short* MACFlag )
	{
		// set STICKY sign flag based on multiplication
		*StatusFlag |= ( dProd.l >> 63 ) & 0x80;
		
		// set ALL sign flag based on ADD
		*StatusFlag |= ( dSum.l >> 63 ) & 0x82;
		*MACFlag |= ( dSum.l >> 63 ) & ( 1 << ( index + 4 ) );
		
		// set STICKY zero based on MUL
		if ( ( dProd.l & c_llMaxPosInt64 ) < c_llPS2DoubleMin )
		{
			// set zero sticky flag based on multiply result
			*StatusFlag |= 0x040;
			
			// set STICKY underflow based on MUL
			if ( dProd.l & c_llMaxPosInt64 )
			{
				// *note* ONLY set underflow sticky flag on underflow
				*StatusFlag |= 0x100;
				
#ifdef ENABLE_PREVIOUS_VALUE_ON_MULTIPY_UNDERFLOW
				// set preliminary result to previous value ?? (or to ACC ??)
				dResult.d = fd;
#else
				// set preliminary result to ACC ??
				dResult.d = dlACC.d;
#endif
			}
		}
		
		// set ALL zero based on ADD
		if ( ( dSum.l & c_llMaxPosInt64 ) < c_llPS2DoubleMin )
		{
			// set all zero flags
			*StatusFlag |= 0x041;
			*MACFlag |= ( 1 << ( index + 0 ) );
		}

		// set ALL overflow based on MUL
		if ( ( dProd.l & c_llMaxPosInt64 ) >= c_llPS2DoubleMax )
		{
			// overflow in MADD //
			*StatusFlag |= 0x208;
			*MACFlag |= ( 1 << ( index + 12 ) );
			
			// set result to +/-max (must use sign of addition result)
			// actually, should be sign of multiplication (MSUB has sign change)
			dResult.l = ( dProd.l & c_llDoubleSignMask ) | c_llPS2DoubleMaxVal;
		}
		
		// otherwise check if ACC already in overflow state
		//else if ( ( dlACC.l & c_llMaxPosInt64 ) >= CvtLongToDouble ( c_lMaxPosInt32 ) )
		else if ( ( dlACC.l & c_llMaxPosInt64 ) >= CvtLongToDouble ( c_lFloat_ExpMask ) )
		{
			// overflow in MADD //
			*StatusFlag |= 0x208;
			*MACFlag |= ( 1 << ( index + 12 ) );
			
			// set result to +/-max (here just return ACC)
			//dResult.l = dlACC.l;
			dResult.l = ( dlACC.l & c_llDoubleSignMask ) | c_llPS2DoubleMaxVal;
		}
		
		// also overflow if the add has overflow
		else if ( ( dSum.l & c_llMaxPosInt64 ) >= c_llPS2DoubleMax )
		{
			// overflow in MADD //
			*StatusFlag |= 0x208;
			*MACFlag |= ( 1 << ( index + 12 ) );
			
			// set result to +/-max (must use sign of addition result)
			dResult.l = ( dSum.l & c_llDoubleSignMask ) | c_llPS2DoubleMaxVal;
		}
		
	}
	
	
	inline static void SetFlagsOnResult_d ( double& dResult, int index, unsigned short* StatusFlag, unsigned short* MACFlag )
	{
		long long llResult;
		
		llResult = (long long&) dResult;
		
		// set zero flags
		//if ( ! ( llResult & c_llMaxPosInt64 ) )
		if ( ( llResult & c_llMaxPosInt64 ) < c_llPS2DoubleMin )
		{
			// set zero flags
			*StatusFlag |= 0x41;
			*MACFlag |= ( 1 << index );
		}
#ifdef DISABLE_SIGN_FLAG_WHEN_ZERO
		else
#endif
		// set sign flags
		// note: only set this strictly when calculation result is negative, not on signed zero
		//if ( Result < 0.0f )
		if ( llResult < 0 )
		{

			// set sign flags
			*StatusFlag |= 0x82;
			*MACFlag |= ( 1 << ( index + 4 ) );
		}


		// check for overflow
		if ( ( llResult & c_llMaxPosInt64 ) >= c_llPS2DoubleMax )
		{
			// overflow //
			*StatusFlag |= 0x208;
			*MACFlag |= ( 1 << ( index + 12 ) );
			
			// set to +/-max
			//lResult = c_lFloat_MantissaMask | lResult;
			llResult = ( llResult & c_llDoubleSignMask ) | CvtLongLongToDouble( 0x7fffffffLL );
			dResult = (double&) llResult;
		}
		
		// check for underflow
		// smallest float value is 2^-126 = 0x00800000
		// value as a double is 1023-126 = 897
		if ( ( ( llResult & c_llMaxPosInt64 ) < c_llPS2DoubleMin ) && ( llResult & c_llMaxPosInt64 ) )
		{
			// underflow //
			*StatusFlag |= 0x104;
			*MACFlag |= ( 1 << ( index + 8 ) );
			
			// set to +/-0
			//lResult = c_lFloat_SignMask & lResult;
			llResult &= c_llDoubleSignMask;
			dResult = (double&) llResult;
			
			// since value has been set to zero, set the zero flag
			// set zero flags
			*StatusFlag |= 0x41;
			*MACFlag |= ( 1 << index );
		}


		
		/*
		//----------------------
		// set sign flags
		if ( Dd < 0.0L )
		{
			// set sign flags
			*StatusFlag |= 0x82;
			*MACFlag |= ( 1 << ( index + 4 ) );
		}
		

		// check for underflow
		// smallest float value is 2^-126 = 0x00800000
		// value as a double is 1023-126 = 897
		if ( ( ( (long long&) Dd ) & ~c_llDoubleAbsMask ) <= c_llPS2DoubleMin && ( ( (long long&) Dd ) & ~c_llDoubleAbsMask ) )
		{
			// underflow //
			*StatusFlag |= 0x104;
			*MACFlag |= ( 1 << ( index + 8 ) );
		}
		
		// check for overflow
		if ( ( ( (long long&) Dd ) & ~c_llDoubleAbsMask ) >= c_llPS2DoubleMax )
		{
			// overflow //
			*StatusFlag |= 0x208;
			*MACFlag |= ( 1 << ( index + 12 ) );
		}
		
		// set zero flags
		if ( Dd == 0.0L )
		{
			// set zero flags
			*StatusFlag |= 0x41;
			*MACFlag |= ( 1 << index );
		}
		*/
	}
	
	
	inline static void SetFlagsOnResult_f ( float& Result, int index, unsigned short* StatusFlag, unsigned short* MACFlag )
	{
		long lResult;
		
		lResult = (long&) Result;
		
		// set zero flags
		//if ( Result == 0.0f )
		if ( ! ( lResult & 0x7fffffff ) )
		{
			// set zero flags
			*StatusFlag |= 0x41;
			*MACFlag |= ( 1 << index );
		}
#ifdef DISABLE_SIGN_FLAG_WHEN_ZERO
		else
#endif
		// set sign flags
		// note: only set this strictly when calculation result is negative, not on signed zero
		//if ( Result < 0.0f )
		if ( lResult < 0 )
		{

			// set sign flags
			*StatusFlag |= 0x82;
			*MACFlag |= ( 1 << ( index + 4 ) );
		}
		
		
		// check for overflow
		if ( ( lResult & c_lFloat_ExpMask ) == c_lFloat_ExpMask )
		{
			// overflow //
			*StatusFlag |= 0x208;
			*MACFlag |= ( 1 << ( index + 12 ) );
			
			// set to +/-max
			lResult = c_lFloat_MantissaMask | lResult;
		}
		
		// check for underflow
		// smallest float value is 2^-126 = 0x00800000
		// value as a double is 1023-126 = 897
		if ( ( lResult & c_lFloat_MantissaMask ) && !( lResult & c_lFloat_ExpMask ) )
		{
			// underflow //
			*StatusFlag |= 0x104;
			*MACFlag |= ( 1 << ( index + 8 ) );
			
			// set to +/-0
			lResult = c_lFloat_SignMask & lResult;
			
			// since value has been set to zero, set the zero flag
			// set zero flags
			*StatusFlag |= 0x41;
			*MACFlag |= ( 1 << index );
		}
		
		Result = (float&) lResult;
	}



	//inline static void FlushDenormal_d ( double& d1 )
	//{
	//}

	inline static void FlushDenormal_f ( float& f1 )
	{
		long lValue;
		
		lValue = (long&) f1;
		
		// check if exponent is zero
		if ( ! ( lValue & c_lFloat_ExpMask ) )
		{
			// exponent is zero, so flush value to zero //
			
			lValue &= c_lFloat_SignMask;
			
			f1 = (float&) lValue;
		}
	}

	
	inline static void FlushDenormal2_f ( float& f1, float& f2 )
	{
		FlushDenormal_f ( f1 );
		FlushDenormal_f ( f2 );
	}

	
	// flush denormals to zero and convert negative float to comparable negative integer
	inline static long FlushConvertToComparableInt_f ( float& f1 )
	{
		long lf1;
		
		lf1 = (long&) f1;
		
		// if exponent is zero, then set it to zero
		if ( ! ( lf1 & c_lFloat_ExpMask ) )
		{
			lf1 = 0;
		}
		
		// if negative, then take two's complement of positive value
		if ( lf1 < 0 )
		{
			lf1 = -( lf1 & 0x7fffffff );
		}
		
		return lf1;
	}
	
	
	inline static void ClampValue_d ( double& d )
	{
		long long ll;
		if ( isNaNorInf_d ( d ) )
		{
			ll = ( c_llPS2DoubleMax | ( c_llDoubleAbsMask & ((long long&) d) ) );
			d = (double&) ll;
		}
	}
	
	inline static void ClampValue_f ( float& f )
	{
		long l;
		
		// ps2 treats denormals as zero
		FlushDenormal_f ( f );
		
		// check for not a number and infinity
		if ( isNaNorInf ( f ) )
		{
#ifdef CALC_WITH_INF
			//Ds.l = c_llPS2DoubleMax | ( Ds.l & c_llDoubleAbsMask );
			//fs = (float&) ( c_lFloat_SignExpMask & ( c_lFloatINF | (long&) fs ) );
			// or could just clear mantissa, or set to next valid value
			l = ( c_lFloat_SignExpMask & ( (long&) f ) );
#else
			// or set to next valid value
			l = ( c_lFloat_ValidMax | ( c_lFloat_SignMask & ( (long&) f ) ) );
#endif

			f = (float&) l;
		}
		
	}


	inline static void ClampValue2_d ( double& d1, double& d2 )
	{
		ClampValue_d ( d1 );
		ClampValue_d ( d2 );
	}

	inline static void ClampValue2_f ( float& f1, float& f2 )
	{
		ClampValue_f ( f1 );
		ClampValue_f ( f2 );
		
		// should also flush denormals to zero
		// this is handle by clamp function now
		//FlushDenormal2_f ( f1, f2 );
	}
	
	
	inline static double CvtPS2FloatToDbl ( float f1 )
	{
		DoubleLong d;
		d.l = CvtPS2FloatToDouble(f1);
		return d.d;
	}

	inline static float CvtDblToPS2Float ( double d1 )
	{
		FloatLong f;
		f.l = CvtDoubleToPS2Float(d1);
		return f.f;
	}

	
	// add float as integer
	inline static long addfloat(long fs, long ft, int index, unsigned short* StatusFlag, unsigned short* MACFlag)
	{
		static const long c_iHiddenBits = 1;
		long es, et, ed;
		long ss, st, sd;

		long temp;

		long ms, mt, md;

		long ext;

		long sign;

		long fd;


		// get the magnitude
		ms = fs & 0x7fffffff;
		mt = ft & 0x7fffffff;

		// sort the values by magnitude
		if (ms < mt)
		{
			temp = fs;
			fs = ft;
			ft = temp;
		}

		// get the exponents
		es = (fs >> 23) & 0xff;
		et = (ft >> 23) & 0xff;
		ed = es - et;

		// debug
		//e1 = es;
		//e2 = et;
		//e3 = ed;

		// get the signs
		ss = fs >> 31;
		st = ft >> 31;

		// get the mantissa and add the hidden bit
		ms = fs & 0x007fffff;
		mt = ft & 0x007fffff;

		// add hidden bit
		ms |= 0x00800000;
		mt |= 0x00800000;

		//cout << "\nAfter hidden bit ms=" << hex << ms << " mt=" << mt;

		// apply the zeros
		if (!es) ms = 0;
		if (!et) mt = 0;

		// apply the signs
		//ms = ( ms ^ ss ) - ss;
		//mt = ( mt ^ st ) - st;

		// do the shift
		//if ( ed > 23 )
		if (ed > (24 + c_iHiddenBits))
		{
			//ed = 24;
			ed = (24 + c_iHiddenBits);
		}

		// shift up so we don't lose precision
		ms <<= c_iHiddenBits;
		mt <<= c_iHiddenBits;

		// shift smaller value down
		mt >>= ed;

		//cout << "\nAfter shift down mt=" << hex << mt;

		// apply the signs
		ms = (ms ^ ss) - ss;
		mt = (mt ^ st) - st;

		//cout << "\nAfter signs ms=" << hex << ms << " mt=" << mt;

		// do the addition
		md = ms + mt;

		// shift back down result
		//md >>= 4;

		// debug
		//e4 = ms;
		//e5 = mt;
		//e6 = md;


		//cout << "\nAfter addition md=" << hex << md;


		// get the sign
		sd = md >> 31;

		// remove sign
		md = (md ^ sd) - sd;


		//cout << "\nAfter removing sign md=" << hex << md;


		// get new shift amount
		//ed = 8 - __builtin_clz( md );
		//ed = 8 - clz32(md);
		ed = (8 - c_iHiddenBits) - clz32(md);




		// update exponent
		ed += es;


		// debug
		//e7 = es;
		//e8 = ed;
		//e9 = md;


		// make zero if the result is zero
		if (!md)
		{
			ed = 0;
		}
		else
		{
			// get rid of the leading one (hidden bit)
			//md <<= ( __builtin_clz( md ) + 1 );
			md <<= (clz32(md) + 1);
			md = ((unsigned long)md) >> 9;
		}


		//cout << "\nAfter removing leading one md=" << hex << md;


		// remove hidden bit
		//md &= 0x7fffff;




		// check for zero
		if (ed <= 0)
		{
			md = 0;

			// set zero flag //
			*MACFlag |= (1 << (index + 0));
			*StatusFlag |= 0x41;

			// check for underflow
			if (ed < 0)
			{
				// set underflow flag //
				//statflag = 0x4008;
				*MACFlag |= (1 << (index + 8));
				*StatusFlag |= 0x104;
			}

			ed = 0;

			// get sign of zero
			sd = ss & st;
		}

		// check for overflow
		if (ed > 255)
		{
			// set overflow flag //
			//statflag = 0x8010;
			*MACFlag |= (1 << (index + 12));
			*StatusFlag |= 0x208;

			md = 0x7fffff;
			ed = 255;
		}

		// set sign flag //
		*MACFlag |= sd & (1 << (index + 4));
		*StatusFlag |= sd & 0x82;


		// debug
		//e10 = md;
		//e11 = ed;
		//e12 = sd;


		// add in exponent
		md += (ed << 23);


		//cout << "\nAfter adding exponent md=" << hex << md;


		// add in sign
		fd = md + (sd << 31);

		//cout << "\nAfter adding sign fd=" << hex << fd;

		return fd;
	}



	// mulitply float as integer
	inline static long multfloat ( long fs, long ft, int index, unsigned short* StatusFlag, unsigned short* MACFlag )
	{
		long es, et, ed;
		long sd;
		
		unsigned long long ms, mt, md;
		
		long ext;
		
		long sign;
		
		long fd;
		
		// sign is always a xor
		fd = ( fs ^ ft ) & -0x80000000;

		// set sign flag //
		sd = fd >> 31;
		*MACFlag |= sd & ( 1 << ( index + 4 ) );
		*StatusFlag |= sd & 0x82;
		
		// get the initial exponent
		es = ( fs >> 23 ) & 0xff;
		et = ( ft >> 23 ) & 0xff;
		ed = es + et - 127;
		
		// get the mantissa and add the hidden bit
		ms = fs & 0x007fffff;
		mt = ft & 0x007fffff;
		
		// optionally mask top bit from bottom ?? (need to test results)
		mt &= ~(mt >> 22);

		// add hidden bit
		ms |= 0x00800000;
		mt |= 0x00800000;
		
		// do the multiply
		md = ms * mt;
		
		
		// debug
		//ll12 = md;
		
		
		// get bit 47
		ext = md >> 47;
		
		// get the result
		md >>= ( 23 + ext );
		
		// remove the hidden bit
		md &= 0x7fffff;
		
		// update exponent
		ed += ext;
		
		if ( ed > 0xff )
		{
			fd |= 0x7fffffff;
			
			// set overflow flag //
			//statflag = 0x8010;
			*MACFlag |= ( 1 << ( index + 12 ) );
			*StatusFlag |= 0x208;
			
		}
		else if ( ( ed > 0 ) && es && et )
		{
			// put in exponent
			fd |= ( ed << 23 ) | md;
		}
		else
		{
			// set zero flag //
			*MACFlag |= ( 1 << ( index + 0 ) );
			*StatusFlag |= 0x41;
			
			// check for underflow
			if ( ( ed < 0 ) && es && et )
			{
				// set underflow flag //
				//statflag = 0x4008;
				*MACFlag |= ( 1 << ( index + 8 ) );
				*StatusFlag |= 0x104;
			}
		}
		
		return fd;
	}

	

	// mulitply float as integer
	// sign=1 for multsub, 0 for multadd
	inline static long multaddfloat ( long fs, long ft, long acc, long sign, int index, unsigned short* StatusFlag, unsigned short* MACFlag )
	{
		long es, et, ed;

		long ss, st, sd;
		
		long temp;
		
		long ms, mt, md;
		unsigned long long md64;
		
		//unsigned long long ms, mt, md;
		
		long ext;
		
		//long sign;
		
		long fd;
		
		// sign is always a xor
		fd = ( fs ^ ft ) & -0x80000000;

		// set sign flag //
		//sd = fd >> 31;
		// *MACFlag |= sd & ( 1 << ( index + 4 ) );
		// *StatusFlag |= sd & 0x82;
		
		// get the initial exponent
		es = ( fs >> 23 ) & 0xff;
		et = ( ft >> 23 ) & 0xff;
		ed = es + et - 127;
		
		// get the mantissa and add the hidden bit
		ms = fs & 0x007fffff;
		mt = ft & 0x007fffff;
		
		// optionally mask top bit from bottom ?? (need to test results)
		mt &= ~(mt >> 22);
		
		// add hidden bit
		ms |= 0x00800000;
		mt |= 0x00800000;
		
		// do the multiply
		md64 = (u64)ms * (u64)mt;
		
		
		// debug
		//ll12 = md;
		
		
		// get bit 47
		ext = md64 >> 47;
		
		// get the result
		md64 >>= ( 23 + ext );
		
		// remove the hidden bit
		md64 &= 0x7fffff;
		
		// update exponent
		ed += ext;
		
		if ( ed > 0xff )
		{
			fd |= 0x7fffffff;
			
			
			// set overflow flag //
			// *MACFlag |= ( 1 << ( index + 12 ) );
			// *StatusFlag |= 0x208;
			
		}
		else if ( ( ed > 0 ) && es && et )
		{
			// put in exponent
			fd |= ( ed << 23 ) | md64;
		}
		else
		{
			// set zero flag //
			// *MACFlag |= ( 1 << ( index + 0 ) );
			// *StatusFlag |= 0x41;
			
			// check for underflow
			if ( ( ed < 0 ) && es && et )
			{
				// set underflow flag //
				//statflag = 0x4008;
				// *MACFlag |= ( 1 << ( index + 8 ) );
				// *StatusFlag |= 0x104;
				// set underflow sticky flag ONLY
				*StatusFlag |= 0x100;

				// todo: calculation should stop here and return fd (previous value) ??
				// 
			}
			
			// this is tricky, because if acc is +/-max then set zero sticky flag ??
			if ( ( acc & 0x7fffffff ) == 0x7fffffff )
			{
				// set zero sticky flag ONLY
				*StatusFlag |= 0x40;
			}
			
		}
		
		
		// sign
		fd = ( fd ^ ( sign << 31 ) );
		
		// now, before doing the add, if fd is +/-max then set to acc
		if ( ( fd & 0x7fffffff ) == 0x7fffffff )
		{
			acc = fd;
		}
		
		// now, before doing the add, if acc is +/-max then set to fd
		//if ( ( acc & 0x7f800000 ) == 0x7f800000 )
		if ( ( acc & 0x7fffffff ) == 0x7fffffff )
		{
			fd = acc;
		}
		
		// now do the add part //
		fs = fd;
		ft = acc;

		// get the magnitude
		// todo: or just sort the values by exponent ?
		ms = fs & 0x7fffffff;
		mt = ft & 0x7fffffff;
		
		// sort the values by magnitude
		if ( ms < mt )
		{
			temp = fs;
			fs = ft;
			ft = temp;
		}
		
		// get the exponents
		es = ( fs >> 23 ) & 0xff;
		et = ( ft >> 23 ) & 0xff;
		ed = es - et;
		
		// debug
		//e1 = es;
		//e2 = et;
		//e3 = ed;
		
		// get the signs
		ss = fs >> 31;
		st = ft >> 31;
		
		// get the mantissa and add the hidden bit
		ms = fs & 0x007fffff;
		mt = ft & 0x007fffff;
		
		// add hidden bit
		ms |= 0x00800000;
		mt |= 0x00800000;
		
		// apply the zeros
		if ( !es ) ms = 0;
		if ( !et ) mt = 0;
		
		// do the shift
		if ( ed > 23 )
		{
			ed = 24;
		}
		
		mt >>= ed;
		
		
		// apply the signs
		ms = ( ms ^ ss ) - ss;
		mt = ( mt ^ st ) - st;
		
		
		
		// do the addition
		md = ms + mt;


		// debug
		//e4 = ms;
		//e5 = mt;
		//e6 = md;
		
		
		// get the sign
		sd = md >> 31;
		
		// remove sign
		md = ( md ^ sd ) - sd;

		

		
		// get new shift amount
		//ed = 8 - __builtin_clz( md );
		ed = 8 - clz32(md);

		
		
		
		// update exponent
		ed += es;

		
		// debug
		//e7 = es;
		//e8 = ed;
		//e9 = md;

		
		// make zero if the result is zero
		if ( !md )
		{
			ed = 0;
		}
		else
		{
			// get rid of the leading one (hidden bit)
			//md <<= ( __builtin_clz( md ) + 1 );
			md <<= (clz32(md) + 1);
			md = ( (unsigned long) md ) >> 9;
		}
		
		
		
		
		// remove hidden bit
		//md &= 0x7fffff;



		
		// check for zero
		if ( ed <= 0 )
		{
			md = 0;
			
			// set zero flag //
			*MACFlag |= ( 1 << ( index + 0 ) );
			*StatusFlag |= 0x41;
			
			// check for underflow
			if ( ed < 0 )
			{
				// set underflow flag //
				//statflag = 0x4008;
				*MACFlag |= ( 1 << ( index + 8 ) );
				*StatusFlag |= 0x104;
			}
			
			ed = 0;
			
			// get sign of zero
			sd = ss & st;
		}
		
		// check for overflow
		if ( ed > 255 )
		{
			// set overflow flag //
			//statflag = 0x8010;
			*MACFlag |= ( 1 << ( index + 12 ) );
			*StatusFlag |= 0x208;
			
			md = 0x7fffff;
			ed = 255;
		}
		
		
		// set sign flag //
		*MACFlag |= ( sd & ( 1 << ( index + 4 ) ) );
		*StatusFlag |= ( sd & 0x82 );

		
		// debug
		//e10 = md;
		//e11 = ed;
		//e12 = sd;

		
		// add in exponent
		md += ( ed << 23 );
		
		
		// add in sign
		fd = md + ( sd << 31 );
		
		return fd;
	}



	
	// PS2 floating point ADD
	inline static float PS2_Float_Add ( float fs, float ft, int index, unsigned short* StatusFlag, unsigned short* MACFlag )		//long* zero, long* sign, long* overflow, long* underflow,
									//long* zero_sticky, long* sign_sticky, long* overflow_sticky, long* underflow_sticky )
	{
		// fd = fs + ft
		
		FloatLong Result;
		
		//short sflag1 = 0, sflag2 = 0, mflag1 = 0, mflag2 = 0;
#ifdef USE_INTEGER_MATH_ADD
		FloatLong fls, flt;
		fls.f = fs;
		flt.f = ft;

		Result.l = addfloat ( fls.l, flt.l, index, StatusFlag, MACFlag );
		
		
#else

#ifdef USE_DOUBLE_MATH_ADD
		DoubleLong ds, dt, dResult;

//cout << "\nPS2_Float_Add";
		//ClampValue2_f ( fs, ft );

		ds.l = CvtPS2FloatToDouble ( fs );
		dt.l = CvtPS2FloatToDouble ( ft );
		
		//fls.l = CvtDoubleToPS2Float( ds.l );
		//flt.l = CvtDoubleToPS2Float( dt.l );
		//fls.f = fs;
		//flt.s = ft;
		
//cout << " fs=" << fs << " ft=" << ft << " ds=" << hex << ds.l << " dt=" << dt.l;
		
		//FlushPS2DoubleToZero ( ds );
		//FlushPS2DoubleToZero ( dt );
		
		dResult.d = ds.d + dt.d;
		//Result.f = fls.f + flt.f;
		//Result.f = fs + ft;
//cout << " ds.d=" << ds.d << " dt.d=" << dt.d << " dResult.d=" << dResult.d;

		//fls.l = CvtDoubleToPS2Float ( dResult.d );
		
//if( ( ( Result.l - fls.l ) > 1 ) || ( ( Result.l - fls.l ) < -1 ) )
//{
//	cout << "\nmismatch Result.l=" << hex << " " << Result.l << " " << Result.f << " fls=" << " " << fls.l << " " << fls.f;
//}
		
		//Result.l = CvtDoubleToPS2Float ( dResult.d );
		//dResult.l = CvtPS2FloatToDouble ( Result.f );
		
		//sflag1 = *StatusFlag;
		//sflag2 = sflag1;
		//mflag1 = *MACFlag;
		//mflag2 = mflag1;

		SetFlagsOnResult_d ( dResult.d, index, StatusFlag, MACFlag );
		//SetFlagsOnResult_f ( Result.f, index, StatusFlag, MACFlag );
		//SetFlagsOnResult_d ( dResult.d, index, & sflag1, & mflag1 );
		//SetFlagsOnResult_f ( Result.f, index, & sflag2, & mflag2 );
		
//if ( sflag1 != sflag2 )
//{
//	cout << "\nmismatch stat flag1=" << hex << " " << sflag1 << " " << sflag2;
//}

//if ( mflag1 != mflag2 )
//{
//	cout << "\nmismatch mac flag1=" << hex << " " << mflag1 << " " << mflag2;
//}



		
		Result.l = CvtDoubleToPS2Float ( dResult.d );
//cout << " Result.l=" << Result.l << " Result.f=" << Result.f;

#else
	

		ClampValue2_f ( fs, ft );
		
		Result.f = fs + ft;

#ifdef ENABLE_FLAG_CHECK

		SetFlagsOnResult_f ( Result.f, index, StatusFlag, MACFlag );
		
#endif

#endif

#endif
		
		// done?
		return Result.f;
	}

	// PS2 floating point SUB
	inline static float PS2_Float_Sub ( float fs, float ft, int index, unsigned short* StatusFlag, unsigned short* MACFlag )		//long* zero, long* sign, long* overflow, long* underflow,
										//long* zero_sticky, long* sign_sticky, long* overflow_sticky, long* underflow_sticky )
	{
		// fd = fs - ft
		
		FloatLong Result;

#ifdef USE_INTEGER_MATH_SUB
		FloatLong fls, flt;
		fls.f = fs;
		flt.f = ft;
		
		flt.l ^= -0x80000000;

		Result.l = addfloat ( fls.l, flt.l, index, StatusFlag, MACFlag );
		
		
#else

#ifdef USE_DOUBLE_MATH_SUB
		//double ds, dt, dResult;
		DoubleLong ds, dt, dResult;

//cout << "\nPS2_Float_Sub";

		ds.l = CvtPS2FloatToDouble ( fs );
		dt.l = CvtPS2FloatToDouble ( ft );
		
//cout << " fs=" << fs << " ft=" << ft << " ds=" << hex << ds.l << " dt=" << dt.l;
		
		//FlushPS2DoubleToZero ( ds );
		//FlushPS2DoubleToZero ( dt );
		
		dResult.d = ds.d - dt.d;
//cout << " ds.d=" << ds.d << " dt.d=" << dt.d << " dResult.d=" << dResult.d;

		SetFlagsOnResult_d ( dResult.d, index, StatusFlag, MACFlag );
		
		Result.l = CvtDoubleToPS2Float ( dResult.d );
//cout << " Result.l=" << Result.l << " Result.f=" << Result.f;
		
#else

		ClampValue2_f ( fs, ft );
		
		Result.f = fs - ft;

		
#ifdef ENABLE_FLAG_CHECK


		SetFlagsOnResult_f ( Result.f, index, StatusFlag, MACFlag );

#endif

#endif	// #ifdef USE_DOUBLE_MATH_SUB

#endif	// #ifdef USE_INTEGER_MATH_SUB
		
		// done?
		return Result.f;
	}

	// PS2 floating point MUL
	inline static float PS2_Float_Mul ( float fs, float ft, int index, unsigned short* StatusFlag, unsigned short* MACFlag )		//long* zero, long* sign, long* underflow, long* overflow,
								//long* zero_sticky, long* sign_sticky, long* underflow_sticky, long* overflow_sticky )
	{
		// fd = fs * ft

		
		FloatLong Result;
		FloatLong flf;

#ifdef USE_INTEGER_MATH_MUL
		FloatLong fls, flt;
		fls.f = fs;
		flt.f = ft;

		Result.l = multfloat ( fls.l, flt.l, index, StatusFlag, MACFlag );
		
		
#else


#ifdef USE_DOUBLE_MATH_MUL
		//double ds, dt, dResult;
		DoubleLong ds, dt, dResult;

//cout << "\nPS2_Float_Mul";

#ifdef ENABLE_MUL_PRECISION_CHOP
		flf.l = (((long&) ft) & 0xfffffffe);
#else
		flf.f = ft;
#endif
		
		ds.l = CvtPS2FloatToDouble ( fs );
		dt.l = CvtPS2FloatToDouble ( flf.f );
		
//cout << " fs=" << fs << " ft=" << ft << " ds=" << hex << ds.l << " dt=" << dt.l;
		
		//FlushPS2DoubleToZero ( ds );
		//FlushPS2DoubleToZero ( dt );
		
		dResult.d = ds.d * dt.d;
//cout << " ds.d=" << ds.d << " dt.d=" << dt.d << " dResult.d=" << dResult.d;

		SetFlagsOnResult_d ( dResult.d, index, StatusFlag, MACFlag );
		
		Result.l = CvtDoubleToPS2Float ( dResult.d );
//cout << " Result.l=" << Result.l << " Result.f=" << Result.f;
		
#else

		ClampValue2_f ( fs, ft );
		
		
#ifdef ENABLE_MUL_PRECISION_CHOP
		// multiply does not use full precision ??
		flf.f = ft;
		flf.l &= 0xfffffffe;
		
		Result.f = fs * flf.f;
#else
		Result.f = fs * ft;
#endif

		
#ifdef ENABLE_FLAG_CHECK

		SetFlagsOnResult_f ( Result.f, index, StatusFlag, MACFlag );
		
#endif

#endif	// #ifdef USE_DOUBLE_MATH_MUL

#endif	// #ifdef USE_INTEGER_MATH_MUL
		
		// done?
		return Result.f;
	}

	// PS2 floating point MADD
	inline static float PS2_Float_Madd ( float dACC, float fd, float fs, float ft, int index, unsigned short* StatusFlag, unsigned short* MACFlag )		//long* zero, long* sign, long* underflow, long* overflow,
								//long* zero_sticky, long* sign_sticky, long* underflow_sticky, long* overflow_sticky )
	{
		// fd = ACC + fs * ft
		
		FloatLong Result;
		FloatLong ACC;
		FloatLong flf;

#ifdef USE_INTEGER_MATH_MADD
		FloatLong fls, flt;
		fls.f = fs;
		flt.f = ft;
		ACC.f = dACC;

		Result.l = multaddfloat ( fls.l, flt.l, ACC.l, 0, index, StatusFlag, MACFlag );
		
#else

#ifdef USE_DOUBLE_MATH_MADD
		//double ds, dt, dd;
		DoubleLong ds, dt, dd;
		DoubleLong dProd, dSum, dResult;
		DoubleLong dlACC;

#ifdef ENABLE_MUL_PRECISION_CHOP
		flf.l = (((long&) ft) & 0xfffffffe);
#else
		flf.f = ft;
#endif

		ds.l = CvtPS2FloatToDouble ( fs );
		dt.l = CvtPS2FloatToDouble ( flf.f );
		
		//FlushPS2DoubleToZero ( ds );
		//FlushPS2DoubleToZero ( dt );
		
		// get accumulator
		dlACC.l = CvtPS2FloatToDouble ( dACC );
		
		// I think it does both the multiply and the sum...
		dProd.d = ds.d * dt.d;
		dSum.d = dlACC.d + dProd.d;
		
		dd.l = CvtPS2FloatToDouble ( fd );
		
		// set preliminary result to the sum
		dResult.d = dSum.d;
		
		SetFlagsOnResultDF_d ( dResult, dlACC, dProd, dSum, dd.d, index, StatusFlag, MACFlag );

				
		Result.l = CvtDoubleToPS2Float ( dResult.d );
		
#else

		ClampValue2_f ( fs, ft );
		
		// also need to clamp accumulator
		// no, actually, you don't
		//ClampValue_f ( dACC );

		
#ifdef ENABLE_MUL_PRECISION_CHOP
		// multiply does not use full precision ??
		flf.f = ft;
		flf.l &= 0xfffffffe;
		
		Result.f = fs * flf.f;
#else
		Result.f = fs * ft;
#endif

		
#ifdef ENABLE_FLAG_CHECK_MADD

	// check for multiply overflow
	if ( ( Result.l & c_lFloat_ExpMask ) == c_lFloat_ExpMask )
	{
		// multiply overflow in MADD //
		*StatusFlag |= 0x208;
		*MACFlag |= ( 1 << ( index + 12 ) );
		
		// sign flag
		if ( Result.l >> 31 )
		{
			*StatusFlag |= 0x82;
			*MACFlag |= ( 1 << ( index + 4 ) );
		}
		
		// set to +/-max
		Result.l = c_lFloat_MantissaMask | Result.l;
		
		// return result
		return Result.f;
	}
	
	// check for ACC overflow
	ACC.f = dACC;
	if ( ( ACC.l & c_lFloat_ExpMask ) == c_lFloat_ExpMask )
	{
		// multiply overflow in MADD //
		*StatusFlag |= 0x208;
		*MACFlag |= ( 1 << ( index + 12 ) );
		
		// sign flag
		if ( ACC.l >> 31 )
		{
			*StatusFlag |= 0x82;
			*MACFlag |= ( 1 << ( index + 4 ) );
		}
		
		// set to +/-max
		ACC.l = c_lFloat_MantissaMask | ACC.l;
		
		// check for multiply underflow, and set sticky underflow ONLY if so
		if ( ( Result.l & c_lFloat_MantissaMask ) && !( Result.l & c_lFloat_ExpMask ) )
		{
			// multiply underflow in MADD //
			
			// *note* ONLY set underflow sticky flag on underflow
			*StatusFlag |= 0x100;
			// *MACFlag |= ( 1 << ( index + 8 ) );

		}
		
		// return result
		return ACC.f;
	}
	
	
	// check for multiply underflow
	if ( ( Result.l & c_lFloat_MantissaMask ) && !( Result.l & c_lFloat_ExpMask ) )
	{
		// multiply underflow in MADD //
		
		// *note* ONLY set underflow sticky flag on underflow
		*StatusFlag |= 0x100;
		// *MACFlag |= ( 1 << ( index + 8 ) );

		// set result to previous value ??
		//Result.f = fd;
		
		// flag check based on ACC ??
		SetFlagsOnResult_f ( ACC.f, index, StatusFlag, MACFlag );
		
#ifdef ENABLE_PREVIOUS_VALUE_ON_MULTIPY_UNDERFLOW
		// return previous value ??
		return fd;
#else
		return ACC.f;
#endif
	}
	
	// if not multiply overflow on final check, then do the add and check flags
	
	// perform the addition
	Result.f += dACC;
	
	SetFlagsOnResult_f ( Result.f, index, StatusFlag, MACFlag );
	
	return Result.f;

#else
		
#ifdef ENABLE_FLAG_CHECK

		//SetFlagsOnResult_f ( Result.f, index, StatusFlag, MACFlag );
		
		
		// if multiply overflow, then set flags and return result
		// for now, just set flags
		if ( ( Result.l & c_lFloat_ExpMask ) == c_lFloat_ExpMask )
		{
			// multiply overflow in MADD //
			*StatusFlag |= 0x208;
			*MACFlag |= ( 1 << ( index + 12 ) );
			
			// sign flag
			if ( Result.l >> 31 )
			{
				*StatusFlag |= 0x82;
				*MACFlag |= ( 1 << ( index + 4 ) );
			}
			
			// set to +/-max
			Result.l = c_lFloat_MantissaMask | Result.l;
			
			// return result
			return Result.f;
		}
		
		
		
		// if multiply underflow, then only store ACC if it is +/-MAX, otherwise keep previous value
		// for now, just set sticky flags
		if ( ( Result.l & c_lFloat_MantissaMask ) && !( Result.l & c_lFloat_ExpMask ) )
		{
			// multiply underflow in MADD //
			
			// *note* ONLY set underflow sticky flag on underflow
			*StatusFlag |= 0x100;
			// *MACFlag |= ( 1 << ( index + 8 ) );

		}
		
		
#endif

#endif

		// perform the addition
		Result.f += dACC;


#ifdef ENABLE_FLAG_CHECK

		SetFlagsOnResult_f ( Result.f, index, StatusFlag, MACFlag );

#endif

#endif

#endif	// #ifdef USE_INTEGER_MATH_MADD
		
		// done?
		return Result.f;
	}

	
	// PS2 floating point MSUB
	inline static float PS2_Float_Msub ( float dACC, float fd, float fs, float ft, int index, unsigned short* StatusFlag, unsigned short* MACFlag )		//long* zero, long* sign, long* underflow, long* overflow,
								//long* zero_sticky, long* sign_sticky, long* underflow_sticky, long* overflow_sticky )
	{
		// fd = ACC + fs * ft
		
		FloatLong Result;
		FloatLong ACC;
		FloatLong flf;
		
		FloatLong fResultTest;
		
		DoubleLong dfs, dft;
		FloatLong fProd;
		
		short sflag1 = 0, sflag2 = 0, mflag1 = 0, mflag2 = 0;

#ifdef USE_INTEGER_MATH_MSUB
		FloatLong fls, flt;
		fls.f = fs;
		flt.f = ft;
		ACC.f = dACC;

		Result.l = multaddfloat ( fls.l, flt.l, ACC.l, 1, index, StatusFlag, MACFlag );
		
#else

#ifdef USE_DOUBLE_MATH_MSUB
		//double ds, dt, dd;
		DoubleLong ds, dt, dd;
		DoubleLong dProd, dSum, dResult;
		DoubleLong dlACC;

#ifdef ENABLE_MUL_PRECISION_CHOP
		flf.l = (((long&) ft) & 0xfffffffe);
#else
		flf.f = ft;
#endif

		ds.l = CvtPS2FloatToDouble ( fs );
		dt.l = CvtPS2FloatToDouble ( flf.f );
		
		//FlushPS2DoubleToZero ( ds );
		//FlushPS2DoubleToZero ( dt );
		
		// get accumulator
		dlACC.l = CvtPS2FloatToDouble ( dACC );
		
		// I think it does both the multiply and the sum...
		// note: probably need to flag check before the subtraction? Even if just to clamp the value?
		dProd.d = -( ds.d * dt.d );
		
		// need to get rid of the extra bits after multiply so you get the correct result
		// should only be 1 sign bit, 11 exponent bits, 23 mantissa bits = 35 bits
		//dProd.l += 0x0000000010000000ull;
		//dProd.l &= 0xffffffffe0000000ull;
		
		dSum.d = dlACC.d + dProd.d;

		// testing
		//fResultTest.f = -( fs * ft );
		//fResultTest.f = dACC + fResultTest.f;
		
		
		dd.l = CvtPS2FloatToDouble ( fd );
		
		dResult.d = dSum.d;
		
		
		SetFlagsOnResultDF_d ( dResult, dlACC, dProd, dSum, dd.d, index, StatusFlag, MACFlag );
		//SetFlagsOnResultDF_d ( dResult, dlACC, dProd, dSum, dd.d, index, &sflag1, &mflag1 );

				
		Result.l = CvtDoubleToPS2Float ( dResult.d );
		//fResultTest.l = Result.l;
		
		
#else
		

		ClampValue2_f ( fs, ft );
		
		// also need to clamp accumulator
		// no, actually, you don't
		//ClampValue_f ( dACC );

		
#ifdef ENABLE_MUL_PRECISION_CHOP
		// multiply does not use full precision ??
		flf.f = ft;
		flf.l &= 0xfffffffe;
		
#ifdef NEGATE_MSUB_MULTIPLY_RESULT
		Result.f = -( fs * flf.f );
#else
		Result.f = fs * flf.f;
#endif

#else

		// probably does -fs times ft, then adds with the accumulator
#ifdef NEGATE_MSUB_MULTIPLY_RESULT
		Result.f = -( fs * ft );
#else
		Result.f = fs * ft;
#endif

#endif	// #ifdef ENABLE_MUL_PRECISION_CHOP


// testing
//fProd.f = Result.f;


#ifdef ENABLE_FLAG_CHECK_MADD

	// check for multiply overflow
	if ( ( Result.l & c_lFloat_ExpMask ) == c_lFloat_ExpMask )
	{
		// multiply overflow in MADD //
		*StatusFlag |= 0x208;
		*MACFlag |= ( 1 << ( index + 12 ) );
		
		// sign flag
		if ( Result.l >> 31 )
		{
			*StatusFlag |= 0x82;
			*MACFlag |= ( 1 << ( index + 4 ) );
		}
		
		// set to +/-max
		Result.l = c_lFloat_MantissaMask | Result.l;
		
		// return result
		return Result.f;
	}
	
	// check for ACC overflow
	ACC.f = dACC;
	if ( ( ACC.l & c_lFloat_ExpMask ) == c_lFloat_ExpMask )
	{
		// multiply overflow in MADD //
		*StatusFlag |= 0x208;
		*MACFlag |= ( 1 << ( index + 12 ) );
		
		// sign flag
		if ( ACC.l >> 31 )
		{
			*StatusFlag |= 0x82;
			*MACFlag |= ( 1 << ( index + 4 ) );
		}
		
		// set to +/-max
		ACC.l = c_lFloat_MantissaMask | ACC.l;
		
		// check for multiply underflow, and set sticky underflow ONLY if so
		if ( ( Result.l & c_lFloat_MantissaMask ) && !( Result.l & c_lFloat_ExpMask ) )
		{
			// multiply underflow in MADD //
			
			// *note* ONLY set underflow sticky flag on underflow
			*StatusFlag |= 0x100;
			// *MACFlag |= ( 1 << ( index + 8 ) );

		}
		
		// return result
		return ACC.f;
	}
	
	
	// check for multiply underflow
	if ( ( Result.l & c_lFloat_MantissaMask ) && !( Result.l & c_lFloat_ExpMask ) )
	{
		// multiply underflow in MADD //
		
		// *note* ONLY set underflow sticky flag on underflow
		*StatusFlag |= 0x100;
		// *MACFlag |= ( 1 << ( index + 8 ) );

		// set result to previous value ??
		//Result.f = fd;
		
		// flag check based on ACC ??
		SetFlagsOnResult_f ( ACC.f, index, StatusFlag, MACFlag );
		
#ifdef ENABLE_PREVIOUS_VALUE_ON_MULTIPY_UNDERFLOW
		// return previous value ??
		return fd;
#else
		return ACC.f;
#endif
	}
	
	// if not multiply overflow on final check, then do the add and check flags
	
	// perform the addition
	Result.f += dACC;
	
	SetFlagsOnResult_f ( Result.f, index, StatusFlag, MACFlag );
	
//if ( ( ( Result.l - fResultTest.l ) > 1 ) || ( ( Result.l - fResultTest.l ) < -1 ) )
//{
//	cout << "\nResultTest failed: Result.l=" << hex << Result.l << " " << Result.f << " fResultTest=" << hex << fResultTest.l << " " << fResultTest.f;
//	cout << " fs=" << fs << " ft=" << ft << " acc=" << dACC;
//	cout << " dfs=" << ds.l << " " << ds.d << " dft=" << dt.l << " " << dt.d << " dResult=" << dResult.l << " " << dResult.d;
//	cout << " fProd=" << fProd.l << " " << fProd.f << " dProd=" << dProd.l << " " << dProd.d;
//	cout << " dlACC=" << dlACC.l << " " << dlACC.d;
//	cout << " fd=" << fd << " " << dd.l << " " << dd.d;
//}

//if ( sflag1 != *StatusFlag )
//{
//	cout << "\nmismatch stat flag1=" << hex << " " << sflag1 << " " << *StatusFlag;
//}

//if ( mflag1 != *MACFlag )
//{
//	cout << "\nmismatch mac flag1=" << hex << " " << mflag1 << " " << *MACFlag;
//}


	// *StatusFlag = sflag1;
	// *MACFlag = mflag1;
	//return fResultTest.f;
	return Result.f;

#else
		
#ifdef ENABLE_FLAG_CHECK

		//SetFlagsOnResult_f ( Result.f, index, StatusFlag, MACFlag );
		
		
		// if multiply overflow, then set flags and return result
		// for now, just set flags
		if ( ( Result.l & c_lFloat_ExpMask ) == c_lFloat_ExpMask )
		{
			// multiply overflow in MADD //
			
			// overflow flag
			*StatusFlag |= 0x208;
			*MACFlag |= ( 1 << ( index + 12 ) );
			
			// sign flag
			if ( Result.l >> 31 )
			{
				*StatusFlag |= 0x82;
				*MACFlag |= ( 1 << ( index + 4 ) );
			}
			
			// set to +/-max
			Result.l = c_lFloat_MantissaMask | Result.l;
			
			// return result
			return Result.f;
		}
		
		
		
		// if multiply underflow, then only store ACC if it is +/-MAX, otherwise keep previous value
		// for now, just set sticky flags
		if ( ( Result.l & c_lFloat_MantissaMask ) && !( Result.l & c_lFloat_ExpMask ) )
		{
			// multiply underflow in MADD //
			
			// *note* ONLY set underflow sticky flag on underflow
			*StatusFlag |= 0x100;
			// *MACFlag |= ( 1 << ( index + 8 ) );

		}
		
#endif

#endif

		
		// perform the addition
#ifdef NEGATE_MSUB_MULTIPLY_RESULT
		Result.f = dACC + Result.f;
#else
		Result.f = dACC - Result.f;
#endif



#ifdef ENABLE_FLAG_CHECK

		SetFlagsOnResult_f ( Result.f, index, StatusFlag, MACFlag );

#endif

#endif

#endif	// #ifdef USE_INTEGER_MATH_MSUB


		// done?
		return Result.f;
	}


	
	inline static long PS2_Float_ToInteger ( float fs )
	{
		//FloatLong Result;
		long lResult;
		
		//if ( isNaNorInf ( fs ) ) (long&) fs = ( ( (long&) fs ) & 0xff800000 );
		//if ( isNaNorInf ( ft ) ) (long&) ft = ( ( (long&) ft ) & 0xff800000 );
		
		lResult = (long&) fs;
		
		// get max
		//fResult = ( ( fs < ft ) ? fs : ft );
		if ( ( lResult & 0x7f800000 ) <= 0x4e800000 )
		{
			lResult = (long) fs;
		}
		else if ( lResult & 0x80000000 )
		{
			// set to negative integer max
			lResult = 0x80000000;
		}
		else
		{
			// set to positive integer max
			lResult = 0x7fffffff;
		}
		
		// MIN does NOT affect any flags
		
		// done?
		return lResult;
	}


	inline static float PS2_Float_Max ( float fs, float ft )
	{
		//FloatLong Result;
		float fResult;
		
#ifdef COMPUTE_MINMAX_AS_INTEGER
		long lfs, lft;
		
		lfs = (long&) fs;
		lft = (long&) ft;
		
		// if value is negative as integer, then take two's compliment of the absolute value?
		// note: instead of taking two's compliment, should just take inverse
		/*
		//lfs = ( lfs >= 0 ) ? lfs : -( lfs & 0x7fffffff );
		//lft = ( lft >= 0 ) ? lft : -( lft & 0x7fffffff );
		lfs = ( lfs >= 0 ) ? lfs : ~( lfs & 0x7fffffff );
		lft = ( lft >= 0 ) ? lft : ~( lft & 0x7fffffff );
		*/
		
		// take two's complement before comparing if negative
		lfs = ( ( lfs >> 31 ) ^ ( lfs & 0x7fffffff ) ) + ( ( lfs >> 31 ) & 1 );
		lft = ( ( lft >> 31 ) ^ ( lft & 0x7fffffff ) ) + ( ( lft >> 31 ) & 1 );

		
		// compare as integer and return original value?
		fResult = ( ( lfs > lft ) ? fs : ft );
		
#else

#ifdef IGNORE_DENORMAL_MAXMIN
		if ( isNaNorInf ( fs ) ) (long&) fs = ( ( (long&) fs ) & 0xff800000 );
		if ( isNaNorInf ( ft ) ) (long&) ft = ( ( (long&) ft ) & 0xff800000 );
#else
		ClampValue2_f ( fs, ft );
#endif
		
		// get max
		fResult = ( ( fs > ft ) ? fs : ft );
		
#endif

		// MAX does NOT affect any flags
		
		// done?
		return fResult;
	}

	inline static float PS2_Float_Min ( float fs, float ft )
	{
		//FloatLong Result;
		float fResult;

#ifdef COMPUTE_MINMAX_AS_INTEGER		
		long lfs, lft;
		
		lfs = (long&) fs;
		lft = (long&) ft;
		
		// if value is negative as integer, then take two's compliment of the absolute value?
		// note: instead of taking two's compliment, should just take inverse
		/*
		//lfs = ( lfs >= 0 ) ? lfs : -( lfs & 0x7fffffff );
		//lft = ( lft >= 0 ) ? lft : -( lft & 0x7fffffff );
		lfs = ( lfs >= 0 ) ? lfs : ~( lfs & 0x7fffffff );
		lft = ( lft >= 0 ) ? lft : ~( lft & 0x7fffffff );
		*/
		
		// take two's complement before comparing if negative
		lfs = ( ( lfs >> 31 ) ^ ( lfs & 0x7fffffff ) ) + ( ( lfs >> 31 ) & 1 );
		lft = ( ( lft >> 31 ) ^ ( lft & 0x7fffffff ) ) + ( ( lft >> 31 ) & 1 );
		
		// compare as integer and return original value?
		fResult = ( ( lfs < lft ) ? fs : ft );
		
#else
		
#ifdef IGNORE_DENORMAL_MAXMIN
		if ( isNaNorInf ( fs ) ) (long&) fs = ( ( (long&) fs ) & 0xff800000 );
		if ( isNaNorInf ( ft ) ) (long&) ft = ( ( (long&) ft ) & 0xff800000 );
#else
		ClampValue2_f ( fs, ft );
#endif
		
		// get max
		fResult = ( ( fs < ft ) ? fs : ft );

#endif		
		// MIN does NOT affect any flags
		
		// done?
		return fResult;
	}

	

	// PS2 floating point SQRT
	inline static float PS2_Float_Sqrt ( float ft, unsigned short* StatusFlag )	//long* invalid_negative, long* invalid_zero,
										//long* divide_sticky, long* invalid_negative_sticky, long* invalid_zero_sticky )
	{
		FloatLong Result;
		long l;
		float f;
		
#ifdef USE_DOUBLE_MATH_SQRT
		// Q = sqrt ( ft )
		DoubleLong Dd, Ds, Dt;
		
		// convert to double
		// note: after conversion, if +/- inf/nan, should stay same
		//Dt.d = (double) ft;
		Dt.l = CvtPS2FloatToDouble ( ft );
		
		// clear affected non-sticky flags
		// note: mind as well clear the sticky flag area too since this shouldn't set the actual flag
		// *StatusFlag &= ~0x30;
		*StatusFlag &= ~0xc30;
		
		// set flag on sqrt of negative value
		if ( Dt.l < 0 )
		{
			*StatusFlag |= 0x410;
		}
		
		// flush ps2 denormal to zero as double
		//FlushPS2DoubleToZero ( Dt.d );
		
		//if ( isNaNorInf_d ( Dt.d ) )
		//{
		//	Dt.l = c_llPS2DoubleMax | ( Dt.l & c_llDoubleAbsMask );
		//}
		
		// absolute value
		Dt.l &= ~c_llDoubleAbsMask;
		
		// sqrt the numbers
		Dd.d = sqrt ( Dt.d );
		
		// flush denormal to zero again
		//FlushPS2DoubleToZero ( Dd.d );
		
		// convert back to float
		// note: for now, this is cool, because...
		// *** todo *** implement proper conversion (max PS2 value of +/- INF does not convert correctly)
		//Result.f = (float) Dd.d;
		Result.l = CvtDoubleToPS2Float ( Dd.d );

		return Result.f;
#else

		ClampValue_f ( ft );
		
		// absolute value of ft
		l = ( 0x7fffffff & (long&) ft );
		f = (float&) l;
		
		Result.f = sqrt ( f );

#endif
		
		// clear affected non-sticky flags
		// note: mind as well clear the sticky flag area too since this shouldn't set the actual flag
		// *StatusFlag &= ~0x30;
		*StatusFlag &= ~0xc30;
		
		// check zero division flag -> set to zero if divide by zero
		// write zero division flag -> set to zero for SQRT
		//*divide = -1;
		
		// write invalid flag (SQRT of negative number or 0/0)
		//*invalid_negative = (long&) ft;
		if ( ft < 0.0f )
		{
			*StatusFlag |= 0x410;
		}
		
		// write zero divide/invalid sticky flags
		// leave divide by zero sticky flag alone, since it did not accumulate
		//*divide_stickyflag &= -1;
		//*invalid_negative_sticky |= (long&) ft;
		
		// invalid zero is ok, since there is no divide here
		// leave sticky flag alone
		//*invalid_zero = 0;
		//*invalid_zero_sticky -> leave alone
		
		// done?
		return Result.f;
	}

	
	// PS2 floating point RSQRT
	inline static float PS2_Float_RSqrt ( float fs, float ft, unsigned short* StatusFlag )	//long* divide, long* invalid_negative, long* invalid_zero,
										//long* divide_sticky, long* invalid_negative_sticky, long* invalid_zero_sticky )
	{
		FloatLong Result;
		long l;
		float f;
		
#ifdef USE_DOUBLE_MATH_RSQRT
		// fd = fs + ft
		DoubleLong Dd, Ds, Dt;
		
		
		long temp1, temp2;
		
		// convert to double
		// note: after conversion, if +/- inf/nan, should stay same
		//Ds.d = (double) fs;
		//Dt.d = (double) ft;
		Ds.l = CvtPS2FloatToDouble ( fs );
		Dt.l = CvtPS2FloatToDouble ( ft );


		// clear affected non-sticky flags
		// note: mind as well clear the sticky flag area too since this shouldn't set the actual flag
		// *StatusFlag &= ~0x30;
		*StatusFlag &= ~0xc30;
		
		// write invalid flag (SQRT of negative number or 0/0)
		//*invalid_negative = (long&) ft;
		//temp1 = (long&) fs;
		//temp2 = (long&) ft;
		// *invalid_zero = temp1 | temp2;
		// *** todo ***
		// *invalid_zero = (long&) fs | (long&) ft;
		if ( ( Dt.l < 0 ) || ! ( ( Ds.l | Dt.l ) & 0x7fffffffffffffffULL ) )
		{
			*StatusFlag |= 0x410;
			
			/*
			if ( fs == 0.0f )
			{
				// make sure result is zero??
				Result.l &= 0x80000000;
			}
			*/
		}
		
		

		
		// flush ps2 denormal to zero as double
		//FlushPS2DoubleToZero ( Ds.d );
		//FlushPS2DoubleToZero ( Dt.d );
		
		//if ( isNaNorInf_d ( Ds.d ) )
		//{
		//	Ds.l = c_llPS2DoubleMax | ( Ds.l & c_llDoubleAbsMask );
		//}
		
		//if ( isNaNorInf_d ( Dt.d ) )
		//{
		//	Dt.l = c_llPS2DoubleMax | ( Dt.l & c_llDoubleAbsMask );
		//}
		
		// absolute value
		Dt.l &= ~c_llDoubleAbsMask;
		
		// RSQRT the numbers
		Dd.d = Ds.d / sqrt ( Dt.d );
		
		// flush denormal to zero again
		//FlushPS2DoubleToZero ( Dd.d );
		PS2DblMaxMin ( Dd.d );
		
		// convert back to float
		// note: for now, this is cool, because...
		// *** todo *** implement proper conversion (max PS2 value of +/- INF does not convert correctly)
		//Result.f = (float) Dd.d;
		Result.l = CvtDoubleToPS2Float ( Dd.d );
		
		
		// write zero division flag -> set to zero for SQRT
		// write denominator
		//*divide = (long&) ft;
		if ( ( Ds.l & 0x7fffffffffffffffULL ) && !( Dt.l & 0x7fffffffffffffffULL ) )
		{
			*StatusFlag |= 0x820;
			
		}
		
		if ( !( Dt.l & 0x7fffffffffffffffULL ) )
		{
			// set result to +max/-max ??
			Result.l = (long&) fs;
			Result.l |= 0x7fffffff;
		}
		
		return Result.f;
#else

		ClampValue2_f ( fs, ft );
		
		// absolute value of ft
		l = ( 0x7fffffff & (long&) ft );
		f = (float&) l;
		
		Result.f = fs / sqrt ( f );

#endif
		
		// clear affected non-sticky flags
		// note: mind as well clear the sticky flag area too since this shouldn't set the actual flag
		// *StatusFlag &= ~0x30;
		*StatusFlag &= ~0xc30;
		
		// write invalid flag (SQRT of negative number or 0/0)
		//*invalid_negative = (long&) ft;
		//temp1 = (long&) fs;
		//temp2 = (long&) ft;
		//*invalid_zero = temp1 | temp2;
		// *** todo ***
		//*invalid_zero = (long&) fs | (long&) ft;
		if ( ( ft < 0.0f ) || ( fs == 0.0f && ft == 0.0f ) )
		{
			*StatusFlag |= 0x410;
			
			if ( fs == 0.0f )
			{
				// make sure result is zero??
				Result.l &= 0x80000000;
			}
		}
		
		
		// write zero division flag -> set to zero for SQRT
		// write denominator
		//*divide = (long&) ft;
		if ( fs != 0.0f && ft == 0.0f )
		{
			*StatusFlag |= 0x820;
			
			// set result to +max/-max ??
			Result.l |= 0x7fffffff;
		}
		
		
		// write zero/sign sticky flags
		//*divide_sticky &= *divide;
		//*invalid_negative_sticky |= *invalid_negative;
		//*invalid_zero_sticky &= *invalid_zero;
		
		// done?
		return Result.f;
	}


	// PS2 floating point DIV
	inline static float PS2_Float_Div ( float fs, float ft, unsigned short* StatusFlag )		//long* divide, long* invalid_negative, long* invalid_zero,
										//long* divide_sticky, long* invalid_negative_sticky, long* invalid_zero_sticky )
	{
		FloatLong Result;
		
		
#ifdef USE_DOUBLE_MATH_DIV
		// fd = fs + ft
		DoubleLong Dd, Ds, Dt;
		
		// convert to double
		// note: after conversion, if +/- inf/nan, should stay same
		//Ds.d = (double) fs;
		//Dt.d = (double) ft;
		Ds.l = CvtPS2FloatToDouble ( fs );
		Dt.l = CvtPS2FloatToDouble ( ft );
		
		// flush ps2 denormal to zero as double
		//FlushPS2DoubleToZero ( Ds.d );
		//FlushPS2DoubleToZero ( Dt.d );
		
		//if ( isNaNorInf_d ( Ds.d ) )
		//{
		//	Ds.l = c_llPS2DoubleMax | ( Ds.l & c_llDoubleAbsMask );
		//}
		
		//if ( isNaNorInf_d ( Dt.d ) )
		//{
		//	Dt.l = c_llPS2DoubleMax | ( Dt.l & c_llDoubleAbsMask );
		//}
		
		// fd = fs / ft
		Dd.d = Ds.d / Dt.d;
		
		// flush denormal to zero again
		//FlushPS2DoubleToZero ( Dd.d );
		PS2DblMaxMin ( Dd.d );
		
		// convert back to float
		// note: for now, this is cool, because...
		// *** todo *** implement proper conversion (max PS2 value of +/- INF does not convert correctly)
		//Result.f = (float) Dd.d;
		Result.l = CvtDoubleToPS2Float ( Dd.d );
		
		// clear affected non-sticky flags
		// note: mind as well clear the sticky flag area too since this shouldn't set the actual flag
		// *StatusFlag &= ~0x30;
		*StatusFlag &= ~0xc30;
		
		// write zero division flag -> set to zero for SQRT
		// write denominator
		//if ( ft == 0.0f )
		if ( ! ( Dt.l & 0x7fffffffffffffffULL ) )
		{
			// also set result to +max or -max
			Result.l = ( Ds.l ^ Dt.l ) >> 32;
			Result.l |= 0x7fffffff;
			
			//if ( fs != 0.0f )
			if ( Ds.l & 0x7fffffffffffffffULL )
			{
				// set divide by zero flag //
				*StatusFlag |= 0x820;
				
			}
			else
			{
				// set invalid flag //
				*StatusFlag |= 0x410;
				
				// set to zero ??
				//Result.l &= 0x80000000;
			}
		}
		
		return Result.f;
#else

		ClampValue2_f ( fs, ft );
		
		Result.f = fs / ft;
		
#endif
		
		// clear affected non-sticky flags
		// note: mind as well clear the sticky flag area too since this shouldn't set the actual flag
		// *StatusFlag &= ~0x30;
		*StatusFlag &= ~0xc30;
		
		// write zero division flag -> set to zero for SQRT
		// write denominator
		if ( ft == 0.0f )
		{
			if ( fs != 0.0f )
			{
				// set divide by zero flag //
				*StatusFlag |= 0x820;
				
				// also set result to +max or -max
				Result.l |= 0x7fffffff;
			}
			else
			{
				// set invalid flag //
				*StatusFlag |= 0x410;
				
				// set to zero ??
				Result.l &= 0x80000000;
			}
		}
		
		// done?
		return Result.f;
	}
	
	

}


#endif



