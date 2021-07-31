#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(int argc,char* argv[],char* envp[]){

    printf("argc  %x  %d\n",&argc,argc);
    printf("argv  %x\n",argv);
    printf("envp  %x\n\n",envp);

    printf("argv:\n");
    for(int i=0;argv[i];i++){printf("%x\t%x\t",argv+i,argv[i]);printf("%s\n",argv[i]);}
    printf("-----------------\n");
    printf("envp:\n");
    for(int i=0;envp[i];i++){printf("%x\t%x\t",envp+i,envp[i]);printf("%s\n",envp[i]);}

    return 0;
}
