// cl /LD /nologo dll.c user32.lib

#include <windows.h>

DWORD WINAPI thread(LPVOID param)
{
	while(TRUE)
	{
		if(GetAsyncKeyState('A') & 0x80000)
		{
			MessageBoxA(NULL, "A pressed", "A pressed", MB_OK);
		}
	
		Sleep(100);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if(dwReason == DLL_PROCESS_ATTACH)
	{
		CreateThread(0, 0, thread, hModule, 0, 0);
	}

    return TRUE;
}
