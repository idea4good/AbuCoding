# 1) apt install nasm
# 2) nasm -f elf64 syscall.s  && ld syscall.o
# 3) ./a.out

section .text
  global _start
    _start:
      mov rdi, 1                    ;arg0
      mov rdx, 8                    ;arg2
      mov rbx, 0x0a646c726f576948   ;arg1 = HiWorld\n
      ;mov rbx, 0x0a83989ff03a6948  ;arg1 = Hi:😃\n
      push rbx
      mov rsi, rsp
      mov rax, 1
      syscall

      mov rax, 60                   ;exit
      syscall
      
