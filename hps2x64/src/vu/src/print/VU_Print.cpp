

#include "VU_Print.h"


using namespace std;
using namespace Vu::Instruction;



const char Print::XyzwLUT [ 4 ] = { 'x', 'y', 'z', 'w' };
const char* Print::BCType [ 4 ] = { "F", "T", "FL", "TL" };



string Print::PrintInstructionLO ( long instruction )
{
	stringstream ss;
	ss.str("");
	
	FunctionList [ Lookup::FindByInstructionLO ( instruction ) ] ( ss, instruction );
	
	return ss.str().c_str ();
}


string Print::PrintInstructionHI ( long instruction )
{
	stringstream ss;
	ss.str("");
	
	FunctionList [ Lookup::FindByInstructionHI ( instruction ) ] ( ss, instruction );
	
	return ss.str().c_str ();
}



void Print::Start ()
{
	// make sure the lookup object has started (note: this can take a LONG time for R5900 currently)
	Lookup::Start ();
}



void Print::INVALID ( stringstream &strInstString, long instruction )
{
	strInstString << "Invalid Instruction: Opcode: " << GET_OPCODE( instruction ) << ", Special: " << GET_SPECIAL( instruction ) << ", Shift: " << GET_SHIFT( instruction );	// << ", RT: " << GET_RT( instruction );
}



//// *** UPPER instructions *** ////


// ABS //

void Print::ABS ( stringstream &strInstString, long instruction )
{
	strInstString << "ABS";
	AddInstArgs ( strInstString, instruction, FTVABS );
}


// ADD //

void Print::ADD ( stringstream &strInstString, long instruction )
{
	strInstString << "ADD";
	AddInstArgs ( strInstString, instruction, FTVADD );
}

void Print::ADDi ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDI";
	AddInstArgs ( strInstString, instruction, FTVADDI );
}

void Print::ADDq ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDQ";
	AddInstArgs ( strInstString, instruction, FTVADDQ );
}

void Print::ADDBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDBCX";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::ADDBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDBCY";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::ADDBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDBCZ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::ADDBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDBCW";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}




// SUB //

void Print::SUB ( stringstream &strInstString, long instruction )
{
	strInstString << "SUB";
	AddInstArgs ( strInstString, instruction, FTVSUB );
}

void Print::SUBi ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBI";
	AddInstArgs ( strInstString, instruction, FTVSUBI );
}

void Print::SUBq ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBQ";
	AddInstArgs ( strInstString, instruction, FTVSUBQ );
}

void Print::SUBBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBBCX";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void Print::SUBBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBBCY";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void Print::SUBBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBBCZ";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void Print::SUBBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBBCW";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}




// MADD //

void Print::MADD ( stringstream &strInstString, long instruction )
{
	strInstString << "MADD";
	AddInstArgs ( strInstString, instruction, FTVMADD );
}

void Print::MADDi ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDI";
	AddInstArgs ( strInstString, instruction, FTVMADDI );
}

void Print::MADDq ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDQ";
	AddInstArgs ( strInstString, instruction, FTVMADDQ );
}

void Print::MADDBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDBCX";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void Print::MADDBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDBCY";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void Print::MADDBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDBCZ";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void Print::MADDBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDBCW";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}



// MSUB //

void Print::MSUB ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUB";
	AddInstArgs ( strInstString, instruction, FTVMSUB );
}

void Print::MSUBi ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBI";
	AddInstArgs ( strInstString, instruction, FTVMSUBI );
}

void Print::MSUBq ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBQ";
	AddInstArgs ( strInstString, instruction, FTVMSUBQ );
}

void Print::MSUBBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBBCX";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void Print::MSUBBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBBCY";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void Print::MSUBBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBBCZ";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void Print::MSUBBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBBCW";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}



// MAX //

void Print::MAX ( stringstream &strInstString, long instruction )
{
	strInstString << "MAX";
	AddInstArgs ( strInstString, instruction, FTVMAX );
}

void Print::MAXi ( stringstream &strInstString, long instruction )
{
	strInstString << "MAXI";
	AddInstArgs ( strInstString, instruction, FTVMAXI );
}

void Print::MAXBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "MAXBCX";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void Print::MAXBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "MAXBCY";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void Print::MAXBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MAXBCZ";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void Print::MAXBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "MAXBCW";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}



// MINI //

void Print::MINI ( stringstream &strInstString, long instruction )
{
	strInstString << "MINI";
	AddInstArgs ( strInstString, instruction, FTVMINI );
}

void Print::MINIi ( stringstream &strInstString, long instruction )
{
	strInstString << "MINII";
	AddInstArgs ( strInstString, instruction, FTVMINII );
}

void Print::MINIBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "MINIBCX";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void Print::MINIBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "MINIBCY";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void Print::MINIBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MINIBCZ";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void Print::MINIBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "MINIBCW";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}




// MUL //

void Print::MUL ( stringstream &strInstString, long instruction )
{
	strInstString << "MUL";
	AddInstArgs ( strInstString, instruction, FTVMUL );
}

void Print::MULi ( stringstream &strInstString, long instruction )
{
	strInstString << "MULI";
	AddInstArgs ( strInstString, instruction, FTVMULI );
}

void Print::MULq ( stringstream &strInstString, long instruction )
{
	strInstString << "MULQ";
	AddInstArgs ( strInstString, instruction, FTVMULQ );
}

void Print::MULBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "MULBCX";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void Print::MULBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "MULBCY";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void Print::MULBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MULBCZ";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void Print::MULBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "MULBCW";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}




// ITOF //

void Print::ITOF0 ( stringstream &strInstString, long instruction )
{
	strInstString << "ITOF0";
	AddInstArgs ( strInstString, instruction, FTVITOF0 );
}

void Print::FTOI0 ( stringstream &strInstString, long instruction )
{
	strInstString << "FTOI0";
	AddInstArgs ( strInstString, instruction, FTVFTOI0 );
}

void Print::ITOF4 ( stringstream &strInstString, long instruction )
{
	strInstString << "ITOF4";
	AddInstArgs ( strInstString, instruction, FTVITOF4 );
}

void Print::FTOI4 ( stringstream &strInstString, long instruction )
{
	strInstString << "FTOI4";
	AddInstArgs ( strInstString, instruction, FTVFTOI4 );
}

void Print::ITOF12 ( stringstream &strInstString, long instruction )
{
	strInstString << "ITOF12";
	AddInstArgs ( strInstString, instruction, FTVITOF12 );
}

void Print::FTOI12 ( stringstream &strInstString, long instruction )
{
	strInstString << "FTOI12";
	AddInstArgs ( strInstString, instruction, FTVFTOI12 );
}

void Print::ITOF15 ( stringstream &strInstString, long instruction )
{
	strInstString << "ITOF15";
	AddInstArgs ( strInstString, instruction, FTVITOF15 );
}

void Print::FTOI15 ( stringstream &strInstString, long instruction )
{
	strInstString << "FTOI15";
	AddInstArgs ( strInstString, instruction, FTVFTOI15 );
}





// ADDA //

void Print::ADDA ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDA";
	AddInstArgs ( strInstString, instruction, FTVADDA );
}

void Print::ADDAi ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDAI";
	AddInstArgs ( strInstString, instruction, FTVADDAI );
}

void Print::ADDAq ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDAQ";
	AddInstArgs ( strInstString, instruction, FTVADDAQ );
}

void Print::ADDABCX ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDABCX";
	AddInstArgs ( strInstString, instruction, FTVADDABC );
}
void Print::ADDABCY ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDABCY";
	AddInstArgs ( strInstString, instruction, FTVADDABC );
}
void Print::ADDABCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDABCZ";
	AddInstArgs ( strInstString, instruction, FTVADDABC );
}
void Print::ADDABCW ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDABCW";
	AddInstArgs ( strInstString, instruction, FTVADDABC );
}




// SUBA //

void Print::SUBA ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBA";
	AddInstArgs ( strInstString, instruction, FTVSUBA );
}

void Print::SUBAi ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBAI";
	AddInstArgs ( strInstString, instruction, FTVSUBAI );
}

void Print::SUBAq ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBAQ";
	AddInstArgs ( strInstString, instruction, FTVSUBAQ );
}

void Print::SUBABCX ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBABCX";
	AddInstArgs ( strInstString, instruction, FTVSUBABC );
}
void Print::SUBABCY ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBABCY";
	AddInstArgs ( strInstString, instruction, FTVSUBABC );
}
void Print::SUBABCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBABCZ";
	AddInstArgs ( strInstString, instruction, FTVSUBABC );
}
void Print::SUBABCW ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBABCW";
	AddInstArgs ( strInstString, instruction, FTVSUBABC );
}



// MADDA //

void Print::MADDA ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDA";
	AddInstArgs ( strInstString, instruction, FTVMADDA );
}

void Print::MADDAi ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDAI";
	AddInstArgs ( strInstString, instruction, FTVMADDAI );
}

void Print::MADDAq ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDAQ";
	AddInstArgs ( strInstString, instruction, FTVMADDAQ );
}

void Print::MADDABCX ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDABCX";
	AddInstArgs ( strInstString, instruction, FTVMADDABC );
}

void Print::MADDABCY ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDABCY";
	AddInstArgs ( strInstString, instruction, FTVMADDABC );
}

void Print::MADDABCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDABCZ";
	AddInstArgs ( strInstString, instruction, FTVMADDABC );
}

void Print::MADDABCW ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDABCW";
	AddInstArgs ( strInstString, instruction, FTVMADDABC );
}


// MSUBA //

void Print::MSUBA ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBA";
	AddInstArgs ( strInstString, instruction, FTVMSUBA );
}

void Print::MSUBAi ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBAI";
	AddInstArgs ( strInstString, instruction, FTVMSUBAI );
}

void Print::MSUBAq ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBAQ";
	AddInstArgs ( strInstString, instruction, FTVMSUBAQ );
}

void Print::MSUBABCX ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBABCX";
	AddInstArgs ( strInstString, instruction, FTVMSUBABC );
}

void Print::MSUBABCY ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBABCY";
	AddInstArgs ( strInstString, instruction, FTVMSUBABC );
}

void Print::MSUBABCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBABCZ";
	AddInstArgs ( strInstString, instruction, FTVMSUBABC );
}

void Print::MSUBABCW ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBABCW";
	AddInstArgs ( strInstString, instruction, FTVMSUBABC );
}



// MULA //

void Print::MULA ( stringstream &strInstString, long instruction )
{
	strInstString << "MULA";
	AddInstArgs ( strInstString, instruction, FTVMULA );
}

void Print::MULAi ( stringstream &strInstString, long instruction )
{
	strInstString << "MULAI";
	AddInstArgs ( strInstString, instruction, FTVMULAI );
}

void Print::MULAq ( stringstream &strInstString, long instruction )
{
	strInstString << "MULAQ";
	AddInstArgs ( strInstString, instruction, FTVMULAQ );
}

void Print::MULABCX ( stringstream &strInstString, long instruction )
{
	strInstString << "MULABCX";
	AddInstArgs ( strInstString, instruction, FTVMULABC );
}

void Print::MULABCY ( stringstream &strInstString, long instruction )
{
	strInstString << "MULABCY";
	AddInstArgs ( strInstString, instruction, FTVMULABC );
}

void Print::MULABCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MULABCZ";
	AddInstArgs ( strInstString, instruction, FTVMULABC );
}

void Print::MULABCW ( stringstream &strInstString, long instruction )
{
	strInstString << "MULABCW";
	AddInstArgs ( strInstString, instruction, FTVMULABC );
}







// other upper instructions //

void Print::CLIP ( stringstream &strInstString, long instruction )
{
	strInstString << "CLIP";
	AddInstArgs ( strInstString, instruction, FTVCLIP );
}



void Print::OPMULA ( stringstream &strInstString, long instruction )
{
	strInstString << "OPMULA";
	AddInstArgs ( strInstString, instruction, FTVOPMULA );
}

void Print::OPMSUB ( stringstream &strInstString, long instruction )
{
	strInstString << "OPMSUB";
	AddInstArgs ( strInstString, instruction, FTVOPMSUB );
}



void Print::NOP ( stringstream &strInstString, long instruction )
{
	strInstString << "NOP";
	AddInstArgs ( strInstString, instruction, FTVNOP );
}






//// *** LOWER instructions *** ////




// branch/jump instructions //

void Print::B ( stringstream &strInstString, long instruction )
{
	strInstString << "B";
	AddInstArgs ( strInstString, instruction, FORMAT43 );
}

void Print::BAL ( stringstream &strInstString, long instruction )
{
	strInstString << "BAL";
	AddInstArgs ( strInstString, instruction, FORMAT47 );
}

void Print::IBEQ ( stringstream &strInstString, long instruction )
{
	strInstString << "IBEQ";
	AddInstArgs ( strInstString, instruction, FORMAT45 );
}

void Print::IBNE ( stringstream &strInstString, long instruction )
{
	strInstString << "IBNE";
	AddInstArgs ( strInstString, instruction, FORMAT45 );
}

void Print::IBLTZ ( stringstream &strInstString, long instruction )
{
	strInstString << "IBLTZ";
	AddInstArgs ( strInstString, instruction, FORMAT46 );
}

void Print::IBGTZ ( stringstream &strInstString, long instruction )
{
	strInstString << "IBGTZ";
	AddInstArgs ( strInstString, instruction, FORMAT46 );
}

void Print::IBLEZ ( stringstream &strInstString, long instruction )
{
	strInstString << "IBLEZ";
	AddInstArgs ( strInstString, instruction, FORMAT46 );
}

void Print::IBGEZ ( stringstream &strInstString, long instruction )
{
	strInstString << "IBGEZ";
	AddInstArgs ( strInstString, instruction, FORMAT46 );
}

void Print::JR ( stringstream &strInstString, long instruction )
{
	strInstString << "JR";
	AddInstArgs ( strInstString, instruction, FORMAT49 );
}

void Print::JALR ( stringstream &strInstString, long instruction )
{
	strInstString << "JALR";
	AddInstArgs ( strInstString, instruction, FORMAT48 );
}









// FC/FM/FS instructions //

void Print::FCEQ ( stringstream &strInstString, long instruction )
{
	strInstString << "FCEQ";
	AddInstArgs ( strInstString, instruction, FORMAT54 );
}

void Print::FCAND ( stringstream &strInstString, long instruction )
{
	strInstString << "FCAND";
	AddInstArgs ( strInstString, instruction, FORMAT54 );
}

void Print::FCOR ( stringstream &strInstString, long instruction )
{
	strInstString << "FCOR";
	AddInstArgs ( strInstString, instruction, FORMAT54 );
}

void Print::FCGET ( stringstream &strInstString, long instruction )
{
	strInstString << "FCGET";
	AddInstArgs ( strInstString, instruction, FORMAT52 );
}

void Print::FCSET ( stringstream &strInstString, long instruction )
{
	strInstString << "FCSET";
	AddInstArgs ( strInstString, instruction, FORMAT54 );
}

void Print::FMEQ ( stringstream &strInstString, long instruction )
{
	strInstString << "FMEQ";
	AddInstArgs ( strInstString, instruction, FORMAT48 );
}

void Print::FMAND ( stringstream &strInstString, long instruction )
{
	strInstString << "FMAND";
	AddInstArgs ( strInstString, instruction, FORMAT48 );
}

void Print::FMOR ( stringstream &strInstString, long instruction )
{
	strInstString << "FMOR";
	AddInstArgs ( strInstString, instruction, FORMAT48 );
}

void Print::FSEQ ( stringstream &strInstString, long instruction )
{
	strInstString << "FSEQ";
	AddInstArgs ( strInstString, instruction, FORMAT56 );
}

void Print::FSSET ( stringstream &strInstString, long instruction )
{
	strInstString << "FSSET";
	AddInstArgs ( strInstString, instruction, FORMAT55 );
}

void Print::FSAND ( stringstream &strInstString, long instruction )
{
	strInstString << "FSAND";
	AddInstArgs ( strInstString, instruction, FORMAT56 );
}

void Print::FSOR ( stringstream &strInstString, long instruction )
{
	strInstString << "FSOR";
	AddInstArgs ( strInstString, instruction, FORMAT56 );
}



// Integer math //


void Print::IADD ( stringstream &strInstString, long instruction )
{
	strInstString << "IADD";
	AddInstArgs ( strInstString, instruction, FTVIADD );
}

void Print::IADDI ( stringstream &strInstString, long instruction )
{
	strInstString << "IADDI";
	AddInstArgs ( strInstString, instruction, FTVIADDI );
}

void Print::IADDIU ( stringstream &strInstString, long instruction )
{
	strInstString << "IADDIU";
	AddInstArgs ( strInstString, instruction, FORMAT57 );
}

void Print::ISUB ( stringstream &strInstString, long instruction )
{
	strInstString << "ISUB";
	AddInstArgs ( strInstString, instruction, FTVISUB );
}

void Print::ISUBIU ( stringstream &strInstString, long instruction )
{
	strInstString << "ISUBIU";
	AddInstArgs ( strInstString, instruction, FORMAT57 );
}


void Print::IAND ( stringstream &strInstString, long instruction )
{
	strInstString << "IAND";
	AddInstArgs ( strInstString, instruction, FTVIAND );
}

void Print::IOR ( stringstream &strInstString, long instruction )
{
	strInstString << "IOR";
	AddInstArgs ( strInstString, instruction, FTVIOR );
}




// Move instructions //

void Print::MFP ( stringstream &strInstString, long instruction )
{
	strInstString << "MFP";
	AddInstArgs ( strInstString, instruction, FORMAT39 );
}

void Print::MOVE ( stringstream &strInstString, long instruction )
{
	strInstString << "MOVE";
	AddInstArgs ( strInstString, instruction, FTVMOVE );
}

void Print::MTIR ( stringstream &strInstString, long instruction )
{
	strInstString << "MTIR";
	AddInstArgs ( strInstString, instruction, FTVMTIR );
}

void Print::MR32 ( stringstream &strInstString, long instruction )
{
	strInstString << "MR32";
	AddInstArgs ( strInstString, instruction, FTVMR32 );
}

void Print::MFIR ( stringstream &strInstString, long instruction )
{
	strInstString << "MFIR";
	AddInstArgs ( strInstString, instruction, FTVMFIR );
}



// Random Number instructions //

void Print::RGET ( stringstream &strInstString, long instruction )
{
	strInstString << "RGET";
	AddInstArgs ( strInstString, instruction, FTVRGET );
}

void Print::RNEXT ( stringstream &strInstString, long instruction )
{
	strInstString << "RNEXT";
	AddInstArgs ( strInstString, instruction, FTVRNEXT );
}

void Print::RINIT ( stringstream &strInstString, long instruction )
{
	strInstString << "RINIT";
	AddInstArgs ( strInstString, instruction, FTVRINIT );
}

void Print::RXOR ( stringstream &strInstString, long instruction )
{
	strInstString << "RXOR";
	AddInstArgs ( strInstString, instruction, FTVRXOR );
}




// Load/Store instructions //

void Print::SQ ( stringstream &strInstString, long instruction )
{
	strInstString << "SQ";
	AddInstArgs ( strInstString, instruction, FORMAT53 );
}

void Print::LQ ( stringstream &strInstString, long instruction )
{
	strInstString << "LQ";
	AddInstArgs ( strInstString, instruction, FORMAT50 );
}

void Print::SQD ( stringstream &strInstString, long instruction )
{
	strInstString << "SQD";
	AddInstArgs ( strInstString, instruction, FTVSQD );
}


void Print::SQI ( stringstream &strInstString, long instruction )
{
	strInstString << "SQI";
	AddInstArgs ( strInstString, instruction, FTVSQI );
}

void Print::LQD ( stringstream &strInstString, long instruction )
{
	strInstString << "LQD";
	AddInstArgs ( strInstString, instruction, FTVLQD );
}

void Print::LQI ( stringstream &strInstString, long instruction )
{
	strInstString << "LQI";
	AddInstArgs ( strInstString, instruction, FTVLQI );
}



void Print::ILWR ( stringstream &strInstString, long instruction )
{
	strInstString << "ILWR";
	AddInstArgs ( strInstString, instruction, FTVILWR );
}

void Print::ISWR ( stringstream &strInstString, long instruction )
{
	strInstString << "ISWR";
	AddInstArgs ( strInstString, instruction, FTVISWR );
}

void Print::ILW ( stringstream &strInstString, long instruction )
{
	strInstString << "ILW";
	AddInstArgs ( strInstString, instruction, FORMAT44 );
}

void Print::ISW ( stringstream &strInstString, long instruction )
{
	strInstString << "ISW";
	AddInstArgs ( strInstString, instruction, FORMAT44 );
}







// X instructions //

void Print::XGKICK ( stringstream &strInstString, long instruction )
{
	strInstString << "XGKICK";
	AddInstArgs ( strInstString, instruction, FORMAT49 );
}


void Print::XTOP ( stringstream &strInstString, long instruction )
{
	strInstString << "XTOP";
	AddInstArgs ( strInstString, instruction, FORMAT52 );
}

void Print::XITOP ( stringstream &strInstString, long instruction )
{
	strInstString << "XITOP";
	AddInstArgs ( strInstString, instruction, FORMAT52 );
}





// WAIT instructions //

void Print::WAITP ( stringstream &strInstString, long instruction )
{
	strInstString << "WAITP";
	//AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::WAITQ ( stringstream &strInstString, long instruction )
{
	strInstString << "WAITQ";
	AddInstArgs ( strInstString, instruction, FTVWAITQ );
}



// lower float math //

void Print::DIV ( stringstream &strInstString, long instruction )
{
	strInstString << "DIV";
	AddInstArgs ( strInstString, instruction, FTVDIV );
}

void Print::RSQRT ( stringstream &strInstString, long instruction )
{
	strInstString << "RSQRT";
	AddInstArgs ( strInstString, instruction, FTVRSQRT );
}

void Print::SQRT ( stringstream &strInstString, long instruction )
{
	strInstString << "SQRT";
	AddInstArgs ( strInstString, instruction, FTVSQRT );
}




// External unit //

void Print::EATAN ( stringstream &strInstString, long instruction )
{
	strInstString << "EATAN";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::EATANxy ( stringstream &strInstString, long instruction )
{
	strInstString << "EATANxy";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::EATANxz ( stringstream &strInstString, long instruction )
{
	strInstString << "EATANxz";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::ERSQRT ( stringstream &strInstString, long instruction )
{
	strInstString << "ERSQRT";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::ERCPR ( stringstream &strInstString, long instruction )
{
	strInstString << "ERCPR";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::EEXP ( stringstream &strInstString, long instruction )
{
	strInstString << "EEXP";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::ESIN ( stringstream &strInstString, long instruction )
{
	strInstString << "ESIN";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::ESQRT ( stringstream &strInstString, long instruction )
{
	strInstString << "ESQRT";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::ESADD ( stringstream &strInstString, long instruction )
{
	strInstString << "ESADD";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void Print::ERSADD ( stringstream &strInstString, long instruction )
{
	strInstString << "ERSADD";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}


void Print::ESUM ( stringstream &strInstString, long instruction )
{
	strInstString << "ESUM";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}


void Print::ELENG ( stringstream &strInstString, long instruction )
{
	strInstString << "ELENG";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}


void Print::ERLENG ( stringstream &strInstString, long instruction )
{
	strInstString << "ERLENG";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
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
			strMipsArgs << " r" << GET_RD( Instruction ) << ", r" << GET_RS( Instruction ) << ", r" << GET_RT( Instruction );
			break;
			
		case FORMAT2:
		
			//op rt, rs, immediate
			strMipsArgs << " r" << GET_RT( Instruction ) << ", r" << GET_RS( Instruction ) << ", " << GET_IMMED( Instruction );
			break;
			
		case FORMAT3:
		
			//op rs, rt, offset
			strMipsArgs << " r" << GET_RS( Instruction ) << ", r" << GET_RT( Instruction ) << ", " << GET_IMMED( Instruction );
			break;
			
		case FORMAT4:
		
			//op rs, offset
			strMipsArgs << " r" << GET_RS( Instruction ) << ", " << GET_IMMED( Instruction );
			break;
			
		case FORMAT5:
		
			//op rs, rt
			strMipsArgs << " r" << GET_RS( Instruction ) << ", r" << GET_RT( Instruction );
			break;
			
		case FORMAT6:
		
			//op rd, rt, sa
			strMipsArgs << " r" << GET_RD( Instruction ) << ", r" << GET_RT( Instruction ) << ", " << GET_SHIFT( Instruction );
			break;
			
		case FORMAT7:
		
			//op rd, rt, rs
			strMipsArgs << " r" << GET_RD( Instruction ) << ", r" << GET_RT( Instruction ) << ", r" << GET_RS( Instruction );
			break;

		case FORMAT8:
		
			//op target
			strMipsArgs << " " << GET_ADDRESS( Instruction );
			break;
			
		case FORMAT9:
		
			//op rd, rs
			strMipsArgs << " r" << GET_RD( Instruction ) << ", " << GET_RS( Instruction );
			break;
			
		case FORMAT10:
		
			//op rs
			strMipsArgs << " r" << GET_RS( Instruction );
			break;
			
		case FORMAT11:
		
			//op rt, offset (base)
			strMipsArgs << " r" << GET_RT( Instruction ) << ", " << GET_IMMED( Instruction ) << "(r" << GET_BASE( Instruction ) << ")";
			break;
			
		case FORMAT12:
		
			//op rt, immediate
			strMipsArgs << " r" << GET_RT( Instruction ) << ", " << GET_IMMED( Instruction );
			break;
			
		case FORMAT13:
		
			//op rd
			strMipsArgs << " r" << GET_RD( Instruction );
			break;
			
		case FORMAT14:
		
			//op hint, offset (base)
			strMipsArgs << " " << GET_HINT( Instruction ) << ", " << GET_IMMED( Instruction ) << "(r" << GET_BASE( Instruction ) << ")";
			break;
			
		case FORMAT15:
		
			//op rd, rt
			strMipsArgs << " r" << GET_RD( Instruction ) << ", r" << GET_RT( Instruction );
			break;
			
		case FORMAT16:
		
			//op
			break;
			
		case FORMAT17:
		
			//op rt
			strMipsArgs << " r" << GET_RT( Instruction );
			break;
			
		case FORMAT18:
		
			//op rt, reg
			strMipsArgs << " r" << GET_RT( Instruction ) << ", " << GET_REG( Instruction );
			break;
			
		case FORMAT19:

			//op rt, rd
			strMipsArgs << " r" << GET_RT ( Instruction ) << ", r" << GET_RD( Instruction );
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
			strMipsArgs << " f" << GET_FT( Instruction ) << ", f" << GET_FS( Instruction );
			break;
			
		case FORMAT27:
		
			//op.dest fd, fs, ft
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " f" << GET_FD( Instruction ) << ", f" << GET_FS( Instruction ) << ", f" << GET_FT( Instruction );
			break;
			
		case FORMAT28:
		
			//op.dest fd, fs
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " f" << GET_FD( Instruction ) << ", f" << GET_FS( Instruction );
			break;
			
		case FORMAT29:

			//op.dest fs, ft
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " f" << GET_FS( Instruction ) << ", f" << GET_FT( Instruction );
			break;
			
		case FORMAT30:
		
			//op.dest fs
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " f" << GET_FS( Instruction );
			break;
			
		case FORMAT31:

			//op Imm15
			strMipsArgs << " " << GET_IMM15( Instruction );
			break;
			
		case FORMAT32:
		
			//op fsfsf, ftftf
			strMipsArgs << " f" << GET_FS( Instruction ) << "." << XyzwLUT [ GET_FSF( Instruction ) ] << ", f" << GET_FT( Instruction ) << "." << XyzwLUT [ GET_FTF( Instruction ) ];
			break;
			
		case FORMAT33:
		
			//op id, is, it
			strMipsArgs << " i" << GET_ID( Instruction ) << ", i" << GET_IS( Instruction ) << ", i" << GET_IT( Instruction );
			break;
			
		case FORMAT34:
		
			//op it, is, Imm5
			strMipsArgs << " i" << GET_IT( Instruction ) << ", i" << GET_IS( Instruction ) << ", " << GET_IMM5( Instruction );
			break;


		case FORMAT35:
		
			//op.dest it, (is)
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " i" << GET_IT( Instruction ) << ", (i" << GET_IS( Instruction ) << ")";
			break;
			
		case FORMAT36:
		
			//op fsfsf
			strMipsArgs << " f" << GET_FS( Instruction ) << "." << XyzwLUT [ GET_FSF( Instruction ) ];
			break;
			
		case FORMAT37:
		
			//op ft, offset (base)
			strMipsArgs << " f" << GET_FT( Instruction ) << ", " << GET_IMMED( Instruction ) << "(r" << GET_BASE( Instruction ) << ")";
			break;
			
		case FORMAT38:
		
			//op ftftf
			strMipsArgs << " f" << GET_FT( Instruction ) << "." << XyzwLUT [ GET_FTF( Instruction ) ];
			break;
			
		case FORMAT39:
		
			//op.dest ft
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " f" << GET_FT( Instruction );
			break;
			
		case FORMAT40:
		
			//op.dest fs, (it)
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " f" << GET_FS( Instruction ) << ", (i" << GET_IT( Instruction ) << ")";
			break;
			
		case FORMAT41:
		
			//op it, fsfsf
			strMipsArgs << " i" << GET_IT( Instruction ) << ", f" << GET_FS( Instruction ) << "." << XyzwLUT [ GET_FSF( Instruction ) ];
			break;

		case FORMAT42:

			//op.dest ft, is
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " f" << GET_FT( Instruction ) << ", i" << GET_IS( Instruction );
			break;

		case FORMAT43:

			//op Imm11
			strMipsArgs << " " << GET_IMM11( Instruction );
			break;
			
		case FORMAT44:

			//op.dest it, Imm11 ( is )
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " i" << GET_IT( Instruction ) << ", " << GET_IMM11( Instruction ) << "(i" << GET_IS( Instruction ) << ")";
			break;
			
		case FORMAT45:
		
			//op it, is, Imm11
			strMipsArgs << " i" << GET_IT( Instruction ) << ", i" << GET_IS( Instruction ) << ", " << GET_IMM11( Instruction );
			break;
			
		case FORMAT46:
		
			//op is, Imm11
			strMipsArgs << " i" << GET_IS( Instruction ) << ", " << GET_IMM11( Instruction );
			break;
			
		case FORMAT47:
		
			//op it, Imm11
			strMipsArgs << " i" << GET_IT( Instruction ) << ", " << GET_IMM11( Instruction );
			break;
			
		case FORMAT48:
		
			//op it, is
			strMipsArgs << " i" << GET_IT( Instruction ) << ", i" << GET_IS( Instruction );
			break;
			
		case FORMAT49:
		
			//op is
			strMipsArgs << " i" << GET_IS( Instruction );
			break;
			
		case FORMAT50:

			//op.dest ft, Imm11 ( is )
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " f" << GET_FT( Instruction ) << ", " << GET_IMM11( Instruction ) << "(i" << GET_IS( Instruction ) << ")";
			break;
			
		case FORMAT51:

			//op.dest ft, ( is )
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " f" << GET_FT( Instruction ) << ", " << "(i" << GET_IS( Instruction ) << ")";
			break;
			
			
		case FORMAT52:
		
			//op it
			strMipsArgs << " i" << GET_IT( Instruction );
			break;
			
		case FORMAT53:

			//op.dest fs, Imm11 ( it )
			AddVuDestArgs ( strMipsArgs, Instruction );
			strMipsArgs << " f" << GET_FS( Instruction ) << ", " << GET_IMM11( Instruction ) << "(i" << GET_IT( Instruction ) << ")";
			break;
			
		case FORMAT54:

			//op Imm24
			strMipsArgs << " " << GET_IMM24( Instruction );
			break;
			
		case FORMAT55:

			//op Imm12
			strMipsArgs << " " << GET_IMM12( Instruction );
			break;
			
		case FORMAT56:

			//op it, Imm12
			strMipsArgs << " i" << GET_IT( Instruction ) << ", " << GET_IMM12( Instruction );
			break;

		case FORMAT57:
		
			//op it, is, Imm15
			strMipsArgs << " i" << GET_IT( Instruction ) << ", i" << GET_IS( Instruction ) << ", " << GET_IMM15( Instruction );
			break;
	}

}



const Print::Function Print::FunctionList []
{
	Print::INVALID,
	
	// VU macro mode instructions //
	
	//Print::COP2
	//Print::QMFC2_NI, Print::QMFC2_I, Print::QMTC2_NI, Print::QMTC2_I, Print::LQC2, Print::SQC2,
	//Print::CALLMS, Print::CALLMSR,
	
	// upper instructions //
	
	Print::ABS,
	Print::ADD, Print::ADDi, Print::ADDq, Print::ADDBCX, Print::ADDBCY, Print::ADDBCZ, Print::ADDBCW,
	Print::ADDA, Print::ADDAi, Print::ADDAq, Print::ADDABCX, Print::ADDABCY, Print::ADDABCZ, Print::ADDABCW,
	Print::CLIP,
	Print::FTOI0, Print::FTOI4, Print::FTOI12, Print::FTOI15,
	Print::ITOF0, Print::ITOF4, Print::ITOF12, Print::ITOF15,
	
	Print::MADD, Print::MADDi, Print::MADDq, Print::MADDBCX, Print::MADDBCY, Print::MADDBCZ, Print::MADDBCW,
	Print::MADDA, Print::MADDAi, Print::MADDAq, Print::MADDABCX, Print::MADDABCY, Print::MADDABCZ, Print::MADDABCW,
	Print::MAX, Print::MAXi, Print::MAXBCX, Print::MAXBCY, Print::MAXBCZ, Print::MAXBCW,
	Print::MINI, Print::MINIi, Print::MINIBCX, Print::MINIBCY, Print::MINIBCZ, Print::MINIBCW,
	
	Print::MSUB, Print::MSUBi, Print::MSUBq, Print::MSUBBCX, Print::MSUBBCY, Print::MSUBBCZ, Print::MSUBBCW,
	Print::MSUBA, Print::MSUBAi, Print::MSUBAq, Print::MSUBABCX, Print::MSUBABCY, Print::MSUBABCZ, Print::MSUBABCW,
	Print::MUL, Print::MULi, Print::MULq, Print::MULBCX, Print::MULBCY, Print::MULBCZ, Print::MULBCW,
	Print::MULA, Print::MULAi, Print::MULAq, Print::MULABCX, Print::MULABCY, Print::MULABCZ, Print::MULABCW,
	Print::NOP, Print::OPMSUB, Print::OPMULA,
	Print::SUB, Print::SUBi, Print::SUBq, Print::SUBBCX, Print::SUBBCY, Print::SUBBCZ, Print::SUBBCW,
	Print::SUBA, Print::SUBAi, Print::SUBAq, Print::SUBABCX, Print::SUBABCY, Print::SUBABCZ, Print::SUBABCW,
	
	// lower instructions //
	
	Print::DIV,
	Print::IADD, Print::IADDI, Print::IAND,
	Print::ILWR,
	Print::IOR, Print::ISUB,
	Print::ISWR,
	Print::LQD, Print::LQI,
	Print::MFIR, Print::MOVE, Print::MR32, Print::MTIR,
	Print::RGET, Print::RINIT, Print::RNEXT,
	Print::RSQRT,
	Print::RXOR,
	Print::SQD, Print::SQI,
	Print::SQRT,
	Print::WAITQ,

	// instructions not in macro mode //
	
	Print::B, Print::BAL,
	Print::FCAND, Print::FCEQ, Print::FCGET, Print::FCOR, Print::FCSET,
	Print::FMAND, Print::FMEQ, Print::FMOR,
	Print::FSAND, Print::FSEQ, Print::FSOR, Print::FSSET,
	Print::IADDIU,
	Print::IBEQ, Print::IBGEZ, Print::IBGTZ, Print::IBLEZ, Print::IBLTZ, Print::IBNE,
	Print::ILW,
	Print::ISUBIU, Print::ISW,
	Print::JALR, Print::JR,
	Print::LQ,
	Print::MFP,
	Print::SQ,
	Print::WAITP,
	Print::XGKICK, Print::XITOP, Print::XTOP,

	// External Unit //

	Print::EATAN, Print::EATANxy, Print::EATANxz, Print::EEXP, Print::ELENG, Print::ERCPR, Print::ERLENG, Print::ERSADD,
	Print::ERSQRT, Print::ESADD, Print::ESIN, Print::ESQRT, Print::ESUM
};





