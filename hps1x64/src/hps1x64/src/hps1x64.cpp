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




#include "hps1x64.h"
#include "WinApiHandler.h"
#include <fstream>
//#include "ConfigFile.h"
#include "StringUtils.h"

#include "guicon.h"


using namespace Playstation1;
using namespace Utilities::Strings;
//using namespace Config;



#ifdef _DEBUG_VERSION_

// debug defines go in here

#endif

#define ENABLE_DIRECT_INPUT



json jsnMenuBar = {
{ "MenuBar", { 
	{ "File", {
		{ "Caption", {
			{ "English", "File" }
		} },
		{ "SubMenu", {
			{ "Load", {
				{ "Caption", {
					{ "English", "Load" }
				} },
				{ "SubMenu", {
					{ "Bios", {
						{ "Caption", {
							{ "English", "Bios" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_File_Load_BIOS }
					} },	// end File | Load | Bios
					{ "State", {
						{ "Caption", {
							{ "English", "State" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_File_Load_State }
					} },	// end File | Load | State
					{ "Insert/Remove Game Disk", {
						{ "Caption", {
							{ "English", "Insert/Remove Game Disk" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_File_Load_GameDisk }
					} },	// end File | Load | Game Disk
					{ "Insert/Remove Audio Disk", {
						{ "Caption", {
							{ "English", "Insert/Remove Audio Disk" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_File_Load_AudioDisk }
					} },	// end File | Load | Audio Disk
				} }	// end File | Load | SubMenu
			} },	// end File | Load
			{ "Save", {
				{ "Caption", {
					{ "English", "Save" }
				} },
				{ "SubMenu", {
					{ "State", {
						{ "Caption", {
							{ "English", "State" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_File_Save_State }
					} }	// end File | Save | State
				} }	// end File | Save | SubMenu
			} },
			{ "Run", {
				{ "Caption", {
					{ "English", "Run" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_File_Run },
				{ "ShortcutKey", {
					{ "Key", 0x52 },
					{ "Modifier", 0x11 },
				} },
			} },
			{ "Reset", {
				{ "Caption", {
					{ "English", "Reset" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_File_Reset }
			} },
			{ "Exit", {
				{ "Caption", {
					{ "English", "Exit" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_File_Exit }
			} }	// end File | Exit

		} }	// end File | Submenu

	} },	// end File
	{ "Debug", {
		{ "Caption", {
			{ "English", "Debug" }
		} },
		{ "SubMenu", {
			{ "Break", {
				{ "Caption", {
					{ "English", "Break" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_Debug_Break }
			} },
			{ "Step Into", {
				{ "Caption", {
					{ "English", "Step Into" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_Debug_StepInto },
				{ "ShortcutKey", {
					{ "Key", 0x41 },
					{ "Modifier", 0 },
				} },
			} },
			{ "Output Current Sector", {
				{ "Caption", {
					{ "English", "Output Current Sector" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_Debug_OutputCurrentSector }
			} },
			{ "Show Window", {
				{ "Caption", {
					{ "English", "Show Window" }
				} },
				{ "SubMenu", {
					{ "All", {
						{ "Caption", {
							{ "English", "All" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Debug_Show_All }
					} },
					{ "Frame Buffer", {
						{ "Caption", {
							{ "English", "Frame Buffer" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Debug_Show_FrameBuffer }
					} },
					{ "R3000A", {
						{ "Caption", {
							{ "English", "R3000A" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Debug_Show_R3000A }
					} },
					{ "Memory", {
						{ "Caption", {
							{ "English", "Memory" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Debug_Show_Memory }
					} },
					{ "DMA", {
						{ "Caption", {
							{ "English", "DMA" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Debug_Show_DMA }
					} },
					{ "Timers", {
						{ "Caption", {
							{ "English", "Timers" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Debug_Show_TIMER }
					} },
					{ "SPU", {
						{ "Caption", {
							{ "English", "SPU" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Debug_Show_SPU }
					} },
					{ "INTC", {
						{ "Caption", {
							{ "English", "INTC" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Debug_Show_INTC }
					} },
					{ "PS1 GPU", {
						{ "Caption", {
							{ "English", "PS1 GPU" }
						} },
						//{ "Function", (unsigned long long) NULL }
					} },
					{ "INTC", {
						{ "Caption", {
							{ "English", "INTC" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Debug_Show_INTC }
					} },
					{ "MDEC", {
						{ "Caption", {
							{ "English", "MDEC" }
						} },
					} },
					{ "SIO", {
						{ "Caption", {
							{ "English", "SIO" }
						} },
					} },
					{ "PIO", {
						{ "Caption", {
							{ "English", "PIO" }
						} },
					} },
					{ "CD", {
						{ "Caption", {
							{ "English", "CD" }
						} },
					} },
					{ "Bus", {
						{ "Caption", {
							{ "English", "Bus" }
						} },
					} },
					{ "I-Cache", {
						{ "Caption", {
							{ "English", "I-Cache" }
						} },
					} },
				} },	// end File | Save | SubMenu
			} },

		} },
	} },
	{ "Peripherals", {
		{ "Caption", {
			{ "English", "Peripherals" }
		} },
		{ "SubMenu", {
			{ "Pad 1", {
				{ "Caption", {
					{ "English", "Pad 1" }
				} },
				{ "SubMenu", {
					{ "Configure Joypad 1...", {
						{ "Caption", {
							{ "English", "Configure Joypad 1..." }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Controllers0_Configure }
					} },
					{ "Pad 1 Type", {
						{ "Caption", {
							{ "English", "Pad 1 Type" }
						} },
						{ "SubMenu", {
							{ "Pad 1 Digital", {
								{ "Caption", {
									{ "English", "Pad 1 Digital" }
								} },
								{ "Function", (unsigned long long) hps1x64::OnClick_Pad1Type_Digital },
							} },
							{ "Pad 1 Analog", {
								{ "Caption", {
									{ "English", "Pad 1 Analog" }
								} },
								{ "Function", (unsigned long long) hps1x64::OnClick_Pad1Type_Analog },
							} },
						} },
					} },
					{ "Pad 1: Input", {
						{ "Caption", {
							{ "English", "Pad 1: Input" }
						} },
						{ "SubMenu", {
							{ "Pad 1: None", {
								{ "Caption", {
									{ "English", "Pad 1: None" }
								} },
								{ "Function", (unsigned long long) hps1x64::OnClick_Pad1Input_None },
							} },
							{ "Pad 1: Device0", {
								{ "Caption", {
									{ "English", "Pad 1: Device0" }
								} },
								{ "Function", (unsigned long long) hps1x64::OnClick_Pad1Input_Device0 },
							} },
							{ "Pad 1: Device1", {
								{ "Caption", {
									{ "English", "Pad 1: Device1" }
								} },
								{ "Function", (unsigned long long) hps1x64::OnClick_Pad1Input_Device1 },
							} },
						} },
					} },
				} },
			} },	// end Pad 1
			{ "Pad 2", {
				{ "Caption", {
					{ "English", "Pad 2" }
				} },
				{ "SubMenu", {
					{ "Configure Joypad 2...", {
						{ "Caption", {
							{ "English", "Configure Joypad 2..." }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Controllers1_Configure }
					} },
					{ "Pad 2 Type", {
						{ "Caption", {
							{ "English", "Pad 2 Type" }
						} },
						{ "SubMenu", {
							{ "Pad 2 Digital", {
								{ "Caption", {
									{ "English", "Pad 2 Digital" }
								} },
								{ "Function", (unsigned long long) hps1x64::OnClick_Pad2Type_Digital },
							} },
							{ "Pad 2 Analog", {
								{ "Caption", {
									{ "English", "Pad 2 Analog" }
								} },
								{ "Function", (unsigned long long) hps1x64::OnClick_Pad2Type_Analog },
							} },
						} },
					} },
					{ "Pad 2: Input", {
						{ "Caption", {
							{ "English", "Pad 2: Input" }
						} },
						{ "SubMenu", {
							{ "Pad 2: None", {
								{ "Caption", {
									{ "English", "Pad 2: None" }
								} },
								{ "Function", (unsigned long long) hps1x64::OnClick_Pad2Input_None },
							} },
							{ "Pad 2: Device0", {
								{ "Caption", {
									{ "English", "Pad 2: Device0" }
								} },
								{ "Function", (unsigned long long) hps1x64::OnClick_Pad2Input_Device0 },
							} },
							{ "Pad 2: Device1", {
								{ "Caption", {
									{ "English", "Pad 2: Device1" }
								} },
								{ "Function", (unsigned long long) hps1x64::OnClick_Pad2Input_Device1 },
							} },
						} },
					} },
				} },
			} },	// end Pad 2
			{ "Memory Cards", {
				{ "Caption", {
					{ "English", "Memory Cards" }
				} },
				{ "SubMenu", {
					{ "Card 1", {
						{ "Caption", {
							{ "English", "Card 1" }
						} },
						{ "SubMenu", {
							{ "Connect Card1", {
								{ "Caption", {
									{ "English", "Connect Card1" },
									{ "Function", (unsigned long long) hps1x64::OnClick_Card1_Connect },
								} },
							} },
							{ "Disconnect Card1", {
								{ "Caption", {
									{ "English", "Disconnect Card1" },
									{ "Function", (unsigned long long) hps1x64::OnClick_Card1_Disconnect },
								} },
							} },
						} },
					} },	// end Card 1
					{ "Card 2", {
						{ "Caption", {
							{ "English", "Card 2" }
						} },
						{ "SubMenu", {
							{ "Connect Card2", {
								{ "Caption", {
									{ "English", "Connect Card2" },
									{ "Function", (unsigned long long) hps1x64::OnClick_Card2_Connect },
								} },
							} },
							{ "Disconnect Card2", {
								{ "Caption", {
									{ "English", "Disconnect Card2" },
									{ "Function", (unsigned long long) hps1x64::OnClick_Card2_Disconnect },
								} },
							} },
						} },
					} },
				} },
			} },
			{ "Re-Detect Joypad(s)", {
				{ "Caption", {
					{ "English", "Re-Detect Joypad(s)" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_Redetect_Pads },
			} },
		} },
	} },
	{ "Region", {
		{ "Caption", {
			{ "English", "Region" }
		} },
		{ "SubMenu", {
			{ "Europe", {
				{ "Caption", {
					{ "English", "Europe" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_Region_Europe }
			} },
			{ "Japan", {
				{ "Caption", {
					{ "English", "Japan" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_Region_Japan }
			} },
			{ "North America", {
				{ "Caption", {
					{ "English", "North America" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_Region_NorthAmerica }
			} },
		} },	// end Region | SubMenu
	} },	// end Region
	{ "Audio", {
		{ "Caption", {
			{ "English", "Audio" }
		} },
		{ "SubMenu", {
			{ "Enable", {
				{ "Caption", {
					{ "English", "Enable" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_Audio_Enable } }
			},
			{ "Volume", {
				{ "Caption", {
					{ "English", "Volume" }
				} },
				{ "SubMenu", {
					{ "100%", {
						{ "Caption", {
							{ "English", "100%" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Audio_Volume_100 }
					} },
					{ "75%", {
						{ "Caption", {
							{ "English", "75%" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Audio_Volume_75 }
					} },
					{ "50%", {
						{ "Caption", {
							{ "English", "50%" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Audio_Volume_50 }
					} },
					{ "25%", {
						{ "Caption", {
							{ "English", "25%" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Audio_Volume_25 }
					} },
				} },
			} },
			{ "Buffer Size", {
				{ "Caption", {
					{ "English", "Buffer Size" }
				} },
				{ "SubMenu", {
					{ "8 KB", {
						{ "Caption", {
							{ "English", "8 KB" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Audio_Buffer_8k }
					} },
					{ "16 KB", {
						{ "Caption", {
							{ "English", "16 KB" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Audio_Buffer_16k }
					} },
					{ "32 KB", {
						{ "Caption", {
							{ "English", "32 KB" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Audio_Buffer_32k }
					} },
					{ "64 KB", {
						{ "Caption", {
							{ "English", "64 KB" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Audio_Buffer_64k }
					} },
					{ "128 KB", {
						{ "Caption", {
							{ "English", "128 KB" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Audio_Buffer_1m }
					} },
				} },
			} },
			{ "Filter", {
				{ "Caption", {
					{ "English", "Filter" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_Audio_Filter }
			} },
		} },
	} },	// end Audio
	{ "Video", {
		{ "Caption", {
			{ "English", "Video" }
		} },
		{ "SubMenu", {
			{ "Scanlines", {
				{ "Caption", {
					{ "English", "Scanlines" }
				} },
				{ "SubMenu", {
					{ "Enable Scanlines", {
						{ "Caption", {
							{ "English", "Enable Scanlines" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Video_ScanlinesEnable }
					} },
					{ "Disable Scanlines", {
						{ "Caption", {
							{ "English", "Disable Scanlines" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_Video_ScanlinesDisable }
					} },
				} },
			} },
			{ "Window Size x1", {
				{ "Caption", {
					{ "English", "Window Size x1" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_Video_WindowSizeX1 }
			} },
			{ "Window Size x1.5", {
				{ "Caption", {
					{ "English", "Window Size x1.5" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_Video_WindowSizeX15 }
			} },
			{ "Window Size x2", {
				{ "Caption", {
					{ "English", "Window Size x2" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_Video_WindowSizeX2 }
			} },
			{ "Full Screen", {
				{ "Caption", {
					{ "English", "Full Screen" }
				} },
				{ "Function", (unsigned long long) hps1x64::OnClick_Video_FullScreen },
				{ "ShortcutKey", {
					{ "Key", 0x46 },
					{ "Modifier", 0x11 },
				} },
			} },
		} },
	} },
	{ "CPU", {
		{ "Caption", {
			{ "English", "CPU" }
		} },
		{ "SubMenu", {
			{ "CPU: R3000A", {
				{ "Caption", {
					{ "English", "CPU: R3000A" }
				} },
				{ "SubMenu", {
					{ "Interpreter: R3000A", {
						{ "Caption", {
							{ "English", "Interpreter: R3000A" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_R3000ACPU_Interpreter }
					} },
					{ "Recompiler: R3000A", {
						{ "Caption", {
							{ "English", "Recompiler: R3000A" }
						} },
						{ "Function", (unsigned long long) hps1x64::OnClick_R3000ACPU_Recompiler }
					} },
				} },
			} },
		} },
	} },

} }	// end MenuBar
};




hps1x64 _HPS1X64;





volatile u32 hps1x64::_MenuClick;
volatile hps1x64::RunMode hps1x64::_RunMode;

WindowClass::Window *hps1x64::ProgramWindow;

// the path for the last bios that was selected
string hps1x64::sLastBiosPath;

string hps1x64::BiosPath;
string hps1x64::ExecutablePath;
char ExePathTemp [ hps1x64::c_iExeMaxPathLength + 1 ];

/*
std::basic_ostream<char, std::char_traits<char>>& operator<<(std::basic_ostream<char, std::char_traits<char>>& os, const char* str)
{
	OutputDebugStringA(str);
	return os;
}

std::basic_ostream<char, std::char_traits<char>>& endl(std::basic_ostream<char, std::char_traits<char>>& os)
{
	OutputDebugStringA("\n");
	return os;
}
*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{

#if !defined(__GNUC__)
	// don't need this for gcc
	RedirectIOToConsole();
#endif


	WindowClass::Register ( hInstance, "testSystem" );
	
	cout << "Initializing program...\n";
	
	// ???
	//DisableProcessWindowsGhosting();
	//ShowCursor ( false );
	
	_HPS1X64.InitializeProgram ();

	// initialize direct input joysticks here for now
	SIO::DJoy.Init ( hps1x64::ProgramWindow->hWnd, hInstance );
	
	cout << "Starting run of program...\n";
	
	_HPS1X64.RunProgram ();
	
	
	//cin.ignore ();
	
	return 0;
}


hps1x64::hps1x64 ()
{
	cout << "Running hps1x64 constructor...\n";
	
	
	// zero object
	// *** PROBLEM *** this clears out all the defaults for the system
	//memset ( this, 0, sizeof( hps1x64 ) );
}


hps1x64::~hps1x64 ()
{
	cout << "Running hps1x64 destructor...\n";
	
	// end the timer resolution
	if ( timeEndPeriod ( 1 ) == TIMERR_NOCANDO )
	{
		cout << "\nhpsx64 ERROR: Problem ending timer period.\n";
	}
}

void hps1x64::Reset ()
{
	_RunMode.Value = 0;
	
	_SYSTEM.Reset ();
}





// returns 0 if menu was not clicked, returns 1 if menu was clicked
int hps1x64::HandleMenuClick ()
{
	int i;
	int MenuWasClicked = 0;
	
	//if ( _MenuClick.Value )
	if ( _MenuClick )
	{
		cout << "\nA menu item was clicked.\n";

		_MenuClick = 0;
		
		// a menu item was clicked
		MenuWasClicked = 1;
		
		
		// update anything that was checked/unchecked
		Update_CheckMarksOnMenu ();
		
		// clear anything that was clicked
		//x64ThreadSafe::Utilities::Lock_Exchange64 ( (long long&)_MenuClick.Value, 0 );
		
		DebugWindow_Update ();
	}
	
	return MenuWasClicked;
}


void hps1x64::Update_CheckMarksOnMenu ()
{
	// uncheck all first
	ProgramWindow->Menus->UnCheckItem("Bios");
	ProgramWindow->Menus->UnCheckItem ( "Insert/Remove Game Disk" );
	ProgramWindow->Menus->UnCheckItem ( "Pad 1 Digital" );
	ProgramWindow->Menus->UnCheckItem ( "Pad 1 Analog" );
	ProgramWindow->Menus->UnCheckItem ( "Pad 1: None" );
	ProgramWindow->Menus->UnCheckItem ( "Pad 1: Device0" );
	ProgramWindow->Menus->UnCheckItem ( "Pad 1: Device1" );
	ProgramWindow->Menus->UnCheckItem ( "Pad 2 Digital" );
	ProgramWindow->Menus->UnCheckItem ( "Pad 2 Analog" );
	ProgramWindow->Menus->UnCheckItem ( "Pad 2: None" );
	ProgramWindow->Menus->UnCheckItem ( "Pad 2: Device0" );
	ProgramWindow->Menus->UnCheckItem ( "Pad 2: Device1" );
	ProgramWindow->Menus->UnCheckItem ( "Disconnect Card1" );
	ProgramWindow->Menus->UnCheckItem ( "Connect Card1" );
	ProgramWindow->Menus->UnCheckItem ( "Disconnect Card2" );
	ProgramWindow->Menus->UnCheckItem ( "Connect Card2" );
	ProgramWindow->Menus->UnCheckItem ( "North America" );
	ProgramWindow->Menus->UnCheckItem ( "Europe" );
	ProgramWindow->Menus->UnCheckItem ( "Japan" );
	ProgramWindow->Menus->UnCheckItem ( "Enable" );
	ProgramWindow->Menus->UnCheckItem ( "100%" );
	ProgramWindow->Menus->UnCheckItem ( "75%" );
	ProgramWindow->Menus->UnCheckItem ( "50%" );
	ProgramWindow->Menus->UnCheckItem ( "25%" );
	ProgramWindow->Menus->UnCheckItem ( "8 KB" );
	ProgramWindow->Menus->UnCheckItem ( "16 KB" );
	ProgramWindow->Menus->UnCheckItem ( "32 KB" );
	ProgramWindow->Menus->UnCheckItem ( "64 KB" );
	ProgramWindow->Menus->UnCheckItem ( "128 KB" );
	ProgramWindow->Menus->UnCheckItem ( "Filter" );
	ProgramWindow->Menus->UnCheckItem ( "Enable Scanlines" );
	ProgramWindow->Menus->UnCheckItem ( "Disable Scanlines" );
	ProgramWindow->Menus->UnCheckItem ( "Interpreter: R3000A" );
	ProgramWindow->Menus->UnCheckItem ( "Recompiler: R3000A" );
#ifdef ENABLE_RECOMPILE2
	ProgramWindow->Menus->UnCheckItem ( "Recompiler2: R3000A" );
#endif
	ProgramWindow->Menus->UnCheckItem ( "1 (multi-thread)" );
	ProgramWindow->Menus->UnCheckItem ( "0 (single-thread)" );

	ProgramWindow->Menus->UnCheckItem ( "Renderer: Software" );
	ProgramWindow->Menus->UnCheckItem ( "Renderer: Hardware" );

	// check if a bios is loaded into memory or not
	if (sLastBiosPath.compare(""))
	{
		// looks like there is a bios currently loaded into memory //

		// add a tick mark next to File | Load | Bios
		ProgramWindow->Menus->CheckItem("Bios");
	}

	// check box for audio output enable //
	if ( _SYSTEM._SPU.AudioOutput_Enabled )
	{
		ProgramWindow->Menus->CheckItem ( "Enable" );
	}
	
	// check box for if disk is loaded and whether data/audio //
	
	if ( !_SYSTEM._CD.isLidOpen )
	{
		switch ( _SYSTEM._CD.isGameCD )
		{
			case true:
				ProgramWindow->Menus->CheckItem ( "Insert/Remove Game Disk" );
				break;
			
			case false:
				ProgramWindow->Menus->CheckItem ( "Insert/Remove Audio Disk" );
				break;
		}
	}
	
	// check box for analog/digital pad 1/2 //
	
	// do pad 1
	switch ( _SYSTEM._SIO.ControlPad_Type [ 0 ] )
	{
		case 0:
			ProgramWindow->Menus->CheckItem ( "Pad 1 Digital" );
			break;
			
		case 1:
			ProgramWindow->Menus->CheckItem ( "Pad 1 Analog" );
			break;
	}

	switch ( _SYSTEM._SIO.PortMapping [ 0 ] )
	{
		case 0:
			ProgramWindow->Menus->CheckItem ( "Pad 1: Device0" );
			break;
			
		case 1:
			ProgramWindow->Menus->CheckItem ( "Pad 1: Device1" );
			break;
			
		default:
			ProgramWindow->Menus->CheckItem ( "Pad 1: None" );
			break;
	}
	
	// do pad 2
	switch ( _SYSTEM._SIO.ControlPad_Type [ 1 ] )
	{
		case 0:
			ProgramWindow->Menus->CheckItem ( "Pad 2 Digital" );
			break;
			
		case 1:
			ProgramWindow->Menus->CheckItem ( "Pad 2 Analog" );
			break;
	}

	switch ( _SYSTEM._SIO.PortMapping [ 1 ] )
	{
		case 0:
			ProgramWindow->Menus->CheckItem ( "Pad 2: Device0" );
			break;
			
		case 1:
			ProgramWindow->Menus->CheckItem ( "Pad 2: Device1" );
			break;
			
		default:
			ProgramWindow->Menus->CheckItem ( "Pad 2: None" );
			break;
	}
	
	
	// check box for memory card 1/2 connected/disconnected //
	
	// do card 1
	switch ( _SYSTEM._SIO.MemoryCard_ConnectionState [ 0 ] )
	{
		case 0:
			ProgramWindow->Menus->CheckItem ( "Connect Card1" );
			break;
			
		case 1:
			ProgramWindow->Menus->CheckItem ( "Disconnect Card1" );
			break;
	}
	
	// do card 2
	switch ( _SYSTEM._SIO.MemoryCard_ConnectionState [ 1 ] )
	{
		case 0:
			ProgramWindow->Menus->CheckItem ( "Connect Card2" );
			break;
			
		case 1:
			ProgramWindow->Menus->CheckItem ( "Disconnect Card2" );
			break;
	}
	
	
	// check box for region //
	switch ( _SYSTEM._CD.Region )
	{
		case 'A':
			ProgramWindow->Menus->CheckItem ( "North America" );
			break;
			
		case 'E':
			ProgramWindow->Menus->CheckItem ( "Europe" );
			break;
			
		case 'I':
			ProgramWindow->Menus->CheckItem ( "Japan" );
			break;
	}
	
	// check box for audio buffer size //
	switch ( _SYSTEM._SPU.NextPlayBuffer_Size )
	{
		case 8192:
			ProgramWindow->Menus->CheckItem ( "8 KB" );
			break;
			
		case 16384:
			ProgramWindow->Menus->CheckItem ( "16 KB" );
			break;
			
		case 32768:
			ProgramWindow->Menus->CheckItem ( "32 KB" );
			break;
			
		case 65536:
			ProgramWindow->Menus->CheckItem ( "64 KB" );
			break;
			
		case 131072:
			ProgramWindow->Menus->CheckItem ( "128 KB" );
			break;
	}
	
	// check box for audio volume //
	switch ( _SYSTEM._SPU.GlobalVolume )
	{
		case 0x400:
			ProgramWindow->Menus->CheckItem ( "25%" );
			break;
			
		case 0x1000:
			ProgramWindow->Menus->CheckItem ( "50%" );
			break;
			
		case 0x3000:
			ProgramWindow->Menus->CheckItem ( "75%" );
			break;
			
		case 0x7fff:
			ProgramWindow->Menus->CheckItem ( "100%" );
			break;
	}
	
	// audio filter enable/disable //
	if ( _SYSTEM._SPU.AudioFilter_Enabled )
	{
		ProgramWindow->Menus->CheckItem ( "Filter" );
	}
	
	// scanlines enable/disable //
	if ( _SYSTEM._GPU.Get_Scanline () )
	{
		ProgramWindow->Menus->CheckItem ( "Enable Scanlines" );
	}
	else
	{
		ProgramWindow->Menus->CheckItem ( "Disable Scanlines" );
	}
	
	// R3000A Interpreter/Recompiler //
	if ( _SYSTEM._CPU.bEnableRecompiler )
	{
		if ( _SYSTEM._CPU.rs->OptimizeLevel == 1 )
		{
			ProgramWindow->Menus->CheckItem ( "Recompiler: R3000A" );
		}
		else
		{
			ProgramWindow->Menus->CheckItem ( "Recompiler2: R3000A" );
		}
	}
	else
	{
		ProgramWindow->Menus->CheckItem ( "Interpreter: R3000A" );
	}
	
	
#ifdef ALLOW_PS1_MULTITHREAD
	if ( _SYSTEM._GPU.ulNumberOfThreads )
	{
		ProgramWindow->Menus->CheckItem ( "1 (multi-thread)" );
	}
	else
	{
		ProgramWindow->Menus->CheckItem ( "0 (single-thread)" );
	}
#endif


#ifdef ALLOW_PS1_HWRENDER
	if ( _SYSTEM._GPU.bEnable_OpenCL )
	{
		ProgramWindow->Menus->CheckItem ( "Renderer: Hardware" );
	}
	else
	{
		ProgramWindow->Menus->CheckItem ( "Renderer: Software" );
	}
#endif

}


int hps1x64::InitializeProgram ()
{
	static constexpr char* ProgramWindow_Caption = "hps1x64";

	u32 xsize, ysize;

	////////////////////////////////////////////////
	// create program window
	xsize = ProgramWindow_Width;
	ysize = ProgramWindow_Height;
	ProgramWindow = new WindowClass::Window ();
	
	
	cout << "\nCreating window";
	
	//ProgramWindow->CreateGLWindow ( ProgramWindow_Caption, ProgramWindow_X, ProgramWindow_Y, xsize, ysize, true, false );
	ProgramWindow->CreateGLWindow ( ProgramWindow_Caption, xsize, ysize, true, false );
	
	ProgramWindow->OutputAllDisplayModes ();
	
	cout << "\nAdding menubar";
		
	////////////////////////////////////////////
	// add menu bar to program window
	WindowClass::MenuBar *m = ProgramWindow->Menus;

	ProgramWindow->CreateMenuFromJson( jsnMenuBar, "English" );

	
	cout << "\nShowing menu bar";
	
	// show the menu bar
	m->Show ();
	
	/*
	cout << "\nAdding shortcut keys";
	
	// need a shortcut key for "step into"
	ProgramWindow->AddShortcutKey ( OnClick_Debug_StepInto, 0x41 );
	
	// need a shortcut key for "run"
	ProgramWindow->AddShortcutKey ( OnClick_File_Run, 0x52 );
	
	// need a shortcut key to toggle full screen
	ProgramWindow->AddShortcutKey ( OnClick_Video_FullScreen, 0x46 );
	ProgramWindow->AddShortcutKey ( OnClick_Video_FullScreen, 0x1b );
	*/
	
	cout << "\nInitializing open gl for program window";

	/////////////////////////////////////////////////////////
	// enable opengl for the program window
	//ProgramWindow->EnableOpenGL ();
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, ProgramWindow_Width, ProgramWindow_Height, 0, 0, 1);
	glMatrixMode (GL_MODELVIEW);
	
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);
	
	cout << "\nReleasing window from OpenGL";
	
	// this window is no longer the window we want to draw to
	ProgramWindow->OpenGL_ReleaseWindow ();
	
	cout << "\nEnabling VSync";
	
	// enable/disable vsync
	ProgramWindow->EnableVSync ();
	//ProgramWindow->DisableVSync ();
	
	
	
	// set the timer resolution
	if ( timeBeginPeriod ( 1 ) == TIMERR_NOCANDO )
	{
		cout << "\nhpsx64 ERROR: Problem setting timer period.\n";
	}


	// we want the screen to display on the main window for the program when the system encouters start of vertical blank
	_SYSTEM._GPU.SetDisplayOutputWindow ( ProgramWindow_Width, ProgramWindow_Height, ProgramWindow );

	
	// start system - must do this here rather than in constructor
	_SYSTEM.Start ();


	
	// get executable path
	int len = GetModuleFileName ( NULL, ExePathTemp, c_iExeMaxPathLength );
	ExePathTemp [ len ] = 0;
	
	// remove program name from path
	ExecutablePath = Left ( ExePathTemp, InStrRev ( ExePathTemp, "\\" ) + 1 );

	
	//cout << "\nExecutable Path=" << ExecutablePath.c_str();
	
	cout << "\nLoading memory cards if available...";
	
	_SYSTEM._SIO.Load_MemoryCardFile ( ExecutablePath + "card0", 0 );
	_SYSTEM._SIO.Load_MemoryCardFile ( ExecutablePath + "card1", 1 );
	
	
	cout << "\nLoading application-level config file...";
	
	// load current configuration settings
	// config settings that are needed:
	// 1. Region
	// 2. Audio - Enable, Volume, Buffer Size, Filter On/Off
	// 3. Peripherals - Pad1/Pad2/PadX keys, Pad1/Pad2/PadX Analog/Digital, Card1/Card2/CardX Connected/Disconnected
	// I like this one... a ".hcfg" file
	LoadConfig ( ExecutablePath + "hps1x64.hcfg" );
	
	
	cout << "\nUpdating check marks";
	
	// update what items are checked or not
	Update_CheckMarksOnMenu ();
	
	
	
	
	cout << "\ndone initializing";
	
	// done
	return 1;
}





int hps1x64::RunProgram ()
{
	unsigned long long i, j;
	
	long xsize, ysize;

	int k;
	
	// the frame counter is 32-bits
	u32 LastFrameNumber;
	volatile u32 *pCurrentFrameNumber;
	
	bool bRunningTooSlow;
	
	s64 MilliSecsToWait;
	
	u64 TicksPerSec, CurrentTimer, TargetTimer;
	s64 TicksLeft;
	double dTicksPerMilliSec;
	
	cout << "\nRunning program";
	
	// get ticks per second for the platform's high-resolution timer
	if ( !QueryPerformanceFrequency ( (LARGE_INTEGER*) &TicksPerSec ) )
	{
		cout << "\nhpsx64 error: Error returned from call to QueryPerformanceFrequency.\n";
	}
	
	// calculate the ticks per milli second
	dTicksPerMilliSec = ( (double) TicksPerSec ) / 1000.0L;
	
	// get a pointer to the current frame number
	pCurrentFrameNumber = (volatile u32*) & _SYSTEM._GPU.Frame_Count;
	
	cout << "\nWaiting for command\n";
	
	// wait for command
	while ( 1 )
	{
		Sleep ( 250 );
		
		// process events
		WindowClass::DoEvents ();

		HandleMenuClick ();
		
		if ( _RunMode.Exit ) break;

		// check if there is any debugging going on
		// this mode is only required if there are breakpoints set
		if ( _RunMode.RunDebug )
		{
			cout << "Running program in debug mode...\n";
			
			ProgramWindow->SetCaption ( "hps1x64" );
			
			while ( _RunMode.RunDebug )
			{
				
				for ( j = 0; j < 60; j++ )
				{

					//while ( _SYSTEM._CPU.CycleCount < _SYSTEM.NextExit_Cycle )
					for ( i = 0; i < CyclesToRunContinuous; i++ )
					{
						// run playstation 1 system in regular mode for at least one cycle
						_SYSTEM.Run ();
						
						// check if any breakpoints were hit
						if ( _SYSTEM._CPU.Breakpoints->Check_IfBreakPointReached () >= 0 ) break;
					}
					
					//cout << "\nSystem is running (debug). " << dec << _SYSTEM._CPU.CycleCount;
					
					// update next to exit loop at
					//_SYSTEM.NextExit_Cycle = _SYSTEM._CPU.CycleCount + CyclesToRunContinuous;
					
					// process events
					WindowClass::DoEventsNoWait ();
					
					// if menu has been clicked then wait
					WindowClass::Window::WaitForModalMenuLoop ();
					
					//k = _SYSTEM._CPU.Breakpoints->Check_IfBreakPointReached ();
					if ( _SYSTEM._CPU.Breakpoints->Get_LastBreakPoint () >= 0 )
					{
						cout << "\nbreakpoint hit";
						_RunMode.Value = 0;
						//_SYSTEM._CPU.Breakpoints->Set_LastBreakPoint ( k );
						break;
					}
					
					// if menu was clicked, hop out of loop
					if ( HandleMenuClick () ) break;
				
				}

				// update all the debug info windows that are showing
				DebugWindow_Update ();
				
				if ( !_RunMode.RunDebug )
				{
					cout << "\n_RunMode.Value=" << _RunMode.Value;
					//cout << "\nk=" << k;
					cout << "\nWaiting for command\n";
				}
			}
		}

		// run program normally and without debugging
		if ( _RunMode.RunNormal )
		{
			u64 ullPerfStart_Timer;
			u64 ullPerfEnd_Timer;
			u32 ulFramesPerSec = 60;
			
			cout << "Running program...\n";
			
			ProgramWindow->SetCaption ( "hps1x64" );
			
			// this actually needs to loop until a frame is drawn by the core simulator... and then it should return the drawn frame + its size...
			// so the actual platform it is running on can then draw it
			
			// get the ticks per second for the timer
			
			// get the start timer value for the run
			if ( !QueryPerformanceCounter ( (LARGE_INTEGER*) &TargetTimer ) )
			{
				cout << "\nhpsx64: Error returned from QueryPerformanceCounter\n";
			}
			
			// the target starts equal to the start
			//SystemTimer_Target = SystemTimer_Start;
			
			// multi-threading testing
			GPU::Start_Frame ();
			
			while ( _RunMode.RunNormal )
			{
				QueryPerformanceCounter ( (LARGE_INTEGER*) &ullPerfStart_Timer );
				
				//for ( j = 0; j < 60; j++ )
				for ( j = 0; j < ulFramesPerSec; j++ )
				{
					// get the last frame number
					LastFrameNumber = *pCurrentFrameNumber;
					
//cout << "In Looping program... LastFrameNumber=" << dec << LastFrameNumber;

					// multi-threading testing
					//GPU::Start_Frame ();

					// loop until we reach the next frame
					//while ( _SYSTEM._CPU.CycleCount < _SYSTEM.NextExit_Cycle )
					//for ( i = 0; i < CyclesToRunContinuous; i++ )
					while ( LastFrameNumber == ( *pCurrentFrameNumber ) )
					{
						// run playstation 1 system in regular mode for one cpu instruction
						_SYSTEM.Run ();
					}
					
					// multi-threading testing
					//GPU::End_Frame ();
					
//cout << "Out Looping program... LastFrameNumber=" << dec << ( *pCurrentFrameNumber );

					// get the target platform timer value for this frame
					// check if this is ntsc or pal
					if ( _SYSTEM._GPU.GPU_CTRL_Read.VIDEO )
					{
						// PAL //
						//TargetTimer += ( TicksPerSec / 50 );
						TargetTimer += ( ( (double) TicksPerSec ) / GPU::PAL_FramesPerSec );
					}
					else
					{
						// NTSC //
						//TargetTimer += ( TicksPerSec / 60 );
						TargetTimer += ( ( (double) TicksPerSec ) / GPU::NTSC_FramesPerSec );
					}
					
					
					// check if we are running slower than target
					if ( !QueryPerformanceCounter ( (LARGE_INTEGER*) &CurrentTimer ) )
					{
						cout << "\nhps1x64: Error returned from QueryPerformanceCounter\n";
					}
					
					TicksLeft = TargetTimer - CurrentTimer;
					
					bRunningTooSlow = false;
					if ( TicksLeft < 0 )
					{
						// running too slow //
						bRunningTooSlow = true;
						
						//MsgWaitForMultipleObjectsEx( NULL, NULL, 1, QS_ALLINPUT, MWMO_ALERTABLE );
						//Sleep ( 1 );
					}
					else
					{
						//MilliSecsToWait = (u64) ( ( (double) TicksLeft ) / dTicksPerMilliSec );
						
						//MsgWaitForMultipleObjectsEx( NULL, NULL, MilliSecsToWait, QS_ALLINPUT, MWMO_ALERTABLE );
						//Sleep ( MilliSecsToWait );
					}
					
					
					// process events
					//WindowClass::DoEventsNoWait ();
					//WindowClass::DoSingleEvent ();
					
					
					
					
					do
					{
						// active-wait
						
						// process events
						WindowClass::DoEventsNoWait ();
						
						
						if ( !QueryPerformanceCounter ( (LARGE_INTEGER*) &CurrentTimer ) )
						{
							cout << "\nhpsx64: Error returned from QueryPerformanceCounter\n";
						}
						
						TicksLeft = TargetTimer - CurrentTimer;
						
						MilliSecsToWait = (u64) ( ( (double) TicksLeft ) / dTicksPerMilliSec );
						
						if ( MilliSecsToWait <= 0 ) MilliSecsToWait = 0;
						
						MsgWaitForMultipleObjectsEx( NULL, NULL, MilliSecsToWait, QS_ALLINPUT, MWMO_ALERTABLE );
						
						// process events
						//WindowClass::DoEventsNoWait ();
						//WindowClass::DoSingleEvent ();
						
						if ( !QueryPerformanceCounter ( (LARGE_INTEGER*) &CurrentTimer ) )
						{
							cout << "\nhpsx64: Error returned from QueryPerformanceCounter\n";
						}
						
					} while ( CurrentTimer < TargetTimer );
					
					
					if ( WindowClass::Window::InModalMenuLoop )
					{
						
						GPU::End_Frame ();
					
						// if menu has been clicked then wait
						WindowClass::Window::WaitForModalMenuLoop ();
						
						GPU::Start_Frame ();
					}
					
					// if menu was clicked, hop out of loop
					if ( HandleMenuClick () ) break;
					
					
					// check if we are running too slow
					//if ( CurrentTimer > TargetTimer )
					if ( bRunningTooSlow )
					{
						// set the new timer target to be the current timer
						if ( !QueryPerformanceCounter ( (LARGE_INTEGER*) &TargetTimer ) )
						{
							cout << "\nhps1x64: Error returned from QueryPerformanceCounter\n";
						}
					}
					
				}
				
				
				// update all the debug info windows that are showing
				DebugWindow_Update ();
				
				
				if ( !_RunMode.RunNormal ) cout << "\nWaiting for command\n";
				
				
				// get the speed as a percentage of full speed
				double dPerf;
				stringstream ss;
				QueryPerformanceCounter ( (LARGE_INTEGER*) &ullPerfEnd_Timer );
				TicksLeft = ullPerfEnd_Timer - ullPerfStart_Timer;
				dPerf = ( ( dTicksPerMilliSec * 1000L ) / TicksLeft ) * 100L;
				if ( _SYSTEM._GPU.GPU_CTRL_Read.VIDEO )
				{
					ss << "hps1x64 - Speed(PAL): " << dPerf << "%";
					ulFramesPerSec = 50;
				}
				else
				{
					ss << "hps1x64 - Speed(NTSC): " << dPerf << "%";
					ulFramesPerSec = 60;
				}
				ProgramWindow->SetCaption( ss.str().c_str() );
			}
			
			// multi-threading testing
			GPU::End_Frame ();
		}
		
	}
	
	cout << "\nDone running program\n";
	
	// write back memory cards
	_SYSTEM._SIO.Store_MemoryCardFile ( ExecutablePath + "card0", 0 );
	_SYSTEM._SIO.Store_MemoryCardFile ( ExecutablePath + "card1", 1 );
	
	cout << "\nSaving config...";

	// save configuration
	SaveConfig ( ExecutablePath + "hps1x64.hcfg" );

	return 1;
}





void hps1x64::OnClick_File_Load_State ( int i )
{
	
	cout << "\nYou clicked File | Load | State\n";
	_HPS1X64.LoadState ();
	
	_MenuClick = 1;
}


void hps1x64::OnClick_File_Load_BIOS ( int i )
{
	
	cout << "\nYou clicked File | Load | BIOS\n";
	_HPS1X64.LoadBIOS ();

	cout << "\n->last bios set to: " << sLastBiosPath.c_str();

	_HPS1X64.Update_CheckMarksOnMenu();

	_MenuClick = 1;
}


void hps1x64::OnClick_File_Load_GameDisk ( int i )
{
	
	string ImagePath;
	bool bDiskOpened;
	
	cout << "\nYou clicked File | Load | Game Disk\n";
	
	if ( _HPS1X64._SYSTEM._CD.isLidOpen )
	{
		// lid is currently open //
		ImagePath = _HPS1X64.LoadDisk ();
		
		if ( ImagePath != "" )
		{
			bDiskOpened = _HPS1X64._SYSTEM._CD.cd_image.OpenDiskImage ( ImagePath );
		
			// *** testing ***
			/*
			DiskImage::CDImage::_DISKIMAGE->SeekTime ( 29, 17, 0 );
			DiskImage::CDImage::_DISKIMAGE->StartReading ();
			DiskImage::CDImage::_DISKIMAGE->ReadNextSector ();
			cout << hex << "\n\n" << (u32) ((DiskImage::CDImage::Sector::SubQ*)DiskImage::CDImage::_DISKIMAGE->CurrentSubBuffer)->AbsoluteAddress [ 0 ];
			cout << hex << "\n" << (u32) ((DiskImage::CDImage::Sector::SubQ*)DiskImage::CDImage::_DISKIMAGE->CurrentSubBuffer)->AbsoluteAddress [ 1 ];
			cout << hex << "\n" << (u32) ((DiskImage::CDImage::Sector::SubQ*)DiskImage::CDImage::_DISKIMAGE->CurrentSubBuffer)->AbsoluteAddress [ 2 ];
			cout << hex << "\n\n";
			*/

			if ( bDiskOpened )
			{
				cout << "\nhpsx64 NOTE: Game Disk opened successfully\n";
				_HPS1X64._SYSTEM._CD.isGameCD = true;
				
				// lid should now be closed since disk is open
				_HPS1X64._SYSTEM._CD.isLidOpen = false;
				
				_HPS1X64._SYSTEM._CD.Event_LidClose ();
				
				// output info for the loaded disk
				_HPS1X64._SYSTEM._CD.cd_image.Output_IndexData ();
				
				// *** testing *** output some test SubQ data
				/*
				_SYSTEM._CD.cd_image.Output_SubQData ( 0, 2, 0 );
				_SYSTEM._CD.cd_image.Output_SubQData ( 0, 2, 1 );
				_SYSTEM._CD.cd_image.Output_SubQData ( 3, 34, 29 );
				_SYSTEM._CD.cd_image.Output_SubQData ( 3, 34, 30 );
				_SYSTEM._CD.cd_image.Output_SubQData ( 3, 34, 31 );
				_SYSTEM._CD.cd_image.Output_SubQData ( 3, 32, 30 );
				_SYSTEM._CD.cd_image.Output_SubQData ( 3, 32, 31 );
				*/
				/*
				unsigned char AMin = 0, ASec = 0, AFrac = 0;
				cout << "\nTrack for 0,2,1=" << dec << _SYSTEM._CD.cd_image.FindTrack ( 0, 2, 1 );
				cout << "\nTrack for 0,2,0=" << dec << _SYSTEM._CD.cd_image.FindTrack ( 0, 2, 0 );
				cout << "\nTrack for 3,35,7=" << dec << _SYSTEM._CD.cd_image.FindTrack ( 3, 35, 7 );
				cout << "\nTrack for 47,0,0=" << dec << _SYSTEM._CD.cd_image.FindTrack ( 47, 0, 0 );
				_SYSTEM._CD.cd_image.GetTrackStart ( 1, AMin, ASec, AFrac );
				cout << "\nTrack 1 Starts At AMin=" << dec << (u32)AMin << " ASec=" << (u32)ASec << " AFrac=" << (u32)AFrac;
				_SYSTEM._CD.cd_image.GetTrackStart ( 2, AMin, ASec, AFrac );
				cout << "\nTrack 2 Starts At AMin=" << dec << (u32)AMin << " ASec=" << (u32)ASec << " AFrac=" << (u32)AFrac;
				_SYSTEM._CD.cd_image.GetTrackStart ( 3, AMin, ASec, AFrac );
				cout << "\nTrack 3 Starts At AMin=" << dec << (u32)AMin << " ASec=" << (u32)ASec << " AFrac=" << (u32)AFrac;
				_SYSTEM._CD.cd_image.GetTrackStart ( 10, AMin, ASec, AFrac );
				cout << "\nTrack 10 Starts At AMin=" << dec << (u32)AMin << " ASec=" << (u32)ASec << " AFrac=" << (u32)AFrac;
				*/
			}
			else
			{
				cout << "\nhpsx64 ERROR: Problem opening disk\n";
			}
		}
		else
		{
			cout << "\nERROR: Unable to open disk image. Either no disk was chosen or other problem.";
		}
	}
	else
	{
		// lid is currently closed //
		
		// open the lid
		_HPS1X64._SYSTEM._CD.isLidOpen = true;
		
		// close the currently open disk image
		_HPS1X64._SYSTEM._CD.cd_image.CloseDiskImage ();
		
		_HPS1X64._SYSTEM._CD.Event_LidOpen ();
	}
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}


void hps1x64::OnClick_File_Load_AudioDisk ( int i )
{
	string ImagePath;
	bool bDiskOpened;
	
	cout << "\nYou clicked File | Load | Audio Disk\n";
	
	if ( _HPS1X64._SYSTEM._CD.isLidOpen )
	{
		// lid is currently open //
		ImagePath = _HPS1X64.LoadDisk ();
		
		if ( ImagePath != "" )
		{
			bDiskOpened = _HPS1X64._SYSTEM._CD.cd_image.OpenDiskImage ( ImagePath );
		
			if ( bDiskOpened )
			{
				cout << "\nhpsx64 NOTE: Audio Disk opened successfully\n";
				_HPS1X64._SYSTEM._CD.isGameCD = false;
				
				// lid should now be closed since disk is open
				_HPS1X64._SYSTEM._CD.isLidOpen = false;
				
				_HPS1X64._SYSTEM._CD.Event_LidClose ();
			}
			else
			{
				cout << "\nhpsx64 ERROR: Problem opening disk\n";
			}
		}
		else
		{
			cout << "\nERROR: Unable to open disk image. Either no disk was chosen or other problem.";
		}
	}
	else
	{
		// lid is currently closed //
		
		// open the lid
		_HPS1X64._SYSTEM._CD.isLidOpen = true;
		
		// close the currently open disk image
		_HPS1X64._SYSTEM._CD.cd_image.CloseDiskImage ();
		
		_HPS1X64._SYSTEM._CD.Event_LidOpen ();
	}
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}



void hps1x64::OnClick_File_Save_State ( int i )
{
	cout << "\nYou clicked File | Save | State\n";
	_HPS1X64.SaveState ();
	
	_MenuClick = 1;
}


void hps1x64::OnClick_File_Reset ( int i )
{
	
	cout << "\nYou clicked File | Reset\n";
	
	// need to call start, not reset
	_HPS1X64._SYSTEM.Start ();
	
	_MenuClick = 1;
}




void hps1x64::OnClick_File_Run ( int i )
{
	
	cout << "\nYou clicked File | Run\n";
	_HPS1X64._RunMode.Value = 0;
	
	// if there are no breakpoints, then we can run in normal mode
	if ( !_HPS1X64._SYSTEM._CPU.Breakpoints->Count() )
	{
		_HPS1X64._RunMode.RunNormal = true;
	}
	else
	{
		_HPS1X64._RunMode.RunDebug = true;
	}
	
	// clear the last breakpoint hit
	_HPS1X64._SYSTEM._CPU.Breakpoints->Clear_LastBreakPoint ();
	
	// clear read/write debugging info
	_HPS1X64._SYSTEM._CPU.Last_ReadAddress = 0;
	_HPS1X64._SYSTEM._CPU.Last_WriteAddress = 0;
	_HPS1X64._SYSTEM._CPU.Last_ReadWriteAddress = 0;
	
	_MenuClick = 1;
}


void hps1x64::OnClick_File_Exit ( int i )
{
	cout << "\nYou clicked File | Exit\n";
	
	// uuuuuuser chose to exit program
	_RunMode.Value = 0;
	_RunMode.Exit = true;
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Debug_Break ( int i )
{
	
	cout << "\nYou clicked Debug | Break\n";
	
	// clear the last breakpoint hit if system is running
	if  ( _RunMode.Value != 0 ) _HPS1X64._SYSTEM._CPU.Breakpoints->Clear_LastBreakPoint ();
	
	_RunMode.Value = 0;
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Debug_StepInto ( int i )
{
	
	cout << "\nYou clicked Debug | Step Into\n";
	_HPS1X64._SYSTEM.Run ();
	
	// clear the last breakpoint hit
	_HPS1X64._SYSTEM._CPU.Breakpoints->Clear_LastBreakPoint ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Debug_OutputCurrentSector ( int i )
{
	
	long *pData = (long*) & SIO::DJoy.gameControllerStates[0];
	
	SIO::DJoy.Update( 0 );
	
	cout << "\ngameControllerState: ";
	cout << "\nPOV0=" << hex << SIO::DJoy.gameControllerStates[0].rgdwPOV[0] << " " << dec << SIO::DJoy.gameControllerStates[0].rgdwPOV[0];
	cout << "\nPOV1=" << hex << SIO::DJoy.gameControllerStates[0].rgdwPOV[1] << " " << dec << SIO::DJoy.gameControllerStates[0].rgdwPOV[1];
	cout << "\nPOV2=" << hex << SIO::DJoy.gameControllerStates[0].rgdwPOV[2] << " " << dec << SIO::DJoy.gameControllerStates[0].rgdwPOV[2];
	cout << "\nPOV3=" << hex << SIO::DJoy.gameControllerStates[0].rgdwPOV[3] << " " << dec << SIO::DJoy.gameControllerStates[0].rgdwPOV[3];
	
	for ( int i = 0; i < 32; i++ )
	{
		cout << "\nPOV" << dec << i << "=" << hex << (unsigned long) SIO::DJoy.gameControllerStates[0].rgbButtons[i] << " " << dec << (unsigned long) SIO::DJoy.gameControllerStates[0].rgbButtons[i];
	}
	
	for ( int i = 0; i < 6; i++ )
	{
		cout << "\nAxis#" << dec << i << "=" << hex << *pData << " " << dec << *pData;
		pData++;
	}
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Debug_Show_All ( int i )
{
	cout << "\nYou clicked Debug | Show Window | All\n";
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Debug_Show_FrameBuffer ( int i )
{
	
	cout << "\nYou clicked Debug | Show Window | FrameBuffer\n";
	
	if ( ProgramWindow->Menus->CheckItem ( "Frame Buffer" ) == MF_CHECKED )
	{
		// was already checked, so disable debug window for R3000A and uncheck item
		cout << "Disabling debug window for GPU\n";
		_HPS1X64._SYSTEM._GPU.DebugWindow_Disable ();
		ProgramWindow->Menus->UnCheckItem ( "Frame Buffer" );
	}
	else
	{
		// was not already checked, so enable debugging
		cout << "Enabling debug window for GPU\n";
		_HPS1X64._SYSTEM._GPU.DebugWindow_Enable ();
	}
	
	cout << "\nNo Crash1";
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Debug_Show_R3000A ( int i )
{
	
	cout << "\nYou clicked Debug | Show Window | R3000A\n";
	
	if ( ProgramWindow->Menus->CheckItem ( "R3000A" ) == MF_CHECKED )
	{
		// was already checked, so disable debug window for R3000A and uncheck item
		cout << "Disabling debug window for R3000A\n";
		_HPS1X64._SYSTEM._CPU.DebugWindow_Disable ();
		ProgramWindow->Menus->UnCheckItem ( "R3000A" );
	}
	else
	{
		// was not already checked, so enable debugging
		cout << "Enabling debug window for R3000A\n";
		_HPS1X64._SYSTEM._CPU.DebugWindow_Enable ();
	}
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Debug_Show_Memory ( int i )
{
	
	cout << "\nYou clicked Debug | Show Window | Memory\n";
	
	if ( ProgramWindow->Menus->CheckItem ( "Memory" ) == MF_CHECKED )
	{
		// was already checked, so disable debug window for R3000A and uncheck item
		cout << "Disabling debug window for Bus\n";
		_HPS1X64._SYSTEM._BUS.DebugWindow_Disable ();
		ProgramWindow->Menus->UnCheckItem ( "Memory" );
	}
	else
	{
		// was not already checked, so enable debugging
		cout << "Enabling debug window for Bus\n";
		_HPS1X64._SYSTEM._BUS.DebugWindow_Enable ();
	}
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Debug_Show_DMA ( int i )
{
	
	cout << "\nYou clicked Debug | Show Window | DMA\n";
	if ( ProgramWindow->Menus->CheckItem ( "DMA" ) == MF_CHECKED )
	{
		// was already checked, so disable debug window for R3000A and uncheck item
		_HPS1X64._SYSTEM._DMA.DebugWindow_Disable ();
		ProgramWindow->Menus->UnCheckItem ( "DMA" );
	}
	else
	{
		// was not already checked, so enable debugging
		_HPS1X64._SYSTEM._DMA.DebugWindow_Enable ();
	}
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Debug_Show_TIMER ( int i )
{
	cout << "\nYou clicked Debug | Show Window | Timers\n";
	if ( ProgramWindow->Menus->CheckItem ( "Timers" ) == MF_CHECKED )
	{
		// was already checked, so disable debug window for R3000A and uncheck item
		_HPS1X64._SYSTEM._TIMERS.DebugWindow_Disable ();
		ProgramWindow->Menus->UnCheckItem ( "Timers" );
	}
	else
	{
		// was not already checked, so enable debugging
		_HPS1X64._SYSTEM._TIMERS.DebugWindow_Enable ();
	}
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Debug_Show_SPU ( int i )
{
	cout << "\nYou clicked Debug | Show Window | SPU\n";
	if ( ProgramWindow->Menus->CheckItem ( "SPU" ) == MF_CHECKED )
	{
		// was already checked, so disable debug window for R3000A and uncheck item
		_HPS1X64._SYSTEM._SPU.DebugWindow_Disable ();
		ProgramWindow->Menus->UnCheckItem ( "SPU" );
	}
	else
	{
		// was not already checked, so enable debugging
		_HPS1X64._SYSTEM._SPU.DebugWindow_Enable ();
	}
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Debug_Show_CD ( int i )
{
	cout << "\nYou clicked Debug | Show Window | CD\n";
	if ( ProgramWindow->Menus->CheckItem ( "CD" ) == MF_CHECKED )
	{
		// was already checked, so disable debug window for R3000A and uncheck item
		_HPS1X64._SYSTEM._CD.DebugWindow_Disable ();
		ProgramWindow->Menus->UnCheckItem ( "CD" );
	}
	else
	{
		// was not already checked, so enable debugging
		_HPS1X64._SYSTEM._CD.DebugWindow_Enable ();
	}
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Debug_Show_INTC ( int i )
{
	cout << "\nYou clicked Debug | Show Window | INTC\n";
	if ( ProgramWindow->Menus->CheckItem ( "INTC" ) == MF_CHECKED )
	{
		// was already checked, so disable debug window for R3000A and uncheck item
		_HPS1X64._SYSTEM._INTC.DebugWindow_Disable ();
		ProgramWindow->Menus->UnCheckItem ( "INTC" );
	}
	else
	{
		// was not already checked, so enable debugging
		_HPS1X64._SYSTEM._INTC.DebugWindow_Enable ();
	}
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Controllers0_Configure ( int i )
{
	cout << "\nYou clicked Controllers | Configure...\n";
	
	Dialog_KeyConfigure::KeyConfigure [ 0 ] = _HPS1X64._SYSTEM._SIO.Key_X [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 1 ] = _HPS1X64._SYSTEM._SIO.Key_O [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 2 ] = _HPS1X64._SYSTEM._SIO.Key_Triangle [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 3 ] = _HPS1X64._SYSTEM._SIO.Key_Square [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 4 ] = _HPS1X64._SYSTEM._SIO.Key_R1 [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 5 ] = _HPS1X64._SYSTEM._SIO.Key_R2 [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 6 ] = _HPS1X64._SYSTEM._SIO.Key_R3 [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 7 ] = _HPS1X64._SYSTEM._SIO.Key_L1 [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 8 ] = _HPS1X64._SYSTEM._SIO.Key_L2 [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 9 ] = _HPS1X64._SYSTEM._SIO.Key_L3 [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 10 ] = _HPS1X64._SYSTEM._SIO.Key_Start [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 11 ] = _HPS1X64._SYSTEM._SIO.Key_Select [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 12 ] = _HPS1X64._SYSTEM._SIO.LeftAnalog_X [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 13 ] = _HPS1X64._SYSTEM._SIO.LeftAnalog_Y [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 14 ] = _HPS1X64._SYSTEM._SIO.RightAnalog_X [ 0 ];
	Dialog_KeyConfigure::KeyConfigure [ 15 ] = _HPS1X64._SYSTEM._SIO.RightAnalog_Y [ 0 ];
	
	// first make sure that there are joysticks connected
	if (!DJoySticks::gameControllers.size())
	{
		Playstation1::SIO::DJoy.ReInit();
	}

	// make sure there is a game controller connected before showing dialog
	if (DJoySticks::gameControllers.size())
	{
		if (Dialog_KeyConfigure::Show_ConfigureKeysDialog(0))
		{
			_HPS1X64._SYSTEM._SIO.Key_X[0] = Dialog_KeyConfigure::KeyConfigure[0];
			_HPS1X64._SYSTEM._SIO.Key_O[0] = Dialog_KeyConfigure::KeyConfigure[1];
			_HPS1X64._SYSTEM._SIO.Key_Triangle[0] = Dialog_KeyConfigure::KeyConfigure[2];
			_HPS1X64._SYSTEM._SIO.Key_Square[0] = Dialog_KeyConfigure::KeyConfigure[3];
			_HPS1X64._SYSTEM._SIO.Key_R1[0] = Dialog_KeyConfigure::KeyConfigure[4];
			_HPS1X64._SYSTEM._SIO.Key_R2[0] = Dialog_KeyConfigure::KeyConfigure[5];
			_HPS1X64._SYSTEM._SIO.Key_R3[0] = Dialog_KeyConfigure::KeyConfigure[6];
			_HPS1X64._SYSTEM._SIO.Key_L1[0] = Dialog_KeyConfigure::KeyConfigure[7];
			_HPS1X64._SYSTEM._SIO.Key_L2[0] = Dialog_KeyConfigure::KeyConfigure[8];
			_HPS1X64._SYSTEM._SIO.Key_L3[0] = Dialog_KeyConfigure::KeyConfigure[9];
			_HPS1X64._SYSTEM._SIO.Key_Start[0] = Dialog_KeyConfigure::KeyConfigure[10];
			_HPS1X64._SYSTEM._SIO.Key_Select[0] = Dialog_KeyConfigure::KeyConfigure[11];
			_HPS1X64._SYSTEM._SIO.LeftAnalog_X[0] = Dialog_KeyConfigure::KeyConfigure[12];
			_HPS1X64._SYSTEM._SIO.LeftAnalog_Y[0] = Dialog_KeyConfigure::KeyConfigure[13];
			_HPS1X64._SYSTEM._SIO.RightAnalog_X[0] = Dialog_KeyConfigure::KeyConfigure[14];
			_HPS1X64._SYSTEM._SIO.RightAnalog_Y[0] = Dialog_KeyConfigure::KeyConfigure[15];
		}
	}
	else
	{
		cout << "\n*** hps1x64: *ERROR* no game controller detected!!! ***\n";
	}
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Controllers1_Configure ( int i )
{
	cout << "\nYou clicked Controllers | Configure...\n";
	
	Dialog_KeyConfigure::KeyConfigure [ 0 ] = _HPS1X64._SYSTEM._SIO.Key_X [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 1 ] = _HPS1X64._SYSTEM._SIO.Key_O [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 2 ] = _HPS1X64._SYSTEM._SIO.Key_Triangle [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 3 ] = _HPS1X64._SYSTEM._SIO.Key_Square [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 4 ] = _HPS1X64._SYSTEM._SIO.Key_R1 [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 5 ] = _HPS1X64._SYSTEM._SIO.Key_R2 [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 6 ] = _HPS1X64._SYSTEM._SIO.Key_R3 [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 7 ] = _HPS1X64._SYSTEM._SIO.Key_L1 [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 8 ] = _HPS1X64._SYSTEM._SIO.Key_L2 [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 9 ] = _HPS1X64._SYSTEM._SIO.Key_L3 [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 10 ] = _HPS1X64._SYSTEM._SIO.Key_Start [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 11 ] = _HPS1X64._SYSTEM._SIO.Key_Select [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 12 ] = _HPS1X64._SYSTEM._SIO.LeftAnalog_X [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 13 ] = _HPS1X64._SYSTEM._SIO.LeftAnalog_Y [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 14 ] = _HPS1X64._SYSTEM._SIO.RightAnalog_X [ 1 ];
	Dialog_KeyConfigure::KeyConfigure [ 15 ] = _HPS1X64._SYSTEM._SIO.RightAnalog_Y [ 1 ];
	
	// first make sure that there are joysticks connected
	if (!DJoySticks::gameControllers.size())
	{
		Playstation1::SIO::DJoy.ReInit();
	}

	// make sure there is a game controller connected before showing dialog
	if (DJoySticks::gameControllers.size())
	{
		if (Dialog_KeyConfigure::Show_ConfigureKeysDialog(1))
		{
			_HPS1X64._SYSTEM._SIO.Key_X[1] = Dialog_KeyConfigure::KeyConfigure[0];
			_HPS1X64._SYSTEM._SIO.Key_O[1] = Dialog_KeyConfigure::KeyConfigure[1];
			_HPS1X64._SYSTEM._SIO.Key_Triangle[1] = Dialog_KeyConfigure::KeyConfigure[2];
			_HPS1X64._SYSTEM._SIO.Key_Square[1] = Dialog_KeyConfigure::KeyConfigure[3];
			_HPS1X64._SYSTEM._SIO.Key_R1[1] = Dialog_KeyConfigure::KeyConfigure[4];
			_HPS1X64._SYSTEM._SIO.Key_R2[1] = Dialog_KeyConfigure::KeyConfigure[5];
			_HPS1X64._SYSTEM._SIO.Key_R3[1] = Dialog_KeyConfigure::KeyConfigure[6];
			_HPS1X64._SYSTEM._SIO.Key_L1[1] = Dialog_KeyConfigure::KeyConfigure[7];
			_HPS1X64._SYSTEM._SIO.Key_L2[1] = Dialog_KeyConfigure::KeyConfigure[8];
			_HPS1X64._SYSTEM._SIO.Key_L3[1] = Dialog_KeyConfigure::KeyConfigure[9];
			_HPS1X64._SYSTEM._SIO.Key_Start[1] = Dialog_KeyConfigure::KeyConfigure[10];
			_HPS1X64._SYSTEM._SIO.Key_Select[1] = Dialog_KeyConfigure::KeyConfigure[11];
			_HPS1X64._SYSTEM._SIO.LeftAnalog_X[1] = Dialog_KeyConfigure::KeyConfigure[12];
			_HPS1X64._SYSTEM._SIO.LeftAnalog_Y[1] = Dialog_KeyConfigure::KeyConfigure[13];
			_HPS1X64._SYSTEM._SIO.RightAnalog_X[1] = Dialog_KeyConfigure::KeyConfigure[14];
			_HPS1X64._SYSTEM._SIO.RightAnalog_Y[1] = Dialog_KeyConfigure::KeyConfigure[15];
		}
	}
	else
	{
		cout << "\n*** hps1x64: *ERROR* no game controller detected!!! ***\n";
	}
	
	_MenuClick = 1;
}



void hps1x64::OnClick_Pad1Type_Digital ( int i )
{
	// set pad 1 to digital
	_HPS1X64._SYSTEM._SIO.ControlPad_Type [ 0 ] = 0;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Pad1Type_Analog ( int i )
{
	// set pad 1 to analog
	_HPS1X64._SYSTEM._SIO.ControlPad_Type [ 0 ] = 1;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Pad1Input_None ( int i )
{
	_HPS1X64._SYSTEM._SIO.PortMapping [ 0 ] = -1;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}
void hps1x64::OnClick_Pad1Input_Device0 ( int i )
{
	_HPS1X64._SYSTEM._SIO.PortMapping [ 0 ] = 0;
	
	if ( _HPS1X64._SYSTEM._SIO.PortMapping [ 1 ] == 0 )
	{
		_HPS1X64._SYSTEM._SIO.PortMapping [ 1 ] = -1;
	}
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}
void hps1x64::OnClick_Pad1Input_Device1 ( int i )
{
	_HPS1X64._SYSTEM._SIO.PortMapping [ 0 ] = 1;
	
	if ( _HPS1X64._SYSTEM._SIO.PortMapping [ 1 ] == 1 )
	{
		_HPS1X64._SYSTEM._SIO.PortMapping [ 1 ] = -1;
	}
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}


void hps1x64::OnClick_Pad2Type_Digital ( int i )
{
	// set pad 2 to digital
	_HPS1X64._SYSTEM._SIO.ControlPad_Type [ 1 ] = 0;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Pad2Type_Analog ( int i )
{
	// set pad 2 to analog
	_HPS1X64._SYSTEM._SIO.ControlPad_Type [ 1 ] = 1;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Pad2Input_None ( int i )
{
	_HPS1X64._SYSTEM._SIO.PortMapping [ 1 ] = -1;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}
void hps1x64::OnClick_Pad2Input_Device0 ( int i )
{
	_HPS1X64._SYSTEM._SIO.PortMapping [ 1 ] = 0;
	
	if ( _HPS1X64._SYSTEM._SIO.PortMapping [ 0 ] == 0 )
	{
		_HPS1X64._SYSTEM._SIO.PortMapping [ 0 ] = -1;
	}
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}
void hps1x64::OnClick_Pad2Input_Device1 ( int i )
{
	_HPS1X64._SYSTEM._SIO.PortMapping [ 1 ] = 1;
	
	if ( _HPS1X64._SYSTEM._SIO.PortMapping [ 0 ] == 1 )
	{
		_HPS1X64._SYSTEM._SIO.PortMapping [ 0 ] = -1;
	}
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}


void hps1x64::OnClick_Card1_Connect ( int i )
{
	// set memory card 1 to connected
	_HPS1X64._SYSTEM._SIO.MemoryCard_ConnectionState [ 0 ] = 0;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Card1_Disconnect ( int i )
{
	// set memory card 1 to disconnected
	_HPS1X64._SYSTEM._SIO.MemoryCard_ConnectionState [ 0 ] = 1;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}


void hps1x64::OnClick_Redetect_Pads ( int i )
{
	// set memory card 1 to disconnected
	SIO::DJoy.ReInit ();
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}


void hps1x64::OnClick_Card2_Connect ( int i )
{
	// set memory card 2 to connected
	_HPS1X64._SYSTEM._SIO.MemoryCard_ConnectionState [ 1 ] = 0;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Card2_Disconnect ( int i )
{
	// set memory card 2 to disconnected
	_HPS1X64._SYSTEM._SIO.MemoryCard_ConnectionState [ 1 ] = 1;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}


void hps1x64::OnClick_Region_Europe ( int i )
{
	_HPS1X64._SYSTEM._CD.Region = 'E';
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Region_Japan ( int i )
{
	_HPS1X64._SYSTEM._CD.Region = 'I';
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Region_NorthAmerica ( int i )
{
	_HPS1X64._SYSTEM._CD.Region = 'A';
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Audio_Enable ( int i )
{
	if ( _HPS1X64._SYSTEM._SPU.AudioOutput_Enabled )
	{
		_HPS1X64._SYSTEM._SPU.AudioOutput_Enabled = false;
	}
	else
	{
		_HPS1X64._SYSTEM._SPU.AudioOutput_Enabled = true;
	}
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Audio_Volume_100 ( int i )
{
	_HPS1X64._SYSTEM._SPU.GlobalVolume = 0x7fff;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Audio_Volume_75 ( int i )
{
	_HPS1X64._SYSTEM._SPU.GlobalVolume = 0x3000;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Audio_Volume_50 ( int i )
{
	_HPS1X64._SYSTEM._SPU.GlobalVolume = 0x1000;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Audio_Volume_25 ( int i )
{
	_HPS1X64._SYSTEM._SPU.GlobalVolume = 0x400;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Audio_Buffer_8k ( int i )
{
	_HPS1X64._SYSTEM._SPU.NextPlayBuffer_Size = 8192;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Audio_Buffer_16k ( int i )
{
	_HPS1X64._SYSTEM._SPU.NextPlayBuffer_Size = 16384;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Audio_Buffer_32k ( int i )
{
	_HPS1X64._SYSTEM._SPU.NextPlayBuffer_Size = 32768;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Audio_Buffer_64k ( int i )
{
	_HPS1X64._SYSTEM._SPU.NextPlayBuffer_Size = 65536;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Audio_Buffer_1m ( int i )
{
	_HPS1X64._SYSTEM._SPU.NextPlayBuffer_Size = 131072;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}

void hps1x64::OnClick_Audio_Filter ( int i )
{
	cout << "\nYou clicked Audio | Filter\n";
	
	if ( _HPS1X64._SYSTEM._SPU.AudioFilter_Enabled )
	{
		_HPS1X64._SYSTEM._SPU.AudioFilter_Enabled = false;
	}
	else
	{
		_HPS1X64._SYSTEM._SPU.AudioFilter_Enabled = true;
	}
	
	_HPS1X64.Update_CheckMarksOnMenu ();
	
	_MenuClick = 1;
}


void hps1x64::OnClick_Video_FullScreen ( int i )
{
	cout << "\nYou clicked Video | FullScreen\n";
	
	GPU::MainProgramWindow_Width = (long) ( ((float)ProgramWindow_Width) * 1.0f );
	GPU::MainProgramWindow_Height = (long) ( ((float)ProgramWindow_Height) * 1.0f );
	
	if( ! ProgramWindow->fullscreen )
	{
		ProgramWindow->SetWindowSize ( GPU::MainProgramWindow_Width, GPU::MainProgramWindow_Height );
	}
	
	ProgramWindow->ToggleGLFullScreen ();
	
	_MenuClick = 1;
}


void hps1x64::OnClick_Video_WindowSizeX1 ( int i )
{
	GPU::MainProgramWindow_Width = (long) ( ((float)ProgramWindow_Width) * 1.0f );
	GPU::MainProgramWindow_Height = (long) ( ((float)ProgramWindow_Height) * 1.0f );
	ProgramWindow->SetWindowSize ( GPU::MainProgramWindow_Width, GPU::MainProgramWindow_Height );
}

void hps1x64::OnClick_Video_WindowSizeX15 ( int i )
{
	GPU::MainProgramWindow_Width = (long) ( ((float)ProgramWindow_Width) * 1.5f );
	GPU::MainProgramWindow_Height = (long) ( ((float)ProgramWindow_Height) * 1.5f );
	ProgramWindow->SetWindowSize ( GPU::MainProgramWindow_Width, GPU::MainProgramWindow_Height );
}

void hps1x64::OnClick_Video_WindowSizeX2 ( int i )
{
	GPU::MainProgramWindow_Width = (long) ( ((float)ProgramWindow_Width) * 2.0f );
	GPU::MainProgramWindow_Height = (long) ( ((float)ProgramWindow_Height) * 2.0f );
	ProgramWindow->SetWindowSize ( GPU::MainProgramWindow_Width, GPU::MainProgramWindow_Height );
}



void hps1x64::OnClick_Video_ScanlinesEnable ( int i )
{
	cout << "\nYou clicked Video | Scanlines | Enable\n";
	
	_HPS1X64._SYSTEM._GPU.Set_Scanline ( true );
	
	_MenuClick = 1;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
}

void hps1x64::OnClick_Video_ScanlinesDisable ( int i )
{
	cout << "\nYou clicked Video | Scanlines | Disable\n";
	
	_HPS1X64._SYSTEM._GPU.Set_Scanline ( false );
	
	_MenuClick = 1;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
}

void hps1x64::OnClick_R3000ACPU_Interpreter ( int i )
{
	cout << "\nYou clicked CPU | R3000A | Interpreter\n";

	_HPS1X64._SYSTEM._CPU.bEnableRecompiler = false;
	
	_HPS1X64.Update_CheckMarksOnMenu ();
}

void hps1x64::OnClick_R3000ACPU_Recompiler ( int i )
{
	cout << "\nYou clicked CPU | R3000A | Recompiler\n";

	_HPS1X64._SYSTEM._CPU.bEnableRecompiler = true;
	
	// need to reset the recompiler
	_HPS1X64._SYSTEM._CPU.rs->Reset ();
	
	_HPS1X64._SYSTEM._CPU.rs->SetOptimizationLevel ( 1 );
	
	_HPS1X64.Update_CheckMarksOnMenu ();
}

void hps1x64::OnClick_R3000ACPU_Recompiler2 ( int i )
{
	cout << "\nYou clicked CPU | R3000A | Recompiler2\n";

	_HPS1X64._SYSTEM._CPU.bEnableRecompiler = true;
	
	// need to reset the recompiler
	_HPS1X64._SYSTEM._CPU.rs->Reset ();

	_HPS1X64._SYSTEM._CPU.rs->SetOptimizationLevel ( 2 );
	
	_HPS1X64.Update_CheckMarksOnMenu ();
}



void hps1x64::OnClick_GPU_0Threads ( int i )
{
	cout << "\nYou clicked GPU | GPU: Threads | 0 threads\n";
	
	if ( _HPS1X64._SYSTEM._GPU.ulNumberOfThreads )
	{
		_HPS1X64._SYSTEM._GPU.Finish();
		_HPS1X64._SYSTEM._GPU.ulNumberOfThreads = 0;
	}
	
	_HPS1X64.Update_CheckMarksOnMenu ();
}

void hps1x64::OnClick_GPU_1Threads ( int i )
{
	cout << "\nYou clicked GPU | GPU: Threads | 1 threads\n";
	
	if ( ! _HPS1X64._SYSTEM._GPU.ulNumberOfThreads )
	{
		_HPS1X64._SYSTEM._GPU.Finish();
		_HPS1X64._SYSTEM._GPU.ulNumberOfThreads = 1;
	}
	
	_HPS1X64.Update_CheckMarksOnMenu ();
}


void hps1x64::OnClick_GPU_Software ( int i )
{
	if( _HPS1X64._SYSTEM._GPU.bEnable_OpenCL )
	{
		// copy vram from gpu into cpu ram
		_HPS1X64._SYSTEM._GPU.Copy_VRAM_toCPU();

		// make the change
		_HPS1X64._SYSTEM._GPU.bEnable_OpenCL = 0;
	}

	_HPS1X64.Update_CheckMarksOnMenu ();
}

void hps1x64::OnClick_GPU_Hardware ( int i )
{
	if( !_HPS1X64._SYSTEM._GPU.bEnable_OpenCL )
	{
		// copy vram from cpu ram into gpu
		_HPS1X64._SYSTEM._GPU.Copy_VRAM_toGPU();

		// make the change
		_HPS1X64._SYSTEM._GPU.bEnable_OpenCL = 1;
	}

	_HPS1X64.Update_CheckMarksOnMenu ();
}



void hps1x64::SaveState ( string FilePath )
{
#ifdef INLINE_DEBUG
	debug << "\r\nEntered function: System::SaveState";
#endif

	static const char* PathToSaveState = "SaveState.hps1";
	
	// make sure cd is not reading asynchronously??
	_SYSTEM._CD.cd_image.WaitForAllReadsComplete ();

	////////////////////////////////////////////////////////
	// We need to prompt for the file to save state to
	if ( !FilePath.compare ( "" ) )
	{
		FilePath = ProgramWindow->ShowFileSaveDialog ();
	}

	ofstream OutputFile ( FilePath.c_str (), ios::binary );
	
	u32 SizeOfFile;
	
	cout << "Saving state.\n";
	
	if ( !OutputFile )
	{
#ifdef INLINE_DEBUG
	debug << "->Error creating Save State";
#endif

		cout << "Error creating Save State.\n";
		return;
	}


#ifdef INLINE_DEBUG
	debug << "; Creating Save State";
#endif

	// wait for all reads or writes to disk to finish
	//while ( _SYSTEM._CD.game_cd_image.isReadInProgress );
	while ( _SYSTEM._CD.cd_image.isReadInProgress );

	// write entire state into memory
	//OutputFile.write ( (char*) this, sizeof( System ) );
	OutputFile.write ( (char*) &_SYSTEM, sizeof( System ) );
	
	OutputFile.close();
	
	cout << "Done Saving state.\n";
	
#ifdef INLINE_DEBUG
	debug << "->Leaving function: System::SaveState";
#endif
}

void hps1x64::LoadState ( string FilePath )
{
#ifdef INLINE_DEBUG
	debug << "\r\nEntered function: System::LoadState";
#endif

	static const char* PathToSaveState = "SaveState.hps1";

	// make sure cd is not reading asynchronously??
	_SYSTEM._CD.cd_image.WaitForAllReadsComplete ();
	
	////////////////////////////////////////////////////////
	// We need to prompt for the file to save state to
	if ( !FilePath.compare( "" ) )
	{
		FilePath = ProgramWindow->ShowFileOpenDialog ();
	}

	ifstream InputFile ( FilePath.c_str (), ios::binary );

	cout << "Loading state.\n";
	
	if ( !InputFile )
	{
#ifdef INLINE_DEBUG
	debug << "->Error loading save state";
#endif

		cout << "Error loading save state.\n";
		return;
	}


#ifdef INLINE_DEBUG
	debug << "; Creating Load State";
#endif

	Reset ();

	// read entire state from memory
	//InputFile.read ( (char*) this, sizeof( System ) );
	InputFile.read ( (char*) &_SYSTEM, sizeof( System ) );
	
	InputFile.close();
	
	cout << "Done Loading state.\n";
	
	// refresh system (reload static pointers)
	_SYSTEM.Refresh ();
	
#ifdef INLINE_DEBUG
	debug << "->Leaving function: System::LoadState";
#endif
}


void hps1x64::LoadBIOS ( string FilePath )
{
	cout << "Loading BIOS.\n";
	
	////////////////////////////////////////////////////////
	// We need to prompt for the TEST program to run
	if ( !FilePath.compare ( "" ) )
	{
		cout << "Prompting for BIOS file.\n";
		FilePath = ProgramWindow->ShowFileOpenDialog ();
	}
	
	if (!FilePath.compare(""))
	{
		cout << "\nNo file was chosen.\n";
		return;
	}

	cout << "Loading into memory.\n";

	if ( !_SYSTEM.LoadTestProgramIntoBios ( FilePath.c_str() ) )
	{
		// run the test code
		cout << "\nProblem loading test code.\n";
		
		// clearing last bios loaded into memory
		sLastBiosPath = "";

#ifdef INLINE_DEBUG
		debug << "\r\nProblem loading test code.";
#endif

	}
	else
	{
		// code loaded successfully
		cout << "\nCode loaded successfully into BIOS.\n";
		
		// set the path for the bios
		BiosPath = FilePath;
		
		// setting path as the last bios loaded into memory
		sLastBiosPath = FilePath;

	}
	
	
	cout << "LoadBIOS done.\n";

	//UpdateDebugWindow ();
	
	//DebugStatus.LoadBios = false;
}


string hps1x64::LoadDisk ( string FilePath )
{
	cout << "Loading Disk.\n";
	
	////////////////////////////////////////////////////////
	// We need to prompt for the TEST program to run
	if ( !FilePath.compare ( "" ) )
	{
		cout << "Prompting for BIOS file.\n";
		FilePath = ProgramWindow->ShowFileOpenDialog ();
	}
	
	
	cout << "LoadDisk done.\n";
	

	return FilePath;
}


// create config file object
//Config::File cfg;


void hps1x64::LoadConfig ( string ConfigFileName )
{

	// read a JSON file
	std::ifstream i( ConfigFileName );

	if ( !i )
	{
		cout << "\nhps1x64::LoadConfig: Problem loading config file: " << ConfigFileName;
		return;
	}

	json jsonSettings;
	i >> jsonSettings;

	// save the controller settings //

	// pad 1 //

	_SYSTEM._SIO.ControlPad_Type [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "DigitalAnalog" ];

	_SYSTEM._SIO.Key_X [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyX" ];
	_SYSTEM._SIO.Key_O [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyO" ];
	_SYSTEM._SIO.Key_Triangle [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyTriangle" ];
	_SYSTEM._SIO.Key_Square [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeySquare" ];
	_SYSTEM._SIO.Key_R1 [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyR1" ];
	_SYSTEM._SIO.Key_R2 [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyR2" ];
	_SYSTEM._SIO.Key_R3 [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyR3" ];
	_SYSTEM._SIO.Key_L1 [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyL1" ];
	_SYSTEM._SIO.Key_L2 [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyL2" ];
	_SYSTEM._SIO.Key_L3 [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyL3" ];
	_SYSTEM._SIO.Key_Start [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyStart" ];
	_SYSTEM._SIO.Key_Select [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeySelect" ];
	_SYSTEM._SIO.LeftAnalog_X [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyLeftAnalogX" ];
	_SYSTEM._SIO.LeftAnalog_Y [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyLeftAnalogY" ];
	_SYSTEM._SIO.RightAnalog_X [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyRightAnalogX" ];
	_SYSTEM._SIO.RightAnalog_Y [ 0 ] = jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyRightAnalogY" ];

	// pad 2 //

	_SYSTEM._SIO.ControlPad_Type [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "DigitalAnalog" ];

	_SYSTEM._SIO.Key_X [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyX" ];
	_SYSTEM._SIO.Key_O [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyO" ];
	_SYSTEM._SIO.Key_Triangle [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyTriangle" ];
	_SYSTEM._SIO.Key_Square [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeySquare" ];
	_SYSTEM._SIO.Key_R1 [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyR1" ];
	_SYSTEM._SIO.Key_R2 [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyR2" ];
	_SYSTEM._SIO.Key_R3 [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyR3" ];
	_SYSTEM._SIO.Key_L1 [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyL1" ];
	_SYSTEM._SIO.Key_L2 [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyL2" ];
	_SYSTEM._SIO.Key_L3 [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyL3" ];
	_SYSTEM._SIO.Key_Start [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyStart" ];
	_SYSTEM._SIO.Key_Select [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeySelect" ];
	_SYSTEM._SIO.LeftAnalog_X [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyLeftAnalogX" ];
	_SYSTEM._SIO.LeftAnalog_Y [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyLeftAnalogY" ];
	_SYSTEM._SIO.RightAnalog_X [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyRightAnalogX" ];
	_SYSTEM._SIO.RightAnalog_Y [ 1 ] = jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyRightAnalogY" ];

	// memory cards //

	_SYSTEM._SIO.MemoryCard_ConnectionState [ 0 ] = jsonSettings [ "Controllers" ] [ "MemoryCard1" ] [ "Disconnected" ];
	_SYSTEM._SIO.MemoryCard_ConnectionState [ 1 ] = jsonSettings [ "Controllers" ] [ "MemoryCard2" ] [ "Disconnected" ];

	// device settings //

	// CD //

	_SYSTEM._CD.Region = jsonSettings [ "CD" ] [ "Region" ];

	// SPU //

	_SYSTEM._SPU.AudioOutput_Enabled = jsonSettings [ "SPU" ] [ "Enable_AudioOutput" ];
	_SYSTEM._SPU.AudioFilter_Enabled = jsonSettings [ "SPU" ] [ "Enable_Filter" ];
	_SYSTEM._SPU.NextPlayBuffer_Size = jsonSettings [ "SPU" ] [ "BufferSize" ];
	_SYSTEM._SPU.GlobalVolume = jsonSettings [ "SPU" ] [ "GlobalVolume" ];

	// CPU //

	_SYSTEM._CPU.bEnableRecompiler = jsonSettings [ "CPU" ] [ "R3000A" ] [ "EnableRecompiler" ];

	// GPU //

	_SYSTEM._GPU.Set_Scanline (  jsonSettings [ "GPU" ] [ "ScanlineEnable" ] );
	_SYSTEM._GPU.ulNumberOfThreads = jsonSettings [ "GPU" ] [ "Threads" ];

	// Interface //

	sLastBiosPath = jsonSettings["Interface"]["LastBiosPath"];

	// should probably also go ahead and make an attempt to load the last bios file here also if it exists
	if (sLastBiosPath.compare(""))
	{
		LoadBIOS(sLastBiosPath);
	}

}


void hps1x64::SaveConfig ( string ConfigFileName )
{

	// save configuration as json (using nlohmann json)
	json jsonSettings;


	// save the controller settings //

	// pad 1 //

	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "DigitalAnalog" ] = _SYSTEM._SIO.ControlPad_Type [ 0 ];

	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyX" ] = _SYSTEM._SIO.Key_X [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyO" ] = _SYSTEM._SIO.Key_O [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyTriangle" ] = _SYSTEM._SIO.Key_Triangle [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeySquare" ] = _SYSTEM._SIO.Key_Square [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyR1" ] = _SYSTEM._SIO.Key_R1 [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyR2" ] = _SYSTEM._SIO.Key_R2 [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyR3" ] = _SYSTEM._SIO.Key_R3 [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyL1" ] = _SYSTEM._SIO.Key_L1 [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyL2" ] = _SYSTEM._SIO.Key_L2 [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyL3" ] = _SYSTEM._SIO.Key_L3 [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyStart" ] = _SYSTEM._SIO.Key_Start [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeySelect" ] = _SYSTEM._SIO.Key_Select [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyLeftAnalogX" ] = _SYSTEM._SIO.LeftAnalog_X [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyLeftAnalogY" ] = _SYSTEM._SIO.LeftAnalog_Y [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyRightAnalogX" ] = _SYSTEM._SIO.RightAnalog_X [ 0 ];
	jsonSettings [ "Controllers" ] [ "Pad1" ] [ "KeyRightAnalogY" ] = _SYSTEM._SIO.RightAnalog_Y [ 0 ];

	// pad 2 //

	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "DigitalAnalog" ] = _SYSTEM._SIO.ControlPad_Type [ 1 ];

	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyX" ] = _SYSTEM._SIO.Key_X [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyO" ] = _SYSTEM._SIO.Key_O [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyTriangle" ] = _SYSTEM._SIO.Key_Triangle [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeySquare" ] = _SYSTEM._SIO.Key_Square [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyR1" ] = _SYSTEM._SIO.Key_R1 [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyR2" ] = _SYSTEM._SIO.Key_R2 [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyR3" ] = _SYSTEM._SIO.Key_R3 [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyL1" ] = _SYSTEM._SIO.Key_L1 [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyL2" ] = _SYSTEM._SIO.Key_L2 [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyL3" ] = _SYSTEM._SIO.Key_L3 [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyStart" ] = _SYSTEM._SIO.Key_Start [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeySelect" ] = _SYSTEM._SIO.Key_Select [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyLeftAnalogX" ] = _SYSTEM._SIO.LeftAnalog_X [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyLeftAnalogY" ] = _SYSTEM._SIO.LeftAnalog_Y [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyRightAnalogX" ] = _SYSTEM._SIO.RightAnalog_X [ 1 ];
	jsonSettings [ "Controllers" ] [ "Pad2" ] [ "KeyRightAnalogY" ] = _SYSTEM._SIO.RightAnalog_Y [ 1 ];

	// memory cards //

	jsonSettings [ "Controllers" ] [ "MemoryCard1" ] [ "Disconnected" ] = _SYSTEM._SIO.MemoryCard_ConnectionState [ 0 ];
	jsonSettings [ "Controllers" ] [ "MemoryCard2" ] [ "Disconnected" ] = _SYSTEM._SIO.MemoryCard_ConnectionState [ 1 ];

	// device settings //

	// CD //

	jsonSettings [ "CD" ] [ "Region" ] = _SYSTEM._CD.Region;

	// SPU //

	jsonSettings [ "SPU" ] [ "Enable_AudioOutput" ] = _SYSTEM._SPU.AudioOutput_Enabled;
	jsonSettings [ "SPU" ] [ "Enable_Filter" ] = _SYSTEM._SPU.AudioFilter_Enabled;
	jsonSettings [ "SPU" ] [ "BufferSize" ] = _SYSTEM._SPU.NextPlayBuffer_Size;
	jsonSettings [ "SPU" ] [ "GlobalVolume" ] = _SYSTEM._SPU.GlobalVolume;

	// CPU //

	jsonSettings [ "CPU" ] [ "R3000A" ] [ "EnableRecompiler" ] = _SYSTEM._CPU.bEnableRecompiler;

	// GPU //

	jsonSettings [ "GPU" ] [ "ScanlineEnable" ] = _SYSTEM._GPU.Get_Scanline ();
	jsonSettings [ "GPU" ] [ "Threads" ] = _SYSTEM._GPU.ulNumberOfThreads;

	// Interface //

	jsonSettings["Interface"]["LastBiosPath"] = sLastBiosPath;


	// write the settings to file as json //

	std::ofstream o( ConfigFileName );
	o << std::setw(4) << jsonSettings << std::endl;	

}



void hps1x64::DebugWindow_Update ()
{
	// can't do anything if they've clicked on the menu
	WindowClass::Window::WaitForModalMenuLoop ();
	
	_SYSTEM._CPU.DebugWindow_Update ();
	_SYSTEM._BUS.DebugWindow_Update ();
	_SYSTEM._DMA.DebugWindow_Update ();
	_SYSTEM._TIMERS.DebugWindow_Update ();
	_SYSTEM._SPU.DebugWindow_Update ();
	_SYSTEM._GPU.DebugWindow_Update ();
	_SYSTEM._CD.DebugWindow_Update ();
}






WindowClass::Window *Dialog_KeyConfigure::wDialog;

WindowClass::Button *Dialog_KeyConfigure::CmdButtonOk, *Dialog_KeyConfigure::CmdButtonCancel;
WindowClass::Button* Dialog_KeyConfigure::KeyButtons [ c_iDialog_NumberOfButtons ];

WindowClass::Static *Dialog_KeyConfigure::InfoLabel;
WindowClass::Static* Dialog_KeyConfigure::KeyLabels [ c_iDialog_NumberOfButtons ];

u32 Dialog_KeyConfigure::isDialogShowing;
volatile s32 Dialog_KeyConfigure::ButtonClick;

u32 Dialog_KeyConfigure::KeyConfigure [ c_iDialog_NumberOfButtons ];


int Dialog_KeyConfigure::population_count64(unsigned long long w)
{
    w -= (w >> 1) & 0x5555555555555555ULL;
    w = (w & 0x3333333333333333ULL) + ((w >> 2) & 0x3333333333333333ULL);
    w = (w + (w >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
    return int((w * 0x0101010101010101ULL) >> 56);
}


int Dialog_KeyConfigure::bit_scan_lsb ( unsigned long v )
{
	//unsigned int v;  // find the number of trailing zeros in 32-bit v 
	int r;           // result goes here
	static const int MultiplyDeBruijnBitPosition[32] = 
	{
	  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
	  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};
	r = MultiplyDeBruijnBitPosition[((unsigned long)((v & -v) * 0x077CB531U)) >> 27];
	return r;
}

bool Dialog_KeyConfigure::Show_ConfigureKeysDialog ( int iPadNum )
{
	static const char* Dialog_Caption = "Configure Keys";
	static const int Dialog_Id = 0x6000;
	static const int Dialog_X = 10;
	static const int Dialog_Y = 10;

	static const char* Label1_Caption = "Instructions: Hold down the button on the joypad, and then click the PS button you want to assign it to (while still holding the button down). For analog sticks, hold the stick in that direction (x or y) and then click on the button to assign that axis.";
	static const int Label1_Id = 0x6001;
	static const int Label1_X = 10;
	static const int Label1_Y = 10;
	static const int Label1_Width = 300;
	static const int Label1_Height = 100;
	
	static const int c_iButtonArea_StartId = 0x6100;
	static const int c_iButtonArea_StartX = 10;
	static const int c_iButtonArea_StartY = Label1_Y + Label1_Height + 10;
	static const int c_iButtonArea_ButtonHeight = 20;
	static const int c_iButtonArea_ButtonWidth = 100;
	static const int c_iButtonArea_ButtonPitch = c_iButtonArea_ButtonHeight + 5;

	static const int c_iLabelArea_StartId = 0x6200;
	static const int c_iLabelArea_StartX = c_iButtonArea_StartX + c_iButtonArea_ButtonWidth + 10;
	static const int c_iLabelArea_StartY = c_iButtonArea_StartY;
	static const int c_iLabelArea_LabelHeight = c_iButtonArea_ButtonHeight;
	static const int c_iLabelArea_LabelWidth = 100;
	static const int c_iLabelArea_LabelPitch = c_iLabelArea_LabelHeight + 5;

	
	static const char* CmdButtonOk_Caption = "OK";
	static const int CmdButtonOk_Id = 0x6300;
	static const int CmdButtonOk_X = 10;
	static const int CmdButtonOk_Y = c_iButtonArea_StartY + ( c_iButtonArea_ButtonPitch * c_iDialog_NumberOfButtons ) + 10;
	static const int CmdButtonOk_Width = 50;
	static const int CmdButtonOk_Height = 20;
	
	static const char* CmdButtonCancel_Caption = "Cancel";
	static const int CmdButtonCancel_Id = 0x6400;
	static const int CmdButtonCancel_X = CmdButtonOk_X + CmdButtonOk_Width + 10;
	static const int CmdButtonCancel_Y = CmdButtonOk_Y;
	static const int CmdButtonCancel_Width = 50;
	static const int CmdButtonCancel_Height = 20;
	
	// now set width and height of dialog
	static const int Dialog_Width = Label1_Width + 20;	//c_iLabelArea_StartX + c_iLabelArea_LabelWidth + 10;
	static const int Dialog_Height = CmdButtonOk_Y + CmdButtonOk_Height + 30;
		
	static const char* PS1_Keys [] = { "X", "O", "Triangle", "Square", "R1", "R2", "R3", "L1", "L2", "L3", "Start", "Select", "Left Analog X", "Left Analog Y", "Right Analog X", "Right Analog Y" };
	static const char* Axis_Labels [] = { "Axis X", "Axis Y", "Axis Z", "Axis R", "Axis U", "Axis V" };
	
	bool ret;
	int iKeyIdx;
	
	//u32 *Key_X, *Key_O, *Key_Triangle, *Key_Square, *Key_R1, *Key_R2, *Key_R3, *Key_L1, *Key_L2, *Key_L3, *Key_Start, *Key_Select, *LeftAnalogX, *LeftAnalogY, *RightAnalogX, *RightAnalogY;
	//u32* Key_Pointers [ c_iDialog_NumberOfButtons ];

	stringstream ss;
	
	Joysticks j;
	
	/*
	Key_Pointers [ 0 ] = &_SYSTEM._SIO.Key_X;
	Key_Pointers [ 1 ] = &_SYSTEM._SIO.Key_O;
	Key_Pointers [ 2 ] = &_SYSTEM._SIO.Key_Triangle;
	Key_Pointers [ 3 ] = &_SYSTEM._SIO.Key_Square;
	Key_Pointers [ 4 ] = &_SYSTEM._SIO.Key_R1;
	Key_Pointers [ 5 ] = &_SYSTEM._SIO.Key_R2;
	Key_Pointers [ 6 ] = &_SYSTEM._SIO.Key_R3;
	Key_Pointers [ 7 ] = &_SYSTEM._SIO.Key_L1;
	Key_Pointers [ 8 ] = &_SYSTEM._SIO.Key_L2;
	Key_Pointers [ 9 ] = &_SYSTEM._SIO.Key_L3;
	Key_Pointers [ 10 ] = &_SYSTEM._SIO.Key_Triangle;
	Key_Pointers [ 11 ] = &_SYSTEM._SIO.Key_Square;
	Key_Pointers [ 12 ] = &_SYSTEM._SIO.LeftAnalog_X;
	Key_Pointers [ 13 ] = &_SYSTEM._SIO.LeftAnalog_Y;
	Key_Pointers [ 14 ] = &_SYSTEM._SIO.RightAnalog_X;
	Key_Pointers [ 15 ] = &_SYSTEM._SIO.RightAnalog_Y;
	*/

	//if ( !isDialogShowing )
	//{
		//x64ThreadSafe::Utilities::Lock_Exchange32 ( (long&) isDialogShowing, true );

		// set the events to use on call back
		//OnClick_Ok = _OnClick_Ok;
		//OnClick_Cancel = _OnClick_Cancel;
		
		// this is the value list that was double clicked on
		// now show a window where the variable can be modified
		// *note* setting the parent to the list-view control
		cout << "\nAllocating dialog";
		wDialog = new WindowClass::Window ();
		
		cout << "\nCreating dialog";
		wDialog->Create ( Dialog_Caption, Dialog_X, Dialog_Y, Dialog_Width, Dialog_Height, WindowClass::Window::DefaultStyle, NULL, hps1x64::ProgramWindow->hWnd );
		wDialog->DisableCloseButton ();
		
		// disable parent window
		cout << "\nDisable parent window";
		hps1x64::ProgramWindow->Disable ();
		
		//cout << "\nCreating static control";
		InfoLabel = new WindowClass::Static ();
		InfoLabel->Create_Text ( wDialog, Label1_X, Label1_Y, Label1_Width, Label1_Height, (char*) Label1_Caption, Label1_Id );
		
		// create the buttons and labels
		cout << "\nAdding buttons and labels.";
		for ( int i = 0; i < c_iDialog_NumberOfButtons; i++ )
		{
			// clear temp string
			//ss.str ( "" );
			
			// get name for label
			/*
			if ( i < 12 )
			{
				// label is for button //
				ss << "Button#" << bit_scan_lsb ( *Key_Pointers [ i ] );
			}
			else
			{
				// label is for analog stick //
				ss << Axis_Labels [ *Key_Pointers [ i ] ];
			}
			*/
			
			// put in a static label for entering a new value
			KeyLabels [ i ] = new WindowClass::Static ();
			KeyLabels [ i ]->Create_Text ( wDialog, c_iLabelArea_StartX, c_iLabelArea_StartY + ( i * c_iLabelArea_LabelPitch ), c_iLabelArea_LabelWidth, c_iLabelArea_LabelHeight, (char*) "test" /*ss.str().c_str()*/, c_iLabelArea_StartId + i );

			// put in a button
			KeyButtons [ i ] = new WindowClass::Button ();
			KeyButtons [ i ]->Create_CmdButton( wDialog, c_iButtonArea_StartX, c_iButtonArea_StartY + ( i * c_iButtonArea_ButtonPitch ), c_iButtonArea_ButtonWidth, c_iButtonArea_ButtonHeight, (char*) PS1_Keys [ i ], c_iButtonArea_StartId + i );
			
			// add event for ok button
			KeyButtons [ i ]->AddEvent ( WM_COMMAND, ConfigureDialog_AnyClick );
		}
		
		// put in an ok button
		CmdButtonOk = new WindowClass::Button ();
		CmdButtonOk->Create_CmdButton( wDialog, CmdButtonOk_X, CmdButtonOk_Y, CmdButtonOk_Width, CmdButtonOk_Height, (char*) CmdButtonOk_Caption, CmdButtonOk_Id );
		
		// add event for ok button
		CmdButtonOk->AddEvent ( WM_COMMAND, ConfigureDialog_AnyClick );
		
		// put in an cancel button
		CmdButtonCancel = new WindowClass::Button ();
		CmdButtonCancel->Create_CmdButton( wDialog, CmdButtonCancel_X, CmdButtonCancel_Y, CmdButtonCancel_Width, CmdButtonCancel_Height, (char*) CmdButtonCancel_Caption, CmdButtonCancel_Id );
		
		// add event for cancel button
		CmdButtonCancel->AddEvent ( WM_COMMAND, ConfigureDialog_AnyClick );
		
		// refresh keys
		Refresh ();
		
		ButtonClick = 0;
		
#ifndef ENABLE_DIRECT_INPUT
		j.InitJoysticks ();
#endif

		if ( _HPS1X64._SYSTEM._SIO.PortMapping [ iPadNum ] == -1 )
		{
			_HPS1X64._SYSTEM._SIO.PortMapping [ iPadNum ] = iPadNum;
		}
		
		while ( ButtonClick != CmdButtonOk_Id && ButtonClick != CmdButtonCancel_Id )
		{
			Sleep ( 10 );
			WindowClass::DoEvents ();
			
#ifdef ENABLE_DIRECT_INPUT
			SIO::DJoy.Update( _HPS1X64._SYSTEM._SIO.PortMapping [ iPadNum ] );
#else
			// read first joystick for now
			j.ReadJoystick ( _HPS1X64._SYSTEM._SIO.PortMapping [ iPadNum ] );
#endif
			
			//if ( ButtonClick != CmdButtonOk_Id && ButtonClick != CmdButtonCancel_Id && ButtonClick != 0 )
			//{
				if ( ( ButtonClick & 0xff00 ) == c_iButtonArea_StartId )
				{
					// read first joystick for now
					//cout << "\nreading joystick.";
					//j.ReadJoystick ( 0 );
					//j.ReadJoystick ( 0 );
					
					if ( ( ButtonClick & 0xff ) < 12 )
					{
#ifdef ENABLE_DIRECT_INPUT
						// get index of key that is pressed
						for ( iKeyIdx = 0; iKeyIdx < 32; iKeyIdx++ )
						{
							if ( SIO::DJoy.gameControllerStates[0].rgbButtons[iKeyIdx] )
							{
								break;
							}
						}
						
						
						if ( iKeyIdx < 32 )
						{
							KeyConfigure [ ButtonClick & 0xff ] = iKeyIdx;
						}
#else
						//cout << "\nchecking button NOT analog.";
						// check for button //
						if ( population_count64 ( j.joyinfo[0].dwButtons ) == 1 )
						{
							KeyConfigure [ ButtonClick & 0xff ] = j.joyinfo[0].dwButtons;
						}
#endif
					}
					else
					{
#ifdef ENABLE_DIRECT_INPUT
						
						u32 axis_value [ 6 ];
						u32 max_index;
						
						long *pData = (long*) & SIO::DJoy.gameControllerStates[0];
						
						//cout << "\n";
						for ( int i = 0; i < 6; i++ )
						{
							//cout << hex << Axis_Labels [ i ] << "=" << ((s32*)(&j.joyinfo.dwXpos)) [ i ] << " ";
							if ( pData [ i ] )
							{
								axis_value [ i ] = _Abs ( pData [ i ] - 0x80 );
							}
							else
							{
								axis_value [ i ] = 0;
							}
						}
						
						// check which axis value is greatest
						max_index = -1;
						for ( int i = 0; i < 6; i++ )
						{
							if ( axis_value [ i ] >= 75 )
							{
								max_index = i;
								break;
							}
						}
						
						// store axis
						if ( max_index != -1 ) KeyConfigure [ ButtonClick & 0xff ] = max_index;
						
#else
						//cout << "\nchecking analog.";
						// check for analog //
						// *** todo ***
						
						u32 axis_value [ 6 ];
						u32 max_index;
						
						//cout << "\n";
						for ( int i = 0; i < 6; i++ )
						{
							//cout << hex << Axis_Labels [ i ] << "=" << ((s32*)(&j.joyinfo.dwXpos)) [ i ] << " ";
							axis_value [ i ] = _Abs ( ((s32*)(&j.joyinfo[0].dwXpos)) [ i ] - 0x7fff );
						}
						
						// check which axis value is greatest
						max_index = -1;
						for ( int i = 0; i < 6; i++ )
						{
							if ( axis_value [ i ] >= 0x7000 )
							{
								max_index = i;
								break;
							}
						}
						
						// store axis
						if ( max_index != -1 ) KeyConfigure [ ButtonClick & 0xff ] = max_index;
#endif
					}
					
					// clear last button click
					ButtonClick = 0;
					
					Refresh ();
				}
				
				
			//}
		}
		
		ret = false;
		
		if ( ButtonClick == CmdButtonOk_Id )
		{
			ret = true;
		}
				
		hps1x64::ProgramWindow->Enable ();
		delete wDialog;
	//}
	
	return ret;
}


void Dialog_KeyConfigure::Refresh ()
{
	static const char* Axis_Labels [] = { "Axis X", "Axis Y", "Axis Z", "Axis R", "Axis U", "Axis V" };
	
	stringstream ss;
	
	for ( int i = 0; i < c_iDialog_NumberOfButtons; i++ )
	{
		// clear temp string
		ss.str ( "" );
		
		// get name for label
		if ( i < 12 )
		{
#ifdef ENABLE_DIRECT_INPUT
			ss << "Button#" << KeyConfigure [ i ];
#else
			// label is for button //
			ss << "Button#" << bit_scan_lsb ( KeyConfigure [ i ] );
#endif
		}
		else
		{
			// label is for analog stick //
			ss << Axis_Labels [ KeyConfigure [ i ] ];
		}
		
		// put in a static label for entering a new value
		KeyLabels [ i ]->SetText ( (char*) ss.str().c_str() );
	}
}

void Dialog_KeyConfigure::ConfigureDialog_AnyClick ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam )
{
	int i;
	HWND Parent_hWnd;
	
	cout << "\nClicked on a button. idCtrl=" << dec << idCtrl;
	
	x64ThreadSafe::Utilities::Lock_Exchange32 ( (long&) ButtonClick, idCtrl );

	/*
	if ( idCtrl > 6
	
	//cout << "\nClicked the OK button";
	
	// get the handle for the parent window
	Parent_hWnd = WindowClass::Window::GetHandleToParent ( hCtrl );
	
	//cout << "\nParent Window #1=" << (unsigned long) Parent_hWnd;
	
	i = FindInputBoxIndex ( Parent_hWnd );
	
	if ( i < 0 ) return;
	
	ListOfInputBoxes [ i ]->ReturnValue = ListOfInputBoxes [ i ]->editBox1->GetText ();
	if ( ListOfInputBoxes [ i ]->OnClick_Ok ) ListOfInputBoxes [ i ]->OnClick_Ok ( ListOfInputBoxes [ i ]->editBox1->GetText () );
	ListOfInputBoxes [ i ]->KillDialog ();
	*/
}


