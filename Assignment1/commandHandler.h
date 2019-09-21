#ifndef COMMAND_HANDLER
#define COMMAND_HANDLER

#include <vector>
#include <string>

using namespace std;

void cmd_pwd();
void cmd_cd(vector<string> commands);
void cmd_exit();
void cmd_path();
void cmd_a2path(vector<string> commands);
#endif