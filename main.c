/*
 * CODIGO DESENVOLVIDO POR: Bruno Piazera Larsen, N USP: 9283872
*/ 

#include "defs.h"

int prog_size;
unsigned char mem[MEM_SIZE];

int PC, IR, A, B, ALUOut, MDR;
int novo_PC, novo_IR, novo_A, novo_B, novo_ALUOut, novo_MDR;
int BR[32];
int UC;//sinais de controle,o bit 19 representa o sinal halt, para saber quando a instrucao eh invalida, e os bits 20-24 representam o estado
int UCA;//sinais de controle da ULA auxiliar
char zero_bit,halt_reason;

void carrega_prog(FILE* f){
	char* tmp=NULL;
	size_t s;
	int cnt=0,i,x;
	while(getline(&tmp,&s,f)>0){
		cnt+=4;
		free(tmp);
		tmp=NULL;
	}
	fseek(f,0,SEEK_SET);
	for(i=0;i<cnt;i+=4){
		fscanf(f," %d",&x);
		mem[i+3]=(x&0xff);
		x>>=8;
		mem[i+2]=(x&0xff);
		x>>=8;
		mem[i+1]=(x&0xff);
		x>>=8;
		mem[i]=x&0xff;
	}
	prog_size=cnt;
}

void print_bin(int k){
	for(int w,j=31;j>=0;j--){
		w=1<<j;
		printf("%d",((k&w)!=0));
	}
	printf("\n");
}

int resolve_mux_MAR(){
	return PC*(!IorD) + ALUOut*IorD;
}

int BNE_mux(){
	return (!BNE)*zero_bit+(BNE)*(!zero_bit);
}

int shift_and_cat(){
	return (IR_ADDR<<2)|(PC&0xf0000000);
}

int novo_PC_mux(){
	return novo_ALUOut*((!PCSrc0)&(!PCSrc1)) + ALUOut*((!PCSrc1)&PCSrc0)+shift_and_cat()*(PCSrc1&(!PCSrc0))+A*(PCSrc0&PCSrc1);
}

//grandes blocos a serem resolvidos a todo ciclo de clock:
//acesso a memoria
void resolve_memoria(){
	int MAR = resolve_mux_MAR(),tmp;
	if(MemRead){
		tmp=mem[MAR];
		tmp=(tmp<<8)+mem[MAR+1];
		tmp=(tmp<<8)+mem[MAR+2];
		tmp=(tmp<<8)+mem[MAR+3];
		novo_IR=tmp*MemRead;
		novo_MDR=tmp*MemRead;
	}else if(MemWrite){
		tmp=BR[IR_RT];
		mem[MAR+3]=tmp&(0xff);
		tmp>>=8;
		mem[MAR+2]=tmp&(0xff);
		tmp>>=8;
		mem[MAR+1]=tmp&(0xff);
		tmp>>=8;
		mem[MAR+1]=tmp&(0xff);
	}
}

int addr_BR_mux(){
	return 31*RegDst1+(!RegDst1)*(RegDst0*IR_RD+(!RegDst0)*IR_RT);
}

int data_BR_mux(){
	return (MemtoReg1)*PC+(!MemtoReg1)*((!MemtoReg0)*ALUOut+MemtoReg0*MDR);
}

//acesso ao BR
void resolve_BR(){
	novo_A=BR[IR_RS];
	novo_B=BR[IR_RT];
	if(RegWrite){
		if(addr_BR_mux()>0){ //simula o fato de $zero ser sempre 0
			BR[addr_BR_mux()]=data_BR_mux();
		}
	}
}

int ALU_A_mux(){
	return PC*(!ALUSrcA)+A*ALUSrcA;
}

int sign_extend(){
	return IR_IMM|(((IR_IMM&0x00008000)>>15)*(0xffff0000));
}

int ALU_B_mux(){
	return (!ALUSrcB1)*(ALUSrcB0*4+(!ALUSrcB0)*B)+(ALUSrcB1)*(sign_extend()<<(ALUSrcB0*2));
}

//faz a operacao da ULA
void resolve_ULA(){
	if((!ALUOp0) && (!ALUOp1)){
		novo_ALUOut=ALU_A_mux()+ALU_B_mux();
	}else if(ALUOp0 && (!ALUOp1)){
		novo_ALUOut=ALU_A_mux()-ALU_B_mux();
		zero_bit=(novo_ALUOut==0);
	}else if((!ALUOp0) && ALUOp1){
		if(IR_FUNC==ADD_CODE)
			novo_ALUOut=ALU_A_mux()+ALU_B_mux();
		else if(IR_FUNC==SUB_CODE){
			novo_ALUOut=ALU_A_mux()-ALU_B_mux();
		}else if(IR_FUNC==AND_CODE)
			novo_ALUOut=ALU_A_mux()&ALU_B_mux();
		else if(IR_FUNC==OR_CODE)
			novo_ALUOut=ALU_A_mux()|ALU_B_mux();
		else if(IR_FUNC==SLT_CODE){
			novo_ALUOut=ALU_A_mux()>ALU_B_mux();
		}
	}
	else if (ALUOp1 && ALUOp0){
		novo_ALUOut=ALU_A_mux()&ALU_B_mux();
	}
}

//finaliza o ciclo: calcula o proximo ciclo e escreve nos devidos registradores
void prox_ciclo(){
	int bit0,bit1,bit2,bit3,bit4,temp;
	PC=(PCWrite || (BNE_mux() && PCWriteCond))*novo_PC_mux() + (!(PCWrite || (BNE_mux() && PCWriteCond)))*PC;
	A=novo_A;
	B=novo_B;
	IR=(IRWrite)*novo_IR+(!IRWrite)*IR;
	ALUOut=novo_ALUOut;
	MDR=novo_MDR;
	//formula explicita de calculo de proximo estado:
	
	bit0=(!estado4) & (!estado3) &(
			((estado1) & (!estado0)) | ((!estado2) & (!estado1) &(
				      (!estado0)	|	((!IRBit(0)) & (!IRBit(2)) &(
						      ((!IRBit(1)) & (!IRBit(3)) & IRBit(4) & (!IRBit(5))) | IRBit(3) & (!IRBit(4)) & (IRBit(1) ^ IRBit(5)))
					      )
				      )
			)
		);
	bit1=(!estado4) & (!estado3) &(
			( (estado1)&(!estado0)&(
				estado2 | (IRBit(0) & (!IRBit(1)) & (!IRBit(2)) & (!IRBit(3)) & IRBit(4) & IRBit(5))
			) ) |( (!estado2) & (!estado1) & (estado0) & (
				(!IRBit(1)) & (
					(IRBit(0) & (!IRBit(3)) & IRBit(4) & IRBit(5) ) | (!IRBit(0)) & (
						( (!IRBit(3)) & (!IRBit(4)) & (!IRBit(5)) ) | ( IRBit(2) & (!IRBit(4)) & (!IRBit(5)) ) | ( (!IRBit(2)) & IRBit(5) & (IRBit(4)^IRBit(3)) )
					)
				) | ( 
					(!IRBit(0)) & IRBit(1) & (!IRBit(2)) & IRBit(3) & (!IRBit(4)) & (!IRBit(5))
				)
			) )
		);
	bit2=(!estado4) & (!estado3) & (
			(estado1 & ( estado2 ^ estado0 ) ) | ((!estado2) & (
				( (!estado1) & estado0 & (!IRBit(0)) & (!IRBit(4)) & (
					IRBit(5) & IRBit(3) & (!IRBit(2)) | (!IRBit(5)) & (!IRBit(1)) & (!(IRBit(2) ^ (IRBit(3))) )
				) ) | ( estado1 & (!estado0) & (!IRBit(1)) & IRBit(2) & (!IRBit(3)) & (
						( IRBit(0) & IRBit(4) & IRBit(5) ) | ( (!IRBit(0)) & (!IRBit(4)) & (!IRBit(5)))
					) )
			) )
		);
	bit3=(!estado4) & (!estado3) & (!estado2) & (
			(estado0 & (!estado1) & (!IRBit(0)) & (
					( (!IRBit(2)) & IRBit(3) & (!IRBit(4)) ) | (!IRBit(1)) & (
						(IRBit(3) & (!IRBit(4)) & (!IRBit(5))) | ( (!IRBit(2)) & (!IRBit(3)) & IRBit(4) )
					) ) | (!estado0) & (estado1) & (!IRBit(0)) & (!IRBit(1)) & IRBit(2) & (!IRBit(3)) & (!IRBit(4)) & (!IRBit(5))
				)
		);
	bit4=(!estado4) & estado3 & estado2 & estado1 & (!estado0);

	//Checagem se o opcode eh valido
	halt_reason = !((IRBit(0) & (!IRBit(1)) & (!IRBit(3)) & IRBit(4) & IRBit(5)) | ((!IRBit(0)) & (((!IRBit(2)) & IRBit(3) & (!IRBit(4))) | ((!IRBit(1)) & ((IRBit(2) & (!IRBit(4)) & (!IRBit(5))) | ((!IRBit(2)) & (!IRBit(3)))) )) ));

	//checagem se o opcode eh de uma instrucao tipo-R e o func eh valido
	halt_reason = halt_reason | (2*((IR_OPCODE==RType) & !(IRBit(26) & (!IRBit(27)) & (((!IRBit(29)) & (IRBit(30)) & (!IRBit(31))) | ((!IRBit(28)) & (!IRBit(29)) & (!IRBit(31))) | ((!IRBit(28)) & (IRBit(29)) & (!IRBit(30))) ))));

	//checagem se eh um estado que acessa a memoria e o endereco passado nao eh grande demais:
	//				    estados que acessam memoria:0,3,5			 memoria negativa,ou maior que 252
	halt_reason = halt_reason | (4*( ( (!bit4) && (!bit3) && (!bit1) && (!(bit2^bit0))) && ((resolve_mux_MAR()<=0) || (resolve_mux_MAR()>252) )));
																				//Contagem dos bits vai da direita para a esquerda
	UC  =	RegDst0_bit * ((!bit4) && (!bit3) && ( bit2) && bit1 && bit0 ) |										//0
		RegDst1_bit * ((!bit4) && ( bit3) && (!bit0) && (bit2 ^ bit1)) |										//1
		RegWrite_bit* ((bit4) || ( (!bit4) && ( ( (bit0)&&(bit2)&&(bit1^bit3)) || ((!bit0)&&(!bit1)&&(bit2)) || (bit3&&(!bit2)&&bit1&&(!bit0))))) |	//2
		ALUSrcA_bit * ((!bit4) && ( ( bit3 && bit2 && bit1 ) || ((!bit3) && bit1 && (!bit0) ) || (bit3 && (!bit2) && (!bit1) && (!bit0)) ) )|		//3
		ALUSrcB0_bit* ((!bit4) && (!bit3) && (!bit2) && (!bit1)) |											//4
		ALUSrcB1_bit* ((!bit4) && ( ( (!bit3) && (!bit2) && (bit0 ^ bit1) ) || (bit3 && bit2 && bit1 && (!bit0)) ) ) |					//5
		ALUOp0_bit * ( (!bit4) && (bit3) && ( (bit2 && bit1) || ( (!bit2) && (!bit1) && (!bit0)) ) ) |							//6
		ALUOp1_bit * ( (!bit4) && bit2 && bit1 && (!bit0)) |												//7
		PCSrc0_bit * ( (!bit4) && bit3 && (! (bit0 ^ bit1) ) ) |											//8
		PCSrc1_bit * ( (!bit4) && bit3 && ( ( (!bit2) && bit0) || ( (!bit0) && (bit2^bit1)))) |								//9
		PCWriteCond_bit * ( (!bit4) && (bit3) && ( (bit2 && bit1 && bit0) || ( (!bit2) && (!bit1) && (!bit0) ) ) ) |					//10
		PCWrite_bit * ( (!bit4) && ((bit3 && (!bit2)&&(bit0 || bit1))||((!bit0)&&(!bit1)&&(!(bit2^bit3)))) ) |						//11
		IorD_bit * ( (!bit4) && (!bit3) && bit0 && (bit1 ^ bit2) ) |											//12
		MemRead_bit * ( (!bit4) && (!bit3) && (!bit2) && (! (bit1 ^ bit0) ) ) |										//13
		MemWrite_bit * ( (!bit4) && (!bit3) && bit2 && (!bit1) && bit0) |										//14
		BNE_bit * ( (!bit4) && bit3 && bit2 && bit1 && bit0 ) |												//15
		IRWrite_bit * ( (!bit4) && (!bit3) && (!bit2) && (!bit1) && (!bit0) ) |										//16
		MemtoReg0_bit * ( (!bit4) && (!bit3) && bit2 && (!bit1) && (!bit0) ) |										//17
		MemtoReg1_bit * ( (!bit4) && bit3 && (!bit0) && (bit1 ^ bit2) ) |										//18
		halt_bit * (halt_reason!=0) |															//19
		(bit4 * estado4_bit) | (bit3 * estado3_bit) | (bit2 * estado2_bit) | (bit1 * estado1_bit) | (bit0 * estado0_bit);				//proximo estado da unidade de controle

}

void termina_prog(){
	int i;
	unsigned int tmp;
	printf("Status de sa\u00EDda: T\u00E9rmino devido \u00E0 ");
	if(halt_reason==1){
		printf("tentativa de execu\u00E7\u00E3o de instru\u00E7\u00E3o inv\u00E1lida.\n");
	}else if(halt_reason==2){
		printf("opera\u00E7\u00E3o inv\u00E1lida da ULA.\n");
	}else{
		printf("acesso inv\u00E1lido de mem\u00F3ria.\n");
	}
	printf("PC=%u\t\tIR=%u\t\tMDR=%u\n",PC,IR,MDR);//eh neccessario imprimir PC - 4 pois o valor de PC ja vai ter sido incrementado quando a finalizacao for chamada
	printf("A=%u\t\tB=%u\t\t\tAluOut=%u\n",A,B,ALUOut);
	printf("Controle=%d\n\n",UC);
	printf("Banco de Registradores\n");
	printf("R00(r0)=%d\t\tR08(t0)=%d\t\tR16(s0)=%d\t\tR24(t8)=%d\n",BR[0],BR[8] ,BR[16],BR[24]);
	printf("R01(at)=%d\t\tR09(t1)=%d\t\tR17(s1)=%d\t\tR25(t9)=%d\n",BR[1],BR[9] ,BR[17],BR[25]);
	printf("R02(v0)=%d\t\tR10(t2)=%d\t\tR18(s2)=%d\t\tR26(k0)=%d\n",BR[2],BR[10],BR[18],BR[26]);
	printf("R03(v1)=%d\t\tR11(t3)=%d\t\tR19(s3)=%d\t\tR27(k1)=%d\n",BR[3],BR[11],BR[19],BR[27]);
	printf("R04(a0)=%d\t\tR12(t4)=%d\t\tR20(s4)=%d\t\tR28(gp)=%d\n",BR[4],BR[12],BR[20],BR[28]);
	printf("R05(a1)=%d\t\tR13(t5)=%d\t\tR21(s5)=%d\t\tR29(sp)=%d\n",BR[5],BR[13],BR[21],BR[29]);
	printf("R06(a2)=%d\t\tR14(t6)=%d\t\tR22(s6)=%d\t\tR30(fp)=%d\n",BR[6],BR[14],BR[22],BR[30]);
	printf("R07(a3)=%d\t\tR15(t7)=%d\t\tR23(s7)=%d\t\tR31(ra)=%d\n",BR[7],BR[15],BR[23],BR[31]);
	printf("\nMemoria (enderecos a byte)\n");
	for(i=0;i<8;i++){
		tmp=mem[4*i];
		tmp=(tmp<<8)+mem[4*i+1];
		tmp=(tmp<<8)+mem[4*i+2];
		tmp=(tmp<<8)+mem[4*i+3];
		printf("[%02d]=%u\t\t",4*i,tmp);
		tmp=mem[4*i+32];
		tmp=(tmp<<8)+mem[4*i+33];
		tmp=(tmp<<8)+mem[4*i+34];
		tmp=(tmp<<8)+mem[4*i+35];
		printf("[%d]=%u\t\t",32+4*i,tmp);
		tmp=mem[4*i+64];
		tmp=(tmp<<8)+mem[4*i+65];
		tmp=(tmp<<8)+mem[4*i+66];
		tmp=(tmp<<8)+mem[4*i+67];
		printf("[%d]=%u\t\t",64+4*i,tmp);
		tmp=mem[4*i+96];
		tmp=(tmp<<8)+mem[4*i+97];
		tmp=(tmp<<8)+mem[4*i+98];
		tmp=(tmp<<8)+mem[4*i+99];
		printf("[%03d]=%u\n",96+4*i,tmp);
	}
}

int main(int argc,char** argv){
	FILE* f;
	int i;
	//SIMULA UM SINAL DE RESET EM TODOS OS REGISTRADORES
	prog_size=0;
	for (i=0;i<MEM_SIZE;i++) mem[i]=0;
	UC=MemRead_bit|IRWrite_bit|ALUSrcB0_bit|PCWrite_bit;
	IR=0;
	A=B=0;
	ALUOut=0;
	MDR=0;
	for(i=0;i<32;i++) BR[i]=0;
	PC=UCA=0;
	//INICIALIZACAO DO PROGRAMA
	if(argc<2){
		f=fopen("code.bin","r");
	}else{
		f=fopen(argv[1],"r");
	}
	if(f==NULL){
		printf("ERROR! Arquivo nao pode ser aberto\n");
	}
	carrega_prog(f);
	fclose(f);
	i=0;
	halt_reason=0;
	//execucao do programa
	while(!HALT){
		resolve_memoria(); //carrega o valor do IR
		resolve_BR(); //faz todas as operacoes relativas ao BR: escreve, le e carrega em A,B
		resolve_ULA(); //executa as operacoes da ULA
		prox_ciclo(); //calcula qual vai ser o proximmo ciclo e escreve nos registradores
		i++;
	}
	termina_prog();
	return 0;
}
