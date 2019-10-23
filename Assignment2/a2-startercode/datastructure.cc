#include "datastructure.h"

/*** Create the data structure and return its pointer ***/
DataStructure * DataStructure_create(){
    DataStructure *ds = new DataStructure();
    pthread_mutex_init(&ds->lock,NULL);
    return ds;
}

/*** insert data into shared data structure ***/
void DataStructure_addData(DataStructure* ds, long partition, char* key, char* value){
    Data* data = new Data();
    data->key = key;
    data->value = value;

    pthread_mutex_lock(&ds->lock);
        ds->hashTable[partition].push_back(data);
    pthread_mutex_unlock(&ds->lock);
}

/*** get the next data ***/
Data* DataStructure_getData(DataStructure *ds, long partition){
    
    Data* data = NULL;
    pthread_mutex_lock(&ds->lock);
        if (ds->hashTable[partition].size() > 0){
            data = ds->hashTable[partition].back();
            ds->hashTable[partition].pop_back();
        }
    pthread_mutex_unlock(&ds->lock);
    
    return data;
}


/*** free the memory ***/
void DataStructure_destroy(DataStructure* ds){
    delete ds;
}