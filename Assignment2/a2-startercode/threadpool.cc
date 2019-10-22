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
    pthread_cond_init(&newThreadPool->runningCondition, NULL);
    pthread_cond_init(&newThreadPool->closingCondition, NULL);

    newThreadPool->closing = false;

    /*** create #num worker threads ***/
    for (int i = 0; i < num; i ++) {
        pthread_t tid;
        newThreadPool->threads.push_back(tid);
        pthread_create(&tid, NULL, Thread_run, newThreadPool);
    }

    return newThreadPool;
}

void *Thread_run(void* tp) {
    ThreadPool_t *threadPool = (ThreadPool_t *)tp;
    
    
    while (true) {
        
  

        if (threadPool->closing == true){
            break;
        } 

        ThreadPool_work_t* work = ThreadPool_get_work(threadPool);
        if (work != NULL){
            printf("get a work\n");
            work->func(work->arg);
            free(work);
        }

    }

    return NULL;
}

void ThreadPool_destroy(ThreadPool_t *tp) {

    tp->closing = true;

    pthread_cond_broadcast(&tp->runningCondition);

    for (pthread_t thread : tp->threads) {
        pthread_join(thread,NULL);

    }

    
    
    pthread_mutex_destroy(&tp->lock);
    pthread_cond_destroy(&tp->runningCondition);
    pthread_cond_destroy(&tp->closingCondition);
    free(tp);
}


bool ThreadPool_add_work(ThreadPool_t *tp, thread_func_t func, void *arg) {
    pthread_mutex_lock(&tp->lock);

    ThreadPool_work_t* work = (ThreadPool_work_t *) malloc(sizeof(ThreadPool_work_t));
    if (work == NULL){
        cerr << "Fail to create work!" << endl;
        return false;
    }

    work->func = func;
    work->arg = arg;

    tp->workQueue.queue.push_back(work);
    pthread_cond_broadcast(&tp->runningCondition); //signal the sleeping thread waiting for work

    pthread_mutex_unlock(&tp->lock);
    
}

ThreadPool_work_t *ThreadPool_get_work(ThreadPool_t *tp){
    printf("get work\n");
    pthread_mutex_lock(&tp->lock);

    while (tp->workQueue.queue.size()==0 && tp->closing == false){ // work queue is empty, waiting for ThreadPool_add_work to add work
        pthread_cond_wait(&tp->runningCondition, &tp->lock);
    }

    if (tp->closing == true){   // if ThreadPool_destroy() is called, release the lock
        pthread_mutex_unlock(&tp->lock);
        return NULL;
    }

    ThreadPool_work_t* work = tp->workQueue.queue.back();
    tp->workQueue.queue.pop_back();

    pthread_mutex_unlock(&tp->lock);

    return work;
}

