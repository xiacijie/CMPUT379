#include "pipeHandler.h"
#include <vector>
#include <unistd.h>
#include <iostream>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <fcntl.h>
#include "global.h"

using namespace std;

void handlePipe(){
    mkfifo(myfifo.c_str(),0666);
}
void leftPipe(){
    int fd = open(myfifo.c_str(),O_WRONLY);
    dup2(fd,STDOUT_FILENO);
    close(fd);

}


void rightPipe(){
    int fd = open(myfifo.c_str(),O_RDONLY);
    dup2(fd,STDIN_FILENO);
    close(fd);

}