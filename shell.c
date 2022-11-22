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

void sigchld_handler(int signo);
void sigint_handler(int signo);
void sigtstp_handler(int signo);
pid_t pid;
int bg = 0;

int main(void){
    char* buf = (char*)malloc(SIZE);
    char* pth_buf = (char*)malloc(SIZE);
    int i = 0;

    node* head = new_list();

    // numbe of words in argv
    extern int words;

    //signals
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    //signal(SIGCHLD, sigchld_handler);

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
        else 
            bg = 0;
        
        // this is "cd"
        if(!strcmp(ARG, "cd"))
            chdir(argv[1]);
        // this is "jobs" 
        else if(words == 1 && !strcmp(ARG, "jobs"))
            print_list(head);
        // this is "exit"
        else if (!strcmp(ARG, "exit"))
        {   
            free(buf);
            free(pth_buf);
            free_list(head);
            exit(127); 
        }
        if(!strcmp(ARG, "fg"))
        {
            printf("hello!\n");
            kill(atoi(argv[1]), SIGCONT);
            bg = 0;
        }
        // this is kill
        else if(!strcmp(ARG, "kill"))
        {   
            int killed_pid;
            if (argv[1][0] == '%')
                killed_pid = get_pid(head,  atoi(argv[1] + 1));
            else
                killed_pid = atoi(argv[1]);

            kill(killed_pid, SIGTERM);
            printf("[1] %d terminated by signal %d\n", killed_pid, SIGTERM);
            head = rm_node(head, killed_pid);
            waitpid(killed_pid, 0, 0);
        }
 
        // run program from command line
        else{
            //first create child process
            pid = fork();
            // child process
            if (pid == 0){    
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
                
                exit(127);
            } 
            //parent process
            else
            {
                    i ++;
                    char* name = (char*)malloc(strlen(ARG));
                    strcpy(name, ARG);

                    job* child_job = (job*)malloc(sizeof(job));
                    child_job->pid = pid;
                    child_job->name = name;
                    child_job->index = i;

                    node* new_job = malloc(sizeof(node));

                    new_job->next = NULL;
                    new_job->data = child_job;

                if(!bg)
                {
                    head = add(head, new_job);
                    waitpid(pid, 0, WUNTRACED);
                    head = rm_node(head, pid);
                }
                else
                { // need to allocate on the heap!
                    head = add(head, new_job);
                    printf("[%d] %d\n",i, pid);
                }
                fflush(stdout);
                freeargv(argv); 
                free(argv);
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
    kill(pid, SIGSTOP);
    fflush(stdout);
}

// SIGCHLD handler
void sigchld_handler(int signo){
    printf("hello I just got a signal %d\n", signo);
}


