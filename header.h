#ifndef SMALLSH_HEADER_H
#define SMALLSH_HEADER_H

#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200112

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_CHARACTERS 2048
#define MAX_WORDS 512

// Structure to hold command information
struct Command {
    char *cmd;
    char **args;
    bool has_path_to_cmd;
    bool is_background;
    int input_file_descriptor;
    int output_file_descriptor;
};
typedef struct Command Command;  // Define a type alias for the Command structure

// Declaration of global variables
extern int exit_status_last_fg;
extern int last_bg_pid;
extern int last_fg_pid;
extern bool is_interactive_mode;

// Function prototypes
char **getInput();
void freeInput(char **input);
void *expandWord(char *input);
Command *parseInput(char **input);
void freeCommand(Command *command);
void signalReadingMode();
void signalIgnoreMode();
void signalDefaultMode();
void signalFgWaitMode();

#endif