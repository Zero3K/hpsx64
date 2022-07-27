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


#ifdef ENABLE_GUI_DEBUGGER
#include "Debug.h"
#endif

#include <iostream>
#include <string.h>
#include "PS2DataBus.h"
#include "R5900_Print.h"
#include "PS2_SIF.h"
#include "VU.h"

#ifndef EE_ONLY_COMPILE
#include "PS1_Intc.h"
#include "PS1DataBus.h"
#endif

using namespace std;
using namespace Playstation2;
using namespace R5900::Instruction;


#define DEBUG_COUT


#define USE_SCRATCHPAD


#define ENABLE_INVALID_ARRAY


#ifdef _DEBUG_VERSION_

// enable debugging
#define INLINE_DEBUG_ENABLE

/*
#define INLINE_DEBUG_WRITE_REG
#define INLINE_DEBUG_READ_REG
#define INLINE_DEBUG_READ_INVALID
#define INLINE_DEBUG_WRITE_INVALID
*/

//#define INLINE_DEBUG_INVALIDATE

//#define INLINE_DEBUG_READ_INVALID_128
//#define INLINE_DEBUG_WRITE_INVALID_128

//#define INLINE_DEBUG_READ_MDEC
//#define INLINE_DEBUG_WRITE_MDEC

//#define INLINE_DEBUG_READ_DIRECTCACHE
//#define INLINE_DEBUG_WRITE_DIRECTCACHE
//#define INLINE_DEBUG_WRITE_REG_TIMER
//#define INLINE_DEBUG_WRITE_REG_INTC



//#define INLINE_DEBUG_READ_REG_SBUS
//#define INLINE_DEBUG_WRITE_REG_SBUS

//#define INLINE_DEBUG_READ_REG_INTC

//#define INLINE_DEBUG_READ_REG_TIMER

//#define INLINE_DEBUG_WRITE_RAM
//#define INLINE_DEBUG_READ_RAM
//#define INLINE_DEBUG_READ_BIOS
//#define INLINE_DEBUG_WRITE_SCRATCHPAD
//#define INLINE_DEBUG_READ_SCRATCHPAD
#define INLINE_DEBUG_READ_VUMEM
#define INLINE_DEBUG_WRITE_VUMEM

//#define INLINE_DEBUG_DATABUS_READ
//#define INLINE_DEBUG_DATABUS_WRITE


//#define INLINE_DEBUG_WRITE_RAMSIZE
//#define INLINE_DEBUG_READ
//#define INLINE_DEBUG_WRITE
//#define INLINE_DEBUG2_READ
//#define INLINE_DEBUG2_WRITE
//#define INLINE_DEBUG3_READ
//#define INLINE_DEBUG3_WRITE
//#define INLINE_DEBUG_ISREADY
//#define INLINE_DEBUG_RESERVE


#endif




namespace Playstation2
{


#ifdef ENABLE_GUI_DEBUGGER
Debug::Log DataBus::debug;
#endif


u32* DataBus::_DebugPC;
u64* DataBus::_DebugCycleCount;


//u32 *DataBus::DebugPC;	// to match up debug info
//Dma *DataBus::DMA_Device;
//CD *DataBus::CD_Device;
//GPU *DataBus::GPU_Device;
//Intc *DataBus::INTC_Device;
//MDEC *DataBus::MDEC_Device;
//PIO *DataBus::PIO_Device;
//SIO *DataBus::SIO_Device;
//SPU *DataBus::SPU_Device;
//Timers *DataBus::Timers_Device;


DataBus *DataBus::_BUS;


DataBus::PS2_BusInterface_Read DataBus::LUT_BusRead [ DataBus::c_LUT_Bus_Size ];
DataBus::PS2_BusInterface_Write DataBus::LUT_BusWrite [ DataBus::c_LUT_Bus_Size ];

DataBus::PS2_BusInterface_Read DataBus::LUT_BusRead8 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS2_BusInterface_Write DataBus::LUT_BusWrite8 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS2_BusInterface_Read DataBus::LUT_BusRead16 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS2_BusInterface_Write DataBus::LUT_BusWrite16 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS2_BusInterface_Read DataBus::LUT_BusRead32 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS2_BusInterface_Write DataBus::LUT_BusWrite32 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS2_BusInterface_Read DataBus::LUT_BusRead64 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS2_BusInterface_Write DataBus::LUT_BusWrite64 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS2_BusInterface_Read DataBus::LUT_BusRead128 [ DataBus::c_LUT_Bus_Size ];
DataBus::PS2_BusInterface_Write DataBus::LUT_BusWrite128 [ DataBus::c_LUT_Bus_Size ];

alignas(16) DataBus::DataBusEntry DataBus::LUT_DataBus_Read [ DataBus::c_LUT_Bus_Size ];
alignas(16) DataBus::DataBusEntry DataBus::LUT_DataBus_Write [ DataBus::c_LUT_Bus_Size ];

alignas(16) u32* DataBus::pRegRefList0x1000_Read [ 65536 >> 4 ];
alignas(16) u32* DataBus::pRegRefList0x1100_Read [ 65536 >> 4 ];
alignas(16) u32* DataBus::pRegRefList0x1200_Read [ 65536 >> 4 ];


DataBus::PS2_BusInterface_Read DataBus::LUT_RegRead [ DataBus::c_LUT_Reg_Size ];
DataBus::PS2_BusInterface_Write DataBus::LUT_RegWrite [ DataBus::c_LUT_Reg_Size ];


#ifdef ENABLE_GUI_DEBUGGER
bool DataBus::DebugWindow_Enabled;
WindowClass::Window *DataBus::DebugWindow;
Debug_MemoryViewer *DataBus::MemoryViewer;
#endif

u32 DataBus::Latency;


u64 *DataBus::MicroMem0;
u64 *DataBus::VuMem0;
u64 *DataBus::MicroMem1;
u64 *DataBus::VuMem1;


u64 DataBus::EndianTemp [ 2 ];


DataBus::MemoryPtr_Format DataBus::MainMemoryPtr;
DataBus::MemoryPtr_Format DataBus::BIOSPtr;
DataBus::MemoryPtr_Format DataBus::ScratchPadPtr;

DataBus::_Invalidate DataBus::InvalidArray;
DataBus::_Invalidate DataBus::Dummy_InvalidArray;




DataBus::DataBus ()
{
	cout << "Running BUS constructor...\n";
/*	
#ifdef INLINE_DEBUG_ENABLE
	// create debug log
	debug.Create ( "DataBus_Log.txt" );
#endif

	
	Reset ();
	
	_BUS = this;
*/
}


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
	debug.Create ( "PS2_DataBus_Log.txt" );
#endif
	
	Reset ();
	
	_BUS = this;
	
	Init_ConnectDevice ();
	Init_ConnectRegs ();
	
	// RAM goes from
	// physical address: 0x00000000-0x01ffffff
	// uncached: 0x20000000-0x21ffffff
	// uncached/accelerated: 0x30000000-0x31ffffff
	// *todo* implement TLB
	
	// physical locations (from Emotion Engine viewpoint):
	//
	// vu0 micro mem: 0x11000000
	// size vu0 micro mem: 0x1000
	// vu1 micro mem: 0x11008000
	// size vu1 micro mem: 0x4000
	//
	// rom/bios: 0x1fc00000
	// rom1/dvd player: 0x1e000000
	// erom/dvd player: 0x1e040000
	// rom2/asian font: 0x1e400000
	//
	// PS1/IOP RAM: 0x1c000000
	// PS1/IOP BUS/DEVICES: 0x1f800000 or 0xbf800000
	
	/*
	ConnectDevice_Read ( 0x00000000, Memory_Read );
	ConnectDevice_Read ( 0x00400000, Memory_Read );
	ConnectDevice_Read ( 0x00800000, Memory_Read );
	ConnectDevice_Read ( 0x00c00000, Memory_Read );
	ConnectDevice_Read ( 0x01000000, Memory_Read );
	ConnectDevice_Read ( 0x01400000, Memory_Read );
	ConnectDevice_Read ( 0x01800000, Memory_Read );
	ConnectDevice_Read ( 0x01c00000, Memory_Read );
	
	ConnectDevice_Read ( 0x20000000, Memory_Read );
	ConnectDevice_Read ( 0x20400000, Memory_Read );
	ConnectDevice_Read ( 0x20800000, Memory_Read );
	ConnectDevice_Read ( 0x20c00000, Memory_Read );
	ConnectDevice_Read ( 0x21000000, Memory_Read );
	ConnectDevice_Read ( 0x21400000, Memory_Read );
	ConnectDevice_Read ( 0x21800000, Memory_Read );
	ConnectDevice_Read ( 0x21c00000, Memory_Read );
	
	ConnectDevice_Read ( 0x30000000, Memory_Read );
	ConnectDevice_Read ( 0x30400000, Memory_Read );
	ConnectDevice_Read ( 0x30800000, Memory_Read );
	ConnectDevice_Read ( 0x30c00000, Memory_Read );
	ConnectDevice_Read ( 0x31000000, Memory_Read );
	ConnectDevice_Read ( 0x31400000, Memory_Read );
	ConnectDevice_Read ( 0x31800000, Memory_Read );
	ConnectDevice_Read ( 0x31c00000, Memory_Read );

	ConnectDevice_Read ( 0x80000000, Memory_Read );
	ConnectDevice_Read ( 0x80400000, Memory_Read );
	ConnectDevice_Read ( 0x80800000, Memory_Read );
	ConnectDevice_Read ( 0x80c00000, Memory_Read );
	ConnectDevice_Read ( 0x81000000, Memory_Read );
	ConnectDevice_Read ( 0x81400000, Memory_Read );
	ConnectDevice_Read ( 0x81800000, Memory_Read );
	ConnectDevice_Read ( 0x81c00000, Memory_Read );

	ConnectDevice_Read ( 0xa0000000, Memory_Read );
	ConnectDevice_Read ( 0xa0400000, Memory_Read );
	ConnectDevice_Read ( 0xa0800000, Memory_Read );
	ConnectDevice_Read ( 0xa0c00000, Memory_Read );
	ConnectDevice_Read ( 0xa1000000, Memory_Read );
	ConnectDevice_Read ( 0xa1400000, Memory_Read );
	ConnectDevice_Read ( 0xa1800000, Memory_Read );
	ConnectDevice_Read ( 0xa1c00000, Memory_Read );
	*/
	
	// main memory ram //
	
	// cached
	ConnectDevice_Read ( 0x00000000, 0x01c00000, Memory_Read );
	
	
	// uncached
	ConnectDevice_Read ( 0x20000000, 0x21c00000, Memory_Read );
	
	
	// uncached accelerated
	ConnectDevice_Read ( 0x30000000, 0x31c00000, Memory_Read );
	
	// probably cached
	ConnectDevice_Read ( 0x80000000, 0x81c00000, Memory_Read );
	
	// probably uncached
	ConnectDevice_Read ( 0xa0000000, 0xa1c00000, Memory_Read );
	
	ConnectDevice_Read_t<0xff> ( 0x00000000, 0x01c00000, Memory_Read_t<0xff> );
	ConnectDevice_Read_t<0xff> ( 0x20000000, 0x21c00000, Memory_Read_t<0xff> );
	ConnectDevice_Read_t<0xff> ( 0x30000000, 0x31c00000, Memory_Read_t<0xff> );
	ConnectDevice_Read_t<0xff> ( 0x80000000, 0x81c00000, Memory_Read_t<0xff> );
	ConnectDevice_Read_t<0xff> ( 0xa0000000, 0xa1c00000, Memory_Read_t<0xff> );

	ConnectDevice_Read_t<0xffff> ( 0x00000000, 0x01c00000, Memory_Read_t<0xffff> );
	ConnectDevice_Read_t<0xffff> ( 0x20000000, 0x21c00000, Memory_Read_t<0xffff> );
	ConnectDevice_Read_t<0xffff> ( 0x30000000, 0x31c00000, Memory_Read_t<0xffff> );
	ConnectDevice_Read_t<0xffff> ( 0x80000000, 0x81c00000, Memory_Read_t<0xffff> );
	ConnectDevice_Read_t<0xffff> ( 0xa0000000, 0xa1c00000, Memory_Read_t<0xffff> );
	
	ConnectDevice_Read_t<0xffffffff> ( 0x00000000, 0x01c00000, Memory_Read_t<0xffffffff> );
	ConnectDevice_Read_t<0xffffffff> ( 0x20000000, 0x21c00000, Memory_Read_t<0xffffffff> );
	ConnectDevice_Read_t<0xffffffff> ( 0x30000000, 0x31c00000, Memory_Read_t<0xffffffff> );
	ConnectDevice_Read_t<0xffffffff> ( 0x80000000, 0x81c00000, Memory_Read_t<0xffffffff> );
	ConnectDevice_Read_t<0xffffffff> ( 0xa0000000, 0xa1c00000, Memory_Read_t<0xffffffff> );

	ConnectDevice_Read_t<0xffffffffffffffffULL> ( 0x00000000, 0x01c00000, Memory_Read_t<0xffffffffffffffffULL> );
	ConnectDevice_Read_t<0xffffffffffffffffULL> ( 0x20000000, 0x21c00000, Memory_Read_t<0xffffffffffffffffULL> );
	ConnectDevice_Read_t<0xffffffffffffffffULL> ( 0x30000000, 0x31c00000, Memory_Read_t<0xffffffffffffffffULL> );
	ConnectDevice_Read_t<0xffffffffffffffffULL> ( 0x80000000, 0x81c00000, Memory_Read_t<0xffffffffffffffffULL> );
	ConnectDevice_Read_t<0xffffffffffffffffULL> ( 0xa0000000, 0xa1c00000, Memory_Read_t<0xffffffffffffffffULL> );

	ConnectDevice_Read_t<0> ( 0x00000000, 0x01c00000, Memory_Read_t<0> );
	ConnectDevice_Read_t<0> ( 0x20000000, 0x21c00000, Memory_Read_t<0> );
	ConnectDevice_Read_t<0> ( 0x30000000, 0x31c00000, Memory_Read_t<0> );
	ConnectDevice_Read_t<0> ( 0x80000000, 0x81c00000, Memory_Read_t<0> );
	ConnectDevice_Read_t<0> ( 0xa0000000, 0xa1c00000, Memory_Read_t<0> );

	

	/*
	ConnectDevice_Write ( 0x00000000, Memory_Write );
	ConnectDevice_Write ( 0x00400000, Memory_Write );
	ConnectDevice_Write ( 0x00800000, Memory_Write );
	ConnectDevice_Write ( 0x00c00000, Memory_Write );
	ConnectDevice_Write ( 0x01000000, Memory_Write );
	ConnectDevice_Write ( 0x01400000, Memory_Write );
	ConnectDevice_Write ( 0x01800000, Memory_Write );
	ConnectDevice_Write ( 0x01c00000, Memory_Write );
	
	ConnectDevice_Write ( 0x20000000, Memory_Write );
	ConnectDevice_Write ( 0x20400000, Memory_Write );
	ConnectDevice_Write ( 0x20800000, Memory_Write );
	ConnectDevice_Write ( 0x20c00000, Memory_Write );
	ConnectDevice_Write ( 0x21000000, Memory_Write );
	ConnectDevice_Write ( 0x21400000, Memory_Write );
	ConnectDevice_Write ( 0x21800000, Memory_Write );
	ConnectDevice_Write ( 0x21c00000, Memory_Write );
	
	ConnectDevice_Write ( 0x30000000, Memory_Write );
	ConnectDevice_Write ( 0x30400000, Memory_Write );
	ConnectDevice_Write ( 0x30800000, Memory_Write );
	ConnectDevice_Write ( 0x30c00000, Memory_Write );
	ConnectDevice_Write ( 0x31000000, Memory_Write );
	ConnectDevice_Write ( 0x31400000, Memory_Write );
	ConnectDevice_Write ( 0x31800000, Memory_Write );
	ConnectDevice_Write ( 0x31c00000, Memory_Write );

	ConnectDevice_Write ( 0x80000000, Memory_Write );
	ConnectDevice_Write ( 0x80400000, Memory_Write );
	ConnectDevice_Write ( 0x80800000, Memory_Write );
	ConnectDevice_Write ( 0x80c00000, Memory_Write );
	ConnectDevice_Write ( 0x81000000, Memory_Write );
	ConnectDevice_Write ( 0x81400000, Memory_Write );
	ConnectDevice_Write ( 0x81800000, Memory_Write );
	ConnectDevice_Write ( 0x81c00000, Memory_Write );

	ConnectDevice_Write ( 0xa0000000, Memory_Write );
	ConnectDevice_Write ( 0xa0400000, Memory_Write );
	ConnectDevice_Write ( 0xa0800000, Memory_Write );
	ConnectDevice_Write ( 0xa0c00000, Memory_Write );
	ConnectDevice_Write ( 0xa1000000, Memory_Write );
	ConnectDevice_Write ( 0xa1400000, Memory_Write );
	ConnectDevice_Write ( 0xa1800000, Memory_Write );
	ConnectDevice_Write ( 0xa1c00000, Memory_Write );
	*/
	
	ConnectDevice_Write ( 0x00000000, 0x01c00000, Memory_Write );
	ConnectDevice_Write ( 0x20000000, 0x21c00000, Memory_Write );
	ConnectDevice_Write ( 0x30000000, 0x31c00000, Memory_Write );
	ConnectDevice_Write ( 0x80000000, 0x81c00000, Memory_Write );
	ConnectDevice_Write ( 0xa0000000, 0xa1c00000, Memory_Write );
	
	ConnectDevice_Write_t<0xff> ( 0x00000000, 0x01c00000, Memory_Write_t<0xff> );
	ConnectDevice_Write_t<0xff> ( 0x20000000, 0x21c00000, Memory_Write_t<0xff> );
	ConnectDevice_Write_t<0xff> ( 0x30000000, 0x31c00000, Memory_Write_t<0xff> );
	ConnectDevice_Write_t<0xff> ( 0x80000000, 0x81c00000, Memory_Write_t<0xff> );
	ConnectDevice_Write_t<0xff> ( 0xa0000000, 0xa1c00000, Memory_Write_t<0xff> );

	ConnectDevice_Write_t<0xffff> ( 0x00000000, 0x01c00000, Memory_Write_t<0xffff> );
	ConnectDevice_Write_t<0xffff> ( 0x20000000, 0x21c00000, Memory_Write_t<0xffff> );
	ConnectDevice_Write_t<0xffff> ( 0x30000000, 0x31c00000, Memory_Write_t<0xffff> );
	ConnectDevice_Write_t<0xffff> ( 0x80000000, 0x81c00000, Memory_Write_t<0xffff> );
	ConnectDevice_Write_t<0xffff> ( 0xa0000000, 0xa1c00000, Memory_Write_t<0xffff> );
	
	ConnectDevice_Write_t<0xffffffff> ( 0x00000000, 0x01c00000, Memory_Write_t<0xffffffff> );
	ConnectDevice_Write_t<0xffffffff> ( 0x20000000, 0x21c00000, Memory_Write_t<0xffffffff> );
	ConnectDevice_Write_t<0xffffffff> ( 0x30000000, 0x31c00000, Memory_Write_t<0xffffffff> );
	ConnectDevice_Write_t<0xffffffff> ( 0x80000000, 0x81c00000, Memory_Write_t<0xffffffff> );
	ConnectDevice_Write_t<0xffffffff> ( 0xa0000000, 0xa1c00000, Memory_Write_t<0xffffffff> );

	ConnectDevice_Write_t<0xffffffffffffffffULL> ( 0x00000000, 0x01c00000, Memory_Write_t<0xffffffffffffffffULL> );
	ConnectDevice_Write_t<0xffffffffffffffffULL> ( 0x20000000, 0x21c00000, Memory_Write_t<0xffffffffffffffffULL> );
	ConnectDevice_Write_t<0xffffffffffffffffULL> ( 0x30000000, 0x31c00000, Memory_Write_t<0xffffffffffffffffULL> );
	ConnectDevice_Write_t<0xffffffffffffffffULL> ( 0x80000000, 0x81c00000, Memory_Write_t<0xffffffffffffffffULL> );
	ConnectDevice_Write_t<0xffffffffffffffffULL> ( 0xa0000000, 0xa1c00000, Memory_Write_t<0xffffffffffffffffULL> );

	ConnectDevice_Write_t<0> ( 0x00000000, 0x01c00000, Memory_Write_t<0> );
	ConnectDevice_Write_t<0> ( 0x20000000, 0x21c00000, Memory_Write_t<0> );
	ConnectDevice_Write_t<0> ( 0x30000000, 0x31c00000, Memory_Write_t<0> );
	ConnectDevice_Write_t<0> ( 0x80000000, 0x81c00000, Memory_Write_t<0> );
	ConnectDevice_Write_t<0> ( 0xa0000000, 0xa1c00000, Memory_Write_t<0> );

	// set LUTs for recompiler
	ConnectDevice_Read2 ( 0x00000000, 0x01c00000, MainMemory.b32, MainMemory_Mask, c_iRAM_Read_Latency );
	ConnectDevice_Read2 ( 0x20000000, 0x21c00000, MainMemory.b32, MainMemory_Mask, c_iRAM_Read_Latency );
	ConnectDevice_Read2 ( 0x30000000, 0x31c00000, MainMemory.b32, MainMemory_Mask, c_iRAM_Read_Latency );
	ConnectDevice_Read2 ( 0x80000000, 0x81c00000, MainMemory.b32, MainMemory_Mask, c_iRAM_Read_Latency );
	ConnectDevice_Read2 ( 0xa0000000, 0xa1c00000, MainMemory.b32, MainMemory_Mask, c_iRAM_Read_Latency );
	
	ConnectDevice_Write2 ( 0x00000000, 0x01c00000, MainMemory.b32, MainMemory_Mask, InvalidArray.b8, c_iRAM_Read_Latency );
	ConnectDevice_Write2 ( 0x20000000, 0x21c00000, MainMemory.b32, MainMemory_Mask, InvalidArray.b8, c_iRAM_Read_Latency );
	ConnectDevice_Write2 ( 0x30000000, 0x31c00000, MainMemory.b32, MainMemory_Mask, InvalidArray.b8, c_iRAM_Read_Latency );
	ConnectDevice_Write2 ( 0x80000000, 0x81c00000, MainMemory.b32, MainMemory_Mask, InvalidArray.b8, c_iRAM_Read_Latency );
	ConnectDevice_Write2 ( 0xa0000000, 0xa1c00000, MainMemory.b32, MainMemory_Mask, InvalidArray.b8, c_iRAM_Read_Latency );
	
	
	// bios has same addresses, but 4MB range
	
	/*
	ConnectDevice_Read ( 0x1fc00000, BIOS_Read );
	ConnectDevice_Read ( 0x9fc00000, BIOS_Read );
	ConnectDevice_Read ( 0xbfc00000, BIOS_Read );
	*/
	
	
	// bios //

	// uncached
	ConnectDevice_Read ( 0x1fc00000, 0x1fc00000, BIOS_Read );
	
	// cached
	ConnectDevice_Read ( 0x9fc00000, 0x9fc00000, BIOS_Read );
	
	// uncached
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

	ConnectDevice_Read_t<0xffffffffffffffffULL> ( 0x1fc00000, 0x1fc00000, BIOS_Read_t<0xffffffffffffffffULL> );
	ConnectDevice_Read_t<0xffffffffffffffffULL> ( 0x9fc00000, 0x9fc00000, BIOS_Read_t<0xffffffffffffffffULL> );
	ConnectDevice_Read_t<0xffffffffffffffffULL> ( 0xbfc00000, 0xbfc00000, BIOS_Read_t<0xffffffffffffffffULL> );

	ConnectDevice_Read_t<0> ( 0x1fc00000, 0x1fc00000, BIOS_Read_t<0> );
	ConnectDevice_Read_t<0> ( 0x9fc00000, 0x9fc00000, BIOS_Read_t<0> );
	ConnectDevice_Read_t<0> ( 0xbfc00000, 0xbfc00000, BIOS_Read_t<0> );


	// set LUTs for recompiler
	ConnectDevice_Read2 ( 0x1fc00000, 0x1fc00000, BIOS.b32, BIOS_Mask, c_iBIOS_Read_Latency );
	ConnectDevice_Read2 ( 0x9fc00000, 0x9fc00000, BIOS.b32, BIOS_Mask, c_iBIOS_Read_Latency );
	ConnectDevice_Read2 ( 0xbfc00000, 0xbfc00000, BIOS.b32, BIOS_Mask, c_iBIOS_Read_Latency );

	
	
	// ***todo*** there is also 0x1c000000 which maps into the IOP memory
	
	
	// there is also a scratch pad ram
	// goes from 0x70000000-0x70003fff
	// will need to mark this as being special, since it is within the Emotion Engine processor
	
	ConnectDevice_Read ( 0x70000000, 0x70000000, ScratchPad_Read );

	ConnectDevice_Read_t<0xff> ( 0x70000000, 0x70000000, ScratchPad_Read_t<0xff> );
	ConnectDevice_Read_t<0xffff> ( 0x70000000, 0x70000000, ScratchPad_Read_t<0xffff> );
	ConnectDevice_Read_t<0xffffffff> ( 0x70000000, 0x70000000, ScratchPad_Read_t<0xffffffff> );
	ConnectDevice_Read_t<0xffffffffffffffffULL> ( 0x70000000, 0x70000000, ScratchPad_Read_t<0xffffffffffffffffULL> );
	ConnectDevice_Read_t<0> ( 0x70000000, 0x70000000, ScratchPad_Read_t<0> );
	
	
	ConnectDevice_Write ( 0x70000000, 0x70000000, ScratchPad_Write );

	ConnectDevice_Write_t<0xff> ( 0x70000000, 0x70000000, ScratchPad_Write_t<0xff> );
	ConnectDevice_Write_t<0xffff> ( 0x70000000, 0x70000000, ScratchPad_Write_t<0xffff> );
	ConnectDevice_Write_t<0xffffffff> ( 0x70000000, 0x70000000, ScratchPad_Write_t<0xffffffff> );
	ConnectDevice_Write_t<0xffffffffffffffffULL> ( 0x70000000, 0x70000000, ScratchPad_Write_t<0xffffffffffffffffULL> );
	ConnectDevice_Write_t<0> ( 0x70000000, 0x70000000, ScratchPad_Write_t<0> );


	// set LUTs for recompiler
	ConnectDevice_Read2 ( 0x70000000, 0x70000000, ScratchPad.b32, ScratchPad_Mask, c_iRAM_Read_Latency );
	ConnectDevice_Write2 ( 0x70000000, 0x70000000, ScratchPad.b32, ScratchPad_Mask, Dummy_InvalidArray.b8, c_iRAM_Read_Latency );

	
	
	// looks like can also use 0xfXXX XXXX to access scratch pad

	ConnectDevice_Read ( 0xffff8000, 0xffff8000, DirectCacheMem_Read );
	ConnectDevice_Write ( 0xffff8000, 0xffff8000, DirectCacheMem_Write );
	
	// looks like the R5900 is referencing hardware registers as 0xb0XX XXXX and 0xb2XX XXXX
	
	ConnectDevice_Read ( 0xb0000000, 0xb0000000, Device_Read );
	ConnectDevice_Write ( 0xb0000000, 0xb0000000, Device_Write );
	ConnectDevice_Read ( 0xb2000000, 0xb2000000, Device_Read );
	ConnectDevice_Write ( 0xb2000000, 0xb2000000, Device_Write );

	// but can also use 0x10XX XXXX and 0x12XX XXXX
	
	ConnectDevice_Read ( 0x10000000, 0x10000000, Device_Read );
	ConnectDevice_Write ( 0x10000000, 0x10000000, Device_Write );

	
	// also need to connect the GPU for Playstation2
	
	ConnectDevice_Read ( 0x12000000, 0x12000000, Device_Read );
	ConnectDevice_Write ( 0x12000000, 0x12000000, Device_Write );
	
	// also include the vu/micromem

	ConnectDevice_Read ( 0xb1000000, 0xb1000000, VuMem_Read );
	ConnectDevice_Write ( 0xb1000000, 0xb1000000, VuMem_Write );
	ConnectDevice_Read ( 0x11000000, 0x11000000, VuMem_Read );
	ConnectDevice_Write ( 0x11000000, 0x11000000, VuMem_Write );
	
	
	// add in pointers for the registers
	ConnectRegs_Read2 ( 0x10000000, 0x10000000, pRegRefList0x1000_Read );
	ConnectRegs_Read2 ( 0xb0000000, 0xb0000000, pRegRefList0x1000_Read );
	ConnectRegs_Read2 ( 0x12000000, 0x12000000, pRegRefList0x1200_Read );
	ConnectRegs_Read2 ( 0xb2000000, 0xb2000000, pRegRefList0x1200_Read );
	ConnectRegs_Read2 ( 0x11000000, 0x11000000, pRegRefList0x1100_Read );
	ConnectRegs_Read2 ( 0xb1000000, 0xb1000000, pRegRefList0x1100_Read );
	
	
	// refresh dma data
	//Playstation2::Dma::Refresh ();
	
	// dma channel#0
	Add_RegReg_0x1000_Read ( 0x10008000, & Playstation2::Dma::pRegData [ 0 ]->CHCR.Value );
	Add_RegReg_0x1000_Read ( 0x10008010, & Playstation2::Dma::pRegData [ 0 ]->MADR.Value );
	Add_RegReg_0x1000_Read ( 0x10008020, & Playstation2::Dma::pRegData [ 0 ]->QWC.Value );
	Add_RegReg_0x1000_Read ( 0x10008030, & Playstation2::Dma::pRegData [ 0 ]->TADR.Value );
	Add_RegReg_0x1000_Read ( 0x10008040, & Playstation2::Dma::pRegData [ 0 ]->ASR0.Value );
	Add_RegReg_0x1000_Read ( 0x10008050, & Playstation2::Dma::pRegData [ 0 ]->ASR1.Value );

	Add_RegReg_0x1000_Read ( 0x10009000, & Playstation2::Dma::pRegData [ 1 ]->CHCR.Value );
	Add_RegReg_0x1000_Read ( 0x10009010, & Playstation2::Dma::pRegData [ 1 ]->MADR.Value );
	Add_RegReg_0x1000_Read ( 0x10009020, & Playstation2::Dma::pRegData [ 1 ]->QWC.Value );
	Add_RegReg_0x1000_Read ( 0x10009030, & Playstation2::Dma::pRegData [ 1 ]->TADR.Value );
	Add_RegReg_0x1000_Read ( 0x10009040, & Playstation2::Dma::pRegData [ 1 ]->ASR0.Value );
	Add_RegReg_0x1000_Read ( 0x10009050, & Playstation2::Dma::pRegData [ 1 ]->ASR1.Value );
	
	Add_RegReg_0x1000_Read ( 0x1000a000, & Playstation2::Dma::pRegData [ 2 ]->CHCR.Value );
	Add_RegReg_0x1000_Read ( 0x1000a010, & Playstation2::Dma::pRegData [ 2 ]->MADR.Value );
	Add_RegReg_0x1000_Read ( 0x1000a020, & Playstation2::Dma::pRegData [ 2 ]->QWC.Value );
	Add_RegReg_0x1000_Read ( 0x1000a030, & Playstation2::Dma::pRegData [ 2 ]->TADR.Value );
	Add_RegReg_0x1000_Read ( 0x1000a040, & Playstation2::Dma::pRegData [ 2 ]->ASR0.Value );
	Add_RegReg_0x1000_Read ( 0x1000a050, & Playstation2::Dma::pRegData [ 2 ]->ASR1.Value );

	Add_RegReg_0x1000_Read ( 0x1000b000, & Playstation2::Dma::pRegData [ 3 ]->CHCR.Value );
	Add_RegReg_0x1000_Read ( 0x1000b010, & Playstation2::Dma::pRegData [ 3 ]->MADR.Value );
	Add_RegReg_0x1000_Read ( 0x1000b020, & Playstation2::Dma::pRegData [ 3 ]->QWC.Value );
	//Add_RegReg_0x1000_Read ( 0x1000b030, & Playstation2::Dma::pRegData [ 3 ]->TADR.Value );
	//Add_RegReg_0x1000_Read ( 0x1000b040, & Playstation2::Dma::pRegData [ 3 ]->ASR0.Value );
	//Add_RegReg_0x1000_Read ( 0x1000b050, & Playstation2::Dma::pRegData [ 3 ]->ASR1.Value );

	Add_RegReg_0x1000_Read ( 0x1000b400, & Playstation2::Dma::pRegData [ 4 ]->CHCR.Value );
	Add_RegReg_0x1000_Read ( 0x1000b410, & Playstation2::Dma::pRegData [ 4 ]->MADR.Value );
	Add_RegReg_0x1000_Read ( 0x1000b420, & Playstation2::Dma::pRegData [ 4 ]->QWC.Value );
	Add_RegReg_0x1000_Read ( 0x1000b430, & Playstation2::Dma::pRegData [ 4 ]->TADR.Value );
	//Add_RegReg_0x1000_Read ( 0x1000b440, & Playstation2::Dma::pRegData [ 4 ]->ASR0.Value );
	//Add_RegReg_0x1000_Read ( 0x1000b450, & Playstation2::Dma::pRegData [ 4 ]->ASR1.Value );

	Add_RegReg_0x1000_Read ( 0x1000c000, & Playstation2::Dma::pRegData [ 5 ]->CHCR.Value );
	Add_RegReg_0x1000_Read ( 0x1000c010, & Playstation2::Dma::pRegData [ 5 ]->MADR.Value );
	Add_RegReg_0x1000_Read ( 0x1000c020, & Playstation2::Dma::pRegData [ 5 ]->QWC.Value );
	//Add_RegReg_0x1000_Read ( 0x1000c030, & Playstation2::Dma::pRegData [ 5 ]->TADR.Value );
	//Add_RegReg_0x1000_Read ( 0x1000c040, & Playstation2::Dma::pRegData [ 5 ]->ASR0.Value );
	//Add_RegReg_0x1000_Read ( 0x1000c050, & Playstation2::Dma::pRegData [ 5 ]->ASR1.Value );

	Add_RegReg_0x1000_Read ( 0x1000c400, & Playstation2::Dma::pRegData [ 6 ]->CHCR.Value );
	Add_RegReg_0x1000_Read ( 0x1000c410, & Playstation2::Dma::pRegData [ 6 ]->MADR.Value );
	Add_RegReg_0x1000_Read ( 0x1000c420, & Playstation2::Dma::pRegData [ 6 ]->QWC.Value );
	Add_RegReg_0x1000_Read ( 0x1000c430, & Playstation2::Dma::pRegData [ 6 ]->TADR.Value );
	//Add_RegReg_0x1000_Read ( 0x1000c440, & Playstation2::Dma::pRegData [ 6 ]->ASR0.Value );
	//Add_RegReg_0x1000_Read ( 0x1000c450, & Playstation2::Dma::pRegData [ 6 ]->ASR1.Value );

	Add_RegReg_0x1000_Read ( 0x1000c800, & Playstation2::Dma::pRegData [ 7 ]->CHCR.Value );
	Add_RegReg_0x1000_Read ( 0x1000c810, & Playstation2::Dma::pRegData [ 7 ]->MADR.Value );
	Add_RegReg_0x1000_Read ( 0x1000c820, & Playstation2::Dma::pRegData [ 7 ]->QWC.Value );
	//Add_RegReg_0x1000_Read ( 0x1000c830, & Playstation2::Dma::pRegData [ 7 ]->TADR.Value );
	//Add_RegReg_0x1000_Read ( 0x1000c840, & Playstation2::Dma::pRegData [ 7 ]->ASR0.Value );
	//Add_RegReg_0x1000_Read ( 0x1000c850, & Playstation2::Dma::pRegData [ 7 ]->ASR1.Value );

	Add_RegReg_0x1000_Read ( 0x1000d000, & Playstation2::Dma::pRegData [ 8 ]->CHCR.Value );
	Add_RegReg_0x1000_Read ( 0x1000d010, & Playstation2::Dma::pRegData [ 8 ]->MADR.Value );
	Add_RegReg_0x1000_Read ( 0x1000d020, & Playstation2::Dma::pRegData [ 8 ]->QWC.Value );
	//Add_RegReg_0x1000_Read ( 0x1000d030, & Playstation2::Dma::pRegData [ 8 ]->TADR.Value );
	//Add_RegReg_0x1000_Read ( 0x1000d040, & Playstation2::Dma::pRegData [ 8 ]->ASR0.Value );
	//Add_RegReg_0x1000_Read ( 0x1000d050, & Playstation2::Dma::pRegData [ 8 ]->ASR1.Value );
	Add_RegReg_0x1000_Read ( 0x1000d080, & Playstation2::Dma::pRegData [ 8 ]->SADR.Value );

	Add_RegReg_0x1000_Read ( 0x1000d400, & Playstation2::Dma::pRegData [ 9 ]->CHCR.Value );
	Add_RegReg_0x1000_Read ( 0x1000d410, & Playstation2::Dma::pRegData [ 9 ]->MADR.Value );
	Add_RegReg_0x1000_Read ( 0x1000d420, & Playstation2::Dma::pRegData [ 9 ]->QWC.Value );
	//Add_RegReg_0x1000_Read ( 0x1000d430, & Playstation2::Dma::pRegData [ 9 ]->TADR.Value );
	//Add_RegReg_0x1000_Read ( 0x1000d440, & Playstation2::Dma::pRegData [ 9 ]->ASR0.Value );
	//Add_RegReg_0x1000_Read ( 0x1000d450, & Playstation2::Dma::pRegData [ 9 ]->ASR1.Value );
	Add_RegReg_0x1000_Read ( 0x1000d480, & Playstation2::Dma::pRegData [ 9 ]->SADR.Value );
	
	Add_RegReg_0x1000_Read ( 0x1000e000, & Playstation2::Dma::_DMA->DMARegs.CTRL.Value );
	Add_RegReg_0x1000_Read ( 0x1000e010, & Playstation2::Dma::_DMA->DMARegs.STAT.Value );
	Add_RegReg_0x1000_Read ( 0x1000e020, & Playstation2::Dma::_DMA->DMARegs.PCR.Value );
	Add_RegReg_0x1000_Read ( 0x1000e030, & Playstation2::Dma::_DMA->DMARegs.SQWC.Value );
	Add_RegReg_0x1000_Read ( 0x1000e040, & Playstation2::Dma::_DMA->DMARegs.RBSR.Value );
	Add_RegReg_0x1000_Read ( 0x1000e050, & Playstation2::Dma::_DMA->DMARegs.RBOR );
	Add_RegReg_0x1000_Read ( 0x1000e060, & Playstation2::Dma::_DMA->DMARegs.STADR );

	Add_RegReg_0x1000_Read ( 0x1000f000, & Playstation2::Intc::_INTC->I_STAT_Reg.Value );
	Add_RegReg_0x1000_Read ( 0x1000f010, & Playstation2::Intc::_INTC->I_MASK_Reg.Value );

	Add_RegReg_0x1000_Read ( 0x1000f520, & Playstation2::Dma::_DMA->DMARegs.ENABLER );
	
	u32 Addr;
	u32 lReg;
	for ( Addr = 0x10003800; Addr <= 0x10003970; Addr += 16 )
	{
		lReg = ( Addr >> 4 ) & 0x1f;
		
		Add_RegReg_0x1000_Read ( Addr, & Playstation2::VU::_VU[0]->VifRegs.Regs [ lReg ] );
	}
	
	for ( Addr = 0x10003c00; Addr <= 0x10003d70; Addr += 16 )
	{
		lReg = ( Addr >> 4 ) & 0x1f;
		
		Add_RegReg_0x1000_Read ( Addr, & Playstation2::VU::_VU[1]->VifRegs.Regs [ lReg ] );
	}
	
	// gif regs
	for ( Addr = 0x10003000; Addr <= 0x100030a0; Addr += 16 )
	{
		lReg = ( Addr >> 4 ) & 0x1f;
	
		// but not STAT
		if ( Addr != 0x10003020 )
		{
			Add_RegReg_0x1000_Read ( Addr, & Playstation2::GPU::_GPU->GIFRegs.Regs [ lReg ] );
		}
	}
	
	// gpu priveleged regs group 0
	for ( Addr = 0x12000000; Addr <= 0x120000e0; Addr += 16 )
	{
		lReg = ( Addr >> 4 ) & 0x1f;
	
		// skip CSR though
		if ( Addr != 0x12000000 )
		{
			Add_RegReg_0x1200_Read ( Addr + 0x0000, (u32*) & Playstation2::GPU::_GPU->GPURegs0.Regs [ lReg ] );
			Add_RegReg_0x1200_Read ( Addr + 0x0400, (u32*) & Playstation2::GPU::_GPU->GPURegs0.Regs [ lReg ] );
			Add_RegReg_0x1200_Read ( Addr + 0x0800, (u32*) & Playstation2::GPU::_GPU->GPURegs0.Regs [ lReg ] );
		}
	}
	
	// gpu priveleged regs group 1
	for ( Addr = 0x12001000; Addr <= 0x12001080; Addr += 16 )
	{
		lReg = ( Addr >> 4 ) & 0x1f;
	
		Add_RegReg_0x1200_Read ( Addr + 0x0000, (u32*) & Playstation2::GPU::_GPU->GPURegs1.Regs [ lReg ] );
		Add_RegReg_0x1200_Read ( Addr + 0x0400, (u32*) & Playstation2::GPU::_GPU->GPURegs1.Regs [ lReg ] );
		Add_RegReg_0x1200_Read ( Addr + 0x0800, (u32*) & Playstation2::GPU::_GPU->GPURegs1.Regs [ lReg ] );
	}
	
	
	
	//Add_RegReg_0x1000_Read ( pRegRefList0x1000_Read, 0 );
	//Add_RegReg_0x1100_Read ( pRegRefList0x1100_Read, 0 );
	//Add_RegReg_0x1200_Read ( pRegRefList0x1200_Read, 0 );
	
	
	// testing intc
	lINTC = 0;	//0x7fff;
	//lSBUS_F240 = 0x2000;
	lSBUS_F260 = 0x1d000060;
	lDMAC_ENABLE = 0x1201;
	
	// connect hardware registers
	
	// no PIO on Playstation2?
	
	
	
	
	/*
	// the SPU2 device is connected to the Playstation 1 part of the Playstation2 system
	// SPU
	*/
}


void DataBus::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( DataBus ) );
	
	// set static pointer into main memory
	MainMemoryPtr.b32 = MainMemory.b32;
	
	// set static pointer into bios
	BIOSPtr.b32 = BIOS.b32;
	
	ScratchPadPtr.b32 = ScratchPad.b32;
	
	// Bus is not busy yet
	//BusyCycles = 0;
}


void DataBus::ConnectDevices ()
{
	//DMA_Device = _Dma_Device;
	//CD_Device = _CD_Device;
	//GPU_Device = _GPU_Device;
	//INTC_Device = _INTC_Device;
	//MDEC_Device = _MDEC_Device;
	//PIO_Device = _PIO_Device;
	//SIO_Device = _SIO_Device;
	//SPU_Device = _SPU_Device;
	//Timers_Device = _Timers_Device;
}


DataBus::~DataBus ()
{
	//delete MainMemory.b32;
	//delete BIOS.b32;
}


/*
static u32 DataBus::RamSize_Read ( u32 Address )
{
	Latency = c_iReg_Read_Latency;
	return _BUS->RamSize;
}

static void DataBus::RamSize_Write ( u32 Address, u32 Data, u32 Mask )
{
	u32 ShiftAmount, ShiftMask;
	ShiftAmount = ( ( Address & 0x3 ) << 3 );
	ShiftMask = ( Mask << ShiftAmount );
	_BUS->RamSize = ( _BUS->RamSize & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
}
*/

u64 DataBus::Memory_Read ( u32 Address, u64 Mask )
{
	// this is main ram
#ifdef INLINE_DEBUG_READ_RAM
	debug << "\r\nREAD; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "; RAM; Data = " << hex << (_BUS->MainMemory.b32 [ ( Address & MainMemory_Mask ) >> 2 ]) << dec << ";";
#endif

	Latency = c_iRAM_Read_Latency;
	
	// this area is a maximum of 32 MB
	Address &= MainMemory_Mask;
	
	if ( !Mask )
	{
		// 128-bit read from memory
		return (u64) ( & ( _BUS->MainMemory.b64 [ Address >> 3 ] ) );
	}

	//return ( ( _BUS->MainMemory.b32 [ Address >> 2 ] ) >> ( ( Address & 0x3 ) << 3 ) );
	return ( ( _BUS->MainMemory.b64 [ Address >> 3 ] ) >> ( ( Address & 0x7 ) << 3 ) );

}

void DataBus::Memory_Write ( u32 Address, u64 Data, u64 Mask )
{
	// this is main ram
#ifdef INLINE_DEBUG_WRITE_RAM
	debug << "\r\nWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data << "; Mask=" << Mask;
	debug << "; RAM";
#endif

	// this area is a maximum of 32 MB
	Address &= MainMemory_Mask;
	
	if ( !Mask )
	{
		// 128-bit write to memory
		//_BUS->MainMemory.b64 [ Address >> 3 ] = ((u64*)Data) [ 1 ];
		//_BUS->MainMemory.b64 [ ( Address >> 3 ) + 1 ] = ((u64*)Data) [ 0 ];
		
		_BUS->MainMemory.b64 [ Address >> 3 ] = ((u64*)Data) [ 0 ];
		_BUS->MainMemory.b64 [ ( Address >> 3 ) + 1 ] = ((u64*)Data) [ 1 ];
		
		//_BUS->MainMemory.b32 [ Address >> 2 ] = ((u32*)Data) [ 3 ];
		//_BUS->MainMemory.b32 [ ( Address >> 2 ) + 1 ] = ((u32*)Data) [ 2 ];
		//_BUS->MainMemory.b32 [ ( Address >> 2 ) + 2 ] = ((u32*)Data) [ 1 ];
		//_BUS->MainMemory.b32 [ ( Address >> 2 ) + 3 ] = ((u32*)Data) [ 0 ];
		
		//_BUS->MainMemory.b32 [ Address >> 2 ] = ((u32*)Data) [ 0 ];
		//_BUS->MainMemory.b32 [ ( Address >> 2 ) + 1 ] = ((u32*)Data) [ 1 ];
		//_BUS->MainMemory.b32 [ ( Address >> 2 ) + 2 ] = ((u32*)Data) [ 2 ];
		//_BUS->MainMemory.b32 [ ( Address >> 2 ) + 3 ] = ((u32*)Data) [ 3 ];
		
#ifdef ENABLE_INVALID_ARRAY
		_BUS->InvalidArray.b8 [ Address >> ( 2 + c_iInvalidate_Shift ) ] = 1;
#endif
		
		return;
	}
	
	u64 ShiftAmount, ShiftMask;
	//ShiftAmount = ( ( Address & 0x3 ) << 3 );
	ShiftAmount = ( ( Address & 0x7 ) << 3 );
	ShiftMask = ( Mask << ShiftAmount );
	//_BUS->MainMemory.b32 [ Address >> 2 ] = ( _BUS->MainMemory.b32 [ Address >> 2 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
	_BUS->MainMemory.b64 [ Address >> 3 ] = ( _BUS->MainMemory.b64 [ Address >> 3 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
	
#ifdef ENABLE_INVALID_ARRAY
		_BUS->InvalidArray.b8 [ Address >> ( 2 + c_iInvalidate_Shift ) ] = 1;
#endif
}

u64 DataBus::BIOS_Read ( u32 Address, u64 Mask )
{
	// this is bios
#ifdef INLINE_DEBUG_READ_BIOS
	debug << "\r\nRead; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "; BIOS; Data = " << hex << (_BUS->BIOS.b32 [ ( Address & BIOS_Mask ) >> 2 ]) << dec << ";";
#endif
	
	Latency = c_iBIOS_Read_Latency;
		
	// this area is 512 KB
	Address &= BIOS_Mask;

	if ( !Mask )
	{
		// 128-bit read from memory
		return (u64) ( & ( _BUS->BIOS.b64 [ Address >> 3 ] ) );
	}

	//return ( ( _BUS->BIOS.b32 [ Address >> 2 ] ) >> ( ( Address & 0x3 ) << 3 ) );
	return ( ( _BUS->BIOS.b64 [ Address >> 3 ] ) >> ( ( Address & 0x7 ) << 3 ) );
	
}

u64 DataBus::ScratchPad_Read ( u32 Address, u64 Mask )
{
	// this is scratch pad ram
#ifdef INLINE_DEBUG_READ_SCRATCHPAD
	debug << "\r\nRead; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "; ScratchPad; Data = " << hex << (u64) ( (_BUS->ScratchPad.b64 [ ( Address & ScratchPad_Mask ) >> 3 ]) >> ( ( Address & 0x7 ) << 3 ) ) << dec << ";";
#endif

	Latency = c_iRAM_Read_Latency;
	
	// this area is a maximum of 32 MB
	Address &= ScratchPad_Mask;
	
	if ( !Mask )
	{
		// 128-bit read from memory
		return (u64) ( & ( _BUS->ScratchPad.b64 [ Address >> 3 ] ) );
	}
	
	//return ( ( _BUS->ScratchPad.b32 [ Address >> 2 ] ) >> ( ( Address & 0x3 ) << 3 ) );
	return ( ( _BUS->ScratchPad.b64 [ Address >> 3 ] ) >> ( ( Address & 0x7 ) << 3 ) );

}

void DataBus::ScratchPad_Write ( u32 Address, u64 Data, u64 Mask )
{
	// this is scratch pad ram
#ifdef INLINE_DEBUG_WRITE_SCRATCHPAD
	debug << "\r\nWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data;
	debug << "; ScratchPad; Data = " << hex << Data << dec << ";";
#endif

	// this area is a maximum of 32 MB
	Address &= ScratchPad_Mask;
	
	if ( !Mask )
	{
		// 128-bit write to memory
		_BUS->ScratchPad.b64 [ Address >> 3 ] = ((u64*)Data) [ 0 ];
		_BUS->ScratchPad.b64 [ ( Address >> 3 ) + 1 ] = ((u64*)Data) [ 1 ];
		
		//_BUS->ScratchPad.b32 [ Address >> 2 ] = ((u32*)Data) [ 3 ];
		//_BUS->ScratchPad.b32 [ ( Address >> 2 ) + 1 ] = ((u32*)Data) [ 2 ];
		//_BUS->ScratchPad.b32 [ ( Address >> 2 ) + 2 ] = ((u32*)Data) [ 1 ];
		//_BUS->ScratchPad.b32 [ ( Address >> 2 ) + 3 ] = ((u32*)Data) [ 0 ];
		
#ifdef ENABLE_INVALID_ARRAY_SCRATCHPAD
		_BUS->InvalidArray.b8 [ Address >> ( 2 + c_iInvalidate_Shift ) ] = 1;
#endif
		return;
	}
	
	u64 ShiftAmount, ShiftMask;
	//ShiftAmount = ( ( Address & 0x3 ) << 3 );
	ShiftAmount = ( ( Address & 0x7 ) << 3 );
	ShiftMask = ( Mask << ShiftAmount );
	//_BUS->ScratchPad.b32 [ Address >> 2 ] = ( _BUS->ScratchPad.b32 [ Address >> 2 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
	_BUS->ScratchPad.b64 [ Address >> 3 ] = ( _BUS->ScratchPad.b64 [ Address >> 3 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
	
#ifdef ENABLE_INVALID_ARRAY_SCRATCHPAD
		_BUS->InvalidArray.b8 [ Address >> ( 2 + c_iInvalidate_Shift ) ] = 1;
#endif
}



u64 DataBus::VuMem_Read ( u32 Address, u64 Mask )
{
	// this is main ram
#ifdef INLINE_DEBUG_READ_VUMEM
	debug << "\r\nREAD; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address;
	debug << "; VUMEM; Data = " << hex << (_BUS->MainMemory.b32 [ ( Address & MainMemory_Mask ) >> 2 ]) << dec << ";";
#endif

	Latency = c_iRAM_Read_Latency;
	
	Address &= 0x1fffffff;
	
	if ( Address < 0x11004000 )
	{
		// micro mem 0 //
		
		Address &= MicroMem0_Mask;
		
		if ( !Mask )
		{
			// 128-bit read from memory
			return (u64) ( & ( _BUS->MicroMem0 [ Address >> 3 ] ) );
		}

		return ( ( _BUS->MicroMem0 [ Address >> 3 ] ) >> ( ( Address & 0x7 ) << 3 ) );
	}
	else if ( Address < 0x11008000 )
	{
		// vu mem 0 //
		
		Address -= 0x11004000;
		Address &= MicroMem0_Mask;
		
		if ( !Mask )
		{
			// 128-bit read from memory
			return (u64) ( & ( _BUS->VuMem0 [ Address >> 3 ] ) );
		}

		return ( ( _BUS->VuMem0 [ Address >> 3 ] ) >> ( ( Address & 0x7 ) << 3 ) );
	}
	else if ( Address < 0x1100c000 )
	{
		// micro mem 1 //
		
		Address -= 0x11008000;
		Address &= MicroMem1_Mask;
		
		if ( !Mask )
		{
			// 128-bit read from memory
			return (u64) ( & ( _BUS->MicroMem1 [ Address >> 3 ] ) );
		}

		return ( ( _BUS->MicroMem1 [ Address >> 3 ] ) >> ( ( Address & 0x7 ) << 3 ) );
	}
	else
	{
		// vu mem 1 //
		
		Address -= 0x1100c000;
		Address &= MicroMem1_Mask;
		
		if ( !Mask )
		{
			// 128-bit read from memory
			return (u64) ( & ( _BUS->VuMem1 [ Address >> 3 ] ) );
		}

		return ( ( _BUS->VuMem1 [ Address >> 3 ] ) >> ( ( Address & 0x7 ) << 3 ) );
	}

}

void DataBus::VuMem_Write ( u32 Address, u64 Data, u64 Mask )
{
	// this is main ram
#ifdef INLINE_DEBUG_WRITE_VUMEM
	debug << "\r\nWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data << "; Mask=" << Mask;
	debug << "; VUMEM";
#endif

	Address &= 0x1fffffff;
	
	if ( Address < 0x11004000 )
	{
		// micro mem 0 //
		
		Address &= MicroMem0_Mask;
		
		// a write to the instruction area will invalidate the whole thing for now for VU
		VU::_VU [ 0 ]->bCodeModified [ 0 ] = 1;
		
		if ( !Mask )
		{
#ifdef INLINE_DEBUG_WRITE_VUMEM
			debug << " Data128=" << ((u64*)Data) [ 0 ] << " " << ((u64*)Data) [ 1 ];
#endif
			// 128-bit write to memory
			_BUS->MicroMem0 [ Address >> 3 ] = ((u64*)Data) [ 0 ];
			_BUS->MicroMem0 [ ( Address >> 3 ) + 1 ] = ((u64*)Data) [ 1 ];

			return;
		}
		
		u64 ShiftAmount, ShiftMask;
		ShiftAmount = ( ( Address & 0x7 ) << 3 );
		ShiftMask = ( Mask << ShiftAmount );
		_BUS->MicroMem0 [ Address >> 3 ] = ( _BUS->MicroMem0 [ Address >> 3 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
	}
	else if ( Address < 0x11008000 )
	{
		// vu mem 0 //
		
		Address -= 0x11004000;
		Address &= MicroMem0_Mask;
		
		if ( !Mask )
		{
#ifdef INLINE_DEBUG_WRITE_VUMEM
			debug << " Data128=" << ((u64*)Data) [ 0 ] << " " << ((u64*)Data) [ 1 ];
#endif
			// 128-bit write to memory
			_BUS->VuMem0 [ Address >> 3 ] = ((u64*)Data) [ 0 ];
			_BUS->VuMem0 [ ( Address >> 3 ) + 1 ] = ((u64*)Data) [ 1 ];
			
			return;
		}
		
		u64 ShiftAmount, ShiftMask;
		ShiftAmount = ( ( Address & 0x7 ) << 3 );
		ShiftMask = ( Mask << ShiftAmount );
		_BUS->VuMem0 [ Address >> 3 ] = ( _BUS->VuMem0 [ Address >> 3 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
	}
	else if ( Address < 0x1100c000 )
	{
		// micro mem 1 //
		
		// micromem must be the instructions
		
		Address -= 0x10008000;
		Address &= MicroMem1_Mask;

		// a write to the instruction area will invalidate the whole thing for now for VU
		VU::_VU [ 1 ]->bCodeModified [ 1 ] = 1;
		
		if ( !Mask )
		{
#ifdef INLINE_DEBUG_WRITE_VUMEM
			debug << " Data128=" << ((u64*)Data) [ 0 ] << " " << ((u64*)Data) [ 1 ];
#endif
			// 128-bit write to memory
			_BUS->MicroMem1 [ Address >> 3 ] = ((u64*)Data) [ 0 ];
			_BUS->MicroMem1 [ ( Address >> 3 ) + 1 ] = ((u64*)Data) [ 1 ];
			
			return;
		}
		
		u64 ShiftAmount, ShiftMask;
		ShiftAmount = ( ( Address & 0x7 ) << 3 );
		ShiftMask = ( Mask << ShiftAmount );
		_BUS->MicroMem1 [ Address >> 3 ] = ( _BUS->MicroMem1 [ Address >> 3 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
	}
	else
	{
		// vu mem 1 //
		
		// vu mem must be the data
		
		Address -= 0x1000c000;
		Address &= MicroMem1_Mask;
		
		if ( !Mask )
		{
#ifdef INLINE_DEBUG_WRITE_VUMEM
			debug << " Data128=" << ((u64*)Data) [ 0 ] << " " << ((u64*)Data) [ 1 ];
#endif
			// 128-bit write to memory
			_BUS->VuMem1 [ Address >> 3 ] = ((u64*)Data) [ 0 ];
			_BUS->VuMem1 [ ( Address >> 3 ) + 1 ] = ((u64*)Data) [ 1 ];
			
			return;
		}
		
		u64 ShiftAmount, ShiftMask;
		ShiftAmount = ( ( Address & 0x7 ) << 3 );
		ShiftMask = ( Mask << ShiftAmount );
		_BUS->VuMem1 [ Address >> 3 ] = ( _BUS->VuMem1 [ Address >> 3 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
	}
}





u64 DataBus::DirectCacheMem_Read ( u32 Address, u64 Mask )
{
	// this is main ram
#ifdef INLINE_DEBUG_READ_DIRECTCACHE
	debug << "\r\nREAD; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Mask=" << Mask;
	debug << "; DirectCache; Data = " << hex << (_BUS->DirectCacheMem.b32 [ ( Address & DirectCacheMem_Mask ) >> 2 ]) << dec << ";";
#endif

	u64 Output;

	Latency = c_iRAM_Read_Latency;
	
	// this area is a maximum of 32 KB ??
	Address &= DirectCacheMem_Mask;
	
	if ( !Mask )
	{
		// 128-bit read from memory
		Output = (u64) ( & ( _BUS->DirectCacheMem.b64 [ Address >> 3 ] ) );
		
		
#ifdef INLINE_DEBUG_READ_DIRECTCACHE
	debug << " Output=" << hex << ((u32*)Output) [ 0 ] << " " << ((u32*)Output) [ 1 ] << " " << ((u32*)Output) [ 2 ] << " " << ((u32*)Output) [ 3 ];
#endif

		return Output;
	}

	
	Output = ( ( _BUS->DirectCacheMem.b64 [ Address >> 3 ] ) >> ( ( Address & 0x7 ) << 3 ) );
	//Output = _BUS->DirectCacheMem.b64 [ Address >> 3 ] & Mask;
	
	//return ( ( _BUS->MainMemory.b32 [ Address >> 2 ] ) >> ( ( Address & 0x3 ) << 3 ) );

#ifdef INLINE_DEBUG_READ_DIRECTCACHE
	debug << " Output=" << hex << Output;
#endif

	return Output;
}

void DataBus::DirectCacheMem_Write ( u32 Address, u64 Data, u64 Mask )
{
	// this is main ram
#ifdef INLINE_DEBUG_WRITE_DIRECTCACHE
	debug << "\r\nWrite; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data << "; Mask=" << Mask;
	debug << "; DirectCache";
#endif

	// this area is a maximum of 32 KB ??
	Address &= DirectCacheMem_Mask;
	
	if ( !Mask )
	{
#ifdef INLINE_DEBUG_WRITE_DIRECTCACHE
	debug << "; Writing: " << hex << ((u32*)Data) [ 0 ] << " " << ((u32*)Data) [ 1 ] << " " << ((u32*)Data) [ 2 ] << " " << ((u32*)Data) [ 3 ];
#endif

		// 128-bit write to memory
		_BUS->DirectCacheMem.b64 [ Address >> 3 ] = ((u64*)Data) [ 0 ];
		_BUS->DirectCacheMem.b64 [ ( Address >> 3 ) + 1 ] = ((u64*)Data) [ 1 ];
		
		return;
	}

	
	u64 ShiftAmount, ShiftMask;
	//ShiftAmount = ( ( Address & 0x3 ) << 3 );
	ShiftAmount = ( ( Address & 0x7 ) << 3 );
	ShiftMask = ( Mask << ShiftAmount );
	//_BUS->DirectCacheMem.b32 [ Address >> 2 ] = ( _BUS->DirectCacheMem.b32 [ Address >> 2 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
	_BUS->DirectCacheMem.b64 [ Address >> 3 ] = ( _BUS->DirectCacheMem.b64 [ Address >> 3 ] & ~ShiftMask ) | ( ( Data << ShiftAmount ) & ShiftMask );
}









u64 DataBus::Device_Read ( u32 Address, u64 Mask )
{
	//u32 PhysicalAddress_Low;

	u64 Output = 0;
	
			// this is a hardware mapped register
			// *** todo *** read from hardware mapped register
			
//#ifdef INLINE_DEBUG_READ_REG
//	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
//#endif

	//Latency = c_iReg_Read_Latency;
	//return LUT_RegRead [ ( Address >> 4 ) & 0x3ff ] ( Address );
	
	
	// check if GPU or other hardware register
	
	// clear off top three bits, because on PS2 they could be set
	Address &= 0x1fffffff;
	
	switch ( Address >> 16 )
	{
		case 0x1000:
		
			// non-GPU hardware register
			
			switch ( Address & 0xff00 )
			{
				case 0x0000:
					
#ifdef INLINE_DEBUG_READ_REG_TIMER
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " Timer0";
#endif

					Output = Timers::_TIMERS->Read ( Address, Mask );
					
#ifdef INLINE_DEBUG_READ_REG_TIMER
				debug << " Output=" << hex << Output;
#endif

					return Output;
					break;
					
				case 0x0800:
				
#ifdef INLINE_DEBUG_READ_REG_TIMER
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " Timer1";
#endif

					Output = Timers::_TIMERS->Read ( Address, Mask );
					
#ifdef INLINE_DEBUG_READ_REG_TIMER
				debug << " Output=" << hex << Output;
#endif

					return Output;
					break;
				
				case 0x1000:
				
#ifdef INLINE_DEBUG_READ_REG_TIMER
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " Timer2";
#endif

					Output = Timers::_TIMERS->Read ( Address, Mask );
					
#ifdef INLINE_DEBUG_READ_REG_TIMER
				debug << " Output=" << hex << Output;
#endif

					return Output;
					break;

				case 0x1800:
				
#ifdef INLINE_DEBUG_READ_REG_TIMER
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " Timer3";
#endif

					Output = Timers::_TIMERS->Read ( Address, Mask );
					
#ifdef INLINE_DEBUG_READ_REG_TIMER
				debug << " Output=" << hex << Output;
#endif

					return Output;
					break;
					
				case 0x2000:
				
#if defined INLINE_DEBUG_READ_REG || defined INLINE_DEBUG_READ_MDEC
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " IPU";
#endif

//#ifdef DEBUG_COUT
//					cout << "\nIPU Read: Address=" << hex << Address << " Mask=" << Mask << "\n";
//#endif

					Output = IPU::_IPU->Read ( Address, Mask );

					break;
				
				case 0x3000:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " GIF";
#endif

					Output = GPU::_GPU->Read ( Address, Mask );
					break;
				
				case 0x3800:
				case 0x3900:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " VIF0";
#endif

					Output = VU0::Read ( Address, Mask );
					break;
				
				case 0x3c00:
				case 0x3d00:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " VIF1";
#endif

					Output = VU1::Read ( Address, Mask );
					
#ifdef INLINE_DEBUG_READ_REG
	debug << " Output(testVIF1)=" << hex << Output;
#endif

					break;
				
				case 0x4000:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " VIF0FIFO";
#endif

					Output = VU0::Read ( Address, Mask );
					break;

				case 0x5000:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " VIF1FIFO";
#endif

					Output = VU1::Read ( Address, Mask );
					break;
					
				case 0x6000:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " GIFFIFO";
#endif

					Output = GPU::_GPU->Read ( Address, Mask );
					break;
				
				case 0x7000:
				
#if defined INLINE_DEBUG_READ_REG || defined INLINE_DEBUG_READ_MDEC
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " IPUFIFO";
#endif

//#ifdef DEBUG_COUT
//					cout << "\nIPU FIFO Read: Address=" << hex << Address << " Mask=" << Mask << "\n";
//#endif

					Output = IPU::_IPU->Read ( Address, Mask );
					
					break;
				
				
				case 0x8000:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " DMA0 (VIF0)";
#endif

					Output = Dma::_DMA->Read ( Address, Mask );
					break;
				
				case 0x9000:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " DMA1 (VIF1)";
#endif

					Output = Dma::_DMA->Read ( Address, Mask );
					break;
				
				case 0xa000:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " DMA2 (GIF)";
#endif

					Output = Dma::_DMA->Read ( Address, Mask );
					break;
				
				case 0xb000:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " DMA3 (fromIPU)";
#endif

					Output = Dma::_DMA->Read ( Address, Mask );
					break;
				
				case 0xb400:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " DMA4 (toIPU)";
#endif

					Output = Dma::_DMA->Read ( Address, Mask );
					break;
				
				case 0xc000:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " DMA5 (SIF0)";
#endif

					Output = Dma::_DMA->Read ( Address, Mask );
					break;
				
				case 0xc400:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " DMA6 (SIF1)";
#endif

					Output = Dma::_DMA->Read ( Address, Mask );
					break;
				
				case 0xc800:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " DMA7 (SIF2)";
#endif

					Output = Dma::_DMA->Read ( Address, Mask );
					break;
				
				case 0xd000:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " DMA8 (fromSPR)";
#endif

					Output = Dma::_DMA->Read ( Address, Mask );
					break;
				
				case 0xd400:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " DMA9 (toSPR)";
#endif

					Output = Dma::_DMA->Read ( Address, Mask );
					break;
				
				case 0xe000:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " DMAC";
#endif

					Output = Dma::_DMA->Read ( Address, Mask );
					break;
				
				case 0xf000:
				
#ifdef INLINE_DEBUG_READ_REG_INTC
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " INTC";
#endif

					Output = Intc::_INTC->Read ( Address, Mask );
					
					//if ( ( Address & 0x1fffffff ) == 0x1000f000 )
					//{
					//	return _BUS->lINTC;
					//}

#ifdef INLINE_DEBUG_READ_REG_INTC
					debug << " Output=" << hex << Output;
#endif

					return Output;
					break;
				
				case 0xf100:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " SIO";
#endif

					Output = SIO::_SIO->Read ( Address, Mask );
					break;
				
				case 0xf200:
				
#ifdef INLINE_DEBUG_READ_REG_SBUS
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " SBUS";
#endif

					// *** SBUS and MCH are currently adapted from pcsx2 *** //
					Output = SIF::EE_Read ( Address, Mask );

					/*
					switch ( Address )
					{
						case 0x1000f200:
							Output = _BUS->lSBUS_F200;
							break;
					
						case 0x1000f210:
							Output = _BUS->lSBUS_F210;
							break;
							
						case 0x1000f220:
							Output = _BUS->lSBUS_F220;
							break;

						case 0x1000f230:
							Output = _BUS->lSBUS_F230;
							break;
							
						case 0x1000f240:
							Output = _BUS->lSBUS_F240 | 0xf0000102;
							break;
					
						case 0x1000f260:
							Output = _BUS->lSBUS_F260;
							break;
					}
					*/
					
#ifdef INLINE_DEBUG_READ_REG_SBUS
				debug << "; Output=" << Output;
#endif

					return Output;
					
					break;
				
				case 0xf400:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " MCH";
#endif

					// *** SBUS and MCH are currently adapted from pcsx2 *** //
					
					if ( Address == 0x1000f410 )
					{
						Output = 0;
						break;
					}
					
					// MCH_RICM
					if ( Address == 0x1000f430 )
					{
						//Output = 0;
						Output = _BUS->lMCH_RICM;
						break;
					}
					
					// MCH_DRD
					if ( Address == 0x1000f440 )
					{
						if( !((_BUS->lMCH_RICM >> 6) & 0xF) )
						{
							switch ((_BUS->lMCH_RICM>>16) & 0xFFF)
							{
								//MCH_RICM: x:4|SA:12|x:5|SDEV:1|SOP:4|SBC:1|SDEV:5

								case 0x21://INIT
									// rdram_devices = 2 or 8
									if( _BUS->RDRAM_SDEVID < 2 /*rdram_devices*/ )
									{
										_BUS->RDRAM_SDEVID++;
										Output = 0x1F;
										break;
									}
								Output = 0;
								break;

								case 0x23://CNFGA
									Output = 0x0D0D;	//PVER=3 | MVER=16 | DBL=1 | REFBIT=5
									break;

								case 0x24://CNFGB
									//0x0110 for PSX  SVER=0 | CORG=8(5x9x7) | SPT=1 | DEVTYP=0 | BYTE=0
									Output = 0x0090;	//SVER=0 | CORG=4(5x9x6) | SPT=1 | DEVTYP=0 | BYTE=0
									break;

								case 0x40://DEVID
									Output = _BUS->lMCH_RICM & 0x1F;	// =SDEV
									break;
							}
						}
						else
						{
							Output=0;
						}
						
						break;
					}
					
					if ( Address == 0x1000f480 )
					{
						Output = _BUS->lMCH_F480;
						break;
					}
					
					if ( Address == 0x1000f490 )
					{
						Output = _BUS->lMCH_F490;
						break;
					}

					break;
				
				case 0xf500:
				
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " DMAC_ENABLER/DMAC_ENABLEW";
#endif

					Output = Dma::_DMA->Read ( Address, Mask );
					
					/*
					if ( ( Address & 0xffff ) == 0xf520 )
					{
#ifdef INLINE_DEBUG_READ_REG
				debug << "; DMAC_ENABLER";
#endif

						// dmac enable r
						Output = _BUS->lDMAC_ENABLE;
						break;
					}
					*/

					break;
				
				default:
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " UNKNOWN REG";
#endif

					break;
				
			}
			
			break;
			
		case 0x1200:
		
			// GPU
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " GPU";
#endif

			Output = GPU::_GPU->Read ( Address, Mask );
			break;
			
		default:
#ifdef INLINE_DEBUG_READ_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DRead: BUS. Address = " << hex << Address;
				debug << " UNKNOWN REG";
#endif

			if ( !Mask )
			{
				Output = (u64) (& ( _BUS->Dummy128 [ 0 ] ));
				break;
			}
			
			
			Output = 0;
			
			break;
	}
	
	
	// keep things from crashing for now
	if ( !Mask )
	{
#ifdef DEBUG_COUT
		cout << "\nhps2x64: ALERT: 128-bit device read needs fixing (set to zero) Address=" << hex << Address << "\n";
#endif

		Output = (u64) (& ( _BUS->Dummy128 [ 0 ] ));
	}
	
#ifdef INLINE_DEBUG_READ_REG
	debug << " Output=" << hex << Output;
#endif
	
	return Output;
}

void DataBus::Device_Write ( u32 Address, u64 Data, u64 Mask )
{
			// this is a hardware mapped register
			// *** todo *** read from hardware mapped register
			
//#ifdef INLINE_DEBUG_WRITE_REG
//	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
//#endif
			
	// clear off top three bits, because on PS2 they could be set
	Address &= 0x1fffffff;

	// check if GPU or other hardware register
	
	switch ( Address >> 16 )
	{
		case 0x1000:
		
			// non-GPU hardware register
			
			switch ( Address & 0xff00 )
			{
				case 0x0000:
					
#ifdef INLINE_DEBUG_WRITE_REG_TIMER
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " Timer0";
#endif

					Timers::_TIMERS->Write ( Address, Data, Mask );
					break;
					
				case 0x0800:
				
#ifdef INLINE_DEBUG_WRITE_REG_TIMER
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " Timer1";
#endif

					Timers::_TIMERS->Write ( Address, Data, Mask );
					break;
				
				case 0x1000:
				
#ifdef INLINE_DEBUG_WRITE_REG_TIMER
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " Timer2";
#endif

					Timers::_TIMERS->Write ( Address, Data, Mask );
					break;

				case 0x1800:
				
#ifdef INLINE_DEBUG_WRITE_REG_TIMER
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " Timer3";
#endif

					Timers::_TIMERS->Write ( Address, Data, Mask );
					break;
					
				case 0x2000:
				
#if defined INLINE_DEBUG_WRITE_REG || defined INLINE_DEBUG_WRITE_MDEC
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " IPU";
#endif

//#ifdef DEBUG_COUT
//					cout << "\nIPU Write: Value=" << hex << Data << " Address=" << Address << "\n";
//#endif

					IPU::_IPU->Write ( Address, Data, Mask );
					break;
				
				case 0x3000:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " GIF";
#endif

					GPU::_GPU->Write ( Address, Data, Mask );
					break;
				
				case 0x3800:
				case 0x3900:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " VIF0";
#endif

					VU0::Write ( Address, Data, Mask );

					break;
				
				case 0x3c00:
				case 0x3d00:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " VIF1";
#endif

					VU1::Write ( Address, Data, Mask );
					break;
				
				case 0x4000:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " VIF0FIFO";
#endif

					VU0::Write ( Address, Data, Mask );
					break;

				case 0x5000:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " VIF1FIFO";
#endif

					VU1::Write ( Address, Data, Mask );
					break;
					
				case 0x6000:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " GIFFIFO";
#endif

					GPU::_GPU->Write ( Address, Data, Mask );
					break;
				
				case 0x7000:
				
#if defined INLINE_DEBUG_WRITE_REG || defined INLINE_DEBUG_WRITE_MDEC
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " IPUFIFO";
#endif

//#ifdef DEBUG_COUT
//					cout << "\nIPU FIFO Write: Value=" << hex << Data << " Address=" << Address << "\n";
//#endif

					IPU::_IPU->Write ( Address, Data, Mask );
					break;
				
				
				case 0x8000:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " DMA0 (VIF0)";
#endif

					Dma::_DMA->Write ( Address, Data, Mask );
					break;
				
				case 0x9000:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " DMA1 (VIF1)";
#endif

					Dma::_DMA->Write ( Address, Data, Mask );
					break;
				
				case 0xa000:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " DMA2 (GIF)";
#endif

					Dma::_DMA->Write ( Address, Data, Mask );
					break;
				
				case 0xb000:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " DMA3 (fromIPU)";
#endif

					Dma::_DMA->Write ( Address, Data, Mask );
					break;
				
				case 0xb400:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " DMA4 (toIPU)";
#endif

					Dma::_DMA->Write ( Address, Data, Mask );
					break;
				
				case 0xc000:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " DMA5 (SIF0)";
#endif

					Dma::_DMA->Write ( Address, Data, Mask );
					break;
				
				case 0xc400:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " DMA6 (SIF1)";
#endif

					Dma::_DMA->Write ( Address, Data, Mask );
					break;
				
				case 0xc800:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " DMA7 (SIF2)";
#endif

					Dma::_DMA->Write ( Address, Data, Mask );
					break;
				
				case 0xd000:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " DMA8 (fromSPR)";
#endif

					Dma::_DMA->Write ( Address, Data, Mask );
					break;
				
				case 0xd400:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " DMA9 (toSPR)";
#endif

					Dma::_DMA->Write ( Address, Data, Mask );
					break;
				
				case 0xe000:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " DMAC";
#endif

					Dma::_DMA->Write ( Address, Data, Mask );
					break;
				
				case 0xf000:
				
#ifdef INLINE_DEBUG_WRITE_REG_INTC
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " INTC";
#endif

					Intc::_INTC->Write ( Address, Data, Mask );
					
					//if ( ( Address & 0x1fffffff ) == 0x1000f000 )
					//{
					//	_BUS->lINTC &= ~Data;
					//}
					
					break;
				
				case 0xf100:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " SIO";
#endif

					//if ( Address == 0x1000f180 )
					//{
					//	cout << (char) ( Data & 0xff );
					//}

					SIO::_SIO->Write ( Address, Data, Mask );
					break;
				
				case 0xf200:
				
#ifdef INLINE_DEBUG_WRITE_REG_SBUS
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " SBUS";
#endif

					SIF::EE_Write ( Address, Data, Mask );

					/*
					switch ( Address )
					{
						case 0x1000f200:
							// EE write path, so store data here
							// 0x1000f210 is the IOP write path
							_BUS->lSBUS_F200 = Data;
							
							// maybe trigger IOP interrupt
							break;
							
						case 0x1000f210:
							// this is from IOP to EE
							break;

						case 0x1000f220:
							// bits get SET from EE side and CLEARED when written from IOP side
							// ??interrupt trigger?? //
							_BUS->lSBUS_F220 |= Data;
							
							// treat bit 16 as an interrupt trigger ??
							if ( _BUS->lSBUS_F220 & 0x10000 )
							{
								//Playstation1::Intc::_INTC->I_STAT_Reg.Unknown0 = 1;
								//Playstation1::Intc::_INTC->UpdateInts ();
							}
							
							break;
					
						case 0x1000f230:
							// bits get cleared when written from EE and set when written from IOP
							_BUS->lSBUS_F230 &= ~Data;
							break;
					
						case 0x1000f240:
							// control register //
							
							if(!(Data & 0x100))
							{
								_BUS->lSBUS_F240 &= ~0x100;
							}
							else
							{
								_BUS->lSBUS_F240 |= 0x100;
							}
							
							break;

						case 0x1000f260:
							// ??? //
							_BUS->lSBUS_F260 = 0;
							break;
					}
					*/
					
					break;
					
				
				case 0xf400:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " MCH";
#endif

					if ( Address == 0x1000f430 )
					{
						if ((((Data >> 16) & 0xFFF) == 0x21) && (((Data >> 6) & 0xF) == 1) && (((_BUS->lMCH_DRD >> 7) & 1) == 0))//INIT & SRP=0
							_BUS->RDRAM_SDEVID = 0;	// if SIO repeater is cleared, reset sdevid
							
						_BUS->lMCH_RICM = Data & ~0x80000000;	//kill the busy bit
					}
					
					if ( Address == 0x1000f440 )
					{
						_BUS->lMCH_DRD = Data;
					}
					
					if ( Address == 0x1000f480 )
					{
						_BUS->lMCH_F480 = Data;
					}
					
					if ( Address == 0x1000f490 )
					{
						_BUS->lMCH_F490 = Data;
					}
					
					break;
				
				case 0xf500:
				
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " DMAC_ENABLER/DMAC_ENABLEW";
#endif

					Dma::_DMA->Write ( Address, Data, Mask );
					
					/*
					if ( ( Address & 0xffff ) == 0xf590 )
					{
#ifdef INLINE_DEBUG_WRITE_REG
				debug << "; DMAC_ENABLEW";
#endif

						// dmac enable w
						//_BUS->lDMAC_ENABLE ^= ( Data & 0x10000 );
						//_BUS->lDMAC_ENABLE = ( Data & 0x10000 ) | ( _BUS->lDMAC_ENABLE & 0xffff );
						_BUS->lDMAC_ENABLE = ( Data & 0x10000 ) | ( _BUS->lDMAC_ENABLE & ~0x10000 );
					}
					*/

					break;
				
				default:
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " UNKNOWN REG";
#endif

					break;
				
			}
			
			break;
			
		case 0x1200:
		
			// GPU
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " GPU";
#endif

			GPU::_GPU->Write ( Address, Data, Mask );
			break;
			
		default:
#ifdef INLINE_DEBUG_WRITE_REG
	debug << "\r\n" << dec << *_DebugCycleCount << " " << hex << setw ( 8 ) << *_DebugPC << " DWrite: BUS. Address = " << hex << Address << "; DataIn = " << Data;
				debug << " UNKNOWN REG";
#endif

			break;
	}

}


u64 DataBus::Read ( u32 Address, u64 Mask )
{
#ifdef INLINE_DEBUG_DATABUS_READ
	debug << "\r\nDataBus::Read; Address=" << hex << Address << " Mask=" << Mask;
#endif

	// clear top 3 bits to get physical address
	// don't clear top 3 bits since scratchpad is at 0x70000000
	//Address &= 0x1fffffff;
	
	// multiplexer
	return LUT_BusRead [ ( Address >> 22 ) & 0x3ff ] ( Address, Mask );
}

// *note* currently for PS1, some of these arguments are reversed
void DataBus::Write ( u32 Address, u64 Data, u64 Mask )
{
#ifdef INLINE_DEBUG_DATABUS_WRITE
	debug << "\r\nDataBus::Write; Address=" << hex << Address << " Data=" << Data << " Mask=" << Mask;
#endif

	// clear top 3 bits to get physical address
	// don't clear top 3 bits since scratchpad is at 0x70000000
	//Address &= 0x1fffffff;
	
	// multiplexer
	LUT_BusWrite [ ( Address >> 22 ) & 0x3ff ] ( Address, Data, Mask );
	
	return;
}


/*
u64* DataBus::Read128 ( u32 Address, u64 Mask )
{
}

void DataBus::Write128 ( u32 Address, u64* Data, u64 Mask )
{
}
*/


#ifdef ENABLE_GUI_DEBUGGER
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
		FileOut << "\r\n" << hex << setw( 8 ) << CurrentPC << " " << (CurrentPC & BIOS_Mask) << " " << R5900::Instruction::Print::PrintInstruction ( i ).c_str () << "; " << i;
		
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
		FileOut << "\r\n" << hex << setw( 8 ) << CurrentPC << " " << (CurrentPC & MainMemory_Mask) << " " << R5900::Instruction::Print::PrintInstruction ( i ).c_str () << "; " << i;
		
		// show status
		if ( ( j & 0xff ) == 0xff ) cout << "\rProgress: " << j << " out of " << NumberOfInstructions;
	}

	cout << "\nDone.\n";
}
#endif


void* DataBus::GetPointer ( u32 Address )
{
	if ( ( ( Address >> 20 ) & 0x1fc ) == 0x1f )
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
		
		// for templates
		LUT_BusRead8 [ i ] = InvalidAddress_Read;
		LUT_BusRead16 [ i ] = InvalidAddress_Read;
		LUT_BusRead32 [ i ] = InvalidAddress_Read;
		LUT_BusRead64 [ i ] = InvalidAddress_Read;
		LUT_BusRead128 [ i ] = InvalidAddress_Read;
		
		// for recompiler
		LUT_DataBus_Read [ i ].pMemoryDevice = NULL;
		LUT_DataBus_Read [ i ].ulMask = 0;
		LUT_DataBus_Read [ i ].ulLatency = c_iReg_Read_Latency;
		LUT_DataBus_Write [ i ].pMemoryDevice = NULL;
		LUT_DataBus_Write [ i ].ulMask = 0;
		LUT_DataBus_Write [ i ].ulLatency = c_iReg_Read_Latency;
	}
	
	for ( i = 0; i < c_LUT_Bus_Size; i++ )
	{
		LUT_BusWrite [ i ] = InvalidAddress_Write;
		
		// for templates
		LUT_BusWrite8 [ i ] = InvalidAddress_Write;
		LUT_BusWrite16 [ i ] = InvalidAddress_Write;
		LUT_BusWrite32 [ i ] = InvalidAddress_Write;
		LUT_BusWrite64 [ i ] = InvalidAddress_Write;
		LUT_BusWrite128 [ i ] = InvalidAddress_Write;
	}
}



void DataBus::ConnectDevice_Read ( u32 AddressStart, u32 AddressEnd, PS2_BusInterface_Read CallbackFunction )
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
		LUT_BusRead64 [ Current ] = CallbackFunction;
		LUT_BusRead128 [ Current ] = CallbackFunction;
	}
}




void DataBus::ConnectDevice_Write ( u32 AddressStart, u32 AddressEnd, PS2_BusInterface_Write CallbackFunction )
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
		LUT_BusWrite64 [ Current ] = CallbackFunction;
		LUT_BusWrite128 [ Current ] = CallbackFunction;
	}
}





void DataBus::Init_ConnectRegs ( void )
{
	int i;
	
	for ( i = 0; i < c_LUT_Reg_Size; i++ )
	{
		LUT_RegRead [ i ] = InvalidAddress_Read;
	}
	
	for ( i = 0; i < c_LUT_Reg_Size; i++ )
	{
		LUT_RegWrite [ i ] = InvalidAddress_Write;
	}
}



void DataBus::ConnectRegs_Read ( u32 AddressStart, PS2_BusInterface_Read CallbackFunction )
{
	// add callback
	LUT_RegRead [ ( AddressStart & 0x00003ff0 ) >> 4 ] = CallbackFunction;
}

void DataBus::ConnectRegs_Write ( u32 AddressStart, PS2_BusInterface_Write CallbackFunction )
{
	// add callback
	LUT_RegWrite [ ( AddressStart & 0x00003ff0 ) >> 4 ] = CallbackFunction;
}



u64 DataBus::InvalidAddress_Read ( u32 Address, u64 Mask )
{
	/*
	if ( ( Address & 0x1fff8000 ) == 0x1fff8000 )
	{
#ifdef USE_SCRATCHPAD
		return ScratchPad_Read ( Address, Mask );
#else
		return DirectCacheMem_Read ( Address, Mask );
#endif
	}
	*/

#ifndef EE_ONLY_COMPILE
	// treat 0x1c000000 as a PS1 ram address
	if ( ( Address & 0x1fc00000 ) == 0x1c000000 )
	{
		return Playstation1::DataBus::_BUS->Memory_Read ( Address & Playstation1::DataBus::MainMemory_Mask );
	}
#endif

#ifdef INLINE_DEBUG_READ_INVALID
	debug << "\r\nBus::Read; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Mask=" << Mask;
	debug << ";INVALID; READ; ADDRESS = " << hex << setw ( 8 ) << Address;
#endif

		
			// *** testing ***
//#ifdef INLINE_DEBUG_READ_INVALID
//	debug << "\r\nREAD from invalid address. PC=" << hex << *_DebugPC << " Address=" << Address << " Cycle=" << dec << *_DebugCycleCount;
//#endif

	if ( !Mask )
	{
#ifdef INLINE_DEBUG_READ_INVALID_128
	debug << "\r\nBus::Read; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Mask=" << Mask;
	debug << ";INVALID; READ; ADDRESS = " << hex << setw ( 8 ) << Address;
#endif

		cout << "\nhps2x64: 128-bit READ from invalid address. PC=" << hex << *_DebugPC << " Address=" << Address << " Cycle=" << dec << *_DebugCycleCount << "\n";
		return (u64) (& ( _BUS->Dummy128 [ 0 ] ));
	}
	
	
	
	
	switch ( Address & 0x1fffffff )
	{
		case 0x1a000000:
		case 0x1a000002:
		case 0x1a000004:
			return 0;
			break;
			
		case 0x1a000006:
			// this is stripped/adapted from pcsx2 //
			_BUS->lREG_1a6++;
			if ( _BUS->lREG_1a6 == 3 ) _BUS->lREG_1a6 = 0;
			return _BUS->lREG_1a6;
			break;
			
			
		// means it is trying to access the disk type that is currently inserted, or if there is one
		case 0x1f402005:
		case 0x1f40200f:
			//_SYSTEM._PS1SYSTEM._CD.CurrentDiskType
			return Playstation1::DataBus::_BUS->DEV5_Read ( Address & 0x1fffffff );
			break;
		
		
		
		case 0x1f803204:
			// some kind of RAM clock divider value or something
			return 0;
			break;
			
		case 0x1f803210:
			break;
			
		// ps1 ram addresses are handled now
		/*
		case 0x1c000000:
			return 0;
			break;
		
		case 0x1c0003f0:
			// once it gets this value, it then loads from it as a PS1 RAM address
			return 0x000032c0;
			break;
			
			
		case 0x1c0032c0:
			return 0x00040000;
			break;

		case 0x1c0032c2:
			return 0x04;
			break;
			
		case 0x1c0032c3:
			return 0x00;
			break;
			
		case 0x1c0032c4:
			return 0x00;
			break;
		*/
			
		default:
			//cout << "\nhps2x64: READ from invalid address. PC=" << hex << *_DebugPC << " Address=" << Address << " Cycle=" << dec << *_DebugCycleCount << "\n";
			break;
			
	}
	
	

	return 0;
}

void DataBus::InvalidAddress_Write ( u32 Address, u64 Data, u64 Mask )
{
	/*
	if ( ( Address & 0x1fff0000 ) == 0x1fff0000 )
	{
#ifdef USE_SCRATCHPAD
		ScratchPad_Write ( Address, Data, Mask );
#else
		DirectCacheMem_Write ( Address, Data, Mask );
#endif
		return;
	}
	*/

#ifndef EE_ONLY_COMPILE
	// treat 0x1c000000 as a PS1 ram address
	if ( ( Address & 0x1fc00000 ) == 0x1c000000 )
	{
		Playstation1::DataBus::_BUS->Memory_Write ( Address & Playstation1::DataBus::MainMemory_Mask, Data, Mask );
		return;
	}
#endif


#ifdef INLINE_DEBUG_WRITE_INVALID
	debug << "\r\nBus::Write; PC=" << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Address=" << hex << Address << "; Data=" << Data << "; Mask=" << Mask;
	debug << ";INVALID; WRITE; ADDRESS = " << hex << setw ( 8 ) << Address;
#endif


		
			// *** testing ***
//#ifdef INLINE_DEBUG_WRITE_INVALID
//	debug << "\r\nWRITE to invalid address. PC=" << hex << *_DebugPC << " Address=" << Address << " Cycle=" << dec << *_DebugCycleCount;
//#endif

	
	switch ( Address & 0x1fffffff )
	{
		case 0x1a000006:
			_BUS->lREG_1a6 = Data & 0xff;
			break;
		
		case 0x1f803210:
			//_BUS->lPS1_CTRL_3210 = Data & 0xff;
			break;
			
		default:
			//cout << "\nhps2x64: WRITE to invalid address. PC=" << hex << *_DebugPC << " Address=" << Address << " Cycle=" << dec << *_DebugCycleCount << "\n";
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

void DataBus::ReserveBus_CPU ( u64 CpuCycleCount, u64 Cycles )
{
	if ( CpuCycleCount >= BusyUntil_Cycle )
	{
		BusyUntil_Cycle = Cycles + CpuCycleCount;
	}
	else
	{
		BusyUntil_Cycle += Cycles;
	}
}

// reserve the cycles on the bus that it will be occupied
void DataBus::ReserveBus_DMA ( u64 DmaCycleCount, u64 Cycles )
{
	//static const u64 c_ullLatentCycle = 0;	//c_iRAM_Read_Latency;
	u64 DMALatentCycle;
	
	DMALatentCycle = DmaCycleCount + c_iRAM_Read_Latency;
	
	if ( DMALatentCycle >= BusyUntil_Cycle )
	{
		BusyUntil_Cycle = Cycles + DMALatentCycle;
	}
	else
	{
		BusyUntil_Cycle += Cycles;
	}
}

// get the next cycle# the bus will be free at for dma device
u64 DataBus::GetNextFreeCycle_CPU ()
{
	return BusyUntil_Cycle;
}

// get the next cycle# the bus will be free at for dma device
u64 DataBus::GetNextFreeCycle_DMA ()
{
	u64 DMALatentCycle;
	
	return BusyUntil_Cycle - c_iRAM_Read_Latency;
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



		
void DataBus::Reset_Invalidate ()
{
	for ( int i = 0; i < c_iInvalidate_Size / sizeof ( u64 ); i++ )
	{
		InvalidArray.b64 [ i ] = 0;
	}
}

void DataBus::InvalidateRange ( u32 StartAddr, u32 WordCount32 )
{
#ifdef INLINE_DEBUG_INVALIDATE
	debug << "\r\nDataBus::InvalidateRange";
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



/////////// Debugging //////////


#ifdef ENABLE_GUI_DEBUGGER
void DataBus::DebugWindow_Enable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	static constexpr char* DebugWindow_Caption = "PS2 Memory Bus Debug Window";
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
#endif


}


