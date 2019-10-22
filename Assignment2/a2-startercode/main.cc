#include "threadpool.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void* foo(void *arg){
    printf("%d\n", *((int *) arg));
    
}

int main(){
    ThreadPool_t *tp = ThreadPool_create(10);
    
    int i = 998;
    ThreadPool_add_work(tp, (thread_func_t)foo, &i);
    ThreadPool_add_work(tp, (thread_func_t)foo, &i);
    //sleep(1);
    ThreadPool_destroy(tp);
    return 0;
}