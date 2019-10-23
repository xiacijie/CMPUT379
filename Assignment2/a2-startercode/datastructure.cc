#include "datastructure.h"
#include <string.h>

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