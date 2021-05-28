#include "stdio.h"

int input_int(){
    char c;
    int res=0;
    while (1){
        c=getchar();
        if(c>='0'&&c<='9')res=res*10+c-'0';
        else return res;
    }
}

int main(int argc, char *argv[]){
    printf("input two int, enter to end:\n");
    int a=input_int();
    int b=input_int();
    printf("sum is %d\n",a+b);
    return 0;
}
