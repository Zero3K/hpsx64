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


#ifndef _R5900_ENCODER_H_
#define _R5900_ENCODER_H_


namespace R5900
{

	class Encoder
	{
	public:
	
		// *** PS1 INSTRUCTIONS *** //
	
		static unsigned long ADDIU ( long rt, long rs, short Immediate );
		static unsigned long ANDI ( long rt, long rs, short Immediate );
		static unsigned long ORI ( long rt, long rs, short Immediate );
		static unsigned long SLTI ( long rt, long rs, short Immediate );
		static unsigned long SLTIU ( long rt, long rs, short Immediate );
		static unsigned long XORI ( long rt, long rs, short Immediate );

		static unsigned long ADDI ( long rt, long rs, short Immediate );

		static unsigned long ADDU ( long rd, long rs, long rt );
		static unsigned long AND ( long rd, long rs, long rt );
		static unsigned long OR ( long rd, long rs, long rt );
		static unsigned long NOR ( long rd, long rs, long rt );
		static unsigned long SLT ( long rd, long rs, long rt );
		static unsigned long SLTU ( long rd, long rs, long rt );
		static unsigned long SUBU ( long rd, long rs, long rt );
		static unsigned long XOR ( long rd, long rs, long rt );

		static unsigned long ADD ( long rd, long rs, long rt );
		static unsigned long SUB ( long rd, long rs, long rt );
		
		static unsigned long DIV ( long rs, long rt );
		static unsigned long DIVU ( long rs, long rt );
		
		static unsigned long MULT ( long rs, long rt );
		static unsigned long MULTU ( long rs, long rt );
		
		static unsigned long SLL ( long rd, long rt, long sa );
		static unsigned long SRA ( long rd, long rt, long sa );
		static unsigned long SRL ( long rd, long rt, long sa );
		
		static unsigned long SLLV ( long rd, long rt, long rs );
		static unsigned long SRAV ( long rd, long rt, long rs );
		static unsigned long SRLV ( long rd, long rt, long rs );
		
		static unsigned long J ( long target );
		static unsigned long JR ( long rs );
		static unsigned long JAL ( long target );
		static unsigned long JALR ( long rd, long rs );
		
		static unsigned long BEQ ( long rs, long rt, short offset );
		static unsigned long BNE ( long rs, long rt, short offset );
		static unsigned long BGEZ ( long rs, short offset );
		static unsigned long BGTZ ( long rs, short offset );
		static unsigned long BLEZ ( long rs, short offset );
		static unsigned long BLTZ ( long rs, short offset );

		static unsigned long BGEZAL ( long rs, short offset );
		static unsigned long BLTZAL ( long rs, short offset );
		
		static unsigned long LUI ( long rt, short immediate );

		static unsigned long MFHI ( long rd );
		static unsigned long MFLO ( long rd );
		static unsigned long MTHI ( long rs );
		static unsigned long MTLO ( long rs );

		static unsigned long SYSCALL ();
		static unsigned long BREAK ();
		//static unsigned long RFE ();

		static unsigned long MFC0 ( long rt, long rd );
		static unsigned long MTC0 ( long rt, long rd );
		
		//static unsigned long MFC2 ( long rt, long rd );
		//static unsigned long MTC2 ( long rt, long rd );
		static unsigned long CFC2 ( long rt, long rd );
		static unsigned long CTC2 ( long rt, long rd );
		
		//static unsigned long COP2 ( long Command );
		
		static unsigned long LB ( long rt, short offset, long base );
		static unsigned long LH ( long rt, short offset, long base );
		static unsigned long LWL ( long rt, short offset, long base );
		static unsigned long LW ( long rt, short offset, long base );
		static unsigned long LBU ( long rt, short offset, long base );
		static unsigned long LHU ( long rt, short offset, long base );
		static unsigned long LWR ( long rt, short offset, long base );
		static unsigned long SB ( long rt, short offset, long base );
		static unsigned long SH ( long rt, short offset, long base );
		static unsigned long SWL ( long rt, short offset, long base );
		static unsigned long SW ( long rt, short offset, long base );
		static unsigned long SWR ( long rt, short offset, long base );
		
		//static unsigned long LWC2 ( long rt, short offset, long base );
		//static unsigned long SWC2 ( long rt, short offset, long base );
		
		
		// *** PS2 INSTRUCTIONS *** //

		static unsigned long BC0T ( short offset );
		static unsigned long BC0TL ( short offset );
		static unsigned long BC0F ( short offset );
		static unsigned long BC0FL ( short offset );
		static unsigned long BC1T ( short offset );
		static unsigned long BC1TL ( short offset );
		static unsigned long BC1F ( short offset );
		static unsigned long BC1FL ( short offset );
		static unsigned long BC2T ( short offset );
		static unsigned long BC2TL ( short offset );
		static unsigned long BC2F ( short offset );
		static unsigned long BC2FL ( short offset );
		
		// ** note: MF0/MT0 should become CFC0/CTC0
		static unsigned long CFC0 ( long rt, long rd );
		static unsigned long CTC0 ( long rt, long rd );
		static unsigned long EI ();
		static unsigned long DI ();
		
		static unsigned long SD ( long rt, short offset, long base );
		static unsigned long LD ( long rt, short offset, long base );
		static unsigned long LWU ( long rt, short offset, long base );
		static unsigned long SDL ( long rt, short offset, long base );
		static unsigned long SDR ( long rt, short offset, long base );
		static unsigned long LDL ( long rt, short offset, long base );
		static unsigned long LDR ( long rt, short offset, long base );
		static unsigned long LQ ( long rt, short offset, long base );
		static unsigned long SQ ( long rt, short offset, long base );
			
			
		// arithemetic instructions //
		static unsigned long DADD ( long rd, long rs, long rt );
		static unsigned long DADDU ( long rd, long rs, long rt );
		static unsigned long DSUB ( long rd, long rs, long rt );
		static unsigned long DSUBU ( long rd, long rs, long rt );
		
		static unsigned long DADDI ( long rt, long rs, short Immediate );
		static unsigned long DADDIU ( long rt, long rs, short Immediate );
		
		static unsigned long DSLL ( long rd, long rt, long sa );
		static unsigned long DSLL32 ( long rd, long rt, long sa );
		static unsigned long DSRA ( long rd, long rt, long sa );
		static unsigned long DSRA32 ( long rd, long rt, long sa );
		static unsigned long DSRL ( long rd, long rt, long sa );
		static unsigned long DSRL32 ( long rd, long rt, long sa );
		
		static unsigned long DSLLV ( long rd, long rt, long rs );
		static unsigned long DSRAV ( long rd, long rt, long rs );
		static unsigned long DSRLV ( long rd, long rt, long rs );
			
			

		static unsigned long MFC1 ( long rt, long fs );
		static unsigned long CFC1 ( long rt, long fs );
		static unsigned long MTC1 ( long rt, long fs );
		static unsigned long CTC1 ( long rt, long fs );
		
		static unsigned long BEQL ( long rs, long rt, short offset );
		static unsigned long BNEL ( long rs, long rt, short offset );
		
		static unsigned long BGEZL ( long rs, short offset );
		static unsigned long BLEZL ( long rs, short offset );
		static unsigned long BGTZL ( long rs, short offset );
		static unsigned long BLTZL ( long rs, short offset );
		static unsigned long BLTZALL ( long rs, short offset );
		static unsigned long BGEZALL ( long rs, short offset );
		
		static unsigned long TGE ( long rs, long rt );
		static unsigned long TGEU ( long rs, long rt );
		static unsigned long TLT ( long rs, long rt );
		static unsigned long TLTU ( long rs, long rt );
		static unsigned long TEQ ( long rs, long rt );
		static unsigned long TNE ( long rs, long rt );
		
		static unsigned long TGEI ( long rs, short immediate );
		static unsigned long TGEIU ( long rs, short immediate );
		static unsigned long TLTI ( long rs, short immediate );
		static unsigned long TLTIU ( long rs, short immediate );
		static unsigned long TEQI ( long rs, short immediate );
		static unsigned long TNEI ( long rs, short immediate );
		
		static unsigned long MOVZ ( long rd, long rs, long rt );
		static unsigned long MOVN ( long rd, long rs, long rt );
		
		// ??
		static unsigned long MOVCI ();
		static unsigned long SYNC ();
		
		static unsigned long MFHI1 ( long rd );
		static unsigned long MFLO1 ( long rd );
		static unsigned long MTHI1 ( long rs );
		static unsigned long MTLO1 ( long rs );
		static unsigned long MULT1 ( long rd, long rs, long rt );
		static unsigned long MULTU1 ( long rd, long rs, long rt );
		static unsigned long DIV1 ( long rs, long rt );
		static unsigned long DIVU1 ( long rs, long rt );
		static unsigned long MADD ( long rd, long rs, long rt );
		static unsigned long MADD1 ( long rd, long rs, long rt );
		static unsigned long MADDU ( long rd, long rs, long rt );
		static unsigned long MADDU1 ( long rd, long rs, long rt );
		
		static unsigned long MFSA ( long rd );
		static unsigned long MTSA ( long rs );
		static unsigned long MTSAB ( long rs, short immediate );
		static unsigned long MTSAH ( long rs, short immediate );
		
		static unsigned long TLBR ();
		static unsigned long TLBWI ();
		static unsigned long TLBWR ();
		static unsigned long TLBP ();
		
		static unsigned long ERET ();
		
		static unsigned long CACHE ( long op, short offset, long base );
		static unsigned long PREF ( long hint, short offset, long base );
		
		// note: there does not appear to be a DERET or WAIT instruction on R5900
		static unsigned long DERET ();
		static unsigned long WAIT ();
			
		// Parallel instructions (SIMD) //
		static unsigned long PABSH ( long rd, long rt );
		static unsigned long PABSW ( long rd, long rt );
		
		static unsigned long PADDB ( long rd, long rs, long rt );
		static unsigned long PADDH ( long rd, long rs, long rt );
		static unsigned long PADDW ( long rd, long rs, long rt );
		static unsigned long PADDSB ( long rd, long rs, long rt );
		static unsigned long PADDSH ( long rd, long rs, long rt );
		static unsigned long PADDSW ( long rd, long rs, long rt );
		static unsigned long PADDUB ( long rd, long rs, long rt );
		static unsigned long PADDUH ( long rd, long rs, long rt );
		static unsigned long PADDUW ( long rd, long rs, long rt );
		static unsigned long PADSBH ( long rd, long rs, long rt );
		static unsigned long PAND ( long rd, long rs, long rt );
		static unsigned long POR ( long rd, long rs, long rt );
		static unsigned long PXOR ( long rd, long rs, long rt );
		static unsigned long PNOR ( long rd, long rs, long rt );
		static unsigned long PCEQB ( long rd, long rs, long rt );
		static unsigned long PCEQH ( long rd, long rs, long rt );
		static unsigned long PCEQW ( long rd, long rs, long rt );
		static unsigned long PCGTB ( long rd, long rs, long rt );
		static unsigned long PCGTH ( long rd, long rs, long rt );
		static unsigned long PCGTW ( long rd, long rs, long rt );
		static unsigned long PCPYLD ( long rd, long rs, long rt );
		static unsigned long PCPYUD ( long rd, long rs, long rt );
		static unsigned long PEXTLB ( long rd, long rs, long rt );
		static unsigned long PEXTLH ( long rd, long rs, long rt );
		static unsigned long PEXTLW ( long rd, long rs, long rt );
		static unsigned long PEXTUB ( long rd, long rs, long rt );
		static unsigned long PEXTUH ( long rd, long rs, long rt );
		static unsigned long PEXTUW ( long rd, long rs, long rt );
		static unsigned long PHMADH ( long rd, long rs, long rt );
		static unsigned long PHMSBH ( long rd, long rs, long rt );
		static unsigned long PINTEH ( long rd, long rs, long rt );
		static unsigned long PINTH ( long rd, long rs, long rt );
		static unsigned long PMSUBH ( long rd, long rs, long rt );
		static unsigned long PMSUBW ( long rd, long rs, long rt );
		static unsigned long PMULTH ( long rd, long rs, long rt );
		static unsigned long PMULTW ( long rd, long rs, long rt );
		static unsigned long PMULTUW ( long rd, long rs, long rt );
		static unsigned long PMADDH ( long rd, long rs, long rt );
		static unsigned long PMADDW ( long rd, long rs, long rt );
		static unsigned long PMADDUW ( long rd, long rs, long rt );
		static unsigned long PMAXH ( long rd, long rs, long rt );
		static unsigned long PMAXW ( long rd, long rs, long rt );
		static unsigned long PMINH ( long rd, long rs, long rt );
		static unsigned long PMINW ( long rd, long rs, long rt );
		static unsigned long PPACB ( long rd, long rs, long rt );
		static unsigned long PPACH ( long rd, long rs, long rt );
		static unsigned long PPACW ( long rd, long rs, long rt );
		static unsigned long PSUBB ( long rd, long rs, long rt );
		static unsigned long PSUBH ( long rd, long rs, long rt );
		static unsigned long PSUBW ( long rd, long rs, long rt );
		static unsigned long PSUBSB ( long rd, long rs, long rt );
		static unsigned long PSUBSH ( long rd, long rs, long rt );
		static unsigned long PSUBSW ( long rd, long rs, long rt );
		static unsigned long PSUBUB ( long rd, long rs, long rt );
		static unsigned long PSUBUH ( long rd, long rs, long rt );
		static unsigned long PSUBUW ( long rd, long rs, long rt );

			
			static unsigned long PCPYH ();
			
			static unsigned long PDIVBW ();
			static unsigned long PDIVUW ();
			static unsigned long PDIVW ();
			
			static unsigned long PEXCH ();
			static unsigned long PEXCW ();
			static unsigned long PEXEH ();
			static unsigned long PEXEW ();
			static unsigned long PEXT5 ();
			
			
			
			
			
			static unsigned long PLZCW ();
			
			
			
			static unsigned long PMFHI ();
			static unsigned long PMFLO ();
			static unsigned long PMTHI ();
			static unsigned long PMTLO ();
			static unsigned long PMFHL_LH ();
			static unsigned long PMFHL_SH ();
			static unsigned long PMFHL_LW ();
			static unsigned long PMFHL_UW ();
			static unsigned long PMFHL_SLW ();
			static unsigned long PMTHL_LW ();
			
			
			
			static unsigned long PPAC5 ();
			
			
			
			static unsigned long PREVH ();
			static unsigned long PROT3W ();
			static unsigned long PSLLH ();
			static unsigned long PSLLVW ();
			static unsigned long PSLLW ();
			static unsigned long PSRAH ();
			static unsigned long PSRAW ();
			static unsigned long PSRAVW ();
			static unsigned long PSRLH ();
			static unsigned long PSRLW ();
			static unsigned long PSRLVW ();
			
			
			static unsigned long QFSRV ();
			

			// floating point instructions //

			static unsigned long LWC1 ( long rt, short offset, long base );
			static unsigned long SWC1 ( long rt, short offset, long base );
			
			static unsigned long ABS_S ();
			static unsigned long ADD_S ();
			static unsigned long ADDA_S ();
			static unsigned long C_EQ_S ();
			static unsigned long C_F_S ();
			static unsigned long C_LE_S ();
			static unsigned long C_LT_S ();
			static unsigned long CVT_S_W ();
			static unsigned long CVT_W_S ();
			static unsigned long DIV_S ();
			static unsigned long MADD_S ();
			static unsigned long MADDA_S ();
			static unsigned long MAX_S ();
			static unsigned long MIN_S ();
			static unsigned long MOV_S ();
			static unsigned long MSUB_S ();
			static unsigned long MSUBA_S ();
			static unsigned long MUL_S ();
			static unsigned long MULA_S ();
			static unsigned long NEG_S ();
			static unsigned long RSQRT_S ();
			static unsigned long SQRT_S ();
			static unsigned long SUB_S ();
			static unsigned long SUBA_S ();
			

			// PS2 COP2 instructions //

			static unsigned long LQC2 ();
			static unsigned long SQC2 ();
			static unsigned long QMFC2_NI ();
			static unsigned long QMTC2_NI ();
			static unsigned long QMFC2_I ();
			static unsigned long QMTC2_I ();
			
			
			static unsigned long VADDBCX ();
			static unsigned long VADDBCY ();
			static unsigned long VADDBCZ ();
			static unsigned long VADDBCW ();
			static unsigned long VSUBBCX ();
			static unsigned long VSUBBCY ();
			static unsigned long VSUBBCZ ();
			static unsigned long VSUBBCW ();
			static unsigned long VMULBCX ();
			static unsigned long VMULBCY ();
			static unsigned long VMULBCZ ();
			static unsigned long VMULBCW ();
			static unsigned long VMADDBCX ();
			static unsigned long VMADDBCY ();
			static unsigned long VMADDBCZ ();
			static unsigned long VMADDBCW ();
			static unsigned long VMSUBBCX ();
			static unsigned long VMSUBBCY ();
			static unsigned long VMSUBBCZ ();
			static unsigned long VMSUBBCW ();
			static unsigned long VMAXBCX ();
			static unsigned long VMAXBCY ();
			static unsigned long VMAXBCZ ();
			static unsigned long VMAXBCW ();
			static unsigned long VMINIBCX ();
			static unsigned long VMINIBCY ();
			static unsigned long VMINIBCZ ();
			static unsigned long VMINIBCW ();
			
			static unsigned long VADD ();
			static unsigned long VADDI ();
			static unsigned long VADDQ ();
			static unsigned long VSUB ();
			static unsigned long VSUBI ();
			static unsigned long VSUBQ ();
			static unsigned long VMUL ();
			static unsigned long VMULI ();
			static unsigned long VMULQ ();
			static unsigned long VMAX ();
			static unsigned long VMAXI ();
			static unsigned long VMINI ();
			static unsigned long VMINII ();
			static unsigned long VMADD ();
			static unsigned long VMADDI ();
			static unsigned long VMADDQ ();
			static unsigned long VMSUB ();
			static unsigned long VMSUBI ();
			static unsigned long VMSUBQ ();
			static unsigned long VDIV ();
			
			static unsigned long VADDA ();
			static unsigned long VADDAI ();
			static unsigned long VADDAQ ();
			static unsigned long VSUBA ();
			static unsigned long VADDABC ();
			static unsigned long VSUBAI ();
			static unsigned long VSUBAQ ();
			static unsigned long VSUBABC ();
			static unsigned long VMULA ();
			static unsigned long VMULAI ();
			static unsigned long VMULAQ ();
			static unsigned long VMULABC ();
			static unsigned long VMADDA ();
			static unsigned long VMADDAI ();
			static unsigned long VMADDAQ ();
			static unsigned long VMADDABC ();
			static unsigned long VMSUBA ();
			static unsigned long VMSUBAI ();
			static unsigned long VMSUBAQ ();
			static unsigned long VMSUBABC ();
			
			static unsigned long VOPMULA ();
			static unsigned long VOPMSUM ();
			static unsigned long VOPMSUB ();

			static unsigned long VNOP ();
			static unsigned long VABS ();
			static unsigned long VCLIP ();
			static unsigned long VSQRT ();
			static unsigned long VRSQRT ();
			static unsigned long VMR32 ();
			static unsigned long VRINIT ();
			static unsigned long VRGET ();
			static unsigned long VRNEXT ();
			static unsigned long VRXOR ();
			static unsigned long VMOVE ();
			static unsigned long VMFIR ();
			static unsigned long VMTIR ();
			static unsigned long VLQD ();
			static unsigned long VLQI ();
			static unsigned long VSQD ();
			static unsigned long VSQI ();
			static unsigned long VWAITQ ();
			
			static unsigned long VFTOI0 ();
			static unsigned long VITOF0 ();
			static unsigned long VFTOI4 ();
			static unsigned long VITOF4 ();
			static unsigned long VFTOI12 ();
			static unsigned long VITOF12 ();
			static unsigned long VFTOI15 ();
			static unsigned long VITOF15 ();
			
			static unsigned long VIADD ();
			static unsigned long VISUB ();
			static unsigned long VIADDI ();
			static unsigned long VIAND ();
			static unsigned long VIOR ();
			static unsigned long VILWR ();
			static unsigned long VISWR ();
			static unsigned long VCALLMS ();
			static unsigned long VCALLMSR ();
		
	};
	
};

#endif

