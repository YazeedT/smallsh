# Smallsh - Custom Shell in C

## Description
Smallsh is a custom shell program written in C, mimicking fundamental functionalities of standard Unix shells. It's designed to execute commands in both interactive and non-interactive modes, handle expansions, manage background processes, and incorporate custom signal handling.

## Features
- Interactive and Non-Interactive Modes: Run commands directly or via a script file.
- Command Execution: Built-in standard Unix commands: "cd, exit"
- Background Process Management: Ability to run processes in the background.
- Signal Handling: Custom handling of SIGINT and SIGTSTP.
- Input Redirection: Support for input redirection using '<'.
- Output Redirection: Support for output redirection using '>'.

## Installation
1. Clone the repository:
   
   ```bash
   git clone https://github.com/YazeedT/smallsh
   ```
3. Navigate to the cloned directory:
   
   ```bash
   cd smallsh
   ```
5. Compile the program using the provided Makefile:

   ```bash
   make
   ```

## Usage
- Run the shell in interactive mode:

  ```bash
  ./smallsh
  ```
- Run the shell in non-interactive mode (script mode):

  ```bash
  ./smallsh scriptname
  ```

## Built-in Commands
- exit: Exits the shell.
- cd: Changes the current working directory.
