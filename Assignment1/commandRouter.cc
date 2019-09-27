#include "commandRouter.h"
#include "commands.h"
#include "commandHandler.h"
#include <iostream>
#include <unistd.h>

using namespace std;

void route(vector<string> commands) {
    if (commands.size() < 1) { //in case commands is empty
        return;
    }

    string firstCommand = commands[0];

    auto it = commandsMap.find(firstCommand);

    /**** internal commands ****/
    if (it != commandsMap.end()) {
        switch (it->second) {
            case pwd:
                cmdPwd();
                break;
            case cd:
                cmdCd(commands);
                break;
            case exit1:
                cmdExit();
                break;
            case path:
                cmdPath();
                break;
            case a2path:
                cmdA2path(commands);
                break;
            default:
                break;
        }
    }
    /**** external commands ****/
    else{
        cmdExternal(commands);
    }
}