#include "mapreduce.h"
#include "threadpool.h"
#include <vector>
#include <list>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <sys/stat.h>

using namespace std;

/**
 * Shared Data Structure definition
 * */
typedef struct {
    char *key;
    char *value;
} Data;

typedef struct {
    pthread_mutex_t lock;
    list<Data*> l;
    list<Data*>::iterator current;
} DataList_t;

typedef struct
{
    vector<DataList_t> dataListContainer;

} DataStructure;

/*** Create the data structure and return its pointer ***/
DataStructure * DataStructure_create(int partitions){
    DataStructure *ds = new DataStructure();
    for (long i = 0 ; i < partitions; i ++){
        DataList_t datalist;

        ds->dataListContainer.push_back(datalist);
        pthread_mutex_init(&ds->dataListContainer[i].lock,NULL); // initialize the lock for every partition
        
    }
    
    return ds;
}

/*** insert data into shared data structure ***/
void DataStructure_addData(DataStructure* ds, long partition, char* key, char* value){
    
    Data* newData = new Data();
    newData->key = new char[128];
    newData->value = new char[128];
    strcpy(newData->key, key);
    strcpy(newData->value, value);

    pthread_mutex_lock(&ds->dataListContainer[partition].lock);
      
        /*** insert the data in acsending order ***/
        list<Data*> *l = &ds->dataListContainer[partition].l;
        list<Data*>::iterator it = l->begin();
        
        while (it != l->end()){
            
            Data* data = *it;
            if (strcmp(data->key, key )<= 0){
                it++;
            }
            else{
                l->insert(it,newData);
                break;
            }
        }
        if (it == l->end()){
            l->insert(it, newData);
        }
        
    pthread_mutex_unlock(&ds->dataListContainer[partition].lock);
}

/*** get the next data and remove it from list ***/
Data* DataStructure_getData(DataStructure *ds, long partition, char* key){
    
    Data* data = NULL;
    pthread_mutex_lock(&ds->dataListContainer[partition].lock);
        
        if (ds->dataListContainer[partition].current != ds->dataListContainer[partition].l.end()
            && strcmp(ds->dataListContainer[partition].l.front()->key, key) == 0){

            data = ds->dataListContainer[partition].l.front();
            ds->dataListContainer[partition].current++;
        }
        
    pthread_mutex_unlock(&ds->dataListContainer[partition].lock);
    
    return data;
}

/*** peek the next data to be processed ***/
char *DataStructure_peekNext(DataStructure*ds, long partition){
    char* key = NULL;
    pthread_mutex_lock(&ds->dataListContainer[partition].lock);
        
    if (ds->dataListContainer[partition].current != ds->dataListContainer[partition].l.end()){
        Data* data = *ds->dataListContainer[partition].current;
        key = data->key;
    }
    pthread_mutex_unlock(&ds->dataListContainer[partition].lock);

    return key;
}


/*** free the memory ***/
void DataStructure_destroy(DataStructure* ds){
    for (unsigned long i = 0 ; i < ds->dataListContainer.size(); i ++){
        list<Data*> *l = &ds->dataListContainer[i].l;
        for (list<Data*>::iterator it = l->begin(); it != l->end(); it++){
            Data* data = *it;
            delete data->key;
            delete data->value;
            delete data;
        }
    }
    delete ds;
}

struct file {
    char* filename;
    long long filesize;
};

/*** Global variables ***/
int M;
int R;
Reducer reducer;
DataStructure *ds;


bool compare(struct file &f1, struct file &f2){
    return f1.filesize > f2.filesize;
}

void MR_Run(int num_files, char *filenames[],Mapper map, int num_mappers,Reducer concate, int num_reducers){
    
    /*** Initializing globals ***/
    M = num_mappers;
    R = num_reducers;
    reducer = concate;
    ds = DataStructure_create(num_reducers);


    /*** sort the files by size ***/ 
    vector<struct file> files;

    for (int i=0; i < num_files; i ++){
        struct stat st;
        
        if (stat(filenames[i], &st)){ // error in get the stat of the file
            continue;
        }

        struct file f = {filenames[i], st.st_size};
        files.push_back(f);
    }

    sort(files.begin(),files.end(),compare);


    ThreadPool_t *mappers = ThreadPool_create(num_mappers);

    for (unsigned int i = 0 ; i < files.size(); i ++){

        if (ThreadPool_add_work(mappers, (thread_func_t) map, files[i].filename) == false){
            
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

    DataStructure_destroy(ds);
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

    //initialize iterator
    ds->dataListContainer[*partition_number].current = ds->dataListContainer[*partition_number].l.begin();
    while ((key = DataStructure_peekNext(ds, *partition_number)) != NULL ){
        reducer(key,*partition_number);
    }

    delete partition_number;
}

char *MR_GetNext(char *key, int partition_number){
    
    if (key == NULL){
        return NULL;
    }

    Data *data =  DataStructure_getData(ds, partition_number, key);

    if (data != NULL){
        return data->value;
    }
    
    return NULL;
}