#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#include "signalHandler.h"

void registerSignalHandlers(){
    if (signal(SIGINT, handleSig) == SIG_ERR){
        perror("Error: Cannot handle signal SIGINT!\n");
    }

    if (signal(SIGTSTP, handleSig) == SIG_ERR){
        perror("Error: Cannot handle signal SIGTSTP!\n");
    }

}

void handleSig(int sigNum){
    write(STDOUT_FILENO, "\ndragon shell > ",16);
    return;
}