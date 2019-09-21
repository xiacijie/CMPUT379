#include "commandHandler.h"
#include <iostream>
#include <unistd.h>

#define BUFFER_SIZE 1024

using namespace std;

void cmd_pwd() {
    char path[BUFFER_SIZE];
    cout << getcwd(path,BUFFER_SIZE) << endl;
}