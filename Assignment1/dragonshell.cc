#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include "util.h"
#include "commandRouter.h"
#define BUFFER_SIZE 1024
using namespace std;


int main(int argc, char **argv) {
  // print the string prompt without a newline, before beginning to read
  // tokenize the input, run the command(s), and print the result
  // do this in a loop
  // string test = "This is a   test string";
  // vector<string> v = tokenize(test," ");
  // for (size_t i = 0; i<v.size();i++){
  //   cout << v[i] << endl;
  // }

  cout << "Welcome to Dragon Shell!" <<endl;

  while (true){

    cout << "dragon shell > ";
    string line;
    
    //get the input from terminal
    if (!getline(cin, line)) { //EOF
        line = "exit";
        cout << endl;
    }
    //tokenize the command
    vector<string> commands =  tokenize(line, " ");
    // command router
    commandRouter(commands);





  }

  return 0;
}