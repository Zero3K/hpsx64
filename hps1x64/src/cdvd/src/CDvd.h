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


#ifndef _PS1_CDVD_H_
#define _PS1_CDVD_H_

#include "types.h"
#include "Debug.h"


#include "PS1_Intc.h"

namespace Playstation1
{

	class CDVD
	{
	
		static Debug::Log debug;
		static Debug::Log debugRead;
	
	public:
	
		static CDVD *_CDVD;
		
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the dma registers start at
		static const long Regs_Start = 0x1f402000;
		
		// where the dma registers end at
		static const long Regs_End = 0x1f402fff;
	
		// distance between numbered groups of registers
		static const long Reg_Size = 0x1;
		
		
		// next event index for callback
		u32 NextEvent_Idx;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;
		
		static u32 Read ( u32 Address );
		static void Write ( u32 Address, u32 Data, u32 Mask );
		void DMA_Read ( u32* Data, int ByteReadCount );
		void DMA_Write ( u32* Data, int ByteWriteCount );
		
		void Start ();
		
		void Reset ();
		
		void Run ();

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////

		
		// constructor
		CDVD ();
		
		
		void Process_MechaConCommand ( u8 Command );
		void Process_SCommand ( u8 Command );
		void Process_NCommand ( u8 Command );
		
		
		static const char c_cRegion_JAP [ 8 ];
		static const char c_cRegion_US [ 8 ];
		static const char c_cRegion_EU [ 8 ];
		static const char c_cRegion_H [ 8 ];
		static const char c_cRegion_R [ 8 ];
		static const char c_cRegion_C [ 8 ];
		static const char c_cRegion_Korea [ 8 ];
		
		// cd x1 is 75 sectors per second
		// dvd x1 is 676.17866 sectors per second
		// ps1 bus on a ps2 is at 36.864 Mhz
		// for dvd
		// so 36.864 mhz / 676.17866 sectors per second = 54518.1357838179 cycles per sector at 1x
		// 54518.1357838179 / 4 = 13629.5339459545 cycles per sector at 4x
		// for cd
		// 36.864 mhz / 75 sectors per second = 491520 cycles per sector at 1x
		// 491520 / 12 = 40960 cycles per sector at 12x
		// 491520 / 24 = 20480 cycles per sector at 24x
		// a seek time of 1ms would be 36864 cycles
		static const long c_lDiskNop_Cycles = 256;
		static const long c_lDiskStandby_Cycles = 1500000;
		static const long c_lDiskPause_Cycles = 1500000;
		static const long c_lDiskStop_Cycles = 1500000;
		
		static const long c_lCDSeek_Cycles = 16;
		static const long c_lDVDSeek_Cycles = 16;
		
		static const long c_lCDRead_Cycles = 20480;
		static const long c_lCDReadx1_Cycles = 491520;
		static const long c_lCDReadComplete_Cycles = 256;

		static constexpr double c_dCDReadx1_Cycles = 491520.0L;
		static constexpr double c_dDVDReadx1_Cycles = 54518.1357838179L;
		
		// must be less than 75 iop cycles ???
		static const u64 c_ullSecondaryReadInterruptDelay = 32;
		
		static const long long llClockSpeed = 36864000;
		static const long long llClockSpeed2 = 37375000;
		
		static const long long c_llCDSeek_CyclesPerSector = 10;	//100;
		static const long long c_llDVDSeek_CyclesPerSector = 5;	//25;
		
		static const long long c_llCDSeekInMilliSecs = 100;
		static const long long c_llCDFastSeekInMilliSecs = 30;

		static const long long c_llDVDSeekInMilliSecs = 100;
		static const long long c_llDVDFastSeekInMilliSecs = 30;
		
		// maximum seek time in cycles
		//static const long long c_llMaxDVDSeekTime = 6635520;
		//static const long long c_llMaxDVDSectors = 


		// this is the delay before data that was read is actually ready for output by CD/DVD via DMA etc
		// should probably be a minimum of 2048
		// large values like 32768 can cause issues ??
		// maybe should be in the range of 8192-16384 ??
		// 16384 might be an upper limit, since it keeps things from loading sometimes
		static const u64 c_ullCDReadDelay = 8;	//4096;	//8;
		static const u64 c_ullDVDReadDelay = 8;	//4096;	//8;
		
		static const long long c_llCDSeekInCycles = ( c_llCDSeekInMilliSecs * llClockSpeed ) / 1000;
		static const long long c_llCDFastSeekInCycles = ( c_llCDFastSeekInMilliSecs * llClockSpeed ) / 1000;
		static const long long c_llCDNoSeekInCycles = 8;	//4096;	//8;	//65536;	//32768;	//16384;	//8192;

		static const long long c_llDVDSeekInCycles = ( c_llDVDSeekInMilliSecs * llClockSpeed ) / 1000;
		static const long long c_llDVDFastSeekInCycles = ( c_llDVDFastSeekInMilliSecs * llClockSpeed ) / 1000;
		static const long long c_llDVDNoSeekInCycles = 8;	//4096;	//8;

		
		// within this number of sectors will negate the seek time and only read
		static const long c_lNoSeekDelta = 8;
		static const long c_lDVDNoSeekDelta = 16;
		
		// within this number of sectors will invoke a "fast seek" timing
		static const long c_lFastCDSeekDelta = 4371;
		static const long c_lFastDVDSeekDelta = 14764;

		// number of sectors that can pre-load in buffer ??
		static const unsigned long long c_ullCDVDBufferSize = 16;
		
		// need to keep track of the fractional portion of the cycles between disk reads
		double dDiskReadCycleTime, dDiskReadCycles;
		
		// need to know where disk starts reading/streaming
		u64 ullReadStart_Sector, ullReadStart_Cycle;
		

		int iTotalAvailableSectors;
		int iTotalReadSectors;
		int iTotalUnreadSectors;


		// need to know for now if read interrupt has been issued
		bool bPendingReadInt;


		char Region;
		
		//char DiskSerial [ 16 ];
		char DiskKey [ 16 ];
		
		u8 DecSet, KeyXor;
		
		u32 NReady, SReady, DiskSpeedType/*, DiskSpeed*/, SectorSize;
		u32 NextCommand, RTC_Hr, RTC_Min, RTC_Sec, RTC_Day, RTC_Month, RTC_Year;
		double DiskSpeed;
		
		// write command 0x16->0x40 Open Config
		u32 ReadWrite, Offset, NumBlocks, BlockIndex;
		
		// looks like this is the current S-Command being executed by CDVD
		u8 SCommand;
		
		// the n-command must be like reading, stuff like that
		u8 NCommand;
		
		// also has status
		u8 Status;
		
		// some type of data request value
		// bit 7 = data request, bit x = ??
		u8 DataRequest;
		
		// the S argument buffer //
		static const int c_iSArgBuffer_Size = 32;
		static const int c_iSArgBuffer_Mask = c_iSArgBuffer_Size - 1;
		u32 lSArgIndex;
		u8 ucSArgBuffer [ c_iSArgBuffer_Size ];
		
		// the N argument buffer //
		static const int c_iNArgBuffer_Size = 32;
		static const int c_iNArgBuffer_Mask = c_iNArgBuffer_Size - 1;
		u32 lNArgIndex;
		u8 ucNArgBuffer [ c_iNArgBuffer_Size ];
		
		// the result buffer //
		static const int c_iResultBuffer_Size = 32;
		static const int c_iResultBuffer_Mask = c_iResultBuffer_Size - 1;
		u32 lResultIndex, lResultCount;
		u8 ucResultBuffer [ c_iResultBuffer_Size ];
		
		u8 mg_datatype;
		
		void EnqueueResult ( u32 Size );
		
		static const int c_iNVMSize = 1024;
		u8 NVM [ c_iNVMSize ];

		// I'll make the bios version variable later
		static const int c_iBiosVersion = 1;
		
		// this should hold the current disk type that is in the drive
		u8 CurrentDiskType;
		
		// this will hold the current interrupt reason (which is usually 0x2, for "command complete")
		u8 InterruptReason;
		
		// looks like the sector buffer is only so large on PS2 ??
		static const u32 c_iMaxSectorRead = 32;
		
		u32 SeekSectorNum, SectorReadCount, SectorReadSize;
		u32 NumSectorsBeingRead;
		u32 AvailableSectorCount;
		u32 ReadCommand;
		
		// the number of sectors that are currently being read
		u32 ReadingSectorCount;
		
		// there must be something up with large transfers, like an interrupt so often or something ??
		static const u32 c_ulIntSectors = 16;
		u32 ulNextSectorInt;
		
		// need to know the last sector seeked to and the cycle that happened at
		static const long c_lSectorBufferSize = 64;
		long ReadSector;
		long lLastSeekSector;
		u64 ullLastSeekCycle;
		
		// note: disk image object needs to be shared with PS1
		
		// these constants are ripped from pcsx2 //
		// SpindleCtrl Masks
		#define CDVD_SPINDLE_SPEED 0x7  // Speed ranges from 0-3 (1, 2, 3, 4x for DVD) and 0-5 (1, 2, 4, 12, 24x for CD)
		#define CDVD_SPINDLE_NOMINAL 0x40 // Changes the speed to be constant (CLV) based on current speed
		#define CDVD_SPINDLE_CAV 0x80  // CAV/CLV selector

		// CDVDreadTrack mode values:
		#define CDVD_MODE_2352	0	// full 2352 bytes
		#define CDVD_MODE_2340	1	// skip sync (12) bytes
		#define CDVD_MODE_2328	2	// skip sync+head+sub (24) bytes
		#define CDVD_MODE_2048	3	// skip sync+head+sub (24) bytes
		#define CDVD_MODE_2368	4	// full 2352 bytes + 16 subq

		// CDVDgetDiskType returns:
		#define CDVD_TYPE_ILLEGAL	0xff	// Illegal Disc
		#define CDVD_TYPE_DVDV		0xfe	// DVD Video
		#define CDVD_TYPE_CDDA		0xfd	// Audio CD
		#define CDVD_TYPE_PS2DVD	0x14	// PS2 DVD
		#define CDVD_TYPE_PS2CDDA	0x13	// PS2 CD (with audio)
		#define CDVD_TYPE_PS2CD		0x12	// PS2 CD
		#define CDVD_TYPE_PSCDDA 	0x11	// PS CD (with audio)
		#define CDVD_TYPE_PSCD		0x10	// PS CD
		#define CDVD_TYPE_UNKNOWN	0x05	// Unknown
		#define CDVD_TYPE_DETCTDVDD	0x04	// Detecting Dvd Dual Sided
		#define CDVD_TYPE_DETCTDVDS	0x03	// Detecting Dvd Single Sided
		#define CDVD_TYPE_DETCTCD	0x02	// Detecting Cd
		#define CDVD_TYPE_DETCT		0x01	// Detecting
		#define CDVD_TYPE_NODISC 	0x00	// No Disc

		// CDVDgetTrayStatus returns:
		#define CDVD_TRAY_CLOSE		0x00
		#define CDVD_TRAY_OPEN		0x01
		
		// values for status
		#define CDVD_STATUS_STOP		0x00
		#define CDVD_STATUS_TRAY_OPEN	0x01	// confirmed to be tray open
		#define CDVD_STATUS_SPIN		0x02
		#define CDVD_STATUS_READ		0x06
		#define CDVD_STATUS_PAUSE		0x0A	// neutral value. Recommended to never rely on this.
		#define CDVD_STATUS_SEEK		0x12
		#define CDVD_STATUS_EMERGENCY	0x20

		// cdvdTD.type (track types for cds)
		#define CDVD_AUDIO_TRACK 0x01
		#define CDVD_MODE1_TRACK 0x41
		#define CDVD_MODE2_TRACK 0x61

		enum nCmds
		{
			N_CD_SYNC = 0x00, // CdSync
			N_CD_NOP = 0x01, // CdNop
			N_CD_STANDBY = 0x02, // CdStandby
			N_CD_STOP = 0x03, // CdStop
			N_CD_PAUSE = 0x04, // CdPause
			N_CD_SEEK = 0x05, // CdSeek
			N_CD_READ = 0x06, // CdRead
			N_CD_READ_CDDA = 0x07, // CdReadCDDA
			N_DVD_READ = 0x08, // DvdRead
			N_CD_GET_TOC = 0x09, // CdGetToc & cdvdman_call19
			N_CMD_B = 0x0B, // CdReadKey
			N_CD_READ_KEY = 0x0C, // CdReadKey
			N_CD_READ_XCDDA = 0x0E, // CdReadXCDDA
			N_CD_CHG_SPDL_CTRL = 0x0F, // CdChgSpdlCtrl
		};

		u32 DMAReadIndex;
		static u64 DMA_ReadyForRead ( void );
		static u32 DMA_ReadBlock ( u32* pMemoryPtr, u32 Address, u32 BS );
		
		u32 CDVDBuffer [ 4096 ];
		
		
		static void sRun () { _CDVD->Run (); }
		static void Set_EventCallback ( funcVoid2 UpdateEvent_CB ) { _CDVD->NextEvent_Idx = UpdateEvent_CB ( sRun ); };
		
		
		// for interrupt call back
		static funcVoid UpdateInterrupts;
		static void Set_IntCallback ( funcVoid UpdateInt_CB ) { UpdateInterrupts = UpdateInt_CB; };
		
		
		static const u32 c_InterruptBit = 11;
		static const u32 c_InterruptBit_CDVD = 2;
		
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
			_R3000A_Cause_13 = _R3000A_Cause;
			_R3000A_Status_12 = _R3000A_Status;
			_ProcStatus = _ProcStat;
		}
		
		
		inline static void SetInterrupt ()
		{
			*_Intc_Stat |= ( 1 << c_InterruptBit );
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}
		
		inline static void ClearInterrupt ()
		{
			*_Intc_Stat &= ~( 1 << c_InterruptBit );
			
			UpdateInterrupts ();
			
			/*
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}

		inline static void SetInterrupt_CDVD ()
		{
			*_Intc_Stat |= ( 1 << c_InterruptBit_CDVD );
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}
		
		inline static void ClearInterrupt_CDVD ()
		{
			*_Intc_Stat &= ~( 1 << c_InterruptBit_CDVD );
			
			UpdateInterrupts ();
			
			/*
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}

		
		static u64* _NextSystemEvent;

		inline void Update_NextEvent ()
		{
			if ( NextEvent_Cycle < *_NextSystemEvent )
			{
				*_NextSystemEvent = NextEvent_Cycle;
				*_NextEventIdx = NextEvent_Idx;
			}
		}
		
		// set the offset to the next event
		inline void Set_NextEvent ( u64 Cycles )
		{
			NextEvent_Cycle = Cycles + *_DebugCycleCount;
			
			//if ( NextEvent_Cycle > *_SystemCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_SystemCycleCount ) ) *_NextSystemEvent = NextEvent_Cycle;
			Update_NextEvent ();
		}
		
		// set the cycle# of the next event
		inline void Set_NextEventCycle ( u64 ullCycle )
		{
			NextEvent_Cycle = ullCycle;
			
			//if ( NextEvent_Cycle > *_SystemCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_SystemCycleCount ) ) *_NextSystemEvent = NextEvent_Cycle;
			Update_NextEvent ();
		}
		
		///////////////////////////////////
		// Debug
		static u32 *_DebugPC;
		static u64 *_DebugCycleCount;
		static u64 *_SystemCycleCount;
		static u32 *_NextEventIdx;
		
		// I'll make this variable standard on all objects
		s64 BusyCycles;
		
		
		
		bool LoadNVMFile ( string FilePath );
		bool Store_NVMFile ( string FilePath );
		
	};
	
	


	// ripped from pcsx2 (most of this is, as I don't know anything about the PS2 CDVD...) //
	// NVM (eeprom) layout info
	struct NVMLayout {
		u32 biosVer;	// bios version that this eeprom layout is for
		s32 config0;	// offset of 1st config block
		s32 config1;	// offset of 2nd config block
		s32 config2;	// offset of 3rd config block
		s32 consoleId;	// offset of console id (?)
		s32 ilinkId;	// offset of ilink id (ilink mac address)
		s32 modelNum;	// offset of ps2 model number (eg "SCPH-70002")
		s32 regparams;	// offset of RegionParams for PStwo
		s32 mac;		// offset of the value written to 0xFFFE0188 and 0xFFFE018C on PStwo
	};

	#define NVM_FORMAT_MAX	2
	static const NVMLayout nvmlayouts[NVM_FORMAT_MAX] =
	{
		{0x000,  0x280, 0x300, 0x200, 0x1C8, 0x1C0, 0x1A0, 0x180, 0x198},	// eeproms from bios v0.00 and up
		{0x146,  0x270, 0x2B0, 0x200, 0x1C8, 0x1E0, 0x1B0, 0x180, 0x198},	// eeproms from bios v1.70 and up
	};
	
};

#endif

