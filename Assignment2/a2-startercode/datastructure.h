#include <unordered_map>
#include <vector>
#include <map>
#include <pthread.h>

using namespace std;

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
DataStructure * DataStructure_create();

/*** insert data into shared data structure ***/
void DataStructure_addData(DataStructure* ds, long partition, char* key, char* value);

/*** return the next key to be processes, if none, return NULL ***/
char* DataStructure_peekNext(DataStructure *ds, long partition);

/*** get the next data by key ***/
Data* DataStructure_getData(DataStructure* ds, long partition, char* key);


/*** free the memory ***/
void DataStructure_destroy(DataStructure* ds);
