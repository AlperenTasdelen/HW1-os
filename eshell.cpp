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
            // Execute the command
            execvp(parsed.inputs[0].data.cmd.args[0], parsed.inputs[0].data.cmd.args);
            perror("exec");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            wait(NULL);
        }
    }
    else {
        //TODO: SUBSHELL
        //cout << "Subshell\n";
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
            cout << "Subshell input type\n";
            // TODO: SUBSHELL
        }
    }

    int pipes[parsed.num_inputs - 1][2]; // Create pipes for communication between commands
    
    // Iterate over commands
    for (int i = 0; i < parsed.num_inputs; i++) {
        if (i < parsed.num_inputs - 1) {
            pipe(pipes[i]); // Create a pipe for each command except the last one
        }
        
        pid_t pid = fork(); // Fork a child process
        
        if (pid == 0) { // Child process
            // Set up input redirection
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO); // Read from the previous pipe
                close(pipes[i - 1][1]); // Close write end of the previous pipe
            }
            
            // Set up output redirection
            if (i < parsed.num_inputs - 1) {
                dup2(pipes[i][1], STDOUT_FILENO); // Write to the current pipe
                close(pipes[i][0]); // Close read end of the current pipe
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
    // Close all pipe descriptors in parent process
    for (int i = 0; i < parsed.num_inputs - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all child processes to finish
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
                //cout << "this part called" << endl;
                
                //single_input_union temp_data = parsed.inputs[i].data;
                //pretty_print(&parsed);
                
                /*
                command cmd1 = parsed.inputs[i].data.pline.commands[0];
                command cmd2 = parsed.inputs[i].data.pline.commands[1];
                
                single_input_union temp_data1 = {
                    cmd: parsed.inputs[i].data.pline.commands[0];
                };

                single_input_union temp_data2 = {
                    cmd: parsed.inputs[i].data.pline.commands[1];
                };

                single_input temp_input1 = {
                    .type = INPUT_TYPE_COMMAND,
                    .data = {
                        cmd: parsed.inputs[i].data.pline.commands[0];
                    };
                };

                single_input temp_input2 = {
                    .type = INPUT_TYPE_COMMAND,
                    .data = {
                        cmd: parsed.inputs[i].data.pline.commands[1];
                    };
                };

                parsed_input temp = {
                    .inputs = {temp_input1, temp_input2},
                    .separator = SEPARATOR_PIPE,
                    .num_inputs = 2 // ????
                };
                */
                //now we need to make a for that does exact same thing up on parsed_input temp
                //but num_inputs is important for a for cycle

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
                //pretty_print(&temp);
                //pretty_print(&parsed);
                /*
                cout << parsed.inputs[i].data.pline.num_commands << endl;
                for(int i = 0; parsed.inputs[i].data.pline.commands[0].args[0][i] != 0; i++){
                    cout << parsed.inputs[i].data.pline.commands[0].args[0][i] << endl;
                }
                for(int i = 0; parsed.inputs[i].data.pline.commands[0].args[0][i] != 0; i++){
                    cout << parsed.inputs[i].data.pline.commands[0].args[0][i] << endl;
                }
                */
                //cout << parsed.inputs[i].data.subshell << endl;
                //execute_subshell(temp);
                /*
                parsed_input temp;
                //execute_pipeline(temp);
                if(parse_line(parsed.inputs[i].data.subshell, &temp)){
                    execute_subshell(temp);
                    free_parsed_input(&temp);
                }
                */
                //execute_subshell(temp);
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
                    //cout << "Failed to parse the input\n" << endl;
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
    // Successfully parsed the input
    if(parsed.separator == SEPARATOR_PIPE){
        //cout << "Pipeline detected\n";
        execute_pipeline(parsed);
    }
    else if(parsed.separator == SEPARATOR_SEQ){
        //cout << "Sequential detected\n";
        execute_sequential(parsed);
    }
    else if(parsed.separator == SEPARATOR_PARA){
        //cout << "Parallel detected\n";
        execute_parallel(parsed);
    }
    else{
        //cout << "No separator detected\n";
        execute_command(parsed);
    }
    // Don't forget to free the allocated memory after you're done using it
    // free_parsed_input(&parsed);
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
                //cout << "Input count: " << parsed.num_inputs << endl;
                if(parsed.separator == SEPARATOR_PIPE){
                    //cout << "Pipeline detected\n";
                    execute_pipeline(parsed);
                }
                else if(parsed.separator == SEPARATOR_SEQ){
                    //cout << "Sequential detected\n";
                    execute_sequential(parsed);
                }
                else if(parsed.separator == SEPARATOR_PARA){
                    //cout << "Parallel detected\n";
                    execute_parallel(parsed);
                }
                else{
                    //cout << "No separator detected\n";
                    execute_command(parsed);
                }
                    
                // Don't forget to free the allocated memory after you're done using it
                free_parsed_input(&parsed);
            } else {
                // Failed to parse the input, handle the error if needed
                cout << "Failed to parse the input\n" << endl;
            }
        }
    }

    return 0;
}
