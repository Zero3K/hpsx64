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


#ifndef _INPUTBOX_H_
#define _INPUTBOX_H_

#include "WinApiHandler.h"

#include <string>

class InputBox
{
	/////// Dialog Stuff /////////////
	HWND Dialog1_hWnd;
	WindowClass::Window *wDialog;
	WindowClass::Static *stLabel1;
	WindowClass::Edit *editBox1;
	WindowClass::Button *CmdButton1;
	WindowClass::Button *CmdButton2;

	
	
	static vector<InputBox*> ListOfInputBoxes;
	

public:

	static volatile u32 isDialogShowing;
	
	// constructor
	InputBox ();
	
	// destructor
	~InputBox ();
	
	static int FindInputBoxIndex ( HWND hWndInputBox );
	
	// this is where string that was entered will be returned
	string ReturnValue;
	
	// *note* don't call this from the GUI thread
	static void WaitForAllInputBoxes ();
	
	typedef void (*InputBoxCallback) ( string ReturnValue );
	
	InputBoxCallback OnClick_Ok;
	InputBoxCallback OnClick_Cancel;
	
	void ShowDialog ( WindowClass::Window* ParentWindow, string WindowCaption, string Prompt, InputBoxCallback _OnClick_Ok, InputBoxCallback _OnClick_Cancel, const char* InitialValue = "" );
	void KillDialog ();
	
	static void Dialog1_OkClick ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam );
	static void Dialog1_CancelClick ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam );
};


#endif



