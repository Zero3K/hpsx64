
#include "MipsOpcode.h"
#include "R3000A_Lookup.h"

using namespace R3000A;
using namespace R3000A::Instruction;


bool Lookup::c_bObjectInitialized = false;

alignas(32) u8 Lookup::LookupTable [ c_iLookupTable_Size ];


// in format: instruction name, opcode, rs, funct, rt
Instruction::Entry Instruction::Lookup::Entries [] = {
{ "BLTZ",		0x1,		ANY_VALUE,		0x0,			ANY_VALUE,		IDX_BLTZ },
{ "BGEZ",		0x1,		ANY_VALUE,		0x1,			ANY_VALUE,		IDX_BGEZ },
{ "BLTZAL",		0x1,		ANY_VALUE,		0x10,			ANY_VALUE,		IDX_BLTZAL },
{ "BGEZAL",		0x1,		ANY_VALUE,		0x11,			ANY_VALUE,		IDX_BGEZAL },
{ "BEQ",		0x4,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_BEQ },
{ "BNE",		0x5,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_BNE },
{ "BLEZ",		0x6,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_BLEZ },
{ "BGTZ",		0x7,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_BGTZ },
{ "J",			0x2,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_J },
{ "JAL",		0x3,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_JAL },
{ "JR",			0x0,		ANY_VALUE,		ANY_VALUE,		0x8,			IDX_JR },
{ "JALR", 		0x0,		ANY_VALUE,		ANY_VALUE,		0x9,			IDX_JALR },
{ "LB",			0x20,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LB },
{ "LH",			0x21,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LH },
{ "LWL",		0x22,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LWL },
{ "LW",			0x23,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LW },
{ "LBU",		0x24,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LBU },
{ "LHU",		0x25,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LHU },
{ "LWR",		0x26,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LWR },
{ "SB",			0x28,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SB },
{ "SH",			0x29,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SH },
{ "SWL",		0x2a,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SWL },
{ "SW",			0x2b,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SW },
{ "SWR",		0x2e,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SWR },
{ "LWC2",		0x32,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LWC2 },
{ "SWC2",		0x3a,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SWC2 },
{ "ADDI",		0x8,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_ADDI },
{ "ADDIU",		0x9,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_ADDIU },
{ "SLTI",		0xa,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SLTI },
{ "SLTIU",		0xb,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SLTIU },
{ "ANDI",		0xc,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_ANDI },
{ "ORI",		0xd,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_ORI },
{ "XORI",		0xe,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_XORI },
{ "LUI",		0xf,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LUI },
{ "SLL",		0x0,		ANY_VALUE,		ANY_VALUE,		0x0,			IDX_SLL },
{ "SRL",		0x0,		ANY_VALUE,		ANY_VALUE,		0x2,			IDX_SRL },
{ "SRA",		0x0,		ANY_VALUE,		ANY_VALUE,		0x3,			IDX_SRA },
{ "SLLV",		0x0,		ANY_VALUE,		ANY_VALUE,		0x4,			IDX_SLLV },
{ "SRLV",		0x0,		ANY_VALUE,		ANY_VALUE,		0x6,			IDX_SRLV },
{ "SRAV",		0x0,		ANY_VALUE,		ANY_VALUE,		0x7,			IDX_SRAV },
{ "SYSCALL",	0x0,		ANY_VALUE,		ANY_VALUE,		0xc,			IDX_SYSCALL },
{ "BREAK",		0x0,		ANY_VALUE,		ANY_VALUE,		0xd,			IDX_BREAK },
{ "MFHI",		0x0,		ANY_VALUE,		ANY_VALUE,		0x10,			IDX_MFHI },
{ "MTHI",		0x0,		ANY_VALUE,		ANY_VALUE,		0x11,			IDX_MTHI },
{ "MFLO",		0x0,		ANY_VALUE,		ANY_VALUE,		0x12,			IDX_MFLO },
{ "MTLO",		0x0,		ANY_VALUE,		ANY_VALUE,		0x13,			IDX_MTLO },
{ "MULT",		0x0,		ANY_VALUE,		ANY_VALUE,		0x18,			IDX_MULT },
{ "MULTU",		0x0,		ANY_VALUE,		ANY_VALUE,		0x19,			IDX_MULTU },
{ "DIV",		0x0,		ANY_VALUE,		ANY_VALUE,		0x1a,			IDX_DIV },
{ "DIVU",		0x0,		ANY_VALUE,		ANY_VALUE,		0x1b,			IDX_DIVU },
{ "ADD",		0x0,		ANY_VALUE,		ANY_VALUE,		0x20,			IDX_ADD },
{ "ADDU",		0x0,		ANY_VALUE,		ANY_VALUE,		0x21,			IDX_ADDU },
{ "SUB",		0x0,		ANY_VALUE,		ANY_VALUE,		0x22,			IDX_SUB },
{ "SUBU",		0x0,		ANY_VALUE,		ANY_VALUE,		0x23,			IDX_SUBU },
{ "AND",		0x0,		ANY_VALUE,		ANY_VALUE,		0x24,			IDX_AND },
{ "OR",			0x0,		ANY_VALUE,		ANY_VALUE,		0x25,			IDX_OR },
{ "XOR",		0x0,		ANY_VALUE,		ANY_VALUE,		0x26,			IDX_XOR },
{ "NOR",		0x0,		ANY_VALUE,		ANY_VALUE,		0x27,			IDX_NOR },
{ "SLT",		0x0,		ANY_VALUE,		ANY_VALUE,		0x2a,			IDX_SLT },
{ "SLTU",		0x0,		ANY_VALUE,		ANY_VALUE,		0x2b,			IDX_SLTU },
{ "MFC0",		0x10,		0x0,			ANY_VALUE,		ANY_VALUE,		IDX_MFC0 },
{ "MTC0",		0x10,		0x4,			ANY_VALUE,		ANY_VALUE,		IDX_MTC0 },
{ "MFC2",		0x12,		0x0,			ANY_VALUE,		ANY_VALUE,		IDX_MFC2 },
{ "CFC2",		0x12,		0x2,			ANY_VALUE,		ANY_VALUE,		IDX_CFC2 },
{ "MTC2",		0x12,		0x4,			ANY_VALUE,		ANY_VALUE,		IDX_MTC2 },
{ "CTC2",		0x12,		0x6,			ANY_VALUE,		ANY_VALUE,		IDX_CTC2 },


{ "RFE",		0x10,		0x10,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x11,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x12,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x13,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x14,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x15,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x16,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x17,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x18,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x19,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x1a,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x1b,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x1c,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x1d,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x1e,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x1f,			ANY_VALUE,		0x10,			IDX_RFE },

// *** COP 2 Instructions ***

{ "RTPS",		0x12,		ANY_VALUE,		ANY_VALUE,		0x1,			IDX_RTPS },
{ "NCLIP",		0x12,		ANY_VALUE,		ANY_VALUE,		0x6,			IDX_NCLIP },
{ "OP",			0x12,		ANY_VALUE,		ANY_VALUE,		0xc,			IDX_OP },
{ "DPCS",		0x12,		ANY_VALUE,		ANY_VALUE,		0x10,			IDX_DPCS },
{ "INTPL",		0x12,		ANY_VALUE,		ANY_VALUE,		0x11,			IDX_INTPL },
{ "MVMVA",		0x12,		ANY_VALUE,		ANY_VALUE,		0x12,			IDX_MVMVA },
{ "NCDS",		0x12,		ANY_VALUE,		ANY_VALUE,		0x13,			IDX_NCDS },
{ "CDP",		0x12,		ANY_VALUE,		ANY_VALUE,		0x14,			IDX_CDP },
{ "NCDT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x16,			IDX_NCDT },
{ "NCCS",		0x12,		ANY_VALUE,		ANY_VALUE,		0x1b,			IDX_NCCS },
{ "CC",			0x12,		ANY_VALUE,		ANY_VALUE,		0x1c,			IDX_CC },
{ "NCS",		0x12,		ANY_VALUE,		ANY_VALUE,		0x1e,			IDX_NCS },
{ "NCT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x20,			IDX_NCT },
{ "SQR",		0x12,		ANY_VALUE,		ANY_VALUE,		0x28,			IDX_SQR },
{ "DCPL",		0x12,		ANY_VALUE,		ANY_VALUE,		0x29,			IDX_DCPL },
{ "DPCT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x2a,			IDX_DPCT },
{ "AVSZ3",		0x12,		ANY_VALUE,		ANY_VALUE,		0x2d,			IDX_AVSZ3 },
{ "AVSZ4",		0x12,		ANY_VALUE,		ANY_VALUE,		0x2e,			IDX_AVSZ4 },
{ "RTPT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x30,			IDX_RTPT },
{ "GPF",		0x12,		ANY_VALUE,		ANY_VALUE,		0x3d,			IDX_GPF },
{ "GPL",		0x12,		ANY_VALUE,		ANY_VALUE,		0x3e,			IDX_GPL },
{ "NCCT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x3f,			IDX_NCCT }
};






//Debug::Log Lookup::debug;


void Lookup::Start ()
{
	u32 Opcode, Rs, Funct, Rt, Index, ElementsInExecute, ElementsInBranchLoad1;
	Instruction::Format i;
	
	u32 ulCounter, ulRemainder;

	if ( c_bObjectInitialized ) return;
	
	cout << "Running constructor for Execute class.\n";

#ifdef INLINE_DEBUG_ENABLE	
	debug.Create ( "R3000A_Execute_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "Running constructor for Execute class.\r\n";
#endif

	
	ElementsInExecute = (sizeof(Entries) / sizeof(Entries[0]));
	
	// clear table first
	cout << "\nSize of R3000A lookup table in bytes=" << dec << sizeof( LookupTable );
	for ( Index = 0; Index < ( sizeof( LookupTable ) >> 3 ); Index++ ) ((u64*)LookupTable) [ Index ] = 0;
	
	for ( Index = ElementsInExecute - 1; Index < ElementsInExecute; Index-- )
	{
		ulRemainder = 0;
		
		Opcode = Entries [ Index ].Opcode;
		Rs = Entries [ Index ].Rs;
		Rt = Entries [ Index ].Rt;
		//Shft = Entries [ Index ].Shift;
		Funct = Entries [ Index ].Funct;
		
		i.Opcode = Opcode;
		i.Rs = Rs;
		i.Rt = Rt;
		//i.Shift = Shft;
		i.Funct = Funct;
		
		for ( ulCounter = 0; ulRemainder == 0; ulCounter++ )
		{
			ulRemainder = ulCounter;
			
			if ( Opcode == 0xff )
			{
				// 6 bits
				i.Opcode = ulRemainder & 0x3f;
				ulRemainder >>= 6;
			}
			
			if ( Rs == 0xff )
			{
				// 5 bits
				i.Rs = ulRemainder & 0x1f;
				ulRemainder >>= 5;
			}
			
			if ( Rt == 0xff )
			{
				// 5 bits
				i.Rt = ulRemainder & 0x1f;
				ulRemainder >>= 5;
			}
			
			//if ( Shft == 0xff )
			//{
			//	// 5 bits
			//	i.Shift = ulRemainder & 0x1f;
			//	ulRemainder >>= 5;
			//}
			
			if ( Funct == 0xff )
			{
				// 6 bits
				i.Funct = ulRemainder & 0x3f;
				ulRemainder >>= 6;
			}
			
			LookupTable [ ( ( i.Value >> 16 ) | ( i.Value << 16 ) ) & c_iLookupTable_Mask ] = Entries [ Index ].InstructionIndex;
		}
	}
	
	/*
	for ( Opcode = 0; Opcode < c_iOpcode_MaxValue; Opcode++ )
	{
		for ( Rs = 0; Rs < c_iRs_MaxValue; Rs++ )
		{
			for ( Funct = 0; Funct < c_iFunct_MaxValue; Funct++ )
			{
				for ( Rt = 0; Rt < c_iRt_MaxValue; Rt++ )
				{
					i.Opcode = Opcode;
					i.Rs = Rs;
					i.Funct = Funct;
					i.Rt = Rt;
				
					// initialize entry in LUT to Invalid instruction
					//LookupTable [ ( ( i.Value >> 16 ) | ( i.Value << 16 ) ) & c_iLookupTable_Mask ] = &Execute::Invalid;
					LookupTable [ ( ( i.Value >> 16 ) | ( i.Value << 16 ) ) & c_iLookupTable_Mask ] = Lookup::IDX_INVALID;
					
					// lookup entry in list of instructions
					for ( Index = 0; Index < ElementsInExecute; Index++ )
					{
						// check if we have found the instruction to insert into current position of LUT
						if ( ( Entries [ Index ].Opcode == Opcode || Entries [ Index ].Opcode == ANY_VALUE )
						&& ( Entries [ Index ].Rs == Rs || Entries [ Index ].Rs == ANY_VALUE )
						&& ( Entries [ Index ].Funct == Funct || Entries [ Index ].Funct == ANY_VALUE )
						&& ( Entries [ Index ].Rt == Rt || Entries [ Index ].Rt == ANY_VALUE ) )
						{
							// enter function for entry into LUT
							//LookupTable [ ( ( i.Value >> 16 ) | ( i.Value << 16 ) ) & c_iLookupTable_Mask ] = Entries [ Index ].FunctionToCall;
							LookupTable [ ( ( i.Value >> 16 ) | ( i.Value << 16 ) ) & c_iLookupTable_Mask ] = Entries [ Index ].InstructionIndex;
							
							//break;
						}

					}
				
				}
				
			}
			
		}
		
	}
	*/
	
	
	// object has now been fully initilized
	c_bObjectInitialized = true;
}
