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



#include "R5900_Print.h"

using namespace std;
using namespace R5900::Instruction;


const char Print::XyzwLUT [ 4 ] = { 'x', 'y', 'z', 'w' };
const char* Print::BCType [ 4 ] = { "F", "T", "FL", "TL" };



string Print::PrintInstruction ( long instruction )
{
	stringstream ss;
	ss.str("");
	
	//FunctionList [ Lookup::FindByInstruction ( instruction ) ] ( s, instruction );
	FunctionList [ Lookup::FindByInstruction ( instruction ) ] ( ss, instruction );
	
	return ss.str().c_str ();
}



void Print::Start ()
{
	// make sure the lookup object has started (note: this can take a LONG time for R5900 currently)
	Lookup::Start ();
}



void Print::Invalid ( stringstream &strInstString, long instruction )
{
	strInstString << "Invalid Instruction: Opcode: " << GET_OPCODE( instruction ) << ", Special: " << GET_SPECIAL( instruction ) << ", RS: " << GET_RS( instruction ) << ", RT: " << GET_RT( instruction );
}



// *** R3000A Instructions *** //



// * R3000A Arithmetic Instructions * //

void Print::ADD ( stringstream &strInstString, long instruction )
{
	strInstString << "ADD";
	AddInstArgs ( strInstString, instruction, FTADD );
}

void Print::ADDU ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDU";
	AddInstArgs ( strInstString, instruction, FTADDU );
}

void Print::SUB ( stringstream &strInstString, long instruction )
{
	strInstString << "SUB";
	AddInstArgs ( strInstString, instruction, FTSUB );
}

void Print::SUBU ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBU";
	AddInstArgs ( strInstString, instruction, FTSUBU );
}

void Print::AND ( stringstream &strInstString, long instruction )
{
	strInstString << "AND";
	AddInstArgs ( strInstString, instruction, FTAND );
}

void Print::OR ( stringstream &strInstString, long instruction )
{
	strInstString << "OR";
	AddInstArgs ( strInstString, instruction, FTOR );
}

void Print::XOR ( stringstream &strInstString, long instruction )
{
	strInstString << "XOR";
	AddInstArgs ( strInstString, instruction, FTXOR );
}

void Print::NOR ( stringstream &strInstString, long instruction )
{
	strInstString << "NOR";
	AddInstArgs ( strInstString, instruction, FTNOR );
}



void Print::SLT ( stringstream &strInstString, long instruction )
{
	strInstString << "SLT";
	AddInstArgs ( strInstString, instruction, FTSLT );
}

void Print::SLTU ( stringstream &strInstString, long instruction )
{
	strInstString << "SLTU";
	AddInstArgs ( strInstString, instruction, FTSLTU );
}


void Print::ADDI ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDI";
	AddInstArgs ( strInstString, instruction, FTADDI );
}

void Print::ADDIU ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDIU";
	AddInstArgs ( strInstString, instruction, FTADDIU );
}

void Print::ANDI ( stringstream &strInstString, long instruction )
{
	strInstString << "ANDI";
	AddInstArgs ( strInstString, instruction, FTANDI );
}

void Print::ORI ( stringstream &strInstString, long instruction )
{
	strInstString << "ORI";
	AddInstArgs ( strInstString, instruction, FTORI );
}

void Print::XORI ( stringstream &strInstString, long instruction )
{
	strInstString << "XORI";
	AddInstArgs ( strInstString, instruction, FTXORI );
}

void Print::MULT ( stringstream &strInstString, long instruction )
{
	strInstString << "MULT";
	AddInstArgs ( strInstString, instruction, FTMULT1 );
}

void Print::MULTU ( stringstream &strInstString, long instruction )
{
	strInstString << "MULTU";
	AddInstArgs ( strInstString, instruction, FTMULTU1 );
}

void Print::DIV ( stringstream &strInstString, long instruction )
{
	strInstString << "DIV";
	AddInstArgs ( strInstString, instruction, FTDIV );
}

void Print::DIVU ( stringstream &strInstString, long instruction )
{
	strInstString << "DIVU";
	AddInstArgs ( strInstString, instruction, FTDIVU );
}


void Print::SLTI ( stringstream &strInstString, long instruction )
{
	strInstString << "SLTI";
	AddInstArgs ( strInstString, instruction, FTSLTI );
}

void Print::SLTIU ( stringstream &strInstString, long instruction )
{
	strInstString << "SLTIU";
	AddInstArgs ( strInstString, instruction, FTSLTIU );
}


void Print::LUI ( stringstream &strInstString, long instruction )
{
	strInstString << "LUI";
	AddInstArgs ( strInstString, instruction, FTLUI );
}



void Print::SLL ( stringstream &strInstString, long instruction )
{
	strInstString << "SLL";
	AddInstArgs ( strInstString, instruction, FTSLL );
}


void Print::SRA ( stringstream &strInstString, long instruction )
{
	strInstString << "SRA";
	AddInstArgs ( strInstString, instruction, FTSRA );
}

void Print::SRL ( stringstream &strInstString, long instruction )
{
	strInstString << "SRL";
	AddInstArgs ( strInstString, instruction, FTSRL );
}

void Print::SLLV ( stringstream &strInstString, long instruction )
{
	strInstString << "SLLV";
	AddInstArgs ( strInstString, instruction, FTSLLV );
}

void Print::SRLV ( stringstream &strInstString, long instruction )
{
	strInstString << "SRLV";
	AddInstArgs ( strInstString, instruction, FTSRLV );
}

void Print::SRAV ( stringstream &strInstString, long instruction )
{
	strInstString << "SRAV";
	AddInstArgs ( strInstString, instruction, FTSRAV );
}


// * R3000A Load/Store Instructions * //


void Print::MFC0 ( stringstream &strInstString, long instruction )
{
	strInstString << "MFC0";	// mind as well make it MFC since debug instructions won't be encountered
	AddInstArgs ( strInstString, instruction, FORMAT19 );
}

void Print::MTC0 ( stringstream &strInstString, long instruction )
{
	strInstString << "MTC0";	// mind as well make it MTC since debug instructions won't be encountered
	AddInstArgs ( strInstString, instruction, FORMAT19 );
}


void Print::LB ( stringstream &strInstString, long instruction )
{
	strInstString << "LB";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void Print::LBU ( stringstream &strInstString, long instruction )
{
	strInstString << "LBU";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}


void Print::LH ( stringstream &strInstString, long instruction )
{
	strInstString << "LH";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void Print::LHU ( stringstream &strInstString, long instruction )
{
	strInstString << "LHU";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void Print::LWR ( stringstream &strInstString, long instruction )
{
	strInstString << "LWR";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void Print::LWL ( stringstream &strInstString, long instruction )
{
	strInstString << "LWL";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}


void Print::SB ( stringstream &strInstString, long instruction )
{
	strInstString << "SB";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}


void Print::SH ( stringstream &strInstString, long instruction )
{
	strInstString << "SH";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void Print::SWL ( stringstream &strInstString, long instruction )
{
	strInstString << "SWL";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void Print::SWR ( stringstream &strInstString, long instruction )
{
	strInstString << "SWR";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}


void Print::SW ( stringstream &strInstString, long instruction )
{
	strInstString << "SW";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void Print::LW ( stringstream &strInstString, long instruction )
{
	strInstString << "LW";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}



void Print::MFHI ( stringstream &strInstString, long instruction )
{
	strInstString << "MFHI";
	AddInstArgs ( strInstString, instruction, FTMFHI );
}

void Print::MTHI ( stringstream &strInstString, long instruction )
{
	strInstString << "MTHI";
	AddInstArgs ( strInstString, instruction, FTMTHI );
}

void Print::MFLO ( stringstream &strInstString, long instruction )
{
	strInstString << "MFLO";
	AddInstArgs ( strInstString, instruction, FTMFLO );
}

void Print::MTLO ( stringstream &strInstString, long instruction )
{
	strInstString << "MTLO";
	AddInstArgs ( strInstString, instruction, FTMTLO );
}



// * R3000A Branch Instructions * //


void Print::J ( stringstream &strInstString, long instruction )
{
	strInstString << "J";
	AddInstArgs ( strInstString, instruction, FTJ );
}

void Print::JAL ( stringstream &strInstString, long instruction )
{
	strInstString << "JAL";
	AddInstArgs ( strInstString, instruction, FTJAL );
}


void Print::JR ( stringstream &strInstString, long instruction )
{
	strInstString << "JR";
	AddInstArgs ( strInstString, instruction, FTJR );
}

void Print::JALR ( stringstream &strInstString, long instruction )
{
	strInstString << "JALR";
	AddInstArgs ( strInstString, instruction, FTJALR );
}



void Print::BEQ ( stringstream &strInstString, long instruction )
{
	strInstString << "BEQ";
	AddInstArgs ( strInstString, instruction, FTBEQ );
}

void Print::BNE ( stringstream &strInstString, long instruction )
{
	strInstString << "BNE";
	AddInstArgs ( strInstString, instruction, FTBNE );
}

void Print::BLEZ ( stringstream &strInstString, long instruction )
{
	strInstString << "BLEZ";
	AddInstArgs ( strInstString, instruction, FTBLEZ );
}

void Print::BGTZ ( stringstream &strInstString, long instruction )
{
	strInstString << "BGTZ";
	AddInstArgs ( strInstString, instruction, FTBGTZ );
}

void Print::BLTZ ( stringstream &strInstString, long instruction )
{
	strInstString << "BLTZ";
	AddInstArgs ( strInstString, instruction, FTBLTZ );
}

void Print::BGEZ ( stringstream &strInstString, long instruction )
{
	strInstString << "BGEZ";
	AddInstArgs ( strInstString, instruction, FTBGEZ );
}





// *** R5900 Instructions *** //


// arithemetic instructions //

void Print::DADD ( stringstream &strInstString, long instruction )
{
	strInstString << "DADD";
	AddInstArgs ( strInstString, instruction, FTDADD );
}

void Print::DADDI ( stringstream &strInstString, long instruction )
{
	strInstString << "DADDI";
	AddInstArgs ( strInstString, instruction, FTDADDI );
}

void Print::DADDU ( stringstream &strInstString, long instruction )
{
	strInstString << "DADDU";
	AddInstArgs ( strInstString, instruction, FTDADDU );
}

void Print::DADDIU ( stringstream &strInstString, long instruction )
{
	strInstString << "DADDIU";
	AddInstArgs ( strInstString, instruction, FTDADDIU );
}

void Print::DSUB ( stringstream &strInstString, long instruction )
{
	strInstString << "DSUB";
	AddInstArgs ( strInstString, instruction, FTDSUB );
}

void Print::DSUBU ( stringstream &strInstString, long instruction )
{
	strInstString << "DSUBU";
	AddInstArgs ( strInstString, instruction, FTDSUBU );
}

void Print::DSLL ( stringstream &strInstString, long instruction )
{
	strInstString << "DSLL";
	AddInstArgs ( strInstString, instruction, FTDSLL );
}

void Print::DSLL32 ( stringstream &strInstString, long instruction )
{
	strInstString << "DSLL32";
	AddInstArgs ( strInstString, instruction, FTDSLL32 );
}

void Print::DSLLV ( stringstream &strInstString, long instruction )
{
	strInstString << "DSLLV";
	AddInstArgs ( strInstString, instruction, FTDSLLV );
}

void Print::DSRA ( stringstream &strInstString, long instruction )
{
	strInstString << "DSRA";
	AddInstArgs ( strInstString, instruction, FTDSRA );
}

void Print::DSRA32 ( stringstream &strInstString, long instruction )
{
	strInstString << "DSRA32";
	AddInstArgs ( strInstString, instruction, FTDSRA32 );
}

void Print::DSRAV ( stringstream &strInstString, long instruction )
{
	strInstString << "DSRAV";
	AddInstArgs ( strInstString, instruction, FTDSRAV );
}

void Print::DSRL ( stringstream &strInstString, long instruction )
{
	strInstString << "DSRL";
	AddInstArgs ( strInstString, instruction, FTDSRL );
}

void Print::DSRL32 ( stringstream &strInstString, long instruction )
{
	strInstString << "DSRL32";
	AddInstArgs ( strInstString, instruction, FTDSRL32 );
}

void Print::DSRLV ( stringstream &strInstString, long instruction )
{
	strInstString << "DSRLV";
	AddInstArgs ( strInstString, instruction, FTDSRLV );
}


void Print::MULT1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MULT1";
	AddInstArgs ( strInstString, instruction, FTMULT1 );
}

void Print::MULTU1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MULTU1";
	AddInstArgs ( strInstString, instruction, FTMULTU1 );
}

void Print::DIV1 ( stringstream &strInstString, long instruction )
{
	strInstString << "DIV1";
	AddInstArgs ( strInstString, instruction, FTDIV1 );
}

void Print::DIVU1 ( stringstream &strInstString, long instruction )
{
	strInstString << "DIVU1";
	AddInstArgs ( strInstString, instruction, FTDIVU1 );
}

void Print::MADD ( stringstream &strInstString, long instruction )
{
	strInstString << "MADD";
	AddInstArgs ( strInstString, instruction, FTMADD );
}

void Print::MADD1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MADD1";
	AddInstArgs ( strInstString, instruction, FTMADD1 );
}

void Print::MADDU ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDU";
	AddInstArgs ( strInstString, instruction, FTMADDU );
}

void Print::MADDU1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDU1";
	AddInstArgs ( strInstString, instruction, FTMADDU1 );
}



// Load/Store instructions //

void Print::SD ( stringstream &strInstString, long instruction )
{
	strInstString << "SD";
	AddInstArgs ( strInstString, instruction, FTSD );
}

void Print::LD ( stringstream &strInstString, long instruction )
{
	strInstString << "LD";
	AddInstArgs ( strInstString, instruction, FTLD );
}

void Print::LWU ( stringstream &strInstString, long instruction )
{
	strInstString << "LWU";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void Print::SDL ( stringstream &strInstString, long instruction )
{
	strInstString << "SDL";
	AddInstArgs ( strInstString, instruction, FTSDL );
}

void Print::SDR ( stringstream &strInstString, long instruction )
{
	strInstString << "SDR";
	AddInstArgs ( strInstString, instruction, FTSDR );
}

void Print::LDL ( stringstream &strInstString, long instruction )
{
	strInstString << "LDL";
	AddInstArgs ( strInstString, instruction, FTLDL );
}

void Print::LDR ( stringstream &strInstString, long instruction )
{
	strInstString << "LDR";
	AddInstArgs ( strInstString, instruction, FTLDR );
}

void Print::LQ ( stringstream &strInstString, long instruction )
{
	strInstString << "LQ";
	AddInstArgs ( strInstString, instruction, FTLQ );
}

void Print::SQ ( stringstream &strInstString, long instruction )
{
	strInstString << "SQ";
	AddInstArgs ( strInstString, instruction, FTSQ );
}


void Print::MOVZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MOVZ";
	AddInstArgs ( strInstString, instruction, FTMOVZ );
}

void Print::MOVN ( stringstream &strInstString, long instruction )
{
	strInstString << "MOVN";
	AddInstArgs ( strInstString, instruction, FTMOVN );
}


void Print::MFHI1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MFHI1";
	AddInstArgs ( strInstString, instruction, FTMFHI1 );
}

void Print::MTHI1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MTHI1";
	AddInstArgs ( strInstString, instruction, FTMTHI1 );
}

void Print::MFLO1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MFLO1";
	AddInstArgs ( strInstString, instruction, FTMFLO1 );
}

void Print::MTLO1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MTLO1";
	AddInstArgs ( strInstString, instruction, FTMTLO1 );
}



void Print::MFSA ( stringstream &strInstString, long instruction )
{
	strInstString << "MFSA";
	AddInstArgs ( strInstString, instruction, FTMFSA );
}

void Print::MTSA ( stringstream &strInstString, long instruction )
{
	strInstString << "MTSA";
	AddInstArgs ( strInstString, instruction, FTMTSA );
}

void Print::MTSAB ( stringstream &strInstString, long instruction )
{
	strInstString << "MTSAB";
	AddInstArgs ( strInstString, instruction, FTMTSA );
}

void Print::MTSAH ( stringstream &strInstString, long instruction )
{
	strInstString << "MTSAH";
	AddInstArgs ( strInstString, instruction, FTMTSA );
}




// Branch instructions //

void Print::BEQL ( stringstream &strInstString, long instruction )
{
	strInstString << "BEQL";
	AddInstArgs ( strInstString, instruction, FTBEQL );
}

void Print::BNEL ( stringstream &strInstString, long instruction )
{
	strInstString << "BNEL";
	AddInstArgs ( strInstString, instruction, FTBNEL );
}

void Print::BGEZL ( stringstream &strInstString, long instruction )
{
	strInstString << "BGEZL";
	AddInstArgs ( strInstString, instruction, FTBGTZL );
}

void Print::BLEZL ( stringstream &strInstString, long instruction )
{
	strInstString << "BLEZL";
	AddInstArgs ( strInstString, instruction, FTBLEZL );
}

void Print::BGTZL ( stringstream &strInstString, long instruction )
{
	strInstString << "BGTZL";
	AddInstArgs ( strInstString, instruction, FTBGTZL );
}

void Print::BLTZL ( stringstream &strInstString, long instruction )
{
	strInstString << "BLTZL";
	AddInstArgs ( strInstString, instruction, FTBLTZL );
}

void Print::BLTZAL ( stringstream &strInstString, long instruction )
{
	strInstString << "BLTZAL";
	AddInstArgs ( strInstString, instruction, FTBLTZAL );
}

void Print::BGEZAL ( stringstream &strInstString, long instruction )
{
	strInstString << "BGEZAL";
	AddInstArgs ( strInstString, instruction, FTBGEZAL );
}

void Print::BLTZALL ( stringstream &strInstString, long instruction )
{
	strInstString << "BLTZALL";
	AddInstArgs ( strInstString, instruction, FTBLTZALL );
}

void Print::BGEZALL ( stringstream &strInstString, long instruction )
{
	strInstString << "BGEZALL";
	AddInstArgs ( strInstString, instruction, FTBGEZALL );
}




void Print::BC0T ( stringstream &strInstString, long instruction )
{
	strInstString << "BC0T";
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}

void Print::BC0TL ( stringstream &strInstString, long instruction )
{
	strInstString << "BC0TL";
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}

void Print::BC0F ( stringstream &strInstString, long instruction )
{
	strInstString << "BC0F";
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}

void Print::BC0FL ( stringstream &strInstString, long instruction )
{
	strInstString << "BC0FL";
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}

void Print::BC1T ( stringstream &strInstString, long instruction )
{
	strInstString << "BC1T";
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}

void Print::BC1TL ( stringstream &strInstString, long instruction )
{
	strInstString << "BC1TL";
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}

void Print::BC1F ( stringstream &strInstString, long instruction )
{
	strInstString << "BC1F";
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}

void Print::BC1FL ( stringstream &strInstString, long instruction )
{
	strInstString << "BC1FL";
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}

void Print::BC2T ( stringstream &strInstString, long instruction )
{
	strInstString << "BC2T";
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}

void Print::BC2TL ( stringstream &strInstString, long instruction )
{
	strInstString << "BC2TL";
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}

void Print::BC2F ( stringstream &strInstString, long instruction )
{
	strInstString << "BC2F";
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}

void Print::BC2FL ( stringstream &strInstString, long instruction )
{
	strInstString << "BC2FL";
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}






void Print::TGEI ( stringstream &strInstString, long instruction )
{
	strInstString << "TGEI";
	AddInstArgs ( strInstString, instruction, FTTGEI );
}

void Print::TGEIU ( stringstream &strInstString, long instruction )
{
	strInstString << "TGEIU";
	AddInstArgs ( strInstString, instruction, FTTGEIU );
}

void Print::TLTI ( stringstream &strInstString, long instruction )
{
	strInstString << "TLTI";
	AddInstArgs ( strInstString, instruction, FTTLTI );
}

void Print::TLTIU ( stringstream &strInstString, long instruction )
{
	strInstString << "TLTIU";
	AddInstArgs ( strInstString, instruction, FTTLTIU );
}

void Print::TEQI ( stringstream &strInstString, long instruction )
{
	strInstString << "TEQI";
	AddInstArgs ( strInstString, instruction, FTTEQI );
}

void Print::TNEI ( stringstream &strInstString, long instruction )
{
	strInstString << "TNEI";
	AddInstArgs ( strInstString, instruction, FTTNEI );
}


void Print::TGE ( stringstream &strInstString, long instruction )
{
	strInstString << "TGE";
	AddInstArgs ( strInstString, instruction, FTTGE );
}

void Print::TGEU ( stringstream &strInstString, long instruction )
{
	strInstString << "TGEU";
	AddInstArgs ( strInstString, instruction, FTTGEU );
}

void Print::TLT ( stringstream &strInstString, long instruction )
{
	strInstString << "TLT";
	AddInstArgs ( strInstString, instruction, FTTLT );
}

void Print::TLTU ( stringstream &strInstString, long instruction )
{
	strInstString << "TLTU";
	AddInstArgs ( strInstString, instruction, FTTLTU );
}

void Print::TEQ ( stringstream &strInstString, long instruction )
{
	strInstString << "TEQ";
	AddInstArgs ( strInstString, instruction, FTTEQ );
}

void Print::TNE ( stringstream &strInstString, long instruction )
{
	strInstString << "TNE";
	AddInstArgs ( strInstString, instruction, FTTNE );
}




// PS1 may be able to use this, so I'll just comment it out for now
//void Print::MFC2 ( stringstream &strInstString, long instruction )
//{
//	strInstString << "MFC2";
//	AddInstArgs ( strInstString, instruction, FTMFC2 );
//}

// PS1 may be able to use this, so I'll just comment it out for now
//void Print::MTC2 ( stringstream &strInstString, long instruction )
//{
//	strInstString << "MTC2";
//	AddInstArgs ( strInstString, instruction, FTMTC2 );
//}


// PS2 has QMFC2/QMTC2 instead of MFC2/MTC2 //
//void Print::MFC2 ( stringstream &strInstString, long instruction )
//{
//	strInstString << "MFC2";
//	AddInstArgs ( strInstString, instruction, FTQMFC2 );
//}
//void Print::MTC2 ( stringstream &strInstString, long instruction )
//{
//	strInstString << "MTC2";
//	AddInstArgs ( strInstString, instruction, FTQMTC2 );
//}














// * R5900 Parallel (SIMD) instructions * //


void Print::PLZCW ( stringstream &strInstString, long instruction )
{
	strInstString << "PLZCW";
	AddInstArgs ( strInstString, instruction, FTPLZCW );
}


void Print::PMFHL_LH ( stringstream &strInstString, long instruction )
{
	strInstString << "PMFHL.LH";
	AddInstArgs ( strInstString, instruction, FTPMFHLLH );
}

void Print::PMFHL_SH ( stringstream &strInstString, long instruction )
{
	strInstString << "PMFHL.SH";
	AddInstArgs ( strInstString, instruction, FTPMFHLSH );
}

void Print::PMFHL_LW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMFHL.LW";
	AddInstArgs ( strInstString, instruction, FTPMFHLLW );
}

void Print::PMFHL_UW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMFHL.UW";
	AddInstArgs ( strInstString, instruction, FTPMFHLUW );
}

void Print::PMFHL_SLW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMFHL.SLW";
	AddInstArgs ( strInstString, instruction, FTPMFHLSLW );
}

void Print::PMTHL_LW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMTHL.LW";
	AddInstArgs ( strInstString, instruction, FTPMTHLLW );
}


void Print::PSLLH ( stringstream &strInstString, long instruction )
{
	strInstString << "PSLLH";
	AddInstArgs ( strInstString, instruction, FTPSLLH );
}

void Print::PSLLW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSLLW";
	AddInstArgs ( strInstString, instruction, FTPSLLW );
}

void Print::PSRLH ( stringstream &strInstString, long instruction )
{
	strInstString << "PSRLH";
	AddInstArgs ( strInstString, instruction, FTPSRLH );
}

void Print::PSRLW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSRLW";
	AddInstArgs ( strInstString, instruction, FTPSRLW );
}


void Print::PSRAH ( stringstream &strInstString, long instruction )
{
	strInstString << "PSRAH";
	AddInstArgs ( strInstString, instruction, FTPSRAH );
}

void Print::PSRAW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSRAW";
	AddInstArgs ( strInstString, instruction, FTPSRAW );
}



void Print::PADDW ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDW";
	AddInstArgs ( strInstString, instruction, FTPADDW );
}

void Print::PSUBW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBW";
	AddInstArgs ( strInstString, instruction, FTPSUBW );
}

void Print::PCGTW ( stringstream &strInstString, long instruction )
{
	strInstString << "PCGTW";
	AddInstArgs ( strInstString, instruction, FTPCGTW );
}

void Print::PMAXW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMAXW";
	AddInstArgs ( strInstString, instruction, FTPMAXW );
}

void Print::PADDH ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDH";
	AddInstArgs ( strInstString, instruction, FTPADDH );
}

void Print::PSUBH ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBH";
	AddInstArgs ( strInstString, instruction, FTPSUBH );
}

void Print::PCGTH ( stringstream &strInstString, long instruction )
{
	strInstString << "PCGTH";
	AddInstArgs ( strInstString, instruction, FTPCGTH );
}

void Print::PMAXH ( stringstream &strInstString, long instruction )
{
	strInstString << "PMAXH";
	AddInstArgs ( strInstString, instruction, FTPMAXH );
}

void Print::PADDB ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDB";
	AddInstArgs ( strInstString, instruction, FTPADDB );
}

void Print::PSUBB ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBB";
	AddInstArgs ( strInstString, instruction, FTPSUBB );
}

void Print::PCGTB ( stringstream &strInstString, long instruction )
{
	strInstString << "PCGTB";
	AddInstArgs ( strInstString, instruction, FTPCGTB );
}

void Print::PADDSW ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDSW";
	AddInstArgs ( strInstString, instruction, FTPADDSW );
}

void Print::PSUBSW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBSW";
	AddInstArgs ( strInstString, instruction, FTPSUBSW );
}

void Print::PEXTLW ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXTLW";
	AddInstArgs ( strInstString, instruction, FTPEXTLW );
}

void Print::PPACW ( stringstream &strInstString, long instruction )
{
	strInstString << "PPACW";
	AddInstArgs ( strInstString, instruction, FTPPACW );
}

void Print::PADDSH ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDSH";
	AddInstArgs ( strInstString, instruction, FTPADDSH );
}

void Print::PSUBSH ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBSH";
	AddInstArgs ( strInstString, instruction, FTPSUBSH );
}

void Print::PEXTLH ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXTLH";
	AddInstArgs ( strInstString, instruction, FTPEXTLH );
}

void Print::PPACH ( stringstream &strInstString, long instruction )
{
	strInstString << "PPACH";
	AddInstArgs ( strInstString, instruction, FTPPACH );
}

void Print::PADDSB ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDSB";
	AddInstArgs ( strInstString, instruction, FTPADDSB );
}

void Print::PSUBSB ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBSB";
	AddInstArgs ( strInstString, instruction, FTPSUBSB );
}

void Print::PEXTLB ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXTLB";
	AddInstArgs ( strInstString, instruction, FTPEXTLB );
}

void Print::PPACB ( stringstream &strInstString, long instruction )
{
	strInstString << "PPACB";
	AddInstArgs ( strInstString, instruction, FTPPACB );
}

void Print::PEXT5 ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXT5";
	AddInstArgs ( strInstString, instruction, FTPEXT5 );
}

void Print::PPAC5 ( stringstream &strInstString, long instruction )
{
	strInstString << "PPAC5";
	AddInstArgs ( strInstString, instruction, FTPPAC5 );
}

void Print::PABSW ( stringstream &strInstString, long instruction )
{
	strInstString << "PABSW";
	AddInstArgs ( strInstString, instruction, FTPABSW );
}

void Print::PCEQW ( stringstream &strInstString, long instruction )
{
	strInstString << "PCEQW";
	AddInstArgs ( strInstString, instruction, FTPCEQW );
}

void Print::PMINW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMINW";
	AddInstArgs ( strInstString, instruction, FTPMINW );
}

void Print::PADSBH ( stringstream &strInstString, long instruction )
{
	strInstString << "PADSBH";
	AddInstArgs ( strInstString, instruction, FTPADSBH );
}

void Print::PABSH ( stringstream &strInstString, long instruction )
{
	strInstString << "PABSH";
	AddInstArgs ( strInstString, instruction, FTPABSH );
}

void Print::PCEQH ( stringstream &strInstString, long instruction )
{
	strInstString << "PCEQH";
	AddInstArgs ( strInstString, instruction, FTPCEQH );
}

void Print::PMINH ( stringstream &strInstString, long instruction )
{
	strInstString << "PMINH";
	AddInstArgs ( strInstString, instruction, FTPMINH );
}

void Print::PCEQB ( stringstream &strInstString, long instruction )
{
	strInstString << "PCEQB";
	AddInstArgs ( strInstString, instruction, FTPCEQB );
}

void Print::PADDUW ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDUW";
	AddInstArgs ( strInstString, instruction, FTPADDUW );
}

void Print::PSUBUW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBUW";
	AddInstArgs ( strInstString, instruction, FTPSUBUW );
}

void Print::PEXTUW ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXTUW";
	AddInstArgs ( strInstString, instruction, FTPEXTUW );
}

void Print::PADDUH ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDUH";
	AddInstArgs ( strInstString, instruction, FTPADDUH );
}

void Print::PSUBUH ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBUH";
	AddInstArgs ( strInstString, instruction, FTPSUBUH );
}

void Print::PEXTUH ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXTUH";
	AddInstArgs ( strInstString, instruction, FTPEXTUH );
}

void Print::PADDUB ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDUB";
	AddInstArgs ( strInstString, instruction, FTPADDUB );
}

void Print::PSUBUB ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBUB";
	AddInstArgs ( strInstString, instruction, FTPSUBUB );
}

void Print::PEXTUB ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXTUB";
	AddInstArgs ( strInstString, instruction, FTPEXTUB );
}

void Print::QFSRV ( stringstream &strInstString, long instruction )
{
	strInstString << "QFSRV";
	AddInstArgs ( strInstString, instruction, FTQFSRV );
}

void Print::PMADDW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMADDW";
	AddInstArgs ( strInstString, instruction, FTPMADDW );
}

void Print::PSLLVW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSLLVW";
	AddInstArgs ( strInstString, instruction, FTPSLLVW );
}

void Print::PSRLVW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSRLVW";
	AddInstArgs ( strInstString, instruction, FTPSRLVW );
}

void Print::PMSUBW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMSUBW";
	AddInstArgs ( strInstString, instruction, FTPMSUBW );
}

void Print::PMFHI ( stringstream &strInstString, long instruction )
{
	strInstString << "PMFHI";
	AddInstArgs ( strInstString, instruction, FTPMFHI );
}

void Print::PMFLO ( stringstream &strInstString, long instruction )
{
	strInstString << "PMFLO";
	AddInstArgs ( strInstString, instruction, FTPMFLO );
}

void Print::PINTH ( stringstream &strInstString, long instruction )
{
	strInstString << "PINTH";
	AddInstArgs ( strInstString, instruction, FTPINTH );
}

void Print::PMULTW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMULTW";
	AddInstArgs ( strInstString, instruction, FTPMULTW );
}

void Print::PDIVW ( stringstream &strInstString, long instruction )
{
	strInstString << "PDIVW";
	AddInstArgs ( strInstString, instruction, FTPDIVW );
}

void Print::PCPYLD ( stringstream &strInstString, long instruction )
{
	strInstString << "PCPYLD";
	AddInstArgs ( strInstString, instruction, FTPCPYLD );
}

void Print::PMADDH ( stringstream &strInstString, long instruction )
{
	strInstString << "PMADDH";
	AddInstArgs ( strInstString, instruction, FTPMADDH );
}

void Print::PHMADH ( stringstream &strInstString, long instruction )
{
	strInstString << "PHMADH";
	AddInstArgs ( strInstString, instruction, FTPHMADH );
}

void Print::PAND ( stringstream &strInstString, long instruction )
{
	strInstString << "PAND";
	AddInstArgs ( strInstString, instruction, FTPAND );
}

void Print::PXOR ( stringstream &strInstString, long instruction )
{
	strInstString << "PXOR";
	AddInstArgs ( strInstString, instruction, FTPXOR );
}

void Print::PMSUBH ( stringstream &strInstString, long instruction )
{
	strInstString << "PMSUBH";
	AddInstArgs ( strInstString, instruction, FTPMSUBH );
}

void Print::PHMSBH ( stringstream &strInstString, long instruction )
{
	strInstString << "PHMSBH";
	AddInstArgs ( strInstString, instruction, FTPHMSBH );
}

void Print::PEXEH ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXEH";
	AddInstArgs ( strInstString, instruction, FTPEXEH );
}

void Print::PREVH ( stringstream &strInstString, long instruction )
{
	strInstString << "PREVH";
	AddInstArgs ( strInstString, instruction, FTPREVH );
}

void Print::PMULTH ( stringstream &strInstString, long instruction )
{
	strInstString << "PMULTH";
	AddInstArgs ( strInstString, instruction, FTPMULTH );
}

void Print::PDIVBW ( stringstream &strInstString, long instruction )
{
	strInstString << "PDIVBW";
	AddInstArgs ( strInstString, instruction, FTPDIVBW );
}

void Print::PEXEW ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXEW";
	AddInstArgs ( strInstString, instruction, FTPEXEW );
}

void Print::PROT3W ( stringstream &strInstString, long instruction )
{
	strInstString << "PROT3W";
	AddInstArgs ( strInstString, instruction, FTPROT3W );
}

void Print::PMADDUW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMADDUW";
	AddInstArgs ( strInstString, instruction, FTPMADDUW );
}

void Print::PSRAVW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSRAVW";
	AddInstArgs ( strInstString, instruction, FTPSRAVW );
}

void Print::PMTHI ( stringstream &strInstString, long instruction )
{
	strInstString << "PMTHI";
	AddInstArgs ( strInstString, instruction, FTPMTHI );
}

void Print::PMTLO ( stringstream &strInstString, long instruction )
{
	strInstString << "PMTLO";
	AddInstArgs ( strInstString, instruction, FTPMTLO );
}

void Print::PINTEH ( stringstream &strInstString, long instruction )
{
	strInstString << "PINTEH";
	AddInstArgs ( strInstString, instruction, FTPINTEH );
}

void Print::PMULTUW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMULTUW";
	AddInstArgs ( strInstString, instruction, FTPMULTUW );
}

void Print::PDIVUW ( stringstream &strInstString, long instruction )
{
	strInstString << "PDIVUW";
	AddInstArgs ( strInstString, instruction, FTPDIVUW );
}

void Print::PCPYUD ( stringstream &strInstString, long instruction )
{
	strInstString << "PCPYUD";
	AddInstArgs ( strInstString, instruction, FTPCPYUD );
}

void Print::POR ( stringstream &strInstString, long instruction )
{
	strInstString << "POR";
	AddInstArgs ( strInstString, instruction, FTPOR );
}

void Print::PNOR ( stringstream &strInstString, long instruction )
{
	strInstString << "PNOR";
	AddInstArgs ( strInstString, instruction, FTPNOR );
}

void Print::PEXCH ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXCH";
	AddInstArgs ( strInstString, instruction, FTPEXCH );
}

void Print::PCPYH ( stringstream &strInstString, long instruction )
{
	strInstString << "PCPYH";
	AddInstArgs ( strInstString, instruction, FTPCPYH );
}

void Print::PEXCW ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXCW";
	AddInstArgs ( strInstString, instruction, FTPEXCW );
}





// * R5900 COP0 instructions * //


void Print::EI ( stringstream &strInstString, long instruction )
{
	strInstString << "EI";
}

void Print::DI ( stringstream &strInstString, long instruction )
{
	strInstString << "DI";
}


void Print::CFC0 ( stringstream &strInstString, long instruction )
{
	strInstString << "CFC0";
	AddInstArgs ( strInstString, instruction, FORMAT19 );
}

void Print::CTC0 ( stringstream &strInstString, long instruction )
{
	strInstString << "CTC0";
	AddInstArgs ( strInstString, instruction, FORMAT19 );
}


void Print::SYSCALL ( stringstream &strInstString, long instruction )
{
	strInstString << "SYSCALL";
	AddInstArgs ( strInstString, instruction, FTSYSCALL );
}

void Print::BREAK ( stringstream &strInstString, long instruction )
{
	strInstString << "BREAK";
	AddInstArgs ( strInstString, instruction, FTBREAK );
}


void Print::SYNC ( stringstream &strInstString, long instruction )
{
	strInstString << "SYNC";
//	AddInstArgs ( strInstString, instruction, FTSYNC );
}


void Print::CACHE ( stringstream &strInstString, long instruction )
{
	strInstString << "CACHE";
	AddInstArgs ( strInstString, instruction, FTCACHE );
}

void Print::PREF ( stringstream &strInstString, long instruction )
{
	strInstString << "PREF";
	AddInstArgs ( strInstString, instruction, FTPREF );
}

void Print::TLBR ( stringstream &strInstString, long instruction )
{
	strInstString << "TLBR";
	AddInstArgs ( strInstString, instruction, FTTLBR );
}

void Print::TLBWI ( stringstream &strInstString, long instruction )
{
	strInstString << "TLBWI";
	AddInstArgs ( strInstString, instruction, FTTLBWI );
}

void Print::TLBWR ( stringstream &strInstString, long instruction )
{
	strInstString << "TLBWR";
	AddInstArgs ( strInstString, instruction, FTTLBWR );
}

void Print::TLBP ( stringstream &strInstString, long instruction )
{
	strInstString << "TLBP";
	AddInstArgs ( strInstString, instruction, FTTLBP );
}

void Print::ERET ( stringstream &strInstString, long instruction )
{
	strInstString << "ERET";
	AddInstArgs ( strInstString, instruction, FTERET );
}

// R5900 does not have RFE instruction //
//void Print::RFE ( stringstream &strInstString, long instruction )
//{
//	strInstString << "RFE";
//	AddInstArgs ( strInstString, instruction, FTRFE );
//}


void Print::DERET ( stringstream &strInstString, long instruction )
{
	strInstString << "DERET";
	//AddInstArgs ( strInstString, instruction, FTDERET );
}

void Print::WAIT ( stringstream &strInstString, long instruction )
{
	strInstString << "WAIT";
	//AddInstArgs ( strInstString, instruction, FTWAIT );
}





// * COP1 (floating point) instructions * //


void Print::MFC1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MFC1";
	AddInstArgs ( strInstString, instruction, FTMFC1 );
}

void Print::MTC1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MTC1";
	AddInstArgs ( strInstString, instruction, FTMTC1 );
}

void Print::CFC1 ( stringstream &strInstString, long instruction )
{
	strInstString << "CFC1";
	AddInstArgs ( strInstString, instruction, FTCFC1 );
}

void Print::CTC1 ( stringstream &strInstString, long instruction )
{
	strInstString << "CTC1";
	AddInstArgs ( strInstString, instruction, FTCTC1 );
}


void Print::LWC1 ( stringstream &strInstString, long instruction )
{
	strInstString << "LWC1";
	AddInstArgs ( strInstString, instruction, FTLWC1 );
}

void Print::SWC1 ( stringstream &strInstString, long instruction )
{
	strInstString << "SWC1";
	AddInstArgs ( strInstString, instruction, FTSWC1 );
}


void Print::CVT_S_W ( stringstream &strInstString, long instruction )
{
	strInstString << "CVT.S.W";
	AddInstArgs ( strInstString, instruction, FTCVTSFMTW );
}

void Print::ADD_S ( stringstream &strInstString, long instruction )
{
	strInstString << "ADD.S";
	AddInstArgs ( strInstString, instruction, FTADDFMTS );
}

void Print::SUB_S ( stringstream &strInstString, long instruction )
{
	strInstString << "SUB.S";
	AddInstArgs ( strInstString, instruction, FTSUBFMTS );
}

void Print::MUL_S ( stringstream &strInstString, long instruction )
{
	strInstString << "MUL.S";
	AddInstArgs ( strInstString, instruction, FTMULFMTS );
}

void Print::MULA_S ( stringstream &strInstString, long instruction )
{
	strInstString << "MUL.S";
	AddInstArgs ( strInstString, instruction, FTMULFMTS );
}


void Print::DIV_S ( stringstream &strInstString, long instruction )
{
	strInstString << "DIV.S";
	AddInstArgs ( strInstString, instruction, FTDIVFMTS );
}

void Print::SQRT_S ( stringstream &strInstString, long instruction )
{
	strInstString << "SQRT.S";
	AddInstArgs ( strInstString, instruction, FTSQRTFMTS );
}

void Print::ABS_S ( stringstream &strInstString, long instruction )
{
	strInstString << "ABS.S";
	AddInstArgs ( strInstString, instruction, FTABSFMTS );
}

void Print::MOV_S ( stringstream &strInstString, long instruction )
{
	strInstString << "MOV.S";
	AddInstArgs ( strInstString, instruction, FTMOVFMTS );
}

void Print::NEG_S ( stringstream &strInstString, long instruction )
{
	strInstString << "NEG.S";
	AddInstArgs ( strInstString, instruction, FTNEGFMTS );
}

void Print::RSQRT_S ( stringstream &strInstString, long instruction )
{
	strInstString << "RSQRT.S";
	AddInstArgs ( strInstString, instruction, FTRSQRTFMTS );
}

void Print::ADDA_S ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDA.S";
	AddInstArgs ( strInstString, instruction, FTADDAFMTS );
}

void Print::SUBA_S ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBA.S";
	AddInstArgs ( strInstString, instruction, FTSUBAFMTS );
}

void Print::MADD_S ( stringstream &strInstString, long instruction )
{
	strInstString << "MADD.S";
	AddInstArgs ( strInstString, instruction, FTMADDFMTS );
}

void Print::MSUB_S ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUB.S";
	AddInstArgs ( strInstString, instruction, FTMSUBFMTS );
}

void Print::MSUBA_S ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUB.S";
	AddInstArgs ( strInstString, instruction, FTMSUBFMTS );
}

void Print::MADDA_S ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDA.S";
	AddInstArgs ( strInstString, instruction, FTMADDAFMTS );
}

void Print::CVT_W_S ( stringstream &strInstString, long instruction )
{
	strInstString << "CVT.W.S";
	AddInstArgs ( strInstString, instruction, FTCVTWFMTS );
}

void Print::MAX_S ( stringstream &strInstString, long instruction )
{
	strInstString << "MAX.S";
	AddInstArgs ( strInstString, instruction, FTMAXFMTS );
}

void Print::MIN_S ( stringstream &strInstString, long instruction )
{
	strInstString << "MIN.S";
	AddInstArgs ( strInstString, instruction, FTMINFMTS );
}

void Print::C_F_S ( stringstream &strInstString, long instruction )
{
	strInstString << "C.F.S";
	AddInstArgs ( strInstString, instruction, FTCFFMTS );
}

void Print::C_EQ_S ( stringstream &strInstString, long instruction )
{
	strInstString << "C.EQ.S";
	AddInstArgs ( strInstString, instruction, FTCEQFMTS );
}

void Print::C_LT_S ( stringstream &strInstString, long instruction )
{
	strInstString << "C.LT.S";
	AddInstArgs ( strInstString, instruction, FTCLTFMTS );
}

void Print::C_LE_S ( stringstream &strInstString, long instruction )
{
	strInstString << "C.LE.S";
	AddInstArgs ( strInstString, instruction, FTCLEFMTS );
}



// * COP2 (VU0) instrutions * //

void Print::CFC2_I ( stringstream &strInstString, long instruction )
{
	strInstString << "CFC2_I";
	AddInstArgs ( strInstString, instruction, FTCFC2 );
}

void Print::CTC2_I ( stringstream &strInstString, long instruction )
{
	strInstString << "CTC2_I";
	AddInstArgs ( strInstString, instruction, FTCTC2 );
}

void Print::CFC2_NI ( stringstream &strInstString, long instruction )
{
	strInstString << "CFC2_NI";
	AddInstArgs ( strInstString, instruction, FTCFC2 );
}

void Print::CTC2_NI ( stringstream &strInstString, long instruction )
{
	strInstString << "CTC2_NI";
	AddInstArgs ( strInstString, instruction, FTCTC2 );
}


// PS2 has LQC2/SQC2 instead of LWC2/SWC2 //
void Print::LQC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "LQC2";
	AddInstArgs ( strInstString, instruction, FORMAT37 /*FTLQC2*/ );
}

void Print::SQC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "SQC2";
	AddInstArgs ( strInstString, instruction, FORMAT37 /*FTSQC2*/ );
}


void Print::QMFC2_NI ( stringstream &strInstString, long instruction )
{
	strInstString << "QMFC2.NI";
	AddInstArgs ( strInstString, instruction, FTQMFC2 );
}

void Print::QMFC2_I ( stringstream &strInstString, long instruction )
{
	strInstString << "QMFC2.I";
	AddInstArgs ( strInstString, instruction, FTQMFC2 );
}

void Print::QMTC2_NI ( stringstream &strInstString, long instruction )
{
	strInstString << "QMTC2.NI";
	AddInstArgs ( strInstString, instruction, FTQMTC2 );
}

void Print::QMTC2_I ( stringstream &strInstString, long instruction )
{
	strInstString << "QMTC2.I";
	AddInstArgs ( strInstString, instruction, FTQMTC2 );
}


void Print::COP2 ( stringstream &strInstString, long instruction )
{
	strInstString << "COP2";
}


void Print::VADDBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDBCX";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::VADDBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDBCY";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::VADDBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDBCZ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::VADDBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDBCW";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::VSUBBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBBCX";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void Print::VSUBBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBBCY";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void Print::VSUBBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBBCZ";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void Print::VSUBBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBBCW";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void Print::VMADDBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDBCX";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void Print::VMADDBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDBCY";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void Print::VMADDBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDBCZ";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void Print::VMADDBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDBCW";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void Print::VMSUBBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBBCX";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void Print::VMSUBBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBBCY";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void Print::VMSUBBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBBCZ";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void Print::VMSUBBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBBCW";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void Print::VMAXBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMAXBCX";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void Print::VMAXBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VMAXBCY";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void Print::VMAXBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMAXBCZ";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void Print::VMAXBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VMAXBCW";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void Print::VMINIBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMINIBCX";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void Print::VMINIBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VMINIBCY";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void Print::VMINIBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMINIBCZ";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void Print::VMINIBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VMINIBCW";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void Print::VMULBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULBCX";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void Print::VMULBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULBCY";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void Print::VMULBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULBCZ";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void Print::VMULBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULBCW";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void Print::VMULq ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULq";
	AddInstArgs ( strInstString, instruction, FTVMULQ );
}

void Print::VMAXi ( stringstream &strInstString, long instruction )
{
	strInstString << "VMAXi";
	AddInstArgs ( strInstString, instruction, FTVMAXI );
}

void Print::VMULi ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULi";
	AddInstArgs ( strInstString, instruction, FTVMULI );
}

void Print::VMINIi ( stringstream &strInstString, long instruction )
{
	strInstString << "VMINIi";
	AddInstArgs ( strInstString, instruction, FTVMINII );
}

void Print::VADDq ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDq";
	AddInstArgs ( strInstString, instruction, FTVADDQ );
}

void Print::VMADDq ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDq";
	AddInstArgs ( strInstString, instruction, FTVMADDQ );
}

void Print::VADDi ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDi";
	AddInstArgs ( strInstString, instruction, FTVADDI );
}

void Print::VMADDi ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDi";
	AddInstArgs ( strInstString, instruction, FTVMADDI );
}

void Print::VSUBq ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBq";
	AddInstArgs ( strInstString, instruction, FTVSUBQ );
}

void Print::VMSUBq ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBq";
	AddInstArgs ( strInstString, instruction, FTVMSUBQ );
}

void Print::VSUBi ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBi";
	AddInstArgs ( strInstString, instruction, FTVSUBI );
}

void Print::VMSUBi ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBi";
	AddInstArgs ( strInstString, instruction, FTVMSUBI );
}

void Print::VADD ( stringstream &strInstString, long instruction )
{
	strInstString << "VADD";
	AddInstArgs ( strInstString, instruction, FTVADD );
}

void Print::VMADD ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADD";
	AddInstArgs ( strInstString, instruction, FTVMADD );
}

void Print::VMUL ( stringstream &strInstString, long instruction )
{
	strInstString << "VMUL";
	AddInstArgs ( strInstString, instruction, FTVMUL );
}

void Print::VMAX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMAX";
	AddInstArgs ( strInstString, instruction, FTVMAX );
}

void Print::VSUB ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUB";
	AddInstArgs ( strInstString, instruction, FTVSUB );
}

void Print::VMSUB ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUB";
	AddInstArgs ( strInstString, instruction, FTVMSUB );
}

void Print::VOPMSUB ( stringstream &strInstString, long instruction )
{
	strInstString << "VOPMSUB";
	AddInstArgs ( strInstString, instruction, FTVOPMSUB );
}

void Print::VMINI ( stringstream &strInstString, long instruction )
{
	strInstString << "VMINI";
	AddInstArgs ( strInstString, instruction, FTVMINI );
}

void Print::VIADD ( stringstream &strInstString, long instruction )
{
	strInstString << "VIADD";
	AddInstArgs ( strInstString, instruction, FTVIADD );
}

void Print::VISUB ( stringstream &strInstString, long instruction )
{
	strInstString << "VISUB";
	AddInstArgs ( strInstString, instruction, FTVISUB );
}

void Print::VIADDI ( stringstream &strInstString, long instruction )
{
	strInstString << "VIADDI";
	AddInstArgs ( strInstString, instruction, FTVIADDI );
}

void Print::VIAND ( stringstream &strInstString, long instruction )
{
	strInstString << "VIAND";
	AddInstArgs ( strInstString, instruction, FTVIAND );
}

void Print::VIOR ( stringstream &strInstString, long instruction )
{
	strInstString << "VIOR";
	AddInstArgs ( strInstString, instruction, FTVIOR );
}

void Print::VCALLMS ( stringstream &strInstString, long instruction )
{
	strInstString << "VCALLMS";
	AddInstArgs ( strInstString, instruction, FTVCALLMS );
}

void Print::VCALLMSR ( stringstream &strInstString, long instruction )
{
	strInstString << "VCALLMSR";
	AddInstArgs ( strInstString, instruction, FTVCALLMSR );
}

void Print::VITOF0 ( stringstream &strInstString, long instruction )
{
	strInstString << "VITOF0";
	AddInstArgs ( strInstString, instruction, FTVITOF0 );
}

void Print::VFTOI0 ( stringstream &strInstString, long instruction )
{
	strInstString << "VFTOI0";
	AddInstArgs ( strInstString, instruction, FTVFTOI0 );
}

void Print::VMULAq ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULAq";
	AddInstArgs ( strInstString, instruction, FTVMULAQ );
}

void Print::VADDAq ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDAq";
	AddInstArgs ( strInstString, instruction, FTVADDAQ );
}

void Print::VSUBAq ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBAq";
	AddInstArgs ( strInstString, instruction, FTVSUBAQ );
}

void Print::VADDA ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDA";
	AddInstArgs ( strInstString, instruction, FTVADDA );
}

void Print::VSUBA ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBA";
	AddInstArgs ( strInstString, instruction, FTVSUBA );
}

void Print::VMOVE ( stringstream &strInstString, long instruction )
{
	strInstString << "VMOVE";
	AddInstArgs ( strInstString, instruction, FTVMOVE );
}

void Print::VLQI ( stringstream &strInstString, long instruction )
{
	strInstString << "VLQI";
	AddInstArgs ( strInstString, instruction, FTVLQI );
}

void Print::VDIV ( stringstream &strInstString, long instruction )
{
	strInstString << "VDIV";
	AddInstArgs ( strInstString, instruction, FTVDIV );
}

void Print::VMTIR ( stringstream &strInstString, long instruction )
{
	strInstString << "VMTIR";
	AddInstArgs ( strInstString, instruction, FTVMTIR );
}

void Print::VRNEXT ( stringstream &strInstString, long instruction )
{
	strInstString << "VRNEXT";
	AddInstArgs ( strInstString, instruction, FTVRNEXT );
}

void Print::VITOF4 ( stringstream &strInstString, long instruction )
{
	strInstString << "VITOF4";
	AddInstArgs ( strInstString, instruction, FTVITOF4 );
}

void Print::VFTOI4 ( stringstream &strInstString, long instruction )
{
	strInstString << "VFTOI4";
	AddInstArgs ( strInstString, instruction, FTVFTOI4 );
}

void Print::VABS ( stringstream &strInstString, long instruction )
{
	strInstString << "VABS";
	AddInstArgs ( strInstString, instruction, FTVABS );
}

void Print::VMADDAq ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDAq";
	AddInstArgs ( strInstString, instruction, FTVMADDAQ );
}

void Print::VMSUBAq ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBAq";
	AddInstArgs ( strInstString, instruction, FTVMSUBAQ );
}

void Print::VMADDA ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDA";
	AddInstArgs ( strInstString, instruction, FTVMADDA );
}

void Print::VMSUBA ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBA";
	AddInstArgs ( strInstString, instruction, FTVMSUBA );
}

void Print::VMR32 ( stringstream &strInstString, long instruction )
{
	strInstString << "VMR32";
	AddInstArgs ( strInstString, instruction, FTVMR32 );
}

void Print::VSQI ( stringstream &strInstString, long instruction )
{
	strInstString << "VSQI";
	AddInstArgs ( strInstString, instruction, FTVSQI );
}

void Print::VSQRT ( stringstream &strInstString, long instruction )
{
	strInstString << "VSQRT";
	AddInstArgs ( strInstString, instruction, FTVSQRT );
}

void Print::VMFIR ( stringstream &strInstString, long instruction )
{
	strInstString << "VMFIR";
	AddInstArgs ( strInstString, instruction, FTVMFIR );
}

void Print::VRGET ( stringstream &strInstString, long instruction )
{
	strInstString << "VRGET";
	AddInstArgs ( strInstString, instruction, FTVRGET );
}

void Print::VADDABCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDABCX";
	AddInstArgs ( strInstString, instruction, FTVADDABC );
}

void Print::VADDABCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDABCY";
	AddInstArgs ( strInstString, instruction, FTVADDABC );
}

void Print::VADDABCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDABCZ";
	AddInstArgs ( strInstString, instruction, FTVADDABC );
}

void Print::VADDABCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDABCW";
	AddInstArgs ( strInstString, instruction, FTVADDABC );
}



void Print::VSUBABCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBABCX";
	AddInstArgs ( strInstString, instruction, FTVSUBABC );
}

void Print::VSUBABCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBABCY";
	AddInstArgs ( strInstString, instruction, FTVSUBABC );
}

void Print::VSUBABCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBABCZ";
	AddInstArgs ( strInstString, instruction, FTVSUBABC );
}

void Print::VSUBABCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBABCW";
	AddInstArgs ( strInstString, instruction, FTVSUBABC );
}



void Print::VMADDABCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDABCX";
	AddInstArgs ( strInstString, instruction, FTVMADDABC );
}

void Print::VMADDABCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDABCY";
	AddInstArgs ( strInstString, instruction, FTVMADDABC );
}

void Print::VMADDABCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDABCZ";
	AddInstArgs ( strInstString, instruction, FTVMADDABC );
}

void Print::VMADDABCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDABCW";
	AddInstArgs ( strInstString, instruction, FTVMADDABC );
}




void Print::VMSUBABCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBABCX";
	AddInstArgs ( strInstString, instruction, FTVMSUBABC );
}

void Print::VMSUBABCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBABCY";
	AddInstArgs ( strInstString, instruction, FTVMSUBABC );
}

void Print::VMSUBABCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBABCZ";
	AddInstArgs ( strInstString, instruction, FTVMSUBABC );
}

void Print::VMSUBABCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBABCW";
	AddInstArgs ( strInstString, instruction, FTVMSUBABC );
}



void Print::VITOF12 ( stringstream &strInstString, long instruction )
{
	strInstString << "VITOF12";
	AddInstArgs ( strInstString, instruction, FTVITOF12 );
}

void Print::VFTOI12 ( stringstream &strInstString, long instruction )
{
	strInstString << "VFTOI12";
	AddInstArgs ( strInstString, instruction, FTVFTOI12 );
}



void Print::VMULABCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULABCX";
	AddInstArgs ( strInstString, instruction, FTVMULABC );
}

void Print::VMULABCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULABCY";
	AddInstArgs ( strInstString, instruction, FTVMULABC );
}

void Print::VMULABCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULABCZ";
	AddInstArgs ( strInstString, instruction, FTVMULABC );
}

void Print::VMULABCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULABCW";
	AddInstArgs ( strInstString, instruction, FTVMULABC );
}



void Print::VMULAi ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULAi";
	AddInstArgs ( strInstString, instruction, FTVMULAI );
}

void Print::VADDAi ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDAi";
	AddInstArgs ( strInstString, instruction, FTVADDAI );
}

void Print::VSUBAi ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBAi";
	AddInstArgs ( strInstString, instruction, FTVSUBAI );
}

void Print::VMULA ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULA";
	AddInstArgs ( strInstString, instruction, FTVMULA );
}

void Print::VOPMULA ( stringstream &strInstString, long instruction )
{
	strInstString << "VOPMULA";
	AddInstArgs ( strInstString, instruction, FTVOPMULA );
}

void Print::VLQD ( stringstream &strInstString, long instruction )
{
	strInstString << "VLQD";
	AddInstArgs ( strInstString, instruction, FTVLQD );
}

void Print::VRSQRT ( stringstream &strInstString, long instruction )
{
	strInstString << "VRSQRT";
	AddInstArgs ( strInstString, instruction, FTVRSQRT );
}

void Print::VILWR ( stringstream &strInstString, long instruction )
{
	strInstString << "VILWR";
	AddInstArgs ( strInstString, instruction, FTVILWR );
}

void Print::VRINIT ( stringstream &strInstString, long instruction )
{
	strInstString << "VRINIT";
	AddInstArgs ( strInstString, instruction, FTVRINIT );
}

void Print::VITOF15 ( stringstream &strInstString, long instruction )
{
	strInstString << "VITOF15";
	AddInstArgs ( strInstString, instruction, FTVITOF15 );
}

void Print::VFTOI15 ( stringstream &strInstString, long instruction )
{
	strInstString << "VFTOI15";
	AddInstArgs ( strInstString, instruction, FTVFTOI15 );
}

void Print::VCLIP ( stringstream &strInstString, long instruction )
{
	strInstString << "VCLIP";
	AddInstArgs ( strInstString, instruction, FTVCLIP );
}

void Print::VMADDAi ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDAi";
	AddInstArgs ( strInstString, instruction, FTVMADDAI );
}

void Print::VMSUBAi ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBAi";
	AddInstArgs ( strInstString, instruction, FTVMSUBAI );
}

void Print::VNOP ( stringstream &strInstString, long instruction )
{
	strInstString << "VNOP";
	AddInstArgs ( strInstString, instruction, FTVNOP );
}

void Print::VSQD ( stringstream &strInstString, long instruction )
{
	strInstString << "VSQD";
	AddInstArgs ( strInstString, instruction, FTVSQD );
}

void Print::VWAITQ ( stringstream &strInstString, long instruction )
{
	strInstString << "VWAITQ";
	AddInstArgs ( strInstString, instruction, FTVWAITQ );
}

void Print::VISWR ( stringstream &strInstString, long instruction )
{
	strInstString << "VISWR";
	AddInstArgs ( strInstString, instruction, FTVISWR );
}

void Print::VRXOR ( stringstream &strInstString, long instruction )
{
	strInstString << "VRXOR";
	AddInstArgs ( strInstString, instruction, FTVRXOR );
}

















void Print::AddVuDestArgs ( stringstream &strVuArgs, long Instruction )
{
	// add the dot
	strVuArgs << ".";
	
	if ( GET_DESTX( Instruction ) ) strVuArgs << "x";
	if ( GET_DESTY( Instruction ) ) strVuArgs << "y";
	if ( GET_DESTZ( Instruction ) ) strVuArgs << "z";
	if ( GET_DESTW( Instruction ) ) strVuArgs << "w";
}


void Print::AddInstArgs ( stringstream &strMipsArgs, long Instruction, long InstFormat )
{

	// don't clear the string since we are adding onto it

	switch ( InstFormat )
	{
		case FORMAT1:
		
			//op rd, rs, rt
			strMipsArgs << dec << " r" << GET_RD( Instruction ) << ", r" << GET_RS( Instruction ) << ", r" << GET_RT( Instruction );
			break;
			
		case FORMAT2:
		
			//op rt, rs, immediate
			strMipsArgs << dec << " r" << GET_RT( Instruction ) << ", r" << GET_RS( Instruction ) << ", " << GET_IMMED( Instruction );
			break;
			
		case FORMAT3:
		
			//op rs, rt, offset
			strMipsArgs << dec << " r" << GET_RS( Instruction ) << ", r" << GET_RT( Instruction ) << ", " << GET_IMMED( Instruction );
			break;
			
		case FORMAT4:
		
			//op rs, offset
			strMipsArgs << dec << " r" << GET_RS( Instruction ) << ", " << GET_IMMED( Instruction );
			break;
			
		case FORMAT5:
		
			//op rs, rt
			strMipsArgs << dec << " r" << GET_RS( Instruction ) << ", r" << GET_RT( Instruction );
			break;
			
		case FORMAT6:
		
			//op rd, rt, sa
			strMipsArgs << dec << " r" << GET_RD( Instruction ) << ", r" << GET_RT( Instruction ) << ", " << GET_SHIFT( Instruction );
			break;
			
		case FORMAT7:
		
			//op rd, rt, rs
			strMipsArgs << dec << " r" << GET_RD( Instruction ) << ", r" << GET_RT( Instruction ) << ", r" << GET_RS( Instruction );
			break;

		case FORMAT8:
		
			//op target
			strMipsArgs << " " << hex << ( GET_ADDRESS( Instruction ) << 2 );
			break;
			
		case FORMAT9:
		
			//op rd, rs
			strMipsArgs << dec << " r" << GET_RD( Instruction ) << ", r" << GET_RS( Instruction );
			break;
			
		case FORMAT10:
		
			//op rs
			strMipsArgs << dec << " r" << GET_RS( Instruction );
			break;
			
		case FORMAT11:
		
			//op rt, offset (base)
			strMipsArgs << dec << " r" << GET_RT( Instruction ) << ", " << ((short)GET_IMMED( Instruction )) << "(r" << GET_BASE( Instruction ) << ")";
			break;
			
		case FORMAT12:
		
			//op rt, immediate
			strMipsArgs << dec << " r" << GET_RT( Instruction ) << ", " << GET_IMMED( Instruction );
			break;
			
		case FORMAT13:
		
			//op rd
			strMipsArgs << dec << " r" << GET_RD( Instruction );
			break;
			
		case FORMAT14:
		
			//op hint, offset (base)
			strMipsArgs << dec << " " << GET_HINT( Instruction ) << ", " << GET_IMMED( Instruction ) << "(r" << GET_BASE( Instruction ) << ")";
			break;
			
		case FORMAT15:
		
			//op rd, rt
			strMipsArgs << dec << " r" << GET_RD( Instruction ) << ", r" << GET_RT( Instruction );
			break;
			
		case FORMAT16:
		
			//op
			break;
			
		case FORMAT17:
		
			//op rt
			strMipsArgs << dec << " r" << GET_RT( Instruction );
			break;
			
		case FORMAT18:
		
			//op rt, reg
			strMipsArgs << dec << " r" << GET_RT( Instruction ) << ", " << GET_REG( Instruction );
			break;
			
		case FORMAT19:

			//op rt, rd
			strMipsArgs << dec << " r" << GET_RT ( Instruction ) << ", r" << GET_RD( Instruction );
			break;
			
		case FORMAT20:
		
			//op fd, fs
			strMipsArgs << " f" << GET_FD( Instruction ) << ", f" << GET_FS( Instruction );
			break;
			
		case FORMAT21:
		
			//op fd, fs, ft
			strMipsArgs << " f" << GET_FD( Instruction ) << ", f" << GET_FS( Instruction ) << ", f" << GET_FT( Instruction );
			break;
			
		case FORMAT22:
		
			//op fs, ft
			strMipsArgs << " f" << GET_FS( Instruction ) << ", f" << GET_FT( Instruction );
			break;
			
		case FORMAT23:
		
			//op offset
			strMipsArgs << " " << GET_IMMED( Instruction );
			break;
			
		case FORMAT24:
		
			//op ft, fs
			strMipsArgs << " f" << GET_FT( Instruction ) << ", f" << GET_FS( Instruction );
			break;
			
		case FORMAT25:
		
			//op fd, ft
			strMipsArgs << " f" << GET_FD( Instruction ) << ", f" << GET_FT( Instruction );
			break;
			
		case FORMAT26:

			//op.dest ft, fs
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FT( Instruction ) << ", vf" << GET_FS( Instruction );
			break;
			
		case FORMAT27:
		
			//op.dest fd, fs, ft
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FD( Instruction ) << ", vf" << GET_FS( Instruction ) << ", vf" << GET_FT( Instruction );
			break;
			
		case FORMAT28:
		
			//op.dest fd, fs
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FD( Instruction ) << ", vf" << GET_FS( Instruction );
			break;
			
		case FORMAT29:

			//op.dest fs, ft
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FS( Instruction ) << ", vf" << GET_FT( Instruction );
			break;
			
		case FORMAT30:
		
			//op.dest fs
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FS( Instruction );
			break;
			
		case FORMAT31:

			//op Imm15
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " " << GET_IMM15( Instruction );
			break;
			
		case FORMAT32:
		
			//op fsfsf, ftftf
			strMipsArgs << " vf" << GET_FS( Instruction ) << "." << XyzwLUT [ GET_FSF( Instruction ) ] << ", vf" << GET_FT( Instruction ) << "." << XyzwLUT [ GET_FTF( Instruction ) ];
			break;
			
		case FORMAT33:
		
			//op id, is, it
			strMipsArgs << " vi" << GET_ID( Instruction ) << ", vi" << GET_IS( Instruction ) << ", vi" << GET_IT( Instruction );
			break;
			
		case FORMAT34:
		
			//op it, is, Imm5
			strMipsArgs << " vi" << GET_IT( Instruction ) << ", vi" << GET_IS( Instruction ) << ", " << GET_IMM5( Instruction );
			break;
			
		case FORMAT35:
		
			//op.dest it, (is)
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vi" << GET_IT( Instruction ) << ", (vi" << GET_IS( Instruction ) << ")";
			break;
			
		case FORMAT36:
		
			//op fsfsf
			strMipsArgs << " vf" << GET_FS( Instruction ) << "." << XyzwLUT [ GET_FSF( Instruction ) ];
			break;
			
		case FORMAT37:
		
			//op ft, offset (base)
			strMipsArgs << " f" << GET_FT( Instruction ) << ", " << GET_IMMED( Instruction ) << "(r" << GET_BASE( Instruction ) << ")";
			break;
			
		case FORMAT38:
		
			//op ftftf
			strMipsArgs << " vf" << GET_FS( Instruction ) << "." << XyzwLUT [ GET_FTF( Instruction ) ];
			break;
			
		case FORMAT39:
		
			//op.dest ft
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FT( Instruction );
			break;
			
		case FORMAT40:
		
			//op.dest fs, (it)
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FS( Instruction ) << ", (vi" << GET_IT( Instruction ) << ")";
			break;
			
		case FORMAT41:
		
			//op it, fsfsf
			strMipsArgs << " vi" << GET_IT( Instruction ) << ", vf" << GET_FS( Instruction ) << "." << XyzwLUT [ GET_FSF( Instruction ) ];
			break;

		case FORMAT42:

			//op.dest ft, is
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " vf" << GET_FT( Instruction ) << ", vi" << GET_IS( Instruction );
			break;

	}

}



const Print::Function Print::FunctionList []
{
	// instructions on both R3000A and R5900
	// 1 + 56 + 6 = 63 instructions //
	Print::Invalid,
	Print::J, Print::JAL, Print::JR, Print::JALR, Print::BEQ, Print::BNE, Print::BGTZ, Print::BGEZ,
	Print::BLTZ, Print::BLEZ, Print::BGEZAL, Print::BLTZAL, Print::ADD, Print::ADDI, Print::ADDU, Print::ADDIU,
	Print::SUB, Print::SUBU, Print::MULT, Print::MULTU, Print::DIV, Print::DIVU, Print::AND, Print::ANDI,
	Print::OR, Print::ORI, Print::XOR, Print::XORI, Print::NOR, Print::LUI, Print::SLL, Print::SRL,
	Print::SRA, Print::SLLV, Print::SRLV, Print::SRAV, Print::SLT, Print::SLTI, Print::SLTU, Print::SLTIU,
	Print::LB, Print::LBU, Print::LH, Print::LHU, Print::LW, Print::LWL, Print::LWR, Print::SB,
	Print::SH, Print::SW, Print::SWL, Print::SWR, Print::MFHI, Print::MTHI, Print::MFLO, Print::MTLO,
	Print::MFC0, Print::MTC0,
	Print::CFC2_I, Print::CTC2_I, Print::CFC2_NI, Print::CTC2_NI,
	Print::SYSCALL, Print::BREAK,
	
	// instructions on R3000A ONLY
	//Print::MFC2, Print::MTC2, Print::LWC2, Print::SWC2, Print::RFE,
	//Print::RTPS, Print::RTPT, Print::CC, Print::CDP, Print::DCPL, Print::DPCS, Print::DPCT, Print::NCS,
	//Print::NCT, Print::NCDS, Print::NCDT, Print::NCCS, Print::NCCT, Print::GPF, Print::GPL, Print::AVSZ3,
	//Print::AVSZ4, Print::SQR, Print::OP, Print::NCLIP, Print::INTPL, Print::MVMVA
	
	// instructions on R5900 ONLY
	// (24*8) + 4 + 6 = 192 + 10 = 202 instructions //
	Print::BEQL, Print::BNEL, Print::BGEZL, Print::BGTZL, Print::BLEZL, Print::BLTZL, Print::BGEZALL, Print::BLTZALL,
	Print::DADD, Print::DADDI, Print::DADDU, Print::DADDIU, Print::DSUB, Print::DSUBU, Print::DSLL, Print::DSLL32,
	Print::DSLLV, Print::DSRA, Print::DSRA32, Print::DSRAV, Print::DSRL, Print::DSRL32, Print::DSRLV, Print::LD,
	Print::LDL, Print::LDR, Print::LWU, Print::LQ, Print::PREF, Print::SD, Print::SDL, Print::SDR,
	Print::SQ, Print::TEQ, Print::TEQI, Print::TNE, Print::TNEI, Print::TGE, Print::TGEI, Print::TGEU,
	Print::TGEIU, Print::TLT, Print::TLTI, Print::TLTU, Print::TLTIU, Print::MOVN, Print::MOVZ, Print::MULT1,
	Print::MULTU1, Print::DIV1, Print::DIVU1, Print::MADD, Print::MADD1, Print::MADDU, Print::MADDU1, Print::MFHI1,
	Print::MTHI1, Print::MFLO1, Print::MTLO1, Print::MFSA, Print::MTSA, Print::MTSAB, Print::MTSAH,
	Print::PABSH, Print::PABSW, Print::PADDB, Print::PADDH, Print::PADDW, Print::PADDSB, Print::PADDSH, Print::PADDSW,
	Print::PADDUB, Print::PADDUH, Print::PADDUW, Print::PADSBH, Print::PAND, Print::POR, Print::PXOR, Print::PNOR,
	Print::PCEQB, Print::PCEQH, Print::PCEQW, Print::PCGTB, Print::PCGTH, Print::PCGTW, Print::PCPYH, Print::PCPYLD,
	Print::PCPYUD, Print::PDIVBW, Print::PDIVUW, Print::PDIVW, Print::PEXCH, Print::PEXCW, Print::PEXEH, Print::PEXEW,
	Print::PEXT5, Print::PEXTLB, Print::PEXTLH, Print::PEXTLW, Print::PEXTUB, Print::PEXTUH, Print::PEXTUW, Print::PHMADH,
	Print::PHMSBH, Print::PINTEH, Print::PINTH, Print::PLZCW, Print::PMADDH, Print::PMADDW, Print::PMADDUW, Print::PMAXH,
	Print::PMAXW, Print::PMINH, Print::PMINW, Print::PMFHI, Print::PMFLO, Print::PMTHI, Print::PMTLO, Print::PMFHL_LH,
	Print::PMFHL_SH, Print::PMFHL_LW, Print::PMFHL_UW, Print::PMFHL_SLW, Print::PMTHL_LW, Print::PMSUBH, Print::PMSUBW, Print::PMULTH,
	Print::PMULTW, Print::PMULTUW, Print::PPAC5, Print::PPACB, Print::PPACH, Print::PPACW, Print::PREVH, Print::PROT3W,
	Print::PSLLH, Print::PSLLVW, Print::PSLLW, Print::PSRAH, Print::PSRAW, Print::PSRAVW, Print::PSRLH, Print::PSRLW,
	Print::PSRLVW, Print::PSUBB, Print::PSUBH, Print::PSUBW, Print::PSUBSB, Print::PSUBSH, Print::PSUBSW, Print::PSUBUB,
	Print::PSUBUH, Print::PSUBUW,
	Print::QFSRV, Print::SYNC,
	
	Print::DI, Print::EI, Print::ERET, Print::CACHE, Print::TLBP, Print::TLBR, Print::TLBWI, Print::TLBWR,
	Print::CFC0, Print::CTC0,
	
	Print::BC0T, Print::BC0TL, Print::BC0F, Print::BC0FL, Print::BC1T, Print::BC1TL, Print::BC1F, Print::BC1FL,
	Print::BC2T, Print::BC2TL, Print::BC2F, Print::BC2FL,
	
	Print::LWC1, Print::SWC1, Print::MFC1, Print::MTC1, Print::CFC1, Print::CTC1,
	Print::ABS_S, Print::ADD_S, Print::ADDA_S, Print::C_EQ_S, Print::C_F_S, Print::C_LE_S, Print::C_LT_S, Print::CVT_S_W,
	Print::CVT_W_S, Print::DIV_S, Print::MADD_S, Print::MADDA_S, Print::MAX_S, Print::MIN_S, Print::MOV_S, Print::MSUB_S,
	Print::MSUBA_S, Print::MUL_S, Print::MULA_S, Print::NEG_S, Print::RSQRT_S, Print::SQRT_S, Print::SUB_S, Print::SUBA_S,
	
	// VU macro mode instructions
	Print::QMFC2_NI, Print::QMFC2_I, Print::QMTC2_NI, Print::QMTC2_I, Print::LQC2, Print::SQC2,
	
	Print::VABS,
	Print::VADD, Print::VADDi, Print::VADDq, Print::VADDBCX, Print::VADDBCY, Print::VADDBCZ, Print::VADDBCW,
	Print::VADDA, Print::VADDAi, Print::VADDAq, Print::VADDABCX, Print::VADDABCY, Print::VADDABCZ, Print::VADDABCW,
	Print::VCALLMS, Print::VCALLMSR, Print::VCLIP, Print::VDIV,
	Print::VFTOI0, Print::VFTOI4, Print::VFTOI12, Print::VFTOI15,
	Print::VIADD, Print::VIADDI, Print::VIAND, Print::VILWR, Print::VIOR, Print::VISUB, Print::VISWR,
	Print::VITOF0, Print::VITOF4, Print::VITOF12, Print::VITOF15,
	Print::VLQD, Print::VLQI,
	
	Print::VMADD, Print::VMADDi, Print::VMADDq, Print::VMADDBCX, Print::VMADDBCY, Print::VMADDBCZ, Print::VMADDBCW,
	Print::VMADDA, Print::VMADDAi, Print::VMADDAq, Print::VMADDABCX, Print::VMADDABCY, Print::VMADDABCZ, Print::VMADDABCW,
	Print::VMAX, Print::VMAXi, Print::VMAXBCX, Print::VMAXBCY, Print::VMAXBCZ, Print::VMAXBCW,
	Print::VMFIR,
	Print::VMINI, Print::VMINIi, Print::VMINIBCX, Print::VMINIBCY, Print::VMINIBCZ, Print::VMINIBCW,
	Print::VMOVE, Print::VMR32,
	
	Print::VMSUB, Print::VMSUBi, Print::VMSUBq, Print::VMSUBBCX, Print::VMSUBBCY, Print::VMSUBBCZ, Print::VMSUBBCW,
	Print::VMSUBA, Print::VMSUBAi, Print::VMSUBAq, Print::VMSUBABCX, Print::VMSUBABCY, Print::VMSUBABCZ, Print::VMSUBABCW,
	Print::VMTIR,
	Print::VMUL, Print::VMULi, Print::VMULq, Print::VMULBCX, Print::VMULBCY, Print::VMULBCZ, Print::VMULBCW,
	Print::VMULA, Print::VMULAi, Print::VMULAq, Print::VMULABCX, Print::VMULABCY, Print::VMULABCZ, Print::VMULABCW,
	Print::VNOP, Print::VOPMSUB, Print::VOPMULA, Print::VRGET, Print::VRINIT, Print::VRNEXT, Print::VRSQRT, Print::VRXOR,
	Print::VSQD, Print::VSQI, Print::VSQRT,
	Print::VSUB, Print::VSUBi, Print::VSUBq, Print::VSUBBCX, Print::VSUBBCY, Print::VSUBBCZ, Print::VSUBBCW,
	Print::VSUBA, Print::VSUBAi, Print::VSUBAq, Print::VSUBABCX, Print::VSUBABCY, Print::VSUBABCZ, Print::VSUBABCW,
	Print::VWAITQ,
	Print::COP2
};





