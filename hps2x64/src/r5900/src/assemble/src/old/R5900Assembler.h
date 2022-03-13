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


namespace R5900
{

	class Assembler
	{
	
	public:
	
		unsigned long* NextAddress;

		void SetAddress ( unsigned long *StartAddress );

		void NOP ( void );

		void ADD ( long rd, long rs, long rt );
		void ADDI ( long rt, long rs, long immediate );
		void ADDIU ( long rt, long rs, long immediate );
		void ADDU ( long rd, long rs, long rt );
		void AND ( long rd, long rs, long rt );
		void ANDI ( long rt, long rs, long immediate );

		void DADDIU ( long rt, long rs, long immediate );
		void DADDU ( long rd, long rs, long rt );
		
		void DSLL ( long rd, long rt, long sa );
		void DSLL32 ( long rd, long rt, long sa );
		void DSLLV ( long rd, long rt, long rs );
		void DSRA ( long rd, long rt, long sa );
		void DSRA32 ( long rd, long rt, long sa );
		void DSRAV ( long rd, long rt, long rs );
		void DSRL ( long rd, long rt, long sa );
		void DSRL32 ( long rd, long rt, long sa );
		void DSRLV ( long rd, long rt, long rs );
		void DSUB ( long rd, long rs, long rt );
		void DSUBU ( long rd, long rs, long rt );

		void LUI ( long rt, long immediate );
		
		void MOVN ( long rd, long rs, long rt );
		void MOVZ ( long rd, long rs, long rt );
		
		void NOR ( long rd, long rs, long rt );
		void OR ( long rd, long rs, long rt );
		void ORI ( long rt, long rs, long immediate );
		void PREF ( long hint, short offset, long base );

		void SLL ( long rd, long rt, long sa );
		void SLLV ( long rd, long rt, long rs );
		void SLT ( long rd, long rs, long rt );
		void SLTI ( long rt, long rs, long immediate );
		void SLTIU ( long rt, long rs, long immediate );
		void SLTU ( long rd, long rs, long rt );
		void SRA ( long rd, long rt, long sa );
		void SRAV ( long rd, long rt, long rs );
		void SRL ( long rd, long rt, long sa );
		void SRLV ( long rd, long rt, long rs );
		void SUB ( long rd, long rs, long rt );
		void SUBU ( long rd, long rs, long rt );

		void XOR ( long rd, long rs, long rt );
		void XORI ( long rt, long rs, long immediate );
		
		// mips branch instructions
		void BC0F ( long offset );
		void BC0FL ( long offset );
		void BC0T ( long offset );
		void BC0TL ( long offset );
		void BC1F ( long offset );
		void BC1FL ( long offset );
		void BC1T ( long offset );
		void BC1TL ( long offset );
		void BC2F ( long offset );	// branches if vu0 is idle
		void BC2FL ( long offset );
		void BC2T ( long offset );	// branches if vu0 is in use
		void BC2TL ( long offset );

		void BEQ ( long rs, long rt, long offset );
		void BEQL ( long rs, long rt, long offset );
		void BGEZ ( long rs, long offset );
		void BGEZAL ( long rs, long offset );
		void BGEZALL ( long rs, long offset );
		void BGEZL ( long rs, long offset );
		void BGTZ ( long rs, long offset );
		void BGTZL ( long rs, long offset );
		void BLEZ ( long rs, long offset );
		void BLEZL ( long rs, long offset );
		void BLTZ ( long rs, long offset );
		void BLTZAL ( long rs, long offset );
		void BLTZALL ( long rs, long offset );
		void BLTZL ( long rs, long offset );
		void BNE ( long rs, long rt, long offset );
		void BNEL ( long rs, long rt, long offset );
		
		void J ( long address );
		void JAL ( long address );
		void JALR ( long rd, long rs );
		void JR ( long rs );
		
		// trap instructions

		void TEQ ( long rs, long rt );
		void TEQI ( long rs, long immediate );
		void TGE ( long rs, long rt );
		void TGEI ( long rs, long immediate );
		void TGEIU ( long rs, long immediate );
		void TGEU ( long rs, long rt );
		void TLT ( long rs, long rt );
		void TLTI ( long rs, long immediate );
		void TLTIU ( long rs, long immediate );
		void TLTU ( long rs, long rt );
		void TNE ( long rs, long rt );
		void TNEI ( long rs, long immediate );

		// load/store instructions
		void LB ( long rt, long base, long offset );
		void SB ( long rt, long base, long offset );
		void LBU ( long rt, long base, long offset );
		void LH ( long rt, long base, long offset );
		void SH ( long rt, long base, long offset );
		void LHU ( long rt, long base, long offset );
		void LW ( long rt, long base, long offset );
		void SW ( long rt, long base, long offset );
		void LWU ( long rt, long base, long offset );
		void LWR ( long rt, long base, long offset );
		void SWR ( long rt, long base, long offset );
		void LWL ( long rt, long base, long offset );
		void SWL ( long rt, long base, long offset );
		void LD ( long rt, long base, long offset );
		void SD ( long rt, long base, long offset );
		void LQ ( long rt, long base, long offset );
		void SQ ( long rt, long base, long offset );

		// ps2 cop1 instructions
		
		void ABSS ( long fd, long fs );
		void ADDS ( long fd, long fs, long ft );
		void ADDAS ( long fs, long ft );
		void CEQS ( long fs, long ft );
		void CFS ( long fs, long ft );
		void CLES ( long fs, long ft );
		void CLTS ( long fs, long ft );
		void CFC1 ( long rt, long fs );
		void CTC1 ( long rt, long fs );
		void CVTSW ( long fd, long fs );
		void CVTWS ( long fd, long fs );
		void DIVS ( long fd, long fs, long ft );
		void LWC1 ( long ft, long base, long offset );
		void MADDS ( long fd, long fs, long ft );
		void MADDAS ( long fs, long ft );
		void MAXS ( long fd, long fs, long ft );
		void MFC1 ( long rt, long fs );
		void MINS ( long fd, long fs, long ft );
		void MOVS ( long fd, long fs );
		void MSUBS ( long fd, long fs, long ft );
		void MSUBAS ( long fs, long ft );
		void MTC1 ( long rt, long fs );
		void MULS ( long fd, long fs, long ft );
		void MULAS ( long fs, long ft );
		void NEGS ( long fd, long fs );
		void RSQRTS ( long fd, long fs, long ft );
		void SQRTS ( long fd, long ft );
		void SUBS ( long fd, long fs, long ft );
		void SUBAS ( long fs, long ft );
		void SWC1 ( long ft, long base, long offset );
		
		// ps2 vector integer instructions
		void PABSH ( long rd, long rt );
		void PABSW ( long rd, long rt );
		void PADDB ( long rd, long rs, long rt );
		void PADDH ( long rd, long rs, long rt );
		void PADDW ( long rd, long rs, long rt );
		void PADDSB ( long rd, long rs, long rt );
		void PADDSH ( long rd, long rs, long rt );
		void PADDSW ( long rd, long rs, long rt );
		void PADDUB ( long rd, long rs, long rt );
		void PADDUH ( long rd, long rs, long rt );
		void PADDUW ( long rd, long rs, long rt );
		void PADSBH ( long rd, long rs, long rt );
		void PAND ( long rd, long rs, long rt );
		void PCEQB ( long rd, long rs, long rt );
		void PCEQH ( long rd, long rs, long rt );
		void PCEQW ( long rd, long rs, long rt );
		void PCGTB ( long rd, long rs, long rt );
		void PCGTH ( long rd, long rs, long rt );
		void PCGTW ( long rd, long rs, long rt );
		void PCPYH ( long rd, long rt );
		void PCPYLD ( long rd, long rs, long rt );
		void PCPYUD ( long rd, long rs, long rt );
		void PDIVBW ( long rs, long rt );
		void PDIVUW ( long rs, long rt );
		void PDIVW ( long rs, long rt );
		void PEXCH ( long rd, long rt );
		void PEXCW ( long rd, long rt );
		void PEXEH ( long rd, long rt );
		void PEXEW ( long rd, long rt );
		void PEXT5 ( long rd, long rt );
		void PEXTLB ( long rd, long rs, long rt );
		void PEXTLH ( long rd, long rs, long rt );
		void PEXTLW ( long rd, long rs, long rt );
		void PEXTUB ( long rd, long rs, long rt );
		void PEXTUH ( long rd, long rs, long rt );
		void PEXTUW ( long rd, long rs, long rt );
		void PHMADH ( long rd, long rs, long rt );
		void PHMSBH ( long rd, long rs, long rt );
		void PINTEH ( long rd, long rs, long rt );
		void PINTH ( long rd, long rs, long rt );
		void PLZCW ( long rd, long rs );
		void PMADDH ( long rd, long rs, long rt );
		void PMADDUW ( long rd, long rs, long rt );
		void PMADDW ( long rd, long rs, long rt );
		void PMAXH ( long rd, long rs, long rt );
		void PMAXW ( long rd, long rs, long rt );
		void PMFHI ( long rd );
		void PMFHLLH ( long rd );
		void PMFHLLW ( long rd );
		void PMFHLSH ( long rd );
		void PMFHLSLW ( long rd );
		void PMFHLULW ( long rd );
		void PMFLO ( long rd );
		void PMINH ( long rd, long rs, long rt );
		void PMINW ( long rd, long rs, long rt );
		void PMSUBH ( long rd, long rs, long rt );
		void PMSUBW ( long rd, long rs, long rt );
		void PMTHI ( long rs );
		void PMTHlLW ( long rs );
		void PMTLO ( long rs );
		void PMULTH ( long rd, long rs, long rt );
		void PMULTUW ( long rd, long rs, long rt );
		void PMULTW ( long rd, long rs, long rt );
		void PNOR ( long rd, long rs, long rt );
		void POR ( long rd, long rs, long rt );
		void PPAC5 ( long rd, long rt );
		void PPACB ( long rd, long rs, long rt );
		void PPACH ( long rd, long rs, long rt );
		void PPACW ( long rd, long rs, long rt );
		void PREVH ( long rd, long rt );
		void PROT3W ( long rd, long rt );
		void PSLLH ( long rd, long rt, long sa );
		void PSLLVW ( long rd, long rt, long rs );
		void PSLLW ( long rd, long rt, long sa );
		void PSRAH ( long rd, long rt, long sa );
		void PSRAVW ( long rd, long rt, long rs );
		void PSRAW ( long rd, long rt, long sa );
		void PSRLH ( long rd, long rt, long sa );
		void PSRLVW ( long rd, long rt, long rs );
		void PSRLW ( long rd, long rt, long sa );
		void PSUBB ( long rd, long rs, long rt );
		void PSUBH ( long rd, long rs, long rt );
		void PSUBSB ( long rd, long rs, long rt );
		void PSUBSH ( long rd, long rs, long rt );
		void PSUBSW ( long rd, long rs, long rt );
		void PSUBUB ( long rd, long rs, long rt );
		void PSUBUH ( long rd, long rs, long rt );
		void PSUBUW ( long rd, long rs, long rt );
		void PSUBW ( long rd, long rs, long rt );
		void PXOR ( long rd, long rs, long rt );
		void QFSRV ( long rd, long rs, long rt );

		// cop2 (vu0) simple integer instructions

		void VIADD ( long id, long is, long it );
		void VIADDI ( long it, long is, long Imm5 );
		void VIAND ( long id, long is, long it );
		void VILWR ( long dest, long it, long is );
		void VIOR ( long id, long is, long it );
		void VISUB ( long id, long is, long it );
		void VISWR ( long dest, long it, long is );

		// testing of vu float instructions
		void CFC2 ( long rt, long id, long I );
		void CTC2 ( long rt, long id, long I );
		void LQC2 ( long ft, long base, long offset );
		void QMFC2 ( long rt, long fd, long I );
		void QMTC2 ( long rt, long fd, long I );
		void SQC2 ( long ft, long base, long offset );
		void VABS ( long dest, long ft, long fs );
		void VADD ( long dest, long fd, long fs, long ft );
		void VADDi ( long dest, long fd, long fs );
		void VADDq ( long dest, long fd, long fs );
		void VADDbc ( long dest, long bc, long fd, long fs, long ft );
		void VADDA ( long dest, long fs, long ft );
		void VADDAi ( long dest, long fs );
		void VADDAq ( long dest, long fs );
		void VADDAbc ( long dest, long bc, long fs, long ft );
		void VCLIP ( long fs, long ft );
		void VDIV ( long ftf, long fsf, long fs, long ft );
		void VFTOI0 ( long dest, long ft, long fs );
		void VFTOI4 ( long dest, long ft, long fs );
		void VFTOI12 ( long dest, long ft, long fs );
		void VFTOI15 ( long dest, long ft, long fs );
		void VITOF0 ( long dest, long ft, long fs );
		void VITOF4 ( long dest, long ft, long fs );
		void VITOF12 ( long dest, long ft, long fs );
		void VITOF15 ( long dest, long ft, long fs );
		void VLQD ( long dest, long ft, long is );
		void VLQI ( long dest, long ft, long is );
		void VMADD ( long dest, long fd, long fs, long ft );
		void VMADDi ( long dest, long fd, long fs );
		void VMADDq ( long dest, long fd, long fs );
		void VMADDbc ( long dest, long bc, long fd, long fs, long ft );
		void VMADDA ( long dest, long fs, long ft );
		void VMADDAi ( long dest, long fs );
		void VMADDAq ( long dest, long fs );
		void VMADDAbc ( long dest, long bc, long fs, long ft );
		void VMAX ( long dest, long fd, long fs, long ft );
		void VMAXi ( long dest, long fd, long fs );
		void VMAXbc ( long dest, long bc, long fd, long fs, long ft );
		void VMFIR ( long dest, long ft, long is );
		void VMINI ( long dest, long fd, long fs, long ft );
		void VMINIi ( long dest, long fd, long fs );
		void VMINIbc ( long dest, long bc, long fd, long fs, long ft );
		void VMOVE ( long dest, long ft, long fs );
		void VMR32 ( long dest, long ft, long fs );
		void VMSUB ( long dest, long fd, long fs, long ft );
		void VMSUBi ( long dest, long fd, long fs );
		void VMSUBq ( long dest, long fd, long fs );
		void VMSUBbc ( long dest, long bc, long fd, long fs, long ft );
		void VMSUBA ( long dest, long fs, long ft );
		void VMSUBAi ( long dest, long fs );
		void VMSUBAq ( long dest, long fs );
		void VMSUBAbc ( long dest, long bc, long fs, long ft );
		void VMTIR ( long fsf, long it, long fs );
		void VMUL ( long dest, long fd, long fs, long ft );
		void VMULi ( long dest, long fd, long fs );
		void VMULq ( long dest, long fd, long fs );
		void VMULbc ( long dest, long bc, long fd, long fs, long ft );
		void VMULA ( long dest, long fs, long ft );
		void VMULAi ( long dest, long fs );
		void VMULAq ( long dest, long fs );
		void VMULAbc ( long dest, long bc, long fs, long ft );
		void VNOP ( void );
		void VOPMULA ( long dest, long fs, long ft );
		void VRGET ( long dest, long ft );
		void VRINIT ( long fsf, long fs );
		void VRNEXT ( long dest, long ft );
		void VRSQRT ( long ftf, long fsf, long fs, long ft );
		void VRXOR ( long fsf, long fs );
		void VSQD ( long dest, long fs, long it );
		void VSQI ( long dest, long fs, long it );
		void VSQRT ( long ftf, long ft );
		void VSUB ( long dest, long fd, long fs, long ft );
		void VSUBi ( long dest, long fd, long fs );
		void VSUBq ( long dest, long fd, long fs );
		void VSUBbc ( long dest, long bc, long fd, long fs, long ft );
		void VSUBA ( long dest, long fs, long ft );
		void VSUBAi ( long dest, long fs );
		void VSUBAq ( long dest, long fs );
		void VSUBAbc ( long dest, long bc, long fs, long ft );
		void VWAITQ ( void );
	};
	
};
		