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
"# 蛤蛤";
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

        /*** save stdout, stdin, stderr fds, restore them later in case of redirection or pipes ***/
        int stdoutFdSave = dup(STDOUT_FILENO); 
        int stderrFdSave = dup(STDERR_FILENO);
        int stdinFdSave = dup(STDIN_FILENO);

        /*** check if need redirection ***/
        if (command.find(">") != string::npos){
          handleRedirection(&command);
        }

        /*** check if it is a pipe ***/
        if (command.find("|") != string::npos){
          cout << "pipe" << endl;
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
          return 0; //exit
        }

        /*** restore stdout, stderr, stdin ***/
        dup2(stdoutFdSave, STDOUT_FILENO);
        dup2(stderrFdSave,STDERR_FILENO);
        dup2(stdinFdSave, STDIN_FILENO);
        close(stdoutFdSave);

    }

  }

  return 0;
}