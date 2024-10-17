// gcc -fstack-protector canary.c
// ./a.out 12345678901234
// ./a.out 123456789012345
// ./a.out 1234567890123456789012345

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CANARY_VALUE 0xFE

// Function to simulate a vulnerable function with a canary
void vulnerableFunction(char *input) {
    // Set up a canary value
    char buffer[16];
    buffer[15] = CANARY_VALUE;

    // Simulate an unsafe copy which might cause buffer overflow
    strcpy(buffer, input);

    // Check if the canary value has been altered
    if (buffer[15] != (char)CANARY_VALUE) {
        printf("Stack smashing detected! Program will terminate.\n");
    } else {
        printf("No overflow detected. Function executed safely.\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input>\n", argv[0]);
        return 1;
    }

    // Call the vulnerable function with user input
    vulnerableFunction(argv[1]);

    return 0;
}
