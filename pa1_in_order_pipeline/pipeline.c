
/*
 * 
 * pipeline.c
 * 
 * Donald Yeung
 */


#include <stdlib.h>
#include "fu.h"
#include "pipeline.h"


void  //change to int type
writeback(state_t *state, int *num_insn) {

/******************Perform Operation********************/
	unsigned int operand1_add, operand2_add, result_add; //address
	operand_t operand1, operand2, result;  //value
	int instr_int, instr_fp;	//instruction
	int datatype;	//datatype
	//char *memory;	//memory start address
	
	instr_int = (state->int_wb).instr; //for int wb
	instr_fp = (state->fp_wb).instr;  //for fp wb
	
	const op_info_t *op_int, *op_fp; 
	int imm_int, imm_fp;
	
	//memory = &(state->mem[0]);
	
	op_int = decode_instr(instr_int, &imm_int); //for int decode
	op_fp = decode_instr(instr_fp, &imm_fp); //for fp decode
	

	//int writeback
	switch(op_int->fu_group_num){
	case FU_GROUP_INT:
		*num_insn=*num_insn+1;	//increase number of instruction 
		if(imm_int!=1){
			operand1_add = FIELD_R1(instr_int);  //rs1 address
			operand2_add = FIELD_R2(instr_int);  //rs2 address 
			result_add = FIELD_R3(instr_int);    //rd  address
			operand1.integer.w = (state->rf_int).reg_int[operand1_add].w;   //rs1 value
			operand2.integer.w = (state->rf_int).reg_int[operand2_add].w;  //
		}
		else{
			operand1_add = FIELD_R1(instr_int);
			result_add = FIELD_R2(instr_int); 
			operand1.integer.w = (state->rf_int).reg_int[operand1_add].w;
			operand2.integer.w = FIELD_IMM(instr_int);
		}
		
		
		switch(op_int->operation){
		case OPERATION_ADD:
			result.integer.w = operand1.integer.w + operand2.integer.w;   //operation
			(state->rf_int).reg_int[result_add].w = result.integer.w;     //write back to register file
			break;
		case OPERATION_ADDU:
			result.integer.wu = operand1.integer.wu + operand2.integer.wu; //unsigned
			(state->rf_int).reg_int[result_add].wu = result.integer.wu;
			break;
		case OPERATION_SUB:
			result.integer.w = operand1.integer.w - operand2.integer.w;
			(state->rf_int).reg_int[result_add].w = result.integer.w;
			break;
		case OPERATION_SUBU:
			result.integer.wu = operand1.integer.wu - operand2.integer.wu; //unsigned
			(state->rf_int).reg_int[result_add].wu = result.integer.wu;
			break;
		case OPERATION_MULT:
			result.integer.w = operand1.integer.w * operand2.integer.w;
			(state->rf_int).reg_int[result_add].w = result.integer.w;
			break;
		case OPERATION_DIV:
			result.integer.w = operand1.integer.w / operand2.integer.w;
			(state->rf_int).reg_int[result_add].w = result.integer.w;
			break;
		case OPERATION_SLL:
			result.integer.w = operand1.integer.w << operand2.integer.w;
			(state->rf_int).reg_int[result_add].w = result.integer.w;
			break;
		case OPERATION_SRL:
			result.integer.w = operand1.integer.w >> operand2.integer.w;
			(state->rf_int).reg_int[result_add].w = result.integer.w;
			break;
		case OPERATION_AND:
			result.integer.w = operand1.integer.w & operand2.integer.w;
			(state->rf_int).reg_int[result_add].w = result.integer.w;
			break;
		case OPERATION_OR:
			result.integer.w = operand1.integer.w | operand2.integer.w;
			(state->rf_int).reg_int[result_add].w = result.integer.w;
			break;
		case OPERATION_XOR:
			result.integer.w = operand1.integer.w ^ operand2.integer.w;
			(state->rf_int).reg_int[result_add].w = result.integer.w;
			break;
		case OPERATION_SLT:
			result.integer.w = operand1.integer.w < operand2.integer.w;
			(state->rf_int).reg_int[result_add].w = result.integer.w;
			break;
		case OPERATION_SGT:
			result.integer.w = operand1.integer.w > operand2.integer.w;
			(state->rf_int).reg_int[result_add].w = result.integer.w;
			break;
		case OPERATION_SLTU:
			result.integer.wu = operand1.integer.wu < operand2.integer.wu; //unsigned
			(state->rf_int).reg_int[result_add].wu = result.integer.wu;
			break;
		case OPERATION_SGTU:
			result.integer.wu = operand1.integer.wu > operand2.integer.wu; //unsigned
			(state->rf_int).reg_int[result_add].wu = result.integer.wu;
			break;
		default:
			break;
		}
	break;//case FU_GROUP_INT
		
	case FU_GROUP_MEM:
		*num_insn=*num_insn+1;	//increase number of instruction 
		operand1_add = FIELD_R1(instr_int);
		result_add = FIELD_R2(instr_int);
		operand1.integer.w = (state->rf_int).reg_int[operand1_add].w;
		operand2.integer.w = FIELD_IMM(instr_int);
			
		switch(op_int->operation){
		case OPERATION_LOAD:
			if(op_int->data_type == DATA_TYPE_W){
				memcpy( &((state->rf_int).reg_int[result_add].w), &(state->mem[operand1.integer.w + operand2.integer.w]), 4);
				//(state->rf_int).reg_int[result_add].w =  state->mem[operand1.integer.w + operand2.integer.w];
				//(state->rf_int).reg_int[result_add].w = ((state->rf_int).reg_int[result_add].w)<<8 + state->mem[operand1.integer.w + operand2.integer.w +1];
				//(state->rf_int).reg_int[result_add].w = ((state->rf_int).reg_int[result_add].w)<<8 + state->mem[operand1.integer.w + operand2.integer.w +2];
				//(state->rf_int).reg_int[result_add].w = ((state->rf_int).reg_int[result_add].w)<<8 + state->mem[operand1.integer.w + operand2.integer.w +3];
				//(state->rf_int).reg_int[result_add].w = memory[operand1.integer.w + operand2.integer.w];
				//(state->rf_int).reg_int[result_add].w = mem_char_to_int(memory, operand1.integer.w + operand2.integer.w)//[operand1.integer.w + operand2.integer.w];
			}
			else if(op_int->data_type == DATA_TYPE_F){
			
				memcpy( &((state->rf_fp).reg_fp[result_add]), &(state->mem[operand1.integer.w + operand2.integer.w]), 4);
				//(state->rf_fp).reg_fp[result_add] =  state->mem[operand1.integer.w + operand2.integer.w];
				//(state->rf_fp).reg_fp[result_add] = ((state->rf_fp).reg_fp[result_add])<<8 + state->mem[operand1.integer.w + operand2.integer.w +1];
				//(state->rf_fp).reg_fp[result_add] = ((state->rf_fp).reg_fp[result_add])<<8 + state->mem[operand1.integer.w + operand2.integer.w +2];
				//(state->rf_fp).reg_fp[result_add] = ((state->rf_fp).reg_fp[result_add])<<8 + state->mem[operand1.integer.w + operand2.integer.w +3];
				//(state->rf_fp).reg_fp[result_add] = memory[operand1.integer.w + operand2.integer.w];
				//(state->rf_fp).reg_fp[result_add] = mem_char_to_int(memory, operand1.integer.w + operand2.integer.w);
			}
		break;
		case OPERATION_STORE:
			if(op_int->data_type == DATA_TYPE_W)
				memcpy( &(state->mem[operand1.integer.w + operand2.integer.w]), &((state->rf_int).reg_int[result_add].w), 4);
				//state->mem[operand1.integer.w + operand2.integer.w] = ((state->rf_int).reg_int[result_add].w) & (0x000F);
				//state->mem[operand1.integer.w + operand2.integer.w + 1] = (((state->rf_int).reg_int[result_add].w)>>8) & (0x000F);
				//state->mem[operand1.integer.w + operand2.integer.w + 2] = (((state->rf_int).reg_int[result_add].w)>>16) & (0x000F);
				//state->mem[operand1.integer.w + operand2.integer.w + 3] = (((state->rf_int).reg_int[result_add].w)>>24) & (0x000F);
				 //memory[operand1.integer.w + operand2.integer.w] = (state->rf_int).reg_int[result_add].w;
				 //mem_int_to_char(memory, operand1.integer.w + operand2.integer.w, (state->rf_int).reg_int[result_add].w);
			else if(op_int->data_type == DATA_TYPE_F)
				memcpy( &(state->mem[operand1.integer.w + operand2.integer.w]), &((state->rf_fp).reg_fp[result_add]), 4);
				 //state->mem[operand1.integer.w + operand2.integer.w] = (state->rf_fp).reg_fp[result_add] & (0x000F);
				 //state->mem[operand1.integer.w + operand2.integer.w + 1] = ((state->rf_fp).reg_fp[result_add]>>8) & (0x000F);
				 //state->mem[operand1.integer.w + operand2.integer.w + 2] = ((state->rf_fp).reg_fp[result_add]>>16) & (0x000F);
				 //state->mem[operand1.integer.w + operand2.integer.w + 3] = ((state->rf_fp).reg_fp[result_add]>>24) & (0x000F);
				 //mem_int_to_char(memory, operand1.integer.w + operand2.integer.w, (state->rf_fp).reg_fp[result_add]);
		break;
		default:
		break;
		}
	break; //case FU_GROUP_MEM

	 
	
	case FU_GROUP_HALT:
		*num_insn=*num_insn+1;	//increase number of instruction 
	break;		
			
	case FU_GROUP_BRANCH:
		*num_insn=*num_insn+1;
		state->pc = state->pc_b + 4; //??+4
		state->if_id.pc=state->pc_b;
		memcpy( &((state->if_id).instr), &(state->mem[state->if_id.pc]), 4);
		//state->fetch_lock = FALSE;	//clear condition hazard
	break;
	
	case FU_GROUP_NONE:
    break;
	
	
	default:
		break;
		
	}
	
	switch(op_fp->fu_group_num){	
	//fp writeback	
	case FU_GROUP_ADD:
		*num_insn=*num_insn+1;	//increase number of instruction 
		operand1_add = FIELD_R1(instr_fp);
		operand2_add = FIELD_R2(instr_fp);
		result_add = FIELD_R3(instr_fp);
		operand1.flt = (state->rf_fp_temp).reg_fp[operand1_add];
		operand2.flt = (state->rf_fp_temp).reg_fp[operand2_add];
		if(op_fp->operation == OPERATION_ADD)
			result.flt = operand1.flt + operand2.flt;
		if(op_fp->operation == OPERATION_SUB)
			result.flt = operand1.flt - operand2.flt;
		(state->rf_fp).reg_fp[result_add] = result.flt;
	break;//case FU_GROUP_ADD
	
	case FU_GROUP_MULT:
		*num_insn=*num_insn+1;	//increase number of instruction 
		operand1_add = FIELD_R1(instr_fp);
		operand2_add = FIELD_R2(instr_fp);
		result_add = FIELD_R3(instr_fp);
		operand1.flt = (state->rf_fp_temp).reg_fp[operand1_add];
		operand2.flt = (state->rf_fp_temp).reg_fp[operand2_add];
		result.flt = operand1.flt * operand2.flt;
		(state->rf_fp).reg_fp[result_add] = result.flt;
	break;//case FU_GROUP_MULT
	
	case FU_GROUP_DIV:
		*num_insn=*num_insn+1;	//increase number of instruction 
		operand1_add = FIELD_R1(instr_fp);
		operand2_add = FIELD_R2(instr_fp);
		result_add = FIELD_R3(instr_fp);
		operand1.flt = (state->rf_fp_temp).reg_fp[operand1_add];
		operand2.flt = (state->rf_fp_temp).reg_fp[operand2_add];
		result.flt = operand1.flt / operand2.flt;
		(state->rf_fp).reg_fp[result_add] = result.flt;
	break;//case FU_GROUP_DIV
			
	}	
	
	(state->int_wb.instr)=0;
	(state->fp_wb.instr)=0;
	
	
}


void
execute(state_t *state) {

	const op_info_t *op_x, *op_d; 
	int imm_x, imm_d;
	int instr_d;
	fu_int_t *fu_int;
	fu_fp_t *fu_fp;
	
	fu_int_stage_t *stage_int;
	fu_fp_stage_t *stage_fp;
	
	int RAW_int, RAW_add, RAW_mult, RAW_div;
	int wb_add, wb_mult, wb_div;
	
	
	instr_d = (state->if_id).instr; //next instruction
	op_d = decode_instr(instr_d, &imm_d);
	

/********************Data Hazards Handling *************************/	
	//******************  case int  *************************//
	fu_int = state->fu_int_list;
	//Deal with RAW hazards
	/*
	if(RAW_int && (fu_int_done(fu_int)==TRUE) ){	//clear RAW hazard
		state->fetch_lock = FALSE;
		RAW_int=0;
	} */ 
	//while (fu_int != NULL) {
		stage_int = fu_int->stage_list;
		RAW_int=0; //calculate stalls
		while (stage_int!= NULL){	
			RAW_int++;
			op_x = decode_instr(stage_int->instr, &imm_x);
			switch(op_x->fu_group_num){
				case FU_GROUP_INT:
				if ( (imm_x==0 ) && (FIELD_R3(stage_int->instr)==FIELD_R1(instr_d) || FIELD_R3(stage_int->instr)==FIELD_R2(instr_d)) ) {
					if(stage_int->current_cycle != -1){
					state->fetch_lock = TRUE;
					if(RAW_int>state->stall)
						state->stall = RAW_int;
					}
				}
				else if( (imm_x==1) && (FIELD_R2(stage_int->instr)==FIELD_R1(instr_d) || FIELD_R2(stage_int->instr)==FIELD_R2(instr_d)) ){
					if(stage_int->current_cycle != -1){
					state->fetch_lock = TRUE;
					if(RAW_int>state->stall)
						state->stall = RAW_int;
					}
				}
				break;
				case FU_GROUP_MEM:
				if ( (FIELD_R2(stage_int->instr)==FIELD_R1(instr_d) || FIELD_R2(stage_int->instr)==FIELD_R2(instr_d))
						&& ((op_x->data_type == DATA_TYPE_W) || ( op_x->data_type == DATA_TYPE_F && op_d->fu_group_num != FU_GROUP_MEM) ) ) {
					if(stage_int->current_cycle != -1){
					state->fetch_lock = TRUE;
					if(RAW_int>state->stall)
						state->stall = RAW_int;
					}
				}
				break;
			}
			//printf("%d\n", op_x->fu_group_num);
			//printf("%d,%d,%d,%d,%d,%d\n", FIELD_R1(stage_int->instr), FIELD_R2(stage_int->instr), FIELD_R3(stage_int->instr), FIELD_R1(instr_d), FIELD_R2(instr_d), FIELD_R3(instr_d) );
			stage_int = stage_int->prev;
		}
		//fu_int = fu_int->next;
	//}
	//advance fu_int
	//stage_int = fu_int->stage_list;
	//if(stage_int->current_cycle==-1)
		//(state->int_wb).instr=0;
	advance_fu_int(fu_int, &(state->int_wb));

	
	
	//******************  case add  *************************//
	fu_fp = state->fu_add_list;
	//Deal with RAW hazard
	/*
	if( RAW_add && (fu_fp_done(fu_fp)==TRUE) ){	//clear RAW hazard
		state->fetch_lock = FALSE;
		RAW_add=0;
	}*/
	//while (fu_fp != NULL) {
		stage_fp = fu_fp->stage_list;
		RAW_add=0;
		while (stage_fp != NULL){
			RAW_add++;
			op_x = decode_instr(stage_fp->instr, &imm_x);
			switch(op_x->fu_group_num){
			case FU_GROUP_ADD:
			if ( (FIELD_R3(stage_fp->instr)==FIELD_R1(instr_d) || FIELD_R3(stage_fp->instr)==FIELD_R2(instr_d) )
				&& (op_d->fu_group_num == FU_GROUP_ADD || op_d->fu_group_num == FU_GROUP_MULT || op_d->fu_group_num == FU_GROUP_DIV ) ) {
				if(stage_fp->current_cycle != -1){
					state->fetch_lock = TRUE;
					if(RAW_add>state->stall)	//in case there is a larger stall before
						state->stall = RAW_add;
				}
			}
			if ( FIELD_R3(stage_fp->instr)==FIELD_R2(instr_d) 
				&& (op_d->operation == OPERATION_STORE && op_d->data_type == DATA_TYPE_F) ) {
				if(stage_fp->current_cycle != -1){
					state->fetch_lock = TRUE;
					if(RAW_add>state->stall)	//in case there is a larger stall before
						state->stall = RAW_add;
				}
			}
			if( RAW_add==3 && stage_fp->current_cycle!=-1 && op_d->operation == OPERATION_LOAD && op_d->data_type == DATA_TYPE_F){ //WAW, L.S & ADD
				state->fetch_lock=TRUE;
				if(state->stall<1)	//in case there is a larger stall before
					state->stall=1;
			}
			break;
			}
			stage_fp = stage_fp->prev;
		}
		//fu_fp = fu_fp->next;
	//}
	//advance fu_add
	//stage_fp = fu_fp->stage_list;
	//if(stage_fp->current_cycle==-1)
		//(state->fp_wb).instr=0;
	advance_fu_fp(fu_fp, &(state->fp_wb));
	
	
	//******************  case mult  *************************//
	fu_fp = state->fu_mult_list;
	//Deal with RAW hazard
	/*
	if( RAW_mult && (fu_fp_done(fu_fp)==TRUE) ){
		state->fetch_lock = FALSE;
		RAW_mult=0;
	}*/
	//while (fu_fp != NULL) {
		stage_fp = fu_fp->stage_list;
		RAW_mult=0;
		while (stage_fp != NULL){
			RAW_mult++;
			op_x = decode_instr(stage_fp->instr, &imm_x);
			switch(op_x->fu_group_num){
			case FU_GROUP_MULT:
			if ( (FIELD_R3(stage_fp->instr)==FIELD_R1(instr_d) || FIELD_R3(stage_fp->instr)==FIELD_R2(instr_d) )
				&& (op_d->fu_group_num == FU_GROUP_ADD || op_d->fu_group_num == FU_GROUP_MULT || op_d->fu_group_num == FU_GROUP_DIV) )  {
				if(stage_fp->current_cycle != -1){
					state->fetch_lock = TRUE;
					if(RAW_mult>state->stall)	//in case there is a larger stall before
						state->stall = RAW_mult;
				}
			}
			if( RAW_mult==3 && stage_fp->current_cycle!=-1 && op_d->fu_group_num == FU_GROUP_MEM && op_d->data_type == DATA_TYPE_F){ //Structural Hazard, L.S & MULT
				state->fetch_lock=TRUE;
				if(state->stall<1)	//in case there is a larger stall before
					state->stall=1;
			}
			if( RAW_mult==4 && stage_fp->current_cycle!=-1 && op_d->fu_group_num == FU_GROUP_MEM && op_d->data_type == DATA_TYPE_F && FIELD_R2(instr_d) == FIELD_R3(stage_fp->instr) ){
				state->fetch_lock=TRUE;
				if(state->stall<2)	//in case there is a larger stall before
					state->stall=2;
			}	//WAW, L.S & MULT
			if( RAW_mult==4 && stage_fp->current_cycle!=-1 && op_d->fu_group_num == FU_GROUP_ADD && op_d->data_type == DATA_TYPE_F){ //Structural Hazard, ADD & MULT
				state->fetch_lock=TRUE;
				if(state->stall<1)	//in case there is a larger stall before
					state->stall=1;
			}
			break;
			}
			stage_fp = stage_fp->prev;
		}
		//fu_fp = fu_fp->next;
	//}
		//stage_fp = fu_fp->stage_list;
		//if(stage_fp->current_cycle==-1)
			//(state->fp_wb).instr=0;
		advance_fu_fp(fu_fp, &(state->fp_wb));		//Structural Hazards when writeback? 
		
	
	//******************  case div  *************************//
	fu_fp = state->fu_div_list;
	//Deal with RAW hazard/
	/*
	if( RAW_div && (fu_fp_done(fu_fp)==TRUE) ){
		state->fetch_lock = FALSE;
		RAW_div=0;
	}*/
		stage_fp = fu_fp->stage_list;
		RAW_div=0;
		while (stage_fp != NULL){
			RAW_div++;
			op_x = decode_instr(stage_fp->instr, &imm_x);
			switch(op_x->fu_group_num){
			case FU_GROUP_DIV:
			if ( (FIELD_R3(stage_fp->instr)==FIELD_R1(instr_d) || FIELD_R3(stage_fp->instr)==FIELD_R2(instr_d) )
				&& (op_d->fu_group_num == FU_GROUP_ADD || op_d->fu_group_num == FU_GROUP_MULT || op_d->fu_group_num == FU_GROUP_DIV) )  {
				if(stage_fp->current_cycle != -1){
					state->fetch_lock = TRUE;
					if(RAW_div>state->stall)	//in case there is a larger stall before
						state->stall = RAW_div;
				}
			}
			if( RAW_div==3 && stage_fp->current_cycle!=-1 && op_d->fu_group_num == FU_GROUP_MEM && op_d->data_type == DATA_TYPE_F ){ //structural hazard, L.S & DIV
				state->fetch_lock=TRUE;
				if(state->stall<1)	//in case there is a larger stall before
					state->stall=1;
			}
			if( RAW_div>3 && stage_fp->current_cycle!=-1 && op_d->fu_group_num == FU_GROUP_MEM && op_d->data_type == DATA_TYPE_F && FIELD_R2(instr_d) == FIELD_R3(stage_fp->instr)){ //WAW, L.S & DIV
				state->fetch_lock=TRUE;
				if(state->stall<RAW_div-2)	//in case there is a larger stall before
					state->stall=RAW_div-2;
			}
			if( RAW_div==4 && stage_fp->current_cycle!=-1 && op_d->fu_group_num == FU_GROUP_ADD && op_d->data_type == DATA_TYPE_F){ //structural hazard, ADD & DIV
				state->fetch_lock=TRUE;
				if(state->stall<1)	//in case there is a larger stall before
					state->stall=1;
			}
			if( RAW_div>4 && stage_fp->current_cycle!=-1 && op_d->fu_group_num == FU_GROUP_ADD && op_d->data_type == DATA_TYPE_F && FIELD_R3(instr_d) == FIELD_R3(stage_fp->instr)){ //WAW, ADD & DIV
				state->fetch_lock=TRUE;
				if(state->stall<RAW_div-3)	//in case there is a larger stall before
					state->stall=RAW_div-3;
			}
			if( RAW_div==5 && stage_fp->current_cycle!=-1 && op_d->fu_group_num == FU_GROUP_MULT && op_d->data_type == DATA_TYPE_F){ //structural hazard, MULT & DIV
				state->fetch_lock=TRUE;
				if(state->stall<1)	//in case there is a larger stall before
					state->stall=1;
			}
			if( RAW_div>5 && stage_fp->current_cycle!=-1 && op_d->fu_group_num == FU_GROUP_MULT && op_d->data_type == DATA_TYPE_F && FIELD_R3(instr_d) == FIELD_R3(stage_fp->instr)){ //WAW, MULT & DIV
				state->fetch_lock=TRUE;
				if(state->stall<RAW_div-4)	//in case there is a larger stall before
					state->stall=RAW_div-4;
			}
			break;
			}
			stage_fp = stage_fp->prev;
		}
		//stage_fp = fu_fp->stage_list;
		//if(stage_fp->current_cycle==-1)
			//(state->fp_wb).instr=0;
		advance_fu_fp(fu_fp, &(state->fp_wb));		//Structural Hazards?
	
	
	
		

		
	
	
}


int
decode(state_t *state) {

	
	const op_info_t *op_d; 
	int imm_d;
	int instr_d;
	unsigned long pc_d;
	int str_hazard=0;
	
	
	fu_int_t *fu_int;
	fu_fp_t *fu_fp;
	
	instr_d = (state->if_id).instr;
	pc_d = (state->if_id).pc;
	op_d = decode_instr(instr_d, &imm_d); 
	
/********************      Issue      *****************************
	fu_int = state->fu_int_list;
	if ( issue_fu_int(fu_int, instr_d) == -1)	//structural hazard when issue
		state->fetch_lock=TRUE;
	
	fu_fp = state->fu_add_list;
	if ( issue_fu_fp(fu_fp, instr_d) == -1)	//structural hazard when issue
		state->fetch_lock=TRUE;	
	
	fu_fp = state->fu_mult_list;
	if ( issue_fu_fp(fu_fp, instr_d) == -1)	//structural hazard when issue
		state->fetch_lock=TRUE;	
		
	fu_fp = state->fu_add_list;
	if ( issue_fu_fp(fu_fp, instr_d) == -1)	//structural hazard when issue
		state->fetch_lock=TRUE;	

********************Perform Operation*****************************/	

	
	switch(op_d->fu_group_num) {
	case FU_GROUP_INT:
		fu_int = state->fu_int_list;
		if ( issue_fu_int(fu_int, instr_d) == -1)	//structural hazard when issue
			state->fetch_lock=TRUE;	//How to clear this lock?
	break;
	
	case FU_GROUP_MEM:
		fu_int = state->fu_int_list;
		if ( issue_fu_int(fu_int, instr_d) == -1)	//structural hazard when issue
			state->fetch_lock=TRUE;	//How to clear this lock?
	break;
	
	case FU_GROUP_ADD:
		(state->rf_fp_temp).reg_fp[FIELD_R1(instr_d)] =	(state->rf_fp).reg_fp[FIELD_R1(instr_d)] ;
		(state->rf_fp_temp).reg_fp[FIELD_R2(instr_d)] =	(state->rf_fp).reg_fp[FIELD_R2(instr_d)] ;
		fu_fp = state->fu_add_list;
		if ( issue_fu_fp(fu_fp, instr_d) == -1)	//structural hazard when issue
			state->fetch_lock=TRUE;	
	break;
			
	case FU_GROUP_MULT:
		(state->rf_fp_temp).reg_fp[FIELD_R1(instr_d)] =	(state->rf_fp).reg_fp[FIELD_R1(instr_d)] ;
		(state->rf_fp_temp).reg_fp[FIELD_R2(instr_d)] =	(state->rf_fp).reg_fp[FIELD_R2(instr_d)] ;
		fu_fp = state->fu_mult_list;
		if ( issue_fu_fp(fu_fp, instr_d) == -1)	//structural hazard when issue
			state->fetch_lock=TRUE;	
	break;
	
	case FU_GROUP_DIV:
		(state->rf_fp_temp).reg_fp[FIELD_R1(instr_d)] =	(state->rf_fp).reg_fp[FIELD_R1(instr_d)] ;
		(state->rf_fp_temp).reg_fp[FIELD_R2(instr_d)] =	(state->rf_fp).reg_fp[FIELD_R2(instr_d)] ;
		fu_fp = state->fu_div_list;
		if ( issue_fu_fp(fu_fp, instr_d) == -1)	//structural hazard when issue
			state->fetch_lock=TRUE;	
	break;
	
	case FU_GROUP_BRANCH:
		switch(op_d->operation){		
			case OPERATION_J:
			//printf("PC_d = %d\n", pc_d);
				pc_d = pc_d + FIELD_OFFSET(instr_d) +4;	//pc = pc + off + 4?
				state->stall=3;
				break;
			case OPERATION_JAL:
				(state->rf_int).reg_int[31].wu = pc_d;		//reg[31] = pc, write back PC? Perhaps wait a few cycles
				pc_d = pc_d + FIELD_OFFSET(instr_d) +4;  	//pc = pc + off + 4
				state->stall=3;
				break;
			case OPERATION_JR:
				pc_d = (state->rf_int).reg_int[ FIELD_R1(instr_d) ].w ;	//pc = reg[r1]
				state->stall=3;
				break;
			case OPERATION_JALR:
				(state->rf_int).reg_int[31].wu = pc_d;					//reg[31] = pc, write back PC?
				pc_d = (state->rf_int).reg_int[ FIELD_R1(instr_d) ].w ;   //pc = reg[r1]
				state->stall=3;
				break;
			case OPERATION_BEQZ:
				if( (state->rf_int).reg_int[ FIELD_R1(instr_d) ].w == 0  ){
					state->stall=3;
					pc_d =  pc_d + FIELD_IMM(instr_d) + 4;
				}
				else{
					state->stall=2;
					pc_d =  pc_d + 4;
				}
				break;
			case OPERATION_BNEZ:
				if( (state->rf_int).reg_int[ FIELD_R1(instr_d) ].w != 0 ){
					pc_d =  pc_d + FIELD_IMM(instr_d) + 4 ;
					state->stall=3;
				}
				else{
					pc_d =  pc_d + 4 ;
					state->stall=2;
				}
				break;
		}
		//printf("PC_d = %d\n", pc_d);
		state->pc_b = pc_d; //
		state->fetch_lock=TRUE; //control hazards, how to clear this lock?
		
		
		fu_int = state->fu_int_list;
		if ( issue_fu_int(fu_int, instr_d) == -1)	//structural hazard when issue
			state->fetch_lock=TRUE;	//How to clear this lock?
		

	break;

	case FU_GROUP_HALT:	//use fu_int
		//printf("inside\n");
			state->fetch_lock=TRUE;
			state->halt =1;
		
		break;

		/*
	case FU_GROUP_NONE:
		fu_int = state->fu_int_list;
		if ( issue_fu_int(fu_int, instr_d) == -1)	//structural hazard when issue
			state->fetch_lock=TRUE;	//How to clear this lock? 
		break;
*/

	case FU_GROUP_INVALID:
		//fprintf(stderr, "error: invalid opcode (instr = %.8X)\n", instr_d);
		break;
  }
  

	
}


void
fetch(state_t *state) {
	/*
	int *memory;
	memory= &(state->mem[0]);
	(state->if_id).instr = memory[state->pc];
	for(i=0; i<4; i++)
		(state->if_id).instr = (((state->if_id).instr)<<8) + state->mem[ state->pc + i ];

	
	*/
	int i;
	state->if_id.pc=state->pc;
	memcpy( &((state->if_id).instr), &(state->mem[state->pc]), 4);
	state->pc += 4;

}

/*
int 
mem_char_to_int(char *addr, unsigned long offset){
	int out=0;
	for(int i=0; i<4; i++)
		out=out<<8 + addr[offset+i];
	return out;
}
	
void 
mem_int_to_char(char *addr, unsigned long offset, int value){
	addr[offset] = value & (0x000F);
	addr[offset+1] = (value>>8) & (0x000F);
	addr[offset+2] = (value>>16) & (0x000F);
	addr[offset+3] = (value>>24) & (0x000F);
}	
*/
	