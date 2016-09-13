
/*
 * main.c
 * 
 * Donald Yeung
 */


#include <stdio.h>
#include <stdlib.h>
#include "fu.h"
#include "pipeline.h"
#include "output.h"

const char usage[] =
"usage: %s <options>\n"
"\toptions:\n"
"\t-b"    "\t<binary file>\n"
"\t-o"    "\t<functional unit options file>\n"
"\t-wbpi" "\t<# interger writeback ports>\n"
"\t-wbpf" "\t<# floating point writeback ports>\n";

void parse_args(int, char **);

static char *bin_file_name = NULL;
static char *fu_file_name = NULL;
static FILE *bin_file,*fu_file;
static int wbpi = 2;
static int wbpf = 2;


/************************************************************/
int
main(int argc, char *argv[]) {
  state_t *state;
  int data_count;
  int commit_ret, dispatch_ret;
  int num_insn, i;
  int halt=0;

  parse_args(argc, argv);
  state = state_create(&data_count, bin_file, fu_file, wbpi, wbpf);
 //freopen("output.txt", "w", stdout);
  if (state == NULL) {
    fclose(bin_file);
    fclose(fu_file);
    return -1;
  }

  fclose(bin_file);
  fclose(fu_file);

  /* main sim loop */
  for (i = 0, num_insn = 0;state->halt==0; i++) {

    printf("\n\n*** CYCLE %d\n", i);
    print_state(state, data_count);

    if( commit(state) ==1 ) 	
		num_insn++;				//printf("commit\n");
    writeback(state);			//printf("wb\n");
    execute(state);				//printf("ex\n");
    memory_disambiguation(state);	//printf("m\n");
    issue(state);					//printf("issue\n");
    if (!(state->fetch_lock)) {
      dispatch(state);
      fetch(state);
    }
  }

  printf("SIMULATION COMPLETE!\n");
  printf("COMMITTED %d INSTRUCTIONS IN %d CYCLES\n", num_insn, i-1);
  printf("CPI:  %.2f\n", (float)(i-1) / (float)num_insn);

  return 0;
}
/************************************************************/


void
parse_args(int argc, char **argv)
{
  int i;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i],"-b") == 0) {
      if (bin_file_name == NULL && i + 1 < argc) {
	bin_file_name = argv[++i];
      } else {
	fprintf(stderr,usage,argv[0]);
        exit(-1);
      }
    } else if (strcmp(argv[i],"-o") == 0) {
      if(fu_file_name == NULL && i + 1 < argc) {
	fu_file_name = argv[++i];
      } else {
	fprintf(stderr,usage,argv[0]);
        exit(-1);
      }
    } else if (strcmp(argv[i],"-wbpi") == 0) {
      if (wbpi == -1 && i + 1 < argc) {
	if (sscanf(argv[++i],"%d",&wbpi) == 1) {
	  if (wbpi <= 0) {
	    fprintf(stderr,usage,argv[0]);
	    exit(-1);
	  }
	} else {
	  fprintf(stderr,usage,argv[0]);
	  exit(-1);
	}
      } else {
	fprintf(stderr,usage,argv[0]);
        exit(-1);
      }
    } else if (strcmp(argv[i],"-wbpf") == 0) {
      if (wbpf == -1 && i + 1 < argc) {
	if (sscanf(argv[++i],"%d",&wbpf) == 1) {
	  if (wbpf <= 0) {
	    fprintf(stderr,usage,argv[0]);
	    exit(-1);
	  }
	} else {
	  fprintf(stderr,usage,argv[0]);
	  exit(-1);
	}
      } else {
	fprintf(stderr,usage,argv[0]);
        exit(-1);
      }
    } else {
      fprintf(stderr,usage,argv[0]);
      exit(-1);
    }
  }

  if (bin_file_name == NULL || fu_file_name == NULL || wbpi == -1 || wbpf == -1) {
    fprintf(stderr,usage,argv[0]);
    exit(-1);
  }

  bin_file = fopen(bin_file_name,"r");
  if (bin_file == NULL) {
    fprintf(stderr,"error: cannot open binary file '%s'\n",bin_file_name);
    exit(-1);
  }

  fu_file = fopen(fu_file_name,"r");
  if (fu_file == NULL) {
    fclose(bin_file);
    fprintf(stderr,"error: cannot open functional unit options file '%s'\n",fu_file_name);
    exit(-1);
  }
}
