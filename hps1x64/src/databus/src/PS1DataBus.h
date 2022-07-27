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


#ifndef _PS1DATABUS_H_
#define _PS1DATABUS_H_

#include "types.h"


#include "PS1_Dma.h"
#include "PS1_CD.h"
#include "PS1_Gpu.h"
#include "PS1_Intc.h"
#include "PS1_MDEC.h"
#include "PS1_PIO.h"
#include "PS1_SIO.h"
#include "PS1_SPU.h"
#include "PS1_Timer.h"

#ifdef PS2_COMPILE
#include "CDvd.h"
#include "PS1_SPU2.h"
#include "PS1_USB.h"
#endif

#include "R3000ADebugPrint.h"

#include "DebugMemoryViewer.h"


// need this for recompiler
#define ENABLE_MEMORY_INVALIDATE



namespace Playstation1
{

	class Dma;
	class MDEC;

	class DataBus
	{
	
		static Debug::Log debug;
		
	public:
	
		static DataBus *_BUS;
		
		
		// 0x1f80 0000 is where the hardware mapped registers start at
		static const u32 HWRegisters_Start = 0x1f800000;
		
		
		
		// cycle that the next event will happen at for this device
		// this will not actually be used here
		u64 NextEvent_Cycle;

		// can also use this to show the interval between dma transfers
		u64 CycleInterval;

		// 0x0000 0000 is physical address of where the regular ram starts at
		static const u32 MainMemory_Start = 0x00000000;
		static const u32 MainMemory_Size = 0x200000;	// 2 MB
		static const u32 MainMemory_Mask = MainMemory_Size - 1;
		
		// 0x1fc0 0000 is physical address of where BIOS starts at
		static const u32 BIOS_Start = 0x1fc00000;
		
#ifdef PS2_COMPILE
		// bios for R3000A on a PS2 is 4MB
		static const u32 BIOS_Size = 0x400000;	// 4MB
#else
		static const u32 BIOS_Size = 0x80000;	// 512 KB
#endif

		static const u32 BIOS_Mask = BIOS_Size - 1;
		
		// reading from BIOS takes 24 cycles (8-bit bus), executing an instruction would be an extra cycle
		static const int c_iBIOS_Read_Latency = 24;
		
		// reading from RAM takes 5 cycles, but if executing an instruction, that takes an additional 1 cycle to execute
		// martin korth psx spec "DMA Channels" section says 6 wait state cycles plus 1 opcode cycle
		static const int c_iRAM_Read_Latency = 5;
		static const int c_iRAM_Write_Latency = 1;
		
		// reading from hardware registers takes around 3 cycles or less
		static const int c_iReg_Read_Latency = 3;


#ifdef ENABLE_MEMORY_INVALIDATE
		
#ifdef INVALIDATE_SIZE_BIT
		// set the number of instructions per block when setting invalidate bits
		static const u32 c_iInvalidate_BlockSize = 64;
		
		// set the size of memory in bytes needed to store invalidate data
		static const u32 c_iInvalidate_Size = ( MainMemory_Size / 4 ) / 8;
		
#else
		//static const u32 c_iInvalidate_Shift = 6;
		static const u32 c_iInvalidate_Shift = 2;
		static const u32 c_iInvalidate_BlockSize = 1 << c_iInvalidate_Shift;
		//static const u32 c_iInvalidate_BlockSize = 32;
		
		// set the size of memory in bits or bytes needed to store invalidate data
		//static const u32 c_iInvalidate_Size = 0x20000000 >> ( 2 + c_iInvalidate_Shift );
		static const u32 c_iInvalidate_Size = 0x00200000 >> ( 2 + c_iInvalidate_Shift );
		static const u32 c_iInvalidate_Mask = c_iInvalidate_Size - 1;
#endif

		void Reset_Invalidate ();
		
		// invalidate range of addresses
		void InvalidateRange ( u32 StartAddr, u32 WordCount32 );

#endif

		
		// bus registers
		u32 RamSize, CD_Delay, DMA_Delay;
		
		
		// reg cache
		u32 RegCache_0x1f8010 [ 256 >> 2 ];
		
		
		// lookup table for bus
		typedef u32 (*PS1_BusInterface_Read) ( u32 Address );
		typedef void (*PS1_BusInterface_Write) ( u32 Address, u32 Data, u32 Mask );
		static const u32 c_LUT_Bus_Size = 0x400;
		static PS1_BusInterface_Read LUT_BusRead [ c_LUT_Bus_Size ];
		static PS1_BusInterface_Write LUT_BusWrite [ c_LUT_Bus_Size ];

		static PS1_BusInterface_Read LUT_BusRead8 [ c_LUT_Bus_Size ];
		static PS1_BusInterface_Write LUT_BusWrite8 [ c_LUT_Bus_Size ];
		static PS1_BusInterface_Read LUT_BusRead16 [ c_LUT_Bus_Size ];
		static PS1_BusInterface_Write LUT_BusWrite16 [ c_LUT_Bus_Size ];
		static PS1_BusInterface_Read LUT_BusRead32 [ c_LUT_Bus_Size ];
		static PS1_BusInterface_Write LUT_BusWrite32 [ c_LUT_Bus_Size ];
		
		
		struct DataBusEntry
		{
			u32* pMemoryDevice;
			u32 ulMask;
			u32 ulLatency;
			u8* pInvalidateDevice;
			u32 ulDeviceTest;
			u32 Dummy;
		};

		alignas(16) static DataBusEntry LUT_DataBus_Read [ c_LUT_Bus_Size ];
		alignas(16) static DataBusEntry LUT_DataBus_Write [ c_LUT_Bus_Size ];
		
		
		
		// connect device with bus
		void Init_ConnectDevice ( void );
		void ConnectDevice_Read ( u32 AddressStart, u32 AddressEnd, PS1_BusInterface_Read CallbackFunction );
		void ConnectDevice_Write ( u32 AddressStart, u32 AddressEnd, PS1_BusInterface_Write CallbackFunction );


		// lookup table for hardware registers
		static const u32 c_LUT_Reg_Size = 0x400;
		static PS1_BusInterface_Read LUT_RegRead [ c_LUT_Reg_Size ];
		static PS1_BusInterface_Write LUT_RegWrite [ c_LUT_Reg_Size ];
		
		// connect registers with bus
		void Init_ConnectRegs ( void );
		void ConnectRegs_Read ( u32 AddressStart, PS1_BusInterface_Read CallbackFunction );
		void ConnectRegs_Write ( u32 AddressStart, PS1_BusInterface_Write CallbackFunction );
		
		
		static u32 InvalidAddress_Read ( u32 Address );
		static void InvalidAddress_Write ( u32 Address, u32 Data, u32 Mask );
		
		static u32 RamSize_Read ( u32 Address );
		static void RamSize_Write ( u32 Address, u32 Data, u32 Mask );

		static u32 Memory_Read ( u32 Address );
		static void Memory_Write ( u32 Address, u32 Data, u32 Mask );
		static u32 BIOS_Read ( u32 Address );
		static u32 PIO_Read ( u32 Address );
		static void PIO_Write ( u32 Address, u32 Data, u32 Mask );
		static u32 Device_Read ( u32 Address );
		static void Device_Write ( u32 Address, u32 Data, u32 Mask );
		

#ifdef PS2_COMPILE
		
		static u32 DEV5_Read ( u32 Address );
		static void DEV5_Write ( u32 Address, u32 Data, u32 Mask );
		
		static u32 SBUS_Read ( u32 Address );
		static void SBUS_Write ( u32 Address, u32 Data, u32 Mask );

		u16 SPU2_Temp [ 2048 ];
		//static u32 SPU2_Read ( u32 Address );
		//static void SPU2_Write ( u32 Address, u32 Data, u32 Mask );

		// reg cache
		u32 RegCache_0x1f8014 [ 256 >> 2 ];
		u32 RegCache_0x1f8015 [ 256 >> 2 ];
		u32 RegCache_0x1f8016 [ 256 >> 2 ];
#endif
		
		
		// Need pointer into main memory
		union MemoryPtr_Format
		{
			u8* b8;
			u16* b16;
			u32* b32;
			s8* sb8;
			s16* sb16;
			s32* sb32;
			
			//MemoryPtr_Format ( u32* _ptr ) { b32 = _ptr; }
		};
		
		
		
		// we need a reference to all the components connected to the data bus?
		static MemoryPtr_Format MainMemoryPtr;
		static MemoryPtr_Format BIOSPtr;
		
		union _MainMemory
		{
			u8 b8 [ MainMemory_Size ];
			u16 b16 [ MainMemory_Size / sizeof ( u16 ) ];
			u32 b32 [ MainMemory_Size / sizeof ( u32 ) ];
			s8 sb8 [ MainMemory_Size ];
			s16 sb16 [ MainMemory_Size / sizeof ( s16 ) ];
			s32 sb32 [ MainMemory_Size / sizeof ( s32 ) ];
		};
		
		union _BIOS
		{
			u8 b8 [ BIOS_Size ];
			u16 b16 [ BIOS_Size / sizeof ( u16 ) ];
			u32 b32 [ BIOS_Size / sizeof ( u32 ) ];
			s8 sb8 [ BIOS_Size ];
			s16 sb16 [ BIOS_Size / sizeof ( s16 ) ];
			s32 sb32 [ BIOS_Size / sizeof ( s32 ) ];
		};
		
		_MainMemory MainMemory;
		_BIOS BIOS;
		
		
#ifdef ENABLE_MEMORY_INVALIDATE

		union _Invalidate
		{
			u8 b8 [ c_iInvalidate_Size ];
			u16 b16 [ c_iInvalidate_Size / sizeof ( u16 ) ];
			u32 b32 [ c_iInvalidate_Size / sizeof ( u32 ) ];
			u64 b64 [ c_iInvalidate_Size / sizeof ( u64 ) ];
			s8 sb8 [ c_iInvalidate_Size ];
			s16 sb16 [ c_iInvalidate_Size / sizeof ( s16 ) ];
			s32 sb32 [ c_iInvalidate_Size / sizeof ( s32 ) ];
			s64 sb64 [ c_iInvalidate_Size / sizeof ( s64 ) ];
		};
		
		static _Invalidate InvalidArray;
		static _Invalidate Dummy_InvalidArray;
		
#endif

		
		
		// need a reference to all the components connected to the data bus
		static u32 *DebugPC;	// to match up debug info
		static Dma* DMA_Device;
		static CD* CD_Device;
		static GPU* GPU_Device;
		static Intc* INTC_Device;
		static MDEC* MDEC_Device;
		static PIO* PIO_Device;
		static SIO* SIO_Device;
		static SPU* SPU_Device;
		static Timers* Timers_Device;
		
#ifdef PS2_COMPILE
		static USB* USB_Device;
#endif
		
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
		
		inline static u32 GetLatency () { return Latency; };
		
		bool isReady ();
		void ReserveBus ( u64 Cycles );
		void ReserveBus_Latency ();
		
		// constructor - needs to connect everything with the bus
		//DataBus ();
		void ConnectDevices ( Dma* _Dma_Device, CD* _CD_Device, GPU* _GPU_Device, Intc* _INTC_Device, MDEC* _MDEC_Device, PIO* _PIO_Device,
							SIO* _SIO_Device, SPU* _SPU_Device, Timers* _Timers_Device
#ifdef PS2_COMPILE
							, USB* _USB_Device
#endif
							);
		
		// destructor
		~DataBus ();
		
		void Reset ();
		
		void Start ();
		
		// request or accept access to the PS1 data bus (32-bit bus)
		// burst transfers possible
		//void Request ( u32 RequestingDeviceIndex, u32 RequestType );
		//bool Accept ( u32 RequestingDeviceIndex );

		// read or write data on PS1 data bus after request accepted
		// returns false when bus could not process request because it was either busy or dma did not release bus
		static u32 Read ( u32 Address );
		static void Write ( u32 Data, u32 Address, u32 Mask );
		void IRead ( u32* DataOut, u32 Address, u32 ReadType );
		
		void Run ();



#ifdef PS2_COMPILE
		u32 EE_Read ( u32 Address, u32 Mask );
		void EE_Write ( u32 Address, u32 Data, u32 Mask );
#endif
		
		
		
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
		static u64* _SystemCycleCount;
		
		// object debug stuff
		// Enable/Disable debug window for object
		// Windows API specific code is in here
		static bool DebugWindow_Enabled;
		static WindowClass::Window *DebugWindow;
		static Debug_MemoryViewer *MemoryViewer;
		static void DebugWindow_Enable ();
		static void DebugWindow_Disable ();
		static void DebugWindow_Update ();
		
		
	static const u32 MASK08 = 0xff;
	static const u32 MASK16 = 0xffff;
	static const u32 MASK32 = 0xffffffff;
	
	// *** template testing *** //
	// templates can be used to get more speed

template<const u32 MASK>
static u32 Memory_Read_t ( u32 Address )
{
	Latency = c_iRAM_Read_Latency;
	
	// this area is a maximum of 2 MB
	Address &= MainMemory_Mask;
			
	// this is main ram
#ifdef INLINE_DEBUG_READ_RAM
	debug << "\r\nBus::DRead; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "RAM. Data = " << hex << (_BUS->MainMemory.b32 [ Address >> 2 ]) << dec << ";";
#endif

#ifdef INLINE_DEBUG_READ_RAM_TESTING
	debug << "\r\n***Bus::DRead; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "RAM. Data = " << hex << (_BUS->MainMemory.b32 [ Address >> 2 ]) << dec << ";";
#endif

	//return ( ( _BUS->MainMemory.b32 [ Address >> 2 ] ) >> ( ( Address & 0x3 ) << 3 ) );
	
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
			
		default:
			cout << "\nhps1x64: DataBus: Unsupported memory read. MASK=" << hex << MASK << "\n";
			break;
	}

	return 0;
}


template<const u32 MASK>
static void Memory_Write_t ( u32 Address, u32 Data, u32 Mask )
{
	// this area is a maximum of 2 MB
	Address &= MainMemory_Mask;
			
			// this is main ram
#ifdef INLINE_DEBUG_WRITE_RAM
	debug << "\r\nBus::DWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << DataIn [ 0 ];
	debug << "RAM. Data = " << hex << Data << dec << ";";
#endif

#ifdef INLINE_DEBUG_WRITE_RAM_TESTING
	debug << "\r\n***Bus::DWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data;
	//debug << "RAM. Data = " << hex << Data << dec << ";";
#endif

	//u32 ShiftAmount, ShiftMask;
	//ShiftAmount = ( ( Address & 0x3 ) << 3 );
	//ShiftMask = ( Mask << ShiftAmount );
	//_BUS->MainMemory.b32 [ Address >> 2 ] = ( _BUS->MainMemory.b32 [ Address >> 2 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );

#ifdef ENABLE_MEMORY_INVALIDATE

#ifdef INVALIDATE_SIZE_BIT
	_BUS->InvalidArray.b64 [ ( Address >> ( 2 + 6 ) ) ] |= ( 1ULL << ( ( Address >> 2 ) & 0x3f ) );
#else
	_BUS->InvalidArray.b8 [ Address >> ( 2 + c_iInvalidate_Shift ) ] = 1;
#endif

#endif

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
			
		default:
			cout << "\nhps1x64: DataBus: Unsupported memory write. MASK=" << hex << MASK << "\n";
			break;
	}
}



template<const u32 MASK>
static u32 BIOS_Read_t ( u32 Address )
{
	if ( ( Address & 0xfff0000 ) == 0xfff00000 )
	{
#ifdef INLINE_DEBUG_READ
		debug << "\r\nBus::DRead; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
		debug << "Unknown. Data = " << hex << (BIOS.b32 [ Address >> 2 ]) << dec << ";";
#endif

		return 0;
	}
	
	Latency = c_iBIOS_Read_Latency;
		
	// this area is 512 KB
	Address &= BIOS_Mask;
			
	
			
			// this is bios
#ifdef INLINE_DEBUG_READ_BIOS
	debug << "\r\nBus::DRead; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "BIOS. Data = " << hex << (_BUS->BIOS.b32 [ Address >> 2 ]) << dec << ";";
#endif

	//return ( ( _BUS->BIOS.b32 [ Address >> 2 ] ) >> ( ( Address & 0x3 ) << 3 ) );
	
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
			
		default:
			cout << "\nhps1x64: DataBus: Unsupported memory read. MASK=" << hex << MASK << "\n";
			break;
	}

	return 0;
}



template<const u32 MASK>
static inline u32 Read_t ( u32 Address )
{
	// clear top 3 bits to get physical address
	Address &= 0x1fffffff;
	
	switch ( MASK )
	{
		case MASK08:
			return LUT_BusRead8 [ ( Address >> 22 ) & 0x3ff ] ( Address );
			break;
		
		case MASK16:
			return LUT_BusRead16 [ ( Address >> 22 ) & 0x3ff ] ( Address );
			break;
			
		case MASK32:
			return LUT_BusRead32 [ ( Address >> 22 ) & 0x3ff ] ( Address );
			break;
			
		default:
			cout << "\nhps1x64: DataBus: Unsupported READ. MASK=" << hex << MASK << "\n";
			break;
	}

	return 0;
}


template<const u32 MASK>
static inline void Write_t ( u32 Data, u32 Address )
{
	// clear top 3 bits to get physical address
	Address &= 0x1fffffff;
	
	// multiplexer
	//LUT_BusWrite [ ( Address >> 22 ) & 0x3ff ] ( Address, Data, Mask );
	
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
			
		default:
			cout << "\nhps1x64: DataBus: Unsupported WRITE. MASK=" << hex << MASK << "\n";
			break;
	}
}


// for templates
template<const u32 MASK>
void ConnectDevice_Read_t ( u32 AddressStart, u32 AddressEnd, PS1_BusInterface_Read CallbackFunction )
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
		}
	}
}

template<const u32 MASK>
void ConnectDevice_Write_t ( u32 AddressStart, u32 AddressEnd, PS1_BusInterface_Write CallbackFunction )
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
		}
	}
}


void ConnectDevice_Read2 ( u32 AddressStart, u32 AddressEnd, u32* pMemoryDevice, u32 ulDeviceMask, u32 ulDeviceLatency = 0, u32 ulDeviceTest = 0 )
{
	u32 Start, End, Current;
	Start = ( AddressStart & 0xffc00000 ) >> 22;
	End = ( AddressEnd & 0xffc00000 ) >> 22;

	for ( Current = Start; Current <= End; Current++ )
	{
		LUT_DataBus_Read [ Current ].pMemoryDevice = pMemoryDevice;
		LUT_DataBus_Read [ Current ].ulMask = ulDeviceMask;
		LUT_DataBus_Read [ Current ].ulLatency = ulDeviceLatency;
		LUT_DataBus_Read [ Current ].ulDeviceTest = ulDeviceTest;
	}
}
void ConnectDevice_Write2 ( u32 AddressStart, u32 AddressEnd, u32* pMemoryDevice, u32 ulDeviceMask, u8* pInvalidateDevice = Dummy_InvalidArray.b8, u32 ulDeviceLatency = 0, u32 ulDeviceTest = 0 )
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
		LUT_DataBus_Write [ Current ].ulDeviceTest = ulDeviceTest;
	}
}


		
	};
	
	


}

#endif



