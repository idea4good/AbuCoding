// cl.exe /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\km" /c /Zl /GS- /kernel /D "_AMD64_" /D "NTDDI_VERSION=0x0A00000C" /D "_WIN32_WINNT=0x0A00" etwDriver.c
// link.exe /SUBSYSTEM:NATIVE /DRIVER /NODEFAULTLIB /ENTRY:DriverEntry /OUT:etwDriver.sys etwDriver.obj ntoskrnl.lib

//sc create etwDriver binpath=%cd%\etwDriver.sys type=kernel
//sc start etwDriver
//sc stop etwDriver
//sc delete etwDriver

#include "ntddk.h"
#include <TraceLoggingProvider.h>

TRACELOGGING_DEFINE_PROVIDER(
    g_hProvider,
    "abuEtwProvider", // Provider name
    // Generate your own GUID with uuidgen.exe or guidgen.exe
    (0x46175A05, 0x20F9, 0x4D43, 0x98, 0x53, 0xFE, 0x29, 0x1E, 0x65, 0x4A, 0x6F)
);

VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);

    TraceLoggingWrite(
        g_hProvider,
        "DriverUnload",
        TraceLoggingString("Driver is unloading", "Message"));

    TraceLoggingUnregister(g_hProvider);
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    // Register the provider
    TraceLoggingRegister(g_hProvider);

    // Emit an event
    TraceLoggingWrite(
        g_hProvider,
        "DriverEntry",
        TraceLoggingString("Driver loaded successfully", "Message"),
        TraceLoggingInt32(123, "InitValue"));

    DriverObject->DriverUnload = DriverUnload;
    return STATUS_SUCCESS;
}
