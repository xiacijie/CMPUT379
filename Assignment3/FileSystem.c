#include "FileSystem.h"

FILE *disk;

uint8_t current_dir = 127; // root
int mounted = 0; // 0 no fs mounted. 1 fs mounted
Super_block super_block;

int main(){
    fs_mount("disk0");
    return 0;
}

void fs_mount(char *new_disk_name) {
    disk = fopen(new_disk_name, "rw");
    if (disk == NULL) {
        fprintf(stderr, "Error: Cannot find disk %s", new_disk_name);
        return;
    }

    consistency_check(new_disk_name);
}

/**
 * 0: no error
 */
int consistency_check(char* new_disk_name ) {
    Super_block temp_super_block; 

    fseek(disk,0,SEEK_SET); // rewind to the start of the file
    fread(&temp_super_block, sizeof(Super_block), 1, disk);

    // 1. 
    int block_flags[128] = {0};  // 0 not used, 1 used

    for (int i = 0 ; i < 16; i ++) { // mask for free block num
        char byte = temp_super_block.free_block_list[i];
        for (int j = 0 ; j < BYTE_LENGTH; j ++) {
            if (is_bit_set(byte,BYTE_LENGTH - j - 1)){
                block_flags[i*BYTE_LENGTH + j] = 1;
            }
        }
    }

    //debug
    for (int i = 0 ; i < 128; i ++){
        printf("%d ",block_flags[i]);
    }

    for (int i = 0 ; i < 126 ; i ++) {
        Inode inode = temp_super_block.inode[i];
        if (inode.used_size != 0) {
            uint8_t used_size  = ((inode.used_size) << 1) >> 1; // shift out the first bit
            uint8_t start_block = inode.start_block;

            for (int j = 0 ; j < used_size ; j ++) {
                if (block_flags[start_block + j] == 0) {
                    fprintf(stderr, "Error: File system in %s is inconsistent (error code: %d)",new_disk_name,1);    
                }
            }
        }
    }
    return 0;
}


int is_bit_set(char ch, int i) {
    char mask = 1 << i;
    return mask & ch; 
}