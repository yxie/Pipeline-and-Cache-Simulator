
/*
 * 
 * fu.c
 * 
   This module was originally written by Paul Kohout and adapted for
   this simulator.

 * Donald Yeung
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fu.h"
#include "pipeline.h"

#define MAX_FIELD_LENGTH 100

const char fu_group_int_name[] = "INT";
const char fu_group_add_name[] =  "ADD";
const char fu_group_mult_name[] =  "MULT";
const char fu_group_div_name[] =  "DIV";


/*
  {{name, fu_group_num, operation, data_type}, sub_table}
*/

const op_t op_table[] = {
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},op_special_table},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},op_fparith_table},
  {{"J",      FU_GROUP_BRANCH, OPERATION_J,    DATA_TYPE_NONE},NULL},
  {{"JAL",    FU_GROUP_BRANCH, OPERATION_JAL,  DATA_TYPE_NONE},NULL},
  {{"BEQZ",   FU_GROUP_BRANCH, OPERATION_BEQZ, DATA_TYPE_NONE},NULL},
  {{"BNEZ",   FU_GROUP_BRANCH, OPERATION_BNEZ, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{"ADDI",   FU_GROUP_INT,    OPERATION_ADD,  DATA_TYPE_NONE},NULL},
  {{"ADDUI",  FU_GROUP_INT,    OPERATION_ADDU, DATA_TYPE_NONE},NULL},
  {{"SUBI",   FU_GROUP_INT,    OPERATION_SUB,  DATA_TYPE_NONE},NULL},
  {{"SUBUI",  FU_GROUP_INT,    OPERATION_SUBU, DATA_TYPE_NONE},NULL},
  {{"ANDI",   FU_GROUP_INT,    OPERATION_AND,  DATA_TYPE_NONE},NULL},
  {{"ORI",    FU_GROUP_INT,    OPERATION_OR,   DATA_TYPE_NONE},NULL},
  {{"XORI",   FU_GROUP_INT,    OPERATION_XOR,  DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{"JR",     FU_GROUP_BRANCH, OPERATION_JR,   DATA_TYPE_NONE},NULL},
  {{"JALR",   FU_GROUP_BRANCH, OPERATION_JALR, DATA_TYPE_NONE},NULL},
  {{"SLLI",   FU_GROUP_INT,    OPERATION_SLL,  DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{"SRLI",   FU_GROUP_INT,    OPERATION_SRL,  DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{"SLTI",   FU_GROUP_INT,    OPERATION_SLT,  DATA_TYPE_NONE},NULL},
  {{"SGTI",   FU_GROUP_INT,    OPERATION_SGT,  DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{"LW",     FU_GROUP_MEM,    OPERATION_LOAD, DATA_TYPE_W},   NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{"L.S",    FU_GROUP_MEM,    OPERATION_LOAD, DATA_TYPE_F},   NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{"SW",     FU_GROUP_MEM,    OPERATION_STORE,DATA_TYPE_W},   NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{"S.S",    FU_GROUP_MEM,    OPERATION_STORE,DATA_TYPE_F},   NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{"SLTUI",  FU_GROUP_INT,    OPERATION_SLTU, DATA_TYPE_NONE},NULL},
  {{"SGTUI",  FU_GROUP_INT,    OPERATION_SGTU, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},NULL},
  {{"HALT",   FU_GROUP_HALT,   OPERATION_NONE, DATA_TYPE_NONE},NULL}
};

const sub_op_t op_special_table[] = {
  {"NOP",    FU_GROUP_NONE,   OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {"SLL",    FU_GROUP_INT,    OPERATION_SLL,  DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {"SRL",    FU_GROUP_INT,    OPERATION_SRL,  DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {"SLTU",   FU_GROUP_INT,    OPERATION_SLTU, DATA_TYPE_NONE},
  {"SGTU",   FU_GROUP_INT,    OPERATION_SGTU, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {"ADD",    FU_GROUP_INT,    OPERATION_ADD,  DATA_TYPE_NONE},
  {"ADDU",   FU_GROUP_INT,    OPERATION_ADDU, DATA_TYPE_NONE},
  {"SUB",    FU_GROUP_INT,    OPERATION_SUB,  DATA_TYPE_NONE},
  {"SUBU",   FU_GROUP_INT,    OPERATION_SUBU, DATA_TYPE_NONE},
  {"AND",    FU_GROUP_INT,    OPERATION_AND,  DATA_TYPE_NONE},
  {"OR",     FU_GROUP_INT,    OPERATION_OR,   DATA_TYPE_NONE},
  {"XOR",    FU_GROUP_INT,    OPERATION_XOR,  DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {"SLT",    FU_GROUP_INT,    OPERATION_SLT,  DATA_TYPE_NONE},
  {"SGT",    FU_GROUP_INT,    OPERATION_SGT,  DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE}
};

const sub_op_t op_fparith_table[] = {
  {"ADD.S",  FU_GROUP_ADD,    OPERATION_ADD,  DATA_TYPE_F},
  {"SUB.S",  FU_GROUP_ADD,    OPERATION_SUB,  DATA_TYPE_F},
  {"MULT.S", FU_GROUP_MULT,   OPERATION_MULT, DATA_TYPE_F},
  {"DIV.S",  FU_GROUP_DIV,    OPERATION_DIV,  DATA_TYPE_F},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE},
  {NULL,     FU_GROUP_INVALID,OPERATION_NONE, DATA_TYPE_NONE}
};


state_t *
state_create(int *data_count, FILE *bin_file, FILE *fu_file) {
  state_t *state;
  fu_int_stage_t *cur_int_stage;
  fu_fp_stage_t *cur_fp_stage;
  char field[MAX_FIELD_LENGTH];
  char field_fmt[16];
  int num;
  int i;

  state = (state_t *)malloc(sizeof(state_t));
  if(state == NULL) {
    fprintf(stderr, "error: unable to allocate resources\n");
    return NULL;
  }

  memset(state, 0, sizeof(state_t));

  /* read machine-code file into instruction/data memory (starting at address 0) */
  i=0;
  while (!feof(bin_file)) {
    if (fread(&state->mem[i], 1, 1, bin_file) != 0) {
      i++;
    } else if (!feof(bin_file)) {
      fprintf(stderr, "error: cannot read address 0x%X from binary file\n", i);
      return NULL;      
    }
  }
  if (data_count != NULL)
    *data_count = i;

  /* allocate and initialize functional unit linked lists */
  while (!feof(fu_file)) {

    sprintf(field_fmt, " %%%d[^,\n]", MAX_FIELD_LENGTH);
    //printf("%s", field_fmt);while(1); 
    if (fscanf(fu_file, field_fmt, field) != 1) {
      fprintf(stderr, "error: cannot parse options file\n");
      return NULL;
    }
    i = strlen(field)-1;
    while (i > 0 && (field[i] == ' ' || field[i] == '\t'))
      field[i--] = 0;
    if (strcmp(field, fu_group_int_name) == 0) {
      if (fu_int_read(&state->fu_int_list, fu_file) != 0) {
	return NULL;
      }
    } else if (strcmp(field, fu_group_add_name) == 0) {
      if (fu_fp_read(&state->fu_add_list, fu_file) != 0) {
	return NULL;
      }
    } else if (strcmp(field, fu_group_mult_name) == 0) {
      if (fu_fp_read(&state->fu_mult_list, fu_file) != 0) {
	return NULL;
      }
    } else if (strcmp(field, fu_group_div_name) == 0) {
      if (fu_fp_read(&state->fu_div_list, fu_file) != 0) {
	return NULL;
      }
    } else {
      fprintf(stderr,"error: invalid functional unit group name\n");
      return NULL;
    }
  }
  
  if (state->fu_int_list == NULL) {
    fprintf(stderr,"error: no %s functional units\n", fu_group_int_name);
    return NULL;
  }
  if (state->fu_add_list == NULL) {
    fprintf(stderr,"error: no %s functional units\n", fu_group_add_name);
    return NULL;
  }
  if (state->fu_mult_list == NULL) {
    fprintf(stderr,"error: no %s functional units\n", fu_group_mult_name);
    return NULL;
  }
  if (state->fu_div_list == NULL) {
    fprintf(stderr,"error: no %s functional units\n", fu_group_div_name);
    return NULL;
  }

  /* initialize wb ports */
  state->int_wb.instr = 0/*NOP*/;
  state->fp_wb.instr = 0/*NOP*/;

  state->fetch_lock = FALSE;

  return state;
}


/* functions to parse FU file */
int
fu_int_read(fu_int_t **fu_int_list, FILE *file) {
  fu_int_t *fu_int;
  fu_int_stage_t *stage;
  char field[MAX_FIELD_LENGTH];
  char field_fmt[16];
  int num;
  int i;

  sprintf(field_fmt, ", %%%d[^,\n]", MAX_FIELD_LENGTH);
  if (fscanf(file, field_fmt, field) != 1) {
    fprintf(stderr, "error: cannot parse options file\n");
    return -1;
  }
  i = strlen(field)-1;
  while (i > 0 && (field[i] == ' ' || field[i] == '\t'))
    field[i--] = 0;

  if (*fu_int_list == NULL) {
    *fu_int_list = (fu_int_t *)malloc(sizeof(fu_int_t));
    fu_int = *fu_int_list;
  } else {
    fu_int = *fu_int_list;
    while (fu_int->next != NULL) fu_int = fu_int->next;
    fu_int->next = (fu_int_t *)malloc(sizeof(fu_int_t));
    fu_int = fu_int->next;
  }
  if (fu_int == NULL) {
    fprintf(stderr, "error: unable to allocate resources\n");
    return -1;
  }

  fu_int->name = (char *)malloc(strlen(field)+1);
  if (fu_int->name == NULL) {
    fprintf(stderr, "error: unable to allocate resources\n");
    return -1;
  }
  fu_int->stage_list = NULL;
  fu_int->next = NULL;
  strcpy(fu_int->name, field);

  stage = NULL;
  while (fscanf(file, ", %d", &num) == 1) {

    if(num <= 0) {
      fprintf(stderr, "error: functional unit '%s' has a stage with an invalid number of cycles\n", fu_int->name);
      return -1;
    }

    stage = (fu_int_stage_t *)malloc(sizeof(fu_int_stage_t));
    if (stage == NULL) {
      fprintf(stderr, "error: unable to allocate resources\n");
      return -1;
    }
    stage->prev = fu_int->stage_list;
    fu_int->stage_list = stage;
    stage->num_cycles = num;
    stage->current_cycle = -1;
  }
  if (fu_int->stage_list == NULL) {
    fprintf(stderr, "error: functional unit '%s' has no stages\n", fu_int->name);
    return -1;
  }

  return 0;
}


int
fu_fp_read(fu_fp_t **fu_fp_list,FILE *file) {
  fu_fp_t *fu_fp;
  fu_fp_stage_t *stage;
  char field[MAX_FIELD_LENGTH];
  char field_fmt[16];
  int num;
  int i;

  sprintf(field_fmt,", %%%d[^,\n]",MAX_FIELD_LENGTH);
  if(fscanf(file,field_fmt,field) != 1) {
    fprintf(stderr,"error: cannot parse options file\n");
    return -1;
  }
  i = strlen(field)-1;
  while(i > 0 && (field[i] == ' ' || field[i] == '\t'))
    field[i--] = 0;

  if(*fu_fp_list == NULL) {
    *fu_fp_list = (fu_fp_t *)malloc(sizeof(fu_fp_t));
    fu_fp = *fu_fp_list;
  } else {
    fu_fp = *fu_fp_list;
    while(fu_fp->next != NULL) fu_fp = fu_fp->next;
    fu_fp->next = (fu_fp_t *)malloc(sizeof(fu_fp_t));
    fu_fp = fu_fp->next;
  }
  if(fu_fp == NULL) {
    fprintf(stderr,"error: unable to allocate resources\n");
    return -1;
  }

  fu_fp->name = (char *)malloc(strlen(field)+1);
  if(fu_fp->name == NULL) {
    fprintf(stderr,"error: unable to allocate resources\n");
    return -1;
  }
  fu_fp->stage_list = NULL;
  fu_fp->next = NULL;
  strcpy(fu_fp->name,field);

  stage = NULL;
  while(fscanf(file,", %d",&num) == 1) {

    if(num <= 0) {
      fprintf(stderr,"error: functional unit '%s' has a stage with an invalid number of cycles\n",fu_fp->name);
      return -1;
    }

    stage = (fu_fp_stage_t *)malloc(sizeof(fu_fp_stage_t));
    if(stage == NULL) {
      fprintf(stderr,"error: unable to allocate resources\n");
      return -1;
    }
    stage->prev = fu_fp->stage_list;
    fu_fp->stage_list = stage;
    stage->num_cycles = num;
    stage->current_cycle = -1;
  }
  if(fu_fp->stage_list == NULL) {
    fprintf(stderr,"error: functional unit '%s' has no stages\n",fu_fp->name);
    return -1;
  }

  return 0;
}


/* Functions to allocate functional units */
int
issue_fu_int(fu_int_t *fu_list, int instr) {
  fu_int_t *fu;
  fu_int_stage_t *stage;

  fu = fu_list;
  while (fu != NULL) {
    stage = fu->stage_list;
    while (stage->prev != NULL)
      stage = stage->prev;
    if (stage->current_cycle == -1) {
      stage->current_cycle = stage->num_cycles-1;
      stage->instr = instr;
      return 0;
    }
    fu = fu->next;
  }
  return -1;   // structural hazard... stall
}


int
issue_fu_fp(fu_fp_t *fu_list, int instr) {
  fu_fp_t *fu;
  fu_fp_stage_t *stage;

  fu = fu_list;
  while (fu != NULL) {
    stage = fu->stage_list;
    while (stage->prev != NULL)
      stage = stage->prev;
    if (stage->current_cycle == -1) {
      stage->current_cycle = stage->num_cycles-1;
      stage->instr = instr;
      return 0;
    }
    fu = fu->next;
  }
  return -1;   // structural hazard... stall
}


/* functions to cycle functional units */
void
advance_fu_int(fu_int_t *fu_list, wb_t *int_wb) {
  fu_int_t *fu;
  fu_int_stage_t *stage, *next_stage;
  int i;

  fu = fu_list;
  while (fu != NULL) {
    stage = fu->stage_list;
    next_stage = NULL;
    while (stage != NULL) {
      switch (stage->current_cycle) {
	/* is fu stage free? */
      case -1:
	break;                                      /* do nothing */

	/* is fu stage done processing? */
      case 0:
	if (next_stage == NULL) {                    /* is this the last stage in the fu? */
	  int_wb->instr = stage->instr;
	  stage->current_cycle = -1;
	} else {
	  if (next_stage->current_cycle == -1) {     /* move to next fu stage */
	    next_stage->current_cycle = next_stage->num_cycles-1;
	    next_stage->instr = stage->instr;
	    stage->current_cycle = -1;
	  }
	}
	break;

	/*  fu stage is still processing */
      default:
	stage->current_cycle--;
      }
      next_stage = stage;
      stage = stage->prev;
    }
    fu = fu->next;
  }
}


void
advance_fu_fp(fu_fp_t *fu_list, wb_t *fp_wb) {
  fu_fp_t *fu;
  fu_fp_stage_t *stage, *next_stage;
  int i;

  fu = fu_list;
  while (fu != NULL) {
    stage = fu->stage_list;
    next_stage = NULL;
    while(stage != NULL) {
      switch(stage->current_cycle) {
	/* is fu stage free? */
      case -1:                                      /* do nothing */
	break;

	/* is fu stage done processing? */
      case 0:
	if(next_stage == NULL) {                  /* is this the last stage in the fu? */
	  fp_wb->instr = stage->instr;
	  stage->current_cycle = -1;
	} else {
	  if(next_stage->current_cycle == -1) {                                  /* move to next fu stage */
	    next_stage->current_cycle = next_stage->num_cycles-1;
	    next_stage->instr = stage->instr;
	    stage->current_cycle = -1;
	  }
	}
	break;

	/* fu stage is still processing */
      default:   
	stage->current_cycle--;
      }
      next_stage = stage;
      stage = stage->prev;
    }
    fu = fu->next;
  }
}


int
fu_int_done(fu_int_t *fu_list)
{
  fu_int_t *fu;
  fu_int_stage_t *stage;

  fu = fu_list;
  while (fu != NULL) {
    stage = fu->stage_list;
    while (stage != NULL) {
      if (stage->current_cycle != -1)
	return FALSE;
      stage = stage->prev;
    }
    fu = fu->next;
  }

  return TRUE;
}


int
fu_fp_done(fu_fp_t *fu_list)
{
  fu_fp_t *fu;
  fu_fp_stage_t *stage;

  fu = fu_list;
  while (fu != NULL) {
    stage = fu->stage_list;
    while (stage != NULL) {
      if (stage->current_cycle != -1)
	return FALSE;
      stage = stage->prev;
    }
    fu = fu->next;
  }

  return TRUE;
}


/* decode an instruction */
const op_info_t *
decode_instr(int instr, int *use_imm) {
  const op_info_t *op_info;

  if (op_table[FIELD_OPCODE(instr)].sub_table == NULL) {
    op_info = &op_table[FIELD_OPCODE(instr)].info;
    *use_imm = 1;
  } else {
    op_info = &op_table[FIELD_OPCODE(instr)].sub_table[FIELD_FUNC(instr)].info;
    *use_imm = 0;
  }
  return op_info;
}


/* perform an instruction */
void 
perform_operation(int instr, unsigned long pc, operand_t operand1,
		  operand_t operand2)
{
  const op_info_t *op_info;
  int use_imm;
  operand_t result;

  op_info = decode_instr(instr, &use_imm);
  switch(op_info->fu_group_num) {
  case FU_GROUP_INT:
    switch(op_info->operation) {
    case OPERATION_ADD:
      result.integer.w = operand1.integer.w + operand2.integer.w;
      break;
	case OPERATION_ADDU:
		result.integer.wu = operand1.integer.wu + operand2.integer.wu;
		break;
	case OPERATION_SUB:
		result.integer.w = operand1.integer.w - operand2.integer.w;
		break;
	case OPERATION_SUBU:
		result.integer.wu = operand1.integer.wu - operand2.integer.wu;
		break;
	case OPERATION_MULT:
		result.integer.w = operand1.integer.w * operand2.integer.w;
		break;
	case OPERATION_DIV:
		result.integer.w = operand1.integer.w / operand2.integer.w;
		break;
	case OPERATION_SLL:
		result.integer.w = operand1.integer.w << operand2.integer.w;
		break;
	case OPERATION_SRL:
		result.integer.w = operand1.integer.w >> operand2.integer.w;
		break;
	case OPERATION_AND:
		result.integer.w = operand1.integer.w & operand2.integer.w;
		break;
	case OPERATION_OR:
		result.integer.w = operand1.integer.w | operand2.integer.w;
		break;
	case OPERATION_XOR:
		result.integer.w = operand1.integer.w ^ operand2.integer.w;
		break;
	case OPERATION_SLT:
		result.integer.w = operand1.integer.w < operand2.integer.w;
		break;
	case OPERATION_SGT:
		result.integer.w = operand1.integer.w > operand2.integer.w;
		break;
	case OPERATION_SLTU:
		result.integer.wu = operand1.integer.wu < operand2.integer.wu;
		break;
	case OPERATION_SGTU:
		result.integer.wu = operand1.integer.wu > operand2.integer.wu;
		break;
	 
		
    }
    break;

  case FU_GROUP_ADD:
	result.flt = operand1.flt + operand2.flt;
    	break;
  case FU_GROUP_MULT:
	result.flt = operand1.flt * operand2.flt;
	break;
  case FU_GROUP_DIV:
	result.flt = operand1.flt / operand2.flt;
	break;

  case FU_GROUP_MEM:
	result.integer.w = operand1.integer.w + operand2.integer.w;
	break;

  case FU_GROUP_BRANCH:
   	switch(op_info->operation){
		case OPERATION_J:
		case OPERATION_JAL:
			result.integer.w = pc + operand1.integer.w + 4;//operand1 is the offset value
			break;
		case OPERATION_JR:
		case OPERATION_JALR:
			result.integer.w = operand1.integer.w;
			break;
		case OPERATION_BEQZ:
		case OPERATION_BNEZ:
			result.integer.w = pc + operand1.integer.w + 4;//operand1 is the immediate value, need to check whether (r1)==0? or (r1)!=0
			break;
	}
	break;

  case FU_GROUP_HALT:
    break;

  case FU_GROUP_NONE:
    break;

  case FU_GROUP_INVALID:
    fprintf(stderr, "error: invalid opcode (instr = %.8X)\n", instr);
  }

}
