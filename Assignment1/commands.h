#ifndef COMMANDS_H
#define COMMANDS_H

#include <map>
#include <string>

enum Commands {
    pwd,
    cd,
    exit1,
    path,
    a2path,
};


map<string, Commands> commands_map = {
    {"pwd", pwd},
    {"cd", cd},
    {"exit",exit1},
    {"$PATH", path },
    {"a2path", a2path}
};

#endif