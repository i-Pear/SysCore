#ifndef SYSCORE_FASTPIPE_H
#define SYSCORE_FASTPIPE_H

#include "VFS.h"

struct PipeData {
    int fd[2];
    size_t start_address;
    size_t front_pointer;
    size_t back_pointer;
};
#define FILE_DESCRIBER_ARRAY_LENGTH 120
extern PipeData* fd_to_buff_address[FILE_DESCRIBER_ARRAY_LENGTH];

class FastPipe {
public:
    static void OpenPipe(const int fd[2]) {
        size_t start_address = alloc_page();
        auto* pipeData = new PipeData{
            .fd = {fd[0], fd[1]},
            .start_address = start_address,
            .front_pointer = 0,
            .back_pointer = 0};
        fd_to_buff_address[fd[0]] = pipeData;
        fd_to_buff_address[fd[1]] = pipeData;
    }

    static int Read(int fd, char buff[], int count) {
        size_t& front = fd_to_buff_address[fd]->front_pointer;
        size_t& back = fd_to_buff_address[fd]->back_pointer;
        char* pipe_buf = (char *) fd_to_buff_address[fd]->start_address;
        int i = 0;
        while (i < count) {
            size_t next_back = (back + 1) % 4096;
            if (back != front) {
                buff[i] = pipe_buf[back];
                back = next_back;
            } else {
                if (get_running_pipe_read_count() > 200) {
                    printf("[Pipe] kill process because of dead lock\n");
                    exit_process(0);
                }
                ++get_running_pipe_read_count();
                __yield();
            }
            ++i;
        }
        get_running_pipe_read_count() = 0;
        return count;
    }

    static int Write(int fd, char buf[], int count) {
        size_t& front = fd_to_buff_address[fd]->front_pointer;
        size_t& back = fd_to_buff_address[fd]->back_pointer;
        char* pipe_buf = (char *) fd_to_buff_address[fd]->start_address;
        int i = 0;
        while (i < count) {
            size_t next_front = (front + 1) % 4096;
            if (next_front != back) {
                pipe_buf[front] = buf[i];
                front = next_front;
            } else {
                panic("Full Pipe");
            }
            ++i;
        }
        return count;
    }

    static int TestPipe(int fd) {
        size_t& front = fd_to_buff_address[fd]->front_pointer;
        size_t& back = fd_to_buff_address[fd]->back_pointer;
        if (front == back){
            yield_with_return(0);
        }
        return 1;
    }

private:

};



#endif //SYSCORE_FASTPIPE_H

