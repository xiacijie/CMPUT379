Design: 
    1. fs_mount:
        This function basically first load the super block from given disk and run consistency check. If the consistency check passes, load the super block to memory. Otherwise discard the super block and close the disk.
        System call: no system call used;
        Test: Load consistent file system and inconsistent file system. 

    2. fs_create:
        This function creates a file or directory by differentiating between the size. 0 -> directoty. Otherwise -> file. And set the inode info respectively.
        System call: none
        Test: create file and repo. Create file or repo that have duplicate names. Create file that can be allocated. Create files that cannot be allocated (no enough block left). keep creating repo until all the inodes run out.

    3. fs_create:
        This function delete file or repo. If it is the repo to delete, recursively delete all elements in the repo. This function is accomplished by a helper function delete_file_or_respository, which handles the recursive deletion.
        System call: none
        Test: delete file. Delete an empty repo. Delete repo has only files. Delete repo has both files and repos. Delete file/repo that does not exist.

    4. fs_read:
        This function read the block into buffer. We wind the file pointer to the corresponding position and use fread to read the block.
        System call: none
        Test: read a file does not exist. Read a file that does not have certain block. 

    5. fs_write:
        This function write a block into buffer. Also wind the file pointer to the right position and use fwrite to write the block into file/disk.
        System call: none
        Test: Same as fs_read

    6. fs_buff:
        Flush the buffer and write bytes into buffer by using loop to set every byte. 
        System call: none
        Test: write bytes into buffer;
    
    7. fs_ls:
        This function list all the files/dir in the current dir. "." and ".." needs special notice. Basically by traversing all the inode and find files/repo belong to the current directory and list them respectively.
        System call: none
        Test: list an empty dir. Create some file or repos and the list them.

    8. fs_resize:
        This function resize the file to a certain size. If the size if smaller than the original size, we just free the block that are not in need and update the free_list and then update the super block. If the size if greater than the original size, we first check if the following free blocks are enough for the allocation. If so, do it. Otherwise we using loop finding the fit blocks and move all the data.
        System call: none
        Test: resize to a smaller size. Resize to a bigger size that can be allocated at the tail of the current blocks. Resize to a bigger size that requires moving the data blocks that can still fit. Resize to a bigger size that cannot fit.

    9. fs_defrag
        This function defragment the data blocks. We find all the in use inodes and sort them by the start block. Then we shift the data blocks one by one until finish.
        System call: none
        Test: defrag when there are holes. defrag when there are no holes.

    10. fs_cd:
        change the directory. We keep two global vriable recording the current dir and parent dir. fs_cd will update them when fs_cd is called.
        System call: none
        Test: change to an existed repo. Change to a not existed repo. Change to the file. Change to ".". Change to "..";

    11. Other helper functions:
        Those functions are written to support functions from 1-10. Most of them are resuable such as search_for_name and get_block_flags that get the flags of the free_data_list.
        System call: none
        Test: Those will be tested along with the above functions.





Reference:
1. trim white space: //https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
2. CMPUT 379 course and lab slides