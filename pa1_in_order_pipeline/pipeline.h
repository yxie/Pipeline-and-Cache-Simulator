
/*
 * 
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

/* decode/execution pipeline register */
/*
typedef struct _id_ix_t {
  int instr;
  unsigned long pc;
  operand_t A;
  operand_t B;
} id_ix_t;
*/

/* Register state */
typedef struct _rf_int_t {
  int_t reg_int[NUMREGS];
} rf_int_t;

typedef struct _rf_fp_t {
  float reg_fp[NUMREGS];
} rf_fp_t;


/* Overall processor state */
typedef struct _state_t {
  /* memory */
  unsigned char mem[MAXMEMORY];

  /* register files */
  rf_int_t rf_int;
  rf_fp_t rf_fp;
  rf_fp_t rf_fp_temp;

  /* pipeline registers */
  unsigned long pc;
  if_id_t if_id;


  fu_int_t *fu_int_list;
  fu_fp_t *fu_add_list;
  fu_fp_t *fu_mult_list;
  fu_fp_t *fu_div_list;

  wb_t int_wb;
  wb_t fp_wb;

  int fetch_lock;
  
  int stall;
  int halt;
  unsigned long pc_b;
  
} state_t;


extern state_t *state_create(int *, FILE *, FILE *);

extern void writeback(state_t *, int *);
extern void execute(state_t *);
extern int decode(state_t *);
extern void fetch(state_t *);

//int mem_char_to_int(char *addr, unsigned long offset)
//void mem_int_to_char(char *addr, unsigned long offset)
