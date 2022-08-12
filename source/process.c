//step 1: echo 0 > /proc/sys/kernel/randomize_va_space
//step 2: gcc process.c -o p1 && ./p1
//step 3: int a = 2;
//step 4: gcc process.c -o p2 && ./p2

#include <stdio.h>
#include <unistd.h>

int a = 1;//or 2
int main()
{
    printf("address: %p, value: %d\n", &a, a);
}
