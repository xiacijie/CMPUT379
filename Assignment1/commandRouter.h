#ifndef COMMAND_ROUTER_H
#define COMMAND_ROUTER_H

#include <string>
#include <vector>

using namespace std;

int route(vector<string> words, bool isBackgroundJob);
void pipeRoute(vector<string> words1, vector<string> words2);

#endif