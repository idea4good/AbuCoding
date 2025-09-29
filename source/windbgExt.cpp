// cl /LD windbgExt.cpp /link
// .load windbgExt.dll
// !hello

#include <windows.h>
#include <wdbgexts.h>
#include <stdio.h>

WINDBG_EXTENSION_APIS ExtensionApis = {0};
EXT_API_VERSION g_ExtApiVersion = {1, 0, EXT_API_VERSION_NUMBER64, 0};

extern "C" __declspec(dllexport)
LPEXT_API_VERSION WDBGAPI ExtensionApiVersion()
{
    return &g_ExtApiVersion;
}

extern "C" __declspec(dllexport)
VOID WDBGAPI WinDbgExtensionDllInit(PWINDBG_EXTENSION_APIS lpExtensionApis, USHORT MajorVersion, USHORT MinorVersion)
{
    ExtensionApis = *lpExtensionApis;
}

extern "C" __declspec(dllexport)
VOID hello()
{
    dprintf("Hello from your WinDbg extension!\n");
}

extern "C" __declspec(dllexport)
VOID WDBGAPI readphys(PCHAR args)
{
    ULONG64 physAddr = 0x18830000;

    BYTE buffer[8] = {0};
    ULONG bytesRead = 0;

    ReadPhysical(physAddr, buffer, sizeof(buffer), &bytesRead);

    dprintf("Read %lu bytes from physical address 0x%I64X:\n", bytesRead, physAddr);
    for (ULONG i = 0; i < bytesRead; ++i) {
        dprintf("%02X ", buffer[i]);
    }
    dprintf("\n");
}
