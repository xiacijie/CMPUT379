#include "commandRouter.h"
#include "commands.h"
#include "global.h"
#include "commandHandler.h"
#include "backgroundHandler.h"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>


using namespace std;

int route(vector<string> words, bool isBackgroundJob) {

    if (words.size() < 1) { //in case commands is empty
        return 0;
    }

    string firstWord = words[0];
    auto it = commandsMap.find(firstWord);

    /**** internal commands ****/
    if (it != commandsMap.end()) {
        switch (it->second) {
            case pwd:
                cmdPwd();
                break;
            case cd:
                cmdCd(words);
                break;
            case exit1:
                cmdExit();
                return 1;
                break;
            case path:
                cmdPath();
                break;
            case a2path:
                cmdA2path(words);
                break;
            default:
                break;
        }
    }

    /**** external commands ****/
    else{
        pid_t pid = fork();
        if (pid < 0){
            cerr << "Error in forking process!" << endl;
        }

        if (pid > 0) { // parent process
            processPool[pid]  = true;
            if (isBackgroundJob){
                hideOutput();
            }
            else{
                waitpid(pid,NULL,0);
                processPool[pid] = false;
            }
        }
        else { // child process
            cmdExternal(words);
        }
        
    }
    return 0;
    
}

void pipeRoute(vector<string> words1, vector<string> words2){

    

    pid_t pid = fork();
    if (pid < 0){
        cerr << "Error in forking process" << endl;
    }

    if (pid > 0){ // parent
        processPool[pid] = true;
        waitpid(pid,NULL,0);
        processPool[pid] = false;
        
    }
    else { //child

        pid_t pid2 = fork();

        int fd[2];

        if (pipe(fd) < 0){
            cerr << "Pipe error!" <<endl;
        }
        if (pid2 < 0){
            cerr << "Error in forking process" << endl;
        }

        if (pid2 > 0) { // inner parent
            processPool[pid2] = true;
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            cmdExternal(words1);
        }
        else { // inner child
            close(fd[1]);
            dup2(fd[0],STDIN_FILENO);
            close(fd[0]);
            cmdExternal(words2);
        }
    }
}