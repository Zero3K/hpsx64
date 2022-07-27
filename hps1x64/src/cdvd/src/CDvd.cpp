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

// NOTE: Much of the CDVD code is "adapted" from pcsx2 or other programs currently

#include "CDvd.h"
#include "PS1_CD.h"
#include "PS1_Dma.h"
#include <string.h>
#include <fstream>

using namespace Playstation1;
using namespace std;

Debug::Log CDVD::debug;
Debug::Log CDVD::debugRead;

CDVD *CDVD::_CDVD;


u64* CDVD::_NextSystemEvent;


// enable this to test interrupts while the disk is still performing a read command
//#define INT_WHILE_READING

// testing an alternate timing for reading data from disk
//#define READ_TIMING_STREAM

// this define actually allows loading in some instances, and stops loading when not enabled ??
#define ENABLE_NREADY4E

//#define ENABLE_NREADY_SPIN


#define DISKREAD_PER_SECTOR
//#define ENABLE_CDVD_READ_PERBLOCK
//#define DISKREAD_INT_PER_SECTOR


#define DISKREAD_FIXED_SEEK
#define DISKREAD_FIXED_SEEK_FAST
//#define DISKREAD_FIXED_SEEK_SLOW

//#define TEST_DISKSEEK
//#define ENABLE_BUFFER_SECTORS

// when this is enabled for all reads this appears to mess up some timings
//#define ADJUST_DISK_SPEED

//#define REQUIRE_STREAM_FOR_ADJUST


//#define ADJUST_STREAM_SPEED


// if bit 7 is set in read speed, then read at max speed?
//#define ENABLE_INITIAL_MAX_SPEED


// interrupts with a complete interrupt when reading is done and the data ready interrupt
// note: proven not to work this way, so the code and associated define can be removed
//#define ENABLE_READ_COMPLETE_INTERRUPT

// this causes the data ready interrupt to ONLY trigger once all data has been transferred from CDVD to local memory via dma
#define INT_AFTER_DATA_TRANSFER



#ifdef _DEBUG_VERSION_

#define INLINE_DEBUG_ENABLE


//#define INLINE_DEBUG_SPLIT
//#define INLINE_DEBUG_SPLIT_READ

/*
//#define INLINE_DEBUG
#define INLINE_DEBUG_READ
#define INLINE_DEBUG_WRITE
#define INLINE_DEBUG_RUN

#define INLINE_DEBUG_DISKREAD
#define INLINE_DEBUG_READBUFFER
#define INLINE_DEBUG_READY
*/

#endif


funcVoid CDVD::UpdateInterrupts;

u32* CDVD::_DebugPC;
u64* CDVD::_DebugCycleCount;
u64* CDVD::_SystemCycleCount;
u32* CDVD::_NextEventIdx;

u32* CDVD::_Intc_Stat;
u32* CDVD::_Intc_Mask;
u32* CDVD::_R3000A_Status_12;
u32* CDVD::_R3000A_Cause_13;
u64* CDVD::_ProcStatus;


const char CDVD::c_cRegion_JAP [ 8 ] = { 0, 'J', 'j', 'p', 'n', 0, 'J', 0 };
const char CDVD::c_cRegion_US [ 8 ] = { 0, 'A', 'e', 'n', 'g', 0, 'U', 0 };
const char CDVD::c_cRegion_EU [ 8 ] = { 0, 'E', 'e', 'n', 'g', 0, 'O', 0 };
const char CDVD::c_cRegion_H [ 8 ] = { 0, 'H', 'e', 'n', 'g', 0, 'E', 0 };
const char CDVD::c_cRegion_R [ 8 ] = { 0, 'R', 'e', 'n', 'g', 0, 'A', 0 };
const char CDVD::c_cRegion_C [ 8 ] = { 0, 'C', 's', 'c', 'h', 0, 'C', 0 };
const char CDVD::c_cRegion_Korea [ 8 ] = { 0, 'K', 'k', 'o', 'r', 0, 'M', 0 };





CDVD::CDVD ()
{
}


void CDVD::Start ()
{
	cout << "Running CDVD::Start...\n";
	
#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create( "CDVD_Log.txt" );
	
#ifdef INLINE_DEBUG_SPLIT_READ
	// put debug output into a separate file
	debugRead.SetSplit ( true );
	debugRead.SetCombine ( false );
#endif
	
	debugRead.Create( "CDVDRead_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering CDVD::Start";
#endif

	Reset ();
	
	_CDVD = this;
	
	// start region with Japan
	Region = 'J';
	
	SReady = 0x40;
	
	// 0x40 or 0x4e and at which times??
	// note: should probably start with 0x4e
	NReady = 0x4e;
	
	DiskSpeed = 4;
	SectorSize = 2064;
	
	// set the amound of data to read from sector to 2048 by default ??
	SectorReadSize = 2048;
	
	
	// need to set the next event as being far in the future, since it hasn't happened yet
	// need to do this for every peripheral on the bus with events
	Set_NextEventCycle ( -1ULL );

#ifdef INLINE_DEBUG
	debug << "->Exiting CDVD::Start";
#endif
}


void CDVD::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( CDVD ) );
}

// returns interrupt;
void CDVD::Run ()
{
	u32 NumberOfReads;
	u64 ullDiskReadCycles;
	
	u64 ullCycleDiff;
	u32 lStartSector, lEndSector;
	double dSectorsRead;

	u64 ullTargetCycle, ullTargetCycle2;
	
	
	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;
	
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nCDVD::Run " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << "; BusyCycles=" << BusyCycles;
#endif


	// need to clear the event in the system right away, since it is being handled here
	Set_NextEventCycle ( -1ULL );

	
	switch ( ReadCommand )
	{
		// SYNC (or motor on?)
		case 0:

		// NOP
		case 1:
			// complete
			InterruptReason = 2;
			SetInterrupt_CDVD ();
			
			// the status is "pause" ??
			Status = CDVD_STATUS_PAUSE;
			
			NReady = 0x40;
			
			// done
			ReadCommand = -1;
			break;
			
		// STANDBY
		case 0x2:
		
			// complete
			InterruptReason = 2;
			SetInterrupt_CDVD ();

			// reset position of read head
			ReadSector = 0;
			
			// the status is "pause" ??
			Status = CDVD_STATUS_PAUSE;
			
			NReady = 0x40;
			
			// done
			ReadCommand = -1;
			break;
		
		// STOP
		case 0x3:
			InterruptReason = 2;
			SetInterrupt_CDVD ();
			
			// the status is "stop" ??
			Status = CDVD_STATUS_STOP;
			
			NReady = 0x40;
			
			// done
			ReadCommand = -1;
			break;
		
		// PAUSE
		case 0x4:
			// complete
			InterruptReason = 2;
			SetInterrupt_CDVD ();
			
			// the status is "pause"
			Status = CDVD_STATUS_PAUSE;
			
			NReady = 0x40;
			
			// done
			ReadCommand = -1;
			break;
			
			
		// SEEK
		case 0x5:
			// complete
			InterruptReason = 2;
			SetInterrupt_CDVD ();
			
			// the status is "pause"
			Status = CDVD_STATUS_PAUSE;
			
			NReady = 0x40;
			
			// done
			ReadCommand = -1;
			break;
			
		// cd read
		case 0x6:
#ifdef INLINE_DEBUG_RUN
	debug << "; CDREAD";
	debug << dec << " Sector#" << SeekSectorNum;
	debug << dec << " SectorReadCount=" << SectorReadCount;
	debug << " ReadSector=" << ReadSector;
	debug << " DiskReadCycleTime=" << dDiskReadCycleTime;
	debug << dec << " BA=" << Dma::_DMA->pRegData [ 3 ]->BCR.BA << " BS=" << Dma::_DMA->pRegData [ 3 ]->BCR.BS;
#endif


			// get the number of sectors being read
			//NumSectorsBeingRead = ( ( SectorReadCount <= c_iMaxSectorRead ) ? SectorReadCount : c_iMaxSectorRead );

			if ( Status != CDVD_STATUS_READ )
			{
#ifdef INLINE_DEBUG_RUN
	debug << "; SEEK_DONE";
#endif



				// reading has begun, so status is "read"
				Status = CDVD_STATUS_READ;

#ifdef DISKREAD_PER_SECTOR

#ifdef ENABLE_CDVD_READ_PERBLOCK
				// get the number of sectors to read
				NumSectorsBeingRead = ( ( SectorReadCount <= c_iMaxSectorRead ) ? SectorReadCount : c_iMaxSectorRead );
#else
				NumSectorsBeingRead = 1;
#endif
				
				// get the time needed to read those sectors
				// note: no need to adjust the disk read timing here since we just did a seek
				ullDiskReadCycles = (u64) ( dDiskReadCycleTime * NumSectorsBeingRead );
					
#else
				// get the number of sectors to read
				NumSectorsBeingRead = SectorReadCount;
				
				ullDiskReadCycles = (u64) ( dDiskReadCycleTime * SectorReadCount );
#endif





				//ullDiskReadCycles = (u64) ( dDiskReadCycleTime );
				//ullDiskReadCycles = (u64) ( dDiskReadCycleTime * NumSectorsBeingRead );
				
				// *new* to make sure we land on or after the sector is read, add another cycle
				//ullDiskReadCycles += 1;
				
				
				// *new* seek is done, so record cycle and sector
				ullLastSeekCycle = *_DebugCycleCount;
				lLastSeekSector = ReadSector;
				
				// come back when done reading data
				Set_NextEvent ( ullDiskReadCycles );
				
#ifdef DISKREAD_PER_SECTOR
				/*
				if ( DiskSpeedType & 0x80 )
				{
				*/
					// set the sector and cycle the read is starting at
					// assume 32 entry buffer size ??
					ullReadStart_Sector = SeekSectorNum;
					ullReadStart_Cycle = *_DebugCycleCount;
				/*
				}
				else
				{
					// set the sector and cycle the read is starting at
					ullReadStart_Sector = SeekSectorNum;
					ullReadStart_Cycle = *_DebugCycleCount;
				}
				*/
#endif

				//if ( SectorReadCount > 8 )
				//{
				//	cout << "\nhps2x64: ***ALERT***: CDVD: SectorReadCount>8.\n";
				//}
				
#ifdef INLINE_DEBUG_RUN
	debug << dec << " ullDiskReadCycles=" << ullDiskReadCycles;
	debug << dec << " ( ( c_lCDReadx1_Cycles / DiskSpeed ) * SectorReadCount )=" << ( ( c_lCDReadx1_Cycles / DiskSpeed ) * SectorReadCount );
#endif

				return;
			}
			else
			{
				// *new* data is being read
				// check how many of the sectors have been read already
				
				// *** NOTE *** disk reading works best when the data is read all at once (probably what the disk cache is used for)

// can do it this way if not reading per sector				
#ifndef DISKREAD_PER_SECTOR

#ifdef INLINE_DEBUG_RUN
	debug << dec << " READING-ALL-SECTORS";
#endif
				// increase number of available sectors
				AvailableSectorCount = SectorReadCount;
				
				// PS1 CD DMA is active, so trigger a transfer //
				//Dma::_DMA->DMA3_Run ( false );
				//Dma::_DMA->Transfer ( 3 );
				Dma::_DMA->Update_ActiveChannel ();
					
				// increment sector being read
				ReadSector += SectorReadCount;
				
				// dec number of sectors remaining to read
				SectorReadCount = 0;
#endif
				
				/*
				// get difference in cycles
				ullCycleDiff = ullLastSeekCycle - *_DebugCycleCount;
				
				// get the difference in sectors read
				dSectorsRead = ( (double) ullCycleDiff ) / dDiskReadCycleTime;

#ifdef READ_TIMING_STREAM				
				// get the end sector
				lEndSector = lLastSeekSector + ( (u64) dSectorsRead );
#else
				lEndSector = ReadSector + 1;
#endif
				
				while ( ReadSector < lEndSector && SectorReadCount > 0 )
				{
					// increase number of available sectors
					AvailableSectorCount++;
					
					// PS1 CD DMA is active, so trigger a transfer //
					Dma::_DMA->DMA3_Run ( false );
					
					// dec number of sectors remaining to read
					SectorReadCount--;
					NumSectorsBeingRead--;
					
					// increment sector being read
					ReadSector++;
					
#ifdef INT_WHILE_READING
					if ( ( SectorReadCount ) && ( SectorReadCount == ulNextSectorInt ) )
					{
						InterruptReason = 1;
						SetInterrupt_CDVD ();
						
						ulNextSectorInt -= c_ulIntSectors;
					}
#endif

				}
				*/
			}

#ifdef INLINE_DEBUG_RUN
	debug << "; READING_DATA";
#endif

			//while ( NumSectorsBeingRead )
			//{
			
			// also need to trigger read via dma, since it is possible to have dma3 ready BEFORE even starting the read command
			/*
			if ( Dma::_DMA->isActive ( 3 ) )
			{
			*/
				// increase number of available sectors
				//AvailableSectorCount++;
				
				/*
				// read from cd
				CD::_CD->cd_image.ReadNextSector ();
				
				CD::_CD->SectorDataBuffer_Index = CD::_CD->cd_image.GetCurrentBufferIndex ();
				
				// currently need to set this
				// no, no... dma transfer is actually not ready until 0x80 is written to request data
				CD::_CD->isSectorDataReady = true;
				
				// this stuff needs to be set again ?? (fix later)
				// set the sector size for ps1 cd handler
				// need to set the data buffer size again since the ps1 handler resets it for now
				CD::_CD->DataBuffer_Size = _CDVD->SectorReadSize;
				//CD::_CD->ReadMode = _CDVD->SectorReadSize;
			
				// set the sector number that is being read
				CD::_CD->DVDSectorNumber = SeekSectorNum;
				*/
				
				// PS1 CD DMA is active, so trigger a transfer //
				//Dma::_DMA->DMA3_Run ( false );
				
				// dec number of sectors remaining to read
				//SectorReadCount--;
				//NumSectorsBeingRead--;
				
				/*
				// advance sector number
				SeekSectorNum++;
				*/

#ifdef INLINE_DEBUG_RUN
			// GetDataBuffer
			//u8 *DiskData = CD::_CD->cd_image.GetDataBuffer ( CD::_CD->cd_image.GetCurrentBufferIndex () );
			//debug << " (from disk)";
			//debug << hex << " Min=" << (u32) DiskData [ 12 ];
			//debug << hex << " Sec=" << (u32) DiskData [ 13 ];
			//debug << hex << " Frac=" << (u32) DiskData [ 14 ];
			debug << " AvailableSectors=" << dec << AvailableSectorCount;
#endif


				// if all the data was transferred, then the next transfer is not ready yet
				/*
				if ( !CD::_CD->DataBuffer_Size )
				{
					CD::_CD->isSectorDataReady = false;
				}
			}
			else
			{
				// this would be a problem
				cout << "\nhps2x64: CDVD: DMA#3 did not transfer a sector due to inactive DMA";
				
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nCDVD: ***DMA#3 did not transfer a sector due to inactive DMA***\r\n";
#endif
				
				// need to stop reading here, nothing transferred
				// it didn't transfer the sector, but should continue reading since reading didn't stop
				//return;
				
				// still need to update variables
				// dec number of sectors remaining to read
				SectorReadCount--;
				NumSectorsBeingRead--;
				
				// increase number of available sectors
				AvailableSectorCount++;
				
				// advance sector number
				SeekSectorNum++;
			}
			*/
			
			//}	// end while
			
			/*
			if ( DiskSpeedType & 0x80 )
			{
				// another sector available now
				AvailableSectorCount += SectorReadCount;
				
				// PS1 CD DMA is active, so trigger a transfer //
				Dma::_DMA->Transfer ( 3 );
				
				// increment sector being read
				ReadSector += SectorReadCount;
				
				// dec number of sectors remaining to read
				SectorReadCount = 0;
			}
			*/
			
			if ( SectorReadCount )
			{

				// get the number of sectors being read
				//NumSectorsBeingRead = ( ( SectorReadCount <= c_iMaxSectorRead ) ? SectorReadCount : c_iMaxSectorRead );
				
				// check if any sectors were just read
				// this could happen if skipping seek like if there is no need to
				if ( !NumSectorsBeingRead )
				{
#ifdef ENABLE_CDVD_READ_PERBLOCK
					// get the number of sectors to read
					NumSectorsBeingRead = ( ( SectorReadCount <= c_iMaxSectorRead ) ? SectorReadCount : c_iMaxSectorRead );
#else
					NumSectorsBeingRead = 1;
#endif

#ifdef INLINE_DEBUG_RUN
	debug << dec << " SECTOR(S)-BEING-READ=" << NumSectorsBeingRead;
#endif


					// get the time needed to read those sectors
					ullDiskReadCycles = (u64) ( dDiskReadCycleTime * NumSectorsBeingRead );
					
#ifdef ADJUST_DISK_SPEED

#ifdef REQUIRE_STREAM_FOR_ADJUST
					// only do this under certain cases ??
					//if ( !( DiskSpeedType & 0x80 ) )
					if ( ( DiskSpeedType & 0x80 ) )
#endif
					{
						// if reading sector by sector, then just need to know if has read past current sector
						ullTargetCycle2 = ullReadStart_Cycle + ( (u64) ( dDiskReadCycleTime * ( ( ReadSector + NumSectorsBeingRead ) - ullReadStart_Sector ) ) );
						
						if ( *_DebugCycleCount < ullTargetCycle2 )
						{
							ullDiskReadCycles = ullTargetCycle2 - ( *_DebugCycleCount );
						}
						else
						{
							ullDiskReadCycles = 8;
						}
					} // end if ( !( DiskSpeedType & 0x80 ) )
#endif

#ifdef INLINE_DEBUG_RUN
	debug << dec << " CYCLES-TO-READ-SECTOR(S)=" << ullDiskReadCycles;
#endif

					// come back when done reading data
					Set_NextEvent ( ullDiskReadCycles );
					
					// done until the next sector is finished reading
					return;
				}

if ( NumSectorsBeingRead > 1 )
{
	cout << "->1st read sectors:" << dec << NumSectorsBeingRead;
}

				AvailableSectorCount += NumSectorsBeingRead;
				

				// increment sector being read
				ReadSector += NumSectorsBeingRead;
				
				// dec number of sectors remaining to read
				SectorReadCount -= NumSectorsBeingRead;
//#else
				// another sector available now
				//AvailableSectorCount++;
				
				// increment sector being read
				//ReadSector++;
				
				// dec number of sectors remaining to read
				//SectorReadCount--;
//#endif

#ifdef INLINE_DEBUG_RUN
	debug << dec << " SECTOR(S)-JUST-READ=" << NumSectorsBeingRead;
	debug << dec << " SECTOR(S)-NOW-AVAIL=" << AvailableSectorCount;
	debug << dec << " SECTOR(S)-LEFT=" << SectorReadCount;
#endif
				
				// PS1 CD DMA is active, so trigger a transfer //
				//Dma::_DMA->Transfer ( 3 );
				Dma::_DMA->Update_ActiveChannel ();
				

				// if there are more sectors to read, then set event
				if ( SectorReadCount )
				{
#ifdef DISKREAD_INT_PER_SECTOR
					// data ready
					InterruptReason = 1;
					SetInterrupt_CDVD ();
#endif


#ifdef ENABLE_CDVD_READ_PERBLOCK
					// get the number of sectors to read
					NumSectorsBeingRead = ( ( SectorReadCount <= c_iMaxSectorRead ) ? SectorReadCount : c_iMaxSectorRead );

					ullDiskReadCycles = (u64) ( dDiskReadCycleTime * NumSectorsBeingRead );
#else
					// get the number of sectors to read
					NumSectorsBeingRead = 1;
					
					//ullDiskReadCycles = (u64) ( dDiskReadCycleTime * SectorReadCount );
					ullDiskReadCycles = (u64) ( dDiskReadCycleTime );
					//ullDiskReadCycles = (u64) ( dDiskReadCycleTime * NumSectorsBeingRead );
#endif
					
#ifdef INLINE_DEBUG_RUN
	debug << dec << " SECTOR(S)-BEING-READ=" << NumSectorsBeingRead;
#endif

					// *new* add another cycle to make sure we land on or after sector is read
					//ullDiskReadCycles += 1;
					
#ifdef DISKREAD_PER_SECTOR

#ifdef ADJUST_DISK_SPEED

#ifdef REQUIRE_STREAM_FOR_ADJUST
				//if ( !( DiskSpeedType & 0x80 ) )
				if ( ( DiskSpeedType & 0x80 ) )
#endif
				{
					// if reading sector by sector, then just need to know if has read past current sector
					ullTargetCycle2 = ullReadStart_Cycle + ( (u64) ( dDiskReadCycleTime * ( ( ReadSector + NumSectorsBeingRead ) - ullReadStart_Sector ) ) );
					
					if ( *_DebugCycleCount < ullTargetCycle2 )
					{
						ullDiskReadCycles = ullTargetCycle2 - ( *_DebugCycleCount );
					}
					else
					{
						ullDiskReadCycles = 8;
					}
				}
#endif

#endif

#ifdef INLINE_DEBUG_RUN
	debug << dec << " CYCLES-TO-READ-SECTOR(S)=" << ullDiskReadCycles;
#endif

					// come back when done reading data
					Set_NextEvent ( ullDiskReadCycles );
					
					// done until the next sector is finished reading
					return;
				}
			}
			
			
			// check if there are sectors left to read (if not, signal command complete ??)
			if ( !SectorReadCount )
			{
#ifdef INLINE_DEBUG_RUN
	debug << "; COMPLETE";
#endif

				//if ( Dma::_DMA->isActive ( 3 ) )
				//{
				//	// need to do something about this
				//	cout << "\nhps2x64: CDVD: DMA#3 still active after transfer complete";
				//}
				
				
				
				
				// no more events for now
				//Set_NextEvent_Value ( -1 );


#ifndef INT_AFTER_DATA_TRANSFER
				// data ready
				InterruptReason = 1;
				// should be both data ready and command complete interrupt in one simultaneously?
				//InterruptReason = 3;
				SetInterrupt_CDVD ();
				//SetInterrupt ();
#else

				/*
				if ( ! Dma::_DMA->isActive ( 3 ) )
				{
					bPendingReadInt = false;

					// data ready
					InterruptReason = 1;
					// should be both data ready and command complete interrupt in one simultaneously?
					//InterruptReason = 3;
					SetInterrupt_CDVD ();
				}
				*/

#endif

				
#ifdef ENABLE_NREADY_SPIN
				Status = CDVD_STATUS_SPIN;
				//Status = CDVD_STATUS_READ;
#else
				// when reading is done, the status is "pause"
				Status = CDVD_STATUS_PAUSE;
#endif
				
				// ***todo*** looks like there should also be an Interrupt 2 immediately after this
				
				// done reading
				ReadCommand = -1;
				
				// ready
				// with 0x4e reading seems to happen sooner, otherwise it might spin on 0x40 for awhile ??
#ifdef ENABLE_NREADY4E
				NReady = 0x4e;
#else
				NReady = 0x40;
#endif
				
				// command complete ??
				//InterruptReason = 2;
				//SetInterrupt_CDVD ();
				
				return;
			}
			
			
			break;
			
			
		// READ TOC
		case 0x9:
		
			// read from toc
			//CD::_CD->cd_image.ReadNextSector ();
			
			//CD::_CD->SectorDataBuffer_Index = CD::_CD->cd_image.GetCurrentBufferIndex ();
			
			// currently need to set this
			// no, no... dma transfer is actually not ready until 0x80 is written to request data
			//CD::_CD->isSectorDataReady = true;
			AvailableSectorCount++;
			
			// 1024?? appears to want 2064 bytes when reading TOC for a DVD
			// I'll let 1024 mean it needs TOC info for now
			_CDVD->SectorReadSize = 1024;
			CD::_CD->ReadMode = 1024;
			
			// this stuff needs to be set again ?? (fix later)
			// set the sector size for ps1 cd handler
			// need to set the data buffer size again since the ps1 handler resets it for now
			CD::_CD->DataBuffer_Size = 2064;	//_CDVD->SectorReadSize;
			//CD::_CD->ReadMode = _CDVD->SectorReadSize;
			
			
			// also need to trigger read via dma, since it is possible to have dma3 ready BEFORE even starting the read command
			/*
			if ( Dma::_DMA->isActive ( 3 ) )
			{
			*/
				// set the sector number that is being read
				//CD::_CD->DVDSectorNumber = SeekSectorNum;
				
				// PS1 CD DMA is active, so trigger a transfer //
				//Dma::_DMA->DMA3_Run ( false );
				//Dma::_DMA->Transfer ( 3 );
				Dma::_DMA->Update_ActiveChannel ();
				
			/*
				// if all the data was transferred, then the next transfer is not ready yet
				if ( !CD::_CD->DataBuffer_Size )
				{
					CD::_CD->isSectorDataReady = false;
				}
			}
			*/
			
#ifdef INLINE_DEBUG_RUN
	debug << "; COMPLETE";
#endif

				// data ready
				// command complete ??
				// might be int 1 then int 2 ??
				InterruptReason = 2;
				SetInterrupt_CDVD ();
				
				// ***todo*** looks like there should also be an Interrupt 2 immediately after this
				
				NReady = 0x40;
				
				// done reading
				ReadCommand = -1;
			
			break;

			
		case 0x20:
#ifdef INLINE_DEBUG_RUN
	debug << "; READ COMPLETE EVENT";
#endif
			// completion event
			// no, this should probably happen immediately
			_CDVD->InterruptReason = 2;
			SetInterrupt_CDVD ();
			
			NReady = 0x40;
			
			// done
			ReadCommand = -1;
			break;
			
		default:
#ifdef INLINE_DEBUG_RUN
	debug << "; ***UNKNOWN READCOMMAND=" << hex << ReadCommand;
#endif

			cout << "hps1x64: ALERT: CDVD: ***UNKNOWN READCOMMAND=" << hex << ReadCommand << " ***";
			break;
	}
	
//#ifdef INLINE_DEBUG_RUN
//	debug << "; Signalling Interrupt";
//#endif

	// time of next event after this one is not known
	//NextEvent_Cycle = 0xffffffffffffffff;
	//Set_NextEvent ( 0xffffffffffffffffULL );

	// signal interrupt
	//SetInterrupt ();
}





u32 CDVD::Read ( u32 Address )
{
#ifdef INLINE_DEBUG_READ
	debug << "\r\nCDVD::Read " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address;
#endif

	u32 Output;
	int temp, key;
	
	switch ( Address & 0xff )
	{
		case 0x4:	// NCOMMAND
#ifdef INLINE_DEBUG_READ
	debug << "; NCOMMAND";
#endif

			Output = _CDVD->NCommand;
			
			break;

			
		case 0x5:	// NREADY
#ifdef INLINE_DEBUG_READ
	debug << "; NREADY";
#endif

			Output = _CDVD->NReady;
			break;


		case 0x6:	// ERROR
#ifdef INLINE_DEBUG_READ
	debug << "; ERROR";
#endif

			Output = 0;
			break;

			
		case 0x7:	// BREAK
#ifdef INLINE_DEBUG_READ
	debug << "; BREAK";
#endif

			Output = 0;
			break;
			
		
		// interrupt reason ?? (usually 0x2)
		case 0x8:
#ifdef INLINE_DEBUG_READ
	debug << "; INTERRUPT REASON";
#endif

			Output = _CDVD->InterruptReason;
			break;
			
		
		// STATUS ??
		case 0xa:
#ifdef INLINE_DEBUG_READ
	debug << "; STATUS";
#endif

			Output = _CDVD->Status;
		
			break;

			
		case 0xb:
#ifdef INLINE_DEBUG_READ
	debug << "; TRAY STATE";
#endif

			// 1 means tray open, 0 means tray closed
			// tray closed for now
			Output = 0;
			
			break;
		
		case 0xf:	// DISK TYPE
#ifdef INLINE_DEBUG_READ
	debug << "; DISK TYPE";
#endif

			Output = _CDVD->CurrentDiskType;

			break;

			
		case 0x13:
#ifdef INLINE_DEBUG_READ
	debug << "; Command 0x13->Unknown";
#endif

			Output = 4;
			break;
			
		case 0x16:	// SCOMMAND
#ifdef INLINE_DEBUG_READ
	debug << "; SCOMMAND";
#endif

			Output = _CDVD->SCommand;
			break;

			
		case 0x17:	// SREADY
#ifdef INLINE_DEBUG_READ
	debug << "; SREADY";
#endif
			
			Output = _CDVD->SReady;
			break;

		
		case 0x18:	// SDATAOUT (this looks like it is the result buffer)
#ifdef INLINE_DEBUG_READ
	debug << "; SDATAOUT";
#endif
		
			Output = 0;
			if ( !( _CDVD->SReady & 0x40 ) && ( _CDVD->lResultIndex < _CDVD->lResultCount ) )
			{
				Output = _CDVD->ucResultBuffer [ _CDVD->lResultIndex++ ];
				
				if ( _CDVD->lResultIndex >= _CDVD->lResultCount ) _CDVD->SReady |= 0x40;
			}
			
			break;
			
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
#ifdef INLINE_DEBUG_READ
	debug << "; READ KEY X1";
#endif
		
			key = Address & 0xff;
			temp = key - 0x20;

			Output = _CDVD->DiskKey[temp];
			break;
			
		case 0x28:
		case 0x29:
		case 0x2A:
		case 0x2B:
		case 0x2C:
#ifdef INLINE_DEBUG_READ
	debug << "; READ KEY X2";
#endif
		
			key = Address & 0xff;
			temp = key - 0x23;

			//CDVD_LOG("cdvdRead%d(Key%d) %x", key, temp, cdvd.Key[temp]);
			Output = _CDVD->DiskKey[temp];
			break;

		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
#ifdef INLINE_DEBUG_READ
	debug << "; READ KEY X3";
#endif
		
			key = Address & 0xff;
			temp = key - 0x26;

			//CDVD_LOG("cdvdRead%d(Key%d) %x", key, temp, cdvd.Key[temp]);
			Output = _CDVD->DiskKey[temp];
			break;

		case 0x38: 		// valid parts of key data (first and last are valid)
#ifdef INLINE_DEBUG_READ
	debug << "; READ KEY 15";
#endif

			//CDVD_LOG("cdvdRead38(KeysValid) %x", cdvd.Key[15]);

			Output = _CDVD->DiskKey[15];
			break;

		case 0x39:	// KEY-XOR
#ifdef INLINE_DEBUG_READ
	debug << "; READ KEY XOR";
#endif

			//CDVD_LOG("cdvdRead39(KeyXor) %x", cdvd.KeyXor);

			Output = _CDVD->KeyXor;
			break;

		case 0x3A: 	// DEC_SET
#ifdef INLINE_DEBUG_READ
	debug << "; READ DEC SET";
#endif

			//CDVD_LOG("cdvdRead3A(DecSet) %x", cdvd.decSet);

			Output = _CDVD->DecSet;
			break;
			
		default:
#ifdef INLINE_DEBUG_READ
	debug << "; UNKNOWN";
#endif

			Output = 0;
			break;
	}

	
#ifdef INLINE_DEBUG_READ
	debug << "; Output =" << hex << Output;
#endif

	return Output;
}


void CDVD::Write ( u32 Address, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nCDVD::Write " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << "; Data=" << Data;
#endif

	switch ( Address & 0xff )
	{
		case 0x4:
#ifdef INLINE_DEBUG_WRITE
	debug << "; NCOMMAND";
#endif

			_CDVD->NCommand = Data;
			
			_CDVD->Process_NCommand ( Data );
			
			break;
		
		case 0x5:
#ifdef INLINE_DEBUG_WRITE
	debug << "; NPARAM";
	debug << " " << dec << _CDVD->lNArgIndex;
#endif

			_CDVD->ucNArgBuffer [ _CDVD->lNArgIndex & c_iNArgBuffer_Mask ] = Data;
			_CDVD->lNArgIndex++;
			
			break;
			
			
		case 0x6:
#ifdef INLINE_DEBUG_WRITE
	debug << "; HOWTO??";
#endif

			// ?? must mean to enable data transfer via DMA
			// ..as in transfer immediately irregardless ??
			// 0x80 = "DataRequest"
			// or maybe bit 7 is a data request and bit...
			_CDVD->DataRequest = Data;
			
			/*
			if ( Data & 0x80 )
			{
				// enable transfer of data via dma
				CD::_CD->isSectorDataReady = true;
				
				// set the data buffer size to allow transfer
				CD::_CD->DataBuffer_Size = _CDVD->SectorReadSize;
				
				// now trigger dma transfer
				if ( Dma::_DMA->isActive ( 3 ) )
				{
#ifdef INLINE_DEBUG_WRITE
	debug << "; DMA3TRANSFER";
#endif

					// PS1 CD DMA is active, so trigger a transfer //
					Dma::_DMA->DMA3_Run ( true );
					
					// if all the data was transferred, then the next transfer is not ready yet
					if ( !CD::_CD->DataBuffer_Size )
					{
						CD::_CD->isSectorDataReady = false;
					}
				}
			}
			*/
			
			break;
			
		case 0x8:
#ifdef INLINE_DEBUG_WRITE
	debug << "; INTERRUPT ACKNOWLEDGE";
#endif

#ifdef ENABLE_READ_COMPLETE_INTERRUPT
			// if acknowledging an int 1, then re-interrupt with an int 2
			if ( _CDVD->InterruptReason == 1 )
			{
				_CDVD->InterruptReason &= ~Data;
				
				if ( !_CDVD->InterruptReason && !_CDVD->SectorReadCount )
				{
					// set the command to send a complete signal
					// will use 0x20 for now for complete
					//_CDVD->InterruptReason = 2;
					//SetInterrupt_CDVD ();
					if ( _CDVD->DataRequest & 0x4 )
					{
#ifdef INLINE_DEBUG_WRITE
	debug << "; SECONDAY READ INTERRUPT";
#endif

						//_CDVD->ReadCommand = 0x20;
						//_CDVD->Set_NextEvent ( c_ullSecondaryReadInterruptDelay );
						
						// the interrupt should probably happen near immediately
						_CDVD->InterruptReason = 2;
						SetInterrupt_CDVD ();
					}
				}
			}
			else
#endif
			{
				// clear the specified interrupt
				_CDVD->InterruptReason &= ~Data;
				
				// if this doesn't clear all the interrupts, then there should still be an interrupt signal
				//if ( _CDVD->InterruptReason )
				//{
				//	SetInterrupt_CDVD ();
				//}
			}
		
			break;
			
		case 0x16:	// SCOMMAND (looks like it's the command for the arguments sent to SDATAIN)
#ifdef INLINE_DEBUG_WRITE
	debug << "; SCOMMAND";
#endif
		
			_CDVD->SCommand = Data;
			
			_CDVD->Process_SCommand ( Data );
			
			break;
			
		case 0x17:	// SDATAIN ?? (looks like it is specifying parameters)
#ifdef INLINE_DEBUG_WRITE
	debug << "; SPARAM";
#endif

			_CDVD->ucSArgBuffer [ _CDVD->lSArgIndex & c_iSArgBuffer_Mask ] = Data;
			_CDVD->lSArgIndex++;
			break;
			
		case 0x3a:
#ifdef INLINE_DEBUG_WRITE
	debug << "; DECSET";
#endif

			// set the info for decrypting data from disk
			CD::_CD->DecryptSetting = Data;
			
			_CDVD->DecSet = Data;
			break;
			
			
		default:
#ifdef INLINE_DEBUG_WRITE
	debug << "; UNKNOWN";
#endif

			break;
	}
}

void CDVD::DMA_Read ( u32* Data, int ByteReadCount )
{
#ifdef INLINE_DEBUG
	debug << "\r\nCDVD::DMA_Read";
#endif

	Data [ 0 ] = 0;
}

void CDVD::DMA_Write ( u32* Data, int ByteWriteCount )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nCDVD::DMA_Write; Data = " << Data [ 0 ];
#endif
}




void CDVD::EnqueueResult ( u32 Size )
{
	// set the number of bytes in result
	lResultCount = Size;
	
	// start the reading from beginning of result buffer
	lResultIndex = 0;
	
	// data is ready in response/result buffer
	SReady &= ~0x40;
	
	// ?? Reset argument buffer index ??
	lSArgIndex = 0;
	
	// testing
	//NReady = 0x40;
}


bool CDVD::LoadNVMFile ( string FilePath )
{
	//static const char* c_sClassName = "CDVD";
	//static const char* c_sMethodName = "LoadNVMFile";
	static const char* c_sPrefix = "CDVD::LoadNVMFile: ";
	static const char* c_sErrorString = "CDVD::LoadNVMFile: Error loading NVM File.";
	static const char* c_sSuccessString = "CDVD::LoadNVMFile: Successfully loaded NVM File.";

#ifdef INLINE_DEBUG
	debug << "\r\nEntered function: LoadTestProgram";
#endif

	ifstream InputFile ( FilePath.c_str (), ios::binary );
	
	if ( !InputFile )
	{
#ifdef INLINE_DEBUG
	debug << c_sErrorString << "\n";
#endif

		cout << c_sErrorString << "\n";
		cout << "Path=" << FilePath.c_str () << "\n";
		return false;
	}


#ifdef INLINE_DEBUG
	debug << c_sPrefix << " loading NVM file." << "\n";
#endif

	// write entire program into memory
	//InputFile.read ( (char*) ( _BUS.BIOS.b32 ), BIOS_SIZE_IN_BYTES );
	InputFile.read ( (char*) ( NVM ), c_iNVMSize );
	
	InputFile.close();
	
#ifdef INLINE_DEBUG
	debug << c_sPrefix << " done." < "\n";
#endif
	
	cout << c_sSuccessString << "\n";
	cout << "Path=" << FilePath.c_str () << "\n";
	return true;
}


bool CDVD::Store_NVMFile ( string FilePath )
{
	FILE *f;
	
	f = fopen(FilePath.c_str(), "wb");
	if (f == NULL)
	{
		cout << "\nhps1x64: CDVD: Encountered problem saving NVM file.\n";
		return false;
	}
	
	fwrite ( (char*) ( NVM ), 1, c_iNVMSize, f );
	
	fclose (f);
	
	return true;
}


void CDVD::Process_MechaConCommand ( u8 Command )
{

	//switch ( _CDVD->ucSArgBuffer [ 0 ] )
	switch ( Command )
	{
		case 0x0:
#ifdef INLINE_DEBUG_WRITE
	debug << "; GetVersion";
#endif

			_CDVD->EnqueueResult ( 4 );
			_CDVD->ucResultBuffer[ 0 ] = 0x03;
			_CDVD->ucResultBuffer[ 1 ] = 0x06;
			_CDVD->ucResultBuffer[ 2 ] = 0x02;
			_CDVD->ucResultBuffer[ 3 ] = 0x00;
			break;
			
		case 0x44:
#ifdef INLINE_DEBUG_WRITE
	debug << "; SetConsoleID";
#endif
			// *** TODO ***
			_CDVD->EnqueueResult ( 1 );
			_CDVD->ucResultBuffer[ 0 ] = 0x00;
			break;
			
		case 0x45:
#ifdef INLINE_DEBUG_WRITE
	debug << "; GetConsoleID";
#endif
			// *** TODO ***
			_CDVD->EnqueueResult ( 9 );
			_CDVD->ucResultBuffer[ 0 ] = 0x00;
			_CDVD->ucResultBuffer[ 1 ] = 0x00;
			_CDVD->ucResultBuffer[ 2 ] = 0x00;
			_CDVD->ucResultBuffer[ 3 ] = 0x00;
			_CDVD->ucResultBuffer[ 4 ] = 0x00;
			_CDVD->ucResultBuffer[ 5 ] = 0x00;
			_CDVD->ucResultBuffer[ 6 ] = 0x00;
			_CDVD->ucResultBuffer[ 7 ] = 0x00;
			_CDVD->ucResultBuffer[ 8 ] = 0x00;
			break;
			
		case 0xfd:
#ifdef INLINE_DEBUG_WRITE
	debug << "; GetRenewalDate";
#endif

			_CDVD->EnqueueResult ( 6 );
			_CDVD->ucResultBuffer[ 0 ] = 0x00;
			_CDVD->ucResultBuffer[ 1 ] = 0x04;	// year
			_CDVD->ucResultBuffer[ 2 ] = 0x12;	// day
			_CDVD->ucResultBuffer[ 3 ] = 0x10;	// month
			_CDVD->ucResultBuffer[ 4 ] = 0x01;	// hour
			_CDVD->ucResultBuffer[ 5 ] = 0x30;	// min
			
			break;
			
		default:
#ifdef INLINE_DEBUG_WRITE
	debug << "; Unknown MECHA-CON Command";
#endif

			_CDVD->EnqueueResult ( 1 );
			_CDVD->ucResultBuffer[ 0 ] = 0x80;
			break;
	}
}


void CDVD::Process_SCommand ( u8 Command )
{
	static const u64 ullCyclesPerSecond = 36864000ULL;
	static const u64 ullCyclesPerMinute = 36864000ULL * 60ULL;
	static const u64 ullCyclesPerHour = 36864000ULL * 60ULL * 60ULL;

	//switch ( Data & 0xff )
	switch ( Command )
	{
		case 0x2:	//READ SUBQ
#ifdef INLINE_DEBUG_WRITE
	debug << "; READ SUBQ";
#endif

			break;
			
		case 0x3:	// MECHA-CON COMMAND ??
#ifdef INLINE_DEBUG_WRITE
	debug << "; MECHA-CON???";
#endif

			_CDVD->Process_MechaConCommand ( _CDVD->ucSArgBuffer [ 0 ] );


			break;
			
		case 0x5:	// TRAY STATE REQUEST
#ifdef INLINE_DEBUG_WRITE
	debug << "; TRAY STATE";
#endif

			// either 1 for open or 0 for closed
			// for now we'll say its closed
			_CDVD->EnqueueResult ( 1 );
			_CDVD->ucResultBuffer[ 0 ] = 0x0;
			
			break;
			
		case 0x6:	// TRAY CTRL
#ifdef INLINE_DEBUG_WRITE
	debug << "; TRAY CTRL";
#endif


			break;
			
		case 0x8:	// READ RTC
#ifdef INLINE_DEBUG_WRITE
	debug << "; READ RTC";
#endif


			//SetResultSize(8);
			_CDVD->EnqueueResult ( 8 );
			
			/*
			cdvd.Result[0] = 0;
			cdvd.Result[1] = itob(cdvd.RTC.second); //Seconds
			cdvd.Result[2] = itob(cdvd.RTC.minute); //Minutes
			cdvd.Result[3] = itob(cdvd.RTC.hour); //Hours
			cdvd.Result[4] = 0; //Nothing
			cdvd.Result[5] = itob(cdvd.RTC.day); //Day
			cdvd.Result[6] = itob(cdvd.RTC.month); //Month
			cdvd.Result[7] = itob(cdvd.RTC.year); //Year
			*/
			
			// ***TODO*** Output correct time RTC time value
			_CDVD->ucResultBuffer [ 0 ] = 0;
			
			// seconds
			// Cycle / CyclesPerSecond
			// DiskImage::CDImage::ConvertDecToBCD8
			_CDVD->ucResultBuffer [ 1 ] = DiskImage::CDImage::ConvertDecToBCD8 ( ( *_DebugCycleCount / ullCyclesPerSecond ) % 60 );
			
			// minutes
			// Cycles / CyclesPerMinute
			_CDVD->ucResultBuffer [ 2 ] = DiskImage::CDImage::ConvertDecToBCD8 ( ( *_DebugCycleCount / ullCyclesPerMinute ) % 60 );
			
			// hours
			// Cycles / CyclesPerHour
			_CDVD->ucResultBuffer [ 3 ] = DiskImage::CDImage::ConvertDecToBCD8 ( *_DebugCycleCount / ullCyclesPerHour );
			
			// zero
			_CDVD->ucResultBuffer [ 4 ] = 0;
			
			// Day
			_CDVD->ucResultBuffer [ 5 ] = 1;
			
			// Month
			_CDVD->ucResultBuffer [ 6 ] = 1;
			
			// year
			_CDVD->ucResultBuffer [ 7 ] = 0x14;
			
			break;
			
		case 0x9:	// WRITE RTC
#ifdef INLINE_DEBUG_WRITE
	debug << "; WRITE RTC";
#endif

			//SetResultSize(1);
			_CDVD->EnqueueResult ( 1 );
			
			//cdvd.Result[0] = 0;
			_CDVD->ucResultBuffer [ 0 ] = 0;
			
			// ***TODO*** write RTC Value
			/*
			cdvd.RTC.pad = 0;
			cdvd.RTC.second = btoi(cdvd.Param[cdvd.ParamP-7]);
			cdvd.RTC.minute = btoi(cdvd.Param[cdvd.ParamP-6]) % 60;
			cdvd.RTC.hour = btoi(cdvd.Param[cdvd.ParamP-5]) % 24;
			cdvd.RTC.day = btoi(cdvd.Param[cdvd.ParamP-3]);
			cdvd.RTC.month = btoi(cdvd.Param[cdvd.ParamP-2] & 0x7f);
			cdvd.RTC.year = btoi(cdvd.Param[cdvd.ParamP-1]);
			*/
			
			break;
		
		// sceCdForbidDVDP (0:1)
		case 0x15:
#ifdef INLINE_DEBUG_WRITE
	debug << "; SCE FORBID DVDP";
#endif

			//SetResultSize(1);
			_CDVD->EnqueueResult ( 1 );
			
			//cdvd.Result[0] = 5;
			_CDVD->ucResultBuffer [ 0 ] = 5;
			
			break;
		
		// CdReadModelNumber (1:9) - from xcdvdman
		case 0x17:
#ifdef INLINE_DEBUG_WRITE
	debug << "; READ MODEL NUMBER";
#endif

			//SetResultSize(9);
			_CDVD->EnqueueResult ( 9 );
			
			//cdvdReadModelNumber(&cdvd.Result[1], cdvd.Param[0]);
			memcpy ( & _CDVD->ucResultBuffer [ 1 ], & ( _CDVD->NVM [ _CDVD->ucSArgBuffer [ 0 ] + nvmlayouts [ c_iBiosVersion ].modelNum ] ), 8 );
			
			break;
			
		// sceCdBootCertify (4:1)//(4:16 in psx?)
		case 0x1a:
#ifdef INLINE_DEBUG_WRITE
	debug << "; SCE BOOT CERTIFY";
#endif

			//on input there are 4 bytes: 1;?10;J;C for 18000; 1;60;E;C for 39002 from ROMVER
			//SetResultSize(1);
			_CDVD->EnqueueResult ( 1 );
			
			//i guess that means okay
			//cdvd.Result[0] = 1;
			_CDVD->ucResultBuffer [ 0 ] = 1;
			
			break;
			
		// sceCdCancelPOffRdy (0:1)
		case 0x1b:
			_CDVD->EnqueueResult ( 1 );
			_CDVD->ucResultBuffer [ 0 ] = 0;
			break;
			
		// sceRemote2Read
		case 0x1e:
#ifdef INLINE_DEBUG_WRITE
	debug << "; Remote2Read";
#endif

			_CDVD->EnqueueResult ( 5 );
			
			_CDVD->ucResultBuffer [ 0 ] = 0x00;
			_CDVD->ucResultBuffer [ 1 ] = 0x14;
			_CDVD->ucResultBuffer [ 2 ] = 0x00;
			_CDVD->ucResultBuffer [ 3 ] = 0x00;
			_CDVD->ucResultBuffer [ 4 ] = 0x00;
			break;
			
		// sceCdReadWakeUpTime
		case 0x22:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CdReadWakeUpTime";
#endif
		
			_CDVD->EnqueueResult ( 10 );
			
			_CDVD->ucResultBuffer [ 0 ] = 0;
			_CDVD->ucResultBuffer [ 1 ] = 0;
			_CDVD->ucResultBuffer [ 2 ] = 0;
			_CDVD->ucResultBuffer [ 3 ] = 0;
			_CDVD->ucResultBuffer [ 4 ] = 0;
			_CDVD->ucResultBuffer [ 5 ] = 0;
			_CDVD->ucResultBuffer [ 6 ] = 0;
			_CDVD->ucResultBuffer [ 7 ] = 0;
			_CDVD->ucResultBuffer [ 8 ] = 0;
			_CDVD->ucResultBuffer [ 9 ] = 0;
			
			break;
			
			
		// sceCdRCBypassCtrl
		case 0x24:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CdRCBypassCtrl";
#endif
		
			_CDVD->EnqueueResult ( 1 );
			
			_CDVD->ucResultBuffer [ 0 ] = 0;
			break;
			
		// region??
		case 0x36:
#ifdef INLINE_DEBUG_WRITE
	debug << "; REGION??";
#endif

			_CDVD->EnqueueResult ( 15 );
			
			// ???
			_CDVD->ucResultBuffer [ 0 ] = 0;
			
			_CDVD->ucResultBuffer [ 1 ] = 1 << 3;
			_CDVD->ucResultBuffer [ 2 ] = 0;
			
			
			// region ??? 8 values ???
			//memcpy ( & _CDVD->ucResultBuffer [ 3 ], & ( _CDVD->NVM [ nvmlayouts [ c_iBiosVersion ].regparams ] ), 8 );
			switch ( _CDVD->Region )
			{
				case 'J':
					memcpy ( & _CDVD->ucResultBuffer [ 3 ], _CDVD->c_cRegion_JAP, 8 );
					break;
					
				case 'A':
					memcpy ( & _CDVD->ucResultBuffer [ 3 ], _CDVD->c_cRegion_US, 8 );
					break;
					
				case 'E':
					memcpy ( & _CDVD->ucResultBuffer [ 3 ], _CDVD->c_cRegion_EU, 8 );
					break;
					
				case 'H':
					memcpy ( & _CDVD->ucResultBuffer [ 3 ], _CDVD->c_cRegion_H, 8 );
					break;
					
				case 'R':
					memcpy ( & _CDVD->ucResultBuffer [ 3 ], _CDVD->c_cRegion_R, 8 );
					break;
					
				case 'C':
					memcpy ( & _CDVD->ucResultBuffer [ 3 ], _CDVD->c_cRegion_C, 8 );
					break;
					
				case 'K':
					memcpy ( & _CDVD->ucResultBuffer [ 3 ], _CDVD->c_cRegion_Korea, 8 );
					break;
			}
			
			//_CDVD->ucResultBuffer [ 3 ] = 0;
			//_CDVD->ucResultBuffer [ 4 ] = 'E';
			//_CDVD->ucResultBuffer [ 5 ] = 'e';
			//_CDVD->ucResultBuffer [ 6 ] = 'n';
			//_CDVD->ucResultBuffer [ 7 ] = 'g';
			//_CDVD->ucResultBuffer [ 8 ] = 0;
			//_CDVD->ucResultBuffer [ 9 ] = 'O';
			//_CDVD->ucResultBuffer [ 10 ] = 0;
			
			
			// ?????
			_CDVD->ucResultBuffer [ 11 ] = 0;
			_CDVD->ucResultBuffer [ 12 ] = 0;
			_CDVD->ucResultBuffer [ 13 ] = 0;
			_CDVD->ucResultBuffer [ 14 ] = 0;
			break;
			
			
		
		case 0x40:	// OPEN CONFIG
#ifdef INLINE_DEBUG_WRITE
	debug << "; OPEN CONFIG";
#endif

			_CDVD->ReadWrite = _CDVD->ucSArgBuffer [ 0 ];
			_CDVD->Offset = _CDVD->ucSArgBuffer [ 1 ];
			_CDVD->NumBlocks = _CDVD->ucSArgBuffer [ 2 ];
			_CDVD->BlockIndex = 0;
			
			_CDVD->ucResultBuffer [ 0 ] = 0;
			
			// data is ready in response buffer
			_CDVD->EnqueueResult ( 1 );
			
			break;
			
		case 0x41:	// READ CONFIG
#ifdef INLINE_DEBUG_WRITE
	debug << "; READ CONFIG";
#endif

			// 16 byte result
			_CDVD->EnqueueResult ( 16 );

			if( _CDVD->ReadWrite != 0 )
			{
				//cout << "\nset to write mode";
				_CDVD->ucResultBuffer[0] = 0x80;
				memset(&_CDVD->ucResultBuffer[1], 0x00, 15);
				return;
			}
			// check if block index is in bounds
			else if( _CDVD->BlockIndex >= _CDVD->NumBlocks )
				return;
			else if(
				((_CDVD->Offset == 0) && (_CDVD->BlockIndex >= 4))||
				((_CDVD->Offset == 1) && (_CDVD->BlockIndex >= 2))||
				((_CDVD->Offset == 2) && (_CDVD->BlockIndex >= 7))
				)
			{
				//cout << "\nconfig#" << _CDVD->Offset << " clearing";
				memset(_CDVD->ucResultBuffer, 0x00, 16);
				return;
			}

			// get config data
			switch (_CDVD->Offset)
			{
				case 0:
					memcpy ( _CDVD->ucResultBuffer, & ( _CDVD->NVM [ ( _CDVD->BlockIndex * 16 ) + nvmlayouts [ c_iBiosVersion ].config0 ] ), 16 );
					_CDVD->BlockIndex++;
					break;
					
				case 2:
					memcpy ( _CDVD->ucResultBuffer, & ( _CDVD->NVM [ ( _CDVD->BlockIndex * 16 ) + nvmlayouts [ c_iBiosVersion ].config2 ] ), 16 );
					_CDVD->BlockIndex++;
					break;
					
				default:
					memcpy ( _CDVD->ucResultBuffer, & ( _CDVD->NVM [ ( _CDVD->BlockIndex * 16 ) + nvmlayouts [ c_iBiosVersion ].config1 ] ), 16 );
					_CDVD->BlockIndex++;
					break;
			}

			break;
			
		case 0x42:	// WRITE CONFIG
#ifdef INLINE_DEBUG_WRITE
	debug << "; WRITE CONFIG";
#endif

			_CDVD->EnqueueResult ( 1 );

			// make sure its in write mode && the block index is in bounds
			if ( ( _CDVD->ReadWrite != 1 ) || ( _CDVD->BlockIndex >= _CDVD->NumBlocks ) )
			{
				_CDVD->ucResultBuffer [ 0 ] = 1;
			}
			else if(
				((_CDVD->Offset == 0) && (_CDVD->BlockIndex >= 4))||
				((_CDVD->Offset == 1) && (_CDVD->BlockIndex >= 2))||
				((_CDVD->Offset == 2) && (_CDVD->BlockIndex >= 7))
				)
			{
				_CDVD->ucResultBuffer [ 0 ] = 0;
			}
			else
			{

				// set config data
				switch (_CDVD->Offset)
				{
					case 0:
						memcpy ( & ( _CDVD->NVM [ ( _CDVD->BlockIndex * 16 ) + nvmlayouts [ c_iBiosVersion ].config0 ] ), _CDVD->ucSArgBuffer, 16 );
						_CDVD->BlockIndex++;
						break;
						
					case 2:
						memcpy ( & ( _CDVD->NVM [ ( _CDVD->BlockIndex * 16 ) + nvmlayouts [ c_iBiosVersion ].config2 ] ), _CDVD->ucSArgBuffer, 16 );
						_CDVD->BlockIndex++;
						break;
						
					default:
						memcpy ( & ( _CDVD->NVM [ ( _CDVD->BlockIndex * 16 ) + nvmlayouts [ c_iBiosVersion ].config1 ] ), _CDVD->ucSArgBuffer, 16 );
						_CDVD->BlockIndex++;
						break;
				}
				
				_CDVD->ucResultBuffer [ 0 ] = 0;
				
			}
			
			break;
			
			
		case 0x43:	// CLOSE CONFIG
#ifdef INLINE_DEBUG_WRITE
	debug << "; CLOSE CONFIG";
#endif

			_CDVD->ucResultBuffer [ 0 ] = 0;
			_CDVD->EnqueueResult ( 1 );
			break;
			
			
		// secrman: __mechacon_auth_0x80
		case 0x80:
#ifdef INLINE_DEBUG_WRITE
	debug << "; MECHACON AUTH 0x80";
#endif
			_CDVD->mg_datatype = 0;
			_CDVD->ucResultBuffer [ 0 ] = 0;
			_CDVD->EnqueueResult ( 1 );
			break;
			
		case 0x81: // secrman: __mechacon_auth_0x81
#ifdef INLINE_DEBUG_WRITE
	debug << "; MECHACON AUTH 0x81";
#endif
			_CDVD->mg_datatype = 0;
			_CDVD->ucResultBuffer [ 0 ] = 0;
			_CDVD->EnqueueResult ( 1 );
			break;

		case 0x82: // secrman: __mechacon_auth_0x82
#ifdef INLINE_DEBUG_WRITE
	debug << "; MECHACON AUTH 0x82";
#endif
			_CDVD->ucResultBuffer [ 0 ] = 0;
			_CDVD->EnqueueResult ( 1 );
			break;

		case 0x83: // secrman: __mechacon_auth_0x83
#ifdef INLINE_DEBUG_WRITE
	debug << "; MECHACON AUTH 0x83";
#endif
			_CDVD->ucResultBuffer [ 0 ] = 0;
			_CDVD->EnqueueResult ( 1 );
			break;

		case 0x84: // secrman: __mechacon_auth_0x84
#ifdef INLINE_DEBUG_WRITE
	debug << "; MECHACON AUTH 0x84";
#endif
			_CDVD->EnqueueResult (1+8+4);
			_CDVD->ucResultBuffer [0] = 0;

			_CDVD->ucResultBuffer [1] = 0x21;
			_CDVD->ucResultBuffer [2] = 0xdc;
			_CDVD->ucResultBuffer [3] = 0x31;
			_CDVD->ucResultBuffer [4] = 0x96;
			_CDVD->ucResultBuffer [5] = 0xce;
			_CDVD->ucResultBuffer [6] = 0x72;
			_CDVD->ucResultBuffer [7] = 0xe0;
			_CDVD->ucResultBuffer [8] = 0xc8;

			_CDVD->ucResultBuffer [9]  = 0x69;
			_CDVD->ucResultBuffer [10] = 0xda;
			_CDVD->ucResultBuffer [11] = 0x34;
			_CDVD->ucResultBuffer [12] = 0x9b;
			break;

		case 0x85: // secrman: __mechacon_auth_0x85
#ifdef INLINE_DEBUG_WRITE
	debug << "; MECHACON AUTH 0x85";
#endif
			_CDVD->EnqueueResult (1+4+8);
			_CDVD->ucResultBuffer[0] = 0;

			_CDVD->ucResultBuffer [1] = 0xeb;
			_CDVD->ucResultBuffer [2] = 0x01;
			_CDVD->ucResultBuffer [3] = 0xc7;
			_CDVD->ucResultBuffer [4] = 0xa9;

			_CDVD->ucResultBuffer [ 5] = 0x3f;
			_CDVD->ucResultBuffer [ 6] = 0x9c;
			_CDVD->ucResultBuffer [ 7] = 0x5b;
			_CDVD->ucResultBuffer [ 8] = 0x19;
			_CDVD->ucResultBuffer [ 9] = 0x31;
			_CDVD->ucResultBuffer [10] = 0xa0;
			_CDVD->ucResultBuffer [11] = 0xb3;
			_CDVD->ucResultBuffer [12] = 0xa3;
			break;

		case 0x86: // secrman: __mechacon_auth_0x86
#ifdef INLINE_DEBUG_WRITE
	debug << "; MECHACON AUTH 0x86";
#endif
			_CDVD->ucResultBuffer [ 0 ] = 0;
			_CDVD->EnqueueResult ( 1 );
			break;

		case 0x87: // secrman: __mechacon_auth_0x87
#ifdef INLINE_DEBUG_WRITE
	debug << "; MECHACON AUTH 0x87";
#endif
			_CDVD->ucResultBuffer [ 0 ] = 0;
			_CDVD->EnqueueResult ( 1 );
			break;
			
		case 0x88: // secrman: __mechacon_auth_0x88	//for now it is the same; so, fall;)
		case 0x8F: // secrman: __mechacon_auth_0x8F
#ifdef INLINE_DEBUG_WRITE
	debug << "; MECHACON AUTH 0x88/0x8f";
#endif

			//SetResultSize(1);//in:0
			_CDVD->EnqueueResult ( 1 );
			
			//if (cdvd.mg_datatype == 1) // header data
			if ( _CDVD->mg_datatype == 1 )
			{
				// *** TODO ***
				cout << "\nhps1x64: CDVD: TODO: mg_datatype = 1!!!\n";
				/*
				u64* psrc, *pdst;
				int bit_ofs, i;

				if ((cdvd.mg_maxsize != cdvd.mg_size)||(cdvd.mg_size < 0x20) || (cdvd.mg_size != *(u16*)&cdvd.mg_buffer[0x14]))
				{
					fail_pol_cal();
					break;
				}

				std::string zoneStr;
				for (i=0; i<8; i++)
				{
					if (cdvd.mg_buffer[0x1C] & (1<<i)) zoneStr += mg_zones[i];
				}

				Console.WriteLn("[MG] ELF_size=0x%X Hdr_size=0x%X unk=0x%X flags=0x%X count=%d zones=%s",
					*(u32*)&cdvd.mg_buffer[0x10], *(u16*)&cdvd.mg_buffer[0x14], *(u16*)&cdvd.mg_buffer[0x16],
					*(u16*)&cdvd.mg_buffer[0x18], *(u16*)&cdvd.mg_buffer[0x1A],
					zoneStr.c_str()
				);

				bit_ofs = mg_BIToffset(cdvd.mg_buffer);

				psrc = (u64*)&cdvd.mg_buffer[bit_ofs-0x20];

				pdst = (u64*)cdvd.mg_kbit;
				pdst[0] = psrc[0];
				pdst[1] = psrc[1];
				//memcpy(cdvd.mg_kbit, &cdvd.mg_buffer[bit_ofs-0x20], 0x10);

				pdst = (u64*)cdvd.mg_kcon;
				pdst[0] = psrc[2];
				pdst[1] = psrc[3];
				//memcpy(cdvd.mg_kcon, &cdvd.mg_buffer[bit_ofs-0x10], 0x10);

				if ((cdvd.mg_buffer[bit_ofs+5] || cdvd.mg_buffer[bit_ofs+6] || cdvd.mg_buffer[bit_ofs+7]) ||
					(cdvd.mg_buffer[bit_ofs+4] * 16 + bit_ofs + 8 + 16 != *(u16*)&cdvd.mg_buffer[0x14]))
				{
					fail_pol_cal();
					break;
				}
				*/
			}
			
			_CDVD->ucResultBuffer [0] = 0; // 0 complete ; 1 busy ; 0x80 error
			break;
			
		default:
#ifdef INLINE_DEBUG_WRITE
	debug << "; UNKNOWN 0x16 Command";
#endif

			break;
	}
}


void CDVD::Process_NCommand ( u8 Command )
{
	s32 PrevSector, sSeekDelta;
	u32 uSeekDelta;
	u64 Temp_SeekTime;
	u64 Temp_ReadTime;
	u64 ullCycleDiff;
	double dSectorsRead, dCurrentSector;
	
	u64 ullDiskReadCycles;
	u64 ullTargetCycle;
	
	
	u8 arg0;
	u16 arg1;
	u32 arg2;
	
	char* DiskSerial;
	
	s32 numbers, letters;
	u32 key_0_3;
	u8 key_4, key_14;
	
	char NumbersStr [ 16 ];
	
	//switch ( Data & 0xff )
	switch ( Command )
	{
		// SYNC (or motor on?)
		case 0:

		// NOP
		case 1:
#ifdef INLINE_DEBUG_WRITE
	debug << "; SYNC/NOP";
#endif

			cout << "\n*** SYNC/NOP ?? ***\n";
			
			// not ready
			NReady = 0;
			
			// set the read command as what the command was
			_CDVD->ReadCommand = Command;
			
			_CDVD->Set_NextEvent ( c_lDiskNop_Cycles );
		
			// reset input fifo for N Commands
			_CDVD->lNArgIndex = 0;
			
			break;
			
		
		// STANDBY
		case 0x2:
		
			cout << "\n*** STANDBY ***\n";
			
			// not ready
			NReady = 0;
			
			// status is now "pause" ??
			Status = CDVD_STATUS_PAUSE;
			
			_CDVD->ReadCommand = Command;
			_CDVD->Set_NextEvent ( c_lDiskStandby_Cycles );
			
			// reset input fifo for N Commands
			_CDVD->lNArgIndex = 0;
			
			break;
			
		// STOP
		case 0x3:
		
			cout << "\n*** STOP ***\n";
			
			// not ready
			NReady = 0;
			
			_CDVD->ReadCommand = Command;
			
			_CDVD->Set_NextEvent ( c_lDiskStop_Cycles );
			
			// reset input fifo for N Commands
			_CDVD->lNArgIndex = 0;
			
			break;
			
		// PAUSE
		case 0x4:
#ifdef INLINE_DEBUG_WRITE
	debug << "; PAUSE";
#endif

			cout << "\n*** PAUSE ***\n";
			
			// not ready
			NReady = 0;
			
			// set the read command as pause
			_CDVD->ReadCommand = 0x4;
			
			// ***todo*** pause might take longer
			_CDVD->Set_NextEvent ( c_lDiskPause_Cycles );
			
			// reset input fifo for N Commands
			_CDVD->lNArgIndex = 0;
			
			break;
			
		
		// SEEK
		case 0x5:
#ifdef INLINE_DEBUG_WRITE
	debug << "; SEEK";
#endif

			cout << "\n*** SEEK ***\n";
			
			// not ready
			NReady = 0;


			// no sectors available yet
			AvailableSectorCount = 0;
			
			
			PrevSector = SeekSectorNum;
			
			SeekSectorNum = ( (u32*) _CDVD->ucNArgBuffer ) [ 0 ];
			
			// get difference from last read sector
			sSeekDelta = PrevSector - SeekSectorNum;
			
			// get absolute value
			uSeekDelta = ( sSeekDelta < 0 ) ? -sSeekDelta : sSeekDelta;
			
			// seek to sector
			CD::_CD->cd_image.SeekSector ( _CDVD->SeekSectorNum + DiskImage::CDImage::c_SectorsInFirstTwoSeconds );
			CD::_CD->cd_image.StartReading ();
			
			// set the read command as SEEK
			_CDVD->ReadCommand = 0x5;
			
			// status is now "seek" ??
			Status = CDVD_STATUS_SEEK;

			
			// *new* set the sector being read from
			ReadSector = SeekSectorNum;

			
			// check if this will be a short seek, longer seek, or longer running seek
			if ( uSeekDelta <= c_lNoSeekDelta )
			{
				// virtually no seek time
				Set_NextEvent ( c_llCDNoSeekInCycles );
				Temp_SeekTime = c_llCDNoSeekInCycles;
				
				// *new* not seeking, set to read
				//Status = CDVD_STATUS_READ;
			}
			else if ( uSeekDelta <= c_lFastCDSeekDelta )
			{
				// fast seek
				Set_NextEvent ( c_llCDFastSeekInCycles );
				Temp_SeekTime = c_llCDFastSeekInCycles;
				//Set_NextEvent ( c_llCDSeek_CyclesPerSector * SeekDelta );
				//Temp_SeekTime = c_llCDSeek_CyclesPerSector * SeekDelta;
				
				// set the sector and cycle the read is starting at
				ullReadStart_Sector = SeekSectorNum;
				ullReadStart_Cycle = *_DebugCycleCount + c_llCDFastSeekInCycles;
			}
			else
			{
				// long running seek
				Set_NextEvent ( c_llCDSeekInCycles );
				Temp_SeekTime = c_llCDSeekInCycles;
				//Set_NextEvent ( c_llCDSeek_CyclesPerSector * SeekDelta );
				//Temp_SeekTime = c_llCDSeek_CyclesPerSector * SeekDelta;
				
				// set the sector and cycle the read is starting at
				ullReadStart_Sector = SeekSectorNum;
				ullReadStart_Cycle = *_DebugCycleCount + c_llCDSeekInCycles;
			}
			
#ifdef INLINE_DEBUG_WRITE
			debug << "\r\nSEEK: to Sector#" << dec << SeekSectorNum << " SeekDelta=" << sSeekDelta << " PrevSector=" << PrevSector << " SeekTime=" << Temp_SeekTime;
#endif

			cout << "\nSEEK: to Sector#" << dec << SeekSectorNum << " SeekDelta=" << sSeekDelta << " PrevSector=" << PrevSector << " SeekTime=" << Temp_SeekTime;
			
			// reset input fifo for N Commands
			_CDVD->lNArgIndex = 0;
			
			break;
			
			
		// CD READ (reads data from CD)
		case 0x6:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CD READ";
#endif

			// read interrupt now pending
			bPendingReadInt = true;

			// not ready
			NReady = 0x80;

			// not reading any sectors yet
			NumSectorsBeingRead = 0;

			// no sectors available yet
			AvailableSectorCount = 0;
			
			// start from beginning for dma
			DMAReadIndex = 0;
			
			//PrevSector = SeekSectorNum;
			PrevSector = ReadSector;
			
			SeekSectorNum = ( (u32*) ucNArgBuffer ) [ 0 ];
			SectorReadCount = ( (u32*) ucNArgBuffer ) [ 1 ];
			

#ifdef INLINE_DEBUG_DISKREAD
			debugRead << "\r\nCDREAD: SectorReadCount=" << dec << SectorReadCount << " SeekSectorNum=" << SeekSectorNum;
#endif

			// check for zero sector read
			if ( !SectorReadCount )
			{
				// can't read zero sectors ??
				SectorReadCount = 1;
			}
			
			// set the next sector to interrupt at ??
			ulNextSectorInt = SectorReadCount - c_ulIntSectors;
			
			
			// get the read speed
			DiskSpeedType = ucNArgBuffer [ 9 ];
			
#ifdef TEST_DISKSPEED
			DiskSpeed = 1;
#else
			// ***todo*** 0x83 might mean x4 for dvd ???
			// maybe the 0x80 or'ed with the disk speed means to read data, and otherwise means to stream
			switch ( DiskSpeedType & 0xf )
			{
				case 0x0:
					DiskSpeed = 1.0;
					break;
				
				case 0x1:
					DiskSpeed = 2.0;
					break;
					
				case 0x2:
					// possibly means x24 ??
					// means x12 ??
					//DiskSpeed = 24;
					DiskSpeed = 4.0;
					//if ( CurrentDiskType == CDVD_TYPE_PS2DVD ) DiskSpeed = 1.0;
					break;
					
				case 0x3:
					// means x4
					DiskSpeed = 4.0;
					//if ( CurrentDiskType == CDVD_TYPE_PS2DVD ) DiskSpeed = 2.0;
					// or possibly means x12 ?? or 8 ??
					//DiskSpeed = 12;
					break;
					
				case 0x4:
					// probably means x4 ??
					DiskSpeed = 12.0;
					//DiskSpeed = 4.0;
					//if ( CurrentDiskType == CDVD_TYPE_PS2DVD ) DiskSpeed = 4.0;
					//DiskSpeed = 24.0;
					break;
					
				
				case 0x5:
					// means x24
					// this must be x2 ??
					DiskSpeed = 24.0;
					//if ( CurrentDiskType == CDVD_TYPE_PS2DVD ) DiskSpeed = 4.0;
					//DiskSpeed = 2;
					break;
				
					
				default:
					// unknown
					cout << "\nhps2x64: CDVD: ***ALERT***: Unknown disk speed set=" << hex << DiskSpeed << "\n";
					DiskSpeed = 24;
					//if ( CurrentDiskType == CDVD_TYPE_PS2DVD ) DiskSpeed = 4.0;
					break;
			}

			if ( CurrentDiskType == CDVD_TYPE_PS2DVD ) DiskSpeed = ( DiskSpeedType & 0x3 ) + 1;

#ifdef ENABLE_INITIAL_MAX_SPEED
			// *** temporary testing ***
			if ( DiskSpeedType & 0x80 )
			{
				DiskSpeed = 24;
				if ( CurrentDiskType == CDVD_TYPE_PS2DVD ) DiskSpeed = 4.0;
			}
#endif

#endif
			
#ifdef INLINE_DEBUG_DISKREAD
			debugRead << " DiskSpeed=" << dec << DiskSpeed;
#endif
			
			
			// set the time to read one sector of  data from disk
			dDiskReadCycleTime = c_dCDReadx1_Cycles / DiskSpeed;
			if ( CurrentDiskType == CDVD_TYPE_PS2DVD ) dDiskReadCycleTime = c_dDVDReadx1_Cycles / DiskSpeed;

			ullDiskReadCycles = (u64) ( dDiskReadCycleTime * SectorReadCount );
			
			// *** temporary testing ***
			//dDiskReadCycleTime = 2.0L;
			
			
			// get difference from last read sector
			sSeekDelta = SeekSectorNum - PrevSector;
			
			// get absolute value
			uSeekDelta = ( sSeekDelta < 0 ) ? -sSeekDelta : sSeekDelta;
			
			
#ifdef INLINE_DEBUG_DISKREAD
			debugRead << " SeekDelta=" << uSeekDelta;
#endif
			
			// byte 8 is retry count ??
			// byte 9 spndlctrl ??
			
			// byte 10 is the sector read size (0: 2048, 1: 2328, 2: 2340)
			switch ( _CDVD->ucNArgBuffer [ 10 ] )
			{
				case 0:
				
					_CDVD->SectorReadSize = 2048;
					
					// ???
					// if CD then read offset would be 24
					if ( _CDVD->CurrentDiskType == CDVD_TYPE_PS2DVD )
					{
						CD::_CD->ReadMode_Offset = 0;
					}
					else
					{
						CD::_CD->ReadMode_Offset = 24;
					}
					break;
					
				case 1:
					_CDVD->SectorReadSize = 2328;
					CD::_CD->ReadMode_Offset = 24;
					break;
					
				case 2:
					_CDVD->SectorReadSize = 2340;
					CD::_CD->ReadMode_Offset = 12;
					break;
					
				default:
					_CDVD->SectorReadSize = 2048;
					CD::_CD->ReadMode_Offset = 24;
					break;
			}
			
#ifdef INLINE_DEBUG_DISKREAD
			debugRead << " After switch";
#endif

			// set the sector size for ps1 cd handler
			CD::_CD->DataBuffer_Size = _CDVD->SectorReadSize;
			CD::_CD->ReadMode = _CDVD->SectorReadSize;
			
#ifdef INLINE_DEBUG_DISKREAD
			debugRead << " After switch2";
#endif

			// reset the index on ps1 side
			CD::_CD->DataBuffer_Index = 0;
			
#ifdef INLINE_DEBUG_DISKREAD
			debugRead << " After switch3";
#endif

			// seek to sector
			if ( sSeekDelta )
			{
cout << "\n->SEEK->";
			CD::_CD->cd_image.SeekSector ( _CDVD->SeekSectorNum + DiskImage::CDImage::c_SectorsInFirstTwoSeconds );
			
#ifdef INLINE_DEBUG_DISKREAD
			debugRead << " After switch4";
#endif

			CD::_CD->cd_image.StartReading ();
			}
			
#ifdef INLINE_DEBUG_DISKREAD
			debugRead << " After switch5";
#endif

			// set the read command as cd read
			_CDVD->ReadCommand = 0x6;
			
			// status is now "seek" ??
			Status = CDVD_STATUS_SEEK;

			
			// *new* set the sector being read from
			ReadSector = SeekSectorNum;

			// init read time
			Temp_ReadTime = 0;
			Temp_SeekTime = 0;

			iTotalAvailableSectors = 0;
			iTotalReadSectors = 0;
			iTotalUnreadSectors = 0;


#ifdef INLINE_DEBUG_DISKREAD
			debugRead << " BeforeSeek";
#endif

			// check if this will be a short seek, longer seek, or longer running seek
			//if ( DiskSpeedType & 0x80 )
			//{

#ifdef TEST_DISKSEEK
			Status = CDVD_STATUS_SEEK;
			Temp_SeekTime = c_llCDSeekInCycles;
			Temp_ReadTime = ullDiskReadCycles;
			Set_NextEvent ( Temp_SeekTime );
#else
			//if ( ((u64)sSeekDelta) <= c_lNoSeekDelta )
			if ( uSeekDelta <= c_lNoSeekDelta )
			{

				iTotalAvailableSectors = ( ( *_DebugCycleCount - ullLastSeekCycle ) / dDiskReadCycleTime );
				iTotalReadSectors = SeekSectorNum - lLastSeekSector;
				iTotalUnreadSectors = iTotalAvailableSectors - iTotalReadSectors;

#ifdef ADJUST_STREAM_SPEED
				NumSectorsBeingRead = iTotalUnreadSectors;
				if ( NumSectorsBeingRead > SectorReadCount ) NumSectorsBeingRead = SectorReadCount;
#endif

#ifdef DISKREAD_FIXED_SEEK
				//Status = CDVD_STATUS_SEEK;
				Status = CDVD_STATUS_READ;
				//Temp_SeekTime = (u64) dDiskReadCycleTime;
				Temp_SeekTime = 8;
				
				//Temp_ReadTime = ullDiskReadCycles;
				Temp_ReadTime = 8;
				
				Set_NextEvent ( Temp_SeekTime );
#else

/*
#ifdef DISKREAD_PER_SECTOR
				// if reading sector by sector, then just need to know if has read past current sector
				ullTargetCycle = ullReadStart_Cycle + ( (u64) ( dDiskReadCycleTime * ( ( SeekSectorNum + 1 ) - ullReadStart_Sector ) ) );
#else
				ullTargetCycle = ullReadStart_Cycle + ( (u64) ( dDiskReadCycleTime * ( ( SeekSectorNum + SectorReadCount ) - ullReadStart_Sector ) ) );
#endif
*/

				if ( sSeekDelta > 0 )
				{
					Status = CDVD_STATUS_SEEK;
					Temp_SeekTime = 8;	//uSeekDelta * 128;
					Temp_ReadTime = 0;
					Set_NextEvent ( Temp_SeekTime );
				}
				else
				{
					if ( SeekSectorNum >= ullReadStart_Sector )
					{
						Status = CDVD_STATUS_READ;
						Temp_ReadTime = 8;
						Temp_SeekTime = 0;
						Set_NextEvent ( Temp_ReadTime );
					}
					else
					{
						//Status = CDVD_STATUS_READ;
						Status = CDVD_STATUS_SEEK;
						Temp_ReadTime = 0;
						Temp_SeekTime = 8;
						Set_NextEvent ( Temp_SeekTime );
					}
				}

#endif
			}
			//else if ( ((u64)sSeekDelta) <= c_lFastCDSeekDelta )
			else if ( uSeekDelta <= c_lFastCDSeekDelta )
			{
				Status = CDVD_STATUS_SEEK;
				
#ifdef DISKREAD_FIXED_SEEK_FAST
				Temp_SeekTime = c_llCDFastSeekInCycles;
#else
				Temp_SeekTime = c_llCDSeek_CyclesPerSector * uSeekDelta;
#endif
				
				Temp_SeekTime = Temp_SeekTime > 33000000 ? 33000000 : Temp_SeekTime;
				Temp_ReadTime = ullDiskReadCycles;
				Set_NextEvent ( Temp_SeekTime );
				
				// set the sector and cycle the read is starting at
				ullReadStart_Sector = SeekSectorNum;
				ullReadStart_Cycle = *_DebugCycleCount;
				
				/*
				Status = CDVD_STATUS_SEEK;
				Temp_SeekTime = c_llCDSeekInCycles;
				Temp_ReadTime = ullDiskReadCycles;
				Set_NextEvent ( Temp_SeekTime );
				
				// set the sector and cycle the read is starting at
				ullReadStart_Sector = SeekSectorNum;
				ullReadStart_Cycle = *_DebugCycleCount;
				*/
			}
			else
			{
				// long running seek
				Status = CDVD_STATUS_SEEK;
				
#ifdef DISKREAD_FIXED_SEEK_SLOW
				Temp_SeekTime = c_llCDSeekInCycles;
#else
				Temp_SeekTime = c_llCDSeek_CyclesPerSector * uSeekDelta;
#endif

				Temp_SeekTime = Temp_SeekTime > 33000000 ? 33000000 : Temp_SeekTime;
				Temp_ReadTime = ullDiskReadCycles;
				Set_NextEvent ( Temp_SeekTime );
				
				// set the sector and cycle the read is starting at
				ullReadStart_Sector = SeekSectorNum;
				ullReadStart_Cycle = *_DebugCycleCount;
				
				// long running seek
				/*
				Set_NextEvent ( c_llCDSeekInCycles );
				Temp_SeekTime = c_llCDSeekInCycles;
				//Set_NextEvent ( c_llCDSeek_CyclesPerSector * SeekDelta );
				//Temp_SeekTime = c_llCDSeek_CyclesPerSector * SeekDelta;
				Temp_ReadTime = ullDiskReadCycles;
				
				// set the sector and cycle the read is starting at
				ullReadStart_Sector = SeekSectorNum;
				ullReadStart_Cycle = *_DebugCycleCount;
				*/
			}
#endif

			//}
			//else
			//{
			//	// long running seek
			//	Set_NextEvent ( c_llCDSeekInCycles );
			//	Temp_SeekTime = c_llCDSeekInCycles;
			//}
			
			
			// need some message for testing
			cout << "\nCDREAD: DecSet=" << hex << (u32) DecSet << " Reading " << dec << SectorReadCount << " sectors at speed x" << DiskSpeed << " from Sector#" << SeekSectorNum << " SeekDelta=" << sSeekDelta << " PrevSector=" << PrevSector << " SeekSector=" << SeekSectorNum << " SeekTime=" << Temp_SeekTime << " ReadTime=" << Temp_ReadTime << " DR=" << hex << (u32) DataRequest << " DS=" << DiskSpeedType << dec << " Cycle#" << *_DebugCycleCount;
			
#ifdef INLINE_DEBUG_DISKREAD
			debugRead << "\r\nCDREAD: DecSet=" << hex << (u32) DecSet << " Reading " << dec << SectorReadCount << " sectors at speed x" << DiskSpeed << " from Sector#" << SeekSectorNum;
			debugRead << " SectorSize=" << _CDVD->SectorReadSize;
			debugRead << " SeekDelta=" << sSeekDelta;
			debugRead << " PrevSector=" << PrevSector;
			debugRead << " SeekSector=" << SeekSectorNum;
			debugRead << " SeekTime=" << Temp_SeekTime;
			debugRead << " ReadTime=" << Temp_ReadTime;
			debugRead << " DR=" << hex << (u32) DataRequest;
			debugRead << " DS=" << hex << DiskSpeedType;
			debugRead  << " Cycle#" << dec << *_DebugCycleCount;
			debugRead  << " NextEvent_Cycle=" << NextEvent_Cycle;
			debugRead << " StartSector=" << dec << ullReadStart_Sector;
			debugRead << " TargetSector=" << dec << ( SeekSectorNum + SectorReadCount );
			debugRead << " TargetCycle#" << dec << ( ullReadStart_Cycle + (u64) ( dDiskReadCycleTime * ( ( SeekSectorNum + SectorReadCount ) - ullReadStart_Sector ) ) );
#endif
			
			//if ( _CDVD->SeekSectorNum < 64 )
			//{
			//	// ***todo*** schedule seek and read to occur
			//	_CDVD->Set_NextEvent ( c_lCDSeek_Cycles );
			//}
			//else
			//{
			//	_CDVD->Set_NextEvent ( 4000000 );
			//}
			
			// command done
			// reset input fifo for N Commands
			_CDVD->lNArgIndex = 0;
			
			// update where the sector will be after read
			//SeekSectorNum += SectorReadCount;
			
			// testing //
			//_CDVD->NReady = 0x40;
			
			break;
			
		
		// DVD READ
		case 0x8:
#ifdef INLINE_DEBUG_WRITE
	debug << "; DVD READ";
#endif

			// read interrupt now pending
			bPendingReadInt = true;

			// not ready
			NReady = 0x80;
			
			// not reading any sectors yet
			NumSectorsBeingRead = 0;
			
			// no sectors available yet
			AvailableSectorCount = 0;
		
			//PrevSector = SeekSectorNum;
			PrevSector = ReadSector;
			
			SeekSectorNum = ( (u32*) _CDVD->ucNArgBuffer ) [ 0 ];
			SectorReadCount = ( (u32*) _CDVD->ucNArgBuffer ) [ 1 ];

			
			// check for zero sector read
			if ( !SectorReadCount )
			{
				// can't read zero sectors ??
				SectorReadCount = 1;
			}
			
			// set the next sector to interrupt at ??
			ulNextSectorInt = SectorReadCount - c_ulIntSectors;
			
			
			// get the read speed
			DiskSpeedType = _CDVD->ucNArgBuffer [ 9 ];


#ifdef TEST_DISKSPEED
			DiskSpeed = 1;
#else
			// ***todo*** 0x83 might mean x4 for dvd ???
			//0x83 for cd should be x24??
			//0x83 for dvd should be x4??
			// 0x2 for dvd should be x2??
			DiskSpeed = (float)(( DiskSpeedType & 0xf ) - 1);
			/*
			switch ( DiskSpeedType & 0xf )
			{
				case 0x1:
					DiskSpeed = 1.0;
					break;
					
				
				case 0x2:
					// maybe x4 for dvd ??
					//... but this appears to actually be x1??
					// this is DEFINITELY x1 dvd speed
					//DiskSpeed = 2.0;
					DiskSpeed = 1.0;
					break;
					
				case 0x3:
					// means x4
					//DiskSpeed = 4;
					// I assume this would be x3 ??
					//DiskSpeed = 4.0;
					DiskSpeed = 2.0;
					break;
					
				case 0x4:
					// probably means x2 ??
					//DiskSpeed = 4.0;
					DiskSpeed = 4.0;	//3;
					break;
					
				case 0x5:
					// means x24
					//DiskSpeed = 24;
					// hmmm.. on dvd this would probably be x1 ??
					//DiskSpeed = 1;
					DiskSpeed = 4.0;
					break;
				
					
				default:
					// unknown
					cout << "\nhps2x64: CDVD: ***ALERT***: Unknown disk speed set=" << hex << DiskSpeed << "\n";
					DiskSpeed = 4.0;
					break;
			}
			*/


#endif
			
			
			// set the time to read one sector of data from disk
			dDiskReadCycleTime = c_dDVDReadx1_Cycles / DiskSpeed;

			ullDiskReadCycles = (u64) ( dDiskReadCycleTime * SectorReadCount );

			// *** temporary testing ***
			//dDiskReadCycleTime = 2.0L;

			
			// get difference from last read sector
			sSeekDelta = SeekSectorNum - PrevSector;
			
			// get absolute value
			uSeekDelta = ( sSeekDelta < 0 ) ? -sSeekDelta : sSeekDelta;
			
			
			// byte 8 is retry count ??
			// byte 9 spndlctrl ??

			
			// byte 8 is retry count ??
			// byte 9 spndlctrl ??
			
			// sector size is always 2064 for DVD read
			_CDVD->SectorReadSize = 2064;
			CD::_CD->ReadMode_Offset = 0;
			
			// start from beginning for dma
			DMAReadIndex = 0;
			
			// set the sector size for ps1 cd handler
			CD::_CD->DataBuffer_Size = _CDVD->SectorReadSize;
			CD::_CD->ReadMode = _CDVD->SectorReadSize;
			
			// reset the index on ps1 side
			CD::_CD->DataBuffer_Index = 0;
			
			if ( sSeekDelta )
			{
cout << "\n->SEEK->";
			// seek to sector
			CD::_CD->cd_image.SeekSector ( _CDVD->SeekSectorNum + DiskImage::CDImage::c_SectorsInFirstTwoSeconds );
			CD::_CD->cd_image.StartReading ();
			}
			
			// set the read command as cd read
			_CDVD->ReadCommand = 0x6;
			
			// status is now "seek" ??
			Status = CDVD_STATUS_SEEK;

			
			// *new* set the sector being read from
			ReadSector = SeekSectorNum;

			// init read time
			Temp_ReadTime = 0;
			Temp_SeekTime = 0;

			iTotalAvailableSectors = 0;
			iTotalReadSectors = 0;
			iTotalUnreadSectors = 0;
			
#ifdef TEST_DISKSEEK
			Status = CDVD_STATUS_SEEK;
			Temp_SeekTime = c_llDVDSeekInCycles;
			Temp_ReadTime = ullDiskReadCycles;
			Set_NextEvent ( Temp_SeekTime );
#else
			//if ( ((u64)sSeekDelta) <= c_lDVDNoSeekDelta )
			if ( uSeekDelta <= c_lDVDNoSeekDelta )
			{
				iTotalAvailableSectors = ( ( *_DebugCycleCount - ullLastSeekCycle ) / dDiskReadCycleTime );
				iTotalReadSectors = SeekSectorNum - lLastSeekSector;
				iTotalUnreadSectors = iTotalAvailableSectors - iTotalReadSectors;

#ifdef ADJUST_STREAM_SPEED
				NumSectorsBeingRead = iTotalUnreadSectors;
				if ( NumSectorsBeingRead > SectorReadCount ) NumSectorsBeingRead = SectorReadCount;
#endif

				// apparently cd/dvd has anywhere from an 8-16 sector buffer ??
#ifdef DISKREAD_FIXED_SEEK
				//Status = CDVD_STATUS_SEEK;
				Status = CDVD_STATUS_READ;
				//Temp_SeekTime = (u64) dDiskReadCycleTime;
				Temp_SeekTime = 8;
				//Temp_ReadTime = ullDiskReadCycles;
				Temp_ReadTime = 8;
				Set_NextEvent ( Temp_SeekTime );
#else

/*				
#ifdef DISKREAD_PER_SECTOR
				// if reading sector by sector, then just need to know if has read past current sector
				ullTargetCycle = ullReadStart_Cycle + ( (u64) ( dDiskReadCycleTime * ( ( SeekSectorNum + 1 ) - ullReadStart_Sector ) ) );
#else
				ullTargetCycle = ullReadStart_Cycle + ( (u64) ( dDiskReadCycleTime * ( ( SeekSectorNum + SectorReadCount ) - ullReadStart_Sector ) ) );
#endif
*/
					
				if ( sSeekDelta > 0 )
				{
					Status = CDVD_STATUS_SEEK;
					Temp_SeekTime = 8;	//uSeekDelta * 128;
					Temp_ReadTime = 0;
					Set_NextEvent ( Temp_SeekTime );
				}
				else
				{
					if ( SeekSectorNum >= ullReadStart_Sector )
					{
						Status = CDVD_STATUS_READ;
						Temp_ReadTime = 8;
						Temp_SeekTime = 0;
						Set_NextEvent ( Temp_ReadTime );
					}
					else
					{
						//Status = CDVD_STATUS_READ;
						Status = CDVD_STATUS_SEEK;
						Temp_ReadTime = 0;
						Temp_SeekTime = 8;
						Set_NextEvent ( Temp_SeekTime );
					}
				}

#endif

			}
			//else if ( ((u64)sSeekDelta) <= c_lFastDVDSeekDelta )
			else if ( uSeekDelta <= c_lFastDVDSeekDelta )
			{
				// fast seek
				/*
				Set_NextEvent ( c_llDVDFastSeekInCycles );
				Temp_SeekTime = c_llDVDFastSeekInCycles;
				Temp_ReadTime = ullDiskReadCycles;
				*/
				
				Status = CDVD_STATUS_SEEK;
				
#ifdef DISKREAD_FIXED_SEEK_FAST
				Temp_SeekTime = c_llDVDFastSeekInCycles;
#else
				Temp_SeekTime = c_llDVDSeek_CyclesPerSector * uSeekDelta;
#endif

				Temp_SeekTime = Temp_SeekTime > 33000000 ? 33000000 : Temp_SeekTime;
				Temp_ReadTime = ullDiskReadCycles;
				Set_NextEvent ( Temp_SeekTime );
				
				// set the sector and cycle the read is starting at
				ullReadStart_Sector = SeekSectorNum;
				ullReadStart_Cycle = *_DebugCycleCount;
				
				//Set_NextEvent ( c_llDVDSeek_CyclesPerSector * SeekDelta );
				//Temp_SeekTime = c_llDVDSeek_CyclesPerSector * SeekDelta;
			}
			else
			{
				// long running seek
				Status = CDVD_STATUS_SEEK;
				
#ifdef DISKREAD_FIXED_SEEK_SLOW
				Temp_SeekTime = c_llDVDSeekInCycles;
#else
				Temp_SeekTime = c_llDVDSeek_CyclesPerSector * uSeekDelta;
#endif

				Temp_SeekTime = Temp_SeekTime > 33000000 ? 33000000 : Temp_SeekTime;
				Temp_ReadTime = ullDiskReadCycles;
				Set_NextEvent ( Temp_SeekTime );
				
				// set the sector and cycle the read is starting at
				ullReadStart_Sector = SeekSectorNum;
				ullReadStart_Cycle = *_DebugCycleCount;
				
				//Set_NextEvent ( c_llDVDSeek_CyclesPerSector * SeekDelta );
				//Temp_SeekTime = c_llDVDSeek_CyclesPerSector * SeekDelta;
			}
#endif

			
			// need some message for testing
			cout << "\nDVDREAD: DecSet=" << hex << (u32) DecSet << " Reading " << dec << SectorReadCount << " sectors at speed x" << DiskSpeed << " from Sector#" << SeekSectorNum << " SeekDelta=" << sSeekDelta << " PrevSector=" << PrevSector << " SeekSector=" << SeekSectorNum << " SeekTime=" << Temp_SeekTime << " ReadTime=" << Temp_ReadTime << " DR=" << hex << (u32) DataRequest << " DS=" << DiskSpeedType << dec << " AS=" << iTotalAvailableSectors << " RS=" << iTotalReadSectors << " US=" << iTotalUnreadSectors << " Cycle#" << *_DebugCycleCount;
			
#ifdef INLINE_DEBUG_DISKREAD
			debugRead << "\r\nDVDREAD: DecSet=" << hex << (u32) DecSet << " Reading " << dec << SectorReadCount << " sectors at speed x" << DiskSpeed << " from Sector#" << SeekSectorNum << " SeekDelta=" << sSeekDelta << " PrevSector=" << PrevSector << " SeekSector=" << SeekSectorNum << " SeekTime=" << Temp_SeekTime << " ReadTime=" << Temp_ReadTime << " DR=" << hex << (u32) DataRequest << " DS=" << DiskSpeedType << dec << " Cycle#" << *_DebugCycleCount;
			debugRead << " AS=" << dec << iTotalUnreadSectors;
			debugRead << " StartSector=" << dec << ullReadStart_Sector;
			debugRead << " TargetSector=" << dec << ( SeekSectorNum + SectorReadCount );
			debugRead << " TargetCycle#" << dec << ( ullReadStart_Cycle + (u64) ( dDiskReadCycleTime * ( ( SeekSectorNum + SectorReadCount ) - ullReadStart_Sector ) ) );
#endif

			// command done
			// reset input fifo for N Commands
			_CDVD->lNArgIndex = 0;
			
			// update where the sector will be after read
			//SeekSectorNum += SectorReadCount;
			
			// testing //
			//_CDVD->NReady = 0x40;
			
			break;
			
			
		// READ TOC
		case 0x9:
#ifdef INLINE_DEBUG_WRITE
	debug << "; READ TOC";
#endif

			cout << "\n*** READ TOC ***\n";
			
			// set the read command as read toc
			_CDVD->ReadCommand = 0x9;
			
			// ***todo*** read toc might take longer
			_CDVD->Set_NextEvent ( c_lCDSeek_Cycles );
			
			// reset input fifo for N Commands
			_CDVD->lNArgIndex = 0;
			break;
			
		// READ KEY
		case 0xc:
#ifdef INLINE_DEBUG_WRITE
	debug << "; READ KEY";
#endif

			cout << "\n*** READ KEY ***\n";
			
			arg0 = _CDVD->ucNArgBuffer [0];
			arg1 = ( (u32) _CDVD->ucNArgBuffer [1] ) | (( (u32) _CDVD->ucNArgBuffer[2] )<<8);
			arg2 = ( (u32) _CDVD->ucNArgBuffer[3] ) | (( (u32) _CDVD->ucNArgBuffer[4] )<<8) | (( (u32) _CDVD->ucNArgBuffer[5] )<<16) | (( (u32) _CDVD->ucNArgBuffer[6] )<<24);
			
			DiskSerial = CD::_CD->cd_image.DiskSerial;
			
			// SLXX_YYY.ZZ
			numbers=0;
			letters=0;
			key_0_3;
			key_4, key_14;
			
			//char NumbersStr [ 16 ];
			
			// ***todo*** read the cd key
			
			// clear key
			memset ( _CDVD->DiskKey, 0, 16 );
			
			// put numbers into a string and convert to a number
			NumbersStr [ 0 ] = DiskSerial [ 5 ];
			NumbersStr [ 1 ] = DiskSerial [ 6 ];
			NumbersStr [ 2 ] = DiskSerial [ 7 ];
			NumbersStr [ 3 ] = DiskSerial [ 9 ];
			NumbersStr [ 4 ] = DiskSerial [ 10 ];
			NumbersStr [ 5 ] = 0;
			
			// convert to a number
			numbers = Utilities::Strings::CLng ( NumbersStr );
			
			// combine the letters
			letters = (((s32)DiskSerial[3]&0x7F)<< 0) | (((s32)DiskSerial[2]&0x7F)<< 7) | (((s32)DiskSerial[1]&0x7F)<<14) | (((s32)DiskSerial[0]&0x7F)<<21);
			
			// calculate magic numbers
			key_0_3 = ((numbers & 0x1FC00) >> 10) | ((0x01FFFFFF & letters) <<  7);	// numbers = 7F  letters = FFFFFF80
			key_4   = ((numbers & 0x0001F) <<  3) | ((0x0E000000 & letters) >> 25);	// numbers = F8  letters = 07
			key_14  = ((numbers & 0x003E0) >>  2) | 0x04;							// numbers = F8  extra   = 04  unused = 03

			// store key values
			_CDVD->DiskKey [ 0] = (key_0_3&0x000000FF)>> 0;
			_CDVD->DiskKey [ 1] = (key_0_3&0x0000FF00)>> 8;
			_CDVD->DiskKey [ 2] = (key_0_3&0x00FF0000)>>16;
			_CDVD->DiskKey [ 3] = (key_0_3&0xFF000000)>>24;
			_CDVD->DiskKey [ 4] = key_4;
			
			switch (arg2)
			{
				case 75:
					_CDVD->DiskKey [14] = key_14;
					_CDVD->DiskKey [15] = 0x05;
					break;

		//      case 3075:
		//          key[15] = 0x01;
		//          break;

				case 4246:
					// 0x0001F2F707 = sector 0x0001F2F7  dec 0x07
					_CDVD->DiskKey [ 0] = 0x07;
					_CDVD->DiskKey [ 1] = 0xF7;
					_CDVD->DiskKey [ 2] = 0xF2;
					_CDVD->DiskKey [ 3] = 0x01;
					_CDVD->DiskKey [ 4] = 0x00;
					_CDVD->DiskKey [15] = 0x01;
					break;

				default:
					_CDVD->DiskKey [15] = 0x01;
					break;
			}
			
			
			
			// send CD interrupt
			SetInterrupt_CDVD ();
			
			// set the reason for the interrupt
			// 0x1 means "data ready"
			// 0x2 means "command complete"
			// 0x4 means "acknowledge" (or maybe 0x3??)
			// 0x8 means "end of data" ?? (or maybe 0x4??)
			_CDVD->InterruptReason = 0x2;
			
			// set xor key
			CD::_CD->XorKey = _CDVD->DiskKey [ 4 ];
			
			// command done
			// reset input fifo for N Commands
			_CDVD->lNArgIndex = 0;
			
			break;
			
		default:
			cout << "\n*** UNKNOWN N-COMMAND = " << hex << (u32) Command << " ***\n";
			break;
	}
}


u64 CDVD::DMA_ReadyForRead ( void )
{
#ifdef INLINE_DEBUG_READY
	debug << "\r\nCDVD::DMA_ReadyForRead";
#endif

	if ( !CDVD::_CDVD->AvailableSectorCount )
	//if ( ( !CDVD::_CDVD->AvailableSectorCount ) && ( CDVD::_CDVD->Status != CDVD_STATUS_READ ) )
	{
		
#ifdef INLINE_DEBUG_READY
	debug << " DMA#3 Not Ready For Reading";
#endif

		return 0;
	}
	
	return 1;
}

u32 CDVD::DMA_ReadBlock ( u32* pMemoryPtr, u32 Address, u32 BS )
{
#ifdef INLINE_DEBUG_READBUFFER
	debug << "\r\nCDVD::DMA_ReadBlock";
#endif

	u32 *Data;
	u32 NumberOfTransfers;
	
	Data = & ( pMemoryPtr [ Address >> 2 ] );
	
	// if no data available, then don't read anything
	if ( !CDVD::_CDVD->AvailableSectorCount )
	{
		return 0;
	}

	if ( !_CDVD->DMAReadIndex )
	{
#ifdef INLINE_DEBUG_READBUFFER
	debug << " READING-SECTOR";
#endif

		// read from cd
		CD::_CD->cd_image.ReadNextSector ();
		
		CD::_CD->SectorDataBuffer_Index = CD::_CD->cd_image.GetCurrentBufferIndex ();
		
		// currently need to set this
		// no, no... dma transfer is actually not ready until 0x80 is written to request data
		//CD::_CD->isSectorDataReady = true;
		
		// this stuff needs to be set again ?? (fix later)
		// set the sector size for ps1 cd handler
		// need to set the data buffer size again since the ps1 handler resets it for now
		CD::_CD->DataBuffer_Size = CDVD::_CDVD->SectorReadSize;
		//CD::_CD->ReadMode = _CDVD->SectorReadSize;

		if ( CDVD::_CDVD->SectorReadSize == 1024 )
		{
			CD::_CD->DataBuffer_Size = 2064;
		}

		// set the sector number that is being read
		CD::_CD->DVDSectorNumber = CDVD::_CDVD->SeekSectorNum;
		
#ifdef INLINE_DEBUG_READBUFFER
	//debug << "\r\nDMA3 Transfer to " << hex << DmaCh [ 3 ].MADR << " where (before) BS=" << DmaCh [ 3 ].BCR.BS << " BA=" << DmaCh [ 3 ].BCR.BA;
#endif

		// should return the amount transfered in 32-bit words
		//NumberOfTransfers = _CD->DMA_ReadBlock ( & _BUS->MainMemory.b32 [ ( DmaCh [ 3 ].MADR & 0x1fffff ) >> 2 ], DmaCh [ 3 ].BCR.BS, DmaCh [ 3 ].BCR.BA );
		//NumberOfTransfers = CD::_CD->DMA_ReadBlock ( _CDVD->CDVDBuffer, 0, _CDVD->SectorReadSize );
		NumberOfTransfers = CD::_CD->DMA_ReadBlock ( _CDVD->CDVDBuffer, 0, CD::_CD->DataBuffer_Size >> 2 );


#ifdef INLINE_DEBUG_READBUFFER
	debug << " Transferred=" << dec << NumberOfTransfers << " 32-bit words";
	//debug << "\r\n";
#endif

		// check if nothing was transferred
		if ( !NumberOfTransfers )
		{
#ifdef INLINE_DEBUG_READBUFFER
	debug << " NO-TRANSFER";
#endif

			// done ??
			return 0;
		}
		
	}
	
#ifdef INLINE_DEBUG_READBUFFER
	debug << " TRANSFER-DATA";
#endif

	// transfer the data
	for ( int i = 0; i < BS; i++ ) *Data++ = _CDVD->CDVDBuffer [ _CDVD->DMAReadIndex++ ];

#ifdef INLINE_DEBUG_READBUFFER
	debug << " (after)DMAReadIndex=" << dec << _CDVD->DMAReadIndex;
#endif
	
	if ( _CDVD->DMAReadIndex >= ( _CDVD->SectorReadSize >> 2 ) )
	{
#ifdef INLINE_DEBUG_READBUFFER
	debug << " SECTOR-END";
#endif

		// must increase the sector number here so correct data gets put in when reading
		// advance sector number
		CDVD::_CDVD->SeekSectorNum++;
		
		// decrease the number of available sectors
		CDVD::_CDVD->AvailableSectorCount--;
		
		// reset the read index, since we need to load the next sector and read from the beginning of it
		_CDVD->DMAReadIndex = 0;
		
#ifdef INT_AFTER_DATA_TRANSFER
		if ( !_CDVD->SectorReadCount && !_CDVD->AvailableSectorCount )
		{
			// only issue interrupt if it is still pending
			// ( _CDVD->bPendingReadInt )
			{
#ifdef INLINE_DEBUG_READBUFFER
	debug << " CDVD-INT";
#endif

			// data ready
			_CDVD->InterruptReason = 1;
			SetInterrupt_CDVD ();
			//SetInterrupt ();
			}
		}
#endif
	}
	
	return BS;

	// update BusyCycles
	// note: this is on a 16-bit bus, so the 32-bit transfers are actually split into 16-bit transfers, meaning twice as many transfers
	// actually, according to martin korth's psx spec, this is an 8-bit bus
	//_BUS->ReserveBus ( NumberOfTransfers << 2 );
	//BusyCycles = ( NumberOfTransfers << 2 ) + 2;

	/*
	u32 BAUpdate;
	if ( DmaCh [ 3 ].BCR.BS )
	{
		if ( _CD->ReadMode == 1024 )
		{
			// read toc command //
			BAUpdate = DmaCh [ 3 ].BCR.BA;
			
			// can set the read mode to 2064 at this point
			_CD->ReadMode = 2064;
		}
		else
		{
			// command to read disk sector //
			BAUpdate = ( _CD->ReadMode / DmaCh [ 3 ].BCR.BS ) >> 2;
		}
	}
	else
	{
		// for now.. and this should work for PS1 also...
		BAUpdate = ( _CD->ReadMode / 0x200 ) >> 2;
	}
	*/
	
	// update BA
	// ***todo*** this is just a placeholder for now
	/*
	if ( DmaCh [ 3 ].BCR.BA >= BAUpdate )
	{
		DmaCh [ 3 ].BCR.BA -= BAUpdate;
		
		// also update MADR ??
		DmaCh [ 3 ].MADR += _CD->ReadMode;
	}

#ifdef INLINE_DEBUG_RUN_DMA3
	debug << "(after) BA=" << DmaCh [ 3 ].BCR.BA;
	debug << "\r\n";
#endif


	// check if transfer is done yet
	if ( DmaCh [ 3 ].BCR.BA < BAUpdate )
	{

		// need to call this when dma transfer is done
		_CD->DMA_End ();
		
		/////////////////////////////////////
		// we are done
		DMA_Finished ( 3 );
	
#ifdef INLINE_DEBUG_RUN_DMA3
debug << "\r\n";
debug << "DMA#3 Transfer is complete";
debug << "\r\n";
#endif

	}
	*/
	
}



