#include <stdio.h>
#include <stdlib.h>
#include "sha512.h"
#include <inttypes.h>

#define CHUNK_SIZE (16 * 1024)

long get_file_size(FILE *file_pointer) {
        long file_size, current_pos;
        current_pos = ftell(file_pointer);
        if(current_pos == -1L) {
                return -1;
        }
        if(fseek(file_pointer, 0, SEEK_END) != 0) {
                return -1;
        }
        file_size = ftell(file_pointer);
        if(file_size == -1L) {
                return -1;
        }
        if(fseek(file_pointer, current_pos, SEEK_SET) != 0) {
                return -1;
        }
        return file_size;
}

int calculate_num_levels(int num_leaves) {
        int levels = 0;
        int nodes = num_leaves;

        while(nodes > 1) {
                nodes = (nodes + 1)/2;
                levels++;
        }
        
        return ++levels;
}


