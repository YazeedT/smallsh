#include "header.h"

// Signal handler function for stopping input. Empty, used as a handler
void signalStopInput(int signo)
{

}

// Function to set the shell in a mode where it can read input
void signalReadingMode()
{
    // Setup to handle CTRL-C for exiting input process
    struct sigaction action={0};
    action.sa_handler = signalStopInput;
    sigfillset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    // Setup to handle CTRL-Z for exiting input process
    struct sigaction action2={0};
    action2.sa_handler = signalStopInput;
    sigfillset(&action2.sa_mask);
    action2.sa_flags = 0;
    sigaction(SIGTSTP, &action2, NULL);
}

// Function to set the shell in ignore mode for specific signals
void signalIgnoreMode(){
    // Setup to ignore CTRL-C in the shell
    struct sigaction action={0};
    action.sa_handler = SIG_IGN;
    sigfillset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &action, NULL);

    // Setup to ignore CTRL-Z in the shell
    struct sigaction action2={0};
    action2.sa_handler = SIG_IGN;
    sigfillset(&action2.sa_mask);
    action2.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &action2, NULL);
}

// Function to reset default signal handling
void signalDefaultMode(){
    // Reset to default handling for CTRL-C
    struct sigaction action={0};
    action.sa_handler = SIG_DFL;
    sigfillset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &action, NULL);

    // Reset to default handling for CTRL-Z
    struct sigaction action2={0};
    action2.sa_handler = SIG_DFL;
    sigfillset(&action2.sa_mask);
    action2.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &action2, NULL);

}

// Signal handler function for passing signals to child processes. Empty, used as a handler
void passToChild(int signo){

}

// Function to set the shell in a mode to wait for foreground processes
void signalFgWaitMode(){
    // Setup to pass CTRL-C to child processes
    struct sigaction action={0};
    action.sa_handler = passToChild;
    sigfillset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &action, NULL);

    // Setup to pass CTRL-Z to child processes
    struct sigaction action2={0};
    action2.sa_handler = passToChild;
    sigfillset(&action2.sa_mask);
    action2.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &action2, NULL);
}