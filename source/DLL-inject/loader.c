// cl /nologo loader.c user32.lib

#include <Windows.h>
#include <tlhelp32.h>

HANDLE searchProcess(PCHAR ProcessName)
{
    PROCESSENTRY32 pe = { .dwSize = sizeof(PROCESSENTRY32) };
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (strcmp(pe.szExeFile, ProcessName) == 0) {
                printf("%s has been opened\n", pe.szExeFile);
                return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return NULL;
}

int main()
{
	HANDLE handle = searchProcess("Notepad.exe");

    LPCSTR DllPath = "C:\\Users\\blues\\Desktop\\dll.dll";//change the path
	LPVOID DllPathForNotepad = VirtualAllocEx(handle, 0, strlen(DllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(handle, DllPathForNotepad, (LPVOID)DllPath, strlen(DllPath) + 1, 0);

    LPTHREAD_START_ROUTINE remoteTask = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA");
	HANDLE hLoadThread = CreateRemoteThread(handle, 0, 0, remoteTask, DllPathForNotepad, 0, 0);
}
