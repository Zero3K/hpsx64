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


#ifndef _DEBUGMEMORYVIEWER_H_
#define _DEBUGMEMORYVIEWER_H_


#include "WinApiHandler.h"
#include "InputBox.h"

#include <string>


class Debug_MemoryViewer
{
	// I'll make this like a control with infinitely scrolling rows
	
	static const int StartID = 20000;
	static int NextID;

	int id;
	HWND hWnd;
	HFONT hf;
	
	int NumberOfColumns;
	int NumberOfRows;
	int BytesShowingPerRow;
	
	int CurrentTopRow;

	// when control needs to draw a cell, it calls this call back function with the row and column of the cell
	typedef string (*CellCallback) ( unsigned int row, unsigned int column );
	
	WindowClass::Window *Parent;
	WindowClass::ListView *lv;
	
	struct MemoryDevice
	{
		string Name;
		u32 StartAddress;
		u32 Size;
		
		u8* Memory;
		
		int StartRowInListView;
	};
	
	vector<MemoryDevice*> ListOfMemoryDevices;
	
	static vector<Debug_MemoryViewer*> ListOfControls;

	void Add_Column ( string Caption, int Width );
	void GoTo_Row ( int row );
	
	// get the device index in the list of memory devices from the row in list view
	int GetIndexFromRow ( int row );
	
	// get the device index in the list of memory devices from the address
	int GetIndexFromAddress ( u32 Address );
	
	// get the memory address from the list view row number
	// returns 0xffffffff when it did not find the row in the list view
	u32 GetAddressFromRow ( int row );
	
	// get the list view row number from the memory address
	int GetRowFromAddress ( u32 Address );



	static bool from_string (u32& t, 
					 const std::string& s, 
					 std::ios_base& (*f)(std::ios_base&))
	{
	  std::istringstream iss(s);
	  return !(iss >> f >> t).fail();
	}

public:
	
	// constructor
	Debug_MemoryViewer ();
	
	// destructor
	~Debug_MemoryViewer ();

	// add a memory device to show in viewer
	void Add_MemoryDevice ( string NameOfDevice, u32 StartAddress, u32 Size, u8* MemoryDevice_Addr );

	InputBox *Dialog;
	static void Dialog_OkClick ( string input );
	static void Dialog_CancelClick ( string input );
	
	u8* GetMemoryPtrFromRow ( int row );
	u8* GetMemoryPtrFromAddress ( u32 Address );

	u32 GetMemoryOffsetFromRow ( int row );

	// have viewer jump to a memory address
	void GoTo_Address ( u32 Address );
	
	// maximum number of rows is around 0x4000000
	void Create ( WindowClass::Window* ParentWindow, int x, int y, int width, int height, int BytesToShowPerRow );
	
	
	void Update ();
	
	static void Event_ListViewUpdate ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam );
	
	// this will be called by GUI Thread to get the text for the list view
	string GetCellText ( int row, int column );

	
	///////// GoTo Dialog ////////////
	static u32 isDialogShowing;
	HWND Dialog1_hWnd;
	WindowClass::Window *wDialog;
	WindowClass::Static *stLabel1;
	WindowClass::Edit *editBox1;
	WindowClass::Button *CmdButton1;
	WindowClass::Button *CmdButton2;

	void ShowDialog_SetValue ();
	void KillDialog_SetValue ();
};


#endif





