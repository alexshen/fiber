.text
.global fiber_get_context
# fiber_get_context(fiber_context* context)
fiber_get_context:
    pushl %eax
    pushl %ebx
    # save current stack pointer to context
    movl 0xc(%esp), %ebx # fixup, point to the argument, ignore ebx, eax
    movl %ebp, (%ebx) # context->ebp
    movl %esp, %eax
    # fixup esp, ignore ebx, eax, return address, as the eip is set to the caller's address
    addl $0xc, %eax
    movl %eax, 0x4(%ebx) # context->esp
    movl 0x8(%esp), %eax
    movl %eax, 0x8(%ebx) # context->eip
    # save general-purpose registers
    movl 0x4(%esp), %eax
    movl %eax, 0xc(%ebx) # context->eax
    movl (%esp), %eax
    movl %eax, 0x10(%ebx) # context->ebx
    movl %ecx, 0x14(%ebx) # context->ecx
    movl %edx, 0x18(%ebx) # context->edx
    movl %esi, 0x1c(%ebx) # context->esi
    movl %edi, 0x20(%ebx) # context->edi
    popl %ebx
    popl %eax
    ret

.end
