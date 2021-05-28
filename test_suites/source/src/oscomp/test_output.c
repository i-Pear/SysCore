#include "stdio.h"

int main(int argc, char *argv[]){
    char c;
    while (1){
        c=getchar();
        if(c!='\n')putchar(c);
        else break;
    }
    return 0;
}
