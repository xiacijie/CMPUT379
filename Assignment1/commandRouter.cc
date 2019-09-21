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
            case cd:
                cmd_cd(commands);
                break;
            case exit1:
                cmd_exit();
                break;
            case path:
                cmd_path();
                break;
            case a2path:
                cmd_a2path(commands);
                break;
            default:
                break;
        }
    }
    else{ // external commands
        cout << "external" << endl;
    }
}