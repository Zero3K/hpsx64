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


#include "R5900_Encoder.h"
#include "MipsOpcode.h"

using namespace R5900;


// *** PS1 INSTRUTIONS *** //


unsigned long Encoder::ADDIU ( long rt, long rs, short Immediate )
{
	return SET_OPCODE ( OPADDIU ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_IMMED ( Immediate );
}

unsigned long Encoder::ANDI ( long rt, long rs, short Immediate )
{
	return SET_OPCODE ( OPANDI ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_IMMED ( Immediate );
}

unsigned long Encoder::ORI ( long rt, long rs, short Immediate )
{
	return SET_OPCODE ( OPORI ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_IMMED ( Immediate );
}

unsigned long Encoder::SLTI ( long rt, long rs, short Immediate )
{
	return SET_OPCODE ( OPSLTI ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_IMMED ( Immediate );
}

unsigned long Encoder::SLTIU ( long rt, long rs, short Immediate )
{
	return SET_OPCODE ( OPSLTIU ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_IMMED ( Immediate );
}

unsigned long Encoder::XORI ( long rt, long rs, short Immediate )
{
	return SET_OPCODE ( OPXORI ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_IMMED ( Immediate );
}


unsigned long Encoder::ADDI ( long rt, long rs, short Immediate )
{
	return SET_OPCODE ( OPADDI ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_IMMED ( Immediate );
}



unsigned long Encoder::ADDU ( long rd, long rs, long rt )
{
	return SET_OPCODE ( OPADDU ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPADDU );
}

unsigned long Encoder::AND ( long rd, long rs, long rt )
{
	return SET_OPCODE ( OPAND ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPAND );
}

unsigned long Encoder::OR ( long rd, long rs, long rt )
{
	return SET_OPCODE ( OPOR ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPOR );
}

unsigned long Encoder::NOR ( long rd, long rs, long rt )
{
	return SET_OPCODE ( OPNOR ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPNOR );
}

unsigned long Encoder::SLT ( long rd, long rs, long rt )
{
	return SET_OPCODE ( OPSLT ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPSLT );
}

unsigned long Encoder::SLTU ( long rd, long rs, long rt )
{
	return SET_OPCODE ( OPSLTU ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPSLTU );
}

unsigned long Encoder::SUBU ( long rd, long rs, long rt )
{
	return SET_OPCODE ( OPSUBU ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPSUBU );
}

unsigned long Encoder::XOR ( long rd, long rs, long rt )
{
	return SET_OPCODE ( OPXOR ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPXOR );
}


unsigned long Encoder::ADD ( long rd, long rs, long rt )
{
	return SET_OPCODE ( OPADD ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPADD );
}

unsigned long Encoder::SUB ( long rd, long rs, long rt )
{
	return SET_OPCODE ( OPSUB ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPSUB );
}


unsigned long Encoder::DIV ( long rs, long rt )
{
	return SET_OPCODE ( OPDIV ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPDIV );
}

unsigned long Encoder::DIVU ( long rs, long rt )
{
	return SET_OPCODE ( OPDIVU ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPDIVU );
}

unsigned long Encoder::MULT ( long rs, long rt )
{
	return SET_OPCODE ( OPMULT ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPMULT );
}

unsigned long Encoder::MULTU ( long rs, long rt )
{
	return SET_OPCODE ( OPMULTU ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPMULTU );
}


unsigned long Encoder::SLL ( long rd, long rt, long sa )
{
	return SET_OPCODE ( OPSLL ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_SHIFT( sa ) | SET_SPECIAL( SPSLL );
}

unsigned long Encoder::SRA ( long rd, long rt, long sa )
{
	return SET_OPCODE ( OPSRA ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_SHIFT( sa ) | SET_SPECIAL( SPSRA );
}

unsigned long Encoder::SRL ( long rd, long rt, long sa )
{
	return SET_OPCODE ( OPSRL ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_SHIFT( sa ) | SET_SPECIAL( SPSRL );
}

unsigned long Encoder::SLLV ( long rd, long rt, long rs )
{
	return SET_OPCODE ( OPSLLV ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_RS( rs ) | SET_SPECIAL( SPSLLV );
}

unsigned long Encoder::SRAV ( long rd, long rt, long rs )
{
	return SET_OPCODE ( OPSRAV ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_RS( rs ) | SET_SPECIAL( SPSRAV );
}

unsigned long Encoder::SRLV ( long rd, long rt, long rs )
{
	return SET_OPCODE ( OPSRAV ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_RS( rs ) | SET_SPECIAL( SPSRAV );
}


unsigned long Encoder::J ( long target )
{
	return SET_OPCODE ( OPJ ) | SET_ADDRESS( target );
}

unsigned long Encoder::JR ( long rs )
{
	return SET_OPCODE ( OPJR ) | SET_RS( rs ) | SET_SPECIAL( SPJR );
}

unsigned long Encoder::JAL ( long target )
{
	return SET_OPCODE ( OPJAL ) | SET_ADDRESS( target );
}

unsigned long Encoder::JALR ( long rd, long rs )
{
	return SET_OPCODE ( OPJALR ) | SET_RD( rd ) | SET_RS( rs ) | SET_SPECIAL( SPJALR );
}


unsigned long Encoder::BEQ ( long rs, long rt, short offset )
{
	return SET_OPCODE ( OPBEQ ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_IMMED ( offset );
}

unsigned long Encoder::BNE ( long rs, long rt, short offset )
{
	return SET_OPCODE ( OPBNE ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_IMMED ( offset );
}

unsigned long Encoder::BGEZ ( long rs, short offset )
{
	return SET_OPCODE ( OPBGEZ ) | SET_RS ( rs ) | SET_RT ( RTBGEZ ) | SET_IMMED ( offset );
}

unsigned long Encoder::BGTZ ( long rs, short offset )
{
	return SET_OPCODE ( OPBGTZ ) | SET_RS ( rs ) | SET_IMMED ( offset );
}

unsigned long Encoder::BLEZ ( long rs, short offset )
{
	return SET_OPCODE ( OPBLEZ ) | SET_RS ( rs ) | SET_IMMED ( offset );
}

unsigned long Encoder::BLTZ ( long rs, short offset )
{
	return SET_OPCODE ( OPBLTZ ) | SET_RS ( rs ) | SET_RT ( RTBLTZ ) | SET_IMMED ( offset );
}

unsigned long Encoder::BGEZAL ( long rs, short offset )
{
	return SET_OPCODE ( OPBGEZAL ) | SET_RS ( rs ) | SET_RT ( RTBGEZAL ) | SET_IMMED ( offset );
}

unsigned long Encoder::BLTZAL ( long rs, short offset )
{
	return SET_OPCODE ( OPBLTZAL ) | SET_RS ( rs ) | SET_RT ( RTBLTZAL ) | SET_IMMED ( offset );
}


unsigned long Encoder::LUI ( long rt, short immediate )
{
	return SET_OPCODE ( OPLUI ) | SET_RT ( rt ) | SET_IMMED ( immediate );
}



unsigned long Encoder::MFHI ( long rd )
{
	return SET_OPCODE ( OPMFHI ) | SET_RD ( rd ) | SET_SPECIAL( SPMFHI );
}

unsigned long Encoder::MFLO ( long rd )
{
	return SET_OPCODE ( OPMFLO ) | SET_RD ( rd ) | SET_SPECIAL( SPMFLO );
}

unsigned long Encoder::MTHI ( long rs )
{
	return SET_OPCODE ( OPMTHI ) | SET_RS ( rs ) | SET_SPECIAL( SPMTHI );
}

unsigned long Encoder::MTLO ( long rs )
{
	return SET_OPCODE ( OPMTLO ) | SET_RS ( rs ) | SET_SPECIAL( SPMTLO );
}


// to add
unsigned long Encoder::SYSCALL ()
{
	return SET_OPCODE ( OPSYSCALL ) | SET_SPECIAL( SPSYSCALL );
}

//unsigned long Encoder::RFE ()
//{
//	return SET_OPCODE ( OPRFE ) | SET_RS ( RSRFE ) | SET_SPECIAL( SPRFE );
//}

unsigned long Encoder::BREAK ()
{
	return SET_OPCODE ( OPBREAK ) | SET_SPECIAL( SPBREAK );
}

// to add
unsigned long Encoder::MFC0 ( long rt, long rd )
{
	return SET_OPCODE ( OPMFC0 ) | SET_RS ( RSMFC0 ) | SET_RT( rt ) | SET_RD ( rd );
}

unsigned long Encoder::MTC0 ( long rt, long rd )
{
	return SET_OPCODE ( OPMTC0 ) | SET_RS ( RSMTC0 ) | SET_RT( rt ) | SET_RD ( rd );
}

//unsigned long Encoder::MFC2 ( long rt, long rd )
//{
//	return SET_OPCODE ( OPMFC2 ) | SET_RS ( RSMFC2 ) | SET_RT( rt ) | SET_RD ( rd );
//}

//unsigned long Encoder::MTC2 ( long rt, long rd )
//{
//	return SET_OPCODE ( OPMTC2 ) | SET_RS ( RSMTC2 ) | SET_RT( rt ) | SET_RD ( rd );
//}

unsigned long Encoder::CFC2 ( long rt, long rd )
{
	return SET_OPCODE ( OPCFC2 ) | SET_RS ( RSCFC2 ) | SET_RT( rt ) | SET_RD ( rd );
}

unsigned long Encoder::CTC2 ( long rt, long rd )
{
	return SET_OPCODE ( OPCTC2 ) | SET_RS ( RSCTC2 ) | SET_RT( rt ) | SET_RD ( rd );
}

//unsigned long Encoder::COP2 ( long Command )
//{
//	return SET_OPCODE ( OPCOP2 ) | SET_RS ( RSCOP2 ) | ( Command & 0x1fffff );
//}

// to add
unsigned long Encoder::LB ( long rt, short offset, long base )
{
	return SET_OPCODE ( OPLB ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
}

unsigned long Encoder::LH ( long rt, short offset, long base )
{
	return SET_OPCODE ( OPLH ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
}

unsigned long Encoder::LWL ( long rt, short offset, long base )
{
	return SET_OPCODE ( OPLWL ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
}

unsigned long Encoder::LW ( long rt, short offset, long base )
{
	return SET_OPCODE ( OPLW ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
}

unsigned long Encoder::LBU ( long rt, short offset, long base )
{
	return SET_OPCODE ( OPLBU ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
}

unsigned long Encoder::LHU ( long rt, short offset, long base )
{
	return SET_OPCODE ( OPLHU ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
}

unsigned long Encoder::LWR ( long rt, short offset, long base )
{
	return SET_OPCODE ( OPLWR ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
}

unsigned long Encoder::SB ( long rt, short offset, long base )
{
	return SET_OPCODE ( OPSB ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
}

unsigned long Encoder::SH ( long rt, short offset, long base )
{
	return SET_OPCODE ( OPSH ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
}

unsigned long Encoder::SWL ( long rt, short offset, long base )
{
	return SET_OPCODE ( OPSWL ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
}

unsigned long Encoder::SW ( long rt, short offset, long base )
{
	return SET_OPCODE ( OPSW ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
}

unsigned long Encoder::SWR ( long rt, short offset, long base )
{
	return SET_OPCODE ( OPSWR ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
}




//unsigned long Encoder::LWC2 ( long rt, short offset, long base )
//{
//	return SET_OPCODE ( OPLWC2 ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
//}

//unsigned long Encoder::SWC2 ( long rt, short offset, long base )
//{
//	return SET_OPCODE ( OPSWC2 ) | SET_BASE ( base ) | SET_RT( rt ) | SET_IMMED ( offset );
//}


// *** PS2 INSTRUTIONS *** //


unsigned long Encoder::BC0T ( short offset ){ return SET_OPCODE ( OPBC0T ) | SET_RS ( RSBC0T ) | SET_RT( RTBC0T ) | SET_IMMED ( offset ); }
unsigned long Encoder::BC0TL ( short offset ){ return SET_OPCODE ( OPBC0TL ) | SET_RS ( RSBC0TL ) | SET_RT( RTBC0TL ) | SET_IMMED ( offset );}
unsigned long Encoder::BC0F ( short offset ){ return SET_OPCODE ( OPBC0F ) | SET_RS ( RSBC0F ) | SET_RT( RTBC0F ) | SET_IMMED ( offset );}
unsigned long Encoder::BC0FL ( short offset ){ return SET_OPCODE ( OPBC0FL ) | SET_RS ( RSBC0FL ) | SET_RT( RTBC0FL ) | SET_IMMED ( offset );}
unsigned long Encoder::BC1T ( short offset ){ return SET_OPCODE ( OPBC1T ) | SET_RS ( RSBC1T ) | SET_RT( RTBC1T ) | SET_IMMED ( offset );}
unsigned long Encoder::BC1TL ( short offset ){ return SET_OPCODE ( OPBC1TL ) | SET_RS ( RSBC1TL ) | SET_RT( RTBC1TL ) | SET_IMMED ( offset );}
unsigned long Encoder::BC1F ( short offset ){ return SET_OPCODE ( OPBC1F ) | SET_RS ( RSBC1F ) | SET_RT( RTBC1F ) | SET_IMMED ( offset );}
unsigned long Encoder::BC1FL ( short offset ){ return SET_OPCODE ( OPBC1FL ) | SET_RS ( RSBC1FL ) | SET_RT( RTBC1FL ) | SET_IMMED ( offset );}
unsigned long Encoder::BC2T ( short offset ){ return SET_OPCODE ( OPBC2T ) | SET_RS ( RSBC2T ) | SET_RT( RTBC2T ) | SET_IMMED ( offset );}
unsigned long Encoder::BC2TL ( short offset ){ return SET_OPCODE ( OPBC2TL ) | SET_RS ( RSBC2TL ) | SET_RT( RTBC2TL ) | SET_IMMED ( offset );}
unsigned long Encoder::BC2F ( short offset ){ return SET_OPCODE ( OPBC2F ) | SET_RS ( RSBC2F ) | SET_RT( RTBC2F ) | SET_IMMED ( offset );}
unsigned long Encoder::BC2FL ( short offset ){ return SET_OPCODE ( OPBC2FL ) | SET_RS ( RSBC2FL ) | SET_RT( RTBC2FL ) | SET_IMMED ( offset );}


unsigned long Encoder::CFC0 ( long rt, long rd ){ return SET_OPCODE ( OPCFC0 ) | SET_RS ( RSCFC0 ) | SET_RT ( rt ) | SET_RD ( rd ); }
unsigned long Encoder::CTC0 ( long rt, long rd ){ return SET_OPCODE ( OPCFC0 ) | SET_RS ( RSCFC0 ) | SET_RT ( rt ) | SET_RD ( rd ); }
unsigned long Encoder::EI (){ return SET_OPCODE ( OPEI ) | SET_RS ( RSCO ) | SET_SPECIAL ( SPEI ); }
unsigned long Encoder::DI (){ return SET_OPCODE ( OPDI ) | SET_RS ( RSCO ) | SET_SPECIAL ( SPDI ); }
	
unsigned long Encoder::SD ( long rt, short offset, long base ){ return SET_OPCODE ( OPSD ) | SET_BASE ( base ) | SET_RT ( rt ) | SET_IMMED ( offset ); }
unsigned long Encoder::LD ( long rt, short offset, long base ){ return SET_OPCODE ( OPLD ) | SET_BASE ( base ) | SET_RT ( rt ) | SET_IMMED ( offset ); }
unsigned long Encoder::LWU ( long rt, short offset, long base ){ return SET_OPCODE ( OPLWU ) | SET_BASE ( base ) | SET_RT ( rt ) | SET_IMMED ( offset ); }
unsigned long Encoder::SDL ( long rt, short offset, long base ){ return SET_OPCODE ( OPSDL ) | SET_BASE ( base ) | SET_RT ( rt ) | SET_IMMED ( offset ); }
unsigned long Encoder::SDR ( long rt, short offset, long base ){ return SET_OPCODE ( OPSDR ) | SET_BASE ( base ) | SET_RT ( rt ) | SET_IMMED ( offset ); }
unsigned long Encoder::LDL ( long rt, short offset, long base ){ return SET_OPCODE ( OPLDL ) | SET_BASE ( base ) | SET_RT ( rt ) | SET_IMMED ( offset ); }
unsigned long Encoder::LDR ( long rt, short offset, long base ){ return SET_OPCODE ( OPLDR ) | SET_BASE ( base ) | SET_RT ( rt ) | SET_IMMED ( offset ); }
unsigned long Encoder::LQ ( long rt, short offset, long base ){ return SET_OPCODE ( OPLQ ) | SET_BASE ( base ) | SET_RT ( rt ) | SET_IMMED ( offset ); }
unsigned long Encoder::SQ ( long rt, short offset, long base ){ return SET_OPCODE ( OPSQ ) | SET_BASE ( base ) | SET_RT ( rt ) | SET_IMMED ( offset ); }
	
	
// arithemetic instructions //
unsigned long Encoder::DADD ( long rd, long rs, long rt ){ return SET_OPCODE ( OPDADD ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPDADD ); }
unsigned long Encoder::DADDU ( long rd, long rs, long rt ){ return SET_OPCODE ( OPDADDU ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPDADDU ); }
unsigned long Encoder::DSUB ( long rd, long rs, long rt ){ return SET_OPCODE ( OPDSUB ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPDSUB ); }
unsigned long Encoder::DSUBU ( long rd, long rs, long rt ){ return SET_OPCODE ( OPDSUBU ) | SET_RD ( rd ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_SPECIAL( SPDSUBU ); }

unsigned long Encoder::DADDI ( long rt, long rs, short Immediate ){ return SET_OPCODE ( OPDADDI ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_IMMED ( Immediate ); }
unsigned long Encoder::DADDIU ( long rt, long rs, short Immediate ){ return SET_OPCODE ( OPDADDIU ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_IMMED ( Immediate ); }

unsigned long Encoder::DSLL ( long rd, long rt, long sa ){ return SET_OPCODE ( OPDSLL ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_SHIFT( sa ) | SET_SPECIAL( SPDSLL ); }
unsigned long Encoder::DSLL32 ( long rd, long rt, long sa ){ return SET_OPCODE ( OPDSLL32 ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_SHIFT( sa ) | SET_SPECIAL( SPDSLL32 ); }
unsigned long Encoder::DSRA ( long rd, long rt, long sa ){ return SET_OPCODE ( OPDSRA ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_SHIFT( sa ) | SET_SPECIAL( SPDSRA ); }
unsigned long Encoder::DSRA32 ( long rd, long rt, long sa ){ return SET_OPCODE ( OPDSRA32 ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_SHIFT( sa ) | SET_SPECIAL( SPDSRA32 ); }
unsigned long Encoder::DSRL ( long rd, long rt, long sa ){ return SET_OPCODE ( OPDSRL ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_SHIFT( sa ) | SET_SPECIAL( SPDSRL ); }
unsigned long Encoder::DSRL32 ( long rd, long rt, long sa ){ return SET_OPCODE ( OPDSRL32 ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_SHIFT( sa ) | SET_SPECIAL( SPDSRL32 ); }

unsigned long Encoder::DSLLV ( long rd, long rt, long rs ){ return SET_OPCODE ( OPDSLLV ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_RS( rs ) | SET_SPECIAL( SPDSLLV ); }
unsigned long Encoder::DSRAV ( long rd, long rt, long rs ){ return SET_OPCODE ( OPDSRAV ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_RS( rs ) | SET_SPECIAL( SPDSRAV ); }
unsigned long Encoder::DSRLV ( long rd, long rt, long rs ){ return SET_OPCODE ( OPDSRLV ) | SET_RD ( rd ) | SET_RT ( rt ) | SET_RS( rs ) | SET_SPECIAL( SPDSRLV ); }
	
	

unsigned long Encoder::MFC1 ( long rt, long fs ){ return SET_OPCODE ( OPMFC1 ) | SET_RS ( RSMFC1 ) | SET_RT( rt ) | SET_RD ( fs ); }
unsigned long Encoder::CFC1 ( long rt, long fs ){ return SET_OPCODE ( OPCFC1 ) | SET_RS ( RSCFC1 ) | SET_RT( rt ) | SET_RD ( fs ); }
unsigned long Encoder::MTC1 ( long rt, long fs ){ return SET_OPCODE ( OPMTC1 ) | SET_RS ( RSMTC1 ) | SET_RT( rt ) | SET_RD ( fs ); }
unsigned long Encoder::CTC1 ( long rt, long fs ){ return SET_OPCODE ( OPCTC1 ) | SET_RS ( RSCTC1 ) | SET_RT( rt ) | SET_RD ( fs ); }

unsigned long Encoder::BEQL ( long rs, long rt, short offset ){ return SET_OPCODE ( OPBEQL ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_IMMED ( offset ); }
unsigned long Encoder::BNEL ( long rs, long rt, short offset ){ return SET_OPCODE ( OPBNEL ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_IMMED ( offset ); }

unsigned long Encoder::BGEZL ( long rs, short offset ){ return SET_OPCODE ( OPBGEZL ) | SET_RS ( rs ) | SET_RT ( RTBGEZL ) | SET_IMMED ( offset ); }
unsigned long Encoder::BLEZL ( long rs, short offset ){ return SET_OPCODE ( OPBLEZL ) | SET_RS ( rs ) | SET_RT ( RTBLEZL ) | SET_IMMED ( offset ); }
unsigned long Encoder::BGTZL ( long rs, short offset ){ return SET_OPCODE ( OPBGTZL ) | SET_RS ( rs ) | SET_RT ( RTBGTZL ) | SET_IMMED ( offset ); }
unsigned long Encoder::BLTZL ( long rs, short offset ){ return SET_OPCODE ( OPBLTZL ) | SET_RS ( rs ) | SET_RT ( RTBLTZL ) | SET_IMMED ( offset ); }
unsigned long Encoder::BLTZALL ( long rs, short offset ){ return SET_OPCODE ( OPBLTZALL ) | SET_RS ( rs ) | SET_RT ( RTBLTZALL ) | SET_IMMED ( offset ); }
unsigned long Encoder::BGEZALL ( long rs, short offset ){ return SET_OPCODE ( OPBGEZALL ) | SET_RS ( rs ) | SET_RT ( RTBGEZALL ) | SET_IMMED ( offset ); }

unsigned long Encoder::MOVZ ( long rd, long rs, long rt ){ return SET_OPCODE ( OPMOVZ ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_RD ( rd ) | SET_SPECIAL ( OPMOVZ ); }
unsigned long Encoder::MOVN ( long rd, long rs, long rt ){ return SET_OPCODE ( OPMOVN ) | SET_RS ( rs ) | SET_RT ( rt ) | SET_RD ( rd ) | SET_SPECIAL ( OPMOVN ); }

unsigned long Encoder::TGE ( long rs, long rt ){}
unsigned long Encoder::TGEU ( long rs, long rt ){}
unsigned long Encoder::TLT ( long rs, long rt ){}
unsigned long Encoder::TLTU ( long rs, long rt ){}
unsigned long Encoder::TEQ ( long rs, long rt ){}
unsigned long Encoder::TNE ( long rs, long rt ){}

unsigned long Encoder::TGEI ( long rs, short immediate ){}
unsigned long Encoder::TGEIU ( long rs, short immediate ){}
unsigned long Encoder::TLTI ( long rs, short immediate ){}
unsigned long Encoder::TLTIU ( long rs, short immediate ){}
unsigned long Encoder::TEQI ( long rs, short immediate ){}
unsigned long Encoder::TNEI ( long rs, short immediate ){}


// ??
unsigned long Encoder::MOVCI (){}
unsigned long Encoder::SYNC (){}

unsigned long Encoder::MFHI1 ( long rd ){}
unsigned long Encoder::MFLO1 ( long rd ){}
unsigned long Encoder::MTHI1 ( long rs ){}
unsigned long Encoder::MTLO1 ( long rs ){}
unsigned long Encoder::MULT1 ( long rd, long rs, long rt ){}
unsigned long Encoder::MULTU1 ( long rd, long rs, long rt ){}
unsigned long Encoder::DIV1 ( long rs, long rt ){}
unsigned long Encoder::DIVU1 ( long rs, long rt ){}
unsigned long Encoder::MADD ( long rd, long rs, long rt ){}
unsigned long Encoder::MADD1 ( long rd, long rs, long rt ){}
unsigned long Encoder::MADDU ( long rd, long rs, long rt ){}
unsigned long Encoder::MADDU1 ( long rd, long rs, long rt ){}

unsigned long Encoder::MFSA ( long rd ){}
unsigned long Encoder::MTSA ( long rs ){}
unsigned long Encoder::MTSAB ( long rs, short immediate ){}
unsigned long Encoder::MTSAH ( long rs, short immediate ){}

unsigned long Encoder::TLBR (){}
unsigned long Encoder::TLBWI (){}
unsigned long Encoder::TLBWR (){}
unsigned long Encoder::TLBP (){}

unsigned long Encoder::ERET (){}

unsigned long Encoder::CACHE ( long op, short offset, long base ){}
unsigned long Encoder::PREF ( long hint, short offset, long base ){}




