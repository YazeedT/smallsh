#include "header.h"

// Global variables to track process states and interactive mode
int exit_status_last_fg = 0;
int last_fg_pid = -1;
int last_bg_pid = -1;
int last_pid = 0;
bool is_interactive_mode = true;

// Function to execute a command
void execute(Command *cmd)
{
    // Handling 'exit' command
    if (strcmp(cmd->cmd, "exit") == 0)
    {
        // Exit with specified status or default status
        if (cmd->args[1] != NULL)
            exit(atoi(cmd->args[1]));
        else
            exit(0);
    }

    // Handling 'cd' command
    if (strcmp(cmd->cmd, "cd") == 0)
    {
        // Change directory to HOME or specified directory
        if (cmd->args[1] == NULL)
            chdir(getenv("HOME"));
        else
            chdir(cmd->args[1]);
    }

    // Handling other commands
    else
    {
        int fork_result = fork();
        if (fork_result == 0)
        {
            // Child process
            // Handle input redirection
            if (cmd->input_file_descriptor != -2)
            {
                // Error handling for input file
                if (cmd->input_file_descriptor == -1)
                {
                    fprintf(stderr, "Error: cannot open input file\n");
                    exit(1);
                }
                else
                {
                    dup2(cmd->input_file_descriptor, 0);
                    close(cmd->input_file_descriptor);
                }
            }

            // Handle output redirection
            if (cmd->output_file_descriptor != -2)
            {
                // Error handling for output file
                if (cmd->output_file_descriptor == -1)
                {
                    fprintf(stderr, "Error: cannot open output file\n");
                    exit(1);
                }
                else
                {
                    dup2(cmd->output_file_descriptor, 1);
                    close(cmd->output_file_descriptor);
                }
            }

            // Execute the command
            if (cmd->has_path_to_cmd)
                execv(cmd->cmd, cmd->args);
            else
                execvp(cmd->cmd, cmd->args);

            // Exec failed
            fprintf(stderr, "Error: exec failed\n");
            exit(1);
        }
        else if (fork_result == -1)
        {
            // Forking failed
            fprintf(stderr, "Error: fork failed\n");
        }
        else
        {
            // Parent process
            last_pid = fork_result;
            if (!cmd->is_background)
            {
                // Wait for foreground process
                last_fg_pid = fork_result;
                signalFgWaitMode();
                waitpid(fork_result, &exit_status_last_fg, 0);
                // Process exit status
                if (WIFSIGNALED(exit_status_last_fg))
                    exit_status_last_fg = WTERMSIG(exit_status_last_fg) + 128;
                else
                    exit_status_last_fg = WEXITSTATUS(exit_status_last_fg);
                signalDefaultMode();
            }
            else
            {
                // Background process
                last_bg_pid = fork_result;
            }
        }
    }
}

// Signal handler for child processes
void childSignalHandler(int signo)
{
    int status;
    pid_t pid;
    char format[100];

    // Loop to handle child processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        bool stopped = WIFSTOPPED(status);
        bool exited = WIFEXITED(status);
        bool signaled = WIFSIGNALED(status);

        // Handling stopped foreground processes
        if (stopped && pid == last_fg_pid)
        {
            sprintf(format, "Child process %d stopped by signal %d\n", pid, WSTOPSIG(status));
            write(STDERR_FILENO, format, strlen(format));
            fflush(stderr);
            kill(pid, SIGCONT); // Continue the stopped process
        }

        // Handling background processes
        if (pid != last_fg_pid)
        {
            if (signaled)
            {
                sprintf(format, "Child process %d done. Signaled %d.\n", pid, WTERMSIG(status));
                write(STDERR_FILENO, format, strlen(format));
                fflush(stderr);
            }
            if (exited)
            {
                sprintf(format, "Child process %d done. Exit status %d.\n", pid, WEXITSTATUS(status));
                write(STDERR_FILENO, format, strlen(format));
                fflush(stderr);
            }
        }
    }
}

// Main function
int main(int argc, char **argv)
{
    // Non-interactive mode if arguments are provided
    if (argc > 1)
    {
        is_interactive_mode = false;
        int desc = open(argv[1], O_RDONLY);
        if (desc == -1)
        {
            fprintf(stderr, "Error: cannot open file\n");
            exit(1);
        }
        else
        {
            dup2(desc, STDIN_FILENO);
            close(desc);
        }
    }
    else
        is_interactive_mode = true;

    // Ignore signals in interactive mode
    if (is_interactive_mode)
        signalIgnoreMode();

    // Setup SIGCHLD handler
    struct sigaction action = {0};
    action.sa_handler = childSignalHandler;
    sigfillset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &action, NULL);

    char **input;

    // Main loop to get and execute commands
    while ((input = getInput()) != NULL)
    {
        if (input[0] == NULL)
        {
            freeInput(input);
            continue;
        }

        Command *command = parseInput(input);
        execute(command);

        freeInput(input);
        freeCommand(command);
    }

    return 0;
}