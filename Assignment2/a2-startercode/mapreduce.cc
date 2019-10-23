#include "mapreduce.h"
#include "threadpool.h"
#include <iostream>
#include <stdio.h>

using namespace std;

/*** Global variables ***/
int M;
int R;

void MR_Run(int num_files, char *filenames[],Mapper map, int num_mappers,Reducer concate, int num_reducers){
    
    M = num_mappers;
    R = num_reducers;
    
    ThreadPool_t *tp = ThreadPool_create(num_mappers);

    for (int i = 0 ; i < num_files; i ++){
        if (ThreadPool_add_work(tp, (thread_func_t) map, filenames[i]) == false){
            cerr << "Fail to add work" << endl;
            exit(1);
        }
    }
    ThreadPool_destroy(tp);
}

void MR_Emit(char *key, char *value){
    cout << key << " : " << value << "|" << MR_Partition(key,R) << endl;
}

unsigned long MR_Partition(char *key, int num_partitions){
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0'){
        hash = hash * 33 + c;
    }
    return hash % num_partitions;
}

void MR_ProcessPartition(int partition_number){

}

char *MR_GetNext(char *key, int partition_number){
    return NULL;
}