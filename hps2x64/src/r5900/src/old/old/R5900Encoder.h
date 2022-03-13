

#ifndef _R5900ENCODER_H_

#define _R5900ENCODER_H_



#include "x64Encoder.h"

#include "SignExtend.h"


#define R0		0
#define R1		1
#define R2		2
#define R3		3
#define R4		4
#define R5		5
#define R6		6
#define R7		7
#define R8		8
#define R9		9
#define R10		10
#define R11		11
#define R12		12
#define R13		13
#define R14		14
#define R15		15
#define R16		16
#define R17		17
#define R18		18
#define R19		19
#define R20		20
#define R21		21
#define R22		22
#define R23		23
#define R24		24
#define R25		25
#define R26		26
#define R27		27
#define R28		28
#define R29		29
#define R30		30
#define R31		31

#define F0		0
#define F1		1
#define F2		2
#define F3		3
#define F4		4
#define F5		5
#define F6		6
#define F7		7
#define F8		8
#define F9		9
#define F10		10
#define F11		11
#define F12		12
#define F13		13
#define F14		14
#define F15		15
#define F16		16
#define F17		17
#define F18		18
#define F19		19
#define F20		20
#define F21		21
#define F22		22
#define F23		23
#define F24		24
#define F25		25
#define F26		26
#define F27		27
#define F28		28
#define F29		29
#define F30		30
#define F31		31

#define REVERSE4BITS(dest)		( ( ( ( dest ) & 0x1 ) << 3 ) | ( ( ( dest ) & 0x2 ) << 1 ) | ( ( ( dest ) & 0x4 ) >> 1 ) | ( ( ( dest ) & 0x8 ) >> 3 ) )

// x64 encoder for 64-bit Mips processsor
class R5900Encoder
{

public:

	long InstructionAddress;	// must know the address of the instruction currently being encoded
	bool BranchDelaySlot;	// we need to know if we are in branch delay slot for trap handling and branch in delay slot, etc.

	x64Encoder* x;
//	MipsEERegAllocator* r;

	// constructor
	R5900Encoder ( x64Encoder* enc );

	// mips simple integer instructions

	bool ADD ( long rd, long rs, long rt );
	bool ADDI ( long rt, long rs, long immediate );
	bool ADDIU ( long rt, long rs, long immediate );
	bool ADDU ( long rd, long rs, long rt );
	bool AND ( long rd, long rs, long rt );
	bool ANDI ( long rt, long rs, long immediate );

	bool DADDIU ( long rt, long rs, long immediate );
	bool DADDU ( long rd, long rs, long rt );
	
	bool DSLL ( long rd, long rt, long sa );
	bool DSLL32 ( long rd, long rt, long sa );
	bool DSLLV ( long rd, long rt, long rs );
	bool DSRA ( long rd, long rt, long sa );
	bool DSRA32 ( long rd, long rt, long sa );
	bool DSRAV ( long rd, long rt, long rs );
	bool DSRL ( long rd, long rt, long sa );
	bool DSRL32 ( long rd, long rt, long sa );
	bool DSRLV ( long rd, long rt, long rs );
	bool DSUB ( long rd, long rs, long rt );
	bool DSUBU ( long rd, long rs, long rt );

	bool LUI ( long rt, long immediate );
	
	bool MOVN ( long rd, long rs, long rt );
	bool MOVZ ( long rd, long rs, long rt );
	
	bool NOR ( long rd, long rs, long rt );
	bool OR ( long rd, long rs, long rt );
	bool ORI ( long rt, long rs, long immediate );
	bool PREF ( long hint, short offset, long base );

	bool SLL ( long rd, long rt, long sa );
	bool SLLV ( long rd, long rt, long rs );
	bool SLT ( long rd, long rs, long rt );
	bool SLTI ( long rt, long rs, long immediate );
	bool SLTIU ( long rt, long rs, long immediate );
	bool SLTU ( long rd, long rs, long rt );
	bool SRA ( long rd, long rt, long sa );
	bool SRAV ( long rd, long rt, long rs );
	bool SRL ( long rd, long rt, long sa );
	bool SRLV ( long rd, long rt, long rs );
	bool SUB ( long rd, long rs, long rt );
	bool SUBU ( long rd, long rs, long rt );

	bool XOR ( long rd, long rs, long rt );
	bool XORI ( long rt, long rs, long immediate );
	
	// mips branch instructions
	bool BC0F ( long offset );
	bool BC0FL ( long offset );
	bool BC0T ( long offset );
	bool BC0TL ( long offset );
	bool BC1F ( long offset );
	bool BC1FL ( long offset );
	bool BC1T ( long offset );
	bool BC1TL ( long offset );
	bool BC2F ( long offset );	// branches if vu0 is idle
	bool BC2FL ( long offset );
	bool BC2T ( long offset );	// branches if vu0 is in use
	bool BC2TL ( long offset );

	bool BEQ ( long rs, long rt, long offset );
	bool BEQL ( long rs, long rt, long offset );
	bool BGEZ ( long rs, long offset );
	bool BGEZAL ( long rs, long offset );
	bool BGEZALL ( long rs, long offset );
	bool BGEZL ( long rs, long offset );
	bool BGTZ ( long rs, long offset );
	bool BGTZL ( long rs, long offset );
	bool BLEZ ( long rs, long offset );
	bool BLEZL ( long rs, long offset );
	bool BLTZ ( long rs, long offset );
	bool BLTZAL ( long rs, long offset );
	bool BLTZALL ( long rs, long offset );
	bool BLTZL ( long rs, long offset );
	bool BNE ( long rs, long rt, long offset );
	bool BNEL ( long rs, long rt, long offset );
	
	bool J ( long address );
	bool JAL ( long address );
	bool JALR ( long rd, long rs );
	bool JR ( long rs );
	
	// trap instructions

	bool TEQ ( long rs, long rt );
	bool TEQI ( long rs, long immediate );
	bool TGE ( long rs, long rt );
	bool TGEI ( long rs, long immediate );
	bool TGEIU ( long rs, long immediate );
	bool TGEU ( long rs, long rt );
	bool TLT ( long rs, long rt );
	bool TLTI ( long rs, long immediate );
	bool TLTIU ( long rs, long immediate );
	bool TLTU ( long rs, long rt );
	bool TNE ( long rs, long rt );
	bool TNEI ( long rs, long immediate );

	// load/store instructions
	bool LB ( long rt, long base, long offset );
	bool SB ( long rt, long base, long offset );
	bool LBU ( long rt, long base, long offset );
	bool LH ( long rt, long base, long offset );
	bool SH ( long rt, long base, long offset );
	bool LHU ( long rt, long base, long offset );
	bool LW ( long rt, long base, long offset );
	bool SW ( long rt, long base, long offset );
	bool LWU ( long rt, long base, long offset );
	bool LWR ( long rt, long base, long offset );
	bool SWR ( long rt, long base, long offset );
	bool LWL ( long rt, long base, long offset );
	bool SWL ( long rt, long base, long offset );
	bool LD ( long rt, long base, long offset );
	bool SD ( long rt, long base, long offset );
	bool LQ ( long rt, long base, long offset );
	bool SQ ( long rt, long base, long offset );

	// ps2 cop1 instructions
	
	bool ABSS ( long fd, long fs );
	bool ADDS ( long fd, long fs, long ft );
	bool ADDAS ( long fs, long ft );
	bool CEQS ( long fs, long ft );
	bool CFS ( long fs, long ft );
	bool CLES ( long fs, long ft );
	bool CLTS ( long fs, long ft );
	bool CFC1 ( long rt, long fs );
	bool CTC1 ( long rt, long fs );
	bool CVTSW ( long fd, long fs );
	bool CVTWS ( long fd, long fs );
	bool DIVS ( long fd, long fs, long ft );
	bool LWC1 ( long ft, long base, long offset );
	bool MADDS ( long fd, long fs, long ft );
	bool MADDAS ( long fs, long ft );
	bool MAXS ( long fd, long fs, long ft );
	bool MFC1 ( long rt, long fs );
	bool MINS ( long fd, long fs, long ft );
	bool MOVS ( long fd, long fs );
	bool MSUBS ( long fd, long fs, long ft );
	bool MSUBAS ( long fs, long ft );
	bool MTC1 ( long rt, long fs );
	bool MULS ( long fd, long fs, long ft );
	bool MULAS ( long fs, long ft );
	bool NEGS ( long fd, long fs );
	bool RSQRTS ( long fd, long fs, long ft );
	bool SQRTS ( long fd, long ft );
	bool SUBS ( long fd, long fs, long ft );
	bool SUBAS ( long fs, long ft );
	bool SWC1 ( long ft, long base, long offset );
	
	// ps2 vector integer instructions
	bool PABSH ( long rd, long rt );
	bool PABSW ( long rd, long rt );
	bool PADDB ( long rd, long rs, long rt );
	bool PADDH ( long rd, long rs, long rt );
	bool PADDW ( long rd, long rs, long rt );
	bool PADDSB ( long rd, long rs, long rt );
	bool PADDSH ( long rd, long rs, long rt );
	bool PADDSW ( long rd, long rs, long rt );
	bool PADDUB ( long rd, long rs, long rt );
	bool PADDUH ( long rd, long rs, long rt );
	bool PADDUW ( long rd, long rs, long rt );
	bool PADSBH ( long rd, long rs, long rt );
	bool PAND ( long rd, long rs, long rt );
	bool PCEQB ( long rd, long rs, long rt );
	bool PCEQH ( long rd, long rs, long rt );
	bool PCEQW ( long rd, long rs, long rt );
	bool PCGTB ( long rd, long rs, long rt );
	bool PCGTH ( long rd, long rs, long rt );
	bool PCGTW ( long rd, long rs, long rt );
	bool PCPYH ( long rd, long rt );
	bool PCPYLD ( long rd, long rs, long rt );
	bool PCPYUD ( long rd, long rs, long rt );
	bool PDIVBW ( long rs, long rt );
	bool PDIVUW ( long rs, long rt );
	bool PDIVW ( long rs, long rt );
	bool PEXCH ( long rd, long rt );
	bool PEXCW ( long rd, long rt );
	bool PEXEH ( long rd, long rt );
	bool PEXEW ( long rd, long rt );
	bool PEXT5 ( long rd, long rt );
	bool PEXTLB ( long rd, long rs, long rt );
	bool PEXTLH ( long rd, long rs, long rt );
	bool PEXTLW ( long rd, long rs, long rt );
	bool PEXTUB ( long rd, long rs, long rt );
	bool PEXTUH ( long rd, long rs, long rt );
	bool PEXTUW ( long rd, long rs, long rt );
	bool PHMADH ( long rd, long rs, long rt );
	bool PHMSBH ( long rd, long rs, long rt );
	bool PINTEH ( long rd, long rs, long rt );
	bool PINTH ( long rd, long rs, long rt );
	bool PLZCW ( long rd, long rs );
	bool PMADDH ( long rd, long rs, long rt );
	bool PMADDUW ( long rd, long rs, long rt );
	bool PMADDW ( long rd, long rs, long rt );
	bool PMAXH ( long rd, long rs, long rt );
	bool PMAXW ( long rd, long rs, long rt );
	bool PMFHI ( long rd );
	bool PMFHLLH ( long rd );
	bool PMFHLLW ( long rd );
	bool PMFHLSH ( long rd );
	bool PMFHLSLW ( long rd );
	bool PMFHLULW ( long rd );
	bool PMFLO ( long rd );
	bool PMINH ( long rd, long rs, long rt );
	bool PMINW ( long rd, long rs, long rt );
	bool PMSUBH ( long rd, long rs, long rt );
	bool PMSUBW ( long rd, long rs, long rt );
	bool PMTHI ( long rs );
	bool PMTHlLW ( long rs );
	bool PMTLO ( long rs );
	bool PMULTH ( long rd, long rs, long rt );
	bool PMULTUW ( long rd, long rs, long rt );
	bool PMULTW ( long rd, long rs, long rt );
	bool PNOR ( long rd, long rs, long rt );
	bool POR ( long rd, long rs, long rt );
	bool PPAC5 ( long rd, long rt );
	bool PPACB ( long rd, long rs, long rt );
	bool PPACH ( long rd, long rs, long rt );
	bool PPACW ( long rd, long rs, long rt );
	bool PREVH ( long rd, long rt );
	bool PROT3W ( long rd, long rt );
	bool PSLLH ( long rd, long rt, long sa );
	bool PSLLVW ( long rd, long rt, long rs );
	bool PSLLW ( long rd, long rt, long sa );
	bool PSRAH ( long rd, long rt, long sa );
	bool PSRAVW ( long rd, long rt, long rs );
	bool PSRAW ( long rd, long rt, long sa );
	bool PSRLH ( long rd, long rt, long sa );
	bool PSRLVW ( long rd, long rt, long rs );
	bool PSRLW ( long rd, long rt, long sa );
	bool PSUBB ( long rd, long rs, long rt );
	bool PSUBH ( long rd, long rs, long rt );
	bool PSUBSB ( long rd, long rs, long rt );
	bool PSUBSH ( long rd, long rs, long rt );
	bool PSUBSW ( long rd, long rs, long rt );
	bool PSUBUB ( long rd, long rs, long rt );
	bool PSUBUH ( long rd, long rs, long rt );
	bool PSUBUW ( long rd, long rs, long rt );
	bool PSUBW ( long rd, long rs, long rt );
	bool PXOR ( long rd, long rs, long rt );
	bool QFSRV ( long rd, long rs, long rt );

	// cop2 (vu0) simple integer instructions

	bool VIADD ( long id, long is, long it );
	bool VIADDI ( long it, long is, long Imm5 );
	bool VIAND ( long id, long is, long it );
	bool VILWR ( long dest, long it, long is );
	bool VIOR ( long id, long is, long it );
	bool VISUB ( long id, long is, long it );
	bool VISWR ( long dest, long it, long is );

	// testing of vu float instructions
	bool CFC2 ( long rt, long id, long I );
	bool CTC2 ( long rt, long id, long I );
	bool LQC2 ( long ft, long base, long offset );
	bool QMFC2 ( long rt, long fd, long I );
	bool QMTC2 ( long rt, long fd, long I );
	bool SQC2 ( long ft, long base, long offset );
	bool VABS ( long dest, long ft, long fs );
	bool VADD ( long dest, long fd, long fs, long ft );
	bool VADDi ( long dest, long fd, long fs );
	bool VADDq ( long dest, long fd, long fs );
	bool VADDbc ( long dest, long bc, long fd, long fs, long ft );
	bool VADDA ( long dest, long fs, long ft );
	bool VADDAi ( long dest, long fs );
	bool VADDAq ( long dest, long fs );
	bool VADDAbc ( long dest, long bc, long fs, long ft );
	bool VCLIP ( long fs, long ft );
	bool VDIV ( long ftf, long fsf, long fs, long ft );
	bool VFTOI0 ( long dest, long ft, long fs );
	bool VFTOI4 ( long dest, long ft, long fs );
	bool VFTOI12 ( long dest, long ft, long fs );
	bool VFTOI15 ( long dest, long ft, long fs );
	bool VITOF0 ( long dest, long ft, long fs );
	bool VITOF4 ( long dest, long ft, long fs );
	bool VITOF12 ( long dest, long ft, long fs );
	bool VITOF15 ( long dest, long ft, long fs );
	bool VLQD ( long dest, long ft, long is );
	bool VLQI ( long dest, long ft, long is );
	bool VMADD ( long dest, long fd, long fs, long ft );
	bool VMADDi ( long dest, long fd, long fs );
	bool VMADDq ( long dest, long fd, long fs );
	bool VMADDbc ( long dest, long bc, long fd, long fs, long ft );
	bool VMADDA ( long dest, long fs, long ft );
	bool VMADDAi ( long dest, long fs );
	bool VMADDAq ( long dest, long fs );
	bool VMADDAbc ( long dest, long bc, long fs, long ft );
	bool VMAX ( long dest, long fd, long fs, long ft );
	bool VMAXi ( long dest, long fd, long fs );
	bool VMAXbc ( long dest, long bc, long fd, long fs, long ft );
	bool VMFIR ( long dest, long ft, long is );
	bool VMINI ( long dest, long fd, long fs, long ft );
	bool VMINIi ( long dest, long fd, long fs );
	bool VMINIbc ( long dest, long bc, long fd, long fs, long ft );
	bool VMOVE ( long dest, long ft, long fs );
	bool VMR32 ( long dest, long ft, long fs );
	bool VMSUB ( long dest, long fd, long fs, long ft );
	bool VMSUBi ( long dest, long fd, long fs );
	bool VMSUBq ( long dest, long fd, long fs );
	bool VMSUBbc ( long dest, long bc, long fd, long fs, long ft );
	bool VMSUBA ( long dest, long fs, long ft );
	bool VMSUBAi ( long dest, long fs );
	bool VMSUBAq ( long dest, long fs );
	bool VMSUBAbc ( long dest, long bc, long fs, long ft );
	bool VMTIR ( long fsf, long it, long fs );
	bool VMUL ( long dest, long fd, long fs, long ft );
	bool VMULi ( long dest, long fd, long fs );
	bool VMULq ( long dest, long fd, long fs );
	bool VMULbc ( long dest, long bc, long fd, long fs, long ft );
	bool VMULA ( long dest, long fs, long ft );
	bool VMULAi ( long dest, long fs );
	bool VMULAq ( long dest, long fs );
	bool VMULAbc ( long dest, long bc, long fs, long ft );
	bool VNOP ( void );
	bool VOPMULA ( long dest, long fs, long ft );
	bool VRGET ( long dest, long ft );
	bool VRINIT ( long fsf, long fs );
	bool VRNEXT ( long dest, long ft );
	bool VRSQRT ( long ftf, long fsf, long fs, long ft );
	bool VRXOR ( long fsf, long fs );
	bool VSQD ( long dest, long fs, long it );
	bool VSQI ( long dest, long fs, long it );
	bool VSQRT ( long ftf, long ft );
	bool VSUB ( long dest, long fd, long fs, long ft );
	bool VSUBi ( long dest, long fd, long fs );
	bool VSUBq ( long dest, long fd, long fs );
	bool VSUBbc ( long dest, long bc, long fd, long fs, long ft );
	bool VSUBA ( long dest, long fs, long ft );
	bool VSUBAi ( long dest, long fs );
	bool VSUBAq ( long dest, long fs );
	bool VSUBAbc ( long dest, long bc, long fs, long ft );
	bool VWAITQ ( void );

private:

	void ClampXMM0AndXMM3 ( void );
	void SetVUFlagsForXMM0 ( void );
	void SetMADDVUFlagsAndGetResult ( void );

	// I need these otherwise this will take a zillion lines of c++
	inline void Format1 ( long &rd, long &rs, long &rt );
	inline void Format2 ( long &rt, long &rs, long &immediate );
	inline void Format3 ( long &rs, long &rt, long &offset );
	inline void Format4 ( long &rs, long &offset );
	inline void Format5 ( long &rs, long &rt );
	inline void Format6 ( long &rd, long &rt, long &sa );
	inline void Format7 ( long &rd, long &rt, long &rs );
	
	inline void Format12 ( long &rt, long immediate );
	
	// formats for vector integer EE instructions

	inline void Format1V ( long &rd, long &rs, long &rt );
	inline void Format15V ( long &rd, long &rt );
	
	// formats for cop1 instructions
	
	inline void Format20 ( long &fd, long &fs );
	inline void Format21 ( long &fd, long &fs, long &ft );
	inline void Format22 ( long &fs, long &ft );
	inline void Format23 ( long &offset );
	inline void Format24 ( long &ft, long &fs );
	inline void Format25 ( long &fd, long &ft );
	inline void Format44 ( long &rt, long &fs );
	inline void Format45 ( long &fs, long &rt );

	// formats for VU instructions

	inline void Format26 ( long &dest, long &ft, long &fs );	// op.dest ft, fs
	inline void Format27 ( long &dest, long &fd, long &fs, long &ft );	// op.dest fd, fs, ft
	inline void Format28 ( long &dest, long &fd, long &fs );	// op.dest fd, fs
	inline void Format29 ( long &dest, long &fs, long &ft );	// op.dest fs, ft
	inline void Format30 ( long &dest, long &fs );	// op.dest fs
	inline void Format31 ( long &Imm15 );	// op.dest Imm15
	inline void Format32 ( long &ftf, long &fsf, long &fs, long &ft );	// op.dest fsfsf, ftftf

	inline void Format33 ( long &id, long &is, long &it );
	inline void Format34 ( long &it, long &is, long &Imm5 );
	inline void Format35FromMem ( long &dest, long &it, long &is );
	inline void Format35ToMem ( long &dest, long &it, long &is );

	inline void Format36 ( long &fsf, long &fs );	// op fsfsf
	inline void Format38 ( long &ftf, long &ft );	// op ftftf
	inline void Format39 ( long &dest, long &ft );	// op.dest ft
	inline void Format40 ( long &dest, long &fs, long &it );	// op.dest fs, (it)
	inline void Format41 ( long &fsf, long &it, long &fs );	// op it, fsfsf
	inline void Format42 ( long &dest, long &ft, long &is );	// op.dest ft, (is)
	inline void Format43 ( long &fs, long &ft );
};

#endif




