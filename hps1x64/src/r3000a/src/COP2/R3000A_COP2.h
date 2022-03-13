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


#ifndef _R3000A_COP2_H_
#define _R3000A_COP2_H_

#include "types.h"
#include "R3000A_Instruction.h"

#include "Debug.h"

// the stuff for the GTE comes from gte.txt by doomed@c64.org and also from source code of programs like MAME and pcsx

namespace R3000A
{

	class Cpu;

	class COP2_Device
	{
	private:
	
		static Debug::Log debug;
	
	public:
	
		static const char* CPC_RegisterNames [ 32 ];

		static const char* CPR_RegisterNames [ 32 ];

		static u32* CPC_RegisterPtrs [ 32 ];

		static u32* CPR_RegisterPtrs [ 32 ];


		// busy until cycles
		u64 BusyUntil_Cycle;

		// MAC 0,1,2,3 are actually 44-bit signed values
		s64 MAC0, MAC1, MAC2, MAC3;
		
		// Control Registers
		union COP2_ControlRegs
		{
			u32 Regs [ 32 ];
			
			struct
			{
				///////////////////
				// 1, 3, 12
				union
				{
					struct
					{
						// bits 0-15
						s16 R11;
						
						// bits 16-31
						s16 R12;
						
						s16 R13;
						s16 R21;
						s16 R22;
						s16 R23;
						s16 R31;
						s16 R32;
						s16 R33;
						s16 zero0;
					};
					
					struct
					{
						// Register #0
						u32 R11R12;
						
						// Register #1
						u32 R13R21;
						
						// Register #2
						u32 R22R23;
						
						// Register #3
						u32 R31R32;
						
						// Register #4
						u32 ZeroR33;
					};
				};
				
				/////////////////
				// 1, 31, 0
				
				// Register #5
				s32 TRX;
				
				// Register #6
				s32 TRY;
				
				// Register #7
				s32 TRZ;

				//////////////////
				// 1, 3, 12
				union
				{
					struct
					{
						s16 L11;
						s16 L12;
						s16 L13;
						s16 L21;
						s16 L22;
						s16 L23;
						s16 L31;
						s16 L32;
						s16 L33;
						s16 zero1;
					};
				
					struct
					{
						// Register #8
						u32 L11L12;
						
						// Register #9
						u32 L13L21;
						
						// Register #10
						u32 L22L23;
						
						// Register #11
						u32 L31L32;
						
						// Register #12
						u32 ZeroL33;
					};
				};

				/////////////////
				// 1, 19, 12
				
				// Register #13
				s32 RBK;
				
				// Register #14
				s32 GBK;
				
				// Register #15
				s32 BBK;

				///////////////////
				// 1, 3, 12
				union
				{
					struct
					{
						s16 LR1;
						s16 LR2;
						s16 LR3;
						s16 LG1;
						s16 LG2;
						s16 LG3;
						s16 LB1;
						s16 LB2;
						s16 LB3;
						s16 zero2;
					};
				
					struct
					{
						// Register #16
						u32 LR1LR2;
						
						// Register #17
						u32 LR3LG1;
						
						// Register #18
						u32 LG2LG3;
						
						// Register #19
						u32 LB1LB2;
						
						// Register #20
						u32 ZeroLB3;
					};
				};

				////////////////
				// 1, 27, 4
				
				// Register #21
				s32 RFC;
				
				// Register #22
				s32 GFC;
				
				// Register #23
				s32 BFC;

				/////////////////
				// 1, 15, 16
				
				// Register #24
				s32 OFX;
				
				// Register #25
				s32 OFY;

				//////////////////
				// 0, 16, 0
				
				// Register #26
				u16 H;
				u16 zero3;

				//////////////////
				// 1, 7, 8
				
				// Register #27
				s16 DQA;
				u16 zero4;

				/////////////////
				// 1, 7, 24
				
				// Register #28
				s32 DQB;

				////////////////////
				// 1, 3, 12
				
				// Register #29
				s16 ZSF3;
				u16 zero5;
				
				// Register #30
				s16 ZSF4;
				u16 zero6;

				// Register #31
				union
				{
					struct
					{
						// bits 0-11 - not used
						u32 NotUsed : 12;
						
						// bit 12 - Value negative or larger than 12 bits
						u32 H : 1;
						
						// bit 13 - Value larger than 10 bits
						u32 G2 : 1;
						
						// bit 14 - Value larger than 10 bits
						u32 G1 : 1;
						
						// bit 15 - Result larger than 31 bits and negative
						u32 FN : 1;
						
						// bit 16 - Result larger than 31 bits and positive
						u32 FP : 1;
						
						// bit 17 - Divide overflow. (quotient > 2.0)
						u32 E : 1;
						
						// bit 18 - Value negative or larger than 16 bits
						u32 D : 1;
						
						// bit 19 - Value negative or larger than 8 bits
						u32 C3 : 1;
						
						// bit 20 - Value negative or larger than 8 bits
						u32 C2 : 1;
						
						// bit 21 - Value negative or larger than 8 bits
						u32 C1 : 1;
						
						// bit 22 - Value negative(lm=1) or larger than 15 bits(lm=0)
						u32 B3 : 1;
						
						// bit 23 - Value negative(lm=1) or larger than 15 bits(lm=0)
						u32 B2 : 1;
						
						// bit 24 - Value negative(lm=1) or larger than 15 bits(lm=0)
						u32 B1 : 1;
						
						// bit 25 - Result larger than 43 bits and negative
						u32 A3N : 1;
						
						// bit 26 - Result larger than 43 bits and negative
						u32 A2N : 1;
						
						// bit 27 - Result larger than 43 bits and negative
						u32 A1N : 1;
						
						// bit 28 - Result larger than 43 bits and positive
						u32 A3P : 1;
						
						// bit 29 - Result larger than 43 bits and positive
						u32 A2P : 1;
						
						// bit 30 - Result larger than 43 bits and positive
						u32 A1P : 1;
						
						// bit 31 - Checksum
						// Logical sum of bits 30-23 and bits 18-13
						u32 Checksum : 1;
						
					};
				
					u32 Value;
				} FLAG;
				
			};
		};

		
		// Data Registers
		union COP2_DataRegs
		{
			u32 Regs [ 32 ];
			
			struct
			{
				//////////////////////////////////
				// 1, 3, 12 or 1, 15, 0 - r/w
				union
				{
					struct
					{
						s16 VX0;
						s16 VY0;
						s16 VZ0;
						s16 zero0;
						s16 VX1;
						s16 VY1;
						s16 VZ1;
						s16 zero1;
						s16 VX2;
						s16 VY2;
						s16 VZ2;
						s16 zero2;
					};
				
					struct
					{
						// Register #0
						u32 VXY0;
						
						// Register #1
						u32 ZeroVZ0;
						
						// Register #2
						u32 VXY1;
						
						// Register #3
						u32 ZeroVZ1;
						
						// Register #4
						u32 VXY2;
						
						// Register #5
						u32 ZeroVZ2;
					};
				};

				///////////////////////////
				// (8,8,8) - r/w
				union
				{
					struct
					{
						u8 R;
						u8 G;
						u8 B;
						u8 CODE;
					};

					// Register #6
					u32 RGB;	// actually stored as BGR
				};

				////////////////////////////////
				// 0, 15, 0 - read only
				// Register #7
				u16 OTZ;
				u16 Zero4;

				///////////////////////////////////////////////////
				// 1, 3, 12 - r/w
				// output can be 1, 11, 4
				// is sign extended to fill the full 32-bits
				
				// Register #8
				s16 IR0;
				s16 signIR0;
				
				// Register #9
				s16 IR1;
				s16 signIR1;
				
				// Register #10
				s16 IR2;
				s16 signIR2;
				
				// Register #11
				s16 IR3;
				s16 signIR3;

				//////////////////////////
				// 1, 15, 0 - r/w
				union
				{
					struct
					{
						s16 SX0;
						s16 SY0;
						s16 SX1;
						s16 SY1;
						s16 SX2;
						s16 SY2;
						s16 SXP;
						s16 SYP;
					};
				
					struct
					{
						// Register #12
						u32 SXY0;
						
						// Register #13
						u32 SXY1;
						
						// Register #14
						u32 SXY2;
						
						// Register #15
						u32 SXYP;
					};
				};

				///////////////////////////////////////////
				// 0, 16, 0 - r/w
				// unsigned and occupies lower 16 bits
				
				// Register #16
				u16 SZ0;
				u16 Zero0;
				
				// Register #17
				u16 SZ1;
				u16 Zero1;
				
				// Register #18
				u16 SZ2;
				u16 Zero2;
				
				// Register #19
				u16 SZ3;
				u16 Zero3;
				
				// r/w
				union
				{
					struct
					{
						u8 R0;
						u8 G0;
						u8 B0;
						u8 Cd0;
					};
					
					// Register #20
					u32 RGB0;
				};
				
				union
				{
					struct
					{
						u8 R1;
						u8 G1;
						u8 B1;
						u8 Cd1;
					};

					// Register #21
					u32 RGB1;
				};
				
				union
				{
					struct
					{
						u8 R2;
						u8 G2;
						u8 B2;
						u8 Cd2;
					};

					// Register #22
					u32 RGB2;
				};

				// Register #23
				u32 Reserved;

				///////////////////////////////////
				// 1, 31, 0 - r/w
				
				// Register #24
				s32 MAC0;
				
				// Register #25
				s32 MAC1;
				
				// Register #26
				s32 MAC2;
				
				// Register #27
				s32 MAC3;


				// Register #28
				u32 IRGB;	// write only
				
				// Register #29
				u32 ORGB;	// read only
				
				///////////////////////////////////
				// 1, 31, 0 - write only
				// Register #30
				s32 LZCS;

				//////////////////////////////
				// 0, 6, 0 - read only
				// Register #31
				u32 LZCR;
			};
			
		};
		
		// need the indexes of the registers
		enum
		{
			INDEX_VXY0 = 0,
			INDEX_ZeroVZ0,
			INDEX_VXY1,
			INDEX_ZeroVZ1,
			INDEX_VXY2,
			INDEX_ZeroVZ2,
			INDEX_RGB,	// actually stored as BGR
			INDEX_OTZ,
			INDEX_IR0,
			INDEX_IR1,
			INDEX_IR2,
			INDEX_IR3,
			INDEX_SXY0,
			INDEX_SXY1,
			INDEX_SXY2,
			INDEX_SXYP,
			INDEX_SZ0,
			INDEX_SZ1,
			INDEX_SZ2,
			INDEX_SZ3,
			INDEX_RGB0,
			INDEX_RGB1,
			INDEX_RGB2,
			INDEX_Reserved,
			INDEX_MAC0,
			INDEX_MAC1,
			INDEX_MAC2,
			INDEX_MAC3,
			INDEX_IRGB,	// write only
			INDEX_ORGB,	// read only
			INDEX_LZCS,
			INDEX_LZCR
		};
		
		enum
		{
			INDEX_R11R12 = 0,
			INDEX_R13R21,
			INDEX_R22R23,
			INDEX_R31R32,
			INDEX_ZeroR33,
			INDEX_TRX,
			INDEX_TRY,
			INDEX_TRZ,
			INDEX_L11L12,
			INDEX_L13L21,
			INDEX_L22L23,
			INDEX_L31L32,
			INDEX_ZeroL33,
			INDEX_RBK,
			INDEX_GBK,
			INDEX_BBK,
			INDEX_LR1LR2,
			INDEX_LR3LG1,
			INDEX_LG2LG3,
			INDEX_LB1LB2,
			INDEX_ZeroLB3,
			INDEX_RFC,
			INDEX_GFC,
			INDEX_BFC,
			INDEX_OFX,
			INDEX_OFY,
			INDEX_H,
			INDEX_DQA,
			INDEX_DQB,
			INDEX_ZSF3,
			INDEX_ZSF4,
			INDEX_FLAG
		};

		enum
		{
			BITMAP_VXY0 = ( 1 << INDEX_VXY0 ),
			BITMAP_ZeroVZ0 = ( 1 << INDEX_ZeroVZ0 ),
			BITMAP_VXY1 = ( 1 << INDEX_VXY1 ),
			BITMAP_ZeroVZ1 = ( 1 << INDEX_ZeroVZ1 ),
			BITMAP_VXY2 = ( 1 << INDEX_VXY2 ),
			BITMAP_ZeroVZ2 = ( 1 << INDEX_ZeroVZ2 ),
			BITMAP_RGB = ( 1 << INDEX_RGB ),
			BITMAP_OTZ = ( 1 << INDEX_OTZ ),
			BITMAP_IR0 = ( 1 << INDEX_IR0 ),
			BITMAP_IR1 = ( 1 << INDEX_IR1 ),
			BITMAP_IR2 = ( 1 << INDEX_IR2 ),
			BITMAP_IR3 = ( 1 << INDEX_IR3 ),
			BITMAP_SXY0 = ( 1 << INDEX_SXY0 ),
			BITMAP_SXY1 = ( 1 << INDEX_SXY1 ),
			BITMAP_SXY2 = ( 1 << INDEX_SXY2 ),
			BITMAP_SXYP = ( 1 << INDEX_SXYP ),
			BITMAP_SZ0 = ( 1 << INDEX_SZ0 ),
			BITMAP_SZ1 = ( 1 << INDEX_SZ1 ),
			BITMAP_SZ2 = ( 1 << INDEX_SZ2 ),
			BITMAP_SZ3 = ( 1 << INDEX_SZ3 ),
			BITMAP_RGB0 = ( 1 << INDEX_RGB0 ),
			BITMAP_RGB1 = ( 1 << INDEX_RGB1 ),
			BITMAP_RGB2 = ( 1 << INDEX_RGB2 ),
			BITMAP_Reserved = ( 1 << INDEX_Reserved ),
			BITMAP_MAC0 = ( 1 << INDEX_MAC0 ),
			BITMAP_MAC1 = ( 1 << INDEX_MAC1 ),
			BITMAP_MAC2 = ( 1 << INDEX_MAC2 ),
			BITMAP_MAC3 = ( 1 << INDEX_MAC3 ),
			BITMAP_IRGB = ( 1 << INDEX_IRGB ),
			BITMAP_ORGB = ( 1 << INDEX_ORGB ),
			BITMAP_LZCS = ( 1 << INDEX_LZCS ),
			BITMAP_LZCR = ( 1 << INDEX_LZCR )
		};
		
		enum
		{
			BITMAP_R11R12 = ( 1 << INDEX_R11R12 ),
			BITMAP_R13R21 = ( 1 << INDEX_R13R21 ),
			BITMAP_R22R23 = ( 1 << INDEX_R22R23 ),
			BITMAP_R31R32 = ( 1 << INDEX_R31R32 ),
			BITMAP_ZeroR33 = ( 1 << INDEX_ZeroR33 ),
			BITMAP_TRX = ( 1 << INDEX_TRX ),
			BITMAP_TRY = ( 1 << INDEX_TRY ),
			BITMAP_TRZ = ( 1 << INDEX_TRZ ),
			BITMAP_L11L12 = ( 1 << INDEX_L11L12 ),
			BITMAP_L13L21 = ( 1 << INDEX_L13L21 ),
			BITMAP_L22L23 = ( 1 << INDEX_L22L23 ),
			BITMAP_L31L32 = ( 1 << INDEX_L31L32 ),
			BITMAP_ZeroL33 = ( 1 << INDEX_ZeroL33 ),
			BITMAP_RBK = ( 1 << INDEX_RBK ),
			BITMAP_GBK = ( 1 << INDEX_GBK ),
			BITMAP_BBK = ( 1 << INDEX_BBK ),
			BITMAP_LR1LR2 = ( 1 << INDEX_LR1LR2 ),
			BITMAP_LR3LG1 = ( 1 << INDEX_LR3LG1 ),
			BITMAP_LG2LG3 = ( 1 << INDEX_LG2LG3 ),
			BITMAP_LB1LB2 = ( 1 << INDEX_LB1LB2 ),
			BITMAP_ZeroLB3 = ( 1 << INDEX_ZeroLB3 ),
			BITMAP_RFC = ( 1 << INDEX_RFC ),
			BITMAP_GFC = ( 1 << INDEX_GFC ),
			BITMAP_BFC = ( 1 << INDEX_BFC ),
			BITMAP_OFX = ( 1 << INDEX_OFX ),
			BITMAP_OFY = ( 1 << INDEX_OFY ),
			BITMAP_H = ( 1 << INDEX_H ),
			BITMAP_DQA = ( 1 << INDEX_DQA ),
			BITMAP_DQB = ( 1 << INDEX_DQB ),
			BITMAP_ZSF3 = ( 1 << INDEX_ZSF3 ),
			BITMAP_ZSF4 = ( 1 << INDEX_ZSF4 ),
			BITMAP_FLAG = ( 1 << INDEX_FLAG )
		};
		
		
		// constructor
		COP2_Device ();
		
		void Start ();
		
		void Reset ();
		
		void Write_MTC ( u32 Register, u32 Value );
		void Write_CTC ( u32 Register, u32 Value );
		u32 Read_MFC ( u32 Register );
		u32 Read_CFC ( u32 Register );
		
		static void Generate_unr_table ();
		u32 GTE_Divide ( u32 H, u32 SZ );

		static u8 unr_table [ 257 ];
		
		COP2_ControlRegs CPC2;
		COP2_DataRegs CPR2;
		
		u32 CPRLoading_Bitmap;
		
		u32 BusyCycles;
		
		// fields
		//u32 lm, cv, v, mx, sf;
		
		// temp variables for intermediate steps
		u64 quotient;
		
		////////////////////////////////////////////////////////
		// COP2 Functions are (lower 6 bits of code):
		// 0x01 - RTPS
		// 0x06 - NCLIP
		// 0x0c - OP
		// 0x10 - DPCS
		// 0x11 - INTPL
		// 0x12 - MVMVA
		// 0x13 - NCDS
		// 0x14 - CDP
		// 0x16 - NCDT
		// 0x1b - NCCS
		// 0x1c - CC
		// 0x1e - NCS
		// 0x20 - NCT
		// 0x28 - SQR
		// 0x29 - DCPL
		// 0x2a - DPCT
		// 0x2d - AVSZ3
		// 0x2e - AVSZ4
		// 0x30 - RTPT
		// 0x3d - GPF
		// 0x3e - GPL
		// 0x3f - NCCT
		
		void RTPS ( Cpu* r, Instruction::Format i );
		void NCLIP ( Cpu* r, Instruction::Format i );
		void OP ( Cpu* r, Instruction::Format i );
		void DPCS ( Cpu* r, Instruction::Format i );
		void INTPL ( Cpu* r, Instruction::Format i );
		void MVMVA ( Cpu* r, Instruction::Format i );
		void NCDS ( Cpu* r, Instruction::Format i );
		void CDP ( Cpu* r, Instruction::Format i );
		void NCDT ( Cpu* r, Instruction::Format i );
		void NCCS ( Cpu* r, Instruction::Format i );
		void CC ( Cpu* r, Instruction::Format i );
		void NCS ( Cpu* r, Instruction::Format i );
		void NCT ( Cpu* r, Instruction::Format i );
		void SQR ( Cpu* r, Instruction::Format i );
		void DCPL ( Cpu* r, Instruction::Format i );
		void DPCT ( Cpu* r, Instruction::Format i );
		void AVSZ3 ( Cpu* r, Instruction::Format i );
		void AVSZ4 ( Cpu* r, Instruction::Format i );
		void RTPT ( Cpu* r, Instruction::Format i );
		void GPF ( Cpu* r, Instruction::Format i );
		void GPL ( Cpu* r, Instruction::Format i );
		void NCCT ( Cpu* r, Instruction::Format i );
		
		// checks if value is greater than 43 bits and positive or larger than 43 bits and negative
		// if it is, then it sets the flag
		// if Lim is true, then we must also restrict value to the range
		// also returns original value
		// Number can be 1,2, or 3, depending on whether it is A1P,A2P,A3P
		// must return 64-bit signed
		// no, actually this clamps to 32-bits and return 32-bit signed or else calculations can overflow into the sign bit
		// so it needs to take in 64-bits but clamp to 32-bits - thats what it must be doing
		/*
		inline s64 A ( const bool Lim, const int Number, s64 Value )
		{
			if ( Value > 0x7ffffffffffLL )
			{
				// mark flag
				CPC2.FLAG.Value |= ( ( (u32) 1 << 31 ) >> Number ) | ( 1 << 31 );
				
				if ( Lim ) return 0x7ffffffffffLL;
			}
			else if ( Value < -0x80000000000LL )
			{
				// mark flag
				CPC2.FLAG.Value |= ( ( (u32) 1 << 28 ) >> Number ) | ( 1 << 31 );
				
				if ( Lim ) return -0x80000000000LL;
			}
			
			return Value;
		}
		*/
		
		inline s64 A64 ( const bool Lim, const int Number, s64 Value )
		{
			if ( Value > 0x7ffffffffffLL )
			{
				// mark flag
				CPC2.FLAG.Value |= ( ( ( (u32) 1 ) << 31 ) >> Number ) | ( 1 << 31 );
				
				if ( Lim ) return 0x7ffffffffffLL;
			}
			else if ( Value < -0x80000000000LL )
			{
				// mark flag
				CPC2.FLAG.Value |= ( ( 1 << 28 ) >> Number ) | ( 1 << 31 );
				
				if ( Lim ) return -0x80000000000LL;
			}
			
			// clip value to 44-bits
			return ( ( Value << 20 ) >> 20 );
		}
		
		inline s64 A64_1 ( s64 Value ) { return A64 ( false, 1, Value ); }
		inline s64 A64_2 ( s64 Value ) { return A64 ( false, 2, Value ); }
		inline s64 A64_3 ( s64 Value ) { return A64 ( false, 3, Value ); }
		
		inline s32 A ( const bool Lim, const int Number, s64 Value )
		{
			if ( Value > 0x7fffffffLL )
			{
				// mark flag
				CPC2.FLAG.Value |= ( ( ( (u32) 1 ) << 31 ) >> Number ) | ( 1 << 31 );
				
				if ( Lim ) return 0x7fffffffLL;
			}
			else if ( Value < -0x80000000LL )
			{
				// mark flag
				CPC2.FLAG.Value |= ( ( 1 << 28 ) >> Number ) | ( 1 << 31 );
				
				if ( Lim ) return -0x80000000LL;
			}
			
			return Value;
		}

		/*
		inline s64 A1 ( s64 Value ) { return A ( false, 1, Value ); }
		inline s64 A2 ( s64 Value ) { return A ( false, 2, Value ); }
		inline s64 A3 ( s64 Value ) { return A ( false, 3, Value ); }
		inline s64 Lm_A1 ( s64 Value ) { return A ( true, 1, Value ); }
		inline s64 Lm_A2 ( s64 Value ) { return A ( true, 2, Value ); }
		inline s64 Lm_A3 ( s64 Value ) { return A ( true, 3, Value ); }
		*/

		inline s32 A1 ( s64 Value ) { return A ( false, 1, Value ); }
		inline s32 A2 ( s64 Value ) { return A ( false, 2, Value ); }
		inline s32 A3 ( s64 Value ) { return A ( false, 3, Value ); }
		inline s32 Lm_A1 ( s64 Value ) { return A ( true, 1, Value ); }
		inline s32 Lm_A2 ( s64 Value ) { return A ( true, 2, Value ); }
		inline s32 Lm_A3 ( s64 Value ) { return A ( true, 3, Value ); }
		
		// check if Value negative(lm=1) or larger than 15 bits(lm=0)
		// *** todo *** B is always Lim, so we can optimize this
		inline s64 B ( const bool Lim, const int Number, s64 Value, u32 _lm, u32 _MarkFlag = 1 )
		{
			if ( !_lm )
			{
				// lm == 0
				
				if ( Value > 0x7fffLL )
				{
					// note: need _MarkFlag since if sf=0 then B3 only saturates for RTP if MAC3>>12 saturates according to martin psx spec
					if ( _MarkFlag )
					{
						// mark flag
						// *note* do not mark checksum for B3
						CPC2.FLAG.Value |= ( ( (u32) 1 << 25 ) >> Number );
						
						if ( Number != 3 ) CPC2.FLAG.Value |= ( 1 << 31 );
					}
					
					if ( Lim ) return 0x7fffLL;
				}
				else if ( Value < -0x8000LL )
				{
					if ( _MarkFlag )
					{
						// mark flag
						// *note* do not mark checksum for B3
						CPC2.FLAG.Value |= ( ( (u32) 1 << 25 ) >> Number );
						
						if ( Number != 3 ) CPC2.FLAG.Value |= ( 1 << 31 );
					}
					
					if ( Lim ) return -0x8000LL;
				}
				
			}
			else
			{
				// lm == 1
				
				if ( Value > 0x7fffLL )
				{
					if ( _MarkFlag )
					{
						// mark flag
						// *note* do not mark checksum for B3
						CPC2.FLAG.Value |= ( ( (u32) 1 << 25 ) >> Number );
						
						if ( Number != 3 ) CPC2.FLAG.Value |= ( 1 << 31 );
					}
					
					if ( Lim ) return 0x7fffLL;
				}
				else if ( Value < 0 )
				{
					if ( _MarkFlag )
					{
						// mark flag
						// *note* do not mark checksum for B3
						CPC2.FLAG.Value |= ( ( (u32) 1 << 25 ) >> Number );
						
						if ( Number != 3 ) CPC2.FLAG.Value |= ( 1 << 31 );
					}
					
					if ( Lim ) return 0;
				}
			}
			
			return Value;
			
		}

		inline s64 B1 ( s64 Value, u32 _lm ) { return B ( false, 1, Value, _lm ); }
		inline s64 B2 ( s64 Value, u32 _lm ) { return B ( false, 2, Value, _lm ); }
		inline s64 B3 ( s64 Value, u32 _lm ) { return B ( false, 3, Value, _lm ); }
		inline s16 Lm_B1 ( s64 Value, u32 _lm ) { return B ( true, 1, Value, _lm ); }
		inline s16 Lm_B2 ( s64 Value, u32 _lm ) { return B ( true, 2, Value, _lm ); }
		inline s16 Lm_B3 ( s64 Value, u32 _lm, u32 _MarkFlag = 1 ) { return B ( true, 3, Value, _lm, _MarkFlag ); }

		// check if Value negative or larger than 8 bits
		// *** todo *** C is always Lim
		inline s64 C ( const bool Lim, const int Number, s64 Value )
		{
			if ( Value > 0xffLL )
			{
				// mark flag
				CPC2.FLAG.Value |= ( ( 1 << 22 ) >> Number );
				
				if ( Lim ) return 0xffLL;
			}
			else if ( Value < 0 )
			{
				// mark flag
				CPC2.FLAG.Value |= ( ( 1 << 22 ) >> Number );
				
				if ( Lim ) return 0;
			}
			
			return Value;
		}

		inline s64 C1 ( s64 Value ) { return C ( false, 1, Value ); }
		inline s64 C2 ( s64 Value ) { return C ( false, 2, Value ); }
		inline s64 C3 ( s64 Value ) { return C ( false, 3, Value ); }
		inline s64 Lm_C1 ( s64 Value ) { return C ( true, 1, Value ); }
		inline s64 Lm_C2 ( s64 Value ) { return C ( true, 2, Value ); }
		inline s64 Lm_C3 ( s64 Value ) { return C ( true, 3, Value ); }

		// check if Value negative or larger than 16 bits
		inline s64 D ( const bool Lim, s32 Value )
		{
			if ( Value > 0xffff )
			{
				// mark flag
				CPC2.FLAG.Value |= ( 1 << 18 ) | ( 1 << 31 );
				
				if ( Lim ) return 0xffff;
			}
			else if ( Value < 0 )
			{
				// mark flag
				CPC2.FLAG.Value |= ( 1 << 18 ) | ( 1 << 31 );
				
				if ( Lim ) return 0;
			}
			
			return Value;
		}

		inline s32 D ( s32 Value ) { return D ( false, Value ); }
		inline s32 Lm_D ( s32 Value ) { return D ( true, Value ); }

		// check if Divide overflow. (quotient > 2.0)
		// *** todo ***
		inline u64 E ( const bool Lim, u64 Value )
		{
			if ( Value > 0x1ffff )
			{
				CPC2.FLAG.Value |= ( 1 << 31 ) | ( 1 << 17 );
				return 0x1ffff;
			}
			
			return Value;
		}

		inline u64 E ( u64 Value ) { return E ( false, Value ); }
		inline u64 Lm_E ( u64 Value ) { return E ( true, Value ); }

		// check if Result larger than 31 bits and positive or larger than 31 bits and negative
		inline s64 F ( const bool Lim, s64 Value )
		{
			if ( Value > 0x7fffffffLL )
			{
				// mark flag
				CPC2.FLAG.Value |= ( 1 << 16 ) | ( 1 << 31 );
				
				if ( Lim ) return 0x7fffffffLL;
			}
			else if ( Value < -0x80000000LL )
			{
				// mark flag
				CPC2.FLAG.Value |= ( 1 << 15 ) | ( 1 << 31 );
				
				if ( Lim ) return -0x80000000LL;
			}
			
			return Value;
		}

		inline s64 F ( s64 Value ) { return F ( false, Value ); }
		inline s64 Lm_F ( s64 Value ) { return F ( true, Value ); }


		// check if Value larger than 10 bits
		// Number can be 1,2, or 3, depending on whether it is G1,G2
		inline s64 G ( const bool Lim, const int Number, s64 Value )
		{
			if ( Value > 0x3ffLL )
			{
				// mark flag
				CPC2.FLAG.Value |= ( ( 1 << 15 ) >> Number ) | ( 1 << 31 );
				
				if ( Lim ) return 0x3ff;
			}
			else if ( Value < -0x400LL )
			{
				// mark flag
				CPC2.FLAG.Value |= ( ( 1 << 15 ) >> Number ) | ( 1 << 31 );
				
				if ( Lim ) return -0x400LL;
			}
			
			return Value;
		}

		inline s64 G1 ( s64 Value ) { return G ( false, 1, Value ); }
		inline s64 G2 ( s64 Value ) { return G ( false, 2, Value ); }
		inline s64 Lm_G1 ( s64 Value ) { return G ( true, 1, Value ); }
		inline s64 Lm_G2 ( s64 Value ) { return G ( true, 2, Value ); }

		// check if Value negative or larger than 12 bits
		inline s64 H ( const bool Lim, s64 Value )
		{
			// mame says the upper bound is 0xfff, but pcsx says the upper bound is 0x1000
			// the upper bound is 0x1000
			
			if ( Value > 0x1000 )
			{
				// mark flag
				// *important* DO NOT MARK CHECKSUM FOR H
				CPC2.FLAG.Value |= ( 1 << 12 );
				
				if ( Lim ) return 0x1000;
			}
			else if ( Value < 0 )
			{
				// mark flag
				// *important* DO NOT MARK CHECKSUM FOR H
				CPC2.FLAG.Value |= ( 1 << 12 );
				
				if ( Lim ) return 0;
			}
			
			return Value;
		}

		inline s64 H ( s64 Value ) { return H ( false, Value ); }
		inline s64 Lm_H ( s64 Value ) { return H ( true, Value ); }
		
		s32 Zero_Vector [ 4 ];	// = { 0, 0, 0, 0 };
		
		
		// signed 16-bit values
		static s16* Matrix_Picker [ 4 ];	// = { &CPC2.R11, &CPC2.L11, &CPC2.LR1, &CPC2.LR1 };
		
		// signed 16-bit values
		static s16* Vector_Picker [ 4 ];	// = { &CPR2.VX0, &CPR2.VX1, &CPR2.VX2, &CPR2.VX2 };
		
		// signed 32-bit values
		static s32* CVector_Picker [ 4 ];	// = { &CPC2.TRX, &CPC2.RBK, &CPC2.RFC, Zero_Vector };
	};

}

#endif


