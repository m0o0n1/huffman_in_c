#include "error.h"



void print_error(char *emsg){
	printf(ANSI_COLOR_RED "[ERROR}: %s" ANSI_COLOR_RESET, emsg); 
}

void print_success(char *smsg){
	printf(ANSI_COLOR_GREEN "[INFO]: %s" ANSI_COLOR_RESET, smsg);
}
