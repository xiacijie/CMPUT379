//
// Created by Cijie Xia on 2019-09-21.
//

#include "util.h"

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