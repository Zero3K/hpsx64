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



#include "PS2_IPU.h"
#include "PS2_Dma.h"


using namespace Playstation2;



//#define VERBOSE_IPU_VDEC


//#define VERBOSE_IPU_READ
//#define VERBOSE_IPU_WRITE
//#define VERBOSE_IPU_BDEC
#define VERBOSE_IPU_IDEC
//#define VERBOSE_IPU_CSC
#define VERBOSE_IPU_PACK


#define ENABLE_IPU_DITHER


//#define DISABLE_INTERRUPTS



#define ENABLE_FDEC_ON_BCLR

// just transfer out 8 qwords at a time
#define USE_FIFO_OUT_SIZE_FOR_TRANSFER



#ifdef _DEBUG_VERSION_

#define INLINE_DEBUG_ENABLE



//#define INLINE_DEBUG_SPLIT
//#define INLINE_DEBUG
//#define INLINE_DEBUG_CONSOLE

/*
#define INLINE_DEBUG_WRITE
//#define INLINE_DEBUG_READ
//#define INLINE_DEBUG_RUN

//#define INLINE_DEBUG_DMA_WRITE
//#define INLINE_DEBUG_DMA_READ



#define INLINE_DEBUG_READ_NOTBUSY

//#define INLINE_DEBUG_VDEC
//#define INLINE_DEBUG_FDEC

//#define INLINE_DEBUG_PROCEED
//#define INLINE_DEBUG_LOADIQ
//#define INLINE_DEBUG_PEEK

//#define INLINE_DEBUG_RESERVE

//#define INLINE_DEBUG_PROCESS
*/

#endif


// static debug items
bool IPU::DebugWindow_Enabled;
WindowClass::Window* IPU::DebugWindow;
DebugValueList<u32>* IPU::ValueList;


u32 *IPU::_DebugPC;
u64 *IPU::_DebugCycleCount;
u32* IPU::_NextEventIdx;

u32* IPU::_Intc_Stat;
u32* IPU::_Intc_Mask;
u32* IPU::_R5900_Status_12;
u32* IPU::_R5900_Cause_13;
u64* IPU::_ProcStatus;


u64* IPU::_NextSystemEvent;


Debug::Log IPU::debug;


IPU *IPU::_IPU;

u64 IPU::DataOut [ 2 ];



const s32 IPU::sDitherArray_4_1 [ 4 * 4 ] = { -4 << 1, 0, -3 << 1, 1 << 1,
													2 << 1, -2 << 1, 3 << 1, -1 << 1,
													-5, 3, -7, 1,
													7, -1, 5, -3 };



const char* IPU::RegNames [ 4 ] = { "CMD", "CTRL", "BP", "TOP" };



IPU::IPU ()
{
	cout << "Running IPU constructor...\n";

/*
#ifdef INLINE_DEBUG_ENABLE
	debug.Create( "PS2_IPU_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering IPU::IPU constructor";
#endif


	Reset ();

	
#ifdef INLINE_DEBUG
	debug << "->Exiting IPU::IPU constructor";
#endif
*/

}


void IPU::Start ()
{
	cout << "Running IPU::Start...\n";

#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create( "PS2_IPU_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering IPU::Start";
#endif

	
	_IPU = this;

	ipu_cmd_pos = & ( ipu_cmd.pos [ 0 ] );
	decoder = & ( thedecoder );

	// must reset AFTER setting static pointers or it could crash
	Reset ();

	// clear events
	Set_NextEventCycle ( -1ULL );
	
#ifdef INLINE_DEBUG
	debug << "->Exiting IPU::Start";
#endif
}


void IPU::Reset ()
{
	// zero object
	memset ( this, 0, sizeof( IPU ) );
	
	// clear the command
	CMD_Write.Value = -1;
	
	// this must be set (like in pcsx2) or the thedecoder does not work right ??
	decoder->picture_structure = FRAME_PICTURE;
	
}





void IPU::Run ()
{


	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;

#ifdef INLINE_DEBUG_RUN
	debug << "\r\n\r\nIPU::Run; CycleCount=" << dec << *_DebugCycleCount;
	debug << " FifoIn_Size=" << FifoIn_Size << " ipu0_data=" << decoder->ipu0_data;
	debug << " FifoOut_Size=" << FifoOut_Size;
	//debug << " DMA4STR=" << Dma::_DMA->DmaCh [ 4 ].CHCR_Reg.STR;
	debug << " CurrentOp=" << dec << CurrentOp;
#endif

	// event is being handled here, so clear event
	Set_NextEventCycle ( -1ULL );



	//if ( ( CMD_Write.Value != -1 ) && ( !decoder->ipu0_data ) )
	if (CMD_Write.Value != -1)
	{
#ifdef INLINE_DEBUG_RUN
	debug << " IPURUNCMD";
	debug << " FifoIn_Size=" << FifoIn_Size;
	debug << " DMA4_CHCR.STR=" << Dma::pRegData [ 4 ]->CHCR.STR;
#endif

		// continue processing the current command
		Process_CMD ();
	}
	
	// done forcing output of data
	//CurrentOp = CURRENTOP_NONE;
	
}


bool IPU::FifoIn_Empty ()
{
	s32 sFifoSize32;
	
	// index is the index into a 64-bit unit rather than 128-bit QWs
	sFifoSize32 = FifoIn_WriteIndex - FifoIn_ReadIndex;
	
	if ( CMD_Write.Value != -1 )
	{
		// if there is a command running, then 2 QWs are probably in the internal input fifo
		// which would be 4 64-bit values
		sFifoSize32 -= 4;
	}
	
	// if the size is zero or less, then fifo is empty
	if ( sFifoSize32 <= 0 ) return true;
	
	return false;
}


bool IPU::FifoOut_Empty ()
{
	if ( FifoOut_Size )
	{
		return false;
	}
	
	return true;
}



void IPU::Process_CMD ()
{
#ifdef INLINE_DEBUG_PROCESS
	debug << "\r\nIPU::Process_CMD";
	debug << " CMD=" << hex << CMD_Write.Value;
#endif

	bool bRet;
	u32 TransferTotal, TransferLeft;
	u64 ullProcessingTime_Cycles;
	
	// check if macro block has been decoded yet //
	//if ( !decoder->ipu0_data )
	do
	{
#ifdef INLINE_DEBUG_PROCESS
	debug << " DecodeMacro";
	debug << " State=" << dec << CommandState;
	debug << " " << dec << ipu_cmd.pos [ 0 ];
#endif

		// macro block has not been decoded yet //
		
		// check that there is actually a command to process
		if ( CMD_Write.Value == -1 )
		{
#ifdef INLINE_DEBUG_PROCESS
	debug << " NONE";
#endif

			// there is no command to process
			return;
		}
		
		// go ahead and decode ALL AT ONCE (for now) (works fine for ps1?) //
		//do
		//{
#ifdef INLINE_DEBUG_PROCESS
	debug << " Execute";
#endif

			// execute the command
			bRet = Execute_CMD ();
			
			Update_IFC ();
			
			// check if the command was successful
			/*
			//if ( !bRet )
			if ( !bRet && !decoder->ipu0_data )
			{
#ifdef INLINE_DEBUG_PROCESS
	debug << " MoreDataNeeded??";
	debug << " State=" << dec << " " << CommandState << " " << ipu_cmd.pos [ 0 ] << " " << ipu_cmd.pos [ 1 ] << " " << ipu_cmd.pos [ 2 ] << " " << ipu_cmd.pos [ 3 ] << " " << ipu_cmd.pos [ 4 ] << " " << ipu_cmd.pos [ 5 ];
	debug << " OutputDataReady(QW)=" << dec << decoder->ipu0_data;
	debug << " FifoIn_Size=" << dec << FifoIn_Size;
	debug << " FifoIn_ReadIndex=" << FifoIn_ReadIndex;
	debug << " FifoIn_WriteIndex=" << FifoIn_WriteIndex;
#endif

				// the command was not successful, meaning it wants more data ??
				
				// input QWs
				// request a transfer from DMA channel#4 (IPUin)
				//Dma::_DMA->Transfer ( 4 );
				
				// determine the number of cycles used to process data
				//ullProcessingTime_Cycles = 16;
				
				// set an event to occur after data is done processing
				//SetNextEvent ( 16 );
				
				// set the cycle device is busy until
				//BusyUntil_Cycle = NextEvent_Cycle;
				
				Update_IFC ();
				
				// try forcing it to input the data
				CurrentOp = CURRENTOP_INPUT;
				
				// done until event hits, then it should see input fifo is empty and reload it
				return;
			}
			*/
			
		//} while ( !bRet && !decoder->ipu0_data );
		
		
		
		
		// *** TODO *** instead of while loop, need to schedule an event to transfer more data in if there is no data to go out
		// so if reason we reached here is that there is no outgoing data ready, and macroblock not complete, then
		// we need to schedule more incoming data for later (can't schedule for now since...
		
		

		// check if data needs to be output
		if ( decoder->ipu0_data )
		{
			// determine the number of cycles used to process data
			ullProcessingTime_Cycles = ( (u64) decoder->ipu0_data ) * c_llCyclesPerQW;
		
#ifdef INLINE_DEBUG_PROCESS
	debug << " Cycles=" << dec << ullProcessingTime_Cycles;
#endif

			// set an event to occur after data is done processing
			//SetNextEvent ( ullProcessingTime_Cycles );
			
			// set the cycle device is busy until
			//BusyUntil_Cycle = NextEvent_Cycle;
			BusyUntil_Cycle = *_DebugCycleCount + ullProcessingTime_Cycles;
			
			
			// *todo*: set the start cycle for the output
			// instead just now says data isn't ready yet in ready function
			//Dma::_DMA->DmaCh [ 3 ].ullStartCycle = BusyUntil_Cycle + 1;
			
			// update output fifo size
			Update_OFC ();
			
			// try forcing it to output the data
			CurrentOp = CURRENTOP_OUTPUT;

			// set an event to occur after data is done processing
			//SetNextEvent ( ullProcessingTime_Cycles );
			
			// current macro block at this point is completely decocded //
			// can be output now (for now) //


			
			// wait until it is time to output data
			return;
		}
		
		
		if ( bRet )
		{
#ifdef INLINE_DEBUG_PROCESS
	debug << " MacroComplete";
	debug << " State=" << dec << CommandState;
	debug << " " << dec << ipu_cmd.pos [ 0 ];
	debug << " (before)MBC=" << dec << MacroBlock_Count;
#endif

			MacroBlock_Count--;
		
#ifdef INLINE_DEBUG_PROCESS
	debug << " (after)MBC=" << dec << MacroBlock_Count;
#endif

			// check if all macro blocks have been decoded //
			if ( MacroBlock_Count <= 0 )
			{
#ifdef INLINE_DEBUG_PROCESS
	debug << " CommandDone";
	debug << " State=" << dec << CommandState;
#endif


				// mark command complete before the data is actually transferred out
				// command complete
				Clear_Busy ();
				
				// no more command to process
				CMD_Write.Value = -1;
			
#ifndef DISABLE_INTERRUPTS
				// trigger interrupt
				SetInterrupt ();
#endif

				// done with all blocks
				return;

			}
			else
			{
				// there is still data left to process //
				
				// there is no more data left to output, but there is more data to process, so need to input
				// wait for event 
				
				// clear state
				CommandState = 0;
				
				// wait for input data
				// set an event to occur after data is done processing
				//SetNextEvent ( 16 );
				
				// set the cycle device is busy until
				//BusyUntil_Cycle = NextEvent_Cycle;
				
				//Update_IFC ();
				
				// try forcing it to input the data
				CurrentOp = CURRENTOP_INPUT;
				
				// done until event hits, then it should see input fifo is empty and reload it
				//return;
			}
		}
		
		
		// check if we need to wait
		// doesn't matter since decoder->ipu0_data would be zero
		//if ( 
		
		
		// need to wait for macro block to finish processing
		//return;
	} while ( bRet );
	
	/*
	// check if data needs to be output
	if ( decoder->ipu0_data )
	{
#ifdef INLINE_DEBUG_PROCESS
	debug << " DataReadyForOutput";
#endif

		// get the number of QWs that need to be transferred
		//TransferTotal = decoder->ipu0_data;
		
		// output the macro block
		// request a transfer from DMA channel#3 (IPUout)
		//Dma::_DMA->Transfer ( 3 );
		
		// determine the number of cycles used to process data
		ullProcessingTime_Cycles = ( (u64) decoder->ipu0_data ) * c_llCyclesPerQW;
	

		// set an event to occur after data is done processing
		//SetNextEvent ( ullProcessingTime_Cycles );
		
		// set the cycle device is busy until
		//BusyUntil_Cycle = NextEvent_Cycle;
		BusyUntil_Cycle = *_DebugCycleCount + ullProcessingTime_Cycles;
		
		// update output fifo size
		Update_OFC ();
		
		// try forcing it to output the data
		CurrentOp = CURRENTOP_OUTPUT;
		
		// wait until it is time to output data
		return;
	}
	*/
}


bool IPU::Execute_CMD ()
{

	if ( CMD_Write.Value == -1 )
	{
		// last command has already been executed
		// return true for command complete
		return true;
	}
	
	// check what command it is
	switch ( CMD_Write.CODE )
	{
		// BCLR - clears the input fifo
		case CMD_BCLR:
#ifdef INLINE_DEBUG_CMD
	debug << "; CMD: BCLR";
#endif
			
			//_IPU->Clear_InputFifo ();
			break;
			
		case CMD_IDEC:
#ifdef INLINE_DEBUG_CMD
	debug << "; CMD: IDEC";
#endif

			return Execute_IDEC ();
			break;
			
		case CMD_BDEC:
#ifdef INLINE_DEBUG_CMD
	debug << "; CMD: BDEC";
#endif

			return Execute_BDEC ();
			break;
			
		case CMD_VDEC:
#ifdef INLINE_DEBUG_CMD
	debug << "; CMD: VDEC";
#endif

			return Execute_VDEC ();
			break;
			
		case CMD_FDEC:
#ifdef INLINE_DEBUG_CMD
	debug << "; CMD: FDEC";
#endif

			// set as busy ??
			//Clear_Busy ();
			
			// clear busy after delay ?
			//SetNextEvent ( FDEC_CYCLES );
			
			// attempt to execute command
			return Execute_FDEC ();

			break;
			
		case CMD_SETIQ:
#ifdef INLINE_DEBUG_CMD
	debug << "; CMD: SETIQ";
#endif

			// get the bits
			if ( _IPU->CMD_Write.IQM )
			{
				//_IPU->Load_IQTable_FromBitstream ( _IPU->niq_table );
				_IPU->Load_IQTable_FromBitstream(decoder->niq);
			}
			else
			{
				//_IPU->Load_IQTable_FromBitstream ( _IPU->iq_table );
				_IPU->Load_IQTable_FromBitstream(decoder->iq);
			}
			
			break;
			
		case CMD_SETVQ:
#ifdef INLINE_DEBUG_CMD
	debug << "; CMD: SETVQ";
#endif

			// vq table is only 2 QWs
			_IPU->Load_VQTable_FromBitstream ( _IPU->vqclut );
			
			break;
			
		case CMD_CSC:
#ifdef INLINE_DEBUG_CMD
	debug << "; CMD: CSC";
#endif

			
			return Execute_CSC ();
			break;
			
		case CMD_PACK:
#ifdef INLINE_DEBUG_CMD
	debug << "; CMD: PACK";
#endif

			break;
			
		case CMD_SETTH:
#ifdef INLINE_DEBUG_CMD
	debug << "; CMD: SETTH";
#endif

			break;
			
		default:
#ifdef INLINE_DEBUG_CMD
	debug << "; CMD: Unknown";
#endif
		
			cout << "\nhps2x64: ALERT: IPU: Unknown IPU command: " << dec << _IPU->CMD_Write.CODE;
			break;
	};

	return true;
}


// dma#4
u64 IPU::DMA_Write_Ready ()
{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "\r\nIPU::DMA_Write_Ready";
	debug << " IFC=" << _IPU->Regs.CTRL.IFC;
#endif

	if ( _IPU->Regs.CTRL.IFC )
	{
		// if the input fifo is not empty, do not refill
		return 0;
	}
	
	// input fifo is empty
	return 1;
}


// dma#3
u64 IPU::DMA_Read_Ready ()
{
#ifdef INLINE_DEBUG_DMA_READ
	debug << "\r\nIPU::DMA_Read_Ready";
	debug << " CYCLE#" << dec << *_DebugCycleCount;
	debug << " FifoOut_Size=" << _IPU->FifoOut_Size;
	debug << " ipu0_data=" << decoder->ipu0_data;
	debug << " BusyUntil_Cycle=" << dec << _IPU->BusyUntil_Cycle;
#endif

	// if the data hasn't been processed yet, then it can't be sent out
	/*
	if ( ( *_DebugCycleCount ) < _IPU->BusyUntil_Cycle )
	{
		_IPU->Set_NextEventCycle( _IPU->BusyUntil_Cycle );
		return false;
	}
	*/

	if ( _IPU->FifoOut_Size )
	{
#ifdef INLINE_DEBUG_DMA_READ
	debug << " READY";
#endif

		// if there is anything in the output fifo, then it needs to be transferred
		// but only transferred after it has been processed
		//return true;
		return _IPU->BusyUntil_Cycle;
	}

#ifdef INLINE_DEBUG_DMA_READ
	debug << " NOT-READY";
#endif
	
	// there is no data in output fifo
	return 0;
}


// dma#4 - toIPU
u32 IPU::DMA_WriteBlock ( u64* Data, u32 QuadwordCount )
{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "\r\n\r\nIPU::DMA_WriteBlock " << hex << setw ( 8 ) << *_DebugPC << " QWC=" << QuadwordCount << " " << dec << *_DebugCycleCount << hex << "; Data= ";
	for ( int i = 0; i < ( QuadwordCount * 2 ); i++ ) debug << " " << Data [ i ];
	//debug << "\r\n";
#endif

#ifdef ENABLE_FDEC_ON_BCLR
	bool bFirstTransfer = false;
	u32 ulData;
	
	if ( !_IPU->FifoIn_WriteIndex )
	{
		bFirstTransfer = true;
	}
#endif

	// write the data into the input fifo
	for ( int i = 0; i < QuadwordCount; i++ )
	{
		_IPU->FifoIn [ _IPU->FifoIn_WriteIndex++ & c_iFifoMask64 ] = *Data++;
		_IPU->FifoIn [ _IPU->FifoIn_WriteIndex++ & c_iFifoMask64 ] = *Data++;
	}
	
	// update the input fifo size
	_IPU->Update_IFC ();


#ifdef ENABLE_FDEC_ON_BCLR
	if ( bFirstTransfer )
	{
		// set the output word like fdec ??
		ulData = _IPU->PeekBE ( 32, _IPU->BitPosition );
		//ulData = (u32)(_IPU->ReadBE64(_IPU->BitPosition)>>32);

		_IPU->Set_Output ( ulData );
		
		// set TOP register as the next 32-bits in the bitstream ??
		//_IPU->Regs.TOP.BSTOP = ulData;
		
		// no longer busy with command
		//_IPU->Clear_Busy ();
		
		// clear the command
		//_IPU->CMD_Write.Value = -1;
	}
#endif

	
	// check for pending command
	//if ( ( _IPU->CMD_Write.Value != -1 ) && ( !decoder->ipu0_data ) )
	if (_IPU->CMD_Write.Value != -1)
	{
		switch ( _IPU->CMD_Write.CODE )
		{
			case CMD_FDEC:
				_IPU->Execute_FDEC ();
				break;
				
			case CMD_VDEC:
				_IPU->Execute_VDEC ();
				break;
				
			case CMD_BDEC:
			case CMD_IDEC:
			case CMD_CSC:
				_IPU->Process_CMD ();
				break;
				
			case CMD_SETIQ:
			case CMD_SETVQ:
				_IPU->Execute_CMD ();
				break;
		};

		// if there is a pending command, then execute it
		//_IPU->Execute_CMD ();
	}
	
	// return the amount of data written
	return QuadwordCount;
}


// dma#3 - fromIPU
u32 IPU::DMA_ReadBlock ( u64* Data, u32 QuadwordCount )
{
#ifdef INLINE_DEBUG_DMA_READ
	debug << "\r\n\r\nIPU::DMA_ReadBlock " << hex << setw ( 8 ) << *_DebugPC << " QWC=" << QuadwordCount << " " << dec << *_DebugCycleCount << hex << "; Data= ";
	debug << " FifoOut_Size=" << _IPU->FifoOut_Size;
	debug << " ipu0_data=" << decoder->ipu0_data;
	debug << "\r\n";
#endif

	u64* OutputData;
	
#ifdef USE_FIFO_OUT_SIZE_FOR_TRANSFER
	// check if the amount to read is greater than amount of data in output fifo
	if ( QuadwordCount > _IPU->FifoOut_Size )
	{
		QuadwordCount = _IPU->FifoOut_Size;
	}
#else
	// transfer all the data to be output at once
	if ( QuadwordCount > decoder->ipu0_data )
	{
		QuadwordCount = decoder->ipu0_data;
	}
#endif
	
	// get a pointer into data to read
	OutputData = decoder->GetIpuDataPtr();
	
	// need to actually write the data
	for ( int i = 0; i < QuadwordCount; i++ )
	{
		*Data++ = *OutputData++;
		*Data++ = *OutputData++;
	}
	
	// the data in output fifo has been read
	decoder->AdvanceIpuDataBy( QuadwordCount );
	
	// update the output fifo size
	_IPU->Update_OFC ();

	//if ( ( _IPU->CMD_Write.Value != -1 ) && ( !decoder->ipu0_data ) )
	if (_IPU->CMD_Write.Value != -1)
	{
		_IPU->Process_CMD ();
	}
	
	
	// return the amount of data written
	return QuadwordCount;
}





u64 IPU::Read ( u32 Address, u64 Mask )
{
#ifdef INLINE_DEBUG_READ
	debug << "\r\nIPU::Read " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address;
#endif

#ifdef VERBOSE_IPU_READ
	cout << "\nIPU::Read " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address;
#endif

	u64 Output;
	

	switch ( Address )
	{
		case IPU_CMD:
		
#ifdef INLINE_DEBUG_READ
	debug << "; CMD";
#endif

			Output = _IPU->CMD_Read.Value;
			
#ifdef INLINE_DEBUG_READ_NOTBUSY
if ( !_IPU->CMD_Read.BUSY )
	debug << "\r\nIPU::Read " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << "; CMD" << " Output=" << Output;
#endif

			break;
			
		case IPU_CTRL:
#ifdef INLINE_DEBUG_READ
	debug << "; CTRL";
#endif

			// set coded block pattern ??
			_IPU->Regs.CTRL.CBP = _IPU->coded_block_pattern;
			
			Output = _IPU->Regs.CTRL.Value;
			
#ifdef INLINE_DEBUG_READ_NOTBUSY
if ( !_IPU->Regs.CTRL.BUSY )
	debug << "\r\nIPU::Read " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << "; CTRL" << " Output=" << Output;
#endif

			break;
			
		case IPU_BP:
#ifdef INLINE_DEBUG_READ
	debug << "; BP";
#endif

			Output = _IPU->Regs.BP.Value;
			
#ifdef INLINE_DEBUG_READ_NOTBUSY
if ( !_IPU->Regs.CTRL.BUSY )
	debug << "\r\nIPU::Read " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << "; BP" << " Output=" << Output;
#endif

			break;

		case IPU_TOP:
#ifdef INLINE_DEBUG_READ
	debug << "; TOP";
#endif

			Output = _IPU->Regs.TOP.Value;
			
#ifdef INLINE_DEBUG_READ_NOTBUSY
if ( !_IPU->Regs.TOP.BUSY )
	debug << "\r\nIPU::Read " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << "; TOP" << " Output=" << Output;
#endif

			break;
			
		case IPU_FIFOout:
#ifdef INLINE_DEBUG_READ
	debug << "; FIFOout";
#endif


			if ( !Mask )
			{
				// 128-bit read from FIFOout
				DataOut [ 0 ] = _IPU->FifoOut [ _IPU->FifoOut_ReadIndex++ ];
				_IPU->FifoOut_ReadIndex &= c_iFifoMask64;
				DataOut [ 1 ] = _IPU->FifoOut [ _IPU->FifoOut_ReadIndex++ ];
				_IPU->FifoOut_ReadIndex &= c_iFifoMask64;
				
				_IPU->FifoOut_Size--;
				
				if ( _IPU->FifoOut_Size < 0 )
				{
					cout << "\nhps2x64: IPU: ALERT: FifoOut_Size < 0";
					
					// set to min
					_IPU->FifoOut_Size = 0;
				}
				
				
				_IPU->Regs.CTRL.OFC = _IPU->FifoOut_Size;
				//_IPU->Regs.BP.IFC = _IPU->FifoIn_Size;
			}
			else
			{
				cout << "\nhps2x64: IPU: ALERT: non-128bit read from IPU FIFOout";
			}
			
			break;

		// WRITE ONLY
		/*
		case IPU_FIFOin:
#ifdef INLINE_DEBUG_READ
	debug << "; FIFOin";
#endif

			Output = 0;
			break;
		*/

			
		default:
#ifdef INLINE_DEBUG_READ
			debug << "; Invalid";
#endif
		
			// invalid IPU Register
			cout << "\nhps2x64 ALERT: Unknown IPU READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
			break;
	};
	
#ifdef INLINE_DEBUG_READ
	debug << "; Output =" << hex << Output;
#endif

#ifdef VERBOSE_IPU_READ
	debug << "; Output =" << hex << Output;
#endif

	return Output;
}


void IPU::Clear_InputFifo ()
{
	FifoIn_ReadIndex = 0;
	FifoIn_WriteIndex = 0;
	FifoIn_Size = 0;
	
	// clear bit position
	BitPosition = 0;

	// should also clear command read since fifo is clear
	CMD_Read.Lo = 0;
	
	// update input fifo
	Update_IFC ();
	
	// update bp
	Update_BP ();
}

void IPU::Set_Busy ()
{
	Regs.CTRL.BUSY = 1;
	Regs.TOP.BUSY = 1;
	CMD_Read.BUSY = 1;
}

void IPU::Clear_Busy ()
{
	Regs.CTRL.BUSY = 0;
	Regs.TOP.BUSY = 0;
	CMD_Read.BUSY = 0;
}


void IPU::Proceed ( u32 iBits )
{
#ifdef INLINE_DEBUG_PROCEED
	debug << "\r\nIPU::Proceed";
	debug << " iBits=" << dec << iBits;
	debug << " BitPosition=" << dec << BitPosition;
	debug << " (before)FifoIn_ReadIndex=" << dec << FifoIn_ReadIndex;
#endif

	u32 Prev_BitPosition;
	
	Prev_BitPosition = BitPosition;
	
#ifdef INLINE_DEBUG_PROCEED
	debug << " Prev_BitPosition=" << dec << Prev_BitPosition;
#endif

	// proceed the specified number of bits in the bit stream
	BitPosition += iBits;
	
	if ( Prev_BitPosition < 64 && BitPosition >= 64 )
	{
		FifoIn_ReadIndex++;
	}
	
	// if we end up in the next quadword, then set correct bit position and advance the qw we are on
	if ( BitPosition >= 128 )
	{
		// bit position should wrap around to the start of the next qw
		BitPosition &= 0x7f;
		
		// go to the next qw
		FifoIn_ReadIndex++;
	}
	
	// update BP register
	Update_BP ();
	
#ifdef INLINE_DEBUG_PROCEED
	debug << " (after)FifoIn_ReadIndex=" << dec << FifoIn_ReadIndex;
#endif
}


bool IPU::Load_IQTable_FromBitstream ( u8* table )
{
#ifdef INLINE_DEBUG_LOADIQ
	debug << "\r\nIPU::Load_IQTable_FromBitstream";
#endif

	u64 uTemp64;
	
	switch ( CommandState )
	{
		case 0:
			if ( !RESERVEBITS ( _IPU->CMD_Write.FB ) )
			{
				return false;
			}
			
			Proceed ( _IPU->CMD_Write.FB );
			
			// update ifc after proceed for now
			Update_IFC ();
			
			CommandState = 1;

			ipu_cmd_pos[0] = 0;
			
		case 1:
			for (int i = ipu_cmd_pos[0]; i < 8; i++)
			{
				if (!RESERVEBITS(64))
				{
					ipu_cmd_pos[0] = i;
					return false;
				}

				uTemp64 = Get(64);
				((u64*)table)[i] = uTemp64;
			}

			/*
			if ( !RESERVEBITS ( 256 ) )
			{
				return false;
			}

			for ( int i = 0; i < 4; i++ )
			{
				uTemp64 = Get ( 64 );
				((u64*) table) [ i ] = uTemp64;
				//((u64*) table) [ i ] = EndianSwap64 ( uTemp64 );
			}
			*/
			
			CommandState = 2;
			
		case 2:
			/*
			if ( !RESERVEBITS ( 256 ) )
			{
				return false;
			}
		
			for ( int i = 0; i < 4; i++ )
			{
				uTemp64 = Get ( 64 );
				((u64*) table) [ i ] = uTemp64;
				//((u64*) table) [ i ] = EndianSwap64 ( uTemp64 );
			}
			*/
			
			CommandState = 3;
			
		case 3:
			// no longer busy with command
			Clear_Busy ();
			
			// clear the command
			CMD_Write.Value = -1;
	
#ifndef DISABLE_INTERRUPTS
		// send interrupt now that command is complete
		SetInterrupt ();
#endif

			break;
	
	}	// end switch
	
	// executed succesfully
	return true;
}

// this table is only used for the VQ command
bool IPU::Load_VQTable_FromBitstream ( u16* table )
{
#ifdef INLINE_DEBUG_LOADVQ
	debug << "\r\nIPU::Load_VQTable_FromBitstream";
#endif

	u64 uTemp64;
	
	switch ( CommandState )
	{
		case 0:
			ipu_cmd_pos[0] = 0;
			CommandState = 1;
		case 1:
			for (int i = ipu_cmd_pos[0]; i < 4; i++)
			{
				if (!RESERVEBITS(64))
				{
					ipu_cmd_pos[0] = i;
					return false;
				}

				uTemp64 = Get(64);
				((u64*)table)[i] = uTemp64;
			}


			/*
			if ( !RESERVEBITS ( 256 ) )
			{
				return false;
			}
			
			for ( int i = 0; i < 4; i++ )
			{
				uTemp64 = Get ( 64 );
				((u64*) table) [ i ] = uTemp64;
				//((u64*) table) [ i ] = EndianSwap64 ( uTemp64 );
			}
			*/
			
			CommandState = 2;
			
		case 2:
			// no longer busy with command
			Clear_Busy ();
			
			// clear the command
			CMD_Write.Value = -1;
	
#ifndef DISABLE_INTERRUPTS
			// send interrupt now that command is complete
			SetInterrupt ();
#endif

			break;
	
	}	// end switch
	
	// executed succesfully
	return true;
}






// peeks at up to 64-bits from bit stream
u64 IPU::PeekBE ( u64 iBits, u32 uBitPosition )
{
#ifdef INLINE_DEBUG_PEEK
	debug << "\r\nIPU::PeekBE";
	debug << " iBits=" << dec << iBits;
	debug << " uBitPosition=" << dec << uBitPosition;
#endif

	// ***todo*** uninitialized variable issue - will research
	u64 uResult64 = 0;

	//u32 uBitsSkip;
	//s32 sBitsRemaining;
	u32 BytePosition;
	u32 LeftShift;
	u64 Mask, Shift;
	
	u32 LeftShift_Bits;
	
	u8 *ptr8;
	
	// get position of byte to start at
	BytePosition = uBitPosition >> 3;
	
	// the 64-bit block has 8 bytes
	BytePosition &= 0x7;
	
	// get pointer to start of data
	ptr8 = (u8*) ( & FifoIn [ FifoIn_ReadIndex & c_iFifoMask64 ] );
	
	// update pointer to the actual byte to start at
	ptr8 += BytePosition;
	
	// read in reverse order (big endian)
	//uResult64 = 0;
	for ( int i = 0; i < ( 8 - BytePosition ); i++ )
	{
		uResult64 <<= 8;
		uResult64 |= (u64) (*ptr8++);
	}
	
	ptr8 = (u8*)(&FifoIn[(FifoIn_ReadIndex + 1) & c_iFifoMask64]);

	if ( BytePosition )
	{
		for ( int i = 0; i < BytePosition; i++ )
		{
			uResult64 <<= 8;
			uResult64 |= (u64) (*ptr8++);
		}
	}
	
#ifdef INLINE_DEBUG_PEEK
	debug << " (MID)uResult64=" << hex << uResult64;
#endif
	
	// fit in the last byte
	if ( uBitPosition & 0x7 )
	{
		// shift left by the number of bits that have been dumped in the byte (8-bits in a byte)
		uResult64 <<= ( uBitPosition & 0x7 );
		
		// put in the last byte
		uResult64 |= ( (u64) (*ptr8) ) >> ( 8 - ( uBitPosition & 0x7 ) );
	}
	
	// shift right by 64 minus number of bits to read
	uResult64 >>= ( ( 64 - iBits ) & 0x3f );
	
	
	/*
	BytePosition = uBitPosition & ~7;
	
	// check which half of qw the bit position is currently in
	if ( BytePosition < 64 )
	{

		// get the number of bits to skip in the first 64-bit block
		//uBitsSkip = BytePosition;
		
		// should shift left though by 64 - byte position
		LeftShift = 64 - BytePosition;
		LeftShift_Bits = 64 - uBitPosition;
	}
	else
	{
		// get the number of bits to skip in the first 64-bit block
		//uBitsSkip = BytePosition - 64;
		
		// should shift left though by 128 - byte position
		LeftShift = 128 - BytePosition;
		LeftShift_Bits = 128 - uBitPosition;
	}

	// get the number of bits remaining in second 64-bit block if any
	//sBitsRemaining = iBits - LeftShift;
	
#ifdef INLINE_DEBUG_PEEK
	//debug << " uBitsSkip=" << dec << uBitsSkip;
	//debug << " sBitsRemaining=" << dec << sBitsRemaining;
	debug << " LeftShift=" << dec << LeftShift;
#endif

	// may only want to shift right by a max of 63
	BytePosition &= 0x3f;

	// shift right by the bit position
	uResult64 = FifoIn [ FifoIn_ReadIndex & c_iFifoMask64 ]  >> BytePosition;
	
	//if ( sBitsRemaining > 0 )
	if ( iBits > LeftShift_Bits )
	{
		//uResult64 |= FifoIn [ ( FifoIn_ReadIndex + 1 ) & c_iFifoMask64 ] << sBitsRemaining;
		uResult64 |= FifoIn [ ( FifoIn_ReadIndex + 1 ) & c_iFifoMask64 ] << LeftShift;
	}
	
	// if bitposition and byteposition are not the same then load is not byte aligned
	if ( BytePosition != uBitPosition )
	{
		Shift = uBitPosition & 0x7;
		
		Mask = 0xff >> Shift;
		Mask |= ( Mask << 8 );
		Mask |= ( Mask << 16 );
		Mask |= ( Mask << 32 );
		
		uResult64 = ( ( ~Mask & ( uResult64 >> 8 ) ) >> ( 8 - Shift ) ) | ( ( Mask & uResult64 ) << Shift );
	}
	
#ifdef INLINE_DEBUG_PEEK
	debug << " (before)Result=" << hex << uResult64;
#endif

	// remove any top bits that were not requested
	if ( iBits & 0x3f )
	{
		uResult64 &= ( ( 1ULL << iBits ) - 1ULL );
	}
	*/
	
#ifdef INLINE_DEBUG_PEEK
	//debug << " Mask0=" << hex << ( 1ULL << iBits );
	//debug << " Mask=" << hex << ( ( 1ULL << iBits ) - 1ULL );
	debug << " Fifo0=" << hex << FifoIn [ FifoIn_ReadIndex & c_iFifoMask64 ];
	debug << " Fifo1=" << hex << FifoIn [ ( FifoIn_ReadIndex + 1 ) & c_iFifoMask64 ];
	debug << " (final)Result=" << hex << uResult64;
#endif

	return uResult64;
}

// reads x number of bits from bitstream
u64 IPU::ReadBE64(u32 uBitPosition)
{
#ifdef INLINE_DEBUG_READBE
	debug << "\r\nIPU::ReadBE";
	debug << " iBits=" << dec << iBits;
	debug << " uBitPosition=" << dec << uBitPosition;
#endif

	// ***todo*** uninitialized variable issue - will research
	u64 uResult64 = 0;
	u64 uResult64_1;
	u64 uShift;

	//u32 uBitsSkip;
	//s32 sBitsRemaining;
	u32 BytePosition;
	u32 LeftShift;
	u64 Mask, Shift;

	u32 LeftShift_Bits;

	u8* ptr8;

	// get position of byte to start at
	BytePosition = uBitPosition >> 3;

	// the 64-bit block has 8 bytes
	BytePosition &= 0x7;

	// get pointer to start of data
	ptr8 = (u8*)(&FifoIn[FifoIn_ReadIndex & c_iFifoMask64]);

	// update pointer to the actual byte to start at
	ptr8 += BytePosition;

	// read in reverse order (big endian)
	//uResult64 = 0;
	for (int i = 0; i < (8 - BytePosition); i++)
	{
		uResult64 <<= 8;
		uResult64 |= (u64)(*ptr8++);
	}

	// might need to read from here again further down
	ptr8 = (u8*)(&FifoIn[(FifoIn_ReadIndex + 1) & c_iFifoMask64]);

	if (BytePosition)
	{
		for (int i = 0; i < BytePosition; i++)
		{
			uResult64 <<= 8;
			uResult64 |= (u64)(*ptr8++);
		}
	}

#ifdef INLINE_DEBUG_READBE
	debug << " (MID)uResult64=" << hex << uResult64;
#endif

	uShift = uBitPosition & 0x7;

	// fit in the last byte
	if (uShift)
	{
		// note: some of this is currently lifted from pcsx2
		// create mask
		Mask = 0xffull >> uShift;
		Mask = Mask | (Mask << 8) | (Mask << 16) | (Mask << 24) | (Mask << 32) | (Mask << 40) | (Mask << 48) | (Mask << 56);

		// shift left by the number of bits that have been dumped in the byte (8-bits in a byte)
		//uResult64 <<= (uBitPosition & 0x7);
		uResult64_1 = uResult64 << 8;

		// put in the last byte
		//uResult64 |= ((u64)(*ptr8)) >> (8 - (uBitPosition & 0x7));
		uResult64_1 |= ((u64)(*ptr8));

		uResult64 = ((~Mask & uResult64_1) >> (8 - uShift)) | (((Mask) & uResult64) << uShift);
	}
	else
	{
		// result is as is //
	}

	// shift right by 64 minus number of bits to read
	//uResult64 >>= ((64 - iBits) & 0x3f);



#ifdef INLINE_DEBUG_READBE
	//debug << " Mask0=" << hex << ( 1ULL << iBits );
	//debug << " Mask=" << hex << ( ( 1ULL << iBits ) - 1ULL );
	debug << " Fifo0=" << hex << FifoIn[FifoIn_ReadIndex & c_iFifoMask64];
	debug << " Fifo1=" << hex << FifoIn[(FifoIn_ReadIndex + 1) & c_iFifoMask64];
	debug << " (final)Result=" << hex << uResult64;
#endif

	return uResult64;
}


u64 IPU::ReadLE64(u32 uBitPosition)
{
#ifdef INLINE_DEBUG_READBE
	debug << "\r\nIPU::ReadBE";
	debug << " iBits=" << dec << iBits;
	debug << " uBitPosition=" << dec << uBitPosition;
#endif

	// ***todo*** uninitialized variable issue - will research
	u64 uResult64 = 0;
	u64 uResult64_1;
	u64 uShift;

	//u32 uBitsSkip;
	//s32 sBitsRemaining;
	u32 BytePosition;
	u32 LeftShift;
	u64 Mask, Shift;

	u32 LeftShift_Bits;

	u8* ptr8;

	// get position of byte to start at
	BytePosition = uBitPosition >> 3;

	// the 64-bit block has 8 bytes
	BytePosition &= 0x7;

	// get pointer to start of data
	ptr8 = (u8*)(&FifoIn[FifoIn_ReadIndex & c_iFifoMask64]);

	// update pointer to the actual byte to start at
	ptr8 += BytePosition;

	// read in normal order (little endian)
	uResult64 = 0;
	int k = 0;
	for (int i = 0; i < (8 - BytePosition); i++)
	{
		//uResult64 <<= 8;
		uResult64 |= ((u64)(*ptr8++)) << ( k << 3 );
		k++;
	}

	// might need to read from here again further down
	ptr8 = (u8*)(&FifoIn[(FifoIn_ReadIndex + 1) & c_iFifoMask64]);

	if (BytePosition)
	{
		for (int i = 0; i < BytePosition; i++)
		{
			//uResult64 <<= 8;
			uResult64 |= ((u64)(*ptr8++)) << (k << 3);
			k++;
		}
	}

#ifdef INLINE_DEBUG_READBE
	debug << " (MID)uResult64=" << hex << uResult64;
#endif

	uShift = uBitPosition & 0x7;

	// fit in the last byte
	if (uShift)
	{
		// note: some of this is currently lifted from pcsx2
		// create mask
		Mask = 0xffull >> uShift;
		Mask = Mask | (Mask << 8) | (Mask << 16) | (Mask << 24) | (Mask << 32) | (Mask << 40) | (Mask << 48) | (Mask << 56);

		// shift left by the number of bits that have been dumped in the byte (8-bits in a byte)
		//uResult64 <<= (uBitPosition & 0x7);
		uResult64_1 = uResult64 >> 8;

		// put in the last byte
		//uResult64 |= ((u64)(*ptr8)) >> (8 - (uBitPosition & 0x7));
		uResult64_1 |= ((u64)(*ptr8)) << (7 << 3);

		uResult64 = ((~Mask & uResult64_1) >> (8 - uShift)) | (((Mask)&uResult64) << uShift);
	}
	else
	{
		// result is as is //
	}

	// shift right by 64 minus number of bits to read
	//uResult64 >>= ((64 - iBits) & 0x3f);



#ifdef INLINE_DEBUG_READBE
	//debug << " Mask0=" << hex << ( 1ULL << iBits );
	//debug << " Mask=" << hex << ( ( 1ULL << iBits ) - 1ULL );
	debug << " Fifo0=" << hex << FifoIn[FifoIn_ReadIndex & c_iFifoMask64];
	debug << " Fifo1=" << hex << FifoIn[(FifoIn_ReadIndex + 1) & c_iFifoMask64];
	debug << " (final)Result=" << hex << uResult64;
#endif

	return uResult64;
}





// peeks at up to 64-bits from bit stream
u64 IPU::Peek ( u64 iBits, u32 uBitPosition )
{
#ifdef INLINE_DEBUG_PEEK
	debug << "\r\nIPU::Peek";
	debug << " iBits=" << dec << iBits;
	debug << " uBitPosition=" << dec << uBitPosition;
#endif

	u64 uResult64;
	//u32 uBitsSkip;
	//s32 sBitsRemaining;
	u32 BytePosition;
	u32 LeftShift;
	u64 Mask, Shift;
	
	u32 LeftShift_Bits;
	
	BytePosition = uBitPosition & ~7;
	
	// check which half of qw the bit position is currently in
	if ( BytePosition < 64 )
	{

		// get the number of bits to skip in the first 64-bit block
		//uBitsSkip = BytePosition;
		
		// should shift left though by 64 - byte position
		LeftShift = 64 - BytePosition;
		LeftShift_Bits = 64 - uBitPosition;
	}
	else
	{
		// get the number of bits to skip in the first 64-bit block
		//uBitsSkip = BytePosition - 64;
		
		// should shift left though by 128 - byte position
		LeftShift = 128 - BytePosition;
		LeftShift_Bits = 128 - uBitPosition;
	}

	// get the number of bits remaining in second 64-bit block if any
	//sBitsRemaining = iBits - LeftShift;
	
#ifdef INLINE_DEBUG_PEEK
	//debug << " uBitsSkip=" << dec << uBitsSkip;
	//debug << " sBitsRemaining=" << dec << sBitsRemaining;
	debug << " LeftShift=" << dec << LeftShift;
#endif

	// may only want to shift right by a max of 63
	BytePosition &= 0x3f;

	// shift right by the bit position
	uResult64 = FifoIn [ FifoIn_ReadIndex & c_iFifoMask64 ]  >> BytePosition;
	
	//if ( sBitsRemaining > 0 )
	if ( iBits > LeftShift_Bits )
	{
		//uResult64 |= FifoIn [ ( FifoIn_ReadIndex + 1 ) & c_iFifoMask64 ] << sBitsRemaining;
		uResult64 |= FifoIn [ ( FifoIn_ReadIndex + 1 ) & c_iFifoMask64 ] << LeftShift;
	}
	
	// if bitposition and byteposition are not the same then load is not byte aligned
	if ( BytePosition != uBitPosition )
	{
		Shift = uBitPosition & 0x7;
		
		Mask = 0xff >> Shift;
		Mask |= ( Mask << 8 );
		Mask |= ( Mask << 16 );
		Mask |= ( Mask << 32 );
		
		uResult64 = ( ( ~Mask & ( uResult64 >> 8 ) ) >> ( 8 - Shift ) ) | ( ( Mask & uResult64 ) << Shift );
	}
	
#ifdef INLINE_DEBUG_PEEK
	debug << " (before)Result=" << hex << uResult64;
#endif

	// remove any top bits that were not requested
	if ( iBits & 0x3f )
	{
		uResult64 &= ( ( 1ULL << iBits ) - 1ULL );
	}
	
#ifdef INLINE_DEBUG_PEEK
	debug << " Mask0=" << hex << ( 1ULL << iBits );
	debug << " Mask=" << hex << ( ( 1ULL << iBits ) - 1ULL );
	debug << " Fifo0=" << hex << FifoIn [ FifoIn_ReadIndex & c_iFifoMask64 ];
	debug << " Fifo1=" << hex << FifoIn [ ( FifoIn_ReadIndex + 1 ) & c_iFifoMask64 ];
	debug << " Result=" << hex << uResult64;
#endif

	return uResult64;
}

// grabs up to 64-bits from bit stream
u64 IPU::Get ( u32 iBits )
{
	u64 Result;
	u32 InputFifoSize;
	
	// get the bits from bit stream
	//Result = PeekBE ( iBits, BitPosition );
	//Result = ReadBE64(BitPosition)>>(64-iBits);
	Result = ReadLE64(BitPosition) >> (64 - iBits);
	
	// proceed in the bit stream
	Proceed ( iBits );
	
	// update IFC (input fifo counter) (CTRL and BP)
	//InputFifoSize = ( FifoIn_WriteIndex - FifoIn_ReadIndex + 1 ) >> 1;
	//Regs.CTRL.IFC = InputFifoSize;
	//Regs.BP.IFC = InputFifoSize;
	Update_IFC ();
	
	// check for error for now
	//if ( InputFifoSize > 8 )
	//{
	//	cout << "\nhps2x64: IPU: ALERT: InputFifo has invalid size = " << dec << InputFifoSize;
	//}
	
	return Result;
}





void IPU::Update_OFC ()
{
	// check if thedecoder has data to send
	if ( decoder->ipu0_data )
	{
		FifoOut_Size = decoder->ipu0_data;
		
		// maximum size of output fifo is 8
		if ( FifoOut_Size > 8 )
		{
			FifoOut_Size = 8;
		}
	}
	else
	{
		FifoOut_Size = 0;
	}
	
	// set OFC
	Regs.CTRL.OFC = FifoOut_Size;
	
	// check if output fifo is empty
	if ( FifoOut_Size )
	{
		// output fifo has data to send //
		
		// request a transfer from DMA channel#3 (IPUout)
		//Dma::_DMA->Transfer ( 3 );
		
		// output fifo is 8 QWs
		// continue what we were doing after data has transferred
		//SetNextEvent ( 8 );
		Dma::_DMA->CheckTransfer ();
	}
}

void IPU::Update_IFC ()
{
	FifoIn_Size = ( FifoIn_WriteIndex - FifoIn_ReadIndex + 1 ) >> 1;
	
#ifdef ORIGINAL_IFC_UPDATE
	// if there is a command running, then 2 QWs are in the internal fifo
	if ( CMD_Write.Value != -1 )
	{
		FifoIn_Size -= 2;
		
		if ( FifoIn_Size < 0 ) FifoIn_Size = 0;
		
		// set FP
		Regs.BP.FP = 2;
	}
	else
	{
		// no command is running, so no data in internal input fifo ??
		Regs.BP.FP = 0;
	}
#else
	if ( FifoIn_Size <= 0 )
	{
		FifoIn_Size = 0;
		Regs.BP.FP = 0;
	}
	else if ( FifoIn_Size == 1 )
	{
		FifoIn_Size = 0;
		Regs.BP.FP = 1;
	}
	else
	{
		FifoIn_Size = FifoIn_Size - 2;
		Regs.BP.FP = 2;
	}
#endif
	
	Regs.CTRL.IFC = FifoIn_Size;
	Regs.BP.IFC = FifoIn_Size;
	
	// check if input fifo is empty
	if ( !FifoIn_Size )
	{
		// input fifo is empty //
		
		// request a transfer from DMA channel#4 (intoIPU)
		//Dma::_DMA->Transfer ( 4 );

		// ***testing***
		// check if dma channel#4 is active
		if ( !Dma::_DMA->pRegData [ 4 ]->CHCR.STR )
		{
			//cout << "\nhps2x64: IPU: Data requested while DMA#4 IPU-IN is not started.\n";

#ifdef ENABLE_INTERRUPT_DATA_REQUEST
				// trigger interrupt
				SetInterrupt ();
#endif
		}
		
		// input fifo is 8 QWs
		// continue what we were doing after data has transferred
		//SetNextEvent ( 8 );
		Dma::_DMA->CheckTransfer ();
	}
}


void IPU::Update_BP ()
{
	Regs.BP.BP = BitPosition & 0x7f;
}

void IPU::Set_Output ( u32 Data )
{
	CMD_Read.DATA = Data;
	//Regs.TOP.BSTOP = Data;
}


bool IPU::Execute_IDEC ()
{
#ifdef INLINE_DEBUG_IDEC
	debug << "\r\nIPU::Execute_IDEC";
	//debug << " TBL=" << CMD_Write.TBL;
#endif

	switch ( CommandState )
	{
	
	case 0:

	if ( !RESERVEBITS ( CMD_Write.FB ) )
	{
#ifdef INLINE_DEBUG_IDEC
	debug << " RESERVEBITS FAILED!!!";
#endif

		return false;
	}
	
	
	// proceed FB bits
	Proceed ( CMD_Write.FB );

	// update ifc after proceed for now
	Update_IFC ();

	CommandState = 1;

	ipu_cmd_pos[0] = 0;
	ipu_cmd_pos[1] = 0;
	ipu_cmd_pos[2] = 0;
	ipu_cmd_pos[3] = 0;
	ipu_cmd_pos[4] = 0;
	ipu_cmd_pos[5] = 0;

	case 1:
	
	if (!mpeg2sliceIDEC()) return false;

	CommandState = 2;
	
	case 2:
		break;
		
	/*
	//ipuRegs.topbusy = 0;
	//ipuRegs.cmd.BUSY = 0;
	Regs.TOP.BUSY = 0;
	CMD_Read.BUSY = 0;
	
	Regs.CTRL.BUSY = 0;
	
	// clear the command
	CMD_Write.Value = -1;
	
#ifndef DISABLE_INTERRUPTS
	// send interrupt now that command is complete
	SetInterrupt ();
#endif
	*/
	
	}	// end switch

	return true;

}


bool IPU::Execute_BDEC ()
{
#ifdef INLINE_DEBUG_VDEC
	debug << "\r\nIPU::Execute_BDEC";
	//debug << " TBL=" << CMD_Write.TBL;
#endif

	switch ( CommandState )
	{
	
	case 0:

	if ( !RESERVEBITS ( CMD_Write.FB ) )
	{
#ifdef INLINE_DEBUG_VDEC
	debug << " RESERVEBITS FAILED!!!";
#endif

		return false;
	}
	
	
	// proceed FB bits
	Proceed ( CMD_Write.FB );

	// update ifc after proceed for now
	Update_IFC ();

	CommandState = 1;

	ipu_cmd_pos[0] = 0;
	ipu_cmd_pos[1] = 0;
	ipu_cmd_pos[2] = 0;
	ipu_cmd_pos[3] = 0;
	ipu_cmd_pos[4] = 0;
	ipu_cmd_pos[5] = 0;

	case 1:
	
	if (!mpeg2_slice()) return false;

	CommandState = 2;


	case 2:
		break;
		
	/*
	//ipuRegs.topbusy = 0;
	//ipuRegs.cmd.BUSY = 0;
	Regs.TOP.BUSY = 0;
	CMD_Read.BUSY = 0;
	
	Regs.CTRL.BUSY = 0;
	
	// clear the command
	CMD_Write.Value = -1;
	
#ifndef DISABLE_INTERRUPTS
	// send interrupt now that command is complete
	SetInterrupt ();
#endif
	*/
	
	}	// end switch

	return true;
}


bool IPU::Execute_VDEC ()
{
#ifdef INLINE_DEBUG_VDEC
	debug << "\r\nIPU::Execute_VDEC";
	debug << " TBL=" << CMD_Write.TBL;
#endif

#ifdef VERBOSE_IPU_VDEC
	cout << "\nIPU::Execute_VDEC";
	cout << " TBL=" << CMD_Write.TBL;
#endif

	u32 Data;
	
	switch ( CommandState )
	{
	
	case 0:
	
	if ( !RESERVEBITS ( CMD_Write.FB ) )
	{
#ifdef INLINE_DEBUG_VDEC
	debug << " RESERVEBITS FAILED!!!";
#endif

		return false;
	}
	
	
	// proceed FB bits
	Proceed ( CMD_Write.FB );

	// update ifc after proceed for now
	Update_IFC ();

	CommandState = 1;
	
	case 1:
	
	// the next command needs 16-bits
	// actually should check for 32 ?
	if ( !BITSTREAM_INIT () )
	{
#ifdef INLINE_DEBUG_VDEC
	debug << " RESERVEBITS FAILED!!!";
#endif

		return false;
	}
	
	// *** todo *** PS2 IPU has a special format where it ORs the length shifted left 16 to all these codes
	// *** todo *** looks like some codes are doing that but others are not ??
	// determine the TBL command
	switch ( CMD_Write.TBL )
	{
		// Macroblock Address Increment
		case 0:
#ifdef INLINE_DEBUG_VDEC
	debug << " MAI";
	debug << " MP1=" << Regs.CTRL.MP1;
#endif

#ifdef VERBOSE_IPU_VDEC
	cout << " MAI";
	cout << " MP1=" << Regs.CTRL.MP1;
#endif

			decoder->mpeg1 = Regs.CTRL.MP1;
			
			Data = get_macroblock_address_increment();
			CMD_Read.DATA = Data;
			
			break;
			
		// Macroblock Type
		case 1:
#ifdef INLINE_DEBUG_VDEC
	debug << " MBT";
	debug << " PCT=" << Regs.CTRL.PCT;
#endif

#ifdef VERBOSE_IPU_VDEC
	cout << " MBT";
	cout << " PCT=" << Regs.CTRL.PCT;
#endif

			decoder->frame_pred_frame_dct = 1;
			decoder->coding_type = Regs.CTRL.PCT;
			
			Data = get_macroblock_modes();
			CMD_Read.DATA = Data;
			break;
			
		// Motion Code
		case 2:
#ifdef INLINE_DEBUG_VDEC
	debug << " get_motion_delta";
#endif

#ifdef VERBOSE_IPU_VDEC
	cout << " get_motion_delta";
#endif

			Data = get_motion_delta(0);
			CMD_Read.DATA = Data;
			break;
			
		case 3:
#ifdef INLINE_DEBUG_VDEC
	debug << " get_dmv";
#endif

#ifdef VERBOSE_IPU_VDEC
	cout << " get_dmv";
#endif

			Data = get_dmv();
			CMD_Read.DATA = Data;
			break;
	}

#ifdef INLINE_DEBUG_VDEC
	debug << " Output=" << hex << Data;
#endif

#ifdef VERBOSE_IPU_VDEC
	cout << " Output=" << hex << Data;
#endif

	//ipuRegs.ctrl.ECD = (ipuRegs.cmd.DATA == 0);
	Regs.CTRL.ECD = ( Data == 0 );
	
	CommandState = 2;
	
	case 2:
	
	// the next part needs 32-bits
	if ( !RESERVEBITS ( 32 ) )
	{
#ifdef INLINE_DEBUG_VDEC
	debug << " RESERVEBITS FAILED!!!";
#endif

		return false;
	}
	
	
	// set TOP register as the next 32-bits in the bit stream
	Regs.TOP.BSTOP = PeekBE ( 32, BitPosition );
	//Regs.TOP.BSTOP = (u32)(ReadBE64(BitPosition) >> 32);
	
	// get 32-bits from FIFO
	//Data = EndianSwap32 ( Peek ( 32, BitPosition ) );
	//Set_Output ( Data );
	
	// no longer busy with command
	Clear_Busy ();
	
	// clear the command
	CMD_Write.Value = -1;
	
#ifndef DISABLE_INTERRUPTS
	// send interrupt now that command is complete
	SetInterrupt ();
#endif
	}	// end switch
	
	// executed succesfully
	return true;
}


bool IPU::Execute_FDEC ()
{
#ifdef INLINE_DEBUG_FDEC
	debug << "\r\nIPU::Execute_FDEC";
	debug << " UBITS(32)=" << hex << Peek ( 32, BitPosition );
#endif

	u32 Data;

	switch ( CommandState )
	{
	
	case 0:
	
	if ( !RESERVEBITS ( CMD_Write.FB ) )
	{
#ifdef INLINE_DEBUG_FDEC
	debug << " RESERVEBITS FAILED!!!";
#endif

		return false;
	}
	
	// proceed FB bits
	Proceed ( CMD_Write.FB );
	
	// update ifc after proceed for now
	Update_IFC ();
	
	CommandState = 1;
	
	case 1:
	

	if ( !RESERVEBITS ( 32 ) )
	{
#ifdef INLINE_DEBUG_FDEC
	debug << " RESERVEBITS FAILED!!!";
#endif

		return false;
	}
	
	CommandState = 2;
	
	case 2:
	
	// get 32-bits from FIFO
	Data = PeekBE ( 32, BitPosition );
	//Data = (u32)(ReadBE64(BitPosition)>>32);

	Set_Output ( Data );
	
	// set TOP register as the next 32-bits in the bitstream ??
	//Regs.TOP.BSTOP = EndianSwap32 ( Peek ( 32, BitPosition ) );
	Regs.TOP.BSTOP = Data;
	
	// no longer busy with command
	Clear_Busy ();
	
	// clear the command
	CMD_Write.Value = -1;

#ifdef INLINE_DEBUG_FDEC
	debug << " (after)TOP=" << hex << EndianSwap32( Data );
	debug << " (reverse)=" << Data;
#endif
	
#ifndef DISABLE_INTERRUPTS
	// send interrupt now that command is complete
	SetInterrupt ();
#endif
	
	};	// end switch
	
	// executed succesfully
	return true;
}


bool IPU::Execute_CSC ()
{
	u32 i, Count, Remaining;
	u64 *pData;

	u64 uTemp64;
	
	// input data from fifo (I'm going to assume the data is 128-bit aligned for now)
	// should be 24 quadwords of input per macroblock for CSC command
	
	switch ( CommandState )
	{
	case 0:
	
	// set the input to the mb8 macroblock for CSC command
	decoder->SetInputTo ( decoder->mb8 );
	
	// now we are in the middle of processing command
	CommandState = 1;

	ipu_cmd_pos[0] = 0;
	
	case 1:

		for (i = ipu_cmd_pos[0]; i < ((2 * 16) + (1 * 16)); i++)
		{
			if (!RESERVEBITS(64))
			{
				ipu_cmd_pos[0] = i;
				return false;
			}

			uTemp64 = Get(64);
			((u64*)decoder->mb8.b)[i] = uTemp64;
		}
	
	/*
	// get remaining amount of data to transfer
	Remaining = decoder->ipu0_data_input;
	
	// get amount of data in input fifo
	Count = ( FifoIn_WriteIndex - FifoIn_ReadIndex ) >> 1;
	
	// if the amount remaining is less than what is in input fifo, then adjust
	if ( Remaining < Count ) Count = Remaining;
	
	// get a pointer into the data to transfer into
	pData = decoder->GetIpuInputPtr ();
	
	// copy the data into macro block
	for ( i = 0; i < Count; i++ )
	{
		*pData++ = FifoIn [ FifoIn_ReadIndex++ & c_iFifoMask64 ];
		*pData++ = FifoIn [ FifoIn_ReadIndex++ & c_iFifoMask64 ];
	}
	
	// advance where to write the rest of the data to
	decoder->AdvanceIpuInputBy ( Count );
	
	// if we are not done transfering the data in, then request more data
	if ( decoder->ipu0_data_input ) return false;
	*/
	
	// need to convert input using CSC
	ipu_csc( decoder->mb8, decoder->rgb32, 0 );
	//ipu_csc( decoder->mb8, decoder->rgb32, CMD_Write.SGN );
	
	// check if we are to output as rgb16
	if ( CMD_Write.OFM )
	{
		// output as rgb16 //
		
		// convert
		ipu_dither( decoder->rgb32, decoder->rgb16, CMD_Write.DTE );
		
		// set correct output block
		decoder->SetOutputTo ( decoder->rgb16 );
	}
	else
	{
		// set correct output block
		decoder->SetOutputTo ( decoder->rgb32 );
	}

	
	// command is done until macro block is counted and state is reset
	CommandState = 2;
	
	case 2:
	
	return true;
	
	}

	return true;
}


void IPU::Write ( u32 Address, u64 Data, u64 Mask )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\nIPU::Write " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << "; Data=" << Data;
#endif

#ifdef VERBOSE_IPU_WRITE
	cout << "\nIPU::Write " << hex << *_DebugPC << " " << dec << *_DebugCycleCount << " Address=" << hex << setw ( 8 ) << Address << "; Data=" << Data;
#endif

	// need to know if a command executed successfully
	bool bRet;
	
	switch ( Address )
	{
		case IPU_CMD:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CMD";
#endif

			
			_IPU->CMD_Write.Value = Data;
			
			// ***todo*** check the command and start/perform it
			switch ( _IPU->CMD_Write.CODE )
			{
				// BCLR - clears the input fifo
				case CMD_BCLR:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CMD: BCLR";
#endif
					
					_IPU->Clear_InputFifo ();
					
					// set bit position
					_IPU->Proceed ( _IPU->CMD_Write.BP );
					
					// command complete
					_IPU->CMD_Write.Value = -1;

#ifndef DISABLE_INTERRUPTS_BCLR
					// all commands interrupt when done ??
					SetInterrupt();
#endif
					break;
					
				case CMD_IDEC:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CMD: IDEC";
#endif

#ifdef VERBOSE_IPU_IDEC
	cout << "\nIPUCMD: IDEC";
	//cout << " PCT=" << _IPU->Regs.CTRL.PCT;
	cout << " MP1=" << _IPU->Regs.CTRL.MP1;
	cout << " QST=" << _IPU->Regs.CTRL.QST;
	cout << " IVF=" << _IPU->Regs.CTRL.IVF;
	cout << " AS=" << _IPU->Regs.CTRL.AS;
	cout << " IDP=" << _IPU->Regs.CTRL.IDP;
	//cout << " DCR=" << _IPU->Regs.CTRL.DCR;
	//cout << " DT=" << _IPU->Regs.CTRL.DT;
	//cout << " MBI=" << _IPU->Regs.CTRL.MBI;
	//cout << " Cycle#" << dec << *_DebugCycleCount;
#endif

					// the max value for FB is 32 for this command
					if ( _IPU->CMD_Write.FB > 32 )
					{
						// problem
						cout << "\nhps2x64: IPU: IDEC: FB is greater than 32. Max is 32! FB=" << dec << _IPU->CMD_Write.FB;
					}

					// set as busy ??
					_IPU->Set_Busy ();
					
					//from IPU_CTRL
					_IPU->Regs.CTRL.PCT = I_TYPE; //Intra DECoding;)

					decoder->coding_type			= _IPU->Regs.CTRL.PCT;
					decoder->mpeg1				= _IPU->Regs.CTRL.MP1;
					decoder->q_scale_type		= _IPU->Regs.CTRL.QST;
					decoder->intra_vlc_format	= _IPU->Regs.CTRL.IVF;
					decoder->scantype			= _IPU->Regs.CTRL.AS;
					decoder->intra_dc_precision	= _IPU->Regs.CTRL.IDP;

					//from IDEC value
					decoder->quantizer_scale		= _IPU->CMD_Write.QSC;
					decoder->frame_pred_frame_dct= !_IPU->CMD_Write.DTD;
					decoder->sgn = _IPU->CMD_Write.SGN;
					decoder->dte = _IPU->CMD_Write.DTE;
					decoder->ofm = _IPU->CMD_Write.OFM;

					//other stuff
					decoder->dcr = 1; // resets DC prediction value
					
					// pcsx2 doesn't appear to do this for IDEC, so commenting out for now
					//memset ( &decoder->mb8, 0, sizeof( decoder->mb8 ) );
					//memset ( &decoder->mb16, 0, sizeof( decoder->mb16 ) );
					
					// init decoding states
					//ipu_cmd.pos [ 0 ] = 0;
					//ipu_cmd.pos [ 1 ] = 0;
					//ipu_cmd.pos [ 2 ] = 0;
					//ipu_cmd.pos [ 3 ] = 0;
					//ipu_cmd.pos [ 4 ] = 0;
					//ipu_cmd.pos [ 5 ] = 0;
					//_IPU->ipu_cmd.clear ();

					// try to execute command
					_IPU->MacroBlock_Count = 1;
					_IPU->CommandState = 0;
					//_IPU->Execute_IDEC ();
					_IPU->Process_CMD ();
					break;
					
				case CMD_BDEC:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CMD: BDEC";
#endif

#ifdef VERBOSE_IPU_BDEC
	cout << "\nIPUCMD: BDEC";
	cout << " PCT=" << _IPU->Regs.CTRL.PCT;
	cout << " MP1=" << _IPU->Regs.CTRL.MP1;
	cout << " QST=" << _IPU->Regs.CTRL.QST;
	cout << " IVF=" << _IPU->Regs.CTRL.IVF;
	cout << " AS=" << _IPU->Regs.CTRL.AS;
	cout << " IDP=" << _IPU->Regs.CTRL.IDP;
	cout << " DCR=" << _IPU->CMD_Write.DCR;
	cout << " DT=" << _IPU->CMD_Write.DT;
	cout << " MBI=" << _IPU->CMD_Write.MBI;
	//cout << " Cycle#" << dec << *_DebugCycleCount;
#endif

					// the max value for FB is 32 for this command
					if ( _IPU->CMD_Write.FB > 32 )
					{
						// problem
						cout << "\nhps2x64: IPU: BDEC: FB is greater than 32. Max is 32! FB=" << dec << _IPU->CMD_Write.FB;
					}

					// set as busy ??
					_IPU->Set_Busy ();
					
					// ripped from pcsx2
					//decoder->coding_type			= I_TYPE;
					decoder->coding_type		= _IPU->Regs.CTRL.PCT;
					decoder->mpeg1				= _IPU->Regs.CTRL.MP1;
					decoder->q_scale_type		= _IPU->Regs.CTRL.QST;
					decoder->intra_vlc_format	= _IPU->Regs.CTRL.IVF;
					decoder->scantype			= _IPU->Regs.CTRL.AS;
					decoder->intra_dc_precision	= _IPU->Regs.CTRL.IDP;

					//from BDEC value
					decoder->quantizer_scale		= ( decoder->q_scale_type ? ( non_linear_quantizer_scale [ _IPU->CMD_Write.QSC ] ) : ( _IPU->CMD_Write.QSC << 1 ) );
					decoder->frame_pred_frame_dct	= 1;
					decoder->dcr					= _IPU->CMD_Write.DCR;
					
					decoder->macroblock_modes		= ( _IPU->CMD_Write.DT ? DCT_TYPE_INTERLACED : 0 );
					decoder->macroblock_modes		|= ( _IPU->CMD_Write.MBI ? MACROBLOCK_INTRA : MACROBLOCK_PATTERN );

					//memzero_sse_a(decoder->mb8);
					//memzero_sse_a(decoder->mb16);
					//memset ( decoder->mb8.b, 0, sizeof( decoder->mb8 ) );
					//memset ( decoder->mb16.b, 0, sizeof( decoder->mb16 ) );
					
					// init decoding states
					//ipu_cmd.pos [ 0 ] = 0;
					//ipu_cmd.pos [ 1 ] = 0;
					//ipu_cmd.pos [ 2 ] = 0;
					//ipu_cmd.pos [ 3 ] = 0;
					//ipu_cmd.pos [ 4 ] = 0;
					//ipu_cmd.pos [ 5 ] = 0;
					//_IPU->ipu_cmd.clear ();
					
					// try to execute command
					_IPU->MacroBlock_Count = 1;
					_IPU->CommandState = 0;
					//_IPU->Execute_BDEC ();
					_IPU->Process_CMD ();
					break;
					
				case CMD_VDEC:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CMD: VDEC";
	debug << " Test=" << hex << _IPU->Peek ( 32, _IPU->BitPosition );
	debug << " TestReverse=" << hex << EndianSwap32 ( _IPU->Peek ( 32, _IPU->BitPosition ) );
#endif

					// the max value for FB is 32 for this command
					if ( _IPU->CMD_Write.FB > 32 )
					{
						// problem
						cout << "\nhps2x64: IPU: VDEC: FB is greater than 32. Max is 32! FB=" << dec << _IPU->CMD_Write.FB;
					}
					
					// set as busy ??
					_IPU->Set_Busy ();
					
					// try to execute command
					_IPU->CommandState = 0;
					bRet = _IPU->Execute_VDEC ();
					
					// if the command does not finish, then this should complete it
					if ( !bRet )
					{
						// backup cycle - to prevent skips for now
						// don't do this, since it is cool if done during a write/read
						//*_DebugCycleCount--;
						
						//Dma::_DMA->Transfer ( 4 );
						//Dma::_DMA->CheckTransfer ();
						
						// restore cycle - to prevent skips for now
						//*_DebugCycleCount++;
					}
					
					break;
					
				case CMD_FDEC:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CMD: FDEC";
	debug << " Test=" << hex << _IPU->Peek ( 32, _IPU->BitPosition );
	debug << " TestReverse=" << hex << EndianSwap32 ( _IPU->Peek ( 32, _IPU->BitPosition ) );
#endif

					// the max value for FB is 32 for this command
					if ( _IPU->CMD_Write.FB > 32 )
					{
						// problem
						cout << "\nhps2x64: IPU: FDEC: FB is greater than 32. Max is 32! FB=" << dec << _IPU->CMD_Write.FB;
					}
					
					// set as busy ??
					_IPU->Set_Busy ();
					
					// clear busy after delay ?
					//_IPU->SetNextEvent ( FDEC_CYCLES );
					
					// try to execute command
					_IPU->CommandState = 0;
					bRet = _IPU->Execute_FDEC ();
					
					// if the command does not finish, then this should complete it
					if ( !bRet )
					{
						// backup cycle - to prevent skips for now
						// don't do this, since it is cool if transfer started during a write/read
						//*_DebugCycleCount--;
						
						//Dma::_DMA->Transfer ( 4 );
						//Dma::_DMA->CheckTransfer ();
						
						// restore cycle - to prevent skips for now
						//*_DebugCycleCount++;
					}

					break;
					
				case CMD_SETIQ:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CMD: SETIQ";
#endif

					// the max value for FB is 32 for this command
					if ( _IPU->CMD_Write.FB > 32 )
					{
						// problem
						cout << "\nhps2x64: IPU: SETIQ: FB is greater than 32. Max is 32! FB=" << dec << _IPU->CMD_Write.FB;
					}
					
					// set as busy ??
					_IPU->Set_Busy ();
					
					// iq tables are only 4 QWs

					// iq tables are only 4 QWs
					_IPU->CommandState = 0;

					// proceed the specified number of bits in the bit stream
					//_IPU->Proceed ( _IPU->CMD_Write.FB );
					
					// get the bits
					if ( _IPU->CMD_Write.IQM )
					{
						//_IPU->Load_IQTable_FromBitstream ( _IPU->niq_table );
						bRet = _IPU->Load_IQTable_FromBitstream ( decoder->niq );
					}
					else
					{
						//_IPU->Load_IQTable_FromBitstream ( _IPU->iq_table );
						bRet = _IPU->Load_IQTable_FromBitstream ( decoder->iq );
					}

					// if the command does not finish, then this should complete it
					if ( !bRet )
					{
						//Dma::_DMA->Transfer ( 4 );
						//Dma::_DMA->CheckTransfer ();
					}
	
					// unsure if this should interrupt?
//#ifndef DISABLE_INTERRUPTS
//					// all commands interrupt when done ??
//					_IPU->SetInterrupt ();
//#endif
					break;
					
				case CMD_SETVQ:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CMD: SETVQ";
#endif

					// set as busy ??
					_IPU->Set_Busy ();
					
					// vq table is only 2 QWs
					_IPU->CommandState = 0;
					
					
					// vq table is only 2 QWs
					bRet = _IPU->Load_VQTable_FromBitstream ( _IPU->vqclut );

					// if the command does not finish, then this should complete it
					if ( !bRet )
					{
						//Dma::_DMA->Transfer ( 4 );
						//Dma::_DMA->CheckTransfer ();
					}
					
					// unsure if this should interrupt?
//#ifndef DISABLE_INTERRUPTS
//					// all commands interrupt when done ??
//					_IPU->SetInterrupt ();
//#endif
					break;
					
				case CMD_CSC:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CMD: CSC";
#endif

#ifdef VERBOSE_IPU_CSC
	cout << "\nIPUCMD: CSC";
	cout << " Cycle#" << dec << *_DebugCycleCount;
#endif

					// set as busy ??
					_IPU->Set_Busy ();
					
					// try to execute command
					_IPU->MacroBlock_Count = _IPU->CMD_Write.MBC;
					_IPU->CommandState = 0;
					//_IPU->Execute_BDEC ();
					_IPU->Process_CMD ();

					break;
					
				case CMD_PACK:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CMD: PACK";
#endif

#ifdef VERBOSE_IPU_PACK
	cout << "\nIPUCMD: PACK";
	cout << " Cycle#" << dec << *_DebugCycleCount;
#endif

#ifndef DISABLE_INTERRUPTS_PACK
					// all commands interrupt when done ??
					SetInterrupt();
#endif

					break;
					
				case CMD_SETTH:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CMD: SETTH";
#endif

					_IPU->TH0 = _IPU->CMD_Write.TH0;
					_IPU->TH1 = _IPU->CMD_Write.TH1;
					
					// command complete
					_IPU->CMD_Write.Value = -1;

#ifndef DISABLE_INTERRUPTS_SETTH
					// all commands interrupt when done ??
					SetInterrupt ();
#endif

					break;
					
				default:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CMD: Unknown";
#endif
				
					cout << "\nhps2x64: ALERT: IPU: Unknown IPU command: " << dec << _IPU->CMD_Write.CODE;
					break;
			};
			
#ifndef DISABLE_INTERRUPTS
			// all commands interrupt when done ??
			//_IPU->SetInterrupt ();
#endif
			
			break;
			
		case IPU_CTRL:
#ifdef INLINE_DEBUG_WRITE
	debug << "; CTRL";
#endif

			_IPU->Regs.CTRL.Value = ( _IPU->Regs.CTRL.Value & CTRL_Write_Mask ) | ( Data & ~CTRL_Write_Mask );
			
			// check if reset is requested
			if ( Data & 0x40000000 )
			{
#ifdef INLINE_DEBUG_WRITE
	debug << "; RESET";
#endif

				_IPU->Clear_InputFifo ();
				_IPU->coded_block_pattern = 0;
				_IPU->Regs.TOP.Value = 0;
				_IPU->CMD_Read.BUSY = 0;
				_IPU->Regs.CTRL.Value = 0;
				_IPU->Regs.BP.Value = 0;
			}
			
			break;
			
		// READ ONLY
		/*
		case IPU_BP:
#ifdef INLINE_DEBUG_WRITE
	debug << "; BP";
#endif


			break;

		case IPU_TOP:
#ifdef INLINE_DEBUG_WRITE
	debug << "; TOP";
#endif

			break;
			
			
		case IPU_FIFOout:
#ifdef INLINE_DEBUG_WRITE
	debug << "; FIFOout";
#endif

			break;
		*/
			

		case IPU_FIFOin:
#ifdef INLINE_DEBUG_WRITE
	debug << "; FIFOin";
#endif

			if ( !Mask )
			{
#ifdef INLINE_DEBUG_WRITE
	debug << " WriteIndex=" << dec << _IPU->FifoIn_WriteIndex;
	debug << " " << hex << ( (u64*) Data ) [ 0 ];
	debug << " " << hex << ( (u64*) Data ) [ 1 ];
#endif

#ifdef ENABLE_FDEC_ON_BCLR
				bool bFirstTransfer = false;
				u32 ulData;
				
				if ( !_IPU->FifoIn_WriteIndex )
				{
					bFirstTransfer = true;
				}
#endif


				// 128-bit write to FIFOin
				_IPU->FifoIn [ _IPU->FifoIn_WriteIndex & c_iFifoMask64 ] = ( (u64*) Data ) [ 0 ];
				_IPU->FifoIn_WriteIndex++;
				_IPU->FifoIn [ _IPU->FifoIn_WriteIndex & c_iFifoMask64 ] = ( (u64*) Data ) [ 1 ];
				_IPU->FifoIn_WriteIndex++;
				
				//_IPU->FifoIn_Size++;
				
				//_IPU->Regs.CTRL.IFC = _IPU->FifoIn_Size;
				//_IPU->Regs.BP.IFC = _IPU->FifoIn_Size;
				_IPU->Update_IFC ();

#ifdef ENABLE_FDEC_ON_BCLR
				if ( bFirstTransfer )
				{
					// set the output word like fdec ??
					ulData = _IPU->PeekBE ( 32, _IPU->BitPosition );
					//ulData = (u32)(_IPU->ReadBE64(_IPU->BitPosition)>>32);

					_IPU->Set_Output ( ulData );
					
					// set TOP register as the next 32-bits in the bitstream ??
					//Regs.TOP.BSTOP = EndianSwap32 ( Peek ( 32, BitPosition ) );
					//_IPU->Regs.TOP.BSTOP = ulData;
					
					// no longer busy with command
					//_IPU->Clear_Busy ();
					
					// clear the command
					//_IPU->CMD_Write.Value = -1;
				}
#endif

				
				if ( _IPU->FifoIn_Size > c_iDevice_FifoSize )
				{
					cout << "\nhps2x64: IPU: ALERT: FifoIn_Size > " << c_iDevice_FifoSize;
					
					// set to max
					//_IPU->FifoIn_Size = c_iDevice_FifoSize;
				}
				
			}
			else
			{
				cout << "\nhps2x64: IPU: ALERT: non-128bit write to IPU FIFOin";
			}
			
			break;
		

			
		default:
#ifdef INLINE_DEBUG_WRITE
			debug << "; Invalid";
#endif
		
			// invalid IPU Register
			cout << "\nhps2x64 ALERT: Unknown IPU WRITE @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data << "\n";
			break;
	};
	
}


// RGB16 -> 4-bit index in CLUT conversion
u8 IPU::VQ ( u16 RGB16 )
{
	s32 r, g, b;
	s32 dr, dg, db;
	u32 d1, Min_d1;
	u32 Index, MinIndex;
	u32 clut_r, clut_g, clut_b, clut_pixel;
	
	// split pixel into components
	r = ( RGB16 >> 10 ) & 0x1f;
	g = ( RGB16 >> 5 ) & 0x1f;
	b = ( RGB16 >> 0 ) & 0x1f;
	
	MinIndex = -1;
	Min_d1 = 0xfffff;
	
	for ( Index = 0; Index < c_iVqClut_Size; Index++ )
	{
		// get pixel from clut
		clut_pixel = vqclut [ Index ];
		
		// split components
		clut_r = ( clut_pixel >> 16 ) & 0xff;
		clut_g = ( clut_pixel >> 8 ) & 0xff;
		clut_b = ( clut_pixel >> 0 ) & 0xff;
		
		// difference
		dr = r - clut_r;
		dg = g - clut_g;
		db = b - clut_b;
		
		// absolute value of difference
		// note: not needed since they get squared next
		//dr = ( dr < 0 ) ? -dr : dr;
		//dg = ( dg < 0 ) ? -dg : dg;
		//db = ( db < 0 ) ? -db : db;
		
		// square and sum
		d1 = ( dr * dr ) + ( dg * dg ) + ( db * db );
		
		// check if we have a smaller value for d1
		if ( d1 < Min_d1 )
		{
			Min_d1 = d1;
			MinIndex = Index;
		}
	}
	
	return MinIndex;
}


// YCbCr -> r,g,b,a conversion
u32 IPU::CSC ( u32 YCbCr )
{
	u32 Y, Cb, Cr;
	s32 Cb2, Cb3, Cr2, Cr3;
	u32 Y2, Y3;
	s32 r0, g0, b0, a0;
	
	// split Y,Cb,Cr components
	Y = ( YCbCr >> 16 ) & 0xff;
	Cb = ( YCbCr >> 8 ) & 0xff;
	Cr = ( YCbCr >> 0 ) & 0xff;
	
	// remove bias of 128 from Cr,Cb
	Cr -= 128;
	Cb -= 128;
	
	// multiply by coefficient
	Cr2 = 0x0cc * Cr;
	Cr3 = 0x068 * Cr;
	Cb2 = 0x032 * Cb;
	Cb3 = 0x102 * Cb;
	
	// chop lower 6-bits ??
	Cr2 >>= 6;
	Cr3 >>= 6;
	Cb2 >>= 6;
	Cb3 >>= 6;
	
	// removed bias of 16 from Y
	Y -= 16;
	
	// multiply by coefficient
	Y2 = 0x095 * Y;
	
	// chop lower 6-bits ??
	Y2 >>= 6;
	
	// get r,g,b
	r0 = Y2 + Cr2;
	g0 = Y2 - Cb2 - Cr3;
	b0 = Y2 + Cb3;
	
	// rounded ??
	r0 = ( r0 >= 0 ) ? ( ( r0 >> 1 ) + ( r0 & 1 ) ) : ( ( r0 >> 1 ) - ( r0 & 1 ) );
	g0 = ( g0 >= 0 ) ? ( ( g0 >> 1 ) + ( g0 & 1 ) ) : ( ( g0 >> 1 ) - ( g0 & 1 ) );
	b0 = ( b0 >= 0 ) ? ( ( b0 >> 1 ) + ( b0 & 1 ) ) : ( ( b0 >> 1 ) - ( b0 & 1 ) );
	
	// clip to 8 bits
	r0 = ( r0 < 0 ) ? 0 : r0;
	g0 = ( g0 < 0 ) ? 0 : g0;
	b0 = ( b0 < 0 ) ? 0 : b0;
	r0 = ( r0 > 255 ) ? 255 : r0;
	g0 = ( g0 > 255 ) ? 255 : g0;
	b0 = ( b0 > 255 ) ? 255 : b0;
	
	// set alpha
	if ( r0 < TH0 && g0 < TH0 && b0 < TH0 )
	{
		r0 = 0;
		g0 = 0;
		b0 = 0;
		a0 = 0;
	}
	else if ( r0 < TH1 && g0 < TH1 && b0 < TH1 )
	{
		a0 = 0x40;
	}
	else
	{
		a0 = 0x80;
	}
	
	// reverse sign if SGN set
	if ( CMD_Write.SGN )
	{
		r0 ^= 0x80;
		g0 ^= 0x80;
		b0 ^= 0x80;
	}
	
	// compose r,g,b,a
	// return pixel
	return ( a0 << 24 ) + ( r0 << 16 ) + ( g0 << 8 ) + b0;
}


/* __fi */ void ipu_csc(macroblock_8& mb8, macroblock_rgb32& rgb32, int sgn)
{
	s32 Y;
	s32 Cb, Cr;
	s32 Cb2, Cb3, Cr2, Cr3;
	s32 Y2, Y3;
	s32 r0, g0, b0, a0;
	//u32 uRGB32;
	
	u32 th0, th1;

	int ix, iy;
	
	// get threshold values
	th0 = IPU::_IPU->TH0;
	th1 = IPU::_IPU->TH1;
	
	for ( iy = 0; iy < 16; iy++ )
	{
		for ( ix = 0; ix < 16; ix++ )
		{
			// get y,cb,cr
			Y = (u32) mb8.Y [ iy ] [ ix ];
			Cb = (s32) mb8.Cb [ iy >> 1 ] [ ix >> 1 ];
			Cr = (s32) mb8.Cr [ iy >> 1 ] [ ix >> 1 ];
			
			// remove bias of 128 from Cr,Cb
			Cr -= 128;
			Cb -= 128;
			
			// multiply by coefficient
			Cr2 = 0x0cc * Cr;
			Cr3 = 0x068 * Cr;
			Cb2 = 0x032 * Cb;
			Cb3 = 0x102 * Cb;
			
			// chop lower 6-bits ??
			Cr2 >>= 6;
			Cr3 >>= 6;
			Cb2 >>= 6;
			Cb3 >>= 6;
			
			// removed bias of 16 from Y
			Y -= 16;
			//Y = std::max( 0, Y );
			Y = ( Y < 0 ) ? 0 : Y;
			
			// multiply by coefficient
			Y2 = 0x095 * Y;
			
			// chop lower 6-bits ??
			Y2 >>= 6;
			
			// get r,g,b
			r0 = Y2 + Cr2;
			g0 = Y2 - Cb2 - Cr3;
			b0 = Y2 + Cb3;
			
			// rounded ??
			//r0 = ( r0 >= 0 ) ? ( ( r0 >> 1 ) + ( r0 & 1 ) ) : ( ( r0 >> 1 ) - ( r0 & 1 ) );
			//g0 = ( g0 >= 0 ) ? ( ( g0 >> 1 ) + ( g0 & 1 ) ) : ( ( g0 >> 1 ) - ( g0 & 1 ) );
			//b0 = ( b0 >= 0 ) ? ( ( b0 >> 1 ) + ( b0 & 1 ) ) : ( ( b0 >> 1 ) - ( b0 & 1 ) );
			r0 = ( r0 + 1 ) >> 1;
			g0 = ( g0 + 1 ) >> 1;
			b0 = ( b0 + 1 ) >> 1;
			
			// clip to 8 bits
			r0 = ( r0 < 0 ) ? 0 : r0;
			g0 = ( g0 < 0 ) ? 0 : g0;
			b0 = ( b0 < 0 ) ? 0 : b0;
			r0 = ( r0 > 255 ) ? 255 : r0;
			g0 = ( g0 > 255 ) ? 255 : g0;
			b0 = ( b0 > 255 ) ? 255 : b0;
			
			// set alpha
			if ( r0 < th0 && g0 < th0 && b0 < th0 )
			{
				r0 = 0;
				g0 = 0;
				b0 = 0;
				a0 = 0;
				//uRGB32 = 0;
			}
			else
			{
				//uRGB32 = ( r0 << 16 ) + ( g0 << 8 ) + b0;
				
				if ( r0 < th1 && g0 < th1 && b0 < th1 )
				{
					a0 = 0x40;
					//uRGB32 |= 0x40000000;
				}
				else
				{
					a0 = 0x80;
					//uRGB32 |= 0x80000000;
				}
			}
			
			// reverse sign if SGN set
			//if ( CMD_Write.SGN )
			if ( sgn )
			{
				r0 ^= 0x80;
				g0 ^= 0x80;
				b0 ^= 0x80;
				//uRGB32 ^= 0x00808080;
			}
			
			// compose r,g,b,a
			// return pixel
			//return ( a0 << 24 ) + ( r0 << 16 ) + ( g0 << 8 ) + b0;
			rgb32.c [ iy ] [ ix ].r = r0;
			rgb32.c [ iy ] [ ix ].g = g0;
			rgb32.c [ iy ] [ ix ].b = b0;
			rgb32.c [ iy ] [ ix ].a = a0;
		}
	}
}

u16 IPU::Dither ( u32 x, u32 y, u32 uPixel32 )
{
	static const u32 uPixelMask32 = ( 0xff << 1 );
	
	s32 sR32, sG32, sB32;
	s32 sDitherValue;
	u16 uPixel16;
	u32 uAlpha16;
	
	// get the r,g,b components in 8.1 fixed point unsigned format
	sR32 = ( uPixel32 >> 15 ) & uPixelMask32;
	sG32 = ( uPixel32 >> 7 ) & uPixelMask32;
	sB32 = ( uPixel32 << 1 ) & uPixelMask32;
	
	// get the dither value in 4.1 fixed point signed format
	sDitherValue = sDitherArray_4_1 [ ( x & c_uDitherArray_Mask ) + ( ( y & c_uDitherArray_Mask ) << 2 ) ];
	
	// add dither value to components
	sR32 += sDitherValue;
	sG32 += sDitherValue;
	sB32 += sDitherValue;
	
	// pixels need to be within the 0-255 range
	sR32 = ( sR32 < 0 ) ? 0 : sR32;
	sG32 = ( sG32 < 0 ) ? 0 : sG32;
	sB32 = ( sB32 < 0 ) ? 0 : sB32;
	sR32 = ( sR32 > 511 ) ? 511 : sR32;
	sG32 = ( sG32 > 511 ) ? 511 : sG32;
	sB32 = ( sB32 > 511 ) ? 511 : sB32;
	
	// round down (3 lower bits + 1 fractional bits = shift right 4 bits)
	sR32 >>= 4;
	sG32 >>= 4;
	sB32 >>= 4;
	
	// get alpha value
	uAlpha16 = ( ( uPixel32 >> 24 ) == 0x40 ) ? 1 : 0;
	
	// compose the pixel
	uPixel16 = ( sR32 << 10 ) | ( sG32 << 5 ) | ( sB32 );
	uPixel16 |= ( uAlpha16 << 15 );
	
	return uPixel16;
}


/* __fi */ void ipu_dither(const macroblock_rgb32& rgb32, macroblock_rgb16& rgb16, int dte)
{
	static const u32 uPixelMask32 = ( 0xff << 1 );
	
	s32 sR32, sG32, sB32;
	s32 sDitherValue;
	u16 uPixel16;
	u32 uAlpha16;
	u32 A32;
	
	int ix, iy;
	
	for ( iy = 0; iy < 16; iy++ )
	{
		for ( ix = 0; ix < 16; ix++ )
		{
			// get the r,g,b components in 8.1 fixed point unsigned format
			sR32 = rgb32.c [ iy ] [ ix ].r;
			sG32 = rgb32.c [ iy ] [ ix ].g;
			sB32 = rgb32.c [ iy ] [ ix ].b;
			sR32 <<= 1;
			sG32 <<= 1;
			sB32 <<= 1;
			
#ifdef ENABLE_IPU_DITHER
			// only dither if dithering is enabled
			if ( dte )
			{
				// get the dither value in 4.1 fixed point signed format
				sDitherValue = IPU::sDitherArray_4_1 [ ( ix & IPU::c_uDitherArray_Mask ) + ( ( iy & IPU::c_uDitherArray_Mask ) << 2 ) ];
				
				// add dither value to components
				sR32 += sDitherValue;
				sG32 += sDitherValue;
				sB32 += sDitherValue;
				
				// pixels need to be within the 0-255 range
				sR32 = ( sR32 < 0 ) ? 0 : sR32;
				sG32 = ( sG32 < 0 ) ? 0 : sG32;
				sB32 = ( sB32 < 0 ) ? 0 : sB32;
				sR32 = ( sR32 > 511 ) ? 511 : sR32;
				sG32 = ( sG32 > 511 ) ? 511 : sG32;
				sB32 = ( sB32 > 511 ) ? 511 : sB32;
			}
#endif
			
			// round down (3 lower bits + 1 fractional bits = shift right 4 bits)
			sR32 >>= 4;
			sG32 >>= 4;
			sB32 >>= 4;
			
			// get alpha value
			A32 = ( rgb32.c [ iy ] [ ix ].a == 0x40 ) ? 1 : 0;
			
			
			// compose the pixel
			//uPixel16 = ( sR32 << 10 ) | ( sG32 << 5 ) | ( sB32 );
			//uPixel16 |= ( uAlpha16 << 15 );
			//return uPixel16;
			rgb16.c [ iy ] [ ix ].r = sR32;
			rgb16.c [ iy ] [ ix ].g = sG32;
			rgb16.c [ iy ] [ ix ].b = sB32;
			rgb16.c [ iy ] [ ix ].a = A32;
			
		}
	}
}


// functions to interface with pcsx2 mpeg thedecoder //
u32 UBITS ( unsigned long Bits )
{
	u32 Result;
	u32 uBitPosition;
	
	uBitPosition = IPU::_IPU->BitPosition;
	
	Result = IPU::_IPU->PeekBE ( Bits, uBitPosition );

	return Result;
}

s32 SBITS ( unsigned long Bits )
{
	s32 Result;
	u32 uBitPosition;
	
	uBitPosition = IPU::_IPU->BitPosition;


	Result = UBITS( Bits );
	Result <<= ( 32 - Bits );
	Result >>= ( 32 - Bits );

	return Result;
}

void DUMPBITS ( unsigned long Bits )
{
	IPU::_IPU->Proceed ( Bits );
	
	// proceed must be followed with ifc update for now
	IPU::_IPU->Update_IFC ();
}

// gets bits via UBITS
u32 GETBITS(uint num)
{
	uint retVal = UBITS(num);
	
	// ***todo*** pcsx2 specific code being comment out for now
	//g_BP.Advance(num);
	DUMPBITS ( num );

	return retVal;
}



// returns false if unable to reserve the specified number of bits
bool RESERVEBITS ( uint num )
{
#ifdef INLINE_DEBUG_RESERVE
	IPU::debug << " RESERVEBITS";
	IPU::debug << " num=" << dec << num;
	IPU::debug << " bp=" << dec << IPU::_IPU->Regs.BP.BP;
	IPU::debug << " fp=" << dec << IPU::_IPU->Regs.BP.FP;
	IPU::debug << " ifc=" << dec << IPU::_IPU->Regs.BP.IFC;
#endif

	u32 RemainingBits, BlockCount, BlockBits;
	s32 sBlockCount;
	
	// get the count of blocks remaining minus the current one
	sBlockCount = IPU::_IPU->FifoIn_WriteIndex - IPU::_IPU->FifoIn_ReadIndex - 1;
	
	// check if there are any blocks remaining period
	if ( sBlockCount < 0 ) return false;
	
	// get the remaining number of bits in the current 64-bit word
	RemainingBits = 64 - ( IPU::_IPU->BitPosition & 0x3f );
	
	// get the bits in the remaining blocks (64-bits per block)
	BlockBits = sBlockCount << 6;
	
	// add them
	RemainingBits += BlockBits;
	
#ifdef INLINE_DEBUG_RESERVE
	IPU::debug << " RemainingBits=" << dec << RemainingBits;
#endif

	// check if it is not enough to cover num bits
	if ( RemainingBits < num ) return false;
	
	// should have enough bits remaining
	return true;
}


void ALIGN ()
{
	u32 BP, AlignedBP, Bits;
	
	BP = IPU::_IPU->BitPosition;
	AlignedBP = ( BP + 7 ) & ~7;
	
	// get the number of bits to proceed to align bit position
	Bits = AlignedBP - BP;
	
	// if not zero, then proceed in bit stream until BP is aligned
	if ( Bits )
	{
		IPU::_IPU->Proceed ( Bits );
		
		// proceed must be followed by ifc update for now
		IPU::_IPU->Update_IFC ();
	}
}


/*
bool GETBITS(u64* Result, uint num)
{
	if ( !RESERVEBITS ( num ) ) return false;

	uint retVal = UBITS(num);
	
	// ***todo*** pcsx2 specific code being comment out for now
	//g_BP.Advance(num);
	DUMPBITS ( num );

	return true;
}
*/

u64 PEEKBITS ( uint num )
{
	return IPU::_IPU->PeekBE ( num, IPU::_IPU->BitPosition );
	//return (IPU::_IPU->ReadBE64(IPU::_IPU->BitPosition)>>(64-num));
}

// verify that 16-bits are available
bool GETWORD ()
{
	return RESERVEBITS ( 16 );
}

// verify that 32-bits are available
bool BITSTREAM_INIT ()
{
	return RESERVEBITS ( 32 );
}



void IPU::Update_NextEventCycle ()
{
	//if ( NextEvent_Cycle > *_DebugCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_DebugCycleCount ) )
	if ( NextEvent_Cycle < *_NextSystemEvent )
	{
		*_NextSystemEvent = NextEvent_Cycle;
		*_NextEventIdx = NextEvent_Idx;
	}
}


void IPU::SetNextEvent ( u64 Cycle )
{
	NextEvent_Cycle = Cycle + *_DebugCycleCount;
	
	Update_NextEventCycle ();
}

void IPU::Set_NextEventCycle ( u64 Cycle )
{
	NextEvent_Cycle = Cycle;
	
	Update_NextEventCycle ();
}




////////////// Debugging ///////////////////////////




void IPU::DebugWindow_Enable()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	const char* DebugWindow_Caption = "PS2 IPU Debug Window";
	static constexpr int DebugWindow_X = 10;
	static constexpr int DebugWindow_Y = 10;
	static constexpr int DebugWindow_Width = 200;
	static constexpr int DebugWindow_Height = 200;

	static constexpr int List_X = 0;
	static constexpr int List_Y = 0;
	static constexpr int List_Width = 150;
	static constexpr int List_Height = 180;

	int i;
	stringstream ss;

	if (!DebugWindow_Enabled)
	{
		// create the main debug window
		DebugWindow = new WindowClass::Window();
		DebugWindow->Create(DebugWindow_Caption, DebugWindow_X, DebugWindow_Y, DebugWindow_Width, DebugWindow_Height);
		DebugWindow->DisableCloseButton();

		// create "value lists"
		ValueList = new DebugValueList<u32>();
		ValueList->Create(DebugWindow, List_X, List_Y, List_Width, List_Height, true, false);


		ValueList->AddVariable("CMD_Write", &(_IPU->CMD_Write.Lo));
		ValueList->AddVariable("CMD_Read.Lo", &(_IPU->CMD_Read.Lo));
		ValueList->AddVariable("CMD_Read.Hi", &(_IPU->CMD_Read.Hi));
		ValueList->AddVariable("TOP.Lo", &(_IPU->Regs.TOP.Lo));
		ValueList->AddVariable("TOP.Hi", &(_IPU->Regs.TOP.Hi));
		ValueList->AddVariable("CTRL", &(_IPU->Regs.CTRL.Lo));
		ValueList->AddVariable("BP", &(_IPU->Regs.BP.Lo));
		ValueList->AddVariable("FifoIn_Size", (u32*) & (_IPU->FifoIn_Size));
		ValueList->AddVariable("FifoOut_Size", (u32*) & (_IPU->FifoOut_Size));
		ValueList->AddVariable("FifoIn_ReadIndex", &(_IPU->FifoIn_ReadIndex));
		ValueList->AddVariable("FifoIn_WriteIndex", &(_IPU->FifoIn_WriteIndex));
		//ValueList->AddVariable("T2_COMP", &(_TIMERS->TheTimers[2].COMP.Value));
		//ValueList->AddVariable("T3_COUNT", &(_TIMERS->TheTimers[3].COUNT.Value));
		//ValueList->AddVariable("T3_COUNT2", (u32*)&(_TIMERS->TheTimers[3].StartValue));
		//ValueList->AddVariable("T3_MODE", &(_TIMERS->TheTimers[3].MODE.Value));
		//ValueList->AddVariable("T3_COMP", &(_TIMERS->TheTimers[3].COMP.Value));

		// mark debug as enabled now
		DebugWindow_Enabled = true;

		// update the value lists
		DebugWindow_Update();
	}

#endif

}

void IPU::DebugWindow_Disable()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;

	if (DebugWindow_Enabled)
	{
		delete DebugWindow;
		delete ValueList;

		// disable debug window
		DebugWindow_Enabled = false;
	}

#endif

}

void IPU::DebugWindow_Update()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	int i;

	if (DebugWindow_Enabled)
	{
		ValueList->Update();
	}

#endif

}




