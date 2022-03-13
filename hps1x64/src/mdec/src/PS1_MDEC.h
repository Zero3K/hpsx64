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


#ifndef _PS1_MDEC_H_
#define _PS1_MDEC_H_

#include "types.h"
#include "Debug.h"

#include "PS1_Dma.h"

namespace Playstation1
{




// I'm going to steal the mdec code from MAME/MESS here for now


/*
 * PlayStation Motion Decoder emulator
 *
 * Copyright 2003-2011 smf
 *
 * Thanks to Oliver Galibert for help figuring out IDCT
 *
 */


#define	DCTSIZE ( 8 )
#define	DCTSIZE2 ( DCTSIZE * DCTSIZE )

#define MDEC_COS_PRECALC_BITS ( 21 )

class psxmdec_device //: public device_t
{
public:
	static constexpr double c_n_cpuclockspeed = 33868800.0L;
	static constexpr double c_n_mdecbytespersecond = 9000000.0L;	//8000000.0L;
	
	static constexpr double c_n_mdec_cyclesperbyte = 33868800.0L / 9000000.0L;	//8000000.0L;


	//psxmdec_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	//WRITE32_MEMBER( write );
	void write ( u32 data, u32 address );
	u32 read ( u32 address );

	void dma_write( u32 n_address, s32 n_size, u32* RAM );
	
	// returns the number of blocks read
	int dma_read( u32 n_address, s32 n_size, u32* RAM/*, INT32 n_decodesize*/ );

//protected:
	//virtual void device_start();
	//virtual void device_reset();
	//virtual void device_post_load();
	void device_start();
	void device_reset();
	void device_post_load();

	void upload_qtable ( u32* p_n_data, u32 n_size );
	void upload_ctable ( u32* p_n_data, u32 n_size );

//private:
	void mdec_cos_precalc();
	void mdec_idct( s32 *p_n_src, s32 *p_n_dst );
	UINT32 mdec_unpack( u32 n_address, u32* RAM );
	UINT16 mdec_clamp_r5( s32 n_r ) const;
	UINT16 mdec_clamp_g5( s32 n_g ) const;
	UINT16 mdec_clamp_b5( s32 n_b ) const;
	UINT16 mdec_clamp8( s32 n_r ) const;
	void mdec_yuv2_to_rgb15( void );
	void mdec_yuv2_to_rgb24( void );
	void mdec_makergb15( u32 n_address, s32 n_r, s32 n_g, s32 n_b, s32 *p_n_y, u16 n_stp, u32 XorValue );
	void mdec_makergb24( u32 n_address, s32 n_r, s32 n_g, s32 n_b, s32 *p_n_y, u32 n_stp, u32 XorValue );

	void rl_decode_block( u32& SrcOffset, s32* blk, u16* src, s32* qt );

	u32 decode_colored_macroblock24 ( u32 SrcOffset, u16* src );
	u32 decode_colored_macroblock15 ( u32 SrcOffset, u16* src );
	u32 decode_monochrome_macroblock8 ( u32 SrcOffset, u16* src );
	u32 decode_monochrome_macroblock4 ( u32 SrcOffset, u16* src );
	
	void mdec_y_to_mono8();
	void mdec_y_to_mono4();

	//u32 n_index_global;
	
	u64 n_nextevent_cycle;
	
	u32 n_readyfordata_in, n_readyfordata_out;
	
	// set this to allow transfer of data into input fifo when decoding
	u32 n_allow_decodetransfer;


	u32 n_decoded;
	u32 n_offset;
	u16 p_n_output[ 24 * 16 ];

	s32 p_n_quantize_y[ DCTSIZE2 ];
	s32 p_n_quantize_uv[ DCTSIZE2 ];
	s32 p_n_cos[ DCTSIZE2 ];
	s32 p_n_cos_precalc[ DCTSIZE2 * DCTSIZE2 ];
	
	s32 p_n_crblk [ DCTSIZE2 ];
	s32 p_n_cbblk [ DCTSIZE2 ];
	s32 p_n_yblk [ DCTSIZE2 * 4 ];
	//INT32 p_n_y1blk [ DCTSIZE2 ];
	//INT32 p_n_y2blk [ DCTSIZE2 ];
	//INT32 p_n_y3blk [ DCTSIZE2 ];
	//INT32 p_n_y4blk [ DCTSIZE2 ];

	u32 n_0_command;
	u32 n_0_address;
	u32 n_0_size;
	u32 n_1_command;
	u32 n_1_status;

	u16 p_n_clamp8[ 256 * 3 ];
	u16 p_n_r5[ 256 * 3 ];
	u16 p_n_g5[ 256 * 3 ];
	u16 p_n_b5[ 256 * 3 ];

	//INT32 m_p_n_unpacked[ DCTSIZE2 * 6 * 2 ];
	
	u32 *p_n_data_in;
	
	u16* p_n_mem_ptr;
	
	u64 n_busyuntil_cycle, n_cycles_used;
	
	// input buffer
	s32 n_writeindex_input, n_readindex_input;
	
	u32 n_compressed_bytes;
	
	// the current block that mdec is on
	// 0-3 : y1-y4; 4: Cr; 5: Cb
	// for mono is always 4: Y
	u32 n_current_block;
	
	// mdec block size is 64 halfwords, or 0x20 words
	
	// block size for mdec in 32-bit words
	static const u32 c_n_mdec_blocksize = 0x20;
	
	// the blocks in the fifo should be a power of 2... like 2, 4, 8, 16... etc
	static const u32 c_n_inputfifoblocks = 16;

	// the total size of the input fifo in half-words
	static const u32 c_n_inputfifosize = 64 * c_n_inputfifoblocks;	// 64 half-words times number of blocks
	static const u32 c_n_inputfifomask = c_n_inputfifosize - 1;
	//static const u32 c_n_inputfifomask_2 = 64;
	
	u16 p_n_inputfifo [ c_n_inputfifosize ];
	
	// output buffer
	s32 n_writeindex_output, n_readindex_output;
	static const int c_n_outputfifosize = 24 * 16 * 2;
	u16 p_n_outputfifo [ 24 * 16 * 2 ];
	
	// param/compressed words remaining/total
	s32 n_wordsremaining, n_wordstotal;
};




	class MDEC
	{
	public:
	
		static MDEC *_MDEC;
		//static DMA *_DMA;
	
		static Debug::Log debug;
	
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the mdec registers start at
		static const long Regs_Start = 0x1f801820;
		
		// where the mdec registers end at
		static const long Regs_End = 0x1f801824;
	
		// distance between numbered groups of registers for dma
		static const long Reg_Size = 0x4;
		
		// I'll probably actually use the mame/mess mdec device
		psxmdec_device mess_mdec;
		
		// should work out to one 4-byte transfer every 16 cycles
		const u32 c_MDECIn_Cycles = 16;
		
		// *** NOTE: this value is important ***
		const u32 c_MDECOut_Cycles = 4;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;
		
		// will need a pointer into RAM
		
		// quantization table for y component
		//u32 Qy [ 64 ];
		
		// quantization table for uv component
		//u32 Quv [ 64 ];
		
		// cosine table
		//u32 CosTable [ 64 ];
		
		
		static u32 Read ( u32 Address );
		static void Write ( u32 Address, u32 Data, u32 Mask );
		
		static u64 DMA_ReadyForRead ();
		static u64 DMA_ReadyForWrite ();
		
		static u32 DMA_ReadBlock ( u32* RAM, u32 Address, u32 Size );
		static u32 DMA_WriteBlock ( u32* RAM, u32 Address, u32 Size );
		void DMA0_Start ();
		void DMA0_End ();
		
		void Start ();
		
		void Reset ();
		
		void Run ();
		
		u64 BusyUntil_Cycle;

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////

		// MDEC data / commands - r/w
		static const long MDEC_CTRL = 0x1f801820;
		
		struct CTRL_Write_Format
		{
			union
			{
				// NOP command (0,4-7)
				// copies bits 25-28 into Status bits 23-26. copies bits 0-15 into status bits 0-15
			
				// decode command (1)
				struct
				{
					// bits 0-24
					//u32 Unknown0 : 25;
					
					// bits 0-15 - Number of Parameter Words (size of compressed data)
					// *note* this is actually the size of the compressed data in words minus 32 words (using 32-bit word size)
					u32 CompressedDataSize_Words : 16;
					
					// bits 16-24 - Not used (should be zero)
					u32 NotUsed0 : 9;
					
					// toggles whether to set bit 15 of the decompressed data (semi-transparency)
					// bit 25
					//u32 STP : 1;
					
					// bit 25 - Data Output Bit15 (0: Clear; 1: Set) (for 15bit depth only)
					u32 DataOut_Bit15 : 1;
					
					// bit 26
					//u32 Unknown1 : 1;
					
					// bit 26 - Data Output Signed (0: Unsigned; 1: Signed)
					u32 DataOut_Signed : 1;
					
					// 0-24bit color;1-16bit color in 16bit mode
					// bit 27
					//u32 RGB24 : 1;

					// bits 28-31
					//u32 Command : 4;
					
					// bits 27-28 - Data Output Depth (0: 4bit; 1: 8bit; 2: 24bit; 3: 15bit)
					u32 DataOut_Depth : 2;
					
					// bits 29-31
					u32 Command : 3;
					
				};
				
				// set quant table(s) command (2)
				struct
				{
					// bit 0 - Color (0: Luminance only; 1: Luminance and color)
					u32 Color : 1;
					
					// bits 1-28 - Not Used - (should be zero) - bits 25-28 copied to STAT.23-26 though
					u32 NotUsed1 : 28;
					
					// bits 29-31 - Command
					u32 Command1 : 3;
				};
				
				// set scale table command (3)
				struct
				{
					// bits 0-28 - Not used - (should be zero) - bits 25-28 copied to STAT.23-26 though
					u32 NotUsed2 : 29;
					
					// bits 29-31 - Command
					u32 Command2 : 3;
				};
				
				u32 Value;
			};
		};
		
		CTRL_Write_Format Reg_CTRL;
		
		u32 Command;

		
		// MDEC Status / Control - r/w
		static const long MDEC_STAT = 0x1f801824;

		struct STAT_Read_Format
		{
			union
			{
				struct
				{
					// bits 0-22
					//u32 Unknown0 : 23;
					
					// bits 0-15 - Number of parameter words remaining minus 1 (0xffff: None)
					u32 ParamWords_Remaining : 16;
					
					// bits 16-18 - Current Block (0: Y1; 1: Y2; 2: Y3; 3: Y4; 4: Cr; 5: Cb) (for mono always 4=Y)
					u32 CurrentBlock : 3;
					
					// bits 19-22 - Not used (always zero)
					u32 zero0 : 4;
				
					// toggles whether to set bit 15 of the decompressed data (semi-transparency)
					// bit 23
					//u32 STP : 1;
					
					// bit 23 - Data Output Bit15 (0: Clear; 1: Set) (for 15-bit depth only)
					//u32 DataOut_Bit15 : 1;
					u32 STP : 1;
					
					// MDEC is transferring data to main memory
					// bit 24
					//u32 OutSync : 1;
					
					// bit 24 - Data Output Signed (0: Unsigned; 1: Signed)
					u32 DataOut_Signed : 1;
					
					// 0-24bit color;1-16bit color in 16bit mode
					// bit 25
					//u32 RGB24 : 1;
					
					// bit 26
					//u32 Unknown1 : 1;
					
					// bits 25-26 - Data Output Depth (0: 4 bit; 1: 8-bit; 2: 24-bit; 3: 15-bit)
					u32 DataOut_Depth : 2;
					
					// Data request
					// bits 27-28
					//u32 DREQ : 2;
					
					// bit 27 - Data-Out Request (set when DMA1 enabled and ready to send data)
					u32 DataOut_Request : 1;
					
					// bit 28 - Data-In Request (set when DMA0 enabled and ready to receive data)
					u32 DataIn_Request : 1;
					
					// MDEC is busy decompressing data
					// bit 29
					//u32 InSync : 1;
					
					// bit 29 - Command Busy (0: Ready; 1: Busy receiving or processing parameters)
					u32 CommandBusy : 1;
					
					// first-in-first-out buffer state
					// bits 30-31
					//u32 FIFO : 2;
					
					// bit 30 - Data-In FIFO Full (0: NOT Full; 1: Full)
					u32 DataInFIFO_Full : 1;
					
					// bit 31 - Data-Out FIFO Empty (0: NOT Empty; 1: Empty)
					u32 DataOutFIFO_Empty : 1;
				};
				
				u32 Value;
			};
		};
		
		STAT_Read_Format Reg_STAT;

		
		struct STAT_Write_Format
		{
			union
			{
				struct
				{
					// bits 0-28 - Unknown/Not Used (usually zero)
					u32 Unknown0 : 29;
					
					
					// bit 29 - Enable Data-Out Request (0: Disable; 1: Enable DMA1 and Status.Bit27)
					u32 DataOutRequest_Enable : 1;
					
					// bit 30 - Enable Data-In Request (0: Disable; 1: Enable DMA0 and Status.Bit28)
					u32 DataInRequest_Enable : 1;
					
					// Resets MDEC
					// bit 31 (0: No change; 1: Abort any command, and set status=0x80040000
					u32 Reset : 1;
				};
				
				u32 Value;
			};
		};
		
		
		// data-in fifo
		// must have a size of 32 words since transfer block size is always 32 words
		static const int c_iDataFIFO_Size = 32;
		u32 DataIn_FIFO [ c_iDataFIFO_Size ];
		u32 DataIn_FIFO_Index, DataIn_FIFO_Size;
		u32 DataOut_FIFO [ c_iDataFIFO_Size ];
		u32 DataOut_FIFO_Index, DataOut_FIFO_Size;
		
		// constructor
		//MDEC ();
		
		void UpdateStatus ();
		
		// busy cycles
		u64 BusyCycle0;
		u64 BusyCycle1;
		
		// Debug
		static u32 *_DebugPC;
		static u64 *_DebugCycleCount;
		static u64 *_SystemCycleCount;
	};
	
	
};





#endif

