#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#include "buffer_manipulation.h"
#include "joblists.h"

#define SIZE 256
#define ARG argv[0]

void sigint_handler(int signo);
void sigint_handle_child(int signo);

void sigtstp_handler(int signo);
void sigtstp_handle_child(int signo);

//to do
// 1. make sure i can free() malloced blocks
// 2. the other shit!
int main(){
    char* buf = (char*)malloc(SIZE);
    char* pth_buf = (char*)malloc(SIZE);
    int bg = 0;

    // numbe of words in argv
    extern int words;

    //signals
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);

    int off = 0;
    //shell loop
    while(1)
    {
        //command line prompt "> "
        printf("> ");
        fflush(stdout);
        
        off = scanf("%[^\n]%*c", buf);
        
        // pointer to argument array
        char **argv;

        // generate argument array with raw command line input
        argv = gen_argv(buf);

        if(words > 1 && !strcmp(argv[1], "&"))
            bg = 1;
        
        // this is cd
        if(!strcmp(ARG, "cd"))
            chdir(argv[1]);
        
        else if (!strcmp(ARG, "exit"))
            exit(127); 
 
        // run program from command line
        else{
            //first create child process
            pid_t pid = fork();

            // child process
            if (pid == 0){         

                signal(SIGINT, sigint_handle_child); 
                signal(SIGTSTP, sigtstp_handle_child); 
                //first check bin/*
                snprintf(pth_buf, SIZE, "/bin/%s", ARG);
                // from here on the program is replaced
                execv(pth_buf, argv);
                //then check current directory
                if (errno == 2)
                {
                    snprintf(pth_buf, SIZE, "./%s", ARG);
                    execv(pth_buf, argv);
                } 
                // finally check given path
                if (errno == 2)
                {
                    execv(argv[0], argv);
                }

               //if nothing works, then print error 
                if (errno == 2)
                {
                    free(buf);
                    free(pth_buf);
                    freeargv(argv);
                    free(argv);

                    perror("command not found");
                }
    
                //free(buf);
                //free(pth_buf);
                //freeargv(argv);
                //free(argv);
                exit(127);
            } 
            //parent process
            else
            {
                fflush(stdout);
                freeargv(argv); 
                free(argv);
                job child = {pid, ARG, 1};
                node head = {NULL, &child};
                if(!bg)
                    waitpid(pid, 0, 0);
                else
                    print_list(&head);
            }
        }
        if (off != 1)
            break;
    }
    
    free(buf);
    free(pth_buf);

    return 0;
}

// SIGINT handler
void sigint_handler(int signo) {
    printf("\n");
    fflush(stdout);
}

// SIGSTP handler
void sigtstp_handler(int signo) {
    printf("\n");
    fflush(stdout);
}

// SIGTSTP handles child
void sigint_handle_child(int signo) {
    printf("hello child\n");
    fflush(stdout);
}

// SIGTSTP handles child
void sigtstp_handle_child(int signo) {
    printf("\n");
    fflush(stdout);
}



