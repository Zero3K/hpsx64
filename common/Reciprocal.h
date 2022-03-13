
namespace Math
{
	namespace Reciprocal
	{
		// take reciprocal of double
		static inline double INVD ( double a ) { return 1.0L / a; }
		
		// take integer part of double-precision floating point value
		static inline long long INTD ( double a ) { return (long long) a; }
		
		// attempt to speed up modulo via reciprocal multiplication
		static inline double RMOD ( double numerator, double denominator, double reciprocal )
		{
			double quotient;
			quotient = numerator * reciprocal;
			return denominator * ( quotient - (long long) quotient );
		}
		
		// take ceil of double
		static inline long long CEILD ( double a )
		{
			long long integer;
			integer = (long long) a;
			return ( ( ( a - integer ) == 0.0L ) ? integer : ( integer + 1 ) );
		}
		
	};
};


