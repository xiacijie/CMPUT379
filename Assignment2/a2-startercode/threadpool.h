#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <pthread.h>
#include <stdlib.h>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

typedef void (*thread_func_t)(void *arg);

typedef struct ThreadPool_work_t {
    thread_func_t func;              // The function pointer
    void *arg;                       // The arguments for the function
    // TODO: Add other members here if needed
} ThreadPool_work_t;

typedef struct {
    // TODO: Add members here
    vector<ThreadPool_work_t*> queue;

} ThreadPool_work_queue_t;

typedef struct {
    // TODO: Add members here
    bool closing;
    vector<pthread_t> threads;
    int counter;
    pthread_mutex_t queueLock;
    pthread_mutex_t counterLock;
    pthread_cond_t runningCondition;
    pthread_cond_t closingCondition;
    ThreadPool_work_queue_t workQueue;

} ThreadPool_t;


/**
* A C style constructor for creating a new ThreadPool object
* Parameters:
*     num - The number of threads to create
* Return:
*     ThreadPool_t* - The pointer to the newly created ThreadPool object
*/
ThreadPool_t *ThreadPool_create(int num);

/**
* A C style destructor to destroy a ThreadPool object
* Parameters:
*     tp - The pointer to the ThreadPool object to be destroyed
*/
void ThreadPool_destroy(ThreadPool_t *tp);

/**
* Add a task to the ThreadPool's task queue
* Parameters:
*     tp   - The ThreadPool object to add the task to
*     func - The function pointer that will be called in the thread
*     arg  - The arguments for the function
* Return:
*     true  - If successful
*     false - Otherwise
*/
bool ThreadPool_add_work(ThreadPool_t *tp, thread_func_t func, void *arg);

/**
* Get a task from the given ThreadPool object
* Parameters:
*     tp - The ThreadPool object being passed
* Return:
*     ThreadPool_work_t* - The next task to run
*/
ThreadPool_work_t *ThreadPool_get_work(ThreadPool_t *tp);

/**
* Run the next task from the task queue
* Parameters:
*     tp - The ThreadPool Object this thread belongs to
*/
void *Thread_run(void *tp);


#ifdef __cplusplus
}
#endif
#endif
