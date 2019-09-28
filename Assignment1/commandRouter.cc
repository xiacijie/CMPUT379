#include "commandRouter.h"
#include "commands.h"
#include "commandHandler.h"
#include <iostream>
#include <unistd.h>

#include <string>


using namespace std;

void route(vector<string> words, bool isBackgroundJob) {

    if (words.size() < 1) { //in case commands is empty
        return;
    }

    string firstWord = words[0];
    auto it = commandsMap.find(firstWord);

    /**** internal commands ****/
    if (it != commandsMap.end()) {
        switch (it->second) {
            case pwd:
                cmdPwd();
                break;
            case cd:
                cmdCd(words);
                break;
            case exit1:
                cmdExit();
                break;
            case path:
                cmdPath();
                break;
            case a2path:
                cmdA2path(words);
                break;
            default:
                break;
        }
    }

    /**** external commands ****/
    else{
        cmdExternal(words, isBackgroundJob);
    }

    

}