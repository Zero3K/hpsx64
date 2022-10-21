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



#include "PS2_SIF.h"
#include "PS1_DMA.h"
#include "PS2_DMA.h"

#include "PS1_Intc.h"

#include "ps1_system.h"

//#include "WinApiHandler.h"

//using namespace Playstation2;


//#define ENABLE_NEW_SIF


//#define ENABLE_BUFFER_SIF


// don't transfer via sif if ps1 dma#3 is active??
//#define ENABLE_WAIT_CDVD_OUT
//#define ENABLE_WAIT_CDVD_IN

// check that the channel is active on ps1 side before transferring
#define REQUIRE_ACTIVE_CHANNEL_9
#define REQUIRE_ACTIVE_CHANNEL_10


// ps1 dma 3 causes issues for sif1 incoming transfer ??
//#define REQUIRE_NO_DMA3_FOR_DMA10
//#define REQUIRE_NO_DMA3_FOR_DMA9


// require ps1 dma to be enabled before transfer across sif
#define REQUIRE_PS1DMA_ENABLE
#define REQUIRE_PS2DMA_ENABLE


// enable timing for new chain transfer channels on IOP side
// note: don't want another channel to run on ps1 side while sif is transferring unless it has priority
// note: current ps1 transfer can be interrupted by a higher priority transfer
//#define ENABLE_IOP_SIF_TIMING


//#define ENABLE_SIF_DMA_TIMING
#define ENABLE_SIF_DMA_SYNC


//#define ENABLE_TRANSFER_DIRECTION


#define DMA9_TOEE_ROUND_UP
#define DMA9_TOEE_RETURN_ROUND_UP



#ifdef _DEBUG_VERSION_

#define INLINE_DEBUG_ENABLE
//#define INLINE_DEBUG

//#define INLINE_DEBUG_SPLIT

/*
#define INLINE_DEBUG_WRITE_EE
#define INLINE_DEBUG_READ_EE
#define INLINE_DEBUG_WRITE_IOP
#define INLINE_DEBUG_READ_IOP

//#define INLINE_DEBUG_DMA_READ_EE
//#define INLINE_DEBUG_DMA_WRITE_EE
//#define INLINE_DEBUG_DMA_READ_IOP
//#define INLINE_DEBUG_DMA_WRITE_IOP


//#define INLINE_DEBUG_DMA_WRITE_EE_CONTENTS
//#define INLINE_DEBUG_DMA_WRITE_IOP_CONTENTS


//#define INLINE_DEBUG_DMA_IN_READY
//#define INLINE_DEBUG_DMA_OUT_READY
//#define INLINE_DEBUG_RUN
*/

#endif



namespace Playstation2
{

u32 *SIF::_DebugPC;
u64 *SIF::_DebugCycleCount;
u32* SIF::_NextEventIdx;

u32* SIF::_R3000A_Intc_Stat;
u32* SIF::_R3000A_Intc_Mask;
u32* SIF::_R3000A_Status_12;
u32* SIF::_R3000A_Cause_13;
u64* SIF::_R3000A_ProcStatus;

u32* SIF::_R5900_Intc_Stat;
u32* SIF::_R5900_Intc_Mask;
u32* SIF::_R5900_Status_12;
u32* SIF::_R5900_Cause_13;
u64* SIF::_R5900_ProcStatus;


u64* SIF::_NextSystemEvent;


Debug::Log SIF::debug;

SIF *SIF::_SIF;


SIF::RegData *SIF::pRegData;


const char* SIF::Reg_Names_EE [ 7 ] = { "F200", "F210", "F220", "F230", "EE_SIF_CTRL", "F250", "F260" };
const char* SIF::Reg_Names_IOP [ 7 ] = { "F200", "F210", "F220", "F230", "IOP_SIF_CTRL", "F250", "F260" };


SIF::SIF ()
{
	cout << "Running SIF constructor...\n";


}


void SIF::Start ()
{
	cout << "Running SIF::Start...\n";

#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create( "PS2_SIF_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering SIF::Start";
#endif


	Reset ();
	
	_SIF = this;
	
	// ???
	SIFRegs.F260 = 0x1d000060;
	
	DebugCount = c_iDebugLines;

	// start the events
	//SetNextEvent ( c_llIntInterval );
	Set_NextEventCycle ( -1ULL );
	
	
	// set pointer into hardware regiters
	pRegData = & SIFRegs;

	
#ifdef INLINE_DEBUG
	debug << "->Exiting SIF::Start";
#endif
}


void SIF::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( SIF ) );
}





void SIF::Run ()
{


	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;

#ifdef INLINE_DEBUG_RUN
	debug << "\r\n\r\nSIF::Run; CycleCount=" << dec << *_DebugCycleCount;
	debug << "; (before) NextEvent_Cycle=" << dec << NextEvent_Cycle;
#endif

	// check if the next transfer is ready
	
	
	// *testing* ?? interrupt ??
	//if ( _SIF->lSBUS_F220 & 0x10000 )
	//{
	//	_SIF->SetInterrupt_IOP_SIF ();
	//}
	
	// this part is figured out
	//_SIF->SetInterrupt_EE_SIF ();


	// doesn't look like this is doing anything
	SetNextEvent ( c_llIntInterval );

#ifdef INLINE_DEBUG_RUN
	debug << "; (after) NextEvent_Cycle=" << dec << NextEvent_Cycle;
#endif
}







u64 SIF::EE_Read ( u32 Address, u64 Mask )
{
#ifdef INLINE_DEBUG_READ_EE
	if ( --_SIF->DebugCount > 0 )
	{
	debug << "\r\nEE::SIF::Read " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << " Mask=" << Mask;
	}
#endif

	u32 Output;
	
	if ( !Mask )
	{
#ifdef INLINE_DEBUG_READ_EE
	if ( _SIF->DebugCount > 0 )
	{
	debug << " (128-bit WRITE)";
	}
#endif

		cout << "\nhps2x64: ALERT: EE: SIF: 128-bit WRITE to SIF. Address=" << hex << Address << "\n";
	}
	
	if ( Address & 0xf )
	{
#ifdef INLINE_DEBUG_READ_EE
	if ( _SIF->DebugCount > 0 )
	{
	debug << " NOT-ALIGNED";
	}
#endif

		cout << "\nhps2x64: ALERT: EE: SIF: Address not aligned. Address=" << hex << Address << "\n";
	}
	
	Address &= 0xffff;
	Address -= 0xf200;
	

	// make sure the address offset is in the right range
	if ( Address < 0x70 )
	{
#ifdef INLINE_DEBUG_READ_EE
	if ( _SIF->DebugCount )
	{
	debug << " " << Reg_Names_EE [ Address >> 4 ];
	}
#endif

		Output = pRegData->Regs [ Address >> 4 ];
	}
	else
	{
#ifdef INLINE_DEBUG_READ_EE
	if ( _SIF->DebugCount )
	{
	debug << " " << "INVALID";
	}
#endif

		// address is not valid
		cout << "\nhps2x64: ALERT: EE: SIF: Invalid Offset=" << hex << Address << "\n";
	}

#ifdef INLINE_DEBUG_READ_EE
	if ( _SIF->DebugCount )
	{
	debug << " OUTPUT=" << hex << Output;
	}
#endif

	return Output;
	
	
	/*
	switch ( Address )
	{
		case EESIF_F200:
#ifdef INLINE_DEBUG_READ_EE
	if ( _SIF->DebugCount > 0 ) { debug << "; F200; EE->IOP"; }
#endif

			Output = _SIF->lSBUS_F200;
			break;
			
		case EESIF_F210:
#ifdef INLINE_DEBUG_READ_EE
	if ( _SIF->DebugCount > 0 ) { debug << "; F210; IOP->EE"; }
#endif

			Output = _SIF->lSBUS_F210;
			break;
			
		case EESIF_F220:
#ifdef INLINE_DEBUG_READ_EE
	if ( _SIF->DebugCount > 0 ) { debug << "; F220"; }
#endif

			Output = _SIF->lSBUS_F220;
			break;
			
		case EESIF_F230:
#ifdef INLINE_DEBUG_READ_EE
	if ( _SIF->DebugCount > 0 ) { debug << "; F230"; }
#endif
	
			Output = _SIF->lSBUS_F230;
			break;
			
		case EESIF_CTRL:
#ifdef INLINE_DEBUG_READ_EE
	if ( _SIF->DebugCount > 0 ) { debug << "; F240; CTRL"; }
#endif

			Output = _SIF->lSBUS_F240 | 0xf0000102;
			break;
			
		case EESIF_F260:
#ifdef INLINE_DEBUG_READ_EE
	if ( _SIF->DebugCount > 0 ) { debug << "; F260"; }
#endif

			Output = _SIF->lSBUS_F260;
			break;
			
		default:
#ifdef INLINE_DEBUG_READ_EE
	if ( _SIF->DebugCount > 0 ) { debug << "; Invalid"; }
#endif
		
			// invalid SIO Register
			cout << "\nhps2x64 ALERT: Unknown SIF READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
			break;
	};
	
#ifdef INLINE_DEBUG_READ_EE
	if ( _SIF->DebugCount > 0 ) { debug << "; Output =" << hex << Output; }
#endif

	return Output;
	*/
}




void SIF::EE_Write ( u32 Address, u64 Data, u64 Mask )
{
#ifdef INLINE_DEBUG_WRITE_EE
	debug << "\r\nEE::SIF::Write " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << " Mask=" << Mask << " Data=" << Data;
	_SIF->DebugCount = c_iDebugLines;
#endif

	
	switch ( Address )
	{
		case EESIF_F200:
#ifdef INLINE_DEBUG_WRITE_EE
			debug << "; F200; EE->IOP";
#endif
			// EE write path, so store data here
			// 0x1000f210 is the IOP write path
			pRegData->F200 = Data;
			break;
			
		case EESIF_F210:
#ifdef INLINE_DEBUG_WRITE_EE
			debug << "; F210; IOP->EE";
#endif

			break;
			
		case EESIF_F220:
#ifdef INLINE_DEBUG_WRITE_EE
			debug << "; F220";
#endif

			// bits get SET from EE side and CLEARED when written from IOP side
			// ??interrupt trigger?? //
			pRegData->F220 |= Data;
			
			// treat bit 16 as an interrupt trigger ??
			if ( pRegData->F220 & 0x10000 )
			{
				//Playstation1::Intc::_INTC->I_STAT_Reg.Unknown0 = 1;
				//Playstation1::Intc::_INTC->UpdateInts ();
			}
			
			break;
			
		case EESIF_F230:
#ifdef INLINE_DEBUG_WRITE_EE
			debug << "; F230";
#endif

			// bits get cleared when written from EE and set when written from IOP
			pRegData->F230 &= ~Data;
			break;
			
		case EESIF_CTRL:
#ifdef INLINE_DEBUG_WRITE_EE
			debug << "; F240; CTRL";
#endif

			// control register //
			
			
			if(!(Data & 0x100))
			{
				pRegData->F240 &= ~0x100;
			}
			else
			{
				pRegData->F240 |= 0x100;
			}
			
			pRegData->F240 |= 0xf0000102;
			break;
			
		case EESIF_F260:
#ifdef INLINE_DEBUG_WRITE_EE
			debug << "; F260";
#endif

			// ??? //
			pRegData->F260 = 0;
			break;

			
		default:
#ifdef INLINE_DEBUG_WRITE_EE
			debug << "; Invalid";
#endif
		
			// invalid SIO Register
			cout << "\nhps2x64 ALERT: Unknown SIF WRITE @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data << "\n";
			break;
	};
	
}






u32 SIF::IOP_Read ( u32 Address )
{
#ifdef INLINE_DEBUG_READ_IOP
	if ( --_SIF->DebugCount > 0 )
	{
	debug << "\r\nIOP::SIF::Read " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address;
	}
#endif

	u32 Output;
	u32 Offset;
	
	if ( Address & 0xf )
	{
#ifdef INLINE_DEBUG_READ_IOP
	if ( _SIF->DebugCount > 0 )
	{
	debug << " NOT-ALIGNED";
	}
#endif

		cout << "\nhps2x64: ALERT: IOP: SIF: Address not aligned. Address=" << hex << Address << "\n";
	}
	
	Offset = Address & 0xffff;
	//Address -= 0xf200;
	

	// make sure the address offset is in the right range
	if ( Offset < 0x70 )
	{
#ifdef INLINE_DEBUG_READ_IOP
	if ( _SIF->DebugCount )
	{
	debug << " " << Reg_Names_IOP [ Offset >> 4 ];
	}
#endif

		Output = pRegData->Regs [ Offset >> 4 ];
	}
	else
	{
#ifdef INLINE_DEBUG_READ_IOP
	if ( _SIF->DebugCount )
	{
	debug << " " << "INVALID";
	}
#endif

		// address is not valid
		cout << "\nhps2x64 ALERT: Unknown SIF READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
		Output = 0;
	}
	
#ifdef INLINE_DEBUG_READ_IOP
	if ( _SIF->DebugCount )
	{
	debug << " Output=" << hex << Output;
	}
#endif

	return Output;
	
	
	
	/*
	switch ( Address )
	{
		case IOPSIF_F200:
#ifdef INLINE_DEBUG_READ_IOP
	if ( _SIF->DebugCount > 0 ) { debug << "; F200; EE->IOP"; }
#endif

			// incoming from EE
			Output = _SIF->lSBUS_F200;
			break;
			
		case IOPSIF_F210:
#ifdef INLINE_DEBUG_READ_IOP
	if ( _SIF->DebugCount > 0 ) { debug << "; F210; IOP->EE"; }
#endif

			// outgoing from IOP
			Output = _SIF->lSBUS_F210;
			break;
			
		case IOPSIF_F220:
#ifdef INLINE_DEBUG_READ_IOP
	if ( _SIF->DebugCount > 0 ) { debug << "; F220"; }
#endif

			Output = _SIF->lSBUS_F220;
			break;
			
		case IOPSIF_F230:
#ifdef INLINE_DEBUG_READ_IOP
	if ( _SIF->DebugCount > 0 ) { debug << "; F230"; }
#endif

			Output = _SIF->lSBUS_F230;
			break;
			
		case IOPSIF_CTRL:
#ifdef INLINE_DEBUG_READ_IOP
	if ( _SIF->DebugCount > 0 ) { debug << "; F240; CTRL"; }
#endif

			// control register ?? //
			Output = _SIF->lSBUS_F240 | 0xf0000002;
			break;
			
		case IOPSIF_F260:
#ifdef INLINE_DEBUG_READ_IOP
	if ( _SIF->DebugCount > 0 ) { debug << "; F260"; }
#endif

			Output = _SIF->lSBUS_F260;
			break;

			
		default:
#ifdef INLINE_DEBUG_READ_IOP
	if ( _SIF->DebugCount > 0 ) { debug << "; Invalid"; }
#endif
		
			// invalid SIO Register
			cout << "\nhps2x64 ALERT: Unknown SIF READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
			break;
	};
	
#ifdef INLINE_DEBUG_READ_IOP
	if ( _SIF->DebugCount > 0 ) { debug << "; Output =" << hex << Output; }
#endif

	return Output;
	*/
}




void SIF::IOP_Write ( u32 Address, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE_IOP
	debug << "\r\nIOP::SIF::Write " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << "; Data=" << Data;
	_SIF->DebugCount = c_iDebugLines;
#endif

	u32 temp;
	u32 PreviousValue;
	
	switch ( Address )
	{
		case IOPSIF_F200:
#ifdef INLINE_DEBUG_WRITE_IOP
			debug << "; F200; EE->IOP";
#endif

			// incoming from EE
			break;
			
		case IOPSIF_F210:
#ifdef INLINE_DEBUG_WRITE_IOP
			debug << "; F210; IOP->EE";
#endif

			// outgoing from IOP
			pRegData->F210 = Data;
			break;
			
		case IOPSIF_F220:
#ifdef INLINE_DEBUG_WRITE_IOP
			debug << "; F220";
#endif

			// bits cleared when written from IOP
			pRegData->F220 &= ~Data;
			break;
			
		case IOPSIF_F230:
#ifdef INLINE_DEBUG_WRITE_IOP
			debug << "; F230";
#endif

			// bits set when written from IOP
			pRegData->F230 |= Data;
			break;
			
		case IOPSIF_CTRL:
#ifdef INLINE_DEBUG_WRITE_IOP
			debug << "; F240; CTRL";
			debug << "; (before) F240=" << hex << pRegData->F240;
#endif

			PreviousValue = pRegData->F240;

			if ( Data & ( 1 << 19 ) )
			{
				cout << "\n*** R3000A RESET SIGNAL ***\n";
			}

			temp = Data & 0xf0;
			if ( Data & 0x20 || Data & 0x80)
			{
				//pRegData->F240 &= ~0xf000;
				//pRegData->F240 |= 0x2000;
			}


			if ( pRegData->F240 & temp )
			{
				pRegData->F240 &= ~temp;
			}
			else
			{
				pRegData->F240 |= temp;
			}
			
			pRegData->F240 |= 0xf0000002;
			
			// this is incorrect
			//if ( Data & 0x40 )
			//{
//#ifdef INLINE_DEBUG_WRITE_IOP
//			debug << "; INT";
//#endif
				// *testing* ?? interrupt ??
				//_SIF->SetInterrupt_IOP_SIF ();
			//}
			
#ifdef ENABLE_SIF_DMA_SYNC
			if ( ( Data ^ PreviousValue ) & 0x60 )
			{
				// if a factor for ps1 dma changes, then update the active channel
				Update_ActiveChannel ();
			}

			// check if time for transfer
			if ( Data & 0x40 )
			{
				Check_TransferToIOP ();
			}
			
			if ( Data & 0x20 )
			{
				Check_TransferFromIOP ();
			}
#endif
			
#ifdef INLINE_DEBUG_WRITE_IOP
			debug << "; (after) F240=" << hex << _SIF->SIFRegs.F240;
#endif
			break;
			
		case IOPSIF_F260:
#ifdef INLINE_DEBUG_WRITE_IOP
			debug << "; F260";
#endif

			pRegData->F260 = 0;
			break;

			
		default:
#ifdef INLINE_DEBUG_WRITE_IOP
			debug << "; Invalid";
#endif
		
			// invalid SIF Register
			cout << "\nhps2x64 ALERT: Unknown SIF WRITE @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data << "\n";
			break;
	};
	
}




void SIF::EE_DMA_ReadBlock ()
{
#ifdef INLINE_DEBUG_DMA_READ_EE
	debug << "\r\nEE_DMA_ReadBlock (IOP->EE) " << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif


#ifdef ENABLE_BUFFER_SIF
	// check if dma#5 on ps2 is active
	//if ( Playstation2::Dma::pRegData [ 5 ]->CHCR.STR
	//	&& ! ( Playstation2::Dma::pDMARegs->ENABLER & 0x10000 ) )
	//{
		// check if there is data in buffer
		if ( _SIF->ulBufferSizeQWC )
		{
			// send data from buffer first
			Playstation2::Dma::_DMA->DMA5_WriteBlock ( _SIF->ullBuffer, _SIF->ulBufferSizeQWC );
			
			// set the amount of time SIF will be busy with transfer until
			//_SIF->BusyUntil_Cycle = *_DebugCycleCount + _SIF->ulBufferSizeQWC;
			_SIF->BusyUntil_Cycle = ( ( *_DebugCycleCount + 3 ) >> 2 ) + ( ( _SIF->ulBufferSizeQWC + 3 ) >> 2 );
			
			// all data was sent
			_SIF->ulBufferSizeQWC = 0;
			
			// ps1 needs to transfer more data into the buffer
			//Playstation1::Dma::_DMA->SetNextEventCh ( 1, 9 );
			Playstation1::Dma::_DMA->Update_ActiveChannel ();
		}
	//}
	
	return;
#endif

	// but need to update ps1 system events first, since otherwise some events can and will get skipped
	//Playstation1::System::_SYSTEM->RunEvents ( 0 );
	//Playstation1::System::_SYSTEM->RunEvents ();
	
	// IOP dma channel #9 is trying to pass data to EE dma channel #5
	//Playstation1::Dma::_DMA->DMA9_Run ();
	Playstation1::Dma::_DMA->Transfer ( 9 );
}

// returns the amound of data written
// Count is in quadwords
// no, should actually return 1 if transfer is complete and zero otherwise
u32 SIF::EE_DMA_WriteBlock ( u64* Data, u32 Count )
{
#ifdef INLINE_DEBUG_DMA_WRITE_EE
	debug << "\r\nEE_DMA_WriteBlock (EE->IOP) " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Count=" << Count;
	debug << " TagForIOP=" << hex << ((u64*) Data) [ 0 ];
#endif

	u32 *SrcDataPtr, *DstDataPtr;
	u32 Data0, Data1, DestAddress, IRQ, ID;
	//u32 IOPCount;

	u32 ulSpaceLeft;
	u32 QuadwordCount;
	
	// EE dma channel #6 is trying to pass data to IOP dma channel #10
	
	// first check if both PS2 dma channel#6 and IOP dma channel #10 are both enabled
	//if ( Playstation1::Dma::_DMA->DmaCh [ 10 ].TR && Playstation2::Dma::_DMA->DmaCh [ 6 ].STR )
	//{
	//	// both channels are not enabled
	//	return;
	//}
	
	/*
	Data0 = *Data++;
	Data1 = *Data++;
	
#ifdef INLINE_DEBUG_DMA_WRITE_EE
	debug << " Data0=" << hex << Data0 << " Data1=" << Data1;
#endif

	// the first 32-bit value is the address, interrupt, and whether to stop or keep transferring
	// for now, get destination address from Destination DMA tag
	DestAddress = Data0 & 0xffffff;
	Playstation1::Dma::_DMA->DmaCh [ 10 ].MADR = DestAddress;
	
	// check for interrupt
	// the tag is for the dma though, and probably isn't supposed to tell the SIF anything
	if ( Data0 & 0x80000000 )
	{
		// ***todo*** SIF interrupt??
	}
	
	// check for transfer stop
	if ( Data0 & 0x40000000 )
	{
		// ***todo*** needs more testing
	}
	*/
	
	// the second 32-bit value is the amount to transfer
	//IOPCount = Data1;
	
	// rest of quadword is ignored
	/*
	Data++;
	Data++;
	*/
	
	// we are now at the data to transfer
	//SrcDataPtr = (u32*) Data;
	
#ifdef INLINE_DEBUG_DMA_WRITE_EE_CONTENTS
	// check transfer contents
	debug << "\r\nData=" << hex;
	for ( int i = 0; i < ( Count << 1 ); i++ )
	{
		debug << " " << ((u64*) Data) [ i ];
	}
#endif
	
	// set the SIF buffer size
	//_SIF->BufferSize = IOPCount;
	
	// set the SIF buffer direction
	//_SIF->BufferDirection = BUFFER_SIF1;
	
	// if F240 does not have 0x40 set, then alert for testing
	if ( !( pRegData->F240 & 0x40 ) )
	{
		cout << "\nhps2x64: ALERT: SIF: F240 does not have 0x40 set, but IS writing data to IOP.\n";
		
#ifdef INLINE_DEBUG_DMA_WRITE_EE
		debug << "\r\nhps2x64: ALERT: SIF: F240 does not have 0x40 set, but IS writing data to IOP.";
#endif
	}
	

#ifdef ENABLE_NEW_SIF
	// ??
	pRegData->F240 |= 0x4000;
#endif
	

#ifdef ENABLE_BUFFER_SIF
	// check if dma#10 on ps1 is active
	/*
	if ( Playstation1::Dma::pRegData [ 10 ]->CHCR.TR
		&& Playstation1::Dma::_DMA->isEnabled ( 10 ) )
	{
		// check if there is data in buffer
		if ( _SIF->ulBufferSizeQWC )
		{
			// send data from buffer first
			//Playstation2::Dma::_DMA->DMA5_WriteBlock ( ullBuffer, _SIF->ulBufferSizeQWC );
			Playstation1::Dma::_DMA->DMA10_WriteBlock ( (u32*) _SIF->ullBuffer, _SIF->ulBufferSizeQWC << 2, *_DebugCycleCount );
			
			// all data was sent
			_SIF->ulBufferSizeQWC = 0;
		}
	}
	else
	*/
	{
		// put the data into buffer //
		
		// should have already checked that buffer is not full
		QuadwordCount = Count;
		
		// get space left in buffer
		ulSpaceLeft = c_iMaxBufferSizeQWC - _SIF->ulBufferSizeQWC;
		
		// get the amount of data that can be transferred
		QuadwordCount = ( QuadwordCount > ulSpaceLeft ) ? ulSpaceLeft : QuadwordCount;
		
		if ( QuadwordCount )
		{
			// set buffer direction
			_SIF->ulBufferDirection = BUFFER_SIF1_EE_TO_IOP;
			
			// transfer into buffer
			for ( int i = 0; i < QuadwordCount; i++ )
			{
				_SIF->ullBuffer [ ( _SIF->ulBufferSizeQWC << 1 ) + 0 ] = ((u64*)Data) [ ( i << 1 ) + 0 ];
				_SIF->ullBuffer [ ( _SIF->ulBufferSizeQWC << 1 ) + 1 ] = ((u64*)Data) [ ( i << 1 ) + 1 ];
				_SIF->ulBufferSizeQWC++;
			}
		}
		
		// set the amount of time SIF will be busy with transfer until
		//_SIF->BusyUntil_Cycle = *_DebugCycleCount + QuadwordCount;
		_SIF->BusyUntil_Cycle = ( ( *_DebugCycleCount + 3 ) >> 2 ) + ( ( QuadwordCount + 3 ) >> 2 );
		
		// ps1 needs to pick up the data once it is done transferring
		//Playstation1::Dma::_DMA->SetNextEventCh ( 1, 10 );
		Playstation1::Dma::_DMA->Update_ActiveChannel ();
		
		// return the amount of data transferred ??
		return QuadwordCount;
	}
#endif


	
	// make a call to transfer the data using PS1 dma
	// the count is the quad word count, so must be multiplied by 4 here
	//Playstation1::Dma::_DMA->DMA10_WriteBlock ( Data, Count << 2 );
	Playstation1::Dma::_DMA->DMA10_WriteBlock ( (u32*) Data, Count << 2, *_DebugCycleCount );
	
	// sif is now busy transferring the data until
	_SIF->BusyUntil_Cycle = R3000A::Cpu::_CPU->CycleCount + 2;
	
	// return the amount of data in quadwords that was written
	return Count;

	// for now, get transfer amount from Destination DMA tag
	
	// for now, transfer data into PS1 RAM
	
	// for now, set IOP dma channel 10 with updated destination address
	
	// check for SIF interrupt in Dest DMA tag
	
	// interrupt IOP if needed
}

u32 SIF::IOP_DMA_ReadBlock ( u32 *pMemory, u32 Address, u32 WordCount )
{
#ifdef INLINE_DEBUG_DMA_READ_IOP
	debug << "\r\nIOP_DMA_ReadBlock (EE->IOP) " << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

	// EE dma channel #6 is trying to pass data to IOP dma channel #10
	
	// make sure that iop set
	
	// events could get skipped - so backup event/system cycle#
	// *_DebugCycleCount = *_DebugCycleCount - 1;

#ifdef ENABLE_BUFFER_SIF
	// check if dma#10 on ps1 is active
	//if ( Playstation1::Dma::pRegData [ 10 ]->CHCR.TR
	//	&& Playstation1::Dma::_DMA->isEnabled ( 10 ) )
	{
		// check if there is data in buffer
		if ( _SIF->ulBufferSizeQWC )
		{
			// send data from buffer first
			//Playstation2::Dma::_DMA->DMA5_WriteBlock ( ullBuffer, _SIF->ulBufferSizeQWC );
			Playstation1::Dma::_DMA->DMA10_WriteBlock ( (u32*) _SIF->ullBuffer, _SIF->ulBufferSizeQWC << 2, *_DebugCycleCount );
			
			// set the amount of time SIF will be busy with transfer until
			//_SIF->BusyUntil_Cycle = *_DebugCycleCount + ( _SIF->ulBufferSizeQWC << 4 );
			_SIF->BusyUntil_Cycle = R3000A::Cpu::_CPU->CycleCount;
			
			// all data was sent
			_SIF->ulBufferSizeQWC = 0;
			
			// after the data is done sending, the ps2 can send more data into the buffer ?
			// when the data is done transferring into buffer, then ps2 dma#5 should pick it up
			//Playstation2::Dma::_DMA->SetNextEventCh_Cycle ( _SIF->BusyUntil_Cycle, 6 );
			//Playstation2::Dma::_DMA->DmaCh [ 6 ].ullStartCycle = _SIF->BusyUntil_Cycle;
			Playstation2::Dma::_DMA->SetNextEventCh_Cycle ( _SIF->BusyUntil_Cycle << 2, 6 );
			Playstation2::Dma::_DMA->DmaCh [ 6 ].ullStartCycle = _SIF->BusyUntil_Cycle << 2;
			
			// update dma device on the ps2 side with the new developments
			Playstation2::Dma::_DMA->CheckTransfer ();
		}
	}
	
	return 0;
#endif
	
	// trigger the transfer start from PS2 side
	Playstation2::Dma::_DMA->Transfer ( 6 );
	
	// events could get skipped - so backup event/system cycle#
	// *_DebugCycleCount = *_DebugCycleCount + 1;
	
	return 0;
}

// should return 1 if transfer is complete on ps2 side, zero otherwise
//void SIF::IOP_DMA_WriteBlock ( u64 EEDMATag, u32* Data, u32 Count )
u32 SIF::IOP_DMA_WriteBlock ( u32 *pMemory, u32 Address, u32 WordCount )
{
#ifdef INLINE_DEBUG_DMA_WRITE_IOP
	debug << "\r\nIOP_DMA_WriteBlock (IOP->EE) " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " WordCount=" << WordCount;
#endif

	u32 QuadwordCount;
	u64 *Data;
	
	u32 ulSpaceLeft;
	
	Data = (u64*) & ( pMemory [ Address >> 2 ] );

	// note: "Count" is a count of 32-bit words being transferred

#ifdef INLINE_DEBUG_DMA_WRITE_IOP_CONTENTS
	// check transfer contents
	debug << "\r\nData=" << hex;
	for ( int i = 0; i < WordCount; i++ )
	{
		debug << " " << ((u32*) Data) [ i ];
	}
#endif

#ifdef DMA9_TOEE_ROUND_UP
	// round count up to nearest 128-bit boundary and put in QWC
	QuadwordCount = ( WordCount + 3 ) >> 2;
#else
	QuadwordCount = ( WordCount + 0 ) >> 2;
#endif
	
	// can't transfer more than 8 quadwords at a time (SIF buffer is only 8 quadwords)
	//QuadwordCount = ( QuadwordCount > 8 ) ? 8 : QuadwordCount;

	
#ifdef ENABLE_NEW_SIF
	// ??
	pRegData->F240 |= 0x2000;
#endif
	
	
#ifdef ENABLE_BUFFER_SIF
	// check if dma#5 on ps2 is active
	/*
	if ( Playstation2::Dma::pRegData [ 5 ]->CHCR.STR
		&& ! ( Playstation2::Dma::pDMARegs->ENABLER & 0x10000 ) )
	{
		// check if there is data in buffer
		if ( _SIF->ulBufferSizeQWC )
		{
			// send data from buffer first
			Playstation2::Dma::_DMA->DMA5_WriteBlock ( _SIF->ullBuffer, _SIF->ulBufferSizeQWC );
			
			// all data was sent
			_SIF->ulBufferSizeQWC = 0;
		}
	}
	else
	*/
	{
		// put the data into buffer //
		
		// should have already checked that buffer is not full
		
		// get space left in buffer
		ulSpaceLeft = c_iMaxBufferSizeQWC - _SIF->ulBufferSizeQWC;
		
		// get the amount of data that can be transferred
		QuadwordCount = ( QuadwordCount > ulSpaceLeft ) ? ulSpaceLeft : QuadwordCount;
		
		if ( QuadwordCount )
		{
			// set buffer direction
			_SIF->ulBufferDirection = BUFFER_SIF0_IOP_TO_EE;
			
			for ( int i = 0; i < QuadwordCount; i++ )
			{
				_SIF->ullBuffer [ ( _SIF->ulBufferSizeQWC << 1 ) + 0 ] = Data [ ( i << 1 ) + 0 ];
				_SIF->ullBuffer [ ( _SIF->ulBufferSizeQWC << 1 ) + 1 ] = Data [ ( i << 1 ) + 1 ];
				_SIF->ulBufferSizeQWC++;
			}
			
			// set the amount of time SIF will be busy with transfer until
			//_SIF->BusyUntil_Cycle = *_DebugCycleCount + ( QuadwordCount << 4 );
			_SIF->BusyUntil_Cycle = R3000A::Cpu::_CPU->CycleCount + ( QuadwordCount << 2 );
			
			// when the data is done transferring into buffer, then ps2 dma#5 should pick it up
			//Playstation2::Dma::_DMA->SetNextEventCh_Cycle ( _SIF->BusyUntil_Cycle, 5 );
			//Playstation2::Dma::_DMA->DmaCh [ 5 ].ullStartCycle = _SIF->BusyUntil_Cycle;
			Playstation2::Dma::_DMA->SetNextEventCh_Cycle ( _SIF->BusyUntil_Cycle << 2, 5 );
			Playstation2::Dma::_DMA->DmaCh [ 5 ].ullStartCycle = _SIF->BusyUntil_Cycle << 2;
			
			// update dma device on the ps2 side with the new developments
			Playstation2::Dma::_DMA->CheckTransfer ();
		}
		
		
		// return the amount of data transferred ??
		return ( QuadwordCount << 2 );
	}
#endif

	
	// IOP dma channel #9 is trying to pass data to EE dma channel #5
	//Playstation2::Dma::_DMA->DMA5_WriteBlock ( EEDMATag, (u64*) Data, Count );
	Playstation2::Dma::_DMA->DMA5_WriteBlock ( Data, QuadwordCount );
	
	// sif is now busy transferring the data until
	// note: ps1 cpu cycle# should not be updated yet at this point
	//_SIF->BusyUntil_Cycle = R3000A::Cpu::_CPU->CycleCount + 2;
	_SIF->BusyUntil_Cycle = R3000A::Cpu::_CPU->CycleCount;

	// add on the time on the ps1 side
	_SIF->BusyUntil_Cycle += ( WordCount * Playstation1::Dma::c_ullAccessTime [ 9 ] );

	// add on the time on the ps2 side
	//_SIF->BusyUntil_Cycle += ( ( QuadwordCount + 3 ) >> 2 );
	_SIF->BusyUntil_Cycle += 8;
	
	//return WordCount;
#ifdef DMA9_TOEE_RETURN_ROUND_UP
	return ( ( QuadwordCount << 2 ) > WordCount ) ? WordCount : ( QuadwordCount << 2 );
#else
	return WordCount;
#endif
}


void SIF::Check_TransferToIOP ()
{
	if ( EE_DMA_Out_Ready () && IOP_DMA_In_Ready () )
	{
		// backup cycle - to prevent skips
		// *_DebugCycleCount = *_DebugCycleCount - 1;
		
		Playstation2::Dma::_DMA->Transfer ( 6 );
		
		// restore cycle
		// *_DebugCycleCount = *_DebugCycleCount + 1;
	}
}



void SIF::Check_TransferFromIOP ()
{
	if ( EE_DMA_In_Ready () && IOP_DMA_Out_Ready () )
	{
		// but need to update ps1 system events first, since otherwise some events can and will get skipped
		//Playstation1::System::_SYSTEM->RunEvents ( 0 );
		Playstation1::System::_SYSTEM->RunEvents ();
		
		//Playstation1::Dma::_DMA->DMA9_Run ();
		Playstation1::Dma::_DMA->Transfer ( 9 );
	}
}



// updates the active channel on the ps1 side
void SIF::Update_ActiveChannel ()
{
	Playstation1::Dma::_DMA->Update_ActiveChannel ();
}


u64 SIF::EE_To_IOP_DMA_Ready ()
{
	if ( EE_DMA_Out_Ready ()
		&& IOP_DMA_In_Ready () )
	{
#ifdef ENABLE_IOP_SIF_TIMING
		if ( *Playstation1::Dma::_DebugCycleCount < _SIF->BusyUntil_Cycle )
		{
			return _SIF->BusyUntil_Cycle;
		}
#endif
		
		return true;
	}
	
	return false;
}


u64 SIF::EE_To_IOP_DMA_Ready_FromEE ()
{
	if ( EE_DMA_Out_Ready ()
		&& IOP_DMA_In_Ready ()
#ifdef REQUIRE_ACTIVE_CHANNEL_10
		&& ( ( Playstation1::Dma::_DMA->ActiveChannel == 10 ) || ( Playstation1::Dma::_DMA->ActiveChannel == -1 ) )
#endif
#ifdef REQUIRE_PS1DMA_ENABLE
		&& ( Playstation1::Dma::_DMA->DMARegs1.DMA_ENABLE & 1 )
#endif
#ifdef REQUIRE_NO_DMA3_FOR_DMA10
		//&& ( ( ! Playstation1::Dma::pRegData [ 3 ]->CHCR.TR ) )
		//&& !( ( Playstation1::Dma::pRegData [ 3 ]->CHCR.TR ) && ( ( CDVD::_CDVD->Status & CDVD_STATUS_READ ) == CDVD_STATUS_READ ) )
		&& ( ( ! Playstation1::Dma::pRegData [ 3 ]->CHCR.TR ) || ( CDVD::_CDVD->DiskSpeedType & 0x80 ) )
#endif
	)
	{
		return 1;
	}
	
	return 0;
}


u64 SIF::IOP_To_EE_DMA_Ready ()
{
	if ( EE_DMA_In_Ready ()
		&& IOP_DMA_Out_Ready () )
	{
#ifdef ENABLE_IOP_SIF_TIMING
		if ( *Playstation1::Dma::_DebugCycleCount < _SIF->BusyUntil_Cycle )
		{
			return _SIF->BusyUntil_Cycle;
		}
#endif

		return true;
	}
	
	return false;
}

u64 SIF::IOP_To_EE_DMA_Ready_FromEE ()
{
	if ( EE_DMA_In_Ready ()
		&& IOP_DMA_Out_Ready ()
#ifdef REQUIRE_ACTIVE_CHANNEL_9
		&& ( ( Playstation1::Dma::_DMA->ActiveChannel == 9 ) || ( Playstation1::Dma::_DMA->ActiveChannel == -1 ) )
#endif
#ifdef REQUIRE_PS1DMA_ENABLE
		&& ( Playstation1::Dma::_DMA->DMARegs1.DMA_ENABLE & 1 )
#endif
#ifdef REQUIRE_NO_DMA3_FOR_DMA9
		//&& ( ( ! Playstation1::Dma::pRegData [ 3 ]->CHCR.TR ) )
		//&& !( ( Playstation1::Dma::pRegData [ 3 ]->CHCR.TR ) && ( ( CDVD::_CDVD->Status & CDVD_STATUS_READ ) == CDVD_STATUS_READ ) )
		&& ( ( ! Playstation1::Dma::pRegData [ 3 ]->CHCR.TR ) || ( CDVD::_CDVD->DiskSpeedType & 0x80 ) )
#endif
	)
	{
		return 1;
	}
	
	return 0;
}

u64 SIF::EE_DMA_In_Ready ()
{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << " SIF::EE_DMA_In_Ready";
	debug << "-Dma5.STR=" << Playstation2::Dma::pRegData [ 5 ]->CHCR.STR;
	//debug << " Dma10.isEnabled=" << Playstation1::Dma::_DMA->isEnabled ( 10 );
	debug << "-SBUS_F240=" << hex << pRegData->F240;
	//debug << " BufSize=" << _SIF->ulBufferSizeQWC;
	//debug << " Dir=" << _SIF->ulBufferDirection;
#endif




#ifdef ENABLE_BUFFER_SIF
	// unless buffer has data going in the other direction, it is either going into buffer or going into ps1 //
	
	// check if data is ready yet
	if ( _SIF->BusyUntil_Cycle > *_DebugCycleCount )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << " SIF-BUSY";
#endif

		return false;
	}

	if ( _SIF->ulBufferDirection != SIF::BUFFER_SIF0_IOP_TO_EE )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << " WRONG-DIRECTION";
#endif

		//if ( _SIF->ulBufferSizeQWC )
		//{
			// if data in buffer is going the opposite direction, then need to wait ??
			return false;
		//}
	}
	
	// if buffer has data, then it is ready to be transferred
	if ( !_SIF->ulBufferSizeQWC )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << " BUFFER-EMPTY";
#endif

		return false;
	}
	
	return true;
	
	// check below if data can go into ps1 //
	
#endif



#ifdef ENABLE_SIF_DMA_SYNC
	if ( ! ( pRegData->F240 & 0x20 ) )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << "-!F240&0x20";
#endif

		return false;
	}
#endif

#ifdef ENABLE_TRANSFER_DIRECTION
	if ( pRegData->F240 & 0x4000 )
	{
		return false;
	}
#endif


	if ( ! Playstation2::Dma::pRegData [ 5 ]->CHCR.STR )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << "-!DMA5.STR";
#endif

		return false;
	}

	if ( Playstation2::Dma::pDMARegs->ENABLER & 0x10000 )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << "-!ENABLER";
#endif

		return false;
	}


#ifdef REQUIRE_PS2DMA_ENABLE
	if ( !( Playstation2::Dma::pDMARegs->CTRL.DMAE ) )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << "-!DMAE";
#endif

		return false;
	}
#endif

	// checks if SIF0 channel#5 is ready on EE
	// maybe incoming dma transfers are not affected by dmac enable
	/*
	//return ( Playstation2::Dma::_DMA->DmaCh [ 5 ].CHCR_Reg.STR && ! ( Playstation2::Dma::_DMA->lDMAC_ENABLE & 0x10000 ) );
	return ( Playstation2::Dma::pRegData [ 5 ]->CHCR.STR
			&& ! ( Playstation2::Dma::pDMARegs->ENABLER & 0x10000 )
//#ifdef ENABLE_SIF_DMA_SYNC
//			&& ( pRegData->F240 & 0x20 )
//#endif
#ifdef ENABLE_SIF_DMA_TIMING
			&& ( *_DebugCycleCount >= _SIF->BusyUntil_Cycle )
#endif
			);
			*/

#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << "-READY";
#endif

	return true;

}

u64 SIF::EE_DMA_Out_Ready ()
{
#ifdef INLINE_DEBUG_DMA_OUT_READY
	debug << " SIF::EE_DMA_Out_Ready";
	//debug << " Dma5.STR=" << Playstation2::Dma::pRegData [ 5 ]->CHCR.STR;
	//debug << " Dma10.isEnabled=" << Playstation1::Dma::_DMA->isEnabled ( 10 );
	debug << "-SBUS_F240=" << hex << pRegData->F240;
	//debug << " BufSize=" << _SIF->ulBufferSizeQWC;
	//debug << " Dir=" << _SIF->ulBufferDirection;
#endif




#ifdef ENABLE_BUFFER_SIF
	// unless buffer has data going in the other direction, it is either coming into EE from buffer or ps1 //

	// check if data is ready yet
	if ( _SIF->BusyUntil_Cycle > *_DebugCycleCount )
	{
#ifdef INLINE_DEBUG_DMA_OUT_READY
	debug << " SIF-BUSY";
#endif

		return false;
	}
	
	if ( _SIF->ulBufferDirection == SIF::BUFFER_SIF0_IOP_TO_EE )
	{
#ifdef INLINE_DEBUG_DMA_OUT_READY
	debug << " WRONG-DIRECTION";
#endif

		// if data in buffer is going the opposite direction, then need to wait ??
		return false;
	}
	
	// check if buffer is full
	if ( _SIF->ulBufferSizeQWC == SIF::c_iMaxBufferSizeQWC )
	{
#ifdef INLINE_DEBUG_DMA_OUT_READY
	debug << " BUFFER-FULL";
#endif

		// data can't fit in buffer
		return false;
	}
	
	// there is no data in the buffer to transfer
	return true;
	
	// check below if data can come from ps1 //
	
#endif



#ifdef ENABLE_SIF_DMA_SYNC
	if ( ! ( pRegData->F240 & 0x40 ) )
	{
#ifdef INLINE_DEBUG_DMA_OUT_READY
	debug << "-!F240&0x40";
#endif

		return false;
	}
#endif

#ifdef ENABLE_TRANSFER_DIRECTION
	if ( pRegData->F240 & 0x2000 )
	{
		return false;
	}
#endif


	if ( ! Playstation2::Dma::pRegData [ 6 ]->CHCR.STR )
	{
#ifdef INLINE_DEBUG_DMA_OUT_READY
	debug << "-!DMA6.STR";
#endif

		return false;
	}

	if ( Playstation2::Dma::pDMARegs->ENABLER & 0x10000 )
	{
#ifdef INLINE_DEBUG_DMA_OUT_READY
	debug << "-!ENABLER";
#endif

		return false;
	}


#ifdef REQUIRE_PS2DMA_ENABLE
	if ( !( Playstation2::Dma::pDMARegs->CTRL.DMAE ) )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << "-!DMAE";
#endif

		return false;
	}
#endif

	// checks if SIF1 channel#6 is ready on EE
	/*
	//return ( Playstation2::Dma::_DMA->DmaCh [ 6 ].CHCR_Reg.STR && ! ( Playstation2::Dma::_DMA->lDMAC_ENABLE & 0x10000 ) );
	return ( Playstation2::Dma::pRegData [ 6 ]->CHCR.STR
			&& ! ( Playstation2::Dma::pDMARegs->ENABLER & 0x10000 )
//#ifdef ENABLE_SIF_DMA_SYNC
//			&& ( pRegData->F240 & 0x40 )
//#endif
#ifdef ENABLE_SIF_DMA_TIMING
			&& ( *_DebugCycleCount >= _SIF->BusyUntil_Cycle )
#endif
			);
			*/

#ifdef INLINE_DEBUG_DMA_OUT_READY
	debug << "-READY";
#endif

	return true;
}


bool SIF::IOP_DMA_In_Ready ()
{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << " SIF::IOP_DMA_In_Ready";
	debug << "-DMA10.TR=" << Playstation1::Dma::pRegData [ 10 ]->CHCR.TR;
	debug << "-DMA10.isEnabled=" << Playstation1::Dma::_DMA->isEnabled ( 10 );
	debug << "-SBUS_F240=" << hex << pRegData->F240;
	//debug << " BufSize=" << _SIF->ulBufferSizeQWC;
	//debug << " Dir=" << _SIF->ulBufferDirection;
#endif




#ifdef ENABLE_BUFFER_SIF
	// unless buffer has data going in the other direction, it is either going into buffer or going into ps1 //

	/*
	// check if data is ready yet
	if ( _SIF->BusyUntil_Cycle > *_DebugCycleCount )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << " SIF-BUSY";
#endif

		return false;
	}
	*/
	
	if ( _SIF->ulBufferDirection != SIF::BUFFER_SIF1_EE_TO_IOP )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << " WRONG-DIRECTION";
#endif

		//if ( _SIF->ulBufferSizeQWC )
		//{
			// if data in buffer is going the opposite direction, then need to wait ??
			return false;
		//}
	}
	
	// if buffer isn't full, then data is going into buffer
	// if buffer has data, then it can be transferred
	//if ( _SIF->ulBufferSizeQWC < SIF::c_iMaxBufferSizeQWC )
	if ( !_SIF->ulBufferSizeQWC )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << " BUFFER-EMPTY";
#endif

		return false;
	}
	
	return true;
	
	// check below if data can go into ee //
	
#endif



#ifdef ENABLE_SIF_DMA_SYNC
	if ( ! ( pRegData->F240 & 0x40 ) )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << "-!F240&0x40";
#endif

		return false;
	}
#endif

#ifdef ENABLE_TRANSFER_DIRECTION
	if ( pRegData->F240 & 0x2000 )
	{
		return false;
	}
#endif


	if ( ! Playstation1::Dma::pRegData [ 10 ]->CHCR.TR )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << "-!DMA10.TR";
#endif

		return false;
	}

	if ( !Playstation1::Dma::_DMA->isEnabled ( 10 ) )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << "-!DMA10.EN";
#endif

		return false;
	}

	// note: looks like it writes 0x40 to F240 to allow DMA transfer into IOP?
	
	// checks if SIF1 channel#10 is ready on IOP
	// note: also need to make sure that bit 6 (0x40) is set in SBUS_F240
	/*
	//return Playstation1::Dma::_DMA->DmaCh [ 10 ].CHCR.TR;
	if ( Playstation1::Dma::pRegData [ 10 ]->CHCR.TR
			&& Playstation1::Dma::_DMA->isEnabled ( 10 )
#ifdef ENABLE_CHECK_ACTIVE_CHANNEL
			&& ( Playstation1::Dma::_DMA->ActiveChannel == 10 )
#endif
//#ifdef ENABLE_SIF_DMA_SYNC
//			&& ( pRegData->F240 & 0x40 )
//#endif
#ifdef ENABLE_SIF_DMA_TIMING
			&& ( *_DebugCycleCount >= _SIF->BusyUntil_Cycle )
#endif
	)
	{
		return true;
	}
	*/

#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << "-READY";
#endif

	return true;
}

bool SIF::IOP_DMA_Out_Ready ()
{
#ifdef INLINE_DEBUG_DMA_OUT_READY
	debug << " SIF::IOP_DMA_Out_Ready";
	//debug << " Dma5.STR=" << Playstation2::Dma::pRegData [ 5 ]->CHCR.STR;
	//debug << " Dma10.isEnabled=" << Playstation1::Dma::_DMA->isEnabled ( 10 );
	debug << " SBUS_F240=" << hex << pRegData->F240;
	debug << " BufSize=" << _SIF->ulBufferSizeQWC;
	debug << " Dir=" << _SIF->ulBufferDirection;
#endif


#ifdef ENABLE_BUFFER_SIF
	// unless buffer has data going in the other direction, it is either coming from buffer or from IOP //
	
	/*
	// check if data is ready yet
	if ( _SIF->BusyUntil_Cycle > *_DebugCycleCount )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << " SIF-BUSY";
#endif

		return false;
	}
	*/
	
	if ( _SIF->ulBufferDirection == SIF::BUFFER_SIF1_EE_TO_IOP )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << " WRONG-DIRECTION";
#endif

		// if data in buffer is going the opposite direction, then need to wait ??
		return false;
	}
	
	// check if buffer is full
	if ( _SIF->ulBufferSizeQWC == SIF::c_iMaxBufferSizeQWC )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << " BUFFER-FULL";
#endif

		// data can't fit in buffer
		return false;
	}
	
	// there is no data in the buffer to transfer
	return true;
	
	// check below if data can come from ee //
	
#endif



#ifdef ENABLE_SIF_DMA_SYNC
	if ( ! ( pRegData->F240 & 0x20 ) )
	{
#ifdef INLINE_DEBUG_DMA_IN_READY
	debug << " !F240&0x20";
#endif

		return false;
	}
#endif

#ifdef ENABLE_TRANSFER_DIRECTION
	if ( pRegData->F240 & 0x4000 )
	{
		return false;
	}
#endif



	// note: looks like it writes 0x20 to F240 to allow DMA transfer FROM IOP to EE?
	
	// checks if SIF0 channel#9 is ready on IOP
	//return Playstation1::Dma::_DMA->DmaCh [ 9 ].CHCR.TR;
	return ( Playstation1::Dma::pRegData [ 9 ]->CHCR.TR
			&& Playstation1::Dma::_DMA->isEnabled ( 9 )
#ifdef ENABLE_CHECK_ACTIVE_CHANNEL
			&& ( Playstation1::Dma::_DMA->ActiveChannel == 9 )
#endif
//#ifdef ENABLE_SIF_DMA_SYNC
//			&& ( pRegData->F240 & 0x20 )
//#endif
#ifdef ENABLE_SIF_DMA_TIMING
			&& ( *_DebugCycleCount >= _SIF->BusyUntil_Cycle )
#endif
			);
}




void SIF::SuspendTransfer_IOP ( int channel )
{
	Playstation1::Dma::_DMA->SuspendTransfer ( channel );
}

void SIF::SuspendTransfer_EE ( int channel )
{
	Playstation2::Dma::_DMA->SuspendTransfer ( channel );
}




void SIF::Update_NextEventCycle ()
{
	//if ( NextEvent_Cycle > *_DebugCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_DebugCycleCount ) )
	if ( NextEvent_Cycle < *_NextSystemEvent )
	{
		*_NextSystemEvent = NextEvent_Cycle;
		*_NextEventIdx = NextEvent_Idx;
	}
}


void SIF::SetNextEvent ( u64 Cycle )
{
	NextEvent_Cycle = Cycle + *_DebugCycleCount;
	
	Update_NextEventCycle ();
}


void SIF::Set_NextEventCycle ( u64 Cycle )
{
	NextEvent_Cycle = Cycle;
	
	Update_NextEventCycle ();
}


//void SIF::GetNextEvent ()
//{
//	SetNextEvent ( WaitCycles0 );
//}

}




