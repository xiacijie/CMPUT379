#include "pipeHandler.h"
#include <vector>
#include <unistd.h>
#include <iostream>
#include "global.h"

using namespace std;

void handlePipe(){
   if (pipe(fd) < 0){
       cerr << "pipe error" << endl;
   }
}
void leftPipe(){
    close(fd[0]); 
    dup2(fd[1], STDOUT_FILENO);
    close(fd[1]); 
}


void rightPipe(){
    close(fd[1]); // child won’t write
    dup2(fd[0], STDIN_FILENO); // stdin = fd[0]
    close(fd[0]); // stdin is still open 
}