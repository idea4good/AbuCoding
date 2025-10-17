// cl /LD windbgExt.cpp /link
// .load windbgExt.dll
// !hello
// !readphys 0x18830000
// !writephys 0x18830000 1
// .unload windbgExt.dll

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
VOID WDBGAPI readphys(PVOID hCurrentProcess, PVOID hCurrentThread, ULONG dwCurrentPc, ULONG dwProcessor, PCSTR args)
{    
    if (!args || !args)
    {
        dprintf("Usage: !readphys address\n");
    }

    ULONG64 address = 0;
    if (sscanf_s(args, "%llx", &address) != 1)
    {
        dprintf("Invalid input. Please provide a hex address, e.g. !readphys 0x1234\n");
        return;
    }

    BYTE buffer[8] = {0};
    ULONG bytesRead = 0;
    ReadPhysical(address, buffer, sizeof(buffer), &bytesRead);

    if (bytesRead > 0)
    {
        dprintf("Read %lu bytes from physical address 0x%I64X:\n", bytesRead, address);
        for (ULONG i = 0; i < bytesRead; ++i)
        {
            dprintf("%02X ", buffer[i]);
        }
        dprintf("\n");
    }
    else
    {
        dprintf("Failed to read to physical address 0x%llx\n", address);
    }
}

extern "C" __declspec(dllexport)
VOID WDBGAPI writephys(PVOID hCurrentProcess, PVOID hCurrentThread, ULONG dwCurrentPc, ULONG dwProcessor, PCSTR args)
{
    ULONG64 address = 0;
    ULONG value = 0;

    if (!args || sscanf_s(args, "%llx %x", &address, &value) != 2) {
        dprintf("Usage: !writephys <hexaddr> <hexvalue>\n");
        return;
    }

    ULONG bytesWritten = 0;
    WritePhysical(address, &value, sizeof(value), &bytesWritten);

    if (bytesWritten > 0)
    {
        dprintf("Wrote %lu bytes to physical address 0x%llx\n", bytesWritten, address);
    }
    else
    {
        dprintf("Failed to write to physical address 0x%llx\n", address);
    }
}
