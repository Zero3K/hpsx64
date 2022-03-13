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



#ifndef _PS1_DMA_H_
#define _PS1_DMA_H_

#include "DebugValueList.h"

#include "types.h"
#include "Debug.h"

#include "PS1DataBus.h"

#include "PS1_Intc.h"

#include "PS1_MDEC.h"
#include "PS1_Gpu.h"
#include "PS1_CD.h"
#include "PS1_SPU.h"


//#define USE_NEW_SCORING_SYSTEM



namespace Playstation1
{

	class DataBus;
	class MDEC;

	class DmaChannel
	{
		
	public:
		static int Count;
		
		u32 Number;
	
		
		
		
		union BCR_Format
		{
			struct
			{
				// this is the size of each block in words (Block Size) (Probably 32-bit words)
				// bits 0-15
				u16 BS;

				// this is the number of blocks (Block Amount)
				// bits 16-31
				u16 BA;
			};

			// the OTC DMA BS is actually this full value
			u32 Value;
		};

		
		union CHCR_Format
		{
			struct
			{
				// Direction. 0 - direction to memory; 1 - direction from memory
				// bit 0
				u32 DR : 1;

				// bit 1 - Memory Step
				// 0: forwards (+4); 1: backwards (-4)
				// for dma6 this is fixed at 1
				u32 MemoryStep : 1;
				
				// bits 2-8 (always zero)
				// PS2: bit 8 might be indicating a preceeding DMA tag
				// PS2: bit8 - chain transfer mode - 1: enable chain transfer, 0: NOT chain transfer
#ifdef PS2_COMPILE
				u32 zero0 : 6;
				u32 ChainTransferMode : 1;
#else
				u32 zero0 : 7;
#endif
				
				// bits 9-10 - Transfer Synchronisation Mode
				// 0: start immediately and transfer all at once (CDROM, OTC)
				// 1: Sync blocks to DMA requests (MDEC, SPU, GPU)
				// 2: Linked-List mode
				// 3: Reserved for PS2?? - where it means to transfer linked list AND sync to dma requests ??
				u32 SyncMode : 2;

				// 1 - continuous stream of data
				// bit 9
				//u32 CO : 1;

				// 1 - transfer linked list (GPU only)
				// bit 10
				//u32 LI : 1;

				// always zero
				// bit 11-15 (always zero)
				u32 zero1 : 5;
				
				// bits 16-18 - Unknown (r/w)
				u32 unknown0 : 3;
				
				// bit 19 - Not Used (always zero)
				u32 zero2 : 1;
				
				// bits 20-22 - Unknown - r/w
				u32 unknown1 : 3;
				
				// bit 23 - Not Used (always zero)
				u32 zero3 : 1;
				
				// 0 - No dma transfer busy; 1 - Start dma transfer/dma transfer busy
				// resets to zero when transfer is complete
				// bit 24
				u32 TR : 1;
				
				// bits 25-27 - Not Used (always zero)
				u32 zero4 : 3;
				
				// bit 28 - Start/Busy (Sync Mode 0 only)
				// resets to zero when transfer is complete
				u32 Start_SyncMode0 : 1;
				
				// bit 29 - Pause ?? (Sync Mode 0 only) ??
				u32 Pause_SyncMode0 : 1;
				
				// bit 30 - Unknown - r/w - bus snooping ?
				// PS2 only?? Maybe something about initiating the transfer on PS2
				// so it probably gets cleared after transfer start then...
#ifdef PS2_COMPILE
				u32 Enable_BusSnooping : 1;
#else
				u32 unknown2 : 1;
#endif
				
				// bit 31 - Not Used (always zero)
				u32 zero5 : 1;
			};
			
			struct
			{
				// bit 0
				u32 padding0 : 1;
				
				// bit 1
				u32 OT0 : 1;
				
				// bits 2-8
				u32 padding1 : 7;
				
				// bit 9 - can also reference this as CO
				u32 CO : 1;
				
				// bit 10 - can also reference this as LI
				u32 LI : 1;
				
				// bits 11-27
				u32 padding2 : 17;
				
				// bit 28
				u32 OT1 : 1;
				
				// bits 29-31
				u32 padding3 : 3;
			};
			
			u32 Value;
		};



		struct RegData
		{
			// DMA base memory address
			u32 MADR;
			
			// Block Count - set to zero when transferring linked list data to GPU
			BCR_Format BCR;
			
			CHCR_Format CHCR;

#ifdef PS2_COMPILE
			u32 TADR;
#else
			u32 CHCRm;
#endif
		};

		
		// ptr to actual registers for object
		//RegData *pRegData;
		
		
		// cycle# at which DMA transfer should actually start
		// this gives the option of adding a dma setup time if there is one
		u64 ullStartCycle;
		
#ifdef PS2_COMPILE
		// used for chain transfer for now
		s32 WordsRemaining;
		s32 WordsRemainingMultiple4;
		
		u32 LastIOPTag;
		
		u64 EEDMATag;
		
		u32 BlockTotal;
		
		u32 WCTransferred;
#endif
		
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
		static const u32 Regs_Start = 0x1f801080;
		
		// where the dma registers end at
		static const u32 Regs_End = 0x1f8010f4;
	
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
		

		static const char* Reg0_Names [ 8 * 4 ];
		static const char* Reg1_Names [ 8 * 4 ];

		// dma setup time, in cycles. 1 means starts on the next cycle after starting
		//static const u32 c_SetupTime = 1;
		
		// time between transfer of linked list primitives
		// *important* - if this value is zero, then recompiler can get locked out, but otherwise you can have other issues
		// so this value has to be determined by gpu timing only probably
		static const u32 c_LinkedListSetupTime = 1;
		
		//static const u32 c_GPU_CycleTime = 2;
		
		// if this is set too high, PS2 bios will crash
		// if this is set too low, then PS1 games will crash
		//static const u32 c_SPU_CycleTime = 16;	//48;	//64;
		
		//static const u32 c_MDEC_CycleTime = 128;

#ifdef PS2_COMPILE
		static const u32 NumberOfChannels = 13;
		static const u32 c_iNumberOfChannels = 13;
#else
		static const u32 NumberOfChannels = 7;
		static const u32 c_iNumberOfChannels = 7;
#endif

		static const int c_iPriorityLevels = 16;	//8;
		
		static const u64 c_llSIFDelayPerWC = 1;
		static const u64 c_ullSIFOverhead = 0;
		
		// busy until cycles for each dma channel
		u64 BusyUntil_Cycle [ c_iNumberOfChannels ];
		
		
		// index for next event
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



		// the ready function should return zero when ready, and non-zero when not
		// if non-zero, the value should be the cycle number device will be ready at if known, otherwise return 1
		typedef u64 (*fnReady) ( void );
		
		// this one should return the amount of data transferred
		// it is possible transfer could wrap around the RAM, so need to pass the transfer address and pointer to the memory device
		// this has to return the amount transferred, because in some cases you might not know otherwise (especially on PS2)
		typedef u32 (*fnTransfer_FromMemory) ( u32* MemPtr, u32 Address, u32 WordCount );
		
		static fnReady cbReadyForRead [ c_iNumberOfChannels ];
		static fnReady cbReadyForWrite [ c_iNumberOfChannels ];
		static fnTransfer_FromMemory cbTransfer_FromMemory [ c_iNumberOfChannels ];
		static fnTransfer_FromMemory cbTransfer_ToMemory [ c_iNumberOfChannels ];
		
		// the time it takes to access 32-bits of data for the current dma channel
		u64 ullAccessTime;
		u64 ullReleaseTime;
		u64 ullReturnCycle;

		// minimum amount of time before transfer begins after being started
		static const u64 c_ullSetupTime [ c_iNumberOfChannels ];

		// time for each 32-bit word of data to transfer
		static const u64 c_ullAccessTime [ c_iNumberOfChannels ];
		
		static const u64 c_ullReleaseTime [ c_iNumberOfChannels ];

		// determine what the newly active channel is
		void Arbitrate ( int iChannel );
		
		void Update_ActiveChannel ();
		
		// perform a transfer for a dma channel
		// note: added FORCE_TRANSFER optional argument to handle dma0 transfers from MDEC for now
		void Transfer ( int iChannel, bool FORCE_TRANSFER = false );
		
		// transfer a BS block of data
		u32 TransferBlock ( int iChannel, u32 Address, u32 WordCount );

		// transfer a block of a PS1-style linked-list transfer
		u32 TransferLinkedList ( int iChannel, u32 Address );

		// check if the device is ready
		// returns: 0 - device not ready, 1 - device ready immediately, other - device ready in the future at returned cycle number
		u64 isDeviceReady ( int iChannel );

		
		// start and end addresses for dma transfers
		u32 StartA, EndA;

		// DMA enabled bitmap
		u32 ChannelEnable_Bitmap;
		
		// need to put a stop to dma channels possibly restarting when they are already in progress
		// this will determine what transfers are already in progress
		// TransferStart will automatically set it's bit, while TransferEnd will clear it
		u32 Channels_InProgress;
		
		// it doesn't look like dmas can necessarily always intrude on currently continuous transfers
		u32 SelectedDMA_Bitmap;
		
		static u32 Read ( u32 Address );
		static void Write ( u32 Address, u32 Data, u32 Mask );
		
		void Start ();
		
		void Run ();
		
		
		u32* DMA0_ReadBlock ();


#ifdef PS2_COMPILE
		u8 SIO2_Buffer [ 24 * 4 * 16 ];
		
		// set the MADR value for DMA10 from Destination DMA tag
		void DMA10_SetMADR ( u32 MADR );
		
		// forces DMA10 to write a block of data to memory
		void DMA10_WriteBlock ( u32* Data, u32 WordCount, u64 PS2CycleCount );
#endif


		u64 BusyCycles;
		
		
		u32 GetPriority ( int Channel );
		u32 isEnabled ( int Channel );
		u32 isEnabled ();
		bool CheckPriority ( int Channel );
		u32 isActive ( int Channel );
		u32 isActive ();
		u32 isEnabledAndActive ( int Channel );
		u32 isEnabledAndActive ();
		bool isTransferring ( int Channel );

		// this gets the priority for the group of channels?
		u32 GetPriority_Offset ( int Channel );

		// get the priority scoring value for a dma channel, higher number has higher priority
		u32 Priority ( int iChannel );


		//////////////////////////////////
		//	Device Specific Parameters	//
		//////////////////////////////////

		// MADR - Memory Address - DMA base address
		static const u32 MADR_Base = 0x1f801080;
	
		// BCR - block count - set to zero when transfering linked list to GPU via DMA2
		static const u32 BCR_Base = 0x1f801084;
		
		
		// CHCR - channel control
		// when bit 18 is zero, then DMA is over and it is ready for commands, otherwise it is not
		static const u32 CHCR_Base = 0x1f801088;
		
		
		// PCR - priority?/primary? control register
		static const u32 PCR = 0x1f8010f0;
		
		union PCR_Format
		{
			struct
			{
				// priority
				// bits 0-2
				// 0: Highest; 7: Lowest
				// when channels have the same priority, then channel 6 has highest priority and channel 0 has lowest priority
				u32 DMA0_Priority : 3;
				
				// this must be set to enable DMA channel
				// bit 3
				u32 DMA0_Enable : 1;

				u32 DMA1_Priority : 3;
				u32 DMA1_Enable : 1;
				
				u32 DMA2_Priority : 3;
				u32 DMA2_Enable : 1;
				
				u32 DMA3_Priority : 3;
				u32 DMA3_Enable : 1;

				u32 DMA4_Priority : 3;
				u32 DMA4_Enable : 1;
				
				u32 DMA5_Priority : 3;
				u32 DMA5_Enable : 1;

				u32 DMA6_Priority : 3;
				u32 DMA6_Enable : 1;
				
				// bits 28-31 - Unknown - r/w
				u32 unknown0 : 4;
			};
		
			u32 Value;
		};
		
		//PCR_Format PCR_Reg;
		
#ifdef PS2_COMPILE
		//PCR_Format PCR2_Reg;
#endif


		// ICR - DMA Interrupt Control
		// bit 15 - ?? DMA channel interrupt enable for SPU DMA ?? - r/w
		static const u32 ICR = 0x1f8010f4;
		
		
		union ICR_Format
		{
			struct
			{
				// bit 0-5 ?? fast DMA0 Transfer ??
				u32 fastDMA0Transfer : 1;
				u32 fastDMA1Transfer : 1;
				u32 fastDMA2Transfer : 1;
				u32 fastDMA3Transfer : 1;
				u32 fastDMA4Transfer : 1;
				u32 fastDMA5Transfer : 1;
				//u32 fastDMA6Transfer : 1;

				// bits 6-14: Not Used (always zero)
				u32 NotUsed0 : 9;
				
				// bit 15 - Force Interrupt (sets bit 31)
				// 0: do NOT force interrupt; 1: force interrupt
				u32 ForceInterrupt : 1;
				
				// bits 16-22 - enable DMA interrupt
				u32 DMA0InterruptEnable : 1;
				u32 DMA1InterruptEnable : 1;
				u32 DMA2InterruptEnable : 1;
				u32 DMA3InterruptEnable : 1;
				u32 DMA4InterruptEnable : 1;
				u32 DMA5InterruptEnable : 1;
				u32 DMA6InterruptEnable : 1;
				
				// bit 23 - ?? should be 1 ??
				// bit 23 - global dma interrupt enable/disable - 0: disable dma interrupts; 1: enable dma interrupts
				u32 DMAInterruptEnable : 1;
				
				// bit 24 - DMA0 Interrupt Pending/Acknowledge
				u32 DMA0InterruptPending : 1;
				u32 DMA1InterruptPending : 1;
				u32 DMA2InterruptPending : 1;
				u32 DMA3InterruptPending : 1;
				u32 DMA4InterruptPending : 1;
				u32 DMA5InterruptPending : 1;
				u32 DMA6InterruptPending : 1;
				
				// bit 31 - ?? should be 1 for acknowledging ??
				// bit 31 - should be 1 when any of the interrupt pending bits are set
				// only triggers interrupt on 0-to-1 transition
				u32 DMAInterruptPending : 1;
			};
			
			u32 Value;
		};
	
		//ICR_Format ICR_Reg;
		
#ifdef PS2_COMPILE
		//ICR_Format ICR2_Reg;
		
		// also include 0x1f801578
		//u32 lReg_1578;
		
		// more CTRL Regs for DMA on PS2
		//u32 lSIF0_CTRL;
		//u32 lSIF1_CTRL;
		//u32 lSIF2_CTRL;
#endif

		
		static const u32 DMA0_MADR = 0x1f801080;
		static const u32 DMA0_BCR = 0x1f801084;
		static const u32 DMA0_CHCR = 0x1f801088;

		static const u32 DMA1_MADR = 0x1f801090;
		static const u32 DMA1_BCR = 0x1f801094;
		static const u32 DMA1_CHCR = 0x1f801098;

		static const u32 DMA2_MADR = 0x1f8010a0;
		static const u32 DMA2_BCR = 0x1f8010a4;
		static const u32 DMA2_CHCR = 0x1f8010a8;

		static const u32 DMA3_MADR = 0x1f8010b0;
		static const u32 DMA3_BCR = 0x1f8010b4;
		static const u32 DMA3_CHCR = 0x1f8010b8;

		static const u32 DMA4_MADR = 0x1f8010c0;
		static const u32 DMA4_BCR = 0x1f8010c4;
		static const u32 DMA4_CHCR = 0x1f8010c8;

		static const u32 DMA5_MADR = 0x1f8010d0;
		static const u32 DMA5_BCR = 0x1f8010d4;
		static const u32 DMA5_CHCR = 0x1f8010d8;

		static const u32 DMA6_MADR = 0x1f8010e0;
		static const u32 DMA6_BCR = 0x1f8010e4;
		static const u32 DMA6_CHCR = 0x1f8010e8;

#ifdef PS2_COMPILE

		static const u32 DMA7_MADR = 0x1f801500;
		static const u32 DMA7_BCR = 0x1f801504;
		static const u32 DMA7_CHCR = 0x1f801508;

		static const u32 DMA8_MADR = 0x1f801510;
		static const u32 DMA8_BCR = 0x1f801514;
		static const u32 DMA8_CHCR = 0x1f801518;

		static const u32 DMA9_MADR = 0x1f801520;
		static const u32 DMA9_BCR = 0x1f801524;
		static const u32 DMA9_CHCR = 0x1f801528;
		static const u32 DMA9_TADR = 0x1f80152c;

		static const u32 DMA10_MADR = 0x1f801530;
		static const u32 DMA10_BCR = 0x1f801534;
		static const u32 DMA10_CHCR = 0x1f801538;

		static const u32 DMA11_MADR = 0x1f801540;
		static const u32 DMA11_BCR = 0x1f801544;
		static const u32 DMA11_CHCR = 0x1f801548;
		static const u32 DMA11_TADR = 0x1f80154c;

		static const u32 DMA12_MADR = 0x1f801550;
		static const u32 DMA12_BCR = 0x1f801554;
		static const u32 DMA12_CHCR = 0x1f801558;

		static const u32 DMA_SIF0_CTRL = 0x1f801560;
		static const u32 DMA_SIF1_CTRL = 0x1f801564;
		static const u32 DMA_SIF2_CTRL = 0x1f801568;
		
		
		static const u32 PCR2 = 0x1f801570;
		static const u32 ICR2 = 0x1f801574;
		
		static const u32 REG_1578 = 0x1f801578;

		static const u32 REG_157C = 0x1f80157c;

#endif

		// 0x1f801080 - 0x1f8010fc
		// PCR: 0x1f8010f0
		// ICR: 0x1f8010f4
		union DMARegs0_t
		{
			struct
			{
				// DMA0: 0x1f801080
				unsigned long DMA0_MADR;
				DmaChannel::BCR_Format DMA0_BCR;
				DmaChannel::CHCR_Format DMA0_CHCR;
				DmaChannel::CHCR_Format DMA0_CHCRm;
				
				// DMA1: 0x1f801090
				unsigned long DMA1_MADR;
				DmaChannel::BCR_Format DMA1_BCR;
				DmaChannel::CHCR_Format DMA1_CHCR;
				DmaChannel::CHCR_Format DMA1_CHCRm;
				
				// DMA2: 0x1f8010a0
				unsigned long DMA2_MADR;
				DmaChannel::BCR_Format DMA2_BCR;
				DmaChannel::CHCR_Format DMA2_CHCR;
				DmaChannel::CHCR_Format DMA2_CHCRm;
				
				// DMA3: 0x1f8010b0
				unsigned long DMA3_MADR;
				DmaChannel::BCR_Format DMA3_BCR;
				DmaChannel::CHCR_Format DMA3_CHCR;
				DmaChannel::CHCR_Format DMA3_CHCRm;
				
				// DMA4: 0x1f8010c0
				unsigned long DMA4_MADR;
				DmaChannel::BCR_Format DMA4_BCR;
				DmaChannel::CHCR_Format DMA4_CHCR;
				DmaChannel::CHCR_Format DMA4_CHCRm;
				
				// DMA5: 0x1f8010d0
				unsigned long DMA5_MADR;
				DmaChannel::BCR_Format DMA5_BCR;
				DmaChannel::CHCR_Format DMA5_CHCR;
				DmaChannel::CHCR_Format DMA5_CHCRm;
				
				// DMA6: 0x1f8010e0
				unsigned long DMA6_MADR;
				DmaChannel::BCR_Format DMA6_BCR;
				DmaChannel::CHCR_Format DMA6_CHCR;
				DmaChannel::CHCR_Format DMA6_CHCRm;
				
				// PCR: 0x1f8010f0
				PCR_Format PCR;
				
				// ICR: 0x1f8010f4
				ICR_Format ICR;
				
				// padding
				unsigned long Unk1;
				unsigned long Unk2;
			};
			
			DmaChannel::RegData RegData [ 8 ];
			
			// 30 32-bit registers
			// 0x1f801080-0x1f8010fc
			unsigned long Regs [ 8 * 4 ];
		};
		
		DMARegs0_t DMARegs0;


#ifdef PS2_COMPILE
		// 0x1f801500 - 0x1f80157c
		// PCR2: 0x1f801570
		// ICR2: 0x1f801574
		union DMARegs1_t
		{
			struct
			{
				// DMA7: 0x1f801500
				unsigned long DMA7_MADR;
				DmaChannel::BCR_Format DMA7_BCR;
				DmaChannel::CHCR_Format DMA7_CHCR;
				DmaChannel::CHCR_Format DMA7_CHCRm;
				
				// DMA8: 0x1f801510
				unsigned long DMA8_MADR;
				DmaChannel::BCR_Format DMA8_BCR;
				DmaChannel::CHCR_Format DMA8_CHCR;
				DmaChannel::CHCR_Format DMA8_CHCRm;
				
				// DMA9: 0x1f801520
				unsigned long DMA9_MADR;
				DmaChannel::BCR_Format DMA9_BCR;
				DmaChannel::CHCR_Format DMA9_CHCR;
				DmaChannel::CHCR_Format DMA9_TADR;
				
				// DMA10: 0x1f801530
				unsigned long DMA10_MADR;
				DmaChannel::BCR_Format DMA10_BCR;
				DmaChannel::CHCR_Format DMA10_CHCR;
				DmaChannel::CHCR_Format DMA10_CHCRm;
				
				// DMA11: 0x1f801540
				unsigned long DMA11_MADR;
				DmaChannel::BCR_Format DMA11_BCR;
				DmaChannel::CHCR_Format DMA11_CHCR;
				DmaChannel::CHCR_Format DMA11_TADR;
				
				// DMA12: 0x1f801550
				unsigned long DMA12_MADR;
				DmaChannel::BCR_Format DMA12_BCR;
				DmaChannel::CHCR_Format DMA12_CHCR;
				DmaChannel::CHCR_Format DMA12_CHCRm;
				
				// SIFX_CTRL: 0x1f801560
				unsigned long SIF0_CTRL;
				unsigned long SIF1_CTRL;
				unsigned long SIF2_CTRL;
				unsigned long Unk0;
				
				// PCR2: 0x1f801570
				PCR_Format PCR2;
				
				// ICR2: 0x1f801574
				ICR_Format ICR2;
				
				// REG_1578 ??
				// 0: disable dma transfers; 1: enable dma transfers
				unsigned long DMA_ENABLE;
				
				// padding
				unsigned long DMA_INTR;
			};
			
			DmaChannel::RegData RegData [ 8 ];
			
			// 30 32-bit registers
			// 0x1f801500-0x1f80157c
			unsigned long Regs [ 8 * 4 ];
		};
		
		DMARegs1_t DMARegs1;
#endif


		//////////////////////////////////////////////////////
		// Channel 0 - MDECin								//
		// Channel 1 - MDECout								//
		// Channel 2 - GPU									//
		// Channel 3 - CD									//
		// Channel 4 - SPU									//
		// Channel 5 - PIO									//
		// Channel 6 - OTC (Ordering Table Clear)			//
		// ------- below channels are PS2 IOP ONLY -------- //
		// Channel 7 - SPU2									//
		// Channel 8 - DEV9									//
		// Channel 9 - SIF0 (IOP->EE)						//
		// Channel 10 - SIF1 (EE->IOP)						//
		// Channel 11 - SIO2in								//
		// Channel 12 - SIO2out								//
		//////////////////////////////////////////////////////
		
		DmaChannel DmaCh [ c_iNumberOfChannels ];
		
		
		// static pointer to register data
		static DmaChannel::RegData* pRegData [ c_iNumberOfChannels ];

		////////////////////////////////////////////////////////////////
		// Also need the count and the next address for linked list
		u32 LL_Count;
		u32 LL_NextAddress;
		
#ifdef PS2_COMPILE
		// the amount of overhead for an SIF1 transfer incoming to PS1
		static const u64 c_llSIF0_DmaSetupTime = 0;	//32;
		static const u64 c_llSIF0_BusCyclesPerTransfer = 1;
		static const u64 c_llSIF0_WaitCyclesPerTransfer = 0;
		
		static const u64 c_llSIF0_TransferTime = 1;	//1;
		static const u64 c_llSIF0_BufferSize = 1000000;	//32;
		
		// not used
		static const u64 c_llSIF1_SetupTime = 2048;
#endif


		
		/////////////////////////////////////////////////////////////////
		// Need to be able to save the transfer size
		u32 TransferSize_Save [ c_iNumberOfChannels ];
		
		u32 ulBlockTotal [ c_iNumberOfChannels ];
		u32 ulBlockTransferred [ c_iNumberOfChannels ];
		
		// we need to know when an interrupt is being requested by dma, but also want to keep component separate for extensive testing
		//Intc::I_STAT_Format Interrupt_Status;

		// gets cleared when no transfer is made
		// 0: ok to transfer; 1: release bus
		u32 BusReleaseToggle;

		// need a pointer to the data bus object
		static DataBus *_BUS;
		
		// need to be able to get MDEC status
		static Playstation1::MDEC *_MDEC;
		
		// need to be able to get GPU status
		static Playstation1::GPU *_GPU;
		
		// need to be able to get CD status
		static Playstation1::CD *_CD;

		// need a reference to the spu
		static Playstation1::SPU *_SPU;
		
		// need a reference to cpu for invalidating cache
		//static R3000A::Cpu *_CPU;
		
		// state of interrupt - *testing*
		u32 Interrupt_State;
		
		// need function to perform a dma
		void StartDMA ( int iChannel );
		void DMA_Run ( int iChannel, bool CycleStealMode = false );
		
		
		// ends transfer without triggering an interrupt (needed for PS2)
		void SuspendTransfer ( int index );

		
		// check if it a dma channel has just been activated and needs to be started
		void DMA_Update ( int DmaChannelNum );
		
		// trigger interrupt for auto dma
		void AutoDMA_Interrupt ( int index );

#ifdef PS2_COMPILE

		void AutoDMA_Run ( int iChannel );
		void AutoDMA4_Run ();
		void AutoDMA7_Run ();
		
		// the number of cycles for an auto dma transfer for now (using 512 samples/transfers for now)
		static const int c_iAutoDmaTransfer_CycleTime = 512;
#endif


#ifdef PS2_COMPILE
		// returns the number of bytes read
		// Address - PS1 address to read data from for DMA transfer - no transfer if NULL
		// Data - pointer to 
		// Size - size in bytes
		u32 EE_DMA_Read ( int Channel, u32 Address, u32* Data );
		void EE_DMA_Write ( int Channel, u32 Address, u32* Data, u32 Size );
#endif

		
		// must use this to add a reference to the bus
		void ConnectDevices ( DataBus *BUS, MDEC *mdec, GPU *g, CD *cd, SPU *spu /* , R3000A::Cpu *cpu */ );
		
		// constructor
		//Dma ();

		static u64 OTC_ReadyForRead ();
		static u32 OTC_Transfer ( u32* pMemPtr, u32 Address, u32 Count );
		
		void Reset ();
		void Update_ICR ( u32 Data );
		
#ifdef PS2_COMPILE
		void Update_ICR2 ( u32 Data );
#endif
		
		u32 GetPriorityOffset ( int iChannel );
		
		// this is the dma channel that is currently transferring data
		u32 ActiveChannel;

		u32 LastActiveChannel;
		
		inline u32 GetPriorityScore ( int Channel )
		{
			u32 Enabled, Transfer, Priority, isTransferringNow;
			
			// get whether channel is enabled or not
			Enabled = isEnabled ( Channel );
			
			// priority 0 is highest priority, 7 the lowest
			Transfer = isActive ( Channel );
			
			// channel 7 has highest priority, 0 the lowest
			Priority = GetPriority ( Channel );
			
#ifdef USE_NEW_SCORING_SYSTEM
			isTransferringNow = isTransferring ( Channel );
			
			// priority score = Enabled * Transfer * ( ( Priority * c_iPriorityLevels ) + Channel + 1 )
			return ( Enabled * Transfer * ( ( ( c_iPriorityLevels - Priority ) * c_iNumberOfChannels ) + Channel ) * isTransferringNow ) + ( ( Enabled && Transfer ) ? Channel : 0 );
#else
			// priority score = Enabled * Transfer * ( ( Priority * c_iPriorityLevels ) + Channel + 1 )
			return Enabled * Transfer * ( ( ( c_iPriorityLevels - Priority ) * c_iNumberOfChannels ) + Channel );
#endif
		}
		
		
		
		void Refresh ()
		{
			for ( int i = 0; i < c_iNumberOfChannels; i++ )
			{
				pRegData [ i ] = & ( DMARegs0.RegData [ i ] );
				
#ifdef PS2_COMPILE
				if ( i > 6 )
				{
					pRegData [ i ] = & ( DMARegs1.RegData [ i - 7 ] );
				}
#endif
			}
		}
		
		u32 GetActiveChannel ();

		static void sRun () { _DMA->Run (); }
		static void Set_EventCallback ( funcVoid2 UpdateEvent_CB ) { _DMA->NextEvent_Idx = UpdateEvent_CB ( sRun ); }
		
		// for interrupt call back
		static funcVoid UpdateInterrupts;
		static void Set_IntCallback ( funcVoid UpdateInt_CB ) { UpdateInterrupts = UpdateInt_CB; };
		
		
		static const u32 c_InterruptBit = 3;

		
		//static u32* _Intc_Master;
		static u32* _Intc_Stat;
		static u32* _Intc_Mask;
		static u32* _R3000A_Status_12;
		static u32* _R3000A_Cause_13;
		static u64* _ProcStatus;
		
		inline void ConnectInterrupt ( u32* _IStat, u32* _IMask, u32* _R3000A_Status, u32* _R3000A_Cause, u64* _ProcStat )
		{
			_Intc_Stat = _IStat;
			_Intc_Mask = _IMask;
			//_Intc_Master = _IMaster;
			_R3000A_Cause_13 = _R3000A_Cause;
			_R3000A_Status_12 = _R3000A_Status;
			_ProcStatus = _ProcStat;
		}
		
		
		inline void SetInterrupt ()
		{
			//*_Intc_Master |= ( 1 << c_InterruptBit );
			*_Intc_Stat |= ( 1 << c_InterruptBit );
			
			UpdateInterrupts ();
			
			/*
			if ( *_Intc_Stat & *_Intc_Mask ) *_R3000A_Cause_13 |= ( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}


		// this probably should not be used
		inline void ClearInterrupt ()
		{
			//*_Intc_Master &= ~( 1 << c_InterruptBit );
			*_Intc_Stat &= ~( 1 << c_InterruptBit );
			
			UpdateInterrupts ();
			
			/*
			if ( ! ( *_Intc_Stat & *_Intc_Mask ) ) *_R3000A_Cause_13 &= ~( 1 << 10 );
			
			if ( ( *_R3000A_Cause_13 & *_R3000A_Status_12 & 0xff00 ) && ( *_R3000A_Status_12 & 1 ) ) *_ProcStatus |= ( 1 << 20 );
			*/
		}



		static u64* _NextSystemEvent;
		
		////////////////////////////////
		// Debug Info
		static u32* _DebugPC;
		static u64* _DebugCycleCount;
		static u64* _SystemCycleCount;
		static u32 *_NextEventIdx;
		//static u32* _Intc_Mask;
		//static u32* _Intc_Stat;
		static u32* _R3000a_Status;
		
		static bool DebugWindow_Enabled;
		static WindowClass::Window *DebugWindow;
		static DebugValueList<u32> *DMA_ValueList;
		static void DebugWindow_Enable ();
		static void DebugWindow_Disable ();
		static void DebugWindow_Update ();
		

	
		
	private:
	
		//u32 DMA_Finished ( int index );
		void DMA_Finished ( int index, bool SuppressDMARestart = false, bool SuppressEventUpdate = false, bool SuppressDMAStop = false );
		u32 DMA_Interrupt_Update ();

	};
	
};

#endif

