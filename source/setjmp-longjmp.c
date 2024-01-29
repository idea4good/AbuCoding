//gcc setjmp-longjmp.c

#include <stdio.h>

__attribute__((naked,returns_twice))
int my_setjmp(void* context)
{
    asm("mov %%rbp, (%%rdi);"
        "mov %%rsp, 8(%%rdi);"
        "mov (%%rsp), %%rax;"
        "mov %%rax, 16(%%rdi);"
        "mov $0, %%rax;"
        "ret;"
        :::);
}

__attribute__((naked,noreturn))
void my_longjmp(void* context, int value)
{
    asm("mov (%%rdi), %%rbp;"
        "mov 8(%%rdi), %%rsp;"
        "mov %%rsi, %%rax;"
        "jmp 16(%%rdi);"
        :::);
}

long context[3];

void func2()
{  
    printf("func2 start\n");
    my_longjmp(context, 100);
    printf("func2 return\n");
}

void func1()
{  
    printf("func1 start\n");
    func2();
    printf("func1 return\n");
}

int main()
{
    printf("main start\n");
    int value = my_setjmp(context);
    if(value == 0 )
    {
        func1();
    }
    printf("main return\n");
    return value;
}
