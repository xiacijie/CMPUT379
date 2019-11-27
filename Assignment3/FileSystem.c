#include "FileSystem.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

FILE *disk;
char disk_name[1024];
uint8_t current_dir = 127; // root
int mounted = 0; // 0 no fs mounted. 1 fs mounted
Super_block super_block;

int main(int argc, char* argv[]){
    if (argc < 2){
        fprintf(stderr, "Format: ./fs <input_file>\n");
        return 1;
    }

    FILE *input_file = fopen(argv[1],"r");

    char line[1024];
    while(fgets(line, sizeof(line), input_file)) {
        char command = line[0];
        printf("%c", command);
        printf("%s", line);
    }

    fs_mount("disk0");
    fs_create("hell",12);
    fs_delete("hell");
    fs_delete("hell");

    fclose(input_file);
    fclose(disk);
    return 0;
}

void fs_mount(char *new_disk_name) {
    disk = fopen(new_disk_name, "r+");
    if (disk == NULL) {
        fprintf(stderr, "Error: Cannot find disk %s\n", new_disk_name);
        return;
    }

    int status = consistency_check();
    if ( status != 0 ) { // Error
        fprintf(stderr, "Error: File system in %s is inconsistent (error code: %d)\n",new_disk_name, status);  
        return;  
    }

    //load the super block
    fseek(disk,0,SEEK_SET); // rewind to the start of the file
    fread(&super_block, sizeof(Super_block), 1, disk);

    mounted = 1;
    current_dir = 127;  // set to root dir
    strcpy(disk_name, new_disk_name);

}

void fs_create(char name[5], int size) {
    if (mounted == 0){
        fprintf(stderr, "Error: No file system is mounted\n" );
        return;
    }
    //check if inodes are full
    int available_inode_index = -1;
    for (int i = 0; i < 126; i ++) {
        Inode inode = super_block.inode[i];
        if (is_bit_set((uint8_t)inode.used_size,BYTE_LENGTH-1) == 0){
            available_inode_index = i;
            break;
        }
    }

    if (available_inode_index == -1){
        fprintf(stderr,"Error: Superblock in disk %s is full, cannot create %s\n", disk_name,name);
        return;
    }

    printf("fs create: use inode %d\n",available_inode_index);

    //trim name
    char temp_name[1024];
    strcpy(temp_name, name);
    char* trimmed_name = trimwhitespace(temp_name);

    // check for uniqueness of file name
    int result = search_for_name(trimmed_name, super_block.inode);
    if (result != -1) {
        fprintf(stderr, "Error: File or directory %s already exists\n", trimmed_name);
        return;
    }

    if (strcmp(trimmed_name, ".") == 0 || strcmp(trimmed_name,"..") == 0) {
        fprintf(stderr, "Error: File or directory %s already exists\n", trimmed_name);
        return;
    }

   
    //set properties
    if (size == 0) { //dir
        strcpy(super_block.inode[available_inode_index].name, trimmed_name);
        set_bit((uint8_t*)&super_block.inode[available_inode_index].used_size,BYTE_LENGTH-1);
        super_block.inode[available_inode_index].dir_parent = current_dir;
        set_bit((uint8_t*)&super_block.inode[available_inode_index].dir_parent,BYTE_LENGTH-1);
    }
    else { // file
         // scan for available data blocks
        int block_flags[128] = {0};
        get_block_flags(block_flags,super_block.free_block_list);
        int start_block = -1;
        int end_block = -1;
        for (int i = 0; i < 128; i ++){
            int find = 0;
            for (int j = i ; j < 128; j ++) {
                if (block_flags[j] == 1) {
                    break;
                }
                else {
                    if (j-i+1 == size) { // find the first block area
                        start_block = i;
                        end_block = j;
                        find = 1;
                        break;

                    }
                }
            }
            if (find) {
                break;
            }
        }

        // no fit blocks
        if (start_block == -1 && end_block == - 1){
            fprintf(stderr, "Error: Cannot allocate %s on %s\n",trimmed_name,disk_name);
            return;
        }

        // update block flags
        printf("start block: %d end block %d\n",start_block,end_block);
        for (int i = start_block; i <= end_block; i ++){
            block_flags[i] = 1;
        }

        //update free block list
        update_free_block_list(block_flags,super_block.free_block_list);
        int new_flags[128];
        get_block_flags(new_flags,super_block.free_block_list);
        printf("\n");
        for (int i = 0 ; i < 128 ; i ++ ){
            
            printf("%d ",new_flags[i]);
        }
        strncpy(super_block.inode[available_inode_index].name, trimmed_name,5);
        super_block.inode[available_inode_index].used_size = size;
        super_block.inode[available_inode_index].start_block = start_block;
        set_bit(&super_block.inode[available_inode_index].used_size,BYTE_LENGTH-1);
        super_block.inode[available_inode_index].dir_parent = current_dir;

    }

    //write to disk
    save_super_block();
}

void fs_delete(char name[5]) {
    if (mounted == 0){
        fprintf(stderr, "Error: No file system is mounted\n" );
        return;
    }
    char temp_name[1024];
    strcpy(temp_name, name);
    char * trimmed_name = trimwhitespace(temp_name);
    int inode_index = search_for_name(trimmed_name, super_block.inode);

    //file does not exist
    if (inode_index == -1) {
        fprintf(stderr, "Error: File or directory %s does not exist\n", trimmed_name);
        return;
    }

    Inode inode = super_block.inode[inode_index];
    if (is_bit_set((uint8_t)inode.dir_parent,BYTE_LENGTH-1)) { // if it is directory
        //delete all files or directories in this dir
        for (int i = 0; i < 126 ; i ++) {
            Inode inode_i = super_block.inode[inode_index];
            uint8_t par_dir = inode_i.dir_parent;
            clear_bit(&par_dir, BYTE_LENGTH-1);
            if (par_dir == inode_index) {
                fs_delete(inode_i.name);
            }
        }

        // clear inode data
        super_block.inode[inode_index].dir_parent = 0;
        memset(super_block.inode[inode_index].name, 0, 5);
        super_block.inode[inode_index].used_size = 0;
        save_super_block();

    }
    else { // it is file

        uint8_t start_block = inode.start_block;
        //clear data blocks
        uint8_t size = inode.used_size;
        clear_bit(&size, BYTE_LENGTH-1);
        clear_data_blocks(start_block, size);

        //clear free list
        int block_flags[128];
        get_block_flags(block_flags, super_block.free_block_list);
        for (uint8_t i = start_block; i < start_block + size; i ++){
            block_flags[i] = 0;
        }

        update_free_block_list(block_flags,super_block.free_block_list);
    
        super_block.inode[inode_index].dir_parent = 0;
        memset(super_block.inode[inode_index].name, 0, 5);
        super_block.inode[inode_index].start_block = 0;
        super_block.inode[inode_index].used_size = 0;

        save_super_block();

    }


}


/************************
 *  Helper functions  ***
 *                    ***
 * *********************/

/**
 * 0: no error
 */
int consistency_check() {
    Super_block temp_super_block; 

    fseek(disk,0,SEEK_SET); // rewind to the start of the file
    fread(&temp_super_block, sizeof(Super_block), 1, disk);

    /***** 1. verify free space *****/
    int block_flags[128] = {0};  // 0 not used, 1 used
    get_block_flags(block_flags, temp_super_block.free_block_list);


    //debug
    for (int i = 0 ; i < 128; i ++){
        printf("%d ",block_flags[i]);
    }

    // Blocks that are marked free in the free-space list cannot be allocated to any file

    int block_reference_count_table[128] = {0};

    for (int i = 0 ; i < 126 ; i ++) {
        Inode inode = temp_super_block.inode[i];
        if (inode.used_size != 0) {
            uint8_t used_size = inode.used_size; 
            clear_bit((uint8_t*)&used_size,BYTE_LENGTH-1);
            uint8_t start_block = inode.start_block;
            printf("used size :%d\n", used_size);
            for (int j = 0 ; j < used_size ; j ++) {

                block_reference_count_table[start_block + j] += 1; // increment the refernce count of this block by 1

                if (block_flags[start_block + j] == 0) { // if this block is marked as unused: Error!
                    printf("%d\n",start_block + j);
                    printf("Fail 1.1\n");
                    return 1;
                }
            }
        }
    }

    // Blocks marked in use in the free-space list must be allocated to exactly one file
    for (int i = 0 ; i < 128 ; i ++ ) {
        if (block_reference_count_table[i] > 1){ // if this block is referred more than once : Error !
            printf("Fail 1.2\n");
            return 1;
        }
    }

    /***** 2. The name of every file/directory must be unique in each directory. *****/
    for (int i = 0 ; i < 126; i++) {
        Inode inode_i = temp_super_block.inode[i];

        uint8_t dir_parent_i = inode_i.dir_parent;
        clear_bit((uint8_t*)&dir_parent_i,BYTE_LENGTH-1);

        for (int j = i+1 ; i < 126; i ++) {
            Inode inode_j = temp_super_block.inode[j];

            uint8_t dir_parent_j = inode_j.dir_parent;
            clear_bit((uint8_t*)&dir_parent_j,BYTE_LENGTH-1);

            if (inode_i.used_size != 0 && inode_j.used_size != 0 ) { // inodes are in use
                if (strcmp(inode_i.name, inode_j.name) == 0) { // duplicate name: Error
                    printf("%s %s --- \n",inode_i.name,inode_j.name);
                    return 2;
                }
            }

        }
    }

    /*** 3. If the state of an inode is free, all bits in this inode must be zero. Otherwise, the name attribute stored
in the inode must have at least one bit that is not zero ***/

    for (int i = 0 ; i < 126; i ++) {
        Inode inode = temp_super_block.inode[i];

        if (is_bit_set((uint8_t)inode.used_size,BYTE_LENGTH-1) == 0) { //inode free
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
        if (is_bit_set((uint8_t)inode.used_size,BYTE_LENGTH-1) && is_bit_set((uint8_t)inode.dir_parent, BYTE_LENGTH-1) == 0 ) { // file
            if (inode.start_block < 1 || inode.start_block > 127) {
                printf("4. %d\n",inode.start_block);
                return 4;
            }
        }
    }

    /*** 5. The size and start block of an inode that is marked as a directory must be zero. ***/
    for (int i = 0 ; i < 126 ; i ++ ) {
        Inode inode = temp_super_block.inode[i];

        if (is_bit_set(inode.dir_parent, BYTE_LENGTH-1)) { // directory
            uint8_t used_size = inode.used_size;
            clear_bit((uint8_t*)&used_size, BYTE_LENGTH-1);
            if (used_size || inode.start_block != 0) {
                return 5;
            }
        }
    }

    /*** 6. For every inode, the index of its parent inode cannot be 126. Moreover, if the index of the parent inode
is between 0 and 125 inclusive, then the parent inode must be in use and marked as a directory. ***/
    for (int i = 0 ; i < 126 ; i ++ ) {
        Inode inode = temp_super_block.inode[i];

        uint8_t parent_index = inode.dir_parent;
        clear_bit((uint8_t*)&parent_index, BYTE_LENGTH-1);
        if (parent_index == 126) {
            return 6;
        }

        if (parent_index >+ 0 && parent_index <= 125) { // directory
            Inode parent_inode = temp_super_block.inode[parent_index];
            if (is_bit_set((uint8_t)parent_inode.used_size,BYTE_LENGTH-1) == 0 || is_bit_set((uint8_t)parent_inode.dir_parent, BYTE_LENGTH-1) == 0 ) {
                return 6;
            }
        }
    }

    return 0;
}


int is_bit_set(uint8_t ch, int i) {
    uint8_t mask = 1 << i;
    return mask & ch; 
}

void set_bit(uint8_t* ch, int i) {
    uint8_t mask = 1 << i;
    *ch = *ch | mask;
}

void clear_bit(uint8_t* ch, int i) {
    uint8_t mask = 1 << i;
    *ch = *ch & ~(mask);
}


//https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

void get_block_flags(int* block_flags, char* free_block_list) {
    for (int i = 0 ; i < 16; i ++) { // mask for free block num
        char byte = free_block_list[i];
        for (int j = 0 ; j < BYTE_LENGTH; j ++) {
            if (is_bit_set(byte,BYTE_LENGTH - j - 1)){
                block_flags[i*BYTE_LENGTH + j] = 1;
            }
            else{
                block_flags[i*BYTE_LENGTH + j] = 0;
            }
        }
    }
}

void update_free_block_list(int* block_flags, char* free_block_list) {
    for (int i = 0 ; i < 16; i ++) { 
        char* byte = &free_block_list[i];
        for (int j = 0 ; j < BYTE_LENGTH; j ++) {
            if (block_flags[i*BYTE_LENGTH + j] == 1){ //set bit
                set_bit((uint8_t*)byte,BYTE_LENGTH - j - 1);
            }
            else{ // clear bit
                clear_bit((uint8_t*)byte,BYTE_LENGTH - j - 1);
            }
        }
    }
}

int search_for_name(char name[5], Inode* inode_list) {
    for (int i = 0 ; i < 126; i ++) {
        Inode inode = inode_list[i];
        if (is_bit_set((uint8_t)inode.used_size, BYTE_LENGTH-1)) { //inode in use
            if (strncmp(inode.name, name,5) == 0) {
                return i;
            }
        }
    }
    return -1; // does not exist
}


void clear_data_blocks(uint8_t start_block, uint8_t size) {
    Block empty_block; 
    memset(empty_block.bytes,0,BLOCK_SIZE); 

    //load the super block

    fseek(disk,start_block * sizeof(Block),SEEK_SET); // rewind to the start block

    for (uint8_t i = start_block; i < start_block + size; i ++) {
        fwrite(&empty_block, sizeof(Block), 1, disk); // clear each data block
    }
}

void save_super_block() {
    //write to disk
    fseek(disk,0,SEEK_SET); // rewind to the start of the file
    fwrite(&super_block, sizeof(Super_block), 1, disk);
}