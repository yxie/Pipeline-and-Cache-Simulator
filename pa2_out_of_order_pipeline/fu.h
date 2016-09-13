
/*
 * fu.h

   This module was originally written by Paul Kohout and adapted for
   this simulator.

 * 
 * Donald Yeung
 */


#include <stdio.h>

#define MAXMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 32 /* number of machine registers */

#define FIELD_OPCODE(instr) ((unsigned int)(((instr)>>26)&0x003F))
#define FIELD_FUNC(instr)   ((unsigned int)((instr)&0x07FF))
#define FIELD_R1(instr)     ((unsigned int)(((instr)>>21)&0x001F))
#define FIELD_R2(instr)     ((unsigned int)(((instr)>>16)&0x001F))
#define FIELD_R3(instr)     ((unsigned int)(((instr)>>11)&0x001F))
#define FIELD_IMM(instr)    ((signed short)((instr)&0xFFFF))
#define FIELD_IMMU(instr)   ((unsigned short)((instr)&0xFFFF))
#define FIELD_OFFSET(instr) ((signed int)(((instr)&0x02000000)?((instr)|0xFC000000):((instr)&0x03FFFFFF)))

#define FU_GROUP_INT     0
#define FU_GROUP_ADD     1
#define FU_GROUP_MULT    2
#define FU_GROUP_DIV     3
#define FU_GROUP_MEM     4
#define FU_GROUP_BRANCH  5
#define FU_GROUP_NONE    -1
#define FU_GROUP_INVALID -2
#define FU_GROUP_HALT    -3

#define OPERATION_ADD   0
#define OPERATION_ADDU  1
#define OPERATION_SUB   2
#define OPERATION_SUBU  3
#define OPERATION_MULT  4
#define OPERATION_DIV   5
#define OPERATION_SLL   6
#define OPERATION_SRL   7
#define OPERATION_AND   8
#define OPERATION_OR    9
#define OPERATION_XOR   10
#define OPERATION_LOAD  11
#define OPERATION_STORE 12
#define OPERATION_J     13
#define OPERATION_JAL   14
#define OPERATION_JR    15
#define OPERATION_JALR  16
#define OPERATION_BEQZ  17
#define OPERATION_BNEZ  18
#define OPERATION_SLT   19
#define OPERATION_SGT   20
#define OPERATION_SLTU  21
#define OPERATION_SGTU  22
#define OPERATION_NONE  -1

#define DATA_TYPE_W    1
#define DATA_TYPE_F    2
#define DATA_TYPE_NONE -1

typedef struct _op_info_t {
  const char *name;
  const int fu_group_num;
  const int operation;
  const int data_type;
} op_info_t;

typedef struct _op_t {
  const struct _op_info_t info;
  const struct _sub_op_t * const sub_table;
} op_t;

typedef struct _sub_op_t {
  const struct _op_info_t info;
} sub_op_t;

extern const op_t op_table[];
extern const sub_op_t op_special_table[];
extern const sub_op_t op_fparith_table[];

extern const char fu_group_int_name[];
extern const char fu_group_mem_name[];
extern const char fu_group_add_name[];
extern const char fu_group_mult_name[];
extern const char fu_group_div_name[];


/* union to handle multiple fixed-point types */
typedef union _int_t {
  signed long w;
  unsigned long wu;
} int_t;

typedef union _operand_t {
  int_t integer;
  float flt;
} operand_t;


/* functional unit and memory port structures */
typedef struct _fu_int_stage_t {
  int num_cycles;
  int current_cycle;
  int tag;
  int branch;
  int link;
  struct _fu_int_stage_t *prev;
} fu_int_stage_t;

typedef struct _fu_int_t {
  char *name;
  fu_int_stage_t *stage_list;
  struct _fu_int_t *next;
} fu_int_t;


typedef struct _fu_mem_stage_t {
  int num_cycles;
  int current_cycle;
  int tag;
  int store;
  int float_mem;
  struct _fu_mem_stage_t *prev;
} fu_mem_stage_t;

typedef struct _fu_mem_t {
  char *name;
  fu_mem_stage_t *stage_list;
  struct _fu_mem_t *next;
} fu_mem_t;


typedef struct _fu_fp_stage_t {
  int num_cycles;
  int current_cycle;
  int tag;
  struct _fu_fp_stage_t *prev;
} fu_fp_stage_t;

typedef struct _fu_fp_t {
  char *name;
  fu_fp_stage_t *stage_list;
  struct _fu_fp_t *next;
} fu_fp_t;


/* writeback pipeline register */
typedef struct _wb_port_int_t {
  int tag;
} wb_port_int_t;

typedef struct _wb_port_fp_t {
  int tag;
} wb_port_fp_t;


extern int fu_int_read(fu_int_t **, FILE *);
extern int fu_mem_read(fu_mem_t **, FILE *);
extern int fu_fp_read(fu_fp_t **, FILE *);

extern int issue_fu_int(fu_int_t *, int, int, int);
extern int issue_fu_mem(fu_mem_t *, int, int, int);
extern int issue_fu_fp(fu_fp_t *,int);

extern void advance_fu_int(fu_int_t *, wb_port_int_t [], int, int *);
extern void advance_fu_mem(fu_mem_t *, wb_port_int_t [], int, wb_port_fp_t [], int);
extern void advance_fu_fp(fu_fp_t *, wb_port_fp_t [],int);

extern int wb_int(wb_port_int_t [], int, int);
extern int wb_fp(wb_port_fp_t [], int, int);

const op_info_t *decode_instr(int, int *);
