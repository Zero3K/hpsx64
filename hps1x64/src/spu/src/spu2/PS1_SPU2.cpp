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



#include "PS1_SPU2.h"
#include "PS1_CD.h"

#include "PS1_Dma.h"


// inlude for multi-threading testing for now
#include "PS2_Gpu.h"


#include "GeneralUtilities.h"





using namespace Playstation1;
using namespace GeneralUtilities;


#define ENABLE_THREAD_LOCK
#define ENABLE_THREAD_LOCK_DATA


// will try to skip some calculations for muted channels
//#define ENABLE_MUTE_CHANNEL_OPTIMIZATION



#define ENABLE_AUTODMA_DMA
#define ENABLE_AUTODMA_READY
#define ENABLE_AUTODMA_RUN
//#define DISABLE_ENDX_ON_KOFF
//#define DISABLE_ENDX_ON_MUTE

// important note: correct operation is to comment out CLEAR_VMIX_ON_CHDONE
//#define CLEAR_VMIX_ON_CHDONE

#define CLEAR_VMIXE_ON_CHDONE

// only allow reverb interrupt if reverb is enabled
//#define ENABLE_REVERB_INT_ONLY_WHEN_REVERB_ENABLED


#define VERBOSE_KEYONOFF_TEST
//#define VERBOSE_READ_TEST
#define VERBOSE_WRITE_TEST
#define VERBOSE_SPDIF_OUT_TEST

//#define INLINE_DEBUG_ENABLE_AUTODMA_INT
#define INLINE_DEBUG_ENABLE_AUTODMA_INT_SPU

// this enables an exclusive autodma between dma4 and dma7
//#define ENABLE_EXCLUSIVE_AUTODMA


#define KEY_ON_OFF_LOOP2


// require each block to have loop flag set in waveform data or else sound will mute
//#define REQUIRE_ALL_LOOP_FLAGS



//#define CLEAR_ADMAS_ON_INT



//#define UPDATE_NEX_IN_BLOCKS



#define ENABLE_REVERB_IN
#define ENABLE_REVERB_OUT
#define ENABLE_REVERB_RUN



//#define VERBOSE_DEBUG_WRITEREVERB



#define ENABLE_VOICE13_MEM
#define ENABLE_MIXDRY_MEM
#define ENABLE_MIXWET_MEM
#define ENABLE_CORE0_MEM

#define ENABLE_AVOL_CALC



#define ENABLE_IMMEDIATE_ADMA




#ifdef _DEBUG_VERSION_

// enable debugging
#define INLINE_DEBUG_ENABLE

/*
#define INLINE_DEBUG_WRITE
#define INLINE_DEBUG_READ


//#define INLINE_DEBUG_RUN2

#define INLINE_DEBUG_DMA_READ

#define INLINE_DEBUG_DMA_WRITE
#define INLINE_DEBUG_DMA_WRITE_RECORD

#define INLINE_DEBUG_INT




//#define INLINE_DEBUG_UPDATEREVERB



//#define INLINE_DEBUG_SPU2RUN


//#define INLINE_DEBUG_REVERB
//#define INLINE_DEBUG_SPU_ERROR_RECORD
//#define INLINE_DEBUG
//#define INLINE_DEBUG_CDSOUND
//#define INLINE_DEBUG_RUN
#define INLINE_DEBUG_RUN_ATTACK
#define INLINE_DEBUG_RUN_DECAY
#define INLINE_DEBUG_RUN_SUSTAIN
#define INLINE_DEBUG_ENVELOPE
//#define INLINE_DEBUG_RUN_VOLUME_ENVELOPE
//#define INLINE_DEBUG_WRITE_DEFAULT
//#define INLINE_DEBUG_RUN_CHANNELONOFF
//#define INLINE_DEBUG_READ_CHANNELONOFF
//#define INLINE_DEBUG_WRITE_CHANNELONOFF
#define INLINE_DEBUG_WRITE_IRQA
#define INLINE_DEBUG_WRITE_CTRL
//#define INLINE_DEBUG_WRITE_STAT
//#define INLINE_DEBUG_WRITE_DATA
//#define INLINE_DEBUG_WRITE_SBA
#define INLINE_DEBUG_WRITE_LSA_X
*/

#endif


funcVoid SPUCore::UpdateInterrupts;
funcVoid SPU2::UpdateInterrupts;

SPUCore::Regs SPUCore::Regs16;

Debug::Log SPUCore::debug;
SPUCore *SPUCore::_SPUCore;
u16 *SPUCore::RAM;


u32* SPUCore::_DebugPC;
u64* SPUCore::_DebugCycleCount;
u64* SPUCore::_SystemCycleCount;

u32* SPUCore::_Intc_Stat;
u32* SPUCore::_Intc_Mask;
u32* SPUCore::_R3000A_Status_12;
u32* SPUCore::_R3000A_Cause_13;
u64* SPUCore::_ProcStatus;


bool SPUCore::DebugWindow_Enabled [ 2 ];
WindowClass::Window *SPUCore::DebugWindow [ 2 ];
DebugValueList<u16> *SPUCore::SPUMaster_ValueList [ 2 ];
DebugValueList<u16> *SPUCore::SPU_ValueList [ 24 ] [ 2 ];
Debug_MemoryViewer *SPUCore::SoundRAM_Viewer [ 2 ];


Api::Thread* SPU2::Threads [ SPU2::c_iMaxThreads ];
u32 SPU2::ulNumberOfThreads_Created;

volatile u64 SPU2::ullThReadIdx;
volatile u64 SPU2::ullThWriteIdx;
volatile u64 SPU2::ullThTargetIdx;
volatile u64 SPU2::ullLastWriteIdx [ SPU2::c_ullThBufferSize ];
volatile u64 SPU2::ullLastSize [ SPU2::c_ullThBufferSize ];

volatile u64 SPU2::ullThreadRunning;

HANDLE SPU2::ghEvent_Update;



u32* SPU2::_DebugPC;
u64* SPU2::_DebugCycleCount;
u64* SPU2::_SystemCycleCount;
u32* SPU2::_NextEventIdx;


u64* SPU2::_NextSystemEvent;


u32* SPU2::_Intc_Stat;
u32* SPU2::_Intc_Mask;
u32* SPU2::_R3000A_Status_12;
u32* SPU2::_R3000A_Cause_13;
u64* SPU2::_ProcStatus;


Debug::Log SPU2::debug;

SPU2 *SPU2::_SPU2;

/*
static s16 *SPU::_vLOUT;
static s16 *SPU::_vROUT;
static u16 *SPU::_mBASE;

static u16 *SPU::_dAPF1;
static u16 *SPU::_dAPF2;
static s16 *SPU::_vIIR;
static s16 *SPU::_vCOMB1;
static s16 *SPU::_vCOMB2;
static s16 *SPU::_vCOMB3;
static s16 *SPU::_vCOMB4;
static s16 *SPU::_vWALL;
static s16 *SPU::_vAPF1;
static s16 *SPU::_vAPF2;
static u16 *SPU::_mLSAME;
static u16 *SPU::_mRSAME;
static u16 *SPU::_mLCOMB1;
static u16 *SPU::_mRCOMB1;
static u16 *SPU::_mLCOMB2;
static u16 *SPU::_mRCOMB2;
static u16 *SPU::_dLSAME;
static u16 *SPU::_dRSAME;
static u16 *SPU::_mLDIFF;
static u16 *SPU::_mRDIFF;
static u16 *SPU::_mLCOMB3;
static u16 *SPU::_mRCOMB3;
static u16 *SPU::_mLCOMB4;
static u16 *SPU::_mRCOMB4;
static u16 *SPU::_dLDIFF;
static u16 *SPU::_dRDIFF;
static u16 *SPU::_mLAPF1;
static u16 *SPU::_mRAPF1;
static u16 *SPU::_mLAPF2;
static u16 *SPU::_mRAPF2;
static s16 *SPU::_vLIN;
static s16 *SPU::_vRIN;
*/



//bool SPUCore::DebugWindow_Enabled;
//WindowClass::Window *SPUCore::DebugWindow;
//DebugValueList<u16> *SPUCore::SPUMaster_ValueList;
//DebugValueList<u16> *SPUCore::SPU_ValueList [ 24 ];
//Debug_MemoryViewer *SPUCore::SoundRAM_Viewer;

u32 SPUCore::Debug_ChannelEnable = 0xffffff;

HWAVEOUT SPU2::hWaveOut; /* device handle */
WAVEFORMATEX SPU2::wfx;
WAVEHDR SPU2::header;
WAVEHDR SPU2::header0;
WAVEHDR SPU2::header1;



//SPUCore::SpuRegs0_t *SPUCore::SpuRegs0;
//SPUCore::SpuRegs1_t *SPUCore::SpuRegs1;



// *** testing ***
//u64 SPU::hWaveOut_Save;


// 419 registers in this section (0x0-0x346 [0-838])
const char* SPUCore::RegisterNames1 [ 419 ] = {
	// 0x0
	// for multi-threading need: ChX_ENV_X, ChX_CVOL_L, ChX_CVOL_R
	"Ch0_VOL_L", "Ch0_VOL_R", "Ch0_PITCH", "Ch0_ADSR_0", "Ch0_ADSR_1", "Ch0_ENV_X", "Ch0_CVOL_L", "Ch0_CVOL_R",
	"Ch1_VOL_L", "Ch1_VOL_R", "Ch1_PITCH", "Ch1_ADSR_0", "Ch1_ADSR_1", "Ch1_ENV_X", "Ch1_CVOL_L", "Ch1_CVOL_R",
	"Ch2_VOL_L", "Ch2_VOL_R", "Ch2_PITCH", "Ch2_ADSR_0", "Ch2_ADSR_1", "Ch2_ENV_X", "Ch2_CVOL_L", "Ch2_CVOL_R",
	"Ch3_VOL_L", "Ch3_VOL_R", "Ch3_PITCH", "Ch3_ADSR_0", "Ch3_ADSR_1", "Ch3_ENV_X", "Ch3_CVOL_L", "Ch3_CVOL_R",
	"Ch4_VOL_L", "Ch4_VOL_R", "Ch4_PITCH", "Ch4_ADSR_0", "Ch4_ADSR_1", "Ch4_ENV_X", "Ch4_CVOL_L", "Ch4_CVOL_R",
	"Ch5_VOL_L", "Ch5_VOL_R", "Ch5_PITCH", "Ch5_ADSR_0", "Ch5_ADSR_1", "Ch5_ENV_X", "Ch5_CVOL_L", "Ch5_CVOL_R",
	"Ch6_VOL_L", "Ch6_VOL_R", "Ch6_PITCH", "Ch6_ADSR_0", "Ch6_ADSR_1", "Ch6_ENV_X", "Ch6_CVOL_L", "Ch6_CVOL_R",
	"Ch7_VOL_L", "Ch7_VOL_R", "Ch7_PITCH", "Ch7_ADSR_0", "Ch7_ADSR_1", "Ch7_ENV_X", "Ch7_CVOL_L", "Ch7_CVOL_R",
	"Ch8_VOL_L", "Ch8_VOL_R", "Ch8_PITCH", "Ch8_ADSR_0", "Ch8_ADSR_1", "Ch8_ENV_X", "Ch8_CVOL_L", "Ch8_CVOL_R",
	"Ch9_VOL_L", "Ch9_VOL_R", "Ch9_PITCH", "Ch9_ADSR_0", "Ch9_ADSR_1", "Ch9_ENV_X", "Ch9_CVOL_L", "Ch9_CVOL_R",
	"Ch10_VOL_L", "Ch10_VOL_R", "Ch10_PITCH", "Ch10_ADSR_0", "Ch10_ADSR_1", "Ch10_ENV_X", "Ch10_CVOL_L", "Ch10_CVOL_R",
	"Ch11_VOL_L", "Ch11_VOL_R", "Ch11_PITCH", "Ch11_ADSR_0", "Ch11_ADSR_1", "Ch11_ENV_X", "Ch11_CVOL_L", "Ch11_CVOL_R",
	"Ch12_VOL_L", "Ch12_VOL_R", "Ch12_PITCH", "Ch12_ADSR_0", "Ch12_ADSR_1", "Ch12_ENV_X", "Ch12_CVOL_L", "Ch12_CVOL_R",
	"Ch13_VOL_L", "Ch13_VOL_R", "Ch13_PITCH", "Ch13_ADSR_0", "Ch13_ADSR_1", "Ch13_ENV_X", "Ch13_CVOL_L", "Ch13_CVOL_R",
	"Ch14_VOL_L", "Ch14_VOL_R", "Ch14_PITCH", "Ch14_ADSR_0", "Ch14_ADSR_1", "Ch14_ENV_X", "Ch14_CVOL_L", "Ch14_CVOL_R",
	"Ch15_VOL_L", "Ch15_VOL_R", "Ch15_PITCH", "Ch15_ADSR_0", "Ch15_ADSR_1", "Ch15_ENV_X", "Ch15_CVOL_L", "Ch15_CVOL_R",
	"Ch16_VOL_L", "Ch16_VOL_R", "Ch16_PITCH", "Ch16_ADSR_0", "Ch16_ADSR_1", "Ch16_ENV_X", "Ch16_CVOL_L", "Ch16_CVOL_R",
	"Ch17_VOL_L", "Ch17_VOL_R", "Ch17_PITCH", "Ch17_ADSR_0", "Ch17_ADSR_1", "Ch17_ENV_X", "Ch17_CVOL_L", "Ch17_CVOL_R",
	"Ch18_VOL_L", "Ch18_VOL_R", "Ch18_PITCH", "Ch18_ADSR_0", "Ch18_ADSR_1", "Ch18_ENV_X", "Ch18_CVOL_L", "Ch18_CVOL_R",
	"Ch19_VOL_L", "Ch19_VOL_R", "Ch19_PITCH", "Ch19_ADSR_0", "Ch19_ADSR_1", "Ch19_ENV_X", "Ch19_CVOL_L", "Ch19_CVOL_R",
	"Ch20_VOL_L", "Ch20_VOL_R", "Ch20_PITCH", "Ch20_ADSR_0", "Ch20_ADSR_1", "Ch20_ENV_X", "Ch20_CVOL_L", "Ch20_CVOL_R",
	"Ch21_VOL_L", "Ch21_VOL_R", "Ch21_PITCH", "Ch21_ADSR_0", "Ch21_ADSR_1", "Ch21_ENV_X", "Ch21_CVOL_L", "Ch21_CVOL_R",
	"Ch22_VOL_L", "Ch22_VOL_R", "Ch22_PITCH", "Ch22_ADSR_0", "Ch22_ADSR_1", "Ch22_ENV_X", "Ch22_CVOL_L", "Ch22_CVOL_R",
	"Ch23_VOL_L", "Ch23_VOL_R", "Ch23_PITCH", "Ch23_ADSR_0", "Ch23_ADSR_1", "Ch23_ENV_X", "Ch23_CVOL_L", "Ch23_CVOL_R",
	// 0x180 (384 dec [192x2])
	// for multi-threading need: VMIXL_0,VMIXL_1,VMIXEL_0,VMIXEL_1,VMIXR_0,VMIXR_1,VMIXER_0,VMIXER_1,MMIX,CTRL
	"PMON_0", "PMON_1", "NON_0", "NON_1", "VMIXL_0", "VMIXL_1", "VMIXEL_0", "VMIXEL_1",
	"VMIXR_0", "VMIXR_1", "VMIXER_0", "VMIXER_1", "MMIX", "CTRL", "IRQA_1", "IRQA_0",
	"KON_0", "KON_1", "KOFF_0", "KOFF_1", "SBA_1", "SBA_0", "DATA", "UNK0",
	"ADMAS?", "UNK1", "UNK2", "UNK3", "UNK4", "UNK5", "UNK6", "UNK7",
	// 0x1c0 (448 dec [224x2])
	"Ch0_SSA_1", "Ch0_SSA_0", "Ch0_LSA_1", "Ch0_LSA_0", "Ch0_NEX_1", "Ch0_NEX_0",
	"Ch1_SSA_1", "Ch1_SSA_0", "Ch1_LSA_1", "Ch1_LSA_0", "Ch1_NEX_1", "Ch1_NEX_0",
	"Ch2_SSA_1", "Ch2_SSA_0", "Ch2_LSA_1", "Ch2_LSA_0", "Ch2_NEX_1", "Ch2_NEX_0",
	"Ch3_SSA_1", "Ch3_SSA_0", "Ch3_LSA_1", "Ch3_LSA_0", "Ch3_NEX_1", "Ch3_NEX_0",
	"Ch4_SSA_1", "Ch4_SSA_0", "Ch4_LSA_1", "Ch4_LSA_0", "Ch4_NEX_1", "Ch4_NEX_0",
	"Ch5_SSA_1", "Ch5_SSA_0", "Ch5_LSA_1", "Ch5_LSA_0", "Ch5_NEX_1", "Ch5_NEX_0",
	"Ch6_SSA_1", "Ch6_SSA_0", "Ch6_LSA_1", "Ch6_LSA_0", "Ch6_NEX_1", "Ch6_NEX_0",
	"Ch7_SSA_1", "Ch7_SSA_0", "Ch7_LSA_1", "Ch7_LSA_0", "Ch7_NEX_1", "Ch7_NEX_0",
	"Ch8_SSA_1", "Ch8_SSA_0", "Ch8_LSA_1", "Ch8_LSA_0", "Ch8_NEX_1", "Ch8_NEX_0",
	"Ch9_SSA_1", "Ch9_SSA_0", "Ch9_LSA_1", "Ch9_LSA_0", "Ch9_NEX_1", "Ch9_NEX_0",
	"Ch10_SSA_1", "Ch10_SSA_0", "Ch10_LSA_1", "Ch10_LSA_0", "Ch10_NEX_1", "Ch10_NEX_0",
	"Ch11_SSA_1", "Ch11_SSA_0", "Ch11_LSA_1", "Ch11_LSA_0", "Ch11_NEX_1", "Ch11_NEX_0",
	"Ch12_SSA_1", "Ch12_SSA_0", "Ch12_LSA_1", "Ch12_LSA_0", "Ch12_NEX_1", "Ch12_NEX_0",
	"Ch13_SSA_1", "Ch13_SSA_0", "Ch13_LSA_1", "Ch13_LSA_0", "Ch13_NEX_1", "Ch13_NEX_0",
	"Ch14_SSA_1", "Ch14_SSA_0", "Ch14_LSA_1", "Ch14_LSA_0", "Ch14_NEX_1", "Ch14_NEX_0",
	"Ch15_SSA_1", "Ch15_SSA_0", "Ch15_LSA_1", "Ch15_LSA_0", "Ch15_NEX_1", "Ch15_NEX_0",
	"Ch16_SSA_1", "Ch16_SSA_0", "Ch16_LSA_1", "Ch16_LSA_0", "Ch16_NEX_1", "Ch16_NEX_0",
	"Ch17_SSA_1", "Ch17_SSA_0", "Ch17_LSA_1", "Ch17_LSA_0", "Ch17_NEX_1", "Ch17_NEX_0",
	"Ch18_SSA_1", "Ch18_SSA_0", "Ch18_LSA_1", "Ch18_LSA_0", "Ch18_NEX_1", "Ch18_NEX_0",
	"Ch19_SSA_1", "Ch19_SSA_0", "Ch19_LSA_1", "Ch19_LSA_0", "Ch19_NEX_1", "Ch19_NEX_0",
	"Ch20_SSA_1", "Ch20_SSA_0", "Ch20_LSA_1", "Ch20_LSA_0", "Ch20_NEX_1", "Ch20_NEX_0",
	"Ch21_SSA_1", "Ch21_SSA_0", "Ch21_LSA_1", "Ch21_LSA_0", "Ch21_NEX_1", "Ch21_NEX_0",
	"Ch22_SSA_1", "Ch22_SSA_0", "Ch22_LSA_1", "Ch22_LSA_0", "Ch22_NEX_1", "Ch22_NEX_0",
	"Ch23_SSA_1", "Ch23_SSA_0", "Ch23_LSA_1", "Ch23_LSA_0", "Ch23_NEX_1", "Ch23_NEX_0",
	// 0x2e0 (736 dec [368x2])
	// for multi-threading need: ALL
	"RVWAS_1", "RVWAS_0", "dAPF1_1", "dAPF1_0", "dAPF2_1", "dAPF2_0", "mLSAME_1", "mLSAME_0",
	"mRSAME_1", "mRSAME_0", "mLCOMB1_1", "mLCOMB1_0", "mRCOMB1_1", "mRCOMB1_0", "mLCOMB2_1", "mLCOMB2_0",
	"mRCOMB2_1", "mRCOMB2_0", "dLSAME_1", "dLSAME_0", "dRSAME_1", "dRSAME_0", "mLDIFF_1", "mLDIFF_0",
	"mRDIFF_1", "mRDIFF_0", "mLCOMB3_1", "mLCOMB3_0", "mRCOMB3_1", "mRCOMB3_0", "mLCOMB4_1", "mLCOMB4_0",
	"mRCOMB4_1", "mRCOMB4_0", "dLDIFF_1", "dLDIFF_0", "dRDIFF_1", "dRDIFF_0", "mLAPF1_1", "mLAPF1_0",
	"mRAPF1_1", "mRAPF1_0", "mLAPF2_1", "mLAPF2_0", "mRAPF2_1", "mRAPF2_0", "RVWAE_1", "RVWAE_0",
	
	// 0x340 (832 dec [416x2])
	"CON_0", "CON_1", "STAT"
	
	// 0x346-0x400 are unknown/not used??
};




// 20 registers in this section
const char* SPUCore::RegisterNames2 [ 20 ] = {
	// core0 regs followed by core1 regs again
	// 0x760
	// for multi-threading need: ALL except MVOL_L,MVOL_R
	"MVOL_L", "MVOL_R", "EVOL_L", "EVOL_R", "AVOL_L", "AVOL_R", "BVOL_L", "BVOL_R",
	"CMVOL_L", "CMVOL_R", "vIIR", "vCOMB1", "vCOMB2", "vCOMB3", "vCOMB4", "vWALL",
	"vAPF1", "vAPF2", "vLIN", "vRIN"
	
	// 0x788
	//"MVOLL", "MVOLR", "EVOL_L", "EVOL_R", "AVOL_L", "AVOL_R", "BVOL_L", "BVOL_R",
	//"MVOLX_L", "MVOLX_R", "vIIR", "vCOMB1", "vCOMB2", "vCOMB3", "vCOMB4", "vWALL",
	//"vAPF1", "vAPF2", "vLIN", "vRIN"
	
	// 0x7b0
	//"UNK_A", "UNK_B", "UNK_C", "UNK_D", "UNK_E", "UNK_F", "UNK_G", "UNK_H",

};


// 5 registers in this section
const char* SPUCore::RegisterNames3 [ 5 ] = {
	// 0x7c0
	"SPDIF_OUT", "SPDIF_IRQINFO", "SPDIF_MODE", "SPDIF_MEDIA", "SPDIF_PROTECT"
};



SPUCore::SPUCore ()
{
	cout << "Running SPUCore constructor...\n";
}

SPUCore::~SPUCore ()
{
	cout << "Running SPUCore destructor...\n";
}

// for now, only call this after setting the core number
void SPUCore::Start ()
{
	cout << "Running SPU::Start...\n";


#ifdef INLINE_DEBUG
	debug << "\r\nEntering SPU::Start";
#endif

	// can't use this for anything, since there are multiple spu cores
	//_SPUCore = this;
	
	Reset ();
	
	// set the pointers that are not static
	// also need to call this AFTER loading a save state
	Refresh ();

#ifdef INLINE_DEBUG
	debug << "->Exiting SPU::Start";
#endif
}


void SPUCore::Reset ()
{
	int i;
	
	// zero object
	memset ( this, 0, sizeof( SPUCore ) );
	
	// pointers for quick access
	/*
	_vLOUT = & ( Regs [ ( vLOUT - SPU_X ) >> 1 ] );
	_vROUT = & ( Regs [ ( vROUT - SPU_X ) >> 1 ] );
	_mBASE = & ( Regs [ ( mBASE - SPU_X ) >> 1 ] );

	_dAPF1 = & ( Regs [ ( dAPF1 - SPU_X ) >> 1 ] );
	_dAPF2 = & ( Regs [ ( dAPF2 - SPU_X ) >> 1 ] );
	_vIIR = & ( Regs [ ( vIIR - SPU_X ) >> 1 ] );
	_vCOMB1 = & ( Regs [ ( vCOMB1 - SPU_X ) >> 1 ] );
	_vCOMB2 = & ( Regs [ ( vCOMB2 - SPU_X ) >> 1 ] );
	_vCOMB3 = & ( Regs [ ( vCOMB3 - SPU_X ) >> 1 ] );
	_vCOMB4 = & ( Regs [ ( vCOMB4 - SPU_X ) >> 1 ] );
	_vWALL = & ( Regs [ ( vWALL - SPU_X ) >> 1 ] );
	_vAPF1 = & ( Regs [ ( vAPF1 - SPU_X ) >> 1 ] );
	_vAPF2 = & ( Regs [ ( vAPF2 - SPU_X ) >> 1 ] );
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
	_vLIN = & ( Regs [ ( vLIN - SPU_X ) >> 1 ] );
	_vRIN = & ( Regs [ ( vRIN - SPU_X ) >> 1 ] );
	*/
	
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
	
	
	// SPU is not needed to run on EVERY cycle
	//Set_NextEvent ( CPUCycles_PerSPUCycle );
}


u32 SPUCore::Read ( u32 Offset, u32 Mask )
{
#ifdef INLINE_DEBUG_READ
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << hex << " Mask=" << Mask << " Offset= " << Offset;
	debug << " Core#" << CoreNumber;
#endif

	//u32 lReg;
	u32 ROffset;
	u32 Output;
	int Channel;

	// make sure register is in the right range
	if ( Offset >= ( c_iNumberOfRegisters << 1 ) ) return 0;
	
	ROffset = Offset;
	
	if ( CoreNumber )
	{
		// check if register is in group 1
		if ( Offset < 0x760 )
		{
			ROffset = Offset - 0x400;
		}
		else
		{
			// 20 registers per core in group 2, 40 bytes
			ROffset = Offset - 40;
		}
	}
	
	
#ifdef INLINE_DEBUG_READ
	debug << hex << " ROffset=" << ROffset;
	// if register is within group1 or 2, then output the name of the register
	if ( ROffset < 0x346 )
	{
		debug << " " << RegisterNames1 [ ROffset >> 1 ];
	}
	else if ( ROffset >= 0x760 && ROffset < 0x788 )
	{
		debug << " " << RegisterNames2 [ ( ROffset - 0x760 ) >> 1 ];
	}
#endif


	
	// Read SPU register value
	//lReg = Offset >> 1;
	
	// perform actions as needed
	switch ( ROffset )
	{
			
		default:
		
			// check if reading from first group of channel registers
			/*
			if ( Offset < 0x180 )
			{
				Channel = ( Offset >> 4 );
				
				switch ( Offset & 0xf )
				{
					// ENV_X ( Channel )
					//case 0xa:
						// ***TODO*** get rid of VOL_ADSR_Value variable
						// for now, return envelope from variable
						//GET_REG16 ( ENVX_CH ( Channel ) ) = VOL_ADSR_Value [ Channel ];
						//break;
				}
			}
			*/
			
			break;
	}
	
	// return value;
	//Output = Regs16 [ lReg ];
	Output = GET_REG16 ( Offset );
	
#ifdef INLINE_DEBUG_READ
	debug << "; Output=" << Output;
#endif

	return Output;
}


void SPUCore::Write ( u32 Offset, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << hex << " Mask=" << Mask << " Offset= " << Offset << " Data=" << Data;
	debug << " Core#" << CoreNumber;
#endif

	//u32 lReg;
	u32 ROffset;
	u16 ModeRate;
	int Channel;

	u32 Temp;

	ChRegs0_Layout *pChRegs0;
	ChRegs1_Layout *pChRegs1;
	
	// make sure register is in the right range
	if ( Offset >= ( c_iNumberOfRegisters << 1 ) ) return;
	
	ROffset = Offset;
	
	if ( CoreNumber )
	{
		// check if register is in group 1
		if ( Offset < 0x760 )
		{
			ROffset = Offset - 0x400;
		}
		else
		{
			// 20 registers per core in group 2, 40 bytes
			ROffset = Offset - 40;
		}
	}
	
	
#ifdef INLINE_DEBUG_WRITE
	debug << hex << " ROffset=" << ROffset;
	// if register is within group1 or 2, then output the name of the register
	if ( ROffset < 0x346 )
	{
		debug << " " << RegisterNames1 [ ROffset >> 1 ];
	}
	else if ( ROffset >= 0x760 && ROffset < 0x788 )
	{
		debug << " " << RegisterNames2 [ ( ROffset - 0x760 ) >> 1 ];
	}
#endif

	
	// all of the data should be 16-bits ?
	Data &= 0xffff;
	
	
	// Get the register number from offset
	//lReg = Offset >> 1;
	
	// perform actions as needed
	switch ( ROffset )
	{
		case STAT:
			// this one should be read-only
			return;
			break;
			
		// reverb work address START
		case RVWAS_0:
		case RVWAS_1:
		
			//_SPU->Regs [ ( ( RVWA - SPU_X ) >> 1 ) & 0xff ] = (u16)Data;
			GET_REG16 ( Offset ) = (u16) Data;
			
			//_SPU->ReverbWork_Start = ( Data & 0xffff ) << 3;
			ReverbWork_Start = SWAPH ( pCoreRegs0->RVWAS );
			
			// mask against size of ram for now
			ReverbWork_Start &= ( c_iRam_Mask >> 1 );
			//ReverbWork_Start &= ( c_iRam_Size >> 1 );
			
			// align to sound block boundary ??
			//ReverbWork_Start &= ~7;
			
			//_SPU->ReverbWork_Size = c_iRam_Size - _SPU->ReverbWork_Start;
			ReverbWork_Size = ReverbWork_End - ReverbWork_Start;
			
			// make sure the size is positive
			if ( ReverbWork_Size < 0 ) ReverbWork_Size = 0;
			
			
			break;
			
		// reverb work address END
		// the only reverb work address end register is the high one
		// the low one does not exist and never gets accessed....
		case RVWAE_1:
		
			// only the bottom 6 bits in this register are valid
			Data &= 0x3f;
			
			//_SPU->Regs [ ( ( RVWA - SPU_X ) >> 1 ) & 0xff ] = (u16)Data;
			GET_REG16 ( Offset ) = (u16) Data;
			
			// the end address is inclusive, the lower 16-bits fixed at 0xffff
			//_SPU->ReverbWork_Start = ( Data & 0xffff ) << 3;
			//ReverbWork_End = ( (u32) GET_REG16 ( RVWAE_1 ) ) << 16;
			ReverbWork_End = ( ( ( Data & 0x3f ) << 16 ) | 0xffff );
			
			// mask against size of ram for now
			ReverbWork_End &= ( c_iRam_Mask >> 1 );
			//ReverbWork_End &= ( c_iRam_Size >> 1 );
			
			// add one to get the non-inclusive address it ends at
			ReverbWork_End += 1;
			
			//_SPU->ReverbWork_Size = c_iRam_Size - _SPU->ReverbWork_Start;
			ReverbWork_Size = ReverbWork_End - ReverbWork_Start;
			
			// make sure the size is positive
			if ( ReverbWork_Size < 0 ) ReverbWork_Size = 0;
			
			//_SPU->Reverb_BufferAddress = _SPU->ReverbWork_Start;
			//Reverb_BufferAddress = ReverbWork_Start;
			break;
			
		case RVWAE_0:
			// the lower part of reverb end address is always zero
			Data = 0;
			break;
			
		////////////////////////////////////////
		// Sound Buffer Address
		case SBA_0:
		case SBA_1:
		
			///////////////////////////////////
			// set next sound buffer address
			//_SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ] = (u16)Data;
			GET_REG16 ( Offset ) = (u16) Data;
			
			//NextSoundBufferAddress = ( Data << 3 ) & c_iRam_Mask;
			NextSoundBufferAddress = SWAPH ( pCoreRegs0->SBA ) & ( c_iRam_Mask >> 1 );
			
			// align - this needs to be aligned to a sound block boundary
			NextSoundBufferAddress &= ~7;

			break;
				
		//////////////////////////////////////////
		// Data forwarding register
		case DATA:
		
			// buffer can be written into at any time
			if ( BufferIndex < 32 )
			{
				Buffer [ BufferIndex++ ] = (u16) Data;
			}
			
			break;
				
		case KON_0:
			/////////////////////////////////////////////
			// Key On 0-15
			
			// clear the end of sample passed flag for keyed on channels
			//GET_REG16 ( CON_0 ) &= ~Data;
			pCoreRegs0->CON_0 &= ~Data;
			
#ifdef KEY_ON_OFF_LOOP2
			Temp = Data;
			while ( Temp )
			{
				Channel = Temp & ( -Temp );
				Temp ^= Channel;
				//Channel = __builtin_ctz( Channel );
				Channel = ctz32(Channel);

				// appears to take like 2T before sound actually starts?
				Start_SampleDecoding ( Channel );
			}
#else
			// when keyed on set channel ADSR to attack mode
			for ( Channel = 0; Channel < 16; Channel++ )
			{
				if ( ( 1 << Channel ) & Data )
				{
					Start_SampleDecoding ( Channel );
				}
			}
#endif
			
			
			break;
			
		case KON_1:
			/////////////////////////////////////////////
			// Key On 16-23
			
			// clear the end of sample passed flag for keyed on channels
			//GET_REG16 ( CON_1 ) &= ~Data;
			pCoreRegs0->CON_1 &= ~Data;
			
#ifdef KEY_ON_OFF_LOOP2
			Temp = Data;
			Temp &= 0xff;
			while ( Temp )
			{
				Channel = Temp & ( -Temp );
				Temp ^= Channel;
				//Channel = 16 + __builtin_ctz( Channel );
				Channel = 16 + ctz32(Channel);

				Start_SampleDecoding ( Channel );
			}
#else
			// when keyed on set channel ADSR to attack mode
			for ( Channel = 16; Channel < 24; Channel++ )
			{
				if ( ( 1 << ( Channel - 16 ) ) & Data )
				{
					Start_SampleDecoding ( Channel );
				}
			}
#endif
			
			break;
			
		case KOFF_0:
			/////////////////////////////////////////////
			// Key off 0-15
			
#ifdef KEY_ON_OFF_LOOP2
			// on key off we need to change ADSR mode to release mode
			Temp = Data;
			while ( Temp )
			{
				Channel = Temp & ( -Temp );
				Temp ^= Channel;
				//Channel = __builtin_ctz( Channel );
				Channel = ctz32(Channel);

#ifdef VERBOSE_KEYONOFF_TEST
				if ( ( CycleCount - StartCycle_Channel [ Channel ] ) < 4 )
				{
					// check if the key-off signal was given within 2T or 3T
					cout << "\nhps1x64: SPU2: ALERT: Channel#" << dec << Channel << " key-off after " << ( CycleCount - StartCycle_Channel [ Channel ] ) << "T";
				}
#endif
				
				pChRegs0 = (ChRegs0_Layout*) ( & ( pCoreRegs0->ChRegs0 [ Channel ] ) );
				//pChRegs1 = (ChRegs1_Layout*) ( & ( pCoreRegs0->ChRegs1 [ Channel ] ) );
				
				// put channel in adsr release phase unconditionally
				ADSR_Status [ Channel ] = ADSR_RELEASE;
				
				// *** TODO *** remove VOL_ADSR_Value variable
				// start envelope for release mode
				//ModeRate = _SPU->Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
				ModeRate = pChRegs0->ADSR_1;
				
				//Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
				Start_VolumeEnvelope ( (s16&) pChRegs0->ENV_X, Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
				
				// loop address not manually specified
				// can still change loop address after sound starts
				// todo: this should be cleared instead at key-on ??
				//LSA_Manual_Bitmap &= ~( 1 << Channel );
			}
#else
			// on key off we need to change ADSR mode to release mode
			for ( Channel = 0; Channel < 16; Channel++ )
			{
				if ( ( 1 << Channel ) & Data )
				{
#ifdef VERBOSE_KEYONOFF_TEST
					if ( ( CycleCount - StartCycle_Channel [ Channel ] ) < 4 )
					{
						// check if the key-off signal was given within 2T or 3T
						cout << "\nhps1x64: SPU2: ALERT: Channel#" << dec << Channel << " key-off after " << ( CycleCount - StartCycle_Channel [ Channel ] ) << "T";
					}
#endif
				
					pChRegs0 = (ChRegs0_Layout*) ( & ( pCoreRegs0->ChRegs0 [ Channel ] ) );
					//pChRegs1 = (ChRegs1_Layout*) ( & ( pCoreRegs0->ChRegs1 [ Channel ] ) );
					
					// put channel in adsr release phase unconditionally
					ADSR_Status [ Channel ] = ADSR_RELEASE;
					
					// *** TODO *** remove VOL_ADSR_Value variable
					// start envelope for release mode
					//ModeRate = _SPU->Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
					ModeRate = pChRegs0->ADSR_1;
					
					//Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
					Start_VolumeEnvelope ( (s16&) pChRegs0->ENV_X, Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
					
					// loop address not manually specified
					// can still change loop address after sound starts
					//LSA_Manual_Bitmap &= ~( 1 << Channel );
				}
			}
#endif
			
			break;
			
		case KOFF_1:
			/////////////////////////////////////////////
			// Key off 16-23
			
#ifdef KEY_ON_OFF_LOOP2
			// on key off we need to change ADSR mode to release mode
			Temp = Data;
			Temp &= 0xff;
			while ( Temp )
			{
				Channel = Temp & ( -Temp );
				Temp ^= Channel;
				//Channel = 16 + __builtin_ctz( Channel );
				Channel = 16 + ctz32(Channel);

#ifdef VERBOSE_KEYONOFF_TEST
				if ( ( CycleCount - StartCycle_Channel [ Channel ] ) < 4 )
				{
					// check if the key-off signal was given within 2T or 3T
					cout << "\nhps1x64: SPU2: ALERT: Channel#" << dec << Channel << " key-off after " << ( CycleCount - StartCycle_Channel [ Channel ] ) << "T";
				}
#endif
				
				pChRegs0 = (ChRegs0_Layout*) ( & ( pCoreRegs0->ChRegs0 [ Channel ] ) );
				//pChRegs1 = (ChRegs1_Layout*) ( & ( pCoreRegs0->ChRegs1 [ Channel ] ) );
				
				// put channel in adsr release phase unconditionally
				ADSR_Status [ Channel ] = ADSR_RELEASE;
				
				// *** TODO *** remove VOL_ADSR_Value variable
				// start envelope for release mode
				//ModeRate = _SPU->Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
				ModeRate = pChRegs0->ADSR_1;
				
				//Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
				Start_VolumeEnvelope ( (s16&) pChRegs0->ENV_X, Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
				
				// loop address not manually specified
				// can still change loop address after sound starts
				//LSA_Manual_Bitmap &= ~( 1 << Channel );
			}
#else
			// on key off we need to change ADSR mode to release mode
			for ( Channel = 16; Channel < 24; Channel++ )
			{
				if ( ( 1 << ( Channel - 16 ) ) & Data )
				{
					pChRegs0 = (ChRegs0_Layout*) ( & ( pCoreRegs0->ChRegs0 [ Channel ] ) );
					
					// put channel in adsr release phase unconditionally
					ADSR_Status [ Channel ] = ADSR_RELEASE;
					
					// *** TODO *** remove VOL_ADSR_Value variable
					// start envelope for release mode
					//ModeRate = _SPU->Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
					ModeRate = pChRegs0->ADSR_1;
					
					//Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
					Start_VolumeEnvelope ( (s16&) pChRegs0->ENV_X, Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
					
					// loop address not manually specified
					// can still change loop address after sound starts
					//LSA_Manual_Bitmap &= ~( 1 << Channel );
				}
			}
#endif
			
			break;

			
		case CTRL:

			// check if spu is turned off
			if ( ! ( Data & 0x8000 ) )
			{
				DecodeBufferOffset = 0;
				pCoreRegs0->STAT = 0;

				pCoreRegs0->STAT = 0;
				pCoreRegs0->ADMAS = 0;
				//pCoreRegs0->CON_0 = 0xffff;
				//pCoreRegs0->CON_1 = 0xff;
				pCoreRegs0->KON = 0;
				pCoreRegs0->KOFF = 0;
				pCoreRegs0->NON = 0;
				pCoreRegs0->PMON = 0;
				//pCoreRegs0->IRQA_0 = 0x400;
				//pCoreRegs0->IRQA_1 = 0;

				// the other bits shouldn't matter?
				Data = 0;
			}

			// check if SPU is reset
			if ( ( Data & 0x8000 ) && !( pCoreRegs0->CTRL & 0x8000 ) )
			{
				pCoreRegs0->STAT = 0;
				pCoreRegs0->CON_0 = 0xffff;
				pCoreRegs0->CON_1 = 0xff;
				pCoreRegs0->KON = 0;
				pCoreRegs0->KOFF = 0;
				//pCoreRegs0->ADMAS = 0;
				//pCoreRegs0->NON = 0;
				//pCoreRegs0->PMON = 0;
				//pCoreRegs0->IRQA_0 = 0x500;
				//pCoreRegs0->IRQA_1 = 0;
			}
			
			// check if interrupt transitions from disabled to enabled
			if ( ~Data & pCoreRegs0->CTRL & 0x40 )
			{
				// clear IRQ INFO
				// for just the core ??
				GET_REG16 ( SPDIF_IRQINFO ) &= ~( 0x4 << CoreNumber );
			}
			
			
			// bits 0-5 of CTRL should be applied to bits 0-5 of STAT (***TODO*** supposed to be delayed application)
			// source: Martin Korth PSX Specification
			pCoreRegs0->STAT &= ~0x3f;
			pCoreRegs0->STAT |= ( Data & 0x3f );
			
			// bit 7 of STAT appears to be bit 5 of CTRL (***TODO*** supposed to be delayed application)
			// source: Martin Korth PSX Specification
			//GET_REG16 ( STAT ) &= ~( 1 << 7 );
			//GET_REG16 ( STAT ) |= ( ( Data << 2 ) & ( 1 << 7 ) );
			// copy bit 5 of ctrl to bit 7 of stat
			//switch ( ( _SPU->Regs [ ( CTRL - SPU_X ) >> 1 ] >> 5 ) & 0x3 )
			switch ( ( Data >> 4 ) & 0x3 )
			{
				// no reads or writes (stop)
				case 0:
				
				// manual write
				case 1:	
				
					//_SPU->Regs [ ( STAT - SPU_X ) >> 1 ] = ( _SPU->Regs [ ( STAT - SPU_X ) >> 1 ] & ~0x0380 ) | ( 0 << 7 );
					pCoreRegs0->STAT &= ~0x0380;
					break;
					
				case 2:
					// dma write
					//_SPU->Regs [ ( STAT - SPU_X ) >> 1 ] = ( _SPU->Regs [ ( STAT - SPU_X ) >> 1 ] & ~0x0380 ) | ( 0x3 << 7 );
					pCoreRegs0->STAT = ( pCoreRegs0->STAT & ~0x0380 ) | ( 0x3 << 7 );
					break;
					
				case 3:
					// dma read
					//_SPU->Regs [ ( STAT - SPU_X ) >> 1 ] = ( _SPU->Regs [ ( STAT - SPU_X ) >> 1 ] & ~0x0380 ) | ( 0x5 << 7 );
					pCoreRegs0->STAT = ( pCoreRegs0->STAT & ~0x0380 ) | ( 0x5 << 7 );
					break;
			}
			
			// check if disabling/acknowledging interrupt
			if ( ! ( Data & 0x40 ) )
			{
				// clear interrupt
				pCoreRegs0->STAT &= ~0x40;
			}

			if ( ( ( Data & 0x30 ) == 0x20 ) || ( ( Data & 0x30 ) == 0x30 ) )
			{
				// dma condition has changed, update transfers //
				Dma::_DMA->Update_ActiveChannel ();
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
				for ( int i = 0; i < BufferIndex; i++ )
				{
					//RAM [ ( ( NextSoundBufferAddress + ( i << 1 ) ) & c_iRam_Mask ) >> 1 ] = Buffer [ i ];
					RAM [ ( NextSoundBufferAddress + i ) & ( c_iRam_Mask >> 1 ) ] = Buffer [ i ];
				}
				
				//////////////////////////////////////////////////////////
				// update next sound buffer address
				//NextSoundBufferAddress += ( BufferIndex << 1 );
				NextSoundBufferAddress += BufferIndex;
				
				// align to sound block boundary
				NextSoundBufferAddress &= ~7;
				
				//////////////////////////////////////////////////
				// reset buffer index
				BufferIndex = 0;
				
				////////////////////////////////////////////////////////////
				// save back into the sound buffer address register
				// sound buffer address register does not change
				//_SPU->Regs [ ( ( REG_SoundBufferAddress - SPU_X ) >> 1 ) & 0xff ] = (u16) (_SPU->NextSoundBufferAddress >> 3);
			}
			
			break;
			
			
		case CON_0:
		case CON_1:
			// writes should clear register to zero
			Data = 0;
			break;
			
		case ADMAS:
		
			//ulADMAS_Output = Data;
			
			// check for transition to zero
			/*
			if ( !( Data & 0xffff ) )
			{
				// clear the ADMA transfer offset for new transfers
				ulADMA_Offset8 = 0;
				
				ulADMA_StartCycle = DecodeBufferOffset;
				ulADMA_PlayOffset = 0;
				bBufferFull [ 0 ] = 0;
				bBufferFull [ 1 ] = 0;
				ulADMA_TransferOffset = 0;
			}
			*/
			
			// check for transition to one
			if ( ( Data & ( 1 << CoreNumber ) ) && !( pCoreRegs0->ADMAS & ( 1 << CoreNumber ) ) )
			{
				// dma condition has changed, update transfers //
				Dma::_DMA->Update_ActiveChannel ();

				ulADMA_PlayOffset = 0;

				/*
				ulADMA_StartCycle = DecodeBufferOffset;
				ulADMA_PlayOffset = 0;
				bBufferFull [ 0 ] = 0;
				bBufferFull [ 1 ] = 0;
				ulADMA_TransferOffset = 0;
				*/
			}
			
			break;
			
		case MVOL_L:
			if ( Data >> 15 )
			{
				Start_VolumeEnvelope ( pCoreRegs1->CMVOL_L, MVOL_L_Cycles, Data & 0x7f, ( Data >> 13 ) & 0x3 );
			}
			else
			{
				// just set the current master volume L
				pCoreRegs1->CMVOL_L = Data << 1;
			}
			break;
			
		case MVOL_R:
			if ( Data >> 15 )
			{
				Start_VolumeEnvelope ( pCoreRegs1->CMVOL_R, MVOL_R_Cycles, Data & 0x7f, ( Data >> 13 ) & 0x3 );
			}
			else
			{
				// just set the current master volume L
				pCoreRegs1->CMVOL_R = Data << 1;
			}
			break;
			
		default:
		

			// this one needs to check ROffset or else it will skip Core#1 regs
			if ( ROffset < 0x180 )
			{

				if ( ( ( Offset >> 1 ) & 7 ) < 2 )
				{
#if defined INLINE_DEBUG_WRITE
	debug << " START-VE";
#endif
					u32 Ch = ( Offset >> 4 ) & 0x1f;
					
					pChRegs0 = (ChRegs0_Layout*) ( & ( pCoreRegs0->ChRegs0 [ Ch ] ) );
					//pChRegs1 = (ChRegs1_Layout*) ( & ( pCoreRegs0->ChRegs1 [ Ch ] ) );
					
					if ( ! ( Offset & 2 ) )
					{
#if defined INLINE_DEBUG_WRITE
	debug << "-VOL_L";
#endif

						// vol l
						if ( Data >> 15 )
						{
							Start_VolumeEnvelope ( pChRegs0->CVOL_L, VOL_L_Cycles [ Ch ], Data & 0x7f, ( Data >> 13 ) & 0x3 );
						}
						else
						{
							pChRegs0->CVOL_L = Data << 1;
						}
					}
					else
					{
#if defined INLINE_DEBUG_WRITE
	debug << "-VOL_R";
#endif

						// vol r
						if ( Data >> 15 )
						{
							Start_VolumeEnvelope ( pChRegs0->CVOL_R, VOL_R_Cycles [ Ch ], Data & 0x7f, ( Data >> 13 ) & 0x3 );
						}
						else
						{
							pChRegs0->CVOL_R = Data << 1;
						}
					}

				}	// end if ( ( ( Offset >> 1 ) & 7 ) < 2 )
				
			}	// end if ( ROffset < 0x180 )
			else if ( ( ROffset >= 0x1c0 ) && ( ROffset < 0x2e0 ) )
			{
				if ( ( ( ( ROffset - 0x1c0 ) >> 1 ) % 6 ) & 2 )
				{
					// LSA //

					u32 Ch = ( ( ( ROffset - 0x1c0 ) >> 1 ) / 6 );
					
					//pChRegs1 = (ChRegs1_Layout*) ( & ( pCoreRegs0->ChRegs1 [ Ch ] ) );

					// if before 4T or so, then ignore LSA setting
					if ( ( CycleCount - StartCycle_Channel [ Ch ] ) < 4 )
					{
						return;
					}

					// loop was manually set, so ignore setting in wave data
					bLoopManuallySet |= ( 1 << Ch );
				}
			}
			
			break;
	}
	
	// set value
	//Regs16 [ lReg ] = Data;
	GET_REG16 ( Offset ) = Data;
}



void SPUCore::UpdatePitch ( int Channel, u32 Pitch, u32 Reg_PMON, s32 PreviousSample )
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


void SPU2::Start_Thread ()
{
	int i;
	int iRet;

	
	// keep track of how many threads were last created
	ulNumberOfThreads_Created = 0;
	

	if ( _SPU2->ulNumThreads )
	{
		//cout << "\nGPU::Start_Frame";
		
		// create event for triggering gpu thread UPDATE
		ghEvent_Update = CreateEvent(
			NULL,			// default security
			false,			// auto-reset event (true for manually reset event)
			false,			// initially not set
			NULL			// name of event object
		);
		
		if ( !ghEvent_Update )
		{
			cout << "\nERROR: Unable to create PS2 SPU2 UPDATE event. " << GetLastError ();
		}
		
		/*
		// create event for triggering/allowing next FRAME
		ghEvent_PS2GPU_Frame = CreateEvent(
			NULL,
			false,
			true,
			NULL
		);
		
		if ( !ghEvent_PS2GPU_Frame )
		{
			cout << "\nERROR: Unable to create PS2 GPU FRAME event. " << GetLastError ();
		}
		
		// create event to trigger FINISH
		ghEvent_PS2GPU_Finish = CreateEvent(
			NULL,
			true,
			true,
			NULL
		);
		
		if ( !ghEvent_PS2GPU_Finish )
		{
			cout << "\nERROR: Unable to create PS2 GPU FINISH event. " << GetLastError ();
		}
		*/
		
		// reset write index
		ullThWriteIdx = 0;
		
		// clear read index
		// the other thread does this
		//ullThReadIdx = 0;
		
		// want thread to run for now
		Lock_Exchange64 ( (long long&) ullThTargetIdx, 0 );


		for ( i = 0; i < _SPU2->ulNumThreads; i++ )
		{
			//cout << "\nCreating GPU thread#" << dec << i;
			
			// create thread
			Threads [ i ] = new Api::Thread( Run_Thread, (void*) NULL, false );
			
			//cout << "\nCreated GPU thread#" << dec << i << " ThreadStarted=" << GPUThreads[ i ]->ThreadStarted;
			
			
			ulNumberOfThreads_Created++;
			
//cout << "\nCREATING: NEW GPU THREAD: " << dec << ulNumberOfThreads_Created;
		}
		
	}

}

void SPU2::End_Thread ()
{
	int i;
	int iRet;
	
//cout << "\nEND FRAME";
	
	if ( ulNumberOfThreads_Created )
	{
		// want thread to stop for now
		Lock_Exchange64 ( (long long&) ullThTargetIdx, -1ull );
		
		// trigger event
		if ( !SetEvent ( ghEvent_Update ) )
		{
			cout << "\nUnable to set PS2 SPU2 UPDATE event. " << GetLastError ();
		}


		for ( i = 0; i < ulNumberOfThreads_Created; i++ )
		{
			//cout << "\nKilling GPU thread#" << dec << i << " ThreadStarted=" << GPUThreads[ i ]->ThreadStarted;
			
			// remove thread
			iRet = Threads [ i ]->Join();
			
			//cout << "\nThreadStarted=" << GPUThreads[ i ]->ThreadStarted;
			
			if ( iRet )
			{
				cout << "\nhps1x64: SPU2: ALERT: Problem with completion of SPU thread#" << dec << i << " iRet=" << iRet;
			}
			
			delete Threads [ i ];
			
			//cout << "\nKilled GPU thread#" << dec << i << " iRet=" << iRet;
		}
		
		
		ulNumberOfThreads_Created = 0;
		
		// remove the events for now
		//CloseHandle ( ghEvent_PS2GPU_Finish );
		//CloseHandle ( ghEvent_PS2GPU_Frame );
		CloseHandle ( ghEvent_Update );
	}

}

int SPU2::Run_Thread( void* Param )
{
	u64 ullIdx, ullSize;
	u64 ullTargetIdx;

	// thread is running
	ullThreadRunning = 1;

	// init read index
	ullThReadIdx = 0;

	ullTargetIdx = 0;

	while ( 1 )
	{
		while ( ullThReadIdx >= ullTargetIdx )
		{
			WaitForSingleObject( ghEvent_Update, INFINITE );

			// read value from the other thread
			ullTargetIdx = Lock_Exchange64 ( (long long&) ullThTargetIdx, 0 );

			// check if killing thread
			if ( ullTargetIdx == -1ull )
			{
				// thread is NOT running
				ullThreadRunning = 0;

				// done
				return 0;
			}
		}

		ullSize = ullLastSize [ ullThReadIdx & c_ullThBuffer_Mask ];
		ullIdx = ullLastWriteIdx [ ullThReadIdx & c_ullThBuffer_Mask ];

		// run offloaded spu2 backend
		_SPU2->Backend_MixSamples ( ullIdx, ullSize );
						
#ifndef ENABLE_THREAD_LOCK
		// clear command data when done
		_mm_stream_si128 ( (__m128i *) & ( p_inputdata [ 14 ] ), _mm_setzero_si128 () );
#endif
		
		ullThReadIdx++;

	}

	// thread is NOT running
	ullThreadRunning = 0;

	// done
	return 0;
}


void SPU2::Run ()
{
	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;


	s64 SampleL, SampleR;
	
#ifdef INLINE_DEBUG_SPU2RUN
	debug << "\r\nSPU2RUN:";
	debug << " Cycle#" << dec << *_DebugCycleCount;
	
	/*
	debug << " SPU1Mode=" << SPU1.isADMATransferMode ();
	debug << " SPU0Mode=" << SPU0.isADMATransferMode ();
	debug << " Enabled7=" << Dma::_DMA->isEnabledAndActive ( 7 );
	debug << " Enabled4=" << Dma::_DMA->isEnabledAndActive ( 4 );
	debug << " Offset7=" << ( SPU1.DecodeBufferOffset & 0x1ff );
	debug << " Offset4=" << ( SPU0.DecodeBufferOffset & 0x1ff );
	*/
#endif


	SPU0.Run ();
	
#ifdef ENABLE_AVOL_CALC
	// ***todo*** multiply SPU0 Output by input volume of SPU1
	//SPU0.SampleL = ( SPU0.SampleL * ( (s32) SPU1.pCoreRegs1->AVOL_L ) ) >> 15;
	//SPU0.SampleR = ( SPU0.SampleR * ( (s32) SPU1.pCoreRegs1->AVOL_R ) ) >> 15;
#endif
	
	//SampleL = SPU0.SampleL + SPU1.SampleL;
	//SampleR = SPU0.SampleR + SPU1.SampleR;
	
	
	SPU1.Run ();

	
	
	
/*
#ifdef ENABLE_AUTODMA_RUN
	// check if starting a new buffer half (each buffer half is 512 bytes or 256 samples)
	// ***note*** this should probably go under SPU2::Run since I might need to change order that DMA 4/7 gets run in
	
	
	
	if ( Dma::_DMA->isEnabledAndActive ( 7 ) )
	{
		// check if in ADMA transfer mode
		// the decode buffer offset is counting bytes and not samples, but each half of buffer is 256 samples
		//if ( ( SPU1.DecodeBufferOffset & 0x1ff ) == 0 && SPU1.isADMATransferMode () )
		if ( ( SPU1.ulADMA_PlayOffset & 0x1ff ) == 0 && SPU1.isADMATransferMode () )
		{
#ifdef INLINE_DEBUG_SPU2RUN
	debug << "\r\nDMA7 ADMA:";
	debug << " BA=" << hex << Dma::_DMA->DmaCh [ 7 ].BCR.BA;
	debug << " Cycle#" << dec << *_DebugCycleCount;
#endif

			// check if DMA 4/7 is enabled and active
			// if DMA 4/7 is enabled and active, then invoke transfer
			
			//cout << "\nhps2x64: SPU2: Attempting AutoDma7 transfer";
			
#ifndef ENABLE_IMMEDIATE_ADMA
			SPU1.SoundDataInput_Enable = true;
			SPU1.SoundDataInput_Offset = 0;
#endif
			
			//Dma::_DMA->AutoDMA7_Run ();
			//Dma::_DMA->DMA7_Run ( false );
			//Dma::_DMA->Transfer ( 7 );
			// update the dma state since external conditions have changed
			Playstation1::Dma::_DMA->Update_ActiveChannel ();
			
		}
	}
#ifdef ENABLE_EXCLUSIVE_AUTODMA
	else
#endif
	if ( Dma::_DMA->isEnabledAndActive ( 4 ) )
	{
		// check if in ADMA transfer mode
		//if ( ( SPU0.DecodeBufferOffset & 0x1ff ) == 0 && SPU0.isADMATransferMode () )
		if ( ( SPU0.ulADMA_PlayOffset & 0x1ff ) == 0 && SPU0.isADMATransferMode () )
		{
#ifdef INLINE_DEBUG_SPU2RUN
	debug << "\r\nDMA4 ADMA:";
	debug << " BA=" << hex << Dma::_DMA->DmaCh [ 4 ].BCR.BA;
	debug << " Cycle#" << dec << *_DebugCycleCount;
#endif

			// check if DMA 4/7 is enabled and active
			// if DMA 4/7 is enabled and active, then invoke transfer
			
			//cout << "\nhps2x64: SPU2: Attempting AutoDma4 transfer";
			
#ifndef ENABLE_IMMEDIATE_ADMA
			SPU0.SoundDataInput_Enable = true;
			SPU0.SoundDataInput_Offset = 0;
#endif
			
			//Dma::_DMA->AutoDMA4_Run ();
			//Dma::_DMA->DMA4_Run ( false );
			//Dma::_DMA->Transfer ( 4 );
			// update the dma state since external conditions have changed
			Playstation1::Dma::_DMA->Update_ActiveChannel ();
			
		}
	}
		
#endif
*/


/*
#ifdef INLINE_DEBUG_RUN2
	if ( SPU1.pCoreRegs0->CTRL >> 15 )
	{
	// show core#1 current block address for first 4 channels
	debug << " BlockAddr=" << hex << SPU1.CurrentBlockAddress [ 0 ] << " " << SPU1.CurrentBlockAddress [ 1 ] << " " << SPU1.CurrentBlockAddress [ 2 ] << " " << SPU1.CurrentBlockAddress [ 3 ];
	}
#endif
*/


	
	// mix samples
	Mixer [ ( Mixer_WriteIdx + 0 ) & c_iMixerMask ] = adpcm_decoder::clamp ( SPU1.SampleL );
	Mixer [ ( Mixer_WriteIdx + 1 ) & c_iMixerMask ] = adpcm_decoder::clamp ( SPU1.SampleR );
	
	// samples is now in mixer
	Mixer_WriteIdx += 2;
	
	// output one l/r sample
	if ( AudioOutput_Enabled && ( Mixer_WriteIdx - Mixer_ReadIdx ) >= PlayBuffer_Size /*c_iPlaySize*/ )
	{
		if ( ulNumThreads )
		{
			ullLastWriteIdx [ ullThWriteIdx & c_ullThBuffer_Mask ] = Mixer_WriteIdx;
			ullLastSize [ ullThWriteIdx & c_ullThBuffer_Mask ] = PlayBuffer_Size;

			ullThWriteIdx++;

			x64ThreadSafe::Utilities::Lock_Exchange64 ( (long long&) ullThTargetIdx, ullThWriteIdx );

			// trigger event
			if ( !SetEvent ( ghEvent_Update ) )
			{
				cout << "\nUnable to set PS2 SPU UPDATE event. " << GetLastError ();
			}

			/*
			u64 arrTemp [ 2 ];
			u64 *p_inputdata = & ( Playstation2::GPU::inputdata [ ( Playstation2::GPU::ulInputBuffer_WriteIndex & Playstation2::GPU::c_ulInputBuffer_Mask ) << Playstation2::GPU::c_ulInputBuffer_Shift ] );
			
#ifdef ENABLE_THREAD_LOCK_DATA
			p_inputdata [ 15 ] = 7;
			p_inputdata [ 14 ] = 7;
			p_inputdata [ 0 ] = Mixer_WriteIdx;
			p_inputdata [ 1 ] = PlayBuffer_Size;
#else
			arrTemp [ 0 ] = Mixer_WriteIdx;
			arrTemp [ 1 ] = PlayBuffer_Size;
			_mm_stream_si128 ( (__m128i const*) & ( p_inputdata [ 0 ] ), _mm_loadu_si128 ( (__m128i const*) arrTemp ) );
			
			arrTemp [ 1 ] = 7;
			arrTemp [ 0 ] = 7;
#ifndef ENABLE_THREAD_LOCK
			x64ThreadSafe::Utilities::Store_Fence ();
#endif

			_mm_stream_si128 ( (__m128i const*) & ( p_inputdata [ 14 ] ), _mm_loadu_si128 ( (__m128i const*) arrTemp ) );
#endif
			
			Playstation2::GPU::ulInputBuffer_WriteIndex++;
			//x64ThreadSafe::Utilities::Lock_ExchangeAdd64 ( (long long&) Playstation2::GPU::ulInputBuffer_WriteIndex, 1 );
			*/

		}
		else
		{
			Backend_MixSamples ( Mixer_WriteIdx, PlayBuffer_Size );
		}
		
		// check if the buffer size changed
		if ( PlayBuffer_Size != NextPlayBuffer_Size )
		{
			PlayBuffer_Size = NextPlayBuffer_Size;
		}

		// data in mixer has been played now
		Mixer_ReadIdx = Mixer_WriteIdx;
	}
	
	
	// update number of spu cycles ran
	CycleCount++;
	
	//NextEvent_Cycle = *_DebugCycleCount + CPUCycles_PerSPUCycle;
	Set_NextEvent ( CPUCycles_PerSPUCycle );
	
#ifdef INLINE_DEBUG_RUN2
	if ( SPU1.pCoreRegs0->CTRL >> 15 )
	{
	debug << "\r\nSPU2::Run Cycle#" << dec << *_DebugCycleCount << " NextEvent_Cycle=" << NextEvent_Cycle;
	}
#endif

}



void SPU2::Backend_MixSamples ( u32 WriteIdx, u32 Size )
{
	int testvalue0, testvalue1;
	
	u32 ReadIdx;
	
	//if ( !hWaveOut ) cout << "\n!hWaveOut; p1\n";
	
	// make sure the read index at write index minus the size of the buffer
	ReadIdx = WriteIdx - Size;
	
	if ( ( WriteIdx - ReadIdx ) == Size )
	{
		// play buffer cannot hold any more data //
		
		while ( !( header0.dwFlags & WHDR_DONE ) && !( header1.dwFlags & WHDR_DONE ) )
		{
			//cout << "\nWaiting for samples to finish playing...";
			
			testvalue0 = waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
			testvalue1 = waveOutUnprepareHeader( hWaveOut, &header1, sizeof(WAVEHDR) );
			
		}
	}
	
	if ( header0.dwFlags & WHDR_DONE )
	{
#ifdef INLINE_DEBUG_CDSOUND
	//debug << "\r\nPlaying; Mixer_WriteIdx=" << dec << Mixer_WriteIdx << " Mixer_ReadIdx=" << dec << Mixer_ReadIdx;
#endif

		ZeroMemory( &header0, sizeof(WAVEHDR) );
		
		//if ( !hWaveOut ) cout << "\n!hWaveOut; p5\n";
		
		// this must be the size in bytes
		header0.dwBufferLength = ( WriteIdx - ReadIdx ) * 2;
		
		// copy samples to play into the play buffer
		for ( int i = 0; i < ( WriteIdx - ReadIdx ); i++ ) PlayBuffer0 [ i ] = Mixer [ ( i + ReadIdx ) & c_iMixerMask ];
		
		//if ( !hWaveOut ) cout << "\n!hWaveOut; p6\n";
		
		header0.lpData = (char*) PlayBuffer0;

		if ( AudioOutput_Enabled )
		{
			testvalue0 = waveOutPrepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
			
			
			testvalue0 = waveOutWrite( hWaveOut, &header0, sizeof(WAVEHDR) );
			//cout << "\nwaveOutwrite testvalue0=" << hex << testvalue0 << "\n";
			
			
			//cout << "\nSent enabled audio successfully.";
		}
		
		// data in mixer has been played now
		//Mixer_ReadIdx = Mixer_WriteIdx;
	}
	//else if ( testvalue1 == MMSYSERR_NOERROR )
	else if ( header1.dwFlags & WHDR_DONE )
	{
		ZeroMemory( &header1, sizeof(WAVEHDR) );
		
		//if ( !hWaveOut ) cout << "\n!hWaveOut; p5\n";
		
		// this must be the size in bytes
		header1.dwBufferLength = ( WriteIdx - ReadIdx ) * 2;
		
		// copy samples to play into the play buffer
		for ( int i = 0; i < ( WriteIdx - ReadIdx ); i++ ) PlayBuffer1 [ i ] = Mixer [ ( i + ReadIdx ) & c_iMixerMask ];
		
		//if ( !hWaveOut ) cout << "\n!hWaveOut; p6\n";
		
		header1.lpData = (char*) PlayBuffer1;

		if ( AudioOutput_Enabled )
		{
			testvalue1 = waveOutPrepareHeader( hWaveOut, &header1, sizeof(WAVEHDR) );
			
			
			testvalue1 = waveOutWrite( hWaveOut, &header1, sizeof(WAVEHDR) );
			//cout << "\nwaveOutwrite testvalue1=" << hex << testvalue1 << "\n";
			
			
			//cout << "\nSent enabled audio successfully.";
		}
		
		// data in mixer has been played now
		//Mixer_ReadIdx = Mixer_WriteIdx;
	}


}



void SPUCore::Run ()
{
	int Channel;
	
	//s64 SampleL, SampleR;
	s64 Sample;

	// ***todo*** uninitialized variable issue
	s64 PreviousSample = 0;

	s64 ChSampleL, ChSampleR;
	
	// ***todo*** uninitialized variable issues - also was using these on startup ??
	s64 CD_SampleL = 0;
	s64 CD_SampleR = 0;
	
	s64 ReverbSampleL, ReverbSampleR;
	s64 FOutputL, FOutputR, ROutputL, ROutputR;

	
	u32 ChannelOn, ChannelNoise, PitchMod, ReverbOn;
	
	u32 ReverbOnL, ReverbOnR;
	u32 SoundOnL, SoundOnR;
	
	u64 Temp;
	u32 ModeRate;
	
	ChRegs0_Layout *pChRegs0;
	ChRegs1_Layout *pChRegs1;

	u32 ulDecodeAddr, ulIrqAddr0, ulIrqAddr1;

	
	// update number of spu cycles ran
	// I'll do this per core for now
	CycleCount++;
	
	
//#ifdef INLINE_DEBUG_RUN
//	debug << "\r\nSPU::Run";
//#endif

	// initialize current sample for left and right
	SampleL = 0;
	SampleR = 0;
	ReverbSampleL = 0;
	ReverbSampleR = 0;

	
	/////////////////////////////////////////////////////////////////////
	// get what channels are enabled
	ChannelOn = pCoreRegs0->CON;
	
	// get what channels are set to noise
	ChannelNoise = pCoreRegs0->NON;
	
	// get what channels are using frequency modulation
	PitchMod = pCoreRegs0->PMON;
	
	// SPU on ps2 has new flags for sending sound data from channel to mixer
	// can send to four mixers, DryL, DryR, WetL, WetR
	// dry mixers do not apply effects, wet mixers apply effects
	
	// get what channels have reverb on
	ReverbOnL = pCoreRegs0->VMIXEL;
	ReverbOnR = pCoreRegs0->VMIXER;
	
	// there's also another flag for sending dry channels to the dry mixers
	SoundOnL = pCoreRegs0->VMIXL;
	SoundOnR = pCoreRegs0->VMIXR;

//#ifdef INLINE_DEBUG_RUN
//	debug << "; ChannelOn=" << ChannelOn << " KeyOn=" << KeyOn;
//#endif

	// if spu is enabled, run noise generator
	//if ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] >> 15 )
	if (pCoreRegs0->CTRL >> 15)
	{
		// can put this line on the other thread when multi-threading
		RunNoiseGenerator();

		if (pCoreRegs1->MVOL_L >> 15)
		{
			// ***TODO*** get rid of MVOL_L_Value variable

			MVOL_L_Value = (s64)((s16)pCoreRegs1->CMVOL_L);

			VolumeEnvelope(pCoreRegs1->CMVOL_L, MVOL_L_Cycles, pCoreRegs1->MVOL_L & 0x7f, (pCoreRegs1->MVOL_L >> 13) & 0x3, true);
		}
		else
		{
			// just set the current master volume L
			pCoreRegs1->CMVOL_L = pCoreRegs1->MVOL_L << 1;

		}	// end else if ( pCoreRegs1->MVOL_L >> 15 )

		if (pCoreRegs1->MVOL_R >> 15)
		{
			// ***TODO*** get rid of MVOL_R_Value variable

			MVOL_R_Value = (s64)((s16)pCoreRegs1->CMVOL_R);

			VolumeEnvelope(pCoreRegs1->CMVOL_R, MVOL_R_Cycles, pCoreRegs1->MVOL_R & 0x7f, (pCoreRegs1->MVOL_R >> 13) & 0x3, true);
		}
		else
		{
			// just set the current master volume R
			pCoreRegs1->CMVOL_R = pCoreRegs1->MVOL_R << 1;

		}	// end else if ( pCoreRegs1->MVOL_R >> 15 )


		// note: at this point can save non-channel vars for multi-threading


		// also process audio if SPU is on
		////////////////////////////
		// loop through channels
		for (Channel = 0; Channel < 24; Channel++)
		{
			pChRegs0 = (ChRegs0_Layout*)(&(pCoreRegs0->ChRegs0[Channel]));
			pChRegs1 = (ChRegs1_Layout*)(&(pCoreRegs0->ChRegs1[Channel]));

			// ***TODO*** channel not supposed to start for like 2T probably ??
			// for now, will just check for spu interrupt at 2T mark
			if ((CycleCount - StartCycle_Channel[Channel]) == 2)
			{
				if (SPU2::_SPU2->SPU0.pCoreRegs0->CTRL & 0x40)
				{
					if ((CurrentBlockAddress[Channel] & (c_iRam_Mask >> 1)) == ((SWAPH(SPU2::_SPU2->SPU0.pCoreRegs0->IRQA)) & (c_iRam_Mask >> 1)))
					{
#ifdef INLINE_DEBUG_INT
						debug << "\r\nSPU:INT:ADDR ch#" << dec << Channel;
#endif
						// we have reached irq address - trigger interrupt
						SetInterrupt();

						// do this for ps2
						//SetInterrupt_Core ( CoreNumber );
						SetInterrupt_Core(0);

						// interrupt
						//pCoreRegs0->STAT |= 0x40;
						SPU2::_SPU2->SPU0.pCoreRegs0->STAT |= 0x40;
					}

				}	// end if ( SPU2::_SPU2->SPU0.pCoreRegs0->CTRL & 0x40 )

				if (SPU2::_SPU2->SPU1.pCoreRegs0->CTRL & 0x40)
				{
					if ((CurrentBlockAddress[Channel] & (c_iRam_Mask >> 1)) == ((SWAPH(SPU2::_SPU2->SPU1.pCoreRegs0->IRQA)) & (c_iRam_Mask >> 1)))
					{
#ifdef INLINE_DEBUG_INT
						debug << "\r\nSPU:INT:ADDR ch#" << dec << Channel;
#endif

						// we have reached irq address - trigger interrupt
						SetInterrupt();

						// do this for ps2
						//SetInterrupt_Core ( CoreNumber );
						SetInterrupt_Core(1);

						// interrupt
						//pCoreRegs0->STAT |= 0x40;
						SPU2::_SPU2->SPU1.pCoreRegs0->STAT |= 0x40;
					}

				}	// end if ( SPU2::_SPU2->SPU1.pCoreRegs0->CTRL & 0x40 )

			}	// end if ( ( CycleCount - StartCycle_Channel [ Channel ] ) == 2 )


			if (pChRegs0->VOL_L >> 15)
			{
#ifdef INLINE_DEBUG_RUN_VOLUME_ENVELOPE
				debug << "\r\nChannel#" << dec << Channel;
				debug << " CORE#" << CoreNumber;
				debug << " VE-VOLL";
				debug << " CVOL=" << hex << pChRegs0->CVOL_L;
				debug << " CYCLES=" << hex << VOL_L_Cycles[Channel];
				debug << " VALUE=" << hex << (pChRegs0->VOL_L & 0x7f);
				debug << " MODE=" << ((pChRegs0->VOL_L >> 13) & 0x3);
#endif

				// *** TODO *** VOL_L_Value variable could be removed

				// set current volume left
				//VOL_L_Value [ Channel ] = (s64) ( (s16) Regs [ ( CVOL_L_START - SPU_X + ( Channel << 1 ) ) >> 1 ] );
				VOL_L_Value[Channel] = (s64)pChRegs0->CVOL_L;

				// perform envelope
				//VolumeEnvelope ( VOL_L_Value [ Channel ], VOL_L_Cycles [ Channel ], Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ] & 0x7f, ( Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ] >> 13 ) & 0x3 );
				VolumeEnvelope(pChRegs0->CVOL_L, VOL_L_Cycles[Channel], pChRegs0->VOL_L & 0x7f, (pChRegs0->VOL_L >> 13) & 0x3, true);

				// store the new current volume left
				//Regs [ ( CVOL_L_START - SPU_X + ( Channel << 1 ) ) >> 1 ] = VOL_L_Value [ Channel ];
				//GET_REG16 ( CVOLL_CH ( Channel ) ) = VOL_L_Value [ Channel ];
			}
			else
			{
				// just set the current volume L
				pChRegs0->CVOL_L = pChRegs0->VOL_L << 1;

			}	// end else if ( pChRegs0->VOL_L >> 15 )

			if (pChRegs0->VOL_R >> 15)
			{
#ifdef INLINE_DEBUG_RUN_VOLUME_ENVELOPE
				debug << "\r\nChannel#" << dec << Channel;
				debug << " CORE#" << CoreNumber;
				debug << " VE-VOLR";
				debug << " CVOL=" << hex << pChRegs0->CVOL_R;
				debug << " CYCLES=" << hex << VOL_R_Cycles[Channel];
				debug << " VALUE=" << hex << (pChRegs0->VOL_R & 0x7f);
				debug << " MODE=" << ((pChRegs0->VOL_R >> 13) & 0x3);
#endif

				// set current volume right
				//VOL_R_Value [ Channel ] = (s64) ( (s16) Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ] );
				VOL_R_Value[Channel] = (s64)pChRegs0->CVOL_R;

				//VolumeEnvelope ( VOL_R_Value [ Channel ], VOL_R_Cycles [ Channel ], Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ] & 0x7f, ( Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ] >> 13 ) & 0x3 );
				VolumeEnvelope(pChRegs0->CVOL_R, VOL_R_Cycles[Channel], pChRegs0->VOL_R & 0x7f, (pChRegs0->VOL_R >> 13) & 0x3, true);

				// store the new current volume right
				//Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ] = VOL_R_Value [ Channel ];
				//GET_REG16 ( CVOLR_CH ( Channel ) ) = VOL_R_Value [ Channel ];
			}
			else
			{
				// just set the current volume R
				pChRegs0->CVOL_R = pChRegs0->VOL_R << 1;

			}	// end else if ( pChRegs0->VOL_R >> 15 )


			/////////////////////////////////////////////////////////////////////
			// update ADSR envelope

#ifdef ENABLE_MUTE_CHANNEL_OPTIMIZATION
			// alot of this can be skipped if channel is muted
			if (ADSR_Status[Channel] == ADSR_MUTE)
			{
				// channel is set to mute //
				Sample = 0;
				ChSampleL = 0;
				ChSampleR = 0;

				// update sound progression
				UpdatePitch(Channel, pChRegs0->PITCH, PitchMod, PreviousSample);
			}
			else
#endif
			{
				// channel is NOT set to mute //

			// check adsr status
				switch (ADSR_Status[Channel])
				{
				case ADSR_MUTE:

					VOL_ADSR_Value[Channel] = 0;

					//Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ] = 0;
					pChRegs0->ENV_X = 0;

					break;

				case ADSR_ATTACK:
#ifdef INLINE_DEBUG_RUN_ATTACK
					debug << "\r\nCore#" << CoreNumber;
					debug << " Channel#" << dec << Channel;
#endif

#ifdef INLINE_DEBUG_RUN_ATTACK
					debug << "; Attack";
#endif

					///////////////////////////////////////////
					// ADSR - Attack Mode

					// saturate and switch to decay mode if volume goes above 32767
					if (pChRegs0->ENV_X >= 32767)
					{
#ifdef INLINE_DEBUG_RUN_ATTACK
						debug << "; DECAY_NEXT ENVX=" << hex << pChRegs0->ENV_X;
#endif

						//VOL_ADSR_Value [ Channel ] = 32767;
						pChRegs0->ENV_X = 32767;

						ADSR_Status[Channel] = ADSR_DECAY;

						// start envelope for decay mode
						//ModeRate = Regs [ ( ADSR_0 >> 1 ) + ( Channel << 3 ) ];
						ModeRate = pChRegs0->ADSR_0;

						//Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ( ModeRate >> 4 ) & 0xf ) << ( 2 ), 0x3 );
						Start_VolumeEnvelope((s16&)pChRegs0->ENV_X, Cycles[Channel], ((ModeRate >> 4) & 0xf) << (2), 0x3, true);

#ifdef INLINE_DEBUG_RUN_ATTACK
						debug << " ->ENVX=" << hex << pChRegs0->ENV_X;
#endif
					}
					else
					{

						////////////////////////////////////////////////////
						// linear or psx pseudo exponential increase

#ifdef INLINE_DEBUG_RUN_ATTACK
						debug << "; (before) VOL_ADSR_Value=" << hex << pChRegs0->ENV_X << "; Cycles=" << dec << Cycles[Channel];
#endif

						//ModeRate = Regs [ ( ADSR_0 >> 1 ) + ( Channel << 3 ) ];
						ModeRate = pChRegs0->ADSR_0;

						//VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 8 ) & 0x7f, ( ModeRate >> 15 ) << 1 );
						VolumeEnvelope((s16&)pChRegs0->ENV_X, Cycles[Channel], (ModeRate >> 8) & 0x7f, (ModeRate >> 15) << 1, false);

						if (pChRegs0->ENV_X >= 32767)
						{
							pChRegs0->ENV_X = 32767;
						}

#ifdef INLINE_DEBUG_RUN_ATTACK
						debug << "; (after) VOL_ADSR_Value=" << hex << pChRegs0->ENV_X << "; Cycles=" << dec << Cycles[Channel] << "; Value=" << hex << ((ModeRate >> 8) & 0x7f) << "; flags=" << ((ModeRate >> 15) << 1);
#endif
					}	// end else if (pChRegs0->ENV_X >= 32767)


					break;

				case ADSR_DECAY:
#ifdef INLINE_DEBUG_RUN_DECAY
					debug << "\r\nCore#" << CoreNumber;
					debug << " Channel#" << dec << Channel;
#endif

#ifdef INLINE_DEBUG_RUN_DECAY
					//debug << "; Decay; Rate=" << (((double)VOL_DECAY_Constant [ Channel ])/(1<<30));
					debug << "; Decay";
#endif

					////////////////////////////////////////////
					// ADSR - Decay Mode

					// switch to sustain mode if we reach sustain level
					if (((s32)((s16)pChRegs0->ENV_X)) <= ((s32)VOL_SUSTAIN_Level[Channel]))
					{
#ifdef INLINE_DEBUG_RUN_DECAY
						debug << "; SUSTAIN_NEXT";
#endif

						// ***TODO*** supposed to go under sustain level for 1 T before hitting sustain level?? //
						pChRegs0->ENV_X = VOL_SUSTAIN_Level[Channel];

						// maximize on the upper bound
						if (((u16)pChRegs0->ENV_X) > 0x7fff)
						{
							pChRegs0->ENV_X = 0x7fff;
						}

						ADSR_Status[Channel] = ADSR_SUSTAIN;

						// start envelope for sustain mode
						//ModeRate = Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
						ModeRate = pChRegs0->ADSR_1;

						//Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 6 ) & 0x7f, ModeRate >> 14 );
						Start_VolumeEnvelope((s16&)pChRegs0->ENV_X, Cycles[Channel], (ModeRate >> 6) & 0x7f, ModeRate >> 14, true);

						// ADSR1 -> bit 14 (0: sustain mode increasing, 1: sustain mode decreasing)
						if ((ModeRate >> 14) & 1)
						{
							// decreasing in sustain mode //

							// or below zero
							/*
							if ( ( (s16) pChRegs0->ENV_X ) < 0 )
							{
								pChRegs0->ENV_X = 0;
							}
							*/
						}
						else
						{
							// increasing in sustain mode //

							// saturate if volume goes above 32767
							if (pChRegs0->ENV_X > 32767)
							{
								pChRegs0->ENV_X = 32767;
							}

						}	// end else if ((ModeRate >> 14) & 1)
					}
					else
					{
						////////////////////////////////////////////////
						// Exponential decrease


#ifdef INLINE_DEBUG_RUN_DECAY
						debug << "; (before) VOL_ADSR_Value=" << hex << pChRegs0->ENV_X << "; Cycles=" << dec << Cycles[Channel];
#endif

						//ModeRate = Regs [ ( ADSR_0 >> 1 ) + ( Channel << 3 ) ];
						ModeRate = pChRegs0->ADSR_0;

						//VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ( ModeRate >> 4 ) & 0xf ) << ( 2 ), 0x3 );
						VolumeEnvelope((s16&)pChRegs0->ENV_X, Cycles[Channel], ((ModeRate >> 4) & 0xf) << (2), 0x3, false);


#ifdef INLINE_DEBUG_RUN_DECAY
						debug << "; (after) VOL_ADSR_Value=" << hex << pChRegs0->ENV_X << "; Cycles=" << dec << Cycles[Channel] << "; Value=" << hex << ((ModeRate >> 4) & 0xf) << "; flags=" << ((ModeRate >> 15) << 1);
#endif
					}	// end else if (((s32)((s16)pChRegs0->ENV_X)) <= ((s32)VOL_SUSTAIN_Level[Channel]))


					/*
					// saturate if volume goes below zero
					// *** TODO *** ADSR volume probably can't go below zero in decay mode since it is always an exponential decrease
					//if ( VOL_ADSR_Value [ Channel ] < 0 )
					if ( ( (s16) pChRegs0->ENV_X ) < 0 )
					{
						//VOL_ADSR_Value [ Channel ] = 0;
						pChRegs0->ENV_X = 0;
					}
					*/


					break;

				case ADSR_SUSTAIN:
#ifdef INLINE_DEBUG_RUN_SUSTAIN
					debug << "\r\nCore#" << CoreNumber;
					debug << " Channel#" << dec << Channel;
#endif

#ifdef INLINE_DEBUG_RUN_SUSTAIN
					//debug << "; Sustain; Rate=" << (((double)VOL_SUSTAIN_Constant [ Channel ])/64536) << "; Rate75=" << (((double)VOL_SUSTAIN_Constant75 [ Channel ])/64536);
					debug << " SUSTAIN";
#endif

					/////////////////////////////////////////////
					// ADSR - Sustain Mode

#ifdef INLINE_DEBUG_RUN_SUSTAIN
					debug << " (before) ENV_X=" << hex << pChRegs0->ENV_X << "; Cycles=" << dec << Cycles[Channel];
#endif

					//ModeRate = Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
					ModeRate = pChRegs0->ADSR_1;

					if (((ModeRate >> 14) & 1) && (((s16)pChRegs0->ENV_X) <= 0))
					{
						// decreasing in sustain mode and at or below zero //

						pChRegs0->ENV_X = 0;

					}
					else
					{

						//VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 6 ) & 0x7f, ModeRate >> 14 );
						VolumeEnvelope((s16&)pChRegs0->ENV_X, Cycles[Channel], (ModeRate >> 6) & 0x7f, ModeRate >> 14, false);

#ifdef INLINE_DEBUG_RUN_SUSTAIN
						debug << " (after) ENV_X=" << hex << pChRegs0->ENV_X << "; Cycles=" << dec << Cycles[Channel] << "; Value=" << hex << ((ModeRate >> 6) & 0x7f) << "; flags=" << ((ModeRate >> 15) << 1);
#endif

						// ***TODO*** potential bug here NEED TO CHECK IF RATE IS INCREASING OR DECREASING
						// ADSR1 -> bit 14 (0: sustain mode increasing, 1: sustain mode decreasing)
						if ((ModeRate >> 14) & 1)
						{
							// decreasing in sustain mode //

							// can go below zero for 1T
							// or below zero
							/*
							//if ( VOL_ADSR_Value [ Channel ] < 0 )
							if ( ( (s16) pChRegs0->ENV_X ) < 0 )
							{
								//VOL_ADSR_Value [ Channel ] = 0;
								pChRegs0->ENV_X = 0;
							}
							*/
						}
						else
						{
							// increasing in sustain mode //

							// saturate if volume goes above 32767
							//if ( VOL_ADSR_Value [ Channel ] > 32767 )
							if (pChRegs0->ENV_X > 32767)
							{
								//VOL_ADSR_Value [ Channel ] = 32767;
								pChRegs0->ENV_X = 32767;
							}

						}	// end else if ((ModeRate >> 14) & 1)

					}	// end else if (((ModeRate >> 14) & 1) && (((s16)pChRegs0->ENV_X) <= 0))

					// we do not switch to release mode until key off signal is given

					break;

				case ADSR_RELEASE:
#ifdef INLINE_DEBUG_RUN_RELEASE
					debug << "\r\nCore#" << CoreNumber;
					debug << " Channel#" << Channel;
#endif

#ifdef INLINE_DEBUG_RUN_RELEASE
					//debug << "; Release; Rate=" << (((double)VOL_RELEASE_Constant [ Channel ])/(1<<30));
					debug << "; Release";
#endif

					///////////////////////////////////////////////
					// ADSR - Release Mode

#ifdef INLINE_DEBUG_RUN_RELEASE
					debug << "; (before) VOL_ADSR_Value=" << hex << pChRegs0->ENV_X << "; Cycles=" << dec << Cycles[Channel];
#endif

					// when at or below zero we turn note off completely and set adsr volume to zero
					if (((s16)pChRegs0->ENV_X) <= 0)
					{
						// ADSR volume is below zero in RELEASE mode //

						// saturate to zero
						//VOL_ADSR_Value [ Channel ] = 0;
						pChRegs0->ENV_X = 0;

						ADSR_Status[Channel] = ADSR_MUTE;

						// the channel on bit is not really for whether the channel is on or off
						//ChannelOn = ChannelOn & ~( 1 << Channel );
					}
					else
					{
						// RELEASE mode //

						// *** note *** it is possible for ADSR volume to go negative for 1T in linear decrement mode //

						//ModeRate = Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
						ModeRate = pChRegs0->ADSR_1;

						//VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
						VolumeEnvelope((s16&)pChRegs0->ENV_X, Cycles[Channel], (ModeRate & 0x1f) << (2), (((ModeRate >> 5) & 1) << 1) | 0x1, false);
					}



#ifdef INLINE_DEBUG_RUN_RELEASE
					debug << "; (after) VOL_ADSR_Value=" << hex << pChRegs0->ENV_X << "; Cycles=" << dec << Cycles[Channel] << "; Value=" << hex << ((ModeRate >> 8) & 0x7f) << "; flags=" << ((ModeRate >> 15) << 1);
#endif



					break;

				}	// end switch (ADSR_Status[Channel])

				/////////////////////////////////////////////////////////////////////////////
				// update ADSR Envelope Volume
				// commented out for ps2
				//Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ] = VOL_ADSR_Value [ Channel ];
				//pChRegs0->ENV_X = VOL_ADSR_Value [ Channel ];

				//////////////////////////////////////////////////////////////
				// load sample

				// check if channel is set to noise
				if (ChannelNoise & (1 << Channel))
				{
					// channel is set to noise //

					Sample = NoiseLevel;
				}
				else
				{
					// channel is not set to noise //

					u32 SampleNumber = CurrentSample_Read[Channel] >> 32;

					Sample = DecodedBlocks[(Channel << 5) + (SampleNumber & 0x1f)];

					///////////////////////////////////////////
					// apply sample interpolation

					Sample = Calc_sample_gx(CurrentSample_Read[Channel] >> 16, Sample, DecodedBlocks[(Channel << 5) + ((SampleNumber - 1) & 0x1f)],
						DecodedBlocks[(Channel << 5) + ((SampleNumber - 2) & 0x1f)], DecodedBlocks[(Channel << 5) + ((SampleNumber - 3) & 0x1f)]);


					////////////////////////////////////
					// apply envelope volume
					// this does not apply when set to noise
					Sample = (Sample * ((s64)((s16)pChRegs0->ENV_X))) >> c_iVolumeShift;

					UpdatePitch(Channel, pChRegs0->PITCH, PitchMod, PreviousSample);

				}	// end else if (ChannelNoise & (1 << Channel))


				//////////////////////////////////////////////////////////////////
				// apply volume processing


				// apply current channel volume
				//ChSampleL = ( Sample * ((s64) ((s16)Regs [ ( CVOL_L_START - SPU_X + ( Channel << 1 ) ) >> 1 ]) ) ) >> c_iVolumeShift;
				//ChSampleR = ( Sample * ((s64) ((s16)Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ]) ) ) >> c_iVolumeShift;
				ChSampleL = (Sample * ((s64)pChRegs0->CVOL_L)) >> c_iVolumeShift;
				ChSampleR = (Sample * ((s64)pChRegs0->CVOL_R)) >> c_iVolumeShift;


				// check if channel is muted for debugging
				if (!(Debug_ChannelEnable & (1 << Channel)))
				{
					ChSampleL = 0;
					ChSampleR = 0;
				}


#ifdef ENABLE_REVERB_IN
				// check if reverb is on for channel
				// for ps2, does the channels separately for left and right, which is different from ps1

				// check if master effect voice volume is enabled for left (WET)
				if (pCoreRegs0->MMIX & 0x200)
				{
					if (ReverbOnL & (1 << Channel))
					{
						//debug << "\r\nREVERB-IN-LEFT-CH#" << dec << Channel << "-CORE#" << dec << CoreNumber << "-SAMPLE:" << hex << ChSampleL;
						ReverbSampleL += ChSampleL;
					}
				}

				// check if master effect voice volume is enabled for right (WET)
				if (pCoreRegs0->MMIX & 0x100)
				{
					if (ReverbOnR & (1 << Channel))
					{
						//debug << "\r\nREVERB-IN-RIGHT-CH#" << dec << Channel << "-CORE#" << dec << CoreNumber << "-SAMPLE:" << hex << ChSampleR;
						ReverbSampleR += ChSampleR;
					}
				}
#else
				// for debugging //
				ReverbSampleL = 0;
				ReverbSampleR = 0;
#endif

				///////////////////////////////////////////////////////////////////
				// mix sample l/r

				// ***TODO*** for PS2 should only do this if VMIXL/VMIXR bit is set for channel
				// ***NEEDS FIXING***

				// check master dry voice output left (DRY)
				if (pCoreRegs0->MMIX & 0x800)
				{
					if (SoundOnL & (1 << Channel))
					{
						SampleL += ChSampleL;
					}
				}

				// check master dry voice output right (DRY)
				if (pCoreRegs0->MMIX & 0x400)
				{
					if (SoundOnR & (1 << Channel))
					{
						SampleR += ChSampleR;
					}
				}

			}	// end else if (ADSR_Status[Channel] == ADSR_MUTE)


			// store previous sample in case next channel uses frequency modulation
			PreviousSample = Sample;

			// save current sample for debugging
			Debug_CurrentRawSample[Channel] = Sample;

			// copy samples for voice1 and voice3 into buffer //


#ifdef ENABLE_VOICE13_MEM
			switch (Channel)
			{
			case 1:
				pVoice1_Out[DecodeBufferOffset >> 1] = adpcm_decoder::clamp(Sample);
				break;

			case 3:
				pVoice3_Out[DecodeBufferOffset >> 1] = adpcm_decoder::clamp(Sample);
				break;
			}
#endif


			//////////////////////////////////////////////////////////////////
			// Advance to next sample for channel
			//CurrentSample_Offset [ Channel ] += dSampleDT [ Channel ];
			//CurrentSample_Read [ Channel ] += dSampleDT [ Channel ];
			
			// save for debugging
			Debug_CurrentSample [ Channel ] = ( CurrentBlockAddress [ Channel ] );
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
				if ( RAM [ CurrentBlockAddress [ Channel ] ] & ( 0x1 << 8 ) )
				{
					////////////////////////////////////////////////////////////////////
					// reached loop/end flag
					
					// make sure channel is not set to mute
#ifdef DISABLE_ENDX_ON_MUTE
					if ( ADSR_Status [ Channel ] != ADSR_MUTE
#ifdef DISABLE_ENDX_ON_KOFF
						&& ADSR_Status [ Channel ] != ADSR_RELEASE
#endif
						)
#endif
					{
						// passed end of sample data, even if looping
						ChannelOn |= ( 1 << Channel );
					}

					// check if envelope should be killed
					//if ( ! ( bLoopSet & ( 1 << Channel ) ) )
					if ( !( RAM [ CurrentBlockAddress [ Channel ] ] & ( 0x2 << 8 ) ) 
#ifdef REQUIRE_ALL_LOOP_FLAGS
						|| !( bLoopSet & ( 1 << Channel ) )
#endif
						)
					{
						// if channel is not already set to mute, then turn off reverb for channel and mark we passed end of sample data
						if ( ADSR_Status [ Channel ] != ADSR_MUTE )
						{
							// turn off reverb for channel
							// for ps2, has a reverbon for both left and right
							//ReverbOn &= ~( 1 << Channel );
#ifdef CLEAR_VMIXE_ON_CHDONE
							ReverbOnL &= ~( 1 << Channel );
							ReverbOnR &= ~( 1 << Channel );
#endif

#ifdef CLEAR_VMIX_ON_CHDONE
							// for PS2, should probably also do this for dry channel bits
							SoundOnL &= ~( 1 << Channel );
							SoundOnR &= ~( 1 << Channel );
#endif
						}
						
						// kill envelope //
						ADSR_Status [ Channel ] = ADSR_MUTE;
						//VOL_ADSR_Value [ Channel ] = 0;
						pChRegs0->ENV_X = 0;
					}
					
					// set address of next sample block to be loop start address
					CurrentBlockAddress [ Channel ] = SWAPH( pChRegs1->LSA ) & ( c_iRam_Mask >> 1 );
					
					// set the address of the next sample for ps2 spu2
					//pChRegs1->NEX = SWAPH ( ( CurrentBlockAddress [ Channel ] & ~0x7 ) | ( ( CurrentSample_Offset [ Channel ] >> 29 ) & 0x7 ) );
					
				}	// end if ( RAM [ CurrentBlockAddress [ Channel ] ] & ( 0x1 << 8 ) )
				else
				{
					// did not reach loop/end flag //
					
					// advance to address of next sample block
					//CurrentBlockAddress [ Channel ] += 16;
					//CurrentBlockAddress [ Channel ] &= c_iRam_Mask;
					CurrentBlockAddress [ Channel ] += 8;
					CurrentBlockAddress [ Channel ] &= ( c_iRam_Mask >> 1 );
					
					// set the address of the next sample for ps2 spu2
					//pChRegs1->NEX = SWAPH ( CurrentBlockAddress [ Channel ] + 1 );

				}	// end else if ( RAM [ CurrentBlockAddress [ Channel ] ] & ( 0x1 << 8 ) )
				
				
				//////////////////////////////////////////////////////////////////////////////
				// Check loop start flag and set loop address if needed
				// LOOP_START is bit 3 actually
				// note: LOOP_START is actually bit 2
				if ( ( RAM [ CurrentBlockAddress [ Channel ] ] & ( 0x4 << 8 ) ) )
				{
					// but only if LSA was not set manually
					if ( ! ( bLoopManuallySet & ( 1 << Channel ) ) )
					{
						// check set if endpoint reached ??
						if ( !( ChannelOn & ( 1 << Channel ) ) )
						{
							///////////////////////////////////////////////////
							// we are at loop start address
							// set loop start address
							pChRegs1->LSA = SWAPH ( CurrentBlockAddress [ Channel ] & ~7 );

							// loop set ?
							//bLoopSet |= ( 1 << Channel );

							// clear killing of envelope
							//KillEnvelope_Bitmap &= ~( 1 << Channel );
						}

					}	// if ( ! ( bLoopManuallySet & ( 1 << Channel ) ) )

				}	// if ( ( RAM [ CurrentBlockAddress [ Channel ] & ( c_iRam_Mask >> 1 ) ] & ( 0x4 << 8 ) ) )


				// clear loop set if loop bit not set
				if ( ! ( RAM [ CurrentBlockAddress [ Channel ] ] & ( 0x2 << 8 ) ) )
				{
					bLoopSet &= ~( 1 << Channel );
				}


#ifdef UPDATE_NEX_IN_BLOCKS
				// check if new block is going to loop
				/*
				if ( RAM [ CurrentBlockAddress [ Channel ] & ( c_iRam_Mask >> 1 ) ] & ( 0x1 << 8 ) )
				{
					pChRegs1->NEX = pChRegs1->LSA;
				}
				else
				{
					pChRegs1->NEX = SWAPH ( ( ( CurrentBlockAddress [ Channel ] & ~7 ) + 8 ) );
				}
				*/

				//pChRegs1->NEX = SWAPH( ( SWAPH( pChRegs1->NEX ) + 8 ) & ( c_iRam_Mask >> 1 ) );
				pChRegs1->NEX = SWAPH ( CurrentBlockAddress [ Channel ] + 1 );
#endif
				
				
				//////////////////////////////////////////////////////////////////////////////
				// check if the IRQ address is in this block and if interrupts are enabled
				// note: it checks the IRQ on both cores

				// note: it works by checking only the irq on the cores where interrupts are enabled
				// then it says that it is the core that triggered the interrupt, even if a different core actually hit the int
				if ( SPU2::_SPU2->SPU0.pCoreRegs0->CTRL & 0x40 )
				{
					if ( ( CurrentBlockAddress [ Channel ] & ( c_iRam_Mask >> 1 ) ) == ( ( SWAPH( SPU2::_SPU2->SPU0.pCoreRegs0->IRQA ) ) & ( c_iRam_Mask >> 1 ) ) )
					{
#ifdef INLINE_DEBUG_INT
	debug << "\r\nSPU:INT:ADDR ch#" << dec << Channel;
#endif
						// we have reached irq address - trigger interrupt
						SetInterrupt ();
						
						// do this for ps2
						//SetInterrupt_Core ( CoreNumber );
						SetInterrupt_Core ( 0 );
						
						// interrupt
						//pCoreRegs0->STAT |= 0x40;
						SPU2::_SPU2->SPU0.pCoreRegs0->STAT |= 0x40;
					}

				}	// end if ( SPU2::_SPU2->SPU0.pCoreRegs0->CTRL & 0x40 )

				if ( SPU2::_SPU2->SPU1.pCoreRegs0->CTRL & 0x40 )
				{
					if ( ( CurrentBlockAddress [ Channel ] & ( c_iRam_Mask >> 1 ) ) == ( ( SWAPH( SPU2::_SPU2->SPU1.pCoreRegs0->IRQA ) ) & ( c_iRam_Mask >> 1 ) ) )
					{
#ifdef INLINE_DEBUG_INT
	debug << "\r\nSPU:INT:ADDR ch#" << dec << Channel;
#endif

						// we have reached irq address - trigger interrupt
						SetInterrupt ();
						
						// do this for ps2
						//SetInterrupt_Core ( CoreNumber );
						SetInterrupt_Core ( 1 );
						
						// interrupt
						//pCoreRegs0->STAT |= 0x40;
						SPU2::_SPU2->SPU1.pCoreRegs0->STAT |= 0x40;
					}

				}	// end if ( SPU2::_SPU2->SPU1.pCoreRegs0->CTRL & 0x40 )

				
				// decode the new block
				CurrentSample_Write [ Channel ] += 28;

#ifdef ENABLE_MUTE_CHANNEL_OPTIMIZATION
				// shouldn't need to do this if channel is set to mute
				if (ADSR_Status[Channel] != ADSR_MUTE)
#endif
				{
					//SampleDecoder [ Channel ].decode_packet32 ( (adpcm_packet*) & ( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] ), DecodedSamples );
					SampleDecoder[Channel].decode_packet32((adpcm_packet*)&(RAM[CurrentBlockAddress[Channel]]), DecodedSamples);
					for (int i = 0; i < 28; i++) DecodedBlocks[(Channel << 5) + ((CurrentSample_Write[Channel] + i) & 0x1f)] = DecodedSamples[i];
				}
			}
			
#ifndef UPDATE_NEX_IN_BLOCKS

			// check if block is going to loop

			// set the address of the next sample for ps2 spu2
			//pChRegs1->NEX = SWAPH ( ( ( CurrentBlockAddress [ Channel ] & ~0x7 ) | ( ( CurrentSample_Offset [ Channel ] >> 29 ) & 0x7 ) ) + 1 );
			//pChRegs1->NEX = SWAPH ( ( ( CurrentBlockAddress [ Channel ] ) + ( ( CurrentSample_Offset [ Channel ] >> 34 ) & 7 ) ) + 1 );
			//pChRegs1->NEX = SWAPH ( ( ( ( CurrentSample_Offset [ Channel ] >> 34 ) & 7 ) ) + 1 );

			// get the current sample it is on
			Temp = ( ( CurrentSample_Offset [ Channel ] >> 34 ) & 7 ) + 1;

			// want the next sample
			Temp++;

			// but don't point to the header of block
			if ( Temp > 7 ) Temp |= 1;

			if ( ( RAM [ CurrentBlockAddress [ Channel ] & ( c_iRam_Mask >> 1 ) ] & ( 0x1 << 8 ) ) && ( Temp > 7 ) )
			{
				// looping //

				pChRegs1->NEX = SWAPH( ( SWAPH( pChRegs1->LSA ) & ~7 ) | 1 );
			}
			else
			{
				// not looping //

				pChRegs1->NEX = SWAPH( CurrentBlockAddress [ Channel ] + Temp );
			}

#endif
			
		}	// for ( Channel = 0; Channel < 24; Channel++ )
		
	}	// end if ( pCoreRegs0->CTRL >> 15 )


#ifdef ENABLE_MIXDRY_MEM
	// copy dry mix of samples (before reverb?)
	pDryLeft_Out [ DecodeBufferOffset >> 1 ] = adpcm_decoder::clamp ( SampleL );
	pDryRight_Out [ DecodeBufferOffset >> 1 ] = adpcm_decoder::clamp ( SampleR );
#endif

	
	
	// check if adma is playing (not if it is active, just playing)
	//if ( bBufferFull [ ( ulADMA_PlayOffset & 512 ) >> 9 ] )
	if ( isADMATransferMode() )
	{
		// should only play if there is data in the half-buffer ??
		//if ( ( NextSoundBufferAddress - ulADMA_PlayOffset ) >= 512 )
		if ( ((s64)( NextSoundBufferAddress - ulADMA_PlayOffset )) > 0 )
		{

		// check if should mix in the sound data input area data (ADMA) (DRY)
		if ( pCoreRegs0->MMIX & 0x40 )
		{
			// mix in the right sample
			SampleR += ( ( ( (s64) pCoreSoundDataInputR [ ( ulADMA_PlayOffset & DecodeBufferMask ) >> 1 ] ) * ( (s64) pCoreRegs1->BVOL_R ) ) >> c_iVolumeShift );
			//SampleR += ( ( ( (s64) pCoreSoundDataInputR [ ulADMA_PlayOffset >> 1 ] ) * ( (s64) pCoreRegs1->BVOL_R ) ) >> c_iVolumeShift );
		}
		
		if ( pCoreRegs0->MMIX & 0x80 )
		{
			// mix in the left sample
			SampleL += ( ( ( (s64) pCoreSoundDataInputL [ ( ulADMA_PlayOffset & DecodeBufferMask ) >> 1 ] ) * ( (s64) pCoreRegs1->BVOL_L ) ) >> c_iVolumeShift );
			//SampleL += ( ( ( (s64) pCoreSoundDataInputL [ ulADMA_PlayOffset >> 1 ] ) * ( (s64) pCoreRegs1->BVOL_L ) ) >> c_iVolumeShift );
		}

		
		// check if should mix in the sound data input area data (ADMA) (WET)
		if ( pCoreRegs0->MMIX & 0x10 )
		{
			// mix in the right sample
			ReverbSampleR += ( ( ( (s64) pCoreSoundDataInputR [ ( ulADMA_PlayOffset & DecodeBufferMask ) >> 1 ] ) * ( (s64) pCoreRegs1->BVOL_R ) ) >> c_iVolumeShift );
			//ReverbSampleR += ( ( ( (s64) pCoreSoundDataInputR [ ulADMA_PlayOffset >> 1 ] ) * ( (s64) pCoreRegs1->BVOL_R ) ) >> c_iVolumeShift );
		}
		
		if ( pCoreRegs0->MMIX & 0x20 )
		{
			// mix in the left sample
			ReverbSampleL += ( ( ( (s64) pCoreSoundDataInputL [ ( ulADMA_PlayOffset & DecodeBufferMask ) >> 1 ] ) * ( (s64) pCoreRegs1->BVOL_L ) ) >> c_iVolumeShift );
			//ReverbSampleL += ( ( ( (s64) pCoreSoundDataInputL [ ulADMA_PlayOffset >> 1 ] ) * ( (s64) pCoreRegs1->BVOL_L ) ) >> c_iVolumeShift );
		}
		
		ulADMA_PlayOffset += 2;
		//ulADMA_PlayOffset &= ( DecodeBufferSize - 1 );

		// check for spu interrupt from playing address
		//if ( pCoreRegs0->CTRL & 0x40 )
		//{
			// interrupts for this spu core are enabled //

		// get the decode buffer offset
		ulDecodeAddr = ( ( ulADMA_PlayOffset & DecodeBufferMask ) >> 1 );

		// core 0 play buffers are in the region 0x2000-0x2400
		// core 1 play buffers are in the region 0x2400-0x2800
		if ( pCoreRegs0->CTRL & 0x40 )
		{
			// get the irq addresses to check against
			ulIrqAddr0 = SWAPH( pCoreRegs0->IRQA );

			// mask
			ulIrqAddr0 &= ( c_iRam_Mask >> 1 );

			if ( !CoreNumber )
			{
				// spu2 core 0 //

				// first check against the irq address in core0 against core0 buffers
				if ( ( ulIrqAddr0 >= 0x2000 ) && ( ulIrqAddr0 < 0x2400 ) )
				{
					// check if decode buffer address is a match
					if ( ( ulDecodeAddr & 0x1ff ) == ( ulIrqAddr0 & 0x1ff ) )
					{
						// trigger interrupt for core0 //
						SetInterrupt ();
						SetInterrupt_Core ( CoreNumber );
						pCoreRegs0->STAT |= 0x40;
					}
				}

			}	// end if ( !CoreNumber )
			else
			{
				// spu2 core 1 //

				if ( ( ulIrqAddr0 >= 0x2400 ) && ( ulIrqAddr0 < 0x2800 ) )
				{
					// check if decode buffer address is a match
					if ( ( ulDecodeAddr & 0x1ff ) == ( ulIrqAddr0 & 0x1ff ) )
					{
						// trigger interrupt for core0 //
						SetInterrupt ();
						SetInterrupt_Core ( CoreNumber );
						pCoreRegs0->STAT |= 0x40;
					}
				}

			}	// end if ( !CoreNumber ) else

		}	//end if ( pCoreRegs0->CTRL & 0x40 )



		
		// if buffer-half is done playing, then clear it for a transfer of samples
		//if ( ! ( ulADMA_PlayOffset & 0x1ff ) )
		if ( ((s64)( NextSoundBufferAddress - ulADMA_PlayOffset )) <= 512 )
		{
			// clear the opposite buffer-half for a transfer since it is done playing
			//bBufferFull [ ( ulADMA_PlayOffset >> 9 ) ^ 1 ] = 0;
			
			// that changes a dma condition, so update dma
			Playstation1::Dma::_DMA->Update_ActiveChannel ();
		}

		}	// end if ( ( NextSoundBufferAddress - ulADMA_PlayOffset ) >= 512 )

	}	// end if ( isADMATransferMode() )

	
	// store to audio buffer l/r
	// check if SPU is muted
	//if ( ! ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] & 0x4000 ) )
	if ( ! ( pCoreRegs0->CTRL & 0x4000 ) )
	{
		SampleL = 0;
		SampleR = 0;
	}



#ifdef ENABLE_AVOL_CALC
	if ( CoreNumber )
	{
		// ***todo*** multiply SPU0 Output by input volume of SPU1
		SPU2::_SPU2->SPU0.SampleL = ( SPU2::_SPU2->SPU0.SampleL * ( (s32) pCoreRegs1->AVOL_L ) ) >> 15;
		SPU2::_SPU2->SPU0.SampleR = ( SPU2::_SPU2->SPU0.SampleR * ( (s32) pCoreRegs1->AVOL_R ) ) >> 15;
		
		// mix dry external left input if enabled (DRY)
		if ( pCoreRegs0->MMIX & 0x8 )
		{
			SampleL += SPU2::_SPU2->SPU0.SampleL;
		}
		
		// mix dry external right input if enabled (DRY)
		if ( pCoreRegs0->MMIX & 0x4 )
		{
			SampleR += SPU2::_SPU2->SPU0.SampleR;
		}
		
		// mix wet external left input if enabled (WET)
		if ( pCoreRegs0->MMIX & 0x2 )
		{
			ReverbSampleL += SPU2::_SPU2->SPU0.SampleL;
		}
		
		// mix wet external right input if enabled (WET)
		if ( pCoreRegs0->MMIX & 0x1 )
		{
			ReverbSampleR += SPU2::_SPU2->SPU0.SampleR;
		}
	}
#endif
	
	
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
			// leave out temporarily for now
			/*
			tVOL_L = (s64) ( (s16) Regs [ ( CDVOL_L - SPU_X ) >> 1 ] );
			tVOL_R = (s64) ( (s16) Regs [ ( CDVOL_R - SPU_X ) >> 1 ] );
			CD_SampleL = ( TempL * tVOL_L ) >> c_iVolumeShift;
			CD_SampleR = ( TempR * tVOL_R ) >> c_iVolumeShift;
			*/
			
			// mix into final audio output for this sample l/r
			
			// sample should also be copied into sound ram for cd audio l/r area
			RAM [ 0x0000 + ( DecodeBufferOffset >> 1 ) ] = TempL;
			RAM [ 0x0200 + ( DecodeBufferOffset >> 1 ) ] = TempR;
			
			
		//}
		
		// check if cd audio output is enabled
		if ( pCoreRegs0->CTRL & 0x1 )
		{
			// mix
			SampleL += CD_SampleL;
			SampleR += CD_SampleR;
			
			// multiply by gain??
			//SampleL *= c_iMixer_Gain;
			//SampleR *= c_iMixer_Gain;
			
			// check if reverb is on for cd
			//if ( REG ( CTRL ) & 0x4 )
			if ( pCoreRegs0->CTRL & 0x4 )
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
	
	///////////////////////////////////////////////////////////////////
	// apply effect processing
	
#ifdef ENABLE_REVERB_RUN
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
#endif

		
#ifdef ENABLE_REVERB_OUT
		// mix
		// the mix of reverb output should happen unconditionally...
		SampleL += ReverbL_Output;
		SampleR += ReverbR_Output;

//debug << "\r\nREVERB-OUT-LEFT-CH#" << dec << Channel << "-CORE#" << dec << CoreNumber << "-SAMPLE:" << hex << ReverbL_Output;
//debug << "\r\nREVERB-OUT-RIGHT-CH#" << dec << Channel << "-CORE#" << dec << CoreNumber << "-SAMPLE:" << hex << ReverbR_Output;
#endif


#ifdef ENABLE_MIXWET_MEM
	// copy wet mix of voices (after reverb?)
	pWetLeft_Out [ DecodeBufferOffset >> 1 ] = adpcm_decoder::clamp ( SampleL );
	pWetRight_Out [ DecodeBufferOffset >> 1 ] = adpcm_decoder::clamp ( SampleR );
#endif

		
		// multiply by gain??
		//SampleL *= c_iMixer_Gain;
		//SampleR *= c_iMixer_Gain;
	//}
	
	

	
	//////////////////////////////////////
	// ***TODO*** apply master volume
	// still need to fix this so it uses the "current master volume" register
	//SampleL = ( SampleL * ( (s64) ((s16)Regs [ ( CMVOL_L - SPU_X ) >> 1 ]) ) ) >> c_iVolumeShift;
	//SampleR = ( SampleR * ( (s64) ((s16)Regs [ ( CMVOL_R - SPU_X ) >> 1 ]) ) ) >> c_iVolumeShift;
	SampleL = ( SampleL * ( (s64) pCoreRegs1->CMVOL_L ) ) >> c_iVolumeShift;
	SampleR = ( SampleR * ( (s64) pCoreRegs1->CMVOL_R ) ) >> c_iVolumeShift;
	
	
#ifdef ENABLE_CORE0_MEM
	// check if this is core0
	if ( !CoreNumber )
	{
		// copy samples into core0 output area
		pCore0Left_Out [ DecodeBufferOffset >> 1 ] = adpcm_decoder::clamp ( SampleL );
		pCore0Right_Out [ DecodeBufferOffset >> 1 ] = adpcm_decoder::clamp ( SampleR );
	}
#endif
	
	
	////////////////////////////////////////////////////////
	// Apply the Program's Global Volume set by user
	// spucore will not be worried about this, just the interface
	//SampleL = ( SampleL * GlobalVolume ) >> c_iVolumeShift;
	//SampleR = ( SampleR * GlobalVolume ) >> c_iVolumeShift;


	
	
	///////////////////////////////////////////////////////////////////////////
	// Update Decode Buffer Offset (for CD L/R,Voice1+Voice3 decode area)
	DecodeBufferOffset += 2;
	DecodeBufferOffset &= ( DecodeBufferSize - 1 );
	
	
	///////////////////////////////////////////////////////
	// update whether decoding in first/second half of buffer
	pCoreRegs0->STAT &= ~( 0x200 << 2 );
	pCoreRegs0->STAT |= ( DecodeBufferOffset & 0x200 ) << 2;




	// check for interrupts in double buffer areas //

	// get the decode buffer offset
	ulDecodeAddr = ( DecodeBufferOffset >> 1 );

	if ( pCoreRegs0->CTRL & 0x40 )
	{
		// interrupts for this spu core are enabled //

		// core0 has buffers in ranges 0x400-0x800,0x1000-0x1800
		// core1 has buffers in ranges 0x800-0x1000,0x1800-0x2000

		// get the irq addresses to check against
		//ulIrqAddr0 = SWAPH( SPU2::_SPU2->SPU0.pCoreRegs0->IRQA );
		ulIrqAddr0 = SWAPH( pCoreRegs0->IRQA );

		// mask
		ulIrqAddr0 &= ( c_iRam_Mask >> 1 );

		// check what core this is
		if ( !CoreNumber )
		{
			// spu2 core 0 //

			// first check against the irq address in core0 against core0 buffers
			if ( 
				( ( ulIrqAddr0 >= 0x400 ) && ( ulIrqAddr0 < 0x800 ) )
				||
				( ( ulIrqAddr0 >= 0x1000 ) && ( ulIrqAddr0 < 0x1800 ) )
			)
			{
				// check if decode buffer address is a match
				if ( ( ulDecodeAddr & 0x1ff ) == ( ulIrqAddr0 & 0x1ff ) )
				{
					// trigger interrupt for core0 //
					SetInterrupt ();
					SetInterrupt_Core ( 0 );
					pCoreRegs0->STAT |= 0x40;
				}
			}

		}	// end if ( !CoreNumber )
		else
		{
			// spu2 core 1 //

			// first check against the irq address in core0 against core1 buffers
			if ( 
				( ( ulIrqAddr0 >= 0x800 ) && ( ulIrqAddr0 < 0x1000 ) )
				||
				( ( ulIrqAddr0 >= 0x1800 ) && ( ulIrqAddr0 < 0x2000 ) )
			)
			{
				// check if decode buffer address is a match
				if ( ( ulDecodeAddr & 0x1ff ) == ( ulIrqAddr0 & 0x1ff ) )
				{
					// trigger interrupt for core0 //
					SetInterrupt ();
					SetInterrupt_Core ( 1 );
					pCoreRegs0->STAT |= 0x40;
				}
			}

		}	// end if ( !CoreNumber ) else

	}	// end if ( pCoreRegs0->CTRL & 0x40 )

	
	
	// store back to CON
	pCoreRegs0->CON = ChannelOn;
	
	// write back reverb on/off
	pCoreRegs0->VMIXEL = ReverbOnL;
	pCoreRegs0->VMIXER = ReverbOnR;
	
	// there's also another flag for sending dry channels to the dry mixers
	pCoreRegs0->VMIXL = SoundOnL;
	pCoreRegs0->VMIXR = SoundOnR;
}






SPU2::SPU2 ()
{
	cout << "Running SPU2 constructor...\n";
}


SPU2::~SPU2 ()
{
	waveOutClose( hWaveOut );
}


void SPU2::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( SPU2 ) );
}



void SPU2::Start ()
{
	cout << "Running SPU::Start...\n";

#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create ( "SPU2Interface_Log.txt" );
#endif

#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT_CORE
	// put debug output into a separate file
	SPUCore::debug.SetSplit ( true );
	SPUCore::debug.SetCombine ( false );
#endif

	SPUCore::debug.Create ( "SPU2Core_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering SPU::Start";
#endif

	
	_SPU2 = this;
	Reset ();
	
	// set pointer into registers for spucores from parent object
	SPUCore::Regs16.u = SPU2::_SPU2->Regs16;
	
	
	// set ram device pointer for SPUCore(s)
	SPUCore::RAM = RAM;
	SPUCore::_DebugPC = _DebugPC;
	SPUCore::_DebugCycleCount = _DebugCycleCount;

	
	// start the cores
	SPU0.Start ();
	SPU1.Start ();
	
	// set the core numbers
	SPU0.SetCoreNumber ( 0 );
	SPU1.SetCoreNumber ( 1 );
	
	Refresh ();
	
	// set the global volume to default
	GlobalVolume = c_iGlobalVolume_Default;
	
	// start the sound buffer out at 1m for now
	PlayBuffer_Size = c_iPlayBuffer_MaxSize;
	NextPlayBuffer_Size = c_iPlayBuffer_MaxSize;

	wfx.nSamplesPerSec = 48000; /* sample rate */
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
	
	
	ulNumThreads = 1;
	
	
	// *** testing ***
	//hWaveOut_Save = (u64)hWaveOut;
	
	// output size of object
	//cout << " Size of SPU class=" << sizeof ( SPU );

	// SPU is not needed to run on EVERY cycle
	Set_NextEvent ( CPUCycles_PerSPUCycle );
	
#ifdef INLINE_DEBUG
	debug << "->Exiting SPU::Start";
#endif
}


void SPU2::Refresh ()
{
	SPU0.Refresh ();
	SPU1.Refresh ();
}


/*
void SPU::Reset ()
{
	int i;
	
	// zero object
	memset ( this, 0, sizeof( SPU ) );
	
	// pointers for quick access
	_vLOUT = & ( Regs [ ( vLOUT - SPU_X ) >> 1 ] );
	_vROUT = & ( Regs [ ( vROUT - SPU_X ) >> 1 ] );
	_mBASE = & ( Regs [ ( mBASE - SPU_X ) >> 1 ] );

	_dAPF1 = & ( Regs [ ( dAPF1 - SPU_X ) >> 1 ] );
	_dAPF2 = & ( Regs [ ( dAPF2 - SPU_X ) >> 1 ] );
	_vIIR = & ( Regs [ ( vIIR - SPU_X ) >> 1 ] );
	_vCOMB1 = & ( Regs [ ( vCOMB1 - SPU_X ) >> 1 ] );
	_vCOMB2 = & ( Regs [ ( vCOMB2 - SPU_X ) >> 1 ] );
	_vCOMB3 = & ( Regs [ ( vCOMB3 - SPU_X ) >> 1 ] );
	_vCOMB4 = & ( Regs [ ( vCOMB4 - SPU_X ) >> 1 ] );
	_vWALL = & ( Regs [ ( vWALL - SPU_X ) >> 1 ] );
	_vAPF1 = & ( Regs [ ( vAPF1 - SPU_X ) >> 1 ] );
	_vAPF2 = & ( Regs [ ( vAPF2 - SPU_X ) >> 1 ] );
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
	_vLIN = & ( Regs [ ( vLIN - SPU_X ) >> 1 ] );
	_vRIN = & ( Regs [ ( vRIN - SPU_X ) >> 1 ] );
	
	
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
	
	
	// SPU is not needed to run on EVERY cycle
	//WaitCycles = CPUCycles_PerSPUCycle;
	//NextEvent_Cycle = CPUCycles_PerSPUCycle;
	Set_NextEvent ( CPUCycles_PerSPUCycle );
}
*/


/*
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
*/




/*
void SPU::Run ()
{
	int Channel;
	
	s64 Sample, PreviousSample, ChSampleL, ChSampleR, SampleL, SampleR, CD_SampleL, CD_SampleR, ReverbSampleL, ReverbSampleR;
	s64 FOutputL, FOutputR, ROutputL, ROutputR;
	
	u32 ChannelOn, ChannelNoise, PitchMod, ReverbOn;
	
	u64 Temp;
	u32 ModeRate;
	
	
	if ( NextEvent_Cycle != *_DebugCycleCount ) return;
	
	// update number of spu cycles ran
	CycleCount++;
	
	//NextEvent_Cycle = *_DebugCycleCount + CPUCycles_PerSPUCycle;
	Set_NextEvent ( CPUCycles_PerSPUCycle );
	
//#ifdef INLINE_DEBUG_RUN
//	debug << "\r\nSPU::Run";
//#endif

	// initialize current sample for left and right
	SampleL = 0;
	SampleR = 0;
	ReverbSampleL = 0;
	ReverbSampleR = 0;

	
	// SPU is on
	
	// *** TODO *** run SPU and output 1 sample LR
	
	/////////////////////////////////////////////////////////////////////
	// get what channels are enabled
	ChannelOn = Regs [ ( ( CON_0 - SPU_X ) >> 1 ) & 0xff ];
	ChannelOn |= ( (u32) ( Regs [ ( ( CON_1 - SPU_X ) >> 1 ) & 0xff ] ) ) << 16;
	
	// get what channels are set to noise
	ChannelNoise = Regs [ ( ( NON_0 - SPU_X ) >> 1 ) & 0xff ];
	ChannelNoise |= ( (u32) ( Regs [ ( ( NON_1 - SPU_X ) >> 1 ) & 0xff ] ) ) << 16;
	
	// get what channels are using frequency modulation
	PitchMod = Regs [ ( PMON_0 - SPU_X ) >> 1 ];
	PitchMod |= ( (u32) ( Regs [ ( PMON_1 - SPU_X ) >> 1 ] ) ) << 16;
	
	// get what channels have reverb on
	ReverbOn = Regs [ ( RON_0 - SPU_X ) >> 1 ];
	ReverbOn |= ( (u32) ( Regs [ ( RON_1 - SPU_X ) >> 1 ] ) ) << 16;

//#ifdef INLINE_DEBUG_RUN
//	debug << "; ChannelOn=" << ChannelOn << " KeyOn=" << KeyOn;
//#endif

	// if spu is enabled, run noise generator
	if ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] >> 15 )
	{
		RunNoiseGenerator ();
	}
	
	////////////////////////////
	// loop through channels
	for ( Channel = 0; Channel < 24; Channel++ )
	{
	
		// check if SPU is on
		if ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] >> 15 )
		{

			//SweepVolume ( Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ], VOL_L_Value [ Channel ], VOL_L_Constant [ Channel ], VOL_L_Constant75 [ Channel ] );
			//SweepVolume ( Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ], VOL_R_Value [ Channel ], VOL_R_Constant [ Channel ], VOL_R_Constant75 [ Channel ] );
			//SweepVolume ( Regs [ ( ( MVOL_L - SPU_X ) >> 1 ) & 0xff ], MVOL_L_Value, MVOL_L_Constant, MVOL_L_Constant75 );
			//SweepVolume ( Regs [ ( ( MVOL_R - SPU_X ) >> 1 ) & 0xff ], MVOL_R_Value, MVOL_R_Constant, MVOL_R_Constant75 );
			
			if ( Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ] >> 15 )
			{
				// set current volume left
				VOL_L_Value [ Channel ] = (s64) ( (s16) Regs [ ( CVOL_L_START - SPU_X + ( Channel << 1 ) ) >> 1 ] );
				
				// perform envelope
				VolumeEnvelope ( VOL_L_Value [ Channel ], VOL_L_Cycles [ Channel ], Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ] & 0x7f, ( Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ] >> 13 ) & 0x3 );
				
				// store the new current volume left
				Regs [ ( CVOL_L_START - SPU_X + ( Channel << 1 ) ) >> 1 ] = VOL_L_Value [ Channel ];
			}
			else
			{
				// just set the current volume L
				Regs [ ( CVOL_L_START - SPU_X + ( Channel << 1 ) ) >> 1 ] = Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ] << 1;
			}

			if ( Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ] >> 15 )
			{
				// set current volume right
				VOL_R_Value [ Channel ] = (s64) ( (s16) Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ] );
				
				VolumeEnvelope ( VOL_R_Value [ Channel ], VOL_R_Cycles [ Channel ], Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ] & 0x7f, ( Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ] >> 13 ) & 0x3 );
				
				// store the new current volume right
				Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ] = VOL_R_Value [ Channel ];
			}
			else
			{
				// just set the current volume R
				Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ] = Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ] << 1;
			}
			
			if ( Regs [ ( MVOL_L - SPU_X ) >> 1 ] >> 15 )
			{
				MVOL_L_Value = (s64) ( (s16) Regs [ ( CMVOL_L - SPU_X ) >> 1 ] );
				
				VolumeEnvelope ( MVOL_L_Value, MVOL_L_Cycles, Regs [ ( MVOL_L - SPU_X ) >> 1 ] & 0x7f, ( Regs [ ( MVOL_L - SPU_X ) >> 1 ] >> 13 ) & 0x3 );
				
				Regs [ ( CMVOL_L - SPU_X ) >> 1 ] = MVOL_L_Value;
			}
			else
			{
				// just set the current master volume L
				Regs [ ( CMVOL_L - SPU_X ) >> 1 ] = Regs [ ( MVOL_L - SPU_X ) >> 1 ] << 1;
			}
		
			if ( Regs [ ( MVOL_R - SPU_X ) >> 1 ] >> 15 )
			{
				MVOL_R_Value = (s64) ( (s16) Regs [ ( CMVOL_R - SPU_X ) >> 1 ] );
				
				VolumeEnvelope ( MVOL_R_Value, MVOL_R_Cycles, Regs [ ( MVOL_R - SPU_X ) >> 1 ] & 0x7f, ( Regs [ ( MVOL_R - SPU_X ) >> 1 ] >> 13 ) & 0x3 );
				
				Regs [ ( CMVOL_R - SPU_X ) >> 1 ] = MVOL_R_Value;
			}
			else
			{
				// just set the current master volume R
				Regs [ ( CMVOL_R - SPU_X ) >> 1 ] = Regs [ ( MVOL_R - SPU_X ) >> 1 ] << 1;
			}
			
			/////////////////////////////////////////////////////////////////////
			// update ADSR envelope
			
			// check adsr status
			switch ( ADSR_Status [ Channel ] )
			{
				case ADSR_MUTE:
				
					VOL_ADSR_Value [ Channel ] = 0;
					Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ] = 0;
				
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
					
#ifdef INLINE_DEBUG_RUN
	debug << "; (before) VOL_ADSR_Value=" << hex << VOL_ADSR_Value [ Channel ] << "; Cycles=" << dec << Cycles [ Channel ];
#endif

					ModeRate = Regs [ ( ADSR_0 >> 1 ) + ( Channel << 3 ) ];
					VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 8 ) & 0x7f, ( ModeRate >> 15 ) << 1 );
					
#ifdef INLINE_DEBUG_RUN
	debug << "; (after) VOL_ADSR_Value=" << hex << VOL_ADSR_Value [ Channel ] << "; Cycles=" << dec << Cycles [ Channel ] << "; Value=" << hex << ( ( ModeRate >> 8 ) & 0x7f ) << "; flags=" << ( ( ModeRate >> 15 ) << 1 );
#endif

					// check if current volume < 0x6000
					
					// saturate and switch to decay mode if volume goes above 32767
					if ( VOL_ADSR_Value [ Channel ] >= 32767 )
					{
#ifdef INLINE_DEBUG_RUN
	debug << "; DECAY_NEXT";
#endif

						VOL_ADSR_Value [ Channel ] = 32767;
						ADSR_Status [ Channel ] = ADSR_DECAY;
						
						// start envelope for decay mode
						ModeRate = Regs [ ( ADSR_0 >> 1 ) + ( Channel << 3 ) ];
						Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ( ModeRate >> 4 ) & 0xf ) << ( 2 ), 0x3 );
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


#ifdef INLINE_DEBUG_RUN
	debug << "; (before) VOL_ADSR_Value=" << hex << VOL_ADSR_Value [ Channel ] << "; Cycles=" << dec << Cycles [ Channel ];
#endif

					ModeRate = Regs [ ( ADSR_0 >> 1 ) + ( Channel << 3 ) ];
					VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ( ModeRate >> 4 ) & 0xf ) << ( 2 ), 0x3 );
					
#ifdef INLINE_DEBUG_RUN
	debug << "; (after) VOL_ADSR_Value=" << hex << VOL_ADSR_Value [ Channel ] << "; Cycles=" << dec << Cycles [ Channel ] << "; Value=" << hex << ( ( ModeRate >> 8 ) & 0x7f ) << "; flags=" << ( ( ModeRate >> 15 ) << 1 );
#endif

					// saturate if volume goes below zero
					// *** TODO *** ADSR volume probably can't go below zero in decay mode since it is always an exponential decrease
					if ( VOL_ADSR_Value [ Channel ] < 0 )
					{
						VOL_ADSR_Value [ Channel ] = 0;
					}
					
					// switch to sustain mode if we reach sustain level
					if ( VOL_ADSR_Value [ Channel ] <= VOL_SUSTAIN_Level [ Channel ] )
					{
#ifdef INLINE_DEBUG_RUN
	debug << "; SUSTAIN_NEXT";
#endif

						VOL_ADSR_Value [ Channel ] = VOL_SUSTAIN_Level [ Channel ];
						ADSR_Status [ Channel ] = ADSR_SUSTAIN;
						
						// start envelope for sustain mode
						ModeRate = Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
						Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 6 ) & 0x7f, ModeRate >> 14 );
					}
					
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
					
					ModeRate = Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
					VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 6 ) & 0x7f, ModeRate >> 14 );

#ifdef INLINE_DEBUG_RUN
	debug << "; (after) VOL_ADSR_Value=" << hex << VOL_ADSR_Value [ Channel ] << "; Cycles=" << dec << Cycles [ Channel ] << "; Value=" << hex << ( ( ModeRate >> 8 ) & 0x7f ) << "; flags=" << ( ( ModeRate >> 15 ) << 1 );
#endif

					
					// saturate if volume goes above 32767
					if ( VOL_ADSR_Value [ Channel ] > 32767 )
					{
						VOL_ADSR_Value [ Channel ] = 32767;
					}
					
					// or below zero
					if ( VOL_ADSR_Value [ Channel ] < 0 )
					{
						VOL_ADSR_Value [ Channel ] = 0;
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
					}
					else
					{
						// RELEASE mode //
						
						// *** note *** it is possible for ADSR volume to go negative for 1T in linear decrement mode //
						
						ModeRate = Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
						VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
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

					// check if linear or exponential (adsr1 bit 5)
					
						
					break;
			}
			
			/////////////////////////////////////////////////////////////////////////////
			// update ADSR Envelope Volume
			Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ] = VOL_ADSR_Value [ Channel ];
			
			//////////////////////////////////////////////////////////////
			// load sample
			
			// check if channel is set to noise
			if ( ChannelNoise & ( 1 << Channel ) )
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
				
				Sample = Calc_sample_gx ( CurrentSample_Read [ Channel ] >> 16, Sample, DecodedBlocks [ ( Channel << 5 ) + ( ( SampleNumber - 1 ) & 0x1f ) ],
				DecodedBlocks [ ( Channel << 5 ) + ( ( SampleNumber - 2 ) & 0x1f ) ], DecodedBlocks [ ( Channel << 5 ) + ( ( SampleNumber - 3 ) & 0x1f ) ] );
				
				
				////////////////////////////////////
				// apply envelope volume
				// this does not apply when set to noise
				Sample = ( Sample * ( (s64) ( (s16) Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ] ) ) ) >> c_iVolumeShift;
				
				UpdatePitch ( Channel, Regs [ ( ( Channel << 4 ) + PITCH ) >> 1 ], PitchMod, PreviousSample );
				
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
			if ( ( ( ( (u32) Regs [ ( IRQA - SPU_X ) >> 1 ] ) << 3 ) == ( DecodeBufferOffset + 0x800 ) ) && ( Regs [ ( CTRL - SPU_X ) >> 1 ] & 0x40 ) )
			{
				// we have reached irq address - trigger interrupt
				SetInterrupt ();
				
				// interrupt
				Regs [ ( STAT - SPU_X ) >> 1 ] |= 0x40;
			}
			
			if ( ( ( ( (u32) Regs [ ( IRQA - SPU_X ) >> 1 ] ) << 3 ) == ( DecodeBufferOffset + 0xc00 ) ) && ( Regs [ ( CTRL - SPU_X ) >> 1 ] & 0x40 ) )
			{
				// we have reached irq address - trigger interrupt
				SetInterrupt ();
				
				// interrupt
				Regs [ ( STAT - SPU_X ) >> 1 ] |= 0x40;
			}
			
			//////////////////////////////////////////////////////////////////
			// apply volume processing
			
			
			// apply channel volume
			//ChSampleL = ( Sample * ( VOL_L_Value [ Channel ] >> 16 ) ) >> c_iVolumeShift;
			//ChSampleR = ( Sample * ( VOL_R_Value [ Channel ] >> 16 ) ) >> c_iVolumeShift;
			ChSampleL = ( Sample * ((s64) ((s16)Regs [ ( CVOL_L_START - SPU_X + ( Channel << 1 ) ) >> 1 ]) ) ) >> c_iVolumeShift;
			ChSampleR = ( Sample * ((s64) ((s16)Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ]) ) ) >> c_iVolumeShift;

			
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
			if ( ReverbOn & ( 1 << Channel ) )
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
						ChannelOn |= ( 1 << Channel );
					}

					// check if envelope should be killed
					//if ( KillEnvelope_Bitmap & ( 1 << Channel ) )
					if ( ( !( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] & ( 0x2 << 8 ) ) )  )
					{
						// if channel is not already set to mute, then turn off reverb for channel and mark we passed end of sample data
						if ( ADSR_Status [ Channel ] != ADSR_MUTE )
						{
							// turn off reverb for channel
							ReverbOn &= ~( 1 << Channel );
						}
						
						// kill envelope //
						ADSR_Status [ Channel ] = ADSR_MUTE;
						VOL_ADSR_Value [ Channel ] = 0;
					}
					
					// set address of next sample block to be loop start address
					CurrentBlockAddress [ Channel ] = ( ( ((u32)Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ]) << 3 ) & c_iRam_Mask );
				}
				else
				{
					// did not reach loop/end flag //
					
					// advance to address of next sample block
					CurrentBlockAddress [ Channel ] += 16;
					CurrentBlockAddress [ Channel ] &= c_iRam_Mask;
				}
				
				
				//////////////////////////////////////////////////////////////////////////////
				// Check loop start flag and set loop address if needed
				// LOOP_START is bit 3 actually
				// note: LOOP_START is actually bit 2
				if ( ( RAM [ ( CurrentBlockAddress [ Channel ] & c_iRam_Mask ) >> 1 ] & ( 0x4 << 8 ) )  )
				{
					///////////////////////////////////////////////////
					// we are at loop start address
					// set loop start address
					Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ] = ( CurrentBlockAddress [ Channel ] >> 3 );
					
					// clear killing of envelope
					//KillEnvelope_Bitmap &= ~( 1 << Channel );
				}
				
				//////////////////////////////////////////////////////////////////////////////
				// check if the IRQ address is in this block and if interrupts are enabled
				if ( ( ( CurrentBlockAddress [ Channel ] >> 4 ) == ( Regs [ ( IRQA - SPU_X ) >> 1 ] >> 1 ) ) && ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] & 0x40 ) )
				{
					// we have reached irq address - trigger interrupt
					SetInterrupt ();
					
					// interrupt
					Regs [ ( 0x1f801dae - SPU_X ) >> 1 ] |= 0x40;
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
	if ( ! ( Regs [ ( 0x1f801daa - SPU_X ) >> 1 ] & 0x4000 ) )
	{
		SampleL = 0;
		SampleR = 0;
	}
	
	
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
			tVOL_L = (s64) ( (s16) Regs [ ( CDVOL_L - SPU_X ) >> 1 ] );
			tVOL_R = (s64) ( (s16) Regs [ ( CDVOL_R - SPU_X ) >> 1 ] );
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
			if ( ( ( ( (u32) Regs [ ( IRQA - SPU_X ) >> 1 ] ) << 3 ) == ( DecodeBufferOffset + 0x000 ) ) && ( Regs [ ( CTRL - SPU_X ) >> 1 ] & 0x40 ) )
			{
				// we have reached irq address - trigger interrupt
				SetInterrupt ();
				
				// interrupt
				Regs [ ( STAT - SPU_X ) >> 1 ] |= 0x40;
			}
			
			if ( ( ( ( (u32) Regs [ ( IRQA - SPU_X ) >> 1 ] ) << 3 ) == ( DecodeBufferOffset + 0x400 ) ) && ( Regs [ ( CTRL - SPU_X ) >> 1 ] & 0x40 ) )
			{
				// we have reached irq address - trigger interrupt
				SetInterrupt ();
				
				// interrupt
				Regs [ ( STAT - SPU_X ) >> 1 ] |= 0x40;
			}
			
		//}
		
		// check if cd audio output is enabled
		if ( REG ( CTRL ) & 0x1 )
		{
			// mix
			SampleL += CD_SampleL;
			SampleR += CD_SampleR;
			
			// multiply by gain??
			//SampleL *= c_iMixer_Gain;
			//SampleR *= c_iMixer_Gain;
			
			// check if reverb is on for cd
			if ( REG ( CTRL ) & 0x4 )
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
	// still need to fix this so it uses the "current master volume" register
	//SampleL = ( SampleL * ( MVOL_L_Value >> 16 ) ) >> c_iVolumeShift;
	//SampleR = ( SampleR * ( MVOL_R_Value >> 16 ) ) >> c_iVolumeShift;
	SampleL = ( SampleL * ( (s64) ((s16)Regs [ ( CMVOL_L - SPU_X ) >> 1 ]) ) ) >> c_iVolumeShift;
	SampleR = ( SampleR * ( (s64) ((s16)Regs [ ( CMVOL_R - SPU_X ) >> 1 ]) ) ) >> c_iVolumeShift;
	
	// update current master volume registers
	//Regs [ ( 0x1f801db8 - SPU_X ) >> 1 ] = MVOL_L_Value >> 16;
	//Regs [ ( 0x1f801dba - SPU_X ) >> 1 ] = MVOL_R_Value >> 16;
	
	////////////////////////////////////////////////////////
	// Apply the Program's Global Volume set by user
	SampleL = ( SampleL * GlobalVolume ) >> c_iVolumeShift;
	SampleR = ( SampleR * GlobalVolume ) >> c_iVolumeShift;

	// mix samples
	Mixer [ ( Mixer_WriteIdx + 0 ) & c_iMixerMask ] = adpcm_decoder::clamp ( SampleL );
	Mixer [ ( Mixer_WriteIdx + 1 ) & c_iMixerMask ] = adpcm_decoder::clamp ( SampleR );
	
	// samples is now in mixer
	Mixer_WriteIdx += 2;
	
	// output one l/r sample
	if ( AudioOutput_Enabled && ( Mixer_WriteIdx - Mixer_ReadIdx ) >= PlayBuffer_Size )
	{
		int testvalue0, testvalue1;
		
		//if ( !hWaveOut ) cout << "\n!hWaveOut; p1\n";
		
		// make sure the read index at write index minus the size of the buffer
		Mixer_ReadIdx = Mixer_WriteIdx - PlayBuffer_Size;
		
		if ( ( Mixer_WriteIdx - Mixer_ReadIdx ) == PlayBuffer_Size )
		{
			// play buffer cannot hold any more data //
			
			
			//testvalue0 = waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
			//testvalue1 = waveOutUnprepareHeader( hWaveOut, &header1, sizeof(WAVEHDR) );
			
			
			//while ( !( ((volatile u32) (header.dwFlags)) & WHDR_DONE ) )
			//while ( waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) ) == WAVERR_STILLPLAYING )
			//while ( testvalue0 == WAVERR_STILLPLAYING && testvalue1 == WAVERR_STILLPLAYING )
			while ( !( header0.dwFlags & WHDR_DONE ) && !( header1.dwFlags & WHDR_DONE ) )
			{
				//cout << "\nWaiting for samples to finish playing...";
				
				//MsgWaitForMultipleObjectsEx( NULL, NULL, 1, QS_ALLINPUT, MWMO_ALERTABLE );
				
				//testvalue = waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
				testvalue0 = waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
				testvalue1 = waveOutUnprepareHeader( hWaveOut, &header1, sizeof(WAVEHDR) );
				
			}
		}
		
		//testvalue = waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
		//testvalue0 = waveOutUnprepareHeader( hWaveOut, &header0, sizeof(WAVEHDR) );
		//testvalue1 = waveOutUnprepareHeader( hWaveOut, &header1, sizeof(WAVEHDR) );
		
		//if ( !hWaveOut ) cout << "\n!hWaveOut; p4\n";
		
		//if ( header.dwFlags & WHDR_DONE )
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
				
				
				testvalue0 = waveOutWrite( hWaveOut, &header0, sizeof(WAVEHDR) );
				
				
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
				
				
				testvalue1 = waveOutWrite( hWaveOut, &header1, sizeof(WAVEHDR) );
				
				
				//cout << "\nSent enabled audio successfully.";
			}
			
			// data in mixer has been played now
			Mixer_ReadIdx = Mixer_WriteIdx;
		}
		
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
	GET_REG16 ( STAT ) &= ~( 0x200 << 2 );
	GET_REG16 ( STAT ) |= ( DecodeBufferOffset & 0x200 ) << 2;
	
	// write back ChannelOn
	//Regs [ ( ( CON_0 - SPU_X ) >> 1 ) & 0xff ] = (u16) ChannelOn;
	//Regs [ ( ( CON_1 - SPU_X ) >> 1 ) & 0xff ] = (u16) ( ChannelOn >> 16 );
	SET_REG32 ( CON_0, ChannelOn );
	
	// write back reverb on
	// SKIP REVERB FOR NOW
	//Regs [ ( RON_0 - SPU_X ) >> 1 ] = (u16) ReverbOn;
	//Regs [ ( RON_1 - SPU_X ) >> 1 ] = (u16) ( ReverbOn >> 16 );
	//SET_REG32( RON_0, ReverbOn );
}
*/



/*
u32 SPUCore::Read ( u32 Offset )
{
//#ifdef INLINE_DEBUG_READ
//	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
//#endif

	u32 lReg;

	if ( Offset >= ( c_iNumberOfRegisters << 1 ) ) return 0;
	
	// Read SPU register value
	lReg = Offset >> 1;

	switch ( Offset )
	{
		///////////////////////////////////
		// SPU Control
		case CTRL:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_CTRL
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; CTRL=" << hex << setw ( 4 ) << Regs [ lReg ];
#endif

			return Regs [ lReg ];
			break;
	
		////////////////////////////////////
		// SPU Status X
		case STAT_X:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_STAT_0
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; STAT_0= " << hex << setw ( 4 ) << Regs [ lReg ];
#endif

			// unknown what this is for - gets loaded with 4 when SPU gets initialized by BIOS
			return Regs [ lReg ];
			break;
			
		/////////////////////////////////////
		// SPU Status
		case STAT:
#if defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_STAT_1
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; STAT_1= " << hex << setw ( 4 ) << Regs [ lReg ];
#endif
			
			// bit 10 - Rd: 0 - SPU ready to transfer; 1 - SPU not ready
			// bit 11 - Dh: 0 - decoding in first half of buffer; 1 - decoding in second half of buffer
			
			return Regs [ lReg ];
			break;
			
		case KON_0:
#if defined INLINE_DEBUG_READ_CHANNELONOFF || defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_KON_0
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; KON_0= " << hex << setw ( 4 ) << Regs [ lReg ];
#endif

			// just return the SPU register value
			return Regs [ lReg ];
			
			break;

		case KON_1:
#if defined INLINE_DEBUG_READ_CHANNELONOFF || defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_KON_1
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; KON_1= " << hex << setw ( 4 ) << Regs [ lReg ];
#endif

			// just return the SPU register value
			return Regs [ lReg ];
			
			break;

		case KOFF_0:
#if defined INLINE_DEBUG_READ_CHANNELONOFF || defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_KOFF_0
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; KOFF_0= " << hex << setw ( 4 ) << Regs [ lReg ];
#endif

			// just return the SPU register value
			return Regs [ lReg ];
			
			break;
			
		case KOFF_1:
#if defined INLINE_DEBUG_READ_CHANNELONOFF || defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_KOFF_1
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; KOFF_1= " << hex << setw ( 4 ) << Regs [ lReg ];
#endif

			// just return the SPU register value
			return Regs [ lReg ];
			
			break;

		case CON_0:
#if defined INLINE_DEBUG_READ_CHANNELONOFF || defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_CON_0
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; CON_0= " << hex << setw ( 4 ) << Regs [ lReg ];
#endif

			// just return the SPU register value
			return Regs [ lReg ];
			
			break;
			
		case CON_1:
#if defined INLINE_DEBUG_READ_CHANNELONOFF || defined INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_CON_1
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	debug << "; CON_1= " << hex << setw ( 4 ) << Regs [ lReg ];
#endif

			// just return the SPU register value
			return Regs [ lReg ];
			
			break;
			
			
		default:
#ifdef INLINE_DEBUG_READ || defined INLINE_DEBUG_READ_CON_DEFAULT
	debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	//debug << "; " << _SPU->RegisterNames [ lReg ] << "=";
	debug << " Reg=";
	debug << hex << setw ( 4 ) << Regs [ lReg ];
#endif

			// just return the SPU register value
			// don't AND with 0xff
			return Regs [ lReg ];
			
			break;
			
	}

	
}
*/


/*
void SPUCore::Write ( u32 Address, u32 Data, u32 Mask )
{
//#ifdef INLINE_DEBUG_WRITE
//	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
//#endif

	u32 Channel;
	u32 Rate;
	
	u16 ModeRate;
	
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
	// Voice Data area #1 for PS2 is offsets 0-0x180
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
				}
	
			
				break;
				
			case VOL_R:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_VOL_R
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)VOL_R=" << Regs [ ( ( Channel << 4 ) + VOL_R ) >> 1 ];
#endif
			
				// writing constant volume
				Regs [ ( ( Channel << 4 ) + VOL_R ) >> 1 ] = Data;

				if ( Data >> 15 )
				{
					Start_VolumeEnvelope ( VOL_R_Value [ Channel ], VOL_R_Cycles [ Channel ], Data & 0x7f, ( Data >> 13 ) & 0x3 );
				}
				else
				{
					// store the new current volume left
					Regs [ ( CVOL_R_START - SPU_X + ( Channel << 1 ) ) >> 1 ] = Data << 1;
				}
				
			
				break;
				
			case PITCH:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_PITCH
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)PITCH=" << Regs [ ( ( Channel << 4 ) + PITCH ) >> 1 ];
#endif
			
				// only bits 0-13 are used
				Data &= 0x3fff;
				
				/////////////////////////////////////////////////////////////
				// Pitch of the sound
				// frequency = (pitch/4096) * f0
				Regs [ ( ( Channel << 4 ) + PITCH ) >> 1 ] = Data;
				

				// (32.32)dSampleDT = Pitch / 4096
				// for PS2 this would be (32.32)dSampleDT = ( 48 / SampleRateInKHZ ) * ( Pitch / 4096 )
				// where SampleRateInKHZ is 41 for a 41000 Hz sample rate
				// *** testing *** copy pitch over on key on
				//_SPU->dSampleDT [ Channel ] = ( ((u64)Data) << 32 ) >> 12;
				
				break;
				
			case SSA_X:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_SSA_X
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)SSA_X=" << Regs [ ( ( Channel << 4 ) + SSA_X ) >> 1 ];
#endif

				// align ??
				//Data = ( Data + 1 ) & ~1;
				Data &= ~1;
			
				////////////////////////////////////////////
				// writing start address for the sound
				Regs [ ( ( Channel << 4 ) + SSA_X ) >> 1 ] = Data;
				
				// *** testing ***
				//_SPU->Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ] = Data;
				
				break;
				
			case ADSR_0:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ADSR_0
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)ADSR_0=" << Regs [ ( ( Channel << 4 ) + ADSR_0 ) >> 1 ];
#endif
			
				//////////////////////////////////////////////
				// writing ADSR_0 register
				Regs [ ( ( Channel << 4 ) + ADSR_0 ) >> 1 ] = Data;
				
				// set sustain level (48.16 fixed point)
				// this is now 16.0 fixed point
				VOL_SUSTAIN_Level [ Channel ] = ( ( Data & 0xf ) + 1 ) << ( 11 );
				
				// set decay rate
				Set_ExponentialDecRates ( Channel, ( ( Data >> 4 ) & 0xf ) << 2, (s32*) VOL_DECAY_Constant, (s32*) VOL_DECAY_Constant );
				
				// set attack rate
				// need to know if this is linear or pseudo exponential increase
				if ( Data >> 15 )
				{
					/////////////////////////////////////////////////////////
					// pseudo exponential increase
					Set_ExponentialIncRates ( Channel, ( Data >> 8 ) & 0x7f, (s32*) VOL_ATTACK_Constant, (s32*) VOL_ATTACK_Constant75 );
				}
				else
				{
					/////////////////////////////////////////////////////////////
					// linear increase
					Set_LinearIncRates ( Channel, ( Data >> 8 ) & 0x7f, (s32*) VOL_ATTACK_Constant, (s32*) VOL_ATTACK_Constant75 );
				}
				
				break;
				
			case ADSR_1:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ADSR_1
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)ADSR_1=" << Regs [ ( ( Channel << 4 ) + ADSR_1 ) >> 1 ];
#endif
			
				///////////////////////////////////////////////////
				// writing ADSR_1 register
				Regs [ ( ( Channel << 4 ) + ADSR_1 ) >> 1 ] = Data;
				
				// set release rate
				// check if release rate is linear or exponential
				if ( ( Data >> 5 ) & 1 )
				{
					/////////////////////////////////////////////////
					// exponential decrease
					Set_ExponentialDecRates ( Channel, ( Data & 0x1f ) << 2, (s32*) VOL_RELEASE_Constant, (s32*) VOL_RELEASE_Constant );
				}
				else
				{
					/////////////////////////////////////////////////
					// linear decrease
					Set_LinearDecRates ( Channel, ( Data & 0x1f ) << 2, (s32*) VOL_RELEASE_Constant, (s32*) VOL_RELEASE_Constant );
				}
				
				// set sustain rate
				switch ( ( Data >> 14 ) & 3 )
				{
					case 0:
					
						//////////////////////////////////////////////////////////////////
						// linear increase
						Set_LinearIncRates ( Channel, ( Data >> 6 ) & 0x7f, (s32*) VOL_SUSTAIN_Constant, (s32*) VOL_SUSTAIN_Constant75 );
						
						break;
					
					case 1:
					
						////////////////////////////////////////////////////////////////////
						// linear decrease
						Set_LinearDecRates ( Channel, ( Data >> 6 ) & 0x7f, (s32*) VOL_SUSTAIN_Constant, (s32*) VOL_SUSTAIN_Constant75 );
						
						break;
					
					case 2:
					
						/////////////////////////////////////////////////////////////////////
						// pseudo exponential increase
						Set_ExponentialIncRates ( Channel, ( Data >> 6 ) & 0x7f, (s32*) VOL_SUSTAIN_Constant, (s32*) VOL_SUSTAIN_Constant75 );
						
						break;
					
					case 3:
					
						/////////////////////////////////////////////////////////
						// exponential decrease
						Set_ExponentialDecRates ( Channel, ( Data >> 6 ) & 0x7f, (s32*) VOL_SUSTAIN_Constant, (s32*) VOL_SUSTAIN_Constant75 );
						
						break;
				}
				
				break;
				
			case ENV_X:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_ENV_X
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)ENV_X=" << Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ];
#endif
			
				////////////////////////////////////////////////
				// writing to current envelope volume register
				
				// *** TODO *** this can actually be set to cause a jump in the value //
				//if ( Data > 0x7fff ) Data = 0x7fff;
				Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ] = Data;
				
				// *** testing *** allow for jump in adsr value at any time
				VOL_ADSR_Value [ Channel ] = Data;
				
				break;
				
			case LSA_X:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_LSA_X
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; Channel#" << Channel;
	debug << "; (before)LSA_X=" << Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ];
	debug << "; CyclesFromStart=" << dec << ( CycleCount - StartCycle_Channel [ Channel ] );
#endif
			
				// align ??
				//Data = ( Data + 1 ) & ~1;
				Data &= ~1;
				
				///////////////////////////////////////////////////////
				// writing to loop start address for sound
				// this gets set by the sound when it is loaded
				Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ] = Data;
				
				// loop start was manually specified - so ignore anything specified in sample
				LSA_Manual_Bitmap |= ( 1 << Channel );
				
				break;
			
		}
	}
	else
	{
	
		/////////////////////////////////////////////////////////////
		// not writing to voice data area
	
		switch ( Address )
		{
			// reverb work address start
			case RVWAS_0:
			
				//ReverbWork_Start = ( Data & 0xffff ) << 3;
				//ReverbWork_Size = c_iRam_Size - _SPU->ReverbWork_Start;
				//Reverb_BufferAddress = _SPU->ReverbWork_Start;
				
				Regs [ RVWAS_0 >> 1 ] = (u16)Data;
				
				// *** TODO ***  make some sort of call to a "UpdateReverbWorkAddress" or something
				
				// check if interrupt triggered by reverb work address
				if ( ( Reverb_BufferAddress == ( ( (u32) Regs [ IRQA_0 >> 1 ] ) << 3 ) ) && ( Regs [ CTRL >> 1 ] & 0x40 ) )
				{
					// we have reached irq address - trigger interrupt
					SetInterrupt ();
					
					// interrupt
					Regs [ STAT >> 1 ] |= 0x40;
				}
				
				break;
			
			case RVWAS_1:
			
				//ReverbWork_Start = ( Data & 0xffff ) << 3;
				//ReverbWork_Size = c_iRam_Size - _SPU->ReverbWork_Start;
				//Reverb_BufferAddress = _SPU->ReverbWork_Start;
				
				Regs [ RVWAS_1 >> 1 ] = (u16)Data;
				
				// *** TODO ***  make some sort of call to a "UpdateReverbWorkAddress" or something
				
				// check if interrupt triggered by reverb work address
				if ( ( Reverb_BufferAddress == ( ( (u32) Regs [ IRQA_0 >> 1 ] ) << 3 ) ) && ( Regs [ CTRL >> 1 ] & 0x40 ) )
				{
					// we have reached irq address - trigger interrupt
					SetInterrupt ();
					
					// interrupt
					Regs [ STAT >> 1 ] |= 0x40;
				}
				
				break;
			
			//////////////////////////////////////////////////////////////////////////////
			// irq address - reading this address in sound buffer causes SPU interrupt
			case IRQA_0:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_IRQA
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)IRQA_0=" << Regs [ IRQA_0 >> 1 ];
#endif

				Regs [ IRQA_0 >> 1 ] = (u16)Data;
				
				break;
			
			case IRQA_1:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_IRQA
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)IRQA_1=" << Regs [ IRQA_1 >> 1 ];
#endif

				Regs [ IRQA_1 >> 1 ] = (u16)Data;
				
				break;
			
			////////////////////////////////////////
			// Sound Buffer Address
			case SBA_0:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_SBA
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)SBA=" << Regs [ SBA_0 >> 1 ];
#endif
			
				///////////////////////////////////
				// set next sound buffer address
				NextSoundBufferAddress = ( Data << 3 ) & c_iRam_Mask;
				Regs [ SBA_0 >> 1 ] = (u16)Data;

				break;

			////////////////////////////////////////
			// Sound Buffer Address
			case SBA_1:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_SBA
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)SBA=" << Regs [ SBA_1 >> 1 ];
#endif
			
				///////////////////////////////////
				// set next sound buffer address
				NextSoundBufferAddress = ( Data << 3 ) & c_iRam_Mask;
				Regs [ SBA_1 >> 1 ] = (u16)Data;

				break;
				
			//////////////////////////////////////////
			// Data forwarding register
			case DATA:	//0x1f801da8
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_DATA
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; DATA; BufferIndex=" << BufferIndex;
#endif
			
				///////////////////////////////////////////////////////
				// Send data to sound buffer and update next address
				//RAM [ ( ( NextSoundBufferAddress & c_iRam_Mask ) >> 1 ) ] = (u16) Data;
				//NextSoundBufferAddress += 2;
				
				///////////////////////////////////////////////////////////////
				// Actually we're supposed to send the data into SPU buffer
				
				// buffer can be written into at any time
				if ( BufferIndex < 32 )
				{
					Buffer [ BufferIndex++ ] = (u16) Data;
				}
				
				break;
				
			///////////////////////////////////
			// SPU Control
			case CTRL:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CTRL
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)CTRL=" << _SPU->Regs [ CTRL >> 1 ];
#endif

				Regs [ CTRL >> 1 ] = (u16)Data;
				
				// copy bits 0-5 to stat
				Regs [ STAT >> 1 ] = ( Regs [ STAT >> 1 ] & 0xffc0 ) | ( Regs [ CTRL >> 1 ] & 0x3f );
				
				// copy bit 5 of ctrl to bit 7 of stat
				switch ( ( Regs [ CTRL >> 1 ] >> 5 ) & 0x3 )
				{
					case 0:
						// no reads or writes (stop)
						Regs [ STAT >> 1 ] = ( Regs [ STAT >> 1 ] & ~0x0380 ) | ( 0 << 7 );
						break;
						
					case 1:
						// manual write
						Regs [ STAT >> 1 ] = ( Regs [ STAT >> 1 ] & ~0x0380 ) | ( 0 << 7 );
						break;
						
					case 2:
						// dma write
						Regs [ STAT >> 1 ] = Regs [ STAT >> 1 ] & ~0x0380 ) | ( 0x3 << 7 );
						break;
						
					case 3:
						// dma read
						Regs [ STAT >> 1 ] = ( Regs [ STAT >> 1 ] & ~0x0380 ) | ( 0x5 << 7 );
						break;
				}
				
				// check if disabling/acknowledging interrupt
				if ( ! ( Data & 0x40 ) )
				{
					// clear interrupt
					Regs [ STAT >> 1 ] &= ~0x40;
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
					for ( int i = 0; i < BufferIndex; i++ )
					{
						_SPU->RAM [ ( ( _SPU->NextSoundBufferAddress + ( i << 1 ) ) & c_iRam_Mask ) >> 1 ] = Buffer [ i ];
					}
					
					//////////////////////////////////////////////////////////
					// update next sound buffer address
					//_SPU->NextSoundBufferAddress += 64;
					NextSoundBufferAddress += ( BufferIndex << 1 );
					
					//////////////////////////////////////////////////
					// reset buffer index
					BufferIndex = 0;
					
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
	debug << "; (before)INIT=" << Regs [ INIT >> 1 ];
#endif
				// unknown what this is for - gets loaded with 4 when SPU gets initialized by BIOS
				Regs [ INIT >> 1 ] = (u16)Data;
				

				break;
				
			/////////////////////////////////////
			// SPU Status
			case STAT:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_STAT
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)STAT=" << _SPU->Regs [ STAT >> 1 ];
#endif
				
				// bit 10 - Rd: 0 - SPU ready to transfer; 1 - SPU not ready
				// bit 11 - Dh: 0 - decoding in first half of buffer; 1 - decoding in second half of buffer
				//_SPU->Regs [ ( ( Address - SPU_X ) >> 1 ) & 0xff ] = (u16)Data;
				
				break;
				
				
			case MVOL_L:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_MVOL_L
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)MVOL_L=" << Regs [ MVOL_L >> 1 ];
#endif
			
				////////////////////////////////////////
				// Master Volume Left
				Regs [ MVOL_L >> 1 ] = (u16) Data;
				
				if ( Data >> 15 )
				{
					Start_VolumeEnvelope ( MVOL_L_Value, MVOL_L_Cycles, Data & 0x7f, ( Data >> 13 ) & 0x3 );
				}
				else
				{
					// store the new current volume left
					Regs [ MVOLX_L >> 1 ] = Data << 1;
				}
				
				break;
				
			case MVOL_R:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_MVOL_R
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)MVOL_R=" << Regs [ MVOL_R >> 1 ];
#endif
			
				////////////////////////////////////////
				// Master Volume Right
				Regs [ MVOL_R >> 1 ] = (u16) Data;
				
				if ( Data >> 15 )
				{
					Start_VolumeEnvelope ( MVOL_R_Value, MVOL_R_Cycles, Data & 0x7f, ( Data >> 13 ) & 0x3 );
				}
				else
				{
					// store the new current volume left
					Regs [ MVOLX_R >> 1 ] = Data << 1;
				}
				
				
				break;
				
			case EVOL_L:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_EVOL_L
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)EVOL_L=" << Regs [ EVOL_L >> 1 ];
#endif
			
				////////////////////////////////////////
				// Effect Volume Left
				Regs [ EVOL_L >> 1 ] = (u16) Data;
				
				break;
				
			case EVOL_R:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_EVOL_R
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)EVOL_R=" << Regs [ EVOL_R >> 1 ];
#endif
			
				////////////////////////////////////////
				// Effect Volume Right
				Regs [ EVOL_R >> 1 ] = (u16) Data;
				
				break;
				
			case KON_0:
#if defined INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "\r\nKON_0; Data=" << hex << setw ( 4 ) << Data << " (Before) KON_0=" << Regs [ KON_0 >> 1 ] << " KOFF_0=" << Regs [ KOFF_0 >> 1 ] << " CON_0=" << Regs [ CON_0 >> 1 ];
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_KON_0
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)KON_0=" << Regs [ KON_0 >> 1 ];
#endif
			
				/////////////////////////////////////////////
				// Key On 0-15
				
				// when keyed on set channel ADSR to attack mode
				for ( Channel = 0; Channel < 16; Channel++ )
				{
					if ( ( 1 << Channel ) & Data )
					{
						Start_SampleDecoding ( Channel );
					}
				}
				
				// store to SPU register
				// just set value
				//_SPU->Regs [ ( KON_0 - SPU_X ) >> 1 ] |= (u16) Data;
				Regs [ KON_0 >> 1 ] = (u16) Data;
				
				// clear channel in key off
				// don't touch this here
				//_SPU->Regs [ ( KOFF_0 - SPU_X ) >> 1 ] &= ~((u16) Data);
				
				// logical Or with channel on/off register
				// unknown if channel on/off register is inverted or not
				// actually this has nothing to do with a channel being on or anything like that, so clear the bits
				Regs [ CON_0 >> 1 ] &= ~( (u16) Data );
				
#if defined INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "; (After) KON_0=" << hex << setw ( 4 ) << _SPU->Regs [ ( KON_0 >> 1 ) ] << " KOFF_0=" << _SPU->Regs [ KOFF_0 >> 1 ] << " CON_0=" << _SPU->Regs [ CON_0 >> 1 ];
#endif
				break;
				
			case KON_1:
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "\r\nKON_1; Data=" << hex << setw ( 4 ) << Data << " (Before) KON_1=" << Regs [ KON_1 >> 1 ] << " KOFF_1=" << Regs [ KOFF_1 >> 1 ] << " CON_1=" << Regs [ CON_1 >> 1 ];
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_KON_1
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)KON_1=" << Regs [ KON_1 >> 1 ];
#endif
				/////////////////////////////////////////////
				// Key on 16-23
				
				////////////////////////////////////////////////////////////////////////
				// upper 8 bits of register are zero and ignored
				Data &= 0xff;
				
				// on key on we need to change ADSR mode to attack mode
				for ( Channel = 16; Channel < 24; Channel++ )
				{
					if ( ( 1 << ( Channel - 16 ) ) & Data )
					{
						Start_SampleDecoding ( Channel );
					}
				}
				
				// store to SPU register
				// just set value
				Regs [ KON_1 >> 1 ] = (u16) Data;

				// logical Or with channel on/off register
				// this isn't channel on/off.. clear the bits
				Regs [ CON_1 >> 1 ] &= ~( (u16) Data );
				
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "; (After) KON_1=" << hex << setw ( 4 ) << _SPU->Regs [ KON_1 >> 1 ] << " KOFF_1=" << _SPU->Regs [ KOFF_1 >> 1 ] << " CON_1=" << _SPU->Regs [ CON_1 >> 1 ];
#endif
				break;
			
				
				
			
				
			case KOFF_0:
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "\r\nKOFF_0; Data=" << hex << setw ( 4 ) << Data << "(Before) KON_0=" << Regs [ KON_0 >> 1 ] << " KOFF_0=" << Regs [ KOFF_0 >> 1 ] << " CON_0=" << Regs [ CON_0 >> 1 ];
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_KOFF_0
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)KOFF_0=" << _SPU->Regs [ KOFF_0 >> 1 ];
#endif
			
				/////////////////////////////////////////////
				// Key off 0-15
				
				// on key off we need to change ADSR mode to release mode
				for ( Channel = 0; Channel < 16; Channel++ )
				{
					if ( ( 1 << Channel ) & Data )
					{
						// put channel in adsr release phase unconditionally
						_SPU->ADSR_Status [ Channel ] = ADSR_RELEASE;
						
						// start envelope for release mode
						ModeRate = _SPU->Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
						Start_VolumeEnvelope ( _SPU->VOL_ADSR_Value [ Channel ], _SPU->Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
						
						// loop address not manually specified
						// can still change loop address after sound starts
						_SPU->LSA_Manual_Bitmap &= ~( 1 << Channel );
					}
				}
				
				// store to SPU register
				// just write value
				Regs [ KOFF_0 >> 1 ] = (u16) Data;
				
				// clear channel in key on
				// note: don't touch key on
				//_SPU->Regs [ ( KON_0 - SPU_X ) >> 1 ] &= ~((u16) Data);

				// temp: for now, also make sure channel is on when set to key off
				//_SPU->Regs [ ( CON_0 - SPU_X ) >> 1 ] |= (u16) Data;
				
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "; (After) KON_0=" << hex << setw ( 4 ) << _SPU->Regs [ ( KON_0 >> 1 ) ] << " KOFF_0=" << _SPU->Regs [ KOFF_0 >> 1 ] << "CON_0=" << _SPU->Regs [ CON_0 >> 1 ];
#endif
				
				break;
				
			case KOFF_1:
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF || defined INLINE_DEBUG_WRITE
			debug << "\r\nKOFF_1; Data=" << hex << setw ( 4 ) << Data << " (Before) KON_1=" << _SPU->Regs [ ( KON_1 >> 1 ) ] << " KOFF_1=" << _SPU->Regs [ KOFF_1 >> 1 ] << " CON_1=" << _SPU->Regs [ CON_1 >> 1 ];
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_KOFF_1
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)KOFF_1=" << _SPU->Regs [ KOFF_1 >> 1 ];
#endif
			
				/////////////////////////////////////////////
				// Key off 16-23

				////////////////////////////////////////////////////////////////////////
				// upper 8 bits of register are zero and ignored
				Data &= 0xff;
				
				// on key off we need to change ADSR mode to release mode
				for ( Channel = 16; Channel < 24; Channel++ )
				{
					if ( ( 1 << ( Channel - 16 ) ) & Data )
					{
						// put channel in adsr release phase unconditionally
						_SPU->ADSR_Status [ Channel ] = ADSR_RELEASE;
						
						// start envelope for release mode
						ModeRate = Regs [ ( ADSR_1 >> 1 ) + ( Channel << 3 ) ];
						Start_VolumeEnvelope ( _SPU->VOL_ADSR_Value [ Channel ], _SPU->Cycles [ Channel ], ( ModeRate & 0x1f ) << ( 2 ), ( ( ( ModeRate >> 5 ) & 1 ) << 1 ) | 0x1 );
						
						// loop address not manually specified
						// can still change loop address after sound starts
						_SPU->LSA_Manual_Bitmap &= ~( 1 << Channel );
					}
				}
				
				// store to SPU register (upper 24 bits of register are zero)
				// just write value
				//_SPU->Regs [ ( KOFF_1 - SPU_X ) >> 1 ] |= (u16) Data;
				Regs [ KOFF_1 >> 1 ] = (u16) Data;

#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "; (After) KON_1=" << hex << setw ( 4 ) << _SPU->Regs [ ( KON_1 >> 1 ) ] << " KOFF_1=" << _SPU->Regs [ KOFF_1 >> 1 ] << " CON_1=" << _SPU->Regs [ CON_1 >> 1 ];
#endif
				break;
			
				
			case CON_0:
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "\r\nCON_0; Data=" << hex << setw ( 4 ) << Data << "(Before) KON_0=" << Regs [ KON_0 >> 1 ] << " KOFF_0=" << Regs [ KOFF_0 >> 1 ] << " CON_0=" << Regs [ CON_0 >> 1 ];
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CON_0
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)CON_0=" << Regs [ CON_0 >> 1 ];
#endif

				///////////////////////////////////////////////////////
				// *todo* this register does not get written to
				
				// logical Or with channel on/off register
				// more of a read-only register. modifying it only changes the value momentarily, then it gets set back
				// should set register to zero
				Regs [ CON_0 >> 1 ] = 0;	//(u16) Data;
				
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "; (After) KON_0=" << hex << setw ( 4 ) << Regs [ KON_0 >> 1 ] << " KOFF_0=" << Regs [ KOFF_0 >> 1 ] << " CON_0=" << Regs [ CON_0 >> 1 ];
#endif
				break;
			
			case CON_1:
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "\r\nCON_1; Data=" << hex << setw ( 4 ) << Data << " (Before) KON_1=" << Regs [ KON_1 >> 1 ] << " KOFF_1=" << Regs [ KOFF_1 >> 1 ] << " CON_1=" << Regs [ CON_1 >> 1 ];
#endif
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_CON_1
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	debug << "; (before)CON_1=" << Regs [ CON_1 >> 1 ];
#endif

				///////////////////////////////////////////////////////
				// *todo* this register does not get written to
			
				////////////////////////////////////////////////////////////////////////
				// upper 8 bits of register are zero and ignored
				//Data &= 0xff;
				
				// logical Or with channel on/off register
				// more of a read-only register. modifying it only changes the value momentarily, then it gets set back
				// should set register to zero
				Regs [ CON_1 >> 1 ] = 0;	//(u16) Data;
				
#ifdef INLINE_DEBUG_WRITE_CHANNELONOFF
			debug << "; (After) KON_1=" << hex << setw ( 4 ) << Regs [ KON_1 >> 1 ] << " KOFF_1=" << Regs [ KOFF_1 >> 1 ] << " CON_1=" << Regs [ CON_1 >> 1 ];
#endif
				break;
				
			case MVOLX_L:
			
				// current master volume left //
				MVOL_L_Value = ((s32) ((s16) Data)) << 16;
				Regs [ MVOLX_L >> 1 ] = Data;
				
				break;
				
			case MVOLX_R:
			
				// current master volume right //
				MVOL_R_Value = ((s32) ((s16) Data)) << 16;
				Regs [ MVOLX_R >> 1 ] = Data;
			
				break;
				
			default:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_WRITE_DEFAULT
	debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Writing: " << Data;
	//debug << "; (before)" << _SPU->RegisterNames [ Offset >> 1 ] << "=";
	debug << " Reg=" << hex << Regs [ Offset >> 1 ];
#endif


				/////////////////////////////////////////////
				// by default just store to SPU regs
				_SPU->Regs [ Offset >> 1 ] = (u16) Data;
				break;
		}
	}
}
*/




bool SPUCore::DMA_ReadyForRead ( void )
{
	// check that spu is set to allow dma read
	if ( ( pCoreRegs0->CTRL & 0x30 ) != 0x30 )
	{
		// not set for dma read //
		return 0;
	}

	return true;
}


bool SPUCore::DMA_ReadyForWrite ( void )
{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "\r\SPUCore::DMA_ReadyForWrite: ";
	debug << " Cycle#" << dec << *_DebugCycleCount;
	debug << " Core#" << dec << CoreNumber;
#endif


	// check that spu is set to allow dma write
	if ( ( ( pCoreRegs0->CTRL & 0x30 ) != 0x20 ) && !isADMATransferMode () )
	{
		// not set for dma write //
		return 0;
	}


#ifdef ENABLE_AUTODMA_READY
	// check for ADMA transfer
	if ( isADMATransferMode () )
	{
#ifdef INLINE_DEBUG_DMA_WRITE
		debug << " TransferOffset=" << hex << NextSoundBufferAddress;
		debug << " PlayOffset=" << hex << ulADMA_PlayOffset;
		debug << " Diff=" << dec << (s32)(NextSoundBufferAddress-(ulADMA_PlayOffset<<1));
#endif

		// ADMA transfer //
		
#ifdef ENABLE_IMMEDIATE_ADMA
		// check if transfer is already in progress, then ok to continue if so
		if ( SoundDataInput_Enable )
		{
			return true;
		}
#endif

		// copy in buffer half when at least half of buffer is free
		//if ( ((s64) (NextSoundBufferAddress - (ulADMA_PlayOffset<<1))) <= 1024 )
		if ( ((s64) (NextSoundBufferAddress - ulADMA_PlayOffset)) <= 512 )
		{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << " HALF-BUFFER-FREE";
#endif

			// enable sound data input
			SoundDataInput_Enable = true;
			
			return true;
		}
		else
		{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << " BUFFER-PLAYING";
#endif

			return false;
		}
		
		/*
		// determine what half of buffer to transfer into
		ulBufferHalf_Offset = ( ( DecodeBufferOffset & 512 ) >> 1 );
		
		// I want to copy data into the other half of the buffer that is not currently being decoded/played
		ulBufferHalf_Offset ^= 256;
		
		ulBufferHalf_Offset = ( ulADMA_PlayOffset & 512 ) >> 9;
		if ( !bBufferFull [ ulBufferHalf_Offset ] )
		{
			ulBufferHalf_Offset = ( ulBufferHalf_Offset ) << 8;
		}
		else if ( !bBufferFull [ ulBufferHalf_Offset ^ 1 ] )
		{
			ulBufferHalf_Offset = ( ulBufferHalf_Offset ^ 1 ) << 8;
		}
		else
		{
			return false;
		}
		
#ifdef ENABLE_IMMEDIATE_ADMA
		// check if buffer that isn't playing has had a transfer of samples yet
		if ( bBufferFull [ ulBufferHalf_Offset >> 8 ] )
		{
			return false;
		}
		
		// start writing from beginning of buffer-half
		SoundDataInput_Offset = 0;
		
		// enable sound data input
		SoundDataInput_Enable = true;
		
		return true;
#endif
		
		// only allow ADMA transfer when given the "all clear" (with a remote start from SPU..)
		//return bADMATransfer;
		
		// just return false for now
		//return false;
		return SoundDataInput_Enable;
		*/
		
		// check if the half of buffer to write samples into is being played //
		// full buffer is 512 samples, half buffer is 256 samples
		/*
		if ( !( ( DecodeBufferOffset ^ ulADMA_Offset8 ) & 0x200 ) )
		{
			// don't know what might happen if offset is at the end of buffer - actually this would not matter if transferring at once
			// probably need this, so that the other audio dma channel can transfer when 256 L/R (512 samples) have transferred
			return false;
		}
		*/
	}
#endif
	
	return true;
}


/*
void SPUCore::DMA_Read ( u32* Data, int ByteReadCount )
{
	u32 Output;
	
	Output = ((u32*)RAM) [ ( ( NextSoundBufferAddress & c_iRam_Mask ) >> 2 ) ];

	//////////////////////////////////////////////////////////
	// update next sound buffer address
	NextSoundBufferAddress += 4;
	NextSoundBufferAddress &= c_iRam_Mask;
	
	////////////////////////////////////////////////////////////
	// save back into the sound buffer address register
	// this value does not update with transfer
	//Regs [ ( ( REG_SoundBufferAddress - SPU_X ) >> 1 ) & 0xff ] = NextSoundBufferAddress >> 3;
	
	Data [ 0 ] = Output;
}
*/


void SPUCore::DMA_Read_Block ( u32* Data, u32 BS )
{
#ifdef INLINE_DEBUG_DMA_READ
	debug << "\r\nDMA_Read: ";
	debug << "(before) NextSoundBufferAddress=" << hex << NextSoundBufferAddress;
#endif
	
	// TODO: check for interrupt (should interrupt on transfers TO and FROM spu ram //

	for ( int i = 0; i < (BS << 1); i++ )
	{
		// write the data into sound RAM
		((u16*) Data) [ i ] = RAM [ ( NextSoundBufferAddress + i ) & ( c_iRam_Mask >> 1 ) ];
		
#ifdef INLINE_DEBUG_DMA_READ_RECORD
	debug << " " << ((u16*) Data) [ i ];
#endif

	}

	
	//////////////////////////////////////////////////
	// reset buffer index
	//BufferIndex = 0;

	//////////////////////////////////////////////////////////
	// update next sound buffer address
	NextSoundBufferAddress += ( BS << 1 );
	
	NextSoundBufferAddress &= ( c_iRam_Mask >> 1 );
	
	// make sure address is aligned to sound block
	NextSoundBufferAddress &= ~7;

	// check for interrupt
	// check if address is set to trigger interrupt
	// or should it be + 0x20? or + 0x10?
	// *** todo*** update pointer + 0x10 or + 0x20 at end of transfer
	// ***todo*** only check for interrupt at end of transfer
	if ( SPU2::_SPU2->SPU0.pCoreRegs0->CTRL & 0x40 )
	{
		if ( ( NextSoundBufferAddress + 0x19 ) == ( ( SWAPH ( SPU2::_SPU2->SPU0.pCoreRegs0->IRQA ) ) & ( c_iRam_Mask >> 1 ) ) )
		{
#ifdef INLINE_DEBUG_INT
	debug << "\r\nSPU:INT:DMAREAD";
#endif

			// we have reached irq address - trigger interrupt
			SetInterrupt ();
			
			// do this for ps2
			SetInterrupt_Core ( 0 );
			
			// interrupt
			//pCoreRegs0->STAT |= 0x40;
			SPU2::_SPU2->SPU0.pCoreRegs0->STAT |= 0x40;
		}

	}	// end if ( SPU2::_SPU2->SPU0.pCoreRegs0->CTRL & 0x40 )

	if ( SPU2::_SPU2->SPU1.pCoreRegs0->CTRL & 0x40 )
	{
		if ( ( NextSoundBufferAddress + 0x19 ) == ( ( SWAPH ( SPU2::_SPU2->SPU1.pCoreRegs0->IRQA ) ) & ( c_iRam_Mask >> 1 ) ) )
		{
#ifdef INLINE_DEBUG_INT
	debug << "\r\nSPU:INT:DMAREAD";
#endif

			// we have reached irq address - trigger interrupt
			SetInterrupt ();
			
			// do this for ps2
			SetInterrupt_Core ( 1 );
			
			// interrupt
			//pCoreRegs0->STAT |= 0x40;
			SPU2::_SPU2->SPU1.pCoreRegs0->STAT |= 0x40;
		}

	}	// end if ( SPU2::_SPU2->SPU1.pCoreRegs0->CTRL & 0x40 )

}


/*
void SPUCore::DMA_Write ( u32* Data, int BlockSizeInWords32 )
{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "\r\nDMA_Write: ";
	debug << "(before) NextSoundBufferAddress=" << hex << NextSoundBufferAddress;
#endif

	///////////////////////////////////////////////////////////////
	// Actually we're supposed to send the data into SPU buffer
	((u32*)Buffer) [ BufferIndex >> 1 ] = Data [ 0 ];
	BufferIndex += 2;
	
	// TODO: check for interrupt (should interrupt on transfers TO and FROM spu ram //

	if ( _SPUCore->BufferIndex >= 32 )
	{
		///////////////////////////////////////////////////////
		// write SPU buffer into sound ram
		for ( int i = 0; i < 16; i++ ) ((u32*)RAM) [ ( ( NextSoundBufferAddress & c_iRam_Mask ) >> 2 ) + i ] = ((u32*)Buffer) [ i ];

		//////////////////////////////////////////////////
		// reset buffer index
		BufferIndex = 0;

		//////////////////////////////////////////////////////////
		// update next sound buffer address
		NextSoundBufferAddress += 64;
		NextSoundBufferAddress &= c_iRam_Mask;

		////////////////////////////////////////////////////////////
		// save back into the sound buffer address register
		//Regs [ ( ( REG_SoundBufferAddress - SPU_X ) >> 1 ) & 0xff ] = NextSoundBufferAddress >> 3;
	}
	
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "; (after) NextSoundBufferAddress=" << NextSoundBufferAddress;
#endif
}
*/


u64 SPU2::DMA_ReadyForRead_Core0 ()
{
	return _SPU2->SPU0.DMA_ReadyForRead ();
}

u64 SPU2::DMA_ReadyForRead_Core1 ()
{
	return _SPU2->SPU1.DMA_ReadyForRead ();
}

u64 SPU2::DMA_ReadyForWrite_Core0 ()
{
	return _SPU2->SPU0.DMA_ReadyForWrite ();
}

u64 SPU2::DMA_ReadyForWrite_Core1 ()
{
	return _SPU2->SPU1.DMA_ReadyForWrite ();
}


u32 SPU2::DMA_ReadBlock_Core0 ( u32* pMemoryPtr, u32 Address, u32 WordCount )
{
	u32 *Data;
	
	Data = & ( pMemoryPtr [ Address >> 2 ] );
	_SPU2->SPU0.DMA_Read_Block ( Data, WordCount );
	
	return WordCount;
}


u32 SPU2::DMA_ReadBlock_Core1 ( u32* pMemoryPtr, u32 Address, u32 WordCount )
{
	u32 *Data;
	
	Data = & ( pMemoryPtr [ Address >> 2 ] );
	_SPU2->SPU1.DMA_Read_Block ( Data, WordCount );
	
	return WordCount;
}


u32 SPU2::DMA_WriteBlock_Core0 ( u32* pMemoryPtr, u32 Address, u32 WordCount )
{
	u32 *Data;
	u32 WC;
	
	Data = & ( pMemoryPtr [ Address >> 2 ] );
	WC = _SPU2->SPU0.DMA_Write_Block ( Data, WordCount );
	
	return WC;
}


u32 SPU2::DMA_WriteBlock_Core1 ( u32* pMemoryPtr, u32 Address, u32 WordCount )
{
	u32 *Data;
	u32 WC;
	
	Data = & ( pMemoryPtr [ Address >> 2 ] );
	WC = _SPU2->SPU1.DMA_Write_Block ( Data, WordCount );
	
	return WC;
}


// BS is half the number of samples, so it is transferring BSx2 samples
u32 SPUCore::DMA_Write_Block ( u32* Data, u32 BS )
{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "\r\nDMA_Write: ";
	debug << " Cycle#" << dec << *_DebugCycleCount;
	debug << " Core#" << dec << CoreNumber;
	debug << " (before) NextSoundBufferAddress=" << hex << NextSoundBufferAddress << " BS=" << BS;
#endif


	
	// TODO: check for interrupt (should interrupt on transfers TO and FROM spu ram //
	
#ifdef ENABLE_AUTODMA_DMA
	// check if this is an ADMA transfer
	if ( isADMATransferMode () )
	{
#ifdef INLINE_DEBUG_DMA_WRITE
		debug << "\r\n***SPU2 RECEIVING AUTODMA DATA VIA DMA Core=" << CoreNumber;
		debug << " BA4=" << hex << Dma::pRegData [ 4 ]->BCR.BA;
		//debug << " BS4=" << hex << Dma::pRegData [ 4 ]->BCR.BS;
		debug << " BA7=" << hex << Dma::pRegData [ 7 ]->BCR.BA;
		debug << " ADMAOffset=" << hex << NextSoundBufferAddress;
		debug << " PlayOffset=" << hex << ulADMA_PlayOffset;
		debug << " Cycle#" << dec << *_DebugCycleCount;
		//debug << " NextCycle7#" << dec << Dma::_DMA->NextEventCh_Cycle [ 7 ];
#endif

		// ADMA transfer //
		
		/*
		if ( SoundDataInput_Offset >= 512 )
		{
			cout << "\nhps1x64: SPU2: ALERT: SoundDataInput_Offset >= 512 Before ADMA Transfer !!! =" << dec << SoundDataInput_Offset << "\n";
			SoundDataInput_Enable = false;
			
			// buffer-half is now filled with samples that haven't been played yet
			bBufferFull [ ulBufferHalf_Offset >> 8 ] = 1;
			
#ifdef INLINE_DEBUG_ENABLE_AUTODMA_INT
			// interrupt after half the buffer is transferred? SPU interrupt or DMA interrupt or both??
			// first need to check if interrupts are enabled
			if ( pCoreRegs0->CTRL & 0x40 )
			{
#ifdef INLINE_DEBUG_INT
	debug << "\r\nSPU#" << dec << CoreNumber << ":INT:ADMA";
#endif
#ifdef INLINE_DEBUG_ENABLE_AUTODMA_INT_SPU

				// trigger interrupt for transferring half of the buffer
				SetInterrupt ();
				
				// do this for ps2
				SetInterrupt_Core ( CoreNumber );
				
				// interrupt
				//Regs [ ( 0x1f801dae - SPU_X ) >> 1 ] |= 0x40;
				pCoreRegs0->STAT |= 0x40;
#else
				switch ( CoreNumber )
				{
					case 0:
						Dma::_DMA->AutoDMA_Interrupt ( 4 );
						break;
						
					case 1:
						Dma::_DMA->AutoDMA_Interrupt ( 7 );
						break;
				}
#endif
			}
#endif
			
			//return;
			return 0;
		}
		*/
		
		//if ( pCoreRegs0->ADMAS )
		//{
			//ulADMA_Active = pCoreRegs0->ADMAS;
			//pCoreRegs0->ADMAS = 0;
			//ulADMAS_Output = 0;
		//}
		
		// determine what half of buffer to transfer into
		//ulBufferHalf_Offset = ( ( DecodeBufferOffset & 512 ) >> 1 );
		
		// I want to copy data into the other half of the buffer that is not currently being decoded/played
		//ulBufferHalf_Offset ^= 256;
		
		for ( int i = 0; i < (BS << 1); i++ )
		{
			// check if transfering left or right samples
			/*
			if ( SoundDataInput_Offset < 256 )
			{
				pCoreSoundDataInputL [ SoundDataInput_Offset + ulBufferHalf_Offset ] = ((u16*) Data) [ i ];
			}
			else
			{
				pCoreSoundDataInputR [ ( SoundDataInput_Offset - 256 ) + ulBufferHalf_Offset ] = ((u16*) Data) [ i ];
			}
			*/

			// half of the play buffer is 256 samples
			// one full play buffer is 512 samples
			// so the full play buffer for both l/r is 1024 samples
			if ( (NextSoundBufferAddress & 1023) < 256 )
			{
				pCoreSoundDataInputL [ NextSoundBufferAddress & 511 ] = ((u16*) Data) [ i ];
			}
			else if ( (NextSoundBufferAddress & 1023) < 512 )
			{
				pCoreSoundDataInputR [ (NextSoundBufferAddress-256) & 511 ] = ((u16*) Data) [ i ];
			}
			else if ( (NextSoundBufferAddress & 1023) < 768 )
			{
				pCoreSoundDataInputL [ (NextSoundBufferAddress-256) & 511 ] = ((u16*) Data) [ i ];
			}
			else
			{
				pCoreSoundDataInputR [ (NextSoundBufferAddress-512) & 511 ] = ((u16*) Data) [ i ];
			}
			
			//SoundDataInput_Offset++;
			//NextSoundBufferAddress += 2;
			NextSoundBufferAddress++;
		}
		
		
		
		
		
#ifdef INLINE_DEBUG_DMA_WRITE
		debug << " (after)ADMAOffset=" << hex << NextSoundBufferAddress;
		debug << " ***\r\n";
#endif

		// check if all of the data for half of the sound data input buffer (ADMA) has been transferred
		//if ( SoundDataInput_Offset >= 512 )
		//if ( ! ( NextSoundBufferAddress & (1023) ) )
		//if ( ! ( NextSoundBufferAddress & (511) ) )
		{
#ifdef INLINE_DEBUG_DMA_WRITE
		debug << "***HALF-BUFFER-TRANSFERRED***\r\n";
#endif

			//if ( ((s64) (NextSoundBufferAddress - (ulADMA_PlayOffset<<1))) > 1024 )
			if ( ((s64) (NextSoundBufferAddress - ulADMA_PlayOffset)) > 512 )
			{
				SoundDataInput_Enable = false;
			}
			
			// buffer-half is now filled with samples that haven't been played yet
			//bBufferFull [ ulBufferHalf_Offset >> 8 ] = 1;
			
#ifdef INLINE_DEBUG_ENABLE_AUTODMA_INT
			// interrupt after half the buffer is transferred? SPU interrupt or DMA interrupt or both??
			// first need to check if interrupts are enabled
			if ( pCoreRegs0->CTRL & 0x40 )
			{
#ifdef INLINE_DEBUG_INT
	debug << "\r\nSPU#" << dec << CoreNumber << ":INT:ADMA";
#endif
#ifdef INLINE_DEBUG_ENABLE_AUTODMA_INT_SPU
				// trigger interrupt for transferring half of the buffer
				SetInterrupt ();
				
				// do this for ps2
				SetInterrupt_Core ( CoreNumber );
				
				// interrupt
				//Regs [ ( 0x1f801dae - SPU_X ) >> 1 ] |= 0x40;
				pCoreRegs0->STAT |= 0x40;
#else
				switch ( CoreNumber )
				{
					case 0:
						Dma::_DMA->AutoDMA_Interrupt ( 4 );
						break;
						
					case 1:
						Dma::_DMA->AutoDMA_Interrupt ( 7 );
						break;
				}
#endif

#ifdef CLEAR_ADMAS_ON_INT
			GET_REG16 ( ADMAS ) = 0;
#endif


			}
#endif
		}
		

		
		// done
		//return;
		return BS;
	}
#endif
	
	//for ( int i = 0; i < 32; i++ )
	for ( int i = 0; i < (BS << 1); i++ )
	{
		// write the data into sound RAM
		RAM [ ( NextSoundBufferAddress + i ) & ( c_iRam_Mask >> 1 ) ] = ((u16*) Data) [ i ];
		
#ifdef INLINE_DEBUG_DMA_WRITE_RECORD
	debug << " " << ((u16*) Data) [ i ];
#endif

	}

	
	//////////////////////////////////////////////////
	// reset buffer index
	BufferIndex = 0;

	//////////////////////////////////////////////////////////
	// update next sound buffer address
	NextSoundBufferAddress += ( BS << 1 );
	
	//NextSoundBufferAddress &= c_iRam_Mask;
	NextSoundBufferAddress &= ( c_iRam_Mask >> 1 );
	
	// make sure address is aligned to sound block
	NextSoundBufferAddress &= ~7;

	// check for interrupt
	// check if address is set to trigger interrupt
	// ***todo*** only check for interrupt at end of transfer
	if ( SPU2::_SPU2->SPU0.pCoreRegs0->CTRL & 0x40 )
	{

		if ( NextSoundBufferAddress == ( ( SWAPH ( SPU2::_SPU2->SPU0.pCoreRegs0->IRQA ) ) & ( c_iRam_Mask >> 1 ) ) )
		{
#ifdef INLINE_DEBUG_INT
	debug << "\r\nSPU:INT:DMAWRITE";
#endif

			// we have reached irq address - trigger interrupt
			SetInterrupt ();
			
			// do this for ps2
			//SetInterrupt_Core ( CoreNumber );
			SetInterrupt_Core ( 0 );
			
			// interrupt
			//pCoreRegs0->STAT |= 0x40;
			SPU2::_SPU2->SPU0.pCoreRegs0->CTRL |= 0x40;
		}

	}	// end if ( SPU2::_SPU2->SPU0.pCoreRegs0->CTRL & 0x40 )

	if ( SPU2::_SPU2->SPU1.pCoreRegs0->CTRL & 0x40 )
	{
		if ( NextSoundBufferAddress == ( ( SWAPH ( SPU2::_SPU2->SPU1.pCoreRegs0->IRQA ) ) & ( c_iRam_Mask >> 1 ) ) )
		{
#ifdef INLINE_DEBUG_INT
	debug << "\r\nSPU1:INT:DMAWRITE";
#endif

			// we have reached irq address - trigger interrupt
			SetInterrupt ();
			
			// do this for ps2
			//SetInterrupt_Core ( CoreNumber );
			SetInterrupt_Core ( 1 );
			
			// interrupt
			//pCoreRegs0->STAT |= 0x40;
			SPU2::_SPU2->SPU1.pCoreRegs0->CTRL |= 0x40;
		}

	}	// end if ( SPU2::_SPU2->SPU1.pCoreRegs0->CTRL & 0x40 )



#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "; (after) NextSoundBufferAddress=" << NextSoundBufferAddress;
#endif

	return BS;
}



void SPUCore::RunNoiseGenerator ()
{
	u32 NoiseStep, NoiseShift, ParityBit;
	
	//NoiseStep = ( ( Regs [ ( CTRL - SPU_X ) >> 1 ] >> 8 ) & 0x3 ) + 4;
	NoiseStep = ( ( pCoreRegs0->CTRL >> 8 ) & 0x3 ) + 4;
	//NoiseShift = ( ( Regs [ ( CTRL - SPU_X ) >> 1 ] >> 10 ) & 0xf );
	NoiseShift = ( ( pCoreRegs0->CTRL >> 10 ) & 0xf );
	
	Timer -= NoiseStep;
	ParityBit = ( ( NoiseLevel >> 15 ) ^ ( NoiseLevel >> 12 ) ^ ( NoiseLevel >> 11 ) ^ ( NoiseLevel >> 10 ) ^ 1 ) & 1;
	if ( Timer < 0 ) NoiseLevel = ( NoiseLevel << 1 ) + ParityBit;
	if ( Timer < 0 ) Timer += ( 0x20000 >> NoiseShift );
	if ( Timer < 0 ) Timer += ( 0x20000 >> NoiseShift );
}


// this function is not used anymore
/*
static void SPUCore::SetSweepVars ( u16 flags, u32 Channel, s32* Rates, s32* Rates75 )
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
*/



// this function is not used anymore
/*
static void SPUCore::SweepVolume ( u16 flags, s64& CurrentVolume, u32 VolConstant, u32 VolConstant75 )
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
*/


s64 SPUCore::Get_VolumeStep ( s16& Level, u32& Cycles, u32 Value, u32 flags )
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


void SPUCore::Start_VolumeEnvelope ( s16& Level, u32& Cycles, u32 Value, u32 flags, bool InitLevel )
{
	//static const s32 StepValues_Inc [] = { 7, 6, 5, 4 };
	//static const s32 StepValues_Dec [] = { -8, -7, -6, -5 };
	
	s32 ShiftValue, StepValue;
	s32 Step;
	
	//ShiftValue = ( Value >> 2 ) & 0xf;
	ShiftValue = ( Value >> 2 ) & 0x1f;
	StepValue = Value & 0x3;

#ifdef INLINE_DEBUG_ENVELOPE
	debug << "\nStart_VolumeEnvelope";
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
	
	// check if exponential AND increase
	if ( ( ( flags & 0x3 ) == 2 ) && ( Level > 0x6000 ) )
	{
		Cycles <<= 2;
	}

#ifdef INLINE_DEBUG_ENVELOPE
	debug << "; Cycles=" << dec << Cycles;
#endif

	if ( ((s32)Cycles) < 0 )
	{
		cout << "\nhps2x64: ERROR: SPU2: CYCLES IS LESS THAN ZERO!!! (START)***\n";
#ifdef INLINE_DEBUG_ENVELOPE
		debug << "\r\nhps2x64: ERROR: SPU2: CYCLES IS LESS THAN ZERO!!! (START)***\r\n";
#endif
		return;
	}
	
	// check if exponential AND decrease
	if ( ( flags & 0x3 ) == 3 )
	{
		Step = ( Step * ( (s32) Level ) ) >> 15;
	}
	
	if ( InitLevel )
	{
		//Level = Step;
		Level += Step;
	}
}

// probably should start cycles at 1 and then let it set it later
// does not saturate Level since it can go to -1 for a cycle
void SPUCore::VolumeEnvelope ( s16& Level, u32& Cycles, u32 Value, u32 flags, bool clamp )
{
	//static const s32 StepValues_Inc [] = { 7, 6, 5, 4 };
	//static const s32 StepValues_Dec [] = { -8, -7, -6, -5 };
	
	s32 ShiftValue, StepValue;
	s32 Step;
	s32 sLevel32;
	
	if ( ((s32)Cycles) < 0 )
	{
		cout << "\nhps2x64: ERROR: SPU2: CYCLES IS LESS THAN ZERO!!!***\n";
#ifdef INLINE_DEBUG_ENVELOPE
		debug << "\r\nhps2x64: ERROR: SPU2: CYCLES IS LESS THAN ZERO!!!***\r\n";
#endif
		return;
	}
	
	Cycles--;
	
	if ( Cycles ) return;
	
	ShiftValue = ( Value >> 2 ) & 0x1f;
	StepValue = Value & 0x3;
	

#ifdef INLINE_DEBUG_ENVELOPE
	debug << "\nVolumeEnvelope";
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

	// check if exponential AND increase
	if ( ( ( flags & 0x3 ) == 2 ) && ( Level > 0x6000 ) )
	{
		Cycles <<= 2;
	}
	
	sLevel32 = Level;
	
	// check if exponential AND decrease
	if ( ( flags & 0x3 ) == 3 )
	{
		Step = ( Step * sLevel32 ) >> 15;
	}
	
#ifdef INLINE_DEBUG_ENVELOPE
	debug << "; Step=" << hex << Step << "; (before) Level=" << Level;
#endif

	sLevel32 += Step;
	
	if ( clamp )
	{
	// clamp level to signed 16-bits
	//if ( sLevel32 > 0x7fffLL ) { sLevel32 = 0x7fffLL; } else if ( sLevel32 < -0x8000LL ) { sLevel32 = -0x8000LL; }
	if ( sLevel32 > 0x7fff ) { sLevel32 = 0x7fff; } else if ( sLevel32 < 0 ) { sLevel32 = 0; }
	}
	
	// set the new level value
	Level = sLevel32;

#ifdef INLINE_DEBUG_ENVELOPE
	debug << "; (after) Level=" << Level;
	debug << " Cycles=" << Cycles;
#endif
}



void SPUCore::ProcessReverbR ( s64 RightInput )
{
	// disable reverb to get PS2 SPU started
	
	s64 Rin, Rout;
	s64 t_mLDIFF, t_mRDIFF;
	s64 t_mRSAME, t_mRAPF1, t_mRAPF2;
	
	//s64 s_dRSAME = ReadReverbBuffer ( ((u32) *_dRSAME) << 2 );
	//s64 s_mRSAME = ReadReverbBuffer ( ( ((u32) *_mRSAME) << 2 ) - 1 );
	s64 s_dRSAME = ReadReverbBuffer ( SWAPH ( pCoreRegs0->dRSAME ) );
	s64 s_mRSAME = ReadReverbBuffer ( ( SWAPH ( pCoreRegs0->mRSAME ) ) - 1 );

	//s64 s_dLDIFF = ReadReverbBuffer ( ((u32) *_dLDIFF) << 2 );
	s64 s_dLDIFF = ReadReverbBuffer ( SWAPH ( pCoreRegs0->dLDIFF ) );
	
	//s64 s_dRDIFF = ReadReverbBuffer ( ((u32) *_dRDIFF) << 2 );
	//s64 s_mRDIFF = ReadReverbBuffer ( ( ((u32) *_mRDIFF) << 2 ) - 1 );
	s64 s_dRDIFF = ReadReverbBuffer ( SWAPH ( pCoreRegs0->dRDIFF ) );
	s64 s_mRDIFF = ReadReverbBuffer ( ( SWAPH ( pCoreRegs0->mRDIFF ) ) - 1 );

	//s64 s_mRCOMB1 = ReadReverbBuffer ( ((u32) *_mRCOMB1) << 2 );
	//s64 s_mRCOMB2 = ReadReverbBuffer ( ((u32) *_mRCOMB2) << 2 );
	//s64 s_mRCOMB3 = ReadReverbBuffer ( ((u32) *_mRCOMB3) << 2 );
	//s64 s_mRCOMB4 = ReadReverbBuffer ( ((u32) *_mRCOMB4) << 2 );
	s64 s_mRCOMB1 = ReadReverbBuffer ( SWAPH ( pCoreRegs0->mRCOMB1 ) );
	s64 s_mRCOMB2 = ReadReverbBuffer ( SWAPH ( pCoreRegs0->mRCOMB2 ) );
	s64 s_mRCOMB3 = ReadReverbBuffer ( SWAPH ( pCoreRegs0->mRCOMB3 ) );
	s64 s_mRCOMB4 = ReadReverbBuffer ( SWAPH ( pCoreRegs0->mRCOMB4 ) );
	
	//s64 s_mRAPF1 = ReadReverbBuffer ( ((u32) *_mRAPF1) << 2 );
	//s64 s_mRAPF1_dAPF1 = ReadReverbBuffer ( ( ((u32) *_mRAPF1) - ((u32) *_dAPF1) ) << 2 );
	s64 s_mRAPF1 = ReadReverbBuffer ( SWAPH ( pCoreRegs0->mRAPF1 ) );
	s64 s_mRAPF1_dAPF1 = ReadReverbBuffer ( ( SWAPH ( pCoreRegs0->mRAPF1 ) - SWAPH ( pCoreRegs0->dAPF1 ) ) );
	
	//s64 s_mRAPF2 = ReadReverbBuffer ( ((u32) *_mRAPF2) << 2 );
	//s64 s_mRAPF2_dAPF2 = ReadReverbBuffer ( ( ((u32) *_mRAPF2) - ((u32) *_dAPF2) ) << 2 );
	s64 s_mRAPF2 = ReadReverbBuffer ( SWAPH ( pCoreRegs0->mRAPF2 ) );
	s64 s_mRAPF2_dAPF2 = ReadReverbBuffer ( ( SWAPH ( pCoreRegs0->mRAPF2 ) - SWAPH ( pCoreRegs0->dAPF2 ) ) );


	// input from mixer //
	//Rin = ( RightInput * ( (s64) *_vRIN ) ) >> 15;
	Rin = adpcm_decoder::clamp( ( RightInput * ( (s64) pCoreRegs1->vRIN ) ) >> 15 );
	
	// same side reflection //
	//[mRSAME] = (Rin + [dRSAME]*vWALL - [mRSAME-2])*vIIR + [mRSAME-2]  ;R-to-R
	//t_mRSAME = ( ( ( Rin + ( ( s_dRSAME * ( (s64) *_vWALL ) ) >> 15 ) - s_mRSAME ) * ( (s64) *_vIIR ) ) >> 15 ) + s_mRSAME;
	t_mRSAME = ( ( ( ( Rin + ( ( s_dRSAME * ( (s64) pCoreRegs1->vWALL ) ) >> 15 ) - s_mRSAME ) * ( (s64) pCoreRegs1->vIIR ) ) >> 15 ) + s_mRSAME );
	
	// Different Side Reflection //
	//[mRDIFF] = (Rin + [dLDIFF]*vWALL - [mRDIFF-2])*vIIR + [mRDIFF-2]  ;L-to-R
	//t_mRDIFF = ( ( ( Rin + ( ( s_dLDIFF * ( (s64) *_vWALL ) ) >> 15 ) - s_mRDIFF ) * ( (s64) *_vIIR ) ) >> 15 ) + s_mRDIFF;
	t_mRDIFF = ( ( ( ( Rin + ( ( s_dLDIFF * ( (s64) pCoreRegs1->vWALL ) ) >> 15 ) - s_mRDIFF ) * ( (s64) pCoreRegs1->vIIR ) ) >> 15 ) + s_mRDIFF );
	
	// Early Echo (Comb Filter, with input from buffer) //
	//Rout=vCOMB1*[mRCOMB1]+vCOMB2*[mRCOMB2]+vCOMB3*[mRCOMB3]+vCOMB4*[mRCOMB4]
	//Rout = ( ( ( (s64) *_vCOMB1 ) * s_mRCOMB1 ) + ( ( (s64) *_vCOMB2 ) * s_mRCOMB2 ) + ( ( (s64) *_vCOMB3 ) * s_mRCOMB3 ) + ( ( (s64) *_vCOMB4 ) * s_mRCOMB4 ) ) >> 15;
	Rout = (( ( ( (s64) pCoreRegs1->vCOMB1 ) * s_mRCOMB1 ) + ( ( (s64) pCoreRegs1->vCOMB2 ) * s_mRCOMB2 ) + ( ( (s64) pCoreRegs1->vCOMB3 ) * s_mRCOMB3 ) + ( ( (s64) pCoreRegs1->vCOMB4 ) * s_mRCOMB4 ) ) >> 15);


	// Late Reverb APF1 (All Pass Filter 1, with input from COMB) //
	//[mRAPF1]=Rout-vAPF1*[mRAPF1-dAPF1], Rout=[mRAPF1-dAPF1]+[mRAPF1]*vAPF1
	//t_mRAPF1 = Rout - ( ( ( (s64) *_vAPF1 ) * s_mRAPF1_dAPF1 ) >> 15 );
	//Rout = s_mRAPF1_dAPF1 + ( ( s_mRAPF1 * ( (s64) *_vAPF1 ) ) >> 15 );
	t_mRAPF1 = ( Rout - ( ( ( (s64) pCoreRegs1->vAPF1 ) * s_mRAPF1_dAPF1 ) >> 15 ) );
	Rout = ( s_mRAPF1_dAPF1 + ( (t_mRAPF1 * ( (s64) pCoreRegs1->vAPF1 ) ) >> 15 ) );

	// clamp vars
	//Rout = adpcm_decoder::uclamp ( Rout );

	// Late Reverb APF2 (All Pass Filter 2, with input from APF1) //
	// [mRAPF2]=Rout-vAPF2*[mRAPF2-dAPF2], Rout=[mRAPF2-dAPF2]+[mRAPF2]*vAPF2
	//t_mRAPF2 = Rout - ( ( ( (s64) *_vAPF2 ) * s_mRAPF2_dAPF2 ) >> 15 );
	//Rout = s_mRAPF2_dAPF2 + ( ( s_mRAPF2 * ( (s64) *_vAPF2 ) ) >> 15 );
	t_mRAPF2 = ( Rout - ( ( ( (s64) pCoreRegs1->vAPF2 ) * s_mRAPF2_dAPF2 ) >> 15 ) );
	Rout = ( s_mRAPF2_dAPF2 + ( (t_mRAPF2 * ( (s64) pCoreRegs1->vAPF2 ) ) >> 15 ) );

	// clamp vars
	//Rout = adpcm_decoder::uclamp ( Rout );

	// Output to Mixer (Output volume multiplied with input from APF2) //
	// RightOutput = Rout*vROUT
	//ReverbR_Output = ( Rout * ( (s64) *_vROUT ) ) >> 15;
	ReverbR_Output = ( ( Rout * ( (s64) pCoreRegs1->vROUT ) ) >> 15 );
	
	// only write to the reverb buffer if reverb is enabled
	//if ( REG ( CTRL ) & 0x80 )
	if ( pCoreRegs0->CTRL & 0x80 )
	{
		// *** TODO *** for PS2, there is no shift of the address as it is the actual address stored
		//WriteReverbBuffer ( ((u32) *_mRSAME) << 2, t_mRSAME );
		//WriteReverbBuffer ( ((u32) *_mRDIFF) << 2, t_mRDIFF );
		//WriteReverbBuffer ( ((u32) *_mRAPF1) << 2, t_mRAPF1 );
		//WriteReverbBuffer ( ((u32) *_mRAPF2) << 2, t_mRAPF2 );
		WriteReverbBuffer ( SWAPH ( pCoreRegs0->mRSAME ), t_mRSAME );
		WriteReverbBuffer ( SWAPH ( pCoreRegs0->mRDIFF ), t_mRDIFF );
		WriteReverbBuffer ( SWAPH ( pCoreRegs0->mRAPF1 ), t_mRAPF1 );
		WriteReverbBuffer ( SWAPH ( pCoreRegs0->mRAPF2 ), t_mRAPF2 );
	}
	
	// update reverb buffer address
	// this should actually happen at 22050 hz unconditionally
	UpdateReverbBuffer ();
}


void SPUCore::ProcessReverbL ( s64 LeftInput )
{
	// vars needed for multi-threading
	// 
	
	
	s64 Lin, Lout;
	
	// outputs
	s64 t_mLDIFF, t_mRDIFF;
	s64 t_mLSAME, t_mLAPF1, t_mLAPF2;
	
	// inputs
	//s64 s_dLSAME = ReadReverbBuffer ( ((u32) *_dLSAME) << 2 );
	//s64 s_mLSAME = ReadReverbBuffer ( ( ((u32) *_mLSAME) << 2 ) - 1 );
	s64 s_dLSAME = ReadReverbBuffer ( SWAPH ( pCoreRegs0->dLSAME ) );
	s64 s_mLSAME = ReadReverbBuffer ( ( SWAPH ( pCoreRegs0->mLSAME ) ) - 1 );
	
	//s64 s_dLDIFF = ReadReverbBuffer ( ((u32) *_dLDIFF) << 2 );
	//s64 s_mLDIFF = ReadReverbBuffer ( ( ((u32) *_mLDIFF) << 2 ) - 1 );
	s64 s_dLDIFF = ReadReverbBuffer ( SWAPH ( pCoreRegs0->dLDIFF ) );
	s64 s_mLDIFF = ReadReverbBuffer ( ( SWAPH ( pCoreRegs0->mLDIFF ) ) - 1 );
	
	//s64 s_dRDIFF = ReadReverbBuffer ( ((u32) *_dRDIFF) << 2 );
	s64 s_dRDIFF = ReadReverbBuffer ( SWAPH ( pCoreRegs0->dRDIFF ) );
	
	//s64 s_mLCOMB1 = ReadReverbBuffer ( ((u32) *_mLCOMB1) << 2 );
	//s64 s_mLCOMB2 = ReadReverbBuffer ( ((u32) *_mLCOMB2) << 2 );
	//s64 s_mLCOMB3 = ReadReverbBuffer ( ((u32) *_mLCOMB3) << 2 );
	//s64 s_mLCOMB4 = ReadReverbBuffer ( ((u32) *_mLCOMB4) << 2 );
	s64 s_mLCOMB1 = ReadReverbBuffer ( SWAPH ( pCoreRegs0->mLCOMB1 ) );
	s64 s_mLCOMB2 = ReadReverbBuffer ( SWAPH ( pCoreRegs0->mLCOMB2 ) );
	s64 s_mLCOMB3 = ReadReverbBuffer ( SWAPH ( pCoreRegs0->mLCOMB3 ) );
	s64 s_mLCOMB4 = ReadReverbBuffer ( SWAPH ( pCoreRegs0->mLCOMB4 ) );
	
	//s64 s_mLAPF1 = ReadReverbBuffer ( ((u32) *_mLAPF1) << 2 );
	//s64 s_mLAPF1_dAPF1 = ReadReverbBuffer ( ( ((u32) *_mLAPF1) - ((u32) *_dAPF1) ) << 2 );
	s64 s_mLAPF1 = ReadReverbBuffer ( SWAPH ( pCoreRegs0->mLAPF1 ) );
	s64 s_mLAPF1_dAPF1 = ReadReverbBuffer ( ( SWAPH ( pCoreRegs0->mLAPF1 ) - SWAPH ( pCoreRegs0->dAPF1 ) ) );
	
	//s64 s_mLAPF2 = ReadReverbBuffer ( ((u32) *_mLAPF2) << 2 );
	//s64 s_mLAPF2_dAPF2 = ReadReverbBuffer ( ( ((u32) *_mLAPF2) - ((u32) *_dAPF2) ) << 2 );
	s64 s_mLAPF2 = ReadReverbBuffer ( SWAPH ( pCoreRegs0->mLAPF2 ) );
	s64 s_mLAPF2_dAPF2 = ReadReverbBuffer ( ( SWAPH ( pCoreRegs0->mLAPF2 ) - SWAPH ( pCoreRegs0->dAPF2 ) ) );

	
	// input from mixer //
	//Lin = ( LeftInput * ( (s64) *_vLIN ) ) >> 15;
	Lin = ( ( LeftInput * ( (s64) pCoreRegs1->vLIN ) ) >> 15 );
	
	// same side reflection //
	//[mLSAME] = (Lin + [dLSAME]*vWALL - [mLSAME-2])*vIIR + [mLSAME-2]  ;L-to-L
	//t_mLSAME = ( ( ( Lin + ( ( s_dLSAME * ( (s64) *_vWALL ) ) >> 15 ) - s_mLSAME ) * ( (s64) *_vIIR ) ) >> 15 ) + s_mLSAME;
	t_mLSAME = ((((Lin + ((s_dLSAME * ((s64)pCoreRegs1->vWALL)) >> 15) - s_mLSAME) * ((s64)pCoreRegs1->vIIR)) >> 15) + s_mLSAME);

	// Different Side Reflection //
	//[mLDIFF] = (Lin + [dRDIFF]*vWALL - [mLDIFF-2])*vIIR + [mLDIFF-2]  ;R-to-L
	//t_mLDIFF = ( ( ( Lin + ( ( s_dRDIFF * ( (s64) *_vWALL ) ) >> 15 ) - s_mLDIFF ) * ( (s64) *_vIIR ) ) >> 15 ) + s_mLDIFF;
	t_mLDIFF = ((((Lin + ((s_dRDIFF * ((s64)pCoreRegs1->vWALL)) >> 15) - s_mLDIFF) * ((s64)pCoreRegs1->vIIR)) >> 15) + s_mLDIFF);

	
	// Early Echo (Comb Filter, with input from buffer) //
	//Lout=vCOMB1*[mLCOMB1]+vCOMB2*[mLCOMB2]+vCOMB3*[mLCOMB3]+vCOMB4*[mLCOMB4]
	//Lout = ( ( (s64) *_vCOMB1 ) * s_mLCOMB1 + ( (s64) *_vCOMB2 ) * s_mLCOMB2 + ( (s64) *_vCOMB3 ) * s_mLCOMB3 + ( (s64) *_vCOMB4 ) * s_mLCOMB4 ) >> 15;
	Lout = (( ( (s64) pCoreRegs1->vCOMB1 ) * s_mLCOMB1 + ( (s64) pCoreRegs1->vCOMB2 ) * s_mLCOMB2 + ( (s64) pCoreRegs1->vCOMB3 ) * s_mLCOMB3 + ( (s64) pCoreRegs1->vCOMB4 ) * s_mLCOMB4 ) >> 15);


	// Late Reverb APF1 (All Pass Filter 1, with input from COMB) //
	//[mLAPF1]=Lout-vAPF1*[mLAPF1-dAPF1], Lout=[mLAPF1-dAPF1]+[mLAPF1]*vAPF1
	//t_mLAPF1 = Lout - ( ( ( (s64) *_vAPF1 ) * s_mLAPF1_dAPF1 ) >> 15 );
	//Lout = s_mLAPF1_dAPF1 + ( ( s_mLAPF1 * ( (s64) *_vAPF1 ) ) >> 15 );
	t_mLAPF1 = (Lout - ((((s64)pCoreRegs1->vAPF1) * s_mLAPF1_dAPF1) >> 15));
	Lout = (s_mLAPF1_dAPF1 + ((t_mLAPF1 * ((s64)pCoreRegs1->vAPF1)) >> 15));

//debug << "\r\nWriteReverb0: " << dec << " Lout=" << Lout << " s_mLAPF2=" << s_mLAPF2 << " s_mLAPF2_dAPF2=" << s_mLAPF2_dAPF2 << " vAPF2=" << pCoreRegs1->vAPF2;


	// Late Reverb APF2 (All Pass Filter 2, with input from APF1) //
	// [mLAPF2]=Lout-vAPF2*[mLAPF2-dAPF2], Lout=[mLAPF2-dAPF2]+[mLAPF2]*vAPF2
	//t_mLAPF2 = Lout - ( ( ( (s64) *_vAPF2 ) * s_mLAPF2_dAPF2 ) >> 15 );
	//Lout = s_mLAPF2_dAPF2 + ( ( s_mLAPF2 * ( (s64) *_vAPF2 ) ) >> 15 );
	t_mLAPF2 = (Lout - ((((s64)pCoreRegs1->vAPF2) * s_mLAPF2_dAPF2) >> 15));
	Lout = ( s_mLAPF2_dAPF2 + ( (t_mLAPF2 * ( (s64) pCoreRegs1->vAPF2 ) ) >> 15 ) );


	// Output to Mixer (Output volume multiplied with input from APF2) //
	// LeftOutput = Lout*vLOUT
	//ReverbL_Output = ( Lout * ( (s64) *_vLOUT ) ) >> 15;
	ReverbL_Output = ( ( Lout * ( (s64) pCoreRegs1->vLOUT ) ) >> 15 );
	
	// only write to the reverb buffer if reverb is enabled
	//if ( REG ( CTRL ) & 0x80 )
	if ( pCoreRegs0->CTRL & 0x80 )
	{
		// write back to reverb buffer
		// *** TODO *** for PS2, there is no shift of the address as it is the actual address stored
		//WriteReverbBuffer ( ((u32) *_mLSAME) << 2, t_mLSAME );
		//WriteReverbBuffer ( ((u32) *_mLDIFF) << 2, t_mLDIFF );
		//WriteReverbBuffer ( ((u32) *_mLAPF1) << 2, t_mLAPF1 );
		//WriteReverbBuffer ( ((u32) *_mLAPF2) << 2, t_mLAPF2 );
		WriteReverbBuffer ( SWAPH ( pCoreRegs0->mLSAME ), t_mLSAME );
		WriteReverbBuffer ( SWAPH ( pCoreRegs0->mLDIFF ), t_mLDIFF );
		WriteReverbBuffer ( SWAPH ( pCoreRegs0->mLAPF1 ), t_mLAPF1 );
		WriteReverbBuffer ( SWAPH ( pCoreRegs0->mLAPF2 ), t_mLAPF2 );

//debug << "\r\nWriteReverb1: " << dec << " t_mLSAME=" << t_mLSAME << " t_mLDIFF=" << t_mLDIFF << " t_mLAPF1=" << t_mLAPF1 << " t_mLAPF2=" << t_mLAPF2;
	}
}



// gets address in reverb work area at offset address
s64 SPUCore::ReadReverbBuffer ( u32 Address )
{
#ifdef INLINE_DEBUG_READREVERB
	debug << "\r\nSPU::ReadReverbBuffer " << "Address=" << hex << Address;
#endif

	s16 Value;
	u32 BeforeAddress;

	// address will be coming straight from register
	// that won't work because of the offsets
	//Address <<= 1;
	
	// if there is no reverb buffer, return zero
	if ( !ReverbWork_Size )
	{
		Address = 0;
	}

	Address += Reverb_BufferAddress;
	
#ifdef INLINE_DEBUG_READREVERB
	debug << " " << hex << Address;
	if ( Address < ReverbWork_Start )
	{
		cout << "\nSPU::ReadReverbBuffer; (before) Address<ReverbWork_Start; Address=" << hex << Address << " ReverbWork_Start=" << ReverbWork_Start << dec << " ReverbWork_Size=" << ReverbWork_Size;
	}
	BeforeAddress = Address;
#endif


	//if ( Address >= ReverbWork_End ) Address = ReverbWork_Start + ( Address - ReverbWork_End );
	if ( Address >= ReverbWork_Size )
	{
		if ( ReverbWork_Size )
		{
			Address -= ReverbWork_Size;
			//Address %= ReverbWork_Size;
		}
		else
		{
			Address = 0;
		}
	}

	Address += ReverbWork_Start;

	// just in case
	Address &= ( c_iRam_Mask >> 1 );
	
	
#ifdef INLINE_DEBUG_READREVERB
	debug << " " << hex << Address;
	if ( Address < ReverbWork_Start )
	{
		cout << "\nSPU::ReadReverbBuffer; (after) Address<ReverbWork_Start; (before) Address=" << hex << BeforeAddress << " (after) Address=" << Address << " ReverbWork_Start=" << ReverbWork_Start << dec << " ReverbWork_Size=" << ReverbWork_Size;
	}
#endif

	//Value = RAM [ Address >> 1 ];
	Value = RAM [ Address ];
	
#ifdef INLINE_DEBUG_READREVERB
	debug << " Value=" << hex << Value;
#endif

	// address is ready for use with shift right by 1
	return Value;
}

void SPUCore::WriteReverbBuffer ( u32 Address, s64 Value )
{
#ifdef INLINE_DEBUG_WRITEREVERB
	debug << "\r\nSPU::WriteReverbBuffer; Reverb_BufferAddress=" << hex << Reverb_BufferAddress << " Value=" << Value << " Address=" << Address;
#endif

	u32 BeforeAddress;

	// address will be coming straight from register
	// that won't work because of the offsets
	//Address <<= 1;
	
	// for debugging
	BeforeAddress = Address;
	
	// if there is no reverb buffer, return zero
	if ( !ReverbWork_Size )
	{
		Address = 0;
	}
	
	
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

	
	//if ( Address >= ReverbWork_End ) Address = ReverbWork_Start + ( Address - ReverbWork_End );
	if ( Address >= ReverbWork_Size )
	{
		if ( ReverbWork_Size )
		{
			Address -= ReverbWork_Size;
			//Address %= ReverbWork_Size;
		}
		else
		{
			Address = 0;
		}
	}
	
	Address += ReverbWork_Start;
	
	// just in case
	Address &= ( c_iRam_Mask >> 1 );
	
#ifdef INLINE_DEBUG_WRITEREVERB
	debug << " " << Address;
#endif

#ifdef VERBOSE_DEBUG_WRITEREVERB
	if ( Address < ReverbWork_Start )
	{
		cout << "\nSPU::WriteReverbBuffer; (after) Address<ReverbWork_Start; (before) Address=" << hex << BeforeAddress << " (after) Address=" << Address << " ReverbWork_Start=" << ReverbWork_Start << " ReverbWork_End=" << ReverbWork_End << " ReverbWork_Size=" << dec << ReverbWork_Size;
	}
	if ( Address >= ReverbWork_End )
	{
		cout << "\nSPU::WriteReverbBuffer; (after) Address>=ReverbWork_End; (before) Address=" << hex << BeforeAddress << " (after) Address=" << Address << " ReverbWork_Start=" << ReverbWork_Start << " ReverbWork_End=" << ReverbWork_End << dec << " ReverbWork_Size=" << ReverbWork_Size;
	}
#endif

	// address is ready for use with shift right by 1
	//RAM [ Address >> 1 ] = adpcm_decoder::clamp ( Value );
	RAM [ Address ] = adpcm_decoder::clamp64 ( Value );
}

void SPUCore::UpdateReverbBuffer ()
{
#ifdef INLINE_DEBUG_UPDATEREVERB
	debug << "\r\nSPU::UpdateReverbBuffer " << " ReverbWork_Start=" << hex << ReverbWork_Start << " ReverbWork_End=" << ReverbWork_End << " Reverb_BufferAddress=" << Reverb_BufferAddress;
#endif

	//Reverb_BufferAddress += 2;
	Reverb_BufferAddress += 1;
	
	//if ( Reverb_BufferAddress >= ReverbWork_End ) Reverb_BufferAddress = ReverbWork_Start;
	if ( Reverb_BufferAddress >= ReverbWork_Size ) Reverb_BufferAddress = 0;


#ifdef ENABLE_REVERB_INT_ONLY_WHEN_REVERB_ENABLED
	// only if reverb is enabled ??
	if ( pCoreRegs0->CTRL & 0x80 )
#endif
	{

	if ( SPU2::_SPU2->SPU0.pCoreRegs0->CTRL & 0x40 )
	{
		// interrupts are enabled for the current spu2 core //

		// check reverb address against irq address for core#0
		if ( ( Reverb_BufferAddress + ReverbWork_Start ) == ( SWAPH ( SPU2::_SPU2->SPU0.pCoreRegs0->IRQA ) ) )
		{
#ifdef INLINE_DEBUG_INT
		debug << "\r\nSPU:INT:REVERB";
#endif

			// we have reached irq address - trigger interrupt
			SetInterrupt ();
			
			// do this for ps2
			//SetInterrupt_Core ( CoreNumber );
			SetInterrupt_Core ( 0 );
			
			// interrupt
			//pCoreRegs0->STAT |= 0x40;
			SPU2::_SPU2->SPU0.pCoreRegs0->STAT |= 0x40;
		}

	}	// end if ( SPU2::_SPU2->SPU0.pCoreRegs0->CTRL & 0x40 )

	if ( SPU2::_SPU2->SPU1.pCoreRegs0->CTRL & 0x40 )
	{
		// check reverb address against irq address for core#1
		if ( ( Reverb_BufferAddress + ReverbWork_Start ) == ( SWAPH ( SPU2::_SPU2->SPU1.pCoreRegs0->IRQA ) ) )
		{
#ifdef INLINE_DEBUG_INT
		debug << "\r\nSPU:INT:REVERB";
#endif
			// we have reached irq address - trigger interrupt
			SetInterrupt ();
			
			// do this for ps2
			//SetInterrupt_Core ( CoreNumber );
			SetInterrupt_Core ( 1 );
			
			// interrupt
			//pCoreRegs0->STAT |= 0x40;
			SPU2::_SPU2->SPU1.pCoreRegs0->STAT |= 0x40;
		}

	}	// end if ( SPU2::_SPU2->SPU1.pCoreRegs0->CTRL & 0x40 )
	
	}	// end if ( pCoreRegs0->CTRL & 0x80 )


#ifdef INLINE_DEBUG_UPDATEREVERB
	debug << " " << Reverb_BufferAddress;
#endif
}


void SPUCore::Refresh ()
{
	pCoreRegs0 = (CoreRegs0_Layout*) ( & ( Regs16.u [ ( CoreNumber << 10 ) >> 1 ] ) );
	pCoreRegs1 = (CoreRegs1_Layout*) ( & ( Regs16.u [ ( 0x760 >> 1 ) + ( CoreNumber * 20 ) ] ) );
	
	//pCoreSoundDataInputL = & ( RAM [ c_lSoundDataInputArea_Start + ( CoreNumber * c_lSoundDataInputArea_Size * 2 ) ] );
	//pCoreSoundDataInputR = & ( RAM [ c_lSoundDataInputArea_Start + ( CoreNumber * c_lSoundDataInputArea_Size * 2 ) + c_lSoundDataInputArea_Size ] );
	
	if ( !CoreNumber )
	{
		// core 0 //
		
		pCore0Left_Out = (s16*) & RAM [ 0x800 ];
		pCore0Right_Out = (s16*) & RAM [ 0xa00 ];
		
		pVoice1_Out = (s16*) & RAM [ 0x400 ];
		pVoice3_Out = (s16*) & RAM [ 0x600 ];
		pDryLeft_Out = (s16*) & RAM [ 0x1000 ];
		pDryRight_Out = (s16*) & RAM [ 0x1200 ];
		pWetLeft_Out = (s16*) & RAM [ 0x1400 ];
		pWetRight_Out = (s16*) & RAM [ 0x1600 ];
		
		pCoreSoundDataInputL = (s16*) & RAM [ 0x2000 ];
		pCoreSoundDataInputR = (s16*) & RAM [ 0x2200 ];
	}
	else
	{
		// core 1 //
		
		pCore0Left_Out = (s16*) & RAM [ 0x800 ];
		pCore0Right_Out = (s16*) & RAM [ 0xa00 ];

		pVoice1_Out = (s16*) & RAM [ 0xc00 ];
		pVoice3_Out = (s16*) & RAM [ 0xe00 ];
		pDryLeft_Out = (s16*) & RAM [ 0x1800 ];
		pDryRight_Out = (s16*) & RAM [ 0x1a00 ];
		pWetLeft_Out = (s16*) & RAM [ 0x1c00 ];
		pWetRight_Out = (s16*) & RAM [ 0x1e00 ];
		
		pCoreSoundDataInputL = (s16*) & RAM [ 0x2400 ];
		pCoreSoundDataInputR = (s16*) & RAM [ 0x2600 ];
	}
}


void SPUCore::SpuTransfer_Complete ()
{
	//Regs [ ( STAT - SPU_X ) >> 1 ] &= ~( 0xf << 7 );
	//GET_REG16 ( CTRL ) &= ~0x30;
	
	// update STAT //
	//GET_REG16 ( STAT ) &= ~( 0x3f | 0x0380 );
	//GET_REG16 ( STAT ) |= ( GET_REG16 ( CTRL ) & 0x3f );
}


// Sample0 is the sample you are on, then Sample1 is previous sample, then Sample2 is the next previous sample, etc.
s32 SPUCore::Calc_sample_gx ( u32 SampleOffset_Fixed16, s32 Sample0, s32 Sample1, s32 Sample2, s32 Sample3 )
{
	u32 i;
	s32 Output;
	
	i = ( SampleOffset_Fixed16 >> 8 ) & 0xff;
	
	Output = ( ( Sample0 * ((s32) gx [ i ]) ) + ( Sample1 * ((s32) gx [0x100 + i]) ) + ( Sample2 * ((s32) gx [0x1ff - i]) ) + ( Sample3 * ((s32) gx [0xff - i]) ) ) >> 15;
	
	adpcm_decoder::clamp ( Output );
	
	return Output;
}


s32 SPUCore::Calc_sample_filter ( s32 x0, s32 x1, s32 x2, s32 y1, s32 y2 )
{
	s32 Output;
	
	Output = ( ( _b0 * x0 ) + ( _b1 * x1 ) + ( _b2 * x2 ) - ( _a1 * y1 ) - ( _a2 * y2 ) ) >> _N;
	
	return Output;
}



void SPUCore::Start_SampleDecoding ( u32 Channel )
{
	u32 ModeRate;
	
	ChRegs0_Layout *pChRegs0;
	ChRegs1_Layout *pChRegs1;
	
	pChRegs0 = (ChRegs0_Layout*) ( & ( pCoreRegs0->ChRegs0 [ Channel ] ) );
	pChRegs1 = (ChRegs1_Layout*) ( & ( pCoreRegs0->ChRegs1 [ Channel ] ) );
	
	// clear sample history for channel
	//History [ Channel ].Value0 = 0;
	//History [ Channel ].Value1 = 0;

	
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
	//SET_REG16 ( ENVX_CH( Channel ), 0 );
	pChRegs0->ENV_X = 0;
	VOL_ADSR_Value [ Channel ] = 0;


	// loop address not manually specified
	// can still change loop address after sound starts (after 4T)
	LSA_Manual_Bitmap &= ~( 1 << Channel );
	bLoopManuallySet &= ~( 1 << Channel );
	
	// start envelope
	//ModeRate = Regs [ ( ADSR_0 >> 1 ) + ( Channel << 3 ) ];
	//ModeRate = GET_REG16 ( ADSR0_CH ( Channel ) );
	ModeRate = pChRegs0->ADSR_0;
	
	//Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 8 ) & 0x7f, ( ModeRate >> 15 ) << 1 );
	//Start_VolumeEnvelope ( VOL_ADSR_Value [ Channel ], Cycles [ Channel ], ( ModeRate >> 8 ) & 0x7f, ( ModeRate >> 15 ) << 1, true );
	Start_VolumeEnvelope ( (s16&) pChRegs0->ENV_X, Cycles [ Channel ], ( ModeRate >> 8 ) & 0x7f, ( ModeRate >> 15 ) << 1, true );


	// set sustain level (48.16 fixed point)
	// this is now 16.0 fixed point
	VOL_SUSTAIN_Level [ Channel ] = ( ( ModeRate & 0xf ) + 1 ) << ( 11 );
	
	
	//Regs [ ( ( Channel << 4 ) + ENV_X ) >> 1 ] = VOL_ADSR_Value [ Channel ];
	//SET_REG16 ( ENVX_CH( Channel ), VOL_ADSR_Value [ Channel ] );
	
	// start sweep for other volumes too
	/*
	Start_VolumeEnvelope ( VOL_L_Value [ Channel ], VOL_L_Cycles [ Channel ], Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ] & 0x7f, ( Regs [ ( VOL_L >> 1 ) + ( Channel << 3 ) ] >> 13 ) & 0x3 );
	Start_VolumeEnvelope ( VOL_R_Value [ Channel ], VOL_R_Cycles [ Channel ], Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ] & 0x7f, ( Regs [ ( VOL_R >> 1 ) + ( Channel << 3 ) ] >> 13 ) & 0x3 );
	Start_VolumeEnvelope ( MVOL_L_Value, MVOL_L_Cycles, Regs [ ( MVOL_L - SPU_X ) >> 1 ] & 0x7f, ( Regs [ ( MVOL_L - SPU_X ) >> 1 ] >> 13 ) & 0x3 );
	Start_VolumeEnvelope ( MVOL_R_Value, MVOL_R_Cycles, Regs [ ( MVOL_R - SPU_X ) >> 1 ] & 0x7f, ( Regs [ ( MVOL_R - SPU_X ) >> 1 ] >> 13 ) & 0x3 );
	*/
	
	// copy the pitch over
	//_SPU->dSampleDT [ Channel ] = ( ((u64) Regs [ ( ( Channel << 4 ) + PITCH ) >> 1 ]) << 32 ) >> 12;
	dSampleDT [ Channel ] = ( ( (u64) pChRegs0->PITCH ) << 32 ) >> 12;


	// prepare for playback //
	
	// clear current sample
	CurrentSample_Offset [ Channel ] = 0;
	CurrentSample_Read [ Channel ] = 0;
	CurrentSample_Write [ Channel ] = 0;
	
	// set start address
	CurrentBlockAddress [ Channel ] = SWAPH ( pChRegs1->SSA ) & ( c_iRam_Mask >> 1 );
	
	
	////////////////////////////////////////////////////////
	// check if the IRQ address is in this block
	// and check if interrupts are enabled
	// TODO: this actually doesn't happen until after 2T. Sound probably doesn't start until after 2T?
	/*
	if ( SPU2::_SPU2->SPU0.pCoreRegs0->CTRL & 0x40 )
	{
		
		if ( ( CurrentBlockAddress [ Channel ] & ( c_iRam_Mask >> 1 ) ) == ( ( SWAPH( SPU2::_SPU2->SPU0.pCoreRegs0->IRQA ) ) & ( c_iRam_Mask >> 1 ) ) )
		{
#ifdef INLINE_DEBUG_INT
	debug << "\r\nSPU:INT:ADDR ch#" << dec << Channel;
#endif

			cout << "\nInterrupt at sample decomding start addr: " << hex << CurrentBlockAddress [ Channel ] << " IRQA: " << SWAPH( SPU2::_SPU2->SPU0.pCoreRegs0->IRQA );

			// we have reached irq address - trigger interrupt
			SetInterrupt ();
			
			// do this for ps2
			//SetInterrupt_Core ( CoreNumber );
			SetInterrupt_Core ( 0 );
			
			// interrupt
			//pCoreRegs0->STAT |= 0x40;
			SPU2::_SPU2->SPU0.pCoreRegs0->STAT |= 0x40;
		}

	}	// end if ( SPU2::_SPU2->SPU0.pCoreRegs0->CTRL & 0x40 )

	if ( SPU2::_SPU2->SPU1.pCoreRegs0->CTRL & 0x40 )
	{
		if ( ( CurrentBlockAddress [ Channel ] & ( c_iRam_Mask >> 1 ) ) == ( ( SWAPH( SPU2::_SPU2->SPU1.pCoreRegs0->IRQA ) ) & ( c_iRam_Mask >> 1 ) ) )
		{
#ifdef INLINE_DEBUG_INT
	debug << "\r\nSPU:INT:ADDR ch#" << dec << Channel;
#endif

			// we have reached irq address - trigger interrupt
			SetInterrupt ();
			
			// do this for ps2
			//SetInterrupt_Core ( CoreNumber );
			SetInterrupt_Core ( 1 );
			
			// interrupt
			//pCoreRegs0->STAT |= 0x40;
			SPU2::_SPU2->SPU1.pCoreRegs0->STAT |= 0x40;
		}

	}	// end if ( SPU2::_SPU2->SPU1.pCoreRegs0->CTRL & 0x40 )
	*/

	//////////////////////////////////////////////////////////////////////////////
	// Check loop start flag and set loop address if needed
	// LOOP_START is bit 3 actually
	// note: LOOP_START is actually bit 2
	//if ( ( RAM [ CurrentBlockAddress [ Channel ] & ( c_iRam_Mask >> 1 ) ] & ( 0x4 << 8 ) ) && ( ! ( LSA_Manual_Bitmap & ( 1 << Channel ) ) ) )
	if ( ( RAM [ CurrentBlockAddress [ Channel ] & ( c_iRam_Mask >> 1 ) ] & ( 0x4 << 8 ) ) )
	{
#ifdef INLINE_DEBUG_WRITE_KON_0
	debug << "; Channel=" << Channel << "; LOOP_AT_START";
#endif

		///////////////////////////////////////////////////
		// we are at loop start address
		// set loop start address
		pChRegs1->LSA = SWAPH ( CurrentBlockAddress [ Channel ] );

	}
	
	// assume loop is set for now for channel
	bLoopSet |= (1 << Channel);

	// clear loop set if loop bit not set
	if ( ! ( RAM [ CurrentBlockAddress [ Channel ] & ( c_iRam_Mask >> 1 ) ] & ( 0x2 << 8 ) ) )
	{
		bLoopSet &= ~( 1 << Channel );
	}

#ifdef UPDATE_NEX_IN_BLOCKS
	// check if new block is going to loop
	/*
	if ( RAM [ CurrentBlockAddress [ Channel ] & ( c_iRam_Mask >> 1 ) ] & ( 0x1 << 8 ) )
	{
		pChRegs1->NEX = pChRegs1->LSA;
	}
	else
	{
		pChRegs1->NEX = SWAPH ( ( ( CurrentBlockAddress [ Channel ] & ~7 ) + 8 ) );
	}
	*/

	pChRegs1->NEX = SWAPH( ( CurrentBlockAddress [ Channel ] + 1 ) & ( c_iRam_Mask >> 1 ) );

#else

	// for ps2, need to set the next sample address also
	pChRegs1->NEX = SWAPH ( ( CurrentBlockAddress [ Channel ] | 1 ) & ( c_iRam_Mask >> 1 ) );

#endif


	// decode the new block
	
	// clear the samples first because of interpolation algorithm
	for ( int i = 0; i < 32; i++ ) DecodedBlocks [ ( Channel << 5 ) + i ] = 0;
	
#ifdef INLINE_DEBUG_SPU_ERROR_RECORD
	//u32 filter = ( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] >> 12 );
	u32 filter = ( RAM [ CurrentBlockAddress [ Channel ] ] >> 12 );
	if ( filter > 4 ) 
	{
		debug << "\r\nhpsx64 ALERT: SPU: Filter value is greater than 4 (invalid): filter=" << dec << filter << hex << " SPUAddress=" << CurrentBlockAddress [ Channel ] << " shifted=" << ( CurrentBlockAddress [ Channel ] >> 3 );
		debug << " Channel=" << dec << Channel << " LSA_X=" << hex << Regs [ ( ( Channel << 4 ) + LSA_X ) >> 1 ] << " SSA_X=" << Regs [ ( ( Channel << 4 ) + SSA_X ) >> 1 ];
	}
#endif
	
	// now decode the sample packet into buffer
	//SampleDecoder [ Channel ].decode_packet32 ( (adpcm_packet*) & ( RAM [ CurrentBlockAddress [ Channel ] >> 1 ] ), DecodedSamples );
	SampleDecoder [ Channel ].decode_packet32 ( (adpcm_packet*) & ( RAM [ CurrentBlockAddress [ Channel ] ] ), DecodedSamples );
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





u32 SPU2::Read ( u32 Address )
{
	u32 Offset;
	u32 Output;
	u32 Mask = 0xffff;
	
	// get the physical address of device
	Address &= 0x1fffffff;
	
	// get offset
	Offset = Address - 0x1f900000;
	
	// check if address is for first group of core0 registers
	if ( Offset < 0x400 )
	{
		Output = _SPU2->SPU0.Read ( Offset, Mask );
	}
	// check if address is for first group of core1 registers
	else if ( Offset < 0x760 )
	{
		//Output = _SPU2->SPU1.Read ( Offset - 0x400, Mask );
		Output = _SPU2->SPU1.Read ( Offset, Mask );
	}
	// check if address is for second group of core0 registers
	else if ( Offset < 0x788 )
	{
		Output = _SPU2->SPU0.Read ( Offset, Mask );
	}
	// check if address is for second group of core1 registers
	else if ( Offset < 0x7b0 )
	{
		// 20 registers in second group per core = 40 bytes
		//Output = _SPU2->SPU1.Read ( Offset - 0x28, Mask );
		Output = _SPU2->SPU1.Read ( Offset, Mask );
	}
	else if ( Offset < 0x800 )
	{
		// doesn't fit any of the other criteria, so this is in the area that applies to both cores
		
		if ( Offset >= 0x7c0 && Offset < 0x7ca )
		{
#ifdef INLINE_DEBUG_READ
			SPUCore::debug << "\r\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << hex << " Mask=" << Mask << " Offset= " << Offset;
			SPUCore::debug << "; " << SPUCore::RegisterNames3 [ ( Offset - 0x7c0 ) >> 1 ];
#endif

#ifdef VERBOSE_READ_TEST
			cout << "\nSPU::Read; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << hex << " Mask=" << Mask << " Offset= " << Offset;
			cout << "; " << SPUCore::RegisterNames3 [ ( Offset - 0x7c0 ) >> 1 ];
#endif
		}
		

		
		// handle any processing
		switch ( Offset )
		{
			default:
				break;
		}
		
		// read value
		//Output = _SPU2->Regs16 [ ( ( Offset - 0x7b0 ) >> 1 ) & c_iNumberOfRegisters_Mask ];
		Output = _SPU2->Regs16 [ ( ( Offset ) >> 1 ) & c_iNumberOfRegisters_Mask ];
		
#ifdef INLINE_DEBUG_READ
			SPUCore::debug << " Output=" << hex << Output;
#endif
	}
	else
	{
		cout << "\nhps1x64: SPU2 READ from Address >=0x800; Address=" << hex << Address;
	}
	
	return Output;
}

void SPU2::Write ( u32 Address, u32 Data, u32 Mask )
{
	u32 Offset;
	
	// get the physical address of device
	Address &= 0x1fffffff;
	
	// get offset
	Offset = Address - 0x1f900000;
	
	// check if address is for first group of core0 registers
	if ( Offset < 0x400 )
	{
		_SPU2->SPU0.Write ( Offset, Data, Mask );
	}
	// check if address is for first group of core1 registers
	else if ( Offset < 0x760 )
	{
		//_SPU2->SPU1.Write ( Offset - 0x400, Data, Mask );
		_SPU2->SPU1.Write ( Offset, Data, Mask );
	}
	// check if address is for second group of core0 registers
	else if ( Offset < 0x788 )
	{
		_SPU2->SPU0.Write ( Offset, Data, Mask );
	}
	// check if address is for second group of core1 registers
	else if ( Offset < 0x7b0 )
	{
		// 20 registers in second group per core = 40 bytes
		//_SPU2->SPU1.Write ( Offset - 0x28, Data, Mask );
		_SPU2->SPU1.Write ( Offset, Data, Mask );
	}
	else if ( Offset < 0x800 )
	{
		// doesn't fit any of the other criteria, so this is in the area that applies to both cores
		
		if ( Offset >= 0x7c0 && Offset < 0x7ca )
		{
#ifdef INLINE_DEBUG_WRITE
	SPUCore::debug << "\r\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << hex << " Mask=" << Mask << " Offset= " << Offset << " Data=" << Data;
	SPUCore::debug << "; " << SPUCore::RegisterNames3 [ ( Offset - 0x7c0 ) >> 1 ];
#endif

#ifdef VERBOSE_WRITE_TEST
	cout << "\nSPU::Write; " << hex << setw(8) << *_DebugPC << " " << dec << *_DebugCycleCount << hex << " Mask=" << Mask << " Offset= " << Offset << " Data=" << Data;
	cout << "; " << SPUCore::RegisterNames3 [ ( Offset - 0x7c0 ) >> 1 ];
#endif
		}


		// handle any processing
		switch ( Offset )
		{
			case SPUCore::SPDIF_OUT:
				// ***TODO*** says what type of data??
#ifdef VERBOSE_SPDIF_OUT_TEST
				if ( Data & 0x4 )
				{
					cout << "\nhps1x64: SPU2: ALERT: 24/32-bit PCM Data sound mode set!!!";
				}
				else if ( Data & 0x100 )
				{
					cout << "\nhps1x64: SPU2: ALERT: BYPASS PCM Data sound mode set!!!";
				}
				// otherwise there is no SPDIF output
#endif
				break;
				
			case SPUCore::SPDIF_IRQINFO:
				// ***TODO*** only gets reset on interrupt transition from disable to enable ??
				break;
				
			default:
				break;
		}
		
		// write value
		//_SPU2->Regs16 [ ( ( Offset - 0x7b0 ) >> 1 ) & c_iNumberOfRegisters_Mask ] = Data;
		_SPU2->Regs16 [ ( ( Offset ) >> 1 ) & c_iNumberOfRegisters_Mask ] = Data;
	}
	else
	{
		cout << "\nhps1x64: SPU2 WRITE to Address >=0x800; Address=" << hex << Address << " Data=" << Data;
	}
}







void SPUCore::DebugWindow_Enable ( int Number )
{

#ifndef _CONSOLE_DEBUG_ONLY_

	static constexpr int SPUList_CountX = 9;
	
	static constexpr int SPUMasterList_X = 0;
	static constexpr int SPUMasterList_Y = 0;
	static constexpr int SPUMasterList_Width = 100;
	static constexpr int SPUMasterList_Height = 130;
	
	static constexpr int SPUList_X = 0;
	static constexpr int SPUList_Y = 0;
	static constexpr int SPUList_Width = SPUMasterList_Width;
	static constexpr int SPUList_Height = SPUMasterList_Height;
	
	const char* DebugWindow_Caption = "SPU1 Debug Window";
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
	
	SPUCore *_SPU;

	CoreRegs0_Layout *pCoreRegs0;
	CoreRegs1_Layout *pCoreRegs1;
	
	ChRegs0_Layout *pChRegs0;
	ChRegs1_Layout *pChRegs1;
	
	
	if ( !DebugWindow_Enabled [ Number ] )
	{
		switch ( Number )
		{
			case 0:
				_SPU = & SPU2::_SPU2->SPU0;
				break;
				
			case 1:
				_SPU = & SPU2::_SPU2->SPU1;
				break;
				
			default:
				cout << "\nhps2x64: ERROR: SPU2 core number does not exist.\n";
				return;
				break;
				
		};
		
		pCoreRegs0 = (CoreRegs0_Layout*) ( & ( _SPU->Regs16.u [ ( Number << 10 ) >> 1 ] ) );
		pCoreRegs1 = (CoreRegs1_Layout*) ( & ( _SPU->Regs16.u [ ( 0x760 >> 1 ) + ( Number * 20 ) ] ) );
		
		// disable debug window for SPU2 for now to get it started
		// create the main debug window
		DebugWindow [ Number ] = new WindowClass::Window ();
		DebugWindow [ Number ]->Create ( DebugWindow_Caption, DebugWindow_X, DebugWindow_Y, DebugWindow_Width, DebugWindow_Height );
		DebugWindow [ Number ]->DisableCloseButton ();
		
		// create "value lists"
		SPUMaster_ValueList [ Number ] = new DebugValueList<u16> ();
		for ( i = 0; i < NumberOfChannels; i++ ) SPU_ValueList [ i ] [ Number ] = new DebugValueList<u16> ();
		
		// create the value lists
		SPUMaster_ValueList [ Number ]->Create ( DebugWindow [ Number ], SPUMasterList_X, SPUMasterList_Y, SPUMasterList_Width, SPUMasterList_Height, true, false );
		
		for ( i = 0; i < NumberOfChannels; i++ )
		{
			SPU_ValueList [ i ] [ Number ]->Create ( DebugWindow [ Number ], SPUList_X + ( ( ( i + 1 ) % SPUList_CountX ) * SPUList_Width ), SPUList_Y + ( ( ( i + 1 ) / SPUList_CountX ) * SPUList_Height ), SPUList_Width, SPUList_Height, true, false );
		}
		

		
		//SPUMaster_ValueList->AddVariable ( "CON_0", & (_SPU->Regs [ ( SPU::CON_0 - SPU::SPU_X ) >> 1 ]) );
		//SPUMaster_ValueList->AddVariable ( "CON_1", & (_SPU->Regs [ ( SPU::CON_1 - SPU::SPU_X ) >> 1 ]) );
		//SPUMaster_ValueList->AddVariable ( "KON_0", & (_SPU->Regs [ ( SPU::KON_0 - SPU::SPU_X ) >> 1 ]) );
		//SPUMaster_ValueList->AddVariable ( "KON_1", & (_SPU->Regs [ ( SPU::KON_1 - SPU::SPU_X ) >> 1 ]) );
		//SPUMaster_ValueList->AddVariable ( "KOFF_0", & (_SPU->Regs [ ( SPU::KOFF_0 - SPU::SPU_X ) >> 1 ]) );
		//SPUMaster_ValueList->AddVariable ( "KOFF_1", & (_SPU->Regs [ ( SPU::KOFF_1 - SPU::SPU_X ) >> 1 ]) );
		//SPUMaster_ValueList->AddVariable ( "MVOL_L", & (_SPU->Regs [ ( SPU::MVOL_L - SPU::SPU_X ) >> 1 ]) );
		//SPUMaster_ValueList->AddVariable ( "MVOL_R", & (_SPU->Regs [ ( SPU::MVOL_R - SPU::SPU_X ) >> 1 ]) );
		//SPUMaster_ValueList->AddVariable ( "EVOL_L", & (_SPU->Regs [ ( SPU::EVOL_L - SPU::SPU_X ) >> 1 ]) );
		//SPUMaster_ValueList->AddVariable ( "EVOL_R", & (_SPU->Regs [ ( SPU::EVOL_R - SPU::SPU_X ) >> 1 ]) );
		//SPUMaster_ValueList->AddVariable ( "SPU_CTRL", & (_SPU->Regs [ ( 0x1f801daa - SPU::SPU_X ) >> 1 ]) );
		//SPUMaster_ValueList->AddVariable ( "SPU_STAT", & (_SPU->Regs [ ( 0x1f801dae - SPU::SPU_X ) >> 1 ]) );
		
		
		SPUMaster_ValueList [ Number ]->AddVariable ( "CON_0", & pCoreRegs0->CON_0 );
		SPUMaster_ValueList [ Number ]->AddVariable ( "CON_1", & pCoreRegs0->CON_1 );
		SPUMaster_ValueList [ Number ]->AddVariable ( "KON_0", & pCoreRegs0->KON_0 );
		SPUMaster_ValueList [ Number ]->AddVariable ( "KON_1", & pCoreRegs0->KON_1 );
		SPUMaster_ValueList [ Number ]->AddVariable ( "KOFF_0", & pCoreRegs0->KOFF_0 );
		SPUMaster_ValueList [ Number ]->AddVariable ( "KOFF_1", & pCoreRegs0->KOFF_1 );

		SPUMaster_ValueList [ Number ]->AddVariable ( "MVOL_L", (u16*) & pCoreRegs1->MVOL_L );
		SPUMaster_ValueList [ Number ]->AddVariable ( "MVOL_R", (u16*) & pCoreRegs1->MVOL_R );

		
		//SPUMaster_ValueList [ Number ]->AddVariable ( "EVOL_L", & pCoreRegs1->vLOUT );
		//SPUMaster_ValueList [ Number ]->AddVariable ( "EVOL_R", & pCoreRegs1->vROUT );
		SPUMaster_ValueList [ Number ]->AddVariable ( "IRQA_0", & pCoreRegs0->IRQA_0 );
		SPUMaster_ValueList [ Number ]->AddVariable ( "IRQA_1", & pCoreRegs0->IRQA_1 );
		SPUMaster_ValueList [ Number ]->AddVariable ( "CTRL", & pCoreRegs0->CTRL );
		SPUMaster_ValueList [ Number ]->AddVariable ( "STAT", & pCoreRegs0->STAT );

		SPUMaster_ValueList[Number]->AddVariable("MMIX", &pCoreRegs0->MMIX);

		SPUMaster_ValueList[Number]->AddVariable("PMON_0", &pCoreRegs0->PMON_0);
		SPUMaster_ValueList[Number]->AddVariable("PMON_1", &pCoreRegs0->PMON_1);
		SPUMaster_ValueList[Number]->AddVariable("NON_0", &pCoreRegs0->NON_0);
		SPUMaster_ValueList[Number]->AddVariable("NON_1", &pCoreRegs0->NON_1);

		SPUMaster_ValueList[Number]->AddVariable("vIIR", (u16*) & pCoreRegs1->vIIR);
		SPUMaster_ValueList[Number]->AddVariable("ADMAS", &pCoreRegs0->ADMAS);

		SPUMaster_ValueList[Number]->AddVariable("SBA_0", &pCoreRegs0->SBA_0);
		SPUMaster_ValueList[Number]->AddVariable("SBA_1", &pCoreRegs0->SBA_1);

		SPUMaster_ValueList[Number]->AddVariable("VMIXL_0", &pCoreRegs0->VMIXL_0);
		SPUMaster_ValueList[Number]->AddVariable("VMIXL_1", &pCoreRegs0->VMIXL_1);
		SPUMaster_ValueList[Number]->AddVariable("VMIXR_0", &pCoreRegs0->VMIXR_0);
		SPUMaster_ValueList[Number]->AddVariable("VMIXR_1", &pCoreRegs0->VMIXR_1);
		SPUMaster_ValueList[Number]->AddVariable("VMIXEL_0", &pCoreRegs0->VMIXEL_0);
		SPUMaster_ValueList[Number]->AddVariable("VMIXEL_1", &pCoreRegs0->VMIXEL_1);
		SPUMaster_ValueList[Number]->AddVariable("VMIXER_0", &pCoreRegs0->VMIXER_0);
		SPUMaster_ValueList[Number]->AddVariable("VMIXER_1", &pCoreRegs0->VMIXER_1);

		SPUMaster_ValueList[Number]->AddVariable("RVWAS_0", &pCoreRegs0->RVWAS_0);
		SPUMaster_ValueList[Number]->AddVariable("RVWAS_1", &pCoreRegs0->RVWAS_1);
		//SPUMaster_ValueList[Number]->AddVariable("RVWAE_0", &pCoreRegs0->RVWAE_0);
		SPUMaster_ValueList[Number]->AddVariable("RVWAE_1", &pCoreRegs0->RVWAE_1);

		
		// add variables into lists
		for ( i = 0; i < NumberOfChannels; i++ )
		{
			static const char* c_sChannelStr = "C";
			
			pChRegs0 = (ChRegs0_Layout*) ( & ( pCoreRegs0->ChRegs0 [ i ] ) );
			pChRegs1 = (ChRegs1_Layout*) ( & ( pCoreRegs0->ChRegs1 [ i ] ) );
			
			ss.str ("");
			ss << c_sChannelStr << dec << i << "_VOLL";
			//SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::VOL_L ) >> 1 ]) );
			SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), (u16*) & pChRegs0->VOL_L );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_VOLR";
			//SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::VOL_L ) >> 1 ]) );
			SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), (u16*) & pChRegs0->VOL_R );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_PITCH";
			//SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::PITCH ) >> 1 ]) );
			SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), & pChRegs0->PITCH );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_SSA0";
			//SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::SSA_X ) >> 1 ]) );
			SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), & pChRegs1->SSA_0 );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_SSA1";
			//SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::SSA_X ) >> 1 ]) );
			SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), & pChRegs1->SSA_1 );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_ADSR0";
			//SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::ADSR_0 ) >> 1 ]) );
			SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), & pChRegs0->ADSR_0 );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_ADSR1";
			//SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::ADSR_1 ) >> 1 ]) );
			SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), & pChRegs0->ADSR_1 );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_ENVX";
			//SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::ENV_X ) >> 1 ]) );
			SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), & pChRegs0->ENV_X );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_LSA0";
			//SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::LSA_X ) >> 1 ]) );
			SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), & pChRegs1->LSA_0 );
			
			ss.str ("");
			ss << c_sChannelStr << i << "_LSA1";
			//SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), &(_SPU->Regs [ ( ( i << 4 ) + SPU::LSA_X ) >> 1 ]) );
			SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), & pChRegs1->LSA_1 );
			
			ss.str ("");
			ss << c_sChannelStr << i << "MADSR";
			SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), &(_SPU->ADSR_Status [ i ]) );
			
			ss.str ("");
			ss << c_sChannelStr << i << "RAW";
			SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), (u16*) &(_SPU->Debug_CurrentRawSample [ i ]) );
			
			ss.str ("");
			ss << c_sChannelStr << i << "SMP";
			SPU_ValueList [ i ] [ Number ]->AddVariable ( ss.str().c_str(), &(_SPU->Debug_CurrentSample [ i ]) );
			
			//ss.str ("");
			//ss << c_sChannelStr << i << "RATE";
			//SPU_ValueList [ i ]->AddVariable ( ss.str().c_str(), &(_SPU->Debug_CurrentRate [ i ]) );
		}
		
		// create the viewer for D-Cache scratch pad
		SoundRAM_Viewer [ Number ] = new Debug_MemoryViewer ();
		
		SoundRAM_Viewer [ Number ]->Create ( DebugWindow [ Number ], MemoryViewer_X, MemoryViewer_Y, MemoryViewer_Width, MemoryViewer_Height, MemoryViewer_Columns );
		SoundRAM_Viewer [ Number ]->Add_MemoryDevice ( "SoundRAM", 0, c_iRam_Size, (unsigned char*) _SPU->RAM );
		
		// mark debug as enabled now
		DebugWindow_Enabled [ Number ] = true;
		
		// update the value lists
		DebugWindow_Update ( Number );
	}
	
	
#endif

}

void SPUCore::DebugWindow_Disable ( int Number )
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;
	
	// this will be part of the interface
	
	if ( DebugWindow_Enabled [ Number ] )
	{
		delete DebugWindow [ Number ];
		delete SPUMaster_ValueList [ Number ];
		for ( i = 0; i < NumberOfChannels; i++ ) delete SPU_ValueList [ i ] [ Number ];
	
		// disable debug window
		DebugWindow_Enabled [ Number ] = false;
	}
	
	
#endif

}

void SPUCore::DebugWindow_Update ( int Number )
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;
	
	// this will be part of the interface
	
	if ( DebugWindow_Enabled [ Number ] )
	{
		SPUMaster_ValueList [ Number ]->Update();
		for ( i = 0; i < NumberOfChannels; i++ ) SPU_ValueList [ i ] [ Number ]->Update();
	}
	
#endif

}






