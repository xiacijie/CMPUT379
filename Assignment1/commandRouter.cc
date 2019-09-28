#include "commandRouter.h"
#include "commands.h"
#include "commandHandler.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <algorithm>

using namespace std;

void route(vector<string> words) {
    if (words.size() < 1) { //in case commands is empty
        return;
    }

    string firstWord = words[0];

    /*** handle redirection ***/
    auto itRedirect = find(words.begin(), words.end(), ">");
    int stdoutFdSave = dup(STDOUT_FILENO); /*** save stdout fd, restore later ***/
    if (itRedirect != words.end()){ /*** need redirection ***/
        string fileName = *(next(itRedirect,1));
        int fd;
        fd = open(fileName.c_str(), O_TRUNC | O_WRONLY | O_CREAT, 0666);
        words.erase(itRedirect,words.end());
        if (fd == -1){
            cerr <<"Open failed!" <<endl;
            return;
        }

        if (dup2(fd, STDOUT_FILENO) == -1) {
            cerr << "Dup2 failed!" << endl;
            return;
        }
        close(fd);
    }
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
        /*** background job ***/
        if (words[words.size()-1].compare("&") == 0 && words[0].compare("&") != 0){
            words.pop_back(); // delete &
            cmdExternal(words,true);
        }else{
            cmdExternal(words,false);
        }

    }

    /*** restore stdout ***/
    dup2(stdoutFdSave, STDOUT_FILENO);
    close(stdoutFdSave);

}