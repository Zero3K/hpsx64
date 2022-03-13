


#include "Playstation2.h"
#include "x64Encoder.h"

#include "SignExtend.h"

#include "R5900Encoder.h"


R5900Encoder::R5900Encoder ( x64Encoder* enc )
{
	// set the encoder to use
	x = enc;
}


// ** Mips Emotion Engine integer instructions ** //

bool R5900Encoder::ADDIU ( long rt, long rs, long immediate )
{
	// register r0 is hardwired to zero, so just return if destination is r0
	if ( rt == 0 ) return true;

	// load source register
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );

	// add immediate
	x->AddRegImm64 ( RAX, SignExtend16To32 ( immediate ) );
	
	// sign extend from 32-bits to 64-bits
	x->Cdqe ();

	// store to destination register and return whether or not instruction was successfully encoded
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
}

bool R5900Encoder::ADDU ( long rd, long rs, long rt )
{
	if ( rd == 0 ) return true;

	// load source register
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	
	// add to other register
	x->AddRegMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	// sign extend from 32-bits to 64-bits
	x->Cdqe ();

	// store to destination register and return whether or not instruction was successfully encoded
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::AND ( long rd, long rs, long rt )
{
	if ( rd == 0 ) return true;

	if ( rs == rd )
	{
		x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
		return x->AndMemReg64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
	}
	
	if ( rt == rd )
	{
		x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
		return x->AndMemReg64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
	}


	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->AndRegMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::ANDI ( long rt, long rs, long immediate )
{
	if ( rt == 0 ) return true;

	if ( rs == rt ) return x->AndMemImm64 ( immediate, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->AndRegImm64 ( RAX, immediate );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
}

bool R5900Encoder::DADDIU ( long rt, long rs, long immediate )
{
	if ( rt == 0 ) return true;

	if ( rs == rt ) return x->AddMemImm64 ( SignExtend16To32 ( immediate ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->AddRegImm64 ( RAX, SignExtend16To32 ( immediate ) );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
}

bool R5900Encoder::DADDU ( long rd, long rs, long rt )
{
	if ( rd == 0 ) return true;

	if ( rs == rd )
	{
		x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
		return x->AndMemReg64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
	}
	
	if ( rt == rd )
	{
		x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
		return x->AndMemReg64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
	}

	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->AddRegMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}


bool R5900Encoder::DSLL ( long rd, long rt, long sa )
{
	if ( rd == 0 ) return true;

	if ( rt == rd )
	{
		return x->ShlMemImm64 ( R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ), sa );
	}

	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->ShlRegImm64 ( RAX, (char) sa );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::DSLL32 ( long rd, long rt, long sa )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->ShlRegImm64 ( RAX, (char) sa + 32 );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::DSLLV ( long rd, long rt, long rs )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->MovRegFromMem32 ( ECX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->ShlRegReg64 ( RAX );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::DSRA ( long rd, long rt, long sa )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->SarRegImm64 ( RAX, (char) sa );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::DSRA32 ( long rd, long rt, long sa )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->SarRegImm64 ( RAX, (char) sa + 32 );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::DSRAV ( long rd, long rt, long rs )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->MovRegFromMem32 ( ECX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->SarRegReg64 ( RAX );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::DSRL ( long rd, long rt, long sa )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->ShrRegImm64 ( RAX, (char) sa );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::DSRL32 ( long rd, long rt, long sa )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->ShrRegImm64 ( RAX, (char) sa + 32 );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::DSRLV ( long rd, long rt, long rs )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->MovRegFromMem32 ( ECX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->ShrRegReg64 ( RAX );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::DSUBU ( long rd, long rs, long rt )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->SubRegMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::LUI ( long rt, long immediate )
{
	// this one sign extends the 32-bit immediate too
	return x->MovMemImm64 ( ( immediate << 16 ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
}

bool R5900Encoder::MOVN ( long rd, long rs, long rt )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
	x->CmpMemImm64 ( 0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );	
	x->CmovNERegMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::MOVZ ( long rd, long rs, long rt )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
	x->CmpMemImm64 ( 0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );	
	x->CmovERegMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::NOR ( long rd, long rs, long rt )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->OrRegMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->NotReg64 ( RAX );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::OR ( long rd, long rs, long rt )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->OrRegMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::ORI ( long rt, long rs, long immediate )
{
	if ( rt == 0 ) return true;
	
	if ( rs == rt ) return x->OrMemImm64 ( immediate, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	
	if ( rs == 0 ) return x->MovMemImm64 ( immediate, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->OrRegImm64 ( RAX, immediate );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
}

bool R5900Encoder::PREF ( long hint, short offset, long base )
{
	// skip for now, will implement later
	return true;
}

bool R5900Encoder::SLL ( long rd, long rt, long sa )
{
	// can't just exit if sa is zero since register still gets sign extended
	if ( rd == 0 ) return true;

	x->MovRegFromMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->ShlRegImm32 ( RAX, (char) sa );
	x->Cdqe ();
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::SLLV ( long rd, long rt, long rs )
{
	x->MovRegFromMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->MovRegFromMem32 ( ECX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->ShlRegReg32 ( RAX );
	x->Cdqe ();
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::SLT ( long rd, long rs, long rt )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->CmpRegMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->Setl ( RAX );	// must do this to register or you don't get correct result
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::SLTI ( long rt, long rs, long immediate )
{
	x->CmpMemImm64 ( SignExtend16To32 ( immediate ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->Setl ( RAX );	// must do this to register or you don't get correct result
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
}

bool R5900Encoder::SLTIU ( long rt, long rs, long immediate )
{
	x->CmpMemImm64 ( SignExtend16To32 ( immediate ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->Setb ( RAX );	// must do this to register or you don't get correct result
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
}

bool R5900Encoder::SLTU ( long rd, long rs, long rt )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->CmpRegMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->Setb ( RAX );	// must do this to register or you don't get correct result
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::SRA ( long rd, long rt, long sa )
{
	// can't just exit if sa is zero since register still gets sign extended
	x->MovRegFromMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->SarRegImm32 ( RAX, (char) sa );
	x->Cdqe ();
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::SRAV ( long rd, long rt, long rs )
{
	x->MovRegFromMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->MovRegFromMem32 ( ECX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->SarRegReg32 ( RAX );
	x->Cdqe ();
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::SRL ( long rd, long rt, long sa )
{
	x->MovRegFromMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->ShrRegImm32 ( RAX, (char) sa );
	x->Cdqe ();
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::SRLV ( long rd, long rt, long rs )
{
	x->MovRegFromMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->MovRegFromMem32 ( ECX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->ShrRegReg32 ( RAX );
	x->Cdqe ();
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::SUBU ( long rd, long rs, long rt )
{
	if ( rd == 0 ) return true;

	x->MovRegFromMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->SubRegMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->Cdqe ();
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::XOR ( long rd, long rs, long rt )
{
	if ( rd == 0 ) return true;

	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->XorRegMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rd ] ) );
}

bool R5900Encoder::XORI ( long rt, long rs, long immediate )
{
	if ( rt == 0 ) return true;

	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );
	x->XorRegImm64 ( RAX, immediate );
	return x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
}

bool R5900Encoder::BEQ ( long rs, long rt, long offset )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );

	// check if we should take branch
	x->CmpRegMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	// set branch taken flag on branch
	x->Set_E ( R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.isBranchTaken ) );

	// set next address if branch is taken
	return x->MovMemImm32 ( PS2SystemState.Mips64.DecodePC + 4 + ( SignExtend16To32( offset ) << 2 ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.NextPC ) );
}

bool R5900Encoder::BEQL ( long rs, long rt, long offset )
{
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rs ] ) );

	// check if we should take branch
	x->CmpRegMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	// set branch taken flag on branch
	x->Set_E ( R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.isBranchTaken ) );

	// set next address if branch is taken
	return x->MovMemImm32 ( PS2SystemState.Mips64.DecodePC + 4 + ( SignExtend16To32( offset ) << 2 ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.NextPC ) );
}

bool R5900Encoder::LB ( long rt, long base, long offset )
{
	// load base register
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ base ] ) );

	// get upper 20 bits of address in base register
	x->MovRegReg32 ( RCX, RAX );
	x->ShrRegImm32 ( RCX, 12 );

	// look up upper 20 bits of address in base register in address table
	x->MovRegFromMem64 ( RDX, R8, RCX, SCALE_EIGHT, GetOffsetForPS2Data ( Mips64.TranslationAddressTable ) );
	
	// if zero, then don't do anything
	x->OrRegReg64 ( RDX, RDX );
	x->Jmp8_E ( 0, 0 );
	
	// look up the mask to use for this address - must use this
	x->MovRegFromMem64 ( RCX, R8, RCX, SCALE_FOUR, GetOffsetForPS2Data ( Mips64.TranslationMaskTable ) );
	
	// mask out upper bits of address so we just have offset into memory device
	x->AndRegReg32 ( RAX, RCX );
	
	// there is an address in RDX, so we can use it to load byte at offset! Must sign extend!
	// can't load into a,b,c,d with this instructon and REX prefix
	x->MovsxReg64Mem8 ( RAX, RDX, RAX, SCALE_NONE, SignExtend16To32 ( offset ) );

	// store result to destination register
	x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	x->SetJmpTarget8 ( 0 );
}

bool R5900Encoder::SB ( long rt, long base, long offset )
{
	// load base register
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ base ] ) );

	// get upper 20 bits of address in base register
	x->MovRegReg32 ( RCX, RAX );
	x->ShrRegImm32 ( RCX, 12 );

	// look up upper 20 bits of address in base register in address table
	x->MovRegFromMem64 ( RDX, R8, RCX, SCALE_EIGHT, GetOffsetForPS2Data ( Mips64.TranslationAddressTable ) );
	
	// if zero, then don't do anything
	x->OrRegReg64 ( RDX, RDX );
	x->Jmp8_E ( 0, 0 );
	
	// look up the mask to use for this address - must use this
	x->MovRegFromMem64 ( RCX, R8, RCX, SCALE_FOUR, GetOffsetForPS2Data ( Mips64.TranslationMaskTable ) );
	
	// mask out upper bits of address so we just have offset into memory device
	x->AndRegReg32 ( RAX, RCX );
	
	// load value to store from register
	x->MovRegFromMem64 ( RCX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	// there is an address in RDX, so we can use it to store byte at offset! Must sign extend!
	// can't load into a,b,c,d with this instructon and REX prefix
	x->MovRegToMem8 ( RCX, RDX, RAX, SCALE_NONE, SignExtend16To32 ( offset ) );


	x->SetJmpTarget8 ( 0 );
}

bool R5900Encoder::LBU ( long rt, long base, long offset )
{
	// load base register
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ base ] ) );

	// get upper 20 bits of address in base register
	x->MovRegReg32 ( RCX, RAX );
	x->ShrRegImm32 ( RCX, 12 );

	// look up upper 20 bits of address in base register in address table
	x->MovRegFromMem64 ( RDX, R8, RCX, SCALE_EIGHT, GetOffsetForPS2Data ( Mips64.TranslationAddressTable ) );
	
	// if zero, then don't do anything
	x->OrRegReg64 ( RDX, RDX );
	x->Jmp8_E ( 0, 0 );
	
	// look up the mask to use for this address - must use this
	x->MovRegFromMem64 ( RCX, R8, RCX, SCALE_FOUR, GetOffsetForPS2Data ( Mips64.TranslationMaskTable ) );
	
	// mask out upper bits of address so we just have offset into memory device
	x->AndRegReg32 ( RAX, RCX );
	
	// there is an address in RDX, so we can use it to load byte at offset! Must sign extend!
	// can't load into a,b,c,d with this instructon and REX prefix
	x->MovRegFromMem8 ( RAX, RDX, RAX, SCALE_NONE, SignExtend16To32 ( offset ) );

	// store result to destination register
	x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	x->SetJmpTarget8 ( 0 );
}

bool R5900Encoder::LH ( long rt, long base, long offset )
{
	// load base register
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ base ] ) );

	// get upper 20 bits of address in base register
	x->MovRegReg32 ( RCX, RAX );
	x->ShrRegImm32 ( RCX, 12 );

	// look up upper 20 bits of address in base register in address table
	x->MovRegFromMem64 ( RDX, R8, RCX, SCALE_EIGHT, GetOffsetForPS2Data ( Mips64.TranslationAddressTable ) );
	
	// if zero, then don't do anything
	x->OrRegReg64 ( RDX, RDX );
	x->Jmp8_E ( 0, 0 );
	
	// look up the mask to use for this address - must use this
	x->MovRegFromMem64 ( RCX, R8, RCX, SCALE_FOUR, GetOffsetForPS2Data ( Mips64.TranslationMaskTable ) );
	
	// mask out upper bits of address so we just have offset into memory device
	x->AndRegReg32 ( RAX, RCX );
	
	// there is an address in RDX, so we can use it to load byte at offset! Must sign extend!
	// can't load into a,b,c,d with this instructon and REX prefix
	x->MovsxReg64Mem16 ( RAX, RDX, RAX, SCALE_NONE, SignExtend16To32 ( offset ) );

	// store result to destination register
	x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	x->SetJmpTarget8 ( 0 );
}

bool R5900Encoder::SH ( long rt, long base, long offset )
{
	// load base register
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ base ] ) );

	// get upper 20 bits of address in base register
	x->MovRegReg32 ( RCX, RAX );
	x->ShrRegImm32 ( RCX, 12 );

	// look up upper 20 bits of address in base register in address table
	x->MovRegFromMem64 ( RDX, R8, RCX, SCALE_EIGHT, GetOffsetForPS2Data ( Mips64.TranslationAddressTable ) );
	
	// if zero, then don't do anything
	x->OrRegReg64 ( RDX, RDX );
	x->Jmp8_E ( 0, 0 );
	
	// look up the mask to use for this address - must use this
	x->MovRegFromMem64 ( RCX, R8, RCX, SCALE_FOUR, GetOffsetForPS2Data ( Mips64.TranslationMaskTable ) );
	
	// mask out upper bits of address so we just have offset into memory device
	x->AndRegReg32 ( RAX, RCX );
	
	// load value to store from register
	x->MovRegFromMem64 ( RCX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	// there is an address in RDX, so we can use it to store byte at offset! Must sign extend!
	// can't load into a,b,c,d with this instructon and REX prefix
	x->MovRegToMem16 ( RCX, RDX, RAX, SCALE_NONE, SignExtend16To32 ( offset ) );


	x->SetJmpTarget8 ( 0 );
}

bool R5900Encoder::LHU ( long rt, long base, long offset )
{
	// load base register
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ base ] ) );

	// get upper 20 bits of address in base register
	x->MovRegReg32 ( RCX, RAX );
	x->ShrRegImm32 ( RCX, 12 );

	// look up upper 20 bits of address in base register in address table
	x->MovRegFromMem64 ( RDX, R8, RCX, SCALE_EIGHT, GetOffsetForPS2Data ( Mips64.TranslationAddressTable ) );
	
	// if zero, then don't do anything
	x->OrRegReg64 ( RDX, RDX );
	x->Jmp8_E ( 0, 0 );
	
	// look up the mask to use for this address - must use this
	x->MovRegFromMem64 ( RCX, R8, RCX, SCALE_FOUR, GetOffsetForPS2Data ( Mips64.TranslationMaskTable ) );
	
	// mask out upper bits of address so we just have offset into memory device
	x->AndRegReg32 ( RAX, RCX );
	
	// there is an address in RDX, so we can use it to load byte at offset! Must sign extend!
	// can't load into a,b,c,d with this instructon and REX prefix
	x->MovRegFromMem16 ( RAX, RDX, RAX, SCALE_NONE, SignExtend16To32 ( offset ) );

	// store result to destination register
	x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	x->SetJmpTarget8 ( 0 );
}

bool R5900Encoder::LW ( long rt, long base, long offset )
{
	// load base register
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ base ] ) );

	// get upper 20 bits of address in base register
	x->MovRegReg32 ( RCX, RAX );
	x->ShrRegImm32 ( RCX, 12 );

	// look up upper 20 bits of address in base register in address table
	x->MovRegFromMem64 ( RDX, R8, RCX, SCALE_EIGHT, GetOffsetForPS2Data ( Mips64.TranslationAddressTable ) );
	
	// if zero, then don't do anything
	x->OrRegReg64 ( RDX, RDX );
	x->Jmp8_E ( 0, 0 );
	
	// look up the mask to use for this address - must use this
	x->MovRegFromMem64 ( RCX, R8, RCX, SCALE_FOUR, GetOffsetForPS2Data ( Mips64.TranslationMaskTable ) );
	
	// mask out upper bits of address so we just have offset into memory device
	x->AndRegReg32 ( RAX, RCX );
	
	// there is an address in RDX, so we can use it to load byte at offset! Must sign extend!
	// can't load into a,b,c,d with this instructon and REX prefix
	x->MovsxdReg64Mem32 ( RAX, RDX, RAX, SCALE_NONE, SignExtend16To32 ( offset ) );

	// store result to destination register
	x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	x->SetJmpTarget8 ( 0 );
}

bool R5900Encoder::SW ( long rt, long base, long offset )
{
	// load base register
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ base ] ) );

	// get upper 20 bits of address in base register
	x->MovRegReg32 ( RCX, RAX );
	x->ShrRegImm32 ( RCX, 12 );

	// look up upper 20 bits of address in base register in address table
	x->MovRegFromMem64 ( RDX, R8, RCX, SCALE_EIGHT, GetOffsetForPS2Data ( Mips64.TranslationAddressTable ) );
	
	// if zero, then don't do anything
	x->OrRegReg64 ( RDX, RDX );
	x->Jmp8_E ( 0, 0 );
	
	// look up the mask to use for this address - must use this
	x->MovRegFromMem64 ( RCX, R8, RCX, SCALE_FOUR, GetOffsetForPS2Data ( Mips64.TranslationMaskTable ) );
	
	// mask out upper bits of address so we just have offset into memory device
	x->AndRegReg32 ( RAX, RCX );
	
	// load value to store from register
	x->MovRegFromMem64 ( RCX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	// there is an address in RDX, so we can use it to store byte at offset! Must sign extend!
	// can't load into a,b,c,d with this instructon and REX prefix
	x->MovRegToMem32 ( RCX, RDX, RAX, SCALE_NONE, SignExtend16To32 ( offset ) );


	x->SetJmpTarget8 ( 0 );
}

bool R5900Encoder::LWU ( long rt, long base, long offset )
{
	// load base register
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ base ] ) );

	// get upper 20 bits of address in base register
	x->MovRegReg32 ( RCX, RAX );
	x->ShrRegImm32 ( RCX, 12 );

	// look up upper 20 bits of address in base register in address table
	x->MovRegFromMem64 ( RDX, R8, RCX, SCALE_EIGHT, GetOffsetForPS2Data ( Mips64.TranslationAddressTable ) );
	
	// if zero, then don't do anything
	x->OrRegReg64 ( RDX, RDX );
	x->Jmp8_E ( 0, 0 );
	
	// look up the mask to use for this address - must use this
	x->MovRegFromMem64 ( RCX, R8, RCX, SCALE_FOUR, GetOffsetForPS2Data ( Mips64.TranslationMaskTable ) );
	
	// mask out upper bits of address so we just have offset into memory device
	x->AndRegReg32 ( RAX, RCX );
	
	// there is an address in RDX, so we can use it to load byte at offset! Must sign extend!
	// can't load into a,b,c,d with this instructon and REX prefix
	x->MovRegFromMem32 ( RAX, RDX, RAX, SCALE_NONE, SignExtend16To32 ( offset ) );

	// store result to destination register
	x->MovRegToMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	x->SetJmpTarget8 ( 0 );
}

bool R5900Encoder::LWL ( long rt, long base, long offset )
{
	// load base register
	x->MovRegFromMem64 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ base ] ) );

	// get upper 20 bits of address in base register
	x->LeaRegRegImm32 ( RDX, RAX, SignExtend16To32 ( offset ) );
	x->ShrRegImm32 ( RDX, 12 );

	// look up upper 20 bits of address in base register in address table
	x->MovRegFromMem64 ( RCX, R8, RDX, SCALE_EIGHT, GetOffsetForPS2Data ( Mips64.TranslationAddressTable ) );

	// if zero, then don't do anything
	x->OrRegReg64 ( RCX, RCX );
	x->Jmp8_E ( 0, 0 );
	
	// look up the mask to use for this address - must use this
	x->MovRegFromMem64 ( RDX, R8, RDX, SCALE_FOUR, GetOffsetForPS2Data ( Mips64.TranslationMaskTable ) );
	
	// mask out upper bits of address so we just have offset into memory device
	x->AndRegReg32 ( RAX, RDX );
	
	// get device base address + base address offset + offset
	x->LeaRegRegImm32 ( RCX, RCX, RAX, SignExtend16To32 ( offset ) );
	
	// there is an address in RDX, so we can use it to load byte at offset! Must sign extend!
	// can't load into a,b,c,d with this instructon and REX prefix
	x->MovsxdReg64Mem32 ( RAX, RCX, NO_INDEX, SCALE_NONE, SignExtend16To32 ( offset ) );
	
	// get lower 2 bits of base+base offset+offset
	x->ShlRegImm32 ( RCX, 30 );

	// get arithmetic shift mask to the right by that times 8 (or shift left 3)
	x->ShrRegImm32 ( RCX, 27 );
	x->MovRegImm32 ( RDX, 0xff000000 );
	x->ShrRegReg32 ( RDX );
	
	// mask source and dest, combine and store to register
	x->AndRegReg32 ( RAX, RDX );
	x->NotReg32 ( RDX );
	x->AndMemReg32 ( RDX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Mips64.GPR [ rt ] ) );

	x->SetJmpTarget8 ( 0 );
}

bool R5900Encoder::SWL ( long rt, long base, long offset )
{
}

bool R5900Encoder::LWR ( long rt, long base, long offset )
{
}

bool R5900Encoder::SWR ( long rt, long base, long offset )
{
}













/*





// ps2 vector integer instructions

bool R5900Encoder::PABSH ( long rd, long rt )
{
	Format15V ( rd, rt );
	return x->pabsw ( rd, rt );
}

bool R5900Encoder::PABSW ( long rd, long rt )
{
	Format15V ( rd, rt );
	return x->pabsd ( rd, rt );
}

bool R5900Encoder::PADDB ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->paddb ( rd, rs, rt );
}

bool R5900Encoder::PADDH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->paddw ( rd, rs, rt );
}

bool R5900Encoder::PADDW ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->paddd ( rd, rs, rt );
}

bool R5900Encoder::PADDSB ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->paddsb ( rd, rs, rt );
}

bool R5900Encoder::PADDSH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->paddsw ( rd, rs, rt );
}

bool R5900Encoder::PADDSW ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	x->paddd ( rd, rs, rt );
	x->pandn ( XMM0, rd, rs );
	x->pandn ( XMM0, XMM0, rt );
	x->psrad ( XMM0, XMM0, 31 );
	x->psrld ( XMM1, XMM0, 1 );
	x->blendvps ( rd, rd, XMM1, XMM0 );
	x->pand ( XMM0, rs, rt );
	x->pandn ( XMM0, XMM0, rd );
	x->pandn ( XMM0, XMM0, XMM1 );
	return x->blendvps ( rd, rd, XMM0, XMM0 );
}

bool R5900Encoder::PADDUB ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->paddusb ( rd, rs, rt );
}

bool R5900Encoder::PADDUH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->paddusw ( rd, rs, rt );
}

bool R5900Encoder::PADDUW ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	x->por ( XMM0, rs, rt );
	x->paddd ( rd, rs, rt );
	x->pandn ( XMM0, XMM0, rd );
	x->psrad ( XMM0, XMM0, 31 );
	return x->blendvps ( rd, rd, XMM0, XMM0 );
}

bool R5900Encoder::PADSBH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	x->paddw ( XMM0, rs, rt );
	x->psubw ( rd, rs, rt );
	return x->pblendw ( rd, rd, XMM0, 0xf0 );
}

bool R5900Encoder::PAND ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->pand ( rd, rs, rt );
}

bool R5900Encoder::PCEQB ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->pcmpeqb ( rd, rs, rt );
}

bool R5900Encoder::PCEQH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->pcmpeqw ( rd, rs, rt );
}

bool R5900Encoder::PCEQW ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->pcmpeqd ( rd, rs, rt );
}

bool R5900Encoder::PCGTB ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->pcmpgtb ( rd, rs, rt );
}

bool R5900Encoder::PCGTH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->pcmpgtw ( rd, rs, rt );
}

bool R5900Encoder::PCGTW ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->pcmpgtd ( rd, rs, rt );
}

bool R5900Encoder::PCPYH ( long rd, long rt )
{
	Format15V ( rd, rt );
	x->pshufhw ( rd, rt, 0 );
	return x->pshuflw ( rd, rd, 0 );
}

bool R5900Encoder::PCPYLD ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->punpcklqdq ( rd, rt, rs );
}

bool R5900Encoder::PCPYUD ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->punpckhqdq ( rd, rs, rt );
}

bool R5900Encoder::PEXCH ( long rd, long rt )
{
	Format15V ( rd, rt );
	x->pshufhw ( rd, rt, ( 3 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | 0 );
	return x->pshuflw ( rd, rd, ( 3 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | 0 );
}

bool R5900Encoder::PEXCW ( long rd, long rt )
{
	Format15V ( rd, rt );
	return x->pshufd ( rd, rt, ( 3 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | 0 );
}

bool R5900Encoder::PEXEH ( long rd, long rt )
{
	Format15V ( rd, rt );
	x->pshufhw ( rd, rt, ( 3 << 6 ) | ( 0 << 4 ) | ( 1 << 2 ) | 2 );
	return x->pshuflw ( rd, rd, ( 3 << 6 ) | ( 0 << 4 ) | ( 1 << 2 ) | 2 );
}

bool R5900Encoder::PEXEW ( long rd, long rt )
{
	Format15V ( rd, rt );
	return x->pshufd ( rd, rt, ( 3 << 6 ) | ( 0 << 4 ) | ( 1 << 2 ) | 2 );
}

bool R5900Encoder::PEXT5 ( long rd, long rt )
{
	Format15V ( rd, rt );
	x->pslld ( rd, rt, 6 );
	x->pblendw ( rd, rd, rt, 0x55 );
	x->psllw ( rd, rd, 11 );
	x->psrlw ( rd, rd, 8 );
	x->psrlw ( XMM0, rt, 15 );
	x->pslld ( XMM0, XMM0, 25 );
	x->pblendw ( XMM0, XMM0, rt, 0x55 );
	x->psrlw ( XMM0, XMM0, 5 );
	x->psllw ( XMM0, XMM0, 11 );
	return x->por ( rd, rd, XMM0 );
}

bool R5900Encoder::PEXTLB ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->punpcklbw ( rd, rt, rs );
}

bool R5900Encoder::PEXTLH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->punpcklwd ( rd, rt, rs );
}

bool R5900Encoder::PEXTLW ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->punpckldq ( rd, rt, rs );
}

bool R5900Encoder::PEXTUB ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->punpckhbw ( rd, rt, rs );
}

bool R5900Encoder::PEXTUH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->punpckhwd ( rd, rt, rs );
}

bool R5900Encoder::PEXTUW ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->punpckhdq ( rd, rt, rs );
}

bool R5900Encoder::PINTEH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	x->pslld ( rd, rs, 16 );
	return x->pblendw ( rd, rd, rt, 0x55 );
}

bool R5900Encoder::PINTH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	x->psrldq ( rd, rs, 8 );
	return x->punpcklwd ( rd, rt, rs );
}

bool R5900Encoder::PMAXH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->pmaxsw ( rd, rs, rt );
}

bool R5900Encoder::PMAXW ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->pmaxsd ( rd, rs, rt );
}

bool R5900Encoder::PMINH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->pminsw ( rd, rs, rt );
}

bool R5900Encoder::PMINW ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->pminsd ( rd, rs, rt );
}

bool R5900Encoder::PNOR ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	x->por ( rd, rs, rt );
	x->pcmpeqd ( XMM0, XMM0, XMM0 );
	return x->pxor ( rd, rd, XMM0 );
}

bool R5900Encoder::POR ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->por ( rd, rs, rt );
}

bool R5900Encoder::PPAC5 ( long rd, long rt )
{
	Format15V ( rd, rt );
	x->psrld ( rd, rt, 15 );
	x->pslld ( XMM0, rt, 8 );
	x->pblendw ( rd, rd, XMM0, 0x55 );
	x->pslld ( rd, rd, 5 );
	x->pblendw ( rd, rd, rt, 0x55 );
	x->pslld ( rd, rd, 5 );
	x->psrld ( XMM0, rt, 8 );
	x->pblendw ( rd, rd, XMM0, 0x55 );
	return x->psrld ( rd, rd, 16 );
}

bool R5900Encoder::PPACB ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	x->psllw ( XMM0, rs, 8 );
	x->psrlw ( XMM0, XMM0, 8 );
	x->psllw ( rd, rt, 8 );
	x->psrlw ( rd, rd, 8 );
	return x->packuswb ( rd, rd, XMM0 );
}

bool R5900Encoder::PPACH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	x->pslld ( XMM0, rs, 16 );
	x->psrld ( XMM0, XMM0, 16 );
	x->pslld ( rd, rt, 16 );
	x->psrld ( rd, rd, 16 );
	return x->packusdw ( rd, rd, XMM0 );
}

bool R5900Encoder::PPACW ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	x->pshufd ( XMM0, rs, ( 2 << 6 ) | ( 0 << 4 ) );
	x->pshufd ( rd, rt, ( 2 << 2 ) | ( 0 << 0 ) );
	return x->pblendw ( rd, rd, XMM0, 0xf0 );
}

bool R5900Encoder::PREVH ( long rd, long rt )
{
	Format15V ( rd, rt );
	x->pshufhw ( rd, rt, ( 0 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | ( 3 << 0 ) );
	return x->pshuflw ( rd, rd, ( 0 << 6 ) | ( 1 << 4 ) | ( 2 << 2 ) | ( 3 << 0 ) );
}

bool R5900Encoder::PROT3W ( long rd, long rt )
{
	Format15V ( rd, rt );
	return x->pshufd ( rd, rt, ( 3 << 6 ) | ( 0 << 4 ) | ( 2 << 2 ) | ( 1 << 0 ) );
}

bool R5900Encoder::PSLLH ( long rd, long rt, long sa )
{
	Format15V ( rd, rt );
	return x->psllw_imm ( rd, rt, sa );
}

bool R5900Encoder::PSLLVW ( long rd, long rt, long rs )
{
	Format1V ( rd, rt, rs );
	x->pslld ( rd, rt, rs );
	x->pshufd ( rd, rd, ( 0x0 << 6 ) | ( 0x0 << 4 ) | ( 0x2 << 2 ) | ( 0x0 ) );
	return x->pmovsxdq ( rd, rd );
}

bool R5900Encoder::PSLLW ( long rd, long rt, long sa )
{
	Format15V ( rd, rt );
	return x->pslld_imm ( rd, rt, sa );
}

bool R5900Encoder::PSRAH ( long rd, long rt, long sa )
{
	Format15V ( rd, rt );
	return x->psraw_imm ( rd, rt, sa );
}

bool R5900Encoder::PSRAVW ( long rd, long rt, long rs )
{
	Format1V ( rd, rt, rs );
	x->psrad ( rd, rt, rs );
	x->pshufd ( rd, rd, ( 0x0 << 6 ) | ( 0x0 << 4 ) | ( 0x2 << 2 ) | ( 0x0 ) );
	return x->pmovsxdq ( rd, rd );
}


bool R5900Encoder::PSRAW ( long rd, long rt, long sa )
{
	Format15V ( rd, rt );
	return x->psrad_imm ( rd, rt, sa );
}

bool R5900Encoder::PSRLH ( long rd, long rt, long sa )
{
	Format15V ( rd, rt );
	return x->psrlw_imm ( rd, rt, sa );
}

bool R5900Encoder::PSRLVW ( long rd, long rt, long rs )
{
	Format1V ( rd, rt, rs );
	x->psrld ( rd, rt, rs );
	x->pshufd ( rd, rd, ( 0x0 << 6 ) | ( 0x0 << 4 ) | ( 0x2 << 2 ) | ( 0x0 ) );
	return x->pmovsxdq ( rd, rd );
}

bool R5900Encoder::PSRLW ( long rd, long rt, long sa )
{
	Format15V ( rd, rt );
	return x->psrld_imm ( rd, rt, sa );
}

bool R5900Encoder::PSUBB ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->psubb ( rd, rs, rt );
}

bool R5900Encoder::PSUBH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->psubw ( rd, rs, rt );
}

bool R5900Encoder::PSUBSB ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->psubsb ( rd, rs, rt );
}

bool R5900Encoder::PSUBSH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->psubsw ( rd, rs, rt );
}

bool R5900Encoder::PSUBSW ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	x->psubd ( rd, rs, rt );
	x->pandn ( XMM0, rs, rt );
	x->pand ( XMM0, XMM0, rd );
	x->psrad ( XMM0, XMM0, 31 );
	x->psrld ( XMM1, XMM0, 1 );
	x->blendvps ( rd, rd, XMM1, XMM0 );
	x->pandn ( XMM0, rt, rs );
	x->pandn ( XMM0, XMM0, rd );
	x->pandn ( XMM0, XMM0, XMM1 );
	return x->blendvps ( rd, rd, XMM0, XMM0 );
}

bool R5900Encoder::PSUBUB ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->psubusb ( rd, rs, rt );
}

bool R5900Encoder::PSUBUH ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->psubusw ( rd, rs, rt );
}

bool R5900Encoder::PSUBUW ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	x->pcmpeqd ( XMM0, rs, rs );
	x->pslld ( XMM0, XMM0, 31 );
	x->paddd ( rd, rt, XMM0 );
	x->paddd ( XMM0, XMM0, rs );
	x->pcmpgtd ( XMM0, rd, XMM0 );
	x->psubd ( rd, rs, rt );
	return x->pandn ( rd, rd, XMM0 );
}

bool R5900Encoder::PSUBW ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->psubd ( rd, rs, rt );
}

bool R5900Encoder::PXOR ( long rd, long rs, long rt )
{
	Format1V ( rd, rs, rt );
	return x->pxor ( rd, rs, rt );
}


// ** Mips Emotion Engine integer branch instructions ** //


bool R5900Encoder::BGEZ ( long rs, long offset )
{
	Format4 ( rs, offset );

	x->MovRegImm32 ( RAX, InstructionAddress + 8 );
	x->MovRegImm32 ( RCX, InstructionAddress + 4 + offset );

	x->CmpRegReg64 ( rs, 0 );

	// put next address into RAX
	x->CmovGERegReg32 ( RAX, RCX );

	// recompiler will terminate code block after encoding branch delay slot instruction so just save return address
	return x->PushReg32 ( RAX );
}

bool R5900Encoder::BGEZAL ( long rs, long offset )
{
	long r31;

	Format4 ( rs, offset );

	// need to cache R31 register for this instruction and say that it will be modified by instruction
	r31 = r->AllocGPRToIntReg ( R31, false );
	r->SetTargetIntRegAsModified ( r31 );

	x->MovRegImm32 ( RAX, InstructionAddress + 8 );
	x->MovRegImm32 ( RCX, InstructionAddress + 4 + offset );

	x->CmpRegImm64 ( rs, 0 );

	// IT ALWAYS STORES ADDRESS+8 TO R31 REGARDLESS - AND IT IS ZERO EXTENDED - put after compare
	x->MovRegReg32 ( r31, RAX );

	// put next address into RAX
	x->CmovGERegReg32 ( RAX, RCX );

	return x->PushReg32 ( RAX );
}

bool R5900Encoder::BGEZALL ( long rs, long offset )
{
	long r31;

	Format4 ( rs, offset );

	// need to cache R31 register for this instruction and say that it will be modified by instruction
	r31 = r->AllocGPRToIntReg ( R31, false );
	r->SetTargetIntRegAsModified ( r31 );

	x->MovRegImm32 ( RAX, InstructionAddress + 8 );

	x->CmpRegImm64 ( rs, 0 );

	// IT ALWAYS STORES ADDRESS+8 TO R31 REGARDLESS - AND IT IS ZERO EXTENDED - put after compare
	x->MovRegReg32 ( r31, RAX );

	// don't execute delay slot if not taking branch
	x->Jmp_L_End ( 0 );

	// recompiler will terminate code block after encoding branch delay slot instruction so just save next address
	return x->PushImm32 ( InstructionAddress + 4 + offset );
}

bool R5900Encoder::BGEZL ( long rs, long offset )
{
	Format4 ( rs, offset );

	x->MovRegImm32 ( RAX, InstructionAddress + 8 );

	x->CmpRegReg64 ( rs, 0 );

	// don't run delay slot if not branching
	x->Jmp_L_End ( 0 );

	// recompiler will terminate code block after encoding branch delay slot instruction so just save next address
	return x->PushImm32 ( InstructionAddress + 4 + offset );
}

bool R5900Encoder::BGTZ ( long rs, long offset )
{
	Format4 ( rs, offset );

	x->MovRegImm32 ( RAX, InstructionAddress + 8 );
	x->MovRegImm32 ( RCX, InstructionAddress + 4 + offset );

	x->CmpRegReg64 ( rs, 0 );

	// put next address into RAX
	x->CmovGRegReg32 ( RAX, RCX );

	// recompiler will terminate code block after encoding branch delay slot instruction so just save return address
	return x->PushReg32 ( RAX );
}

bool R5900Encoder::BGTZL ( long rs, long offset )
{
	Format4 ( rs, offset );

	x->MovRegImm32 ( RAX, InstructionAddress + 8 );

	x->CmpRegReg64 ( rs, 0 );

	// don't run delay slot if not branching
	x->Jmp_LE_End ( 0 );

	// recompiler will terminate code block after encoding branch delay slot instruction so just save next address
	return x->PushImm32 ( InstructionAddress + 4 + offset );
}

bool R5900Encoder::BLEZ ( long rs, long offset )
{
	Format4 ( rs, offset );

	x->MovRegImm32 ( RAX, InstructionAddress + 8 );
	x->MovRegImm32 ( RCX, InstructionAddress + 4 + offset );

	x->CmpRegReg64 ( rs, 0 );

	// put next address into RAX
	x->CmovLERegReg32 ( RAX, RCX );

	// recompiler will terminate code block after encoding branch delay slot instruction so just save return address
	return x->PushReg32 ( RAX );
}

bool R5900Encoder::BLEZL ( long rs, long offset )
{
	Format4 ( rs, offset );

	x->MovRegImm32 ( RAX, InstructionAddress + 8 );

	x->CmpRegReg64 ( rs, 0 );

	// don't run delay slot if not branching
	x->Jmp_G_End ( 0 );

	// recompiler will terminate code block after encoding branch delay slot instruction so just save next address
	return x->PushImm32 ( InstructionAddress + 4 + offset );
}

bool R5900Encoder::BLTZ ( long rs, long offset )
{
	Format4 ( rs, offset );

	x->MovRegImm32 ( RAX, InstructionAddress + 8 );
	x->MovRegImm32 ( RCX, InstructionAddress + 4 + offset );

	x->CmpRegReg64 ( rs, 0 );

	// put next address into RAX
	x->CmovLRegReg32 ( RAX, RCX );

	// recompiler will terminate code block after encoding branch delay slot instruction so just save return address
	return x->PushReg32 ( RAX );
}

bool R5900Encoder::BLTZAL ( long rs, long offset )
{
	long r31;

	Format4 ( rs, offset );

	// need to cache R31 register for this instruction and say that it will be modified by instruction
	r31 = r->AllocGPRToIntReg ( R31, false );
	r->SetTargetIntRegAsModified ( r31 );

	x->MovRegImm32 ( RAX, InstructionAddress + 8 );
	x->MovRegImm32 ( RCX, InstructionAddress + 4 + offset );

	x->CmpRegImm64 ( rs, 0 );

	// IT ALWAYS STORES ADDRESS+8 TO R31 REGARDLESS - AND IT IS ZERO EXTENDED - put after compare
	x->MovRegReg32 ( r31, RAX );

	// put next address into RAX
	x->CmovLRegReg32 ( RAX, RCX );

	return x->PushReg32 ( RAX );
}

bool R5900Encoder::BLTZALL ( long rs, long offset )
{
	long r31;

	Format4 ( rs, offset );

	// need to cache R31 register for this instruction and say that it will be modified by instruction
	r31 = r->AllocGPRToIntReg ( R31, false );
	r->SetTargetIntRegAsModified ( r31 );

	x->MovRegImm32 ( RAX, InstructionAddress + 8 );

	x->CmpRegImm64 ( rs, 0 );

	// IT ALWAYS STORES ADDRESS+8 TO R31 REGARDLESS - AND IT IS ZERO EXTENDED - put after compare
	x->MovRegReg32 ( r31, RAX );

	// don't execute delay slot if not taking branch
	x->Jmp_GE_End ( 0 );

	// recompiler will terminate code block after encoding branch delay slot instruction so just save next address
	return x->PushImm32 ( InstructionAddress + 4 + offset );
}

bool R5900Encoder::BLTZL ( long rs, long offset )
{
	Format4 ( rs, offset );

	x->MovRegImm32 ( RAX, InstructionAddress + 8 );

	x->CmpRegReg64 ( rs, 0 );

	// don't run delay slot if not branching
	x->Jmp_GE_End ( 0 );

	// recompiler will terminate code block after encoding branch delay slot instruction so just save next address
	return x->PushImm32 ( InstructionAddress + 4 + offset );
}

bool R5900Encoder::BNE ( long rs, long rt, long offset )
{
	Format3 ( rs, rt, offset );
	
	x->MovRegImm32 ( RAX, InstructionAddress + 8 );
	x->MovRegImm32 ( RCX, InstructionAddress + 4 + offset );

	x->CmpRegReg64 ( rs, rt );

	// put next address into RAX
	x->CmovNERegReg32 ( RAX, RCX );

	// recompiler will terminate code block after encoding branch delay slot instruction so just save return address
	return x->PushReg32 ( RAX );
}

bool R5900Encoder::BNEL ( long rs, long rt, long offset )
{
	Format3 ( rs, rt, offset );

	x->MovRegImm32 ( RAX, InstructionAddress + 8 );

	x->CmpRegReg64 ( rs, rt );

	// don't execute delay slot if not taking branch - just terminate code block
	x->Jmp_E_End ( 0 );

	// recompiler will terminate code block after encoding branch delay slot instruction so just save next address
	return x->PushImm32 ( InstructionAddress + 4 + offset );
}

bool R5900Encoder::J ( long address )
{
	// still need to execute branch delay slot, so save next address
	return x->PushImm32 ( ( InstructionAddress & 0xf0000000 ) | ( address << 2 ) );
}

bool R5900Encoder::JAL ( long address )
{
	long r31;

	// need to cache R31 register for this instruction and say that it will be modified by instruction
	r31 = r->AllocGPRToIntReg ( R31, false );
	r->SetTargetIntRegAsModified ( r31 );

	// link
	x->MovRegImm32 ( r31, InstructionAddress + 8 );

	// still need to execute branch delay slot, so save next address
	return x->PushImm32 ( ( InstructionAddress & 0xf0000000 ) | ( address << 2 ) );
}

bool R5900Encoder::JALR ( long rd, long rs )
{
	rs = r->AllocGPRToIntReg ( rs, true );
	rd = r->AllocGPRToIntReg ( rd, false );
	r->SetTargetIntRegAsModified ( rd );
	
	// link
	x->MovRegImm32 ( rd, InstructionAddress + 8 );
	
	// jump - still need to execute branch delay slot, so we'll just save the next address
	return x->PushReg32 ( rs );
}

bool R5900Encoder::JR ( long rs )
{
	rs = r->AllocGPRToIntReg ( rs, true );

	// jump - still need to execute branch delay slot, so we'll just save the next address
	return x->PushReg32 ( rs );
}

bool R5900Encoder::TEQ ( long rs, long rt )
{
	// check if trap will be taken
	
	// if not, then branch out

	// if so, switch to kernel mode (1->Status.EXL)

	// set EPC/Cause.BD

	// Cause.BD - set if trap is in branch delay slot, clear otherwise
	
	// EPC Register address of trap instruction unless it occurs in branch delay slot, when it is address of preceding branch
	
	// Cause.ExcCode = 12 or 13 ?? - need to check this


	// if in branch delay slot, then pop previous next address and push the new one
	// next return address if BEV=0 is 0x80000180, if BEV=1 then it is 0xbfc00380
	
	// branch target
}

bool R5900Encoder::TEQI ( long rs, long immediate )
{
}

bool R5900Encoder::TGE ( long rs, long rt )
{
}

bool R5900Encoder::TGEI ( long rs, long immediate )
{
}

bool R5900Encoder::TGEIU ( long rs, long immediate )
{
}

bool R5900Encoder::TGEU ( long rs, long rt )
{
}

bool R5900Encoder::TLT ( long rs, long rt )
{
}

bool R5900Encoder::TLTI ( long rs, long immediate )
{
}

bool R5900Encoder::TLTIU ( long rs, long immediate )
{
}

bool R5900Encoder::TLTU ( long rs, long rt )
{
}

bool R5900Encoder::TNE ( long rs, long rt )
{
}

bool R5900Encoder::TNEI ( long rs, long immediate )
{
}





// cop1 instructions

bool R5900Encoder::ABSS ( long fd, long fs )
{
	Format20 ( fd, fs );
	
	// no need to check that floating point input is valid since this is just a logical AND
	
	// perform op
	x->andnps ( fd, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstSignOnlyMask ) );

	// clear O (bit 15) and U (bit 14) Flags in FCR31
	return x->AndMemImm32 ( ~( ( 1 << 14 ) + ( 1 << 15 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );
}

bool R5900Encoder::ADDS ( long fd, long fs, long ft )
{
	Format21 ( fd, fs, ft );
	
	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// convert
	x->cvtps2pd ( XMM0, XMM0 );

	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );

	// split
	x->vperm2f128 ( XMM1, XMM0, XMM0, 1 );

	// perform op
	x->addpd ( XMM0, XMM0, XMM1 );
	
	// check for underflow

		// get absolute value of result - put absolute value of result in XMM1
		x->andnpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );
		
		// check if lower than minimum positive ps2 value
		x->cmpltpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Min ) );
	
	// extract and mask for underflow status flag (bit 14) and underflow sticky flag (bit 3) in FCR31
	x->pextrq ( RAX, XMM2, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 14 ) + ( 1 << 3 ) );

	// check for overflow
	x->cmpgtpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	
	// extract and mask for overflow status flag (bit 15) and overflow sticky flag (bit 4) in FCR31
	x->pextrq ( RCX, XMM2, 0 );
	x->AndRegImm32 ( RCX, ( 1 << 15 ) + ( 1 << 4 ) );

	// logical OR flags
	x->OrRegReg32 ( RAX, RCX );

	// clear O and U status flags in FCR31
	x->AndMemImm32 ( ~( ( 1 << 14 ) + ( 1 << 15 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// logical OR with FCR31
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// convert and store
	return x->cvtpd2ps ( fd, XMM0 );
}

bool R5900Encoder::ADDAS ( long fs, long ft )
{
	Format22 ( fs, ft );
	
	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// convert
	x->cvtps2pd ( XMM0, XMM0 );

	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );

	// split
	x->vperm2f128 ( XMM1, XMM0, XMM0, 1 );

	// perform op
	x->addpd ( XMM0, XMM0, XMM1 );
	
	// check for underflow

		// get absolute value of result - put absolute value of result in XMM1
		x->andnpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );
		
		// check if lower than minimum positive ps2 value
		x->cmpltpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Min ) );
	
	// extract and mask for underflow status flag (bit 14) and underflow sticky flag (bit 3) in FCR31
	x->pextrq ( RAX, XMM2, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 14 ) + ( 1 << 3 ) );

	// check for overflow
	x->cmpgtpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	
	// extract and mask for overflow status flag (bit 15) and overflow sticky flag (bit 4) in FCR31
	x->pextrq ( RCX, XMM2, 0 );
	x->AndRegImm32 ( RCX, ( 1 << 15 ) + ( 1 << 4 ) );

	// logical OR flags
	x->OrRegReg32 ( RAX, RCX );

	// clear O and U status flags in FCR31
	x->AndMemImm32 ( ~( ( 1 << 14 ) + ( 1 << 15 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// logical OR with FCR31
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// convert
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// store to accumulator
	return x->movss_to_mem128 ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( ACC ) );
}

bool R5900Encoder::CEQS ( long fs, long ft )
{
	Format22 ( fs, ft );
	
	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// clamp
	x->cmpunordps ( XMM1, XMM0, XMM0 );
	x->andps ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstMantissaOnlyMask ) );
	x->andnps ( XMM0, XMM0, XMM1 );

	// split
	x->shufps ( XMM1, XMM0, XMM0, ( 2 ) + ( 2 << 2 ) + ( 2 << 4 ) + ( 2 << 6 ) );

	// perform op
	x->cmpeqpd ( XMM2, XMM1, XMM0 );
	
	// extract
	x->extractps ( RAX, XMM2, 0 );
	
	// mask
	x->AndRegImm32 ( RAX, ( 1 << 23 ) );
	
	// clear C status flag (bit 23) in FCR31
	x->AndMemImm32 ( ~( ( 1 << 23 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// logical OR with FCR31
	return x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );
}

bool R5900Encoder::CFS ( long fs, long ft )
{
	// clear C status flag (bit 23) in FCR31
	return x->AndMemImm32 ( ~( ( 1 << 23 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );
}

bool R5900Encoder::CLES ( long fs, long ft )
{
	Format22 ( fs, ft );
	
	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// clamp
	x->cmpunordps ( XMM1, XMM0, XMM0 );
	x->andps ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstMantissaOnlyMask ) );
	x->andnps ( XMM0, XMM0, XMM1 );

	// split
	x->shufps ( XMM1, XMM0, XMM0, ( 2 ) + ( 2 << 2 ) + ( 2 << 4 ) + ( 2 << 6 ) );

	// perform op
	x->cmpleps ( XMM2, XMM0, XMM1 );
	
	// extract
	x->extractps ( RAX, XMM2, 0 );
	
	// mask
	x->AndRegImm32 ( RAX, ( 1 << 23 ) );
	
	// clear C status flag (bit 23) in FCR31
	x->AndMemImm32 ( ~( ( 1 << 23 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// logical OR with FCR31
	return x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );
}

bool R5900Encoder::CLTS ( long fs, long ft )
{
	Format22 ( fs, ft );
	
	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// clamp
	x->cmpunordps ( XMM1, XMM0, XMM0 );
	x->andps ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstMantissaOnlyMask ) );
	x->andnps ( XMM0, XMM0, XMM1 );

	// split
	x->shufps ( XMM1, XMM0, XMM0, ( 2 ) + ( 2 << 2 ) + ( 2 << 4 ) + ( 2 << 6 ) );

	// perform op
	x->cmpltps ( XMM2, XMM0, XMM1 );
	
	// extract
	x->extractps ( RAX, XMM2, 0 );
	
	// mask
	x->AndRegImm32 ( RAX, ( 1 << 23 ) );
	
	// clear C status flag (bit 23) in FCR31
	x->AndMemImm32 ( ~( ( 1 << 23 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// logical OR with FCR31
	return x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );
}

bool R5900Encoder::CFC1 ( long rt, long fs )
{
	rt = r->AllocGPRToIntReg ( rt, false );
	r->SetTargetIntRegAsModified ( rt );
	
	x->MovRegFromMem32 ( rt, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ fs ] ) );
	return x->MovsxdReg64Reg32 ( rt, rt );
}

bool R5900Encoder::CTC1 ( long rt, long fs )
{
	rt = r->AllocGPRToIntReg ( rt, true );
	
	return x->MovRegToMem32 ( rt, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ fs ] ) );
}

bool R5900Encoder::CVTSW ( long fd, long fs )
{
	Format20 ( fd, fs );

	// convert 32-bit value into float
	return x->cvtdq2ps ( fd, fs );
}

bool R5900Encoder::CVTWS ( long fd, long fs )
{
	Format20 ( fd, fs );
	
	// ** todo ** make sure floating point value is valid
	
	// convert from floating point to fixed point
	x->cvtps2dq ( fd, fs );
	
	// check if source is greater than 2147483647
	x->cmpgtps ( XMM1, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst2147483647 ) );
	
	// bring in max positive value if needed
	return x->blendvps ( fd, fd, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( IntegerConst2147483647 ), XMM1 );
}

// in ieee floating point:
// x / 0 = +/-inf
// 0 / 0 = invalid
bool R5900Encoder::DIVS ( long fd, long fs, long ft )
{
	Format21 ( fd, fs, ft );

	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// convert
	x->cvtps2pd ( XMM0, XMM0 );

	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );

	// split
	x->vperm2f128 ( XMM1, XMM0, XMM0, 1 );

	// perform op
	// x / 0 = +/-Inf in ieee
	// 0 / 0 = Invalid in ieee, but may be able to clamp result
	x->divpd ( XMM2, XMM0, XMM1 );
	
	// check if fs = ft = 0
	x->orpd ( XMM3, XMM0, XMM1 );
	x->cmpeqpd ( XMM3, XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstZero ) );
	
	// put this in I Flag (bit 17) and SI Sticky Flag (bit 6)
	x->extractps ( RAX, XMM3, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 17 ) + ( 1 << 6 ) );
	
	// check if ft = 0 and fs <> 0
	x->cmpeqpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstZero ) );
	x->andnpd ( XMM1, XMM1, XMM3 );
	
	// put this in D Flag (bit 16) and SD Sticky Flag (bit 5)
	x->extractps ( RCX, XMM1, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 16 ) + ( 1 << 5 ) );
	
	// combine flags
	x->OrRegReg32 ( RAX, RCX );
	
	// clear I Flag and D Flag
	x->AndMemImm32 ( ~( ( 1 << 17 ) + ( 1 << 16 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );
	
	// logical or flags
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );
	
	// fix unordered result after 0 / 0
	x->andpd ( XMM3, XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstMantissaOnlyMask ) );
	x->andnpd ( XMM2, XMM2, XMM3 );
	
	// convert
	return x->cvtpd2ps ( fd, XMM2 );
}

bool R5900Encoder::MADDS ( long fd, long fs, long ft )
{
	Format21 ( fd, fs, ft );
	
	// load acc
	x->movss_from_mem128 ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( ACC ) );

	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// combine
	x->shufps ( XMM0, XMM0, XMM2, ( 0 ) + ( 2 << 2 ) + ( 0 << 4 ) );
	
	// convert
	x->cvtps2pd ( XMM0, XMM0 );

	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );

	// split
	x->vperm2f128 ( XMM2, XMM0, XMM0, 1 );

	// split
	x->shufps ( XMM1, XMM0, XMM0, 1 );

	// perform op
	x->mulpd ( XMM0, XMM0, XMM1 );
	x->addpd ( XMM0, XMM0, XMM2 );

	// check for underflow

		// get absolute value of result - put absolute value of result in XMM1
		x->andnpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );
		
		// check if lower than minimum positive ps2 value
		x->cmpltpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Min ) );

	// extract and mask for underflow status flag (bit 14) and underflow sticky flag (bit 3) in FCR31
	x->pextrq ( RAX, XMM2, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 14 ) + ( 1 << 3 ) );

	// check for overflow
	x->cmpgtpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );

	// extract and mask for overflow status flag (bit 15) and overflow sticky flag (bit 4) in FCR31
	x->pextrq ( RCX, XMM2, 0 );
	x->AndRegImm32 ( RCX, ( 1 << 15 ) + ( 1 << 4 ) );

	// logical OR flags
	x->OrRegReg32 ( RAX, RCX );

	// clear O and U status flags in FCR31
	x->AndMemImm32 ( ~( ( 1 << 14 ) + ( 1 << 15 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// logical OR with FCR31
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// convert and store
	return x->cvtpd2ps ( fd, XMM0 );
}

bool R5900Encoder::MADDAS ( long fs, long ft )
{
	Format22 ( fs, ft );

	// load acc
	x->movss_from_mem128 ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( ACC ) );

	// combine
	x->shufps ( XMM0, fs, ft, 0 );

	// combine
	x->shufps ( XMM0, XMM0, XMM2, ( 0 ) + ( 2 << 2 ) + ( 0 << 4 ) );

	// convert
	x->cvtps2pd ( XMM0, XMM0 );

	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );

	// split
	x->vperm2f128 ( XMM2, XMM0, XMM0, 1 );

	// split
	x->shufps ( XMM1, XMM0, XMM0, 1 );
	
	// perform op
	x->mulpd ( XMM0, XMM0, XMM1 );
	x->addpd ( XMM0, XMM0, XMM2 );
	
	// check for underflow

		// get absolute value of result - put absolute value of result in XMM1
		x->andnpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );
		
		// check if lower than minimum positive ps2 value
		x->cmpltpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Min ) );
	
	// extract and mask for underflow status flag (bit 14) and underflow sticky flag (bit 3) in FCR31
	x->pextrq ( RAX, XMM2, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 14 ) + ( 1 << 3 ) );

	// check for overflow
	x->cmpgtpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	
	// extract and mask for overflow status flag (bit 15) and overflow sticky flag (bit 4) in FCR31
	x->pextrq ( RCX, XMM2, 0 );
	x->AndRegImm32 ( RCX, ( 1 << 15 ) + ( 1 << 4 ) );

	// logical OR flags
	x->OrRegReg32 ( RAX, RCX );

	// clear O and U status flags in FCR31
	x->AndMemImm32 ( ~( ( 1 << 14 ) + ( 1 << 15 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// logical OR with FCR31
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// convert
	x->cvtpd2ps ( XMM0, XMM0 );

	// store to accumulator
	return x->movss_to_mem128 ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( ACC ) );
}

bool R5900Encoder::MSUBS ( long fd, long fs, long ft )
{
	Format21 ( fd, fs, ft );
	
	// load acc
	x->movss_from_mem128 ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( ACC ) );

	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// combine
	x->shufps ( XMM0, XMM0, XMM2, ( 0 ) + ( 2 << 2 ) + ( 0 << 4 ) );
	
	// convert
	x->cvtps2pd ( XMM0, XMM0 );

	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );

	// split
	x->vperm2f128 ( XMM2, XMM0, XMM0, 1 );

	// split
	x->shufps ( XMM1, XMM0, XMM0, 1 );

	// perform op
	x->mulpd ( XMM0, XMM0, XMM1 );
	x->subpd ( XMM0, XMM0, XMM2 );
	
	// check for underflow

		// get absolute value of result - put absolute value of result in XMM1
		x->andnpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );
		
		// check if lower than minimum positive ps2 value
		x->cmpltpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Min ) );
	
	// extract and mask for underflow status flag (bit 14) and underflow sticky flag (bit 3) in FCR31
	x->pextrq ( RAX, XMM2, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 14 ) + ( 1 << 3 ) );

	// check for overflow
	x->cmpgtpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	
	// extract and mask for overflow status flag (bit 15) and overflow sticky flag (bit 4) in FCR31
	x->pextrq ( RCX, XMM2, 0 );
	x->AndRegImm32 ( RCX, ( 1 << 15 ) + ( 1 << 4 ) );

	// logical OR flags
	x->OrRegReg32 ( RAX, RCX );

	// clear O and U status flags in FCR31
	x->AndMemImm32 ( ~( ( 1 << 14 ) + ( 1 << 15 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// logical OR with FCR31
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// convert and store
	return x->cvtpd2ps ( fd, XMM0 );
}

bool R5900Encoder::MSUBAS ( long fs, long ft )
{
	Format22 ( fs, ft );
	
	// load acc
	x->movss_from_mem128 ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( ACC ) );

	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// combine
	x->shufps ( XMM0, XMM0, XMM2, ( 0 ) + ( 2 << 2 ) + ( 0 << 4 ) );
	
	// convert
	x->cvtps2pd ( XMM0, XMM0 );

	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );

	// split
	x->vperm2f128 ( XMM2, XMM0, XMM0, 1 );

	// split
	x->shufps ( XMM1, XMM0, XMM0, 1 );

	// perform op
	x->mulpd ( XMM0, XMM0, XMM1 );
	x->subpd ( XMM0, XMM0, XMM2 );
	
	// check for underflow

		// get absolute value of result - put absolute value of result in XMM1
		x->andnpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );
		
		// check if lower than minimum positive ps2 value
		x->cmpltpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Min ) );
	
	// extract and mask for underflow status flag (bit 14) and underflow sticky flag (bit 3) in FCR31
	x->pextrq ( RAX, XMM2, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 14 ) + ( 1 << 3 ) );

	// check for overflow
	x->cmpgtpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	
	// extract and mask for overflow status flag (bit 15) and overflow sticky flag (bit 4) in FCR31
	x->pextrq ( RCX, XMM2, 0 );
	x->AndRegImm32 ( RCX, ( 1 << 15 ) + ( 1 << 4 ) );

	// logical OR flags
	x->OrRegReg32 ( RAX, RCX );

	// clear O and U status flags in FCR31
	x->AndMemImm32 ( ~( ( 1 << 14 ) + ( 1 << 15 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// logical OR with FCR31
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// convert
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// store to accumulator
	return x->movss_to_mem128 ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( ACC ) );
}

bool R5900Encoder::MAXS ( long fd, long fs, long ft )
{
	Format21 ( fd, fs, ft );

	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// clamp
	x->cmpunordps ( XMM1, XMM0, XMM0 );
	x->andps ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstMantissaOnlyMask ) );
	x->andnps ( XMM0, XMM0, XMM1 );

	// split
	x->shufps ( XMM1, XMM0, XMM0, ( 2 ) + ( 2 << 2 ) + ( 2 << 4 ) + ( 2 << 6 ) );

	// perform op
	x->maxps ( fd, XMM0, XMM1 );

	// clear O (bit 15) and U (bit 14) Flags in FCR31
	return x->AndMemImm32 ( ~( ( 1 << 14 ) + ( 1 << 15 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );
}

bool R5900Encoder::MFC1 ( long rt, long fs )
{
	Format44 ( rt, fs );
	
	x->extractps ( rt, fs, 0 );
	return x->MovsxdReg64Reg32 ( rt, rt );
}

bool R5900Encoder::MINS ( long fd, long fs, long ft )
{
	Format21 ( fd, fs, ft );

	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// clamp
	x->cmpunordps ( XMM1, XMM0, XMM0 );
	x->andps ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstMantissaOnlyMask ) );
	x->andnps ( XMM0, XMM0, XMM1 );

	// split
	x->shufps ( XMM1, XMM0, XMM0, ( 2 ) + ( 2 << 2 ) + ( 2 << 4 ) + ( 2 << 6 ) );

	// perform op
	x->minps ( fd, XMM0, XMM1 );

	// clear O (bit 15) and U (bit 14) Flags in FCR31
	return x->AndMemImm32 ( ~( ( 1 << 14 ) + ( 1 << 15 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );
}

bool R5900Encoder::MOVS ( long fd, long fs )
{
	Format20 ( fd, fs );

	return x->movaps ( fd, fs );
}

bool R5900Encoder::MTC1 ( long rt, long fs )
{
	Format45 ( fs, rt );
	
	return x->pinsrd ( rt, fs, fs, (char) 0 );
}

bool R5900Encoder::MULS ( long fd, long fs, long ft )
{
	Format21 ( fd, fs, ft );
	
	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// convert
	x->cvtps2pd ( XMM0, XMM0 );

	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );

	// split
	x->vperm2f128 ( XMM1, XMM0, XMM0, 1 );

	// perform op
	x->mulpd ( XMM0, XMM0, XMM1 );
	
	// check for underflow

		// get absolute value of result - put absolute value of result in XMM1
		x->andnpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );
		
		// check if lower than minimum positive ps2 value
		x->cmpltpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Min ) );
	
	// extract and mask for underflow status flag (bit 14) and underflow sticky flag (bit 3) in FCR31
	x->pextrq ( RAX, XMM2, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 14 ) + ( 1 << 3 ) );

	// check for overflow
	x->cmpgtpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	
	// extract and mask for overflow status flag (bit 15) and overflow sticky flag (bit 4) in FCR31
	x->pextrq ( RCX, XMM2, 0 );
	x->AndRegImm32 ( RCX, ( 1 << 15 ) + ( 1 << 4 ) );

	// logical OR flags
	x->OrRegReg32 ( RAX, RCX );

	// clear O and U status flags in FCR31
	x->AndMemImm32 ( ~( ( 1 << 14 ) + ( 1 << 15 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// logical OR with FCR31
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// convert and store
	return x->cvtpd2ps ( fd, XMM0 );
}

bool R5900Encoder::MULAS ( long fs, long ft )
{
	Format22 ( fs, ft );
	
	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// convert
	x->cvtps2pd ( XMM0, XMM0 );

	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );

	// split
	x->vperm2f128 ( XMM1, XMM0, XMM0, 1 );

	// perform op
	x->mulpd ( XMM0, XMM0, XMM1 );
	
	// check for underflow

		// get absolute value of result - put absolute value of result in XMM1
		x->andnpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );
		
		// check if lower than minimum positive ps2 value
		x->cmpltpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Min ) );
	
	// extract and mask for underflow status flag (bit 14) and underflow sticky flag (bit 3) in FCR31
	x->pextrq ( RAX, XMM2, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 14 ) + ( 1 << 3 ) );

	// check for overflow
	x->cmpgtpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	
	// extract and mask for overflow status flag (bit 15) and overflow sticky flag (bit 4) in FCR31
	x->pextrq ( RCX, XMM2, 0 );
	x->AndRegImm32 ( RCX, ( 1 << 15 ) + ( 1 << 4 ) );

	// logical OR flags
	x->OrRegReg32 ( RAX, RCX );

	// clear O and U status flags in FCR31
	x->AndMemImm32 ( ~( ( 1 << 14 ) + ( 1 << 15 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// logical OR with FCR31
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// convert
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// store to accumulator
	return x->movss_to_mem128 ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( ACC ) );
}

bool R5900Encoder::NEGS ( long fd, long fs )
{
	Format20 ( fd, fs );
	
	return x->xorps ( fd, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstSignOnlyMask ) );
}

bool R5900Encoder::RSQRTS ( long fd, long fs, long ft )
{
	Format21 ( fd, fs, ft );

	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// convert
	x->cvtps2pd ( XMM0, XMM0 );

	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );

	// split
	x->vperm2f128 ( XMM1, XMM0, XMM0, 1 );

	// absolute value of ft
	x->andnpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );

	// perform op
	// if ft is zero then result should be +/- max
	x->sqrtpd ( XMM3, XMM2 );
	x->divpd ( XMM2, XMM0, XMM3 );

	// check if ft is zero
	x->cmpeqps ( XMM1, ft, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstZero ) );

	// Make D Flag (bit 16) and SD Flag (bit 5)
	x->extractps ( RAX, XMM1, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 16 ) + ( 1 << 5 ) );
	
	// check if ft < 0
	x->extractps ( RCX, ft, 0 );
	x->SarRegImm32 ( RCX, 31 );
	
	// Make I Flag (bit 17) and SI Flag (bit 6)
	x->AndRegImm32 ( RCX, ( 1 << 17 ) + ( 1 << 6 ) );
	
	// combine flags
	x->OrRegReg32 ( RAX, RCX );
	
	// clear I Flag and D Flag in FCR [ 31 ]
	x->AndMemImm32 ( ~( ( 1 << 16 ) + ( 1 << 17 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );
	
	// logical Or flags with FCR [ 31 ]
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// convert result
	x->cvtpd2ps ( fd, XMM2 );
	
	// bring +/-max into result if ft is zero
	x->andps ( XMM0, fd, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstSignOnlyMask ) );
	x->orps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstPositivePS2Max ) );
	return x->blendps ( fd, fd, XMM0, XMM1 );
}

// in ieee floating point:
// sqrt ( -x ) = -invalid
// sqrt ( +inf ) = +inf
// sqrt ( -inf ) = -invalid
bool R5900Encoder::SQRTS ( long fd, long ft )
{
	Format25 ( fd, ft );
	
	// convert
	x->cvtps2pd ( XMM0, XMM0 );

	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );
	
	// absolute value
	x->andnpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );

	// perform op
	x->sqrtpd ( XMM2, XMM1 );

	// clear I Flag (bit 17) and D Flag (bit 16)
	x->AndMemImm32 ( ( 1 << 17 ) + ( 1 << 16 ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// check if ft is below zero
	x->cmpltpd ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstZero ) );

	// make I flag (bit 17) and SI flag (bit 6) - it is ok to use ps instruction here
	x->extractps ( RAX, XMM0, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 17 ) + ( 1 << 6 ) );

	// logical or with status flag
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// convert result
	return x->cvtpd2ps ( fd, XMM2 );
}

bool R5900Encoder::SUBS ( long fd, long fs, long ft )
{
	Format21 ( fd, fs, ft );
	
	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// convert
	x->cvtps2pd ( XMM0, XMM0 );

	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );

	// split
	x->vperm2f128 ( XMM1, XMM0, XMM0, 1 );

	// perform op
	x->subpd ( XMM0, XMM0, XMM1 );
	
	// check for underflow

		// get absolute value of result - put absolute value of result in XMM1
		x->andnpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );
		
		// check if lower than minimum positive ps2 value
		x->cmpltpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Min ) );
	
	// extract and mask for underflow status flag (bit 14) and underflow sticky flag (bit 3) in FCR31
	x->pextrq ( RAX, XMM2, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 14 ) + ( 1 << 3 ) );

	// check for overflow
	x->cmpgtpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	
	// extract and mask for overflow status flag (bit 15) and overflow sticky flag (bit 4) in FCR31
	x->pextrq ( RCX, XMM2, 0 );
	x->AndRegImm32 ( RCX, ( 1 << 15 ) + ( 1 << 4 ) );

	// logical OR flags
	x->OrRegReg32 ( RAX, RCX );

	// clear O and U status flags in FCR31
	x->AndMemImm32 ( ~( ( 1 << 14 ) + ( 1 << 15 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// logical OR with FCR31
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// convert and store
	return x->cvtpd2ps ( fd, XMM0 );
}

bool R5900Encoder::SUBAS ( long fs, long ft )
{
	Format22 ( fs, ft );
	
	// combine
	x->shufps ( XMM0, fs, ft, 0 );
	
	// convert
	x->cvtps2pd ( XMM0, XMM0 );

	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );

	// split
	x->vperm2f128 ( XMM1, XMM0, XMM0, 1 );

	// perform op
	x->subpd ( XMM0, XMM0, XMM1 );
	
	// check for underflow

		// get absolute value of result - put absolute value of result in XMM1
		x->andnpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );
		
		// check if lower than minimum positive ps2 value
		x->cmpltpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Min ) );
	
	// extract and mask for underflow status flag (bit 14) and underflow sticky flag (bit 3) in FCR31
	x->pextrq ( RAX, XMM2, 0 );
	x->AndRegImm32 ( RAX, ( 1 << 14 ) + ( 1 << 3 ) );

	// check for overflow
	x->cmpgtpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	
	// extract and mask for overflow status flag (bit 15) and overflow sticky flag (bit 4) in FCR31
	x->pextrq ( RCX, XMM2, 0 );
	x->AndRegImm32 ( RCX, ( 1 << 15 ) + ( 1 << 4 ) );

	// logical OR flags
	x->OrRegReg32 ( RAX, RCX );

	// clear O and U status flags in FCR31
	x->AndMemImm32 ( ~( ( 1 << 14 ) + ( 1 << 15 ) ), R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// logical OR with FCR31
	x->OrMemReg32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FCR [ 31 ] ) );

	// convert
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// store to accumulator
	return x->movss_to_mem128 ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( ACC ) );
}


// cop2 (vu0) simple integer instructions

bool R5900Encoder::VIADD ( long id, long is, long it )
{
	Format33 ( id, is, it );
	return x->LeaRegRegReg16 ( id, is, it );
}

bool R5900Encoder::VIADDI ( long it, long is, long Imm5 )
{
	Format34 ( it, is, Imm5 );
	return x->LeaRegRegImm16 ( it, is, Imm5 );
}

bool R5900Encoder::VIAND ( long id, long is, long it )
{
	Format33 ( id, is, it );
	
	if ( id == is ) return x->AndRegReg16 ( id, it );
	if ( id == it ) return x->AndRegReg16 ( id, is );
	if ( is == it ) return x->MovRegReg16 ( id, is );
	if ( id == is && id == it ) return true;
	
	x->MovRegReg16 ( id, is );
	return x->AndRegReg16 ( id, it );
}

bool R5900Encoder::VILWR ( long dest, long it, long is )
{
	Format35FromMem( dest, it, is );
	
	// we need address * 16
	x->LeaRegRegReg16 ( RAX, is, is );
	
	return x->MovRegFromMem16 ( it, R8, RAX, SCALE_EIGHT, ((long long) &PS2SystemState.VU0Mem [ dest * 4 ] ) - ((long long) &PS2SystemState) );
}

bool R5900Encoder::VIOR ( long id, long is, long it )
{
	Format33 ( id, is, it );
	
	if ( id == is ) return x->OrRegReg16 ( id, it );
	if ( id == it ) return x->OrRegReg16 ( id, is );
	if ( is == it ) return x->MovRegReg16 ( id, is );
	if ( id == is && id == it ) return true;

	x->MovRegReg16 ( id, is );
	return x->OrRegReg16 ( id, it );
}

bool R5900Encoder::VISUB ( long id, long is, long it )
{
	Format33 ( id, is, it );
	
	if ( id == is ) return x->SubRegReg16 ( id, it );
	if ( id == it )
	{
		x->NegReg16 ( id );
		return x->AddRegReg16 ( id, is );
	}
	if ( is == it ) return x->XorRegReg16 ( id, id );
	
	x->MovRegReg16 ( id, is );
	return x->SubRegReg16 ( id, it );
}

bool R5900Encoder::VISWR ( long dest, long it, long is )
{
	Format35ToMem( dest, it, is );
	
	// we need address * 16
	x->LeaRegRegReg16 ( RAX, is, is );
	
	return x->MovRegToMem16 ( it, R8, RAX, SCALE_EIGHT, ((long long) &PS2SystemState.VU0Mem [ dest * 4 ] ) - ((long long) &PS2SystemState) );
}


// vu float instructions


bool R5900Encoder::VABS ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );
	x->andnps ( XMM0, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstSignOnlyMask ) );
	return x->blendps ( ft, ft, XMM0, dest );
}

// 8 instructions
void R5900Encoder::ClampXMM0AndXMM3 ( void )
{
	// clamp
	x->andpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM0, XMM0, XMM1 );

//	x->andpd ( XMM1, XMM0, R8, GetOffsetForPS2Data ( DoubleConstSignAndExponentOnlyMask ) );
//	x->cmpgtpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
//	x->blendvpd ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ), XMM2 );
//	x->cmpltpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
//	x->blendvpd ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ), XMM2 );

	// clamp
	x->andpd ( XMM1, XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstExponentOnlyMask ) );
	x->cmpgtpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->andnpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->andnpd ( XMM3, XMM3, XMM1 );

//	x->andpd ( XMM2, XMM3, R8, GetOffsetForPS2Data ( DoubleConstSignAndExponentOnlyMask ) );
//	x->cmpgtpd ( XMM1, XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
//	x->blendvpd ( XMM3, XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ), XMM1 );
//	x->cmpltpd ( XMM1, XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
//	x->blendvpd ( XMM3, XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ), XMM1 );
}

// 12 instructions
void R5900Encoder::SetVUFlagsForXMM0 ( void )
{
	// store to mac sign flag
	x->movapdtomem ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].MACFlagSign ) );
	
	// logical or with sign sticky flag and store
	x->vmaskmovpdtomem ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagSign ), XMM0 );
//	x->orpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagSign ) );
//	x->movapd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagSign ) );
	
	// get absolute value of result
	x->andnpd ( XMM2, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );

	// check for overflow
	x->cmpgtpd ( XMM1, XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	
	// store to mac overflow flag
	x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].MACFlagOverflow ) );
	
	// logical or with sticky overflow flag and store
	x->vmaskmovpdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagOverflow ), XMM1 );
//	x->orpd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagOverflow ) );
//	x->movapd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagOverflow ) );

	// check for underflow
	x->cmpltpd ( XMM1, XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Min ) );

	// store to mac underflow flag
	x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].MACFlagUnderflow ) );

	// logical or with sticky underflow flag and store
	x->vmaskmovpdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagUnderflow ), XMM1 );
//	x->orpd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagUnderflow ) );
//	x->movapd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagUnderflow ) );

	// check for zero
	x->cmpeqpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstZero ) );

	// store to mac zero flag
	x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].MACFlagZero ) );

	// logical or with sticky zero flag and store
	x->vmaskmovpdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagZero ), XMM1 );
//	x->orpd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagZero ) );
//	x->movapd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagZero ) );
}

// with 128-bit registers:
// inf + inf = inf
// -inf + inf = -ind
// -inf + -inf = -inf
// -inf + x = -inf
// inf + -inf = -ind
bool R5900Encoder::VADD ( long dest, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, ft );
	
	ClampXMM0AndXMM3 ();
	
	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();

	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, dest );
}

bool R5900Encoder::VADDi ( long dest, long fd, long fs )
{
	Format28 ( dest, fd, fs );

	// load in I register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].I ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, dest );
}

bool R5900Encoder::VADDq ( long dest, long fd, long fs )
{
	Format28 ( dest, fd, fs );

	// load in Q register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, dest );
}

bool R5900Encoder::VADDbc ( long dest, long bc, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );

	// broadcast bc
	x->shufps ( XMM3, ft, ft, (char) (( bc << 6 ) + ( bc << 4 ) + ( bc << 2 ) + bc) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();

	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );

	// blend
	return x->blendpd ( fd, fd, XMM0, dest );
}

bool R5900Encoder::VADDA ( long dest, long fs, long ft )
{
	Format29 ( dest, fs, ft );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, ft );
	
	ClampXMM0AndXMM3 ();
	
	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VADDAi ( long dest, long fs )
{
	Format30 ( dest, fs );

	// load in I register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].I ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VADDAq ( long dest, long fs )
{
	Format30 ( dest, fs );

	// load in Q register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VADDAbc ( long dest, long bc, long fs, long ft )
{
	Format29 ( dest, fs, ft );

	// broadcast bc
	x->shufps ( XMM3, ft, ft, (char) (( bc << 6 ) + ( bc << 4 ) + ( bc << 2 ) + bc) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();

	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VCLIP ( long fs, long ft )
{
	Format43 ( fs, ft );
	
	// ** todo ** don't forget to make sure floating point values are valid
	
	// broadcast w from ft
	x->shufps ( XMM0, ft, ft, 0xff );

	// clear sign
	x->andps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatExponentAndMantissaMask ) );
	
	// set sign
	x->orps ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatSignMask ) );
	
	// check if greater than plus - x+,y+,z+
	x->cmpltps ( XMM0, XMM0, fs );
	
	// check if less than minus - x-,y-,z-
	x->cmpltps ( XMM1, fs, XMM1 );
	
	// shuffle - x+,y+,x-,y-
	x->shufps ( XMM2, XMM0, XMM1, ( 0 ) + ( 1 << 2 ) + ( 0 << 4 ) + ( 1 << 6 ) );
	
	// shuffle - x+,x-,y+,y-
	x->shufps ( XMM2, XMM2, XMM2, ( 3 << 6 ) + ( 1 << 4 ) + ( 2 << 2 ) + ( 0 ) );

	// shuffle - z+,z+,z-,z-
	x->shufps ( XMM0, XMM0, XMM1, ( 2 << 6 ) + ( 2 << 4 ) + ( 2 << 2 ) + ( 2 ) );
	
	// shuffle - z+, z-
	x->shufps ( XMM0, XMM0, XMM0, ( 2 << 2 ) + ( 0 ) );
	
	// permute
	x->vperm2f128 ( XMM2, XMM2, XMM0, ( 2 << 4 ) + ( 0 ) );
	
	// create flags
	x->movmskps256 ( RAX, XMM2 );
	
	// load clipping flag
	x->MovRegFromMem32 ( RCX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ClippingFlag ) );
	
	// shift clipping flag
	x->ShlRegImm32 ( RCX, 6 );
	
	// mask flag
	x->AndRegImm16 ( RAX, 0x3f );
	
	// logical or with clipping flag
	x->OrRegReg32 ( RCX, RAX );
	
	// store clipping flag
	x->MovRegToMem32 ( RCX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ClippingFlag ) );
}

bool R5900Encoder::VDIV ( long ftf, long fsf, long fs, long ft )
{
	Format32 ( ftf, fsf, fs, ft );

	// float to double
	x->cvtps2pd ( XMM1, ft );
	x->cvtps2pd ( XMM0, fs );

	// clamp double
	x->cmpgtpd ( XMM2, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->blendvpd ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ), XMM2 );
	x->cmpltpd ( XMM2, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->blendvpd ( XMM0, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ), XMM2 );
	x->cmpgtpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->blendvpd ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ), XMM2 );
	x->cmpltpd ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->blendvpd ( XMM1, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ), XMM2 );

	// perform division
	x->divpd ( XMM0, XMM0, XMM1 );

	// check for divide by zero
	x->cvtpd2ps ( XMM1, XMM1 );
	x->cmpeqps ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstZero ) );

	// store to D flag
	x->movapstomem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StatusFlagDivide ) );

	// logical or with D sticky flag
	x->orps ( XMM1, XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagDivide ) );

	// store to D Sticky flag
	x->movapstomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagDivide ) );
	
	// double result to float
	x->cvtpd2ps ( XMM0, XMM0 );

	// maximize on divide by zero but preserve sign
	x->blendvps ( XMM1, XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstExponentAndMantissaMask ), XMM2 );
	x->andnps ( XMM0, XMM0, XMM1 );
	x->blendvps ( XMM2, XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstPositivePS2Max ), XMM2 );
	x->orps ( XMM0, XMM0, XMM2 );

	// store to Q
	return x->movapstomem ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );
}

// cvttps2dq
// inf = invalid integer
bool R5900Encoder::VFTOI0 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );
	
	// ** todo ** make sure floating point value is valid
	
	// convert from floating point to fixed point
	x->cvtps2dq ( XMM0, fs );
	
	// check if source is greater than 2147483647
	x->cmpgtps ( XMM1, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst2147483647 ) );
	
	// bring in max positive value if needed
	x->blendvps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( IntegerConst2147483647 ), XMM1 );
	
	// put in result
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool R5900Encoder::VFTOI4 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );
	
	// ** todo ** make sure floating point value is valid

	// multiply by 16
	x->mulps ( XMM0, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst16 ) );

	// convert from floating point to fixed point
	x->cvtps2dq ( XMM0, XMM0 );
	
	// check if source is greater than 2147483647
	x->cmpgtps ( XMM1, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst2147483647 ) );
	
	// bring in max positive value if needed
	x->blendvps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( IntegerConst2147483647 ), XMM1 );
	
	// put in result
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool R5900Encoder::VFTOI12 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );

	// ** todo ** make sure floating point value is valid

	// multiply by 4096
	x->mulps ( XMM0, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst4096 ) );

	// convert from floating point to fixed point
	x->cvtps2dq ( XMM0, XMM0 );
	
	// check if source is greater than 2147483647
	x->cmpgtps ( XMM1, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst2147483647 ) );
	
	// bring in max positive value if needed
	x->blendvps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( IntegerConst2147483647 ), XMM1 );
	
	// put in result
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool R5900Encoder::VFTOI15 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );

	// ** todo ** make sure floating point value is valid

	// multiply by 32768
	x->mulps ( XMM0, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst32768 ) );

	// convert from floating point to fixed point
	x->cvtps2dq ( XMM0, XMM0 );
	
	// check if source is greater than 2147483647
	x->cmpgtps ( XMM1, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst2147483647 ) );
	
	// bring in max positive value if needed
	x->blendvps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( IntegerConst2147483647 ), XMM1 );
	
	// put in result
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool R5900Encoder::VITOF0 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );

	// convert 32-bit value into float
	x->cvtdq2ps ( XMM0, fs );

	// put into result
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool R5900Encoder::VITOF4 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );

	// convert 32-bit value into float
	x->cvtdq2ps ( XMM0, fs );
	
	// divide by 16
	x->divps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst16 ) );

	// put into result
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool R5900Encoder::VITOF12 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );

	// convert 32-bit value into float
	x->cvtdq2ps ( XMM0, fs );

	// divide by 4096
	x->divps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst4096 ) );

	// put into result
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool R5900Encoder::VITOF15 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );

	// convert 32-bit value into float
	x->cvtdq2ps ( XMM0, fs );

	// divide by 32768
	x->divps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst32768 ) );

	// put into result
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

void R5900Encoder::SetMADDVUFlagsAndGetResult ( void )
{
	// start of accurateness
	
	// ** underflow flag ** // - complete

	// set underflow sticky flag ONLY on MULT underflow ONLY - accurate
	
		// get absolute value of MULT result
		x->andnpd ( XMM7, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );

		// check if smaller than smallest possible PS2 float value - puts MULT underflow into XMM7
		x->cmpltpd ( XMM7, XMM7, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Min ) );

		// store underflow sticky flag
		x->vmaskmovpdtomem ( XMM7, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagUnderflow ), XMM7 );
	
	// always clear underflow mac flags ONLY - accurate
	x->xorpd ( XMM3, XMM3, XMM3 );
	x->movapdtomem ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].MACFlagUnderflow ) );

	// ** zero flag ** // - complete

	// set zero mac status and sticky flags from ADD result if ACC normal, else set zero sticky flag ONLY from MULT result ONLY and clear zero mac flag - accurate

		// check if ACC is +/-max

		// get absolute value of ACC
		x->andnpd ( XMM4, XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );

		// check if it is max - put ACC overflow into XMM4
		x->cmpgepd ( XMM4, XMM4, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
		
		// do zero mac status flags first

		// when checking zero mac status flag, will need to chop off bottom bits
		// for mac flags, if ACC overflow, bring in zero, else bring in ADD result - will be using raw result for mac zero flags
		x->blendpd ( XMM3, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstZero ), XMM4 );
		
		// store to zero mac flags
		x->movapdtomem ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].MACFlagZero ) );

		// now do zero sticky flags
		
		// bring in MULT result if ACC +/-max, else bring in ADD result
		x->blendpd ( XMM3, XMM0, XMM1, XMM4 );
		
		// chop off bottom bits since PS2 only does single precision floats
		x->andpd ( XMM3, XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstFloatChopMask ) );
		
		// check if zero
		x->cmpeqpd ( XMM3, XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstZero ) );

		// set zero sticky flags
		x->vmaskmovpdtomem ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagZero ), XMM3 );

	// ** overflow flag ** // - complete

	// set overflow mac AND sticky flags on MULT overflow, ACC overflow, or ADD overflow - accurate
	
		// check for MULT overflow - put MULT overflow into XMM3
		x->andnpd ( XMM3, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );
		x->cmpgtpd ( XMM3, XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
		
		// MULT overflow or ACC overflow - use XMM5 as temp register
		x->orpd ( XMM5, XMM4, XMM3 );
		
		// check for ADD overflow - put ADD overflow into XMM6
		x->andnpd ( XMM6, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );
		x->cmpgtpd ( XMM6, XMM6, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
		
		// logical OR
		x->orpd ( XMM5, XMM5, XMM6 );
		
		// store to overflow mac status and sticky flags
		x->movapdtomem ( XMM5, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].MACFlagOverflow ) );
		x->vmaskmovpdtomem ( XMM5, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagOverflow ), XMM5 );
	
	// ** sign flag ** //

	// set sign mac flag from ADD result ONLY - accurate
	x->movapdtomem ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].MACFlagSign ) );

	// set sign sticky flag ONLY from MULT sign on MULT overflow - accurate
	x->vmaskmovpdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagOverflow ), XMM3 );

	// set sign sticky flag ONLY from MULT OR ADD if not MULT overflow and ACC not normal - accurate

		// check if NOT MULT overflow
		x->xorpd ( XMM5, XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( IntegerConstMaxUnsignedLong ) );
	
		// AND ACC overflow
		x->andpd ( XMM5, XMM5, XMM4 );
		
		// perform MULT OR ADD
		x->orpd ( XMM6, XMM0, XMM1 );
		
		// store overflow sticky flag
		x->vmaskmovpdtomem ( XMM6, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagSign ), XMM5 );

	// set sign sticky flag ONLY from ADD result if ACC normal - accurate

		// check if NOT ACC overflow
		x->xorpd ( XMM5, XMM4, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( IntegerConstMaxUnsignedLong ) );

		// store sign sticky flag from ADD result
		x->vmaskmovpdtomem ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagSign ), XMM5 );

	// end of accurateness


	// now get result
	
	// if MULT overflow then maximize sign of ADD result
	
		// get just sign of ADD result
		x->andpd ( XMM5, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );
		
		// logical or in max positive ps2 float value
		x->orpd ( XMM5, XMM5, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
		
		// blend into result on MULT overflow
		x->blendpd ( XMM0, XMM0, XMM5, XMM3 );
	
	// if not MULT overflow and ACC overflow, then result is ACC
	
		// check if ACC overflow and NOT MULT overflow
		x->andnpd ( XMM5, XMM4, XMM3 );
		
		// blend from ACC into result on condition
		x->blendpd ( XMM0, XMM0, XMM2, XMM5 );
	
	// if MULT underflow and ACC not overflow, then do not store result
	
		// check if MULT underflow and NOT ACC overflow
		x->andnpd ( XMM5, XMM7, XMM4 );
		
		// restore previous result on condition
//		x->cvtps2pd ( XMM1, fd );		// this should stay in, but needs rethought
		x->blendpd ( XMM0, XMM0, XMM1, XMM5 );
	
	// if ACC normal and not MULT underflow and not MULT overflow, then result is ADD

}

// basically an OR of flag values of both ADD and MULT, where +/-max for ACC are treated as overflow, and MULT has to really overflow/underflow
// and underflow mac status flags are never set, only underflow sticky flags
// always clear underflow mac status flag
// if MULT underflow, then set underflow sticky flags
// if single precision ACC is unordered, or MULT result is overflow, or addition result is overflow, then set overflow mac status and sticky flags
// always set sign mac status flag according to results of addition
// always set zero mac status flag and sticky flags according to result of addition
// this should cover must of the flags
bool R5900Encoder::VMADD ( long dest, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );


	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, ft );
	
	ClampXMM0AndXMM3 ();

	// perform op - multiplication result in XMM1, addition result in XMM0, leaving ACC in XMM2
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->addpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();
	
	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMADDi ( long dest, long fd, long fs )
{
	Format28 ( dest, fd, fs );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// load in I register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].I ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->addpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();
	
	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMADDq ( long dest, long fd, long fs )
{
	Format28 ( dest, fd, fs );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// load in Q register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->addpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();
	
	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMADDbc ( long dest, long bc, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// broadcast bc
	x->shufps ( XMM3, ft, ft, (char) (( bc << 6 ) + ( bc << 4 ) + ( bc << 2 ) + bc) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->addpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();

	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );

	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMADDA ( long dest, long fs, long ft )
{
	Format29 ( dest, fs, ft );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, ft );
	
	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->addpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VMADDAi ( long dest, long fs )
{
	Format30 ( dest, fs );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// load in I register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].I ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->addpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VMADDAq ( long dest, long fs )
{
	Format30 ( dest, fs );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// load in Q register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->addpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VMADDAbc ( long dest, long bc, long fs, long ft )
{
	Format29 ( dest, fs, ft );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// broadcast bc
	x->shufps ( XMM3, ft, ft, (char) (( bc << 6 ) + ( bc << 4 ) + ( bc << 2 ) + bc) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->addpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();

	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

// using 128-bit sse registers:
// -inf max +inf = +inf
// -inf max -inf = -inf
// +inf max +inf = +inf
// +x max +inf = +inf
// -x max +inf = +inf
// +x max -inf = +x
// NaNs don't work at all
bool R5900Encoder::VMAX ( long dest, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );

	// check if exponent is 255 in source 1
	x->cmpunordps ( XMM0, fs, fs );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( fs, fs, XMM0 );

	// check if exponent is 255 in source 2
	x->cmpunordps ( XMM0, ft, ft );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( ft, ft, XMM0 );

	x->maxps ( XMM0, fs, ft );
	return x->blendps ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMAXi ( long dest, long fd, long fs )
{
	Format28 ( dest, fd, fs );

	// load in I register
	x->vbroadcastss ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].I ) );

	// check if exponent is 255 in source 1
	x->cmpunordps ( XMM0, fs, fs );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( fs, fs, XMM0 );

	// check if exponent is 255 in source 2
	x->cmpunordps ( XMM0, XMM1, XMM1 );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( XMM1, XMM1, XMM0 );

	x->maxps ( XMM1, XMM1, fs );
	return x->blendps ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMAXbc ( long dest, long bc, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );

	// broadcast bc
	x->shufps ( XMM1, ft, ft, (char) (( bc << 6 ) + ( bc << 4 ) + ( bc << 2 ) + bc) );
	
	// check if exponent is 255 in source 1
	x->cmpunordps ( XMM0, fs, fs );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( fs, fs, XMM0 );

	// check if exponent is 255 in source 2
	x->cmpunordps ( XMM0, XMM1, XMM1 );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( XMM1, XMM1, XMM0 );

	x->maxps ( XMM0, XMM1, fs );
	return x->blendps ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMFIR ( long dest, long ft, long is )
{
	Format42 ( dest, ft, is );
	x->MovsxReg32Reg16 ( EAX, is );
	x->pinsrd ( XMM0, XMM0, EAX, 0 );
	x->shufps ( XMM0, XMM0, XMM0, 0 );
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool R5900Encoder::VMINI ( long dest, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );

	// check if exponent is 255 in source 1
	x->cmpunordps ( XMM0, fs, fs );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( fs, fs, XMM0 );

	// check if exponent is 255 in source 2
	x->cmpunordps ( XMM0, ft, ft );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( ft, ft, XMM0 );

	x->minps ( XMM0, fs, ft );
	return x->blendps ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMINIi ( long dest, long fd, long fs )
{
	Format28 ( dest, fd, fs );

	// load in I register
	x->vbroadcastss ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].I ) );

	// check if exponent is 255 in source 1
	x->cmpunordps ( XMM0, fs, fs );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( fs, fs, XMM0 );

	// check if exponent is 255 in source 2
	x->cmpunordps ( XMM0, XMM1, XMM1 );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( XMM1, XMM1, XMM0 );

	x->minps ( XMM1, XMM1, fs );
	return x->blendps ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMINIbc ( long dest, long bc, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );

	// broadcast bc
	x->shufps ( XMM1, ft, ft, (char) (( bc << 6 ) + ( bc << 4 ) + ( bc << 2 ) + bc) );
	
	// check if exponent is 255 in source 1
	x->cmpunordps ( XMM0, fs, fs );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( fs, fs, XMM0 );

	// check if exponent is 255 in source 2
	x->cmpunordps ( XMM0, XMM1, XMM1 );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( XMM1, XMM1, XMM0 );

	x->minps ( XMM0, XMM1, fs );
	return x->blendps ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMOVE ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );
	return x->blendps ( ft, ft, fs, (char) dest );
}

bool R5900Encoder::VMR32 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );
	x->shufps ( XMM0, fs, fs, ( 2 << 6 ) + ( 1 << 4 ) + ( 0 << 2 ) + ( 3 ) );
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool R5900Encoder::VMSUB ( long dest, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );


	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, ft );
	
	ClampXMM0AndXMM3 ();

	// perform op - multiplication result in XMM1, addition result in XMM0, leaving ACC in XMM2
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->subpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();
	
	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMSUBi ( long dest, long fd, long fs )
{
	Format28 ( dest, fd, fs );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// load in I register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].I ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->subpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();
	
	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMSUBq ( long dest, long fd, long fs )
{
	Format28 ( dest, fd, fs );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// load in Q register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->subpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();
	
	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMSUBbc ( long dest, long bc, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// broadcast bc
	x->shufps ( XMM3, ft, ft, (char) (( bc << 6 ) + ( bc << 4 ) + ( bc << 2 ) + bc) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->subpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();

	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );

	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMSUBA ( long dest, long fs, long ft )
{
	Format29 ( dest, fs, ft );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, ft );
	
	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->subpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VMSUBAi ( long dest, long fs )
{
	Format30 ( dest, fs );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// load in I register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].I ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->subpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VMSUBAq ( long dest, long fs )
{
	Format30 ( dest, fs );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// load in Q register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->subpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VMSUBAbc ( long dest, long bc, long fs, long ft )
{
	Format29 ( dest, fs, ft );

	// load accumulator
	x->movapdfrommem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );

	// broadcast bc
	x->shufps ( XMM3, ft, ft, (char) (( bc << 6 ) + ( bc << 4 ) + ( bc << 2 ) + bc) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM1, XMM0, XMM3 );
	x->subpd ( XMM0, XMM1, XMM2 );
	
	SetMADDVUFlagsAndGetResult ();

	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VMUL ( long dest, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, ft );
	
	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();

	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMULi ( long dest, long fd, long fs )
{
	Format28 ( dest, fd, fs );

	// load in I register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].I ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMULq ( long dest, long fd, long fs )
{
	Format28 ( dest, fd, fs );

	// load in Q register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VMULbc ( long dest, long bc, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );

	// broadcast bc
	x->shufps ( XMM3, ft, ft, (char) (( bc << 6 ) + ( bc << 4 ) + ( bc << 2 ) + bc) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();

	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );

	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}


bool R5900Encoder::VMULA ( long dest, long fs, long ft )
{
	Format29 ( dest, fs, ft );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, ft );
	
	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();

	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VMULAi ( long dest, long fs )
{
	Format30 ( dest, fs );

	// load in I register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].I ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VMULAq ( long dest, long fs )
{
	Format30 ( dest, fs );

	// load in Q register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VMULAbc ( long dest, long bc, long fs, long ft )
{
	Format29 ( dest, fs, ft );

	// broadcast bc
	x->shufps ( XMM3, ft, ft, (char) (( bc << 6 ) + ( bc << 4 ) + ( bc << 2 ) + bc) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();

	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}


bool R5900Encoder::VNOP ( void )
{
	return true;
}

bool R5900Encoder::VOPMULA ( long dest, long fs, long ft )
{
	Format29 ( dest, fs, ft );
	
	// shuffle fs from xyzw to yzx
	x->shufps ( XMM0, fs, fs,  ( 1 ) + ( 2 << 2 ) + ( 0 << 4 ) );
	
	// shuffle ft from xyzw to zxy
	x->shufps ( XMM3, ft, ft, ( 2 ) + ( 0 << 2 ) + ( 1 << 4 ) );

	// convert to double
	x->cvtps2pd ( XMM0, XMM0 );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->mulpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();

	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VMTIR ( long fsf, long it, long fs )
{
	Format41 ( fsf, it, fs );
	return x->pextrw ( it, XMM0, ( fsf << 1 ) );
}

bool R5900Encoder::VRGET ( long dest, long ft )
{
	Format39 ( dest, ft );

	// broadcast R
	x->vbroadcastss ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].R ) );

	// store to ft
	x->blendps ( ft, ft, XMM0, (char) dest );
}

bool R5900Encoder::VRINIT ( long fsf, long fs )
{
	Format36 ( fsf, fs );
	
	// get value to set as random
	x->shufps ( XMM0, fs, fs, fsf );
	
	// we only want mantissa (23 bits)
	x->andps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstMantissaOnlyMask ) );

	// add in sign and exponent for random numbers
	x->orps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstRandomNumberExponent ) );

	// store to R register
	x->movss_to_mem128 ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].R ) );
}

bool R5900Encoder::VRNEXT ( long dest, long ft )
{
	Format39 ( dest, ft );

	// load R register
	x->vbroadcastss ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].R ) );

	// we only want mantissa (23 bits)
	x->andps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstMantissaOnlyMask ) );
	
	// put X^1 into R^5 and R^23
	x->movaps ( XMM1, XMM0 );
	x->movaps ( XMM2, XMM0 );
	
	// compute square X^2
	x->pmuludq ( XMM0, XMM0, XMM0 );
	
	// add into R^23
	x->pmuludq ( XMM2, XMM2, XMM0 );
	
	// compute square X^4
	x->pmuludq ( XMM0, XMM0, XMM0 );
	
	// add into R^5 and R^23
	x->pmuludq ( XMM1, XMM1, XMM0 );
	x->pmuludq ( XMM2, XMM2, XMM0 );
	
	// compute X^16
	x->pmuludq ( XMM0, XMM0, XMM0 );
	x->pmuludq ( XMM0, XMM0, XMM0 );
	
	// add into R^23
	x->pmuludq ( XMM2, XMM2, XMM0 );
	
	// add
	x->paddd ( XMM0, XMM1, XMM2 );
	
	// add 1
	x->vbroadcastss ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( IntegerConst1 ) );
	x->paddd ( XMM0, XMM0, XMM1 );
	
	// get just mantissa
	x->andps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstMantissaOnlyMask ) );
	
	// add in sign and exponent for random numbers
	x->orps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstRandomNumberExponent ) );
	
	// store to ft
	x->blendps ( ft, ft, XMM0, (char) dest );
	
	// store to R register
	return x->movss_to_mem128 ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].R ) );
}

bool R5900Encoder::VRSQRT ( long ftf, long fsf, long fs, long ft )
{
	Format32 ( ftf, fsf, fs, ft );

	// check if exponent is 255 in source 1
	x->cmpunordps ( XMM0, fs, fs );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( fs, fs, XMM0 );

	// check if exponent is 255 in source 2
	x->cmpunordps ( XMM0, ft, ft );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( ft, ft, XMM0 );

	// broadcast ft
	x->shufps ( XMM0, ft, ft, ftf );

	// make I flag
	x->psrad ( XMM1, XMM0, 31 );

	// store I flag
	x->movapstomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StatusFlagInvalid ) );
	
	// logical or with I sticky flag
	x->orps ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagInvalid ) );
	
	// store to I sticky flag
	x->movapstomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagInvalid ) );

	// absolute value
	x->andnps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstSignOnlyMask ) );

	// square root
	x->sqrtps ( XMM0, XMM0 );
	
	// square root of infinity should be 1.84467441x10^19
	x->cmpeqps ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstPositivePS2Max ) );
	x->blendvps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstInfinitySquareRoot ), XMM1 );

	// float to double
	x->cvtps2pd ( XMM1, XMM0 );
	x->cvtps2pd ( XMM0, fs );

	// clamp double - just need to worry about fs
	x->cmpgtpd ( XMM2, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	x->blendvpd ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ), XMM2 );
	x->cmpltpd ( XMM2, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ) );
	x->blendvpd ( XMM0, fs, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstNegativePS2Max ), XMM2 );

	// perform division
	x->divpd ( XMM0, XMM0, XMM1 );

	// check for divide by zero
	x->cvtpd2ps ( XMM1, XMM1 );
	x->cmpeqps ( XMM2, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstZero ) );

	// store to D flag
	x->movapstomem ( XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StatusFlagDivide ) );

	// logical or with D sticky flag
	x->orps ( XMM1, XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagDivide ) );

	// store to D Sticky flag
	x->movapstomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagDivide ) );
	
	// double result to float
	x->cvtpd2ps ( XMM0, XMM0 );

	// maximize on divide by zero but preserve sign
	x->blendps ( XMM1, XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstExponentAndMantissaMask ), XMM2 );
	x->andnps ( XMM0, XMM0, XMM1 );
	x->blendps ( XMM2, XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstPositivePS2Max ), XMM2 );
	x->orps ( XMM0, XMM0, XMM2 );

	// store to Q
	return x->movapstomem ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );
}

bool R5900Encoder::VRXOR ( long fsf, long fs )
{
	Format36 ( fsf, fs );
	
	// get fsfsf
	x->shufps ( XMM0, fs, fs, fsf );
	
	// get R register - would xor here, but there is no xorss instruction
	x->movss_from_mem128 ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].R ) );
	
	// xor
	x->xorps ( XMM0, XMM0, XMM1 );
	
	// get just mantissa
	x->andps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstMantissaOnlyMask ) );
	
	// add in sign and exponent for random numbers
	x->orps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstRandomNumberExponent ) );

	// store to R register
	return x->movss_to_mem128 ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].R ) );
}

bool R5900Encoder::VSQRT ( long ftf, long ft )
{
	// check if exponent is 255 in source 1
	x->cmpunordps ( XMM0, ft, ft );
	x->psrld ( XMM0, XMM0, 9 );
	x->andnps ( ft, ft, XMM0 );

	// broadcast ft
	x->shufps ( XMM0, ft, ft, ftf );

	// make I flag
	x->psrad ( XMM1, XMM0, 31 );

	// store I flag
	x->movapstomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StatusFlagInvalid ) );
	
	// logical or with I sticky flag
	x->orps ( XMM1, XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagInvalid ) );
	
	// store to I sticky flag
	x->movapstomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StickyFlagInvalid ) );
	
	// clear D flag
	x->xorps ( XMM1, XMM1, XMM1 );
	x->movapstomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].StatusFlagDivide ) );

	// absolute value
	x->andnps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstSignOnlyMask ) );

	// square root
	x->sqrtps ( XMM0, XMM0 );
	
	// square root of infinity should be 1.84467441x10^19
	x->cmpeqps ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstPositivePS2Max ) );
	x->blendvps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstInfinitySquareRoot ), XMM1 );

	// store to Q
	return x->movapstomem ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );
}

// cvtps2pd gives QNAN when it is positive invalid
// cvtps2pd gives -QNAN when it is negative invalid
// cvtps2pd gives +/-inf when it is +/-inf
bool R5900Encoder::VSUB ( long dest, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, ft );
	
	ClampXMM0AndXMM3 ();
	
	// perform op
	x->subpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();

	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VSUBi ( long dest, long fd, long fs )
{
	Format28 ( dest, fd, fs );

	// load in I register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].I ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->subpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VSUBq ( long dest, long fd, long fs )
{
	Format28 ( dest, fd, fs );

	// load in Q register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->subpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );
	
	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VSUBbc ( long dest, long bc, long fd, long fs, long ft )
{
	Format27 ( dest, fd, fs, ft );

	// broadcast bc
	x->shufps ( XMM3, ft, ft, (char) (( bc << 6 ) + ( bc << 4 ) + ( bc << 2 ) + bc) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->subpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();

	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );

	// blend
	return x->blendpd ( fd, fd, XMM0, (char) dest );
}

bool R5900Encoder::VSUBA ( long dest, long fs, long ft )
{
	Format29 ( dest, fs, ft );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, ft );
	
	ClampXMM0AndXMM3 ();
	
	// perform op
	x->subpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VSUBAi ( long dest, long fs )
{
	Format30 ( dest, fs );

	// load in I register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].I ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->subpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VSUBAq ( long dest, long fs )
{
	Format30 ( dest, fs );

	// load in Q register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->subpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VSUBAbc ( long dest, long bc, long fs, long ft )
{
	Format29 ( dest, fs, ft );

	// broadcast bc
	x->shufps ( XMM3, ft, ft, (char) (( bc << 6 ) + ( bc << 4 ) + ( bc << 2 ) + bc) );

	// convert to double
	x->cvtps2pd ( XMM0, fs );
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
	
	// perform op
	x->subpd ( XMM0, XMM0, XMM3 );
	
	SetVUFlagsForXMM0 ();

	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
	
	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, (char) dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].ACC ) );
}

bool R5900Encoder::VWAITQ ( void )
{
	return true;
}
*/



