#include <iostream>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include "util.h"
#include "redirectionHandler.h"
using namespace std;

/*** handle redirection ***/
void handleRedirection(string* command){

  vector<string> words = tokenize(*command,">");
  if (words.size() < 2){
    *command = "";
    cerr << "Invalid redirection syntax" << endl;
    return;
  }

  /*** only keeps the commands part to execute ***/
  *command = words[0];
  
  string fileName = tokenize(words[1]," ")[0];
  int fd = open(fileName.c_str(), O_TRUNC | O_WRONLY | O_CREAT, 0666);

  if (fd == -1){
      cerr <<"Fail to open" << fileName << endl;
      return;
  }

  /*** redirect STDOUT to the file descriptor ***/
  if (dup2(fd, STDOUT_FILENO) == -1) {
      cerr << "Dup2 failed!" << endl;
      return;
  }
  close(fd);
  
}