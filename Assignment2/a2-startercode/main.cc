#include "threadpool.h"

void foo(){

}

int main(){
    ThreadPool_t *tp = ThreadPool_create(10);
    ThreadPool_add_work(tp, (thread_func_t)foo, NULL);
    ThreadPool_add_work(tp, (thread_func_t)foo, NULL);
    ThreadPool_destroy(tp);
    return 0;
}