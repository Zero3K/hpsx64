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



#ifndef WINAPIHANDLER_H_
#define WINAPIHANDLER_H_


//#include <GL/glew.h>


#include "types.h"
#include "MultiThread.h"

#include "StringUtils.h"

#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>

#include <gl/gl.h>
// Include GLEW
//#include <GL/glew.h>


#include <string>
#include <vector>

#include <string.h>

#include "Debug.h"


#include "json.hpp"

using json = nlohmann::ordered_json;





namespace WindowClass
{
using namespace std;

	extern Debug::Log debug;

	extern WNDCLASS wc;
	extern HINSTANCE hInst;
	extern LPCTSTR className;
	extern MSG msg;

	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void Register ( HINSTANCE hInstance, LPCTSTR lpszClassName = "Default Class", LPCTSTR lpszMenuName = NULL, WNDPROC lpfnWndProc = WindowClass::WndProc, UINT style = CS_OWNDC, HBRUSH hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH ), HICON hIcon = LoadIcon( NULL, IDI_APPLICATION ), HCURSOR hCursor = LoadCursor( NULL, IDC_ARROW ), int cbClsExtra = 0, int cbWndExtra = 0 );

	// call this at least once a frame to keep everything running as it should
	void DoEvents ();
	void DoEventsNoWait ();
	void DoSingleEvent ();

	// this is the id we should start numbering controls at when the ID to use is not specified
	static const u32 StartId = 9000;
	
	extern volatile u32 NextIndex;

	
	// gets the identifier of a control from its handle
	inline int GetCtrlIdFromHandle ( HWND hCtrl ) { return GetDlgCtrlID( hCtrl ); }


	class MenuBar
	{
	public:
		typedef void (*Function) ( int MenuItemId );
	
		class MenuBarItem
		{
		public:
		
			u32 MenuBarId;
			u32 ParentId;
		
			unsigned long long Id;
			string Caption;
			Function CallbackFunc;
			
			HMENU Menu;
			
			// _Caption - what the text of the menu item should show
			// sKey - what unique text should be used to refer back to the menu item later (not always same as caption)
			// _CallbackFunc - function to call when menu item is clicked, or else NULL
			MenuBarItem* AddItem ( string _Caption, string sKey, Function _CallbackFunc = NULL, u32 _Id = NULL );
			
			// a menu it different from a menu bar item because it presents another menu with more options to click on
			MenuBarItem* AddMenu ( string _Caption, u32 _Id = NULL );
			
			inline int CheckItem () { return CheckMenuItem( WindowClass::MenuBar::FindMenuBarById ( MenuBarId )->MainMenu, Id, MF_BYCOMMAND | MF_CHECKED ); }
			inline int UnCheckItem () { return CheckMenuItem( WindowClass::MenuBar::FindMenuBarById ( MenuBarId )->MainMenu, Id, MF_BYCOMMAND | MF_UNCHECKED ); }

			
			//MenuBarItem* FindByIndex ( u32 Index );
			//MenuBarItem* FindByName ( string _Name );
			
			// constructor
			MenuBarItem ( u32 _MenuBarId, u32 _ParentId, string _Caption, u32 _Id = NULL, Function _CallbackFunc = NULL );
			
			// destructor
			~MenuBarItem ();
		};
		
		u32 Id;
		
		HMENU MainMenu;
		
		// this is the handle for the parent window
		HWND hWnd;
		
		static vector<MenuBar*> ListOfMenuBars;
		static vector<MenuBarItem*> ListOfMenuBarItems;
		//vector<MenuBarItem*> Menus;
		
		// constructor
		MenuBar ( HWND _hWnd );
		
		// destructor
		~MenuBar ();
		

		
		// adds an main menu item into menu bar
		MenuBarItem* AddMainMenuItem ( string _Caption, u32 _Id = NULL );
		
		// adds a sub item into a menu
		MenuBarItem* AddItem ( u32 IdToAddTo, string _Caption, Function CallBackFuncWhenClicked = NULL, u32 IdOfNewItem = NULL );
		MenuBarItem* AddItem ( string CaptionToAddTo, string _Caption, Function CallBackFuncWhenClicked = NULL, u32 IdOfNewItem = NULL );
		
		MenuBarItem* AddMenu ( u32 IdToAddTo, string _Caption, u32 IdOfNewItem = NULL );
		MenuBarItem* AddMenu ( string CaptionToAddTo, string _Caption, u32 IdOfNewItem = NULL );

		// return value for Check/UnCheck functions specifies previous state of check mark for menu item (MF_CHECKED/MF_UNCHECKED)
		int CheckItem ( u32 _Id );
		int UnCheckItem ( u32 _Id );
		int CheckItem ( string _Caption );
		int UnCheckItem ( string _Caption );
		
		MenuBarItem* FindItemById ( u32 _Id );
		MenuBarItem* FindItemByCaption ( string _Caption );
		
		static MenuBar* GetMenuBarForWindow ( HWND _hWnd );
		static MenuBar* FindMenuBarById ( u32 _Id );
		
		// shows the menu bar
		void Show ();
	};


	class Window
	{
	
	public:
	
		static const unsigned long DefaultStyle = WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE;
	
		// *note* event functions must provide handle for control
		typedef void (*EventFunction) ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam );
		
		// the result from the last operation (remote call)
		// wait for busy to be 0, then read LastResult
		static volatile unsigned long Busy;
		static volatile unsigned long long LastResult;
		
		typedef unsigned long long (*RemoteFunction) ( void* Params );
		
		struct RemoteCallData
		{
			unsigned long long Param;
			RemoteFunction FunctionToCall;
		};
		
		////////////////////////////////////////////////////////////////////
		// this will be used for the circular buffer for remote call data
		
		// buffer size must be a power of 2
		static const unsigned long c_RemoteCallBufferSize = 2048;
		static const unsigned long c_RemoteCallBufferMask = c_RemoteCallBufferSize - 1;
		static volatile unsigned long ReadIndex;
		static volatile unsigned long WriteIndex;
		static volatile RemoteCallData RemoteCall_Buffer [ c_RemoteCallBufferSize ];
		
		struct Event
		{
			HWND hwndParent;
			HWND hwndCtrl;
			long long id;
			unsigned int message;
			EventFunction ef;
			
			Event ( HWND _hwndParent, HWND _hwndCtrl, int _id, unsigned int _message, EventFunction _ef )
			{ hwndParent = _hwndParent; hwndCtrl = _hwndCtrl; id = _id; message = _message; ef = _ef;  }
		};
		
		// *** TODO *** Don't forget to delete the events associated with windows when destroying the window
		static vector<Event*> EventList;

		void CreateMenuFromJson( json jsnMenu, const char* sLanguage );

		static void OutputAllDisplayModes ();

		// this will add an event for a window/control
		// specify NULL if an argument doesn't matter
		static bool AddEvent ( HWND hParentWindow, HWND hControl, int CtrlId, unsigned int message, EventFunction Func );
		
		// this will remove all events for an identifier
		// returns true on success and false otherwise
		static bool RemoveEvent ( HWND hParentWindow, long long id, unsigned int message );
	
		
		HWND CreateControl ( const char* ClassName, int x, int y, int width, int height, const char* Caption, int flags, HMENU hMenu = NULL );
	
		HWND Create ( const char* _lpWindowName, int _x, int _y, int _nWidth, int _nHeight, DWORD _dwStyle = DefaultStyle, HMENU _hMenu = NULL, HWND _hWndParent = NULL, LPVOID _lpParam = NULL, HINSTANCE _hInstance = NULL, LPCTSTR _lpClassName = className );
		
		
		void KillGLWindow();
		BOOL CreateGLWindow(const char* title, int width, int height, bool has_menu, bool fullscreenflag);
		void ToggleGLFullScreen ();
		
		bool Redraw ();
		
		static HFONT CreateFontObject ( int fontSize, const char* fontName = "Times New Roman", bool Bold = false, bool Underline = false, bool Italic = false, bool StrikeOut = false );
		

		// when done with the window this will destroy it
		bool Destroy ();
		
		bool Enable () { return EnableWindow( hWnd, true ); }
		bool Disable () { return EnableWindow( hWnd, false ); }
		
		bool DisableCloseButton () { return EnableMenuItem( GetSystemMenu( hWnd, false ), SC_CLOSE, MF_BYCOMMAND | MF_GRAYED ); }

		bool SetWindowSize( long width, long height );
		
		HWND GetHandleToParent () { return GetParent( hWnd ); }
		static HWND GetHandleToParent ( HWND hwnd ) { return GetParent( hwnd ); }
		
		static volatile u32 LastKeyPressed;
		
		HWND hWnd;
		HDC hDC;
		HGLRC hRC;
		HFONT hFont;
		HFONT hFontOld;
		HINSTANCE hInstance;
		
		bool OpenGL_Enabled;
		bool fullscreen;
		int WindowWidth, WindowHeight;
		
		// this is important, because the GUI thread can't get any messages if it is in a loop
		static volatile u32 InModalMenuLoop;
		
		// lets also toss in a Menu Bar
		MenuBar* Menus;

		// constructor should clear this as it does not create the window
		volatile long isWindowCreated;

		static int NextShortcutKeyID;
		
		typedef void (*Function) ( int ID );
		
		struct ShortcutKey_Entry
		{
			HWND hWnd;
			int ID;
			unsigned int Key;
			unsigned int Modifier;
			Function CallbackFunc;
		};
		
		static vector<ShortcutKey_Entry> ShortcutKey_Entries;

		// window parameters
		/*
		char* lpWindowName;
		int x;
		int y;
		int nWidth;
		int nHeight;
		DWORD dwStyle;
		HMENU hMenu;
		HWND hWndParent;
		LPVOID lpParam;
		HINSTANCE hInstance;
		LPCTSTR lpClassName;
		*/


		// enumeration of functions that can be called by posting messages to GUI thread or target window/GUI thread
		enum { GUI_CREATE = WM_APP, GUI_SETTEXT, GUI_CHANGETEXTCOLOR, GUI_CHANGETEXTBKCOLOR, GUI_CHANGEFONT, GUI_CLEAR, GUI_CHANGEBKMODE,
				GUI_PRINTTEXT, GUI_ADDSHORTCUTKEY, GUI_DRAWPIXEL, GUI_BEGINDRAWING, GUI_ENDDRAWING, GUI_REMOTECALL };

		static long GUIThread_isRunning;
		static Api::Thread* GUIThread;

		//static int CreateMessageHandlerThread ();

		// the gui should be on its own thread so it doesn't get in the way
		// it will be idle most of the time anyways
		static void StartGUIThread ();
		
		// this will wait for any modal menu loops to finish before proceeding
		static inline void WaitForModalMenuLoop () { while ( InModalMenuLoop ); }
		
		

		// contructor
		Window ();

		// destructor
		~Window ();


		// *** Dialog Boxes *** //
		string ShowFileOpenDialog ();
		vector<string> ShowFileOpenDialogMultiSelect ();
		string ShowFileSaveDialog ();



		// this enables opengl for the window
		bool EnableOpenGL ( void );

		// this disables opengl for the window
		void DisableOpenGL ( void );

		// *** Functions for use with OpenGL *** //

		// use this to enable vsync on an OpenGL window
		bool EnableVSync ( void );

		// use this to disable vsync on an OpenGL window
		bool DisableVSync ( void );

		// use this with double buffering to flip the screen
		void FlipScreen ( void );
		
		
		// use this to make the current open gl drawing window
		inline void OpenGL_MakeCurrentWindow () { wglMakeCurrent( hDC, hRC ); }
		inline static void OpenGL_ReleaseWindow () { wglMakeCurrent(NULL, NULL); }
		
		// *** Window Functions *** //
		
		// modifier is CTRL key as default
		// modifier: 1 - alt, 2 - control, 4 - shift
		void AddShortcutKey ( Function CallbackFunc, unsigned int key, unsigned int modifier = 0 );

		
		/////////// Show_ContextMenu ///////////////////////
		// shows a context menu
		// returns -1 on no selection, otherwise returns zero-based index of selection
		volatile int Show_ContextMenu ( int x, int y, string ContextMenu_Options );

		// *** GDI Functions *** //

/*
		// call this before you start any drawing that is not opengl related
		bool BeginDrawing ( void );

		// call this when you are done drawing so the window will update
		bool EndDrawing ( void );
*/

		// *** Functions for use with BeginDrawing/EndDrawing functions *** //

		// clears the window
		bool Clear ( void );

		// sets the text color
		bool ChangeTextColor ( u32 color );

		// sets the text background color
		bool ChangeTextBkColor ( u32 color );

		// sets the BkMode - can be either TRANSPARENT or OPAQUE - determines what happens when you draw text or use brush/pen
		bool ChangeBkMode ( int iBkMode );

		// sets the font and font size - *** deprecated ***
		//bool ChangeFont ( u32 fontSize, char* fontName = "Times New Roman" );

		
		void Set_Font ( HFONT _hFont );
		
		int Get_TextWidth ( string text, UINT uFormat = DT_SINGLELINE | DT_EXTERNALLEADING | DT_LEFT | DT_TOP | DT_CALCRECT );
		int Get_TextHeight ( string text, UINT uFormat = DT_SINGLELINE | DT_EXTERNALLEADING | DT_LEFT | DT_TOP | DT_CALCRECT );

		bool SetText ( long x, long y, const char* text, u32 fontSize = 8, UINT uFormat = DT_SINGLELINE | DT_NOCLIP );

		// Draws text on a particular line number of the window
		bool PrintText ( long x, long LineNumber, const char* text, u32 fontSize = 8, UINT uFormat = DT_SINGLELINE | DT_NOCLIP );
		
		void DrawPixel ( int x, int y, COLORREF ColorRGB24 );
		
		// gets the size of a window including non-viewable area
		// returns true on success, false on error
		bool GetWindowSize ( long* width, long* height );

		// gets the viewable width and height for window
		bool GetViewableArea ( long* width, long* height );
		
		
		inline bool SetCaption ( const char* sCaption ) { return SetWindowText ( hWnd, (LPCSTR) sCaption ); }
		
		// gets the size of the window that is needed so that the viewable area is as specified
		static bool GetRequiredWindowSize ( long* width, long* height, BOOL hasMenu = TRUE, long WindowStyle = DefaultStyle );
		
		//////////// RemoteCall ////////////////////
		// calls a function from the GUI thread (some things in the Windows API have to be done from the thread the window was created in)
		static unsigned long long RemoteCall ( RemoteFunction FunctionToCall, void* Parameters, bool WaitForReturnValue = true );
		

	private:

		

		
		///////////// Create Font ///////////////////
		// creates a font
		struct _CreateFontObject_Params { int fontSize; const char* fontName; bool Bold; bool Underline; bool Italic; bool StrikeOut;
		_CreateFontObject_Params ( int _fontSize, const char* _fontName, bool _Bold, bool _Underline, bool _Italic, bool _StrikeOut )
		{ fontSize = _fontSize; fontName = _fontName; Bold = _Bold; Underline = _Underline; Italic = _Italic; StrikeOut = _StrikeOut; } };
		
		static HFONT _CreateFontObject ( _CreateFontObject_Params* p );

		//////////// Create Control ////////////////////
		// creates a control on the window
		struct _CreateControl_Params { HWND ParentWindow; const char* ClassName; int x; int y; int width; int height; const char* Caption; int flags; HMENU hMenu;
		_CreateControl_Params ( HWND _ParentWindow, const char* _ClassName, int _x, int _y, int _width, int _height, const char* _Caption, int _flags, HMENU _hMenu )
		{ ParentWindow = _ParentWindow; ClassName = _ClassName; x = _x; y = _y; width = _width; height = _height; Caption = _Caption; flags = _flags; hMenu = _hMenu; } };
		
		// returns a handle to the control
		static unsigned long long _CreateControl ( _CreateControl_Params* Params );
	
		//////////// Create ///////////////////////
		// creates a window
		struct _Create_Params { const char* lpWindowName; int x; int y; int nWidth; int nHeight; DWORD dwStyle; HMENU hMenu; HWND hWndParent; LPVOID lpParam;
								HINSTANCE hInstance; LPCTSTR lpClassName; WindowClass::Window* w;	// this is needed to set hWnd
		_Create_Params ( WindowClass::Window* _w, const char* _lpWindowName, int _x, int _y, int _nWidth, int _nHeight, DWORD _dwStyle, HMENU _hMenu, HWND _hWndParent, LPVOID _lpParam, HINSTANCE _hInstance, LPCTSTR _lpClassName )
		{ w = _w; lpWindowName = _lpWindowName; x = _x; y = _y; nWidth = _nWidth; nHeight = _nHeight; dwStyle = _dwStyle; hMenu = _hMenu;
		hWndParent = _hWndParent; lpParam = _lpParam; hInstance = _hInstance; lpClassName = _lpClassName; } };

		// this function will actually create the window, and do it on the GUI thread
		// returns a handle to the window
		static unsigned long long _Create ( _Create_Params* Params ); //WindowClass::Window* _w, char* _lpWindowName, int _x, int _y, int _nWidth, int _nHeight, DWORD _dwStyle, HMENU _hMenu, HWND _hWndParent, LPVOID _lpParam, HINSTANCE _hInstance, LPCTSTR _lpClassName );
		
		
		/////////////// Redraw /////////////////////////
		// redraws a window
		struct _Redraw_Params { HWND hWnd; unsigned int flags;
		_Redraw_Params ( HWND _hWnd, unsigned int _flags ) { hWnd = _hWnd; flags = _flags; } };
		
		static unsigned long long _Redraw ( _Redraw_Params* p );
		
		
		//////////// DestroyWindow ///////////////
		struct _Destroy_Params { HWND hWnd;
		_Destroy_Params ( HWND _hWnd ) { hWnd = _hWnd; } };
		
		static unsigned long long _Destroy ( _Destroy_Params* p );

		
		//////////// Show_ContextMenu /////////////////
		struct _Show_ContextMenu_Params { HWND hWnd; int x; int y; string st;
		_Show_ContextMenu_Params ( HWND _hWnd, int _x, int _y, string _st ) { hWnd = _hWnd; x = _x; y = _y; st = _st; } };
		static unsigned long long _Show_ContextMenu ( _Show_ContextMenu_Params* p );
		
		
		/////////////// Get_TextWidth /////////////////////
		struct _Get_TextWidth_Params { HWND hWnd; string text; int format;
		_Get_TextWidth_Params ( HWND _hWnd, string _text, int _format ) { hWnd = _hWnd; text = _text; format = _format; } };
		static unsigned long long _Get_TextWidth ( _Get_TextWidth_Params* p );
		static unsigned long long _Get_TextHeight ( _Get_TextWidth_Params* p );


		///////////////// Set_Font /////////////////////////
		struct _Set_Font_Params { HWND hWnd; HFONT hFont;
		_Set_Font_Params ( HWND _hWnd, HFONT _hFont ) { hWnd = _hWnd; hFont = _hFont; } };
		static unsigned long long _Set_Font ( _Set_Font_Params* p );
	
		// running this on a separate thread keeps the window for going into "Not Responding" mode
		static int StartWindowMessageLoop ( void* Param );
		static int WindowMessageLoop ();


		union _wParams
		{
			u64 Value;

			struct
			{
				s32 x : 12;
				s32 y : 12;
				u32 fontSize : 8;
				u32 uFormat;
			};

			_wParams ( s32 _x, s32 _y, u32 _fontSize, u32 _uFormat ) { x = _x; y = _y; uFormat = _uFormat; fontSize = _fontSize; }
			_wParams () {}
		};
		
		static void _AddShortcutKey ( ShortcutKey_Entry *s );

		static bool _SetText ( HWND _hWnd, long _x, long _y, const char* _text, u32 _fontSize, UINT _uFormat );
		static bool _ChangeTextColor ( HWND _hWnd, u32 color );
		static bool _ChangeTextBkColor ( HWND _hWnd, u32 color );
		//static bool _ChangeFont ( HWND _hWnd, u32 fontSize, char* fontName );
		static bool _Clear ( HWND _hWnd );
		static bool _ChangeBkMode ( HWND _hWnd, int iBkMode );
		static bool _PrintText ( HWND _hWnd, long x, long LineNumber, const char* text, u32 fontSize, UINT uFormat );

		// these can only be called from GUI thread with HDC
		static long _GetTextHeight ( HWND _hWnd, HDC _hDC, const char* text, UINT uFormat = DT_CALCRECT );
		static long _GetTextWidth ( HWND _hWnd, HDC _hDC, const char* text, UINT uFormat = DT_CALCRECT );
		
		static void _DrawPixel ( HWND _hWnd, int x, int y, COLORREF ColorRGB24 );
	};
	
	
	// *** TODO *** delete fonts in destructor for controls, or use parent window font possibly

	class Button
	{
		static const char* const ClassName;
	
		static const int DefaultFlags_CmdButton = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON;
		static const int DefaultFlags_RadioButtonGroup = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON | WS_GROUP;
		static const int DefaultFlags_RadioButton = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON;
		static const int DefaultFlags_CheckBox = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX;
	
		Window* Parent;

		HWND hWnd;
		long long id;
		int x, y, width, height;
		char caption [ 256 ];
		
	public:
		
		// this will create a command button on the window
		HWND Create_CmdButton ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption = NULL, long long id = NULL, int flags = DefaultFlags_CmdButton );
		
		// this will create a radio button on the window
		// to start a group of radio buttons, use the WS_GROUP style for the first radio button in the group
		HWND Create_RadioButtonGroup ( Window* ParentWindow, int x, int y, int width = NULL, int height = NULL, const char* Caption = NULL, long long id = NULL, int flags = DefaultFlags_RadioButtonGroup );
		HWND Create_RadioButton ( Window* ParentWindow, int x, int y, int width = NULL, int height = NULL, const char* Caption = NULL, long long id = NULL, int flags = DefaultFlags_RadioButton );
		
		// this will create a check box on the window
		HWND Create_CheckBox ( Window* ParentWindow, int x, int y, int width = NULL, int height = NULL, const char* Caption = NULL, long long id = NULL, int flags = DefaultFlags_CheckBox );
		
		HWND GetHandleToParent () { return GetParent( hWnd ); }
		
		inline bool Enable () { return Button_Enable( hWnd, true ); }
		inline bool Disable () { return Button_Enable( hWnd, false ); }
		
		inline int GetIdealWidth () { SIZE t;/*BOOL*/ Button_GetIdealSize( hWnd, &t ); return t.cx; }
		inline int GetIdealHeight () { SIZE t;/*BOOL*/ Button_GetIdealSize( hWnd, &t ); return t.cy; }

		inline bool Set_XY ( int x, int y ) { return SetWindowPos ( hWnd, NULL, x, y, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER ); }
		inline bool Set_Size ( int width, int height ) { return SetWindowPos ( hWnd, NULL, NULL, NULL, width, height, SWP_NOMOVE | SWP_NOZORDER ); }
		
		
		inline int Get_X () { RECT rect; /*BOOL*/ GetWindowRect( hWnd, &rect ); return rect.left; }
		inline int Get_Y () { RECT rect; /*BOOL*/ GetWindowRect( hWnd, &rect ); return rect.top; }
		inline int Get_Width () { RECT rect; /*BOOL*/ GetWindowRect( hWnd, &rect ); return rect.right - rect.left; }
		inline int Get_Height () { RECT rect; /*BOOL*/ GetWindowRect( hWnd, &rect ); return rect.bottom - rect.top; }
		inline char* GetCaption () { return caption; }
		inline int SetCaption ( char* Caption ) { return Button_SetText( hWnd, (LPTSTR) Caption ); }
		inline void SetStyle ( int flags ) { Button_SetStyle( hWnd, (DWORD) flags, true ); }
		
		// this function returns either BST_CHECKED, BST_UNCHECKED, BST_INDETERMINATE
		// checks if a radio button or checkbox is checked (BS_RADIOBUTTON/BS_CHECKBOX)
		inline int GetCheck () { return Button_GetCheck( hWnd ); }
		
		// this function accepts either BST_CHECKED, BST_UNCHECKED, BST_INDETERMINATE
		// sets the "checked/unchecked" state of a radio button or checkbox
		inline void SetCheck ( int check ) { Button_SetCheck ( hWnd, check ); }

		///////You can also operate on object with just handle to control/////////
		static inline bool Enable ( HWND hWnd ) { return Button_Enable( hWnd, true ); }
		static inline bool Disable ( HWND hWnd ) { return Button_Enable( hWnd, false ); }
		static inline int SetCaption ( HWND hWnd, char* Caption ) { return Button_SetText( hWnd, (LPTSTR) Caption ); }
		static inline void SetStyle ( HWND hWnd, int flags ) { Button_SetStyle( hWnd, (DWORD) flags, true ); }
		
		// *note* must use BST_CHECKED/BST_UNCHECKED/BST_INDETERMINATE for these
		static inline int GetCheck ( HWND hWnd ) { return Button_GetCheck( hWnd ); }
		static inline void SetCheck ( HWND hWnd, int check ) { Button_SetCheck ( hWnd, check ); }
		
		// this will add an event for a window/control
		// specify NULL if an argument doesn't matter
		inline bool AddEvent ( unsigned int message, WindowClass::Window::EventFunction Func ) { return WindowClass::Window::AddEvent ( Parent->hWnd, hWnd, id, message, Func ); }
		
		// this will remove all events for an identifier
		// returns true on success and false otherwise
		inline bool RemoveEvent ( unsigned int message ) { return WindowClass::Window::RemoveEvent ( Parent->hWnd, id, message ); }
		
		// sets the font
		inline void SetFont ( HFONT hf ) { SendDlgItemMessage( Parent->hWnd, id, WM_SETFONT, (WPARAM) hf, (LPARAM) true ); }
		
	};

	// can show text, bitmap, or icon
	class Static
	{
		static const char* const ClassName;
	
		static const int DefaultFlags_Text = WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT;
		static const int DefaultFlags_Bitmap = WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_BITMAP;
		
		Window* Parent;
		
		HWND hWnd;
		long long id;
		int x, y, width, height;
		char caption [ 256 ];
		
	public:
		
		HWND Create_Text ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption = NULL, long long id = NULL, int flags = DefaultFlags_Text );
		HWND Create_Bitmap ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption = NULL, long long id = NULL, int flags = DefaultFlags_Bitmap );
		
		HWND GetHandleToParent () { return GetParent( hWnd ); }

		inline bool Set_XY ( int x, int y ) { return SetWindowPos ( hWnd, NULL, x, y, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER ); }
		inline bool Set_Size ( int width, int height ) { return SetWindowPos ( hWnd, NULL, NULL, NULL, width, height, SWP_NOMOVE | SWP_NOZORDER ); }
		
		inline bool SetText ( char* text ) { return /*BOOL WINAPI*/ SetWindowText( hWnd, (LPCTSTR) text ); }
		inline int SetFont ( HWND hFont ) { return /*LRESULT*/ SendDlgItemMessage( Parent->hWnd, id, WM_SETFONT, (WPARAM) hFont, (LPARAM) 0 ); }
		inline int SetImage ( HANDLE hBitmap ) { return SendDlgItemMessage( Parent->hWnd, id, STM_SETIMAGE , (WPARAM) IMAGE_BITMAP, (LPARAM) hBitmap ); }

		static inline HANDLE LoadPicture ( char* FilePathToPicture ) { return LoadImage ( NULL, FilePathToPicture, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE ); }
		
		inline bool AddEvent ( unsigned int message, WindowClass::Window::EventFunction Func ) { return WindowClass::Window::AddEvent ( Parent->hWnd, hWnd, id, message, Func ); }
		inline bool RemoveEvent ( unsigned int message ) { return WindowClass::Window::RemoveEvent ( Parent->hWnd, id, message ); }
		
		inline void SetFont ( HFONT hf ) { SendDlgItemMessage( Parent->hWnd, id, WM_SETFONT, (WPARAM) hf, (LPARAM) true ); }
	};
	
	// allows display of text for editing, or for read only
	class Edit
	{
		static const char* const ClassName;
	
		static const int DefaultFlags = WS_TABSTOP | WS_VISIBLE | WS_CHILD;
		static const int MaxStringLength = 256;
		
		Window* Parent;
		
		HWND hWnd;
		long long id;
		int x, y, width, height;
		char caption [ MaxStringLength ];
		
	public:
		
		HWND Create ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption = NULL, long long id = NULL, int flags = DefaultFlags );
		
		HWND GetHandleToParent () { return GetParent( hWnd ); }
		
		inline bool Enable () { return Edit_Enable( hWnd, true ); }
		inline bool Disable () { return Edit_Enable( hWnd, false ); }
		
		inline bool Set_XY ( int x, int y ) { return SetWindowPos ( hWnd, NULL, x, y, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER ); }
		inline bool Set_Size ( int width, int height ) { return SetWindowPos ( hWnd, NULL, NULL, NULL, width, height, SWP_NOMOVE | SWP_NOZORDER ); }
		
		inline char* GetText () { Edit_GetText( hWnd, (LPTSTR) caption, MaxStringLength ); return caption; }
		inline int GetTextLength () { return Edit_GetTextLength( hWnd ); }
		inline void SetMaxTextLength ( int MaxLength ) { Edit_LimitText( hWnd, MaxLength ); }
		inline bool SetReadOnly () { return Edit_SetReadOnly( hWnd, true ); }
		inline bool SetReadAndWrite () { return Edit_SetReadOnly( hWnd, false ); }
		inline int SetText ( char* text ) { return Edit_SetText( hWnd, (LPTSTR) text ); }

		inline bool AddEvent ( unsigned int message, WindowClass::Window::EventFunction Func ) { return WindowClass::Window::AddEvent ( Parent->hWnd, hWnd, id, message, Func ); }
		inline bool RemoveEvent ( unsigned int message ) { return WindowClass::Window::RemoveEvent ( Parent->hWnd, id, message ); }
		
		inline void SetFont ( HFONT hf ) { SendDlgItemMessage( Parent->hWnd, id, WM_SETFONT, (WPARAM) hf, (LPARAM) true ); }
	};
	
	class ComboBox
	{
		static const char* const ClassName;
	
		// displays the list box at all times
		static const int DefaultFlags_Simple = WS_TABSTOP | WS_VISIBLE | WS_CHILD | CBS_SIMPLE | CBS_HASSTRINGS;
		
		// displays an edit control and only the list box if dropped down
		static const int DefaultFlags_DropDown = WS_TABSTOP | WS_VISIBLE | WS_CHILD | CBS_DROPDOWN | CBS_HASSTRINGS;
		
		// displays a static control and only the list box if dropped down
		static const int DefaultFlags_DropDownList = WS_TABSTOP | WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS;
		
		static const int MaxStringLength = 256;
		
		Window* Parent;
		
		HWND hWnd;
		long long id;
		int x, y, width, height;
		char caption [ MaxStringLength ];
		
	public:
		
		HWND Create_Simple ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption = NULL, long long id = NULL, int flags = DefaultFlags_Simple );
		HWND Create_DropDown ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption = NULL, long long id = NULL, int flags = DefaultFlags_DropDown );
		HWND Create_DropDownList ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption = NULL, long long id = NULL, int flags = DefaultFlags_DropDownList );
		
		HWND GetHandleToParent () { return GetParent( hWnd ); }
		
		inline bool Enable () { return ComboBox_Enable( hWnd, true ); }
		inline bool Disable () { return ComboBox_Enable( hWnd, false ); }
		
		inline bool Set_XY ( int x, int y ) { return SetWindowPos ( hWnd, NULL, x, y, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER ); }
		inline bool Set_Size ( int width, int height ) { return SetWindowPos ( hWnd, NULL, NULL, NULL, width, height, SWP_NOMOVE | SWP_NOZORDER ); }
		
		inline int AddItem ( char* text ) { return ComboBox_AddString( hWnd, (LPCTSTR) text ); }
		inline int DeleteItem ( int index ) { return ComboBox_DeleteString( hWnd, index ); }
		inline int Reset () { return ComboBox_ResetContent( hWnd ); }

		// gets/selects an item in the combo box by zero-based index
		inline int GetCurSel () { return ComboBox_GetCurSel( hWnd ); }
		inline int SetCurSel ( int IndexOfItemToSelect ) { return ComboBox_SetCurSel( hWnd, IndexOfItemToSelect ); }

		inline int SetText ( char* text ) { return ComboBox_SetText( hWnd, (LPTSTR) text ); }


		inline int SetListHeight ( int HeightInPixels ) { return ComboBox_SetItemHeight( hWnd, 0, HeightInPixels ); }
		inline int SetItemHeight ( int HeightInPixels ) { return ComboBox_SetItemHeight( hWnd, -1, HeightInPixels ); }

		inline int GetText () { return ComboBox_GetText( hWnd, (LPTSTR) caption, MaxStringLength ); }
		inline int GetTextLength () { return ComboBox_GetTextLength( hWnd ); }


		inline int GetCount () { return ComboBox_GetCount( hWnd ); }

		inline bool AddEvent ( unsigned int message, WindowClass::Window::EventFunction Func ) { return WindowClass::Window::AddEvent ( Parent->hWnd, hWnd, id, message, Func ); }
		inline bool RemoveEvent ( unsigned int message ) { return WindowClass::Window::RemoveEvent ( Parent->hWnd, id, message ); }
		
		inline void SetFont ( HFONT hf ) { SendDlgItemMessage( Parent->hWnd, id, WM_SETFONT, (WPARAM) hf, (LPARAM) true ); }
	};
	
	// use LBS_MULTICOLUMN to make a multi column list box
	class ListBox
	{
		static const char* const ClassName;
	
		static const int DefaultFlags = WS_TABSTOP | WS_VISIBLE | WS_CHILD | LBS_HASSTRINGS;
		
		Window* Parent;
		
		HWND hWnd;
		long long id;
		int x, y, width, height;
		char caption [ 256 ];
		
		HWND Create ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption = NULL, long long id = NULL, int flags = DefaultFlags );
		
		HWND GetHandleToParent () { return GetParent( hWnd ); }
		
		inline bool Set_XY ( int x, int y ) { return SetWindowPos ( hWnd, NULL, x, y, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER ); }
		inline bool Set_Size ( int width, int height ) { return SetWindowPos ( hWnd, NULL, NULL, NULL, width, height, SWP_NOMOVE | SWP_NOZORDER ); }
		
		inline int AddString ( char* text ) { return SendDlgItemMessage( Parent->hWnd, id, LB_ADDSTRING, (WPARAM) NULL, (LPARAM) text ); }
		inline int DeleteString ( int index ) { return SendDlgItemMessage( Parent->hWnd, id, LB_DELETESTRING, (WPARAM) index, (LPARAM) NULL ); }
		
		inline bool AddEvent ( unsigned int message, WindowClass::Window::EventFunction Func ) { return WindowClass::Window::AddEvent ( Parent->hWnd, hWnd, id, message, Func ); }
		inline bool RemoveEvent ( unsigned int message ) { return WindowClass::Window::RemoveEvent ( Parent->hWnd, id, message ); }
		
		inline void SetFont ( HFONT hf ) { SendDlgItemMessage( Parent->hWnd, id, WM_SETFONT, (WPARAM) hf, (LPARAM) true ); }
	};
	
	class ListView
	{
		static const char* const ClassName;
		
		static const int DefaultFlags_Report_wHeader = WS_TABSTOP | WS_VISIBLE | WS_CHILD | LVS_REPORT;
		static const int DefaultFlags_Report_NoHeader = WS_TABSTOP | WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_NOCOLUMNHEADER;
		static const int DefaultFlags_Report_Dynamic_wHeader = WS_TABSTOP | WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_OWNERDATA;
		
		static const int MaxStringLength = 256;
		
		Window* Parent;
		
		HWND hWnd;
		long long id;
		int x, y, width, height;
		char caption [ MaxStringLength ];
		
		static LVITEM lvi;
		static LVCOLUMN lvc;
		
	public:
		
		HWND Create_wHeader ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption = "", long long id = NULL, int flags = DefaultFlags_Report_wHeader );
		HWND Create_NoHeader ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption = "", long long id = NULL, int flags = DefaultFlags_Report_NoHeader );
		HWND Create_Dynamic_wHeader ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption = "", long long id = NULL, int flags = DefaultFlags_Report_Dynamic_wHeader );
		
		HWND GetHandleToParent () { return GetParent( hWnd ); }
		
		inline bool Set_XY ( int x, int y ) { return SetWindowPos ( hWnd, NULL, x, y, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER ); }
		inline bool Set_Size ( int width, int height ) { return SetWindowPos ( hWnd, NULL, NULL, NULL, width, height, SWP_NOMOVE | SWP_NOZORDER ); }
		
		// *note* row and column values are zero based

		// column zero is the column label, then the other stuff starts at column 1
		inline void SetItemText ( int row, int column, const char* text ) { ListView_SetItemText( hWnd, row, column, (LPSTR) text ); }
		inline char* GetItemText ( int row, int column ) { ListView_GetItemText( hWnd, row, column, (LPTSTR) caption, MaxStringLength ); return caption; }
		
		inline int GetRowOfSelectedItem () { return ListView_GetNextItem ( hWnd, -1, LVNI_SELECTED ); }
		
		inline bool isRowVisible ( int row ) { return ListView_IsItemVisible( hWnd, row ); }
		inline int GetVisibleItemsCount () { return ListView_GetCountPerPage( hWnd ); }

		inline bool DeleteColumn ( int column ) { return ListView_DeleteColumn( hWnd, column ); }
		inline bool DeleteRow ( int row ) { return ListView_DeleteItem( hWnd, row ); }

		// makes sure that a row is visible by scrolling to it if it is not
		inline bool EnsureRowVisible ( int row ) { return ListView_EnsureVisible( hWnd, row, false ); }

		// no need for this
		// msdn says to use setitemcountex for virtual list views
		inline void SetItemCount ( int NumberOfItems ) { ListView_SetItemCount( hWnd, NumberOfItems ); }
		inline void SetItemCountEx ( int NumberOfItems ) { ListView_SetItemCountEx( hWnd, NumberOfItems, NULL ); }

		// set column width
		inline bool SetColumnWidth ( int column, int width ) { return ListView_SetColumnWidth( hWnd, column, width ); }
		inline bool AutoSizeColumn ( int column ) { return ListView_SetColumnWidth( hWnd, column, LVSCW_AUTOSIZE ); }


		// deletes all items in list-view control
		inline bool Reset () { return ListView_DeleteAllItems( hWnd ); }

		inline static LPLVCOLUMN CreateColumn ( int column, int WidthInPixels, char* HeaderText )
		{
			//LPLVCOLUMN l = new LVCOLUMN ();
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;
			lvc.cx = WidthInPixels;
			lvc.pszText = (LPTSTR) HeaderText;
			//l->cchTextMax = strlen ( HeaderText ) + 1;
			lvc.iSubItem = column;
			return &lvc;
		}
		
		inline int InsertColumn ( int column, LPLVCOLUMN data ) { return ListView_InsertColumn( hWnd, column, (const LPLVCOLUMN) data ); }
		
		inline static LPLVITEM CreateItem ( int row, int column, char* text )
		{
			//LPLVITEM l = new LVITEM ();
			lvi.mask = LVIF_TEXT | LVIF_STATE;
			lvi.state = 0;
			lvi.stateMask = 0;
			lvi.pszText = (LPTSTR) text;
			//l->cchTextMax = strlen ( text ) + 1;
			lvi.iItem = row;
			lvi.iSubItem = column;
			return &lvi;
		}
		
		inline int InsertRow ( int row ) { return ListView_InsertItem( hWnd, (const LPLVITEM) CreateItem ( row, 0, (char*)"" ) ); }


		// *note* must call this first
		inline static void InitCommonControls ()
		{
			INITCOMMONCONTROLSEX iccx;
			iccx.dwSize = sizeof( INITCOMMONCONTROLSEX );
			iccx.dwICC = ICC_LISTVIEW_CLASSES;
			InitCommonControlsEx( &iccx );
		}

		inline bool AddEvent ( unsigned int message, WindowClass::Window::EventFunction Func ) { return WindowClass::Window::AddEvent ( Parent->hWnd, hWnd, id, message, Func ); }
		inline bool RemoveEvent ( unsigned int message ) { return WindowClass::Window::RemoveEvent ( Parent->hWnd, id, message ); }
		
		inline void SetFont ( HFONT hf ) { SendDlgItemMessage( Parent->hWnd, id, WM_SETFONT, (WPARAM) hf, (LPARAM) true ); }
	};
	
	// use SBS_HORZ/SBS_VERT to create horizontal/vertical scroll bars
	// use SBS_RIGHTALIGN w/ SBS_VERT to right align vertical scroll bar
	// use SBS_BOTTOMALIGN w/ SBS_HORZ to bottom align a horizontal scroll bar
	class ScrollBar
	{
		static const char* const ClassName;
	
		static const int DefaultFlags = WS_TABSTOP | WS_VISIBLE | WS_CHILD;
		
		Window* Parent;
		
		HWND hWnd;
		long long id;
		int x, y, width, height;
		char caption [ 256 ];
		
		HWND Create ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption = NULL, long long id = NULL, int flags = DefaultFlags );
		
		inline bool Set_XY ( int x, int y ) { return SetWindowPos ( hWnd, NULL, x, y, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER ); }
		inline bool Set_Size ( int width, int height ) { return SetWindowPos ( hWnd, NULL, NULL, NULL, width, height, SWP_NOMOVE | SWP_NOZORDER ); }
		
		inline bool AddEvent ( unsigned int message, WindowClass::Window::EventFunction Func ) { return WindowClass::Window::AddEvent ( Parent->hWnd, hWnd, id, message, Func ); }
		inline bool RemoveEvent ( unsigned int message ) { return WindowClass::Window::RemoveEvent ( Parent->hWnd, id, message ); }
		
		inline void SetFont ( HFONT hf ) { SendDlgItemMessage( Parent->hWnd, id, WM_SETFONT, (WPARAM) hf, (LPARAM) true ); }
	};
	
	
};


#endif /* WINAPIHANDLER_H_ */
