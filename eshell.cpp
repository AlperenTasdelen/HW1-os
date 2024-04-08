#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <csignal>
#include <stdlib.h>
#include "parser.h"

using namespace std;

void execute_subshell(parsed_input parsed);

void execute_command(parsed_input parsed) {
    if(parsed.inputs[0].type == INPUT_TYPE_COMMAND) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // Child process
            execvp(parsed.inputs[0].data.cmd.args[0], parsed.inputs[0].data.cmd.args);
            perror("exec");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            wait(NULL);
        }
    }
    else {
        parsed_input temp;
        if(parse_line(parsed.inputs[0].data.subshell, &temp)) {
            execute_subshell(temp);
            free_parsed_input(&temp);
        } else {
            cout << "Failed to parse the input\n" << endl;
        }
    }
}

void execute_pipeline(parsed_input parsed) {

    char** commands[MAX_INPUTS];
    for(int i = 0; i < parsed.num_inputs; i++) {
        if(parsed.inputs[i].type == INPUT_TYPE_COMMAND) {
            commands[i] = parsed.inputs[i].data.cmd.args;
        } else {
            //cout << "Subshell input type\n";
            //redirected to subshell
        }
    }

    int pipes[parsed.num_inputs - 1][2]; // Create pipes for communication between commands
    
    // Iterate over commands
    for (int i = 0; i < parsed.num_inputs; i++) {
        if (i < parsed.num_inputs - 1) {
            pipe(pipes[i]);
        }
        
        pid_t pid = fork();
        
        if (pid == 0) { // Child process
            // Set up input redirection
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
                close(pipes[i - 1][1]);
            }
            
            // Set up output redirection
            if (i < parsed.num_inputs - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
                close(pipes[i][0]);
            }
            
            // Execute the command
            if(parsed.inputs[i].type == INPUT_TYPE_COMMAND){
                execvp(commands[i][0], commands[i]);
            }
            else{
                parsed_input temp;
                if(parse_line(parsed.inputs[i].data.subshell, &temp)) {
                    execute_subshell(temp);
                    free_parsed_input(&temp);
                } else {
                    cout << "Failed to parse the input\n" << endl;
                }
                exit(EXIT_SUCCESS);
            }
            perror("exec");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
    
    for (int i = 0; i < parsed.num_inputs - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    for (int i = 0; i < parsed.num_inputs; i++) {
        wait(NULL);
    }
}

void execute_sequential(parsed_input parsed) {
    for(int i = 0; i < parsed.num_inputs; i++) {

        pid_t pid = fork();

        if(pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if(pid == 0) { // Child process
            if(parsed.inputs[i].type == INPUT_TYPE_COMMAND) {
                parsed_input temp = {
                    .inputs = {parsed.inputs[i]},
                    .separator = parsed.separator,
                    .num_inputs = 1
                };
                execute_command(temp);
                exit(EXIT_SUCCESS);
            }
            else if(parsed.inputs[i].type == INPUT_TYPE_PIPELINE) {

                single_input temp_inputs[MAX_INPUTS];

                for(int j = 0; j < parsed.inputs[i].data.pline.num_commands; j++){
                    
                    temp_inputs[j] = {
                        .type = INPUT_TYPE_COMMAND,
                        .data = {
                            .cmd = parsed.inputs[i].data.pline.commands[j]
                        }
                    };
                }

                parsed_input temp = {
                    .inputs = {},
                    .separator = SEPARATOR_PIPE,
                    .num_inputs = parsed.inputs[i].data.pline.num_commands
                };

                for(int j = 0; j < parsed.inputs[i].data.pline.num_commands; j++){
                    temp.inputs[j] = temp_inputs[j];
                }
                
                execute_pipeline(temp);
                exit(EXIT_SUCCESS);
            }
            else {
                //TODO: SUBSHELL
                parsed_input temp;
                if(parse_line(parsed.inputs[i].data.subshell, &temp)) {
                    execute_subshell(temp);
                    free_parsed_input(&temp);
                } else {
                    cout << "Failed to parse the input\n" << endl;
                }
                exit(EXIT_SUCCESS);
            }
            exit(EXIT_FAILURE);
        }
        else { // Parent process
            wait(NULL);
        }
    }
}

void execute_parallel(parsed_input parsed){
    for(int i = 0; i < parsed.num_inputs; i++) {
        pid_t pid = fork();

        if(pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if(pid == 0) { // Child process
            if(parsed.inputs[i].type == INPUT_TYPE_COMMAND) {
                parsed_input temp = {
                    .inputs = {parsed.inputs[i]},
                    .separator = parsed.separator,
                    .num_inputs = 1
                };
                execute_command(temp);
                exit(EXIT_SUCCESS);
            }
            else if(parsed.inputs[i].type == INPUT_TYPE_PIPELINE) {
                
                single_input temp_inputs[MAX_INPUTS];

                for(int j = 0; j < parsed.inputs[i].data.pline.num_commands; j++){
                    
                    temp_inputs[j] = {
                        .type = INPUT_TYPE_COMMAND,
                        .data = {
                            .cmd = parsed.inputs[i].data.pline.commands[j]
                        }
                    };
                }

                parsed_input temp = {
                    .inputs = {},
                    .separator = SEPARATOR_PIPE,
                    .num_inputs = parsed.inputs[i].data.pline.num_commands
                };

                for(int j = 0; j < parsed.inputs[i].data.pline.num_commands; j++){
                    temp.inputs[j] = temp_inputs[j];
                }

                execute_pipeline(temp);
                exit(EXIT_SUCCESS);
            }
            else {
                //TODO: SUBSHELL
                parsed_input temp;
                if(parse_line(parsed.inputs[i].data.subshell, &temp)) {
                    execute_subshell(temp);
                    free_parsed_input(&temp);
                } else {
                    cout << "Failed to parse the input\n" << endl;
                }
                exit(EXIT_SUCCESS);
            }
            
            exit(EXIT_FAILURE);
        }
        else { // Parent process
            
        }
    }
    for(int i = 0; i < parsed.num_inputs; i++) {
        wait(NULL);
    }
}

void execute_subshell(parsed_input parsed){
    if(parsed.separator == SEPARATOR_PIPE){
        execute_pipeline(parsed);
    }
    else if(parsed.separator == SEPARATOR_SEQ){
        execute_sequential(parsed);
    }
    else if(parsed.separator == SEPARATOR_PARA){
        execute_parallel(parsed);
    }
    else{
        execute_command(parsed);
    }
}

int main() {
    char input[1000];

    while (1) {
        cout << "/>";
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; // Remove newline character
        
        if (strcmp(input, "quit") == 0) {
            break;
        } else {
            parsed_input parsed;
            if (parse_line(input, &parsed)) {
                // Successfully parsed the input
                if(parsed.separator == SEPARATOR_PIPE){
                    execute_pipeline(parsed);
                }
                else if(parsed.separator == SEPARATOR_SEQ){
                    execute_sequential(parsed);
                }
                else if(parsed.separator == SEPARATOR_PARA){
                    execute_parallel(parsed);
                }
                else{
                    execute_command(parsed);
                }
                free_parsed_input(&parsed);
            } else {
                cout << "Failed to parse the input\n" << endl;
            }
        }
    }

    return 0;
}
