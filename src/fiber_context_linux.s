#  @file fiber_context_linux.s
#
#  Copyright 2011 Alex Shen. Distributed under the Boost
#  Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

.text
.global fiber_get_context
# fiber_get_context(fiber_context* context)
fiber_get_context:
    # save current stack pointer to context
    movl 0x4(%esp), %ecx # fixup, point to the argument, ignore return address
    movl %ebp, (%ecx) # context->ebp
    movl %esp, %eax
    # fixup esp, ignore return address, as the eip is set to the caller's address
    addl $0x4, %eax
    movl %eax, 0x4(%ecx) # context->esp
    movl (%esp), %eax
    movl %eax, 0x8(%ecx) # context->eip
    # save callee-save general-purpose registers
    movl %ebx,  0xc(%ecx) # context->ebx
    movl %esi, 0x10(%ecx) # context->esi
    movl %edi, 0x14(%ecx) # context->edi
    ret

.end
