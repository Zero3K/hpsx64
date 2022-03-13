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

#include "StringUtils.h"
#include "R5900_Assembler.h"
#include "R5900_Encoder.h"
#include "R5900_Lookup.h"

#include <iostream>

using namespace std;

using namespace R5900::Utilities;

using namespace Utilities::Strings;

using namespace R5900;
using namespace R5900::Instruction;


// for debugging
//#define INLINE_DEBUG


#ifdef INLINE_DEBUG
Debug::Log Assembler::debug;
#endif

vector<Assembler::LabelEntry*> Assembler::Labels;



Assembler::Assembler ()
{

#ifdef INLINE_DEBUG
	debug.Create ( "Assembler_Log.txt" );
#endif

	// don't forget to start object for looking up the instruction
	Lookup::Start ();
}

bool Assembler::SetInputFile ( string InputFileName )
{
	InputFile = new ifstream( InputFileName.c_str(), ios::binary );
	
	if ( !InputFile ) return false;
	
	return true;
}

bool Assembler::SetOutputFile ( string OutputFileName )
{
	OutputFile = new ofstream ( OutputFileName.c_str(), ios::binary );

	if ( !OutputFile ) return false;
	
	return true;
}

Assembler::~Assembler ()
{
	InputFile->close();
	OutputFile->close();
	
	delete InputFile;
	delete OutputFile;
	
	// deallocate label entries
	for ( int i = 0; i < Labels.size(); i++ )
	{
		delete Labels [ i ];
	}
	
	// remove label entries
	while ( !Labels.empty() )
	{
		Labels.pop_back ();
	}
}



// use pass1 to just get the addresses for the labels
void Assembler::Pass1 ()
{
	string Line;
	int LineNumber = 1;
	
	u32 CurrentAddress = 0;
	
	// keep running until we reach end of file
	while ( InputFile->good() )
	{

		// get a line from source file
		//cout << "Getting Line#" << LineNumber << " from the input file.\n";
		getline ( *InputFile, Line );
		
		// remove any comments from the line
		//cout << "Removing comments from line.\n";
		Line = removeComment ( Line );
		
		// check if line contains an instruction
		//cout << "Checking if Line has an instruction or a label.\n";
		if ( hasInstruction ( Line ) >= 0 )
		{
			// the address of the next label goes up by 4 bytes
			//cout << "Line has an instruction.\n";
			CurrentAddress += 4;
		}
		
		// check if the line contains a label
		else if ( hasLabel ( Line ) )
		{
			// add the label and it's address into vector
			//cout << "Line has a label. Adding label.\n";
			Labels.push_back ( new LabelEntry ( Line.substr( 0, Line.find(":") ), CurrentAddress ) );
		}
		
		LineNumber++;
	}
	
}


// use pass2 to put object code into the output file replacing labels with addresses
// I'll use "%" to precede register names, "$" to precede constant values
void Assembler::Pass2 ()
{
	string Line;
	string Line_NoComment;
	
	long Code;
	
	long LineNumber = 1;
	
	long CurrentAddress = 0;
	
	int InstIndex;

	bool isError = false;

	InputFile->clear ();
	InputFile->seekg ( 0, ios::beg );
	
	// keep running until we reach end of file
	while ( InputFile->good() )
	{
		// get a line from source file
		//cout << "Pass2: Getting Line#" << LineNumber << " from file.\n";
		getline ( *InputFile, Line );
		
		// remove comments from line
		//cout << "Pass2: removing comments from line.\n";
		Line_NoComment = removeComment ( Line );
		
		// remove labels from line
		//cout << "Pass2: removing labels from line.\n";
		Line_NoComment = removeLabel ( Line_NoComment );
		
		// remove extra spaces from line
		//cout << "Pass2: removing extra spaces from line.\n";
		Line_NoComment = Trim ( Line_NoComment );
		
		// check if line contains an instruction
		//cout << "Pass2: checking if line has an instruction.\n";
		InstIndex = hasInstruction ( Line_NoComment );
		if ( InstIndex >= 0 )
		{
#ifdef INLINE_DEBUG
			debug << "matched instruction with: " << Entries [ InstIndex ].Name << "\n";
#endif

			//cout << "matched instruction with: " << Lookup::Entries [ InstIndex ].Name << "\n";
			
			// call lookup to get instruction decoded
			//if ( !Entries [ InstIndex ].FunctionToCall ( Code, Line_NoComment, CurrentAddress ) )
			if ( ! ( FunctionList [ InstIndex ] ( Code, Line_NoComment, CurrentAddress ) ) )
			{
				// there was an error parsing line of assembly code
				cout << "Error on line#" << LineNumber << ". Unimplemented instruction for assembler: Index=" << InstIndex << "\n";

				isError = true;
			}
			else
			{
				// write decoded instruction into file
				//cout << "Pass2: writing instruction from Line#" << LineNumber << " into file.\n";
#ifdef INLINE_DEBUG
				debug << "Pass2: writing instrution " << hex << Code << dec << " into file.\n";
#endif

				cout << "Pass2: writing instrution " << hex << Code << dec << " into file.\n";
				
				OutputFile->write ( (const char*) (&Code), 4 );
				
				// update current address
				CurrentAddress += 4;
			}
		}

		// go to next line of assembly source code
		LineNumber++;
	}
	
	if ( isError )
	{
		// and error occurred during compilation
		cout << "An error occurrecd while assembling source code.\n";
	}
	else
	{
		cout << "Source code assembled successfully. No errors detected.\n";
	}
	
}


bool Assembler::hasLabel ( string Line )
{
	if ( removeComment ( Line ).find (":") != string::npos ) return true;
	
	return false;
}

bool Assembler::hasComment ( string Line )
{
	if ( Line.find(";") != string::npos ) return true;
	
	return false;
}

bool Assembler::hasDirective ( string Line )
{
}

// semicolon is for comment
string Assembler::removeComment ( string Line )
{
	int Pos;
	
	Pos = Line.find ( CommentString );
	
	if ( Pos == string::npos ) return Line;
	
	return Line.substr( 0, Pos );
}

string Assembler::removeLabel ( string Line )
{
	int Pos;
	
	Pos = Line.find ( ":" );
	
	if ( Pos == string::npos ) return Line;
	
	return Line.substr ( Pos + 1 );
}



// returns -1 when no register was found
// register name can also be preceded by letter "r"
bool Assembler::GetRegister ( long& Result, string Line, long Instance )
{
	int RegStartPos = 0, RegEndPos;
	string RegName;

	for ( int i = 0; i < Instance; i++ )
	{
		RegStartPos = Line.find_first_of ( RegisterString, RegStartPos );
		
		if ( RegStartPos == string::npos ) return false;
		
		RegEndPos = Line.find_first_of ( " \r\t\n,", RegStartPos );
		RegName = Line.substr ( RegStartPos, RegEndPos - RegStartPos );
		
		RegStartPos = RegEndPos;
	}
	
	RegName = Replace ( RegName, RegisterString, "" );
	RegName = Replace ( RegName, "r", "" );
	
	//cout << "\nAssembler::GetRegister; Instance=" << Instance << " Line=" << Line.c_str() << " RegStartPos=" << dec << RegStartPos << " RegEndPos=" << RegEndPos << " RegName=" << RegName.c_str();
	
	// convert register number into an integer
	if ( !from_string<long> ( Result, RegName, std::dec ) ) return false;
	
	//cout << "\nAssembler::GetRegister->done\n";
	
	return true;
}

// integers will be preceded by a "$" symbol
bool Assembler::GetImmed ( long& Result, string Line, long Instance )
{
	int RegStartPos = 0, RegEndPos;
	string Immed;

	for ( int i = 0; i < Instance; i++ )
	{
		RegStartPos = Line.find_first_of ( ImmedString, RegStartPos );
		
		if ( RegStartPos == string::npos ) return false;
		
		RegEndPos = Line.find_first_of ( " \r\t\n,", RegStartPos );
		Immed = Line.substr ( RegStartPos, RegEndPos - RegStartPos );
		
		RegStartPos = RegEndPos;
	}
	
	Immed = Replace ( Immed, ImmedString, "" );
	//Immed = Replace ( Immed, "r", "" );
	
	//cout << "\nAssembler::GetImmed; Line=" << Line.c_str() << " RegStartPos=" << dec << RegStartPos << " RegEndPos=" << RegEndPos << " Immed=" << Immed;
	
	// convert immediate number into an integer
	
	// check if immediate is in hexadecimal
	if ( InStr ( LCase ( Immed ), "0x" ) != string::npos )
	{
		// immediate is hexadecimal
		Immed = Replace ( Immed, "0x", "" );
		if ( !from_string<long> ( Result, Immed, std::hex ) ) return false;
	}
	else
	{
		// immediate is decimal
		if ( !from_string<long> ( Result, Immed, std::dec ) ) return false;
	}
	
	//cout << "\nAssembler::GetImmed->true";
	
	return true;
}

// labels will be preceded by a ">" symbol
// returns label index if found, -1 otherwise
bool Assembler::GetLabel ( long& Result, string Line, long Instance )
{
	int RegStartPos = 0, RegEndPos;
	string RegName;

	for ( int i = 0; i < Instance; i++ )
	{
		RegStartPos = Line.find_first_of ( LabelString, RegStartPos );
		
		if ( RegStartPos == string::npos ) return false;
		
		RegEndPos = Line.find_first_of ( " \r\t\n,", RegStartPos );
		RegName = Line.substr ( RegStartPos, RegEndPos - RegStartPos );
		
		RegStartPos = RegEndPos;
	}
	
	RegName = Replace ( RegName, LabelString, "" );
	//RegName = Replace ( RegName, "r", "" );
	
	// convert register number into an integer
	//if ( !from_string<long> ( Result, RegName, std::dec ) ) return -1;
	
	// lookup label value
	for ( i = 0; i < Labels.size(); i++ )
	{
		if ( !RegName.compare( Labels [ i ]->Name ) )
		{
			Result = Labels [ i ]->Address;
			return true;
		}
	}
	
	return false;
}




inline bool Assembler::Format1 ( long &rt, long &rs, long &immediate, string sInst )
{
	//cout << "\nFormat1->GetRegister; sInst=" << sInst.c_str();
	
	if ( !GetRegister ( rt, sInst, 1 ) || !GetRegister ( rs, sInst, 2 ) ) return false;
	
	//cout << "\nFormat1->GetImmed";
	
	if ( !GetImmed ( immediate, sInst, 1 ) )
	{
		//cout << "\nFormat1->GetLabel";
		if ( !GetLabel ( immediate, sInst, 1 ) ) return false;
	}
	
	//cout << "\nFormat1->true";
	return true;
}

inline bool Assembler::Format2 ( long &rd, long &rs, long &rt, string sInst )
{
	if ( !GetRegister ( rd, sInst, 1 ) || !GetRegister ( rs, sInst, 2 ) || !GetRegister ( rt, sInst, 3 ) ) return false;
	
	return true;
}

inline bool Assembler::Format3 ( long &rs, long &rt, string sInst )
{
	if ( !GetRegister ( rs, sInst, 1 ) || !GetRegister ( rt, sInst, 2 ) ) return false;
	
	return true;
}

inline bool Assembler::Format4 ( long &rs, long &immediate, string sInst )
{
	if ( !GetRegister ( rs, sInst, 1 ) ) return false;
	
	if ( !GetImmed ( immediate, sInst, 1 ) )
	{
		if ( !GetLabel ( immediate, sInst, 1 ) ) return false;
	}
	
	return true;
}

inline bool Assembler::Format5 ( long &rs, string sInst )
{
	if ( !GetRegister ( rs, sInst, 1 ) ) return false;
	
	return true;
}

// inst offset
inline bool Assembler::Format19 ( long &offset, string sInst )
{
	if ( !GetImmed ( offset, sInst, 1 ) )
	{
		if ( !GetLabel ( offset, sInst, 1 ) ) return false;
	}
	
	return true;
}






bool Assembler::ADDIU ( u32& Code, string sInst, long CurrentAddress )
{
	// addiu rt, rs, immediate
	long rt, rs, immediate;
	
	if ( !Format1 ( rt, rs, immediate, sInst ) ) return false;
	
	Code = Encoder::ADDIU ( rt, rs, immediate );
	return true;
}

bool Assembler::ANDI ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rs, immediate;
	
	if ( !Format1 ( rt, rs, immediate, sInst ) ) return false;
	
	Code = Encoder::ANDI ( rt, rs, immediate );
	return true;
}

bool Assembler::ORI ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rs, immediate;
	
	if ( !Format1 ( rt, rs, immediate, sInst ) ) return false;
	
	Code = Encoder::ORI ( rt, rs, immediate );
	return true;
}

bool Assembler::SLTI ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rs, immediate;
	
	if ( !Format1 ( rt, rs, immediate, sInst ) ) return false;
	
	Code = Encoder::SLTI ( rt, rs, immediate );
	return true;
}

bool Assembler::SLTIU ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rs, immediate;
	
	if ( !Format1 ( rt, rs, immediate, sInst ) ) return false;
	
	Code = Encoder::SLTIU ( rt, rs, immediate );
	return true;
}

bool Assembler::XORI ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rs, immediate;
	
	//cout << "\nAssembler::XORI; sInst=" << sInst.c_str() << "\n";
	
	if ( !Format1 ( rt, rs, immediate, sInst ) ) return false;
	
	Code = Encoder::XORI ( rt, rs, immediate );
	
	//cout << "\nXORI returning true\n";
	return true;
}


bool Assembler::ADDI ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rs, immediate;
	
	if ( !Format1 ( rt, rs, immediate, sInst ) ) return false;
	
	Code = Encoder::ADDI ( rt, rs, immediate );
	return true;
}



bool Assembler::ADDU ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rs, rt;
	
	if ( !Format2 ( rd, rs, rt, sInst ) ) return false;
	
	Code = Encoder::ADDU ( rd, rs, rt );
	return true;
}

bool Assembler::AND ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rs, rt;
	
	if ( !Format2 ( rd, rs, rt, sInst ) ) return false;
	
	Code = Encoder::AND ( rd, rs, rt );
	return true;
}

bool Assembler::OR ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rs, rt;
	
	if ( !Format2 ( rd, rs, rt, sInst ) ) return false;
	
	Code = Encoder::OR ( rd, rs, rt );
	return true;
}

bool Assembler::NOR ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rs, rt;
	
	if ( !Format2 ( rd, rs, rt, sInst ) ) return false;
	
	Code = Encoder::NOR ( rd, rs, rt );
	return true;
}

bool Assembler::SLT ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rs, rt;
	
	if ( !Format2 ( rd, rs, rt, sInst ) ) return false;
	
	Code = Encoder::SLT ( rd, rs, rt );
	return true;
}

bool Assembler::SLTU ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rs, rt;
	
	if ( !Format2 ( rd, rs, rt, sInst ) ) return false;
	
	Code = Encoder::SLTU ( rd, rs, rt );
	return true;
}

bool Assembler::SUBU ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rs, rt;
	
	if ( !Format2 ( rd, rs, rt, sInst ) ) return false;
	
	Code = Encoder::SUBU ( rd, rs, rt );
	return true;
}

bool Assembler::XOR ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rs, rt;
	
	//cout << "\nAssembler::XOR; sInst=" << sInst.c_str() << "\n";
	
	if ( !Format2 ( rd, rs, rt, sInst ) ) return false;
	
	Code = Encoder::XOR ( rd, rs, rt );
	
	//cout << "\nXOR returning true\n";
	return true;
}


bool Assembler::ADD ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rs, rt;
	
	if ( !Format2 ( rd, rs, rt, sInst ) ) return false;
	
	Code = Encoder::ADD ( rd, rs, rt );
	return true;
}

bool Assembler::SUB ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rs, rt;
	
	if ( !Format2 ( rd, rs, rt, sInst ) ) return false;
	
	Code = Encoder::SUB ( rd, rs, rt );
	return true;
}


bool Assembler::DIV ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, rt;
	
	if ( !Format3 ( rs, rt, sInst ) ) return false;
	
	Code = Encoder::DIV ( rs, rt );
	return true;
}

bool Assembler::DIVU ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, rt;
	
	if ( !Format3 ( rs, rt, sInst ) ) return false;
	
	Code = Encoder::DIVU ( rs, rt );
	return true;
}

bool Assembler::MULT ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, rt;
	
	if ( !Format3 ( rs, rt, sInst ) ) return false;
	
	Code = Encoder::MULT ( rs, rt );
	return true;
}

bool Assembler::MULTU ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, rt;
	
	if ( !Format3 ( rs, rt, sInst ) ) return false;
	
	Code = Encoder::MULTU ( rs, rt );
	return true;
}


bool Assembler::SLL ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rt, sa;
	if ( !Format1 ( rd, rt, sa, sInst ) ) return false;
	Code = Encoder::SLL ( rd, rt, sa );
	return true;
}

bool Assembler::SRA ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rt, sa;
	if ( !Format1 ( rd, rt, sa, sInst ) ) return false;
	Code = Encoder::SRA ( rd, rt, sa );
	return true;
}

bool Assembler::SRL ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rt, sa;
	if ( !Format1 ( rd, rt, sa, sInst ) ) return false;
	Code = Encoder::SRL ( rd, rt, sa );
	return true;
}


bool Assembler::SLLV ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rt, rs;
	if ( !Format2 ( rd, rt, rs, sInst ) ) return false;
	Code = Encoder::SLLV ( rd, rt, rs );
	return true;
}

bool Assembler::SRAV ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rt, rs;
	if ( !Format2 ( rd, rt, rs, sInst ) ) return false;
	Code = Encoder::SRAV ( rd, rt, rs );
	return true;
}

bool Assembler::SRLV ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rt, rs;
	if ( !Format2 ( rd, rt, rs, sInst ) ) return false;
	Code = Encoder::SRLV ( rd, rt, rs );
	return true;
}


bool Assembler::J ( u32& Code, string sInst, long CurrentAddress )
{
	long target;
	
	if ( !GetImmed ( target, sInst, 1 ) )
	{
		if ( !GetLabel ( target, sInst, 1 ) ) return false;
	}
	
	Code = Encoder::J ( target );
	return true;
}

bool Assembler::JR ( u32& Code, string sInst, long CurrentAddress )
{
	long rs;
	if ( !Format5 ( rs, sInst ) ) return false;
	Code = Encoder::JR ( rs );
	return true;
}

bool Assembler::JAL ( u32& Code, string sInst, long CurrentAddress )
{
	long target;
	
	if ( !GetImmed ( target, sInst, 1 ) )
	{
		if ( !GetLabel ( target, sInst, 1 ) ) return false;
	}
	
	Code = Encoder::JAL ( target );
	return true;
}

bool Assembler::JALR ( u32& Code, string sInst, long CurrentAddress )
{
	long rd, rs;
	if ( !Format3 ( rd, rs, sInst ) ) return false;
	Code = Encoder::JALR ( rd, rs );
	return true;
}



bool Assembler::BEQ ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, rt, offset, LabelAddress;

	if ( !Format1 ( rs, rt, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BEQ ( rs, rt, offset );
	return true;
}

bool Assembler::BNE ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, rt, offset, LabelAddress;

	if ( !Format1 ( rs, rt, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BNE ( rs, rt, offset );
	return true;
}

bool Assembler::BGEZ ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, offset, LabelAddress;

	if ( !Format4 ( rs, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BGEZ ( rs, offset );
	return true;
}

bool Assembler::BGTZ ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, offset, LabelAddress;

	if ( !Format4 ( rs, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BGTZ ( rs, offset );
	return true;
}

bool Assembler::BLEZ ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, offset, LabelAddress;

	if ( !Format4 ( rs, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BLEZ ( rs, offset );
	return true;
}

bool Assembler::BLTZ ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, offset, LabelAddress;

	if ( !Format4 ( rs, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BLTZ ( rs, offset );
	return true;
}

bool Assembler::BGEZAL ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, offset, LabelAddress;

	if ( !Format4 ( rs, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BGEZAL ( rs, offset );
	return true;
}

bool Assembler::BLTZAL ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, offset, LabelAddress;

	if ( !Format4 ( rs, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BLTZAL ( rs, offset );
	return true;
}



bool Assembler::LUI ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, immediate;

	if ( !Format4 ( rs, immediate, sInst ) ) return false;

	Code = Encoder::LUI ( rs, immediate );
	return true;
}



bool Assembler::MFHI ( u32& Code, string sInst, long CurrentAddress )
{
	long rd;
	if ( !Format5 ( rd, sInst ) ) return false;
	Code = Encoder::MFHI ( rd );
	return true;
}

bool Assembler::MFLO ( u32& Code, string sInst, long CurrentAddress )
{
	long rd;
	if ( !Format5 ( rd, sInst ) ) return false;
	Code = Encoder::MFLO ( rd );
	return true;
}

bool Assembler::MTHI ( u32& Code, string sInst, long CurrentAddress )
{
	long rs;
	if ( !Format5 ( rs, sInst ) ) return false;
	Code = Encoder::MTHI ( rs );
	return true;
}

bool Assembler::MTLO ( u32& Code, string sInst, long CurrentAddress )
{
	long rs;
	if ( !Format5 ( rs, sInst ) ) return false;
	Code = Encoder::MTLO ( rs );
	return true;
}


bool Assembler::SYSCALL ( u32& Code, string sInst, long CurrentAddress )
{
	Code = Encoder::SYSCALL ();
	return true;
}

//bool Assembler::RFE ( u32& Code, string sInst, long CurrentAddress )
//{
//	Code = Encoder::RFE ();
//	return true;
//}

bool Assembler::BREAK ( u32& Code, string sInst, long CurrentAddress )
{
	Code = Encoder::BREAK ();
	return true;
}


bool Assembler::MFC0 ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rd;
	
	if ( !Format3 ( rt, rd, sInst ) ) return false;
	
	Code = Encoder::MFC0 ( rt, rd );
	return true;
}

bool Assembler::MTC0 ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rd;
	
	if ( !Format3 ( rt, rd, sInst ) ) return false;
	
	Code = Encoder::MTC0 ( rt, rd );
	return true;
}

//bool Assembler::MFC2 ( u32& Code, string sInst, long CurrentAddress )
//{
//	long rt, rd;
//	
//	if ( !Format3 ( rt, rd, sInst ) ) return false;
//	
//	Code = Encoder::MFC2 ( rt, rd );
//	return true;
//}

//bool Assembler::MTC2 ( u32& Code, string sInst, long CurrentAddress )
//{
//	long rt, rd;
//	
//	if ( !Format3 ( rt, rd, sInst ) ) return false;
//	
//	Code = Encoder::MTC2 ( rt, rd );
//	return true;
//}

bool Assembler::CFC2 ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rd;
	
	if ( !Format3 ( rt, rd, sInst ) ) return false;
	
	Code = Encoder::CFC2 ( rt, rd );
	return true;
}

bool Assembler::CTC2 ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rd;
	
	if ( !Format3 ( rt, rd, sInst ) ) return false;
	
	Code = Encoder::CTC2 ( rt, rd );
	return true;
}


//bool Assembler::COP2 ( u32& Code, string sInst, long CurrentAddress )
//{
//	long Command;
//	if ( !GetImmed ( Command, sInst, 1 ) ) return false;
//	Code = Encoder::COP2 ( Command );
//	return true;
//}

bool Assembler::LB ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::LB ( rt, offset, base );
	return true;
}

bool Assembler::LH ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::LH ( rt, offset, base );
	return true;
}

bool Assembler::LWL ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::LWL ( rt, offset, base );
	return true;
}

bool Assembler::LW ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::LW ( rt, offset, base );
	return true;
}

bool Assembler::LBU ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::LBU ( rt, offset, base );
	return true;
}

bool Assembler::LHU ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::LHU ( rt, offset, base );
	return true;
}

bool Assembler::LWR ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::LWR ( rt, offset, base );
	return true;
}

bool Assembler::SB ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::SB ( rt, offset, base );
	return true;
}

bool Assembler::SH ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::SH ( rt, offset, base );
	return true;
}

bool Assembler::SWL ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::SWL ( rt, offset, base );
	return true;
}

bool Assembler::SW ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::SW ( rt, offset, base );
	return true;
}

bool Assembler::SWR ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::SWR ( rt, offset, base );
	return true;
}

//bool Assembler::LWC2 ( u32& Code, string sInst, long CurrentAddress )
//{
//	long rt, offset, base;
//	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
//	Code = Encoder::LWC2 ( rt, offset, base );
//	return true;
//}

//bool Assembler::SWC2 ( u32& Code, string sInst, long CurrentAddress )
//{
//	long rt, offset, base;
//	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
//	Code = Encoder::SWC2 ( rt, offset, base );
//	return true;
//}

bool Assembler::Invalid ( u32& Code, string sInst, long CurrentAddress )
{
	return false;
}






static bool Assembler::BC0T ( u32& Code, string sInst, long CurrentAddress )
{
	long offset, LabelAddress;

	if ( !Format19 ( LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BC0T ( offset );
	return true;
}

static bool Assembler::BC0TL ( u32& Code, string sInst, long CurrentAddress )
{
	long offset, LabelAddress;

	if ( !Format19 ( LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BC0T ( offset );
	return true;
}

static bool Assembler::BC0F ( u32& Code, string sInst, long CurrentAddress )
{
	long offset, LabelAddress;

	if ( !Format19 ( LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BC0T ( offset );
	return true;
}

static bool Assembler::BC0FL ( u32& Code, string sInst, long CurrentAddress )
{
	long offset, LabelAddress;

	if ( !Format19 ( LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BC0T ( offset );
	return true;
}

static bool Assembler::BC1T ( u32& Code, string sInst, long CurrentAddress )
{
	long offset, LabelAddress;

	if ( !Format19 ( LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BC0T ( offset );
	return true;
}

static bool Assembler::BC1TL ( u32& Code, string sInst, long CurrentAddress )
{
	long offset, LabelAddress;

	if ( !Format19 ( LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BC0T ( offset );
	return true;
}

static bool Assembler::BC1F ( u32& Code, string sInst, long CurrentAddress )
{
	long offset, LabelAddress;

	if ( !Format19 ( LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BC0T ( offset );
	return true;
}

static bool Assembler::BC1FL ( u32& Code, string sInst, long CurrentAddress )
{
	long offset, LabelAddress;

	if ( !Format19 ( LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BC0T ( offset );
	return true;
}

static bool Assembler::BC2T ( u32& Code, string sInst, long CurrentAddress )
{
	long offset, LabelAddress;

	if ( !Format19 ( LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BC0T ( offset );
	return true;
}

static bool Assembler::BC2TL ( u32& Code, string sInst, long CurrentAddress )
{
	long offset, LabelAddress;

	if ( !Format19 ( LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BC0T ( offset );
	return true;
}

static bool Assembler::BC2F ( u32& Code, string sInst, long CurrentAddress )
{
	long offset, LabelAddress;

	if ( !Format19 ( LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BC0T ( offset );
	return true;
}

static bool Assembler::BC2FL ( u32& Code, string sInst, long CurrentAddress )
{
	long offset, LabelAddress;

	if ( !Format19 ( LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BC0T ( offset );
	return true;
}



static bool Assembler::CFC0 ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rd;
	
	if ( !Format3 ( rt, rd, sInst ) ) return false;
	
	Code = Encoder::CFC0 ( rt, rd );
	return true;
}

static bool Assembler::CTC0 ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rd;
	
	if ( !Format3 ( rt, rd, sInst ) ) return false;
	
	Code = Encoder::CTC0 ( rt, rd );
	return true;
}

static bool Assembler::EI ( u32& Code, string sInst, long CurrentAddress )
{
	Code = Encoder::EI ();
	return true;
}

static bool Assembler::DI ( u32& Code, string sInst, long CurrentAddress )
{
	Code = Encoder::DI ();
	return true;
}

//static bool Assembler::MF0 ( u32& Code, string sInst, long CurrentAddress )
//{
//	Code = Encoder::MF0 ();
//	return true;
//}

//static bool Assembler::MT0 ( u32& Code, string sInst, long CurrentAddress )
//{
//	Code = Encoder::MT0 ();
//	return true;
//}



static bool Assembler::SD ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::SD ( rt, offset, base );
	return true;
}

static bool Assembler::LD ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::LD ( rt, offset, base );
	return true;
}

static bool Assembler::LWU ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::LWU ( rt, offset, base );
	return true;
}

static bool Assembler::SDL ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::SDL ( rt, offset, base );
	return true;
}

static bool Assembler::SDR ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::SDR ( rt, offset, base );
	return true;
}

static bool Assembler::LDL ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::LDL ( rt, offset, base );
	return true;
}

static bool Assembler::LDR ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::LDR ( rt, offset, base );
	return true;
}

static bool Assembler::LQ ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::LQ ( rt, offset, base );
	return true;
}

static bool Assembler::SQ ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, offset, base;
	if ( !Format1 ( rt, base, offset, sInst ) ) return false;
	Code = Encoder::SQ ( rt, offset, base );
	return true;
}



// arithemetic instructions //
static bool Assembler::DADD ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DADDI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DADDU ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DADDIU ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DSUB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DSUBU ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DSLL ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DSLL32 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DSLLV ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DSRA ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DSRA32 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DSRAV ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DSRL ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DSRL32 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DSRLV ( u32& Code, string sInst, long CurrentAddress ){}



static bool Assembler::MFC1 ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rd;
	
	if ( !Format3 ( rt, rd, sInst ) ) return false;
	
	Code = Encoder::MFC1 ( rt, rd );
	return true;
}

static bool Assembler::CFC1 ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rd;
	
	if ( !Format3 ( rt, rd, sInst ) ) return false;
	
	Code = Encoder::CFC1 ( rt, rd );
	return true;
}

static bool Assembler::MTC1 ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rd;
	
	if ( !Format3 ( rt, rd, sInst ) ) return false;
	
	Code = Encoder::MTC1 ( rt, rd );
	return true;
}

static bool Assembler::CTC1 ( u32& Code, string sInst, long CurrentAddress )
{
	long rt, rd;
	
	if ( !Format3 ( rt, rd, sInst ) ) return false;
	
	Code = Encoder::CTC1 ( rt, rd );
	return true;
}


static bool Assembler::BEQL ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, rt, offset, LabelAddress;

	if ( !Format1 ( rs, rt, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BEQL ( rs, rt, offset );
	return true;
}

static bool Assembler::BNEL ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, rt, offset, LabelAddress;

	if ( !Format1 ( rs, rt, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BNEL ( rs, rt, offset );
	return true;
}


static bool Assembler::BGEZL ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, offset, LabelAddress;

	if ( !Format4 ( rs, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BGEZL ( rs, offset );
	return true;
}

static bool Assembler::BLEZL ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, offset, LabelAddress;

	if ( !Format4 ( rs, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BLEZL ( rs, offset );
	return true;
}

static bool Assembler::BGTZL ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, offset, LabelAddress;

	if ( !Format4 ( rs, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BGTZL ( rs, offset );
	return true;
}

static bool Assembler::BLTZL ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, offset, LabelAddress;

	if ( !Format4 ( rs, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BLTZL ( rs, offset );
	return true;
}

static bool Assembler::BLTZALL ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, offset, LabelAddress;

	if ( !Format4 ( rs, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BLTZALL ( rs, offset );
	return true;
}

static bool Assembler::BGEZALL ( u32& Code, string sInst, long CurrentAddress )
{
	long rs, offset, LabelAddress;

	if ( !Format4 ( rs, LabelAddress, sInst ) ) return false;

	// calculate the offset from current address and label address
	offset = ( LabelAddress - CurrentAddress - 4 ) >> 2;

	Code = Encoder::BGEZALL ( rs, offset );
	return true;
}



static bool Assembler::CACHE ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PREF ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::TGEI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TGEIU ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TLTI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TLTIU ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TEQI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TNEI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TGE ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TGEU ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TLT ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TLTU ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TEQ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TNE ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::MOVCI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MOVZ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MOVN ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::SYNC ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::MFHI1 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MTHI1 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MFLO1 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MTLO1 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MULT1 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MULTU1 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DIV1 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DIVU1 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MADD ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MADD1 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MADDU ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MADDU1 ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::MFSA ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MTSA ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MTSAB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MTSAH ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::TLBR ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TLBWI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TLBWR ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::TLBP ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::ERET ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DERET ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::WAIT ( u32& Code, string sInst, long CurrentAddress ){}


// Parallel instructions (SIMD) //
static bool Assembler::PABSH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PABSW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PADDB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PADDH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PADDW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PADDSB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PADDSH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PADDSW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PADDUB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PADDUH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PADDUW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PADSBH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PAND ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::POR ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PXOR ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PNOR ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PCEQB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PCEQH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PCEQW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PCGTB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PCGTH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PCGTW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PCPYH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PCPYLD ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PCPYUD ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PDIVBW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PDIVUW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PDIVW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PEXCH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PEXCW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PEXEH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PEXEW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PEXT5 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PEXTLB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PEXTLH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PEXTLW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PEXTUB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PEXTUH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PEXTUW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PHMADH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PHMSBH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PINTEH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PINTH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PLZCW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMADDH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMADDW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMADDUW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMAXH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMAXW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMINH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMINW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMFHI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMFLO ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMTHI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMTLO ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMFHL_LH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMFHL_SH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMFHL_LW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMFHL_UW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMFHL_SLW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMTHL_LW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMSUBH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMSUBW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMULTH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMULTW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PMULTUW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PPAC5 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PPACB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PPACH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PPACW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PREVH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PROT3W ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSLLH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSLLVW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSLLW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSRAH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSRAW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSRAVW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSRLH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSRLW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSRLVW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSUBB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSUBH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSUBW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSUBSB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSUBSH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSUBSW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSUBUB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSUBUH ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::PSUBUW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::QFSRV ( u32& Code, string sInst, long CurrentAddress ){}


// floating point instructions //

static bool Assembler::LWC1 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::SWC1 ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::ABS_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::ADD_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::ADDA_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::C_EQ_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::C_F_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::C_LE_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::C_LT_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::CVT_S_W ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::CVT_W_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::DIV_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MADD_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MADDA_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MAX_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MIN_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MOV_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MSUB_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MSUBA_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MUL_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::MULA_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::NEG_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::RSQRT_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::SQRT_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::SUB_S ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::SUBA_S ( u32& Code, string sInst, long CurrentAddress ){}


// PS2 COP2 instructions //

static bool Assembler::LQC2 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::SQC2 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::QMFC2_NI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::QMTC2_NI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::QMFC2_I ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::QMTC2_I ( u32& Code, string sInst, long CurrentAddress ){}


static bool Assembler::COP2 ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::VADDBCX ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VADDBCY ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VADDBCZ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VADDBCW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSUBBCX ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSUBBCY ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSUBBCZ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSUBBCW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMULBCX ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMULBCY ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMULBCZ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMULBCW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMADDBCX ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMADDBCY ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMADDBCZ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMADDBCW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMSUBBCX ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMSUBBCY ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMSUBBCZ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMSUBBCW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMAXBCX ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMAXBCY ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMAXBCZ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMAXBCW ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMINIBCX ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMINIBCY ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMINIBCZ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMINIBCW ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::VADD ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VADDI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VADDQ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSUB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSUBI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSUBQ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMUL ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMULI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMULQ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMAX ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMAXI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMINI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMINII ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMADD ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMADDI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMADDQ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMSUB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMSUBI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMSUBQ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VDIV ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::VADDA ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VADDAI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VADDAQ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSUBA ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VADDABC ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSUBAI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSUBAQ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSUBABC ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMULA ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMULAI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMULAQ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMULABC ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMADDA ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMADDAI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMADDAQ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMADDABC ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMSUBA ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMSUBAI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMSUBAQ ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMSUBABC ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::VOPMULA ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VOPMSUM ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VOPMSUB ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::VNOP ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VABS ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VCLIP ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSQRT ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VRSQRT ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMR32 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VRINIT ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VRGET ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VRNEXT ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VRXOR ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMOVE ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMFIR ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VMTIR ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VLQD ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VLQI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSQD ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VSQI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VWAITQ ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::VFTOI0 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VITOF0 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VFTOI4 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VITOF4 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VFTOI12 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VITOF12 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VFTOI15 ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VITOF15 ( u32& Code, string sInst, long CurrentAddress ){}

static bool Assembler::VIADD ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VISUB ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VIADDI ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VIAND ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VIOR ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VILWR ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VISWR ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VCALLMS ( u32& Code, string sInst, long CurrentAddress ){}
static bool Assembler::VCALLMSR ( u32& Code, string sInst, long CurrentAddress ){}
			






// if there is an instruction, returns its index. Otherwise returns -1
int Assembler::hasInstruction ( string Line )
{
	return Lookup::FindByName ( Line );
}



static const Assembler::Function Assembler::FunctionList []
{
	// instructions on both R3000A and R5900
	// 1 + 56 + 6 = 63 instructions //
	Assembler::Invalid,
	Assembler::J, Assembler::JAL, Assembler::JR, Assembler::JALR, Assembler::BEQ, Assembler::BNE, Assembler::BGTZ, Assembler::BGEZ,
	Assembler::BLTZ, Assembler::BLEZ, Assembler::BGEZAL, Assembler::BLTZAL, Assembler::ADD, Assembler::ADDI, Assembler::ADDU, Assembler::ADDIU,
	Assembler::SUB, Assembler::SUBU, Assembler::MULT, Assembler::MULTU, Assembler::DIV, Assembler::DIVU, Assembler::AND, Assembler::ANDI,
	Assembler::OR, Assembler::ORI, Assembler::XOR, Assembler::XORI, Assembler::NOR, Assembler::LUI, Assembler::SLL, Assembler::SRL,
	Assembler::SRA, Assembler::SLLV, Assembler::SRLV, Assembler::SRAV, Assembler::SLT, Assembler::SLTI, Assembler::SLTU, Assembler::SLTIU,
	Assembler::LB, Assembler::LBU, Assembler::LH, Assembler::LHU, Assembler::LW, Assembler::LWL, Assembler::LWR, Assembler::SB,
	Assembler::SH, Assembler::SW, Assembler::SWL, Assembler::SWR, Assembler::MFHI, Assembler::MTHI, Assembler::MFLO, Assembler::MTLO,
	Assembler::MFC0, Assembler::MTC0, Assembler::CFC2, Assembler::CTC2, Assembler::SYSCALL, Assembler::BREAK,
	
	// instructions on R3000A ONLY
	//Assembler::MFC2, Assembler::MTC2, Assembler::LWC2, Assembler::SWC2, Assembler::RFE,
	//Assembler::RTPS, Assembler::RTPT, Assembler::CC, Assembler::CDP, Assembler::DCPL, Assembler::DPCS, Assembler::DPCT, Assembler::NCS,
	//Assembler::NCT, Assembler::NCDS, Assembler::NCDT, Assembler::NCCS, Assembler::NCCT, Assembler::GPF, Assembler::GPL, Assembler::AVSZ3,
	//Assembler::AVSZ4, Assembler::SQR, Assembler::OP, Assembler::NCLIP, Assembler::INTPL, Assembler::MVMVA
	
	// instructions on R5900 ONLY
	// (24*8) + 4 + 6 = 192 + 10 = 202 instructions //
	Assembler::BEQL, Assembler::BNEL, Assembler::BGEZL, Assembler::BGTZL, Assembler::BLEZL, Assembler::BLTZL, Assembler::BGEZALL, Assembler::BLTZALL,
	Assembler::DADD, Assembler::DADDI, Assembler::DADDU, Assembler::DADDIU, Assembler::DSUB, Assembler::DSUBU, Assembler::DSLL, Assembler::DSLL32,
	Assembler::DSLLV, Assembler::DSRA, Assembler::DSRA32, Assembler::DSRAV, Assembler::DSRL, Assembler::DSRL32, Assembler::DSRLV, Assembler::LD,
	Assembler::LDL, Assembler::LDR, Assembler::LWU, Assembler::LQ, Assembler::PREF, Assembler::SD, Assembler::SDL, Assembler::SDR,
	Assembler::SQ, Assembler::TEQ, Assembler::TEQI, Assembler::TNE, Assembler::TNEI, Assembler::TGE, Assembler::TGEI, Assembler::TGEU,
	Assembler::TGEIU, Assembler::TLT, Assembler::TLTI, Assembler::TLTU, Assembler::TLTIU, Assembler::MOVN, Assembler::MOVZ, Assembler::MULT1,
	Assembler::MULTU1, Assembler::DIV1, Assembler::DIVU1, Assembler::MADD, Assembler::MADD1, Assembler::MADDU, Assembler::MADDU1, Assembler::MFHI1,
	Assembler::MTHI1, Assembler::MFLO1, Assembler::MTLO1, Assembler::MFSA, Assembler::MTSA, Assembler::MTSAB, Assembler::MTSAH,
	Assembler::PABSH, Assembler::PABSW, Assembler::PADDB, Assembler::PADDH, Assembler::PADDW, Assembler::PADDSB, Assembler::PADDSH, Assembler::PADDSW,
	Assembler::PADDUB, Assembler::PADDUH, Assembler::PADDUW, Assembler::PADSBH, Assembler::PAND, Assembler::POR, Assembler::PXOR, Assembler::PNOR,
	Assembler::PCEQB, Assembler::PCEQH, Assembler::PCEQW, Assembler::PCGTB, Assembler::PCGTH, Assembler::PCGTW, Assembler::PCPYH, Assembler::PCPYLD,
	Assembler::PCPYUD, Assembler::PDIVBW, Assembler::PDIVUW, Assembler::PDIVW, Assembler::PEXCH, Assembler::PEXCW, Assembler::PEXEH, Assembler::PEXEW,
	Assembler::PEXT5, Assembler::PEXTLB, Assembler::PEXTLH, Assembler::PEXTLW, Assembler::PEXTUB, Assembler::PEXTUH, Assembler::PEXTUW, Assembler::PHMADH,
	Assembler::PHMSBH, Assembler::PINTEH, Assembler::PINTH, Assembler::PLZCW, Assembler::PMADDH, Assembler::PMADDW, Assembler::PMADDUW, Assembler::PMAXH,
	Assembler::PMAXW, Assembler::PMINH, Assembler::PMINW, Assembler::PMFHI, Assembler::PMFLO, Assembler::PMTHI, Assembler::PMTLO, Assembler::PMFHL_LH,
	Assembler::PMFHL_SH, Assembler::PMFHL_LW, Assembler::PMFHL_UW, Assembler::PMFHL_SLW, Assembler::PMTHL_LW, Assembler::PMSUBH, Assembler::PMSUBW, Assembler::PMULTH,
	Assembler::PMULTW, Assembler::PMULTUW, Assembler::PPAC5, Assembler::PPACB, Assembler::PPACH, Assembler::PPACW, Assembler::PREVH, Assembler::PROT3W,
	Assembler::PSLLH, Assembler::PSLLVW, Assembler::PSLLW, Assembler::PSRAH, Assembler::PSRAW, Assembler::PSRAVW, Assembler::PSRLH, Assembler::PSRLW,
	Assembler::PSRLVW, Assembler::PSUBB, Assembler::PSUBH, Assembler::PSUBW, Assembler::PSUBSB, Assembler::PSUBSH, Assembler::PSUBSW, Assembler::PSUBUB,
	Assembler::PSUBUH, Assembler::PSUBUW,
	Assembler::QFSRV, Assembler::SYNC,
	
	Assembler::DI, Assembler::EI, Assembler::ERET, Assembler::CACHE, Assembler::TLBP, Assembler::TLBR, Assembler::TLBWI, Assembler::TLBWR,
	Assembler::CFC0, Assembler::CTC0,
	
	Assembler::BC0T, Assembler::BC0TL, Assembler::BC0F, Assembler::BC0FL, Assembler::BC1T, Assembler::BC1TL, Assembler::BC1F, Assembler::BC1FL,
	Assembler::BC2T, Assembler::BC2TL, Assembler::BC2F, Assembler::BC2FL,
	
	Assembler::LWC1, Assembler::SWC1, Assembler::MFC1, Assembler::MTC1, Assembler::CFC1, Assembler::CTC1,
	Assembler::ABS_S, Assembler::ADD_S, Assembler::ADDA_S, Assembler::C_EQ_S, Assembler::C_F_S, Assembler::C_LE_S, Assembler::C_LT_S, Assembler::CVT_S_W,
	Assembler::CVT_W_S, Assembler::DIV_S, Assembler::MADD_S, Assembler::MADDA_S, Assembler::MAX_S, Assembler::MIN_S, Assembler::MOV_S, Assembler::MSUB_S,
	Assembler::MSUBA_S, Assembler::MUL_S, Assembler::MULA_S, Assembler::NEG_S, Assembler::RSQRT_S, Assembler::SQRT_S, Assembler::SUB_S, Assembler::SUBA_S,
	
	Assembler::QMFC2_NI, Assembler::QMFC2_I, Assembler::QMTC2_NI, Assembler::QMTC2_I, Assembler::LQC2, Assembler::SQC2, Assembler::COP2
	//Assembler::LQC2, Assembler::SQC2, Assembler::QMFC2, Assembler::QMTC2,
	//Assembler::VADDBCX, Assembler::VADDBCY, Assembler::VADDBCZ, Assembler::VADDBCW, Assembler::VSUBBCX, Assembler::VSUBBCY, Assembler::VSUBBCZ, Assembler::VSUBBCW,
	//Assembler::VMULBCX, Assembler::VMULBCY, Assembler::VMULBCZ, Assembler::VMULBCW, Assembler::VMADDBCX, Assembler::VMADDBCY, Assembler::VMADDBCZ, Assembler::VMADDBCW,
	//Assembler::VMSUBBCX, Assembler::VMSUBBCY, Assembler::VMSUBBCZ, Assembler::VMSUBBCW, Assembler::VMAXBCX, Assembler::VMAXBCY, Assembler::VMAXBCZ, Assembler::VMAXBCW,
	//Assembler::VMINIBCX, Assembler::VMINIBCY, Assembler::VMINIBCZ, Assembler::VMINIBCW,
	
	//Assembler::VADD, Assembler::VADDI, Assembler::VADDQ, Assembler::VSUB, Assembler::VSUBI, Assembler::VSUBQ, Assembler::VMUL, Assembler::VMULI,
	//Assembler::VMULQ, Assembler::VMAX, Assembler::VMAXI, Assembler::VMINI, Assembler::VMINII, Assembler::VMADD, Assembler::VMADDI, Assembler::VMADDQ,
	//Assembler::VMSUB, Assembler::VMSUBI, Assembler::VMSUBQ, Assembler::DIV, Assembler::VADDA, Assembler::VADDAI, Assembler::VADDAQ, Assembler::VSUBA,
	//Assembler::VSUBAI, Assembler::VSUBAQ, Assembler::VMULA, Assembler::VMULAI, Assembler::VMULAQ, Assembler::VMADDA, Assembler::VMADDAI, Assembler::VMADDAQ,
	//Assembler::VMSUBA, Assembler::VMSUBAI, Assembler::VMSUBAQ, Assembler::VOPMULA, Assembler::VOPMSUM, Assembler::VNOP, Assembler::VABS, Assembler::VCLIP,
	//Assembler::VSQRT, Assembler::VRSQRT, Assembler::VMR32, Assembler::VRINIT, Assembler::VRGET, Assembler::VRNEXT, Assembler::VRXOR, Assembler::VMOVE,
	//Assembler::VMFIR, Assembler::VMTIR, Assembler::VLQD, Assembler::VLQI, Assembler::VSQD, Assembler::VSQI, Assembler::VWAITQ,
	
	//Assembler::VFTOI0, Assembler::VITOF0, Assembler::VFTOI4, Assembler::VITOF4, Assembler::VFTOI12, Assembler::VITOF12, Assembler::VFTOI15, Assembler::VITOF15,
	//Assembler::VIADD, Assembler::VISUB, Assembler::VIADDI, Assembler::VIAND, Assembler::VIOR, Assembler::VILWR, Assembler::VISWR,
	//Assembler::VCALLMS, Assembler::VCALLMSR
};






