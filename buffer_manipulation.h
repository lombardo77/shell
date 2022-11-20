#ifndef BUFFER_MANIPULATIONS
#define BUFFER_MANIPULATIONS
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

char** gen_argv(char* buf);
void print_array(char** arr);
char* rm_ws(char* str);
char* rm_wsb(char* s, char a);
void freeargv(char** argv);

#endif
