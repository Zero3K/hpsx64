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



#ifndef _PS2_DMA_H_
#define _PS2_DMA_H_

#include "DebugValueList.h"

#include "types.h"
#include "Debug.h"

#include "PS2DataBus.h"

#ifndef EE_ONLY_COMPILE
#include "PS1DataBus.h"
#endif

#include "PS2_Intc.h"

//#include "PS2_MDEC.h"
#include "PS2_Gpu.h"
//#include "R5900.h"


namespace Playstation2
{

	class DataBus;
	class MDEC;
	class Dma;

	class DmaChannel
	{
		
	public:
		static int Count;
		
		u32 Number;
	
		// bus cycle# that transfer was started at
		//u64 StartCycle;
		
		union CHCR_Format
		{
			struct
			{
				// Direction. 0 - direction to memory; 1 - direction from memory
				// bit 0
				// for PS2, only used for channels 1/7 (VIF1/SIF2)
				u32 DIR : 1;

				// bit 1 - not used
				u32 zero0 : 1;
				
				// bits 2-3 - Mode - 00: Normal, 01: Chain, 10: Interleave
				u32 MOD : 2;
				
				// bits 4-5 - Address Stack Pointer - 00: No Addressed pushed by CALL tag, 01: 1 address has been pushed, 10: 2 addresses have been pushed
				u32 ASP : 2;
				
				// bit 6 - Tag transfer enable - 0: does NOT transfer DMA tag itself, 1: transfers DMA tag
				u32 TTE : 1;
				
				// bit 7 - Tag interrupt enable - 0: disables IRQ bit of DMA tag, 1: enables IRQ bit of DMA tag
				u32 TIE : 1;
				
				// bit 8 - DMA Start - 0: stops DMA, 1: starts DMA
				// note: This is the only bit that can be written while DMA is running
				u32 STR : 1;
				
				// bits 9-15 (always zero)
				u32 zero1: 7;
				
				// bits 16-31 - DMA tag - Holds bits 16-31 (IRQ/ID/PCE) of the DMA tag last read in CHAIN mode
				u32 TAG : 16;
				
			};
			
			
			struct
			{
				// quad word count
				// bits 0-15
				u32 filler0 : 16;
				
				// zero
				// bits 16-25
				u32 zero2 : 10;
				
				// PCE - Priority Control - 00: None, 01: not used, 10: Priority disabled (PCR.PCE=0), 11: Priority enabled (PCR.PCE=1)
				// bits 26-27
				u32 PCE : 2;
				
				// ID - Tag ID
				// for source chain: 000: refe, 001: cnt, 010: next, 011: ref, 100: refs, 101: call, 110: ret, 111: end
				// for destination chain: 000: cnts, 001: cnt, 111: end
				// bits 28-30
				u32 ID : 3;
				
				// IRQ - Interrupt request - 0: none, 1: interrupt requested at end of packet transfer
				// bit 31
				u32 IRQ : 1;
			};
			
			u32 Value;
		};

		//CHCR_Format CHCR_Reg;
		
		static const u32 CHCR_Offset = 0x0;
		
		
		union MADR_Format
		{
			struct
			{
				// this is the address to transfer to/from
				// bits 0-30 - lower four bits should be zero
				u32 Address : 31;

				// bit 31
				// Memory/SPR select - 0: Memory address, 1: SPR Address
				u32 SPR : 1;
			};

			u32 Value;
		};
		
		// DMA base memory address
		//MADR_Format MADR_Reg;
		
		static const u32 MADR_Offset = 0x1;
		
		union QWC_Format
		{
			struct
			{
				// this is the size of each block in quadwords (Block Size) (128-bit quadwords)
				// bits 0-15
				u16 QWC;

				// bits 16-31
				// for PS2 this is zero
				u16 zero0;
			};

			// the OTC DMA BS is actually this full value
			u32 Value;
		};

		// Block Count - set to zero when transferring linked list data to GPU
		// lower 16-bits hold the quadword count for transfer, upper 16-bits should be zero
		//QWC_Format QWC_Reg;
		
		static const u32 QWC_Offset = 0x2;
		
		

		
		// Next tag address register
		//MADR_Format TADR_Reg;
		
		static const u32 TADR_Offset = 0x3;
		
		
		// tag address save registers
		//MADR_Format ASR0_Reg;
		//MADR_Format ASR1_Reg;
		
		static const u32 ASR0_Offset = 0x4;
		static const u32 ASR1_Offset = 0x5;
		
		
		// SPR transfer address - specifies scrathpad memory address for DMA Channel 8/9 (fromSPR/toSPR)
		union SADR_Format
		{
			struct
			{
				// bits 0-13 - quad word aligned address of scratch pad memory
				u32 Address : 14;
				
				// bits 14-31 - zero
				u32 zero0 : 18;
			};
			
			u32 Value;
		};
		
		//SADR_Format SADR_Reg;

		static const u32 SADR_Offset = 0x8;



		struct RegData
		{
			// CHCR: Offset 0x0
			CHCR_Format CHCR;
			
			// filler
			//u32 Filler0 [ 3 ];
			
			// MADR: Offset 0x1
			// DMA base memory address
			MADR_Format MADR;

			// filler
			//u32 Filler1 [ 3 ];
			
			// QWC: Offset 0x2
			// Block Count - set to zero when transferring linked list data to GPU
			// lower 16-bits hold the quadword count for transfer, upper 16-bits should be zero
			QWC_Format QWC;

			// filler
			//u32 Filler2 [ 3 ];
			
			// TADR: Offset 0x3
			// Next tag address register
			MADR_Format TADR;

			// filler
			//u32 Filler3 [ 3 ];
			
			// ASR0: Offset 0x4
			// tag address save registers
			MADR_Format ASR0;
			
			// filler
			//u32 Filler4 [ 3 ];

			// ASR1: Offset 0x5
			MADR_Format ASR1;
			
			// filler
			//u32 Filler5 [ 3 ];
			
			// Reserved: Offset 0x6-0x7
			u32 Res67 [ 2 ];
			
			// SADR: Offset 0x8
			SADR_Format SADR;
			
			// filler
			//u32 Filler6 [ 3 ];
			
			// Reserved: Offset 0x9-0xf
			u32 Res9f [ 7 ];
		};


		
		
		s32 QWCRemaining;
		s32 QWCTransferred;
		
		//Dma::DMATag DTag;
		
		// cycle# at which dma should start
		u64 ullStartCycle;
		
		// constructor
		DmaChannel ();
		
		// resets dma channel
		void Reset ();
		
	};
	
	

	class Dma
	{
	
		static Debug::Log debug;
		
	public:
	
		static Dma *_DMA;
		
	
		//////////////////////////
		//	General Parameters	//
		//////////////////////////
		
		// where the dma registers start at
		static const u32 Regs_Start = 0x10008000;
		
		// where the dma registers end at
		static const u32 Regs_End = 0x1000eff0;
	
		// distance between numbered groups of registers for dma
		static const u32 Reg_Size = 0x10;
		
		struct HW_Register
		{
			bool ReadOK;
			bool WriteOK;
			bool Unknown;
			char* Name;
			u32 Address;
			u32 SizeInBytes;
			u32* DataPtr;
		};
		
		HW_Register Registers [ Regs_End - Regs_Start + Reg_Size ];
		
		
		static const char* Reg_Names [ 512 ];
		
		

		// dma setup time, in cycles. 1 means starts on the next cycle after starting
		static const u32 c_SetupTime = 1;
		
		// time between transfer of linked list primitives
		//static const u32 c_LinkedListSetupTime = 4;
		
		//static const u32 c_GPU_CycleTime = 2;
		//static const u32 c_SPU_CycleTime = 64;
		//static const u32 c_MDEC_CycleTime = 128;

		static const int NumberOfChannels = 10;
		static const int c_iNumberOfChannels = 10;

		//static const int c_iPriorityLevels = 8;
		
		// index for the next event
		u32 NextEvent_Idx;
		
		// cycle that the next event will happen at for this device
		u64 NextEventCh_Cycle [ NumberOfChannels ];
		u64 NextEvent_Cycle;

		// set the number of cycles dma channel will be busy for
		void SetNextEventCh ( u64 Cycles, u32 Channel );
		
		// set the next exact cycle dma channel will be free at
		void SetNextEventCh_Cycle ( u64 Cycle, u32 Channel );

		// update what cycle the next event is at for this device
		void Update_NextEventCycle ();

		// triggers a dma event for the specified dma channel to happen on the next cycle
		// designed to be called from other devices to signal an event, hence the interface
		typedef void (*fnRequestData) ( int, int );
		static void RequestData ( int Channel, int NumberOfWords );
		
		// start and end addresses for dma transfers
		u32 StartA, EndA;

		// DMA enabled bitmap
		u32 ChannelEnable_Bitmap;
		
		// it doesn't look like dmas can necessarily always intrude on currently continuous transfers
		u32 SelectedDMA_Bitmap;
		
		// need to know what the last channel was that ran
		int iLastChannel;
		
		// has mfifo started or not
		u32 bMfifoStarted;
		
		static u64 Read ( u32 Address, u64 Mask );
		static void Write ( u32 Address, u64 Data, u64 Mask );

		// get priority for channel
		u32 Get_ChannelPriority ( int iChannel );
		
		void Start ();
		
		void Run ();
		
		int GetNextActiveChannel ();
		
		void StartDMATransfer ( int Channel );
		
		
		//u32* DMA0_ReadBlock ();

		
		u64 BusyCycles;
		
		
		u32 GetPriority ( int Channel );
		u32 isEnabled ( int Channel );
		u32 isEnabled ();
		bool CheckPriority ( int Channel );
		u32 isActive ( int Channel );
		u32 isActive ();
		u32 isEnabledAndActive ( int Channel );
		u32 isEnabledAndActive ();

		// this wraps address around mfifo
		inline u32 Get_MfifoAddr ( u32 Address ) { return ( ( Address & DMARegs.RBSR.Value ) | DMARegs.RBOR ); }

		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////

		// CHCR - channel control
		static const u32 CHCR_Base = 0x10008000;
		
		// MADR - Memory Address - DMA base address
		static const u32 MADR_Base = 0x10008010;
	
		// QWC - quadword count - number of quadwords to transfer
		static const u32 QWC_Base = 0x10008020;
		
		// TADR - tag address
		static const u32 TADR_Base = 0x10008030;
		
		// ASR0 - address stack 0
		static const u32 ASR0_Base = 0x10008040;
		
		// ASR1 - address stack 1
		static const u32 ASR1_Base = 0x10008050;
		
		// SADR - SPR RAM address
		static const u32 SADR_Base = 0x10008080;
		

		// DMA tag
		union DMATag
		{
			struct
			{
				// quad word count
				// bits 0-15
				u64 QWC : 16;
				
				// zero
				// bits 16-25
				u64 zero0 : 10;
				
				// PCE - Priority Control - 00: None, 01: not used, 10: Priority disabled (PCR.PCE=0), 11: Priority enabled (PCR.PCE=1)
				// bits 26-27
				u64 PCE : 2;
				
				// ID - Tag ID
				// for source chain: 000: refe, 001: cnt, 010: next, 011: ref, 100: refs, 101: call, 110: ret, 111: end
				// for destination chain: 000: cnts, 001: cnt, 111: end
				// bits 28-30
				u64 ID : 3;
				
				// IRQ - Interrupt request - 0: none, 1: interrupt requested at end of packet transfer
				// bit 31
				u64 IRQ : 1;
				
				// ADDR - address of packet or next tag instruction (with qword alignment, lower four bits become zero)
				// bits 32-62
				u64 ADDR : 31;
				
				// SPR - Memory/SPR Selection - 0: Memory address, 1: SPR Address
				// bit 63
				u64 SPR : 1;
			};
			
			u64 Value;
		};


		
		
		// need the current source dma tag for channel
		DMATag SourceDMATag [ c_iNumberOfChannels ];

		// might need the IOP dma tag?
		DMATag IOPDMATag [ c_iNumberOfChannels ];
		
		// DMA Tag definitions
		// cnt: 
		
		

		// CTRL - DMA control register
		static const u32 CTRL = 0x1000e000;
		
		union CTRL_Format
		{
			struct
			{
				// DMA Enable
				// bit 0 - 0: disables ALL DMAs, 1: enables all DMAs
				u32 DMAE : 1;
				
				// Cycle Steal Mode/Release Enable
				// bit 1 - 0: Cycle stealing on, 1: Cycle stealing off
				u32 CycleStealMode : 1;
				
				// Memory FIFO Drain Channel
				// bits 2-3 - 00: None, 01: Unknown, 10: VIF1 Channel (channel 1), 11: GIF Channel (channel 2)
				u32 MFIFO : 2;
				
				// Stall control source channel
				// bits 4-5 - 00: Non-specified (does not update STADR), 01: SIF0 Channel (ch 5), 10: fromSPR channel (ch 8), 11: fromIPU Channel (ch 3)
				u32 STS : 2;

				// Stall control drain channel
				// bits 6-7 - 00: None, 01: VIF1 Channel (ch 1), 10: GIF channel (ch 2), 11: SIF1 Channel (ch 6)
				u32 STD : 2;
				
				// Release Cycle
				// bits 8-10 - 000: 8, 001: 16, 010: 32, 011: 64, 100: 128, 101: 256
				u32 RCYC : 3;
				
				// bits 11-31 - zero
				u32 zero0 : 21;
			};
		
			u32 Value;
		};
		
		//CTRL_Format CTRL_Reg;



		// STAT - DMA interrupt status
		static const u32 STAT = 0x1000e010;
		
		union STAT_Format
		{
			struct
			{
				// Channel interrupt status - is set to one when a transfer ends, write a one to clear
				// bits 0-9 - 0: transfer for channel NOT complete, 1: transfer for channel complete
				u32 CIS : 10;
				
				// bits 10-12 - zero
				u32 zero0 : 3;
				
				// bit 13 - SIS - DMA Stall interrupt status
				// gets set to 1 when stall condition is met, cleared when 1 is written
				u32 SIS : 1;
				
				// bit 14 - MEIS - MFIFO Empty interrupt status
				// gets set to 1 when mfifo empties, cleared when 1 is written
				u32 MEIS : 1;
				
				// bit 15 - BEIS - DMA Stall interrupt
				// gets set to 1 when bus error occurs, cleared when 1 is written
				u32 BEIS : 1;
				
				// Channel interrupt mask - write a one to reverse
				// bits 16-25 - 0: disable, 1: enable
				u32 CIM : 10;
				
				// bits 26-28 - zero
				u32 zero1 : 3;
				
				// bit 29 - SIM - DMA Stall interrupt mask
				// reversed when 1 is written
				u32 SIM : 1;
				
				// bit 30 - MEIM - MFIFO Empty interrupt mask
				// reversed when 1 is written
				u32 MEIM : 1;
			};
		
			u32 Value;
		};
		
		//STAT_Format STAT_Reg;


		// *** TODO ***
		
		
		// PCR - priority control register
		static const u32 PCR = 0x1000e020;
		
		union PCR_Format
		{
			struct
			{
				// COP Output Control - CPC - bits 0-9 - 0: disable channel status output to CPCOND[0], 1: enable channel status output to CPCOND[0]
				u32 CPC0 : 1;
				u32 CPC1 : 1;
				u32 CPC2 : 1;
				u32 CPC3 : 1;
				u32 CPC4 : 1;
				u32 CPC5 : 1;
				u32 CPC6 : 1;
				u32 CPC7 : 1;
				u32 CPC8 : 1;
				u32 CPC9 : 1;
				
				// bits 10-15 - zero
				u32 zero0 : 6;
				
				// DMA Channel enable - CDE - bits 16-25 - 0: disable dma channel, 1: enable dma channel
				u32 CDE0 : 1;
				u32 CDE1 : 1;
				u32 CDE2 : 1;
				u32 CDE3 : 1;
				u32 CDE4 : 1;
				u32 CDE5 : 1;
				u32 CDE6 : 1;
				u32 CDE7 : 1;
				u32 CDE8 : 1;
				u32 CDE9 : 1;
				
				// bits 26-30 - zero
				u32 zero1 : 5;
				
				// PCR Enable - bit 31 - 0: enable all dma channels regardless CDE, 1: enable dma channels based on CDE value
				u32 PCE : 1;
			};
		
			u32 Value;
		};
		
		//PCR_Format PCR_Reg;


		union SQWC_Format
		{
			struct
			{
				u8 SQWC;
				u8 zero0;
				u8 TQWC;
				u8 zero1;
			};
		
			u32 Value;
		};
		
		// interleave count register
		//SQWC_Format SQWC_Reg;
		
		// ring buffer address register
		//u32 RBOR_Reg;


		union RBSR_Format
		{
			struct
			{
				u32 zero0 : 4;
				u32 RMSK : 27;
				u32 zero1 : 1;
			};
		
			u32 Value;
		};

		// ring buffer size register
		//RBSR_Format RBSR_Reg;
		
		// stall address register
		//u32 STADR_Reg;

		// DMA Enable r/w
		//u32 lDMAC_ENABLE;


		static const u32 DMA0_CHCR = 0x10008000;
		static const u32 DMA0_MADR = 0x10008010;
		static const u32 DMA0_QWC = 0x10008020;
		static const u32 DMA0_TADR = 0x10008030;
		static const u32 DMA0_ASR0 = 0x10008040;
		static const u32 DMA0_ASR1 = 0x10008050;

		static const u32 DMA1_CHCR = 0x10009000;
		static const u32 DMA1_MADR = 0x10009010;
		static const u32 DMA1_QWC = 0x10009020;
		static const u32 DMA1_TADR = 0x10009030;
		static const u32 DMA1_ASR0 = 0x10009040;
		static const u32 DMA1_ASR1 = 0x10009050;

		static const u32 DMA2_CHCR = 0x1000a000;
		static const u32 DMA2_MADR = 0x1000a010;
		static const u32 DMA2_QWC = 0x1000a020;
		static const u32 DMA2_TADR = 0x1000a030;
		static const u32 DMA2_ASR0 = 0x1000a040;
		static const u32 DMA2_ASR1 = 0x1000a050;

		static const u32 DMA3_CHCR = 0x1000b000;
		static const u32 DMA3_MADR = 0x1000b010;
		static const u32 DMA3_QWC = 0x1000b020;

		static const u32 DMA4_CHCR = 0x1000b400;
		static const u32 DMA4_MADR = 0x1000b410;
		static const u32 DMA4_QWC = 0x1000b420;
		static const u32 DMA4_TADR = 0x1000b430;

		static const u32 DMA5_CHCR = 0x1000c000;
		static const u32 DMA5_MADR = 0x1000c010;
		static const u32 DMA5_QWC = 0x1000c020;

		static const u32 DMA6_CHCR = 0x1000c400;
		static const u32 DMA6_MADR = 0x1000c410;
		static const u32 DMA6_QWC = 0x1000c420;
		static const u32 DMA6_TADR = 0x1000c430;

		static const u32 DMA7_CHCR = 0x1000c800;
		static const u32 DMA7_MADR = 0x1000c810;
		static const u32 DMA7_QWC = 0x1000c820;

		static const u32 DMA8_CHCR = 0x1000d000;
		static const u32 DMA8_MADR = 0x1000d010;
		static const u32 DMA8_QWC = 0x1000d020;
		static const u32 DMA8_SADR = 0x1000d080;

		static const u32 DMA9_CHCR = 0x1000d400;
		static const u32 DMA9_MADR = 0x1000d410;
		static const u32 DMA9_QWC = 0x1000d420;
		static const u32 DMA9_TADR = 0x1000d430;
		
		
		static const u32 DMA_CTRL = 0x1000e000;
		static const u32 DMA_STAT = 0x1000e010;
		static const u32 DMA_PCR = 0x1000e020;
		static const u32 DMA_SQWC = 0x1000e030;
		static const u32 DMA_RBSR = 0x1000e040;
		static const u32 DMA_RBOR = 0x1000e050;
		static const u32 DMA_STADR = 0x1000e060;
		
		static const u32 DMA_ENABLER = 0x1000f520;
		static const u32 DMA_ENABLEW = 0x1000f590;



		union DMARegs_t
		{
			struct
			{
				unsigned long Registers0 [ 6 * 16 * 4 ];
				
				// 0x1000e000
				CTRL_Format CTRL;
				STAT_Format STAT;
				PCR_Format PCR;
				SQWC_Format SQWC;
				RBSR_Format RBSR;
				unsigned long RBOR;
				
				// 0x1000e060
				u32 STADR;
				
				// 0x1000e070-0x1000e0f0
				unsigned long Registers1 [ 9 ];
				
				// 0x1000e400-0x1000f00f
				unsigned long Registers2 [ 1 * 16 * 4 ];
				
				// 0x1000f500
				// 0x1000f510
				unsigned long Registers3 [ 2 ];
				
				// 0x1000f520
				unsigned long ENABLER;
				
				// 0x1000f530-0x1000f580
				unsigned long Registers4 [ 6 ];
				
				// 0x1000f590
				unsigned long ENABLEW;
				
				// 0x1000f5a0-0x1000f5f0
				unsigned long Registers5 [ 6 ];
				
				// 0x1000f800-0x1000fcf0
				unsigned long padding0 [ 2 * 16 * 4 ];
			};
			
			DmaChannel::RegData RegData [ 8 * 4 ];
			
			// 30 32-bit registers
			// 0x10008000-0x1000f5f0
			unsigned long Regs [ 8 * 16 * 4 ];
		};
		
		DMARegs_t DMARegs;

		// static pointers to access the registers for peripheral
		static DmaChannel::RegData* pRegData [ c_iNumberOfChannels ];
		static DMARegs_t* pDMARegs;
		
		void Refresh ()
		{
			pDMARegs = & DMARegs;
			
			pRegData [ 0 ] = & ( DMARegs.RegData [ 0 ] );
			pRegData [ 1 ] = & ( DMARegs.RegData [ 4 ] );
			pRegData [ 2 ] = & ( DMARegs.RegData [ 8 ] );
			pRegData [ 3 ] = & ( DMARegs.RegData [ 12 ] );
			pRegData [ 4 ] = & ( DMARegs.RegData [ 13 ] );
			pRegData [ 5 ] = & ( DMARegs.RegData [ 16 ] );
			pRegData [ 6 ] = & ( DMARegs.RegData [ 17 ] );
			pRegData [ 7 ] = & ( DMARegs.RegData [ 18 ] );
			pRegData [ 8 ] = & ( DMARegs.RegData [ 20 ] );
			pRegData [ 9 ] = & ( DMARegs.RegData [ 21 ] );
		}

		//////////////////////////////////////////////////////
		// Channel 0 - VU0/VIF0								//
		// Channel 1 - VU1/VIF1								//
		// Channel 2 - GPU/GIF								//
		// Channel 3 - MDEC/IPU out							//
		// Channel 4 - MDEC/IPU in							//
		// Channel 5 - SIF0 (from SIF/IOP)					//
		// Channel 6 - SIF1 (to SIF/IOP)					//
		// Channel 7 - SIF2									//
		// Channel 8 - SPR out								//
		// Channel 9 - SPR in								//
		//////////////////////////////////////////////////////
		
		static const char* DmaCh_Names [ c_iNumberOfChannels ];
		
		DmaChannel DmaCh [ c_iNumberOfChannels ];

		// busy until cycles for each dma channel
		u64 BusyUntil_Cycle [ c_iNumberOfChannels ];
		
		//static const u64 c_llSIFDelayPerQWC = 1;
		
		// 8 qword buffer * 128 bits per qword = 1024 ps1 bus cycles
		// things can crash when this is set to zero. things can also crash if this value is too high.
		// 32 seems to be a good value for the transfer "overhead" for SIF on PS2 side
		//static const u64 c_ullSIFOverhead = 0;	//32 << 4;
		
		// there has to be a dma setup time, in case nothing is transferred
		static const u64 c_iSetupTime = 2;
		
		static const u64 c_iVU0_TransferTime = 1;
		static const u64 c_iVU1_TransferTime = 1;
		static const u64 c_iGIF_TransferTime = 1;
		static const u64 c_iMDECout_TransferTime = 1;
		static const u64 c_iMDECin_TransferTime = 1;
		
		// IOP -> EE
		static const u64 c_iSIF0_TransferTime = 1;	//1;
		
		// 1 qword transferred = ( 4 iop/cycles per word * 4 words/quadword ) + ( 2 cycles per qw transfer from ram or 1 cycle per qw transfer SPR )
		// no, its actually 1 QWC transferred = ( 1 iop cycle/word * 4 words/qwc * 4 ps2 bus cycles/iop cycle ) + overhead = ~16 ps2 bus cycles delay per QWC
		// 64 is too high here it seems. 1 is too low.
		// looks like this is all wrong, because it must be freezing both iop and ee on sbus transfers
		// EE -> IOP
		static const u64 c_iSIF1_TransferTime = 1;	//0;	//16;	//1
		
		
		static const u64 c_iDmaSetupTime [ c_iNumberOfChannels ];
		static const u64 c_iDmaTransferTimePerQwc [ c_iNumberOfChannels ];
		
		static const u64 c_iDeviceBufferSize [ c_iNumberOfChannels ];
		
		/////////////////////////////////////////////////////////////////
		// Need to be able to save the transfer size
		//u32 TransferSize_Save [ c_iNumberOfChannels ];
		
		// need to know the amount of data left to transfer for dma channel in current block
		// -1 transferred means that block still needs to be started
		s32 QWC_Transferred [ c_iNumberOfChannels ];
		s32 QWC_BlockTotal [ c_iNumberOfChannels ];
		
		// helper function for transferring a full/partial block via dma
		u64 Chain_TransferBlock ( int iChannel );
		
		// we need to know when an interrupt is being requested by dma, but also want to keep component separate for extensive testing
		//Intc::I_STAT_Format Interrupt_Status;

		// gets cleared when no transfer is made
		// 0: ok to transfer; 1: release bus
		u32 BusReleaseToggle;

		// need a pointer to the data bus object
		//static DataBus *_BUS;
		
		// need to be able to get MDEC status
		//static MDEC *_MDEC;
		
		// need to be able to get GPU status
		//static GPU *_GPU;
		
		// need a reference to cpu for invalidating cache
		//static R5900::Cpu *_CPU;
		
		// state of interrupt - *testing*
		u32 Interrupt_State;
		
		// need function to perform a dma
		void StartTransfer ( int iChannel );
		void Transfer ( int iChannel );
		void EndTransfer ( int iChannel, bool SuppressEventUpdate = false );
		
		// checks for and resumes the dma transfer(s)
		void CheckTransfer ();
		
		// suspends dma transfer without triggering interrupt
		void SuspendTransfer ( int iChannel );
		
		// use this to complete transfer after dmas are restarted after a suspension
		void UpdateTransfer ();
		
		//void DMA_Run ( int iChannel, bool CycleStealMode = false );
		//void DMA6_Run ( bool ContinueToCompletion );
		//void DMA5_Run ( bool ContinueToCompletion );
		//void DMA4_Run ( bool ContinueToCompletion );
		//void DMA3_Run ( bool ContinueToCompletion );
		//void DMA2_Run ( bool ContinueToCompletion );
		//void DMA1_Run ( bool ContinueToCompletion );
		//void DMA0_Run ( bool ContinueToCompletion );

		
		static u32 DMA5_WriteBlock ( u64 *Data64, u32 QWC_Count );	//( u64 EEDMATag, u64* Data64, u32 QWC_Count );

		// must use this to add a reference to the bus
		//void ConnectDevices ( DataBus *BUS, MDEC *mdec, GPU *g, CD *cd, SPU *spu, R5900::Cpu *cpu );
		
		typedef bool (*fnReady) ( void );
		
		// this one should return the amount of data transferred
		typedef u32 (*fnTransfer_FromMemory) ( u64* Data, u32 QuadwordCount );
		
		static fnReady cbReady [ c_iNumberOfChannels ];
		static fnReady cbReady_ToMemory [ c_iNumberOfChannels ];
		static fnTransfer_FromMemory cbTransfer_FromMemory [ c_iNumberOfChannels ];
		static fnTransfer_FromMemory cbTransfer_ToMemory [ c_iNumberOfChannels ];

		//static fnTransfer_FromMemory cbTransferTag_FromMemory [ c_iNumberOfChannels ];
		
		static const u32 c_iStallSource_LUT [ 4 ];
		static const u32 c_iStallDest_LUT [ 4 ];
		static const u32 c_iMfifoDrain_LUT [ 4 ];
		
		void NormalTransfer_ToMemory ( int iChannel );
		void NormalTransfer_FromMemory ( int iChannel );
		void ChainTransfer_ToMemory ( int iChannel );
		void ChainTransfer_FromMemory ( int iChannel );
		void InterleaveTransfer_ToMemory ( int iChannel );
		void InterleaveTransfer_FromMemory ( int iChannel );
		
		// constructor
		Dma ();
		
		void Reset ();
		void Update_ICR ( u32 Data );
		
		
		// call this to update interrupts whenever interrupt related registers get modified
		void UpdateInterrupt ();
		
		// call this to update CPCOND0
		// this MUST to be called after PCR gets changed or STAT gets changed for any reason
		void Update_CPCOND0 ();

		
		static bool SPRout_DMA_Ready ();
		static bool SPRin_DMA_Ready ();
		
		static u32 SPRout_DMA_Read ( u64* Data, u32 QuadwordCount );
		static u32 SPRin_DMA_Write ( u64* Data, u32 QuadwordCount );
		
		
		// this will get the address ptr from the dma address value
		// so if the address has SPR set, it will get the SPR address pointer, otherwise the main memory pointer
		static u64* GetMemoryPtr ( u32 Address );

		
		// this is the dma channel that is currently transferring data
		u32 ActiveChannel;
		
		
		static u32* _SBUS_F240;
		static u32* _CPCOND0_Out;
		
		inline void ConnectDevices ( u32* _inSBUS_F240, u32* _CPCOND0 )
		{
			_SBUS_F240 = _inSBUS_F240;
			_CPCOND0_Out = _CPCOND0;
		}


		static void sRun () { _DMA->Run (); }
		static void Set_EventCallback ( funcVoid2 UpdateEvent_CB ) { _DMA->NextEvent_Idx = UpdateEvent_CB ( sRun ); };

		
		static const u32 c_InterruptBit = 10;
		
		static u32* _Intc_Stat;
		static u32* _Intc_Mask;
		static u32* _R5900_Status_12;
		static u32* _R5900_Cause_13;
		static u64* _ProcStatus;
		
		
		inline void ConnectInterrupt ( u32* _IStat, u32* _IMask, u32* _R5900_Status, u32* _R5900_Cause, u64* _ProcStat )
		{
			_Intc_Stat = _IStat;
			_Intc_Mask = _IMask;
			_R5900_Cause_13 = _R5900_Cause;
			_R5900_Status_12 = _R5900_Status;
			_ProcStatus = _ProcStat;
		}
		
		
		static const int c_iCpuInt1_Bit = 11;
		
		inline void SetInterrupt ()
		{
			// ***todo*** for the DMA, INT 1 should get set in Cause register
			
			// set int 1 directly - which looks like it is bit 11 in Status/Cause
			*_R5900_Cause_13 |= ( 1 << c_iCpuInt1_Bit );
			
			// PS2 DMA does NOT update INTC
			// check if R5900 is in an interrupt status
			//if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 0 );
			if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0x8c00 ) && ( *_R5900_Status_12 & 1 ) && ( *_R5900_Status_12 & 0x10000 ) && !( *_R5900_Status_12 & 0x6 ) )
			{
				*_ProcStatus |= ( 1 << 0 ); 
			}
			/*
			else
			{
				*_ProcStatus &= ~( 1 << 0 );
			}
			*/
			
			// check stat register to see if interrupt should be triggered or not
			//*_Intc_Stat |= ( 1 << c_InterruptBit );
			//if ( *_Intc_Stat & *_Intc_Mask ) *_R5900_Cause_13 |= ( 1 << 10 );
			
			//if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 0 );
		}
		
		inline void ClearInterrupt ()
		{
			// clear int 1 directly - which looks like it is bit 11 in Status/Cause
			*_R5900_Cause_13 &= ~( 1 << c_iCpuInt1_Bit );
			
			// PS2 DMA does NOT update INTC
			// check if R5900 is in an interrupt status
			//if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 0 ); else *_ProcStatus &= ~( 1 << 0 );
			
			//*_Intc_Stat &= ~( 1 << c_InterruptBit );
			//if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R5900_Cause_13 &= ~( 1 << 10 );
			
			//if ( ( *_R5900_Cause_13 & *_R5900_Status_12 & 0xff00 ) && ( *_R5900_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 0 );
		}
		
		static u64* _NextSystemEvent;
		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		static u32* _NextEventIdx;
		//static u32* _Intc_Mask;
		//static u32* _Intc_Stat;
		static u32* _R5900_Status;
		
		static bool DebugWindow_Enabled;
		static WindowClass::Window *DebugWindow;
		static DebugValueList<u32> *DMA_ValueList;
		static void DebugWindow_Enable ();
		static void DebugWindow_Disable ();
		static void DebugWindow_Update ();
		

	
		
	private:
	
		//u32 DMA_Finished ( int index );
		void DMA_Finished ( int index, bool SuppressDMARestart = false );
		u32 DMA_Interrupt_Update ();

	};
	
};

#endif

