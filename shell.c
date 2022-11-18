#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

char** split_buf(char* buf);
void print_array(int arr_len, char** arr);
char* rm_ws(char* str);
char* rm_wsb(char* s, char a);

//to do
// 1. split buf into multiple strings to put into argv 
// (which is supposed to be a char**)
// 2. remove spaces from strings
// 3. ensure "ls " doesn't raise error
int main(){
    char* buf = (char*)malloc(100);
    char* cwd = (char*)malloc(100);
    char* pth_buf = (char*)malloc(100);

    int off = 0;
    //shell loop
    while(1){
        printf("> ");
        off = scanf("%[^\n]%*c", buf);
        
        char **argv = (char**)malloc(100);
        argv = split_buf(buf);

        // this is cd
        if(strcmp(argv[0], "cd") == 0){
            chdir(argv[1]);
        } 

        else{
            pid_t pid = fork();
            // child process
            if (pid == 0){              
                  
                //first check bin
                snprintf(pth_buf, 100, "/bin/%s", argv[0]);
                execv(pth_buf, argv);
                //then check this directory
                if (errno == 2){
                    snprintf(pth_buf, 100, "./%s", argv[0]);
                    execv(pth_buf, argv);
                } 
                //finally check given path
                if (errno == 2){
                    execv(argv[0], argv);
                }
               //if nothing works, then print error 
                if (errno == 2){
                    perror("command not found");
                }
                exit(127);
            }
            else{   //parent process

                waitpid(pid, 0, 0);
            }
        }
        if (off != 1)
            break;
    }
    free(buf);
    free(cwd);
    free(pth_buf);

    return 0;
}

//prints array
void print_array(int arr_len, char** arr){
    for (int i = 0; i < arr_len ;i ++)
        printf("%s, ", arr[i]);
}

// this function creates argv from buf.
// to do: create a copy of each string.
char** split_buf(char* buf){
    
    int spaces = 0; 

    char* buf_cpy0 = malloc(100);
    char* buf_cpy;

    char** argv = malloc(100);
    
   //remove white space from front of string -- i.e., "   ls" -> "ls"
    strcpy(buf_cpy0, buf);

    buf_cpy = rm_wsb(buf_cpy0, ' ');
    
   // places buf_cpy into argv[0] 
    argv[0] = buf_cpy;
    for (int i = 0; i < 100; i ++){

        // if the string is "echo 'hello world'"
        // places null terminator at last space char
        // e.g., "hello,         world" -> "hello,       '\0'world"
        if (buf_cpy[i] == ' ' && buf_cpy[i + 1] != ' '){
            
            spaces ++;
            argv[spaces] = &buf_cpy[i + 1];
            buf_cpy[i] = '\0';

        }
    }

    // replace elements of argv with copies of those elements
    // this is to ensure that I can free bud_cpy

    for(int i =0; i < spaces + 1 ; i++){
        char* token_cpy = (char*)malloc(strlen(argv[i]));
        strcpy(token_cpy, argv[i]);
        token_cpy = rm_ws(token_cpy);
        argv[i] = token_cpy;
    }

    argv[spaces + 2] = NULL;
    free(buf_cpy0);
    return argv;
}

char* rm_ws(char *str)
{
    int nsc = 0;
 
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] != ' ')
        {
            str[nsc] = str[i];
            nsc++;        }    
    }
    
    str[nsc] = '\0';
    return str;
}


//removes white space in string
//void rm_ws(char* s, char a) {
//    char* d = s;
//    do {
//            r+d;
//        }
//    } while (*s++ = *d++);
//}



//removes white space in beg of string
char* rm_wsb(char* s, char a) {
    char* d = s;
    while (*d == a)
        ++d;
    return d;
}









