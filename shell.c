#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include "buffer_manipulation.h"
#include <signal.h>

#define SIZE 256


void sigint_handler(int signo);

struct pid_

//to do
// 1. make sure i can free() malloced blocks
// 2. the other shit!
int main(){
    char* buf = (char*)malloc(SIZE);
    char* pth_buf = (char*)malloc(SIZE);

    //signals
    signal(SIGINT, sigint_handler);

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

//gets pid from job list
pid_t get_pid(void){
    
}

// SIGINT handler
void sigint_handler(int signo) {
    printf("Caught SIGINT\n");
    pid_t pid = get_pid();
    kill(SIGINT, pid);
}


