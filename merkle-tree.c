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
// calculating total number of nodes and number of nodes per level
	int num_levels = calculate_num_levels(num_leaves);
	int number_of_nodes_per_level[num_levels];
	int current_number_of_nodes = num_leaves;
	int total_nodes = 0;
	for(int i = 0; i < num_levels; i++) {
		total_nodes += current_number_of_nodes;
		number_of_nodes_per_level[i] = current_number_of_nodes;
		if(current_number_of_nodes == 1) {
			break;
		}
		current_number_of_nodes = (current_number_of_nodes + 1)/2;
	}

	uint64_t *tree = (uint64_t*)malloc(total_nodes * (sizeof(uint64_t)*8));

// constructing the tree 
	for(int i = 0; i < num_leaves; i++) {
		uint8_t *starting_pointer = binary_data + i*CHUNK_SIZE;
		long current_size;
		if(i == num_leaves - 1) {
			current_size = size - i*CHUNK_SIZE;
		}
		else {
			current_size = CHUNK_SIZE;
		}
		uint64_t *hash = sha512(starting_pointer, current_size);
		for(int j = 0; j < 8; j++) {
			tree[i*8+j] = hash[j];
		}
//		free(hash);
		hash = NULL;
	}
	
	int hash_offset = 0;
	int write_offset = num_leaves * 8;

	for(int level = 1; level < num_levels; level++) {

		int prev_level_nodes = number_of_nodes_per_level[level-1];
		int current_level_nodes = number_of_nodes_per_level[level];

		for(int i = 0; i < current_level_nodes; i++) {
			int left_index = i * 2;
		        int right_index = left_index + 1;
		
		// if both the left and the right children exist
		        if (right_index < prev_level_nodes) {
            			uint8_t *ptr = (uint8_t*)(tree + hash_offset + left_index * 8);
            			uint64_t *parent_hash = sha512(ptr, 128);
            			for (int j = 0; j < 8; j++) {
                			tree[write_offset + i * 8 + j] = parent_hash[j];
            			}
 //           			free(parent_hash);
        		} 

			//else if only left child exists (odd number of nodes) promote the last child as it is
			else { 
            			for (int j = 0; j < 8; j++) {
                			tree[write_offset + i * 8 + j] =
                    			tree[hash_offset + left_index * 8 + j];
            			}
        		}
		}

		hash_offset = write_offset;
		write_offset += current_level_nodes * 8;
	}
	uint64_t* root = malloc(8 * sizeof(uint64_t));
	for (int i = 0; i < 8; i++) {
    		root[i] = tree[(total_nodes - 1) * 8 + i];
	}
	free(tree);
	free(binary_data);
	tree = NULL;
	binary_data = NULL;

	return root;
}

