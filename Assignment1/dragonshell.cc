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
#include "global.h"

#define BUFFER_SIZE 1024
using namespace std;



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

        /****** tokenize the command *****/
        vector<string> words =  tokenize(command, " ");

        /******* command router for handling a single command, internal or external ******/
        route(words);
    }

  }

  return 0;
}