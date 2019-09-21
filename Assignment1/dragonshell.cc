#include <vector>
#include <iostream>
#include <string>
#include <cstring>

#include <unistd.h>
#include "command.h"

#define BUFFER_SIZE 1024
using namespace std;
/**
 * @brief Tokenize a string 
 * 
 * @param str - The string to tokenize
 * @param delim - The string containing delimiter character(s)
 * @return vector<string> - The list of tokenized strings. Can be empty
 */
vector<string> tokenize(const string &str, const char *delim) {
  char* cstr = new char[str.size() + 1];
  strcpy(cstr, str.c_str());

  char* tokenized_string = strtok(cstr, delim);

  vector<string> tokens;
  while (tokenized_string != NULL)
  {
    tokens.push_back(string(tokenized_string));
    tokenized_string = strtok(NULL, delim);
  }
  delete[] cstr;

  return tokens;
}

void commandRoute(string first_command, ){
  switch (command_map[])
  {
  case /* constant-expression */:
    /* code */
    break;
  
  default:
    break;
  }
}

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
    
    //get the input from termial
    cin >> line;

    //tokenize the commands
    vector<string> commands =  tokenize(line, " ");
    if (commands.size() < 1){ // tokenized commands empty 
      cout << "Command Not Found!" << endl;
      continue;
    }
    else{
      string first_command = commands[0];

    }

    char path[BUFFER_SIZE];
    cout << getcwd(path, BUFFER_SIZE);


    cout << line << endl;
  }

  return 0;
}