#include "mapreduce.h"
#include "threadpool.h"
#include <unordered_map>
#include <vector>
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

typedef struct
{
    unordered_map<long, vector<Data*>> hashTable;
    pthread_mutex_t lock;

} DataStructure;

/*** Create the data structure and return its pointer ***/
DataStructure * DataStructure_create(){
    DataStructure *ds = new DataStructure();
    pthread_mutex_init(&ds->lock,NULL);
    return ds;
}

/*** insert data into shared data structure ***/
void DataStructure_addData(DataStructure* ds, long partition, char* key, char* value){
    
    Data* newData = new Data();
    newData->key = new char[128];
    newData->value = new char[128];
    strcpy(newData->key, key);
    strcpy(newData->value, value);

    pthread_mutex_lock(&ds->lock);
        auto it = ds->hashTable.find(partition);
        if (it == ds->hashTable.end()){
            
            ds->hashTable[partition].push_back(newData);
        }
        else{
            
            /*** insert the data in acsending order ***/
            vector<Data*> *v = &ds->hashTable[partition];
            vector<Data*>::iterator it = v->begin();
            
            while (it != v->end()){
                
                Data* data = *it;
                if (strcmp(data->key, key )<= 0){
                    it++;
                }
                else{
                    v->insert(it,newData);
                    break;
                }
            }
            if (it == v->end()){
                v->insert(it, newData);
            }

        }
        
    pthread_mutex_unlock(&ds->lock);
}

/*** get the next data and remove it from list ***/
Data* DataStructure_getData(DataStructure *ds, long partition, char* key){
    
    Data* data = NULL;
    pthread_mutex_lock(&ds->lock);
        
        if (ds->hashTable[partition].size() > 0 && strcmp(ds->hashTable[partition][0]->key, key) == 0){
            data = ds->hashTable[partition][0];
            ds->hashTable[partition].erase(ds->hashTable[partition].begin()); //remove the data
        }
        
    pthread_mutex_unlock(&ds->lock);
    
    return data;
}

/*** peek the next data to be processed ***/
char *DataStructure_peekNext(DataStructure*ds, long partition){
    char* key = NULL;
    pthread_mutex_lock(&ds->lock);
        
        if (ds->hashTable[partition].size() != 0){
            key = new char[128];
            strcpy(key,ds->hashTable[partition][0]->key);
        }
    pthread_mutex_unlock(&ds->lock);

    return key;
}


/*** free the memory ***/
void DataStructure_destroy(DataStructure* ds){
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
    ds = DataStructure_create();


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
    
    while ((key = DataStructure_peekNext(ds, *partition_number)) != NULL ){
        reducer(key,*partition_number);
        delete key;
    }

    
    delete partition_number;
}

/*** WARNING: whoever uses this method should free the memory of the pointer it returns ***/
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