#include "FileSystem.h"
#include <string.h>

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

    int status = consistency_check();
    if ( status != 0 ) { // Error
        fprintf(stderr, "Error: File system in %s is inconsistent (error code: %d)",new_disk_name, status);    
    }
}

/**
 * 0: no error
 */
int consistency_check() {
    Super_block temp_super_block; 

    fseek(disk,0,SEEK_SET); // rewind to the start of the file
    fread(&temp_super_block, sizeof(Super_block), 1, disk);

    /***** 1. verify free space *****/
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

    // Blocks that are marked free in the free-space list cannot be allocated to any file

    int block_reference_count_table[128] = {0};

    for (int i = 0 ; i < 126 ; i ++) {
        Inode inode = temp_super_block.inode[i];
        if (inode.used_size != 0) {
            uint8_t used_size  = ((inode.used_size) << 1) >> 1; // shift out the first bit
            uint8_t start_block = inode.start_block;

            for (int j = 0 ; j < used_size ; j ++) {

                block_reference_count_table[start_block + j] += 1; // increment the refernce count of this block by 1

                if (block_flags[start_block + j] == 0) { // if this block is marked as unused: Error!
                    return 1;
                }
            }
        }
    }

    // Blocks marked in use in the free-space list must be allocated to exactly one file
    for (int i = 0 ; i < 128 ; i ++ ) {
        if (block_reference_count_table[i] > 1){ // if this block is referred more than once : Error !
            return 1;
        }
    }

    /***** 2. The name of every file/directory must be unique in each directory. *****/
    for (int i = 0 ; i < 126; i++) {
        Inode inode_i = temp_super_block.inode[i];

        uint8_t dir_parent_i = ((inode_i.dir_parent) << 1 ) >> 1;

        for (int j = i ; i < 126; i ++) {
            Inode inode_j = temp_super_block.inode[j];

            uint8_t dir_parent_j = ((inode_j.dir_parent) << 1 ) >> 1;

            if (inode_i.used_size != 0 && inode_j.used_size != 0 ) { // inodes are in use
                if (strcmp(inode_i.name, inode_j.name) == 0) { // duplicate name: Error
                    return 2;
                }
            }

        }
    }

    /*** 3. If the state of an inode is free, all bits in this inode must be zero. Otherwise, the name attribute stored
in the inode must have at least one bit that is not zero ***/

    for (int i = 0 ; i < 126; i ++) {
        Inode inode = temp_super_block.inode[i];

        if (is_bit_set(inode.used_size,BYTE_LENGTH-1) == 0) { //inode free
            // check name
            for (int j = 0 ; j < 5; j ++){
                if (inode.name[j] != 0) { // name is not empty: Error
                    return 3;
                }
            }

            //check others
            if (inode.start_block != 0 || inode.dir_parent != 0 || inode.used_size != 0 ){
                return 3;
            }
        }
        else { //inode in use
            //check name, at least one bit is not zero
            int name_invalid = 1; // invalid initially
            for (int j = 0 ; j < 5 ; j ++ ){
                if (inode.name[j] != 0 ) {
                    name_invalid = 0; // valid
                    break;
                }
            }

            if (name_invalid) {
                return 3;
            }
        }
    }


    /*** 4. The start block of every inode that is marked as a file must have a value between 1 and 127
inclusive ***/
    for (int i = 0 ; i < 126 ; i ++ ) {
        Inode inode = temp_super_block.inode[i];

        if (inode.used_size != 0 && is_bit_set(inode.dir_parent, BYTE_LENGTH-1) == 0 ) { // file
            if (inode.start_block < 1 || inode.start_block > 127) {
                return 4;
            }
        }
    }

    /*** 5. The size and start block of an inode that is marked as a directory must be zero. ***/
    for (int i = 0 ; i < 126 ; i ++ ) {
        Inode inode = temp_super_block.inode[i];

        if (is_bit_set(inode.dir_parent, BYTE_LENGTH-1)) { // directory
            if (inode.used_size <<1 >> 1 != 0 || inode.start_block != 0) {
                return 5;
            }
        }
    }

    /*** 6. For every inode, the index of its parent inode cannot be 126. Moreover, if the index of the parent inode
is between 0 and 125 inclusive, then the parent inode must be in use and marked as a directory. ***/
    for (int i = 0 ; i < 126 ; i ++ ) {
        Inode inode = temp_super_block.inode[i];

        uint8_t parent_index = inode.dir_parent << 1 >> 1;
        if (parent_index == 126) {
            return 6;
        }

        if (parent_index >+ 0 && parent_index <= 125) { // directory
            Inode parent_inode = temp_super_block.inode[parent_index];
            if (is_bit_set(parent_inode.used_size,BYTE_LENGTH-1) == 0 || is_bit_set(parent_inode.dir_parent, BYTE_LENGTH-1) == 0 ) {
                return 6;
            }
        }
    }


    super_block = temp_super_block; // replace the super block
    mounted = 1;
    return 0;
}


int is_bit_set(uint8_t ch, int i) {
    uint8_t mask = 1 << i;
    return mask & ch; 
}