#ifndef COMMANDS_H
#define COMMANDS_H

#include <map>
#include <string>

enum Commands {
    pwd,
    cd
};


map<string, Commands> commands_map = {
    {"pwd", pwd},
    {"cd", cd}
};

#endif