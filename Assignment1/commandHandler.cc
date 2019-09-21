#include "commandHandler.h"
#include <iostream>
#include <unistd.h>
#include "global.h"
#include "util.h"

#define BUFFER_SIZE 1024

using namespace std;

void cmd_pwd() {
    char path[BUFFER_SIZE];
    cout << getcwd(path,BUFFER_SIZE) << endl;
}

void cmd_cd(vector<string> commands) {
    if (commands.size() < 2){
        cout << "dragonshell: expected argument to 'cd' " << endl;
        return;
    }

    string path = commands[1];
    if (chdir(path.c_str()) == -1) {
        cout << "dragonshell: No such file or directory" << endl;
        return;
    }
}

void cmd_exit(){
    cout << "Exiting" << endl;
    _exit(0);
}

void cmd_path(){
    cout << "Current PATH: " << PATH << endl;
}

void cmd_a2path(vector<string> commands){
    if (commands.size() < 2){
        cout << "dragonshell: expected argument to 'a2path' " << endl;
        return;
    }

    string path_line = commands[1];
    vector<string> paths = tokenize(path_line, ":");
    if (paths.size() < 1){
        cout << "Invalid path specified!" << endl;
        return;
    }

    //constructing new path
    string new_path = "";
    for (size_t i = 0 ; i < paths.size(); i ++){
        if (paths[i].compare("$PATH") ==  0){ //replace $PATH by real path
            new_path += PATH;
        }
        else{
            new_path += paths[i];
        }
        if (i != paths.size()-1){
            new_path += ":";
        }
    }

    PATH = new_path;
}