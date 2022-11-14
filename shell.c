#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    char* buf = (char*)malloc(100);
    char cwd[100];
    int off = 0;
    while(1){
        printf("%s> ", getcwd(cwd, 100));
        off = scanf("%s", buf);
        if (off != 1)
            break;
    }
    free(buf);
}
