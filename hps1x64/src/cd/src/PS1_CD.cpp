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



#include "PS1_CD.h"
#include "PS1_SPU.h"

#ifdef PS2_COMPILE
#include "CDvd.h"
#endif

using namespace Playstation1;
using namespace DiskImage;

// enable debugging


#ifdef _DEBUG_VERSION_

#define INLINE_DEBUG_ENABLE

//#define INLINE_DEBUG_SPLIT

//#define INLINE_DEBUG_DMA_READ
//#define INLINE_DEBUG_DMA_READ_DATA

/*
#define INLINE_DEBUG_TESTING

#define INLINE_DEBUG_RUN


//#define INLINE_DEBUG_XA
//#define INLINE_DEBUG_PLAY

#define INLINE_DEBUG_INTQ_READ
#define INLINE_DEBUG_INTQ

//#define INLINE_DEBUG_EVENT
//#define INLINE_DEBUG
#define INLINE_DEBUG_READ
#define INLINE_DEBUG_WRITE
//#define INLINE_DEBUG_INTQ
*/

#endif

#define INLINE_DEBUG_LINES 20


funcVoid CD::UpdateInterrupts;

u32* CD::_DebugPC;
u64* CD::_DebugCycleCount;
u64* CD::_SystemCycleCount;
u32* CD::_NextEventIdx;

u64* CD::_NextSystemEvent;

//u32* CD::_Intc_Master;
u32* CD::_Intc_Stat;
u32* CD::_Intc_Mask;
u32* CD::_R3000A_Status_12;
u32* CD::_R3000A_Cause_13;
u64* CD::_ProcStatus;

CD *CD::_CD;

Debug::Log CD::debug;



bool CD::DebugWindow_Enabled;
WindowClass::Window *CD::DebugWindow;
DebugValueList<u32> *CD::ValueList;




// this is taken/adapted from "Dr Hell's" web page
const u32 CommandExecutionTimes [] = { 0x900 /*1*/, 30547, 31557, 51444, 193250, 125119, 75701, 74753,
											43503, 27648, 80295, 36114, 32768, 39105, 41082, 40163,
											43335, 37937, /*98762*/ 0x800, 40833, 39091, 41771, 62041, 1,
											1, 40864, 30980, 81925, 25847, 1, 73292, 1 };

											
const u32 CommandExecutionTimes_Secondary [] = { 0, 0, 0, 0, 0, 0, 450666, 51429486,
														12767194, /*700000*/ /*3665794*/ 1500000, /*80295*/ 3656479, 0, 0, 0, 0, 0,
														0, 0, /*57701665*/ 0x800, 0, 0, 1500000 /*41771*/, 1500000 /*62041*/, 0,
														0, 0, 1500000 /*19658*/, 450666, 0, 0, 31330364, 0 };

const char* CommandList [] = { "Sync", "Nop", "SetLoc", "Play", "Forward", "Backward", "ReadN", "Standby",
									"Stop", "Pause", "Init", "Mute", "DeMute", "SetFilter", "SetMode", "GetParam",
									"GetLocL", "GetLocP", "ReadT", "GetTN", "GetTD", "SeekL", "SeekP", "SetClock",
									"GetClock", "Test", "ID", "ReadS", "Reset", "Unknown0", "ReadTOC", "Unknown1" };



									

									
CD::CD ()
{
	cout << "Running CD constructor...\n";
}


void CD::Start ()
{
#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create( "CD_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering CD::Start";
#endif

	cout << "Running CD::Start...\n";
	
	Reset ();
	
	_CD = this;
	
	// by default, I'll make the region... I don't know
	Region = 'I';
	
	//SectorDataIndex = 0;
	
	// testing
	DebugCount = 80;
	
	
	// need to set all the events to far in the future (have not happened yet)
	// need to do this for all peripherals on the bus
	SetNextStartCycle_Cycle ( -1ULL );
	SetNextReadCycle_Cycle ( -1ULL );
	SetNextActionCycle_Cycle ( -1ULL );
	
	cout << "\nCD NextEvent=" << dec << NextEvent_Cycle;

#ifdef INLINE_DEBUG
	debug << "->Exiting CD::Start";
#endif
}


void CD::Reset ()
{
#ifdef INLINE_DEBUG
	debug << "\r\nEntering CD::Reset";
#endif

	// zero object
	memset ( this, 0, sizeof( CD ) );
	
	// no commands yet
	Command = -1;
	
	// no read commands yet either
	ReadCommand = -1;
	
	// no pending commands
	REG_Command = -1;
	
	// start with lid open
	isLidOpen = true;
	
#ifdef INLINE_DEBUG
	debug << "->Exiting CD::Reset";
#endif
}





/*
void CD::StartCommand ()
{
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nCommand Started. REG_Command=" << hex << (u32)REG_Command;
#endif
	
		// clear the next start command cycle for now just in case
		SetNextStartCycle ( 0 );
		
		// command has started
		Command = REG_Command;
		
		// move arguments into parameter buffer
		Load_ParameterBuf ( Arguments );
		
		// command is no longer in transit
		REG_Command = -1;
		
		// also reset parameters index here
		// don't do this!!!
		//ArgumentsIndex = 0;
		
		// update mode/status register
		UpdateREG_ModeStatus ();
		
		// check if command start interrupt is requested
		if ( isCommandStartInterruptRequested )
		{
			// set command start interrupt
			REG_InterruptFlag |= 0x10;
			
			// only requested for one command
			isCommandStartInterruptRequested = false;
		}
		
		// update interrupt flag register - should trigger interrupt if needed
		UpdateREG_InterruptFlag ();
		
#ifdef INLINE_DEBUG_RUN
	debug << "; CurrentCycle=" << dec << *_DebugCycleCount << "; NextEventCycle=" << NextEvent_Cycle << "; NextAction_Cycle=" << NextAction_Cycle;
#endif
#ifdef INLINE_DEBUG_RUN
	DebugCount = 20;
#endif
}
*/

// returns true if report created, false otherwise
bool CD::Get_ReportData ( u8* TempBuffer )
{
	u8* pSubBuffer;
	
	if ( MODE.Report )
	{
		// report mode is on //
		
		u8 Temp_AFrac;
		
		// get afrac
		if ( cd_image.isSubOpen )
		{
			// get the pointer into the sub buffer here
			pSubBuffer = cd_image.GetCurrentSubBuffer ();
			
			Temp_AFrac = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 2 ];
		}
		else
		{
			//Temp_AFrac = ConvertDecToBCD ( GetLocP_AFrac );
			Temp_AFrac = ConvertDecToBCD ( cd_image.SubQ_AFrac );
		}
		
		if ( Temp_AFrac == 0x00 || Temp_AFrac == 0x10 || Temp_AFrac == 0x20 || Temp_AFrac == 0x30 || Temp_AFrac == 0x40
			|| Temp_AFrac == 0x50 || Temp_AFrac == 0x60 || Temp_AFrac == 0x70 )
		{

			// first result in buffer is the status
			TempBuffer [ 0 ] = Status;
			
			if ( cd_image.isSubOpen )
			{
				// .sub file is available //
				
				TempBuffer [ 1 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackNumber;
				TempBuffer [ 2 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->IndexNumber;
			
				// on afrac = 0x00, 0x20, 0x40, 0x60 return absolute time
				if ( Temp_AFrac == 0x00 || Temp_AFrac == 0x20 || Temp_AFrac == 0x40 || Temp_AFrac == 0x60 )
				{
					// return absolute time
					TempBuffer [ 3 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 0 ];
					TempBuffer [ 4 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 1 ];
					TempBuffer [ 5 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 2 ];
				}
				else if ( Temp_AFrac == 0x10 || Temp_AFrac == 0x30 || Temp_AFrac == 0x50 || Temp_AFrac == 0x70 )
				{
					// return relative time with sec+0x80
					TempBuffer [ 3 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 0 ];
					TempBuffer [ 4 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 1 ] + 0x80;
					TempBuffer [ 5 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 2 ];
				}
			}
			else
			{
				// .sub file is NOT available
				
				//TempBuffer [ 1 ] = ConvertDecToBCD ( GetLocP_Track );
				TempBuffer [ 1 ] = ConvertDecToBCD ( cd_image.SubQ_Track );
				//TempBuffer [ 2 ] = ConvertDecToBCD ( GetLocP_Index );
				TempBuffer [ 2 ] = ConvertDecToBCD ( cd_image.SubQ_Index );
			
				// on afrac = 0x00, 0x20, 0x40, 0x60 return absolute time
				if ( Temp_AFrac == 0x00 || Temp_AFrac == 0x20 || Temp_AFrac == 0x40 || Temp_AFrac == 0x60 )
				{
					// return absolute time
					//TempBuffer [ 3 ] = ConvertDecToBCD ( GetLocP_AMin );
					TempBuffer [ 3 ] = ConvertDecToBCD ( cd_image.SubQ_AMin );
					//TempBuffer [ 4 ] = ConvertDecToBCD ( GetLocP_ASec );
					TempBuffer [ 4 ] = ConvertDecToBCD ( cd_image.SubQ_ASec );
					//TempBuffer [ 5 ] = ConvertDecToBCD ( GetLocP_AFrac );
					TempBuffer [ 5 ] = ConvertDecToBCD ( cd_image.SubQ_AFrac );
				}
				else if ( Temp_AFrac == 0x10 || Temp_AFrac == 0x30 || Temp_AFrac == 0x50 || Temp_AFrac == 0x70 )
				{
					// return relative time with sec+0x80
					//TempBuffer [ 3 ] = ConvertDecToBCD ( GetLocP_Min );
					TempBuffer [ 3 ] = ConvertDecToBCD ( cd_image.SubQ_Min );
					//TempBuffer [ 4 ] = ConvertDecToBCD ( GetLocP_Sec ) + 0x80;
					TempBuffer [ 4 ] = ConvertDecToBCD ( cd_image.SubQ_Sec ) + 0x80;
					//TempBuffer [ 5 ] = ConvertDecToBCD ( GetLocP_Frac );
					TempBuffer [ 5 ] = ConvertDecToBCD ( cd_image.SubQ_Frac );
				}
			}
			
			TempBuffer [ 6 ] = 0;
			TempBuffer [ 7 ] = 0;
			
#ifdef INLINE_DEBUG_RUN
	debug << "; REPORT";
#endif


			//EnqueueInterrupt_Read ( TempBuffer, 8, 0x1 );
			return true;
		}
	}
	
	return false;
}



void CD::Run ()
{
	u8 TempBuffer [ 16 ];
	u8* pSectorDataBuffer;
	u8* pSubBuffer;
	u32 Disk_File, Disk_Chan, Disk_SubMode;
	
	u8 TMin, TSec, TFrac;
	
	if ( NextStart_Cycle == *_DebugCycleCount )
	{
	
		// clear the next start command cycle for now just in case
		// by setting to an absolute zero, sets a clear indication there is no event
		// but to simplify, will set to -1ULL
		//SetNextStartCycle ( 0 );
		SetNextStartCycle_Cycle ( -1ULL );
		
		// make sure command is not to check for interrupt
		if ( Command == CDREG1_CMD_CHECK_INT )
		{
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nChecking INT again. Command=" << hex << (u32)Command;
#endif

			// check for pending interrupt, and trigger if needed
			Check_Interrupt ();
			
			// clear command
			Command = -1;
		}
		else
		{
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nCommand Started. REG_Command=" << hex << (u32)REG_Command;
#endif
		
			// command has started
			Command = REG_Command;
			
			// move arguments into parameter buffer
			Load_ParameterBuf ( Arguments );
			ParameterBuf_Size = ArgumentsIndex;
			
			// command is no longer in transit
			//REG_Command = -1;
			REG_Command = -1; ArgumentsIndex = 0;
			
			// also reset parameters index here
			// don't do this!!!
			//ArgumentsIndex = 0;
			
			// update mode/status register
			UpdateREG_ModeStatus ();
			
			// check if command start interrupt is requested
			if ( isCommandStartInterruptRequested )
			{
				// set command start interrupt
				REG_InterruptFlag |= 0x10;
				
				// only requested for one command
				isCommandStartInterruptRequested = false;
			}
			
			// update interrupt flag register - should trigger interrupt if needed
			UpdateREG_InterruptFlag ();
		}
		
#ifdef INLINE_DEBUG_RUN
	debug << "; CurrentCycle=" << dec << *_DebugCycleCount << "; NextEventCycle=" << NextEvent_Cycle << "; NextAction_Cycle=" << NextAction_Cycle;
#endif
//#ifdef INLINE_DEBUG_RUN
//	DebugCount = 20;
//#endif
	}
	

	
	// check if we need to execute the read command first
	if ( NextRead_Cycle == *_DebugCycleCount )
	{
	
#if defined INLINE_DEBUG_RUN || defined INLINE_DEBUG_PLAY
	debug << "\r\nRun::READCMD " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

		// clear the next read cycle for now just in case
		// by setting to an absolute zero, sets a clear indication there is no event
		// but to simplify, will set to -1ULL
		//SetNextReadCycle ( 0 );
		SetNextReadCycle_Cycle ( -1ULL );

		// debugging: always show what happens after command
		DebugCount = INLINE_DEBUG_LINES;	//80;
		
		// execute the read command
		switch ( ReadCommand )
		{
			
			case CDREG1_CMD_PLAY_2:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; PLAY2";
#endif

#ifdef INLINE_DEBUG_PLAY
	debug << "\r\nRun::READCMD " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount;
	debug << "; PLAY2";
#endif

				
#ifdef INLINE_DEBUG_RUN
	debug << " CurrentSector_Number=" << dec << CurrentSector_Number << " NextTrackSector_Number=" << NextTrackSector_Number;
#endif

				// check if need to auto pause or if it is the end of the disk
				//if ( ( CurrentSector_Number >= NextTrackSector_Number && MODE.AutoPause ) || ( CurrentSector_Number >= cd_image.LastSector_Number ) )
				if ( ( cd_image.CurrentSector >= NextTrackSector_Number /*cd_image.NextTrack_Sector*/ && MODE.AutoPause ) || ( cd_image.CurrentSector >= cd_image.LastSector_Number ) )
				{
#ifdef INLINE_DEBUG_RUN
	debug << "; AUTOPAUSE_CDDA";
#endif

					// auto pause cdda or end of disk //
					//Command = CDREG1_CMD_PAUSE;
					ReadCommand = -1;
					//SetNextActionCycle ( 0 );
					
					Status &= ~( CDREG1_READ_STAT_READ | CDREG1_READ_STAT_PLAY );
					TempBuffer [ 0 ] = Status;
					
					EnqueueInterrupt_Read ( TempBuffer, 1, 0x4 );
					
					break;
				}
				

				// read the next sector to play it
				pSectorDataBuffer = cd_image.ReadNextSector ();
				
				/*
				if ( !isReadingFirstSector )
				{
					// update the sector number for the next sector
					CurrentSector_Number++;
					
					// update relative values too
					Inc_DiskPosition ( Min, Sec, Frac );
					
					// update to next sector for next read
					Inc_DiskPosition ( AMin, ASec, AFrac );
				}
				
				// not reading the first sector any more
				isReadingFirstSector = false;
				
				GetLocP_Index = 1;
				GetLocP_Track = cd_image.FindTrack ( AMin, ASec, AFrac );	//CurrentTrack;
				GetLocP_Min = Min;
				GetLocP_Sec = Sec;
				GetLocP_Frac = Frac;
				GetLocP_AMin = AMin;
				GetLocP_ASec = ASec;
				GetLocP_AFrac = AFrac;
				*/
				
				// copy into sector data buffer
				//for ( int i = 0; i < c_SectorSize; i++ ) SectorDataBuffer [ i ] = pSectorDataBuffer [ i ];
					
				// copy into sub buffer
				/*
				if ( cd_image.isSubOpen )
				{
					// copy subchannel data
					for ( int i = 0; i < DiskImage::CDImage::c_SubChannelSizePerSector; i++ ) SubBuffer [ i ] = cd_image.CurrentSubBuffer [ i ];
				}
				*/
				
				
				if ( Get_ReportData ( TempBuffer ) )
				{
					EnqueueInterrupt_Read ( TempBuffer, 8, 0x1 );
				}


				/*
				if ( MODE.Report )
				{
					// report mode is on //
					
					u8 Temp_AFrac;
					
					// get afrac
					if ( cd_image.isSubOpen )
					{
						// get the pointer into the sub buffer here
						pSubBuffer = cd_image.GetCurrentSubBuffer ();
						
						Temp_AFrac = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 2 ];
					}
					else
					{
						//Temp_AFrac = ConvertDecToBCD ( GetLocP_AFrac );
						Temp_AFrac = ConvertDecToBCD ( cd_image.SubQ_AFrac );
					}
					
					if ( Temp_AFrac == 0x00 || Temp_AFrac == 0x10 || Temp_AFrac == 0x20 || Temp_AFrac == 0x30 || Temp_AFrac == 0x40
						|| Temp_AFrac == 0x50 || Temp_AFrac == 0x60 || Temp_AFrac == 0x70 )
					{

						// first result in buffer is the status
						TempBuffer [ 0 ] = Status;
						
						if ( cd_image.isSubOpen )
						{
							// .sub file is available //
							
							TempBuffer [ 1 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackNumber;
							TempBuffer [ 2 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->IndexNumber;
						
							// on afrac = 0x00, 0x20, 0x40, 0x60 return absolute time
							if ( Temp_AFrac == 0x00 || Temp_AFrac == 0x20 || Temp_AFrac == 0x40 || Temp_AFrac == 0x60 )
							{
								// return absolute time
								TempBuffer [ 3 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 0 ];
								TempBuffer [ 4 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 1 ];
								TempBuffer [ 5 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 2 ];
							}
							else if ( Temp_AFrac == 0x10 || Temp_AFrac == 0x30 || Temp_AFrac == 0x50 || Temp_AFrac == 0x70 )
							{
								// return relative time with sec+0x80
								TempBuffer [ 3 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 0 ];
								TempBuffer [ 4 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 1 ] + 0x80;
								TempBuffer [ 5 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 2 ];
							}
						}
						else
						{
							// .sub file is NOT available
							
							//TempBuffer [ 1 ] = ConvertDecToBCD ( GetLocP_Track );
							TempBuffer [ 1 ] = ConvertDecToBCD ( cd_image.SubQ_Track );
							//TempBuffer [ 2 ] = ConvertDecToBCD ( GetLocP_Index );
							TempBuffer [ 2 ] = ConvertDecToBCD ( cd_image.SubQ_Index );
						
							// on afrac = 0x00, 0x20, 0x40, 0x60 return absolute time
							if ( Temp_AFrac == 0x00 || Temp_AFrac == 0x20 || Temp_AFrac == 0x40 || Temp_AFrac == 0x60 )
							{
								// return absolute time
								//TempBuffer [ 3 ] = ConvertDecToBCD ( GetLocP_AMin );
								TempBuffer [ 3 ] = ConvertDecToBCD ( cd_image.SubQ_AMin );
								//TempBuffer [ 4 ] = ConvertDecToBCD ( GetLocP_ASec );
								TempBuffer [ 4 ] = ConvertDecToBCD ( cd_image.SubQ_ASec );
								//TempBuffer [ 5 ] = ConvertDecToBCD ( GetLocP_AFrac );
								TempBuffer [ 5 ] = ConvertDecToBCD ( cd_image.SubQ_AFrac );
							}
							else if ( Temp_AFrac == 0x10 || Temp_AFrac == 0x30 || Temp_AFrac == 0x50 || Temp_AFrac == 0x70 )
							{
								// return relative time with sec+0x80
								//TempBuffer [ 3 ] = ConvertDecToBCD ( GetLocP_Min );
								TempBuffer [ 3 ] = ConvertDecToBCD ( cd_image.SubQ_Min );
								//TempBuffer [ 4 ] = ConvertDecToBCD ( GetLocP_Sec ) + 0x80;
								TempBuffer [ 4 ] = ConvertDecToBCD ( cd_image.SubQ_Sec ) + 0x80;
								//TempBuffer [ 5 ] = ConvertDecToBCD ( GetLocP_Frac );
								TempBuffer [ 5 ] = ConvertDecToBCD ( cd_image.SubQ_Frac );
							}
						}
						
						TempBuffer [ 6 ] = 0;
						TempBuffer [ 7 ] = 0;
						EnqueueInterrupt_Read ( TempBuffer, 8, 0x1 );
						
#ifdef INLINE_DEBUG_RUN
	debug << "; REPORT";
#endif
					}
				}
				*/
				
				
				
				// make sure it is enabled to play from CD
				// note: MODE.CDDA is ignored by PLAY command
				//if ( MODE.CDDA )
				//{
					// make sure it is not a data sector
					if ( !isDataSector ( pSectorDataBuffer ) )
					{
						// sector is audio data //
						
						// if reading at single speed, put all samples into spu read buffer, otherwise put every other l/r sample in spu read buffer
						// put entire sector into buffer to be read by spu if single speed reading
						if ( !MODE.Speed )
						{
							for ( int i = 0; i < ( c_SectorSize / 4 ); i++ ) SpuBuffer [ ( SpuBuffer_WriteIndex + i ) & c_iSpuBuffer_Mask ] = Apply_CDDA_Volume( ((u32*)pSectorDataBuffer) [ i ] );
							
							// update write index for spu buffer
							// 2 bytes hold one 16-bit sample
							SpuBuffer_WriteIndex += ( c_SectorSize / 4 );
						}
						else
						{
							// cd is spinning at double speed, so put only every other l/r sample into buffer for spu //
							
							for ( int i = 0; i < ( c_SectorSize / 8 ); i++ ) ((u32*)SpuBuffer) [ ( SpuBuffer_WriteIndex + i ) & ( c_iSpuBuffer_Mask >> 1 ) ] = Apply_CDDA_Volume( ((u32*)pSectorDataBuffer) [ i << 1 ] );
							
							// update write index for spu buffer
							// 2 bytes hold one 16-bit sample - cd is double speed so it is only every other l/r sample
							SpuBuffer_WriteIndex += ( c_SectorSize / 4 );
						}
						
						// if read index is more than buffer size behind write index, then update it
						if ( ( SpuBuffer_ReadIndex + c_iSpuBuffer_Size ) < SpuBuffer_WriteIndex )
						{
							SpuBuffer_ReadIndex = SpuBuffer_WriteIndex - c_iSpuBuffer_Size;
						}
					}
				//}
				
#ifdef INLINE_DEBUG_RUN
	//debug << "; (Local) AMin=" << dec << (u32)GetLocP_AMin << " ASec=" << (u32)GetLocP_ASec << " AFrac=" << (u32)GetLocP_AFrac << " Min=" << (u32)GetLocP_Min << " Sec=" << (u32)GetLocP_Sec << " Frac=" << (u32)GetLocP_Frac;
	debug << "; (Local) AMin=" << dec << (u32)cd_image.Current_AMin << " ASec=" << (u32)cd_image.Current_ASec << " AFrac=" << (u32)cd_image.Current_AFrac << " Min=" << (u32)cd_image.SubQ_Min << " Sec=" << (u32)cd_image.SubQ_Sec << " Frac=" << (u32)cd_image.SubQ_Frac;
	if ( cd_image.isSubOpen )
	{
	pSubBuffer = cd_image.GetCurrentSubBuffer ();
	debug << "; (SubQ) AMin=" << hex << (u32)((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 0 ] << " ASec=" << (u32)((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 1 ] << " AFrac=" << (u32)((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 2 ];
	debug << " Min=" << (u32)((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 0 ] << " Sec=" << (u32)((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 1 ] << " Frac=" << (u32)((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 2 ];
	}
#endif

#ifdef INLINE_DEBUG_PLAY
	//debug << "; AMin=" << dec << (u32)AMin << " ASec=" << (u32)ASec << " AFrac=" << (u32)AFrac;
	debug << "; AMin=" << dec << (u32)cd_image.Current_AMin << " ASec=" << (u32)cd_image.Current_ASec << " AFrac=" << (u32)cd_image.Current_AFrac;
#endif

				// set the next command and busy cycles
				// we are going to read the next sector automatically and keep going
				ReadCommand = CDREG1_CMD_PLAY_2;
				BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READN ];
					
				// if we are reading at double speed, then we need to divide the busy cycles by 2
				if ( MODE.Speed ) BusyCycles_Read >>= 1;
				
				// get cycle for next read event
				SetNextReadCycle ( BusyCycles_Read );
				
				break;
				

			case CDREG1_CMD_FORWARD_2:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; FORWARD2";
#endif

				// read the next sector to play it
				pSectorDataBuffer = cd_image.ReadNextSector ();
				
				if ( Get_ReportData ( TempBuffer ) )
				{
					EnqueueInterrupt_Read ( TempBuffer, 8, 0x1 );
				}
				
				
				
				// copy into sector data buffer
				//for ( int i = 0; i < c_SectorSize; i++ ) SectorDataBuffer [ i ] = pSectorDataBuffer [ i ];
				/*
				if ( MODE.Report )
				{
					// report mode is on //
					
					u8 Temp_AFrac;
					
					// get afrac
					if ( cd_image.isSubOpen )
					{
						// get the pointer into the sub buffer here
						pSubBuffer = cd_image.GetCurrentSubBuffer ();
						
						Temp_AFrac = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 2 ];
					}
					else
					{
						Temp_AFrac = GetLocP_AFrac;
					}

					if ( Temp_AFrac == 0x00 || Temp_AFrac == 0x10 || Temp_AFrac == 0x20 || Temp_AFrac == 0x30 || Temp_AFrac == 0x40
						|| Temp_AFrac == 0x50 || Temp_AFrac == 0x60 || Temp_AFrac == 0x70 )
					{
						// first result in buffer is the status
						TempBuffer [ 0 ] = Status;
						
						if ( cd_image.isSubOpen )
						{
							// .sub file is available //
							
							TempBuffer [ 1 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackNumber;
							TempBuffer [ 2 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->IndexNumber;
						
							// on afrac = 0x00, 0x20, 0x40, 0x60 return absolute time
							if ( Temp_AFrac == 0x00 || Temp_AFrac == 0x20 || Temp_AFrac == 0x40 || Temp_AFrac == 0x60 )
							{
								// return absolute time
								TempBuffer [ 3 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 0 ];
								TempBuffer [ 4 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 1 ];
								TempBuffer [ 5 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 2 ];
							}
							else if ( Temp_AFrac == 0x10 || Temp_AFrac == 0x30 || Temp_AFrac == 0x50 || Temp_AFrac == 0x70 )
							{
								// return relative time with sec+0x80
								TempBuffer [ 3 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 0 ];
								TempBuffer [ 4 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 1 ] + 0x80;
								TempBuffer [ 5 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 2 ];
							}
						}
						else
						{
							// .sub file is NOT available
							
							TempBuffer [ 1 ] = GetLocP_Track;
							TempBuffer [ 2 ] = GetLocP_Index;
						
							// on afrac = 0x00, 0x20, 0x40, 0x60 return absolute time
							if ( Temp_AFrac == 0x00 || Temp_AFrac == 0x20 || Temp_AFrac == 0x40 || Temp_AFrac == 0x60 )
							{
								// return absolute time
								TempBuffer [ 3 ] = GetLocP_AMin;
								TempBuffer [ 4 ] = GetLocP_ASec;
								TempBuffer [ 5 ] = GetLocP_AFrac;
							}
							else if ( Temp_AFrac == 0x10 || Temp_AFrac == 0x30 || Temp_AFrac == 0x50 || Temp_AFrac == 0x70 )
							{
								// return relative time with sec+0x80
								TempBuffer [ 3 ] = GetLocP_Min;
								TempBuffer [ 4 ] = GetLocP_Sec + 0x80;
								TempBuffer [ 5 ] = GetLocP_Frac;
							}
						}
						
						//TempBuffer [ 6 ] = 0;
						//TempBuffer [ 7 ] = 0;
						TempBuffer [ 6 ] = pSectorDataBuffer [ 0 ];
						TempBuffer [ 7 ] = pSectorDataBuffer [ 1 ];
						EnqueueInterrupt_Read ( TempBuffer, 8, 0x1 );
					}
				}
				
				GetLocP_Index = 1;
				GetLocP_Track = Track;
				GetLocP_Min = Min;
				GetLocP_Sec = Sec;
				GetLocP_Frac = Frac;
				GetLocP_AMin = AMin;
				GetLocP_ASec = ASec;
				GetLocP_AFrac = AFrac;
				
				// update relative values too
				Inc_DiskPosition ( Min, Sec, Frac );
				
				// update to next sector for next read
				Inc_DiskPosition ( AMin, ASec, AFrac );
				*/
				
				// set the next command and busy cycles
				// we are going to read the next sector automatically and keep going
				ReadCommand = CDREG1_CMD_FORWARD_2;
				BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READN ];
				
				// if we are reading at double speed, then we need to divide the busy cycles by 2
				if ( MODE.Speed ) BusyCycles_Read >>= 1;
				
				// get cycle for next read event
				SetNextReadCycle ( BusyCycles_Read );
				
				break;
				

			case CDREG1_CMD_BACKWARD_2:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; BACKWARD2";
#endif

				if ( Get_ReportData ( TempBuffer ) )
				{
					EnqueueInterrupt_Read ( TempBuffer, 8, 0x1 );
				}
				
				// read the next sector to play it
				// needs to read the previous sector
				//SectorDataBuffer = cd_image.ReadNextSector ();
				
				// wait until read is complete
				//cd_image.WaitForSectorReadComplete ();
				
				/*
				if ( MODE.Report )
				{
					// report mode is on //
					
					u8 Temp_AFrac;
					
					// get afrac
					if ( cd_image.isSubOpen )
					{
						// get the pointer into the sub buffer here
						pSubBuffer = cd_image.GetCurrentSubBuffer ();
						
						Temp_AFrac = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 2 ];
					}
					else
					{
						Temp_AFrac = GetLocP_AFrac;
					}

					if ( Temp_AFrac == 0x00 || Temp_AFrac == 0x10 || Temp_AFrac == 0x20 || Temp_AFrac == 0x30 || Temp_AFrac == 0x40
						|| Temp_AFrac == 0x50 || Temp_AFrac == 0x60 || Temp_AFrac == 0x70 )
					{
						// first result in buffer is the status
						TempBuffer [ 0 ] = Status;
						
						if ( cd_image.isSubOpen )
						{
							// .sub file is available //
							
							TempBuffer [ 1 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackNumber;
							TempBuffer [ 2 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->IndexNumber;
						
							// on afrac = 0x00, 0x20, 0x40, 0x60 return absolute time
							if ( Temp_AFrac == 0x00 || Temp_AFrac == 0x20 || Temp_AFrac == 0x40 || Temp_AFrac == 0x60 )
							{
								// return absolute time
								TempBuffer [ 3 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 0 ];
								TempBuffer [ 4 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 1 ];
								TempBuffer [ 5 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 2 ];
							}
							else if ( Temp_AFrac == 0x10 || Temp_AFrac == 0x30 || Temp_AFrac == 0x50 || Temp_AFrac == 0x70 )
							{
								// return relative time with sec+0x80
								TempBuffer [ 3 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 0 ];
								TempBuffer [ 4 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 1 ] + 0x80;
								TempBuffer [ 5 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 2 ];
							}
						}
						else
						{
							// .sub file is NOT available
							
							TempBuffer [ 1 ] = GetLocP_Track;
							TempBuffer [ 2 ] = GetLocP_Index;
						
							// on afrac = 0x00, 0x20, 0x40, 0x60 return absolute time
							if ( Temp_AFrac == 0x00 || Temp_AFrac == 0x20 || Temp_AFrac == 0x40 || Temp_AFrac == 0x60 )
							{
								// return absolute time
								TempBuffer [ 3 ] = GetLocP_AMin;
								TempBuffer [ 4 ] = GetLocP_ASec;
								TempBuffer [ 5 ] = GetLocP_AFrac;
							}
							else if ( Temp_AFrac == 0x10 || Temp_AFrac == 0x30 || Temp_AFrac == 0x50 || Temp_AFrac == 0x70 )
							{
								// return relative time with sec+0x80
								TempBuffer [ 3 ] = GetLocP_Min;
								TempBuffer [ 4 ] = GetLocP_Sec + 0x80;
								TempBuffer [ 5 ] = GetLocP_Frac;
							}
						}
						
						TempBuffer [ 6 ] = 0;
						TempBuffer [ 7 ] = 0;
						EnqueueInterrupt_Read ( TempBuffer, 8, 0x1 );
					}
				}
				
				GetLocP_Index = 1;
				GetLocP_Track = Track;
				GetLocP_Min = Min;
				GetLocP_Sec = Sec;
				GetLocP_Frac = Frac;
				GetLocP_AMin = AMin;
				GetLocP_ASec = ASec;
				GetLocP_AFrac = AFrac;
				
				// update relative values too
				Inc_DiskPosition ( Min, Sec, Frac );
				
				// update to next sector for next read
				Inc_DiskPosition ( AMin, ASec, AFrac );
				*/
				
				// set the next command and busy cycles
				// we are going to read the next sector automatically and keep going
				ReadCommand = CDREG1_CMD_BACKWARD_2;
				BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READN ];
				
				// if we are reading at double speed, then we need to divide the busy cycles by 2
				if ( MODE.Speed ) BusyCycles_Read >>= 1;
				
				// get cycle for next read event
				SetNextReadCycle ( BusyCycles_Read );
				
				break;
				
			case CDREG1_CMD_READN_SEEK:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; READN_SEEK";
#endif

				// done with seeking
				Status &= ~CDREG1_READ_STAT_SEEK;
				
				// now reading
				Status |= CDREG1_READ_STAT_READ;
				
				// enqueue interrupt
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				ReadCommand = CDREG1_CMD_READN_2;
				BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READN ];
				
				// if we are reading at double speed, then we need to divide the busy cycles by 2
				if ( MODE.Speed ) BusyCycles_Read >>= 1;
				
				// get cycle for next read event
				SetNextReadCycle ( BusyCycles_Read );
				
				break;
				
			case CDREG1_CMD_READS_SEEK:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; READS_SEEK";
#endif

				// done with seeking
				Status &= ~CDREG1_READ_STAT_SEEK;
				
				// now reading
				Status |= CDREG1_READ_STAT_READ;
				
				// interrupt after the seek has completed
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				ReadCommand = CDREG1_CMD_READS_2;
				BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READS ];
				
				// if we are reading at double speed, then we need to divide the busy cycles by 2
				if ( MODE.Speed ) BusyCycles_Read >>= 1;
				
				// get cycle for next read event
				SetNextReadCycle ( BusyCycles_Read );
				
				break;
				
			case CDREG1_CMD_READN_2:
				
#ifdef INLINE_DEBUG_RUN
	debug << "; READN2";
#endif

				// set the next command and busy cycles
				// we are going to read the next sector automatically and keep going
				ReadCommand = CDREG1_CMD_READN_2;
				BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READN ];
				
				// if we are reading at double speed, then we need to divide the busy cycles by 2
				if ( MODE.Speed ) BusyCycles_Read >>= 1;
				
				// get cycle for next read event
				SetNextReadCycle ( BusyCycles_Read );
				
				
				Process_Read ();

				
				
				
#ifdef INLINE_DEBUG_RUN
	//DebugCount = 20;
#endif
				
				break;


			case CDREG1_CMD_READS_2:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; READS2" << hex << "; Filter_File=" << (u32)Filter_File << "; Filter_Chan=" << (u32)Filter_Chan;
#endif

				ReadCommand = CDREG1_CMD_READS_2;
				BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READS ];
				
				// account for double speed reading
				if ( MODE.Speed ) BusyCycles_Read >>= 1;
				
				// set the next cycle for read
				SetNextReadCycle ( BusyCycles_Read );
				
				
				Process_Read ();

				
				
				
#ifdef INLINE_DEBUG_RUN
	//DebugCount = 80;
#endif
			
				break;


			case CDREG1_CMD_STOP_2:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; STOP2";
#endif

				// make sure bit 1 is cleared in second response
				Status &= ~CDREG1_READ_STAT_STANDBY;
				
				// also make sure playing is stopped
				Status &= ~CDREG1_READ_STAT_PLAY;
				
				// also need to clear read command if we want to stop reading
				ReadCommand = -1;
				CurrentTrack = -1;
				
				// kill commands
				//Command = -1;
				//BusyCycles = 0xffffffffffffffffULL;
				ReadCommand = -1;
				//SetNextActionCycle ( 0 );
				//SetNextReadCycle ( 0 );
				
				TempBuffer [ 0 ] = Status;
				
				// enqueue secondary interrupt
				EnqueueInterrupt_Read ( TempBuffer, 1, 0x2 );

				// command done
				//Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;
				
				
				
			case CDREG1_CMD_PAUSE_2:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; PAUSE2";
#endif

				// second response returns the new status (with bit 5 cleared)

				// disk is still spinning, and that is just about all it is doing
				Status = CDREG1_READ_STAT_STANDBY;
				
				// also stop reading which could be ongoing
				//Status &= ~CDREG1_READ_STAT_READ;
				
				// it's also no longer playing anything
				//Status &= ~CDREG1_READ_STAT_PLAY;
				
				// kill commands
				//Command = -1;
				ReadCommand = -1;
				
				TempBuffer [ 0 ] = Status;
				
				// enqueue secondary interrupt
				EnqueueInterrupt_Read ( TempBuffer, 1, 0x2 );

				// command done
				//Command = -1;
				
				break;
				
				
			case CDREG1_CMD_INIT_2:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; INIT2";
#endif

				Status |= CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				
				// enqueue secondary interrupt
				EnqueueInterrupt_Read ( TempBuffer, 1, 0x2 );

				// abort all commands
				Command = -1;
				ReadCommand = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;
				

			case CDREG1_CMD_READT_2:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; READT2";
#endif

				// done seeking to session
				Status &= ~CDREG1_READ_STAT_SEEK;
				
				TempBuffer [ 0 ] = Status;
				
				EnqueueInterrupt_Read ( TempBuffer, 1, 0x2 );
				
				// abort read
				ReadCommand = -1;
				
				// command done
				//Command = -1;
				
				break;


			case CDREG1_CMD_SEEKL_2:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; SEEKL2";
#endif

				Status |= CDREG1_READ_STAT_STANDBY;	// disk is spinning
				Status &= ~CDREG1_READ_STAT_SEEK;	// not seeking anymore
				
				// after the seek, bit 7 is set to zero since audio playback is off
				Status &= ~CDREG1_READ_STAT_PLAY;
				
				TempBuffer [ 0 ] = Status;
				
				// enqueue secondary interrupt
				EnqueueInterrupt_Read ( TempBuffer, 1, 0x2 );

				// copy into sub buffer
				// seek should update data for GETLOC commands
				/*
				if ( cd_image.isSubOpen )
				{
					// copy subchannel data
					for ( int i = 0; i < DiskImage::CDImage::c_SubChannelSizePerSector; i++ ) SubBuffer [ i ] = cd_image.CurrentSubBuffer [ i ];
				}
				*/
				
				// command done
				ReadCommand = -1;
				//Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;

				
			case CDREG1_CMD_SEEKP_2:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; SEEKP2";
#endif

				Status |= CDREG1_READ_STAT_STANDBY;	// disk is spinning
				Status &= ~CDREG1_READ_STAT_SEEK;	// not seeking anymore
				
				// after the seek, bit 7 is set to zero since audio playback is off
				Status &= ~CDREG1_READ_STAT_PLAY;
				
				TempBuffer [ 0 ] = Status;
				
				// enqueue secondary interrupt
				EnqueueInterrupt_Read ( TempBuffer, 1, 0x2 );

				// copy into sub buffer
				// seek should update data for GETLOC commands
				/*
				if ( cd_image.isSubOpen )
				{
					// copy subchannel data
					for ( int i = 0; i < DiskImage::CDImage::c_SubChannelSizePerSector; i++ ) SubBuffer [ i ] = cd_image.CurrentSubBuffer [ i ];
				}
				*/
				
				// command done
				ReadCommand = -1;
				//Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;


			case CDREG1_CMD_ID_2:
			
#ifdef INLINE_DEBUG_RUN
				debug << "; ID2";
#endif

				// encapsulating this so that static constants lose scope afterwards
				{
				// read result { 2, 0, 0x20, 0, S, C, E, I }
				// do this for now, but this command REALLY identifies whether it is audio cd or game cd or not
				// will fix later
				// goto menu for now
				// last letter in "SCE" string actually depends on the region
				// North America = "SCEA"; Japan = "SCEI"; Europe = "SCEE"

				// if the lid/shell is open, it also does interrupt 0x5 instead of interrupt 0x2
				static const unsigned char ID_Results_LidOpen[] = { 0x11, 0x80 };

				// if door/lid/shell closed but no disk then interrupt 0x5 and...
				static const char ID_Results_LidClosed_NoDisk[] = { 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

				// if lid closed and audio disk then interrupt 0x5 and...
				static const unsigned char ID_Results_LidClosed_Audio[] = { 0x0a, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

				// if lid closed and game disk then interrupt 0x2 and...
				static const char ID_Results_LidClosed_Game[] = { 0x02, 0x00, 0x20, 0x00, 'S', 'C', 'E', 'I' };


				// this is the sutff from earlier
				//static const char ID_Results_Wierd [] = { 0x08, 0x80, 0x00, 0x00, 'S', 'C', 'E', 'I' };
				//static const char ID_Results_Menu [] = { 0x00, 0x80, 0x00, 0x00, 'S', 'C', 'E', 'I' };
				//static const char ID_Results_Audio [] = { 0x08, 0x90, 0x00, 0x00, 'S', 'C', 'E', 'I' };
				//static const char ID_Results_Game [] = { 0x08, 0x00, 0x00, 0x00, 'S', 'C', 'E', 'I' };


				if (isLidOpen)
				{
					// lid is open //

					for (int i = 0; i < sizeof(ID_Results_LidOpen); i++) TempBuffer[i] = ID_Results_LidOpen[i];

					// enqueue secondary interrupt
					EnqueueInterrupt_Read(TempBuffer, sizeof(ID_Results_LidOpen), 0x5);
				}
				else
				{
					// lid is closed //

					// check if there is a disk
					if (cd_image.isDiskOpen)
					{
						// there is a disk //
						if (isGameCD)
						{
							// is a game disk //

							// copy into temp buffer
							for (int i = 0; i < sizeof(ID_Results_LidClosed_Game); i++) TempBuffer[i] = ID_Results_LidClosed_Game[i];

							// put in the region
							TempBuffer[7] = Region;

							EnqueueInterrupt_Read(TempBuffer, sizeof(ID_Results_LidClosed_Game), 0x2);
						}
						else
						{
							// is an audio disk //

							for (int i = 0; i < sizeof(ID_Results_LidClosed_Audio); i++) TempBuffer[i] = ID_Results_LidClosed_Audio[i];
							EnqueueInterrupt_Read(TempBuffer, sizeof(ID_Results_LidClosed_Audio), 0x5);
						}
					}
					else
					{
						// there is no disk //

						for (int i = 0; i < sizeof(ID_Results_LidClosed_NoDisk); i++) TempBuffer[i] = ID_Results_LidClosed_NoDisk[i];
						EnqueueInterrupt_Read(TempBuffer, sizeof(ID_Results_LidClosed_NoDisk), 0x5);
					}
				}

				// command done
				ReadCommand = -1;
				//Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;

				}	// end scope for static constants for now

#ifdef INLINE_DEBUG_RUN
	//DebugCount = 80;
#endif
				
				break;


			case CDREG1_CMD_READTOC_2:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; READTOC2";
#endif

				Status |= CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				
				// enqueue secondary interrupt
				EnqueueInterrupt_Read ( TempBuffer, 1, 0x2 );

				// command done
				//Command = -1;
				
				// command started
				ReadCommand = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
#ifdef INLINE_DEBUG_RUN
	//DebugCount = 40;
#endif
				
				break;

				
			case CDREG1_CMD_LID_1:
			
				// disk is rotating
				Status |= CDREG1_READ_STAT_STANDBY;
				
				// act like shell is still open?
				//Status |= CDREG1_READ_STAT_SHELLOPEN;
				
				// no longer seeking
				//Status &= ~CDREG1_READ_STAT_SEEK;
			
				ReadCommand = CDREG1_CMD_LID_2;
				BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READS ] * 3;
				
				// set the next cycle for read
				SetNextReadCycle ( BusyCycles_Read );
					
				break;
				
				
			case CDREG1_CMD_LID_2:
			
				// not rotating, shell not open //
			
				// disk is not rotating and not open anymore?
				Status &= ~( CDREG1_READ_STAT_STANDBY | CDREG1_READ_STAT_SHELLOPEN );
				
				// shell not open anymore?
				//Status &= ~CDREG1_READ_STAT_SHELLOPEN;
			
				ReadCommand = CDREG1_CMD_LID_3;
				BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READS ] * 3;
				
				// set the next cycle for read
				SetNextReadCycle ( BusyCycles_Read );
					
				break;
				
				
			case CDREG1_CMD_LID_3:
			
				// seeking and spinning, get rid of error //
			
				// disk is rotating and seeking?
				Status |= ( CDREG1_READ_STAT_STANDBY | CDREG1_READ_STAT_SEEK );
				
				// get rid of error?
				Status &= ~CDREG1_READ_STAT_ERROR;
			
				ReadCommand = CDREG1_CMD_LID_4;
				BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READS ] * 3;
				
				// set the next cycle for read
				SetNextReadCycle ( BusyCycles_Read );
					
				break;
				
				
			case CDREG1_CMD_LID_4:
			
				// no longer seeking //
			
				// no longer seeking?
				Status &= ~CDREG1_READ_STAT_SEEK;
				
				// shell closing process is done
				ReadCommand = -1;
			
				//ReadCommand = CDREG1_CMD_LID_4;
				//BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READS ] * 3;
				
				// set the next cycle for read
				//SetNextReadCycle ( BusyCycles_Read );
					
				break;
				
				
			case 0xff:
				// command was cancelled //
				break;
				
			default:
				cout << "\nhps1x64 WARNING: Unknown CD READ command @ Cycle#" << dec << *_DebugCycleCount << " ReadCommand=" << hex << (u32)ReadCommand << " PC=" << *_DebugPC << "\n";
				break;

		}
	}

	
	if ( NextAction_Cycle != *_DebugCycleCount ) return;

	
	// clear next action event just in case
	SetNextActionCycle_Cycle ( -1ULL );
	

	// check if CD device was processing a command
	if ( Command != -1 )
	{
		// debugging: always show what happens after command
		DebugCount = INLINE_DEBUG_LINES;	//80;
		
		/////////////////////////////////////////////////
		// Command was being processsed by CD device
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nRun::CMD " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

		// clear next action event just in case
		// by setting to an absolute zero, sets a clear indication there is no event
		// but to simplify, will set to -1ULL
		//SetNextActionCycle ( 0 );

		// *** TESTING *** clear lower bit of reg0 in case this was set
		//CD_REG0_Reg &= ~1;

		// perform the command and set status and signal interrupt
		switch ( Command )
		{
			// *** TODO *** SYNC command 0x00
		
			///////////////////////////////////////
			// Nop - used to get status
			case CDREG1_CMD_NOP:	// 0x1 - Does nothing. Used to get the status.
			
#ifdef INLINE_DEBUG_RUN
	debug << "; NOP";
#endif

				// set cd as spinning
				// CDREG1 reads the status
				//Status |= CDREG1_READ_STAT_STANDBY;
				
				// check if the lid is open
				/*
				if ( isLidOpen )
				{
					// the lid is open //
					Status |= CDREG1_READ_STAT_SHELLOPEN;
					
					// disk can't be spinning when it is open
					Status &= ~CDREG1_READ_STAT_STANDBY;
				}
				*/
				
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
#ifdef INLINE_DEBUG_RUN
	debug << "; Status=" << hex << (u32)Status;
#endif

				// note: this command also has the special feature of resetting the lid/shell open flag if it is closed
				if ( !isLidOpen )
				{
					// lid/shell is closed //
					
					// clear shell open flag to indicate that lid/shell is closed
					Status &= ~CDREG1_READ_STAT_SHELLOPEN;
				}
								
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
#ifdef INLINE_DEBUG_RUN
	//DebugCount = 80;
	//debug << " _R3000A_Status=" << *_R3000A_Status << " _Intc_Stat=" << *_Intc_Stat << " _Intc_Mask=" << *_Intc_Mask << " _Intc_Master" << *_Intc_Master;
#endif
				
				break;
				
			
			case CDREG1_CMD_SETLOC:	// 0x2 - Sets target for SeekL and SeekP
			
#ifdef INLINE_DEBUG_RUN
	debug << "; SETLOC";
#endif

				// if there is no disk, then return an error
				if ( !cd_image.isDiskOpen )
				{
					// returns an error ?
					Status |= CDREG1_READ_STAT_ERROR;
					
					// returns an error ?
					TempBuffer [ 0 ] = Status;
					TempBuffer [ 1 ] = 0x80;
					
					EnqueueInterrupt ( TempBuffer, 2, 0x5 );
					
					// clear the error for the next command
					Status &= ~CDREG1_READ_STAT_ERROR;
					
					Command = -1;
					
					break;
				}

				// set cd as spinning
				// CDREG1 reads the status
				// note: setloc has no effect except to set next loc and mark as not seeked to
				//Status = CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// *** testing *** possibly need to cancel reading
				//ReadCommand = -1;
				
				// get the arguments
				// *note* the arguments ARE passed in BCD format for SETLOC command
				//AMin = ConvertBCDToDec ( Get_Parameter ( 0 ) );	//Arguments [ 0 ] );
				//ASec = ConvertBCDToDec ( Get_Parameter ( 1 ) );	//Arguments [ 1 ] );
				//AFrac = ConvertBCDToDec ( Get_Parameter ( 2 ) );	//Arguments [ 2 ] );
				SetLoc_Min = ConvertBCDToDec ( Get_Parameter ( 0 ) );
				SetLoc_Sec = ConvertBCDToDec ( Get_Parameter ( 1 ) );
				SetLoc_Frac = ConvertBCDToDec ( Get_Parameter ( 2 ) );
				
				// init relative values
				//Min = 0;
				//Sec = 0;
				//Frac = 0;
				
				// *** TODO *** get correct relative track values *** //
				/*
				AMin = SetLoc_Min;
				ASec = SetLoc_Sec;
				AFrac = SetLoc_Frac;
				
				// get the track number for this
				CurrentTrack = cd_image.FindTrack ( AMin, ASec, AFrac );
				
				// get where that track starts at
				cd_image.GetTrackStart ( CurrentTrack, Min, Sec, Frac );
				
				// get the relative offset from track start
				CDImage::SplitSectorNumber ( CDImage::GetSectorNumber ( AMin, ASec, AFrac ) - CDImage::GetSectorNumber ( Min, Sec, Frac ), Min, Sec, Frac );
				*/
				
				// location has not been seeked to yet
				// but only if it didn't seek to the location yet
				// note: looks like this location gets marked as not having been seeked to yet unconditionally
				/*
				if ( SetLoc_Min == AMin && SetLoc_Sec == ASec && SetLoc_Frac == AFrac && hasSeeked )
				{
					// already seeked to setloc location
				}
				else
				{
					// have not seeked to setloc location
					hasSeeked = false;
				}
				*/
				
				hasSeeked = false;
				
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
#ifdef INLINE_DEBUG_RUN
	debug << "; SetLoc_Min=" << dec << (u32) SetLoc_Min << "; SetLoc_Sec=" << (u32) SetLoc_Sec << "; SetLoc_Frac=" << (u32) SetLoc_Frac;
#endif

#ifdef INLINE_DEBUG_RUN
	//DebugCount = 80;
#endif

				break;
				
				
			// *** TODO *** PLAY command 0x3
			case CDREG1_CMD_PLAY:	// 0x3 - plays from the specified track or from current location if zero specified
			
#ifdef INLINE_DEBUG_RUN
	debug << "; PLAY";
	debug << "; Track=" << dec << Track;
#endif

#ifdef INLINE_DEBUG_PLAY
	debug << "\r\nRun::CMD " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount;
	debug << "; PLAY";
	debug << "; Track=" << dec << Track;
#endif

				// set cd as spinning
				// CDREG1 reads the status
				Status |= CDREG1_READ_STAT_STANDBY;
				
				// will be reading the first sector
				isReadingFirstSector = true;
				
				if ( !hasSeeked )
				{
#ifdef INLINE_DEBUG_RUN
	debug << "; !hasSeeked";
#endif

					// now seeking
					hasSeeked = true;
					
					// perform the seek //
				
					AMin = SetLoc_Min;
					ASec = SetLoc_Sec;
					AFrac = SetLoc_Frac;
					
#ifdef INLINE_DEBUG_RUN
	debug << "; AMin=" << dec << (u32)AMin << " ASec=" << (u32)ASec << " AFrac=" << (u32)AFrac;
#endif

					// get the track number for this
					CurrentTrack = cd_image.FindTrack ( AMin, ASec, AFrac );
					
#ifdef INLINE_DEBUG_RUN
	debug << "; CurrentTrack=" << (u32)CurrentTrack;
#endif

					// get where that track starts at
					cd_image.GetTrackStart ( CurrentTrack, Min, Sec, Frac );
					
					// get the relative offset from track start
					CDImage::SplitSectorNumber ( CDImage::GetSectorNumber ( AMin, ASec, AFrac ) - CDImage::GetSectorNumber ( Min, Sec, Frac ), Min, Sec, Frac );
				
#ifdef INLINE_DEBUG_RUN
	debug << "; Min=" << dec << (u32)AMin << " Sec=" << (u32)ASec << " Frac=" << (u32)Frac;
#endif

					// perform seek to location first
					cd_image.SeekTime ( AMin, ASec, AFrac );
#ifdef INLINE_DEBUG_RUN
	debug << "; ReadNextSector; isReadInProgress=" << dec << cd_image.isReadInProgress << "; isDiskOpen=" << cd_image.isDiskOpen << "; ReadIndex=" << cd_image.ReadIndex << "; WriteIndex=" << cd_image.WriteIndex << "; NextSector=" << cd_image.NextSector;
#endif
					
					// Start reading
					cd_image.StartReading ();
					
					
//#ifdef INLINE_DEBUG_RUN
//	debug << "; ReadNextSector; isReadInProgress=" << dec << cd_image.isReadInProgress << "; isDiskOpen=" << cd_image.isDiskOpen << "; ReadIndex=" << cd_image.ReadIndex << "; WriteIndex=" << cd_image.WriteIndex << "; NextSector=" << cd_image.NextSector << " hasSeeked=" << hasSeeked;
//#endif
					
					// status has changed
					Status |= CDREG1_READ_STAT_STANDBY;
					Status &= ~( CDREG1_READ_STAT_READ | CDREG1_READ_STAT_PLAY );
					
					// re-issue PLAY to happen after seek
					Command = CDREG1_CMD_PLAY;
					BusyCycles = CommandExecutionTimes [ CDREG1_CMD_SEEKP ];
					SetNextActionCycle ( BusyCycles );
					
					// enqueue interrupt
					//TempBuffer [ 0 ] = Status;
					//EnqueueInterrupt ( TempBuffer, 1, 0x3 );

					// set seek status after command has started
					Status |= CDREG1_READ_STAT_SEEK;
				}
				else
				{
#ifdef INLINE_DEBUG_RUN
	debug << "; hasSeeked";
#endif
					// disk is playing
					Status |= CDREG1_READ_STAT_PLAY;
					
					// no longer seeking
					Status &= ~CDREG1_READ_STAT_SEEK;
					
					TempBuffer [ 0 ] = Status;
					EnqueueInterrupt ( TempBuffer, 1, 0x3 );
					
#ifdef INLINE_DEBUG_RUN
	debug << "; ParameterBuf_Size=" << dec << ParameterBuf_Size;
	debug << "; Param=" << dec << (u32)Get_Parameter ( 0 ) << " Track=" << (u32)Track << " AMin=" << dec << (u32)AMin << " ASec=" << (u32)ASec << " AFrac=" << (u32)AFrac;
#endif
					
					// set Track - probably gets specified as BCD
					if ( ParameterBuf_Size > 0 )
					{

						Track = ConvertBCDToDec ( Get_Parameter ( 0 ) );
					}
					else
					{
						Track = 0;
					}
					
					// if trying to start the current track that is playing, then ignore command //
					if ( Track == CurrentTrack && Track != 0 )
					{
						// command is done //
						Command = -1;
						REG_Command = -1; ArgumentsIndex = 0;
						break;
					}
					
					if ( Track > 0 )
					{

						// wants to play the specified track //
						
						if ( Track <= cd_image.iNumberOfTracks )
						{
							CurrentTrack = Track;
						}
						
						// play from specified location //
						Min = 0;
						Sec = 0;
						Frac = 0;
						AMin = cd_image.TrackData [ CurrentTrack ].Min;
						ASec = cd_image.TrackData [ CurrentTrack ].Sec;
						AFrac = cd_image.TrackData [ CurrentTrack ].Frac;
					}
					else
					{
						// play from current location //
					}

#ifdef INLINE_DEBUG_RUN
	debug << "; Param=" << dec << (u32)Get_Parameter ( 0 ) << " Track=" << (u32)Track << " AMin=" << dec << (u32)AMin << " ASec=" << (u32)ASec << " AFrac=" << (u32)AFrac;
#endif

#ifdef INLINE_DEBUG_PLAY
	debug << "; Param=" << dec << (u32)Get_Parameter ( 0 ) << " Track=" << (u32)Track << " AMin=" << dec << (u32)AMin << " ASec=" << (u32)ASec << " AFrac=" << (u32)AFrac;
#endif

					// get the current sector number
					u32 CurrentTrack_Number, NextTrack_Number;
					u8 Next_AMin, Next_ASec, Next_AFrac;
					
					CurrentSector_Number = CDImage::GetSectorNumber ( AMin, ASec, AFrac );
					
					// get the sector number for the start of the next track
					CurrentTrack_Number = cd_image.FindTrack ( AMin, ASec, AFrac );
					NextTrack_Number = CurrentTrack_Number + 1;
					
					// get the start sector for the track
					cd_image.GetTrackStart ( NextTrack_Number, Next_AMin, Next_ASec, Next_AFrac );
					
					// make sure that amin,asec (excluding afrac) of current track are not equal to that of next track
					if ( AMin == Next_AMin && ASec == Next_ASec )
					{
						// try again
						NextTrack_Number++;
						cd_image.GetTrackStart ( NextTrack_Number, Next_AMin, Next_ASec, Next_AFrac );
					}
					
					NextTrackSector_Number = CDImage::GetSectorNumber ( Next_AMin, Next_ASec, Next_AFrac );
					
					
					// seek to track
					cd_image.SeekTime ( AMin, ASec, AFrac );
					
					// Start reading
					cd_image.StartReading ();
					
					
					// cd is now playing
					ReadCommand = CDREG1_CMD_PLAY_2;
					BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READN ];
					
					// if we are reading at double speed, then we need to divide the busy cycles by 2
					if ( MODE.Speed ) BusyCycles_Read >>= 1;
					
					// get cycle for next read event
					SetNextReadCycle ( BusyCycles_Read );
					
					// command done
					Command = -1;
					//REG_Command = -1; ArgumentsIndex = 0;
					
				}
				
				break;
			
			// *** TODO *** FORWARD command 0x4
			case CDREG1_CMD_FORWARD:	// 0x4 - goes forward continuosly
			
#ifdef INLINE_DEBUG_RUN
	debug << "; FORWARD";
#endif

				// set cd as spinning
				// CDREG1 reads the status
				Status |= CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// cd is now seeking forward
				ReadCommand = CDREG1_CMD_FORWARD_2;
				BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READN ];
				
				// if we are reading at double speed, then we need to divide the busy cycles by 2
				if ( MODE.Speed ) BusyCycles_Read >>= 1;
				
				// get cycle for next read event
				SetNextReadCycle ( BusyCycles_Read );
				
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;
			
			
			// *** TODO *** BACKWARD command 0x5
			case CDREG1_CMD_BACKWARD:	// 0x5 - goes backward continuously
			
#ifdef INLINE_DEBUG_RUN
	debug << "; BACKWARD";
#endif

				// set cd as spinning
				// CDREG1 reads the status
				Status |= CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// cd is now playing
				ReadCommand = CDREG1_CMD_BACKWARD_2;
				BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READN ];
				
				// if we are reading at double speed, then we need to divide the busy cycles by 2
				if ( MODE.Speed ) BusyCycles_Read >>= 1;
				
				// get cycle for next read event
				SetNextReadCycle ( BusyCycles_Read );
				
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;

				
			case CDREG1_CMD_READN:	// 0x6 - Read with retry. If ModeRept is on, then each sector causes IRQ.
				
#ifdef INLINE_DEBUG_RUN
	debug << "; READN";
#endif
				// *** Perform Command *** //
				
				// reset number of sectors read - used to determine when first sector is read
				isReadingFirstSector = 1;
				isReadingFirstAudioSector = 1;
				
				// start interpolation for sound from cd if it is not cd sound
				StartInterpolation ();
				
				// for now, set the data sector slot number to the sector data q slot number
				DataBuffer_SlotNumber = SectorDataQ_Index;
				
				// clear queued buffers
				for ( int i = 0; i < c_iMaxQueuedSectors; i++ ) SectorDataQ_Active [ i ] = 0;
				
				// looks like this may be needed for a number of things
				ReadCount = 0;
				
				// if location has not been seeked to yet, then perform seek and add in time for seek
				if ( !hasSeeked )
				{
					// now seeking
					hasSeeked = true;
					
					// perform the seek //
					
					// seeking should kill the read command
					ReadCommand = -1;
					
					
					// *** testing ***
					
					// status has changed
					Status |= CDREG1_READ_STAT_STANDBY;
					Status &= ~( CDREG1_READ_STAT_READ | CDREG1_READ_STAT_PLAY );
					
					// re-issue READN to happen after seek
					//ReadCommand = CDREG1_CMD_READN_SEEK;
					Command = CDREG1_CMD_READN;
					
					//BusyCycles = CommandExecutionTimes [ CDREG1_CMD_SEEKL ];
					BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_SEEKL ];
					//SetNextReadCycle ( BusyCycles );
					SetNextActionCycle ( BusyCycles );
					
					// enqueue interrupt
					//TempBuffer [ 0 ] = Status;
					//EnqueueInterrupt ( TempBuffer, 1, 0x3 );

					// set seek status after command has started
					Status |= CDREG1_READ_STAT_SEEK;
					
					
					AMin = SetLoc_Min;
					ASec = SetLoc_Sec;
					AFrac = SetLoc_Frac;
					
					/*
					// get the track number for this
					CurrentTrack = cd_image.FindTrack ( AMin, ASec, AFrac );
					
					// get where that track starts at
					cd_image.GetTrackStart ( CurrentTrack, Min, Sec, Frac );
					
					// get the relative offset from track start
					CDImage::SplitSectorNumber ( CDImage::GetSectorNumber ( AMin, ASec, AFrac ) - CDImage::GetSectorNumber ( Min, Sec, Frac ), Min, Sec, Frac );
				
					// set new disk position
					GetLocP_Index = 1;
					GetLocP_Track = CurrentTrack;
					GetLocP_Min = Min;
					GetLocP_Sec = Sec;
					GetLocP_Frac = Frac;
					GetLocP_AMin = AMin;
					GetLocP_ASec = ASec;
					GetLocP_AFrac = AFrac;
					*/
					
					//cd_image.SeekTime ( AMin, ASec, AFrac );
					cd_image.SeekTime ( SetLoc_Min, SetLoc_Sec, SetLoc_Frac );
					
#ifdef INLINE_DEBUG_RUN
	debug << "; ReadNextSector; isReadInProgress=" << dec << cd_image.isReadInProgress << "; isDiskOpen=" << cd_image.isDiskOpen << "; ReadIndex=" << cd_image.ReadIndex << "; WriteIndex=" << cd_image.WriteIndex << "; NextSector=" << cd_image.NextSector;
#endif
					
					cd_image.StartReading ();
					
#ifdef INLINE_DEBUG_RUN
	debug << "; ReadNextSector; isReadInProgress=" << dec << cd_image.isReadInProgress << "; isDiskOpen=" << cd_image.isDiskOpen << "; ReadIndex=" << cd_image.ReadIndex << "; WriteIndex=" << cd_image.WriteIndex << "; NextSector=" << cd_image.NextSector;
#endif
					
					// main command is done
					//Command = -1;
					
				}
				else
				{
					// if filter is not set, then clear SF
					//if ( !isFilterSet ) MODE.SF = 0;
					//isFilterSet = 0;
				
					Status |= CDREG1_READ_STAT_STANDBY;
					Status &= ~( CDREG1_READ_STAT_SEEK | CDREG1_READ_STAT_PLAY );

					// or maybe set before command starts??
					//Status |= CDREG1_READ_STAT_READ;
					

					// set read status after command has started
					Status |= CDREG1_READ_STAT_READ;
					
					// *** testing ***
					//ReadCount = 99999999;
					
					// enqueue interrupt
					TempBuffer [ 0 ] = Status;
					EnqueueInterrupt ( TempBuffer, 1, 0x3 );
					
					
					// set the next command and busy cycles
					// this needs to be a "read command" since other commands can execute concurrently with reads
					ReadCommand = CDREG1_CMD_READN_2;
					BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READN ];
					
					// if we are reading at double speed, then we need to divide the busy cycles by 2
					if ( MODE.Speed ) BusyCycles_Read >>= 1;
					SetNextReadCycle ( BusyCycles_Read );
					
					
					// main command is done
					Command = -1;
					//REG_Command = -1; ArgumentsIndex = 0;
				}

				break;
				
			
			case CDREG1_CMD_STANDBY:	// 0x7 - cd aborts all reads and playing but keeps spinning. cd does not try to keep its place.

#ifdef INLINE_DEBUG_RUN
	debug << "; STANDBY";
#endif

				// clear REG0 status
				//CD_REG0_Reg = 0;

				// first command probably returns the current status
				//Status = CDREG1_READ_STAT_STANDBY;
				
				// returns an error ?
				Status |= CDREG1_READ_STAT_ERROR;
				
				// returns an error ?
				TempBuffer [ 0 ] = Status;
				TempBuffer [ 1 ] = 0x20;
				//EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				EnqueueInterrupt ( TempBuffer, 2, 0x5 );
				
				// clear the error for the next command
				Status &= ~CDREG1_READ_STAT_ERROR;
				
				// doesn't affect reading ?
				// abort reading
				//ReadCommand = -1;
				
				// command done ?
				// set the next command and busy cycles
				//Command = CDREG1_CMD_STANDBY_2;
				//BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_STANDBY ];
				//SetNextActionCycle ( BusyCycles );
				Command = -1;

				// can put in another command
				//REG_Command = -1; ArgumentsIndex = 0;
				
#ifdef INLINE_DEBUG_RUN
				//DebugCount = 40;
#endif
				
				break;


			case CDREG1_CMD_STANDBY_2:

#ifdef INLINE_DEBUG_RUN
	debug << "; STANDBY2";
#endif

				// clear REG0 status
				CD_REG0_Reg = 0;

				// aborts all reads and playing, but continues spinning
				Status = CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				
				// enqueue secondary interrupt
				EnqueueInterrupt_Read ( TempBuffer, 1, 0x2 );
				
				// command done - so abort all reads and playing and stuff
				Command = -1;
				ReadCommand = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
#ifdef INLINE_DEBUG_RUN
	//DebugCount = 40;
#endif

				break;


			case CDREG1_CMD_STOP:	// 0x8 - Stops motor. Stops music??
			
#ifdef INLINE_DEBUG_RUN
	debug << "; STOP";
#endif

				// status might not be disk spinning??
				//Status |= CDREG1_READ_STAT_STANDBY;
				
				// make sure bit 5 is cleared in first response
				// should probably clear playing also (bit 7), but will leave this out for now
				// note: don't clear bit 2 yet, until the second response
				//Status &= ~CDREG1_READ_STAT_PLAY;
				Status &= ~CDREG1_READ_STAT_READ;
				
				// abort reading
				ReadCommand = -1;
				
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// set the next command and busy cycles
				//Command = CDREG1_CMD_STOP_2;
				//BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_STOP ];
				//SetNextActionCycle ( BusyCycles );
				ReadCommand = CDREG1_CMD_STOP_2;
				BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_STOP ];
				SetNextReadCycle ( BusyCycles );

				// can put in another command
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;


				
			case CDREG1_CMD_PAUSE:	// 0x9 - like standby, but maintains the current position

#ifdef INLINE_DEBUG_RUN
	debug << "; PAUSE";
#endif

#ifdef INLINE_DEBUG_RUN
	//DebugCount = 20;
#endif

				// first response returns current status (with bit 5 set if read was active)
				
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// stop the reading immediately - or should I do this later??
				ReadCommand = -1;
				
				// check if was reading
				if ( Status & CDREG1_READ_STAT_READ )
				{
					// was reading from cd //
					
					// set next read to start from current position //
					cd_image.SetFirstSector ();
				}
				
				// check if a read was in progress. If no read in progress then disk was already in a paused state
				//if ( Status & ( CDREG1_READ_STAT_READ | CDREG1_READ_STAT_PLAY ) )
				//{
					// disk is reading something //
					
					// set the next command and busy cycles
					//Command = CDREG1_CMD_PAUSE_2;
					//BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_PAUSE ];
					//SetNextActionCycle ( BusyCycles );
					ReadCommand = CDREG1_CMD_PAUSE_2;
					BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_PAUSE ];
					SetNextReadCycle ( BusyCycles );
				//}
				//else
				//{
				//	// disk was not reading anything //
				//	
				//	Command = CDREG1_CMD_PAUSE_2;
				//	BusyCycles = 0x800;	//CommandExecutionTimes_Secondary [ CDREG1_CMD_PAUSE ];
				//	SetNextActionCycle ( BusyCycles );
				//}
				
				
				// if reading, then the first stat will indicate reading, and the second one will have read flag cleared
				Status &= ~CDREG1_READ_STAT_READ;
				Status &= ~CDREG1_READ_STAT_PLAY;
				
#ifdef INLINE_DEBUG_RUN
	//DebugCount = 80;
#endif

				
				// can put in another command now - also clears out reg_command
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;

				break;

				
			
				
			///////////////////////////////////////////
			// Initialize CD
			case CDREG1_CMD_INIT:	// 0xa - Set Mode = 0, standby, aborts all commands
				
#ifdef INLINE_DEBUG_RUN
	debug << "; INIT";
#endif

				// set cd as spinning
				// CDREG1 reads the status
				Status = CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// clears mode to zero
				MODE.Value = 0;
				
				// abort reading
				ReadCommand = -1;
				
				// set the next command and busy cycles
				//Command = CDREG1_CMD_INIT_2;
				//BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_INIT ];
				//SetNextActionCycle ( BusyCycles );
				ReadCommand = CDREG1_CMD_INIT_2;
				BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_INIT ];
				SetNextReadCycle ( BusyCycles );
				
				// ready for new command
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;


			case CDREG1_CMD_MUTE:		// 0xb - turns off cdda streaming to spu

#ifdef INLINE_DEBUG_RUN
	debug << "; MUTE";
#endif

				// *** TODO *** MUTE CD AUDIO
				Mute = true;

				Status |= CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;
				

			case CDREG1_CMD_DEMUTE:		// 0xc - turns on cdda streaming to spu

#ifdef INLINE_DEBUG_RUN
	debug << "; DEMUTE";
#endif

				// *** TODO *** TURN ON CD AUDIO
				Mute = false;
				
				Status |= CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;
			

			case CDREG1_CMD_SETFILTER:		// 0xd - ignores data for sectors that do not have the same file and channel in subheader as specified

#ifdef INLINE_DEBUG_RUN
	debug << "; SETFILTER; File=" << (u32) Get_Parameter ( 0 ) << " Chan=" << (u32) Get_Parameter ( 1 );
#endif

				Status |= CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// get the arguments
				
				// Argument #1: File
				Filter_File = Get_Parameter ( 0 );	//Arguments [ 0 ];
				
				// Argument #2: Chan
				Filter_Chan = Get_Parameter ( 1 );	//Arguments [ 1 ];
				
				// filter is set
				//isFilterSet = true;
				
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;

				
			case CDREG1_CMD_SETMODE:	// 0xe - Sets parameters such as read mode and spin speed
			
#ifdef INLINE_DEBUG_RUN
	debug << "; SETMODE";
#endif

				// set cd as spinning
				// CDREG1 reads the status
				Status |= CDREG1_READ_STAT_STANDBY;
				
				// set the mode variable
				MODE.Value = Get_Parameter ( 0 );	//Arguments [ 0 ];
				
#ifdef INLINE_DEBUG_RUN
	debug << "; MODE=" << hex << (u32) MODE.Value;
#endif

				// stop the reading ??
				// *** testing ***
				// note: don't do this!
				//ReadCommand = -1;

				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// if mode is zero then stop reading???
				//if ( !MODE.Value )
				//{
				//	Status &= ~CDREG1_READ_STAT_READ;
				//	
				//	if ( ReadCommand == CDREG1_CMD_READN_2 || ReadCommand == CDREG1_CMD_READS_2 )
				//	{
				//		//ReadCommand = -1;
				//	}
				//}
				
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;

				

				
			case CDREG1_CMD_GETPARAM:		// 0xf - gets Stat,Mode,0,File,Chan
			
#ifdef INLINE_DEBUG_RUN
	debug << "; GETPARAM";
#endif

				// set cd as spinning
				// CDREG1 reads the status
				Status |= CDREG1_READ_STAT_STANDBY;
				
				// *** PERFORM COMMAND *** //
				// gets Stat,Mode,0,File,Chan
				
				// note: this is actually supposed to return MODE from setmode command and File/Chan from setfilter command
				TempBuffer [ 0 ] = Status;
				TempBuffer [ 1 ] = MODE.Value;
				TempBuffer [ 2 ] = 0;
				TempBuffer [ 3 ] = Filter_File;
				TempBuffer [ 4 ] = Filter_Chan;
				
#ifdef INLINE_DEBUG_RUN
	debug << "; Stat=" << hex << (u32) TempBuffer [ 0 ] << " Mode=" << (u32) TempBuffer [ 1 ] << " 0=" << (u32) TempBuffer [ 2 ] << " File=" << (u32) TempBuffer [ 3 ] << " Chan=" << (u32) TempBuffer [ 4 ];
#endif

				EnqueueInterrupt ( TempBuffer, 5, 0x3 );
				
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;


			case CDREG1_CMD_GETLOCL:		// 0x10 - gets the 13th to the 20th bytes of the last read sector
			
#ifdef INLINE_DEBUG_RUN
	debug << "; GETLOCL";
#endif

				// set cd as spinning
				// CDREG1 reads the status
				Status |= CDREG1_READ_STAT_STANDBY;
				
				// *** PERFORM COMMAND *** //
				// get 13th to 20th bytes of last read sector

				pSectorDataBuffer = cd_image.GetCurrentDataBuffer ();
				
				TempBuffer [ 0 ] = pSectorDataBuffer [ 12 ];
				TempBuffer [ 1 ] = pSectorDataBuffer [ 13 ];
				TempBuffer [ 2 ] = pSectorDataBuffer [ 14 ];
				TempBuffer [ 3 ] = pSectorDataBuffer [ 15 ];
				TempBuffer [ 4 ] = pSectorDataBuffer [ 16 ];
				TempBuffer [ 5 ] = pSectorDataBuffer [ 17 ];
				TempBuffer [ 6 ] = pSectorDataBuffer [ 18 ];
				TempBuffer [ 7 ] = pSectorDataBuffer [ 19 ];
				//TempBuffer [ 0 ] = GetLocL_Data [ 12 ];
				//TempBuffer [ 1 ] = GetLocL_Data [ 13 ];
				//TempBuffer [ 2 ] = GetLocL_Data [ 14 ];
				//TempBuffer [ 3 ] = GetLocL_Data [ 15 ];
				//TempBuffer [ 4 ] = GetLocL_Data [ 16 ];
				//TempBuffer [ 5 ] = GetLocL_Data [ 17 ];
				//TempBuffer [ 6 ] = GetLocL_Data [ 18 ];
				//TempBuffer [ 7 ] = GetLocL_Data [ 19 ];
				
#ifdef INLINE_DEBUG_RUN
	debug << "; Min=" << hex << (u32)TempBuffer [ 0 ] << " Sec=" << (u32)TempBuffer [ 1 ] << " Frac=" << (u32)TempBuffer[ 2 ] << " Mode=" << (u32)TempBuffer [ 3 ] << " File=" << (u32)TempBuffer [ 4 ] << " Chan=" << (u32)TempBuffer [ 5 ] << " SubMode=" << (u32)TempBuffer [ 6 ] << " Coding=" << (u32)TempBuffer [ 7 ];
#endif

				EnqueueInterrupt ( TempBuffer, 8, 0x3 );
				
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;



			case CDREG1_CMD_GETLOCP:		// 0x11 - gets Track,Index,Min,Sec,Frac,AMin,ASec,AFrac
			
#ifdef INLINE_DEBUG_RUN
	debug << "; GETLOCP";
#endif

				// *** testing ***
				//cout << "\nhps1x64: ALERT: CD: GetLocP\n";

				// set cd as spinning
				// CDREG1 reads the status
				Status |= CDREG1_READ_STAT_STANDBY;
				
				// *** PERFORM COMMAND *** //
				// get Track,Index,Min,Sec,Frac,AMin,ASec,AFrac
				// values are in dec, so convert to bcd
				
				// check if .sub file is open
				if ( cd_image.isSubOpen )
				{
#ifdef INLINE_DEBUG_RUN
	debug << "; REALSUBQ";
	debug << "; ReadIndex=" << dec << cd_image.ReadIndex << "; Next_ReadIndex=" << cd_image.Next_ReadIndex;
	// get info from disk too
	//pSectorDataBuffer = cd_image.GetDataBuffer ( cd_image.Next_ReadIndex );
	//debug << "; Header=" << hex << ((DiskImage::CDImage::Sector::Mode2_2328*)pSectorDataBuffer)->Header;
#endif

					// get the pointer into the sub buffer here
					pSubBuffer = cd_image.GetCurrentSubBuffer ();
						
					// real subchannel data is available //
					TempBuffer [ 0 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackNumber;
					TempBuffer [ 1 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->IndexNumber;
					TempBuffer [ 2 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 0 ];
					TempBuffer [ 3 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 1 ];
					TempBuffer [ 4 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->TrackRelativeAddress [ 2 ];
					TempBuffer [ 5 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 0 ];
					TempBuffer [ 6 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 1 ];
					TempBuffer [ 7 ] = ((CDImage::Sector::SubQ*)pSubBuffer)->AbsoluteAddress [ 2 ];
				}
				else
				{
#ifdef INLINE_DEBUG_RUN
	debug << "; LOCALSUBQ";
#endif

					
					// do not have real subchannel data since there is no .sub file //
					/*
					TempBuffer [ 0 ] = ConvertDecToBCD( GetLocP_Track );
					TempBuffer [ 1 ] = ConvertDecToBCD( GetLocP_Index );
					TempBuffer [ 2 ] = ConvertDecToBCD( GetLocP_Min );
					TempBuffer [ 3 ] = ConvertDecToBCD( GetLocP_Sec );
					TempBuffer [ 4 ] = ConvertDecToBCD( GetLocP_Frac );
					TempBuffer [ 5 ] = ConvertDecToBCD( GetLocP_AMin );
					TempBuffer [ 6 ] = ConvertDecToBCD( GetLocP_ASec );
					TempBuffer [ 7 ] = ConvertDecToBCD( GetLocP_AFrac );
					*/
					
					TempBuffer [ 0 ] = ConvertDecToBCD( cd_image.SubQ_Track );
					TempBuffer [ 1 ] = ConvertDecToBCD( cd_image.SubQ_Index );
					TempBuffer [ 2 ] = ConvertDecToBCD( cd_image.SubQ_Min );
					TempBuffer [ 3 ] = ConvertDecToBCD( cd_image.SubQ_Sec );
					TempBuffer [ 4 ] = ConvertDecToBCD( cd_image.SubQ_Frac );
					TempBuffer [ 5 ] = ConvertDecToBCD( cd_image.SubQ_AMin );
					TempBuffer [ 6 ] = ConvertDecToBCD( cd_image.SubQ_ASec );
					TempBuffer [ 7 ] = ConvertDecToBCD( cd_image.SubQ_AFrac );
				}
				
#ifdef INLINE_DEBUG_RUN
	debug << "; Track=" << hex << (u32)TempBuffer [ 0 ] << " Index=" << (u32)TempBuffer [ 1 ] << " Min=" << (u32)TempBuffer [ 2 ] << " Sec=" << (u32)TempBuffer [ 3 ] << " Frac=" << (u32)TempBuffer [ 4 ] << " AMin=" << (u32)TempBuffer [ 5 ] << " ASec=" << (u32)TempBuffer [ 6 ] << " AFrac=" << (u32)TempBuffer [ 7 ];
#endif
				
				EnqueueInterrupt ( TempBuffer, 8, 0x3 );
				
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
#ifdef INLINE_DEBUG_RUN
	//DebugCount = 40;
#endif

				break;
				
				
			// *** TODO *** READT command 0x12
			case CDREG1_CMD_READT:
			
#ifdef INLINE_DEBUG_RUN
	debug << "; READT";
	debug << "; Session=" << (u32) Get_Parameter ( 0 );
#endif

				// set cd as spinning
				// CDREG1 reads the status
				Status |= CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				
				// seeking to session
				Status |= CDREG1_READ_STAT_SEEK;
				
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				//Command = CDREG1_CMD_READT_2;
				//BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_READT ];
				//SetNextActionCycle ( BusyCycles );
				ReadCommand = CDREG1_CMD_READT_2;
				BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_READT ];
				SetNextReadCycle ( BusyCycles );
				
				// command done
				Command = -1;
				
				break;
				
				

				
			case CDREG1_CMD_GETTN:	// 0x13 - Get first track number and number of tracks in the TOC
			
#ifdef INLINE_DEBUG_RUN
	debug << "; GETTN";
#endif

				// set cd as spinning
				// CDREG1 reads the status
				Status |= CDREG1_READ_STAT_STANDBY;
				
				// *** TODO *** get first track number and number of tracks in TOC
				TempBuffer [ 0 ] = Status;
				
				// *** TODO *** get first track number
				TempBuffer [ 1 ] = 0x01;
				
				// *** TODO *** get number of tracks in TOC
				//TempBuffer [ 2 ] = cd_image.iNumberOfTracks;
				//TempBuffer [ 2 ] = ConvertDecToBCD ( cd_image.iNumberOfTracks );
				TempBuffer [ 2 ] = ConvertDecToBCD ( cd_image.IndexData [ cd_image.iNumberOfIndexes - 1 ].Track );
				
#ifdef INLINE_DEBUG_RUN
	debug << "; Stat=" << hex << (u32)TempBuffer [ 0 ] << " First=" << (u32)TempBuffer [ 1 ] << " Last=" << (u32)TempBuffer [ 2 ];
#endif
				

				EnqueueInterrupt ( TempBuffer, 3, 0x3 );
				
				// *** testing *** this might stop reading
				Status &= ~( CDREG1_READ_STAT_READ | CDREG1_READ_STAT_PLAY );
				//ReadCommand = -1;
				// try stopping reading after a certain number of reads
				//ReadCount = 8;
				
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;

				
				
			case CDREG1_CMD_GETTD:	// 0x14 - Gets start of specified track
			
#ifdef INLINE_DEBUG_RUN
	debug << "; GETTD; (BCD) Track=" << hex << (u32)(Get_Parameter ( 0 )) << " (DEC) Track=" << dec << (u32)ConvertBCDToDec ( Get_Parameter ( 0 ) );
#endif

				// set cd as spinning
				// CDREG1 reads the status
				Status |= CDREG1_READ_STAT_STANDBY;
				
				/////////////////////////////
				// RETURNS: STAT, AMIN, ASEC
				
				// get stat
				TempBuffer [ 0 ] = Status;
				
				if ( ConvertBCDToDec ( Get_Parameter ( 0 ) ) > 0 )
				{
					// get the start of the track number specified
					cd_image.GetTrackStart ( ConvertBCDToDec ( Get_Parameter ( 0 ) ), TMin, TSec, TFrac );
				
					// *** TODO *** get the Min location of specified track in TOC
					//TempBuffer [ 1 ] = cd_image.TrackData [ Get_Parameter ( 0 ) ].Min;
					//TempBuffer [ 1 ] = ConvertDecToBCD ( cd_image.TrackData [ ConvertBCDToDec ( Get_Parameter ( 0 ) ) ].Min );
					TempBuffer [ 1 ] = ConvertDecToBCD ( TMin );
					
					// *** TODO *** get the Sec location of specified track in TOC
					//TempBuffer [ 2 ] = cd_image.TrackData [ Get_Parameter ( 0 ) ].Sec;
					//TempBuffer [ 2 ] = ConvertDecToBCD ( cd_image.TrackData [ ConvertBCDToDec ( Get_Parameter ( 0 ) ) ].Sec );
					TempBuffer [ 2 ] = ConvertDecToBCD ( TSec );
					
					// *** testing *** maybe track 1 should start at MIN=0 SEC=2?
					// note: all times get added with 2 seconds actually
					/*
					if ( TempBuffer [ 1 ] == 0 && TempBuffer [ 2 ] == 0 )
					{
						TempBuffer [ 1 ] = 0;
						TempBuffer [ 2 ] = 2;
					}
					*/
				}
				else
				{
					// wants the end of the last track
					//TempBuffer [ 1 ] = ConvertDecToBCD ( cd_image.TrackData [ cd_image.iNumberOfTracks + 1 ].Min );
					//TempBuffer [ 2 ] = ConvertDecToBCD ( cd_image.TrackData [ cd_image.iNumberOfTracks + 1 ].Sec );
					TempBuffer [ 1 ] = ConvertDecToBCD ( cd_image.IndexData [ cd_image.iNumberOfIndexes ].AMin );
					TempBuffer [ 2 ] = ConvertDecToBCD ( cd_image.IndexData [ cd_image.iNumberOfIndexes ].ASec );
				}
				
#ifdef INLINE_DEBUG_RUN
	debug << hex << "; Output: MIN=" << (u32)TempBuffer [ 1 ] << " SEC=" << (u32)TempBuffer [ 2 ];
#endif

				//EnqueueInterrupt ( TempBuffer, 2, 0x3 );
				EnqueueInterrupt ( TempBuffer, 3, 0x3 );
				
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;


			case CDREG1_CMD_SEEKL:	// 0x15 - seek to SetLoc's location in data mode. Can only seek to data sectors, but is accurate to the sector
			
#ifdef INLINE_DEBUG_RUN
	debug << "; SEEKL";
#endif

				// *** PERFORM COMMAND *** //
				// seek to the specified location on disk

				// set cd as spinning
				// CDREG1 reads the status
				Status |= CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// this must be incase status gets checked in the meantime
				Status |= CDREG1_READ_STAT_SEEK;
				
				// seek should abort reading
				Status &= ~( CDREG1_READ_STAT_READ | CDREG1_READ_STAT_PLAY );	//~CDREG1_READ_STAT_READ;
				ReadCommand = -1;
				
				// set the next command and busy cycles
				//Command = CDREG1_CMD_SEEKL_2;
				//BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_SEEKL ];
				//SetNextActionCycle ( BusyCycles );
				ReadCommand = CDREG1_CMD_SEEKL_2;
				BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_SEEKL ];
				SetNextReadCycle ( BusyCycles );

				
				// perform the seek here //
				
				AMin = SetLoc_Min;
				ASec = SetLoc_Sec;
				AFrac = SetLoc_Frac;
/*				
#ifdef INLINE_DEBUG_RUN
	debug << "; AMin=" << dec << (u32)AMin << " ASec=" << (u32)ASec << " AFrac=" << (u32)AFrac;
#endif

				// get the track number for this
				CurrentTrack = cd_image.FindTrack ( AMin, ASec, AFrac );
				
#ifdef INLINE_DEBUG_RUN
	debug << "; CurrentTrack=" << (u32)CurrentTrack;
#endif

				// get where that track starts at
				cd_image.GetTrackStart ( CurrentTrack, Min, Sec, Frac );
				
				// get the relative offset from track start
				CDImage::SplitSectorNumber ( CDImage::GetSectorNumber ( AMin, ASec, AFrac ) - CDImage::GetSectorNumber ( Min, Sec, Frac ), Min, Sec, Frac );
				
#ifdef INLINE_DEBUG_RUN
	debug << "; Min=" << dec << (u32)AMin << " Sec=" << (u32)ASec << " Frac=" << (u32)Frac;
#endif
*/

				// has now seeked to location, go ahead and start reading too
				hasSeeked = true;
				
				// perform seek to location first
				//cd_image.SeekTime ( AMin, ASec, AFrac );
				cd_image.SeekTime ( SetLoc_Min, SetLoc_Sec, SetLoc_Frac );
				
				/*
				// set new disk position
				GetLocP_Index = 1;
				GetLocP_Track = CurrentTrack;
				GetLocP_Min = Min;
				GetLocP_Sec = Sec;
				GetLocP_Frac = Frac;
				GetLocP_AMin = AMin;
				GetLocP_ASec = ASec;
				GetLocP_AFrac = AFrac;
				*/
					
#ifdef INLINE_DEBUG_RUN
	debug << "; ReadNextSector; isReadInProgress=" << dec << cd_image.isReadInProgress << "; isDiskOpen=" << cd_image.isDiskOpen << "; ReadIndex=" << cd_image.ReadIndex << "; WriteIndex=" << cd_image.WriteIndex << "; NextSector=" << cd_image.NextSector;
#endif
				
				// Start reading
				cd_image.StartReading ();
				
#ifdef INLINE_DEBUG_RUN
	debug << "; ReadNextSector; isReadInProgress=" << dec << cd_image.isReadInProgress << "; isDiskOpen=" << cd_image.isDiskOpen << "; ReadIndex=" << cd_image.ReadIndex << "; WriteIndex=" << cd_image.WriteIndex << "; NextSector=" << cd_image.NextSector;
#endif

				// can put in another command
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;
				
			


			case CDREG1_CMD_SEEKP:	// 0x16 - seek to SetLoc's location in audio mode. Only accurate to the second
			
#ifdef INLINE_DEBUG_RUN
	debug << "; SEEKP";
#endif

				// *** PERFORM COMMAND *** //
				// seek to the specified location on disk

				// set cd as spinning
				// CDREG1 reads the status
				Status |= CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// this must be incase status gets checked in the meantime
				Status |= CDREG1_READ_STAT_SEEK;
				
				// seek should abort reading
				Status &= ~( CDREG1_READ_STAT_READ | CDREG1_READ_STAT_PLAY );	//~CDREG1_READ_STAT_READ;
				ReadCommand = -1;
				
				// set the next command and busy cycles
				//Command = CDREG1_CMD_SEEKP_2;
				//BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_SEEKP ];
				//SetNextActionCycle ( BusyCycles );
				ReadCommand = CDREG1_CMD_SEEKP_2;
				BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_SEEKP ];
				SetNextReadCycle ( BusyCycles );
				
				// perform the seek here //

				// seekp is only accurate to the second
				// no, that is incorrect according to Martin Korth PSX Spec and it actually seeks to the MM:SS:FF
				AMin = SetLoc_Min;
				ASec = SetLoc_Sec;
				AFrac = SetLoc_Frac;
				
				// for now, go to start of next track at index 1
				//u32 TheTrackNumber;
				//TheTrackNumber = cd_image.FindTrack ( AMin, ASec, AFrac );
				
				// go to next track
				//cd_image.GetTrackStart ( TheTrackNumber + 1, AMin, ASec, AFrac );
				
				/*
				// get the track number for this
				CurrentTrack = cd_image.FindTrack ( AMin, ASec, AFrac );
				
				// get where that track starts at
				cd_image.GetTrackStart ( CurrentTrack, Min, Sec, Frac );
				
				// get the relative offset from track start
				CDImage::SplitSectorNumber ( CDImage::GetSectorNumber ( AMin, ASec, AFrac ) - CDImage::GetSectorNumber ( Min, Sec, Frac ), Min, Sec, Frac );
				
				// set new disk position
				GetLocP_Index = 1;
				GetLocP_Track = CurrentTrack;
				GetLocP_Min = Min;
				GetLocP_Sec = Sec;
				GetLocP_Frac = Frac;
				GetLocP_AMin = AMin;
				GetLocP_ASec = ASec;
				GetLocP_AFrac = AFrac;
				*/
				
				
				// has now seeked to location, go ahead and start reading too
				hasSeeked = true;
				
				// perform seek to location first
				// only accurate to the second
				// according to PSX Specification, not only accurate to the second
				//cd_image.SeekTime ( AMin, ASec, 0 );
				//cd_image.SeekTime ( AMin, ASec, AFrac );
				cd_image.SeekTime ( SetLoc_Min, SetLoc_Sec, SetLoc_Frac );
				
#ifdef INLINE_DEBUG_RUN
	debug << "; ReadNextSector; isReadInProgress=" << dec << cd_image.isReadInProgress << "; isDiskOpen=" << cd_image.isDiskOpen << "; ReadIndex=" << cd_image.ReadIndex << "; WriteIndex=" << cd_image.WriteIndex << "; NextSector=" << cd_image.NextSector;
#endif
				
				// Start reading
				cd_image.StartReading ();
				
#ifdef INLINE_DEBUG_RUN
	debug << "; ReadNextSector; isReadInProgress=" << dec << cd_image.isReadInProgress << "; isDiskOpen=" << cd_image.isDiskOpen << "; ReadIndex=" << cd_image.ReadIndex << "; WriteIndex=" << cd_image.WriteIndex << "; NextSector=" << cd_image.NextSector;
	pSectorDataBuffer = cd_image.GetDataBuffer ( cd_image.Next_ReadIndex );
	debug << "; Header=" << hex << ((DiskImage::CDImage::Sector::Mode2_2328*)pSectorDataBuffer)->Header;
#endif
				
				// can put in another command
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;
				
			

				
				
			// *** TODO *** SETCLOCK command 0x17
			case CDREG1_CMD_SETCLOCK:

#ifdef INLINE_DEBUG_RUN
	debug << "; SETCLOCK";
#endif

				// returns an error ?
				Status |= CDREG1_READ_STAT_ERROR;
				
				// returns an error ?
				TempBuffer [ 0 ] = Status;
				TempBuffer [ 1 ] = 0x40;
				EnqueueInterrupt ( TempBuffer, 2, 0x5 );
				
				// clear the error for the next command
				Status &= ~CDREG1_READ_STAT_ERROR;
				
				
				// can put in another command
				Command = -1;
				
				break;
			
			
			// *** TODO *** GETCLOCK command 0x18
			case CDREG1_CMD_GETCLOCK:

#ifdef INLINE_DEBUG_RUN
	debug << "; GETCLOCK";
#endif

				// returns an error ?
				Status |= CDREG1_READ_STAT_ERROR;
				
				// returns an error ?
				TempBuffer [ 0 ] = Status;
				TempBuffer [ 1 ] = 0x40;
				EnqueueInterrupt ( TempBuffer, 2, 0x5 );
				
				// clear the error for the next command
				Status &= ~CDREG1_READ_STAT_ERROR;
				
				
				// can put in another command
				Command = -1;
				
				break;
				
				
			case CDREG1_CMD_TEST:	// 0x19 - has many subcommands which are completely different
			
#ifdef INLINE_DEBUG_RUN
	debug << "; TEST; ArgumentsIndex=" << dec << ArgumentsIndex << "; Arguments [ 0 ]=" << hex << (u32)(Get_Parameter ( 0 )) << ";";
#endif

				// containing scope of the static consts for the vc++ compiler
				{
				// these constants are ripped from pcsx
				static const unsigned char Test04[] = { 0 };
				static const unsigned char Test05[] = { 0 };
				static const unsigned char Test20[] = { 0x98, 0x06, 0x10, 0xC3 };
				static const unsigned char Test22[] = { 0x66, 0x6F, 0x72, 0x20, 0x45, 0x75, 0x72, 0x6F };
				static const unsigned char Test23[] = { 0x43, 0x58, 0x44, 0x32, 0x39 ,0x34, 0x30, 0x51 };
				
				// check the argument from cdreg2
				switch ( Get_Parameter ( 0 ) )	//;	Arguments [ 0 ] )
				{
					case 0x04:
#ifdef INLINE_DEBUG_RUN
	debug << "; Test04 Get_Parameter(0)=" << (u32)Get_Parameter( 0 );
#endif
						for ( int i = 0; i < sizeof( Test04 ); i++ ) TempBuffer [ i ] = Test04 [ i ];
						EnqueueInterrupt ( TempBuffer, sizeof( Test04 ), 0x3 );
						break;
						
					case 0x05:
#ifdef INLINE_DEBUG_RUN
	debug << "; Test05";
#endif
						for ( int i = 0; i < sizeof( Test05 ); i++ ) TempBuffer [ i ] = Test05 [ i ];
						EnqueueInterrupt ( TempBuffer, sizeof( Test05 ), 0x3 );
						break;
						
					case 0x20:

						for ( int i = 0; i < sizeof( Test20 ); i++ ) TempBuffer [ i ] = Test20 [ i ];
						
#ifdef INLINE_DEBUG_RUN
	debug << "; Test20=" << (u32)Test20 [ 0 ] << " " << (u32)Test20 [ 1 ] << " " << (u32)Test20 [ 2 ] << " " << (u32)Test20 [ 3 ];
	debug << "; TempBuffer=" << (u32)TempBuffer [ 0 ] << " " << (u32)TempBuffer [ 1 ] << " " << (u32)TempBuffer [ 2 ] << " " << (u32)TempBuffer [ 3 ];
#endif

						EnqueueInterrupt ( TempBuffer, sizeof( Test20 ), 0x3 );
						break;
						
					case 0x22:
#ifdef INLINE_DEBUG_RUN
	debug << "; Test22";
#endif
						for ( int i = 0; i < sizeof( Test22 ); i++ ) TempBuffer [ i ] = Test22 [ i ];
						EnqueueInterrupt ( TempBuffer, sizeof( Test22 ), 0x3 );
						break;
						
					case 0x23:
#ifdef INLINE_DEBUG_RUN
	debug << "; Test23";
#endif
						for ( int i = 0; i < sizeof( Test23 ); i++ ) TempBuffer [ i ] = Test23 [ i ];
						EnqueueInterrupt ( TempBuffer, sizeof( Test23 ), 0x3 );
						break;
				}
				
				// command done
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;

				}	// end scope of static constants
				
#ifdef INLINE_DEBUG_RUN
	//DebugCount = 20;
#endif
				
				break;

				
			///////////////////////////////////////////////////
			// Identify type of CD
			case CDREG1_CMD_ID:	// 0x1a - returns whether game disk, audio disk, etc
				
#ifdef INLINE_DEBUG_RUN
	debug << "; ID";
#endif

				Status = CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				
				// I'll have all commands with a secondary interrupt stop the reading
				ReadCommand = -1;
				
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// set the next command and busy cycles
				//Command = CDREG1_CMD_ID_2;
				//BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_ID ];
				//SetNextActionCycle ( BusyCycles );
				ReadCommand = CDREG1_CMD_ID_2;
				BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_ID ];
				SetNextReadCycle ( BusyCycles );
								
				// can put in another command
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
#ifdef INLINE_DEBUG_RUN
	//DebugCount = 40;
#endif
				
				break;
				


			case CDREG1_CMD_READS:	// command 0x1b
			
#ifdef INLINE_DEBUG_RUN
	debug << "; READS";
#endif

				// *** Perform Command *** //
				
				/*
				if ( ReadCommand == CDREG1_CMD_READS_2 )
				{
					TempBuffer [ 0 ] = Status;
					EnqueueInterrupt ( TempBuffer, 1, 0x3 );
					
					// main command is done
					Command = -1;
					REG_Command = -1; ArgumentsIndex = 0;
					
					break;
				}
				*/
				
				// check if the next sector to be read is the same as set loc sector
				/*
				u32 SecNum;
				SecNum = cd_image.GetSectorNumber ( SetLoc_Min, SetLoc_Sec, SetLoc_Frac );
				if ( SecNum == ( cd_image.CurrentSector + 1 ) )
				{
#ifdef INLINE_DEBUG_RUN
	debug << "; Proceeding";
#endif

					// wants to seek to the next sector to be read anyway
					hasSeeked = true;
					
					// send int3
					TempBuffer [ 0 ] = Status;
					EnqueueInterrupt ( TempBuffer, 1, 0x3 );
					
					// main command is done
					Command = -1;
					
					break;
				}
				*/
				
				// reset number of sectors read - used to determine when first sector is read
				isReadingFirstSector = 1;
				isReadingFirstAudioSector = 1;
				
				// start interpolation for sound from cd if it is not cd sound
				StartInterpolation ();

				// for now, set the data sector slot number to the sector data q slot number
				DataBuffer_SlotNumber = SectorDataQ_Index;
				
				// clear queued buffers
				for ( int i = 0; i < c_iMaxQueuedSectors; i++ ) SectorDataQ_Active [ i ] = 0;
				
				// looks like this may be needed for a number of things
				ReadCount = 0;
				
				if ( !hasSeeked )
				{
#ifdef INLINE_DEBUG_RUN
	debug << "; !hasSeeked";
#endif

					// now seeking
					hasSeeked = true;
					
					// perform the seek //
					
					// seeking should kill the read command
					ReadCommand = -1;
				
					
					AMin = SetLoc_Min;
					ASec = SetLoc_Sec;
					AFrac = SetLoc_Frac;
					
					/*
					// get the track number for this
					CurrentTrack = cd_image.FindTrack ( AMin, ASec, AFrac );
					
					// get where that track starts at
					cd_image.GetTrackStart ( CurrentTrack, Min, Sec, Frac );
					
					// get the relative offset from track start
					CDImage::SplitSectorNumber ( CDImage::GetSectorNumber ( AMin, ASec, AFrac ) - CDImage::GetSectorNumber ( Min, Sec, Frac ), Min, Sec, Frac );
				
#ifdef INLINE_DEBUG_RUN
	debug << "; ReadNextSector; isReadInProgress=" << dec << cd_image.isReadInProgress << "; isDiskOpen=" << cd_image.isDiskOpen << "; ReadIndex=" << cd_image.ReadIndex << "; WriteIndex=" << cd_image.WriteIndex << "; NextSector=" << cd_image.NextSector;
#endif

					// set new disk position
					GetLocP_Index = 1;
					GetLocP_Track = CurrentTrack;
					GetLocP_Min = Min;
					GetLocP_Sec = Sec;
					GetLocP_Frac = Frac;
					GetLocP_AMin = AMin;
					GetLocP_ASec = ASec;
					GetLocP_AFrac = AFrac;
					*/
					
					// perform seek to location first
					//cd_image.SeekTime ( AMin, ASec, AFrac );
					cd_image.SeekTime ( SetLoc_Min, SetLoc_Sec, SetLoc_Frac );
					
					// Start reading
					cd_image.StartReading ();
					
//#ifdef INLINE_DEBUG_RUN
//	debug << "; ReadNextSector; isReadInProgress=" << dec << cd_image.isReadInProgress << "; isDiskOpen=" << cd_image.isDiskOpen << "; ReadIndex=" << cd_image.ReadIndex << "; WriteIndex=" << cd_image.WriteIndex << "; NextSector=" << cd_image.NextSector << " hasSeeked=" << hasSeeked;
//#endif
					
					// *** testing ***
					//ReadCount = 99999999;
					
					// status has changed
					Status |= CDREG1_READ_STAT_STANDBY;
					Status &= ~( CDREG1_READ_STAT_READ | CDREG1_READ_STAT_PLAY );
					
					// re-issue READN to happen after seek
					//ReadCommand = CDREG1_CMD_READS_SEEK;
					//BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_SEEKL ];
					//SetNextReadCycle ( BusyCycles );
					// *** testing ***
					// note: MUST interrupt AFTER the seek
					Command = CDREG1_CMD_READS;
					BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_SEEKL ];
					SetNextActionCycle ( BusyCycles );
					
					//TempBuffer [ 0 ] = Status;
					//EnqueueInterrupt ( TempBuffer, 1, 0x3 );
					
					// set seek status after command has started
					Status |= CDREG1_READ_STAT_SEEK;
					
					// main command is done
					//Command = -1;
				}
				else
				{
#ifdef INLINE_DEBUG_RUN
	debug << "; hasSeeked";
#endif

					// if filter is not set, then clear SF
					//if ( !isFilterSet ) MODE.SF = 0;
					//isFilterSet = 0;
				
					Status |= CDREG1_READ_STAT_STANDBY;
					Status &= ~( CDREG1_READ_STAT_SEEK | CDREG1_READ_STAT_PLAY );

					// set read status after command has started
					Status |= CDREG1_READ_STAT_READ;
					
					TempBuffer [ 0 ] = Status;
					EnqueueInterrupt ( TempBuffer, 1, 0x3 );

					
					// set the next command and busy cycles
					// this needs to be a "read command" since other commands can execute concurrently with reads
					ReadCommand = CDREG1_CMD_READS_2;
					BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READS ];
					
					// if we are reading at double speed, then we need to divide the busy cycles by 2
					if ( MODE.Speed ) BusyCycles_Read >>= 1;
					SetNextReadCycle ( BusyCycles_Read );
					
					// main command is done
					Command = -1;
					//REG_Command = -1; ArgumentsIndex = 0;
				}
				
#ifdef INLINE_DEBUG_RUN
	//DebugCount = 20;
#endif
				
				break;

				
			case CDREG1_CMD_RESET:	// 0x1c - resets to home position
			
#ifdef INLINE_DEBUG_RUN
	debug << "; RESET";
#endif

				// set cd as spinning
				// CDREG1 reads the status
				Status = CDREG1_READ_STAT_STANDBY;
				
				TempBuffer [ 0 ] = Status;
				EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				// result is ready
				CD_REG0_Reg |= CDREG0_READ_STAT_RESULTREADY;
				
				// *** Perform Reset Here *** //
				
				
				// stop the reading
				ReadCommand = -1;
				
				// abort all commands
				Command = -1;
				ReadCommand = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
				break;
				
			case CDREG1_CMD_READTOC:	// 0x1e

#ifdef INLINE_DEBUG_RUN
	debug << "; READTOC";
#endif

#ifdef INLINE_DEBUG_RUN
	//DebugCount = 40;
#endif

				// disk is spinning
				Status |= CDREG1_READ_STAT_STANDBY;
				
				// not returning any of the read data
				ReadMode = 0;
				
				// check if command is starting
				/*
				if ( isCommandStart )
				{
#ifdef INLINE_DEBUG_RUN
	debug << "; CommandStart";
#endif

					isCommandStart = false;
					
					TempBuffer [ 0 ] = Status;
					EnqueueInterrupt ( TempBuffer, 1, 0x10 );
					
					BusyCycles = CommandExecutionTimes [ CDREG1_CMD_READTOC ];
					SetNextActionCycle ( BusyCycles );
					
					// I would imagine this aborts read command since it has to read TOC from the disk
					ReadCommand = -1;
					
					// command started
					REG_Command = -1; ArgumentsIndex = 0;
					
					break;
				}
				*/
				
				TempBuffer [ 0 ] = Status;
				//EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				
				//if ( IntQueue.Peek )
				//{
					EnqueueInterrupt ( TempBuffer, 1, 0x3 );
				//}
				//else
				//{
				//	EnqueueInterrupt_Read ( TempBuffer, 1, 0x3 );
				//}
				
				// I would imagine this aborts read command since it has to read TOC from the disk
				ReadCommand = -1;
				SetNextReadCycle ( 0 );
				
				
				// set the next command and busy cycles
				//Command = CDREG1_CMD_READTOC_2;
				//BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_READTOC ];
				//SetNextActionCycle ( BusyCycles );
				ReadCommand = CDREG1_CMD_READTOC_2;
				BusyCycles = CommandExecutionTimes_Secondary [ CDREG1_CMD_READTOC ];
				SetNextReadCycle ( BusyCycles );
				
				
				// command started
				Command = -1;
				//REG_Command = -1; ArgumentsIndex = 0;
				
#ifdef INLINE_DEBUG_RUN
	//DebugCount = 40;
#endif
				
				break;

				
			default:
				cout << "\nhps1x64 WARNING: Unknown CD command @ Cycle#" << dec << *_DebugCycleCount << " Command=" << hex << (u32)Command << " PC=" << *_DebugPC << "\n";
				break;
		}
		
		// if last command is complete, check if there is a pending command
		Check_Command ();
		
	}
	
}



void CD::Event_LidOpen ()
{
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nEVENT_LIDOPEN";
#endif

	u8 TempBuffer [ 16 ];
	
	if ( isLidOpen )
	{
#ifdef INLINE_DEBUG_RUN
	debug << "; LIDISOPEN";
#endif

		// status gets error bit set and lid is open
		Status |= ( CDREG1_READ_STAT_ERROR | CDREG1_READ_STAT_SHELLOPEN );

		// disk can't be spinning when it is open
		Status &= ~CDREG1_READ_STAT_STANDBY;
		
		//Status |= CDREG1_READ_STAT_STANDBY;
		TempBuffer [ 0 ] = Status;
		TempBuffer [ 1 ] = 0x80;
		
		// enqueue secondary interrupt
		EnqueueInterrupt_Read ( TempBuffer, 2, 0x5 );
		
		/*
		ReadCommand = CDREG1_CMD_LID_1;
		BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READS ] * 3;
		
		// set the next cycle for read
		SetNextReadCycle ( BusyCycles_Read );
		*/
		
#ifdef INLINE_DEBUG_RUN
	DebugCount = 80;
#endif
	}
}




int CD::Get_MaxChan ( u8 CodingInfo, int DoubleSpeed )
{
	return ( ( ( ( ( CodingInfo ^ 1 ) & 1 ) + 1 ) * 2 ) * ( ( ( ( ( CodingInfo ) >> 2 ) & 1 ) + 1 ) * 2 ) ) * ( ( DoubleSpeed & 1 ) + 1 );
}




void CD::Process_Read ()
{
	u8* pSectorDataBuffer;
	u8* pSubBuffer;
	u8 TempBuffer [ 8 ];
	
	u32 Disk_File, Disk_Chan, Disk_SubMode;
	
	// also need the max chan according to control/adr
	u32 Disk_CodingInfo;
	u32 Disk_Min, Disk_Sec, Disk_Frac;
	
	// no longer seeking
	//Status &= ~CDREG1_READ_STAT_SEEK;
	
	/*
	if ( !MODE.Report && !MODE.SF && !MODE.Size && MODE.AutoPause && ReadCount >= 16 )
	{
#ifdef INLINE_DEBUG_RUN
	debug << "; AutoPause";
#endif
		// ????
		Status &= ~( CDREG1_READ_STAT_READ );
		TempBuffer [ 0 ] = Status;
		EnqueueInterrupt ( TempBuffer, 1, 0x4 );
		ReadCommand = -1;
		return;
	}
	*/
	
	
	// *** each sector causes a type 1 IRQ ***
	// this command does not just read one sector. To stop the reading, uses the PAUSE or STANDBY command
	
						
	// read sector
	// *** should probably also read sector into buffer *** //
	pSectorDataBuffer = cd_image.ReadNextSector ();
	
	/*
	// always copy first 20 bytes for getlocl
	for ( int i; i < 20; i++ ) GetLocL_Data [ i ] = pSectorDataBuffer [ i ];
	*/
	
	// copy into sector data buffer
	// don't do this when buffering sectors
	//for ( int i = 0; i < c_SectorSize; i++ ) SectorDataBuffer [ i ] = pSectorDataBuffer [ i ];
	
	// copy data into data buffer
	// *note* the amount of data to copy is probably specified by mode variable
	if ( MODE.Size )
	{
		ReadMode = 2340;
		ReadMode_Offset = 12;
		//DataBuffer = SectorDataBuffer + 12;
	}
	else
	{
		ReadMode = 2048;
		ReadMode_Offset = 24;
		//DataBuffer = SectorDataBuffer + 24;
	}


	
	// copy into data buffer
	// don't do this when buffering sectors
	//for ( int i = 0; i < ReadMode; i++ ) DataBuffer [ i ] = SectorDataBuffer [ i + ReadMode_Offset ];
	
	// copy into sub buffer
	if ( cd_image.isSubOpen )
	{
		// get the pointer into the sub buffer here
		pSubBuffer = cd_image.GetCurrentSubBuffer ();
						
		/*
		// copy subchannel data
		for ( int i = 0; i < DiskImage::CDImage::c_SubChannelSizePerSector; i++ ) SubBuffer [ i ] = cd_image.CurrentSubBuffer [ i ];
		*/
	}

				
#ifdef INLINE_DEBUG_RUN
	debug << "; ReadMode=" << dec << ReadMode;
#endif

	// we should start reading from where it is writing to
	// don't do this here when buffering sectors
	//SectorDataIndex = 0;

	// sector data is now ready to be read and data is ready for dma
	isSectorDataReady = true;
	
	/*
	// update relative values too
	Inc_DiskPosition ( Min, Sec, Frac );
	
	// update to next sector for next read
	Inc_DiskPosition ( AMin, ASec, AFrac );
	
	// Before updating disk position, save values for GetLocP
	// *** TODO *** put in actual index and track values after reading cue sheet
	GetLocP_Index = 1;
	GetLocP_Track = CurrentTrack;
	GetLocP_Min = Min;
	GetLocP_Sec = Sec;
	GetLocP_Frac = Frac;
	GetLocP_AMin = AMin;
	GetLocP_ASec = ASec;
	GetLocP_AFrac = AFrac;
	*/
	
	// *** TESTING *** //
	// wait until read is complete
	//cd_image.WaitForSectorReadComplete ();
				

				
	Disk_File = ((DiskImage::CDImage::Sector::Mode2_2328*)pSectorDataBuffer)->File;
	Disk_Chan = ((DiskImage::CDImage::Sector::Mode2_2328*)pSectorDataBuffer)->Chan;
	Disk_SubMode = ((DiskImage::CDImage::Sector::Mode2_2328*)pSectorDataBuffer)->Submode;
	Disk_CodingInfo = ((DiskImage::CDImage::Sector::Mode2_2328*)pSectorDataBuffer)->Coding;
	
	Disk_Min = ((DiskImage::CDImage::Sector::Mode2_2328*)pSectorDataBuffer)->Minutes;
	Disk_Sec = ((DiskImage::CDImage::Sector::Mode2_2328*)pSectorDataBuffer)->Seconds;
	Disk_Frac = ((DiskImage::CDImage::Sector::Mode2_2328*)pSectorDataBuffer)->Sector;
	
	Disk_MaxChan = Get_MaxChan ( Disk_CodingInfo, MODE.Speed );
	
#ifdef INLINE_DEBUG_RUN
	debug << "isReadingFirstSector=" << isReadingFirstSector << "; On Disk: File=" << hex << Disk_File;
	debug << "; Chan=" << Disk_Chan;
	debug << "; Submode=" << Disk_SubMode;
	debug << "; Coding=" << Disk_CodingInfo;
	debug << "; Min=" << Disk_Min;
	debug << "; Sec=" << Disk_Sec;
	debug << "; Frac=" << Disk_Frac;
	debug << "; MaxChan=" << (u32) Disk_MaxChan;
	debug << "; NextChan=" << (u32) Disk_NextChan;
	debug << " SectorDataQ_Index=" << dec << SectorDataQ_Index;
	
	if ( cd_image.isSubOpen )
	{
	// also should take a peak at SUBQ
	debug << "; ControlAndADR=" << hex << (u32)((DiskImage::CDImage::Sector::SubQ*)pSubBuffer)->ControlAndADR;
	debug << "; Track=" << (u32)((DiskImage::CDImage::Sector::SubQ*)pSubBuffer)->TrackNumber;
	debug << "; Index=" << (u32)((DiskImage::CDImage::Sector::SubQ*)pSubBuffer)->IndexNumber;
	debug << "; SubP=" << ((u32*)pSubBuffer) [ 0 ] << " " << ((u32*)pSubBuffer) [ 1 ] << " " << ((u32*)pSubBuffer) [ 2 ];
	}
	
#endif

	
	// important note: must also be a data sector to return data!!!
	if ( isDataSector ( pSectorDataBuffer ) )
	{
	
		// check if this is the first audio sector being read
		if ( isReadingFirstAudioSector )
		{
			// check if filter was specified already - meaning what file/channel we are to play
			if ( ( !MODE.SF ) && ( Disk_SubMode & 0x4 ) )
			{
#ifdef INLINE_DEBUG_RUN
	debug << "; FirstSector; FilterNotSet";
#endif

				// grab file and chan to use
				Filter_File = Disk_File;
				Filter_Chan = Disk_Chan;
				
				// no longer reading first sector after this sector
				isReadingFirstAudioSector = 0;
			}
			
		}
				
		// no longer reading first sector after this sector
		isReadingFirstSector = 0;

		
#ifdef INLINE_DEBUG_RUN
	if ( MODE.Strsnd ) debug << "; STRSND";
	if ( MODE.SF ) debug << "; SF";
	//if ( hasEDC ) debug << "; EDC";
#endif
				
		// *** TESTING ***
		// it is data, just like readn
		// READN actually just keeps reading
		//Status |= ( CDREG1_READ_STAT_STANDBY | CDREG1_READ_STAT_READ );
		
			
		// check if sector is audio - if mode bit 6 is set then do not trigger int 1 for audio, just send to SPU
		//if ( ( ( Disk_SubMode & 0x7f ) == 0x64 ) && MODE.Strsnd )
		// note: MUST also make sure it is not set as data!!
		if ( ( Disk_SubMode & 0x4 ) && MODE.Strsnd && !( Disk_SubMode & 0x8 ) )
		{
#ifdef INLINE_DEBUG_RUN
	debug << "; AUDIO";
#endif
					
			// check if this is the correct file/channel, unless there is no filter set
			if ( ( ( Disk_File == Filter_File ) && ( Disk_Chan == Filter_Chan ) ) )
			{
#ifdef INLINE_DEBUG_RUN
	debug << "; STREAM";
#endif
				// *** testing ***
				//Status |= ( CDREG1_READ_STAT_READ | CDREG1_READ_STAT_STANDBY );
				

				//if ( Status & CDREG1_READ_STAT_READ )
				//{
					//Disk_MaxChan = Get_MaxChan ( Disk_CodingInfo, MODE.Speed );
					
					Process_XASector ( pSectorDataBuffer );
					
					
					// *** testing *** there must be something you need to do here??
					/*
					if ( ( Disk_SubMode & 0x80 ) && MODE.SF )
					{
						// next read should use next buffer
						//SectorDataQ_Index++;
						//SectorDataQ_Index &= c_iMaxQueuedSectors_Mask;
						
						// *** testing *** can only buffer two sectors
						// *** todo *** buffer only two sectors
						
						// *** testing ***
						Status &= ~( CDREG1_READ_STAT_READ );
						TempBuffer [ 0 ] = Status;
						EnqueueInterrupt ( TempBuffer, 1, 0x4 );
						ReadCommand = -1;
						
						// no longer streaming sound after this sector
						//MODE.Strsnd = 0;
						
						// does it stop reading??
						//break;
					}
					*/
				//}
			}
			
		}
		else
		{
#ifdef INLINE_DEBUG_RUN
	debug << "; DATA";
#endif

			// *** TODO *** if a command is already started then set interrupt as a pending read interrupt
			// *** testing *** only fire interrupt if there is no command in process already
			

			// must also check if set filter is cleared or set
			//if ( ( !MODE.SF || ( ( Disk_SubMode & 0x7f ) != 0x64 ) || ( Disk_File == Filter_File && Disk_Chan == Filter_Chan ) ) && ( hasEDC || MODE.CDDA ) )
			// note: possibly should also comment out or remove the "!MODE.SF" here - no, don't do this
			if ( ( !MODE.SF ) || ( !( Disk_SubMode & 0x4 ) ) || ( Disk_File == Filter_File && Disk_Chan == Filter_Chan ) )
			{
				//if ( Disk_Chan != Disk_SubMode || Disk_Chan != Disk_CodingInfo /*Disk_SubMode | Disk_Chan |*/ || !MODE.SF )
				//{
				
#ifdef INLINE_DEBUG_RUN
	debug << "; SEND";
	debug << "; IntQueue.Size=" << dec << IntQueue.Size ();
#endif

					
					// *** testing ***
					//Status |= ( CDREG1_READ_STAT_READ | CDREG1_READ_STAT_STANDBY );
					
					
						SectorDataQ_Buffer [ SectorDataQ_Index & c_iMaxQueuedSectors_Mask ] = cd_image.GetCurrentBufferIndex ();
						
						// mark entry in sector buffer as active
						// ONLY if it is NOT the current buffer
						//if ( ( SectorDataQ_Index & c_iMaxQueuedSectors_Mask ) != ( DataBuffer_SlotNumber & c_iMaxQueuedSectors_Mask ) )
						//{
							SectorDataQ_Active [ SectorDataQ_Index & c_iMaxQueuedSectors_Mask ] = 1;
						//}
						
						TempBuffer [ 0 ] = Status;
						EnqueueInterrupt_Read ( TempBuffer, 1, 0x1 );
						
					// *** testing ***
					// next read should use next buffer
					SectorDataQ_Index++;
					
					//}
					
						
					// increment the count of sectors read
					ReadCount++;
				//}
			}
			
		}
	}
	
	//Disk_NextChan++;
	//Disk_NextChan &= ( Disk_MaxChan - 1 );
}


void CD::Event_LidClose ()
{
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nEVENT_LIDCLOSE";
#endif

	u8 TempBuffer [ 16 ];
	
	if ( !isLidOpen )
	{
#ifdef INLINE_DEBUG_RUN
	debug << "; LIDISCLOSED";
#endif

		// status gets error bit set and lid is open
		Status |= ( CDREG1_READ_STAT_ERROR | CDREG1_READ_STAT_SHELLOPEN );

		// disk can't be spinning when it is open
		Status &= ~CDREG1_READ_STAT_STANDBY;
		
		//Status |= CDREG1_READ_STAT_STANDBY;
		/*
		TempBuffer [ 0 ] = Status;
		TempBuffer [ 1 ] = 0x80;
		
		// enqueue secondary interrupt
		EnqueueInterrupt_Read ( TempBuffer, 2, 0x5 );
		*/
		
		ReadCommand = CDREG1_CMD_LID_1;
		BusyCycles_Read = CommandExecutionTimes_Secondary [ CDREG1_CMD_READS ] * 3;
		
		// set the next cycle for read
		SetNextReadCycle ( BusyCycles_Read );
		
#ifdef INLINE_DEBUG_RUN
	DebugCount = 80;
#endif
	}
}





void CD::UpdateREG_ModeStatus ()
{
	// Mode/Status Register
	// bit 0-1 - Mode
	// bit 2 - Unknown (always zero) - CD XA Buffer Empty flag (0-empty; 1- NOT empty)
	// bit 3 - Parameter FIFO Empty (0: NOT Empty; 1: Empty) (triggered before writing first byte)
	// bit 4 - Parameter FIFO Full (0: Full; 1: NOT Full) (triggered after writing 16 bytes)
	// bit 5 - Response FIFO Empty (0: Empty; 1: NOT Empty) (triggered after reading last byte)
	// bit 6 - Data FIFO Empty (0: Empty; 1: NOT Empty) (triggered after reading last byte)
	// bit 7 - Command/Parameter transmission busy
	
	// check if CD-XA buffer is empty //
	if ( SpuBuffer_WriteIndex == SpuBuffer_ReadIndex )
	{
		// cd-xa buffr is empty //
		REG_ModeStatus &= ~( 1 << 2 );
		//REG_ModeStatus &= ~( 1 << 7 );
	}
	else
	{
		// cd-xa buffer is not empty //
		REG_ModeStatus |= ( 1 << 2 );
		//REG_ModeStatus |= ( 1 << 7 );
	}
	
	// keep cd-xa buffer as empty, since as long as it is being played it is empty it looks like
	//REG_ModeStatus &= ~( 1 << 2 );
	
	// check is Parameter FIFO empty //
	if ( ArgumentsIndex == 0 )
	{
		// parameter fifo is empty //
		REG_ModeStatus |= ( 1 << 3 );
	}
	else
	{
		// parameter fifo is NOT empty //
		REG_ModeStatus &= ~( 1 << 3 );
	}
	
	// check is Parameter FIFO full //
	if ( ArgumentsIndex >= 16 )
	{
		// parameter fifo is full //
		REG_ModeStatus &= ~( 1 << 4 );
	}
	else
	{
		// parameter fifo is NOT full //
		REG_ModeStatus |= ( 1 << 4 );
	}
	
	// check is Response FIFO Empty //
	/*
	if ( IntQueue.Peek () )
	{
		if ( ResponseBuf_Index >= IntQueue.Peek ()->ResponseSize )
		{
			// response fifo is empty //
			REG_ModeStatus &= ~( 1 << 5 );
		}
		else
		{
			// response fifo NOT empty //
			REG_ModeStatus |= ( 1 << 5 );
		}
	}
	else
	{
		// response fifo is empty //
		REG_ModeStatus &= ~( 1 << 5 );
	}
	*/
	
	// *** testing *** global response buffer
	if ( ResponseBuf_Index >= ResponseBuf_Size )
	{
		// response fifo is empty //
		REG_ModeStatus &= ~( 1 << 5 );
	}
	else
	{
		// response fifo NOT empty //
		REG_ModeStatus |= ( 1 << 5 );
	}
	
	// check is data fifo empty //
	if ( DataBuffer_Index >= DataBuffer_Size || DataBuffer_Size == 0 )
	{
		// data fifo is empty //
		REG_ModeStatus &= ~( 1 << 6 );
	}
	else
	{
		// data fifo is NOT empty //
		REG_ModeStatus |= ( 1 << 6 );
	}
	
	// check if command has not started processing yet
	if ( REG_Command != -1 )
	{
		// command is waiting to start processing //
		REG_ModeStatus |= ( 1 << 7 );
	}
	else
	{
		// there is no command waiting //
		REG_ModeStatus &= ~( 1 << 7 );
	}
	
	// bit 2 is always zero
	// *not always*
	//REG_ModeStatus &= ~( 1 << 2 );
}



void CD::Check_Interrupt ()
{
	u8 TempBuffer [ 8 ];
	
	// check for pending interrupt first //
	
	if ( PendingInt.InterruptEnabled )
	{
		// there is a pending secondary interrupt //
		
		// make sure there is no pending primary interrupt and no active primary interrupt
		if ( !CurrentInt.InterruptPending && !CurrentInt.InterruptEnabled )
		{
			// set as current interrupt //
			CurrentInt = PendingInt;
			
			// check if it is a data interrupt AND there is a READS/READN command active //
			
			if ( PendingInt.InterruptReason == 1 && ( ReadCommand == CDREG1_CMD_READN_2 || ReadCommand == CDREG1_CMD_READS_2 ) )
			{
				// clear pending interrupt if there are no more data buffers ready
				if ( !SectorDataQ_Active [ ( DataBuffer_SlotNumber + 1 ) & c_iMaxQueuedSectors_Mask ] )
				{
					// disable pending interrupt //
					PendingInt.InterruptEnabled = false;
				}
			}
			else
			{
				// disable pending interrupt
				PendingInt.InterruptEnabled = false;
			}
		}
	}
	
	// check for current interrupt //
	
	if ( CurrentInt.InterruptEnabled )
	{
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; CHECKING_INT " << hex << REG_InterruptFlag << " " << REG_InterruptEnable;
}
#endif

		// if the interrupt is in the slot, then set the associated stuff //
		
		if ( !CurrentInt.ResponseSent )
		{
			// set the interrupt flag //
			REG_InterruptFlag |= CurrentInt.InterruptReason;
			UpdateREG_InterruptFlag ();
			
			// send the response and reset the current response buffer index //
			ResponseBuf_Index = 0;
			ResponseBuf_Size = CurrentInt.ResponseSize;
			for ( int i = 0; i < ResponseBuf_Size; i++ ) Current_ResponseBuf [ i ] = CurrentInt.ResponseBuf [ i ];
			
			// the interrupt response has been sent now
			CurrentInt.ResponseSent = true;
		}
		
		
		// check if it is a data interrupt where the data has not been sent yet //
		if ( CurrentInt.InterruptReason == 1 && !CurrentInt.DataSent )
		{
			// set the current data buffer index to use for data reads, and its size //
			LastDataBufferSize = CurrentInt.SectorDataQ_Size;
			LastDataBufferIndex = DataBuffer_SlotNumber;	//CurrentInt.SectorDataQ_Index;
			
			// data is sent, so buffer is no longer waiting for transfer
			SectorDataQ_Active [ DataBuffer_SlotNumber & c_iMaxQueuedSectors_Mask ] = 0;
			
			// update data buffer index
			DataBuffer_SlotNumber++;
			
			// data has now been sent
			CurrentInt.DataSent = true;
		}

		
		// check if interrupt should trigger from CD device side
		if ( REG_InterruptFlag & REG_InterruptEnable & 0x1f )
		{
			if ( !CurrentInt.InterruptSent )
			{
			
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; INT";
}
#endif

				// trigger the interrupt only once
				SetInterrupt ();
				
				// interrupt has been sent
				CurrentInt.InterruptSent = true;
			}
		}
		else
		{
			// check if interrupt was acknowledged
			// interrupt was acknowledged
			CurrentInt.InterruptEnabled = false;
		}
	}
}


void CD::UpdateREG_InterruptFlag ( u32 IntOk )
{
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; UpdateREG_InterruptFlag";
}
#endif

	/*
	if ( CurrentInt.InterruptEnabled )
	{
		REG_InterruptFlag = ( REG_InterruptFlag & 0xe0 ) | ( CurrentInt.InterruptReason & 0x1f );
	}
	else
	{
		REG_InterruptFlag = ( REG_InterruptFlag & 0xe0 ) | ( 0 & 0xf );
	}
	*/

	/*
	//REG_InterruptFlag = ( REG_InterruptFlag & 0xf0 ) | ( InterruptQ [ InterruptQ_Index ].InterruptReason & 0xf );
	if ( IntQueue.Peek() )
	{

		REG_InterruptFlag = ( REG_InterruptFlag & 0xe0 ) | ( IntQueue.Peek ()->InterruptReason & 0x1f );
	}
	else
	{

		REG_InterruptFlag = ( REG_InterruptFlag & 0xe0 ) | ( 0 & 0xf );
	}
	*/
	
	// bit 3 usually zero
	// needs more testing here
	//REG_InterruptFlag &= ~( 1 << 3 );

	// bits 5-7 always 1's
	REG_InterruptFlag |= 0xe0;
	
	/*
	//if ( InterruptQ [ InterruptQ_Index ].InterruptEnabled && ( REG_InterruptFlag & 0x1f ) )
	if ( !IntQueue.IsEmpty() && ( REG_InterruptFlag & 0x1f ) )
	{
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; CHECKING_INT " << hex << REG_InterruptFlag << " " << REG_InterruptEnable;
}
#endif

		if ( REG_InterruptFlag & REG_InterruptEnable & 0x1f )
		{
		
			if ( IntOk )
			{
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; INT";
}
#endif

				SetInterrupt ();
				
				// if there is a data ready interrupt getting acknowledged, set the next data buffer and size
				if ( _CD->IntQueue.Peek() )
				{
					if ( _CD->IntQueue.Peek()->InterruptReason == 1 )
					{
						_CD->LastDataBufferSize = _CD->IntQueue.Peek()->SectorDataQ_Size;
						_CD->LastDataBufferIndex = _CD->IntQueue.Peek()->SectorDataQ_Index;
					}
				}
			
				// must reset the index for the response buffer
				// *** testing *** global response buffer
				//ResponseBuf_Index = 0;
				
				// *** TODO *** copy in the data for the response buffer
				ResponseBuf_Index = 0;
				ResponseBuf_Size = IntQueue.Peek()->ResponseSize;
				for ( int i = 0; i < ResponseBuf_Size; i++ ) Current_ResponseBuf [ i ] = IntQueue.Peek()->ResponseBuf [ i ];
			}
		}
	}
	else
	{
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; ELSE";
}
#endif

		// disable the current interrrupt
		//InterruptQ [ InterruptQ_Index ].InterruptEnabled = 0;
		
		if ( IntQueue.IsEmpty () )
		{

			return;
		}
		
		if ( !IntQueue.Peek()->InterruptEnabled )
		{
			return;
		}
		
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; REMOVE";
}
#endif

		// remove the previous interrupt
		IntQueue.Remove ();
		
		// check if there are any remaining interrupts
		if ( !IntQueue.IsEmpty () )
		{
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; INT_Remaining";
}
#endif

			// there is another pending non-read interrupt //
			
			// set the interrupt flag
			REG_InterruptFlag = ( REG_InterruptFlag & 0xf0 ) | ( IntQueue.Peek ()->InterruptReason & 0xf );
			
			// trigger interrupt
			SetInterrupt ();
			
			// don't do this after acknowledging an interrupt?
			// if there is a data ready interrupt getting acknowledged, set the next data buffer and size
			//if ( _CD->IntQueue.Peek() )
			//{
			//	if ( _CD->IntQueue.Peek()->InterruptReason == 1 )
			//	{
			//		_CD->LastDataBufferSize = _CD->IntQueue.Peek()->SectorDataQ_Size;
			//		_CD->LastDataBufferIndex = _CD->IntQueue.Peek()->SectorDataQ_Index;
			//	}
			//}
			
			// must reset the index for the response buffer
			ResponseBuf_Index = 0;
			
			// *** TODO *** copy in the data for the response buffer
			ResponseBuf_Index = 0;
			ResponseBuf_Size = IntQueue.Peek()->ResponseSize;
			for ( int i = 0; i < ResponseBuf_Size; i++ ) Current_ResponseBuf [ i ] = IntQueue.Peek()->ResponseBuf [ i ];
		}
		else
		{
			// there are no pending non-read interrupts //
			
			// check if there are any read interrupts
			if ( !IntQueue_Read.IsEmpty () )
			{
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; INT_PendingRead";
}
#endif

				// there is a pending read interrupt //
				
				// add interrupt into the main interrupt queue
				IntQueue.Add ( *IntQueue_Read.Peek () );
				
				IntQueue_Read.Remove ();
				
				// set the interrupt flag
				REG_InterruptFlag = ( REG_InterruptFlag & 0xf0 ) | ( IntQueue.Peek ()->InterruptReason & 0xf );
				
				// trigger interrupt
				SetInterrupt ();
				
				// if there is a data ready interrupt getting acknowledged, set the next data buffer and size
				if ( _CD->IntQueue.Peek() )
				{
					if ( _CD->IntQueue.Peek()->InterruptReason == 1 )
					{
						_CD->LastDataBufferSize = _CD->IntQueue.Peek()->SectorDataQ_Size;
						_CD->LastDataBufferIndex = _CD->IntQueue.Peek()->SectorDataQ_Index;
					}
				}
				
				// must reset the index for the response buffer
				ResponseBuf_Index = 0;
			}
		}
		
	}
	*/
}


void CD::Check_Command ()
{
#ifdef INLINE_DEBUG_RUN
	debug << "; Checking Command";
	debug << "; Command=" << hex << (u16)Command << " REG_Command=" << (u16)REG_Command;
#endif

	// if there is no command currently executing, then send command
	//if ( Command == -1 && REG_Command != -1 && IntQueue.IsEmpty() )
	if ( Command == -1 && REG_Command != -1 && !CurrentInt.InterruptEnabled )
	{
		SendCommand ( REG_Command );
		
		// command has started
		Command = REG_Command;
		
		// move arguments into parameter buffer
		Load_ParameterBuf ( Arguments );
		
		// set size of parameter buffer
		ParameterBuf_Size = ArgumentsIndex;
		
		// clear the pending command as there is none pending
		// don't do this because command has not started yet
		//_CD->REG_Command = -1;
	}
}


u32 CD::Read ( u32 Address )
{
	u32 Data;
	u8* pSectorDataBuffer;

#ifdef INLINE_DEBUG_READ
	debug << "\r\nCD::Read; PC = " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif

#ifdef INLINE_DEBUG_RUN
	_CD->DebugCount--;
	if ( _CD->DebugCount > 0 )
	{
		debug << "\r\nCD::Read; PC = " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
	}
#endif

	switch ( Address )
	{
		case CD_REG0:
		// Mode/Status Register (R)
		// bit 0-1 - Mode
		// bit 2 - Unknown (always zero) - CD-XA Buffer Empty flag
		// bit 3 - Parameter FIFO Empty (0: NOT Empty; 1: Empty) (triggered before writing first byte)
		// bit 4 - Parameter FIFO Full (0: Full; 1: NOT Full) (triggered after writing 16 bytes)
		// bit 5 - Response FIFO Empty (0: Empty; 1: NOT Empty) (triggered after reading last byte)
		// bit 6 - Data FIFO Empty (0: Empty; 1: NOT Empty) (triggered after reading last byte)
		// bit 7 - Command/Parameter transmission busy
		
			// incoming read from CDREG0
			// it looks like the 0x18 must be or'ed with the value
			_CD->UpdateREG_ModeStatus ();
			
#ifdef INLINE_DEBUG_READ
	debug << "; CDREG0;Out=" << hex << (u32) _CD->REG_ModeStatus << "; REG_ModeStatus" << dec << " DataBuffer_Index=" << _CD->DataBuffer_Index << " DataBuffer_Size=" << _CD->DataBuffer_Size;
#endif

#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; CDREG0;Out=" << hex << (u32) _CD->REG_ModeStatus << "; REG_ModeStatus" << dec << " DataBuffer_Index=" << _CD->DataBuffer_Index << " DataBuffer_Size=" << _CD->DataBuffer_Size;
}
#endif

			return _CD->REG_ModeStatus;
			
			break;
			
		case CD_REG1:
			// Mode 0-3 (All Modes) (R): Response FIFO //
			// buffer is 16-bytes
			// after reading past 16-bytes, it wraps around and reads first byte again
			
			// *** TODO *** detect 16-bit reads ***
		
#ifdef INLINE_DEBUG_READ
	debug << "; CDREG1; ResponseBuf_Index=" << dec << _CD->ResponseBuf_Index;
#endif

			//if ( _CD->IntQueue.Peek() )
			if ( _CD->ResponseBuf_Index < _CD->ResponseBuf_Size )
			{
				//_CD->CD_REG1_Reg = _CD->IntQueue.Peek()->ResponseBuf [ _CD->ResponseBuf_Index & c_ResponseBuf_Mask ];
				_CD->CD_REG1_Reg = _CD->Current_ResponseBuf [ _CD->ResponseBuf_Index & c_ResponseBuf_Mask ];
			}
			else
			{
				// for now just say the disk is spinning
				// *** todo *** fix response fifo issues after interrupt acknowledge
				//_CD->CD_REG1_Reg = 0x2;
				_CD->CD_REG1_Reg = 0;
			}
			
			_CD->ResponseBuf_Index++;


#ifdef INLINE_DEBUG_READ
	debug << ";Out=" << hex << (u32) _CD->CD_REG1_Reg << "; Response FIFO";
#endif

#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; CDREG1" << ";Out=" << hex << (u32) _CD->CD_REG1_Reg << "; Response FIFO";
}
#endif

			return (u32) _CD->CD_REG1_Reg;
			
			break;

		case CD_REG2:

			// Mode 0-3 (All Modes) (R): Data FIFO //
			// Reads from CDREG2 is like a DATA register where it reads from disk, just like with dma except it reads a byte at a time
			// ***TODO*** can also read 2 bytes at a time
			
			// *** testing *** use SectorDataBuffer for now
			//_CD->CD_REG2_Reg = _CD->DataBuffer [ _CD->DataBuffer_Index ];
			//_CD->CD_REG2_Reg = _CD->SectorDataBuffer [ _CD->ReadMode_Offset + _CD->DataBuffer_Index ];
			
			// *** testing *** get pointer to buffer from index
			pSectorDataBuffer = _CD->cd_image.GetDataBuffer ( _CD->SectorDataBuffer_Index );
			_CD->CD_REG2_Reg = pSectorDataBuffer [ _CD->ReadMode_Offset + _CD->DataBuffer_Index ];
			
			_CD->DataBuffer_Index += 1;

			
#ifdef INLINE_DEBUG_READ
	debug << "; CDREG2;Out=" << (u32) _CD->CD_REG2_Reg << "; Data FIFO";
#endif

#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; CDREG2;Out=" << (u32) _CD->CD_REG2_Reg << "; Data FIFO";
}
#endif
			
			// incoming read from CDREG2
			return (u32) _CD->CD_REG2_Reg;
			
			break;

		case CD_REG3:
#ifdef INLINE_DEBUG_READ
	debug << "; CDREG3";
#endif
			// incoming read from CDREG3
			
			// determine port mode (R)
			switch ( _CD->REG_ModeStatus & 3 )
			{
				case 0:
				case 2:
					// Interrupt Enable Register (R) //
					// bits 0-4: Interrupt enable bits (usually all set; Write 0x1f: Enable all irqs)
					// bits 5-7: Unknown/unused - When reading all these bits are 1's
					
					//_CD->CD_REG3_Reg = 0xff;
					
					// bits 5-7 are always 1's when reading
					_CD->REG_InterruptEnable |= 0xe0;
					
					_CD->CD_REG3_Reg = _CD->REG_InterruptEnable;
					
#ifdef INLINE_DEBUG_READ
	debug << "; CDREG3;Out=" << (u32) _CD->CD_REG3_Reg << "; REG_InterruptEnable";
#endif
					
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; CDREG3" << ";Out=" << (u32) _CD->CD_REG3_Reg << "; REG_InterruptEnable";
}
#endif

					break;
					
				case 1:
				case 3:
				
					// Interrupt Flag Register (R) //
					// bits 0-2: Read: Response Received
					// bit 3 - Unknown (usually zero)
					// bit 4: Read: Command Start
					// bits 5-7: always 1's

					//_CD->InterruptReason = _CD->QueuedBufferInterrupt [ 0 ];
					//_CD->CD_REG3_Reg = _CD->InterruptReason | 0xe0;
					
					_CD->UpdateREG_InterruptFlag ();
					_CD->CD_REG3_Reg = _CD->REG_InterruptFlag;
					
#ifdef INLINE_DEBUG_READ
	debug << "; CDREG3;Out=" << (u32) _CD->CD_REG3_Reg << "; REG_InterruptFlag";
#endif
					
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
	debug << "; CDREG3" << ";Out=" << hex <<(u32) _CD->CD_REG3_Reg << "; REG_InterruptFlag";
	debug << "; FIFOSize=" << _CD->IntQueue.Size ();
}
#endif

					break;
			}

			return (u32) _CD->CD_REG3_Reg;
			
			break;
			
			
		default:
#ifdef INLINE_DEBUG_READ
			debug << "; Invalid";
#endif
		
			// invalid CD Register
			cout << "\nhps1x64 ALERT: Unknown CD READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
			break;
	};
	
	return 0;
}


void CD::Write ( u32 Address, u32 Data, u32 Mask )
{

	bool isDataReadyInterrupt;
	u8* pSectorDataBuffer;

#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nCD::Write; PC = " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif

#ifdef INLINE_DEBUG_RUN
_CD->DebugCount--;
if ( _CD->DebugCount > 0 )
{
	debug << "\r\nCD::Write; PC = " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
}
#endif

	// *** testing *** check if mask is a word write
	if ( Mask != 0xff )
	{
		cout << "\nhps1x64 ALERT: CD::Write Mask=" << hex << Mask;
	}


	switch ( Address )
	{
		case CD_REG0:
		
			//////////////////////////////
			// *** CD REG 0 (Write) *** //
			//////////////////////////////
		
#ifdef INLINE_DEBUG_WRITE
	debug << "; CDREG0; REG_ModeStatus";
#endif

#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; REG_ModeStatus";
}
#endif

			// incoming write to CDREG0
			// Mode/Status Register
			// bits 0-1 are r/w, bits 2-7 are read-only
			//_CD->CD_REG0_Reg = ( _CD->CD_REG0_Reg & ~3 ) | (u8) Data;
			//_CD->CD_REG0_Reg = ( _CD->CD_REG0_Reg & ~3 ) | ( Data & 3 );
			_CD->REG_ModeStatus = ( _CD->REG_ModeStatus & ~3 ) | ( Data & 3 );
			
			break;

			
		case CD_REG1:
		
			//////////////////////////////
			// *** CD REG 1 (Write) *** //
			//////////////////////////////
		
#ifdef INLINE_DEBUG_WRITE
	debug << "; CDREG1";
#endif

			// determine port mode (W)
			switch ( _CD->REG_ModeStatus & 3 )
			{
				case 0:
					// sending command //
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; REG_Command; Command=" << hex << _CD->Command << " REG_Command=" << _CD->REG_Command;
}

					debug << "\r\nWriting command: " << CommandList [ Data & 0x1f ];
#endif
#ifdef INLINE_DEBUG_WRITE
					debug << "; REG_Command; Command=" << hex << _CD->Command << " REG_Command=" << _CD->REG_Command;
					debug << "\r\nWriting command: " << CommandList [ Data & 0x1f ];
#endif

					// check if another command is already processing
					_CD->UpdateREG_ModeStatus ();
					if ( ! ( _CD->REG_ModeStatus & 0x80 ) )
					{
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; FREE";
}
#endif

						// no commands are being processed //
						
						// set the command as pending
						_CD->REG_Command = Data;
						
						// reserve a spot in interrupt queue
						// note: this should be done when sending command
						//InterruptQ_Entry ie;
						//ie.Clear ();
						//_CD->IntQueue.Add ( ie );
						
						// check if ready to send command
						_CD->Check_Command ();
					}
					else
					{
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; INPROGRESS";
}
#endif

						// overwrite the current command
						_CD->REG_Command = Data;
						
						/*
						// don't clear the current command in progress
						_CD->Command = -1;
						
						if ( _CD->IntQueue.IsEmpty () )
						{
							// the secondary command must have been in processing //
							
							InterruptQ_Entry ie;
							
							// reserve a spot in interrupt queue
							ie.Clear ();
							_CD->IntQueue.Add ( ie );
						}
						
						_CD->Check_Command ();
						*/
					}
					
					break;

				case 1:
				case 2:
					// unknown/unused or mirror?? //
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; Port 1 or 2: Unknown/Unused?? Mirror??";
}
#endif
#ifdef INLINE_DEBUG_WRITE
					debug << "; Port 1 or 2: Unknown/Unused?? Mirror??";
#endif

					cout << "\nhps1x64 WARNING: CDREG1 Port 1/2: Unknown/Unused?? Mirror??\n";

					break;
				
				case 3:
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; PendingVolumeCDRightToSPURight";
}
#endif
#ifdef INLINE_DEBUG_WRITE
					debug << "; PendingVolumeCDRightToSPURight";
#endif

					// audio volume for right cd-out to right spu input //
					_CD->PendingVolume_CDRightToSPURight = Data;

					break;
			}

			break;


		case CD_REG2:
		
			//////////////////////////////
			// *** CD REG 2 (Write) *** //
			//////////////////////////////
		
#ifdef INLINE_DEBUG_WRITE
	debug << "; CDREG2";
#endif

			switch ( _CD->REG_ModeStatus & 3 )
			{
				case 0:
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; Parameter FIFO";
}
#endif
#ifdef INLINE_DEBUG_WRITE
					debug << "; Parameter FIFO";
#endif

					// Parameter FIFO //
					// before sending any commands, write parameters to here
					
					if ( _CD->ArgumentsIndex < c_ArgumentBuf_Size )
					{
						_CD->Arguments [ _CD->ArgumentsIndex++ ] = Data;
					}
					
					break;
					
				case 1:
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; REG_InterruptEnable";
}
#endif
#ifdef INLINE_DEBUG_WRITE
					debug << "; REG_InterruptEnable";
#endif

					// Interrupt Enable Register (W) //
					// bits 0-4 interrupt enable bits (writing 0x1f enables all irqs)
					// bits 5-7 Unknown/unused (should be 0 when writing, but always 1 when reading)
					
					//if ( Data == 0x18 )
					//{
					//	// *** todo *** needs more testing
					//	Data = 0xff;
					//}
					
					_CD->REG_InterruptEnable = Data;
					
					// bits 5-7 - always set when reading
					_CD->REG_InterruptEnable |= 0xe0;
					
					break;
					
				case 2:
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; PendingVolumeCDLeftToSPULeft";
}
#endif
#ifdef INLINE_DEBUG_WRITE
					debug << "; PendingVolumeCDLeftToSPULeft";
#endif

					// Audio Volume for Left CD-out to Left SPU input //
					_CD->PendingVolume_CDLeftToSPULeft = Data;
					break;
					
				case 3:
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; PendingVolumeCDRightToSPULeft";
}
#endif
#ifdef INLINE_DEBUG_WRITE
					debug << "; PendingVolumeCDRightToSPULeft";
#endif

					// Audio Volume for Right CD-out to Left SPU input //
					_CD->PendingVolume_CDRightToSPULeft = Data;
					break;
			}
			
			break;
			

		case CD_REG3:
		
			//////////////////////////////
			// *** CD REG 3 (Write) *** //
			//////////////////////////////
		
#ifdef INLINE_DEBUG_WRITE
	debug << "; CDREG3";
#endif

			switch ( _CD->REG_ModeStatus & 3 )
			{
				case 0:
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; REG_RequestRegister";
}
#endif
#ifdef INLINE_DEBUG_WRITE
					debug << "; REG_RequestRegister";
#endif

					// Request Register (W) //
					// bits 0-4 - not used (should be zero)
					// bit 5 - want command start interrupt on next command (0: no change; 1: yes)
					// bit 6 - not used (should be zero)
					// bit 7 - Wants data (0: No/Reset Data FIFO; 1: Yes/Load Data FIFO)
					
					if ( Data & 0x20 )
					{
						// wants command start interrupt on the next command //
						_CD->isCommandStartInterruptRequested = true;
					}
					
					if ( Data & 0x80 )
					{
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; ReadMode=" << dec << _CD->ReadMode << " DataBuffer_Index=" << _CD->DataBuffer_Index;
}
#endif
#ifdef INLINE_DEBUG_WRITE
					debug << "; ReadMode=" << dec << _CD->ReadMode << " DataBuffer_Index=" << _CD->DataBuffer_Index;
#endif

						// this means to get the dma transfer ready //
						// can do this any number of times when reading
						
						
						// data is ready in buffer
						/*
						_CD->DataBuffer_Size = _CD->ReadMode;
						
						if ( _CD->ReadMode == 2048 )
						{
							_CD->DataBuffer_Size = 2048;
						}
						else
						{
							_CD->DataBuffer_Size = 2340;
							//_CD->DataBuffer_Size = 2060;
						}
						*/
						
						_CD->DataBuffer_Size = _CD->LastDataBufferSize;
						_CD->SectorDataBuffer_Index = _CD->SectorDataQ_Buffer [ _CD->LastDataBufferIndex & c_iMaxQueuedSectors_Mask ];
						
						// *** testing *** can only buffer up to two sectors //
						
						// for now just handle current sector
						// *** todo *** buffer up to two sectors
						
						
						// update data buffer slot number
						/*
						if ( !_CD->SectorDataQ_Active [ _CD->DataBuffer_SlotNumber & c_iMaxQueuedSectors_Mask ] )
						{
							if ( _CD->SectorDataQ_Active [ ( _CD->DataBuffer_SlotNumber + 1 ) & c_iMaxQueuedSectors_Mask ] ) _CD->DataBuffer_SlotNumber++;
						}
						
						//if ( _CD->DataBuffer_SlotNumber > _CD->SectorDataQ_Index ) _CD->DataBuffer_SlotNumber = _CD->SectorDataQ_Index;
						
						// for now just handle current sector
						// *** todo *** buffer up to two sectors
						// *** tessting *** did not request any data here
						//pSectorDataBuffer = (u8*) (& (_CD->SectorDataQ_Buffer [ 0 ]));	// DiskImage::CDImage::c_SectorSize * _CD->IntQueue.Peek()->SectorDataQ_Index ]));
						//pSectorDataBuffer = (u8*) (& (_CD->SectorDataQ_Buffer [ DiskImage::CDImage::c_SectorSize * ( _CD->DataBuffer_SlotNumber & c_iMaxQueuedSectors_Mask ) ]));
						
						_CD->SectorDataBuffer_Index = _CD->SectorDataQ_Buffer [ _CD->DataBuffer_SlotNumber & c_iMaxQueuedSectors_Mask ];
						*/
					
						// copy sector buffer
						//for ( int i = 0; i < c_SectorSize; i++ ) _CD->SectorDataBuffer [ i ] = pSectorDataBuffer [ i ];
						
						// copy data buffer
						//for ( int i = 0; i < _CD->ReadMode; i++ ) _CD->DataBuffer [ i ] = pSectorDataBuffer [ i + _CD->ReadMode_Offset ];
						
						/*
						pSectorDataBuffer = (u8*) (& (_CD->SectorDataQ_Buffer [ 0 ])); //DiskImage::CDImage::c_SectorSize * _CD->IntQueue.Peek()->SectorDataQ_Index ]));
					
						// copy sector buffer
						for ( int i = 0; i < c_SectorSize; i++ ) _CD->SectorDataBuffer [ i ] = pSectorDataBuffer [ i ];
						
						// copy data buffer
						for ( int i = 0; i < _CD->ReadMode; i++ ) _CD->DataBuffer [ i ] = pSectorDataBuffer [ i + _CD->ReadMode_Offset ];
						*/
							
						/*
						//pSectorDataBuffer = (u8*) (& (_CD->SectorDataQ_Buffer [ DiskImage::CDImage::c_SectorSize * _CD->InterruptQ [ _CD->InterruptQ_Index & c_InterruptQ_Mask ].SectorDataQ_Index ]));
						if ( _CD->IntQueue.Peek () )
						{
							pSectorDataBuffer = (u8*) (& (_CD->SectorDataQ_Buffer [ DiskImage::CDImage::c_SectorSize * _CD->IntQueue.Peek()->SectorDataQ_Index ]));
						
							// copy sector buffer
							for ( int i = 0; i < c_SectorSize; i++ ) _CD->SectorDataBuffer [ i ] = pSectorDataBuffer [ i ];
							
							// copy data buffer
							for ( int i = 0; i < _CD->ReadMode; i++ ) _CD->DataBuffer [ i ] = pSectorDataBuffer [ i + _CD->ReadMode_Offset ];
						}
						*/
						
#ifdef INLINE_DEBUG_RUN
	pSectorDataBuffer = _CD->cd_image.GetDataBuffer ( _CD->SectorDataBuffer_Index );
	debug << "\r\nLoadFIFO; ReadMode=" << dec << _CD->ReadMode << "; ReadMode_Offset=" << _CD->ReadMode_Offset << "; Disk: Mode=" << hex << ((u32)((DiskImage::CDImage::Sector::Mode1*) pSectorDataBuffer)->Mode);
	debug << " Min=" << ((u32)((DiskImage::CDImage::Sector::Mode1*) pSectorDataBuffer)->Minutes);
	debug << " Sec=" << ((u32)((DiskImage::CDImage::Sector::Mode1*) pSectorDataBuffer)->Seconds);
	debug << " Frac=" << ((u32)((DiskImage::CDImage::Sector::Mode1*) pSectorDataBuffer)->Sector);
	debug << " SectorDataBuffer_Index=" << dec << _CD->SectorDataBuffer_Index << " LastDataBufferIndex=" << _CD->LastDataBufferIndex << " DataBuffer_Size=" << _CD->DataBuffer_Size << " ReadCount=" << _CD->ReadCount;
#endif
					}
					
					if ( ! ( Data & 0x80 ) )
					{
						// *** TODO *** reset data FIFO
						// looks like this means it wants to move to the next data buffer, then writes 0x80 in mode0 before reading from buffer
						
						// reset data FIFO
						_CD->DataBuffer_Index = 0;
						
						// mark entry in sector buffer as not active
						//_CD->SectorDataQ_Active [ _CD->DataBuffer_SlotNumber & c_iMaxQueuedSectors_Mask ] = 0;
						
						// did not request data, so data buffer is empty
						_CD->DataBuffer_Size = 0;
						
						// *** testing *** can only buffer up to two sectors //
						
						/*
						// update data buffer slot number
						_CD->DataBuffer_SlotNumber++;
						
						if ( _CD->DataBuffer_SlotNumber > _CD->SectorDataQ_Index ) _CD->DataBuffer_SlotNumber = _CD->SectorDataQ_Index;
						
						// for now just handle current sector
						// *** todo *** buffer up to two sectors
						// *** tessting *** did not request any data here
						//pSectorDataBuffer = (u8*) (& (_CD->SectorDataQ_Buffer [ 0 ]));	// DiskImage::CDImage::c_SectorSize * _CD->IntQueue.Peek()->SectorDataQ_Index ]));
						pSectorDataBuffer = (u8*) (& (_CD->SectorDataQ_Buffer [ DiskImage::CDImage::c_SectorSize * ( _CD->DataBuffer_SlotNumber & c_iMaxQueuedSectors_Mask ) ]));
					
						// copy sector buffer
						for ( int i = 0; i < c_SectorSize; i++ ) _CD->SectorDataBuffer [ i ] = pSectorDataBuffer [ i ];
						
						// copy data buffer
						for ( int i = 0; i < _CD->ReadMode; i++ ) _CD->DataBuffer [ i ] = pSectorDataBuffer [ i + _CD->ReadMode_Offset ];
						*/
						
						
						/*
						if ( _CD->IntQueue.Peek() )
						{
							//pSectorDataBuffer = (u8*) (& (_CD->SectorDataQ_Buffer [ DiskImage::CDImage::c_SectorSize * _CD->InterruptQ [ _CD->InterruptQ_Index & c_InterruptQ_Mask ].SectorDataQ_Index ]));
							pSectorDataBuffer = (u8*) (& (_CD->SectorDataQ_Buffer [ DiskImage::CDImage::c_SectorSize * _CD->IntQueue.Peek()->SectorDataQ_Index ]));
							
							// copy sector buffer
							for ( int i = 0; i < c_SectorSize; i++ ) _CD->SectorDataBuffer [ i ] = pSectorDataBuffer [ i ];
							
							// copy data buffer
							for ( int i = 0; i < _CD->ReadMode; i++ ) _CD->DataBuffer [ i ] = pSectorDataBuffer [ i + _CD->ReadMode_Offset ];
						}
						*/
						
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nResetFIFO; ReadMode=" << dec << _CD->ReadMode; //<< "; Disk: Mode=" << hex << ((u32)((DiskImage::CDImage::Sector::Mode1*) _CD->SectorDataBuffer)->Mode);
	//debug << " Min=" << ((u32)((DiskImage::CDImage::Sector::Mode1*) _CD->SectorDataBuffer)->Minutes);
	//debug << " Sec=" << ((u32)((DiskImage::CDImage::Sector::Mode1*) _CD->SectorDataBuffer)->Seconds);
	//debug << " Frac=" << ((u32)((DiskImage::CDImage::Sector::Mode1*) _CD->SectorDataBuffer)->Sector);
	debug << " SectorDataBuffer_Index=" << dec << _CD->SectorDataBuffer_Index << " LastDataBufferIndex=" << _CD->LastDataBufferIndex;
#endif

					}
					
					break;
					
				case 1:
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; REG_InterruptFlag";
}
#endif

#ifdef INLINE_DEBUG_WRITE
					debug << "; REG_InterruptFlag";
#endif

					// Interrupt Flag Register (W) //
					// bits 0-2 - Response Received - Write 7: Acknowledge
					// bit 3 - Unknown - Write 1: Acknowledge
					// bit 4 - Command Start - Write 1: Acknowledge
					// bit 5 - Unknown
					// bit 6 - Reset Parameter FIFO - Write 1: Resets parameter FIFO
					// bit 7 - Unknown
					
					if ( Data & 0x40 )
					{
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; RESET_PARAMETER_FIFO";
}
#endif

						// bit 6 is set, so reset parameter FIFO
						_CD->ArgumentsIndex = 0;
					}
					
					// check for data interrupt
					/*
					isDataReadyInterrupt = false;
					
					if ( ( _CD->REG_InterruptFlag & 0x7 ) == 0x1 )
					{
						isDataReadyInterrupt = true;
					}
					*/
					
					// acknowledge interrupts
					// need to clear bits in both places
					// interrupt does not need to be enabled for it to be cleared
					_CD->REG_InterruptFlag &= ~( Data & 0x1f );
					
#ifdef INLINE_DEBUG_WRITE
					debug << "; Clearing Interrupts";
#endif

					/*
					if ( isDataReadyInterrupt && ( _CD->REG_InterruptFlag & 0x7 ) == 0x0 )
					{
						// data ready interrupt was just acknowledged //
						
						// so, set the last buffer to be that of interrupt
						if ( _CD->IntQueue.Peek() )
						{
							_CD->LastDataBufferSize = _CD->IntQueue.Peek()->SectorDataQ_Size;
							_CD->LastDataBufferIndex = _CD->IntQueue.Peek()->SectorDataQ_Index;
						}
						
					}
					

					//_CD->InterruptQ [ _CD->InterruptQ_Index ].InterruptReason &= ~( Data & _CD->REG_InterruptEnable );
					if ( _CD->IntQueue.Peek () ) _CD->IntQueue.Peek()->InterruptReason &= ~( Data );
					*/
					
#ifdef INLINE_DEBUG_WRITE
					debug << "; Updating interrupt flag";
#endif

					
					// check if there is an primary interrupt enabled
					if ( _CD->CurrentInt.InterruptEnabled )
					{
						// update the interrupt flag //
						
						// update interrupt flag register
						_CD->UpdateREG_InterruptFlag ();
						
						// check on the current interrupt
						// this should disable/remove the interrupt if it is acknowledged
						_CD->Check_Interrupt ();
						
						// if interrupt is acknowledged, then check if there is a pending command //
						if ( !_CD->CurrentInt.InterruptEnabled )
						{
							// interrupt has JUST been acknowledged //
							
							// check for pending command first
							_CD->Check_Command ();
							
							// if there is no pending primary interrupt/command, then check for pending secondary interrupt //
							if ( !_CD->CurrentInt.InterruptPending && _CD->PendingInt.InterruptEnabled )
							{
								// set to check interrupt again later
								_CD->Command = CDREG1_CMD_CHECK_INT;
								_CD->SetNextStartCycle ( c_iPendingInterruptWait_Cycles );
							}
						}
					}
					
					// check if there is a pending secondary interrupt enabled
					
					// if interrupt is acknowledged, then check for next command
					// this gets checked when a command is complete
					//_CD->Check_Command ();
					
					break;
					
				case 2:
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; PendingVolumeCDLeftToSPURight";
}
#endif
#ifdef INLINE_DEBUG_WRITE
					debug << "; PendingVolumeCDLeftToSPURight";
#endif

					// Audio Volume for Left CD-out to Right SPU input //
					_CD->PendingVolume_CDLeftToSPURight = Data;
					break;
					
				case 3:
#ifdef INLINE_DEBUG_RUN
if ( _CD->DebugCount > 0 )
{
					debug << "; ApplyVolume";
}
#endif
#ifdef INLINE_DEBUG_WRITE
					debug << "; ApplyVolume";
#endif

					// Audio Volume Apply Changes (by writing 1 to bit 5)
					// bits 0-4 - Unknown
					// bit 5 - Apply Audio Volume Changes (0: No; 1: Apply)
					// bits 6-7 - Unknown
					
					if ( Data & 0x20 )
					{
						_CD->AppliedVolume_CDLeftToSPULeft = _CD->PendingVolume_CDLeftToSPULeft;
						_CD->AppliedVolume_CDLeftToSPURight = _CD->PendingVolume_CDLeftToSPURight;
						_CD->AppliedVolume_CDRightToSPULeft = _CD->PendingVolume_CDRightToSPULeft;
						_CD->AppliedVolume_CDRightToSPURight = _CD->PendingVolume_CDRightToSPURight;
					}
					
					break;
			}
			
			break;
			
		default:
#ifdef INLINE_DEBUG_WRITE
			debug << "; Invalid";
#endif
		
			// invalid CD Register
			cout << "\nhps1x64 ALERT: Unknown CD WRITE @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data << "\n";
			break;
	};

}



u64 CD::DMA_ReadyForRead ()
{
	return _CD->isSectorDataReady;

	/*
	// return variable that says if the data from the disk is ready to be read or not
	//return isSectorDataReady;
	if ( DataReadIndex >= DataWriteIndex )
	{
//#ifdef INLINE_DEBUG_RUN
//	debug << "\r\nREADNOTREADY";
//#endif

		return false;
	}
	else
	{
		return true;
	}
	*/
}


void CD::DMA_Start ()
{
}

void CD::DMA_End ()
{
}

void CD::DMA_Read ( u32* Data, int ByteReadCount )
{
	u8* pSectorDataBuffer;
	
	// wants to read data from CD
	// must be a game data sector, otherwise it would be playing a track
	
	//cd_image.WaitForSectorReadComplete ();
	
	// get pointer into data buffer
	pSectorDataBuffer = cd_image.GetDataBuffer ( SectorDataBuffer_Index );

	if ( DataBuffer_Index < DataBuffer_Size )
	{
		//Data [ 0 ] = ((u32*)DataBuffer) [ DataBuffer_Index >> 2 ];
		//Data [ 0 ] = ((u32*)SectorDataBuffer) [ ( ReadMode_Offset + DataBuffer_Index ) >> 2 ];
		Data [ 0 ] = ((u32*)pSectorDataBuffer) [ ( ReadMode_Offset + DataBuffer_Index ) >> 2 ];
		
		DataBuffer_Index += 4;
		
		if ( DataBuffer_Index >= DataBuffer_Size )
		{
			// the entire buffer has been read, so it can be deactivated //
			
			// mark entry in sector buffer as not active
			//_CD->SectorDataQ_Active [ _CD->DataBuffer_SlotNumber & c_iMaxQueuedSectors_Mask ] = 0;
			
			// *** IMPORTANT *** the index wraps around
			DataBuffer_Index -= DataBuffer_Size;
		}
	}
	else
	{
		//Data [ 0 ] = ((u32*)DataBuffer) [ ( DataBuffer_Index % DataBuffer_Size ) >> 2 ];
		//Data [ 0 ] = ((u32*)SectorDataBuffer) [ ( ReadMode_Offset + ( DataBuffer_Index % DataBuffer_Size ) ) >> 2 ];
		Data [ 0 ] = ((u32*)pSectorDataBuffer) [ ( ReadMode_Offset + ( DataBuffer_Index % DataBuffer_Size ) ) >> 2 ];
		
		DataBuffer_Index += 4;
	}
	
}


// should return the amount of data read from CD
//static u32 CD::DMA_ReadBlock ( u32* Data, u32 BS, u32 BA )
u32 CD::DMA_ReadBlock ( u32* pMemoryPtr, u32 Address, u32 BS )
{
#ifdef INLINE_DEBUG_DMA_READ
	debug << "\r\nDMA_ReadBlock" << dec;
	debug << dec << " Cycle#" << *_DebugCycleCount;
	debug << " BS=" << BS;
	//debug << "; BA=" << BA;
	debug << " DataBuffer_Index=" << _CD->DataBuffer_Index;
	debug << " DataBuffer_Size=" << _CD->DataBuffer_Size;
	debug << " ReadMode=" << _CD->ReadMode;
	debug << " READ-OFFSET=" << _CD->ReadMode_Offset;
#endif

	u8* pSectorDataBuffer;
	
	// number of words to transfer
	u32 WordTransferCount;
	
	u32 *Data;
	
	Data = & ( pMemoryPtr [ Address >> 2 ] );

#ifdef PS2_COMPILE
	u32 lsn;
	u8 DataByte;
	u32 ShiftAmount, PerformShift, PerformXor;
	u32 XorValue;
	
	// ***todo*** assume single layer dvd for now
	lsn = _CD->DVDSectorNumber + 0x30000;
	
	PerformXor = _CD->DecryptSetting & 1;
	PerformShift = _CD->DecryptSetting & 2;
	ShiftAmount = ( _CD->DecryptSetting >> 4 ) & 7;

	// for PS2, if there are no sectors available, then no transfer

#ifdef INLINE_DEBUG_DMA_READ
	debug << " SECTOR#" << _CD->DVDSectorNumber;
#endif

#endif
	
	// get pointer into data buffer
	pSectorDataBuffer = _CD->cd_image.GetDataBuffer ( _CD->SectorDataBuffer_Index );
	
	/*
	if ( BA )
	{
		if ( DataBuffer_Size == 2048 )
		{
			BS = 0x200;
		}
		else
		{
			if ( !DataBuffer_Index )
			{
				BS = 3;
			}
			else
			{
				BS = 0x100;
			}
		}
	}
	*/
	
	if ( !BS )
	{
#ifdef VERBOSE_CDDMA_BSZERO
		cout << "\nhps1x64 ALERT: DMA3 (CD): BS Transfer size is zero.\n";
#endif

		BS = 0x200;
	}
	
	/*
	if ( BA <= 1 )
	{
		WordTransferCount = BS;
	}
	else
	{
		// need to account for transferring more than one block (ps2 only??)
		WordTransferCount = BS * BA;
	}
	
#ifdef PS2_COMPILE
	// for now, also take the minimum of WordTransferCount and DataBuffer_Size
	WordTransferCount = ( WordTransferCount <= ( _CD->DataBuffer_Size >> 2 ) ) ? WordTransferCount : ( _CD->DataBuffer_Size >> 2 );
#endif
	*/

	// might need to fix this for ps2 usage
	WordTransferCount = BS;
	
	//for ( int i = 0; i < BS; i++ )
	for ( int i = 0; i < WordTransferCount; i++ )
	{
#ifdef PS2_COMPILE

		if ( _CD->ReadMode == 1024 )
		{
			if ( !i )
			{
				memset ( (u8*) Data, 0, _CD->DataBuffer_Size );

//#define FORCE_SINGLE_LAYER

#ifndef FORCE_SINGLE_LAYER
				if ( _CD->cd_image.SizeOfImage <= 4707319808ull )
#endif
				{
				// toc for a single sided dvd ??
				((u8*)Data) [ 0 ] = 0x04;
				((u8*)Data) [ 1 ] = 0x02;
				((u8*)Data) [ 2 ] = 0xf2;
				((u8*)Data) [ 3 ] = 0x00;
				((u8*)Data) [ 4 ] = 0x86;
				((u8*)Data) [ 5 ] = 0x72;

				((u8*)Data) [ 16 ] = 0x00;
				((u8*)Data) [ 17 ] = 0x03;
				((u8*)Data) [ 18 ] = 0x00;
				((u8*)Data) [ 19 ] = 0x00;
				}
#ifndef FORCE_SINGLE_LAYER
				else
				{
					// dual layer dvd ?? //

					u64 layer1start =  _CD->cd_image.layer1start;
					//u32 layer1start = src->GetLayerBreakAddress() + 0x30000;
					layer1start = layer1start + 0x30000 - 1;

					// dual sided
					((u8*)Data)[0] = 0x24;
					((u8*)Data)[1] = 0x02;
					((u8*)Data)[2] = 0xF2;
					((u8*)Data)[3] = 0x00;
					((u8*)Data)[4] = 0x41;
					((u8*)Data)[5] = 0x95;

					((u8*)Data)[14] = 0x60; // PTP
					//((u8*)Data)[14] = 0x61; // PTP
					//((u8*)Data)[14] = 0x71; // OTP

					((u8*)Data)[16] = 0x00;
					((u8*)Data)[17] = 0x03;
					((u8*)Data)[18] = 0x00;
					((u8*)Data)[19] = 0x00;

					((u8*)Data)[20] = (layer1start >> 24);
					((u8*)Data)[21] = (layer1start >> 16) & 0xff;
					((u8*)Data)[22] = (layer1start >> 8) & 0xff;
					((u8*)Data)[23] = (layer1start >> 0) & 0xff;

				}
#endif

			}	// end if ( !i )
			

			/*
			// cd toc //
			// Read Toc ??
			((u8*)Data) [ 0 ] = 0x41;
			((u8*)Data) [ 1 ] = 0;
			
			// starting track ??
			((u8*)Data) [ 2 ] = 0xa0;
			((u8*)Data) [ 7 ] = 1;
			
			// ending track ??
			((u8*)Data) [ 12 ] = 0xa1;
			((u8*)Data) [ 17 ] = 1;
			
			// disk length ??
			u8 tempMin, tempSec, tempFrac;
			cd_image.SplitSectorNumber ( cd_image.LastSector_Number, tempMin, tempSec, tempFrac );
			((u8*)Data) [ 22 ] = 0xa2;
			((u8*)Data) [ 27 ] = cd_image.ConvertDecToBCD8( tempMin );
			((u8*)Data) [ 28 ] = cd_image.ConvertDecToBCD8( tempSec );
			((u8*)Data) [ 29 ] = cd_image.ConvertDecToBCD8( tempFrac );
			
			((u8*)Data) [ 22 ] = 0xa2;
			*/
		}
		else if ( _CD->ReadMode == 2064 && i <= 2 )
		{
			if ( !i )
			{
				// check for dual layer
#ifndef FORCE_SINGLE_LAYER
				if ( ( _CD->cd_image.SizeOfImage > 4707319808ull ) && ( ( lsn - 0x30000 ) >= _CD->cd_image.layer1start ) )
				{
					// layer 1 //
					lsn -= _CD->cd_image.layer1start;
					((u8*)Data) [ 0 ] = 0x21;
				}
				else
#endif
				{
					((u8*)Data) [ 0 ] = 0x20;
				}

				((u8*)Data) [ 1 ] = ( lsn >> 16 );
				((u8*)Data) [ 2 ] = ( lsn >> 8 );
				((u8*)Data) [ 3 ] = lsn;
			}
			else
			{
				// make zero for now
				Data [ i ] = 0;
			}
		}
		else if ( _CD->ReadMode == 2064 && i >= 515 )
		{
			// make zero
			Data [ i ] = 0;
		}
		else
		{
		
			if ( _CD->ReadMode == 2064 )
			{
				Data [ i ] = ((u32*)pSectorDataBuffer) [ ( _CD->ReadMode_Offset + ( _CD->DataBuffer_Index - 12 ) ) >> 2 ];
			}
			else
			{
#endif
	
	
		//if ( DataBuffer_Index < DataBuffer_Size )
		//{
			//Data [ i ] = ((u32*)DataBuffer) [ DataBuffer_Index >> 2 ];
			//Data [ i ] = ((u32*)SectorDataBuffer) [ ( ReadMode_Offset + DataBuffer_Index ) >> 2 ];
			Data [ i ] = ((u32*)pSectorDataBuffer) [ ( _CD->ReadMode_Offset + _CD->DataBuffer_Index ) >> 2 ];
			
#ifdef PS2_COMPILE
			
			} // end if ReadMode == 2064
			
			if ( _CD->DecryptSetting )
			{
				
				// check if data is set to be decrypted
				for ( int j = 0; j < 4; j++ )
				{
					//DataByte = ((u8*)Data) [ j ];
					DataByte = ((u8*)Data) [ j + _CD->DataBuffer_Index ];
					
					// xor
					if ( PerformXor ) DataByte ^= _CD->XorKey;
					
					// shift
					if ( PerformShift ) DataByte = ( DataByte >> ShiftAmount ) | ( DataByte << ( 8 - ShiftAmount ) );
					
					// write back
					//((u8*)Data) [ j ] = DataByte;
					((u8*)Data) [ j + _CD->DataBuffer_Index ] = DataByte;
				}
			}	// end if DecryptSetting
			
		}	// end else
			
#endif

			_CD->DataBuffer_Index += 4;
			
			
			if ( _CD->DataBuffer_Index >= _CD->DataBuffer_Size )
			{
				// the entire buffer has been read, so it can be deactivated //

#ifdef INLINE_DEBUG_DMA_READ_DATA
	debug << hex << " SECTORDATA:";
	for ( int k = 0; k < _CD->DataBuffer_Size; k++ )
	{
	debug << " " << ((u32) (((u8*)Data) [ k ]));
	}
#endif

				// mark entry in sector buffer as not active
				//_CD->SectorDataQ_Active [ _CD->DataBuffer_SlotNumber & c_iMaxQueuedSectors_Mask ] = 0;
				
				// *** IMPORTANT *** the index wraps around
				//DataBuffer_Index -= DataBuffer_Size;
				_CD->DataBuffer_Index = 0;
				
				// done in data buffer?
				_CD->DataBuffer_Size = 0;

				// done reading the sector
				//break;
			}
		//}
		
		/*
		else
		{
			//Data [ i ] = ((u32*)DataBuffer) [ ( DataBuffer_Index % DataBuffer_Size ) >> 2 ];
			//Data [ i ] = ((u32*)SectorDataBuffer) [ ( ReadMode_Offset + ( DataBuffer_Index % DataBuffer_Size ) ) >> 2 ];
			Data [ i ] = ((u32*)pSectorDataBuffer) [ ( ReadMode_Offset + ( DataBuffer_Index % DataBuffer_Size ) ) >> 2 ];
			
			DataBuffer_Index += 4;
		}
		*/

	}	// end for ( int i = 0; i < WordTransferCount; i++ )


	
#ifdef PS2_COMPILE
	// for PS2, for now, update the sector index and decrease the number of available sectors
	//SectorDataBuffer_Index++;
	
#ifdef INLINE_DEBUG_TESTING
	debug << "\r\nDVDSector#" << dec << _CD->DVDSectorNumber;
	_CD->OutputCurrentSector ();
#endif
#endif


	
	
#ifdef INLINE_DEBUG_RUN
	//debug << "; (Output) WordsRead=" << BS;
	debug << "; (Output) WordsRead=" << WordTransferCount;
#endif
	
	//return BS;
	return WordTransferCount;
}


void CD::DMA_Write ( u32* Data, int ByteWriteCount )
{
}


int CD::isDataSector ( unsigned char* data )
{
	static const int c_iNumberOfSyncBytes = 12;
	static unsigned char c_SyncBytes [ c_iNumberOfSyncBytes ] = { 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 };
	
	for ( int i = 0; i < c_iNumberOfSyncBytes; i++ )
	{
		if ( data [ i ] != c_SyncBytes [ i ] ) return false;
	}
	
	return true;
}


int CD::isPlaying ()
{
	if ( CD::_CD->Status & CDREG1_READ_STAT_PLAY ) return true;
	
	return false;
}


void CD::DeactivateBuffer ()
{
	QueuedBufferActive [ 0 ] = 0;
	
	// start from beginning of buffer
	QueueIndex = 0;
}

/*
void CD::FlushBuffer ()
{
	int i;
	
#ifdef INLINE_DEBUG_RUN
	debug << "; FlushBuffer; QueueIndex=" << QueueIndex << " QueuedBufferSize [ 0 ]=" << QueuedBufferSize [ 0 ] << " (before) " << QueuedBufferActive [ 0 ] << " " << QueuedBufferActive [ 1 ] << " " << QueuedBufferActive [ 2 ] << " " << QueuedBufferActive [ 3 ];
#endif

	//if ( QueueIndex >= QueuedBufferSize [ 0 ] )
	//{
		// just bring in the next buffers and whether or not the buffer is active
		for ( i = 1; i < c_InterruptQueueDepth; i++ )
		{
			QueuedResultBuffers.b64 [ i - 1 ] = QueuedResultBuffers.b64 [ i ];
			QueuedBufferSize [ i - 1 ] = QueuedBufferSize [ i ];
			QueuedBufferActive [ i - 1 ] = QueuedBufferActive [ i ];
		}
		
		// clear out the last buffer and deactivate
		QueuedResultBuffers.b64 [ c_InterruptQueueDepth - 1 ] = 0;
		QueuedBufferSize [ c_InterruptQueueDepth - 1 ] = 0;
		QueuedBufferActive [ c_InterruptQueueDepth - 1 ] = 0;
		
		// reset index in queue
		QueueIndex = 0;
	//}
	
#ifdef INLINE_DEBUG_RUN
	debug << " (after) " << QueuedBufferActive [ 0 ] << " " << QueuedBufferActive [ 1 ] << " " << QueuedBufferActive [ 2 ] << " " << QueuedBufferActive [ 3 ];
#endif
}
*/

/*
void CD::FlushIRQ ()
{
	// bring in next interrupt
	for ( int i = 1; i < c_InterruptQueueDepth; i++ )
	{
		QueuedBufferInterrupt [ i - 1 ] = QueuedBufferInterrupt [ i ];
		//QueuedBufferActive [ i - 1 ] = QueuedBufferActive [ i ];
	}
	
	// clear out the last interrupt
	QueuedBufferInterrupt [ c_InterruptQueueDepth - 1 ] = 0;
	//QueuedBufferActive [ c_InterruptQueueDepth - 1 ] = 0;
	
	// reset index in queue
	QueueIndex = 0;
}
*/

// queues interrupt for CD
void CD::EnqueueInterrupt ( u8* Data, u32 Size, u8 Interrupt )
{
#ifdef INLINE_DEBUG_INTQ
	CD::debug << "\r\nCD::EnqueueInterrupt; Size=" << Size << " Interrupt=" << (u32) Interrupt << " QueueSize=" << IntQueue.Size();
	CD::debug << " Data=" << (u32) Data [ 0 ] << " " << (u32) Data [ 1 ] << " " << (u32) Data [ 2 ] << " " << (u32) Data [ 3 ];
#endif

//#ifdef INLINE_DEBUG_RUN
//	CD::debug << "; INT";
//#endif

	int i;
	
	InterruptQ_Entry *IntQ_Entry;
	
	// interrupt is no longer pending
	CurrentInt.InterruptPending = false;
	
	// nothing sent for interrupt yet
	CurrentInt.InterruptSent = false;
	CurrentInt.ResponseSent = false;
	CurrentInt.DataSent = false;
	
	// set as the current interrupt
	CurrentInt.Set ( Interrupt, Data, Size );
	CurrentInt.isDataReadyInterrupt = false;
	CurrentInt.InterruptEnabled = true;
	
	// Trigger interrupt
	
	UpdateREG_ModeStatus ();
	//UpdateREG_InterruptFlag ( true );
	
	// check if current interrupt should be sent and send what is needed
	Check_Interrupt ();
	
	
	/*
	if ( IntQueue.Peek () )
	{
		IntQ_Entry = IntQueue.Peek ();
		IntQ_Entry->Set ( Interrupt, Data, Size );
		IntQ_Entry->SectorDataQ_Index = SectorDataQ_Index & c_iMaxQueuedSectors_Mask;
		IntQ_Entry->SectorDataQ_Size = ReadMode;
		
		// start at beginning of the response buffer
		// *note* only do this when triggering actual interrupt
		//ResponseBuf_Index = 0;
		
		// enable the interrupt
		IntQ_Entry->InterruptEnabled = true;
		
		// copy response buffer
		// note: only copy in the response buffer when the interrupt is generated
		//ResponseBuf_Index = 0;
		//ResponseBuf_Size = IntQ_Entry->ResponseSize;
		//for ( i = 0; i < ResponseBuf_Size; i++ ) Current_ResponseBuf [ i ] = IntQ_Entry->ResponseBuf [ i ];
		
		// update interrupt flags register
		UpdateREG_InterruptFlag ( true );
		
		// update mode/status register
		UpdateREG_ModeStatus ();
	}
	*/
	
	/*
	for ( i = 0; i < c_InterruptQueueDepth; i++ )
	{
		if ( ! ( InterruptQ [ ( InterruptQ_Index + i ) & c_InterruptQ_Mask ].InterruptEnabled ) )
		{
			// interruptq slot is empty //
			
			// clear out first
			InterruptQ [ ( InterruptQ_Index + i ) & c_InterruptQ_Mask ].Clear ();
			
			// populate slot
			InterruptQ [ ( InterruptQ_Index + i ) & c_InterruptQ_Mask ].Set ( Interrupt, Data, Size );
			
			// reset index for response buffer
			ResponseBuf_Index = 0;
			
			// *** testing ***
			InterruptQ [ ( InterruptQ_Index + i ) & c_InterruptQ_Mask ].SectorDataQ_Index = SectorDataQ_Index & c_iMaxQueuedSectors_Mask;
			InterruptQ [ ( InterruptQ_Index + i ) & c_InterruptQ_Mask ].SectorDataQ_Size = ReadMode;
			
			// update interrupt flags register
			UpdateREG_InterruptFlag ();
			
			// update mode/status register
			UpdateREG_ModeStatus ();
			
			// done
			break;
		}

	}
	*/
	
}

void CD::EnqueueInterrupt_Read ( u8* Data, u32 Size, u8 Interrupt )
{
#ifdef INLINE_DEBUG_INTQ_READ
	debug << "\r\nCD::EnqueueInterrupt_Read; Size=" << Size << " Interrupt=" << (u32) Interrupt << " Pending=" << CurrentInt.InterruptPending << " Enabled=" << CurrentInt.InterruptEnabled;
#endif

	InterruptQ_Entry IntQ_Entry, *pIntQ_Entry;
	
	// check for an active or pending primary interrupt
	if ( !CurrentInt.InterruptPending && !CurrentInt.InterruptEnabled )
	{
#ifdef INLINE_DEBUG_INTQ_READ
	debug << "; !CurrentInt.InterruptPending && !CurrentInt.InterruptEnabled";
#endif

		// set as the current interrupt
		CurrentInt.Set ( Interrupt, Data, Size );
		
		// interrupt is no longer pending
		CurrentInt.InterruptPending = false;
		
		CurrentInt.InterruptPending = false;
		CurrentInt.InterruptEnabled = true;
		
		// nothing sent for interrupt yet
		CurrentInt.InterruptSent = false;
		CurrentInt.ResponseSent = false;
		CurrentInt.DataSent = false;
		
		// set data queue size
		CurrentInt.SectorDataQ_Size = ReadMode;
		
		// set the interrupt flag register
		//REG_InterruptFlag |= Interrupt;
		
		//UpdateREG_InterruptFlag ( true );
		UpdateREG_ModeStatus ();
		
		// just enabled the current interrupt, so send it
		Check_Interrupt ();
	}
	else
	{
#ifdef INLINE_DEBUG_INTQ_READ
	debug << "; CurrentInt.InterruptPending || CurrentInt.InterruptEnabled";
#endif

		// set as the pending interrupt
		
		// set as the current interrupt
		PendingInt.Set ( Interrupt, Data, Size );
		
		// pending interrupt is enabled
		PendingInt.InterruptEnabled = true;
		
		// interrupt is no longer pending
		PendingInt.InterruptPending = false;
		
		// nothing sent for interrupt yet
		PendingInt.InterruptSent = false;
		PendingInt.ResponseSent = false;
		PendingInt.DataSent = false;
		
		// set data queue size
		PendingInt.SectorDataQ_Size = ReadMode;
		
		/*
		if ( Interrupt == 1 )
		{
			PendingInt.isDataReadyInterrupt = true;
		}
		else
		{
			CurrentInt.isDataReadyInterrupt = false;
		}
		*/
		
		PendingInt.InterruptEnabled = true;
		
		//UpdateREG_InterruptFlag ( true );
		//UpdateREG_ModeStatus ();
	}
	
	/*
	IntQ_Entry.Set ( Interrupt, Data, Size );
	
	IntQ_Entry.SectorDataQ_Index = SectorDataQ_Index & c_iMaxQueuedSectors_Mask;
	IntQ_Entry.SectorDataQ_Size = ReadMode;
	//pIntQ_Entry.AssociatedDataBuffer_Index = cd_image.GetCurrentBufferIndex ();
	
	// enable the interrupt
	IntQ_Entry.InterruptEnabled = true;
	
	pIntQ_Entry = IntQueue.Add ( IntQ_Entry );
	//pIntQ_Entry = IntQueue.Peek ();
	
	if ( pIntQ_Entry )
	{
		pIntQ_Entry->Set ( Interrupt, Data, Size );
		pIntQ_Entry->SectorDataQ_Index = SectorDataQ_Index & c_iMaxQueuedSectors_Mask;
		pIntQ_Entry->SectorDataQ_Size = ReadMode;
		//pIntQ_Entry->AssociatedDataBuffer_Index = cd_image.GetCurrentBufferIndex ();
	
		// enable the interrupt
		pIntQ_Entry->InterruptEnabled = true;
		
		// copy response buffer
		// note: only copy over if there is an interrupt
		//ResponseBuf_Index = 0;
		//ResponseBuf_Size = Size;
		//for ( int i = 0; i < ResponseBuf_Size; i++ ) Current_ResponseBuf [ i ] = Data [ i ];
	}
	*/
	
	
//#ifdef INLINE_DEBUG_INTQ_READ
//	debug << "; IsEmpty=" << (u32)IntQueue.IsEmpty() << "; Reason=" << (u32)IntQueue.Peek()->InterruptReason << "; InterruptEnabled=" << (u32)IntQueue.Peek()->InterruptEnabled << " QueueSize=" << IntQueue.Size() << "\r\n";
//#endif

	// update interrupt flags register
	//UpdateREG_InterruptFlag ( true );
	
	// update mode/status register
	//UpdateREG_ModeStatus ();
}




void CD::OutputCurrentSector ()
{
	int i, j;
	u8* pSectorDataBuffer;
	
	//pSectorDataBuffer = cd_image.GetCurrentDataBuffer ();
	pSectorDataBuffer = cd_image.GetDataBuffer ( SectorDataBuffer_Index );
	
	if ( !pSectorDataBuffer )
	{
		cout << "\nCD::OutputCurrentSector. No current sector to output.\n";
		return;
	}
	
	//cout << "\nCD::OutputCurrentSector. Sector Output:\n";
	//debug << \r\n
	
	for ( j = 0; j < 16; j++ )
	{
	
	debug << "\r\n";
	
	//for ( i = 0; i < 2352; i++ )
	for ( i = 0; i < 32; i++ )
	{
		//cout << hex << (u32) pSectorDataBuffer [ i ] << " ";
		debug << hex << ((u32*)pSectorDataBuffer) [ ( j * 32 ) + i ] << " ";
	}
	
	}
}


void CD::SetNextEventCycle ()
{
	/*
	if ( NextRead_Cycle > *_SystemCycleCount && ( NextRead_Cycle < NextAction_Cycle || NextAction_Cycle <= *_SystemCycleCount ) && ( NextRead_Cycle < NextStart_Cycle || NextStart_Cycle <= *_SystemCycleCount ) )
	{
		// the next read event is the next event for device
		NextEvent_Cycle = NextRead_Cycle;
	}
	else if ( NextAction_Cycle > *_SystemCycleCount && ( NextAction_Cycle < NextStart_Cycle || NextStart_Cycle <= *_SystemCycleCount ) )
	{
		NextEvent_Cycle = NextAction_Cycle;
	}
	else if ( NextStart_Cycle > *_SystemCycleCount )
	{
		NextEvent_Cycle = NextStart_Cycle;
	}

	if ( NextEvent_Cycle > *_SystemCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_SystemCycleCount ) ) *_NextSystemEvent = NextEvent_Cycle;
	*/
	
	NextEvent_Cycle = -1ULL;
	
	if ( NextRead_Cycle < NextEvent_Cycle )
	{
		// the next read event is the next event for device
		NextEvent_Cycle = NextRead_Cycle;
	}
	
	if ( NextAction_Cycle < NextEvent_Cycle )
	{
		NextEvent_Cycle = NextAction_Cycle;
	}
	
	if ( NextStart_Cycle < NextEvent_Cycle )
	{
		NextEvent_Cycle = NextStart_Cycle;
	}

	if ( NextEvent_Cycle < *_NextSystemEvent )
	{
		*_NextSystemEvent = NextEvent_Cycle;
		*_NextEventIdx = NextEvent_Idx;
	}
}


void CD::SetNextStartCycle ( u64 Cycles )
{
	NextStart_Cycle = Cycles + *_DebugCycleCount;
	
	SetNextEventCycle ();
}

void CD::SetNextReadCycle ( u64 Cycles )
{
	NextRead_Cycle = Cycles + *_DebugCycleCount;
	
	SetNextEventCycle ();
}

void CD::SetNextActionCycle ( u64 Cycles )
{
	NextAction_Cycle = Cycles + *_DebugCycleCount;
	
	SetNextEventCycle ();
}



// sometimes next to set the exact cycle number
void CD::SetNextStartCycle_Cycle ( u64 ullCycle )
{
	NextStart_Cycle = ullCycle;
	
	SetNextEventCycle ();
}

void CD::SetNextReadCycle_Cycle ( u64 ullCycle )
{
	NextRead_Cycle = ullCycle;
	
	SetNextEventCycle ();
}

void CD::SetNextActionCycle_Cycle ( u64 ullCycle )
{
	NextAction_Cycle = ullCycle;
	
	SetNextEventCycle ();
}



void CD::SendCommand ( u8 CommandToSend )
{
#ifdef INLINE_DEBUG_RUN
	debug << "\r\nSending Command: " << CommandList [ CommandToSend & 0x1f ];
#endif

	// *** testing *** if command is a setmod command then stop reading anything
	//if ( CommandToSend == CDREG1_CMD_SETMODE )
	//{
	//	// stop reading anything
	//	ReadCommand = -1;
	//}

	// *** testing *** if command is a read command and setloc has been invoked, then stop any reading that is in progress
	if ( ( CommandToSend == CDREG1_CMD_READN || CommandToSend == CDREG1_CMD_READS ) && !hasSeeked )
	{
		// stop reading anything
		ReadCommand = -1;
	}

	// reserve a spot for primary command result in the interrupt queue
	//InterruptQ_Entry ie;
	//ie.Clear ();
	//_CD->IntQueue.Add ( ie );
	CurrentInt.InterruptPending = true;

	// set the command
	Command = CommandToSend;
	
	// get the clock requirements from lookup table
	BusyCycles = CommandExecutionTimes [ CommandToSend & 0x1f ];
	SetNextActionCycle ( _CD->BusyCycles );
	
	// command is starting
	isCommandStart = true;
	
	// set the command to start after 0x800 cycles
	SetNextStartCycle ( 0x800 );
	
	UpdateREG_ModeStatus ();
}


u8 CD::ConvertBCDToDec ( u8 BCD )
{
	if ( BCD > 0x99 ) BCD -= 0x80;
	return ( BCD & 0xf ) + ( ( BCD >> 4 ) * 10 );
}

u8 CD::ConvertDecToBCD ( u8 Dec )
{
	return ( ( Dec / 10 ) << 4 ) + ( ( Dec ) % 10 );
}


void CD::Inc_DiskPosition ( u8& tMin, u8& tSec, u8& tFrac )
{
	tFrac++;
	if ( tFrac >= 75 )
	{
		tFrac = 0;
		tSec++;
		if ( tSec >= 60 )
		{
			tSec = 0;
			tMin++;
		}
	}
}



void CD::InterruptQ_Entry::Clear ()
{
	InterruptEnabled = 0;
	InterruptReason = 0;
	ResponseSize = 0;
	((u64*)ResponseBuf) [ 0 ] = 0;
	((u64*)ResponseBuf) [ 1 ] = 0;
}

void CD::InterruptQ_Entry::Set ( u32 Reason, u8* Data, u32 Size )
{
#ifdef INLINE_DEBUG_INTQ
	CD::debug << "\r\nInterruptQ_Entry::Set";
#endif

	// clear the entry first to get all zeros in response buffer
	Clear ();

	InterruptEnabled = 1;
	InterruptReason = Reason;
	ResponseSize = Size;
	for ( int i = 0; i < Size; i++ ) ResponseBuf [ i ] = Data [ i ];
	
#ifdef INLINE_DEBUG_INTQ
	CD::debug << hex << "; Enabled=" << InterruptEnabled << " Reason=" << InterruptReason << " Size=" << Size << " Response=" << (u32) ResponseBuf [ 0 ] << " " << (u32) ResponseBuf [ 1 ] << " " << (u32) ResponseBuf [ 2 ] << " " << (u32) ResponseBuf [ 3 ];
	CD::debug << " Data=" << (u32) Data [ 0 ] << " " << (u32) Data [ 1 ] << " " << (u32) Data [ 2 ] << " " << (u32) Data [ 3 ];
#endif
}



// note: this should only be used for CDDA, NOT CDXA
u32 CD::Apply_CDDA_Volume ( u32 LRSample )
{
	u32 Output, OutputLeft, OutputRight;
	
	// there is sound data in buffer intended for SPU //
	//Output = SpuBuffer [ SpuBuffer_ReadIndex & c_iSpuBuffer_Mask ];
	Output = LRSample;
	//SpuBuffer_ReadIndex++;
	
	// apply volume
	// assuming that left is the low halfword and right is the high halfword
	OutputLeft = adpcm_decoder::clamp ( ( ( ((s32) ((s16) (Output & 0xffff))) * ((s32)((u32)AppliedVolume_CDLeftToSPULeft)) ) >> 7 ) + ( ( ((s32) ((s16) ( ( Output >> 16 ) & 0xffff))) * ((s32)((u32)AppliedVolume_CDRightToSPULeft)) ) >> 7 ) );
	OutputRight = adpcm_decoder::clamp ( ( ( ((s32) ((s16) (Output & 0xffff))) * ((s32)((u32)AppliedVolume_CDLeftToSPURight)) ) >> 7 ) + ( ( ((s32) ((s16) ( ( Output >> 16 ) & 0xffff))) * ((s32)((u32)AppliedVolume_CDRightToSPURight)) ) >> 7 ) );
	
	Output = ( OutputRight << 16 ) | ( OutputLeft & 0xffff );
	
	return Output;
}


// this is for both CDDA and CDXA
u32 CD::Spu_ReadNextSample ()
{
	u32 Output;
	
	u32 OutputLeft, OutputRight;
	
	if ( SpuBuffer_ReadIndex >= SpuBuffer_WriteIndex )
	{
		// there is no sound data coming from CD device intended for the SPU //
		Output = 0;
	}
	else
	{
		// there is sound data in buffer intended for SPU //
		Output = SpuBuffer [ SpuBuffer_ReadIndex & c_iSpuBuffer_Mask ];
		SpuBuffer_ReadIndex++;
		
		// apply volume
		// assuming that left is the low halfword and right is the high halfword
		/*
		OutputLeft = adpcm_decoder::clamp ( ( ( ((s32) ((s16) (Output & 0xffff))) * ((s32)((u32)AppliedVolume_CDLeftToSPULeft)) ) >> 7 ) + ( ( ((s32) ((s16) ( ( Output >> 16 ) & 0xffff))) * ((s32)((u32)AppliedVolume_CDRightToSPULeft)) ) >> 7 ) );
		OutputRight = adpcm_decoder::clamp ( ( ( ((s32) ((s16) (Output & 0xffff))) * ((s32)((u32)AppliedVolume_CDLeftToSPURight)) ) >> 7 ) + ( ( ((s32) ((s16) ( ( Output >> 16 ) & 0xffff))) * ((s32)((u32)AppliedVolume_CDRightToSPURight)) ) >> 7 ) );
		
		Output = ( OutputRight << 16 ) | ( OutputLeft & 0xffff );
		*/
	}
	
	// don't output anything if set to mute
	if ( Mute ) Output = 0;
	
	return Output;
}


u8 SplitData [ 14 * 4 * 2 ];
s32 TempSamplesL [ 28 ];
s32 TempSamplesR [ 28 ];


u32 CD::DecodeXA_Sector ( long* Output, char* FullSectorData )
{
#ifdef INLINE_DEBUG_XA
	//debug << "\r\nDecodeXA_Sector";
#endif

	//u32 c_iHeaderOffset_Stereo [ 8 ] = { 0, 0, 2, 2, 8, 8, 10, 10 };
	//u32 c_iHeaderOffset_Mono [ 8 ] = { 0, 1, 2, 3, 8, 9, 10, 11 };
	
	int Channels, SampleRate;
	u32 SampleRateAndChannels;
	int shift, filter;
	CDImage::Sector::Mode2_2328 *data;
	
	u32 Offset, SplitOffset, HeaderOffset, OutputCount;
	
	unsigned char *header, *adpcm_data;
	
	OutputCount = 0;
	
	data = (CDImage::Sector::Mode2_2328*) FullSectorData;
	
	// determine if data is stereo or mono
	// bit 0 of code byte is for stereo/mono (0: mono; 1: stereo)
	Channels = ( data->Coding & 1 ) + 1;
	
	// determine audio frequency of data
	// bit 2 of code byte is for sample rate (0: 37800hz; 1: 18900 hz)
	SampleRate = ( data->Coding & 0x4 ) ? 18900 : 37800;
	
	SampleRateAndChannels = ( SampleRate << 16 ) | Channels;
	
#ifdef INLINE_DEBUG_XA
	debug << "; Channels=" << dec << Channels << "; SampleRate=" << SampleRate;
#endif

	// 18 blocks per sector
	for ( Offset = 0; Offset < 0x900; Offset += 128 )
	{
		// get a pointer to adpcm headers
		header = & (data->Data [ Offset + 4 ]);
		//header = & (data->Data [ Offset + 0 ]);
		
		// get a pointer to adpcm data
		adpcm_data = & ( data->Data [ Offset + 16 ] );
		
		// split the blocks //
		SplitOffset = 0;
		
		for ( int block = 0; block < 4; block++ )
		{
			// first 28 samples are in the lo nibbles
			for ( int sample = 0; sample < 28; sample += 2 )
			{
				SplitData [ SplitOffset ] = ( adpcm_data [ block + ( ( sample + 1 ) * 4 ) ] & 0xf ) << 4;
				SplitData [ SplitOffset ] |= ( adpcm_data [ block + ( ( sample + 0 ) * 4 ) ] & 0xf );
				SplitOffset++;
			}
			
			// next 28 samples are in the hi nibbles
			for ( int sample = 0; sample < 28; sample += 2 )
			{
				SplitData [ SplitOffset ] = ( adpcm_data [ block + ( ( sample + 1 ) * 4 ) ] & 0xf0 );
				SplitData [ SplitOffset ] |= ( adpcm_data [ block + ( ( sample + 0 ) * 4 ) ] & 0xf0 ) >> 4;
				SplitOffset++;
			}
		}
		
		// decode the data //
		
		if ( Channels == 2 )
		{
			// stereo //
			
			for ( SplitOffset = 0, HeaderOffset = 0; SplitOffset < ( 14 * 4 * 2 ); SplitOffset += 28, HeaderOffset += 2 )
			{
				// get shift and filter from header for block
				//shift = header [ HeaderOffset + 0 ] & 0xf;
				//filter = header [ HeaderOffset + 0 ] >> 4;
				
				// first come samples on the left
				//LeftSpeaker.decode_samples ( shift, filter, TempSamplesL, & (SplitData [ SplitOffset + 0 ]), 28 );
				LeftSpeaker.decode_packet_xa32 ( header [ HeaderOffset + 0 ], (adpcm_packet_xa*) & (SplitData [ SplitOffset + 0 ]), TempSamplesL );
				
				// get shift and filter from header for block
				//shift = header [ HeaderOffset + 1 ] & 0xf;
				//filter = header [ HeaderOffset + 1 ] >> 4;
				
				//RightSpeaker.decode_samples ( shift, filter, TempSamplesR, & (SplitData [ SplitOffset + 14 ]), 28 );
				RightSpeaker.decode_packet_xa32 ( header [ HeaderOffset + 1 ], (adpcm_packet_xa*) & (SplitData [ SplitOffset + 14 ]), TempSamplesR );
				
				// interleave
				for ( int i = 0; i < 28; i++ )
				{
					Output [ OutputCount++ ] = TempSamplesL [ i ];
					Output [ OutputCount++ ] = TempSamplesR [ i ];
				}
			}
		}
		else
		{
			// mono //
			
			for ( SplitOffset = 0, HeaderOffset = 0; SplitOffset < ( 14 * 4 * 2 ); SplitOffset += 14, HeaderOffset++ )
			{
				// get shift and filter from header for block
				//shift = header [ HeaderOffset ] & 0xf;
				//filter = header [ HeaderOffset ] >> 4;
				
				// first come samples on the left
				//LeftSpeaker.decode_samples ( shift, filter, TempSamplesL, & (SplitData [ SplitOffset + 0 ]), 28 );
				LeftSpeaker.decode_packet_xa32 ( header [ HeaderOffset + 0 ], (adpcm_packet_xa*) & (SplitData [ SplitOffset + 0 ]), TempSamplesL );
				
				// interleave
				for ( int i = 0; i < 28; i++ )
				{
					Output [ OutputCount++ ] = TempSamplesL [ i ];
					Output [ OutputCount++ ] = TempSamplesL [ i ];
				}
			}
		}
	}
	
#ifdef INLINE_DEBUG_XA
	debug << "; Count=" << dec << OutputCount;
#endif
	
	return SampleRateAndChannels;
}



void CD::Process_XASector ( u8* pSectorDataBuffer )
{
	int SampleRate, CurrentSample;
	u32 SampleRateAndChannels;
	u64 SampleUpdate;
	//s32 SampleLR;
	s32 SampleL, SampleR;
	u32 NumOfChannels;
	u32 DecodeSize;
	
	// decode and send xa audio to spu //
	SampleRateAndChannels = DecodeXA_Sector ( TempXA_Buffer, (char*) pSectorDataBuffer );
	//SampleRate = DecodeXA_Sector ( TempXA_Buffer [ c_iXADecode_Size * ( TempXA_BufferCount & 1 ) ], SectorDataBuffer );
	
	SampleRate = SampleRateAndChannels >> 16;
	NumOfChannels = SampleRateAndChannels & 0xffff;
	
	// interpolate samples into output buffer to spu
	//for ( int i = 0; i < ( c_iXADecode_Size / 2 ); i++ ) SpuBuffer [ ( SpuBuffer_WriteIndex + i ) & c_iSpuBuffer_Mask ] = ((u32*)TempXA_Buffer) [ i ];
	int i = 0;
	
	CurrentSample = 0;
	//CurrentSample = c_iXADecode_Size * ( TempXA_BufferCount & 1 );
	//CurrentSample %= c_iXADecode_Size;
	
	if ( SampleRate == 37800 )
	{
		SampleUpdate = c_iSampleUpdate;
	}
	else
	{
		SampleUpdate = c_iSampleUpdate2;
	}
	
	if ( NumOfChannels == 1 ) DecodeSize = 8064; else DecodeSize = 4032;
	
	// put samples into circular buffers
	BufferL [ CurrentSample & 0x3 ] = TempXA_Buffer [ ( CurrentSample << 1 ) + 0 ];
	BufferR [ CurrentSample & 0x3 ] = TempXA_Buffer [ ( CurrentSample << 1 ) + 1 ];
	
	//while ( CurrentSample < ( c_iXADecode_Size / 2 ) )
	while ( CurrentSample < ( DecodeSize / 2 ) )
	{
		// interpolate samples
		// since circular buffer is multiple of 4, it should also have previous values already in there
		//BufferL [ i & 0x3 ] = TempXA_Buffer [ ( CurrentSample << 1 ) + 0 ];
		//BufferR [ i & 0x3 ] = TempXA_Buffer [ ( CurrentSample << 1 ) + 1 ];
		
		SampleL = SPU::Calc_sample_gx ( SampleInterpolation >> 16, BufferL [ CurrentSample & 3 ], BufferL [ ( CurrentSample - 1 ) & 3 ], BufferL [ ( CurrentSample - 2 ) & 3 ], BufferL [ ( CurrentSample - 3 ) & 3 ] );
		SampleR = SPU::Calc_sample_gx ( SampleInterpolation >> 16, BufferR [ CurrentSample & 3 ], BufferR [ ( CurrentSample - 1 ) & 3 ], BufferR [ ( CurrentSample - 2 ) & 3 ], BufferR [ ( CurrentSample - 3 ) & 3 ] );
		//SampleL = SPU::Calc_sample_gx ( SampleInterpolation >> 16, BufferL [ i & 3 ], BufferL [ ( i - 1 ) & 3 ], BufferL [ ( i - 2 ) & 3 ], BufferL [ ( i - 3 ) & 3 ] );
		//SampleR = SPU::Calc_sample_gx ( SampleInterpolation >> 16, BufferR [ i & 3 ], BufferR [ ( i - 1 ) & 3 ], BufferR [ ( i - 2 ) & 3 ], BufferR [ ( i - 3 ) & 3 ] );
		
		// clamp samples
		SampleL = adpcm_decoder::clamp ( SampleL );
		SampleR = adpcm_decoder::clamp ( SampleR );
		
		// put sample into buffer to be output to spu
		//SpuBuffer [ ( SpuBuffer_WriteIndex + i ) & c_iSpuBuffer_Mask ] = ((u32*)TempXA_Buffer) [ CurrentSample ];
		SpuBuffer [ ( SpuBuffer_WriteIndex + i ) & c_iSpuBuffer_Mask ] = ( SampleL << 16 ) | ( SampleR & 0xffff );
		
		// update sample output counter
		i++;
		
		//SampleInterpolation += c_iSampleUpdate;
		SampleInterpolation += SampleUpdate;
		
		if ( SampleInterpolation >= ( 1ULL << 32 ) )
		{
			SampleInterpolation -= ( 1ULL << 32 );
			CurrentSample++;
			//CurrentSample += 2;
			
			// put samples into circular buffers
			BufferL [ CurrentSample & 0x3 ] = TempXA_Buffer [ ( CurrentSample << 1 ) + 0 ];
			BufferR [ CurrentSample & 0x3 ] = TempXA_Buffer [ ( CurrentSample << 1 ) + 1 ];
		}
	}
	
	// update write index for spu buffer
	// 2 bytes hold one 16-bit sample
	//SpuBuffer_WriteIndex += ( c_iXADecode_Size / 2 );
	SpuBuffer_WriteIndex += i;

	if ( ( SpuBuffer_ReadIndex + c_iSpuBuffer_Size ) < SpuBuffer_WriteIndex )
	{
		SpuBuffer_ReadIndex = SpuBuffer_WriteIndex - c_iSpuBuffer_Size;
	}
	
#ifdef INLINE_DEBUG_RUN
	debug << "; XA_SAMPLE_COUNT=" << dec << i << "; CurrentSample=" << CurrentSample;
#endif
}



void CD::DebugWindow_Enable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	const char* DebugWindow_Caption = "PS1 CD Debug Window";
	static constexpr int DebugWindow_X = 10;
	static constexpr int DebugWindow_Y = 10;
	static constexpr int DebugWindow_Width = 200;
	static constexpr int DebugWindow_Height = 200;
	
	static constexpr int List_X = 0;
	static constexpr int List_Y = 0;
	static constexpr int List_Width = 150;
	static constexpr int List_Height = 180;
	
	int i;
	stringstream ss;
	
	if ( !DebugWindow_Enabled )
	{
		// create the main debug window
		DebugWindow = new WindowClass::Window ();
		DebugWindow->Create ( DebugWindow_Caption, DebugWindow_X, DebugWindow_Y, DebugWindow_Width, DebugWindow_Height );
		DebugWindow->DisableCloseButton ();
		
		// create "value lists"
		ValueList = new DebugValueList<u32> ();
		ValueList->Create ( DebugWindow, List_X, List_Y, List_Width, List_Height, true, false );
		
		_CD->UpdateREG_InterruptFlag ();
		_CD->UpdateREG_ModeStatus ();

		ValueList->AddVariable ( "MSTAT", (u32*) & _CD->REG_ModeStatus );
		ValueList->AddVariable ( "IENABLE", (u32*) & _CD->REG_InterruptEnable );
		ValueList->AddVariable ( "IFLAG", (u32*) & _CD->REG_InterruptFlag );
		
		// mark debug as enabled now
		DebugWindow_Enabled = true;
		
		// update the value lists
		DebugWindow_Update ();
	}
	
#endif

}

void CD::DebugWindow_Disable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;
	
	if ( DebugWindow_Enabled )
	{
		delete DebugWindow;
		delete ValueList;
	
		// disable debug window
		DebugWindow_Enabled = false;
	}
	
#endif

}

void CD::DebugWindow_Update ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;
	
	if ( DebugWindow_Enabled )
	{
		ValueList->Update();
	}
	
#endif

}

