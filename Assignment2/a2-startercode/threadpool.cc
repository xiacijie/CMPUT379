#include "threadpool.h"
#include <iostream>

using namespace std;


ThreadPool_t *ThreadPool_create(int num) {
    ThreadPool_t *newThreadPool = (ThreadPool_t *)malloc(sizeof(ThreadPool_t));

    if (newThreadPool == NULL) {
        cerr << "Fail to create new thread pool!" << endl;
        exit(1);
    }

    pthread_mutex_init(&newThreadPool->lock, NULL);
    pthread_cond_init(&newThreadPool->condition, NULL);

    /*** create #num worker threads ***/
    for (int i = 0; i < num; i ++) {
        pthread_t tid;
        pthread_create(&tid, NULL, Thread_run, newThreadPool);
    }

    return newThreadPool;
}

void *Thread_run(void* tp) {
    ThreadPool_t *threadPool = (ThreadPool_t *)tp;
    
    
    while (true) {
        
        //pthread_mutex_lock(&threadPool->lock);
        
        while (threadPool->workQueue.queue.size()==0){ // work queue is empty, waiting for ThreadPool_add_work to add work
            
            pthread_cond_wait(&threadPool->condition, &threadPool->lock);
        }
        
        ThreadPool_work_t work = ThreadPool_get_work(threadPool);
        printf("Getting a work!!!\n");




        //pthread_mutex_unlock(&threadPool->lock);
    }
}

void ThreadPool_destroy(ThreadPool_t *tp) {
    pthread_mutex_destroy(&tp->lock);
    pthread_cond_destroy(&tp->condition);
    free(tp);
}


bool ThreadPool_add_work(ThreadPool_t *tp, thread_func_t func, void *arg) {
    pthread_mutex_lock(&tp->lock);

    ThreadPool_work_t work = {func, arg};

    tp->workQueue.queue.push_back(work);
    pthread_cond_signal(&tp->condition); //signal the sleeping thread waiting for work

    pthread_mutex_unlock(&tp->lock);
    
}

ThreadPool_work_t ThreadPool_get_work(ThreadPool_t *tp){

    pthread_mutex_lock(&tp->lock);

    ThreadPool_work_t work = tp->workQueue.queue.back();
    tp->workQueue.queue.pop_back();

    pthread_mutex_unlock(&tp->lock);

    return work;
}

