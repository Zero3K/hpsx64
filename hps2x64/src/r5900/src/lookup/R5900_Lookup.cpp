
#include "R5900_Lookup.h"

using namespace R5900;
using namespace R5900::Instruction;


bool Lookup::c_bObjectInitialized = false;


u16 Lookup::LookupTable [ c_iLookupTable_Size ];


// used opcodes: 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf
// used opcodes: 0x10, 0x11, 0x12, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1e, 0x1f
// used opcodes: 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
// used opcodes: 0x31, 0x33, 0x36, 0x37, 0x39, 0x3e, 0x3f

// unused opcodes: 0x13, 0x1d, 0x32 (PS2 Only), 0x34, 0x35, 0x38, 0x3a, 0x3b, 0x3c, 0x3d

// in format: instruction name, opcode, rs, rt, shift, funct, index/id
const Instruction::Entry Instruction::Lookup::Entries [] = {

// unused opcodes //
//{ "INVALID",	0x13,	ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_Invalid },
//{ "INVALID",	0x1d,	ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_Invalid },
//{ "INVALID",	0x32,	ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_Invalid },
//{ "INVALID",	0x34,	ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_Invalid },
//{ "INVALID",	0x35,	ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_Invalid },
//{ "INVALID",	0x38,	ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_Invalid },
//{ "INVALID",	0x3a,	ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_Invalid },
//{ "INVALID",	0x3b,	ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_Invalid },
//{ "INVALID",	0x3c,	ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_Invalid },
//{ "INVALID",	0x3d,	ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_Invalid },


// used opcodes //

{ "BLTZ",		0x1,		ANY_VALUE,		0x0,			ANY_VALUE,		ANY_VALUE,		IDX_BLTZ },
{ "BGEZ",		0x1,		ANY_VALUE,		0x1,			ANY_VALUE,		ANY_VALUE,		IDX_BGEZ },
{ "BLTZAL",		0x1,		ANY_VALUE,		0x10,			ANY_VALUE,		ANY_VALUE,		IDX_BLTZAL },
{ "BGEZAL",		0x1,		ANY_VALUE,		0x11,			ANY_VALUE,		ANY_VALUE,		IDX_BGEZAL },
{ "BEQ",		0x4,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_BEQ },
{ "BNE",		0x5,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_BNE },
{ "BLEZ",		0x6,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_BLEZ },
{ "BGTZ",		0x7,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_BGTZ },
{ "J",			0x2,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_J },
{ "JAL",		0x3,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_JAL },
{ "JR",			0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x8,			IDX_JR },
{ "JALR", 		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x9,			IDX_JALR },
{ "LB",			0x20,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LB },
{ "LH",			0x21,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LH },
{ "LWL",		0x22,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LWL },
{ "LW",			0x23,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LW },
{ "LBU",		0x24,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LBU },
{ "LHU",		0x25,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LHU },
{ "LWR",		0x26,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LWR },
{ "SB",			0x28,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SB },
{ "SH",			0x29,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SH },
{ "SWL",		0x2a,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SWL },
{ "SW",			0x2b,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SW },
{ "SWR",		0x2e,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SWR },
{ "ADDI",		0x8,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_ADDI },
{ "ADDIU",		0x9,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_ADDIU },
{ "SLTI",		0xa,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SLTI },
{ "SLTIU",		0xb,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SLTIU },
{ "ANDI",		0xc,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_ANDI },
{ "ORI",		0xd,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_ORI },
{ "XORI",		0xe,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_XORI },
{ "LUI",		0xf,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LUI },
{ "SLL",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x0,			IDX_SLL },
{ "SRL",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x2,			IDX_SRL },
{ "SRA",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x3,			IDX_SRA },
{ "SLLV",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x4,			IDX_SLLV },
{ "SRLV",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x6,			IDX_SRLV },
{ "SRAV",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x7,			IDX_SRAV },
{ "SYSCALL",	0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0xc,			IDX_SYSCALL },
{ "BREAK",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0xd,			IDX_BREAK },
{ "MFHI",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x10,			IDX_MFHI },
{ "MTHI",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x11,			IDX_MTHI },
{ "MFLO",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x12,			IDX_MFLO },
{ "MTLO",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x13,			IDX_MTLO },
{ "MULT",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x18,			IDX_MULT },
{ "MULTU",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x19,			IDX_MULTU },
{ "DIV",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x1a,			IDX_DIV },
{ "DIVU",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x1b,			IDX_DIVU },
{ "ADD",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x20,			IDX_ADD },
{ "ADDU",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x21,			IDX_ADDU },
{ "SUB",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x22,			IDX_SUB },
{ "SUBU",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x23,			IDX_SUBU },
{ "AND",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x24,			IDX_AND },
{ "OR",			0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x25,			IDX_OR },
{ "XOR",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x26,			IDX_XOR },
{ "NOR",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x27,			IDX_NOR },
{ "SLT",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x2a,			IDX_SLT },
{ "SLTU",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x2b,			IDX_SLTU },
{ "MFC0",		0x10,		0x0,			ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_MFC0 },
{ "MTC0",		0x10,		0x4,			ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_MTC0 },

// on the R5900, this could have the I-bit set for interlocking, or NOT set for non-interlocking
{ "CFC2_I",		0x12,		0x2,			ANY_VALUE,		ANY_VALUE,		0x1,			IDX_CFC2_I },
{ "CTC2_I",		0x12,		0x6,			ANY_VALUE,		ANY_VALUE,		0x1,			IDX_CTC2_I },
{ "CFC2_NI",	0x12,		0x2,			ANY_VALUE,		ANY_VALUE,		0x0,			IDX_CFC2_NI },
{ "CTC2_NI",	0x12,		0x6,			ANY_VALUE,		ANY_VALUE,		0x0,			IDX_CTC2_NI },

// THERE ARE NO RFE OR GTE INSTRUCTIONS ON PS2 R5900 //
/*
{ "LWC2",		0x32,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LWC2 },
{ "SWC2",		0x3a,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SWC2 },
{ "MFC2",		0x12,		0x0,			ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_MFC2 },
{ "MTC2",		0x12,		0x4,			ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_MTC2 },
{ "RFE",		0x10,		0x10,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x11,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x12,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x13,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x14,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x15,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x16,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x17,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x18,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x19,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x1a,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x1b,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x1c,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x1d,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x1e,			ANY_VALUE,		0x10,			IDX_RFE },
{ "RFE",		0x10,		0x1f,			ANY_VALUE,		0x10,			IDX_RFE },


// *** COP 2 Instructions ***

{ "RTPS",		0x12,		ANY_VALUE,		ANY_VALUE,		0x1,			IDX_RTPS },
{ "NCLIP",		0x12,		ANY_VALUE,		ANY_VALUE,		0x6,			IDX_NCLIP },
{ "OP",			0x12,		ANY_VALUE,		ANY_VALUE,		0xc,			IDX_OP },
{ "DPCS",		0x12,		ANY_VALUE,		ANY_VALUE,		0x10,			IDX_DPCS },
{ "INTPL",		0x12,		ANY_VALUE,		ANY_VALUE,		0x11,			IDX_INTPL },
{ "MVMVA",		0x12,		ANY_VALUE,		ANY_VALUE,		0x12,			IDX_MVMVA },
{ "NCDS",		0x12,		ANY_VALUE,		ANY_VALUE,		0x13,			IDX_NCDS },
{ "CDP",		0x12,		ANY_VALUE,		ANY_VALUE,		0x14,			IDX_CDP },
{ "NCDT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x16,			IDX_NCDT },
{ "NCCS",		0x12,		ANY_VALUE,		ANY_VALUE,		0x1b,			IDX_NCCS },
{ "CC",			0x12,		ANY_VALUE,		ANY_VALUE,		0x1c,			IDX_CC },
{ "NCS",		0x12,		ANY_VALUE,		ANY_VALUE,		0x1e,			IDX_NCS },
{ "NCT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x20,			IDX_NCT },
{ "SQR",		0x12,		ANY_VALUE,		ANY_VALUE,		0x28,			IDX_SQR },
{ "DCPL",		0x12,		ANY_VALUE,		ANY_VALUE,		0x29,			IDX_DCPL },
{ "DPCT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x2a,			IDX_DPCT },
{ "AVSZ3",		0x12,		ANY_VALUE,		ANY_VALUE,		0x2d,			IDX_AVSZ3 },
{ "AVSZ4",		0x12,		ANY_VALUE,		ANY_VALUE,		0x2e,			IDX_AVSZ4 },
{ "RTPT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x30,			IDX_RTPT },
{ "GPF",		0x12,		ANY_VALUE,		ANY_VALUE,		0x3d,			IDX_GPF },
{ "GPL",		0x12,		ANY_VALUE,		ANY_VALUE,		0x3e,			IDX_GPL },
{ "NCCT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x3f,			IDX_NCCT }
*/


// *** New instructions for PS2 *** //
// Instruction, Opcode, Rs, Rt, Shift, Funct

	// branch instructions //
{ "BGEZL",		0x1,		ANY_VALUE,		0x3,			ANY_VALUE,		ANY_VALUE,		IDX_BGEZL },
{ "BLTZL",		0x1,		ANY_VALUE,		0x2,			ANY_VALUE,		ANY_VALUE,		IDX_BLTZL },
{ "BGEZALL",	0x1,		ANY_VALUE,		0x13,			ANY_VALUE,		ANY_VALUE,		IDX_BGEZALL },
{ "BLTZALL",	0x1,		ANY_VALUE,		0x12,			ANY_VALUE,		ANY_VALUE,		IDX_BLTZALL },

{ "BEQL",		0x14,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_BEQL },
{ "BNEL",		0x15,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_BNEL },
{ "BLEZL",		0x16,		ANY_VALUE,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		IDX_BLEZL },
{ "BGTZL",		0x17,		ANY_VALUE,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		IDX_BGTZL },

	// arithemetic instructions w/o immediate //
{ "DADD",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x2c,			IDX_DADD },
{ "DADDU",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x2d,			IDX_DADDU },
{ "DSUB",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x2e,			IDX_DSUB },
{ "DSUBU",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x2f,			IDX_DSUBU },

{ "DSLL",		0x0,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		0x38,			IDX_DSLL },
{ "DSRL",		0x0,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		0x3a,			IDX_DSRL },
{ "DSRA",		0x0,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		0x3b,			IDX_DSRA },
{ "DSLL32",		0x0,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		0x3c,			IDX_DSLL32 },
{ "DSRL32",		0x0,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		0x3e,			IDX_DSRL32 },
{ "DSRA32",		0x0,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		0x3f,			IDX_DSRA32 },

{ "DSLLV",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x14,			IDX_DSLLV },
{ "DSRLV",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x16,			IDX_DSRLV },
{ "DSRAV",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x17,			IDX_DSRAV },

	// arithemetic instructions w/ immediate //
{ "DADDI",		0x18,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_DADDI },
{ "DADDIU",		0x19,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_DADDIU },

	// load/store instructions //
{ "PREF",		0x33,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_PREF },

{ "LWU",		0x27,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LWU },

{ "LD",			0x37,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LD },
{ "SD",			0x3f,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SD },

{ "LDL",		0x1a,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LDL },
{ "LDR",		0x1b,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LDR },
{ "SDL",		0x2c,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SDL },
{ "SDR",		0x2d,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SDR },

{ "LQ",			0x1e,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LQ },
{ "SQ",			0x1f,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SQ },

	// trap instructions w/o immediate //
{ "TGE",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x30,			IDX_TGE },
{ "TGEU",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x31,			IDX_TGEU },
{ "TLT",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x32,			IDX_TLT },
{ "TLTU",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x33,			IDX_TLTU },
{ "TEQ",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x34,			IDX_TEQ },
{ "TNE",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x36,			IDX_TNE },

	// trap instructions w/ immediate //
{ "TGEI",		0x1,		ANY_VALUE,		0x8,			ANY_VALUE,		ANY_VALUE,		IDX_TGEI },
{ "TGEIU",		0x1,		ANY_VALUE,		0x9,			ANY_VALUE,		ANY_VALUE,		IDX_TGEIU },
{ "TLTI",		0x1,		ANY_VALUE,		0xa,			ANY_VALUE,		ANY_VALUE,		IDX_TLTI },
{ "TLTIU",		0x1,		ANY_VALUE,		0xb,			ANY_VALUE,		ANY_VALUE,		IDX_TLTIU },
{ "TEQI",		0x1,		ANY_VALUE,		0xc,			ANY_VALUE,		ANY_VALUE,		IDX_TEQI },
{ "TNEI",		0x1,		ANY_VALUE,		0xe,			ANY_VALUE,		ANY_VALUE,		IDX_TNEI },

	// data movement instructions //
{ "MOVZ",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0xa,			IDX_MOVZ },
{ "MOVN",		0x0,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0xb,			IDX_MOVN },

	// ps2 specific instructions //
{ "MULT1",		0x1c,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x18,			IDX_MULT1 },
{ "MULTU1",		0x1c,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x19,			IDX_MULTU1 },
{ "DIV1",		0x1c,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x1a,			IDX_DIV1 },
{ "DIVU1",		0x1c,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x1b,			IDX_DIVU1 },
{ "MADD",		0x1c,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x0,			IDX_MADD },
{ "MADD1",		0x1c,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x20,			IDX_MADD1 },
{ "MADDU",		0x1c,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x1,			IDX_MADDU },
{ "MADDU1",		0x1c,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x21,			IDX_MADDU1 },
{ "MFHI1",		0x1c,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x10,			IDX_MFHI1 },
{ "MTHI1",		0x1c,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x11,			IDX_MTHI1 },
{ "MFLO1",		0x1c,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x12,			IDX_MFLO1 },
{ "MTLO1",		0x1c,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x13,			IDX_MTLO1 },

{ "MFSA",		0x0,		ANY_VALUE,		ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x28,			IDX_MFSA },
{ "MTSA",		0x0,		ANY_VALUE,		ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x29,			IDX_MTSA },
{ "MTSAB",		0x1,		ANY_VALUE,		0x18,			ANY_VALUE,		ANY_VALUE,		IDX_MTSAB },
{ "MTSAH",		0x1,		ANY_VALUE,		0x19,			ANY_VALUE,		ANY_VALUE,		IDX_MTSAH },

{ "PABSH",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		0x5,			0x28,			IDX_PABSH },
{ "PABSW",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		0x1,			0x28,			IDX_PABSW },
{ "PADDB",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x8,			0x8,			IDX_PADDB },
{ "PADDH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x4,			0x8,			IDX_PADDH },
{ "PADDSB",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x18,			0x8,			IDX_PADDSB },
{ "PADDSH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x14,			0x8,			IDX_PADDSH },
{ "PADDSW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x10,			0x8,			IDX_PADDSW },
{ "PADDUB",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x18,			0x28,			IDX_PADDUB },
{ "PADDUH", 	0x1c,		ANY_VALUE,		ANY_VALUE,		0x14,			0x28,			IDX_PADDUH },
{ "PADDUW", 	0x1c,		ANY_VALUE,		ANY_VALUE,		0x10,			0x28,			IDX_PADDUW },
{ "PADDW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x0,			0x8,			IDX_PADDW },
{ "PADSBH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x4,			0x28,			IDX_PADSBH },
{ "PAND",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x12,			0x9,			IDX_PAND },
{ "PCEQB",		0x1c,		ANY_VALUE,		ANY_VALUE,		0xa,			0x28,			IDX_PCEQB },
{ "PCEQH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x6,			0x28,			IDX_PCEQH },
{ "PCEQW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x2,			0x28,			IDX_PCEQW },
{ "PCGTB",		0x1c,		ANY_VALUE,		ANY_VALUE,		0xa,			0x8,			IDX_PCGTB },
{ "PCGTH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x6,			0x8,			IDX_PCGTH },
{ "PCGTW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x2,			0x8,			IDX_PCGTW },
{ "PCPYH",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		0x1b,			0x29,			IDX_PCPYH },
{ "PCPYLD",		0x1c,		ANY_VALUE,		ANY_VALUE,		0xe,			0x9,			IDX_PCPYLD },
{ "PCPYUD",		0x1c,		ANY_VALUE,		ANY_VALUE,		0xe,			0x29,			IDX_PCPYUD },
{ "PDIVBW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x1d,			0x9,			IDX_PDIVBW },
{ "PDIVUW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0xd,			0x29,			IDX_PDIVUW },
{ "PDIVW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0xd,			0x9,			IDX_PDIVW },
{ "PEXCH",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		0x1a,			0x29,			IDX_PEXCH },
{ "PEXCW",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		0x1e,			0x29,			IDX_PEXCW },
{ "PEXEH",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		0x1a,			0x9,			IDX_PEXEH },
{ "PEXEW",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		0x1e,			0x9,			IDX_PEXEW },
{ "PEXT5",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		0x1e,			0x8,			IDX_PEXT5 },
{ "PEXTLB",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x1a,			0x8,			IDX_PEXTLB },
{ "PEXTLH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x16,			0x8,			IDX_PEXTLH },
{ "PEXTLW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x12,			0x8,			IDX_PEXTLW },
{ "PEXTUB",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x1a,			0x28,			IDX_PEXTUB },
{ "PEXTUH", 	0x1c,		ANY_VALUE,		ANY_VALUE,		0x16,			0x28,			IDX_PEXTUH },
{ "PEXTUW", 	0x1c,		ANY_VALUE,		ANY_VALUE,		0x12,			0x28,			IDX_PEXTUW },
{ "PHMADH", 	0x1c,		ANY_VALUE,		ANY_VALUE,		0x11,			0x9,			IDX_PHMADH },
{ "PHMSBH", 	0x1c,		ANY_VALUE,		ANY_VALUE,		0x15,			0x9,			IDX_PHMSBH },
{ "PINTEH", 	0x1c,		ANY_VALUE,		ANY_VALUE,		0xa,			0x29,			IDX_PINTEH },
{ "PINTH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0xa,			0x9,			IDX_PINTH },
{ "PLZCW",		0x1c,		ANY_VALUE,		0x0,			0x0,			0x4,			IDX_PLZCW },
{ "PMADDH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x10,			0x9,			IDX_PMADDH },
{ "PMADDUW",	0x1c,		ANY_VALUE,		ANY_VALUE,		0x0,			0x29,			IDX_PMADDUW },
{ "PMADDW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x0,			0x9,			IDX_PMADDW },
{ "PMAXH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x7,			0x8,			IDX_PMAXH },
{ "PMAXW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x3,			0x8,			IDX_PMAXW },
{ "PMFHI",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x8,			0x9,			IDX_PMFHI },
{ "PMFHL_LH",	0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x3,			0x30,			IDX_PMFHL_LH },
{ "PMFHL_LW",	0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x0,			0x30,			IDX_PMFHL_LW },
{ "PMFHL_SH",	0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x4,			0x30,			IDX_PMFHL_SH },
{ "PMFHL_SLW",	0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x2,			0x30,			IDX_PMFHL_SLW },
{ "PMFHL_UW",	0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x1,			0x30,			IDX_PMFHL_UW },
{ "PMFLO",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x9,			0x9,			IDX_PMFLO },
{ "PMINH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x7,			0x28,			IDX_PMINH },
{ "PMINW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x3,			0x28,			IDX_PMINW },
{ "PMSUBH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x14,			0x9,			IDX_PMSUBH },
{ "PMSUBW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x4,			0x9,			IDX_PMSUBW },
{ "PMTHI",		0x1c,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x8,			0x29,			IDX_PMTHI },
{ "PMTHL_LW",	0x1c,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x0,			0x31,			IDX_PMTHL_LW },
{ "PMTLO",		0x1c,		ANY_VALUE,		ANY_VALUE /*0x0*/,			0x9,			0x29,			IDX_PMTLO },
{ "PMULTH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x1c,			0x9,			IDX_PMULTH },
{ "PMULTUW",	0x1c,		ANY_VALUE,		ANY_VALUE,		0xc,			0x29,			IDX_PMULTUW },
{ "PMULTW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0xc,	 		0x9,			IDX_PMULTW },
{ "PNOR",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x13,			0x29,			IDX_PNOR },
{ "POR",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x12,			0x29,			IDX_POR },
{ "PPAC5",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		0x1f,			0x8,			IDX_PPAC5 },
{ "PPACB",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x1b,			0x8,			IDX_PPACB },
{ "PPACH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x17,			0x8,			IDX_PPACH },
{ "PPACW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x13,			0x8,			IDX_PPACW },
{ "PREVH",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		0x1b,			0x9,			IDX_PREVH },
{ "PROT3W",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		0x1f,			0x9,			IDX_PROT3W },
{ "PSLLH",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		0x34,			IDX_PSLLH },
{ "PSLLVW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x2,			0x9,			IDX_PSLLVW },
{ "PSLLW",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		0x3c,			IDX_PSLLW },
{ "PSRAH",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		0x37,			IDX_PSRAH },
{ "PSRAVW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x3,			0x29,			IDX_PSRAVW },
{ "PSRAW",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		0x3f,			IDX_PSRAW },
{ "PSRLH",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		0x36,			IDX_PSRLH },
{ "PSRLVW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x3,			0x9,			IDX_PSRLVW },
{ "PSRLW",		0x1c,		ANY_VALUE /*0x0*/,			ANY_VALUE,		ANY_VALUE,		0x3e,			IDX_PSRLW },
{ "PSUBB",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x9,			0x8,			IDX_PSUBB },
{ "PSUBH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x5,			0x8,			IDX_PSUBH },
{ "PSUBSB",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x19,			0x9,			IDX_PSUBSB },
{ "PSUBSH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x15,			0x8,			IDX_PSUBSH },
{ "PSUBSW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x11,			0x8,			IDX_PSUBSW },
{ "PSUBUB",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x19,			0x28,			IDX_PSUBUB },
{ "PSUBUH",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x15,			0x28,			IDX_PSUBUH },
{ "PSUBUW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x11,			0x28,			IDX_PSUBUW },
{ "PSUBW",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x1,			0x8,			IDX_PSUBW },
{ "PXOR",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x13,			0x9,			IDX_PXOR },
{ "QFSRV",		0x1c,		ANY_VALUE,		ANY_VALUE,		0x1b,			0x28,			IDX_QFSRV },

{ "SYNC",		0x0,		ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			ANY_VALUE,		0xf,			IDX_SYNC },

{ "CACHE",		0x2f,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_CACHE },

{ "BC0F",		0x10,		0x8,			0x0,			ANY_VALUE,		ANY_VALUE,		IDX_BC0F },
{ "BC0FL",		0x10,		0x8,			0x2,			ANY_VALUE,		ANY_VALUE,		IDX_BC0FL },
{ "BC0T",		0x10,		0x8,			0x1,			ANY_VALUE,		ANY_VALUE,		IDX_BC0T },
{ "BC0TL",		0x10,		0x8,			0x3,			ANY_VALUE,		ANY_VALUE,		IDX_BC0TL },
{ "DI",			0x10,		0x10,			ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x39,			IDX_DI },
{ "EI",			0x10,		0x10,			ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x38,			IDX_EI },
{ "ERET",		0x10,		0x10,			ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x18,			IDX_ERET },

//{ "MF0",		0x10,		0x0,			ANY_VALUE,		0x0,			0x0,			IDX_CFC0 },
//{ "MT0",		0x10,		0x4,			ANY_VALUE,		0x0,			0x0,			IDX_CTC0 },

{ "TLBP",		0x10,		0x10,			ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x8,			IDX_TLBP },
{ "TLBR",		0x10,		0x10,			ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x1,			IDX_TLBR },
{ "TLBWI",		0x10,		0x10,			ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x2,			IDX_TLBWI },
{ "TLBWR",		0x10,		0x10,			ANY_VALUE /*0x0*/,			ANY_VALUE /*0x0*/,			0x6,			IDX_TLBWR },

{ "BC1F",		0x11,		0x8,			0x0,			ANY_VALUE,		ANY_VALUE,		IDX_BC1F },
{ "BC1FL",		0x11,		0x8,			0x2,			ANY_VALUE,		ANY_VALUE,		IDX_BC1FL },
{ "BC1T",		0x11,		0x8,			0x1,			ANY_VALUE,		ANY_VALUE,		IDX_BC1T },
{ "BC1TL",		0x11,		0x8,			0x3,			ANY_VALUE,		ANY_VALUE,		IDX_BC1TL },

{ "BC2F",		0x12,		0x8,			0x0,			ANY_VALUE,		ANY_VALUE,		IDX_BC2F },
{ "BC2FL",		0x12,		0x8,			0x2,			ANY_VALUE,		ANY_VALUE,		IDX_BC2FL },
{ "BC2T",		0x12,		0x8,			0x1,			ANY_VALUE,		ANY_VALUE,		IDX_BC2T },
{ "BC2TL",		0x12,		0x8,			0x3,			ANY_VALUE,		ANY_VALUE,		IDX_BC2TL },

{ "MFC1",		0x11,		0x0,			ANY_VALUE,		0x0,			0x0,			IDX_MFC1 },
{ "CFC1",		0x11,		0x2,			ANY_VALUE,		0x0,			0x0,			IDX_CFC1 },
{ "MTC1",		0x11,		0x4,			ANY_VALUE,		0x0,			0x0,			IDX_MTC1 },
{ "CTC1",		0x11,		0x6,			ANY_VALUE,		0x0,			0x0,			IDX_CTC1 },

{ "LWC1",		0x31,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LWC1 },
{ "SWC1",		0x39,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SWC1 },

{ "ABS.S",		0x11,		0x10,			0x0,			ANY_VALUE,		0x5,			IDX_ABS_S },
{ "ADD.S",		0x11,		0x10,			ANY_VALUE,		ANY_VALUE,		0x0,			IDX_ADD_S },
{ "ADDA.S",		0x11,		0x10,			ANY_VALUE,		0x0,			0x18,			IDX_ADDA_S },
{ "C.EQ.S",		0x11,		0x10,			ANY_VALUE,		0x0,			0x32,			IDX_C_EQ_S },
{ "C.F.S",		0x11,		0x10,			ANY_VALUE,		0x0,			0x30,			IDX_C_F_S },
{ "C.LE.S",		0x11,		0x10,			ANY_VALUE,		0x0,			0x36,			IDX_C_LE_S },
{ "C.LT.S",		0x11,		0x10,			ANY_VALUE,		0x0,			0x34,			IDX_C_LT_S },
{ "CVT.S.W",	0x11,		0x14,			0x0,			ANY_VALUE,		0x20,			IDX_CVT_S_W },
{ "CVT.W.S",	0x11,		0x10,			0x0,			ANY_VALUE,		0x24,			IDX_CVT_W_S },
{ "DIV.S",		0x11,		0x10,			ANY_VALUE,		ANY_VALUE,		0x3,			IDX_DIV_S },
{ "MADD.S",		0x11,		0x10,			ANY_VALUE,		ANY_VALUE,		0x1c,			IDX_MADD_S },
{ "MADDA.S",	0x11,		0x10,			ANY_VALUE,		0x0,			0x1e,			IDX_MADDA_S },
{ "MAX.S",		0x11,		0x10,			ANY_VALUE,		ANY_VALUE,		0x28,			IDX_MAX_S },
{ "MIN.S",		0x11,		0x10,			ANY_VALUE,		ANY_VALUE,		0x29,			IDX_MIN_S },
{ "MOV.S",		0x11,		0x10,			0x0,			ANY_VALUE,		0x6,			IDX_MOV_S },
{ "MSUB.S",		0x11,		0x10,			ANY_VALUE,		ANY_VALUE,		0x1d,			IDX_MSUB_S },
{ "MSUBA.S",	0x11,		0x10,			ANY_VALUE,		0x0,			0x1f,			IDX_MSUBA_S },
{ "MUL.S",		0x11,		0x10,			ANY_VALUE,		ANY_VALUE,		0x2,			IDX_MUL_S },
{ "MULA.S",		0x11,		0x10,			ANY_VALUE,		0x0,			0x1a,			IDX_MULA_S },
{ "NEG.S",		0x11,		0x10,			0x0,			ANY_VALUE,		0x7,			IDX_NEG_S },
{ "RSQRT.S",	0x11,		0x10,			ANY_VALUE,		ANY_VALUE,		0x16,			IDX_RSQRT_S },
{ "SQRT.S",		0x11,		0x10,			ANY_VALUE,		ANY_VALUE,		0x4,			IDX_SQRT_S },
{ "SUB.S",		0x11,		0x10,			ANY_VALUE,		ANY_VALUE,		0x1,			IDX_SUB_S },
{ "SUBA.S",		0x11,		0x10,			ANY_VALUE,		0x0,			0x19,			IDX_SUBA_S },

{ "QMFC2.NI",	0x12,		0x1,			ANY_VALUE,		0x0,			0x0,			IDX_QMFC2_NI },
{ "QMFC2.I",	0x12,		0x1,			ANY_VALUE,		0x0,			0x1,			IDX_QMFC2_I },
{ "QMTC2.NI",	0x12,		0x5,			ANY_VALUE,		0x0,			0x0,			IDX_QMTC2_NI },
{ "QMTC2.I",	0x12,		0x5,			ANY_VALUE,		0x0,			0x1,			IDX_QMTC2_I },
{ "LQC2",		0x36,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_LQC2 },
{ "SQC2",		0x3e,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_SQC2 },


{ "VABS",		0x12,		ANY_VALUE,		ANY_VALUE,		0x7,			0x3d,			IDX_VABS },

{ "VADD",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x28,			IDX_VADD },
{ "VADDi",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x22,			IDX_VADDi },
{ "VADDq",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x20,			IDX_VADDq },
{ "VADDX",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x00,			IDX_VADDBCX },
{ "VADDY",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x01,			IDX_VADDBCY },
{ "VADDZ",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x02,			IDX_VADDBCZ },
{ "VADDW",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x03,			IDX_VADDBCW },

{ "VADDA",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0a,			0x3c,			IDX_VADDA },
{ "VADDAi",		0x12,		ANY_VALUE,		ANY_VALUE,		0x08,			0x3e,			IDX_VADDAi },
{ "VADDAq",		0x12,		ANY_VALUE,		ANY_VALUE,		0x08,			0x3c,			IDX_VADDAq },
{ "VADDAX",		0x12,		ANY_VALUE,		ANY_VALUE,		0x00,			0x3c,			IDX_VADDABCX },
{ "VADDAY",		0x12,		ANY_VALUE,		ANY_VALUE,		0x00,			0x3d,			IDX_VADDABCY },
{ "VADDAZ",		0x12,		ANY_VALUE,		ANY_VALUE,		0x00,			0x3e,			IDX_VADDABCZ },
{ "VADDAW",		0x12,		ANY_VALUE,		ANY_VALUE,		0x00,			0x3f,			IDX_VADDABCW },

{ "VCALLMS",	0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x38,			IDX_VCALLMS },
{ "VCALLMSR",	0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x39,			IDX_VCALLMSR },
{ "VCLIP",		0x12,		ANY_VALUE,		ANY_VALUE,		0x07,			0x3f,			IDX_VCLIP },
{ "VDIV",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0e,			0x3c,			IDX_VDIV },

{ "VFTOI0",		0x12,		ANY_VALUE,		ANY_VALUE,		0x05,			0x3c,			IDX_VFTOI0 },
{ "VFTOI4",		0x12,		ANY_VALUE,		ANY_VALUE,		0x05,			0x3d,			IDX_VFTOI4 },
{ "VFTOI12",	0x12,		ANY_VALUE,		ANY_VALUE,		0x05,			0x3e,			IDX_VFTOI12 },
{ "VFTOI15",	0x12,		ANY_VALUE,		ANY_VALUE,		0x05,			0x3f,			IDX_VFTOI15 },

{ "VIADD",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x30,			IDX_VIADD },
{ "VIADDI",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x32,			IDX_VIADDI },
{ "VIAND",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x34,			IDX_VIAND },
{ "VILWR",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0f,			0x3e,			IDX_VILWR },
{ "VIOR",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x35,			IDX_VIOR },
{ "VISUB",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x31,			IDX_VISUB },
{ "VISWR",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0f,			0x3f,			IDX_VISWR },

{ "VITOF0",		0x12,		ANY_VALUE,		ANY_VALUE,		0x04,			0x3c,			IDX_VITOF0 },
{ "VITOF4",		0x12,		ANY_VALUE,		ANY_VALUE,		0x04,			0x3d,			IDX_VITOF4 },
{ "VITOF12",	0x12,		ANY_VALUE,		ANY_VALUE,		0x04,			0x3e,			IDX_VITOF12 },
{ "VITOF15",	0x12,		ANY_VALUE,		ANY_VALUE,		0x04,			0x3f,			IDX_VITOF15 },

{ "VLQD",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0d,			0x3e,			IDX_VLQD },
{ "VLQI",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0d,			0x3c,			IDX_VLQI },

{ "VMADD",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x29,			IDX_VMADD },
{ "VMADDi",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x23,			IDX_VMADDi },
{ "VMADDq",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x21,			IDX_VMADDq },
{ "VMADDX",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x08,			IDX_VMADDBCX },
{ "VMADDY",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x09,			IDX_VMADDBCY },
{ "VMADDZ",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x0a,			IDX_VMADDBCZ },
{ "VMADDW",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x0b,			IDX_VMADDBCW },

{ "VMADDA",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0a,			0x3d,			IDX_VMADDA },
{ "VMADDAi",	0x12,		ANY_VALUE,		ANY_VALUE,		0x08,			0x3f,			IDX_VMADDAi },
{ "VMADDAq",	0x12,		ANY_VALUE,		ANY_VALUE,		0x08,			0x3d,			IDX_VMADDAq },
{ "VMADDAX",	0x12,		ANY_VALUE,		ANY_VALUE,		0x02,			0x3c,			IDX_VMADDABCX },
{ "VMADDAY",	0x12,		ANY_VALUE,		ANY_VALUE,		0x02,			0x3d,			IDX_VMADDABCY },
{ "VMADDAZ",	0x12,		ANY_VALUE,		ANY_VALUE,		0x02,			0x3e,			IDX_VMADDABCZ },
{ "VMADDAW",	0x12,		ANY_VALUE,		ANY_VALUE,		0x02,			0x3f,			IDX_VMADDABCW },

{ "VMAX",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x2b,			IDX_VMAX },
{ "VMAXi",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x1d,			IDX_VMAXi },
{ "VMAXX",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x10,			IDX_VMAXBCX },
{ "VMAXY",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x11,			IDX_VMAXBCY },
{ "VMAXZ",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x12,			IDX_VMAXBCZ },
{ "VMAXW",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x13,			IDX_VMAXBCW },

{ "VMFIR",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0f,			0x3d,			IDX_VMFIR },

{ "VMINI",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x2f,			IDX_VMINI },
{ "VMINIi",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x1f,			IDX_VMINIi },
{ "VMINIX",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x14,			IDX_VMINIBCX },
{ "VMINIY",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x15,			IDX_VMINIBCY },
{ "VMINIZ",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x16,			IDX_VMINIBCZ },
{ "VMINIW",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x17,			IDX_VMINIBCW },

{ "VMOVE",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0c,			0x3c,			IDX_VMOVE },
{ "VMR32",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0c,			0x3d,			IDX_VMR32 },

{ "VMSUB",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x2d,			IDX_VMSUB },
{ "VMSUBi",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x27,			IDX_VMSUBi },
{ "VMSUBq",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x25,			IDX_VMSUBq },
{ "VMSUBX",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x0c,			IDX_VMSUBBCX },
{ "VMSUBY",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x0d,			IDX_VMSUBBCY },
{ "VMSUBZ",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x0e,			IDX_VMSUBBCZ },
{ "VMSUBW",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x0f,			IDX_VMSUBBCW },

{ "VMSUBA",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0b,			0x3d,			IDX_VMSUBA },
{ "VMSUBAi",	0x12,		ANY_VALUE,		ANY_VALUE,		0x09,			0x3f,			IDX_VMSUBAi },
{ "VMSUBAq",	0x12,		ANY_VALUE,		ANY_VALUE,		0x09,			0x3d,			IDX_VMSUBAq },
{ "VMSUBAX",	0x12,		ANY_VALUE,		ANY_VALUE,		0x03,			0x3c,			IDX_VMSUBABCX },
{ "VMSUBAY",	0x12,		ANY_VALUE,		ANY_VALUE,		0x03,			0x3d,			IDX_VMSUBABCY },
{ "VMSUBAZ",	0x12,		ANY_VALUE,		ANY_VALUE,		0x03,			0x3e,			IDX_VMSUBABCZ },
{ "VMSUBAW",	0x12,		ANY_VALUE,		ANY_VALUE,		0x03,			0x3f,			IDX_VMSUBABCW },

{ "VMTIR",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0f,			0x3c,			IDX_VMTIR },

{ "VMUL",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x2a,			IDX_VMUL },
{ "VMULi",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x1e,			IDX_VMULi },
{ "VMULq",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x1c,			IDX_VMULq },
{ "VMULX",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x18,			IDX_VMULBCX },
{ "VMULY",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x19,			IDX_VMULBCY },
{ "VMULZ",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x1a,			IDX_VMULBCZ },
{ "VMULW",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x1b,			IDX_VMULBCW },

{ "VMULA",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0a,			0x3e,			IDX_VMULA },
{ "VMULAi",		0x12,		ANY_VALUE,		ANY_VALUE,		0x07,			0x3e,			IDX_VMULAi },
{ "VMULAq",		0x12,		ANY_VALUE,		ANY_VALUE,		0x07,			0x3c,			IDX_VMULAq },
{ "VMULAX",		0x12,		ANY_VALUE,		ANY_VALUE,		0x06,			0x3c,			IDX_VMULABCX },
{ "VMULAY",		0x12,		ANY_VALUE,		ANY_VALUE,		0x06,			0x3d,			IDX_VMULABCY },
{ "VMULAZ",		0x12,		ANY_VALUE,		ANY_VALUE,		0x06,			0x3e,			IDX_VMULABCZ },
{ "VMULAW",		0x12,		ANY_VALUE,		ANY_VALUE,		0x06,			0x3f,			IDX_VMULABCW },

{ "VNOP",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0b,			0x3f,			IDX_VNOP },
{ "VOPMULA",	0x12,		ANY_VALUE,		ANY_VALUE,		0x0b,			0x3e,			IDX_VOPMULA },
{ "VOPMSUB",	0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x2e,			IDX_VOPMSUB },
{ "VRGET",		0x12,		ANY_VALUE,		ANY_VALUE,		0x10,			0x3d,			IDX_VRGET },
{ "VRINIT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x10,			0x3e,			IDX_VRINIT },
{ "VRNEXT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x10,			0x3c,			IDX_VRNEXT },
{ "VRSQRT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0e,			0x3e,			IDX_VRSQRT },
{ "VRXOR",		0x12,		ANY_VALUE,		ANY_VALUE,		0x10,			0x3f,			IDX_VRXOR },
{ "VSQD",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0d,			0x3f,			IDX_VSQD },
{ "VSQI",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0d,			0x3d,			IDX_VSQI },
{ "VSQRT",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0e,			0x3d,			IDX_VSQRT },

{ "VSUB",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x2c,			IDX_VSUB },
{ "VSUBi",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x26,			IDX_VSUBi },
{ "VSUBq",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x24,			IDX_VSUBq },
{ "VSUBX",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x04,			IDX_VSUBBCX },
{ "VSUBY",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x05,			IDX_VSUBBCY },
{ "VSUBZ",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x06,			IDX_VSUBBCZ },
{ "VSUBW",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		0x07,			IDX_VSUBBCW },

{ "VSUBA",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0b,			0x3c,			IDX_VSUBA },
{ "VSUBAi",		0x12,		ANY_VALUE,		ANY_VALUE,		0x09,			0x3e,			IDX_VSUBAi },
{ "VSUBAq",		0x12,		ANY_VALUE,		ANY_VALUE,		0x09,			0x3c,			IDX_VSUBAq },
{ "VSUBAX",		0x12,		ANY_VALUE,		ANY_VALUE,		0x01,			0x3c,			IDX_VSUBABCX },
{ "VSUBAY",		0x12,		ANY_VALUE,		ANY_VALUE,		0x01,			0x3d,			IDX_VSUBABCY },
{ "VSUBAZ",		0x12,		ANY_VALUE,		ANY_VALUE,		0x01,			0x3e,			IDX_VSUBABCZ },
{ "VSUBAW",		0x12,		ANY_VALUE,		ANY_VALUE,		0x01,			0x3f,			IDX_VSUBABCW },

{ "VWAITQ",		0x12,		ANY_VALUE,		ANY_VALUE,		0x0e,			0x3f,			IDX_VWAITQ },

{ "COP2",		0x12,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_COP2 },

//{ "INVALID",	ANY_VALUE,	ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		ANY_VALUE,		IDX_Invalid }

};


// returns -1 on error
int Lookup::FindByName ( string NameOfInstruction )
{
	bool found;
	string Line_NoComment;
	string sInst;
	int StartPos, EndPos;
	
	// get rid of any comments
	//cout << "hasInstruction: removing comment from line. Line=" << NameOfInstruction.c_str() << "\n";
	//Line_NoComment = removeLabel( removeComment ( NameOfInstruction ) );
	
	// remove whitespace
	//cout << "hasInstruction: trimming line. Line_NoComment=" << NameOfInstruction.c_str() << "\n";
	Line_NoComment = Trim ( NameOfInstruction );
	
	// convert to upper case
	//cout << "hasInstruction: converting line to uppercase. Line_NoComment=" << Line_NoComment << "\n";
	Line_NoComment = UCase ( Line_NoComment );
	
	// get the instruction
	//cout << "hasInstruction: Getting the instruction.\n";
	//cout << "hasInstruction: Getting StartPos. Line_NoComment=" << Line_NoComment << "\n";
	StartPos = Line_NoComment.find_first_not_of ( " \r\n\t" );
	
	//cout << "StartPos=" << StartPos << "\n";
	
	if ( StartPos != string::npos )
	{
		//cout << "hasInstruction: Getting EndPos.\n";
		EndPos = Line_NoComment.find_first_of ( " \r\n\t", StartPos );
		
		if ( EndPos == string::npos ) EndPos = Line_NoComment.length();
		
		//cout << "hasInstruction: Getting just the instruction. EndPos=" << EndPos << "\n";
		sInst = Line_NoComment.substr( StartPos, EndPos - StartPos );
		
#ifdef INLINE_DEBUG
		debug << "hasInstruction: Instruction=" << sInst.c_str () << "\n";
#endif
		
		//cout << "hasInstruction: Instruction=" << sInst.c_str () << "\n";
		//cout << "\nnum instructions = " << dec << ( sizeof( Entries ) / sizeof( Entries [ 0 ] ) );
		
		// see if we find the instruction in the list
		for ( int i = 0; i < ( sizeof( Entries ) / sizeof( Entries [ 0 ] ) ); i++ )
		{
			if ( !sInst.compare( Entries [ i ].Name ) )
			{
				//cout << "\nfound instruction: " << sInst.c_str() << "\n";
				return Entries [ i ].InstructionIndex;
			}
		}
	}
	
	//cout << "\ndid not find instruction: " << NameOfInstruction;
	return -1;
}



//Debug::Log Lookup::debug;


void Lookup::Start ()
{
	u32 Opcode, Rs, Rt, Shft, Funct, Index, ElementsInExecute, ElementsInBranchLoad1;
	Instruction::Format i;
	
	u32 ulCounter, ulRemainder;

	cout << "Running constructor for R5900::Lookup class.\n";
	
	if ( c_bObjectInitialized ) return;

#ifdef INLINE_DEBUG_ENABLE	
	debug.Create ( "R5900_Lookup_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "Running constructor for R5900::Lookup class.\r\n";
#endif

	
	ElementsInExecute = (sizeof(Entries) / sizeof(Entries[0]));
	
	// clear table first
	cout << "\nSize of R5900 lookup table in bytes=" << dec << sizeof( LookupTable );
	for ( Index = 0; Index < ( sizeof( LookupTable ) >> 3 ); Index++ ) ((u64*)LookupTable) [ Index ] = 0;
	
	for ( Index = ElementsInExecute - 1; Index < ElementsInExecute; Index-- )
	{
		ulRemainder = 0;
		
		Opcode = Entries [ Index ].Opcode;
		Rs = Entries [ Index ].Rs;
		Rt = Entries [ Index ].Rt;
		Shft = Entries [ Index ].Shift;
		Funct = Entries [ Index ].Funct;
		
		i.Opcode = Opcode;
		i.Rs = Rs;
		i.Rt = Rt;
		i.Shift = Shft;
		i.Funct = Funct;
		
		for ( ulCounter = 0; ulRemainder == 0; ulCounter++ )
		{
			ulRemainder = ulCounter;
			
			if ( Opcode == 0xff )
			{
				// 6 bits
				i.Opcode = ulRemainder & 0x3f;
				ulRemainder >>= 6;
			}
			
			if ( Rs == 0xff )
			{
				// 5 bits
				i.Rs = ulRemainder & 0x1f;
				ulRemainder >>= 5;
			}
			
			if ( Rt == 0xff )
			{
				// 5 bits
				i.Rt = ulRemainder & 0x1f;
				ulRemainder >>= 5;
			}
			
			if ( Shft == 0xff )
			{
				// 5 bits
				i.Shift = ulRemainder & 0x1f;
				ulRemainder >>= 5;
			}
			
			if ( Funct == 0xff )
			{
				// 6 bits
				i.Funct = ulRemainder & 0x3f;
				ulRemainder >>= 6;
			}
			
			LookupTable [ ( ( i.Value >> 16 ) | ( i.Value << 16 ) ) & c_iLookupTable_Mask ] = Entries [ Index ].InstructionIndex;
		}
	}
	

	/*
	for ( Opcode = 0; Opcode < c_iOpcode_MaxValue; Opcode++ )
	{
		//cout << " " << Opcode;
		
		for ( Rs = 0; Rs < c_iRs_MaxValue; Rs++ )
		{
			for ( Rt = 0; Rt < c_iRt_MaxValue; Rt++ )
			{
				for ( Shft = 0; Shft < c_iShiftAmount_MaxValue; Shft++ )
				{
					for ( Funct = 0; Funct < c_iFunct_MaxValue; Funct++ )
					{
						i.Opcode = Opcode;
						i.Rs = Rs;
						i.Rt = Rt;
						i.Shift = Shft;
						i.Funct = Funct;
					
						// initialize entry in LUT to Invalid instruction
						LookupTable [ ( ( i.Value >> 16 ) | ( i.Value << 16 ) ) & c_iLookupTable_Mask ] = IDX_Invalid;
						
						// lookup entry in list of instructions
						for ( Index = 0; Index < ElementsInExecute; Index++ )
						{
							// check if we have found the instruction to insert into current position of LUT
							if ( ( Entries [ Index ].Opcode == Opcode || Entries [ Index ].Opcode == ANY_VALUE )
							&& ( Entries [ Index ].Rs == Rs || Entries [ Index ].Rs == ANY_VALUE )
							&& ( Entries [ Index ].Rt == Rt || Entries [ Index ].Rt == ANY_VALUE )
							&& ( Entries [ Index ].Shift == Shft || Entries [ Index ].Shift == ANY_VALUE )
							&& ( Entries [ Index ].Funct == Funct || Entries [ Index ].Funct == ANY_VALUE ) )
							{
								// enter function for entry into LUT
								LookupTable [ ( ( i.Value >> 16 ) | ( i.Value << 16 ) ) & c_iLookupTable_Mask ] = Entries [ Index ].InstructionIndex;
								
								// the value was found, so need to break since INVALID entry is at end of list
								break;
							}
						}
					}
				
				}
				
			}
			
		}
		
	}
	*/
	
	// object has now been fully initilized
	c_bObjectInitialized = true;
}
