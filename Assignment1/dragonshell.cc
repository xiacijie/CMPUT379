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
#include "redirectionHandler.h"
#include "SignalHandler.h"

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
  cout << "Welcome to Dragon Shell!" <<endl;

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
          words.pop_back(); //delete &
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