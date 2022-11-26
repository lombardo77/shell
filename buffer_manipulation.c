#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#define SIZE 100

int words = 0;

//removes ws from string
char* rm_ws(char *str)
{
    int nsc = 0;
 
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] != ' ') {
            str[nsc] = str[i];
            nsc++;        
        }
    }
    
    str[nsc] = '\0';
    return str;
}


//removes white space in beg of string
char* rm_wsb(char* s, char a) 
{
    char* d = s;
    while (*d == a)
        ++d;
    return d;
}

// free the allcated copies in argv
void freeargv(char** argv)
{
    int i = 0;
    while (argv[i] != NULL) {   
        free(argv[i]);
        i++;
    }
    free(argv[i]);
}


//prints array
void print_array(char** arr)
{
    for (int i = 0; arr[i] != NULL ;i ++)
        printf("%s, ", arr[i]);
}

// this function creates argv from buf.
char** gen_argv(char* buf) 
{
    // spaces holds the number of spaces in buf
    int spaces = 0; 
    
    // copy buf to avoid issues with parent process
    char* buf_cpy0 = malloc(SIZE);
    char* buf_cpy;
    strcpy(buf_cpy0, buf);
    buf_cpy = rm_wsb(buf_cpy0, ' ');
    
    // allocate argv
    char** argv = malloc(SIZE);

    // places buf_cpy into argv[0] 
    argv[0] = buf_cpy;

    for (int i = 0; i < SIZE; i ++) {

        /*places null terminator at last space char, counts spaces 
        and adds pointer to first letter in each term to argv*/
        if (buf_cpy[i] == ' ' 
                && buf_cpy[i + 1] != '\0'
                && buf_cpy[i + 1] != ' ') {
            spaces ++;
            argv[spaces] = &buf_cpy[i + 1];
            buf_cpy[i] = '\0';

        }
    }

    /*replace elements of argv with copies, copies are malloced. 
     To free() call free_argv*/
    for (int i =0; i < spaces + 1 ; i++) {
        char* tmp = rm_ws(argv[i]);
        char* token_cpy = (char*)malloc(strlen(tmp) + 1);
        strcpy(token_cpy, tmp);
        argv[i] = token_cpy;
    }

    //finally add NULL at the end of argv
    argv[spaces + 1] = NULL;
    words = spaces + 1;

    //free buf_cpy0
    free(buf_cpy0);
    
    return argv;
}


