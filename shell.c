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
void sigchld_handler(int signo);
void stop_fgprc();
void term_fgprc();
void segfault();

job* fg_prc;
node* jobs_lst;
node* active_jobs;

volatile int  sigtstp;
volatile int sigint;
volatile int sigsegv;

int status;

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
    
    fg_prc = NULL;
    pid_t pid;

    // number of words in argv
    extern int words;

    //signals
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGCHLD, sigchld_handler);

    int off = 0;
    //shell loop
    while (1) {
        if (sigtstp)
            stop_fgprc();
        if (sigint)
            term_fgprc();
        if (sigsegv)
            segfault();

        //command line prompt "> "
        printf("> ");
        fflush(stdout);
        off = scanf("%[^\n]%*c", buf);
        fflush(stdin);

        // pointer to argument array
        argv = gen_argv(buf);

        if (words > 1 && !strcmp(argv[words - 1], "&"))
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
                setpgid(pid, pid);   
                //first check bin/*
                snprintf(pth_buf, SIZE, "/bin/%s", ARG0);
                execv(pth_buf, argv);
                
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
                //lists share jobs, not nodes
                job* curjob = createjob(i, ARG0, pid, bg, RUNNING, buf);
                node* new_job = createnode(curjob);
                node* new_job_bg = createnode(curjob);
                
                //add to lists
                jobs_lst = add(jobs_lst, new_job);
                active_jobs = add(active_jobs, new_job_bg);

                //set fgjob
                fg_prc = curjob;
        
                if (!bg) { //bg is false, running in fg
                    waitpid(pid, &status, WUNTRACED);
                    if (!sigtstp) { // if it did not stop, then it finished.
                        curjob->status = DONE;
                        active_jobs = rm_node(active_jobs, fg_prc->pid);
                    }
                } else  // bg is 1, will run in background
                    printf("[%d] %d running in background\n",i, pid);
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
    if (fg_prc != NULL && 
            fg_prc->bg == FG && 
            fg_prc->status == RUNNING) {
        kill(fg_prc->pid, SIGINT);
        sigint = 1;
        waitpid(fg_prc->pid, 0, 0);
    }
    fflush(stdout);
}

// SIGSTP handler
void sigtstp_handler(int signo) 
{
    write(STDOUT_FILENO, "\n> ", 1);
    if (fg_prc != NULL && 
            fg_prc->bg == FG && 
            fg_prc->status == RUNNING)
        kill(fg_prc->pid, SIGSTOP);
    sigtstp = 1;
}

//SIGSEGV handler
void sigchld_handler(int signo)
{
    if (status == SIGSEGV) {
        sigsegv = 1;
        status = 0;
    }
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
    int pid_tfg;
    if (arg1[0] == '%') 
        pid_tfg = getpidbi(active_jobs, atoi(arg1 + 1));
    else
        pid_tfg = atoi(arg1);

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
    int pid_tfg;
    if (arg1[0] == '%') 
        pid_tfg = getpidbi(active_jobs, atoi(arg1 + 1));
    else
        pid_tfg = atoi(arg1);


    // continue the process
    kill(pid_tfg, SIGCONT);
    
    // change status and bg
    fg_prc = getjob(jobs_lst, pid_tfg);

    fg_prc->bg = BG;
    fg_prc->status = RUNNING;
}


// stops fg process
void stop_fgprc() 
{
    if (fg_prc != NULL && fg_prc->bg == FG) {
        fg_prc->status = STOPPED;
        fg_prc->bg = BG;
    }
    fflush(stdout);
    sigtstp = 0;
}

// terminates fg process
void term_fgprc() 
{
    if (fg_prc != NULL && fg_prc->bg == FG) {
            printf("[%d] %d terminated with signal %d\n", 
            fg_prc->index, fg_prc->pid, SIGINT);
    }

        active_jobs = rm_node(active_jobs, fg_prc->pid);
    sigint = 0;
}

// terminated with segfault
void segfault() 
{
    printf("Segmentation fault\n");
    sigsegv = 0;
}
