#include "threadpool.h"
#include <iostream>

using namespace std;

/*** Warning: remember to free the memory after use ***/
ThreadPool_t *ThreadPool_create(int num) {
    ThreadPool_t *newThreadPool = (ThreadPool_t *)malloc(sizeof(ThreadPool_t));

    if (newThreadPool == NULL) {
        cerr << "Fail to create new thread pool!" << endl;
        exit(1);
    }

    for (u_int32_t i = 0 ; i < num ; i ++ ){
        pthread_t tid;
        newThreadPool->workers.push_back(tid);
    }

    newThreadPool->lock = PTHREAD_MUTEX_INITIALIZER;

    return newThreadPool;
}


void ThreadPool_destroy(ThreadPool_t *tp) {
    free(tp);
}


bool ThreadPool_add_work(ThreadPool_t *tp, thread_func_t func, void *arg) {
    
}

