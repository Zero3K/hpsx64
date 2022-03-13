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


#ifndef _DEBUGVALUELIST_H_
#define _DEBUGVALUELIST_H_


#include "WinApiHandler.h"

#include <string>

using namespace x64ThreadSafe::Utilities;

template<typename Type>
class DebugValueList
{
	static const int StartID = 4000;
	static int NextID;

	// this is the size of the variables in the value list in bytes. Will be used to allow variable to be specified in conditional breakpoints
	int ValueSizeInBytes;
	
	WindowClass::Window *Parent;
	
	int id;
	HWND hWnd;
	
	volatile unsigned long ListWasDoubleClicked;
	
	bool Locked;
	
	HFONT hf;
	
	WindowClass::ListView *lv;
	
	vector<Type*> ListOfVariables;
	
	static vector<DebugValueList*> ListOfValueLists;

	/////// Dialog Stuff /////////////
	int SelectedItemIndex;
	HWND Dialog1_hWnd;
	WindowClass::Window *wDialog;
	WindowClass::Static *stLabel1;
	WindowClass::Edit *editBox1;
	WindowClass::Button *CmdButton1;
	WindowClass::Button *CmdButton2;

	
public:

	int NumberOfColumns;
	bool ShowHex, ShowDec;

	DebugValueList () { ListWasDoubleClicked = 0; WindowClass::ListView::InitCommonControls (); ValueSizeInBytes = sizeof ( Type ); }

	~DebugValueList ();

	bool Create ( WindowClass::Window* ParentWindow, int x, int y, int width, int height, bool _ShowHex = true, bool _ShowDec = true );
	
	// returns a pointer to the variable if it is found, otherwise returns NULL if it is not found. Will be used for conditional breakpoints
	Type* FindVariable ( string Name );
	
	// this will look for the variable in all the value lists created for Type
	static Type* FindVariableGlobal ( string Name )
	{
		int i;
		Type* Var;
		
		for ( i = 0; i < ListOfValueLists.size(); i++ )
		{
			Var = ListOfValueLists [ i ]->FindVariable ( Name );
			
			if ( Var ) return Var;
		}
		
		return NULL;
	}
	
	bool AddVariable ( const char* Name, Type* Var );
	
	void ModifyVariable ( int index, Type NewValue );
	
	Type GetVariable ( int index ) { return *(ListOfVariables [ index ]); }
	
	bool Update ();
	
	inline void EnableVariableEdits () { Locked = false; }
	inline void DisableVariableEdits () { Locked = true; }
	
	void ShowDialog_SetValue ();
	void KillDialog_SetValue ();
	
	static void DebugValueList_DblClick ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam )
	{
		int i;

		// make sure this is a double click event
		if ( ((LPNMHDR)lParam)->code == NM_DBLCLK )
		{
			//cout << "\nDouble Click";

			// find the DebugValueList that was clicked on
			for ( i = 0; i < ListOfValueLists.size (); i++ )
			{
				if ( ListOfValueLists [ i ]->hWnd == hCtrl && ListOfValueLists [ i ]->id == idCtrl )
				{
					//cout << "\nList-View Clicked";
					
					ListOfValueLists [ i ]->ShowDialog_SetValue ();
					
					break;
				}
			}
		}
				
	}
	
	
	
	static void Dialog1_OkClick ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam )
	{
		int i;
		Type Value;
		HWND Parent_hWnd;
		
		char* EnteredValue;
		
		//cout << "\nClicked the OK button";
		
		// get the handle for the parent window
		Parent_hWnd = WindowClass::Window::GetHandleToParent ( hCtrl );
		
		//cout << "\nParent Window #1=" << (unsigned long) Parent_hWnd;
		
		for ( i = 0; i < ListOfValueLists.size (); i++ )
		{
			if ( ListOfValueLists [ i ]->Dialog1_hWnd == Parent_hWnd )
			{
				//cout << "\nFound parent window";
				
				// enable parent window
				ListOfValueLists [ i ]->Parent->Enable ();
				
				// get the value that was entered
				EnteredValue = ListOfValueLists [ i ]->editBox1->GetText ();
				
				// convert to a number
				if ( EnteredValue [ 0 ] == '0' && EnteredValue [ 1 ] == 'x' )
				{
					// hexadecimal
					EnteredValue++;
					EnteredValue++;
					if ( from_string ( Value, EnteredValue, std::hex ) )
					{
						// write back the value that was entered
						ListOfValueLists [ i ]->ModifyVariable ( ListOfValueLists [ i ]->SelectedItemIndex, Value );
					}
					else
					{
						MessageBox( hCtrl, "Invalid Value Entered", "Error", NULL );
					}
				}
				else
				{
					// decimal
					if ( from_string ( Value, EnteredValue, std::dec ) )
					{
						// write back the value that was entered
						ListOfValueLists [ i ]->ModifyVariable ( ListOfValueLists [ i ]->SelectedItemIndex, Value );
					}
					else
					{
						MessageBox( hCtrl, "Invalid Value Entered", "Error", NULL );
					}
				}
				
				// kill the dialog window
				ListOfValueLists [ i ]->KillDialog_SetValue ();
				
				break;
			}
			
		}
		
	}
	
	
	
	
	static void Dialog1_CancelClick ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam )
	{
		int i;
		HWND Parent_hWnd;
		
		//cout << "\nClicked the Cancel button";
		
		// get the handle for the parent window
		Parent_hWnd = WindowClass::Window::GetHandleToParent ( hCtrl );
		
		
		for ( i = 0; i < ListOfValueLists.size (); i++ )
		{
			if ( ListOfValueLists [ i ]->Dialog1_hWnd == Parent_hWnd )
			{
				//cout << "\nfound window for cancel";
				
				// kill the dialog window
				ListOfValueLists [ i ]->KillDialog_SetValue ();
				
				break;
			}
		}
	}


	
	static bool from_string (Type& t, 
					 const std::string& s, 
					 std::ios_base& (*f)(std::ios_base&))
	{
	  std::istringstream iss(s);
	  return !(iss >> f >> t).fail();
	}
	
};

template<typename Type>
int DebugValueList<Type>::NextID = DebugValueList<Type>::StartID;

template<typename Type>
vector<DebugValueList<Type>*> DebugValueList<Type>::ListOfValueLists;


template<typename Type>
DebugValueList<Type>::~DebugValueList ()
{
	typename vector<DebugValueList<Type>*>::iterator i;
	DeleteObject ( hf );
	delete lv;
	
	// delete the DebugValueList from the list of value lists
	for ( i = ListOfValueLists.begin(); i != ListOfValueLists.end(); i++ )
	{
		if ( (*i)->id == id && (*i)->hWnd == hWnd )
		{
			// object is already being deleted, so don't delete or it crashes!
			// just erase from list

			// erase element in list
			ListOfValueLists.erase ( i );
			
			// iterator is no longer good after doing an erase
			break;
		}
	}
	
}



template<typename Type>
Type* DebugValueList<Type>::FindVariable ( string NameOfVariableToFind )
{
	int i;
	for ( i = 0; i < ListOfVariables.size (); i++ )
	{
		if ( !NameOfVariableToFind.compare ( lv->GetItemText ( i, 0 ) ) )
		{
			// found the variable -> return the pointer
			return ListOfVariables [ i ];
		}
	}
	
	// never heard of that variable name
	return NULL;
}




template<typename Type>
void DebugValueList<Type>::ModifyVariable ( int index, Type NewValue )
{
	*(ListOfVariables [ index ]) = NewValue;
}

template<typename Type>
bool DebugValueList<Type>::Create( WindowClass::Window* ParentWindow, int x, int y, int width, int height, bool _ShowHex, bool _ShowDec )
{
	static const char* Headings [] = { "Name", "hex", "dec" };
	
	static const char* FontName = "Courier New";
	static const int FontSize = 6;
	
	const static int c_iColWhiteSpace = 12;
	static int ColumnWidths [] = { 55, ( sizeof(Type) * 10 ) + c_iColWhiteSpace, ( sizeof(Type) * 10 ) + c_iColWhiteSpace };
	switch ( sizeof(Type) )
	{
		case 1:
			ColumnWidths [ 2 ] = ( 5 * 3 ) + c_iColWhiteSpace;
			break;
			
		case 2:
			ColumnWidths [ 2 ] = ( 5 * 5 ) + c_iColWhiteSpace;
			break;
			
		case 4:
			ColumnWidths [ 2 ] = ( 5 * 10 ) + c_iColWhiteSpace;
			break;
			
		case 8:
			ColumnWidths [ 2 ] = ( 5 * 10 ) + c_iColWhiteSpace;
			break;
	}
	
	// store parameters
	ShowHex = _ShowHex;
	ShowDec = _ShowDec;
	
	// set the parent window
	Parent = ParentWindow;

	// create the id
	id = NextID++;
	
	// create the list view
	lv = new WindowClass::ListView ();
	
	// create a list view control with a header row
	hWnd = lv->Create_wHeader ( ParentWindow, x, y, width, height, "", id );
	
	//cout << "ListView handle=" << (unsigned long) hWnd;
	
	// add the columns
	lv->InsertColumn ( 0, WindowClass::ListView::CreateColumn ( 0, (int) (ColumnWidths [ 0 ]), (char*) (Headings [ 0 ]) ) );
	if ( ShowHex ) lv->InsertColumn ( 1, WindowClass::ListView::CreateColumn ( 1, (int) (ColumnWidths [ 1 ]), (char*) (Headings [ 1 ]) ) );
	if ( ShowDec ) lv->InsertColumn ( 2, WindowClass::ListView::CreateColumn ( 2, (int) (ColumnWidths [ 2 ]), (char*) (Headings [ 2 ]) ) );
	
	// set the font for the list view control
	// *** todo *** delete the font when done with it
	hf = WindowClass::Window::CreateFontObject ( (int) FontSize, (char*) FontName );
	lv->SetFont ( hf );
	
	// save reference so we can handle events
	ListOfValueLists.push_back ( this );
	
	// add event to modify variables when you double click on any of them
	lv->AddEvent ( WM_NOTIFY, DebugValueList<Type>::DebugValueList_DblClick );
	
	return true;
}


template<typename Type>
bool DebugValueList<Type>::AddVariable ( const char* Name, Type* Var )
{
	// insert row
	lv->InsertRow ( ListOfVariables.size () );

	// set row label
	lv->SetItemText ( ListOfVariables.size (), 0, Name );
	
	// set the pointer to the variable
	ListOfVariables.push_back ( Var );
	
	return true;
}


template<typename Type>
bool DebugValueList<Type>::Update ()
{
	int i;
	stringstream ss;
	
	if ( ListOfVariables.size() == 0 ) return false;
	
	for ( i = 0; i < ListOfVariables.size (); i++ )
	{
		if ( ShowHex )
		{
			ss.str("");
			
			if ( sizeof(Type) == 4 )
			{
				ss << hex << setw ( sizeof(Type) << 1 ) << setfill ( '0' ) << (unsigned long&) *(ListOfVariables [ i ]);
			}
			else
			{
				ss << hex << setw ( sizeof(Type) << 1 ) << setfill ( '0' ) << *(ListOfVariables [ i ]);
			}
			
			lv->SetItemText ( i, 1, (char*) ss.str().c_str() );
		}
		
		if ( ShowDec )
		{
			ss.str("");
			ss << dec << *(ListOfVariables [ i ]);
			lv->SetItemText ( i, 2, (char*) ss.str().c_str() );
		}
	}
	
	//lv->AutoSizeColumn ( 0 );
	//if ( ShowHex ) lv->AutoSizeColumn ( 1 );
	//if ( ShowDec ) lv->AutoSizeColumn ( 2 );
	
	return true;
}


template<typename Type>
void DebugValueList<Type>::KillDialog_SetValue ()
{
	// enable parent window
	Parent->Enable ();
	
	// now Destroy the window and delete any associated objects
	delete wDialog;
	delete stLabel1;
	delete editBox1;
	delete CmdButton1;
	
	// clear handle for dialog since we destroyed it
	Dialog1_hWnd = NULL;
	
	// update list view
	Update ();
}


template<typename Type>
void DebugValueList<Type>::ShowDialog_SetValue ()
{
	
	static const char* Dialog1_Caption = "Modify";
	static const int Dialog1_Id = 5000;
	static const int Dialog1_X = 10;
	static const int Dialog1_Y = 10;
	static const int Dialog1_Width = 200;
	static const int Dialog1_Height = 150;

	static const char* Label1_Text = "Enter new value:";
	static const int Label1_Id = 5001;
	static const int Label1_X = 10;
	static const int Label1_Y = 10;
	static const int Label1_Width = 100;
	static const int Label1_Height = 20;

	static const int Edit1_Id = 5002;
	static const int Edit1_X = 10;
	static const int Edit1_Y = 40;
	static const int Edit1_Width = 100;
	static const int Edit1_Height = 20;
	
	static const char* CmdButton1_Caption = "OK";
	static const int CmdButton1_Id = 5003;
	static const int CmdButton1_X = 10;
	static const int CmdButton1_Y = 90;
	static const int CmdButton1_Width = 50;
	static const int CmdButton1_Height = 20;
	
	static const char* CmdButton2_Caption = "Cancel";
	static const int CmdButton2_Id = 5004;
	static const int CmdButton2_X = 70;
	static const int CmdButton2_Y = 90;
	static const int CmdButton2_Width = 50;
	static const int CmdButton2_Height = 20;
	
	stringstream ss;
	
	SelectedItemIndex = lv->GetRowOfSelectedItem ();

	if ( SelectedItemIndex >= 0 && !Locked )
	{
		Lock_Exchange32 ( (long&) ListWasDoubleClicked, 1 );
		
		// this is the value list that was double clicked on
		// now show a window where the variable can be modified
		// *note* setting the parent to the list-view control
		//cout << "\nAllocating dialog";
		wDialog = new WindowClass::Window ();
		//cout << "\nCreating dialog";
		Dialog1_hWnd = wDialog->Create ( (char*) Dialog1_Caption, Dialog1_X, Dialog1_Y, Dialog1_Width, Dialog1_Height, WindowClass::Window::DefaultStyle, NULL, hWnd );
		wDialog->DisableCloseButton ();
		
		//cout << "\nListView Handle=" << (unsigned long)hWnd << " Dialog handle=" << (unsigned long) Dialog1_hWnd;
		
		//cout << "\nCreating static control";
		
		// put in a static label for entering a new value
		stLabel1 = new WindowClass::Static ();
		stLabel1->Create_Text ( wDialog, Label1_X, Label1_Y, Label1_Width, Label1_Height, (char*) Label1_Text, Label1_Id );
		
		//cout << "\nCreating edit control. SelectedItemIndex=" << SelectedItemIndex << " Text=" << lv->GetItemText ( SelectedItemIndex, 1 );
		
		// put in an edit box to edit the value
		ss << "0x" << hex << setw( sizeof(Type) << 1 ) << setfill ( '0' ) << GetVariable ( SelectedItemIndex );
		editBox1 = new WindowClass::Edit ();
		editBox1->Create ( wDialog, Edit1_X, Edit1_Y, Edit1_Width, Edit1_Height, (char*) ss.str().c_str(), Edit1_Id );
		
		// set the edit box to the value of the item initially
		//editBox1->SetText ( lv->GetItemText ( SelectedItemIndex, 1 ) );
		
		//cout << "\nCreating ok button";
		
		// put in an ok button
		CmdButton1 = new WindowClass::Button ();
		CmdButton1->Create_CmdButton( wDialog, CmdButton1_X, CmdButton1_Y, CmdButton1_Width, CmdButton1_Height, (char*) CmdButton1_Caption, CmdButton1_Id );
		
		// ***todo*** add event for ok button
		CmdButton1->AddEvent ( WM_COMMAND, Dialog1_OkClick );
		
		// put in an ok button
		CmdButton2 = new WindowClass::Button ();
		CmdButton2->Create_CmdButton( wDialog, CmdButton2_X, CmdButton2_Y, CmdButton2_Width, CmdButton2_Height, (char*) CmdButton2_Caption, CmdButton2_Id );
		
		// ***todo*** add event for cancel button
		CmdButton2->AddEvent ( WM_COMMAND, Dialog1_CancelClick );
		
		//cout << "\nDisabling parent window";
		
		// disable the parent window of the value list
		Parent->Disable ();
		
		//cout << "\ndone";
	}
	
}





#endif




