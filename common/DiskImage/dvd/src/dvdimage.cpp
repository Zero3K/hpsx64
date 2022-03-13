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


#include "cdimage.h"


using namespace DiskImage;


bool DVDImage::OpenDiskImage ( char* DiskImagePath )
{
	image = fopen64 ( DiskImagePath, "r" );
	
	if ( !image ) return false;
	
	return true;
}

bool DVDImage::CloseDiskImage ()
{
	if ( !fclose ( image ) ) return true;
	
	return false;
}


void DVDImage::SeekSector ( u64 LogicalSectorNumber )
{
	fseeko64 ( image, ( LogicalSectorNumber ) * c_SectorSize, SEEK_SET );
}

int DVDImage::ReadData ( u8* Data )
{
	fread ( Data, c_SectorSize, 1, image );
	return 2048;
}






