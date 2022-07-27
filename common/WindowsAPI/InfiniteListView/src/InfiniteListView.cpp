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


#include "InfiniteListView.h"


int InfiniteListView::NextID = InfiniteListView::StartID;


vector<InfiniteListView*> InfiniteListView::ListOfControls;


InfiniteListView::InfiniteListView ()
{
	NumberOfColumns = 0;
	NumberOfRows = 0;
}


InfiniteListView::~InfiniteListView ()
{
	vector<InfiniteListView*>::iterator i;
	
	DeleteObject ( hf );
	delete lv;

	// delete the DebugValueList from the list of value lists
	for ( i = ListOfControls.begin(); i != ListOfControls.end(); i++ )
	{
		if ( (*i)->id == id && (*i)->hWnd == hWnd )
		{
			// object is already being deleted, so don't delete or it crashes!
			// just erase from list

			// erase element in list
			ListOfControls.erase ( i );
			
			// iterator is no longer good after doing an erase
			break;
		}
	}
}



void InfiniteListView::Create ( WindowClass::Window* ParentWindow, int x, int y, int width, int height, unsigned int NumberOfRows, CellCallback CallbackFunc )
{
	static const char* FontName = "Courier New";
	static const int FontSize = 6;
	
	// set the parent window
	Parent = ParentWindow;
	
	// set the callback
	GetCellText = CallbackFunc;

	NumberOfColumns = 0;
	CurrentTopRow = 0;
	
	// create the id
	id = NextID++;
	
	// create the list view
	lv = new WindowClass::ListView ();
	
	// create a list view control with a header row
	hWnd = lv->Create_Dynamic_wHeader ( ParentWindow, x, y, width, height, "", id );
	
	lv->SetItemCount ( NumberOfRows );

	// set the font for the list view control
	hf = WindowClass::Window::CreateFontObject ( (int) FontSize, (char*) FontName );
	lv->SetFont ( hf );
	
	// add into list of controls
	ListOfControls.push_back ( this );

	// add event to call when list scrolls
	lv->AddEvent ( WM_NOTIFY, Event_ListBoxUpdate );
}


void InfiniteListView::GoToRow ( int row )
{
	lv->EnsureRowVisible ( row );
}


void InfiniteListView::Update ()
{
	int w, h;
	
	Parent->Redraw ();
	
/*	
	for ( w = 0; w < NumberOfColumns; w++ )
	{
		for ( h = 0; h < NumberOfRows; h++ )
		{
			lv->SetItemText ( h, w, (char*) GetCellText ( h, w ).c_str() );
		}
	}
*/
}


void InfiniteListView::AddColumn ( string Caption, int Width )
{
	lv->InsertColumn ( NumberOfColumns, WindowClass::ListView::CreateColumn ( NumberOfColumns, (int) (Width), (char*) (Caption.c_str()) ) );
	NumberOfColumns++;
}



void InfiniteListView::Event_ListBoxUpdate ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam )
{
	int i;
	NMLVDISPINFO *nmlvd;
	LVITEM *lvi;
	unsigned int row, col;
	
	if (((LPNMHDR)lParam)->code == LVN_GETDISPINFO )
	{
		// make call back to get cell info to display
		nmlvd = (NMLVDISPINFO*) lParam;
		lvi = &(nmlvd)->item;
		
		// get row and column that needs updating
		row = lvi->iItem;
		col = lvi->iSubItem;
		
		// find the list view
		for ( i = 0; i < ListOfControls.size(); i++ )
		{
			if ( ListOfControls [ i ]->hWnd == hCtrl && ListOfControls [ i ]->id == idCtrl )
			{
				// make sure it is text being requested
				if ( lvi->mask & LVIF_TEXT )
				{
					lstrcpyn( lvi->pszText, ListOfControls [ i ]->GetCellText ( row, col ).c_str(), lvi->cchTextMax);
				}

				// no need to loop further
				break;
			}
			
		}
		
		// done
		return;
	}
	
	if (((LPNMHDR)lParam)->code == NM_CLICK)
	{
		cout << "\nclicked on the list view\n";
	}
}






