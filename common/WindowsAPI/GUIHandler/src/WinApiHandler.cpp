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



#include "WinApiHandler.h"

#include <iostream>

#include <windows.h>

#include <gl/gl.h>

#pragma comment(lib, "comctl32")

#pragma comment(lib, "opengl32")


using namespace x64ThreadSafe::Utilities;
using namespace Utilities::Strings;





//////////// Static Class String Constants ///////////////////////////////////

const char* const WindowClass::Button::ClassName = "BUTTON";
const char* const WindowClass::Static::ClassName = "STATIC";
const char* const WindowClass::Edit::ClassName = "EDIT";
const char* const WindowClass::ComboBox::ClassName = "COMBOBOX";
const char* const WindowClass::ListBox::ClassName = "LISTBOX";
const char* const WindowClass::ScrollBar::ClassName = "SCROLLBAR";
const char* const WindowClass::ListView::ClassName = "SysListView32";



//////////////// Static Class Variables ////////////////////////////////////

WNDCLASS WindowClass::wc;
HINSTANCE WindowClass::hInst;
LPCTSTR WindowClass::className;
MSG WindowClass::msg;


vector<WindowClass::Window::Event*> WindowClass::Window::EventList;


Debug::Log WindowClass::debug;


volatile u32 WindowClass::Window::LastKeyPressed;


volatile unsigned long WindowClass::Window::Busy;
volatile unsigned long long WindowClass::Window::LastResult;

volatile u32 WindowClass::Window::InModalMenuLoop = false;


LVITEM WindowClass::ListView::lvi;
LVCOLUMN WindowClass::ListView::lvc;


///////// Remote Call Buffer //////////////////

volatile unsigned long WindowClass::Window::ReadIndex = 0;
volatile unsigned long WindowClass::Window::WriteIndex = 0;
volatile WindowClass::Window::RemoteCallData WindowClass::Window::RemoteCall_Buffer [ c_RemoteCallBufferSize ];



//////////////// Defines //////////////////////////////

// enable debugging
//#define INLINE_DEBUG


////////////////////// Class Functions ///////////////////////////////////

void WindowClass::Register ( HINSTANCE hInstance, LPCTSTR lpszClassName, LPCTSTR lpszMenuName, WNDPROC lpfnWndProc, UINT style, HBRUSH hbrBackground, HICON hIcon, HCURSOR hCursor, int cbClsExtra, int cbWndExtra )
{
#ifdef INLINE_DEBUG
	debug.Create ( "WindowClass_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\n\r\nEntering WindowClass::Register";
#endif

#ifdef INLINE_DEBUG
	debug << "; Setting up struct";
#endif

	wc.style = style;
	wc.lpfnWndProc = lpfnWndProc;
	wc.cbClsExtra = cbClsExtra;
	wc.cbWndExtra = cbWndExtra;
	wc.hInstance = hInstance;
	wc.hIcon = hIcon;
	wc.hCursor = hCursor;
	wc.hbrBackground = hbrBackground;
	wc.lpszMenuName = lpszMenuName;
	wc.lpszClassName = lpszClassName;
	RegisterClass( &wc );

#ifdef INLINE_DEBUG
	debug << "; Setting class name";
#endif

	// set class name
	className = lpszClassName;

#ifdef INLINE_DEBUG
	debug << "; Checking if GUI Thread is running";
#endif

	// start gui thread if it is not already started
	if ( !WindowClass::Window::GUIThread_isRunning )
	{
#ifdef INLINE_DEBUG
	debug << "; GUI Thread not running...starting GUI Thread";
#endif

		//WindowClass::Window::StartGUIThread ();
		WindowClass::Window::GUIThread_isRunning = 1;
	}
	
#ifdef INLINE_DEBUG
	debug << "->Exiting WindowClass::Register";
#endif
}


void WindowClass::DoSingleEvent ()
{
	MSG Msg;
	
	// need to block for messages
	// actually will just peek at the message
	if ( PeekMessage ( &Msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD ) > 0 )
	{
		///////////////////////////////
		// message was returned
		
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
		

	}
}


void WindowClass::DoEventsNoWait ()
{
	MSG Msg;
	
	// need to block for messages
	// actually will just peek at the message
	while ( PeekMessage ( &Msg, NULL, 0, 0, PM_REMOVE ) )
	{
		///////////////////////////////
		// message was returned
		
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
		

	}
}


void WindowClass::DoEvents ()
{
#ifdef INLINE_DEBUG
	debug << "\r\n\r\nMESSAGE LOOP STARTED";
#endif

	MSG Msg;

	// *** TESTING *** need to enter an alertable wait state just in case
	MsgWaitForMultipleObjectsEx( NULL, NULL, 1 /*cWaitPeriod*/, QS_ALLINPUT, MWMO_ALERTABLE );
	
	// need to block for messages
	// actually will just peek at the message
	while ( PeekMessage ( &Msg, NULL, 0, 0, PM_REMOVE ) )
	{
		///////////////////////////////
		// message was returned
		
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
		

	}
	
}


LRESULT CALLBACK WindowClass::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WindowClass::MenuBar::MenuBarItem* MBI;
	WindowClass::MenuBar* Menu;
	int i;
	WindowClass::Window::Event *e;
	
	bool found;
	
	// find out if an event was triggered
	for ( i = 0; i < WindowClass::Window::EventList.size (); i++ )
	{
		// get the event to check
		e = WindowClass::Window::EventList [ i ];

		// check that parent window matches and that message matches and control identifier matches
		if ( ( e->hwndParent == hWnd ) && ( e->message == message || ( !e->message ) ) && ( e->id == LOWORD(wParam) || ( !e->id  ) ) )
		{
			e->ef ( e->hwndCtrl, e->id, e->message, wParam, lParam );
			return 0;
		}
	}
	
	switch (message)
	{
	
		case WM_ENTERMENULOOP:
#ifdef INLINE_DEBUG
	debug << "\r\nEntered modal menu loop.\r\n";
#endif

			Lock_Exchange32 ( (long&) WindowClass::Window::InModalMenuLoop, 1 );
			break;
			
		case WM_EXITMENULOOP:
#ifdef INLINE_DEBUG
	debug << "\r\nExited modal menu loop.\r\n";
#endif

			Lock_Exchange32 ( (long&) WindowClass::Window::InModalMenuLoop, 0 );
			break;
		
		case WM_CREATE:
			return 0;

		case WM_CLOSE:
			PostQuitMessage( 0 );
			return 0;

		case WM_DESTROY:
			return 0;

		case WM_KEYUP:
			Window::LastKeyPressed = 0;
			return 0;
		
		case WM_KEYDOWN:
		
			switch ( wParam )
			{

				case VK_ESCAPE:
					PostQuitMessage(0);
					return 0;

			}
			
			////////////////////////////////////////////////////////
			// Search for the hotkey
			for ( i = 0; i < WindowClass::Window::ShortcutKey_Entries.size(); i++ )
			{
				if ( WindowClass::Window::ShortcutKey_Entries [ i ].hWnd == hWnd && WindowClass::Window::ShortcutKey_Entries [ i ].Key == wParam )
				{
#ifdef INLINE_DEBUG
					debug << "\r\nFound Hot Key; GetKeyState = " << hex << GetKeyState ( WindowClass::Window::ShortcutKey_Entries [ i ].Modifier );
#endif
				
					if ( ( GetKeyState ( WindowClass::Window::ShortcutKey_Entries [ i ].Modifier ) & 0x8000 ) || !WindowClass::Window::ShortcutKey_Entries [ i ].Modifier )
					{
#ifdef INLINE_DEBUG
						debug << "->Calling Hot Key function";
#endif

						cout << "\nKey= " << hex << WindowClass::Window::ShortcutKey_Entries [ i ].Key;
						cout << "\nModifier= " << hex << WindowClass::Window::ShortcutKey_Entries [ i ].Modifier;
						cout << "\nGetKeyState= " << hex << GetKeyState ( WindowClass::Window::ShortcutKey_Entries [ i ].Modifier );

						//////////////////////////////////////////////////////////////
						// make a call to the function associated with the hotkey
						WindowClass::Window::ShortcutKey_Entries [ i ].CallbackFunc ( (int) wParam );
						return 0;
					}
				}
			}
			
			// get the key that is down
			Window::LastKeyPressed = (u32) wParam;
			
			return 0;

			
		case WM_COMMAND:
		
#ifdef INLINE_DEBUG
	debug << "\r\n\r\nRECEIVED COMMAND. LoCommand=" << dec << LOWORD( wParam ) << " HiCommand=" << HIWORD ( wParam ) << " hWnd=" << (unsigned long long) hWnd;
#endif

			// check if there is an event for this command
			found = false;

			// check if they clicked on a menu in a window
			
			// find the menu bar for the window
			Menu = WindowClass::MenuBar::GetMenuBarForWindow ( hWnd );
			

			if ( Menu != NULL )
			{
#ifdef INLINE_DEBUG
	debug << "\r\nFound Menu bar for the window.";
#endif

				//MessageBox( hWnd, "Found menu bar for window.", "", NULL );
				// find out if a menu bar item was clicked
				MBI = Menu->FindItemById ( LOWORD(wParam) );
			
				if ( MBI != NULL )
				{
#ifdef INLINE_DEBUG
	debug << "\r\nFound the menu bar item on the menu bar";
#endif
					//MessageBox( NULL, "Calling the callback function.", "", NULL );

					// found the menu bar item, now call the callback function
					found = true;
					
					// only make a call back if there is a callback function for the menu item
					if ( MBI->CallbackFunc )
					{
#ifdef INLINE_DEBUG
	debug << "\r\nCalling call back function";
#endif
						MBI->CallbackFunc ( LOWORD( wParam ) );
					}
				}
				else
				{
					//MessageBox( NULL, "Did not find the menu bar item.", "", NULL );
#ifdef INLINE_DEBUG
	debug << "\r\nDid NOT find the menu bar item on the menu bar";
#endif
				}
				
			}
			
			// find out if an event was triggered
			//for ( i = 0; i < WindowClass::Window::EventList.size (); i++ )
			//{
			//	if ( WindowClass::Window::EventList [ i ]->id == LOWORD(wParam) )
			//	{
			//		// the event was triggered, call the function
			//		found = true;
			//		WindowClass::Window::EventList [ i ]->ef ( WindowClass::Window::EventList [ i ]->hwndCtrl, WindowClass::Window::EventList [ i ]->id );
			//		break;
			//	}
			//}

			return 0;
			
			
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );

	}

	return 0;
}




void WindowClass::Window::CreateMenuFromJson( json jsnMenuBar, const char* sLanguage )
{
	WindowClass::MenuBar::MenuBarItem* oMainMenuItem;

	WindowClass::MenuBar* oMenu = this->Menus;

	for ( const auto& item : jsnMenuBar["MenuBar"].items() )
	{
		//cout << "\nkey= " << item.key() << " value= " << item.value();
		//cout << "\nis_object= " << item.value().is_object();

		// menu needs a caption
		if ( item.value().contains( "Caption" ) )
		{
			// check if it has a caption for the language
			if ( item.value()["Caption"].contains( sLanguage ) )
			{
				// get the caption
				auto sMainMenuCaption = item.value()["Caption"][sLanguage];
				oMainMenuItem = oMenu->AddMainMenuItem ( sMainMenuCaption );

				cout << "\nAdded main menu item: " << sMainMenuCaption;

				// check for submenu(s)
				if ( item.value().contains( "SubMenu" ) )
				{
					// loop through the submenus //

					for ( const auto& subitem : item.value()["SubMenu"].items() )
					{

						// check if submenu item has a caption
						if ( subitem.value().contains( "Caption" ) )
						{
							// check it has caption for the language
							if ( subitem.value()["Caption"].contains( sLanguage ) )
							{
								// get the caption
								string sCaption = subitem.value()["Caption"][sLanguage];
								string sKey = subitem.key();

								// check if this is a menu or an item
								if ( !subitem.value().contains( "SubMenu" ) )
								{
									// this is an item //

									// check if there is a function
									WindowClass::MenuBar::Function pFunction = NULL;
									if ( subitem.value().contains("Function") )
									{
										unsigned long long ullFunction = subitem.value()["Function"];
										pFunction = (WindowClass::MenuBar::Function) ( ullFunction );

										// check if there is a shortcut key
										if ( subitem.value().contains("ShortcutKey") )
										{
											unsigned long ulShortcutKey = 0;
											unsigned long ulModifierKey = 0;
											string sShortcutKey;

											//ulShortcutKey = subitem.value()["ShortcutKey"];
											ulShortcutKey = subitem.value()["ShortcutKey"]["Key"];
											ulModifierKey = subitem.value()["ShortcutKey"]["Modifier"];

											// add shortcut key to program
											this->AddShortcutKey( pFunction, ulShortcutKey, ulModifierKey );

											sShortcutKey = (char) ulShortcutKey;
											if ( ulModifierKey )
											{
												sShortcutKey = "CTRL+" + sShortcutKey;
											}
											
											// add shortcut key to caption //
											sCaption += "\t" + sShortcutKey;

										}
									}

									// add the item to the menu
									oMainMenuItem->AddItem( sCaption, sKey, pFunction );


								}
								else
								{
									WindowClass::MenuBar::MenuBarItem* oSubMenuItem;

									// this is a menu //
									oSubMenuItem = oMainMenuItem->AddMenu( sCaption );

									// loop through the submenu //

									for ( const auto& subitem2 : subitem.value()["SubMenu"].items() )
									{

										// check if submenu item has a caption
										if ( subitem2.value().contains( "Caption" ) )
										{
											// check it has caption for the language
											if ( subitem2.value()["Caption"].contains( sLanguage ) )
											{
												// get the caption
												string sCaption = subitem2.value()["Caption"][sLanguage];
												string sKey = subitem2.key();

												// check if this is a menu or an item
												if ( !subitem2.value().contains( "SubMenu" ) )
												{
													// this is an item //

													// check if there is a function
													WindowClass::MenuBar::Function pFunction = NULL;
													if ( subitem2.value().contains("Function") )
													{
														unsigned long long ullFunction = subitem2.value()["Function"];
														pFunction = (WindowClass::MenuBar::Function) ( ullFunction );

														// check if there is a shortcut key
														if ( subitem2.value().contains("ShortcutKey") )
														{
															unsigned long ulShortcutKey = 0;
															unsigned long ulModifierKey = 0;
															string sShortcutKey;

															//sShortcutKey = subitem2.value()["ShortcutKey"];
															ulShortcutKey = subitem2.value()["ShortcutKey"]["Key"];
															ulModifierKey = subitem2.value()["ShortcutKey"]["Modifier"];

															// add shortcut key to program
															this->AddShortcutKey( pFunction, ulShortcutKey, ulModifierKey );

															sShortcutKey = (char) ulShortcutKey;
															if ( ulModifierKey )
															{
																sShortcutKey = "CTRL+" + sShortcutKey;
															}
															
															// add shortcut key to caption //
															sCaption += "\t" + sShortcutKey;
														}
													}

													// add the item to the menu
													oSubMenuItem->AddItem( sCaption, sKey, pFunction );


												}
												else
												{
													WindowClass::MenuBar::MenuBarItem* oSubMenuItem2;

													// this is a menu //
													oSubMenuItem2 = oSubMenuItem->AddMenu( sCaption );

													// loop through the submenu3 //

													for ( const auto& subitem3 : subitem2.value()["SubMenu"].items() )
													{

														// check if submenu item has a caption
														if ( subitem3.value().contains( "Caption" ) )
														{
															// check it has caption for the language
															if ( subitem3.value()["Caption"].contains( sLanguage ) )
															{
																// get the caption
																string sCaption = subitem3.value()["Caption"][sLanguage];
																string sKey = subitem3.key();

																// check if this is a menu or an item
																if ( !subitem3.value().contains( "SubMenu" ) )
																{
																	// this is an item //

																	// check if there is a function
																	WindowClass::MenuBar::Function pFunction = NULL;
																	if ( subitem3.value().contains("Function") )
																	{
																		unsigned long long ullFunction = subitem3.value()["Function"];
																		pFunction = (WindowClass::MenuBar::Function) ( ullFunction );

																		// check if there is a shortcut key
																		if ( subitem3.value().contains("ShortcutKey") )
																		{
																			unsigned long ulShortcutKey = 0;
																			unsigned long ulModifierKey = 0;
																			string sShortcutKey;

																			//sShortcutKey = subitem3.value()["ShortcutKey"];
																			ulShortcutKey = subitem3.value()["ShortcutKey"]["Key"];
																			ulModifierKey = subitem3.value()["ShortcutKey"]["Modifier"];

																			// add shortcut key to program
																			this->AddShortcutKey( pFunction, ulShortcutKey, ulModifierKey );

																			sShortcutKey = (char) ulShortcutKey;
																			if ( ulModifierKey )
																			{
																				sShortcutKey = "CTRL+" + sShortcutKey;
																			}
																			
																			// add shortcut key to caption //
																			sCaption += "\t" + sShortcutKey;
																		}
																	}

																	// add the item to the menu
																	oSubMenuItem2->AddItem( sCaption, sKey, pFunction );

																}

															}

														}

													}


												}	// end if else if ( !subitem2.value().contains( "SubMenu" ) )

											}	// end if ( subitem2.value()["Caption"].contains( sLanguage ) )

										}	// end if ( subitem2.value().contains( "Caption" ) )

									}	// end for ( const auto& subitem2 : subitem.value()["SubMenu"].items() )


								}	// end if else if ( !subitem.value().contains( "SubMenu" ) )

							}	// end if ( subitem.value()["Caption"].contains( sLanguage ) )

						}	// end if ( subitem.value().contains( "Caption" ) )

					}	// end for ( const auto& subitem : item.value()["SubMenu"].items() )

				}	// end if ( item.value().contains( "SubMenu" ) )

			}	// end if ( item.value()["Caption"].contains( sLanguage ) )

		}	// end if ( item.value().contains( "Caption" ) )

	}	// end for ( const auto& item : jsnMenuBar["MenuBar"].items() )

}





long WindowClass::Window::GUIThread_isRunning = 0;
Api::Thread* WindowClass::Window::GUIThread;

/*
int WindowClass::Window::CreateMessageHandlerThread ()
{
	// check if this is in a GUI Thread, and if not convert to one
	IsGUIThread ( true );

	// We'll try creating the window on a separate thread
	MessageHandlerThread = new Api::Thread ( StartWindowMessageLoop, (LPVOID) NULL );

	//cout << "Created new thread with id: " << MessageHandlerThread->ThreadId << "\n";

	return MessageHandlerThread->Attach ( GetCurrentThreadId () );
}
*/

WindowClass::Window::Window ( void )
{
	// zero structure
	// zero object
	memset ( this, 0, sizeof( Window ) );
	
	// init vars
	hWnd = 0;
	hDC = 0;
	hRC = 0;
	hFont = 0;
	hFontOld = 0;

	// constructor does not create the window
	isWindowCreated = 0;
}

WindowClass::Window::~Window ( void )
{
	if ( OpenGL_Enabled )
	{
		DisableOpenGL ();
	}
	

	// destroy the actual window
	// ***Problem*** : Cannot destroy windows created by a different thread - need to use remote call!!!
	//DestroyWindow( hWnd );
	Destroy ();
	
	if ( hFont ) DeleteObject ( hFont );
	
	// remove any events associated with window
	RemoveEvent ( hWnd, NULL, NULL );
			
	// destroy the menu bar
	delete Menus;
}




void WindowClass::Window::StartGUIThread ()
{
	GUIThread = new Api::Thread ( StartWindowMessageLoop, (LPVOID) NULL );
}

int WindowClass::Window::StartWindowMessageLoop ( void* Param )
{
	// check if this is a GUI thread - and if not convert to one! This works.
	//IsGUIThread ( true );

	//WindowClass::Window* w = (WindowClass::Window*) Param;
	return WindowClass::Window::WindowMessageLoop ();
}

//HDC CurrentHDC;

int WindowClass::Window::WindowMessageLoop ()
{
#ifdef INLINE_DEBUG
	debug << "\r\n\r\nMESSAGE LOOP STARTED";
#endif

	MSG Msg;

	_Create_Params* c;
	_wParams d;
	string* s;
	
	bool GUIThreadBusy;
	

	/*
	hWnd = CreateWindow( (LPCTSTR) lpClassName, (LPCTSTR) lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam );
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);
	*/
	
	while ( 1 )
	{

		// no need to use up processor cycles
		// I'll do this further down
		//Sleep ( 1 );
		
		// if the gui thread is busy, then let it keep running without yielding to other threads
		//GUIThreadBusy = false;
		
		// need to get messages but also handle remote calls
		//cout << "\nSetting timer";
		//SetTimer( NULL, 1, 1, NULL );
		
		// *** TESTING *** need to enter an alertable wait state just in case
		MsgWaitForMultipleObjectsEx( NULL, NULL, 1 /*cWaitPeriod*/, QS_ALLINPUT, MWMO_ALERTABLE );
		
		// process remote calls
		while ( ( ReadIndex & c_RemoteCallBufferMask ) != ( WriteIndex & c_RemoteCallBufferMask ) )
		{
			//cout << "\nPerforming remote call";
			
			//GUIThreadBusy = true;
			
			// make the remote call
			LastResult = ( RemoteCall_Buffer [ ( ReadIndex & c_RemoteCallBufferMask ) ].FunctionToCall ) ( (void*) RemoteCall_Buffer [ ( ReadIndex & c_RemoteCallBufferMask ) ].Param );
			
			// update read index
			Lock_ExchangeAdd32 ( (long&)ReadIndex, 1 );
			
			// remote call is done now
			Lock_Exchange32 ( (long&)WindowClass::Window::Busy, 0 );
		}
			
		// need to block for messages
		// actually will just peek at the message
		//while(GetMessage(&Msg, NULL, 0, 0) > 0)
		if ( PeekMessage ( &Msg, NULL, 0, 0, PM_REMOVE ) )
		{
			///////////////////////////////
			// message was returned
			
			//GUIThreadBusy = true;

			
			///////////////////////////////////////////////////////////////
			// *** TODO *** need to check for quit message manually
			////////////////////////////////////////////////////////////////

			
			// check if it is time for a "remote call"
			if ( Msg.message == WM_TIMER /*&& Msg.hwnd == NULL && Msg.lParam == NULL*/ )
			{
				// kill timer
				//cout << "\nKilling timer";
				KillTimer( NULL, Msg.wParam );
				
				// process remote calls
				while ( ( ReadIndex & c_RemoteCallBufferMask ) != ( WriteIndex & c_RemoteCallBufferMask ) )
				{
					//cout << "\nPerforming remote call";
					
					//GUIThreadBusy = true;
					
					// make the remote call
					LastResult = ( RemoteCall_Buffer [ ( ReadIndex & c_RemoteCallBufferMask ) ].FunctionToCall ) ( (void*) RemoteCall_Buffer [ ( ReadIndex & c_RemoteCallBufferMask ) ].Param );
					
					// update read index
					Lock_ExchangeAdd32 ( (long&)ReadIndex, 1 );
					
					// remote call is done now
					Lock_Exchange32 ( (long&)WindowClass::Window::Busy, 0 );
				}
				
			}
			else if ( Msg.message >= WM_APP )
			{

				switch ( Msg.message )
				{
				
					case GUI_REMOTECALL:
				
#ifdef INLINE_DEBUG
	debug << "; GUI_REMOTECALL";
#endif
						//cout << "\nMaking Remote call";

			
						// just need to make a call
						WindowClass::Window::LastResult = ((WindowClass::Window::RemoteFunction) Msg.wParam) ( (void*) Msg.lParam );
					
#ifdef INLINE_DEBUG
	debug << "; Call Complete\r\n";
#endif

						//cout << "\nRemote call complete";
						
						// remote function call is done now
						Lock_Exchange32 ( (long&)WindowClass::Window::Busy, 0 );
						
						break;

					case GUI_CREATE:
						// I'll make this the create window function
						//c = (_Create_Params*) Msg.wParam;
						//_Create ( c->w, c->lpWindowName, c->x, c->y, c->nWidth, c->nHeight, c->dwStyle, c->hMenu, c->hWndParent, c->lpParam, c->hInstance, c->lpClassName );

						// we must delete the Params object because nothing else will
						//delete c;

						break;

					case GUI_SETTEXT:
						d.Value = (u64) Msg.wParam;
						s = (string*) Msg.lParam;
						_SetText ( Msg.hwnd, d.x, d.y, s->c_str(), d.fontSize, d.uFormat );

						// must delete anything that was allocated on heap
						delete s;
						break;

					case GUI_PRINTTEXT:
						d.Value = (u64) Msg.wParam;
						s = (string*) Msg.lParam;
						_PrintText ( Msg.hwnd, d.x, d.y, s->c_str(), d.fontSize, d.uFormat );

						// must delete anything that was allocated on heap
						delete s;
						break;

					case GUI_CHANGETEXTCOLOR:
						_ChangeTextColor ( Msg.hwnd, (u32) Msg.wParam );
						break;

					case GUI_CHANGETEXTBKCOLOR:
						_ChangeTextBkColor ( Msg.hwnd, (u32) Msg.wParam );
						break;

					/*
					case GUI_CHANGEFONT:
						s = (string*) Msg.lParam;
						_ChangeFont ( Msg.hwnd, (u32) Msg.wParam, s->c_str() );
						delete s;
						break;
					*/

					case GUI_CLEAR:
						_Clear ( Msg.hwnd );
						break;

					case GUI_CHANGEBKMODE:
						_ChangeBkMode ( Msg.hwnd, (int) Msg.wParam );
						break;
						
					case GUI_ADDSHORTCUTKEY:
						_AddShortcutKey ( (ShortcutKey_Entry*) Msg.wParam );
						break;
						
					case GUI_DRAWPIXEL:
						_DrawPixel ( Msg.hwnd, (int) ( Msg.wParam >> 16 ), (int) ( Msg.wParam & 0xffff ), (COLORREF) Msg.lParam );
						break;
/*					
					case GUI_BEGINDRAWING:
						CurrentHDC = GetDC ( Msg.hwnd );
						break;
						
					case GUI_ENDDRAWING:
						ReleaseDC ( Msg.hwnd, CurrentHDC );
						break;
*/
				}
			}
			else
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
			

			// need to set the timer again
			//cout << "\nSetting Timer";
			//SetTimer( NULL, 1, 1, NULL );

		}
		
		///////////////////////////////////////////////////
		// *** Process Remote Calls Here ***

		/*
		// check if there is a function to remote call
		if ( ( ReadIndex & c_RemoteCallBufferMask ) != ( WriteIndex & c_RemoteCallBufferMask ) )
		{
			//cout << "\nNew style remote function call.\n";
			
			GUIThreadBusy = true;
			
			// make the remote call
			LastResult = ( RemoteCall_Buffer [ ( ReadIndex & c_RemoteCallBufferMask ) ].FunctionToCall ) ( RemoteCall_Buffer [ ( ReadIndex & c_RemoteCallBufferMask ) ].Param );
			
			// update read index
			Lock_ExchangeAdd32 ( (long&)ReadIndex, 1 );
			
			// remote call is done now
			Lock_Exchange32 ( (long&)WindowClass::Window::Busy, 0 );
		}
		
		if ( !GUIThreadBusy )
		{
			// no need to make a remote call, so thread can stop burning clock cycles for a millisecond
			Sleep ( 1 );
		}
		*/
	}

	//MessageHandlerThread->Exit ( Msg.wParam );

#ifdef INLINE_DEBUG
	debug << "\r\n\r\nMESSAGE LOOP DONE";
#endif

	return 0;
}


string WindowClass::Window::ShowFileOpenDialog ()
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[1024];       // buffer for file name
	//HWND hwnd;              // owner window
	//HANDLE hf;              // file handle

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box.

	if (GetOpenFileName(&ofn)==TRUE)
	{
		//cout << "\nDEBUG: GetOpenFileName = TRUE.";
		
		return ofn.lpstrFile;
		//return szFile;
	}

	//cout << "\nDEBUG: GetOpenFileName = FALSE.";
	
	return "";
}


vector<string> WindowClass::Window::ShowFileOpenDialogMultiSelect ()
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[2048];       // buffer for file name
	//HWND hwnd;              // owner window
	//HANDLE hf;              // file handle

	string FilePath;
	string FileName;
	vector<string> buffer;

	char* Strings;

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	// Display the Open dialog box.

	if (GetOpenFileName(&ofn)==TRUE)
	{
		// get a pointer into result
		Strings = (char*)ofn.lpstrFile;

		// get file path
		FilePath = Strings;

		//cout << "File path: " << FilePath.c_str() << "\n";
		int Count = 0;

		// advance Strings pointer into next file name
		for ( int i = 0;; i++ )
		{
			if ( (*Strings++) == 0 )
			{
				// another zero means we are done
				if ( (*Strings) == 0 )
				{
					if ( Count == 0 )
					{
						// use the file path as the file since only 1 file was selected
						buffer.push_back ( FilePath );
					}

					break;
				}

				FileName = Strings;
				//cout << "File name: " << FileName.c_str() << "\n";
				buffer.push_back ( FilePath + "\\" + FileName );
				Count++;
			}
		}
	}

	return buffer;
}


string WindowClass::Window::ShowFileSaveDialog ()
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name
	//HWND hwnd;              // owner window
	//HANDLE hf;              // file handle

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST;	// | OFN_FILEMUSTEXIST;

	// Display the Save dialog box.

	if (GetSaveFileName(&ofn)==TRUE)  return ofn.lpstrFile;

	return "";
}



bool WindowClass::Window::EnableOpenGL ( void )
{
	PIXELFORMATDESCRIPTOR pfd;
	int format;

	// get the device context (DC)
	hDC = GetDC( hWnd );

	// set the pixel format for the DC
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat( hDC, &pfd );
	SetPixelFormat( hDC, format, &pfd );

	// create and enable the render context (RC)
	hRC = wglCreateContext( hDC );
	wglMakeCurrent( hDC, hRC );
	
	OpenGL_Enabled = true;

	return true;
}

void WindowClass::Window::DisableOpenGL ( void )
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hRC );
	ReleaseDC( hWnd, hDC );
	
	OpenGL_Enabled = false;
}

bool WindowClass::Window::EnableVSync ( void )
{
	typedef BOOL (WINAPI * WGLSwapIntervalEXT) (int);

	WGLSwapIntervalEXT wglSwapIntervalEXT = (WGLSwapIntervalEXT) wglGetProcAddress( "wglSwapIntervalEXT" );

	if( wglSwapIntervalEXT )
	{
		wglSwapIntervalEXT( 1 );
		return true;
	}

	return false;
}

bool WindowClass::Window::DisableVSync ( void )
{
	typedef BOOL (WINAPI * WGLSwapIntervalEXT) (int);

	WGLSwapIntervalEXT wglSwapIntervalEXT = (WGLSwapIntervalEXT) wglGetProcAddress( "wglSwapIntervalEXT" );

	if( wglSwapIntervalEXT )
	{
		wglSwapIntervalEXT( 0 );
		return true;
	}

	return false;
}

void WindowClass::Window::FlipScreen ( void )
{
	SwapBuffers( hDC );
	
	// ***TESTING***
	//glFlush ();
	//glFinish ();
}

/*
bool WindowClass::Window::BeginDrawing ( void )
{
	hDC = GetDC ( hWnd );
	return true;
}
*/

/*
bool WindowClass::Window::EndDrawing ( void )
{
	ReleaseDC( hWnd, hDC );
	return true;
}
*/

bool WindowClass::Window::ChangeTextColor ( u32 color )
{
	return PostMessage ( hWnd, GUI_CHANGETEXTCOLOR, (WPARAM) color, (LPARAM) NULL );

	//SetTextColor( hDC, (COLORREF) color );
	//return true;
}

bool WindowClass::Window::_ChangeTextColor ( HWND _hWnd, u32 color )
{
	HDC _hDC;

	_hDC = GetDC ( _hWnd );
	SetTextColor( _hDC, (COLORREF) color );
	ReleaseDC( _hWnd, _hDC );
	return true;
}


bool WindowClass::Window::ChangeTextBkColor ( u32 color )
{
	PostMessage ( hWnd, GUI_CHANGETEXTBKCOLOR, (WPARAM) color, (LPARAM) NULL );
	//SetBkColor( hDC, (COLORREF) color );
	return true;
}

bool WindowClass::Window::_ChangeTextBkColor ( HWND _hWnd, u32 color )
{
	HDC _hDC;

	_hDC = GetDC ( _hWnd );
	SetBkColor( _hDC, (COLORREF) color );
	ReleaseDC( _hWnd, _hDC );
	return true;
}


bool WindowClass::Window::ChangeBkMode ( int iBkMode )
{
	return PostMessage ( hWnd, GUI_CHANGEBKMODE, (WPARAM) iBkMode, (LPARAM) NULL );
	//SetBkMode( hDC, iBkMode );
}

bool WindowClass::Window::_ChangeBkMode ( HWND _hWnd, int iBkMode )
{
	HDC _hDC;

	_hDC = GetDC ( _hWnd );
	SetBkMode( _hDC, iBkMode );
	ReleaseDC( _hWnd, _hDC );

	return true;
}

void WindowClass::Window::DrawPixel ( int x, int y, COLORREF ColorRGB24 )
{
	while( !PostMessage ( hWnd, GUI_DRAWPIXEL, (WPARAM) ( x << 16 ) | y, (LPARAM) ColorRGB24 ) );
}

void WindowClass::Window::_DrawPixel ( HWND _hWnd, int x, int y, COLORREF ColorRGB24 )
{
	HDC _hDC;

	_hDC = GetDC ( _hWnd );
	SetPixelV( _hDC, x, y, ColorRGB24 );
	ReleaseDC( _hWnd, _hDC );
}



/*
bool WindowClass::Window::ChangeFont ( u32 fontSize, char* fontName )
{
	PostMessage ( hWnd, GUI_CHANGEFONT, (WPARAM) fontSize, (LPARAM) new string ( fontName ) );
}

bool WindowClass::Window::_ChangeFont ( HWND _hWnd, u32 fontSize, char* fontName )
{
	long lfHeight;
	HFONT hf;
	HFONT _hFontOld;
	HDC _hDC;

	lfHeight = -MulDiv( fontSize, GetDeviceCaps(_hDC, LOGPIXELSY), 72);
	hf = CreateFont( lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, fontName );

	_hDC = GetDC ( _hWnd );
	_hFontOld = (HFONT) SelectObject(_hDC, hf);
	ReleaseDC( _hWnd, _hDC );

	if ( _hFontOld ) DeleteObject ( _hFontOld );
}
*/

bool WindowClass::Window::SetText ( long x, long y, const char* text, u32 fontSize, UINT uFormat )
{
	_wParams p( x, y, fontSize, uFormat );
	return PostMessage ( hWnd, GUI_SETTEXT, (WPARAM) p.Value, (LPARAM) new string( text ) );
}

// this calls the function from the GUI thread
bool WindowClass::Window::_SetText ( HWND _hWnd, long x, long y, const char* text, u32 fontSize, UINT uFormat )
{
	RECT rect;
	HDC _hDC;

	long lfHeight;
	HFONT hf;
	HFONT _hFontOld;

	GetClientRect ( _hWnd, &rect );
	rect.left = x;
	rect.top = y;

	_hDC = GetDC ( _hWnd );

	// we'll put the font stuff in here
	lfHeight = -MulDiv( fontSize, GetDeviceCaps(_hDC, LOGPIXELSY), 72);
	hf = CreateFont( lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Courier New" /*"Arial"*/ /*fontName*/ );

	_hFontOld = (HFONT) SelectObject(_hDC, hf);

	// draw the text
	DrawText( _hDC, (LPCTSTR) text, -1, &rect, uFormat );

	// select back the old font and delete the one we just created
	SelectObject(_hDC, _hFontOld);
	if ( hf ) DeleteObject ( hf );

	ReleaseDC( _hWnd, _hDC );

	return true;
}


long WindowClass::Window::_GetTextHeight ( HWND _hWnd, HDC _hDC, const char* text, UINT uFormat )
{
	RECT rect;

	GetClientRect ( _hWnd, &rect );
	rect.left = 0;
	rect.top = 0;
	return (long) DrawText( _hDC, (LPCTSTR) text, -1, &rect, uFormat | DT_CALCRECT );
}

long WindowClass::Window::_GetTextWidth ( HWND _hWnd, HDC _hDC, const char* text, UINT uFormat )
{
	RECT rect;

	GetClientRect ( _hWnd, &rect );
	rect.left = 0;
	rect.top = 0;
	DrawText( _hDC, (LPCTSTR) text, -1, &rect, uFormat | DT_CALCRECT );
	return (long) rect.right;
}

bool WindowClass::Window::PrintText ( long x, long LineNumber, const char* text, u32 fontSize, UINT uFormat )
{
	_wParams p( x, LineNumber, fontSize, uFormat );
	return PostMessage ( hWnd, GUI_PRINTTEXT, (WPARAM) p.Value, (LPARAM) new string( text ) );
}

bool WindowClass::Window::_PrintText ( HWND _hWnd, long x, long LineNumber, const char* text, u32 fontSize, UINT uFormat )
{
	long LineHeight;
	RECT rect;
	HDC _hDC;

	long lfHeight;
	HFONT hf;
	HFONT _hFontOld;

	_hDC = GetDC ( _hWnd );

	// we'll put the font stuff in here
	lfHeight = -MulDiv( fontSize, GetDeviceCaps(_hDC, LOGPIXELSY), 72);
	hf = CreateFont( lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Courier New" /*"Arial"*/ /*fontName*/ );

	_hFontOld = (HFONT) SelectObject(_hDC, hf);

	// get the text height
	GetClientRect ( _hWnd, &rect );
	rect.left = 0;
	rect.top = 0;
	LineHeight = _GetTextHeight ( _hWnd, _hDC, "TEST", uFormat );

	// draw the text
	GetClientRect ( _hWnd, &rect );
	rect.left = x;
	rect.top = LineNumber * LineHeight;
	DrawText( _hDC, (LPCTSTR) text, -1, &rect, uFormat );

	// select back the old font and delete the one we just created
	SelectObject(_hDC, _hFontOld);
	if ( hf ) DeleteObject ( hf );

	ReleaseDC( _hWnd, _hDC );

	return true;
}

bool WindowClass::Window::Clear ( void )
{
	return PostMessage ( hWnd, GUI_CLEAR, (WPARAM) NULL, (LPARAM) NULL );
	/*
	RECT rect;
	GetClientRect ( hWnd, &rect );

	// clear the window
	if ( FillRect ( hDC, &rect, (HBRUSH)GetStockObject( BLACK_BRUSH ) ) == 0 ) return false;

	return true;
	*/
}


bool WindowClass::Window::_Clear ( HWND _hWnd )
{
	HDC _hDC;
	RECT rect;
	GetClientRect ( _hWnd, &rect );

	// clear the window
	_hDC = GetDC ( _hWnd );
	if ( FillRect ( _hDC, &rect, (HBRUSH)GetStockObject( BLACK_BRUSH ) ) == 0 ) return false;
	ReleaseDC( _hWnd, _hDC );

	return true;
}






////////////////////
// Menu Bar Stuff //
////////////////////

volatile u32 WindowClass::NextIndex = WindowClass::StartId;

vector<WindowClass::MenuBar*> WindowClass::MenuBar::ListOfMenuBars;
vector<WindowClass::MenuBar::MenuBarItem*> WindowClass::MenuBar::ListOfMenuBarItems;

WindowClass::MenuBar::MenuBar ( HWND _hWnd )
{
	MainMenu = CreateMenu ();
	
	// set the window to place menu on
	hWnd = _hWnd;
	
	// set the id for the menu bar
	// other threads might be using this
	Id = (u32) Lock_ExchangeAdd32 ( (long&)NextIndex, 1 );
	
	// add reference into list of created menu bars
	ListOfMenuBars.push_back ( this );
}


WindowClass::MenuBar::~MenuBar ()
{
	vector<MenuBarItem*>::iterator j;
	
	// erase pointer in list of menu bars
	for ( vector<MenuBar*>::iterator i = ListOfMenuBars.begin(); i != ListOfMenuBars.end(); i++ )
	{
		// if this menu bar is found, then delete it
		if ( (*i)->hWnd == hWnd )
		{
			// object is already being deleted, so don't delete or it crashes!
			// just erase from master list of menu bars
			
			ListOfMenuBars.erase ( i );
			break;
		}
	}

	// erase all menu bar items for menu bar
	j = ListOfMenuBarItems.begin();
	while ( j != ListOfMenuBarItems.end() )
	{
		// delete any menu bar items for this menu bar
		for ( j = ListOfMenuBarItems.begin(); j != ListOfMenuBarItems.end(); j++ )
		{
			if ( (*j)->MenuBarId == Id )
			{
				delete (*j);
				ListOfMenuBarItems.erase ( j );
				break;
			}
		}
	}

}


WindowClass::MenuBar::MenuBarItem* WindowClass::MenuBar::AddMainMenuItem ( string _Caption, u32 _Id )
{
	MenuBarItem* NewMenu = new MenuBarItem ( Id, Id, _Caption, _Id );

	ListOfMenuBarItems.push_back ( NewMenu );
	
	AppendMenu( MainMenu, MF_STRING | MF_POPUP, (UINT64)NewMenu->Menu, _Caption.c_str() );
	
	return NewMenu;
}


int WindowClass::MenuBar::CheckItem ( u32 _Id )
{
	MenuBarItem *m;
	m = FindItemById ( _Id );
	if ( m ) return m->CheckItem();
	return NULL;
}

int WindowClass::MenuBar::UnCheckItem ( u32 _Id )
{
	MenuBarItem *m;
	m = FindItemById ( _Id );
	if ( m ) return m->UnCheckItem();
	return NULL;
}


int WindowClass::MenuBar::CheckItem ( string _Caption )
{
	MenuBarItem *m;
	m = FindItemByCaption ( _Caption );
	if ( m ) return m->CheckItem();
	return NULL;
}

int WindowClass::MenuBar::UnCheckItem ( string _Caption )
{
	MenuBarItem *m;
	m = FindItemByCaption ( _Caption );
	if ( m ) return m->UnCheckItem();
	return NULL;
}


WindowClass::MenuBar* WindowClass::MenuBar::GetMenuBarForWindow ( HWND _hWnd )
{
	for ( int i = 0; i < ListOfMenuBars.size(); i++ )
	{
		// if this menu bar is found, then return it
		if ( ListOfMenuBars [ i ]->hWnd == _hWnd )
		{
			return ListOfMenuBars [ i ];
		}
	}
	
	return NULL;
}

WindowClass::MenuBar* WindowClass::MenuBar::FindMenuBarById ( u32 _Id )
{
	for ( int i = 0; i < ListOfMenuBars.size(); i++ )
	{
		// if this menu bar is found, then return it
		if ( ListOfMenuBars [ i ]->Id == _Id )
		{
			return ListOfMenuBars [ i ];
		}
	}
	
	return NULL;
}


void WindowClass::MenuBar::Show ()
{
	// set the menu
	SetMenu( hWnd, MainMenu );
}

WindowClass::MenuBar::MenuBarItem* WindowClass::MenuBar::FindItemById ( u32 _Id )
{
#ifdef INLINE_DEBUG
	debug << "\r\nThere are " << ListOfMenuBarItems.size () << " menu bar items.";
#endif

	for ( int i = 0; i < ListOfMenuBarItems.size (); i++ )
	{
#ifdef INLINE_DEBUG
	debug << "\r\nTraversing: MenuBarId=" << ListOfMenuBarItems [ i ]->MenuBarId << " MenuBarItemId=" << ListOfMenuBarItems [ i ]->Id << " Caption=" << ListOfMenuBarItems [ i ]->Caption.c_str();
#endif
		if ( ListOfMenuBarItems [ i ]->MenuBarId == Id && ListOfMenuBarItems [ i ]->Id == _Id )
		{
#ifdef INLINE_DEBUG
	debug << "\r\nFound: MenuBarId=" << ListOfMenuBarItems [ i ]->MenuBarId << " MenuBarItemId=" << ListOfMenuBarItems [ i ]->Id << " Caption=" << ListOfMenuBarItems [ i ]->Caption.c_str();
#endif
			return ListOfMenuBarItems [ i ];
		}
	}
	
	return NULL;
}

WindowClass::MenuBar::MenuBarItem* WindowClass::MenuBar::FindItemByCaption ( string _Caption )
{
	for ( int i = 0; i < ListOfMenuBarItems.size (); i++ )
	{
		if ( ListOfMenuBarItems [ i ]->MenuBarId == Id && !_Caption.compare( ListOfMenuBarItems [ i ]->Caption ) )
		{
			return ListOfMenuBarItems [ i ];
		}
	}
	
	return NULL;
}

WindowClass::MenuBar::MenuBarItem* WindowClass::MenuBar::AddItem ( u32 IdToAddTo, string _Caption, WindowClass::MenuBar::Function CallbackFuncWhenClicked, u32 IdOfNewItem )
{
	MenuBarItem* m;
	m = FindItemById ( IdToAddTo );
	if ( m != NULL ) return m->AddItem ( _Caption, _Caption, CallbackFuncWhenClicked, IdOfNewItem );
	return NULL;
}

WindowClass::MenuBar::MenuBarItem* WindowClass::MenuBar::AddItem ( string CaptionToAddTo, string _Caption, WindowClass::MenuBar::Function CallbackFuncWhenClicked, u32 IdOfNewItem )
{
	MenuBarItem* m;
	m = FindItemByCaption ( CaptionToAddTo );
	if ( m != NULL ) return m->AddItem ( _Caption, _Caption, CallbackFuncWhenClicked, IdOfNewItem );
	return NULL;
}

WindowClass::MenuBar::MenuBarItem* WindowClass::MenuBar::AddMenu ( u32 IdToAddTo, string _Caption, u32 IdOfNewItem )
{
	MenuBarItem* m;
	m = FindItemById ( IdToAddTo );
	if ( m != NULL ) return m->AddMenu ( _Caption, IdOfNewItem );
	return NULL;
}

WindowClass::MenuBar::MenuBarItem* WindowClass::MenuBar::AddMenu ( string CaptionToAddTo, string _Caption, u32 IdOfNewItem )
{
	MenuBarItem* m;
	m = FindItemByCaption ( CaptionToAddTo );
	if ( m != NULL ) return m->AddMenu ( _Caption, IdOfNewItem );
	return NULL;
}




/////////////////////////
// Menu Bar Item Stuff //
/////////////////////////


WindowClass::MenuBar::MenuBarItem::MenuBarItem ( u32 _MenuBarId, u32 _ParentId, string _Caption, u32 _Id, WindowClass::MenuBar::Function _CallbackFunc )
{
	// create menu to append items to
	Menu = CreatePopupMenu();

	// set properties for menu bar item
	Caption = _Caption;
	CallbackFunc = _CallbackFunc;
	MenuBarId = _MenuBarId;
	ParentId = _ParentId;
	Id = _Id;
	
	if ( !Id )
	{
		// other threads might be using this
		Id = (u32) Lock_ExchangeAdd32 ( (long&)NextIndex, 1 );
	}
}

WindowClass::MenuBar::MenuBarItem::~MenuBarItem ()
{
	//if ( SubItems.size () > 0 )
	//{
	//	for ( int i = 0; i < SubItems.size (); i++ ) delete SubItems [ i ];
	//}
	
	//SubItems.clear ();
}

WindowClass::MenuBar::MenuBarItem* WindowClass::MenuBar::MenuBarItem::AddItem ( string _Caption, string sKey, WindowClass::MenuBar::Function _CallbackFunc, u32 _Id )
{
	//MenuBarItem* NewMenuItem = new MenuBarItem ( MenuBarId, Id, _Caption, _Id, _CallbackFunc );
	MenuBarItem* NewMenuItem = new MenuBarItem ( MenuBarId, Id, sKey, _Id, _CallbackFunc );

	// add into list of menu items incase we need to find the call back function
	ListOfMenuBarItems.push_back ( NewMenuItem );
	
	// append to menu
	AppendMenu( Menu, MF_STRING, NewMenuItem->Id, _Caption.c_str() );
	
	return NewMenuItem;
}

WindowClass::MenuBar::MenuBarItem* WindowClass::MenuBar::MenuBarItem::AddMenu ( string _Caption, u32 _Id )
{
	MenuBarItem* NewMenu = new MenuBarItem ( MenuBarId, Id, _Caption, _Id );

	// add into list of menu items in case we need to find the call back function
	ListOfMenuBarItems.push_back ( NewMenu );
	
	// append to menu
	AppendMenu(Menu, MF_STRING | MF_POPUP, (UINT64)NewMenu->Menu, _Caption.c_str());
	
	
	return NewMenu;
}



///////////////// Shortcut Key Stuff ///////////////////////////


int WindowClass::Window::NextShortcutKeyID = 0;
vector<WindowClass::Window::ShortcutKey_Entry> WindowClass::Window::ShortcutKey_Entries;

void WindowClass::Window::AddShortcutKey ( Function CallbackFunc, unsigned int key, unsigned int modifier )
{
	ShortcutKey_Entry *Entry = new ShortcutKey_Entry;

	Entry->hWnd = hWnd;
	Entry->ID = NextShortcutKeyID++;
	Entry->Key = key;
	Entry->Modifier = modifier;
	Entry->CallbackFunc = CallbackFunc;


	//////////////////////////////////////////////////////
	// Register the hot key with Windows API
	//PostMessage ( hWnd, GUI_ADDSHORTCUTKEY, (WPARAM) Entry, (LPARAM) NULL );
	_AddShortcutKey ( Entry );
	
}

void WindowClass::Window::_AddShortcutKey ( ShortcutKey_Entry *s )
{
	//////////////////////////////////////////////////////
	// Register the hot key with Windows API
	//RegisterHotKey ( s->hWnd, s->ID, s->Modifier, s->Key );
	
	///////////////////////////////////////////////////
	// add shortcut key into list of shortcut keys
	// *** todo *** don't forget to delete static vectors
	ShortcutKey_Entries.push_back ( *s );
	
	//////////////////////////////////////////////////////
	// de-allocate memory
	delete s;
}



//////////////////////////////////////////////////////////////////////
///////////////// NEW STUFF //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool WindowClass::Window::AddEvent ( HWND hParentWindow, HWND hControl, int CtrlId, unsigned int message, EventFunction Func )
{
	WindowClass::Window::Event *e = new WindowClass::Window::Event ( hParentWindow, hControl, CtrlId, message, Func );
	WindowClass::Window::EventList.push_back( e );
	return true;
}

bool WindowClass::Window::RemoveEvent ( HWND hParentWindow, long long id, unsigned int message )
{
	bool event_removed;
	vector<WindowClass::Window::Event*>::iterator i;	
	
	event_removed = false;
	
	do
	{

		for ( i = WindowClass::Window::EventList.begin(); i != WindowClass::Window::EventList.end(); i++ )
		{
			// if this menu bar is found, then delete it
			if ( ( (*i)->id == id || ( !id ) ) && ( (*i)->message == message || ( !message ) ) && (*i)->hwndParent == hParentWindow )
			{
				// object is already being deleted, so don't delete or it crashes!
				// just erase from list
				
				// de-allocate event object
				delete ( *i );

				// erase element in event list
				WindowClass::Window::EventList.erase ( i );
				
				event_removed = true;
				
				// iterator is no longer good after doing an erase
				break;
			}
		} 
		
	} while ( i != WindowClass::Window::EventList.end() );
	
	return event_removed;
}	


// this will call a function from the GUI Thread
unsigned long long WindowClass::Window::RemoteCall ( WindowClass::Window::RemoteFunction FunctionToCall, void* Parameters, bool WaitForReturnValue )
{
	int i;

#ifdef INLINE_DEBUG
	debug << "\r\n\r\nRemoteCall";
#endif

#ifdef INLINE_DEBUG
	debug << "; Marking window object as busy";
#endif

	/*
	if ( WaitForReturnValue )
	{
		// Wait for remote call to be free
		cout << "\nWaiting for remote call to finish before starting a new one.";
		while ( Busy );
		cout << "\nRemote call ready.";
	}
	*/

	// Remote call is busy and has not completed yet
	Lock_Exchange32 ( (long&)Busy, 1 );

#ifdef INLINE_DEBUG
	debug << "; Posting message to GUI thread" << dec << GUIThread->ThreadId;
#endif

	// check if we are already on the GUI Thread or not. If we are already on the GUI Thread, then we need to make a local call instead of remote
	/*
	if ( WindowClass::Window::GUIThread->ThreadId == GetCurrentThreadId () )
	{
	*/
		WindowClass::Window::LastResult = ((WindowClass::Window::RemoteFunction) FunctionToCall) ( (void*) Parameters );
		
		// remote function call is done now
		Lock_Exchange32 ( (long&)WindowClass::Window::Busy, 0 );
	/*
	}
	else
	{
		//i = PostThreadMessage ( (DWORD) GUIThread->ThreadId, GUI_REMOTECALL, (WPARAM) FunctionToCall, (LPARAM) Parameters );
		
		//cout << "\nWaiting for a free slot in buffer";
		
		// wait for a free slot in buffer
		while ( ( ( WriteIndex + 1 ) & c_RemoteCallBufferMask ) == ( ReadIndex & c_RemoteCallBufferMask ) );
		
		// set the parameters
		RemoteCall_Buffer [ WriteIndex & c_RemoteCallBufferMask ].Param = (unsigned long long) Parameters;
		RemoteCall_Buffer [ WriteIndex & c_RemoteCallBufferMask ].FunctionToCall = FunctionToCall;
		
		// update write index
		Lock_ExchangeAdd32 ( (long&)WriteIndex, 1 );
		
		//cout << "\nRemote call request sent";
	}
	*/
	
#ifdef INLINE_DEBUG
	debug << "; Waiting for remote call to finish. PostThreadMessage=" << i;
#endif

	/*
	if ( WaitForReturnValue )
	{
		// wait for remote call to finish
		//while ( Lock_Exchange32 ( (long&) Busy, 1 ) );
		//cout << "\nWaiting for remote call to finish before exiting.";
		while ( Busy );
		//cout << "\nRemote call finished.";
	}
	*/
	
#ifdef INLINE_DEBUG
	debug << "; Remote call complete";
#endif

	// no longer busy
	//Lock_Exchange32 ( (long&)Busy, 0 );
	
#ifdef INLINE_DEBUG
	debug << "; ->Returning";
#endif

	// return the result if there is one
	return LastResult;
}

// this one can be called normally
HWND WindowClass::Window::CreateControl ( const char* ClassName, int x, int y, int width, int height, const char* Caption, int flags, HMENU hMenu )
{
	HWND handle;

	_CreateControl_Params* Params = new _CreateControl_Params ( hWnd, ClassName, x, y, width, height, Caption, flags, hMenu );
	handle =  (HWND) RemoteCall ( (WindowClass::Window::RemoteFunction) _CreateControl, (void*) Params );
	
	
	delete Params;
	return handle;
}

// this one must be called from GUI thread
unsigned long long WindowClass::Window::_CreateControl ( _CreateControl_Params* p )
{
#ifdef INLINE_DEBUG
	debug << "\r\nRemoteCall: _CreateControl";
#endif

	HWND handle;
	
	handle = CreateWindow ( p->ClassName, p->Caption, p->flags, p->x, p->y, p->width, p->height, p->ParentWindow, p->hMenu, GetModuleHandle( NULL ), NULL );
	
	// we'll delete the memory used to pass the parameters here
	//delete p;
	
	return (unsigned long long) handle;
}

HWND WindowClass::Window::Create ( const char* _lpWindowName, int _x, int _y, int _nWidth, int _nHeight, DWORD _dwStyle, HMENU _hMenu, HWND _hWndParent, LPVOID _lpParam, HINSTANCE _hInstance, LPCTSTR _lpClassName )
{
	_Create_Params* Params = new _Create_Params ( this, _lpWindowName, _x, _y, _nWidth, _nHeight, _dwStyle, _hMenu, _hWndParent, _lpParam, _hInstance,
												_lpClassName );
												
	// call the function on gui thread and get the handle to the window that was created
	hWnd = (HWND) RemoteCall ( (WindowClass::Window::RemoteFunction) _Create, (void*) Params );
	
	// now create the menu bar object incase we want to utilize it
	Menus = new MenuBar ( hWnd );
	
	delete Params;
	return hWnd;
}





// this function will actually create the window on the GUI thread
unsigned long long WindowClass::Window::_Create ( _Create_Params* p )
{
#ifdef INLINE_DEBUG
	debug << "\r\nRemoteCall: _Create";
#endif

	HWND handle;
	
	handle = CreateWindow( (LPCTSTR) p->lpClassName, (LPCTSTR) p->lpWindowName, p->dwStyle, p->x, p->y, p->nWidth, p->nHeight, p->hWndParent, p->hMenu, p->hInstance, p->lpParam );
	ShowWindow( handle, SW_SHOWNORMAL );
	UpdateWindow( handle );
	
	//delete p;

	return (unsigned long long) handle;
}


// KillGLWindow and CreateGLWindow below are both ripped from http://nehe.gamedev.net/tutorial/creating_an_opengl_window_(win32)/13001/
// ripped from Nehe tutorial Lesson 1

void WindowClass::Window::KillGLWindow()								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
 
BOOL WindowClass::Window::CreateGLWindow(const char* title, int width, int height, bool has_menu, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	// my code - save window width and height
	WindowWidth = width;
	WindowHeight = height;
	
	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		EnumDisplaySettings ( NULL, ENUM_CURRENT_SETTINGS, &dmScreenSettings );
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		//dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmBitsPerPel	= 32;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;							// Windows Style
	}

	//AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size
	AdjustWindowRectEx(&WindowRect, dwStyle, has_menu, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	//if ( fullscreen ) cin.ignore ();
	
	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		24 /*bits*/,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	
	// added by TheGangster //
	// set the pixel format for the DC
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	
	
	//cin.ignore ();
	
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	
	//cin.ignore ();
	
	
	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	//ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	/*
	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}
	*/
	
	// my code - open gl should be enabled
	OpenGL_Enabled = true;
	hInst = hInstance;

	
	//cin.ignore ();
	
	
	// my code - now create the menu bar object incase we want to utilize it
	if ( has_menu )
	{
		if ( !Menus )
		{
			Menus = new MenuBar ( hWnd );
		}
		else
		{
			Menus->hWnd = hWnd;
			SetMenu ( hWnd, Menus->MainMenu );
		}
	}
	else
	{
		SetMenu ( hWnd, NULL );
	}
	
	
	//cin.ignore ();
	
	
	return TRUE;									// Success
}

/*
HWND WindowClass::Window::CreateFullscreenWindow(HWND hWnd)
{
 HMONITOR hmon = MonitorFromWindow(hWnd,
                                   MONITOR_DEFAULTTONEAREST);
 MONITORINFO mi = { sizeof(mi) };
 if (!GetMonitorInfo(hmon, &mi)) return NULL;
 return CreateWindow(TEXT("static"),
       TEXT("something interesting might go here"),
       WS_POPUP | WS_VISIBLE,
       mi.rcMonitor.left,
       mi.rcMonitor.top,
       mi.rcMonitor.right - mi.rcMonitor.left,
       mi.rcMonitor.bottom - mi.rcMonitor.top,
       hWnd, NULL, hInstance, 0);
}
*/


void WindowClass::Window::ToggleGLFullScreen ()
{
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)WindowWidth;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)WindowHeight;		// Set Bottom Value To Requested Height
	
	//OpenGL_MakeCurrentWindow ();
	//KillGLWindow ();
	fullscreen ^= 1;
	//CreateGLWindow ( "hps1x64", WindowWidth, WindowHeight, !fullscreen, fullscreen );
	
	
	//cin.ignore ();
	
	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_VISIBLE | WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_VISIBLE | WS_OVERLAPPEDWINDOW;							// Windows Style
		//dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_VISIBLE | WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;							// Windows Style
		ShowCursor(TRUE);										// Hide Mouse Pointer
	}
	
	SetWindowLong ( hWnd, GWL_STYLE, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
	SetWindowLong ( hWnd, GWL_EXSTYLE, dwExStyle );
	
	
	
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		EnumDisplaySettings ( NULL, ENUM_CURRENT_SETTINGS, &dmScreenSettings );
		dmScreenSettings.dmPelsWidth	= WindowWidth;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= WindowHeight;				// Selected Screen Height
		//dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmBitsPerPel	= 32;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return;									// Return FALSE
			}
		}
	}
	else
	{

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings( NULL, NULL )!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Windowed Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=true;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return;									// Return FALSE
			}
		}
	}

	
	if ( fullscreen )
	{
		SetWindowPos( hWnd, NULL, 0, 0, WindowWidth, WindowHeight, /* SWP_NOMOVE | */ SWP_NOZORDER | SWP_NOACTIVATE );
		SetMenu ( hWnd, NULL );
	}
	else
	{
		AdjustWindowRectEx(&WindowRect, dwStyle, true, dwExStyle);		// Adjust Window To True Requested Size
		SetWindowPos( hWnd, NULL, 0, 0, WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
		//Menus->hWnd = hWnd;
		SetMenu ( hWnd, Menus->MainMenu );
	}

	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	
	//OpenGL_ReleaseWindow ();
	
	/*
	if ( !fullscreen )
	{
		KillGLWindow ();
		CreateGLWindow ( "hps1x64", WindowWidth, WindowHeight, true, false );
		//return;
	}
	*/
}



void WindowClass::Window::OutputAllDisplayModes ()
{
DEVMODE dm = { 0 };
dm.dmSize = sizeof(dm);
for( int iModeNum = 0; EnumDisplaySettings( NULL, iModeNum, &dm ) != 0; iModeNum++ ) {
  cout << "Mode #" << iModeNum << " = " << dm.dmPelsWidth << "x" << dm.dmPelsHeight << " " << dm.dmBitsPerPel << endl;
  }
}



bool WindowClass::Window::Redraw ()
{
	bool b;
	_Redraw_Params* p = new _Redraw_Params ( hWnd, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN );

	// call the function on gui thread and get the handle to the window that was created
	b = (bool) RemoteCall ( (WindowClass::Window::RemoteFunction) _Redraw, (void*) p );
	
	delete p;
	return b;
}

unsigned long long WindowClass::Window::_Redraw ( _Redraw_Params* p )
{
#ifdef INLINE_DEBUG
	debug << "\r\nRemoteCall: _Redraw";
#endif

	return RedrawWindow( p->hWnd, NULL, NULL, p->flags );
}


bool WindowClass::Window::Destroy ()
{
	bool b;
	_Destroy_Params* p = new _Destroy_Params ( hWnd );

	// call the function on gui thread and get the handle to the window that was created
	b = (bool) RemoteCall ( (WindowClass::Window::RemoteFunction) _Destroy, (void*) p );
	
	delete p;
	return b;
}


unsigned long long WindowClass::Window::_Destroy ( _Destroy_Params* p )
{
#ifdef INLINE_DEBUG
	debug << "\r\nRemoteCall: _Destroy";
#endif

	return DestroyWindow( p->hWnd );
}


volatile int WindowClass::Window::Show_ContextMenu ( int x, int y, string ContextMenu_Options )
{
	int ReturnValue;

	//cout << "\nWindowClass::Window::Show_ContextMenu";
	
	_Show_ContextMenu_Params* Params = new _Show_ContextMenu_Params ( hWnd, x, y, ContextMenu_Options );
	ReturnValue =  (int) RemoteCall ( (WindowClass::Window::RemoteFunction) _Show_ContextMenu, (void*) Params );
	
	delete Params;
	return ReturnValue;
}


unsigned long long WindowClass::Window::_Show_ContextMenu ( _Show_ContextMenu_Params* p )
{
	static const int ID_Base = 11050;
	
	HMENU hPopup;
	string st;
	vector<string> vst;
	int i, j, ReturnValue;
	
	st = Trim ( p->st );
	vst = Split ( st, "|" );

	//cout << "\nCreatePopupMenu";
	hPopup = CreatePopupMenu();

	for ( i = 0; i < vst.size(); i++ )
	{
		//cout << "\nAppending menu item";
		AppendMenu( hPopup, MF_STRING, ID_Base + i, (LPCSTR) Trim ( vst [ i ] ).c_str() );
	}
	
	//cout << "\nAbout to track pop up menu.";

	SetForegroundWindow(p->hWnd);
	ReturnValue = TrackPopupMenu(hPopup, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, p->x, p->y, 0, p->hWnd, NULL);
	
	if ( !ReturnValue ) return -1;
	return ( ReturnValue - ID_Base );
}



// gets the size of a window including non-viewable area
bool WindowClass::Window::GetWindowSize ( long* width, long* height )
{
	RECT Rect;
	bool ret;
	
	ret = GetWindowRect( hWnd, (LPRECT) &Rect );
	
	*width = Rect.right - Rect.left;
	*height = Rect.bottom - Rect.top;
	
	return ret;
}


// gets the viewable width and height for window
bool WindowClass::Window::GetViewableArea ( long* width, long* height )
{
	RECT Rect;
	bool ret;
	
	ret = GetClientRect( hWnd, (LPRECT) &Rect );
	
	*width = Rect.right - Rect.left;
	*height = Rect.bottom - Rect.top;
	
	return ret;
}


// gets the size of the window that is needed so that the viewable area is as specified
bool WindowClass::Window::GetRequiredWindowSize ( long* width, long* height, BOOL hasMenu, long WindowStyle )
{
	RECT Rect;
	bool ret;
	
	Rect.top = 0;
	Rect.left = 0;
	Rect.bottom = *height;
	Rect.right = *width;

	ret = AdjustWindowRect( (LPRECT) &Rect, WindowStyle, hasMenu );
	
	*width = Rect.right - Rect.left;
	*height = Rect.bottom - Rect.top;
	
	return ret;
}



bool WindowClass::Window::SetWindowSize( long width, long height )
{
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height
	
	//OpenGL_MakeCurrentWindow ();
	//KillGLWindow ();
	//fullscreen ^= 1;
	//CreateGLWindow ( "hps1x64", WindowWidth, WindowHeight, !fullscreen, fullscreen );
	
	
	//cin.ignore ();
	
	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		return true;
		/*
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_VISIBLE | WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
		*/
	}
	else
	{
		//dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		//dwStyle=WS_VISIBLE | WS_OVERLAPPEDWINDOW;							// Windows Style
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;							// Windows Style
		ShowCursor(TRUE);										// Hide Mouse Pointer
	}
	
	//SetWindowLong ( hWnd, GWL_STYLE, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
	//SetWindowLong ( hWnd, GWL_EXSTYLE, dwExStyle );
	
	
	/*
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		EnumDisplaySettings ( NULL, ENUM_CURRENT_SETTINGS, &dmScreenSettings );
		dmScreenSettings.dmPelsWidth	= WindowWidth;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= WindowHeight;				// Selected Screen Height
		//dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmBitsPerPel	= 32;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}
	else
	{

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings( NULL, NULL )!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Windowed Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=true;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}
	*/

	
	if ( fullscreen )
	{
		//SetWindowPos( hWnd, NULL, 0, 0, WindowWidth, WindowHeight, /* SWP_NOMOVE | */ SWP_NOZORDER | SWP_NOACTIVATE );
		//SetMenu ( hWnd, NULL );
	}
	else
	{
		AdjustWindowRectEx(&WindowRect, dwStyle, true, dwExStyle);		// Adjust Window To True Requested Size
		SetWindowPos( hWnd, NULL, 0, 0, WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
		//Menus->hWnd = hWnd;
		SetMenu ( hWnd, Menus->MainMenu );
	}

	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	
	// set new size of window
	WindowWidth = width;
	WindowHeight = height;
	
	return true;
}


///////////////// Command Buttons //////////////////

HWND WindowClass::Button::Create_CmdButton ( WindowClass::Window* ParentWindow, int x, int y, int width, int height, const char* Caption, long long Id, int flags )
{
	Parent = ParentWindow;
	id = Id;
	hWnd = ParentWindow->CreateControl ( ClassName, x, y, width, height, Caption, flags, (HMENU) id );
	if ( Parent->hFont ) SetFont ( Parent->hFont );
	return hWnd;
}

HWND WindowClass::Button::Create_CheckBox ( WindowClass::Window* ParentWindow, int x, int y, int width, int height, const char* Caption, long long Id, int flags )
{
	Parent = ParentWindow;
	id = Id;
	hWnd = ParentWindow->CreateControl ( ClassName, x, y, width, height, Caption, flags, (HMENU) id );
	if ( Parent->hFont ) SetFont ( Parent->hFont );
	return hWnd;
}

HWND WindowClass::Button::Create_RadioButtonGroup ( WindowClass::Window* ParentWindow, int x, int y, int width, int height, const char* Caption, long long Id, int flags )
{
	Parent = ParentWindow;
	id = Id;
	hWnd = ParentWindow->CreateControl ( ClassName, x, y, width, height, Caption, flags, (HMENU) id );
	if ( Parent->hFont ) SetFont ( Parent->hFont );
	return hWnd;
}

HWND WindowClass::Button::Create_RadioButton ( WindowClass::Window* ParentWindow, int x, int y, int width, int height, const char* Caption, long long Id, int flags )
{
	Parent = ParentWindow;
	id = Id;
	hWnd = ParentWindow->CreateControl ( ClassName, x, y, width, height, Caption, flags, (HMENU) id );
	if ( Parent->hFont ) SetFont ( Parent->hFont );
	return hWnd;
}


/////////////////////// Edit Control ////////////////////////////////

HWND WindowClass::Edit::Create ( WindowClass::Window* ParentWindow, int x, int y, int width, int height, const char* Caption, long long Id, int flags )
{
	Parent = ParentWindow;
	id = Id;
	hWnd = ParentWindow->CreateControl ( ClassName, x, y, width, height, Caption, flags, (HMENU) id );
	if ( Parent->hFont ) SetFont ( Parent->hFont );
	return hWnd;
}


//////////////////// Combo Box Control //////////////////////////////

HWND WindowClass::ComboBox::Create_Simple ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption, long long Id, int flags )
{
	Parent = ParentWindow;
	id = Id;
	hWnd = ParentWindow->CreateControl ( ClassName, x, y, width, height, Caption, flags, (HMENU) id );
	if ( Parent->hFont ) SetFont ( Parent->hFont );
	return hWnd;
}

HWND WindowClass::ComboBox::Create_DropDown ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption, long long Id, int flags )
{
	Parent = ParentWindow;
	id = Id;
	hWnd = ParentWindow->CreateControl ( ClassName, x, y, width, height, Caption, flags, (HMENU) id );
	if ( Parent->hFont ) SetFont ( Parent->hFont );
	return hWnd;
}

HWND WindowClass::ComboBox::Create_DropDownList ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption, long long Id, int flags )
{
	Parent = ParentWindow;
	id = Id;
	hWnd = ParentWindow->CreateControl ( ClassName, x, y, width, height, Caption, flags, (HMENU) id );
	if ( Parent->hFont ) SetFont ( Parent->hFont );
	return hWnd;
}


//////////////////////// Static Control /////////////////////////////////////

HWND WindowClass::Static::Create_Text ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption, long long Id, int flags )
{
	Parent = ParentWindow;
	id = Id;
	hWnd = ParentWindow->CreateControl ( ClassName, x, y, width, height, Caption, flags, (HMENU) id );
	if ( Parent->hFont ) SetFont ( Parent->hFont );
	return hWnd;
}

HWND WindowClass::Static::Create_Bitmap ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption, long long Id, int flags )
{
	Parent = ParentWindow;
	id = Id;
	hWnd = ParentWindow->CreateControl ( ClassName, x, y, width, height, Caption, flags, (HMENU) id );
	return hWnd;
}


/////////////////////// List-View Control ////////////////////////////////////

HWND WindowClass::ListView::Create_wHeader ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption, long long Id, int flags )
{
	Parent = ParentWindow;
	id = Id;
	hWnd = ParentWindow->CreateControl ( WC_LISTVIEW, x, y, width, height, Caption, flags, (HMENU) id );
	if ( Parent->hFont ) SetFont ( Parent->hFont );
	return hWnd;
}

HWND WindowClass::ListView::Create_NoHeader ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption, long long Id, int flags )
{
	Parent = ParentWindow;
	id = Id;
	hWnd = ParentWindow->CreateControl ( WC_LISTVIEW, x, y, width, height, Caption, flags, (HMENU) id );
	if ( Parent->hFont ) SetFont ( Parent->hFont );
	return hWnd;
}

HWND WindowClass::ListView::Create_Dynamic_wHeader ( Window* ParentWindow, int x, int y, int width, int height, const char* Caption, long long Id, int flags )
{
	Parent = ParentWindow;
	id = Id;
	hWnd = ParentWindow->CreateControl ( WC_LISTVIEW, x, y, width, height, Caption, flags, (HMENU) id );
	if ( Parent->hFont ) SetFont ( Parent->hFont );
	return hWnd;
}


///////////////////////////// Create Font Object //////////////////////////////////////

HFONT WindowClass::Window::CreateFontObject ( int fontSize, const char* fontName, bool Bold, bool Underline, bool Italic, bool StrikeOut )
{
	HFONT hF;
	
	//PostMessage ( hWnd, GUI_CHANGEFONT, (WPARAM) fontSize, (LPARAM) new string ( fontName ) );
	_CreateFontObject_Params *p = new _CreateFontObject_Params ( fontSize, fontName, Bold, Underline, Italic, StrikeOut );
	hF = (HFONT) RemoteCall ( (WindowClass::Window::RemoteFunction) _CreateFontObject, (void*) p );
	
	delete p;
	return hF;
}

HFONT WindowClass::Window::_CreateFontObject ( _CreateFontObject_Params* p )
{
#ifdef INLINE_DEBUG
	debug << "\r\nRemoteCall: _CreateFontObject";
#endif

	long lfHeight;
	HFONT hf;
	HFONT _hFontOld;
	HDC _hDC;

	_hDC = GetDC ( NULL );
	lfHeight = -MulDiv( p->fontSize, GetDeviceCaps(_hDC, LOGPIXELSY), 72);
	ReleaseDC ( NULL, _hDC );
	
	hf = CreateFont( lfHeight, 0, 0, 0, p->Bold * FW_BOLD, p->Italic, p->Underline, p->StrikeOut, 0, 0, 0, 0, 0, p->fontName );

	//_hDC = GetDC ( _hWnd );
	//_hFontOld = (HFONT) SelectObject(_hDC, hf);
	//ReleaseDC( _hWnd, _hDC );

	//if ( _hFontOld ) DeleteObject ( _hFontOld );
	
	return hf;
}



int WindowClass::Window::Get_TextWidth ( string text, UINT uFormat )
{
	int ReturnValue;
	
	_Get_TextWidth_Params *p = new _Get_TextWidth_Params ( hWnd, text, uFormat );
	ReturnValue = (int) RemoteCall ( (WindowClass::Window::RemoteFunction) _Get_TextWidth, (void*) p );
	
	delete p;
	return ReturnValue;
}

int WindowClass::Window::Get_TextHeight ( string text, UINT uFormat )
{
	int ReturnValue;
	
	_Get_TextWidth_Params *p = new _Get_TextWidth_Params ( hWnd, text, uFormat );
	ReturnValue = (int) RemoteCall ( (WindowClass::Window::RemoteFunction) _Get_TextHeight, (void*) p );
	
	delete p;
	return ReturnValue;
}



unsigned long long WindowClass::Window::_Get_TextWidth ( _Get_TextWidth_Params* p )
{
	RECT rect;
	HDC _hDC;

	_hDC = GetDC ( p->hWnd );
	
	GetClientRect ( p->hWnd, &rect );
	rect.left = 0;
	rect.top = 0;
	DrawText( _hDC, (LPCTSTR) p->text.c_str(), -1, &rect, p->format | DT_CALCRECT );
	
	ReleaseDC ( p->hWnd, _hDC );
	
	return rect.right;
}

unsigned long long WindowClass::Window::_Get_TextHeight ( _Get_TextWidth_Params* p )
{
/*
	RECT rect;

	GetClientRect ( _hWnd, &rect );
	rect.left = 0;
	rect.top = 0;
	return (long) DrawText( _hDC, (LPCTSTR) text, -1, &rect, uFormat | DT_CALCRECT );
*/

	RECT rect;
	HDC _hDC;

	_hDC = GetDC ( p->hWnd );
	
	GetClientRect ( p->hWnd, &rect );
	rect.left = 0;
	rect.top = 0;
	DrawText( _hDC, (LPCTSTR) p->text.c_str(), -1, &rect, p->format | DT_CALCRECT );
	
	ReleaseDC ( p->hWnd, _hDC );
	
	return rect.bottom;
}



void WindowClass::Window::Set_Font ( HFONT _hFont )
{
	int ReturnValue;
	
	hFont = _hFont;
	
	_Set_Font_Params *p = new _Set_Font_Params ( hWnd, _hFont );
	ReturnValue = (int) RemoteCall ( (WindowClass::Window::RemoteFunction) _Set_Font, (void*) p );
	
	delete p;
}


unsigned long long WindowClass::Window::_Set_Font ( _Set_Font_Params* p )
{
	HDC _hDC;

	_hDC = GetDC ( p->hWnd );
	
	SelectObject(_hDC, p->hFont);
	
	ReleaseDC ( p->hWnd, _hDC );

	return true;
}





