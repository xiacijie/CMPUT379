#include "commandRouter.h"
#include "commands.h"
#include <iostream>

void commandRouter(vector<string> commands) {
    if (commands.size()<1){ //in case command is empty
        cout << "Error: Command Not Found;";
    }
    else{ // deal with the commands
        string firstCommand = commands[0];

        // internal commands
        if (commands_map.find(firstCommand) != commands_map.end()) {
            cout << firstCommand << endl;
        }
        else{
            cout << "external" << endl;
        }
//        switch (expression)
//        {
//        case /* constant-expression */:
//            /* code */
//            break;
//
//        default:
//            break;
//        }
    }
    
}