#include <stdio.h>
#include <string.h>
#include "parser.h"

int main() {
    char input[1000];

    while (1) {
        printf("/>");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; // Remove newline character
        
        if (strcmp(input, "quit") == 0) {
            break;
        } else {

        }
    }

    return 0;
}
