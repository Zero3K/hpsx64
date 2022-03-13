

#ifndef _VUENCODER_H_

#define _VUENCODER_H_

#include "x64Encoder.h"

#include "SignExtend.h"



#define REVERSE4BITS(dest)		( ( ( ( dest ) & 0x1 ) << 3 ) | ( ( ( dest ) & 0x2 ) << 1 ) | ( ( ( dest ) & 0x4 ) >> 1 ) | ( ( ( dest ) & 0x8 ) >> 3 ) )

// x64 encoder for 64-bit Mips processsor
class VuEncoder
{

public:

	long InstructionAddress;	// must know the address of the instruction currently being encoded
	bool BranchDelaySlot;	// we need to know if we are in branch delay slot for trap handling and branch in delay slot, etc.

	x64Encoder* x;
	
	long VuNumber;
	
	// constructor
	VuEncoder ( x64Encoder* enc, long VuNbr );
	

	
	// upper instructions
	
	bool ABS ( long dest, long ft, long fs );
	bool ADD ( long dest, long fd, long fs, long ft );


	// lower instructions
	bool IADD ( long id, long is, long it );

	// vu simple integer instructions

	bool VIADDI ( long it, long is, long Imm5 );
	bool VIAND ( long id, long is, long it );
	bool VILWR ( long dest, long it, long is );
	bool VIOR ( long id, long is, long it );
	bool VISUB ( long id, long is, long it );
	bool VISWR ( long dest, long it, long is );

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
	
	// * vu instruction headers * //
	
	void HEADER ( long fs, long ft );
	void HEADERi ( long fs );
	void HEADERq ( long fs );
	void HEADERbc ( long bc, long fs, long ft );

	// * vu instruction headers * //
	
	bool FOOTER ( long dest, long fd );
	bool FOOTERA ( long dest );


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


