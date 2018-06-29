#ifndef DEFINES
#define DEFINES
#include <stdio.h>
#include <stdlib.h>

#define MEM_SIZE 256

//bits que representam cada um dos sinais de controle

#define RegDst0_bit 		0x00000001
#define RegDst1_bit 		0x00000002
#define RegWrite_bit 		0x00000004
#define ALUSrcA_bit 		0x00000008
#define ALUSrcB0_bit 		0x00000010
#define ALUSrcB1_bit 		0x00000020
#define ALUOp0_bit 		0x00000040
#define ALUOp1_bit 		0x00000080
#define PCSrc0_bit 		0x00000100
#define PCSrc1_bit 		0x00000200
#define PCWriteCond_bit 	0x00000400
#define PCWrite_bit 		0x00000800
#define IorD_bit 		0x00001000
#define MemRead_bit 		0x00002000
#define MemWrite_bit 		0x00004000
#define BNE_bit 		0x00008000
#define IRWrite_bit 		0x00010000
#define MemtoReg0_bit 		0x00020000
#define MemtoReg1_bit 		0x00040000
#define halt_bit		0x00080000
#define estado0_bit 		0x00100000
#define estado1_bit 		0x00200000
#define estado2_bit 		0x00400000
#define estado3_bit 		0x00800000
#define estado4_bit 		0x01000000

//funcao para pegar o valor de cada sinal de controle como 0 ou 1

#define RegDst0 	((RegDst0_bit&UC)	    )
#define RegDst1 	((RegDst1_bit&UC)	>> 1)
#define RegWrite 	((RegWrite_bit&UC)	>> 2)
#define ALUSrcA 	((ALUSrcA_bit&UC)	>> 3)
#define ALUSrcB0 	((ALUSrcB0_bit&UC)	>> 4)
#define ALUSrcB1 	((ALUSrcB1_bit&UC)	>> 5)
#define ALUOp0 		((ALUOp0_bit&UC)	>> 6)
#define ALUOp1 		((ALUOp1_bit&UC)	>> 7)
#define PCSrc0 		((PCSrc0_bit&UC)	>> 8)
#define PCSrc1 		((PCSrc1_bit&UC)	>> 9)
#define PCWriteCond 	((PCWriteCond_bit&UC)	>>10)
#define PCWrite 	((PCWrite_bit&UC)	>>11)
#define IorD 		((IorD_bit &UC)		>>12)
#define MemRead 	((MemRead_bit&UC)	>>13)
#define MemWrite 	((MemWrite_bit&UC)	>>14)
#define BNE 		((BNE_bit&UC)		>>15)
#define IRWrite 	((IRWrite_bit&UC)	>>16)
#define MemtoReg0 	((MemtoReg0_bit&UC)	>>17)
#define MemtoReg1 	((MemtoReg1_bit&UC)	>>18)
#define HALT 		((halt_bit&UC)		>>19)
#define estado0 	((estado0_bit&UC)	>>20)
#define estado1 	((estado1_bit&UC)	>>21)
#define estado2 	((estado2_bit&UC)	>>22)
#define estado3 	((estado3_bit&UC)	>>23)
#define estado4 	((estado4_bit&UC)	>>24)
#define ESTADO 		(((estado0_bit|estado1_bit|estado2_bit|estado3_bit|estado4_bit)&UC)>>20)

//funcao para pegar cada um dos campos do IR
#define IR_OPCODE 	((IR & 0xfc000000)>>26)
#define IR_ADDR 	((IR & 0x03ffffff))
#define IR_RS 		((IR & 0x03e00000)>>21)
#define IR_RT 		((IR & 0x001f0000)>>16)
#define IR_IMM 		((IR & 0x0000ffff))
#define IR_RD 		((IR & 0x0000f800)>>11)
#define IR_SHAMT 	((IR & 0x000007c0)>>4)
#define IR_FUNC 	((IR & 0x0000003f))
#define IRBit(x) 	((IR>>(31-x)) & 1)

//codigo binario para cada uma das operacoes
#define LW 	35	//100011
#define SW 	43	//101011
#define RType 	0	//000000
#define BEQ 	4	//000100
#define ADDI 	8	//001000
#define BNECODE 5	//000101
#define JMP 	2	//000010
#define JAL 	3	//000011
#define JR 	20	//010100
#define JALR 	21	//010101
#define ANDI 	12	//001100

//codigo do campo FUNC para as instrucoes do tipo R
#define ADD_CODE 32	//100000
#define SUB_CODE 34	//100010
#define SLT_CODE 42	//101010
#define AND_CODE 36	//100100
#define OR_CODE  37	//100101
#endif
