#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include "util.h"
#include "commandRouter.h"

#define BUFFER_SIZE 1024
using namespace std;


int main(int argc, char **argv) {
  // print the string prompt without a newline, before beginning to read
  // tokenize the input, run the command(s), and print the result
  // do this in a loop
  // string test = "This is a   test string";
  // vector<string> v = tokenize(test," ");
  // for (size_t i = 0; i<v.size();i++){
  //   cout << v[i] << endl;
  // }

  cout << "Welcome to Dragon Shell!" <<endl;

  while (true){

    cout << "dragon shell > ";
    string line;
    
    //get the input from terminal
    if (!getline(cin, line)) { //EOF
        line = "exit";
        cout << endl;
    }
    //tokenize the command
    vector<string> commands =  tokenize(line, " ");

    cout << "*** " << getpid() << endl;
    //****************create new process ************
    int status;
    pid_t pid = fork();
    if (pid > 0){ // parent process
        cout << "====== " << pid << endl;
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status) == 2){ //terminate all processes and exit the shell
            kill(0,SIGTERM);
            _exit(0);
        }
    }
    else if (pid == 0){ // child process
        // command router
        commandRouter(commands);
    }
    else{ // error
        cout << "Error in forking new process" << endl;
    }

  }

  return 0;
}