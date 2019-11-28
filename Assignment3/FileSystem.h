#include <stdio.h>
#include <stdint.h>

#define BLOCK_SIZE 1024 // 1024 bytes
#define BLOCK_NUM  128 // 128 blocks in total
#define BYTE_LENGTH 8 // 1 byte = 8 bit 
typedef struct {
	char name[5];        // Name of the file or directory
	uint8_t used_size;   // Inode state and the size of the file or directory
	uint8_t start_block; // Index of the start file block
	uint8_t dir_parent;  // Inode mode and the index of the parent inode
} Inode;

typedef struct {
	char free_block_list[16];
	Inode inode[126];
} Super_block;

typedef struct {
	char bytes[BLOCK_SIZE]; // 1 kb
} Block; 

typedef struct {
	char bytes[1024]; // 1024
} Buffer;


void fs_mount(char *new_disk_name);
void fs_create(char name[5], int size);
void fs_delete(char name[5]);
void fs_read(char name[5], int block_num);
void fs_write(char name[5], int block_num);
void fs_buff(uint8_t buff[1024]);
void fs_ls(void);
void fs_resize(char name[5], int new_size);
void fs_defrag(void);
void fs_cd(char name[5]);

/*** Helper functions ***/
int num_words(char* sentence);
int find_fit_blocks(int size);
int compare(const void *a, const void* b);
void print_command_error(char* input_file, int line_num);
int consistency_check();
int is_bit_set(uint8_t ch, int i);
void set_bit(uint8_t *ch, int i);
void clear_bit(uint8_t *ch, int i);
char *trimwhitespace(char *str);
void get_block_flags(int* block_flags, char* free_block_list);
void update_free_block_list(int* block_flags, char* free_block_list);
int search_for_name(uint8_t current_dir, char name[5], Inode* inode_list);
void clear_data_blocks(uint8_t start_block, uint8_t size);
void write_data_block(Block block, int global_block_num);
Block get_data_block(int global_block_num);
int get_directory_size(uint8_t current_dir,Inode* inode_list);
void save_super_block();