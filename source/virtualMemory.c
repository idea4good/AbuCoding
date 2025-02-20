//step 1: echo 0 > /proc/sys/kernel/randomize_va_space
//step 2: gcc virtualMemory.c -o p1 && ./p1
//step 3: int a = 2;
//step 4: gcc virtualMemory.c -o p2 && ./p2
//we will see the same address, but the value is different.

#include <stdio.h>

int a = 1;//or 2
int main()
{
    printf("address: %p, value: %d\n", &a, a);
}
