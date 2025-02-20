//cl.exe .\kernelDriver.c ntoskrnl.lib /I .\ /link /out:kernelDriver.sys /subsystem:native /driver:wdm -entry:DriverEntry

//sc create abuDriver binpath=%cd%\kernelDriver.sys type=kernel
//sc start abuDriver
//sc stop abuDriver
//sc delete abuDriver

#define _AMD64_
#include "wdm.h"

const char* secret = "Hello AbuCoding";

NTSTATUS DriverEntry(void* a, void* b)
{
	DbgPrint("%p, %s", secret, secret);
}
