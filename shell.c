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
int fg_prc;
/*
 * job fg_prc = job{pid,name,index,TRUE,status};
 * */

int main(void){
    char* buf = (char*)malloc(SIZE);
    char* pth_buf = (char*)malloc(SIZE);
    int i = 0;
    int bg = 0;

    node* jobs_lst = new_list();
    node* bg_prc = new_list();
    pid_t pid;

    // numbe of words in argv
    extern int words;

    //signals
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGCHLD, sigchld_handler);

    int off = 0;
    //shell loop
    while(1)
    {
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
            bg = 1;
        else 
            bg = 0;
       
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
        else if (!strcmp(ARG, "exit"))
        {   
            free(buf);
            free(pth_buf);
            free_list(jobs_lst);
            exit(127); 
        }
        else if(!strcmp(ARG, "fg"))
        {
            int pid_tfg = atoi(argv[1]);
            fg_prc = pid_tfg;
            kill(pid_tfg, SIGCONT);
            waitpid(pid_tfg, 0, 0);
        }
        else if(!strcmp(ARG, "bg"))
        {
            int pid_tfg = atoi(argv[1]);
            kill(pid_tfg, SIGCONT);
            //then change status to running
            bg = 1;
        }

        // this is kill
        else if(!strcmp(ARG, "kill"))
        {   
            int killed_pid;
            if (argv[1][0] == '%')
                killed_pid = get_pid(bg_prc,  atoi(argv[1] + 1));
            else
                killed_pid = atoi(argv[1]);

            kill(killed_pid, SIGTERM);
            printf("[1] %d terminated by signal %d\n", killed_pid, SIGTERM);
            bg_prc = rm_node(bg_prc, killed_pid);
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
                
                // from here on the program is replaced only if there is a 
                // program in /bin/*

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
                node* new_job = newjobnode(i, ARG, pid, bg, 1);
                node* new_job_bg = newjobnode(i, ARG, pid, bg, 1);

                if(!bg)
                {
                    fg_prc = pid;
                    jobs_lst = add(jobs_lst, new_job);
                    waitpid(pid, 0, WUNTRACED);
                }
                else
                { // need to allocate on the heap!
                    bg_prc = add(bg_prc, new_job);
                    jobs_lst = add(jobs_lst, new_job_bg);
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
    kill(fg_prc, SIGTERM);
    fflush(stdout);
}

// SIGSTP handler
void sigtstp_handler(int signo) {
    write(STDOUT_FILENO, "\n", 1);
    kill(fg_prc, SIGSTOP);
    fflush(stdout);
}


void sigchld_handler(int signo){
    printf("I got a signal: %d\n", getpid());
}

