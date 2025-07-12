# How to demo

## Please shutdown the Windows real-time protection for this demo.

1. change the **DLL path** on line 27 in loader.c 
2. cl /LD /nologo dll.c user32.lib
3. cl /nologo loader.c user32.lib
4. open notepad
5. run loader.exe
6. Press 'A' on keyboard
