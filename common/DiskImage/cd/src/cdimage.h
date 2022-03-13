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



#ifndef _CDIMAGE_H_
#define _CDIMAGE_H_


#include <stdio.h>
#include <fstream>

#include "types.h"
#include "WinFile.h"
#include "GNUThreading_x64.h"

#include "WinApiHandler.h"

#include "StringUtils.h"

namespace DiskImage
{

	class CDImage
	{
	public:
	
		static CDImage* _DISKIMAGE;
	
	
		// standard 74-minute CD contains 333,000 blocks or "sectors"
		
		// each sector is ALWAYS 2,352 bytes which has 2048 bytes of PC (mode1) data, 2,336 bytes of PSX/VCD (mode2) data, or 2,352 bytes of audio
		
		// each sector is divided into 98 24-byte frames
		
		// a 1x speed cd drive reads 75 consecutive sectors per second
		
		// the playing time of a CD is 74 minutes, or 4,440 seconds
		
		// this makes for 333000/4440 = 75 sectors/second for a CD
		// this makes for 333000/74 = 4500 sectors/minute for a CD
		
		// sector 0 starts at 2 seconds
		// frac is the same as sector - PS1 only lets you specify up to the sector
		
		// 96 subcode bytes per sector - not included in 2352
		// subcode in 4 packets of 24 bytes a piece = 96 bytes for subcode per sector
		
		static const u32 c_FrameSize = 24;
		static const u32 c_SectorSize = 2352;
		static const u32 c_SubChannelSizePerSector = 96;
		
		static const u32 c_SectorsPerMinute = 4500;
		static const u32 c_SectorsPerSecond = 75;
		
		static const u32 c_FramesPerSector = 98;
		
		static const u32 c_SectorsInFirstTwoSeconds = c_SectorsPerSecond * 2;
		static const u32 c_SectorsInFirstMinute = c_SectorsPerMinute - c_SectorsInFirstTwoSeconds;
		
		
		// dvd constants
		
		// the maximum sector size of a dvd sector
		static const int c_iDvdFullSectorSize = 2064;
		
		class Sector
		{
		public:

			// each sector has 96 bytes of sub-channel data
			//  this particular structure is from pcsx source code
			struct SubQ {
			
				// Sub-Channel P (Pause bits. Usually either all set or all cleared)
				unsigned char SubChannelP[12];
				
				// Sub-Channel Q
				unsigned char ControlAndADR;
				unsigned char TrackNumber;
				unsigned char IndexNumber;
				unsigned char TrackRelativeAddress[3];
				unsigned char Filler;
				unsigned char AbsoluteAddress[3];
				unsigned char CRC[2];
				
				// Sub-Channels R-W (Can be used for CD-TEXT)
				char res1[72];
			};

			// contents of SubMode byte
			union SubMode_Contents
			{
				struct
				{
					// bit 0
					u8 EOR : 1;
					
					// bit 1
					u8 VIDEO : 1;
					
					// bit 2 - whether or not audio data
					u8 AUDIO : 1;
					
					// bit 3
					u8 DATA : 1;
					
					// bit 4
					u8 TRIGGER : 1;
					
					// bit 5
					u8 FORM : 1;
					
					// bit 6
					u8 REALTIME : 1;
					
					// bit 7 - if you have reached end of data
					u8 _EOF : 1;
				};
				
				u8 SubMode;
			};

			// this one is for audio data
			struct Mode0
			{
				// an audio sector does not have a header or anything
				u8 Data [ 2352 ];
			};
			
			struct Mode1
			{
				// this identifies the start of a sector
				u8 Sync [ 12 ];

				// this is the header, which just has Minutes, Seconds, Sector, Mode (1,2,etc)
				union
				{
					u32 Header;
					
					struct
					{
						// these are in BCD Format!!
						u8 Minutes;
						u8 Seconds;
						u8 Sector;
						u8 Mode;
					};
				};
				
				// this is the actual user data
				u8 Data [ 2048 ];

				// error detecting code
				u32 EDC;
				
				// this is zero and unused
				u64 zero;
				
				// error-correcting codes
				u8 ECC [ 276 ];
				
			};
			
			struct Mode2_2328
			{
				// this identifies the start of a sector
				u8 Sync [ 12 ];

				// this is the header, which just has Minutes, Seconds, Sector, Mode (1,2,etc)
				union
				{
					u32 Header;
					
					struct
					{
						// these are in BCD Format!!
						u8 Minutes;
						u8 Seconds;
						u8 Sector;
						u8 Mode;
					};
				};
				
				union
				{
					struct
					{
						u8  File;
						u8  Chan;
						u8  Submode;
						u8  Coding;

						u8  File2;
						u8  Chan2;
						u8  Submode2;
						u8  Coding2;
					};
					
					// for mode 2, there is also a sub-header
					u64 SubHeader;
				};
				
				// this is the actual user data
				u8 Data [ 2048 ];
				
				u32 EDC;
				
				u8 ECC [ 276 ];
				
			};
			
			struct Mode2_2328_Form2
			{
				// this identifies the start of a sector
				u8 Sync [ 12 ];

				// this is the header, which just has Minutes, Seconds, Sector, Mode (1,2,etc)
				union
				{
					u32 Header;
					
					struct
					{
						// these are in BCD Format!!
						u8 Minutes;
						u8 Seconds;
						u8 Sector;
						u8 Mode;
					};
				};
				
				union
				{
					struct
					{
						u8  File;
						u8  Chan;
						u8  Submode;
						u8  Coding;

						u8  File2;
						u8  Chan2;
						u8  Submode2;
						u8  Coding2;
					};
					
					// for mode 2, there is also a sub-header
					u64 SubHeader;
				};
				
				// this is the actual user data
				u8 Data [ 2324 ];
				
				u32 EDC;
				
			};
			
			// this must be a psx only mode or something - cuz the sector is only 2352 bytes if you don't count subcode data
			struct Mode2_2340
			{
				// this identifies the start of a sector
				u8 Sync [ 12 ];

				// this is the user data
				u8 Data [ 2340 ];
			};
			
		};
		
		// these are static since you can only put one disc in a ps1/ps2 at a time and will need to use callback function for asychronous file reading
		// c_BufferSectorCount needs to be a power of 2 and cannot be 2
#ifdef PS2_COMPILE
		// ps2 reads lots of sectors and faster, so needs a MUCH bigger buffer or you get speed issues
		// ps1 on the other hand only reads one sector at a time and slower
		static const u32 c_BufferSectorCount = 128;
#else
		static const u32 c_BufferSectorCount = 32;
#endif
		
		static const u32 c_BufferSectorMask = c_BufferSectorCount - 1;
		
		// this is the number parts the buffer is split into
		static const u32 c_BufferPartitionCount = 2;
		
		// this is the number of sectors to read at a time
		static const u32 c_SectorReadCount = c_BufferSectorCount / c_BufferPartitionCount;
		static const u32 c_SectorReadCountMask = c_SectorReadCount - 1;
		
		bool isReadingFirstSector;
		s64 ReadIndex, Next_ReadIndex;
		s64 WriteIndex;
		u64 NumberOfSectorsToWrite;
		u8 Buffer [ c_SectorSize * c_BufferSectorCount ];
		u8 SubBuffer [ c_SubChannelSizePerSector * c_BufferSectorCount ];
		
		static ifstream* CueFile;
		static ifstream* CcdFile;
		
		
		u32 ImageType;
		enum { IMAGETYPE_BIN = 0, IMAGETYPE_IMG = 1, IMAGETYPE_DVD = 2 };
		
		// need a place to store pointer offset into current subchannel data
		u8* CurrentSubBuffer;
		
		// number of the next sector to be read
		// needs to be static since it needs to seek once data has been read
		// make sure that read is not in progress before performing a seektime
		u64 NextSector;
		u32 CurrentSector, Next_CurrentSector;
		
		// the sector at which the next track starts at
		u32 CurrentTrack, NextTrack, NextTrack_Sector;
		
		
		// the sector number for the last externally seeked sector
		//s64 SectorOffset;
	
		// determines if a file read is in progress in the background for this particular object
		static volatile u32 isReadInProgress;
		static volatile u32 isSubReadInProgress;
		
		static unsigned long isDiskOpen;
		
		// maximum number of tracks on a CD disk
		static const int c_iMaxTracks = 101;
		
		// c++ standard files aren't working properly in g++ for some reason
		//FILE *image;
		static WinApi::File image;
		static WinApi::File sub;
		static bool isSubOpen;
		
		static const unsigned long c_MaxPathLength = 2048;
		char ImagePath [ c_MaxPathLength ];
		char SubPath [ c_MaxPathLength ];
		
		
		struct ReadAsync_Params
		{
			char* DataOut;
			unsigned long BytesToRead;
			unsigned long long SeekPosition;
			void* Callback_Function;
		};
		
		static ReadAsync_Params _rap_params;
		
		// the current position
		u8 Current_AMin, Current_ASec, Current_AFrac;
		
		struct _Index
		{
			// note: if index is zero, then start counting down, otherwise start counting up
			u8 Track, Index, Min, Sec, Frac, AMin, ASec, AFrac;
			u32 SectorNumber;
			
			// for bin files, there can be a pregap in the cue file that is removed from the image completely
			// so for areas that are not on the image, this will be -1
			u32 SectorNumber_InImage;
			
			// need to know what file has the track data
			string sTrackFileName;
		};
		
		// start of layer 1
		u32 layer1start;
		
		// the index corresponds with the track that data is for, so this means that index zero is not used
		int iNumberOfTracks;
		_Index TrackData [ 101 ];
		
		u64 SizeOfImage, SectorsInImage;
		int iNumberOfIndexes;
		int iPreGap;
		_Index IndexData [ 256 ];
		
		u64 LastSector_Number;
		
		char DiskSerial [ 16 ];
		
		u64 SectorSize;
		
		// if disk is detected as being a CD during load, then this is set to TRUE
		bool bIsDiskCD;
		
		// need to know if it is a cue with multiple file sources
		bool bIsCueMultiFile;
		
		// need the path to the program folder
		static string sProgramPath;
		static char sProgPathTemp [ 2048 ];
		static string sCDPath;
		
		// need to be able to get the track number for a particular disk location
		int FindTrack ( int AMin, int ASec, int AFrac );
		int FindTrack ( u32 SectorNumber );
		void GetTrackStart ( int TrackNumber, unsigned char & AMin, unsigned char & ASec, unsigned char & AFrac );
		
		// constructor
		CDImage ();
		
		// destructor
		// needs to remove any temp files used
		~CDImage ();
		
		// open a disk image for reading
		// returns true if opened successfully, returns false otherwise
		// will only support .bin/.img/.iso/.sub/.cue/.ccd
		bool OpenDiskImage ( string DiskImagePath, u32 DiskSectorSize = c_SectorSize );
		
		// close the currently open disk image
		// returns true if closed successfully, returns false otherwise
		bool CloseDiskImage ();

		// parse cue/ccd files
		bool ParseCueSheet ( string FilePath );
		bool ParseCcdSheet ( string FilePath );
		
		// seeks to time and sector offset
		// time values are just regular numbers, NOT bcd
		bool SeekTime ( s32 Minutes, s32 Seconds, s32 Sector );
		
		// seeks to the absolute sector number
		void SeekSector ( u64 Sector );
		
		// reads just the data in sector and returns the number of bytes read, up to 2352 bytes
		//int ReadData ( u8* Data );
		
		// must call this once before performing calls to ReadNextSector
		// initializes the streaming from disk
		void StartReading ();
		
		// reads entire 2352-byte sector
		// will now need to return a pointer to the new buffer with the data, since after testing sectors will need to be pre-read asynchronously
		u8* ReadNextSector ();
		
		
		// sets the current sector as the first sector
		void SetFirstSector ();
		
		// gets the index for the current sector
		u64 GetCurrentBufferIndex ();
		
		// these functions will get pointers to the desired Data/Sub buffer so the data does not need to be copied
		u8* GetDataBuffer ( u64 Index );
		u8* GetCurrentDataBuffer ();
		
		// this should probably be just the last read sub buffer
		u8* GetCurrentSubBuffer ();
		
		// returns true if it is ok to read sector from buffer, false otherwise
		// reads will happen asynchronously, so this must be checked
		void WaitForSectorReadComplete ();
		
		// waits for all reads to complete
		void WaitForAllReadsComplete ();

		// used to open disk image on the gui thread, where it will be read from asynchronously
		bool _RemoteCall_OpenDiskImage ( string FullImagePath );

		// used to perform async reads remotely on the gui thread
		static void _RemoteCall_ReadAsync ();

		// call back function for asynchronous disk reading
		static void DiskRead_Callback ();
		static void SubRead_Callback ();
		
		static u32 ConvertBCDToDec ( u8 BCD );
		static u8 ConvertDecToBCD8 ( u32 Dec );
		
		static unsigned long GetSectorNumber ( u32 Min, u32 Sec, u32 Frac );
		static void SplitSectorNumber ( unsigned long SectorNumber, u8& Min, u8& Sec, u8& Frac );
		
		// will also need some more functions
		
		// will output all of the index data from .cue or .ccd file to console
		void Output_IndexData ();
		void Output_SubQData ( u32 AMin, u32 ASec, u32 AFrac );
		
		// get the index into the index data for the specified sector number
		u32 GetIndexData_Index ( u32 SectorNumber );
		
		// gets the sector number of where the absolute disk location specified can be found in the actual disk image
		// returns -1 when it is not in the disk image file and should be zeroed out
		u32 GetSectorNumber_InImage ( u32 AMin, u32 ASec, u32 AFrac );
		u32 GetSectorNumber_InImage ( u32 SectorNumber );
		
		// get track/index from subq for the current position
		// uses non-real values if the SUBQ file is not available or disabled
		u8 SubQ_Index, SubQ_Track, SubQ_AMin, SubQ_ASec, SubQ_AFrac, SubQ_Min, SubQ_Sec, SubQ_Frac;
		void UpdateSubQ_Data ( u32 AMin, u32 ASec, u32 AFrac );
		void UpdateSubQ_Data ();
		void UpdateSubQ_Data ( u32 SectorNumber );
		
	};

}

#endif


