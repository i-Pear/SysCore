#include "stddef.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void execute(char* path){
    if(fork()==0){
        exec(path);
    }else{
        int exit_code;
        wait(&exit_code);
    }
}

struct file_list{
    char filename[128];
    struct file_list* next;
};

struct file_list* get_dir_files(char* path){
    struct file_list* res;
    get_997((void*)path,(void*)&res);
    return res;
}

int main(){
    char env_cwd[100];
    char cmd[100];
    while(1){
        getcwd(env_cwd,100);
        printf("%s >",env_cwd);
        gets(cmd);
        char* argv[100];
        int argc=0;
        int is_space=1;
        int cmd_len=strlen(cmd);
        // analyze tokens
        for(int i=0;cmd[i]!='\0';i++){
            if(cmd[i]==' '){
                cmd[i]='\0';
                is_space=1;
            }else{
                if(is_space){
                    argv[argc++]=&cmd[i];
                    is_space=0;
                }
            }
        }
        // do operations
        if(strcmp(argv[0], "ls")==0){
            struct file_list* cnt;
            if(argc==2){
                cnt=get_dir_files(argv[1]);
            }else if(argc==1){
                cnt=get_dir_files(0);
            }else{
                printf("arguments error.\n");
                continue;
            }
            if(cnt==0){
                printf("There isn't any file in %s, is the path wrong?\n",argv[1]);
            }else{
                int file_count=0;
                while(cnt){
                    file_count++;
                    printf("%s\n",cnt->filename);
                    cnt=cnt->next;
                }
                printf("%d files in total.\n",file_count);
            }
        }else if(strcmp(argv[0], "pwd")==0){
            if(argc!=1){
                printf("arguments error.\n");
                continue;
            }
            char cwd[100];
            getcwd(cwd,100);
            printf("Current path is:\n%s\n",cwd);
        }else if(strcmp(argv[0], "cat")==0){
            if(argc!=2){
                printf("arguments error.\n");
                continue;
            }
            int fd = open(argv[1], 0);
            if(fd<0){
                printf("file not exist.\n");
                continue;
            }
            char buf[1024];
            int size = read(fd, buf,1024);
            if (size < 0) {
                size = 0;
            }
            write(STDOUT, buf, size);
            close(fd);
        }else if(strcmp(argv[0], "cd")==0){
            if(argc!=2){
                printf("arguments error.\n");
                continue;
            }
            int exist=0;
            if(strcmp(argv[1],".")==0)exist=1;
            if(strcmp(argv[1],"..")==0)exist=1;
            struct file_list* cnt=get_dir_files(0);
            while(cnt){
                    if(strcmp(cnt->filename,argv[1])==0){
                        exist=1;
                        break;
                    }
                    cnt=cnt->next;
                }
            if(exist==0){
                printf("dir not exist!\n");
                continue;
            }
            chdir(argv[1]);
        }else if(strcmp(argv[0], "help")==0){
            printf(
            "Commands:\n"
            "help -- show this page\n"
            "ls -- list files\n"
            "pwd -- show where current dictionary is\n"
            "cat -- read content from text file\n"
            "cd -- change dictionary\n"
            "You can also run a program by typing its name.\n"
            );
        }else if(strcmp(argv[0], "write")==0){
            int fd = open(argv[1], O_CREATE | O_RDWR);
            char* buf=argv[1];
            for(int i=0;i<cmd_len;i++){
                if(cmd[i]=='\0')cmd[i]=' ';
            }
            printf("cmd_len=%d\n",cmd_len);
            printf("write %d bytes\n",cmd_len-6);
            write(fd,buf,cmd_len-6);
            close(fd);
            printf("write success.\n");
        }else{
            // exec
            int exist=0;
            if(strcmp(argv[1],".")==0)exist=1;
            if(strcmp(argv[1],"..")==0)exist=1;
            struct file_list* cnt=get_dir_files(0);
            while(cnt){
                    if(strcmp(cnt->filename,argv[1])==0){
                        exist=1;
                        break;
                    }
                    cnt=cnt->next;
            }
            if(exist==0){
                printf("dir not exist!\n");
                continue;
            }
            execute(argv[1]);
        }
    }
}

