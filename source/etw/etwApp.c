// cl.exe etwApp.c
// traceview(realtime trace): file -> create new log session -> add provider -> input GUID(46175A05-20F9-4D43-9853-FE291E654A6F)
// run etwApp.exe

// xperf.exe -start trace -f abu.etl -on *abuEtwProvider
// run etwApp.exe
// xperf.exe -stop trace
// tracefmt.exe -displayonly abu.etl # looks failed

#include <windows.h>
#include <TraceLoggingProvider.h>

#pragma comment(lib, "Advapi32.lib")

// Declare a TraceLogging provider with a unique GUID
TRACELOGGING_DEFINE_PROVIDER(
    g_hProvider,
    "abuEtwProvider", // Provider name
    // Generate your own GUID with uuidgen.exe or guidgen.exe
    (0x46175A05, 0x20F9, 0x4D43, 0x98, 0x53, 0xFE, 0x29, 0x1E, 0x65, 0x4A, 0x6F)
);

int main(void)
{
    // Register the provider
    TraceLoggingRegister(g_hProvider);

    // Emit a simple event
    TraceLoggingWrite(g_hProvider, "HelloEvent", TraceLoggingString("Hello ETW!", "Message"), TraceLoggingInt32(123, "Value"));

    // Unregister when done
    TraceLoggingUnregister(g_hProvider);

    return 0;
}
