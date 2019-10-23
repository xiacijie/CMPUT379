#include <unordered_map>
#include <vector>
#include <pthread.h>

using namespace std;

typedef struct {
    char *key;
    char *value;
} Data;

typedef struct
{
    unordered_map<int, vector<Data>> hashTable;
    pthread_mutex_t lock;

} DataStructure;

/*** Create the data structure and return its pointer ***/
DataStructure * DataStructure_create();

/*** insert data into shared data structure ***/
void DataStructure_addData(long partition, char* key, char* value);

/*** get the next data ***/
Data DataStructure_getData(long partition);


/*** free the memory ***/
void DataStructure_destroy(DataStructure* ds);
