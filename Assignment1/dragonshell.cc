#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include "util.h"
#include "commandRouter.h"
#include "global.h"

#define BUFFER_SIZE 1024
using namespace std;

/*** handle redirection ***/
void handleRedirection(string* command){

  vector<string> words = tokenize(*command,">");
  if (words.size() < 2){
    *command = "";
    cerr << "Invalid redirection syntax" << endl;
    return;
  }

  /*** only keeps the commands part to execute ***/
  *command = words[0];
  
  string fileName = tokenize(words[1]," ")[0];
  int fd = open(fileName.c_str(), O_TRUNC | O_WRONLY | O_CREAT, 0666);

  if (fd == -1){
      cerr <<"Fail to open" << fileName << endl;
      return;
  }

  /*** redirect STDOUT to the file descriptor ***/
  if (dup2(fd, STDOUT_FILENO) == -1) {
      cerr << "Dup2 failed!" << endl;
      return;
  }
  close(fd);
  
}

/*** handle background job ***/
void handleBackgroundJob(vector<string> words){
  words.pop_back(); // delete &
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

int main(int argc, char **argv) {

  cout << "Welcome to Dragon Shell!" <<endl;

  while (true){

    cout << "dragon shell > ";
    string line;
    
    /****** get the input from terminal ******/
    if (!getline(cin, line)) { //EOF
        line = "exit";
        cout << endl;
    }

    

    /*** process multiple commands ***/
    vector<string> commands = tokenize(line,";");

    for (string command: commands) {

        /*** save stdout fd, restore later ***/
        int stdoutFdSave = dup(STDOUT_FILENO); 

        /*** check if need redirection ***/
        if (command.find(">") != string::npos){
          handleRedirection(&command);
        }

        /****** tokenize the command *****/
        vector<string> words =  tokenize(command, " ");

        /*** check if it is a background job ***/
        bool isBackgoroundJob = false;
        if (words[words.size()-1].compare("&") == 0 && words[0].compare("&") != 0){
          handleBackgroundJob(words);
          isBackgoroundJob = true;
        }

        /******* command router for handling a single command, internal or external ******/
        route(words,isBackgoroundJob);

        /*** restore stdout ***/
        dup2(stdoutFdSave, STDOUT_FILENO);
        close(stdoutFdSave);
    }

  }

  return 0;
}