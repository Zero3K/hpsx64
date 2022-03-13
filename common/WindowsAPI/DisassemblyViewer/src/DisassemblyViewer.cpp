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


#include "DisassemblyViewer.h"

using namespace Utilities::Strings;



int Debug_DisassemblyViewer::NextID = Debug_DisassemblyViewer::StartID;


vector<Debug_DisassemblyViewer*> Debug_DisassemblyViewer::ListOfControls;


u32 Debug_DisassemblyViewer::isDialogShowing = false;


Debug_DisassemblyViewer::Debug_DisassemblyViewer ( Debug_BreakPoints* dbp )
{
	NumberOfColumns = 0;
	NumberOfRows = 0;
	
	ProgramCounter = NULL;
	
	Breakpoints = dbp;
	
	// create input box object
	Dialog = new InputBox ();
}


Debug_DisassemblyViewer::~Debug_DisassemblyViewer ()
{
	vector<Debug_DisassemblyViewer*>::iterator i;
	
	DeleteObject ( hf );
	delete lv;
	
	delete Dialog;

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



void Debug_DisassemblyViewer::Create ( WindowClass::Window* ParentWindow, int x, int y, int width, int height, DisAsmFunction DisassemblyFunction_Even, DisAsmFunction DisassemblyFunction_Odd )
{
	static const char* FontName = "Courier New";
	static const int FontSize = 6;
	
	static const int LabelColumn_Width = 50;
	static const int ValueColumn_Width = 22;
	
	int i;
	stringstream ss;
	
	// set function to disassemble
	DisassemblerFunc_Even = DisassemblyFunction_Even;
	DisassemblerFunc_Odd = DisassemblyFunction_Odd;
	
	// set number of bytes showing per row
	//BytesShowingPerRow = BytesToShowPerRow;
	
	// set the parent window
	Parent = ParentWindow;
	
	// set the callback
	//GetCellText = CallbackFunc;

	NumberOfColumns = 0;
	CurrentTopRow = 0;
	
	// create the id
	id = NextID++;
	
	// create the list view
	lv = new WindowClass::ListView ();
	
	// create a list view control with a header row
	hWnd = lv->Create_Dynamic_wHeader ( ParentWindow, x, y, width, height, "", id );
	
	//lv->SetItemCount ( NumberOfRows );

	// set the font for the list view control
	hf = WindowClass::Window::CreateFontObject ( (int) FontSize, (char*) FontName );
	lv->SetFont ( hf );
	
	// put in the label column
	Add_Column ( "Address", LabelColumn_Width );
	Add_Column ( "@", 17 );
	Add_Column ( ">", 17 );
	Add_Column ( "Instruction", 115 );
	Add_Column ( "hex", 52 );
	
	// add into list of controls
	ListOfControls.push_back ( this );

	// add event to call when list scrolls
	lv->AddEvent ( WM_NOTIFY, Event_ListViewUpdate );
}



void Debug_DisassemblyViewer::Add_MemoryDevice ( string NameOfDevice, u32 StartAddress, u32 Size, u8* MemoryDevice_Addr )
{
	MemoryDevice *memdv = new MemoryDevice ();
	
	memdv->Name = NameOfDevice;
	memdv->StartAddress = StartAddress;
	memdv->Size = Size;
	memdv->Memory = MemoryDevice_Addr;
	memdv->StartRowInListView = NumberOfRows;
	
	// add memory device into view
	ListOfMemoryDevices.push_back ( memdv );
	
	// update number of total rows in the list view control
	NumberOfRows += Size;
	
	// update number of rows shown in list view control
	lv->SetItemCount ( NumberOfRows );
}


void Debug_DisassemblyViewer::GoTo_Address ( u32 Address )
{
	int row;
	
	row = GetRowFromAddress ( Address );
	
	// go to that row
	GoTo_Row ( row );
}

// returns -1 when not found
int Debug_DisassemblyViewer::GetIndexFromRow ( int row )
{
	int i;
	
	int RowOffset;
	u32 Address;
	
	for ( i = 0; i < ListOfMemoryDevices.size(); i++ )
	{
		if ( row >= ListOfMemoryDevices [ i ]->StartRowInListView && ( row < ( ListOfMemoryDevices [ i ]->StartRowInListView + ListOfMemoryDevices [ i ]->Size ) ) )
		{
			return i;
		}
	}
	
	return -1;
}

// returns -1 if address not found
int Debug_DisassemblyViewer::GetIndexFromAddress ( u32 Address )
{
	int i;
	u32 Diff, RowOffset;
	
	// for now, strip off top 3 bits of address for MIPS
	Address &= 0x1fffffff;
	
	// loop through list of memory devices looking for address
	for ( i = 0; i < ListOfMemoryDevices.size(); i++ )
	{
		if ( Address >= ListOfMemoryDevices [ i ]->StartAddress && ( Address < ( ListOfMemoryDevices [ i ]->StartAddress + ListOfMemoryDevices [ i ]->Size ) ) )
		{
			return i;
		}
	}
	
	return -1;
}


// returns 0xffffffff when it did not find the row in the list view
u32 Debug_DisassemblyViewer::GetAddressFromRow ( int row )
{
	int i;
	
	int RowOffset;
	u32 Address;
	
	i = GetIndexFromRow ( row );
	
	if ( i < 0 ) return -1;

	// get the row offset
	RowOffset = row - ListOfMemoryDevices [ i ]->StartRowInListView;
	
	// multiply by the number of bytes showing per row
	RowOffset *= BytesShowingPerRow;
	
	// add to the address
	Address = RowOffset + ListOfMemoryDevices [ i ]->StartAddress;
	
	return Address;
}


u32 Debug_DisassemblyViewer::GetMemoryOffsetFromRow ( int row )
{
	int i;
	
	int RowOffset;
	u32 Address, Offset;
	
	i = GetIndexFromRow ( row );
	
	if ( i < 0 ) return -1;

	// get the row offset
	RowOffset = row - ListOfMemoryDevices [ i ]->StartRowInListView;
	
	// multiply by the number of bytes showing per row
	RowOffset *= BytesShowingPerRow;
	
	return RowOffset;
}



int Debug_DisassemblyViewer::GetRowFromAddress ( u32 Address )
{
	int i;
	u32 Diff, RowOffset;
	
	// for now, strip off top 3 bits of address for MIPS
	Address &= 0x1fffffff;
	
	i = GetIndexFromAddress ( Address );
	
	if ( i < 0 ) return -1;
	
	// subtract address from start address
	Diff = Address - ListOfMemoryDevices [ i ]->StartAddress;
	
	// divide by number of bytes showing per row in the viewer
	RowOffset = Diff / BytesShowingPerRow;
	
	// add to the offset for the device
	RowOffset += ListOfMemoryDevices [ i ]->StartRowInListView;

	// return the row
	return RowOffset;
}

u8* Debug_DisassemblyViewer::GetMemoryPtrFromRow ( int row )
{
	int i;
	
	int RowOffset;
	u32 Address;
	
	i = GetIndexFromRow ( row );
	
	if ( i < 0 ) return NULL;
	
	return ListOfMemoryDevices [ i ]->Memory;
}

u8* Debug_DisassemblyViewer::GetMemoryPtrFromAddress ( u32 Address )
{
	int i;
	u32 Diff, RowOffset;
	
	// for now, strip off top 3 bits of address for MIPS
	Address &= 0x1fffffff;
	
	i = GetIndexFromAddress ( Address );
	
	if ( i < 0 ) return NULL;
	
	return ListOfMemoryDevices [ i ]->Memory;
}

void Debug_DisassemblyViewer::GoTo_Row ( int row )
{
	lv->EnsureRowVisible ( row );
}

void Debug_DisassemblyViewer::Add_Column ( string Caption, int Width )
{
	lv->InsertColumn ( NumberOfColumns, WindowClass::ListView::CreateColumn ( NumberOfColumns, (int) (Width), (char*) (Caption.c_str()) ) );
	NumberOfColumns++;
}


void Debug_DisassemblyViewer::Update ()
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




void Debug_DisassemblyViewer::Event_ListViewUpdate ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam )
{
	int i, j, k;
	NMLVDISPINFO *nmlvd;
	LVITEM *lvi;
	unsigned int row, col;
	HWND hWndParent;
	
	int SelectedItem;
	LPNMITEMACTIVATE plnmia;
	
	if (((LPNMHDR)lParam)->code == LVN_GETDISPINFO )
	{
		// make call back to get cell info to display
		nmlvd = (NMLVDISPINFO*) lParam;
		lvi = &(nmlvd)->item;
		
		// get row and column that needs updating
		row = lvi->iItem;
		col = lvi->iSubItem;
		
		// find the memory viewer
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
	
	if (((LPNMHDR)lParam)->code == NM_DBLCLK)
	{
		for ( i = 0; i < ListOfControls.size(); i++ )
		{
			if ( ListOfControls [ i ]->hWnd == hCtrl && ListOfControls [ i ]->id == idCtrl )
			{
				cout << "\ndouble clicked on the list view\n";
				ListOfControls [ i ]->Dialog->ShowDialog ( ListOfControls [ i ]->Parent, "GoTo", "GoTo", Dialog_OkClick, Dialog_CancelClick );
				return;
			}
		}
	}
	
	if (((LPNMHDR)lParam)->code == NM_RCLICK)
	{
		for ( i = 0; i < ListOfControls.size(); i++ )
		{
			if ( ListOfControls [ i ]->hWnd == hCtrl && ListOfControls [ i ]->id == idCtrl )
			{
				cout << "\nright clicked on the list view\n";
				
				// get selected item
				SelectedItem = ListOfControls [ i ]->lv->GetRowOfSelectedItem ();
					
				plnmia = (LPNMITEMACTIVATE) lParam;
				j = ListOfControls [ i ]->Parent->Show_ContextMenu ( plnmia->ptAction.x + 220 + 10, plnmia->ptAction.y + 10, "Toggle Breakpoint | Go To Address" );
				
				// check what option was chosen on context menu if any chosen
				if ( j == 0 )
				{
					// check if breakpoint exists
					if ( isNumeric ( string ( "0x" ) + ListOfControls [ i ]->lv->GetItemText ( SelectedItem, 0 ) ) )
					{
						k = ListOfControls [ i ]->Breakpoints->isPrimaryBreakPoint ( CLng ( string ( "0x" ) + ListOfControls [ i ]->lv->GetItemText ( SelectedItem, 0 ) ) );
						
						// set a breakpoint at address
						if ( k >= 0 )
						{
							// breakpoint exists, so remove it
							ListOfControls [ i ]->Breakpoints->Remove_BreakPoint ( k );
						}
						else
						{
							// breakpoint does not exist, so add breakpoint
							ListOfControls [ i ]->Breakpoints->Add_BreakPoint ( string( "__PC==0x" ) + ListOfControls [ i ]->lv->GetItemText ( SelectedItem, 0 ) , string( "Execute: " ) + ListOfControls [ i ]->lv->GetItemText ( SelectedItem, 0 ), "Execute Breakpoint" );
						}
						
						ListOfControls [ i ]->Update();
					}
				}
				
				if ( j == 1 )
				{
					ListOfControls [ i ]->Dialog->ShowDialog ( ListOfControls [ i ]->Parent, "GoTo", "GoTo", Dialog_OkClick, Dialog_CancelClick );
				}
				
				return;
			}
		}
	}

}


string Debug_DisassemblyViewer::GetCellText ( int row, int column )
{
	stringstream ss;
	
	u32 isOddAddress;
	
	ss.str ( "" );
	
	if ( !column )
	{
		// this is the label column
		ss << hex << setw ( 8 ) << setfill ( '0' ) << GetAddressFromRow ( row );
	}
	else
	{
		switch ( column )
		{
			case 1:
			
				// check if address is at a breakpoint
				if ( Breakpoints->isPrimaryBreakPoint ( GetAddressFromRow ( row ) ) >= 0 )
				{
					// show breakpoint indicator next to address in the disassembly window
					ss << "@";
				}
				
				break;
				
			case 2:
				if ( ProgramCounter )
				{
					if ( ( *ProgramCounter & 0x1fffffff ) == GetAddressFromRow ( row ) )
					{
						ss << ">";
					}
				}
				
				break;
			
			case 3:
				// make sure the disassembly function is available
				isOddAddress = 1 & ( ( GetMemoryOffsetFromRow ( row ) + ( column - 1 ) ) >> 2 );
				
				if ( isOddAddress )
				{
					if ( DisassemblerFunc_Odd )
					{
						ss << DisassemblerFunc_Odd ( ((u32*) GetMemoryPtrFromRow ( row )) [ ( GetMemoryOffsetFromRow ( row ) + ( column - 1 ) ) >> 2 ] );
					}
					else
					{
						if ( DisassemblerFunc_Even )
						{
							ss << DisassemblerFunc_Even ( ((u32*) GetMemoryPtrFromRow ( row )) [ ( GetMemoryOffsetFromRow ( row ) + ( column - 1 ) ) >> 2 ] );
						}
					}
				}
				else
				{
					if ( DisassemblerFunc_Even )
					{
						ss << DisassemblerFunc_Even ( ((u32*) GetMemoryPtrFromRow ( row )) [ ( GetMemoryOffsetFromRow ( row ) + ( column - 1 ) ) >> 2 ] );
					}
				}
				
				break;
			
			
			case 4:
				ss << hex << setw ( 8 ) << setfill ( '0' ) << ( ((u32*) GetMemoryPtrFromRow ( row )) [ ( GetMemoryOffsetFromRow ( row ) + ( column - 1 ) ) >> 2 ] );
				break;

		}
	}
	
	return ss.str().c_str();
}






void Debug_DisassemblyViewer::Dialog_OkClick ( string input )
{
	int i;
	HWND Parent_hWnd;
	u32 Address;
	
	//cout << "\nClicked the OK button";
	
	
	//cout << "\nParent Window #1=" << (unsigned long) Parent_hWnd;
	
	for ( i = 0; i < ListOfControls.size (); i++ )
	{
		if ( ListOfControls [ i ]->Dialog->isDialogShowing )
		{
			if ( from_string ( Address, input.c_str (), hex ) )
			{
				ListOfControls [ i ]->GoTo_Address ( Address );
			}
			
			break;
		}
	}
	
}

void Debug_DisassemblyViewer::Dialog_CancelClick ( string input )
{
}





