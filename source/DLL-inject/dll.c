// cl /LD /nologo dll.c user32.lib

#include <windows.h>

DWORD WINAPI task(LPVOID param)
{
	while(TRUE)
	{
		if(GetAsyncKeyState('A') & 0x80000)
		{
			MessageBoxA(NULL, "key A detected", "Info from DLL", MB_OK);
		}
	
		Sleep(100);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if(dwReason == DLL_PROCESS_ATTACH)
	{
		CreateThread(0, 0, task, hModule, 0, 0);
	}

    return TRUE;
}
