//Linux: g++ outOfOrder.cpp -pthread
//Windows: cl outOfOrder.cpp

//run on single CPU core 
//Linux: taskset -c 0 ./a.out
//Windows: start /affinity 0x1 outOfOrder.exe

#include <iostream>
#include <thread>

using namespace std;

int x = 0, y = 0, a = 0, b = 0;

void thread1()
{
    x = 1;
    a = y;
}

void thread2()
{
    y = 1;
    b = x;
}

int main()
{
    size_t round = 0;

    do
    {
        x = y = 0;

        thread first(thread1);
        thread second(thread2);

        first.join();
        second.join();
        ++round;
    }
    while(!(a == 0 && b == 0));

    cout << "a = " << a << ", b = " << b << " at round:" << round << endl;

    getchar();
}
