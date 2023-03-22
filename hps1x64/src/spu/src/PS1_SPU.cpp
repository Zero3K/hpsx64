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



#include "PS1_SPU.h"
#include "PS1_CD.h"

#include "GeneralUtilities.h"




using namespace Playstation1;
using namespace GeneralUtilities;


#define ENABLE_FILTER_PER_CHANNEL
//#define ENABLE_FILTER_PER_CORE

//#define ENABLE_FILTER_PER_CHANNEL_FIR



// require each block to have loop flag set in waveform data or else sound will mute
//#define REQUIRE_ALL_LOOP_FLAGS


#define KEY_ON_OFF_LOOP2


#define VERBOSE_KEYONOFF_TEST


//#define VERBOSE_REVERSE_PHASE



#ifdef _DEBUG_VERSION_

// enable debugging
#define INLINE_DEBUG_ENABLE

//#define INLINE_DEBUG_RUN_NOTIFY

/*
//#define INLINE_DEBUG_REVERB
#define INLINE_DEBUG_DMA_WRITE
//#define INLINE_DEBUG_DMA_WRITE_RECORD
//#define INLINE_DEBUG_SPU_ERROR_RECORD
//#define INLINE_DEBUG
//#define INLINE_DEBUG_CDSOUND
#define INLINE_DEBUG_RUN
//#define INLINE_DEBUG_ENVELOPE
#define INLINE_DEBUG_WRITE
#define INLINE_DEBUG_READ
//#define INLINE_DEBUG_WRITE_DEFAULT
//#define INLINE_DEBUG_RUN_CHANNELONOFF
//#define INLINE_DEBUG_READ_CHANNELONOFF
//#define INLINE_DEBUG_WRITE_CHANNELONOFF
//#define INLINE_DEBUG_WRITE_IRQA
//#define INLINE_DEBUG_WRITE_CTRL
//#define INLINE_DEBUG_WRITE_STAT
//#define INLINE_DEBUG_WRITE_DATA
//#define INLINE_DEBUG_WRITE_SBA
//#define INLINE_DEBUG_WRITE_LSA_X
*/

#endif


funcVoid SPU::UpdateInterrupts;


u32* SPU::_DebugPC;
u64* SPU::_DebugCycleCount;
u64* SPU::_SystemCycleCount;
u32* SPU::_NextEventIdx;


u64* SPU::_NextSystemEvent;


u32* SPU::_Intc_Stat;
u32* SPU::_Intc_Mask;
//u32* SPU::_Intc_Master;
u32* SPU::_R3000A_Status_12;
u32* SPU::_R3000A_Cause_13;
u64* SPU::_ProcStatus;


Debug::Log SPU::debug;

SPU *SPU::_SPU;

s16 *SPU::_vLOUT;
s16 *SPU::_vROUT;
u16 *SPU::_mBASE;

u16 *SPU::_dAPF1;
u16 *SPU::_dAPF2;
s16 *SPU::_vIIR;
s16 *SPU::_vCOMB1;
s16 *SPU::_vCOMB2;
s16 *SPU::_vCOMB3;
s16 *SPU::_vCOMB4;
s16 *SPU::_vWALL;
s16 *SPU::_vAPF1;
s16 *SPU::_vAPF2;
u16 *SPU::_mLSAME;
u16 *SPU::_mRSAME;
u16 *SPU::_mLCOMB1;
u16 *SPU::_mRCOMB1;
u16 *SPU::_mLCOMB2;
u16 *SPU::_mRCOMB2;
u16 *SPU::_dLSAME;
u16 *SPU::_dRSAME;
u16 *SPU::_mLDIFF;
u16 *SPU::_mRDIFF;
u16 *SPU::_mLCOMB3;
u16 *SPU::_mRCOMB3;
u16 *SPU::_mLCOMB4;
u16 *SPU::_mRCOMB4;
u16 *SPU::_dLDIFF;
u16 *SPU::_dRDIFF;
u16 *SPU::_mLAPF1;
u16 *SPU::_mRAPF1;
u16 *SPU::_mLAPF2;
u16 *SPU::_mRAPF2;
s16 *SPU::_vLIN;
s16 *SPU::_vRIN;



bool SPU::DebugWindow_Enabled;
WindowClass::Window *SPU::DebugWindow;
DebugValueList<u16> *SPU::SPUMaster_ValueList;
DebugValueList<u16> *SPU::SPU_ValueList [ 24 ];
Debug_MemoryViewer *SPU::SoundRAM_Viewer;

u32 SPU::Debug_ChannelEnable = 0xffffff;

HWAVEOUT SPU::hWaveOut; /* device handle */
WAVEFORMATEX SPU::wfx;
WAVEHDR SPU::header;
WAVEHDR SPU::header0;
WAVEHDR SPU::header1;




SPU::CoreRegs_Layout *SPU::pCoreRegs;


const char* SPU::RegisterNames [ 256 + 48 ] = { 
	"Ch0_VOL_L", "Ch0_VOL_R", "Ch0_PITCH", "Ch0_SSA_X", "Ch0_ADSR_0", "Ch0_ADSR_0", "Ch0_ENV_X", "Ch0_LSA_X",
	"Ch1_VOL_L", "Ch1_VOL_R", "Ch1_PITCH", "Ch1_SSA_X", "Ch1_ADSR_0", "Ch1_ADSR_1", "Ch1_ENV_X", "Ch1_LSA_X",
	"Ch2_VOL_L", "Ch2_VOL_R", "Ch2_PITCH", "Ch2_SSA_X", "Ch2_ADSR_0", "Ch2_ADSR_2", "Ch2_ENV_X", "Ch2_LSA_X",
	"Ch3_VOL_L", "Ch3_VOL_R", "Ch3_PITCH", "Ch3_SSA_X", "Ch3_ADSR_0", "Ch3_ADSR_3", "Ch3_ENV_X", "Ch3_LSA_X",
	"Ch4_VOL_L", "Ch4_VOL_R", "Ch4_PITCH", "Ch4_SSA_X", "Ch4_ADSR_0", "Ch4_ADSR_4", "Ch4_ENV_X", "Ch4_LSA_X",
	"Ch5_VOL_L", "Ch5_VOL_R", "Ch5_PITCH", "Ch5_SSA_X", "Ch5_ADSR_0", "Ch5_ADSR_5", "Ch5_ENV_X", "Ch5_LSA_X",
	"Ch6_VOL_L", "Ch6_VOL_R", "Ch6_PITCH", "Ch6_SSA_X", "Ch6_ADSR_0", "Ch6_ADSR_6", "Ch6_ENV_X", "Ch6_LSA_X",
	"Ch7_VOL_L", "Ch7_VOL_R", "Ch7_PITCH", "Ch7_SSA_X", "Ch7_ADSR_0", "Ch7_ADSR_7", "Ch7_ENV_X", "Ch7_LSA_X",
	"Ch8_VOL_L", "Ch8_VOL_R", "Ch8_PITCH", "Ch8_SSA_X", "Ch8_ADSR_0", "Ch8_ADSR_8", "Ch8_ENV_X", "Ch8_LSA_X",
	"Ch9_VOL_L", "Ch9_VOL_R", "Ch9_PITCH", "Ch9_SSA_X", "Ch9_ADSR_0", "Ch9_ADSR_9", "Ch9_ENV_X", "Ch9_LSA_X",
	"Ch10_VOL_L", "Ch10_VOL_R", "Ch10_PITCH", "Ch10_SSA_X", "Ch10_ADSR_0", "Ch10_ADSR_10", "Ch10_ENV_X", "Ch10_LSA_X",
	"Ch11_VOL_L", "Ch11_VOL_R", "Ch11_PITCH", "Ch11_SSA_X", "Ch11_ADSR_0", "Ch11_ADSR_11", "Ch11_ENV_X", "Ch11_LSA_X",
	"Ch12_VOL_L", "Ch12_VOL_R", "Ch12_PITCH", "Ch12_SSA_X", "Ch12_ADSR_0", "Ch12_ADSR_12", "Ch12_ENV_X", "Ch12_LSA_X",
	"Ch13_VOL_L", "Ch13_VOL_R", "Ch13_PITCH", "Ch13_SSA_X", "Ch13_ADSR_0", "Ch13_ADSR_13", "Ch13_ENV_X", "Ch13_LSA_X",
	"Ch14_VOL_L", "Ch14_VOL_R", "Ch14_PITCH", "Ch14_SSA_X", "Ch14_ADSR_0", "Ch14_ADSR_14", "Ch14_ENV_X", "Ch14_LSA_X",
	"Ch15_VOL_L", "Ch15_VOL_R", "Ch15_PITCH", "Ch15_SSA_X", "Ch15_ADSR_0", "Ch15_ADSR_15", "Ch15_ENV_X", "Ch15_LSA_X",
	"Ch16_VOL_L", "Ch16_VOL_R", "Ch16_PITCH", "Ch16_SSA_X", "Ch16_ADSR_0", "Ch16_ADSR_16", "Ch16_ENV_X", "Ch16_LSA_X",
	"Ch17_VOL_L", "Ch17_VOL_R", "Ch17_PITCH", "Ch17_SSA_X", "Ch17_ADSR_0", "Ch17_ADSR_17", "Ch17_ENV_X", "Ch17_LSA_X",
	"Ch18_VOL_L", "Ch18_VOL_R", "Ch18_PITCH", "Ch18_SSA_X", "Ch18_ADSR_0", "Ch18_ADSR_18", "Ch18_ENV_X", "Ch18_LSA_X",
	"Ch19_VOL_L", "Ch19_VOL_R", "Ch19_PITCH", "Ch19_SSA_X", "Ch19_ADSR_0", "Ch19_ADSR_19", "Ch19_ENV_X", "Ch19_LSA_X",
	"Ch20_VOL_L", "Ch20_VOL_R", "Ch20_PITCH", "Ch20_SSA_X", "Ch20_ADSR_0", "Ch20_ADSR_20", "Ch20_ENV_X", "Ch20_LSA_X",
	"Ch21_VOL_L", "Ch21_VOL_R", "Ch21_PITCH", "Ch21_SSA_X", "Ch21_ADSR_0", "Ch21_ADSR_21", "Ch21_ENV_X", "Ch21_LSA_X",
	"Ch22_VOL_L", "Ch22_VOL_R", "Ch22_PITCH", "Ch22_SSA_X", "Ch22_ADSR_0", "Ch22_ADSR_22", "Ch22_ENV_X", "Ch22_LSA_X",
	"Ch23_VOL_L", "Ch23_VOL_R", "Ch23_PITCH", "Ch23_SSA_X", "Ch23_ADSR_0", "Ch23_ADSR_23", "Ch23_ENV_X", "Ch23_LSA_X",
	"MVOL_L", "MVOL_R", "EVOL_L", "EVOL_R", "KON_0", "KON_1", "KOFF_0", "KOFF_1", "PMON_0", "PMON_1", "NON_0", "NON_1",
	"RON_0", "RON_1", "CON_0", "CON_1"
	"Unknown", "RVWA", "IRQA", "SBA", "DATA", "CTRL", "INIT", "STAT", "CDVOL_L", "CDVOL_R", "EXTVOL_L", "EXTVOL_R",
	"CMVOL_L", "CMVOL_R", "Unknown", "Unknown",
	"dAPF1", "dAPF2", "vIIR", "vCOMB1", "vCOMB2", "vCOMB3", "vCOMB4", "vWALL", "vAPF1", "vAPF2", "mLSAME", "mRSAME", "mLCOMB1", "mRCOMB1", "mLCOMB2", "mRCOMB2",
	"dLSAME", "dRSAME", "mLDIFF", "mRDIFF", "mLCOMB3", "mRCOMB3", "mLCOMB4", "mRCOMB4", "dLDIFF", "dRDIFF", "mLAPF1", "mRAPF1", "mLAPF2", "mRAPF2", "vLIN", "vRIN",
	"Ch0_CVOL_L", "Ch0_CVOL_R", "Ch1_CVOL_L", "Ch1_CVOL_R", "Ch2_CVOL_L", "Ch2_CVOL_R", "Ch3_CVOL_L", "Ch3_CVOL_R", "Ch4_CVOL_L", "Ch4_CVOL_R",
	"Ch5_CVOL_L", "Ch5_CVOL_R", "Ch6_CVOL_L", "Ch6_CVOL_R", "Ch7_CVOL_L", "Ch7_CVOL_R", "Ch8_CVOL_L", "Ch8_CVOL_R", "Ch9_CVOL_L", "Ch9_CVOL_R",
	"Ch10_CVOL_L", "Ch10_CVOL_R", "Ch11_CVOL_L", "Ch11_CVOL_R", "Ch12_CVOL_L", "Ch12_CVOL_R", "Ch13_CVOL_L", "Ch13_CVOL_R", "Ch14_CVOL_L", "Ch14_CVOL_R",
	"Ch15_CVOL_L", "Ch15_CVOL_R", "Ch16_CVOL_L", "Ch16_CVOL_R", "Ch17_CVOL_L", "Ch17_CVOL_R", "Ch18_CVOL_L", "Ch18_CVOL_R", "Ch19_CVOL_L", "Ch19_CVOL_R",
	"Ch20_CVOL_L", "Ch20_CVOL_R", "Ch21_CVOL_L", "Ch21_CVOL_R", "Ch22_CVOL_L", "Ch22_CVOL_R", "Ch23_CVOL_L", "Ch23_CVOL_R"
};



/*
SPU::SPU ()
{
	cout << "Running SPU constructor...\n";
}
*/

SPU::~SPU ()
{
	waveOutClose( hWaveOut );
}



void SPU::Start ()
{
	cout << "Running SPU::Start...\n";

#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create ( "SPU_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering SPU::Start";
#endif

	
	_SPU = this;
	Reset ();
	
	pCoreRegs = (CoreRegs_Layout*) Regs;
	
	// set the global volume to default
	GlobalVolume = c_iGlobalVolume_Default;
	
	// start the sound buffer out at 1m for now
	//PlayBuffer_Size = c_iPlayBuffer_MaxSize;
	//NextPlayBuffer_Size = c_iPlayBuffer_MaxSize;
	PlayBuffer_Size = 8192;
	NextPlayBuffer_Size = 8192;

	wfx.nSamplesPerSec = 44100; /* sample rate */
	wfx.wBitsPerSample = 16; /* sample size */
	wfx.nChannels = 2; /* channels*/
	/*
	 * WAVEFORMATEX also has other fields which need filling.
	 * as long as the three fields above are filled this should
	 * work for any PCM (pulse code modulation) format.
	 */
	wfx.cbSize = 0; /* size of _extra_ info */
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	if( waveOutOpen( &hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL ) != MMSYSERR_NOERROR)
	{
		cout << "\nunable to open WAVE_MAPPER device\n";
		//ExitProcess(1);
	}
	else
	{
		cout << "\naudio device was opened successfully\n";
	}
	
	// disable audio output for now
	AudioOutput_Enabled = 1;
	
	// audio device is done with audio buffer
	//header.dwFlags |= WHDR_DONE;
	header0.dwFlags |= WHDR_DONE;
	header1.dwFlags |= WHDR_DONE;
	
	// *** testing ***
	//hWaveOut_Save = (u64)hWaveOut;
	
	// output size of object
	//cout << " Size of SPU class=" << sizeof ( SPU );

#ifdef PS2_COMPILE
	// ps1 spu not needed unless in ps1 mode for ps2
	Set_NextEventCycle ( -1ULL );
#else
	// SPU is not needed to run on EVERY cycle
	//WaitCycles = CPUCycles_PerSPUCycle;
	//NextEvent_Cycle = CPUCycles_PerSPUCycle;
	Set_NextEvent ( CPUCycles_PerSPUCycle );
#endif

	
#ifdef INLINE_DEBUG
	debug << "->Exiting SPU::Start";
#endif
}


void SPU::Reset ()
{
	int i;
	
	// zero object
	memset ( this, 0, sizeof( SPU ) );
	
	// pointers for quick access
	_vLOUT = (s16*) & ( Regs [ ( vLOUT - SPU_X ) >> 1 ] );
	_vROUT = (s16*) & ( Regs [ ( vROUT - SPU_X ) >> 1 ] );
	_mBASE = & ( Regs [ ( mBASE - SPU_X ) >> 1 ] );

	_dAPF1 = & ( Regs [ ( dAPF1 - SPU_X ) >> 1 ] );
	_dAPF2 = & ( Regs [ ( dAPF2 - SPU_X ) >> 1 ] );
	_vIIR = (s16*) & ( Regs [ ( vIIR - SPU_X ) >> 1 ] );
	_vCOMB1 = (s16*) & ( Regs [ ( vCOMB1 - SPU_X ) >> 1 ] );
	_vCOMB2 = (s16*) & ( Regs [ ( vCOMB2 - SPU_X ) >> 1 ] );
	_vCOMB3 = (s16*) & ( Regs [ ( vCOMB3 - SPU_X ) >> 1 ] );
	_vCOMB4 = (s16*) & ( Regs [ ( vCOMB4 - SPU_X ) >> 1 ] );
	_vWALL = (s16*) & ( Regs [ ( vWALL - SPU_X ) >> 1 ] );
	_vAPF1 = (s16*) & ( Regs [ ( vAPF1 - SPU_X ) >> 1 ] );
	_vAPF2 = (s16*) & ( Regs [ ( vAPF2 - SPU_X ) >> 1 ] );
	_mLSAME = & ( Regs [ ( mLSAME - SPU_X ) >> 1 ] );
	_mRSAME = & ( Regs [ ( mRSAME - SPU_X ) >> 1 ] );
	_mLCOMB1 = & ( Regs [ ( mLCOMB1 - SPU_X ) >> 1 ] );
	_mRCOMB1 = & ( Regs [ ( mRCOMB1 - SPU_X ) >> 1 ] );
	_mLCOMB2 = & ( Regs [ ( mLCOMB2 - SPU_X ) >> 1 ] );
	_mRCOMB2 = & ( Regs [ ( mRCOMB2 - SPU_X ) >> 1 ] );
	_dLSAME = & ( Regs [ ( dLSAME - SPU_X ) >> 1 ] );
	_dRSAME = & ( Regs [ ( dRSAME - SPU_X ) >> 1 ] );
	_mLDIFF = & ( Regs [ ( mLDIFF - SPU_X ) >> 1 ] );
	_mRDIFF = & ( Regs [ ( mRDIFF - SPU_X ) >> 1 ] );
	_mLCOMB3 = & ( Regs [ ( mLCOMB3 - SPU_X ) >> 1 ] );
	_mRCOMB3 = & ( Regs [ ( mRCOMB3 - SPU_X ) >> 1 ] );
	_mLCOMB4 = & ( Regs [ ( mLCOMB4 - SPU_X ) >> 1 ] );
	_mRCOMB4 = & ( Regs [ ( mRCOMB4 - SPU_X ) >> 1 ] );
	_dLDIFF = & ( Regs [ ( dLDIFF - SPU_X ) >> 1 ] );
	_dRDIFF = & ( Regs [ ( dRDIFF - SPU_X ) >> 1 ] );
	_mLAPF1 = & ( Regs [ ( mLAPF1 - SPU_X ) >> 1 ] );
	_mRAPF1 = & ( Regs [ ( mRAPF1 - SPU_X ) >> 1 ] );
	_mLAPF2 = & ( Regs [ ( mLAPF2 - SPU_X ) >> 1 ] );
	_mRAPF2 = & ( Regs [ ( mRAPF2 - SPU_X ) >> 1 ] );
	_vLIN = (s16*) & ( Regs [ ( vLIN - SPU_X ) >> 1 ] );
	_vRIN = (s16*) & ( Regs [ ( vRIN - SPU_X ) >> 1 ] );
	
	// enable audio filter by default
	AudioFilter_Enabled = true;

	
	// zero out registers
	//for ( i = 0; i < 256; i++ ) Regs [ i ] = 0;
	
	//VoiceOn_Bitmap = 0;
	
	s32 LowPassFilterCoefs [ 5 ];
	
	LowPassFilterCoefs [ 0 ] = gx [ 682 ];
	LowPassFilterCoefs [ 1 ] = gx [ 1365 ];
	LowPassFilterCoefs [ 2 ] = gx [ 2048 ];
	LowPassFilterCoefs [ 3 ] = gx [ 2730 ];
	LowPassFilterCoefs [ 4 ] = gx [ 3413 ];
	
	// reset the low pass filters
	LPF_L.Reset ();
	LPF_R.Reset ();
	LPF_L_Reverb.Reset ();
	LPF_R_Reverb.Reset ();
	
	// set lpf coefs
	LPF_L.SetFilter ( LowPassFilterCoefs );
	LPF_R.SetFilter ( LowPassFilterCoefs );
	LPF_L_Reverb.SetFilter ( LowPassFilterCoefs );
	LPF_R_Reverb.SetFilter ( LowPassFilterCoefs );
	
	
}


void SPU::UpdatePitch ( int Channel, u32 Pitch, u32 Reg_PMON, s32 PreviousSample )
{
	s64 Pitch_Step;
	s64 Pitch_Factor;
	
	Pitch_Step = Pitch;
	
	if ( Reg_PMON & ( 1 << Channel ) & ~1 )
	{
		// pitch modulation is enabled for channel //
		
		Pitch_Factor = ((s64)adpcm_decoder::clamp ( PreviousSample ));
		Pitch_Factor += 0x8000;
		Pitch_Step = ( Pitch_Step << 48 ) >> 48;
		Pitch_Step = ( Pitch_Step * Pitch_Factor ) >> 15;
		Pitch_Step &= 0xffff;
	}
	
	if ( Pitch_Step > 0x3fff ) Pitch_Step = 0x3fff;
	
	//Pitch_Counter += Pitch_Step;
	CurrentSample_Offset [ Channel ] += ( Pitch_Step << 20 );
	CurrentSample_Read [ Channel ] += ( Pitch_Step << 20 );
}

void SPU::Run ()
{
	int Channel;
	
	s32 Sample, PreviousSample, ChSampleL, ChSampleR, SampleL, SampleR, CD_SampleL, CD_SampleR, ReverbSampleL, ReverbSampleR;
	s32 FOutputL, FOutputR, ROutputL, ROutputR;
	
	//u32 ChannelOn, ChannelNoise, PitchMod, ReverbOn;
	
	u64 Temp;
	u32 ModeRate;
	
	ChRegs0_Layout *pChRegs0;
	ChRegs1_Layout *pChRegs1;
	
	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;
	
	// update number of spu cycles ran
	CycleCount++;
	
	//NextEvent_Cycle = *_DebugCycleCount + CPUCycles_PerSPUCycle;
	Set_NextEvent ( CPUCycles_PerSPUCycle );
	
#ifdef INLINE_DEBUG_RUN_NOTIFY
	debug << "\r\nSPU::Run";
	debug << " Cycle#" << dec << *_DebugCycleCount;
	debug << " NextEventCycle#" << dec << NextEvent_Cycle;
#endif

	// initialize current sample for left and right
	SampleL = 0;
	SampleR = 0;
	ReverbSampleL = 0;
	ReverbSampleR = 0;

	/*
	// check if SPU is on
	if ( ! ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] >> 15 ) )
	{
		// SPU is not on

#ifdef INLINE_DEBUG_RUN
	debug << "; Off" << "; Offset = " << ( 0x1f801daa - SPU_X ) << "; SPU_CTRL = " << Regs [ ( 0x1f801daa - SPU_X ) >> 1 ];
#endif

		return 0;
	}
	*/
	
	// SPU is on
	
	// *** TODO *** run SPU and output 1 sample LR
	
	/////////////////////////////////////////////////////////////////////
	// get what channels are enabled
	//ChannelOn = Regs [ ( ( CON_0 - SPU_X ) >> 1 ) & 0xff ];
	//ChannelOn |= ( (u32) ( Regs [ ( ( CON_1 - SPU_X ) >> 1 ) & 0xff ] ) ) << 16;
	
	// get what channels are set to noise
	//ChannelNoise = Regs [ ( ( NON_0 - SPU_X ) >> 1 ) & 0xff ];
	//ChannelNoise |= ( (u32) ( Regs [ ( ( NON_1 - SPU_X ) >> 1 ) & 0xff ] ) ) << 16;
	
	// get what channels are using frequency modulation
	//PitchMod = Regs [ ( PMON_0 - SPU_X ) >> 1 ];
	//PitchMod |= ( (u32) ( Regs [ ( PMON_1 - SPU_X ) >> 1 ] ) ) << 16;
	
	// get what channels have reverb on
	//ReverbOn = Regs [ ( RON_0 - SPU_X ) >> 1 ];
	//ReverbOn |= ( (u32) ( Regs [ ( RON_1 - SPU_X ) >> 1 ] ) ) << 16;

//#ifdef INLINE_DEBUG_RUN
//	debug << "; ChannelOn=" << ChannelOn << " KeyOn=" << KeyOn;
//#endif

	// if spu is enabled, run noise generator
	//if ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] >> 15 )
	if ( pCoreRegs->CTRL >> 15 )
	{
		RunNoiseGenerator ();
		
		// don't want this to happen per channel
		//if ( Regs [ ( MVOL_L - SPU_X ) >> 1 ] >> 15 )
		if ( pCoreRegs->MVOL_L >> 15 )
		{
			//MVOL_L_Value = (s64) ( (s16) Regs [ ( CMVOL_L - SPU_X ) >> 1 ] );
			MVOL_L_Value = (s32) ( (s16) pCoreRegs->CMVOL_L );
			
			//VolumeEnvelope ( MVOL_L_Value, MVOL_L_Cycles, Regs [ ( MVOL_L - SPU_X ) >> 1 ] & 0x7f, ( Regs [ ( MVOL_L - SPU_X ) >> 1 ] >> 13 ) & 0x3 );
			VolumeEnvelope ( MVOL_L_Value, MVOL_L_Cycles, pCoreRegs->MVOL_L & 0x7f, ( pCoreRegs->MVOL_L >> 13 ) & 0x3, true );
			
			//Regs [ ( CMVOL_L - SPU_X ) >> 1 ] = MVOL_L_Value;
			pCoreRegs->CMVOL_L = MVOL_L_Value;
		}
		else
		{
			// just set the current master volume L
			//Regs [ ( CMVOL_L - SPU_X ) >> 1 ] = Regs [ ( MVOL_L - SPU_X ) >> 1 ] << 1;
			pCoreRegs->CMVOL_L = pCoreRegs->MVOL_L << 1;
		}
	
		// don't want this to happen per channel
		//if ( Regs [ ( MVOL_R - SPU_X ) >> 1 ] >> 15 )
		if ( pCoreRegs->MVOL_R >> 15 )
		{
			//MVOL_R_Value = (s64) ( (s16) Regs [ ( CMVOL_R - SPU_X ) >> 1 ] );
			MVOL_R_Value = (s32) ( (s16) pCoreRegs->CMVOL_R );
			
			//VolumeEnvelope ( MVOL_R_Value, MVOL_R_Cycles, Regs [ ( MVOL_R - SPU_X ) >> 1 ] & 0x7f, ( Regs [ ( MVOL_R - SPU_X ) >> 1 ] >> 13 ) & 0x3 );
			VolumeEnvelope ( MVOL_R_Value, MVOL_R_Cycles, pCoreRegs->MVOL_R & 0x7f, ( pCoreRegs->MVOL_R >> 13 ) & 0x3, true );
			
			//Regs [ ( CMVOL_R - SPU_X ) >> 1 ] = MVOL_R_Value;
			pCoreRegs->CMVOL_R = MVOL_R_Value;
		}
		else
		{
			// just set the current master volume R
			//Regs [ ( CMVOL_R - SPU_X ) >> 1 ] = Regs [ ( MVOL_R - SPU_X ) >> 1 ] << 1;
			pCoreRegs->CMVOL_R = pCoreRegs->MVOL_R << 1;
		}
	}
	
	////////////////////////////
	// loop through channels
	for ( Channel = 0; Channel < 24; Channel++ )
	{
	
		// check if SPU is on
		//if ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] >> 15 )
		if ( pCoreRegs->CTRL >> 15 )
		{
			pChRegs0 = (ChRegs0_Layout*) ( & ( pCoreRegs->ChRegs0 [ Channel ] ) );
			pChRegs1 = (ChRegs1_Layout*) ( & ( pCoreRegs->ChRegs1 [ Channel ] ) );

			//SweepVolume ( Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ], VOL_L_Value [ Channel ], VOL_L_Constant [ Channel ], VOL_L_Constant75 [ Channel ] );
			//SweepVolume ( Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ], VOL_R_Value [ Channel ], VOL_R_Constant [ Channel ], VOL_R_Constant75 [ Channel ] );
			//SweepVolume ( Regs [ ( ( MVOL_L - SPU_X ) >> 1 ) & 0xff ], MVOL_L_Value, MVOL_L_Constant, MVOL_L_Constant75 );
			//SweepVolume ( Regs [ ( ( MVOL_R - SPU_X ) >> 1 ) & 0xff ], MVOL_R_Value, MVOL_R_Constant, MVOL_R_Constant75 );
			
			//if ( Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ] >> 15 )
			if ( pChRegs0->VOL_L >> 15 )
			{
				// set current volume left
				//VOL_L_Value [ Channel ] = (s64) ( (s16) Regs [ ( CVOL_L_START - SPU_X + ( Channel << 1 ) ) >> 1 ] );
				VOL_L_Value [ Channel ] = (s32) ( (s16) pChRegs1->CVOL_L );
				
				// perform envelope
				//VolumeEnvelope ( VOL_L_Value [ Channel ], VOL_L_Cycles [ Channel ], Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ] & 0x7f, ( Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ] >> 13 ) & 0x3 );
				VolumeEnvelope ( VOL_L_Value [ Channel ], VOL_L_Cycles [ Channel ], pChRegs0->VOL_L & 0x7f, ( pChRegs0->VOL_L >> 13 ) & 0x3, true );
				
				// store the new current volume left
				//Regs [ ( CVOL_L_START - SPU_X + ( Channel << 1 ) ) >> 1 ] = VOL_L_Value [ Channel ];
				pChRegs1->CVOL_L = VOL_L_Value [ Channel ];
			}
			else
			{
				// just set the current volume L
				//Regs [ ( CVOL_L_START - SPU_X + ( Channel << 1 ) ) >> 1 ] = Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ] << 1;
				pChRegs1->CVOL_L = pChRegs0->VOL_L << 1;
			}

			//if ( Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ] >> 15 )
			if ( pChRegs0->VOL_R >> 15 )
			{
				// set current volume right
				//VOL_R_Value [ Channel ] = (s64) ( (s16) Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ] );
				VOL_R_Value [ Channel ] = (s32) ( (s16) pChRegs1->CVOL_R );
				
				//VolumeEnvelope ( VOL_R_Value [ Channel ], VOL_R_Cycles [ Channel ], Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ] & 0x7f, ( Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ] >> 13 ) & 0x3 );
				VolumeEnvelope ( VOL_R_Value [ Channel ], VOL_R_Cycles [ Channel ], pChRegs0->VOL_R & 0x7f, ( pChRegs0->VOL_R >> 13 ) & 0x3, true );
				
				// store the new current volume right
				//Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ] = VOL_R_Value [ Channel ];
				pChRegs1->CVOL_R = VOL_R_Value [ Channel ];
			}
			else
			{
				// just set the current volume R
				//Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ] = Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ] << 1;
				pChRegs1->CVOL_R = pChRegs0->VOL_R << 1;
			}
			
			
			/////////////////////////////////////////////////////////////////////
			// update ADSR envelope
			
			// check adsr status
			switch ( ADSR_Status [ Channel ] )
			{
				case ADSR_MUTE:
				
					VOL_ADSR_Value [ Channel ] = 0;
					
					//Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ] = 0;
					pChRegs0->ENV_X = 0;
				
					break;
					
				case ADSR_ATTACK:
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nChannel#" << Channel;
#endif
			
#ifdef INLINE_DEBUG_RUN
	//debug << "; Attack; Rate=" << (((double)VOL_ATTACK_Constant [ Channel ])/64536) << "; Rate75=" << (((double)VOL_ATTACK_Constant75 [ Channel ])/64536);
	debug << "; Attack";
#endif

					///////////////////////////////////////////
					// ADSR - Attack Mode
					
					////////////////////////////////////////////////////
					// linear or psx pseudo exponential increase
					

					

					// saturate and switch to decay mode if volume goes above 32767
					if ( VOL_ADSR_Value [ Channel ] >= 32767 )
					{
#ifdef INLINE_DEBUG_RUN
	debug << "; DECAY_NEXT";
#endif

						VOL_ADSR_Value [ Channel ] = 32767;
						ADSR_Status [ Channel ] = ADSR_DECAY;
						
						// start envelope for decay mode
						ModeRate = pChRegs0->ADSR_0;
						
						//Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ( ModeRate >> 4 ) & 0xf ) << ( 2 ), 0x3 );
						Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ( ModeRate >> 4 ) & 0xf ) << ( 2 ), 0x3, false );
						
						// decay runs for 1T regardless
						// initialize cycles for start
						//VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ( ModeRate >> 4 ) & 0xf ) << ( 2 ), 0x3 );
					}
					else
					{
#ifdef INLINE_DEBUG_RUN
	debug << "; (before) VOL_ADSR_Value=" << hex << VOL_ADSR_Value [ Channel ] << "; Cycles=" << dec << Cycles [ Channel ];
#endif

						ModeRate = pChRegs0->ADSR_0;
						
						VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 8 ) & 0x7f, ( ModeRate >> 15 ) << 1, false );
						
						// clamp against upper bound
						if ( VOL_ADSR_Value [ Channel ] > 32767 )
						{
							VOL_ADSR_Value [ Channel ] = 32767;
						}
						
#ifdef INLINE_DEBUG_RUN
	debug << "; (after) VOL_ADSR_Value=" << hex << VOL_ADSR_Value [ Channel ] << "; Cycles=" << dec << Cycles [ Channel ] << "; Value=" << hex << ( ( ModeRate >> 8 ) & 0x7f ) << "; flags=" << ( ( ModeRate >> 15 ) << 1 );
#endif
					}
					
					break;
					
				case ADSR_DECAY:
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nChannel#" << Channel;
#endif
			
#ifdef INLINE_DEBUG_RUN
	//debug << "; Decay; Rate=" << (((double)VOL_DECAY_Constant [ Channel ])/(1<<30));
	debug << "; Decay";
#endif

					////////////////////////////////////////////
					// ADSR - Decay Mode
				
					////////////////////////////////////////////////
					// Exponential decrease

					
					// switch to sustain mode if we reach sustain level
					if ( VOL_ADSR_Value [ Channel ] < VOL_SUSTAIN_Level [ Channel ] )
					{
#ifdef INLINE_DEBUG_RUN
	debug << "; SUSTAIN_NEXT";
#endif
						ADSR_Status [ Channel ] = ADSR_SUSTAIN;
						
						/*
						VOL_ADSR_Value [ Channel ] = VOL_SUSTAIN_Level [ Channel ];

						// clamp against upper bound
						if ( VOL_ADSR_Value [ Channel ] > 32767 )
						{
							VOL_ADSR_Value [ Channel ] = 32767;
						}
						*/
						
						// start envelope for sustain mode
						//ModeRate = Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
						ModeRate = pChRegs0->ADSR_1;
						
						Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 6 ) & 0x7f, ModeRate >> 14, false );
						
						// clamp against upper bound
						if ( VOL_ADSR_Value [ Channel ] > 32767 )
						{
							VOL_ADSR_Value [ Channel ] = 32767;
						}
						
						// clamp against lower bound
						if ( VOL_ADSR_Value [ Channel ] < 0 )
						{
							VOL_ADSR_Value [ Channel ] = 32767;
						}
						
						// go ahead and get sustain mode started
						//VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 6 ) & 0x7f, ModeRate >> 14 );
					}
					else
					{
#ifdef INLINE_DEBUG_RUN
	debug << "; (before) VOL_ADSR_Value=" << hex << VOL_ADSR_Value [ Channel ] << "; Cycles=" << dec << Cycles [ Channel ];
#endif

						//ModeRate = Regs [ ( ADSR_0 >> 1 ) + ( Channel << 3 ) ];
						ModeRate = pChRegs0->ADSR_0;
						
						VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ( ModeRate >> 4 ) & 0xf ) << ( 2 ), 0x3, false );
					
#ifdef INLINE_DEBUG_RUN
	debug << "; (after) VOL_ADSR_Value=" << hex << VOL_ADSR_Value [ Channel ] << "; Cycles=" << dec << Cycles [ Channel ] << "; Value=" << hex << ( ( ModeRate >> 8 ) & 0x7f ) << "; flags=" << ( ( ModeRate >> 15 ) << 1 );
#endif
					}
					
					// *TODO* need to clamp envelope here because it could go above 0x7fff
					// also need to allow level to dip below sustain level for 1T
					
					break;
					
				case ADSR_SUSTAIN:
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nChannel#" << Channel;
#endif
			
#ifdef INLINE_DEBUG_RUN
	//debug << "; Sustain; Rate=" << (((double)VOL_SUSTAIN_Constant [ Channel ])/64536) << "; Rate75=" << (((double)VOL_SUSTAIN_Constant75 [ Channel ])/64536);
	debug << "; Sustain";
#endif

					/////////////////////////////////////////////
					// ADSR - Sustain Mode
					
#ifdef INLINE_DEBUG_RUN
	debug << "; (before) VOL_ADSR_Value=" << hex << VOL_ADSR_Value [ Channel ] << "; Cycles=" << dec << Cycles [ Channel ];
#endif
					
					//ModeRate = Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
					ModeRate = pChRegs0->ADSR_1;
					
					if ( ( ( ModeRate >> 14 ) & 1 ) && ( ( VOL_ADSR_Value [ Channel ] ) <= 0 ) )
					{
						// decreasing in sustain mode and at or below zero //

						pChRegs0->ENV_X = 0;
						
					}
					else
					{
					VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 6 ) & 0x7f, ModeRate >> 14, false );

#ifdef INLINE_DEBUG_RUN
	debug << "; (after) VOL_ADSR_Value=" << hex << VOL_ADSR_Value [ Channel ] << "; Cycles=" << dec << Cycles [ Channel ] << "; Value=" << hex << ( ( ModeRate >> 8 ) & 0x7f ) << "; flags=" << ( ( ModeRate >> 15 ) << 1 );
#endif

					
					// ***TODO*** potential bug here NEED TO CHECK IF RATE IS INCREASING OR DECREASING
					// ADSR1 -> bit 14 (0: sustain mode increasing, 1: sustain mode decreasing)
					if ( ( ModeRate >> 14 ) & 1 )
					{
						// decreasing in sustain mode //
						
						// or below zero
						/*
						if ( VOL_ADSR_Value [ Channel ] < 0 )
						{
							VOL_ADSR_Value [ Channel ] = 0;
						}
						*/
					}
					else
					{
						// increasing in sustain mode //
						
						// saturate if volume goes above 32767
						if ( VOL_ADSR_Value [ Channel ] > 32767 )
						{
							VOL_ADSR_Value [ Channel ] = 32767;
						}
					}
					
					}
					
					// we do not switch to release mode until key off signal is given
					
					break;
					
				case ADSR_RELEASE:
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nChannel#" << Channel;
#endif
			
#ifdef INLINE_DEBUG_RUN
	//debug << "; Release; Rate=" << (((double)VOL_RELEASE_Constant [ Channel ])/(1<<30));
	debug << "; Release";
#endif

					///////////////////////////////////////////////
					// ADSR - Release Mode
				
#ifdef INLINE_DEBUG_RUN
	debug << "; (before) VOL_ADSR_Value=" << hex << VOL_ADSR_Value [ Channel ] << "; Cycles=" << dec << Cycles [ Channel ];
#endif

					// when at or below zero we turn note off completely and set adsr volume to zero
					if ( VOL_ADSR_Value [ Channel ] <= 0 )
					{
						// ADSR volume is below zero in RELEASE mode //
					
						// saturate to zero
						VOL_ADSR_Value [ Channel ] = 0;
						
						// the channel on bit is not really for whether the channel is on or off
						//ChannelOn = ChannelOn & ~( 1 << Channel );
						
						// has not reached the end flag, but the channel should will not turn back on until it is keyed on
						// so should change status here to mute ??
						// otherwise, it is possible to change volume and ADSR settings, and start envelope without key-on
						// kill envelope //
						// if channel is not already set to mute, then turn off reverb for channel and mark we passed end of sample data
						if ( ADSR_Status [ Channel ] != ADSR_MUTE )
						{
							// turn off reverb for channel
							// when the channel ends
							//ReverbOn &= ~( 1 << Channel );
							pCoreRegs->RON &= ~( 1 << Channel );
						}
						
						// kill envelope //
						// turn off the channel
						ADSR_Status [ Channel ] = ADSR_MUTE;
						VOL_ADSR_Value [ Channel ] = 0;
					}
					else
					{
						// RELEASE mode //
						
						// *** note *** it is possible for ADSR volume to go negative for 1T in linear decrement mode //
						
						//ModeRate = Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
						ModeRate = pChRegs0->ADSR_1;
						
						VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1, false );
						
						//if ( VOL_ADSR_Value [ Channel ] < 0 )
						//{
						//	VOL_ADSR_Value [ Channel ] = -1;
						//}
					}
					
					// *** testing ***
					if ( VOL_ADSR_Value [ Channel ] < 0 )
					{
						// saturate to zero
						VOL_ADSR_Value [ Channel ] = 0;
					}

#ifdef INLINE_DEBUG_RUN
	debug << "; (after) VOL_ADSR_Value=" << hex << VOL_ADSR_Value [ Channel ] << "; Cycles=" << dec << Cycles [ Channel ] << "; Value=" << hex << ( ( ModeRate >> 8 ) & 0x7f ) << "; flags=" << ( ( ModeRate >> 15 ) << 1 );
#endif

					
						
					break;
			}
			
			/////////////////////////////////////////////////////////////////////////////
			// update ADSR Envelope Volume
			//Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ] = VOL_ADSR_Value [ Channel ];
			pChRegs0->ENV_X = VOL_ADSR_Value [ Channel ];
			
			//////////////////////////////////////////////////////////////
			// load sample
			
			// check if channel is set to noise
			//if ( ChannelNoise & ( 1 << Channel ) )
			if ( pCoreRegs->NON & ( 1 << Channel ) )
			{
				// channel is set to noise //
				
				Sample = NoiseLevel;
			}
			else
			{
				// channel is not set to noise //
				
				u32 SampleNumber = CurrentSample_Read [ Channel ] >> 32;
				
				//Sample = DecodedBlocks [ ( Channel * 28 ) + ( CurrentSample_Offset [ Channel ] >> 32 ) ];
				Sample = DecodedBlocks [ ( Channel << 5 ) + ( SampleNumber & 0x1f ) ];
				
				///////////////////////////////////////////
				// apply sample interpolation
				
#ifdef ENABLE_FILTER_PER_CHANNEL
				Sample = Calc_sample_gx ( CurrentSample_Read [ Channel ] >> 16, Sample, DecodedBlocks [ ( Channel << 5 ) + ( ( SampleNumber - 1 ) & 0x1f ) ],
				DecodedBlocks [ ( Channel << 5 ) + ( ( SampleNumber - 2 ) & 0x1f ) ], DecodedBlocks [ ( Channel << 5 ) + ( ( SampleNumber - 3 ) & 0x1f ) ] );
#else
				FilterBuf [ ( Channel << 2 ) + ( CycleCount & 3 ) ] = Sample;
				Sample = Calc_sample_gx ( CurrentSample_Read [ Channel ] >> 16, Sample, FilterBuf [ ( Channel << 2 ) + ( ( CycleCount - 1 ) & 3 ) ],
										FilterBuf [ ( Channel << 2 ) + ( ( CycleCount - 2 ) & 3 ) ], FilterBuf [ ( Channel << 2 ) + ( ( CycleCount - 3 ) & 3 ) ] );
#endif

				
				////////////////////////////////////
				// apply envelope volume
				// this does not apply when set to noise
				//Sample = ( Sample * ( (s64) ( (s16) Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ] ) ) ) >> c_iVolumeShift;
				Sample = ( Sample * ( (s32) ( (s16) pChRegs0->ENV_X ) ) ) >> c_iVolumeShift;
				
				//UpdatePitch ( Channel, Regs [ ( ( Channel << 4 ) + PITCH ) >> 1 ], PitchMod, PreviousSample );
				UpdatePitch ( Channel, pChRegs0->PITCH, pCoreRegs->PMON, PreviousSample );
				
				// check if channel is using frequency modulation
				// note: must ignore first channel
				/*
				if ( PitchMod & ( 1 << Channel ) & ~0x1 )
				{
					// channel is using frequency modulation //
					
					// for pitch LFO/frequency modulation? use formula
					// Pitch(channel) = (1 + Volume(channel-1)) * Pitch_Old(channel)
					// or rather Pitch(channel) = (32768 + Volume(channel-1)) * Pitch_Old(channel)
					
					//cout << "\nChannel#" << dec << Channel << " is using frequency modulation.\n";
					
					u32 NewPitch;
					
					NewPitch = ( ( ((s64)adpcm_decoder::clamp ( PreviousSample )) + 32768LL ) * ( (u64) Regs [ ( ( Channel << 4 ) + PITCH ) >> 1 ] ) ) >> c_iVolumeShift;
					
					// clamp pitch
					if ( NewPitch == 0 ) NewPitch = 1;
					if ( NewPitch > 0x3fff ) NewPitch = 0x3fff;
					
					// set new pitch
					Regs [ ( ( Channel << 4 ) + PITCH ) >> 1 ] = NewPitch;
					dSampleDT [ Channel ] = ( ((u64)NewPitch) << 32 ) >> 12;
				}
				*/
			}

			
			// store previous sample in case next channel uses frequency modulation
			PreviousSample = Sample;
			
			// save current sample for debugging
			Debug_CurrentRawSample [ Channel ] = Sample;

			// copy samples for voice1 and voice3 into buffer //
			
			if ( Channel == 1 )
			{
				RAM [ ( 0x0800 + DecodeBufferOffset ) >> 1 ] = Sample;
			}
			
			if ( Channel == 3 )
			{
				RAM [ ( 0x0c00 + DecodeBufferOffset ) >> 1 ] = Sample;
			}
			
			// check for interrupts
			//if ( ( ( ( (u32) Regs [ ( IRQA - SPU_X ) >> 1 ] ) << 3 ) == ( DecodeBufferOffset + 0x800 ) ) && ( Regs [ ( CTRL - SPU_X ) >> 1 ] & 0x40 ) )
			if ( ( ( ( (u32) pCoreRegs->IRQA ) << 3 ) == ( DecodeBufferOffset + 0x800 ) ) && ( pCoreRegs->CTRL & 0x40 ) )
			{
				// we have reached irq address - trigger interrupt
				SetInterrupt ();
				
				// interrupt
				//Regs [ ( STAT - SPU_X ) >> 1 ] |= 0x40;
				pCoreRegs->STAT |= 0x40;
			}
			
			//if ( ( ( ( (u32) Regs [ ( IRQA - SPU_X ) >> 1 ] ) << 3 ) == ( DecodeBufferOffset + 0xc00 ) ) && ( Regs [ ( CTRL - SPU_X ) >> 1 ] & 0x40 ) )
			if ( ( ( ( (u32) pCoreRegs->IRQA ) << 3 ) == ( DecodeBufferOffset + 0xc00 ) ) && ( pCoreRegs->CTRL & 0x40 ) )
			{
				// we have reached irq address - trigger interrupt
				SetInterrupt ();
				
				// interrupt
				//Regs [ ( STAT - SPU_X ) >> 1 ] |= 0x40;
				pCoreRegs->STAT |= 0x40;
			}
			
			//////////////////////////////////////////////////////////////////
			// apply volume processing
			
			
			// apply channel volume
			//ChSampleL = ( Sample * ((s64) ((s16)Regs [ ( CVOL_L_START - SPU_X + ( Channel << 1 ) ) >> 1 ]) ) ) >> c_iVolumeShift;
			//ChSampleR = ( Sample * ((s64) ((s16)Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ]) ) ) >> c_iVolumeShift;
			ChSampleL = ( Sample * ((s32) ( (s16) pChRegs1->CVOL_L ) ) ) >> c_iVolumeShift;
			ChSampleR = ( Sample * ((s32) ( (s16) pChRegs1->CVOL_R ) ) ) >> c_iVolumeShift;

#ifdef ENABLE_FILTER_PER_CHANNEL_FIR
				InputFirBufL [ ( Channel << 2 ) + ( CycleCount & 3 ) ] = ChSampleL;
				ChSampleL = Calc_sample_filter ( ChSampleL, InputFirBufL [ ( Channel << 2 ) + ( ( CycleCount - 1 ) & 3 ) ],
										InputFirBufL [ ( Channel << 2 ) + ( ( CycleCount - 2 ) & 3 ) ], OutputFirBufL [ ( Channel << 1 ) + ( ( CycleCount - 1 ) & 1 ) ],
										OutputFirBufL [ ( Channel << 1 ) + ( ( CycleCount - 2 ) & 1 ) ] );
										
				OutputFirBufL [ ( Channel << 1 ) + ( CycleCount & 1 ) ] = ChSampleL;

				InputFirBufL [ ( Channel << 2 ) + ( CycleCount & 3 ) ] = Sample;
				ChSampleL = Calc_sample_filter ( SampleL, InputFirBufL [ ( Channel << 2 ) + ( ( CycleCount - 1 ) & 3 ) ],
										InputFirBufL [ ( Channel << 2 ) + ( ( CycleCount - 2 ) & 3 ) ], OutputFirBufL [ ( Channel << 1 ) + ( ( CycleCount - 1 ) & 1 ) ],
										OutputFirBufL [ ( Channel << 1 ) + ( ( CycleCount - 2 ) & 1 ) ] );
										
				OutputFirBufL [ ( Channel << 1 ) + ( CycleCount & 1 ) ] = Sample;
#endif
			
			// check if channel is muted for debugging
			if ( !( Debug_ChannelEnable & ( 1 << Channel ) ) )
			{
				ChSampleL = 0;
				ChSampleR = 0;
			}

			// update current left/right volume for channel
			//Regs [ 256 + ( Channel << 1 ) + 0 ] = VOL_L_Value [ Channel ] >> 16;
			//Regs [ 256 + ( Channel << 1 ) + 1 ] = VOL_R_Value [ Channel ] >> 16;
			
			// check if reverb is on for channel
			//if ( ReverbOn & ( 1 << Channel ) )
			if ( pCoreRegs->RON & ( 1 << Channel ) )
			{
				ReverbSampleL += ChSampleL;
				ReverbSampleR += ChSampleR;
				
				// multiply by gain??
				//ReverbSampleL *= c_iMixer_Gain;
				//ReverbSampleR *= c_iMixer_Gain;
			}
			
			///////////////////////////////////////////////////////////////////
			// mix sample l/r
			
			SampleL += ChSampleL;
			SampleR += ChSampleR;
			
			// multiply by gain??
			//SampleL *= c_iMixer_Gain;
			//SampleR *= c_iMixer_Gain;
			
			//////////////////////////////////////////////////////////////////
			// Advance to next sample for channel
			//CurrentSample_Offset [ Channel ] += dSampleDT [ Channel ];
			//CurrentSample_Read [ Channel ] += dSampleDT [ Channel ];
			
			// save for debugging
			Debug_CurrentSample [ Channel ] = ( CurrentBlockAddress [ Channel ] >> 3 );	//CurrentSample_Offset [ Channel ] >> 32;
			Debug_CurrentRate [ Channel ] = dSampleDT [ Channel ] >> 20;
			
			// check if greater than or equal to 28 samples
			if ( CurrentSample_Offset [ Channel ] >= ( 28ULL << 32 ) )
			{
				// subtract 28
				CurrentSample_Offset [ Channel ] -= ( 28ULL << 32 );
				
				
				// check loop/end flag for current block
				/////////////////////////////////////////////////////////////////////////////////////////////
				// Check end flag and loop if needed (also checking to make sure loop bit is set also)
				// the loop/end flag is actually bit 0
				if ( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] & ( 0x1 << 8 ) )
				{
					////////////////////////////////////////////////////////////////////
					// reached loop/end flag
					
					// make sure channel is not set to mute
					if ( ADSR_Status [ Channel ] != ADSR_MUTE )
					{
						// passed end of sample data, even if looping
						//ChannelOn |= ( 1 << Channel );
						pCoreRegs->CON |= ( 1 << Channel );
					}

					// check if envelope should be killed
					//if ( KillEnvelope_Bitmap & ( 1 << Channel ) )
					if ( ( !( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] & ( 0x2 << 8 ) ) ) 
#ifdef REQUIRE_ALL_LOOP_FLAGS
						|| !(bLoopSet & (1 << Channel))
#endif
						)
					{
						// if channel is not already set to mute, then turn off reverb for channel and mark we passed end of sample data
						if ( ADSR_Status [ Channel ] != ADSR_MUTE )
						{
							// turn off reverb for channel
							//ReverbOn &= ~( 1 << Channel );
							pCoreRegs->RON &= ~( 1 << Channel );
						}
						
						// kill envelope //
						ADSR_Status [ Channel ] = ADSR_MUTE;
						VOL_ADSR_Value [ Channel ] = 0;
					}
					
					// set address of next sample block to be loop start address
					//CurrentBlockAddress [ Channel ] = ( ( ((u32)Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ]) << 3 ) & c_iRam_Mask );
					CurrentBlockAddress [ Channel ] = ( ( ( (u32)pChRegs0->LSA ) << 3 ) & c_iRam_Mask );
				}
				else
				{
					// did not reach loop/end flag //
					
					// advance to address of next sample block
					CurrentBlockAddress [ Channel ] += 16;
					CurrentBlockAddress [ Channel ] &= c_iRam_Mask;
				}
				
				/*
				// check for loop
				if ( ! ( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] & ( 0x2 << 8 ) ) )
				{
					// the loop is still there, just kill the envelope //
					KillEnvelope_Bitmap |= ( 1 << Channel );
				}
				
				// check if this is loop start
				if ( RAM [ ( CurrentBlockAddress [ Channel ] & c_iRam_Mask ) >> 1 ] & ( 0x4 << 8 ) )
				{
					// don't kill the envelope yet
					KillEnvelope_Bitmap &= ~( 1 << Channel );
				}
				*/
				
				//////////////////////////////////////////////////////////////////////////////
				// Check loop start flag and set loop address if needed
				// LOOP_START is bit 3 actually
				// note: LOOP_START is actually bit 2
				if ( ( RAM [ ( CurrentBlockAddress [ Channel ] & c_iRam_Mask ) >> 1 ] & ( 0x4 << 8 ) ) /* && ! ( LSA_Manual_Bitmap & ( 1 << Channel ) ) */ )
				{
					///////////////////////////////////////////////////
					// we are at loop start address
					// set loop start address
					//Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ] = ( CurrentBlockAddress [ Channel ] >> 3 );
					pChRegs0->LSA = ( CurrentBlockAddress [ Channel ] >> 3 );
					
					// clear killing of envelope
					//KillEnvelope_Bitmap &= ~( 1 << Channel );
				}
				
				//////////////////////////////////////////////////////////////////////////////
				// check if the IRQ address is in this block and if interrupts are enabled
				//if ( ( ( CurrentBlockAddress [ Channel ] >> 4 ) == ( Regs [ ( IRQA - SPU_X ) >> 1 ] >> 1 ) ) && ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] & 0x40 ) )
				if ( ( ( CurrentBlockAddress [ Channel ] >> 4 ) == ( pCoreRegs->IRQA >> 1 ) ) && ( pCoreRegs->CTRL & 0x40 ) )
				{
					// we have reached irq address - trigger interrupt
					SetInterrupt ();
					
					// interrupt
					//Regs [ ( 0x1f801dae - SPU_X ) >> 1 ] |= 0x40;
					pCoreRegs->STAT |= 0x40;
				}
				
				// decode the new block
				//SampleDecoder [ Channel ].decode_packet ( (adpcm_packet*) & ( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] ), & ( DecodedBlocks [ Channel * c_iSamplesPerBlock ] ) );
				CurrentSample_Write [ Channel ] += 28;
				SampleDecoder [ Channel ].decode_packet32 ( (adpcm_packet*) & ( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] ), DecodedSamples );
				for ( int i = 0; i < 28; i++ ) DecodedBlocks [ ( Channel << 5 ) + ( ( CurrentSample_Write [ Channel ] + i ) & 0x1f ) ] = DecodedSamples [ i ];
			}
		}
	}
	
	// store to audio buffer l/r
	// check if SPU is muted
	//if ( ! ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] & 0x4000 ) )
	if ( ! ( pCoreRegs->CTRL & 0x4000 ) )
	{
		SampleL = 0;
		SampleR = 0;
	}
	
	///////////////////////////////////////////////////////////////
	// handle extern audio input if it is enabled
	/*
	if ( REG ( CTRL ) & 0x2 )
	{
		/////////////////////////////////////////////////////////////////
		// Extern audio is enabled
		
		// request external l/r audio sample from device
		
		// apply volume processing for extern audio l/r
		
		// mix into final audio output for this sample l/r
	}
	*/
	
	// load from spu unconditionally //
	// request l/r audio sample from cd device
	s32 TempL, TempR, TempSample;
	TempSample = CD::_CD->Spu_ReadNextSample ();
	TempL = TempSample >> 16;
	TempR = ( TempSample << 16 ) >> 16;
			
	//////////////////////////////////////////////////////////////
	// handle CD audio input if it is enabled and cd is playing
	// note: 0x1 bit should only control output of cd audio, not input of cd audio to SPU
	//if ( REG ( CTRL ) & 0x1 )
	//{
		// cd audio is enabled for output
		//CD_SampleL = 0;
		//CD_SampleR = 0;
		
		//if ( CD::_CD->isPlaying () )
		//{
			/////////////////////////////////////////////////////
			// CD audio is enabled
			s32 tVOL_L, tVOL_R;
			
		
#ifdef INLINE_DEBUG_CDSOUND
	if ( TempL != 0 || TempR != 0 )
	{
		debug << "\r\nMixing CD; SampleL=" << hex << TempL << " SampleR=" << TempR;
	}
#endif

			// apply volume processing for cd audio l/r
			//tVOL_L = (s64) ( (s16) Regs [ ( CDVOL_L - SPU_X ) >> 1 ] );
			//tVOL_R = (s64) ( (s16) Regs [ ( CDVOL_R - SPU_X ) >> 1 ] );
			tVOL_L = (s32) ( (s16) pCoreRegs->CDVOL_L );
			tVOL_R = (s32) ( (s16) pCoreRegs->CDVOL_R );
			
			CD_SampleL = ( TempL * tVOL_L ) >> c_iVolumeShift;
			CD_SampleR = ( TempR * tVOL_R ) >> c_iVolumeShift;
			
			// store to cd audio buffer l/r
			//Mixer [ ( Mixer_WriteIdx + 0 ) & c_iMixerMask ] += TempL;
			//Mixer [ ( Mixer_WriteIdx + 1 ) & c_iMixerMask ] += TempR;
			
			// mix into final audio output for this sample l/r
			
			// sample should also be copied into sound ram for cd audio l/r area
			RAM [ ( 0x0000 + DecodeBufferOffset ) >> 1 ] = TempL;
			RAM [ ( 0x0400 + DecodeBufferOffset ) >> 1 ] = TempR;
			
			// check for interrupts
			//if ( ( ( ( (u32) Regs [ ( IRQA - SPU_X ) >> 1 ] ) << 3 ) == ( DecodeBufferOffset + 0x000 ) ) && ( Regs [ ( CTRL - SPU_X ) >> 1 ] & 0x40 ) )
			if ( ( ( ( (u32) pCoreRegs->IRQA ) << 3 ) == ( DecodeBufferOffset + 0x000 ) ) && ( pCoreRegs->CTRL & 0x40 ) )
			{
				// we have reached irq address - trigger interrupt
				SetInterrupt ();
				
				// interrupt
				//Regs [ ( STAT - SPU_X ) >> 1 ] |= 0x40;
				pCoreRegs->STAT |= 0x40;
			}
			
			//if ( ( ( ( (u32) Regs [ ( IRQA - SPU_X ) >> 1 ] ) << 3 ) == ( DecodeBufferOffset + 0x400 ) ) && ( Regs [ ( CTRL - SPU_X ) >> 1 ] & 0x40 ) )
			if ( ( ( ( (u32) pCoreRegs->IRQA ) << 3 ) == ( DecodeBufferOffset + 0x400 ) ) && ( pCoreRegs->CTRL & 0x40 ) )
			{
				// we have reached irq address - trigger interrupt
				SetInterrupt ();
				
				// interrupt
				//Regs [ ( STAT - SPU_X ) >> 1 ] |= 0x40;
				pCoreRegs->STAT |= 0x40;
			}
			
		//}
		
		// check if cd audio output is enabled
		//if ( REG ( CTRL ) & 0x1 )
		if ( pCoreRegs->CTRL & 0x1 )
		{
			// mix
			SampleL += CD_SampleL;
			SampleR += CD_SampleR;
			
			// multiply by gain??
			//SampleL *= c_iMixer_Gain;
			//SampleR *= c_iMixer_Gain;
			
			// check if reverb is on for cd
			//if ( REG ( CTRL ) & 0x4 )
			if ( pCoreRegs->CTRL & 0x4 )
			{
				// reverb is enabled for cd
				ReverbSampleL += CD_SampleL;
				ReverbSampleR += CD_SampleR;
				
				// multiply by gain??
				//ReverbSampleL *= c_iMixer_Gain;
				//ReverbSampleR *= c_iMixer_Gain;
			}
		}
	//}
	
	///////////////////////////////////////////////////////////
	// Apply FIR filter ??
	
	//SampleL = LPF_L.ApplyFilter ( SampleL );
	//SampleR = LPF_R.ApplyFilter ( SampleR );
	//ReverbSampleL = LPF_L_Reverb.ApplyFilter ( ReverbSampleL );
	//ReverbSampleR = LPF_R_Reverb.ApplyFilter ( ReverbSampleR );

#ifdef ENABLE_FILTER_PER_CORE
	// perform filter for regular audio out
	FOutputL = Calc_sample_filter ( SampleL, Lx1, Lx2, Ly1, Ly2 );
	FOutputR = Calc_sample_filter ( SampleR, Rx1, Rx2, Ry1, Ry2 );
	ROutputL = Calc_sample_filter ( ReverbSampleL, LReverb_x1, LReverb_x2, LReverb_y1, LReverb_y2 );
	ROutputR = Calc_sample_filter ( ReverbSampleR, RReverb_x1, RReverb_x2, RReverb_y1, RReverb_y2 );
	
	// clamp
	
	// put samples in history
	Lx2 = Lx1; Lx1 = SampleL;
	Ly2 = Ly1; Ly1 = FOutputL;
	Rx2 = Rx1; Rx1 = SampleR;
	Ry2 = Ry1; Ry1 = FOutputR;
	LReverb_x2 = LReverb_x1; LReverb_x1 = ReverbSampleL;
	LReverb_y2 = LReverb_y1; LReverb_y1 = ROutputL;
	RReverb_x2 = RReverb_x1; RReverb_x1 = ReverbSampleR;
	RReverb_y2 = RReverb_y1; RReverb_y1 = ROutputR;
	
	// haven't decided which sounds better, so this is optional for now
	if ( AudioFilter_Enabled )
	{
		// set filter outputs
		SampleL = FOutputL;
		SampleR = FOutputR;
		ReverbSampleL = ROutputL;
		ReverbSampleR = ROutputR;
	}
#endif

	
	///////////////////////////////////////////////////////////////////
	// apply effect processing
	
	// check that reverb is enabled
	//if ( REG ( CTRL ) & 0x80 )
	//{
		// reverb is enabled //
		// or rather, the output is always enabled //
		
		// determine if we do reverb for left or for right on this cycle
		if ( CycleCount & 1 )
		{
			
			// do reverb @ 22050 hz //
			// *** TODO *** check for interrupt in reverb buffer
			ProcessReverbR ( ReverbSampleR );
			
		}
		else
		{
			// process reverb
			ProcessReverbL ( ReverbSampleL );
		}
		
		// mix
		// the mix of reverb output should happen unconditionally...
		SampleL += ReverbL_Output;
		SampleR += ReverbR_Output;
		
		// multiply by gain??
		//SampleL *= c_iMixer_Gain;
		//SampleR *= c_iMixer_Gain;
	//}
	
	
	//////////////////////////////////////
	// ***TODO*** apply master volume
	//SampleL = ( SampleL * ( (s64) ((s16)Regs [ ( CMVOL_L - SPU_X ) >> 1 ]) ) ) >> c_iVolumeShift;
	//SampleR = ( SampleR * ( (s64) ((s16)Regs [ ( CMVOL_R - SPU_X ) >> 1 ]) ) ) >> c_iVolumeShift;
	SampleL = ( SampleL * ( (s32) ((s16)pCoreRegs->CMVOL_L) ) ) >> c_iVolumeShift;
	SampleR = ( SampleR * ( (s32) ((s16)pCoreRegs->CMVOL_R) ) ) >> c_iVolumeShift;
	
	// update current master volume registers
	//Regs [ ( 0x1f801db8 - SPU_X ) >> 1 ] = MVOL_L_Value >> 16;
	//Regs [ ( 0x1f801dba - SPU_X ) >> 1 ] = MVOL_R_Value >> 16;
	
	////////////////////////////////////////////////////////
	// Apply the Program's Global Volume set by user
	SampleL = ( SampleL * ( (s32) GlobalVolume ) ) >> c_iVolumeShift;
	SampleR = ( SampleR * ( (s32) GlobalVolume ) ) >> c_iVolumeShift;

	// mix samples
	Mixer [ ( Mixer_WriteIdx + 0 ) & c_iMixerMask ] = adpcm_decoder::clamp ( SampleL );
	Mixer [ ( Mixer_WriteIdx + 1 ) & c_iMixerMask ] = adpcm_decoder::clamp ( SampleR );
	
	// samples is now in mixer
	Mixer_WriteIdx += 2;
	
	// output one l/r sample
	if ( AudioOutput_Enabled && ( Mixer_WriteIdx - Mixer_ReadIdx ) >= PlayBuffer_Size /*c_iPlaySize*/ )
	{
		int testvalue0, testvalue1;
		
		//if ( !hWaveOut ) cout << "\n!hWaveOut; p1\n";
		
		// make sure the read index at write index minus the size of the buffer
		Mixer_ReadIdx = Mixer_WriteIdx - PlayBuffer_Size;
		
		if ( ( Mixer_WriteIdx - Mixer_ReadIdx ) == PlayBuffer_Size /*c_iPlayBuffer_Size*/ )
		{
			// play buffer cannot hold any more data //
			
			
			//testvalue0 = waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
			//testvalue1 = waveOutUnprepareHeader( hWaveOut, &header1, sizeof(WAVEHDR) );
			
			/*
			if ( !hWaveOut ) cout << "\n!hWaveOut; p2\n";
			
			if ( testvalue != MMSYSERR_NOERROR )
			{
				cout << "\nhps1x64 Error: waveOutUnprepareHeader returned an error\n";
			}
			
			if ( testvalue == MMSYSERR_INVALHANDLE )
			{
				cout << "\nSPU::Run; Invalid handle for audio output.";
			}
			
			if ( testvalue == MMSYSERR_NODRIVER )
			{
				cout << "\nSPU::Run; No audio device driver is present.";
			}
			
			if ( testvalue == MMSYSERR_NOMEM )
			{
				cout << "\nSPU::Run; Unable to allocate or lock memory.";
			}
			*/
			
			//while ( !( ((volatile u32) (header.dwFlags)) & WHDR_DONE ) )
			//while ( waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) ) == WAVERR_STILLPLAYING )
			//while ( testvalue0 == WAVERR_STILLPLAYING && testvalue1 == WAVERR_STILLPLAYING )
			while ( !( header0.dwFlags & WHDR_DONE ) && !( header1.dwFlags & WHDR_DONE ) )
			{
				//cout << "\nWaiting for samples to finish playing...";
				
				//MsgWaitForMultipleObjectsEx( NULL, NULL, 1 /*cWaitPeriod*/, QS_ALLINPUT, MWMO_ALERTABLE );
				
				//testvalue = waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
				testvalue0 = waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
				testvalue1 = waveOutUnprepareHeader( hWaveOut, &header1, sizeof(WAVEHDR) );
				
				/*
				if ( !hWaveOut ) cout << "\n!hWaveOut; p3\n";
				
				if ( testvalue != MMSYSERR_NOERROR )
				{
					cout << "\nhps1x64 Error: waveOutUnprepareHeader returned an error\n";
				}
				
				if ( testvalue == MMSYSERR_INVALHANDLE )
				{
					cout << "\nSPU::Run; Invalid handle for audio output.";
				}
				
				if ( testvalue == MMSYSERR_NODRIVER )
				{
					cout << "\nSPU::Run; No audio device driver is present.";
				}
				
				if ( testvalue == MMSYSERR_NOMEM )
				{
					cout << "\nSPU::Run; Unable to allocate or lock memory.";
				}
				*/
			}
		}
		
		//testvalue = waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
		//testvalue0 = waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
		//testvalue1 = waveOutUnprepareHeader( hWaveOut, &header1, sizeof(WAVEHDR) );
		
		//if ( !hWaveOut ) cout << "\n!hWaveOut; p4\n";
		
		//if ( header.dwFlags & WHDR_DONE /*waveOutUnprepareHeader( hWaveOut, &header, sizeof(WAVEHDR) ) != WAVERR_STILLPLAYING*/ )
		//if ( testvalue0 == MMSYSERR_NOERROR )
		if ( header0.dwFlags & WHDR_DONE )
		{
#ifdef INLINE_DEBUG_CDSOUND
	//debug << "\r\nPlaying; Mixer_WriteIdx=" << dec << Mixer_WriteIdx << " Mixer_ReadIdx=" << dec << Mixer_ReadIdx;
#endif

			ZeroMemory( &header0, sizeof(WAVEHDR) );
			
			//if ( !hWaveOut ) cout << "\n!hWaveOut; p5\n";
			
			// this must be the size in bytes
			header0.dwBufferLength = ( Mixer_WriteIdx - Mixer_ReadIdx ) * 2;	//size;
			
			// copy samples to play into the play buffer
			for ( int i = 0; i < ( Mixer_WriteIdx - Mixer_ReadIdx ); i++ ) PlayBuffer0 [ i ] = Mixer [ ( i + Mixer_ReadIdx ) & c_iMixerMask ];
			
			//if ( !hWaveOut ) cout << "\n!hWaveOut; p6\n";
			
			header0.lpData = (char*) PlayBuffer0;

			if ( AudioOutput_Enabled )
			{
				testvalue0 = waveOutPrepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
				
				/*
				if ( !hWaveOut ) cout << "\n!hWaveOut; p7\n";
				
				// only play audio if audio output is enabled by user
				if ( testvalue != MMSYSERR_NOERROR )
				{
					cout << "\nhps1x64 Error: waveOutPrepareHeader; Returns error.\n";
					
					if ( testvalue == MMSYSERR_INVALHANDLE )
					{
						cout << "\nSPU::Run; Invalid handle for audio output. CPU_CycleCount=" << *_DebugCycleCount;
						cout << "\nhWaveOut=" << (u64)hWaveOut << " hWaveOut_Save=" << hWaveOut_Save;
					}
					
					if ( testvalue == MMSYSERR_NODRIVER )
					{
						cout << "\nSPU::Run; No audio device driver is present.";
					}
					
					if ( testvalue == MMSYSERR_NOMEM )
					{
						cout << "\nSPU::Run; Unable to allocate or lock memory.";
					}
				}
				*/
				
				testvalue0 = waveOutWrite( hWaveOut, &header0, sizeof(WAVEHDR) );
				
				/*
				if ( !hWaveOut ) cout << "\n!hWaveOut; p8\n";
				
				if ( testvalue != MMSYSERR_NOERROR )
				{
					cout << "\nhps1x64 Error: waveOutWrite; Returns error.\n";
					
					if ( testvalue == MMSYSERR_INVALHANDLE )
					{
						cout << "\nSPU::Run; Invalid handle for audio output.";
					}
					
					if ( testvalue == MMSYSERR_NODRIVER )
					{
						cout << "\nSPU::Run; No audio device driver is present.";
					}
					
					if ( testvalue == MMSYSERR_NOMEM )
					{
						cout << "\nSPU::Run; Unable to allocate or lock memory.";
					}
				}
				*/
				
				//cout << "\nSent enabled audio successfully.";
			}
			
			// data in mixer has been played now
			Mixer_ReadIdx = Mixer_WriteIdx;
		}
		//else if ( testvalue1 == MMSYSERR_NOERROR )
		else if ( header1.dwFlags & WHDR_DONE )
		{
			ZeroMemory( &header1, sizeof(WAVEHDR) );
			
			//if ( !hWaveOut ) cout << "\n!hWaveOut; p5\n";
			
			// this must be the size in bytes
			header1.dwBufferLength = ( Mixer_WriteIdx - Mixer_ReadIdx ) * 2;	//size;
			
			// copy samples to play into the play buffer
			for ( int i = 0; i < ( Mixer_WriteIdx - Mixer_ReadIdx ); i++ ) PlayBuffer1 [ i ] = Mixer [ ( i + Mixer_ReadIdx ) & c_iMixerMask ];
			
			//if ( !hWaveOut ) cout << "\n!hWaveOut; p6\n";
			
			header1.lpData = (char*) PlayBuffer1;

			if ( AudioOutput_Enabled )
			{
				testvalue1 = waveOutPrepareHeader( hWaveOut, &header1, sizeof(WAVEHDR) );
				
				/*
				if ( !hWaveOut ) cout << "\n!hWaveOut; p7\n";
				
				// only play audio if audio output is enabled by user
				if ( testvalue != MMSYSERR_NOERROR )
				{
					cout << "\nhps1x64 Error: waveOutPrepareHeader; Returns error.\n";
					
					if ( testvalue == MMSYSERR_INVALHANDLE )
					{
						cout << "\nSPU::Run; Invalid handle for audio output. CPU_CycleCount=" << *_DebugCycleCount;
						cout << "\nhWaveOut=" << (u64)hWaveOut << " hWaveOut_Save=" << hWaveOut_Save;
					}
					
					if ( testvalue == MMSYSERR_NODRIVER )
					{
						cout << "\nSPU::Run; No audio device driver is present.";
					}
					
					if ( testvalue == MMSYSERR_NOMEM )
					{
						cout << "\nSPU::Run; Unable to allocate or lock memory.";
					}
				}
				*/
				
				testvalue1 = waveOutWrite( hWaveOut, &header1, sizeof(WAVEHDR) );
				
				/*
				if ( !hWaveOut ) cout << "\n!hWaveOut; p8\n";
				
				if ( testvalue != MMSYSERR_NOERROR )
				{
					cout << "\nhps1x64 Error: waveOutWrite; Returns error.\n";
					
					if ( testvalue == MMSYSERR_INVALHANDLE )
					{
						cout << "\nSPU::Run; Invalid handle for audio output.";
					}
					
					if ( testvalue == MMSYSERR_NODRIVER )
					{
						cout << "\nSPU::Run; No audio device driver is present.";
					}
					
					if ( testvalue == MMSYSERR_NOMEM )
					{
						cout << "\nSPU::Run; Unable to allocate or lock memory.";
					}
				}
				*/
				
				//cout << "\nSent enabled audio successfully.";
			}
			
			// data in mixer has been played now
			Mixer_ReadIdx = Mixer_WriteIdx;
		}
		/*
		else
		{
			cout << "\nhps1x64 Error: waveOutUnPrepareHeader returned an error\n";
			
			if ( testvalue == MMSYSERR_INVALHANDLE )
			{
				cout << "\nSPU::Run; Invalid handle for audio output.";
			}
			
			if ( testvalue == MMSYSERR_NODRIVER )
			{
				cout << "\nSPU::Run; No audio device driver is present.";
			}
			
			if ( testvalue == MMSYSERR_NOMEM )
			{
				cout << "\nSPU::Run; Unable to allocate or lock memory.";
			}
		}
		*/

		/*
		if ( waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) ) != WAVERR_STILLPLAYING )
		{
#ifdef INLINE_DEBUG_CDSOUND
	//debug << "\r\nPlaying; Mixer_WriteIdx=" << dec << Mixer_WriteIdx << " Mixer_ReadIdx=" << dec << Mixer_ReadIdx;
#endif

			ZeroMemory( &header0, sizeof(WAVEHDR) );
			
			// this must be the size in bytes
			header0.dwBufferLength = ( Mixer_WriteIdx - Mixer_ReadIdx ) * 2;	//size;
			
			// copy samples to play into the play buffer
			for ( int i = 0; i < ( Mixer_WriteIdx - Mixer_ReadIdx ); i++ ) PlayBuffer0 [ i ] = Mixer [ ( i + Mixer_ReadIdx ) & c_iMixerMask ];
			
			header0.lpData = (char*) PlayBuffer0;
			
			waveOutPrepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
			waveOutWrite( hWaveOut, &header0, sizeof(WAVEHDR) );
			
			// data in mixer has been played now
			Mixer_ReadIdx = Mixer_WriteIdx;
		}
		else if ( waveOutUnprepareHeader( hWaveOut, &header1, sizeof(WAVEHDR) ) != WAVERR_STILLPLAYING )
		{
#ifdef INLINE_DEBUG_CDSOUND
	//debug << "\r\nPlaying; Mixer_WriteIdx=" << dec << Mixer_WriteIdx << " Mixer_ReadIdx=" << dec << Mixer_ReadIdx;
#endif

			ZeroMemory( &header1, sizeof(WAVEHDR) );
			
			// this must be the size in bytes
			header1.dwBufferLength = ( Mixer_WriteIdx - Mixer_ReadIdx ) * 2;	//size;
			
			// copy samples to play into the play buffer
			for ( int i = 0; i < ( Mixer_WriteIdx - Mixer_ReadIdx ); i++ ) PlayBuffer1 [ i ] = Mixer [ ( i + Mixer_ReadIdx ) & c_iMixerMask ];
			
			header.lpData = (char*) PlayBuffer1;
			
			waveOutPrepareHeader( hWaveOut, &header1, sizeof(WAVEHDR) );
			waveOutWrite( hWaveOut, &header1, sizeof(WAVEHDR) );
			
			// data in mixer has been played now
			Mixer_ReadIdx = Mixer_WriteIdx;
		}
		*/
		
		// check if the buffer size changed
		if ( PlayBuffer_Size != NextPlayBuffer_Size )
		{
			PlayBuffer_Size = NextPlayBuffer_Size;
		}

	}
	
	///////////////////////////////////////////////////////////////////////////
	// Update Decode Buffer Offset (for CD L/R,Voice1+Voice3 decode area)
	DecodeBufferOffset += 2;
	DecodeBufferOffset &= ( DecodeBufferSize - 1 );
	
	///////////////////////////////////////////////////////
	// update whether decoding in first/second half of buffer
	//Regs [ ( 0x1f801dae - SPU_X ) >> 1 ] &= ~( 0x200 << 2 );
	//Regs [ ( 0x1f801dae - SPU_X ) >> 1 ] |= ( DecodeBufferOffset & 0x200 ) << 2;
	pCoreRegs->STAT &= ~( 0x200 << 2 );
	pCoreRegs->STAT |= ( DecodeBufferOffset & 0x200 ) << 2;
	
	// write back ChannelOn
	//Regs [ ( ( CON_0 - SPU_X ) >> 1 ) & 0xff ] = (u16) ChannelOn;
	//Regs [ ( ( CON_1 - SPU_X ) >> 1 ) & 0xff ] = (u16) ( ChannelOn >> 16 );
	
	// write back reverb on
	//Regs [ ( RON_0 - SPU_X ) >> 1 ] = (u16) ReverbOn;
	//Regs [ ( RON_1 - SPU_X ) >> 1 ] = (u16) ( ReverbOn >> 16 );
}



u32 SPU::Read ( u32 Address )
{

	u32 Channel;

//#ifdef INLINE_DEBUG_READ
//	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
//#endif

	// when address is outside range, return zero for now
	if ( Address >= ( SPU_X + ( c_iNumberOfRegisters << 1 ) ) ) return 0;
	
	// Read SPU register value

#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_CON_DEFAULT
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; " << _SPU->RegisterNames [ ( Address - SPU_X ) >> 1 ] << "=" << hex << setw ( 4 ) << _SPU->Regs [ ( Address - SPU_X ) >> 1 ];
#endif


#ifdef INLINE_DEBUG_READ
	if ( ( ( Address >> 4 ) & 0xff ) <= 0xd7 )
	{
		///////////////////////////////////////////////////////////////////
		// this is the voice data area
		
		Channel = ( ( Address >> 4 ) & 0xff ) - 0xc0;
		
//#ifdef INLINE_DEBUG_WRITE
//	debug << "; Channel#" << Channel;
//#endif

		///////////////////////////////////////////////////////////////
		// Determine what register this is for
		switch ( Address & 0xf )
		{
			case ENV_X:
#ifdef INLINE_DEBUG_READ
debug << "; ADSR0=" << _SPU->Regs [ ( ( Channel << 4 ) + ADSR_0 ) >> 1 ] << " ADSR1=" << _SPU->Regs [ ( ( Channel << 4 ) + ADSR_1 ) >> 1 ];
debug << " State=" << _SPU->ADSR_Status [ Channel ] << " Cycles=" << dec << _SPU->Cycles [ Channel ];
//debug << " Step=" << 
#endif

				break;
		}
	}
#endif
	

	// just return the SPU register value
	// don't AND with 0xff
	return (u32) _SPU->Regs [ ( Address - SPU_X ) >> 1 ];

	
	/*
	switch ( Address )
	{
		///////////////////////////////////
		// SPU Control
		case 0x1f801daa:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_CTRL
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; CTRL=" << hex << setw ( 4 ) << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif

			return (u32) _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
			break;
	
		////////////////////////////////////
		// SPU Status
		case 0x1f801dac:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_STAT_0
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; STAT_0= " << hex << setw ( 4 ) << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif

			// unknown what this is for - gets loaded with 4 when SPU gets initialized by BIOS
			return (u32) _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
			break;
			
		/////////////////////////////////////
		// SPU Status
		case 0x1f801dae:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_STAT_1
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; STAT_1= " << hex << setw ( 4 ) << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif
			
			// bit 10 - Rd: 0 - SPU ready to transfer; 1 - SPU not ready
			// bit 11 - Dh: 0 - decoding in first half of buffer; 1 - decoding in second half of buffer
			
			return (u32) _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
			break;
			
		case KON_0:
#if defined INLINE_DEBUG_READ_CHANNELONOFF || defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_KON_0
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; KON_0= " << hex << setw ( 4 ) << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif

			// just return the SPU register value
			return (u32) _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
			
			break;

		case KON_1:
#if defined INLINE_DEBUG_READ_CHANNELONOFF || defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_KON_1
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; KON_1= " << hex << setw ( 4 ) << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif

			// just return the SPU register value
			return (u32) _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
			
			break;

		case KOFF_0:
#if defined INLINE_DEBUG_READ_CHANNELONOFF || defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_KOFF_0
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; KOFF_0= " << hex << setw ( 4 ) << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif

			// just return the SPU register value
			return (u32) _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
			
			break;
			
		case KOFF_1:
#if defined INLINE_DEBUG_READ_CHANNELONOFF || defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_KOFF_1
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; KOFF_1= " << hex << setw ( 4 ) << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif

			// just return the SPU register value
			return (u32) _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
			
			break;

		case CON_0:
#if defined INLINE_DEBUG_READ_CHANNELONOFF || defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_CON_0
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; CON_0= " << hex << setw ( 4 ) << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif

			// just return the SPU register value
			return (u32) _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
			
			break;
			
		case CON_1:
#if defined INLINE_DEBUG_READ_CHANNELONOFF || defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_CON_1
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; CON_1= " << hex << setw ( 4 ) << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif

			// just return the SPU register value
			return (u32) _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
			
			break;
			
			
		default:
#ifdef INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_CON_DEFAULT
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; " << _SPU->RegisterNames [ ( Address - SPU_X ) >> 1 ] << "=" << hex << setw ( 4 ) << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif

			// just return the SPU register value
			// don't AND with 0xff
			return (u32) _SPU->Regs [ ( Address - SPU_X ) >> 1 ];
			
			break;
			
	}
	*/
	
}


void SPU::Write ( u32 Address, u32 Data, u32 Mask )
{
//#ifdef INLINE_DEBUG_WRITE
//	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
//#endif

	u32 Channel;
	u32 Rate;
	
	u16 ModeRate;
	
	int Ch;
	
	//////////////////////////////////////////////////////////////////////
	// *** TODO *** WRITES TO ODD ADDRESSES ARE IGNORED
	if ( Address & 1 ) return;
	
	////////////////////////////////////////////////////////////////////
	// *** TODO *** PROPERLY IMPLEMENT 8-BIT WRITES WITH 16-BIT VALUES
		
	// make sure the address is in the correct range
	//if ( Address < Regs_Start || Address > Regs_End ) return;
	
	if ( Address >= ( SPU_X + ( c_iNumberOfRegisters << 1 ) ) ) return;
	
	// Write SPU register value
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// ***TODO*** 32-bit writes are probably treated as 16-bit writes on hardware

	////////////////////////////////////////////
	// writes to SPU are 16 bit writes
	Data &= 0xffff;
	
	if ( Mask != 0xffff ) cout << "\nhps1x64 ALERT: SPU::Write Mask=" << hex << Mask;
	
	////////////////////////////////////////////////////////////////////
	// Check if this is the voice data area (0x1f801c00-0x1f801d7f)
	if ( ( ( Address >> 4 ) & 0xff ) <= 0xd7 )
	{
		///////////////////////////////////////////////////////////////////
		// this is the voice data area
		
		Channel = ( ( Address >> 4 ) & 0xff ) - 0xc0;
		
//#ifdef INLINE_DEBUG_WRITE
//	debug << "; Channel#" << Channel;
//#endif

		///////////////////////////////////////////////////////////////
		// Determine what register this is for
		switch ( Address & 0xf )
		{
			case VOL_L:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_VOL_L
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)VOL_L=" << _SPU->Regs [ ( ( Channel << 4 ) + VOL_L ) >> 1 ];
#endif
			
				// writing constant volume
				_SPU->Regs [ ( ( Channel << 4 ) + VOL_L ) >> 1 ] = Data;
				
				if ( Data >> 15 )
				{
					Start_VolumeEnvelope ( _SPU->VOL_L_Value [ Channel ], _SPU->VOL_L_Cycles [ Channel ], Data & 0x7f, ( Data >> 13 ) & 0x3 );
				}
				else
				{
					// store the new current volume left
					_SPU->Regs [ ( CVOL_L_START - SPU_X + ( Channel << 1 ) ) >> 1 ] = Data << 1;
					
#ifdef VERBOSE_REVERSE_PHASE
					if ( _SPU->Regs [ ( CVOL_L_START - SPU_X + ( Channel << 1 ) ) >> 1 ] & 0x8000 )
					{
						cout << "\nhps1x64: SPU: ALERT: CVOL_L is negative.\n";
					}
#endif
				}
	
				break;
				
			case VOL_R:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_VOL_R
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)VOL_R=" << _SPU->Regs [ ( ( Channel << 4 ) + VOL_R ) >> 1 ];
#endif
			
				// writing constant volume
				_SPU->Regs [ ( ( Channel << 4 ) + VOL_R ) >> 1 ] = Data;

				if ( Data >> 15 )
				{
					Start_VolumeEnvelope ( _SPU->VOL_R_Value [ Channel ], _SPU->VOL_R_Cycles [ Channel ], Data & 0x7f, ( Data >> 13 ) & 0x3 );
				}
				else
				{
					// store the new current volume left
					_SPU->Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ] = Data << 1;
					
#ifdef VERBOSE_REVERSE_PHASE
					if ( _SPU->Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ] & 0x8000 )
					{
						cout << "\nhps1x64: SPU: ALERT: CVOL_R is negative.\n";
					}
#endif
				}
				
			
				break;
				
			case PITCH:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_PITCH
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)PITCH=" << _SPU->Regs [ ( ( Channel << 4 ) + PITCH ) >> 1 ];
#endif
			
				// only bits 0-13 are used
				Data &= 0x3fff;
				
				/////////////////////////////////////////////////////////////
				// Pitch of the sound
				// frequency = (pitch/4096) * f0
				_SPU->Regs [ ( ( Channel << 4 ) + PITCH ) >> 1 ] = Data;
				

				// (32.32)dSampleDT = Pitch / 4096
				// for PS2 this would be (32.32)dSampleDT = ( 48 / SampleRateInKHZ ) * ( Pitch / 4096 )
				// where SampleRateInKHZ is 41 for a 41000 Hz sample rate
				// *** testing *** copy pitch over on key on
				_SPU->dSampleDT [ Channel ] = ( ((u64)Data) << 32 ) >> 12;
				
				break;
				
			case SSA_X:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_SSA_X
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)SSA_X=" << _SPU->Regs [ ( ( Channel << 4 ) + SSA_X ) >> 1 ];
#endif

				// align ??
				//Data = ( Data + 1 ) & ~1;
				Data &= ~1;
			
				////////////////////////////////////////////
				// writing start address for the sound
				_SPU->Regs [ ( ( Channel << 4 ) + SSA_X ) >> 1 ] = Data;
				
				// *** testing ***
				//_SPU->Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ] = Data;
				
				break;
				
			case ADSR_0:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ADSR_0
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)ADSR_0=" << _SPU->Regs [ ( ( Channel << 4 ) + ADSR_0 ) >> 1 ];
#endif
			
				//////////////////////////////////////////////
				// writing ADSR_0 register
				_SPU->Regs [ ( ( Channel << 4 ) + ADSR_0 ) >> 1 ] = Data;
				
				// set sustain level (48.16 fixed point)
				// this is now 16.0 fixed point
				// but this might be signed possibly, where 0xf means -1 ?
				_SPU->VOL_SUSTAIN_Level [ Channel ] = ( ( Data & 0xf ) + 1 ) << ( 11 );
				//_SPU->VOL_SUSTAIN_Level [ Channel ] = ( ( ( Data & 0xf ) + 1 ) << ( 11 ) ) - 1;
				
				break;
				
			case ADSR_1:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ADSR_1
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)ADSR_1=" << _SPU->Regs [ ( ( Channel << 4 ) + ADSR_1 ) >> 1 ];
#endif
			
				///////////////////////////////////////////////////
				// writing ADSR_1 register
				_SPU->Regs [ ( ( Channel << 4 ) + ADSR_1 ) >> 1 ] = Data;
				
				
				break;
				
			case ENV_X:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ENV_X
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)ENV_X=" << _SPU->Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ];
#endif
			
				////////////////////////////////////////////////
				// writing to current envelope volume register
				
				// *** TODO *** this can actually be set to cause a jump in the value //
				//if ( Data > 0x7fff ) Data = 0x7fff;
				_SPU->Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ] = Data;
				
				// *** testing *** allow for jump in adsr value at any time
				_SPU->VOL_ADSR_Value [ Channel ] = Data;
				
				break;
				
			case LSA_X:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_LSA_X
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)LSA_X=" << _SPU->Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ];
	debug << "; CyclesFromStart=" << dec << ( _SPU->CycleCount - _SPU->StartCycle_Channel [ Channel ] );
#endif
			
				// can set this after 4T of key-on
				if ( ( _SPU->CycleCount - _SPU->StartCycle_Channel [ Channel ] ) >= 4 )
				{
					// align ??
					//Data = ( Data + 1 ) & ~1;
					Data &= ~1;
					
					///////////////////////////////////////////////////////
					// writing to loop start address for sound
					// this gets set by the sound when it is loaded
					_SPU->Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ] = Data;
					
					// loop start was manually specified - so ignore anything specified in sample
					_SPU->LSA_Manual_Bitmap |= ( 1 << Channel );
				}
				
				break;
			
		}
	}
	else
	{
	
		/////////////////////////////////////////////////////////////
		// not writing to voice data area
	
		switch ( Address )
		{
			// reverb work address
			case RVWA:
			
				_SPU->ReverbWork_Start = ( Data & 0xffff ) << 3;
				_SPU->ReverbWork_Size = c_iRam_Size - _SPU->ReverbWork_Start;
				_SPU->Reverb_BufferAddress = _SPU->ReverbWork_Start;
				
				//_SPU->Regs [ ( ( RVWA - SPU_X ) >> 1 ) & 0xff ] = (u16)Data;
				pCoreRegs->RVWA = Data;
				
				// check if interrupt triggered by reverb work address
				//if ( ( _SPU->Reverb_BufferAddress == ( ( (u32) _SPU->Regs [ ( IRQA - SPU_X ) >> 1 ] ) << 3 ) ) && ( _SPU->Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] & 0x40 ) )
				if ( ( _SPU->Reverb_BufferAddress == ( ( (u32) pCoreRegs->IRQA ) << 3 ) ) && ( pCoreRegs->CTRL & 0x40 ) )
				{
					// we have reached irq address - trigger interrupt
					SetInterrupt ();
					
					// interrupt
					//_SPU->Regs [ ( 0x1f801dae - SPU_X ) >> 1 ] |= 0x40;
					pCoreRegs->STAT |= 0x40;
				}
				
				break;
			
			
			//////////////////////////////////////////////////////////////////////////////
			// irq address - reading this address in sound buffer causes SPU interrupt
			case IRQA:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_IRQA
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)IRQA=" << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif

				_SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ] = (u16)Data;
				
				break;
			
			
			////////////////////////////////////////
			// Sound Buffer Address
			case SBA:	//0x1f801da6
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_SBA
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)SBA=" << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif
			
				///////////////////////////////////
				// set next sound buffer address
				_SPU->NextSoundBufferAddress = ( Data << 3 ) & c_iRam_Mask;
				_SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ] = (u16)Data;

				break;
				
			//////////////////////////////////////////
			// Data forwarding register
			case DATA:	//0x1f801da8
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_DATA
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; DATA; _SPU->BufferIndex=" << _SPU->BufferIndex;
#endif
			
				///////////////////////////////////////////////////////
				// Send data to sound buffer and update next address
				//RAM [ ( ( NextSoundBufferAddress & c_iRam_Mask ) >> 1 ) ] = (u16) Data;
				//NextSoundBufferAddress += 2;
				
				///////////////////////////////////////////////////////////////
				// Actually we're supposed to send the data into SPU buffer
				
				// buffer can be written into at any time
				if ( _SPU->BufferIndex < 32 )
				{
					_SPU->Buffer [ _SPU->BufferIndex++ ] = (u16) Data;
				}
				
				break;
				
			///////////////////////////////////
			// SPU Control
			case CTRL:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CTRL
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)CTRL=" << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif

				//_SPU->Regs [ ( ( CTRL - SPU_X ) >> 1 ) & 0xff ] = (u16)Data;
				pCoreRegs->CTRL = Data;
				
				// copy bits 0-5 to stat
				//_SPU->Regs [ ( STAT - SPU_X ) >> 1 ] = ( _SPU->Regs [ ( STAT - SPU_X ) >> 1 ] & 0xffc0 ) | ( _SPU->Regs [ ( CTRL - SPU_X ) >> 1 ] & 0x3f );
				pCoreRegs->STAT = ( pCoreRegs->STAT & 0xffc0 ) | ( pCoreRegs->CTRL & 0x3f );
				
				// copy bit 5 of ctrl to bit 7 of stat
				// the shift needs to be by 4 here, or else this makes no sense??
				//switch ( ( _SPU->Regs [ ( CTRL - SPU_X ) >> 1 ] >> 4 ) & 0x3 )
				switch ( ( pCoreRegs->CTRL >> 4 ) & 0x3 )
				{
					case 0:
						// no reads or writes (stop)
						//_SPU->Regs [ ( STAT - SPU_X ) >> 1 ] = ( _SPU->Regs [ ( STAT - SPU_X ) >> 1 ] & ~0x0380 ) | ( 0 << 7 );
						pCoreRegs->STAT = ( pCoreRegs->STAT & ~0x0380 ) | ( 0 << 7 );
						break;
						
					case 1:
						// manual write
						//_SPU->Regs [ ( STAT - SPU_X ) >> 1 ] = ( _SPU->Regs [ ( STAT - SPU_X ) >> 1 ] & ~0x0380 ) | ( 0 << 7 );
						pCoreRegs->STAT = ( pCoreRegs->STAT & ~0x0380 ) | ( 0 << 7 );
						break;
						
					case 2:
						// dma write
						//_SPU->Regs [ ( STAT - SPU_X ) >> 1 ] = ( _SPU->Regs [ ( STAT - SPU_X ) >> 1 ] & ~0x0380 ) | ( 0x3 << 7 );
						pCoreRegs->STAT = ( pCoreRegs->STAT & ~0x0380 ) | ( 0x3 << 7 );
						break;
						
					case 3:
						// dma read
						//_SPU->Regs [ ( STAT - SPU_X ) >> 1 ] = ( _SPU->Regs [ ( STAT - SPU_X ) >> 1 ] & ~0x0380 ) | ( 0x5 << 7 );
						pCoreRegs->STAT = ( pCoreRegs->STAT & ~0x0380 ) | ( 0x5 << 7 );
						break;
				}
				
				// check if disabling/acknowledging interrupt
				if ( ! ( Data & 0x40 ) )
				{
					// clear interrupt
					//_SPU->Regs [ ( STAT - SPU_X ) >> 1 ] &= ~0x40;
					pCoreRegs->STAT &= ~0x40;
				}
				
				///////////////////////////////////////////////////////////////////////////
				// if DMA field was written as 01 then write SPU buffer into sound RAM
				if ( ( Data & 0x30 ) == 0x10 )
				{
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CTRL
	debug << "; MANUAL WRITE";
#endif

					///////////////////////////////////////////////////////
					// write SPU buffer into sound ram
					//for ( int i = 0; i < 32; i++ ) _SPU->RAM [ ( ( _SPU->NextSoundBufferAddress & c_iRam_Mask ) >> 1 ) + i ] = _SPU->Buffer [ i ];
					for ( int i = 0; i < _SPU->BufferIndex; i++ )
					{
						_SPU->RAM [ ( ( _SPU->NextSoundBufferAddress + ( i << 1 ) ) & c_iRam_Mask ) >> 1 ] = _SPU->Buffer [ i ];
					}
					
					//////////////////////////////////////////////////////////
					// update next sound buffer address
					//_SPU->NextSoundBufferAddress += 64;
					_SPU->NextSoundBufferAddress += ( _SPU->BufferIndex << 1 );
					
					//////////////////////////////////////////////////
					// reset buffer index
					_SPU->BufferIndex = 0;
					
					////////////////////////////////////////////////////////////
					// save back into the sound buffer address register
					// sound buffer address register does not change
					//_SPU->Regs [ ( ( REG_SoundBufferAddress - SPU_X ) >> 1 ) & 0xff ] = (u16) (_SPU->NextSoundBufferAddress >> 3);

					// *** testing ***
					//_SPU->SpuTransfer_Complete ();
				}
				

				break;
		
			////////////////////////////////////
			// SPU Status
			case INIT:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_INIT
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)INIT=" << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif
				// unknown what this is for - gets loaded with 4 when SPU gets initialized by BIOS
				_SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ] = (u16)Data;
				

				break;
				
			/////////////////////////////////////
			// SPU Status
			case STAT:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_STAT
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)STAT=" << _SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ];
#endif
				
				// bit 10 - Rd: 0 - SPU ready to transfer; 1 - SPU not ready
				// bit 11 - Dh: 0 - decoding in first half of buffer; 1 - decoding in second half of buffer
				//_SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ] = (u16)Data;
				
				break;
				
				
			case MVOL_L:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_MVOL_L
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)MVOL_L=" << _SPU->Regs [ ( MVOL_L - SPU_X ) >> 1 ];
#endif
			
				////////////////////////////////////////
				// Master Volume Left
				//_SPU->Regs [ ( MVOL_L - SPU_X ) >> 1 ] = (u16) Data;
				pCoreRegs->MVOL_L = Data;
				
				if ( Data >> 15 )
				{
					Start_VolumeEnvelope ( _SPU->MVOL_L_Value, _SPU->MVOL_L_Cycles, Data & 0x7f, ( Data >> 13 ) & 0x3 );
					
#ifdef VERBOSE_REVERSE_PHASE
					// check if reverse phase
					if ( pCoreRegs->MVOL_L & ( 1 << 12 ) )
					{
						cout << "\nhps1x64: SPU: ALERT: MVOL_L is negative.\n";
					}
#endif

				}
				else
				{
					// store the new current volume left
					//_SPU->Regs [ ( CMVOL_L - SPU_X ) >> 1 ] = Data << 1;
					pCoreRegs->CMVOL_L = Data << 1;
					
#ifdef VERBOSE_REVERSE_PHASE
					// need to know if volume is negative for now
					if ( pCoreRegs->CMVOL_L & 0x8000 )
					{
						cout << "\nhps1x64: SPU: ALERT: CMVOL_L is negative.\n";
					}
#endif

				}
				
				break;
				
			case MVOL_R:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_MVOL_R
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)MVOL_R=" << _SPU->Regs [ ( MVOL_R - SPU_X ) >> 1 ];
#endif
			
				////////////////////////////////////////
				// Master Volume Right
				//_SPU->Regs [ ( MVOL_R - SPU_X ) >> 1 ] = (u16) Data;
				pCoreRegs->MVOL_R = Data;
				
				if ( Data >> 15 )
				{
					Start_VolumeEnvelope ( _SPU->MVOL_R_Value, _SPU->MVOL_R_Cycles, Data & 0x7f, ( Data >> 13 ) & 0x3 );
					
#ifdef VERBOSE_REVERSE_PHASE
					// check if reverse phase
					if ( pCoreRegs->MVOL_R & ( 1 << 12 ) )
					{
						cout << "\nhps1x64: SPU: ALERT: MVOL_R is negative.\n";
					}
#endif

				}
				else
				{
					// store the new current volume left
					//_SPU->Regs [ ( CMVOL_R - SPU_X ) >> 1 ] = Data << 1;
					pCoreRegs->CMVOL_R = Data << 1;
					
#ifdef VERBOSE_REVERSE_PHASE
					// need to know if volume is negative for now
					if ( pCoreRegs->CMVOL_R & 0x8000 )
					{
						cout << "\nhps1x64: SPU: ALERT: CMVOL_R is negative.\n";
					}
#endif

				}
				
				break;
				
			case EVOL_L:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_EVOL_L
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)EVOL_L=" << _SPU->Regs [ ( EVOL_L - SPU_X ) >> 1 ];
#endif
			
				////////////////////////////////////////
				// Effect Volume Left
				//_SPU->Regs [ ( EVOL_L - SPU_X ) >> 1 ] = (u16) Data;
				pCoreRegs->EVOL_L = Data;
				
#ifdef VERBOSE_REVERSE_PHASE
				if ( pCoreRegs->EVOL_L & 0x8000 )
				{
					cout << "\nhps1x64: SPU: ALERT: EVOL_L is negative.\n";
				}
#endif

				break;
				
			case EVOL_R:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_EVOL_R
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)EVOL_R=" << _SPU->Regs [ ( EVOL_R - SPU_X ) >> 1 ];
#endif
			
				////////////////////////////////////////
				// Effect Volume Right
				//_SPU->Regs [ ( EVOL_R - SPU_X ) >> 1 ] = (u16) Data;
				pCoreRegs->EVOL_R = Data;
				
#ifdef VERBOSE_REVERSE_PHASE
				if ( pCoreRegs->EVOL_R & 0x8000 )
				{
					cout << "\nhps1x64: SPU: ALERT: EVOL_R is negative.\n";
				}
#endif

				break;
				
			case KON_0:
#if defined INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "\r\nKON_0; Data=" << hex << setw ( 4 ) << Data << " (Before) KON_0=" << _SPU->Regs [ ( ( KON_0 - SPU_X ) >> 1 ) ] << " KOFF_0=" << _SPU->Regs [ ( KOFF_0 - SPU_X ) >> 1 ] << " CON_0=" << _SPU->Regs [ ( CON_0 - SPU_X ) >> 1 ];
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_KON_0
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)KON_0=" << _SPU->Regs [ ( KON_0 - SPU_X ) >> 1 ];
#endif
			
				/////////////////////////////////////////////
				// Key On 0-15


				// store to SPU register
				// just set value
				//_SPU->Regs [ ( KON_0 - SPU_X ) >> 1 ] = (u16) Data;
				pCoreRegs->KON_0 = Data;
				
				// clear channel in key off
				// don't touch this here
				//_SPU->Regs [ ( KOFF_0 - SPU_X ) >> 1 ] &= ~((u16) Data);
				
				// logical Or with channel on/off register
				// unknown if channel on/off register is inverted or not
				// actually this has nothing to do with a channel being on or anything like that, so clear the bits
				//_SPU->Regs [ ( CON_0 - SPU_X ) >> 1 ] &= ~( (u16) Data );
				pCoreRegs->CON_0 &= ~( (u16) Data );
				

#ifdef KEY_ON_OFF_LOOP2
				while ( Data )
				{
					Channel = Data & ( -Data );
					Data ^= Channel;
					//Channel = CountTrailingZeros16 ( Channel );
					//Channel = __builtin_ctz( Channel );
					Channel = ctz32(Channel);

					_SPU->Start_SampleDecoding ( Channel );
				}
#else
				// when keyed on set channel ADSR to attack mode
				for ( Channel = 0; Channel < 16; Channel++ )
				{
					if ( ( 1 << Channel ) & Data )
					{
						_SPU->Start_SampleDecoding ( Channel );
					}
				}
#endif
				
				
#if defined INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "; (After) KON_0=" << hex << setw ( 4 ) << _SPU->Regs [ ( ( KON_0 - SPU_X ) >> 1 ) ] << " KOFF_0=" << _SPU->Regs [ ( KOFF_0 - SPU_X ) >> 1 ] << " CON_0=" << _SPU->Regs [ ( CON_0 - SPU_X ) >> 1 ];
#endif
				break;
				
			case KON_1:
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "\r\nKON_1; Data=" << hex << setw ( 4 ) << Data << " (Before) KON_1=" << _SPU->Regs [ ( ( KON_1 - SPU_X ) >> 1 ) ] << " KOFF_1=" << _SPU->Regs [ ( KOFF_1 - SPU_X ) >> 1 ] << " CON_1=" << _SPU->Regs [ ( CON_1 - SPU_X ) >> 1 ];
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_KON_1
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)KON_1=" << _SPU->Regs [ ( KON_1 - SPU_X ) >> 1 ];
#endif
				/////////////////////////////////////////////
				// Key on 16-23
				
				////////////////////////////////////////////////////////////////////////
				// upper 8 bits of register are zero and ignored
				Data &= 0xff;
				
				// store to SPU register
				// just set value
				//_SPU->Regs [ ( KON_1 - SPU_X ) >> 1 ] = (u16) Data;
				pCoreRegs->KON_1 = Data;

				// logical Or with channel on/off register
				// this isn't channel on/off.. clear the bits
				//_SPU->Regs [ ( CON_1 - SPU_X ) >> 1 ] &= ~( (u16) Data );
				pCoreRegs->CON_1 &= ~( (u16) Data );
				
#ifdef KEY_ON_OFF_LOOP2
				while ( Data )
				{
					Channel = Data & ( -Data );
					Data ^= Channel;
					//Channel = 16 + CountTrailingZeros16 ( Channel );
					//Channel = 16 + __builtin_ctz( Channel );
					Channel = 16 + ctz32(Channel);

					_SPU->Start_SampleDecoding ( Channel );
				}
#else
				// on key on we need to change ADSR mode to attack mode
				for ( Channel = 16; Channel < 24; Channel++ )
				{
					if ( ( 1 << ( Channel - 16 ) ) & Data )
					{
						_SPU->Start_SampleDecoding ( Channel );
					}
				}
#endif
				
				
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "; (After) KON_1=" << hex << setw ( 4 ) << _SPU->Regs [ ( ( KON_1 - SPU_X ) >> 1 ) ] << " KOFF_1=" << _SPU->Regs [ ( KOFF_1 - SPU_X ) >> 1 ] << " CON_1=" << _SPU->Regs [ ( CON_1 - SPU_X ) >> 1 ];
#endif
				break;
			
				
				
			
				
			case KOFF_0:
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "\r\nKOFF_0; Data=" << hex << setw ( 4 ) << Data << "(Before) KON_0=" << _SPU->Regs [ ( ( KON_0 - SPU_X ) >> 1 ) ] << " KOFF_0=" << _SPU->Regs [ ( KOFF_0 - SPU_X ) >> 1 ] << " CON_0=" << _SPU->Regs [ ( CON_0 - SPU_X ) >> 1 ];
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_KOFF_0
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)KOFF_0=" << _SPU->Regs [ ( KOFF_0 - SPU_X ) >> 1 ];
#endif
			
				/////////////////////////////////////////////
				// Key off 0-15
				
				// store to SPU register
				// just write value
				//_SPU->Regs [ ( KOFF_0 - SPU_X ) >> 1 ] |= (u16) Data;
				//_SPU->Regs [ ( KOFF_0 - SPU_X ) >> 1 ] = (u16) Data;
				pCoreRegs->KOFF_0 = Data;
				
				
#ifdef KEY_ON_OFF_LOOP2
				while ( Data )
				{
					Channel = Data & ( -Data );
					Data ^= Channel;
					//Channel = CountTrailingZeros16 ( Channel );
					//Channel = __builtin_ctz( Channel );
					Channel = ctz32(Channel);

					if ( ( _SPU->CycleCount - _SPU->StartCycle_Channel [ Channel ] ) >= c_iKeyOffT )
					{

						// put channel in adsr release phase unconditionally
						_SPU->ADSR_Status [ Channel ] = ADSR_RELEASE;
						
						// start envelope for release mode
						ModeRate = _SPU->Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
						Start_VolumeEnvelope ( _SPU->VOL_ADSR_Value [ Channel ], _SPU->Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
						
						// loop address not manually specified
						// can still change loop address after sound starts
						//_SPU->LSA_Manual_Bitmap &= ~( 1 << Channel );
					}
#ifdef VERBOSE_KEYONOFF_TEST
					else
					{
						// check if the key-off signal was given within 2T or 3T
						cout << "\nhps1x64: SPU2: ALERT: Channel#" << dec << Channel << " key-off after " << ( _SPU->CycleCount - _SPU->StartCycle_Channel [ Channel ] ) << "T";
					}
#endif

				}
#else
				// on key off we need to change ADSR mode to release mode
				for ( Channel = 0; Channel < 16; Channel++ )
				{
					if ( ( 1 << Channel ) & Data )
					{
						if ( ( _SPU->CycleCount - _SPU->StartCycle_Channel [ Channel ] ) >= c_iKeyOffT )
						{

							// put channel in adsr release phase unconditionally
							_SPU->ADSR_Status [ Channel ] = ADSR_RELEASE;
							
							// start envelope for release mode
							ModeRate = _SPU->Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
							Start_VolumeEnvelope ( _SPU->VOL_ADSR_Value [ Channel ], _SPU->Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
							
							// loop address not manually specified
							// can still change loop address after sound starts
							//_SPU->LSA_Manual_Bitmap &= ~( 1 << Channel );
						}
						else
						{
#ifdef VERBOSE_KEYONOFF_TEST
							// check if the key-off signal was given within 2T or 3T
							cout << "\nhps1x64: SPU2: ALERT: Channel#" << dec << Channel << " key-off after " << ( _SPU->CycleCount - _SPU->StartCycle_Channel [ Channel ] ) << "T";
#endif
						}
					}
				}
#endif
				
				
				// clear channel in key on
				// note: don't touch key on
				//_SPU->Regs [ ( KON_0 - SPU_X ) >> 1 ] &= ~((u16) Data);

				// temp: for now, also make sure channel is on when set to key off
				//_SPU->Regs [ ( CON_0 - SPU_X ) >> 1 ] |= (u16) Data;
				
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "; (After) KON_0=" << hex << setw ( 4 ) << _SPU->Regs [ ( ( KON_0 - SPU_X ) >> 1 ) ] << " KOFF_0=" << _SPU->Regs [ ( KOFF_0 - SPU_X ) >> 1 ] << "CON_0=" << _SPU->Regs [ ( CON_0 - SPU_X ) >> 1 ];
#endif
				
				break;
				
			case KOFF_1:
#if defined INLINE_DEBUG_WRITE_CHANNELONOFF || defined INLINE_DEBUG_WRITE
			debug << "\r\nKOFF_1; Data=" << hex << setw ( 4 ) << Data << " (Before) KON_1=" << _SPU->Regs [ ( ( KON_1 - SPU_X ) >> 1 ) ] << " KOFF_1=" << _SPU->Regs [ ( KOFF_1 - SPU_X ) >> 1 ] << " CON_1=" << _SPU->Regs [ ( CON_1 - SPU_X ) >> 1 ];
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_KOFF_1
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)KOFF_1=" << _SPU->Regs [ ( KOFF_1 - SPU_X ) >> 1 ];
#endif
			
				/////////////////////////////////////////////
				// Key off 16-23

				////////////////////////////////////////////////////////////////////////
				// upper 8 bits of register are zero and ignored
				Data &= 0xff;
				
				
				// store to SPU register (upper 24 bits of register are zero)
				// just write value
				//_SPU->Regs [ ( KOFF_1 - SPU_X ) >> 1 ] = (u16) Data;
				pCoreRegs->KOFF_1 = Data;
				
				
#ifdef KEY_ON_OFF_LOOP2
				while ( Data )
				{
					Channel = Data & ( -Data );
					Data ^= Channel;
					//Channel = 16 + CountTrailingZeros16 ( Channel );
					//Channel = 16 + __builtin_ctz( Channel );
					Channel = 16 + ctz32(Channel);

					if ( ( _SPU->CycleCount - _SPU->StartCycle_Channel [ Channel ] ) >= c_iKeyOffT )
					{

						// put channel in adsr release phase unconditionally
						_SPU->ADSR_Status [ Channel ] = ADSR_RELEASE;
						
						// start envelope for release mode
						ModeRate = _SPU->Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
						Start_VolumeEnvelope ( _SPU->VOL_ADSR_Value [ Channel ], _SPU->Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
						
						// loop address not manually specified
						// can still change loop address after sound starts
						//_SPU->LSA_Manual_Bitmap &= ~( 1 << Channel );
					}
#ifdef VERBOSE_KEYONOFF_TEST
					else
					{
						// check if the key-off signal was given within 2T or 3T
						cout << "\nhps1x64: SPU2: ALERT: Channel#" << dec << Channel << " key-off after " << ( _SPU->CycleCount - _SPU->StartCycle_Channel [ Channel ] ) << "T";
					}
#endif

				}
#else
				// on key off we need to change ADSR mode to release mode
				for ( Channel = 16; Channel < 24; Channel++ )
				{
					if ( ( 1 << ( Channel - 16 ) ) & Data )
					{
						if ( ( _SPU->CycleCount - _SPU->StartCycle_Channel [ Channel ] ) >= c_iKeyOffT )
						{

							// put channel in adsr release phase unconditionally
							_SPU->ADSR_Status [ Channel ] = ADSR_RELEASE;
							
							// start envelope for release mode
							ModeRate = _SPU->Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
							Start_VolumeEnvelope ( _SPU->VOL_ADSR_Value [ Channel ], _SPU->Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
							
							// loop address not manually specified
							// can still change loop address after sound starts
							//_SPU->LSA_Manual_Bitmap &= ~( 1 << Channel );
							
						}
						else
						{
#ifdef VERBOSE_KEYONOFF_TEST
							// check if the key-off signal was given within 2T or 3T
							cout << "\nhps1x64: SPU2: ALERT: Channel#" << dec << Channel << " key-off after " << ( _SPU->CycleCount - _SPU->StartCycle_Channel [ Channel ] ) << "T";
#endif
						}
					}
				}
#endif
				

#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "; (After) KON_1=" << hex << setw ( 4 ) << _SPU->Regs [ ( ( KON_1 - SPU_X ) >> 1 ) ] << " KOFF_1=" << _SPU->Regs [ ( KOFF_1 - SPU_X ) >> 1 ] << " CON_1=" << _SPU->Regs [ ( CON_1 - SPU_X ) >> 1 ];
#endif
				break;
			
				
			case CON_0:
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "\r\nCON_0; Data=" << hex << setw ( 4 ) << Data << "(Before) KON_0=" << _SPU->Regs [ ( ( KON_0 - SPU_X ) >> 1 ) ] << " KOFF_0=" << _SPU->Regs [ ( KOFF_0 - SPU_X ) >> 1 ] << " CON_0=" << _SPU->Regs [ ( CON_0 - SPU_X ) >> 1 ];
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CON_0
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)CON_0=" << _SPU->Regs [ ( CON_0 - SPU_X ) >> 1 ];
#endif

				///////////////////////////////////////////////////////
				// *todo* this register does not get written to
				
				// logical Or with channel on/off register
				// more of a read-only register. modifying it only changes the value momentarily, then it gets set back
				// should set register to zero
				//_SPU->Regs [ ( CON_0 - SPU_X ) >> 1 ] = 0;	//(u16) Data;
				pCoreRegs->CON_0 = 0;
				
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "; (After) KON_0=" << hex << setw ( 4 ) << _SPU->Regs [ ( ( KON_0 - SPU_X ) >> 1 ) ] << " KOFF_0=" << _SPU->Regs [ ( KOFF_0 - SPU_X ) >> 1 ] << " CON_0=" << _SPU->Regs [ ( CON_0 - SPU_X ) >> 1 ];
#endif
				break;
			
			case CON_1:
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "\r\nCON_1; Data=" << hex << setw ( 4 ) << Data << " (Before) KON_1=" << _SPU->Regs [ ( ( KON_1 - SPU_X ) >> 1 ) ] << " KOFF_1=" << _SPU->Regs [ ( KOFF_1 - SPU_X ) >> 1 ] << " CON_1=" << _SPU->Regs [ ( CON_1 - SPU_X ) >> 1 ];
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CON_1
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)CON_1=" << _SPU->Regs [ ( CON_1 - SPU_X ) >> 1 ];
#endif

				///////////////////////////////////////////////////////
				// *todo* this register does not get written to
			
				////////////////////////////////////////////////////////////////////////
				// upper 8 bits of register are zero and ignored
				//Data &= 0xff;
				
				// logical Or with channel on/off register
				// more of a read-only register. modifying it only changes the value momentarily, then it gets set back
				// should set register to zero
				//_SPU->Regs [ ( CON_1 - SPU_X ) >> 1 ] = 0;	//(u16) Data;
				pCoreRegs->CON_1 = 0;
				
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "; (After) KON_1=" << hex << setw ( 4 ) << _SPU->Regs [ ( ( KON_1 - SPU_X ) >> 1 ) ] << " KOFF_1=" << _SPU->Regs [ ( KOFF_1 - SPU_X ) >> 1 ] << " CON_1=" << _SPU->Regs [ ( CON_1 - SPU_X ) >> 1 ];
#endif
				break;
				
			case 0x1f801db8:
			
				// current master volume left //
				// ***todo*** this looks wrong
				//_SPU->MVOL_L_Value = ((s32) ((s16) Data)) << 16;
				
				//_SPU->Regs [ ( 0x1f801db8 - SPU_X ) >> 1 ] = Data;
				pCoreRegs->CMVOL_L = Data;
				
#ifdef VERBOSE_REVERSE_PHASE
					// need to know if volume is negative for now
					if ( pCoreRegs->CMVOL_L & 0x8000 )
					{
						cout << "\nhps1x64: SPU: ALERT: CMVOL_L is negative.\n";
					}
#endif

				break;
				
			case 0x1f801dba:
			
				// current master volume right //
				// ***todo*** this looks wrong
				//_SPU->MVOL_R_Value = ((s32) ((s16) Data)) << 16;
				
				//_SPU->Regs [ ( 0x1f801dba - SPU_X ) >> 1 ] = Data;
				pCoreRegs->CMVOL_R = Data;
			
#ifdef VERBOSE_REVERSE_PHASE
					// need to know if volume is negative for now
					if ( pCoreRegs->CMVOL_R & 0x8000 )
					{
						cout << "\nhps1x64: SPU: ALERT: CMVOL_R is negative.\n";
					}
#endif

				break;
				
			default:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_DEFAULT
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)" << _SPU->RegisterNames [ ( Address - SPU_X ) >> 1 ] << "=" << _SPU->Regs [ ( Address - SPU_X ) >> 1 ];
#endif

				// *** TODO *** write to current volume if those registers are modified
				if ( Address >= 0x1f801e00 )
				{
					// get the channel and l/r
					Ch = ( Address & 0xff ) >> 1;
					
					// write to volume value
					if ( Ch & 1 )
					{
						// modify R volume //
						_SPU->VOL_R_Value [ Ch >> 1 ] = ((u32) Data) << 16;
					}
					else
					{
						// modify L volume //
						_SPU->VOL_L_Value [ Ch >> 1 ] = ((u32) Data) << 16;
					}
					
#ifdef VERBOSE_REVERSE_PHASE
					// need to know if volume is negative for now
					if ( _SPU->Regs [ ( Address - SPU_X ) >> 1 ] & 0x8000 )
					{
						cout << "\nhps1x64: SPU: ALERT: CVOL_L/R is negative. Address=" << hex << Address << " Value=" << Data << " Channel#" << dec << ( Ch >> 1 ) << "\n";

					}
#endif

				}

				/////////////////////////////////////////////
				// by default just store to SPU regs
				_SPU->Regs [ ( Address - SPU_X ) >> 1 ] = (u16) Data;
				
				break;
		}
	}
}


u64 SPU::DMA_ReadyForRead ( void )
{
	return 1;
}


u64 SPU::DMA_ReadyForWrite ( void )
{
	return 1;
}


//void SPU::DMA_ReadBlock ( u32* Data, u32 BS )
u32 SPU::DMA_ReadBlock ( u32* pMemory, u32 Address, u32 BS )
{
	u32 *Data;
	
	Data = & ( pMemory [ Address >> 2 ] );
	
	// TODO: check for interrupt (should interrupt on transfers TO and FROM spu ram //
	
	for ( int i = 0; i < BS; i++ )
	{
		Data [ i ] = ((u32*)_SPU->RAM) [ ( ( _SPU->NextSoundBufferAddress & c_iRam_Mask ) >> 2 ) ];

		// check if address is set to trigger interrupt
		//if ( ( NextSoundBufferAddress == ( ( (u32) Regs [ ( IRQA - SPU_X ) >> 1 ] ) << 3 ) ) && ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] & 0x40 ) )
		if ( ( _SPU->NextSoundBufferAddress == ( ( (u32) pCoreRegs->IRQA ) << 3 ) ) && ( pCoreRegs->CTRL & 0x40 ) )
		{
			// we have reached irq address - trigger interrupt
			SetInterrupt ();
			
			// interrupt
			//Regs [ ( 0x1f801dae - SPU_X ) >> 1 ] |= 0x40;
			pCoreRegs->STAT |= 0x40;
		}
		
		//////////////////////////////////////////////////////////
		// update next sound buffer address
		_SPU->NextSoundBufferAddress += 4;
		_SPU->NextSoundBufferAddress &= c_iRam_Mask;
	}
	
	// return the amount transferred
	return BS;
}




u32 SPU::DMA_WriteBlock ( u32* pMemory, u32 Address, u32 BS )
{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "\r\nDMA_Write: ";
	debug << " Cycle#" << dec << *_DebugCycleCount;
	debug << " (before) NextSoundBufferAddress=" << hex << _SPU->NextSoundBufferAddress << " BS=" << BS;
#endif

	u32 *Data;
	
	Data = & ( pMemory [ Address >> 2 ] );
	
	// TODO: check for interrupt (should interrupt on transfers TO and FROM spu ram //
	
	//for ( int i = 0; i < 32; i++ )
	for ( int i = 0; i < (BS << 1); i++ )
	{
		// write the data into sound RAM
		_SPU->RAM [ ( ( _SPU->NextSoundBufferAddress + ( i << 1 ) ) & c_iRam_Mask ) >> 1 ] = ((u16*) Data) [ i ];
		
#ifdef INLINE_DEBUG_DMA_WRITE_RECORD
	debug << " " << ((u16*) Data) [ i ];
#endif

		// check for interrupt
		// check if address is set to trigger interrupt
		//if ( ( ( ( NextSoundBufferAddress + ( i << 1 ) ) & c_iRam_Mask ) == ( ( (u32) Regs [ ( IRQA - SPU_X ) >> 1 ] ) << 3 ) ) && ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] & 0x40 ) )
		if ( ( ( ( _SPU->NextSoundBufferAddress + ( i << 1 ) ) & c_iRam_Mask ) == ( ( (u32) pCoreRegs->IRQA ) << 3 ) ) && ( pCoreRegs->CTRL & 0x40 ) )
		{
			// we have reached irq address - trigger interrupt
			SetInterrupt ();
			
			// interrupt
			//Regs [ ( 0x1f801dae - SPU_X ) >> 1 ] |= 0x40;
			pCoreRegs->STAT |= 0x40;
		}
	}
	
	//////////////////////////////////////////////////
	// reset buffer index
	_SPU->BufferIndex = 0;

	//////////////////////////////////////////////////////////
	// update next sound buffer address
	//NextSoundBufferAddress += 64;
	_SPU->NextSoundBufferAddress += ( BS << 2 );
	_SPU->NextSoundBufferAddress &= c_iRam_Mask;

	////////////////////////////////////////////////////////////
	// save back into the sound buffer address register
	// this value does not change with transfer
	//Regs [ ( REG_SoundBufferAddress - SPU_X ) >> 1 ] = NextSoundBufferAddress >> 3;
	
	// *** testing ***
	_SPU->SpuTransfer_Complete ();

#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "; (after) NextSoundBufferAddress=" << _SPU->NextSoundBufferAddress;
#endif

	// return the amount transferred
	return BS;
}



void SPU::RunNoiseGenerator ()
{
	u32 NoiseStep, NoiseShift, ParityBit;
	
	//NoiseStep = ( ( Regs [ ( CTRL - SPU_X ) >> 1 ] >> 8 ) & 0x3 ) + 4;
	//NoiseShift = ( ( Regs [ ( CTRL - SPU_X ) >> 1 ] >> 10 ) & 0xf );
	NoiseStep = ( ( pCoreRegs->CTRL >> 8 ) & 0x3 ) + 4;
	NoiseShift = ( ( pCoreRegs->CTRL >> 10 ) & 0xf );
	
	Timer -= NoiseStep;
	ParityBit = ( ( NoiseLevel >> 15 ) ^ ( NoiseLevel >> 12 ) ^ ( NoiseLevel >> 11 ) ^ ( NoiseLevel >> 10 ) ^ 1 ) & 1;
	if ( Timer < 0 ) NoiseLevel = ( NoiseLevel << 1 ) + ParityBit;
	if ( Timer < 0 ) Timer += ( 0x20000 >> NoiseShift );
	if ( Timer < 0 ) Timer += ( 0x20000 >> NoiseShift );
}


void SPU::SetSweepVars ( u16 flags, u32 Channel, s32* Rates, s32* Rates75 )
{
	u32 Rate;
	
	// also set constants if volume is sweeping
	if ( flags >> 15 )
	{
		Rate = ( flags & 0x7f );

		// writing sweep value - check mode
		switch ( ( flags >> 13 ) & 0x3 )
		{
			case 0:
			
				//////////////////////////////////////////
				// writing Linear increment mode

				// to keep this simple this needs to be an inline function - just sets correct constants for linear increment mode
				Set_LinearIncRates ( Channel, Rate, Rates, Rates75 );
				
				break;
				
			case 1:
			
				//////////////////////////////////////////
				// writing Linear decrement mode
				
				Set_LinearDecRates ( Channel, Rate, Rates, Rates75 );
				
				break;
				
			case 2:
			
				///////////////////////////////////////////////////////////////
				// writing psx pseudo inverse exponential increment mode
				
				Set_ExponentialIncRates ( Channel, Rate, Rates, Rates75 );
				
				break;
				
			case 3:
				
				///////////////////////////////////////////////////////////////
				// writing exponential decrement mode
				
				Set_ExponentialDecRates ( Channel, Rate, Rates, Rates75 );
				
				break;
				
		}
	}
}


void SPU::SweepVolume ( u16 flags, s64& CurrentVolume, u32 VolConstant, u32 VolConstant75 )
{
	u64 Temp;
	
	//////////////////////////////////////////////
	// check if volume should sweep for VOL_L
	if ( flags >> 15 )
	{
//#ifdef INLINE_DEBUG_RUN
//	debug << "; Sweep";
//#endif

		////////////////////////////////////////////////////
		// Volume should sweep
		
		// check if linear sweep or exponential sweep for L
		if ( ( ( flags >> 13 ) & 3 ) != 3 )
		{
//#ifdef INLINE_DEBUG_RUN
//	debug << "; Linear/Pseudo";
//#endif

			////////////////////////////////////////////////////
			// linear or psx pseudo exponential sweep
			
			// check if current volume < 0x6000
			if ( CurrentVolume < ( 0x6000 << 16 ) )
			{
				CurrentVolume += VolConstant;
			}
			else
			{
				CurrentVolume += VolConstant75;
			}
		}
		else
		{
//#ifdef INLINE_DEBUG_RUN
//	debug << "; Exponential";
//#endif

			////////////////////////////////////////////////////
			// exponential decrement sweep
			Temp = (u64) (CurrentVolume);
			CurrentVolume = (u32) ( ( ( Temp >> 15 ) * VolConstant ) >> 15 );
		}
		
		// clamp volume
		if ( CurrentVolume > ( 32767 << 16 ) )
		{
			CurrentVolume = ( 32767 << 16 );
		}
		
		if ( CurrentVolume < 0 )
		{
			CurrentVolume = 0;
		}
		
	}
	else
	{
//#ifdef INLINE_DEBUG_RUN
//	debug << "; Constant";
//#endif

		///////////////////////////////////////////
		// Volume is constant
		CurrentVolume = ( (u32) flags ) << 17;
		
		// sign extend to 64-bits
		CurrentVolume = ( CurrentVolume << 32 ) >> 32;
	}
}


s64 SPU::Get_VolumeStep ( s64& Level, u32& Cycles, u32 Value, u32 flags )
{
	s32 ShiftValue, StepValue;
	s64 Step;
	
	ShiftValue = ( Value >> 2 ) & 0xf;
	StepValue = Value & 0x3;
	
	// check if increase or decrease
	if ( ! ( flags & 0x1 ) )
	{
		// increase //
		//StepValue = StepValues_Inc [ StepValue ];
		StepValue = 7 - StepValue;
	}
	else
	{
		// decrease //
		//StepValue = StepValues_Dec [ StepValue ];
		StepValue -= 8;
	}
	
	Cycles = 1 << ( ( ( ShiftValue - 11 ) < 0 ) ? 0 : ( ShiftValue - 11 ) );
	Step = StepValue << ( ( ( 11 - ShiftValue ) < 0 ) ? 0 : ( 11 - ShiftValue ) );
	
	// check if exponential AND increase
	if ( ( ( flags & 0x3 ) == 2 ) && ( Level > 0x6000 ) )
	{
		Cycles <<= 2;
	}
	
	// check if exponential AND decrease
	if ( ( flags & 0x3 ) == 3 )
	{
		Step = ( Step * Level ) >> 15;
	}
	
	return Step;
}


void SPU::Start_VolumeEnvelope ( s64& Level, u32& Cycles, u32 Value, u32 flags, bool InitLevel )
{
	//static const s32 StepValues_Inc [] = { 7, 6, 5, 4 };
	//static const s32 StepValues_Dec [] = { -8, -7, -6, -5 };
	
	s32 ShiftValue, StepValue;
	s64 Step;
	
	//ShiftValue = ( Value >> 2 ) & 0xf;
	ShiftValue = ( Value >> 2 ) & 0x1f;
	StepValue = Value & 0x3;
	
	// check if increase or decrease
	if ( ! ( flags & 0x1 ) )
	{
		// increase //
		//StepValue = StepValues_Inc [ StepValue ];
		StepValue = 7 - StepValue;
	}
	else
	{
		// decrease //
		//StepValue = StepValues_Dec [ StepValue ];
		StepValue -= 8;
	}
	
	Cycles = 1 << ( ( ( ShiftValue - 11 ) < 0 ) ? 0 : ( ShiftValue - 11 ) );
	Step = StepValue << ( ( ( 11 - ShiftValue ) < 0 ) ? 0 : ( 11 - ShiftValue ) );
	
	
	// check if exponential AND decrease
	if ( ( flags & 0x3 ) == 3 )
	{
		Step = ( Step * Level ) >> 15;
		
		if ( !Step ) Step = -1;
	}
	

	// check if exponential AND increase
	// note: this needs to go last because it is checking the result of the "Level" variable
	if ( ( ( flags & 0x3 ) == 2 ) && ( Level > 0x6000 ) )
	{
		Cycles <<= 2;
	}

	if ( ( InitLevel ) || ( Cycles == 1 ) )
	{
		Level += Step;
	}
	
	// ***TESTING*** initial cycles = 1 ??
	//Cycles = 1;
}

// probably should start cycles at 1 and then let it set it later
// does not saturate Level since it can go to -1 for a cycle
void SPU::VolumeEnvelope ( s64& Level, u32& Cycles, u32 Value, u32 flags, bool clamp )
{
	//static const s32 StepValues_Inc [] = { 7, 6, 5, 4 };
	//static const s32 StepValues_Dec [] = { -8, -7, -6, -5 };
	
	s32 ShiftValue, StepValue;
	s32 Step;
	
	Cycles--;
	
	if ( Cycles ) return;
	
	ShiftValue = ( Value >> 2 ) & 0x1f;
	StepValue = Value & 0x3;
	
#ifdef INLINE_DEBUG_ENVELOPE
	debug << "; ShiftValue=" << hex << ShiftValue << "; StepValue=" << StepValue;
#endif

	// check if increase or decrease
	if ( ! ( flags & 0x1 ) )
	{
		// increase //
		//StepValue = StepValues_Inc [ StepValue ];
		StepValue = 7 - StepValue;
	}
	else
	{
		// decrease //
		//StepValue = StepValues_Dec [ StepValue ];
		StepValue -= 8;
	}
	
#ifdef INLINE_DEBUG_ENVELOPE
	debug << "; StepValue=" << StepValue;
#endif

	Cycles = 1 << ( ( ( ShiftValue - 11 ) < 0 ) ? 0 : ( ShiftValue - 11 ) );
	Step = StepValue << ( ( ( 11 - ShiftValue ) < 0 ) ? 0 : ( 11 - ShiftValue ) );
	
#ifdef INLINE_DEBUG_ENVELOPE
	debug << "; Cycles=" << dec << Cycles;
#endif

	
	// check if exponential AND decrease
	if ( ( flags & 0x3 ) == 3 )
	{
		Step = ( Step * Level ) >> 15;
		
		if ( !Step ) Step = -1;
	}
	
#ifdef INLINE_DEBUG_ENVELOPE
	debug << "; Step=" << hex << Step << "; (before) Level=" << Level;
#endif

	Level += Step;
	
	if ( clamp )
	{
	// clamp level to signed 16-bits
	//if ( Level > 0x7fffLL ) { Level = 0x7fffLL; } else if ( Level < -0x8000LL ) { Level = -0x8000LL; }
	if ( Level > 0x7fffLL ) { Level = 0x7fffLL; } else if ( Level < 0 ) { Level = 0; }
	}

	// check if exponential AND increase
	// note: this needs to go last because it is checking the result of the "Level" variable
	if ( ( ( flags & 0x3 ) == 2 ) && ( Level > 0x6000 ) )
	{
		Cycles <<= 2;
	}

#ifdef INLINE_DEBUG_ENVELOPE
	debug << "; (after) Level=" << Level;
#endif
}



void SPU::ProcessReverbR ( s32 RightInput )
{
	s32 Rin, Rout;
	s32 t_mLDIFF, t_mRDIFF;
	s32 t_mRSAME, t_mRAPF1, t_mRAPF2;
	
	s32 s_dRSAME = ReadReverbBuffer ( ((u32) *_dRSAME) << 2 );
	s32 s_mRSAME = ReadReverbBuffer ( ( ((u32) *_mRSAME) << 2 ) - 1 );

	s32 s_dLDIFF = ReadReverbBuffer ( ((u32) *_dLDIFF) << 2 );
	
	s32 s_dRDIFF = ReadReverbBuffer ( ((u32) *_dRDIFF) << 2 );
	s32 s_mRDIFF = ReadReverbBuffer ( ( ((u32) *_mRDIFF) << 2 ) - 1 );

	s32 s_mRCOMB1 = ReadReverbBuffer ( ((u32) *_mRCOMB1) << 2 );
	s32 s_mRCOMB2 = ReadReverbBuffer ( ((u32) *_mRCOMB2) << 2 );
	s32 s_mRCOMB3 = ReadReverbBuffer ( ((u32) *_mRCOMB3) << 2 );
	s32 s_mRCOMB4 = ReadReverbBuffer ( ((u32) *_mRCOMB4) << 2 );
	
	s32 s_mRAPF1 = ReadReverbBuffer ( ((u32) *_mRAPF1) << 2 );
	s32 s_mRAPF1_dAPF1 = ReadReverbBuffer ( ( ((u32) *_mRAPF1) - ((u32) *_dAPF1) ) << 2 );
	
	s32 s_mRAPF2 = ReadReverbBuffer ( ((u32) *_mRAPF2) << 2 );
	s32 s_mRAPF2_dAPF2 = ReadReverbBuffer ( ( ((u32) *_mRAPF2) - ((u32) *_dAPF2) ) << 2 );
	
	// input from mixer //
	Rin = ( RightInput * ( (s32) *_vRIN ) ) >> 15;
	
	// same side reflection //
	//[mRSAME] = (Rin + [dRSAME]*vWALL - [mRSAME-2])*vIIR + [mRSAME-2]  ;R-to-R
	t_mRSAME = ( ( ( Rin + ( ( s_dRSAME * ( (s32) *_vWALL ) ) >> 15 ) - s_mRSAME ) * ( (s32) *_vIIR ) ) >> 15 ) + s_mRSAME;
	
	// Different Side Reflection //
	//[mRDIFF] = (Rin + [dLDIFF]*vWALL - [mRDIFF-2])*vIIR + [mRDIFF-2]  ;L-to-R
	t_mRDIFF = ( ( ( Rin + ( ( s_dLDIFF * ( (s32) *_vWALL ) ) >> 15 ) - s_mRDIFF ) * ( (s32) *_vIIR ) ) >> 15 ) + s_mRDIFF;
	
	// Early Echo (Comb Filter, with input from buffer) //
	//Rout=vCOMB1*[mRCOMB1]+vCOMB2*[mRCOMB2]+vCOMB3*[mRCOMB3]+vCOMB4*[mRCOMB4]
	Rout = ( ( ( (s32) *_vCOMB1 ) * s_mRCOMB1 ) + ( ( (s32) *_vCOMB2 ) * s_mRCOMB2 ) + ( ( (s32) *_vCOMB3 ) * s_mRCOMB3 ) + ( ( (s32) *_vCOMB4 ) * s_mRCOMB4 ) ) >> 15;
	
	// Late Reverb APF1 (All Pass Filter 1, with input from COMB) //
	//[mRAPF1]=Rout-vAPF1*[mRAPF1-dAPF1], Rout=[mRAPF1-dAPF1]+[mRAPF1]*vAPF1
	t_mRAPF1 = Rout - ( ( ( (s32) *_vAPF1 ) * s_mRAPF1_dAPF1 ) >> 15 );
	Rout = s_mRAPF1_dAPF1 + ( (t_mRAPF1 * ( (s32) *_vAPF1 ) ) >> 15 );
	
	// Late Reverb APF2 (All Pass Filter 2, with input from APF1) //
	// [mRAPF2]=Rout-vAPF2*[mRAPF2-dAPF2], Rout=[mRAPF2-dAPF2]+[mRAPF2]*vAPF2
	t_mRAPF2 = Rout - ( ( ( (s32) *_vAPF2 ) * s_mRAPF2_dAPF2 ) >> 15 );
	Rout = s_mRAPF2_dAPF2 + ( (t_mRAPF2 * ( (s32) *_vAPF2 ) ) >> 15 );
	
	// Output to Mixer (Output volume multiplied with input from APF2) //
	// RightOutput = Rout*vROUT
	ReverbR_Output = ( Rout * ( (s32) *_vROUT ) ) >> 15;
	
	// only write to the reverb buffer if reverb is enabled
	if ( REG ( CTRL ) & 0x80 )
	{
		WriteReverbBuffer ( ((u32) *_mRSAME) << 2, t_mRSAME );
		WriteReverbBuffer ( ((u32) *_mRDIFF) << 2, t_mRDIFF );
		WriteReverbBuffer ( ((u32) *_mRAPF1) << 2, t_mRAPF1 );
		WriteReverbBuffer ( ((u32) *_mRAPF2) << 2, t_mRAPF2 );
	}
	
	// update reverb buffer address
	// this should actually happen at 22050 hz unconditionally
	UpdateReverbBuffer ();
}


void SPU::ProcessReverbL ( s32 LeftInput )
{
	s32 Lin, Lout;
	
	// outputs
	s32 t_mLDIFF, t_mRDIFF;
	s32 t_mLSAME, t_mLAPF1, t_mLAPF2;
	
	// inputs
	s32 s_dLSAME = ReadReverbBuffer ( ((u32) *_dLSAME) << 2 );
	s32 s_mLSAME = ReadReverbBuffer ( ( ((u32) *_mLSAME) << 2 ) - 1 );
	
	s32 s_dLDIFF = ReadReverbBuffer ( ((u32) *_dLDIFF) << 2 );
	s32 s_mLDIFF = ReadReverbBuffer ( ( ((u32) *_mLDIFF) << 2 ) - 1 );
	
	s32 s_dRDIFF = ReadReverbBuffer ( ((u32) *_dRDIFF) << 2 );
	
	s32 s_mLCOMB1 = ReadReverbBuffer ( ((u32) *_mLCOMB1) << 2 );
	s32 s_mLCOMB2 = ReadReverbBuffer ( ((u32) *_mLCOMB2) << 2 );
	s32 s_mLCOMB3 = ReadReverbBuffer ( ((u32) *_mLCOMB3) << 2 );
	s32 s_mLCOMB4 = ReadReverbBuffer ( ((u32) *_mLCOMB4) << 2 );
	
	s32 s_mLAPF1 = ReadReverbBuffer ( ((u32) *_mLAPF1) << 2 );
	s32 s_mLAPF1_dAPF1 = ReadReverbBuffer ( ( ((u32) *_mLAPF1) - ((u32) *_dAPF1) ) << 2 );
	
	s32 s_mLAPF2 = ReadReverbBuffer ( ((u32) *_mLAPF2) << 2 );
	s32 s_mLAPF2_dAPF2 = ReadReverbBuffer ( ( ((u32) *_mLAPF2) - ((u32) *_dAPF2) ) << 2 );
	
	// input from mixer //
	Lin = ( LeftInput * ( (s32) *_vLIN ) ) >> 15;
	
	// same side reflection //
	//[mLSAME] = (Lin + [dLSAME]*vWALL - [mLSAME-2])*vIIR + [mLSAME-2]  ;L-to-L
	t_mLSAME = ( ( ( Lin + ( ( s_dLSAME * ( (s32) *_vWALL ) ) >> 15 ) - s_mLSAME ) * ( (s32) *_vIIR ) ) >> 15 ) + s_mLSAME;
	
	// Different Side Reflection //
	//[mLDIFF] = (Lin + [dRDIFF]*vWALL - [mLDIFF-2])*vIIR + [mLDIFF-2]  ;R-to-L
	t_mLDIFF = ( ( ( Lin + ( ( s_dRDIFF * ( (s32) *_vWALL ) ) >> 15 ) - s_mLDIFF ) * ( (s32) *_vIIR ) ) >> 15 ) + s_mLDIFF;
	
	// Early Echo (Comb Filter, with input from buffer) //
	//Lout=vCOMB1*[mLCOMB1]+vCOMB2*[mLCOMB2]+vCOMB3*[mLCOMB3]+vCOMB4*[mLCOMB4]
	Lout = ( ( (s32) *_vCOMB1 ) * s_mLCOMB1 + ( (s32) *_vCOMB2 ) * s_mLCOMB2 + ( (s32) *_vCOMB3 ) * s_mLCOMB3 + ( (s32) *_vCOMB4 ) * s_mLCOMB4 ) >> 15;
	
	// Late Reverb APF1 (All Pass Filter 1, with input from COMB) //
	//[mLAPF1]=Lout-vAPF1*[mLAPF1-dAPF1], Lout=[mLAPF1-dAPF1]+[mLAPF1]*vAPF1
	t_mLAPF1 = Lout - ( ( ( (s32) *_vAPF1 ) * s_mLAPF1_dAPF1 ) >> 15 );
	Lout = s_mLAPF1_dAPF1 + ( (t_mLAPF1 * ( (s32) *_vAPF1 ) ) >> 15 );
	
	// Late Reverb APF2 (All Pass Filter 2, with input from APF1) //
	// [mLAPF2]=Lout-vAPF2*[mLAPF2-dAPF2], Lout=[mLAPF2-dAPF2]+[mLAPF2]*vAPF2
	t_mLAPF2 = Lout - ( ( ( (s32) *_vAPF2 ) * s_mLAPF2_dAPF2 ) >> 15 );
	Lout = s_mLAPF2_dAPF2 + ( (t_mLAPF2 * ( (s32) *_vAPF2 ) ) >> 15 );
	
	// Output to Mixer (Output volume multiplied with input from APF2) //
	// LeftOutput = Lout*vLOUT
	ReverbL_Output = ( Lout * ( (s32) *_vLOUT ) ) >> 15;
	
	// only write to the reverb buffer if reverb is enabled
	if ( REG ( CTRL ) & 0x80 )
	{
		// write back to reverb buffer
		WriteReverbBuffer ( ((u32) *_mLSAME) << 2, t_mLSAME );
		WriteReverbBuffer ( ((u32) *_mLDIFF) << 2, t_mLDIFF );
		WriteReverbBuffer ( ((u32) *_mLAPF1) << 2, t_mLAPF1 );
		WriteReverbBuffer ( ((u32) *_mLAPF2) << 2, t_mLAPF2 );
	}
}



// gets address in reverb work area at offset address
s32 SPU::ReadReverbBuffer ( u32 Address )
{
#ifdef INLINE_DEBUG_READREVERB
	debug << "\r\nSPU::ReadReverbBuffer " << "Address=" << hex << Address;
#endif

	s16 Value;
	u32 BeforeAddress;

	// address will be coming straight from register
	// that won't work because of the offsets
	Address <<= 1;

	Address += Reverb_BufferAddress;
	
#ifdef INLINE_DEBUG_READREVERB
	debug << " " << hex << Address;
	if ( Address < ReverbWork_Start )
	{
		cout << "\nSPU::ReadReverbBuffer; (before) Address<ReverbWork_Start; Address=" << hex << Address << " ReverbWork_Start=" << ReverbWork_Start << dec << " ReverbWork_Size=" << ReverbWork_Size;
	}
	BeforeAddress = Address;
#endif


	if ( Address >= c_iRam_Size ) Address = ReverbWork_Start + ( Address & c_iRam_Mask );
	
	
#ifdef INLINE_DEBUG_READREVERB
	debug << " " << hex << Address;
	if ( Address < ReverbWork_Start )
	{
		cout << "\nSPU::ReadReverbBuffer; (after) Address<ReverbWork_Start; (before) Address=" << hex << BeforeAddress << " (after) Address=" << Address << " ReverbWork_Start=" << ReverbWork_Start << dec << " ReverbWork_Size=" << ReverbWork_Size;
	}
#endif

	Value = RAM [ Address >> 1 ];
	
#ifdef INLINE_DEBUG_READREVERB
	debug << " Value=" << hex << Value;
#endif

	// address is ready for use with shift right by 1
	return Value;
}

void SPU::WriteReverbBuffer ( u32 Address, s32 Value )
{
#ifdef INLINE_DEBUG_WRITEREVERB
	debug << "\r\nSPU::WriteReverbBuffer; Reverb_BufferAddress=" << hex << Reverb_BufferAddress << " Value=" << Value << " Address=" << Address;
#endif


	// address will be coming straight from register
	// that won't work because of the offsets
	Address <<= 1;
	
	Address += Reverb_BufferAddress;
	
#ifdef INLINE_DEBUG_WRITEREVERB
	u32 BeforeAddress;
	debug << " " << Address;
	if ( Address < ReverbWork_Start )
	{
		cout << "\nSPU::WriteReverbBuffer; (before) Address<ReverbWork_Start; Address=" << hex << Address << " ReverbWork_Start=" << ReverbWork_Start << dec << " ReverbWork_Size=" << ReverbWork_Size;
	}
	BeforeAddress = Address;
#endif

	
	if ( Address >= c_iRam_Size ) Address = ReverbWork_Start + ( Address & c_iRam_Mask );
	
	
#ifdef INLINE_DEBUG_WRITEREVERB
	debug << " " << Address;
	if ( Address < ReverbWork_Start )
	{
		cout << "\nSPU::WriteReverbBuffer; (after) Address<ReverbWork_Start; (before) Address=" << hex << BeforeAddress << " (after) Address=" << Address << " ReverbWork_Start=" << ReverbWork_Start << dec << " ReverbWork_Size=" << ReverbWork_Size;
	}
#endif

	// address is ready for use with shift right by 1
	RAM [ Address >> 1 ] = adpcm_decoder::clamp ( Value );
}

void SPU::UpdateReverbBuffer ()
{
#ifdef INLINE_DEBUG_UPDATEREVERB
	debug << "\r\nSPU::UpdateReverbBuffer " << " ReverbWork_Start=" << hex << ReverbWork_Start << " Reverb_BufferAddress=" << Reverb_BufferAddress;
#endif

	Reverb_BufferAddress += 2;
	if ( Reverb_BufferAddress >= c_iRam_Size ) Reverb_BufferAddress = ReverbWork_Start;
	
	// check if address in reverb buffer is set to trigger interrupt
	//if ( ( Reverb_BufferAddress == ( ( (u32) Regs [ ( IRQA - SPU_X ) >> 1 ] ) << 3 ) ) && ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] & 0x40 ) )
	if ( ( Reverb_BufferAddress == ( ( (u32) pCoreRegs->IRQA ) << 3 ) ) && ( pCoreRegs->CTRL & 0x40 ) )
	{
		// we have reached irq address - trigger interrupt
		SetInterrupt ();
		
		// interrupt
		//Regs [ ( 0x1f801dae - SPU_X ) >> 1 ] |= 0x40;
		pCoreRegs->STAT |= 0x40;
	}
	
#ifdef INLINE_DEBUG_UPDATEREVERB
	debug << " " << Reverb_BufferAddress;
#endif
}




void SPU::SpuTransfer_Complete ()
{
	//Regs [ ( STAT - SPU_X ) >> 1 ] &= ~( 0xf << 7 );
}


// Sample0 is the sample you are on, then Sample1 is previous sample, then Sample2 is the next previous sample, etc.
s32 SPU::Calc_sample_gx ( u32 SampleOffset_Fixed16, s32 Sample0, s32 Sample1, s32 Sample2, s32 Sample3 )
{
	u32 i;
	s32 Output;
	
	i = ( SampleOffset_Fixed16 >> 8 ) & 0xff;
	
	Output = ( ( Sample0 * ((s32) gx [ i ]) ) + ( Sample1 * ((s32) gx [0x100 + i]) ) + ( Sample2 * ((s32) gx [0x1ff - i]) ) + ( Sample3 * ((s32) gx [0xff - i]) ) ) >> 15;
	
	// this would have to set the "Output" variable to the result to do anything
	//Output = adpcm_decoder::clamp ( Output );
	
	return Output;
}


s32 SPU::Calc_sample_filter ( s32 x0, s32 x1, s32 x2, s32 y1, s32 y2 )
{
	s32 Output;
	
	Output = ( ( _b0 * x0 ) + ( _b1 * x1 ) + ( _b2 * x2 ) - ( _a1 * y1 ) - ( _a2 * y2 ) ) >> _N;
	
	return Output;
}



void SPU::Start_SampleDecoding ( u32 Channel )
{
	u32 ModeRate;
	
	ChRegs0_Layout *pChRegs0;
	//ChRegs1_Layout *pChRegs1;
	
	pChRegs0 = (ChRegs0_Layout*) ( & ( pCoreRegs->ChRegs0 [ Channel ] ) );
	
	// clear sample history for channel
	//History [ Channel ].Value0 = 0;
	//History [ Channel ].Value1 = 0;
	
	// clear filter buffer (buffer for filter for channel)
	FilterBuf [ ( Channel << 2 ) + 0 ] = 0;
	FilterBuf [ ( Channel << 2 ) + 1 ] = 0;
	FilterBuf [ ( Channel << 2 ) + 2 ] = 0;
	FilterBuf [ ( Channel << 2 ) + 3 ] = 0;
	
	// set the cycle sample was keyed on at
	StartCycle_Channel [ Channel ] = CycleCount;
	
	// starts adsr in attack phase
	ADSR_Status [ Channel ] = ADSR_ATTACK;
	
	// appears to copy voice start address to voice repeat address unconditionally
	// *** testing *** maybe not
	// *note* the PS1 does NOT copy voice start address to voice repeat address when a voice is keyed on
	/*
	if ( !( LSA_Manual_Bitmap & ( 1 << Channel ) ) )
	{
		//Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ] = Regs [ ( ( Channel << 4 ) + SSA_X ) >> 1 ];
	}
	*/
	
		/*
		// check for loop
		if ( ! ( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] & ( 0x2 << 8 ) ) )
		{
			// the loop is still there, just kill the envelope //
			KillEnvelope_Bitmap |= ( 1 << Channel );
		}
		
		// check if this is loop start
		if ( RAM [ ( CurrentBlockAddress [ Channel ] & c_iRam_Mask ) >> 1 ] & ( 0x4 << 8 ) )
		{
			// don't kill the envelope yet
			KillEnvelope_Bitmap &= ~( 1 << Channel );
		}
		*/
				
	// automatically initializes adsr volume to zero
	// *** testing *** try it the other way
	//Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ] = 0;
	pChRegs0->ENV_X = 0;
	
	VOL_ADSR_Value [ Channel ] = 0;
	
	// loop address not manually specified
	// can still change loop address after sound starts
	LSA_Manual_Bitmap &= ~( 1 << Channel );
	
	// start envelope
	//ModeRate = Regs [ ( ADSR_0 >> 1 ) + ( Channel << 3 ) ];
	ModeRate = pChRegs0->ADSR_0;
	
	//Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 8 ) & 0x7f, ( ModeRate >> 15 ) << 1 );
	Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 8 ) & 0x7f, ( ModeRate >> 15 ) << 1, false );
	
	//Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ] = VOL_ADSR_Value [ Channel ];
	pChRegs0->ENV_X = VOL_ADSR_Value [ Channel ];
	

	// copy the pitch over
	// *problem* *TODO* the pitch and volume look like they can be changed while the sample is playing
	// so the pitch needs to update when it is set also
	//_SPU->dSampleDT [ Channel ] = ( ((u64) Regs [ ( ( Channel << 4 ) + PITCH ) >> 1 ]) << 32 ) >> 12;
	_SPU->dSampleDT [ Channel ] = ( ((u64) pChRegs0->PITCH) << 32 ) >> 12;


	// prepare for playback //
	
	// clear current sample
	CurrentSample_Offset [ Channel ] = 0;
	CurrentSample_Read [ Channel ] = 0;
	CurrentSample_Write [ Channel ] = 0;
	
	// set start address
	//CurrentBlockAddress [ Channel ] = ( (u32) Regs [ ( ( Channel << 4 ) + SSA_X ) >> 1 ] ) << 3;
	CurrentBlockAddress [ Channel ] = ( (u32) pChRegs0->SSA ) << 3;
	
	////////////////////////////////////////////////////////
	// check if the IRQ address is in this block
	// and check if interrupts are enabled
	//if ( ( ( CurrentBlockAddress [ Channel ] >> 4 ) == ( Regs [ ( IRQA - SPU_X ) >> 1 ] >> 1 ) ) && ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] & 0x40 ) )
	if ( ( ( CurrentBlockAddress [ Channel ] >> 4 ) == ( pCoreRegs->IRQA >> 1 ) ) && ( pCoreRegs->CTRL & 0x40 ) )
	{
		// we have reached irq address - trigger interrupt
		SetInterrupt ();
		
		// interrupt
		//Regs [ ( 0x1f801dae - SPU_X ) >> 1 ] |= 0x40;
		pCoreRegs->STAT |= 0x40;
	}
	
	//////////////////////////////////////////////////////////////////////////////
	// Check loop start flag and set loop address if needed
	// LOOP_START is bit 3 actually
	// note: LOOP_START is actually bit 2
	//if ( ( RAM [ ( CurrentBlockAddress [ Channel ] & c_iRam_Mask ) >> 1 ] & ( 0x4 << 8 ) ) && ( ! ( LSA_Manual_Bitmap & ( 1 << Channel ) ) ) )
	if ( ( RAM [ ( CurrentBlockAddress [ Channel ] & c_iRam_Mask ) >> 1 ] & ( 0x4 << 8 ) ) )
	{
#ifdef INLINE_DEBUG_WRITE_KON_0
	debug << "; Channel=" << Channel << "; LOOP_AT_START";
#endif

		///////////////////////////////////////////////////
		// we are at loop start address
		// set loop start address
		//Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ] = ( CurrentBlockAddress [ Channel ] >> 3 );
		pChRegs0->LSA = ( CurrentBlockAddress [ Channel ] >> 3 );
	}

	// assume loop bits are set in waveform at start of key-on
	bLoopSet |= (1 << Channel);

	// clear loop set if loop bit not set
	if (!(RAM[(CurrentBlockAddress[Channel] & c_iRam_Mask) >> 1] & (0x2 << 8)))
	{
		bLoopSet &= ~(1 << Channel);
	}

	// decode the new block
	
	// clear the samples first because of interpolation algorithm
	for ( int i = 0; i < 32; i++ ) DecodedBlocks [ ( Channel << 5 ) + i ] = 0;
	
#ifdef INLINE_DEBUG_SPU_ERROR_RECORD
	u32 filter = ( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] >> 12 );
	if ( filter > 4 ) 
	{
		debug << "\r\nhpsx64 ALERT: SPU: Filter value is greater than 4 (invalid): filter=" << dec << filter << hex << " SPUAddress=" << CurrentBlockAddress [ Channel ] << " shifted=" << ( CurrentBlockAddress [ Channel ] >> 3 );
		debug << " Channel=" << dec << Channel << " LSA_X=" << hex << Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ] << " SSA_X=" << Regs [ ( ( Channel << 4 ) + SSA_X ) >> 1 ];
	}
#endif
	
	// now decode the sample packet into buffer
	//SampleDecoder [ Channel ].decode_packet ( (adpcm_packet*) & ( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] ), & ( DecodedBlocks [ ( Channel << 5 ) + ( ( CurrentSample_Read [ Channel ] >> 32 ) & 0x1f ) ] ) );
	SampleDecoder [ Channel ].decode_packet32 ( (adpcm_packet*) & ( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] ), DecodedSamples );
	for ( int i = 0; i < 28; i++ ) DecodedBlocks [ ( Channel << 5 ) + ( ( CurrentSample_Write [ Channel ] + i ) & 0x1f ) ] = DecodedSamples [ i ];
	
	// clear killing of envelope
	// let's actually try killing the envelope
	//KillEnvelope_Bitmap &= ~( 1 << Channel );
	//KillEnvelope_Bitmap |= ( 1 << Channel );
	
	// check for loop
	//if ( ! ( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] & ( 0x2 << 8 ) ) )
	//{
	//	// the loop is still there, just kill the envelope //
	//	KillEnvelope_Bitmap |= ( 1 << Channel );
	//}
	
	// *** testing *** try resetting decoder
	//SampleDecoder [ Channel ].reset ();
}






void SPU::DebugWindow_Enable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	static constexpr int SPUList_CountX = 9;
	
	static constexpr int SPUMasterList_X = 0;
	static constexpr int SPUMasterList_Y = 0;
	static constexpr int SPUMasterList_Width = 90;
	static constexpr int SPUMasterList_Height = 120;
	
	static constexpr int SPUList_X = 0;
	static constexpr int SPUList_Y = 0;
	static constexpr int SPUList_Width = SPUMasterList_Width;
	static constexpr int SPUList_Height = SPUMasterList_Height;
	
	static constexpr char* DebugWindow_Caption = "SPU1 Debug Window";
	static constexpr int DebugWindow_X = 10;
	static constexpr int DebugWindow_Y = 10;
	static constexpr int DebugWindow_Width = ( SPUList_Width * SPUList_CountX ) + 10;
	static constexpr int DebugWindow_Height = ( ( ( ( c_iNumberOfChannels + 1 ) / SPUList_CountX ) + 1 ) * SPUList_Height ) + 20;
	
	static constexpr int MemoryViewer_X = SPUList_Width * 7;
	static constexpr int MemoryViewer_Y = SPUList_Height * 2;
	static constexpr int MemoryViewer_Width = SPUList_Width * 2;
	static constexpr int MemoryViewer_Height = SPUList_Height;
	static constexpr int MemoryViewer_Columns = 4;

	
	
	int i;
	stringstream ss;
	
	if ( !DebugWindow_Enabled )
	{
		// create the main debug window
		DebugWindow = new WindowClass::Window ();
		DebugWindow->Create ( DebugWindow_Caption, DebugWindow_X, DebugWindow_Y, DebugWindow_Width, DebugWindow_Height );
		DebugWindow->DisableCloseButton ();
		
		// create "value lists"
		SPUMaster_ValueList = new DebugValueList<u16> ();
		for ( i = 0; i < NumberOfChannels; i++ ) SPU_ValueList [ i ] = new DebugValueList<u16> ();
		
		// create the value lists
		SPUMaster_ValueList->Create ( DebugWindow, SPUMasterList_X, SPUMasterList_Y, SPUMasterList_Width, SPUMasterList_Height, true, false );
		
		for ( i = 0; i < NumberOfChannels; i++ )
		{
			SPU_ValueList [ i ]->Create ( DebugWindow, SPUList_X + ( ( ( i + 1 ) % SPUList_CountX ) * SPUList_Width ), SPUList_Y + ( ( ( i + 1 ) / SPUList_CountX ) * SPUList_Height ), SPUList_Width, SPUList_Height, true, false );
		}
		

		SPUMaster_ValueList->AddVariable ( "CON_0", & (_SPU->Regs [ ( SPU::CON_0 - SPU::SPU_X ) >> 1 ]) );
		SPUMaster_ValueList->AddVariable ( "CON_1", & (_SPU->Regs [ ( SPU::CON_1 - SPU::SPU_X ) >> 1 ]) );
		SPUMaster_ValueList->AddVariable ( "KON_0", & (_SPU->Regs [ ( SPU::KON_0 - SPU::SPU_X ) >> 1 ]) );
		SPUMaster_ValueList->AddVariable ( "KON_1", & (_SPU->Regs [ ( SPU::KON_1 - SPU::SPU_X ) >> 1 ]) );
		SPUMaster_ValueList->AddVariable ( "KOFF_0", & (_SPU->Regs [ ( SPU::KOFF_0 - SPU::SPU_X ) >> 1 ]) );
		SPUMaster_ValueList->AddVariable ( "KOFF_1", & (_SPU->Regs [ ( SPU::KOFF_1 - SPU::SPU_X ) >> 1 ]) );
		SPUMaster_ValueList->AddVariable ( "MVOL_L", & (_SPU->Regs [ ( SPU::MVOL_L - SPU::SPU_X ) >> 1 ]) );
		SPUMaster_ValueList->AddVariable ( "MVOL_R", & (_SPU->Regs [ ( SPU::MVOL_R - SPU::SPU_X ) >> 1 ]) );
		SPUMaster_ValueList->AddVariable ( "EVOL_L", & (_SPU->Regs [ ( SPU::EVOL_L - SPU::SPU_X ) >> 1 ]) );
		SPUMaster_ValueList->AddVariable ( "EVOL_R", & (_SPU->Regs [ ( SPU::EVOL_R - SPU::SPU_X ) >> 1 ]) );
		SPUMaster_ValueList->AddVariable ( "SPU_CTRL", & (_SPU->Regs [ ( 0x1f801daa - SPU::SPU_X ) >> 1 ]) );
		SPUMaster_ValueList->AddVariable ( "SPU_STAT", & (_SPU->Regs [ ( 0x1f801dae - SPU::SPU_X ) >> 1 ]) );
		
		// add variables into lists
		for ( i = 0; i < NumberOfChannels; i++ )
		{
			static const char* c_sChannelStr = "C";
			
			ss.str ("");
			ss << c_sChannelStr << dec << i << "_VOLL";
			SPU_ValueList [ i ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::VOL_L ) >> 1 ]) );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_VOLR";
			SPU_ValueList [ i ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::VOL_R ) >> 1 ]) );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_PITCH";
			SPU_ValueList [ i ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::PITCH ) >> 1 ]) );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_SSAX";
			SPU_ValueList [ i ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::SSA_X ) >> 1 ]) );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_ADSR0";
			SPU_ValueList [ i ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::ADSR_0 ) >> 1 ]) );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_ADSR1";
			SPU_ValueList [ i ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::ADSR_1 ) >> 1 ]) );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_ENVX";
			SPU_ValueList [ i ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::ENV_X ) >> 1 ]) );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_LSAX";
			SPU_ValueList [ i ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::LSA_X ) >> 1 ]) );
			
			ss.str ("");
			ss << c_sChannelStr << i << "MADSR";
			SPU_ValueList [ i ]->AddVariable ( ss.str().c_str(), &(_SPU->ADSR_Status [ i ]) );
			
			ss.str ("");
			ss << c_sChannelStr << i << "RAW";
			SPU_ValueList [ i ]->AddVariable ( ss.str().c_str(), (u16*) &(_SPU->Debug_CurrentRawSample [ i ]) );
			
			ss.str ("");
			ss << c_sChannelStr << i << "SMP";
			SPU_ValueList [ i ]->AddVariable ( ss.str().c_str(), &(_SPU->Debug_CurrentSample [ i ]) );
			
			//ss.str ("");
			//ss << c_sChannelStr << i << "RATE";
			//SPU_ValueList [ i ]->AddVariable ( ss.str().c_str(), &(_SPU->Debug_CurrentRate [ i ]) );
		}
		
		// create the viewer for D-Cache scratch pad
		SoundRAM_Viewer = new Debug_MemoryViewer ();
		
		SoundRAM_Viewer->Create ( DebugWindow, MemoryViewer_X, MemoryViewer_Y, MemoryViewer_Width, MemoryViewer_Height, MemoryViewer_Columns );
		SoundRAM_Viewer->Add_MemoryDevice ( "SoundRAM", 0, c_iRam_Size, (unsigned char*) _SPU->RAM );
		
		// mark debug as enabled now
		DebugWindow_Enabled = true;
		
		// update the value lists
		DebugWindow_Update ();
	}
	
#endif

}

void SPU::DebugWindow_Disable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;
	
	if ( DebugWindow_Enabled )
	{
		delete DebugWindow;
		delete SPUMaster_ValueList;
		for ( i = 0; i < NumberOfChannels; i++ ) delete SPU_ValueList [ i ];
	
		// disable debug window
		DebugWindow_Enabled = false;
	}
	
#endif

}

void SPU::DebugWindow_Update ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;
	
	if ( DebugWindow_Enabled )
	{
		SPUMaster_ValueList->Update();
		for ( i = 0; i < NumberOfChannels; i++ ) SPU_ValueList [ i ]->Update();
	}
	
#endif

}






