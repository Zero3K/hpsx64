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




#include "ps1_system.h"


namespace Playstation1
{


	class hps1x64
	{
	public:
	
		static const unsigned long long CyclesToRunContinuous = 500000;

		static const int ProgramWindow_X = 10;
		static const int ProgramWindow_Y = 10;
		//static const int ProgramWindow_Width = GPU::c_iScreenOutput_MaxWidth;	//640;
		//static const int ProgramWindow_Height = GPU::c_iScreenOutput_MaxHeight;	//480;
		static const int ProgramWindow_Width = 640;
		static const int ProgramWindow_Height = 480;
		
		
		static WindowClass::Window *ProgramWindow;

		System _SYSTEM;
		
		// this can't be static
		u32 bEnableVsync;
		
		union MenuClicked
		{
			struct
			{
				u64 File_Load_State : 1;
				u64 File_Load_BIOS : 1;
				u64 File_Load_GameDisk : 1;
				u64 File_Load_AudioDisk : 1;
				
				u64 File_Save_State : 1;
				
				u64 File_Reset : 1;
				u64 File_Run : 1;
				u64 File_Exit : 1;
				
				u64 Debug_Break : 1;
				u64 Debug_StepInto : 1;

				u64 Debug_ShowWindow_All : 1;
				u64 Debug_ShowWindow_FrameBuffer : 1;
				u64 Debug_ShowWindow_R3000A : 1;
				u64 Debug_ShowWindow_Memory : 1;
				u64 Debug_ShowWindow_DMA : 1;
				u64 Debug_ShowWindow_TIMER : 1;
				u64 Debug_ShowWindow_SPU : 1;
				u64 Debug_ShowWindow_INTC : 1;
				u64 Debug_ShowWindow_GPU : 1;
				u64 Debug_ShowWindow_MDEC : 1;
				u64 Debug_ShowWindow_SIO : 1;
				u64 Debug_ShowWindow_PIO : 1;
				u64 Debug_ShowWindow_CD : 1;
				u64 Debug_ShowWindow_BUS : 1;
				u64 Debug_ShowWindow_ICACHE : 1;
				
				u64 Controllers_Configure : 1;
				u64 Pad1Type_Digital : 1;
				u64 Pad1Type_Analog : 1;
				u64 Pad2Type_Digital : 1;
				u64 Pad2Type_Analog : 1;
				
				u64 MemoryCard1_Connected : 1;
				u64 MemoryCard1_Disconnected : 1;
				u64 MemoryCard2_Connected : 1;
				u64 MemoryCard2_Disconnected : 1;
				
				u64 Region_Europe : 1;
				u64 Region_Japan : 1;
				u64 Region_NorthAmerica : 1;
				
				u64 Audio_Enable : 1;
				u64 Audio_Volume_100 : 1;
				u64 Audio_Volume_75 : 1;
				u64 Audio_Volume_50 : 1;
				u64 Audio_Volume_25 : 1;
				u64 Audio_Buffer_8k : 1;
				u64 Audio_Buffer_16k : 1;
				u64 Audio_Buffer_32k : 1;
				u64 Audio_Buffer_64k : 1;
				u64 Audio_Buffer_1m : 1;
				u64 Audio_Filter : 1;
				
				u64 Video_FullScreen : 1;
			};
			
			u64 Value;
		};
		
		static volatile u32 _MenuClick;
		
		union RunMode
		{
			struct
			{
				u64 RunNormal : 1;
				u64 RunDebug : 1;
				u64 Exit : 1;
			};
			
			u64 Value;
		};
		
		static volatile RunMode _RunMode;
		
	public:
	
		// constructor
		hps1x64 ();
		
		// destructor
		~hps1x64 ();
		
		// reset application
		void Reset ();
	
		void Update_CheckMarksOnMenu ();
		
		int InitializeProgram ();

		int RunProgram ();
		
		int HandleMenuClick ();
		
		void DrawFrameToProgramWindow ();

		void SaveState ( string FilePath = "" );
		void LoadState ( string FilePath = "" );
		void LoadBIOS ( string FilePath = "" );
		string LoadDisk ( string FilePath = "" );
		
		void LoadConfig ( string ConfigFileName );
		void SaveConfig ( string ConfigFileName );
		
		static const int c_iExeMaxPathLength = 2048;
		static string ExecutablePath;
		static string BiosPath;
		
		// events for menu items //
		
		static void OnClick_Debug_Break ( int i );
		static void OnClick_Debug_StepInto ( int i );
		static void OnClick_Debug_OutputCurrentSector ( int i );
		static void OnClick_Debug_Show_All ( int i );
		static void OnClick_Debug_Show_FrameBuffer ( int i );
		static void OnClick_Debug_Show_R3000A ( int i );
		static void OnClick_Debug_Show_Memory ( int i );
		static void OnClick_Debug_Show_DMA ( int i );
		static void OnClick_Debug_Show_TIMER ( int i );
		static void OnClick_Debug_Show_SPU ( int i );
		static void OnClick_Debug_Show_CD ( int i );
		static void OnClick_Debug_Show_INTC ( int i );

		static void OnClick_File_Load_State ( int i );
		static void OnClick_File_Load_BIOS ( int i );
		static void OnClick_File_Load_GameDisk ( int i );
		static void OnClick_File_Load_AudioDisk ( int i );
		static void OnClick_File_Save_State ( int i );
		static void OnClick_File_Reset ( int i );
		static void OnClick_File_Run ( int i );
		static void OnClick_File_Exit ( int i );
		
		static void OnClick_Controllers0_Configure ( int i );
		static void OnClick_Controllers1_Configure ( int i );
		static void OnClick_Pad1Type_Digital ( int i );
		static void OnClick_Pad1Type_Analog ( int i );
		static void OnClick_Pad2Type_Digital ( int i );
		static void OnClick_Pad2Type_Analog ( int i );

		static void OnClick_Pad1Input_None ( int i );
		static void OnClick_Pad1Input_Device0 ( int i );
		static void OnClick_Pad1Input_Device1 ( int i );
		static void OnClick_Pad2Input_None ( int i );
		static void OnClick_Pad2Input_Device0 ( int i );
		static void OnClick_Pad2Input_Device1 ( int i );
		
		static void OnClick_Card1_Connect ( int i );
		static void OnClick_Card1_Disconnect ( int i );
		static void OnClick_Card2_Connect ( int i );
		static void OnClick_Card2_Disconnect ( int i );
		
		static void OnClick_Redetect_Pads ( int i );
		
		static void OnClick_Region_Europe ( int i );
		static void OnClick_Region_Japan ( int i );
		static void OnClick_Region_NorthAmerica ( int i );
		
		static void OnClick_Audio_Enable ( int i );
		static void OnClick_Audio_Volume_100 ( int i );
		static void OnClick_Audio_Volume_75 ( int i );
		static void OnClick_Audio_Volume_50 ( int i );
		static void OnClick_Audio_Volume_25 ( int i );
		static void OnClick_Audio_Buffer_8k ( int i );
		static void OnClick_Audio_Buffer_16k ( int i );
		static void OnClick_Audio_Buffer_32k ( int i );
		static void OnClick_Audio_Buffer_64k ( int i );
		static void OnClick_Audio_Buffer_1m ( int i );
		static void OnClick_Audio_Filter ( int i );
		
		static void OnClick_Video_ScanlinesEnable ( int i );
		static void OnClick_Video_ScanlinesDisable ( int i );
		static void OnClick_Video_WindowSizeX1 ( int i );
		static void OnClick_Video_WindowSizeX15 ( int i );
		static void OnClick_Video_WindowSizeX2 ( int i );
		static void OnClick_Video_FullScreen ( int i );
		
		static void OnClick_R3000ACPU_Interpreter ( int i );
		static void OnClick_R3000ACPU_Recompiler ( int i );
		static void OnClick_R3000ACPU_Recompiler2 ( int i );
		
		
		static void OnClick_GPU_0Threads ( int i );
		static void OnClick_GPU_1Threads ( int i );
		
		static void OnClick_GPU_Software ( int i );
		static void OnClick_GPU_Hardware ( int i );
		
		static void LoadClick ( int i );
		static void SaveStateClick ( int i );
		static void LoadStateClick ( int i );
		static void LoadBiosClick ( int i );
		static void StartClick ( int i );
		static void StopClick ( int i );
		static void StepInstructionClick ( int i );
		//static void ExitClick ( int i );
		static void SaveBIOSClick ( int i );
		static void SaveRAMClick ( int i );
		static void SetBreakPointClick ( int i );
		static void SetCycleBreakPointClick ( int i );
		static void SetAddressBreakPointClick ( int i );
		static void SetValueClick ( int i );
		static void SetMemoryClick ( int i );

		
		void DebugWindow_Update ();
	};

	
	class Dialog_KeyConfigure
	{
	public:
		static const int c_iDialog_NumberOfButtons = 16;
		
		static WindowClass::Window *wDialog;
		
		static WindowClass::Button *CmdButtonOk, *CmdButtonCancel;
		static WindowClass::Button* KeyButtons [ c_iDialog_NumberOfButtons ];
		
		static WindowClass::Static *InfoLabel;
		static WindowClass::Static* KeyLabels [ c_iDialog_NumberOfButtons ];
		
		static u32 isDialogShowing;
		static volatile s32 ButtonClick;
		
		static u32 KeyConfigure [ c_iDialog_NumberOfButtons ];
		
		static inline u32 _Abs ( s32 Value )
		{
			return ( ( Value >> 31 ) ^ Value ) - ( Value >> 31 );
		}
		
		static int population_count64(unsigned long long w);
		static int bit_scan_lsb ( unsigned long v );
		
		// returns true if they want to keep settings, false when the dialog was cancelled
		static bool Show_ConfigureKeysDialog ( int iPadNum );
		
		static void ConfigureDialog_AnyClick ( HWND hCtrl, int idCtrl, unsigned int message, WPARAM wParam, LPARAM lParam );
		
		static void Refresh ();
	};
}

