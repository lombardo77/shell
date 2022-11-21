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
    int i = 0;

    job a = {0, "null", 0};
    node head = {NULL, &a};

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

        i++;

        if(words > 1 && !strcmp(argv[1], "&"))
            bg = 1;
        else 
            bg = 0;
        
        // this is cd
        if(!strcmp(ARG, "cd"))
            chdir(argv[1]);
        
        else if(!strcmp(ARG, "jobs"))
            print_list(&head);

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
                //make copy of argv[0]
                char* name = (char*)malloc(strlen(ARG));
                strcpy(name, ARG);

                freeargv(argv); 
                free(argv);
               
               // need to allocate on the heap!!
                job* child_job = (job*)malloc(sizeof(job));
                child_job->pid = pid;
                child_job->name = name;
                child_job->index = i;

                node* new_job = malloc(sizeof(node));

                new_job->next = NULL;
                new_job->data = child_job;

                head = *add(&head, new_job);
               
                if (new_job->next == NULL)
                  printf("hello!\n");
                else 
                  printf("%s\n", new_job->next->data->name); 

                if(!bg)
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



