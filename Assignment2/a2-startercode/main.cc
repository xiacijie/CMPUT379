#include "threadpool.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void* foo(void *arg){
    sleep(*((int *) arg));
    printf("%d\n", *((int *) arg));
    return NULL;
    
}

int main(){
    ThreadPool_t *tp = ThreadPool_create(10);
    
    int i = 1;
    int j = 2;
    int k = 3;
    ThreadPool_add_work(tp, (thread_func_t)foo, &i);
    ThreadPool_add_work(tp, (thread_func_t)foo, &j);
    ThreadPool_add_work(tp, (thread_func_t)foo, &k);
    ThreadPool_add_work(tp, (thread_func_t)foo, &i);
    ThreadPool_add_work(tp, (thread_func_t)foo, &j);
    ThreadPool_add_work(tp, (thread_func_t)foo, &k);


    ThreadPool_destroy(tp);
    return 0;
}