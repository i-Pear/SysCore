#include "stddef.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int string2int(char* s){
    int res=0;
    while (*s!='\0'){
        res=res*10+*s-'0';
    }
    return res;
}

int main(){
    printf("A simple calculator test:\n");
    printf("input number's count: ");
    char num[100];
    gets(num);
    int count= string2int(num);

    int sum=0;
    while (count--){
        gets(num);
        sum+= string2int(num);
    }
    printf("Sum is %d.\n",sum);
}