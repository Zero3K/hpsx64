
#include "adpcm.h"
#include <iostream>

using namespace std;

/*

    Sony PlayStation SPU (CXD2922BQ/CXD2925Q) emulator
    by pSXAuthor
    MAME adaptation by R. Belmont

*/






signed short *adpcm_decoder::decode_packet(adpcm_packet *ap, signed short *dp)
{
	long shift=ap->info&0xf,
			filter=ap->info>>4,
			f0=filter_coef[filter][0],
			f1=filter_coef[filter][1];
			
	// according to Martin Korth no$cash PSX spec
	// invalid shift values (13-15) act same as shift=9 for 4-bit adpcm samples
	if ( shift > 12 ) shift = 9;
	
	// *** testing *** unsure what to do about filter
	// invalid values are >4 for SPU-adpcm
	if ( filter > 4 )
	{
		//cout << "\nhpsx64 ALERT: SPU-ADPCM: Filter value is greater than 4 (invalid): filter=" << dec << filter;
		filter = 4;
	}

	shift = 12 - shift;
			
	for (int i=0; i<14; i++)
	{
		unsigned char b=ap->data[i];
		
		//short bl=(b&0xf)<<12,
					//bh=(b>>4)<<12;
		long bl = sign_extend4 ( b & 0xf ) << shift,	//(b&0xf)<<12,
			bh = sign_extend4 ( b >> 4 ) << shift;	//(b>>4)<<12;

		//bl=(bl>>shift)+(((l0*f0)+(l1*f1)+32)>>6);
		bl = bl + ( ( (l0*f0) + (l1*f1) + 32 ) >> 6 );
		*dp++ = clamp ( bl );
		l1=l0;
		l0=bl;

		//bh=(bh>>shift)+(((l0*f0)+(l1*f1)+32)>>6);
		bh = bh + ( ( (l0*f0) + (l1*f1) + 32 ) >> 6 );
		*dp++ = clamp ( bh );
		l1=l0;
		l0=bh;

		/*
		if ( bl > 0x7fff ) cout << "\nbl>0x7fff\n";
		if ( bl < -0x8000 ) cout << "\nbl<-0x8000\n";
		if ( bh > 0x7fff ) cout << "\nbh>0x7fff\n";
		if ( bh < -0x8000 ) cout << "\nbh<-0x8000\n";
		*/
	}

	return dp;
}


short *adpcm_decoder::decode_packet_xa (unsigned char info, adpcm_packet_xa *ap, signed short *dp)
{
	long shift=/*ap->*/info&0xf,
			filter=/*ap->*/info>>4,
			f0=filter_coef[filter][0],
			f1=filter_coef[filter][1];

	// according to Martin Korth no$cash PSX spec
	// invalid shift values (13-15) act same as shift=9 for 4-bit adpcm samples
	if ( shift > 12 ) shift = 9;
	
	// *** testing *** unsure what to do about filter
	// invalid values are >3 for XA-adpcm
	if ( filter > 3 )
	{
		//cout << "\nhpsx64 ALERT: XA-ADPCM: Filter value is greater than 3 (invalid): filter=" << dec << filter;
		filter = 3;
	}

	shift = 12 - shift;
			
	for (int i=0; i<14; i++)
	{
		unsigned char b=ap->data[i];
		
		//short bl=(b&0xf)<<12,
					//bh=(b>>4)<<12;
		long bl = sign_extend4 ( b & 0xf ) << shift,	//(b&0xf)<<12,
			bh = sign_extend4 ( b >> 4 ) << shift;	//(b>>4)<<12;

		//bl=(bl>>shift)+(((l0*f0)+(l1*f1)+32)>>6);
		bl = bl + ( ( (l0*f0) + (l1*f1) + 32 ) >> 6 );
		*dp++= clamp ( bl );
		l1=l0;
		l0=bl;

		//bh=(bh>>shift)+(((l0*f0)+(l1*f1)+32)>>6);
		bh = bh + ( ( (l0*f0) + (l1*f1) + 32 ) >> 6 );
		*dp++= clamp ( bh );
		l1=l0;
		l0=bh;

		/*
		if ( bl > 0x7fff ) cout << "\nbl>0x7fff\n";
		if ( bl < -0x8000 ) cout << "\nbl<-0x8000\n";
		if ( bh > 0x7fff ) cout << "\nbh>0x7fff\n";
		if ( bh < -0x8000 ) cout << "\nbh<-0x8000\n";
		*/
	}

	return dp;
}





long *adpcm_decoder::decode_packet32(adpcm_packet *ap, long *dp)
{
	long shift=ap->info&0xf,
			filter=ap->info>>4;
	long f0=filter_coef[filter][0],
			f1=filter_coef[filter][1];

	// according to Martin Korth no$cash PSX spec
	// invalid shift values (13-15) act same as shift=9 for 4-bit adpcm samples
	if ( shift > 12 ) shift = 9;
	
	// *** testing *** unsure what to do about filter
	// invalid values are >4 for SPU-adpcm
	if ( filter > 4 )
	{
		//cout << "\nhpsx64 ALERT: SPU-ADPCM: Filter value is greater than 4 (invalid): filter=" << dec << filter;
		filter = 4;
	}

	shift = 12 - shift;
			
	for (int i=0; i<14; i++)
	{
		unsigned char b=ap->data[i];
		
		//short bl=(b&0xf)<<12,
					//bh=(b>>4)<<12;
		long bl = sign_extend4 ( b & 0xf ) << shift,	//(b&0xf)<<12,
			bh = sign_extend4 ( b >> 4 ) << shift;	//(b>>4)<<12;

		//bl=(bl>>shift)+(((l0*f0)+(l1*f1)+32)>>6);
		bl = bl + ( ( (l0*f0) + (l1*f1) + 32 ) >> 6 );
		//bl = clamp64 ( bl );
		*dp++ = bl;
		l1=l0;
		l0=bl;

		//bh=(bh>>shift)+(((l0*f0)+(l1*f1)+32)>>6);
		bh = bh + ( ( (l0*f0) + (l1*f1) + 32 ) >> 6 );
		//bh = clamp64 ( bh );
		*dp++ = bh;
		l1=l0;
		l0=bh;
	}

	return dp;
}


long *adpcm_decoder::decode_packet_xa32 (unsigned char info, adpcm_packet_xa *ap, long *dp)
{
	long shift= info & 0xf,
			filter= ( info>>4 ) & 0xf;
	long f0=filter_coef[filter][0],
			f1=filter_coef[filter][1];

	// according to Martin Korth no$cash PSX spec
	// invalid shift values (13-15) act same as shift=9 for 4-bit adpcm samples
	if ( shift > 12 ) shift = 9;
	
	// *** testing *** unsure what to do about filter
	// invalid values are >3 for XA-adpcm
	if ( filter > 3 )
	{
		//cout << "\nhpsx64 ALERT: XA-ADPCM: Filter value is greater than 3 (invalid): filter=" << dec << filter;
		filter = 3;
	}

	shift = 12 - shift;
			
	for (int i=0; i<14; i++)
	{
		unsigned char b=ap->data[i];
		
		//short bl=(b&0xf)<<12,
					//bh=(b>>4)<<12;
		long bl = sign_extend4 ( b & 0xf ) << shift,	//(b&0xf)<<12,
			bh = sign_extend4 ( b >> 4 ) << shift;	//(b>>4)<<12;

		//bl=(bl>>shift)+(((l0*f0)+(l1*f1)+32)>>6);
		bl = bl + ( ( (l0*f0) + (l1*f1) + 32 ) >> 6 );
		*dp++=bl;
		l1=l0;
		l0=bl;

		//bh=(bh>>shift)+(((l0*f0)+(l1*f1)+32)>>6);
		bh = bh + ( ( (l0*f0) + (l1*f1) + 32 ) >> 6 );
		*dp++=bh;
		l1=l0;
		l0=bh;
	}

	return dp;
}





/*
short *adpcm_decoder::decode_samples ( int shift, int filter, short *dp, unsigned char *input, int numberofsamples )
{
	int f0=filter_coef[filter][0];
	int f1=filter_coef[filter][1];
	
	numberofsamples = ( numberofsamples / 2 );

	for (int i=0; i< numberofsamples; i++)
	{
		//unsigned char b=ap->data[i];
		unsigned char b=input[i];
		short bl=(b&0xf)<<12,
					bh=(b>>4)<<12;

		bl=(bl>>shift)+(((l0*f0)+(l1*f1)+32)>>6);
		*dp++=bl;
		l1=l0;
		l0=bl;

		bh=(bh>>shift)+(((l0*f0)+(l1*f1)+32)>>6);
		*dp++=bh;
		l1=l0;
		l0=bh;
	}
	
	return dp;
}
*/

