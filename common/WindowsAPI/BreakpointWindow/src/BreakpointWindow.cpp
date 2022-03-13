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


#include "BreakpointWindow.h"

using namespace Utilities::Strings;


int Debug_BreakpointWindow::NextID = Debug_BreakpointWindow::StartID;
vector<Debug_BreakpointWindow*> Debug_BreakpointWindow::ListOf_BreakpointWindows;



Debug_BreakpointWindow::~Debug_BreakpointWindow ()
{
}


bool Debug_BreakpointWindow::Create( WindowClass::Window* ParentWindow, int x, int y, int width, int height )
{
	static const char* Headings [] = { ">", "Breakpoint", "Condition" };
	static constexpr double ColumnWidths_Ratio [] = { 0.05, 0.5, 0.5 };
	
	static const char* FontName = "Courier New";
	static const int FontSize = 6;
	
	int i;
	
	// set the parent window
	Parent = ParentWindow;

	// create the id
	id = NextID++;
	
	// create the list view
	lv = new WindowClass::ListView ();
	
	// create a list view control with a header row
	hWnd = lv->Create_wHeader ( ParentWindow, x, y, width, height, "", id );
	
	// add the columns
	for ( i = 0; i < (sizeof(Headings)/sizeof(Headings[0])); i++ )
	{
		lv->InsertColumn ( i, WindowClass::ListView::CreateColumn ( i, (int) (ColumnWidths_Ratio [ i ] * width), (char*) (Headings [ i ]) ) );
	}
	
	// set the font for the list view control
	// *** todo *** delete the font when done with it
	hf = WindowClass::Window::CreateFontObject ( (int) FontSize, (char*) FontName );
	lv->SetFont ( hf );
	
	// save reference so we can handle events
	ListOf_BreakpointWindows.push_back ( this );
	
	// add event to modify variables when you double click on any of them
	lv->AddEvent ( WM_NOTIFY, Debug_BreakpointWindow::BreakpointWindow_Event );
	
	return true;
}


bool Debug_BreakpointWindow::Update ()
{
	int i;
	
	// clear items in list view control
	lv->Reset ();
	
	// put in the breakpoints into list view control
	for ( i = 0; i < dbp->NumberOfBreakPoints; i++ )
	{
		// make a row for the item
		lv->InsertRow ( i );
		
		// put in the last breakpoint indicator
		if ( i == dbp->Get_LastBreakPoint () ) lv->SetItemText ( i, 0, ">" );
		
		// put in the name for breakpoint
		lv->SetItemText ( i, 1, dbp->Get_BreakPoint_Name ( i ).c_str () );
		
		// put in the condition for breakpoint
		lv->SetItemText ( i, 2, dbp->Get_BreakPoint_Condition ( i ).c_str () );
	}

	return true;
}



void Debug_BreakpointWindow::BreakpointWindow_Event ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam )
{
	int i, j;
	
	static const /*wchar_t**/ char* ContextMenu_Items [] = { "Remove", "Add" };
	static const int ContextMenu_IDs [] = { ID_REMOVE, ID_ADD };
	
	int SelectedItem;
	LPNMITEMACTIVATE plnmia;

	// make sure this is a double click event
	if ( ((LPNMHDR)lParam)->code == NM_DBLCLK )
	{
		cout << "\nDebug_BreakpointWindow::Double Click";

		// find the Breakpoint Window that was clicked on
		for ( i = 0; i < ListOf_BreakpointWindows.size (); i++ )
		{
			if ( ListOf_BreakpointWindows [ i ]->hWnd == hCtrl && ListOf_BreakpointWindows [ i ]->id == idCtrl )
			{
				//cout << "\nList-View Clicked";
				
				//ListOf_BreakpointWindows [ i ]->ShowDialog_Modify ();
				ListOf_BreakpointWindows [ i ]->bpd.Show();
				
				break;
			}
		}
		
		return;
	}
	
	// if this is a right click, then show context menu
	if ( ((LPNMHDR)lParam)->code == NM_RCLICK )
	{
		cout << "\nDebug_BreakpointWindow::Right Click";
		
		for ( i = 0; i < ListOf_BreakpointWindows.size (); i++ )
		{
			if ( ListOf_BreakpointWindows [ i ]->hWnd == hCtrl && ListOf_BreakpointWindows [ i ]->id == idCtrl )
			{
				cout << "\nListView::Right Click";

				cout << "\nAbout to call TrackPopupMenu";
				
				// get selected item
				SelectedItem = ListOf_BreakpointWindows [ i ]->lv->GetRowOfSelectedItem ();
					
				plnmia = (LPNMITEMACTIVATE) lParam;
				j = ListOf_BreakpointWindows [ i ]->Parent->Show_ContextMenu ( plnmia->ptAction.x + 220 + 10, plnmia->ptAction.y + 10, "Remove | Add" );
				
				// check if they chose to remove breakpoint
				if ( j == 0 )
				{
					// remove selected breakpoint
					ListOf_BreakpointWindows [ i ]->dbp->Remove_BreakPoint ( SelectedItem );
					ListOf_BreakpointWindows [ i ]->Update ();
				}
				
				if ( j == 1 )
				{
					ListOf_BreakpointWindows [ i ]->bpd.Show();
				}
				
				/*
				plnmia = (LPNMITEMACTIVATE) lParam;
				
				cout << "\nCreatePopupMenu";
				ListOf_BreakpointWindows [ i ]->hContextMenu = CreatePopupMenu();
			
				for ( j = 0; j < (sizeof(ContextMenu_Items)/sizeof(ContextMenu_Items[0])); j++ )
				{
					cout << "\nAppending menu item";
					AppendMenu( ListOf_BreakpointWindows [ i ]->hContextMenu, MF_STRING, ContextMenu_IDs [ j ], (LPCSTR) ContextMenu_Items [ j ] );
				}
				
				cout << "\nAbout to track pop up menu.";
			
				//InsertMenu(hContextMenu, 0, MF_BYPOSITION | MF_STRING, ID_REMOVE, L"Play");
				//InsertMenu(hContextMenu, 0, MF_BYPOSITION | MF_STRING, ID_ADD, L"Play");
				//SetForegroundWindow(ListOf_BreakpointWindows [ i ]->Parent->hWnd);
				cout << "\nMouse Click x=" << plnmia->ptAction.x << " y=" << plnmia->ptAction.y;
				cout << "\nTrackPopupMenu=" << TrackPopupMenu(ListOf_BreakpointWindows [ i ]->hContextMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, plnmia->ptAction.x + 220 + 10, plnmia->ptAction.y + 10, 0, ListOf_BreakpointWindows [ i ]->Parent->hWnd, NULL);
				*/
				
				return;
			}
		}
	}

}



void Debug_BreakpointWindow::ShowDialog_Add ()
{
}


void Debug_BreakpointWindow::ShowDialog_Modify ()
{
}


void Debug_BreakpointWindow::KillDialog_Add ()
{
}

void Debug_BreakpointWindow::KillDialog_Modify ()
{
}






volatile u32 Debug_BreakpointWindow::BreakPoint_Dialog::isShowing_Global = 0;


Debug_BreakpointWindow::BreakPoint_Dialog::BreakPoint_Dialog ()
{
	// dialog is not showing
	Lock_Exchange32 ( (long&)Active, 0 );
}

void Debug_BreakpointWindow::BreakPoint_Dialog::Show ()
{
	// make sure no windows are showing for any breakpoint dialogs
	if ( !isShowing_Global )
	{
		// activate window
		Lock_Exchange32 ( (long&)isShowing_Global, 1 );
		Lock_Exchange32 ( (long&)Active, 1 );
		
		static const char* Dialog_FontName = "Times New Roman";
		static const int Dialog_FontSize = 8;

		static const char* Dialog_caption = "Add Breakpoint";
		static const int Dialog_x = 10;
		static const int Dialog_y = 10;
		static const int Dialog_width = 350;
		static const int Dialog_height = 150;
		
		Dialog = new WindowClass::Window ();
		Dialog->Create ( Dialog_caption, Dialog_x, Dialog_y, Dialog_width, Dialog_height );
		Dialog->Set_Font ( Dialog->CreateFontObject ( 8 ) );

		static const char* RadioBtn_Execute_caption = "Execute";
		static const int RadioBtn_Execute_id = 0x3001;
		static const int RadioBtn_Execute_x = 10;
		static const int RadioBtn_Execute_y = 10;
		static const int RadioBtn_Execute_width = 75;
		static const int RadioBtn_Execute_height = 20;
		
		RadioBtn_Execute = new WindowClass::Button ();
		RadioBtn_Execute->Create_RadioButtonGroup ( Dialog, RadioBtn_Execute_x, RadioBtn_Execute_y, RadioBtn_Execute_width, RadioBtn_Execute_height, RadioBtn_Execute_caption, RadioBtn_Execute_id );
		RadioBtn_Execute->AddEvent ( NULL, DefaultCheckBoxHandler );
		
		static const char* RadioBtn_Memory_caption = "Memory";
		static const int RadioBtn_Memory_id = 0x3002;
		static const int RadioBtn_Memory_x = RadioBtn_Execute_x + RadioBtn_Execute_width + 20;
		static const int RadioBtn_Memory_y = RadioBtn_Execute_y;
		static const int RadioBtn_Memory_width = 75;
		static const int RadioBtn_Memory_height = RadioBtn_Execute_height;
		
		RadioBtn_Memory = new WindowClass::Button ();
		RadioBtn_Memory->Create_RadioButton ( Dialog, RadioBtn_Memory_x, RadioBtn_Memory_y, RadioBtn_Memory_width, RadioBtn_Memory_height, RadioBtn_Memory_caption, RadioBtn_Memory_id );
		RadioBtn_Memory->AddEvent ( NULL, DefaultCheckBoxHandler );
		
		static const char* CheckBox_Read_caption = "Read";
		static const int CheckBox_Read_id = 0x3003;
		static const int CheckBox_Read_x = RadioBtn_Memory_x + RadioBtn_Memory_width + 5;
		static const int CheckBox_Read_y = RadioBtn_Execute_y;
		static const int CheckBox_Read_width = 50;
		static const int CheckBox_Read_height = RadioBtn_Execute_height;
		
		CheckBox_Read = new WindowClass::Button ();
		CheckBox_Read->Create_CheckBox ( Dialog, CheckBox_Read_x, CheckBox_Read_y, CheckBox_Read_width, CheckBox_Read_height, CheckBox_Read_caption, CheckBox_Read_id );
		CheckBox_Read->AddEvent ( NULL, DefaultCheckBoxHandler );
		
		static const char* CheckBox_Write_caption = "Write";
		static const int CheckBox_Write_id = 0x3004;
		static const int CheckBox_Write_x = CheckBox_Read_x + CheckBox_Read_width + 5;
		static const int CheckBox_Write_y = RadioBtn_Execute_y;
		static const int CheckBox_Write_width = 50;
		static const int CheckBox_Write_height = RadioBtn_Execute_height;
		
		CheckBox_Write = new WindowClass::Button ();
		CheckBox_Write->Create_CheckBox ( Dialog, CheckBox_Write_x, CheckBox_Write_y, CheckBox_Write_width, CheckBox_Write_height, CheckBox_Write_caption, CheckBox_Write_id );
		CheckBox_Write->AddEvent ( NULL, DefaultCheckBoxHandler );
		
		static const char* Label_Address_caption = "Address";
		static const int Label_Address_id = 0x3005;
		static const int Label_Address_x = RadioBtn_Execute_x;
		static const int Label_Address_y = RadioBtn_Execute_y + RadioBtn_Execute_height + 5;
		static const int Label_Address_width = 70;
		static const int Label_Address_height = 20;
		
		Label_Address = new WindowClass::Static ();
		Label_Address->Create_Text ( Dialog, Label_Address_x, Label_Address_y, Label_Address_width, Label_Address_height, Label_Address_caption, Label_Address_id );
		
		static const int Edit_Address_id = 0x3006;
		static const int Edit_Address_x = Label_Address_x + Label_Address_width + 5;
		static const int Edit_Address_y = Label_Address_y;
		static const int Edit_Address_width = 100;
		static const int Edit_Address_height = 20;
		
		Edit_Address = new WindowClass::Edit ();
		Edit_Address->Create ( Dialog, Edit_Address_x, Edit_Address_y, Edit_Address_width, Edit_Address_height, "", Edit_Address_id );
		
		static const char* Label_Size_caption = "Size";
		static const int Label_Size_id = 0x3007;
		static const int Label_Size_x = Edit_Address_x + Edit_Address_width + 5;
		static const int Label_Size_y = Label_Address_y;
		static const int Label_Size_width = 50;
		static const int Label_Size_height = 20;
		
		Label_Size = new WindowClass::Static ();
		Label_Size->Create_Text ( Dialog, Label_Size_x, Label_Size_y, Label_Size_width, Label_Size_height, Label_Size_caption, Label_Size_id );
		
		static const int Edit_Size_id = 0x3008;
		static const int Edit_Size_x = Label_Size_x + Label_Size_width + 5;
		static const int Edit_Size_y = Label_Address_y;
		static const int Edit_Size_width = 30;
		static const int Edit_Size_height = 20;
		
		Edit_Size = new WindowClass::Edit ();
		Edit_Size->Create ( Dialog, Edit_Size_x, Edit_Size_y, Edit_Size_width, Edit_Size_height, "", Edit_Size_id );
		
		static const char* Label_Condition_caption = "Condition";
		static const int Label_Condition_id = 0x3009;
		static const int Label_Condition_x = Label_Address_x;
		static const int Label_Condition_y = Label_Address_y + Label_Address_height + 5;
		static const int Label_Condition_width = 80;
		static const int Label_Condition_height = 20;
		
		Label_Condition = new WindowClass::Static ();
		Label_Condition->Create_Text ( Dialog, Label_Condition_x, Label_Condition_y, Label_Condition_width, Label_Condition_height, Label_Condition_caption, Label_Condition_id );
		
		static const int Edit_Condition_id = 0x300a;
		static const int Edit_Condition_x = Label_Condition_x + Label_Condition_width + 5;
		static const int Edit_Condition_y = Label_Condition_y;
		static const int Edit_Condition_width = 200;
		static const int Edit_Condition_height = 20;
		
		Edit_Condition = new WindowClass::Edit ();
		Edit_Condition->Create ( Dialog, Edit_Condition_x, Edit_Condition_y, Edit_Condition_width, Edit_Condition_height, "", Edit_Condition_id );
		
		static const char* CmdBtn_Add_caption = "Add";
		static const int CmdBtn_Add_id = 0x300b;
		static const int CmdBtn_Add_x = Label_Condition_x;
		static const int CmdBtn_Add_y = Label_Condition_y + Label_Condition_height + 15;
		static const int CmdBtn_Add_width = 50;
		static const int CmdBtn_Add_height = 20;
		
		CmdBtn_Add = new WindowClass::Button ();
		CmdBtn_Add->Create_CmdButton ( Dialog, CmdBtn_Add_x, CmdBtn_Add_y, CmdBtn_Add_width, CmdBtn_Add_height, CmdBtn_Add_caption, CmdBtn_Add_id );
		CmdBtn_Add->AddEvent ( NULL, Add_Click );
		
		static const char* CmdBtn_Cancel_caption = "Cancel";
		static const int CmdBtn_Cancel_id = 0x300c;
		static const int CmdBtn_Cancel_x = CmdBtn_Add_x + CmdBtn_Add_width + 10;
		static const int CmdBtn_Cancel_y = CmdBtn_Add_y;
		static const int CmdBtn_Cancel_width = 60;
		static const int CmdBtn_Cancel_height = 20;
		
		CmdBtn_Cancel = new WindowClass::Button ();
		CmdBtn_Cancel->Create_CmdButton ( Dialog, CmdBtn_Cancel_x, CmdBtn_Cancel_y, CmdBtn_Cancel_width, CmdBtn_Cancel_height, CmdBtn_Cancel_caption, CmdBtn_Cancel_id );
		CmdBtn_Cancel->AddEvent ( NULL, Cancel_Click );
	}
}



void Debug_BreakpointWindow::BreakPoint_Dialog::Kill ()
{
	// deactivate window if it is showing
	if ( Active )
	{
		Lock_Exchange32 ( (long&)isShowing_Global, 0 );
		Lock_Exchange32 ( (long&)Active, 0 );

		delete RadioBtn_Execute;
		delete RadioBtn_Memory;
		delete CheckBox_Read;
		delete CheckBox_Write;
		delete Label_Address;
		delete Edit_Address;
		delete Label_Size;
		delete Edit_Size;
		delete Label_Condition;
		delete Edit_Condition;
		delete CmdBtn_Add;
		delete CmdBtn_Cancel;
		delete Dialog;
	}
}


void Debug_BreakpointWindow::BreakPoint_Dialog::Add_Breakpoint ( Debug_BreakPoints* dbp )
{
	static const unsigned long AddressMask = 0x1fffffff;
	string breakpoint = "";
	string Reason = "";
	BreakPoint_Dialog *bd;
	u32 Address;
	int i;
	
	cout << "\nDebug_BreakpointWindow::BreakPoint_Dialog::Add_Breakpoint";
	
	// save data entered into dialog
	bExecute = ( RadioBtn_Execute->GetCheck () == BST_CHECKED );
	bMemory = ( RadioBtn_Memory->GetCheck () == BST_CHECKED );
	bRead = ( CheckBox_Read->GetCheck () == BST_CHECKED );
	bWrite = ( CheckBox_Write->GetCheck () == BST_CHECKED );
	sAddress = Edit_Address->GetText ();
	sSize = Edit_Size->GetText ();
	sCondition = Edit_Condition->GetText ();
	
	cout << "\nbExecute=" << bExecute << " bMemory=" << bMemory << " bRead=" << bRead << " bWrite=" << bWrite << " sAddress=" << sAddress << " sSize=" << sSize << " sCondition=" << sCondition;
	
	// make sure that the address is numeric
	if ( isNumeric( sAddress ) )
	{
		cout << "\nAddress is numeric.";
		
		// address is a valid numeric value - don't forget to clear top 3 bits of address
		Address = CLng ( sAddress ) & AddressMask;
		
		cout << "\nAddress=" << Address;
		
		// check if execute break point
		if ( bExecute )
		{
			// execute breakpoint
			
			// reason is execute breakpoint
			Reason += "Execute Breakpoint ";
			
			breakpoint += "__PC==" + sAddress;
			
			// check if there is a condition
			if ( sCondition != "" )
			{
				// add in condition
				breakpoint += "&&" + sCondition;
			}
			
			cout << "\nExecute Breakpoint=" << breakpoint.c_str();
			
			// add execute breakpoint
			i = dbp->Add_BreakPoint ( breakpoint, "Execute:" + sAddress, "Execute Breakpoint " + sAddress + " w/ " + sCondition );
			
			cout << "\nAdded breakpoint=" << i;
		}
		else if ( bMemory )
		{
			// memory breakpoint
			
			// reason is memory breakpoint
			Reason += "Memory-";
			
			// check if read memory breakpoint
			if ( bRead && !bWrite )
			{
				breakpoint += "__LastReadAddress==" + sAddress;
				Reason += "R";
				
				// check if there was a size specified
				if ( sSize != "" )
				{
					// size is not blank. make sure it is a number
					if ( isNumeric ( sSize ) )
					{
						// specify size
						breakpoint += "&!" + sSize;
					}
				}
			}
			else if ( bWrite && !bRead )
			{
				if ( breakpoint != "" )
				{
					breakpoint += "&&";
				}
				
				breakpoint += "__LastWriteAddress==" + sAddress;
				Reason += "W";
				
				// check if there was a size specified
				if ( sSize != "" )
				{
					// size is not blank. make sure it is a number
					if ( isNumeric ( sSize ) )
					{
						// specify size
						breakpoint += "&!" + sSize;
					}
				}
			}
			else
			{
				if ( breakpoint != "" )
				{
					breakpoint += "&&";
				}
				
				breakpoint += "__LastReadWriteAddress==" + sAddress;
				Reason += "RW";
				
				// check if there was a size specified
				if ( sSize != "" )
				{
					// size is not blank. make sure it is a number
					if ( isNumeric ( sSize ) )
					{
						// specify size
						breakpoint += "&!" + sSize;
					}
				}
			}
			
			// check if there is a condition
			if ( sCondition != "" )
			{
				if ( breakpoint != "" )
				{
					breakpoint += "&&";
				}
				
				// add in condition
				breakpoint += sCondition;
			}
			
			cout << "\nMemory Breakpoint=" << breakpoint.c_str();
			
			// add memory breakpoint
			i = dbp->Add_BreakPoint ( breakpoint, Reason, Reason + sAddress + " w/ " + sCondition );
			
			cout << "\nAdded breakpoint=" << i;
		}
		
		return;
	}
	else if ( Replace( sAddress, " ", "" ) == "" )
	{
		cout << "\nAddress is not numeric but there is a condition.";
		
		// strictly conditional breakpoint
		breakpoint += sCondition;
		
		cout << "\nCondition Breakpoint=" << breakpoint.c_str();
			
		// add memory breakpoint
		i = dbp->Add_BreakPoint ( breakpoint, "Condition:" + sCondition, "Condition: " + sCondition );
		
		cout << "\nAdded breakpoint=" << i;
		
		return;
	}
	
	cout << "\nThere was a problem.";
}


void Debug_BreakpointWindow::BreakPoint_Dialog::Add_Click ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam )
{
	int i;
	
	cout << "\nClicked Add Button.";
	
	// find breakpoint dialog
	for ( i = 0; i < ListOf_BreakpointWindows.size (); i++ )
	{
		if ( ListOf_BreakpointWindows [ i ]->bpd.Active )
		{
			cout << "\nFound Dialog.\n";
			
			ListOf_BreakpointWindows [ i ]->bpd.Add_Breakpoint( ListOf_BreakpointWindows [ i ]->dbp );
			ListOf_BreakpointWindows [ i ]->bpd.Kill();
			ListOf_BreakpointWindows [ i ]->Update();
			
			return;
		}
			
	}
}

void Debug_BreakpointWindow::BreakPoint_Dialog::Cancel_Click ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam )
{
	int i;
	
	cout << "\nClicked Cancel Button.";
	
	// find breakpoint dialog
	for ( i = 0; i < ListOf_BreakpointWindows.size (); i++ )
	{
		if ( ListOf_BreakpointWindows [ i ]->bpd.Active )
		{
			cout << "\nFound Dialog.\n";
			
			ListOf_BreakpointWindows [ i ]->bpd.Kill();
			ListOf_BreakpointWindows [ i ]->Update();
			
			return;
		}
	}
}


void Debug_BreakpointWindow::BreakPoint_Dialog::DefaultCheckBoxHandler ( HWND hCtrl, int id, unsigned int message, WPARAM wParam, LPARAM lParam )
{
	if ( WindowClass::Button::GetCheck ( hCtrl ) == BST_CHECKED )
	{
		WindowClass::Button::SetCheck ( hCtrl, BST_UNCHECKED );
	}
	else
	{
		WindowClass::Button::SetCheck ( hCtrl, BST_CHECKED );
	}
}




