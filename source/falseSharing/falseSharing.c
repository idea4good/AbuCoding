// Measure-Command {.\falseSharing.exe}
// foreach ($i in 1..10) { Measure-Command {.\falseSharing.exe} }
// .\time.ps1 -exepath .\falseSharing.exe

/*
the same cluster: 
CPU 0 + CPU1 | false sharing: 5.9s
CPU 0 + CPU1 | cache-line isolated: 4.9s

cross-clusters: 
CPU 0 + CPU6 | same cache line: 5.7s
CPU 0 + CPU6 | cache-line isolated: 5.2s

single-core
false sharing: 9.2s
cache-line isolated: 9.2s
*/

#include <windows.h>
#include <stdio.h>

#define FalseSharing //only work for multi-cores

#ifdef FalseSharing
// same cache line
long counter1 = 0;
long counter2 = 0;
#else
// cross cache line
__declspec(align(64)) long counter1 = 0;
__declspec(align(64)) long counter2 = 0;
#endif

DWORD WINAPI thread_func(LPVOID arg) {
    long* counter = (long*) arg;
    while(*counter < 2000000000)
    {
        (*counter)++;
    }
    return 0;
}

int main() {
    HANDLE t1, t2;

    t1 = CreateThread(NULL, 0, thread_func, &counter1, 0, NULL);

    t2 = CreateThread(NULL, 0, thread_func, &counter2, 0, NULL);

    if (t1 == NULL || t2 == NULL) {
        printf("Failed to create threads\n");
        return -1;
    }

    SetThreadAffinityMask(t1, 0x1);  // pin to CPU 0
    SetThreadAffinityMask(t2, 0x2);  // pin to CPU 1

    // wait for both threads
    HANDLE threads[2] = { t1, t2 };
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    return 0;
}
