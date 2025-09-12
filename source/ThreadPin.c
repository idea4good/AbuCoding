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

void printThreadInfo(HANDLE hThread)
{
    THREAD_BASIC_INFORMATION tbi;
    ULONG retLen = 0;

    if (NtQueryInformationThread(hThread, 0, &tbi, sizeof(tbi), &retLen) == 0) 
    {
        printf("✅ Process ID: %llu | Thread ID: %llu | Affinity: 0x%llX | Priority: %ld\n", (unsigned long long)tbi.ClientId.UniqueProcess, (unsigned long long)tbi.ClientId.UniqueThread, (unsigned long long)tbi.AffinityMask, tbi.Priority);
    }
    else
    {
        printf("❌ NtQueryInformationThread failed with error: 0x%X\n", GetLastError());
    }
}

HANDLE pinThread(DWORD pid, DWORD tid, DWORD affinityMask)
{
	THREADENTRY32 te = { .dwSize = sizeof(THREADENTRY32) };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	BOOL found = FALSE;
	Thread32First(hSnapshot, &te);
	while(Thread32Next(hSnapshot, &te))
	{
		if(te.th32OwnerProcessID == pid && (te.th32ThreadID == tid || 0 == tid))
		{
			printf("✅ Thread  %lu found\n", te.th32ThreadID);
			found = TRUE;

			HANDLE hTargetThread = OpenThread(THREAD_SET_INFORMATION | THREAD_QUERY_INFORMATION, FALSE, te.th32ThreadID);
			if(0 == hTargetThread)
			{
				printf("❌ OpenThread failed! thread ID = %lu, error = %lu\n", te.th32ThreadID, GetLastError());
				continue;
			}

			if(0 == affinityMask)
			{
				printThreadInfo(hTargetThread);
				continue;
			}

			if(0 == SetThreadAffinityMask(hTargetThread, affinityMask))
			{
				printf("❌ SetThreadAffinityMask failed! thread ID = %lu, error = %lu\n", te.th32ThreadID, GetLastError());
			}
			else
			{
				printf("✅ SetThreadAffinityMask succeed! thread ID = %lu\n", te.th32ThreadID);
			}

			if(0 != tid)
			{
				break;
			}
		}
	}

	if(!found)
	{
		printf("❌ Thread %lu not found, check the parameter first\n", tid);
	}

	CloseHandle(hSnapshot);
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc != 4 && argc != 3)
	{
		printf ("❌ Invalid parameters!\nUsage: ThreadPin.exe <ProcessName> <ThreadId> [Affinity]\nThreadId = 0 -> Apply to all threads\nExample: ThreadPin.exe Notepad.exe 1234 0xF");
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

	DWORD affinityMask = 0;
	if(argc == 4)
	{
		affinityMask = strtol(argv[3], &endptr, 0);
	}

	pinThread(targetProcessID, targetThreadID, affinityMask);
}
