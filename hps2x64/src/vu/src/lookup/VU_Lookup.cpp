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


#include "VU_Lookup.h"

using namespace Vu;
using namespace Vu::Instruction;


bool Lookup::c_bObjectInitialized = false;


u8 Lookup::LookupTableLO [ c_iLookupTableLO_Size ];
u8 Lookup::LookupTableHI [ c_iLookupTableHI_Size ];



// in format: instruction name, opcode, rs, rt, shift, funct, index/id
const Instruction::Entry Instruction::Lookup::EntriesHI [] = {

// upper instructions //

{ "ABS",		ANY_VALUE,		0x7,			0x3d,			IDX_ABS },

{ "ADD",		ANY_VALUE,		ANY_VALUE,		0x28,			IDX_ADD },
{ "ADDi",		ANY_VALUE,		ANY_VALUE,		0x22,			IDX_ADDi },
{ "ADDq",		ANY_VALUE,		ANY_VALUE,		0x20,			IDX_ADDq },
{ "ADDX",		ANY_VALUE,		ANY_VALUE,		0x00,			IDX_ADDBCX },
{ "ADDY",		ANY_VALUE,		ANY_VALUE,		0x01,			IDX_ADDBCY },
{ "ADDZ",		ANY_VALUE,		ANY_VALUE,		0x02,			IDX_ADDBCZ },
{ "ADDW",		ANY_VALUE,		ANY_VALUE,		0x03,			IDX_ADDBCW },

{ "ADDA",		ANY_VALUE,		0x0a,			0x3c,			IDX_ADDA },
{ "ADDAi",		ANY_VALUE,		0x08,			0x3e,			IDX_ADDAi },
{ "ADDAq",		ANY_VALUE,		0x08,			0x3c,			IDX_ADDAq },
{ "ADDAX",		ANY_VALUE,		0x00,			0x3c,			IDX_ADDABCX },
{ "ADDAY",		ANY_VALUE,		0x00,			0x3d,			IDX_ADDABCY },
{ "ADDAZ",		ANY_VALUE,		0x00,			0x3e,			IDX_ADDABCZ },
{ "ADDAW",		ANY_VALUE,		0x00,			0x3f,			IDX_ADDABCW },

//{ "CALLMS",	0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x38,			IDX_CALLMS },
//{ "CALLMSR",	0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x39,			IDX_CALLMSR },

{ "CLIP",		ANY_VALUE,		0x07,			0x3f,			IDX_CLIP },

{ "FTOI0",		ANY_VALUE,		0x05,			0x3c,			IDX_FTOI0 },
{ "FTOI4",		ANY_VALUE,		0x05,			0x3d,			IDX_FTOI4 },
{ "FTOI12",		ANY_VALUE,		0x05,			0x3e,			IDX_FTOI12 },
{ "FTOI15",		ANY_VALUE,		0x05,			0x3f,			IDX_FTOI15 },


{ "ITOF0",		ANY_VALUE,		0x04,			0x3c,			IDX_ITOF0 },
{ "ITOF4",		ANY_VALUE,		0x04,			0x3d,			IDX_ITOF4 },
{ "ITOF12",		ANY_VALUE,		0x04,			0x3e,			IDX_ITOF12 },
{ "ITOF15",		ANY_VALUE,		0x04,			0x3f,			IDX_ITOF15 },


{ "MADD",		ANY_VALUE,		ANY_VALUE,		0x29,			IDX_MADD },
{ "MADDi",		ANY_VALUE,		ANY_VALUE,		0x23,			IDX_MADDi },
{ "MADDq",		ANY_VALUE,		ANY_VALUE,		0x21,			IDX_MADDq },
{ "MADDX",		ANY_VALUE,		ANY_VALUE,		0x08,			IDX_MADDBCX },
{ "MADDY",		ANY_VALUE,		ANY_VALUE,		0x09,			IDX_MADDBCY },
{ "MADDZ",		ANY_VALUE,		ANY_VALUE,		0x0a,			IDX_MADDBCZ },
{ "MADDW",		ANY_VALUE,		ANY_VALUE,		0x0b,			IDX_MADDBCW },

{ "MADDA",		ANY_VALUE,		0x0a,			0x3d,			IDX_MADDA },
{ "MADDAi",		ANY_VALUE,		0x08,			0x3f,			IDX_MADDAi },
{ "MADDAq",		ANY_VALUE,		0x08,			0x3d,			IDX_MADDAq },
{ "MADDAX",		ANY_VALUE,		0x02,			0x3c,			IDX_MADDABCX },
{ "MADDAY",		ANY_VALUE,		0x02,			0x3d,			IDX_MADDABCY },
{ "MADDAZ",		ANY_VALUE,		0x02,			0x3e,			IDX_MADDABCZ },
{ "MADDAW",		ANY_VALUE,		0x02,			0x3f,			IDX_MADDABCW },

{ "MAX",		ANY_VALUE,		ANY_VALUE,		0x2b,			IDX_MAX },
{ "MAXi",		ANY_VALUE,		ANY_VALUE,		0x1d,			IDX_MAXi },
{ "MAXX",		ANY_VALUE,		ANY_VALUE,		0x10,			IDX_MAXBCX },
{ "MAXY",		ANY_VALUE,		ANY_VALUE,		0x11,			IDX_MAXBCY },
{ "MAXZ",		ANY_VALUE,		ANY_VALUE,		0x12,			IDX_MAXBCZ },
{ "MAXW",		ANY_VALUE,		ANY_VALUE,		0x13,			IDX_MAXBCW },


{ "MINI",		ANY_VALUE,		ANY_VALUE,		0x2f,			IDX_MINI },
{ "MINIi",		ANY_VALUE,		ANY_VALUE,		0x1f,			IDX_MINIi },
{ "MINIX",		ANY_VALUE,		ANY_VALUE,		0x14,			IDX_MINIBCX },
{ "MINIY",		ANY_VALUE,		ANY_VALUE,		0x15,			IDX_MINIBCY },
{ "MINIZ",		ANY_VALUE,		ANY_VALUE,		0x16,			IDX_MINIBCZ },
{ "MINIW",		ANY_VALUE,		ANY_VALUE,		0x17,			IDX_MINIBCW },


{ "MSUB",		ANY_VALUE,		ANY_VALUE,		0x2d,			IDX_MSUB },
{ "MSUBi",		ANY_VALUE,		ANY_VALUE,		0x27,			IDX_MSUBi },
{ "MSUBq",		ANY_VALUE,		ANY_VALUE,		0x25,			IDX_MSUBq },
{ "MSUBX",		ANY_VALUE,		ANY_VALUE,		0x0c,			IDX_MSUBBCX },
{ "MSUBY",		ANY_VALUE,		ANY_VALUE,		0x0d,			IDX_MSUBBCY },
{ "MSUBZ",		ANY_VALUE,		ANY_VALUE,		0x0e,			IDX_MSUBBCZ },
{ "MSUBW",		ANY_VALUE,		ANY_VALUE,		0x0f,			IDX_MSUBBCW },

{ "MSUBA",		ANY_VALUE,		0x0b,			0x3d,			IDX_MSUBA },
{ "MSUBAi",		ANY_VALUE,		0x09,			0x3f,			IDX_MSUBAi },
{ "MSUBAq",		ANY_VALUE,		0x09,			0x3d,			IDX_MSUBAq },
{ "MSUBAX",		ANY_VALUE,		0x03,			0x3c,			IDX_MSUBABCX },
{ "MSUBAY",		ANY_VALUE,		0x03,			0x3d,			IDX_MSUBABCY },
{ "MSUBAZ",		ANY_VALUE,		0x03,			0x3e,			IDX_MSUBABCZ },
{ "MSUBAW",		ANY_VALUE,		0x03,			0x3f,			IDX_MSUBABCW },


{ "MUL",		ANY_VALUE,		ANY_VALUE,		0x2a,			IDX_MUL },
{ "MULi",		ANY_VALUE,		ANY_VALUE,		0x1e,			IDX_MULi },
{ "MULq",		ANY_VALUE,		ANY_VALUE,		0x1c,			IDX_MULq },
{ "MULX",		ANY_VALUE,		ANY_VALUE,		0x18,			IDX_MULBCX },
{ "MULY",		ANY_VALUE,		ANY_VALUE,		0x19,			IDX_MULBCY },
{ "MULZ",		ANY_VALUE,		ANY_VALUE,		0x1a,			IDX_MULBCZ },
{ "MULW",		ANY_VALUE,		ANY_VALUE,		0x1b,			IDX_MULBCW },

{ "MULA",		ANY_VALUE,		0x0a,			0x3e,			IDX_MULA },
{ "MULAi",		ANY_VALUE,		0x07,			0x3e,			IDX_MULAi },
{ "MULAq",		ANY_VALUE,		0x07,			0x3c,			IDX_MULAq },
{ "MULAX",		ANY_VALUE,		0x06,			0x3c,			IDX_MULABCX },
{ "MULAY",		ANY_VALUE,		0x06,			0x3d,			IDX_MULABCY },
{ "MULAZ",		ANY_VALUE,		0x06,			0x3e,			IDX_MULABCZ },
{ "MULAW",		ANY_VALUE,		0x06,			0x3f,			IDX_MULABCW },

{ "NOP",		ANY_VALUE,		0x0b,			0x3f,			IDX_NOP },
{ "OPMULA",		ANY_VALUE,		0x0b,			0x3e,			IDX_OPMULA },
{ "OPMSUB",		ANY_VALUE,		ANY_VALUE,		0x2e,			IDX_OPMSUB },

{ "SUB",		ANY_VALUE,		ANY_VALUE,		0x2c,			IDX_SUB },
{ "SUBi",		ANY_VALUE,		ANY_VALUE,		0x26,			IDX_SUBi },
{ "SUBq",		ANY_VALUE,		ANY_VALUE,		0x24,			IDX_SUBq },
{ "SUBX",		ANY_VALUE,		ANY_VALUE,		0x04,			IDX_SUBBCX },
{ "SUBY",		ANY_VALUE,		ANY_VALUE,		0x05,			IDX_SUBBCY },
{ "SUBZ",		ANY_VALUE,		ANY_VALUE,		0x06,			IDX_SUBBCZ },
{ "SUBW",		ANY_VALUE,		ANY_VALUE,		0x07,			IDX_SUBBCW },

{ "SUBA",		ANY_VALUE,		0x0b,			0x3c,			IDX_SUBA },
{ "SUBAi",		ANY_VALUE,		0x09,			0x3e,			IDX_SUBAi },
{ "SUBAq",		ANY_VALUE,		0x09,			0x3c,			IDX_SUBAq },
{ "SUBAX",		ANY_VALUE,		0x01,			0x3c,			IDX_SUBABCX },
{ "SUBAY",		ANY_VALUE,		0x01,			0x3d,			IDX_SUBABCY },
{ "SUBAZ",		ANY_VALUE,		0x01,			0x3e,			IDX_SUBABCZ },
{ "SUBAW",		ANY_VALUE,		0x01,			0x3f,			IDX_SUBABCW }


//{ "COP2",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_COP2 },

};

// lower instructions //


const Instruction::Entry Instruction::Lookup::EntriesLO [] = {

{ "DIV",		0x40,		0x0e,			0x3c,			IDX_DIV },

{ "IADD",		0x40,		ANY_VALUE,		0x30,			IDX_IADD },
{ "IADDI",		0x40,		ANY_VALUE,		0x32,			IDX_IADDI },
{ "IAND",		0x40,		ANY_VALUE,		0x34,			IDX_IAND },
{ "ILWR",		0x40,		0x0f,			0x3e,			IDX_ILWR },
{ "IOR",		0x40,		ANY_VALUE,		0x35,			IDX_IOR },
{ "ISUB",		0x40,		ANY_VALUE,		0x31,			IDX_ISUB },
{ "ISWR",		0x40,		0x0f,			0x3f,			IDX_ISWR },

{ "LQD",		0x40,		0x0d,			0x3e,			IDX_LQD },
{ "LQI",		0x40,		0x0d,			0x3c,			IDX_LQI },

{ "MFIR",		0x40,		0x0f,			0x3d,			IDX_MFIR },
{ "MOVE",		0x40,		0x0c,			0x3c,			IDX_MOVE },
{ "MR32",		0x40,		0x0c,			0x3d,			IDX_MR32 },
{ "MTIR",		0x40,		0x0f,			0x3c,			IDX_MTIR },

{ "RGET",		0x40,		0x10,			0x3d,			IDX_RGET },
{ "RINIT",		0x40,		0x10,			0x3e,			IDX_RINIT },
{ "RNEXT",		0x40,		0x10,			0x3c,			IDX_RNEXT },

{ "RSQRT",		0x40,		0x0e,			0x3e,			IDX_RSQRT },

{ "RXOR",		0x40,		0x10,			0x3f,			IDX_RXOR },

{ "SQD",		0x40,		0x0d,			0x3f,			IDX_SQD },
{ "SQI",		0x40,		0x0d,			0x3d,			IDX_SQI },

{ "SQRT",		0x40,		0x0e,			0x3d,			IDX_SQRT },

{ "WAITQ",		0x40,		0x0e,			0x3f,			IDX_WAITQ },


// Instructions not available in MACRO mode //

{ "B",			0x20,		ANY_VALUE,		ANY_VALUE,		IDX_B },
{ "BAL",		0x21,		ANY_VALUE,		ANY_VALUE,		IDX_BAL },

{ "FCAND",		0x12,		ANY_VALUE,		ANY_VALUE,		IDX_FCAND },
{ "FCEQ",		0x10,		ANY_VALUE,		ANY_VALUE,		IDX_FCEQ },
{ "FCGET",		0x1c,		ANY_VALUE,		ANY_VALUE,		IDX_FCGET },
{ "FCOR",		0x13,		ANY_VALUE,		ANY_VALUE,		IDX_FCOR },
{ "FCSET",		0x11,		ANY_VALUE,		ANY_VALUE,		IDX_FCSET },

{ "FMAND",		0x1a,		ANY_VALUE,		ANY_VALUE,		IDX_FMAND },
{ "FMEQ",		0x18,		ANY_VALUE,		ANY_VALUE,		IDX_FMEQ },
{ "FMOR",		0x1b,		ANY_VALUE,		ANY_VALUE,		IDX_FMOR },

{ "FSAND",		0x16,		ANY_VALUE,		ANY_VALUE,		IDX_FSAND },
{ "FSEQ",		0x14,		ANY_VALUE,		ANY_VALUE,		IDX_FSEQ },
{ "FSOR",		0x17,		ANY_VALUE,		ANY_VALUE,		IDX_FSOR },
{ "FSSET",		0x15,		ANY_VALUE,		ANY_VALUE,		IDX_FSSET },

{ "IADDIU",		0x08,		ANY_VALUE,		ANY_VALUE,		IDX_IADDIU },

{ "IBEQ",		0x28,		ANY_VALUE,		ANY_VALUE,		IDX_IBEQ },
{ "IBGEZ",		0x2f,		ANY_VALUE,		ANY_VALUE,		IDX_IBGEZ },
{ "IBGTZ",		0x2d,		ANY_VALUE,		ANY_VALUE,		IDX_IBGTZ },
{ "IBLEZ",		0x2e,		ANY_VALUE,		ANY_VALUE,		IDX_IBLEZ },
{ "IBLTZ",		0x2c,		ANY_VALUE,		ANY_VALUE,		IDX_IBLTZ },
{ "IBNE",		0x29,		ANY_VALUE,		ANY_VALUE,		IDX_IBNE },

{ "ILW",		0x04,		ANY_VALUE,		ANY_VALUE,		IDX_ILW },

{ "ISUBIU",		0x09,		ANY_VALUE,		ANY_VALUE,		IDX_ISUBIU },
{ "ISW",		0x05,		ANY_VALUE,		ANY_VALUE,		IDX_ISW },

{ "JALR",		0x25,		ANY_VALUE,		ANY_VALUE,		IDX_JALR },
{ "JR",			0x24,		ANY_VALUE,		ANY_VALUE,		IDX_JR },

{ "LQ",			0x00,		ANY_VALUE,		ANY_VALUE,		IDX_LQ },

{ "MFP",		0x40,		0x19,			0x3c,			IDX_MFP },

{ "SQ",			0x01,		ANY_VALUE,		ANY_VALUE,		IDX_SQ },

{ "WAITP",		0x40,		0x1e,			0x3f,			IDX_WAITP },

{ "XGKICK",		0x40,		0x1b,			0x3c,			IDX_XGKICK },

{ "XITOP",		0x40,		0x1a,			0x3d,			IDX_XITOP },
{ "XTOP",		0x40,		0x1a,			0x3c,			IDX_XTOP },

// External Unit //

{ "EATAN",		0x40,		0x1f,			0x3d,			IDX_EATAN },
{ "EATANxy",	0x40,		0x1d,			0x3c,			IDX_EATANXY },
{ "EATANxz",	0x40,		0x1d,			0x3d,			IDX_EATANXZ },
{ "EEXP",		0x40,		0x1f,			0x3e,			IDX_EEXP },
{ "ELENG",		0x40,		0x1c,			0x3e,			IDX_ELENG },
{ "ERCPR",		0x40,		0x1e,			0x3e,			IDX_ERCPR },
{ "ERLENG",		0x40,		0x1c,			0x3f,			IDX_ERLENG },
{ "ERSADD",		0x40,		0x1c,			0x3d,			IDX_ERSADD },
{ "ERSQRT",		0x40,		0x1e,			0x3d,			IDX_ERSQRT },
{ "ESADD",		0x40,		0x1c,			0x3c,			IDX_ESADD },
{ "ESIN",		0x40,		0x1f,			0x3c,			IDX_ESIN },
{ "ESQRT",		0x40,		0x1e,			0x3c,			IDX_ESQRT },
{ "ESUM",		0x40,		0x1d,			0x3e,			IDX_ESUM }




//{ "INVALID",	ANY_VALUE,	ANY_VALUE,		ANY_VALUE,		IDX_Invalid }

};


// returns -1 on error
int Lookup::FindByNameHI ( string NameOfInstruction )
{
	bool found;
	string Line_NoComment;
	string sInst;
	int StartPos, EndPos;
	
	// get rid of any comments
	//cout << "hasInstruction: removing comment from line. Line=" << NameOfInstruction.c_str() << "\n";
	//Line_NoComment = removeLabel( removeComment ( NameOfInstruction ) );
	
	// remove whitespace
	//cout << "hasInstruction: trimming line. Line_NoComment=" << NameOfInstruction.c_str() << "\n";
	Line_NoComment = Trim ( NameOfInstruction );
	
	// convert to upper case
	//cout << "hasInstruction: converting line to uppercase. Line_NoComment=" << Line_NoComment << "\n";
	Line_NoComment = UCase ( Line_NoComment );
	
	// get the instruction
	//cout << "hasInstruction: Getting the instruction.\n";
	//cout << "hasInstruction: Getting StartPos. Line_NoComment=" << Line_NoComment << "\n";
	StartPos = Line_NoComment.find_first_not_of ( " \r\n\t" );
	
	//cout << "StartPos=" << StartPos << "\n";
	
	if ( StartPos != string::npos )
	{
		//cout << "hasInstruction: Getting EndPos.\n";
		EndPos = Line_NoComment.find_first_of ( " \r\n\t", StartPos );
		
		if ( EndPos == string::npos ) EndPos = Line_NoComment.length();
		
		//cout << "hasInstruction: Getting just the instruction. EndPos=" << EndPos << "\n";
		sInst = Line_NoComment.substr( StartPos, EndPos - StartPos );
		
#ifdef INLINE_DEBUG
		debug << "hasInstruction: Instruction=" << sInst.c_str () << "\n";
#endif
		
		//cout << "hasInstruction: Instruction=" << sInst.c_str () << "\n";
		//cout << "\nnum instructions = " << dec << ( sizeof( Entries ) / sizeof( Entries [ 0 ] ) );
		
		// see if we find the instruction in the list
		for ( int i = 0; i < ( sizeof( EntriesHI ) / sizeof( EntriesHI [ 0 ] ) ); i++ )
		{
			if ( !sInst.compare( EntriesHI [ i ].Name ) )
			{
				//cout << "\nfound instruction: " << sInst.c_str() << "\n";
				return EntriesHI [ i ].InstructionIndex;
			}
		}
	}
	
	//cout << "\ndid not find instruction: " << NameOfInstruction;
	return -1;
}




// returns -1 on error
int Lookup::FindByNameLO ( string NameOfInstruction )
{
	bool found;
	string Line_NoComment;
	string sInst;
	int StartPos, EndPos;
	
	// get rid of any comments
	//cout << "hasInstruction: removing comment from line. Line=" << NameOfInstruction.c_str() << "\n";
	//Line_NoComment = removeLabel( removeComment ( NameOfInstruction ) );
	
	// remove whitespace
	//cout << "hasInstruction: trimming line. Line_NoComment=" << NameOfInstruction.c_str() << "\n";
	Line_NoComment = Trim ( NameOfInstruction );
	
	// convert to upper case
	//cout << "hasInstruction: converting line to uppercase. Line_NoComment=" << Line_NoComment << "\n";
	Line_NoComment = UCase ( Line_NoComment );
	
	// get the instruction
	//cout << "hasInstruction: Getting the instruction.\n";
	//cout << "hasInstruction: Getting StartPos. Line_NoComment=" << Line_NoComment << "\n";
	StartPos = Line_NoComment.find_first_not_of ( " \r\n\t" );
	
	//cout << "StartPos=" << StartPos << "\n";
	
	if ( StartPos != string::npos )
	{
		//cout << "hasInstruction: Getting EndPos.\n";
		EndPos = Line_NoComment.find_first_of ( " \r\n\t", StartPos );
		
		if ( EndPos == string::npos ) EndPos = Line_NoComment.length();
		
		//cout << "hasInstruction: Getting just the instruction. EndPos=" << EndPos << "\n";
		sInst = Line_NoComment.substr( StartPos, EndPos - StartPos );
		
#ifdef INLINE_DEBUG
		debug << "hasInstruction: Instruction=" << sInst.c_str () << "\n";
#endif
		
		//cout << "hasInstruction: Instruction=" << sInst.c_str () << "\n";
		//cout << "\nnum instructions = " << dec << ( sizeof( Entries ) / sizeof( Entries [ 0 ] ) );
		
		// see if we find the instruction in the list
		for ( int i = 0; i < ( sizeof( EntriesLO ) / sizeof( EntriesLO [ 0 ] ) ); i++ )
		{
			if ( !sInst.compare( EntriesLO [ i ].Name ) )
			{
				//cout << "\nfound instruction: " << sInst.c_str() << "\n";
				return EntriesLO [ i ].InstructionIndex;
			}
		}
	}
	
	//cout << "\ndid not find instruction: " << NameOfInstruction;
	return -1;
}



//Debug::Log Lookup::debug;


void Lookup::Start ()
{
	u32 Opcode, Fd, Funct, Index, ElementsInExecute, ElementsInBranchLoad1;
	Instruction::Format i;
	
	u32 ulCounter, ulRemainder;

	cout << "Running constructor for R5900::Lookup class.\n";
	
	if ( c_bObjectInitialized ) return;

#ifdef INLINE_DEBUG_ENABLE	
	debug.Create ( "VU_Lookup_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "Running constructor for R5900::Lookup class.\r\n";
#endif

	
	ElementsInExecute = (sizeof(EntriesLO) / sizeof(EntriesLO[0]));
	
	// clear table first
	cout << "\nSize of VU lookup table lo in bytes=" << dec << sizeof( LookupTableLO );
	for ( Index = 0; Index < ( sizeof( LookupTableLO ) >> 3 ); Index++ ) ((u64*)LookupTableLO) [ Index ] = 0;
	
	for ( Index = ElementsInExecute - 1; Index < ElementsInExecute; Index-- )
	{
		ulRemainder = 0;
		
		Opcode = EntriesLO [ Index ].Opcode;
		Fd = EntriesLO [ Index ].Fd;
		Funct = EntriesLO [ Index ].Funct;
		
		i.Opcode = Opcode;
		i.Fd = Fd;
		i.Funct = Funct;
		
		for ( ulCounter = 0; ulRemainder == 0; ulCounter++ )
		{
			ulRemainder = ulCounter;
			
			if ( Opcode == 0xff )
			{
				// 7 bits
				i.Opcode = ulRemainder & 0x7f;
				ulRemainder >>= 7;
			}
			
			if ( Fd == 0xff )
			{
				// 5 bits
				i.Fd = ulRemainder & 0x1f;
				ulRemainder >>= 5;
			}
			
			if ( Funct == 0xff )
			{
				// 6 bits
				i.Funct = ulRemainder & 0x3f;
				ulRemainder >>= 6;
			}
			
			LookupTableLO [ ( ( i.Value >> 25 ) | ( i.Value << 7 ) ) & c_iLookupTableLO_Mask ] = EntriesLO [ Index ].InstructionIndex;
		}
	}



	ElementsInExecute = (sizeof(EntriesHI) / sizeof(EntriesHI[0]));
	
	// clear table first
	cout << "\nSize of VU lookup table hi in bytes=" << dec << sizeof( LookupTableHI );
	for ( Index = 0; Index < ( sizeof( LookupTableHI ) >> 3 ); Index++ ) ((u64*)LookupTableHI) [ Index ] = 0;
	
	for ( Index = ElementsInExecute - 1; Index < ElementsInExecute; Index-- )
	{
		ulRemainder = 0;
		
		Opcode = EntriesHI [ Index ].Opcode;
		Fd = EntriesHI [ Index ].Fd;
		Funct = EntriesHI [ Index ].Funct;
		
		i.Opcode = Opcode;
		i.Fd = Fd;
		i.Funct = Funct;
		
		for ( ulCounter = 0; ulRemainder == 0; ulCounter++ )
		{
			ulRemainder = ulCounter;
			
			//if ( Opcode == 0xff )
			//{
			//	// 7 bits
			//	i.Opcode = ulRemainder & 0x7f;
			//	ulRemainder >>= 7;
			//}
			
			if ( Fd == 0xff )
			{
				// 5 bits
				i.Fd = ulRemainder & 0x1f;
				ulRemainder >>= 5;
			}
			
			if ( Funct == 0xff )
			{
				// 6 bits
				i.Funct = ulRemainder & 0x3f;
				ulRemainder >>= 6;
			}
			
			LookupTableHI [ i.Value & c_iLookupTableHI_Mask ] = EntriesHI [ Index ].InstructionIndex;
		}
	}
	

	
	// object has now been fully initilized
	c_bObjectInitialized = true;
}
