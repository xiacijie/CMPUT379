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

    /****** tokenize the command *****/
    vector<string> commands =  tokenize(line, " ");

    /******* command router for handling different commands, internal or external ******/
    route(commands);

  }

  return 0;
}