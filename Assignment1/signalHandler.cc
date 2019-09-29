//
// Created by Cijie Xia on 2019-09-21.
//
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include "SignalHandler.h"
#include "signal.h"
#include "global.h"
#include "commandRouter.h"


using namespace std;
void registerSignalHandlers(){
    if (signal(SIGINT, handleSig) == SIG_ERR){
        cerr << "Error: Cannot handle signal SIGINT!" << endl;
    }

    if (signal(SIGTSTP, handleSig) == SIG_ERR){
        cerr << "Error: Cannot handle signal SIGTSTP!" << endl;
    }

}

void handleSig(int sigNum) {
    for (auto it = processPool.begin(); it!= processPool.end(); it++) {
        if (it->second == true) { // if the process is not claimed by parent's wait()
            kill(it->first, sigNum); // send signal to it
        }
    }

    /*** go to a new line in dragonshell and return control to user***/
    vector<string> emptyCommands;
    write(STDOUT_FILENO, "\ndragon shell > ",16);
    
}