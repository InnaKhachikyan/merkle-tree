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

uint64_t* final_hash(const char *filename) {

// opening file to get the binary data
        FILE *fp = fopen(filename, "rb");
        if(!fp) {
                perror("fopen failed");
                exit(1);
        }
        long size = get_file_size(fp);
        if(size < 0) {
                perror("get_file_size failed");
                fclose(fp);
                exit(1);
        }
        uint8_t *binary_data = (uint8_t*)malloc(size);
        if(!binary_data) {
                perror("malloc");
                exit(1);
        }
        if(fread(binary_data, 1, size, fp) != size) {
                perror("fread");
                free(binary_data);
                binary_data = NULL;
                fclose(fp);
                exit(1);
        }
        fclose(fp);

        int num_leaves;
        if(size%CHUNK_SIZE == 0) {
                num_leaves = size/CHUNK_SIZE;
        }
        else {
                num_leaves = size/CHUNK_SIZE + 1;
        }
}
