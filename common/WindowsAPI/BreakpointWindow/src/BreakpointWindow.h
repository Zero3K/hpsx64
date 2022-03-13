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


#ifndef _BREAKPOINTWINDOW_H_
#define _BREAKPOINTWINDOW_H_


#include "WinApiHandler.h"

#include <string>

#include "StringUtils.h"

#include "breakpoint.h"

using namespace x64ThreadSafe::Utilities;



class Debug_BreakpointWindow
{
	static const int StartID = 25000;
	static int NextID;
	
	static const int ID_MODIFY = 10001;
	static const int ID_REMOVE = 10002;
	static const int ID_ADD = 10003;

	// this is the size of the variables in the value list in bytes. Will be used to allow variable to be specified in conditional breakpoints
	int ValueSizeInBytes;
	
	WindowClass::Window *Parent;
	
	int id;
	HWND hWnd;
	
	volatile unsigned long ListWasDoubleClicked;
	
	bool Locked;
	
	class BreakPoint_Dialog
	{
	public:
		static volatile u32 isShowing_Global;
		volatile u32 Active;
		
		bool bExecute, bMemory, bRead, bWrite;
		string sAddress, sSize, sCondition;
		
		BreakPoint_Dialog ();
		
		WindowClass::Window* Dialog;
		
		WindowClass::Button* RadioBtn_Execute;
		WindowClass::Button* RadioBtn_Memory;
		
		WindowClass::Button* CheckBox_Read;
		WindowClass::Button* CheckBox_Write;
		
		WindowClass::Static* Label_BreakPointType;
		WindowClass::Static* Label_Address;
		WindowClass::Static* Label_Size;
		WindowClass::Static* Label_Condition;
		
		WindowClass::Edit* Edit_Address;
		WindowClass::Edit* Edit_Size;
		WindowClass::Edit* Edit_Condition;
		
		WindowClass::Button* CmdBtn_Add;
		WindowClass::Button* CmdBtn_Cancel;
		
		void Show ();
		void Kill ();
		
		void Add_Breakpoint ( Debug_BreakPoints* dbp );
		
		static void Add_Click ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam );
		static void Cancel_Click ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam );
		
		static void DefaultCheckBoxHandler ( HWND hCtrl, int id, unsigned int message, WPARAM wParam, LPARAM lParam );
	};
	
	BreakPoint_Dialog bpd;
	
	HFONT hf;
	
	WindowClass::ListView *lv;
	
	//vector<Type*> ListOfVariables;
	
	static vector<Debug_BreakpointWindow*> ListOf_BreakpointWindows;

	/////// Dialog Stuff /////////////
	int SelectedItemIndex;
	HWND Dialog1_hWnd;
	WindowClass::Window *wDialog;
	WindowClass::Static *stLabel1;
	WindowClass::Edit *editBox1;
	WindowClass::Button *CmdButton1;
	WindowClass::Button *CmdButton2;
	
	Debug_BreakPoints* dbp;
	
public:

	HMENU hContextMenu;

	u32 isDialogShowing;
	static u32 isDialogShowing_Global;

	int NumberOfColumns;

	Debug_BreakpointWindow ( Debug_BreakPoints* d ) { dbp = d; WindowClass::ListView::InitCommonControls (); }

	~Debug_BreakpointWindow ();

	bool Create ( WindowClass::Window* ParentWindow, int x, int y, int width, int height );

	// update list of breakpoints
	bool Update ();
	
	// return count of breakpoints
	inline int Count () { return dbp->Count (); }
	
	void ShowContextMenu ();
	
	void ShowDialog_Add ();
	void ShowDialog_Modify ();
	//void ShowDialog_Remove ();
	void KillDialog_Add ();
	void KillDialog_Modify ();
	//void KillDialog_Remove ();
	
	
	static void BreakpointWindow_Event ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam );
	//static void BreakpointWindow_RightClick ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam );
	
	static void Dialog1_OkClick ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam );
	static void Dialog1_CancelClick ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam );
	
	
};


#endif




