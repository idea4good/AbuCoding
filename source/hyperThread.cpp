// cl.exe hyperThread.cpp

#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>

void bind_cpu(int cpu)
{
    SetThreadAffinityMask(GetCurrentThread(), (DWORD_PTR)1 << cpu);
}

void work(int cpu)
{
    bind_cpu(cpu);

    volatile unsigned long long x = 1;

    for (long long i = 0; i < 2000000000LL; ++i)
    {
        x *= 3;
        x += 1;
        x *= 7;
        x += 3;
    }
}

double test(int cpu1, int cpu2)
{
    auto start = std::chrono::steady_clock::now();

    std::thread A(work, cpu1);
    std::thread B(work, cpu2);

    A.join();
    B.join();

    auto end = std::chrono::steady_clock::now();

    return std::chrono::duration<double>(end - start).count();
}

int main()
{
    // CPU 0 + CPU 1: Assume they belong to the same physical core
    double smt = test(0, 1);

    // CPU 0 + CPU 2: Assume they belong to two different physical cores
    double physical = test(0, 2);

    std::cout << "2 hyper threads : " << smt << " seconds\n";

    std::cout << "2 physical cores: " << physical << " seconds\n";

    std::cout << "Performance Gain from Physical Cores: " << 100 * (smt - physical) / smt << "%\n";
}