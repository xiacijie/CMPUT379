#include <fcntl.h>
#include <unistd.h>
#include<iostream>
#include <sys/types.h>
#include <signal.h>
#include "backgroundHandler.h"
#include "global.h"

using namespace std;

void hideOutput(){
    int fd = open("/dev/null",O_WRONLY);
    if (fd == -1){
        cerr << "Fail to open /dev/null" << endl;
        return;
    }

    /*** redirect STDOUT to /dev/null ***/
    if (dup2(fd, STDOUT_FILENO) == -1) {
        cerr << "Dup2 failed" << endl;
        return;
    }
    close(fd);
}

void terminateAllChildProcesses() {
    for (auto it = processPool.begin(); it!= processPool.end(); it++) {
        if (it->second == true) { // if the process is not claimed by parent's wait()
            kill(it->first, SIGTERM); // kill this process
        }
    }
}