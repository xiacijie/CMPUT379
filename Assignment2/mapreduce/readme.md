WARNING: Please read the following notes before using the mapreduce library.
1. Make sure you free the memory of the char* returned by function char *MR_GetNext(char *key, int partition_number) otherwise it will cause memory leak!

Design of the intermediate data structure:
    1. The data stucture contains a hash table (unordered_map in C++ STL) and a mutex lock.
    2. The key of the hash table is the partition number and the value is a list (vector in C++ STL) containing the data.
    3. The data is a struct with two fields - key and value.
    4. The data structure has two operation methods. One is DataStructure_addData, which insert data into the hash table at the corresponding partition and maintain the keya in ascending order in the data list. The other is DataStructure_getData, which gets the data with certain key and remove it from the data list. If the key has is not in the list, NULL will be returned. Everytime those two methods manipulating the hash table, they should first acquire the mutex lock, then do the operation and release the lock when finished.
    5. The DataStructure_peekNext is returning the next key that is unprocessed. If the list is empty, NULL will be returned.

Time Complexity of MR_Emit:
    O(n). Since the data list should be kept in ascending order, before new data is added to the list, it should first find the position to insert ( by traversing the list). 

Time complexity of MR_GetNext:
    O(1). Since the data list is in ascending order, we can simply pop data from the head of the list.

Design of the threadpool:
    1. Implementation of the task queue.
        a. The task queue is implememted by using queue in C++ STL, which is a first-in-first-out data structure.
    2. The threadpool contains the following members: 
        a. A thread list contains all the thread ids
        b. A boolean closing indicating whether ThreadPool_destroy() has been called.
        c. A counter indicating the number of active threads that are working on the tasks
        d. Two mutex locks. One for the task queue. The other for the counter.
        e. Two condition variables. One for the condition to pick up work from the task queue. One for the condition to destroy the threadpool.
        f. A task queue mentioned in 1. .
    3. Implementation:
        a. ThreadPool_t *ThreadPool_create(int num); will initialize all the members and create a series of threads running.
        b. void ThreadPool_destroy(ThreadPool_t *tp); will block until the task queue is empty as well as there are no threads processing the works.
        c. bool ThreadPool_add_work(ThreadPool_t *tp, thread_func_t func, void *arg); will add work to the task queue. It should first acqueue the queue mutex lock before modifying the task queue and reslease it upon completion.
        d. ThreadPool_work_t *ThreadPool_get_work(ThreadPool_t *tp); will return the work from task queue. Synchronization on the task queue is required.
        e. void *Thread_run(void *tp); Every thread will run this function. The thread will be blocked if there is no work in the work queue and the thread will finish if ThreadPool_destroy() is called. 
    4. synchronization primitives:
        mutex and condition variables are used for threadpool library.

Test strategies:
    1. One work and one thread.
    2. Multiple works and one thread.
    3. one work and multiple threads.
    4. Multiple workds and multiple threads.
    5. Run each of the above cases several times and see if they produce different results or causing deadlocks.

    Test wordcount: run with same input files a bunch of times and check if all the results are consistent.

References:
    1. CMPUT 379 course slides.
    2. APUE
    3. How to use stat() system call: https://stackoverflow.com/questions/9367528 getting-the-length-of-a-file-using-stat-in-c
    4. sort a vector of structs: https://stackoverflow.com/questions/4892680/sorting-a-vector-of-structs
    5. hash function: from the assignment description.