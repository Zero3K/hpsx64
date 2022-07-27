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


#include "R3000A_COP2.h"
#include "R3000A.h"

//#include "gte_divide.h"
#include "GeneralUtilities.h"

using namespace R3000A;
using namespace GeneralUtilities;


#ifdef _DEBUG_VERSION_

// enable debugging
#define INLINE_DEBUG_ENABLE

//#define INLINE_DEBUG
//#define INLINE_DEBUG_COP2_ALL
//#define INLINE_DEBUG_COP2_RTPT
//#define INLINE_DEBUG_COP2_NCLIP
//#define INLINE_DEBUG_COP2_AVSZ3
//#define INLINE_DEBUG_COP2_AVSZ4
//#define INLINE_DEBUG_COP2_NCDS
//#define INLINE_DEBUG_COP2_GPF

//#define INLINE_DEBUG
//#define INLINE_DEBUG_NAME
//#define INLINE_DEBUG_COP2_RTPS
//#define INLINE_DEBUG_COP2_SQR
//#define INLINE_DEBUG_COP2_OP
//#define INLINE_DEBUG_COP2_MVMVA

//#define INLINE_DEBUG_COP2_WRITE_MTC
//#define INLINE_DEBUG_COP2_WRITE_CTC
//#define INLINE_DEBUG_COP2_READ_MFC
//#define INLINE_DEBUG_COP2_READ_CFC

#endif


Debug::Log COP2_Device::debug;


u32* COP2_Device::CPC_RegisterPtrs [ 32 ];
u32* COP2_Device::CPR_RegisterPtrs [ 32 ];
s16* COP2_Device::Matrix_Picker [ 4 ];	// = { &CPC2.R11, &CPC2.L11, &CPC2.LR1, &CPC2.LR1 };
s16* COP2_Device::Vector_Picker [ 4 ];	// = { &CPR2.VX0, &CPR2.VX1, &CPR2.VX2, &CPR2.VX2 };
s32* COP2_Device::CVector_Picker [ 4 ];	// = { &CPC2.TRX, &CPC2.RBK, &CPC2.RFC, Zero_Vector };

u8 COP2_Device::unr_table [ 257 ];


const char* COP2_Device::CPC_RegisterNames [ 32 ] = { 
	"R11R12", "R13R21", "R22R23", "R31R32", "ZeroR33", "TRX", "TRY", "TRZ",
	"L11L12", "L13L21", "L22L23", "L31L32", "ZeroL33", "RBK", "GBK", "BBK",
	"LR1LR2", "LR3LG1", "LG2LG3", "LB1LB2", "ZeroLB3", "RFC", "GFC", "BFC",
	"OFX", "OFY", "H", "DQA", "DQB", "ZSF3", "ZSF4", "FLAG"
};

const char* COP2_Device::CPR_RegisterNames [ 32 ] = {
	"VXY0", "ZeroVZ0", "VXY1", "ZeroVZ1", "VXY2", "ZeroVZ2", "RGB", "OTZ",
	"IR0", "IR1", "IR2", "IR3", "SXY0", "SXY1", "SXY2", "SXYP",
	"ZeroSZ0", "ZeroSZ1", "ZeroSZ2", "ZeroSZ3", "RGB0", "RGB1", "RGB2", "Reserved",
	"MAC0", "MAC1", "MAC2", "MAC3", "IRGB", "ORGB", "LZCS", "LZCR"
};





COP2_Device::COP2_Device ()
{

/*
#ifdef INLINE_DEBUG_ENABLE
	debug.Create ( "COP2_Log.txt" );
#endif

	Reset ();
*/

}


void COP2_Device::Generate_unr_table ()
{
	s64 Temp;
	
	for ( int i = 0; i < 257; i++ )
	{
		// unr_table[i]=min(0,(40000h/(i+100h)+1)/2-101h)
		Temp = ( ( 0x40000 / ( i + 0x100 ) + 1 ) / 2 ) - 0x101;
		if ( Temp < 0 ) Temp = 0;
		unr_table [ i ] = Temp;
	}
}


u32 COP2_Device::GTE_Divide ( u32 H, u32 SZ )
{
/*
if (H < SZ3*2) then                            ;check if overflow
    z = count_leading_zeroes(SZ3)                ;z=0..0Fh (for 16bit SZ3)
    n = (H SHL z)                                ;n=0..7FFF8000h
    d = (SZ3 SHL z)                              ;d=8000h..FFFFh
    u = unr_table[(d-7FC0h) SHR 7] + 101h        ;u=200h..101h
    d = ((2000080h - (d * u)) SHR 8)             ;d=10000h..0FF01h
    d = ((0000080h + (d * u)) SHR 8)             ;d=20000h..10000h
    n = min(1FFFFh, (((n*d) + 8000h) SHR 16))    ;n=0..1FFFFh
  else n = 1FFFFh, FLAG.Bit17=1, FLAG.Bit31=1    ;n=1FFFFh plus overflow flag
  */
  
	s64 z, n, d, u;
  
	if ( H < ( SZ * 2 ) )
	{
		z = CountLeadingZeros32 ( SZ ) - 16;
		n = ( H << z );
		d = ( SZ << z );
		u = ( (u32) unr_table [ ( d - 0x7fc0 ) >> 7 ] ) + 0x101;
		d = ( ( 0x2000080 - ( d * u ) ) >> 8 );
		d = ( ( 0x0000080 + ( d * u ) ) >> 8 );
		n = ( ( ( n * d ) + 0x8000 ) >> 16 );
		if ( n > 0x1ffff ) n = 0x1ffff;
		return n;
	}
	else
	{
		return Lm_E ( 0xfffff );
	}

}


void COP2_Device::Start ()
{
#ifdef INLINE_DEBUG_ENABLE
	debug.Create ( "COP2_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nCOP2_Device::Start";
#endif

	Reset ();
	
	// this gets cleared by reset
	//s16 Zero_Vector [ 4 ];	// = { 0, 0, 0, 0 };
	
	// generate the division table
	Generate_unr_table ();

	// this is rotation matrix, light matrix, or color matrix
	Matrix_Picker [ 0 ] = &CPC2.R11;
	Matrix_Picker [ 1 ] = &CPC2.L11;
	Matrix_Picker [ 2 ] = &CPC2.LR1;
	Matrix_Picker [ 3 ] = &CPC2.LR1;
	
	// this is v0, v1, v2, ir1-ir3
	// but v0,v1,v2 are 16-bit while IR is 16-bit but stored as 32-bit??
	//Vector_Picker [ 0 ] = &CPR2.VX0;
	//Vector_Picker [ 1 ] = &CPR2.VX1;
	//Vector_Picker [ 2 ] = &CPR2.VX2;
	//Vector_Picker [ 3 ] = &CPR2.IR1;
	
	// this is translation vector, back color vector, far color vector, zero
	CVector_Picker [ 0 ] = &CPC2.TRX;
	CVector_Picker [ 1 ] = &CPC2.RBK;
	CVector_Picker [ 2 ] = &CPC2.RFC;
	CVector_Picker [ 3 ] = & (Zero_Vector [ 0 ]);
	
	CPC_RegisterPtrs [ 0 ] = &CPC2.R11R12;
	CPC_RegisterPtrs [ 1 ] = &CPC2.R13R21;
	CPC_RegisterPtrs [ 2 ] = &CPC2.R22R23;
	CPC_RegisterPtrs [ 3 ] = &CPC2.R31R32;
	CPC_RegisterPtrs [ 4 ] = &CPC2.ZeroR33;
	CPC_RegisterPtrs [ 5 ] = (u32*) &CPC2.TRX;
	CPC_RegisterPtrs [ 6 ] = (u32*) &CPC2.TRY;
	CPC_RegisterPtrs [ 7 ] = (u32*) &CPC2.TRZ;
	CPC_RegisterPtrs [ 8 ] = &CPC2.L11L12;
	CPC_RegisterPtrs [ 9 ] = &CPC2.L13L21;
	CPC_RegisterPtrs [ 10 ] = &CPC2.L22L23;
	CPC_RegisterPtrs [ 11 ] = &CPC2.L31L32;
	CPC_RegisterPtrs [ 12 ] = &CPC2.ZeroL33;
	CPC_RegisterPtrs [ 13 ] = (u32*) &CPC2.RBK;
	CPC_RegisterPtrs [ 14 ] = (u32*) &CPC2.GBK;
	CPC_RegisterPtrs [ 15 ] = (u32*) &CPC2.BBK;
	CPC_RegisterPtrs [ 16 ] = &CPC2.LR1LR2;
	CPC_RegisterPtrs [ 17 ] = &CPC2.LR3LG1;
	CPC_RegisterPtrs [ 18 ] = &CPC2.LG2LG3;
	CPC_RegisterPtrs [ 19 ] = &CPC2.LB1LB2;
	CPC_RegisterPtrs [ 20 ] = &CPC2.ZeroLB3;
	CPC_RegisterPtrs [ 21 ] = (u32*) &CPC2.RFC;
	CPC_RegisterPtrs [ 22 ] = (u32*) &CPC2.GFC;
	CPC_RegisterPtrs [ 23 ] = (u32*) &CPC2.BFC;
	CPC_RegisterPtrs [ 24 ] = (u32*) &CPC2.OFX;
	CPC_RegisterPtrs [ 25 ] = (u32*) &CPC2.OFY;
	CPC_RegisterPtrs [ 26 ] = (u32*) &CPC2.H;
	CPC_RegisterPtrs [ 27 ] = (u32*) &CPC2.DQA;
	CPC_RegisterPtrs [ 28 ] = (u32*) &CPC2.DQB;
	CPC_RegisterPtrs [ 29 ] = (u32*) &CPC2.ZSF3;
	CPC_RegisterPtrs [ 30 ] = (u32*) &CPC2.ZSF4;
	CPC_RegisterPtrs [ 31 ] = &CPC2.FLAG.Value;
	
	
	CPR_RegisterPtrs [ 0 ] = &CPR2.VXY0;
	CPR_RegisterPtrs [ 1 ] = &CPR2.ZeroVZ0;
	CPR_RegisterPtrs [ 2 ] = &CPR2.VXY1;
	CPR_RegisterPtrs [ 3 ] = &CPR2.ZeroVZ1;
	CPR_RegisterPtrs [ 4 ] = &CPR2.VXY2;
	CPR_RegisterPtrs [ 5 ] = &CPR2.ZeroVZ2;
	CPR_RegisterPtrs [ 6 ] = &CPR2.RGB;
	CPR_RegisterPtrs [ 7 ] = (u32*) &CPR2.OTZ;
	CPR_RegisterPtrs [ 8 ] = (u32*)&CPR2.IR0;
	CPR_RegisterPtrs [ 9 ] = (u32*)&CPR2.IR1;
	CPR_RegisterPtrs [ 10 ] = (u32*)&CPR2.IR2;
	CPR_RegisterPtrs [ 11 ] = (u32*)&CPR2.IR3;
	CPR_RegisterPtrs [ 12 ] = &CPR2.SXY0;
	CPR_RegisterPtrs [ 13 ] = &CPR2.SXY1;
	CPR_RegisterPtrs [ 14 ] = &CPR2.SXY2;
	CPR_RegisterPtrs [ 15 ] = &CPR2.SXYP;
	CPR_RegisterPtrs [ 16 ] = (u32*) &CPR2.SZ0;
	CPR_RegisterPtrs [ 17 ] = (u32*) &CPR2.SZ1;
	CPR_RegisterPtrs [ 18 ] = (u32*) &CPR2.SZ2;
	CPR_RegisterPtrs [ 19 ] = (u32*) &CPR2.SZ3;
	CPR_RegisterPtrs [ 20 ] = &CPR2.RGB0;
	CPR_RegisterPtrs [ 21 ] = &CPR2.RGB1;
	CPR_RegisterPtrs [ 22 ] = &CPR2.RGB2;
	CPR_RegisterPtrs [ 23 ] = &CPR2.Reserved;
	CPR_RegisterPtrs [ 24 ] = (u32*) &CPR2.MAC0;
	CPR_RegisterPtrs [ 25 ] = (u32*) &CPR2.MAC1;
	CPR_RegisterPtrs [ 26 ] = (u32*) &CPR2.MAC2;
	CPR_RegisterPtrs [ 27 ] = (u32*) &CPR2.MAC3;
	CPR_RegisterPtrs [ 28 ] = &CPR2.IRGB;
	CPR_RegisterPtrs [ 29 ] = &CPR2.ORGB;
	CPR_RegisterPtrs [ 30 ] = (u32*) &CPR2.LZCS;
	CPR_RegisterPtrs [ 31 ] = &CPR2.LZCR;

#ifdef INLINE_DEBUG
	debug << "\r\nexiting COP2_Device::Start";
#endif
}



void COP2_Device::Reset ()
{
#ifdef INLINE_DEBUG
	debug << "\r\nResetting COP2";
#endif

	// zero object
	memset ( this, 0, sizeof( COP2_Device ) );
	
#ifdef INLINE_DEBUG
	debug << "->Reset of COP2 complete.";
#endif

}

// writing data regs
void COP2_Device::Write_MTC ( u32 Register, u32 Value )
{
#if defined INLINE_DEBUG_COP2_WRITE_MTC || defined INLINE_DEBUG_COP2_ALL || defined INLINE_DEBUG_NAME
	debug << "\r\n\r\n" << hex << setw( 8 ) << Cpu::_CPU->PC << dec << " " << Cpu::_CPU->CycleCount << " WRITE_MTC" << " Register#" << Register << " Value=" << hex << Value;
#endif

	// check if storing to lzcs
	switch ( Register )
	{
		/*
		case 1:
		case 3:
		case 5:
			// for these the upper part of register is zero
			CPR2.Regs [ Register ] = (u16) Value;
			break;
		*/
			
		case 1:
		case 3:
		case 5:
		case 8:
		case 9:
		case 10:
		case 11:
			// here, you need to show the sign in the upper 16-bits of 32-bit COP2 registers for IR0,IR1,IR2,IR3
			CPR2.Regs [ Register ] = (s32) ( (s16) Value );
			break;
			
			
		case 7:
		case 16:
		case 17:
		case 18:
		case 19:
			// upper 16-bits of register is zero
			CPR2.Regs [ Register ] = (u16) Value;
			break;
			
		case 15:
			// writing to SXYP puts data onto SXY fifo
			CPR2.SXY0 = CPR2.SXY1;
			CPR2.SXY1 = CPR2.SXY2;
			CPR2.SXY2 = Value;
			
			break;
			
		case 28:
			// writing to IRGB
			
			// store value
			CPR2.Regs [ 28 ] = Value;
			
			// set output
			CPR2.IR1 = ( Value & 0x1f ) << 7;
			CPR2.IR2 = ( Value & 0x3e0 ) << 2;
			CPR2.IR3 = ( Value & 0x7c00 ) >> 3;
			break;
	
		case 30:
			// now we have to put the result for leading zero count in lzcr
			
			// note: results are in the range of 1-32
			
			// store value
			CPR2.Regs [ 30 ] = Value;
			
			if ( ((s32)Value) < 0 )
			{
				// if negative, it is actually a leading one count
				//CPR2.Regs [ 31 ] = 31 - x64Asm::Utilities::BSR ( ~Value );
				CPR2.Regs [ 31 ] = CountLeadingZeros32 ( ~Value );
			}
			else
			{
				// is positive, so do a leading zero count //
				//CPR2.Regs [ 31 ] = 31 - x64Asm::Utilities::BSR ( Value );
				CPR2.Regs [ 31 ] = CountLeadingZeros32 ( Value );
			}
			
			break;

			
		// these registers are read-only
		case 29:
		case 31:
			break;
			
		default:
			CPR2.Regs [ Register ] = Value;
			break;
		
	}
}

// writing control regs
void COP2_Device::Write_CTC ( u32 Register, u32 Value )
{
#if defined INLINE_DEBUG_COP2_WRITE_CTC || defined INLINE_DEBUG_COP2_ALL || defined INLINE_DEBUG_NAME
	debug << "\r\n\r\n" << hex << setw( 8 ) << Cpu::_CPU->PC << dec << " " << Cpu::_CPU->CycleCount << " WRITE_CTC" << " Register#" << Register << " Value=" << hex << Value;
#endif

	switch ( Register )
	{
		case 4:
		case 12:
		case 20:
		case 26:
		case 27:
		case 29:
		case 30:
			CPC2.Regs [ Register ] = (s32) ( (s16) Value );
			break;
			
		case 31:
		
			// *** testing ***
			//cout << "\nhps1x64: writing flag register @ Cycle#" << dec << R3000A::Cpu::_CPU->CycleCount << " PC=" << hex << R3000A::Cpu::_CPU->PC << " Value=" << Value << "\n";
			
			Value = Value & 0x7ffff000;
			if ( Value & 0x7f87e000 ) Value |= 0x80000000;
			CPC2.Regs [ Register ] = Value;
			break;
			
		default:
			CPC2.Regs [ Register ] = Value;
			break;
	}
}

u32 COP2_Device::Read_MFC ( u32 Register )
{
#if defined INLINE_DEBUG_COP2_READ_MFC || defined INLINE_DEBUG_COP2_ALL || defined INLINE_DEBUG_NAME
	debug << "\r\n\r\n" << hex << setw( 8 ) << Cpu::_CPU->PC << dec << " " << Cpu::_CPU->CycleCount << " READ_MFC" << " Register#" << Register;
#endif

	switch ( Register )
	{
		/*
		case 1:
		case 3:
		case 5:
			// upper part of register should be zero here
			return ( (u16) (CPR2.Regs [ Register ]) );
			break;
		*/
		
		case 1:
		case 3:
		case 5:
		case 8:
		case 9:
		case 10:
		case 11:
			// IR0, IR1, IR2, IR3 are signed 16-bit values with the upper portion of the register repeating the sign bit
			return ((s32) ( (s16) (CPR2.Regs [ Register ]) ));
			break;
		
		// this stuff is cool since the upper 16-bits are always zero
		// but unfortunately, the upper 16-bits keeps getting stuff in there, so I need to put this back
		case 7:
		case 16:
		case 17:
		case 18:
		case 19:
			return ((u32) ( (u16) (CPR2.Regs [ Register ]) ));
			break;
			
		case 15:
			return CPR2.SXY2;
			break;

		case 28:
		case 29:
			// reading from IRGB/ORGB
			u32 IR, IG, IB;
			IR = ( CPR2.IR1 >> 7 ) > 0x1f ? 0x1f : ( ( CPR2.IR1 >> 7 ) < 0 ? 0 : ( CPR2.IR1 >> 7 ) );
			IG = ( CPR2.IR2 >> 7 ) > 0x1f ? 0x1f : ( ( CPR2.IR2 >> 7 ) < 0 ? 0 : ( CPR2.IR2 >> 7 ) );
			IB = ( CPR2.IR3 >> 7 ) > 0x1f ? 0x1f : ( ( CPR2.IR3 >> 7 ) < 0 ? 0 : ( CPR2.IR3 >> 7 ) );
			return ( ( IB << 10 ) | ( IG << 5 ) | ( IR ) );
			break;
			
		default:
			return CPR2.Regs [ Register ];
			break;

	}
	
	return CPR2.Regs [ Register ];
}

u32 COP2_Device::Read_CFC ( u32 Register )
{
#if defined INLINE_DEBUG_COP2_READ_CFC || defined INLINE_DEBUG_COP2_ALL || defined INLINE_DEBUG_NAME
	debug << "\r\n\r\n" << hex << setw( 8 ) << Cpu::_CPU->PC << dec << " " << Cpu::_CPU->CycleCount << " READ_CFC" << " Register#" << Register;
#endif

	CPC2.zero0 = 0;
	CPC2.zero1 = 0;
	CPC2.zero2 = 0;
	CPC2.zero3 = 0;
	CPC2.zero4 = 0;
	CPC2.zero5 = 0;
	CPC2.zero6 = 0;
	
	switch ( Register )
	{
		// previous stuff
		//case 26:
		
		// *** testing *** new stuff
		case 4:
		case 12:
		case 20:
		case 26:
		case 27:
		case 29:
		case 30:
			// H register returns as sign extended, even though it is unsigned
			// issue only applies to COP2 mov instructions, not actual calculations
			return ((s32) ((s16) CPC2.Regs [ Register ]));
			break;
			
		case 31:
		
			// *** testing ***
			//cout << "\nhps1x64: reading flag register @ Cycle#" << dec << R3000A::Cpu::_CPU->CycleCount << " PC=" << hex << R3000A::Cpu::_CPU->PC << " Value=" << CPC2.Regs [ Register ] << "\n";
			
			return CPC2.Regs [ Register ];
			break;
		
		default:
			return CPC2.Regs [ Register ];
			break;
	}
	
	return CPC2.Regs [ Register ];
}



// if a register is loading after load-delay slot, then we need to stall the pipeline until it finishes loading from memory
// if register is loading but is stored to, then we should clear loading flag for register and cancel load
#define CHECK_LOADING_COP2(arg)	if ( CPRLoading_Bitmap & ( arg ) ) /*{ r->BusyUntil_Cycle = r->Bus->BusyUntil_Cycle; return false; }*/

// we can also cancel the loading of a register if it gets written to before load is complete
#define CANCEL_LOADING_COP2(arg) ( CPRLoading_Bitmap &= ~( arg ) )




void COP2_Device::RTPS ( Cpu* r, Instruction::Format i )
{
	static const u32 InputCPCRegs [] = { INDEX_TRX, INDEX_TRY, INDEX_TRZ, INDEX_R11R12, INDEX_R13R21, INDEX_R22R23, INDEX_R31R32, INDEX_ZeroR33,
								INDEX_H, INDEX_OFX, INDEX_OFY, INDEX_DQA, INDEX_DQB };
								
	static const u32 InputCPRRegs [] = { INDEX_VXY0, INDEX_ZeroVZ0, INDEX_SZ3, INDEX_SXY2 };

	static const u32 OutputCPCRegs [] = { INDEX_FLAG };
	static const u32 OutputCPRRegs [] = { INDEX_MAC1, INDEX_MAC2, INDEX_MAC3, INDEX_IR0, INDEX_IR1, INDEX_IR2, INDEX_IR3, INDEX_SZ0, INDEX_SZ1,
										INDEX_SZ2, INDEX_SZ3, INDEX_SXY0, INDEX_SXY1, INDEX_SXY2 };

#if defined INLINE_DEBUG_COP2_RTPS || defined INLINE_DEBUG_COP2_ALL || defined INLINE_DEBUG_NAME
	//debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << "  " << "RTPS" << "  " << dec << r->CycleCount << hex;
	debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << dec << " " << r->CycleCount << " " << "RTPS";
	debug << " " << i.Value << " lm=" << i.lm << " sf=" << i.sf;
#endif

#if defined INLINE_DEBUG_COP2_RTPS || defined INLINE_DEBUG_COP2_ALL
	int k;
	// show input values
	debug << "\r\n" << hex << "Input(CPC): ";
	for ( k = 0; k < sizeof(InputCPCRegs)/sizeof(InputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ InputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ InputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Input(CPR): ";
	for ( k = 0; k < sizeof(InputCPRRegs)/sizeof(InputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ InputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ InputCPRRegs [ k ] ]) << ";";
	}
#endif

	// 15 cycles
	// cop2 $0180001
	static const u32 c_InstructionCycles = 15;

	// this constant says what CPR registers need to finish loading before we can execute instruction
	// *note* this should include all CPR registers on the right side of the equal sign that are being read from before being stored to
	static const u32 InputRegs_Bitmap = BITMAP_VXY0 | BITMAP_ZeroVZ0 | BITMAP_SZ3 | BITMAP_SXY2;

	// input (CPC): TRX, TRY, TRZ, R11, R12, R13, R21, R22, R23, R31, R32, R33, H, OFX, OFY, DQA, DQB
	// input (CPR): VX0, VY0, VZ0, SZ3, SXY2
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR0, IR1, IR2, IR3, SZ0, SZ1, SZ2, SZ3, SXY0, SXY1, SX2, SY2
	
	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	s64 MAC3_Temp;
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	// clear flags
	CPC2.FLAG.Value = 0;
	
	// [1,31,0] MAC1=A1[TRX + R11*VX0 + R12*VY0 + R13*VZ0]            [1,31,12]
	MAC1 = A64_1( ( ( ((s64)CPC2.TRX) << 12 ) + ( (((s64)CPC2.R11) * ((s64)CPR2.VX0)) + (((s64)CPC2.R12) * ((s64)CPR2.VY0)) + (((s64)CPC2.R13) * ((s64)CPR2.VZ0)) ) ) ) >> shift;
	
	// [1,31,0] MAC2=A2[TRY + R21*VX0 + R22*VY0 + R23*VZ0]            [1,31,12]
	MAC2 = A64_2( ( ( ((s64)CPC2.TRY) << 12 ) + ( (((s64)CPC2.R21) * ((s64)CPR2.VX0)) + (((s64)CPC2.R22) * ((s64)CPR2.VY0)) + (((s64)CPC2.R23) * ((s64)CPR2.VZ0)) ) ) ) >> shift;
	
	// [1,31,0] MAC3=A3[TRZ + R31*VX0 + R32*VY0 + R33*VZ0]            [1,31,12]
	// note: according to martin korth psx spec, B3 is only set when MAC3 >> 12 saturates regardless of sf
	MAC3 = A64_3( ( ( ((s64)CPC2.TRZ) << 12 ) + ( (((s64)CPC2.R31) * ((s64)CPR2.VX0)) + (((s64)CPC2.R32) * ((s64)CPR2.VY0)) + (((s64)CPC2.R33) * ((s64)CPR2.VZ0)) ) ) ) >> shift;

//#define DEBUG_RTPS
#ifdef DEBUG_RTPS
	debug << "\r\nMAC1=" << dec << CPR2.MAC1 << " R11=" << CPC2.R11 << " R12=" << CPC2.R12 << " R13=" << CPC2.R13 << " VX0=" << CPR2.VX0 << " VY0=" << CPR2.VY0 << " VZ0=" << CPR2.VZ0 << " TRX=" << CPC2.TRX;
#endif

	
	// [1,15,0] IR1= Lm_B1[MAC1]                                      [1,31,0]
	CPR2.IR1 = Lm_B1 ( MAC1, 0 );
	
	//[1,15,0] IR2= Lm_B2[MAC2]                                      [1,31,0]
	CPR2.IR2 = Lm_B2 ( MAC2, 0 );
	
	//[1,15,0] IR3= Lm_B3[MAC3]                                      [1,31,0]
	// note: according to martin korth psx spec, B3 is only set when MAC3 >> 12 saturates regardless of sf
	CPR2.IR3 = Lm_B3 ( MAC3, 0 );
	
	//         SZ0<-SZ1<-SZ2<-SZ3
	CPR2.SZ0 = CPR2.SZ1;
	CPR2.SZ1 = CPR2.SZ2;
	CPR2.SZ2 = CPR2.SZ3;
	
	//[0,16,0] SZ3= Lm_D(MAC3)                                       [1,31,0]
	CPR2.SZ3 = (u16) Lm_D ( MAC3 >> ( 12 - shift ) );
	
	//         SX0<-SX1<-SX2, SY0<-SY1<-SY2
	CPR2.SXY0 = CPR2.SXY1;
	CPR2.SXY1 = CPR2.SXY2;
	
	// Get H/SZ		[ 0, 16, 0 ]
	// getting in 16.16 fixed point
	//quotient = Lm_E ( gte_divide ( CPC2.H, CPR2.SZ3 ) );		//(u64) ( ( ((u32)CPC2.H) << 16 ) / (u32)CPR2.SZ3);
	//if ( !CPR2.SZ3 ) quotient = Lm_E ( 0xffffff ); else quotient = Lm_E ( ( ( ((u64)CPC2.H) << 16 ) + ( ((u64)CPR2.SZ3) >> 1 ) ) / ((u64)CPR2.SZ3) );
	quotient = GTE_Divide ( CPC2.H, CPR2.SZ3 );
	
	//[1,15,0] SX2= Lm_G1[F[OFX + IR1*(H/SZ)]]                       [1,27,16]
	CPR2.SX2 = (s16) ( Lm_G1( F( ((s64)CPC2.OFX) + ( ((s64)CPR2.IR1) * ((s64)quotient) ) ) >> 16 ) );
	
	//[1,15,0] SY2= Lm_G2[F[OFY + IR2*(H/SZ)]]                       [1,27,16]
	CPR2.SY2 = (s16) ( Lm_G2( F( ((s64)CPC2.OFY) + ( ((s64)CPR2.IR2) * ((s64)quotient) ) ) >> 16 ) );
	
#ifdef DEBUG_RTPS
	debug << "\r\nOFX=" << CPC2.OFX << " IR1=" << CPR2.IR1 << " quotient=" << quotient << " SX2=" << CPR2.SX2;
#endif

	// this also should get stored to SXYP
	//CPR2.SXYP = CPR2.SXY2;
	
	//[1,31,0] MAC0= F[DQB + DQA * (H/SZ)]                           [1,19,24]
	//CPR2.MAC0 = F( ( ((s64)CPC2.DQB) + ( (s64) ( ((s64)CPC2.DQA) * ((s64)quotient) ) ) ) >> 12 );
	MAC0 = F( ( ((s64)CPC2.DQB) + ( (s64) ( ((s64)CPC2.DQA) * ((s64)quotient) ) ) ) );
	
	//[1,15,0] IR0= Lm_H[MAC0]                                       [1,31,0]
	//CPR2.IR0 = Lm_H ( CPR2.MAC0 );
	//CPR2.IR0 = Lm_H ( CPR2.MAC0 >> 12 );
	CPR2.IR0 = Lm_H ( MAC0 >> 12 );
	
	// write back MAC (44-bit register)
	CPR2.MAC0 = (s32) MAC0;
	CPR2.MAC1 = (s32) MAC1;
	CPR2.MAC2 = (s32) MAC2;
	CPR2.MAC3 = (s32) MAC3;
	
#if defined INLINE_DEBUG_COP2_RTPS || defined INLINE_DEBUG_COP2_ALL
	// show output values
	debug << "\r\n" << hex << "Output(CPC): ";
	for ( k = 0; k < sizeof(OutputCPCRegs)/sizeof(OutputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ OutputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ OutputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Output(CPR): ";
	for ( k = 0; k < sizeof(OutputCPRRegs)/sizeof(OutputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ OutputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ OutputCPRRegs [ k ] ]) << ";";
	}
	
#endif

	//return true;
}

void COP2_Device::RTPT ( Cpu* r, Instruction::Format i )
{

	static const u32 InputCPCRegs [] = { INDEX_TRX, INDEX_TRY, INDEX_TRZ, INDEX_R11R12, INDEX_R13R21, INDEX_R22R23, INDEX_R31R32, INDEX_ZeroR33,
								INDEX_H, INDEX_OFX, INDEX_OFY, INDEX_DQA, INDEX_DQB };
								
	static const u32 InputCPRRegs [] = { INDEX_VXY0, INDEX_ZeroVZ0, INDEX_VXY1, INDEX_ZeroVZ1, INDEX_VXY2, INDEX_ZeroVZ2, INDEX_SZ3, INDEX_SXY2 };

	static const u32 OutputCPCRegs [] = { INDEX_FLAG };
	static const u32 OutputCPRRegs [] = { INDEX_MAC1, INDEX_MAC2, INDEX_MAC3, INDEX_IR0, INDEX_IR1, INDEX_IR2, INDEX_IR3, INDEX_SZ0, INDEX_SZ1,
										INDEX_SZ2, INDEX_SZ3, INDEX_SXY0, INDEX_SXY1, INDEX_SXY2 };

#if defined INLINE_DEBUG_COP2_RTPT || defined INLINE_DEBUG_COP2_ALL || defined INLINE_DEBUG_NAME
	//debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << "  " << "RTPT" << "  " << dec << r->CycleCount << hex;
	debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << dec << " " << r->CycleCount << " " << "RTPT";
	debug << " " << i.Value << " lm=" << i.lm << " sf=" << i.sf;
#endif
	
#if defined INLINE_DEBUG_COP2_RTPT || defined INLINE_DEBUG_COP2_ALL
	int k;
	// show input values
	debug << "\r\n" << hex << "Input(CPC): ";
	for ( k = 0; k < sizeof(InputCPCRegs)/sizeof(InputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ InputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ InputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Input(CPR): ";
	for ( k = 0; k < sizeof(InputCPRRegs)/sizeof(InputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ InputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ InputCPRRegs [ k ] ]) << ";";
	}
#endif


	// 23 cycles
	// cop2 $0280030
	static const u32 c_InstructionCycles = 23;
	
	static const u32 InputRegs_Bitmap = BITMAP_VXY0 | BITMAP_ZeroVZ0 | 
										BITMAP_VXY1 | BITMAP_ZeroVZ1 | 
										BITMAP_VXY2 | BITMAP_ZeroVZ2 | 
										BITMAP_SZ3 | BITMAP_SXY2;
	
	// input (CPC): TRX, TRY, TRZ, R11, R12, R13, R21, R22, R23, R31, R32, R33, H, OFX, OFY, DQA, DQB
	// input (CPR): VX0, VY0, VZ0, VX1, VY1, VZ1, VX2, VY2, VZ2, SZ3, SXY2
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR0, IR1, IR2, IR3, SZ0, SZ1, SZ2, SZ3, SXY0, SXY1, SXY2
	
	s64 MAC3_Temp;
	
	// same as RTPS, just repeat for V1 and V2
	
	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	// clear flags
	CPC2.FLAG.Value = 0;
	
	// for V0
	
	// [1,31,0] MAC1=A1[TRX + R11*VX0 + R12*VY0 + R13*VZ0]            [1,31,12]
	MAC1 = A64_1( ( ( ((s64)CPC2.TRX) << 12 ) + ( (((s64)CPC2.R11) * ((s64)CPR2.VX0)) + (((s64)CPC2.R12) * ((s64)CPR2.VY0)) + (((s64)CPC2.R13) * ((s64)CPR2.VZ0)) ) ) ) >> shift;
	
	// [1,31,0] MAC2=A2[TRY + R21*VX0 + R22*VY0 + R23*VZ0]            [1,31,12]
	MAC2 = A64_2( ( ( ((s64)CPC2.TRY) << 12 ) + ( (((s64)CPC2.R21) * ((s64)CPR2.VX0)) + (((s64)CPC2.R22) * ((s64)CPR2.VY0)) + (((s64)CPC2.R23) * ((s64)CPR2.VZ0)) ) ) ) >> shift;
	
	// [1,31,0] MAC3=A3[TRZ + R31*VX0 + R32*VY0 + R33*VZ0]            [1,31,12]
	MAC3 = A64_3( ( ( ((s64)CPC2.TRZ) << 12 ) + ( (((s64)CPC2.R31) * ((s64)CPR2.VX0)) + (((s64)CPC2.R32) * ((s64)CPR2.VY0)) + (((s64)CPC2.R33) * ((s64)CPR2.VZ0)) ) ) ) >> shift;
	
//#define DEBUG_RTPT

#ifdef DEBUG_RTPT
	debug << "\r\nMAC1=" << hex << CPR2.MAC1 << " MAC2=" << CPR2.MAC2 << " MAC3=" << CPR2.MAC3;
#endif

#ifdef DEBUG_RTPS
	debug << "\r\nMAC1=" << dec << CPR2.MAC1 << " R11=" << CPC2.R11 << " R12=" << CPC2.R12 << " R13=" << CPC2.R13 << " VX0=" << CPR2.VX0 << " VY0=" << CPR2.VY0 << " VZ0=" << CPR2.VZ0 << " TRX=" << CPC2.TRX;
	debug << "\r\nMAC2=" << dec << CPR2.MAC2 << " R21=" << CPC2.R21 << " R22=" << CPC2.R22 << " R23=" << CPC2.R23;
	debug << "\r\nMAC3=" << dec << CPR2.MAC3 << " R31=" << CPC2.R31 << " R32=" << CPC2.R32 << " R33=" << CPC2.R33;
#endif
	
	// [1,15,0] IR1= Lm_B1[MAC1]                                      [1,31,0]
	CPR2.IR1 = Lm_B1 ( MAC1, 0 );
	
	//[1,15,0] IR2= Lm_B2[MAC2]                                      [1,31,0]
	CPR2.IR2 = Lm_B2 ( MAC2, 0 );
	
	//[1,15,0] IR3= Lm_B3[MAC3]                                      [1,31,0]
	CPR2.IR3 = Lm_B3 ( MAC3, 0 );
	
#ifdef DEBUG_RTPT
	debug << "\r\nIR1=" << hex << CPR2.IR1 << " IR2=" << CPR2.IR2 << " IR3=" << CPR2.IR3;
#endif

	//         SZ0<-SZ1<-SZ2<-SZ3
	//CPR2.SZ0 = CPR2.SZ1;
	//CPR2.SZ1 = CPR2.SZ2;
	//CPR2.SZ2 = CPR2.SZ3;
	
	//[0,16,0] SZ3= Lm_D(MAC3)                                       [1,31,0]
	//CPR2.SZ3 = Lm_D ( CPR2.MAC3 );
	CPR2.SZ0 = CPR2.SZ3;
	CPR2.SZ1 = Lm_D ( MAC3 >> ( 12 - shift ) );
	
	//         SX0<-SX1<-SX2, SY0<-SY1<-SY2
	//CPR2.SXY0 = CPR2.SXY1;
	//CPR2.SXY1 = CPR2.SXY2;
	
	// Get H/SZ		[ 0, 16, 0 ]
	// getting in 16.16 fixed point
	//quotient = Lm_E ( gte_divide ( CPC2.H, CPR2.SZ1 ) );
	//if ( !CPR2.SZ1 ) quotient = Lm_E ( 0xffffff ); else quotient = Lm_E ( ( ( ((u64)CPC2.H) << 16 ) + ( ((u64)CPR2.SZ1) >> 1 ) ) / ((u64)CPR2.SZ1) );
	quotient = GTE_Divide ( CPC2.H, CPR2.SZ1 );
	
	//[1,15,0] SX2= Lm_G1[F[OFX + IR1*(H/SZ)]]                       [1,27,16]
	//CPR2.SX2 = Lm_G1( F( ((s64)CPC2.OFX) + ( ((s64)CPR2.IR1) * ((s64)quotient) ) ) >> 16 );
	CPR2.SX0 = Lm_G1( F( ((s64)CPC2.OFX) + ( ((s64)CPR2.IR1) * ((s64)quotient) ) ) >> 16 );
	
	//[1,15,0] SY2= Lm_G2[F[OFY + IR2*(H/SZ)]]                       [1,27,16]
	//CPR2.SY2 = Lm_G2( F( ((s64)CPC2.OFY) + ( ((s64)CPR2.IR2) * ((s64)quotient) ) ) >> 16 );
	CPR2.SY0 = Lm_G2( F( ((s64)CPC2.OFY) + ( ((s64)CPR2.IR2) * ((s64)quotient) ) ) >> 16 );
	
#ifdef DEBUG_RTPT
	debug << "\r\nSZ1=" << hex << CPR2.SZ1 << " quotient=" << quotient << " SX0=" << CPR2.SX0 << " SY0=" << CPR2.SY0;
#endif

#ifdef DEBUG_RTPS
	debug << "\r\nOFX=" << CPC2.OFX << " IR1=" << CPR2.IR1 << " quotient=" << quotient << " SX0=" << CPR2.SX0;
#endif

	// this also should get stored to SXYP
	//CPR2.SXYP = CPR2.SXY2;
	
	// *** must do these to set the flags *** //
	
	//[1,31,0] MAC0= F[DQB + DQA * (H/SZ)]                           [1,19,24]
	//CPR2.MAC0 = Lm_F( ( ((s64)CPC2.DQB) + ( ((s64)CPC2.DQA) * ((s64)quotient) ) ) >> 12 );
	MAC0 = F( ( ((s64)CPC2.DQB) + ( ((s64)CPC2.DQA) * ((s64)quotient) ) ) );
	
	//[1,15,0] IR0= Lm_H[MAC0]                                       [1,31,0]
	//CPR2.IR0 = Lm_H ( CPR2.MAC0 );
	CPR2.IR0 = Lm_H ( MAC0 >> 12 );

	////////////
	// for V1 //
	////////////
	
	// [1,31,0] MAC1=A1[TRX + R11*VX0 + R12*VY0 + R13*VZ0]            [1,31,12]
	MAC1 = A64_1( ( ( ((s64)CPC2.TRX) << 12 ) + ( (((s64)CPC2.R11) * ((s64)CPR2.VX1)) + (((s64)CPC2.R12) * ((s64)CPR2.VY1)) + (((s64)CPC2.R13) * ((s64)CPR2.VZ1)) ) ) ) >> shift;
	
	// [1,31,0] MAC2=A2[TRY + R21*VX0 + R22*VY0 + R23*VZ0]            [1,31,12]
	MAC2 = A64_2( ( ( ((s64)CPC2.TRY) << 12 ) + ( (((s64)CPC2.R21) * ((s64)CPR2.VX1)) + (((s64)CPC2.R22) * ((s64)CPR2.VY1)) + (((s64)CPC2.R23) * ((s64)CPR2.VZ1)) ) ) ) >> shift;
	
	// [1,31,0] MAC3=A3[TRZ + R31*VX0 + R32*VY0 + R33*VZ0]            [1,31,12]
	MAC3 = A64_3( ( ( ((s64)CPC2.TRZ) << 12 ) + ( (((s64)CPC2.R31) * ((s64)CPR2.VX1)) + (((s64)CPC2.R32) * ((s64)CPR2.VY1)) + (((s64)CPC2.R33) * ((s64)CPR2.VZ1)) ) ) ) >> shift;
	
	// [1,15,0] IR1= Lm_B1[MAC1]                                      [1,31,0]
	CPR2.IR1 = Lm_B1 ( MAC1, 0 );
	
	//[1,15,0] IR2= Lm_B2[MAC2]                                      [1,31,0]
	CPR2.IR2 = Lm_B2 ( MAC2, 0 );
	
	//[1,15,0] IR3= Lm_B3[MAC3]                                      [1,31,0]
	CPR2.IR3 = Lm_B3 ( MAC3, 0 );
	
#ifdef DEBUG_RTPS
	debug << "\r\nMAC1=" << dec << CPR2.MAC1 << " VX1=" << CPR2.VX1 << " VY1=" << CPR2.VY1 << " VZ1=" << CPR2.VZ1;
	debug << "\r\nMAC2=" << dec << CPR2.MAC2 << " VX1=" << CPR2.VX1 << " VY1=" << CPR2.VY1 << " VZ1=" << CPR2.VZ1;
	debug << "\r\nMAC3=" << dec << CPR2.MAC3 << " VX1=" << CPR2.VX1 << " VY1=" << CPR2.VY1 << " VZ1=" << CPR2.VZ1;
#endif

	//         SZ0<-SZ1<-SZ2<-SZ3
	//CPR2.SZ0 = CPR2.SZ1;
	//CPR2.SZ1 = CPR2.SZ2;
	//CPR2.SZ2 = CPR2.SZ3;
	
	//[0,16,0] SZ3= Lm_D(MAC3)                                       [1,31,0]
	//CPR2.SZ3 = Lm_D ( CPR2.MAC3 );
	CPR2.SZ2 = Lm_D ( MAC3 >> ( 12 - shift ) );
	
	//         SX0<-SX1<-SX2, SY0<-SY1<-SY2
	//CPR2.SXY0 = CPR2.SXY1;
	//CPR2.SXY1 = CPR2.SXY2;
	
	// Get H/SZ		[ 0, 16, 0 ]
	// getting in 16.16 fixed point
	//quotient = Lm_E ( gte_divide ( CPC2.H, CPR2.SZ2 ) );
	//if ( !CPR2.SZ2 ) quotient = Lm_E ( 0xffffff ); else quotient = Lm_E ( ( ( ((u64)CPC2.H) << 16 ) + ( ((u64)CPR2.SZ2) >> 1 ) ) / ((u64)CPR2.SZ2) );
	quotient = GTE_Divide ( CPC2.H, CPR2.SZ2 );
	
	//[1,15,0] SX2= Lm_G1[F[OFX + IR1*(H/SZ)]]                       [1,27,16]
	//CPR2.SX2 = Lm_G1( F( ((s64)CPC2.OFX) + ( ((s64)CPR2.IR1) * ((s64)quotient) ) ) >> 16 );
	CPR2.SX1 = Lm_G1( F( ((s64)CPC2.OFX) + ( ((s64)CPR2.IR1) * ((s64)quotient) ) ) >> 16 );
	
	//[1,15,0] SY2= Lm_G2[F[OFY + IR2*(H/SZ)]]                       [1,27,16]
	//CPR2.SY2 = Lm_G2( F( ((s64)CPC2.OFY) + ( ((s64)CPR2.IR2) * ((s64)quotient) ) ) >> 16 );
	CPR2.SY1 = Lm_G2( F( ((s64)CPC2.OFY) + ( ((s64)CPR2.IR2) * ((s64)quotient) ) ) >> 16 );
	
	// this also should get stored to SXYP
	//CPR2.SXYP = CPR2.SXY2;
	
	// *** must do these to set the flags *** //
	
	//[1,31,0] MAC0= F[DQB + DQA * (H/SZ)]                           [1,19,24]
	//CPR2.MAC0 = Lm_F( ( ((s64)CPC2.DQB) + ( ((s64)CPC2.DQA) * ((s64)quotient) ) ) >> 12 );
	MAC0 = F( ( ((s64)CPC2.DQB) + ( ((s64)CPC2.DQA) * ((s64)quotient) ) ) );
	
	//[1,15,0] IR0= Lm_H[MAC0]                                       [1,31,0]
	//CPR2.IR0 = Lm_H ( CPR2.MAC0 );
	CPR2.IR0 = Lm_H ( MAC0 >> 12 );

	////////////
	// for V2 //
	////////////
	
	// [1,31,0] MAC1=A1[TRX + R11*VX0 + R12*VY0 + R13*VZ0]            [1,31,12]
	MAC1 = A64_1( ( ( ((s64)CPC2.TRX) << 12 ) + ( (((s64)CPC2.R11) * ((s64)CPR2.VX2)) + (((s64)CPC2.R12) * ((s64)CPR2.VY2)) + (((s64)CPC2.R13) * ((s64)CPR2.VZ2)) ) ) ) >> shift;
	
	// [1,31,0] MAC2=A2[TRY + R21*VX0 + R22*VY0 + R23*VZ0]            [1,31,12]
	MAC2 = A64_2( ( ( ((s64)CPC2.TRY) << 12 ) + ( (((s64)CPC2.R21) * ((s64)CPR2.VX2)) + (((s64)CPC2.R22) * ((s64)CPR2.VY2)) + (((s64)CPC2.R23) * ((s64)CPR2.VZ2)) ) ) ) >> shift;
	
	// [1,31,0] MAC3=A3[TRZ + R31*VX0 + R32*VY0 + R33*VZ0]            [1,31,12]
	MAC3 = A64_3( ( ( ((s64)CPC2.TRZ) << 12 ) + ( (((s64)CPC2.R31) * ((s64)CPR2.VX2)) + (((s64)CPC2.R32) * ((s64)CPR2.VY2)) + (((s64)CPC2.R33) * ((s64)CPR2.VZ2)) ) ) ) >> shift;
	
	// [1,15,0] IR1= Lm_B1[MAC1]                                      [1,31,0]
	CPR2.IR1 = Lm_B1 ( MAC1, 0 );
	
	//[1,15,0] IR2= Lm_B2[MAC2]                                      [1,31,0]
	CPR2.IR2 = Lm_B2 ( MAC2, 0 );
	
	//[1,15,0] IR3= Lm_B3[MAC3]                                      [1,31,0]
	CPR2.IR3 = Lm_B3 ( MAC3, 0 );
	
#ifdef DEBUG_RTPS
	debug << "\r\nMAC1=" << dec << CPR2.MAC1 << " VX2=" << CPR2.VX2 << " VY2=" << CPR2.VY2 << " VZ2=" << CPR2.VZ2;
	debug << "\r\nMAC2=" << dec << CPR2.MAC2 << " VX2=" << CPR2.VX2 << " VY2=" << CPR2.VY2 << " VZ2=" << CPR2.VZ2;
	debug << "\r\nMAC3=" << dec << CPR2.MAC3 << " VX2=" << CPR2.VX2 << " VY2=" << CPR2.VY2 << " VZ2=" << CPR2.VZ2;
#endif

	//         SZ0<-SZ1<-SZ2<-SZ3
	//CPR2.SZ0 = CPR2.SZ1;
	//CPR2.SZ1 = CPR2.SZ2;
	//CPR2.SZ2 = CPR2.SZ3;
	
	//[0,16,0] SZ3= Lm_D(MAC3)                                       [1,31,0]
	CPR2.SZ3 = Lm_D ( MAC3 >> ( 12 - shift ) );
	
	//         SX0<-SX1<-SX2, SY0<-SY1<-SY2
	//CPR2.SXY0 = CPR2.SXY1;
	//CPR2.SXY1 = CPR2.SXY2;
	
	// Get H/SZ		[ 0, 16, 0 ]
	// getting in 16.16 fixed point
	//quotient = Lm_E ( gte_divide ( CPC2.H, CPR2.SZ3 ) );		//(u64) ( ( ((u32)CPC2.H) << 16 ) / (u32)CPR2.SZ3 );
	//if ( !CPR2.SZ3 ) quotient = Lm_E ( 0xffffff ); else quotient = Lm_E ( ( ( ((u64)CPC2.H) << 16 ) + ( ((u64)CPR2.SZ3) >> 1 ) ) / ((u64)CPR2.SZ3) );
	quotient = GTE_Divide ( CPC2.H, CPR2.SZ3 );
	
	//[1,15,0] SX2= Lm_G1[F[OFX + IR1*(H/SZ)]]                       [1,27,16]
	CPR2.SX2 = Lm_G1( F( ((s64)CPC2.OFX) + ( ((s64)CPR2.IR1) * ((s64)quotient) ) ) >> 16 );
	
	//[1,15,0] SY2= Lm_G2[F[OFY + IR2*(H/SZ)]]                       [1,27,16]
	CPR2.SY2 = Lm_G2( F( ((s64)CPC2.OFY) + ( ((s64)CPR2.IR2) * ((s64)quotient) ) ) >> 16 );
	
	// this also should get stored to SXYP
	//CPR2.SXYP = CPR2.SXY2;
	
	//[1,31,0] MAC0= F[DQB + DQA * (H/SZ)]                           [1,19,24]
	//CPR2.MAC0 = Lm_F( ( ((s64)CPC2.DQB) + ( ((s64)CPC2.DQA) * ((s64)quotient) ) ) >> 12 );
	MAC0 = F( ( ((s64)CPC2.DQB) + ( ((s64)CPC2.DQA) * ((s64)quotient) ) ) );
	
	//[1,15,0] IR0= Lm_H[MAC0]                                       [1,31,0]
	//CPR2.IR0 = Lm_H ( CPR2.MAC0 );
	CPR2.IR0 = Lm_H ( MAC0 >> 12 );

	// write back MAC (44-bit register)
	CPR2.MAC0 = (s32) MAC0;
	CPR2.MAC1 = (s32) MAC1;
	CPR2.MAC2 = (s32) MAC2;
	CPR2.MAC3 = (s32) MAC3;

#if defined INLINE_DEBUG_COP2_RTPT || defined INLINE_DEBUG_COP2_ALL
	// show output values
	debug << "\r\n" << hex << "Output(CPC): ";
	for ( k = 0; k < sizeof(OutputCPCRegs)/sizeof(OutputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ OutputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ OutputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Output(CPR): ";
	for ( k = 0; k < sizeof(OutputCPRRegs)/sizeof(OutputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ OutputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ OutputCPRRegs [ k ] ]) << ";";
	}
	
#endif

	//return true;
}

void COP2_Device::NCLIP ( Cpu* r, Instruction::Format i )
{
	//static const u32 InputCPCRegs [] = { };
								
	static const u32 InputCPRRegs [] = { INDEX_SXY0, INDEX_SZ0, INDEX_SXY1, INDEX_SZ1, INDEX_SXY2, INDEX_SZ2 };

	static const u32 OutputCPCRegs [] = { INDEX_FLAG };
	static const u32 OutputCPRRegs [] = { INDEX_MAC0 };

#if defined INLINE_DEBUG_COP2_NCLIP || defined INLINE_DEBUG_COP2_ALL || defined INLINE_DEBUG_NAME
	debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << "  " << "NCLIP";
#endif

#if defined INLINE_DEBUG_COP2_NCLIP || defined INLINE_DEBUG_COP2_ALL
	int k;
	// show input values
	debug << "\r\n" << hex << "Input(CPC): ";
	//for ( k = 0; k < sizeof(InputCPCRegs)/sizeof(InputCPCRegs[0]); k++ )
	//{
	//	debug << CPC_RegisterNames [ InputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ InputCPCRegs [ k ] ]) << ";";
	//}
	
	debug << "\r\n" << hex << "Input(CPR): ";
	for ( k = 0; k < sizeof(InputCPRRegs)/sizeof(InputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ InputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ InputCPRRegs [ k ] ]) << ";";
	}
	
#endif


	// 8 cycles
	// cop2 $1400006
	static const u32 c_InstructionCycles = 8;
	
	static const u32 InputRegs_Bitmap = BITMAP_SXY0 | BITMAP_SXY1 | BITMAP_SXY2 | BITMAP_SZ0 | BITMAP_SZ1 | BITMAP_SZ2;
	
	// input (CPC):
	// input (CPR): SX0, SX1, SX2, SY0, SY1, SY2, SZ0, SZ1, SZ2
	// output (CPC): FLAG
	// output (CPR): MAC0
	
	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;
	
	//[1,31,0] MAC0 = F[SX0*SY1+SX1*SY2+SX2*SY0-SX0*SY2-SX1*SY0-SX2*SY1] [1,43,0]
	CPR2.MAC0 = F( ( ((s64)CPR2.SX0) * ( ((s64)CPR2.SY1) - ((s64)CPR2.SY2) ) )
					+ ( ((s64)CPR2.SX1) * ( ((s64)CPR2.SY2) - ((s64)CPR2.SY0) ) )
					+ ( ((s64)CPR2.SX2) * ( ((s64)CPR2.SY0) - ((s64)CPR2.SY1) ) ) );

#if defined INLINE_DEBUG_COP2_NCLIP || defined INLINE_DEBUG_COP2_ALL
	// show output values
	debug << "\r\n" << hex << "Output(CPC): ";
	for ( k = 0; k < sizeof(OutputCPCRegs)/sizeof(OutputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ OutputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ OutputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Output(CPR): ";
	for ( k = 0; k < sizeof(OutputCPRRegs)/sizeof(OutputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ OutputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ OutputCPRRegs [ k ] ]) << ";";
	}
	
#endif


	//return true;
}

void COP2_Device::DPCS ( Cpu* r, Instruction::Format i )
{
#ifdef INLINE_DEBUG
	debug << "\r\n" << hex << setw( 8 ) << r->PC << dec << "  " << "DPCS" << "  " << dec << r->CycleCount << hex;
#endif

	// 8 cycles
	// cop2 $0780010
	static const u32 c_InstructionCycles = 8;
	
	static const u32 InputRegs_Bitmap = BITMAP_RGB | BITMAP_IR0;
	
	// input (CPC): RFC, GFC, BFC
	// input (CPR): RGB, IR0
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3, RGB0, RGB1, RGB2
	
	// *** todo *** unknown if instruction passes sf flag -> need to check further

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;
	
	// get lm
	u32 lm = i.lm;
	
	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) | ( shift << 2 );
	
	// *** note *** a typo in gte.txt has been corrected below with B1,B1,B1 vs B1,B2,B3

	/*
	// [1,27,4]  MAC1=A1[(R + IR0*(Lm_B1[RFC - R])]                   [1,27,16][lm=0]
	CPR2.MAC1 = A1( ( ( ((u64)CPR2.R) << 16 ) + ( ((s64)CPR2.IR0) * Lm_B1( ((s64)CPC2.RFC) - ( ((u64)CPR2.R) << 4 ), 0 ) ) ) >> shift );
	
	// [1,27,4]  MAC2=A2[(G + IR0*(Lm_B1[GFC - G])]                   [1,27,16][lm=0]
	CPR2.MAC2 = A2( ( ( ((u64)CPR2.G) << 16 ) + ( ((s64)CPR2.IR0) * Lm_B2( ((s64)CPC2.GFC) - ( ((u64)CPR2.G) << 4 ), 0 ) ) ) >> shift );
	
	// [1,27,4]  MAC3=A3[(B + IR0*(Lm_B1[BFC - B])]                   [1,27,16][lm=0]
	CPR2.MAC3 = A3( ( ( ((u64)CPR2.B) << 16 ) + ( ((s64)CPR2.IR0) * Lm_B3( ((s64)CPC2.BFC) - ( ((u64)CPR2.B) << 4 ), 0 ) ) ) >> shift );
	*/
	
	// [MAC1,MAC2,MAC3] = [R,G,B] SHL 16                     ;<--- for DPCS/DPCT
	CPR2.MAC1 = A1( ((u64)CPR2.R) << 16 );
	CPR2.MAC2 = A2( ((u64)CPR2.G) << 16 );
	CPR2.MAC3 = A3( ((u64)CPR2.B) << 16 );
	
	// [MAC1,MAC2,MAC3] = MAC+(FC-MAC)*IR0
	// [IR1,IR2,IR3] = (([RFC,GFC,BFC] SHL 12) - [MAC1,MAC2,MAC3]) SAR (sf*12)
	CPR2.IR1 = Lm_B1( ((s64) ( ( ((s64)CPC2.RFC) << 12 ) - ((s64)CPR2.MAC1) )) >> shift, 0 );
	CPR2.IR2 = Lm_B2( ((s64) ( ( ((s64)CPC2.GFC) << 12 ) - ((s64)CPR2.MAC2) )) >> shift, 0 );
	CPR2.IR3 = Lm_B3( ((s64) ( ( ((s64)CPC2.BFC) << 12 ) - ((s64)CPR2.MAC3) )) >> shift, 0 );
	
	// [MAC1,MAC2,MAC3] = (([IR1,IR2,IR3] * IR0) + [MAC1,MAC2,MAC3])
	CPR2.MAC1 = A1( ( ( ((s64)CPR2.IR1) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC1) ) );
	CPR2.MAC2 = A2( ( ( ((s64)CPR2.IR2) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC2) ) );
	CPR2.MAC3 = A3( ( ( ((s64)CPR2.IR3) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC3) ) );
	
	// [MAC1,MAC2,MAC3] = [MAC1,MAC2,MAC3] SAR (sf*12)
	CPR2.MAC1 = CPR2.MAC1 >> shift;
	CPR2.MAC2 = CPR2.MAC2 >> shift;
	CPR2.MAC3 = CPR2.MAC3 >> shift;
	
	// [1,11,4]  IR1=Lm_B1[MAC1]                                      [1,27,4][lm=0]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, lm );
	
	// [1,11,4]  IR2=Lm_B2[MAC2]                                      [1,27,4][lm=0]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, lm );
	
	// [1,11,4]  IR3=Lm_B3[MAC3]                                      [1,27,4][lm=0]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, lm);
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );
	
	//return true;
}

void COP2_Device::DPCT ( Cpu* r, Instruction::Format i )
{
#ifdef INLINE_DEBUG
	debug << "\r\n" << hex << setw( 8 ) << r->PC << dec << "  " << "DPCT";
#endif

	// 17 cycles
	// cop2 $0F8002A
	static const u32 c_InstructionCycles = 17;
	
	static const u32 InputRegs_Bitmap = BITMAP_RGB | BITMAP_IR0;
	
	// input (CPC): RFC, GFC, BFC
	// input (CPR): RGB, IR0
	// output (CPC):
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3, RGB0, RGB1, RGB2

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;
	
	// get lm
	u32 lm = i.lm;
	
	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) | ( shift << 2 );
	
	for ( int i = 0; i < 3; i++ )
	{
		/*
		// [1,27,4]  MAC1=A1[(R + IR0*(Lm_B1[RFC - R])]                   [1,27,16][lm=0]
		CPR2.MAC1 = A1( ( ( ((u32)CPR2.R) << 16 ) + ( CPR2.IR0 * Lm_B1( CPC2.RFC - ( CPR2.R << 4 ), 0 ) ) ) >> 12 );
		
		// [1,27,4]  MAC2=A2[(G + IR0*(Lm_B1[GFC - G])]                   [1,27,16][lm=0]
		CPR2.MAC2 = A2( ( ( ((u32)CPR2.G) << 16 ) + ( CPR2.IR0 * Lm_B2( CPC2.GFC - ( CPR2.G << 4 ), 0 ) ) ) >> 12 );
		
		// [1,27,4]  MAC3=A3[(B + IR0*(Lm_B1[BFC - B])]                   [1,27,16][lm=0]
		CPR2.MAC3 = A3( ( ( ((u32)CPR2.B) << 16 ) + ( CPR2.IR0 * Lm_B3( CPC2.BFC - ( CPR2.B << 4 ), 0 ) ) ) >> 12 );
		*/
		
		// [MAC1,MAC2,MAC3] = [R,G,B] SHL 16                     ;<--- for DPCS/DPCT
		CPR2.MAC1 = A1( ((u64)CPR2.R0) << 16 );
		CPR2.MAC2 = A2( ((u64)CPR2.G0) << 16 );
		CPR2.MAC3 = A3( ((u64)CPR2.B0) << 16 );
		
		// [MAC1,MAC2,MAC3] = MAC+(FC-MAC)*IR0
		// [IR1,IR2,IR3] = (([RFC,GFC,BFC] SHL 12) - [MAC1,MAC2,MAC3]) SAR (sf*12)
		CPR2.IR1 = Lm_B1( ((s64) ( ( ((s64)CPC2.RFC) << 12 ) - ((s64)CPR2.MAC1) )) >> shift, 0 );
		CPR2.IR2 = Lm_B2( ((s64) ( ( ((s64)CPC2.GFC) << 12 ) - ((s64)CPR2.MAC2) )) >> shift, 0 );
		CPR2.IR3 = Lm_B3( ((s64) ( ( ((s64)CPC2.BFC) << 12 ) - ((s64)CPR2.MAC3) )) >> shift, 0 );
		
		// [MAC1,MAC2,MAC3] = (([IR1,IR2,IR3] * IR0) + [MAC1,MAC2,MAC3])
		// [MAC1,MAC2,MAC3] = [MAC1,MAC2,MAC3] SAR (sf*12)
		CPR2.MAC1 = A1( ( ( ((s64)CPR2.IR1) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC1) ) >> shift );
		CPR2.MAC2 = A2( ( ( ((s64)CPR2.IR2) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC2) ) >> shift );
		CPR2.MAC3 = A3( ( ( ((s64)CPR2.IR3) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC3) ) >> shift );
		
		// [1,11,4]  IR1=Lm_B1[MAC1]                                      [1,27,4][lm=0]
		CPR2.IR1 = Lm_B1( CPR2.MAC1, 0 );
		
		// [1,11,4]  IR2=Lm_B2[MAC2]                                      [1,27,4][lm=0]
		CPR2.IR2 = Lm_B2( CPR2.MAC2, 0 );
		
		// [1,11,4]  IR3=Lm_B3[MAC3]                                      [1,27,4][lm=0]
		CPR2.IR3 = Lm_B3( CPR2.MAC3, 0 );
		
		// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
		// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
		// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
		// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
		CPR2.RGB0 = CPR2.RGB1;
		CPR2.RGB1 = CPR2.RGB2;
		CPR2.Cd2 = CPR2.CODE;
		CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
		CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
		CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );
	}
	
	//return true;
}

void COP2_Device::INTPL ( Cpu* r, Instruction::Format i )
{
#ifdef INLINE_DEBUG
	debug << "\r\n" << hex << setw( 8 ) << r->PC << dec << "  " << "INTPL";
#endif

	// 8 cycles
	// cop2 $0980011
	static const u32 c_InstructionCycles = 8;
	
	static const u32 InputRegs_Bitmap = BITMAP_IR0 | BITMAP_IR1 | BITMAP_IR2 | BITMAP_IR3;
	
	// input (CPC): RFC, GFC, BFC
	// input (CPR): IR0, IR1, IR2, IR3
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3, RGB0, RGB1, RGB2
	
	// *** todo *** unsure if this is using sf and lm

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	
	// get lm
	u32 lm = i.lm;

	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );

	/*
	// [1,27,4]  MAC1=A1[IR1 + IR0*(Lm_B1[RFC - IR1])]                [1,27,16]
	CPR2.MAC1 = A1( ( ( ((s64)CPR2.IR1) << 12 ) + ((s64)CPR2.IR0) * Lm_B1( ((s64)CPC2.RFC) - ((s64)CPR2.IR1), 0 ) ) >> shift );
	
	// [1,27,4]  MAC2=A2[IR2 + IR0*(Lm_B1[GFC - IR2])]                [1,27,16]
	CPR2.MAC2 = A2( ( ( ((s64)CPR2.IR2) << 12 ) + ((s64)CPR2.IR0) * Lm_B2( ((s64)CPC2.GFC) - ((s64)CPR2.IR2), 0 ) ) >> shift );
	
	// [1,27,4]  MAC3=A3[IR3 + IR0*(Lm_B1[BFC - IR3])]                [1,27,16]
	CPR2.MAC3 = A3( ( ( ((s64)CPR2.IR3) << 12 ) + ((s64)CPR2.IR0) * Lm_B3( ((s64)CPC2.BFC) - ((s64)CPR2.IR3), 0 ) ) >> shift );
	*/
	
	// [MAC1,MAC2,MAC3] = [IR1,IR2,IR3] SHL 12               ;<--- for INTPL only
	CPR2.MAC1 = A1( ((s64)CPR2.IR1) << 12 );
	CPR2.MAC2 = A2( ((s64)CPR2.IR2) << 12 );
	CPR2.MAC3 = A3( ((s64)CPR2.IR3) << 12 );
	
	// [MAC1,MAC2,MAC3] = MAC+(FC-MAC)*IR0
	// [IR1,IR2,IR3] = (([RFC,GFC,BFC] SHL 12) - [MAC1,MAC2,MAC3]) SAR (sf*12)
	CPR2.IR1 = Lm_B1( ((s64) ( ( ((s64)CPC2.RFC) << 12 ) - ((s64)CPR2.MAC1) )) >> shift, 0 );
	CPR2.IR2 = Lm_B2( ((s64) ( ( ((s64)CPC2.GFC) << 12 ) - ((s64)CPR2.MAC2) )) >> shift, 0 );
	CPR2.IR3 = Lm_B3( ((s64) ( ( ((s64)CPC2.BFC) << 12 ) - ((s64)CPR2.MAC3) )) >> shift, 0 );
	
	// [MAC1,MAC2,MAC3] = (([IR1,IR2,IR3] * IR0) + [MAC1,MAC2,MAC3])
	CPR2.MAC1 = A1( ( ( ((s64)CPR2.IR1) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC1) ) );
	CPR2.MAC2 = A2( ( ( ((s64)CPR2.IR2) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC2) ) );
	CPR2.MAC3 = A3( ( ( ((s64)CPR2.IR3) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC3) ) );

	// [MAC1,MAC2,MAC3] = [MAC1,MAC2,MAC3] SAR (sf*12)
	CPR2.MAC1 = CPR2.MAC1 >> shift;
	CPR2.MAC2 = CPR2.MAC2 >> shift;
	CPR2.MAC3 = CPR2.MAC3 >> shift;
	
	// [1,11,4]  IR1=Lm_B1[MAC1]                                      [1,27,4]
	CPR2.IR1 = Lm_B1 ( CPR2.MAC1, lm );
	
	// [1,11,4]  IR2=Lm_B2[MAC2]                                      [1,27,4]
	CPR2.IR2 = Lm_B2 ( CPR2.MAC2, lm );
	
	// [1,11,4]  IR3=Lm_B3[MAC3]                                      [1,27,4]
	CPR2.IR3 = Lm_B3 ( CPR2.MAC3, lm );
	

	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );
	
	//return true;
}


void COP2_Device::NCDS ( Cpu* r, Instruction::Format i )
{
	static const u32 InputCPCRegs [] = { INDEX_L11L12, INDEX_L13L21, INDEX_L22L23, INDEX_L31L32, INDEX_ZeroL33, INDEX_LR1LR2, INDEX_LR3LG1,
										INDEX_LG2LG3, INDEX_LB1LB2, INDEX_ZeroLB3, INDEX_RBK, INDEX_GBK, INDEX_BBK, INDEX_RFC, INDEX_GFC, INDEX_BFC };
								
	static const u32 InputCPRRegs [] = { INDEX_IR0, INDEX_VXY0, INDEX_ZeroVZ0, INDEX_RGB };

	static const u32 OutputCPCRegs [] = { INDEX_FLAG };
	static const u32 OutputCPRRegs [] = { INDEX_MAC1, INDEX_MAC2, INDEX_MAC3, INDEX_IR1, INDEX_IR2, INDEX_IR3, INDEX_RGB0, INDEX_RGB1, INDEX_RGB2 };

	// 19 cycles
	// cop2 $0e80413
	static const u32 c_InstructionCycles = 19;
	
	static const u32 InputRegs_Bitmap = BITMAP_VXY0 | BITMAP_ZeroVZ0 | BITMAP_RGB;
	
	// input (CPC): L11, L12, L13, L21, L22, L23, L31, L32, L33, LR1, LR2, LR3, LG1, LG2, LG3, LB1, LB2, LB3, RBK, GBK, BBK, RFC, GFC, BFC
	// input (CPR): VX0, VY0, VZ0, RGB, IR0
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3, RGB0, RGB1, RGB2



#if defined INLINE_DEBUG_COP2_NCDS || defined INLINE_DEBUG_COP2_ALL || defined INLINE_DEBUG_NAME
	debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << "  " << "NCDS";
#endif
	
#if defined INLINE_DEBUG_COP2_NCDS || defined INLINE_DEBUG_COP2_ALL
	int k;
	debug << "\r\n" << hex << "Input(CPC): ";
	debug << " L11L12=" << CPC2.L11L12 << " L13L21=" << CPC2.L13L21 << " L22L23=" << CPC2.L22L23 << " L31L32=" << CPC2.L31L32 << " ZeroL33=" << CPC2.ZeroL33;
	debug << " LR1LR2=" << CPC2.LR1LR2 << " LR3LG1=" << CPC2.LR3LG1 << " LG2LG3=" << CPC2.LG2LG3 << " LB1LB2=" << CPC2.LB1LB2 << " ZeroLB3=" << CPC2.ZeroLB3;
	debug << " RBK=" << CPC2.RBK << " GBK=" << CPC2.GBK << " BBK=" << CPC2.BBK << " RFC=" << CPC2.RFC << " GFC=" << CPC2.GFC << " BFC=" << CPC2.BFC;
	
	debug << "\r\n" << hex << "Input(CPR): ";
	debug << " IR0=" << CPR2.IR0 << " VXY0=" << CPR2.VXY0 << " ZeroVZ0=" << CPR2.ZeroVZ0 << " RGB=" << CPR2.RGB;

	/*
	// show input values
	debug << "\r\n" << hex << "Input(CPC): ";
	for ( k = 0; k < sizeof(InputCPCRegs)/sizeof(InputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ InputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ InputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Input(CPR): ";
	for ( k = 0; k < sizeof(InputCPRRegs)/sizeof(InputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ InputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ InputCPRRegs [ k ] ]) << ";";
	}
	*/
	
#endif


	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	// get lm
	u32 lm = i.lm;

	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	// [1,19,12] MAC1=A1[L11*VX0 + L12*VY0 + L13*VZ0]                 [1,19,24]
	CPR2.MAC1 = A1( ( (s64)CPC2.L11 * (s64)CPR2.VX0 + (s64)CPC2.L12 * (s64)CPR2.VY0 + (s64)CPC2.L13 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,19,12] MAC2=A1[L21*VX0 + L22*VY0 + L23*VZ0]                 [1,19,24]
	CPR2.MAC2 = A2( ( (s64)CPC2.L21 * (s64)CPR2.VX0 + (s64)CPC2.L22 * (s64)CPR2.VY0 + (s64)CPC2.L23 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,19,12] MAC3=A1[L31*VX0 + L32*VY0 + L33*VZ0]                 [1,19,24]
	CPR2.MAC3 = A3( ( (s64)CPC2.L31 * (s64)CPR2.VX0 + (s64)CPC2.L32 * (s64)CPR2.VY0 + (s64)CPC2.L33 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( (s64)CPC2.RBK << 12 ) + (s64)CPC2.LR1 * (s64)CPR2.IR1 + (s64)CPC2.LR2 * (s64)CPR2.IR2 + (s64)CPC2.LR3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A1[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( (s64)CPC2.GBK << 12 ) + (s64)CPC2.LG1 * (s64)CPR2.IR1 + (s64)CPC2.LG2 * (s64)CPR2.IR2 + (s64)CPC2.LG3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A1[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( (s64)CPC2.BBK << 12 ) + (s64)CPC2.LB1 * (s64)CPR2.IR1 + (s64)CPC2.LB2 * (s64)CPR2.IR2 + (s64)CPC2.LB3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,27,4]  MAC1=A1[R*IR1]                                        [1,27,16]
	CPR2.MAC1 = A1( ( ( ((u64)CPR2.R) << 4 ) * ((s64)CPR2.IR1) ) );
	
	// [1,27,4]  MAC2=A2[G*IR2]                                        [1,27,16]
	CPR2.MAC2 = A2( ( ( ((u64)CPR2.G) << 4 ) * ((s64)CPR2.IR2) ) );
	
	// [1,27,4]  MAC3=A3[B*IR3]                                        [1,27,16]
	CPR2.MAC3 = A3( ( ( ((u64)CPR2.B) << 4 ) * ((s64)CPR2.IR3) ) );
	
	// [MAC1,MAC2,MAC3] = MAC+(FC-MAC)*IR0                   ;<--- for NCDx only
	// [IR1,IR2,IR3] = (([RFC,GFC,BFC] SHL 12) - [MAC1,MAC2,MAC3]) SAR (sf*12)
	CPR2.IR1 = Lm_B1( ((s64) ( ( ((s64)CPC2.RFC) << 12 ) - ((s64)CPR2.MAC1) )) >> shift, 0 );
	CPR2.IR2 = Lm_B2( ((s64) ( ( ((s64)CPC2.GFC) << 12 ) - ((s64)CPR2.MAC2) )) >> shift, 0 );
	CPR2.IR3 = Lm_B3( ((s64) ( ( ((s64)CPC2.BFC) << 12 ) - ((s64)CPR2.MAC3) )) >> shift, 0 );
	
	// [MAC1,MAC2,MAC3] = (([IR1,IR2,IR3] * IR0) + [MAC1,MAC2,MAC3])
	// [MAC1,MAC2,MAC3] = [MAC1,MAC2,MAC3] SAR (sf*12)
	CPR2.MAC1 = A1( ( ( ((s64)CPR2.IR1) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC1) ) >> shift );
	CPR2.MAC2 = A2( ( ( ((s64)CPR2.IR2) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC2) ) >> shift );
	CPR2.MAC3 = A3( ( ( ((s64)CPR2.IR3) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC3) ) >> shift );

	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,27,4][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,27,4][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,27,4][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );


#if defined INLINE_DEBUG_COP2_NCDS || defined INLINE_DEBUG_COP2_ALL

	debug << "\r\n" << hex << "Output(CPC): ";
	debug << " FLAG=" << CPC2.FLAG.Value;
	debug << "\r\n" << hex << "Output(CPR): ";
	debug << " MAC1=" << CPR2.MAC1 << " MAC2=" << CPR2.MAC2 << " MAC3=" << CPR2.MAC3 << " IR1=" << CPR2.IR1 << " IR2=" << CPR2.IR2 << " IR3=" << CPR2.IR3 << " RGB0=" << CPR2.RGB0 << " RGB1=" << CPR2.RGB1 << " RGB2=" << CPR2.RGB2;

	/*
	// show output values
	debug << "\r\n" << hex << "Output(CPC): ";
	for ( k = 0; k < sizeof(OutputCPCRegs)/sizeof(OutputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ OutputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ OutputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Output(CPR): ";
	for ( k = 0; k < sizeof(OutputCPRRegs)/sizeof(OutputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ OutputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ OutputCPRRegs [ k ] ]) << ";";
	}
	*/
	
#endif

	
	//return true;
}

void COP2_Device::NCDT ( Cpu* r, Instruction::Format i )
{
#ifdef INLINE_DEBUG
	debug << "\r\n" << hex << setw( 8 ) << r->PC << dec << "  " << "NCDT";
#endif

	// 44 cycles
	// cop2 $0f80416
	static const u32 c_InstructionCycles = 44;

	static const u32 InputRegs_Bitmap = BITMAP_VXY0 | BITMAP_ZeroVZ0 |
										BITMAP_VXY1 | BITMAP_ZeroVZ1 |
										BITMAP_VXY2 | BITMAP_ZeroVZ2 |
										BITMAP_RGB;
	
	// input (CPC): L11, L12, L13, L21, L22, L23, L31, L32, L33, LR1, LR2, LR3, LG1, LG2, LG3, LB1, LB2, LB3, RBK, GBK, BBK, RFC, GFC, BFC
	// input (CPR): VX0, VY0, VZ0, VX1, VY1, VZ1, VX2, VY2, VZ2, RGB
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3, RGB0, RGB1, RGB2
	
	// same as NCDS just repeat for v1 and v2

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	// get lm
	u32 lm = i.lm;

	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	////////////
	// for V0 //
	////////////
	
	// [1,19,12] MAC1=A1[L11*VX0 + L12*VY0 + L13*VZ0]                 [1,19,24]
	CPR2.MAC1 = A1( ( (s64)CPC2.L11 * (s64)CPR2.VX0 + (s64)CPC2.L12 * (s64)CPR2.VY0 + (s64)CPC2.L13 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,19,12] MAC2=A1[L21*VX0 + L22*VY0 + L23*VZ0]                 [1,19,24]
	CPR2.MAC2 = A2( ( (s64)CPC2.L21 * (s64)CPR2.VX0 + (s64)CPC2.L22 * (s64)CPR2.VY0 + (s64)CPC2.L23 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,19,12] MAC3=A1[L31*VX0 + L32*VY0 + L33*VZ0]                 [1,19,24]
	CPR2.MAC3 = A3( ( (s64)CPC2.L31 * (s64)CPR2.VX0 + (s64)CPC2.L32 * (s64)CPR2.VY0 + (s64)CPC2.L33 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( (s64)CPC2.RBK << 12 ) + (s64)CPC2.LR1 * (s64)CPR2.IR1 + (s64)CPC2.LR2 * (s64)CPR2.IR2 + (s64)CPC2.LR3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A1[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( (s64)CPC2.GBK << 12 ) + (s64)CPC2.LG1 * (s64)CPR2.IR1 + (s64)CPC2.LG2 * (s64)CPR2.IR2 + (s64)CPC2.LG3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A1[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( (s64)CPC2.BBK << 12 ) + (s64)CPC2.LB1 * (s64)CPR2.IR1 + (s64)CPC2.LB2 * (s64)CPR2.IR2 + (s64)CPC2.LB3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,27,4]  MAC1=A1[R*IR1]                                        [1,27,16]
	CPR2.MAC1 = A1( ( ( ((u64)CPR2.R) << 4 ) * ((s64)CPR2.IR1) ) );
	
	// [1,27,4]  MAC2=A2[G*IR2]                                        [1,27,16]
	CPR2.MAC2 = A2( ( ( ((u64)CPR2.G) << 4 ) * ((s64)CPR2.IR2) ) );
	
	// [1,27,4]  MAC3=A3[B*IR3]                                        [1,27,16]
	CPR2.MAC3 = A3( ( ( ((u64)CPR2.B) << 4 ) * ((s64)CPR2.IR3) ) );
	
	// [MAC1,MAC2,MAC3] = MAC+(FC-MAC)*IR0                   ;<--- for NCDx only
	// [IR1,IR2,IR3] = (([RFC,GFC,BFC] SHL 12) - [MAC1,MAC2,MAC3]) SAR (sf*12)
	CPR2.IR1 = Lm_B1( ((s64) ( ( ((s64)CPC2.RFC) << 12 ) - ((s64)CPR2.MAC1) )) >> shift, 0 );
	CPR2.IR2 = Lm_B2( ((s64) ( ( ((s64)CPC2.GFC) << 12 ) - ((s64)CPR2.MAC2) )) >> shift, 0 );
	CPR2.IR3 = Lm_B3( ((s64) ( ( ((s64)CPC2.BFC) << 12 ) - ((s64)CPR2.MAC3) )) >> shift, 0 );
	
	// [MAC1,MAC2,MAC3] = (([IR1,IR2,IR3] * IR0) + [MAC1,MAC2,MAC3])
	// [MAC1,MAC2,MAC3] = [MAC1,MAC2,MAC3] SAR (sf*12)
	CPR2.MAC1 = A1( ( ( ((s64)CPR2.IR1) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC1) ) >> shift );
	CPR2.MAC2 = A2( ( ( ((s64)CPR2.IR2) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC2) ) >> shift );
	CPR2.MAC3 = A3( ( ( ((s64)CPR2.IR3) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC3) ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,27,4][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,27,4][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,27,4][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );

	////////////
	// for V1 //
	////////////
	
	// [1,19,12] MAC1=A1[L11*VX0 + L12*VY0 + L13*VZ0]                 [1,19,24]
	CPR2.MAC1 = A1( ( (s64)CPC2.L11 * (s64)CPR2.VX1 + (s64)CPC2.L12 * (s64)CPR2.VY1 + (s64)CPC2.L13 * (s64)CPR2.VZ1 ) >> shift );
	
	// [1,19,12] MAC2=A1[L21*VX0 + L22*VY0 + L23*VZ0]                 [1,19,24]
	CPR2.MAC2 = A2( ( (s64)CPC2.L21 * (s64)CPR2.VX1 + (s64)CPC2.L22 * (s64)CPR2.VY1 + (s64)CPC2.L23 * (s64)CPR2.VZ1 ) >> shift );
	
	// [1,19,12] MAC3=A1[L31*VX0 + L32*VY0 + L33*VZ0]                 [1,19,24]
	CPR2.MAC3 = A3( ( (s64)CPC2.L31 * (s64)CPR2.VX1 + (s64)CPC2.L32 * (s64)CPR2.VY1 + (s64)CPC2.L33 * (s64)CPR2.VZ1 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( (s64)CPC2.RBK << 12 ) + (s64)CPC2.LR1 * (s64)CPR2.IR1 + (s64)CPC2.LR2 * (s64)CPR2.IR2 + (s64)CPC2.LR3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A1[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( (s64)CPC2.GBK << 12 ) + (s64)CPC2.LG1 * (s64)CPR2.IR1 + (s64)CPC2.LG2 * (s64)CPR2.IR2 + (s64)CPC2.LG3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A1[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( (s64)CPC2.BBK << 12 ) + (s64)CPC2.LB1 * (s64)CPR2.IR1 + (s64)CPC2.LB2 * (s64)CPR2.IR2 + (s64)CPC2.LB3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,27,4]  MAC1=A1[R*IR1]                                        [1,27,16]
	CPR2.MAC1 = A1( ( ( ((u64)CPR2.R) << 4 ) * ((s64)CPR2.IR1) ) );
	
	// [1,27,4]  MAC2=A2[G*IR2]                                        [1,27,16]
	CPR2.MAC2 = A2( ( ( ((u64)CPR2.G) << 4 ) * ((s64)CPR2.IR2) ) );
	
	// [1,27,4]  MAC3=A3[B*IR3]                                        [1,27,16]
	CPR2.MAC3 = A3( ( ( ((u64)CPR2.B) << 4 ) * ((s64)CPR2.IR3) ) );
	
	// [MAC1,MAC2,MAC3] = MAC+(FC-MAC)*IR0                   ;<--- for NCDx only
	// [IR1,IR2,IR3] = (([RFC,GFC,BFC] SHL 12) - [MAC1,MAC2,MAC3]) SAR (sf*12)
	CPR2.IR1 = Lm_B1( ((s64) ( ( ((s64)CPC2.RFC) << 12 ) - ((s64)CPR2.MAC1) )) >> shift, 0 );
	CPR2.IR2 = Lm_B2( ((s64) ( ( ((s64)CPC2.GFC) << 12 ) - ((s64)CPR2.MAC2) )) >> shift, 0 );
	CPR2.IR3 = Lm_B3( ((s64) ( ( ((s64)CPC2.BFC) << 12 ) - ((s64)CPR2.MAC3) )) >> shift, 0 );
	
	// [MAC1,MAC2,MAC3] = (([IR1,IR2,IR3] * IR0) + [MAC1,MAC2,MAC3])
	// [MAC1,MAC2,MAC3] = [MAC1,MAC2,MAC3] SAR (sf*12)
	CPR2.MAC1 = A1( ( ( ((s64)CPR2.IR1) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC1) ) >> shift );
	CPR2.MAC2 = A2( ( ( ((s64)CPR2.IR2) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC2) ) >> shift );
	CPR2.MAC3 = A3( ( ( ((s64)CPR2.IR3) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC3) ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,27,4][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,27,4][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,27,4][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );

	////////////
	// for V2 //
	////////////
	
	// [1,19,12] MAC1=A1[L11*VX0 + L12*VY0 + L13*VZ0]                 [1,19,24]
	CPR2.MAC1 = A1( ( (s64)CPC2.L11 * (s64)CPR2.VX2 + (s64)CPC2.L12 * (s64)CPR2.VY2 + (s64)CPC2.L13 * (s64)CPR2.VZ2 ) >> shift );
	
	// [1,19,12] MAC2=A1[L21*VX0 + L22*VY0 + L23*VZ0]                 [1,19,24]
	CPR2.MAC2 = A2( ( (s64)CPC2.L21 * (s64)CPR2.VX2 + (s64)CPC2.L22 * (s64)CPR2.VY2 + (s64)CPC2.L23 * (s64)CPR2.VZ2 ) >> shift );
	
	// [1,19,12] MAC3=A1[L31*VX0 + L32*VY0 + L33*VZ0]                 [1,19,24]
	CPR2.MAC3 = A3( ( (s64)CPC2.L31 * (s64)CPR2.VX2 + (s64)CPC2.L32 * (s64)CPR2.VY2 + (s64)CPC2.L33 * (s64)CPR2.VZ2 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( (s64)CPC2.RBK << 12 ) + (s64)CPC2.LR1 * (s64)CPR2.IR1 + (s64)CPC2.LR2 * (s64)CPR2.IR2 + (s64)CPC2.LR3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A1[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( (s64)CPC2.GBK << 12 ) + (s64)CPC2.LG1 * (s64)CPR2.IR1 + (s64)CPC2.LG2 * (s64)CPR2.IR2 + (s64)CPC2.LG3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A1[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( (s64)CPC2.BBK << 12 ) + (s64)CPC2.LB1 * (s64)CPR2.IR1 + (s64)CPC2.LB2 * (s64)CPR2.IR2 + (s64)CPC2.LB3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,27,4]  MAC1=A1[R*IR1]                                        [1,27,16]
	CPR2.MAC1 = A1( ( ( ((u64)CPR2.R) << 4 ) * ((s64)CPR2.IR1) ) );
	
	// [1,27,4]  MAC2=A2[G*IR2]                                        [1,27,16]
	CPR2.MAC2 = A2( ( ( ((u64)CPR2.G) << 4 ) * ((s64)CPR2.IR2) ) );
	
	// [1,27,4]  MAC3=A3[B*IR3]                                        [1,27,16]
	CPR2.MAC3 = A3( ( ( ((u64)CPR2.B) << 4 ) * ((s64)CPR2.IR3) ) );
	
	// [MAC1,MAC2,MAC3] = MAC+(FC-MAC)*IR0                   ;<--- for NCDx only
	// [IR1,IR2,IR3] = (([RFC,GFC,BFC] SHL 12) - [MAC1,MAC2,MAC3]) SAR (sf*12)
	CPR2.IR1 = Lm_B1( ((s64) ( ( ((s64)CPC2.RFC) << 12 ) - ((s64)CPR2.MAC1) )) >> shift, 0 );
	CPR2.IR2 = Lm_B2( ((s64) ( ( ((s64)CPC2.GFC) << 12 ) - ((s64)CPR2.MAC2) )) >> shift, 0 );
	CPR2.IR3 = Lm_B3( ((s64) ( ( ((s64)CPC2.BFC) << 12 ) - ((s64)CPR2.MAC3) )) >> shift, 0 );
	
	// [MAC1,MAC2,MAC3] = (([IR1,IR2,IR3] * IR0) + [MAC1,MAC2,MAC3])
	// [MAC1,MAC2,MAC3] = [MAC1,MAC2,MAC3] SAR (sf*12)
	CPR2.MAC1 = A1( ( ( ((s64)CPR2.IR1) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC1) ) >> shift );
	CPR2.MAC2 = A2( ( ( ((s64)CPR2.IR2) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC2) ) >> shift );
	CPR2.MAC3 = A3( ( ( ((s64)CPR2.IR3) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC3) ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,27,4][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,27,4][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,27,4][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );

	//return true;
}

void COP2_Device::CDP ( Cpu* r, Instruction::Format i )
{
#ifdef INLINE_DEBUG
	debug << "\r\n" << hex << setw( 8 ) << r->PC << dec << "  " << "CDP";
#endif

	// 13 cycles
	// cop2 $1280414
	static const u32 c_InstructionCycles = 13;
	
	static const u32 InputRegs_Bitmap = BITMAP_IR0 | BITMAP_IR1 | BITMAP_IR2 | BITMAP_IR3 | BITMAP_RGB;
	
	// input (CPC): RBK, GBK, BBK, RFC, GFC, BFC, LR1, LG1, LB1, LR2, LG2, LB2, LR3, LG3, LB3
	// input (CPR): IR0, IR1, IR2, IR3, RGB
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3, RGB0, RGB1, RGB2

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	// get lm
	u32 lm = i.lm;

	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( (s64)CPC2.RBK << 12 ) + (s64)CPC2.LR1 * (s64)CPR2.IR1 + (s64)CPC2.LR2 * (s64)CPR2.IR2 + (s64)CPC2.LR3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A2[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( (s64)CPC2.GBK << 12 ) + (s64)CPC2.LG1 * (s64)CPR2.IR1 + (s64)CPC2.LG2 * (s64)CPR2.IR2 + (s64)CPC2.LG3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A3[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( (s64)CPC2.BBK << 12 ) + (s64)CPC2.LB1 * (s64)CPR2.IR1 + (s64)CPC2.LB2 * (s64)CPR2.IR2 + (s64)CPC2.LB3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,27,4]  MAC1=A1[R*IR1]                                        [1,27,16]
	CPR2.MAC1 = A1( ( ( (u64)CPR2.R << 4 ) * ((s64)CPR2.IR1) ) );
	
	// [1,27,4]  MAC2=A2[G*IR2]                                        [1,27,16]
	CPR2.MAC2 = A2( ( ( (u64)CPR2.G << 4 ) * ((s64)CPR2.IR2 )) );
	
	// [1,27,4]  MAC3=A3[B*IR3]                                        [1,27,16]
	CPR2.MAC3 = A3( ( ( (u64)CPR2.B << 4 ) * ((s64)CPR2.IR3) ) );
	
	// [MAC1,MAC2,MAC3] = MAC+(FC-MAC)*IR0                   ;<--- for CDP only
	// [MAC1,MAC2,MAC3] = MAC+(FC-MAC)*IR0
	// [IR1,IR2,IR3] = (([RFC,GFC,BFC] SHL 12) - [MAC1,MAC2,MAC3]) SAR (sf*12)
	CPR2.IR1 = Lm_B1( ((s64) ( ( ((s64)CPC2.RFC) << 12 ) - ((s64)CPR2.MAC1) )) >> shift, 0 );
	CPR2.IR2 = Lm_B2( ((s64) ( ( ((s64)CPC2.GFC) << 12 ) - ((s64)CPR2.MAC2) )) >> shift, 0 );
	CPR2.IR3 = Lm_B3( ((s64) ( ( ((s64)CPC2.BFC) << 12 ) - ((s64)CPR2.MAC3) )) >> shift, 0 );
	
	// [MAC1,MAC2,MAC3] = (([IR1,IR2,IR3] * IR0) + [MAC1,MAC2,MAC3])
	// [MAC1,MAC2,MAC3] = [MAC1,MAC2,MAC3] SAR (sf*12)
	CPR2.MAC1 = A1( ( ( ((s64)CPR2.IR1) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC1) ) >> shift );
	CPR2.MAC2 = A2( ( ( ((s64)CPR2.IR2) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC2) ) >> shift );
	CPR2.MAC3 = A3( ( ( ((s64)CPR2.IR3) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC3) ) >> shift );

	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,27,4][lm=1]
	// left side must be using [1,11,4] format here
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,27,4][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,27,4][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );
	
	//return true;
}


void COP2_Device::NCCS ( Cpu* r, Instruction::Format i )
{
#ifdef INLINE_DEBUG
	debug << "\r\n" << hex << setw( 8 ) << r->PC << dec << "  " << "NCCS";
#endif

	// 17 cycles
	// cop2 $108041B
	static const u32 c_InstructionCycles = 17;
	
	static const u32 InputRegs_Bitmap = BITMAP_VXY0 | BITMAP_ZeroVZ0 | BITMAP_RGB;
	
	// input (CPC): L11, L12, L13, L21, L22, L23, L31, L32, L33, LR1, LR2, LR3, LG1, LG2, LG3, LB1, LB2, LB3, RBK, GBK, BBK
	// input (CPR): VX0, VY0, VZ0, RGB
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3, RGB0, RGB1, RGB2

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	// get lm
	u32 lm = i.lm;

	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	// [1,19,12] MAC1=A1[L11*VX0 + L12*VY0 + L13*VZ0]                 [1,19,24]
	CPR2.MAC1 = A1( ( CPC2.L11 * CPR2.VX0 + CPC2.L12 * CPR2.VY0 + CPC2.L13 * CPR2.VZ0 ) >> shift );
	
	// [1,19,12] MAC2=A1[L21*VX0 + L22*VY0 + L23*VZ0]                 [1,19,24]
	CPR2.MAC2 = A2( ( CPC2.L21 * CPR2.VX0 + CPC2.L22 * CPR2.VY0 + CPC2.L23 * CPR2.VZ0 ) >> shift );
	
	// [1,19,12] MAC3=A1[L31*VX0 + L32*VY0 + L33*VZ0]                 [1,19,24]
	CPR2.MAC3 = A3( ( CPC2.L31 * CPR2.VX0 + CPC2.L32 * CPR2.VY0 + CPC2.L33 * CPR2.VZ0 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( CPC2.RBK << 12 ) + CPC2.LR1 * CPR2.IR1 + CPC2.LR2 * CPR2.IR2 + CPC2.LR3 * CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A2[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( CPC2.GBK << 12 ) + CPC2.LG1 * CPR2.IR1 + CPC2.LG2 * CPR2.IR2 + CPC2.LG3 * CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A3[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( CPC2.BBK << 12 ) + CPC2.LB1 * CPR2.IR1 + CPC2.LB2 * CPR2.IR2 + CPC2.LB3 * CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,27,4]  MAC1=A1[R*IR1]                                        [1,27,16]
	CPR2.MAC1 = A1( ( ( ((u64)CPR2.R) << 4 ) * ((s64)CPR2.IR1) ) >> shift );
	
	// [1,27,4]  MAC2=A2[G*IR2]                                        [1,27,16]
	CPR2.MAC2 = A2( ( ( ((u64)CPR2.G) << 4 ) * ((s64)CPR2.IR2) ) >> shift );
	
	// [1,27,4]  MAC3=A3[B*IR3]                                        [1,27,16]
	CPR2.MAC3 = A3( ( ( ((u64)CPR2.B) << 4 ) * ((s64)CPR2.IR3) ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,27,4][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,27,4][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,27,4][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                              [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                              [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                              [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );
	
	//return true;
}

void COP2_Device::NCCT ( Cpu* r, Instruction::Format i )
{
#ifdef INLINE_DEBUG
	debug << "\r\n" << hex << setw( 8 ) << r->PC << dec << "  " << "NCCT";
#endif

	// 39 cycles
	// cop2 $118043F
	static const u32 c_InstructionCycles = 39;
	
	static const u32 InputRegs_Bitmap = BITMAP_VXY0 | BITMAP_ZeroVZ0 |
										BITMAP_VXY1 | BITMAP_ZeroVZ1 |
										BITMAP_VXY2 | BITMAP_ZeroVZ2 |
										BITMAP_RGB;
	
	// input (CPC): L11, L12, L13, L21, L22, L23, L31, L32, L33, LR1, LR2, LR3, LG1, LG2, LG3, LB1, LB2, LB3, RBK, GBK, BBK
	// input (CPR): VX0, VY0, VZ0, VX1, VY1, VZ1, VX2, VY2, VZ2, RGB
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3, RGB0, RGB1, RGB2
	
	// same as NCCS just repeat for V1 and V2

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	// get lm
	u32 lm = i.lm;

	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	////////////
	// for V0 //
	////////////
	
	// [1,19,12] MAC1=A1[L11*VX0 + L12*VY0 + L13*VZ0]                 [1,19,24]
	CPR2.MAC1 = A1( ( (s64)CPC2.L11 * (s64)CPR2.VX0 + (s64)CPC2.L12 * (s64)CPR2.VY0 + (s64)CPC2.L13 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,19,12] MAC2=A1[L21*VX0 + L22*VY0 + L23*VZ0]                 [1,19,24]
	CPR2.MAC2 = A2( ( (s64)CPC2.L21 * (s64)CPR2.VX0 + (s64)CPC2.L22 * (s64)CPR2.VY0 + (s64)CPC2.L23 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,19,12] MAC3=A1[L31*VX0 + L32*VY0 + L33*VZ0]                 [1,19,24]
	CPR2.MAC3 = A3( ( (s64)CPC2.L31 * (s64)CPR2.VX0 + (s64)CPC2.L32 * (s64)CPR2.VY0 + (s64)CPC2.L33 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( (s64)CPC2.RBK << 12 ) + (s64)CPC2.LR1 * (s64)CPR2.IR1 + (s64)CPC2.LR2 * (s64)CPR2.IR2 + (s64)CPC2.LR3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A2[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( (s64)CPC2.GBK << 12 ) + (s64)CPC2.LG1 * (s64)CPR2.IR1 + (s64)CPC2.LG2 * (s64)CPR2.IR2 + (s64)CPC2.LG3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A3[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( (s64)CPC2.BBK << 12 ) + (s64)CPC2.LB1 * (s64)CPR2.IR1 + (s64)CPC2.LB2 * (s64)CPR2.IR2 + (s64)CPC2.LB3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,27,4]  MAC1=A1[R*IR1]                                        [1,27,16]
	CPR2.MAC1 = A1( ( ( ((u64)CPR2.R) << 4 ) * (s64)CPR2.IR1 ) >> shift );
	
	// [1,27,4]  MAC2=A2[G*IR2]                                        [1,27,16]
	CPR2.MAC2 = A2( ( ( ((u64)CPR2.G) << 4 ) * (s64)CPR2.IR2 ) >> shift );
	
	// [1,27,4]  MAC3=A3[B*IR3]                                        [1,27,16]
	CPR2.MAC3 = A3( ( ( ((u64)CPR2.B) << 4 ) * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,27,4][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,27,4][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,27,4][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                              [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                              [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                              [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );

	////////////
	// for V1 //
	////////////
	
	// [1,19,12] MAC1=A1[L11*VX0 + L12*VY0 + L13*VZ0]                 [1,19,24]
	CPR2.MAC1 = A1( ( (s64)CPC2.L11 * (s64)CPR2.VX1 + (s64)CPC2.L12 * (s64)CPR2.VY1 + (s64)CPC2.L13 * (s64)CPR2.VZ1 ) >> shift );
	
	// [1,19,12] MAC2=A1[L21*VX0 + L22*VY0 + L23*VZ0]                 [1,19,24]
	CPR2.MAC2 = A2( ( (s64)CPC2.L21 * (s64)CPR2.VX1 + (s64)CPC2.L22 * (s64)CPR2.VY1 + (s64)CPC2.L23 * (s64)CPR2.VZ1 ) >> shift );
	
	// [1,19,12] MAC3=A1[L31*VX0 + L32*VY0 + L33*VZ0]                 [1,19,24]
	CPR2.MAC3 = A3( ( (s64)CPC2.L31 * (s64)CPR2.VX1 + (s64)CPC2.L32 * (s64)CPR2.VY1 + (s64)CPC2.L33 * (s64)CPR2.VZ1 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( (s64)CPC2.RBK << 12 ) + (s64)CPC2.LR1 * (s64)CPR2.IR1 + (s64)CPC2.LR2 * (s64)CPR2.IR2 + (s64)CPC2.LR3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A2[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( (s64)CPC2.GBK << 12 ) + (s64)CPC2.LG1 * (s64)CPR2.IR1 + (s64)CPC2.LG2 * (s64)CPR2.IR2 + (s64)CPC2.LG3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A3[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( (s64)CPC2.BBK << 12 ) + (s64)CPC2.LB1 * (s64)CPR2.IR1 + (s64)CPC2.LB2 * (s64)CPR2.IR2 + (s64)CPC2.LB3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,27,4]  MAC1=A1[R*IR1]                                        [1,27,16]
	CPR2.MAC1 = A1( ( ( ((u64)CPR2.R) << 4 ) * (s64)CPR2.IR1 ) >> shift );
	
	// [1,27,4]  MAC2=A2[G*IR2]                                        [1,27,16]
	CPR2.MAC2 = A2( ( ( ((u64)CPR2.G) << 4 ) * (s64)CPR2.IR2 ) >> shift );
	
	// [1,27,4]  MAC3=A3[B*IR3]                                        [1,27,16]
	CPR2.MAC3 = A3( ( ( ((u64)CPR2.B) << 4 ) * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,27,4][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,27,4][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,27,4][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                              [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                              [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                              [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );

	////////////
	// for V2 //
	////////////
	
	// [1,19,12] MAC1=A1[L11*VX0 + L12*VY0 + L13*VZ0]                 [1,19,24]
	CPR2.MAC1 = A1( ( (s64)CPC2.L11 * (s64)CPR2.VX2 + (s64)CPC2.L12 * (s64)CPR2.VY2 + (s64)CPC2.L13 * (s64)CPR2.VZ2 ) >> shift );
	
	// [1,19,12] MAC2=A1[L21*VX0 + L22*VY0 + L23*VZ0]                 [1,19,24]
	CPR2.MAC2 = A2( ( (s64)CPC2.L21 * (s64)CPR2.VX2 + (s64)CPC2.L22 * (s64)CPR2.VY2 + (s64)CPC2.L23 * (s64)CPR2.VZ2 ) >> shift );
	
	// [1,19,12] MAC3=A1[L31*VX0 + L32*VY0 + L33*VZ0]                 [1,19,24]
	CPR2.MAC3 = A3( ( (s64)CPC2.L31 * (s64)CPR2.VX2 + (s64)CPC2.L32 * (s64)CPR2.VY2 + (s64)CPC2.L33 * (s64)CPR2.VZ2 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( (s64)CPC2.RBK << 12 ) + (s64)CPC2.LR1 * (s64)CPR2.IR1 + (s64)CPC2.LR2 * (s64)CPR2.IR2 + (s64)CPC2.LR3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A2[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( (s64)CPC2.GBK << 12 ) + (s64)CPC2.LG1 * (s64)CPR2.IR1 + (s64)CPC2.LG2 * (s64)CPR2.IR2 + (s64)CPC2.LG3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A3[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( (s64)CPC2.BBK << 12 ) + (s64)CPC2.LB1 * (s64)CPR2.IR1 + (s64)CPC2.LB2 * (s64)CPR2.IR2 + (s64)CPC2.LB3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,27,4]  MAC1=A1[R*IR1]                                        [1,27,16]
	CPR2.MAC1 = A1( ( ( ((u64)CPR2.R) << 4 ) * (s64)CPR2.IR1 ) >> shift );
	
	// [1,27,4]  MAC2=A2[G*IR2]                                        [1,27,16]
	CPR2.MAC2 = A2( ( ( ((u64)CPR2.G) << 4 ) * (s64)CPR2.IR2 ) >> shift );
	
	// [1,27,4]  MAC3=A3[B*IR3]                                        [1,27,16]
	CPR2.MAC3 = A3( ( ( ((u64)CPR2.B) << 4 ) * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,27,4][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,27,4][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,27,4][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                              [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                              [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                              [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );
	
	//return true;
}

void COP2_Device::CC ( Cpu* r, Instruction::Format i )
{
#ifdef INLINE_DEBUG
	debug << "\r\n" << hex << setw( 8 ) << r->PC << dec << "  " << "CC";
#endif

	// 11 cycles
	// cop2 $138041C
	static const u32 c_InstructionCycles = 11;

	static const u32 InputRegs_Bitmap = BITMAP_IR1 | BITMAP_IR2 | BITMAP_IR3 | BITMAP_RGB;
	
	// input (CPC): LR1, LR2, LR3, LG1, LG2, LG3, LB1, LB2, LB3, RBK, GBK, BBK
	// input (CPR): IR1, IR2, IR3, RGB
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3, RGB0, RGB1, RGB2
	
	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	// get lm
	u32 lm = i.lm;

	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( (s64)CPC2.RBK << 12 ) + (s64)CPC2.LR1 * (s64)CPR2.IR1 + (s64)CPC2.LR2 * (s64)CPR2.IR2 + (s64)CPC2.LR3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A2[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( (s64)CPC2.GBK << 12 ) + (s64)CPC2.LG1 * (s64)CPR2.IR1 + (s64)CPC2.LG2 * (s64)CPR2.IR2 + (s64)CPC2.LG3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A3[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( (s64)CPC2.BBK << 12 ) + (s64)CPC2.LB1 * (s64)CPR2.IR1 + (s64)CPC2.LB2 * (s64)CPR2.IR2 + (s64)CPC2.LB3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,27,4]  MAC1=A1[R*IR1]                                        [1,27,16]
	CPR2.MAC1 = A1( ( ( (u64)CPR2.R << 4 ) * ((s64)CPR2.IR1) ) );
	
	// [1,27,4]  MAC2=A2[G*IR2]                                        [1,27,16]
	CPR2.MAC2 = A2( ( ( (u64)CPR2.G << 4 ) * ((s64)CPR2.IR2 )) );
	
	// [1,27,4]  MAC3=A3[B*IR3]                                        [1,27,16]
	CPR2.MAC3 = A3( ( ( (u64)CPR2.B << 4 ) * ((s64)CPR2.IR3) ) );

	// [MAC1,MAC2,MAC3] = [MAC1,MAC2,MAC3] SAR (sf*12)
	CPR2.MAC1 = A1( CPR2.MAC1 >> shift );
	CPR2.MAC2 = A2( CPR2.MAC2 >> shift );
	CPR2.MAC3 = A3( CPR2.MAC3 >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,27,4][lm=1]
	// left side must be using [1,11,4] format here
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,27,4][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,27,4][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );
	
	//return true;
}

void COP2_Device::NCS ( Cpu* r, Instruction::Format i )
{
#ifdef INLINE_DEBUG
	debug << "\r\n" << hex << setw( 8 ) << r->PC << dec << "  " << "NCS";
#endif

	// 14 cycles
	// cop2 $0C8041E
	static const u32 c_InstructionCycles = 14;
	
	static const u32 InputRegs_Bitmap = BITMAP_VXY0 | BITMAP_ZeroVZ0 | BITMAP_RGB;
	
	// input (CPC): L11, L12, L13, L21, L22, L23, L31, L32, L33, LR1, LR2, LR3, LG1, LG2, LG3, LB1, LB2, LB3, RBK, GBK, BBK
	// input (CPR): VX0, VY0, VZ0, RGB
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3, RGB0, RGB1, RGB2
	
	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	// get lm
	u32 lm = i.lm;

	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	// [1,19,12] MAC1=A1[L11*VX0 + L12*VY0 + L13*VZ0]                 [1,19,24]
	CPR2.MAC1 = A1( ( (s64)CPC2.L11 * (s64)CPR2.VX0 + (s64)CPC2.L12 * (s64)CPR2.VY0 + (s64)CPC2.L13 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,19,12] MAC2=A1[L21*VX0 + L22*VY0 + L23*VZ0]                 [1,19,24]
	CPR2.MAC2 = A2( ( (s64)CPC2.L21 * (s64)CPR2.VX0 + (s64)CPC2.L22 * (s64)CPR2.VY0 + (s64)CPC2.L23 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,19,12] MAC3=A1[L31*VX0 + L32*VY0 + L33*VZ0]                 [1,19,24]
	CPR2.MAC3 = A3( ( (s64)CPC2.L31 * (s64)CPR2.VX0 + (s64)CPC2.L32 * (s64)CPR2.VY0 + (s64)CPC2.L33 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( (s64)CPC2.RBK << 12 ) + (s64)CPC2.LR1 * (s64)CPR2.IR1 + (s64)CPC2.LR2 * (s64)CPR2.IR2 + (s64)CPC2.LR3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A2[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( (s64)CPC2.GBK << 12 ) + (s64)CPC2.LG1 * (s64)CPR2.IR1 + (s64)CPC2.LG2 * (s64)CPR2.IR2 + (s64)CPC2.LG3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A3[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( (s64)CPC2.BBK << 12 ) + (s64)CPC2.LB1 * (s64)CPR2.IR1 + (s64)CPC2.LB2 * (s64)CPR2.IR2 + (s64)CPC2.LB3 * (s64)CPR2.IR3 ) >> shift );
	
	
	
	
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );
	
	//return true;
}

void COP2_Device::NCT ( Cpu* r, Instruction::Format i )
{
#ifdef INLINE_DEBUG
	debug << "\r\n" << hex << setw( 8 ) << r->PC << dec << "  " << "NCT";
#endif

	// 30 cycles
	// cop2 $0D80420
	static const u32 c_InstructionCycles = 30;
	
	static const u32 InputRegs_Bitmap = BITMAP_VXY0 | BITMAP_ZeroVZ0 |
										BITMAP_VXY1 | BITMAP_ZeroVZ1 |
										BITMAP_VXY2 | BITMAP_ZeroVZ2 |
										BITMAP_RGB;
	
	// input (CPC): L11, L12, L13, L21, L22, L23, L31, L32, L33, LR1, LR2, LR3, LG1, LG2, LG3, LB1, LB2, LB3, RBK, GBK, BBK
	// input (CPR): VX0, VY0, VZ0, VX1, VY1, VZ1, VX2, VY2, VZ2, RGB
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3, RGB0, RGB1, RGB2
	
	// same as NCS just repeat for v1 and v2

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	// get lm
	u32 lm = i.lm;

	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	////////////
	// for V0 //
	////////////
	
	// [1,19,12] MAC1=A1[L11*VX0 + L12*VY0 + L13*VZ0]                 [1,19,24]
	CPR2.MAC1 = A1( ( (s64)CPC2.L11 * (s64)CPR2.VX0 + (s64)CPC2.L12 * (s64)CPR2.VY0 + (s64)CPC2.L13 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,19,12] MAC2=A1[L21*VX0 + L22*VY0 + L23*VZ0]                 [1,19,24]
	CPR2.MAC2 = A2( ( (s64)CPC2.L21 * (s64)CPR2.VX0 + (s64)CPC2.L22 * (s64)CPR2.VY0 + (s64)CPC2.L23 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,19,12] MAC3=A1[L31*VX0 + L32*VY0 + L33*VZ0]                 [1,19,24]
	CPR2.MAC3 = A3( ( (s64)CPC2.L31 * (s64)CPR2.VX0 + (s64)CPC2.L32 * (s64)CPR2.VY0 + (s64)CPC2.L33 * (s64)CPR2.VZ0 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( (s64)CPC2.RBK << 12 ) + (s64)CPC2.LR1 * (s64)CPR2.IR1 + (s64)CPC2.LR2 * (s64)CPR2.IR2 + (s64)CPC2.LR3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A2[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( (s64)CPC2.GBK << 12 ) + (s64)CPC2.LG1 * (s64)CPR2.IR1 + (s64)CPC2.LG2 * (s64)CPR2.IR2 + (s64)CPC2.LG3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A3[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( (s64)CPC2.BBK << 12 ) + (s64)CPC2.LB1 * (s64)CPR2.IR1 + (s64)CPC2.LB2 * (s64)CPR2.IR2 + (s64)CPC2.LB3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );

	////////////
	// for V1 //
	////////////
	
	// [1,19,12] MAC1=A1[L11*VX0 + L12*VY0 + L13*VZ0]                 [1,19,24]
	CPR2.MAC1 = A1( ( (s64)CPC2.L11 * (s64)CPR2.VX1 + (s64)CPC2.L12 * (s64)CPR2.VY1 + (s64)CPC2.L13 * (s64)CPR2.VZ1 ) >> shift );
	
	// [1,19,12] MAC2=A1[L21*VX0 + L22*VY0 + L23*VZ0]                 [1,19,24]
	CPR2.MAC2 = A2( ( (s64)CPC2.L21 * (s64)CPR2.VX1 + (s64)CPC2.L22 * (s64)CPR2.VY1 + (s64)CPC2.L23 * (s64)CPR2.VZ1 ) >> shift );
	
	// [1,19,12] MAC3=A1[L31*VX0 + L32*VY0 + L33*VZ0]                 [1,19,24]
	CPR2.MAC3 = A3( ( (s64)CPC2.L31 * (s64)CPR2.VX1 + (s64)CPC2.L32 * (s64)CPR2.VY1 + (s64)CPC2.L33 * (s64)CPR2.VZ1 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( (s64)CPC2.RBK << 12 ) + (s64)CPC2.LR1 * (s64)CPR2.IR1 + (s64)CPC2.LR2 * (s64)CPR2.IR2 + (s64)CPC2.LR3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A2[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( (s64)CPC2.GBK << 12 ) + (s64)CPC2.LG1 * (s64)CPR2.IR1 + (s64)CPC2.LG2 * (s64)CPR2.IR2 + (s64)CPC2.LG3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A3[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( (s64)CPC2.BBK << 12 ) + (s64)CPC2.LB1 * (s64)CPR2.IR1 + (s64)CPC2.LB2 * (s64)CPR2.IR2 + (s64)CPC2.LB3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );

	////////////
	// for V2 //
	////////////
	
	// [1,19,12] MAC1=A1[L11*VX0 + L12*VY0 + L13*VZ0]                 [1,19,24]
	CPR2.MAC1 = A1( ( (s64)CPC2.L11 * (s64)CPR2.VX2 + (s64)CPC2.L12 * (s64)CPR2.VY2 + (s64)CPC2.L13 * (s64)CPR2.VZ2 ) >> shift );
	
	// [1,19,12] MAC2=A1[L21*VX0 + L22*VY0 + L23*VZ0]                 [1,19,24]
	CPR2.MAC2 = A2( ( (s64)CPC2.L21 * (s64)CPR2.VX2 + (s64)CPC2.L22 * (s64)CPR2.VY2 + (s64)CPC2.L23 * (s64)CPR2.VZ2 ) >> shift );
	
	// [1,19,12] MAC3=A1[L31*VX0 + L32*VY0 + L33*VZ0]                 [1,19,24]
	CPR2.MAC3 = A3( ( (s64)CPC2.L31 * (s64)CPR2.VX2 + (s64)CPC2.L32 * (s64)CPR2.VY2 + (s64)CPC2.L33 * (s64)CPR2.VZ2 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [1,19,12] MAC1=A1[RBK + LR1*IR1 + LR2*IR2 + LR3*IR3]           [1,19,24]
	CPR2.MAC1 = A1( ( ( (s64)CPC2.RBK << 12 ) + (s64)CPC2.LR1 * (s64)CPR2.IR1 + (s64)CPC2.LR2 * (s64)CPR2.IR2 + (s64)CPC2.LR3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC2=A2[GBK + LG1*IR1 + LG2*IR2 + LG3*IR3]           [1,19,24]
	CPR2.MAC2 = A2( ( ( (s64)CPC2.GBK << 12 ) + (s64)CPC2.LG1 * (s64)CPR2.IR1 + (s64)CPC2.LG2 * (s64)CPR2.IR2 + (s64)CPC2.LG3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,19,12] MAC3=A3[BBK + LB1*IR1 + LB2*IR2 + LB3*IR3]           [1,19,24]
	CPR2.MAC3 = A3( ( ( (s64)CPC2.BBK << 12 ) + (s64)CPC2.LB1 * (s64)CPR2.IR1 + (s64)CPC2.LB2 * (s64)CPR2.IR2 + (s64)CPC2.LB3 * (s64)CPR2.IR3 ) >> shift );
	
	// [1,3,12]  IR1= Lm_B1[MAC1]                                     [1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,3,12]  IR2= Lm_B2[MAC2]                                     [1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,3,12]  IR3= Lm_B3[MAC3]                                     [1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );
	
	//return true;
}

void COP2_Device::DCPL ( Cpu* r, Instruction::Format i )
{
#ifdef INLINE_DEBUG
	debug << "\r\n" << hex << setw( 8 ) << r->PC << dec << "  " << "DCPL";
#endif

	// 8 cycles
	// cop2 $0680029
	static const u32 c_InstructionCycles = 8;
	
	static const u32 InputRegs_Bitmap = BITMAP_IR0 | BITMAP_IR1 | BITMAP_IR2 | BITMAP_IR3 | BITMAP_RGB;

	// input (CPC): RFC, GFC, BFC
	// input (CPR): RGB, IR0, IR1, IR2, IR3
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, RGB0, RGB1, RGB2

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	// get lm
	u32 lm = i.lm;

	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	/*
	// [1,27,4]  MAC1=A1[R*IR1 + IR0*(Lm_B1[RFC-R*IR1])]              [1,27,16][lm=0]
	CPR2.MAC1 = A1( ( ( (u64)CPR2.R << 4 ) * (s64)CPR2.IR1 + (s64)CPR2.IR0 * Lm_B1( (s64)CPC2.RFC - ( ( (u64)CPR2.R * (s64)CPR2.IR1 ) >> 8 ), 0 ) ) >> shift );
	
	// [1,27,4]  MAC2=A1[G*IR2 + IR0*(Lm_B2[GFC-G*IR2])]              [1,27,16][lm=0]
	CPR2.MAC2 = A2( ( ( (u64)CPR2.G << 4 ) * (s64)CPR2.IR2 + (s64)CPR2.IR0 * Lm_B2( (s64)CPC2.GFC - ( ( (u64)CPR2.G * (s64)CPR2.IR2 ) >> 8 ), 0 ) ) >> shift );
	
	// [1,27,4]  MAC3=A1[B*IR3 + IR0*(Lm_B3[BFC-B*IR3])]              [1,27,16][lm=0]
	CPR2.MAC3 = A3( ( ( (u64)CPR2.B << 4 ) * (s64)CPR2.IR3 + (s64)CPR2.IR0 * Lm_B3( (s64)CPC2.BFC - ( ( (u64)CPR2.B * (s64)CPR2.IR3 ) >> 8 ), 0 ) ) >> shift );
	*/
	
	// [MAC1,MAC2,MAC3] = [R*IR1,G*IR2,B*IR3] SHL 4          ;<--- for DCPL only
	CPR2.MAC1 = A1( ( ((u64)CPR2.R) * ((s64)CPR2.IR1) ) << 4 );
	CPR2.MAC2 = A2( ( ((u64)CPR2.G) * ((s64)CPR2.IR2) ) << 4 );
	CPR2.MAC3 = A3( ( ((u64)CPR2.B) * ((s64)CPR2.IR3) ) << 4 );

	// [MAC1,MAC2,MAC3] = MAC+(FC-MAC)*IR0
	// [IR1,IR2,IR3] = (([RFC,GFC,BFC] SHL 12) - [MAC1,MAC2,MAC3]) SAR (sf*12)
	CPR2.IR1 = Lm_B1( ((s64) ( ( ((s64)CPC2.RFC) << 12 ) - ((s64)CPR2.MAC1) )) >> shift, 0 );
	CPR2.IR2 = Lm_B2( ((s64) ( ( ((s64)CPC2.GFC) << 12 ) - ((s64)CPR2.MAC2) )) >> shift, 0 );
	CPR2.IR3 = Lm_B3( ((s64) ( ( ((s64)CPC2.BFC) << 12 ) - ((s64)CPR2.MAC3) )) >> shift, 0 );
	
	// [MAC1,MAC2,MAC3] = (([IR1,IR2,IR3] * IR0) + [MAC1,MAC2,MAC3])
	// [MAC1,MAC2,MAC3] = [MAC1,MAC2,MAC3] SAR (sf*12)
	CPR2.MAC1 = A1( ( ( ((s64)CPR2.IR1) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC1) ) >> shift );
	CPR2.MAC2 = A2( ( ( ((s64)CPR2.IR2) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC2) ) >> shift );
	CPR2.MAC3 = A3( ( ( ((s64)CPR2.IR3) * ((s64)CPR2.IR0) ) + ((s64)CPR2.MAC3) ) >> shift );
	
	//[1,11,4]  IR1=Lm_B1[MAC1]                                      [1,27,4]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, lm );
	
	//[1,11,4]  IR2=Lm_B2[MAC2]                                      [1,27,4]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, lm );
	
	//[1,11,4]  IR3=Lm_B3[MAC3]                                      [1,27,4]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, lm );
	
	//[0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	//[0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]                             [1,27,4]
	//[0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]                             [1,27,4]
	//[0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]                             [1,27,4]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );
	
	//return true;
}


void COP2_Device::AVSZ3 ( Cpu* r, Instruction::Format i )
{
	static const u32 InputCPCRegs [] = { INDEX_ZSF3 };
								
	static const u32 InputCPRRegs [] = { INDEX_SZ1, INDEX_SZ2, INDEX_SZ3 };

	static const u32 OutputCPCRegs [] = { INDEX_FLAG };
	static const u32 OutputCPRRegs [] = { INDEX_MAC0, INDEX_OTZ };


#if defined INLINE_DEBUG_COP2_AVSZ3 || defined INLINE_DEBUG_COP2_ALL || defined INLINE_DEBUG_NAME
	debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << "  " << "AVSZ3";
#endif
	
#if defined INLINE_DEBUG_COP2_AVSZ3 || defined INLINE_DEBUG_COP2_ALL
	int k;
	// show input values
	debug << "\r\n" << hex << "Input(CPC): ";
	for ( k = 0; k < sizeof(InputCPCRegs)/sizeof(InputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ InputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ InputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Input(CPR): ";
	for ( k = 0; k < sizeof(InputCPRRegs)/sizeof(InputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ InputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ InputCPRRegs [ k ] ]) << ";";
	}
	
#endif


	// 5 cycles
	// cop2 $158002D
	static const u32 c_InstructionCycles = 5;
	
	static const u32 InputRegs_Bitmap = BITMAP_SZ1 | BITMAP_SZ2 | BITMAP_SZ3;

	// input (CPC): ZSF3
	// input (CPR): SZ1, SZ2, SZ3
	// output (CPC): FLAG
	// output (CPR): MAC0, OTZ

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	// [1,31,0] MAC0=F[ZSF3*SZ1 + ZSF3*SZ2 + ZSF3*SZ3]                [1,31,12]
	CPR2.MAC0 = F( ((s64)CPC2.ZSF3) * ((s64) ( (u64) ( ((u64)CPR2.SZ1) + ((u64)CPR2.SZ2) + ((u64)CPR2.SZ3) ) ) ) );
	
	// [0,16,0] OTZ=Lm_D[MAC0]                                        [1,31,0]
	CPR2.OTZ = (u16) Lm_D( CPR2.MAC0 >> 12 );


#if defined INLINE_DEBUG_COP2_AVSZ3 || defined INLINE_DEBUG_COP2_ALL
	// show output values
	debug << "\r\n" << hex << "Output(CPC): ";
	for ( k = 0; k < sizeof(OutputCPCRegs)/sizeof(OutputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ OutputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ OutputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Output(CPR): ";
	for ( k = 0; k < sizeof(OutputCPRRegs)/sizeof(OutputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ OutputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ OutputCPRRegs [ k ] ]) << ";";
	}
	
#endif


	//return true;
}

void COP2_Device::AVSZ4 ( Cpu* r, Instruction::Format i )
{
	static const u32 InputCPCRegs [] = { INDEX_ZSF4 };
								
	static const u32 InputCPRRegs [] = { INDEX_SZ0, INDEX_SZ1, INDEX_SZ2, INDEX_SZ3 };

	static const u32 OutputCPCRegs [] = { INDEX_FLAG };
	static const u32 OutputCPRRegs [] = { INDEX_MAC0, INDEX_OTZ };

#if defined INLINE_DEBUG_COP2_AVSZ4 || defined INLINE_DEBUG_COP2_ALL || defined INLINE_DEBUG_NAME
	debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << "  " << "AVSZ4";
#endif
	
#if defined INLINE_DEBUG_COP2_AVSZ4 || defined INLINE_DEBUG_COP2_ALL
	int k;
	// show input values
	debug << "\r\n" << hex << "Input(CPC): ";
	for ( k = 0; k < sizeof(InputCPCRegs)/sizeof(InputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ InputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ InputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Input(CPR): ";
	for ( k = 0; k < sizeof(InputCPRRegs)/sizeof(InputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ InputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ InputCPRRegs [ k ] ]) << ";";
	}
	
#endif


	// 6 cycles
	// cop2 $168002E
	static const u32 c_InstructionCycles = 6;

	static const u32 InputRegs_Bitmap = BITMAP_SZ0 | BITMAP_SZ1 | BITMAP_SZ2 | BITMAP_SZ3;
	
	// input (CPC): ZSF4
	// input (CPR): SZ0, SZ1, SZ2, SZ3
	// output (CPC): FLAG
	// output (CPR): MAC0, OTZ

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	// [1,31,0] MAC0=F[ZSF4*SZ0 + ZSF4*SZ1 + ZSF4*SZ2 + ZSF4*SZ3]     [1,31,12]
	CPR2.MAC0 = F( ((s64)CPC2.ZSF4) * ( (s64) ( (u64) ( ((u64)CPR2.SZ0) + ((u64)CPR2.SZ1) + ((u64)CPR2.SZ2) + ((u64)CPR2.SZ3) ) ) ) );
	
	// [0,16,0] OTZ=Lm_D[MAC0]                                        [1,31,0]
	CPR2.OTZ = (u16) Lm_D( CPR2.MAC0 >> 12 );


#if defined INLINE_DEBUG_COP2_AVSZ4 || defined INLINE_DEBUG_COP2_ALL
	// show output values
	debug << "\r\n" << hex << "Output(CPC): ";
	for ( k = 0; k < sizeof(OutputCPCRegs)/sizeof(OutputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ OutputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ OutputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Output(CPR): ";
	for ( k = 0; k < sizeof(OutputCPRRegs)/sizeof(OutputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ OutputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ OutputCPRRegs [ k ] ]) << ";";
	}
	
#endif

	
	//return true;
}



void COP2_Device::SQR ( Cpu* r, Instruction::Format i )
{
	//static const u32 InputCPCRegs [] = { };
								
	static const u32 InputCPRRegs [] = { INDEX_IR1, INDEX_IR2, INDEX_IR3 };

	static const u32 OutputCPCRegs [] = { INDEX_FLAG };
	static const u32 OutputCPRRegs [] = { INDEX_MAC1, INDEX_MAC2, INDEX_MAC3, INDEX_IR1, INDEX_IR2, INDEX_IR3 };

#if defined INLINE_DEBUG_COP2_SQR || defined INLINE_DEBUG_COP2_ALL || defined INLINE_DEBUG_NAME
	debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << dec << " " << r->CycleCount << " " << "SQR";
	debug << " " << i.Value << " lm=" << i.lm << " sf=" << i.sf;
#endif
	
#if defined INLINE_DEBUG_COP2_SQR || defined INLINE_DEBUG_COP2_ALL
	int k;
	// show input values
	debug << "\r\n" << hex << "Input(CPC): ";
	//for ( k = 0; k < sizeof(InputCPCRegs)/sizeof(InputCPCRegs[0]); k++ )
	//{
	//	debug << CPC_RegisterNames [ InputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ InputCPCRegs [ k ] ]) << ";";
	//}
	
	debug << "\r\n" << hex << "Input(CPR): ";
	for ( k = 0; k < sizeof(InputCPRRegs)/sizeof(InputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ InputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ InputCPRRegs [ k ] ]) << ";";
	}
	
#endif


	// 5 cycles
	// fields: sf
	// cop2 $0a00428
	static const u32 c_InstructionCycles = 5;
	
	static const u32 InputRegs_Bitmap = BITMAP_IR1 | BITMAP_IR2 | BITMAP_IR3;
	
	// input (CPC):
	// input (CPR): IR1, IR2, IR3
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;
	
	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) | ( shift << 2 );
	
	// *note* lm has no effect since the results of a square are always positive
	//u32 lm = i.lm;


	// Calculation: (left format sf=0, right format sf=1)

	// [1,31,0][1,19,12] MAC1=A1[IR1*IR1]                     [1,43,0][1,31,12]
	CPR2.MAC1 = A1( ( ((s64)CPR2.IR1) * ((s64)CPR2.IR1) ) >> shift );
	
	// [1,31,0][1,19,12] MAC2=A2[IR2*IR2]                     [1,43,0][1,31,12]
	CPR2.MAC2 = A2( ( ((s64)CPR2.IR2) * ((s64)CPR2.IR2) ) >> shift );
	
	// [1,31,0][1,19,12] MAC3=A3[IR3*IR3]                     [1,43,0][1,31,12]
	CPR2.MAC3 = A3( ( ((s64)CPR2.IR3) * ((s64)CPR2.IR3) ) >> shift );
	
	// [1,15,0][1,3,12]  IR1=Lm_B1[MAC1]                      [1,31,0][1,19,12][lm=1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, 1 );
	
	// [1,15,0][1,3,12]  IR2=Lm_B2[MAC2]                      [1,31,0][1,19,12][lm=1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, 1 );
	
	// [1,15,0][1,3,12]  IR3=Lm_B3[MAC3]                      [1,31,0][1,19,12][lm=1]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, 1 );


#if defined INLINE_DEBUG_COP2_SQR || defined INLINE_DEBUG_COP2_ALL
	// show output values
	debug << "\r\n" << hex << "Output(CPC): ";
	for ( k = 0; k < sizeof(OutputCPCRegs)/sizeof(OutputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ OutputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ OutputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Output(CPR): ";
	for ( k = 0; k < sizeof(OutputCPRRegs)/sizeof(OutputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ OutputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ OutputCPRRegs [ k ] ]) << ";";
	}
	
#endif

	
	//return true;
}

void COP2_Device::GPF ( Cpu* r, Instruction::Format i )
{
	static const u32 InputCPRRegs [] = { INDEX_IR0, INDEX_IR1, INDEX_IR2, INDEX_IR3 };

	static const u32 OutputCPCRegs [] = { INDEX_FLAG };
	static const u32 OutputCPRRegs [] = { INDEX_MAC1, INDEX_MAC2, INDEX_MAC3, INDEX_IR1, INDEX_IR2, INDEX_IR3, INDEX_RGB0, INDEX_RGB1, INDEX_RGB2 };
	
#ifdef INLINE_DEBUG
	debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << dec << " " << r->CycleCount << " " << "GPF";
	debug << " " << i.Value << " lm=" << i.lm << " sf=" << i.sf;
#endif

#if defined INLINE_DEBUG_COP2_GPF || defined INLINE_DEBUG_COP2_ALL
	int k;
	// show input values
	debug << "\r\n" << hex << "Input(CPC): ";
	//for ( k = 0; k < sizeof(InputCPCRegs)/sizeof(InputCPCRegs[0]); k++ )
	//{
	//	debug << CPC_RegisterNames [ InputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ InputCPCRegs [ k ] ]) << ";";
	//}
	
	debug << "\r\n" << hex << "Input(CPR): ";
	for ( k = 0; k < sizeof(InputCPRRegs)/sizeof(InputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ InputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ InputCPRRegs [ k ] ]) << ";";
	}
	
#endif

	// 5 cycles
	// fields: sf
	// cop2 $190003D
	static const u32 c_InstructionCycles = 5;
	
	static const u32 InputRegs_Bitmap = BITMAP_IR0 | BITMAP_IR1 | BITMAP_IR2 | BITMAP_IR3;
	
	// input (CPC):
	// input (CPR): IR0, IR1, IR2, IR3
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3, RGB0, RGB1, RGB2

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	u32 lm = i.lm;
	
	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	// MAC1=A1[IR0 * IR1]
	CPR2.MAC1 = A1( ( ((s64)CPR2.IR0) * ((s64)CPR2.IR1) ) >> shift );
	
	// MAC2=A2[IR0 * IR2]
	CPR2.MAC2 = A2( ( ((s64)CPR2.IR0) * ((s64)CPR2.IR2) ) >> shift );
	
	// MAC3=A3[IR0 * IR3]
	CPR2.MAC3 = A3( ( ((s64)CPR2.IR0) * ((s64)CPR2.IR3) ) >> shift );
	
	// IR1=Lm_B1[MAC1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, lm );
	
	// IR2=Lm_B2[MAC2]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, lm );
	
	// IR3=Lm_B3[MAC3]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, lm );
	
	// [0,8,0]   Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]   R0<-R1<-R2<- Lm_C1[MAC1]
	// [0,8,0]   G0<-G1<-G2<- Lm_C2[MAC2]
	// [0,8,0]   B0<-B1<-B2<- Lm_C3[MAC3]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );
	
	//return true;
#if defined INLINE_DEBUG_COP2_GPF || defined INLINE_DEBUG_COP2_ALL
	// show output values
	debug << "\r\n" << hex << "Output(CPC): ";
	for ( k = 0; k < sizeof(OutputCPCRegs)/sizeof(OutputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ OutputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ OutputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Output(CPR): ";
	for ( k = 0; k < sizeof(OutputCPRRegs)/sizeof(OutputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ OutputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ OutputCPRRegs [ k ] ]) << ";";
	}
	
#endif
}

void COP2_Device::GPL ( Cpu* r, Instruction::Format i )
{
#ifdef INLINE_DEBUG
	debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << dec << " " << r->CycleCount << " " << "GPL";
	debug << " " << i.Value << " lm=" << i.lm << " sf=" << i.sf;
#endif

	// 5 cycles
	// fields: sf
	// cop2 $1A0003E
	static const u32 c_InstructionCycles = 5;

	static const u32 InputRegs_Bitmap = BITMAP_IR0 | BITMAP_IR1 | BITMAP_IR2 | BITMAP_IR3;
	
	// input (CPC):
	// input (CPR): MAC1, MAC2, MAC3, IR0, IR1, IR2, IR3
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3, RGB0, RGB1, RGB2

	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;

	u32 lm = i.lm;
	
	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	// MAC1=A1[MAC1 + IR0 * IR1]
	CPR2.MAC1 = A1( ( ( ((s64)CPR2.MAC1) << shift ) + ((s64)CPR2.IR0) * ((s64)CPR2.IR1) ) >> shift );
	
	// MAC2=A2[MAC2 + IR0 * IR2]
	CPR2.MAC2 = A2( ( ( ((s64)CPR2.MAC2) << shift ) + ((s64)CPR2.IR0) * ((s64)CPR2.IR2) ) >> shift );
	
	// MAC3=A3[MAC3 + IR0 * IR3]
	CPR2.MAC3 = A3( ( ( ((s64)CPR2.MAC3) << shift ) + ((s64)CPR2.IR0) * ((s64)CPR2.IR3) ) >> shift );
	
	// IR1=Lm_B1[MAC1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, lm );
	
	// IR2=Lm_B2[MAC2]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, lm );
	
	// IR3=Lm_B3[MAC3]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, lm );
	
	// [0,8,0]  Cd0<-Cd1<-Cd2<- CODE
	// [0,8,0]  R0<-R1<-R2<- Lm_C1[MAC1]
	// [0,8,0]  G0<-G1<-G2<- Lm_C2[MAC2]
	// [0,8,0]  B0<-B1<-B2<- Lm_C3[MAC3]
	CPR2.RGB0 = CPR2.RGB1;
	CPR2.RGB1 = CPR2.RGB2;
	CPR2.Cd2 = CPR2.CODE;
	CPR2.R2 = (u8) Lm_C1( CPR2.MAC1 >> 4 );
	CPR2.G2 = (u8) Lm_C2( CPR2.MAC2 >> 4 );
	CPR2.B2 = (u8) Lm_C3( CPR2.MAC3 >> 4 );
	
	//return true;
}


void COP2_Device::OP ( Cpu* r, Instruction::Format i )
{
	static const u32 InputCPCRegs [] = { INDEX_R11R12, INDEX_R22R23, INDEX_ZeroR33 };

	static const u32 InputCPRRegs [] = { INDEX_IR1, INDEX_IR2, INDEX_IR3 };

	static const u32 OutputCPCRegs [] = { INDEX_FLAG };
	static const u32 OutputCPRRegs [] = { INDEX_MAC1, INDEX_MAC2, INDEX_MAC3, INDEX_IR1, INDEX_IR2, INDEX_IR3 };
	
//#ifdef INLINE_DEBUG
//	debug << "\r\n" << hex << setw( 8 ) << r->PC << dec << "  " << "OP";
//#endif
#if defined INLINE_DEBUG_COP2_OP || defined INLINE_DEBUG_COP2_ALL || defined INLINE_DEBUG_NAME
	debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << dec << " " << r->CycleCount << " " << "OP";
	debug << " " << i.Value << " lm=" << i.lm << " sf=" << i.sf;
#endif

#if defined INLINE_DEBUG_COP2_OP || defined INLINE_DEBUG_COP2_ALL
	int k;
	// show input values
	debug << "\r\n" << hex << "Input(CPC): ";
	for ( k = 0; k < sizeof(InputCPCRegs)/sizeof(InputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ InputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ InputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Input(CPR): ";
	for ( k = 0; k < sizeof(InputCPRRegs)/sizeof(InputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ InputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ InputCPRRegs [ k ] ]) << ";";
	}
#endif

	// 6 cycles
	// fields: sf
	// cop2 $170000C
	static const u32 c_InstructionCycles = 6;
	
	static const u32 InputRegs_Bitmap = BITMAP_IR1 | BITMAP_IR2 | BITMAP_IR3;
	
	// input (CPC): R11, R22, R33
	// input (CPR): IR1, IR2, IR3
	// output (CPC): FLAG
	// output (CPR): MAC1, MAC2, MAC3, IR1, IR2, IR3

	// mame uses lm field
	
	//CHECK_LOADING_COP2( InputRegs_Bitmap );
	
	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;
	
	// get lm
	u32 lm = i.lm;

	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	// Calculation: (D1=R11R12,D2=R22R23,D3=R33)

	// MAC1=A1[D2*IR3 - D3*IR2]
	CPR2.MAC1 = A1( ( ( ( ((s64)CPC2.R22) * ((s64)CPR2.IR3) ) - ( ((s64)CPC2.R33) * ((s64)CPR2.IR2) ) ) ) >> shift );
	
	// MAC2=A2[D3*IR1 - D1*IR3]
	CPR2.MAC2 = A2( ( ( ( ((s64)CPC2.R33) * ((s64)CPR2.IR1) ) - ( ((s64)CPC2.R11) * ((s64)CPR2.IR3) ) ) ) >> shift );
	
	// MAC3=A3[D1*IR2 - D2*IR1]
	CPR2.MAC3 = A3( ( ( ( ((s64)CPC2.R11) * ((s64)CPR2.IR2) ) - ( ((s64)CPC2.R22) * ((s64)CPR2.IR1) ) ) ) >> shift );
	
	// IR1=Lm_B1[MAC0]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, lm );
	
	// IR2=Lm_B2[MAC1]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, lm );
	
	// IR3=Lm_B3[MAC2]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, lm );


#if defined INLINE_DEBUG_COP2_OP || defined INLINE_DEBUG_COP2_ALL
	// show output values
	debug << "\r\n" << hex << "Output(CPC): ";
	for ( k = 0; k < sizeof(OutputCPCRegs)/sizeof(OutputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ OutputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ OutputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Output(CPR): ";
	for ( k = 0; k < sizeof(OutputCPRRegs)/sizeof(OutputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ OutputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ OutputCPRRegs [ k ] ]) << ";";
	}
#endif

	//return true;
}


void COP2_Device::MVMVA ( Cpu* r, Instruction::Format i )
{
	//static const u32 InputCPCRegs [] = { };
	
	static const u32 InputCPRRegs [] = { INDEX_IR1, INDEX_IR2, INDEX_IR3 };

	static const u32 OutputCPCRegs [] = { INDEX_FLAG };
	static const u32 OutputCPRRegs [] = { INDEX_MAC1, INDEX_MAC2, INDEX_MAC3, INDEX_IR1, INDEX_IR2, INDEX_IR3 };
	

	// 8 cycles
	// fields: sf,mx,v,cv,lm
	// cop2 $0400012
	static const u32 c_InstructionCycles = 8;
	
	// input (CPC):
	// input (CPR):
	// output (CPC):
	// output (CPR):
	
	// matrix pointers
	s16 *MX;
	s16 *V;
	s32 *CV;
	
	s16 Vect [ 3 ];
	
	s16 GarbageMatrix [ 9 ] = { -0x60, 0x60, CPR2.IR0, CPC2.R13, CPC2.R13, CPC2.R13, CPC2.R22, CPC2.R22, CPC2.R22 };
	
	Matrix_Picker [ 3 ] = GarbageMatrix;
	
	// get lm
	u32 lm = i.lm;

	// get shift = sf * 12
	u32 shift = i.sf;
	shift = ( shift << 3 ) + ( shift << 2 );
	
	// Calculation:
	// MX = matrix specified by mx
	// V = vector specified by v
	// CV = vector specified by cv
	
	u32 mx = i.mx;
	u32 v = i.v;
	u32 cv = i.cv;

	// *** todo ***
	MX = Matrix_Picker [ mx ];
	//V = Vector_Picker [ v ];
	CV = CVector_Picker [ cv ];

	/*
	switch ( cv )
	{
		case 0:
			CV = &CPC2.TRX;
			break;
			
		case 1:
			CV = &CPC2.RBK;
			break;
			
		case 2:
			CV = &CPC2.RFC;
			break;
		
		case 3:
			CV = &(Zero_Vector [ 0 ]);
			break;
	}
	
	switch ( mx )
	{
		case 0:
			MX = &CPC2.R11;
			break;
			
		case 1:
			MX = &CPC2.L11;
			break;
			
		case 2:
			MX = &CPC2.LR1;
			break;
			
		case 3:
			MX = &CPC2.LR1;
			break;
	}
	*/

	switch ( v )
	{
		case 0:
			Vect [ 0 ] = CPR2.VX0;
			Vect [ 1 ] = CPR2.VY0;
			Vect [ 2 ] = CPR2.VZ0;
			break;
			
		case 1:
			Vect [ 0 ] = CPR2.VX1;
			Vect [ 1 ] = CPR2.VY1;
			Vect [ 2 ] = CPR2.VZ1;
			break;
			
		case 2:
			Vect [ 0 ] = CPR2.VX2;
			Vect [ 1 ] = CPR2.VY2;
			Vect [ 2 ] = CPR2.VZ2;
			break;
			
		case 3:
			Vect [ 0 ] = CPR2.IR1;
			Vect [ 1 ] = CPR2.IR2;
			Vect [ 2 ] = CPR2.IR3;
			break;
		
	}


#if defined INLINE_DEBUG_COP2_MVMVA || defined INLINE_DEBUG_COP2_ALL || defined INLINE_DEBUG_NAME
	//debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << "  " << dec << r->CycleCount << hex << " MVMVA " << i.Value << " mx=" << i.mx << " cv=" << i.cv << " " << cv << " v=" << i.v;
	debug << "\r\n\r\n" << hex << setw( 8 ) << r->PC << dec << " " << r->CycleCount << " " << "MVMVA";
	debug << " " << i.Value << " lm=" << i.lm << " sf=" << i.sf;
	debug << " mx=" << i.mx << " cv=" << i.cv << " " << cv << " v=" << i.v;
#endif

#if defined INLINE_DEBUG_COP2_MVMVA || defined INLINE_DEBUG_COP2_ALL
	int k;
	// show input values
	debug << "\r\n" << hex << "Input(CPC): ";
	debug << "\r\nMX[0]=" << MX[0] << " MX[1]=" << MX[1] << " MX[2]=" << MX[2] << " MX[3]=" << MX[3] << " MX[4]=" << MX[4] << " MX[5]=" << MX[5] << " MX[6]=" << MX[6] << " MX[7]=" << MX[7] << " MX[8]=" << MX[8];
	debug << "\r\nCV[0]=" << CV[0] << " CV[1]=" << CV[1] << " CV[2]=" << CV[2];
	//for ( k = 0; k < sizeof(InputCPCRegs)/sizeof(InputCPCRegs[0]); k++ )
	//{
	//	debug << CPC_RegisterNames [ InputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ InputCPCRegs [ k ] ]) << ";";
	//}
	
	debug << "\r\n" << hex << "Input(CPR): ";
	debug << "\r\nVect[0]=" << Vect[0] << " Vect[1]=" << Vect[1] << " Vect[2]=" << Vect[2];
	//for ( k = 0; k < sizeof(InputCPRRegs)/sizeof(InputCPRRegs[0]); k++ )
	//{
	//	debug << CPR_RegisterNames [ InputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ InputCPRRegs [ k ] ]) << ";";
	//}
	
#endif

	// *** TESTING *** //
	//CHECK_LOADING_COP2( 0xffffffff );
	
	// make sure cop2 is not busy
	if ( BusyUntil_Cycle > r->CycleCount )
	{
		// for now, just add onto memory latency
		//Cpu::MemoryLatency += BusyUntil_Cycle - r->CycleCount;
		r->CycleCount = BusyUntil_Cycle;
		
		// must update events after updating cyclecount //
		//r->UpdateEvents ();
		
		/*
		// get the cycle that COP2 is busy until
		r->BusyUntil_Cycle = BusyUntil_Cycle;
		
		// wait until COP2 is ready
		r->WaitForCpuReady1 ();
		*/
	}
	
	// set the amount of time cop2 will be busy for
	BusyUntil_Cycle = r->CycleCount + c_InstructionCycles;
	
	// clear flags
	CPC2.FLAG.Value = 0;


	// MAC1=A1[CV1 + MX11*V1 + MX12*V2 + MX13*V3]
	CPR2.MAC1 = A1( ( ( ((s64)(CV [ 0 ])) << 12 ) + ((s64)MX [ 0 ]) * ((s64)Vect [ 0 ]) + ((s64)MX [ 1 ]) * ((s64)Vect [ 1 ]) + ((s64)MX [ 2 ]) * ((s64)Vect [ 2 ]) ) >> shift );

	// MAC2=A2[CV2 + MX21*V1 + MX22*V2 + MX23*V3]
	CPR2.MAC2 = A2( ( ( ((s64)(CV [ 1 ])) << 12 ) + ((s64)MX [ 3 ]) * ((s64)Vect [ 0 ]) + ((s64)MX [ 4 ]) * ((s64)Vect [ 1 ]) + ((s64)MX [ 5 ]) * ((s64)Vect [ 2 ]) ) >> shift );
	
	// MAC3=A3[CV3 + MX31*V1 + MX32*V2 + MX33*V3]
	CPR2.MAC3 = A3( ( ( ((s64)(CV [ 2 ])) << 12 ) + ((s64)MX [ 6 ]) * ((s64)Vect [ 0 ]) + ((s64)MX [ 7 ]) * ((s64)Vect [ 1 ]) + ((s64)MX [ 8 ]) * ((s64)Vect [ 2 ]) ) >> shift );
	
	// IR1=Lm_B1[MAC1]
	CPR2.IR1 = Lm_B1( CPR2.MAC1, lm );
	
	// IR2=Lm_B2[MAC2]
	CPR2.IR2 = Lm_B2( CPR2.MAC2, lm );
	
	// IR3=Lm_B3[MAC3]
	CPR2.IR3 = Lm_B3( CPR2.MAC3, lm );

	// Notes:
	// The cv field allows selection of the far color vector, but this vector
	// is not added correctly by the GTE.
	if ( cv == 2 )
	{
		CPR2.MAC1 = A1( ( ((s64)MX [ 2 ]) * ((s64)Vect [ 2 ]) ) >> shift );
		CPR2.MAC2 = A2( ( ((s64)MX [ 5 ]) * ((s64)Vect [ 2 ]) ) >> shift );
		CPR2.MAC3 = A3( ( ((s64)MX [ 8 ]) * ((s64)Vect [ 2 ]) ) >> shift );
		CPR2.IR1 = Lm_B1( CPR2.MAC1, lm );
		CPR2.IR2 = Lm_B2( CPR2.MAC2, lm );
		CPR2.IR3 = Lm_B3( CPR2.MAC3, lm );
	}


#if defined INLINE_DEBUG_COP2_MVMVA || defined INLINE_DEBUG_COP2_ALL
	// show output values
	debug << "\r\n" << hex << "Output(CPC): ";
	for ( k = 0; k < sizeof(OutputCPCRegs)/sizeof(OutputCPCRegs[0]); k++ )
	{
		debug << CPC_RegisterNames [ OutputCPCRegs [ k ] ] << "=" << *(CPC_RegisterPtrs [ OutputCPCRegs [ k ] ]) << ";";
	}
	
	debug << "\r\n" << hex << "Output(CPR): ";
	for ( k = 0; k < sizeof(OutputCPRRegs)/sizeof(OutputCPRRegs[0]); k++ )
	{
		debug << CPR_RegisterNames [ OutputCPRRegs [ k ] ] << "=" << *(CPR_RegisterPtrs [ OutputCPRRegs [ k ] ]) << ";";
	}
#endif

	
	//return true;
}


