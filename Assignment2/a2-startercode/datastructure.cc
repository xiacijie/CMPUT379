#include "datastructure.h"

/*** Create the data structure and return its pointer ***/
DataStructure * DataStructure_create();

/*** insert data into shared data structure ***/
void DataStructure_addData(long partition, char* key, char* value);

/*** get the next data ***/
Data DataStructure_getData(long partition);


/*** free the memory ***/
void DataStructure_destroy(DataStructure* ds);