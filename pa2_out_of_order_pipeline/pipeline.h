
/*
 * pipeline.h
 * 
 * Donald Yeung
 */


#define TRUE 1
#define FALSE 0


/* fetch/decode pipeline register */
typedef struct _if_id_t {
  int instr;
  unsigned long pc;
} if_id_t;


/* Register state */
typedef struct _reg_int_t {
  int_t integer[NUMREGS];
} reg_int_t;

typedef struct _rf_int_t {
  reg_int_t reg_int;
  int tag[NUMREGS];
} rf_int_t;

typedef struct _reg_fp_t {
  float flt[NUMREGS];
} reg_fp_t;

typedef struct _rf_fp_t {
  reg_fp_t reg_fp;
  int tag[NUMREGS];
} rf_fp_t;


/* Queue structures */
#define IQ_SIZE 32
typedef struct _IQ_t {
  int instr;
  unsigned long pc;
  int issued;
  int ROB_index;
  int tag1;
  operand_t operand1;
  int tag2;
  operand_t operand2;
} IQ_t;

#define CQ_SIZE 32
typedef struct _CQ_t {
  int instr;
  int store;
  int issued;
  int ROB_index;
  int tag1;
  operand_t address;
  int tag2;
  operand_t result;
} CQ_t;

#define ROB_SIZE 64
typedef struct _ROB_t {
  int instr;
  int completed;
  operand_t target;
  operand_t result;
} ROB_t;


/* Overall processor state */
typedef struct _state_t {
  /* memory */
  unsigned char mem[MAXMEMORY];

  /* register files */
  rf_int_t rf_int;
  rf_fp_t rf_fp;

  /* pipeline registers */
  unsigned long pc;
  if_id_t if_id;

  IQ_t *IQ;
  int IQ_head, IQ_tail;
  CQ_t *CQ;
  int CQ_head, CQ_tail;
  ROB_t *ROB;
  int ROB_head, ROB_tail;

  fu_int_t *fu_int_list;
  fu_mem_t *fu_mem_list;
  fu_fp_t *fu_add_list;
  fu_fp_t *fu_mult_list;
  fu_fp_t *fu_div_list;
  wb_port_int_t *wb_port_int;
  int wb_port_int_num;
  wb_port_fp_t *wb_port_fp;
  int wb_port_fp_num;
  int branch_tag;

  int fetch_lock;
  
  int halt;
  int full;
} state_t;

extern state_t *state_create(int *,FILE *,FILE *,int,int);

extern int commit(state_t *);
extern void writeback(state_t *);
extern void execute(state_t *);
extern int memory_disambiguation(state_t *);
extern int issue(state_t *);
extern int dispatch(state_t *);
extern void fetch(state_t *);
