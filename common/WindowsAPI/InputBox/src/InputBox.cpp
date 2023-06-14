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


#include "InputBox.h"


volatile u32 InputBox::isDialogShowing = false;
vector<InputBox*> InputBox::ListOfInputBoxes;



InputBox::InputBox ()
{
	// add input box into list
	ListOfInputBoxes.push_back ( this );
}

InputBox::~InputBox ()
{
	typename vector<InputBox*>::iterator i;
	//DeleteObject ( hf );
	
	// delete the DebugValueList from the list of value lists
	for ( i = ListOfInputBoxes.begin(); i != ListOfInputBoxes.end(); i++ )
	{
		if ( (*i)->Dialog1_hWnd == Dialog1_hWnd )
		{
			// object is already being deleted, so don't delete or it crashes!
			// just erase from list

			// erase element in list
			ListOfInputBoxes.erase ( i );
			
			// iterator is no longer good after doing an erase
			break;
		}
	}
}	



void InputBox::WaitForAllInputBoxes ()
{
	// make sure we are not on the GUI Thread
	if ( WindowClass::Window::GUIThread->ThreadId == GetCurrentThreadId () ) return;
	
	while ( isDialogShowing );
}

void InputBox::KillDialog ()
{
	// enable parent window
	//Parent->Enable ();
	
	// now Destroy the window and delete any associated objects
	delete wDialog;
	delete stLabel1;
	delete editBox1;
	delete CmdButton1;
	delete CmdButton2;
	
	// clear handle for dialog since we destroyed it
	Dialog1_hWnd = NULL;
	
	x64ThreadSafe::Utilities::Lock_Exchange32 ( (long&) isDialogShowing, false );
	
	// update list view
	//Update ();
}



void InputBox::ShowDialog ( WindowClass::Window* ParentWindow, string WindowCaption, string Prompt, InputBoxCallback _OnClick_Ok, InputBoxCallback _OnClick_Cancel, const char* InitialValue )
{
	
	const char* Dialog1_Caption = "Modify";
	static constexpr int Dialog1_Id = 5000;
	static constexpr int Dialog1_X = 10;
	static constexpr int Dialog1_Y = 10;
	static constexpr int Dialog1_Width = 200;
	static constexpr int Dialog1_Height = 150;

	const char* Label1_Text = "Enter new value:";
	static constexpr int Label1_Id = 5001;
	static constexpr int Label1_X = 10;
	static constexpr int Label1_Y = 10;
	static constexpr int Label1_Width = 100;
	static constexpr int Label1_Height = 20;

	static constexpr int Edit1_Id = 5002;
	static constexpr int Edit1_X = 10;
	static constexpr int Edit1_Y = 40;
	static constexpr int Edit1_Width = 100;
	static constexpr int Edit1_Height = 20;
	
	const char* CmdButton1_Caption = "OK";
	static constexpr int CmdButton1_Id = 5003;
	static constexpr int CmdButton1_X = 10;
	static constexpr int CmdButton1_Y = 90;
	static constexpr int CmdButton1_Width = 50;
	static constexpr int CmdButton1_Height = 20;
	
	const char* CmdButton2_Caption = "Cancel";
	static constexpr int CmdButton2_Id = 5004;
	static constexpr int CmdButton2_X = 70;
	static constexpr int CmdButton2_Y = 90;
	static constexpr int CmdButton2_Width = 50;
	static constexpr int CmdButton2_Height = 20;
	
	stringstream ss;
	
	if ( !isDialogShowing )
	{
		x64ThreadSafe::Utilities::Lock_Exchange32 ( (long&) isDialogShowing, true );

		// set the events to use on call back
		OnClick_Ok = _OnClick_Ok;
		OnClick_Cancel = _OnClick_Cancel;
		
		// this is the value list that was double clicked on
		// now show a window where the variable can be modified
		// *note* setting the parent to the list-view control
		//cout << "\nAllocating dialog";
		wDialog = new WindowClass::Window ();
		//cout << "\nCreating dialog";
		Dialog1_hWnd = wDialog->Create ( (char*) WindowCaption.c_str(), Dialog1_X, Dialog1_Y, Dialog1_Width, Dialog1_Height, WindowClass::Window::DefaultStyle, NULL, ParentWindow->hWnd );
		wDialog->DisableCloseButton ();
		
		//cout << "\nListView Handle=" << (unsigned long)hWnd << " Dialog handle=" << (unsigned long) Dialog1_hWnd;
		
		//cout << "\nCreating static control";
		
		// put in a static label for entering a new value
		stLabel1 = new WindowClass::Static ();
		stLabel1->Create_Text ( wDialog, Label1_X, Label1_Y, Label1_Width, Label1_Height, (char*) Prompt.c_str(), Label1_Id );
		
		//cout << "\nCreating edit control. SelectedItemIndex=" << SelectedItemIndex << " Text=" << lv->GetItemText ( SelectedItemIndex, 1 );
		
		// put in an edit box to edit the value
		//ss << "0x" << hex << setw( sizeof(Type) << 1 ) << setfill ( '0' ) << GetVariable ( SelectedItemIndex );
		editBox1 = new WindowClass::Edit ();
		editBox1->Create ( wDialog, Edit1_X, Edit1_Y, Edit1_Width, Edit1_Height, (char*) InitialValue );
		
		// set the edit box to the value of the item initially
		//editBox1->SetText ( lv->GetItemText ( SelectedItemIndex, 1 ) );
		
		//cout << "\nCreating ok button";
		
		// put in an ok button
		CmdButton1 = new WindowClass::Button ();
		CmdButton1->Create_CmdButton( wDialog, CmdButton1_X, CmdButton1_Y, CmdButton1_Width, CmdButton1_Height, (char*) CmdButton1_Caption, CmdButton1_Id );
		
		// add event for ok button
		CmdButton1->AddEvent ( WM_COMMAND, Dialog1_OkClick );
		
		// put in an ok button
		CmdButton2 = new WindowClass::Button ();
		CmdButton2->Create_CmdButton( wDialog, CmdButton2_X, CmdButton2_Y, CmdButton2_Width, CmdButton2_Height, (char*) CmdButton2_Caption, CmdButton2_Id );
		
		// add event for cancel button
		CmdButton2->AddEvent ( WM_COMMAND, Dialog1_CancelClick );
		
		
		//cout << "\nDisabling parent window";
		
		// disable the parent window of the value list
		//Parent->Disable ();
		
		//cout << "\ndone";
	}
	
}


int InputBox::FindInputBoxIndex ( HWND hWndInputBox )
{
	int i;
	
	for ( i = 0; i < ListOfInputBoxes.size (); i++ )
	{
		if ( ListOfInputBoxes [ i ]->Dialog1_hWnd == hWndInputBox )
		{
			return i;
		}
	}
	
	return -1;
}

void InputBox::Dialog1_OkClick ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam )
{
	int i;
	HWND Parent_hWnd;
	
	//cout << "\nClicked the OK button";
	
	// get the handle for the parent window
	Parent_hWnd = WindowClass::Window::GetHandleToParent ( hCtrl );
	
	//cout << "\nParent Window #1=" << (unsigned long) Parent_hWnd;
	
	i = FindInputBoxIndex ( Parent_hWnd );
	
	if ( i < 0 ) return;
	
	ListOfInputBoxes [ i ]->ReturnValue = ListOfInputBoxes [ i ]->editBox1->GetText ();
	if ( ListOfInputBoxes [ i ]->OnClick_Ok ) ListOfInputBoxes [ i ]->OnClick_Ok ( ListOfInputBoxes [ i ]->editBox1->GetText () );
	ListOfInputBoxes [ i ]->KillDialog ();
}

void InputBox::Dialog1_CancelClick ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam )
{
	int i;
	HWND Parent_hWnd;
	
	//cout << "\nClicked the Cancel button";
	
	// get the handle for the parent window
	Parent_hWnd = WindowClass::Window::GetHandleToParent ( hCtrl );
	
	i = FindInputBoxIndex ( Parent_hWnd );
	
	if ( i < 0 ) return;
	
	ListOfInputBoxes [ i ]->ReturnValue = "";
	if ( ListOfInputBoxes [ i ]->OnClick_Cancel ) ListOfInputBoxes [ i ]->OnClick_Cancel ( "" );
	ListOfInputBoxes [ i ]->KillDialog ();
}

