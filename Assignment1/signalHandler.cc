//
// Created by Cijie Xia on 2019-09-21.
//
#include <iostream>
#include "SignalHandler.h"
#include "signal.h"

using namespace std;
void registerSignalHandlers(){
    if (signal(SIGINT, handleSIGINT) == SIG_ERR){
        cerr << "Error: Cannot handle signal SIGINT!" << endl;
    }

    if (signal(SIGTSTP, handleSIGTSTP) == SIG_ERR){
        cerr << "Error: Cannot handle signal SIGTSTP!" << endl;
    }

}

void handleSIGINT(int sigNum) {
    cout << endl;
}

void handleSIGTSTP(int sigNum) {
    
}