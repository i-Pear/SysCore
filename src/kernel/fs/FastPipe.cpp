#include "FastPipe.h"

PipeData* fd_to_buff_address[FILE_DESCRIBER_ARRAY_LENGTH];

char pipe_buffer[MAX_PIPE_COUNT][PIPE_BUF_LENGTH];

int current_pipe_buffer_pointer;
