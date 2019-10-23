#include "mapreduce.h"
#include "threadpool.h"
#include "datastructure.h"
#include <iostream>
#include <stdio.h>

using namespace std;

/*** Global variables ***/
int M;
int R;
Reducer reducer;
DataStructure *ds;

void MR_Run(int num_files, char *filenames[],Mapper map, int num_mappers,Reducer concate, int num_reducers){
    
    /*** Initializing globals ***/
    M = num_mappers;
    R = num_reducers;
    reducer = concate;
    ds = DataStructure_create();
    
    ThreadPool_t *mappers = ThreadPool_create(num_mappers);

    for (int i = 0 ; i < num_files; i ++){
        if (ThreadPool_add_work(mappers, (thread_func_t) map, filenames[i]) == false){
            cerr << "Fail to add work!" << endl;
            exit(1);
        }
    }
    ThreadPool_destroy(mappers);


    ThreadPool_t *reducers = ThreadPool_create(num_reducers);
    for (int i = 0 ; i < num_reducers; i ++){
        int *partition_number = new int(i);
        if (ThreadPool_add_work(reducers, (thread_func_t)MR_ProcessPartition, partition_number) == false) {
            cerr << "Fail to add work!" << endl;
            exit(1);
        }
    }
    ThreadPool_destroy(reducers);

}

void MR_Emit(char *key, char *value){
    
    DataStructure_addData(ds,MR_Partition(key, R),key,value);
}

unsigned long MR_Partition(char *key, int num_partitions){

    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0'){
        hash = hash * 33 + c;
    }
    return hash % num_partitions;
}

void MR_ProcessPartition(int* partition_number){
    char *key;
    
    // for (unsigned int i = 0 ; i < ds->hashTable[*partition_number].size(); i ++){
    //     printf("%s | %s\n", ds->hashTable[*partition_number][i]->key,ds->hashTable[*partition_number][i]->value);
    // }
    
    while ((key = DataStructure_peekNext(ds, *partition_number)) != NULL ){
        reducer(key,*partition_number);
        delete key;
    }

    
    delete partition_number;
}

char *MR_GetNext(char *key, int partition_number){
    
    if (key == NULL){
        return NULL;
    }

    Data *data =  DataStructure_getData(ds, partition_number, key);

    char* value = NULL;
    if (data != NULL){
        value = new char[128];
        strcpy(value, data->value);

        delete data->key;
        delete data->value;
        delete data;
    }
    
    return value;
}