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


#ifndef _SPU_H_
#define _SPU_H_

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

	class SPU
	{
		static Debug::Log debug;
		static SPU *_SPU;
		
	public:
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the dma registers start at
		static const long Regs_Start = 0x1f801c00;
		
		// where the dma registers end at
		static const long Regs_End = 0x1f801dfe;
	
		// distance between numbered groups of registers for dma
		static const long Reg_Size = 0x2;
		
		// used for debugging sound problems
		static u32 Debug_ChannelEnable;
		
		// index of next event
		u32 NextEvent_Idx;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;
		
		static const int c_iNumberOfRegisters = 256 + 48;

		// added 48 for the internal channel volume registers
		static const char* RegisterNames [ 256 + 48 ];
		

		static const int NumberOfChannels = 24;
		static const int c_iNumberOfChannels = 24;
		
		// the number of pcm samples per adpcm block
		static const int c_iSamplesPerBlock = 28;
		
		
		// minimum number of Ts before key-off can be specified
		static const int c_iKeyOffT = 2;
		
		// gain value for mixing samples??
		// a 1 would mean a shift right of 1 each time you mix
		static constexpr float c_iMixer_Gain = 1.0f;
		
		// w0 = 2*pi*f0/Fs
		// b0 = ( 1 - cos(w0) ) / 2
		// b1 = 1 - cos(w0)
		// b2 = ( 1 - cos(w0) ) / 2
		// y[n] = (b0_ * x[n] + b1_ * x[n - 1] + b2_ * x[n - 2] - a1_ * y[n - 1] - a2_ * y[n - 2]) >> N;
		
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
		inline u16 REG ( u32 Address ) { return Regs [ ( Address - Regs_Start ) >> 1 ]; }
		inline s32 REG32 ( u32 Address ) { return ( (s32) ( (s16) Regs [ ( Address - Regs_Start ) >> 1 ] ) ); }
		inline s64 REG64 ( u32 Address ) { return ( (s64) ( (s16) Regs [ ( Address - Regs_Start ) >> 1 ] ) ); }


		
		struct ChRegs0_Layout
		{
			// channel volume setting left/right
			s16 VOL_L, VOL_R;
			
			// pitch
			u16 PITCH,
			
			// sound start address
			SSA,
			
			// ADSR setting
			ADSR_0, ADSR_1,
			
			// current envelope level
			ENV_X,
			
			// loop start address
			LSA;
		};

		struct ChRegs1_Layout
		{
			// current channel volume left/right
			s16 CVOL_L, CVOL_R;
		};

		struct CoreRegs_Layout
		{
			// VOL_L, VOL_R, PITCH, SSA, ADSR_0, ADSR_1, ENV_X, LSA
			ChRegs0_Layout ChRegs0 [ 24 ];
			
			// master volume setting
			u16 MVOL_L, MVOL_R;
			
			// effect volume
			u16 EVOL_L, EVOL_R;
			
			union 
			{
				struct
				{
					// key on
					u16 KON_0, KON_1,
					
					// key off
					KOFF_0, KOFF_1,
					
					// pitch modulation enable
					PMON_0, PMON_1,
					
					// noise enable
					NON_0, NON_1,
					
					// reverb enable
					RON_0, RON_1,
					
					// sample end-point flag
					CON_0, CON_1;
				};
				
				struct
				{
					u32 KON, KOFF, PMON, NON, RON, CON;
				};
			};
			
			// unknown
			u16 UNK0;
			
			// reverb work area start
			u16 RVWA;
			
			// address that triggers IRQ
			u16 IRQA;
			
			// sound buffer address (used for writting data)
			u16 SBA;
			
			// data register (write sound data here when writing manually)
			u16 DATA;
			
			// control and status registers
			u16 CTRL, INIT, STAT;
			
			// cd volume
			u16 CDVOL_L, CDVOL_R;
			
			// EXT volume
			u16 EXTVOL_L, EXTVOL_R;
			
			// current master volume
			u16 CMVOL_L, CMVOL_R;
			
			// unknown
			u16 UNK1, UNK2;
			
			// reverb registers
			u16 dAPF1, dAPF2, vIIR, vCOMB1, vCOMB2, vCOMB3, vCOMB4, vWALL, vAPF1, vAPF2, mLSAME, mRSAME, mLCOMB1, mRCOMB1, mLCOMB2, mRCOMB2;
			u16 dLSAME, dRSAME, mLDIFF, mRDIFF, mLCOMB3, mRCOMB3, mLCOMB4, mRCOMB4, dLDIFF, dRDIFF, mLAPF1, mRAPF1, mLAPF2, mRAPF2, vLIN, vRIN;
			
			// current channel volume registers
			ChRegs1_Layout ChRegs1 [ 24 ];
			//u16 PMON_0, PMON_1, NON_0, NON_1, VMIXL_0, VMIXL_1, VMIXEL_0, VMIXEL_1, VMIXR_0, VMIXR_1, VMIXER_0, VMIXER_1;
		};
		
		static CoreRegs_Layout *pCoreRegs;
		
		
		
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
		
		// SPU cycle at which channel was keyed on
		u64 StartCycle_Channel [ c_iNumberOfChannels ];
		u16 ActiveLSA_Channel [ c_iNumberOfChannels ];
		
		void Start_SampleDecoding ( u32 Channel );
		
		u32 ReverbWork_Size, ReverbWork_Start;
		
		// gets address in reverb work area at offset address
		s32 ReadReverbBuffer ( u32 Address );
		void WriteReverbBuffer ( u32 Address, s32 Value );
		void UpdateReverbBuffer ();
		
		
		// get value in SPU register specified by its channel number and offset
		inline u16 REG ( u32 Channel, u32 Offset ) { return Regs [ ( ( Channel << 4 ) + Offset ) >> 1 ]; }
		
		void SpuTransfer_Complete ();
		
		static u32 Read ( u32 Address );
		static void Write ( u32 Address, u32 Data, u32 Mask );
		void DMA_Read ( u32* Data, int ByteReadCount );
		static u32 DMA_ReadBlock ( u32* pMemory, u32 Address, u32 BS );	//( u32* Data, u32 BS );
		void DMA_Write ( u32* Data, int ByteWriteCount );
		static u32 DMA_WriteBlock ( u32* pMemory, u32 Address, u32 BS );	//( u32* Data, u32 BS );
		//void DMA_Write_All ( u32* Data, int TransferAmountInWords32, int BlockSizeInWords32 );
		
		// returns true if device is ready for a read from DMA
		static u64 DMA_ReadyForRead ( void );
		static u64 DMA_ReadyForWrite ( void );
		
		void Reset ();
		
		void Start ();
		
		void Run ();
		
		s32 Timer;
		s16 NoiseLevel;
		void RunNoiseGenerator ();
		
		
		// processes reverb
		s32 ReverbL_Output, ReverbR_Output;
		u32 Reverb_BufferAddress;
		void ProcessReverbL ( s32 LeftInput );
		void ProcessReverbR ( s32 RightInput );
		
		// simulator needs to have its own internal volume level
		static const s32 c_iGlobalVolume_Default = 0x1000;
		//s64 GlobalVolume;
		s32 GlobalVolume;
		
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
		u16 Regs [ c_iNumberOfRegisters ];
		
		///////////////////////////////////////////////////
		// 64 byte buffer
		u32 BufferIndex;
		s16 Buffer [ 32 ];
		
		u64 CycleCount;
		
		// unsure what this value should be yet
		static const int c_iVolumeShift = 15;
		
		static const int c_iRam_Size = 524288;
		static const int c_iRam_Mask = c_iRam_Size - 1;
		
		////////////////////////////////////////////
		// 512 KB of Sound RAM for SPU
		u16 RAM [ c_iRam_Size / 2 ];
		
		// sound output *** testing ***
		// these should be static, but need to be left like this to fix a program crash/mute bug first
		static HWAVEOUT hWaveOut; /* device handle */
		static WAVEFORMATEX wfx;
		static WAVEHDR header;
		static WAVEHDR header0;
		static WAVEHDR header1;
		//static u64 hWaveOut_Save;
		
		s32 AudioOutput_Enabled;
		s32 AudioFilter_Enabled;
		
		
		
		// need to know when a channel is playing or not
		u32 ChannelPlaying_Bitmap;
		
		// need to know when to kill the envelope for a channel
		u32 KillEnvelope_Bitmap;
		
		// loop specified bitmap
		u32 LSA_Manual_Bitmap;
		
		
		// play size is in samples
		// 1024 does not work right, but 2048 is ok, and 4096 is bit of a delay.. need to fix the buffer size multiply
		// using 262144 for testing
		static const int c_iPlaySize = 262144;
		static const int c_iPlayBuffer_MaxSize = 131072;	//c_iPlaySize * 4;
		//u16 alignas(32) PlayBuffer [ c_iPlayBuffer_Size ];
		alignas(32) s16 PlayBuffer0 [ c_iPlayBuffer_MaxSize ];
		alignas(32) s16 PlayBuffer1 [ c_iPlayBuffer_MaxSize ];
		
		// dynamic size of sound buffer
		s32 NextPlayBuffer_Size;
		u32 PlayBuffer_Size;
		
		static const int c_iMixerSize = c_iPlayBuffer_MaxSize;	//1048576;
		static const int c_iMixerMask = c_iMixerSize - 1;
		u64 Mixer_ReadIdx, Mixer_WriteIdx;
		s16 Mixer [ c_iMixerSize ];
		
		
		//SPU ();
		~SPU ();
		
		void UpdatePitch ( int Channel, u32 Pitch, u32 Reg_PMON, s32 PreviousSample );
		
		u32 Cycles [ c_iNumberOfChannels ];
		
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
		
		// buffer for filter
		s32 FilterBuf [ c_iNumberOfChannels * 4 ];
		s32 OutputFirBufL [ c_iNumberOfChannels * 2 ];
		s32 InputFirBufL [ c_iNumberOfChannels * 4 ];
		s32 OutputFirBufR [ c_iNumberOfChannels * 2 ];
		s32 InputFirBufR [ c_iNumberOfChannels * 4 ];
		s32 Core_OutputFirBuf [ 2 * 2 ];
		s32 Core_InputFirBuf [ 2 * 4 ];
		
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
		static const u32 DecodeBufferSize = 0x400;
		u32 DecodeBufferOffset;	// this is where the current sample is being decoded and mixed to
		
		//////////////////////////////////////////////
		// start of SPU registers
		static const u32 SPU_X = 0x1f801c00;
		
		///////////////////////////////////////////////////////////////////
		// register address voice data area offsets
		
		// global volume left
		static const u32 VOL_L = 0;
		
		// global volume right
		static const u32 VOL_R = 2;
		
		// pitch value
		static const u32 PITCH = 4;
		
		// start of sound address
		static const u32 SSA_X = 6;
		
		// adsr register 0
		static const u32 ADSR_0 = 8;
		
		// adsr registr 1
		static const u32 ADSR_1 = 0xa;
		
		// current envelope volume value
		static const u32 ENV_X = 0xc;
		
		// sound loop start address
		static const u32 LSA_X = 0xe;
		
		//////////////////////////////////////////////////////////////////////
		// global sound register addresses
		
		// master volume left
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
		
		// current main volume
		static const u32 CMVOL_L = 0x1f801db8;
		static const u32 CMVOL_R = 0x1f801dba;
		
		// current channel volume
		static const u32 CVOL_SIZE = 0x4;
		static const u32 CVOL_L_START = 0x1f801e00;
		static const u32 CVOL_R_START = 0x1f801e02;
		
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
		
		static void SetSweepVars ( u16 flags, u32 Channel, s32* Rates, s32* Rates75 );
		static void SweepVolume ( u16 flags, s64& CurrentVolume, u32 VolConstant, u32 VolConstant75 );
		
		static s64 Get_VolumeStep ( s64& Level, u32& Cycles, u32 Value, u32 flags );
		static void Start_VolumeEnvelope ( s64& Level, u32& Cycles, u32 Value, u32 flags, bool InitLevel = false );
		static void VolumeEnvelope ( s64& Level, u32& Cycles, u32 Value, u32 flags, bool clamp = true );
		
		
		
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
		static const u32 REG_SoundBufferAddress = 0x1f801da6;
		u32 NextSoundBufferAddress;
		
		// need to know when manual write buffer is full
		u32 ManualWriteBuffer_isFull;
		
		///////////////////////////////////////////////////////////////////////
		// Data forwarding reg
		// - data gets sent to sound buffer via non-dma transfers
		static const u32 REG_DataForwarding = 0x1f801da8;
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		// *** ALL CONSTANTS IN ANY OF MY CLASSES CAN BE MODIFIED WHEN MY VALUES ARE NOT CORRECT ***
		// *** THESE CONSTANTS SHOULD BE FOUND AT THE TOP OF EACH OBJECT IN HEADER FILE ***
		static const u32 SPU_Frequency = 44100;
		static const u32 CPUCycles_PerSPUCycle = 768;
		
		// number of cycles to wait before running device
		u32 WaitCycles;

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
						u8 NotUsed0 : 1;
						
						// bit 1 - LOOP_END - when set indicates that this is the last block of waveform data
						u8 LOOP_END : 1;
						
						// bit 2 - LOOP - must be set in every block if there is a loop point
						u8 LOOP : 1;
						
						// bit 3 - LOOP_START - when set sets loop address to the beginning of the current block
						u8 LOOP_START : 1;
						
						u8 NotUsed1 : 4;
					};
					
					u8 flags;
				};
				
				u8 EncodedSoundData [ 14 ];
			};
			
			u8 Value [ 16 ];
		};


		static void sRun () { _SPU->Run (); }
		static void Set_EventCallback ( funcVoid2 UpdateEvent_CB ) { _SPU->NextEvent_Idx = UpdateEvent_CB ( sRun ); };
		
		
		// for interrupt call back
		static funcVoid UpdateInterrupts;
		static void Set_IntCallback ( funcVoid UpdateInt_CB ) { UpdateInterrupts = UpdateInt_CB; };
		
		
		static const u32 c_InterruptBit = 9;
		
		//static u32* _Intc_Master;
		static u32* _Intc_Stat;
		static u32* _Intc_Mask;
		static u32* _R3000A_Status_12;
		static u32* _R3000A_Cause_13;
		static u64* _ProcStatus;
		
		inline void ConnectInterrupt ( u32* _IStat, u32* _IMask, u32* _R3000A_Status, u32* _R3000A_Cause, u64* _ProcStat )
		{
			_Intc_Stat = _IStat;
			_Intc_Mask = _IMask;
			//_Intc_Master = _IMaster;
			_R3000A_Cause_13 = _R3000A_Cause;
			_R3000A_Status_12 = _R3000A_Status;
			_ProcStatus = _ProcStat;
		}
		
		
		static inline void SetInterrupt ()
		{
			//*_Intc_Master |= ( 1 << c_InterruptBit );
			*_Intc_Stat |= ( 1 << c_InterruptBit );
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}
		
		static inline void ClearInterrupt ()
		{
			//*_Intc_Master &= ~( 1 << c_InterruptBit );
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
		
		inline void Set_NextEventCycle ( u64 Cycle )
		{
			NextEvent_Cycle = Cycle;
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
		
	private:
	
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


		
		
	};




};

#endif


