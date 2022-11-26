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
#define ARG0 argv[0]

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
void stop_fgprc();
void term_fgprc();

job* fg_prc;
node* jobs_lst;
node* active_jobs;

volatile int  sigtstp;
volatile int sigint;

int main(void) 
{
    char* buf = (char*)malloc(SIZE);
    char* pth_buf = (char*)malloc(SIZE);
    char **argv;
    int i = 0;
    int bg = 0;

    //job lists (bg and all jobs)
    jobs_lst = new_list();
    active_jobs = new_list();
    pid_t pid;
    fg_prc = NULL;

    // number of words in argv
    extern int words;

    //signals
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);

    int off = 0;
    //shell loop
    while (1) {
        if (sigtstp)
            stop_fgprc();
        if (sigint)
            term_fgprc();

        //command line prompt "> "
        printf("> ");
        fflush(stdout);
        off = scanf("%[^\n]%*c", buf);
        fflush(stdin);

        // pointer to argument array
        argv = gen_argv(buf);

        if (words > 1 && !strcmp(argv[1], "&"))
            bg = BG;
        else 
            bg = FG;
        /* custom commands here:
         * cd, jobs, exit, fg, bg, kill
         * will put into functions for readability*/
        // this is "cd"
        if (!strcmp(ARG0, "cd"))
            chdir(argv[1]);
        else if (words == 1 && !strcmp(ARG0, "jobs"))
            print_list(active_jobs);
        else if (words == 1 && !strcmp(ARG0, "all_jobs"))
            print_list(jobs_lst);
        else if (!strcmp(ARG0, "exit")) {   
            free(buf);
            free(pth_buf);
            freeargv(argv);
            free(argv);
            
            freelnaj(jobs_lst);
            freejob(active_jobs->data);
            freelno(active_jobs);

            exit(127); 
        } else if (!strcmp(ARG0, "fg"))
            fgcommand(argv[1]);
          else if (!strcmp(ARG0, "bg"))
            bgcommand(argv[1]);
          else if (!strcmp(ARG0, "kill"))   
            killcommand(argv[1]);
          else {
            pid = fork();
            // child process
            if (pid == 0) {    
                //first check bin/*
                snprintf(pth_buf, SIZE, "/bin/%s", ARG0);
                execv(pth_buf, argv);
                //then check current directory
                if (errno == 2) {
                    snprintf(pth_buf, SIZE, "./%s", ARG0);
                    execv(pth_buf, argv);
                } 
                // finally check given path
                if (errno == 2) {
                    execv(argv[0], argv);
                }

               //if nothing works, then print error 
                if (errno == 2) {
                    free(buf);
                    free(pth_buf);
                    freeargv(argv);
                    free(argv);
                    perror("command not found");
                }
                
                exit(127);
            } else {
                i ++;
                job* curjob = createjob(i, ARG0, pid, bg, RUNNING);
                node* new_job = createnode(curjob);
                node* new_job_bg = createnode(curjob);
                jobs_lst = add(jobs_lst, new_job);
                active_jobs = add(active_jobs, new_job_bg);
                fg_prc = curjob;

                if (!bg) { //bg is false, running in fg
                    waitpid(pid, 0, WUNTRACED);
                    if (!sigtstp) { // if it did not stop, then it finished.
                        printf("Finished [%d]\n", curjob->pid);
                        curjob->status = DONE;
                        active_jobs = rm_node(active_jobs, fg_prc->pid);
                    }
                } else  // bg is 1, will run in background
                    printf("[%d] %d\n",i, pid);
                fflush(stdout);
            }
        }
        if (off != 1)
            break;
        freeargv(argv); 
        free(argv);
    }
    
    free(buf);
    free(pth_buf);
    return 0;
}

// SIGINT handler
void sigint_handler(int signo) 
{
    write(STDOUT_FILENO, "\n> ", 1);
    if (fg_prc != NULL) 
        kill(fg_prc->pid, SIGTERM);
    fflush(stdout);
    sigint = 1;
}

// SIGSTP handler
void sigtstp_handler(int signo) 
{
    write(STDOUT_FILENO, "\n> ", 1);
    if (fg_prc != NULL)
        kill(fg_prc->pid, SIGSTOP);
    sigtstp = 1;
}

//parses arg1 and kills pid
void killcommand(char* arg1) 
{
    int killed_pid;
    if (arg1[0] == '%')
        killed_pid = getpidbi(active_jobs,  atoi(arg1 + 1));
    else
        killed_pid = atoi(arg1);
    if (!is_running(getjob(active_jobs, killed_pid)))
        kill(killed_pid, SIGCONT);
    
    kill(killed_pid, SIGTERM);
    printf("[1] %d terminated by signal %d\n", killed_pid, SIGTERM);
    active_jobs = rm_node(active_jobs, killed_pid);
    waitpid(killed_pid, 0, WUNTRACED);
}

// foregrounds process id arg1
void fgcommand(char* arg1) 
{
    //first arg to int
    int pid_tfg = atoi(arg1);

    //get process from jobs list to set fg variable
    fg_prc = getjob(jobs_lst, pid_tfg);

    if (fg_prc->status == STOPPED)
        kill(pid_tfg, SIGCONT);

    fg_prc->bg = FG;
    fg_prc->status = RUNNING;

    //wait for the process to end
    waitpid(pid_tfg, 0, WUNTRACED);
}

//backgrounds process id arg1
void bgcommand(char* arg1) 
{
    int pid_tfg = atoi(arg1);

    // continue the process
    kill(pid_tfg, SIGCONT);
    
    // change status and bg
    getjob(active_jobs, pid_tfg)->bg = BG;
    getjob(active_jobs, pid_tfg)->status = RUNNING;
}


// stops fg process
void stop_fgprc() 
{
    if (fg_prc != NULL) {
        fg_prc->status = STOPPED;
        fg_prc->bg = BG;
    }
    fflush(stdout);
    sigtstp = 0;
}

// terminates fg process
void term_fgprc() 
{
    if (fg_prc != NULL)
        active_jobs = rm_node(active_jobs, fg_prc->pid);
    sigint = 0;
}
