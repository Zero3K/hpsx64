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


#ifndef _PS2DATABUS_H_
#define _PS2DATABUS_H_

#include "types.h"


#include "PS2_Dma.h"
#include "PS2_Gpu.h"
#include "PS2_Intc.h"
//#include "PS2_MDEC.h"
//#include "PS2_PIO.h"
#include "PS2_SIO.h"
#include "PS2_Timer.h"
#include "PS2_IPU.h"

//#include "R5900DebugPrint.h"

#ifdef ENABLE_GUI_DEBUGGER
#include "DebugMemoryViewer.h"
#endif


#define ENABLE_INVALID_ARRAY


//#define INLINE_DEBUG_GetIMemPtr



namespace Playstation2
{

	//class Dma;
	//class MDEC;

	class DataBus
	{
	
#ifdef ENABLE_GUI_DEBUGGER
		static Debug::Log debug;
#endif

		
	public:
	
		static DataBus *_BUS;
		
		
		// 0x1f80 0000 is where the hardware mapped registers start at for PS1
		// 0x1000 0000 is where the hardware mapped registers start at for PS2
		static const u32 HWRegisters_Start = 0x10000000;
		
		
		// cycle that the next event will happen at for this device
		// this will not actually be used here
		u64 NextEvent_Cycle;
		
		
		// PS2 Bus speed can be 147.456 MHZ in earlier models, and 149.5 MHZ in later models
		// R5900 runs at twice that speed
		// PS1 Bus speed when used with PS2 is 1/4 of that, unless in PS1 configuration
		static const u64 c_ClockSpeed1 = 147456000;
		static const u64 c_ClockSpeed2 = 149500000;


		// 0x0000 0000 is physical address of where the regular ram starts at
		static const u32 MainMemory_Start = 0x00000000;
		static const u32 MainMemory_Size = 0x2000000;	// 32 MB
		static const u32 MainMemory_Mask = MainMemory_Size - 1;
		
		// 0x1fc0 0000 is physical address of where BIOS starts at
		static const u32 BIOS_Start = 0x1fc00000;
		static const u32 BIOS_Size = 0x400000;	// 4MB
		static const u32 BIOS_Mask = BIOS_Size - 1;

		// 0x70000000 is physical address of where scratch pad starts at
		static const u32 ScratchPad_Start = 0x70000000;
		static const u32 ScratchPad_Size = 0x4000;	// 16 KB
		static const u32 ScratchPad_Mask = ScratchPad_Size - 1;

		// 0x11000000 is physical address of where micromem starts at
		static const u32 MicroMem0_Start = 0x11000000;
		static const u32 MicroMem0_Size = 0x1000;	// 4 KB
		static const u32 MicroMem0_Mask = MicroMem0_Size - 1;
		
		static const u32 VuMem0_Start = 0x11004000;
		static const u32 VuMem0_Size = 0x1000;	// 4 KB
		static const u32 VuMem0_Mask = VuMem0_Size - 1;

		
		static const u32 MicroMem1_Start = 0x11008000;
		static const u32 MicroMem1_Size = 0x4000;	// 16 KB
		static const u32 MicroMem1_Mask = MicroMem1_Size - 1;
		
		static const u32 VuMem1_Start = 0x1100c000;
		static const u32 VuMem1_Size = 0x4000;	// 16 KB
		static const u32 VuMem1_Mask = VuMem1_Size - 1;

		
		static const u32 DirectCacheMem_Start = 0xffff8000;
		static const u32 DirectCacheMem_Size = 0x2000;	//0x8000;	// 32 KB or 16 KB or 8 KB ?? (trying 32KB first)
		static const u32 DirectCacheMem_Mask = DirectCacheMem_Size - 1;
		
		
		// reading from BIOS takes 24 cycles (8-bit bus), executing an instruction would be an extra cycle
		static const int c_iBIOS_Read_Latency = 24;
		
		// reading from RAM takes 5 cycles, but if executing an instruction, that takes an additional 1 cycle to execute
		static const int c_iRAM_Read_Latency = 8;	//14;
		
		// reading from hardware registers takes around 3 cycles or less
		static const int c_iReg_Read_Latency = 3;
		
		// bus registers
		//u32 RamSize, CD_Delay, DMA_Delay;
		
		
		
		// lookup table for bus
		typedef u64 (*PS2_BusInterface_Read) ( u32 Address, u64 Mask );
		typedef void (*PS2_BusInterface_Write) ( u32 Address, u64 Data, u64 Mask );
		
		// I'll treat this one extra special
		typedef u64* (*PS2_BusInterface_Read128) ( u32 Address, u64 Mask );
		typedef void (*PS2_BusInterface_Write128) ( u32 Address, u64* Data, u64 Mask );
		
		static const u32 c_LUT_Bus_Size = 0x400;
		static PS2_BusInterface_Read LUT_BusRead [ c_LUT_Bus_Size ];
		static PS2_BusInterface_Write LUT_BusWrite [ c_LUT_Bus_Size ];

		//static PS2_BusInterface_Read128 LUT_BusRead128 [ c_LUT_Bus_Size ];
		//static PS2_BusInterface_Write128 LUT_BusWrite128 [ c_LUT_Bus_Size ];
		
		
		// new stuff for templates
		static PS2_BusInterface_Read LUT_BusRead8 [ c_LUT_Bus_Size ];
		static PS2_BusInterface_Write LUT_BusWrite8 [ c_LUT_Bus_Size ];
		static PS2_BusInterface_Read LUT_BusRead16 [ c_LUT_Bus_Size ];
		static PS2_BusInterface_Write LUT_BusWrite16 [ c_LUT_Bus_Size ];
		static PS2_BusInterface_Read LUT_BusRead32 [ c_LUT_Bus_Size ];
		static PS2_BusInterface_Write LUT_BusWrite32 [ c_LUT_Bus_Size ];
		static PS2_BusInterface_Read LUT_BusRead64 [ c_LUT_Bus_Size ];
		static PS2_BusInterface_Write LUT_BusWrite64 [ c_LUT_Bus_Size ];
		static PS2_BusInterface_Read LUT_BusRead128 [ c_LUT_Bus_Size ];
		static PS2_BusInterface_Write LUT_BusWrite128 [ c_LUT_Bus_Size ];

		
		struct DataBusEntry
		{
			u32* pMemoryDevice;
			u32 ulMask;
			u32 ulLatency;
			
			// no need for invalidate if it is a read ?
			u8* pInvalidateDevice;
			
			// reference to registers, and then mask will be for register
			u32** pRegPtrs;
		};

		alignas(16) static DataBusEntry LUT_DataBus_Read [ c_LUT_Bus_Size ];
		alignas(16) static DataBusEntry LUT_DataBus_Write [ c_LUT_Bus_Size ];
		
		
		// references to the registers for reading
		// registers are placed 16-bytes apart, so can shift down by 4
		alignas(16) static u32* pRegRefList0x1000_Read [ 65536 >> 4 ];
		alignas(16) static u32* pRegRefList0x1100_Read [ 65536 >> 4 ];
		alignas(16) static u32* pRegRefList0x1200_Read [ 65536 >> 4 ];
		
		
		// connect device with bus
		void Init_ConnectDevice ( void );
		void ConnectDevice_Read ( u32 AddressStart, u32 AddressEnd, PS2_BusInterface_Read CallbackFunction );
		void ConnectDevice_Write ( u32 AddressStart, u32 AddressEnd, PS2_BusInterface_Write CallbackFunction );
		
		
		void ConnectDevice_Read8 ( u32 AddressStart, PS2_BusInterface_Read CallbackFunction );
		void ConnectDevice_Write8 ( u32 AddressStart, PS2_BusInterface_Write CallbackFunction );
		void ConnectDevice_Read16 ( u32 AddressStart, PS2_BusInterface_Read CallbackFunction );
		void ConnectDevice_Write16 ( u32 AddressStart, PS2_BusInterface_Write CallbackFunction );
		void ConnectDevice_Read32 ( u32 AddressStart, PS2_BusInterface_Read CallbackFunction );
		void ConnectDevice_Write32 ( u32 AddressStart, PS2_BusInterface_Write CallbackFunction );
		void ConnectDevice_Read64 ( u32 AddressStart, PS2_BusInterface_Read CallbackFunction );
		void ConnectDevice_Write64 ( u32 AddressStart, PS2_BusInterface_Write CallbackFunction );
		void ConnectDevice_Read128 ( u32 AddressStart, PS2_BusInterface_Read CallbackFunction );
		void ConnectDevice_Write128 ( u32 AddressStart, PS2_BusInterface_Write CallbackFunction );
		
		//void ConnectDevice_Read128 ( u32 AddressStart, PS2_BusInterface_Read128 CallbackFunction );
		//void ConnectDevice_Write128 ( u32 AddressStart, PS2_BusInterface_Write128 CallbackFunction );
		
		
		// lookup table for hardware registers
		static const u32 c_LUT_Reg_Size = 0x400;
		static PS2_BusInterface_Read LUT_RegRead [ c_LUT_Reg_Size ];
		static PS2_BusInterface_Write LUT_RegWrite [ c_LUT_Reg_Size ];
		
		// connect registers with bus
		void Init_ConnectRegs ( void );
		void ConnectRegs_Read ( u32 AddressStart, PS2_BusInterface_Read CallbackFunction );
		void ConnectRegs_Write ( u32 AddressStart, PS2_BusInterface_Write CallbackFunction );
		
		
		static u64 InvalidAddress_Read ( u32 Address, u64 Mask );
		static void InvalidAddress_Write ( u32 Address, u64 Data, u64 Mask );
		

		static u64 Memory_Read ( u32 Address, u64 Mask );
		static void Memory_Write ( u32 Address, u64 Data, u64 Mask );
		
		static u64 BIOS_Read ( u32 Address, u64 Mask );
		
		static u64 ScratchPad_Read ( u32 Address, u64 Mask );
		static void ScratchPad_Write ( u32 Address, u64 Data, u64 Mask );
		
		static u64 Device_Read ( u32 Address, u64 Mask );
		static void Device_Write ( u32 Address, u64 Data, u64 Mask );
		
		static u64 VuMem_Read ( u32 Address, u64 Mask );
		static void VuMem_Write ( u32 Address, u64 Data, u64 Mask );
		
		static u64 DirectCacheMem_Read ( u32 Address, u64 Mask );
		static void DirectCacheMem_Write ( u32 Address, u64 Data, u64 Mask );
		
		
		//static u32* RamSize_Read ( u32 Address, u64 Mask );
		//static void RamSize_Write ( u32 Address, u32 Data, u64 Mask );
		//static u32* PIO_Read ( u32 Address );
		//static void PIO_Write ( u32 Address, u32* Data, u32 Mask );
		
		/*
		template<const int c_iTransferWidth>
		class BusInterface_t
		{
			PS2_BusInterface_Read LUT_BusRead [ c_LUT_Bus_Size ];
			PS2_BusInterface_Write LUT_BusWrite [ c_LUT_Bus_Size ];
			
			void Init_ConnectDevice ( void );
			void ConnectDevice_Read ( u32 AddressStart, PS2_BusInterface_Read CallbackFunction );
			void ConnectDevice_Write ( u32 AddressStart, PS2_BusInterface_Write CallbackFunction );
			
			static u32 InvalidAddress_Read ( u32 Address );
			static void InvalidAddress_Write ( u32 Address, u32 Data, u32 Mask );
			
			static u32 Memory_Read ( u32 Address );
			static void Memory_Write ( u32 Address, u32 Data, u32 Mask );
			
			static u32 BIOS_Read ( u32 Address );
			
			static u32 ScratchPad_Read ( u32 Address );
			static void ScratchPad_Write ( u32 Address, u32 Data, u32 Mask );
		};
		*/
		
		
		// Need pointer into main memory
		union MemoryPtr_Format
		{
			u8* b8;
			u16* b16;
			u32* b32;
			u64* b64;
			s8* sb8;
			s16* sb16;
			s32* sb32;
			s64* sb64;
			
			//MemoryPtr_Format ( u32* _ptr ) { b32 = _ptr; }
		};
		
		// we need a reference to all the components connected to the data bus?
		static MemoryPtr_Format MainMemoryPtr;
		static MemoryPtr_Format BIOSPtr;
		static MemoryPtr_Format ScratchPadPtr;
		
		
		union _MainMemory
		{
			alignas(16) u8 b8 [ MainMemory_Size ];
			alignas(16) u16 b16 [ MainMemory_Size / sizeof ( u16 ) ];
			alignas(16) u32 b32 [ MainMemory_Size / sizeof ( u32 ) ];
			alignas(16) u64 b64 [ MainMemory_Size / sizeof ( u64 ) ];
			alignas(16) s8 sb8 [ MainMemory_Size ];
			alignas(16) s16 sb16 [ MainMemory_Size / sizeof ( s16 ) ];
			alignas(16) s32 sb32 [ MainMemory_Size / sizeof ( s32 ) ];
			alignas(16) s64 sb64 [ MainMemory_Size / sizeof ( s64 ) ];
		};
		
		union _BIOS
		{
			alignas(16) u8 b8 [ BIOS_Size ];
			alignas(16) u16 b16 [ BIOS_Size / sizeof ( u16 ) ];
			alignas(16) u32 b32 [ BIOS_Size / sizeof ( u32 ) ];
			alignas(16) u64 b64 [ BIOS_Size / sizeof ( u64 ) ];
			alignas(16) s8 sb8 [ BIOS_Size ];
			alignas(16) s16 sb16 [ BIOS_Size / sizeof ( s16 ) ];
			alignas(16) s32 sb32 [ BIOS_Size / sizeof ( s32 ) ];
			alignas(16) s64 sb64 [ BIOS_Size / sizeof ( s64 ) ];
		};

		union _ScratchPad
		{
			alignas(16) u8 b8 [ ScratchPad_Size ];
			alignas(16) u16 b16 [ ScratchPad_Size / sizeof ( u16 ) ];
			alignas(16) u32 b32 [ ScratchPad_Size / sizeof ( u32 ) ];
			alignas(16) u64 b64 [ ScratchPad_Size / sizeof ( u64 ) ];
			alignas(16) s8 sb8 [ ScratchPad_Size ];
			alignas(16) s16 sb16 [ ScratchPad_Size / sizeof ( s16 ) ];
			alignas(16) s32 sb32 [ ScratchPad_Size / sizeof ( s32 ) ];
			alignas(16) s64 sb64 [ ScratchPad_Size / sizeof ( s64 ) ];
		};
		
		union _MicroMem0
		{
			alignas(16) u8 b8 [ MicroMem0_Size ];
			alignas(16) u16 b16 [ MicroMem0_Size / sizeof ( u16 ) ];
			alignas(16) u32 b32 [ MicroMem0_Size / sizeof ( u32 ) ];
			alignas(16) u64 b64 [ MicroMem0_Size / sizeof ( u64 ) ];
			alignas(16) s8 sb8 [ MicroMem0_Size ];
			alignas(16) s16 sb16 [ MicroMem0_Size / sizeof ( s16 ) ];
			alignas(16) s32 sb32 [ MicroMem0_Size / sizeof ( s32 ) ];
			alignas(16) s64 sb64 [ MicroMem0_Size / sizeof ( s64 ) ];
		};


		union _MicroMem1
		{
			alignas(16) u8 b8 [ MicroMem1_Size ];
			alignas(16) u16 b16 [ MicroMem1_Size / sizeof ( u16 ) ];
			alignas(16) u32 b32 [ MicroMem1_Size / sizeof ( u32 ) ];
			alignas(16) u64 b64 [ MicroMem1_Size / sizeof ( u64 ) ];
			alignas(16) s8 sb8 [ MicroMem1_Size ];
			alignas(16) s16 sb16 [ MicroMem1_Size / sizeof ( s16 ) ];
			alignas(16) s32 sb32 [ MicroMem1_Size / sizeof ( s32 ) ];
			alignas(16) s64 sb64 [ MicroMem1_Size / sizeof ( s64 ) ];
		};

		
		
		union _DirectCacheMem
		{
			alignas(16) u8 b8 [ DirectCacheMem_Size ];
			alignas(16) u16 b16 [ DirectCacheMem_Size / sizeof ( u16 ) ];
			alignas(16) u32 b32 [ DirectCacheMem_Size / sizeof ( u32 ) ];
			alignas(16) u64 b64 [ DirectCacheMem_Size / sizeof ( u64 ) ];
			alignas(16) s8 sb8 [ DirectCacheMem_Size ];
			alignas(16) s16 sb16 [ DirectCacheMem_Size / sizeof ( s16 ) ];
			alignas(16) s32 sb32 [ DirectCacheMem_Size / sizeof ( s32 ) ];
			alignas(16) s64 sb64 [ DirectCacheMem_Size / sizeof ( s64 ) ];
		};
		

		
		
		_MainMemory MainMemory;
		_BIOS BIOS;
		_ScratchPad ScratchPad;
		
		// micromem is the vu program code memory, where vumem is the vu data memory
		//_MicroMem0 MicroMem0;
		//_MicroMem0 VuMem0;
		//_MicroMem1 MicroMem1;
		//_MicroMem1 VuMem1;
		
		static u64 *MicroMem0;
		static u64 *VuMem0;
		static u64 *MicroMem1;
		static u64 *VuMem1;
		
		
		
		
		
		// temp buffer used for 128-bit reads when needed
		alignas(16) u32 TempBuffer [ 4 ];
		
		_DirectCacheMem DirectCacheMem;
		
		
		// put sbus/mch data here for now
		u32 lSBUS_F200, lSBUS_F210, lSBUS_F220, lSBUS_F230, lSBUS_F240, lSBUS_F260;
		u32 lMCH_RICM, lMCH_DRD, lMCH_F480, lMCH_F490;
		u32 RDRAM_SDEVID;
		
		static const u32 c_MCM_RICM = 0x1000f430;
		static const u32 c_MCM_DRD = 0x1000f440;
		
		// ??? PS1 CTRL ???
		u32 lPS1_CTRL_3210;
		u32 lINTC;
		
		u32 lREG_1a6;
		
		// test dmac enable
		u32 lDMAC_ENABLE;
		
		// need a reference to all the components connected to the data bus
		//static u32 *DebugPC;	// to match up debug info
		
		//static Dma* DMA_Device;
		//static GPU* GPU_Device;
		//static Intc* INTC_Device;
		//static MDEC* MDEC_Device;
		//static PIO* PIO_Device;
		//static SIO* SIO_Device;
		//static Timers* Timers_Device;
		
		//R3000A::Cpu* cpu;
		
		//static const u32 Read_BusyCycles = 1;
		//static const u32 Write_BusyCycles = 1;
		//static const u32 Burst4_BusyCycles = 4;
		
		// says whether dma has accessed bus for a cycle
		// true if we are ok to access the bus
		//bool AccessOK;
		
		u32 BusyCycles;
		
		u64 BusyUntil_Cycle;
		
		// this can be static since it is read right after it is written to
		static u32 Latency;
		
		alignas(16) u64 Dummy128 [ 2 ];
		
		bool isReady ();
		
		//void ReserveBus ( u64 Cycles );
		void ReserveBus_CPU ( u64 CpuCycleCount, u64 Cycles );
		void ReserveBus_DMA ( u64 DmaCycleCount, u64 Cycles );
		u64 GetNextFreeCycle_CPU ();
		u64 GetNextFreeCycle_DMA ();
		
		void ReserveBus_Latency ();
		
		inline static u32 GetLatency () { return Latency; };
		
		
		//static const u32 c_iInvalidate_Shift = 6;
		static const u32 c_iInvalidate_Shift = 4;
		static const u32 c_iInvalidate_BlockSize = 1 << c_iInvalidate_Shift;
		
		// set the size of memory in bits or bytes needed to store invalidate data
		//static const u32 c_iInvalidate_Size = 0x20000000 >> ( 2 + c_iInvalidate_Shift );
		static const u32 c_iInvalidate_Size = 0x02000000 >> ( 2 + c_iInvalidate_Shift );
		static const u32 c_iInvalidate_Mask = c_iInvalidate_Size - 1;

		void Reset_Invalidate ();
		
		// invalidate range of addresses
		void InvalidateRange ( u32 StartAddr, u32 WordCount32 );


		union _Invalidate
		{
			alignas(16) u8 b8 [ c_iInvalidate_Size ];
			alignas(16) u16 b16 [ c_iInvalidate_Size / sizeof ( u16 ) ];
			alignas(16) u32 b32 [ c_iInvalidate_Size / sizeof ( u32 ) ];
			alignas(16) u64 b64 [ c_iInvalidate_Size / sizeof ( u64 ) ];
			alignas(16) s8 sb8 [ c_iInvalidate_Size ];
			alignas(16) s16 sb16 [ c_iInvalidate_Size / sizeof ( s16 ) ];
			alignas(16) s32 sb32 [ c_iInvalidate_Size / sizeof ( s32 ) ];
			alignas(16) s64 sb64 [ c_iInvalidate_Size / sizeof ( s64 ) ];
		};
		
		static _Invalidate InvalidArray;
		static _Invalidate Dummy_InvalidArray;

		
		
		// constructor - needs to connect everything with the bus
		DataBus ();
		void ConnectDevices (
			//Dma* _Dma_Device,
			//CD* _CD_Device,
			//GPU* _GPU_Device,
			//Intc* _INTC_Device,
			//MDEC* _MDEC_Device,
			//PIO* _PIO_Device,
			//SIO* _SIO_Device,
			//SPU* _SPU_Device,
			//Timers* _Timers_Device
		);
		
		// destructor
		~DataBus ();
		
		void Reset ();
		
		void Start ();
		
		
		inline static void Connect_VuMem ( u64* _MicroMem0, u64* _VuMem0, u64* _MicroMem1, u64* _VuMem1 )
		{
			MicroMem0 = _MicroMem0;
			VuMem0 = _VuMem0;
			MicroMem1 = _MicroMem1;
			VuMem1 = _VuMem1;
		}
		
		// request or accept access to the PS1 data bus (32-bit bus)
		// burst transfers possible
		//void Request ( u32 RequestingDeviceIndex, u32 RequestType );
		//bool Accept ( u32 RequestingDeviceIndex );

		// read or write data on PS1 data bus after request accepted
		// returns false when bus could not process request because it was either busy or dma did not release bus
		static u64 Read ( u32 Address, u64 Mask );
		static void Write ( u32 Address, u64 Data, u64 Mask );
		
		//u64* Read128 ( u32 Address, u64 Mask );
		//void Write128 ( u32 Address, u64* Data, u64 Mask );
		
		
		
		void IRead ( u32* DataOut, u32 Address, u32 ReadType );


		// get a pointer into instruction
		// can be used to get pointer into data for reading into i-cache line
		inline u32* GetIMemPtr ( u32 Address )
		{
#ifdef INLINE_DEBUG_GetIMemPtr
	debug << "\r\nDataBus::GetIMemPtr; Address=" << hex << Address;
#endif

			/*
			if ( ( ( Address >> 28 ) & 7 ) == 7 )
			{
#ifdef INLINE_DEBUG_GetIMemPtr
	debug << " ScratchPad";
#endif

				// scratch pad ram //
				Latency = c_iRAM_Read_Latency;
				return (u32*) ( & ScratchPad.b8 [ Address & ScratchPad_Mask ] );
			}
			*/
			
			//else if ( ( ( Address >> 24 ) & 0x1f ) == 0x1f )
			if ( ( Address & 0x1fc00000 ) == 0x1fc00000 )
			{
#ifdef INLINE_DEBUG_GetIMemPtr
	debug << " BIOS";
#endif

				// bios //
				Latency = c_iBIOS_Read_Latency;
				return (u32*) ( & BIOS.b8 [ Address & BIOS_Mask ] );
			}
			else
			{
#ifdef INLINE_DEBUG_GetIMemPtr
	debug << " RAM";
#endif

				// ram //
				Latency = c_iRAM_Read_Latency;
				return (u32*) ( & MainMemory.b8 [ Address & MainMemory_Mask ] );
			}
			
			return NULL;
		}


		
		void Run ();
		
		static u64 EndianTemp [ 2 ];
		
		void* GetPointer ( u32 Address );


		enum { RW_8, RW_16, RW_32, RW_BURST4 };	// values for Read/Write Type
		
		
		/////////////////////////////////////////////
		// For Debugging
		
		void SaveBIOSToFile ( u32 Address, u32 NumberOfInstructions );
		void SaveRAMToFile ( u32 Address, u32 NumberOfInstructions );
		
		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		
#ifdef ENABLE_GUI_DEBUGGER
		// object debug stuff
		// Enable/Disable debug window for object
		// Windows API specific code is in here
		static bool DebugWindow_Enabled;
		static WindowClass::Window *DebugWindow;
		static Debug_MemoryViewer *MemoryViewer;
		static void DebugWindow_Enable ();
		static void DebugWindow_Disable ();
		static void DebugWindow_Update ();
#endif


	static const u64 MASK08 = 0xffULL;
	static const u64 MASK16 = 0xffffULL;
	static const u64 MASK32 = 0xffffffffULL;
	static const u64 MASK64 = 0xffffffffffffffffULL;
	static const u64 MASK128 = 0;

template<const u64 MASK>
static u64 Read_t ( u32 Address )
{
#ifdef INLINE_DEBUG_DATABUS_READ
	debug << "\r\nDataBus::Read; Address=" << hex << Address << " Mask=" << MASK;
#endif

	switch ( MASK )
	{
		case MASK08:
			return LUT_BusRead8 [ ( Address >> 22 ) & 0x3ff ] ( Address, MASK );
			break;
			
		case MASK16:
			return LUT_BusRead16 [ ( Address >> 22 ) & 0x3ff ] ( Address, MASK );
			break;

		case MASK32:
			return LUT_BusRead32 [ ( Address >> 22 ) & 0x3ff ] ( Address, MASK );
			break;
			
		case MASK64:
			return LUT_BusRead64 [ ( Address >> 22 ) & 0x3ff ] ( Address, MASK );
			break;
			
		case MASK128:
			return LUT_BusRead128 [ ( Address >> 22 ) & 0x3ff ] ( Address, MASK );
			break;
			
		default:
			cout << "\nhps2x64: DataBus: Unsupported READ. MASK=" << hex << MASK << "\n";
			break;
			
	}
}


template<const u64 MASK>
static void Write_t ( u32 Address, u64 Data )
{
#ifdef INLINE_DEBUG_DATABUS_WRITE
	debug << "\r\nDataBus::Write; Address=" << hex << Address << " Data=" << Data << " Mask=" << MASK;
#endif

	switch ( MASK )
	{
		case MASK08:
			LUT_BusWrite8 [ ( Address >> 22 ) & 0x3ff ] ( Address, Data, MASK );
			break;
		case MASK16:
			LUT_BusWrite16 [ ( Address >> 22 ) & 0x3ff ] ( Address, Data, MASK );
			break;
		case MASK32:
			LUT_BusWrite32 [ ( Address >> 22 ) & 0x3ff ] ( Address, Data, MASK );
			break;
		case MASK64:
			LUT_BusWrite64 [ ( Address >> 22 ) & 0x3ff ] ( Address, Data, MASK );
			break;
		case MASK128:
			LUT_BusWrite128 [ ( Address >> 22 ) & 0x3ff ] ( Address, Data, MASK );
			break;
	}
	
	return;
}



template<const u64 MASK>
static u64 Memory_Read_t ( u32 Address, u64 Mask )
{
	// this is main ram
#ifdef INLINE_DEBUG_READ_RAM
	debug << "\r\nREAD; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "; RAM; Data = " << hex << (_BUS->MainMemory.b32 [ ( Address & MainMemory_Mask ) >> 2 ]) << dec << ";";
#endif

	Latency = c_iRAM_Read_Latency;
	
	// this area is a maximum of 32 MB
	Address &= MainMemory_Mask;
	
	switch ( MASK )
	{
		case MASK08:
			return MainMemoryPtr.b8 [ Address ];
			break;
			
		case MASK16:
			return MainMemoryPtr.b16 [ Address >> 1 ];
			break;
			
		case MASK32:
			return MainMemoryPtr.b32 [ Address >> 2 ];
			break;
			
		case MASK64:
			return MainMemoryPtr.b64 [ Address >> 3 ];
			break;
			
		case MASK128:
			return (u64) ( & ( MainMemoryPtr.b64 [ Address >> 3 ] ) );
			break;
	}
	
	/*
	if ( !Mask )
	{
		// 128-bit read from memory
		return (u64) ( & ( _BUS->MainMemory.b64 [ Address >> 3 ] ) );
	}

	return ( ( _BUS->MainMemory.b64 [ Address >> 3 ] ) >> ( ( Address & 0x7 ) << 3 ) );
	*/

}


template<const u64 MASK>
static void Memory_Write_t ( u32 Address, u64 Data, u64 Mask )
{
	// this is main ram
#ifdef INLINE_DEBUG_WRITE_RAM
	debug << "\r\nWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data << "; Mask=" << Mask;
	debug << "; RAM";
#endif

	// this area is a maximum of 32 MB
	Address &= MainMemory_Mask;
	
	switch ( MASK )
	{
		case MASK08:
			MainMemoryPtr.b8 [ Address ] = Data;
			break;
			
		case MASK16:
			MainMemoryPtr.b16 [ Address >> 1 ] = Data;
			break;
			
		case MASK32:
			MainMemoryPtr.b32 [ Address >> 2 ] = Data;
			break;
			
		case MASK64:
			MainMemoryPtr.b64 [ Address >> 3 ] = Data;
			break;
			
		case MASK128:
			MainMemoryPtr.b64 [ Address >> 3 ] = ((u64*)Data) [ 0 ];
			MainMemoryPtr.b64 [ ( Address >> 3 ) + 1 ] = ((u64*)Data) [ 1 ];
			break;
	}
	
	
	/*
	if ( !Mask )
	{
		// 128-bit write to memory
		
		_BUS->MainMemory.b64 [ Address >> 3 ] = ((u64*)Data) [ 0 ];
		_BUS->MainMemory.b64 [ ( Address >> 3 ) + 1 ] = ((u64*)Data) [ 1 ];
		
		
		return;
	}
	
	u64 ShiftAmount, ShiftMask;
	ShiftAmount = ( ( Address & 0x7 ) << 3 );
	ShiftMask = ( Mask << ShiftAmount );
	_BUS->MainMemory.b64 [ Address >> 3 ] = ( _BUS->MainMemory.b64 [ Address >> 3 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
	*/
	
#ifdef ENABLE_INVALID_ARRAY
		_BUS->InvalidArray.b8 [ Address >> ( 2 + c_iInvalidate_Shift ) ] = 1;
#endif
}


template<const u64 MASK>
static u64 BIOS_Read_t ( u32 Address, u64 Mask )
{
	// this is bios
#ifdef INLINE_DEBUG_READ_BIOS
	debug << "\r\nRead; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "; BIOS; Data = " << hex << (_BUS->BIOS.b32 [ ( Address & BIOS_Mask ) >> 2 ]) << dec << ";";
#endif
	
	Latency = c_iBIOS_Read_Latency;
		
	// this area is 512 KB
	Address &= BIOS_Mask;

	switch ( MASK )
	{
		case MASK08:
			return BIOSPtr.b8 [ Address ];
			break;
			
		case MASK16:
			return BIOSPtr.b16 [ Address >> 1 ];
			break;
			
		case MASK32:
			return BIOSPtr.b32 [ Address >> 2 ];
			break;
			
		case MASK64:
			return BIOSPtr.b64 [ Address >> 3 ];
			break;
			
		case MASK128:
			return (u64) ( & ( BIOSPtr.b64 [ Address >> 3 ] ) );
			break;
	}
	
	/*
	if ( !Mask )
	{
		// 128-bit read from memory
		return (u64) ( & ( _BUS->BIOS.b64 [ Address >> 3 ] ) );
	}

	//return ( ( _BUS->BIOS.b32 [ Address >> 2 ] ) >> ( ( Address & 0x3 ) << 3 ) );
	return ( ( _BUS->BIOS.b64 [ Address >> 3 ] ) >> ( ( Address & 0x7 ) << 3 ) );
	*/
	
}


template<const u64 MASK>
static u64 ScratchPad_Read_t ( u32 Address, u64 Mask )
{
	// this is scratch pad ram
#ifdef INLINE_DEBUG_READ_SCRATCHPAD
	debug << "\r\nRead; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "; ScratchPad; Data = " << hex << (u64) ( (_BUS->ScratchPad.b64 [ ( Address & ScratchPad_Mask ) >> 3 ]) >> ( ( Address & 0x7 ) << 3 ) ) << dec << ";";
#endif

	Latency = c_iRAM_Read_Latency;
	
	// this area is a maximum of 32 MB
	Address &= ScratchPad_Mask;
	
	switch ( MASK )
	{
		case MASK08:
			return ScratchPadPtr.b8 [ Address ];
			break;
			
		case MASK16:
			return ScratchPadPtr.b16 [ Address >> 1 ];
			break;
			
		case MASK32:
			return ScratchPadPtr.b32 [ Address >> 2 ];
			break;
			
		case MASK64:
			return ScratchPadPtr.b64 [ Address >> 3 ];
			break;
			
		case MASK128:
			return (u64) ( & ( ScratchPadPtr.b64 [ Address >> 3 ] ) );
			break;
	}
	
	/*
	if ( !Mask )
	{
		// 128-bit read from memory
		return (u64) ( & ( _BUS->ScratchPad.b64 [ Address >> 3 ] ) );
	}
	
	//return ( ( _BUS->ScratchPad.b32 [ Address >> 2 ] ) >> ( ( Address & 0x3 ) << 3 ) );
	return ( ( _BUS->ScratchPad.b64 [ Address >> 3 ] ) >> ( ( Address & 0x7 ) << 3 ) );
	*/

}


template<const u64 MASK>
static void ScratchPad_Write_t ( u32 Address, u64 Data, u64 Mask )
{
	// this is scratch pad ram
#ifdef INLINE_DEBUG_WRITE_SCRATCHPAD
	debug << "\r\nWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data;
	debug << "; ScratchPad; Data = " << hex << Data << dec << ";";
#endif

	// this area is a maximum of 32 MB
	Address &= ScratchPad_Mask;

	switch ( MASK )
	{
		case MASK08:
			ScratchPadPtr.b8 [ Address ] = Data;
			break;
			
		case MASK16:
			ScratchPadPtr.b16 [ Address >> 1 ] = Data;
			break;
			
		case MASK32:
			ScratchPadPtr.b32 [ Address >> 2 ] = Data;
			break;
			
		case MASK64:
			ScratchPadPtr.b64 [ Address >> 3 ] = Data;
			break;
			
		case MASK128:
			ScratchPadPtr.b64 [ Address >> 3 ] = ((u64*)Data) [ 0 ];
			ScratchPadPtr.b64 [ ( Address >> 3 ) + 1 ] = ((u64*)Data) [ 1 ];
			break;
	}
	
	/*
	if ( !Mask )
	{
		// 128-bit write to memory
		_BUS->ScratchPad.b64 [ Address >> 3 ] = ((u64*)Data) [ 0 ];
		_BUS->ScratchPad.b64 [ ( Address >> 3 ) + 1 ] = ((u64*)Data) [ 1 ];
		
		//_BUS->ScratchPad.b32 [ Address >> 2 ] = ((u32*)Data) [ 3 ];
		//_BUS->ScratchPad.b32 [ ( Address >> 2 ) + 1 ] = ((u32*)Data) [ 2 ];
		//_BUS->ScratchPad.b32 [ ( Address >> 2 ) + 2 ] = ((u32*)Data) [ 1 ];
		//_BUS->ScratchPad.b32 [ ( Address >> 2 ) + 3 ] = ((u32*)Data) [ 0 ];
		
		return;
	}
	
	u64 ShiftAmount, ShiftMask;
	//ShiftAmount = ( ( Address & 0x3 ) << 3 );
	ShiftAmount = ( ( Address & 0x7 ) << 3 );
	ShiftMask = ( Mask << ShiftAmount );
	//_BUS->ScratchPad.b32 [ Address >> 2 ] = ( _BUS->ScratchPad.b32 [ Address >> 2 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
	_BUS->ScratchPad.b64 [ Address >> 3 ] = ( _BUS->ScratchPad.b64 [ Address >> 3 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
	*/
	
#ifdef ENABLE_INVALID_ARRAY_SCRATCHPAD
		_BUS->InvalidArray.b8 [ Address >> ( 2 + c_iInvalidate_Shift ) ] = 1;
#endif
}


// for template
template<const u64 MASK>
void ConnectDevice_Read_t ( u32 AddressStart, u32 AddressEnd, PS2_BusInterface_Read CallbackFunction )
{
	u32 Start, End, Current;
	Start = ( AddressStart & 0xffc00000 ) >> 22;
	End = ( AddressEnd & 0xffc00000 ) >> 22;

	for ( Current = Start; Current <= End; Current++ )
	{
		switch ( MASK )
		{
			case MASK08:
				LUT_BusRead8 [ Current ] = CallbackFunction;
				break;
			case MASK16:
				LUT_BusRead16 [ Current ] = CallbackFunction;
				break;
			case MASK32:
				LUT_BusRead32 [ Current ] = CallbackFunction;
				break;
			case MASK64:
				LUT_BusRead64 [ Current ] = CallbackFunction;
				break;
			case MASK128:
				LUT_BusRead128 [ Current ] = CallbackFunction;
				break;
			default:
				cout << "hps2x64: BUS: ConnectDevice_Read_t: invalid MASK=" << hex << MASK;
				break;
		}
		
	}
}


void ConnectDevice_Read2 ( u32 AddressStart, u32 AddressEnd, u32* pMemoryDevice, u32 ulDeviceMask, u32 ulDeviceLatency = 0 )
{
	u32 Start, End, Current;
	Start = ( AddressStart & 0xffc00000 ) >> 22;
	End = ( AddressEnd & 0xffc00000 ) >> 22;

	for ( Current = Start; Current <= End; Current++ )
	{
		LUT_DataBus_Read [ Current ].pMemoryDevice = pMemoryDevice;
		LUT_DataBus_Read [ Current ].ulMask = ulDeviceMask;
		LUT_DataBus_Read [ Current ].ulLatency = ulDeviceLatency;
	}
}
void ConnectDevice_Write2 ( u32 AddressStart, u32 AddressEnd, u32* pMemoryDevice, u32 ulDeviceMask, u8* pInvalidateDevice = Dummy_InvalidArray.b8, u32 ulDeviceLatency = 0 )
{
	u32 Start, End, Current;
	Start = ( AddressStart & 0xffc00000 ) >> 22;
	End = ( AddressEnd & 0xffc00000 ) >> 22;

	for ( Current = Start; Current <= End; Current++ )
	{
		LUT_DataBus_Write [ Current ].pMemoryDevice = pMemoryDevice;
		LUT_DataBus_Write [ Current ].ulMask = ulDeviceMask;
		LUT_DataBus_Write [ Current ].ulLatency = ulDeviceLatency;
		LUT_DataBus_Write [ Current ].pInvalidateDevice = pInvalidateDevice;
	}
}

void ConnectRegs_Read2 ( u32 AddressStart, u32 AddressEnd, u32** pRegPtrs )
{
	u32 Start, End, Current;
	Start = ( AddressStart & 0xffc00000 ) >> 22;
	End = ( AddressEnd & 0xffc00000 ) >> 22;

	for ( Current = Start; Current <= End; Current++ )
	{
		//LUT_DataBus_Read [ Current ].pMemoryDevice = pMemoryDevice;
		//LUT_DataBus_Read [ Current ].ulMask = ulDeviceMask;
		//LUT_DataBus_Read [ Current ].ulLatency = ulDeviceLatency;
		LUT_DataBus_Read [ Current ].pRegPtrs = pRegPtrs;
	}
}

		
void Add_RegReg_0x1000_Read ( u32 Address, u32* pRegPtr )
{
	pRegRefList0x1000_Read [ ( Address & 0xffff ) >> 4 ] = pRegPtr;
}

// this is micro/vu mem, can be treated like a ram with a mask for vu1, but not for vu0
void Add_RegReg_0x1100_Read ( u32 Address, u32* pRegPtr )
{
	pRegRefList0x1100_Read [ ( Address & 0xffff ) >> 4 ] = pRegPtr;
}

// gpu registers
void Add_RegReg_0x1200_Read ( u32 Address, u32* pRegPtr )
{
	pRegRefList0x1200_Read [ ( Address & 0xffff ) >> 4 ] = pRegPtr;
}


// for template
template<const u64 MASK>
void ConnectDevice_Write_t ( u32 AddressStart, u32 AddressEnd, PS2_BusInterface_Write CallbackFunction )
{
	u32 Start, End, Current;
	Start = ( AddressStart & 0xffc00000 ) >> 22;
	End = ( AddressEnd & 0xffc00000 ) >> 22;

	for ( Current = Start; Current <= End; Current++ )
	{
		switch ( MASK )
		{
			case MASK08:
				LUT_BusWrite8 [ Current ] = CallbackFunction;
				break;
			case MASK16:
				LUT_BusWrite16 [ Current ] = CallbackFunction;
				break;
			case MASK32:
				LUT_BusWrite32 [ Current ] = CallbackFunction;
				break;
			case MASK64:
				LUT_BusWrite64 [ Current ] = CallbackFunction;
				break;
			case MASK128:
				LUT_BusWrite128 [ Current ] = CallbackFunction;
				break;
			default:
				cout << "hps2x64: BUS: ConnectDevice_Write_t: invalid MASK=" << hex << MASK;
				break;
		}
	}
}




	};
	




	
}

#endif



