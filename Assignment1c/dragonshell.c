#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "util.h"
#include "signalHandler.h"
#define BUFFER_SIZE 1024

void logWelcomeMessage(){
  char* msg  = 
"#\n"
"#                ,######## \n"
"#              #############\n"
"#             ################\n"
"#            ##################\n"
"#           ########       #####\n"
"#           #####            ####\n"
"#          ######             ###\n"
"#          ######             ###\n"
"#         #######              ###\n"
"#         #######              ###\n"
"#         #######              ###\n"
"#         ##########           ##\n"
"#          #######   #  ##   # ##\n"
"#          ########## #####   ###\n"
"#          #######   ### #  # # #\n"
"#          #####     #        #\n"
"#          #####     #  #     #\n"
"#          ####### ###\n"
"#           ####   #### #\n"
"#           #####  ####\n"
"#            #######\n"
"#            #######\n"
"#            #### #######\n"
"#            #### ##\n"
"#            ##### #\n"
"#            #######\n"
"#           ## ######\n"
"#         ##### ##########\n"
"#       ######## #########  ###\n"
"#     ###########  #####    #####\n"
"#   #############    #      ########\n"
"# ################     #    ##########\n"
"# ################   #####  ############K\n"
"# ################# ######  ##############\n"
"# ################## ####   ##############\n"
"# ##################   ##   ##############\n"
"# ###################  ##   ##############\n"
"# ################### ####  ##############\n"
"# ######################### ##############\n"
"#\n"
"# 蛤蛤";
  printf("%s\n\n",msg);

}

int main(int argc, char **argv) {
  // print the string prompt without a newline, before beginning to read
  // tokenize the input, run the command(s), and print the result
  // do this in a loop

  logWelcomeMessage();
  registerSignalHandlers();

  while (1) {
    printf("dragon shell > ");

    char* buffer;
    size_t bufferSize = BUFFER_SIZE;

    buffer = (char*)malloc(bufferSize*sizeof(char));
    if (buffer == NULL){
      perror("Unable to allocate buffer\n");
      _exit(1);
    }

    /****** get the input from terminal ******/

    getline(&buffer,&bufferSize,stdin);
    if (feof(stdin)){
      free(buffer);
      _exit(1);
    }

    /**** remove the trailing new line char from the input ****/
    size_t ln = strlen(buffer) - 1;
    if (*buffer && buffer[ln] == '\n') 
      buffer[ln] = '\0';

    char *commands[BUFFER_SIZE];

    size_t commandsLength = tokenize(buffer,";",commands);

    for (size_t i =0; i < commandsLength; i++){
        char* words[BUFFER_SIZE];
        size_t wordsLength = tokenize(commands[i]," ",words);
        for (size_t j = 0; j < wordsLength; j++){
          printf("%s\n",words[j]);
        }
    }
 
    free(buffer);
  }
  
  return 0;
}