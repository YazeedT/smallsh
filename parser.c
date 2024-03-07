#include "header.h"

// Buffer and words array for input processing
char inp_buffer[MAX_CHARACTERS + 128]; // Extra space for expansion
char *inp_words[MAX_WORDS];

// Function to read and process input
char **getInput()
{
    // Display prompt in interactive mode
    if (is_interactive_mode)
    {
        char *PS1 = getenv("PS1");
        if (PS1)
            fprintf(stderr, "%s", PS1);
        fflush(stderr);
    }

    // Clear the input buffer
    memset(inp_buffer, '\0', sizeof(inp_buffer));

    // Set signal reading mode for interactive mode
    if (is_interactive_mode)
        signalReadingMode();

    // Read input and return NULL if read fails
    if (read(STDIN_FILENO, inp_buffer, MAX_CHARACTERS) <= 0)
        return NULL;

    // Ignore signals after reading input
    if (is_interactive_mode)
        signalIgnoreMode();

    // Process comment and newline characters
    for (int i = 0; i < MAX_CHARACTERS; i++)
    {
        if (inp_buffer[i] == '#' || inp_buffer[i] == '\n')
        {
            inp_buffer[i] = '\0';
            break;
        }
    }

    // Handle empty input
    int inp_len = (int)strlen(inp_buffer);
    if (inp_len == 0)
    {
        inp_words[0] = NULL;
        return inp_words;
    }

    // Trim leading and trailing whitespace
    char *start_pointer = inp_buffer;
    while (isspace(*start_pointer))
        start_pointer++;

    char *end_pointer = inp_buffer + inp_len - 1;
    while (isspace(*end_pointer) || *end_pointer == '\\')
        end_pointer--;
    *(end_pointer + 1) = '\0';

    // Tokenize input into words
    int start = -1;
    int word_count = 0;
    for (int i = 0;; i++)
    {
        char c = start_pointer[i];
        if (isspace(c) || c == '\0')
        {
            if (start != -1)
            {
                int end = i;
                inp_words[word_count] = malloc(sizeof(char) * (end - start + 1));
                memset(inp_words[word_count], '\0', sizeof(char) * (end - start + 1));
                for (int j = start, curr = 0; j < end; j++)
                    inp_words[word_count][curr++] = start_pointer[j];
                word_count++;
            }
            start = -1;
        }
        else if (c == '\\' && start_pointer[i + 1] == ' ')
            i++;
        else if (start == -1)
            start = i;

        if (c == '\0')
            break;
    }

    // Mark the end of words array
    inp_words[word_count] = NULL;

    // Expand special characters in words
    for (int i = 0; i < word_count; i++)
        inp_words[i] = expandWord(inp_words[i]);

    return inp_words;
}

// Function to duplicate strings
char *strdup(const char *s)
{
    char *d = malloc(strlen(s) + 1);
    if (d == NULL)
        return NULL;

    strcpy(d, s);
    return d;
}

// Function to expand special words
void *expandWord(char *input)
{
    for (int i = 0; input[i]; i++)
    {
        if (input[i] == '\\')
        {
            for (int j = i; input[j]; j++)
                input[j] = input[j + 1];

        }
    }

    // Allocate space for new word
    char *new_word = malloc(sizeof(char) * MAX_WORDS);
    memset(new_word, '\0', sizeof(char) * MAX_WORDS);
    int i = 0, j = 0;
    for (; input[i] && input[i + 1]; i++)
    {
        // Expand special symbols $$, $?, $!, and ${}
        if (input[i] == '$' && input[i + 1] == '$')
        {
            char *pid = malloc(sizeof(char) * 10);
            sprintf(pid, "%d", getpid()); //

            for (int k = 0; pid[k]; k++)
                new_word[j++] = pid[k];
            free(pid);
            i++;
        }
        else if (input[i] == '$' && input[i + 1] == '?')
        {
            char *exit_status = malloc(sizeof(char) * 10);
            sprintf(exit_status, "%d", exit_status_last_fg);

            for (int k = 0; exit_status[k]; k++)
                new_word[j++] = exit_status[k];

            free(exit_status);
            i++;
        }
        else if (input[i] == '$' && input[i + 1] == '!')
        {

            if (last_bg_pid == -1)
            { // replace with empty string

                i++;
            }
            else
            {
                char *pid = malloc(sizeof(char) * 10);
                sprintf(pid, "%d", last_bg_pid);
                input[i] = '\0';

                for (int k = 0; pid[k]; k++)
                    new_word[j++] = pid[k];

                free(pid);
                j++;
            }
        }
        else if (input[i] == '$' && input[i + 1] == '{')
        {
            int end = i + 2;
            while (input[end] && input[end] != '}')
                end++;

            if (input[end] == '}')
            {
                char *env_var = malloc(sizeof(char) * (end - (i + 2) + 1));
                memset(env_var, '\0', sizeof(char) * (end - (i + 2) + 1));

                strncpy(env_var, input + i + 2, end - (i + 2));

                char *env_val = getenv(env_var);
                if (env_val != NULL)
                    for (int k = 0; env_val[k]; k++)
                        new_word[j++] = env_val[k];

                free(env_var);
                i = end;
            }
        }
        else
        {
            new_word[j++] = input[i];
        }
    }

    if (input[i])
        new_word[j++] = input[i];

    // Duplicate the expanded word for memory efficiency (we dont need the whole MAX_WORDS keep occupied)
    char *temp = strdup(new_word);

    // Free memory and return the duplicated string
    free(new_word);
    free(input);

    return temp;
}

// Function to parse input into a Command structure
Command *parseInput(char **input)
{
    // Allocate and initialize Command structure
    Command *command = malloc(sizeof(Command));
    command->cmd = NULL;
    command->is_background = false;
    command->input_file_descriptor = -2;
    command->output_file_descriptor = -2;

    command->cmd = strdup(input[0]);
    command->has_path_to_cmd = strchr(command->cmd, '/') != NULL;


    // Parse redirection operators
    if (strchr(command->cmd, '/') != NULL)
    {
        command->has_path_to_cmd = true;
    }
    else
    {
        command->has_path_to_cmd = false;
    }

    // Check if has input redirection
    for (int i = 0; input[i] != NULL; i++)
    {
        if (!strcmp(input[i], "<"))
        {

            if (input[i + 1] == NULL)
            {
                fprintf(stderr, "Error: no input file specified\n");
                return NULL;
            }

            if(command->input_file_descriptor != -1){
                close(command->input_file_descriptor);
            }

            command->input_file_descriptor = open(input[i + 1], O_RDONLY);
        }
    }

    // Check if has output redirection
    for (int i = 0; input[i] != NULL; i++)
    {
        if (!strcmp(input[i], ">"))
        {

            if (input[i + 1] == NULL)
            {
                fprintf(stderr, "Error: no output file specified\n");
                return NULL;
            }

            if(command->output_file_descriptor != -1){
                close(command->output_file_descriptor);
            }

            command->output_file_descriptor = open(input[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0777);
        }
        else if (!strcmp(input[i], ">>"))
        {

            if (input[i + 1] == NULL)
            {
                fprintf(stderr, "Error: no output file specified\n");
                return NULL;
            }

            if(command->output_file_descriptor != -1){
                close(command->output_file_descriptor);
            }

            command->output_file_descriptor = open(input[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0777);
        }
    }

    // Check for background process
    for (int i = 0; input[i] != NULL; i++)
    {
        if (!strcmp(input[i], "&"))
        {

            if (input[i + 1] != NULL)
            {
                fprintf(stderr, "Error: background process must be last argument\n");
                return NULL;
            }

            command->is_background = true;
        }
    }

    // Count arguments and allocate memory
    int arg_count = 0;
    for (int i = 0; input[i] != NULL; i++)
    {

        if (strcmp(input[i], "<") == 0 || strcmp(input[i], ">") == 0 || strcmp(input[i], ">>") == 0)
        {
            i++;
            continue;
        }
        if (strcmp(input[i], "&") == 0)
            continue;

        arg_count++;
    }

    // Allocate memory for arguments
    command->args = malloc(sizeof(char *) * (arg_count + 1));

    // Copy arguments
    int j = 0;
    for (int i = 0; input[i] != NULL; i++)
    {

        if (strcmp(input[i], "<") == 0 || strcmp(input[i], ">") == 0 || strcmp(input[i], ">>") == 0)
        {
            i++;
            continue;
        }
        if (strcmp(input[i], "&") == 0)
            continue;

        command->args[j] = strdup(input[i]);
        j++;
    }

    command->args[arg_count] = NULL;

    return command;
}

// Function to free Command structure
void freeCommand(Command *command)
{
    free(command->cmd);
    for (int i = 0; command->args[i] != NULL; i++)
        free(command->args[i]);
    free(command->args);
    free(command);
}

// Function to free input words
void freeInput(char **input)
{
    for (int i = 0; input[i] != NULL; i++)
        free(input[i]);
}