#ifndef _PARSE_H_
#define _PARSE_H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>

const int status_success = 0;
const int status_failure = 1;

// Symbol type represents the command seperator
typedef enum SymbolType {
    RedirectIn,             // 0 - <
    RedirectInAppend,       // 1 - <<
    RedirectOut,            // 2 - >
    RedirectOutAppend,      // 3 - >>
    ExecuteOnSuccess,       // 4 - && - exec on success
    ExecuteOnFailure,       // 5 - || - exec on failure 
    Pipe,                   // 6 - | 
    Null,                   // 7 - end of string (null character encountered)
    NewLine,                // 8 - end of command due to new line
    Semicolon,              // 9 ;
} SymbolType;

// Command contains the parsed command. These structures are chained in a doubly linked list 
typedef struct Command {
    char *file;             // file to execute 
    char **arglist;         // argument list to executable
    SymbolType symbolType;  // command seperator 
    FILE *inFilePtr;        // file pointer to input stream 
    FILE *outFilePtr;       // file pointer to output stream 
    FILE *errorFilePtr;     // file pointer to error stream 
    int inFileHandle;       // file handle to input stream 
    int outFileHandle;      // file handle to output stream
    int errorFileHandle;    // file handle to error stream 
    int status;             // exit code of the commnad

    struct Command *next, *prev;   
} Command;


//function defination
int GetCommandChain(Command **head);
int DeleteCommandChain(Command *head);
void DumpCommandChain(Command *head);
int  createArgList(char ** outarglist,char *input);
#endif
