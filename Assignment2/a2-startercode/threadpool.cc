#include "threadpool.h"
#include <iostream>

using namespace std;

// threads will start running after this function is called
ThreadPool_t *ThreadPool_create(int num) {
    ThreadPool_t *newThreadPool = (ThreadPool_t *)malloc(sizeof(ThreadPool_t));

    if (newThreadPool == NULL) {
        cerr << "Fail to create new thread pool!" << endl;
        exit(1);
    }

    pthread_mutex_init(&newThreadPool->queueLock, NULL);
    pthread_mutex_init(&newThreadPool->counterLock, NULL);
    
    pthread_cond_init(&newThreadPool->runningCondition, NULL);
    pthread_cond_init(&newThreadPool->closingCondition, NULL);

    newThreadPool->closing = false; // indicates whether destroy threadpool is called
    newThreadPool->counter = 0;
    /*** create #num worker threads ***/
    for (int i = 0; i < num; i ++) {
        pthread_t tid;
        newThreadPool->threads.push_back(tid);
        pthread_create(&tid, NULL, Thread_run, newThreadPool);
    }

    return newThreadPool;
}

//running the thread infinitely until threadpool destroy function is called
void *Thread_run(void* tp) {
    ThreadPool_t *threadPool = (ThreadPool_t *)tp;
    
    
    while (true) {
        
        ThreadPool_work_t* work = ThreadPool_get_work(threadPool);
        if (work != NULL){
            printf("processing a work...\n");

            pthread_mutex_lock(&threadPool->counterLock);
                threadPool->counter += 1;
            pthread_mutex_unlock(&threadPool->counterLock);

            work->func(work->arg);

            printf("Finish a work\n");

            pthread_mutex_lock(&threadPool->counterLock);
                threadPool->counter -= 1;
                pthread_cond_signal(&threadPool->closingCondition);
            pthread_mutex_unlock(&threadPool->counterLock);
            
            free(work);
        }

        if (threadPool->closing == true){ // if the threadpool destroy is called, quit this thread. 
            printf("quitting the thread...\n");
            pthread_cond_signal(&threadPool->closingCondition); //finish the work, signal closing condition
            break;
        } 

    }

    return NULL;
}

void ThreadPool_destroy(ThreadPool_t *tp) {

    pthread_mutex_lock(&tp->queueLock);

    while (tp->workQueue.queue.size() != 0){ // block until the queue is empty
        pthread_cond_wait(&tp->closingCondition, &tp->queueLock);
    }

    
    
    pthread_mutex_unlock(&tp->queueLock);
    printf("-----queue is empty\n");

    // block until all ongoing tasks are done
    pthread_mutex_lock(&tp->counterLock);

    while(tp->counter != 0){
        pthread_cond_wait(&tp->closingCondition,&tp->counterLock);
    }

    pthread_mutex_unlock(&tp->counterLock);

    tp->closing = true;
    pthread_cond_signal(&tp->runningCondition);


    pthread_mutex_destroy(&tp->queueLock);
    pthread_mutex_destroy(&tp->counterLock);
    pthread_cond_destroy(&tp->runningCondition);
    pthread_cond_destroy(&tp->closingCondition);
    free(tp);
}


bool ThreadPool_add_work(ThreadPool_t *tp, thread_func_t func, void *arg) {
    pthread_mutex_lock(&tp->queueLock);

    ThreadPool_work_t* work = (ThreadPool_work_t *) malloc(sizeof(ThreadPool_work_t));
    if (work == NULL){
        cerr << "Fail to create work!" << endl;
        pthread_mutex_unlock(&tp->queueLock);
        return false;
    }

    work->func = func;
    work->arg = arg;

    tp->workQueue.queue.push_back(work);
    printf("adding work...\n");
    pthread_cond_signal(&tp->runningCondition); //signal the sleeping thread waiting for work

    pthread_mutex_unlock(&tp->queueLock);
    return true;
    
}

ThreadPool_work_t *ThreadPool_get_work(ThreadPool_t *tp){
    
    pthread_mutex_lock(&tp->queueLock);

    while (tp->workQueue.queue.size()==0 && tp->closing == false){ // work queue is empty, waiting for ThreadPool_add_work to add work
        pthread_cond_wait(&tp->runningCondition, &tp->queueLock);
    }

    if (tp->workQueue.queue.size()==0 && tp->closing == true){   // if ThreadPool_destroy() is called, release the lock
        pthread_mutex_unlock(&tp->queueLock);
        return NULL;
    }

    ThreadPool_work_t* work = tp->workQueue.queue.back();
    tp->workQueue.queue.pop_back();

    pthread_cond_signal(&tp->closingCondition);
    pthread_mutex_unlock(&tp->queueLock);

    return work;
}

