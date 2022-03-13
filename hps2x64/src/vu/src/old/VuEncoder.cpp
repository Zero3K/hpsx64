

#include "Playstation2.h"
#include "x64Encoder.h"

#include "SignExtend.h"

#include "VuEncoder.h"


// need this to use dest with blend instructions
#define REVERSE4BITS(dest)		( ( ( ( dest ) & 0x1 ) << 3 ) | ( ( ( dest ) & 0x2 ) << 1 ) | ( ( ( dest ) & 0x4 ) >> 1 ) | ( ( ( dest ) & 0x8 ) >> 3 ) )

VuEncoder::VuEncoder ( x64Encoder* enc, long VuNbr )
{
	// set the encoder to use
	x = enc;
	
	// set vu number
	VuNumber = VuNbr;
	
	// set the x64 register allocator
//	r = rallocat;
}



bool VuEncoder::ABS ( long dest, long ft, long fs )
{
	REVERSE4BITS( dest );
	x->movaps_from_mem128 ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConstExponentAndMantissaMask ) );
	x->movaps_from_mem128 ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].Vf [ ft ] ) );
	x->andps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].Vf [ fs ] ) );
	x->blendps ( XMM0, XMM1, XMM0, dest );
	x->movaps_to_mem128 ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].Vf [ fs ] ) );
}

// 8 instructions
void VuEncoder::ClampXMM0AndXMM3 ( void )
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
void VuEncoder::SetVUFlagsForXMM0 ( void )
{
	// store to mac sign flag
	x->movapdtomem ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].MACFlagSign ) );
	
	// logical or with sign sticky flag and store
	x->vmaskmovpdtomem ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].StickyFlagSign ), XMM0 );
//	x->orpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagSign ) );
//	x->movapd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagSign ) );
	
	// get absolute value of result
	x->andnpd ( XMM2, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstSignOnlyMask ) );

	// check for overflow
	x->cmpgtpd ( XMM1, XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Max ) );
	
	// store to mac overflow flag
	x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].MACFlagOverflow ) );
	
	// logical or with sticky overflow flag and store
	x->vmaskmovpdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].StickyFlagOverflow ), XMM1 );
//	x->orpd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagOverflow ) );
//	x->movapd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagOverflow ) );

	// check for underflow
	x->cmpltpd ( XMM1, XMM2, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstPositivePS2Min ) );

	// store to mac underflow flag
	x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].MACFlagUnderflow ) );

	// logical or with sticky underflow flag and store
	x->vmaskmovpdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].StickyFlagUnderflow ), XMM1 );
//	x->orpd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagUnderflow ) );
//	x->movapd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagUnderflow ) );

	// check for zero
	x->cmpeqpd ( XMM1, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( DoubleConstZero ) );

	// store to mac zero flag
	x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].MACFlagZero ) );

	// logical or with sticky zero flag and store
	x->vmaskmovpdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].StickyFlagZero ), XMM1 );
//	x->orpd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagZero ) );
//	x->movapd ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( StickyFlagZero ) );
}



// * vu instruction headers * //

void VuEncoder::HEADER ( long fs, long ft )
{
	// convert to double
	x->cvtps2pd ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].Vf [ fs ] ) );
	x->cvtps2pd ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].Vf [ ft ] ) );
	
	ClampXMM0AndXMM3 ();
}


void VuEncoder::HEADERi ( long fs )
{
	// convert to double
	x->cvtps2pd ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].Vf [ fs ] ) );

	// load in I register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].I ) );

	// convert to double
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
}


void VuEncoder::HEADERq ( long fs )
{
	// convert to double
	x->cvtps2pd ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].Vf [ fs ] ) );

	// load in Q register
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Q ) );

	// convert to double
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
}

void VuEncoder::HEADERbc ( long bc, long fs, long ft )
{
	// convert to double
	x->cvtps2pd ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].Vf [ fs ] ) );

	// broadcast bc
//	x->shufps ( XMM3, ft, ft, (char) (( bc << 6 ) + ( bc << 4 ) + ( bc << 2 ) + bc) );
	x->vbroadcastss ( XMM3, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].Vf [ ft ].l [ bc ] ) );

	// convert to double
	x->cvtps2pd ( XMM3, XMM3 );

	ClampXMM0AndXMM3 ();
}


// * vu instructions footers * //

bool VuEncoder::FOOTER ( long dest, long fd )
{
	SetVUFlagsForXMM0 ();

	// convert result to float
	x->cvtpd2ps ( XMM0, XMM0 );

	// looks like these bits need to be reversed
	dest = REVERSE4BITS( dest );

	// blend
	x->blendps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].Vf [ fd ] ), ( dest ^ 0xf ) );

	// store result to register
	return x->movaps_to_mem128 ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].Vf [ fd ] ) );
}

bool VuEncoder::FOOTERA ( long dest )
{
	SetVUFlagsForXMM0 ();
	
	// load accumulator (256-bits)
	x->movapdfrommem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].ACC ) );
	
	// looks like these bits need to be reversed
	dest = REVERSE4BITS( dest );

	// blend with accumulator
	x->blendpd ( XMM1, XMM1, XMM0, dest );

	// write to accumulator
	return x->movapdtomem ( XMM1, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].ACC ) );
}



// with 128-bit registers:
// inf + inf = inf
// -inf + inf = -ind
// -inf + -inf = -inf
// -inf + x = -inf
// inf + -inf = -ind
bool VuEncoder::ADD ( long dest, long fd, long fs, long ft )
{
	HEADER ( fs, ft );

	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );

	return FOOTER ( dest, fd );
}

bool VuEncoder::VADDi ( long dest, long fd, long fs )
{
	HEADERi ( fs );

	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	return FOOTER ( dest, fd );
}

bool VuEncoder::VADDq ( long dest, long fd, long fs )
{
	HEADERq ( fs );

	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	FOOTER ( dest, fd );
}

bool VuEncoder::VADDbc ( long dest, long bc, long fd, long fs, long ft )
{
	HEADERbc ( bc, fs, ft );

	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	FOOTER ( dest, fd );
}

bool VuEncoder::VADDA ( long dest, long fs, long ft )
{
	HEADER ( fs, ft );

	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	FOOTERA ( dest );
}

bool VuEncoder::VADDAi ( long dest, long fs )
{
	HEADERi ( fs );

	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	FOOTERA ( dest );
}

bool VuEncoder::VADDAq ( long dest, long fs )
{
	HEADERq ( fs );

	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	FOOTERA ( dest );
}

bool VuEncoder::VADDAbc ( long dest, long bc, long fs, long ft )
{
	HEADERbc ( bc, fs, ft );
	
	// perform op
	x->addpd ( XMM0, XMM0, XMM3 );
	
	FOOTERA ( dest );
}






bool VuEncoder::IADD ( long id, long is, long it )
{
	if ( id == 0 ) return true;
	
	x->MovRegFromMem16 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].Vi [ is ] ) );
	x->AddRegMem16 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].Vi [ it ] ) );
	return x->MovRegToMem16 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ VuNumber ].Vi [ id ] ) );
}







// cop2 (vu0) simple integer instructions
/*

bool VuEncoder::VIADDI ( long it, long is, long Imm5 )
{
	Format34 ( it, is, Imm5 );
	return x->LeaRegRegImm16 ( it, is, Imm5 );
}

bool VuEncoder::VIAND ( long id, long is, long it )
{
	Format33 ( id, is, it );
	
	if ( id == is ) return x->AndRegReg16 ( id, it );
	if ( id == it ) return x->AndRegReg16 ( id, is );
	if ( is == it ) return x->MovRegReg16 ( id, is );
	if ( id == is && id == it ) return true;
	
	x->MovRegReg16 ( id, is );
	return x->AndRegReg16 ( id, it );
}

bool VuEncoder::VILWR ( long dest, long it, long is )
{
	Format35FromMem( dest, it, is );
	
	// we need address * 16
	x->LeaRegRegReg16 ( RAX, is, is );
	
	return x->MovRegFromMem16 ( it, R8, RAX, SCALE_EIGHT, ((long long) &PS2SystemState.VU0Mem [ dest * 4 ] ) - ((long long) &PS2SystemState) );
}

bool VuEncoder::VIOR ( long id, long is, long it )
{
	Format33 ( id, is, it );
	
	if ( id == is ) return x->OrRegReg16 ( id, it );
	if ( id == it ) return x->OrRegReg16 ( id, is );
	if ( is == it ) return x->MovRegReg16 ( id, is );
	if ( id == is && id == it ) return true;

	x->MovRegReg16 ( id, is );
	return x->OrRegReg16 ( id, it );
}

bool VuEncoder::VISUB ( long id, long is, long it )
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

bool VuEncoder::VISWR ( long dest, long it, long is )
{
	Format35ToMem( dest, it, is );
	
	// we need address * 16
	x->LeaRegRegReg16 ( RAX, is, is );
	
	return x->MovRegToMem16 ( it, R8, RAX, SCALE_EIGHT, ((long long) &PS2SystemState.VU0Mem [ dest * 4 ] ) - ((long long) &PS2SystemState) );
}


// vu float instructions






bool VuEncoder::VADDi ( long dest, long fd, long fs )
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

bool VuEncoder::VADDq ( long dest, long fd, long fs )
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

bool VuEncoder::VADDbc ( long dest, long bc, long fd, long fs, long ft )
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

bool VuEncoder::VADDA ( long dest, long fs, long ft )
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

bool VuEncoder::VADDAi ( long dest, long fs )
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

bool VuEncoder::VADDAq ( long dest, long fs )
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

bool VuEncoder::VADDAbc ( long dest, long bc, long fs, long ft )
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

bool VuEncoder::VCLIP ( long fs, long ft )
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

bool VuEncoder::VDIV ( long ftf, long fsf, long fs, long ft )
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
bool VuEncoder::VFTOI0 ( long dest, long ft, long fs )
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

bool VuEncoder::VFTOI4 ( long dest, long ft, long fs )
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

bool VuEncoder::VFTOI12 ( long dest, long ft, long fs )
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

bool VuEncoder::VFTOI15 ( long dest, long ft, long fs )
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

bool VuEncoder::VITOF0 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );

	// convert 32-bit value into float
	x->cvtdq2ps ( XMM0, fs );

	// put into result
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool VuEncoder::VITOF4 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );

	// convert 32-bit value into float
	x->cvtdq2ps ( XMM0, fs );
	
	// divide by 16
	x->divps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst16 ) );

	// put into result
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool VuEncoder::VITOF12 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );

	// convert 32-bit value into float
	x->cvtdq2ps ( XMM0, fs );

	// divide by 4096
	x->divps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst4096 ) );

	// put into result
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool VuEncoder::VITOF15 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );

	// convert 32-bit value into float
	x->cvtdq2ps ( XMM0, fs );

	// divide by 32768
	x->divps ( XMM0, XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( FloatConst32768 ) );

	// put into result
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

void VuEncoder::SetMADDVUFlagsAndGetResult ( void )
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
bool VuEncoder::VMADD ( long dest, long fd, long fs, long ft )
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

bool VuEncoder::VMADDi ( long dest, long fd, long fs )
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

bool VuEncoder::VMADDq ( long dest, long fd, long fs )
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

bool VuEncoder::VMADDbc ( long dest, long bc, long fd, long fs, long ft )
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

bool VuEncoder::VMADDA ( long dest, long fs, long ft )
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

bool VuEncoder::VMADDAi ( long dest, long fs )
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

bool VuEncoder::VMADDAq ( long dest, long fs )
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

bool VuEncoder::VMADDAbc ( long dest, long bc, long fs, long ft )
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
bool VuEncoder::VMAX ( long dest, long fd, long fs, long ft )
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

bool VuEncoder::VMAXi ( long dest, long fd, long fs )
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

bool VuEncoder::VMAXbc ( long dest, long bc, long fd, long fs, long ft )
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

bool VuEncoder::VMFIR ( long dest, long ft, long is )
{
	Format42 ( dest, ft, is );
	x->MovsxReg32Reg16 ( EAX, is );
	x->pinsrd ( XMM0, XMM0, EAX, 0 );
	x->shufps ( XMM0, XMM0, XMM0, 0 );
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool VuEncoder::VMINI ( long dest, long fd, long fs, long ft )
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

bool VuEncoder::VMINIi ( long dest, long fd, long fs )
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

bool VuEncoder::VMINIbc ( long dest, long bc, long fd, long fs, long ft )
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

bool VuEncoder::VMOVE ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );
	return x->blendps ( ft, ft, fs, (char) dest );
}

bool VuEncoder::VMR32 ( long dest, long ft, long fs )
{
	Format26 ( dest, ft, fs );
	x->shufps ( XMM0, fs, fs, ( 2 << 6 ) + ( 1 << 4 ) + ( 0 << 2 ) + ( 3 ) );
	return x->blendps ( ft, ft, XMM0, (char) dest );
}

bool VuEncoder::VMSUB ( long dest, long fd, long fs, long ft )
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

bool VuEncoder::VMSUBi ( long dest, long fd, long fs )
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

bool VuEncoder::VMSUBq ( long dest, long fd, long fs )
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

bool VuEncoder::VMSUBbc ( long dest, long bc, long fd, long fs, long ft )
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

bool VuEncoder::VMSUBA ( long dest, long fs, long ft )
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

bool VuEncoder::VMSUBAi ( long dest, long fs )
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

bool VuEncoder::VMSUBAq ( long dest, long fs )
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

bool VuEncoder::VMSUBAbc ( long dest, long bc, long fs, long ft )
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

bool VuEncoder::VMUL ( long dest, long fd, long fs, long ft )
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

bool VuEncoder::VMULi ( long dest, long fd, long fs )
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

bool VuEncoder::VMULq ( long dest, long fd, long fs )
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

bool VuEncoder::VMULbc ( long dest, long bc, long fd, long fs, long ft )
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


bool VuEncoder::VMULA ( long dest, long fs, long ft )
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

bool VuEncoder::VMULAi ( long dest, long fs )
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

bool VuEncoder::VMULAq ( long dest, long fs )
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

bool VuEncoder::VMULAbc ( long dest, long bc, long fs, long ft )
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


bool VuEncoder::VNOP ( void )
{
	return true;
}

bool VuEncoder::VOPMULA ( long dest, long fs, long ft )
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

bool VuEncoder::VMTIR ( long fsf, long it, long fs )
{
	Format41 ( fsf, it, fs );
	return x->pextrw ( it, XMM0, ( fsf << 1 ) );
}

bool VuEncoder::VRGET ( long dest, long ft )
{
	Format39 ( dest, ft );

	// broadcast R
	x->vbroadcastss ( XMM0, R8, NO_INDEX, SCALE_NONE, GetOffsetForPS2Data ( Vu [ 0 ].R ) );

	// store to ft
	x->blendps ( ft, ft, XMM0, (char) dest );
}

bool VuEncoder::VRINIT ( long fsf, long fs )
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

bool VuEncoder::VRNEXT ( long dest, long ft )
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

bool VuEncoder::VRSQRT ( long ftf, long fsf, long fs, long ft )
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

bool VuEncoder::VRXOR ( long fsf, long fs )
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

bool VuEncoder::VSQRT ( long ftf, long ft )
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
bool VuEncoder::VSUB ( long dest, long fd, long fs, long ft )
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

bool VuEncoder::VSUBi ( long dest, long fd, long fs )
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

bool VuEncoder::VSUBq ( long dest, long fd, long fs )
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

bool VuEncoder::VSUBbc ( long dest, long bc, long fd, long fs, long ft )
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

bool VuEncoder::VSUBA ( long dest, long fs, long ft )
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

bool VuEncoder::VSUBAi ( long dest, long fs )
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

bool VuEncoder::VSUBAq ( long dest, long fs )
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

bool VuEncoder::VSUBAbc ( long dest, long bc, long fs, long ft )
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

bool VuEncoder::VWAITQ ( void )
{
	return true;
}
*/



