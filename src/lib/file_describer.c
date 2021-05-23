#include "file_describer.h"


#define FILE_DESCRIBER_ARRAY_LENGTH 100

bool file_describer_array_occupied[FILE_DESCRIBER_ARRAY_LENGTH];
File_Describer file_describer_array[FILE_DESCRIBER_ARRAY_LENGTH];

void init_file_describer(){
    memset(file_describer_array_occupied,0, sizeof(file_describer_array_occupied));
    for (int i = 0; i < 10; ++i) {
        file_describer_array_occupied[i] = true;
    }
}

int get_new_file_describer(){
    for(int i=0;i<FILE_DESCRIBER_ARRAY_LENGTH;i++){
        if(!file_describer_array_occupied[i]){
            file_describer_array_occupied[i]=true;
            return i;
        }
    }
    return -1;
}

void erase_file_describer(int n){
    assert(n>=0);
    assert(n<FILE_DESCRIBER_ARRAY_LENGTH);
    assert(file_describer_array_occupied[n]);
    file_describer_array_occupied[n]=false;
}
