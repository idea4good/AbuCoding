// cl /nologo ThreadPin.c user32.lib ntdll.lib

#include <Windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <stdio.h>

typedef struct _THREAD_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PVOID TebBaseAddress;
    CLIENT_ID ClientId;
    ULONG_PTR AffinityMask;
    LONG Priority;
    LONG BasePriority;
} THREAD_BASIC_INFORMATION;

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

HANDLE EnumThread(DWORD pid, DWORD tid)
{
	THREADENTRY32 te = { .dwSize = sizeof(THREADENTRY32) };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	Thread32First(hSnapshot, &te);
	while(Thread32Next(hSnapshot, &te))
	{
		if(te.th32OwnerProcessID == pid && te.th32ThreadID == tid)
		{
			printf("✅ the thread found\n");
			return OpenThread(THREAD_SET_INFORMATION | THREAD_QUERY_INFORMATION, FALSE, te.th32ThreadID);
		}
	}

	CloseHandle(hSnapshot);
	return 0;
}

void printThreadInfo(HANDLE hThread)
{
    THREAD_BASIC_INFORMATION tbi;
    ULONG retLen = 0;

    if (NtQueryInformationThread(hThread, 0, &tbi, sizeof(tbi), &retLen) == 0) 
    {
        printf("✅ Process ID: 0x%llX | Thread ID: 0x%llX | Affinity Mask: 0x%llX | Priority: %ld\n", (unsigned long long)tbi.ClientId.UniqueProcess, (unsigned long long)tbi.ClientId.UniqueThread, (unsigned long long)tbi.AffinityMask, tbi.Priority);
    }
    else
    {
        printf("NtQueryInformationThread failed with error: 0x%X\n", GetLastError());
    }
}

int main(int argc, char *argv[])
{
	if(argc != 4 && argc != 3)
	{
		printf ("❌ Invalid parameters!\nUsage: ThreadPin.exe <ProcessName> <ThreadId> [AffinityMask]\nExample: ThreadPin.exe Notepad.exe 0x1234 0xF");
		return -1;
	}

	DWORD targetProcessID = searchProcessID(argv[1]);
	if(!targetProcessID)
	{
		printf("❌ %s not found!\n🚀 Run it first\n", argv[1]);
		return -2;
	}

	char *endptr;
	DWORD targetThreadID = strtol(argv[2], &endptr, 0);
	HANDLE hTargetThread = EnumThread(targetProcessID, targetThreadID);
	if(!hTargetThread)
	{
		printf("❌ Get thread Handle failed\n");
		return -3;
	}

	if(argc == 3)
	{
		printThreadInfo(hTargetThread);
		return 0;
	}

	DWORD AffinityMask = strtol(argv[3], &endptr, 0);
	if(0 == SetThreadAffinityMask(hTargetThread, AffinityMask))
	{
		printf("❌ SetThreadAffinityMask failed: %lu\n",  GetLastError());
	}
	else
	{
		printf("✅ SetThreadAffinityMask succeed\n");
	}
}
