#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//to do
// 1. split buf into multiple strings to put into argv

int main(){
    char* buf = (char*)malloc(100);
    char* cwd = (char*)malloc(100);
    char* pth_buf = (char*)malloc(100);
    int off = 0;
    while(1){
        printf("%s> ", getcwd(cwd, 100));
        off = scanf("%[^\n]%*c", buf);
        
        pid_t pid = fork();
        
        if (pid == 0){   //child process
            char *argv[] = {buf, NULL};
            //format path
            snprintf(pth_buf, 100, "/bin/%s", buf);
            execv(pth_buf, argv);
            exit(127);
        }
        else{   //parent process
            waitpid(pid, 0, 0);
        }

        if (off != 1)
            break;
    }
    free(buf);
    free(cwd);
    free(pth_buf);

    return 0;
}
