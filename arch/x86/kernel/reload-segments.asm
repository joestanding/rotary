global gdt_flush
global tss_flush

gdt_flush:
    jmp     0x08:.gdt_reload_ds
.gdt_reload_ds:
    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     ss, ax
    ret

tss_flush:
    mov     ax, 0x28
    ltr     ax
    ret
