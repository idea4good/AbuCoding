//g++ memFence.cpp --std=c++20 -lpthread
//g++ memFence.cpp --std=c++20 -lpthread -O3
#include <stdio.h>
#include <semaphore.h>
#include <thread>

int v1, v2, r1, r2;
sem_t start1, start2, complete;

void thread1()
{
    while(true)
    {
        sem_wait(&start1);  //wait for start
        v1 = 1;
        
        //asm ("mfence" ::: "memory");

        r1 = v2;
        sem_post(&complete);  //complete & trigger a signal
    }
}

void thread2()
{
    while(true)
    {
        sem_wait(&start2);  //wait for start
        v2 = 1;
        
        //asm ("mfence" ::: "memory");

        r2 = v1;
        sem_post(&complete);  //complete & trigger a signal
    }
}

int main()
{
    sem_init(&start1, 0, 0);
    sem_init(&start2, 0, 0);
    sem_init(&complete, 0, 0);

    std::thread t1(thread1);
    std::thread t2(thread2);

    for(int i = 0; i < 300000; i++)
    {
        v1 = v2 = 0;
        
        sem_post(&start1);  //start t1
        sem_post(&start2);  //start t2

        //wait for t1&t2 completion
        sem_wait(&complete);
        sem_wait(&complete);

        if((r1 == 0) && (r2 == 0))
        {
            printf("reorder detected @ %d\n", i);
        }
    }

    t1.detach();
    t2.detach();
}
