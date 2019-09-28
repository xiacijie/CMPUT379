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
  
  int fd;
  string fileName = words[1];
  fd = open(fileName.c_str(), O_TRUNC | O_WRONLY | O_CREAT, 0666);

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

        /******* command router for handling a single command, internal or external ******/
        route(words);

        /*** restore stdout ***/
        dup2(stdoutFdSave, STDOUT_FILENO);
        close(stdoutFdSave);
    }

  }

  return 0;
}