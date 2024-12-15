//gcc atomic.c -lpthread

#include <pthread.h>
#include <stdio.h>

int a;
//_Atomic int a; //uncomment this code to fix race condition

void* sum()
{
    for(int i = 0; i < 10000000; i++)
    {
        a++;
    }
}

void test()
{
    a = 0;
    pthread_t t1, t2;
    pthread_create(&t1, NULL, sum, NULL);
    pthread_create(&t2, NULL, sum, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("a = %d\n", a);
}

int main()
{
    test();
    test();
    test();
}