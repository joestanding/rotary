global usermode_test
section .usermode_test
usermode_test:
.loop:
    xchg     bx, bx
    ;mov     eax, 0x01       ; write
    ;mov     edi, 1          ; STDIN
    ;mov     esi, 0xDEADBEEF ; Data buffer pointer
    ;mov     edx, 1024       ; Bytes to write

    ;int     64
    ;xchg    bx, bx
    jmp     .loop
