#include "mapreduce.h"
#include "threadpool.h"
#include <unordered_map>
#include <list>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <sys/stat.h>

using namespace std;

/**
 * Shared Data Structure definition
 * */
struct Node{
    char *key;
    char *value;
    Node *next;
};

typedef struct {
    pthread_mutex_t lock;
    Node* head;
    Node* processing;
} DataList_t;

typedef struct
{
    unordered_map<long, DataList_t> hashTable;

} DataStructure;

/*** Create the data structure and return its pointer ***/
DataStructure * DataStructure_create(int partitions){
    DataStructure *ds = new DataStructure();
    for (long i = 0 ; i < partitions; i ++){
        DataList_t datalist;
        datalist.head = NULL;

        ds->hashTable[i] = datalist;
        pthread_mutex_init(&ds->hashTable[i].lock,NULL); // initialize the lock for every partition
        
    }
    
    return ds;
}

/*** insert data into shared data structure ***/
void DataStructure_addData(DataStructure* ds, long partition, char* key, char* value){
    
    Node* newNode = new Node();
    newNode->key = new char[128];
    newNode->value = new char[128];
    newNode->next = NULL;
    strcpy(newNode->key, key);
    strcpy(newNode->value, value);

    pthread_mutex_lock(&ds->hashTable[partition].lock);

            
        /*** insert the data in acsending order ***/
        Node* current = ds->hashTable[partition].head;
        Node* prev = NULL;
       
        if (current == NULL){
           
            ds->hashTable[partition].head = newNode;
        }
        else{
            while (current != NULL){
                
                Node* node = current;
                
                if (strcmp(node->key, key )< 0){
                    prev = current;
                    current = current->next;
                }
                else{
                    if (prev == NULL){
                        ds->hashTable[partition].head = newNode;
                        newNode->next = current;
                    }
                    else{
                        newNode->next = current;
                        prev->next = newNode;
                    }
                    break;
                }
            }
            if (current == NULL){ //tail
                prev->next = newNode;
            }
        }
        
    pthread_mutex_unlock(&ds->hashTable[partition].lock);
}

/*** get the next data and remove it from list ***/
Node* DataStructure_getData(DataStructure *ds, long partition, char* key){
    
    Node* node = NULL;
    pthread_mutex_lock(&ds->hashTable[partition].lock);
        
        if (ds->hashTable[partition].processing != NULL && strcmp(ds->hashTable[partition].processing->key, key) == 0){
            node = ds->hashTable[partition].processing;
            ds->hashTable[partition].processing = ds->hashTable[partition].processing->next; //forward the processing by one
        }
        
    pthread_mutex_unlock(&ds->hashTable[partition].lock);
    
    return node;
}

/*** peek the next data to be processed ***/
char *DataStructure_peekNext(DataStructure*ds, long partition){
    char* key = NULL;
    pthread_mutex_lock(&ds->hashTable[partition].lock);
        
        if (ds->hashTable[partition].processing != NULL){
            key = ds->hashTable[partition].processing->key;
        }
    pthread_mutex_unlock(&ds->hashTable[partition].lock);

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
    
    //set the processing of each data list
    ds->hashTable[*partition_number].processing = ds->hashTable[*partition_number].head;

    while ((key = DataStructure_peekNext(ds, *partition_number)) != NULL ){
        reducer(key,*partition_number); 
    }

    //free the meomry of each partition
    Node* head = ds->hashTable[*partition_number].head;

    while (head){
        Node *data = head;
        head = head->next;
        delete data->key;
        delete data->value;
        delete data;
    }

    delete partition_number;
}

char *MR_GetNext(char *key, int partition_number){
    
    if (key == NULL){
        return NULL;
    }

    Node *node =  DataStructure_getData(ds, partition_number, key);

    if (node != NULL){
        return node->key;
    }
    
    return NULL;
}