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


#include "PS1_MDEC.h"

using namespace Playstation1;


#ifdef _DEBUG_VERSION_

#define INLINE_DEBUG_ENABLE


#define INLINE_DEBUG_WRITE
#define INLINE_DEBUG_READ
#define INLINE_DEBUG_DMA_READ
#define INLINE_DEBUG_DMA_WRITE

//#define INLINE_DEBUG_RECORD
//#define INLINE_DEBUG
//#define INLINE_DEBUG_ERROR


#endif


Debug::Log MDEC::debug;

u32* MDEC::_DebugPC;
u64* MDEC::_DebugCycleCount;
u64* MDEC::_SystemCycleCount;

MDEC* MDEC::_MDEC;

/*
MDEC::MDEC ()
{
	cout << "Running MDEC constructor...\n";
}
*/


void MDEC::Start ()
{
#ifdef INLINE_DEBUG_ENABLE
	debug.Create( "MDEC_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering MDEC::MDEC constructor";
#endif


	Reset ();
	
	_MDEC = this;
	
	// mame/mess mdec start
	mess_mdec.device_start ();
	mess_mdec.device_reset ();

#ifdef INLINE_DEBUG
	debug << "->Exiting MDEC::MDEC constructor";
#endif
}


void MDEC::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( MDEC ) );
	
	// mame/mess mdec reset
	mess_mdec.device_reset ();
}


void MDEC::Run ()
{
}



u32 MDEC::Read ( u32 Address )
{

	u32 Data;

#ifdef INLINE_DEBUG_READ
	debug << "\r\nMDEC::Read; " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "\r\nn_wordsremaining=" << dec << _MDEC->mess_mdec.n_wordsremaining << " n_wordstototal=" << _MDEC->mess_mdec.n_wordstotal << " BusyCycle=" << _MDEC->mess_mdec.n_busyuntil_cycle;
	debug << " n_0_command=" << hex << _MDEC->mess_mdec.n_0_command;
#endif

	// *** will use mame/mess device for now ***
	Data = _MDEC->mess_mdec.read ( Address );
	
#ifdef INLINE_DEBUG_READ
	debug << "; Data=" << hex << Data;
#endif

	return Data;


	/*
	switch ( Address )
	{
		case MDEC_CTRL:
			// incoming read from DATA
			
#ifdef INLINE_DEBUG_READ
	debug << "; CTRL";
#endif

			break;
			
		case MDEC_STAT:
			// incoming read from CTRL

#ifdef INLINE_DEBUG_READ
	debug << "; STAT; STP=" << Reg_STAT.STP << "; OutSync=" << Reg_STAT.OutSync << "; RGB24=" << Reg_STAT.RGB24 << "; DREQ=" << Reg_STAT.DREQ << "; InSync=" << Reg_STAT.InSync << "; FIFO=" << Reg_STAT.FIFO;
#endif
			
			return Reg_STAT.Value;
			break;
	};
	*/
}

void MDEC::Write ( u32 Address, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nMDEC::Write; " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
#endif

	// *** testing *** check if mask is a word write
	if ( Mask != 0xffffffff )
	{
		cout << "\nhps1x64 ALERT: MDEC::Write Mask=" << hex << Mask;
	}


	// *** will use mame/mess device for now ***
	_MDEC->mess_mdec.write ( Data, Address );
	return;

	/*
	switch ( Address )
	{
		case MDEC_CTRL:
			// incoming write to CTRL
			Reg_CTRL.Value = Data;
			Reg_STAT.RGB24 = Reg_CTRL.RGB24;
			Reg_STAT.STP = Reg_CTRL.STP;
			Command = Reg_CTRL.Command;
			
#ifdef INLINE_DEBUG_WRITE
	debug << "; CTRL; STP=" << Reg_CTRL.STP << "; RGB24=" << Reg_CTRL.RGB24 << "; Command=" << Reg_CTRL.Command;
#endif

			break;
			
		case MDEC_STAT:
			// incoming write to CTRL
			
			// check if we need to reset mdec
			if ( Data >> 31 )
			{
#ifdef INLINE_DEBUG_WRITE
	debug << "; RESET";
#endif

				// reset command
				Reg_CTRL.Value = 0;
				Reg_STAT.Value = 0;
			}
			
			break;
	};
	*/
}

// dma 0 - mdec in
u64 MDEC::DMA_ReadyForWrite ()
{
	// dma0 must have started
	_MDEC->DMA0_Start ();
	
	// ready for write if the command is 0x2, 0x3, but not 0x1
	
	switch ( _MDEC->mess_mdec.n_0_command >> 29 )
	{
		case 0x1:
			//return 0;
			return _MDEC->mess_mdec.n_allow_decodetransfer;
			break;
			
		case 0x2:
			return 1;
			break;
			
		 case 0x3:
			return 1;
			break;
			
		default:
			return 0;
			break;
	}

	// doesn't appear the code can reach here, should be commented out
	return _MDEC->mess_mdec.n_readyfordata_in;
	
	/*
	return true;

	if ( (*_DebugCycleCount) > BusyCycle0 )
	{
		// next time the transfer is ready after a different cycle
		BusyCycle0 = (*_DebugCycleCount) + c_MDECIn_Cycles;
		
		return true;
	}
	
	return false;
	*/
}

// dma 1 - mdec out
u64 MDEC::DMA_ReadyForRead ()
{

	// *** testing ***
	switch ( _MDEC->mess_mdec.n_0_command >> 29 )
	{
		case 0x1:
		
			if ( *_DebugCycleCount < _MDEC->mess_mdec.n_busyuntil_cycle )
			{
				// MDEC is busy until the specified cycle#
				return _MDEC->mess_mdec.n_busyuntil_cycle;
			}
			
			// MDEC is ready immediately
			return 1;
			
			break;
			
		default:
			return 0;
			break;
	}


	//return ( mess_mdec.n_0_size > 32 );
	
	
	// for now, I'll say its always ready for read
	// note: doesn't appear the code can reach here now, should be commented out
	return 1;
	
	/*
	if ( (*_DebugCycleCount) > BusyCycle1 )
	{
		// next time the transfer is ready after a different cycle
		BusyCycle1 = (*_DebugCycleCount) + c_MDECOut_Cycles;
		
		return 1;
	}
	
	return 0;
	*/
}


// returns the number of blocks read
u32 MDEC::DMA_ReadBlock ( u32* RAM, u32 Address, u32 Size )
{
#ifdef INLINE_DEBUG_DMA_READ
	//for ( int i = 0; i < Size; i++ )
	//{
		debug << "\r\nMDEC::DMA_Read; " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount;
	//}
	debug << "; Size=" << dec << Size << "; n_readindex_input=" << (u32)( _MDEC->mess_mdec.n_readindex_input );
#endif

	u32 lNumBlocksRead;

	// *** will use mame/mess device for now ***
	lNumBlocksRead = _MDEC->mess_mdec.dma_read ( Address, Size, RAM );
	
	// if no data was read, then return so
	if ( !lNumBlocksRead ) return 0;
	
	// otherwise, return the number of 32-bit words read (in this particular case, shouldn't be more than 1 BS block)
	return Size;
}


u32 MDEC::DMA_WriteBlock ( u32* RAM, u32 Address, u32 Size )
{
#ifdef INLINE_DEBUG_DMA_WRITE
	//for ( int i = 0; i < Size; i++ )
	//{
		debug << "\r\nMDEC::DMA_Write; " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount /*<< "; Writing: (hex)" << hex << RAM [ ( ( Address & 0x1fffff ) >> 2 ) + i ] << "; (dec)" << dec << RAM [ ( ( Address & 0x1fffff ) >> 2 ) + i ]*/;
	//}
	debug << "; Size=" << dec << Size;
#endif

	// *** will use mame/mess device for now ***
	_MDEC->mess_mdec.dma_write ( Address, Size, RAM );
	
	// return the amount of data that was transferred
	return Size;
}


void MDEC::DMA0_Start ()
{
	// STP should be set while the DMA0 is running
	Reg_STAT.STP = 1;
}

void MDEC::DMA0_End ()
{
	// STP should be set while the DMA0 is running
	Reg_STAT.STP = 0;
}


// this small note is from pcsx source
/* memory speed is 1 byte per MDEC_BIAS psx clock
 * That mean (PSXCLK / MDEC_BIAS) B/s
 * MDEC_BIAS = 2.0 => ~16MB/s
 * MDEC_BIAS = 3.0 => ~11MB/s
 * and so on ...
 * I guess I have 50 images in 50Hz ... (could be 25 images ?)
 * 320x240x24@50Hz => 11.52 MB/s
 * 320x240x24@60Hz => 13.824 MB/s
 * 320x240x16@50Hz => 7.68 MB/s
 * 320x240x16@60Hz => 9.216 MB/s
 * so 2.0 to 4.0 should be fine.
 */




/*
 * PlayStation Motion Decoder emulator
 *
 * Copyright 2003-2011 smf
 *
 * Thanks to Oliver Galibert for help figuring out IDCT
 *
 * Modifications by TheGangster based on Martin's psx spec and testing
 *
 */



void psxmdec_device::device_reset()
{
	n_0_command = 0;
	n_0_address = 0;
	n_0_size = 0;
	n_1_command = 0;
	n_1_status = 0;
	n_offset = 0;
	n_decoded = 0;
}


void psxmdec_device::device_start()
{
	for( int n = 0; n < 256; n++ )
	{
		/*
		p_n_clamp8[ n ] = 0;
		p_n_clamp8[ n + 256 ] = n;
		p_n_clamp8[ n + 512 ] = 255;
		*/
		p_n_clamp8[ n ] = ( 0 - 128 ) & 0xff;
		p_n_clamp8[ n + 256 ] = ( n - 128 ) & 0xff;
		p_n_clamp8[ n + 512 ] = ( 255 - 128 ) & 0xff;

		/*
		p_n_r5[ n ] = 0;
		p_n_r5[ n + 256 ] = ( n >> 3 );
		p_n_r5[ n + 512 ] = ( 255 >> 3 );
		*/
		p_n_r5[ n ] = ( ( 0 - 128 ) & 0xff ) >> 3;
		p_n_r5[ n + 256 ] = ( ( n - 128 ) & 0xff ) >> 3;
		p_n_r5[ n + 512 ] = ( ( 255 - 128 ) & 0xff ) >> 3;

		/*
		p_n_g5[ n ] = 0;
		p_n_g5[ n + 256 ] = ( n >> 3 ) << 5;
		p_n_g5[ n + 512 ] = ( 255 >> 3 ) << 5;
		*/
		p_n_g5[ n ] = ( ( ( 0 - 128 ) & 0xff ) >> 3 ) << 5;
		p_n_g5[ n + 256 ] = ( ( ( n - 128 ) & 0xff ) >> 3 ) << 5;
		p_n_g5[ n + 512 ] = ( ( ( 255 - 128 ) & 0xff ) >> 3 ) << 5;

		/*
		p_n_b5[ n ] = 0;
		p_n_b5[ n + 256 ] = ( n >> 3 ) << 10;
		p_n_b5[ n + 512 ] = ( 255 >> 3 ) << 10;
		*/
		p_n_b5[ n ] = ( ( ( 0 - 128 ) & 0xff ) >> 3 ) << 10;
		p_n_b5[ n + 256 ] = ( ( ( n - 128 ) & 0xff ) >> 3 ) << 10;
		p_n_b5[ n + 512 ] = ( ( ( 255 - 128 ) & 0xff ) >> 3 ) << 10;
	}
}

#ifdef UNUSED_FUNCTION
void psxmdec_device::device_post_load()
{
	mdec_cos_precalc();
}

inline void psxwriteword( u32 *p_n_psxram, u32 n_address, u16 n_data )
{
	//*( (UINT16 *)( (UINT8 *)p_n_psxram + WORD_XOR_LE( n_address ) ) ) = n_data;
	*( (u16 *)( (u8 *)p_n_psxram + ( n_address ) ) ) = n_data;
}

inline u16 psxreadword( u32 *p_n_psxram, u32 n_address )
{
	//return *( (UINT16 *)( (UINT8 *)p_n_psxram + WORD_XOR_LE( n_address ) ) );
	return *( (u16 *)( (u8 *)p_n_psxram + ( n_address ) ) );
}
#endif


static const u32 m_p_n_mdec_zigzag[ DCTSIZE2 ] =
{
	 0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};


void psxmdec_device::mdec_idct( s32 *p_n_src, s32 *p_n_dst )
{
	s32 *p_n_precalc = p_n_cos_precalc;

	for( UINT32 n_yx = 0; n_yx < DCTSIZE2; n_yx++ )
	{
		s32 p_n_z[ 8 ];
		s32 *p_n_data = p_n_src;

		memset( p_n_z, 0, sizeof( p_n_z ) );

		for( u32 n_vu = 0; n_vu < DCTSIZE2 / 8; n_vu++ )
		{
			p_n_z[ 0 ] += p_n_data[ 0 ] * p_n_precalc[ 0 ];
			p_n_z[ 1 ] += p_n_data[ 1 ] * p_n_precalc[ 1 ];
			p_n_z[ 2 ] += p_n_data[ 2 ] * p_n_precalc[ 2 ];
			p_n_z[ 3 ] += p_n_data[ 3 ] * p_n_precalc[ 3 ];
			p_n_z[ 4 ] += p_n_data[ 4 ] * p_n_precalc[ 4 ];
			p_n_z[ 5 ] += p_n_data[ 5 ] * p_n_precalc[ 5 ];
			p_n_z[ 6 ] += p_n_data[ 6 ] * p_n_precalc[ 6 ];
			p_n_z[ 7 ] += p_n_data[ 7 ] * p_n_precalc[ 7 ];
			p_n_data += 8;
			p_n_precalc += 8;
		}

		*( p_n_dst++ ) = ( p_n_z[ 0 ] + p_n_z[ 1 ] + p_n_z[ 2 ] + p_n_z[ 3 ] +
			p_n_z[ 4 ] + p_n_z[ 5 ] + p_n_z[ 6 ] + p_n_z[ 7 ] ) >> ( MDEC_COS_PRECALC_BITS + 2 );
	}
}

inline u16 mdec_unpack_run( u16 n_packed )
{
	return n_packed >> 10;
}

inline s32 mdec_unpack_val( u16 n_packed )
{
	return ( ( (s32)n_packed ) << 22 ) >> 22;
}

// *** todo *** need to fix this so it only unpacks one block (like cr, cb, y, etc). just return with source index/pointer advanced
//UINT32 psxmdec_device::mdec_unpack( UINT32 n_address, u32* RAM )
void psxmdec_device::rl_decode_block( u32& SrcOffset, s32* blk, u16* src, s32* qt )
{
	//psx_state *p_psx = machine().driver_data<psx_state>();
	
	// data might not be straight from ram
	// fix ram pointer
	//UINT32 *p_n_psxram = p_psx->m_p_n_psxram;
	//UINT32 *p_n_psxram = RAM;	//p_psx->m_p_n_psxram;

	u8 n_z;
	s32 n_qscale;
	u16 n_packed;
	
	s32 n_val;
	
	// set the destination for the unpacked data
	s32 p_n_unpacked[ 64 ];
	
	// set the quantization table
	s32 *p_n_q;
	//p_n_q = p_n_quantize_uv;
	p_n_q = qt;
	
	// set the destination for the final unpacked data
	s32 *p_n_block;
	//p_n_block = m_p_n_unpacked;
	p_n_block = blk;

	//for( UINT32 n_block = 0; n_block < 6; n_block++ )
	//{
		memset( p_n_unpacked, 0, sizeof( p_n_unpacked ) );

		//if( n_block == 2 )
		//{
		//	p_n_q = p_n_quantize_y;
		//}
		
#ifdef INLINE_DEBUG_RECORD
	MDEC::debug << "\r\nstartblock";
#endif

		//n_packed = src [ SrcOffset++ ];
		n_packed = src [ SrcOffset++ & c_n_inputfifomask ];
		//n_address += 2;
		
#ifdef INLINE_DEBUG_RECORD
	MDEC::debug << "\r\n" << hex << n_packed;
#endif

		
		//if( n_packed == 0xfe00 )
		while( n_packed == 0xfe00 )
		{
			n_packed = src [ SrcOffset++ & c_n_inputfifomask ];
			//return;
			
#ifdef INLINE_DEBUG_RECORD
	MDEC::debug << "\r\n" << hex << n_packed;
#endif
		}
		
		

		n_qscale = mdec_unpack_run( n_packed );
		//p_n_unpacked[ 0 ] = mdec_unpack_val( n_packed ) * p_n_q[ 0 ];
		n_val = mdec_unpack_val( n_packed ) * p_n_q[ 0 ];

		n_z = 0;
		//for( ;; )
		while ( n_z < 64 )
		{
			// store the value
			p_n_unpacked[ m_p_n_mdec_zigzag[ n_z ] ] = n_val;
			
			//n_packed = src [ SrcOffset++ ];
			n_packed = src [ SrcOffset++ & c_n_inputfifomask ];
			//n_address += 2;

#ifdef INLINE_DEBUG_RECORD
	MDEC::debug << "\r\n" << hex << n_packed;
#endif

			/*
			if( n_packed == 0xfe00 )
			{
				break;
			}
			*/
			
			// get the amount to run
			n_z += mdec_unpack_run( n_packed ) + 1;
			
			/*
			if( n_z > 63 )
			{
				break;
			}
			*/
			
			// get the value
			//p_n_unpacked[ m_p_n_mdec_zigzag[ n_z ] ] = ( mdec_unpack_val( n_packed ) * p_n_q[ n_z ] * n_qscale ) / 8;
			n_val = ( mdec_unpack_val( n_packed ) * p_n_q[ n_z ] * n_qscale ) / 8;
		}
		mdec_idct( p_n_unpacked, p_n_block );
		
		// must decode/unpack only one block at a time
		//p_n_block += DCTSIZE2;
	//}
	
#ifdef INLINE_DEBUG_RECORD
	MDEC::debug << "\r\ncleanup";
#endif

	
	if ( SrcOffset < n_writeindex_input )
	{
		n_packed = src [ SrcOffset & c_n_inputfifomask ];
		//n_address += 2;
		
#ifdef INLINE_DEBUG_RECORD
	MDEC::debug << "\r\n" << hex << n_packed;
#endif

		//if( n_packed == 0xfe00 )
		while( n_packed == 0xfe00 && SrcOffset < n_writeindex_input )
		{
			SrcOffset++;
			n_packed = src [ SrcOffset & c_n_inputfifomask ];
			//return;
			
#ifdef INLINE_DEBUG_RECORD
	MDEC::debug << "\r\n" << hex << n_packed;
#endif
		}
		
#ifdef INLINE_DEBUG_RECORD
	MDEC::debug << "\r\nendblock";
#endif

		//if ( n_packed != 0xfe00 && SrcOffset < n_writeindex_input )
		//{
		//	SrcOffset--;
		//}
	}
	
	
	// return the updated pointer into the source data
	//return n_address;
	//return src;
}


// returns updated pointer into source data
u32 psxmdec_device::decode_colored_macroblock24 ( u32 SrcOffset, u16* src )
{
	rl_decode_block ( SrcOffset, p_n_crblk, src, p_n_quantize_uv );
	rl_decode_block ( SrcOffset, p_n_cbblk, src, p_n_quantize_uv );
	rl_decode_block ( SrcOffset, & ( p_n_yblk [ 0 ] ), src, p_n_quantize_y );
	rl_decode_block ( SrcOffset, & ( p_n_yblk [ 64 ] ), src, p_n_quantize_y );
	rl_decode_block ( SrcOffset, & ( p_n_yblk [ 128 ] ), src, p_n_quantize_y );
	rl_decode_block ( SrcOffset, & ( p_n_yblk [ 192 ] ), src, p_n_quantize_y );
	
	mdec_yuv2_to_rgb24();
	
	return SrcOffset;
}


u32 psxmdec_device::decode_colored_macroblock15 ( u32 SrcOffset, u16* src )
{
	rl_decode_block ( SrcOffset, p_n_crblk, src, p_n_quantize_uv );
	rl_decode_block ( SrcOffset, p_n_cbblk, src, p_n_quantize_uv );
	rl_decode_block ( SrcOffset, & ( p_n_yblk [ 0 ] ), src, p_n_quantize_y );
	rl_decode_block ( SrcOffset, & ( p_n_yblk [ 64 ] ), src, p_n_quantize_y );
	rl_decode_block ( SrcOffset, & ( p_n_yblk [ 128 ] ), src, p_n_quantize_y );
	rl_decode_block ( SrcOffset, & ( p_n_yblk [ 192 ] ), src, p_n_quantize_y );
	
	mdec_yuv2_to_rgb15();
	
	return SrcOffset;
}



u32 psxmdec_device::decode_monochrome_macroblock8 ( u32 SrcOffset, u16* src )
{
	rl_decode_block ( SrcOffset, & ( p_n_yblk [ 0 ] ), src, p_n_quantize_y );
	
	mdec_y_to_mono8();
	
	return SrcOffset;
}


u32 psxmdec_device::decode_monochrome_macroblock4 ( u32 SrcOffset, u16* src )
{
	rl_decode_block ( SrcOffset, & ( p_n_yblk [ 0 ] ), src, p_n_quantize_y );
	
	mdec_y_to_mono4();
	
	return SrcOffset;
}


inline s32 mdec_cr_to_r( s32 n_cr )
{
	return ( 1435 * n_cr ) >> 10;
}

inline s32 mdec_cr_to_g( s32 n_cr )
{
	return ( -731 * n_cr ) >> 10;
}

inline s32 mdec_cb_to_g( s32 n_cb )
{
	return ( -351 * n_cb ) >> 10;
}

inline s32 mdec_cb_to_b( s32 n_cb )
{
	return ( 1814 * n_cb ) >> 10;
}

u16 psxmdec_device::mdec_clamp_r5( s32 n_r ) const
{
	return p_n_r5[ n_r + 128 + 256 ];
}

u16 psxmdec_device::mdec_clamp_g5( s32 n_g ) const
{
	return p_n_g5[ n_g + 128 + 256 ];
}

u16 psxmdec_device::mdec_clamp_b5( s32 n_b ) const
{
	return p_n_b5[ n_b + 128 + 256 ];
}

void psxmdec_device::mdec_makergb15( u32 n_address, s32 n_r, s32 n_g, s32 n_b, s32 *p_n_y, u16 n_stp, u32 XorValue )
{
	//p_n_output[ WORD_XOR_LE( n_address + 0 ) / 2 ] = n_stp |
	p_n_output[ ( n_address + 0 ) / 2 ] = ( n_stp |
		mdec_clamp_r5( p_n_y[ 0 ] + n_r ) |
		mdec_clamp_g5( p_n_y[ 0 ] + n_g ) |
		mdec_clamp_b5( p_n_y[ 0 ] + n_b ) ) ^ XorValue;

	//p_n_output[ WORD_XOR_LE( n_address + 2 ) / 2 ] = n_stp |
	p_n_output[ ( n_address + 2 ) / 2 ] = ( n_stp |
		mdec_clamp_r5( p_n_y[ 1 ] + n_r ) |
		mdec_clamp_g5( p_n_y[ 1 ] + n_g ) |
		mdec_clamp_b5( p_n_y[ 1 ] + n_b ) ) ^ XorValue;
}

void psxmdec_device::mdec_yuv2_to_rgb15()
{
	s32 n_r;
	s32 n_g;
	s32 n_b;
	s32 n_cb;
	s32 n_cr;
	s32 *p_n_cb;
	s32 *p_n_cr;
	s32 *p_n_y;
	u32 n_x;
	u32 n_y;
	u32 n_z;
	u16 n_stp;
	int n_address = 0;

	u32 XorValue;
	
	// check if data is signed or unsigned
	XorValue = 0;
	if ( ! ( ( n_0_command >> 26 ) & 1 ) )
	{
		// unsigned //
		XorValue = ( 1 << 14 ) + ( 1 << 9 ) + ( 1 << 4 );
	}
	
	
	if( ( n_0_command & ( 1L << 25 ) ) != 0 )
	{
		n_stp = 0x8000;
	}
	else
	{
		n_stp = 0x0000;
	}

	//p_n_cr = &m_p_n_unpacked[ 0 ];
	//p_n_cb = &m_p_n_unpacked[ DCTSIZE2 ];
	//p_n_y = &m_p_n_unpacked[ DCTSIZE2 * 2 ];
	p_n_cr = p_n_crblk;
	p_n_cb = p_n_cbblk;
	p_n_y = p_n_yblk;

	for( n_z = 0; n_z < 2; n_z++ )
	{
		for( n_y = 0; n_y < 4; n_y++ )
		{
			for( n_x = 0; n_x < 4; n_x++ )
			{
				n_cr = *( p_n_cr );
				n_cb = *( p_n_cb );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb15( ( n_address +  0 ), n_r, n_g, n_b, p_n_y, n_stp, XorValue );
				mdec_makergb15( ( n_address + 32 ), n_r, n_g, n_b, p_n_y + 8, n_stp, XorValue );

				n_cr = *( p_n_cr + 4 );
				n_cb = *( p_n_cb + 4 );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb15( ( n_address + 16 ), n_r, n_g, n_b, p_n_y + DCTSIZE2, n_stp, XorValue );
				mdec_makergb15( ( n_address + 48 ), n_r, n_g, n_b, p_n_y + DCTSIZE2 + 8, n_stp, XorValue );

				p_n_cr++;
				p_n_cb++;
				p_n_y += 2;
				
				n_address += 4;
			}
			
			p_n_cr += 4;
			p_n_cb += 4;
			p_n_y += 8;
			
			n_address += 48;
		}
		p_n_y += DCTSIZE2;
	}
	n_decoded = ( 16 * 16 ) / 2;
}

UINT16 psxmdec_device::mdec_clamp8( s32 n_r ) const
{
	return p_n_clamp8[ n_r + 128 + 256 ];
}

void psxmdec_device::mdec_makergb24( u32 n_address, s32 n_r, s32 n_g, s32 n_b, s32 *p_n_y, u32 n_stp, u32 XorValue )
{
	p_n_output[ ( n_address + 0 ) / 2 ] = ( ( mdec_clamp8( p_n_y[ 0 ] + n_g ) << 8 ) | mdec_clamp8( p_n_y[ 0 ] + n_r ) ) ^ XorValue;
	p_n_output[ ( n_address + 2 ) / 2 ] = ( ( mdec_clamp8( p_n_y[ 1 ] + n_r ) << 8 ) | mdec_clamp8( p_n_y[ 0 ] + n_b ) ) ^ XorValue;
	p_n_output[ ( n_address + 4 ) / 2 ] = ( ( mdec_clamp8( p_n_y[ 1 ] + n_b ) << 8 ) | mdec_clamp8( p_n_y[ 1 ] + n_g ) ) ^ XorValue;
}

void psxmdec_device::mdec_yuv2_to_rgb24()
{
	s32 n_r;
	s32 n_g;
	s32 n_b;
	s32 n_cb;
	s32 n_cr;
	s32 *p_n_cb;
	s32 *p_n_cr;
	s32 *p_n_y;
	u32 n_x;
	u32 n_y;
	u32 n_z;
	u32 n_stp;
	int n_address = 0;
	
	u32 XorValue;
	
	// check if data is signed or unsigned
	XorValue = 0;
	if ( ! ( ( n_0_command >> 26 ) & 1 ) )
	{
		// unsigned //
		XorValue = 0x8080;
	}

	/*
	if( ( n_0_command & ( 1L << 25 ) ) != 0 )
	{
		n_stp = 0x80008000;
	}
	else
	{
		n_stp = 0x00000000;
	}
	*/
	n_stp = 0x00000000;

	// specify where the unpacked data is
	//p_n_cr = &m_p_n_unpacked[ 0 ];
	//p_n_cb = &m_p_n_unpacked[ DCTSIZE2 ];
	//p_n_y = &m_p_n_unpacked[ DCTSIZE2 * 2 ];
	p_n_cr = p_n_crblk;
	p_n_cb = p_n_cbblk;
	p_n_y = p_n_yblk;

	for( n_z = 0; n_z < 2; n_z++ )
	{
		for( n_y = 0; n_y < 4; n_y++ )
		{
			for( n_x = 0; n_x < 4; n_x++ )
			{
				n_cr = *( p_n_cr );
				n_cb = *( p_n_cb );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb24( ( n_address +  0 ), n_r, n_g, n_b, p_n_y, n_stp, XorValue );
				mdec_makergb24( ( n_address + 48 ), n_r, n_g, n_b, p_n_y + 8, n_stp, XorValue );

				n_cr = *( p_n_cr + 4 );
				n_cb = *( p_n_cb + 4 );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb24( ( n_address + 24 ), n_r, n_g, n_b, p_n_y + DCTSIZE2, n_stp, XorValue );
				mdec_makergb24( ( n_address + 72 ), n_r, n_g, n_b, p_n_y + DCTSIZE2 + 8, n_stp, XorValue );

				p_n_cr++;
				p_n_cb++;
				p_n_y += 2;
				
				n_address += 6;
			}
			p_n_cr += 4;
			p_n_cb += 4;
			p_n_y += 8;
			
			n_address += 72;
		}
		p_n_y += DCTSIZE2;
	}
	n_decoded = ( 24 * 16 ) / 2;
}




void psxmdec_device::mdec_y_to_mono8()
{
	s32 *p_n_y;
	u32 n_x;
	
	u32 Y;
	u32 XorValue;
	
	// check if data is signed or unsigned
	XorValue = 0;
	if ( ! ( ( n_0_command >> 26 ) & 1 ) )
	{
		// unsigned //
		XorValue = 0x80;
	}
	
	p_n_y = p_n_yblk;
	
	
	for( n_x = 0; n_x < 64; n_x++ )
	{
		Y = p_n_y [ n_x ];
		
		// check if unsigned, and if so xor 0x80
		Y ^= XorValue;
		
		((u8*)p_n_output)[ n_x ] = Y;
	}
	
	n_decoded = 64;
}

void psxmdec_device::mdec_y_to_mono4()
{
	s32 *p_n_y;
	u32 n_x;
	
	u32 Pack;

	u32 Y;
	u32 XorValue;
	
	// check if data is signed or unsigned
	XorValue = 0;
	if ( ! ( ( n_0_command >> 26 ) & 1 ) )
	{
		// unsigned //
		XorValue = 0x80;
	}
	
	
	p_n_y = p_n_yblk;
	
	
	n_x = 0;
	
	while ( n_x < 64 )
	{
		Y = p_n_y [ n_x + 0 ];
		
		// check if unsigned, and if so xor 0x80
		Y ^= XorValue;
		
		Pack = ( Y >> 4 ) & 0xf;
		
		Y = p_n_y [ n_x + 1 ];
		
		// check if unsigned, and if so xor 0x80
		Y ^= XorValue;
		
		Pack |= Y & 0xf0;
		
		((u8*)p_n_output)[ n_x ] = Pack;
		
		n_x += 2;
	}
	
	n_decoded = 32;
}






// *** PUBLIC FUNCTIONS *** //

void psxmdec_device::dma_write( u32 n_address, s32 n_size, u32* RAM )
{
	u16 *p_n_mem_ptr;
	
	//psx_state *p_psx = machine().driver_data<psx_state>();
	// fix ram pointer
	//UINT32 *p_n_psxram = p_psx->m_p_n_psxram;
	u32 *p_n_psxram = RAM;
	//int n_index;
	
	// get physical ps1 address
	n_address &= 0x1fffff;

	//verboselog( machine(), 2, "mdec0_write( %08x, %08x )\n", n_address, n_size );

	switch( n_0_command >> 29 )
	{
		case 0x1:
			//verboselog( machine(), 1, "mdec decode %08x %08x %08x\n", n_0_command, n_address, n_size );
			
			//n_0_address = n_address;
			//n_0_size = n_size * 4;
			//n_1_status |= ( 1 << 29 );
			
			// put this here just for now
			//p_n_mem_ptr = (u16*) & ( RAM [ n_0_address >> 2 ] );
			
			// get pointer into the input data
			p_n_mem_ptr = (u16*) & ( p_n_psxram [ n_address >> 2 ] );
			
			// transfer into input fifo
			for ( int i = 0; i < ( c_n_mdec_blocksize * 2 ); i++ ) p_n_inputfifo [ n_writeindex_input++ & c_n_inputfifomask ] = *p_n_mem_ptr++;
			
			// that was 128 compressed bytes
			n_compressed_bytes += 128;

			
			break;
			
		case 0x2:
			//verboselog( machine(), 1, "mdec quantize table %08x %08x %08x\n", n_0_command, n_address, n_size );
			upload_qtable ( & ( p_n_psxram [ n_address >> 2 ] ), n_size );
			
			// loaded all the data
			n_readyfordata_in = 0;
			
			break;
			
		case 0x3:
			//verboselog( machine(), 1, "mdec cosine table %08x %08x %08x\n", n_0_command, n_address, n_size );
			upload_ctable ( & ( p_n_psxram [ n_address >> 2 ] ), n_size );
			
			// loaded all the data
			n_readyfordata_in = 0;
			
			break;
			
		default:
			//verboselog( machine(), 0, "mdec unknown command %08x %08x %08x\n", n_0_command, n_address, n_size );
			cout << "\nmdec unknown command " << n_0_command << " " << n_address << " " << n_size << "\n";
			
#ifdef INLINE_DEBUG_ERROR
	MDEC::debug << "\r\n" << hex << setw( 8 ) << *MDEC::_DebugPC << " " << dec << *MDEC::_DebugCycleCount << "mdec unknown command " << (u32)n_0_command << " " << (u32)n_address << " " << (u32)n_size;
#endif

			break;
	}
}


// returns the number of blocks (32 word units) read
int psxmdec_device::dma_read( u32 n_address, s32 n_size, u32* RAM )
{
#ifdef INLINE_DEBUG_DMA_READ
	MDEC::debug << "\r\ndma_read";
	MDEC::debug << " n_address=" << hex << n_address;
	MDEC::debug << " n_size=" << dec << n_size;
#endif

	//psx_state *p_psx = machine().driver_data<psx_state>();
	
	// the number of blocks read by dma
	u32 n_blocks_read = 0;
	
	// fix ram pointer
	//UINT32 *p_n_psxram = p_psx->m_p_n_psxram;
	u32 *p_n_psxram = RAM;	//p_psx->m_p_n_psxram;
	
	// get physical ps1 address
	n_address &= 0x1fffff;
	
	//UINT32 n_this;
	//UINT32 n_nextaddress;
	
	u32 n_nextindex;
	u32 n_previousindex;
	
	u32 n_words_to_transfer;
	
	u32 n_wordsdecoded = 0;
	//u32 n_compressed_bytes = 0;
	
	u16* p_n_ptr = (u16*) & ( RAM [ n_0_address >> 2 ] );
	
	// mdec is now busy?
	n_1_status |= ( 1 << 29 );
	
	// source data pointer
	u16* src = (u16*) & ( RAM [ n_0_address >> 2 ] );
	
#ifdef INLINE_DEBUG_DMA_READ
	MDEC::debug << " n_decoded=" << n_decoded;
	MDEC::debug << " n_size=" << n_size;
	MDEC::debug << " n_readindex_output=" << n_readindex_output;
#endif

	// check if the number of words decoded is greater than zero
	while ( n_decoded > 0 )
	{
#ifdef INLINE_DEBUG_DMA_READ
	MDEC::debug << " whileloop";
	MDEC::debug << " n_decoded=" << n_decoded;
	MDEC::debug << " n_size=" << n_size;
	MDEC::debug << " n_readindex_output=" << n_readindex_output;
#endif
		// transfer the remainder of what was decoded //
		
		// transfer the lesser value between the last amount of data decoded and the amount requested
		n_words_to_transfer = ( n_decoded > n_size ) ? n_size : n_decoded;
		
		// output the data to RAM
		memcpy( (UINT8 *)p_n_psxram + n_address, (UINT8 *)p_n_output + n_readindex_output, n_words_to_transfer * 4 );
		
		// update total number of word decoded
		n_wordsdecoded += n_words_to_transfer;
		
		n_size -= n_words_to_transfer;
		n_decoded -= n_words_to_transfer;
		
		n_address += ( n_words_to_transfer * 4 );
		n_readindex_output += ( n_words_to_transfer * 4 );
		
		if ( ( !n_size ) && ( n_decoded ) )
		{
			n_blocks_read = n_wordsdecoded / c_n_mdec_blocksize;
			return n_blocks_read;
		}
	}
	
	// initialize the number of words decoded
	n_decoded = 0;

#ifdef INLINE_DEBUG_DMA_READ
		MDEC::debug << ";(before) n_readindex_input=" << n_readindex_input << "; n_writeindex_input=" << n_writeindex_input << "; n_decoded=" << dec << (u32)n_decoded << "; n_address=" << (u32)n_address << "; n_blocks_read=" << (u32)n_blocks_read << " n_busyuntil_cycle=" << n_busyuntil_cycle;
#endif

	//verboselog( machine(), 2, "mdec1_read( %08x, %08x )\n", n_address, n_size );
	if( ( ( n_0_command >> 29 ) == 1 ) /*&& n_0_size != 0*/ )
	{
		//n_index = 0;
		//n_wordsdecoded = 0;
		
		//while ( n_wordsdecoded < n_size )
		if ( n_wordsdecoded < n_size )
		{
			if ( n_readindex_input > n_writeindex_input )
			{
				cout << "\nhps1x64: MDEC read index has over taken write index.\n";
				cout << "\nn_readindex_input=" << dec << n_readindex_input << " n_writeindex_input=" << n_writeindex_input << "\n";
				
#ifdef INLINE_DEBUG_DMA_READ
				MDEC::debug << "\r\nhps1x64: MDEC read index has over taken write index.";
				MDEC::debug << "\r\nn_readindex_input=" << dec << n_readindex_input << " n_writeindex_input=" << n_writeindex_input;
#endif
			}
			
			// *** check block *** //
			// the run function will decode one macroblock at a time since it can't do anything during the time it takes to process
			if ( n_readindex_input > ( n_writeindex_input - ((s32) ( c_n_inputfifosize / 2 ) ) ) )
			{
				// load new block in //
				
				//for ( int j = 0; j < ( c_n_inputfifoblocks / 2 ); j++ )
				while ( n_writeindex_input < ( n_readindex_input + c_n_inputfifosize - ( c_n_mdec_blocksize * 2 * ( c_n_inputfifoblocks / 2 ) ) ) )
				{
					// check if there is data to transfer for MDECin dma#0
					if ( !Dma::_DMA->isActive ( 0 ) )
					{
						// no more data to transfer into MDEC
						break;
					}
					
					//p_n_mem_ptr = (u16*) Dma::_DMA->DMA0_ReadBlock ();
					// set that we are ready for deconding dma transfer
					n_allow_decodetransfer = 1;
					Dma::_DMA->Transfer ( 0, true );
					n_allow_decodetransfer = 0;
					
					/*
					if ( p_n_mem_ptr )
					{
#ifdef INLINE_DEBUG_RECORD
				MDEC::debug << "\r\nload block";
#endif

						// load in the new block at that position
						// note: for now, data is being transferred in half-words
						//for ( int i = 0; i < ( c_n_inputfifosize / 2 ); i++ ) p_n_inputfifo [ n_writeindex_input++ & c_n_inputfifomask ] = *p_n_mem_ptr++;
						for ( int i = 0; i < ( c_n_mdec_blocksize * 2 ); i++ ) p_n_inputfifo [ n_writeindex_input++ & c_n_inputfifomask ] = *p_n_mem_ptr++;
						
						// that was 128 compressed bytes
						n_compressed_bytes += 128;
					}
					else
					{
#ifdef INLINE_DEBUG_RECORD
				MDEC::debug << "\r\nno more blocks to load";
#endif

						//for ( int i = 0; i < ( c_n_mdec_blocksize * 2 ); i++ ) p_n_inputfifo [ n_writeindex_input++ & c_n_inputfifomask ] = 0;
						
						// unable to load anymore data from dma0
						break;
					}
					*/
					
				}
			}
			
			// n_0_address is the address to output the data to
			//n_index = n_0_address >> 1;
			
			n_nextindex = -1;
			//n_previousindex = n_readindex_input;
			
			// n_index is the index of the current encoded halfword
			// n_0_size is the size of the compressed data in bytes
			//if ( ( n_readindex_input << 1 ) < n_0_size )
			if ( n_readindex_input < n_writeindex_input )
			{
#ifdef INLINE_DEBUG_RECORD
				MDEC::debug << "\r\ndecode macro block";
#endif

				// there is more compressed data remaining //
				
				//if( ( n_0_command & ( 1L << 27 ) ) != 0 )
				// n_0_command is the same as the MDEC CTRL register
				switch ( ( n_0_command >> 27 ) & 0x3 )
				{
					case 0:
						// 4-bit color //
						
						//n_nextindex = decode_monochrome_macroblock4 ( n_readindex_input, src );
						n_nextindex = decode_monochrome_macroblock4 ( n_readindex_input, p_n_inputfifo );
						break;
						
					case 1:
						// 8-bit color //
						
						//n_nextindex = decode_monochrome_macroblock8 ( n_readindex_input, src );
						n_nextindex = decode_monochrome_macroblock8 ( n_readindex_input, p_n_inputfifo );
						break;
						
					case 2:
						// 24-bit color //
						
						//n_nextindex = decode_colored_macroblock24 ( n_readindex_input, src );
						n_nextindex = decode_colored_macroblock24 ( n_readindex_input, p_n_inputfifo );
						break;
						
					case 3:
						// 15-bit color //
						
						// decode the macroblock
						// pass the starting half-word offset for source, the source data pointer and the output data pointer for macro block
						//n_nextindex = decode_colored_macroblock15 ( n_readindex_input, src );
						n_nextindex = decode_colored_macroblock15 ( n_readindex_input, p_n_inputfifo );
						break;
				}
				
				
				// check if we read past the write index
				if ( n_nextindex > n_writeindex_input )
				{
#ifdef INLINE_DEBUG_DMA_READ
				MDEC::debug << "\r\nhps1x64 problem: n_nextindex > n_writeindex_input; n_nextindex=" << dec << n_nextindex << " n_writeindex=" << n_writeindex_input;
#endif

					// this should not happen at all
					cout << "\nhps1x64 problem: n_nextindex > n_writeindex_input";
					
					// still decode, just ran out of data //
					
					// backtrack
					n_nextindex = -1;
					
					// the read index should equal the write index
					//n_readindex_input = n_writeindex_input;
					
					// did not read a full macro block
					n_decoded = 0;
					n_blocks_read = 0;
				}
				else
				{
					// copy the data to ram
					
					// note: don't transfer more than was requested
					n_words_to_transfer = ( n_decoded > n_size ) ? n_size : n_decoded;
					
					// update the number of words decoded
					//n_address += n_decoded * 4;
					//n_wordsdecoded += n_decoded;
					n_wordsdecoded += n_words_to_transfer;
					
					// doing n_decoded * 4 gives the number of bytes
					// n_decoded holds the number of 32-bit words
					//memcpy( (UINT8 *)p_n_psxram + n_address, (UINT8 *)p_n_output, n_decoded * 4 );
					memcpy( (UINT8 *)p_n_psxram + n_address, (UINT8 *)p_n_output, n_words_to_transfer * 4 );
					
					//n_blocks_read = n_decoded / c_n_mdec_blocksize;
					//n_blocks_read = n_words_to_transfer / c_n_mdec_blocksize;
					n_blocks_read = n_wordsdecoded / c_n_mdec_blocksize;
					
					// update the remaining number of decoded words to transfer
					n_decoded -= n_words_to_transfer;
					
					// update the amount remaining to be decoded
					n_size -= n_words_to_transfer;
					
					// update index in output buffer
					n_readindex_output = n_words_to_transfer * 4;
				}
				
				
			}
			else
			{
				// there is no more compressed data remaining //
				cout << "\nhps1x64 problem: MDEC: there is no more compressed data remaining.\n";
				
#ifdef INLINE_DEBUG_DMA_READ
				MDEC::debug << "\r\nhps1x64 problem: MDEC: there is no more compressed data remaining.";
#endif

			}
			
			if ( n_nextindex != -1 )
			{
#ifdef INLINE_DEBUG_RECORD
				MDEC::debug << "\r\nn_nextindex != -1; n_nextindex=" << dec << n_nextindex;
#endif
				//if ( ( n_nextindex - n_readindex_input ) > ( c_n_inputfifosize / 2 ) ) cout << "\nhps1x64: MDEC reading more than it can chew.\n";
				
				//if ( n_size >= n_decoded )
				//{
					// update where data will be output to next
					n_readindex_input = n_nextindex;
				//}
			}
			else
			{
#ifdef INLINE_DEBUG_RECORD
				MDEC::debug << "\r\nn_nextindex == -1";
#endif

				// data in fifo is not full
				//n_1_status &= ~( 1 << 30 );
				
				// data out fifo is empty
				//n_1_status |= ( 1 << 31 );
				
				// last block was Y4
				//n_1_status &= ~( 0x7 << 16 );
				//n_1_status |= ( 3 << 16 );
				
				// *** testing *** clear out status
				//n_1_status = 0;
			}
			
		}
		
		// update number of compressed words remaining
		n_wordsremaining = n_wordstotal - ( n_readindex_input >> 1 ) - 1;
		if ( n_wordsremaining < 0 ) n_wordsremaining = 0xffff;
		n_1_status = ( n_1_status & ~( 0xffff ) ) | ( n_wordsremaining & 0xffff );
		
#ifdef INLINE_DEBUG_RECORD
				MDEC::debug << "\r\nn_wordsremaining=" << dec << n_wordsremaining << " n_wordstotal=" << dec << n_wordstotal;
#endif

	}
	else
	{
		// wrong mode is set or something //
		cout << "\nhps1x64 problem: MDEC: wrong mode is set for reading MDEC.\n";
		
#ifdef INLINE_DEBUG_DMA_READ
		MDEC::debug << "\r\nhps1x64 problem: MDEC: wrong mode is set for reading MDEC.";
#endif
	}
	
	// mdec is going to be busy for the time it was processing data
	// note: it looks like the time it takes to decompress goes by the compressed size
	n_cycles_used = (u64) ( ( n_wordsdecoded * 4 ) * c_n_mdec_cyclesperbyte );
	//n_busyuntil_cycle = *MDEC::_DebugCycleCount + n_cycles_used;
	n_busyuntil_cycle = *MDEC::_DebugCycleCount + n_cycles_used + 1;
	//n_busyuntil_cycle = *MDEC::_DebugCycleCount + (u64) ( n_compressed_bytes * c_n_mdec_cyclesperbyte );
	
#ifdef INLINE_DEBUG_DMA_READ
		MDEC::debug << ";(after) n_readindex_input=" << n_readindex_input << "; n_writeindex_input=" << n_writeindex_input << "; n_decoded=" << dec << (u32)n_decoded << "; n_address=" << (u32)n_address << "; n_blocks_read=" << (u32)n_blocks_read << "; n_busyuntil_cycle=" << n_busyuntil_cycle;
#endif

	n_blocks_read = n_wordsdecoded / c_n_mdec_blocksize;
	
	// return the number of blocks read
	return n_blocks_read;
	
	// *** testing ***
	// this is incorrect because the MDEC must stay busy until the output data is transferred
	// *** todo *** this will be fixed with the other planned modifications
	//n_1_status &= ~( 1 << 29 );
}

//WRITE32_MEMBER( psxmdec_device::write )
void psxmdec_device::write ( u32 data, u32 address )
{
	switch( address )
	{
		case 0x1f801820:


			//verboselog( machine(), 2, "mdec 0 command %08x\n", data );
			n_0_command = data;
			
			// command bits 25-28 ALWAYS are reflected in status bits 23-26 unconditionally
			// update status
			// set stp - bit 25 in command, but bit 23 in status
			// set data out signed - bit 26 in command, but bit 24 in status
			// set data out depth - bits 27-28 in command, but bits 25-26 in status
			n_1_status = ( n_1_status & ~( 0xf << 23 ) ) | ( ( n_0_command >> 2 ) & ( 0xf << 23 ) );
			
			// also set status bits 0-15 from command bits 0-15 unconditionally
			n_1_status = ( n_1_status & ~( 0xffff ) ) | ( ( n_0_command ) & ( 0xffff ) );
			
			// initialize command
			switch ( n_0_command >> 29 )
			{
				case 0x1:
				
					// decode //
					
					//verboselog( machine(), 1, "mdec decode %08x %08x %08x\n", n_0_command, n_address, n_size );
					
					// initialize the decoding
					//n_index_global = 0;
					
					
					// *** start block *** //
					
					// initialize input fifo
					n_readindex_input = 0;
					n_writeindex_input = 0;
					
					
					// get the number of compressed words minus some to decompress
					n_wordstotal = n_0_command & 0xffff;
					
					// *** testing ***
					n_0_size = n_wordstotal * 4;
					
					
					n_compressed_bytes = 0;
					
					/*
					while ( n_writeindex_input < ( n_readindex_input + c_n_inputfifosize - ( c_n_mdec_blocksize * 2 ) ) )
					{
						//Dma::_DMA->DMA0_ReadBlock ( (u32*) p_n_mem_ptr );
						p_n_mem_ptr = (u16*) Dma::_DMA->DMA0_ReadBlock ();
						
						if ( p_n_mem_ptr )
						{
							// load in the new block at that position
							// note: for now, data is being transferred in half-words
							//for ( int i = 0; i < ( c_n_inputfifosize / 2 ); i++ ) p_n_inputfifo [ n_writeindex_input++ & c_n_inputfifomask ] = *p_n_mem_ptr++;
							for ( int i = 0; i < ( c_n_mdec_blocksize * 2 ); i++ ) p_n_inputfifo [ n_writeindex_input++ & c_n_inputfifomask ] = *p_n_mem_ptr++;
							
							// that was 128 compressed bytes
							n_compressed_bytes += 128;
						}
						else
						{
							//for ( int i = 0; i < ( c_n_mdec_blocksize * 2 ); i++ ) p_n_inputfifo [ n_writeindex_input++ & c_n_inputfifomask ] = 0;
							
							// unable to load anymore data from dma0
							break;
						}
						
					}
					*/
					
#ifdef INLINE_DEBUG_WRITE
	MDEC::debug << "; n_compressed_bytes=" << dec << n_compressed_bytes;
#endif

					break;
					
				case 0x2:
				
					// upload quantization table //
					n_readyfordata_in = 1;
					
					// initialize input fifo
					n_readindex_input = 0;
					n_writeindex_input = 0;
					// get the number of compressed words minus some to decompress
					n_wordstotal = 0;
					// *** testing ***
					n_0_size = 0;
					n_compressed_bytes = 0;
					
					break;
					
				case 0x3:
				
					// upload cosine table //
					n_readyfordata_in = 1;
					
					// initialize input fifo
					n_readindex_input = 0;
					n_writeindex_input = 0;
					// get the number of compressed words minus some to decompress
					n_wordstotal = 0;
					// *** testing ***
					n_0_size = 0;
					n_compressed_bytes = 0;
					
					break;
					
				default:
					//verboselog( machine(), 0, "mdec unknown command %08x %08x %08x\n", n_0_command, n_address, n_size );
					cout << "hps1x64: MDEC unknown command. Data=" << hex << n_0_command << " Address=" << address << " PC=" << *MDEC::_DebugPC << " Cycle#" << dec << *MDEC::_DebugCycleCount << "\n";
					break;
			}
			
#ifdef INLINE_DEBUG_WRITE
	MDEC::debug << "; n_0_size=" << dec << (u32)n_0_size;
#endif

			break;
			
		case 0x1f801824:
			//verboselog( machine(), 2, "mdec 1 command %08x\n", data );
			n_1_command = data;
			
			// bit 31 = Reset MDEC; status = 0x80040000 //
			if ( n_1_command & ( 1 << 31 ) )
			{
				// reset //
				n_1_status = 0x80040000;
				
				// initialize input fifo
				n_readindex_input = 0;
				n_writeindex_input = 0;
				n_wordstotal = 0;
				
				// initialize the number of words decoded
				n_decoded = 0;
				
				// initialize read index for output buffer
				n_readindex_output = 0;
				
				// reset the command ??
				//n_0_command = 0;
			}
			
			// bit 30 = Enable data in request; enables DMA0 and Status bit 28 //
			if ( n_1_command & ( 1 << 30 ) )
			{
				// enable data in request //
				// *** todo ***
			}
			
			// bit 29 = Enable data out request; enables DMA1 and Status bit 27 //
			if ( n_1_command & ( 1 << 29 ) )
			{
				// enable data out request //
				// *** todo ***
			}
			
			break;
			
		default:
			cout << "\nhps1x64 NOTE: Invalid MDEC Device Write @ Cycle#" << dec << *MDEC::_DebugCycleCount << " PC=" << hex << *MDEC::_DebugPC << " Address=" << address << "\n";
			break;
	}
	
}

//READ32_MEMBER( psxmdec_device::read )
u32 psxmdec_device::read ( u32 address )
{
	switch( address )
	{
		case 0x1f801820:
			//verboselog( machine(), 2, "mdec 0 status %08x\n", 0 );
			return 0;
			
		case 0x1f801824:
		
			/*
			if ( *MDEC::_DebugCycleCount < n_busyuntil_cycle )
			{
				// MDEC is busy processing something //
				n_1_status |= ( 1 << 29 );
			}
			else if ( *MDEC::_DebugCycleCount >= n_busyuntil_cycle )
			{
				// MDEC is not busy with anything
				n_1_status &= ~( 1 << 29 );
			}
			*/
			
			n_wordsremaining = n_wordstotal - ( n_readindex_input >> 1 ) - 1;
			
			// note: probably should be set to not busy processing if mdec dma_in is not started
			//if ( ( ( n_wordsremaining > 0 ) && Dma::_DMA->isActive ( 1 ) && Dma::_DMA->isActive ( 0 ) ) || ( *MDEC::_DebugCycleCount < n_busyuntil_cycle ) )
			//if ( *MDEC::_DebugCycleCount < n_busyuntil_cycle )
			if ( n_wordsremaining > 0 )
			{
				// MDEC is busy processing something //
				n_1_status |= ( 1 << 29 );
			}
			else
			{
				// MDEC is not busy with anything
				n_1_status &= ~( 1 << 29 );
			}
			
			if ( n_wordsremaining < 0 ) n_wordsremaining = 0xffff;
			n_1_status = ( n_1_status & ~( 0xffff ) ) | ( n_wordsremaining & 0xffff );
			
			//verboselog( machine(), 2, "mdec 1 status %08x\n", n_1_status );
			return n_1_status;
			
		default:
			cout << "\nhps1x64 NOTE: Invalid MDEC Device Read @ Cycle#" << dec << *MDEC::_DebugCycleCount << " PC=" << hex << *MDEC::_DebugPC << " Address=" << address << "\n";
			break;
	}
	
	return 0;
}


///////////////////////////////////////////////////////////////////////
// *** Below here is the code that will remain untouched for now *** //


void psxmdec_device::mdec_cos_precalc()
{
	u32 n_x;
	u32 n_y;
	u32 n_u;
	u32 n_v;
	s32 *p_n_precalc = p_n_cos_precalc;

	for( n_y = 0; n_y < 8; n_y++ )
	{
		for( n_x = 0; n_x < 8; n_x++ )
		{
			for( n_v = 0; n_v < 8; n_v++ )
			{
				for( n_u = 0; n_u < 8; n_u++ )
				{
					*( p_n_precalc++ ) =
						( ( p_n_cos[ ( n_u * 8 ) + n_x ] *
						p_n_cos[ ( n_v * 8 ) + n_y ] ) >> ( 30 - MDEC_COS_PRECALC_BITS ) );
				}
			}
		}
	}
}


// uploads cosine table to mdec
void psxmdec_device::upload_ctable ( u32* p_n_data, u32 n_size )
{
	int n_index = 0;
	int n_address = 0;
	
	while( n_size > 0 )
	{
		//p_n_cos[ n_index + 0 ] = (INT16)( ( p_n_psxram[ n_address / 4 ] >> 0 ) & 0xffff );
		//p_n_cos[ n_index + 1 ] = (INT16)( ( p_n_psxram[ n_address / 4 ] >> 16 ) & 0xffff );
		p_n_cos[ n_index + 0 ] = (INT16)( ( p_n_data[ n_address >> 2 ] >> 0 ) & 0xffff );
		p_n_cos[ n_index + 1 ] = (INT16)( ( p_n_data[ n_address >> 2 ] >> 16 ) & 0xffff );
		n_index += 2;
		n_address += 4;
		n_size--;
	}
	
	mdec_cos_precalc();
}


// uploads quantization table to mdec
void psxmdec_device::upload_qtable ( u32* p_n_data, u32 n_size )
{
	int n_index = 0;
	int n_address = 0;
	while( n_size > 0 )
	{
		if( n_index < DCTSIZE2 )
		{
			p_n_quantize_y[ n_index + 0 ] = ( p_n_data[ n_address >> 2 ] >> 0 ) & 0xff;
			p_n_quantize_y[ n_index + 1 ] = ( p_n_data[ n_address >> 2 ] >> 8 ) & 0xff;
			p_n_quantize_y[ n_index + 2 ] = ( p_n_data[ n_address >> 2 ] >> 16 ) & 0xff;
			p_n_quantize_y[ n_index + 3 ] = ( p_n_data[ n_address >> 2 ] >> 24 ) & 0xff;
		}
		else if( n_index < DCTSIZE2 * 2 )
		{
			p_n_quantize_uv[ n_index + 0 - DCTSIZE2 ] = ( p_n_data[ n_address >> 2 ] >> 0 ) & 0xff;
			p_n_quantize_uv[ n_index + 1 - DCTSIZE2 ] = ( p_n_data[ n_address >> 2 ] >> 8 ) & 0xff;
			p_n_quantize_uv[ n_index + 2 - DCTSIZE2 ] = ( p_n_data[ n_address >> 2 ] >> 16 ) & 0xff;
			p_n_quantize_uv[ n_index + 3 - DCTSIZE2 ] = ( p_n_data[ n_address >> 2 ] >> 24 ) & 0xff;
		}
		
		n_index += 4;
		n_address += 4;
		n_size--;
	}
}



