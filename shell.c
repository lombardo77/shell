#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

char** split_buf(char* buf);
void print_array(int arr_len, char** arr);
void rm_ws(char* s, char a);


//to do
// 1. split buf into multiple strings to put into argv 
// (which is supposed to be a char**)
// 2. remove spaces from strings
int main(){
    char* buf = (char*)malloc(100);
    char* cwd = (char*)malloc(100);
    char* pth_buf = (char*)malloc(100);
    
    int off = 0;
    //shell loop
    while(1){
        printf("%s> ", getcwd(cwd, 100));
        off = scanf("%[^\n]%*c", buf);
        pid_t pid = fork();
        
        if (pid == 0){   //child process
            
            char **argv = (char**)malloc(100);
            argv = split_buf(buf);
           
            // clean argv[0]
            rm_ws(argv[0], '/');
            if (argv[0][0] == '.')
                argv[0] = &argv[0][1];
            //first check bin
            snprintf(pth_buf, 100, "/bin/%s", argv[0]);
            execv(pth_buf, argv);
            //erno seems to return 2 when there is no program to run in bin
            if (errno == 2){
                //snprintf(pth_buf, 100, "%s/%s", getcwd(cwd, 100), argv[0]);
                snprintf(pth_buf, 100, "./%s", argv[0]);
                execv(pth_buf, argv);
            } 
            printf("%d\n", errno);
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




void print_array(int arr_len, char** arr){
    for (int i = 0; i < arr_len ;i ++)
        printf("%s, ", arr[i]);
}

// this function creates argv from buf.
// to do: create a copy of each string.
char** split_buf(char* buf){
    
    int spaces = 0; 
    char* buf_cpy = malloc(100);
    char** argv = malloc(100);
    strcpy(buf_cpy, buf);
    argv[0] = buf_cpy;
    for (int i = 0; i < 100; i ++){

        // if the string is "echo 'hello world'"
        if (buf_cpy[i] == ' ' && buf_cpy[i + 1] != ' '){
            
            spaces ++;
            argv[spaces] = &buf_cpy[i + 1];
            buf_cpy[i] = '\0';

        }
    }

    // replace elements of argv with copies of those elements
    // this is to ensure that I can free bud_cpy

    for(int i =0; i < spaces + 1 ; i++){
        // first i have to allocate mem to save the string. I'll use 
        // strlen to get the size to allocate
        // then I copy arv[i] into that allocated mem
        // finally I replace arv[i] with the allocated mem
        char* token_cpy = (char*)malloc(strlen(argv[i]));
        strcpy(token_cpy, argv[i]);
        rm_ws(token_cpy, ' ');
        argv[i] = token_cpy;
    }
    argv[spaces + 2] = NULL;
    free(buf_cpy);
    return argv;
}


//removes white space in string
void rm_ws(char* s, char a) {
    char* d = s;
    do {
        while (*d == a) {
            ++d;
        }
    } while (*s++ = *d++);
}








