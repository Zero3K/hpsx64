


#include <iostream>
#include <sstream>
#include <iomanip>

#include "R5900DebugPrint.h"

#include "WinApiHandler.h"

#include "Playstation2.h"
#include "R5900.h"
#include "R5900Decoder.h"

#include "typedefs.h"


#include "x64Encoder.h"

#include "QueryR5900.h"

using namespace std;




R5900::Cpu::Cpu ( void )
{
	// not yet debugging
	debug_enabled = false;
}

R5900::Cpu::~Cpu ( void )
{
	if ( debug_enabled ) delete DebugWindow;
}

bool R5900::Cpu::Create ( void )
{
	// create encoder to encode x64 instructions
	x = new x64Encoder ( 1024, 1024 );
	
	// create R3000A encoder to encode Mips instructions into x64 instructions using the x64Encoder
	r = new R5900Encoder ( x );

	// reset processor
	Reset ();

	return true;
}

void R5900::Cpu::Reset ( void )
{
	unsigned long i;

	// zero out all registers
	for ( i = 0; i < 32; i++ )
	{
		GPR [ i ].Reg64 [ 0 ] = 0;
		GPR [ i ].Reg64 [ 1 ] = 0;

		FPR [ i ].l = 0;
		FCR [ i ] = 0;

		CPR0.Regs [ i ] = 0;
		CPR2 [ i ] = 0;
	}

	Hi.Reg64 [ 0 ] = 0;
	Hi.Reg64 [ 1 ] = 0;
	Lo.Reg64 [ 0 ] = 0;
	Lo.Reg64 [ 1 ] = 0;
	
	ACC.l = 0;
	
	SA = 0;
	
	// set initial values for CPR0 registers
	CPR0.Config.DC = 1;
	CPR0.Config.IC = 2;
	CPR0.PRId.Imp = 0x2e;
	CPR0.Status.BEV = 1;

	// this is the start address for the program counter when a playstation is reset
	PC = 0xbfc00000;

	CycleCount = 0;
	CycleTarget = 0;
	
	// ** initialize the address translation tables ** //

	
	for ( i = 0; i < 0x100000; i++ )
	{
		// 0x00000000 - main ram physical address start - cached : 32 MB
		// 0x20000000 - main ram start - uncached
		// 0x30000000 - main ram start - uncached accelerated
		// 0xa0000000 - main ram start mirror
		// 0x80000000 - main ram start mirror
		if ( ( i >= 0x00000 && i <=0x01fff ) || ( i >= 0x20000 && i <=0x21fff ) || ( i >= 0x30000 && i <=0x31fff ) || ( i >= 0x80000 && i <=0x81fff ) || ( i >= 0xa0000 && i <=0xa1fff ) )
		{
			TranslationAddressTable [ i ] = (unsigned long long) &PS2SystemState.MainMemory;
			TranslationMaskTable [ i ] = (unsigned long) 0x01ffffff;
		}

		// scratch pad is from 0x70000000 - 0x70003fff : 16KB
		else if ( i == 0x70003 )
		{
			TranslationAddressTable [ i ] = (unsigned long long) &PS2SystemState.ScratchPad;
			TranslationMaskTable [ i ] = (unsigned long) 0x00003fff;
		}
		
		// bios is from:
		// 0x1fc00000 - 0x1fffffff
		// 0x9fc00000 - 0x9fffffff
		// 0xbfc00000 - 0xbfffffff
		else if ( ( i >= 0x1fc00 && i <=0x1ffff ) || ( i >= 0x9fc00 && i <=0x9ffff ) || ( i >= 0xbfc00 && i <=0xbffff ) )
		{
			TranslationAddressTable [ i ] = (unsigned long long) &PS2SystemState.Bios;
			TranslationMaskTable [ i ] = (unsigned long) 0x003fffff;
		}

		// otherwise we need to make it zero so that we know that the area is not mapped
		else
		{
			TranslationAddressTable [ i ] = 0;
			TranslationMaskTable [ i ] = 0;
		}

	}

	Stop = 0;
}

// time sensitive function - call to execute cycles on processor until we have executed NumberOfCycles
void R5900::Cpu::Run ( long NumberOfCycles )
{
	// to increase speed, the recompiler will handle updating program counter and cycle counts
	
	char input [ 256 ];
	
	cout << "R5900::Execute\n";

	// we need to reach current cycle + NumberOfCycles
	CycleTarget = CycleCount + NumberOfCycles;
	
	cout << "R5900::CycleTarget = " << CycleTarget << "\n";
	cout << "R5900::CycleCount = " << CycleCount << "\n";
	
//	cin >> input;
	
	while ( CycleCount < CycleTarget )
	{

		cout << "In while loop: R5900::CycleTarget = " << CycleTarget << "\n";
		cout << "In while loop: R5900::CycleCount = " << CycleCount << "\n";
		
//		cin >> input;

		// check if next instruction to be executed is already decoded
		if ( isDecoded () )
		{
			cout << "R5900::Instruction is already decoded. Executing instruction\n";
			
			// if so, then execute it and any preceding ones until we reach cycle target
			Execute ();
		}
		else
		{
			cout << "R5900::Instruction is not decoded yet. Decoding instruction\n";
			
			// otherwise we need to decode the instructions before we execute them
			DecodeCycles ( NumberOfCycles );
			Execute ();
		}
		
	}
}


void R5900::Cpu::Step ( long NumberOfInstructions )
{
	char input [ 256 ];
	
	long InstructionsExecuted;
	
	cout << "R5900::Execute\n";

	// we need to reach current cycle + NumberOfCycles
//	CycleTarget = CycleCount + NumberOfCycles;

	// init instructions executed
	InstructionsExecuted = 0;
	
	cout << "R5900::CycleTarget = " << CycleTarget << "\n";
	cout << "R5900::CycleCount = " << CycleCount << "\n";
	
//	cin >> input;
	
	while ( InstructionsExecuted < NumberOfInstructions )
	{

		cout << "In while loop: R5900::CycleTarget = " << CycleTarget << "\n";
		cout << "In while loop: R5900::CycleCount = " << CycleCount << "\n";
		
//		cin >> input;

		// check if next instruction to be executed is already decoded
		if ( isDecoded () )
		{
			cout << "R5900::Instruction is already decoded. Executing instruction\n";
			
			// if so, then execute it and any preceding ones until we reach cycle target
			Execute ();
		}
		else
		{
			cout << "R5900::Instruction is not decoded yet. Decoding instruction\n";
			
			// otherwise we need to decode the instructions before we execute them
			DecodeInstruction ();
			Execute ();
		}
		
		InstructionsExecuted++;
		
	}
}




void R5900::Cpu::EnableDebugging ( void )
{
	// have to make sure debug window is not already created
	if ( !debug_enabled )
	{
		// create the window
		DebugWindow = new WindowClass::Window ();
		DebugWindow->Create ( "R5900 Debug Window", 10, 10, 768, 512 );
		DebugWindow->BeginDrawing ();
		DebugWindow->ChangeTextColor ( 0x00ffffff );
		DebugWindow->ChangeTextBkColor ( 0x0000ffff );
		DebugWindow->ChangeBkMode ( TRANSPARENT );
		DebugWindow->ChangeFont ( 8 );
		DebugWindow->EndDrawing ();
		
		debug_enabled = true;
	}
}

void R5900::Cpu::UpdateDebugWindow ( void )
{
	const char* COP0_Names [] = { "Index", "Random", "EntryLo0", "EntryLo1", "Context", "PageMask", "Wired", "(Reserved)",
								"BadVAddr", "Count", "EntryHi", "Compare", "Status", "Cause", "EPC", "PRId",
								"Config", "(Reserved)", "(Reserved)", "(Reserved)", "(Reserved)", "(Reserved)", "(Reserved)", "BadPAddr",
								"Debug", "Perf", "(Reserved)", "(Reserved)", "TagLo", "TagHi", "ErrorEPC", "(Reserved)" };

	const long x_start = 0;
	const long x_spacing = 45;
	const long start_line = 0;
	long x, count, row, instruction, rowsave;

	long TempPC;

	long* MemoryDeviceBase;
	long MemoryDeviceMask;

	long Address;

	stringstream ss;

	if ( debug_enabled )
	{
		// start on row 0
		row = start_line;
		
		// start drawing
		DebugWindow->BeginDrawing ();
		
		// clear window
		DebugWindow->Clear ();
		
		// Display Heading
		DebugWindow->PrintText ( 0, row++, "R5900 Status" );
		DebugWindow->PrintText ( 0, row++, "--------------------------------------" );
		
		rowsave = row;

		// Display GPRs Heading 1
		for ( x = x_start, count = 0; count < 16; count++, x+=x_spacing )
		{
			row = rowsave;
			
			// show first row of GPR headings
			ss << "R" << count;
			DebugWindow->PrintText ( x, row++, (char*) ss.str().c_str() );
			ss.str ("");
			
			// show first row of GPR status
			ss << hex << GPR [ count ].Reg32 [ 0 ] << dec;
			DebugWindow->PrintText ( x, row++, (char*) ss.str().c_str() );
			ss.str ("");
			ss << hex << GPR [ count ].Reg32 [ 1 ] << dec;
			DebugWindow->PrintText ( x, row++, (char*) ss.str().c_str() );
			ss.str ("");
			ss << hex << GPR [ count ].Reg32 [ 2 ] << dec;
			DebugWindow->PrintText ( x, row++, (char*) ss.str().c_str() );
			ss.str ("");
			ss << hex << GPR [ count ].Reg32 [ 3 ] << dec;
			DebugWindow->PrintText ( x, row++, (char*) ss.str().c_str() );
			ss.str ("");
			
			// add an additional row
			row++;

			// show second row of GPR headings
			ss << "R" << count + 16;
			DebugWindow->PrintText ( x, row++, (char*) ss.str().c_str() );
			ss.str ("");

			// show second row of GPR status
			ss << hex << GPR [ count + 16 ].Reg32 [ 0 ] << dec;
			DebugWindow->PrintText ( x, row++, (char*) ss.str().c_str() );
			ss.str ("");
			ss << hex << GPR [ count + 16 ].Reg32 [ 1 ] << dec;
			DebugWindow->PrintText ( x, row++, (char*) ss.str().c_str() );
			ss.str ("");
			ss << hex << GPR [ count + 16 ].Reg32 [ 2 ] << dec;
			DebugWindow->PrintText ( x, row++, (char*) ss.str().c_str() );
			ss.str ("");
			ss << hex << GPR [ count + 16 ].Reg32 [ 3 ] << dec;
			DebugWindow->PrintText ( x, row++, (char*) ss.str().c_str() );
			ss.str ("");
			
			// add an additional row
			row++;
			
			// show first row of COP0 headings
			DebugWindow->PrintText ( x, row++, (char*) COP0_Names [ count ] );
			// show COP data
			ss << hex << CPR0.Regs [ count ] << dec;
			DebugWindow->PrintText ( x, row++, (char*) ss.str().c_str() );
			ss.str ("");

			// show second row of COP0 headings
			DebugWindow->PrintText ( x, row++, (char*) COP0_Names [ count + 16 ] );
			
			ss << hex << CPR0.Regs [ count + 16 ] << dec;
			DebugWindow->PrintText ( x, row++, (char*) ss.str().c_str() );
			ss.str ("");

		}
		
		x = x_start;
		
		// show pc
		ss << "PC";
		DebugWindow->PrintText ( x, row, (char*) ss.str().c_str() );
		ss.str ("");
		ss << hex << PC << dec;
		DebugWindow->PrintText ( x, row + 1, (char*) ss.str().c_str() );
		ss.str ("");
		
		x += x_spacing;
		
		// show next pc
		ss << "NextPC";
		DebugWindow->PrintText ( x, row, (char*) ss.str().c_str() );
		ss.str ("");
		ss << hex << NextPC << dec;
		DebugWindow->PrintText ( x, row + 1, (char*) ss.str().c_str() );
		ss.str ("");
		
		x += x_spacing;
		
		// show isBranchTaken
		ss << "isBranchTaken";
		DebugWindow->PrintText ( x, row, (char*) ss.str().c_str() );
		ss.str ("");
		ss << hex << isBranchTaken << dec;
		DebugWindow->PrintText ( x, row + 1, (char*) ss.str().c_str() );
		ss.str ("");

		x += x_spacing;
		
		// show BranchTest
		ss << "BranchTest";
		DebugWindow->PrintText ( x, row, (char*) ss.str().c_str() );
		ss.str ("");
		ss << hex << BranchTest << dec;
		DebugWindow->PrintText ( x, row + 1, (char*) ss.str().c_str() );
		ss.str ("");

		x += x_spacing;

		// show CycleCount
		ss << "CycleCount";
		DebugWindow->PrintText ( x, row, (char*) ss.str().c_str() );
		ss.str ("");
		ss << hex << CycleCount << dec;
		DebugWindow->PrintText ( x, row + 1, (char*) ss.str().c_str() );
		ss.str ("");

		row += 2;
		
		// get program counter minus 5 instructions
		TempPC = PC - 20;
		row++;
		
		for ( count = 0; count < 10; count++, TempPC += 4 )
		{
			// get base pointer of where we will read instructions from
			MemoryDeviceBase = (long*) (PS2MemoryMapLUT [ ( TempPC >> 22 ) & 0x3ff ]);
			
			if ( MemoryDeviceBase != NULL )
			{
			
				// get mask for memory device
				MemoryDeviceMask = PS2MemoryMapMask [ ( TempPC >> 22 ) & 0x3ff ];
				
				// get pc address after mask - we don't want to read memory from out side of memory device area
				Address = ( TempPC & MemoryDeviceMask ) | ( TempPC & 0xffc00000 );
				
				// check if TempPC is equal to the PC
				if ( TempPC == PC )
				{
					// if so, add current line indicator into string
					ss << "> ";
				}
				else
				{
					// otherwise, add blank space
					ss << "  ";
				}
				
				// display
				DebugWindow->PrintText ( 10, row, (char*) ss.str().c_str() );
				
				// clear the string
				ss.str ("");
				
				// add the address into string
				ss << hex << setw ( 8 ) << Address << "   " << dec;
				
				// get the instruction at that memory location
				instruction = MemoryDeviceBase [ ( TempPC >> 2 ) & ( MemoryDeviceMask >> 2 ) ];
				
				// put the mnemonic into string
				R5900DebugPrint::PrintInstruction ( ss, instruction );
				
				// display
				DebugWindow->PrintText ( 20, row, (char*) ss.str().c_str() );
				
				// clear the string
				ss.str ("");

				// next address should go on next row
				row++;
			}
		}


		// done drawing
		DebugWindow->EndDrawing ();
	}
}


// determines if the next instruction has been decoded already
bool R5900::Cpu::isDecoded ( void )
{
	long ShiftedPC = ( PC >> 2 );
	return ( ( x->x64CodeHashTable [ ShiftedPC & ( x->lNumberOfCodeBlocks - 1 ) ] ) == ShiftedPC );	// must test with shifted pc
}

void R5900::Cpu::InstructionFooter ( long NumberOfCyclesDecoded, long instruction, long PreviousInstruction )
{
/*
		// check if last instruction decoded is a trap
		// if so, then somehow we need to allow for changing program counter for trap
		// Note: if trap is in branch delay slot, then we need to test branch when it returns, so we also need to save branch status
		if ( R5900::Query::isTrap ( instruction ) )
		{
			// trap is like a branch without delay slot, so no need to check if trap should be taken
			// just process exception, return, and set branch target unless it is an unconditional trap

			// check if we are to take trap and reset branch taken flag
//			x->BtrMemImm32 ( 0, R8, NO_INDEX, SCALE_NONE, GetOffset ( isTrapTaken ) );

			// if we are not to take trap, then jump to next instruction
//			x->Jmp8_B ( 0, 0 );
			
			// Level 1 exception procedure

			// first you must mask out previous value in Cause.ExcCode
			x->AndMemImm32 ( ~( 0x1f << 2 ), R8, NO_INDEX, SCALE_NONE, GetOffset ( CPR0.Cause.l ) );
			
			// store exception cause codes (set Cause.ExcCode, etc.)
			// Cause.ExcCode = cause = Trap
				// Cause.ExcCode for an system call trap is 8
				// Cause.ExcCode for a "reserved instruction" trap is 10
			// **** TODO **** ADD IN SYSTEMCALL TRAP AND RESERVED INSTRUCTION TRAP 
			if ( R5900::Query::isOverflowTrap ( instruction ) )
			{
				// Cause.ExcCode for an overflow trap is 12
				
				// then you must set new value
				x->OrMemImm32 ( ( 12 << 2 ), R8, NO_INDEX, SCALE_NONE, GetOffset ( CPR0.Cause.l ) );
			}
			else
			{
				// Cause.ExcCode for a regular Trap instruction is 13
				
				// then you must set new value
				x->OrMemImm32 ( ( 13 << 2 ), R8, NO_INDEX, SCALE_NONE, GetOffset ( CPR0.Cause.l ) );
			}


			// Save program counter and branch delay state unless already in an exception handler (if already in handler common vector is used for TLBRefill)
			// (save the current PC into EPC, unless we are in branch delay slot, then save PC-4 into EPC)
			// (if we are in branch delay slot, then set Cause.BD to 1, otherwise clear it to zero)
			x->MovRegFromMem32 ( RCX, R8, NO_INDEX, SCALE_NONE, GetOffset ( isInBranchDelaySlot ) );
			x->MovRegFromMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffset ( PC ) );
			x->ShlRegImm32 ( RCX, 2 );
			x->SubRegReg32 ( RAX, RDX );
			x->MovRegToMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffset ( CPR0.EPC ) );
			x->ShlRegImm32 ( RCX, 29 );
			x->AndMemReg32 ( ~( 1 << 31 ), R8, NO_INDEX, SCALE_NONE, GetOffset ( CPR0.Cause.l ) );
			x->OrMemReg32 ( RCX, R8, NO_INDEX, SCALE_NONE, GetOffset ( CPR0.Cause.l ) );
			
			// **** TODO **** save branch delay state for processor and clear it
			
			// **** TODO **** check if we need to disable interrupts or if handler handles that
			
			// switch into kernel mode (set Status.EXL to 1)
			x->BtsMemImm32 ( 1, R8, NO_INDEX, SCALE_NONE, GetOffset ( CPR0.Status.l ) );
			
			// if BEV = 0, then interrupt to 0x80000180, otherwise add 0x30000200 and interrupt to 0xbfc00380
			x->MovRegImm32 ( RAX, 0x80000180 );
			x->MovRegImm32 ( RCX, 0xbfc00380 );
			x->BtMemImm32 ( 22, R8, NO_INDEX, SCALE_NONE, GetOffset ( CPR0.Status.l ) );
			x->CmovBRegReg32 ( RAX, RCX );
			x->MovRegToMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffset ( PC ) );

			// update cycle count and add in half cycles used by taken trap (instruction pipeline refill)
			x->AddMemImm32 ( NumberOfCyclesDecoded + BranchHalfCycles, R8, NO_INDEX, SCALE_NONE, GetOffset ( CycleCount ) );
			
			// return from block of code
			x->Ret ();
			
			// Set target for branch and then we'll continue with decoding instructions into block
			// only do this if trap is conditional
			if ( !R5900::Query::isTrapAlways ( instruction ) ) x->SetJmpTarget8 ( 0 );
		}
*/		

		// check if current instruction is a Branch Likely
		if ( R5900::Query::isBranchLikely ( instruction ) )
		{
			// if branch is not taken, we need to exit code block

			// check if we are to take branch but do not reset branch taken flag
			x->BtMemImm32 ( 0, R8, NO_INDEX, SCALE_NONE, GetOffset ( isBranchTaken ) );

			// if we are to take branch, then jump to next instruction
			x->Jmp8_B ( 0, 0 );

			// branch is not taken, so put in next value of program counter for after branch delay slot
			// DecodePC already has value for branch delay slot, so just add 4
			x->MovMemImm32 ( DecodePC + InstructionSize, R8, NO_INDEX, SCALE_NONE, GetOffset ( PC ) );

			// update cycle count and add in half cycles used by taken branch (instruction pipeline refill)
			x->AddMemImm32 ( NumberOfCyclesDecoded + BranchHalfCycles, R8, NO_INDEX, SCALE_NONE, GetOffset ( CycleCount ) );
			
			// return from block of code
			x->Ret ();
			
			// Set target for branch and then we'll continue with decoding instructions into block
			x->SetJmpTarget8 ( 0 );
			
		}

		// check if previous instruction is a Branch Likely
		if ( R5900::Query::isBranchLikely ( PreviousInstruction ) )
		{
			// check if branch was taken and clear flag
			x->BtrMemImm32 ( 0, R8, NO_INDEX, SCALE_NONE, GetOffset ( isBranchTaken ) );
			
			// if branch was not taken then jump to next instruction
			x->Jmp8_AE ( 0, 0 );

			// branch is taken, so put in next value of program counter after branch
			x->MovRegFromMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffset ( NextPC ) );
			x->MovRegToMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffset ( PC ) );

			// update cycle count and add in half cycles used by taken branch (instruction pipeline refill)
			x->AddMemImm32 ( NumberOfCyclesDecoded + BranchHalfCycles, R8, NO_INDEX, SCALE_NONE, GetOffset ( CycleCount ) );
			
			// return from block of code
			x->Ret ();
			
			// Set target for branch and then we'll continue with decoding instructions into block
			x->SetJmpTarget8 ( 0 );
		}


		// check if previous instruction was a branch
		// even if current instruction is a trap, this should check if processor is in branch delay slot in case trap is not taken
		if ( R5900::Query::isBranch ( PreviousInstruction ) && !R5900::Query::isBranchLikely ( PreviousInstruction ) )
		{
			// previous instruction is a branch, so processor should check if it is in a branch delay slot
			// if in a branch delay slot, then processor should check if it should jump or not
			
			// even if it is an unconditional branch, we still end up with the same instruction sequence
			// because it's possible that previous instruction was not executed since they could have just jumped into code anywhere
			
			// ** For both conditional and unconditional branches - Even if current instruction is a TRAP instruction ** //

			// check if we are to take branch and reset branch taken flag
			x->BtrMemImm32 ( 0, R8, NO_INDEX, SCALE_NONE, GetOffset ( isBranchTaken ) );

			// if we are not to take branch, then jump to next instruction
			x->Jmp8_AE ( 0, 0 );

			// this line is just for testing
			//x->MovMemImm32 ( 5, R8, NO_INDEX, SCALE_NONE, GetOffset ( BranchTest ) );
			
			// branch is taken, so put in next value of program counter after branch
			x->MovRegFromMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffset ( NextPC ) );
			x->MovRegToMem32 ( RAX, R8, NO_INDEX, SCALE_NONE, GetOffset ( PC ) );

			// update cycle count and add in half cycles used by taken branch (instruction pipeline refill)
			x->AddMemImm32 ( NumberOfCyclesDecoded + BranchHalfCycles, R8, NO_INDEX, SCALE_NONE, GetOffset ( CycleCount ) );
			
			// return from block of code
			x->Ret ();
			
			// Set target for branch and then we'll continue with decoding instructions into block
			x->SetJmpTarget8 ( 0 );
		}
}

void R5900::Cpu::CodeBlockFooter ( long NumberOfCyclesDecoded )
{
	// update cycle count
	x->AddMemImm32 ( NumberOfCyclesDecoded, R8, NO_INDEX, SCALE_NONE, GetOffset ( CycleCount ) );

	// update program counter
	x->MovMemImm32 ( DecodePC, R8, NO_INDEX, SCALE_NONE, GetOffset ( PC ) );

	// return from dynamic block of code
	x->Ret ();
}

// decodes instructions until either it runs out of memory, it reaches an abosolute branch, or a certain number of cycles have been reached
long R5900::Cpu::DecodeCycles ( long NumberOfCyclesToDecode )
{
	long NumberOfCyclesDecoded, LastCyclesDecoded;
	long Offset = 0;
	
	long instruction;
	
	long* MemoryDeviceBase;	// like whether it's bios, main memory, etc. that instructions are being executed from. need base addr for mem device
	long MemoryDeviceMask;
	
	long PreviousInstruction;
	
	// we are just decoding instructions, not executing, so we need temporary program counter
	DecodePC = PC;
	
	cout << "R5900::DecodeInstructions DecodePC = " << DecodePC << "\n";

	// get base pointer of where we will read instructions from
//	MemoryDeviceBase = (long*) (PS2MemoryMapLUT [ ( DecodePC >> 22 ) & 0x3ff ]);
	MemoryDeviceBase = (long*) (TranslationAddressTable [ ( DecodePC >> 12 ) & 0xfffff ]);
	
	// get mask for memory device
//	MemoryDeviceMask = PS2MemoryMapMask [ ( DecodePC >> 22 ) & 0x3ff ];
	MemoryDeviceMask = (long) TranslationMaskTable [ ( DecodePC >> 12 ) & 0xfffff ];

	// start dynamic code block
	x->x64StartCodeBlock ( DecodePC >> 2 );

	// * add header - load R8 with data pointer *
	x->MovRegImm64 ( R8, (long long) &PS2SystemState );
	
	// init number of cycles decoded
	NumberOfCyclesDecoded = 0;
	
	// have to init last cycles decoded to something greater than zero
	LastCyclesDecoded = 1;

	// decode/encode instructions into dynamic code block until we need to stop
	while ( ( NumberOfCyclesDecoded < NumberOfCyclesToDecode ) && ( LastCyclesDecoded > 0 ) )
	{
		cout << "in loop: R5900::DecodeInstructions NumberOfCyclesDecoded = " << NumberOfCyclesDecoded << "\n";
		cout << "in loop: R5900::DecodeInstructions NumberOfCyclesToDecode = " << NumberOfCyclesToDecode << "\n";
		cout << "in loop: R5900::DecodeInstructions DecodePC = " << DecodePC << "\n";
		cout << "in loop: R5900::DecodeInstructions LastCyclesDecoded = " << LastCyclesDecoded << "\n";
		cout << "in loop: x64Encoder::x64NextOffset = " << x->x64NextOffset << "\n";
	
		// load instrution from memory/bios/rom and decode/encode
		instruction = MemoryDeviceBase [ ( DecodePC >> 2 ) & ( MemoryDeviceMask >> 2 ) ];
		
		// load previous instruction
		PreviousInstruction = MemoryDeviceBase [ ( ( DecodePC - InstructionSize ) >> 2 ) & ( MemoryDeviceMask >> 2 ) ];
		
		cout << "R5900::DecodeInstructions Decoding instruction\n";
		
		// decode/encode instruction
		LastCyclesDecoded = R5900Decoder::DecodeInstruction ( instruction, r );

		cout << "R5900::DecodeInstructions Updating PC\n";

		// update number of cycles decoded
		NumberOfCyclesDecoded += LastCyclesDecoded;

		// update temp program counter to next instruction
		DecodePC += InstructionSize;

		cout << "R5900::DecodeInstructions Updating cycles decoded\n";

		
		// add instruction footer - depends on instruction
		InstructionFooter ( NumberOfCyclesDecoded, instruction, PreviousInstruction );
		



		// if instruction is an unconditional trap, then we can also stop decoding
		// can't do this for unconditional branch because of the branch delay slot
		if ( R5900::Query::isTrapAlways ( instruction ) ) break;

		// if instruction was invalid then stop decoding
		if ( R5900::Query::isInvalid ( instruction ) ) break;
		
	}


	// * add footer - update cycles, update program counter, return *
	CodeBlockFooter ( NumberOfCyclesDecoded );

	cout << "R5900::DecodeInstructions NumberOfCyclesDecoded = " << NumberOfCyclesDecoded << "\n";
	cout << "R5900::DecodeInstructions NumberOfCyclesToDecode = " << NumberOfCyclesToDecode << "\n";
	cout << "R5900::DecodeInstructions DecodePC = " << DecodePC << "\n";
	cout << "R5900::DecodeInstructions LastCyclesDecoded = " << LastCyclesDecoded << "\n";
	cout << "x64Encoder::x64NextOffset = " << x->x64NextOffset << "\n";

	// done encoding in code block
	x->x64EndCodeBlock ();

	return NumberOfCyclesDecoded;
}

long R5900::Cpu::DecodeInstruction ( void )
{
	long NumberOfCyclesDecoded, LastCyclesDecoded;
	long Offset = 0;
	
	long instruction;
	
	long* MemoryDeviceBase;	// like whether it's bios, main memory, etc. that instructions are being executed from. need base addr for mem device
	long MemoryDeviceMask;
	
	long PreviousInstruction;
	
	// we are just decoding instructions, not executing, so we need temporary program counter
	DecodePC = PC;
	
	cout << "R5900::DecodeInstructions DecodePC = " << DecodePC << "\n";

	// get base pointer of where we will read instructions from
//	MemoryDeviceBase = (long*) (PS2MemoryMapLUT [ ( DecodePC >> 22 ) & 0x3ff ]);
	MemoryDeviceBase = (long*) (TranslationAddressTable [ ( DecodePC >> 12 ) & 0xfffff ]);
	
	// get mask for memory device
//	MemoryDeviceMask = PS2MemoryMapMask [ ( DecodePC >> 22 ) & 0x3ff ];
	MemoryDeviceMask = (long) TranslationMaskTable [ ( DecodePC >> 12 ) & 0xfffff ];

	// start dynamic code block
	x->x64StartCodeBlock ( DecodePC >> 2 );

	// * add header - load R8 with data pointer *
	x->MovRegImm64 ( R8, (long long) &PS2SystemState );
	
	// init number of cycles decoded
	NumberOfCyclesDecoded = 0;
	
	// have to init last cycles decoded to something greater than zero
	LastCyclesDecoded = 1;

	// decode/encode instructions into dynamic code block until we need to stop
//	while ( ( NumberOfCyclesDecoded < NumberOfCyclesToDecode ) && ( LastCyclesDecoded > 0 ) )
//	{
		cout << "in loop: R5900::DecodeInstructions NumberOfCyclesDecoded = " << NumberOfCyclesDecoded << "\n";
//		cout << "in loop: R5900::DecodeInstructions NumberOfCyclesToDecode = " << NumberOfCyclesToDecode << "\n";
		cout << "in loop: R5900::DecodeInstructions DecodePC = " << DecodePC << "\n";
		cout << "in loop: R5900::DecodeInstructions LastCyclesDecoded = " << LastCyclesDecoded << "\n";
		cout << "in loop: x64Encoder::x64NextOffset = " << x->x64NextOffset << "\n";
	
		// load instrution from memory/bios/rom and decode/encode
		instruction = MemoryDeviceBase [ ( DecodePC >> 2 ) & ( MemoryDeviceMask >> 2 ) ];
		
		// load previous instruction
		PreviousInstruction = MemoryDeviceBase [ ( ( DecodePC - InstructionSize ) >> 2 ) & ( MemoryDeviceMask >> 2 ) ];
		
		cout << "R5900::DecodeInstructions Decoding instruction\n";
		
		// decode/encode instruction
		LastCyclesDecoded = R5900Decoder::DecodeInstruction ( instruction, r );

		cout << "R5900::DecodeInstructions Updating PC\n";

		// update number of cycles decoded
		NumberOfCyclesDecoded += LastCyclesDecoded;

		// update temp program counter to next instruction
		DecodePC += InstructionSize;

		cout << "R5900::DecodeInstructions Updating cycles decoded\n";

		
		// add instruction footer - depends on instruction
		InstructionFooter ( NumberOfCyclesDecoded, instruction, PreviousInstruction );



	// * add footer - update cycles, update program counter, return *
	CodeBlockFooter ( NumberOfCyclesDecoded );

	cout << "R5900::DecodeInstructions NumberOfCyclesDecoded = " << NumberOfCyclesDecoded << "\n";
//	cout << "R5900::DecodeInstructions NumberOfCyclesToDecode = " << NumberOfCyclesToDecode << "\n";
	cout << "R5900::DecodeInstructions DecodePC = " << DecodePC << "\n";
	cout << "R5900::DecodeInstructions LastCyclesDecoded = " << LastCyclesDecoded << "\n";
	cout << "x64Encoder::x64NextOffset = " << x->x64NextOffset << "\n";

	// done encoding in code block
	x->x64EndCodeBlock ();

	return NumberOfCyclesDecoded;
}


// executes block of decoded instructions that start at PC - assumes we have checked to make sure they are decoded
void R5900::Cpu::Execute ( void )
{
	x->x64ExecuteCodeBlock ( ( PC >> 2 ) & ( x->lNumberOfCodeBlocks - 1 ) );
}



char* R5900::Cpu::PS2MemoryMapLUT [ 1024 ] = { V, V, V, V, V, V, V, V, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x000-0x07c; 000000000000-000001111100
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x080-0x0fc; 000010000000-000011111100
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x100-0x17c; 000100000000-000101111100
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,	//0x180-0x1fc; 000110000000-000111111100
								V, V, V, V, V, V, V, V, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x200-0x27c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x280-0x2fc
								V, V, V, V, V, V, V, V, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x300-0x37c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x380-0x3fc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x400-0x47c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x480-0x4fc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x500-0x57c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x580-0x5fc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x600-0x67c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x680-0x6fc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x700-0x77c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x780-0x7fc
								V, V, V, V, V, V, V, V, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x800-0x87c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x880-0x8fc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x900-0x97c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,	//0x980-0x9fc
								V, V, V, V, V, V, V, V, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xa00-0xa7c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xa80-0xafc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xb00-0xb7c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W,	//0xb80-0xbfc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xc00-0xc7c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xc80-0xcfc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xd00-0xd7c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xd80-0xdfc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xe00-0xe7c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xe80-0xefc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xf00-0xf7c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	//0xf80-0xffc

const long R5900::Cpu::PS2MemoryMapMask [ 1024 ] = { 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x000-0x07c; 000000000000-000001111100
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x080-0x0fc; 000010000000-000011111100
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x100-0x17c; 000100000000-000101111100
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x003FFFFF,	//0x180-0x1fc; 000110000000-000111111100
								0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x200-0x27c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x280-0x2fc
								0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x300-0x37c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x380-0x3fc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x400-0x47c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x480-0x4fc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x500-0x57c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x580-0x5fc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x600-0x67c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x680-0x6fc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x700-0x77c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x780-0x7fc
								0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x800-0x87c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x880-0x8fc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0x900-0x97c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x003FFFFF,	//0x980-0x9fc
								0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0x01FFFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xa00-0xa7c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xa80-0xafc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xb00-0xb7c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x003FFFFF,	//0xb80-0xbfc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xc00-0xc7c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xc80-0xcfc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xd00-0xd7c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xd80-0xdfc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xe00-0xe7c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xe80-0xefc
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//0xf00-0xf7c
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	//0xf80-0xffc





