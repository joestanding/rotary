global task_context_switch
extern tss

; This routine switches tasks by saving registers that are not
; already preserved by the cdecl calling convention after this
; routine is called by task_schedule().

; The routine then stores the current stack pointer in the
; data structure for the current task, and updates ESP to the
; stored stack pointer in the structure for the next task.

; Once the stack pointer is updated, the following pops and
; ret will retrieve stored values from the next task's stack,
; including a return address that will transition EIP to the
; next task's code.

%define TASK_OFFSET_KSTACK_TOP 20
%define TASK_OFFSET_KSTACK_BOT 24
%define TASK_OFFSET_VM_SPACE   32

%define TSS_OFFSET_ESP0        4

task_context_switch:
    ; EAX = current task
    mov     eax, [esp + 4]
    ; EDX = next task
    mov     edx, [esp + 8]

    ; ------------------------------------------------------------- ;
    ; Storing Existing Task State
    ; --------------------------------------------------------------;

    push    ebp
    push    ebx
    push    esi
    push    edi

    ; We need to save the stack pointer of the task we're
    ; switching away from (kernel_esp is second element of task struct)
    mov     [eax + TASK_OFFSET_KSTACK_TOP], esp

    ; ------------------------------------------------------------- ;
    ; Loading Next Task State
    ; --------------------------------------------------------------;

    ; Now we need to load the stack pointer of the next task
    mov     esp, [edx + TASK_OFFSET_KSTACK_TOP]

    ; Retrieve a pointer to the page table within the task's vm_space struct
    mov     ebx, [edx + TASK_OFFSET_VM_SPACE]
    mov     ebx, [ebx]

    ; Check whether the task uses a different page directory
    mov     ecx, cr3
    cmp     ecx, ebx
    je      .end_task_context_switch

    ; Task uses a diff. PD, update CR3 to point to the new page directory
    mov     cr3, ebx

.end_task_context_switch:
    ; Restore registers that aren't preserved by cdecl
    pop     edi
    pop     esi
    pop     ebx
    pop     ebp

    ; As we've changed ESP to that of the other task's stack,
    ; the return address on the stack that will be popped by
    ; this ret will take us into that task's code instead.
    ret
