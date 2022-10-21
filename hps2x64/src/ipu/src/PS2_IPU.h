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



#ifndef _PS2_IPU_H_
#define _PS2_IPU_H_

#include "types.h"
#include "Debug.h"

#include "DebugValueList.h"

// include of modified pcsx2 mpeg code
#include "Mpeg.h"


//#include "PS1_Intc.h"

//#include <stdio.h>
//#include <sys/stat.h>



namespace Playstation2
{

	class IPU
	{
	
	public:
	
		static Debug::Log debug;
		
	
		static IPU *_IPU;
		
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the dma registers start at
		static const long Regs_Start = 0x10002000;
		
		// where the dma registers end at
		static const long Regs_End = 0x10002030;
	
		// distance between numbered groups of registers for dma
		static const long Reg_Size = 0x4;

		static const char* RegNames [ 4 ];

		
		// ???
		u32 testvar;
		
		// cycle that the next event will happen at for this device
		u64 NextEvent_Cycle;

		
		
		void GetNextEvent ();
		void SetNextEvent ( u64 Cycle );
		void Set_NextEventCycle ( u64 Cycle );
		void Update_NextEventCycle ();
		
		static u64 Read ( u32 Address, u64 Mask );
		static void Write ( u32 Address, u64 Data, u64 Mask );
		
		void Reset ();
		
		void Start ();
		
		void Run ();
		
		

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////

		// CMD
		static const long IPU_CMD = 0x10002000;
		
		// CTRL
		static const long IPU_CTRL = 0x10002010;
		
		// BP - control of input FIFO
		static const long IPU_BP = 0x10002020;
		
		// TOP - bit stream starting data
		static const long IPU_TOP = 0x10002030;
		
		// FIFO OUT
		static const long IPU_FIFOout = 0x10007000;

		// FIFO IN
		static const long IPU_FIFOin = 0x10007010;
		

		enum
		{
			CMD_BCLR = 0,
			CMD_IDEC = 1,
			CMD_BDEC = 2,
			CMD_VDEC = 3,
			CMD_FDEC = 4,
			CMD_SETIQ = 5,
			CMD_SETVQ = 6,
			CMD_CSC = 7,
			CMD_PACK = 8,
			CMD_SETTH = 9
		};


		enum
		{
			BCLR_CYCLES = 0,
			IDEC_CYCLES = 0,
			BDEC_CYCLES = 0,
			VDEC_CYCLES = 0,
			FDEC_CYCLES = 1024,
			SETIQ_CYCLES = 0,
			SETVQ_CYCLES = 0,
			CSC_CYCLES = 0,
			PACK_CYCLES = 0,
			SETTH_CYCLES = 0
		};

		
		union CMD_Write_t
		{
			struct
			{
				// bits 0-27 - Option - options for the command
				u64 OPTION : 28;
				
				// bits 28-31 - Code - the actual command code
				u64 CODE : 4;
			};
			
			struct
			{
				// bits 0-5 - FB - bits to move forward in bit stream
				u64 FB : 6;
				
				// bits 6-26 - filler
				u64 filler0 : 21;
				
				// bit 27 - IQM - (0: loading intra quatization matrix, 1: loading non-intra quantization matrix)
				u64 IQM : 1;
				
				// bits 28-31 - Code - the actual command code
				//u64 CODE : 4;
			};
			
			struct
			{
				// bits 0-8 - TH0 - transparent threshold value
				u64 TH0 : 9;
				
				// bits 9-15 - filler
				u64 filler1 : 7;
				
				// bits 16-24 - TH1 - translucent threshold value
				u64 TH1 : 9;
				
				// bit 25 - filler
				u64 filler4 : 1;
				
				// bits 26-27 - TBL
				u64 TBL : 2;
			};
			
			struct
			{
				// bits 0-6 - BP - set bit position
				u64 BP : 7;
				
				// bits 7-15 - filler
				u64 filler2 : 9;
				
				// bits 16-20 - QSC - Quantizer step code
				u64 QSC : 5;
				
				// bits 21-23 - filler
				u64 filler3 : 3;
				
				// bit 24 - DTD - DT Decode
				u64 DTD : 1;
				
				// bit 25 - SGN - sign
				u64 SGN : 1;
				
				// bit 26 - DTE - dither disable/enable
				u64 DTE : 1;
				
				// bit 27 - OFM - format of output (0: RGB32, 1: RGB16)
				u64 OFM : 1;
			};
			
			struct
			{
				// bits 0-10 - macroblock count
				u64 MBC : 11;
				
				// bits 11-24 - filler
				u64 filler5 : 14;
				
				// bit 25 - DT - DCT Type (0: Frame DCT, 1: Field DCT)
				u64 DT : 1;
				
				// bit 26 - DCR - DC Reset (0: Does not reset DC prediction value, 1: Resets DC prediction value)
				u64 DCR : 1;
				
				// bit 27 - MBI - Macroblock Intra (0: non-intra macroblock, 1: intra macroblock)
				u64 MBI : 1;
			};
			
			struct
			{
				u32 Lo;
				u32 Hi;
			};

			u64 Value;
		};
		
		CMD_Write_t CMD_Write;
		

		union CMD_Read_t
		{
			struct
			{
				// bits 0-31 - Data
				u64 DATA : 32;
				
				// bits 32-62 - unknown
				u64 unk0 : 31;
				
				// bit 63 - Busy - set when the command is busy
				u64 BUSY : 1;
			};
			
			struct
			{
				u32 Lo;
				u32 Hi;
			};

			u64 Value;
		};
		
		CMD_Read_t CMD_Read;


		// READ ONLY ??
		union TOP_t
		{
			struct
			{
				// bits 0-31 - BSTOP - Data
				// reading data from here does NOT advance the bit stream
				u64 BSTOP : 32;
				
				// bits 32-62 - unknown
				u64 unk0 : 31;
				
				// bit 63 - Busy - set when the command is busy
				u64 BUSY : 1;
			};

			struct
			{
				u32 Lo;
				u32 Hi;
			};
			
			u64 Value;
		};

		
		
		union CTRL_t
		{
			struct
			{
				// bits 0-3 - IFC - count of items in input fifo - READ ONLY
				u64 IFC : 4;
				
				// bits 4-7 - OFC - count of items in output fifo - READ ONLY
				u64 OFC : 4;
				
				// bits 8-13 - CBP - coded block pattern - READ ONLY
				u64 CBP : 6;
				
				// bit 14 - ECD - set if error code is detected - READ ONLY
				u64 ECD : 1;
				
				// bit 15 - SCD - set if start code is detected - READ ONLY
				u64 SCD : 1;
				
				// bits 16-17 - IDP - Intra DC precision (00: 8-bits, 01: 9-bits, 10: 10-bits, 11: Reserved) - R/W
				u64 IDP : 2;
				
				// bits 18-19 - unknown
				u64 unk0 : 2;
				
				// bit 20 - Alternate Scan (0: zig-zag scan, 1: alternate scan) - R/W
				u64 AS : 1;
				
				// bit 21 - Intra VLC Format - (0: MPEG1 compatible VLC table, 1: special VLC table) - R/W
				u64 IVF : 1;
				
				// bit 22 - Q scale step (0: linear step, 1: non-linear step) - R/W
				u64 QST : 1;
				
				// bit 23 - MP1 (0: mpeg2, 1: mpeg1) - R/W
				u64 MP1 : 1;
				
				// bits 24-26 - Picture Type - (000: Reserved, 001: I-picture, 010: P-picture, 011: B-picture, 100: D-picture) - R/W
				u64 PCT : 3;
				
				// bits 27-29 - unknown
				u64 unk1 : 3;
				
				// bit 30 - Reset - completely resets ipu - WRITE ONLY
				u64 RST : 1;
				
				// bit 31 - Busy - set if command is executing - READ ONLY
				// triggers an interrupt when this bit transitions from one to zero ?? (processing done?)
				u64 BUSY : 1;
			};

			struct
			{
				u32 Lo;
				u32 Hi;
			};

			u64 Value;
		};
		
		static const u64 CTRL_Write_Mask = 0xc000ffffULL;


		// READ ONLY
		union BP_t
		{
			struct
			{
				// bits 0-6 - bit stream ptr - bit location to start decoding at - READ ONLY
				u64 BP : 7;
				
				// bit 7 - unknown
				u64 unk0 : 1;
				
				// bits 8-11 - count of items in input fifo - READ ONLY
				u64 IFC : 4;
				
				// bits 12-15 - unknown
				u64 unk1 : 4;
				
				// bits 16-17 - fifo pointer - READ ONLY
				u64 FP : 2;
			};
			
			struct
			{
				u32 Lo;
				u32 Hi;
			};

			u64 Value;
		};

		
		
		// ipu registers
		static const int c_iNumRegs = 4;
		union Regs_t
		{
			u64 Regs [ c_iNumRegs ];
			
			struct
			{
				u64 CMD;	// register #0
				TOP_t TOP;
				CTRL_t CTRL;
				BP_t BP;
			};
		};
		
		Regs_t Regs;
		
		
		static constexpr double c_dCyclesPerByte = 1.0L;
		
		// amount of time to decode data
		// 1024 seems to cause problems?
		static const u64 c_llCyclesPerQW = 1;

		// fifo size of the device
		static const int c_iDevice_FifoSize = 8;
		
		static const long c_lMacroBlock_Size = 16;
		static const long c_lMacroBlock_Mask = c_lMacroBlock_Size - 1;
		
		// size of fifo for storing data needs to be a power of 2, but also needs to be at least fifosize of device +2
		static const int c_iFifoSize128 = 8 << 1;
		static const u32 c_iFifoMask128 = c_iFifoSize128 - 1;
		
		static const int c_iFifoSize64 = 16 << 1;
		static const u32 c_iFifoMask64 = c_iFifoSize64 - 1;
		
		// the dither array for IPU, with elements in a signed value with 4 integer bits and 1 fractional bit
		static const u32 c_uDitherArray_Size = 4;
		static const u32 c_uDitherArray_Mask = c_uDitherArray_Size - 1;
		static const s32 sDitherArray_4_1 [ 4 * 4 ];
		
		u32 MacroBlockX, MacroBlockY;
		
		u32 FifoIn_ReadIndex, FifoIn_WriteIndex, FifoOut_ReadIndex, FifoOut_WriteIndex;
		s32 FifoIn_Size, FifoOut_Size;
		u64 FifoIn [ c_iFifoSize64 ];
		u64 FifoOut [ c_iFifoSize64 ];
		
		u32 InputCount, OutputCount;
		
		// need to know how many macro blocks to convert
		s32 MacroBlock_Count;
		
		u32 CommandState;

		struct ipu_cmd_t
		{
			u32 pos [ 6 ];
			
			void clear () { pos [ 0 ] = 0; pos [ 1 ] = 0; pos [ 2 ] = 0; pos [ 3 ] = 0; pos [ 4 ] = 0; pos [ 5 ] = 0; }
		};
		
		// decoder object for modified pcsx2 mpeg decoder
		__aligned16 decoder_t thedecoder;

		ipu_cmd_t ipu_cmd;
		
		// intra quantization matrix
		u8 iq_table [ 8 * 8 ];
		
		// non-intra quantization matrix
		u8 niq_table [ 8 * 8 ];
		
		static u64 DataOut [ 2 ];
		
		// 16 pixel clut of 16-bit pixels set by setvq command
		static const int c_iVqClut_Size = 16;
		u16 vqclut [ c_iVqClut_Size ];
		
		u32 BitPosition;
		
		u32 TH0;
		u32 TH1;
		
		u64 BusyUntil_Cycle;

		static inline u32 EndianSwap32 ( u32 Value ) { return ( Value << 24 ) | ( Value >> 24 ) | ( ( Value << 8 ) & 0xff0000 ) | ( ( Value >> 8 ) & 0xff00 ); }
		static inline u64 EndianSwap64 ( u64 Value ) { return ( Value << 56 ) | ( Value >> 56 ) | ( ( Value << 40 ) & 0x00ff000000000000ull ) | ( ( Value >> 40 ) & 0xff00ull ) | ( ( Value << 24 ) & 0xff0000000000ull ) | ( ( Value >> 24 ) & 0xff0000ull ) | ( ( Value << 8 ) & 0xff00000000ull ) | ( ( Value >> 8 ) & 0xff000000ull ); }
		
		void Clear_InputFifo ();
		void Set_Busy ();
		void Clear_Busy ();

		
		void Proceed ( u32 iBits );
		u64 PeekBE ( u64 iBits, u32 uBitPosition );
		u64 ReadBE64(u32 uBitPosition);
		u64 ReadLE64(u32 uBitPosition);
		u64 Peek ( u64 iBits, u32 uBitPosition );
		u64 Get ( u32 iBits );
		bool Load_IQTable_FromBitstream ( u8* table );
		bool Load_VQTable_FromBitstream ( u16* table );
		
		// for interface with modified pcsx2 mpeg decoder
		static u32 UBITS ( u32 Bits );
		static s32 SBITS ( u32 Bits );
		static void DUMPBITS ( u32 Bits );

		void Update_IFC ();
		void Update_OFC ();

		static u64 DMA_Write_Ready ();
		static u64 DMA_Read_Ready ();
		
		static u32 DMA_WriteBlock ( u64* Data, u32 QuadwordCount );
		static u32 DMA_ReadBlock ( u64* Data, u32 QuadwordCount );

		bool Execute_CSC ();
		bool Execute_IDEC ();
		bool Execute_BDEC ();
		bool Execute_VDEC ();
		bool Execute_FDEC ();
		bool Execute_CMD ();
		
		void Process_CMD ();

		void Update_BP ();
		void Set_Output ( u32 Data );

		// check fifos for emptiness
		bool FifoIn_Empty ();
		bool FifoOut_Empty ();
		
		// RGB16 -> 4-bit index in CLUT conversion
		u8 VQ ( u16 RGB16 );
		
		// perform dither function on a pixel at x,y in macroblock
		u16 Dither ( u32 x, u32 y, u32 uPixel32 );

		// YCbCr -> r,g,b,a conversion
		u32 CSC ( u32 YCbCr );
		
		
		// for compatibility with pcsx2 decoding method
		u32 coded_block_pattern;
		
		
		u32 CurrentOp;
		enum { CURRENTOP_NONE = 0, CURRENTOP_OUTPUT = 1, CURRENTOP_INPUT = 2 };
		
		// constructor
		IPU ();
		
		// Debug
		static u32 *_DebugPC;
		static u64 *_DebugCycleCount;
		static u32* _NextEventIdx;
		

		// object debug stuff
		// Enable/Disable debug window for object
		// Windows API specific code is in here
		static bool DebugWindow_Enabled;
		static WindowClass::Window* DebugWindow;
		static DebugValueList<u32>* ValueList;
		static void DebugWindow_Enable();
		static void DebugWindow_Disable();
		static void DebugWindow_Update();

		

		static u64* _NextSystemEvent;

		// index for the next event
		u32 NextEvent_Idx;

		static void sRun () { _IPU->Run (); }
		static void Set_EventCallback ( funcVoid2 UpdateEvent_CB ) { _IPU->NextEvent_Idx = UpdateEvent_CB ( sRun ); };
		

		static const u32 c_InterruptBit = 8;
		static const u32 c_InterruptCpuNotifyBit = 0;
		
		
		static u32* _Intc_Stat;
		static u32* _Intc_Mask;
		static u32* _R5900_Status_12;
		static u32* _R5900_Cause_13;
		static u64* _ProcStatus;
		
		inline void ConnectInterrupt ( u32* _IStat, u32* _IMask, u32* _R5900_Status, u32* _R5900_Cause, u64* _ProcStat )
		{
			_Intc_Stat = _IStat;
			_Intc_Mask = _IMask;
			_R5900_Cause_13 = _R5900_Cause;
			_R5900_Status_12 = _R5900_Status;
			_ProcStatus = _ProcStat;
		}
		
		
		
		
		inline void SetInterrupt ()
		{
			*_Intc_Stat |= ( 1 << c_InterruptBit );
			if ( *_Intc_Stat & *_Intc_Mask ) *_R5900_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) )
			{
				*_ProcStatus |= ( 1 << c_InterruptCpuNotifyBit );
			}
			else
			{
				*_ProcStatus &= ~( 1 << c_InterruptCpuNotifyBit );
			}
		}
		
		/*
		inline void ClearInterrupt_SIO ()
		{
			// *_Intc_Stat &= ~( 1 << c_InterruptBit_SIO );
			//if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R5900_Cause_13 &= ~( 1 << 10 );
			
			//if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
		}
		*/
		
		
	};
	
};




u32 UBITS ( unsigned long Bits );
s32 SBITS ( unsigned long Bits );
void DUMPBITS ( unsigned long Bits );
u32 GETBITS(uint num);
bool RESERVEBITS ( uint num );
void ALIGN ();
u64 PEEKBITS ( uint num );
bool GETWORD ();
bool BITSTREAM_INIT ();



#endif

