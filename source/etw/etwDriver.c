// cl.exe /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\km" /c /Zl /GS- /kernel /D "_AMD64_" /D "NTDDI_VERSION=0x0A00000C" /D "_WIN32_WINNT=0x0A00" etwDriver.c
// link.exe /SUBSYSTEM:NATIVE /DRIVER /NODEFAULTLIB /ENTRY:DriverEntry /OUT:etwDriver.sys etwDriver.obj ntoskrnl.lib

// traceview(realtime trace): file -> create new log session -> add provider -> input GUID(46175A05-20F9-4D43-9853-FE291E654A6F)

//sc create etwDriver binpath=%cd%\etwDriver.sys type=kernel
//sc start etwDriver
//sc stop etwDriver
//sc delete etwDriver

#include <ntddk.h>

static const GUID MyProviderGuid =
{ 0x46175A05, 0x20F9, 0x4D43, { 0x98, 0x53, 0xFE, 0x29, 0x1E, 0x65, 0x4A, 0x6F } };


// 全局 ETW 注册句柄
REGHANDLE g_EtwRegHandle = 0;

// 驱动卸载函数
VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);

    if (g_EtwRegHandle != 0) {
        EtwUnregister(g_EtwRegHandle);
        g_EtwRegHandle = 0;
    }

    DbgPrint("MyDriver: Unloaded and ETW unregistered.\n");
}

// 驱动入口函数
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject,
                     _In_ PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    DriverObject->DriverUnload = DriverUnload;

    NTSTATUS status = EtwRegister(
        &MyProviderGuid,   // Provider GUID
        NULL,              // 可选回调函数
        NULL,              // Context
        &g_EtwRegHandle    // 返回的句柄
    );

    if (!NT_SUCCESS(status)) {
        DbgPrint("MyDriver: EtwRegister failed (0x%x)\n", status);
        return status;
    }

    // 定义事件描述符（事件 ID = 1，级别 = 信息）
    EVENT_DESCRIPTOR eventDesc = {0};
    eventDesc.Id = 1;
    eventDesc.Version = 0;
    eventDesc.Channel = 0;
    eventDesc.Level = 1;//TRACE_LEVEL_INFORMATION;
    eventDesc.Opcode = 0;
    eventDesc.Task = 0;
    eventDesc.Keyword = 0;

    // 准备事件数据
    const char* message = "Hello from EtwWrite!";
    EVENT_DATA_DESCRIPTOR dataDesc;
    EventDataDescCreate(&dataDesc, message, (ULONG)strlen(message) + 1);

    // 写入事件
    status = EtwWrite(
        g_EtwRegHandle,
        &eventDesc,        // 事件描述符
        NULL,              // ActivityId（可为 NULL）
        1,                 // 数据项数量
        &dataDesc          // 数据项数组
    );

    if (!NT_SUCCESS(status)) {
        DbgPrint("MyDriver: EtwWrite failed (0x%x)\n", status);
    } else {
        DbgPrint("MyDriver: ETW event written successfully.\n");
    }

    return STATUS_SUCCESS;
}
