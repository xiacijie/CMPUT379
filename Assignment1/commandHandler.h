#ifndef COMMAND_HANDLER
#define COMMAND_HANDLER

#include <vector>
#include <string>

using namespace std;

void terminateAllChildProcesses();
void cmdPwd();
void cmdCd(vector<string> commands);
void cmdExit();
void cmdPath();
void cmdA2path(vector<string> commands);
void cmdExternal(vector<string> commands);

#endif