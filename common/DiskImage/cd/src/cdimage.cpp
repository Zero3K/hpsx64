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


#include "string.h"
#include "cdimage.h"
#include "ConfigFile.h"

#include <sys/stat.h>


using namespace DiskImage;
using namespace x64ThreadSafe::Utilities;

using namespace Utilities::Strings;


CDImage* CDImage::_DISKIMAGE;
WinApi::File CDImage::image;
CDImage::ReadAsync_Params CDImage::_rap_params;
WinApi::File CDImage::sub;
bool CDImage::isSubOpen;
unsigned long CDImage::isDiskOpen;
volatile u32 CDImage::isReadInProgress;
volatile u32 CDImage::isSubReadInProgress;

ifstream* CDImage::CueFile;
ifstream* CDImage::CcdFile;

char CDImage::sProgPathTemp [ 2048 ];
string CDImage::sProgramPath;
string CDImage::sCDPath;


// need to be able to toggle between reading synchronously and asynchronously
//#define DISK_READ_SYNC
#define DISK_READ_ASYNC


CDImage::CDImage ()
{
	cout << "\nCDImage object constructor...";
	
	isDiskOpen = false;
	isSubOpen = false;
	
	isReadInProgress = false;
	isSubReadInProgress = false;
	
	ReadIndex = -1;
	WriteIndex = 0;
	
	// get the path to the program (note: WinAPI specific)
	int len = GetModuleFileName ( NULL, sProgPathTemp, 2047 );
	sProgPathTemp [ len ] = 0;
	
	sProgramPath = GetPath ( sProgPathTemp );
	
	cout << "\nPath to program: " << sProgramPath.c_str();
	
	// set pointer to self for static functions
	_DISKIMAGE =  this;
}


CDImage::~CDImage ()
{
	string sOutputFile;
	
	CloseDiskImage ();
	
	sOutputFile = GetPath ( sProgPathTemp ) + "MultiFileCueData.bin";
	remove ( sOutputFile.c_str() );
}


bool CDImage::ParseCueSheet ( string FilePath )
{
	static const char* c_sMultiData_FileName = "MultiFileCueData.bin";
	
	string Line;
	int LineNumber = 1;
	
	int iIndex;
	int iTrack;
	int iMin_InFile, iSec_InFile, iFrac_InFile, iMin_OnDisk, iSec_OnDisk, iFrac_OnDisk;
	int iPreGap_Min, iPreGap_Sec, iPreGap_Frac;
	
	u8 iMin = 0, iSec = 0, iFrac = 0;
	
	bool bInsertPreGap;
	
	// current file that the data is in for track
	string sCurrentTrackFile;
	
	// the size of the current file
	long lCurrentTrackFile_Size;
	
	// the total of the track file sizes (so that the offsets can be added to the times provided in CUE file)
	long lTrackFileSize_Total;
	
	// count of the files in cue sheet
	int iFileCount;
	
	// the file to read from for multi-file cue
	FILE* fInputFile;
	long lInputFile_Size;
	
	// the file to write to for multi-file cue
	FILE* fOutputFile;
	
	char pTransferData [ c_SectorSize ];
	long lTransferSize;
	
	// full path and name of the output file
	string sOutputFileName;
	
	// have not found any files in cue sheet yet
	iFileCount = 0;
	
	// total of all the files with the track data starts at zero
	lTrackFileSize_Total = 0;
	
	//int PreGap = 0;
	
	// set number of tracks
	iNumberOfTracks = 0;
	
	// set number of indexes - first entry is for TOC, so start at Entry#1
	iNumberOfIndexes = 1;
	
	// don't insert any stuff for any pregaps yet
	bInsertPreGap = false;
	
	// initialize the pregap
	iPreGap = 0;
	
	CueFile = new ifstream( FilePath.c_str(), ios::binary );
	
	if ( !CueFile->is_open() || CueFile->fail() ) return false;
	
	//cout << "\nCue file is valid. Beginning to parse. FilePath=" << FilePath;
	
	// keep running until we reach end of file
	while ( CueFile->good() )
	{
		// get a line from source file
		//cout << "Getting Line#" << LineNumber << " from the input file.\n";
		getline ( *CueFile, Line );

		// remove extra spaces
		Line = Trim ( Line );
		
		// check whether this is REM, TRACK, INDEX, etc
		if ( Left ( Line, 3 ) == "REM" )
		{
			// this is a comment //
		}
		else if ( Left ( Line, 5 ) == "TRACK" )
		{
			// this is a track number //
			
			// get the track number
			iTrack = CLng ( Split ( Line, " " ) [ 1 ] );
			
			// clear track data
			TrackData [ iTrack ].Min = 0;
			TrackData [ iTrack ].Sec = 0;
			TrackData [ iTrack ].Frac = 0;
			
			// increment number of tracks
			iNumberOfTracks++;
			
			cout << "\nTrack#" << dec << iTrack;
		}
		else if ( Left ( Line, 5 ) == "INDEX" )
		{
			// this is an index number //
			
			// get index number
			iIndex = CLng ( Split ( Line, " " ) [ 1 ] );
			
			// get min:sec:frac
			Line = Split ( Line, " " ) [ 2 ];
			
			// get min
			iMin = CLng ( Split ( Line, ":" ) [ 0 ] );
			
			// get sec
			iSec = CLng ( Split ( Line, ":" ) [ 1 ] );
			
			// get frac
			iFrac = CLng ( Split ( Line, ":" ) [ 2 ] );
			
			// the values in the cue file actually give the location in the .bin file, NOT on the disk
			iMin_InFile = iMin;
			iSec_InFile = iSec;
			iFrac_InFile = iFrac;
			
			// add 2 seconds to the time
			// and add pregap time up until this point
			iSec += 2 + iPreGap;
			if ( iSec >= 60 )
			{
				iSec -= 60;
				iMin++;
			}
			
			// check if pregap data should be inserted
			if ( bInsertPreGap )
			{
				// handling pregap now
				bInsertPreGap = false;
				
				// set this as an all zeroed sector that is NOT in the .bin image
				IndexData [ iNumberOfIndexes ].SectorNumber_InImage = -1;
				
				// put in track number
				IndexData [ iNumberOfIndexes ].Track = iTrack;
				
				// put in the index number (zero)
				IndexData [ iNumberOfIndexes ].Index = 0;
				
				// should probably start at pregap time and count down relatively
				IndexData [ iNumberOfIndexes ].Min = iPreGap_Min;
				IndexData [ iNumberOfIndexes ].Sec = iPreGap_Sec;
				IndexData [ iNumberOfIndexes ].Frac = iPreGap_Frac;
				
				// put in the absolute times
				IndexData [ iNumberOfIndexes ].AMin = iMin;
				IndexData [ iNumberOfIndexes ].ASec = iSec;
				IndexData [ iNumberOfIndexes ].AFrac = iFrac;
				
				// set the absolute sector number
				IndexData [ iNumberOfIndexes ].SectorNumber =  GetSectorNumber ( iMin, iSec, iFrac );
				
				// go to next index
				iNumberOfIndexes++;
				
				// update pregap time HERE
				iPreGap += iPreGap_Sec;
				
				// add new pregap seconds to the time
				iSec += iPreGap_Sec;
				if ( iSec >= 60 )
				{
					iSec -= 60;
					iMin++;
				}
			}
			
			cout << " Min=" << (long) iMin << " Sec=" << (long) iSec << " Frac=" << (long) iFrac;
			
			// put min/sec/frac data in for track
			TrackData [ iTrack ].Min = iMin;
			TrackData [ iTrack ].Sec = iSec;
			TrackData [ iTrack ].Frac = iFrac;
			
			// put in rest of the new data
			
			// if the track number is greater than 1, and the index number is not zero, then put in index zero if not already there
			if ( iTrack > 1 && iIndex != 0 && IndexData [ iNumberOfIndexes - 1 ].Index != 0 )
			{
				u8 TMin, TSec, TFrac;
				
				// put in index zero //
				
				// set the sector to find the data at in the actual disk image
				IndexData [ iNumberOfIndexes ].SectorNumber_InImage = GetSectorNumber ( iMin_InFile, iSec_InFile, iFrac_InFile ) - 150;

				// put in track number
				IndexData [ iNumberOfIndexes ].Track = iTrack;
				
				// put in the index number (assume zero for now)
				IndexData [ iNumberOfIndexes ].Index = 0;
				
				// default of 2 minutes
				IndexData [ iNumberOfIndexes ].Min = 0;
				IndexData [ iNumberOfIndexes ].Sec = 2;
				IndexData [ iNumberOfIndexes ].Frac = 0;
				
				// put in the absolute times
				SplitSectorNumber ( GetSectorNumber ( iMin, iSec, iFrac ) - 150, TMin, TSec, TFrac );
				IndexData [ iNumberOfIndexes ].AMin = TMin;
				IndexData [ iNumberOfIndexes ].ASec = TSec;
				IndexData [ iNumberOfIndexes ].AFrac = TFrac;
				
				// set the absolute sector number
				IndexData [ iNumberOfIndexes ].SectorNumber =  GetSectorNumber ( iMin, iSec, iFrac ) - 150;
				
				// go to next index
				iNumberOfIndexes++;
			}
			
			// set the sector to find the data at in the actual disk image
			IndexData [ iNumberOfIndexes ].SectorNumber_InImage = GetSectorNumber ( iMin_InFile, iSec_InFile, iFrac_InFile );
			
			// put in track number
			IndexData [ iNumberOfIndexes ].Track = iTrack;
			
			// put in the index number (assume one for now)
			IndexData [ iNumberOfIndexes ].Index = iIndex;
			
			// should probably start at pregap time and count down relatively
			IndexData [ iNumberOfIndexes ].Min = 0;
			IndexData [ iNumberOfIndexes ].Sec = 0;
			IndexData [ iNumberOfIndexes ].Frac = 0;
			
			// put in the absolute times
			IndexData [ iNumberOfIndexes ].AMin = iMin;
			IndexData [ iNumberOfIndexes ].ASec = iSec;
			IndexData [ iNumberOfIndexes ].AFrac = iFrac;
			
			// set the absolute sector number
			IndexData [ iNumberOfIndexes ].SectorNumber =  GetSectorNumber ( iMin, iSec, iFrac );
			
			// if there are multiple source files in the CUE, then need to change some of these values
			if ( iFileCount > 1 )
			{
				// offset sector data based on the total size of the files traversed so far //
				
				IndexData [ iNumberOfIndexes ].SectorNumber_InImage = ( lTrackFileSize_Total / c_SectorSize ) + GetSectorNumber ( iMin_InFile, iSec_InFile, iFrac_InFile );
				
				SplitSectorNumber ( ( lTrackFileSize_Total / c_SectorSize ) + GetSectorNumber ( iMin, iSec, iFrac ), iMin, iSec, iFrac );
				IndexData [ iNumberOfIndexes ].AMin = iMin;
				IndexData [ iNumberOfIndexes ].ASec = iSec;
				IndexData [ iNumberOfIndexes ].AFrac = iFrac;
				
				// set the absolute sector number
				IndexData [ iNumberOfIndexes ].SectorNumber =  GetSectorNumber ( iMin, iSec, iFrac );
				
				// put min/sec/frac data in for track
				TrackData [ iTrack ].Min = iMin;
				TrackData [ iTrack ].Sec = iSec;
				TrackData [ iTrack ].Frac = iFrac;
			}
			
			// if index zero was provided, then need to update the starting time //
			if ( ( IndexData [ iNumberOfIndexes - 1 ].Index == 0 ) && ( IndexData [ iNumberOfIndexes ].Index == 1 ) && ( iTrack > 1 ) )
			{
				// correct the start time for index 0 //
				
				long Index0_Sector, Index1_Sector;
				u8 cMin = 0, cSec = 0, cFrac = 0;
				
				Index0_Sector = GetSectorNumber ( IndexData [ iNumberOfIndexes - 1 ].AMin, IndexData [ iNumberOfIndexes - 1 ].ASec, IndexData [ iNumberOfIndexes - 1 ].AFrac );
				Index1_Sector = GetSectorNumber ( IndexData [ iNumberOfIndexes ].AMin, IndexData [ iNumberOfIndexes ].ASec, IndexData [ iNumberOfIndexes ].AFrac );
				
				// get the difference between index1 and index0 time
				SplitSectorNumber ( Index1_Sector - Index0_Sector, cMin, cSec, cFrac );
				
				// set index0 start time with the difference
				IndexData [ iNumberOfIndexes - 1 ].Min = cMin;
				IndexData [ iNumberOfIndexes - 1 ].Sec = cSec;
				IndexData [ iNumberOfIndexes - 1 ].Frac = cFrac;
			}
			
			// go to next index
			iNumberOfIndexes++;
		}
		else if ( Left ( Line, 6 ) == "PREGAP" )
		{
			// this is a pregap //
			
			// insert pregap when processing the next index
			bInsertPreGap = true;
			
			// get min:sec:frac
			Line = Split ( Line, " " ) [ 1 ];
			
			// get min
			iPreGap_Min = CLng ( Split ( Line, ":" ) [ 0 ] );
			
			// get sec
			iPreGap_Sec = CLng ( Split ( Line, ":" ) [ 1 ] );
			
			// get frac
			iPreGap_Frac = CLng ( Split ( Line, ":" ) [ 2 ] );
			
			cout << " PreGap_Min=" << iPreGap_Min << " PreGap_Sec=" << iPreGap_Sec << " PreGap_Frac=" << iPreGap_Frac;
			
			// *** todo *** add pregap into list of tracks, etc
		}
		else if ( Left ( Line, 4 ) == "FILE" )
		{
			// this is a FILE statement //
			
			cout << "\nFile Count: " << iFileCount;
			
			// check if we are just finding out this is a multi-file cue sheet
			if ( iFileCount == 1 )
			{
				// there is more than one file in the CUE sheet //
				
				cout << "\nOpening multi data file.";
				
				sOutputFileName = GetPath ( sProgPathTemp ) + c_sMultiData_FileName;
				
				cout << "\nPath to program: " << sProgramPath.c_str ();
				cout << "\nPath to program (from array): " << sProgPathTemp;
				cout << "\nCreating multi data file at: " << sOutputFileName.c_str();
				
				// set the name of the file for CD
				sCDPath = sOutputFileName;

				// create a combined file to dump all the CD data into
				fOutputFile = fopen ( sOutputFileName.c_str(), "wb+" );
			}
			
			// check if the current file's data needs to be dumped
			if ( iFileCount )
			{
				// get the size of the input file
				int rc;
				struct stat stat_buf;
				rc = stat ( sCurrentTrackFile.c_str(), &stat_buf );
				
				// check if we successfully have file size or not
				if ( rc )
				{
					cout << "\nProblem getting size of file: " << sCurrentTrackFile.c_str();
					return false;
				}
				
				// retrieve file size
				lInputFile_Size = stat_buf.st_size;
				
				// now take the size of the file and add it to the sector offset for parse
				lTrackFileSize_Total += lInputFile_Size;
				
				cout << "\nSize of input file is: " << lInputFile_Size;
				cout << " Number Of Sectors= " << dec << ( lInputFile_Size / c_SectorSize );
				cout << " Total Sectors: " << dec << ( lTrackFileSize_Total / c_SectorSize );
				
				// open the input file
				fInputFile = fopen ( sCurrentTrackFile.c_str(), "rb" );
				
				if ( !fInputFile )
				{
					cout << "\nError opening input file: " << sCurrentTrackFile.c_str();
				}
				
				// transfer data from input file into the combined CD data file sector by sector
				for ( int i = 0; i < ( lInputFile_Size / c_SectorSize ); i++ )
				{
					lTransferSize = fread ( pTransferData, 1, c_SectorSize, fInputFile );
					
					if ( lTransferSize != c_SectorSize )
					{
						cout << "\nError during READ of CUE CD data. i=" << i << " num sectors=" << ( lInputFile_Size / c_SectorSize ) << " lTransferSize=" << lTransferSize;
						return false;
					}
					
					lTransferSize = fwrite ( pTransferData, 1, c_SectorSize, fOutputFile );
					
					if ( lTransferSize != c_SectorSize )
					{
						cout << "\nError during WRITE of CUE CD data. i=" << i;
						return false;
					}
				}
				
				if ( fclose ( fInputFile ) )
				{
					cout << "\nProblem closing the input file: " << sCurrentTrackFile.c_str();
					return false;
				}
			}
			
			// ***TODO*** need to get the file name here
			cout << "\n FILE: " << Line.c_str() << "\n";
			
			// get the file name
			//TrackData [ iTrack ].sTrackFileName = Mid ( Line, InStr ( Line, "\"") + 1, InStrRev ( Line, "\"") - InStr ( Line, "\"") - 1 );
			sCurrentTrackFile = GetPath ( FilePath ) + Mid ( Line, InStr ( Line, "\"") + 1, InStrRev ( Line, "\"") - InStr ( Line, "\"") - 1 );
			
			cout << "\n The filename is: " << sCurrentTrackFile.c_str();
			
			if ( !iFileCount )
			{
				// check if file exists
				struct stat buffer;
				if (stat (sCurrentTrackFile.c_str(), &buffer) == 0)
				{
					// set the name of the file for CD
					sCDPath = sCurrentTrackFile;
				}
				else
				{
					// unable to find the file specified, but no problem yet //
					cout << "\nALERT: CDIMAGE: unable to find the file specified in CUE sheet: " << sCurrentTrackFile.c_str();
				}
				
			}
			
			// need to open the file to lock it
			
			
			// add the size of the previous track file to the total
			//lTrackFileSize_Total += lCurrentTrackFile_Size;
			
			
			// found another file in the cue sheet
			iFileCount++;
		}
		
	}
	
	
	if ( iFileCount > 1 )
	{
		// finalize the temp file for cue sheet with multiple files //
		
		// is a cue with more than one file
		bIsCueMultiFile = true;
		
		int rc;
		struct stat stat_buf;
		rc = stat ( sCurrentTrackFile.c_str(), &stat_buf );
		
		// check if we successfully have file size or not
		if ( rc )
		{
			cout << "\nProblem getting size of file: " << sCurrentTrackFile.c_str();
			return false;
		}
		
		// retrieve file size
		lInputFile_Size = stat_buf.st_size;
		
		// now take the size of the file and add it to the sector offset for parse
		lTrackFileSize_Total += lInputFile_Size;
		
		cout << "\nSize of input file is: " << lInputFile_Size;
		
		// open the input file
		fInputFile = fopen ( sCurrentTrackFile.c_str(), "rb" );
		
		if ( !fInputFile )
		{
			cout << "\nError opening input file: " << sCurrentTrackFile.c_str();
		}
		
		// transfer data from input file into the combined CD data file sector by sector
		for ( int i = 0; i < ( lInputFile_Size / c_SectorSize ); i++ )
		{
			lTransferSize = fread ( pTransferData, 1, c_SectorSize, fInputFile );
			
			if ( lTransferSize != c_SectorSize )
			{
				cout << "\nError during READ of CUE CD data. i=" << i << " num sectors=" << ( lInputFile_Size / c_SectorSize ) << " lTransferSize=" << lTransferSize;
				return false;
			}
			
			lTransferSize = fwrite ( pTransferData, 1, c_SectorSize, fOutputFile );
			
			if ( lTransferSize != c_SectorSize )
			{
				cout << "\nError during WRITE of CUE CD data. i=" << i;
				return false;
			}
		}
		
		if ( fclose ( fInputFile ) )
		{
			cout << "\nProblem closing the input file: " << sCurrentTrackFile.c_str();
			return false;
		}
		
		if ( fclose ( fOutputFile ) )
		{
			cout << "\nProblem closing the input file: " << c_sMultiData_FileName;
			return false;
		}
	}
	
	
	if ( !iNumberOfTracks )
	{
		cout << "\nhps1x64 ERROR: CDImage has zero tracks. Error in CUE file.\n";
	}
	
	CueFile->close ();
	return true;
}


bool CDImage::ParseCcdSheet ( string FilePath )
{
	string Line;
	int LineNumber = 1;
	
	int iTrack, iMin, iSec, iFrac, iIndex, iNextIndex, iSector;
	
	// set number of tracks
	iNumberOfTracks = 0;
	
	// set number of indexes
	iNumberOfIndexes = 1;
	
	CcdFile = new ifstream( FilePath.c_str(), ios::binary );
	
	if ( !CcdFile->is_open() || CcdFile->fail() ) return false;
	
	//cout << "\nCcd file is valid. Beginning to parse. FilePath=" << FilePath;
	
	
	// keep running until we reach end of file
	while ( CcdFile->good() )
	{
		// get a line from source file
		//cout << "Getting Line#" << LineNumber << " from the input file.\n";
		getline ( *CcdFile, Line );

		// remove extra spaces
		Line = Trim ( Line );
		
		// check whether this is REM, TRACK, INDEX, etc
		/*
		if ( Left ( Line, 3 ) == "REM" )
		{
			// this is a comment //
		}
		else
		*/
		
		if ( Left ( Line, 6 ) == "[TRACK" )
		{
			// this is a track number //
			
			// get the track number
			iTrack = CLng ( Replace ( Split ( Line, " " ) [ 1 ], "]", "" ) );
			
			// clear track data
			TrackData [ iTrack ].Min = 0;
			TrackData [ iTrack ].Sec = 0;
			TrackData [ iTrack ].Frac = 0;
			
			// did not get index yet
			iNextIndex = 1;
			
			// increment number of tracks
			iNumberOfTracks++;
			
			cout << "\nTrack#" << dec << iTrack;
		}
		else if ( Left ( Line, 5 ) == "INDEX" )
		{
			// this is an index number //
			
			// only get first index listed for now
			// actually, just get the last one for PS1
			//if ( iNextIndex == 1 )
			//{
				// get index#=sector
				Line = Split ( Line, " " ) [ 1 ];
				
				// get index??
				iIndex = CLng ( Split ( Line, "=" ) [ 0 ] ) /*+ 1*/;
				
				// get min:sec:frac
				//Line = Split ( Line, " " ) [ 2 ];
				// get sector
				iSector = CLng ( Split ( Line, "=" ) [ 1 ] );
				
				// get min
				//iMin = CLng ( Split ( Line, ":" ) [ 0 ] );
				iMin = iSector / c_SectorsPerMinute;
				
				// get sec
				//iSec = CLng ( Split ( Line, ":" ) [ 1 ] );
				iSec = ( iSector % c_SectorsPerMinute ) / c_SectorsPerSecond;
				
				// get frac
				//iFrac = CLng ( Split ( Line, ":" ) [ 2 ] );
				iFrac = iSector % c_SectorsPerSecond;
				
				cout << " Index#" << iIndex << " Sector=" << iSector << " Min=" << iMin << " Sec=" << iSec << " Frac=" << iFrac;
				
				// add 2 seconds to the time
				iSec += 2;
				if ( iSec >= 60 )
				{
					iSec -= 60;
					iMin++;
				}
			
				// put min/sec/frac data in for track
				TrackData [ iTrack ].Min = iMin;
				TrackData [ iTrack ].Sec = iSec;
				TrackData [ iTrack ].Frac = iFrac;
				
				// get next index
				iNextIndex++;
				
				// now put in the new stuff //
				
				// if the track number is greater than 1, and the index number is not zero, then put in index zero if not already there
				if ( iTrack > 1 && iIndex != 0 && IndexData [ iNumberOfIndexes - 1 ].Index != 0 )
				{
					u8 TMin, TSec, TFrac;
					
					// put in index zero //
					
					// set the sector to find the data at in the actual disk image
					IndexData [ iNumberOfIndexes ].SectorNumber_InImage = GetSectorNumber ( iMin, iSec, iFrac ) - 150;

					// put in track number
					IndexData [ iNumberOfIndexes ].Track = iTrack;
					
					// put in the index number (assume zero for now)
					IndexData [ iNumberOfIndexes ].Index = 0;
					
					// default of 2 minutes
					IndexData [ iNumberOfIndexes ].Min = 0;
					IndexData [ iNumberOfIndexes ].Sec = 2;
					IndexData [ iNumberOfIndexes ].Frac = 0;
					
					// put in the absolute times
					SplitSectorNumber ( GetSectorNumber ( iMin, iSec, iFrac ) - 150, TMin, TSec, TFrac );
					IndexData [ iNumberOfIndexes ].AMin = TMin;
					IndexData [ iNumberOfIndexes ].ASec = TSec;
					IndexData [ iNumberOfIndexes ].AFrac = TFrac;
					
					// set the absolute sector number
					IndexData [ iNumberOfIndexes ].SectorNumber =  GetSectorNumber ( iMin, iSec, iFrac ) - 150;
					
					// go to next index
					iNumberOfIndexes++;
				}
				
				// put in the index/track
				IndexData [ iNumberOfIndexes ].Index = iIndex;
				IndexData [ iNumberOfIndexes ].Track = iTrack;
				
				// put in the relative time values
				if ( !iIndex )
				{
					// assume two seconds for now
					IndexData [ iNumberOfIndexes ].Min = 0;
					IndexData [ iNumberOfIndexes ].Sec = 2;
					IndexData [ iNumberOfIndexes ].Frac = 0;
				}
				else
				{
					IndexData [ iNumberOfIndexes ].Min = 0;
					IndexData [ iNumberOfIndexes ].Sec = 0;
					IndexData [ iNumberOfIndexes ].Frac = 0;
				}
				
				// put in the absolute time values
				IndexData [ iNumberOfIndexes ].AMin = iMin;
				IndexData [ iNumberOfIndexes ].ASec = iSec;
				IndexData [ iNumberOfIndexes ].AFrac = iFrac;
				
				// put in the sector this is found at in disk image
				IndexData [ iNumberOfIndexes ].SectorNumber_InImage = iSector;
				
				// put in the sector number on the actual physical disk
				IndexData [ iNumberOfIndexes ].SectorNumber = GetSectorNumber ( iMin, iSec, iFrac );
				
				// go to next index
				iNumberOfIndexes++;
				
			//}
			
		}
		
	}
	
	if ( !iNumberOfTracks )
	{
		cout << "\nhps1x64 ERROR: CDImage has zero tracks. Error in CCD file.\n";
	}

	CcdFile->close ();
	return true;
}


int CDImage::FindTrack ( int AMin, int ASec, int AFrac )
{
	int i;
	u32 SectorNumber;
	
	SectorNumber = GetSectorNumber ( AMin, ASec, AFrac );
	i = GetIndexData_Index ( SectorNumber );
	return IndexData [ i ].Track;
	
	/*
	for ( i = 1; i <= iNumberOfTracks; i++ )
	{
		if ( AMin <= TrackData [ i ].Min )
		{
			if ( AMin < TrackData [ i ].Min ) break;
			
			// TrackData [ i ].Min == AMin //
			if ( ASec <= TrackData [ i ].Sec )
			{
				if ( ASec < TrackData [ i ].Sec ) break;
				
				// TrackData [ i ].Sec == ASec //
				if ( AFrac < TrackData [ i ].Frac ) break;
			}
		}
	}
	
	return i - 1;
	*/
}



int CDImage::FindTrack ( u32 SectorNumber )
{
	int i;
	//u32 SectorNumber;
	
	//SectorNumber = GetSectorNumber ( AMin, ASec, AFrac );
	i = GetIndexData_Index ( SectorNumber );
	return IndexData [ i ].Track;
}



// sets all -1 on error
void CDImage::GetTrackStart ( int TrackNumber, unsigned char & AMin, unsigned char & ASec, unsigned char & AFrac )
{
	int i;
	
	for ( i = iNumberOfIndexes; i >= 0; i-- )
	{
		if ( TrackNumber == IndexData [ i ].Track && IndexData [ i ].Index == 1 )
		{
			AMin = IndexData [ i ].AMin;
			ASec = IndexData [ i ].ASec;
			AFrac = IndexData [ i ].AFrac;
			return;
		}
	}
	
	AMin = -1;
	ASec = -1;
	AFrac = -1;
	
	//Min = TrackData [ TrackNumber ].Min;
	//Sec = TrackData [ TrackNumber ].Sec;
	//Frac = TrackData [ TrackNumber ].Frac;
}


bool CDImage::OpenDiskImage ( string DiskImagePath, u32 DiskSectorSize )
{
//cout << "\nCalling CDImage::OpenDiskImage; DiskImagePath=" << DiskImagePath.c_str();
	
	bool bDiskOpenedSuccessfully;
	
	string CueFile_Path;
	string CcdFile_Path;
	string SubFile_Path;
	
	bool ret;
	
	_DISKIMAGE = this;
	
	// set the sector size
	SectorSize = DiskSectorSize;
	
	// get path to sub file
	SubFile_Path = GetPath ( DiskImagePath ) + GetFile ( DiskImagePath ) + ".sub";
	
	// copy path
	strcpy ( ImagePath, DiskImagePath.c_str() );
	strcpy ( SubPath, SubFile_Path.c_str () );
	
	//cout << "\nImagePath=" << ImagePath;
	
	//asm volatile ( "mfence" );
	
	isDiskOpen = false;
	//Lock_Exchange32 ( (long&)isDiskOpen, false );
	
	// file cannot be accessed while it is opening, and we also need to send data to other thread
	//Lock_Exchange32 ( (long&)isReadInProgress, true );
	isReadInProgress = true;
	
	// determine if disk is CD
	bIsDiskCD = Config::PSXDiskUtility::isDataCD ( DiskImagePath.c_str () );
	
	if ( LCase ( GetExtension ( DiskImagePath ) ) == ".cue" ) bIsDiskCD = true;
	if ( LCase ( GetExtension ( DiskImagePath ) ) == ".ccd" ) bIsDiskCD = true;
	
	// set the sector size based on whether CD or DVD
	if ( bIsDiskCD )
	{
		// CD disk was detected //
		
		cout << "\nCDImage::OpenDiskImage: CD was detected. Setting sector size to 2352";
		
		SectorSize = 2352;
	}
	else
	{
		// DVD disk was detected //
		
		cout << "\nCDImage::OpenDiskImage: DVD was detected. Setting sector size to 2048";
		
		SectorSize = 2048;
	}
	
	// first load the disk serial
	// ***todo*** sector size should be variable since this should work for DVDs too
	// assume this is a cd for now, will add disk check later
	ret = Config::PSXDiskUtility::GetPSXIDString ( DiskSerial, DiskImagePath.c_str (), SectorSize );
	
	if ( !ret )
	{
		cout << "\n***ERROR*** DiskImage: There was a problem obtaining disk serial. May not be a PSX disk.";
	}
	else
	{
		cout << "\nDiskImage: Disk Serial=" << DiskSerial;
	}

	// get layer 1 start
	layer1start = Config::PSXDiskUtility::GetLayer1Start ( NULL, DiskImagePath.c_str (), SectorSize );
	
	// disk image probably needs to be opened on gui thread or else the handle is invalid
	//image = fopen64 ( DiskImagePath, "r" );
	//image = fopen ( DiskImagePath, "rb" );
	//if ( !image.Create ( DiskImagePath, OPEN_EXISTING, GENERIC_READ, NULL ) ) return false;
	//if ( !image.CreateAsync ( DiskImagePath ) ) return false;
	//bDiskOpenedSuccessfully = WindowClass::Window::RemoteCall ( (WindowClass::Window::RemoteFunction) _RemoteCall_OpenDiskImage, (void*) NULL, true );
	bDiskOpenedSuccessfully = _RemoteCall_OpenDiskImage ( DiskImagePath );
	
	// now try to open file with subchannel data
	//WindowClass::Window::RemoteCall ( (WindowClass::Window::RemoteFunction) _RemoteCall_OpenSubImage, (void*) NULL, true );
	
	/*
	// get just path and file name and add cue extension
	CueFile_Path = GetPath ( DiskImagePath ) + GetFile ( DiskImagePath ) + ".cue";
	
	if ( !ParseCueSheet ( CueFile_Path.c_str() ) )
	{
		cout << "\nCue file not found or other error. Should be: " << CueFile_Path.c_str();
		
		// get just path and file name and add ccd extension
		CcdFile_Path = GetPath ( DiskImagePath ) + GetFile ( DiskImagePath ) + ".ccd";
		
		if ( !ParseCcdSheet ( CcdFile_Path.c_str() ) )
		{
			cout << "\nCcd file not found or other error. Should be: " << CcdFile_Path.c_str();
			cout << "\nAssuming single data track.\n";
			
			// since there was an error, assume a single data track
			// single track starts at two seconds
			iNumberOfTracks = 1;
			TrackData [ 1 ].Min = 0;
			TrackData [ 1 ].Sec = 2;
			TrackData [ 1 ].Frac = 0;
		}
		else
		{
			cout << "\nCcd file found.";
		}
	}
	else
	{
		cout << "\nCue file found.";
	}
	*/

//cout << "\nExiting CDImage::OpenDiskImage";
	
	// disk image was opened successfully
	//isDiskOpen = true;
	return bDiskOpenedSuccessfully;
}

bool CDImage::CloseDiskImage ()
{
	// close disk image and make sure it closed successfully
	if ( isDiskOpen )
	{
		//if ( !fclose ( image ) ) return true;
		if ( !image.Close() ) return false;
	}

	// check if .sub file was also open //
	if ( isSubOpen )
	{
		// .sub file is open, so close it too //
		if ( !sub.Close() ) return false;
	}

	// disk image is no longer open since it is being closed
	isDiskOpen = false;
	isSubOpen = false;
	
	//return false;
	return true;
}


// returns false if seek failed
// time values are just regular numbers, NOT bcd
// use for seeking via Min,Sec,Frac
bool CDImage::SeekTime ( s32 Minutes, s32 Seconds, s32 Sector )
{
	int i;
	u32 SectorNumber;
	
	// you can't read data from the disk if you are already reading
	// so we'll just wait until it is done reading
	// important note: should make sure all reads are complete before doing seek, since Read/Write indexes could possibly get overwritted otherwise
	WaitForAllReadsComplete ();

	// get what should be the offset into the sector on the disk
	//SectorOffset = ( Minutes * c_SectorsPerMinute ) + ( Seconds * c_SectorsPerSecond ) + Sector;
	SectorNumber = GetSectorNumber ( Minutes, Seconds, Sector );
	
	// this should no longer chop off the first two seconds
	/*
	// CD disk images start at 2 seconds, though, regardless of whether it is img or bin file
	SectorOffset -= c_SectorsInFirstTwoSeconds;

	// seek to anything under two seconds and it fails
	if ( SectorOffset < 0 ) return false;
	*/

	// reset read and write index
	//ReadIndex = -1;
	//WriteIndex = 0;
	// *** testing ***
	isReadingFirstSector = true;
	Next_ReadIndex = WriteIndex;
	Next_CurrentSector = SectorNumber;
	
	// the next sector to read needs to be available to multiple threads from here
	//NextSector = SectorOffset;
	NextSector = SectorNumber;
	
	// get the sector at which the next track starts at (used for auto pause, etc)
	i = GetIndexData_Index ( SectorNumber );
	
	// get current track
	CurrentTrack = IndexData [ i ].Track;
	
	// only want to know that the track is not equal to the current one
	if ( IndexData [ i + 1 ].Track != CurrentTrack )
	{
		NextTrack = IndexData [ i + 1 ].Track;
		NextTrack_Sector = IndexData [ i + 1 ].SectorNumber;
	}
	else
	{
		NextTrack = IndexData [ i + 2 ].Track;
		NextTrack_Sector = IndexData [ i + 2 ].SectorNumber;
	}

	
	// also set the same for the current sector and read index
	ReadIndex = Next_ReadIndex;
	CurrentSector = Next_CurrentSector;
	
	// is now at the new location after performing the seek, so must update disk location immediately //
	
	// set the min, sec, frac
	SplitSectorNumber ( CurrentSector, Current_AMin, Current_ASec, Current_AFrac );
	
	// update the simulated subq data
	UpdateSubQ_Data ();
	
	
	return true;
}


// use for seeking by sector number
void CDImage::SeekSector ( u64 Sector )
{
	u32 SectorOffset, SectorNumber, i;

//cout << "\nCalled: CDImage::SeekSector";
//cout << "\n Sector=" << dec << Sector;

	// you can't read data from the disk if you are already reading
	// so we'll just wait until it is done reading
	// important note: should make sure all reads are complete before doing seek, since Read/Write indexes could possibly get overwritted otherwise
	WaitForAllReadsComplete ();
	

	SectorOffset = Sector;
	SectorNumber = Sector;
	
	// reset read and write index
	//ReadIndex = -1;
	//WriteIndex = 0;
	
	// the next sector to read needs to be available to multiple threads from here
	NextSector = SectorOffset;
	//Lock_Exchange64 ( (long long&) NextSector, SectorOffset );
	
	// *** testing ***
	isReadingFirstSector = true;
	Next_ReadIndex = WriteIndex;
	Next_CurrentSector = SectorNumber;
	
	// the next sector to read needs to be available to multiple threads from here
	//NextSector = SectorOffset;
	//NextSector = SectorNumber;
	
	
	// get the sector at which the next track starts at (used for auto pause, etc)
	i = GetIndexData_Index ( SectorNumber );
	
	// get current track
	CurrentTrack = IndexData [ i ].Track;
	
	// only want to know that the track is not equal to the current one
	if ( IndexData [ i + 1 ].Track != CurrentTrack )
	{
		NextTrack = IndexData [ i + 1 ].Track;
		NextTrack_Sector = IndexData [ i + 1 ].SectorNumber;
	}
	else
	{
		NextTrack = IndexData [ i + 2 ].Track;
		NextTrack_Sector = IndexData [ i + 2 ].SectorNumber;
	}

	
	// also set the same for the current sector and read index
	ReadIndex = Next_ReadIndex;
	CurrentSector = Next_CurrentSector;
	
	// is now at the new location after performing the seek, so must update disk location immediately //
	
	// set the min, sec, frac
	SplitSectorNumber ( CurrentSector, Current_AMin, Current_ASec, Current_AFrac );
	

	// update the simulated subq data
	UpdateSubQ_Data ();
	
	
	//return true;
	//fseeko64 ( image, ( Sector ) * c_SectorSize, SEEK_SET );
	//fseek ( image, ( Sector ) * c_SectorSize, SEEK_SET );
	//image.Seek ( ( Sector ) * c_SectorSize );
	
	// reset read and write index
	// don't set next sector or read/write index here since this function is used by object
	//ReadIndex = 0;
	//WriteIndex = 0;
	
//cout << "\nExiting: CDImage::SeekSector";
}

/*
int CDImage::ReadData ( u8* Data )
{
}
*/

// only call this after doing a seek first
void CDImage::StartReading ()
{
//cout << "\nCalled: CDImage::StartReading";

	// just pre-read some sectors
	//NumberOfSectorsToWrite = c_SectorReadCount;
	
	// you can't read data from the disk if you are already reading
	// so we'll just wait until it is done reading
	// already waited when it did the SeekSector etc
	WaitForAllReadsComplete ();
	
	/*
	while ( isReadInProgress )
	{
		//cout << "\ncdimage: Waiting for read to finish1\n";
		WindowClass::DoEvents ();
	}
	
	// check if .sub file is open //
	if ( isSubOpen )
	{
		// .sub file was open, so wait until read is done from there too //
		while ( isSubReadInProgress )
		{
			WindowClass::DoEvents ();
		}
	}
	*/
	
	/*
	// read is in progress
	isReadInProgress = true;
	//Lock_Exchange32 ( (long&)isReadInProgress, true );
	
	// if .sub file is open, then will be reading from it too //
	if ( isSubOpen )
	{
		//Lock_Exchange32 ( (long&)isSubReadInProgress, true );
		isSubReadInProgress = true;
	}
	*/
	
	// seek to sector?? or I think this happens asynchronously??
	
	
	// schedule read to happen asynchronously and return the pointer into the buffer for reading data
	// schedule read to happen asynchronously
	// this has to be called on the gui thread since it does the alertable wait stait thing it needs for asynchronous reading
	// I would use fread for portability, but this is not working properly for some reason in MinGW64
	// *** TODO ***
	WindowClass::Window::RemoteCall ( (WindowClass::Window::RemoteFunction) _RemoteCall_ReadAsync, (void*) NULL, false );

	
	// *** TESTING ***
	//WaitForAllReadsComplete ();
	

	// didn't read the data yet, but did read the subchannel info
	if ( isSubOpen )
	{
		//CurrentSubBuffer = & ( SubBuffer [ ( /*ReadIndex*/ 0 & c_BufferSectorMask ) * c_SubChannelSizePerSector ] );
		CurrentSubBuffer = & ( SubBuffer [ ( Next_ReadIndex & c_BufferSectorMask ) * c_SubChannelSizePerSector ] );
		
		//cout << "\n\nStart: AMin=" << hex << (u32)((CDImage::Sector::SubQ*)CurrentSubBuffer)->AbsoluteAddress [ 0 ] << " ASec=" << (u32)((CDImage::Sector::SubQ*)CurrentSubBuffer)->AbsoluteAddress [ 1 ] << " AFrac=" << (u32)((CDImage::Sector::SubQ*)CurrentSubBuffer)->AbsoluteAddress [ 2 ];
		//cout << "\nMin=" << hex << (u32)((CDImage::Sector::SubQ*)CurrentSubBuffer)->TrackRelativeAddress [ 0 ] << " Sec=" << (u32)((CDImage::Sector::SubQ*)CurrentSubBuffer)->TrackRelativeAddress [ 1 ] << " Frac=" << (u32)((CDImage::Sector::SubQ*)CurrentSubBuffer)->TrackRelativeAddress [ 2 ];
	}
	

//cout << "\nExiting: CDImage::StartReading";
}


// returns a pointer to the buffer with sector data that will be read
// must call isSectorReadComplete () to make sure the data is ready before reading
u8* CDImage::ReadNextSector ()
{
	u8* BufferToReadDataFrom;
	
//cout << "\nCDImage::ReadNextSector";
	
	// make sure that at least the first block has been read before proceeding
	//while ( WriteIndex < c_SectorReadCount )
	//WaitForAllReadsComplete ();
	
	/*
	while ( isReadInProgress )
	{
		WindowClass::DoEvents ();
	}
	
	if ( isSubOpen )
	{
		// .sub file was open, so wait until read is done from there too //
		while ( isSubReadInProgress )
		{
			//cout << "\ncdimage: Waiting for sub read to finish...\n";
			WindowClass::DoEvents ();
		}
	}
	*/
	
	
	if ( isReadingFirstSector )
	{
//cout << "\nisReadingFirstSector";

		// jump to where the first sector should be read from
		ReadIndex = Next_ReadIndex;
		
		// set the current sector
		CurrentSector = Next_CurrentSector;
		
		// no longer reading the first sector
		isReadingFirstSector = false;
	}
	else
	{
//cout << "\n!isReadingFirstSector";

		// update read index first thing
		ReadIndex++;
		
		// update the current sector number
		CurrentSector++;
	}
	
	// set the min, sec, frac
	SplitSectorNumber ( CurrentSector, Current_AMin, Current_ASec, Current_AFrac );
	
	// update the simulated subq data
	UpdateSubQ_Data ();
	
	// if ReadIndex has reached WriteIndex, then need to wait for the pending transfers to finish before can read anything
	if ( ReadIndex >= WriteIndex )
	{
//cout << "\nReadIndex>=WriteIndex";
		WaitForSectorReadComplete ();
	}
	
	//if ( ReadIndex & c_SectorReadCountMask )
	//if ( ReadIndex < ( WriteIndex - c_SectorReadCount ) )
	if ( ReadIndex < WriteIndex )
	{
//cout << "\nReadIndex<WriteIndex";
		// get buffer to read sector data from when it becomes ready
		//BufferToReadDataFrom = & ( Buffer [ ( ReadIndex & c_BufferSectorMask ) * c_SectorSize ] );
		BufferToReadDataFrom = & ( Buffer [ ( ReadIndex & c_BufferSectorMask ) * SectorSize ] );

		// also need buffer for subchannel data if .sub file is open //
		if ( isSubOpen )
		{
			CurrentSubBuffer = & ( SubBuffer [ ( ReadIndex & c_BufferSectorMask ) * c_SubChannelSizePerSector ] );
		}
		
		// return pointer into buffer for reading data
		// but since read is asynchronous, must call 
		//return ReadIndex;
		//return BufferToReadDataFrom;
	}
	
	
	if ( ReadIndex == ( WriteIndex - ( c_SectorReadCount ) ) )
	{
//cout << "\nReadIndex == ( WriteIndex - ( c_SectorReadCount >> 1 ) )";
	///////////////////////////////////////////////////////////////////
	// Time to load in the other half of read buffer
	
	// you can't read data from the file if you are already reading from it
	// so we'll just wait until it is done reading from the file
	//WaitForAllReadsComplete ();
	
	/*
	while ( isReadInProgress )
	{
		//cout << "\ncdimage: Waiting for read to finish...\n";
		WindowClass::DoEvents ();
	}
	
	// check if .sub file is open //
	if ( isSubOpen )
	{
		// .sub file was open, so wait until read is done from there too //
		while ( isSubReadInProgress )
		{
			//cout << "\ncdimage: Waiting for sub read to finish...\n";
			WindowClass::DoEvents ();
		}
	}
	*/
	
	
	// just pre-read some sectors
	//NumberOfSectorsToWrite = c_SectorReadCount;
	
	/*
	// read is in progress - do this before starting the read
	isReadInProgress = true;
	//Lock_Exchange32 ( (long&)isReadInProgress, true );
	
	// if .sub file is open, then will be reading from it too //
	if ( isSubOpen )
	{
		//Lock_Exchange32 ( (long&)isSubReadInProgress, true );
		isSubReadInProgress = true;
	}
	*/
	
	// seek to sector?? or I think this happens asynchronously??
	
	// schedule read to happen asynchronously and return the pointer into the buffer for reading data
	// schedule read to happen asynchronously
	// this has to be called on the gui thread
	// *** TODO ***
	WindowClass::Window::RemoteCall ( (WindowClass::Window::RemoteFunction) _RemoteCall_ReadAsync, (void*) NULL, false );
	
	// *** TESTING ***
	/*
	WaitForAllReadsComplete ();
	
	// get buffer to read sector data from when it becomes ready
	//BufferToReadDataFrom = & ( Buffer [ ( ReadIndex & c_BufferSectorMask ) * c_SectorSize ] );
	BufferToReadDataFrom = & ( Buffer [ ( ReadIndex & c_BufferSectorMask ) * SectorSize ] );
	
	// if .sub file is open, then will need buffer to read sub data from //
	if ( isSubOpen )
	{
		CurrentSubBuffer = & ( SubBuffer [ ( ReadIndex & c_BufferSectorMask ) * c_SubChannelSizePerSector ] );
	}
	*/

	// update read index
	// no, this should be done first thing
	//Lock_ExchangeAdd64 ( (long long&)ReadIndex, 1 );
	//ReadIndex++;
	}
	
//cout << "\nExiting<-CDImage::ReadNextSector";

	// return pointer into buffer for reading data
	// but since read is asynchronous, must call 
	return BufferToReadDataFrom;
	//return ReadIndex;
}


// marks the current sector as being the first sector
void CDImage::SetFirstSector ()
{
	Next_ReadIndex = ReadIndex;
	
	// also set the next current sector number
	Next_CurrentSector = CurrentSector;
	
	isReadingFirstSector = true;
}


u64 CDImage::GetCurrentBufferIndex ()
{
	return ReadIndex;
}

u8* CDImage::GetDataBuffer ( u64 Index )
{
	//return (& ( Buffer [ ( Index & c_BufferSectorMask ) * c_SectorSize ] ));
	return (& ( Buffer [ ( Index & c_BufferSectorMask ) * SectorSize ] ));
}

u8* CDImage::GetCurrentDataBuffer ()
{
	//return (& ( Buffer [ ( ReadIndex & c_BufferSectorMask ) * c_SectorSize ] ));
	return (& ( Buffer [ ( ReadIndex & c_BufferSectorMask ) * SectorSize ] ));
}

u8* CDImage::GetCurrentSubBuffer ()
{
	if ( isReadingFirstSector )
	{
		return (& ( SubBuffer [ ( Next_ReadIndex & c_BufferSectorMask ) * c_SubChannelSizePerSector ] ));
	}
	else
	{
		return (& ( SubBuffer [ ( ReadIndex & c_BufferSectorMask ) * c_SubChannelSizePerSector ] ));
	}
	
	return (& ( SubBuffer [ ( ReadIndex & c_BufferSectorMask ) * c_SubChannelSizePerSector ] ));
}


u32 CDImage::ConvertBCDToDec ( u8 BCD )
{
	//if ( BCD > 0x99 ) BCD -= 0x80;
	return ( BCD & 0xf ) + ( ( BCD >> 4 ) * 10 );
}


u8 CDImage::ConvertDecToBCD8 ( u32 Dec )
{
	return ( Dec % 10 ) | ( ( ( Dec / 10 ) % 10 ) << 4 );
}


bool CDImage::_RemoteCall_OpenDiskImage ( string FullImagePath )
{
	// this will be called from the PS1 thread now, so it does not have to be called on another thread - can return whether successful or not
	
	//cout << "\nCalled CDImage::_RemoteCall_OpenDiskImage";
	
	bool bRet;
	
	string TempString;
	string FileName, Extension, Path;
	
	// first load the image //
	
	// get just the file name without path or extension
	Path = GetPath ( FullImagePath );
	FileName = GetFile ( FullImagePath );
	Extension = GetExtension ( FullImagePath );
	
	//cout << "\nDEBUG: Path= '" << Path.c_str() << "'";
	cout << "\nDEBUG: FileName= '" << FileName.c_str() << "'";
	cout << "\nDEBUG: Extension= '" << Extension.c_str () << "'";


	// set track zero to zero
	TrackData [ 0 ].Min = 0;
	TrackData [ 0 ].Sec = 0;
	TrackData [ 0 ].Frac = 0;
	
	// put in the new stuff too //
	IndexData [ 0 ].Track = 0xff;
	IndexData [ 0 ].Index = 1;
	
	IndexData [ 0 ].Min = 0;
	IndexData [ 0 ].Sec = 0;
	IndexData [ 0 ].Frac = 0;
	
	IndexData [ 0 ].AMin = 0;
	IndexData [ 0 ].ASec = 0;
	IndexData [ 0 ].AFrac = 0;
	
	IndexData [ 0 ].SectorNumber = 0;
	IndexData [ 0 ].SectorNumber_InImage = -1;
	
	// initialize pregap
	iPreGap = 0;
	
	
	// check to make sure disk image format is supported
	if ( LCase( Extension ) == ".bin" )
	{
		// BIN format //
		
		cout << "\nINFO: Found image file. Image is in .bin format which is supported.";
		
		// initially set the path to image as a .bin file
		sCDPath = Path + FileName + ".bin";
		
		if ( ParseCueSheet ( Path + FileName + ".cue" ) )
		{
			cout << "\nNOTE: Found .cue file for disk image. Parsed successfully.";
			
#ifdef DISK_READ_SYNC
			bRet = _DISKIMAGE->image.CreateSync ( sCDPath );
#else
			bRet = _DISKIMAGE->image.CreateAsync ( sCDPath );
#endif

		}
		else
		{
			cout << "\nERROR: Unable to find .cue file for .bin disk image.";
			cout << "\nERROR: .cue file should be in the same folder and have same name as the .bin disk image.";
			cout << "\nWARNING: continuing without use of .cue file. Assuming single data track.";
			
			// since there was an error, assume a single data track
			// single track starts at two seconds
			iNumberOfTracks = 1;
			TrackData [ 1 ].Min = 0;
			TrackData [ 1 ].Sec = 2;
			TrackData [ 1 ].Frac = 0;
			
			// put in the new stuff
			iNumberOfIndexes = 2;
			
			IndexData [ 1 ].Track = 1;
			IndexData [ 1 ].Index = 1;
			
			IndexData [ 1 ].Min = 0;
			IndexData [ 1 ].Sec = 0;
			IndexData [ 1 ].Frac = 0;
			
			IndexData [ 1 ].AMin = 0;
			IndexData [ 1 ].ASec = 2;
			IndexData [ 1 ].AFrac = 0;
			
			IndexData [ 1 ].SectorNumber = 150;
			IndexData [ 1 ].SectorNumber_InImage = 0;
			
#ifdef DISK_READ_SYNC
			bRet = _DISKIMAGE->image.CreateSync ( FullImagePath );
#else
			bRet = _DISKIMAGE->image.CreateAsync ( FullImagePath );
#endif
		}
	}
	else if ( LCase ( Extension ) == ".img" )
	{
		// IMG format //
		
		cout << "\nINFO: Found image file. Image is in .img format which is supported.";
		
		if ( ParseCcdSheet ( Path + FileName + ".ccd" ) )
		{
			cout << "\nNOTE: Found .ccd file for disk image. Parsed successfully.";
		}
		else
		{
			cout << "\nERROR: Unable to find .ccd file for .img disk image.";
			cout << "\nERROR: .ccd file should be in the same folder and have same name as the .img disk image.";
			cout << "\nWARNING: continuing without use of .ccd file. Assuming single data track.";
			
			// since there was an error, assume a single data track
			// single track starts at two seconds
			iNumberOfTracks = 1;
			TrackData [ 1 ].Min = 0;
			TrackData [ 1 ].Sec = 2;
			TrackData [ 1 ].Frac = 0;
			
			// put in the new stuff
			iNumberOfIndexes = 2;
			
			IndexData [ 1 ].Track = 1;
			IndexData [ 1 ].Index = 1;
			
			IndexData [ 1 ].Min = 0;
			IndexData [ 1 ].Sec = 0;
			IndexData [ 1 ].Frac = 0;
			
			IndexData [ 1 ].AMin = 0;
			IndexData [ 1 ].ASec = 2;
			IndexData [ 1 ].AFrac = 0;
			
			IndexData [ 1 ].SectorNumber = 150;
			IndexData [ 1 ].SectorNumber_InImage = 0;
		}
		
#ifdef DISK_READ_SYNC
			bRet = _DISKIMAGE->image.CreateSync ( FullImagePath );
#else
			bRet = _DISKIMAGE->image.CreateAsync ( FullImagePath );
#endif
	}
	else if ( LCase ( Extension ) == ".iso" )
	{
		// ISO format //
		
		cout << "\nINFO: Found image file. Image is in .iso format which is supported.";
		
		// initially set the path to image as a .bin file
		sCDPath = Path + FileName + ".iso";
		
#ifdef DISK_READ_SYNC
		bRet = _DISKIMAGE->image.CreateSync ( Path + FileName + ".iso" );
#else
		bRet = _DISKIMAGE->image.CreateAsync ( Path + FileName + ".iso" );
#endif

		if ( bRet )
		{
			cout << "\nINFO: Found image file. Image is in .iso format which is supported.";
		}
		else
		{
			cout << "\nERROR: Unable to open cd image file. Only .bin, .img, .iso image formats are supported.";
			return false;
		}
	}
	else if ( LCase ( Extension ) == ".ccd" )
	{
		if ( ParseCcdSheet ( Path + FileName + ".ccd" ) )
		{
			cout << "\nNOTE: Found .ccd file for disk image. Parsed successfully.";
		}
		else
		{
			cout << "\nProblem parsing ccd file. Assuming single data track.";
			
			// since there was an error, assume a single data track
			// single track starts at two seconds
			iNumberOfTracks = 1;
			TrackData [ 1 ].Min = 0;
			TrackData [ 1 ].Sec = 2;
			TrackData [ 1 ].Frac = 0;
			
			// put in the new stuff
			iNumberOfIndexes = 2;
			
			IndexData [ 1 ].Track = 1;
			IndexData [ 1 ].Index = 1;
			
			IndexData [ 1 ].Min = 0;
			IndexData [ 1 ].Sec = 0;
			IndexData [ 1 ].Frac = 0;
			
			IndexData [ 1 ].AMin = 0;
			IndexData [ 1 ].ASec = 2;
			IndexData [ 1 ].AFrac = 0;
			
			IndexData [ 1 ].SectorNumber = 150;
			IndexData [ 1 ].SectorNumber_InImage = 0;
		}
		
#ifdef DISK_READ_SYNC
		bRet = _DISKIMAGE->image.CreateSync ( Path + FileName + ".img" );
#else
		bRet = _DISKIMAGE->image.CreateAsync ( Path + FileName + ".img" );
#endif
	}
	else if ( LCase ( Extension ) == ".cue" )
	{
		// initially set the path to image as a .bin file
		sCDPath = Path + FileName + ".bin";
		
		if ( ParseCueSheet ( Path + FileName + ".cue" ) )
		{
			cout << "\nNOTE: Found .cue file for disk image. Parsed successfully.";
			
#ifdef DISK_READ_SYNC
			bRet = _DISKIMAGE->image.CreateSync ( sCDPath );
#else
			bRet = _DISKIMAGE->image.CreateAsync ( sCDPath );
#endif
		}
		else
		{
			cout << "\nProblem parsing cue file. Assuming single data track.";
			
			// since there was an error, assume a single data track
			// single track starts at two seconds
			iNumberOfTracks = 1;
			TrackData [ 1 ].Min = 0;
			TrackData [ 1 ].Sec = 2;
			TrackData [ 1 ].Frac = 0;
			
			// put in the new stuff
			iNumberOfIndexes = 2;
			
			IndexData [ 1 ].Track = 1;
			IndexData [ 1 ].Index = 1;
			
			IndexData [ 1 ].Min = 0;
			IndexData [ 1 ].Sec = 0;
			IndexData [ 1 ].Frac = 0;
			
			IndexData [ 1 ].AMin = 0;
			IndexData [ 1 ].ASec = 2;
			IndexData [ 1 ].AFrac = 0;
			
			IndexData [ 1 ].SectorNumber = 150;
			IndexData [ 1 ].SectorNumber_InImage = 0;
			
#ifdef DISK_READ_SYNC
			bRet = _DISKIMAGE->image.CreateSync ( Path + FileName + ".bin" );
#else
			bRet = _DISKIMAGE->image.CreateAsync ( Path + FileName + ".bin" );
#endif
		}
	}
	else
	{
		// format is unsupported //
		
		cout << "\nERROR: Unable to open cd image file. Only .bin, .img, .iso, .ccd, .cue image formats are supported.";
		return false;
	}
	

	if ( iNumberOfIndexes < 2 )
	{
		// since there was an error, assume a single data track
		// single track starts at two seconds
		iNumberOfTracks = 1;
		TrackData [ 1 ].Min = 0;
		TrackData [ 1 ].Sec = 2;
		TrackData [ 1 ].Frac = 0;
		
		// put in the new stuff
		iNumberOfIndexes = 2;
		
		IndexData [ 1 ].Track = 1;
		IndexData [ 1 ].Index = 1;
		
		IndexData [ 1 ].Min = 0;
		IndexData [ 1 ].Sec = 0;
		IndexData [ 1 ].Frac = 0;
		
		IndexData [ 1 ].AMin = 0;
		IndexData [ 1 ].ASec = 2;
		IndexData [ 1 ].AFrac = 0;
		
		IndexData [ 1 ].SectorNumber = 150;
		IndexData [ 1 ].SectorNumber_InImage = 0;
	}


	
	/*
	// try each image type that is supported one by one (.bin,.img,.iso)
	if ( bRet )
	{
		cout << "\nINFO: Found image file. Image is in .bin format which is supported.";
	}
	else
	{
#ifdef DISK_READ_SYNC
		bRet = _DISKIMAGE->image.CreateSync ( Path + FileName + ".img" );
#else
		bRet = _DISKIMAGE->image.CreateAsync ( Path + FileName + ".img" );
#endif

		if ( bRet )
		{
			cout << "\nINFO: Found image file. Image is in .img format which is supported.";
		}
		else
		{
#ifdef DISK_READ_SYNC
			bRet = _DISKIMAGE->image.CreateSync ( Path + FileName + ".iso" );
#else
			bRet = _DISKIMAGE->image.CreateAsync ( Path + FileName + ".iso" );
#endif

			if ( bRet )
			{
				cout << "\nINFO: Found image file. Image is in .iso format which is supported.";
			}
			else
			{
				cout << "\nERROR: Unable to open cd image file. Only .bin, .img, .iso image formats are supported.";
				return false;
			}
		}
	}
	*/
	
	
	// sub file starts out as not opened
	isSubOpen = false;

	// disable sub files for now //
#ifdef ENABLE_SUB_FILES

#ifdef DISK_READ_SYNC	
	bRet = _DISKIMAGE->sub.CreateSync ( Path + FileName + ".sub" );
#else
	bRet = _DISKIMAGE->sub.CreateAsync ( Path + FileName + ".sub" );
#endif
	
	// now look for a .sub file //
	if ( !bRet )
	{
		// .sub file was not opened
		cout << "\nERROR: Problem opening .sub file.";
		isSubOpen = false;
	}
	else
	{
		// .sub image was opened successfully
		cout << "\nINFO: .sub file opened successfully.";
		isSubOpen = true;
		_DISKIMAGE->isSubReadInProgress = false;
	}

#endif

	
	/*
	// set track zero to zero
	TrackData [ 0 ].Min = 0;
	TrackData [ 0 ].Sec = 0;
	TrackData [ 0 ].Frac = 0;
	
	// put in the new stuff too //
	IndexData [ 0 ].Track = 0xff;
	IndexData [ 0 ].Index = 1;
	
	IndexData [ 0 ].Min = 0;
	IndexData [ 0 ].Sec = 0;
	IndexData [ 0 ].Frac = 0;
	
	IndexData [ 0 ].AMin = 0;
	IndexData [ 0 ].ASec = 0;
	IndexData [ 0 ].AFrac = 0;
	
	IndexData [ 0 ].SectorNumber = 0;
	IndexData [ 0 ].SectorNumber_InImage = -1;
	
	// initialize pregap
	iPreGap = 0;
	
	// also check for .cue or .ccd file //
	
	if ( ParseCueSheet ( Path + FileName + ".cue" ) )
	{
		cout << "\nNOTE: Found .cue file for disk image. Parsed successfully.";
	}
	else if ( ParseCcdSheet ( Path + FileName + ".ccd" ) )
	{
		cout << "\nNOTE: Found .ccd file for disk image. Parsed successfully.";
	}
	else
	{
		cout << "\nERROR: Unable to find .cue or .ccd file for disk image.";
		cout << "\nERROR: .cue or .ccd file should be in the same folder as the disk image.";
		cout << "\nWARNING: continuing without use of .cue or .ccd file.";
		
		// since there was an error, assume a single data track
		// single track starts at two seconds
		iNumberOfTracks = 1;
		TrackData [ 1 ].Min = 0;
		TrackData [ 1 ].Sec = 2;
		TrackData [ 1 ].Frac = 0;
		
		// put in the new stuff
		iNumberOfIndexes = 2;
		
		IndexData [ 1 ].Track = 1;
		IndexData [ 1 ].Index = 1;
		
		IndexData [ 1 ].Min = 0;
		IndexData [ 1 ].Sec = 0;
		IndexData [ 1 ].Frac = 0;
		
		IndexData [ 1 ].AMin = 0;
		IndexData [ 1 ].ASec = 2;
		IndexData [ 1 ].AFrac = 0;
		
		IndexData [ 1 ].SectorNumber = 150;
		IndexData [ 1 ].SectorNumber_InImage = 0;
	}
	
	if ( iNumberOfIndexes < 2 )
	{
		// since there was an error, assume a single data track
		// single track starts at two seconds
		iNumberOfTracks = 1;
		TrackData [ 1 ].Min = 0;
		TrackData [ 1 ].Sec = 2;
		TrackData [ 1 ].Frac = 0;
		
		// put in the new stuff
		iNumberOfIndexes = 2;
		
		IndexData [ 1 ].Track = 1;
		IndexData [ 1 ].Index = 1;
		
		IndexData [ 1 ].Min = 0;
		IndexData [ 1 ].Sec = 0;
		IndexData [ 1 ].Frac = 0;
		
		IndexData [ 1 ].AMin = 0;
		IndexData [ 1 ].ASec = 2;
		IndexData [ 1 ].AFrac = 0;
		
		IndexData [ 1 ].SectorNumber = 150;
		IndexData [ 1 ].SectorNumber_InImage = 0;
	}
	*/
	
	
	// get size of file to get end of last track
	SizeOfImage = _DISKIMAGE->image.Size ();
	//SectorsInImage = SizeOfImage / c_SectorSize;
	SectorsInImage = SizeOfImage / SectorSize;
	
	cout << "\nSizeOfImage= " << dec << SizeOfImage;
	cout << "\nfrom file path: " << sCDPath.c_str();
	
	// add in the two seconds of sectors at the beginning = 150 sectors
	SectorsInImage += c_SectorsInFirstTwoSeconds;
	
	// also add in any pregap time
	SectorsInImage += ( iPreGap * 75 );
	
	cout << "\nSectorsInImage= " << dec << SectorsInImage << " SectorSize=" << SectorSize;
	
	// get the total length of the disk
	SplitSectorNumber ( SectorsInImage, _DISKIMAGE->TrackData [ iNumberOfTracks + 1 ].Min, _DISKIMAGE->TrackData [ iNumberOfTracks + 1 ].Sec, _DISKIMAGE->TrackData [ iNumberOfTracks + 1 ].Frac );
	
	// also put this into the new index data
	SplitSectorNumber ( SectorsInImage, _DISKIMAGE->IndexData [ iNumberOfIndexes ].AMin, _DISKIMAGE->IndexData [ iNumberOfIndexes ].ASec, _DISKIMAGE->IndexData [ iNumberOfIndexes ].AFrac );
	
	// also set the number of sectors in disk (can be used later for a number of things)
	_DISKIMAGE->IndexData [ iNumberOfIndexes ].SectorNumber = SectorsInImage;
	_DISKIMAGE->IndexData [ iNumberOfIndexes ].SectorNumber_InImage = -1;
	
	// set track number for lead out area
	_DISKIMAGE->IndexData [ iNumberOfIndexes ].Track = 0xaa;
	
	// save the last sector number
	LastSector_Number = SectorsInImage;
	
	cout << "\nNOTE: Size of disk image is: " << dec << SizeOfImage;
	cout << "\nNOTE: End of last track is at: Min=" << dec << (u32)_DISKIMAGE->TrackData [ iNumberOfTracks + 1 ].Min << " Sec=" << (u32)_DISKIMAGE->TrackData [ iNumberOfTracks + 1 ].Sec << " Frac=" << (u32)_DISKIMAGE->TrackData [ iNumberOfTracks + 1 ].Frac;
	
	// disk is now open
	_DISKIMAGE->isDiskOpen = true;
	//Lock_Exchange32 ( (long&)_DISKIMAGE->isDiskOpen, true );
	
	// alert other thread that read is done and disk is open
	//Lock_Exchange32 ( (long&) _DISKIMAGE->isReadInProgress, false );
	_DISKIMAGE->isReadInProgress = false;
	
	//cout << "\nExiting CDImage::_RemoteCall_OpenDiskImage; Path=" << _DISKIMAGE->ImagePath;
	
	// disk was opened successfully
	return true;
}


void CDImage::_RemoteCall_ReadAsync ()
{
//cout << "\nCalled CDImage::_RemoteCall_ReadAsync isReadInProgress=" << isReadInProgress;
	
	int Ret;
	u64 SectorNumber;
	
//cout << "\nCDImage::_RemoteCall_ReadAsync isReadInProgress=" << isReadInProgress;

	// make sure disk is not already reading
	// no, don't do this since it would freeze
	//while ( _DISKIMAGE->isReadInProgress );
	
#ifdef DISK_READ_SYNC

	SectorNumber = _DISKIMAGE->GetSectorNumber_InImage ( _DISKIMAGE->NextSector );

	if ( SectorNumber == -1 )
	{
		// zero sector
		//for ( int i = 0; i < ( c_SectorSize * c_SectorReadCount ); i++ )
		for ( int i = 0; i < ( _DISKIMAGE->SectorSize * c_SectorReadCount ); i++ )
		{
			//_DISKIMAGE->Buffer [ i + ( ( _DISKIMAGE->WriteIndex & c_BufferSectorMask ) * c_SectorSize ) ] = 0;
			_DISKIMAGE->Buffer [ i + ( ( _DISKIMAGE->WriteIndex & c_BufferSectorMask ) * _DISKIMAGE->SectorSize ) ] = 0;
		}
	}
	else
	{
		// load sector from image
		//Ret = _DISKIMAGE->image.ReadSync ( & ( _DISKIMAGE->Buffer [ ( _DISKIMAGE->WriteIndex & c_BufferSectorMask ) * c_SectorSize ] ), c_SectorSize * c_SectorReadCount, _DISKIMAGE->NextSector * c_SectorSize );
		//Ret = _DISKIMAGE->image.ReadSync ( & ( _DISKIMAGE->Buffer [ ( _DISKIMAGE->WriteIndex & c_BufferSectorMask ) * c_SectorSize ] ), c_SectorSize * c_SectorReadCount, SectorNumber * c_SectorSize );
		Ret = _DISKIMAGE->image.ReadSync ( & ( _DISKIMAGE->Buffer [ ( _DISKIMAGE->WriteIndex & c_BufferSectorMask ) * _DISKIMAGE->SectorSize ] ), _DISKIMAGE->SectorSize * c_SectorReadCount, SectorNumber * _DISKIMAGE->SectorSize );
	}
	
	// if .sub file is open, then also read subchannel data //
	if ( isSubOpen )
	{
		//Ret = _DISKIMAGE->sub.ReadSync ( & ( _DISKIMAGE->SubBuffer [ ( _DISKIMAGE->WriteIndex & c_BufferSectorMask ) * c_SubChannelSizePerSector ] ), c_SubChannelSizePerSector * c_SectorReadCount, _DISKIMAGE->NextSector * c_SubChannelSizePerSector );
		Ret = _DISKIMAGE->sub.ReadSync ( & ( _DISKIMAGE->SubBuffer [ ( _DISKIMAGE->WriteIndex & c_BufferSectorMask ) * c_SubChannelSizePerSector ] ), c_SubChannelSizePerSector * c_SectorReadCount, SectorNumber * c_SubChannelSizePerSector );
	}
	
	// update write index
	//Lock_ExchangeAdd64 ( (long long&)_DISKIMAGE->WriteIndex, c_SectorReadCount );
	_DISKIMAGE->WriteIndex += c_SectorReadCount;
	
	// disk is done reading
	//isReadInProgress = false;
	//Lock_Exchange32 ( (long&)_DISKIMAGE->isReadInProgress, false );
	_DISKIMAGE->isReadInProgress = false;
	
	// disk is done reading
	//Lock_Exchange32 ( (long&)_DISKIMAGE->isSubReadInProgress, false );
	_DISKIMAGE->isSubReadInProgress = false;
	
#else

//cout << "\nisReadInProgress isReadInProgress=" << isReadInProgress;
	
	// make sure disk is not already reading
	//while ( isReadInProgress )
	if ( isReadInProgress )
	{
		_DISKIMAGE->WaitForAllReadsComplete ();
	}
	
	isReadInProgress = true;
	
//cout << "\nGetSectorNumber_InImage";

	SectorNumber = _DISKIMAGE->GetSectorNumber_InImage ( _DISKIMAGE->NextSector );
	
	if ( SectorNumber == -1ull )
	{
//cout << "\nSectorNumber==-1";
		// zero sector
		//for ( int i = 0; i < ( c_SectorSize * c_SectorReadCount ); i++ )
		for ( int i = 0; i < ( _DISKIMAGE->SectorSize * c_SectorReadCount ); i++ )
		{
			//_DISKIMAGE->Buffer [ i + ( ( _DISKIMAGE->WriteIndex & c_BufferSectorMask ) * c_SectorSize ) ] = 0;
			_DISKIMAGE->Buffer [ i + ( ( _DISKIMAGE->WriteIndex & c_BufferSectorMask ) * _DISKIMAGE->SectorSize ) ] = 0;
		}
		
		_DISKIMAGE->WriteIndex += c_SectorReadCount;
		
		isReadInProgress = false;
	}
	else
	{
//cout << "\nSectorNumber!=-1";
		//_DISKIMAGE->image.ReadAsync ( _params.DataOut, _params.BytesToRead, _params.SeekPosition, _params.Callback_Function );
		//Ret = _DISKIMAGE->image.ReadAsync ( & ( _DISKIMAGE->Buffer [ ( _DISKIMAGE->WriteIndex & c_BufferSectorMask ) * c_SectorSize ] ), c_SectorSize * c_SectorReadCount, _DISKIMAGE->NextSector * c_SectorSize, DiskRead_Callback );
		//Ret = _DISKIMAGE->image.ReadAsync ( & ( _DISKIMAGE->Buffer [ ( _DISKIMAGE->WriteIndex & c_BufferSectorMask ) * _DISKIMAGE->SectorSize ] ), _DISKIMAGE->SectorSize * c_SectorReadCount, _DISKIMAGE->NextSector * _DISKIMAGE->SectorSize, DiskRead_Callback );
		Ret = _DISKIMAGE->image.ReadAsync ( (char*) & ( _DISKIMAGE->Buffer [ ( _DISKIMAGE->WriteIndex & c_BufferSectorMask ) * _DISKIMAGE->SectorSize ] ), _DISKIMAGE->SectorSize * c_SectorReadCount, SectorNumber * _DISKIMAGE->SectorSize, (void*) DiskRead_Callback );
	}
	
	// if .sub file is open, then also read subchannel data //
	if ( isSubOpen )
	{
//cout << "\nisSubOpen";
		//while ( _DISKIMAGE->isSubReadInProgress )
		if ( _DISKIMAGE->isSubReadInProgress )
		{
			_DISKIMAGE->WaitForAllReadsComplete ();
		}
		
		_DISKIMAGE->isSubReadInProgress = true;
	
		Ret = _DISKIMAGE->sub.ReadAsync ( (char*) & ( _DISKIMAGE->SubBuffer [ ( _DISKIMAGE->WriteIndex & c_BufferSectorMask ) * c_SubChannelSizePerSector ] ), c_SubChannelSizePerSector * c_SectorReadCount, _DISKIMAGE->NextSector * c_SubChannelSizePerSector, (void*) SubRead_Callback );
	}
	
//cout << "\nUpdate NextSector";
#endif

	// can update the next sector here
	// value no longer needs to be made available to the calling thread
	//Lock_ExchangeAdd64 ( (long long&)_DISKIMAGE->NextSector, c_SectorReadCount );
	_DISKIMAGE->NextSector += c_SectorReadCount;
	
//cout << "\nExiting CDImage::_RemoteCall_ReadAsync; ReadAsync=" << Ret << "; LastError=" << GetLastError ();
}


void CDImage::DiskRead_Callback ()
{
//cout << "\nStart->DiskRead_Callback";

//cout << "\nCalled CDImage::DiskRead_Callback";
	
	// update the sector offset for next sectors to be read
	// this is now done right after sending the asynchronous read
	//SectorOffset += _DISKIMAGE->NumberOfSectorsToWrite;
	//Lock_ExchangeAdd64 ( (long long&)_DISKIMAGE->NextSector, _DISKIMAGE->NumberOfSectorsToWrite );
	
	//asm volatile ( "sfence" );
	
	// output debug info
	//cout << "\nSector Read: " << dec << _DISKIMAGE->NextSector << "; debug data: " << hex << (u32) _DISKIMAGE->Buffer [ 2352 + 24 + 0 ] << " " << (u32) _DISKIMAGE->Buffer [ 2352 + 24 + 1 ] << " ";
	//cout << (u32) _DISKIMAGE->Buffer [ 2352 + 24 + 2 ] << " " << (u32) _DISKIMAGE->Buffer [ 2352 + 24 + 3 ] << " " << (u32) _DISKIMAGE->Buffer [ 2352 + 24 + 4 ] << " " << (u32) _DISKIMAGE->Buffer [ 2352 + 24 + 5 ];
	
	// update write index
	//Lock_ExchangeAdd64 ( (long long&)_DISKIMAGE->WriteIndex, c_SectorReadCount );
	_DISKIMAGE->WriteIndex += c_SectorReadCount;
	
	// disk is done reading
	//isReadInProgress = false;
	//Lock_Exchange32 ( (long&)_DISKIMAGE->isReadInProgress, false );
	_DISKIMAGE->isReadInProgress = false;
	
//cout << "\nExiting CDImage::DiskRead_Callback";

//cout << "\nDone->DiskRead_Callback";
}


void CDImage::SubRead_Callback ()
{
	// update write index
	//Lock_ExchangeAdd64 ( (long long&)_DISKIMAGE->WriteIndex, c_SectorReadCount );
	
	// disk is done reading
	//Lock_Exchange32 ( (long&)_DISKIMAGE->isSubReadInProgress, false );
	_DISKIMAGE->isSubReadInProgress = false;
	
	//cout << "\nExiting CDImage::DiskRead_Callback";
}

void CDImage::WaitForSectorReadComplete ()
{
//cout << "\nStart->WaitForSectorReadComplete";

//cout << "\nWaitAsync";

	while ( isReadInProgress )
	{
	_DISKIMAGE->image.WaitAsync ();
	}

//cout << "\nisSubOpen";

	if ( isSubOpen )
	{
		while ( isSubReadInProgress )
		{
		_DISKIMAGE->sub.WaitAsync ();
		}
	}
	
	/*
	while ( isReadInProgress )
	{
		//cout << "\ncdimage: Waiting for read to finish...\n";
		WindowClass::DoEvents ();
		//WindowClass::DoEventsNoWait ();
		//MsgWaitForMultipleObjectsEx( NULL, NULL, 1, QS_ALLINPUT, MWMO_ALERTABLE );
	}
	
	// check if .sub file is open //
	if ( isSubOpen )
	{
		// .sub file was open, so wait until read is done from there too //
		while ( isSubReadInProgress )
		{
			//cout << "\ncdimage: Waiting for sub read to finish...\n";
			WindowClass::DoEvents ();
			//WindowClass::DoEventsNoWait ();
			//MsgWaitForMultipleObjectsEx( NULL, NULL, 1, QS_ALLINPUT, MWMO_ALERTABLE );
		}
	}
	*/
	
//cout << "\nwhile ( ReadIndex >= WriteIndex )";

	while ( ReadIndex >= WriteIndex )
	{
		//WindowClass::DoEvents ();
		//cout << "\nWaiting for sector read complete...\n";
		_DISKIMAGE->image.WaitAsync ();
	}

//cout << "\nDone<-WaitForSectorReadComplete";
}


void CDImage::WaitForAllReadsComplete ()
{
//cout << "\nStart->WaitForAllReadsComplete";

	while ( isReadInProgress )
	{
	_DISKIMAGE->image.WaitAsync ();
	}

//cout << "\nisSubOpen";

	if ( isSubOpen )
	{
		while ( isSubReadInProgress )
		{
		_DISKIMAGE->sub.WaitAsync ();
		}
	}

	
	/*
	while ( isReadInProgress )
	{
		//cout << "\ncdimage: Waiting for read to finish...\n";
		WindowClass::DoEvents ();
		//WindowClass::DoEventsNoWait ();
		//MsgWaitForMultipleObjectsEx( NULL, NULL, 1, QS_ALLINPUT, MWMO_ALERTABLE );
	}
	
	// check if .sub file is open //
	if ( isSubOpen )
	{
		// .sub file was open, so wait until read is done from there too //
		while ( isSubReadInProgress )
		{
			//cout << "\ncdimage: Waiting for sub read to finish...\n";
			WindowClass::DoEvents ();
			//WindowClass::DoEventsNoWait ();
			//MsgWaitForMultipleObjectsEx( NULL, NULL, 1, QS_ALLINPUT, MWMO_ALERTABLE );
		}
	}
	*/
	
//cout << "\nDone->WaitForAllReadsComplete";
}


unsigned long CDImage::GetSectorNumber ( u32 Min, u32 Sec, u32 Frac )
{
	return ( Min * c_SectorsPerMinute ) + ( Sec * c_SectorsPerSecond ) + Frac;
}

void CDImage::SplitSectorNumber ( unsigned long SectorNumber, u8& Min, u8& Sec, u8& Frac )
{
	// get min
	//iMin = CLng ( Split ( Line, ":" ) [ 0 ] );
	Min = SectorNumber / c_SectorsPerMinute;
	
	// get sec
	//iSec = CLng ( Split ( Line, ":" ) [ 1 ] );
	Sec = ( SectorNumber % c_SectorsPerMinute ) / c_SectorsPerSecond;
	
	// get frac
	//iFrac = CLng ( Split ( Line, ":" ) [ 2 ] );
	Frac = SectorNumber % c_SectorsPerSecond;
}


void CDImage::Output_IndexData ()
{
	cout << "\nIndex Output:";
	
	for ( int i = 0; i < iNumberOfIndexes; i++ )
	{
		cout << "\nEntry#" << dec << i;
		cout << " Track=" << (u32) IndexData [ i ].Track << " Index=" << (u32) IndexData [ i ].Index;
		cout << " AMin=" << (u32) IndexData [ i ].AMin << " ASec=" << (u32) IndexData [ i ].ASec << " AFrac=" << (u32) IndexData [ i ].AFrac;
		cout << " Min=" << (u32) IndexData [ i ].Min << " Sec=" << (u32) IndexData [ i ].Sec << " Frac=" << (u32) IndexData [ i ].Frac;
		cout << " SectorNumber=" << (u32) IndexData [ i ].SectorNumber << " SectorNumber_InImage=" << (u32) IndexData [ i ].SectorNumber_InImage;
	}
	
	// also output the end of the disk
	cout << "\n\nEnd of disk: " << " AMin=" << (u32) IndexData [ iNumberOfIndexes ].AMin << " ASec=" << (u32) IndexData [ iNumberOfIndexes ].ASec << " AFrac=" << (u32) IndexData [ iNumberOfIndexes ].AFrac;
}



void CDImage::Output_SubQData ( u32 AMin, u32 ASec, u32 AFrac )
{
	UpdateSubQ_Data ( AMin, ASec, AFrac );
	
	cout << "\n\nSubQ Data for AMin=" << dec << AMin << " ASec=" << ASec << " AFrac=" << AFrac;
	cout << "\nTrack=" << (u32)SubQ_Track << " Index=" << (u32)SubQ_Index << " Min=" << (u32)SubQ_Min << " Sec=" << (u32)SubQ_Sec << " Frac=" << (u32)SubQ_Frac;
	cout << "\nSectorNumber=" << GetSectorNumber ( AMin, ASec, AFrac ) << " SectorNumber_InImage=" << GetSectorNumber_InImage ( AMin, ASec, AFrac );
}


// probably returns -1 on error
u32 CDImage::GetIndexData_Index ( u32 SectorNumber )
{
	int i;
	
	// looking backwards keeps it simple
	for ( i = iNumberOfIndexes; i >= 0; i-- )
	{
		if ( SectorNumber >= IndexData [ i ].SectorNumber ) break;
	}
	
	return i;
}


u32 CDImage::GetSectorNumber_InImage ( u32 AMin, u32 ASec, u32 AFrac )
{
	return GetSectorNumber_InImage( GetSectorNumber ( AMin, ASec, AFrac ) );
}


u32 CDImage::GetSectorNumber_InImage ( u32 SectorNumber )
{
	int i;
	u32 SectorOffset;
	
	//SectorNumber = GetSectorNumber ( AMin, ASec, AFrac );
	i = GetIndexData_Index ( SectorNumber );
	
	SectorOffset = SectorNumber - IndexData [ i ].SectorNumber;
	
	// if sector is not in image file, then return -1
	if ( IndexData [ i ].SectorNumber_InImage == -1 ) return -1;
	
	return ( IndexData [ i ].SectorNumber_InImage + SectorOffset );
}



void CDImage::UpdateSubQ_Data ( u32 AMin, u32 ASec, u32 AFrac )
{
	UpdateSubQ_Data ( GetSectorNumber ( AMin, ASec, AFrac ) );
}



void CDImage::UpdateSubQ_Data ()
{
	UpdateSubQ_Data ( CurrentSector );
}




void CDImage::UpdateSubQ_Data ( u32 SectorNumber )
{
	u32 SectorNumber_Relative, SectorNumber_Start_Relative;
	int i;
	
	i = GetIndexData_Index ( SectorNumber );
	
	// looking backwards keeps it simple
	//for ( int i = iNumberOfSectors; i >= 0; i-- )
	//{
		//if ( SectorNumber >= IndexData [ i ].SectorNumber )
		//{
			// found start of the index we are looking for
			
			// set index/track
			SubQ_Index = IndexData [ i ].Index;
			SubQ_Track = IndexData [ i ].Track;
			
			// set absolute time
			//SubQ_AMin = AMin;
			//SubQ_ASec = ASec;
			//SubQ_AFrac = AFrac;
			SplitSectorNumber ( SectorNumber, SubQ_AMin, SubQ_ASec, SubQ_AFrac );
			
			// set relative time //
			
			// check if index 0
			if ( !SubQ_Index )
			{
				// count down to index 1 //
				
				SectorNumber_Start_Relative = 150;
				
				// get sector distance from start of sector
				SectorNumber_Relative = SectorNumber - IndexData [ i ].SectorNumber;
				
				// get relative sector number for index 0
				SectorNumber_Relative = SectorNumber_Start_Relative - SectorNumber_Relative;
			}
			else
			{
				SectorNumber_Start_Relative = 0;
				
				// get sector distance from start of sector
				SectorNumber_Relative = SectorNumber - IndexData [ i ].SectorNumber;
			}
			
			// set the relative time
			SplitSectorNumber ( SectorNumber_Relative, SubQ_Min, SubQ_Sec, SubQ_Frac );
			
			// done
			//return;
		//}
	//}
}





