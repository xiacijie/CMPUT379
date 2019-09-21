#include "commandRouter.h"
#include "commands.h"
#include "commandHandler.h"
#include <iostream>

using namespace std;

void commandRouter(vector<string> commands) {
    if (commands.size() < 1) { //in case commands is empty
        cout << "Error: Command Not Found;";
        return;
    }

    string firstCommand = commands[0];

    // internal commands
    auto it = commands_map.find(firstCommand);
    if (it != commands_map.end()) {
        switch (it->second) {
            case pwd:
                cmd_pwd();
                break;
            default:
                break;
        }
    }
    else{ // external commands
        cout << "external" << endl;
    }
}