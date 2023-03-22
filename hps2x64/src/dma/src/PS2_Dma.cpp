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


#include "PS2_Dma.h"
#include "PS2_GPU.h"
#include "PS2_SIF.h"
#include "PS2_IPU.h"

#include "VU.h"

#include "R3000A.h"

//using namespace Playstation2;


//#define ENABLE_NEW_SIF


//#define ENABLE_SIF_DMA_TIMING
//#define ENABLE_SIF_DMA_SYNC

#define TEST_ASYNC_DMA
#define TEST_ASYNC_DMA_STAGE2


// attempt to synchronize transfers with ps1 for now
#define ENABLE_PS1_SYNC_DMA5
//#define ENABLE_PS1_SYNC_DMA6


//#define ENABLE_DMA_END_REMOTE


#define VERBOSE_CHANNEL_INTERRUPTED


#define VERBOSE_CHAIN

//#define VERBOSE_CYCLESTEAL
//#define VERBOSE_MFIFO
#define VERBOSE_STS
#define VERBOSE_STD
#define VERBOSE_RCYC


//#define VERBOSE_DEBUG_DMA2_TTE


/*
#define VERBOSE_NORMAL_TOMEM
#define VERBOSE_NORMAL_FROMMEM
#define VERBOSE_CHAIN_FROMMEM
#define VERBOSE_CHAIN_TOMEM
*/

//#define PRIORITY_DMA0
// don't want this to race through and get invalid vif code errors
//#define PRIORITY_DMA1


//#define VERBOSE_DMA_1_7_TO_MEM

//#define VERBOSE_DMA_UNDERFLOW

//#define CLEAR_QWC_ON_COMPLETE

// restart vu command when dma#1 ends
// note: appears that the command to the vu terminates when the dma transfer ends
//#define DMA1_END_RESTART_VUCOMMAND

// restart vu command when a new chain starts
//#define DMA1_CHAIN_RESTART_VUCOMMAND


// appears that tag transfer to vu strictly only transfers the high 64-bits ??
#define DMA01_TAG_TRANSFER_HIGH

//#define ONLY_TRANSFER_MFIFO_SOURCE_ON_EQUAL

//#define INT_ON_DMA5_STOP

//#define UPDATE_QWC_ONLYONSLICE

//#define CLEAR_TADR_ON_COMPLETE
//#define SET_QWC_CHAIN_TRANSFER


// if ps1 is busy, then delay dma start for sif 0/1
//#define ALIGN_DMA5_START_WITH_PS1
//#define ALIGN_DMA6_START_WITH_PS1


// disable channel completely if PCE sets it to low priority
//#define ENABLE_PCE_CHANNEL_DISABLE


#define FIX_UNALIGNED_READ
#define FIX_UNALIGNED_WRITE



#ifdef _DEBUG_VERSION_

// enable debugging

#define INLINE_DEBUG_ENABLE


#define INLINE_DEBUG_WRITE
#define INLINE_DEBUG_READ
#define INLINE_DEBUG_START
#define INLINE_DEBUG_END
//#define INLINE_DEBUG_CHECK

//#define INLINE_DEBUG_SPR_IN
//#define INLINE_DEBUG_SPR_OUT
//#define INLINE_DEBUG_SPR_OUT_DATA

//#define INLINE_DEBUG_TRANSFER_NORMAL_TOMEM

#define INLINE_DEBUG_TRANSFER
//#define INLINE_DEBUG_TRANSFER_NORMAL
//#define INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
//#define INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
#define INLINE_DEBUG_TRANSFER_CHAIN
#define INLINE_DEBUG_TRANSFER_CHAIN_TOMEM
#define INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
#define INLINE_DEBUG_TRANSFER_CHAIN_0
#define INLINE_DEBUG_TRANSFER_CHAIN_1
#define INLINE_DEBUG_TRANSFER_CHAIN_2
#define INLINE_DEBUG_TRANSFER_CHAIN_3
#define INLINE_DEBUG_TRANSFER_CHAIN_4
#define INLINE_DEBUG_TRANSFER_CHAIN_5
#define INLINE_DEBUG_TRANSFER_CHAIN_6
#define INLINE_DEBUG_TRANSFER_CHAIN_7
#define INLINE_DEBUG_TRANSFER_CHAIN_8

#define INLINE_DEBUG_RUN_DMA5

//#define INLINE_DEBUG_GETNEXTACTIVECHANNEL
//#define INLINE_DEBUG_GETNEXTACTIVECHANNEL2


//#define INLINE_DEBUG_NEXTEVENT
//#define INLINE_DEBUG_TEST5
//#define INLINE_DEBUG_INT



//#define INLINE_DEBUG_COMPLETE

//#define INLINE_DEBUG_READ_CHCR
//#define INLINE_DEBUG_WRITE_CHCR
//#define INLINE_DEBUG_READ_CTRL
//#define INLINE_DEBUG_WRITE_CTRL
//#define INLINE_DEBUG_READ_INVALID
//#define INLINE_DEBUG_WRITE_INVALID

//#define INLINE_DEBUG_WRITE_DMA2
//#define INLINE_DEBUG_RUN_DMA2
//#define INLINE_DEBUG_RUN_DMA2_CO


//#define INLINE_DEBUG_WRITE_PCR
//#define INLINE_DEBUG_READ_PCR

//#define INLINE_DEBUG

//#define INLINE_DEBUG_RUN_DMA0
//#define INLINE_DEBUG_RUN_DMA1
//#define INLINE_DEBUG_RUN_DMA3
//#define INLINE_DEBUG_RUN_DMA6
//#define INLINE_DEBUG_RUN_DMA4
//#define INLINE_DEBUG_RUN
//#define INLINE_DEBUG_CD
//#define INLINE_DEBUG_SPU
//#define INLINE_DEBUG_ACK


#endif



#define ENABLE_MEMORY_INVALIDATE


namespace Playstation2
{

u32* Dma::_SBUS_F240;

u32* Dma::_DebugPC;
u64* Dma::_DebugCycleCount;
u32* Dma::_NextEventIdx;

u32* Dma::_R5900_Status;

//u32* Dma::_Intc_Master;
u32* Dma::_Intc_Stat;
u32* Dma::_Intc_Mask;
u32* Dma::_R5900_Cause_13;
u32* Dma::_R5900_Status_12;
u64* Dma::_ProcStatus;

u32* Dma::_CPCOND0_Out;


Debug::Log Dma::debug;

Dma *Dma::_DMA;

bool Dma::DebugWindow_Enabled;
WindowClass::Window *Dma::DebugWindow;
DebugValueList<u32> *Dma::DMA_ValueList;



int DmaChannel::Count = 0;


u64* Dma::_NextSystemEvent;


//DataBus *Dma::_BUS;
//MDEC *Dma::_MDEC;
//GPU *Dma::_GPU;
//R5900::Cpu *Dma::_CPU;


DmaChannel::RegData* Dma::pRegData [ Dma::c_iNumberOfChannels ];
Dma::DMARegs_t* Dma::pDMARegs;



const char* Dma::DmaCh_Names [ c_iNumberOfChannels ] = { "VU0/VIF0", "VU1/VIF1", "GPU/GIF", "MDEC/IPU out", "MDEC/IPU in", "SIF0 (from SIF/IOP)", "SIF1 (to SIF/IOP)",
													"SIF2", "SPR out", "SPR in" };

													

const char* Dma::Reg_Names [ 32 * 16 ] = { "D0_CHCR", "D0_MADR", "D0_QWC", "D0_TADR", "D0_ASR0", "D0_ASR1", "D0_Res0", "D0_Res1", "D0_Res2", "D0_Res3", "D0_Res4", "D0_Res5", "D0_Res6", "D0_Res7", "D0_Res8", "D0_Res9",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"D1_CHCR", "D1_MADR", "D1_QWC", "D1_TADR", "D1_ASR0", "D1_ASR1", "D1_Res0", "D1_Res1", "D1_Res2", "D1_Res3", "D1_Res4", "D1_Res5", "D1_Res6", "D1_Res7", "D1_Res8", "D1_Res9",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"D2_CHCR", "D2_MADR", "D2_QWC", "D2_TADR", "D2_ASR0", "D2_ASR1", "D2_Res0", "D2_Res1", "D2_Res2", "D2_Res3", "D2_Res4", "D2_Res5", "D2_Res6", "D2_Res7", "D2_Res8", "D2_Res9",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"D3_CHCR", "D3_MADR", "D3_QWC", "D3_ResA", "D3_ResB", "D3_ResC", "D3_Res0", "D3_Res1", "D3_Res2", "D3_Res3", "D3_Res4", "D3_Res5", "D3_Res6", "D3_Res7", "D3_Res8", "D3_Res9",
										"D4_CHCR", "D4_MADR", "D4_QWC", "D4_TADR", "D4_ResB", "D4_ResC", "D4_Res0", "D4_Res1", "D4_Res2", "D4_Res3", "D4_Res4", "D4_Res5", "D4_Res6", "D4_Res7", "D4_Res8", "D4_Res9",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"D5_CHCR", "D5_MADR", "D5_QWC", "D5_ResA", "D5_ResB", "D5_ResC", "D5_Res0", "D5_Res1", "D5_Res2", "D5_Res3", "D5_Res4", "D5_Res5", "D5_Res6", "D5_Res7", "D5_Res8", "D5_Res9",
										"D6_CHCR", "D6_MADR", "D6_QWC", "D6_TADR", "D6_ResB", "D6_ResC", "D6_Res0", "D6_Res1", "D6_Res2", "D6_Res3", "D6_Res4", "D6_Res5", "D6_Res6", "D6_Res7", "D6_Res8", "D6_Res9",
										"D7_CHCR", "D7_MADR", "D7_QWC", "D7_ResA", "D7_ResB", "D7_ResC", "D7_Res0", "D7_Res1", "D7_Res2", "D7_Res3", "D7_Res4", "D7_Res5", "D7_Res6", "D7_Res7", "D7_Res8", "D7_Res9",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"D8_CHCR", "D8_MADR", "D8_QWC", "D8_ResA", "D8_ResB", "D8_ResC", "D8_Res0", "D8_Res1", "D8_SADR", "D8_Res3", "D8_Res4", "D8_Res5", "D8_Res6", "D8_Res7", "D8_Res8", "D8_Res9",
										"D9_CHCR", "D9_MADR", "D9_QWC", "D9_TADR", "D9_ResB", "D9_ResC", "D9_Res0", "D9_Res1", "D9_SADR", "D9_Res3", "D9_Res4", "D9_Res5", "D9_Res6", "D9_Res7", "D9_Res8", "D9_Res9",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"D_CTRL", "D_STAT", "D_PCR", "D_SQWC", "D_RBSR", "D_RBOR", "D_STADR", "D_Res7", "D_Res8", "D_Res9", "D_ResA", "D_ResB", "D_ResC", "D_ResD", "D_ResE", "D_ResF"
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "ENABLER", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "ENABLEW", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF"
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										"RES0", "RES1", "RES2", "RES3", "RES4", "RES5", "RES6", "RES7", "RES8", "RES9", "RESA", "RESB", "RESC", "RESD", "RESE", "RESF",
										};


const u64 Dma::c_iDmaSetupTime [ c_iNumberOfChannels ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

const u64 Dma::c_iDmaTransferTimePerQwc [ c_iNumberOfChannels ] = {
c_iVU0_TransferTime,
c_iVU1_TransferTime,
c_iGIF_TransferTime,
c_iMDECout_TransferTime,
c_iMDECin_TransferTime,
c_iSIF0_TransferTime,
c_iSIF1_TransferTime,
1,	// sif2
1,	// spr out
1 };	// spr in

//const u64 Dma::c_iDmaTransferTimePerQwc [ c_iNumberOfChannels ] = { 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 };
//const u64 Dma::c_iDmaTransferTimePerQwc [ c_iNumberOfChannels ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

const u64 Dma::c_iDeviceBufferSize [ c_iNumberOfChannels ] = 
{
// dma 0 - vu0
8,
// dma 1 - vu1
16,
// dma2 - gpu
16,
// dma 3 - MDEC/IPU out	
8,
// dma 4 - MDEC/IPU in
8,
// dma 5 - sif0 iop->ee
8,
// dma 6 - sif1 ee->iop
1000000,
// dma 7 - sif2
8,
// dma 8
1000000,
// dma 9
1000000
};

Dma::fnReady Dma::cbReady [ c_iNumberOfChannels ] = {
// channel 0 - VU0/VIF0
VU0::DMA_Write_Ready,
// channel 1 - VU1/VIF1
VU1::DMA_Write_Ready,
// channel 2 - GPU/GIF
GPU::DMA_Write_Ready,
// channel 3 - MDEC/IPU out
IPU::DMA_Read_Ready,
// channel 4 - MDEC/IPU in
IPU::DMA_Write_Ready,
// channel 5 - SIF0 (from SIF/IOP)
SIF::IOP_To_EE_DMA_Ready_FromEE,	//SIF::IOP_To_EE_DMA_Ready,	//SIF::IOP_DMA_Out_Ready,
// channel 6 - SIF1 (to SIF/IOP)
SIF::EE_To_IOP_DMA_Ready_FromEE,	//SIF::EE_To_IOP_DMA_Ready,	//SIF::IOP_DMA_In_Ready,
// channel 7 - SIF2
NULL,
// channel 8 - SPR out
SPRout_DMA_Ready,
// channel 9 - SPR in
SPRin_DMA_Ready
};

Dma::fnReady Dma::cbReady_ToMemory [ c_iNumberOfChannels ] = {
// channel 0 - VU0/VIF0
VU0::DMA_Write_Ready,
// channel 1 - VU1/VIF1
VU1::DMA_Read_Ready,
// channel 2 - GPU/GIF
GPU::DMA_Write_Ready,
// channel 3 - MDEC/IPU out
IPU::DMA_Read_Ready,
// channel 4 - MDEC/IPU in
IPU::DMA_Write_Ready,
// channel 5 - SIF0 (from SIF/IOP)
SIF::IOP_To_EE_DMA_Ready_FromEE,	//SIF::IOP_To_EE_DMA_Ready,	//SIF::IOP_DMA_Out_Ready,
// channel 6 - SIF1 (to SIF/IOP)
SIF::EE_To_IOP_DMA_Ready_FromEE,	//SIF::EE_To_IOP_DMA_Ready,	//SIF::IOP_DMA_In_Ready,
// channel 7 - SIF2
NULL,
// channel 8 - SPR out
SPRout_DMA_Ready,
// channel 9 - SPR in
SPRin_DMA_Ready
};
Dma::fnTransfer_FromMemory Dma::cbTransfer_FromMemory [ c_iNumberOfChannels ] = {
// channel 0 - VU0/VIF0
VU0::DMA_WriteBlock,
// channel 1 - VU1/VIF1
VU1::DMA_WriteBlock,
// channel 2 - GPU/GIF
GPU::DMA_WriteBlock,
// channel 3 - MDEC/IPU out
NULL,
// channel 4 - MDEC/IPU in
IPU::DMA_WriteBlock,
// channel 5 - SIF0 (from SIF/IOP)
NULL,
// channel 6 - SIF1 (to SIF/IOP)
SIF::EE_DMA_WriteBlock,
// channel 7 - SIF2
NULL,
// channel 8 - SPR out
SPRout_DMA_Read,
// channel 9 - SPR in
SPRin_DMA_Write
};


Dma::fnTransfer_FromMemory Dma::cbTransfer_ToMemory [ c_iNumberOfChannels ] = {
// channel 0 - VU0/VIF0
NULL,
// channel 1 - VU1/VIF1
VU1::DMA_ReadBlock,
// channel 2 - GPU/GIF
NULL,
// channel 3 - MDEC/IPU out
IPU::DMA_ReadBlock,
// channel 4 - MDEC/IPU in
NULL,
// channel 5 - SIF0 (from SIF/IOP)
NULL,
// channel 6 - SIF1 (to SIF/IOP)
NULL,
// channel 7 - SIF2
NULL,
// channel 8 - SPR out
SPRout_DMA_Read,
// channel 9 - SPR in
NULL
};


const u32 Dma::c_iStallSource_LUT [ 4 ] = { -1, 5, 8, 3 };
const u32 Dma::c_iStallDest_LUT [ 4 ] = { -1, 1, 2, 6 };
const u32 Dma::c_iMfifoDrain_LUT [ 4 ] = { -1, -1, 1, 2 };


Dma::Dma ()
{
	cout << "Running DMA constructor...\n";

/*	
#ifdef INLINE_DEBUG_ENABLE
	debug.Create ( "DMAController_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering DMA controller constructor";
#endif


	// set the current dma object
	_DMA = this;

	Reset ();
	
	
#ifdef INLINE_DEBUG
	debug << "->Exiting DMA controller constructor";
#endif
*/

}


void Dma::Start ()
{
	cout << "Running PS2::DMA::Start...\n";

#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create ( "PS2_DMA_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering DMA::Start";
#endif

	// set the current dma object
	_DMA = this;

	Reset ();
	
	// none of the dma channels are running yet
	for ( int iChannel = 0; iChannel < c_iNumberOfChannels; iChannel++ )
	{
		QWC_Transferred [ iChannel ] = -1;
	}
	
	// ???
	//lDMAC_ENABLE = 0x1201;
	pDMARegs->ENABLEW = 0x1201;
	pDMARegs->ENABLER = 0x1201;
	
	
	// clear events
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		SetNextEventCh_Cycle ( -1ULL, i );
	}
	
#ifdef INLINE_DEBUG
	debug << "->Exiting PS2::DMA::Start";
#endif
}



void Dma::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( Dma ) );
	
	// set static pointers
	Refresh ();
	
	// allow all dma channels to run
	//SelectedDMA_Bitmap = 0xffffffff;
	
	// no dma channels are active
	//ActiveChannel = -1;
}


/*
void Dma::ConnectDevices ( DataBus *BUS, MDEC* mdec, GPU *g, CD *cd, SPU *spu, R5900::Cpu *cpu )
{
	_BUS = BUS;
	_MDEC = mdec;
	_GPU = g;
	_CPU = cpu;
}
*/


int Dma::GetNextActiveChannel ()
{
#ifdef INLINE_DEBUG_GETNEXTACTIVECHANNEL
	debug << "\r\nDma::GetNextActiveChannel";
#endif

	int iChannel, iCurrentCh;
	u32 ulCurrentScore, ulCheckScore;

	// get channel with the highest priority //
	
	// start with channel 0
	iCurrentCh = -1;
	ulCurrentScore = 0;
	for ( iChannel = 0; iChannel < c_iNumberOfChannels; iChannel++ )
	{
#ifdef INLINE_DEBUG_GETNEXTACTIVECHANNEL
		debug << " CH#" << dec << iChannel;
#endif

		ulCheckScore = Get_ChannelPriority ( iChannel );
		
#ifdef INLINE_DEBUG_GETNEXTACTIVECHANNEL
		debug << " SCR:" << dec << ulCheckScore;
#endif

		if ( ulCheckScore > ulCurrentScore )
		{
			iCurrentCh = iChannel;
			ulCurrentScore = ulCheckScore;
		}
	}

#ifdef INLINE_DEBUG_GETNEXTACTIVECHANNEL2
		debug << " NAC#" << dec << iCurrentCh;
#endif

	return iCurrentCh;
}


void Dma::Run ()
{
	//u32 Temp;
	//u32 Data [ 4 ];
	
	// will use this for MDEC for now
	//u32 NumberOfTransfers;
	int iChannel, iCurrentCh;
	u32 ulCurrentScore, ulCheckScore;
	
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nDma::Run";
	debug << " NextEvent=" << dec << NextEvent_Cycle;
	debug << " CycleCount=" << *_DebugCycleCount;
	//debug << " P3Q=" << _GPU->GIFRegs.STAT.P3Q;
#endif

	// check if dma is doing anything starting at this particular cycle
	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;
	
#ifdef IGNORE_CHANNEL_PRIORITY
	// check for the channel(s) that needs to be run
	for ( iChannel = 0; iChannel < c_iNumberOfChannels; iChannel++ )
	{
		// need to use the current cycle count to check what dma channel is to run
		if ( *_DebugCycleCount == NextEventCh_Cycle [ iChannel ] )
		{
#ifdef INLINE_DEBUG_RUN
	debug << " RUNNING CH#" << dec << iChannel;
#endif

			// ***todo*** check channel priority
			Transfer ( iChannel );
		}
		
	}
	
#else
	
	// get channel with the highest priority //
	
	iCurrentCh = GetNextActiveChannel ();


	// check if score is greater than zero
	//if ( ulCurrentScore )
	if ( iCurrentCh != -1 )
	{
#ifdef INLINE_DEBUG_RUN
	debug << " RUNNING CH#" << dec << iCurrentCh;
	//debug << " SCORE:" << dec << ulCurrentScore;
	debug << " LASTCH#" << dec << iLastChannel;
#endif

		// run the transfer
		Transfer ( iCurrentCh );
		
		// if score is less than next priority level, set as last channel that was run
		//if ( ulCurrentScore < 100 )
		//{
			iLastChannel = iCurrentCh;
		//}
	}
#endif
	
	// get the cycle number of the next event for device
	Update_NextEventCycle ();
	

}


u32 Dma::Get_ChannelPriority ( int iChannel )
{
	u32 ulPriority;
	u64 ullReadyCycle;
	
	// check if all dma is disabled //
	if ( ! pDMARegs->CTRL.DMAE )
	{
		return 0;
	}
	
	// check if dma transfers are being held
	if ( ( DMARegs.ENABLEW & 0x10000 ) /* && ( iChannel != 5 ) */ )
	{
		// all dma transfers disabled
		return 0;
	}
	
	// check if channel is started //
	if ( !pRegData [ iChannel ]->CHCR.STR )
	{
		// channel is not running if it has not been started
		return 0;
	}


	// also need to check cde bit to see if channel is enabled
	/*
	if ( ( DMARegs.PCR.PCE ) && !( ( DMARegs.PCR.Value >> 16 ) & ( 1 << iChannel ) ) )
	{

		return 0;
	}
	*/


	// channel is started //
	
	// check if channel is ready //
	
	// ***TODO*** check channel direction
	if ( pRegData [ iChannel ]->CHCR.DIR )
	{
		// from memory //
		
		// check there is a ready function
		if ( cbReady [ iChannel ] )
		{
			// get whether dma channel is ready or not or in the future
			ullReadyCycle = cbReady[iChannel]();

			// check if dma channel is ready or not
			if ( !ullReadyCycle )
			{
				return 0;
			}
			// check if dma channel will be ready in the future
			else if (ullReadyCycle > *_DebugCycleCount)
			{
				// make sure that event is set for in the future ??
				SetNextEventCh_Cycle(ullReadyCycle, iChannel);

				// channel is not transferring right now since device is busy
				return 0;
			}

			// otherwise dma channel is ready immdiately when equal to 1 //
		}
		else
		{
			return 0;
		}
	}
	else
	{
		// to memory //
		
		if ( cbReady_ToMemory [ iChannel ] )
		{
			// get whether dma channel is ready or not or in the future
			ullReadyCycle = cbReady_ToMemory[iChannel]();

			if ( !ullReadyCycle )
			{
				return 0;
			}
			// check if dma channel will be ready in the future
			else if (ullReadyCycle > *_DebugCycleCount)
			{
				// make sure that event is set for in the future ??
				SetNextEventCh_Cycle(ullReadyCycle, iChannel);

				// channel is not transferring right now since device is busy
				return 0;
			}

			// otherwise dma channel is ready immdiately when equal to 1 //
		}
		else
		{
			return 0;
		}
	}
	
	// channel is ready //
	
	// check round robin rotation (lowest priority) //
	

	// ( number of channels - channel ), then add number of channels if greter than last channel
	ulPriority = ( c_iNumberOfChannels - iChannel ) + ( ( iChannel > iLastChannel ) ? c_iNumberOfChannels : 0 );

	
	// check pce priority (highest priority) //
	if ( ( !pDMARegs->PCR.PCE ) || ( pDMARegs->PCR.Value & ( 1 << ( iChannel + 16 ) ) ) )
	{
		ulPriority += 1000;
	}
#ifdef ENABLE_PCE_CHANNEL_DISABLE
	else
	{
		// if channel is disabled, then it does not run?
		return 0;
	}
#endif
	
	// check if channel is zero (next highest priority) //
	if ( !iChannel ) ulPriority += 100;
	
	return ulPriority;
}


u64 Dma::Read ( u32 Address, u64 Mask )
{
#if defined INLINE_DEBUG_READ
	debug << "\r\n\r\nDMA::Read; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << " Mask=" << Mask;
#endif

	//static const u8 c_ucDMAChannel_LUT [ 32 ] = { 0, -1, -1, -1, 1, -1, -1, -1,
	//												2, -1, -1, -1, 3, 4, -1, -1,
	//												5, 6, 7, -1, 8, 9, -1, -1,
	//												-1, -1, -1, -1, -1, -1, -1, -1 };
	
	//u32 DmaChannelNum;
	u32 Shift;
	u64 Output64;
	
	// check if reading 128-bit value
	if ( !Mask )
	{
#if defined INLINE_DEBUG_READ
	debug << " (128-bit READ)";
#endif

		cout << "\nhps2x64: ALERT: DMA: Reading 128-bit value. Address=" << hex << Address << "\n";
	}


#ifdef VERBOSE_UNALIGNED_READ	
	// check if read not aligned
	if ( Address & 0xf )
	{
#if defined INLINE_DEBUG_READ
	debug << " NOT-ALIGNED";
#endif

		cout << "\nhps2x64: ALERT: DMA: Read not aligned. Address=" << hex << Address << "\n";
	}
#endif

	
#ifdef FIX_UNALIGNED_READ
	// get the amount to shift output right (it is possible to read from the high part or middle part of register if it is aligned)
	Shift = ( Address & 0x7 ) << 3;
#else
	Shift = 0;
#endif
	
	// get the upper part of address
	//Address &= ~0xf;
	
	// get lower 16-bits of address
	Address &= 0xffff;
	
	// subtract the offset (registers start at 0x10008000)
	Address -= 0x8000;
	
	// check that register is in range
	if ( Address < 0x8000 )
	{
#if defined INLINE_DEBUG_READ
			debug << " " << Reg_Names [ ( ( Address & 0xf0 ) >> 4 ) | ( ( Address & 0x7c00 ) >> 6 ) ];
#endif

		// read the value
		Output64 = pDMARegs->Regs [ ( ( Address & 0xf0 ) >> 4 ) | ( ( Address & 0x7c00 ) >> 6 ) ] >> Shift;
	
	}
	else
	{
#if defined INLINE_DEBUG_READ
			debug << " " << "INVALID";
#endif

		Output64 = 0;
	}
	
#if defined INLINE_DEBUG_READ
			debug << " = " << hex << Output64;
#endif

	return Output64;
	
}



void Dma::Write ( u32 Address, u64 Data, u64 Mask )
{
#if defined INLINE_DEBUG_WRITE
	debug << "\r\n\r\nDMA::Write; PC= " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << " Mask=" << Mask << " Data = " << Data;
#endif

	static const u8 c_ucDMAChannel_LUT [ 32 ] = { 0, -1, -1, -1, 1, -1, -1, -1,
													2, -1, -1, -1, 3, 4, -1, -1,
													5, 6, 7, -1, 8, 9, -1, -1,
													-1, -1, -1, -1, -1, -1, -1, -1 };

	// the dir field for only channels #1 and #7 are configurable, rest are fixed
	static const u32 c_ulChcrOR [ 10 ] = { 1, 0, 1, 0, 1, 0, 1, 0, 0, 1 };
	static const u32 c_ulChcrAND [ 10 ] = { -1, -1, -1, ~1, -1, ~1, -1, -1, ~1, -1 };

	u32 DmaChannelNum;
	u32 Temp;
	u32 Shift;
	u32 Offset;
	u32 PreviousValue;
	u64 Output64;

	
	// didn't account for a 128-bit write to the dma
	if ( !Mask )
	{
#if defined INLINE_DEBUG_WRITE
	debug << " (128-bit WRITE)";
#endif

		cout << "\nhps2x64: ALERT: DMA: 128-bit write. Address=" << hex << Address;
	}
	
	
#ifdef VERBOSE_UNALIGNED_WRITE
	// check if write not aligned
	if ( Address & 0xf )
	{
#if defined INLINE_DEBUG_WRITE
	debug << " NOT-ALIGNED";
#endif

		cout << "\nhps2x64: ALERT: DMA: WRITE not aligned. Address=" << hex << Address << "\n";
	}
#endif


#ifdef FIX_UNALIGNED_WRITE
	// get the amount to shift input left (it is possible to read from the high part or middle part of register if it is aligned)
	Shift = ( Address & 0x7 ) << 3;
	Mask <<= Shift;
	Data <<= Shift;
#else
	Shift = 0;
#endif
	
	// apply write mask here for now
	Data &= Mask;
	
	// get the upper part of address
	Address &= ~0xf;
	
	// get lower 16-bits of address
	Address &= 0xffff;
	
	// subtract the offset (registers start at 0x10008000)
	Address -= 0x8000;
	
	// check that register is in range
	if ( Address < 0x8000 )
	{
#if defined INLINE_DEBUG_WRITE
			debug << " " << Reg_Names [ ( ( Address & 0xf0 ) >> 4 ) | ( ( Address & 0x7c00 ) >> 6 ) ];
#endif

		// check if ENABLER register
		if ( Address == ( ( DMA_ENABLER & 0xffff ) - 0x8000 ) )
		{
			// ENABLER //

			// this should be read-only
			return;
		}


		// check if STAT register
		if ( Address == ( ( DMA_STAT & 0xffff ) - 0x8000 ) )
		{
			// STAT //

			// the bottom 16-bits get cleared when a one is written to the bit
			pDMARegs->STAT.Value &= ~( Data & 0xffff );
			
			// the upper 16-bits get inverted when a one is written to the bit
			// but keep bits 26-28 zero, and bit 31 zero
			pDMARegs->STAT.Value ^= ( Data & 0x63ff0000 );
			
			// *** TODO *** INT1 == CIS&CIM || BEIS
			// update interrupts when STAT gets modified
			_DMA->UpdateInterrupt ();
			
			// update CPCOND0
			_DMA->Update_CPCOND0 ();
			
			return;
		}
		

		// get offset to hardware register
		Offset = ( ( Address & 0xf0 ) >> 4 ) | ( ( Address & 0x7c00 ) >> 6 );
		
		// write the value
		PreviousValue = pDMARegs->Regs [ Offset ];
		pDMARegs->Regs [ Offset ] = ( PreviousValue & ~Mask ) | ( Data );
		
		// check if CTRL register
		if ( Address == ( ( DMA_CTRL & 0xffff ) - 0x8000 ) )
		{
			// CTRL //

			// check if DMAE bit changed
			if ( ( PreviousValue ^ Data ) & 0x1 )
			{
				// a condition has changed for ps1 dma, so update there first
				SIF::Update_ActiveChannel ();
				
				// check if there are any newly active dma transfers
				_DMA->CheckTransfer ();
			}
			
			return;
		}

		// check if PCR register
		if ( Address == ( ( DMA_PCR & 0xffff ) - 0x8000 ) )
		{
			// PCR //

			// update CPCOND0
			_DMA->Update_CPCOND0 ();
			
			// check if there are any newly active dma transfers
			_DMA->CheckTransfer ();
			
			return;
		}
		
		// check if ENABLEW register
		if ( Address == ( ( DMA_ENABLEW & 0xffff ) - 0x8000 ) )
		{
			// ENABLEW //

			// also write value to DMA ENABLER in case for when it gets read
			pDMARegs->ENABLER = pDMARegs->ENABLEW;

			// if there is any change, ps1 dma conditions updated
			if ( ( Data ^ PreviousValue ) & 0x10000 )
			{
				// a condition has changed for ps1 dma, so update there first
				SIF::Update_ActiveChannel ();

				_DMA->CheckTransfer ();
			}

#ifdef VERBOSE_CHANNEL_INTERRUPTED
			// check if a channel got interrupted during transfer
			if ( Data & 0x10000 )
			{
				// dma halted //
				for ( int iIdx = 0; iIdx < c_iNumberOfChannels; iIdx++ )
				{
					if ( pRegData [ iIdx ]->CHCR.STR && ( iIdx != 5 ) )
					{
						//cout << "\nhps2x64: ALERT: DMA#" << dec << iIdx << " HALTED during transfer";

#if defined INLINE_DEBUG_WRITE
			debug << "\r\nhps2x64: ALERT: DMA#" << dec << iIdx << " HALTED during transfer\r\n";
#endif
					}
				}
			}
#endif

			/*
			// check if there is a transition from one to zero
			//if ( ( Data ^ 0x10000 ) & _DMA->lDMAC_ENABLE & 0x10000 )
			if ( ( Data ^ 0x10000 ) & PreviousValue & 0x10000 )
			{
				// transition from zero to one, so store and update transfers
				//_DMA->lDMAC_ENABLE = ( _DMA->lDMAC_ENABLE & ~Mask ) | ( Data );
				//_DMA->UpdateTransfer ();
			}
			//else
			//{
			//	// ???
			//	//_DMA->lDMAC_ENABLE = Data;
			//	_DMA->lDMAC_ENABLE = ( _DMA->lDMAC_ENABLE & ~Mask ) | ( Data );
			//}
			*/
			
			return;
		}


		// check for MADR
		if ( ( Address & 0xf0 ) == 0x10 )
		{
			if ( Address < ( ( DMA_CTRL & 0xffff ) - 0x8000 ) )
			{
				// MADR //

				// lookup what dma channel corresponds to the address
				DmaChannelNum = c_ucDMAChannel_LUT [ ( Address >> 10 ) & 0x1f ];

				// check for channel 8 or 9
				if ( DmaChannelNum >= 8 )
				{
					// SPR bit is fixed to zero
					pDMARegs->Regs [ Offset ] &= 0x7fffffff;
				}
				
				return;
			}

		}
		
		// check for CHCR
		if ( !( Address & 0xf0 ) )
		{
			if ( Address < ( ( DMA_CTRL & 0xffff ) - 0x8000 ) )
			{
				// CHCR //


				// lookup what dma channel corresponds to the address
				DmaChannelNum = c_ucDMAChannel_LUT [ ( Address >> 10 ) & 0x1f ];


				// check if dma channel is running already
				if (PreviousValue & 0x100)
				{
#if defined INLINE_DEBUG_WRITE
					debug << "\r\nWrite to DMA#" << dec << DmaChannelNum << " CHCR while CHCR alread enabled = " << hex << PreviousValue;
#endif

					//cout << "\nWRITE to dma channel#" << hex << DmaChannelNum << " while running previous value:" << hex << PreviousValue << " new value:" << pDMARegs->Regs[Offset];
					// check if dma has been halted/stopped
					if (!(pDMARegs->ENABLER & 0x10000))
					{
						// ps2 dma NOT halted/stopped //
						
						//cout << "\nps2 dma not halted";

#if defined INLINE_DEBUG_WRITE
						debug << "\r\nWrite to DMA#" << dec << DmaChannelNum << " CHCR while DMA is NOT halted ENABLER/W.";
#endif

						// should only be able to modify CHCR while running if dma is halted/stopped ??
						// only modify chcr
						pDMARegs->Regs[Offset] = (pDMARegs->Regs[Offset] & 0x100) | (PreviousValue & ~0x100);

						return;
					}
				}


				// some values in register are fixed for DIR field for some channels
				pDMARegs->Regs [ Offset ] &= c_ulChcrAND [ DmaChannelNum ];
				pDMARegs->Regs [ Offset ] |= c_ulChcrOR [ DmaChannelNum ];

				if ( pDMARegs->CTRL.DMAE
#ifdef ENABLE_PCE_CHANNEL_DISABLE
					&& ( !pDMARegs->PCR.PCE || ( pDMARegs->PCR.Value & ( 1 << ( DmaChannelNum + 16 ) ) ) )
#endif
					&& pRegData [ DmaChannelNum ]->CHCR.STR )
				{
					// transfer is set to start //
					
					// start transfer
					_DMA->StartTransfer ( DmaChannelNum );
					//_DMA->Transfer ( DmaChannelNum );
					_DMA->CheckTransfer ();
				}
				
				return;
			}
		}
	}
	else
	{
#if defined INLINE_DEBUG_WRITE
			debug << " " << "INVALID";
#endif

	}


	return;
	

}




void Dma::StartTransfer ( int iChannel )
{
#ifdef INLINE_DEBUG_START
	debug << "\r\nDma::StartTransfer; Channel#" << dec << iChannel;
#endif

	u64 ullPs1Cycle;


	// check if dma transfers are disabled
	if ( ! ( DMARegs.CTRL.DMAE ) )
	{
		cout << "\n***hps2x64: DMA: ALERT: Channel#" << dec << iChannel << " TransferStart while ps2 dma tranfers are DISABLED***\n";
	}

	// check if channel is already running and was simply suspended


	// check for channel end conditions //

	// check type of transfer
	switch ( pRegData [ iChannel ]->CHCR.MOD )
	{
		// NORMAL
		case 0:

		// INTERLEAVE
		case 2:
#ifdef INLINE_DEBUG_START
	debug << " NEW-NONCHAIN-TRANSFER";
#endif

			// new transfer
			// clear bits 4-7 and 9-31
			// note: looks like this should be left alone for normal/interleave transfer
			//pRegData [ iChannel ]->CHCR.Value &= 0x10f;
			break;

		// CHAIN
		case 1:
			// check that QWC is zero
			if ( ! pRegData [ iChannel ]->QWC.QWC )
			{
				// check if this is a new transfer
				switch ( pRegData [ iChannel ]->CHCR.ID )
				{
					// REFE
					case 0:

					// END
					case 7:
#ifdef INLINE_DEBUG_START
	debug << " NEW-CHAIN-TRANSFER";
#endif

						// new transfer
						pRegData [ iChannel ]->CHCR.Value &= 0x1cf;
						break;

					// CALL
					case 5:
						if ( pRegData [ iChannel ]->CHCR.ASP & 0x2 )
						{
#ifdef INLINE_DEBUG_START
	debug << " NEW-CHAIN-TRANSFER";
#endif

							// new transfer (for now)
							pRegData [ iChannel ]->CHCR.Value &= 0x1cf;
						}
						break;

					// RET
					case 6:
						if ( !pRegData [ iChannel ]->CHCR.ASP )
						{
#ifdef INLINE_DEBUG_START
	debug << " NEW-CHAIN-TRANSFER";
#endif

							// new transfer (for now)
							pRegData [ iChannel ]->CHCR.Value &= 0x1cf;
						}
						break;
				} // end switch ( pRegData [ iChannel ]->CHCR.ID )

			}
			
			break;

	} // end switch ( pRegData [ iChannel ]->CHCR.MOD )


	// start/continue transfer //

	// clear previous tag in CHCR
	// must write tag back anyway when writing to CHCR
	//DmaCh [ iChannel ].CHCR_Reg.Value &= 0xffff;
	
	// reset the QWC that has been transferred in current block
	// -1 means that block still needs to be started
	QWC_Transferred [ iChannel ] = -1;
	
	// set the cycle# that transfer was started at
	//DmaCh [ iChannel ].StartCycle = *_DebugCycleCount;
	
	// clear CISx ??
	//STAT_Reg.Value &= ~( 1 << iChannel );
	

	switch ( iChannel )
	{
		// SIF0
		case 5:

#ifdef ALIGN_DMA5_START_WITH_PS1
			// start transfer when ps1 is ready
			ullPs1Cycle = R3000A::Cpu::_CPU->CycleCount << 2;
			if ( ullPs1Cycle > *_DebugCycleCount )
			{
				DmaCh [ 5 ].ullStartCycle = ullPs1Cycle;
			}
#endif
			
			// starting dma#5 sets bit 13 (0x2000) in SBUS CTRL register
			//DataBus::_BUS->lSBUS_F240 |= 0x2000;
			*_SBUS_F240 |= 0x2000;
			
			// a condition has changed, so need to update active channel on ps1 side
			SIF::Update_ActiveChannel ();
			
			break;
			
		// SIF1
		case 6:

#ifdef ALIGN_DMA6_START_WITH_PS1
			// start transfer when ps1 is ready
			ullPs1Cycle = R3000A::Cpu::_CPU->CycleCount << 2;
			if ( ullPs1Cycle > *_DebugCycleCount )
			{
				DmaCh [ 6 ].ullStartCycle = ullPs1Cycle;
			}
#endif

			// starting dma#6 sets bit 14 (0x4000) in SBUS CTRL register
			//DataBus::_BUS->lSBUS_F240 |= 0x4000;
#ifdef ENABLE_NEW_SIF
			if ( ! ( *_SBUS_F240 & 0x2000 ) )
#endif
			{
				*_SBUS_F240 |= 0x4000;
			}

			// a condition has changed, so need to update active channel on ps1 side
			SIF::Update_ActiveChannel ();
			
			break;
	}
}



void Dma::SuspendTransfer ( int iChannel )
{
#ifdef INLINE_DEBUG_END
	debug << "\r\nDma::SuspendTransfer; Channel#" << dec << iChannel;
	debug << "; (before) CHCR=" << hex << pRegData [ iChannel ]->CHCR.Value << " STAT=" << DMARegs.STAT.Value;
#endif

	// clear STR for channel
	pRegData [ iChannel ]->CHCR.STR = 0;
}



void Dma::CheckTransfer ()
{
#ifdef INLINE_DEBUG_CHECK
	debug << "\r\nDma::CheckTransfer";
	//debug << "; (before) STAT=" << DMARegs.STAT.Value;
#endif

	u64 ullNextFreeCycle_DMA;
	int iNextActiveChannel;
	
	ullNextFreeCycle_DMA = DataBus::_BUS->GetNextFreeCycle_DMA ();

#ifdef INLINE_DEBUG_CHECK
	debug << " NFDC=" << dec << ullNextFreeCycle_DMA;
#endif

	iNextActiveChannel = GetNextActiveChannel ();


	//if ( NextEvent_Cycle == -1ULL )
	//{
	//	NextEvent_Cycle = ( *_DebugCycleCount ) + 1;
	//}
	
	
	
	if ( iNextActiveChannel != -1 )
	{
		if ( ( *_DebugCycleCount ) >= ullNextFreeCycle_DMA )
		{
			//NextEvent_Cycle = ( *_DebugCycleCount ) + 1;
			SetNextEventCh ( 1, iNextActiveChannel );
		}
		else
		{
			//NextEvent_Cycle = ullNextFreeCycle_DMA;
			SetNextEventCh_Cycle ( ullNextFreeCycle_DMA, iNextActiveChannel );
		}
		
		//if ( NextEvent_Cycle < *_NextSystemEvent )
		//{
		//	*_NextSystemEvent = NextEvent_Cycle;
		//	*_NextEventIdx = NextEvent_Idx;
		//}
	}
}


void Dma::EndTransfer ( int iChannel, bool SuppressEventUpdate )
{
#ifdef INLINE_DEBUG_END
	debug << "\r\nDma::EndTransfer; Channel#" << dec << iChannel;
	debug << "; (before) CHCR=" << hex << pRegData [ iChannel ]->CHCR.Value << " STAT=" << DMARegs.STAT.Value;
#endif

	int iDrainChannel;

	// channel is done with transfer //
	
	// clear STR for channel
	pRegData [ iChannel ]->CHCR.STR = 0;
	
	// set CIS for channel - (STAT register)
	DMARegs.STAT.Value |= ( 1 << iChannel );
	
	// check for interrupt
	UpdateInterrupt ();
	
	// update CPCOND0
	Update_CPCOND0 ();

#ifdef CLEAR_QWC_ON_COMPLETE
	pRegData [ iChannel ]->QWC.Value = 0;
#endif



	switch ( iChannel )
	{
		case 0:
			
			// transfer is done, clear buffer for now
			VU::_VU[ 0 ]->VifRegs.STAT.FQC = 0;
			
			// reset the current command ??
			VU::_VU[ 0 ]->lVifCodeState = 0;
			
			
			break;
			
		case 1:
			// for dma#1, if transfer ends then un-mask path3 transfers ??
			//GPU::_GPU->GIFRegs.STAT.M3R = 0;
			GPU::_GPU->GIFRegs.STAT.M3P = 0;
			
			// if done, then no longer interrupting path 3 obviously?
			GPU::_GPU->GIFRegs.STAT.IP3 = 0;
			
			// transfer is done, clear buffer for now
			VU::_VU[ 1 ]->VifRegs.STAT.FQC = 0;
			
			// reset the current command ??
			VU::_VU[ 1 ]->lVifCodeState = 0;

			// *** testing *** clear dbf??
			//VU::_VU[ 1 ]->VifRegs.STAT.DBF = 0;

			if ( VU::_VU[ 1 ]->ulThreadCount )
			{
				VU::_VU[ 1 ]->CopyResetTo_CommandBuffer ();
			}
			
			// if not transferring anything, then definitely not using path 2
			/*
			if ( !VU::_VU[ 1 ]->lVifCodeState )
			{
				if ( GPU::_GPU->GIFRegs.STAT.APATH == 2 )
				{
					GPU::_GPU->GIFRegs.STAT.APATH = 0;
					
					// path 3 obviously not being interrupted if path 2 not running
					GPU::_GPU->GIFRegs.STAT.IP3 = 0;
				}
			}
			*/
			
			break;
			
			
		case 2:
		
#ifdef INLINE_DEBUG_END
	debug << " *PS2CH2END* ";
#endif

			// transfer is done, clear buffer for now
			GPU::_GPU->GIFRegs.STAT.FQC = 0;
			
			// if path 3, then set to idle for now
			//if ( GPU::_GPU->GIFRegs.STAT.APATH == 3 )
			if ((GPU::_GPU->GIFRegs.STAT.APATH == 3) && (GPU::_GPU->EndOfPacket[3]))
			{
				GPU::_GPU->GIFRegs.STAT.APATH = 0;
			}

			// path 3 obviously not being interrupted if not running
			GPU::_GPU->GIFRegs.STAT.IP3 = 0;

			// path3 no longer in queue ?
			GPU::_GPU->GIFRegs.STAT.P3Q = 0;
			
			if ( VU::_VU[ 1 ]->VifStopped )
			{
				// clear vif stop because condition may have cleared
				VU::_VU[ 1 ]->VifStopped = 0;
				
				// check transfer now that potential condition for dma#1 has cleared
				//Dma::_DMA->CheckTransfer ();
			}
			
			// if dma#2 completes, then restart dma#1 if it is active
			// this is because GPU path 3 (dma#2) can stop dma#1 for now if it is currently transferring a packet
			//Transfer ( 1 );
			break;
			
		// SIF0
		case 5:
			// clear bus direction for SIF?
			SIF::_SIF->ulBufferDirection = SIF::BUFFER_NONE;

			// a condition has changed, so need to update active channel on ps1 side
			SIF::Update_ActiveChannel ();
			

#ifdef ENABLE_DMA_END_REMOTE
			// ***TESTING*** suspend channel#9 on IOP
			SIF::_SIF->SuspendTransfer_IOP ( 9 );
#endif
			
			// ending dma#5 clears bits 5 (0x20) and 13 (0x2000) in SBUS CTRL register
#ifdef ENABLE_SIF_DMA_SYNC
			*_SBUS_F240 &= ~0x2000;
#else
			//DataBus::_BUS->lSBUS_F240 &= ~0x2020;
		
			*_SBUS_F240 &= ~0x2000;
			
			// check if IOP dma #9 is done also ?
			if ( !SIF::_SIF->IOP_DMA_Out_Ready () )
			{
#ifdef INLINE_DEBUG_END
	debug << " CLEAR_F240_DMA5_END";
#endif

				*_SBUS_F240 &= ~0x2020;
			}
#ifdef ENABLE_NEW_SIF
			else
			{
			
				*_SBUS_F240 &= ~0x2000;
			}
#endif

#endif
			
			break;
			
		// SIF1
		case 6:
			// clear bus direction for SIF?
			// note: try this on the receiving end only for now
			//SIF::_SIF->ulBufferDirection = SIF::BUFFER_NONE;

			// a condition has changed, so need to update active channel on ps1 side
			SIF::Update_ActiveChannel ();
			
		
#ifdef ENABLE_DMA_END_REMOTE
			// ***TESTING*** suspend channel#10 on IOP
			//SIF::_SIF->SuspendTransfer_IOP ( 10 );
#endif
		
			// ending dma#6 clears bits 6 (0x40) and 14 (0x4000) in SBUS CTRL register
#ifdef ENABLE_SIF_DMA_SYNC
			*_SBUS_F240 &= ~0x4000;
#else
			//DataBus::_BUS->lSBUS_F240 &= ~0x4040;
			
			*_SBUS_F240 &= ~0x4000;
			
			// check if IOP dma #10 is done also?
			if ( !SIF::_SIF->IOP_DMA_In_Ready () )
			{
#ifdef INLINE_DEBUG_END
	debug << " CLEAR_F240_DMA6_END";
#endif

				*_SBUS_F240 &= ~0x4040;
			}
#ifdef ENABLE_NEW_SIF
			else
			{
			
				*_SBUS_F240 &= ~0x4000;
			}
#endif

#endif
		
			break;
	}



	// set the next channel to transfer next
	Dma::_DMA->CheckTransfer();


	if ( !SuppressEventUpdate )
	{
		Update_NextEventCycle ();
	}
	
	// as long as this is not dma#5 that is ending (could happen at anytime), restart any pending dma channels
	//UpdateTransfer ();
	// get the next transfer started
	
	// instead of resuming a dma channel, will for now run the Dma::Run function later
	SetNextEventCh ( 8, iChannel );




	
#ifdef ENABLE_SIF_DMA_TIMING
	// if channel#5, then check if channel#6 is ready to go since it would have been held up
	if( iChannel == 5 )
	{
		SIF::_SIF->Check_TransferToIOP ();
	}
#endif
	
#ifdef INLINE_DEBUG_END
	debug << "; (after) CHCR=" << hex << pRegData [ iChannel ]->CHCR.Value << " STAT=" << DMARegs.STAT.Value;
	//debug << " P3Q=" << _GPU->GIFRegs.STAT.P3Q;
#endif
}


// need to call this whenever updating interrupt related registers
void Dma::UpdateInterrupt ()
{
#ifdef INLINE_DEBUG_INT
	debug << "; Dma::UpdateInterrupt";
#endif

	// check for interrupt
	if ( DMARegs.STAT.Value & ( DMARegs.STAT.Value >> 16 ) & 0x63ff )
	{
#ifdef INLINE_DEBUG_INT
	debug << "; INT";
#endif

		// interrupt (SET INT1 on R5900)
		SetInterrupt ();
	}
	else
	{
#ifdef INLINE_DEBUG_INT
	debug << "; CLEAR_INT";
#endif

		// this should clear the interrupt on INT1 on R5900
		ClearInterrupt ();
	}
	
}


// this needs to be called whenever PCR gets changed or STAT gets changed for any reason
void Dma::Update_CPCOND0 ()
{
	// update CPCOND0
	if ( ( ( DMARegs.STAT.Value | ~DMARegs.PCR.Value ) & 0x3ff ) == 0x3ff )
	{
		// CPCOND0 = 1
		*_CPCOND0_Out = 1;
	}
	else
	{
		// CPCOND0 = 0
		*_CPCOND0_Out = 0;
	}
}


// should return 1 if transfer is complete on PS2 side, zero otherwise
// returns the number of quadwords transferred
u32 Dma::DMA5_WriteBlock ( u64* Data64, u32 QWC_Count )
{
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "\r\nDMA5_WriteBlock: DMA5: SIF0 IOP->EE";
	//debug << " PS1Cycle#" << dec << *R3000A::Cpu::_DebugCycleCount;
	debug << " R3000ACycle#" << dec << R3000A::Cpu::_CPU->CycleCount << " x4#" << (R3000A::Cpu::_CPU->CycleCount<<2);
	debug << " PS2Cycle#" << dec << *_DebugCycleCount;
	//debug << " R5900Cycle#" << dec << R5900::Cpu::_CPU->CycleCount;
	debug << " QWC=" << dec << QWC_Count;
#endif

	// dma transfer has been started //
	
	u32 Temp;
	//u32 Data [ 4 ];
	//u32 NumberOfTransfers;
	
	u32 TransferCount;
	u32 QWC_Remaining;
	
	u32 Data0, Data1, DestAddress, IRQ, ID;
	
	u32 TotalTransferred;
	
	//DMATag EETag;
	//EETag.Value = EEDMATag;
	
	u64 *DstPtr64;
	//u64 *Data64;
	
	bool TransferInProgress = true;
	
	// this delay should be handled at the source of the transfer
	static u64 CycleDelay = 0;

	
	// set pointer into data
	//Data64 = (u64*) & ( pMemory [ Address >> 2 ] );


	// check if ps2 dma transfers are enabled
	if ( _DMA->DMARegs.ENABLER & 0x10000 )
	{
		cout << "\n***hps2x64: DMA: ALERT: DMA5 incoming while ps2 dma tranfers are DISABLED***\n";
	}

	
	// check if dma transfers are disabled
	if ( ! ( _DMA->DMARegs.CTRL.Value & 1 ) )
	{
		cout << "\n***hps2x64: DMA: ALERT: DMA5 incoming while ps2 dma tranfers are DISABLED***\n";
	}


	/*
	if ( CycleDelay > *_DebugCycleCount )
	{
		CycleDelay += c_llSIFDelayPerQWC * (u64) Count;
	}
	else
	{
		CycleDelay = *_DebugCycleCount + (u64) ( c_llSIFDelayPerQWC * Count );
	}
	*/

	//Data0 = *Data++;
	//Data1 = *Data++;
	

	// rest of quadword is ignored
	//Data++;
	//Data++;
	

//#ifdef INLINE_DEBUG_RUN_DMA5
//#endif


	// set buffer direction
	Playstation2::SIF::_SIF->ulBufferDirection = Playstation2::SIF::BUFFER_SIF0_IOP_TO_EE;



	// just out of curiosity, check if tag transfer is enabled for IOP->EE DMA#5 transfer
	if ( pRegData [ 5 ]->CHCR.TTE )
	{
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "\r\nhps2x64: DMA: ALERT: TagTransfer enabled for DMA#5 (IOP->EE) transfer!";
#endif

#ifdef VERBOSE_CHAIN
		cout << "\nhps2x64: DMA: ALERT: TagTransfer enabled for DMA#5 (IOP->EE) transfer!";
#endif
	}
	
	// its going to end up transferring all the sent data into memory
	TotalTransferred = QWC_Count;

	// transfer all the data that was sent
	//while ( TransferInProgress )
	while ( QWC_Count )
	{
		// if in destination chain mode, then pull tag and set address first
		if ( pRegData [ 5 ]->CHCR.MOD == 1 )
		{

			//if ( !DmaCh [ 5 ].QWCRemaining )
			if ( !pRegData [ 5 ]->QWC.QWC )
			{
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "; QWCRemaining=0 -> Getting TAG";
	//debug << "; EETag=" << hex << EEDMATag;
#endif

				// get the IOP tag
				_DMA->IOPDMATag [ 5 ].Value = Data64 [ 0 ];

				// set the tag
				//SourceDMATag [ 5 ].Value = EEDMATag;
				_DMA->SourceDMATag [ 5 ].Value = Data64 [ 1 ];
				
				// subtract from count of data sent
				// the QWC in TAG does not include the tag
				//QWC_Count--;
				
				// also set upper bits to tag upper bits in chcr
				//DmaCh [ 5 ].CHCR_Reg.Value = ( DmaCh [ 5 ].CHCR_Reg.Value & 0xffffL ) | ( SourceDMATag [ 5 ].Value & 0xffff0000L );
				pRegData [ 5 ]->CHCR.Value = ( pRegData [ 5 ]->CHCR.Value & 0xffffL ) | ( Data64 [ 1 ] & 0xffff0000L );
				
				// set MADR
				//DmaCh [ 5 ].MADR_Reg.Value = ( SourceDMATag [ 5 ].Value >> 32 );
				pRegData [ 5 ]->MADR.Value = ( Data64 [ 1 ] >> 32 );
				
				// set the QWC to transfer
				//DmaCh [ 5 ].QWCRemaining = SourceDMATag [ 5 ].QWC;
				_DMA->DmaCh [ 5 ].QWCRemaining = Data64 [ 1 ] & 0xffff;
			
				// need to set QWC
				pRegData [ 5 ]->QWC.QWC = Data64 [ 1 ] & 0xffff;
				
				
				// just read the tag, so QWC-1
				QWC_Count -= 1;
				
				
				// have not transferred anything for current tag yet
				_DMA->DmaCh [ 5 ].QWCTransferred = 0;
			
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << " IOP-TAG=" << hex << Data64[0];
	debug << " EE-TAG=" << hex << Data64[1];
	debug << " EETag.QWC=" << dec << _DMA->SourceDMATag [ 5 ].QWC;
	debug << " Tag.ID=" << _DMA->SourceDMATag [ 5 ].ID << " Tag.IRQ=" << _DMA->SourceDMATag [ 5 ].IRQ << " Tag.PCE=" << _DMA->SourceDMATag [ 5 ].PCE;
	debug << "; Tag.MADR=" << hex << pRegData [ 5 ]->MADR.Value;
#endif
			}

			// check if there is data remaining in transfer from IOP besides the EE tag
			if ( !QWC_Count )
			{
				// will need to wait for more data
				return TotalTransferred;
				//return 0;
			}

			//DstPtr64 = GetMemoryPtr ( DmaCh [ 5 ].MADR_Reg.Value + ( DmaCh [ 5 ].QWCTransferred << 4 ) );
			DstPtr64 = GetMemoryPtr ( pRegData [ 5 ]->MADR.Value );



			// check the ID
			//switch ( _DMA->SourceDMATag [ 5 ].ID )
			switch (pRegData[5]->CHCR.ID)
			{
				// ID: CNTS
			case 0:
#ifdef INLINE_DEBUG_RUN_DMA5
				debug << "; ID=CNTS";
				debug << "; NOT IMPLEMENTED";
#endif

				// need some type of message here..
				//cout << "\nhps2x64: ***ALERT***: PS2DMA#5 Destination Tag CNTS not implemented\n";
				//break;
				// fall-through

			// ID: CNT
			case 1:

				// ID: END
			case 7:

#ifdef INLINE_DEBUG_RUN_DMA5
if (pRegData[5]->CHCR.ID == 1)
{
	debug << "; ID=CNT";
}
#endif

#ifdef INLINE_DEBUG_RUN_DMA5
if (pRegData[5]->CHCR.ID == 7)
{
	debug << "; ID=END";
	//debug << "; NOT IMPLEMENTED";
}
#endif


					//TransferCount = ( QWC_Count > DmaCh [ 5 ].QWCRemaining ) ? DmaCh [ 5 ].QWCRemaining : QWC_Count;
					TransferCount = ( QWC_Count > pRegData [ 5 ]->QWC.QWC ) ? pRegData [ 5 ]->QWC.QWC : QWC_Count;

#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "\r\n***EE SIF0 (IOP->EE) Writing QWC=" << dec << TransferCount << hex << " to MADR=" << pRegData [ 5 ]->MADR.Value << " Cycle#" << dec << *_DebugCycleCount << "\r\n";
#endif

					// transfer the data after the tag
					//for ( int i = 0; i < EETag.QWC; i++ )
					for ( int i = 0; i < TransferCount; i++ )
					{
						// transfer 128-bit quadword
						*DstPtr64++ = *Data64++;
						*DstPtr64++ = *Data64++;
						//*DstPtr++ = *Data++;
						//*DstPtr++ = *Data++;
					}
					
#ifdef ENABLE_MEMORY_INVALIDATE
					DataBus::_BUS->InvalidateRange ( pRegData [ 5 ]->MADR.Value & DataBus::MainMemory_Mask, TransferCount << 2 );
#endif

					// update QWC Transferred for tag
					_DMA->DmaCh [ 5 ].QWCTransferred += TransferCount;
					QWC_Count -= TransferCount;
					
					_DMA->DmaCh [ 5 ].QWCRemaining -= TransferCount;
					
					// update MADR
					pRegData [ 5 ]->MADR.Value += ( TransferCount << 4 );
					
					// update QWC
					pRegData [ 5 ]->QWC.QWC -= TransferCount;


					// check if cnts
					if (!pRegData[5]->CHCR.ID)
					{
						// STALL CONTROL //
						// since chain transfer, first check that tag is cnts (destination tag=0)
						// if channel is a stall source channel, then copy MADR to STADR
						if ( _DMA->DMARegs.CTRL.STS == 1 )
						{
							_DMA->DMARegs.STADR = pRegData [ 5 ]->MADR.Value;
						}
					}



					// set amount of time bus will be busy for
					if ( TotalTransferred )
					{
						DataBus::_BUS->ReserveBus_DMA ( *_DebugCycleCount, TotalTransferred );
					}

					
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << " Transferred=" << dec << TransferCount;
	debug << " Remaining=" << dec << _DMA->DmaCh [ 5 ].QWCRemaining;
#endif

					// check transfer is complete
					//if ( DmaCh [ 5 ].QWCRemaining <= 0 )
					if ( !pRegData [ 5 ]->QWC.QWC )
					{
						// make sure this value is zero
						_DMA->DmaCh [ 5 ].QWCRemaining = 0;
						
						// ***TODO*** NEED TO CHECK TIE BIT ????
						// check if IRQ requested
						//if ( SourceDMATag [ 5 ].IRQ && DmaCh [ 5 ].CHCR_Reg.TIE )
						if (
							( pRegData [ 5 ]->CHCR.IRQ && pRegData [ 5 ]->CHCR.TIE )
							 ||
							 ( pRegData[5]->CHCR.ID == 7 )
						)
						{
#ifdef ENABLE_SIF_DMA_TIMING
							// actually end the transfer after data has been transferred serially
							// this delay should be handled at the source of the transfer
							SetNextEventCh_Cycle ( CycleDelay, 5 );
							
							// let the SIF know that the EE will be busy for awhile with the data just transferred
							SIF::_SIF->EE_BusyUntil ( CycleDelay );
#else
							// set CIS
							//Stat_Reg.Value |= ( 1 << 5 );
							
							// clear STR
							//DmaCh [ 5 ].CHCR_Reg.STR = 0;
							
							// end transfer
							_DMA->EndTransfer ( 5, true );
							
							// interrupt for sif ??
							//SIF::SetInterrupt_EE_SIF ();
							
							// done - and finished
							return TotalTransferred;
							//return;
#endif
						}
						
#ifdef INT_ON_DMA5_STOP
						// check if IOP was done after transfer
						if ( _DMA->IOPDMATag [ 5 ].IRQ )
						{
							// send interrupt but keep dma 5 active ??
							
							// set CIS for channel - (STAT register)
							_DMA->DMARegs.STAT.Value |= ( 1 << 5 );
							
							// check for interrupt
							_DMA->UpdateInterrupt ();
							
							// update CPCOND0
							_DMA->Update_CPCOND0 ();
							
							// send interrupt and end dma 5 ??
							//_DMA->EndTransfer ( 5, true );
							
							
							return TotalTransferred;
							//return;
						}
#endif

						
						// has not transferred new tag yet, so need to return until next transfer comes in to get the new tag
						//return;
						//return 0;
					}
					
					break;

					
				// ID: END
					/*
				case 7:
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "; ID=END";
	debug << "; NOT IMPLEMENTED";
#endif

					// need some type of message here..
					cout << "\nhps2x64: ***ALERT***: PS2DMA#5 Destination Tag END not implemented\n";
					break;
					*/
				
				
				default:
#ifdef INLINE_DEBUG_RUN_DMA5
	debug << "; DestTag=Unimplemented/Unknown";
#endif

					// need some type of message here..
					cout << "\nhps2x64: ***ALERT***: DMA Destination Tag UNKNOWN/IMPOSSIBLE not implemented\n";
					break;
			}	// end switch ( ID )
		
		}	// end if
		else
		{
			// not in destination chain mode for channel 5??
			cout << "\nhps2x64: DMA: ***ALERT***: DMA Channel#5 not in destination chain mode.\n";
		}
		
	}	// end while ( QWC_Count )
		

	
	if ( c_iDmaTransferTimePerQwc [ 5 ] )
	{
		// if transfer is not finished, then schedule transfer to continue later
		_DMA->SetNextEventCh ( ( c_iDmaTransferTimePerQwc [ 5 ] * TotalTransferred ) + c_iSetupTime, 5 );
		
#ifdef ENABLE_PS1_SYNC_DMA5
		u64 Cyclet;
		
		// note/todo: the R3000A cycle count probably isn't updated until after the transfer
		// so would probably need to account for that
		Cyclet = R3000A::Cpu::_CPU->CycleCount << 2;

		// add time to transfer data from ram to sif buffer on ps1 side
		Cyclet += TotalTransferred << 4;

		// add time to transfer from sif buffer to ram on ps2 side
		//Cyclet += c_iDmaTransferTimePerQwc [ 5 ] * TotalTransferred;
		
		if ( Cyclet > *_DebugCycleCount )
		{
			_DMA->DmaCh [ 5 ].ullStartCycle = Cyclet + 8;
			_DMA->SetNextEventCh_Cycle ( Cyclet + 8, 5 );
			
			//_DMA->iLastChannel = 5;
			//if ( _DMA->GetNextActiveChannel () != _DMA->iLastChannel )
			//{
			//	_DMA->CheckTransfer ();
			//}
		}
#endif
		
		// continue transfer later
		//return;
		
	}	// end if ( c_iDmaTransferTimePerQwc [ 5 ] )
		
	// transfer not finished, but returning
	//return 0;
	return TotalTransferred;
}




void Dma::NormalTransfer_ToMemory ( int iChannel )
{
	u32 SrcAddress, DstAddress, TransferCount, TransferIndex;
	u64 *SrcDataPtr, *DstDataPtr;
	
	u32 NextTagAddress, NextDataAddress;
	
	u64 QWC_TransferCount;
	
	u32 iDrainChannel, iQWRemaining1, iQWRemaining2, TransferAddress;

	u64 ullReadyCycle;
	
	static const u64 c_LoopTimeout = 33554432;
	u64 LoopCount;
	
	bool TransferInProgress = true;
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << "; Normal";
#endif

#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << "; ToMemory";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " SADR=" << hex << pRegData [ iChannel ]->SADR.Value;
	debug << " QWC=" << hex << pRegData [ iChannel ]->QWC.Value;
#endif

	for ( LoopCount = 0; LoopCount < c_LoopTimeout; LoopCount++ )
	{
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << " @Cycle#" << dec << *_DebugCycleCount;
#endif

		// check if transfer of block has started yet
		if ( QWC_Transferred [ iChannel ] < 0 )
		{
			// set the amount total in the block to be transferred
			QWC_BlockTotal [ iChannel ] = pRegData [ iChannel ]->QWC.QWC;
			
			// if qwc is zero, then transfer 0x10000
			if (!pRegData[iChannel]->QWC.QWC)
			{
				QWC_BlockTotal[iChannel] = 0x10000;
			}

			// nothing transferred yet
			QWC_Transferred [ iChannel ] = 0;
		}

		
		// check if channel has a ready function to check if its ready
		// if not, then skip
		if ( cbReady_ToMemory [ iChannel ] )
		{
			ullReadyCycle = cbReady_ToMemory[iChannel]();

			// check that channel is ready
			if ( !ullReadyCycle )
			{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << "; ChannelNOTReady";
#endif

				return;
			}
			else if (ullReadyCycle > *_DebugCycleCount)
			{
				// channel will be ready in the future //
				SetNextEventCh_Cycle(ullReadyCycle, iChannel);

				return;
			}
		}
		
		
		// check if channel has a transfer function
		// if not, then unable to transfer
		if ( !cbTransfer_ToMemory [ iChannel ] )
		{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << "; ***TransferNOTImplemented***";
#endif

#ifdef DEBUG_COUT
			cout << "\nhps2x64: ALERT: PS2 DMA Normal Transfer not implemented. Channel=" << dec << iChannel << "\n";
#endif

			return;
		}
		
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << "; TransferingData";
#endif


		// transfer all data at once for now
		
		// get pointer to source data
		//SrcDataPtr = & DataBus::_BUS->MainMemory.b64 [ ( ( DmaCh [ iChannel ].MADR_Reg.Value & DataBus::MainMemory_Mask ) & ~0xf ) >> 3 ];
		SrcDataPtr = GetMemoryPtr ( pRegData [ iChannel ]->MADR.Value );
		
		// get pointer to where transfer last left off from
		// not sure if dma has an internal counter... will tak a look at this later
		//SrcDataPtr = & ( SrcDataPtr [ QWC_Transferred [ iChannel ] << 1 ] );

		// the amount of data to attemp to transfer depends on the buffer size for the device

#ifdef TEST_ASYNC_DMA
		QWC_TransferCount = ( c_iDeviceBufferSize [ iChannel ] < ( QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] ) ) ? c_iDeviceBufferSize [ iChannel ] : ( QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] );
		
		// STALL CONTROL //
		// check if channel is a stall destination
		// note: can't be a stall drain channel if transferring TO memory, only if transferring FROM memory
		/*
		if ( iChannel == c_iStallDest_LUT [ DMARegs.CTRL.STD ] )
		{
			if ( ( pRegData [ iChannel ]->MADR.Value + 8 ) > DMARegs.STADR )
			{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << " *STALL*";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " STADR=" << DMARegs.STADR;
#endif

				// there is no data to transfer for stall destination
				
				// set stall interrupt
				DMARegs.STAT.SIS = 1;
				
				// update interrupts
				UpdateInterrupt ();
				
				// don't do this for stall interrupt
				//Update_CPCOND0 ();
				
				return;
			}
		}
		*/
		
		// only transfer if there is data to transfer
		if ( QWC_TransferCount )
		{
			if ( ( DMARegs.CTRL.MFIFO < 2 ) || ( iChannel != 8 ) )
			{
				QWC_TransferCount = cbTransfer_ToMemory [ iChannel ] ( SrcDataPtr, QWC_TransferCount );
				
#ifdef ENABLE_MEMORY_INVALIDATE
				if ( ! ( pRegData [ iChannel ]->MADR.Value & 0x80000000 ) )
				{
					DataBus::_BUS->InvalidateRange ( pRegData [ iChannel ]->MADR.Value & DataBus::MainMemory_Mask, QWC_TransferCount << 2 );
				}
#endif

			}
			else
			{
				// source channel //

				// get drain channel
				iDrainChannel = DMARegs.CTRL.MFIFO - 1;

#ifdef VERBOSE_MFIFO_INVALID
				if (!iDrainChannel)
				{
					// alert if mfifo is set to an invalid value
					cout << "\nhps2x64: ALERT: DMA: MFIFO=1!!!\n";
				}
#endif

#ifdef ONLY_TRANSFER_MFIFO_SOURCE_ON_EQUAL
				// make sure that dma#8 MADR is equal to drain channel TADR before transfer ??
				if (pRegData[iDrainChannel]->CHCR.STR)
				{
					if ((pRegData[8]->MADR.Value & DMARegs.RBSR.Value) != (pRegData[iDrainChannel]->TADR.Value & DMARegs.RBSR.Value))
					{
						// only transfer when they are equal
						return;
					}
				}
#endif


				// the amount of data that can be written to mfifo is difference between the MADRs minus the size of mfifo
				// note: probably supposed to compare with tadr from drain channel instead ??
				iQWRemaining2 = ( ( DMARegs.RBSR.Value + ( 1 << 4 ) ) - ( pRegData [ 8 ]->MADR.Value - pRegData [ iDrainChannel ]->TADR.Value ) ) >> 4;
				
				// then get the amount of data to write before you wraparound
				iQWRemaining1 = ( ( ( pRegData [ 8 ]->MADR.Value | DMARegs.RBSR.Value ) + ( 1 << 4 ) ) - pRegData [ 8 ]->MADR.Value ) >> 4;
				
				// check if amount that wants to be transfered is larger than amount that can be transferred
				iQWRemaining2 = ( ( QWC_TransferCount <= iQWRemaining2 ) ? QWC_TransferCount : iQWRemaining2 );
				
				// check if the amount to transfer before wrap around is greater then total amount to transfer
				iQWRemaining1 = ( ( iQWRemaining1 > iQWRemaining2 ) ? iQWRemaining2 : iQWRemaining1 );
				
				// get address MADR
				TransferAddress = pRegData [ iChannel ]->MADR.Value;
				
				// initialize amount of data transferred
				//QWC_TransferCount = 0;
				
				if ( iQWRemaining1 )
				{
					// get the data pointer
					SrcDataPtr = GetMemoryPtr ( ( TransferAddress & DMARegs.RBSR.Value ) | DMARegs.RBOR );
					
					// transfer the first run
					//QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, iQWRemaining1 );
					QWC_TransferCount = cbTransfer_ToMemory [ iChannel ] ( SrcDataPtr, iQWRemaining1 );
					
#ifdef ENABLE_MEMORY_INVALIDATE
					DataBus::_BUS->InvalidateRange ( pRegData [ iChannel ]->MADR.Value & DataBus::MainMemory_Mask, QWC_TransferCount << 2 );
#endif

					// update transfer address
					TransferAddress += ( QWC_TransferCount << 4 );
					
					// remove from the total amount to transfer
					iQWRemaining2 -= iQWRemaining1;
					
					// if all the data was transferred, then transfer the wraparound also
					// note: can do this here since iQWRemaining1 is only zero when there is nothing to tranfer at all
					if ( iQWRemaining2 && ( QWC_TransferCount == iQWRemaining1 ) )
					{
						// get the data pointer
						SrcDataPtr = GetMemoryPtr ( ( TransferAddress & DMARegs.RBSR.Value ) | DMARegs.RBOR );
						
						// channel 8 needs MADR updated before transfer for invalidate
						pRegData [ iChannel ]->MADR.Value = ( TransferAddress & DMARegs.RBSR.Value ) | DMARegs.RBOR;
						TransferAddress -= ( QWC_TransferCount << 4 );
						
						// transfer the second run (wraparound)
						//QWC_TransferCount += cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, iQWRemaining2 );
						QWC_TransferCount += cbTransfer_ToMemory [ iChannel ] ( SrcDataPtr, iQWRemaining2 );

#ifdef ENABLE_MEMORY_INVALIDATE
						DataBus::_BUS->InvalidateRange ( pRegData [ iChannel ]->MADR.Value & DataBus::MainMemory_Mask, iQWRemaining2 << 2 );
#endif

						// restore MADR after transfer
						pRegData [ 8 ]->MADR.Value = TransferAddress;
						
						// note: transfer address gets updated below
					}
					
				}
				
			}
		}
#else
		// perform transfer
		QWC_TransferCount = cbTransfer_ToMemory [ iChannel ] ( SrcDataPtr, QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] );
#endif
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
		debug << " QWC_TransferCount=" << dec << QWC_TransferCount;
		debug << " Transferred=" << dec << QWC_Transferred [ iChannel ];
		debug << " Total=" << dec << QWC_BlockTotal [ iChannel ];
#endif

		// update address for next data to transfer
		//DmaCh [ iChannel ].MADR_Reg.Value += ( (u32) DmaCh [ iChannel ].QWC_Reg.QWC ) << 4;
		pRegData [ iChannel ]->MADR.Value += QWC_TransferCount << 4;
		
		// STALL CONTROL //
		// if channel is a stall source channel, then copy MADR to STADR
		if ( iChannel == c_iStallSource_LUT [ DMARegs.CTRL.STS ] )
		{
			DMARegs.STADR = pRegData [ iChannel ]->MADR.Value;
			
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_TOMEM
	debug << " *STALLCTRL*";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " STADR=" << DMARegs.STADR;
#endif
		}
		
		// does update MADR, but also updates QWC on slice
#ifdef UPDATE_QWC_ONLYONSLICE
		if ( iChannel < 8 )
		{
#endif
			// non-spr dma channel //
			
			// update QWC after transfer of block
			pRegData [ iChannel ]->QWC.QWC -= QWC_TransferCount;
			
#ifdef UPDATE_QWC_ONLYONSLICE
		}
#endif
		
		// update QWC transferred so far
		QWC_Transferred [ iChannel ] += QWC_TransferCount;
		
		
		// ***todo*** update time that bus will be in use
		// set amount of time bus will be busy for
		if ( QWC_TransferCount )
		{
			DataBus::_BUS->ReserveBus_DMA ( *_DebugCycleCount, QWC_TransferCount );
		}

		
		// check if all data in the block has transferred
		if ( QWC_Transferred [ iChannel ] >= QWC_BlockTotal [ iChannel ] )
		{
			// all the data has been transferred in block, since this is not a chain transfer we are done
			EndTransfer ( iChannel );
			
			// transfer complete
			QWC_Transferred [ iChannel ] = -1;
			return;
		}
#ifdef TEST_ASYNC_DMA_STAGE2
		else if ( c_iDmaTransferTimePerQwc [ iChannel ] )
		{
			// if transfer is not finished, then schedule transfer to continue later
			SetNextEventCh ( ( c_iDmaTransferTimePerQwc [ iChannel ] * QWC_TransferCount ) + c_iSetupTime, iChannel );
			
			// continue transfer later
			return;
		}
#endif

	} // for LoopCount
	
	// if code ever reaches here, that means there was a timeout
	cout << "\nhps2x64 ERROR: Normal DMA Transfer to Channel#" << iChannel << " TIMED OUT";
}

void Dma::NormalTransfer_FromMemory ( int iChannel )
{
	//u64 Data0, Data1;
	//DMATag SrcDtag;
	//DMATag DstDtag;
	
	u32 SrcAddress, DstAddress, TransferCount, TransferIndex;
	u64 *SrcDataPtr, *DstDataPtr;
	
	u32 NextTagAddress, NextDataAddress;

	u64 ullReadyCycle;
	
	u64 QWC_TransferCount;
	
	static const u64 c_LoopTimeout = 33554432;
	u64 LoopCount;
	
	bool TransferInProgress = true;
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << "; Normal";
#endif

#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << "; FromMemory";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
#endif

	for ( LoopCount = 0; LoopCount < c_LoopTimeout; LoopCount++ )
	{
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << " @Cycle#" << dec << *_DebugCycleCount;
#endif

		// check if transfer of block has started yet
		if ( QWC_Transferred [ iChannel ] < 0 )
		{
			// set the amount total in the block to be transferred
			QWC_BlockTotal [ iChannel ] = pRegData [ iChannel ]->QWC.QWC;

			// if qwc is zero, then transfer 0x10000
			if (!pRegData[iChannel]->QWC.QWC)
			{
				QWC_BlockTotal[iChannel] = 0x10000;
			}
			
			// nothing transferred yet
			QWC_Transferred [ iChannel ] = 0;
		}

		
		// check if channel has a ready function to check if its ready
		// if not, then skip
		if ( cbReady [ iChannel ] )
		{
			ullReadyCycle = cbReady[iChannel]();

			// check that channel is ready
			if ( !( cbReady [ iChannel ] () ) )
			{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << "; ChannelNOTReady";
#endif

				return;
			}
			else if (ullReadyCycle > *_DebugCycleCount)
			{
				// channel will be ready in the future //
				SetNextEventCh_Cycle(ullReadyCycle, iChannel);

				return;
			}
		}
		
		
		// check if channel has a transfer function
		// if not, then unable to transfer
		if ( !cbTransfer_FromMemory [ iChannel ] )
		{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << "; ***TransferNOTImplemented***";
#endif

#ifdef DEBUG_COUT
			cout << "\nhps2x64: ALERT: PS2 DMA Normal Transfer not implemented. Channel=" << dec << iChannel << "\n";
#endif

			return;
		}
		
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << "; TransferingData";
#endif


		// transfer all data at once for now
		
		// get pointer to source data
		//SrcDataPtr = & DataBus::_BUS->MainMemory.b64 [ ( ( DmaCh [ iChannel ].MADR_Reg.Value & DataBus::MainMemory_Mask ) & ~0xf ) >> 3 ];
		SrcDataPtr = GetMemoryPtr ( pRegData [ iChannel ]->MADR.Value );
		
		// get pointer to where transfer last left off from
		// not sure if dma has an internal counter... will tak a look at this later
		//SrcDataPtr = & ( SrcDataPtr [ QWC_Transferred [ iChannel ] << 1 ] );

		// the amount of data to attemp to transfer depends on the buffer size for the device

#ifdef TEST_ASYNC_DMA
		QWC_TransferCount = ( c_iDeviceBufferSize [ iChannel ] < ( QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] ) ) ? c_iDeviceBufferSize [ iChannel ] : ( QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] );

		// STALL CONTROL //
		// check if channel is a stall destination
		if ( iChannel == c_iStallDest_LUT [ DMARegs.CTRL.STD ] )
		{
			// todo: supposed to transfer 8-qwords at a time and then not transfer less than 8 at a time
			//if ( ( pRegData [ iChannel ]->MADR.Value + 8 ) > DMARegs.STADR )
			//if ( ( pRegData [ iChannel ]->MADR.Value + ( 1 << 4 ) ) > DMARegs.STADR )
			if ( ( pRegData [ iChannel ]->MADR.Value + ( 8 << 4 ) ) > DMARegs.STADR )
			{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << " *STALL*";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " STADR=" << DMARegs.STADR;
#endif

				// there is no data to transfer for stall destination
				
				// set stall interrupt
				DMARegs.STAT.SIS = 1;
				
				// update interrupts
				UpdateInterrupt ();
				
				// don't do this for stall interrupt
				//Update_CPCOND0 ();
				
				// todo: only needs to return if there is no more data to transfer ??
				return;
			}
			
			// make sure that MADR + transfercount <= STADR
			if ( ( pRegData [ iChannel ]->MADR.Value + ( QWC_TransferCount << 4 ) ) > DMARegs.STADR )
			{
				QWC_TransferCount = ( DMARegs.STADR >> 4 ) - ( pRegData [ iChannel ]->MADR.Value >> 4 );
				//QWC_TransferCount = 8;
			}
		}
		
		// only transfer if there is data to transfer
		if ( QWC_TransferCount )
		{
			QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_TransferCount );
		}
#else
		// perform transfer
		QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] );
#endif
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
		debug << " QWC_TransferCount=" << hex << QWC_TransferCount;
#endif

		// update address for next data to transfer
		//DmaCh [ iChannel ].MADR_Reg.Value += ( (u32) DmaCh [ iChannel ].QWC_Reg.QWC ) << 4;
		pRegData [ iChannel ]->MADR.Value += QWC_TransferCount << 4;
		
		
		// STALL CONTROL //
		// if channel is a stall source channel, then copy MADR to STADR
		// note: can't be a stall source if transferring FROM memory, only if transferring TO memory
		/*
		if ( iChannel == c_iStallSource_LUT [ DMARegs.CTRL.STS ] )
		{
			DMARegs.STADR = pRegData [ iChannel ]->MADR.Value;
			
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_NORMAL_FROMMEM
	debug << " *STALLCTRL*";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " STADR=" << DMARegs.STADR;
#endif
		}
		*/
		
		// does update MADR, but also updates QWC on slice
#ifdef UPDATE_QWC_ONLYONSLICE
		if ( iChannel < 8 )
		{
#endif
			// non-spr dma channel //
			
			// update QWC after transfer of block
			pRegData [ iChannel ]->QWC.QWC -= QWC_TransferCount;
			
#ifdef UPDATE_QWC_ONLYONSLICE
		}
#endif
		
		// update QWC transferred so far
		QWC_Transferred [ iChannel ] += QWC_TransferCount;
		
		
		// ***todo*** update time that bus will be in use
		// set amount of time bus will be busy for
		if ( QWC_TransferCount )
		{
			DataBus::_BUS->ReserveBus_DMA ( *_DebugCycleCount, QWC_TransferCount );
		}
		
		
		// check if all data in the block has transferred
		if ( QWC_Transferred [ iChannel ] >= QWC_BlockTotal [ iChannel ] )
		{
			// all the data has been transferred in block, since this is not a chain transfer we are done
			EndTransfer ( iChannel );
			
			// transfer complete
			QWC_Transferred [ iChannel ] = -1;
			return;
		}
#ifdef TEST_ASYNC_DMA_STAGE2
		else if ( c_iDmaTransferTimePerQwc [ iChannel ] )
		{
			// if transfer is not finished, then schedule transfer to continue later
			SetNextEventCh ( ( c_iDmaTransferTimePerQwc [ iChannel ] * QWC_TransferCount ) + c_iSetupTime, iChannel );
			
			// continue transfer later
			return;
		}
#endif

	} // for LoopCount
	
	// if code ever reaches here, that means there was a timeout
	cout << "\nhps2x64 ERROR: Normal DMA Transfer to Channel#" << iChannel << " TIMED OUT";
}

void Dma::ChainTransfer_ToMemory ( int iChannel )
{
}


// should return the amount of data transferred
u64 Dma::Chain_TransferBlock ( int iChannel )
{
	u64 *TagDataPtr, *SrcDataPtr;
	u64 ullTagToTransfer [ 2 ];
	u32 QWC_TransferCount = 0;
	
	u32 iDrainChannel, iQWRemaining1, iQWRemaining2, TransferAddress;

	SrcDataPtr = GetMemoryPtr ( pRegData [ iChannel ]->MADR.Value );
	
	// check if transfer is just starting
	if ( QWC_Transferred [ iChannel ] < 0 )
	{

		
		
		{
			// tag should NOT be transferred //
			
			// if transfer is just starting but no tag transferring, then enable transfer of data
			QWC_Transferred [ iChannel ] = 0;
		}
		
//#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_3
//	debug << " QWC_Transferred=" << hex << QWC_Transferred [ iChannel ];
//	debug << " ADDR=" << SourceDMATag [ iChannel ].ADDR;
//#endif
	}


//#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN
//	debug << " SrcDataPtr=" << (u64) SrcDataPtr;
//	debug << " Ptr=" << (u64) & ( DataBus::_BUS->MainMemory.b64 [ ( SourceDMATag [ iChannel ].ADDR & DataBus::MainMemory_Mask ) >> 3 ] );
//#endif
	
	// added - update source data pointer so that it points to the current quadword to transfer next
	//SrcDataPtr = & ( SrcDataPtr [ QWC_Transferred [ iChannel ] << 1 ] );

	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_4
	debug << "\r\n***DMA#" << iChannel << " " << DmaCh_Names [ iChannel ] << " Transfering QWC=" << hex << SourceDMATag [ iChannel ].QWC;
	debug << " from ADDR=" << SourceDMATag [ iChannel ].ADDR;
	debug << " TADR=" << pRegData [ iChannel ]->TADR.Value;
	debug << " MADR=" << pRegData [ iChannel ]->MADR.Value;
	debug << " SADR=" << pRegData [ iChannel ]->SADR.Value;
	debug << " CHCR=" << pRegData [ iChannel ]->CHCR.Value;
	debug << " EETag=" << SourceDMATag [ iChannel ].Value << " IOPTag=" << *((u64*) SrcDataPtr) << "\r\n";
#endif

	//if ( QWC_BlockTotal [ iChannel ] )
	if ( pRegData [ iChannel ]->QWC.QWC )
	{
		
		// check if channel has a transfer function
		if ( cbTransfer_FromMemory [ iChannel ] )
		{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_5
	debug << "\r\nLeft(QWC)=" << dec << pRegData [ iChannel ]->QWC.QWC;
#endif


#ifdef TEST_ASYNC_DMA

			// actually need to account for cycle steal mode and dma channel
			//QWC_TransferCount = ( c_iDeviceBufferSize [ iChannel ] < ( DmaCh [ iChannel ].QWC_Reg.QWC ) ) ? c_iDeviceBufferSize [ iChannel ] : ( DmaCh [ iChannel ].QWC_Reg.QWC );
			
			// the amount to transfer in one go depends on whether Cycle Stealing is on (meaning the cpu can take over) and the dma channel
			// has the most effect on channels 8 and 9, so must handle that
			if ( ( iChannel >= 8 ) )
			{

				if ( ( !DMARegs.CTRL.CycleStealMode ) )
				{
#if defined INLINE_DEBUG_TRANSFER
	debug << " CYCLESTEAL-OFF";
#endif

				// cycle steal mode is off and dma# 8 or 9 //
				
				// transfer all at once
				QWC_TransferCount = pRegData [ iChannel ]->QWC.QWC;
				}
				else
				{
#if defined INLINE_DEBUG_TRANSFER
	debug << " CYCLESTEAL-ON";
#endif
				// cycle steal mode //

				// releases to bus every 8 qwords //
				QWC_TransferCount = ( pRegData [ iChannel ]->QWC.QWC > 8 ) ? 8 : pRegData [ iChannel ]->QWC.QWC;

				}
			}
			else
			{
				//if ( ( !DMARegs.CTRL.CycleStealMode ) )
				{
#if defined INLINE_DEBUG_TRANSFER
	debug << " CYCLESTEAL-OFF";
#endif
				// the maximum amount that can be transferred is what can fit in the device buffer (for channels 8 and 9 use 8 QWs)
				//QWC_TransferCount = ( c_iDeviceBufferSize [ iChannel ] < ( pRegData [ iChannel ]->QWC.QWC ) ) ? c_iDeviceBufferSize [ iChannel ] : ( pRegData [ iChannel ]->QWC.QWC );
				QWC_TransferCount = ( pRegData [ iChannel ]->QWC.QWC > c_iDeviceBufferSize [ iChannel ] ) ? c_iDeviceBufferSize [ iChannel ] : pRegData [ iChannel ]->QWC.QWC;
				}

				/*
				else
				{
#if defined INLINE_DEBUG_TRANSFER
	debug << " CYCLESTEAL-ON";
#endif
					// release to the bus every 8 qwords //
					QWC_TransferCount = ( pRegData [ iChannel ]->QWC.QWC > 8 ) ? 8 : pRegData [ iChannel ]->QWC.QWC;
				}
				*/
			}

			// STALL CONTROL //
			// since chain transfer, check for refs tag
			if ( pRegData [ iChannel ]->CHCR.ID == 4 )
			{
				// check if channel is a stall destination
				if ( iChannel == c_iStallDest_LUT [ DMARegs.CTRL.STD ] )
				{
					// todo: supposed to transfer 8-qwords at a time and then not transfer less than 8 at a time
					//if ( ( pRegData [ iChannel ]->MADR.Value + 8 ) > DMARegs.STADR )
					//if ( ( pRegData [ iChannel ]->MADR.Value + ( 1 << 4 ) ) > DMARegs.STADR )
					if ( ( pRegData [ iChannel ]->MADR.Value + ( 8 << 4 ) ) > DMARegs.STADR )
					{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_5
	debug << " *STALL*";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " STADR=" << DMARegs.STADR;
#endif

						// there is no data to transfer for stall destination
						
						// set stall interrupt
						DMARegs.STAT.SIS = 1;
						
						// update interrupts
						UpdateInterrupt ();
						
						// don't do this for stall interrupt
						//Update_CPCOND0 ();
						
						// todo: only needs to return if there is no more data to transfer ??
						return 0;
					}
					
					// make sure that MADR + transfercount <= STADR
					if ( ( pRegData [ iChannel ]->MADR.Value + ( QWC_TransferCount << 4 ) ) > DMARegs.STADR )
					{
						QWC_TransferCount = ( DMARegs.STADR >> 4 ) - ( pRegData [ iChannel ]->MADR.Value >> 4 );
						//QWC_TransferCount = 8;
					}
				}
			}
			
			// only transfer if there is data to transfer
			if ( QWC_TransferCount )
			{
				// get drain channel
				iDrainChannel = DMARegs.CTRL.MFIFO - 1;
				
#ifdef VERBOSE_MFIFO_INVALID
				if ( !iDrainChannel )
				{
					// alert if mfifo is set to an invalid value
					cout << "\nhps2x64: ALERT: DMA: MFIFO=1!!!\n";
				}
#endif
				
				// check if mfifo is set
				// no need to check if dma#8 since it can only be run in normal mode for mfifo
				if ( ( DMARegs.CTRL.MFIFO < 2 ) || ( iChannel != iDrainChannel ) )
				{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; NO_MFIFO_TRANSFER";
	debug << " QWC_TransferCount=" << dec << QWC_TransferCount;
#endif

					// no mfifo //
					
					QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_TransferCount );
				}
				else
				{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; MFIFO";
	debug << " D8_MADR=" << hex << pRegData [ 8 ]->MADR.Value;
	debug << " D8_CHCR=" << hex << pRegData [ 8 ]->CHCR.Value;
	//debug << " bMfifoStarted=" << bMfifoStarted;
#endif

					// mfifo //
					
					
					// channel must be the drain channel since this is a chain transfer
					
					
					// drain channel //
					
					/*
					// if drain channel TADR is same as source channel MADR, then mfifo is empty
					// must check TADR and not MADR for drain channel or else TADR will update
					if ( pRegData [ 8 ]->MADR.Value == pRegData [ iChannel ]->TADR.Value )
					{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; MFIFO_EMPTY";
#endif

						// mfifo is empty //
						
						// set mfifo empty interrupt
						DMARegs.STAT.MEIS = 1;
						
						// update interrupts
						UpdateInterrupt ();
						
						// no data was transferred
						return 0;
					}
					*/
					
					// check if tag is cnt or end
					if ( ( pRegData [ iChannel ]->CHCR.ID == 1 )
						|| ( pRegData [ iChannel ]->CHCR.ID == 7 )
						//|| ( pRegData [ iChannel ]->CHCR.ID == 0 )
						//|| ( pRegData [ iChannel ]->CHCR.ID == 3 )
						//|| ( pRegData [ iChannel ]->CHCR.ID == 4 )
					)
					{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; MFIFO_TRANSFER";
	debug << " RBSR=" << hex << DMARegs.RBSR.Value;
	debug << " RBOR=" << hex << DMARegs.RBOR;
#endif

						// transfer data but check for mfifo wrap around
					
						// the amount of data to read from buffer is distance between source/drain madr
						// note: probably supposed to compare with tadr from drain channel instead ??
						//iQWRemaining2 = ( DmaCh [ 8 ].MADR_Reg.Value - DmaCh [ iChannel ].MADR_Reg.Value ) >> 4;
						
						// start by wrapping drain channel MADR around mfifo just in case
						pRegData [ iChannel ]->MADR.Value = Get_MfifoAddr ( pRegData [ iChannel ]->MADR.Value );
						
						// then get the amount of data to read before you wraparound
						iQWRemaining1 = ( ( ( pRegData [ iChannel ]->MADR.Value | DMARegs.RBSR.Value ) + ( 1 << 4 ) ) - pRegData [ iChannel ]->MADR.Value ) >> 4;
						
						// check if amount that wants to be transfered is larger than amount that can be transferred
						//iQWRemaining2 = ( ( QWC_TransferCount <= iQWRemaining2 ) ? QWC_TransferCount : iQWRemaining2 );
						
						// check if the amount to transfer before wrap around is greater then total amount to transfer
						//iQWRemaining1 = ( ( iQWRemaining1 > iQWRemaining2 ) ? iQWRemaining2 : iQWRemaining1 );
						iQWRemaining1 = ( ( iQWRemaining1 > QWC_TransferCount ) ? QWC_TransferCount : iQWRemaining1 );
						
						// get address MADR
						//TransferAddress = DmaCh [ iChannel ].MADR_Reg.Value;
						
						// initialize amount of data transferred
						//QWC_TransferCount = 0;
						
						if ( iQWRemaining1 )
						{
							// get the data pointer
							//SrcDataPtr = GetMemoryPtr ( ( TransferAddress & RBSR_Reg.Value ) | RBOR_Reg );
							SrcDataPtr = GetMemoryPtr ( pRegData [ iChannel ]->MADR.Value );
							
							// transfer the first run
							iQWRemaining2 = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, iQWRemaining1 );
							
							// update transfer address
							pRegData [ iChannel ]->MADR.Value += ( iQWRemaining2 << 4 );
							
							// update QWC
							pRegData [ iChannel ]->QWC.QWC -= iQWRemaining2;
							
							// wrap drain MADR around mfifo
							pRegData [ iChannel ]->MADR.Value = Get_MfifoAddr ( pRegData [ iChannel ]->MADR.Value );
							
							// update amount remaining to be transferred for now
							//QWC_TransferCount -= iQWRemaining2;
							
							// remove from the total amount to transfer
							//iQWRemaining2 -= iQWRemaining1;
							
							// if all the data was transferred, then transfer the wraparound also
							// note: can do this here since iQWRemaining1 is only zero when there is nothing to tranfer at all
							//if ( iQWRemaining2 && ( QWC_TransferCount == iQWRemaining1 ) )
							if ( ( iQWRemaining1 == iQWRemaining2 ) && ( ( QWC_TransferCount - iQWRemaining2 ) > 0 ) )
							{
								// get the data pointer
								SrcDataPtr = GetMemoryPtr ( pRegData [ iChannel ]->MADR.Value );
								
								// transfer the second run (wraparound)
								iQWRemaining1 = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_TransferCount - iQWRemaining2 );
								
								// update transfer address
								pRegData [ iChannel ]->MADR.Value += ( iQWRemaining1 << 4 );
								
								// update QWC
								pRegData [ iChannel ]->QWC.QWC -= iQWRemaining1;
								
								// wrap drain MADR around mfifo
								pRegData [ iChannel ]->MADR.Value = Get_MfifoAddr ( pRegData [ iChannel ]->MADR.Value );
								
								// set the total amount of data that was transferred
								QWC_TransferCount = iQWRemaining1 + iQWRemaining2;
								
							}
							else
							{
								// set the amount of data that was transferred
								QWC_TransferCount = iQWRemaining2;
							} // end if ( ( iQWRemaining1 == iQWRemaining2 ) && ( ( QWC_TransferCount - iQWRemaining2 ) > 0 ) )
							
							// update total data transferred
							QWC_Transferred [ iChannel ] += QWC_TransferCount;
							
							
							// ***todo*** update time that bus will be in use

							
							
							return QWC_TransferCount;
						}
						else
						{
							// nothing was transferred
							return 0;
						} // end if ( iQWRemaining1 )
					}
					else
					{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; REGULAR_TRANSFER";
#endif

						// if tag is NOT cnt and NOT end, then just transfer the data
						
						// for now, just read the data
						QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_TransferCount );
						
					}
					
				}
			}
#else
			// perform transfer
			//QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, TransferCount );
			QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] );
#endif

			// update QWC
			pRegData [ iChannel ]->QWC.QWC -= QWC_TransferCount;
			
			// update MADR
			pRegData [ iChannel ]->MADR.Value += ( QWC_TransferCount << 4 );
			
			// STALL CONTROL //
			// since chain transfer, first check that tag is cnts (destination tag=0)
			// first check if source stall channel is enabled
			if ( !pRegData [ iChannel ]->CHCR.ID )
			{
				if ( DMARegs.CTRL.STS )
				{
					// if channel is a stall source channel, then copy MADR to STADR
					if ( iChannel == c_iStallSource_LUT [ DMARegs.CTRL.STS ] )
					{
						DMARegs.STADR = pRegData [ iChannel ]->MADR.Value;
						
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_5
	debug << " *STALLCTRL*";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " STADR=" << DMARegs.STADR;
#endif
					}
				}
			}
			
//#ifdef INLINE_DEBUG_TRANSFER
//	debug << " QWC_TransferCount=" << hex << QWC_TransferCount;
//#endif

			// update total data transferred
			QWC_Transferred [ iChannel ] += QWC_TransferCount;
			
			
			
			// ***todo*** update time that bus will be in use

			
			
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_7
	debug << "\r\nTransferred(QWC)=" << dec << QWC_TransferCount;
	debug << "\r\nRemaining(QWC)=" << dec << pRegData [ iChannel ]->QWC.QWC;
#endif
		}
		else
		{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_8
	debug << "\r\n***ChainTransferNotImplemented; DMA Channel Number=" << dec << iChannel << " ***";
#endif

			// no transfer function, no transfer
#ifdef DEBUG_COUT
			cout << "\nhps2x64: ALERT: PS2 DMA Chain Transfer not implemented. Channel=" << dec << iChannel << "\n";
#endif

			return 0;
		}
	}
	
	// return the number of QWs transferred to device
	return QWC_TransferCount;
}


void Dma::ChainTransfer_FromMemory ( int iChannel )
{
	u64 Data0, Data1;
	//DMATag SrcDtag;
	//DMATag DstDtag;
	
	u64 ullTagToTransfer [ 2 ];
	
	u32 TransferAddress;
	
	//u32 SrcAddress, DstAddress, TransferCount, TransferIndex;
	u64 *SrcDataPtr, *DstDataPtr, *TagDataPtr;

	u64 Cyclet;
	u64 ullReleaseTime;

	u64 ullReadyCycle;
	
	//u32 NextTagAddress, NextDataAddress;
	
	u64 QWC_TransferCount = 0;
	u64 Tag_TransferCount = 0;
	
	//bool TransferInProgress = true;
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; Chain (Source [FromMemory])";
#endif

				

	//while ( TransferInProgress )
	while ( true )
	{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; TagSource: MainMemory";
	debug << " @Cycle#" << dec << *_DebugCycleCount;
	//debug << " P3Q=" << _GPU->GIFRegs.STAT.P3Q;
#endif



		// check if transfer of block/tag has started yet
		//if ( QWC_Transferred [ iChannel ] < 0 )
		if ( !pRegData [ iChannel ]->QWC.QWC )
		{
		
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "\r\nDMA#" << iChannel << " Loading Tag from TADR=" << hex << pRegData [ iChannel ]->TADR.Value << "\r\n";
#endif

			// no tag transferred yet
			Tag_TransferCount = 0;
			


			// MFIFO //
			// if mfifo, then make sure TADR is less than MADR for source
			if ( ( DMARegs.CTRL.MFIFO > 1 ) && ( iChannel == c_iMfifoDrain_LUT [ DMARegs.CTRL.MFIFO ] ) )
			{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; MFIFO";
	debug << " RBSR=" << hex << DMARegs.RBSR.Value;
	debug << " RBOR=" << hex << DMARegs.RBOR;
	debug << " bMfifoStarted=" << bMfifoStarted;
#endif

				// MFIFO drain channel //

				/*
				if ( !bMfifoStarted )
				{
					// mfifo transfer has not started yet
					return;
				}
				*/

				if (pRegData[iChannel]->TADR.Value == pRegData[8]->MADR.Value)
				{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
		debug << " MFIFOEMPTY";
#endif

					// mfifo is empty //

#ifdef ONLY_TRANSFER_MFIFO_SOURCE_ON_EQUAL
					if (!pRegData[8]->CHCR.STR)
#endif
					{
						// set mfifo empty interrupt
						DMARegs.STAT.MEIS = 1;

						// update interrupts
						UpdateInterrupt();
					}
					
					// update next transfer
					iLastChannel = iChannel;
					if ( GetNextActiveChannel () != iLastChannel )
					{
						// also needs to return to handle any other active channels when done
						CheckTransfer ();
					}
					
					// no data was transferred
					return;
				}
				
				// get transfer address for TAG (needs to wrap around for mfifo but not write back to TADR?)
				TransferAddress = ( pRegData [ iChannel ]->TADR.Value & DMARegs.RBSR.Value ) | DMARegs.RBOR;
			}
			else
			{
				// NOT mfifo //
				
				// otherwise, tag address is just TADR
				TransferAddress = pRegData [ iChannel ]->TADR.Value;
			}
			
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << " TADR=" << hex << pRegData [ iChannel ]->TADR.Value;
	debug << " TADR_TransferAddress=" << TransferAddress;
#endif

			// if destination chain mode channel#8, then the tag comes from SADR, and on channel#5 comes from the input data
			if ( iChannel == 8 )
			{
				TagDataPtr = & ( DataBus::_BUS->ScratchPad.b64 [ ( ( pRegData [ 8 ]->SADR.Address & DataBus::ScratchPad_Mask ) >> 3 ) & ~1 ] );
				Data0 = TagDataPtr [ 0 ];
				Data1 = TagDataPtr [ 1 ];
				
				// the data is 16 bytes after the tag in destination chain mode
				pRegData [ iChannel ]->SADR.Value += 16;
			}
			else
			{
				// load 128-bits from the tag address
				//TagDataPtr = GetMemoryPtr ( DmaCh [ iChannel ].TADR_Reg.Value );
				TagDataPtr = GetMemoryPtr ( TransferAddress );
				Data0 = TagDataPtr [ 0 ];
				Data1 = TagDataPtr [ 1 ];
			}

#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; TagData0=" << hex << Data0 << "; TagData1=" << Data1;
#endif



			// looks like the upper 32-bits of Data0 is the address and the lower 32-bits are ID/PCE/FLG etc
			// Data1 appears to be zero and not used anyway
			// this is where I'll need to process the TAG since the address could mean anything
			
			// set the source dma tag
			SourceDMATag [ iChannel ].Value = Data0;
			
			// set bits 16-31 of CHCR to the last tag read bits 16-31
			//DmaCh [ iChannel ].CHCR_Reg.Value = ( DmaCh [ iChannel ].CHCR_Reg.Value & 0xffffL ) | ( SourceDMATag [ iChannel ].Value & 0xffff0000L );
			pRegData [ iChannel ]->CHCR.Value = ( pRegData [ iChannel ]->CHCR.Value & 0xffffL ) | ( Data0 & 0xffff0000L );

#ifdef VERBOSE_DEBUG_PCE
// if pce is non-zero, then need to alert for now
if ( pRegData [ iChannel ]->CHCR.PCE )
{
cout << "\nhps2x64: ALERT: PCE is non-zero in dma tag!!!";
}
#endif

#if defined INLINE_DEBUG_TRANSFER
	debug << "; SETMADR";
#endif

			// if this is destination chain mode, then MADR gets set with ADDR for now
			// ***todo*** error checking
			if ( iChannel == 8 )
			{
				switch ( pRegData [ iChannel ]->CHCR.ID )
				{
					// CNTS
					case 0:

					// CNT
					case 1:
					
					// END
					case 7:
						pRegData [ iChannel ]->MADR.Value = Data0 >> 32;
						break;
						
					default:
						cout << "\nhps2x64: PS2DMA#" << dec << iChannel << ": ERROR: Invalid Destination tag ID=" << pRegData [ iChannel ]->CHCR.ID;
						
						// undefined tag //
						// end transfer
						EndTransfer ( iChannel );
						
						// done
						return;
						
						
						break;
				}
			}
			else
			{
				// transfer the tag
				// ***todo*** this is probably the point at which you're supposed to transfer the tag
				//if ( DmaCh [ iChannel ].CHCR_Reg.TTE && QWC_BlockTotal [ iChannel ] )
				if ( pRegData [ iChannel ]->CHCR.TTE )
				{
				
					// make sure that device is ready for transfer of the next block
					// check if device is ready for transfer
					// check if channel has a ready function
					if ( cbReady [ iChannel ] )
					{
						// get whether dma channel is ready or not or in the future
						ullReadyCycle = cbReady[iChannel]();

						// check if channel is ready for transfer
						if ( !ullReadyCycle )
						{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; DeviceNotReady";
#endif
							
							return;
						}
						else if (ullReadyCycle > *_DebugCycleCount)
						{
							// channel will be ready in the future //
							SetNextEventCh_Cycle(ullReadyCycle, iChannel);

							return;
						}

					} // end if ( cbReady [ iChannel ] )
					
			
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; DeviceIsReady";
#endif

				
					// check if channel has a transfer function
					if ( cbTransfer_FromMemory [ iChannel ] )
					{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_0
	debug << "; TagTransfer";
#endif

						// Tag should be transferred //


	#ifdef DMA1_CHAIN_RESTART_VUCOMMAND
						if ( iChannel < 2 )
						{
						VU1::_VU1->lVifIdx = 0;
						VU1::_VU1->lVifCodeState = 0;
						}
	#endif

	#ifdef DMA01_TAG_TRANSFER_HIGH
						if ( iChannel < 2 )
						{
							// transferring tag to either vu0 or vu1 //

							// if vifidx is not zero, then something must be wrong
							//if ( VU::_VU[ iChannel ]->lVifIdx )
							//{
							//	cout << "\nALERT: PS2: DMA:Performing a tag transfer to vu#" << iChannel << " while lVifIdx is NOT zero. lVifIdx=" << dec << VU::_VU[ iChannel ]->lVifIdx;
							//}

							//VU::_VU[ iChannel ]->lVifIdx |= 2;

							if ( VU::_VU[ iChannel ]->lVifIdx < 2 )
							{
								VU::_VU[ iChannel ]->lVifIdx = 2;
							}
						}
#endif
						
						//TagDataPtr = GetMemoryPtr ( DmaCh [ iChannel ].TADR_Reg.Value );

						// channel 9 needs to transfer the full tag as is
						if ( iChannel == 9 )
						{
						ullTagToTransfer [ 0 ] = TagDataPtr [ 0 ];
						}
						else
						{
						// ***TODO*** when transferring tag, need to zero bottom 64-bits
						ullTagToTransfer [ 0 ] = 0;
						}
						
						ullTagToTransfer [ 1 ] = TagDataPtr [ 1 ];
						
						// ?? don't actually transfer tag ??
						// or don't transfer for dma#2 ??
						if ( iChannel != 2 )
						{
						//cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, 1 );
						Tag_TransferCount = cbTransfer_FromMemory [ iChannel ] ( ullTagToTransfer, 1 );
						
						// check if the tag was not transferred
						if ( !Tag_TransferCount )
						{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; UnableToTransferTagYet";
#endif

							iLastChannel = iChannel;
							if ( GetNextActiveChannel () != iLastChannel )
							{
								// also needs to return to handle any other active channels when done
								CheckTransfer ();
							}

							// the tag did not fully transfer
							//return 0;
							return;
						}
						}
						else
						{
#ifdef VERBOSE_DEBUG_DMA2_TTE
							cout << "\nhps2x64: GPU: ALERT: GPU DMA TTE Enabled!!!\n";
#endif
						}
						
						
						// update amount transferred
						//QWC_Transferred [ iChannel ] = 0;
					}

				}	// end if ( pRegData [ iChannel ]->CHCR.TTE )


				// should probably also set MADR here
				switch ( pRegData [ iChannel ]->CHCR.ID )
				{
					// 0: refe - ADDR
					// 3: ref - ADDR
					// 4: refs - ADDR
					case 0:
					case 3:
					case 4:
						pRegData [ iChannel ]->MADR.Value = Data0 >> 32;
						break;
						
					// 1: cnt - next to tag
					// 2: next - next to tag
					// 5: call - next to tag
					// 6: ret - next to tag
					// 7: end - next to tag
					case 1:
					case 2:
					case 5:
					case 6:
					case 7:
						pRegData [ iChannel ]->MADR.Value = pRegData [ iChannel ]->TADR.Value + 16;
						break;
				}
				
			

#if defined INLINE_DEBUG_TRANSFER
	debug << "; SETTADR";
#endif

				// this is probably where the TAG address gets updated if necessary
				switch ( pRegData [ iChannel ]->CHCR.ID )
				{
					// 0: refe - does not update tag (but probably does??)
					// 3: ref - next to tag
					// 4: refs - next to tag
					case 0:
					case 3:
					case 4:
#if defined INLINE_DEBUG_TRANSFER
	debug << "; REF/REFE/REFS";
#endif

						pRegData [ iChannel ]->TADR.Value += 16;
						break;
					
					// looks like cnt is handled after transfer of block completes??
					// 1: cnt - next to transfer data
					//case 1:
					//	DmaCh [ iChannel ].TADR_Reg.Value += ( ( Data0 & 0xffff ) << 4 ) + 16;
					//	break;
						
					// 2: next - ADDR
					case 2:
#if defined INLINE_DEBUG_TRANSFER
	debug << "; NEXT";
#endif

						pRegData [ iChannel ]->TADR.Value = Data0 >> 32;
						break;
						
					// 5: call - ADDR (special handling)
					case 5:
#if defined INLINE_DEBUG_TRANSFER
	debug << "; CALL";
#endif
					
						// if ASP is 2 when the CALL tag is read, then the packet is NOT transferred
						// and also the address of the CALL tag is left in TADR
						if ( pRegData [ iChannel ]->CHCR.ASP >= 2 )
						{
							// end transfer
							EndTransfer ( iChannel );
							
							// done
							return;
						}
						
						// set next tag address to ADDR
						// this is probably done before transfer starts
						pRegData [ iChannel ]->TADR.Value = Data0 >> 32;
						
						// tag to push onto stack is next to data
						// this stuff is probably done when transfer completes
						//DmaCh [ iChannel ].TADR_Reg.Value += ( ( Data0 & 0xffff ) << 4 ) + 16;
						
						// push address onto stack
						//switch ( DmaCh [ iChannel ].CHCR_Reg.ASP )
						//{
						//	case 0:
						//		DmaCh [ iChannel ].ASR0_Reg.Value = DmaCh [ iChannel ].TADR_Reg.Value;
						//		break;
						//		
						//	case 1:
						//		DmaCh [ iChannel ].ASR1_Reg.Value = DmaCh [ iChannel ].TADR_Reg.Value;
						//		break;
						//}
							
						// increase the amound pushed onto stack
						//DmaCh [ iChannel ].CHCR_Reg.ASP += 1;
						
						
						break;
						
					// 6: ret - DxASR
					//case 6:
					//	// probably should be handled at end of transfer
					//	break;
					
				}	// end switch ( pRegData [ iChannel ]->CHCR.ID )

			}	// end else if ( iChannel == 8 )
			
			// MFIFO //
			// check if channel is drain channel
			// if so, then need to wrap around TADR
			
			// ?? put address into MADR ??
			//DmaCh [ iChannel ].MADR_Reg.Value = SrcDtag.ADDR;
			
			// reading from address 0 is allowed?
			// if it doesn't read from address zero, then if address is missing other issues can happen
			//if ( !pRegData [ iChannel ]->MADR.Value )
			//{
			//	// shouldn't be reading from address zero ??
			//	cout << "\nhps2x64: PS2DMA: ALERT: MADR is zero during chain transfer.\n";
			//}


			// set the transfer count
			//TransferCount = SrcDtag.QWC;
			//QWC_BlockTotal [ iChannel ] = SrcDtag.QWC;
			//QWC_BlockTotal [ iChannel ] = SourceDMATag [ iChannel ].QWC;
			QWC_BlockTotal [ iChannel ] = Data0 & 0xffff;
			
			// this is also probably supposed to set QWC
			pRegData [ iChannel ]->QWC.Value = Data0 & 0xffff;
			



#if defined INLINE_DEBUG_TRANSFER
	debug << " SetQWC=" << dec << pRegData [ iChannel ]->QWC.Value;
	debug << " Data0=" << hex << Data0;
#endif

			
			// ***todo*** might need to add onto block total if tag is being transferred too
			// instead set to -1 if transferring tag
			
				QWC_Transferred [ iChannel ] = -1;
			
			
			// the data is ready to transfer, then return for now
			// so that if the dma gets halted, qwc is not zero
			//if ( !pRegData [ iChannel ]->QWC.QWC )
			//{
			//	SetNextEventCh ( 1 + ( c_iDmaTransferTimePerQwc [ iChannel ] * ( Tag_TransferCount + QWC_TransferCount ) ), iChannel );
			//	return;
			//}
			
		}	// if ( !pRegData [ iChannel ]->QWC.QWC )

		
		
		if ( pRegData [ iChannel ]->QWC.QWC )
		{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	//debug << " CALL-READY";
	//debug << " P3Q=" << _GPU->GIFRegs.STAT.P3Q;
#endif

			if ( cbReady [ iChannel ] )
			{
				// get whether dma channel is ready or not or in the future
				ullReadyCycle = cbReady[iChannel]();

				// check if channel is ready for transfer
				if ( !ullReadyCycle )
				{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_CHAIN_FROMMEM
	debug << "; DeviceNotReady";
#endif

					return;
				}
				else if (ullReadyCycle > *_DebugCycleCount)
				{
					// channel will be ready in the future //
					SetNextEventCh_Cycle(ullReadyCycle, iChannel);

					return;
				}

			} // end if ( cbReady [ iChannel ] )

#if defined INLINE_DEBUG_TRANSFER
	debug << "; READY";
	debug << " (before)QWC=" << dec << pRegData [ iChannel ]->QWC.QWC;
	debug << " TADR=" << hex << pRegData [ iChannel ]->TADR.Value;
	//debug << " P3Q=" << _GPU->GIFRegs.STAT.P3Q;
#endif


			// perform the transfer
			QWC_TransferCount = Chain_TransferBlock ( iChannel );


			// set amount of time bus will be busy for
			if ( QWC_TransferCount )
			{
				if ( iChannel == 6 )
				{
				DataBus::_BUS->ReserveBus_DMA ( *_DebugCycleCount, QWC_TransferCount << 4 );
				}
				else
				{
				DataBus::_BUS->ReserveBus_DMA ( *_DebugCycleCount, QWC_TransferCount );
				}
			}

			
#if defined INLINE_DEBUG_TRANSFER
	debug << " (after)QWC=" << dec << pRegData [ iChannel ]->QWC.QWC << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	debug << " TADR=" << hex << pRegData [ iChannel ]->TADR.Value;
	debug << " QWC_TransferCount=" << dec << QWC_TransferCount;
#endif



		} // end if ( DmaCh [ iChannel ].QWC_Reg.QWC )
		
		

		// check if transfer of block is complete
		//if ( QWC_Transferred [ iChannel ] >= QWC_BlockTotal [ iChannel ] )
		if ( !pRegData [ iChannel ]->QWC.QWC )
		{
#if defined INLINE_DEBUG_TRANSFER
	debug << "; !QWC";
	debug << " Channel#" << iChannel;
	debug << " ID=" << pRegData [ iChannel ]->CHCR.ID;
	debug << " LAST-TAG-ID=" << SourceDMATag [ iChannel ].ID;
#endif

			// next tag is next to data
			//DmaCh [ iChannel ].TADR_Reg.Value += ( QWC_BlockTotal [ iChannel ] << 4 ) + 16;
			
			// start new tag
			QWC_Transferred [ iChannel ] = -1;
			
			// check if it is destination chain mode
			if ( iChannel == 8 )
			{
#if defined INLINE_DEBUG_TRANSFER
	debug << " DST-CHAIN";
#endif
			
				// destination chain mode //
			
				switch ( pRegData [ iChannel ]->CHCR.ID )
				{
					// CNTS
					case 0:
						//cout << "\nhps2x64: ALERT: DMA: PS2DMA#8 CNTS destination tag not implemented yet.\n";

						// STALL CONTROL //
						// since chain transfer, first check that tag is cnts (destination tag=0)
						// if channel is a stall source channel, then copy MADR to STADR
						if (_DMA->DMARegs.CTRL.STS == 2)
						{
							_DMA->DMARegs.STADR = pRegData[8]->MADR.Value;
						}
						break;

					// if the tag is end, then end the transfer
					case 7:

					// ??
					//case 2:
					//case 3:
					//case 4:
					
						// end transfer
						EndTransfer ( iChannel );
						
						// done
						return;
						
						break;
						
						
					default:
						break;
						
				}
				
			}	// end if ( iChannel == 8 )
			else
			{
#if defined INLINE_DEBUG_TRANSFER
	debug << " SRC-CHAIN";
#endif
			
				// source chain mode //
			
				//switch ( SourceDMATag [ iChannel ].ID )
				switch ( pRegData [ iChannel ]->CHCR.ID )
				{
					// if the tag is end or refe, then end the transfer
					case 0:
					case 7:
#if defined INLINE_DEBUG_TRANSFER
	debug << " REFE/END";
#endif
					
						// end transfer
						EndTransfer ( iChannel );
						
						// done
						return;
						
						break;
						
					// cnt probably does not update the tag address until after the end of transfer it appears
					// 1: cnt - next to transfer data
					case 1:
#if defined INLINE_DEBUG_TRANSFER
	debug << " CNT";
#endif

						//DmaCh [ iChannel ].TADR_Reg.Value += ( ( Data0 & 0xffff ) << 4 ) + 16;
						//DmaCh [ iChannel ].TADR_Reg.Value = DmaCh [ iChannel ].MADR_Reg.Value;
						pRegData [ iChannel ]->TADR.Value += ( SourceDMATag [ iChannel ].QWC << 4 ) + 16;
						break;
					
					// if the tag is call, probably best if it is handled...
					case 5:
#if defined INLINE_DEBUG_TRANSFER
	debug << " CALL";
#endif

						// tag to push onto stack is next to data
						// this stuff is probably done when transfer completes
						//DmaCh [ iChannel ].TADR_Reg.Value += ( ( Data0 & 0xffff ) << 4 ) + 16;
						
						// push address onto stack
						switch ( pRegData [ iChannel ]->CHCR.ASP )
						{
							case 0:
								pRegData [ iChannel ]->ASR0.Value = pRegData [ iChannel ]->MADR.Value;
								break;
								
							case 1:
								pRegData [ iChannel ]->ASR1.Value = pRegData [ iChannel ]->MADR.Value;
								break;
						}
							
						// increase the amound pushed onto stack
						pRegData [ iChannel ]->CHCR.ASP += 1;
						
						break;
						
						
					// if the tag is ret, then return, but if ASP is zero, then done
					case 6:
#if defined INLINE_DEBUG_TRANSFER
	debug << " RET";
#endif
					
						if ( pRegData [ iChannel ]->CHCR.ASP > 0 )
						{
#if defined INLINE_DEBUG_TRANSFER
	debug << " ASP>0";
#endif

							// decrease the amound pushed onto stack
							pRegData [ iChannel ]->CHCR.ASP -= 1;
							
							// next tag is popped from stack
							switch ( pRegData [ iChannel ]->CHCR.ASP )
							{
								case 0:
									pRegData [ iChannel ]->TADR.Value = pRegData [ iChannel ]->ASR0.Value;
									break;
									
								case 1:
									pRegData [ iChannel ]->TADR.Value = pRegData [ iChannel ]->ASR1.Value;
									break;
							}
						}
						else
						{
#if defined INLINE_DEBUG_TRANSFER
	debug << " ASP=0";
#endif

							// ret when ASP is zero //
							
							// set TADR to zero ?
							//pRegData [ iChannel ]->TADR.Value = 0;
							
							// end transfer
							EndTransfer ( iChannel );
							
							// done
							return;
						}
						
						break;

				}	// end switch ( pRegData [ iChannel ]->CHCR.ID )
				
			}	// end if ( iChannel == 8 ) else
			
			// if IRQ bit is set, then transfer no longer in progress
			// also check Tag Interrupt Enable (TIE)
			//if ( SourceDMATag [ iChannel ].IRQ && DmaCh [ iChannel ].CHCR_Reg.TIE )
			if ( pRegData [ iChannel ]->CHCR.IRQ && pRegData [ iChannel ]->CHCR.TIE )
			{
#if defined INLINE_DEBUG_TRANSFER
	debug << " IRQ+TIE";
#endif

				// interrupt??
				EndTransfer ( iChannel );
				
				//TransferInProgress = false;
				return;
			}

			// QWC=0, so need to update the QWC with the TAG etc in case DMA is halted
			// QWC should not be zero unless chain transfer dma is finished?

		}	// end if ( !pRegData [ iChannel ]->QWC.QWC )



		
#ifdef TEST_ASYNC_DMA_STAGE2
		// block of data has been transferred, so if there delay for data transfer implement here
		if ( c_iDmaTransferTimePerQwc [ iChannel ] )
		{
#ifdef INLINE_DEBUG_TEST5
	debug << "\r\n***Testing***";
	debug << " iChannel=" << dec << iChannel;
	debug << " c_iDmaTransferTimePerQwc[]=" << c_iDmaTransferTimePerQwc [ iChannel ];
	debug << " QWC_TransferCount=" << QWC_TransferCount;
	debug << " c_iSetupTime=" << c_iSetupTime;
	debug << " Passing: " << ( c_iDmaTransferTimePerQwc [ iChannel ] * QWC_TransferCount ) + c_iSetupTime;
#endif

#if defined INLINE_DEBUG_TRANSFER
	debug << " TRANSFERTIME";
#endif

			// don't want to stop here if cpu is unable to interrupt the transfer
			//if ( ( DMARegs.CTRL.CycleStealMode ) || ( iChannel < 8 ) )
			{
#if defined INLINE_DEBUG_TRANSFER
	debug << " DELAY";
#endif

				// if cyclestealing is on, get release time
				ullReleaseTime = 0;
				//if ( DMARegs.CTRL.CycleStealMode )
				//{
				//	ullReleaseTime = 1 << ( DMARegs.CTRL.RCYC + 3 );
				//}

				// if channel#6 testing
				if( iChannel == 6 )
				{
					
					Cyclet = R3000A::Cpu::_CPU->CycleCount << 2;


					Cyclet += ullReleaseTime;
					_DMA->DmaCh [ 6 ].ullStartCycle = Cyclet;
					
					_DMA->SetNextEventCh_Cycle ( Cyclet, 6 );
				}
				else
				{
					// continue transfer after data in device buffer has been processed
					Cyclet = ( c_iDmaTransferTimePerQwc [ iChannel ] * QWC_TransferCount ) + c_iSetupTime;
					Cyclet += ullReleaseTime;
					SetNextEventCh ( Cyclet, iChannel );
				}

#ifdef ENABLE_PS1_SYNC_DMA6
				if ( iChannel == 6 )
				{
					u64 Cyclet;
					
					Cyclet = R3000A::Cpu::_CPU->CycleCount << 2;

					// add in the time on the ps2 side to transfer into sif buffer from ram
					Cyclet += c_iDmaTransferTimePerQwc [ iChannel ] * QWC_TransferCount;

					// add in the time on the ps1 side to transfer from sif buffer into ram
					// note: don't do this because it is after the transfer, so the ps1 side already added this in
					//Cyclet += QWC_TransferCount << 4;

					if ( Cyclet > *_DebugCycleCount )
					{
						_DMA->DmaCh [ 6 ].ullStartCycle = Cyclet;
						
						_DMA->SetNextEventCh_Cycle ( Cyclet, 6 );
						
						iLastChannel = iChannel;
						if ( GetNextActiveChannel () != iLastChannel )
						{
							CheckTransfer ();
						}
					}
				}
#endif

				
#if defined INLINE_DEBUG_TRANSFER
	debug << " UNTIL " << dec << NextEventCh_Cycle [ iChannel ];
#endif


				// don't return here, so that qwc is not zero on return unless transfer is complete
				// or if tag doesn't transfer?
				// should be able to return as long as QWC is not zero
				//if ( pRegData [ iChannel ]->QWC.QWC )
				//if ( iChannel != 2 )
				{
					return;
				}

			}	// end if ( ( DMARegs.CTRL.CycleStealMode ) || ( iChannel < 8 ) )

		}	// end if ( c_iDmaTransferTimePerQwc [ iChannel ] )
#endif
		
	}	// end while ( true )
}

void Dma::InterleaveTransfer_ToMemory ( int iChannel )
{
}

void Dma::InterleaveTransfer_FromMemory ( int iChannel )
{
	u64 QWC_TransferCount = 0;
	u64 *SrcDataPtr;

	u64 ullReadyCycle;

	// check if transfer of block/tag has started yet
	if ( !pRegData [ iChannel ]->QWC.QWC || ( !DMARegs.SQWC.TQWC ) )
	{
		// if QWC is zero, it means there is no transfer and complete
		
		// if STR is set, then should probably also call end transfer?
		if ( pRegData [ iChannel ]->CHCR.STR )
		{
			EndTransfer ( iChannel );
		}
		
		// done
		return;
	}
	
	// check if channel is a function to check if it is ready
	if ( cbReady [ iChannel ] )
	{
		// get whether dma channel is ready or not or in the future
		ullReadyCycle = cbReady[iChannel]();

		// check if channel is ready for transfer
		if ( !ullReadyCycle )
		{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_INTERLEAVE
	debug << "; DeviceNotReady";
#endif

			// channel/device not ready for transfer
			return;
		}
		else if (ullReadyCycle > *_DebugCycleCount)
		{
			// channel will be ready in the future //
			SetNextEventCh_Cycle(ullReadyCycle, iChannel);

			return;
		}
	}
	
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_INTERLEAVE
	debug << "; DeviceIsReady";
#endif

#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_INTERLEAVE
	debug << " TQWC=" << dec << (u32) DMARegs.SQWC.TQWC;
	debug << " SQWC=" << dec << (u32) DMARegs.SQWC.SQWC;
	debug << " QWC=" << dec << pRegData [ iChannel ]->QWC.QWC;
#endif

	// transfer the data
		
	// check if channel has a transfer function
	if ( cbTransfer_FromMemory [ iChannel ] )
	{
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_INTERLEAVE
	//debug << "\r\nLeft(QWC)=" << dec << ( QWC_BlockTotal [ iChannel ] - QWC_Transferred [ iChannel ] );
#endif

		while ( true )
		{
			// get memory data pointer
			SrcDataPtr = GetMemoryPtr ( pRegData [ iChannel ]->MADR.Value );
			
			// get the amount to transfer
			QWC_TransferCount = ( ( (u32) DMARegs.SQWC.TQWC ) < ( pRegData [ iChannel ]->QWC.QWC ) ) ? ( (u32) DMARegs.SQWC.TQWC ) : ( pRegData [ iChannel ]->QWC.QWC );

			// perform the transfer
			QWC_TransferCount = cbTransfer_FromMemory [ iChannel ] ( SrcDataPtr, QWC_TransferCount );

			// update QWC
			pRegData [ iChannel ]->QWC.QWC -= QWC_TransferCount;
			
#if defined INLINE_DEBUG_TRANSFER || defined INLINE_DEBUG_TRANSFER_INTERLEAVE
	debug << " QWC-TC=" << dec << QWC_TransferCount;
	debug << " QWC-NEW=" << dec << pRegData [ iChannel ]->QWC.QWC;
#endif

			// update MADR based on QWC transferred and QWC to skip also for interleave transfer
			pRegData [ iChannel ]->MADR.Value += ( QWC_TransferCount << 4 ) + ( ( (u32) DMARegs.SQWC.SQWC ) << 4 );
			
			
			// set amount of time bus will be busy for
			if ( QWC_TransferCount )
			{
				DataBus::_BUS->ReserveBus_DMA ( *_DebugCycleCount, QWC_TransferCount );
			}
			
			
			// check if done
			if ( !pRegData [ iChannel ]->QWC.QWC )
			{
				// if QWC is zero, it means there is no transfer and complete
				
				// if STR is set, then should probably also call end transfer?
				if ( pRegData [ iChannel ]->CHCR.STR )
				{
					EndTransfer ( iChannel );
				}
				
				// done
				return;
			}
			
			
			// check if transferring all at once or not
			if ( ( DMARegs.CTRL.CycleStealMode ) || ( iChannel < 8 ) )
			{
				// cycle steal mode is either on or it doesn't matter for now //
				
				// come back later
				SetNextEventCh ( ( c_iDmaTransferTimePerQwc [ iChannel ] * QWC_TransferCount ) + c_iSetupTime, iChannel );
				
				// done for now
				return;
				
			}
			
		} // end while ( true )
		
	} // end if ( cbTransfer_FromMemory [ iChannel ] )
		
}


// use this to complete transfer after dmas are restarted after a suspension
void Dma::UpdateTransfer ()
{
	for ( int i = 0; i < c_iNumberOfChannels; i++ )
	{
		if ( i != 7 )
		{
			if ( pRegData [ i ]->CHCR.STR )
			{
				Transfer ( i );
			}
		}
	}
	
	/*
	// channel 0 has the highest priority
	if ( DmaCh [ 0 ].CHCR_Reg.STR )
		Transfer ( 0 );
	
	// the other channels come after that
	// skip channel 7
	if ( DmaCh [ 1 ].CHCR_Reg.STR )
		Transfer ( 1 );
		
	if ( DmaCh [ 2 ].CHCR_Reg.STR )
		Transfer ( 2 );
		
	if ( DmaCh [ 3 ].CHCR_Reg.STR )
		Transfer ( 3 );
		
	if ( DmaCh [ 4 ].CHCR_Reg.STR )
		Transfer ( 4 );
		
	if ( DmaCh [ 5 ].CHCR_Reg.STR )
		Transfer ( 5 );
		
	if ( DmaCh [ 6 ].CHCR_Reg.STR )
		Transfer ( 6 );
		
	if ( DmaCh [ 8 ].CHCR_Reg.STR )
		Transfer ( 8 );
		
	if ( DmaCh [ 9 ].CHCR_Reg.STR )
		Transfer ( 9 );
	*/
}


void Dma::Transfer ( int iChannel )
{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "\r\nDma:Transfer Ch#" << dec << iChannel;
	debug << " PS2-SYSTEM-CYCL#" << *_DebugCycleCount;
	debug << "; (before) CHCR=" << hex << pRegData [ iChannel ]->CHCR.Value << " STAT=" << DMARegs.STAT.Value;
	debug << "; (before) MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
	//debug << " P3Q=" << _GPU->GIFRegs.STAT.P3Q;
#endif

	//u64 Data0, Data1;
	
	//DMATag DstDtag;
	
	//u32 SrcAddress, DstAddress, TransferCount, TransferIndex;
	//u64 *SrcDataPtr, *DstDataPtr;
	
	//u32 NextTagAddress, NextDataAddress;
	
	//bool TransferInProgress = true;


	if ( !( DMARegs.CTRL.DMAE ) )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << " !DMAE";
#endif

		cout << "\n***hps2x64: DMA: ALERT: Channel#" << dec << iChannel << " Transfer while ps2 dma tranfers are DISABLED***\n";


		// dma transfers disabled
		return;
	}

	
	// check if dma transfers are being held
	if ( ( DMARegs.ENABLEW & 0x10000 ) )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; HOLD";
	debug << "; MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
#endif

		// all dma transfers disabled
		return;
	}

	
	// check if channel STR is 1
	if ( !pRegData [ iChannel ]->CHCR.STR )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; Channel STR=0; Channel not enabled";
#endif

		return;
	}


	// also need to check cde bit to see if channel is enabled
	/*
	if ( ( DMARegs.PCR.PCE ) && !( ( DMARegs.PCR.Value >> 16 ) & ( 1 << iChannel ) ) )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; PCE=1 && Channel PCR=0; Channel not enabled";
#endif

		return;
	}
	*/


	
#ifdef PRIORITY_DMA0
	// if dma#0 is running and this is not dma#0, dma#0 has priority
	if ( pRegData [ 0 ]->CHCR.STR && iChannel )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " DMA#0-HAS-PRIORITY";
#endif

		Transfer ( 0 );
		return;
	}
#endif
	

#ifdef PRIORITY_DMA1
	// if trying to transfer via dma#2, then check if there is currently a packet transferring for dma#2
	// if there is not, then if dma#1 is running give it priority?
	if ( pRegData [ 1 ]->CHCR.STR && iChannel == 2 && !( GPU::_GPU->PacketInProgress [ 2 ] ) )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " DMA#1-HAS-PRIORITY";
#endif

		Transfer ( 1 );
		return;
	}
#endif
	
	// if channel 5/6 (SIF0/SIF1) then make sure IOP is ready for transfer
	switch ( iChannel )
	{
		case 5:
			/*
			if ( !SIF::_SIF->IOP_DMA_Out_Ready () )
			{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; PS1 SIF0 NOT READY";
#endif

				return;
			}
			
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; PS1 SIF0 READY";
#endif
			*/

			break;
			
		case 6:
			/*
			if ( !SIF::_SIF->IOP_DMA_In_Ready () )
			{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; PS1 SIF1 NOT READY";
#endif

				return;
			}
			
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; PS1 SIF1 READY";
#endif
			*/

			break;
	}
	
	
	// check that dma is ready to run
	if ( *_DebugCycleCount < DmaCh [ iChannel ].ullStartCycle )
	{
#ifdef INLINE_DEBUG_TRANSFER
	debug << " DMA-WAIT";
	debug << " NEXT-RUN-AT-CYCL#" << dec << DmaCh [ iChannel ].ullStartCycle;
#endif

		SetNextEventCh_Cycle ( DmaCh [ iChannel ].ullStartCycle, iChannel );
		
		// check if there are any transfers that can run in the meantime
		iLastChannel = iChannel;
		if ( GetNextActiveChannel () != iLastChannel )
		{
			CheckTransfer ();
		}
		
		return;
	}
	
	// check if channel has a dma setup time
	// dma setup times are disabled for now
	// will use the ullStartCycle variable for this
	/*
	if ( c_iDmaSetupTime [ iChannel ] )
	{
		// check if it is not time for DMA transfer to continue
		if ( NextEvent_Cycle != NextEventCh_Cycle [ iChannel ] )
		//if ( *_DebugCycleCount < ( DmaCh [ iChannel ].StartCycle + c_iDmaSetupTime [ iChannel ] ) )
		{
			// set transfer to continue after setup time
			SetNextEventCh ( c_iDmaSetupTime [ iChannel ], iChannel );
			//SetNextEventCh_Cycle ( DmaCh [ iChannel ].StartCycle + c_iDmaSetupTime [ iChannel ], iChannel );

			return;
		}
	}
	*/

	
	

#ifdef INLINE_DEBUG_TRANSFER
	debug << "; CHCR=" << hex << pRegData [ iChannel ]->CHCR.Value;
#endif

	// check if transfer is in chain mode
	switch ( pRegData [ iChannel ]->CHCR.MOD )
	{
		// Normal transfer //
		case 0:
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; Normal";
#endif

			switch ( iChannel )
			{

				// the channels that are always TO memory are 3, 5, 8
				case 3:
				case 5:	// will pull this in later
				case 8:
				
#ifdef VERBOSE_NORMAL_TOMEM
					cout << "\nhps2x64 ALERT: DMA: attempted NORMAL transfer TO memory via DMA Channel#" << dec << iChannel << "\n";
#endif

					// a normal transfer from dma#5 isn't accounted for
					// so alert if this happens and stop transfer
					if ( iChannel == 5 )
					{
						cout << "\nhps2x64: ALERT: DMA: Normal transfer for dma#5!!!\n";
						return;
					}
					
					// will use the transfer FROM memory function for now
					// perform NORMAL transfer FROM memory
					NormalTransfer_ToMemory ( iChannel );
					
					break;
					
				// the channels that are always FROM memory are 0, 2, 4, 6, 9
				case 0:
				case 2:
				case 4:
				case 6:
				case 9:
				
					if ( iChannel != 2 )
					{
#ifdef VERBOSE_NORMAL_FROMMEM
						cout << "\nhps2x64: ALERT: DMA: attempted non-gpu normal transfer from memory. Channel#" << iChannel << "\n";
#endif
					}
				
					// a normal transfer from dma#6 isn't accounted for
					// so alert if this happens and stop transfer
					if ( iChannel == 6 )
					{
						cout << "\nhps2x64: ALERT: DMA: Normal transfer for dma#6!!!\n";
						return;
					}
					
					// perform NORMAL transfer FROM memory
					NormalTransfer_FromMemory ( iChannel );
				
					break;
					
				// direction of dma transfer only matters for channels 1 and 7
				case 1:
				case 7:
				
					// check if this is going from memory or to memory
					switch ( pRegData [ iChannel ]->CHCR.DIR )
					{
						// to memory
						case 0:
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; ToMemory";
#endif

#ifdef VERBOSE_DMA_1_7_TO_MEM
							cout << "\nhps2x64 ALERT: DMA: attempted NORMAL transfer to memory via DMA Channel#" << dec << iChannel << "\n";
#endif
							
							NormalTransfer_ToMemory ( iChannel );
							break;
							
						// from memory
						case 1:
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; FromMemory";
	debug << " MADR=" << hex << pRegData [ iChannel ]->MADR.Value;
#endif

							//if ( iChannel != 2 )
							//{
							//	cout << "\nhps2x64: ALERT: DMA: attempted non-gpu normal transfer from memory. Channel#" << iChannel << "\n";
							//}
							
							// perform NORMAL transfer FROM memory
							NormalTransfer_FromMemory ( iChannel );

							break;
					}
						
					break;

			}	// switch for channel number
				
			break;	// NORMAL Transfer

				
		// Chain transfer (Source) //
		case 1:
		case 3:	// ??
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; Chain (Source [FromMemory])";
#endif


			// check the channel number
			switch ( iChannel )
			{
				case 5:
				
					// first make sure it is ready for the transfer
					if ( cbReady_ToMemory [ 5 ] )
					{
						if ( cbReady_ToMemory [ 5 ] () )
						{
							// pull data from SIF for transfer
							// ***TODO*** this should actually read the data using a consistent method
							SIF::EE_DMA_ReadBlock ();
							
							iLastChannel = 5;
							if ( GetNextActiveChannel () != iLastChannel )
							{
								// also needs to return to handle any other active channels when done
								CheckTransfer ();
							}
						}
					}
						
				
					break;
					
				/*
				case 6:
				
					
					if ( c_ullSIFOverhead )
					{
					
						if ( *_DebugCycleCount == NextEventCh_Cycle [ 6 ] )
						{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "\r\nCH#6 TRANSFER";
#endif

							// pull data from SIF for transfer
							// ***TODO*** this should actually read the data using a consistent method
							ChainTransfer_FromMemory ( 6 );
						}
						else
						{
#ifdef INLINE_DEBUG_TRANSFER
	debug << "\r\nCH#6 OVERHEAD";
#endif

							// need to make the transfer later
							// can't transfer and then wait on the receiving side, have wait, then transfer on the sending side
							// might be that PS1 is clearing out data quickly after starting transfer
							// SIF buffer is 8 qwords, so I'll try waiting 8*128*2=2048 PS1 bus cycles
							//BusyUntil_Cycle [ 5 ] = *_DebugCycleCount + c_ullSIFOverhead;
							
							// repeat transfer after wait (transfer overhead)
							SetNextEventCh( c_ullSIFOverhead, 6 );
						}
					
					}
					else
					{
						ChainTransfer_FromMemory ( 6 );
					}
					
					
					break;
				*/
				
				// the channels that probably have direction controlled on the device side are 0,2,3,
				
				// the channels that are always TO memory are 3, 5, 8
				case 3:
				//case 5:	// will pull this in later
				case 8:

#ifdef VERBOSE_CHAIN_TOMEM
					cout << "\nhps2x64 ALERT: DMA: attempted DESTINATION CHAIN transfer TO memory via DMA Channel#" << dec << iChannel << "\n";
					//cout << "\n*** UNIMPLEMENTED ***\n";
#endif
					
					ChainTransfer_FromMemory ( iChannel );
					break;
				
				// the channels that are always FROM memory are 0, 2, 4, 6, 9
				case 0:
				case 2:
				case 4:
				case 6:
				case 9:
				
					// check for unimplemented items for testing
#ifdef VERBOSE_CHAIN_FROMMEM
					if ( iChannel == 4 || iChannel == 9 )
					{
						cout << "\nhps2x64 ALERT: DMA: attempted CHAIN transfer FROM memory via DMA Channel#" << dec << iChannel << "\n";
					}
#endif
					
					// perform CHAIN transfer FROM memory
					ChainTransfer_FromMemory ( iChannel );
				
					break;
					
				// direction of dma transfer only matters for channels 1 and 7
				case 1:
				case 7:
						
					// check if this is going from memory or to memory
					switch ( pRegData [ iChannel ]->CHCR.DIR )
					{
						// to memory
						case 0:
							cout << "\nhps2x64 ALERT: DMA: attempted CHAIN transfer TO memory via DMA Channel#" << dec << iChannel << "\n";
							break;
						
						// from memory
						case 1:
							ChainTransfer_FromMemory ( iChannel );
							break;
							
					}	// switch for DMA CHAIN transfer direction
					
			}	// switch for channel number
			
			
			break;	// CHAIN transfer
			
		// Interleave transfer (Scratch Pad)
		case 2:
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; Interleave";
#endif

			if ( iChannel >= 8 )
			{
#ifdef VERBOSE_INTERLEAVE
				// Interleave mode transfers data in a more rectangular pattern to/from scratchpad
				cout << "\nhps2x64: ALERT: DMA: Attempting Interleave DMA transfer. DMA Channel#" << dec << iChannel << "\n";
#endif
				
				// perform interleave transfer
				InterleaveTransfer_FromMemory ( iChannel );
			}
			else
			{
				// invalid interleave transfer
				cout << "\nhps2x64: ALERT: DMA: INVALID Interleave DMA transfer. DMA Channel#" << dec << iChannel << "\n";
			}

			break;
			
		default:
#ifdef INLINE_DEBUG_TRANSFER
	debug << "; INVALID/ERROR";
#endif

			cout << "\nhps2x64: ALERT: DMA: INVALID DMA transfer mode. MOD=" << dec << pRegData [ iChannel ]->CHCR.MOD << "\n";
			break;
			
	}	// switch for DMA transfer type (normal, chain, interleave)
	
	// MFIFO //
	// check if the source channel (dma#8) has just written data for mfifo
	if ( ( DMARegs.CTRL.MFIFO > 1 ) && ( iChannel == 8 ) )
	{
		// if the drain channel is enabled, then attempt to run it
		if ( pRegData [ DMARegs.CTRL.MFIFO - 1 ]->CHCR.STR )
		{
			//Transfer ( DMARegs.CTRL.MFIFO - 1 );
			iLastChannel = iChannel;
			if ( GetNextActiveChannel () != iLastChannel )
			{
				CheckTransfer ();
			}
		}
	}
	
	// check if the drain channel has just written data for mfifo
	/*
	if ( ( CTRL_Reg.MFIFO > 1 ) && ( iChannel == c_iMfifoDrain_LUT [ CTRL_Reg.MFIFO ] ) )
	{
		// if the source channel is enabled, then attempt to run it
		if ( DmaCh [ 8 ].CHCR_Reg.STR )
		{
			Transfer ( 8 );
		}
	}
	*/
	
	// STALL CONTROL //
	// check if there is a stall destination channel with source channel
	// if the source channel has just performed a transfer
	if ( iChannel == c_iStallSource_LUT [ DMARegs.CTRL.STS ] )
	{
		if ( DMARegs.CTRL.STD && DMARegs.CTRL.STS )
		{
			// there is a stall destination channel set //
			
			// make sure it is not the same channel we are processing
			if ( iChannel != c_iStallDest_LUT [ DMARegs.CTRL.STD ] )
			{
				// if the drain channel is enabled, then run it
				if ( pRegData [ c_iStallDest_LUT [ DMARegs.CTRL.STD ] ]->CHCR.STR )
				{
					// retry the stall destination transfer
					//Transfer ( c_iStallDest_LUT [ DMARegs.CTRL.STD ] );
					iLastChannel = iChannel;
					if ( GetNextActiveChannel () != iLastChannel )
					{
						// also needs to return to handle any other active channels when done
						CheckTransfer ();
					}
				}
			}
		}
	}
}




u64 Dma::SPRout_DMA_Ready ()
{
	return 1;
}

u64 Dma::SPRin_DMA_Ready ()
{
	return 1;
}

// dma channel #8
u32 Dma::SPRout_DMA_Read ( u64* Data, u32 QuadwordCount )
{
#ifdef INLINE_DEBUG_SPR_OUT
	debug << " SPRout";
	debug << " CycleStealing=" << pDMARegs->CTRL.CycleStealMode;
#endif

	u64 *pSrcDataPtr64;
	u32 MADR;
	u32 iDrainChannel;
	u32 iQWRemaining;
	
	// note: the transfer possibly does not update SADR or MADR, but can update this later
	// note: transfer is possibly a one-shot transfer
	

	// it is possible that scratchpad address is actually something else, so check
	if ( ( ( pRegData [ 8 ]->SADR.Address >> 24 ) & 0x11 ) == 0x11 )
	{

		cout << "hps2x64: ALERT: DMA8 SADR does not have a ScratchPad Address!!! Address=" << hex << pRegData [ 8 ]->SADR.Address;
	}

#ifdef VERBOSE_DMA8_MADR
	// it is possible that scratchpad address is actually something else, so check
	if ( ( ( pRegData [ 8 ]->MADR.Address >> 24 ) & 0x11 ) == 0x11 )
	{
		cout << "hps2x64: ALERT: DMA8 MADR does not have a RAM Address!!! Address=" << hex << pRegData [ 8 ]->MADR.Address;
	}
#endif
	
	// get ptr into scratch pad
	pSrcDataPtr64 = & ( DataBus::_BUS->ScratchPad.b64 [ ( ( pRegData [ 8 ]->SADR.Address & DataBus::ScratchPad_Mask ) >> 3 ) & ~1 ] );
	
#ifdef INLINE_DEBUG_SPR_OUT_DATA
	debug << " Data:";
#endif

	for ( int i = 0; i < QuadwordCount; i++ )
	{
#ifdef INLINE_DEBUG_SPR_OUT_DATA
	debug << hex << pSrcDataPtr64[0] << " " << pSrcDataPtr64[1] << " ";
#endif

		*Data++ = *pSrcDataPtr64++;
		*Data++ = *pSrcDataPtr64++;
	}
	
#ifdef ENABLE_MEMORY_INVALIDATE
	DataBus::_BUS->InvalidateRange ( pRegData [ 8 ]->MADR.Value & DataBus::MainMemory_Mask, QuadwordCount << 2 );
#endif

	// update SADR
	pRegData [ 8 ]->SADR.Address += ( QuadwordCount << 4 );

	// return amount transferred
	return QuadwordCount;
}

// dma channel #9
u32 Dma::SPRin_DMA_Write ( u64* Data, u32 QuadwordCount )
{
#ifdef INLINE_DEBUG_SPR_IN
	debug << " SPRin";
	debug << " CycleStealing=" << pDMARegs->CTRL.CycleStealMode;
#endif

	u64 *pDstDataPtr64;
	
	// note: the transfer possibly does not update SADR or MADR, but can update this later
	// note: transfer is possibly a one-shot transfer
	
	// it is possible that scratchpad address is actually something else, so check
	if ( ( ( pRegData [ 9 ]->SADR.Address >> 24 ) & 0x11 ) == 0x11 )
	{
		cout << "hps2x64: ALERT: DMA9 SADR does not have a ScratchPad Address!!! Address=" << hex << pRegData [ 9 ]->SADR.Address;
	}

#ifdef VERBOSE_DMA9_MADR
	if ( ( ( pRegData [ 9 ]->MADR.Address >> 24 ) & 0x11 ) == 0x11 )
	{
		cout << "hps2x64: ALERT: DMA9 MADR does not have a RAM Address!!! Address=" << hex << pRegData [ 9 ]->MADR.Address;
	}
#endif
	
	// get ptr into scratch pad
	pDstDataPtr64 = & ( DataBus::_BUS->ScratchPad.b64 [ ( ( pRegData [ 9 ]->SADR.Address & DataBus::ScratchPad_Mask ) >> 3 ) & ~1 ] );
	
	for ( int i = 0; i < QuadwordCount; i++ )
	{
		*pDstDataPtr64++ = *Data++;
		*pDstDataPtr64++ = *Data++;
	}
	
#ifdef ENABLE_MEMORY_INVALIDATE_SCRATCHPAD
	DataBus::_BUS->InvalidateRange ( pRegData [ 9 ]->SADR.Value & DataBus::ScratchPad_Mask, QuadwordCount << 2 );
#endif

	// update SADR
	pRegData [ 9 ]->SADR.Address += ( QuadwordCount << 4 );
	

	// return amount transferred
	return QuadwordCount;
}



//const char* DmaChannelLogText [ 7 ] = { "DMA0_Log.txt", "DMA1_Log.txt", "DMA2_Log.txt", "DMA3_Log.txt", "DMA4_Log.txt", "DMA5_Log.txt", "DMA6_Log.txt" };

DmaChannel::DmaChannel ()
{
	// set the dma channel number
	Number = Count++;
	
	Reset ();
}


void DmaChannel::Reset ()
{
	// initialize MADR, BCR, & CHCR
	//MADR_Reg.Value = 0;
	//QWC_Reg.Value = 0;
	//CHCR_Reg.Value = 0;
	memset ( this, 0, sizeof(DmaChannel) );
}





void Dma::SetNextEventCh ( u64 Cycles, u32 Channel )
{
#ifdef INLINE_DEBUG_NEXTEVENT
	debug << "\r\nDma::SetNextEventCh; CycleCount=" << dec << *_DebugCycleCount;
	debug << " (before) Cycles=" << Cycles << " Channel=" << Channel;
#endif

	NextEventCh_Cycle [ Channel ] = Cycles + *_DebugCycleCount;
	
	Update_NextEventCycle ();
}


void Dma::SetNextEventCh_Cycle ( u64 Cycle, u32 Channel )
{
#ifdef INLINE_DEBUG_NEXTEVENT
	debug << "\r\nDma::SetNextEventCh_Cycle; CycleCount=" << dec << *_DebugCycleCount;
	debug << " (before) Cycle=" << Cycle << " Channel=" << Channel;
#endif

	NextEventCh_Cycle [ Channel ] = Cycle;
	
	//cout << "\nTEST: Channel=" << dec << Channel << " NextEventCh_Cycle [ Channel ]=" << NextEventCh_Cycle [ Channel ] << " Cycle=" << Cycle;
	
	Update_NextEventCycle ();
}

void Dma::Update_NextEventCycle ()
{
#ifdef INLINE_DEBUG_NEXTEVENT
	debug << "\r\nDma::Update_NextEventCycle; Cycle=" << dec << *_DebugCycleCount;
	debug << " (before) NextEvent_Cycle=" << NextEvent_Cycle << " NextSystemEvent=" << *_NextSystemEvent;
#endif

	NextEvent_Cycle = -1LL;
	
	for ( int i = 0; i < NumberOfChannels; i++ )
	{
		//if ( NextEventCh_Cycle [ i ] > *_DebugCycleCount && ( NextEventCh_Cycle [ i ] < NextEvent_Cycle || NextEvent_Cycle <= *_DebugCycleCount ) )
		//if ( NextEventCh_Cycle [ i ] < NextEvent_Cycle )
		if ( ( NextEventCh_Cycle [ i ] > *_DebugCycleCount ) && ( NextEventCh_Cycle [ i ] < NextEvent_Cycle ) )
		{
			// the next event is the next event for device
			NextEvent_Cycle = NextEventCh_Cycle [ i ];
		}
	}

	//if ( NextEvent_Cycle > *_DebugCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_DebugCycleCount ) )
	if ( NextEvent_Cycle < *_NextSystemEvent )
	{
		*_NextSystemEvent = NextEvent_Cycle;
		*_NextEventIdx = NextEvent_Idx;
	}
	
	//cout << "\nTEST: dma1 next event cycle=" << dec << NextEventCh_Cycle [ 1 ];
	//cout << "\nTEST: dma next event cycle=" << dec << NextEvent_Cycle;
	
#ifdef INLINE_DEBUG_NEXTEVENT
	debug << " (after) NextEvent_Cycle=" << NextEvent_Cycle << " NextSystemEvent=" << *_NextSystemEvent;
#endif
}



u64* Dma::GetMemoryPtr ( u32 Address )
{
	if (Address < DataBus::MainMemory_Size)
	{
		// otherwise, it is a main memory address
		return &(DataBus::_BUS->MainMemory.b64[((Address & DataBus::MainMemory_Mask) >> 3) & ~1]);
	}
	
	if (Address < 0x10000000)
	{
		// pcsx2 treats this as zeros for read and write?
		cout << "\n***PS2 DMA ADDRESS SHOULD BE ZEROS FOR R/W? ADDR=" << hex << Address;
	}

	if (Address < 0x10004000)
	{
		// pcsx2 treats this as scratch pad area
		return &(DataBus::_BUS->ScratchPad.b64[((Address & DataBus::ScratchPad_Mask) >> 3) & ~1]);
	}

	if ((Address & 0x70000000) == 0x70000000)
	{
		// treat as scratchpad too
		return &(DataBus::_BUS->ScratchPad.b64[((Address & DataBus::ScratchPad_Mask) >> 3) & ~1]);
	}

	if ( Address >> 31 )
	{
		// if SPR bit is set, then it is an SPR Memory address
		return & ( DataBus::_BUS->ScratchPad.b64 [ ( ( Address & DataBus::ScratchPad_Mask ) >> 3 ) & ~1 ] );
	}

	
	if ( ( Address >> 24 ) == 0x11 )
	{
		// in this case, it must be a VU memory address
		if ( Address < 0x11004000 )
		{
			// micro mem 0 address
			return & ( DataBus::_BUS->MicroMem0 [ ( ( Address & DataBus::MicroMem0_Mask ) >> 3 ) & ~1 ] );
		}
		else if ( Address < 0x11008000 )
		{
			// vu mem 0 //
			Address -= 0x11004000;
			return & ( DataBus::_BUS->VuMem0 [ ( ( Address & DataBus::MicroMem0_Mask ) >> 3 ) & ~1 ] );
		}
		else if ( Address < 0x1100c000 )
		{
			// micro mem 1 //
			Address -= 0x10008000;
			return & ( DataBus::_BUS->MicroMem1 [ ( ( Address & DataBus::MicroMem1_Mask ) >> 3 ) & ~1 ] );
		}
		
		// vu mem 1 //
		Address -= 0x1100c000;
		return & ( DataBus::_BUS->VuMem1 [ ( ( Address & DataBus::MicroMem1_Mask ) >> 3 ) & ~1 ] );
	}


	// invalid address ?? //
	
	cout << "\n***hps2x64: ALERT: unknown PS2 DMA Address. using main memory. Addr=" << hex << Address;

	// otherwise, for now treat it as a main memory address
	return & ( DataBus::_BUS->MainMemory.b64 [ ( ( Address & DataBus::MainMemory_Mask ) >> 3 ) & ~1 ] );
}




void Dma::DebugWindow_Enable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	static constexpr char* DebugWindow_Caption = "PS2 DMA Debug Window";
	static constexpr int DebugWindow_X = 10;
	static constexpr int DebugWindow_Y = 10;
	static constexpr int DebugWindow_Width = 250;
	static constexpr int DebugWindow_Height = 300;
	
	static constexpr int DMAList_X = 0;
	static constexpr int DMAList_Y = 0;
	static constexpr int DMAList_Width = 220;
	static constexpr int DMAList_Height = 250;
	
	int i;
	stringstream ss;
	
	if ( !DebugWindow_Enabled )
	{
		// create the main debug window
		DebugWindow = new WindowClass::Window ();
		DebugWindow->Create ( DebugWindow_Caption, DebugWindow_X, DebugWindow_Y, DebugWindow_Width, DebugWindow_Height );
		DebugWindow->DisableCloseButton ();
		
		// create "value lists"
		DMA_ValueList = new DebugValueList<u32> ();
		DMA_ValueList->Create ( DebugWindow, DMAList_X, DMAList_Y, DMAList_Width, DMAList_Height );
		
		DMA_ValueList->AddVariable ( "DMA_CTRL", &( pDMARegs->CTRL.Value ) );
		DMA_ValueList->AddVariable ( "DMA_STAT", &( pDMARegs->STAT.Value ) );
		DMA_ValueList->AddVariable ( "DMA_PCR", &( pDMARegs->PCR.Value ) );

		for ( i = 0; i < NumberOfChannels; i++ )
		{
			ss.str ("");
			ss << "DMA" << i << "_MADR";
			DMA_ValueList->AddVariable ( ss.str().c_str(), &( pRegData [ i ]->MADR.Value ) );
			
			ss.str ("");
			ss << "DMA" << i << "_BCR";
			DMA_ValueList->AddVariable ( ss.str().c_str(), &( pRegData [ i ]->QWC.Value ) );
			
			ss.str ("");
			ss << "DMA" << i << "_CHCR";
			DMA_ValueList->AddVariable ( ss.str().c_str(), &( pRegData [ i ]->CHCR.Value ) );
		}
		
		// add start and end addresses for dma transfers
		//DMA_ValueList->AddVariable ( "StartA", &( _DMA->StartA ) );
		//DMA_ValueList->AddVariable ( "EndA", &( _DMA->EndA ) );
		
		// add primitive count and frame count here for now
		//DMA_ValueList->AddVariable ( "PCount", &( _GPU->Primitive_Count ) );
		//DMA_ValueList->AddVariable ( "FCount", &( _GPU->Frame_Count ) );
		
		DebugWindow_Enabled = true;
		
		// update the value lists
		DebugWindow_Update ();
	}

#endif

}

void Dma::DebugWindow_Disable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	if ( DebugWindow_Enabled )
	{
		delete DebugWindow;
		delete DMA_ValueList;
	
		// disable debug window
		DebugWindow_Enabled = false;
	}
	
#endif

}

void Dma::DebugWindow_Update ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	if ( DebugWindow_Enabled )
	{
		DMA_ValueList->Update();
	}
	
#endif

}


}



