
/*
 * output.c
 * 
 * Donald Yeung
 */


#include <stdio.h>
#include "fu.h"
#include "pipeline.h"
#include "output.h"


/************************************************************/
void
print_state(state_t *state,int num_memory) {
  int i, index;

  printf("Memory\n");
  printf("\tAddress\t\tData");
  for (i = 0; i < num_memory; i++) {
    if((i & 0x0000000F) == 0) {
      printf("\n\t0x%.8X\t%.2X",i,(unsigned int)state->mem[i]);
    } else {
      printf(" %.2X",(unsigned int)state->mem[i]);
    }
  }
  printf("\n");

  printf("Registers (integer):\n");
  for (i = 0; i < NUMREGS; i += 4)
    printf("\tR%d=0x%.8X\tR%d=0x%.8X\tR%d=0x%.8X\tR%d=0x%.8X\n",
	   i, state->rf_int.reg_int.integer[i].wu,
	   i+1, state->rf_int.reg_int.integer[i+1].wu,
	   i+2, state->rf_int.reg_int.integer[i+2].wu,
	   i+3, state->rf_int.reg_int.integer[i+3].wu);

  printf("Registers (floating point):\n");
  for (i = 0; i < NUMREGS; i += 4)
    printf("\tF%d=%-10.6g\tF%d=%-10.6g\tF%d=%-10.6g\tF%d=%-10.6g\n", 
	   i, state->rf_fp.reg_fp.flt[i],
	   i+1, state->rf_fp.reg_fp.flt[i+1],
	   i+2, state->rf_fp.reg_fp.flt[i+2],
	   i+3, state->rf_fp.reg_fp.flt[i+3]);

  printf("pc:\n");
  printf("\tpc\t0x%.8X\n",state->pc);

  printf("ifid:\n");
  printf("\tinstr\t");
  printInstruction(state->if_id.instr);
  printf("\n");

  printf("IQ:\n");
  for (index = state->IQ_head; index != state->IQ_tail; 
       index = (index + 1) & (IQ_SIZE-1)) {
    printf("\t\t%d  ", state->IQ[index].ROB_index);
    printInstruction(state->IQ[index].instr);
    if (state->IQ[index].tag1 == -1)
      printf("  READY (%d)", state->IQ[index].operand1.integer.w);
    else
      printf("  TAG=%2d", state->IQ[index].tag1);
    if (state->IQ[index].tag2 == -1)
      printf("  READY (%d)", state->IQ[index].operand2.integer.w);
    else
      printf("  TAG=%2d", state->IQ[index].tag2);
    printf("  %s\n", state->IQ[index].issued ? "ISSUED" : "NOT ISSUED");
  }

  printf("CQ:\n");
  for (index = state->CQ_head; index != state->CQ_tail; 
       index = (index + 1) & (CQ_SIZE-1)) {
    printf("\t\t%d  ", state->CQ[index].ROB_index);
    printInstruction(state->CQ[index].instr);
    if (state->CQ[index].tag1 == -1)
      printf("  READY ");
    else
      printf("  TAG=%2d", state->CQ[index].tag1);
    if (state->CQ[index].tag2 == -1)
      printf("  READY ");
    else
      printf("  TAG=%2d", state->CQ[index].tag2);
    printf("  %s\n", state->CQ[index].issued ? "ISSUED" : "NOT ISSUED");
  }

  printf("%s fu:\n",fu_group_int_name);
  print_fu_group_int(state->fu_int_list);
  printf("%s fu:\n",fu_group_mem_name);
  print_fu_group_mem(state->fu_mem_list);
  printf("%s fu:\n",fu_group_add_name);
  print_fu_group_fp(state->fu_add_list);
  printf("%s fu:\n",fu_group_mult_name);
  print_fu_group_fp(state->fu_mult_list);
  printf("%s fu:\n",fu_group_div_name);
  print_fu_group_fp(state->fu_div_list);

  printf("integer writeback ports:\n");
  for (i = 0; i < state->wb_port_int_num; i++) {
    if (state->wb_port_int[i].tag != -1) {
      if (state->wb_port_int[i].tag < ROB_SIZE)
	printf("\t\tTAG = %d, result = 0x%.8X\n", state->wb_port_int[i].tag,
	       state->ROB[state->wb_port_int[i].tag].result.integer.w);
      else
	printf("\t\tTAG = %d, result = 0x%.8X\n", state->wb_port_int[i].tag,
	       state->ROB[state->wb_port_int[i].tag-ROB_SIZE].target.integer.w);
    }    
  }

  printf("floating writeback ports:\n");
  for (i = 0; i < state->wb_port_fp_num; i++) {
    if (state->wb_port_fp[i].tag != -1)
      printf("\t\tTAG = %d, result = 0x%.8X\n", state->wb_port_fp[i].tag,
	     state->ROB[state->wb_port_fp[i].tag].result.integer.w);
  }

  printf("branch PC update:\n");
  if (state->branch_tag != -1)
    printf("\t\tTAG = %d, outcome = %d, target = 0x%.8X\n", state->branch_tag,
	   state->ROB[state->branch_tag].result.integer.w,
	   state->ROB[state->branch_tag].target.integer.w);

  printf("ROB:\n");
  for (index = state->ROB_head; index != state->ROB_tail; 
       index = (index + 1) & (ROB_SIZE-1)) {
    printf("\t\t%d  ", index);
    printInstruction(state->ROB[index].instr);
    printf("  %s\n", state->ROB[index].completed ? "COMPLETED" : "NOT COMPLETED");
  }

  printf("\n");
}
/************************************************************/


void
print_fu_group_int(fu_int_t *fu_int_list) {
  fu_int_t *fu_int;
  fu_int_stage_t *stage;
  int j;

  fu_int = fu_int_list;
  while (fu_int != NULL) {
    j = 0;
    stage = fu_int->stage_list;
    while (stage != NULL) {
      if (stage->current_cycle != -1)
	printf("\t\tname '%s', stage %d (from end), cycle %d/%d, TAG = %d\n",
	       fu_int->name, j, stage->current_cycle, stage->num_cycles, stage->tag);
      j++;
      stage = stage->prev;
    }
    fu_int = fu_int->next;
  }
}

void
print_fu_group_mem(fu_mem_t *fu_mem_list) {
  fu_mem_t *fu_mem;
  fu_mem_stage_t *stage;
  int j;

  fu_mem = fu_mem_list;
  while (fu_mem != NULL) {
    j = 0;
    stage = fu_mem->stage_list;
    while (stage != NULL) {
      if (stage->current_cycle != -1)
	printf("\t\tname '%s', stage %d (from end), cycle %d/%d, TAG = %d\n",
	       fu_mem->name, j, stage->current_cycle, stage->num_cycles, stage->tag);
      j++;
      stage = stage->prev;
    }
    fu_mem = fu_mem->next;
  }
}

void
print_fu_group_fp(fu_fp_t *fu_fp_list) {
  fu_fp_t *fu_fp;
  fu_fp_stage_t *stage;
  int j;

  fu_fp = fu_fp_list;
  while(fu_fp != NULL) {
    j = 0;
    stage = fu_fp->stage_list;
    while(stage != NULL) {
      if(stage->current_cycle != -1) {
	printf("\t\tname '%s', stage %d (from end), cycle %d/%d, TAG = %d\n",
	       fu_fp->name, j, stage->current_cycle, stage->num_cycles, stage->tag);
      }
      j++;
      stage = stage->prev;
    }
    fu_fp = fu_fp->next;
  }
}


/************************************************************/
void
printInstruction(int instr) {
  const op_info_t *op_info;

  if(op_table[FIELD_OPCODE(instr)].sub_table == NULL) {
    op_info = &op_table[FIELD_OPCODE(instr)].info;
    if(op_info->name == NULL)
      printf("0x%.8X",instr);
    else {
      switch(op_info->fu_group_num) {
      case FU_GROUP_INT:
	printf("%s R%d R%d #%d",op_info->name,FIELD_R2(instr),FIELD_R1(instr),FIELD_IMM(instr));
	break;
      case FU_GROUP_MEM:
	switch(op_info->data_type) {
	case DATA_TYPE_W:
	  printf("%s R%d (%d)R%d",op_info->name,FIELD_R2(instr),FIELD_IMM(instr),FIELD_R1(instr));
	  break;
	case DATA_TYPE_F:
	  printf("%s F%d (%d)R%d",op_info->name,FIELD_R2(instr),FIELD_IMM(instr),FIELD_R1(instr));
	  break;
	}
	break;
      case FU_GROUP_BRANCH:
	switch(op_info->operation) {

	case OPERATION_JAL:
	case OPERATION_J:
	  printf("%s #%d",op_info->name,FIELD_OFFSET(instr));
	  break;

	case OPERATION_JALR:
	case OPERATION_JR:
	  printf("%s R%d",op_info->name,FIELD_R1(instr));
	  break;

	case OPERATION_BEQZ:
	case OPERATION_BNEZ:
	  printf("%s R%d #%d",op_info->name,FIELD_R1(instr),FIELD_IMM(instr));
	  break;

	}
	break;
      default:
	printf("%s",op_info->name);
      }
    }
  } else {
    op_info = &op_table[FIELD_OPCODE(instr)].sub_table[FIELD_FUNC(instr)].info;
    if(op_info->name == NULL)
      printf("0x%.8X",instr);
    else {
      switch(op_info->fu_group_num) {
      case FU_GROUP_INT:
	printf("%s R%d R%d R%d",op_info->name,FIELD_R3(instr),FIELD_R1(instr),FIELD_R2(instr));
	break;
      case FU_GROUP_ADD:
      case FU_GROUP_MULT:
      case FU_GROUP_DIV:
	printf("%s F%d F%d F%d",op_info->name,FIELD_R3(instr),FIELD_R1(instr),FIELD_R2(instr));
	break;
      default:
	printf("%s",op_info->name);
      }
    }
  }
}
/************************************************************/
