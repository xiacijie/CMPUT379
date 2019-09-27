#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "commandHandler.h"
#include "global.h"
#include "util.h"
#include <sys/wait.h>


#define BUFFER_SIZE 1024

using namespace std;

void terminateAllChildProcesses() {
    for (auto it = processPool.begin(); it!= processPool.end(); it++) {
        if (it->second == true) { // if the process is active
            cout << it->first << endl;
            kill(it->first, SIGTERM); // kill this process
        }
    }
}

void cmdPwd() {
    char path[BUFFER_SIZE];
    cout << getcwd(path,BUFFER_SIZE) << endl;
}

void cmdCd(vector<string> commands) {
    if (commands.size() < 2){
        cout << "dragonshell: expected argument to 'cd' " << endl;
        return;
    }

    string path = commands[1];
    if (chdir(path.c_str()) == -1) {
        cout << "dragonshell: No such file or directory" << endl;
        return;
    }
}

void cmdExit(){
    cout << "Exiting" << endl;
    terminateAllChildProcesses();
    _exit(0);
}

void cmdPath(){
    cout << "Current PATH: " << PATH << endl;
}

void cmdA2path(vector<string> commands){
    if (commands.size() == 1){ // empty path specified
        PATH = "";
        return;
    }

    string pathLine = commands[1];
    vector<string> paths = tokenize(pathLine, ":");
    if (paths.size() < 1){
        cout << "Invalid path specified!" << endl;
        return;
    }

    //constructing new path
    string newPath = "";
    for (size_t i = 0 ; i < paths.size(); i ++){
        if (paths[i].compare("$PATH") ==  0){ //replace $PATH by real path
            newPath += PATH;
        }
        else{
            newPath += paths[i];
        }
        if (i != paths.size()-1){
            newPath += ":";
        }
    }

    PATH = newPath;
}

void cmdExternal(vector<string> commands) {
    pid_t pid = fork();
    if (pid > 0) { // parent process
        waitpid(pid,NULL,0);
    }
    else if (pid == 0){ // child process
        string firstCommand = commands[0];
        char *argv[64];
        for (unsigned int i=1; i < commands.size();i++){
            argv[i-1] = (char*)commands[i].c_str();
        }

        argv[commands.size()-1] = NULL;
        char * envp[] = {NULL};

        /**** use the path directly ****/
        execve(firstCommand.c_str(), argv, envp);

        /***** search in PATHs ******/
        vector<string> paths = tokenize(PATH,":");

        for (unsigned int i = 0 ; i < paths.size(); i ++){
            string path = paths[i] + firstCommand;
            execve(path.c_str(), argv, envp);
        }

        cout << "dragonshell: Command not found" << endl;
        _exit(1);
    }
    else{
        cout << "Error in forking the child process" << endl;
    }


}