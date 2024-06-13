//Build with Visual Studio choose:X86

#include <stdio.h>
#include <Windows.h>
#include <thread>

int v1, v2, r1, r2;
HANDLE start1, start2, complete;

void thread1()
{
    while (true)
    {
        WaitForSingleObject(start1, INFINITE); //wait for start
        v1 = 1;

        //__asm { mfence }

        r1 = v2;
        ReleaseSemaphore(complete, 1, NULL); //complete & trigger a signal
    }
}

void thread2()
{
    while (true)
    {
        WaitForSingleObject(start2, INFINITE);  //wait for start
        v2 = 1;

        //__asm { mfence }

        r2 = v1;
        ReleaseSemaphore(complete, 1, NULL); //complete & trigger a signal
    }
}

int main()
{
    start1 = CreateSemaphore(NULL, 0, 1, NULL);
    start2 = CreateSemaphore(NULL, 0, 1, NULL);
    complete = CreateSemaphore(NULL, 0, 2, NULL);

    std::thread t1(thread1);
    std::thread t2(thread2);

    for (int i = 0; i < 300000; i++)
    {
        v1 = v2 = 0;

        ReleaseSemaphore(start1, 1, NULL);  //start t1
        ReleaseSemaphore(start2, 1, NULL);  //start t2

        //wait for t1&t2 completion
        WaitForSingleObject(complete, INFINITE);
        WaitForSingleObject(complete, INFINITE);

        if ((r1 == 0) && (r2 == 0))
        {
            printf("reorder detected @ %d\n", i);
        }
    }

    t1.detach();
    t2.detach();
}
