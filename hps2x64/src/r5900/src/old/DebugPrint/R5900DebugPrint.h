/*
	Copyright (C) 2012-2016

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


#include <sstream>
#include <string>


#include "MipsOpcode.h"


namespace R5900DebugPrint
{
	using namespace std;

	typedef void (*Function) ( stringstream &s, long instruction );

	// this gets the instruction text
	void PrintInstruction ( stringstream &strInstString, long instruction );

	const char* BCType [ 4 ] = { "F", "T", "FL", "TL" };

	void BC0 ( stringstream &strInstString, long instruction );
	void BC1 ( stringstream &strInstString, long instruction );
	void BC2 ( stringstream &strInstString, long instruction );
	void VOPMSUB ( stringstream &strInstString, long instruction );
	void CO ( stringstream &strInstString, long instruction );
	void MF0 ( stringstream &strInstString, long instruction );
	void MT0 ( stringstream &strInstString, long instruction );
	void EI ( stringstream &strInstString, long instruction );
	void DI ( stringstream &strInstString, long instruction );
	void SD ( stringstream &strInstString, long instruction );
	void SW ( stringstream &strInstString, long instruction );
	void LW ( stringstream &strInstString, long instruction );
	void LD ( stringstream &strInstString, long instruction );
	void LHU ( stringstream &strInstString, long instruction );
	void LWR ( stringstream &strInstString, long instruction );
	void LWU ( stringstream &strInstString, long instruction );
	void SB ( stringstream &strInstString, long instruction );
	void SDL ( stringstream &strInstString, long instruction );
	void SDR ( stringstream &strInstString, long instruction );
	void SH ( stringstream &strInstString, long instruction );
	void SWL ( stringstream &strInstString, long instruction );
	void SWR ( stringstream &strInstString, long instruction );
	void LB ( stringstream &strInstString, long instruction );
	void LBU ( stringstream &strInstString, long instruction );
	void LDL ( stringstream &strInstString, long instruction );
	void LDR ( stringstream &strInstString, long instruction );
	void LH ( stringstream &strInstString, long instruction );
	void LWL ( stringstream &strInstString, long instruction );
	void LQ ( stringstream &strInstString, long instruction );
	void SQ ( stringstream &strInstString, long instruction );
	void VUSpecial ( stringstream &strInstString, long instruction );
	void LWC1 ( stringstream &strInstString, long instruction );
	void SWC1 ( stringstream &strInstString, long instruction );
	void SRL ( stringstream &strInstString, long instruction );


	void Special ( stringstream &strInstString, long instruction );
	void Regimm ( stringstream &strInstString, long instruction );
	void J ( stringstream &strInstString, long instruction );
	void JAL ( stringstream &strInstString, long instruction );
	void BEQ ( stringstream &strInstString, long instruction );
	void BNE ( stringstream &strInstString, long instruction );
	void BLEZ ( stringstream &strInstString, long instruction );
	void BGTZ ( stringstream &strInstString, long instruction );
	void ADDI ( stringstream &strInstString, long instruction );
	void ADDIU ( stringstream &strInstString, long instruction );
	void SLTI ( stringstream &strInstString, long instruction );
	void SLTIU ( stringstream &strInstString, long instruction );
	void ANDI ( stringstream &strInstString, long instruction );
	void ORI ( stringstream &strInstString, long instruction );
	void XORI ( stringstream &strInstString, long instruction );
	void LUI ( stringstream &strInstString, long instruction );
	void COP0 ( stringstream &strInstString, long instruction );
	void COP1 ( stringstream &strInstString, long instruction );
	void COP2 ( stringstream &strInstString, long instruction );
	void None ( stringstream &strInstString, long instruction );
	void BEQL ( stringstream &strInstString, long instruction );
	void BNEL ( stringstream &strInstString, long instruction );
	void BLEZL ( stringstream &strInstString, long instruction );
	void BGTZL ( stringstream &strInstString, long instruction );
	void DADDI ( stringstream &strInstString, long instruction );
	void DADDIU ( stringstream &strInstString, long instruction );
	void MMI ( stringstream &strInstString, long instruction );
	void CACHE ( stringstream &strInstString, long instruction );
	void PREF ( stringstream &strInstString, long instruction );
	void LQC2 ( stringstream &strInstString, long instruction );
	void SQC2 ( stringstream &strInstString, long instruction );
	void BLTZ ( stringstream &strInstString, long instruction );
	void BGEZ ( stringstream &strInstString, long instruction );
	void BLTZL ( stringstream &strInstString, long instruction );
	void TGEI ( stringstream &strInstString, long instruction );
	void TGEIU ( stringstream &strInstString, long instruction );
	void TLTI ( stringstream &strInstString, long instruction );
	void TLTIU ( stringstream &strInstString, long instruction );
	void TEQI ( stringstream &strInstString, long instruction );
	void TNEI ( stringstream &strInstString, long instruction );
	void BLTZAL ( stringstream &strInstString, long instruction );
	void BGEZAL ( stringstream &strInstString, long instruction );
	void BLTZALL ( stringstream &strInstString, long instruction );
	void BGEZALL ( stringstream &strInstString, long instruction );
	void SLL ( stringstream &strInstString, long instruction );
	void MOVCI ( stringstream &strInstString, long instruction );
	void SRA ( stringstream &strInstString, long instruction );
	void SLLV ( stringstream &strInstString, long instruction );
	void SRLV ( stringstream &strInstString, long instruction );
	void SRAV ( stringstream &strInstString, long instruction );
	void JR ( stringstream &strInstString, long instruction );
	void JALR ( stringstream &strInstString, long instruction );
	void MOVZ ( stringstream &strInstString, long instruction );
	void MOVN ( stringstream &strInstString, long instruction );
	void SYSCALL ( stringstream &strInstString, long instruction );
	void BREAK ( stringstream &strInstString, long instruction );
	void SYNC ( stringstream &strInstString, long instruction );
	void MFHI ( stringstream &strInstString, long instruction );
	void MTHI ( stringstream &strInstString, long instruction );
	void MFLO ( stringstream &strInstString, long instruction );
	void MTLO ( stringstream &strInstString, long instruction );
	void DSLLV ( stringstream &strInstString, long instruction );
	void DSRLV ( stringstream &strInstString, long instruction );
	void DSRAV ( stringstream &strInstString, long instruction );
	void MULT ( stringstream &strInstString, long instruction );
	void MULTU ( stringstream &strInstString, long instruction );
	void DIV ( stringstream &strInstString, long instruction );
	void DIVU ( stringstream &strInstString, long instruction );
	void ADD ( stringstream &strInstString, long instruction );
	void ADDU ( stringstream &strInstString, long instruction );
	void SUB ( stringstream &strInstString, long instruction );
	void SUBU ( stringstream &strInstString, long instruction );
	void AND ( stringstream &strInstString, long instruction );
	void OR ( stringstream &strInstString, long instruction );
	void XOR ( stringstream &strInstString, long instruction );
	void NOR ( stringstream &strInstString, long instruction );
	void MFSA ( stringstream &strInstString, long instruction );
	void MTSA ( stringstream &strInstString, long instruction );
	void SLT ( stringstream &strInstString, long instruction );
	void SLTU ( stringstream &strInstString, long instruction );
	void DADD ( stringstream &strInstString, long instruction );
	void DADDU ( stringstream &strInstString, long instruction );
	void DSUB ( stringstream &strInstString, long instruction );
	void DSUBU ( stringstream &strInstString, long instruction );
	void TGE ( stringstream &strInstString, long instruction );
	void TGEU ( stringstream &strInstString, long instruction );
	void TLT ( stringstream &strInstString, long instruction );
	void TLTU ( stringstream &strInstString, long instruction );
	void TEQ ( stringstream &strInstString, long instruction );
	void TNE ( stringstream &strInstString, long instruction );
	void DSLL ( stringstream &strInstString, long instruction );
	void DSRL ( stringstream &strInstString, long instruction );
	void DSRA ( stringstream &strInstString, long instruction );
	void DSLL32 ( stringstream &strInstString, long instruction );
	void DSRL32 ( stringstream &strInstString, long instruction );
	void DSRA32 ( stringstream &strInstString, long instruction );
	void MADD ( stringstream &strInstString, long instruction );
	void MADDU ( stringstream &strInstString, long instruction );
	void PLZCW ( stringstream &strInstString, long instruction );
	void MMI0 ( stringstream &strInstString, long instruction );
	void MMI2 ( stringstream &strInstString, long instruction );
	void MFHI1 ( stringstream &strInstString, long instruction );
	void MTHI1MTLO1 ( stringstream &strInstString, long instruction );
	void MULT1 ( stringstream &strInstString, long instruction );
	void MULTU1 ( stringstream &strInstString, long instruction );
	void DIV1 ( stringstream &strInstString, long instruction );
	void DIVU1 ( stringstream &strInstString, long instruction );
	void MFLO1 ( stringstream &strInstString, long instruction );
	void MADD1 ( stringstream &strInstString, long instruction );
	void MADDU1 ( stringstream &strInstString, long instruction );
	void MMI1 ( stringstream &strInstString, long instruction );
	void MMI3 ( stringstream &strInstString, long instruction );
	void PMFHLFMT ( stringstream &strInstString, long instruction );
	void PMTHL ( stringstream &strInstString, long instruction );
	void PSLLH ( stringstream &strInstString, long instruction );
	void PSRLH ( stringstream &strInstString, long instruction );
	void PSRAH ( stringstream &strInstString, long instruction );
	void PSLLW ( stringstream &strInstString, long instruction );
	void PSRLW ( stringstream &strInstString, long instruction );
	void PSRAW ( stringstream &strInstString, long instruction );
	void USECOP1B1CODE ( stringstream &strInstString, long instruction );
	void TLBR ( stringstream &strInstString, long instruction );
	void TLBWI ( stringstream &strInstString, long instruction );
	void TLBWR ( stringstream &strInstString, long instruction );
	void TLBP ( stringstream &strInstString, long instruction );
	void ERET ( stringstream &strInstString, long instruction );
	void DERET ( stringstream &strInstString, long instruction );
	void WAIT ( stringstream &strInstString, long instruction );
	void MFC2 ( stringstream &strInstString, long instruction );
	void CFC2 ( stringstream &strInstString, long instruction );
	void MTC2 ( stringstream &strInstString, long instruction );
	void Cop2BC ( stringstream &strInstString, long instruction );
	void MFC1 ( stringstream &strInstString, long instruction );
	void CFC1 ( stringstream &strInstString, long instruction );
	void MTC1 ( stringstream &strInstString, long instruction );
	void CTC1 ( stringstream &strInstString, long instruction );
	void Cop1BC ( stringstream &strInstString, long instruction );
	void S ( stringstream &strInstString, long instruction );
	void CVTSFMTW ( stringstream &strInstString, long instruction );
	void ADDFMTS ( stringstream &strInstString, long instruction );
	void SUBFMTS ( stringstream &strInstString, long instruction );
	void MULFMTS ( stringstream &strInstString, long instruction );
	void DIVFMTS ( stringstream &strInstString, long instruction );
	void SQRTFMTS ( stringstream &strInstString, long instruction );
	void ABSFMTS ( stringstream &strInstString, long instruction );
	void MOVFMTS ( stringstream &strInstString, long instruction );
	void NEGFMTS ( stringstream &strInstString, long instruction );
	void RSQRTFMTS ( stringstream &strInstString, long instruction );
	void ADDAFMTS ( stringstream &strInstString, long instruction );
	void SUBAFMTS ( stringstream &strInstString, long instruction );
	void MADDFMTS ( stringstream &strInstString, long instruction );
	void MSUBFMTS ( stringstream &strInstString, long instruction );
	void MADDAFMTS ( stringstream &strInstString, long instruction );
	void CVTWFMTS ( stringstream &strInstString, long instruction );
	void MAXFMTS ( stringstream &strInstString, long instruction );
	void MINFMTS ( stringstream &strInstString, long instruction );
	void CFFMTS ( stringstream &strInstString, long instruction );
	void CEQFMTS ( stringstream &strInstString, long instruction );
	void CLTFMTS ( stringstream &strInstString, long instruction );
	void CLEFMTS ( stringstream &strInstString, long instruction );
	void PADDW ( stringstream &strInstString, long instruction );
	void PSUBW ( stringstream &strInstString, long instruction );
	void PCGTW ( stringstream &strInstString, long instruction );
	void PMAXW ( stringstream &strInstString, long instruction );
	void PADDH ( stringstream &strInstString, long instruction );
	void PSUBH ( stringstream &strInstString, long instruction );
	void PCGTH ( stringstream &strInstString, long instruction );
	void PMAXH ( stringstream &strInstString, long instruction );
	void PADDB ( stringstream &strInstString, long instruction );
	void PSUBB ( stringstream &strInstString, long instruction );
	void PCGTB ( stringstream &strInstString, long instruction );
	void PADDSW ( stringstream &strInstString, long instruction );
	void PSUBSW ( stringstream &strInstString, long instruction );
	void PEXTLW ( stringstream &strInstString, long instruction );
	void PPACW ( stringstream &strInstString, long instruction );
	void PADDSH ( stringstream &strInstString, long instruction );
	void PSUBSH ( stringstream &strInstString, long instruction );
	void PEXTLH ( stringstream &strInstString, long instruction );
	void PPACH ( stringstream &strInstString, long instruction );
	void PADDSB ( stringstream &strInstString, long instruction );
	void PSUBSB ( stringstream &strInstString, long instruction );
	void PEXTLB ( stringstream &strInstString, long instruction );
	void PPACB ( stringstream &strInstString, long instruction );
	void PEXT5 ( stringstream &strInstString, long instruction );
	void PPAC5 ( stringstream &strInstString, long instruction );
	void PABSW ( stringstream &strInstString, long instruction );
	void PCEQW ( stringstream &strInstString, long instruction );
	void PMINW ( stringstream &strInstString, long instruction );
	void PADSBH ( stringstream &strInstString, long instruction );
	void PABSH ( stringstream &strInstString, long instruction );
	void PCEQH ( stringstream &strInstString, long instruction );
	void PMINH ( stringstream &strInstString, long instruction );
	void PCEQB ( stringstream &strInstString, long instruction );
	void PADDUW ( stringstream &strInstString, long instruction );
	void PSUBUW ( stringstream &strInstString, long instruction );
	void PEXTUW ( stringstream &strInstString, long instruction );
	void PADDUH ( stringstream &strInstString, long instruction );
	void PSUBUH ( stringstream &strInstString, long instruction );
	void PEXTUH ( stringstream &strInstString, long instruction );
	void PADDUB ( stringstream &strInstString, long instruction );
	void PSUBUB ( stringstream &strInstString, long instruction );
	void PEXTUB ( stringstream &strInstString, long instruction );
	void QFSRV ( stringstream &strInstString, long instruction );
	void PMADDW ( stringstream &strInstString, long instruction );
	void PSLLVW ( stringstream &strInstString, long instruction );
	void PSRLVW ( stringstream &strInstString, long instruction );
	void PMSUBW ( stringstream &strInstString, long instruction );
	void PMFHI ( stringstream &strInstString, long instruction );
	void PMFLO ( stringstream &strInstString, long instruction );
	void PINTH ( stringstream &strInstString, long instruction );
	void PMULTW ( stringstream &strInstString, long instruction );
	void PDIVW ( stringstream &strInstString, long instruction );
	void PCPYLD ( stringstream &strInstString, long instruction );
	void PMADDH ( stringstream &strInstString, long instruction );
	void PHMADH ( stringstream &strInstString, long instruction );
	void PAND ( stringstream &strInstString, long instruction );
	void PXOR ( stringstream &strInstString, long instruction );
	void PMSUBH ( stringstream &strInstString, long instruction );
	void PHMSBH ( stringstream &strInstString, long instruction );
	void PEXEH ( stringstream &strInstString, long instruction );
	void PREVH ( stringstream &strInstString, long instruction );
	void PMULTH ( stringstream &strInstString, long instruction );
	void PDIVBW ( stringstream &strInstString, long instruction );
	void PEXEW ( stringstream &strInstString, long instruction );
	void PROT3W ( stringstream &strInstString, long instruction );
	void PMADDUW ( stringstream &strInstString, long instruction );
	void PSRAVW ( stringstream &strInstString, long instruction );
	void PMTHI ( stringstream &strInstString, long instruction );
	void PMTLO ( stringstream &strInstString, long instruction );
	void PINTEH ( stringstream &strInstString, long instruction );
	void PMULTUW ( stringstream &strInstString, long instruction );
	void PDIVUW ( stringstream &strInstString, long instruction );
	void PCPYUD ( stringstream &strInstString, long instruction );
	void POR ( stringstream &strInstString, long instruction );
	void PNOR ( stringstream &strInstString, long instruction );
	void PEXCH ( stringstream &strInstString, long instruction );
	void PCPYH ( stringstream &strInstString, long instruction );
	void PEXCW ( stringstream &strInstString, long instruction );
	void QMFC2 ( stringstream &strInstString, long instruction );
	void QMTC2 ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VU ( stringstream &strInstString, long instruction );
	void VADDBCX ( stringstream &strInstString, long instruction );
	void VADDBCY ( stringstream &strInstString, long instruction );
	void VADDBCZ ( stringstream &strInstString, long instruction );
	void VADDBCW ( stringstream &strInstString, long instruction );
	void VSUBBCX ( stringstream &strInstString, long instruction );
	void VSUBBCY ( stringstream &strInstString, long instruction );
	void VSUBBCZ ( stringstream &strInstString, long instruction );
	void VSUBBCW ( stringstream &strInstString, long instruction );
	void VMADDBCX ( stringstream &strInstString, long instruction );
	void VMADDBCY ( stringstream &strInstString, long instruction );
	void VMADDBCZ ( stringstream &strInstString, long instruction );
	void VMADDBCW ( stringstream &strInstString, long instruction );
	void VMSUBBCX ( stringstream &strInstString, long instruction );
	void VMSUBBCY ( stringstream &strInstString, long instruction );
	void VMSUBBCZ ( stringstream &strInstString, long instruction );
	void VMSUBBCW ( stringstream &strInstString, long instruction );
	void VMAXBCX ( stringstream &strInstString, long instruction );
	void VMAXBCY ( stringstream &strInstString, long instruction );
	void VMAXBCZ ( stringstream &strInstString, long instruction );
	void VMAXBCW ( stringstream &strInstString, long instruction );
	void VMINIBCX ( stringstream &strInstString, long instruction );
	void VMINIBCY ( stringstream &strInstString, long instruction );
	void VMINIBCZ ( stringstream &strInstString, long instruction );
	void VMINIBCW ( stringstream &strInstString, long instruction );
	void VMULBCX ( stringstream &strInstString, long instruction );
	void VMULBCY ( stringstream &strInstString, long instruction );
	void VMULBCZ ( stringstream &strInstString, long instruction );
	void VMULBCW ( stringstream &strInstString, long instruction );
	void VMULQ ( stringstream &strInstString, long instruction );
	void VMAXI ( stringstream &strInstString, long instruction );
	void VMULI ( stringstream &strInstString, long instruction );
	void VMINII ( stringstream &strInstString, long instruction );
	void VADDQ ( stringstream &strInstString, long instruction );
	void VMADDQ ( stringstream &strInstString, long instruction );
	void VADDI ( stringstream &strInstString, long instruction );
	void VMADDI ( stringstream &strInstString, long instruction );
	void VSUBQ ( stringstream &strInstString, long instruction );
	void VMSUBQ ( stringstream &strInstString, long instruction );
	void VSUBI ( stringstream &strInstString, long instruction );
	void VMSUBI ( stringstream &strInstString, long instruction );
	void VADD ( stringstream &strInstString, long instruction );
	void VMADD ( stringstream &strInstString, long instruction );
	void VMUL ( stringstream &strInstString, long instruction );
	void VMAX ( stringstream &strInstString, long instruction );
	void VSUB ( stringstream &strInstString, long instruction );
	void VMSUB ( stringstream &strInstString, long instruction );
	void VOPMSUM ( stringstream &strInstString, long instruction );
	void VMINI ( stringstream &strInstString, long instruction );
	void VIADD ( stringstream &strInstString, long instruction );
	void VISUB ( stringstream &strInstString, long instruction );
	void VIADDI ( stringstream &strInstString, long instruction );
	void VIAND ( stringstream &strInstString, long instruction );
	void VIOR ( stringstream &strInstString, long instruction );
	void VCALLMS ( stringstream &strInstString, long instruction );
	void VCALLMSR ( stringstream &strInstString, long instruction );
	void VU60 ( stringstream &strInstString, long instruction );
	void VU61 ( stringstream &strInstString, long instruction );
	void VU62 ( stringstream &strInstString, long instruction );
	void VU63 ( stringstream &strInstString, long instruction );
	void VITOF0 ( stringstream &strInstString, long instruction );
	void VFTOI0 ( stringstream &strInstString, long instruction );
	void VMULAQ ( stringstream &strInstString, long instruction );
	void VADDAQ ( stringstream &strInstString, long instruction );
	void VSUBAQ ( stringstream &strInstString, long instruction );
	void VADDA ( stringstream &strInstString, long instruction );
	void VSUBA ( stringstream &strInstString, long instruction );
	void VMOVE ( stringstream &strInstString, long instruction );
	void VLQI ( stringstream &strInstString, long instruction );
	void VDIV ( stringstream &strInstString, long instruction );
	void VMTIR ( stringstream &strInstString, long instruction );
	void VRNEXT ( stringstream &strInstString, long instruction );
	void VITOF4 ( stringstream &strInstString, long instruction );
	void VFTOI4 ( stringstream &strInstString, long instruction );
	void VABS ( stringstream &strInstString, long instruction );
	void VMADDAQ ( stringstream &strInstString, long instruction );
	void VMSUBAQ ( stringstream &strInstString, long instruction );
	void VMADDA ( stringstream &strInstString, long instruction );
	void VMSUBA ( stringstream &strInstString, long instruction );
	void VMR32 ( stringstream &strInstString, long instruction );
	void VSQI ( stringstream &strInstString, long instruction );
	void VSQRT ( stringstream &strInstString, long instruction );
	void VMFIR ( stringstream &strInstString, long instruction );
	void VRGET ( stringstream &strInstString, long instruction );
	void VADDABC ( stringstream &strInstString, long instruction );
	void VSUBABC ( stringstream &strInstString, long instruction );
	void VMADDABC ( stringstream &strInstString, long instruction );
	void VMSUBABC ( stringstream &strInstString, long instruction );
	void VITOF12 ( stringstream &strInstString, long instruction );
	void VFTOI12 ( stringstream &strInstString, long instruction );
	void VMULABC ( stringstream &strInstString, long instruction );
	void VMULAI ( stringstream &strInstString, long instruction );
	void VADDAI ( stringstream &strInstString, long instruction );
	void VSUBAI ( stringstream &strInstString, long instruction );
	void VMULA ( stringstream &strInstString, long instruction );
	void VOPMULA ( stringstream &strInstString, long instruction );
	void VLQD ( stringstream &strInstString, long instruction );
	void VRSQRT ( stringstream &strInstString, long instruction );
	void VILWR ( stringstream &strInstString, long instruction );
	void VRINIT ( stringstream &strInstString, long instruction );
	void VITOF15 ( stringstream &strInstString, long instruction );
	void VFTOI15 ( stringstream &strInstString, long instruction );
	void VCLIP ( stringstream &strInstString, long instruction );
	void VMADDAI ( stringstream &strInstString, long instruction );
	void VMSUBAI ( stringstream &strInstString, long instruction );
	void VNOP ( stringstream &strInstString, long instruction );
	void VSQD ( stringstream &strInstString, long instruction );
	void VWAITQ ( stringstream &strInstString, long instruction );
	void VISWR ( stringstream &strInstString, long instruction );
	void VRXOR ( stringstream &strInstString, long instruction );

	const char XyzwLUT [ 4 ] = { 'x', 'y', 'z', 'w' };

	void AddVuDestArgs ( stringstream &strVuArgs, long Instruction );

	void AddInstArgs ( stringstream &strMipsArgs, long Instruction, long InstFormat );

	// to decode instruction, just look up the opcode here first, then call the function in the array which either formulates x64 code or looks up in other arrays
	// array complete
	const Function AllOpcodes [ 64 ] = { Special, Regimm, J, JAL, BEQ, BNE, BLEZ, BGTZ,	// 0-7
										ADDI, ADDIU, SLTI, SLTIU, ANDI, ORI, XORI, LUI,		// 8-15
										COP0, COP1, VU, None, BEQL, BNEL, BLEZL, BGTZL,			// 16-23
										DADDI, DADDIU, LDL, LDR, MMI, None, LQ, SQ,	// 24-31
										LB, LH, LWL, LW, LBU, LHU, LWR, LWU,	// 32-39
										SB, SH, SWL, SW, SDL, SDR, SWR, CACHE,	// 40-47
										None, LWC1, None, PREF, None, None, LQC2, LD,	//48-55
										None, SWC1, None, None, None, None, SQC2, SD };	//56-63
													
													
	// this is where you look up "REGIMM" codes in B2 slot
	// array complete
	const Function AllRegimmB2Codes [ 32 ] = { BLTZ, BGEZ, BLTZL, None, None, None, None, None, //0-7
													TGEI, TGEIU, TLTI, TLTIU, TEQI, None, TNEI, None, //8-15
													BLTZAL, BGEZAL, BLTZALL, BGEZALL, None, None, None, None, //16-23
													None, None, None, None, None, None, None, None }; //24-31

													

	// this one has only Playstation 2 special codes
	const Function AllPS2SpecialInsts [ 64 ] = {	SLL, MOVCI, SRL, SRA, SLLV, None, SRLV, SRAV, //0-7
													JR, JALR, MOVZ, MOVN, SYSCALL, BREAK, None, SYNC, //8-15
													MFHI, MTHI, MFLO, MTLO, DSLLV, None, DSRLV, DSRAV, //16-23
													MULT, MULTU, DIV, DIVU, None, None, None, None, //24-31
													ADD, ADDU, SUB, SUBU, AND, OR, XOR, NOR, //32-39
													MFSA, MTSA, SLT, SLTU, DADD, DADDU, DSUB, DSUBU, //40-47
													TGE, TGEU, TLT, TLTU, TEQ, None, TNE, None, //48-55
													DSLL, None, DSRL, DSRA, DSLL32, None, DSRL32, DSRA32 }; //56-63



	// These are for MMI instructions - these are found in the "Special" code in the 6 lowest of the instruction
	const Function AllMMIInsts [ 64 ] = { MADD, MADDU, None, None, PLZCW, None, None, None, //0-7
													MMI0, MMI2, None, None, None, None, None, None, //8-15
													MFHI1, MTHI1MTLO1, None, None, None, None, None, None, //16-23
													MULT1, MULTU1, DIV1, DIVU1, MFLO1, None, None, None, //24-31
													MADD1, MADDU1, None, None, None, None, None, None, //32-39
													MMI1, MMI3, None, None, None, None, None, None, //40-47
													PMFHLFMT, PMTHL, None, None, PSLLH, None, PSRLH, PSRAH, //48-55
													None, None, None, None, PSLLW, None, PSRLW, PSRAW }; //56-63




	// this is where you look up "COP0" instructions - lookup using "Special" slot
	// array complete
	const Function AllCop0Insts [ 64 ] = { None, TLBR, TLBWI, None, None, None, TLBWR, None, //0-7
													TLBP, None, None, None, None, None, None, None, //8-15
													None, None, None, None, None, None, None, None, //16-23
													ERET, None, None, None, None, None, None, None, //24-31
													None, None, None, None, None, None, None, None, //32-39
													None, None, None, None, None, None, None, None, //40-47
													None, None, None, None, None, None, None, None, //48-55
													None, None, None, None, None, None, None, None }; //56-63
													
	const Function AllCop0FmtCodes [ 32 ] = { MF0, None, None, None, MT0, None, None, None, //0-7
													BC0, None, None, None, None, None, None, None, //8-15
													CO, None, None, None, None, None, None, None, //16-23
													None, None, None, None, None, None, None, None }; //24-31

	const Function AllC0Insts [ 64 ] = { None, TLBR, TLBWI, None, None, None, TLBWR, None, //0-7
													TLBP, None, None, None, None, None, None, None, //8-15
													None, None, None, None, None, None, None, None, //16-23
													ERET, None, None, None, None, None, None, None, //24-31
													None, None, None, None, None, None, None, None, //32-39
													None, None, None, None, None, None, None, None, //40-47
													None, None, None, None, None, None, None, None, //48-55
													EI, DI, None, None, None, None, None, None }; //56-63

													

	// this one has just the Playstation 2 COP2 Fmt codes
	//const Function AllDebugCop2FmtCodes [ 32 ] = { MFC2, None, CFC2, None, MTC2, None, None, None, //0-7
	//												Cop2BC, None, None, None, None, None, None, None, //8-15
	//												None, None, None, None, None, None, None, None, //16-23
	//												None, None, None, None, None, None, None, None }; //24-31



	// this is one with just PS2 COP1 Fmt codes
	const Function AllPS2Cop1FmtCodes [ 32 ] = { MFC1, None, CFC1, None, MTC1, None, CTC1, None, //0-7
													BC1, None, None, None, None, None, None, None, //8-15
													S, None, None, None, CVTSFMTW, None, None, None, //16-23
													None, None, None, None, None, None, None, None}; //24-31




	// this is the table with just the PS2 codes
	const Function AllPS2Cop1SInsts [ 64 ] = { ADDFMTS, SUBFMTS, MULFMTS, DIVFMTS, SQRTFMTS, ABSFMTS, MOVFMTS, NEGFMTS, //0-7
													None, None, None, None, None, None, None, None, //8-15
													None, None, None, None, None, None, RSQRTFMTS, None, //16-23
													ADDAFMTS, SUBAFMTS, None, None, MADDFMTS, MSUBFMTS, MADDAFMTS, None, //24-31
													None, None, None, None, CVTWFMTS, None, None, None, //32-39
													MAXFMTS, MINFMTS, None, None, None, None, None, None, //40-47
													CFFMTS, None, CEQFMTS, None, CLTFMTS, None, CLEFMTS, None, //48-55
													None, None, None, None, None, None, None, None }; //56-63

													
													

													
	// **** LUTs for MMI Special 2 codes ****

	const Function AllMMI0B4Codes [ 32 ] = { PADDW, PSUBW, PCGTW, PMAXW, PADDH, PSUBH, PCGTH, PMAXH, //0-7
													PADDB, PSUBB, PCGTB, None, None, None, None, None, //8-15
													PADDSW, PSUBSW, PEXTLW, PPACW, PADDSH, PSUBSH, PEXTLH, PPACH, //16-23
													PADDSB, PSUBSB, PEXTLB, PPACB, None, None, PEXT5, PPAC5 }; //24-31

	const Function AllMMI1B4Codes [ 32 ] = { None, PABSW, PCEQW, PMINW, PADSBH, PABSH, PCEQH, PMINH, //0-7
													None, None, PCEQB, None, None, None, None, None, //8-15
													PADDUW, PSUBUW, PEXTUW, None, PADDUH, PSUBUH, PEXTUH, None, //16-23
													PADDUB, PSUBUB, PEXTUB, QFSRV, None, None, None, None }; //24-31

	const Function AllMMI2B4Codes [ 32 ] = { PMADDW, None, PSLLVW, PSRLVW, PMSUBW, None, None, None, //0-7
													PMFHI, PMFLO, PINTH, None, PMULTW, PDIVW, PCPYLD, None, //8-15
													PMADDH, PHMADH, PAND, PXOR, PMSUBH, PHMSBH, None, None, //16-23
													None, None, PEXEH, PREVH, PMULTH, PDIVBW, PEXEW, PROT3W }; //24-31

	const Function AllMMI3B4Codes [ 32 ] = { PMADDUW, None, None, PSRAVW, None, None, None, None, //0-7
													PMTHI, PMTLO, PINTEH, None, PMULTUW, PDIVUW, PCPYUD, None, //8-15
													None, None, POR, PNOR, None, None, None, None, //16-23
													None, None, PEXCH, PCPYH, None, None, PEXCW, None }; //24-31

	// LUTs for VU Macro mode instructions
													
	// For Playstation 2 COP2 instructions, look up this B1 (the 5 bits after the opcode) code first
	const Function AllVU0B1Codes [ 32 ] = { None, QMFC2, None, None, None, QMTC2, None, None, //0-7
													BC2, None, None, None, None, None, None, None, //8-15
													VUSpecial, VUSpecial, VUSpecial, VUSpecial, VUSpecial, VUSpecial, VUSpecial, VUSpecial, //16-23
													VUSpecial, VUSpecial, VUSpecial, VUSpecial, VUSpecial, VUSpecial, VUSpecial, VUSpecial }; //24-31

	// then look up this "Special" (the lowest 6 bits of the instruction) code if you get "VU"
	const Function AllVU0SpecialCodes [ 64 ] = { VADDBCX, VADDBCY, VADDBCZ, VADDBCW, VSUBBCX, VSUBBCY, VSUBBCZ, VSUBBCW, //0-7
													VMADDBCX, VMADDBCY, VMADDBCZ, VMADDBCW, VMSUBBCX, VMSUBBCY, VMSUBBCZ, VMSUBBCW, //8-15
													VMAXBCX, VMAXBCY, VMAXBCZ, VMAXBCW, VMINIBCX, VMINIBCY, VMINIBCZ, VMINIBCW, //16-23
													VMULBCX, VMULBCY, VMULBCZ, VMULBCW, VMULQ, VMAXI, VMULI, VMINII, //24-31
													VADDQ, VMADDQ, VADDI, VMADDI, VSUBQ, VMSUBQ, VSUBI, VMSUBI, //32-39
													VADD, VMADD, VMUL, VMAX, VSUB, VMSUB, VOPMSUB, VMINI, //40-47
													VIADD, VISUB, VIADDI, None, VIAND, VIOR, None, None, //48-55
													VCALLMS, VCALLMSR, None, None, VU60, VU61, VU62, VU63 }; //56-63



	// then look up these B4 (the 5 bits to the left of the "Special" code) codes if you get VU60, VU61, VU62, or VU63												
	const Function AllVU60B4Codes [ 32 ] = { None, None, None, None, VITOF0, VFTOI0, None, VMULAQ, //0-7
													VADDAQ, VSUBAQ, VADDA, VSUBA, VMOVE, VLQI, VDIV, VMTIR, //8-15
													VRNEXT, None, None, None, None, None, None, None, //16-23
													None, None, None, None, None, None, None, None }; //24-31

	const Function AllVU61B4Codes [ 32 ] = { None, None, None, None, VITOF4, VFTOI4, None, VABS, //0-7
													VMADDAQ, VMSUBAQ, VMADDA, VMSUBA, VMR32, VSQI, VSQRT, VMFIR, //8-15
													VRGET, None, None, None, None, None, None, None, //16-23
													None, None, None, None, None, None, None, None }; //24-31

	const Function AllVU62B4Codes [ 32 ] = { VADDABC, VSUBABC, VMADDABC, VMSUBABC, VITOF12, VFTOI12, VMULABC, VMULAI, //0-7
													VADDAI, VSUBAI, VMULA, VOPMULA, None, VLQD, VRSQRT, VILWR, //8-15
													VRINIT, None, None, None, None, None, None, None, //16-23
													None, None, None, None, None, None, None, None }; //24-31

	const Function AllVU63B4Codes [ 32 ] = { None, None, None, None, VITOF15, VFTOI15, None, VCLIP, //0-7
													VMADDAI, VMSUBAI, None, VNOP, None, VSQD, VWAITQ, VISWR, //8-15
													VRXOR, None, None, None, None, None, None, None, //16-23
													None, None, None, None, None, None, None, None }; //24-31


}


void R5900DebugPrint::PrintInstruction ( stringstream &strInstString, long instruction )
{
	AllOpcodes [ GET_OPCODE( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::Special ( stringstream &strInstString, long instruction )
{
	AllPS2SpecialInsts [ GET_SPECIAL( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::Regimm ( stringstream &strInstString, long instruction )
{
	AllRegimmB2Codes [ GET_B2 ( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::COP0 ( stringstream &strInstString, long instruction )
{
	AllCop0FmtCodes [ GET_FMT( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::COP1 ( stringstream &strInstString, long instruction )
{
	AllPS2Cop1FmtCodes [ GET_FMT( instruction ) ] ( strInstString, instruction );
}

//void R5900DebugPrint::COP2 ( stringstream &strInstString, long instruction )
//{
//	AllDebugCop2FmtCodes [ GET_FMT( instruction ) ] ( strInstString, instruction );
//}

void R5900DebugPrint::S ( stringstream &strInstString, long instruction )
{
	AllPS2Cop1SInsts [ GET_SPECIAL ( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::CO ( stringstream &strInstString, long instruction )
{
	AllC0Insts [ GET_SPECIAL( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::MMI ( stringstream &strInstString, long instruction )
{
	AllMMIInsts [ GET_SPECIAL( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::MMI0 ( stringstream &strInstString, long instruction )
{
	AllMMI0B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::MMI2 ( stringstream &strInstString, long instruction )
{
	AllMMI2B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::MMI1 ( stringstream &strInstString, long instruction )
{
	AllMMI1B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::MMI3 ( stringstream &strInstString, long instruction )
{
	AllMMI3B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::VU ( stringstream &strInstString, long instruction )
{
	AllVU0B1Codes [ GET_B1( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::VUSpecial ( stringstream &strInstString, long instruction )
{
	AllVU0SpecialCodes [ GET_SPECIAL( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::VU60 ( stringstream &strInstString, long instruction )
{
	AllVU60B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::VU61 ( stringstream &strInstString, long instruction )
{
	AllVU61B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::VU62 ( stringstream &strInstString, long instruction )
{
	AllVU62B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}

void R5900DebugPrint::VU63 ( stringstream &strInstString, long instruction )
{
	AllVU63B4Codes [ GET_B4( instruction ) ] ( strInstString, instruction );
}


void R5900DebugPrint::BC0 ( stringstream &strInstString, long instruction )
{
	strInstString << "BC0" << BCType [ GET_B2( instruction ) & 3 ];
	AddInstArgs ( strInstString, instruction, FORMAT23 );
	strInstString << "    " << GET_B2( instruction );
}

void R5900DebugPrint::BC1 ( stringstream &strInstString, long instruction )
{
	strInstString << "BC1" << BCType [ GET_B2( instruction ) & 3 ];
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}

void R5900DebugPrint::BC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "BC2" << BCType [ GET_B2( instruction ) & 3 ];
	AddInstArgs ( strInstString, instruction, FORMAT23 );
}


void R5900DebugPrint::MF0 ( stringstream &strInstString, long instruction )
{
	strInstString << "MFC0";	// mind as well make it MFC since debug instructions won't be encountered
	AddInstArgs ( strInstString, instruction, FORMAT18 );
}

void R5900DebugPrint::MT0 ( stringstream &strInstString, long instruction )
{
	strInstString << "MTC0";	// mind as well make it MTC since debug instructions won't be encountered
	AddInstArgs ( strInstString, instruction, FORMAT19 );
}

void R5900DebugPrint::EI ( stringstream &strInstString, long instruction )
{
	strInstString << "EI";
}

void R5900DebugPrint::DI ( stringstream &strInstString, long instruction )
{
	strInstString << "DI";
}

void R5900DebugPrint::LWC1 ( stringstream &strInstString, long instruction )
{
	strInstString << "LWC1";
	AddInstArgs ( strInstString, instruction, FORMAT37 );
}

void R5900DebugPrint::SWC1 ( stringstream &strInstString, long instruction )
{
	strInstString << "SWC1";
	AddInstArgs ( strInstString, instruction, FORMAT37 );
}

void R5900DebugPrint::LB ( stringstream &strInstString, long instruction )
{
	strInstString << "LB";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::LBU ( stringstream &strInstString, long instruction )
{
	strInstString << "LBU";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::LDL ( stringstream &strInstString, long instruction )
{
	strInstString << "LDL";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::LDR ( stringstream &strInstString, long instruction )
{
	strInstString << "LDR";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::LH ( stringstream &strInstString, long instruction )
{
	strInstString << "LH";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::LHU ( stringstream &strInstString, long instruction )
{
	strInstString << "LHU";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::LWR ( stringstream &strInstString, long instruction )
{
	strInstString << "LWR";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::LWL ( stringstream &strInstString, long instruction )
{
	strInstString << "LWL";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::LWU ( stringstream &strInstString, long instruction )
{
	strInstString << "LWU";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::SB ( stringstream &strInstString, long instruction )
{
	strInstString << "SB";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::SDL ( stringstream &strInstString, long instruction )
{
	strInstString << "SDL";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::SDR ( stringstream &strInstString, long instruction )
{
	strInstString << "SDR";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::SH ( stringstream &strInstString, long instruction )
{
	strInstString << "SH";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::SWL ( stringstream &strInstString, long instruction )
{
	strInstString << "SWL";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::SWR ( stringstream &strInstString, long instruction )
{
	strInstString << "SWR";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::SD ( stringstream &strInstString, long instruction )
{
	strInstString << "SD";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::LD ( stringstream &strInstString, long instruction )
{
	strInstString << "LD";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::SW ( stringstream &strInstString, long instruction )
{
	strInstString << "SW";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::LW ( stringstream &strInstString, long instruction )
{
	strInstString << "LW";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::LQ ( stringstream &strInstString, long instruction )
{
	strInstString << "LQ";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::SQ ( stringstream &strInstString, long instruction )
{
	strInstString << "SQ";
	AddInstArgs ( strInstString, instruction, FORMAT11 );
}

void R5900DebugPrint::J ( stringstream &strInstString, long instruction )
{
	strInstString << "J";
	AddInstArgs ( strInstString, instruction, FTJ );
}

void R5900DebugPrint::JAL ( stringstream &strInstString, long instruction )
{
	strInstString << "JAL";
	AddInstArgs ( strInstString, instruction, FTJAL );
}

void R5900DebugPrint::BEQ ( stringstream &strInstString, long instruction )
{
	strInstString << "BEQ";
	AddInstArgs ( strInstString, instruction, FTBEQ );
}

void R5900DebugPrint::BNE ( stringstream &strInstString, long instruction )
{
	strInstString << "BNE";
	AddInstArgs ( strInstString, instruction, FTBNE );
}

void R5900DebugPrint::BLEZ ( stringstream &strInstString, long instruction )
{
	strInstString << "BLEZ";
	AddInstArgs ( strInstString, instruction, FTBLEZ );
}

void R5900DebugPrint::BGTZ ( stringstream &strInstString, long instruction )
{
	strInstString << "BGTZ";
	AddInstArgs ( strInstString, instruction, FTBGTZ );
}

void R5900DebugPrint::ADDI ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDI";
	AddInstArgs ( strInstString, instruction, FTADDI );
}

void R5900DebugPrint::ADDIU ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDIU";
	AddInstArgs ( strInstString, instruction, FTADDIU );
}

void R5900DebugPrint::SLTI ( stringstream &strInstString, long instruction )
{
	strInstString << "SLTI";
	AddInstArgs ( strInstString, instruction, FTSLTI );
}

void R5900DebugPrint::SLTIU ( stringstream &strInstString, long instruction )
{
	strInstString << "SLTIU";
	AddInstArgs ( strInstString, instruction, FTSLTIU );
}

void R5900DebugPrint::ANDI ( stringstream &strInstString, long instruction )
{
	strInstString << "ANDI";
	AddInstArgs ( strInstString, instruction, FTANDI );
}

void R5900DebugPrint::ORI ( stringstream &strInstString, long instruction )
{
	strInstString << "ORI";
	AddInstArgs ( strInstString, instruction, FTORI );
}

void R5900DebugPrint::XORI ( stringstream &strInstString, long instruction )
{
	strInstString << "XORI";
	AddInstArgs ( strInstString, instruction, FTXORI );
}

void R5900DebugPrint::LUI ( stringstream &strInstString, long instruction )
{
	strInstString << "LUI";
	AddInstArgs ( strInstString, instruction, FTLUI );
}


void R5900DebugPrint::None ( stringstream &strInstString, long instruction )
{
	strInstString << "Invalid Instruction: Opcode: " << GET_OPCODE( instruction ) << ", Special: " << GET_SPECIAL( instruction ) << ", B1: " << GET_B1( instruction ) << ", B2: " << GET_B2( instruction );
}

void R5900DebugPrint::BEQL ( stringstream &strInstString, long instruction )
{
	strInstString << "BEQL";
	AddInstArgs ( strInstString, instruction, FTBEQL );
}

void R5900DebugPrint::BNEL ( stringstream &strInstString, long instruction )
{
	strInstString << "BNEL";
	AddInstArgs ( strInstString, instruction, FTBNEL );
}

void R5900DebugPrint::BLEZL ( stringstream &strInstString, long instruction )
{
	strInstString << "BLEZL";
	AddInstArgs ( strInstString, instruction, FTBLEZL );
}

void R5900DebugPrint::BGTZL ( stringstream &strInstString, long instruction )
{
	strInstString << "BGTZL";
	AddInstArgs ( strInstString, instruction, FTBGTZL );
}

void R5900DebugPrint::DADDI ( stringstream &strInstString, long instruction )
{
	strInstString << "DADDI";
	AddInstArgs ( strInstString, instruction, FTDADDI );
}

void R5900DebugPrint::DADDIU ( stringstream &strInstString, long instruction )
{
	strInstString << "DADDIU";
	AddInstArgs ( strInstString, instruction, FTDADDIU );
}


void R5900DebugPrint::CACHE ( stringstream &strInstString, long instruction )
{
	strInstString << "CACHE";
	AddInstArgs ( strInstString, instruction, FTCACHE );
}

void R5900DebugPrint::PREF ( stringstream &strInstString, long instruction )
{
	strInstString << "PREF";
	AddInstArgs ( strInstString, instruction, FTPREF );
}

void R5900DebugPrint::LQC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "LQC2";
	AddInstArgs ( strInstString, instruction, FTLQC2 );
}

void R5900DebugPrint::SQC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "SQC2";
	AddInstArgs ( strInstString, instruction, FTSQC2 );
}

void R5900DebugPrint::BLTZ ( stringstream &strInstString, long instruction )
{
	strInstString << "BLTZ";
	AddInstArgs ( strInstString, instruction, FTBLTZ );
}

void R5900DebugPrint::BGEZ ( stringstream &strInstString, long instruction )
{
	strInstString << "BGEZ";
	AddInstArgs ( strInstString, instruction, FTBGEZ );
}

void R5900DebugPrint::BLTZL ( stringstream &strInstString, long instruction )
{
	strInstString << "BLTZL";
	AddInstArgs ( strInstString, instruction, FTBLTZL );
}

void R5900DebugPrint::TGEI ( stringstream &strInstString, long instruction )
{
	strInstString << "TGEI";
	AddInstArgs ( strInstString, instruction, FTTGEI );
}

void R5900DebugPrint::TGEIU ( stringstream &strInstString, long instruction )
{
	strInstString << "TGEIU";
	AddInstArgs ( strInstString, instruction, FTTGEIU );
}

void R5900DebugPrint::TLTI ( stringstream &strInstString, long instruction )
{
	strInstString << "TLTI";
	AddInstArgs ( strInstString, instruction, FTTLTI );
}

void R5900DebugPrint::TLTIU ( stringstream &strInstString, long instruction )
{
	strInstString << "TLTIU";
	AddInstArgs ( strInstString, instruction, FTTLTIU );
}

void R5900DebugPrint::TEQI ( stringstream &strInstString, long instruction )
{
	strInstString << "TEQI";
	AddInstArgs ( strInstString, instruction, FTTEQI );
}

void R5900DebugPrint::TNEI ( stringstream &strInstString, long instruction )
{
	strInstString << "TNEI";
	AddInstArgs ( strInstString, instruction, FTTNEI );
}

void R5900DebugPrint::BLTZAL ( stringstream &strInstString, long instruction )
{
	strInstString << "BLTZAL";
	AddInstArgs ( strInstString, instruction, FTBLTZAL );
}

void R5900DebugPrint::BGEZAL ( stringstream &strInstString, long instruction )
{
	strInstString << "BGEZAL";
	AddInstArgs ( strInstString, instruction, FTBGEZAL );
}

void R5900DebugPrint::BLTZALL ( stringstream &strInstString, long instruction )
{
	strInstString << "BLTZALL";
	AddInstArgs ( strInstString, instruction, FTBLTZALL );
}

void R5900DebugPrint::BGEZALL ( stringstream &strInstString, long instruction )
{
	strInstString << "BGEZALL";
	AddInstArgs ( strInstString, instruction, FTBGEZALL );
}

void R5900DebugPrint::SLL ( stringstream &strInstString, long instruction )
{
	strInstString << "SLL";
	AddInstArgs ( strInstString, instruction, FTSLL );
}

void R5900DebugPrint::MOVCI ( stringstream &strInstString, long instruction )
{
	strInstString << "MOVCI";
//	AddInstArgs ( strInstString, instruction, FTMOVCI );
}

void R5900DebugPrint::SRA ( stringstream &strInstString, long instruction )
{
	strInstString << "SRA";
	AddInstArgs ( strInstString, instruction, FTSRA );
}

void R5900DebugPrint::SRL ( stringstream &strInstString, long instruction )
{
	strInstString << "SRL";
	AddInstArgs ( strInstString, instruction, FTSRL );
}

void R5900DebugPrint::SLLV ( stringstream &strInstString, long instruction )
{
	strInstString << "SLLV";
	AddInstArgs ( strInstString, instruction, FTSLLV );
}

void R5900DebugPrint::SRLV ( stringstream &strInstString, long instruction )
{
	strInstString << "SRLV";
	AddInstArgs ( strInstString, instruction, FTSRLV );
}

void R5900DebugPrint::SRAV ( stringstream &strInstString, long instruction )
{
	strInstString << "SRAV";
	AddInstArgs ( strInstString, instruction, FTSRAV );
}

void R5900DebugPrint::JR ( stringstream &strInstString, long instruction )
{
	strInstString << "JR";
	AddInstArgs ( strInstString, instruction, FTJR );
}

void R5900DebugPrint::JALR ( stringstream &strInstString, long instruction )
{
	strInstString << "JALR";
	AddInstArgs ( strInstString, instruction, FTJALR );
}

void R5900DebugPrint::MOVZ ( stringstream &strInstString, long instruction )
{
	strInstString << "MOVZ";
	AddInstArgs ( strInstString, instruction, FTMOVZ );
}

void R5900DebugPrint::MOVN ( stringstream &strInstString, long instruction )
{
	strInstString << "MOVN";
	AddInstArgs ( strInstString, instruction, FTMOVN );
}

void R5900DebugPrint::SYSCALL ( stringstream &strInstString, long instruction )
{
	strInstString << "SYSCALL";
	AddInstArgs ( strInstString, instruction, FTSYSCALL );
}

void R5900DebugPrint::BREAK ( stringstream &strInstString, long instruction )
{
	strInstString << "BREAK";
	AddInstArgs ( strInstString, instruction, FTBREAK );
}

void R5900DebugPrint::SYNC ( stringstream &strInstString, long instruction )
{
	strInstString << "SYNC";
//	AddInstArgs ( strInstString, instruction, FTSYNC );
}

void R5900DebugPrint::MFHI ( stringstream &strInstString, long instruction )
{
	strInstString << "MFHI";
	AddInstArgs ( strInstString, instruction, FTMFHI );
}

void R5900DebugPrint::MTHI ( stringstream &strInstString, long instruction )
{
	strInstString << "MTHI";
	AddInstArgs ( strInstString, instruction, FTMTHI );
}

void R5900DebugPrint::MFLO ( stringstream &strInstString, long instruction )
{
	strInstString << "MFLO";
	AddInstArgs ( strInstString, instruction, FTMFLO );
}

void R5900DebugPrint::MTLO ( stringstream &strInstString, long instruction )
{
	strInstString << "MTLO";
	AddInstArgs ( strInstString, instruction, FTMTLO );
}

void R5900DebugPrint::DSLLV ( stringstream &strInstString, long instruction )
{
	strInstString << "DSLLV";
	AddInstArgs ( strInstString, instruction, FTDSLLV );
}

void R5900DebugPrint::DSRLV ( stringstream &strInstString, long instruction )
{
	strInstString << "DSRLV";
	AddInstArgs ( strInstString, instruction, FTDSRLV );
}

void R5900DebugPrint::DSRAV ( stringstream &strInstString, long instruction )
{
	strInstString << "DSRAV";
	AddInstArgs ( strInstString, instruction, FTDSRAV );
}

void R5900DebugPrint::MULT ( stringstream &strInstString, long instruction )
{
	strInstString << "MULT";
	AddInstArgs ( strInstString, instruction, FTMULT );
}

void R5900DebugPrint::MULTU ( stringstream &strInstString, long instruction )
{
	strInstString << "MULTU";
	AddInstArgs ( strInstString, instruction, FTMULTU );
}

void R5900DebugPrint::DIV ( stringstream &strInstString, long instruction )
{
	strInstString << "DIV";
	AddInstArgs ( strInstString, instruction, FTDIV );
}

void R5900DebugPrint::DIVU ( stringstream &strInstString, long instruction )
{
	strInstString << "DIVU";
	AddInstArgs ( strInstString, instruction, FTDIVU );
}

void R5900DebugPrint::ADD ( stringstream &strInstString, long instruction )
{
	strInstString << "ADD";
	AddInstArgs ( strInstString, instruction, FTADD );
}

void R5900DebugPrint::ADDU ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDU";
	AddInstArgs ( strInstString, instruction, FTADDU );
}

void R5900DebugPrint::SUB ( stringstream &strInstString, long instruction )
{
	strInstString << "SUB";
	AddInstArgs ( strInstString, instruction, FTSUB );
}

void R5900DebugPrint::SUBU ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBU";
	AddInstArgs ( strInstString, instruction, FTSUBU );
}

void R5900DebugPrint::AND ( stringstream &strInstString, long instruction )
{
	strInstString << "AND";
	AddInstArgs ( strInstString, instruction, FTAND );
}

void R5900DebugPrint::OR ( stringstream &strInstString, long instruction )
{
	strInstString << "OR";
	AddInstArgs ( strInstString, instruction, FTOR );
}

void R5900DebugPrint::XOR ( stringstream &strInstString, long instruction )
{
	strInstString << "XOR";
	AddInstArgs ( strInstString, instruction, FTXOR );
}

void R5900DebugPrint::NOR ( stringstream &strInstString, long instruction )
{
	strInstString << "NOR";
	AddInstArgs ( strInstString, instruction, FTNOR );
}

void R5900DebugPrint::MFSA ( stringstream &strInstString, long instruction )
{
	strInstString << "MFSA";
	AddInstArgs ( strInstString, instruction, FTMFSA );
}

void R5900DebugPrint::MTSA ( stringstream &strInstString, long instruction )
{
	strInstString << "MTSA";
	AddInstArgs ( strInstString, instruction, FTMTSA );
}

void R5900DebugPrint::SLT ( stringstream &strInstString, long instruction )
{
	strInstString << "SLT";
	AddInstArgs ( strInstString, instruction, FTSLT );
}

void R5900DebugPrint::SLTU ( stringstream &strInstString, long instruction )
{
	strInstString << "SLTU";
	AddInstArgs ( strInstString, instruction, FTSLTU );
}

void R5900DebugPrint::DADD ( stringstream &strInstString, long instruction )
{
	strInstString << "DADD";
	AddInstArgs ( strInstString, instruction, FTDADD );
}

void R5900DebugPrint::DADDU ( stringstream &strInstString, long instruction )
{
	strInstString << "DADDU";
	AddInstArgs ( strInstString, instruction, FTDADDU );
}

void R5900DebugPrint::DSUB ( stringstream &strInstString, long instruction )
{
	strInstString << "DSUB";
	AddInstArgs ( strInstString, instruction, FTDSUB );
}

void R5900DebugPrint::DSUBU ( stringstream &strInstString, long instruction )
{
	strInstString << "DSUBU";
	AddInstArgs ( strInstString, instruction, FTDSUBU );
}

void R5900DebugPrint::TGE ( stringstream &strInstString, long instruction )
{
	strInstString << "TGE";
	AddInstArgs ( strInstString, instruction, FTTGE );
}

void R5900DebugPrint::TGEU ( stringstream &strInstString, long instruction )
{
	strInstString << "TGEU";
	AddInstArgs ( strInstString, instruction, FTTGEU );
}

void R5900DebugPrint::TLT ( stringstream &strInstString, long instruction )
{
	strInstString << "TLT";
	AddInstArgs ( strInstString, instruction, FTTLT );
}

void R5900DebugPrint::TLTU ( stringstream &strInstString, long instruction )
{
	strInstString << "TLTU";
	AddInstArgs ( strInstString, instruction, FTTLTU );
}

void R5900DebugPrint::TEQ ( stringstream &strInstString, long instruction )
{
	strInstString << "TEQ";
	AddInstArgs ( strInstString, instruction, FTTEQ );
}

void R5900DebugPrint::TNE ( stringstream &strInstString, long instruction )
{
	strInstString << "TNE";
	AddInstArgs ( strInstString, instruction, FTTNE );
}

void R5900DebugPrint::DSLL ( stringstream &strInstString, long instruction )
{
	strInstString << "DSLL";
	AddInstArgs ( strInstString, instruction, FTDSLL );
}

void R5900DebugPrint::DSRL ( stringstream &strInstString, long instruction )
{
	strInstString << "DSRL";
	AddInstArgs ( strInstString, instruction, FTDSRL );
}

void R5900DebugPrint::DSRA ( stringstream &strInstString, long instruction )
{
	strInstString << "DSRA";
	AddInstArgs ( strInstString, instruction, FTDSRA );
}

void R5900DebugPrint::DSLL32 ( stringstream &strInstString, long instruction )
{
	strInstString << "DSLL32";
	AddInstArgs ( strInstString, instruction, FTDSLL32 );
}

void R5900DebugPrint::DSRL32 ( stringstream &strInstString, long instruction )
{
	strInstString << "DSRL32";
	AddInstArgs ( strInstString, instruction, FTDSRL32 );
}

void R5900DebugPrint::DSRA32 ( stringstream &strInstString, long instruction )
{
	strInstString << "DSRA32";
	AddInstArgs ( strInstString, instruction, FTDSRA32 );
}

void R5900DebugPrint::MADD ( stringstream &strInstString, long instruction )
{
	strInstString << "MADD";
	AddInstArgs ( strInstString, instruction, FTMADD );
}

void R5900DebugPrint::MADDU ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDU";
	AddInstArgs ( strInstString, instruction, FTMADDU );
}

void R5900DebugPrint::PLZCW ( stringstream &strInstString, long instruction )
{
	strInstString << "PLZCW";
	AddInstArgs ( strInstString, instruction, FTPLZCW );
}


void R5900DebugPrint::MFHI1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MFHI1";
	AddInstArgs ( strInstString, instruction, FTMFHI1 );
}

void R5900DebugPrint::MTHI1MTLO1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MTHI1MTLO1";
//	AddInstArgs ( strInstString, instruction, FTMTHI1MTLO1 );
}

void R5900DebugPrint::MULT1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MULT1";
	AddInstArgs ( strInstString, instruction, FTMULT1 );
}

void R5900DebugPrint::MULTU1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MULTU1";
	AddInstArgs ( strInstString, instruction, FTMULTU1 );
}

void R5900DebugPrint::DIV1 ( stringstream &strInstString, long instruction )
{
	strInstString << "DIV1";
	AddInstArgs ( strInstString, instruction, FTDIV1 );
}

void R5900DebugPrint::DIVU1 ( stringstream &strInstString, long instruction )
{
	strInstString << "DIVU1";
	AddInstArgs ( strInstString, instruction, FTDIVU1 );
}

void R5900DebugPrint::MFLO1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MFLO1";
	AddInstArgs ( strInstString, instruction, FTMFLO1 );
}

void R5900DebugPrint::MADD1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MADD1";
	AddInstArgs ( strInstString, instruction, FTMADD1 );
}

void R5900DebugPrint::MADDU1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDU1";
	AddInstArgs ( strInstString, instruction, FTMADDU1 );
}


void R5900DebugPrint::PMFHLFMT ( stringstream &strInstString, long instruction )
{
	strInstString << "PMFHL";
	AddInstArgs ( strInstString, instruction, FTPMFHL );
}

void R5900DebugPrint::PMTHL ( stringstream &strInstString, long instruction )
{
	strInstString << "PMTHL";
	AddInstArgs ( strInstString, instruction, FTPMTHLLW );
}

void R5900DebugPrint::PSLLH ( stringstream &strInstString, long instruction )
{
	strInstString << "PSLLH";
	AddInstArgs ( strInstString, instruction, FTPSLLH );
}

void R5900DebugPrint::PSRLH ( stringstream &strInstString, long instruction )
{
	strInstString << "PSRLH";
	AddInstArgs ( strInstString, instruction, FTPSRLH );
}

void R5900DebugPrint::PSRAH ( stringstream &strInstString, long instruction )
{
	strInstString << "PSRAH";
	AddInstArgs ( strInstString, instruction, FTPSRAH );
}

void R5900DebugPrint::PSLLW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSLLW";
	AddInstArgs ( strInstString, instruction, FTPSLLW );
}

void R5900DebugPrint::PSRLW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSRLW";
	AddInstArgs ( strInstString, instruction, FTPSRLW );
}

void R5900DebugPrint::PSRAW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSRAW";
	AddInstArgs ( strInstString, instruction, FTPSRAW );
}

void R5900DebugPrint::TLBR ( stringstream &strInstString, long instruction )
{
	strInstString << "TLBR";
	AddInstArgs ( strInstString, instruction, FTTLBR );
}

void R5900DebugPrint::TLBWI ( stringstream &strInstString, long instruction )
{
	strInstString << "TLBWI";
	AddInstArgs ( strInstString, instruction, FTTLBWI );
}

void R5900DebugPrint::TLBWR ( stringstream &strInstString, long instruction )
{
	strInstString << "TLBWR";
	AddInstArgs ( strInstString, instruction, FTTLBWR );
}

void R5900DebugPrint::TLBP ( stringstream &strInstString, long instruction )
{
	strInstString << "TLBP";
	AddInstArgs ( strInstString, instruction, FTTLBP );
}

void R5900DebugPrint::ERET ( stringstream &strInstString, long instruction )
{
	strInstString << "ERET";
	AddInstArgs ( strInstString, instruction, FTERET );
}

//void R5900DebugPrint::DERET ( stringstream &strInstString, long instruction )
//{
//	strInstString << "DERET";
//	AddInstArgs ( strInstString, instruction, FTDERET );
//}

//void R5900DebugPrint::WAIT ( stringstream &strInstString, long instruction )
//{
//	strInstString << "WAIT";
//	AddInstArgs ( strInstString, instruction, FTWAIT );
//}

// PS1 may be able to use this, so I'll just comment it out for now
//void R5900DebugPrint::MFC2 ( stringstream &strInstString, long instruction )
//{
//	strInstString << "MFC2";
//	AddInstArgs ( strInstString, instruction, FTMFC2 );
//}

void R5900DebugPrint::CFC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "CFC2";
	AddInstArgs ( strInstString, instruction, FTCFC2 );
}

// PS1 may be able to use this, so I'll just comment it out for now
//void R5900DebugPrint::MTC2 ( stringstream &strInstString, long instruction )
//{
//	strInstString << "MTC2";
//	AddInstArgs ( strInstString, instruction, FTMTC2 );
//}


void R5900DebugPrint::MFC1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MFC1";
	AddInstArgs ( strInstString, instruction, FTMFC1 );
}

void R5900DebugPrint::CFC1 ( stringstream &strInstString, long instruction )
{
	strInstString << "CFC1";
	AddInstArgs ( strInstString, instruction, FTCFC1 );
}

void R5900DebugPrint::MTC1 ( stringstream &strInstString, long instruction )
{
	strInstString << "MTC1";
	AddInstArgs ( strInstString, instruction, FTMTC1 );
}

void R5900DebugPrint::CTC1 ( stringstream &strInstString, long instruction )
{
	strInstString << "CTC1";
	AddInstArgs ( strInstString, instruction, FTCTC1 );
}

void R5900DebugPrint::CVTSFMTW ( stringstream &strInstString, long instruction )
{
	strInstString << "CVTSFMTW";
	AddInstArgs ( strInstString, instruction, FTCVTSFMTW );
}

void R5900DebugPrint::ADDFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDFMTS";
	AddInstArgs ( strInstString, instruction, FTADDFMTS );
}

void R5900DebugPrint::SUBFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBFMTS";
	AddInstArgs ( strInstString, instruction, FTSUBFMTS );
}

void R5900DebugPrint::MULFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "MULFMTS";
	AddInstArgs ( strInstString, instruction, FTMULFMTS );
}

void R5900DebugPrint::DIVFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "DIVFMTS";
	AddInstArgs ( strInstString, instruction, FTDIVFMTS );
}

void R5900DebugPrint::SQRTFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "SQRTFMTS";
	AddInstArgs ( strInstString, instruction, FTSQRTFMTS );
}

void R5900DebugPrint::ABSFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "ABSFMTS";
	AddInstArgs ( strInstString, instruction, FTABSFMTS );
}

void R5900DebugPrint::MOVFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "MOVFMTS";
	AddInstArgs ( strInstString, instruction, FTMOVFMTS );
}

void R5900DebugPrint::NEGFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "NEGFMTS";
	AddInstArgs ( strInstString, instruction, FTNEGFMTS );
}

void R5900DebugPrint::RSQRTFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "RSQRTFMTS";
	AddInstArgs ( strInstString, instruction, FTRSQRTFMTS );
}

void R5900DebugPrint::ADDAFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "ADDAFMTS";
	AddInstArgs ( strInstString, instruction, FTADDAFMTS );
}

void R5900DebugPrint::SUBAFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "SUBAFMTS";
	AddInstArgs ( strInstString, instruction, FTSUBAFMTS );
}

void R5900DebugPrint::MADDFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDFMTS";
	AddInstArgs ( strInstString, instruction, FTMADDFMTS );
}

void R5900DebugPrint::MSUBFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "MSUBFMTS";
	AddInstArgs ( strInstString, instruction, FTMSUBFMTS );
}

void R5900DebugPrint::MADDAFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "MADDAFMTS";
	AddInstArgs ( strInstString, instruction, FTMADDAFMTS );
}

void R5900DebugPrint::CVTWFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "CVTWFMTS";
	AddInstArgs ( strInstString, instruction, FTCVTWFMTS );
}

void R5900DebugPrint::MAXFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "MAXFMTS";
	AddInstArgs ( strInstString, instruction, FTMAXFMTS );
}

void R5900DebugPrint::MINFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "MINFMTS";
	AddInstArgs ( strInstString, instruction, FTMINFMTS );
}

void R5900DebugPrint::CFFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "CFFMTS";
	AddInstArgs ( strInstString, instruction, FTCFFMTS );
}

void R5900DebugPrint::CEQFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "CEQFMTS";
	AddInstArgs ( strInstString, instruction, FTCEQFMTS );
}

void R5900DebugPrint::CLTFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "CLTFMTS";
	AddInstArgs ( strInstString, instruction, FTCLTFMTS );
}

void R5900DebugPrint::CLEFMTS ( stringstream &strInstString, long instruction )
{
	strInstString << "CLEFMTS";
	AddInstArgs ( strInstString, instruction, FTCLEFMTS );
}

void R5900DebugPrint::PADDW ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDW";
	AddInstArgs ( strInstString, instruction, FTPADDW );
}

void R5900DebugPrint::PSUBW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBW";
	AddInstArgs ( strInstString, instruction, FTPSUBW );
}

void R5900DebugPrint::PCGTW ( stringstream &strInstString, long instruction )
{
	strInstString << "PCGTW";
	AddInstArgs ( strInstString, instruction, FTPCGTW );
}

void R5900DebugPrint::PMAXW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMAXW";
	AddInstArgs ( strInstString, instruction, FTPMAXW );
}

void R5900DebugPrint::PADDH ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDH";
	AddInstArgs ( strInstString, instruction, FTPADDH );
}

void R5900DebugPrint::PSUBH ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBH";
	AddInstArgs ( strInstString, instruction, FTPSUBH );
}

void R5900DebugPrint::PCGTH ( stringstream &strInstString, long instruction )
{
	strInstString << "PCGTH";
	AddInstArgs ( strInstString, instruction, FTPCGTH );
}

void R5900DebugPrint::PMAXH ( stringstream &strInstString, long instruction )
{
	strInstString << "PMAXH";
	AddInstArgs ( strInstString, instruction, FTPMAXH );
}

void R5900DebugPrint::PADDB ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDB";
	AddInstArgs ( strInstString, instruction, FTPADDB );
}

void R5900DebugPrint::PSUBB ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBB";
	AddInstArgs ( strInstString, instruction, FTPSUBB );
}

void R5900DebugPrint::PCGTB ( stringstream &strInstString, long instruction )
{
	strInstString << "PCGTB";
	AddInstArgs ( strInstString, instruction, FTPCGTB );
}

void R5900DebugPrint::PADDSW ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDSW";
	AddInstArgs ( strInstString, instruction, FTPADDSW );
}

void R5900DebugPrint::PSUBSW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBSW";
	AddInstArgs ( strInstString, instruction, FTPSUBSW );
}

void R5900DebugPrint::PEXTLW ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXTLW";
	AddInstArgs ( strInstString, instruction, FTPEXTLW );
}

void R5900DebugPrint::PPACW ( stringstream &strInstString, long instruction )
{
	strInstString << "PPACW";
	AddInstArgs ( strInstString, instruction, FTPPACW );
}

void R5900DebugPrint::PADDSH ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDSH";
	AddInstArgs ( strInstString, instruction, FTPADDSH );
}

void R5900DebugPrint::PSUBSH ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBSH";
	AddInstArgs ( strInstString, instruction, FTPSUBSH );
}

void R5900DebugPrint::PEXTLH ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXTLH";
	AddInstArgs ( strInstString, instruction, FTPEXTLH );
}

void R5900DebugPrint::PPACH ( stringstream &strInstString, long instruction )
{
	strInstString << "PPACH";
	AddInstArgs ( strInstString, instruction, FTPPACH );
}

void R5900DebugPrint::PADDSB ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDSB";
	AddInstArgs ( strInstString, instruction, FTPADDSB );
}

void R5900DebugPrint::PSUBSB ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBSB";
	AddInstArgs ( strInstString, instruction, FTPSUBSB );
}

void R5900DebugPrint::PEXTLB ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXTLB";
	AddInstArgs ( strInstString, instruction, FTPEXTLB );
}

void R5900DebugPrint::PPACB ( stringstream &strInstString, long instruction )
{
	strInstString << "PPACB";
	AddInstArgs ( strInstString, instruction, FTPPACB );
}

void R5900DebugPrint::PEXT5 ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXT5";
	AddInstArgs ( strInstString, instruction, FTPEXT5 );
}

void R5900DebugPrint::PPAC5 ( stringstream &strInstString, long instruction )
{
	strInstString << "PPAC5";
	AddInstArgs ( strInstString, instruction, FTPPAC5 );
}

void R5900DebugPrint::PABSW ( stringstream &strInstString, long instruction )
{
	strInstString << "PABSW";
	AddInstArgs ( strInstString, instruction, FTPABSW );
}

void R5900DebugPrint::PCEQW ( stringstream &strInstString, long instruction )
{
	strInstString << "PCEQW";
	AddInstArgs ( strInstString, instruction, FTPCEQW );
}

void R5900DebugPrint::PMINW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMINW";
	AddInstArgs ( strInstString, instruction, FTPMINW );
}

void R5900DebugPrint::PADSBH ( stringstream &strInstString, long instruction )
{
	strInstString << "PADSBH";
	AddInstArgs ( strInstString, instruction, FTPADSBH );
}

void R5900DebugPrint::PABSH ( stringstream &strInstString, long instruction )
{
	strInstString << "PABSH";
	AddInstArgs ( strInstString, instruction, FTPABSH );
}

void R5900DebugPrint::PCEQH ( stringstream &strInstString, long instruction )
{
	strInstString << "PCEQH";
	AddInstArgs ( strInstString, instruction, FTPCEQH );
}

void R5900DebugPrint::PMINH ( stringstream &strInstString, long instruction )
{
	strInstString << "PMINH";
	AddInstArgs ( strInstString, instruction, FTPMINH );
}

void R5900DebugPrint::PCEQB ( stringstream &strInstString, long instruction )
{
	strInstString << "PCEQB";
	AddInstArgs ( strInstString, instruction, FTPCEQB );
}

void R5900DebugPrint::PADDUW ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDUW";
	AddInstArgs ( strInstString, instruction, FTPADDUW );
}

void R5900DebugPrint::PSUBUW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBUW";
	AddInstArgs ( strInstString, instruction, FTPSUBUW );
}

void R5900DebugPrint::PEXTUW ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXTUW";
	AddInstArgs ( strInstString, instruction, FTPEXTUW );
}

void R5900DebugPrint::PADDUH ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDUH";
	AddInstArgs ( strInstString, instruction, FTPADDUH );
}

void R5900DebugPrint::PSUBUH ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBUH";
	AddInstArgs ( strInstString, instruction, FTPSUBUH );
}

void R5900DebugPrint::PEXTUH ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXTUH";
	AddInstArgs ( strInstString, instruction, FTPEXTUH );
}

void R5900DebugPrint::PADDUB ( stringstream &strInstString, long instruction )
{
	strInstString << "PADDUB";
	AddInstArgs ( strInstString, instruction, FTPADDUB );
}

void R5900DebugPrint::PSUBUB ( stringstream &strInstString, long instruction )
{
	strInstString << "PSUBUB";
	AddInstArgs ( strInstString, instruction, FTPSUBUB );
}

void R5900DebugPrint::PEXTUB ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXTUB";
	AddInstArgs ( strInstString, instruction, FTPEXTUB );
}

void R5900DebugPrint::QFSRV ( stringstream &strInstString, long instruction )
{
	strInstString << "QFSRV";
	AddInstArgs ( strInstString, instruction, FTQFSRV );
}

void R5900DebugPrint::PMADDW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMADDW";
	AddInstArgs ( strInstString, instruction, FTPMADDW );
}

void R5900DebugPrint::PSLLVW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSLLVW";
	AddInstArgs ( strInstString, instruction, FTPSLLVW );
}

void R5900DebugPrint::PSRLVW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSRLVW";
	AddInstArgs ( strInstString, instruction, FTPSRLVW );
}

void R5900DebugPrint::PMSUBW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMSUBW";
	AddInstArgs ( strInstString, instruction, FTPMSUBW );
}

void R5900DebugPrint::PMFHI ( stringstream &strInstString, long instruction )
{
	strInstString << "PMFHI";
	AddInstArgs ( strInstString, instruction, FTPMFHI );
}

void R5900DebugPrint::PMFLO ( stringstream &strInstString, long instruction )
{
	strInstString << "PMFLO";
	AddInstArgs ( strInstString, instruction, FTPMFLO );
}

void R5900DebugPrint::PINTH ( stringstream &strInstString, long instruction )
{
	strInstString << "PINTH";
	AddInstArgs ( strInstString, instruction, FTPINTH );
}

void R5900DebugPrint::PMULTW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMULTW";
	AddInstArgs ( strInstString, instruction, FTPMULTW );
}

void R5900DebugPrint::PDIVW ( stringstream &strInstString, long instruction )
{
	strInstString << "PDIVW";
	AddInstArgs ( strInstString, instruction, FTPDIVW );
}

void R5900DebugPrint::PCPYLD ( stringstream &strInstString, long instruction )
{
	strInstString << "PCPYLD";
	AddInstArgs ( strInstString, instruction, FTPCPYLD );
}

void R5900DebugPrint::PMADDH ( stringstream &strInstString, long instruction )
{
	strInstString << "PMADDH";
	AddInstArgs ( strInstString, instruction, FTPMADDH );
}

void R5900DebugPrint::PHMADH ( stringstream &strInstString, long instruction )
{
	strInstString << "PHMADH";
	AddInstArgs ( strInstString, instruction, FTPHMADH );
}

void R5900DebugPrint::PAND ( stringstream &strInstString, long instruction )
{
	strInstString << "PAND";
	AddInstArgs ( strInstString, instruction, FTPAND );
}

void R5900DebugPrint::PXOR ( stringstream &strInstString, long instruction )
{
	strInstString << "PXOR";
	AddInstArgs ( strInstString, instruction, FTPXOR );
}

void R5900DebugPrint::PMSUBH ( stringstream &strInstString, long instruction )
{
	strInstString << "PMSUBH";
	AddInstArgs ( strInstString, instruction, FTPMSUBH );
}

void R5900DebugPrint::PHMSBH ( stringstream &strInstString, long instruction )
{
	strInstString << "PHMSBH";
	AddInstArgs ( strInstString, instruction, FTPHMSBH );
}

void R5900DebugPrint::PEXEH ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXEH";
	AddInstArgs ( strInstString, instruction, FTPEXEH );
}

void R5900DebugPrint::PREVH ( stringstream &strInstString, long instruction )
{
	strInstString << "PREVH";
	AddInstArgs ( strInstString, instruction, FTPREVH );
}

void R5900DebugPrint::PMULTH ( stringstream &strInstString, long instruction )
{
	strInstString << "PMULTH";
	AddInstArgs ( strInstString, instruction, FTPMULTH );
}

void R5900DebugPrint::PDIVBW ( stringstream &strInstString, long instruction )
{
	strInstString << "PDIVBW";
	AddInstArgs ( strInstString, instruction, FTPDIVBW );
}

void R5900DebugPrint::PEXEW ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXEW";
	AddInstArgs ( strInstString, instruction, FTPEXEW );
}

void R5900DebugPrint::PROT3W ( stringstream &strInstString, long instruction )
{
	strInstString << "PROT3W";
	AddInstArgs ( strInstString, instruction, FTPROT3W );
}

void R5900DebugPrint::PMADDUW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMADDUW";
	AddInstArgs ( strInstString, instruction, FTPMADDUW );
}

void R5900DebugPrint::PSRAVW ( stringstream &strInstString, long instruction )
{
	strInstString << "PSRAVW";
	AddInstArgs ( strInstString, instruction, FTPSRAVW );
}

void R5900DebugPrint::PMTHI ( stringstream &strInstString, long instruction )
{
	strInstString << "PMTHI";
	AddInstArgs ( strInstString, instruction, FTPMTHI );
}

void R5900DebugPrint::PMTLO ( stringstream &strInstString, long instruction )
{
	strInstString << "PMTLO";
	AddInstArgs ( strInstString, instruction, FTPMTLO );
}

void R5900DebugPrint::PINTEH ( stringstream &strInstString, long instruction )
{
	strInstString << "PINTEH";
	AddInstArgs ( strInstString, instruction, FTPINTEH );
}

void R5900DebugPrint::PMULTUW ( stringstream &strInstString, long instruction )
{
	strInstString << "PMULTUW";
	AddInstArgs ( strInstString, instruction, FTPMULTUW );
}

void R5900DebugPrint::PDIVUW ( stringstream &strInstString, long instruction )
{
	strInstString << "PDIVUW";
	AddInstArgs ( strInstString, instruction, FTPDIVUW );
}

void R5900DebugPrint::PCPYUD ( stringstream &strInstString, long instruction )
{
	strInstString << "PCPYUD";
	AddInstArgs ( strInstString, instruction, FTPCPYUD );
}

void R5900DebugPrint::POR ( stringstream &strInstString, long instruction )
{
	strInstString << "POR";
	AddInstArgs ( strInstString, instruction, FTPOR );
}

void R5900DebugPrint::PNOR ( stringstream &strInstString, long instruction )
{
	strInstString << "PNOR";
	AddInstArgs ( strInstString, instruction, FTPNOR );
}

void R5900DebugPrint::PEXCH ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXCH";
	AddInstArgs ( strInstString, instruction, FTPEXCH );
}

void R5900DebugPrint::PCPYH ( stringstream &strInstString, long instruction )
{
	strInstString << "PCPYH";
	AddInstArgs ( strInstString, instruction, FTPCPYH );
}

void R5900DebugPrint::PEXCW ( stringstream &strInstString, long instruction )
{
	strInstString << "PEXCW";
	AddInstArgs ( strInstString, instruction, FTPEXCW );
}

void R5900DebugPrint::QMFC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "QMFC2";
	AddInstArgs ( strInstString, instruction, FTQMFC2 );
}

void R5900DebugPrint::QMTC2 ( stringstream &strInstString, long instruction )
{
	strInstString << "QMTC2";
	AddInstArgs ( strInstString, instruction, FTQMTC2 );
}


void R5900DebugPrint::VADDBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDBCX";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void R5900DebugPrint::VADDBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDBCY";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void R5900DebugPrint::VADDBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDBCZ";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void R5900DebugPrint::VADDBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDBCW";
	AddInstArgs ( strInstString, instruction, FTVADDBC );
}

void R5900DebugPrint::VSUBBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBBCX";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void R5900DebugPrint::VSUBBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBBCY";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void R5900DebugPrint::VSUBBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBBCZ";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void R5900DebugPrint::VSUBBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBBCW";
	AddInstArgs ( strInstString, instruction, FTVSUBBC );
}

void R5900DebugPrint::VMADDBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDBCX";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void R5900DebugPrint::VMADDBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDBCY";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void R5900DebugPrint::VMADDBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDBCZ";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void R5900DebugPrint::VMADDBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDBCW";
	AddInstArgs ( strInstString, instruction, FTVMADDBC );
}

void R5900DebugPrint::VMSUBBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBBCX";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void R5900DebugPrint::VMSUBBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBBCY";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void R5900DebugPrint::VMSUBBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBBCZ";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void R5900DebugPrint::VMSUBBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBBCW";
	AddInstArgs ( strInstString, instruction, FTVMSUBBC );
}

void R5900DebugPrint::VMAXBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMAXBCX";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void R5900DebugPrint::VMAXBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VMAXBCY";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void R5900DebugPrint::VMAXBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMAXBCZ";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void R5900DebugPrint::VMAXBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VMAXBCW";
	AddInstArgs ( strInstString, instruction, FTVMAXBC );
}

void R5900DebugPrint::VMINIBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMINIBCX";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void R5900DebugPrint::VMINIBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VMINIBCY";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void R5900DebugPrint::VMINIBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMINIBCZ";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void R5900DebugPrint::VMINIBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VMINIBCW";
	AddInstArgs ( strInstString, instruction, FTVMINIBC );
}

void R5900DebugPrint::VMULBCX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULBCX";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void R5900DebugPrint::VMULBCY ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULBCY";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void R5900DebugPrint::VMULBCZ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULBCZ";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void R5900DebugPrint::VMULBCW ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULBCW";
	AddInstArgs ( strInstString, instruction, FTVMULBC );
}

void R5900DebugPrint::VMULQ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULQ";
	AddInstArgs ( strInstString, instruction, FTVMULQ );
}

void R5900DebugPrint::VMAXI ( stringstream &strInstString, long instruction )
{
	strInstString << "VMAXI";
	AddInstArgs ( strInstString, instruction, FTVMAXI );
}

void R5900DebugPrint::VMULI ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULI";
	AddInstArgs ( strInstString, instruction, FTVMULI );
}

void R5900DebugPrint::VMINII ( stringstream &strInstString, long instruction )
{
	strInstString << "VMINII";
	AddInstArgs ( strInstString, instruction, FTVMINII );
}

void R5900DebugPrint::VADDQ ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDQ";
	AddInstArgs ( strInstString, instruction, FTVADDQ );
}

void R5900DebugPrint::VMADDQ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDQ";
	AddInstArgs ( strInstString, instruction, FTVMADDQ );
}

void R5900DebugPrint::VADDI ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDI";
	AddInstArgs ( strInstString, instruction, FTVADDI );
}

void R5900DebugPrint::VMADDI ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDI";
	AddInstArgs ( strInstString, instruction, FTVMADDI );
}

void R5900DebugPrint::VSUBQ ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBQ";
	AddInstArgs ( strInstString, instruction, FTVSUBQ );
}

void R5900DebugPrint::VMSUBQ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBQ";
	AddInstArgs ( strInstString, instruction, FTVMSUBQ );
}

void R5900DebugPrint::VSUBI ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBI";
	AddInstArgs ( strInstString, instruction, FTVSUBI );
}

void R5900DebugPrint::VMSUBI ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBI";
	AddInstArgs ( strInstString, instruction, FTVMSUBI );
}

void R5900DebugPrint::VADD ( stringstream &strInstString, long instruction )
{
	strInstString << "VADD";
	AddInstArgs ( strInstString, instruction, FTVADD );
}

void R5900DebugPrint::VMADD ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADD";
	AddInstArgs ( strInstString, instruction, FTVMADD );
}

void R5900DebugPrint::VMUL ( stringstream &strInstString, long instruction )
{
	strInstString << "VMUL";
	AddInstArgs ( strInstString, instruction, FTVMUL );
}

void R5900DebugPrint::VMAX ( stringstream &strInstString, long instruction )
{
	strInstString << "VMAX";
	AddInstArgs ( strInstString, instruction, FTVMAX );
}

void R5900DebugPrint::VSUB ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUB";
	AddInstArgs ( strInstString, instruction, FTVSUB );
}

void R5900DebugPrint::VMSUB ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUB";
	AddInstArgs ( strInstString, instruction, FTVMSUB );
}

void R5900DebugPrint::VOPMSUB ( stringstream &strInstString, long instruction )
{
	strInstString << "VOPMSUB";
	AddInstArgs ( strInstString, instruction, FTVOPMSUB );
}

void R5900DebugPrint::VMINI ( stringstream &strInstString, long instruction )
{
	strInstString << "VMINI";
	AddInstArgs ( strInstString, instruction, FTVMINI );
}

void R5900DebugPrint::VIADD ( stringstream &strInstString, long instruction )
{
	strInstString << "VIADD";
	AddInstArgs ( strInstString, instruction, FTVIADD );
}

void R5900DebugPrint::VISUB ( stringstream &strInstString, long instruction )
{
	strInstString << "VISUB";
	AddInstArgs ( strInstString, instruction, FTVISUB );
}

void R5900DebugPrint::VIADDI ( stringstream &strInstString, long instruction )
{
	strInstString << "VIADDI";
	AddInstArgs ( strInstString, instruction, FTVIADDI );
}

void R5900DebugPrint::VIAND ( stringstream &strInstString, long instruction )
{
	strInstString << "VIAND";
	AddInstArgs ( strInstString, instruction, FTVIAND );
}

void R5900DebugPrint::VIOR ( stringstream &strInstString, long instruction )
{
	strInstString << "VIOR";
	AddInstArgs ( strInstString, instruction, FTVIOR );
}

void R5900DebugPrint::VCALLMS ( stringstream &strInstString, long instruction )
{
	strInstString << "VCALLMS";
	AddInstArgs ( strInstString, instruction, FTVCALLMS );
}

void R5900DebugPrint::VCALLMSR ( stringstream &strInstString, long instruction )
{
	strInstString << "VCALLMSR";
	AddInstArgs ( strInstString, instruction, FTVCALLMSR );
}

void R5900DebugPrint::VITOF0 ( stringstream &strInstString, long instruction )
{
	strInstString << "VITOF0";
	AddInstArgs ( strInstString, instruction, FTVITOF0 );
}

void R5900DebugPrint::VFTOI0 ( stringstream &strInstString, long instruction )
{
	strInstString << "VFTOI0";
	AddInstArgs ( strInstString, instruction, FTVFTOI0 );
}

void R5900DebugPrint::VMULAQ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULAQ";
	AddInstArgs ( strInstString, instruction, FTVMULAQ );
}

void R5900DebugPrint::VADDAQ ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDAQ";
	AddInstArgs ( strInstString, instruction, FTVADDAQ );
}

void R5900DebugPrint::VSUBAQ ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBAQ";
	AddInstArgs ( strInstString, instruction, FTVSUBAQ );
}

void R5900DebugPrint::VADDA ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDA";
	AddInstArgs ( strInstString, instruction, FTVADDA );
}

void R5900DebugPrint::VSUBA ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBA";
	AddInstArgs ( strInstString, instruction, FTVSUBA );
}

void R5900DebugPrint::VMOVE ( stringstream &strInstString, long instruction )
{
	strInstString << "VMOVE";
	AddInstArgs ( strInstString, instruction, FTVMOVE );
}

void R5900DebugPrint::VLQI ( stringstream &strInstString, long instruction )
{
	strInstString << "VLQI";
	AddInstArgs ( strInstString, instruction, FTVLQI );
}

void R5900DebugPrint::VDIV ( stringstream &strInstString, long instruction )
{
	strInstString << "VDIV";
	AddInstArgs ( strInstString, instruction, FTVDIV );
}

void R5900DebugPrint::VMTIR ( stringstream &strInstString, long instruction )
{
	strInstString << "VMTIR";
	AddInstArgs ( strInstString, instruction, FTVMTIR );
}

void R5900DebugPrint::VRNEXT ( stringstream &strInstString, long instruction )
{
	strInstString << "VRNEXT";
	AddInstArgs ( strInstString, instruction, FTVRNEXT );
}

void R5900DebugPrint::VITOF4 ( stringstream &strInstString, long instruction )
{
	strInstString << "VITOF4";
	AddInstArgs ( strInstString, instruction, FTVITOF4 );
}

void R5900DebugPrint::VFTOI4 ( stringstream &strInstString, long instruction )
{
	strInstString << "VFTOI4";
	AddInstArgs ( strInstString, instruction, FTVFTOI4 );
}

void R5900DebugPrint::VABS ( stringstream &strInstString, long instruction )
{
	strInstString << "VABS";
	AddInstArgs ( strInstString, instruction, FTVABS );
}

void R5900DebugPrint::VMADDAQ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDAQ";
	AddInstArgs ( strInstString, instruction, FTVMADDAQ );
}

void R5900DebugPrint::VMSUBAQ ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBAQ";
	AddInstArgs ( strInstString, instruction, FTVMSUBAQ );
}

void R5900DebugPrint::VMADDA ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDA";
	AddInstArgs ( strInstString, instruction, FTVMADDA );
}

void R5900DebugPrint::VMSUBA ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBA";
	AddInstArgs ( strInstString, instruction, FTVMSUBA );
}

void R5900DebugPrint::VMR32 ( stringstream &strInstString, long instruction )
{
	strInstString << "VMR32";
	AddInstArgs ( strInstString, instruction, FTVMR32 );
}

void R5900DebugPrint::VSQI ( stringstream &strInstString, long instruction )
{
	strInstString << "VSQI";
	AddInstArgs ( strInstString, instruction, FTVSQI );
}

void R5900DebugPrint::VSQRT ( stringstream &strInstString, long instruction )
{
	strInstString << "VSQRT";
	AddInstArgs ( strInstString, instruction, FTVSQRT );
}

void R5900DebugPrint::VMFIR ( stringstream &strInstString, long instruction )
{
	strInstString << "VMFIR";
	AddInstArgs ( strInstString, instruction, FTVMFIR );
}

void R5900DebugPrint::VRGET ( stringstream &strInstString, long instruction )
{
	strInstString << "VRGET";
	AddInstArgs ( strInstString, instruction, FTVRGET );
}

void R5900DebugPrint::VADDABC ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDABC";
	AddInstArgs ( strInstString, instruction, FTVADDABC );
}

void R5900DebugPrint::VSUBABC ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBABC";
	AddInstArgs ( strInstString, instruction, FTVSUBABC );
}

void R5900DebugPrint::VMADDABC ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDABC";
	AddInstArgs ( strInstString, instruction, FTVMADDABC );
}

void R5900DebugPrint::VMSUBABC ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBABC";
	AddInstArgs ( strInstString, instruction, FTVMSUBABC );
}

void R5900DebugPrint::VITOF12 ( stringstream &strInstString, long instruction )
{
	strInstString << "VITOF12";
	AddInstArgs ( strInstString, instruction, FTVITOF12 );
}

void R5900DebugPrint::VFTOI12 ( stringstream &strInstString, long instruction )
{
	strInstString << "VFTOI12";
	AddInstArgs ( strInstString, instruction, FTVFTOI12 );
}

void R5900DebugPrint::VMULABC ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULABC";
	AddInstArgs ( strInstString, instruction, FTVMULABC );
}

void R5900DebugPrint::VMULAI ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULAI";
	AddInstArgs ( strInstString, instruction, FTVMULAI );
}

void R5900DebugPrint::VADDAI ( stringstream &strInstString, long instruction )
{
	strInstString << "VADDAI";
	AddInstArgs ( strInstString, instruction, FTVADDAI );
}

void R5900DebugPrint::VSUBAI ( stringstream &strInstString, long instruction )
{
	strInstString << "VSUBAI";
	AddInstArgs ( strInstString, instruction, FTVSUBAI );
}

void R5900DebugPrint::VMULA ( stringstream &strInstString, long instruction )
{
	strInstString << "VMULA";
	AddInstArgs ( strInstString, instruction, FTVMULA );
}

void R5900DebugPrint::VOPMULA ( stringstream &strInstString, long instruction )
{
	strInstString << "VOPMULA";
	AddInstArgs ( strInstString, instruction, FTVOPMULA );
}

void R5900DebugPrint::VLQD ( stringstream &strInstString, long instruction )
{
	strInstString << "VLQD";
	AddInstArgs ( strInstString, instruction, FTVLQD );
}

void R5900DebugPrint::VRSQRT ( stringstream &strInstString, long instruction )
{
	strInstString << "VRSQRT";
	AddInstArgs ( strInstString, instruction, FTVRSQRT );
}

void R5900DebugPrint::VILWR ( stringstream &strInstString, long instruction )
{
	strInstString << "VILWR";
	AddInstArgs ( strInstString, instruction, FTVILWR );
}

void R5900DebugPrint::VRINIT ( stringstream &strInstString, long instruction )
{
	strInstString << "VRINIT";
	AddInstArgs ( strInstString, instruction, FTVRINIT );
}

void R5900DebugPrint::VITOF15 ( stringstream &strInstString, long instruction )
{
	strInstString << "VITOF15";
	AddInstArgs ( strInstString, instruction, FTVITOF15 );
}

void R5900DebugPrint::VFTOI15 ( stringstream &strInstString, long instruction )
{
	strInstString << "VFTOI15";
	AddInstArgs ( strInstString, instruction, FTVFTOI15 );
}

void R5900DebugPrint::VCLIP ( stringstream &strInstString, long instruction )
{
	strInstString << "VCLIP";
	AddInstArgs ( strInstString, instruction, FTVCLIP );
}

void R5900DebugPrint::VMADDAI ( stringstream &strInstString, long instruction )
{
	strInstString << "VMADDAI";
	AddInstArgs ( strInstString, instruction, FTVMADDAI );
}

void R5900DebugPrint::VMSUBAI ( stringstream &strInstString, long instruction )
{
	strInstString << "VMSUBAI";
	AddInstArgs ( strInstString, instruction, FTVMSUBAI );
}

void R5900DebugPrint::VNOP ( stringstream &strInstString, long instruction )
{
	strInstString << "VNOP";
	AddInstArgs ( strInstString, instruction, FTVNOP );
}

void R5900DebugPrint::VSQD ( stringstream &strInstString, long instruction )
{
	strInstString << "VSQD";
	AddInstArgs ( strInstString, instruction, FTVSQD );
}

void R5900DebugPrint::VWAITQ ( stringstream &strInstString, long instruction )
{
	strInstString << "VWAITQ";
	AddInstArgs ( strInstString, instruction, FTVWAITQ );
}

void R5900DebugPrint::VISWR ( stringstream &strInstString, long instruction )
{
	strInstString << "VISWR";
	AddInstArgs ( strInstString, instruction, FTVISWR );
}

void R5900DebugPrint::VRXOR ( stringstream &strInstString, long instruction )
{
	strInstString << "VRXOR";
	AddInstArgs ( strInstString, instruction, FTVRXOR );
}


void R5900DebugPrint::AddVuDestArgs ( stringstream &strVuArgs, long Instruction )
{
	// add the dot
	strVuArgs << ".";
	
	if ( GET_DESTX( Instruction ) ) strVuArgs << "x";
	if ( GET_DESTY( Instruction ) ) strVuArgs << "y";
	if ( GET_DESTZ( Instruction ) ) strVuArgs << "z";
	if ( GET_DESTW( Instruction ) ) strVuArgs << "w";
}


void R5900DebugPrint::AddInstArgs ( stringstream &strMipsArgs, long Instruction, long InstFormat )
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



