
/*
 * pipeline.c
 * 
 * Donald Yeung
 */


#include <stdlib.h>
#include "fu.h"
#include "pipeline.h"


int
commit(state_t *state) {
	int ROB_addr;
	int instr_c;
	const op_info_t *op_c;
	int imm_c;
	
	int store;
	int float_mem;
	
	int CQ_temp;
	
	int sth_committed=0;
	
	
	ROB_addr = state->ROB_head;
	//while(ROB_addr!=state->ROB_tail && sth_committed==0){
		instr_c = state->ROB[ROB_addr].instr;
		op_c = decode_instr(instr_c, &imm_c);
		if(state->ROB[ROB_addr].completed == 1){
			//sth_committed = 1;
			switch(op_c->fu_group_num){
				case FU_GROUP_INT:
					//write back to RF
					if(imm_c == 0){
						state->rf_int.reg_int.integer[FIELD_R3(instr_c)].w = state->ROB[ROB_addr].result.integer.w;
						//check tag
						if(ROB_addr == state->rf_int.tag[FIELD_R3(instr_c)])
							state->rf_int.tag[FIELD_R3(instr_c)] = -1;
					}
					else{
						state->rf_int.reg_int.integer[FIELD_R2(instr_c)].w = state->ROB[ROB_addr].result.integer.w;
						//check tag
						if(ROB_addr == state->rf_int.tag[FIELD_R2(instr_c)])
							state->rf_int.tag[FIELD_R3(instr_c)] = -1;
					}
					state->ROB_head = ((state->ROB_head)+1)%ROB_SIZE;
					sth_committed = 1;
				break;
				case FU_GROUP_ADD:
				case FU_GROUP_MULT:
				case FU_GROUP_DIV:
					state->rf_fp.reg_fp.flt[FIELD_R3(instr_c)] = state->ROB[ROB_addr].result.flt;
					if(ROB_addr == state->rf_fp.tag[FIELD_R3(instr_c)])
						state->rf_fp.tag[FIELD_R3(instr_c)] = -1;	
					state->ROB_head = ((state->ROB_head)+1)%ROB_SIZE;
					sth_committed = 1;
				break;
				case FU_GROUP_MEM:
					if(op_c->operation == OPERATION_LOAD){
						if(op_c->data_type == DATA_TYPE_W){
							state->rf_int.reg_int.integer[FIELD_R2(instr_c)].w = state->ROB[ROB_addr].result.integer.w;
							if(ROB_addr == state->rf_int.tag[FIELD_R2(instr_c)])
								state->rf_int.tag[FIELD_R2(instr_c)] = -1;
						}
						else{
							state->rf_fp.reg_fp.flt[FIELD_R2(instr_c)] = state->ROB[ROB_addr].result.flt;
							if(ROB_addr == state->rf_fp.tag[FIELD_R2(instr_c)])
								state->rf_fp.tag[FIELD_R2(instr_c)] = -1;
						}
						state->ROB_head = ((state->ROB_head)+1)%ROB_SIZE;
						sth_committed = 1;
					}
					else{
						if(op_c->data_type == DATA_TYPE_F)
							float_mem=1;
						else 
							float_mem=0;
						store=1;
							//issue_fu_mem(fu_mem_t *fu_list, int tag, int float_mem, int store)
						if(issue_fu_mem(state->fu_mem_list, ROB_addr, float_mem, store) == 0){	//issue STORE during commit stage
							CQ_temp=state->CQ_head;
							while(CQ_temp!=state->CQ_tail){	//CQ update
								if(state->CQ[CQ_temp].ROB_index == ROB_addr){
									state->CQ[CQ_temp].issued=1;
									if(CQ_temp == state->CQ_head)	//try
										state->CQ_head=((state->CQ_head)+1)%CQ_SIZE;
								}
								CQ_temp = (CQ_temp+1)%CQ_SIZE;
							}
							state->ROB_head = ((state->ROB_head)+1)%ROB_SIZE;	//update ROB if store is successfully issued
							sth_committed = 1;
							
							//execute
							if(op_c->data_type == DATA_TYPE_W){
								memcpy(&(state->mem[state->ROB[ROB_addr].target.integer.w]), &(state->ROB[ROB_addr].result.integer.w), 4);
								//if(ROB_addr == state->rf_int.tag[FIELD_R2(instr_c)])
								//	state->rf_int.tag[FIELD_R2(instr_c)] = -1;
							}
							else{
								memcpy(&(state->mem[state->ROB[ROB_addr].target.integer.w]), &(state->ROB[ROB_addr].result.flt), 4);
								//printf("Reg num = %d, ROB_addr = %d, tag = %d\n\n", FIELD_R2(instr_c), ROB_addr, state->rf_fp.tag[FIELD_R2(instr_c)]);
								//if(ROB_addr == state->rf_fp.tag[FIELD_R2(instr_c)])
								//	state->rf_fp.tag[FIELD_R2(instr_c)] = -1;
								//printf("Reg num = %d, tag = %d\n\n", FIELD_R2(instr_c), state->rf_fp.tag[FIELD_R2(instr_c)]);
							}
						}
					}
				break;
				case FU_GROUP_BRANCH:
					state->ROB_head = ((state->ROB_head)+1)%ROB_SIZE; 
					sth_committed = 1;
				break;
				case FU_GROUP_HALT:
					state->ROB_head = ((state->ROB_head)+1)%ROB_SIZE; 
					state->halt = 1;
					sth_committed = 1;
				break;
			}
		}
		//ROB_addr++;
	//}
	return sth_committed;
}


void
writeback(state_t *state) {
/*
int
wb_int(wb_port_int_t wb_port[], int wb_port_num, int tag) {
  int i;

  for(i = 0; i < wb_port_num; i++) {
    if(wb_port[i].tag == -1) {
      wb_port[i].tag = tag;
      return 0;
    }
  }
  return -1;   // structural hazard... stall
}
*/

	int i;
	int ROB_addr;
	const op_info_t *op_w, *op_temp;
	int imm_w, imm_temp;
	int instr_w, instr_temp;
	
	int CQ_temp=0;
	int IQ_temp=0;
	
	
	for(i = 0; i < state->wb_port_int_num; i++) {
	//1. set ROB complete
	
		if(state->wb_port_int[i].tag != -1) {
			ROB_addr = state->wb_port_int[i].tag;
			instr_w = state->ROB[ROB_addr%64].instr;
			op_w = decode_instr(instr_w, &imm_w);
			if(ROB_addr<64)	//exclude the ADDI portion of L/S
				state->ROB[ROB_addr].completed = 1;	//set completed = 1
			state->wb_port_int[i].tag=-1;	//update wb int tag
		}
		//2. broadcast to IQ CQ ?
		IQ_temp=state->IQ_head;
		while(IQ_temp != (state->IQ_tail)){
			if( state->IQ[IQ_temp].tag1 == ROB_addr && state->IQ[IQ_temp].instr !=0 ){
				state->IQ[IQ_temp].tag1 = -1;
				state->IQ[IQ_temp].operand1.integer.w = state->ROB[ROB_addr].result.integer.w;
			}
			if( state->IQ[IQ_temp].tag2 == ROB_addr && state->IQ[IQ_temp].instr !=0  ){
				state->IQ[IQ_temp].tag2 = -1;
				state->IQ[IQ_temp].operand2.integer.w = state->ROB[ROB_addr].result.integer.w;
			}
			IQ_temp=(IQ_temp+1)%IQ_SIZE;
		}
		
		CQ_temp=state->CQ_head;
		while(CQ_temp != (state->CQ_tail)){
			if( state->CQ[CQ_temp].tag1 == ROB_addr  && state->CQ[CQ_temp].instr != 0){
				state->CQ[CQ_temp].tag1 = -1;
				state->CQ[CQ_temp].address.integer.w = state->ROB[ROB_addr%64].target.integer.w;
			}
			if( state->CQ[CQ_temp].tag2 == ROB_addr  &&  state->CQ[CQ_temp].instr != 0 ){
				state->CQ[CQ_temp].tag2 = -1;
				state->CQ[CQ_temp].result.integer.w = state->ROB[ROB_addr%64].result.integer.w;
			}
			CQ_temp=(CQ_temp+1)%CQ_SIZE;
		}
				
			
	}
	
	
	
	for(i = 0; i < state->wb_port_fp_num; i++) {
		if(state->wb_port_fp[i].tag != -1) {
			ROB_addr = state->wb_port_fp[i].tag;
			instr_w = state->ROB[ROB_addr%64].instr;
			op_w = decode_instr(instr_w, &imm_w);
			if(ROB_addr<64)
				state->ROB[ROB_addr].completed = 1;	//set completed = 1
			state->wb_port_fp[i].tag=-1;	//update wb fp tag
		}
		
		IQ_temp=state->IQ_head;
		while(IQ_temp != state->IQ_tail){
			if( state->IQ[IQ_temp].tag1 == ROB_addr && state->IQ[IQ_temp].instr !=0 ){
				state->IQ[IQ_temp].tag1=-1;
				state->IQ[IQ_temp].operand1.flt = state->ROB[ROB_addr].result.flt;
			}
			if( state->IQ[IQ_temp].tag2 == ROB_addr && state->IQ[IQ_temp].instr !=0 ){
				state->IQ[IQ_temp].tag2=-1;
				state->IQ[IQ_temp].operand2.flt = state->ROB[ROB_addr].result.flt;
			}
			IQ_temp=(IQ_temp+1)%IQ_SIZE;
		}
		
		CQ_temp=state->CQ_head;
		while(CQ_temp != state->CQ_tail){
			if( state->CQ[CQ_temp].tag2 == ROB_addr && state->CQ[CQ_temp].instr != 0  ){
				state->CQ[CQ_temp].tag2 = -1;
				state->CQ[CQ_temp].result.flt = state->ROB[ROB_addr].result.flt;
			}
			CQ_temp=(CQ_temp+1)%CQ_SIZE;
		}
	}
	
	
	
	//3. branch
	if(state->branch_tag != -1){
		state->fetch_lock = FALSE; //stop stalling
		state->ROB[state->branch_tag].completed = 1;
		instr_temp = state->ROB[state->branch_tag].instr;
		op_temp = decode_instr(instr_temp, &imm_temp);
		switch(op_temp->operation){
			case OPERATION_J:
			case OPERATION_JR:
				state->pc = state->ROB[state->branch_tag].target.integer.w;
				state->if_id.instr = 0;	//NOP
			break;
			case OPERATION_JAL:
			case OPERATION_JALR:
				state->pc = state->ROB[state->branch_tag].target.integer.w;
				state->if_id.instr = 0;
			break;
			case OPERATION_BEQZ:
			case OPERATION_BNEZ:
				if(state->ROB[state->branch_tag].result.integer.w == 1){ //taken
					state->pc = state->ROB[state->branch_tag].target.integer.w;
					state->if_id.instr = 0;
				}
			break;
			default:
			break;
		}
		state->branch_tag=-1;	//branch_tag update
	}
	
}


void
execute(state_t *state) {
	//advance_fu_int(fu_int_t *fu_list, wb_port_int_t wb_port[], int wb_port_num, int *branch_tag)
	//advance_fu_mem(fu_mem_t *fu_list, wb_port_int_t wb_int_port[], int wb_int_port_num, wb_port_fp_t wb_fp_port[], int wb_fp_port_num)
	//advance_fu_fp(fu_fp_t *fu_list,wb_port_fp_t wb_port[],int wb_port_num)
	advance_fu_int(state->fu_int_list, state->wb_port_int, state->wb_port_int_num, &state->branch_tag);
	advance_fu_mem(state->fu_mem_list, state->wb_port_int, state->wb_port_int_num, state->wb_port_fp, state->wb_port_fp_num);
	advance_fu_fp(state->fu_add_list, state->wb_port_fp, state->wb_port_fp_num);
	advance_fu_fp(state->fu_mult_list, state->wb_port_fp, state->wb_port_fp_num);
	advance_fu_fp(state->fu_div_list, state->wb_port_fp, state->wb_port_fp_num);
	
}


int
memory_disambiguation(state_t *state) {
	CQ_t CQ_m;
	int CQ_addr;
	int CQ_temp=0;
	int ROB_temp=0;
	int instr_m, instr_t;
	const op_info_t *op_m, *op_t;
	int imm_m=0, imm_t=0;
	int sth_issued=0;
	int float_mem=0;
	int store=0;
	int conflict=0;
	
	int stop=0;
	
	//execution variables
	operand_t address, result;
	
	CQ_addr = state->CQ_head;
	CQ_m = state->CQ[CQ_addr];
	instr_m = CQ_m.instr;
	op_m=decode_instr(instr_m, &imm_m);
	
	while( sth_issued==0 && CQ_addr != (state->CQ_tail) ){ 
		if(CQ_m.tag1==-1 && CQ_m.tag2==-1 && CQ_m.issued==0){
			
			if(op_m->operation == OPERATION_STORE){		//store
			/*	if(op_m->data_type == DATA_TYPE_F)
					float_mem=1;
				else 
					float_mem=0;
				store=1;
			//issue_fu_mem(fu_mem_t *fu_list, int tag, int float_mem, int store)
				//if(issue_fu_mem(state->fu_mem_list, CQ_m.ROB_index, float_mem, store) == 0){
					//CQ_m.issued=1;
					//state->CQ_head=((state->CQ_head)+1)%CQ_SIZE;
					//sth_issued=1;
			*/
					state->CQ[CQ_addr].issued=1;
					//execution
					if(op_m->data_type == DATA_TYPE_F)
						state->ROB[CQ_m.ROB_index].result.flt = CQ_m.result.flt;
					else
						state->ROB[CQ_m.ROB_index].result.integer.w = CQ_m.result.integer.w;	//result
					state->ROB[CQ_m.ROB_index].target.integer.w = CQ_m.address.integer.w;	//address
					state->ROB[CQ_m.ROB_index].completed = TRUE; //STORE complete for forwarding
				//}
			}
			else{		//load
				//check for conflict in CQ
				CQ_temp=state->CQ_head;
				conflict=0;
				if(CQ_addr==0){
					if(op_m->data_type == DATA_TYPE_W)
							memcpy(&(state->ROB[CQ_m.ROB_index].result.integer.w), &(state->mem[state->ROB[CQ_m.ROB_index].target.integer.w]), 4);
					if(op_m->data_type == DATA_TYPE_F)
							memcpy(&(state->ROB[CQ_m.ROB_index].result.flt), &(state->mem[state->ROB[CQ_m.ROB_index].target.integer.w]), 4);
				}
				
				while( (CQ_temp != CQ_addr ) && conflict ==0 ){
					instr_t=state->CQ[CQ_temp].instr;
					op_t=decode_instr(instr_t, &imm_t);
					if(state->CQ[CQ_temp].address.integer.w == state->CQ[CQ_addr].address.integer.w && op_t->operation == OPERATION_STORE && state->CQ[CQ_temp].issued == 0)	//LAS
						conflict=1;
					CQ_temp=(CQ_temp+1)%CQ_SIZE;
				}
					
				if(conflict==0){	
					if(op_m->data_type == DATA_TYPE_F)
						float_mem=1;
					else 
						float_mem=0;
					store=0;
					if(issue_fu_mem(state->fu_mem_list, CQ_m.ROB_index, float_mem, store)==0){
						state->CQ[CQ_addr].issued=1;
						if(CQ_addr == state->CQ_head)
							state->CQ_head=((state->CQ_head)+1)%CQ_SIZE;
						sth_issued=1;
						//execution
						/*
						if(op_m->data_type == DATA_TYPE_F)
							state->ROB[CQ_m.ROB_index].result.flt = CQ_m.result.flt;
						else
							state->ROB[CQ_m.ROB_index].result.integer.w = CQ_m.result.integer.w;	//result
						state->ROB[CQ_m.ROB_index].targer.integer.w = CQ_m.address.integer.w;	//address
						*/
						//check ROB for LAS
						ROB_temp=state->ROB_head;
						
							if(op_m->data_type == DATA_TYPE_W)
									memcpy(&(state->ROB[CQ_m.ROB_index].result.integer.w), &(state->mem[state->ROB[CQ_m.ROB_index].target.integer.w]), 4);
							if(op_m->data_type == DATA_TYPE_F)
									memcpy(&(state->ROB[CQ_m.ROB_index].result.flt), &(state->mem[state->ROB[CQ_m.ROB_index].target.integer.w]), 4);
						
						
						while(ROB_temp!=CQ_m.ROB_index){	//check earlier instructions in ROB
							instr_t=state->ROB[ROB_temp].instr;
							op_t=decode_instr(instr_t, &imm_t);
							if(state->ROB[ROB_temp].target.integer.w == state->ROB[CQ_m.ROB_index].target.integer.w && op_t->operation == OPERATION_STORE && state->ROB[ROB_temp].completed==1 && op_m->data_type == op_t->data_type){
								if(op_m->data_type == op_t->data_type == DATA_TYPE_W)
									state->ROB[CQ_m.ROB_index].result.integer.w = state->ROB[ROB_temp].result.integer.w;
								if(op_m->data_type == op_t->data_type == DATA_TYPE_F)
									state->ROB[CQ_m.ROB_index].result.flt = state->ROB[ROB_temp].result.flt;
							}
							ROB_temp = (ROB_temp+1)%ROB_SIZE;
						}
					}
				}
				
			}
				
		}
		CQ_addr = (CQ_addr+1)%CQ_SIZE;
		CQ_m = state->CQ[CQ_addr];
		instr_m = CQ_m.instr;
		op_m=decode_instr(instr_m, &imm_m);
	}
	
	CQ_addr = state->CQ_head;
	while(CQ_addr != (state->CQ_tail) && stop ==0 ){
		if(state->CQ[CQ_addr].issued == 1)
			CQ_addr = (CQ_addr+1)%CQ_SIZE;
		else
			stop=1;
	}
	state->CQ_head = CQ_addr;

}


int
issue(state_t *state) {
	IQ_t IQ_i;
	int IQ_addr;
	int instr_i;
	const op_info_t *op_i;
	int imm_i=0;
	int branch=0;
	int link=0;
	int float_mem=0;
	int store=0;
	int sth_issued=0;
	int stop=0;
	
	//execution variables
	operand_t operand1, operand2;
	
	IQ_addr = state->IQ_head;
	IQ_i = state->IQ[IQ_addr];
	instr_i = IQ_i.instr;
	op_i=decode_instr(instr_i, &imm_i);
	
	while( sth_issued==0 && IQ_addr != (state->IQ_tail) ){ 
		//printf("TAG1=%d, tag2=%d\n\n\n\n", IQ_i.tag1, IQ_i.tag2);
		if(IQ_i.tag1==-1 && IQ_i.tag2==-1 && IQ_i.issued==0){
			switch(op_i->fu_group_num){
				case FU_GROUP_INT:
					branch=0;
					link=0;
				//issue_fu_int(fu_int_t *fu_list, int tag, int branch, int link)
					if( issue_fu_int(state->fu_int_list, IQ_i.ROB_index, branch, link) == 0 ){	//if issued
						//IQ_i.issued=1;
						state->IQ[IQ_addr].issued=1;
						if(IQ_addr==state->IQ_head)	//if the first instruction of IQ is issued, update head
							state->IQ_head=((state->IQ_head)+1)%IQ_SIZE;
						sth_issued=1;
						
						//execution
						operand1=IQ_i.operand1;
						operand2=IQ_i.operand2;
						
						switch(op_i->operation){
							case OPERATION_ADD:
								state->ROB[IQ_i.ROB_index].result.integer.w = operand1.integer.w + operand2.integer.w;   
								break;
							case OPERATION_ADDU:
								state->ROB[IQ_i.ROB_index].result.integer.wu = operand1.integer.wu + operand2.integer.wu; //unsigned
								break;
							case OPERATION_SUB:
								state->ROB[IQ_i.ROB_index].result.integer.w = operand1.integer.w - operand2.integer.w;
								break;
							case OPERATION_SUBU:
								state->ROB[IQ_i.ROB_index].result.integer.wu = operand1.integer.wu - operand2.integer.wu; //unsigned
								break;
							case OPERATION_MULT:
								state->ROB[IQ_i.ROB_index].result.integer.w = operand1.integer.w * operand2.integer.w;
								break;
							case OPERATION_DIV:
								state->ROB[IQ_i.ROB_index].result.integer.w = operand1.integer.w / operand2.integer.w;
								break;
							case OPERATION_SLL:
								state->ROB[IQ_i.ROB_index].result.integer.w = operand1.integer.w << operand2.integer.w;
								break;
							case OPERATION_SRL:
								state->ROB[IQ_i.ROB_index].result.integer.w = operand1.integer.w >> operand2.integer.w;
								break;
							case OPERATION_AND:
								state->ROB[IQ_i.ROB_index].result.integer.w = operand1.integer.w & operand2.integer.w;
								break;
							case OPERATION_OR:
								state->ROB[IQ_i.ROB_index].result.integer.w = operand1.integer.w | operand2.integer.w;
								break;
							case OPERATION_XOR:
								state->ROB[IQ_i.ROB_index].result.integer.w = operand1.integer.w ^ operand2.integer.w;
								break;
							case OPERATION_SLT:
								state->ROB[IQ_i.ROB_index].result.integer.w = operand1.integer.w < operand2.integer.w;
								break;
							case OPERATION_SGT:
								state->ROB[IQ_i.ROB_index].result.integer.w = operand1.integer.w > operand2.integer.w;
								break;
							case OPERATION_SLTU:
								state->ROB[IQ_i.ROB_index].result.integer.wu = operand1.integer.wu < operand2.integer.wu; //unsigned
								break;
							case OPERATION_SGTU:
								state->ROB[IQ_i.ROB_index].result.integer.wu = operand1.integer.wu > operand2.integer.wu; //unsigned
								break;
							default:
								break;
						}
						
					}
				break;
				case FU_GROUP_MEM:	//calculate effective address
				//issue_fu_int(fu_int_t *fu_list, int tag, int branch, int link) 
					branch=0;
					link=0;
					if( issue_fu_int(state->fu_int_list, IQ_i.ROB_index+64, branch, link)  == 0){  
					//printf("issued\n\n\n");
						state->IQ[IQ_addr].issued=1;
					//printf("addr=%d, issued=%d\n\n\n", IQ_addr, IQ_i.issued);
					//printf("addr=%d, issued=%d\n\n\n", IQ_addr, state->IQ[IQ_addr].issued);
						if(IQ_addr==state->IQ_head)
							state->IQ_head=((state->IQ_head)+1)%IQ_SIZE;
						sth_issued=1;
						//execution
						operand1=IQ_i.operand1;
						operand2=IQ_i.operand2;
						state->ROB[IQ_i.ROB_index].target.integer.w = operand1.integer.w + operand2.integer.w;
					}
				break;
				case FU_GROUP_ADD:
				//issue_fu_fp(fu_fp_t *fu_list, int tag) 
					if( issue_fu_fp(state->fu_add_list, IQ_i.ROB_index) == 0){
						//IQ_i.issued=1;
						state->IQ[IQ_addr].issued=1;
						if(IQ_addr==state->IQ_head)
							state->IQ_head=((state->IQ_head)+1)%IQ_SIZE;
						sth_issued=1;
						//execution
						operand1=IQ_i.operand1;
						operand2=IQ_i.operand2;
						if(op_i->operation == OPERATION_ADD)
							state->ROB[IQ_i.ROB_index].result.flt = operand1.flt + operand2.flt;		
						if(op_i->operation == OPERATION_SUB)
							state->ROB[IQ_i.ROB_index].result.flt = operand1.flt - operand2.flt;
					}
				break;
				case FU_GROUP_MULT:
					if( issue_fu_fp(state->fu_mult_list, IQ_i.ROB_index) == 0){
						//IQ_i.issued=1;
						state->IQ[IQ_addr].issued=1;
						if(IQ_addr==state->IQ_head)
							state->IQ_head=((state->IQ_head)+1)%IQ_SIZE;
						sth_issued=1;
						//execution
						operand1=IQ_i.operand1;
						operand2=IQ_i.operand2;
						state->ROB[IQ_i.ROB_index].result.flt = operand1.flt * operand2.flt;
					}
				break;
				case FU_GROUP_DIV:
					if( issue_fu_fp(state->fu_div_list, IQ_i.ROB_index) == 0){
						//IQ_i.issued=1;
						state->IQ[IQ_addr].issued=1;
						if(IQ_addr==state->IQ_head)
							state->IQ_head=((state->IQ_head)+1)%IQ_SIZE;
						sth_issued=1;
						//execution
						operand1=IQ_i.operand1;
						operand2=IQ_i.operand2;
						state->ROB[IQ_i.ROB_index].result.flt = operand1.flt / operand2.flt;
					}
				break;
				case FU_GROUP_BRANCH:
					branch=1;
					if(op_i->operation==OPERATION_JAL || op_i->operation==OPERATION_JALR)
						link=1;
					else
						link=0;
					if( issue_fu_int(state->fu_int_list, IQ_i.ROB_index, branch, link) ==0 ){
						//IQ_i.issued=1;
						state->IQ[IQ_addr].issued=1;
						if(IQ_addr==state->IQ_head)
							state->IQ_head=((state->IQ_head)+1)%IQ_SIZE;
						sth_issued=1;
						
						//execution
						switch(op_i->operation){		
							case OPERATION_J:
								state->ROB[IQ_i.ROB_index].target.integer.w = IQ_i.pc + FIELD_OFFSET(instr_i) +4;	//pc = pc + off + 4?
								break;
							case OPERATION_JAL:
								state->ROB[IQ_i.ROB_index].result.integer.w = IQ_i.pc;		//reg[31] = pc, write back PC? Perhaps wait a few cycles
								state->ROB[IQ_i.ROB_index].target.integer.w = IQ_i.pc + FIELD_OFFSET(instr_i) +4;  	//pc = pc + off + 4
								break;
							case OPERATION_JR:
								state->ROB[IQ_i.ROB_index].target.integer.w = IQ_i.operand1.integer.w ;	//pc = reg[r1]
								break;
							case OPERATION_JALR:
								state->ROB[IQ_i.ROB_index].result.integer.w = IQ_i.pc;					//reg[31] = pc, write back PC?
								state->ROB[IQ_i.ROB_index].target.integer.w = IQ_i.operand1.integer.w ;   //pc = reg[r1]
								break;
							case OPERATION_BEQZ:
								state->ROB[IQ_i.ROB_index].target.integer.w = IQ_i.pc + FIELD_IMM(instr_i) +4;
								if( IQ_i.operand1.integer.w == 0  ){
									state->ROB[IQ_i.ROB_index].result.integer.w = 1;	//taken
								}
								else{
									state->ROB[IQ_i.ROB_index].result.integer.w = 0;	//not taken
								}
								break;
							case OPERATION_BNEZ:
								state->ROB[IQ_i.ROB_index].target.integer.w = IQ_i.pc + FIELD_IMM(instr_i) +4;
								if( IQ_i.operand1.integer.w == 0 != 0 ){
									state->ROB[IQ_i.ROB_index].result.integer.w = 1;	//taken
								}
								else{
									state->ROB[IQ_i.ROB_index].result.integer.w = 0;	//not taken
								}
								break;
						}
					}
				break;
				default:
				break;
						
			}
		
		}
		//printf("addr=%d, issued=%d\n\n\n", IQ_addr, state->IQ[IQ_addr].issued);
		//next IQ entry
		IQ_addr = (IQ_addr+1)%IQ_SIZE;
		IQ_i = state->IQ[IQ_addr];
		instr_i = IQ_i.instr;
		op_i=decode_instr(instr_i, &imm_i);
	} 	
		
	//IQ_head update
	
	IQ_addr = state->IQ_head;
	while( IQ_addr != ((state->IQ_tail)) && stop==0 ){ 
		if(state->IQ[IQ_addr].issued==1)
			IQ_addr = (IQ_addr+1)%IQ_SIZE;
		else
			stop=1;
	}
	state->IQ_head = IQ_addr;
		//printf("%d, %d, %d\n\n\n\n", state->IQ_head==(state->IQ_tail+1)%IQ_SIZE, state->CQ_head==(state->CQ_tail+1)%CQ_SIZE, state->ROB_head==(state->ROB_tail+1)%ROB_SIZE);
	if(state->fetch_lock==0 && ( state->IQ_head==(state->IQ_tail+1)%IQ_SIZE || state->CQ_head==(state->CQ_tail+1)%CQ_SIZE || state->ROB_head==(state->ROB_tail+1)%ROB_SIZE)){
		state->fetch_lock=1;
		state->full=1;
	}
		
		//printf("%d, %d, %d\n\n\n\n", state->IQ_head!=(state->IQ_tail+1)%IQ_SIZE, state->CQ_head!=(state->CQ_tail+1)%CQ_SIZE, state->ROB[state->ROB_head].completed);
	if(state->full==1 && state->IQ_head!=(state->IQ_tail+1)%IQ_SIZE && state->CQ_head!=(state->CQ_tail+1)%CQ_SIZE && state->ROB[state->ROB_head].completed){
		state->fetch_lock=0;
		state->full=0;
	}
}


int
dispatch(state_t *state) {
	const op_info_t *op_d; 
	int imm_d;
	int instr_d;

	instr_d = (state->if_id).instr;
	op_d = decode_instr(instr_d, &imm_d); 
	
	//ROB update
	state->ROB[state->ROB_tail].instr = instr_d;
	state->ROB[state->ROB_tail].completed=0;
	if(op_d->fu_group_num == FU_GROUP_NONE)
		return 0;
	if(op_d->fu_group_num == FU_GROUP_HALT){
		state->ROB[state->ROB_tail].completed = 1;
		state->fetch_lock = 1;
	}
	else{
	//IQ Update
	state->IQ[state->IQ_tail].instr = instr_d;
	state->IQ[state->IQ_tail].pc = state->if_id.pc;
	state->IQ[state->IQ_tail].ROB_index = state->ROB_tail;
	state->IQ[state->IQ_tail].issued=0;

		switch(op_d->fu_group_num){
			case FU_GROUP_MEM:		//check source R1, save imm_d to operand2
				if(state->rf_int.tag[ FIELD_R1(instr_d) ] == -1){	//if data is present
					state->IQ[state->IQ_tail].tag1 = -1;
					state->IQ[state->IQ_tail].operand1.integer.w = state->rf_int.reg_int.integer[ FIELD_R1(instr_d) ].w;
				}
				else{
					if(state->ROB[ state->rf_int.tag[FIELD_R1(instr_d)] ].completed){	//if data is not present but in ROB and completed
						state->IQ[state->IQ_tail].tag1 = -1;
						state->IQ[state->IQ_tail].operand1.integer.w = state->ROB[ state->rf_int.tag[FIELD_R1(instr_d)] ].result.integer.w;
					}
					else{	//data is not ready, set to ROB index of the in-flight instruction 
						state->IQ[state->IQ_tail].tag1 = state->rf_int.tag[FIELD_R1(instr_d)];
					}	
				}
				//save immediate value to operand2
				state->IQ[state->IQ_tail].tag2 = -1;
				state->IQ[state->IQ_tail].operand2.integer.w = FIELD_IMM(instr_d);
				//rename the dest reg
				if(op_d->operation != OPERATION_STORE)
					if(op_d->data_type == DATA_TYPE_W)
						state->rf_int.tag[ FIELD_R2(instr_d) ] = state->ROB_tail; 
					else
						state->rf_fp.tag[ FIELD_R2(instr_d) ] = state->ROB_tail; 
			break;
			
			case FU_GROUP_INT:
				if(imm_d == 1){		//use imm value
					//R1
					if(state->rf_int.tag[ FIELD_R1(instr_d) ] == -1){	//if data is present
						state->IQ[state->IQ_tail].tag1 = -1;
						state->IQ[state->IQ_tail].operand1.integer.w = state->rf_int.reg_int.integer[ FIELD_R1(instr_d) ].w;
					}
					else{
						if(state->ROB[ state->rf_int.tag[FIELD_R1(instr_d)] ].completed){	//if data is not present but in ROB and completed
							state->IQ[state->IQ_tail].tag1 = -1;
							state->IQ[state->IQ_tail].operand1.integer.w = state->ROB[ state->rf_int.tag[FIELD_R1(instr_d)] ].result.integer.w;
						}
						else{	//data is not ready, set to ROB index of the in-flight instruction 
							state->IQ[state->IQ_tail].tag1 = state->rf_int.tag[FIELD_R1(instr_d)];
						}	
					}
					state->IQ[state->IQ_tail].tag2 = -1;
					state->IQ[state->IQ_tail].operand2.integer.w = FIELD_IMM(instr_d);
					//rename the dest reg
					state->rf_int.tag[ FIELD_R2(instr_d) ] = state->ROB_tail;
				}
				else{		//don't use imm value, two source reg R1 R2
					//R1
					if(state->rf_int.tag[ FIELD_R1(instr_d) ] == -1){	//if data is present
						state->IQ[state->IQ_tail].tag1 = -1;
						state->IQ[state->IQ_tail].operand1.integer.w = state->rf_int.reg_int.integer[ FIELD_R1(instr_d) ].w;
					}
					else{
						if(state->ROB[ state->rf_int.tag[FIELD_R1(instr_d)] ].completed){	//if data is not present but in ROB and completed
							state->IQ[state->IQ_tail].tag1 = -1;
							state->IQ[state->IQ_tail].operand1.integer.w = state->ROB[ state->rf_int.tag[FIELD_R1(instr_d)] ].result.integer.w;
						}
						else{	//data is not ready, set to ROB index of the in-flight instruction 
							state->IQ[state->IQ_tail].tag1 = state->rf_int.tag[FIELD_R1(instr_d)];
						}	
					}
					//R2
					if(state->rf_int.tag[ FIELD_R2(instr_d) ] == -1){	//if data is present
						state->IQ[state->IQ_tail].tag2 = -1;
						state->IQ[state->IQ_tail].operand2.integer.w = state->rf_int.reg_int.integer[ FIELD_R2(instr_d) ].w;
					}
					else{
						if(state->ROB[ state->rf_int.tag[FIELD_R2(instr_d)] ].completed){	//if data is not present but in ROB and completed
							state->IQ[state->IQ_tail].tag2 = -1;
							state->IQ[state->IQ_tail].operand2.integer.w = state->ROB[ state->rf_int.tag[FIELD_R2(instr_d)] ].result.integer.w;
						}
						else{	//data is not ready, set to ROB index of the in-flight instruction 
							state->IQ[state->IQ_tail].tag2 = state->rf_int.tag[FIELD_R2(instr_d)];
						}	
					}
					//rename the dest reg
					state->rf_int.tag[ FIELD_R3(instr_d) ] = state->ROB_tail;
				}
			break;
			
			case FU_GROUP_ADD:
			case FU_GROUP_MULT:
			case FU_GROUP_DIV:
				//don't use imm value, two source reg R1 R2
					//R1
					if(state->rf_fp.tag[ FIELD_R1(instr_d) ] == -1){	//if data is present
						state->IQ[state->IQ_tail].tag1 = -1;
						state->IQ[state->IQ_tail].operand1.flt = state->rf_fp.reg_fp.flt[ FIELD_R1(instr_d) ];
					}
					else{
						if(state->ROB[ state->rf_fp.tag[FIELD_R1(instr_d)] ].completed){	//if data is not present but in ROB and completed
							state->IQ[state->IQ_tail].tag1 = -1;
							state->IQ[state->IQ_tail].operand1.flt = state->ROB[ state->rf_fp.tag[FIELD_R1(instr_d)] ].result.flt;
						}
						else{	//data is not ready, set to ROB index of the in-flight instruction 
							state->IQ[state->IQ_tail].tag1 = state->rf_fp.tag[FIELD_R1(instr_d)];
						}	
					}
					//R2
					if(state->rf_fp.tag[ FIELD_R2(instr_d) ] == -1){	//if data is present
						state->IQ[state->IQ_tail].tag2 = -1;
						state->IQ[state->IQ_tail].operand2.flt = state->rf_fp.reg_fp.flt[ FIELD_R2(instr_d) ];
					}
					else{
						if(state->ROB[ state->rf_fp.tag[FIELD_R2(instr_d)] ].completed){	//if data is not present but in ROB and completed
							state->IQ[state->IQ_tail].tag2 = -1;
							state->IQ[state->IQ_tail].operand2.flt = state->ROB[ state->rf_fp.tag[FIELD_R2(instr_d)] ].result.flt;
						}
						else{	//data is not ready, set to ROB index of the in-flight instruction 
							state->IQ[state->IQ_tail].tag2 = state->rf_fp.tag[FIELD_R2(instr_d)];
						}	
					}
					//rename the dest reg
					state->rf_fp.tag[ FIELD_R3(instr_d) ] = state->ROB_tail;
			break;
			
			
			case FU_GROUP_BRANCH:
				if(op_d->operation == OPERATION_JR || op_d->operation == OPERATION_BEQZ || op_d->operation == OPERATION_BNEZ){
					//R1
					if(state->rf_int.tag[ FIELD_R1(instr_d) ] == -1){	//if data is present
						state->IQ[state->IQ_tail].tag1 = -1;
						state->IQ[state->IQ_tail].operand1.integer.w = state->rf_int.reg_int.integer[ FIELD_R1(instr_d) ].w;
					}
					else{
						if(state->ROB[ state->rf_int.tag[FIELD_R1(instr_d)] ].completed){	//if data is not present but in ROB and completed
							state->IQ[state->IQ_tail].tag1 = -1;
							state->IQ[state->IQ_tail].operand1.integer.w = state->ROB[ state->rf_int.tag[FIELD_R1(instr_d)] ].result.integer.w;
						}
						else{	//data is not ready, set to ROB index of the in-flight instruction 
							state->IQ[state->IQ_tail].tag1 = state->rf_int.tag[FIELD_R1(instr_d)];
						}	
					}
					state->IQ[state->IQ_tail].tag2 = -1;
					//don't rename the dest reg
				}
				else{
					state->IQ[state->IQ_tail].tag1 = -1;
					state->IQ[state->IQ_tail].tag2 = -1;
				}
				
				if(op_d->operation == OPERATION_JAL || op_d->operation == OPERATION_JALR){
					//rename the dest reg
					state->rf_int.tag[ 31 ] = state->ROB_tail;
				}
				state->fetch_lock = TRUE; //when to set false?
			break;
			
			//case FU_GROUP_HALT:
			//	state->fetch_lock = TRUE;
			//break;
			
			default:
			break;
	
		}
			//if( (state->IQ_tail) == state->IQ_head)
				//state->fetch_lock = 1;
			state->IQ_tail = ((state->IQ_tail)+1)%IQ_SIZE;
			
		
		//CQ Update
		if(op_d->fu_group_num == FU_GROUP_MEM){
			state->CQ[state->CQ_tail].instr = instr_d;
			state->CQ[state->CQ_tail].ROB_index = state->ROB_tail;
			state->CQ[state->CQ_tail].issued = 0;
			if(op_d->operation == OPERATION_LOAD){	//Load, address tag not ready, don't need result tag
				state->CQ[state->CQ_tail].store = FALSE;
				state->CQ[state->CQ_tail].tag2 = -1;
				state->CQ[state->CQ_tail].tag1 = state->ROB_tail+64;
			}
			else{		//Store, address tag not ready, need result tag
				state->CQ[state->CQ_tail].store = TRUE;
				state->CQ[state->CQ_tail].tag1 = state->ROB_tail+64;
				if(state->rf_int.tag[ FIELD_R2(instr_d) ] == -1 && op_d->data_type == DATA_TYPE_W ){	//result tag is ready, int
					state->CQ[state->CQ_tail].tag2 = -1;
					state->CQ[state->CQ_tail].result.integer.w = state->rf_int.reg_int.integer[ FIELD_R2(instr_d) ].w;
				}
				else if(state->rf_fp.tag[ FIELD_R2(instr_d) ] == -1 && op_d->data_type == DATA_TYPE_F){	//result tag is ready, flt
					state->CQ[state->CQ_tail].tag2 = -1;
					state->CQ[state->CQ_tail].result.flt = state->rf_fp.reg_fp.flt[ FIELD_R2(instr_d) ];
				}
				else{	//result tag is not ready
					if(state->ROB[ state->rf_int.tag[FIELD_R2(instr_d)] ].completed && op_d->data_type == DATA_TYPE_W){	//if data is not present but in ROB and completed
						state->CQ[state->CQ_tail].tag2 = -1;
						state->CQ[state->CQ_tail].result.integer.w = state->ROB[ state->rf_int.tag[FIELD_R2(instr_d)] ].result.integer.w;
					}
					else if(state->ROB[ state->rf_int.tag[FIELD_R2(instr_d)] ].completed==0 && op_d->data_type == DATA_TYPE_W)	//data is not ready, set to ROB index of the in-flight instruction 
						state->CQ[state->CQ_tail].tag2 = state->rf_int.tag[FIELD_R2(instr_d)];
					else if (state->ROB[ state->rf_fp.tag[FIELD_R2(instr_d)] ].completed && op_d->data_type == DATA_TYPE_F){
						state->CQ[state->CQ_tail].tag2 = -1;
						state->CQ[state->CQ_tail].result.flt = state->ROB[ state->rf_fp.tag[FIELD_R2(instr_d)] ].result.flt;
					}
					else if(state->ROB[ state->rf_fp.tag[FIELD_R2(instr_d)] ].completed==0 && op_d->data_type == DATA_TYPE_F)
						state->CQ[state->CQ_tail].tag2 = state->rf_fp.tag[FIELD_R2(instr_d)];			
				}
			}
				//if( (state->CQ_tail) == state->CQ_head)
					//state->fetch_lock=1;
				state->CQ_tail = ((state->CQ_tail)+1)%CQ_SIZE;
				//if(state->CQ_tail == state->CQ_head)
					//state->fetch_lock=1;
		}
	
	}
		//if( (state->ROB_tail) == state->ROB_head)
			//state->fetch_lock =1;
		state->ROB_tail = ((state->ROB_tail)+1)%ROB_SIZE;
		//if(state->ROB_tail == state->ROB_head)
			//state->fetch_lock =1;
	

}


void
fetch(state_t *state) {
	int i;
	state->if_id.pc=state->pc;
	memcpy( &((state->if_id).instr), &(state->mem[state->pc]), 4);
	state->pc += 4;
}
