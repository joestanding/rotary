; Multiboot Header and Initial Boot
; ---------------------------------
; In this file, we define the header structures that GRUB will look for to identify Multiboot
; enabled operating systems. We define them as their own sections, so that we can reference
; them in our linker script and define their location within our final operating system image.
;
; Once we've defined our Multiboot headers, we need to configure our initial page tables to 
; identity map our kernel (loaded at 0x00100000), and map it to 0xC0000000. Identity mapping this
; will ensure our boot code is still addressable once we enable paging.
;
; Finally, once paging has been enabled and the page tables have been assigned, we'll jump to the kernel entry.
MAGIC               equ 0x1BADB002
FLAG_ALIGN          equ 1 << 0
FLAG_MEMINFO        equ 1 << 1
FLAGS               equ FLAG_ALIGN | FLAG_MEMINFO
CHECKSUM            equ -(MAGIC + FLAGS)
KERNEL_PHYS_ADDR    equ 0x00100000

; Insert the Multiboot header
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Define our stack
section .bss
alignb 16
global KERNEL_STACK_BOTTOM
global KERNEL_STACK_TOP
KERNEL_STACK_BOTTOM:
resb 32768
KERNEL_STACK_TOP:

section .text
global initial_page_directory
align 4096
page_directory:
times 4096 dw 0
page_table:
times 4096 dw 0

; Define our hand-off point for Multiboot, from which we can begin OS initialisation, enable
; paging, and then finally hand off to the C kernel
global _start
extern kernel_main
_start:
    ; Configure our stack pointer
    mov esp,    KERNEL_STACK_TOP

    ; Push the Multiboot values onto the stack for use by kernel_main, which is defined as:
    ; kernel_main(uint32 mboot_magic, void * mboot_info)
    push    ebx
    push    eax

    ; As we're going into protected mode, we'll need some initial mapping. We'll amend it after boot.
    ; We'll need to:
    ;   - Identity map the lowest 4MB of phys. memory, which is where this code is running
    ;   - Map 0x00000000 -> 0x003FFFFF    to    virtual 0xC0000000 -> 0xC03FFFFF

    mov eax, 0x0 ; Our index
    mov ebx, 0x0 ; Our kernel address
    .fill_table:
        mov ecx, ebx
        or ecx, 3                   ; Set the present and writable bits
        mov [page_table+eax*4], ecx  ; Write the phys. mem address with flags to the table
        add ebx, 4096               ; Iterate to the next 4KB of memory to address
        inc eax                     ; Increment our index iterator
        cmp eax, 1024               ; If we're at 1024, we've filled the table
        je .end                     ; We're done
        jmp .fill_table
    .end:

    ; Add our identity mapping to the page directory
    mov dword [page_directory + (0 * 4)], page_table + 0x003

    ; Add our kernel mapping to the page directory
    mov dword [page_directory + (768 * 4)], page_table + 0x003

    ; Configure CR3 to point to our initial page directory
    mov     eax, page_directory
    mov     cr3, eax

    ; Configure CR4 to enable Page Size Extensions and Page Global Enable
    mov     eax, cr4
    or      eax, 0x90
    mov     cr4, eax

    ; Enable paging
    mov     eax, cr0
    or      eax, 0x80000000
    mov     cr0, eax

    ; Update our stack pointer to be within the higher half
    add     esp, 0xC0000000

    ; Jump to our kernel
    call kernel_main
    jmp $
