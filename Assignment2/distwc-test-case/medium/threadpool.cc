#include "threadpool.h"
#include <iostream>

using namespace std;

// threads will start running after this function is called
ThreadPool_t *ThreadPool_create(int num) {
    ThreadPool_t *newThreadPool = new ThreadPool_t();

    if (newThreadPool == NULL) {
        cerr << "Fail to create new thread pool!" << endl;
        exit(1);
    }

    pthread_mutex_init(&newThreadPool->queueLock, NULL); // the lock for the work queue
    pthread_mutex_init(&newThreadPool->counterLock, NULL); // the lock for the counter
    
    pthread_cond_init(&newThreadPool->workingCondition, NULL); // condition indicates if the threadpool should start picking up works
    pthread_cond_init(&newThreadPool->stopingCondition, NULL); // condition indicates if the threadpool should stop



    newThreadPool->closing = false; // indicates whether destroy threadpool is called
    newThreadPool->counter = 0; // indicates the # of active running threads that are processing works

    /*** create #num worker threads ***/
    for (int i = 0; i < num; i ++) {

        pthread_t tid;
        newThreadPool->threads.push_back(tid);
        pthread_create(&newThreadPool->threads.back(), NULL, &Thread_run, newThreadPool);
    }

    return newThreadPool;
}

//running the thread infinitely until threadpool destroy function is called
void *Thread_run(void* tp) {
    ThreadPool_t *threadPool = (ThreadPool_t *)tp;
    
    while (true) {
        
        ThreadPool_work_t* work = ThreadPool_get_work(threadPool); // pick up the work from queue, if empty, block until there is work
        if (work != NULL){
            
            pthread_mutex_lock(&threadPool->counterLock);
                threadPool->counter += 1; 
            pthread_mutex_unlock(&threadPool->counterLock);

            work->func(work->arg); // processing the work

            pthread_mutex_lock(&threadPool->counterLock);
                threadPool->counter -= 1;
                pthread_cond_signal(&threadPool->stopingCondition); // signal the blocked destroy threadpool function 
            pthread_mutex_unlock(&threadPool->counterLock);
            
            delete work;
        }

        if (threadPool->closing == true){ // if the threadpool destroy is called, quit this thread. 
            break;
        } 

    }
    
    pthread_exit(NULL);
}

void ThreadPool_destroy(ThreadPool_t *tp) {

    // block until the queue is empty
    pthread_mutex_lock(&tp->queueLock);

        while (tp->workQueue.workQueue.size() != 0) { 
            pthread_cond_wait(&tp->stopingCondition, &tp->queueLock);
        }

    pthread_mutex_unlock(&tp->queueLock);

    // block until all ongoing tasks are done( counter is 0)
    pthread_mutex_lock(&tp->counterLock);

        while(tp->counter != 0){
            pthread_cond_wait(&tp->stopingCondition,&tp->counterLock);
        }

    pthread_mutex_unlock(&tp->counterLock);

    tp->closing = true;
    pthread_cond_broadcast(&tp->workingCondition);


    //join all the threads
    for (unsigned int i = 0 ; i < tp->threads.size(); i++){
        pthread_join(tp->threads[i],NULL);
    }

    //destory threads and condition variables
    pthread_mutex_destroy(&tp->queueLock);
    pthread_mutex_destroy(&tp->counterLock);
    pthread_cond_destroy(&tp->workingCondition);
    pthread_cond_destroy(&tp->stopingCondition);
    delete tp;
}


bool ThreadPool_add_work(ThreadPool_t *tp, thread_func_t func, void *arg) {
    pthread_mutex_lock(&tp->queueLock);

        ThreadPool_work_t* work = new ThreadPool_work_t();
        if (work == NULL){
            cerr << "Fail to create work!" << endl;
            pthread_mutex_unlock(&tp->queueLock);
            return false;
        }

        work->func = func;
        work->arg = arg;

        tp->workQueue.workQueue.push(work);
        
        pthread_cond_broadcast(&tp->workingCondition); //signal the sleeping thread waiting for work

    pthread_mutex_unlock(&tp->queueLock);
    return true;
    
}

ThreadPool_work_t *ThreadPool_get_work(ThreadPool_t *tp){
    
    pthread_mutex_lock(&tp->queueLock);

        while (tp->workQueue.workQueue.size()==0 && tp->closing == false){ // work queue is empty, waiting for ThreadPool_add_work to add work
            
            pthread_cond_wait(&tp->workingCondition, &tp->queueLock);
        }

        if (tp->workQueue.workQueue.size()==0 && tp->closing == true){   // if ThreadPool_destroy() is called, release the lock
            pthread_mutex_unlock(&tp->queueLock);
            return NULL;
        }

        ThreadPool_work_t* work = tp->workQueue.workQueue.front();
        tp->workQueue.workQueue.pop();

        

    pthread_mutex_unlock(&tp->queueLock);

    return work;
}

