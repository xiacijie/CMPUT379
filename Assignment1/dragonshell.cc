#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include "util.h"
#include "commandRouter.h"
#include "global.h"
#include "redirectionHandler.h"
#include "signalHandler.h"


#define BUFFER_SIZE 1024
using namespace std;

void logWelcomeMessage(){
  string msg  = 
"#\n"
"#                ,######## \n"
"#              #############\n"
"#             ################\n"
"#            ##################\n"
"#           ########       #####\n"
"#           #####            ####\n"
"#          ######             ###\n"
"#          ######             ###\n"
"#         #######              ###\n"
"#         #######              ###\n"
"#         #######              ###\n"
"#         ##########           ##\n"
"#          #######   #  ##   # ##\n"
"#          ########## #####   ###\n"
"#          #######   ### #  # # #\n"
"#          #####     #        #\n"
"#          #####     #  #     #\n"
"#          ####### ###\n"
"#           ####   #### #\n"
"#           #####  ####\n"
"#            #######\n"
"#            #######\n"
"#            #### #######\n"
"#            #### ##\n"
"#            ##### #\n"
"#            #######\n"
"#           ## ######\n"
"#         ##### ##########\n"
"#       ######## #########  ###\n"
"#     ###########  #####    #####\n"
"#   #############    #      ########\n"
"# ################     #    ##########\n"
"# ################   #####  ############K\n"
"# ################# ######  ##############\n"
"# ################## ####   ##############\n"
"# ##################   ##   ##############\n"
"# ###################  ##   ##############\n"
"# ################### ####  ##############\n"
"# ######################### ##############\n"
"#\n"
"Welcome to dragon shell!";
  cout << msg << endl;
  cout << endl;

}

int main(int argc, char **argv) {

  logWelcomeMessage();
  registerSignalHandlers();

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

        /*** check if it is a pipe ***/
        vector<string> pipes = tokenize(command,"|");
        if (pipes.size() == 2){
            pipeCommand(pipes[0],pipes[1]);
        }
        else{
          if (singleCommand(command) == 1){
            return 0;
          }
        }
    }
  }

  return 0;
}