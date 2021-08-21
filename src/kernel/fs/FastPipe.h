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
#define MAX_PIPE_COUNT (10)
#define PIPE_BUF_LENGTH (4096 * 30)
extern PipeData* fd_to_buff_address[FILE_DESCRIBER_ARRAY_LENGTH];
extern char pipe_buffer[MAX_PIPE_COUNT][PIPE_BUF_LENGTH];
extern int current_pipe_buffer_pointer;

class FastPipe {
public:
    static void Init() {
        memset(fd_to_buff_address, 0, sizeof(fd_to_buff_address));
        current_pipe_buffer_pointer = 0;
    }

    static void OpenPipe(const int fd[2]) {
        if (current_pipe_buffer_pointer >= MAX_PIPE_COUNT) {
            panic("[Pipe] Full Pipe Buffer\n");
        }
        size_t start_address = (size_t) pipe_buffer[current_pipe_buffer_pointer++];
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
            size_t next_back = (back + 1) % PIPE_BUF_LENGTH;
            if (back != front) {
                buff[i] = pipe_buf[back];
                back = next_back;
            } else {
                if (i == 0) {
                    if (get_running_pipe_read_count() > 200) {
                        LOG("[Pipe] kill process because of dead lock\n");
                        exit_process(0);
                    }
                    ++get_running_pipe_read_count();
                    __yield();
                }else {
                    get_running_pipe_read_count() = 0;
                    yield_with_return(i);
                }
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
            size_t next_front = (front + 1) % PIPE_BUF_LENGTH;
            if (next_front != back) {
                pipe_buf[front] = buf[i];
                front = next_front;
            } else {
                if (get_running_pipe_write_count() > 110) {
                    exit_process(0);
                }
                get_running_pipe_write_count()++;
                if (i == 0) {
                    __yield();
                } else {
                    get_running_pipe_write_count() = 0;
                    yield_with_return(i);
                }
                panic("Full Pipe");
            }
            ++i;
        }
        get_running_pipe_write_count() = 0;
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
};



#endif //SYSCORE_FASTPIPE_H

