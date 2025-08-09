// cl /nologo hijack.c user32.lib
// double checkout the address of MessageBoxA, correct the address in line 52 if needed.

#include <Windows.h>
#include <tlhelp32.h>
#include <stdio.h>

typedef int (WINAPI *MessageBoxAFunc)(HWND, LPCSTR, LPCSTR, UINT);
MessageBoxAFunc msgFunc;
DWORD64 returnAddress;

DWORD searchProcessID(PCHAR ProcessName)
{
    PROCESSENTRY32 pe = { .dwSize = sizeof(PROCESSENTRY32) };
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (strcmp(pe.szExeFile, ProcessName) == 0) {
                printf("%s found\n", pe.szExeFile);
                return pe.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return 0;
}

HANDLE EnumThread(DWORD pid)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	THREADENTRY32 te;
	te.dwSize = sizeof(te);

	Thread32First(hSnapshot, &te);

	while(Thread32Next(hSnapshot, &te))
	{
		if(te.th32OwnerProcessID == pid)
		{
			return OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, te.th32ThreadID);
		}
	}

	return 0;
}

void yourCode()
{
	MessageBoxAFunc msgFunc = (MessageBoxAFunc)0x00007FF9D088D230;//Do not call or reference MessageBoxA, as it uses a relative address.
	msgFunc(NULL, 0, 0, MB_OK | MB_ICONINFORMATION);
	while(1);
}

void hijackRemoteThread(HANDLE remoteThread, LPVOID codeAddress)
{
	SuspendThread(remoteThread);

	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_CONTROL;
	GetThreadContext(remoteThread, &ctx);
	returnAddress = ctx.Rip;
	
	ctx.Rip = (DWORD64)codeAddress;
	SetThreadContext(remoteThread, &ctx);

	ResumeThread(remoteThread);
}

int main()
{
	DWORD processID = searchProcessID("Notepad.exe");
	HANDLE hThread = EnumThread(processID);
	HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, processID);
	LPVOID remoteMemory = VirtualAllocEx(hProcess, 0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if(!remoteMemory || !hThread)
	{
		printf("Error: failed to open thread or allocate memory.\n");
		return -1;
	}

	printf("address of MessageBoxA = %p\n", MessageBoxA);

	WriteProcessMemory(hProcess, remoteMemory, (LPVOID)yourCode, 0x100, 0);
	hijackRemoteThread(hThread, remoteMemory);
}
