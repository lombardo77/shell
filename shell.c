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

#define RUNNING 1
#define STOPPED -1
#define DONE 0

#define BG 1
#define FG 0

void bgcommand(char* arg1);
void fgcommand(char* arg1);
void killcommand(char* arg1);
void sigint_handler(int signo);
void sigtstp_handler(int signo);

job* fg_prc;
node* jobs_lst;
node* bg_prc;

volatile int  sigtstp;
volatile int sigint;

int main(void){
    char* buf = (char*)malloc(SIZE);
    char* pth_buf = (char*)malloc(SIZE);
    int i = 0;
    int bg = 0;

    //job lists (bg and all jobs)
    jobs_lst = new_list();
    bg_prc = new_list();
    pid_t pid;

    // number of words in argv
    extern int words;

    //signals
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);

    int off = 0;
    //shell loop
    while(1)
    {
        if (sigtstp){
            // sets the status of the foregound process to STOPPED and bg to 1
            node* new_bg_node = newjobnode(
            fg_prc->index, fg_prc->name, fg_prc->pid, 
            BG, STOPPED);
            bg_prc = add(bg_prc, new_bg_node);
            print_list(bg_prc);
            fg_prc = NULL;
            fflush(stdout);
            sigtstp = 0;
        }
        if (sigint){
            rm_node(bg_prc, fg_prc->pid);
            sigint = 0;
        }

        //command line prompt "> "
        printf("> ");
        fflush(stdout);
        
        off = scanf("%[^\n]%*c", buf);
        fflush(stdin); 

        // pointer to argument array
        char **argv;

        // generate argument array with raw command line input
        argv = gen_argv(buf);

        if(words > 1 && !strcmp(argv[1], "&"))
            bg = BG;
        else 
            bg = FG;
        /* custom commands here:
         * cd, jobs, exit, fg, bg, kill
         * will put into functions for readability*/
        // this is "cd"
        if(!strcmp(ARG, "cd"))
            chdir(argv[1]);
        // this is "jobs" 
        else if(words == 1 && !strcmp(ARG, "jobs"))
            print_list(bg_prc);
        // this is "exit"
        else if (!strcmp(ARG, "exit")){   
            free(buf);
            free(pth_buf);
            free_list(jobs_lst);
            exit(127); 
        }
        //fg puts first arg into foreground
        else if(!strcmp(ARG, "fg")){
            fgcommand(argv[1]);
        }
        //take stopped process and run it in the backgroud
        else if(!strcmp(ARG, "bg")){
            bgcommand(argv[1]);
        }

        // this is kill
        else if(!strcmp(ARG, "kill")){   
            killcommand(argv[1]);
        }
        
        // run program from command line
        else{
            //first create child process
            pid = fork();
            // child process
            if (pid == 0){    
                //first check bin/*
                snprintf(pth_buf, SIZE, "/bin/%s", ARG);
                
                // from here on the program is replaced only if there is a 
                // program in /bin/*

                execv(pth_buf, argv);
                //then check current directory
                if (errno == 2){
                    snprintf(pth_buf, SIZE, "./%s", ARG);
                    execv(pth_buf, argv);
                } 
                // finally check given path
                if (errno == 2){
                    execv(argv[0], argv);
                }

               //if nothing works, then print error 
                if (errno == 2){
                    free(buf);
                    free(pth_buf);
                    freeargv(argv);
                    free(argv);
                    perror("command not found");
                }
                
                exit(127);
            } 
            //parent process
            else{
                i ++;
                node* new_job = newjobnode(i, ARG, pid, bg, RUNNING);
                node* new_job_bg = newjobnode(i, ARG, pid, bg, RUNNING);
                jobs_lst = add(jobs_lst, new_job);

                if(!bg){ //bg is 0
                    fg_prc = getjob(jobs_lst, pid);
                    printf("%d is running in fg\n", fg_prc->pid);
                    waitpid(pid, 0, WUNTRACED);
                } else{ // bg is 1
                    bg_prc = add(bg_prc, new_job_bg);
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
    write(STDOUT_FILENO, "\n", 1);
    kill(fg_prc->pid, SIGTERM);
    fflush(stdout);
    sigint = 1;
}

// SIGSTP handler
void sigtstp_handler(int signo) {
    write(STDOUT_FILENO, "\n", 1);
    kill(fg_prc->pid, SIGSTOP);
    sigtstp = 1;
}

//parses arg1 and kills pid
void killcommand(char* arg1){
    int killed_pid;
    if (arg1[0] == '%')
        killed_pid = getpidbi(bg_prc,  atoi(arg1 + 1));
    else
        killed_pid = atoi(arg1);

    kill(killed_pid, SIGTERM);
    printf("[1] %d terminated by signal %d\n", killed_pid, SIGTERM);
    bg_prc = rm_node(bg_prc, killed_pid);
    waitpid(killed_pid, 0, 0);
}

// foregrounds process id arg1
void fgcommand(char* arg1){
    //first arg to int
    int pid_tfg = atoi(arg1);

    //get process from jobs list to set fg variable
    fg_prc = getjob(jobs_lst, pid_tfg);
    fg_prc->bg = FG;
    fg_prc->status = RUNNING;

    //remove fg_prc from background list
    bg_prc = rm_node(bg_prc, fg_prc->pid);

    // continue the process
    kill(pid_tfg, SIGCONT);

    //wait for the process to end
    waitpid(pid_tfg, 0, WUNTRACED);

}

//backgrounds process id arg1
void bgcommand(char* arg1){
    int pid_tfg = atoi(arg1);

    // continue the process
    kill(pid_tfg, SIGCONT);
    
    // change status and bg
    getjob(bg_prc, pid_tfg)->bg = BG;
    getjob(bg_prc, pid_tfg)->status = RUNNING;
}

