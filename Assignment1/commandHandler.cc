#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "commandHandler.h"
#include "global.h"
#include "util.h"
#include "backgroundHandler.h"



#define BUFFER_SIZE 1024

using namespace std;

void cmdPwd() {
    char path[BUFFER_SIZE];
    cout << getcwd(path,BUFFER_SIZE) << endl;
}

void cmdCd(vector<string> words) {
    if (words.size() < 2){
        cout << "dragonshell: expected argument to 'cd' " << endl;
        return;
    }

    string path = words[1];
    if (chdir(path.c_str()) == -1) {
        cout << "dragonshell: No such file or directory" << endl;
        return;
    }
}

void cmdExit(){
    cout << "Exiting" << endl;
    terminateAllChildProcesses();
}

void cmdPath(){
    cout << "Current PATH: " << PATH << endl;
}

void cmdA2path(vector<string> words){
    if (words.size() == 1){ // empty path specified
        PATH = "";
        return;
    }

    string pathLine = words[1];
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

void cmdExternal(vector<string> words, bool isBackgroundJob) {
    pid_t pid = fork();
    if (pid > 0) { // parent process

        if (isBackgroundJob){
            /*** register the pid in processPool to indicate this pid is alive ***/
            processPool[pid] = true;
            cout << "PID " << pid << " is running in the background" <<endl;

        }
        else{
            processPool[pid] = true;
            waitpid(pid,NULL,0);
            processPool[pid] = false;
        }

    }
    else if (pid == 0){ // child process

        if (isBackgroundJob){
            /*** hide the output from the terminal ***/
            hideOutput();
        }

        string firstWord = words[0];
        char * argv[64];
        for (unsigned int i=0; i < words.size();i++){
            argv[i] = (char*)words[i].c_str();
        }

        argv[words.size()] = NULL;

        char * envp[] = {NULL};

        char currentDir[BUFFER_SIZE];
        string currentDirString(getcwd(currentDir,BUFFER_SIZE));
        vector<string> paths = tokenize(PATH,":");

        paths.insert(paths.begin(),""); /*** in case the user enter the full path of the command ***/
        paths.insert(paths.begin(),currentDirString); /*** in case the user want to run the command in the current Dir ***/

        for (unsigned int i = 0 ; i < paths.size(); i ++){ /***** search in all PATHs ******/
            string path = paths[i] + "/" + firstWord;
            execve((char*)path.c_str(), argv, envp);
        }

        cerr << "dragonshell: Command not found" << endl;
        _exit(1);
    }
    else{
        cout << "Error in forking the child process" << endl;
    }


}