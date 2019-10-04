//
// Created by Cijie Xia on 2019-09-21.
//

#ifndef ASSIGNMENT1_GLOBAL_H
#define ASSIGNMENT1_GLOBAL_H

#include <string>
#include <map>
#include <unistd.h>

using namespace std;

/*** The ENV PATH ***/
extern string PATH;

/**** contains all the processes the parent process creates ***/
/**** key: pid, value: if the process is active ****/
extern map<pid_t, bool> processPool;

extern string myfifo;
#endif //ASSIGNMENT1_GLOBAL_H
