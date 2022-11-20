#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#define SIZE 256

char** gen_argv(char* buf);
void print_array(char** arr);
char* rm_ws(char* str);
char* rm_wsb(char* s, char a);
void freeargv(char** argv);


//to do
// 1. make sure i can free() malloced blocks
// 2. the other shit!
int main(){
    char* buf = (char*)malloc(SIZE);
    char* pth_buf = (char*)malloc(SIZE);

    int off = 0;
    //shell loop
    while(1)
    {
        //command line prompt "> "
        printf("> ");
        off = scanf("%[^\n]%*c", buf);
        
        // pointer to argument array
        char **argv;

        // generate argument array with raw command line input
        argv = gen_argv(buf);

        // this is cd
        if(strcmp(argv[0], "cd") == 0){
            chdir(argv[1]);
        } 
        
        // run program from command line
        else{
            //first create child process
            pid_t pid = fork();

            // child process
            if (pid == 0){              
                  
                //first check bin/*
                snprintf(pth_buf, SIZE, "/bin/%s", argv[0]);
                execv(pth_buf, argv);

                //then check current directory
                if (errno == 2){
                    snprintf(pth_buf, SIZE, "./%s", argv[0]);
                    execv(pth_buf, argv);
                } 
                // finally check given path
                if (errno == 2){
                    execv(argv[0], argv);
                }

               //if nothing works, then print error 
                if (errno == 2){
                    perror("command not found");
                }
                free(buf);
                free(pth_buf);
                freeargv(argv);
                free(argv);
                exit(127);
            } 
            //parent process
            else{
                freeargv(argv);  
                free(argv);
                waitpid(pid, 0, 0);
            }
        }
        if (off != 1)
            break;
    }
    
    free(buf);
    free(pth_buf);

    return 0;
}

//prints array
void print_array(char** arr){
    for (int i = 0; arr[i] != NULL ;i ++)
        printf("%s, ", arr[i]);
}

// this function creates argv from buf.
char** gen_argv(char* buf){
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

    for (int i = 0; i < SIZE; i ++)
    {

        /*places null terminator at last space char, counts spaces 
        and adds pointer to first letter in each term to argv*/
        if (buf_cpy[i] == ' ' 
                && buf_cpy[i + 1] != '\0'
                && buf_cpy[i + 1] != ' ') 
        {
            spaces ++;
            argv[spaces] = &buf_cpy[i + 1];
            buf_cpy[i] = '\0';

        }
    }

    /*replace elements of argv with copies, copies are malloced. 
     To free() call free_argv*/
    for(int i =0; i < spaces + 1 ; i++)
    {
        char* tmp = rm_ws(argv[i]);
        char* token_cpy = (char*)malloc(strlen(tmp) + 1);
        strcpy(token_cpy, tmp);
        argv[i] = token_cpy;
    }

    //finally add NULL at the end of argv
    argv[spaces + 1] = NULL;

    //free buf_cpy0
    free(buf_cpy0);
    
    return argv;
}

//removes ws from string
char* rm_ws(char *str)
{
    int nsc = 0;
 
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] != ' ')
        {
            str[nsc] = str[i];
            nsc++;        }    
    }
    
    str[nsc] = '\0';
    return str;
}


//removes white space in beg of string
char* rm_wsb(char* s, char a) {
    char* d = s;
    while (*d == a)
        ++d;
    return d;
}

// free the allcated copies in argv
void freeargv(char** argv){
    int i = 0;
    while(argv[i] != NULL)
    {   free(argv[i]);
        i++;
    }
    free(argv[i]);

}





