#include "commandRouter.h"
#include "commands.h"
#include "global.h"
#include "commandHandler.h"
#include "backgroundHandler.h"
#include "redirectionHandler.h"
#include "util.h"
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
                 cout << "PID " << pid << " is running in the background" <<endl;
            }
            else{
                waitpid(pid,NULL,0);
                processPool[pid] = false;
            }
        }
        else { // child process

            if (isBackgroundJob){
                hideOutput();
            }
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
        int fd[2];
        if (pipe(fd) < 0){
            cerr << "Pipe error!" <<endl;
        }
        pid_t pid2 = fork();

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

int singleCommand(string command){
  /*** save stdout, stdin, stderr fds, restore them later in case of redirection or pipes ***/
  int stdoutFdSave = dup(STDOUT_FILENO); 
  int stderrFdSave = dup(STDERR_FILENO);
  int stdinFdSave = dup(STDIN_FILENO);

  /*** check if need redirection ***/
  if (command.find(">") != string::npos){
    handleRedirection(&command);
  }

  /*** check if it is a background job ***/
  bool isBackgoroundJob = false;
  if (command.back() == '&'){
    command.erase(command.end()-1); /*** delete & ***/
    isBackgoroundJob = true;
  }

  /****** tokenize the command *****/
  vector<string> words =  tokenize(command, " ");

  /******* command router for handling a single command, internal or external ******/
  if (route(words,isBackgoroundJob)){
    return 1; //exit
  }

   /*** restore stdout, stderr, stdin ***/
    dup2(stdoutFdSave, STDOUT_FILENO);
    dup2(stderrFdSave,STDERR_FILENO);
    dup2(stdinFdSave, STDIN_FILENO);
    close(stdoutFdSave);
    close(stderrFdSave);
    close(stdinFdSave);

  return 0;
}

void pipeCommand(string comand1, string command2){

  int stdoutFdSave = dup(STDOUT_FILENO); 
  int stdinFdSave = dup(STDIN_FILENO);

  pipeRoute(tokenize(comand1," "), tokenize(command2," "));

  /*** restore stdout stdin ***/
  dup2(stdoutFdSave, STDOUT_FILENO);
  dup2(stdinFdSave, STDIN_FILENO);
  close(stdoutFdSave);
  close(stdinFdSave);

}