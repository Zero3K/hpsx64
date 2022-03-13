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



#ifndef _PS1_CD_H
#define _PS1_CD_H


#include "types.h"
#include "Debug.h"
#include "DebugValueList.h"

#include "GeneralEmuDataStructures.h"

#include "cdimage.h"

#include "PS1_Intc.h"

#include "adpcm.h"

namespace Playstation1
{


	class CD
	{

		static Debug::Log debug;
	
	public:
		static CD *_CD;
		
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the dma registers start at
		static const long Regs_Start = 0x1f801800;
		
		// where the dma registers end at
		static const long Regs_End = 0x1f801803;
	
		// distance between numbered groups of registers for dma
		static const long Reg_Size = 0x1;
		
		static const u64 c_iPendingInterruptWait_Cycles = 1024;
		
		// the index for the callback for next event
		u32 NextEvent_Idx;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;
		
		// cycle that the next read event will happen at for this device
		u64 NextRead_Cycle;

		// cycle that the next command should start at
		u64 NextStart_Cycle;
		
		// cycle that the next non-read action will happen at for this device
		u64 NextAction_Cycle;

		// set the next event cycle for read
		void SetNextEventCycle ();
		
		// set cycle offset for next event
		//void SetNextIntCycle ( u64 Cycles );
		void SetNextStartCycle ( u64 Cycles );
		void SetNextReadCycle ( u64 Cycles );
		void SetNextActionCycle ( u64 Cycles );

		// set exact cycle# of next event
		void SetNextStartCycle_Cycle ( u64 ullCycle );
		void SetNextReadCycle_Cycle ( u64 ullCycle );
		void SetNextActionCycle_Cycle ( u64 ullCycle );

		

		u64 BusyCycles;
		
		// need to know if a read is in progress
		u32 isReading;
		
		// need to know if lid is open or closed
		u32 isLidOpen;
		
		static u32 Read ( u32 Address );
		static void Write ( u32 Address, u32 Data, u32 Mask );
		
		// returns true if device is ready for a read from DMA
		static u64 DMA_ReadyForRead ( void );
		static u64 DMA_ReadyForWrite ( void );
		
		// need to call this when dma transfer is starting
		void DMA_Start ();
		
		// need to call this when dma transfer is ending
		void DMA_End ();

		// allows dma read or write - probably only dma_read is needed here
		void DMA_Read ( u32* Data, int ByteReadCount );
		static u32 DMA_ReadBlock ( u32* Data, u32 BS, u32 BA = 0 );
		void DMA_Write ( u32* Data, int ByteWriteCount );
		
		void Reset ();
		
		void Start ();

		void Run ();
		
		void SendCommand ( u8 CommandToSend );
		
		// call this when lid is opened/closed
		void Event_LidOpen ();
		void Event_LidClose ();
		
		static const u32 c_ParameterBuf_Size = 16;
		static const u32 c_ParameterBuf_Mask = c_ParameterBuf_Size - 1;
		u32 ParameterBuf_Size;
		
		u8 ParameterBuf [ c_ParameterBuf_Size ];
		inline void Load_ParameterBuf ( u8* ArgsBuf ) { for ( int i = 0; i < c_ParameterBuf_Size; i++ ) ParameterBuf [ i ] = ArgsBuf [ i ]; }
		inline u8 Get_Parameter ( int index ) { return ParameterBuf [ index & c_ParameterBuf_Mask ]; }

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////
		
		
		// these are the values that go in CDREG3 after the command is executed and interrupt is generated
		// always 3 for command complete
		//static const u8 InterruptPrimaryResults [] = { 

		// *todo* will fill this in later
		//static const u8 InterruptSecondaryResults [] = { 
		
		
		// Write: 0-to send command;1-to get the result
		// Read: see below
		static const long CD_REG0 = 0x1f801800;
		
		struct REG0_Read_Format
		{
			union
			{
				struct
				{
					// 0-REG1 command send;1-REG1 data read
					// bit 0
					//u8 CommandSendDataRecv : 1;
					
					//0-data transfer finished;1-data transfer ready/in progress
					// bit 1
					//u8 TransferInProgress : 1;
					
					// *** BITS 0-2 are r/w; BITS 3-7 are read-only *** //
					
					// bits 0-1 - Port Mode
					// 0: Mode 0; 1: Mode 1; 2: Mode2; 3: Mode 3
					u8 PortMode : 2;
					
					// bit 2 - Unknown (always zero)
					// bit 2 - cd-xa buffer empty flag (0- empty, 1-NOT empty)
					u8 CDXA_BufferNotEmpty : 1;
					
					// bit 3 - Parameter FIFO empty (triggered before writing 1st byte)
					// 0: Not Empty; 1: Empty
					u8 ParamFifoEmpty : 1;
					
					// bit 4 - Parameter FIFO full (triggered after writing 16 bytes)
					// 0: Full; 1: Not Full
					u8 ParamFifoFull : 1;
					
					// unknown
					// bits 2-4
					//u8 Unused : 3;
					
					// bit 5 - Response/Result FIFO empty (triggered after reading last byte)
					// 0: Empty; 1: Not Empty
					u8 ResultReady : 1;
					
					// bit 5 - result ready
					// 0: result NOT ready; 1: result IS ready
					//u8 ResultReady : 1;
					
					// bit 6 - Data FIFO empty
					// 0: empty; 1: not empty
					u8 DmaReady : 1;
					
					// bit 6 - dma ready
					// 0: dma NOT ready; 1: dma IS ready
					//u8 DmaReady : 1;
					
					// bit 7 - Command/Parameter transmission busy
					// 0: NOT busy; 1: busy
					u8 CommandProcessing : 1;
					
					// 1-command being processed; 0: command NOT being processed
					// bit 7
					//u8 CommandProcessing : 1;
				};
				
				u8 Value;
			};
		};
		
		u8 CD_REG0_Reg;


		enum
		{
			CDREG0_READ_DATAREAD = ( 1 << 0 ),
			CDREG0_READ_TRANSFERINPROGRESS = ( 1 << 1 ),
			CDREG0_READ_STAT_RESULTREADY = ( 1 << 5 ),
			CDREG0_READ_STAT_DMAREADY = ( 1 << 6 ),
			CDREG0_READ_STAT_COMMANDPROCESSING = ( 1 << 7 )
		};


		// write: command
		// read: results
		static const long CD_REG1 = 0x1f801801;

		u8 CD_REG1_Reg;
		
		//u8 PendingCommand;
		s32 Command;
		u8 Status;
		
		
		// CD Registers
		s32 REG_ModeStatus, REG_Command, REG_Request, REG_InterruptEnable, REG_InterruptFlag;
		
		void UpdateREG_ModeStatus ();
		void UpdateREG_InterruptFlag ( u32 IntOk = false );
		
		// check for an interrupt or if interrupt has been acknowledged
		void Check_Interrupt ();
		
		// check if there is a command waiting to be sent, and if there is then send it
		void Check_Command ();
		
		// some commands can be executing while cd is currently executing a separate "read command"
		u8 ReadCommand;
		u64 BusyCycles_Read;
		
		u32 ReadMode, ReadMode_Offset;
		u32 SectorDataIndex;
		u32 isReadingFirstSector;
		u32 isReadingFirstAudioSector;
		u64 ReadCount;

		
		u8 InterruptReason;
		
		// interrupt registers
		u8 InterruptEnableRegister;
		u8 InterruptFlagRegister;
		
		// audio volume pending/applied
		u8 PendingVolume_CDLeftToSPULeft;
		u8 PendingVolume_CDLeftToSPURight;
		u8 PendingVolume_CDRightToSPULeft;
		u8 PendingVolume_CDRightToSPURight;
		u8 AppliedVolume_CDLeftToSPULeft;
		u8 AppliedVolume_CDLeftToSPURight;
		u8 AppliedVolume_CDRightToSPULeft;
		u8 AppliedVolume_CDRightToSPURight;
		
		// if a command is executing, then you cannot send another one
		u32 isCommandExecuting;
		u32 isCommandStartInterruptRequested;
		u32 isCommandStart;
		
		// the results to be read
		u32 ResultActive;
		u32 ResultIndex;
		u32 ResultSize;
		u8 ResultInterrupt;
		u8 Result [ 16 ];
		
		// queued results buffer
		u32 QueuedResultActive;
		u32 QueuedResultSize;
		u8 QueuedInterrupt;
		u8 QueuedResult [ 8 ];
		
		static const u32 c_ResponseBuf_Size = 16;
		static const u32 c_ResponseBuf_Mask = c_ResponseBuf_Size - 1;
		
		u32 LastDataBufferSize;
		u32 LastDataBufferIndex;
		
		class InterruptQ_Entry
		{
		public:
		
			u32 InterruptPending;
			u32 InterruptSent;
			u32 ResponseSent;
			u32 DataSent;
			u32 InterruptEnabled;
			u32 InterruptReason;
			u32 ResponseSize;
			u8 ResponseBuf [ c_ResponseBuf_Size ];
			
			u32 isDataReadyInterrupt;
			u64 SectorDataQ_Index;
			u32 SectorDataQ_Size;
			u32 SectorDataQ_Offset;
			
			//u64 AssociatedDataBuffer_Index;
			
			void Clear ();
			void Set ( u32 Reason, u8* Data, u32 Size );
		};
		
		InterruptQ_Entry CurrentInt;
		InterruptQ_Entry PrimaryInt;
		InterruptQ_Entry PendingInt;
		
		// I actually need to make the interrupt queue like 4 deep possibly
		static const u32 c_InterruptQueueDepth = 4;
		
		static const u32 c_InterruptQ_Mask = c_InterruptQueueDepth - 1;

		u32 InterruptQ_Index, ResponseBuf_Index, ResponseBuf_Size;
		u8 Current_ResponseBuf [ 16 ];
		InterruptQ_Entry InterruptQ [ c_InterruptQueueDepth ];
		
		// will use data structures for this
		Emulator::DataStructures::FIFO<InterruptQ_Entry,4> IntQueue;
		Emulator::DataStructures::FIFO<InterruptQ_Entry,4> IntQueue_Read;

		// sector data queue. PS1 does not have a way to detect buffer overruns
		static const int c_iMaxQueuedSectors = 8;
		static const int c_iMaxQueuedSectors_Mask = c_iMaxQueuedSectors - 1;
		u64 SectorDataQ_Index;
		u8 SectorDataQ_Buffer [ c_iMaxQueuedSectors ];	//[ DiskImage::CDImage::c_SectorSize * c_iMaxQueuedSectors ];
		u32 SectorDataQ_Active [ c_iMaxQueuedSectors ];
		u64 SectorDataQ_FullIndex [ c_iMaxQueuedSectors ];
		
		u32 SectorDataQ_BufferSize [ c_iMaxQueuedSectors ];
		u32 SectorDataQ_BufferOffset [ c_iMaxQueuedSectors ];
		
		class SectorDataQ_Entry
		{
			u32 Size;
			u8 Interrupt;
			u8 SectorDataQ_Buffer [ DiskImage::CDImage::c_SectorSize ];
		};
		
		Emulator::DataStructures::FIFO<SectorDataQ_Entry,8> SectorQueue;
		
		static const u32 c_NumberOfDataBuffers = 8;
		static const u32 c_DataBuffers_Mask = c_NumberOfDataBuffers - 1;
		
		static const u32 c_DataBuf_MaxSize = 2352;
		u32 DataBuffer_Index, DataBuffer_Size;
		u32 DataBuffer_Mask;
		//u8* DataBuffer;
		//u8* RawDataBuffer;
		
		// can't use pointers or else it breaks the save states
		u32 DataBuffer_Offset;
		
		// need to know what the current track playing is
		u32 CurrentTrack;
		
		// need to know where to put next interrupt in the queue
		u32 QueueIndex;

		// says whether the queued buffer has anything in it or not
		u32 QueuedBufferActive [ c_InterruptQueueDepth ];
		
		// the interrupt associated with the queued buffer
		u8 QueuedBufferInterrupt [ c_InterruptQueueDepth ];
		
		// also need to know the amount of bytes int the buffer
		u32 QueuedBufferSize [ c_InterruptQueueDepth ];
		
		union t_QueuedResultBuffers
		{
			u8 b8 [ 8 * c_InterruptQueueDepth ];
			u64 b64 [ c_InterruptQueueDepth ];
		};
		
		t_QueuedResultBuffers QueuedResultBuffers;
		
		// there is no requirement to seek before reading, so if they did not seek first you need to seek first
		u32 hasSeeked;
		
		// need to know if reading is paused
		u32 isPaused;
		
		// spu read buffer
		// each element is a u32 since it holds a 16-bit sample for each left and right
		static const u32 c_iSpuBuffer_Size = 32768;
		static const u32 c_iSpuBuffer_Mask = c_iSpuBuffer_Size - 1;
		u64 SpuBuffer_ReadIndex, SpuBuffer_WriteIndex;
		u32 SpuBuffer [ c_iSpuBuffer_Size ];
		
		u32 Apply_CDDA_Volume ( u32 LRSample );
		u32 Spu_ReadNextSample ();

		// checks if sector is a data sector rather than an audio sector
		// returns 1 if a data sector, 0 if not a data sector
		static int isDataSector ( unsigned char* data );
		
		// checks if cd is playing
		// returns false if not playing anyting, true otherwise
		static int isPlaying ();
		
		// this buffer needs to be a circular one for sample interpolation
		// for stereo: 18 blocks * 8 blocks/block * 28 samples/block * 2 bytes per sample * 1 sample/channel = 4032
		// for mono: 18 blocks * 8 blocks/block * 28 samples/block * 2 bytes per sample * 1 sample/channel = 4032*2 = 8064
		static const int c_iXADecode_Size = 32768;
		s32 TempXA_Buffer [ c_iXADecode_Size * 2 ];
		
		// circular buffers for sample interpolation
		s32 BufferL [ 4 ];
		s32 BufferR [ 4 ];
		inline void StartInterpolation () { ((u64*)BufferL) [ 0 ] = 0; ((u64*)BufferL) [ 1 ] = 0; ((u64*)BufferR) [ 0 ] = 0; ((u64*)BufferR) [ 1 ] = 0; }
		
		adpcm_decoder LeftSpeaker, RightSpeaker;
		u64 SampleInterpolation;
		static const u64 c_iSampleUpdate = ( 37800ULL << 32 ) / 44100ULL;
		static const u64 c_iSampleUpdate2 = c_iSampleUpdate >> 1;
		
		// returns the sample rate
		//u32 DecodeXA_Sector ( short* Output, char* FullSectorData );
		u32 DecodeXA_Sector ( long* Output, char* FullSectorData );
		
		// process xa sector
		void Process_XASector ( u8* pSectorDataBuffer );

		// perform read command
		void Process_Read ();
		
		
		// get the maximum chan value per coding info
		static int Get_MaxChan ( u8 CodingInfo, int DoubleSpeed );
		

		void DeactivateBuffer ();
		void FlushBuffer ();
		void FlushIRQ ();

		enum
		{
			CDREG1_READ_STAT_ERROR = ( 1 << 0 ),
			CDREG1_READ_STAT_STANDBY = ( 1 << 1 ),
			CDREG1_READ_STAT_SEEKERROR = ( 1 << 2 ),
			CDREG1_READ_STAT_ERROR3 = ( 1 << 3 ),
			CDREG1_READ_STAT_SHELLOPEN = ( 1 << 4 ),
			CDREG1_READ_STAT_READ = ( 1 << 5 ),
			CDREG1_READ_STAT_SEEK = ( 1 << 6 ),
			CDREG1_READ_STAT_PLAY = ( 1 << 7 )
		};

		
		enum
		{
			// primary commands
			CDREG1_CMD_SYNC = 0x00,
			CDREG1_CMD_NOP = 0x01,
			CDREG1_CMD_SETLOC = 0x02,
			CDREG1_CMD_PLAY = 0x03,
			CDREG1_CMD_FORWARD = 0x04,
			CDREG1_CMD_BACKWARD = 0x05,
			CDREG1_CMD_READN = 0x06,
			CDREG1_CMD_STANDBY = 0x07,
			CDREG1_CMD_STOP = 0x08,
			CDREG1_CMD_PAUSE = 0x09,
			CDREG1_CMD_INIT = 0x0a,
			CDREG1_CMD_MUTE = 0x0b,
			CDREG1_CMD_DEMUTE = 0x0c,
			CDREG1_CMD_SETFILTER = 0x0d,
			CDREG1_CMD_SETMODE = 0x0e,
			CDREG1_CMD_GETPARAM = 0x0f,
			CDREG1_CMD_GETLOCL = 0x10,
			CDREG1_CMD_GETLOCP = 0x11,
			CDREG1_CMD_READT = 0x12,
			CDREG1_CMD_GETTN = 0x13,
			CDREG1_CMD_GETTD = 0x14,
			CDREG1_CMD_SEEKL = 0x15,
			CDREG1_CMD_SEEKP = 0x16,
			CDREG1_CMD_SETCLOCK = 0x17,
			CDREG1_CMD_GETCLOCK = 0x18,
			CDREG1_CMD_TEST = 0x19,
			CDREG1_CMD_ID = 0x1a,
			CDREG1_CMD_READS = 0x1b,
			CDREG1_CMD_RESET = 0x1c,
			CDREG1_CMD_UNKNOWN0 = 0x1d,
			CDREG1_CMD_READTOC = 0x1e,
			CDREG1_CMD_UNKNOWN1 = 0x1f,
			
			// secondary commands
			CDREG1_CMD_PLAY_2 = 0x23,
			CDREG1_CMD_FORWARD_2 = 0x24,
			CDREG1_CMD_BACKWARD_2 = 0x25,
			CDREG1_CMD_READN_2 = 0x26,
			CDREG1_CMD_STANDBY_2 = 0x27,
			CDREG1_CMD_STOP_2 = 0x28,
			CDREG1_CMD_PAUSE_2 = 0x29,
			CDREG1_CMD_INIT_2 = 0x2a,
			CDREG1_CMD_READT_2 = 0x32,
			CDREG1_CMD_SEEKL_2 = 0x35,
			CDREG1_CMD_SEEKP_2 = 0x36,
			CDREG1_CMD_ID_2 = 0x3a,
			CDREG1_CMD_READS_2 = 0x3b,
			CDREG1_CMD_READTOC_2 = 0x3e,
			
			// lid commands - these are "read" commands
			CDREG1_CMD_LID_1 = 0x41,
			CDREG1_CMD_LID_2 = 0x42,
			CDREG1_CMD_LID_3 = 0x43,
			CDREG1_CMD_LID_4 = 0x44,
			CDREG1_CMD_LID_5 = 0x45,
			
			CDREG1_CMD_READN_SEEK = 0x56,
			CDREG1_CMD_READS_SEEK = 0x5b,
			
			CDREG1_CMD_CHECK_INT = 0x61
		};

		
		
		// write: send arguments
		// write: 7-flush arg buffer?
		static const long CD_REG2 = 0x1f801802;

		u8 CD_REG2_Reg;
		
		// commands can require arguments before they are executed
		static const u32 c_ArgumentBuf_Size = 16;
		u32 ArgumentsIndex;
		u32 ArgumentsSize;
		u8 Arguments [ c_ArgumentBuf_Size ];
		
		
		// write: 7-flush irq
		// read: low nibble-interrupt status; high nibble: ?
		static const long CD_REG3 = 0x1f801803;
		
		u8 CD_REG3_Reg;
		
		struct REG3_Read_Format
		{
			union
			{
				struct
				{
					// interrupt status
					// 0-No interrupt;1-Data ready;2-Acknowledge/command complete;3-Complete/Acknowledge;4-End/end of data detected;5-DiskError/Error detected
					// bits 0-3
					u8 LowNibble : 4;
					
					u8 Unknown : 4;
				};
			};
		};
		
		// dataend is used when streaming cd-da or xa sound
		enum
		{
			CDREG3_READ_INTR_NONE = 0,
			CDREG3_READ_INTR_DATAREADY = 1,
			CDREG3_READ_INTR_CMDCOMPLETE = 2,
			CDREG3_READ_INTR_ACK = 3,
			CDREG3_READ_INTR_DATAEND = 4,
			CDREG3_READ_INTR_DISKERROR = 5
		};
		
		struct MODE_Format
		{
			union
			{
				struct
				{
					// bit 0 - M_CDDA - 0: cd-da off; 1: cd-da on
					u8 CDDA : 1;
					
					// bit 1 - M_AutoPause - 0: Auto Pause off; 1: auto pause on
					u8 AutoPause : 1;
					
					// bit 2 - M_Report - 0: Report off; 1: Report on
					u8 Report : 1;
					
					// bit 3 - M_SF - 0: channel off; 1: channel on
					u8 SF : 1;
					
					// bit 4 - M_Size2 - 0: ----; 1: 2328 byte
					// bit 4 - Ignore?? - 0: normal; 1: ignore??
					//u8 Size2 : 1;
					u8 Ignore : 1;
					
					// bit 5 - M_Size - 0: 2048 byte (data only); 1: 2340 byte (all except sync bytes)
					u8 Size : 1;
					
					// bit 6 - M_Strsnd - 0: adpcm off; 1: adpcm on
					u8 Strsnd : 1;
					
					// bit 7 - M_Speed - 0: normal speed; 1: double speed
					u8 Speed : 1;
				};
				
				u8 Value;
			};
		};
		
		MODE_Format MODE;
		
		// this is for autopause
		u32 CurrentSector_Number, NextTrackSector_Number;

		// this will hold the data for the last read sector
		// *note* the buffer is 4096 because when it is 2352, for some reason only 2048 bytes get read from an fread, so I read 4096 instead
		// switched from using fread
		// buffer is actually 128 KB on PS1
		static const u64 c_BufferSize = 131072;
		static const u32 c_SectorSize = 2352;
		static const u64 c_BufferMask = 0x1ffff;
		volatile bool isSectorDataReady;
		u64 DataReadIndex;
		u64 DataWriteIndex;
		//u8 SectorDataBuffer [ c_SectorSize ];
		//u8 DataBuffer [ c_SectorSize ];
		//u8 SubBuffer [ DiskImage::CDImage::c_SubChannelSizePerSector ];
		
		u64 SectorDataBuffer_Index;
		
		// need to store 13th through 20th bytes for GetLocL
		// need to capture sync bytes too since it is possible for GetLocL to fail 8 + 12 = 20 bytes
		//u8 GetLocL_Data [ 20 ];
		
		u64 DataBuffer_SlotNumber;
		
		// the region is important
		//char Region;
		s32 Region;
		
		// cd images are not longer static to allow it to be saved with the save state
		//DiskImage::CDImage game_cd_image;
		//DiskImage::CDImage audio_cd_image;
		u32 isGameCD;
		DiskImage::CDImage cd_image;
		
		// this should be local
		//u8* pSectorDataBuffer;


#ifdef PS2_COMPILE
		u8 DecryptSetting;
		u8 XorKey;
		u32 DVDSectorNumber;
#endif
		

		// constructor
		CD ();
		
		void OutputCurrentSector ();
		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		static u64* _SystemCycleCount;
		static u32 *_NextEventIdx;
		
		int DebugCount;

		
		static void sRun () { _CD->Run (); }
		static void Set_EventCallback ( funcVoid2 UpdateEvent_CB ) { _CD->NextEvent_Idx = UpdateEvent_CB ( sRun ); };
		
		
		// for interrupt call back
		static funcVoid UpdateInterrupts;
		static void Set_IntCallback ( funcVoid UpdateInt_CB ) { UpdateInterrupts = UpdateInt_CB; };
		
		
		static const u32 c_InterruptBit = 2;
		
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
		
		inline static void SetInterrupt ()
		{
			//*_Intc_Master |= ( 1 << c_InterruptBit );
			*_Intc_Stat |= ( 1 << c_InterruptBit );
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}
		
		inline static void ClearInterrupt ()
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

		
	private:
	
		// arguments
		
		// 0x2 - SetLoc
		u8 AMin, ASec, AFrac;
		u8 SetLoc_Min, SetLoc_Sec, SetLoc_Frac;
		
		// relative
		u8 Min, Sec, Frac;
		
		// 0x3 - Play
		u8 Track;
		
		// 0xb and 0xc - mute and demute
		u32 Mute;
		
		// 0xd - SetFilter
		u8 Filter_File, Filter_Chan;
		u8 DataFilter_File, DataFilter_Chan;
		u8 isFilterSet;
		
		// 0xe - SetMode
		u8 AMode;
		
		// 0x12 - ReadT
		u8 H0x1;
		
		// 0x14 - GetTD
		//u8 Track;
		
		// 0x19 - Test
		u8 Num;
		
		// 0x11 - GetLocP
		u8 GetLocP_Index, GetLocP_Track, GetLocP_Min, GetLocP_Sec, GetLocP_Frac, GetLocP_AMin, GetLocP_ASec, GetLocP_AFrac;
		
		
		// data for last read sector
		u8 LRS_File, LRS_Mode, LRS_AMin, LRS_ASec, LRS_AFrac, LRS_Min, LRS_Sec, LRS_Frac;
		u8 LRS_Min0, LRS_Sec0, LRS_Frac0;
		
		// for processing sectors
		u8 Disk_NextChan, Disk_MaxChan;
		
		// this keeps track of last track number for setloc/seek commands
		//u32 CurrentTrackNumber;

	
	
		// *** TODO *** need to use a stack to enqueue interrupts, and then need to disable interrupts while a command is in progress
	
		//void SetResults_CdReg1 ( u8* Data, u32 Size, u8 Interrupt );
		void EnqueueInterrupt ( u8* Data, u32 Size, u8 Interrupt );
		void EnqueueInterrupt_Read ( u8* Data, u32 Size, u8 Interrupt );
		u8 GetResult_CdReg1 ();
		
		
		// returns true if report created, false otherwise
		bool Get_ReportData ( u8* TempBuffer );
		
		static void Inc_DiskPosition ( u8& tMin, u8& tSec, u8& tFrac );

		static u8 ConvertBCDToDec ( u8 BCD );
		static u8 ConvertDecToBCD ( u8 Dec );

public:		
		// object debug stuff
		// Enable/Disable debug window for object
		// Windows API specific code is in here
		static bool DebugWindow_Enabled;
		static WindowClass::Window *DebugWindow;
		static DebugValueList<u32> *ValueList;
		static void DebugWindow_Enable ();
		static void DebugWindow_Disable ();
		static void DebugWindow_Update ();
	};
	
};


#endif

