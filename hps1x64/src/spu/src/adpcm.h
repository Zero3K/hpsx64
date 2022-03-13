
// I'll steal stuff from MAME/MESS

/*

    Sony PlayStation SPU (CXD2922BQ/CXD2925Q) emulator
    by pSXAuthor
    MAME adaptation by R. Belmont

*/


#ifndef _ADPCM_H_
#define _ADPCM_H_


// need to determine what type will hold samples
typedef long long ssX;


struct adpcm_packet
{
	unsigned char info,
								flags,
								data[14];
};


struct adpcm_packet_xa
{
	unsigned char data[14];
};


static const long filter_coef[5][2]=
{
	{ 0,0 },
	{ 60,0 },
	{ 115,-52 },
	{ 98,-55 },
	{ 122,-60 },
};


class adpcm_decoder
{
	long l0,l1;

public:
	adpcm_decoder()
	{
		reset();
	}

	adpcm_decoder(const adpcm_decoder &other)
	{
		operator =(other);
	}

	adpcm_decoder &operator =(const adpcm_decoder &other)
	{
		l0=other.l0;
		l1=other.l1;
		return *this;
	}

	void reset()
	{
		l0=l1=0;
	}
	
	static inline long clamp ( long sample ) { return ( ( sample > 0x7fff ) ? 0x7fff : ( ( sample < -0x8000 ) ? -0x8000 : sample ) ); }
	static inline long clamp64 ( long long sample ) { return ( ( sample > 0x7fff ) ? 0x7fff : ( ( sample < -0x8000 ) ? -0x8000 : sample ) ); }

	static inline long uclamp ( long sample ) { return ( ( sample > 0x7fff ) ? 0x7fff : ( ( sample < 0 ) ? 0 : sample ) ); }
	static inline long uclamp64 ( long long sample ) { return ( ( sample > 0x7fff ) ? 0x7fff : ( ( sample < 0 ) ? 0 : sample ) ); }

	static inline long sign_extend4 ( long value ) { return ( ( value << 28 ) >> 28 ); }
	
	signed short *decode_packet(adpcm_packet *ap, signed short *dp);
	
	// modification from mame - (cd-xa is same, but uses 28 instead of 14 samples)
	short *decode_packet_xa (unsigned char info, adpcm_packet_xa *ap, signed short *dp);
	//short *decode_samples ( int shift, int filter, short *dp, unsigned char *input, int numberofsamples );
	
	long *decode_packet32(adpcm_packet *ap, long *dp);
	
	// modification from mame - (cd-xa is same, but uses 28 instead of 14 samples)
	long *decode_packet_xa32 (unsigned char info, adpcm_packet_xa *ap, long *dp);
};

#endif

