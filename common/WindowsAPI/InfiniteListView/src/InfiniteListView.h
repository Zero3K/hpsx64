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


#ifndef _INFINITELISTVIEW_H_
#define _INFINITELISTVIEW_H_


#include "WinApiHandler.h"

#include <string>


class InfiniteListView
{
	// I'll make this like a control with infinitely scrolling rows
	
	static const int StartID = 5000;
	static int NextID;

	int id;
	HWND hWnd;
	HFONT hf;
	
	int NumberOfColumns;
	int NumberOfRows;
	
	int CurrentTopRow;

	// when control needs to draw a cell, it calls this call back function with the row and column of the cell
	typedef string (*CellCallback) ( unsigned int row, unsigned int column );
	
	CellCallback GetCellText;
	
	WindowClass::Window *Parent;
	WindowClass::ListView *lv;
	
	static vector<InfiniteListView*> ListOfControls;

public:
	
	// constructor
	InfiniteListView ();
	
	// destructor
	~InfiniteListView ();
	
	// maximum number of rows is around 0x4000000
	void Create ( WindowClass::Window* ParentWindow, int x, int y, int width, int height, unsigned int NumberOfRows, CellCallback cb );
	
	void AddColumn ( string Caption, int Width );
	
	void GoToRow ( int row );
	
	void Update ();
	
	static void Event_ListBoxUpdate ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam );
};


#endif





