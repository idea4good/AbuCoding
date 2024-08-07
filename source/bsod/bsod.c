//cl.exe bsod.c ntoskrnl.lib /I .\ /link /out:bsod.sys /subsystem:native /driver:wdm -entry:DriverEntry
//sc create bsodDriver binpath=%cd%\bsod.sys type=kernel
//sc start bsodDriver

int func(int input)
{
	return 1/input;
}

#define _AMD64_
#include "wdm.h"

NTSTATUS DriverEntry(void* a, void* b)
{
	func(0);
}

/*
//cl.exe bsod.c user32.lib /link /entry:main
#include <Windows.h>
int main()
{
	func(0);
	MessageBoxA(NULL, "Abu Coding", NULL, MB_OK);
}
*/