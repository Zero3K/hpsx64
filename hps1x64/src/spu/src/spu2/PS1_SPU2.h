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


#ifndef _SPU2_H_
#define _SPU2_H_

#include "types.h"
#include "Debug.h"

#include "DebugValueList.h"
#include "DebugMemoryViewer.h"

#include "adpcm.h"
#include "LowPassFilter.h"

#include <algorithm>
using namespace std;

namespace Playstation1
{





	///////////////////////////////////////////////////////
	// *** anything below this point is my own stuff ***
	///////////////////////////////////////////////////////

	class SPUCore
	{
		
	public:
	
		static Debug::Log debug;
		static SPUCore *_SPUCore;
		
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the dma registers start at
		static constexpr long Regs_Start = 0x1f900000;
		
		// where the dma registers end at
		static constexpr long Regs_End = 0x1f900800;
	
		// distance between numbered groups of registers for dma
		static constexpr long Reg_Size = 0x2;

		
#ifdef PS2_COMPILE
		////////////////////////////////////////////
		// 2MB of Sound RAM for SPU2 ??
		static constexpr int c_iRam_Size = 2097152;
#else
		static constexpr int c_iRam_Size = 524288;
#endif
		
		static constexpr int c_iRam_Mask = c_iRam_Size - 1;
		
		// spu frequency
		// this should change for PS2
		static constexpr u32 SPU_Frequency = 44100;
		static constexpr u32 CPUCycles_PerSPUCycle = 768;
		
		u64 CycleCount;
		
		
		// the ram needs to be part of the parent object (SPU1 Interface/SPU2 Interface, etc)
		static u16 *RAM;
		
		int CoreNumber;
		
		
		s32 AudioFilter_Enabled;
		
		// used for debugging sound problems
		static u32 Debug_ChannelEnable;
		
		
		// index of next event
		u32 NextEvent_Idx;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;
		
		// for PS2, I will allow storing to 2048 bytes registers, which is 1024 bytes per core, or 512 registers
		//static const int c_iNumberOfRegisters = 256 + 48;
		static constexpr int c_iNumberOfRegisters = 0x400;
		static constexpr u32 c_iRegistersMask = c_iNumberOfRegisters - 1;
		
		
		// where sound data input area starts (PS2 only)
		// first area is for core0 Left samples, then next is core0 Right samples, then core1 Left samples, then core1 Right samples
		// this address is in halfwords
		static constexpr u32 c_lSoundDataInputArea_Start = 0x2000;
		
		// the total size in samples for all core 0/1 L/R samples in sound data input area (ADMA)
		static constexpr u32 c_lSoundDataInputArea_TotalSize = 0x200 * 4;
		
		// the size in samples (2 bytes) for only one core and one channel of sound data input area
		static constexpr u32 c_lSoundDataInputArea_Size = 0x200;
		

		// added 48 for the internal channel volume registers
		static const char* RegisterNames1 [ 419 ];
		static const char* RegisterNames2 [ 20 ];
		static const char* RegisterNames3 [ 5 ];
		
		// the registers need to be part of the parent object
		//union Regs
		//{
		//	u16 u [ c_iNumberOfRegisters ];
		//	s16 s [ c_iNumberOfRegisters ];
		//};
		union Regs
		{
			u16 *u;
			s16 *s;
		};
		
		static Regs Regs16;
		
		#define GET_REG16u(idx) Regs16.u [ idx >> 1 ]
		#define GET_REG16s(idx) Regs16.s [ idx >> 1 ]
		
		#define GET_REG16(idx) Regs16.u [ idx >> 1 ]
		#define SET_REG16(idx,data) Regs16.u [ idx >> 1 ] = data
		
		
		#define GET_REG32(idx) ( ( ( (u32) Regs16.u [ ( idx >> 1 ) + 1 ] ) << 16 ) | ( (u32) Regs16.u [ idx >> 1 ] ) )
		#define SET_REG32(idx,data) Regs16.u [ ( idx >> 1 ) + 1 ] = ( data >> 16 ); Regs16.u [ idx >> 1 ] = data
		
		#define GET_REG32X(core,regname) ( ( ( (u32) SpuRegs0->CoreRegs0 [ core ].regname##_1 ) << 16 ) | ( (u32) SpuRegs0->CoreRegs0 [ core ].regname##_0 ) )
		#define SET_REG32X(core,regname,data) SpuRegs0->CoreRegs0 [ core ].regname##_1 = ( data >> 16 ); SpuRegs0->CoreRegs0 [ core ].regname##_0 = data

		#define GET_REG16X(core,regname) ( SpuRegs0->CoreRegs0 [ core ].regname )
		#define SET_REG16X(core,regname,data) SpuRegs0->CoreRegs0 [ core ].regname = data
		#define GET_REG16X1(core,regname) ( SpuRegs1->CoreRegs1 [ core ].regname )
		#define SET_REG16X1(core,regname,data) SpuRegs1->CorerRegs1 [ core ].regname = data
		
		// the higher register is actually in the lower part of the full 32-bit value, and the lower register is in the upper part of the 32-bit value
		// the new registers get reversed when read as a 32-bit value
		#define GET_REG32R(core,regname) ( ( ( (u32) SpuRegs0->CoreRegs0 [ core ].regname::_0 ) << 16 ) | ( (u32) SpuRegs0->CoreRegs0 [ core ].regname::_1 ) )
		#define SET_REG32R(core,regname,data) SpuRegs0->CoreRegs0 [ core ].regname::_0 = ( data >> 16 ); SpuRegs0->CoreRegs0 [ core ].regname::_1 = data
		
		// reverse the halfwords for the value (some 32-bit registers have the halfwords reversed?)
		#define SWAPH(value32) ( ( ( (u32) value32 ) >> 16 ) | ( ( (u32) value32 ) << 16 ) )
		
		u32 lSPDIF_OUT, lSPDIF_IRQINFO, lSPDIF_MODE, lSPDIF_MEDIA, lSPDIF_PROTECT;


		static constexpr int NumberOfChannels = 24;
		static constexpr int c_iNumberOfChannels = 24;
		
		// the number of pcm samples per adpcm block
		static constexpr int c_iSamplesPerBlock = 28;
		
		// gain value for mixing samples??
		// a 1 would mean a shift right of 1 each time you mix
		static constexpr float c_iMixer_Gain = 1.0f;
		
		// w0 = 2*pi*f0/Fs
		// b0 = ( 1 - cos(w0) ) / 2
		// b1 = 1 - cos(w0)
		// b2 = ( 1 - cos(w0) ) / 2
		
		// Dr Hell's FIR filter constants
		static const s32 _N = 14;
		static const s32 _a1 = -2397;
		static const s32 _a2 = 88;
		static const s32 _b0 = 3519;
		static const s32 _b1 = 7037;
		static const s32 _b2 = 3519;
		/*
		static const s32 _N = 14;
		static const s32 _a1 = -30788;
		static const s32 _a2 = 14518;
		static const s32 _b0 = 15423;
		static const s32 _b1 = -30845;
		static const s32 _b2 = 15423;
		*/
		
		
		// get value in an SPU register specified by its full physical address
		//inline u16 REG ( u32 Address ) { return Regs [ ( Address - Regs_Start ) >> 1 ]; }
		//inline s32 REG32 ( u32 Address ) { return ( (s32) ( (s16) Regs [ ( Address - Regs_Start ) >> 1 ] ) ); }
		//inline s64 REG64 ( u32 Address ) { return ( (s64) ( (s16) Regs [ ( Address - Regs_Start ) >> 1 ] ) ); }


		// testing something from Dr Hell's web page
		static const short gx [ 512 ];
		//s64 gx_buffer [ c_iNumberOfChannels * 4 ];
		static s32 Calc_sample_gx ( u32 SampleOffset_Fixed16, s32 Sample0, s32 Sample1, s32 Sample2, s32 Sample3 );
		
		static s32 Calc_sample_filter ( s32 x0, s32 x1, s32 x2, s32 y1, s32 y2 );
		
		// sample history for filter
		//u64 Reverb_FSampleCounter, FSampleCounter;
		s32 LReverb_x1, LReverb_x2, LReverb_y1, LReverb_y2;
		s32 RReverb_x1, RReverb_x2, RReverb_y1, RReverb_y2;
		s32 Lx1, Lx2, Ly1, Ly2;
		s32 Rx1, Rx2, Ry1, Ry2;
		//s32 LInputSampleHistory [ 4 ];
		//s32 LOutputSampleHistory [ 4 ];
		//s32 RInputSampleHistory [ 4 ];
		//s32 ROutputSampleHistory [ 4 ];
		
		// these are global for PS2 for now
		s64 SampleL, SampleR;
		
		// SPU cycle at which channel was keyed on
		u64 StartCycle_Channel [ c_iNumberOfChannels ];
		u16 ActiveLSA_Channel [ c_iNumberOfChannels ];
		
		void Start_SampleDecoding ( u32 Channel );
		
		s32 ReverbWork_Size, ReverbWork_Start, ReverbWork_End;
		
		// gets address in reverb work area at offset address
		s64 ReadReverbBuffer ( u32 Address );
		void WriteReverbBuffer ( u32 Address, s64 Value );
		void UpdateReverbBuffer ();
		
		
		// set the core number
		inline void SetCoreNumber ( int Number ) { CoreNumber = Number; }
		
		
		// get value in SPU register specified by its channel number and offset
		//inline u16 REG ( u32 Channel, u32 Offset ) { return Regs [ ( ( Channel << 4 ) + Offset ) >> 1 ]; }
		
		void SpuTransfer_Complete ();
		
		u32 Read ( u32 Address, u32 Mask );
		void Write ( u32 Address, u32 Data, u32 Mask );
		void DMA_Read ( u32* Data, int ByteReadCount );
		void DMA_Read_Block ( u32* Data, u32 BS );
		void DMA_Write ( u32* Data, int ByteWriteCount );
		u32 DMA_Write_Block ( u32* Data, u32 BS );
		//void DMA_Write_All ( u32* Data, int TransferAmountInWords32, int BlockSizeInWords32 );
		
		
		// returns true if device is ready for a read from DMA
		bool DMA_ReadyForRead ( void );
		bool DMA_ReadyForWrite ( void );
		
		void Reset ();
		
		void Start ();
		
		void Run ();
		
		s32 Timer;
		s16 NoiseLevel;
		void RunNoiseGenerator ();
		
		
		// processes reverb
		s64 ReverbL_Output, ReverbR_Output;
		u32 Reverb_BufferAddress;
		void ProcessReverbL ( s64 LeftInput );
		void ProcessReverbR ( s64 RightInput );
		
		
		Emulator::Audio::LowPassFilter<5,8> LPF_L;
		Emulator::Audio::LowPassFilter<5,8> LPF_R;
		Emulator::Audio::LowPassFilter<5,8> LPF_L_Reverb;
		Emulator::Audio::LowPassFilter<5,8> LPF_R_Reverb;

		// need to be able to decode sample blocks
		// BlockStartAddress is the byte at which the block starts at in sound RAM
		// should not decode block again if it is cached
		//void DecodeBlock ( u16* DecodedSamples, EncodedSoundBlock* esb );
		
		// need to be able to deallocate cached blocks when they are overwritten
		// Address is the byte that is being overwritten in sound RAM
		void InvalidateCachedSampleBlock ( u32 Address );
		
		// need to be able to get a sample from a block that has been decoded
		// make sure block is decoded first
		// BlockStartAddress is the byte that sample block starts at in sound RAM
		// SampleNumberInBlock is which sample to pull from the sample block
		u32 GetSampleFromBlock ( u32 BlockStartAddress, u32 SampleNumberInBlock );
		
		// need to be able to input CD-DA data from CD device
		// should receive raw samples from CD
		// *note* cd-da data is just stored on the CD as raw pcm data
		void CDTransfer_CDDA ( u32* Data, u32 SizeInBytes );
		
		// need to be able to input CD-XA data from CD device
		// should receive raw samples from CD
		void CDTransfer_CDXA ( u32* Data, u32 SizeInBytes );

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////
		
		static const long SPU_REGXX_Base = 0x1f801c00;
		
		///////////////////////////////////////
		// 256 + 48 16-bit SPU Registers
		//u16 Regs [ 256 ];
		//u16 Regs [ c_iNumberOfRegisters ];
		
		///////////////////////////////////////////////////
		// 64 byte buffer
		u32 BufferIndex;
		u16 Buffer [ 32 ];
		
		
		// unsure what this value should be yet
		static constexpr int c_iVolumeShift = 15;
		
		/*
		// the ram should not be in the SPU core object, since it is used by both cores in PS2
		static const int c_iRam_Size = 524288;
		static const int c_iRam_Mask = c_iRam_Size - 1;
		
		////////////////////////////////////////////
		// 512 KB of Sound RAM for SPU
		u16 RAM [ c_iRam_Size / 2 ];
		
		// sound output *** testing ***
		// these should be static, but need to be left like this to fix a program crash/mute bug first
		static HWAVEOUT hWaveOut;
		static WAVEFORMATEX wfx;
		static WAVEHDR header;
		static WAVEHDR header0;
		static WAVEHDR header1;
		//static u64 hWaveOut_Save;
		
		s32 AudioOutput_Enabled;
		s32 AudioFilter_Enabled;
		*/
		
		
		// need to know when a channel is playing or not
		u32 ChannelPlaying_Bitmap;
		
		// need to know when to kill the envelope for a channel
		u32 KillEnvelope_Bitmap;
		
		// loop specified bitmap
		u32 LSA_Manual_Bitmap;
		
		
		// play size is in samples
		// 1024 does not work right, but 2048 is ok, and 4096 is bit of a delay.. need to fix the buffer size multiply
		// using 262144 for testing
		/*
		static const int c_iPlaySize = 262144;
		static const int c_iPlayBuffer_MaxSize = 131072;	//c_iPlaySize * 4;
		//u16 PlayBuffer [ c_iPlayBuffer_Size ] __attribute__ ((aligned (32)));
		s16 PlayBuffer0 [ c_iPlayBuffer_MaxSize ] __attribute__ ((aligned (32)));
		s16 PlayBuffer1 [ c_iPlayBuffer_MaxSize ] __attribute__ ((aligned (32)));
		
		// dynamic size of sound buffer
		s32 NextPlayBuffer_Size;
		u32 PlayBuffer_Size;
		
		static const int c_iMixerSize = c_iPlayBuffer_MaxSize;	//1048576;
		static const int c_iMixerMask = c_iMixerSize - 1;
		u64 Mixer_ReadIdx, Mixer_WriteIdx;
		s16 Mixer [ c_iMixerSize ];
		*/
		
		
		SPUCore ();
		~SPUCore ();
		
		void UpdatePitch ( int Channel, u32 Pitch, u32 Reg_PMON, s32 PreviousSample );
		
		// returns 0 if the first half of buffer is in use, otherwise the second half of buffer is in use
		inline u32 GetBufferInUse () { return ( DecodeBufferOffset & 0x200 ); }
		
		inline u32 isADMATransferMode () { return pCoreRegs0->ADMAS & ( 1 << CoreNumber ); }
		
		
		
		// this must be checked only AFTER a transfer has been made
		// returns ZERO for interrupt, NO interrupt otherwise
		// but not needed for now, since will transfer 256/512 samples at a time for now
		//inline u32 isADMAInterrupt () { return ulADMA_Offset8 & 0x3ff; }
		
		
		// need to know if buffer-half is full of samples
		u32 bBufferFull [ 2 ];
		
		// the buffer-half that is not currently playing
		u32 ulBufferHalf_Offset;
		
		// current pointer for ADMA playing
		u64 ulADMA_PlayOffset;
		
		// current pointer for ADMA Transfer
		// if this is less than 512 or this minus play offset is 256 or less, then ok to transfer
		u64 ulADMA_TransferOffset;
		
		// offset for ADMA from decoding ??
		u64 ulADMA_StartCycle;
		
		u32 Cycles [ c_iNumberOfChannels ];
		
		// current offset for ADMA transfer
		u32 ulADMA_Offset8;
		bool bADMATransfer;
		
		////////////////////////////////////////////////////////////////////////////
		// We need to know the address of the current sample for each channel
		// this will be in 32.32 fixed point
		
		// these two are update by the pitch. the offset is the offset in samples packet, the read is the read position used as offset into buffer
		u64 CurrentSample_Offset [ c_iNumberOfChannels ];
		u64 CurrentSample_Read [ c_iNumberOfChannels ];
		
		// this is the place to unload the next packet of samples
		u64 CurrentSample_Write [ c_iNumberOfChannels ];
		
		u64 Pitch_Counter [ c_iNumberOfChannels ];
		
		// 24 channels * 28 samples per adpcm block
		//s16 DecodedBlocks [ c_iNumberOfChannels * 28 ];
		s32 DecodedSamples [ 28 ];
		s32 DecodedBlocks [ c_iNumberOfChannels * 32 ];
		
		///////////////////////////////////////////////////////////////////////////
		// We need to know how many samples to advance on every SPU cycle
		// this will be in 32.32 fixed point
		u64 dSampleDT [ c_iNumberOfChannels ];
		u64 SampleRate [ c_iNumberOfChannels ];
		
		// need to debug samples
		s16 Debug_CurrentRawSample [ c_iNumberOfChannels ];
		s16 Debug_CurrentSampleL [ c_iNumberOfChannels ];
		s16 Debug_CurrentSampleR [ c_iNumberOfChannels ];
		u16 Debug_CurrentSample [ c_iNumberOfChannels ];
		u16 Debug_CurrentRate [ c_iNumberOfChannels ];
		
		// also need the current and next block addresses
		u32 CurrentBlockAddress [ c_iNumberOfChannels ];
		u32 NextBlockAddress [ c_iNumberOfChannels ];
		
		// also need a decoder for each channel
		adpcm_decoder SampleDecoder [ c_iNumberOfChannels ];
		
		/////////////////////////////////////////////////////////////////////////////
		// We need to know where to output processed sample to
		//$00000-$003ff  CD audio left
		//$00400-$007ff  CD audio right
		//$00800-$00bff  Voice 1
		//$00c00-$00fff  Voice 3
		// note: decode buffer size below is in bytes (as in 1024 bytes)
		static constexpr u64 DecodeBufferSize = 0x400;
		static constexpr u64 DecodeBufferMask = DecodeBufferSize - 1;
		u64 DecodeBufferOffset;	// this is where the current sample is being decoded and mixed to
		
		//////////////////////////////////////////////
		// start of SPU2 registers
		static constexpr u32 SPU2_X = 0x1f900000;
		
		
		// structures to access both groups of spu registers
		
		struct ChRegs0_Layout
		{
			s16 VOL_L, VOL_R;
			u16 PITCH, ADSR_0, ADSR_1, ENV_X;
			s16 CVOL_L, CVOL_R;
		};
		
		struct ChRegs1_Layout
		{
			union
			{
				struct
				{
					u16 SSA_1, SSA_0, LSA_1, LSA_0, NEX_1, NEX_0;
				};
				
				struct
				{
					u32 SSA, LSA, NEX;
				};
			};
		};
		
		struct CoreRegs0_Layout
		{
			// VOL_L, VOL_R, PITCH, ADSR_0, ADSR_1, ENV_X, CVOL_L, CVOL_R
			ChRegs0_Layout ChRegs0 [ 24 ];
			
			union 
			{
				struct
				{
					u16 PMON_0, PMON_1, NON_0, NON_1, VMIXL_0, VMIXL_1, VMIXEL_0, VMIXEL_1, VMIXR_0, VMIXR_1, VMIXER_0, VMIXER_1;
				};
				
				struct
				{
					u32 PMON, NON, VMIXL, VMIXEL, VMIXR, VMIXER;
				};
			};
			
			u16 MMIX, CTRL;
			
			union 
			{
				struct
				{
					u16 IRQA_1, IRQA_0, KON_0, KON_1, KOFF_0, KOFF_1, SBA_1, SBA_0;
				};
				
				struct
				{
					u32 IRQA, KON, KOFF, SBA;
				};
			};
			
			u16 DATA, UNK0, ADMAS, UNK1, UNK2, UNK3, UNK4, UNK5, UNK6, UNK7;
			
			// SSA_0, SSA_1, LSA_0, LSA_1, NEX_0, NEX_1
			ChRegs1_Layout ChRegs1 [ 24 ];
			
			union
			{
				struct
				{
					u16 RVWAS_1, RVWAS_0, dAPF1_1, dAPF1_0, dAPF2_1, dAPF2_0, mLSAME_1, mLSAME_0,
					mRSAME_1, mRSAME_0, mLCOMB1_1, mLCOMB1_0, mRCOMB1_1, mRCOMB1_0, mLCOMB2_1, mLCOMB2_0,
					mRCOMB2_1, mRCOMB2_0, dLSAME_1, dLSAME_0, dRSAME_1, dRSAME_0, mLDIFF_1, mLDIFF_0,
					mRDIFF_1, mRDIFF_0, mLCOMB3_1, mLCOMB3_0, mRCOMB3_1, mRCOMB3_0, mLCOMB4_1, mLCOMB4_0,
					mRCOMB4_1, mRCOMB4_0, dLDIFF_1, dLDIFF_0, dRDIFF_1, dRDIFF_0, mLAPF1_1, mLAPF1_0,
					mRAPF1_1, mRAPF1_0, mLAPF2_1, mLAPF2_0, mRAPF2_1, mRAPF2_0, RVWAE_1, RVWAE_0,
					CON_0, CON_1;
				};
				
				struct
				{
					u32 RVWAS, dAPF1, dAPF2, mLSAME, mRSAME, mLCOMB1, mRCOMB1, mLCOMB2, mRCOMB2, dLSAME, dRSAME, mLDIFF,
						mRDIFF, mLCOMB3, mRCOMB3, mLCOMB4, mRCOMB4, dLDIFF, dRDIFF, mLAPF1, mRAPF1, mLAPF2, mRAPF2, RVWAE, CON;
				};
			};
				
			u16 STAT;
		};
		
		//struct SpuRegs0_Layout
		//{
		//	CoreRegs0_t CoreRegs0 [ 2 ];
		//};
		
		struct CoreRegs1_Layout
		{
			// start at 0x760
			s16 MVOL_L, MVOL_R, vLOUT /*EVOL_L*/, vROUT /*EVOL_R*/, AVOL_L, AVOL_R, BVOL_L, BVOL_R,
			CMVOL_L, CMVOL_R, vIIR, vCOMB1, vCOMB2, vCOMB3, vCOMB4, vWALL,
			vAPF1, vAPF2, vLIN, vRIN;
		};



		struct ChRegsMT_Layout
		{
			u16 ENV_X;
			s16 CVOL_L, CVOL_R;
		};

		struct CoreRegsMT_Layout
		{
			// ENV_X, CVOL_L, CVOL_R
			ChRegsMT_Layout ChRegs0 [ 24 ];
			
			union 
			{
				struct
				{
					u16 PMON_0, PMON_1, NON_0, NON_1, VMIXL_0, VMIXL_1, VMIXEL_0, VMIXEL_1, VMIXR_0, VMIXR_1, VMIXER_0, VMIXER_1;
				};
				
				struct
				{
					u32 PMON, NON, VMIXL, VMIXEL, VMIXR, VMIXER;
				};
			};
			
			u16 MMIX, CTRL;
			
			
			u16 ADMAS;
			
			
			union
			{
				struct
				{
					u16 RVWAS_1, RVWAS_0, dAPF1_1, dAPF1_0, dAPF2_1, dAPF2_0, mLSAME_1, mLSAME_0,
					mRSAME_1, mRSAME_0, mLCOMB1_1, mLCOMB1_0, mRCOMB1_1, mRCOMB1_0, mLCOMB2_1, mLCOMB2_0,
					mRCOMB2_1, mRCOMB2_0, dLSAME_1, dLSAME_0, dRSAME_1, dRSAME_0, mLDIFF_1, mLDIFF_0,
					mRDIFF_1, mRDIFF_0, mLCOMB3_1, mLCOMB3_0, mRCOMB3_1, mRCOMB3_0, mLCOMB4_1, mLCOMB4_0,
					mRCOMB4_1, mRCOMB4_0, dLDIFF_1, dLDIFF_0, dRDIFF_1, dRDIFF_0, mLAPF1_1, mLAPF1_0,
					mRAPF1_1, mRAPF1_0, mLAPF2_1, mLAPF2_0, mRAPF2_1, mRAPF2_0, RVWAE_1, RVWAE_0,
					CON_0, CON_1;
				};
				
				struct
				{
					u32 RVWAS, dAPF1, dAPF2, mLSAME, mRSAME, mLCOMB1, mRCOMB1, mLCOMB2, mRCOMB2, dLSAME, dRSAME, mLDIFF,
						mRDIFF, mLCOMB3, mRCOMB3, mLCOMB4, mRCOMB4, dLDIFF, dRDIFF, mLAPF1, mRAPF1, mLAPF2, mRAPF2, RVWAE, CON;
				};
			};
				
			// start at 0x760
			s16 MVOL_L, MVOL_R, vLOUT /*EVOL_L*/, vROUT /*EVOL_R*/, AVOL_L, AVOL_R, BVOL_L, BVOL_R,
			CMVOL_L, CMVOL_R, vIIR, vCOMB1, vCOMB2, vCOMB3, vCOMB4, vWALL,
			vAPF1, vAPF2, vLIN, vRIN;
		};
		
		
		
		
		CoreRegs0_Layout *pCoreRegs0;
		CoreRegs1_Layout *pCoreRegs1;
		
		
		// parameters for multi-threading //
		bool bMTEnabled;
		static constexpr int c_iMaxMTBufferSize = 1 << 12;
		static u64 ullMTWriteIndex;
		static u64 ullMTReadIndex;
		static CoreRegsMT_Layout vMTBuffer [ c_iMaxMTBufferSize ];
		static CoreRegsMT_Layout *pCoreRegsMT;
		
		
		
		// value after multiplication with envelope of voices 1 and 3
		// address: core0: 0x400-0x5ff, core1: 0xc00-0xdff
		s16 *pVoice1_Out;
		// address: core0: 0x600-0x7ff, core1: 0xe00-0xfff
		s16 *pVoice3_Out;
		
		// dry left/right after mixing all voices
		// address: core0: 0x1000-0x11ff, core1: 0x1800-19ff
		s16 *pDryLeft_Out;
		// address: core0: 0x1200-13ff, core1: 0x1a00-0x1bff
		s16 *pDryRight_Out;
		
		// wet left/right after mixing all voices
		// address: core0: 0x1400-0x15ff, core1: 0x1c00-0x1dff
		s16 *pWetLeft_Out;
		// address: core0: 0x1600-0x17ff, core1: 0x1e00-0x1fff
		s16 *pWetRight_Out;
		
		// output from core 0
		// address: 0x800-0x9ff
		s16 *pCore0Left_Out;
		// address: 0xa00-0xbff
		s16 *pCore0Right_Out;
		
		// these point to the sound data input area, which is 512 samples in size total for a channel (1024 bytes)
		s16 *pCoreSoundDataInputL;
		s16 *pCoreSoundDataInputR;
		
		// enables sound data input via ADMA
		u32 SoundDataInput_Enable;
		//u32 ulADMA_Active;
		//u32 ulADMAS_Output;
		
		// the sample offset to store incoming sound data at, the MSB (bit 8) determines L/R
		// always stores into the buffer that is not being read from in the double buffer
		// values range from 0-511
		u32 SoundDataInput_Offset;


		// bitmap indicating whether a loop should mute the sound or not
		// need to keep track of whether any wave blocks have the bit set to not loop
		// needs to be set to 1 at key-on for channel, and then cleared if loop not set in any blocks
		u32 bLoopSet;

		// need to know for which channels the loop point (LSA) was set manually by software
		// because in that case the loop point set by wave data gets ignored?
		u32 bLoopManuallySet;

		
		// sets the pointers that are not static
		// also need to call this AFTER loading a save state
		void Refresh ();
		
		
		///////////////////////////////////////////////////////////////////
		// register address voice data area offsets
		
		// this section gets repeated for each channel
		// runs a total of 16 bytes * 24 channels = 384 bytes
		// so on PS2, this goes from 0x1f90 0000 to 0x1f90 0180
		
		// global volume left
		static const u32 VOL_L = 0;
		#define VOLL_CH(ch) ( VOL_L + ( ch << 4 ) )
		#define VOLL_CRCH(cr,ch) ( VOL_L + ( ch << 4 ) + ( cr << 10 ) )
		
		// global volume right
		static const u32 VOL_R = 2;
		#define VOLR_CH(ch) ( VOL_R + ( ch << 4 ) )
		#define VOLR_CRCH(cr,ch) ( VOL_R + ( ch << 4 ) + ( cr << 10 ) )
		
		// pitch value
		static const u32 PITCH = 4;
		#define PITCH_CH(ch) ( PITCH + ( ch << 4 ) )
		#define PITCH_CRCH(cr,ch) ( PITCH + ( ch << 4 ) + ( cr << 10 ) )
		
		// start of sound address
		// for SPU2, this is located elsewhere
		//static const u32 SSA_X = 6;
		
		// adsr register 0
		// this changes for PS2
		//static const u32 ADSR_0 = 8;
		static const u32 ADSR_0 = 6;
		#define ADSR0_CH(ch) ( ADSR_0 + ( ch << 4 ) )
		#define ADSR0_CRCH(cr,ch) ( ADSR_0 + ( ch << 4 ) + ( cr << 10 ) )
		
		// adsr registr 1
		// this changes for PS2
		//static const u32 ADSR_1 = 0xa;
		static const u32 ADSR_1 = 8;
		#define ADSR1_CH(ch) ( ADSR_1 + ( ch << 4 ) )
		#define ADSR1_CRCH(cr,ch) ( ADSR_1 + ( ch << 4 ) + ( cr << 10 ) )
		
		// current envelope volume value
		// this changes for PS2
		//static const u32 ENV_X = 0xc;
		static const u32 ENV_X = 0xa;
		#define ENVX_CH(ch) ( ENV_X + ( ch << 4 ) )
		#define ENVX_CRCH(cr,ch) ( ENV_X + ( ch << 4 ) + ( cr << 10 ) )
		
		// sound loop start address
		// this is located elsewhere on PS2
		//static const u32 LSA_X = 0xe;
		
		// current voice volume left
		static const u32 CVOL_L = 0xc;
		#define CVOLL_CH(ch) ( CVOL_L + ( ch << 4 ) )
		#define CVOLL_CRCH(cr,ch) ( CVOL_L + ( ch << 4 ) + ( cr << 10 ) )
		
		// current voice volume right
		static const u32 CVOL_R = 0xe;
		#define CVOLR_CH(ch) ( CVOL_R + ( ch << 4 ) )
		#define CVOLR_CRCH(cr,ch) ( CVOL_R + ( ch << 4 ) + ( cr << 10 ) )
		
		//////////////////////////////////////////////////////////////////////
		// global sound register addresses
		
		// start of PS2 offsets for global sound register addresses
		
		// pitch modulation
		static const u32 PMON_0 = 0x180;
		static const u32 PMON_1 = 0x182;
		#define PMON0_CR(cr) ( PMON_0 + ( cr << 10 ) )
		#define PMON1_CR(cr) ( PMON_1 + ( cr << 10 ) )
		
		// noise generation
		static const u32 NON_0 = 0x184;
		static const u32 NON_1 = 0x186;
		#define NON0_CR(cr) ( NON_0 + ( cr << 10 ) )
		#define NON1_CR(cr) ( NON_1 + ( cr << 10 ) )
		
		// voice output mix left (dry)
		static const u32 VMIXL_0 = 0x188;
		static const u32 VMIXL_1 = 0x18a;
		#define VMIXL0_CR(cr) ( VMIXL_0 + ( cr << 10 ) )
		#define VMIXL1_CR(cr) ( VMIXL_1 + ( cr << 10 ) )
		
		// voice output mix left (wet)
		static const u32 VMIXEL_0 = 0x18c;
		static const u32 VMIXEL_1 = 0x18e;
		#define VMIXEL0_CR(cr) ( VMIXEL_0 + ( cr << 10 ) )
		#define VMIXEL1_CR(cr) ( VMIXEL_1 + ( cr << 10 ) )
		
		// voice output mix right (dry)
		static const u32 VMIXR_0 = 0x190;
		static const u32 VMIXR_1 = 0x192;
		#define VMIXR0_CR(cr) ( VMIXR_0 + ( cr << 10 ) )
		#define VMIXR1_CR(cr) ( VMIXR_1 + ( cr << 10 ) )
		
		// voice output mix right (wet)
		static const u32 VMIXER_0 = 0x194;
		static const u32 VMIXER_1 = 0x196;
		#define VMIXER0_CR(cr) ( VMIXER_0 + ( cr << 10 ) )
		#define VMIXER1_CR(cr) ( VMIXER_1 + ( cr << 10 ) )
		
		// output spec after voice mix
		static const u32 MMIX = 0x198;
		#define MMIX_CR(cr) ( MMIX + ( cr << 10 ) )
		
		// core x attrib
		// this is actually PROBABLY the exact or near same as the main SPU CTRL reg from PS1
		static const u32 CTRL = 0x19a;
		#define CTRL_CR(cr) ( CTRL + ( cr << 10 ) )
		
		// interrupt address
		static const u32 IRQA_1 = 0x19c;
		static const u32 IRQA_0 = 0x19e;
		#define IRQA0_CR(cr) ( IRQA_0 + ( cr << 10 ) )
		#define IRQA1_CR(cr) ( IRQA_1 + ( cr << 10 ) )
		
		// key on
		static const u32 KON_0 = 0x1a0;
		static const u32 KON_1 = 0x1a2;
		#define KON0_CR(cr) ( KON_0 + ( cr << 10 ) )
		#define KON1_CR(cr) ( KON_1 + ( cr << 10 ) )

		// key off
		static const u32 KOFF_0 = 0x1a4;
		static const u32 KOFF_1 = 0x1a6;
		#define KOFF0_CR(cr) ( KOFF_0 + ( cr << 10 ) )
		#define KOFF1_CR(cr) ( KOFF_1 + ( cr << 10 ) )
		
		// sound buffer address - next transfer to this address
		static const u32 SBA_1 = 0x1a8;
		static const u32 SBA_0 = 0x1aa;
		#define SBA0_CR(cr) ( SBA_0 + ( cr << 10 ) )
		#define SBA1_CR(cr) ( SBA_1 + ( cr << 10 ) )
		
		// data transfer
		static const u32 DATA = 0x1ac;
		#define DATA_CR(cr) ( DATA + ( cr << 10 ) )
		
		// ??
		static const u32 UNKNOWN0 = 0x1ae;

		// ?? auto dma status ??
		static const u32 ADMAS = 0x1b0;
		#define ADMAS_CR(cr) ( ADMAS + ( cr << 10 ) )

		// 1b2, 1b4, 1b6, 1b8, 1ba, 1bc, 1be are unknown ??
		
		// sound start address
		static const u32 SSA_1 = 0x1c0;
		static const u32 SSA_0 = 0x1c2;
		#define SSA0_CH(ch) ( SSA_0 + ( ch * 12 ) )
		#define SSA1_CH(ch) ( SSA_1 + ( ch * 12 ) )
		#define SSA0_CRCH(cr,ch) ( SSA_0 + ( cr << 10 ) + ( ch * 12 ) )
		#define SSA1_CRCH(cr,ch) ( SSA_1 + ( cr << 10 ) + ( ch * 12 ) )
		
		// loop start address
		static const u32 LSA_1 = 0x1c4;
		static const u32 LSA_0 = 0x1c6;
		#define LSA0_CH(ch) ( LSA_0 + ( ch * 12 ) )
		#define LSA1_CH(ch) ( LSA_1 + ( ch * 12 ) )
		#define LSA0_CRCH(cr,ch) ( LSA_0 + ( cr << 10 ) + ( ch * 12 ) )
		#define LSA1_CRCH(cr,ch) ( LSA_1 + ( cr << 10 ) + ( ch * 12 ) )

		// ?? waveform data that should be read next ??
		static const u32 NEX_1 = 0x1c8;
		static const u32 NEX_0 = 0x1ca;
		#define NEX0_CH(ch) ( NEX_0 + ( ch * 12 ) )
		#define NEX1_CH(ch) ( NEX_1 + ( ch * 12 ) )
		#define NEX0_CRCH(cr,ch) ( NEX_0 + ( cr << 10 ) + ( ch * 12 ) )
		#define NEX1_CRCH(cr,ch) ( NEX_1 + ( cr << 10 ) + ( ch * 12 ) )

		// .. repeated for each voice ..

		// reverb work area start
		static const u32 RVWAS_1 = 0x2e0;
		static const u32 RVWAS_0 = 0x2e2;
		#define RVWAS0_CR(cr) ( RVWAS_0 + ( cr << 10 ) )
		#define RVWAS1_CR(cr) ( RVWAS_1 + ( cr << 10 ) )
		
		
		// use 32-bit reads/writes for these
		static const u32 dAPF1_1 = 0x2e4;
		static const u32 dAPF1_0 = 0x2e6;
		static const u32 dAPF2_1 = 0x2e8;
		static const u32 dAPF2_0 = 0x2ea;
		static const u32 mLSAME_1 = 0x2ec;
		static const u32 mLSAME_0 = 0x2ee;
		static const u32 mRSAME_1 = 0x2f0;
		static const u32 mRSAME_0 = 0x2f2;
		static const u32 mLCOMB1_1 = 0x2f4;
		static const u32 mLCOMB1_0 = 0x2f6;
		static const u32 mRCOMB1_1 = 0x2f8;
		static const u32 mRCOMB1_0 = 0x2fa;
		static const u32 mLCOMB2_1 = 0x2fc;
		static const u32 mLCOMB2_0 = 0x2fe;
		static const u32 mRCOMB2_1 = 0x300;
		static const u32 mRCOMB2_0 = 0x302;
		static const u32 dLSAME_1 = 0x304;
		static const u32 dLSAME_0 = 0x306;
		static const u32 dRSAME_1 = 0x308;
		static const u32 dRSAME_0 = 0x30a;
		static const u32 mLDIFF_1 = 0x30c;
		static const u32 mLDIFF_0 = 0x30e;
		static const u32 mRDIFF_1 = 0x310;
		static const u32 mRDIFF_0 = 0x312;
		static const u32 mLCOMB3_1 = 0x314;
		static const u32 mLCOMB3_0 = 0x316;
		static const u32 mRCOMB3_1 = 0x318;
		static const u32 mRCOMB3_0 = 0x31a;
		static const u32 mLCOMB4_1 = 0x31c;
		static const u32 mLCOMB4_0 = 0x31e;
		static const u32 mRCOMB4_1 = 0x320;
		static const u32 mRCOMB4_0 = 0x322;
		static const u32 dLDIFF_1 = 0x324;
		static const u32 dLDIFF_0 = 0x326;
		static const u32 dRDIFF_1 = 0x328;
		static const u32 dRDIFF_0 = 0x32a;
		static const u32 mLAPF1_1 = 0x32c;
		static const u32 mLAPF1_0 = 0x32e;
		static const u32 mRAPF1_1 = 0x330;
		static const u32 mRAPF1_0 = 0x332;
		static const u32 mLAPF2_1 = 0x334;
		static const u32 mLAPF2_0 = 0x336;
		static const u32 mRAPF2_1 = 0x338;
		static const u32 mRAPF2_0 = 0x33a;
		#define dAPF10_CR(cr) ( dAPF1_0 + ( cr << 10 ) )
		#define dAPF11_CR(cr) ( dAPF1_1 + ( cr << 10 ) )
		#define dAPF20_CR(cr) ( dAPF2_0 + ( cr << 10 ) )
		#define dAPF21_CR(cr) ( dAPF2_1 + ( cr << 10 ) )
		#define mLSAME0_CR(cr) ( mLSAME_0 + ( cr << 10 ) )
		#define mLSAME1_CR(cr) ( mLSAME_1 + ( cr << 10 ) )
		#define mRSAME0_CR(cr) ( mRSAME_0 + ( cr << 10 ) )
		#define mRSAME1_CR(cr) ( mRSAME_1 + ( cr << 10 ) )
		#define mLCOMB10_CR(cr) ( mLCOMB1_0 + ( cr << 10 ) )
		#define mLCOMB11_CR(cr) ( mLCOMB1_1 + ( cr << 10 ) )
		#define mRCOMB10_CR(cr) ( mRCOMB1_0 + ( cr << 10 ) )
		#define mRCOMB11_CR(cr) ( mRCOMB1_1 + ( cr << 10 ) )
		#define mLCOMB20_CR(cr) ( mLCOMB2_0 + ( cr << 10 ) )
		#define mLCOMB21_CR(cr) ( mLCOMB2_1 + ( cr << 10 ) )
		#define mRCOMB20_CR(cr) ( mRCOMB2_0 + ( cr << 10 ) )
		#define mRCOMB21_CR(cr) ( mRCOMB2_1 + ( cr << 10 ) )
		#define dLSAME0_CR(cr) ( dLSAME_0 + ( cr << 10 ) )
		#define dLSAME1_CR(cr) ( dLSAME_1 + ( cr << 10 ) )
		#define dRSAME0_CR(cr) ( dRSAME_0 + ( cr << 10 ) )
		#define dRSAME1_CR(cr) ( dRSAME_1 + ( cr << 10 ) )
		#define mLDIFF0_CR(cr) ( mLDIFF_0 + ( cr << 10 ) )
		#define mLDIFF1_CR(cr) ( mLDIFF_1 + ( cr << 10 ) )
		#define mRDIFF0_CR(cr) ( mRDIFF_0 + ( cr << 10 ) )
		#define mRDIFF1_CR(cr) ( mRDIFF_1 + ( cr << 10 ) )
		#define mLCOMB30_CR(cr) ( mLCOMB3_0 + ( cr << 10 ) )
		#define mLCOMB31_CR(cr) ( mLCOMB3_1 + ( cr << 10 ) )
		#define mRCOMB30_CR(cr) ( mRCOMB3_0 + ( cr << 10 ) )
		#define mRCOMB31_CR(cr) ( mRCOMB3_1 + ( cr << 10 ) )
		#define mLCOMB40_CR(cr) ( mLCOMB4_0 + ( cr << 10 ) )
		#define mLCOMB41_CR(cr) ( mLCOMB4_1 + ( cr << 10 ) )
		#define mRCOMB40_CR(cr) ( mRCOMB4_0 + ( cr << 10 ) )
		#define mRCOMB41_CR(cr) ( mRCOMB4_1 + ( cr << 10 ) )
		#define dLDIFF0_CR(cr) ( dLDIFF_0 + ( cr << 10 ) )
		#define dLDIFF1_CR(cr) ( dLDIFF_1 + ( cr << 10 ) )
		#define dRDIFF0_CR(cr) ( dRDIFF_0 + ( cr << 10 ) )
		#define dRDIFF1_CR(cr) ( dRDIFF_1 + ( cr << 10 ) )
		#define mLAPF10_CR(cr) ( mLAPF1_0 + ( cr << 10 ) )
		#define mLAPF11_CR(cr) ( mLAPF1_1 + ( cr << 10 ) )
		#define mRAPF10_CR(cr) ( mRAPF1_0 + ( cr << 10 ) )
		#define mRAPF11_CR(cr) ( mRAPF1_1 + ( cr << 10 ) )
		#define mLAPF20_CR(cr) ( mLAPF2_0 + ( cr << 10 ) )
		#define mLAPF21_CR(cr) ( mLAPF2_1 + ( cr << 10 ) )
		#define mRAPF20_CR(cr) ( mRAPF2_0 + ( cr << 10 ) )
		#define mRAPF21_CR(cr) ( mRAPF2_1 + ( cr << 10 ) )
		
		
		/*
		#define R_FB_SRC_A       0x02E4		// Feedback Source A
		#define R_FB_SRC_B       0x02E8		// Feedback Source B
		#define R_IIR_DEST_A0    0x02EC
		#define R_IIR_DEST_A1    0x02F0
		#define R_ACC_SRC_A0     0x02F4
		#define R_ACC_SRC_A1     0x02F8
		#define R_ACC_SRC_B0     0x02FC
		#define R_ACC_SRC_B1     0x0300
		#define R_IIR_SRC_A0     0x0304
		#define R_IIR_SRC_A1     0x0308
		#define R_IIR_DEST_B0    0x030C
		#define R_IIR_DEST_B1    0x0310
		#define R_ACC_SRC_C0     0x0314
		#define R_ACC_SRC_C1     0x0318
		#define R_ACC_SRC_D0     0x031C
		#define R_ACC_SRC_D1     0x0320
		#define R_IIR_SRC_B0     0x0324		// Some sources have R_IIR_SRC_B0 and R_IIR_SRC_B1 swapped ><
		#define R_IIR_SRC_B1     0x0328		// Assume a typo in the docs and B0 is actually at 324, B1 at 328 in the HW.
		#define R_MIX_DEST_A0    0x032C
		#define R_MIX_DEST_A1    0x0330
		#define R_MIX_DEST_B0    0x0334
		#define R_MIX_DEST_B1    0x0338
		*/


		// reverb work area end
		static const u32 RVWAE_1 = 0x33c;
		static const u32 RVWAE_0 = 0x33e;
		#define RVWAE0_CR(cr) ( RVWAE_0 + ( cr << 10 ) )
		#define RVWAE1_CR(cr) ( RVWAE_1 + ( cr << 10 ) )

		// endpoint flag - this is like channel on or something in spu1 code
		static const u32 CON_0 = 0x340;
		static const u32 CON_1 = 0x342;
		#define CON0_CR(cr) ( CON_0 + ( cr << 10 ) )
		#define CON1_CR(cr) ( CON_1 + ( cr << 10 ) )

		// ?? status ??
		// might be init??
		// no, no, this is the "STAT" register from PS1
		static const u32 STAT = 0x344;
		//static const u32 INIT = 0x344;
		#define STAT_CR(cr) ( STAT + ( cr << 10 ) )

		// 0x346 .. 0x3fe are unknown (unused?)

		// core 1 has the same registers with 0x400 added.
		// core 1 ends at 0x746

		// 0x746 .. 0x75e are unknown
		
		
		// another register area
		
		// master volume L/R
		static const u32 MVOL_L = 0x760;
		static const u32 MVOL_R = 0x762;
		#define MVOLL_CR(cr) ( MVOL_L + ( cr * 40 ) )
		#define MVOLR_CR(cr) ( MVOL_R + ( cr * 40 ) )
		
		// effect/reverb volume L/R
		static const u32 EVOL_L = 0x764;
		static const u32 EVOL_R = 0x766;
		static const u32 vLOUT = 0x764;
		static const u32 vROUT = 0x766;
		#define vLOUT_CR(cr) ( vLOUT + ( cr * 40 ) )
		#define vROUT_CR(cr) ( vROUT + ( cr * 40 ) )
		
		// core external input volume L/R (only for core1)
		static const u32 AVOL_L = 0x768;
		static const u32 AVOL_R = 0x76a;
		#define AVOLL_CR(cr) ( AVOL_L + ( cr * 40 ) )
		#define AVOLR_CR(cr) ( AVOL_R + ( cr * 40 ) )
		
		// sound data volume L/R
		static const u32 BVOL_L = 0x76c;
		static const u32 BVOL_R = 0x76e;
		#define BVOLL_CR(cr) ( BVOL_L + ( cr * 40 ) )
		#define BVOLR_CR(cr) ( BVOL_R + ( cr * 40 ) )
		
		// current master volume L/R
		static const u32 CMVOL_L = 0x770;
		static const u32 CMVOL_R = 0x772;
		#define CMVOLL_CR(cr) ( CMVOL_L + ( cr * 40 ) )
		#define CMVOLR_CR(cr) ( CMVOL_R + ( cr * 40 ) )
		
		static const u32 vIIR = 0x774;
		static const u32 vCOMB1 = 0x776;
		static const u32 vCOMB2 = 0x778;
		static const u32 vCOMB3 = 0x77a;
		static const u32 vCOMB4 = 0x77c;
		static const u32 vWALL = 0x77e;
		static const u32 vAPF1 = 0x780;
		static const u32 vAPF2 = 0x782;
		static const u32 vLIN = 0x784;
		static const u32 vRIN = 0x786;
		#define vIIR_CR(cr) ( vIIR + ( cr * 40 ) )
		#define vCOMB1_CR(cr) ( vCOMB1 + ( cr * 40 ) )
		#define vCOMB2_CR(cr) ( vCOMB2 + ( cr * 40 ) )
		#define vCOMB3_CR(cr) ( vCOMB3 + ( cr * 40 ) )
		#define vCOMB4_CR(cr) ( vCOMB4 + ( cr * 40 ) )
		#define vWALL_CR(cr) ( vWALL + ( cr * 40 ) )
		#define vAPF1_CR(cr) ( vAPF1 + ( cr * 40 ) )
		#define vAPF2_CR(cr) ( vAPF2 + ( cr * 40 ) )
		#define vLIN_CR(cr) ( vLIN + ( cr * 40 ) )
		#define vRIN_CR(cr) ( vRIN + ( cr * 40 ) )

		
		/*
		#define R_IIR_ALPHA      0x0774		//IIR alpha (% used)
		#define R_ACC_COEF_A     0x0776
		#define R_ACC_COEF_B     0x0778
		#define R_ACC_COEF_C     0x077A
		#define R_ACC_COEF_D     0x077C
		#define R_IIR_COEF       0x077E
		#define R_FB_ALPHA       0x0780		//feedback alpha (% used)
		#define R_FB_X           0x0782		//feedback
		#define R_IN_COEF_L      0x0784
		#define R_IN_COEF_R      0x0786
		*/
		
		// values repeat for core1

		
		// register area for all cores
		
		static const u32 SPDIF_OUT = 0x7c0;
		static const u32 SPDIF_IRQINFO = 0x7c2;
		static const u32 SPDIF_MODE = 0x7c6;
		static const u32 SPDIF_MEDIA = 0x7c8;
		static const u32 SPDIF_PROTECT = 0x7cc;

		


		
		
		// master volume left
		/*
		static const u32 MVOL_L = 0x1f801d80;
		
		// master volume right
		static const u32 MVOL_R = 0x1f801d82;
		
		// effect volume left
		static const u32 EVOL_L = 0x1f801d84;
		
		// effect volume right
		static const u32 EVOL_R = 0x1f801d86;
		
		// voice key on
		static const u32 KON_0 = 0x1f801d88;
		static const u32 KON_1 = 0x1f801d8a;
		
		// voice key off
		static const u32 KOFF_0 = 0x1f801d8c;
		static const u32 KOFF_1 = 0x1f801d8e;
		
		// pitch modulation on
		static const u32 PMON_0 = 0x1f801d90;
		static const u32 PMON_1 = 0x1f801d92;
		
		// noise on - set voice to noise
		static const u32 NON_0 = 0x1f801d94;
		static const u32 NON_1 = 0x1f801d96;
		
		// reverb on - process reverb for channel
		static const u32 RON_0 = 0x1f801d98;
		static const u32 RON_1 = 0x1f801d9a;
		
		// channel on/off - returns whether channel is mute or not
		static const u32 CON_0 = 0x1f801d9c;
		static const u32 CON_1 = 0x1f801d9e;
		
		// reverb work area start - address in sound buffer divided by eight
		static const u32 RVWA = 0x1f801da2;
		
		// sound buffer IRQ address - address in sound buffer divided by eight
		static const u32 IRQA = 0x1f801da4;
		
		// sound buffer address - next transfer to this address - address divided by eight
		static const u32 SBA = 0x1f801da6;
		
		// spu data - forwards data to SPU RAM at SBA_X when you write to it
		static const u32 DATA = 0x1f801da8;
		
		// spu control
		static const u32 CTRL = 0x1f801daa;
		
		// spu init
		static const u32 INIT = 0x1f801dac;
		
		// spu status
		static const u32 STAT = 0x1f801dae;
		
		// cd volume left/right
		static const u32 CDVOL_L = 0x1f801db0;
		static const u32 CDVOL_R = 0x1f801db2;
		
		// extern volume left/right
		static const u32 EXTVOL_L = 0x1f801db4;
		static const u32 EXTVOL_R = 0x1f801db6;
		
		
		
		// reverb registers //
		// 0x1f801dc0 - 0x1f801dff //
		
		static const u32 vLOUT = 0x1f801d84;
		static const u32 vROUT = 0x1f801d86;
		static const u32 mBASE = 0x1f801da2;
		
		static const u32 dAPF1 = 0x1f801dc0;
		static const u32 dAPF2 = 0x1f801dc2;
		static const u32 vIIR = 0x1f801dc4;
		static const u32 vCOMB1 = 0x1f801dc6;
		static const u32 vCOMB2 = 0x1f801dc8;
		static const u32 vCOMB3 = 0x1f801dca;
		static const u32 vCOMB4 = 0x1f801dcc;
		static const u32 vWALL = 0x1f801dce;
		static const u32 vAPF1 = 0x1f801dd0;
		static const u32 vAPF2 = 0x1f801dd2;
		static const u32 mLSAME = 0x1f801dd4;
		static const u32 mRSAME = 0x1f801dd6;
		static const u32 mLCOMB1 = 0x1f801dd8;
		static const u32 mRCOMB1 = 0x1f801dda;
		static const u32 mLCOMB2 = 0x1f801ddc;
		static const u32 mRCOMB2 = 0x1f801dde;
		static const u32 dLSAME = 0x1f801de0;
		static const u32 dRSAME = 0x1f801de2;
		static const u32 mLDIFF = 0x1f801de4;
		static const u32 mRDIFF = 0x1f801de6;
		static const u32 mLCOMB3 = 0x1f801de8;
		static const u32 mRCOMB3 = 0x1f801dea;
		static const u32 mLCOMB4 = 0x1f801dec;
		static const u32 mRCOMB4 = 0x1f801dee;
		static const u32 dLDIFF = 0x1f801df0;
		static const u32 dRDIFF = 0x1f801df2;
		static const u32 mLAPF1 = 0x1f801df4;
		static const u32 mRAPF1 = 0x1f801df6;
		static const u32 mLAPF2 = 0x1f801df8;
		static const u32 mRAPF2 = 0x1f801dfa;
		static const u32 vLIN = 0x1f801dfc;
		static const u32 vRIN = 0x1f801dfe;
		*/
		
		
		// quick access pointers
		static s16 *_vLOUT;
		static s16 *_vROUT;
		static u16 *_mBASE;
		
		static u16 *_dAPF1;
		static u16 *_dAPF2;
		static s16 *_vIIR;
		static s16 *_vCOMB1;
		static s16 *_vCOMB2;
		static s16 *_vCOMB3;
		static s16 *_vCOMB4;
		static s16 *_vWALL;
		static s16 *_vAPF1;
		static s16 *_vAPF2;
		static u16 *_mLSAME;
		static u16 *_mRSAME;
		static u16 *_mLCOMB1;
		static u16 *_mRCOMB1;
		static u16 *_mLCOMB2;
		static u16 *_mRCOMB2;
		static u16 *_dLSAME;
		static u16 *_dRSAME;
		static u16 *_mLDIFF;
		static u16 *_mRDIFF;
		static u16 *_mLCOMB3;
		static u16 *_mRCOMB3;
		static u16 *_mLCOMB4;
		static u16 *_mRCOMB4;
		static u16 *_dLDIFF;
		static u16 *_dRDIFF;
		static u16 *_mLAPF1;
		static u16 *_mRAPF1;
		static u16 *_mLAPF2;
		static u16 *_mRAPF2;
		static s16 *_vLIN;
		static s16 *_vRIN;


		// current volume registers //
		/*
		// current main volume
		static const u32 CMVOL_L = 0x1f801db8;
		static const u32 CMVOL_R = 0x1f801dba;
		
		// current channel volume
		static const u32 CVOL_SIZE = 0x4;
		static const u32 CVOL_L_START = 0x1f801e00;
		static const u32 CVOL_R_START = 0x1f801e02;
		*/
		
		////////////////////////////////////////////////////////////////////////
		// 0x1f801c00-0x1f801d7f
		
		
		///////////////////////////////////////////////////////
		// 0x1f801xx0 Volume Left
		// 0x1f801xx2 Volume Right
		struct VolumeLR
		{
			// bits 0-13
			u16 VoiceVolume : 14;
			
			// 0-Phase normal; 1-Phase inverted
			u16 S : 1;
			
			u16 zero0 : 1;
		};

		
		
		///////////////////////////////////////////////////////
		// 0x1f801xx0 Volume Left Sweep Mode
		// 0x1f801xx2 Volume Right Sweep Mode
		struct VolumeSweepLR
		{
			// bits 0-6
			u16 VoiceVolume : 7;
			
			u16 zero0 : 5;
			
			// bit 12
			// 0-Phase normal; 1-Phase inverted
			u16 Ph : 1;
			
			// bit 13
			// 0-increase; 1-decrease
			u16 Dr : 1;
			
			// bit 14
			// 0-linear slope; 1-exponential slope
			u16 Sl : 1;
			
			u16 One0 : 1;
		};
		
		////////////////////////////////////////////////////////////////////////////////////////
		// Linear sweep up/down is accomplished with a fixed add/mul per SPU clock (1/41000)
		// for this we would need to store current volume in 16.16 since add/mul value is 16.16
		// for exponential sweep up, this value changes at volume 0x6000
		
		// constant value to add or multiply with volume for channel
		u32 VOL_L_Constant [ 24 ];
		
		// the constant to use above 75% or 0x6000
		u32 VOL_L_Constant75 [ 24 ];
		
		// current value of the volume for channel
		s64 VOL_L_Value [ 24 ];
		
		u32 VOL_L_Cycles [ 24 ];
		
		// need that for both left and right
		u32 VOL_R_Constant [ 24 ];
		u32 VOL_R_Constant75 [ 24 ];
		s64 VOL_R_Value [ 24 ];
		
		u32 VOL_R_Cycles [ 24 ];
		
		//static void SetSweepVars ( u16 flags, u32 Channel, s32* Rates, s32* Rates75 );
		//static void SweepVolume ( u16 flags, s64& CurrentVolume, u32 VolConstant, u32 VolConstant75 );
		
		static s64 Get_VolumeStep ( s16& Level, u32& Cycles, u32 Value, u32 flags );
		static void Start_VolumeEnvelope ( s16& Level, u32& Cycles, u32 Value, u32 flags, bool InitLevel = false );
		static void VolumeEnvelope ( s16& Level, u32& Cycles, u32 Value, u32 flags, bool clamp = true );
		
		
		
		////////////////////////////////////////////////////////////////////
		// 0x1f801xx4 Pitch
		union Pitch
		{
			struct
			{
				// bits 0-13 - the pitch
				// 0x1000 - sample pitch; 0x0800: -1 octave; 0x0400: -2 octaves, etc; 0x2000: +1 octave; 0x3fff: +2 octaves, etc
				// SampleRate = 44100 * ( Pitch / 0x1000 )
				u16 Pt : 14;
				
				u16 zero0 : 2;
			};
			
			u16 Value;
		};
		
		/////////////////////////////////////////////////////////
		// 0x1f801xx6 Start Address Of Sound In Buffer
		
		/////////////////////////////////////////////////////////
		// 0x1f801xx8 Attack Rate, Decay Rate, Sustain Level
		union AttackDecayRate
		{
			struct
			{
				// bits 0-3: Sustain level
				u16 Sl : 4;
				
				// bits 4-7: Decay Rate
				u16 Dr : 4;
				
				// bits 8-14: Attack Rate
				u16 Ar : 7;
				
				// bit 15: Attack Mode
				// 0-Linear increase; 1-Exponential increase
				u16 Am : 1;
			};
			
			u16 Value;
		};
		
		///////////////////////////////////////////////////////////////
		// 0x1f801xxa Sustain Rate, Release Rate
		// Note: Decay mode is always exponential decrease
		union SustainReleaseRate
		{
			struct
			{
				// bits 0-4: Release Rate
				u16 Rr : 5;
				
				// bit 5: Release Mode
				// 0-Linear decrease; 1-Exponential decrease
				u16 Rm : 1;
				
				// bits 6-12: Sustain Rate
				u16 Sr : 7;
				
				u16 zero0 : 1;
				
				// bit 14: Sustain Rate Mode Increase/Decrease
				// 0-increase; 1-decrease
				u16 Sd : 1;
				
				// bit 15: Sustain Rate Mode Linear/Exponential
				// 0-linear; 1-exponential
				u16 Sm : 1;
			};
			
			u16 Value;
		};
		
		//SustainReleaseRate SustainReleaseRate_Reg [ 24 ];

		s32 VOL_ATTACK_Constant [ 24 ];
		s32 VOL_ATTACK_Constant75 [ 24 ];
		s32 VOL_DECAY_Constant [ 24 ];
		s32 VOL_SUSTAIN_Constant [ 24 ];
		s32 VOL_SUSTAIN_Constant75 [ 24 ];
		s32 VOL_RELEASE_Constant [ 24 ];
		
		// also need sustain level
		s64 VOL_SUSTAIN_Level [ 24 ];
		
		u16 ADSR_Status [ 24 ];
		enum { ADSR_MUTE, ADSR_ATTACK, ADSR_DECAY, ADSR_SUSTAIN, ADSR_RELEASE };
		
		////////////////////////////////////////////////////////////////////////////////////
		// 0x1f801xxc Current ADSR volume - returns the 16-bit ADSR volume when read
		
		// will store internally in 16.16 fixed point here
		s64 VOL_ADSR_Value [ 24 ];

		
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 0x1f801xxe Repeat Address
		// setting this register only has effect after voice has started, else loop address gets reset by sample

		//u16 RepeatAddress_Reg [ 24 ];
		
		/////////////////////////////////////////////////////////////////
		// 0x1f801d80 Main Volume Left
		// 0x1f801d82 Main Volume Right
		// Note: these work the same as the channel volume registers
		
		s32 MVOL_L_Constant;
		s32 MVOL_L_Constant75;
		s64 MVOL_L_Value;
		
		u32 MVOL_L_Cycles;
		
		s32 MVOL_R_Constant;
		s32 MVOL_R_Constant75;
		s64 MVOL_R_Value;
		
		u32 MVOL_R_Cycles;

		
		////////////////////////////////////////////////////////////////////////////
		// 0x1f801d84 Reverberation depth Left
		// 0x1f801d86 Reverberation depth Right
		
		union ReverbDepthLR
		{
			struct
			{
				// bits 0-14: Sets the wet volume for the effect
				// 0x0000-0x7fff
				u16 Rvd : 15;
				
				// bit 15: Phase
				// 0-normal; 1-inverted
				u16 P : 1;
			};
			
			u16 Value;
		};
		
		
		/////////////////////////////////////////////////////////////////////////////
		// 0x1f801d88 Voice On (0-15)
		// 0x1f801d8a Voice On (16-23)
		
		union VoiceOn0
		{
			struct
			{
				// 0-mode for channelX off; 1-mode for channelX on
				u16 c0 : 1;
				u16 c1 : 1;
				u16 c2 : 1;
				u16 c3 : 1;
				u16 c4 : 1;
				u16 c5 : 1;
				u16 c6 : 1;
				u16 c7 : 1;
				u16 c8 : 1;
				u16 c10 : 1;
				u16 c11 : 1;
				u16 c12 : 1;
				u16 c13 : 1;
				u16 c14 : 1;
				u16 c15 : 1;
			};
			
			u16 Value;
		};
		
		union VoiceOn1
		{
			struct
			{
				u16 c16 : 1;
				u16 c17 : 1;
				u16 c18 : 1;
				u16 c19 : 1;
				u16 c20 : 1;
				u16 c21 : 1;
				u16 c22 : 1;
				u16 c23 : 1;
			};
			
			u16 Value;
		};
		
		u32 VoiceOn_Bitmap;
		
		////////////////////////////////////////////////////////////////////
		// 0x1f801d8c Voice Off (0-15)
		// 0x1f801d8e Voice Off (16-23)

		union VoiceOff0
		{
			struct
			{
				// 0-mode for channelX off; 1-mode for channelX on
				u16 c0 : 1;
				u16 c1 : 1;
				u16 c2 : 1;
				u16 c3 : 1;
				u16 c4 : 1;
				u16 c5 : 1;
				u16 c6 : 1;
				u16 c7 : 1;
				u16 c8 : 1;
				u16 c10 : 1;
				u16 c11 : 1;
				u16 c12 : 1;
				u16 c13 : 1;
				u16 c14 : 1;
				u16 c15 : 1;
			};
			
			u16 Value;
		};
		
		union VoiceOff1
		{
			struct
			{
				u16 c16 : 1;
				u16 c17 : 1;
				u16 c18 : 1;
				u16 c19 : 1;
				u16 c20 : 1;
				u16 c21 : 1;
				u16 c22 : 1;
				u16 c23 : 1;
			};
			
			u16 Value;
		};
		
		///////////////////////////////////////////////////////////////////////////
		// 0x1f801d90 Channel FM (pitch lfo) mode (0-15)
		// 0x1f801d92 Channel FM (pitch lfo) mode (16-23)
		// sets the channel frequency modulation. Uses previous channel as modulator
		
		union FMPitchMode0
		{
			struct
			{
				// 0-mode for channelX off; 1-mode for channelX on
				u16 c0 : 1;
				u16 c1 : 1;
				u16 c2 : 1;
				u16 c3 : 1;
				u16 c4 : 1;
				u16 c5 : 1;
				u16 c6 : 1;
				u16 c7 : 1;
				u16 c8 : 1;
				u16 c10 : 1;
				u16 c11 : 1;
				u16 c12 : 1;
				u16 c13 : 1;
				u16 c14 : 1;
				u16 c15 : 1;
			};
			
			u16 Value;
		};
		
		union FMPitchMode1
		{
			struct
			{
				u16 c16 : 1;
				u16 c17 : 1;
				u16 c18 : 1;
				u16 c19 : 1;
				u16 c20 : 1;
				u16 c21 : 1;
				u16 c22 : 1;
				u16 c23 : 1;
			};
			
			u16 Value;
		};
		
		
		/////////////////////////////////////////////////////////////
		// 0x1f801d94 Channel Noise mode (0-15)
		// 0x1f801d96 Channel Noise mode (16-23)
		
		union NoiseMode0
		{
			struct
			{
				// 0-mode for channelX off; 1-mode for channelX on
				u16 c0 : 1;
				u16 c1 : 1;
				u16 c2 : 1;
				u16 c3 : 1;
				u16 c4 : 1;
				u16 c5 : 1;
				u16 c6 : 1;
				u16 c7 : 1;
				u16 c8 : 1;
				u16 c10 : 1;
				u16 c11 : 1;
				u16 c12 : 1;
				u16 c13 : 1;
				u16 c14 : 1;
				u16 c15 : 1;
			};
			
			u16 Value;
		};
		
		union NoiseMode1
		{
			struct
			{
				u16 c16 : 1;
				u16 c17 : 1;
				u16 c18 : 1;
				u16 c19 : 1;
				u16 c20 : 1;
				u16 c21 : 1;
				u16 c22 : 1;
				u16 c23 : 1;
			};
			
			u16 Value;
		};
		
		
		////////////////////////////////////////////////////////////////////////
		// 0x1f801d98 Channel Reverb mode (0-15)
		// 0x1f801d9a Channel Reverb mode (16-23)
		
		union ReverbMode0
		{
			struct
			{
				// 0-mode for channelX off; 1-mode for channelX on
				u16 c0 : 1;
				u16 c1 : 1;
				u16 c2 : 1;
				u16 c3 : 1;
				u16 c4 : 1;
				u16 c5 : 1;
				u16 c6 : 1;
				u16 c7 : 1;
				u16 c8 : 1;
				u16 c10 : 1;
				u16 c11 : 1;
				u16 c12 : 1;
				u16 c13 : 1;
				u16 c14 : 1;
				u16 c15 : 1;
			};
			
			u16 Value;
		};
		
		union ReverbMode1
		{
			struct
			{
				u16 c16 : 1;
				u16 c17 : 1;
				u16 c18 : 1;
				u16 c19 : 1;
				u16 c20 : 1;
				u16 c21 : 1;
				u16 c22 : 1;
				u16 c23 : 1;
			};
			
			u16 Value;
		};
		
		
		/////////////////////////////////////////////////////////////////////////
		// 0x1f801d9c Channel ON/OFF (0-15)
		// 0x1f801d9e Channel ON/OFF (16-23)

		union ChannelOnOff0
		{
			struct
			{
				// 0-mode for channelX off; 1-mode for channelX on
				u16 c0 : 1;
				u16 c1 : 1;
				u16 c2 : 1;
				u16 c3 : 1;
				u16 c4 : 1;
				u16 c5 : 1;
				u16 c6 : 1;
				u16 c7 : 1;
				u16 c8 : 1;
				u16 c10 : 1;
				u16 c11 : 1;
				u16 c12 : 1;
				u16 c13 : 1;
				u16 c14 : 1;
				u16 c15 : 1;
			};
			
			u16 Value;
		};
		
		union ChannelOnOff1
		{
			struct
			{
				u16 c16 : 1;
				u16 c17 : 1;
				u16 c18 : 1;
				u16 c19 : 1;
				u16 c20 : 1;
				u16 c21 : 1;
				u16 c22 : 1;
				u16 c23 : 1;
			};
			
			u16 Value;
		};
		
		
		/////////////////////////////////////////////////////////////////////////////////////
		// 0x1f801da2 Reverb work area start
		
		////////////////////////////////////////////////////////////////////////////////////////
		// 0x1f801da4 Sound buffer IRQ address
		
		/////////////////////////////////////////////////////////////////////
		// 0x1f801da6 Sound buffer address
		
		///////////////////////////////////////////////////////////////////
		// 0x1f801da8 SPU data
		
		////////////////////////////////////////////////////////
		// 0x1f801daa SPU control
		union SPU_CTRL
		{
			struct
			{
				// bit 0 - cd audio off/on
				// 0: off; 1: on
				u16 Ce : 1;
				
				// bit 1 - External audio off/on
				// 0: off; 1: on
				u16 Ee : 1;
				
				// bit 2 - Reverb for cd off/on
				// 0: off; 1: on
				u16 Cr : 1;
				
				// bit 3 - Reverb for external off/on
				// 0: off; 1: on
				u16 Er : 1;
				
				// bits 4-5 - DMA transfer direction
				// 01: Non-DMA write; 10: DMA Write; 11: DMA Read
				u16 DMA : 2;
				
				// bit 6 - IRQ enable/disable
				// 0: Irq disabled; 1: Irq enabled
				u16 Irq : 1;
				
				// bit 7 - Reverb enable/disable
				// 0: disabled; 1: enabled
				u16 Rv : 1;
				
				// bits 8-13 - Noise clock frequency
				u16 Noise : 6;
				
				// bit 14 - Mute on/off
				// 0: Mute SPU; 1: Unmute SPU
				u16 Mu : 1;
				
				// bit 15 - SPU off/on
				// 0: SPU off; 1: SPU on
				u16 En : 1;
			};
			
			u16 Value;
		};
		
		
		////////////////////////////////////////////////////////////////////
		// 0x1f801dac SPU status
		
		/////////////////////////////////////////////////////////////////
		// 0x1f801dae SPU status
		
		union SPU_STAT
		{
			struct
			{
				u16 Unknown : 10;
				
				// bit 10 - SPU ready/not ready
				// 0: Spu ready; 1: SPU not ready
				u16 Rd : 1;
				
				// bit 11 - Buffer decoding status
				// 0: decoding in first half of buffer; 1: decoding in second half of buffer
				u16 Dh : 1;
				
				u16 Unknown1 : 4;
			};
			
			u16 Value;
		};
		
		
		///////////////////////////////////////////////////////////////////////////////
		// 0x1f801db0 CD volume left
		// 0x1f801db2 CD volume right
		
		union CDVolume
		{
			struct
			{
				// bits 0-14 - Sets volume of cd input
				u16 CDVol : 15;
				
				// bits 15 - Phase
				// 0: Normal phase; 1: Inverted phase
				u16 P : 1;
			};
			
			u16 Value;
		};
		
		
		/////////////////////////////////////////////////////////////////////////////////////
		// 0x1f801db4 Extern volume left
		// 0x1f801db6 Extern volume right
		
		union ExternVolume
		{
			struct
			{
				// bits 0-14 - Sets volume of external input
				u16 ExVol : 15;
				
				// bits 15 - Phase
				// 0: Normal phase; 1: Inverted phase
				u16 P : 1;
			};
			
			u16 Value;
		};
		
		
		
		//////////////////////////////////////////////////////////////////////////
		// Sound Buffer Address
		// - Address in sound buffer divided by eight
		// - Next transfer to 0x1f801da8 gets sent to this address in Sound RAM
		//static const u32 REG_SoundBufferAddress = 0x1f801da6;
		u64 NextSoundBufferAddress;
		
		// need to know when manual write buffer is full
		u32 ManualWriteBuffer_isFull;
		
		///////////////////////////////////////////////////////////////////////
		// Data forwarding reg
		// - data gets sent to sound buffer via non-dma transfers
		//static const u32 REG_DataForwarding = 0x1f801da8;
		
		
		// number of cycles to wait before running device
		//u32 WaitCycles;

		static const u32 c_EncodedBlockSize = 16;
		static const u32 c_SamplesPerBlock = 28;

		union EncodedSoundBlock
		{
			struct
			{
				// predictor number and shift factor
				union
				{
					struct
					{
						u8 shift_factor : 4;
						u8 predictor_number : 4;
					};
					
					u8 pack_info;
				};
				
				union
				{
					struct
					{
						// not used
						//u8 NotUsed0 : 1;
						
						// bit 0 - LOOP_END - when set indicates that this is the last block of waveform data
						u8 LOOP_END : 1;
						
						// bit 1 - LOOP - must be set in every block if there is a loop point
						u8 LOOP : 1;
						
						// bit 2 - LOOP_START - when set sets loop address to the beginning of the current block
						u8 LOOP_START : 1;
						
						u8 NotUsed1 : 4;
					};
					
					u8 flags;
				};
				
				u8 EncodedSoundData [ 14 ];
			};
			
			u8 Value [ 16 ];
		};


		
		
		// for interrupt call back
		static funcVoid UpdateInterrupts;
		static void Set_IntCallback ( funcVoid UpdateInt_CB ) { UpdateInterrupts = UpdateInt_CB; };
		
		
		static const u32 c_InterruptBit = 9;
		
		static u32* _Intc_Stat;
		static u32* _Intc_Mask;
		static u32* _R3000A_Status_12;
		static u32* _R3000A_Cause_13;
		static u64* _ProcStatus;
		
		static inline void ConnectInterrupt ( u32* _IStat, u32* _IMask, u32* _R3000A_Status, u32* _R3000A_Cause, u64* _ProcStat )
		{
			_Intc_Stat = _IStat;
			_Intc_Mask = _IMask;
			_R3000A_Cause_13 = _R3000A_Cause;
			_R3000A_Status_12 = _R3000A_Status;
			_ProcStatus = _ProcStat;
		}
		
		
		static inline void SetInterrupt ()
		{
			*_Intc_Stat |= ( 1 << c_InterruptBit );
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}
		
		// call this after "SetInterrupt"
		// sets the extra interrupt bits on PS2 in SPDIF registers ??
		static inline void SetInterrupt_Core ( int Core )
		{
#ifdef SET_SPDIF_OUT_INTERRUPT
			// here too ??
			// note: bit 2 in spdif_out actually means to use 24/32-bit pcm data streaming
			//lSPDIF_OUT |= ( 0x4 << Core );
			GET_REG16 ( SPDIF_OUT ) |= ( 0x4 << Core );
#endif
			
			// need to check if this gets cleared after read ??
			//lSPDIF_IRQINFO |= ( 0x4 << Core );
			GET_REG16 ( SPDIF_IRQINFO ) |= ( 0x4 << Core );
		}
		
		static inline void ClearInterrupt ()
		{
			*_Intc_Stat &= ~( 1 << c_InterruptBit );
			
			UpdateInterrupts ();
			
			/*
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}
		
		// this is for the interface
		/*
		static u64* _NextSystemEvent;
		
		inline void Set_NextEvent ( u64 Cycle )
		{
			NextEvent_Cycle = Cycle + *_DebugCycleCount;
			if ( NextEvent_Cycle > *_DebugCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_DebugCycleCount ) ) *_NextSystemEvent = NextEvent_Cycle;
		}
		*/
		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		static u64* _SystemCycleCount;
		
		// ***TODO*** this stuff is part of the interface
		// object debug stuff
		// Enable/Disable debug window for object
		// Windows API specific code is in here
		static bool DebugWindow_Enabled [ 2 ];
		static WindowClass::Window *DebugWindow [ 2 ];
		static DebugValueList<u16> *SPUMaster_ValueList [ 2 ];
		static DebugValueList<u16> *SPU_ValueList [ 24 ] [ 2 ];
		static Debug_MemoryViewer *SoundRAM_Viewer [ 2 ];
		static void DebugWindow_Enable ( int Number );
		static void DebugWindow_Disable ( int Number );
		static void DebugWindow_Update ( int Number );
		
	private:
	
		/*
		inline static void Set_LinearIncRates ( u32 Channel, u32 Rate, s32* Rates, s32* Rates75 )
		{
			// check if mode is less than 48
			// timing is in first 7 bits
			if ( Rate < 48 )
			{
				// use (7 - (RATE & 3)) <<(11 - (RATE>> 2))
				// shifting by 16 because this is in 16.16 fixed point
				Rates [ Channel ] = ( ( 7 - ( Rate & 3 ) ) << ( 11 - ( Rate >> 2 ) ) ) << 16;
				Rates75 [ Channel ] = ( ( 7 - ( Rate & 3 ) ) << ( 11 - ( Rate >> 2 ) ) ) << 16;
			}
			else
			{
				// use 7 - (RATE & 3) / (1 <<((RATE>> 2) - 11))
				Rates [ Channel ] = ( ( 7 - ( Rate & 3 ) ) << 16 ) >> ( ( Rate >> 2 ) - 11 );
				Rates75 [ Channel ] = ( ( 7 - ( Rate & 3 ) ) << 16 ) >> ( ( Rate >> 2 ) - 11 );
			}
			
			if ( Rate == 127 )
			{
				// rate is max, so set Constant to zero
				Rates [ Channel ] = 0;
				Rates75 [ Channel ] = 0;
			}
		}

		inline static void Set_LinearDecRates ( u32 Channel, u32 Rate, s32* Rates, s32* Rates75 )
		{
			// check if mode is less than 48
			// timing is in first 7 bits
			if ( Rate < 48 )
			{
				// use (-8 + (RATE & 3)) <<(11 - (RATE>> 2))
				// shifting by 16 because this is in 16.16 fixed point
				Rates [ Channel ] = ( ( -8 + ( Rate & 3 ) ) << ( 11 - ( Rate >> 2 ) ) ) << 16;
				Rates75 [ Channel ] = ( ( -8 + ( Rate & 3 ) ) << ( 11 - ( Rate >> 2 ) ) ) << 16;
			}
			else
			{
				// use -8 + (RATE & 3) / (1 <<((RATE>> 2) - 11))
				Rates [ Channel ] = ( ( -8 + ( Rate & 3 ) ) << 16 ) >> ( ( Rate >> 2 ) - 11 );
				Rates75 [ Channel ] = ( ( -8 + ( Rate & 3 ) ) << 16 ) >> ( ( Rate >> 2 ) - 11 );
			}
			
			if ( Rate == 127 )
			{
				// rate is max, so set Constant to zero
				Rates [ Channel ] = 0;
				Rates75 [ Channel ] = 0;
			}
		}
		
		inline static void Set_ExponentialIncRates ( u32 Channel, u32 Rate, s32* Rates, s32* Rates75 )
		{
			// check if mode is less than 48
			// timing is in first 7 bits
			if ( Rate < 48 )
			{
				// use (7 - (RATE & 3)) <<(11 - (RATE>> 2))
				// shifting by 16 because this is in 16.16 fixed point
				Rates [ Channel ] = ( ( 7 - ( Rate & 3 ) ) << ( 11 - ( Rate >> 2 ) ) ) << 16;
			}
			else
			{
				// use 7 - (RATE & 3) / (1 <<((RATE>> 2) - 11))
				Rates [ Channel ] = ( ( 7 - ( Rate & 3 ) ) << 16 ) >> ( ( Rate >> 2 ) - 11 );
			}
			
			Rate += 8;
			
			if ( Rate < 48 )
			{
				// use (7 - (RATE & 3)) <<(11 - (RATE>> 2))
				// shifting by 16 because this is in 16.16 fixed point
				Rates75 [ Channel ] = ( ( 7 - ( Rate & 3 ) ) << ( 11 - ( Rate >> 2 ) ) ) << 16;
			}
			else
			{
				// use 7 - (RATE & 3) / (1 <<((RATE>> 2) - 11))
				Rates75 [ Channel ] = ( ( 7 - ( Rate & 3 ) ) << 16 ) >> ( ( Rate >> 2 ) - 11 );
			}


			if ( Rate >= 127 )
			{
				// rate is max, so set Constant to zero
				Rates [ Channel ] = 0;
				Rates75 [ Channel ] = 0;
			}
		}
		
		inline static void Set_ExponentialDecRates ( u32 Channel, u64 Rate, s32* Rates, s32* Rates75 )
		{
			// check if mode is less than 48
			// timing is in first 7 bits
			if ( Rate < 48 )
			{
				// use (-8 + (RATE & 3)) <<(11 - (RATE>> 2))
				// multiplier values will be in 1.31 fixed point
				Rates [ Channel ] = ( ( ( -8 + ( Rate & 3 ) ) << ( 11 - ( Rate >> 2 ) ) ) + 32768 ) << 15;
				Rates75 [ Channel ] = ( ( ( -8 + ( Rate & 3 ) ) << ( 11 - ( Rate >> 2 ) ) ) + 32768 ) << 15;
			}
			else
			{
				// use -8 + (RATE & 3) / (1 <<((RATE>> 2) - 11))
				Rates [ Channel ] = ( ( (( ( ( -8LL + ( Rate & 3LL ) ) << 16LL ) >> ( ( Rate >> 2LL ) - 11LL ) )) + ( 32768LL << 16LL ) ) >> 1LL );
				Rates75 [ Channel ] = ( ( (( ( ( -8LL + ( Rate & 3LL ) ) << 16LL ) >> ( ( Rate >> 2LL ) - 11LL ) )) + ( 32768LL << 16LL ) ) >> 1LL );
			}
			
			if ( Rate == 127 )
			{
				// rate is max, so set Constant to one
				Rates [ Channel ] = (1<<30);
				Rates75 [ Channel ] = (1<<30);
			}
		}
		*/


		
		
	};


#ifdef PS2_COMPILE	

	// looks like the SPU2 has registers in all different locations from PS1
	// this probably needs a separate file
	class SPU2
	{
	public:
	
		static Debug::Log debug;
		static SPU2 *_SPU2;
		
		// there are not that many registers that apply to BOTH cores
		//static const int c_iNumberOfRegisters = 16;
		// should have all the registers in one place, since some registers that are needed would be in the other core
		static const int c_iNumberOfRegisters = 0x800 >> 1;
		static const int c_iNumberOfRegisters_Mask = c_iNumberOfRegisters - 1;
		
		// spu frequency
		// this should change for PS2
		static const u32 SPU_Frequency = 44100;
		static const u32 SPU2_Frequency = 48000;
		static const u32 CPUCycles_PerSPUCycle = 768;
		
		u64 CycleCount;
		
		
		
		u16 Regs16 [ c_iNumberOfRegisters ];
		
		
		
		SPUCore SPU0;
		SPUCore SPU1;

#ifdef PS2_COMPILE
		////////////////////////////////////////////
		// 2MB of Sound RAM for SPU2 ??
		static const int c_iRam_Size = 2097152;
#else
		static const int c_iRam_Size = 524288;
#endif

		static const int c_iRam_Mask = c_iRam_Size - 1;
		
		u16 RAM [ c_iRam_Size / 2 ];
		
		static u32 Read ( u32 Address );
		static void Write ( u32 Address, u32 Data, u32 Mask );
		
		// the dma is per core
		//void DMA_Read ( u32* Data, int ByteReadCount );
		//void DMA_ReadBlock ( u32* Data, u32 BS );
		//void DMA_Write ( u32* Data, int ByteWriteCount );
		//void DMA_Write_Block ( u32* Data, u32 BS );

		static u64 DMA_ReadyForRead_Core0 ();
		static u64 DMA_ReadyForRead_Core1 ();
		static u64 DMA_ReadyForWrite_Core0 ();
		static u64 DMA_ReadyForWrite_Core1 ();
		
		static u32 DMA_ReadBlock_Core0 ( u32* pMemoryPtr, u32 Address, u32 WordCount );
		static u32 DMA_ReadBlock_Core1 ( u32* pMemoryPtr, u32 Address, u32 WordCount );
		static u32 DMA_WriteBlock_Core0 ( u32* pMemoryPtr, u32 Address, u32 WordCount );
		static u32 DMA_WriteBlock_Core1 ( u32* pMemoryPtr, u32 Address, u32 WordCount );
		
		
		SPU2 ();
		~SPU2 ();
		
		// reset object/device
		void Reset ();
		
		// start the object/device
		void Start ();


		void Refresh ();
		
		// will run SPU core0 then run SPU core1
		void Run ();
		
		
		// sound output *** testing ***
		// these should be static, but need to be left like this to fix a program crash/mute bug first
		static HWAVEOUT hWaveOut;
		static WAVEFORMATEX wfx;
		static WAVEHDR header;
		static WAVEHDR header0;
		static WAVEHDR header1;
		//static u64 hWaveOut_Save;
		
		s32 AudioOutput_Enabled;
		s32 AudioFilter_Enabled;
		
		
		
		// play size is in samples
		// 1024 does not work right, but 2048 is ok, and 4096 is bit of a delay.. need to fix the buffer size multiply
		// using 262144 for testing
		//static const int c_iPlaySize = 262144;
		static const int c_iPlayBuffer_MaxSize = 131072;	//c_iPlaySize * 4;
		//u16 PlayBuffer [ c_iPlayBuffer_Size ] __attribute__ ((aligned (32)));
		alignas(32) s16 PlayBuffer0 [ c_iPlayBuffer_MaxSize ];
		alignas(32) s16 PlayBuffer1 [ c_iPlayBuffer_MaxSize ];
		
		// dynamic size of sound buffer
		s32 NextPlayBuffer_Size;
		u32 PlayBuffer_Size;
		
		static const int c_iMixerSize = c_iPlayBuffer_MaxSize * 16;
		static const int c_iMixerMask = c_iMixerSize - 1;
		u64 Mixer_ReadIdx, Mixer_WriteIdx;
		s16 Mixer [ c_iMixerSize ];

		
		void Backend_MixSamples ( u32 WriteIdx, u32 Size );
		

		// multi-threading //
		static const u32 c_iMaxThreads = 2;
		static Api::Thread* Threads [ c_iMaxThreads ];

		u32 ulNumThreads;
		static u32 ulNumberOfThreads_Created;

		static const u64 c_ullThBufferSize = 1 << 2;
		static const u64 c_ullThBuffer_Mask = c_ullThBufferSize - 1;
		static volatile u64 ullThReadIdx;
		static volatile u64 ullThWriteIdx;
		static volatile u64 ullThTargetIdx;
		static volatile u64 ullLastWriteIdx [ c_ullThBufferSize ];
		static volatile u64 ullLastSize [ c_ullThBufferSize ];
		//static volatile u64 ullWorkCount;

		static volatile u64 ullThreadRunning;
		//static volatile u64 ullKillThread;

		static HANDLE ghEvent_Update;
		//HANDLE ghEvent_PS2SPU_Frame;
		//HANDLE ghEvent_PS2SPU_Finish;

		// this runs parts that are multi-threaded
		static int Run_Thread( void* Param );
		static void Start_Thread ();
		static void End_Thread ();

		
		// simulator needs to have its own internal volume level
		static const s32 c_iGlobalVolume_Default = 0x1000;
		//s64 GlobalVolume;
		s32 GlobalVolume;

		// index for the next event
		u32 NextEvent_Idx;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;
		

		static void sRun () { _SPU2->Run (); }
		static void Set_EventCallback ( funcVoid2 UpdateEvent_CB ) { _SPU2->NextEvent_Idx = UpdateEvent_CB ( sRun ); };
		
		
		// for interrupt call back
		static funcVoid UpdateInterrupts;
		static void Set_IntCallback ( funcVoid UpdateInt_CB ) { UpdateInterrupts = UpdateInt_CB; SPUCore::Set_IntCallback ( UpdateInt_CB ); };
		
		
		static const u32 c_InterruptBit = 9;
		//static const u32 c_InterruptBit_AutoDMA = 11;
		
		static u32* _Intc_Stat;
		static u32* _Intc_Mask;
		static u32* _R3000A_Status_12;
		static u32* _R3000A_Cause_13;
		static u64* _ProcStatus;
		
		inline void ConnectInterrupt ( u32* _IStat, u32* _IMask, u32* _R3000A_Status, u32* _R3000A_Cause, u64* _ProcStat )
		{
			_Intc_Stat = _IStat;
			_Intc_Mask = _IMask;
			_R3000A_Cause_13 = _R3000A_Cause;
			_R3000A_Status_12 = _R3000A_Status;
			_ProcStatus = _ProcStat;
			
			SPUCore::ConnectInterrupt ( _IStat, _IMask, _R3000A_Status, _R3000A_Cause, _ProcStat );
		}
		
		
		static inline void SetInterrupt ()
		{
			*_Intc_Stat |= ( 1 << c_InterruptBit );
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}

		/*
		// call this after "SetInterrupt"
		// sets the extra interrupt bits on PS2 in SPDIF registers ??
		static inline void SetInterrupt_Core ( int Core )
		{
			// here too ??
			//lSPDIF_OUT |= ( 0x4 << Core );
			GET_REG16 ( SPDIF_OUT ) |= ( 0x4 << Core );
			
			// need to check if this gets cleared after read ??
			//lSPDIF_IRQINFO |= ( 0x4 << Core );
			GET_REG16 ( SPDIF_IRQINFO ) |= ( 0x4 << Core );
		}
		*/

		//static inline void SetInterrupt_AutoDMA ()
		//{
		//	*_Intc_Stat |= ( 1 << c_InterruptBit_AutoDMA );
		//	if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
		//	
		//	if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
		//}


		
		static inline void ClearInterrupt ()
		{
			*_Intc_Stat &= ~( 1 << c_InterruptBit );
			
			UpdateInterrupts ();
			
			/*
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}
		
		static u64* _NextSystemEvent;
		
		inline void Set_NextEvent ( u64 Cycle )
		{
			NextEvent_Cycle = Cycle + *_DebugCycleCount;
			//if ( NextEvent_Cycle > *_SystemCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_SystemCycleCount ) ) *_NextSystemEvent = NextEvent_Cycle;
			if ( NextEvent_Cycle < *_NextSystemEvent )
			{
				*_NextSystemEvent = NextEvent_Cycle;
				*_NextEventIdx = NextEvent_Idx;
			}
		}
		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		static u64* _SystemCycleCount;
		static u32 *_NextEventIdx;
		
		// object debug stuff
		// Enable/Disable debug window for object
		// Windows API specific code is in here
		static bool DebugWindow_Enabled;
		static WindowClass::Window *DebugWindow;
		static DebugValueList<u16> *SPUMaster_ValueList;
		static DebugValueList<u16> *SPU_ValueList [ 24 ];
		static Debug_MemoryViewer *SoundRAM_Viewer;
		static void DebugWindow_Enable ();
		static void DebugWindow_Disable ();
		static void DebugWindow_Update ();
	};
#endif


};

#endif


