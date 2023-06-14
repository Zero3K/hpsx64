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


#include "Debug.h"

#include "PS1DataBus.h"



#ifdef PS2_COMPILE

#include "PS2DataBus.h"
#include "PS2_SIF.h"

#endif


#include "R3000A.h"



using namespace Playstation1;
using namespace R3000A::Instruction;


#define ENABLE_MEMORY_INVALIDATE



#ifdef _DEBUG_VERSION_

// enable debugging
#define INLINE_DEBUG_ENABLE

//#define INLINE_DEBUG_INVALIDATE

//#define INLINE_DEBUG_READ_RAM_TESTING
//#define INLINE_DEBUG_WRITE_RAM_TESTING

//#define INLINE_DEBUG_WRITE_DEV5
//#define INLINE_DEBUG_READ_DEV5


//#define INLINE_DEBUG_READ_INVALID
//#define INLINE_DEBUG_WRITE_INVALID
//#define INLINE_DEBUG_READ_INVALID_14XX
//#define INLINE_DEBUG_WRITE_INVALID_14XX



//#define INLINE_DEBUG_READ_SPU2
//#define INLINE_DEBUG_WRITE_SPU2



//#define INLINE_DEBUG_WRITE_REG
//#define INLINE_DEBUG_READ_REG

//#define INLINE_DEBUG_WRITE_SBUS
//#define INLINE_DEBUG_READ_SBUS


//#define INLINE_DEBUG_WRITE_RAMSIZE
//#define INLINE_DEBUG_READ
//#define INLINE_DEBUG_WRITE
//#define INLINE_DEBUG2_READ
//#define INLINE_DEBUG2_WRITE
//#define INLINE_DEBUG3_READ
//#define INLINE_DEBUG3_WRITE
//#define INLINE_DEBUG_READ_INVALID_INVALID
//#define INLINE_DEBUG_WRITE_INVALID_INVALID
//#define INLINE_DEBUG_ISREADY
//#define INLINE_DEBUG_RESERVE


#endif


Debug::Log DataBus::debug;

u32* DataBus::_DebugPC;
u64* DataBus::_DebugCycleCount;
u64* DataBus::_SystemCycleCount;


u32 *DataBus::DebugPC;	// to match up debug info
Dma *DataBus::DMA_Device;
CD *DataBus::CD_Device;
GPU *DataBus::GPU_Device;
Intc *DataBus::INTC_Device;
MDEC *DataBus::MDEC_Device;
PIO *DataBus::PIO_Device;
SIO *DataBus::SIO_Device;
SPU *DataBus::SPU_Device;
Timers *DataBus::Timers_Device;


DataBus::_Invalidate DataBus::InvalidArray;
DataBus::_Invalidate DataBus::Dummy_InvalidArray;


#ifdef PS2_COMPILE

USB *DataBus::USB_Device;

#endif


DataBus *DataBus::_BUS;


DataBus::PS1_BusInterface_Read DataBus::LUT_BusRead [ DataBus::c_LUT_Bus_Size ];
DataBus::PS1_BusInterface_Write DataBus::LUT_BusWrite [ DataBus::c_LUT_Bus_Size ];

DataBus::PS1_BusInterface_Read DataBus::LUT_BusRead8 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS1_BusInterface_Write DataBus::LUT_BusWrite8 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS1_BusInterface_Read DataBus::LUT_BusRead16 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS1_BusInterface_Write DataBus::LUT_BusWrite16 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS1_BusInterface_Read DataBus::LUT_BusRead32 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS1_BusInterface_Write DataBus::LUT_BusWrite32 [ DataBus::c_LUT_Bus_Size ];



DataBus::PS1_BusInterface_Read DataBus::LUT_RegRead [ DataBus::c_LUT_Reg_Size ];
DataBus::PS1_BusInterface_Write DataBus::LUT_RegWrite [ DataBus::c_LUT_Reg_Size ];


alignas(16) DataBus::DataBusEntry DataBus::LUT_DataBus_Read [ DataBus::c_LUT_Bus_Size ];
alignas(16) DataBus::DataBusEntry DataBus::LUT_DataBus_Write [ DataBus::c_LUT_Bus_Size ];


bool DataBus::DebugWindow_Enabled;
WindowClass::Window *DataBus::DebugWindow;
Debug_MemoryViewer *DataBus::MemoryViewer;

u32 DataBus::Latency;

DataBus::MemoryPtr_Format DataBus::MainMemoryPtr;
DataBus::MemoryPtr_Format DataBus::BIOSPtr;



//DataBus::MemoryPtr_Format DataBus::MainMemory = new u32 [ DataBus::MainMemory_Size / 4 ];
//DataBus::MemoryPtr_Format DataBus::BIOS = new u32 [ DataBus::BIOS_Size / 4 ];

/*
u32 Request_Bitmap;
u32 Accept_Bitmap;
*/


/*
DataBus::DataBus ()
{
	cout << "Running BUS constructor...\n";
}
*/


void DataBus::Start ()
{
	cout << "Running BUS::Start...\n";
	
#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	// create debug log
	debug.Create ( "DataBus_Log.txt" );
#endif
	
	Reset ();
	
	_BUS = this;
	
	Init_ConnectDevice ();
	Init_ConnectRegs ();
	
	/*
	ConnectDevice_Read ( 0x00000000, Memory_Read );
	ConnectDevice_Read ( 0x00200000, Memory_Read );
	ConnectDevice_Read ( 0x00400000, Memory_Read );
	ConnectDevice_Read ( 0x00600000, Memory_Read );
	ConnectDevice_Read ( 0x80000000, Memory_Read );
	ConnectDevice_Read ( 0x80200000, Memory_Read );
	ConnectDevice_Read ( 0x80400000, Memory_Read );
	ConnectDevice_Read ( 0x80600000, Memory_Read );
	ConnectDevice_Read ( 0xa0000000, Memory_Read );
	ConnectDevice_Read ( 0xa0200000, Memory_Read );
	ConnectDevice_Read ( 0xa0400000, Memory_Read );
	ConnectDevice_Read ( 0xa0600000, Memory_Read );
	*/
	
	ConnectDevice_Read ( 0x00000000, 0x00600000, Memory_Read );
	ConnectDevice_Read ( 0x80000000, 0x80600000, Memory_Read );
	ConnectDevice_Read ( 0xa0000000, 0xa0600000, Memory_Read );
	
	ConnectDevice_Read_t<0xff> ( 0x00000000, 0x00600000, Memory_Read_t<0xff> );
	ConnectDevice_Read_t<0xff> ( 0x80000000, 0x80600000, Memory_Read_t<0xff> );
	ConnectDevice_Read_t<0xff> ( 0xa0000000, 0xa0600000, Memory_Read_t<0xff> );

	ConnectDevice_Read_t<0xffff> ( 0x00000000, 0x00600000, Memory_Read_t<0xffff> );
	ConnectDevice_Read_t<0xffff> ( 0x80000000, 0x80600000, Memory_Read_t<0xffff> );
	ConnectDevice_Read_t<0xffff> ( 0xa0000000, 0xa0600000, Memory_Read_t<0xffff> );

	ConnectDevice_Read_t<0xffffffff> ( 0x00000000, 0x00600000, Memory_Read_t<0xffffffff> );
	ConnectDevice_Read_t<0xffffffff> ( 0x80000000, 0x80600000, Memory_Read_t<0xffffffff> );
	ConnectDevice_Read_t<0xffffffff> ( 0xa0000000, 0xa0600000, Memory_Read_t<0xffffffff> );


	ConnectDevice_Read2 ( 0x00000000, 0x00600000, MainMemory.b32, MainMemory_Mask, c_iRAM_Read_Latency );
	ConnectDevice_Read2 ( 0x80000000, 0x80600000, MainMemory.b32, MainMemory_Mask, c_iRAM_Read_Latency );
	ConnectDevice_Read2 ( 0xa0000000, 0xa0600000, MainMemory.b32, MainMemory_Mask, c_iRAM_Read_Latency );
	ConnectDevice_Write2 ( 0x00000000, 0x00600000, MainMemory.b32, MainMemory_Mask, InvalidArray.b8, c_iRAM_Write_Latency );
	ConnectDevice_Write2 ( 0x80000000, 0x80600000, MainMemory.b32, MainMemory_Mask, InvalidArray.b8, c_iRAM_Write_Latency );
	ConnectDevice_Write2 ( 0xa0000000, 0xa0600000, MainMemory.b32, MainMemory_Mask, InvalidArray.b8, c_iRAM_Write_Latency );
	
	
	/*
	ConnectDevice_Write ( 0x00000000, Memory_Write );
	ConnectDevice_Write ( 0x00200000, Memory_Write );
	ConnectDevice_Write ( 0x00400000, Memory_Write );
	ConnectDevice_Write ( 0x00600000, Memory_Write );
	ConnectDevice_Write ( 0x80000000, Memory_Write );
	ConnectDevice_Write ( 0x80200000, Memory_Write );
	ConnectDevice_Write ( 0x80400000, Memory_Write );
	ConnectDevice_Write ( 0x80600000, Memory_Write );
	ConnectDevice_Write ( 0xa0000000, Memory_Write );
	ConnectDevice_Write ( 0xa0200000, Memory_Write );
	ConnectDevice_Write ( 0xa0400000, Memory_Write );
	ConnectDevice_Write ( 0xa0600000, Memory_Write );
	*/

	ConnectDevice_Write ( 0x00000000, 0x00600000, Memory_Write );
	ConnectDevice_Write ( 0x80000000, 0x80600000, Memory_Write );
	ConnectDevice_Write ( 0xa0000000, 0xa0600000, Memory_Write );
	
	ConnectDevice_Write_t<0xff> ( 0x00000000, 0x00600000, Memory_Write_t<0xff> );
	ConnectDevice_Write_t<0xff> ( 0x80000000, 0x80600000, Memory_Write_t<0xff> );
	ConnectDevice_Write_t<0xff> ( 0xa0000000, 0xa0600000, Memory_Write_t<0xff> );

	ConnectDevice_Write_t<0xffff> ( 0x00000000, 0x00600000, Memory_Write_t<0xffff> );
	ConnectDevice_Write_t<0xffff> ( 0x80000000, 0x80600000, Memory_Write_t<0xffff> );
	ConnectDevice_Write_t<0xffff> ( 0xa0000000, 0xa0600000, Memory_Write_t<0xffff> );

	ConnectDevice_Write_t<0xffffffff> ( 0x00000000, 0x00600000, Memory_Write_t<0xffffffff> );
	ConnectDevice_Write_t<0xffffffff> ( 0x80000000, 0x80600000, Memory_Write_t<0xffffffff> );
	ConnectDevice_Write_t<0xffffffff> ( 0xa0000000, 0xa0600000, Memory_Write_t<0xffffffff> );


	/*
	ConnectDevice_Read ( 0x1fc00000, BIOS_Read );
	ConnectDevice_Read ( 0x9fc00000, BIOS_Read );
	ConnectDevice_Read ( 0xbfc00000, BIOS_Read );
	*/

	ConnectDevice_Read ( 0x1fc00000, 0x1fc00000, BIOS_Read );
	ConnectDevice_Read ( 0x9fc00000, 0x9fc00000, BIOS_Read );
	ConnectDevice_Read ( 0xbfc00000, 0xbfc00000, BIOS_Read );

	ConnectDevice_Read_t<0xff> ( 0x1fc00000, 0x1fc00000, BIOS_Read_t<0xff> );
	ConnectDevice_Read_t<0xff> ( 0x9fc00000, 0x9fc00000, BIOS_Read_t<0xff> );
	ConnectDevice_Read_t<0xff> ( 0xbfc00000, 0xbfc00000, BIOS_Read_t<0xff> );

	ConnectDevice_Read_t<0xffff> ( 0x1fc00000, 0x1fc00000, BIOS_Read_t<0xffff> );
	ConnectDevice_Read_t<0xffff> ( 0x9fc00000, 0x9fc00000, BIOS_Read_t<0xffff> );
	ConnectDevice_Read_t<0xffff> ( 0xbfc00000, 0xbfc00000, BIOS_Read_t<0xffff> );

	ConnectDevice_Read_t<0xffffffff> ( 0x1fc00000, 0x1fc00000, BIOS_Read_t<0xffffffff> );
	ConnectDevice_Read_t<0xffffffff> ( 0x9fc00000, 0x9fc00000, BIOS_Read_t<0xffffffff> );
	ConnectDevice_Read_t<0xffffffff> ( 0xbfc00000, 0xbfc00000, BIOS_Read_t<0xffffffff> );

	
	ConnectDevice_Read2 ( 0x1fc00000, 0x1fc00000, BIOS.b32, BIOS_Mask, c_iBIOS_Read_Latency );
	ConnectDevice_Read2 ( 0x9fc00000, 0x9fc00000, BIOS.b32, BIOS_Mask, c_iBIOS_Read_Latency );
	ConnectDevice_Read2 ( 0xbfc00000, 0xbfc00000, BIOS.b32, BIOS_Mask, c_iBIOS_Read_Latency );
	
	
	ConnectDevice_Read ( 0x1f000000, 0x1f000000, PIO_Read );
	ConnectDevice_Write ( 0x1f000000, 0x1f000000, PIO_Write );

	ConnectDevice_Read ( 0x1f800000, 0x1f800000, Device_Read );
	ConnectDevice_Write ( 0x1f800000, 0x1f800000, Device_Write );

	ConnectDevice_Read2 ( 0x1f800000, 0x1f800000, R3000A::Cpu::_CPU->DCache.b32, 0x3ff, 0, ~0xff8003ff );
	ConnectDevice_Write2 ( 0x1f800000, 0x1f800000, R3000A::Cpu::_CPU->DCache.b32, 0x3ff, Dummy_InvalidArray.b8, 0, ~0xff8003ff );
	
#ifdef PS2_COMPILE
	ConnectDevice_Read2 ( 0xbf800000, 0xbf800000, R3000A::Cpu::_CPU->DCache.b32, 0x3ff, 0, ~0xff8003ff );
	ConnectDevice_Write2 ( 0xbf800000, 0xbf800000, R3000A::Cpu::_CPU->DCache.b32, 0x3ff, Dummy_InvalidArray.b8, 0, ~0xff8003ff );
#endif
	


	
#ifdef PS2_COMPILE

	// SPU2??/CDvd
	ConnectDevice_Read ( 0x1f400000, 0x1f400000, DEV5_Read );
	ConnectDevice_Write ( 0x1f400000, 0x1f400000, DEV5_Write );

	// communication to EE (PS2) from IOP (PS1)
	//ConnectDevice_Read ( 0x1d000000, SBUS_Read );
	//ConnectDevice_Write ( 0x1d000000, SBUS_Write );
	ConnectDevice_Read ( 0x1d000000, 0x1d000000, Playstation2::SIF::IOP_Read );
	ConnectDevice_Write ( 0x1d000000, 0x1d000000, Playstation2::SIF::IOP_Write );
	
#endif
	
	
	// connect hardware registers
	
	// SIO
	ConnectRegs_Read ( 0x1f801040, SIO_Device->Read );
	ConnectRegs_Read ( 0x1f801050, SIO_Device->Read );
	ConnectRegs_Write ( 0x1f801040, SIO_Device->Write );
	ConnectRegs_Write ( 0x1f801050, SIO_Device->Write );
	
#ifdef PS2_COMPILE
	// SIO2 0x1f808260-0x1f808280
	// actually 0x1f808200-0x1f808280
	// looks like some kind of SIO interface
	
	ConnectRegs_Read ( 0x1f808200, SIO_Device->Read );
	ConnectRegs_Read ( 0x1f808210, SIO_Device->Read );
	ConnectRegs_Read ( 0x1f808220, SIO_Device->Read );
	ConnectRegs_Read ( 0x1f808230, SIO_Device->Read );
	ConnectRegs_Read ( 0x1f808240, SIO_Device->Read );
	ConnectRegs_Read ( 0x1f808250, SIO_Device->Read );
	ConnectRegs_Read ( 0x1f808260, SIO_Device->Read );
	ConnectRegs_Read ( 0x1f808270, SIO_Device->Read );
	ConnectRegs_Read ( 0x1f808280, SIO_Device->Read );
	
	ConnectRegs_Write ( 0x1f808200, SIO_Device->Write );
	ConnectRegs_Write ( 0x1f808210, SIO_Device->Write );
	ConnectRegs_Write ( 0x1f808220, SIO_Device->Write );
	ConnectRegs_Write ( 0x1f808230, SIO_Device->Write );
	ConnectRegs_Write ( 0x1f808240, SIO_Device->Write );
	ConnectRegs_Write ( 0x1f808250, SIO_Device->Write );
	ConnectRegs_Write ( 0x1f808260, SIO_Device->Write );
	ConnectRegs_Write ( 0x1f808270, SIO_Device->Write );
	ConnectRegs_Write ( 0x1f808280, SIO_Device->Write );
	
#endif


	// RAM Size
	ConnectRegs_Read ( 0x1f801060, RamSize_Read );
	ConnectRegs_Write ( 0x1f801060, RamSize_Write );
	
	// INTC
	ConnectRegs_Read ( 0x1f801070, INTC_Device->Read );
	ConnectRegs_Write ( 0x1f801070, INTC_Device->Write );
	
	// DMA
	ConnectRegs_Read ( 0x1f801080, DMA_Device->Read );
	ConnectRegs_Read ( 0x1f801090, DMA_Device->Read );
	ConnectRegs_Read ( 0x1f8010a0, DMA_Device->Read );
	ConnectRegs_Read ( 0x1f8010b0, DMA_Device->Read );
	ConnectRegs_Read ( 0x1f8010c0, DMA_Device->Read );
	ConnectRegs_Read ( 0x1f8010d0, DMA_Device->Read );
	ConnectRegs_Read ( 0x1f8010e0, DMA_Device->Read );
	ConnectRegs_Read ( 0x1f8010f0, DMA_Device->Read );
	
	ConnectRegs_Write ( 0x1f801080, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f801090, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f8010a0, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f8010b0, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f8010c0, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f8010d0, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f8010e0, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f8010f0, DMA_Device->Write );
	
#ifdef PS2_COMPILE
	// Extra IOP DMA Channels 0x1f801500-0x1f80155c
	// 6 more DMA Channels

	ConnectRegs_Read ( 0x1f801500, DMA_Device->Read );
	ConnectRegs_Read ( 0x1f801510, DMA_Device->Read );
	ConnectRegs_Read ( 0x1f801520, DMA_Device->Read );
	ConnectRegs_Read ( 0x1f801530, DMA_Device->Read );
	ConnectRegs_Read ( 0x1f801540, DMA_Device->Read );
	ConnectRegs_Read ( 0x1f801550, DMA_Device->Read );
	ConnectRegs_Read ( 0x1f801560, DMA_Device->Read );

	ConnectRegs_Write ( 0x1f801500, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f801510, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f801520, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f801530, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f801540, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f801550, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f801560, DMA_Device->Write );
	
	// there's also PCR2 and ICR2
	ConnectRegs_Read ( 0x1f801570, DMA_Device->Read );
	ConnectRegs_Write ( 0x1f801570, DMA_Device->Write );
	
	
	// then there's some stuff in the range 0x1f8016XX ??
	// this is the USB area ??
	ConnectRegs_Read ( 0x1f801600, USB_Device->Read );
	ConnectRegs_Write ( 0x1f801600, USB_Device->Write );
	ConnectRegs_Read ( 0x1f801610, USB_Device->Read );
	ConnectRegs_Write ( 0x1f801610, USB_Device->Write );
	ConnectRegs_Read ( 0x1f801620, USB_Device->Read );
	ConnectRegs_Write ( 0x1f801620, USB_Device->Write );
	ConnectRegs_Read ( 0x1f801630, USB_Device->Read );
	ConnectRegs_Write ( 0x1f801630, USB_Device->Write );
	ConnectRegs_Read ( 0x1f801640, USB_Device->Read );
	ConnectRegs_Write ( 0x1f801640, USB_Device->Write );
	ConnectRegs_Read ( 0x1f801650, USB_Device->Read );
	ConnectRegs_Write ( 0x1f801650, USB_Device->Write );
	ConnectRegs_Read ( 0x1f801660, USB_Device->Read );
	ConnectRegs_Write ( 0x1f801660, USB_Device->Write );
	ConnectRegs_Read ( 0x1f801670, USB_Device->Read );
	ConnectRegs_Write ( 0x1f801670, USB_Device->Write );
	ConnectRegs_Read ( 0x1f801680, USB_Device->Read );
	ConnectRegs_Write ( 0x1f801680, USB_Device->Write );
	ConnectRegs_Read ( 0x1f801690, USB_Device->Read );
	ConnectRegs_Write ( 0x1f801690, USB_Device->Write );
	
	
#endif
	
	
	// TIMER
	ConnectRegs_Read ( 0x1f801100, Timers_Device->Read );
	ConnectRegs_Read ( 0x1f801110, Timers_Device->Read );
	ConnectRegs_Read ( 0x1f801120, Timers_Device->Read );
	ConnectRegs_Write ( 0x1f801100, Timers_Device->Write );
	ConnectRegs_Write ( 0x1f801110, Timers_Device->Write );
	ConnectRegs_Write ( 0x1f801120, Timers_Device->Write );


#ifdef PS2_COMPILE
	// Extra IOP Timers 0x1f801480-0x1f8014a8
	// 3 more timers
	
	ConnectRegs_Read ( 0x1f801480, Timers_Device->Read );
	ConnectRegs_Read ( 0x1f801490, Timers_Device->Read );
	ConnectRegs_Read ( 0x1f8014a0, Timers_Device->Read );
	
	ConnectRegs_Write ( 0x1f801480, Timers_Device->Write );
	ConnectRegs_Write ( 0x1f801490, Timers_Device->Write );
	ConnectRegs_Write ( 0x1f8014a0, Timers_Device->Write );
#endif

	
	// CD
	ConnectRegs_Read ( 0x1f801800, CD_Device->Read );
	ConnectRegs_Write ( 0x1f801800, CD_Device->Write );
	
	// GPU
	ConnectRegs_Read ( 0x1f801810, GPU_Device->Read );
	ConnectRegs_Write ( 0x1f801810, GPU_Device->Write );
	
	// MDEC
	ConnectRegs_Read ( 0x1f801820, MDEC_Device->Read );
	ConnectRegs_Write ( 0x1f801820, MDEC_Device->Write );
	
	// SPU
	for ( int i = 0; i <= 0x250; i += 0x10 )
	{
		ConnectRegs_Read ( 0x1f801c00 + i, SPU_Device->Read );
		ConnectRegs_Write ( 0x1f801c00 + i, SPU_Device->Write );
	}
	
	/*
	ConnectRegs_Read ( 0x1f801c00, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801c10, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801c20, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801c30, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801c40, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801c50, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801c60, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801c70, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801c80, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801c90, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801ca0, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801cb0, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801cc0, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801cd0, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801ce0, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801cf0, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801d00, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801d10, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801d20, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801d30, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801d40, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801d50, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801d60, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801d70, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801d80, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801d90, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801da0, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801db0, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801dc0, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801dd0, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801de0, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801df0, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801e00, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801e10, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801e20, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801e30, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801e40, SPU_Device->Read );
	ConnectRegs_Read ( 0x1f801e50, SPU_Device->Read );
	
	ConnectRegs_Write ( 0x1f801c00, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801c10, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801c20, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801c30, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801c40, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801c50, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801c60, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801c70, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801c80, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801c90, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801ca0, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801cb0, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801cc0, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801cd0, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801ce0, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801cf0, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801d00, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801d10, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801d20, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801d30, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801d40, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801d50, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801d60, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801d70, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801d80, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801d90, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801da0, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801db0, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801dc0, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801dd0, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801de0, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801df0, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801e00, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801e10, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801e20, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801e30, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801e40, SPU_Device->Write );
	ConnectRegs_Write ( 0x1f801e50, SPU_Device->Write );
	*/


#ifdef PS2_COMPILE

	// SPU2 has a different interface than SPU1, but the interface is basically the same for both cores
	//ConnectDevice_Read ( 0x1f900000, SPU2_Read );
	//ConnectDevice_Write ( 0x1f900000, SPU2_Write );
	
#endif

	

#ifdef PS2_COMPILE

	/*
	// SIO2 0x1f808260-0x1f808280
	//ConnectRegs_Write ( 0x1f808260, SIO2_Device->Write );
	//ConnectRegs_Write ( 0x1f808270, SIO2_Device->Write );
	//ConnectRegs_Write ( 0x1f808280, SIO2_Device->Write );
	
	// Extra IOP Timers 0x1f801480-0x1f8014a8
	// 3 more timers
	ConnectRegs_Write ( 0x1f801480, Timers_Device->Write );
	ConnectRegs_Write ( 0x1f801490, Timers_Device->Write );
	ConnectRegs_Write ( 0x1f8014a0, Timers_Device->Write );
	
	// Extra IOP DMA Channels 0x1f801500-0x1f80155c
	// 6 more DMA Channels
	ConnectRegs_Write ( 0x1f801500, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f801510, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f801520, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f801530, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f801540, DMA_Device->Write );
	ConnectRegs_Write ( 0x1f801550, DMA_Device->Write );
	
	// appears that cdvd is at 0x1f4020xx
	
	// SPU2 is at 0x1f90XXXX
	
	// SBUS regs are at 0x1d00XXXX
	
	
	// dev9 is at 0x1000XXXX (from PS1/IOP)
	
	
	
	*/
	
#endif

}


void DataBus::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( DataBus ) );
	
	// set static pointer into main memory
	MainMemoryPtr.b32 = MainMemory.b32;
	
	// set static pointer into bios
	BIOSPtr.b32 = BIOS.b32;
	
	// Bus is not busy yet
	//BusyCycles = 0;
	
	// init
	NextEvent_Cycle = -1LL;
	CycleInterval = -1LL;
}


void DataBus::ConnectDevices ( Dma* _Dma_Device, CD* _CD_Device, GPU* _GPU_Device, Intc* _INTC_Device, MDEC* _MDEC_Device, PIO* _PIO_Device,
					SIO* _SIO_Device, SPU* _SPU_Device, Timers* _Timers_Device
#ifdef PS2_COMPILE
					, USB* _USB_Device
#endif
					)
{
	DMA_Device = _Dma_Device;
	CD_Device = _CD_Device;
	GPU_Device = _GPU_Device;
	INTC_Device = _INTC_Device;
	MDEC_Device = _MDEC_Device;
	PIO_Device = _PIO_Device;
	SIO_Device = _SIO_Device;
	SPU_Device = _SPU_Device;
	Timers_Device = _Timers_Device;
	
#ifdef PS2_COMPILE
	USB_Device = _USB_Device;
#endif
}


DataBus::~DataBus ()
{
	//delete MainMemory.b32;
	//delete BIOS.b32;
}



u32 DataBus::RamSize_Read ( u32 Address )
{
	Latency = c_iReg_Read_Latency;
	return _BUS->RamSize;
}

void DataBus::RamSize_Write ( u32 Address, u32 Data, u32 Mask )
{
	u32 ShiftAmount, ShiftMask;
	ShiftAmount = ( ( Address & 0x3 ) << 3 );
	ShiftMask = ( Mask << ShiftAmount );
	_BUS->RamSize = ( _BUS->RamSize & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
}

u32 DataBus::Memory_Read ( u32 Address )
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
//testing
if ( Address == 0x115f4 )
{
	debug << "\r\n***Bus::DRead; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "RAM. Data = " << hex << (_BUS->MainMemory.b32 [ Address >> 2 ]) << dec << ";";
}
#endif

	return ( ( _BUS->MainMemory.b32 [ Address >> 2 ] ) >> ( ( Address & 0x3 ) << 3 ) );

}

void DataBus::Memory_Write ( u32 Address, u32 Data, u32 Mask )
{
	// this area is a maximum of 2 MB
	Address &= 0x1fffff;
			
			// this is main ram
#ifdef INLINE_DEBUG_WRITE_RAM
	debug << "\r\nBus::DWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << DataIn [ 0 ];
	debug << "RAM. Data = " << hex << Data << dec << ";";
#endif

#ifdef INLINE_DEBUG_WRITE_RAM_TESTING
//testing
if ( Address == 0x115f4 )
{
	debug << "\r\n***Bus::DWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data;
	//debug << "RAM. Data = " << hex << Data << dec << ";";
}
#endif

	u32 ShiftAmount, ShiftMask;
	ShiftAmount = ( ( Address & 0x3 ) << 3 );
	ShiftMask = ( Mask << ShiftAmount );
	_BUS->MainMemory.b32 [ Address >> 2 ] = ( _BUS->MainMemory.b32 [ Address >> 2 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );

#ifdef ENABLE_MEMORY_INVALIDATE

#ifdef INVALIDATE_SIZE_BIT
	_BUS->InvalidArray.b64 [ ( Address >> ( 2 + 6 ) ) ] |= ( 1ULL << ( ( Address >> 2 ) & 0x3f ) );
#else
	_BUS->InvalidArray.b8 [ Address >> ( 2 + c_iInvalidate_Shift ) ] = 1;
#endif

#endif
}

u32 DataBus::BIOS_Read ( u32 Address )
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

	return ( ( _BUS->BIOS.b32 [ Address >> 2 ] ) >> ( ( Address & 0x3 ) << 3 ) );
	
}


u32 DataBus::PIO_Read ( u32 Address )
{
#ifdef INLINE_DEBUG_READ_PIO
	debug << "\r\nBus::DRead; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << ";PIO";
#endif

	// I guess this is for the parallel port
	return PIO_Device->Read ( Address );
}

void DataBus::PIO_Write ( u32 Address, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE_PIO
	debug << "\r\nBus::DWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << DataIn [ 0 ];
	debug << "; PIO";
#endif

	PIO_Device->Write ( Address, Data, Mask );
}



#ifdef PS2_COMPILE

u32 DataBus::DEV5_Read ( u32 Address )
{
#ifdef INLINE_DEBUG_READ_DEV5
	debug << "\r\nBus::DRead; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "; DEV5";
#endif

	if ( Address < 0x1f402000 )
	{
#ifdef INLINE_DEBUG_READ_DEV5
	debug << "; SPU2???";
#endif

	}
	else
	{
#ifdef INLINE_DEBUG_READ_DEV5
	debug << "; CDVD";
#endif

		return CDVD::_CDVD->Read ( Address );
	}

	return 0;
}

void DataBus::DEV5_Write ( u32 Address, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE_DEV5
	debug << "\r\nBus::DWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data << "; Mask=" << Mask;
	debug << "; DEV5";
#endif

	if ( Address < 0x1f402000 )
	{
#ifdef INLINE_DEBUG_READ_DEV5
	debug << "; SPU2";
#endif

	}
	else
	{
#ifdef INLINE_DEBUG_READ_DEV5
	debug << "; CDVD";
#endif

		CDVD::_CDVD->Write ( Address, Data, Mask );
	}

}

u32 DataBus::SBUS_Read ( u32 Address )
{
#ifdef INLINE_DEBUG_READ_SBUS
	debug << "\r\nBus::DRead; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "; SBUS";
#endif

	// SBUS stuff is currently ripped/adapted from pcsx2 for now //

	u32 Output;
	
	// ??
	Latency = 3;

	Address &= 0x1fffffff;

	switch ( Address )
	{
		case 0x1d000000:
			
			// incoming from EE
			Output = Playstation2::DataBus::_BUS->lSBUS_F200;
			break;
			
		case 0x1d000010:
			// outgoing from IOP
			Output = Playstation2::DataBus::_BUS->lSBUS_F210;
			break;
			
		case 0x1d000020:
			Output = Playstation2::DataBus::_BUS->lSBUS_F220;
			break;
			
		case 0x1d000030:
			Output = Playstation2::DataBus::_BUS->lSBUS_F230;
			break;
			
		case 0x1d000040:
			// control register ?? //
			Output = Playstation2::DataBus::_BUS->lSBUS_F240 | 0xF0000002;
			break;
			
		case 0x1d000060:
			Output = Playstation2::DataBus::_BUS->lSBUS_F260;
			break;
	}
	
#ifdef INLINE_DEBUG_READ_SBUS
	debug << "; Output = " << Output;
#endif

	return Output;
}

void DataBus::SBUS_Write ( u32 Address, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE_SBUS
	debug << "\r\nBus::DWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data << " Mask=" << Mask;
	debug << "; SBUS";
#endif

	u32 temp;

	// ??
	Latency = 3;

	Address &= 0x1fffffff;

	switch ( Address )
	{
		case 0x1d000000:
			
			// incoming from EE
			break;
			
		case 0x1d000010:
			// outgoing from IOP
			Playstation2::DataBus::_BUS->lSBUS_F210 = Data;
			break;
			
		case 0x1d000020:
			// bits cleared when written from IOP
			Playstation2::DataBus::_BUS->lSBUS_F220 &= ~Data;
			break;
			
		case 0x1d000030:
			// bits set when written from IOP
			Playstation2::DataBus::_BUS->lSBUS_F230 |= Data;
			break;
			
		case 0x1d000040:		// Control Register
			temp = Data & 0xf0;
			if ( Data & 0x20 || Data & 0x80)
			{
				Playstation2::DataBus::_BUS->lSBUS_F240 &= ~0xf000;
				Playstation2::DataBus::_BUS->lSBUS_F240 |= 0x2000;
			}


			if ( Playstation2::DataBus::_BUS->lSBUS_F240 & temp )
				Playstation2::DataBus::_BUS->lSBUS_F240 &= ~temp;
			else
				Playstation2::DataBus::_BUS->lSBUS_F240 |= temp;
				
			break;

		case 0x1d000060:
			Playstation2::DataBus::_BUS->lSBUS_F260 = 0;
			break;
	}

}

/*
u32 DataBus::SPU2_Read ( u32 Address )
{
#ifdef INLINE_DEBUG_READ_SPU2
	debug << "\r\nBus::DRead; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "; SPU2";
#endif

	u32 Output;

	// ??
	Latency = 3;
	
	Address &= 0x1fffffff;
	
	Output = _BUS->SPU2_Temp [ ( Address - 0x1f900000 ) >> 1 ];

#ifdef INLINE_DEBUG_READ_SPU2
	debug << "; Output=" << Output;
#endif

	return Output;
}


static void DataBus::SPU2_Write ( u32 Address, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE_SPU2
	debug << "\r\nBus::DWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data << " Mask=" << Mask;
	debug << "; SPU2";
#endif

	// ??
	Latency = 3;

	Address &= 0x1fffffff;
	
	_BUS->SPU2_Temp [ ( Address - 0x1f900000 ) >> 1 ] = Data;

}
*/

#endif



u32 DataBus::Device_Read ( u32 Address )
{
	u32 PhysicalAddress_Low;

			// this is a hardware mapped register
			// *** todo *** read from hardware mapped register
			
#ifdef PS2_COMPILE
	if ( ( ( Address >> 20 ) & 0x1ff ) == 0x1f9 )
	{
		return SPU2::Read ( Address );
	}
#endif


#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
#endif

	Latency = c_iReg_Read_Latency;
	

	return LUT_RegRead [ ( Address >> 4 ) & 0x3ff ] ( Address );
			
}

void DataBus::Device_Write ( u32 Address, u32 Data, u32 Mask )
{
			// this is a hardware mapped register
			// *** todo *** read from hardware mapped register
			
#ifdef PS2_COMPILE
	if ( ( ( Address >> 20 ) & 0x1ff ) == 0x1f9 )
	{
		SPU2::Write ( Address, Data, Mask );
		return;
	}
#endif


#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
#endif
			

	LUT_RegWrite [ ( Address >> 4 ) & 0x3ff ] ( Address, Data, Mask );
	
	return;

}


u32 DataBus::Read ( u32 Address )
{
	// clear top 3 bits to get physical address
	Address &= 0x1fffffff;
	
	// multiplexer
	return LUT_BusRead [ ( Address >> 22 ) & 0x3ff ] ( Address );
}


void DataBus::Write ( u32 Data, u32 Address, u32 Mask )
{
	// clear top 3 bits to get physical address
	Address &= 0x1fffffff;
	
	// multiplexer
	LUT_BusWrite [ ( Address >> 22 ) & 0x3ff ] ( Address, Data, Mask );
	
	return;
}


void DataBus::SaveBIOSToFile ( u32 Address, u32 NumberOfInstructions )
{
	int j;
	u32 CurrentPC;
	Debug::Log FileOut;
	u32 i;
	
	CurrentPC = Address;
	
	FileOut.Create ( "BIOS_DebugOut.txt" );
	
	FileOut << "BIOS Data Output...";
	
	cout << "\nSaving BIOS contents into file named BIOS_DebugOut.txt\n";
	
	for ( j = 0; j < NumberOfInstructions; j++, CurrentPC += 4 )
	{
		i = BIOS.b32 [ ( CurrentPC & BIOS_Mask ) >> 2 ];
		FileOut << "\r\n" << hex << setw( 8 ) << CurrentPC << " " << (CurrentPC & BIOS_Mask) << " " << Print::PrintInstruction ( i ).c_str () << "; " << i;
		
		// show status
		if ( ( j & 0xff ) == 0xff ) cout << "\rProgress: " << j << " out of " << NumberOfInstructions;
	}
	
	cout << "\nDone.\n";
}

void DataBus::SaveRAMToFile ( u32 Address, u32 NumberOfInstructions )
{
	int j;
	u32 CurrentPC;
	Debug::Log FileOut;
	u32 i;
	
	CurrentPC = Address;
	
	FileOut.Create ( "RAM_DebugOut.txt" );
	
	FileOut << "RAM Data Output...";
	
	cout << "\nSaving RAM contents into file named RAM_DebugOut.txt\n";
	
	for ( j = 0; j < NumberOfInstructions; j++, CurrentPC += 4 )
	{
		i = MainMemory.b32 [ ( CurrentPC & MainMemory_Mask ) >> 2 ];
		FileOut << "\r\n" << hex << setw( 8 ) << CurrentPC << " " << (CurrentPC & MainMemory_Mask) << " " << Print::PrintInstruction ( i ).c_str () << "; " << i;
		
		// show status
		if ( ( j & 0xff ) == 0xff ) cout << "\rProgress: " << j << " out of " << NumberOfInstructions;
	}

	cout << "\nDone.\n";
}


void* DataBus::GetPointer ( u32 Address )
{
	if ( ( ( Address >> 20 ) & 0x1fc ) == 0x1fc )
	{
		return (void*) &(BIOS.b8 [ Address & 0x7ffff ]);
	}
	else
	{
		return (void*) &(MainMemory.b8 [ Address & 0x1fffff ]);
	}
}



void DataBus::Init_ConnectDevice ( void )
{
	int i;
	
	for ( i = 0; i < c_LUT_Bus_Size; i++ )
	{
		LUT_BusRead [ i ] = InvalidAddress_Read;
		
		// for template
		LUT_BusRead8 [ i ] = InvalidAddress_Read;
		LUT_BusRead16 [ i ] = InvalidAddress_Read;
		LUT_BusRead32 [ i ] = InvalidAddress_Read;
		
		// for recompiler
		LUT_DataBus_Read [ i ].pMemoryDevice = NULL;
		LUT_DataBus_Read [ i ].ulMask = 0;
		LUT_DataBus_Read [ i ].ulLatency = 0;
		LUT_DataBus_Write [ i ].pMemoryDevice = NULL;
		LUT_DataBus_Write [ i ].ulMask = 0;
		LUT_DataBus_Write [ i ].ulLatency = 0;
	}
	
	for ( i = 0; i < c_LUT_Bus_Size; i++ )
	{
		LUT_BusWrite [ i ] = InvalidAddress_Write;
		
		// for template
		LUT_BusWrite8 [ i ] = InvalidAddress_Write;
		LUT_BusWrite16 [ i ] = InvalidAddress_Write;
		LUT_BusWrite32 [ i ] = InvalidAddress_Write;
	}
}



void DataBus::ConnectDevice_Read ( u32 AddressStart, u32 AddressEnd, PS1_BusInterface_Read CallbackFunction )
{
	u32 Start, End, Current;
	Start = ( AddressStart & 0xffc00000 ) >> 22;
	End = ( AddressEnd & 0xffc00000 ) >> 22;

	for ( Current = Start; Current <= End; Current++ )
	{
		// add callback
		LUT_BusRead [ Current ] = CallbackFunction;
		
		// for templates
		LUT_BusRead8 [ Current ] = CallbackFunction;
		LUT_BusRead16 [ Current ] = CallbackFunction;
		LUT_BusRead32 [ Current ] = CallbackFunction;
	}
}

void DataBus::ConnectDevice_Write ( u32 AddressStart, u32 AddressEnd, PS1_BusInterface_Write CallbackFunction )
{
	u32 Start, End, Current;
	Start = ( AddressStart & 0xffc00000 ) >> 22;
	End = ( AddressEnd & 0xffc00000 ) >> 22;

	for ( Current = Start; Current <= End; Current++ )
	{
		// add callback
		LUT_BusWrite [ Current ] = CallbackFunction;
		
		// for templates
		LUT_BusWrite8 [ Current ] = CallbackFunction;
		LUT_BusWrite16 [ Current ] = CallbackFunction;
		LUT_BusWrite32 [ Current ] = CallbackFunction;
	}
}





void DataBus::Init_ConnectRegs ( void )
{
	int i;
	
	for ( i = 0; i < c_LUT_Reg_Size; i++ )
	{
		LUT_RegRead [ i ] = InvalidAddress_Read;
		
		// for templates
		//LUT_RegRead8 [ i ] = InvalidAddress_Read;
		//LUT_RegRead16 [ i ] = InvalidAddress_Read;
		//LUT_RegRead32 [ i ] = InvalidAddress_Read;
	}
	
	for ( i = 0; i < c_LUT_Reg_Size; i++ )
	{
		LUT_RegWrite [ i ] = InvalidAddress_Write;
		
		// for templates
		//LUT_RegWrite8 [ i ] = InvalidAddress_Write;
		//LUT_RegWrite16 [ i ] = InvalidAddress_Write;
		//LUT_RegWrite32 [ i ] = InvalidAddress_Write;
	}
}



void DataBus::ConnectRegs_Read ( u32 AddressStart, PS1_BusInterface_Read CallbackFunction )
{
	// add callback
	LUT_RegRead [ ( AddressStart & 0x00003ff0 ) >> 4 ] = CallbackFunction;
}

void DataBus::ConnectRegs_Write ( u32 AddressStart, PS1_BusInterface_Write CallbackFunction )
{
	// add callback
	LUT_RegWrite [ ( AddressStart & 0x00003ff0 ) >> 4 ] = CallbackFunction;
}



u32 DataBus::InvalidAddress_Read ( u32 Address )
{
	if (((Address & 0x1fffffff) >> 8) < 0x1f8010)
	{
		// unsure if this is R3000A data cache mirror or not //
		return 0;
	}


	if (((Address & 0x1fffffff) >> 8) == 0x1f8010)
	{
		// pull from reg cache
		return _BUS->RegCache_0x1f8010 [ ( Address >> 2 ) & 0x3f ];
	}
	
#ifdef PS2_COMPILE

	if ( ( ( Address & 0x1fffffff ) >> 8 ) == 0x1f8014 )
	{
#ifdef INLINE_DEBUG_READ_INVALID_14XX
	debug << "\r\nBus::Read; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << ";INVALID; READ; ADDRESS = " << hex << setw ( 8 ) << Address;
	debug << " Data=" << _BUS->RegCache_0x1f8014 [ ( Address >> 2 ) & 0x3f ];
#endif

		// pull from reg cache
		return _BUS->RegCache_0x1f8014 [ ( Address >> 2 ) & 0x3f ];
	}

	if ( ( ( Address & 0x1fffffff ) >> 8 ) == 0x1f8015 )
	{
#ifdef INLINE_DEBUG_READ_INVALID_15XX
	debug << "\r\nBus::Read; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << ";INVALID; READ; ADDRESS = " << hex << setw ( 8 ) << Address;
	debug << " Data=" << _BUS->RegCache_0x1f8015 [ ( Address >> 2 ) & 0x3f ];
#endif

		// ALERT
		cout << "\nhps1x64: READ from invalid address. PC=" << hex << *_DebugPC << " Address=" << Address << " Cycle=" << dec << *_DebugCycleCount << " Output=" << hex << _BUS->RegCache_0x1f8015 [ ( Address >> 2 ) & 0x3f ] << "\n";
		
		// pull from reg cache
		return _BUS->RegCache_0x1f8015 [ ( Address >> 2 ) & 0x3f ];
	}

	if ( ( ( Address & 0x1fffffff ) >> 8 ) == 0x1f8016 )
	{
#ifdef INLINE_DEBUG_READ_INVALID_16XX
	debug << "\r\nBus::Read; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << ";INVALID; READ; ADDRESS = " << hex << setw ( 8 ) << Address;
	debug << " Data=" << _BUS->RegCache_0x1f8016 [ ( Address >> 2 ) & 0x3f ];
#endif

		// ALERT
		cout << "\nhps1x64: READ from invalid address. PC=" << hex << *_DebugPC << " Address=" << Address << " Cycle=" << dec << *_DebugCycleCount << " Output=" << hex << _BUS->RegCache_0x1f8016 [ ( Address >> 2 ) & 0x3f ] << "\n";
		
		// pull from reg cache
		//return _BUS->RegCache_0x1f8016 [ ( Address >> 2 ) & 0x3f ];
		
		// this is the USB, might want to just return zero for now
		return 0;
	}
	
#endif


#ifdef INLINE_DEBUG_READ_INVALID
	debug << "\r\nBus::Read; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << ";INVALID; READ; ADDRESS = " << hex << setw ( 8 ) << Address;
#endif

	if ( ( Address >> 16 ) == 0xfffe )
	{
		// cache control //
		
		return 0;
	}
	
	if ( ( Address >> 16 ) == 0x1e00 )
	{
		// ROM line ?? //
		
		return 0;
	}
	
	
	

	switch ( Address )
	{
	
		case 0x1f801000:
		
			// Expansion 1 base address (usually 0x1f000000)
			return 0x1f000000;
			
			break;
			
		case 0x1f801004:
		
			// Expansion 2 base address (usually 0x1f802000)
			return 0x1f802000;
			
			break;
			
		case 0x1f801008:
		
			// Expansion 1 Delay/Size (usually 0013243Fh; 512Kbytes 8bit-bus)
			return 0x0013243f;
			
			break;
			
		case 0x1f80100c:
		
			// Expansion 3 Delay/Size (usually 00003022h; 1 byte)
			return 0x00003022;
			
			break;
			
		case 0x1f801010:
		
			// BIOS ROM Delay/Size (usually 0013243Fh; 512Kbytes 8bit-bus)
			return 0x0013243f;
			
			break;
			
		case 0x1f801014:
		
			// SPU Delay/Size (usually 200931E1h)
			return 0x200931e1;
			
			break;
			
		case 0x1f801018:
		
			// CDROM Delay/Size (usually 00020843h or 00020943h)
			return 0x00020843;
			
			break;
			
		case 0x1f80101c:
		
			// Expansion 2 Delay/Size (usually 00070777h; 128-bytes 8bit-bus)
			return 0x00070777;

			break;
			
			
		case 0x1f801020:
		
			// COM/COMMON Delay (00031125h or 0000132Ch or 00001325h)
			return 0x00031125;

			break;
			
		
		case 0x1f802041:
		
			// POST (7-segment display output)

			break;

		case 0x1ffe0130:
		
			// cache control
			
			break;
			

#ifdef PS2_COMPILE

		case 0x1f801450:
		
			// ?? Unknown? No, no, no... !!!
			return 0;
			break;
			
		case 0x1f8014a0:
		
			// ?? Unknown? No, no, no... !!!
			return 0;
			break;
			
#endif

		default:
		
			// *** testing ***
#ifdef INLINE_DEBUG_READ_INVALID_INVALID
	debug << "\r\nBus::Read; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << ";INVALID; READ; ADDRESS = " << hex << setw ( 8 ) << Address;
#endif

			cout << "\nhps1x64: READ from invalid address. PC=" << hex << *_DebugPC << " Address=" << Address << " Cycle=" << dec << *_DebugCycleCount << "\n";
			break;
	
	}
	

	return 0;
}

void DataBus::InvalidAddress_Write ( u32 Address, u32 Data, u32 Mask )
{
	if (((Address & 0x1fffffff) >> 8) < 0x1f8010)
	{
		// unsure if this is R3000A data cache mirror or not //
		return;
	}

	if ( ( ( Address & 0x1fffffff ) >> 8 ) == 0x1f8010 )
	{
		// pull from reg cache
		_BUS->RegCache_0x1f8010 [ ( Address >> 2 ) & 0x3f ] = Data;
		return;
	}
	
#ifdef PS2_COMPILE

	if ( ( ( Address & 0x1fffffff ) >> 8 ) == 0x1f8014 )
	{
#ifdef INLINE_DEBUG_WRITE_INVALID_14XX
	debug << "\r\nBus::Write; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data;
	debug << ";INVALID; WRITE; ADDRESS = " << hex << setw ( 8 ) << Address;
#endif

		// pull from reg cache
		// 0x1f801414 -> bit 17 ?? Maybe a reset of R3000A processor?? bit 17: Reset R3000A ??
		_BUS->RegCache_0x1f8014 [ ( Address >> 2 ) & 0x3f ] = Data;
		
		return;
	}

	if ( ( ( Address & 0x1fffffff ) >> 8 ) == 0x1f8015 )
	{
#ifdef INLINE_DEBUG_WRITE_INVALID_15XX
	debug << "\r\nBus::Write; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data;
	debug << ";INVALID; WRITE; ADDRESS = " << hex << setw ( 8 ) << Address;
#endif

		// ALERT
		cout << "\nhps1x64: WRITE to invalid address. PC=" << hex << *_DebugPC << " Address=" << Address << " Cycle=" << dec << *_DebugCycleCount << " Input=" << hex << Data << "\n";
		
		// pull from reg cache
		_BUS->RegCache_0x1f8015 [ ( Address >> 2 ) & 0x3f ] = Data;
		
		return;
	}

	if ( ( ( Address & 0x1fffffff ) >> 8 ) == 0x1f8016 )
	{
#ifdef INLINE_DEBUG_WRITE_INVALID_16XX
	debug << "\r\nBus::Write; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data;
	debug << ";INVALID; WRITE; ADDRESS = " << hex << setw ( 8 ) << Address;
#endif

		// ALERT
		cout << "\nhps1x64: WRITE to invalid address. PC=" << hex << *_DebugPC << " Address=" << Address << " Cycle=" << dec << *_DebugCycleCount << " Input=" << hex << Data << "\n";
		
		// store to reg cache
		_BUS->RegCache_0x1f8016 [ ( Address >> 2 ) & 0x3f ] = Data;
		
		return;
	}
	
#endif


#ifdef INLINE_DEBUG_WRITE_INVALID
	debug << "\r\nBus::Write; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data;
	debug << ";INVALID; WRITE; ADDRESS = " << hex << setw ( 8 ) << Address;
#endif

	if ( ( Address >> 16 ) == 0x1e00 )
	{
		// ROM line ?? //
		
		return;
	}

	switch ( Address )
	{
	
		case 0x1f801000:
		
			// Expansion 1 base address (usually 0x1f000000)
			
			break;
			
		case 0x1f801004:
		
			// Expansion 2 base address (usually 0x1f802000)
			
			break;
			
		case 0x1f801008:
		
			// Expansion 1 Delay/Size (usually 0013243Fh; 512Kbytes 8bit-bus)
			
			break;
			
		case 0x1f80100c:
		
			// Expansion 3 Delay/Size (usually 00003022h; 1 byte)
			
			break;
			
		case 0x1f801010:
		
			// BIOS ROM Delay/Size (usually 0013243Fh; 512Kbytes 8bit-bus)
			
			break;
			
		case 0x1f801014:
		
			// SPU Delay/Size (usually 200931E1h)
			
			break;
			
		case 0x1f801018:
		
			// CDROM Delay/Size (usually 00020843h or 00020943h)
			
			break;
			
		case 0x1f80101c:
		
			// Expansion 2 Delay/Size (usually 00070777h; 128-bytes 8bit-bus)

			break;
			
			
		case 0x1f801020:
		
			// COM/COMMON Delay (00031125h or 0000132Ch or 00001325h)

			break;

		case 0x1f802040:
		
			// dip switches
			
			break;
		
		case 0x1f802041:
		
			// POST (7-segment display output)

			break;

			
		case 0x1f802070:
		
			// POST 2
		
			break;
			
			
		case 0x1fa00000:
		
			// POST 3
			
			break;
		
		
		case 0x1ffe0130:
		
			// cache control
			
			break;

		
#ifdef PS2_COMPILE

		case 0x1f801450:
		
			// ?? Unknown? No, no, no... !!!
			
			// IOP probably just writes zeros to this one, but wanna play it safe
			if ( Data & 0xffffffff ) cout << "\nhps1x64: DataBus: writing non-zero value to 0x1f801450!";
			break;
			
#endif


		default:
		
			// *** testing ***
#ifdef INLINE_DEBUG_WRITE_INVALID_INVALID
	debug << "\r\nBus::Write; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data;
	debug << ";INVALID; WRITE; ADDRESS = " << hex << setw ( 8 ) << Address;
#endif

			cout << "\nhps1x64: WRITE to invalid address. PC=" << hex << *_DebugPC << " Address=" << Address << " Data=" << Data << " Cycle#" << dec << *_DebugCycleCount << "\n";
			break;
	
	}
	
	return;
}


bool DataBus::isReady ()
{
#ifdef INLINE_DEBUG_ISREADY
	if ( *_DebugCycleCount > 0x16a70 )
	{
		debug << "\r\nDataBus::isReady; BusyUntil_Cycle=" << dec << BusyUntil_Cycle << "; CycleCount=" << *_DebugCycleCount << "; isReady=" << ( BusyUntil_Cycle <= *_DebugCycleCount );
	}
#endif

	return ( BusyUntil_Cycle <= *_DebugCycleCount );
}

void DataBus::ReserveBus ( u64 Cycles )
{
	if ( *_DebugCycleCount >= BusyUntil_Cycle )
	{
		BusyUntil_Cycle = Cycles + *_DebugCycleCount;
	}
	else
	{
		BusyUntil_Cycle += Cycles;
	}
}


void DataBus::ReserveBus_Latency ()
{
	if ( *_DebugCycleCount >= BusyUntil_Cycle )
	{
		BusyUntil_Cycle = Latency + *_DebugCycleCount;
	}
	else
	{
		BusyUntil_Cycle += Latency;
	}
}


#ifdef ENABLE_MEMORY_INVALIDATE

void DataBus::Reset_Invalidate ()
{
	for ( int i = 0; i < c_iInvalidate_Size / sizeof ( u64 ); i++ )
	{
		InvalidArray.b64 [ i ] = 0;
	}
}

#ifdef INVALIDATE_SIZE_BIT
void DataBus::InvalidateRange ( u32 StartAddr, u32 WordCount32 )
{
	u32 WordAddr, ShiftCount1, ShiftCount2;
	//u32 WordCount32;
	
	//WordCount32 = ( ( EndAddr >> 2 ) - ( StartAddr >> 2 ) ) + 1;
	
	WordAddr = StartAddr >> 2;
	ShiftCount1 = WordAddr & 0x3f;
	ShiftCount2 = ShiftCount1 + WordCount32;
	
	WordAddr >>= 6;
	InvalidArray.b64 [ WordAddr ] |= ( 1ULL << ( ShiftCount2 >= 64 ? 0 : ShiftCount2 ) ) - ( 1ULL << ShiftCount1 );
	WordAddr++;
	
	ShiftCount2 -= 64;
	
	while ( ShiftCount2 >= 64 )
	{
		InvalidArray.b64 [ WordAddr++ ] |= -1ULL;
		ShiftCount2 -= 64;
	}
	
	if ( ShiftCount2 > 0 )
	{
		InvalidArray.b64 [ WordAddr ] |= ( ( 1ULL << ShiftCount2 ) - 1ULL );
	}
	
}
#else
void DataBus::InvalidateRange ( u32 StartAddr, u32 WordCount32 )
{
#ifdef INLINE_DEBUG_INVALIDATE
	debug << "\r\nInvalidateRange";
	debug << " StartAddr=" << hex << StartAddr;
	debug << " WordCount32=" << dec << WordCount32;
#endif

	u32 WordAddr, WordAddr2, ShiftCount1, ShiftCount2;
	//u32 WordCount32;
	
	//WordCount32 = ( ( EndAddr >> 2 ) - ( StartAddr >> 2 ) ) + 1;
	
	WordAddr = StartAddr >> 2;
	WordAddr2 = WordAddr + WordCount32;
	
	WordAddr >>= c_iInvalidate_Shift;
	WordAddr2 >>= c_iInvalidate_Shift;
	
	while ( WordAddr <= WordAddr2 )
	{
		InvalidArray.b8 [ WordAddr & c_iInvalidate_Mask ] = 1;
		WordAddr++;
	}
}
#endif

#endif



/////////// Debugging //////////


void DataBus::DebugWindow_Enable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	const char* DebugWindow_Caption = "PS1 Memory Bus Debug Window";
	static constexpr int DebugWindow_X = 10;
	static constexpr int DebugWindow_Y = 10;
	static constexpr int DebugWindow_Width = 220;
	static constexpr int DebugWindow_Height = 220;
	
	static constexpr int MemoryViewer_Columns = 8;
	static constexpr int MemoryViewer_X = 0;
	static constexpr int MemoryViewer_Y = 0;
	static constexpr int MemoryViewer_Width = 200;
	static constexpr int MemoryViewer_Height = 200;
	
	int i;
	stringstream ss;
	
	if ( !DebugWindow_Enabled )
	{
		// create the main debug window
		DebugWindow = new WindowClass::Window ();
		DebugWindow->Create ( DebugWindow_Caption, DebugWindow_X, DebugWindow_Y, DebugWindow_Width, DebugWindow_Height );
		DebugWindow->DisableCloseButton ();
		
		MemoryViewer = new Debug_MemoryViewer ();
		
		MemoryViewer->Create ( DebugWindow, MemoryViewer_X, MemoryViewer_Y, MemoryViewer_Width, MemoryViewer_Height, MemoryViewer_Columns );
		MemoryViewer->Add_MemoryDevice ( "RAM", 0x00000000, MainMemory_Size, _BUS->MainMemory.b8 );
		MemoryViewer->Add_MemoryDevice ( "BIOS", 0x1fc00000, BIOS_Size, _BUS->BIOS.b8 );
		
	
		// mark debug as enabled now
		DebugWindow_Enabled = true;
		
		// update the value lists
		DebugWindow_Update ();
	}
	
#endif

}

void DataBus::DebugWindow_Disable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;
	
	if ( DebugWindow_Enabled )
	{
		delete DebugWindow;
		delete MemoryViewer;
	
		// disable debug window
		DebugWindow_Enabled = false;
	}
	
#endif

}

void DataBus::DebugWindow_Update ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;
	
	if ( DebugWindow_Enabled )
	{
		MemoryViewer->Update();
	}
	
#endif

}



