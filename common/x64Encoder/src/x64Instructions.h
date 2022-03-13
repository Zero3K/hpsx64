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


#ifndef _X64INSTRUCTIONS_H_

#define _X64INSTRUCTIONS_H_


// x64 encoding section

// optional group prefixes

// Group 1

#define LOCK_PREFIX		0xf0

// Group 2

#define BRANCH_NOT_TAKEN	0X2e
#define BRANCH_TAKEN		0x3e

// REX Prefix

#define REX_PREFIX_START	(0x4<<4)

// Overide prefix for 16-bit operations - would come before rex prefix if there is a rex prefix
#define PREFIX_16BIT		0x66

// REX Prefix - W bit

#define OPERAND_64BIT		0x1
#define OPERAND_CSD		0x0


// REX Prefix - R bit - extension of Reg/Opcode (bit added to high bit, the leftmost one)

#define RBIT			0x1

// REX Prefix - X bit - extension of the index in SIB

#define XBIT			0x1

// REX Prefix - B bit - extension of R/M or SIB (bit added to high bit, the leftmost one)

#define BBIT			0x1


// x64 Opcodes

#define CREATE_REX(w,r,x,b)		( REX_PREFIX_START | (w<<3) | (r<<2) | (x<<1) | (b) )
#define CREATE_REX64(r,x,b)		( REX_PREFIX_START | (OPERAND_64BIT<<3) | (r<<2) | (x<<1) | (b) )

// use this one to make a rex prefix - I like this one best
#define MAKE_REX(w,regopcode,index,rmbase)		( REX_PREFIX_START | (w<<3) | ( ( ( regopcode ) & 8 ) >> 1 ) | ( ( ( index ) & 8 ) >> 2 ) | ( ( ( rmbase ) & 8 ) >> 3 ) )

#define MAKE_MODRMREGREG(regopcode,rm)		( ( 0x3 << 6 ) | ( ( ( regopcode ) & 0x7 ) << 3 ) | ( ( rm ) & 7 ) )
#define MAKE_MODRMREGMEM(address_mode,reg,rmbase)	( ( ( address_mode ) << 6 ) | ( ( ( reg ) & 7 ) << 3 ) | ( ( rmbase ) & 7 ) )

// sometimes the instruction is not just the op code, but you also have to put another value in reg/opcode field of mod r/m byte
#define MAKE_MODRMREGOP(address_mode,rm,op)	( ( ( address_mode ) << 6 ) | ( ( ( op ) & 7 ) << 3 ) | ( ( rm ) & 7 ) )

// Mod - Addressing modes

#define	REGREG				3
#define REGMEM_NOOFFSET		0
#define REGMEM_OFFSET8		1
#define REGMEM_OFFSET32		2

// special value for rm RIP-relative addressing (also use MOD=0)
#define RMBASE_USINGRIP		5

// special value for rm with register-memory instructions - need to use this if using an SIB byte

#define RMBASE_USINGSIB		4

// SIB byte

#define MAKE_SIB(scale,index,base)	( ( ( scale ) << 6 ) | ( ( ( index ) & 7 ) << 3 ) | ( ( base ) & 7 ) )


// special value for index

#define NO_INDEX		4

// SIB - values for scale - it just scales the index

#define SCALE_NONE		0
#define SCALE_TWO		1
#define SCALE_FOUR		2
#define SCALE_EIGHT		3

// SIB - values for index - it's just the index of the register to use as the index register

// SIB - values for base - it's just the index of the register to use as the base register


// x64 registers

/*
#define AL			0
#define CL			1
#define DL			2
#define BL			3
#define AH			4
#define CH			5
#define DH			6
#define BH			7

#define AX			0
#define CX			1
#define DX			2
#define BX			3
#define SP			4
#define BP			5
#define SI			6
#define DI			7
*/

#define EAX			0
#define ECX			1
#define EDX			2
#define EBX			3
#define ESP			4
#define EBP			5
#define ESI			6
// this causes a conflict with R5900 recompiler
//#define EDI			7

#define RAX			0
#define RCX			1
#define RDX			2
#define RBX			3
#define RSP			4
#define RBP			5
#define RSI			6
#define RDI			7

/*
#define R8			8
#define R9			9
#define R10			10
#define R11			11
#define R12			12
#define R13			13
#define R14			14
#define R15			15
*/

#define MM0			0
#define MM1			1
#define MM2			2
#define MM3			3
#define MM4			4
#define MM5			5
#define MM6			6
#define MM7			7

#define XMM0			0
#define XMM1			1
#define XMM2			2
#define XMM3			3
#define XMM4			4
#define XMM5			5
#define XMM6			6
#define XMM7			7

#define XMM8			8
#define XMM9			9
#define XMM10			10
#define XMM11			11
#define XMM12			12
#define XMM13			13
#define XMM14			14
#define XMM15			15



// **** VEX Prefix stuff ****

// start prefix with this
// prefix of 0x0f is implied with a 2 byte vex prefix
#define VEX_START_2BYTE			0xc5
#define VEX_START_3BYTE			0xc4

// these are for the 2nd byte in two byte vex prefixes
// R is the extension of the reg/opcode field, but in one's complement (define takes care of that part)
// vvvv specifies a register in 1's complement or is 1111 if unused - specifies first source operand in instruction syntax, except in certain shifts
// L is the vector length (128-bit or 256-bit)
// pp implies an simd prefix
// prefix of 0x0f is implied with a 2 byte vex prefix
#define VEX_2BYTE_END(R,vvvv,L,pp)	( ( ( ( ~( R ) ) & 0x8 ) << 4 ) | ( ( ( ~( vvvv ) ) & 0xf ) << 3 ) | ( ( L ) << 2 ) | ( pp ) )

// these are for the values of L
#define VEX_128BIT				0x0
#define VEX_256BIT				0x1

// these are for the values of pp
#define VEX_PREFIXNONE			0
#define VEX_PREFIX66			1
#define VEX_PREFIXF3			2
#define VEX_PREFIXF2			3


// these are for the 2nd byte in a 3-byte VEX prefix
// R is the extension of the reg/opcode field, exactly like from REX prefix but inverted
// X is exactly like from REX prefix but inverted
// B is exactly like from REX prefix but inverted
#define VEX_3BYTE_MID(R,X,B,mmmmm)		( ( ( ( ~( R ) ) & 0x8 ) << 4 ) | ( ( ( ~( X ) ) & 0x8 ) << 3 ) | ( ( ( ~( B ) ) & 0x8 ) << 2 ) | ( mmmmm ) )

// these are for mmmmm
#define VEX_PREFIX0F			1
#define VEX_PREFIX0F38			2
#define VEX_PREFIX0F3A			3

// this is for the last byte in a 3-byte VEX prefix
// W is just like in REX prefix, but inverted. Used to specify 64-bit operation with general purpose registers in sse instructions
#define VEX_3BYTE_END(W,vvvv,L,pp)	( ( ( ( ( W ) ) & 0x8 ) << 4 ) | ( ( ( ~( vvvv ) ) & 0xf ) << 3 ) | ( ( L ) << 2 ) | ( pp ) )


// **** Instruction Encoding ****

// x64 Opcodes

// ** short jump instructions ** //

// JMP8 - unconditional near short-jump
// JMP rel8
#define X64OP_JMP8			0xeb

// JA - jump if unsigned above (carry flag=0 and zero flag=0)
// JA rel8
#define X64OP_JA			0x77

// JAE - jump if unsigned above or equal (carry flag=0)
// JAE rel8
#define X64OP_JAE			0x73

// JB - jump if unsigned above (carry flag=0 and zero flag=0)
// JB rel8
#define X64OP_JB			0x72

// JBE - jump if unsigned above (carry flag=0 and zero flag=0)
// JBE rel8
#define X64OP_JBE			0x76

// JE - jump if equal (zero flag=1)
// JE rel8
#define X64OP_JE			0x74

// JNE - jump if not equal (zero flag=0)
// JNE rel8
#define X64OP_JNE			0x75

// JG - jump short if signed greater (zero flag=0 and sf=of)
// JG rel8
#define X64OP_JG			0x7f

// JGE - jump short if signed greater or equal (sf=of)
// JGE rel8
#define X64OP_JGE			0x7d

// JL - jump short if signed lesser (sf!=of)
// JL rel8
#define X64OP_JL			0x7c

// JLE - jump short if signed lesser or equal (zero flag=1 or sf!=of)
// JLE rel8
#define X64OP_JLE			0x7e


// JNO - jump short if not overflow
// JNO rel8
#define X64OP_JNO			0x71


// JCX/JECX/JRCX - jump short if signed lesser or equal (zero flag=1 or sf!=of)
// JCX rel8
// JECX rel8
// JRCX rel8
#define X64OP_JCX			0xe3
#define X64OP_JECX			0xe3
#define X64OP_JRCX			0xe3



// ** absolute jump instructions ** //

// Jmp instruction
// JMP rel32
#define X64OP_JMP			0xe9

// ** Conditional Branch Instructions ** //

// jump if equal
// JE rel32
#define X64OP1_JMP_E			0x0f
#define X64OP2_JMP_E			0x84

// jump if not equal
// JNE rel32
#define X64OP1_JMP_NE			0x0f
#define X64OP2_JMP_NE			0x85


// jump if signed greater
// JG rel32
#define X64OP1_JMP_G			0x0f
#define X64OP2_JMP_G			0x8f

// jump if signed greater or equal
// JGE rel32
#define X64OP1_JMP_GE			0x0f
#define X64OP2_JMP_GE			0x8d

// jump if signed less than
// JL rel32
#define X64OP1_JMP_L			0x0f
#define X64OP2_JMP_L			0x8c

// jump if signed less than or equal
// JLE rel32
#define X64OP1_JMP_LE			0x0f
#define X64OP2_JMP_LE			0x8e

// jump if unsigned below
// JB rel32
#define X64OP1_JMP_B			0x0f
#define X64OP2_JMP_B			0x82

// jump if unsigned above
// JBE rel32
#define X64OP1_JMP_BE			0x0f
#define X64OP2_JMP_BE			0x86

// jump if unsigned above
// JA rel32
#define X64OP1_JMP_A			0x0f
#define X64OP2_JMP_A			0x87

// jump if unsigned above
// JAE rel32
#define X64OP1_JMP_AE			0x0f
#define X64OP2_JMP_AE			0x83




// ** ADD Instructions ** //

// Add instruction
// ADD r64, r/m64
// ADD r32, r/m32
#define X64OP_ADD			0x03

// ADD r/m16, r16
// ADD r/m32, r32
// ADD r/m64, r64
#define X64OP_ADD_TOMEM		0x01

// Add immediate 32-bit
// ADD r/m64, imm32
// ADD r/m32, imm32
#define X64OP_ADD_IMM		0x81
#define MODRM_ADD_IMM		0x0

// Add immediate 8-bit
// ADD r/m64, imm8
// ADD r/m32, imm8
// ADD r/m16, imm8
#define X64OP_ADD_IMM8		0x83
#define MODRM_ADD_IMM8		0x0

// Add immedate to Accumulator
// ADD AX, imm16
// ADD EAX, imm32
// ADD RAX, imm32
#define X64OP_ADD_IMMA		0x05


// ** ADC Instructions ** //

// ADC r64, r/m64
// ADC r32, r/m32
#define X64OP_ADC			0x13

// ADD r/m16, r16
// ADD r/m32, r32
// ADD r/m64, r64
#define X64OP_ADC_TOMEM		0x11

// Add immediate 32-bit
// ADD r/m64, imm32
// ADD r/m32, imm32
#define X64OP_ADC_IMM		0x81
#define MODRM_ADC_IMM		0x2

// Add immediate 8-bit
// ADD r/m64, imm8
// ADD r/m32, imm8
// ADD r/m16, imm8
#define X64OP_ADC_IMM8		0x83
#define MODRM_ADC_IMM8		0x2

// Add immedate to Accumulator
// ADD AX, imm16
// ADD EAX, imm32
// ADD RAX, imm32
#define X64OP_ADC_IMMA		0x15


// ** and instruction ** //

// AND r16, r/m16
// AND r32, r/m32
// AND r64, r/m64
#define X64OP_AND			0x23

// AND r/m16, r16
// AND r/m32, r32
// AND r/m64, r64
#define X64OP_AND_TOMEM		0x21

// AND r/m16, imm16
// AND r/m32, imm32
// AND r/m64, imm64
#define X64OP_AND_IMM		0x81
#define MODRM_AND_IMM		0x4

// And immediate 8-bit
// AND r/m64, imm8
// AND r/m32, imm8
// AND r/m16, imm8
#define X64OP_AND_IMM8		0x83
#define MODRM_AND_IMM8		0x4

// And immedate to Accumulator
// AND AX, imm16
// AND EAX, imm32
// AND RAX, imm32
#define X64OP_AND_IMMA		0x25


// ** bsr instruction ** //

// BSR r16, r/m16
// BSR r32, r/m32
// BSR r64, r/m64
#define X64OP_BSR_OP1			0x0f
#define X64OP_BSR_OP2			0xbd


// ** bt instruction - bit test (stores specified bit in carry flag) ** //

// bt - bit test
// BT r/m16, r16
// BT r/m32, r32
// BT r/m64, r64
#define X64OP1_BT			0x0f
#define X64OP2_BT			0xa3

// BT r/m16, imm8
// BT r/m32, imm8
// BT r/m64, imm8
#define X64OP1_BT_IMM		0x0f
#define X64OP2_BT_IMM		0xba
#define MODRM_BT_IMM		0x4

// ** btc instruction - bit test and compliment (stores specified bit in carry flag) ** //

// btc - bit test and complement
// BTC r/m16, r16
// BTC r/m32, r32
// BTC r/m64, r64
#define X64OP1_BTC			0x0f
#define X64OP2_BTC			0xbb

// BTC r/m16, imm8
// BTC r/m32, imm8
// BTC r/m64, imm8
#define X64OP1_BTC_IMM		0x0f
#define X64OP2_BTC_IMM		0xba
#define MODRM_BTC_IMM		0x7

// ** btr instruction - bit test and reset (stores specified bit in carry flag) ** //

// btr - bit test and reset
// BTR r/m16, r16
// BTR r/m32, r32
// BTR r/m64, r64
#define X64OP1_BTR			0x0f
#define X64OP2_BTR			0xb3

// BTR r/m16, imm8
// BTR r/m32, imm8
// BTR r/m64, imm8
#define X64OP1_BTR_IMM		0x0f
#define X64OP2_BTR_IMM		0xba
#define MODRM_BTR_IMM		0x6

// ** bts instruction - bit test and set (stores specified bit in carry flag) ** //

// bts - bit test
// BTS r/m16, r16
// BTS r/m32, r32
// BTS r/m64, r64
#define X64OP1_BTS			0x0f
#define X64OP2_BTS			0xab

// BTS r/m16, imm8
// BTS r/m32, imm8
// BTS r/m64, imm8
#define X64OP1_BTS_IMM		0x0f
#define X64OP2_BTS_IMM		0xba
#define MODRM_BTS_IMM		0x5



// ** cbw, cwde, cdqe - sign extention for RAX - from byte to word, word to double word, or double word to quadword respectively ** //

// CBW
// CWDE
// CDQE
#define X64OP_CBW			0x98
#define X64OP_CWDE			0x98
#define X64OP_CDQE			0x98


// ** cwd, cdq, cqo - sign extention for RAX - from word to double word, or double word to quadword respectively into RAX:RDX ** //

// CWD
// CDQ
// CQO
#define X64OP_CWD			0x99
#define X64OP_CDQ			0x99
#define X64OP_CQO			0x99



// ** cmov instruction - conditional move ** //

// cmove - conditional move when zero flag is set
// CMOVE r16, r/m16
// CMOVE r32, r/m32
// CMOVE r64, r/m64
#define X64OP1_CMOVE		0x0f
#define X64OP2_CMOVE		0x44

// cmovne - conditional move when zero flag is cleared
// CMOVNE r16, r/m16
// CMOVNE r32, r/m32
// CMOVNE r64, r/m64
#define X64OP1_CMOVNE		0x0f
#define X64OP2_CMOVNE		0x45

// * unsigned conditional move * //

// cmovb - unsigned move if below (cf=1)
// CMOVB r16, r/m16
// CMOVB r32, r/m32
// CMOVB r64, r/m64
#define X64OP1_CMOVB		0x0f
#define X64OP2_CMOVB		0x42

// cmovb - unsigned move if below or equal (cf=1 or zf=1)
// CMOVBE r16, r/m16
// CMOVBE r32, r/m32
// CMOVBE r64, r/m64
#define X64OP1_CMOVBE		0x0f
#define X64OP2_CMOVBE		0x46

// cmova - unsigned move if above (cf=0 zf=0)
// CMOVA r16, r/m16
// CMOVA r32, r/m32
// CMOVA r64, r/m64
#define X64OP1_CMOVA		0x0f
#define X64OP2_CMOVA		0x47

// cmovae - unsigned move if above or equal (cf=0)
// CMOVAE r16, r/m16
// CMOVAE r32, r/m32
// CMOVAE r64, r/m64
#define X64OP1_CMOVAE		0x0f
#define X64OP2_CMOVAE		0x43

// * signed conditional move * //

// cmovl - signed move if less
// CMOVL r16, r/m16
// CMOVL r32, r/m32
// CMOVL r64, r/m64
#define X64OP1_CMOVL		0x0f
#define X64OP2_CMOVL		0x4c

// cmovle - signed move if less or equal
// CMOVLE r16, r/m16
// CMOVLE r32, r/m32
// CMOVLE r64, r/m64
#define X64OP1_CMOVLE		0x0f
#define X64OP2_CMOVLE		0x4e

// cmovg - signed move if greater
// CMOVG r16, r/m16
// CMOVG r32, r/m32
// CMOVG r64, r/m64
#define X64OP1_CMOVG		0x0f
#define X64OP2_CMOVG		0x4f

// cmovge - signed move if greater or equal
// CMOVGE r16, r/m16
// CMOVGE r32, r/m32
// CMOVGE r64, r/m64
#define X64OP1_CMOVGE		0x0f
#define X64OP2_CMOVGE		0x4d

// cmovs - conditional move when sign flag is set
// CMOVS r16, r/m16
// CMOVS r32, r/m32
// CMOVS r64, r/m64
#define X64OP1_CMOVS		0x0f
#define X64OP2_CMOVS		0x48

// cmovns - conditional move when sign flag is NOT set
// CMOVNS r16, r/m16
// CMOVNS r32, r/m32
// CMOVNS r64, r/m64
#define X64OP1_CMOVNS		0x0f
#define X64OP2_CMOVNS		0x49



// ** cmp instruction ** //

// cmp - compare 2 registers
// CMP r16, r/m16
// CMP r32, r/m32
// CMP r64, r/m64
#define X64OP_CMP			0x3b

// CMP r8, r/m8
#define X64OP_CMP8			0x3a

// CMP r/m16, r16
// CMP r/m32, r32
// CMP r/m64, r64
#define X64OP_CMP_TOMEM		0x39

// cmp - compare immediate
// CMP r/m16, imm16
// CMP r/m32, imm32
// CMP r/m64, imm64
#define X64OP_CMP_IMM		0x81
#define MODRM_CMP_IMM		0x7

// cmp immediate 8-bit
// CMP r/m64, imm8
// CMP r/m32, imm8
// CMP r/m16, imm8
#define X64OP_CMP_IMM8		0x83
#define MODRM_CMP_IMM8		0x7

// cmp immedate to Accumulator
// CMP AX, imm16
// CMP EAX, imm32
// CMP RAX, imm32
#define X64OP_CMP_IMMA		0x3d


// ** div instruction ** //

// Unsigned divide D:A by r/m, with result stored in A <- Quotient, D <- Remainder
// DIV r/m16
// DIV r/m32
// DIV r/m64
#define X64OP_DIV			0xf7
#define MODRM_DIV			0x6


// ** signed divide - idiv instruction ** //

// Signed divide D:A by r/m, with result stored in A <- Quotient, D <- Remainder
// IDIV r/m16
// IDIV r/m32
// IDIV r/m64
#define X64OP_IDIV			0xf7
#define MODRM_IDIV			0x7

// ** signed multiply - imul instruction ** //

// D:A <- A * r/m
// IMUL r/m16
// IMUL r/m32
// IMUL r/m64
#define X64OP_IMUL			0xf7
#define MODRM_IMUL			0x5

// Quadword register <- Quadword register * r/m64
// IMUL r16, r/m16
// IMUL r32, r/m32
// IMUL r64, r/m64
#define X64OP1_IMUL			0x00
#define X64OP2_IMUL			0xaf


// ** lea instruction - can be used as a 3 operand add instruction ** //

// Lea instruction
// Store effective address form in register r64
// LEA r64,m
#define X64OP_LEA			0x8d


// ** mov instructions ** //

// Move r8 to r/m8
// MOV r/m8,r8
#define X64OP_MOV_TOMEM8	0x88

// Move r/m8 to r8
// MOV r8,r/m8
#define X64OP_MOV_FROMMEM8	0x8a

// Mov instruction
// Move r64 to r/m64
// MOV r/m64, r64
// MOV r/m32, r32
#define X64OP_MOV_TOMEM		0x89

// Mov instruction
// Move r/m64 to r64
// MOV r64, r/m64
// MOV r32, r/m32
#define X64OP_MOV_FROMMEM	0x8b
#define X64OP_MOV			0x8b

// Mov immediate instruction - this one can sign extend a 32-bit immediate to 64-bits
// MOV r64, imm32
// MOV r32, imm32
// MOV r16, imm16
#define X64OP_MOV_IMM		0xc7
#define MODRM_MOV_IMM		0

// Mov immediate instruction - this one can sign extend a 32-bit immediate to 64-bits
// MOV r8, imm8
#define X64OP_MOV_IMM8		0xc6
#define MODRM_MOV_IMM8		0

// Mov immediate instruction (fewer bytes needed)
// MOV r64, imm64
// MOV r32, imm32
// ** Note: Don't use this one, it's only used in the Mov encoding instructions **
#define X64BOP_MOV_IMM		0xb8

// Mov immediate instruction

// Mov immediate instruction (fewer bytes needed)
#define X64BOP_MOV_IMM8		0xb0


// ** movsx instruction ** //

// Movsxb instruction
// move with sign extenstion from 16-bits
// MOVSX r16, r/m8
// MOVSX r32, r/m8
// MOVSX r64, r/m8
#define X64OP1_MOVSXB		0x0f
#define X64OP2_MOVSXB		0xbe

// Movsxh instruction
// move with sign extenstion from 16-bits
// MOVSX r32, r/m16
// MOVSX r64, r/m16
#define X64OP1_MOVSXH		0x0f
#define X64OP2_MOVSXH		0xbf

// Movsxd instruction
// move with sign extension from 32-bits to 64-bits
// MOVSXD r64, r/m32
#define X64OP_MOVSXD		0x63



// ** movzx instruction ** //

// Movzxb instruction
// move with zero extenstion from 16-bits
// MOVSX r16, r/m8
// MOVSX r32, r/m8
// MOVSX r64, r/m8
#define X64OP1_MOVZXB		0x0f
#define X64OP2_MOVZXB		0xb6

// Movzxh instruction
// move with zero extenstion from 16-bits
// MOVSX r32, r/m16
// MOVSX r64, r/m16
#define X64OP1_MOVZXH		0x0f
#define X64OP2_MOVZXH		0xb7


// Mul instruction
// Unsigned multiply (RDX:RAX <- RAX * r/m64
// MUL r/m64
// MUL r/m32
// MUL r/m16
#define X64OP_MUL			0xf7
#define MODRM_MUL			0x4

// Neg instruction
// Two's complement negate r/m64
// NEG r/m64
// NEG r/m32
// NEG r/m16
#define X64OP_NEG			0xf7
#define MODRM_NEG			0x3

// Not instruction
// NOT r/m64
// NOT r/m32
// NOT r/m16
#define X64OP_NOT			0xf7
#define MODRM_NOT			0x2

// Inc instruction
// INC r/m64
// INC r/m32
// INC r/m16
#define X64OP_INC			0xff
#define MODRM_INC			0x0

// Dec instruction
// DEC r/m64
// DEC r/m32
// DEC r/m16
#define X64OP_DEC			0xff
#define MODRM_DEC			0x1


// *** or instructions ** //

// Or instruction
// OR r64, r/m64
// OR r32, r/m32 
// OR r16, r/m16
#define X64OP_OR			0x0b

// OR r/m16, r16
// OR r/m32, r32
// OR r/m64, r64
#define X64OP_OR_TOMEM		0x09

// OR r/m8, r8
#define X64OP_OR_TOMEM8		0x08

// Or instruction
// OR r64, immed32
// OR r32, immed32
#define X64OP_OR_IMM		0x81
#define MODRM_OR_IMM		1

// or immediate 8-bit
// OR r/m64, imm8
// OR r/m32, imm8
// OR r/m16, imm8
#define X64OP_OR_IMM8		0x83
#define MODRM_OR_IMM8		0x1

// or immedate to Accumulator
// OR AX, imm16
// OR EAX, imm32
// OR RAX, imm32
#define X64OP_OR_IMMA		0x0d


// ** pop instruction ** //
// POP r16
// POP r32
// POP r64
#define X64BOP_POP			0x58

// ** push instruction ** //
// PUSH r16
// PUSH r32
// PUSH r64
#define X64BOP_PUSH			0x50

// PUSH Imm8
#define X64OP_PUSH_IMM8		0x6a

// PUSH Imm16
// PUSH Imm32
#define X64OP_PUSH_IMM		0x68


// ** call instruction ** //
// Call instruction
// CALL rel32
#define X64OP_CALL			0xe8


// ** ret instruction ** //

// Ret instruction
// Near return to calling procedure
// RET
#define X64OP_RET			0xc3


// ** set on condition ** //

// seta - set on unsigned above
// SETA r/m8
#define X64OP1_SETA			0x0f
#define X64OP2_SETA			0x97

// setae - set on unsigned above or equal
// SETAE r/m8
#define X64OP1_SETAE			0x0f
#define X64OP2_SETAE			0x93

// setb - set on unsigned below
// SETB r/m8
#define X64OP1_SETB			0x0f
#define X64OP2_SETB			0x92

// setbe - set on unsigned below or equal
// SETBE r/m8
#define X64OP1_SETBE		0x0f
#define X64OP2_SETBE		0x96

// sete - set on equal
// SETE r/m8
#define X64OP1_SETE			0x0f
#define X64OP2_SETE			0x94

// setne - set on unsigned below
// SETNE r/m8
#define X64OP1_SETNE		0x0f
#define X64OP2_SETNE		0x95

// setl - set on signed less than
// SETL r/m8
#define X64OP1_SETL			0x0f
#define X64OP2_SETL			0x9c

// setle - set on signed less than or equal
// SETLE r/m8
#define X64OP1_SETLE		0x0f
#define X64OP2_SETLE		0x9e

// setg - set on signed greater than
// SETG r/m8
#define X64OP1_SETG			0x0f
#define X64OP2_SETG			0x9f

// setge - set on signed greater than or equal
// SETGE r/m8
#define X64OP1_SETGE		0x0f
#define X64OP2_SETGE		0x9d

// sets - set on signed
// SETS r/m8
#define X64OP1_SETS			0x0f
#define X64OP2_SETS			0x98

// setpo - set on parity odd
// SETPO r/m8
#define X64OP1_SETPO		0x0f
#define X64OP2_SETPO		0x9b


// popcnt - population count
// popcnt r/m32
#define X64OP1_POPCNT		0xf3
#define X64OP2_POPCNT		0x0f
#define X64OP3_POPCNT		0xb8



// **** shift instructions **** //

// ** shl instruction - shift left (logical) ** //

// Multiply r/m by 2, CL times
// SHL r/m16, CL
// SHL r/m32, CL
// SHL r/m64, CL
#define X64OP_SHL			0xd3
#define MODRM_SHL			0x4

// Multiply r/m by 2, imm8 times
// SHL r/m16, imm8
// SHL r/m32, imm8
// SHL r/m64, imm8
#define X64OP_SHL_IMM		0xc1
#define MODRM_SHL_IMM		0x4


// ** shift arithmetic right - sar instruction ** //

// Signed divide* r/m16 by 2, CL times
// SAR r/m16, CL
// SAR r/m32, CL
// SAR r/m64, CL
#define X64OP_SAR			0xd3
#define MODRM_SAR			0x7

// Signed divide* r/m by 2, imm8 times
// SAR r/m16, imm8
// SAR r/m32, imm8
// SAR r/m64, imm8
#define X64OP_SAR_IMM		0xc1
#define MODRM_SAR_IMM		0x7

// ** shift logical right - shr instruction ** //

// Unsigned divide r/m by 2, CL times
// SHR r/m16, CL
// SHR r/m32, CL
// SHR r/m64, CL
#define X64OP_SHR			0xd3
#define MODRM_SHR			0x5

// Unsigned divide r/m16 by 2, imm8 times
// SHR r/m16, imm8
// SHR r/m32, imm8
// SHR r/m64, imm8
#define X64OP_SHR_IMM		0xc1
#define MODRM_SHR_IMM		0x5

// ** sub instructions ** //

// Sub instruction
// Subtract r/m64 from r64
// r64 = r64 - r/m64
// SUB r64, r/m64
// SUB r32, r/m32
#define X64OP_SUB			0x2b

// SUB r/m16, r16
// SUB r/m32, r32
// SUB r/m64, r64
#define X64OP_SUB_TOMEM		0x29

// Sub Immediate instruction
// SUB r/m64, imm32
// SUB r/m32, imm32
#define X64OP_SUB_IMM		0x81
#define MODRM_SUB_IMM		0x5

// Sub immediate 8-bit
// SUB r/m64, imm8
// SUB r/m32, imm8
// SUB r/m16, imm8
#define X64OP_SUB_IMM8		0x83
#define MODRM_SUB_IMM8		0x5

// Sub immedate to Accumulator
// SUB AX, imm16
// SUB EAX, imm32
// SUB RAX, imm32
#define X64OP_SUB_IMMA		0x2d


// ** test instructions ** //

// TEST r/m16, r16
// TEST r/m32, r32
// TEST r/m64, r64
#define X64OP_TEST_TOMEM		0x85

// test immediate instruction
// TEST r/m64, imm32
// TEST r/m32, imm32
#define X64OP_TEST_IMM		0xf7
#define MODRM_TEST_IMM		0

// Test immediate 8-bit
// TEST r/m64, imm8
// TEST r/m32, imm8
// TEST r/m16, imm8
#define X64OP_TEST_IMM8		0xf6
#define MODRM_TEST_IMM8		0x0

// Test immedate to Accumulator
// TEST AX, imm16
// TEST EAX, imm32
// TEST RAX, imm32
#define X64OP_TEST_IMMA		0xa9



// ** xchg instructions ** //

// xchg instruction
// XCHG r64, r/m64
// XCHG r32, r/m32
#define X64OP_XCHG			0x87

// ** xor instructions ** //

// xor instruction

// XOR r8, r/m8
#define X64OP_XOR8			0x32

// XOR r64, r/m64
// XOR r32, r/m32
#define X64OP_XOR			0x33

// XOR r/m16, r16
// XOR r/m32, r32
// XOR r/m64, r64
#define X64OP_XOR_TOMEM		0x31

// xor 8-bit immediate 8-bit
// XOR r/m8, imm8
#define X64OP_XOR8_IMM8		0x80
#define MODRM_XOR8_IMM8		6


// xor immediate instruction
// XOR r/m64, imm32
// XOR r/m32, imm32
#define X64OP_XOR_IMM		0x81
#define MODRM_XOR_IMM		6

// Xor immediate 8-bit
// XOR r/m64, imm8
// XOR r/m32, imm8
// XOR r/m16, imm8
#define X64OP_XOR_IMM8		0x83
#define MODRM_XOR_IMM8		0x6

// Xor immedate to Accumulator
// XOR AX, imm16
// XOR EAX, imm32
// XOR RAX, imm32
#define X64OP_XOR_IMMA		0x35


// **** SSE/AVX Instructions **** //

// ****** SSE ***** //


// Move doubleword to SSE from register/memory
// 66 REX.W 0F 6E /r
// MOVD xmm, r/m32
#define X64OP1_MOVD_FROMMEM	0x0f
#define X64OP2_MOVD_FROMMEM	0x6e


// Move doubleword from SSE to register/memory
// 66 0F 7E /r
// MOVD r/m32, xmm
#define X64OP1_MOVD_TOMEM	0x0f
#define X64OP2_MOVD_TOMEM	0x7e

// Move quadword to SSE from register/memory
// 66 0F 6E /r
// MOVQ xmm, r/m64
#define X64OP1_MOVQ_FROMMEM	0x0f
#define X64OP2_MOVQ_FROMMEM	0x6e


// Move quadword from SSE to register/memory
// 66 REX.W 0F 7E /r
// MOVQ r/m64, xmm
#define X64OP1_MOVQ_TOMEM	0x0f
#define X64OP2_MOVQ_TOMEM	0x7e


// Add scalar double precision floating point values
// F2 0F 58 /r
// ADDSD xmm1, xmm2/m64
#define X64OP1_ADDSD		0x0f
#define X64OP2_ADDSD		0x58


// Subtract scalar double precision floating point values
// F2 0F 5C /r
// SUBSD xmm1, xmm2/m64
#define X64OP1_SUBSD		0x0f
#define X64OP2_SUBSD		0x5C


// Multiply scalar double precision floating point values
// F2 0F 59 /r
// MULSD xmm1, xmm2/m64
#define X64OP1_MULSD		0x0f
#define X64OP2_MULSD		0x59


// Divide scalar double precision floating point values
// F2 0F 5E /r
// DIVSD xmm1, xmm2/m64
#define X64OP1_DIVSD		0x0f
#define X64OP2_DIVSD		0x5E


// Square root scalar double precision floating point value
// F2 0F 51 /r
// SQRTSD xmm1, xmm2/m64
#define X64OP1_SQRTSD		0x0f
#define X64OP2_SQRTSD		0x51


// convert with truncation scalar single precision floating point value to general purpose register signed doubleword
// F3 0F 2C /r
// CVTTSS2SI r32, xmm1/m32
#define X64OP1_CVTTSS2SI	0x0f
#define X64OP2_CVTTSS2SI	0x2c

// convert with truncation packed single precision floating point value to packed general purpose register signed doubleword
// F3 0F 5B /r
// CVTTPS2DQ r32, xmm1/m32
#define X64OP1_CVTTPS2DQ	0x0f
#define X64OP2_CVTTPS2DQ	0x5b


// convert doubleword integer to scalar double precision floating point value
// F2 0F 2A /r
// CVTSI2SD xmm1, r32/m32
#define X64OP1_CVTSI2SD		0x0f
#define X64OP2_CVTSI2SD		0x2a

// convert packed doubleword integers to packed double precision floating point values
// F3 0F E6 /r
// CVTSI2SD xmm1, r32/m32
#define X64OP1_CVTDQ2PD		0x0f
#define X64OP2_CVTDQ2PD		0xe6

// replicate lower double floating point value
// F2 0F 12 /r
// MOVDDUP xmm1, xmm2/m64
#define X64OP1_MOVDDUP		0x0f
#define X64OP2_MOVDDUP		0x12




// ****** AVX ***** //

// *** floating point instructions *** //

// VBLENDVPS - blend 32-bit values with most significant bit as selector
// VBLENDVPS xmm1, xmm2, xmm3/m128, xmm4
// VEX.NDS.128.66.0F3A.W0 4A /r /is4
#define AVXOP_BLENDVPS			0x4a
#define PP_BLENDVPS				VEX_PREFIX66
#define MMMMM_BLENDVPS			VEX_PREFIX0F3A

// VBLENDVPD ymm1, ymm2, ymm3/m256, ymm4
// VEX.NDS.256.66.0F3A.W0 4B /r /is4
// VBLENDVPD ymm1, ymm2, ymm3/m256, ymm4
#define AVXOP_BLENDVPD			0x4b
#define PP_BLENDVPD				VEX_PREFIX66
#define MMMMM_BLENDVPD			VEX_PREFIX0F3A

// VBLENDPS - blend packed 32-bit values
// VBLENDPS xmm1, xmm2, xmm3/m128, imm8
// VEX.NDS.128.66.0F3A.WIG 0C /r ib
#define AVXOP_BLENDPS			0x0c
#define PP_BLENDPS				VEX_PREFIX66
#define MMMMM_BLENDPS			VEX_PREFIX0F3A

// VBLENDPD ymm1, ymm2, ymm3/m256, imm8
// VEX.NDS.256.66.0F3A.WIG 0D /r ib
// VBLENDPD ymm1, ymm2, ymm3/m256, imm8
#define AVXOP_BLENDPD			0x0d
#define PP_BLENDPD				VEX_PREFIX66
#define MMMMM_BLENDPD			VEX_PREFIX0F3A

// VMOVAPD - move packed double precision float from mem
// VEX.256.66.0F.WIG 28 /r
// VMOVAPD ymm1, ymm2/m256
#define AVXOP_MOVAPD_FROMMEM	0x28
#define PP_MOVAPD_FROMMEM		VEX_PREFIX66
#define MMMMM_MOVAPD_FROMMEM	VEX_PREFIX0F

// VMOVAPD - move packed double precision float to mem
// VEX.256.66.0F.WIG 29 /r
// VMOVAPD ymm2/m256, ymm1
#define AVXOP_MOVAPD_TOMEM		0x29
#define PP_MOVAPD_TOMEM			VEX_PREFIX66
#define MMMMM_MOVAPD_TOMEM		VEX_PREFIX0F

// VEXTRACTPS
// VEXTRACTPS r/m32, xmm1, imm8
// VEX.128.66.0F3A.WIG 17 /r ib
#define AVXOP_EXTRACTPS			0x17
#define PP_EXTRACTPS			VEX_PREFIX66
#define MMMMM_EXTRACTPS			VEX_PREFIX0F3A

// VMOVMSKPS
// VMOVMSKPS reg, ymm2
// VEX.256.0F.WIG 50 /r
#define AVXOP_MOVMSKPS			0x50
#define PP_MOVMSKPS				VEX_PREFIXNONE
#define MMMMM_MOVMSKPS			VEX_PREFIX0F

// VADDPS - add packed single-precision floating point values
// VADDPS xmm1,xmm2, xmm3/m128
// VEX.NDS.128.0F.WIG 58 /r
// VADDPS ymm1, ymm2, ymm3/m256
// VEX.NDS.256.0F.WIG 58 /r
#define AVXOP_ADDPS				0x58
#define PP_ADDPS				VEX_PREFIXNONE
#define MMMMM_ADDPS				VEX_PREFIX0F

// VADDSS - add single-precision floating point values
// VADDSS xmm1,xmm2, xmm3/m32
// VEX.NDS.LIG.F3.0F.WIG 58 /r
#define AVXOP_ADDSS				0x58
#define PP_ADDSS				VEX_PREFIXF3
#define MMMMM_ADDSS				VEX_PREFIX0F

// VADDPD  - add of packed double-precision floating point values
// VADDPD xmm1,xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 58 /r
// VADDPD ymm1, ymm2, ymm3/m256
// VEX.NDS.256.66.0F.WIG 58 /r
#define AVXOP_ADDPD				0x58
#define PP_ADDPD				VEX_PREFIX66
#define MMMMM_ADDPD				VEX_PREFIX0F

// VADDSD - add a single double precision float value
// VEX.NDS.LIG.F2.0F.WIG 58 /r
// VADDSD xmm1, xmm2, xmm3/m64
#define AVXOP_ADDSD				0x58
#define PP_ADDSD				VEX_PREFIXF2
#define MMMMM_ADDSD				VEX_PREFIX0F

// VANDPS - logical AND of packed values
// VANDPS xmm1,xmm2, xmm3/m128
// VEX.NDS.128.0F.WIG 54 /r
// VANDPS ymm1, ymm2, ymm3/m256
// VEX.NDS.256.0F.WIG 54 /r
#define AVXOP_ANDPS				0x54
#define PP_ANDPS				VEX_PREFIXNONE
#define MMMMM_ANDPS				VEX_PREFIX0F

// VANDPD - logical and packed double precision float values
// VANDPD xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 54 /r
// VANDPD ymm1, ymm2, ymm3/m256
// VEX.NDS.256.66.0F.WIG 54 /r
#define AVXOP_ANDPD				0x54
#define PP_ANDPD				VEX_PREFIX66
#define MMMMM_ANDPD				VEX_PREFIX0F

// VANDNPS - logical AND packed single-precision float values
// VANDNPS xmm1, xmm2, xmm3/m128
// VEX.NDS.128.0F.WIG 55 /r
// VANDNPS ymm1, ymm2, ymm3/m256
// VEX.NDS.256.0F.WIG 55 /r
#define AVXOP_ANDNPS			0x55
#define PP_ANDNPS				VEX_PREFIXNONE
#define MMMMM_ANDNPS			VEX_PREFIX0F

// VANDNPD - logial and-not packed double precision float values
// VEX.NDS.256.66.0F.WIG 55/r
// VANDNPD ymm1, ymm2, ymm3/m256
#define AVXOP_ANDNPD			0x55
#define PP_ANDNPD				VEX_PREFIX66
#define MMMMM_ANDNPD			VEX_PREFIX0F

// VCMPPS - compare of packed single-precision floats
// VCMPPS xmm1, xmm2, xmm3/m128, imm8
// VEX.NDS.128.0F.WIG C2 /r ib
#define AVXOP_CMPPS				0xc2
#define PP_CMPPS				VEX_PREFIXNONE
#define MMMMM_CMPPS				VEX_PREFIX0F

// VCMPSS - compare of single-precision floats
// VCMPSS xmm1, xmm2, xmm3/m32, imm8
// VEX.NDS.LIG.F3.0F.WIG C2 /r ib
#define AVXOP_CMPSS				0xc2
#define PP_CMPSS				VEX_PREFIXF3
#define MMMMM_CMPSS				VEX_PREFIX0F

// VCMPSD - compare double-precision floats
// VCMPSD xmm1, xmm2, xmm3/m64, imm8
// VEX.NDS.LIG.F2.0F.WIG C2 /r ib
#define AVXOP_CMPSD				0xc2
#define PP_CMPSD				VEX_PREFIXF2
#define MMMMM_CMPSD				VEX_PREFIX0F

// VCMPPD - compare packed double precision float values
// VEX.NDS.256.66.0F.WIG C2 /r ib
// VCMPPD ymm1, ymm2, ymm3/m256, imm8
#define AVXOP_CMPPD				0xc2
#define PP_CMPPD				VEX_PREFIX66
#define MMMMM_CMPPD				VEX_PREFIX0F

// VCMP comparison constants
#define EQ_OQ					0x0
#define NEQ_OQ					0xc
#define LT_OQ					0x11
#define LE_OQ					0x12
#define GE_OQ					0x1d
#define GT_OQ					0x1e
#define UNORD_Q					0x3
#define ORD_Q					0x7
#define TRUE_US					0x1f


// VCVTDQ2PD - convert packed 32-bit integers into 4 double-precision floats
// VCVTDQ2PD ymm1, xmm2/m128
// VEX.256.F3.0F.WIG E6 /r
#define AVXOP_CVTDQ2PD				0xe6
#define PP_CVTDQ2PD					VEX_PREFIXF3
#define MMMMM_CVTDQ2PD				VEX_PREFIX0F

// VCVTDQ2PS - convert packed 32-bit integers into 4 double-precision floats
// VCVTDQ2PS xmm1, xmm2/m128
// VEX.128.0F.WIG 5B /r
#define AVXOP_CVTDQ2PS				0x5b
#define PP_CVTDQ2PS					VEX_PREFIXNONE
#define MMMMM_CVTDQ2PS				VEX_PREFIX0F

// VCVTPD2DQ - convert packed double-precision floats to 4 packed 32-bit integers
// VCVTPD2DQ xmm1, ymm2/m256
// VEX.256.F2.0F.WIG E6 /r
#define AVXOP_CVTPD2DQ				0xe6
#define PP_CVTPD2DQ					VEX_PREFIXF2
#define MMMMM_CVTPD2DQ				VEX_PREFIX0F

// VCVTPS2DQ - convert packed single-precision floats to 4 packed 32-bit integers
// VCVTPS2DQ xmm1, xmm2/m128
// VEX.128.66.0F.WIG 5B /r
#define AVXOP_CVTPS2DQ				0x5b
#define PP_CVTPS2DQ					VEX_PREFIX66
#define MMMMM_CVTPS2DQ				VEX_PREFIX0F

// VCVTPS2PD - convert packed single-precision float values to packed double-precision
// VCVTPS2PD ymm1, xmm2/m128
// VEX.256.0F.WIG 5A /r
#define AVXOP_CVTPS2PD			0x5a
#define PP_CVTPS2PD				VEX_PREFIXNONE
#define MMMMM_CVTPS2PD			VEX_PREFIX0F

// VCVTPD2PS - convert packed double precision floats to packed single precision
// VCVTPD2PS xmm1, ymm2/m256
// VEX.256.66.0F.WIG 5A /r
#define AVXOP_CVTPD2PS			0x5a
#define PP_CVTPD2PS				VEX_PREFIX66
#define MMMMM_CVTPD2PS			VEX_PREFIX0F

// VDIVPS - divide
// VDIVPS xmm1, xmm2, xmm3/m128
// VEX.NDS.128.0F.WIG 5E /r
#define AVXOP_DIVPS				0x5e
#define PP_DIVPS				VEX_PREFIXNONE
#define MMMMM_DIVPS				VEX_PREFIX0F

// VDIVSS - divide
// VDIVSS xmm1, xmm2, xmm3/m32
// VEX.NDS.LIG.F3.0F.WIG 5E /r
#define AVXOP_DIVSS				0x5e
#define PP_DIVSS				VEX_PREFIXF3
#define MMMMM_DIVSS				VEX_PREFIX0F

// VDIVPD - divide packed double precision floats
// VEX.NDS.256.66.0F.WIG 5E /r
// VDIVPD ymm1, ymm2, ymm3/m256
#define AVXOP_DIVPD				0x5e
#define PP_DIVPD				VEX_PREFIX66
#define MMMMM_DIVPD				VEX_PREFIX0F

// VDIVSD - dvvide a single double precision float
// VEX.NDS.LIG.F2.0F.WIG 5E /r
// VDIVSD xmm1, xmm2, xmm3/m64
#define AVXOP_DIVSD				0x5e
#define PP_DIVSD				VEX_PREFIXF2
#define MMMMM_DIVSD				VEX_PREFIX0F

// VMAXPD ymm1, ymm2, ymm3/m256
// VEX.NDS.256.66.0F.WIG 5F /r
// VMAXPD ymm1, ymm2, ymm3/m256
#define AVXOP_MAXPD				0x5f
#define PP_MAXPD				VEX_PREFIX66
#define MMMMM_MAXPD				VEX_PREFIX0F

// VMAXSD - max of single double precision float value
// VEX.NDS.LIG.F2.0F.WIG 5F /r
// VMAXSD xmm1, xmm2, xmm3/m64
#define AVXOP_MAXSD				0x5f
#define PP_MAXSD				VEX_PREFIXF2
#define MMMMM_MAXSD				VEX_PREFIX0F

// VMAXPS - get minimum of packed single-precision floats
// VMAXPS xmm1,xmm2, xmm3/m128
// VEX.NDS.128.0F.WIG 5F /r
#define AVXOP_MAXPS				0x5f
#define PP_MAXPS				VEX_PREFIXNONE
#define MMMMM_MAXPS				VEX_PREFIX0F

// VMAXSS - get minimum of single-precision floats
// VMAXSS xmm1, xmm2, xmm3/m32
// VEX.NDS.LIG.F3.0F.WIG 5F /r
#define AVXOP_MAXSS				0x5f
#define PP_MAXSS				VEX_PREFIXF3
#define MMMMM_MAXSS				VEX_PREFIX0F

// VMINPS - get minimum of packed single-precision floats
// VMINPS xmm1,xmm2, xmm3/m128
// VEX.NDS.128.0F.WIG 5D /r
#define AVXOP_MINPS				0x5d
#define PP_MINPS				VEX_PREFIXNONE
#define MMMMM_MINPS				VEX_PREFIX0F

// VMINSS - get minimum of single-precision floats
// VMINSS xmm1,xmm2, xmm3/m32
// VEX.NDS.LIG.F3.0F.WIG 5D /r
#define AVXOP_MINSS				0x5d
#define PP_MINSS				VEX_PREFIXF3
#define MMMMM_MINSS				VEX_PREFIX0F

// VMINPD - get minimum of packed double precision floats
// VEX.NDS.256.66.0F.WIG 5D /r
// VMINPD ymm1, ymm2, ymm3/m256
#define AVXOP_MINPD				0x5d
#define PP_MINPD				VEX_PREFIX66
#define MMMMM_MINPD				VEX_PREFIX0F

// VMINSD - get minimum of single double precision float
// VEX.NDS.LIG.F2.0F.WIG 5D /r
// VMINSD xmm1, xmm2, xmm3/m64
#define AVXOP_MINSD				0x5d
#define PP_MINSD				VEX_PREFIXF2
#define MMMMM_MINSD				VEX_PREFIX0F

// VMULSS - multiply packed single-precision floats
// VMULSS xmm1,xmm2, xmm3/m32
// VEX.NDS.LIG.F3.0F.WIG 59 /r
#define AVXOP_MULSS				0x59
#define PP_MULSS				VEX_PREFIXF3
#define MMMMM_MULSS				VEX_PREFIX0F

// VMULPS - multiply packed single-precision floats
// VMULPS xmm1,xmm2, xmm3/m128
// VEX.NDS.128.0F.WIG 59 /r
#define AVXOP_MULPS				0x59
#define PP_MULPS				VEX_PREFIXNONE
#define MMMMM_MULPS				VEX_PREFIX0F

// VMULSD - multiply single-precision floats
// VMULSD xmm1,xmm2, xmm3/m64
// VEX.NDS.LIG.F2.0F.WIG 59/r
#define AVXOP_MULSD				0x59
#define PP_MULSD				VEX_PREFIXF2
#define MMMMM_MULSD				VEX_PREFIX0F

// VMULPD - multiply packed double precision floats
// VEX.NDS.256.66.0F.WIG 59 /r
// VMULPD ymm1, ymm2, ymm3/m256
#define AVXOP_MULPD				0x59
#define PP_MULPD				VEX_PREFIX66
#define MMMMM_MULPD				VEX_PREFIX0F

// VORPS - packed logical OR
// VORPS xmm1, xmm2, xmm3/m128
// VEX.NDS.128.0F.WIG 56 /r
#define AVXOP_ORPS				0x56
#define PP_ORPS					VEX_PREFIXNONE
#define MMMMM_ORPS				VEX_PREFIX0F

// VORPD - logical or of packed double precision float values
// VEX.NDS.256.66.0F.WIG 56 /r
// VORPD ymm1, ymm2, ymm3/m256
#define AVXOP_ORPD				0x56
#define PP_ORPD					VEX_PREFIX66
#define MMMMM_ORPD				VEX_PREFIX0F

// VRSQRTPS - reverse square root
// VRSQRTPS xmm1, xmm2/m128
// VEX.128.0F.WIG 52 /r
#define AVXOP_RSQRTPS			0x52
#define PP_RSQRTPS				VEX_PREFIXNONE
#define MMMMM_RSQRTPS			VEX_PREFIX0F

// VRSQRTSS - reverse square root
// VRSQRTSS xmm1, xmm2, xmm3/m32
// VEX.NDS.LIG.F3.0F.WIG 52 /r
#define AVXOP_RSQRTSS			0x52
#define PP_RSQRTSS				VEX_PREFIXF3
#define MMMMM_RSQRTSS			VEX_PREFIX0F

// VSHUFPS - shuffle of packed single-precision float values
// VSHUFPS xmm1, xmm2, xmm3/m128, imm8
// VEX.NDS.128.0F.WIG C6 /r ib
#define AVXOP_SHUFPS			0xc6
#define PP_SHUFPS				VEX_PREFIXNONE
#define MMMMM_SHUFPS			VEX_PREFIX0F

// VSHUFPD - shuffle of packed double precision float values
// VEX.NDS.256.66.0F.WIG C6 /r ib
// VSHUFPD ymm1, ymm2, ymm3/m256, imm8
#define AVXOP_SHUFPD			0xc6
#define PP_SHUFPD				VEX_PREFIX66
#define MMMMM_SHUFPD			VEX_PREFIX0F

// VSQRTPS - packed single-precision square root
// VSQRTPS xmm1, xmm2/m128
// VEX.128.0F.WIG 51 /r
#define AVXOP_SQRTPS			0x51
#define PP_SQRTPS				VEX_PREFIXNONE
#define MMMMM_SQRTPS			VEX_PREFIX0F

// VSQRTSS - single-precision square root
// VSQRTSS xmm1, xmm2, xmm3/m32
// VEX.NDS.LIG.F3.0F.WIG 51
#define AVXOP_SQRTSS			0x51
#define PP_SQRTSS				VEX_PREFIXF3
#define MMMMM_SQRTSS			VEX_PREFIX0F

// VSQRTPD - square root of packed double precision float values
// VEX.256.66.0F.WIG 51/r
// VSQRTPD ymm1, ymm2/m256
#define AVXOP_SQRTPD			0x51
#define PP_SQRTPD				VEX_PREFIX66
#define MMMMM_SQRTPD			VEX_PREFIX0F

// VSQRTSD  - square root of single double precision float value
// VEX.NDS.LIG.F2.0F.WIG 51/
// VSQRTSD xmm1,xmm2, xmm3/m64
#define AVXOP_SQRTSD			0x51
#define PP_SQRTSD				VEX_PREFIXF2
#define MMMMM_SQRTSD			VEX_PREFIX0F

// subtract packed double-precision floats
// 66 0F 5C /r
// SUBPD xmm1, xmm2/m128
#define X64OP1_SUBPD		0x0f
#define X64OP2_SUBPD		0x5c

// subtract packed single-precision floats
// 0F 5C /r
// SUBPS xmm1, xmm2/m128
#define X64OP1_SUBPS		0x0f
#define X64OP2_SUBPS		0x5c


// VSUBPS - subtract packed single-precision float values
// VSUBPS xmm1,xmm2, xmm3/m128
// VEX.NDS.128.0F.WIG 5C /r
// VSUBPS ymm1, ymm2, ymm3/m256
// VEX.NDS.256.0F.WIG 5C /r
#define AVXOP_SUBPS				0x5c
#define PP_SUBPS				VEX_PREFIXNONE
#define MMMMM_SUBPS				VEX_PREFIX0F

// VSUBSS - subtract single-precision float values
// VSUBSS xmm1,xmm2, xmm3/m32
// VEX.NDS.LIG.F3.0F.WIG 5C /r
#define AVXOP_SUBSS				0x5c
#define PP_SUBSS				VEX_PREFIXF3
#define MMMMM_SUBSS				VEX_PREFIX0F

// VSUBPD - subtract packed double precision float values
// VEX.NDS.256.66.0F.WIG 5C /r
// VSUBPD ymm1, ymm2, ymm3/m256
#define AVXOP_SUBPD				0x5c
#define PP_SUBPD				VEX_PREFIX66
#define MMMMM_SUBPD				VEX_PREFIX0F

// VSUBSD - subtract single double precision float value
// VEX.NDS.LIG.F2.0F.WIG 5C /r
// VSUBSD xmm1,xmm2, xmm3/m64
#define AVXOP_SUBSD				0x5c
#define PP_SUBSD				VEX_PREFIXF2
#define MMMMM_SUBSD				VEX_PREFIX0F

// VPERM2F128 - permute/shuffle two 128-bit values
// VPERM2F128 ymm1, ymm2, ymm3/m256, imm8
// VEX.NDS.256.66.0F3A.W0 06 /r ib
#define AVXOP_VPERM2F128		0x06
#define PP_VPERM2F128			VEX_PREFIX66
#define MMMMM_VPERM2F128		VEX_PREFIX0F3A

// VXORPS - logical XOR of packed values
// VXORPS xmm1,xmm2, xmm3/m128
// VEX.NDS.128.0F.WIG 57 /r
// VXORPS ymm1, ymm2, ymm3/m256
// VEX.NDS.256.0F.WIG 57 /r
#define AVXOP_XORPS				0x57
#define PP_XORPS				VEX_PREFIXNONE
#define MMMMM_XORPS				VEX_PREFIX0F

// VXORPD - xor of packed double precision float values
// VEX.NDS.256.66.0F.WIG 57 /r
// VXORPD ymm1, ymm2, ymm3/m256
#define AVXOP_XORPD				0x57
#define PP_XORPD				VEX_PREFIX66
#define MMMMM_XORPD				VEX_PREFIX0F


// *** integer instructions *** //

// ** pblendw instruction ** //

// VPBLENDW xmm1, xmm2, xmm3/m128, imm8
// VEX.NDS.128.66.0F3A.WIG 0E /r ib
#define AVXOP_PBLENDW			0x0e

// ** pinsr instruction ** //

// VPINSRB xmm1, xmm2, r32/m8, imm8
// VEX.NDS.128.66.0F3A.W0 20 /r ib
#define AVXOP_PINSRB			0x20

// VPINSRW xmm1, xmm2, r32/m16, imm8
// VEX.NDS.128.66.0F.W0 C4 /r ib
#define AVXOP_PINSRW			0xc4

// VPINSRD xmm1, xmm2, r32/m32, imm8
// VEX.NDS.128.66.0F3A.W0 22 /r ib
#define AVXOP_PINSRD			0x22

// VPINSRQ xmm1, xmm2, r64/m64, imm8
// VEX.NDS.128.66.0F3A.W1 22 /r ib
#define AVXOP_PINSRQ			0x22

// ** pextr instruction ** //

// VPEXTRB reg/m8, xmm2, imm8
// VEX.128.66.0F3A.W0 14 /r ib
#define AVXOP_PEXTRB			0x14

// VPEXTRW reg/m16, xmm2, imm8
// VEX.128.66.0F3A.W0 15 /r ib
#define AVXOP_PEXTRW			0x15

// VPEXTRD r32/m32, xmm2, imm8
// VEX.128.66.0F3A.W0 16 /r ib
#define AVXOP_PEXTRD			0x16

// VPEXTRQ r64/m64, xmm2, imm8
// VEX.128.66.0F3A.W1 16 /r ib
#define AVXOP_PEXTRQ			0x16

// ** movss instructions ** //

// movss - move scalar single precision floating point
// VMOVSS xmm1, m32
// VEX.LIG.F3.0F.WIG 10 /r
#define AVXOP_MOVSS_FROMMEM		0x10

// VMOVSS m32, xmm1
// VEX.LIG.F3.0F.WIG 11 /r
#define AVXOP_MOVSS_TOMEM		0x11


// ** movaps instruction ** //

// VMOVAPS xmm1, xmm2/m128
// VEX.128.0F.WIG 28 /r
#define AVXOP_MOVAPS_FROMMEM		0x28
#define PP_MOVAPS_FROMMEM			VEX_PREFIXNONE
#define MMMMM_MOVAPS_FROMMEM		VEX_PREFIX0F

// VMOVAPS xmm2/m128, xmm1
// VEX.128.0F.WIG 29 /r
#define AVXOP_MOVAPS_TOMEM		0x29
#define PP_MOVAPS_TOMEM			VEX_PREFIXNONE
#define MMMMM_MOVAPS_TOMEM		VEX_PREFIX0F

// ** movapd instruction ** //

// VMOVAPD - move packed aligned data from/to memory
// VMOVAPD ymm1, ymm2/m256
// VEX.256.66.0F.WIG 28 /r
#define AVXOP_MOVAPD_FROMMEM	0x28
#define PP_MOVAPD_FROMMEM		VEX_PREFIX66
#define MMMMM_MOVAPD_FROMMEM	VEX_PREFIX0F

// VMOVAPD ymm2/m256, ymm1
// VEX.256.66.0F.WIG 29 /r
#define AVXOP_MOVAPD_TOMEM		0x29
#define PP_MOVAPD_TOMEM			VEX_PREFIX66
#define MMMMM_MOVAPD_TOMEM		VEX_PREFIX0F

// VBROADCAST
// VBROADCASTSS xmm1, m32
// VEX.128.66.0F38.W0 18 /r
#define AVXOP_VBROADCASTSS		0x18
#define PP_VBROADCASTSS			VEX_PREFIX66
#define MMMMM_VBROADCASTSS		VEX_PREFIX0F38

// VBROADCASTSD ymm1, m64
// VEX.256.66.0F38.W0 19 /r
#define AVXOP_VBROADCASTSD		0x19
#define PP_VBROADCASTSD			VEX_PREFIX66
#define MMMMM_VBROADCASTSD		VEX_PREFIX0F38

// VMASKMOV
// VMASKMOVPS m128, xmm1, xmm2
// VEX.NDS.128.66.0F38.W0 2E /r
#define AVXOP_VMASKMOVPS_TOMEM	0x2e
#define PP_VMASKMOVPS_TOMEM		VEX_PREFIX66
#define MMMMM_VMASKMOVPS_TOMEM	VEX_PREFIX0F38

// VMASKMOVPD m256, ymm1, ymm2
// VEX.NDS.256.66.0F38.W0 2F /r
#define AVXOP_VMASKMOVPD_TOMEM	0x2f
#define PP_VMASKMOVPD_TOMEM		VEX_PREFIX66
#define MMMMM_VMASKMOVPD_TOMEM	VEX_PREFIX0F38

// ** movdqa instruction ** //

#define PP_VMOVDQA				VEX_PREFIX66
#define MMMMM_VMOVDQA			VEX_PREFIX0F

// MOVDQA—Move Aligned Double Quadword
// VMOVDQA xmm1, xmm2/m128
// VEX.128.66.0F.WIG 6F /r
#define AVXOP_MOVDQA_FROMMEM	0x6f

// VMOVDQA xmm2/m128, xmm1
// VEX.128.66.0F.WIG 7F /r
#define AVXOP_MOVDQA_TOMEM		0x7f




// PMOVZX—Move Aligned Double Quadword

#define PP_VPMOVZX				VEX_PREFIX66
#define MMMMM_VPMOVZX			VEX_PREFIX0F38

// VPMOVZXBW xmm1, xmm2/m128
// VEX.128.66.0F38.WIG 6F /r
#define AVXOP_PMOVZXBW		0x30

// VPMOVZXBD xmm2/m128, xmm1
// VEX.128.66.0F38.WIG 7F /r
#define AVXOP_PMOVZXBD		0x31

// VPMOVZXBQ xmm2/m128, xmm1
// VEX.128.66.0F38.WIG 7F /r
#define AVXOP_PMOVZXBQ		0x32

// VPMOVZXWD xmm2/m128, xmm1
// VEX.128.66.0F38.WIG 7F /r
#define AVXOP_PMOVZXWD		0x33

// VPMOVZXWQ xmm2/m128, xmm1
// VEX.128.66.0F38.WIG 7F /r
#define AVXOP_PMOVZXWQ		0x34

// VPMOVZXDQ xmm2/m128, xmm1
// VEX.128.66.0F38.WIG 7F /r
#define AVXOP_PMOVZXDQ		0x35




// VMOVHLPS - combine high quadwords of xmm registers, 3rd operand on bottom
// VMOVHLPS xmm1, xmm2, xmm3
// VEX.NDS.128.0F.WIG 12 /r
#define AVXOP_MOVHLPS			0x12

// VMOVLHPS - combine low quadwords of xmm registers, 3rd operand on top
// VMOVLHPS xmm1, xmm2, xmm3
// VEX.NDS.128.0F.WIG 16 /r
#define AVXOP_MOVLHPS			0x16


// ** pabs instruction ** //

#define PP_VPABS				VEX_PREFIX66
#define MMMMM_VPABS				VEX_PREFIX0F38

// VPABSB - packed absolute value of 8-bit values
// VPABSB xmm1, xmm2/m128
// VEX.128.66.0F38.WIG 1C /r
#define AVXOP_PABSB				0x1c

// VPABSW - packed absolute value of 16-bit values
// VPABSW xmm1, xmm2/m128
// VEX.128.66.0F38.WIG 1D /r
#define AVXOP_PABSW				0x1d

// VPABSD - packed absolute value of 32-bit values
// VPABSD xmm1, xmm2/m128
// VEX.128.66.0F38.WIG 1E /r
#define AVXOP_PABSD				0x1e

// VPACKUSDW - pack 32-bit values into 16-bit values using unsigned saturation - source 1 on bottom
// VPACKUSDW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F38.WIG 2B /r
#define AVXOP_PACKUSDW			0x2b

// VPACKUSWB - pack 16-bit values into 8-bit values using unsigned saturation - source 1 on bottom
// VPACKUSWB xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 67 /r
#define AVXOP_PACKUSWB			0x67

// VPADDB - add packed byte integers
// VPADDB xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG FC /r
#define AVXOP_PADDB				0xfc

// VPADDW - add packed 16-bit values
// VPADDW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG FD /r
#define AVXOP_PADDW				0xfd

// VPADDD - add packed 32-bit values
// VPADDD xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG FE /r
#define AVXOP_PADDD				0xfe

// VPADDUSB - add packed unsigned bytes and saturate the results
// VPADDUSB xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG DC /r
#define AVXOP_PADDUSB			0xdc

// VPADDUSW - add packed unsigned 16-bit values and saturate the results
// VPADDUSW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG DD /r
#define AVXOP_PADDUSW			0xdd

// VPADDSB - add packed signed bytes with saturation
// VPADDSB xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG EC /r
#define AVXOP_PADDSB			0xec

// VPADDSW - add packed signed 16-values with saturation
// VPADDSW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG ED /r
#define AVXOP_PADDSW			0xed

// VPAND - logical and 128-bit values
// VPAND xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG DB /r
#define AVXOP_PAND				0xdb

// VPANDN - logical and not of 128-bit values
// VPANDN xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG DF /r
#define AVXOP_PANDN				0xdf

// VPCMPEQB - compare packed bytes for equality - sets destination to all 1s if equal, sets to all 0s otherwise
// VPCMPEQB xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 74 /r
#define AVXOP_PCMPEQB			0x74

// VPCMPEQW - compare packed 16-bit values for equality - sets destination to all 1s if equal, sets to all 0s otherwise
// VPCMPEQW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 75 /r
#define AVXOP_PCMPEQW			0x75

// VPCMPEQD - compare packed 32-bit values for equality - sets destination to all 1s if equal, sets to all 0s otherwise
// VPCMPEQD xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 76 /r
#define AVXOP_PCMPEQD			0x76

// VPCMPGTB - Compare packed signed byte integers for greater than
// VPCMPGTB xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 64 /r
#define AVXOP_PCMPGTB			0x64

// VPCMPGTW - Compare packed signed 16-bit integers for greater than
// VPCMPGTW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 65 /r
#define AVXOP_PCMPGTW			0x65

// VPCMPGTD - Compare packed signed 32-bit integers for greater than
// VPCMPGTD xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 66 /r
#define AVXOP_PCMPGTD			0x66

// VPMAXSD - get maximum of packed signed 32-bit values
// VPMAXSD xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F38.WIG 3D /r
#define AVXOP_PMAXSD			0x3d

// VPMAXSW - get maximum of packed signed 16-bit values
// VPMAXSW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG EE /r
#define AVXOP_PMAXSW			0xee

// VPMINSD - get minimum of packed signed 32-bit values
// VPMINSD xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F38.WIG 39 /r
#define AVXOP_PMINSD			0x39

// VPMINSW - get minimum of packed signed 16-bit values
// VPMINSW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG EA /r
#define AVXOP_PMINSW			0xea

// VPMOVSXDQ - sign extend lower 2 32-bit packed integers into 2 packed 64-bit integers
// VPMOVSXDQ xmm1, xmm2/m64
// VEX.128.66.0F38.WIG 25 /r
#define AVXOP_PMOVSXDQ			0x25

// VPMULUDQ - unsigned multiply of packed 32-bit integers
// VPMULUDQ xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG F4 /r
#define AVXOP_PMULUDQ			0xf4
#define PP_PMULUDQ				VEX_PREFIX66
#define MMMMM_PMULUDQ			VEX_PREFIX0F

// VPOR - logical or 128-bit values
// VPOR xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG EB /r
#define AVXOP_POR					0xeb

// VPSHUFB - shuffle packed bytes according to contents of an xmm register
// VPSHUFB xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F38.WIG 00 /r
#define AVXOP_PSHUFB				0x00

// VPSHUFD - shuffle packed 32-bit values
// VPSHUFD xmm1, xmm2/m128, imm8
// VEX.128.66.0F.WIG 70 /r ib
#define AVXOP_PSHUFD				0x70

// VPSLLDQ - shift left logical 128-bit value
// VPSLLDQ xmm1, xmm2, imm8
// VEX.NDD.128.66.0F.WIG 73 /7 ib
#define AVXOP_PSLLDQ				0x73
#define MODRM_PSLLDQ				0x7

// VPSHUFHW - shuffle packed high 16-bit values
// VPSHUFHW xmm1, xmm2/m128, imm8
// VEX.128.F3.0F.WIG 70 /r ib
#define AVXOP_PSHUFHW				0x70

// VPSHUFLW - shuffle packed low 16-bit values
// VPSHUFLW xmm1, xmm2/m128, imm8
// VEX.128.F2.0F.WIG 70 /r ib
#define AVXOP_PSHUFLW				0x70

// VPSLLW - shift left logical packed 16-bit values
// VPSLLW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG F1 /r
#define AVXOP_PSLLW					0xf1

// VPSLLW xmm1, xmm2, imm8
// VEX.NDD.128.66.0F.WIG 71 /6 ib
#define AVXOP_PSLLW_IMM				0x71
#define MODRM_PSLLW_IMM				0x6

// VPSLLD - shift left logical packed 32-bit values
// VPSLLD xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG F2 /r
#define AVXOP_PSLLD					0xf2

// VPSLLD xmm1, xmm2, imm8
// VEX.NDD.128.66.0F.WIG 72 /6 ib
#define AVXOP_PSLLD_IMM				0x72
#define MODRM_PSLLD_IMM				0x6

// VPSRAW - shift arithmetic right packed 16-bit value
// VPSRAW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG E1 /r
#define AVXOP_PSRAW					0xe1

// VPSRAW xmm1, xmm2, imm8
// VEX.NDD.128.66.0F.WIG 71 /4 ib
#define AVXOP_PSRAW_IMM				0x71
#define MODRM_PSRAW_IMM				0x4

// VPSRAD - shift arithmetic right packed 32-bit values
// VPSRAD xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG E2 /r
#define AVXOP_PSRAD					0xe2

// VPSRAD xmm1, xmm2, imm8
// VEX.NDD.128.66.0F.WIG 72 /4 ib
#define AVXOP_PSRAD_IMM				0x72
#define MODRM_PSRAD_IMM				0x4

// VPSRLDQ - shift right logical 128-bit value
// VPSRLDQ xmm1, xmm2, imm8
// VEX.NDD.128.66.0F.WIG 73 /3 ib
#define AVXOP_PSRLDQ_IMM			0x73
#define MODRM_PSRLDQ_IMM			0x3

// VPSRLW - shift right logical packed 16-bit values
// VPSRLW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG D1 /r
#define AVXOP_PSRLW					0xd1

// VPSRLW xmm1, xmm2, imm8
// VEX.NDD.128.66.0F.WIG 71 /2 ib
#define AVXOP_PSRLW_IMM				0x71
#define MODRM_PSRLW_IMM				0x2

// VPSRLD - shift right logical packed 32-bit values
// VPSRLD xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG D2 /r
#define AVXOP_PSRLD					0xd2

// VPSRLD xmm1, xmm2, imm8
// VEX.NDD.128.66.0F.WIG 72 /2 ib
#define AVXOP_PSRLD_IMM				0x72
#define MODRM_PSRLD_IMM				0x2

// VPSUBB - subtract packed 8-bit values
// VPSUBB xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG F8 /r
#define AVXOP_PSUBB					0xf8

// VPSUBW - subtract packed 16-bit values
// VPSUBW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG F9 /r
#define AVXOP_PSUBW					0xf9

// VPSUBD - subtract packed 32-bit values
// VPSUBD xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG FA /r
#define AVXOP_PSUBD					0xfa

// VPSUBSB - subtract packed 8-bit values with signed saturation
// VPSUBSB xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG E8 /r
#define AVXOP_PSUBSB				0xe8

// VPSUBSW - subtract packed 16-bit values with signed saturation
// VPSUBSW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG E9 /r
#define AVXOP_PSUBSW				0xe9

// VPSUBUSB - subtract unsigned packed 8-bit values with unsigned saturation
// VPSUBUSB xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG D8 /r
#define AVXOP_PSUBUSB				0xd8

// VPSUBUSW - subtract unsigned packed 8-bit values with unsigned saturation
// VPSUBUSW xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG D9 /r
#define AVXOP_PSUBUSW				0xd9

// VPUNPCKHBW - interleave high order bytes - first source on bottom
// VPUNPCKHBW xmm1,xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 68/r
#define AVXOP_PUNPCKHBW				0x68

// VPUNPCKHWD - interleave high order 16-bit values - first source on bottom
// VPUNPCKHWD xmm1,xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 69/r
#define AVXOP_PUNPCKHWD				0x69

// VPUNPCKHDQ - interleave high order 32-bit values - first source on bottom
// VPUNPCKHDQ xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 6A/r
#define AVXOP_PUNPCKHDQ				0x6a

// VPUNPCKHQDQ - interleave high order 64-bit values - first source on bottom
// VPUNPCKHQDQ xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 6D/r
#define AVXOP_PUNPCKHQDQ			0x6d

// VPUNPCKLBW - interleave low order bytes into 16-bit values - first source register goes on top
// VPUNPCKLBW xmm1,xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 60/r
#define AVXOP_PUNPCKLBW				0x60

// VPUNPCKLWD - interleave low order 16-bit values - first source on bottom
// VPUNPCKLWD xmm1,xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 61/r
#define AVXOP_PUNPCKLWD				0x61

// VPUNPCKLDQ - interleave low order 32-bit values - first source on bottom
// VPUNPCKLDQ xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 62/r
#define AVXOP_PUNPCKLDQ				0x62

// VPUNPCKLQDQ - interleave low order 64-bit values - first source on bottom
// VPUNPCKLQDQ xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG 6C/r
#define AVXOP_PUNPCKLQDQ			0x6c

// VPXOR - logical xor 128-bit values
// VPXOR xmm1, xmm2, xmm3/m128
// VEX.NDS.128.66.0F.WIG EF /r
#define AVXOP_PXOR				0xef


#endif


