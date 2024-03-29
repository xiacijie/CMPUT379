#include "FileSystem.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

typedef struct {
    Inode inode;
    int index;
} InodeIndex;

FILE *disk = NULL;
char disk_name[512];
uint8_t current_dir = 127; // root  .
uint8_t parent_dir = 127; // root ..
int mounted = 0; // 0 no fs mounted. 1 fs mounted
Super_block super_block;
Buffer buffer;

int main(int argc, char* argv[]){
    if (argc < 2){
        fprintf(stderr, "Format: ./fs <input_file>\n");
        return 1;
    }

    FILE *input_file = fopen(argv[1],"r");

    char line[1024];
    int line_num = 0;
    while(fgets(line, sizeof(line), input_file)) {
        line_num ++;
        char* trimmed_line = trimwhitespace(line);
        // printf("%s\n",line);
        char command = line[0];

        switch (command)
        {
        case 'M': { //fs_mount

            char disk_name[512];
            if (sscanf(trimmed_line, "M %s\n",disk_name) != 1 || num_words(trimmed_line) != 2){
                print_command_error(argv[1], line_num);
                break;
            }
            fs_mount(disk_name);
            break;
            
        }
        case 'C': { //fs_create
  
            char file_name[512];
            int size;
            if (sscanf(trimmed_line, "C %s %d\n", file_name, &size) != 2 || strlen(file_name) > 5 || num_words(trimmed_line) != 3 || size < 0 || size > 127) {
                print_command_error(argv[1], line_num);
                break;
            }
            fs_create(file_name, size);
            break;
        }
        case 'D' : { //fs_delete

            char file_name[512];
            if (sscanf(trimmed_line, "D %s\n", file_name) != 1 || strlen(file_name) > 5 || num_words(trimmed_line) != 2) {
                print_command_error(argv[1], line_num);
                break;
            }
            fs_delete(file_name);
            break;

        }

        case 'B' : { // fs_buff

            char buff[1024];
            char copy[1024];
            strcpy(copy, trimmed_line);

            if (num_words(copy) == 1 || strlen(trimmed_line) - 2 > 1024) {
                print_command_error(argv[1], line_num);
                break;
            }

            strncpy(buff, line+2, strlen(trimmed_line)-3);  // dot not copy /n
            buff[strlen(trimmed_line)-3] = '\0';

            fs_buff((uint8_t*)buff);
            break;

        }

        case 'W' : {
            
            char name[1024];
            int block_num;
            if (sscanf(trimmed_line, "W %s %d\n", name, &block_num) != 2 || strlen(name) > 5 || num_words(trimmed_line) != 3) {
                print_command_error(argv[1], line_num);
                break;
            }

            fs_write(name, block_num);
            break;
        }

        case 'L': {
            if ( num_words(trimmed_line) != 1){
                print_command_error(argv[1], line_num);
                break;
            }
            fs_ls();
            break;
        }

        case 'O': {
            if (num_words(trimmed_line) != 1){
                print_command_error(argv[1], line_num);
                break;
            }
            fs_defrag();
            break;
        }

        case 'E': {

            char name[1024];
            int new_size;
            if (sscanf(trimmed_line, "E %s %d\n",name,&new_size) != 2 || strlen(name) > 5 || num_words(trimmed_line) != 3) {
                print_command_error(argv[1], line_num);
                break;
            }

            fs_resize(name, new_size);
            break;
        }
        case 'R': {
            char name[1024];
            int block_num;
            if (sscanf(trimmed_line, "R %s %d\n",name,&block_num) != 2 || strlen(name) > 5|| num_words(trimmed_line) != 3) {
                print_command_error(argv[1], line_num);
                break;
            }

            fs_read(name, block_num);
            break;
        }
        case 'Y': {
            char name[1024];
            if (sscanf(trimmed_line, "Y %s\n", name) != 1 || strlen(name) > 5|| num_words(trimmed_line) != 2) {
                print_command_error(argv[1], line_num);
                break;
            }

            fs_cd(name);
            break;
        }

        default:
            print_command_error(argv[1], line_num);
            break;
        }
        
    }
    fclose(input_file);
    fclose(disk);
    return 0;
}

void fs_mount(char *new_disk_name) {


    FILE* temp_disk = fopen(new_disk_name, "r+");
    if (temp_disk == NULL) {
        fprintf(stderr, "Error: Cannot find disk %s\n", new_disk_name);
        return;
    }

    int status = consistency_check(temp_disk);
    if ( status != 0 ) { // Error
        fprintf(stderr, "Error: File system in %s is inconsistent (error code: %d)\n",new_disk_name, status);  
        fclose(temp_disk);
        return;  
    }

    if (disk != NULL) {
        fclose(disk);
    }

    disk = temp_disk;
    //load the super block
    fseek(disk,0,SEEK_SET); // rewind to the start of the file
    fread(&super_block, sizeof(Super_block), 1, disk);

    mounted = 1;
    current_dir = 127;  // set to root dir
    parent_dir = 127;
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

    //trim name
    char temp_name[1024];
    strcpy(temp_name, name);
    char* trimmed_name = trimwhitespace(temp_name);

    // check for uniqueness of file name
    int result = search_for_name(current_dir ,trimmed_name, super_block.inode);
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
        int start_block = find_fit_blocks(size);
        // no fit blocks
        if (start_block == -1){
            fprintf(stderr, "Error: Cannot allocate %s on %s\n",trimmed_name,disk_name);
            return;
        }

        int block_flags[128] = {0};
        get_block_flags(block_flags,super_block.free_block_list);

        // update block flags
        for (int i = start_block; i < start_block + size; i ++){
            block_flags[i] = 1;
        }

        //update free block list
        update_free_block_list(block_flags,super_block.free_block_list);

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
    int inode_index = search_for_name(current_dir, trimmed_name, super_block.inode);

    //file does not exist
    if (inode_index == -1) {
        fprintf(stderr, "Error: File or directory %s does not exist\n", trimmed_name);
        return;
    }

    delete_file_or_directory(current_dir, trimmed_name);
  

}

void fs_buff(uint8_t buff[1024]) {
    if (mounted == 0){
        fprintf(stderr, "Error: No file system is mounted\n" );
        return;
    }
    memset(buffer.bytes,0,1024); // flush the buffer
    // set the buffer
    for (int i = 0 ; i < 1024; i ++) {
        buffer.bytes[i] = (char)buff[i];
    } 
}

void fs_write(char name[5], int block_num) {
    if (mounted == 0){
        fprintf(stderr, "Error: No file system is mounted\n" );
        return;
    }
    int inode_index = search_for_name(current_dir, name, super_block.inode);
    if ( inode_index == -1) {
        fprintf(stderr, "Error: File %s does not exist\n", name);
        return;
    }
    Inode inode = super_block.inode[inode_index];
    
    uint8_t start_block = inode.start_block;
    uint8_t size = inode.used_size;
    clear_bit(&size, BYTE_LENGTH-1);
    if (block_num > size - 1 || block_num < 0) {
        fprintf(stderr, "Error: %s does not have block %d\n",name, block_num);
        return;
    }

    Block new_block;
    for (int i = 0 ; i < 1024; i ++ ){
        new_block.bytes[i] = buffer.bytes[i];
    }

    write_data_block(new_block, start_block + block_num);
}

void fs_ls(void) {
    if (mounted == 0){
        fprintf(stderr, "Error: No file system is mounted\n" );
        return;
    }
    printf("%-5s %3d\n", ".", get_directory_size(current_dir, super_block.inode));
    printf("%-5s %3d\n", "..", get_directory_size(parent_dir, super_block.inode));

    for (int i = 0 ; i < 126; i ++) {
        Inode inode = super_block.inode[i];
        if (is_bit_set(inode.used_size, BYTE_LENGTH-1)) {
            uint8_t dir_par = inode.dir_parent;
            uint8_t parent = dir_par;
            clear_bit(&parent, BYTE_LENGTH-1);

            if (is_bit_set(dir_par,BYTE_LENGTH-1) == 0) { //file

                if (parent == current_dir) {
                    uint8_t size = inode.used_size;
                    clear_bit(&size,BYTE_LENGTH-1);
                    char print_name[6];
                    strncpy(print_name,inode.name,5);
                    print_name[5] = '\0';
                    printf("%-5s %3d KB\n", print_name, size);
                }
            }
            else { //dir
                if (parent == current_dir) {
                    char print_name[6];
                    strncpy(print_name,inode.name,5);
                    print_name[5] = '\0';
                    printf("%-5s %3d\n", print_name, get_directory_size(i, super_block.inode));
                }
            }
        }
    }
}

void fs_defrag(void) {
    if (mounted == 0){
        fprintf(stderr, "Error: No file system is mounted\n" );
        return;
    }
    int block_flags[128] = {0};
    get_block_flags(block_flags,super_block.free_block_list);

    InodeIndex sorted_files[126];

    int files_size = 0;
    for (int i = 0; i < 128; i ++) {
        Inode inode = super_block.inode[i];
        if (is_bit_set(inode.used_size, BYTE_LENGTH-1) && is_bit_set(inode.dir_parent,BYTE_LENGTH-1)==0) {
            InodeIndex ii;
            ii.index = i;
            ii.inode = inode;
            sorted_files[files_size++] = ii;
        }
    }

    //sort the files by start block
    qsort(sorted_files,files_size,sizeof(InodeIndex), compare);

    for (int i = 0 ; i < files_size; i ++) {
        InodeIndex ii = sorted_files[i];
        Inode inode = ii.inode;

        uint8_t start_block = inode.start_block;
        uint8_t size = inode.used_size;
        clear_bit(&size, BYTE_LENGTH-1);
        for (int j = start_block; j < start_block + size; j ++ ) {

            int smallest_number = j - 1;
            while (1) {
                if (block_flags[smallest_number] == 1 || smallest_number == 0) {
                    break;
                }
                else{
                    smallest_number --;
                }
            }
            //set the start block
            if (j == start_block) {
                super_block.inode[ii.index].start_block = smallest_number + 1;
            }

            Block block = get_data_block(j);
            clear_data_blocks(j,1);
            write_data_block(block, smallest_number+1);
            block_flags[j] = 0;
            block_flags[smallest_number+1] = 1;
            
        }

    }

    update_free_block_list(block_flags,super_block.free_block_list);
    save_super_block();

}

void fs_resize(char name[5], int new_size) {
    if (mounted == 0){
        fprintf(stderr, "Error: No file system is mounted\n" );
        return;
    }
    int inode_index = search_for_name(current_dir,name,super_block.inode);
    if (inode_index == -1) {
        fprintf(stderr, "Error: File %s does not exist\n", name);
    }
    Inode inode = super_block.inode[inode_index];
    uint8_t size = inode.used_size;
    clear_bit(&size, BYTE_LENGTH-1);

    if (new_size == size) {
        return;
    }

    int block_flags[128] = {0};
    get_block_flags(block_flags,super_block.free_block_list);

    uint8_t start_block = inode.start_block;
    if (new_size < size) { //size smaller that current size
        for (int i = start_block + size -1 ; i >= start_block + new_size; i --) {
            block_flags[i] = 0;
            clear_data_blocks(i,1);
        }

        uint8_t updated_size = new_size;
        set_bit(&updated_size, BYTE_LENGTH-1);
        super_block.inode[inode_index].used_size = updated_size;
        update_free_block_list(block_flags,super_block.free_block_list);
        save_super_block();
    }
    else { // size greater than the current size
        int size_gap = new_size - size;
        int following_size = 0;
        int end_block = start_block + size;
        int k = end_block;

        while (k < 128 && block_flags[k] == 0){
            following_size ++;
            k ++;
        }

        if (following_size >= size_gap) { // append after the current allocated blocks
            for (int i = end_block; i < end_block + size_gap; i ++) {
                block_flags[i] = 1;
            }
            update_free_block_list(block_flags, super_block.free_block_list);
            uint8_t updated_size = new_size;
            set_bit(&updated_size, BYTE_LENGTH-1);
            super_block.inode[inode_index].used_size = updated_size;
            save_super_block();
        }
        else { // find other blocks
            int new_start_block = find_fit_blocks(new_size);
            if (new_start_block == -1){
                fprintf(stderr,"Error: File %s cannot expand to size %d\n",name,new_size);
                return;
            }

            //copy old block to new blocks
            for (int i = 0; i < size; i ++) {
                Block block = get_data_block(i + start_block);
                //copy to new block
                write_data_block(block,new_start_block + i);
                //clear old block
                clear_data_blocks(start_block + 1, 1);
                //set flags
                block_flags[i+ start_block] = 0;
                block_flags[i + new_start_block] = 1;
            }

            //allocate the following blocks
            for (int i = 0 ; i < size_gap; i ++ ) {
                block_flags[i+size+new_start_block] = 1;
            }

            super_block.inode[inode_index].start_block = new_start_block;
            uint8_t updated_size = new_size;
            set_bit(&updated_size, BYTE_LENGTH-1);
            super_block.inode[inode_index].used_size = updated_size;
            update_free_block_list(block_flags,super_block.free_block_list);
            save_super_block();
        }

    }
}

void fs_read(char name[5], int block_num) {
    if (mounted == 0){
        fprintf(stderr, "Error: No file system is mounted\n" );
        return;
    }
    int inode_index = search_for_name(current_dir, name, super_block.inode);
    if (inode_index == -1){
        fprintf(stderr, "Error: File %s does not exist\n", name);
        return;
    }

    Inode inode = super_block.inode[inode_index];
    
    uint8_t start_block = inode.start_block;
    uint8_t size = inode.used_size;
    clear_bit(&size, BYTE_LENGTH-1);
    if (block_num > size - 1 || block_num < 0) {
        fprintf(stderr, "Error: %s does not have block %d\n",name, block_num);
        return;
    }

    Block block = get_data_block(start_block + block_num);
    for (int i = 0; i < 1024; i ++) {
        buffer.bytes[i] = block.bytes[i];
    }

}

void fs_cd(char name[5]) {
    if (mounted == 0){
        fprintf(stderr, "Error: No file system is mounted\n" );
        return;
    }
    int inode_index;

    if (strcmp(name,".") == 0 ){
        return;
    }

    if (strcmp(name,"..") == 0) {
        if (current_dir == 127){
            return;
        }
        if (parent_dir == 127) {
            current_dir = 127;
            return;
        }
        inode_index = parent_dir;
    }
    else{
        inode_index = search_for_name(current_dir, name, super_block.inode);
    }


    if (inode_index == -1 || is_bit_set(super_block.inode[inode_index].dir_parent,BYTE_LENGTH-1) == 0){
        fprintf(stderr, "Error: Directory %s does not exist\n",name);
        return;
    }

    uint8_t par_dir = super_block.inode[inode_index].dir_parent;
    clear_bit(&par_dir, BYTE_LENGTH-1);
    parent_dir = par_dir;
    current_dir = inode_index;
}

/************************
 *  Helper functions  ***
 *                    ***
 * *********************/

void delete_file_or_directory(uint8_t current_dir, char name[5]) {

    int inode_index = search_for_name(current_dir, name, super_block.inode);
    Inode inode = super_block.inode[inode_index];
    if (is_bit_set((uint8_t)inode.dir_parent,BYTE_LENGTH-1)) { // if it is directory
        //delete all files or directories in this dir
        for (int i = 0; i < 126 ; i ++) {
            Inode inode_i = super_block.inode[i];
            uint8_t par_dir = inode_i.dir_parent;
            clear_bit(&par_dir, BYTE_LENGTH-1);

            if (is_bit_set(inode_i.used_size,BYTE_LENGTH-1) && par_dir == inode_index) {
                delete_file_or_directory(inode_index, inode_i.name);
            }
        }

        // clear inode data
        super_block.inode[inode_index].dir_parent = 0;
        memset(super_block.inode[inode_index].name, 0, 5);
        super_block.inode[inode_index].start_block = 0;
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

int num_words(char* sentence) {
    char* token;
    int counter = 0;
    token = strtok(sentence, " ");
    for(int i = 0; token != NULL; ++i){
        counter ++;
        token = strtok(NULL, " ");
    }
    return counter;
}

int find_fit_blocks(int size) {

    int block_flags[128] = {0};
    get_block_flags(block_flags,super_block.free_block_list);
    int start_block = -1;
    for (int i = 0; i < 128; i ++){
        int find = 0;
        for (int j = i ; j < 128; j ++) {
            if (block_flags[j] == 1) {
                break;
            }
            else {
                if (j-i+1 == size) { // find the first block area
                    start_block = i;
                    find = 1;
                    break;

                }
            }
        }
        if (find) {
            break;
        }
    }
    return start_block;
}

int compare(const void *a, const void* b) {
    InodeIndex *inode1 = (InodeIndex *)a;
    InodeIndex *inode2 = (InodeIndex* )b;
    return (inode1->inode.start_block -  inode2->inode.start_block);
}

void print_command_error(char* input_file, int line_num){
    fprintf(stderr, "Command Error: %s, %d\n", input_file, line_num);
}

/**
 * 0: no error
 */
int consistency_check(FILE * temp_disk) {
    Super_block temp_super_block; 

    fseek(temp_disk,0,SEEK_SET); // rewind to the start of the file
    fread(&temp_super_block, sizeof(Super_block), 1, temp_disk);

    /***** 1. verify free space *****/
    int block_flags[128] = {0};  // 0 not used, 1 used
    get_block_flags(block_flags, temp_super_block.free_block_list);

    // Blocks that are marked free in the free-space list cannot be allocated to any file
    // Blocks marked in use in the free-space list must be allocated to exactly one file

    for (int i = 1 ; i < 128; i ++) {
        int once = 0;
        for (int j = 0; j < 126 ; j ++) {
            Inode inode = temp_super_block.inode[j];
            if (is_bit_set(inode.used_size,BYTE_LENGTH-1) && is_bit_set(inode.dir_parent,BYTE_LENGTH-1) == 0) {
                uint8_t used_size = inode.used_size;
                clear_bit(&used_size,BYTE_LENGTH-1);
                if (i >= inode.start_block && i < inode.start_block + used_size) {
                    if (block_flags[i] == 0){
                        return 1;
                    }

                    if (block_flags[i] == 1){
                        if (once == 0){
                            once = 1;
                        }
                        else{
                            return 1;
                        }
                    }
                }
            }
        }

        if (block_flags[i] == 1 && !once) {
            return 1;
        }

    }


    /***** 2. The name of every file/directory must be unique in each directory. *****/
    for (int i = 0 ; i < 126; i++) {
        Inode inode_i = temp_super_block.inode[i];

        for (int j = i+1 ; j < 126; j ++) {
            Inode inode_j = temp_super_block.inode[j];

            if (is_bit_set(inode_i.used_size,BYTE_LENGTH-1) && is_bit_set(inode_j.used_size,BYTE_LENGTH-1) != 0 ) { // inodes are in use
                if (strncmp(inode_i.name, inode_j.name,5) == 0) { // duplicate name: Error
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
        if (is_bit_set(inode.used_size,BYTE_LENGTH-1)) { //in use
            uint8_t parent_index = inode.dir_parent;
            clear_bit((uint8_t*)&parent_index, BYTE_LENGTH-1);
            if (parent_index == 126) {

                return 6;
            }

            if (parent_index >= 0 && parent_index <= 125) { // directory
                Inode parent_inode = temp_super_block.inode[parent_index];
                uint8_t size = parent_inode.used_size;
                clear_bit(&size, BYTE_LENGTH-1);
                if (is_bit_set((uint8_t)parent_inode.used_size,BYTE_LENGTH-1) == 0 ) {
                    return 6;
                }

                if ( is_bit_set((uint8_t)parent_inode.dir_parent, BYTE_LENGTH-1) == 0) {
                    return 6;
                }
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

int search_for_name(uint8_t current_dir, char name[5], Inode* inode_list) {
    for (int i = 0 ; i < 126; i ++) {
        Inode inode = inode_list[i];
        uint8_t par_dir = inode.dir_parent;
        clear_bit(&par_dir,BYTE_LENGTH-1);
        if (is_bit_set((uint8_t)inode.used_size, BYTE_LENGTH-1) && current_dir == par_dir  ) { //inode in use and it is in current directory
            if (strncmp(inode.name, name,5) == 0) {
                return i;
            }
        }
    }
    return -1; // does not exist
}

int get_directory_size(uint8_t current_dir,Inode* inode_list) {
    int counter = 0;
    for (int i = 0 ; i < 126; i ++) {
        Inode inode = inode_list[i];
        uint8_t par_dir = inode.dir_parent;
        clear_bit(&par_dir,BYTE_LENGTH-1);
        if (is_bit_set(inode.used_size, BYTE_LENGTH-1) && par_dir == current_dir ) {
            // printf("get_dir size: inode: %d\n",i);
            counter += 1;
        }
    }

    return counter + 2;
}

void clear_data_blocks(uint8_t start_block, uint8_t size) {
    Block empty_block; 
    memset(empty_block.bytes,0,BLOCK_SIZE); 

    fseek(disk,start_block * sizeof(Block),SEEK_SET); // rewind to the start block

    for (uint8_t i = 0; i < size; i ++) {
        fwrite(&empty_block, sizeof(Block), 1, disk); // clear each data block
    }
}

Block get_data_block(int global_block_num) {
    fseek(disk, global_block_num* sizeof(Block), SEEK_SET);
    Block block;
    fread(&block,sizeof(Block),1,disk);
    return block;
}

void write_data_block(Block block, int global_block_num) {
    fseek(disk, global_block_num* sizeof(Block), SEEK_SET);
    fwrite(&block, sizeof(Block),1 , disk);
}

void save_super_block() {
    //write to disk
    fseek(disk,0,SEEK_SET); // rewind to the start of the file
    int i = fwrite(&super_block, sizeof(Super_block), 1, disk);
    if (i != 1){
        printf("Fail to save super block!\n");
    }
}