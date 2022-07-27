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



#ifndef _MIPSOPCODE_H_

#define _MIPSOPCODE_H_


// this gives you what you need to know about a Mips Instruction
struct MipsInstructionInfo
{
	char* InstName;
//	char InstAttributes;
};

// Mips registers
enum { R0 = 0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15, R16, R17, R18, R19, R20, R21, R22, R23, R24, R25, R26,
		R27, R28, R29, R30, R31 };


// **** Here are the InstAtributes, these can be ORed ****

// set if it is a 64-bit instruction
#define IS64BIT		1

// set if the instruction uses Rs
#define USESRS		2

// set if the instuction uses Rt
#define USESRT		4

// set if the instruction uses Rd
#define USESRD		8

#define USESRALL	( USESRS | USESRT | USESRD )

#define USESFT		16
#define USESFS		32
#define USESFD		64
#define USESFALL	( USESFT | USESFS | USESFD )


// **** This might help with the opcodes ****

#define OPCOP0		16
#define OPCOP1		17
#define OPCOP2		18

#define OPCOP1X		19

#define OPSPECIAL	0
#define OPSPECIAL2	28

#define OPREGIMM	1


#define RSBC		8


// for fmt
// .w = fmt=20,fmt3=4
// .s fmt=16,fmt3=0
// .d fmt=17,fmt3=1
//.L fmt=21,fmt3=5
//.ps fmt=22,fmt=6

// **** These will help with fmt and fmt3 fields ****

#define FMTS		16
#define FMTD		17
#define FMTW		20
#define FMTL		21
#define FMTPS		22

#define FMT3S		0
#define FMT3D		1
#define FMT3W		4
#define FMT3L		5
#define FMT3PS		6

// **** This should help with encoding instructions to test them **** //

#define SET_SPECIAL(inst)	( ( inst ) & 0x3f )
#define SET_SHIFT(inst)		( ( ( inst ) & 0x1f ) << 6 )
#define SET_RD(inst)		( ( ( inst ) & 0x1f ) << 11 )
#define SET_RT(inst)		( ( ( inst ) & 0x1f ) << 16 )
#define SET_RS(inst)		( ( ( inst ) & 0x1f ) << 21 )
#define SET_OPCODE(inst)	( ( ( inst ) & 0x3f ) << 26 )
#define SET_IMMED(inst)		( ( inst ) & 0xffff )
#define SET_ADDRESS(inst)	( ( inst ) & 0x3ffffff )
#define SET_BASE(inst)		( ( ( inst ) & 0x1f ) << 21 )
#define SET_HINT(inst)		( ( ( inst ) & 0x1f ) << 16 )
#define SET_REG(inst)		( ( ( inst ) & 0x1f ) << 1 )

#define SET_FD(inst)		( ( ( inst ) & 0x1f ) << 6 )
#define SET_FS(inst)		( ( ( inst ) & 0x1f ) << 11 )
#define SET_FT(inst)		( ( ( inst ) & 0x1f ) << 16 )
#define SET_FMT(inst)		( ( ( inst ) & 0x1f ) << 21 )

// This is the same as FMT
#define SET_RS(inst)		( ( ( inst ) & 0x1f ) << 21 )

// I'll define this as the next 5 bits after the opcode and "FMT/RS" on branch instructions
#define SET_RT(inst)		( ( ( inst ) & 0x1f ) << 16 )

// this will be the 5 bits like where the shift amount goes in shift instructions
#define SET_SF(inst)		( ( ( inst ) & 0x1f ) << 6 )

// this stuff is for vu
#define SET_DEST(inst)		( ( ( inst ) & 0xf ) << 21 )
#define SET_DESTX(inst)		( ( ( inst ) & 0x1 ) << 24 )
#define SET_DESTY(inst)		( ( ( inst ) & 0x1 ) << 23 )
#define SET_DESTZ(inst)		( ( ( inst ) & 0x1 ) << 22 )
#define SET_DESTW(inst)		( ( ( inst ) & 0x1 ) << 21 )
#define SET_IMM11(inst)		( ( ( inst ) & 0x7ff ) )
#define SET_IMM12(inst)		( ( ( ( inst ) & 0x800 ) << 10 ) | ( ( inst ) & 0x7ff ) )
#define SET_IMM15(inst)		( ( ( ( inst ) & 0x7800 ) << 10 ) | ( ( inst ) & 0x7ff ) )
#define SET_IMM24(inst)		( ( inst ) & 0xffffff )
#define SET_FSF(inst)		( ( ( inst ) & 0x3 ) << 21 )
#define SET_FTF(inst)		( ( ( inst ) & 0x3 ) << 23 )
#define SET_IT(inst)		( ( ( inst ) & 0x1f ) << 16 )
#define SET_IS(inst)		( ( ( inst ) & 0x1f ) << 11 )
#define SET_ID(inst)		( ( ( inst ) & 0x1f ) << 6 )
#define SET_IMM5(inst)		( ( ( inst ) & 0x1f ) << 6 )



// **** This should help with decoding instructions ****

#define GET_SPECIAL(inst)	( ( inst ) & 0x3f )
#define GET_SHIFT(inst)		( ( ( inst ) >> 6 ) & 0x1f )
#define GET_RD(inst)		( ( ( inst ) >> 11 ) & 0x1f )
#define GET_RT(inst)		( ( ( inst ) >> 16 ) & 0x1f )
#define GET_RS(inst)		( ( ( inst ) >> 21 ) & 0x1f )
#define GET_OPCODE(inst)	( ( ( inst ) >> 26 ) & 0x3f )
#define GET_IMMED(inst)		( ( inst ) & 0xffff )
#define GET_ADDRESS(inst)	( ( inst ) & 0x3ffffff )
#define GET_BASE(inst)		( ( ( inst ) >> 21 ) & 0x1f )
#define GET_HINT(inst)		( ( ( inst ) >> 16 ) & 0x1f )
#define GET_REG(inst)		( ( ( inst ) >> 1 ) & 0x1f )

#define GET_FD(inst)		( ( ( inst ) >> 6 ) & 0x1f )
#define GET_FS(inst)		( ( ( inst ) >> 11 ) & 0x1f )
#define GET_FT(inst)		( ( ( inst ) >> 16 ) & 0x1f )
#define GET_FMT(inst)		( ( ( inst ) >> 21 ) & 0x1f )

// This is the same as FMT
#define GET_RS(inst)		( ( ( inst ) >> 21 ) & 0x1f )

// I'll define this as the next 5 bits after the opcode and "FMT/RS" on branch instructions
#define GET_RT(inst)		( ( ( inst ) >> 16 ) & 0x1f )

// this will be the 5 bits like where the shift amount goes in shift instructions
#define GET_SF(inst)		( ( ( inst ) >> 6 ) & 0x1f )

// this stuff is for vu
#define GET_UPPER(inst)		( ( ( inst ) >> 32 ) & 0xffffffff )
#define GET_LOWER(inst)		( ( inst ) & 0xffffffff )
#define GET_DEST(inst)		( ( ( inst ) >> 21 ) & 0xf )
#define GET_DESTX(inst)		( ( ( inst ) >> 24 ) & 0x1 )
#define GET_DESTY(inst)		( ( ( inst ) >> 23 ) & 0x1 )
#define GET_DESTZ(inst)		( ( ( inst ) >> 22 ) & 0x1 )
#define GET_DESTW(inst)		( ( ( inst ) >> 21 ) & 0x1 )
#define GET_IMM11(inst)		( ( inst ) & 0x7ff )
#define GET_IMM12(inst)		( ( ( ( inst ) >> 10 ) & 0x800 ) | ( ( inst ) & 0x7ff ) )
#define GET_IMM15(inst)		( ( ( ( inst ) >> 10 ) & 0x7800 ) | ( ( inst ) & 0x7ff ) )
#define GET_IMM24(inst)		( ( inst ) & 0xffffff )
#define GET_FSF(inst)		( ( ( inst ) >> 21 ) & 0x3 )
#define GET_FTF(inst)		( ( ( inst ) >> 23 ) & 0x3 )
#define GET_IT(inst)		( ( ( inst >> 16 ) ) & 0x1f )
#define GET_IS(inst)		( ( ( inst >> 11 ) ) & 0x1f )
#define GET_ID(inst)		( ( ( inst >> 6 ) ) & 0x1f )
#define GET_IMM5(inst)		( ( ( inst >> 6 ) ) & 0x1f )

// **** This should helps with the opcodes ****
// "OP" means it is an opcode
// "SP" means it goes in the "Special" field

// **** "Special" opcodes **** (Mostly R-Type Instructions)
#define SPSLL			0
#define SPMOVCI			1
#define SPSRL			2
#define SPSRA			3
#define SPSLLV			4
#define SPSRLV			6
#define SPSRAV			7
#define SPJR			8
#define SPJALR			9
#define SPMOVZ			10
#define SPMOVN			11
#define SPSYSCALL		12
#define SPBREAK			13
#define SPSYNC			15
#define SPMFHI			16
#define SPMTHI			17
#define SPMFLO			18
#define SPMTLO			19
#define SPDSLLV			20
#define SPDSRLV			22
#define SPDSRAV			23
#define SPMULT			24
#define SPMULTU			25
#define SPDIV			26
#define SPDIVU			27
#define SPDMULT			28
#define SPDMULTU		29
#define SPDDIV			30
#define SPDDIVU			31
#define SPADD			32
#define SPADDU			33
#define SPSUB			34
#define SPSUBU			35
#define SPAND			36
#define SPOR			37
#define SPXOR			38
#define SPNOR			39
#define SPSLT			42
#define SPSLTU			43
#define SPDADD			44
#define SPDADDU			45
#define SPDSUB			46
#define	SPDSUBU			47
#define SPTGE			48
#define SPTGEU			49
#define SPTLT			50
#define SPTLTU			51
#define SPTEQ			52
#define SPTNE			54
#define	SPDSLL			56
#define SPDSRL			58
#define SPDSRA			59
#define SPDSLL32		60
#define SPDSRL32		62
#define SPDSRA32		63



// **** "Special2" opcodes ****

#define SPMADD			0
#define SPMADDU			1
#define SPMUL			2
#define SPMSUB			4
#define SPMSUBU			5
#define SPCLZ			32
#define SPCLO			33
#define SPDCLZ			36
#define SPDCLO			37
#define SPSDBBP			63



// **** COP1 codes **** (it's just a floating point co-processor)
// Note: For these instructions check the opcode then check fmt to determine what type of instruction it is
// it looks like you have to use the fmt value to determine what's going on
// these are the defines for Playstation 2

#define SPADDFMTS		0
#define SPSUBFMTS		1
#define SPMULFMTS		2
#define SPDIVFMTS		3
#define	SPSQRTFMTS		4
#define SPABSFMTS		5
#define SPMOVFMTS		6
#define SPNEGFMTS		7
#define SPRSQRTFMTS		22
#define SPADDAFMTS		24
#define SPSUBAFMTS		25
#define SPMULAFMTS		26
#define SPMADDFMTS		28
#define SPMSUBFMTS		29
#define SPMADDAFMTS		30
#define SPMSUBAFMTS		31
#define SPCVTSFMTW		32
#define SPCVTWFMTS		36
#define SPMAXFMTS		40
#define SPMINFMTS		41
#define SPCFFMTS		48
#define SPCEQFMTS		50
#define SPCLTFMTS		52
#define SPCLEFMTS		54


// COP1 codes not used by PS2 //

#define SPROUNDLFMT		8
#define SPTRUNCLFMT		9
#define SPCEILLFMT		10
#define SPFLOORLFMT		11
#define SPROUNDWFMT		12
#define SPTRUNCWFMT		13
#define SPCEILWFMT		14
#define	SPFLOORWFMT		15
#define SPMOVCF			17
#define SPMOVZFMT		18
#define SPMOVNFMT		19
#define SPRECIPFMT		21
#define	SPRSQRTFMT		22
#define SPCVTSFMTW		32
#define SPCVTSPU		32
#define SPCVTDFMT		33
#define SPCVTWFMTS		36
#define SPCVTLFMT		37
#define SPCVTPSS		38
#define SPCVTSPL		40
#define SPPLLPS			44
#define SPPLUPS			45
#define SPPULPS			46
#define SPPUUPS			47





#define RSMFC1			0
#define RSDMFC1			1
#define RSCFC1			2
#define RSMTC1			4
#define RSDMTC1			5
#define RSCTC1			6

#define RSBC0F			8
#define RSBC0FL			8
#define RSBC0T			8
#define RSBC0TL			8

#define RSBC1F			8
#define RSBC1FL			8
#define RSBC1T			8
#define RSBC1TL			8

#define RSBC2F			8
#define RSBC2FL			8
#define RSBC2T			8
#define RSBC2TL			8

#define RSS				16
#define RSW				20


#define RTBC0F			0
#define RTBC0FL			2
#define RTBC0T			1
#define RTBC0TL			3

#define RTBC1F			0
#define RTBC1FL			2
#define RTBC1T			1
#define RTBC1TL			3

#define RTBC2F			0
#define RTBC2FL			2
#define RTBC2T			1
#define RTBC2TL			3



// **** Immediate opcodes **** (Mostly I-Type instructions)

#define OPADDI			8
#define OPADDIU			9
#define OPSLTI			10
#define OPSLTIU			11
#define OPANDI			12
#define OPORI			13
#define OPXORI			14
#define OPLUI			15
#define OPDADDI			24
#define	OPDADDIU		25



// **** COP1X codes ****
/*
#define SPLWXC1			0
#define SPLDXC1			1
#define SPLUXC1			5
#define SPSWXC1			8
#define SPSDXC1			9
#define SPSUXC1			13
#define SPPREFX			15
#define SPALNVPS		30
#define SPMADDFMTS		32
#define SPMADDFMTD		33
#define SPMADDFMTPS		38
#define SPMSUBFMTS		40
#define SPMSUBFMTD		41
#define SPMSUBFMTPS		46
*/


// **** REGIMM codes ****

#define RTBLTZ			0
#define RTBGEZ			1
#define RTBLTZL			2
#define RTBGEZL			3
#define RTTGEI			8
#define RTTGEIU			9
#define RTTLTI			10
#define RTTLTIU			11
#define RTTEQI			12
#define RTTNEI			14
#define RTBLTZAL		16
#define RTBGEZAL		17
#define RTBLTZALL		18
#define RTBGEZALL		19



// **** REGIMM instructions ****

#define OPBGEZ			OPREGIMM
#define OPBGEZAL		OPREGIMM
#define OPBGEZALL		OPREGIMM
#define OPBGEZL			OPREGIMM
#define OPBLTZ			OPREGIMM
#define OPBLTZAL		OPREGIMM
#define OPBLTZALL		OPREGIMM
#define OPBLTZL			OPREGIMM
#define OPTEQI			OPREGIMM
#define OPTGEI			OPREGIMM
#define OPTGEIU			OPREGIMM
#define OPTLTI			OPREGIMM
#define OPTLTIU			OPREGIMM
#define OPTNEI			OPREGIMM


// **** "Special" Instructions ****

#define OPADD			OPSPECIAL
#define OPADDU			OPSPECIAL
#define OPAND			OPSPECIAL
#define OPBREAK			OPSPECIAL
#define OPDADD			OPSPECIAL
#define OPDADDU			OPSPECIAL
#define OPDDIV			OPSPECIAL
#define OPDDIVU			OPSPECIAL
#define OPDIV			OPSPECIAL
#define OPDIVU			OPSPECIAL
#define OPDMULT			OPSPECIAL
#define OPDMULTU		OPSPECIAL
#define OPDSLL			OPSPECIAL
#define OPDSLL32		OPSPECIAL
#define OPDSLLV			OPSPECIAL
#define OPDSRA			OPSPECIAL
#define OPDSRA32		OPSPECIAL
#define OPDSRAV			OPSPECIAL
#define OPDSRL			OPSPECIAL
#define OPDSRL32		OPSPECIAL
#define OPDSRLV			OPSPECIAL
#define OPDSUB			OPSPECIAL
#define OPDSUBU			OPSPECIAL
#define OPJALR			OPSPECIAL
#define OPJR			OPSPECIAL
#define OPMFHI			OPSPECIAL
#define OPMFLO			OPSPECIAL
#define OPMOVF			OPSPECIAL
#define OPMOVN			OPSPECIAL
#define OPMOVT			OPSPECIAL
#define OPMOVZ			OPSPECIAL
#define OPMTHI			OPSPECIAL
#define OPMTLO			OPSPECIAL
#define OPMULT			OPSPECIAL
#define OPMULTU			OPSPECIAL
#define OPNOP			OPSPECIAL
#define OPNOR			OPSPECIAL
#define OPOR			OPSPECIAL
#define OPSLL			OPSPECIAL
#define	OPSLLV			OPSPECIAL
#define OPSLT			OPSPECIAL
#define OPSLTU			OPSPECIAL
#define OPSRA			OPSPECIAL
#define OPSRAV			OPSPECIAL
#define OPSRL			OPSPECIAL
#define OPSRLV			OPSPECIAL
#define OPSSNOP			OPSPECIAL
#define OPSUB			OPSPECIAL
#define OPSUBU			OPSPECIAL
#define OPSYNC			OPSPECIAL
#define OPSYSCALL		OPSPECIAL
#define OPTEQ			OPSPECIAL
#define OPTGE			OPSPECIAL
#define OPTGEU			OPSPECIAL
#define OPTLT			OPSPECIAL
#define OPTLTU			OPSPECIAL
#define OPTNE			OPSPECIAL
#define OPXOR			OPSPECIAL



// **** "Special2" Instructions ****
/*
#define OPCLO			OPSPECIAL2
#define OPCLZ			OPSPECIAL2
#define OPDCLO			OPSPECIAL2
#define OPDCLZ			OPSPECIAL2
#define OPMADD			OPSPECIAL2
#define OPMADDU			OPSPECIAL2
#define OPMSUB			OPSPECIAL2
#define OPMSUBU			OPSPECIAL2
#define OPMUL			OPSPECIAL2
#define OPSDBBP			OPSPECIAL2
*/



// **** COP1 Instructions ****
// these include the ones for Playstation 2

#define OPABSFMTS		OPCOP1
#define OPADDFMTS		OPCOP1
#define OPBC1F			OPCOP1
#define OPBC1FL			OPCOP1
#define OPBC1T			OPCOP1
#define OPBC1TL			OPCOP1
#define OPCCONDFMT		OPCOP1
#define OPCFS			OPCOP1
#define OPCEQS			OPCOP1
#define OPCLTS			OPCOP1
#define OPCLES			OPCOP1
#define OPCEILLFMT		OPCOP1
#define OPCEILWFMT		OPCOP1
#define OPCFC1			OPCOP1
#define OPCTC1			OPCOP1
#define OPCVTDFMT		OPCOP1
#define OPCVTLFMT		OPCOP1
#define OPCVTPSS		OPCOP1
#define OPCVTSFMTW		OPCOP1
#define OPCVTSPL		OPCOP1
#define OPCVTSPU		OPCOP1
#define	OPCVTWFMTS		OPCOP1
#define OPDIVFMTS		OPCOP1
#define OPDMFC1			OPCOP1
#define OPDMTC1			OPCOP1
#define OPFLOORLFMT		OPCOP1
#define OPFLOORWFMT		OPCOP1
#define OPMFC1			OPCOP1
#define OPMOVFMTS		OPCOP1
#define OPMOVFFMT		OPCOP1
#define OPMOVNFMT		OPCOP1
#define OPMOVTFMT		OPCOP1
#define OPMOVZFMT		OPCOP1
#define OPMTC1			OPCOP1
#define OPMULFMTS		OPCOP1
#define OPNEGFMTS		OPCOP1
#define OPPLLPS			OPCOP1
#define OPPLUPS			OPCOP1
#define OPPULPS			OPCOP1
#define OPPUUPS			OPCOP1
#define OPRECIPFMT		OPCOP1
#define OPROUNDLFMT		OPCOP1
#define OPROUNDWFMT		OPCOP1
#define OPRSQRTFMTS		OPCOP1
#define OPSQRTFMTS		OPCOP1
#define OPSUBFMTS		OPCOP1
#define OPTRUNCLFMT		OPCOP1
#define OPTRUNCWFMT		OPCOP1

// * Playstation 2 specific Cop1 opcodes * //

#define OPADDAFMTS		OPCOP1
#define OPSUBFMTS		OPCOP1
#define OPMADDFMTS		OPCOP1
#define OPMSUBFMTS		OPCOP1
#define OPCVTWFMTS		OPCOP1
#define OPMAXFMTS		OPCOP1
#define OPMINFMTS		OPCOP1
#define OPCFFMTS		OPCOP1
#define OPCEQFMTS		OPCOP1
#define OPCLTFMTS		OPCOP1
#define OPCLEFMTS		OPCOP1



// **** COP1X Instructions ****
/*
#define OPALNVPS		OPCOP1X
#define OPLDXC1			OPCOP1X
#define OPLUXC1			OPCOP1X
#define OPLWXC1			OPCOP1X
#define OPMADDFMT		OPCOP1X
#define OPMSUBFMT		OPCOP1X
#define OPNMADDFMT		OPCOP1X
#define OPNSUBFMT		OPCOP1X
#define OPPREFX			OPCOP1X
#define OPSDXC1			OPCOP1X
#define OPSUXC1			OPCOP1X
#define OPSWXC1			OPCOP1X
*/


// **** Load Instructions ****

#define OPLB			32
#define OPLBU			36
#define OPLD			55
#define OPLDC1			53
#define OPLDC2			54
#define OPLDL			26
#define OPLDR			27
#define OPLH			33
#define OPLHU			37
#define OPLL			48
#define OPLLD			52
#define OPLW			35
#define OPLWC1			49
#define OPLWC2			50
#define OPLWL			34
#define OPLWR			38
#define OPLWU			39


// **** Store Instructions ****

#define OPSB			40
#define OPSC			56
#define OPSCD			60
#define OPSD			63
#define OPSDC1			61
#define OPSDC2			62
#define OPSDL			44
#define OPSDR			45
#define OPSH			41
#define OPSW			43
#define OPSWC1			57
#define OPSWC2			58
#define OPSWL			42
#define OPSWR			46


// **** COP0 codes ****

#define SPUSERSCODE		0
#define SPTLBR			1
#define SPTLBWI			2
#define SPTLBWR			6
#define SPTLBP			8
#define SPERET			24
#define SPDERET			31
#define SPWAIT			32

#define SPDI			57
#define SPEI			56

// * R3000A *
#define SPRFE			16


#define RSMFC0			0
#define	RSDMFC0			1
#define RSMTC0			4
#define RSDMTC0			5

#define RSCFC0			0
#define RSCTC0			4
#define RSCO			16

// * R3000A *
#define RSRFE			16


// **** COP0 Opcodes ****

#define OPBC0F			OPCOP0
#define OPBC0FL			OPCOP0
#define OPBC0T			OPCOP0
#define OPBC0TL			OPCOP0

#define OPDERET			OPCOP0
#define OPDMFC0			OPCOP0
#define OPDMTC0			OPCOP0
#define OPERET			OPCOP0
#define OPMFC0			OPCOP0
#define OPMTC0			OPCOP0
#define OPTLBP			OPCOP0
#define OPTLBR			OPCOP0
#define OPTLBWI			OPCOP0
#define OPTLBWR			OPCOP0
#define OPWAIT			OPCOP0

#define OPEI			OPCOP0
#define OPDI			OPCOP0
#define OPCFC0			OPCOP0
#define OPCTC0			OPCOP0


// * R3000A *
#define OPRFE			OPCOP0



// **** COP2 codes ****

#define RSMFC2			0
#define RSDMFC2			1
#define	RSCFC2			2
#define RSMTC2			4
#define RSDMTC2			5
#define RSCTC2			6
#define RSBC2F			8
#define RSBC2FL			8
#define RSBC2T			8
#define RSBC2TL			8
#define RSCOP2			16



// **** COP2 Opcodes ****

#define OPBC2F			OPCOP2
#define OPBC2FL			OPCOP2
#define OPBC2T			OPCOP2
#define OPBC2TL			OPCOP2

#define OPCFC2			OPCOP2
#define OPCTC2			OPCOP2
#define OPDMFC2			OPCOP2
#define OPDMTC2			OPCOP2
#define OPMFC2			OPCOP2
#define OPMTC2			OPCOP2


// **** Branch codes ****

#define RTBGTZ			0
#define RTBGTZL			0
#define RTBLEZ			0
#define RTBLEZL			0


// **** Branch Instruction Opcodes ****

#define OPBEQ			4
#define OPBNE			5
#define OPBLEZ			6
#define OPBGTZ			7
#define OPBEQL			20
#define OPBNEL			21
#define OPBLEZL			22
#define OPBGTZL			23


// **** Jump Instruction Opcodes ****

#define OPJ				2
#define OPJAL			3


// **** Other Opcodes ****

#define OPCACHE			47
#define OPPREF			51



//#define OPB				OPBEQ	// commented out since this is really BEQ R0, R0, offset
//#define OPBAL			OPREGIMM	// commented out since this is really BGEZAL r0, offset
//#define RSBAL			0
//#define RTBAL			RTBGEZAL
//#define OPBEQ			OPBEQ	// commenting out since this was already defined
//#define OPCOP2			OPCOP2
#define SPMOVF			SPMOVCI
#define SPMOVFFMT		SPMOVCF
#define SPMOVT			SPMOVCI
#define SPMOVTFMT		SPMOVCF
#define SPNOP			SPSLL
#define SPSSNOP			SPSLL



// **** MMI Opcodes **** //

// this seems to be the same as Special2
#define OPMMI			28

#define OPDIV1			OPMMI
#define OPDIVU1			OPMMI
#define OPMADD			OPMMI
#define OPMADD1			OPMMI
#define OPMADDU			OPMMI
#define OPMADDU1		OPMMI
#define OPMFHI1			OPMMI
#define OPMFLO1			OPMMI
#define OPMTHI1			OPMMI
#define OPMTLO1			OPMMI
#define OPMULT1			OPMMI
#define OPMULTU1		OPMMI
#define OPPABSH			OPMMI
#define OPPLZCW			OPMMI
#define OPPMFHLFMT		OPMMI
#define OPPMTHL			OPMMI
#define OPPSLLH			OPMMI
#define OPPSRLH			OPMMI
#define OPPSRAH			OPMMI
#define OPPSLLW			OPMMI
#define OPPSRLW			OPMMI
#define OPPSRAW			OPMMI

// **** Opcode MMI "Special" (actually "Special2") codes **** //

#define SPMADD			0
#define SPMADDU			1
#define SPPLZCW			4
#define SPMMI0			8
#define SPMMI2			9
#define SPMFHI1			16
#define SPMTHI1			17	/* This might be wrong, will find out during "testing" */
#define SPMTLO1			17	/* This might be wrong, will find out during "testing" */
#define SPMULT1			24
#define SPMULTU1		25
#define SPDIV1			26
#define SPDIVU1			27
#define SPMFLO1			28
#define SPMADD1			32
#define SPMADDU1		33
#define SPMMI1			40
#define SPMMI3			41
#define SPPMFHLFMT		48
#define SPPMTHL			49
#define SPPSLLH			52
#define SPPSRLH			54
#define SPPSRAH			55
#define SPPSLLW			60
#define SPPSRLW			62
#define SPPSRAW			63



// **** MMI0 Opcodes **** //

#define OPPADDB			OPMMI
#define OPPADDH			OPMMI
#define OPPADDSB		OPMMI
#define OPPADDSH		OPMMI
#define OPPADDSW		OPMMI
#define OPPADDW			OPMMI
#define OPPCGTB			OPMMI
#define OPPCGTH			OPMMI
#define OPPCGTW			OPMMI
#define OPPEXT5			OPMMI
#define OPPEXTLB		OPMMI
#define OPPEXTLH		OPMMI
#define OPPEXTLW		OPMMI
#define OPPMAXH			OPMMI
#define OPPMAXW			OPMMI
#define OPPPAC5			OPMMI
#define OPPPACB			OPMMI
#define OPPPACH			OPMMI
#define OPPPACW			OPMMI
#define OPPSUBB			OPMMI
#define OPPSUBH			OPMMI
#define OPPSUBSB		OPMMI
#define OPPSUBSH		OPMMI
#define OPPSUBSW		OPMMI
#define OPPSUBW			OPMMI

// **** MMI1 Opcodes **** //

#define OPPABSH			OPMMI
#define OPPABSW			OPMMI
#define OPPADDUB		OPMMI
#define OPPADDUH		OPMMI
#define OPPADDUW		OPMMI
#define OPPADSBH		OPMMI
#define OPPCEQB			OPMMI
#define OPPCEQH			OPMMI
#define OPPCEQW			OPMMI
#define OPPEXTUB		OPMMI
#define OPPEXTUH		OPMMI
#define OPPEXTUW		OPMMI
#define OPPMINH			OPMMI
#define OPPMINW			OPMMI
#define OPPSUBUB		OPMMI
#define OPPSUBUH		OPMMI
#define OPPSUBUW		OPMMI
#define OPQFSRV			OPMMI

// **** MMI2 Opcodes **** //

#define OPPAND			OPMMI
#define OPPCPYLD		OPMMI
#define OPPDIVBW		OPMMI
#define OPPDIVW			OPMMI
#define OPPEXEH			OPMMI
#define OPPEXEW			OPMMI
#define OPPHMADH		OPMMI
#define OPPHMSBH		OPMMI
#define OPPINTH			OPMMI
#define OPPMADDH		OPMMI
#define OPPMADDW		OPMMI
#define OPPMFHI			OPMMI
#define OPPMFLO			OPMMI
#define OPPMSUBH		OPMMI
#define OPPMSUBW		OPMMI
#define OPPMULTH		OPMMI
#define OPPMULTW		OPMMI
#define OPPREVH			OPMMI
#define OPPROT3W		OPMMI
#define OPPSLLVW		OPMMI
#define OPPSRLVW		OPMMI
#define OPPXOR			OPMMI

// **** MMI3 Opcodes **** //

#define OPPCPYH			OPMMI
#define OPPCPYUD		OPMMI
#define OPPDIVUW		OPMMI
#define OPPEXCH			OPMMI
#define OPPEXCW			OPMMI
#define OPPINTEH		OPMMI
#define OPPMADDUW		OPMMI
#define OPPMTHI			OPMMI
#define OPPMTLO			OPMMI
#define OPPMULTUW		OPMMI
#define OPPNOR			OPMMI
#define OPPOR			OPMMI
#define OPPSRAVW		OPMMI


// **** MMI0 "Special" codes ****


#define SPPADDB			SPMMI0
#define SPPADDH			SPMMI0
#define SPPADDSB		SPMMI0
#define SPPADDSH		SPMMI0
#define SPPADDSW		SPMMI0
#define SPPADDW			SPMMI0
#define SPPCGTB			SPMMI0
#define SPPCGTH			SPMMI0
#define SPPCGTW			SPMMI0
#define SPPEXT5			SPMMI0
#define SPPEXTLB		SPMMI0
#define SPPEXTLH		SPMMI0
#define SPPEXTLW		SPMMI0
#define SPPMAXH			SPMMI0
#define SPPMAXW			SPMMI0
#define SPPPAC5			SPMMI0
#define SPPPACB			SPMMI0
#define SPPPACH			SPMMI0
#define SPPPACW			SPMMI0
#define SPPSUBB			SPMMI0
#define SPPSUBH			SPMMI0
#define SPPSUBSB		SPMMI0
#define SPPSUBSH		SPMMI0
#define SPPSUBSW		SPMMI0
#define SPPSUBW			SPMMI0



// **** MMI1 "Special" codes ****


#define SPPABSH			SPMMI1
#define SPPABSW			SPMMI1
#define SPPADDUB		SPMMI1
#define SPPADDUH		SPMMI1
#define SPPADDUW		SPMMI1
#define SPPADSBH		SPMMI1
#define SPPCEQB			SPMMI1
#define SPPCEQH			SPMMI1
#define SPPCEQW			SPMMI1
#define SPPEXTUB		SPMMI1
#define SPPEXTUH		SPMMI1
#define SPPEXTUW		SPMMI1
#define SPPMINH			SPMMI1
#define SPPMINW			SPMMI1
#define SPPSUBUB		SPMMI1
#define SPPSUBUH		SPMMI1
#define SPPSUBUW		SPMMI1
#define SPQFSRV			SPMMI1



// **** MMI2 "Special" codes ****


#define SPPAND			SPMMI2
#define SPPCPYLD		SPMMI2
#define SPPDIVBW		SPMMI2
#define SPPDIVW			SPMMI2
#define SPPEXEH			SPMMI2
#define SPPEXEW			SPMMI2
#define SPPHMADH		SPMMI2
#define SPPHMSBH		SPMMI2
#define SPPINTH			SPMMI2
#define SPPMADDH		SPMMI2
#define SPPMADDW		SPMMI2
#define SPPMFHI			SPMMI2
#define SPPMFLO			SPMMI2
#define SPPMSUBH		SPMMI2
#define SPPMSUBW		SPMMI2
#define SPPMULTH		SPMMI2
#define SPPMULTW		SPMMI2
#define SPPREVH			SPMMI2
#define SPPROT3W		SPMMI2
#define SPPSLLVW		SPMMI2
#define SPPSRLVW		SPMMI2
#define SPPXOR			SPMMI2



// **** MMI3 "Special" codes ****


#define SPPCPYH			SPMMI3
#define SPPCPYUD		SPMMI3
#define SPPDIVUW		SPMMI3
#define SPPEXCH			SPMMI3
#define SPPEXCW			SPMMI3
#define SPPINTEH		SPMMI3
#define SPPMADDUW		SPMMI3
#define SPPMTHI			SPMMI3
#define SPPMTLO			SPMMI3
#define SPPMULTUW		SPMMI3
#define SPPNOR			SPMMI3
#define SPPOR			SPMMI3
#define SPPSRAVW		SPMMI3



// **** MMI0 SF codes **** //

#define SFPADDW			0
#define SFPSUBW			1
#define SFPCGTW			2
#define SFPMAXW			3
#define SFPADDH			4
#define SFPSUBH			5
#define SFPCGTH			6
#define SFPMAXH			7
#define SFPADDB			8
#define SFPSUBB			9
#define SFPCGTB			10
#define SFPADDSW		16
#define SFPSUBSW		17
#define SFPEXTLW		18
#define SFPPACW			19
#define SFPADDSH		20
#define SFPSUBSH		21
#define SFPEXTLH		22
#define SFPPACH			23
#define SFPADDSB		24
#define SFPSUBSB		25
#define SFPEXTLB		26
#define SFPPACB			27
#define SFPEXT5			30
#define SFPPAC5			31



// **** MMI1 SF codes **** //

#define SFPABSW			1
#define SFPCEQW			2
#define SFPMINW			3
#define SFPADSBH		4
#define SFPABSH			5
#define SFPCEQH			6
#define SFPMINH			7
#define SFPCEQB			10
#define SFPADDUW		16
#define SFPSUBUW		17
#define SFPEXTUW		18
#define SFPADDUH		20
#define SFPSUBUH		21
#define SFPEXTUH		22
#define SFPADDUB		24
#define SFPSUBUB		25
#define SFPEXTUB		26
#define SFQFSRV			27



// **** MMI2 SF codes **** //

#define SFPMADDW		0
#define SFPSLLVW		2
#define SFPSRLVW		3
#define SFPMSUBW		4
#define SFPMFHI			8
#define SFPMFLO			9
#define SFPINTH			10
#define SFPMULTW		12
#define SFPDIVW			13
#define SFPCPYLD		14
#define SFPMADDH		16
#define SFPHMADH		17
#define SFPAND			18
#define SFPXOR			19
#define SFPMSUBH		20
#define SFPHMSBH		21
#define SFPEXEH			26
#define SFPREVH			27
#define SFPMULTH		28
#define SFPDIVBW		29
#define SFPEXEW			30
#define SFPROT3W		31




// **** MMI3 SF codes **** //

#define SFPMADDUW		0
#define SFPSRAVW		3
#define SFPMTHI			8
#define SFPMTLO			9
#define SFPINTEH		10
#define SFPMULTUW		12
#define SFPDIVUW		13
#define SFPCPYUD		14
#define SFPOR			18
#define SFPNOR			19
#define SFPEXCH			26
#define SFPCPYH			27
#define SFPEXCW			30







// **** Playstation 2 specific COP1 Special codes ****

#define SPADDAFMTS		24
#define SPSUBAFMTS		25
#define SPMADDFMTS		28
#define SPMSUBFMTS		29
#define SPMADDAFMTS		30
#define SPMAXFMTS		40
#define SPMINFMTS		41
#define SPCFFMTS		48
#define SPCEQFMTS		50
#define SPCLTFMTS		52
#define SPCLEFMTS		54



// **** Other Playstation 2 specific opcodes ****

#define OPLQ			30
#define OPSQ			31
#define OPMFSA			OPSPECIAL
#define OPMTSA			OPSPECIAL
#define OPMTSAB			OPREGIMM
#define OPMTSAH			OPREGIMM

// **** Other Playstation 2 specific "Special" codes (check just above to see the opcodes are really "Special") ****

#define SPMFSA			40
#define SPMTSA			41

// **** Other Playstation 2 specific codes ****

#define RTMTSAB			24
#define RTMTSAH			25


// **** VU0 Macro Mode Instructions with their own opcodes **** //

#define OPLQC2			54
#define OPSQC2			62


// **** VU0 Macro Mode Opcodes **** //

#define OPQMFC2			OPCOP2
#define OPQMTC2			OPCOP2
#define OPVABS			OPCOP2
#define OPVADD			OPCOP2
#define	OPVADDI			OPCOP2
#define OPVADDQ			OPCOP2
#define OPVADDBC		OPCOP2
#define OPVADDA			OPCOP2
#define OPVADDAI		OPCOP2
#define OPVADDAQ		OPCOP2
#define OPVADDABC		OPCOP2
#define OPVCALLMS		OPCOP2
#define OPVCALLMSR		OPCOP2
#define OPVCLIP			OPCOP2
#define OPVDIV			OPCOP2
#define OPVFTOI0		OPCOP2
#define OPVFTOI4		OPCOP2
#define OPVFTOI12		OPCOP2
#define OPVFTOI15		OPCOP2
#define OPVIADD			OPCOP2
#define OPVIADDI		OPCOP2
#define OPVIAND			OPCOP2
#define OPVILWR			OPCOP2
#define OPVIOR			OPCOP2
#define OPVISUB			OPCOP2
#define OPVISWR			OPCOP2
#define OPVITOF0		OPCOP2
#define OPVITOF4		OPCOP2
#define OPVITOF12		OPCOP2
#define OPVITOF15		OPCOP2
#define OPVLQD			OPCOP2
#define OPVLQI			OPCOP2
#define OPVMADD			OPCOP2
#define OPVMADDI		OPCOP2
#define OPVMADDQ		OPCOP2
#define OPVMADDBC		OPCOP2
#define OPVMADDA		OPCOP2
#define OPVMADDAI		OPCOP2
#define OPVMADDAQ		OPCOP2
#define OPVMADDABC		OPCOP2
#define OPVMAX			OPCOP2
#define OPVMAXI			OPCOP2
#define OPVMAXBC		OPCOP2
#define OPVMFIR			OPCOP2
#define OPVMINI			OPCOP2
#define OPVMINII		OPCOP2
#define OPVMINIBC		OPCOP2
#define OPVMOVE			OPCOP2
#define OPVMR32			OPCOP2
#define OPVMSUB			OPCOP2
#define OPVMSUBI		OPCOP2
#define OPVMSUBQ		OPCOP2
#define OPVMSUBBC		OPCOP2
#define OPVMSUBA		OPCOP2
#define OPVMSUBAI		OPCOP2
#define OPVMSUBAQ		OPCOP2
#define OPVMSUBABC		OPCOP2
#define OPVMTIR			OPCOP2
#define OPVMUL			OPCOP2
#define OPVMULI			OPCOP2
#define OPVMULQ			OPCOP2
#define OPVMULBC		OPCOP2
#define OPVMULA			OPCOP2
#define OPVMULAI		OPCOP2
#define OPVMULAQ		OPCOP2
#define OPVMULABC		OPCOP2
#define OPVNOP			OPCOP2
#define OPVOPMULA		OPCOP2
#define OPVOPMSUM		OPCOP2
#define OPVRGET			OPCOP2
#define OPVRINIT		OPCOP2
#define OPVRNEXT		OPCOP2
#define OPVRSQRT		OPCOP2
#define OPVRXOR			OPCOP2
#define OPVSQD			OPCOP2
#define OPVSQI			OPCOP2
#define OPVSQRT			OPCOP2
#define OPVSUB			OPCOP2
#define OPVSUBI			OPCOP2
#define OPVSUBQ			OPCOP2
#define OPVSUBBC		OPCOP2
#define OPVSUBA			OPCOP2
#define OPVSUBAI		OPCOP2
#define OPVSUBAQ		OPCOP2
#define OPVSUBABC		OPCOP2
#define OPVWAITQ		OPCOP2
#define OPVADDBCX		OPCOP2
#define OPVADDBCY		OPCOP2
#define OPVADDBCZ		OPCOP2
#define OPVADDBCW		OPCOP2
#define OPVSUBBCX		OPCOP2
#define OPVSUBBCY		OPCOP2
#define OPVSUBBCZ		OPCOP2
#define OPVSUBBCW		OPCOP2
#define OPVMADDBCX		OPCOP2
#define OPVMADDBCY		OPCOP2
#define OPVMADDBCZ		OPCOP2
#define OPVMADDBCW		OPCOP2
#define OPVMSUBBCX		OPCOP2
#define OPVMSUBBCY		OPCOP2
#define OPVMSUBBCZ		OPCOP2
#define OPVMSUBBCW		OPCOP2
#define OPVMAXBCX		OPCOP2
#define OPVMAXBCY		OPCOP2
#define OPVMAXBCZ		OPCOP2
#define OPVMAXBCW		OPCOP2
#define OPVMINIBCX		OPCOP2
#define OPVMINIBCY		OPCOP2
#define OPVMINIBCZ		OPCOP2
#define OPVMINIBCW		OPCOP2
#define OPVMULBCX		OPCOP2
#define OPVMULBCY		OPCOP2
#define OPVMULBCZ		OPCOP2
#define OPVMULBCW		OPCOP2



// **** VUO Macro Mode "Special" codes **** //

// I need to call these 4 something.. anything
#define SPVU60			60
#define SPVU61			61
#define SPVU62			62
#define SPVU63			63

#define SPVADDBCX		0
#define SPVADDBCY		1
#define SPVADDBCZ		2
#define SPVADDBCW		3
#define SPVSUBBCX		4
#define SPVSUBBCY		5
#define SPVSUBBCZ		6
#define SPVSUBBCW		7
#define SPVMADDBCX		8
#define SPVMADDBCY		9
#define SPVMADDBCZ		10
#define SPVMADDBCW		11
#define SPVMSUBBCX		12
#define SPVMSUBBCY		13
#define SPVMSUBBCZ		14
#define SPVMSUBBCW		15
#define SPVMAXBCX		16
#define SPVMAXBCY		17
#define SPVMAXBCZ		18
#define SPVMAXBCW		19
#define SPVMINIBCX		20
#define SPVMINIBCY		21
#define SPVMINIBCZ		22
#define SPVMINIBCW		23
#define SPVMULBCX		24
#define SPVMULBCY		25
#define SPVMULBCZ		26
#define SPVMULBCW		27
#define SPVMULQ			28
#define SPVMAXI			29
#define SPVMULI			30
#define SPVMINII		31
#define SPVADDQ			32
#define SPVMADDQ		33
#define SPVADDI			34
#define SPVMADDI		35
#define SPVSUBQ			36
#define SPVMSUBQ		37
#define SPVSUBI			38
#define SPVMSUBI		39
#define SPVADD			40
#define SPVMADD			41
#define SPVMUL			42
#define SPVMAX			43
#define SPVSUB			44
#define SPVMSUB			45
#define SPVOPMSUM		46
#define SPVMINI			47
#define SPVIADD			48
#define SPVISUB			49
#define SPVIADDI		50
#define SPVIAND			52
#define SPVIOR			53
#define SPVCALLMS		56
#define SPVCALLMSR		57
#define SPVADDA			60
#define SPVDIV			60
#define SPVFTOI0		60
#define SPVADDAQ		60
#define SPVADDABCX		60
#define SPVITOF0		60
#define SPVLQI			60
#define SPVMOVE			60
#define SPVMSUBABCX		60
#define SPVMTIR			60
#define SPVMULAQ		60
#define SPVMULABCX		60
#define SPVRNEXT		60
#define SPVSUBA			60
#define SPVSUBAQ		60
#define SPVSUBABCX		60
#define SPVMADDABCX		60
#define SPVADDABCY		61
#define SPVFTOI4		61
#define SPVITOF4		61
#define SPVMADDA		61
#define SPVMADDAQ		61
#define SPVMADDABCY		61
#define SPVABS			61
#define SPVMR32			61
#define SPVMSUBA		61
#define SPVMSUBAQ		61
#define SPVMSUBABCY		61
#define SPVMULABCY		61
#define SPVRGET			61
#define SPVSQI			61
#define SPVSQRT			61
#define SPVSUBABCY		61
#define SPVMFIR			61
#define SPVADDAI		62
#define SPVADDABCZ		62
#define SPVFTOI12		62
#define SPVILWR			62
#define SPVITOF12		62
#define SPVLQD			62
#define SPVMADDABCZ		62
#define SPVMSUBABCZ		62
#define SPVMULA			62
#define SPVMULAI		62
#define SPVMULABCZ		62
#define SPVOPMULA		62
#define SPVRINIT		62
#define SPVRSQRT		62
#define SPVSUBAI		62
#define SPVSUBABCZ		62
#define SPVADDABCW		63
#define SPVCLIP			63
#define SPVFTOI15		63
#define SPVISWR			63
#define SPVITOF15		63
#define SPVMADDAI		63
#define SPVMADDABCW		63
#define SPVMSUBAI		63
#define SPVMSUBABCW		63
#define SPVMULABCW		63
#define SPVNOP			63
#define SPVRXOR			63
#define SPVSQD			63
#define SPVSUBABCW		63
#define SPVWAITQ		63



// **** VU0 Macro Mode RS codes **** //
// if high bit (CO) is set then use Special code, then SF if needed

#define RSQMFC2			1
#define RSQMTC2			5


// **** VU0 Macro Mode SF codes ****//

// codes where special = 60

#define SFVITOF0		4
#define SFVFTOI0		5
#define SFVMULAQ		7
#define SFVADDAQ		8
#define SFVSUBAQ		9
#define SFVADDA			10
#define SFVSUBA			11
#define SFVMOVE			12
#define SFVLQI			13
#define SFVDIV			14
#define SFVMTIR			15
#define SFVRNEXT		16



// codes where special = 61

#define SFVITOF4		4
#define SFVFTOI4		5
#define SFVABS			7
#define SFVMADDAQ		8
#define SFVMSUBAQ		9
#define SFVMADDA		10
#define SFVMSUBA		11
#define SFVMR32			12
#define SFVSQI			13
#define SFVSQRT			14
#define SFVMFIR			15
#define SFVRGET			16



// codes where special = 62

#define SFVADDABC		0
#define SFVSUBABC		1
#define SFVMADDABC		2
#define SFVMSUBABC		3
#define SFVITOF12		4
#define SFVFTOI12		5
#define SFVMULABC		6
#define SFVMULAI		7
#define SFVADDAI		8
#define SFVSUBAI		9
#define SFVMULA			10
#define SFVOPMULA		11
#define SFVLQD			13
#define SFVRSQRT		14
#define SFVILWR			15
#define SFVRINIT		16



// codes where special = 63

#define SFVITOF15		4
#define SFVFTOI15		5
#define SFVCLIP			7
#define SFVMADDAI		8
#define SFVMSUBAI		9
#define SFVNOP			11
#define SFVSQD			13
#define SFVWAITQ		14
#define SFVISWR			15
#define SFVRXOR			16


///////////////////////////////////
// **** Instruction Formats **** //
///////////////////////////////////

//op rd, rs, rt
#define FORMAT1			0

//op rt, rs, immediate
#define FORMAT2			1

//op rs, rt, offset
#define FORMAT3			2

//op rs, offset
#define FORMAT4			3

//op rs, rt
#define FORMAT5			4

//op rd, rt, sa
#define FORMAT6			5

//op rd, rt, rs
#define FORMAT7			6

//op target
#define FORMAT8			7

//op rd, rs
#define FORMAT9			8

//op rs
#define FORMAT10		9

//op rt, offset (base)
#define FORMAT11		10

//op rt, immediate
#define FORMAT12		11

//op rd
#define FORMAT13		12

//op hint, offset (base)
#define FORMAT14		13

//op rd, rt
#define FORMAT15		14

//op
#define FORMAT16		15

//op rt
#define FORMAT17		16

//op rt, reg
#define FORMAT18		17

//op rt, rd
#define FORMAT19		18

//op fd, fs
#define FORMAT20		19

//op fd, fs, ft
#define FORMAT21		20

//op fs, ft
#define FORMAT22		21

//op offset
#define FORMAT23		22

//op ft, fs
#define FORMAT24		23

//op fd, ft
#define FORMAT25		24

//op rt, fs
#define FORMAT44		43

//op fs, rt
#define FORMAT45		44

// ** Instruction formats below are for VU ** //

//op.dest ft, fs
#define FORMAT26		25

//op.dest fd, fs, ft
#define FORMAT27		26

//op.dest fd, fs
#define FORMAT28		27

//op.dest fs, ft
#define FORMAT29		28

//op.dest fs
#define FORMAT30		29

//op Imm15
#define FORMAT31		30

//op fsfsf, ftftf
#define FORMAT32		31

//op id, is, it
#define FORMAT33		32

//op it, is, Imm5
#define FORMAT34		33

//op.dest it, (is)
#define FORMAT35		34

//op fsfsf
#define FORMAT36		35

//op ft, offset (base)
#define FORMAT37		36

//op ftftf
#define FORMAT38		37

//op.dest ft
#define FORMAT39		38

//op.dest fs, (it)
#define FORMAT40		39

//op it, fsfsf
#define FORMAT41		40

//op.dest ft, is
#define FORMAT42		41

//op fs, ft
#define FORMAT43		42

#define FORMAT46		45
#define FORMAT47		46
#define FORMAT48		47
#define FORMAT49		48
#define FORMAT50		49
#define FORMAT51		50
#define FORMAT52		51
#define FORMAT53		52
#define FORMAT54		53
#define FORMAT55		54
#define FORMAT56		55
#define FORMAT57		56



// **** List of Instructions (and their formats) **** //

#define FTADD			FORMAT1
#define FTADDI			FORMAT2
#define FTADDIU			FORMAT2
#define FTADDU			FORMAT1
#define FTAND			FORMAT1
#define FTANDI			FORMAT2
#define FTBEQ			FORMAT3
#define FTBEQL			FORMAT3
#define FTBGEZ			FORMAT4
#define FTBGEZAL		FORMAT4
#define FTBGEZALL		FORMAT4
#define FTBGEZL			FORMAT4
#define FTBGTZ			FORMAT4
#define FTBGTZL			FORMAT4
#define FTBLEZ			FORMAT4
#define FTBLEZL			FORMAT4
#define FTBLTZ			FORMAT4
#define FTBLTZAL		FORMAT4
#define FTBLTZALL		FORMAT4
#define FTBLTZL			FORMAT4
#define FTBNE			FORMAT3
#define FTBNEL			FORMAT3
#define FTBREAK			FORMAT16
#define FTDADD			FORMAT1
#define FTDADDI			FORMAT2
#define FTDADDIU		FORMAT2
#define FTDADDU			FORMAT1
#define FTDIV			FORMAT5
#define FTDIVU			FORMAT5
#define FTDSLL			FORMAT6
#define FTDSLL32		FORMAT6
#define FTDSLLV			FORMAT7
#define FTDSRA			FORMAT6
#define FTDSRA32		FORMAT6
#define FTDSRAV			FORMAT7
#define FTDSRL			FORMAT6
#define FTDSRL32		FORMAT6
#define FTDSRLV			FORMAT7
#define FTDSUB			FORMAT1
#define FTDSUBU			FORMAT1
#define FTJ				FORMAT8
#define FTJAL			FORMAT8
#define FTJALR			FORMAT9
#define FTJR			FORMAT10
#define FTLB			FORMAT11
#define FTLBU			FORMAT11
#define FTLD			FORMAT11
#define FTLDL			FORMAT11
#define FTLDR			FORMAT11
#define FTLH			FORMAT11
#define FTLHU			FORMAT11
#define FTLUI			FORMAT12
#define FTLW			FORMAT11
#define FTLWL			FORMAT11
#define FTLWR			FORMAT11
#define FTLWU			FORMAT11
#define FTMFHI			FORMAT13
#define FTMFLO			FORMAT13
#define FTMOVN			FORMAT1
#define FTMOVZ			FORMAT1
#define FTMTHI			FORMAT10
#define FTMTLO			FORMAT10
#define FTMULT			FORMAT5
#define FTMULTU			FORMAT5
#define FTNOR			FORMAT1
#define FTOR			FORMAT1
#define FTORI			FORMAT2
#define FTPREF			FORMAT14
#define FTSB			FORMAT11
#define FTSD			FORMAT11
#define FTSDL			FORMAT11
#define FTSDR			FORMAT11
#define FTSH			FORMAT11
#define FTSLL			FORMAT6
#define FTSLLV			FORMAT7
#define FTSLT			FORMAT1
#define FTSLTI			FORMAT2
#define FTSLTIU			FORMAT2
#define FTSLTU			FORMAT1
#define FTSRA			FORMAT6
#define FTSRAV			FORMAT7
#define FTSRL			FORMAT6
#define FTSRLV			FORMAT7
#define FTSUB			FORMAT1
#define FTSUBU			FORMAT1
#define FTSW			FORMAT11
#define FTSWL			FORMAT11
#define FTSWR			FORMAT11
#define FTSYNCSTYPE		FORMAT16
#define FTSYSCALL		FORMAT16
#define FTTEQ			FORMAT5
#define FTTEQI			FORMAT4
#define FTTGE			FORMAT5
#define FTTGEI			FORMAT4
#define FTTGEIU			FORMAT4
#define FTTGEU			FORMAT5
#define FTTLT			FORMAT5
#define FTTLTI			FORMAT4
#define FTTLTIU			FORMAT4
#define FTTLTU			FORMAT5
#define FTTNE			FORMAT5
#define FTTNEI			FORMAT4
#define FTXOR			FORMAT1
#define FTXORI			FORMAT2

// ** ps2 mips specific instructions ** //

#define FTDIV1			FORMAT5
#define FTDIVU1			FORMAT5
#define FTLQ			FORMAT11
#define FTMADD			FORMAT1
#define FTMADD1			FORMAT1
#define FTMADDU			FORMAT1
#define FTMADDU1		FORMAT1
#define FTMFHI1			FORMAT13
#define FTMFLO1			FORMAT13
#define FTMFSA			FORMAT13
#define FTMTHI1			FORMAT10
#define FTMTLO1			FORMAT10
#define FTMTSA			FORMAT10
#define FTMTSAB			FORMAT4
#define FTMTSAH			FORMAT4
#define FTMULT3			FORMAT1
#define FTMULT1			FORMAT1
#define FTMULTU3		FORMAT1
#define FTMULTU1		FORMAT1
#define FTPABSH			FORMAT15
#define FTPABSW			FORMAT15
#define FTPADDB			FORMAT1
#define FTPADDH			FORMAT1
#define FTPADDSB		FORMAT1
#define FTPADDSH		FORMAT1
#define FTPADDSW		FORMAT1
#define FTPADDUB		FORMAT1
#define FTPADDUH		FORMAT1
#define FTPADDUW		FORMAT1
#define FTPADDW			FORMAT1
#define FTPADSBH		FORMAT1
#define FTPAND			FORMAT1
#define FTPCEQB			FORMAT1
#define FTPCEQH			FORMAT1
#define FTPCEQW			FORMAT1
#define FTPCGTB			FORMAT1
#define FTPCGTH			FORMAT1
#define FTPCGTW			FORMAT1
#define FTPCPYH			FORMAT15
#define FTPCPYLD		FORMAT1
#define FTPCPYUD		FORMAT1
#define FTPDIVBW		FORMAT5
#define FTPDIVUW		FORMAT5
#define FTPDIVW			FORMAT5
#define FTPEXCH			FORMAT15
#define FTPEXCW			FORMAT15
#define FTPEXEH			FORMAT15
#define FTPEXEW			FORMAT15
#define FTPEXT5			FORMAT15
#define FTPEXTLB		FORMAT1
#define FTPEXTLH		FORMAT1
#define FTPEXTLW		FORMAT1
#define FTPEXTUB		FORMAT1
#define FTPEXTUH		FORMAT1
#define FTPEXTUW		FORMAT1
#define FTPHMADH		FORMAT1
#define FTPHMSBH		FORMAT1
#define FTPINTEH		FORMAT1
#define FTPINTH			FORMAT1
#define FTPLZCW			FORMAT9
#define FTPMADDH		FORMAT1
#define FTPMADDUW		FORMAT1
#define FTPMADDW		FORMAT1
#define FTPMAXH			FORMAT1
#define FTPMAXW			FORMAT1
#define FTPMFHI			FORMAT13
#define FTPMFHL			FORMAT13
#define FTPMFHLLH		FORMAT13
#define FTPMFHLLW		FORMAT13
#define FTPMFHLSH		FORMAT13
#define FTPMFHLSLW		FORMAT13
#define FTPMFHLUW		FORMAT13
#define FTPMFLO			FORMAT13
#define FTPMINH			FORMAT1
#define FTPMINW			FORMAT1
#define FTPMSUBH		FORMAT1
#define FTPMSUBW		FORMAT1
#define FTPMTHI			FORMAT10
#define FTPMTHLLW		FORMAT10
#define FTPMTLO			FORMAT10
#define FTPMULTH		FORMAT1
#define FTPMULTUW		FORMAT1
#define FTPMULTW		FORMAT1
#define FTPNOR			FORMAT1
#define FTPOR			FORMAT1
#define FTPPAC5			FORMAT15
#define FTPPACB			FORMAT1
#define FTPPACH			FORMAT1
#define FTPPACW			FORMAT1
#define FTPREVH			FORMAT15
#define FTPROT3W		FORMAT15
#define FTPSLLH			FORMAT6
#define FTPSLLVW		FORMAT7
#define FTPSLLW			FORMAT6
#define FTPSRAH			FORMAT6
#define FTPSRAVW		FORMAT7
#define FTPSRAW			FORMAT6
#define FTPSRLH			FORMAT6
#define FTPSRLVW		FORMAT7
#define FTPSRLW			FORMAT6
#define FTPSUBB			FORMAT1
#define FTPSUBH			FORMAT1
#define FTPSUBSB		FORMAT1
#define FTPSUBSH		FORMAT1
#define FTPSUBSW		FORMAT1
#define FTPSUBUB		FORMAT1
#define FTPSUBUH		FORMAT1
#define FTPSUBUW		FORMAT1
#define FTPSUBW			FORMAT1
#define FTPXOR			FORMAT1
#define FTQFSRV			FORMAT1
#define FTSQ			FORMAT11

// ** Cop0 instructions ** //

#define FTBC0F			FORMAT23
#define FTBC0FL			FORMAT23
#define FTBC0T			FORMAT23
#define FTBC0TL			FORMAT23
#define FTCACHE			FORMAT14
#define FTCACHEBFH		FORMAT14
#define FTCACHEBHINBT	FORMAT14
#define FTCACHEBXLBT	FORMAT14
#define FTCACHEBXSBT	FORMAT14
#define FTCACHEDHIN		FORMAT14
#define FTCACHEDHWBIN	FORMAT14
#define FTCACHEDHWOIN	FORMAT14
#define FTCACHEDXIN		FORMAT14
#define FTCACHEDXLDT	FORMAT14
#define FTCACHEDXLTG	FORMAT14
#define FTCACHEDXSDT	FORMAT14
#define FTCACHEDXSTG	FORMAT14
#define FTCACHEDXWBIN	FORMAT14
#define FTCACHEIFL		FORMAT14
#define FTCACHEIHIN		FORMAT14
#define FTCACHEIXIN		FORMAT14
#define FTCACHEIXLDT	FORMAT14
#define FTCACHEIXLTG	FORMAT14
#define FTCACHEIXSDT	FORMAT14
#define FTCACHEIXSTG	FORMAT14
#define FTDI			FORMAT16
#define FTEI			FORMAT16
#define FTERET			FORMAT16
#define FTMFBPC			FORMAT17
#define FTMFC0			FORMAT19
#define FTMFDAB			FORMAT17
#define FTMFDABM		FORMAT17
#define FTMFDVB			FORMAT17
#define FTMFDVBM		FORMAT17
#define FTMFIAB			FORMAT17
#define FTMFIABM		FORMAT17
#define FTMFPC			FORMAT18
#define FTMFPS			FORMAT18
#define FTMTBPC			FORMAT17
#define FTMTC0			FORMAT19
#define FTMTDAB			FORMAT17
#define FTMTDABM		FORMAT17
#define FTMTDVB			FORMAT17
#define FTMTDVBM		FORMAT17
#define FTMTIAB			FORMAT17
#define FTMTIABM		FORMAT17
#define FTMTPC			FORMAT18
#define FTMTPS			FORMAT18
#define FTTLBP			FORMAT16
#define FTTLBR			FORMAT16
#define FTTLBWI			FORMAT16
#define FTTLBWR			FORMAT16

#define FTCFC0			FORMAT19
#define FTCTC0			FORMAT19

// R3000A only
#define FTRFE			FORMAT16

// ** Cop1 instructions ** //

#define FTABSFMTS		FORMAT20
#define FTADDFMTS		FORMAT21
#define FTADDAFMTS		FORMAT22
#define FTBC1F			FORMAT23
#define FTBC1FL			FORMAT23
#define FTBC1T			FORMAT23
#define FTBC1TL			FORMAT23
#define FTCEQFMTS		FORMAT22
#define FTCFFMTS		FORMAT22
#define FTCLEFMTS		FORMAT22
#define FTCLTFMTS		FORMAT22
#define FTCFC1			FORMAT24
#define FTCTC1			FORMAT24
#define FTCVTSFMTW		FORMAT20
#define FTCVTWFMTS		FORMAT20
#define FTDIVFMTS		FORMAT21
#define FTLWC1			FORMAT11
#define FTMADDFMTS		FORMAT21
#define FTMADDAFMTS		FORMAT22
#define FTMAXFMTS		FORMAT21
#define FTMFC1			FORMAT19
#define FTMINFMTS		FORMAT21
#define FTMOVFMTS		FORMAT20
#define FTMSUBFMTS		FORMAT21
#define FTMSUBAFMTS		FORMAT22
#define FTMTC1			FORMAT19
#define FTMULFMTS		FORMAT21
#define FTMULAFMTS		FORMAT22
#define FTNEGFMTS		FORMAT20
#define FTRSQRTFMTS		FORMAT21
#define FTSQRTFMTS		FORMAT25
#define FTSUBFMTS		FORMAT21
#define FTSUBAFMTS		FORMAT22
#define FTSWC1			FORMAT11

// ** Cop2 instructions ** //

#define FTBC2F		FORMAT23
#define FTBC2FL		FORMAT23
#define FTBC2T		FORMAT23
#define FTBC2TL		FORMAT23
#define FTCFC2		FORMAT19
#define FTCTC2		FORMAT19
#define FTLQC2		FORMAT11
#define FTQMFC2		FORMAT19
#define FTQMTC2		FORMAT19
#define FTSQC2		FORMAT11
#define FTVABS		FORMAT26
#define FTVADD		FORMAT27
#define FTVADDI		FORMAT28
#define FTVADDQ		FORMAT28
#define FTVADDBC	FORMAT27
#define FTVADDA		FORMAT29
#define FTVADDAI	FORMAT30
#define FTVADDAQ	FORMAT30
#define FTVADDABC	FORMAT29
#define FTVCALLMS	FORMAT31
#define FTVCALLMSR	FORMAT16
#define FTVCLIP		FORMAT29
#define FTVDIV		FORMAT32
#define FTVFTOI0	FORMAT26
#define FTVFTOI4	FORMAT26
#define FTVFTOI12	FORMAT26
#define FTVFTOI15	FORMAT26
#define FTVIADD		FORMAT33
#define FTVIADDI	FORMAT34
#define FTVIAND		FORMAT33
#define FTVILWR		FORMAT35
#define FTVIOR		FORMAT33
#define FTVISUB		FORMAT33
#define FTVISWR		FORMAT35
#define FTVITOF0	FORMAT26
#define FTVITOF4	FORMAT26
#define FTVITOF12	FORMAT26
#define FTVITOF15	FORMAT26
#define FTVLQD		FORMAT51
#define FTVLQI		FORMAT51
#define FTVMADD		FORMAT27
#define FTVMADDI	FORMAT28
#define FTVMADDQ	FORMAT28
#define FTVMADDBC	FORMAT27
#define FTVMADDA	FORMAT29
#define FTVMADDAI	FORMAT30
#define FTVMADDAQ	FORMAT30
#define FTVMADDABC	FORMAT29
#define FTVMAX		FORMAT27
#define FTVMAXI		FORMAT28
#define FTVMAXBC	FORMAT27
#define FTVMFIR		FORMAT42
#define FTVMINI		FORMAT27
#define FTVMINII	FORMAT28
#define FTVMINIBC	FORMAT27
#define FTVMOVE		FORMAT26
#define FTVMR32		FORMAT26
#define FTVMSUB		FORMAT27
#define FTVMSUBI	FORMAT28
#define FTVMSUBQ	FORMAT28
#define FTVMSUBBC	FORMAT27
#define FTVMSUBA	FORMAT29
#define FTVMSUBAI	FORMAT30
#define FTVMSUBAQ	FORMAT30
#define FTVMSUBABC	FORMAT29
#define FTVMTIR		FORMAT41
#define FTVMUL		FORMAT27
#define FTVMULI		FORMAT28
#define FTVMULQ		FORMAT28
#define FTVMULBC	FORMAT27
#define FTVMULA		FORMAT29
#define FTVMULAI	FORMAT30
#define FTVMULAQ	FORMAT30
#define FTVMULABC	FORMAT29
#define FTVNOP		FORMAT16
#define FTVOPMULA	FORMAT29
#define FTVOPMSUB	FORMAT27
#define FTVRGET		FORMAT39
#define FTVRINIT	FORMAT36
#define FTVRNEXT	FORMAT39
#define FTVRSQRT	FORMAT32
#define FTVRXOR		FORMAT36
#define FTVSQD		FORMAT40
#define FTVSQI		FORMAT40
#define FTVSQRT		FORMAT38
#define FTVSUB		FORMAT27
#define FTVSUBI		FORMAT28
#define FTVSUBQ		FORMAT28
#define FTVSUBBC	FORMAT27
#define FTVSUBA		FORMAT29
#define FTVSUBAI	FORMAT30
#define FTVSUBAQ	FORMAT30
#define FTVSUBABC	FORMAT29
#define FTVWAITQ	FORMAT16






// r-type opcodes are 0
// i-type opcodes are all except for 000000,00001x, and 0100xx
// j-type opcodes are 00001x
// co-processor opcodes are 0100xx

// these are all the "Special" opcodes - "Special" refers to register only instructions typically
// their index in array corresponds with their "Special" code

// this is going to be the function called from the lookup table
typedef long (*OpFunction) ( long instruction, void* pointertosomething );


#endif

