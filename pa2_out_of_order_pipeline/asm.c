/*
 * 
 * asm.c
 *
   This assembler was written by Paul Kohout.
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define MAXLINELENGTH 1000
#define MAXNUMLABELS 10000
#define MAXLABELLENGTH 10

#define INSTR_MASK_REG    0x0000001F
#define INSTR_MASK_IMM    0x0000FFFF
#define INSTR_MASK_OFFSET 0x03FFFFFF

#define INSTR_SHIFT_OPCODE 26
#define INSTR_SHIFT_RF1    21
#define INSTR_SHIFT_RF2    16
#define INSTR_SHIFT_RF3    11
#define INSTR_SHIFT_FUNC   0
#define INSTR_SHIFT_IMM    0
#define INSTR_SHIFT_OFFSET 0

#define OP_TYPE_R     0
#define OP_TYPE_J     1
#define OP_TYPE_I     2
#define OP_TYPE_DATA  3

#define REG_TYPE_INT  0
#define REG_TYPE_FLT  1
#define REG_TYPE_DBL  2
#define REG_TYPE_NONE -1

#define VAL_TYPE_REQ  0
#define VAL_TYPE_NONE -1

#define DATA_TYPE_B    0
#define DATA_TYPE_H    1
#define DATA_TYPE_W    2
#define DATA_TYPE_F    3
#define DATA_TYPE_D    4
#define DATA_TYPE_NONE -1

#define VAL_RELATIVE_ZERO 0
#define VAL_RELATIVE_PC   1
#define VAL_RELATIVE_NONE -1

typedef struct _op_info_t {
  const char *name;
  const int op_type;
  const int opcode;
  const int func;
  const int position_rf1;
  const int reg_type_rf1;
  const int position_rf2;
  const int reg_type_rf2;
  const int position_rf3;
  const int reg_type_rf3;
  const int position_imm;
  const int val_type_imm;
  const int position_offset;
  const int val_type_offset;
  const int data_type;
  const int val_relative;
} op_info_t;

typedef union _data_t {
  char b;
  short h;
  long w;
  float f;
  double d;
} data_t;

const op_info_t op_table[] = {
  {"NOP",    OP_TYPE_R,   0, 0, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SLL",    OP_TYPE_R,   0, 4, 1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SRL",    OP_TYPE_R,   0, 6, 1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SRA",    OP_TYPE_R,   0, 7, 1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
//{"TRAP",   OP_TYPE_R,   0, 12
  {"SEQU",   OP_TYPE_R,   0, 16,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SNEU",   OP_TYPE_R,   0, 17,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SLTU",   OP_TYPE_R,   0, 18,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SGTU",   OP_TYPE_R,   0, 19,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SLEU",   OP_TYPE_R,   0, 20,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SGEU",   OP_TYPE_R,   0, 21,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"ADD",    OP_TYPE_R,   0, 32,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"ADDU",   OP_TYPE_R,   0, 33,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SUB",    OP_TYPE_R,   0, 34,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SUBU",   OP_TYPE_R,   0, 35,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"AND",    OP_TYPE_R,   0, 36,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"OR",     OP_TYPE_R,   0, 37,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"XOR",    OP_TYPE_R,   0, 38,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SEQ",    OP_TYPE_R,   0, 40,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SNE",    OP_TYPE_R,   0, 41,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SLT",    OP_TYPE_R,   0, 42,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SGT",    OP_TYPE_R,   0, 43,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SLE",    OP_TYPE_R,   0, 44,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SGE",    OP_TYPE_R,   0, 45,1,REG_TYPE_INT, 2,REG_TYPE_INT, 0,REG_TYPE_INT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
//{"MOVI2S", OP_TYPE_R,   0, 48
//{"MOVS2I", OP_TYPE_R,   0, 49
//{"MOVF",   OP_TYPE_R,   0, 50
//{"MOVD",   OP_TYPE_R,   0, 51
//{"MOVFP2I",OP_TYPE_R,   0, 52
//{"MOVI2FP",OP_TYPE_R,   0, 53
  {"ADD.S",   OP_TYPE_R,   1, 0, 1,REG_TYPE_FLT, 2,REG_TYPE_FLT, 0,REG_TYPE_FLT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SUB.S",   OP_TYPE_R,   1, 1, 1,REG_TYPE_FLT, 2,REG_TYPE_FLT, 0,REG_TYPE_FLT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"MULT.S",  OP_TYPE_R,   1, 2, 1,REG_TYPE_FLT, 2,REG_TYPE_FLT, 0,REG_TYPE_FLT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"DIV.S",   OP_TYPE_R,   1, 3, 1,REG_TYPE_FLT, 2,REG_TYPE_FLT, 0,REG_TYPE_FLT, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"ADD.D",   OP_TYPE_R,   1, 4, 1,REG_TYPE_DBL, 2,REG_TYPE_DBL, 0,REG_TYPE_DBL, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SUB.D",   OP_TYPE_R,   1, 5, 1,REG_TYPE_DBL, 2,REG_TYPE_DBL, 0,REG_TYPE_DBL, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"MULT.D",  OP_TYPE_R,   1, 6, 1,REG_TYPE_DBL, 2,REG_TYPE_DBL, 0,REG_TYPE_DBL, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"DIV.D",   OP_TYPE_R,   1, 7, 1,REG_TYPE_DBL, 2,REG_TYPE_DBL, 0,REG_TYPE_DBL, 0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
//{"CVTF2D", OP_TYPE_R,   1, 8
//{"CVTF2I", OP_TYPE_R,   1, 9
//{"CVTD2F", OP_TYPE_R,   1, 10
//{"CVTD2I", OP_TYPE_R,   1, 11
//{"CVTI2F", OP_TYPE_R,   1, 12
//{"CVTI2D", OP_TYPE_R,   1, 13
//{"MULT",   OP_TYPE_R,   1, 14
//{"DIV",    OP_TYPE_R,   1, 15
  {"EQF",    OP_TYPE_R,   1, 16,0,REG_TYPE_FLT, 1,REG_TYPE_FLT, 0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"NEF",    OP_TYPE_R,   1, 17,0,REG_TYPE_FLT, 1,REG_TYPE_FLT, 0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"LTF",    OP_TYPE_R,   1, 18,0,REG_TYPE_FLT, 1,REG_TYPE_FLT, 0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"GTF",    OP_TYPE_R,   1, 19,0,REG_TYPE_FLT, 1,REG_TYPE_FLT, 0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"LEF",    OP_TYPE_R,   1, 20,0,REG_TYPE_FLT, 1,REG_TYPE_FLT, 0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"GEF",    OP_TYPE_R,   1, 21,0,REG_TYPE_FLT, 1,REG_TYPE_FLT, 0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
//{"MULTU",  OP_TYPE_R,   1, 22
//{"DIVU",   OP_TYPE_R,   1, 23
  {"EQD",    OP_TYPE_R,   1, 24,0,REG_TYPE_DBL, 1,REG_TYPE_DBL, 0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"NED",    OP_TYPE_R,   1, 25,0,REG_TYPE_DBL, 1,REG_TYPE_DBL, 0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"LTD",    OP_TYPE_R,   1, 26,0,REG_TYPE_DBL, 1,REG_TYPE_DBL, 0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"GTD",    OP_TYPE_R,   1, 27,0,REG_TYPE_DBL, 1,REG_TYPE_DBL, 0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"LED",    OP_TYPE_R,   1, 28,0,REG_TYPE_DBL, 1,REG_TYPE_DBL, 0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"GED",    OP_TYPE_R,   1, 29,0,REG_TYPE_DBL, 1,REG_TYPE_DBL, 0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"J",      OP_TYPE_J,   2, 0, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_REQ, DATA_TYPE_NONE,VAL_RELATIVE_PC},
  {"JAL",    OP_TYPE_J,   3, 0, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_REQ, DATA_TYPE_NONE,VAL_RELATIVE_PC},
  {"BEQZ",   OP_TYPE_I,   4, 0, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,1,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_PC},
  {"BNEZ",   OP_TYPE_I,   5, 0, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,1,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_PC},
  {"BFPT",   OP_TYPE_I,   6, 0, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_PC},
  {"BFPF",   OP_TYPE_I,   7, 0, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_PC},
  {"ADDI",   OP_TYPE_I,   8, 0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"ADDUI",  OP_TYPE_I,   9, 0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SUBI",   OP_TYPE_I,   10,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SUBUI",  OP_TYPE_I,   11,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"ANDI",   OP_TYPE_I,   12,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"ORI",    OP_TYPE_I,   13,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"XORI",   OP_TYPE_I,   14,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
//{"LHI",    OP_TYPE_I,   15,0
//{"RFE",    OP_TYPE_I,   16,0
//{"TRAP",   OP_TYPE_I,   17,0
  {"JR",     OP_TYPE_I,   18,0, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"JALR",   OP_TYPE_I,   19,0, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {"SLLI",   OP_TYPE_I,   20,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SRLI",   OP_TYPE_I,   22,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SRAI",   OP_TYPE_I,   23,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SEQI",   OP_TYPE_I,   24,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SNEI",   OP_TYPE_I,   25,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SLTI",   OP_TYPE_I,   26,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SGTI",   OP_TYPE_I,   27,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SLEI",   OP_TYPE_I,   28,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SGEI",   OP_TYPE_I,   29,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"LB",     OP_TYPE_I,   32,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"LH",     OP_TYPE_I,   33,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"LW",     OP_TYPE_I,   35,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"LBU",    OP_TYPE_I,   36,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"LHU",    OP_TYPE_I,   37,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"L.S",     OP_TYPE_I,   38,0, 1,REG_TYPE_INT, 0,REG_TYPE_FLT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"L.D",     OP_TYPE_I,   39,0, 1,REG_TYPE_INT, 0,REG_TYPE_DBL, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SB",     OP_TYPE_I,   40,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SH",     OP_TYPE_I,   41,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SW",     OP_TYPE_I,   43,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"S.S",     OP_TYPE_I,   46,0, 1,REG_TYPE_INT, 0,REG_TYPE_FLT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"S.D",     OP_TYPE_I,   47,0, 1,REG_TYPE_INT, 0,REG_TYPE_DBL, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SEQUI",  OP_TYPE_I,   48,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SNEUI",  OP_TYPE_I,   49,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SLTUI",  OP_TYPE_I,   50,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SGTUI",  OP_TYPE_I,   51,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SLEUI",  OP_TYPE_I,   52,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"SGEUI",  OP_TYPE_I,   53,0, 1,REG_TYPE_INT, 0,REG_TYPE_INT, 0,REG_TYPE_NONE,2,VAL_TYPE_REQ, 0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_ZERO},
  {"HALT",   OP_TYPE_I,   63,0, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_NONE,VAL_RELATIVE_NONE},
  {".DB",    OP_TYPE_DATA,0, 0, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_B,   VAL_RELATIVE_NONE},
  {".DH",    OP_TYPE_DATA,0, 0, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_H,   VAL_RELATIVE_NONE},
  {".DW",    OP_TYPE_DATA,0, 0, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_W,   VAL_RELATIVE_NONE},
  {".DF",    OP_TYPE_DATA,0, 0, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_F,   VAL_RELATIVE_NONE},
  {".DD",    OP_TYPE_DATA,0, 0, 0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,REG_TYPE_NONE,0,VAL_TYPE_NONE,0,VAL_TYPE_NONE,DATA_TYPE_D,   VAL_RELATIVE_NONE}
};

char * readAndParse(FILE *, char **, char **, char **, char **, char **);
int get_label_address(char *);

char Labels[MAXNUMLABELS][MAXLABELLENGTH];
int Addresses[MAXNUMLABELS];
int NumValidLabels=0;

int main(int argc, char *argv[]) {
  FILE *inFilePtr, *outFilePtr;
  char *label;
  char *opcode;
  char *instr_arg[3];
  int instr_rf1;
  int instr_rf2;
  int instr_rf3;
  int instr_imm;
  int instr_offset;
  data_t instr_data;
  int instr_data_size;
  int instr;
  int address;
  int line;
  int i;

  if (argc != 3) {
    fprintf(stderr,"error: usage: %s <assembly-code-file> <machine-code-file> \n",argv[0]);
    exit(1);
  }

  inFilePtr = fopen(argv[1], "r");
  if (inFilePtr == NULL) {
    fprintf(stderr,"error in opening %s\n",argv[1]);
    exit(1);
  }
  outFilePtr = fopen(argv[2], "w");
  if (outFilePtr == NULL) {
    fprintf(stderr,"error in opening %s\n",argv[2]);
    exit(1);
  }

  /* PASS ONE -- map symbols to addresses */

  /* assume address start at 0 */
  address = 0;
  line = 1;

  while(readAndParse(inFilePtr, &label, &opcode, &instr_arg[0], &instr_arg[1], &instr_arg[2]) != NULL) {

    if (label != NULL) {
      /* look for duplicate label */
      if ((i = get_label_address(label)) < 0) {
	/* label not found -- a good sign */
	/* but first -- make sure we don't overrun buffer */
	if (NumValidLabels >= MAXNUMLABELS) {
	  /* we will exceed the size of the array */
	  fprintf(stderr,"error (line %d): too many labels (label=%s)\n",line,label);
	  exit(1);
	}
	strcpy(Labels[NumValidLabels], label);
	Addresses[NumValidLabels] = address;
	NumValidLabels++;
      } else {
	/* duplicate label -- terminate */
	fprintf(stderr,"error (line %d): duplicate label %s \n",line,label);
	exit(1);
      }
    }

    for(i = 0; i < sizeof(op_table)/sizeof(op_info_t); i ++) {
      if(strcasecmp(opcode,op_table[i].name) == 0)
	break;
    }

    if(i == sizeof(op_table)/sizeof(op_info_t)) {
      fprintf(stderr,"error (line %d): invalid opcode %s\n",line,opcode);
      exit(1);
    }

    if(get_data(op_table[i].data_type,instr_arg[0],NULL,&instr_data_size,line) != 0) {
      exit(1);
    }

    switch(op_table[i].op_type) {
    case OP_TYPE_R:
    case OP_TYPE_J:
    case OP_TYPE_I:
      address += 4;
      break;
    case OP_TYPE_DATA:
      address += instr_data_size;
      break;
    }

    line++;
  }

  //printf("debug 1\n");

  /* PASS TWO -- print machine code, with symbols filled in as addresses */

  rewind(inFilePtr);
  address = 0;
  line = 1;

  while(readAndParse(inFilePtr, &label, &opcode, &instr_arg[0], &instr_arg[1], &instr_arg[2]) != NULL) {

    for(i = 0; i < sizeof(op_table)/sizeof(op_info_t); i ++) {
      if(strcasecmp(opcode,op_table[i].name) == 0)
	break;
    }

    if(i == sizeof(op_table)/sizeof(op_info_t)) {
      fprintf(stderr,"error (line %d): invalid opcode %s\n",line,opcode);
      exit(1);
    }

    if((get_reg(op_table[i].reg_type_rf1,instr_arg[op_table[i].position_rf1],&instr_rf1,INSTR_MASK_REG,line) != 0) ||
       (get_reg(op_table[i].reg_type_rf2,instr_arg[op_table[i].position_rf2],&instr_rf2,INSTR_MASK_REG,line) != 0) ||
       (get_reg(op_table[i].reg_type_rf3,instr_arg[op_table[i].position_rf3],&instr_rf3,INSTR_MASK_REG,line) != 0) ||
       (get_val(op_table[i].val_type_imm,instr_arg[op_table[i].position_imm],&instr_imm,INSTR_MASK_IMM,op_table[i].val_relative,address,line) != 0) ||
       (get_val(op_table[i].val_type_offset,instr_arg[op_table[i].position_offset],&instr_offset,INSTR_MASK_OFFSET,op_table[i].val_relative,address,line) != 0) ||
       (get_data(op_table[i].data_type,instr_arg[0],&instr_data,&instr_data_size,line) != 0)) {
      exit(1);
    }

    switch(op_table[i].op_type) {
    case OP_TYPE_R:
      instr = (op_table[i].opcode << INSTR_SHIFT_OPCODE) |
 	      (instr_rf1 << INSTR_SHIFT_RF1) |
	      (instr_rf2 << INSTR_SHIFT_RF2) |
	      (instr_rf3 << INSTR_SHIFT_RF3) |
	      (op_table[i].func << INSTR_SHIFT_FUNC);
      fwrite(&instr,4,1,outFilePtr);
      address += 4;
      break;
    case OP_TYPE_J:
      instr_offset = instr_offset & INSTR_MASK_OFFSET;
      instr = (op_table[i].opcode << INSTR_SHIFT_OPCODE) |
	      (instr_offset << INSTR_SHIFT_OFFSET);
      fwrite(&instr,4,1,outFilePtr);
      address += 4;
      break;
    case OP_TYPE_I:
      instr = (op_table[i].opcode << INSTR_SHIFT_OPCODE) |
	      (instr_rf1 << INSTR_SHIFT_RF1) |
	      (instr_rf2 << INSTR_SHIFT_RF2) |
	      (instr_imm << INSTR_SHIFT_IMM);
      fwrite(&instr,4,1,outFilePtr);
      address += 4;
      break;
    case OP_TYPE_DATA:
      fwrite(&instr_data,instr_data_size,1,outFilePtr);
      address += instr_data_size;
      break;
    }

    line++;
  }
  return 0;
}

char *readAndParse(FILE *inFilePtr, char **labelPtr,
		   char **opcodePtr, char **arg0Ptr, char **arg1Ptr,
		   char **arg2Ptr) {
  char lineString[MAXLINELENGTH+1];
  char *statusString;

  statusString = fgets(lineString, MAXLINELENGTH, inFilePtr);
  if (statusString != NULL) {
    if (lineString[0] == '\t') {
      *labelPtr = NULL;
      *opcodePtr = strtok(lineString, "\t\n");
    } else {
      *labelPtr = strtok(lineString, "\t\n");
      *opcodePtr = strtok(NULL, "\t\n");
    }
    *arg0Ptr = strtok(NULL, "\t\n");
    *arg1Ptr = strtok(NULL, "\t\n");
    *arg2Ptr = strtok(NULL, "\t\n");
  }
  return(statusString);
}

int get_label_address(char *s) {
  int i;

  for (i=0; i<NumValidLabels; i++) {
    if (strlen(Labels[i]) == 0) {
      return -1;
    }
    if (strcmp(Labels[i], s) == 0) {
      return Addresses[i];
    }
  }
  return -1;
}

int get_reg(int reg_type,char *instr_arg,int *instr_reg,int instr_mask_reg,int line) {
  int reg;

  switch(reg_type) {
  case REG_TYPE_INT:
    if(sscanf(instr_arg,"R%d",&reg) != 1) {
      fprintf(stderr,"error (line %d): invalid register field %s\n",line,instr_arg);
      return -1;
    }
    if((reg & ~INSTR_MASK_REG) != 0) {
      fprintf(stderr,"error (line %d): register field R%d out of bounds\n",line,reg);
      return -1;
    }
    break;
  case REG_TYPE_FLT:
    if(sscanf(instr_arg,"F%d",&reg) != 1) {
      fprintf(stderr,"error (line %d): invalid register field %s\n",line,instr_arg);
      return -1;
    }
    if((reg & ~INSTR_MASK_REG) != 0) {
      fprintf(stderr,"error (line %d): register field F%d out of bounds\n",line,reg);
      return -1;
    }
    break;
  case REG_TYPE_DBL:
    if(sscanf(instr_arg,"F%d",&reg) != 1) {
      fprintf(stderr,"error (line %d): invalid register field %s\n",line,instr_arg);
      return -1;
    }
    if((reg & ~INSTR_MASK_REG) != 0) {
      fprintf(stderr,"error (line %d): register field F%d out of bounds\n",line,reg);
      return -1;
    }
    if((reg & 1) != 0) {
      fprintf(stderr,"error (line %d): register field F%d must be even\n",line,reg);
      return -1;
    }
    break;
  case REG_TYPE_NONE:
    reg = 0;
    break;
  }
  if(instr_reg != NULL)
    *instr_reg = reg;
  return 0;
}

int get_val(int val_type,char *instr_arg,int *instr_val,int instr_mask_val,int val_relative,int address,int line) {
  int val;

  switch(val_type) {
  case VAL_TYPE_REQ:
    if(sscanf(instr_arg,"#%d",&val) != 1) {
      val = get_label_address(instr_arg);
      if(val < 0) {
	fprintf(stderr,"error (line %d): invalid value %s\n",line,instr_arg);
	return -1;
      }
    }
    if(((val < 0) && ((val | instr_mask_val) != -1)) ||
       ((val >= 0) && ((val & ~instr_mask_val) != 0))) {
      fprintf(stderr,"error (line %d): value #%d out of bounds\n",line,val);
      return -1;
    }
    switch(val_relative) {
    case VAL_RELATIVE_ZERO:
      break;
    case VAL_RELATIVE_PC:
      val -= address + 4;
      break;
    }
    break;
  case VAL_TYPE_NONE:
    val = 0;
    break;
  }
  if(instr_val != NULL)
    *instr_val = val & instr_mask_val;
  return 0;
}

int get_data(int data_type,char *instr_arg,data_t *instr_data,int *instr_data_size,int line) {
  int data_int;
  float data_flt;
  double data_dbl;

  switch(data_type) {
  case DATA_TYPE_B:
    if(sscanf(instr_arg,"%d",&data_int) != 1) {
      fprintf(stderr,"error (line %d): invalid value %s\n",line,instr_arg);
      return -1;
    }
    if(((data_int < 0) && ((data_int | 0xFF) != -1)) ||
       ((data_int >= 0) && ((data_int & ~0xFF) != 0))) {
      fprintf(stderr,"error (line %d): value %d out of bounds\n",line,data_int);
      return -1;
    }
    if(instr_data != NULL)
      instr_data->b = data_int & 0xFF;
    if(instr_data_size != NULL)
      *instr_data_size = 1;
    break;
  case DATA_TYPE_H:
    if(sscanf(instr_arg,"%d",&data_int) != 1) {
      fprintf(stderr,"error (line %d): invalid value %s\n",line,instr_arg);
      return -1;
    }
    if(((data_int < 0) && ((data_int | 0xFFFF) != -1)) ||
       ((data_int >= 0) && ((data_int & ~0xFFFF) != 0))) {
      fprintf(stderr,"error (line %d): value %d out of bounds\n",line,data_int);
      return -1;
    }
    if(instr_data != NULL)
      instr_data->h = data_int & 0xFFFF;
    if(instr_data_size != NULL)
      *instr_data_size = 2;
    break;
  case DATA_TYPE_W:
    if(sscanf(instr_arg,"%d",&data_int) != 1) {
      data_int = get_label_address(instr_arg);
      if(data_int < 0) {
	fprintf(stderr,"error (line %d): invalid value %s\n",line,instr_arg);
	return -1;
      }
    }
    if(instr_data != NULL)
      instr_data->w = data_int;
    if(instr_data_size != NULL)
      *instr_data_size = 4;
    break;
  case DATA_TYPE_F:
    if(sscanf(instr_arg,"%f",&data_flt) != 1) {
      fprintf(stderr,"error (line %d): invalid value %s\n",line,instr_arg);
      return -1;
    }
    if(instr_data != NULL)
      instr_data->f = data_flt;
    if(instr_data_size != NULL)
      *instr_data_size = 4;
    break;
  case DATA_TYPE_D:
    if(sscanf(instr_arg,"%lf",&data_dbl) != 1) {
      fprintf(stderr,"error (line %d): invalid value %s\n",line,instr_arg);
      return -1;
    }
    if(instr_data != NULL)
      instr_data->d = data_dbl;
    if(instr_data_size != NULL)
      *instr_data_size = 8;
    break;
  }
  return 0;
}
