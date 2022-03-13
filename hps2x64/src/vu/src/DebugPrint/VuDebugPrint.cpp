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


void VuDebugPrint::PrintInstruction ( stringstream &strInstString, unsigned long long instruction )
{
	VLIWOpcodes [ ( instruction >> 59 ) & 0x1f ] ( strInstString, instruction );
}

void VuDebugPrint::Normal ( stringstream &strInstString, unsigned long long instruction )
{
	PrintUpperInstruction ( strInstString, (long) ( ( instruction >> 32 ) & 0xffffffff ) );
	strInstString << " ; ";
	PrintLowerInstruction ( strInstString, (long) ( instruction & 0xffffffff ) );
}

void VuDebugPrint::IImmediate ( stringstream &strInstString, unsigned long long instruction )
{
}

void VuDebugPrint::EBit ( stringstream &strInstString, unsigned long long instruction )
{
	// "E" must stand for "Exit"
}

void VuDebugPrint::MInterlock ( stringstream &strInstString, unsigned long long instruction )
{
}

void VuDebugPrint::TDebug ( stringstream &strInstString, unsigned long long instruction )
{
}

void VuDebugPrint::DDebug ( stringstream &strInstString, unsigned long long instruction )
{
}



void VuDebugPrint::PrintUpperInstruction ( stringstream &strInstString, long instruction )
{
	UpperSpecialCodes [ GET_SPECIAL( instruction ) ] ( strInstString, instruction );
}

void VuDebugPrint::PrintLowerInstruction ( stringstream &strInstString, long instruction )
{
	LowerOpcodes [ ( instruction >> 25 ) & 0x7f ] ( strInstString, instruction );
}

void VuDebugPrint::LowerOp ( stringstream &strInstString, long instruction )
{
	LowerSpecialCodes [ GET_SPECIAL( instruction ) ] ( strInstString, instruction );
}


void VuDebugPrint::Upper60 ( stringstream &strInstString, long instruction )
{
	Upper60B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}

void VuDebugPrint::Upper61 ( stringstream &strInstString, long instruction )
{
	Upper61B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}

void VuDebugPrint::Upper62 ( stringstream &strInstString, long instruction )
{
	Upper62B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}

void VuDebugPrint::Upper63 ( stringstream &strInstString, long instruction )
{
	Upper63B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}



void VuDebugPrint::Lower60 ( stringstream &strInstString, long instruction )
{
	Lower60B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}

void VuDebugPrint::Lower61 ( stringstream &strInstString, long instruction )
{
	Lower61B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}

void VuDebugPrint::Lower62 ( stringstream &strInstString, long instruction )
{
	Lower62B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}

void VuDebugPrint::Lower63 ( stringstream &strInstString, long instruction )
{
	Lower63B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}









void VuDebugPrint::WAITP ( stringstream &strInstString, long instruction )
{
	strInstString << "WAITP";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}


void VuDebugPrint::ERCPR ( stringstream &strInstString, long instruction )
{
	strInstString << "ERCPR";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::EEXP ( stringstream &strInstString, long instruction )
{
	strInstString << "EEXP";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::XGKICK ( stringstream &strInstString, long instruction )
{
	strInstString << "XGKICK";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}


void VuDebugPrint::XTOP ( stringstream &strInstString, long instruction )
{
	strInstString << "XTOP";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::XITOP ( stringstream &strInstString, long instruction )
{
	strInstString << "XITOP";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::ERSQRT ( stringstream &strInstString, long instruction )
{
	strInstString << "ERSQRT";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::MFP ( stringstream &strInstString, long instruction )
{
	strInstString << "MFP";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::B ( stringstream &strInstString, long instruction )
{
	strInstString << "B";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::BAL ( stringstream &strInstString, long instruction )
{
	strInstString << "BAL";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::IBEQ ( stringstream &strInstString, long instruction )
{
	strInstString << "IBEQ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::IBNE ( stringstream &strInstString, long instruction )
{
	strInstString << "IBNE";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::IBLTZ ( stringstream &strInstString, long instruction )
{
	strInstString << "IBLTZ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::IBGTZ ( stringstream &strInstString, long instruction )
{
	strInstString << "IBGTZ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::IBLEZ ( stringstream &strInstString, long instruction )
{
	strInstString << "IBLEZ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::IBGEZ ( stringstream &strInstString, long instruction )
{
	strInstString << "IBGEZ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::JR ( stringstream &strInstString, long instruction )
{
	strInstString << "JR";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::JALR ( stringstream &strInstString, long instruction )
{
	strInstString << "JALR";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::EATAN ( stringstream &strInstString, long instruction )
{
	strInstString << "EATAN";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::EATANxy ( stringstream &strInstString, long instruction )
{
	strInstString << "EATANxy";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::EATANxz ( stringstream &strInstString, long instruction )
{
	strInstString << "EATANxz";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::FCEQ ( stringstream &strInstString, long instruction )
{
	strInstString << "FCEQ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::FCAND ( stringstream &strInstString, long instruction )
{
	strInstString << "FCAND";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::FCOR ( stringstream &strInstString, long instruction )
{
	strInstString << "FCOR";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::FCGET ( stringstream &strInstString, long instruction )
{
	strInstString << "FCGET";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::FCSET ( stringstream &strInstString, long instruction )
{
	strInstString << "FCSET";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::FMEQ ( stringstream &strInstString, long instruction )
{
	strInstString << "FMEQ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::FMAND ( stringstream &strInstString, long instruction )
{
	strInstString << "FMAND";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::FMOR ( stringstream &strInstString, long instruction )
{
	strInstString << "FMOR";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::FSEQ ( stringstream &strInstString, long instruction )
{
	strInstString << "FSEQ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::FSSET ( stringstream &strInstString, long instruction )
{
	strInstString << "FSSET";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::FSAND ( stringstream &strInstString, long instruction )
{
	strInstString << "FSAND";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::FSOR ( stringstream &strInstString, long instruction )
{
	strInstString << "FSOR";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::ISUBIU ( stringstream &strInstString, long instruction )
{
	strInstString << "ISUBIU";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::IADDIU ( stringstream &strInstString, long instruction )
{
	strInstString << "IADDIU";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}


void VuDebugPrint::ILW ( stringstream &strInstString, long instruction )
{
	strInstString << "ILW";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::ISW ( stringstream &strInstString, long instruction )
{
	strInstString << "ISW";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::SQ ( stringstream &strInstString, long instruction )
{
	strInstString << "SQ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::LQ ( stringstream &strInstString, long instruction )
{
	strInstString << "LQ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::ESIN ( stringstream &strInstString, long instruction )
{
	strInstString << "ESIN";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::ESQRT ( stringstream &strInstString, long instruction )
{
	strInstString << "ESQRT";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::ESADD ( stringstream &strInstString, long instruction )
{
	strInstString << "ESADD";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::ERSADD ( stringstream &strInstString, long instruction )
{
	strInstString << "ERSADD";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}


void VuDebugPrint::ESUM ( stringstream &strInstString, long instruction )
{
	strInstString << "ESUM";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}


void VuDebugPrint::ELENG ( stringstream &strInstString, long instruction )
{
	strInstString << "ELENG";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}


void VuDebugPrint::ERLENG ( stringstream &strInstString, long instruction )
{
	strInstString << "ERLENG";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}


void VuDebugPrint::None ( stringstream &strInstString, long instruction )
{
	strInstString << "UNKNOWN OPCODE";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::NoneVLIW ( stringstream &strInstString, unsigned long long instruction )
{
	strInstString << "UNKNOWN VLIW OPCODE";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::ADDBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDBCX";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::ADDBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDBCY";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::ADDBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDBCZ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::ADDBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDBCW";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void VuDebugPrint::SUBBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBBCX";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void VuDebugPrint::SUBBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBBCY";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void VuDebugPrint::SUBBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBBCZ";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void VuDebugPrint::SUBBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBBCW";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void VuDebugPrint::MADDBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDBCX";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void VuDebugPrint::MADDBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDBCY";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void VuDebugPrint::MADDBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDBCZ";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void VuDebugPrint::MADDBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDBCW";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void VuDebugPrint::MSUBBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBBCX";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void VuDebugPrint::MSUBBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBBCY";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void VuDebugPrint::MSUBBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBBCZ";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void VuDebugPrint::MSUBBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBBCW";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void VuDebugPrint::MAXBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "MAXBCX";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void VuDebugPrint::MAXBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "MAXBCY";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void VuDebugPrint::MAXBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MAXBCZ";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void VuDebugPrint::MAXBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "MAXBCW";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void VuDebugPrint::MINIBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "MINIBCX";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void VuDebugPrint::MINIBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "MINIBCY";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void VuDebugPrint::MINIBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MINIBCZ";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void VuDebugPrint::MINIBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "MINIBCW";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void VuDebugPrint::MULBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "MULBCX";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void VuDebugPrint::MULBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "MULBCY";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void VuDebugPrint::MULBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MULBCZ";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void VuDebugPrint::MULBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "MULBCW";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void VuDebugPrint::MULQ ( stringstream &strInstString, long instruction )
{
	strInstString << "MULQ";
	AddInstArgs ( strInstString, instruction, FTVMULQ );
}

void VuDebugPrint::MAXI ( stringstream &strInstString, long instruction )
{
	strInstString << "MAXI";
	AddInstArgs ( strInstString, instruction, FTVMAXI );
}

void VuDebugPrint::MULI ( stringstream &strInstString, long instruction )
{
	strInstString << "MULI";
	AddInstArgs ( strInstString, instruction, FTVMULI );
}

void VuDebugPrint::MINII ( stringstream &strInstString, long instruction )
{
	strInstString << "MINII";
	AddInstArgs ( strInstString, instruction, FTVMINII );
}

void VuDebugPrint::ADDQ ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDQ";
	AddInstArgs ( strInstString, instruction, FTVADDQ );
}

void VuDebugPrint::MADDQ ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDQ";
	AddInstArgs ( strInstString, instruction, FTVMADDQ );
}

void VuDebugPrint::ADDI ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDI";
	AddInstArgs ( strInstString, instruction, FTVADDI );
}

void VuDebugPrint::MADDI ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDI";
	AddInstArgs ( strInstString, instruction, FTVMADDI );
}

void VuDebugPrint::SUBQ ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBQ";
	AddInstArgs ( strInstString, instruction, FTVSUBQ );
}

void VuDebugPrint::MSUBQ ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBQ";
	AddInstArgs ( strInstString, instruction, FTVMSUBQ );
}

void VuDebugPrint::SUBI ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBI";
	AddInstArgs ( strInstString, instruction, FTVSUBI );
}

void VuDebugPrint::MSUBI ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBI";
	AddInstArgs ( strInstString, instruction, FTVMSUBI );
}

void VuDebugPrint::ADD ( stringstream &strInstString, long instruction )
{
	strInstString << "ADD";
	AddInstArgs ( strInstString, instruction, FTVADD );
}

void VuDebugPrint::MADD ( stringstream &strInstString, long instruction )
{
	strInstString << "MADD";
	AddInstArgs ( strInstString, instruction, FTVMADD );
}

void VuDebugPrint::MUL ( stringstream &strInstString, long instruction )
{
	strInstString << "MUL";
	AddInstArgs ( strInstString, instruction, FTVMUL );
}

void VuDebugPrint::MAX ( stringstream &strInstString, long instruction )
{
	strInstString << "MAX";
	AddInstArgs ( strInstString, instruction, FTVMAX );
}

void VuDebugPrint::SUB ( stringstream &strInstString, long instruction )
{
	strInstString << "SUB";
	AddInstArgs ( strInstString, instruction, FTVSUB );
}

void VuDebugPrint::MSUB ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUB";
	AddInstArgs ( strInstString, instruction, FTVMSUB );
}

void VuDebugPrint::OPMSUB ( stringstream &strInstString, long instruction )
{
	strInstString << "OPMSUB";
	AddInstArgs ( strInstString, instruction, FTVOPMSUB );
}

void VuDebugPrint::MINI ( stringstream &strInstString, long instruction )
{
	strInstString << "MINI";
	AddInstArgs ( strInstString, instruction, FTVMINI );
}

void VuDebugPrint::IADD ( stringstream &strInstString, long instruction )
{
	strInstString << "IADD";
	AddInstArgs ( strInstString, instruction, FTVIADD );
}

void VuDebugPrint::ISUB ( stringstream &strInstString, long instruction )
{
	strInstString << "ISUB";
	AddInstArgs ( strInstString, instruction, FTVISUB );
}

void VuDebugPrint::IADDI ( stringstream &strInstString, long instruction )
{
	strInstString << "IADDI";
	AddInstArgs ( strInstString, instruction, FTVIADDI );
}

void VuDebugPrint::IAND ( stringstream &strInstString, long instruction )
{
	strInstString << "IAND";
	AddInstArgs ( strInstString, instruction, FTVIAND );
}

void VuDebugPrint::IOR ( stringstream &strInstString, long instruction )
{
	strInstString << "IOR";
	AddInstArgs ( strInstString, instruction, FTVIOR );
}

void VuDebugPrint::CALLMS ( stringstream &strInstString, long instruction )
{
	strInstString << "CALLMS";
	AddInstArgs ( strInstString, instruction, FTVCALLMS );
}

void VuDebugPrint::CALLMSR ( stringstream &strInstString, long instruction )
{
	strInstString << "CALLMSR";
	AddInstArgs ( strInstString, instruction, FTVCALLMSR );
}

void VuDebugPrint::ITOF0 ( stringstream &strInstString, long instruction )
{
	strInstString << "ITOF0";
	AddInstArgs ( strInstString, instruction, FTVITOF0 );
}

void VuDebugPrint::FTOI0 ( stringstream &strInstString, long instruction )
{
	strInstString << "FTOI0";
	AddInstArgs ( strInstString, instruction, FTVFTOI0 );
}

void VuDebugPrint::MULAQ ( stringstream &strInstString, long instruction )
{
	strInstString << "MULAQ";
	AddInstArgs ( strInstString, instruction, FTVMULAQ );
}

void VuDebugPrint::ADDAQ ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDAQ";
	AddInstArgs ( strInstString, instruction, FTVADDAQ );
}

void VuDebugPrint::SUBAQ ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBAQ";
	AddInstArgs ( strInstString, instruction, FTVSUBAQ );
}

void VuDebugPrint::ADDA ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDA";
	AddInstArgs ( strInstString, instruction, FTVADDA );
}

void VuDebugPrint::SUBA ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBA";
	AddInstArgs ( strInstString, instruction, FTVSUBA );
}

void VuDebugPrint::MOVE ( stringstream &strInstString, long instruction )
{
	strInstString << "MOVE";
	AddInstArgs ( strInstString, instruction, FTVMOVE );
}

void VuDebugPrint::LQI ( stringstream &strInstString, long instruction )
{
	strInstString << "LQI";
	AddInstArgs ( strInstString, instruction, FTVLQI );
}

void VuDebugPrint::DIV ( stringstream &strInstString, long instruction )
{
	strInstString << "DIV";
	AddInstArgs ( strInstString, instruction, FTVDIV );
}

void VuDebugPrint::MTIR ( stringstream &strInstString, long instruction )
{
	strInstString << "MTIR";
	AddInstArgs ( strInstString, instruction, FTVMTIR );
}

void VuDebugPrint::RNEXT ( stringstream &strInstString, long instruction )
{
	strInstString << "RNEXT";
	AddInstArgs ( strInstString, instruction, FTVRNEXT );
}

void VuDebugPrint::ITOF4 ( stringstream &strInstString, long instruction )
{
	strInstString << "ITOF4";
	AddInstArgs ( strInstString, instruction, FTVITOF4 );
}

void VuDebugPrint::FTOI4 ( stringstream &strInstString, long instruction )
{
	strInstString << "FTOI4";
	AddInstArgs ( strInstString, instruction, FTVFTOI4 );
}

void VuDebugPrint::ABS ( stringstream &strInstString, long instruction )
{
	strInstString << "ABS";
	AddInstArgs ( strInstString, instruction, FTVABS );
}

void VuDebugPrint::MADDAQ ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDAQ";
	AddInstArgs ( strInstString, instruction, FTVMADDAQ );
}

void VuDebugPrint::MSUBAQ ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBAQ";
	AddInstArgs ( strInstString, instruction, FTVMSUBAQ );
}

void VuDebugPrint::MADDA ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDA";
	AddInstArgs ( strInstString, instruction, FTVMADDA );
}

void VuDebugPrint::MSUBA ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBA";
	AddInstArgs ( strInstString, instruction, FTVMSUBA );
}

void VuDebugPrint::MR32 ( stringstream &strInstString, long instruction )
{
	strInstString << "MR32";
	AddInstArgs ( strInstString, instruction, FTVMR32 );
}

void VuDebugPrint::SQI ( stringstream &strInstString, long instruction )
{
	strInstString << "SQI";
	AddInstArgs ( strInstString, instruction, FTVSQI );
}

void VuDebugPrint::SQRT ( stringstream &strInstString, long instruction )
{
	strInstString << "SQRT";
	AddInstArgs ( strInstString, instruction, FTVSQRT );
}

void VuDebugPrint::MFIR ( stringstream &strInstString, long instruction )
{
	strInstString << "MFIR";
	AddInstArgs ( strInstString, instruction, FTVMFIR );
}

void VuDebugPrint::RGET ( stringstream &strInstString, long instruction )
{
	strInstString << "RGET";
	AddInstArgs ( strInstString, instruction, FTVRGET );
}

void VuDebugPrint::ADDABC ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDABC";
	AddInstArgs ( strInstString, instruction, FTVADDABC );
}

void VuDebugPrint::SUBABC ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBABC";
	AddInstArgs ( strInstString, instruction, FTVSUBABC );
}

void VuDebugPrint::MADDABC ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDABC";
	AddInstArgs ( strInstString, instruction, FTVMADDABC );
}

void VuDebugPrint::MSUBABC ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBABC";
	AddInstArgs ( strInstString, instruction, FTVMSUBABC );
}

void VuDebugPrint::ITOF12 ( stringstream &strInstString, long instruction )
{
	strInstString << "ITOF12";
	AddInstArgs ( strInstString, instruction, FTVITOF12 );
}

void VuDebugPrint::FTOI12 ( stringstream &strInstString, long instruction )
{
	strInstString << "FTOI12";
	AddInstArgs ( strInstString, instruction, FTVFTOI12 );
}

void VuDebugPrint::MULABC ( stringstream &strInstString, long instruction )
{
	strInstString << "MULABC";
	AddInstArgs ( strInstString, instruction, FTVMULABC );
}

void VuDebugPrint::MULAI ( stringstream &strInstString, long instruction )
{
	strInstString << "MULAI";
	AddInstArgs ( strInstString, instruction, FTVMULAI );
}

void VuDebugPrint::ADDAI ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDAI";
	AddInstArgs ( strInstString, instruction, FTVADDAI );
}

void VuDebugPrint::SUBAI ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBAI";
	AddInstArgs ( strInstString, instruction, FTVSUBAI );
}

void VuDebugPrint::MULA ( stringstream &strInstString, long instruction )
{
	strInstString << "MULA";
	AddInstArgs ( strInstString, instruction, FTVMULA );
}

void VuDebugPrint::OPMULA ( stringstream &strInstString, long instruction )
{
	strInstString << "OPMULA";
	AddInstArgs ( strInstString, instruction, FTVOPMULA );
}

void VuDebugPrint::LQD ( stringstream &strInstString, long instruction )
{
	strInstString << "LQD";
	AddInstArgs ( strInstString, instruction, FTVLQD );
}

void VuDebugPrint::RSQRT ( stringstream &strInstString, long instruction )
{
	strInstString << "RSQRT";
	AddInstArgs ( strInstString, instruction, FTVRSQRT );
}

void VuDebugPrint::ILWR ( stringstream &strInstString, long instruction )
{
	strInstString << "ILWR";
	AddInstArgs ( strInstString, instruction, FTVILWR );
}

void VuDebugPrint::RINIT ( stringstream &strInstString, long instruction )
{
	strInstString << "RINIT";
	AddInstArgs ( strInstString, instruction, FTVRINIT );
}

void VuDebugPrint::ITOF15 ( stringstream &strInstString, long instruction )
{
	strInstString << "ITOF15";
	AddInstArgs ( strInstString, instruction, FTVITOF15 );
}

void VuDebugPrint::FTOI15 ( stringstream &strInstString, long instruction )
{
	strInstString << "FTOI15";
	AddInstArgs ( strInstString, instruction, FTVFTOI15 );
}

void VuDebugPrint::CLIP ( stringstream &strInstString, long instruction )
{
	strInstString << "CLIP";
	AddInstArgs ( strInstString, instruction, FTVCLIP );
}

void VuDebugPrint::MADDAI ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDAI";
	AddInstArgs ( strInstString, instruction, FTVMADDAI );
}

void VuDebugPrint::MSUBAI ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBAI";
	AddInstArgs ( strInstString, instruction, FTVMSUBAI );
}

void VuDebugPrint::NOP ( stringstream &strInstString, long instruction )
{
	strInstString << "NOP";
	AddInstArgs ( strInstString, instruction, FTVNOP );
}

void VuDebugPrint::SQD ( stringstream &strInstString, long instruction )
{
	strInstString << "SQD";
	AddInstArgs ( strInstString, instruction, FTVSQD );
}

void VuDebugPrint::WAITQ ( stringstream &strInstString, long instruction )
{
	strInstString << "WAITQ";
	AddInstArgs ( strInstString, instruction, FTVWAITQ );
}

void VuDebugPrint::ISWR ( stringstream &strInstString, long instruction )
{
	strInstString << "ISWR";
	AddInstArgs ( strInstString, instruction, FTVISWR );
}

void VuDebugPrint::RXOR ( stringstream &strInstString, long instruction )
{
	strInstString << "RXOR";
	AddInstArgs ( strInstString, instruction, FTVRXOR );
}












void VuDebugPrint::AddVuDestArgs ( stringstream &strVuArgs, long Instruction )
{
	// add the dot
	strVuArgs << ".";
	
	if ( GET_DESTX( Instruction ) ) strVuArgs << "x";
	if ( GET_DESTY( Instruction ) ) strVuArgs << "y";
	if ( GET_DESTZ( Instruction ) ) strVuArgs << "z";
	if ( GET_DESTW( Instruction ) ) strVuArgs << "w";
}


void VuDebugPrint::AddInstArgs ( stringstream &strMipsArgs, long Instruction, long InstFormat )
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
