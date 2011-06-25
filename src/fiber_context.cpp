#include "fiber_context.h"
#include <cassert>
#include <cstring>
using namespace std;

#if defined(FIBER_X86)
void fiber_make_context(fiber_context* context, fiber_entry entry, void* arg)
{
    assert(context && entry);
    // default alignment of the stack
    const int alignment = 16;

    context->esp = (reinterpret_cast<uintptr_t>(context->stack) + context->stack_size) & ~(alignment - 1);
    context->ebp = context->esp;
    context->eip = reinterpret_cast<uintptr_t>(entry);
    context->userarg = arg;

    // push the argument onto the stack
    char* top = reinterpret_cast<char*>(context->esp);
    memcpy(top, &context->userarg, sizeof(void*));
    // make space for the pushed argument
    context->esp -= sizeof(void*);

    // clear all general purpose registers
    context->eax = context->ebx = context->ecx
                 = context->edx = context->esi
                 = context->edi = 0;
}

#if defined(_MSC_VER)
// We're naked...
__declspec(naked) void fiber_get_context(fiber_context* context)
{
    // TODO: how much space need to reserve for assert ?
    //assert(context);
    // save the current context in `context' and return
    __asm {
        push eax
        push ebx
        // save current stack pointer to context
        mov ebx, [esp + 0xc] ; fixup, point to the argument, ignore ebx, eax
        mov dword ptr [ebx], ebp ; context->ebp
        mov eax, esp
        // fixup esp, ignore ebx, eax, return address, as the eip is set to the caller's address
        add eax, 0xc 
        mov dword ptr [ebx + 0x4], eax ; context->esp
        mov eax, [esp + 0x8]
        mov dword ptr [ebx + 0x8], eax ; context->eip
        // save general-purpose registers
        mov eax, dword ptr [esp + 0x4]
        mov dword ptr [ebx + 0xc], eax ; context->eax
        mov eax, dword ptr [esp]
        mov dword ptr [ebx + 0x10], eax; context->ebx
        mov dword ptr [ebx + 0x14], ecx; context->ecx
        mov dword ptr [ebx + 0x18], edx; context->edx
        mov dword ptr [ebx + 0x1c], esi; context->esi
        mov dword ptr [ebx + 0x20], edi; context->edi
        pop ebx
        pop eax
        ret
    }
}

#pragma warning(push)
#pragma warning(disable : 4731) // inline assembly modifies ebp
void fiber_swap_context(fiber_context* oldcontext, fiber_context* newcontext)
{
    assert(oldcontext && newcontext);
    // save the current context in the oldcontext and set the current context from newcontext
    __asm {
        push oldcontext
        call fiber_get_context
        // fixup oldcontext->esp, ignore pushed arguments, since we'll resume from restore
        push eax
        mov eax, oldcontext
        add dword ptr[eax + 0x4], 0x4
        // fixup return address
        mov dword ptr[eax + 0x8], offset restore
        pop eax
        // switch to newcontext
        push newcontext
        call fiber_set_context
restore:
    }
}

void fiber_set_context(fiber_context* context)
{
    __asm {
        ; restore the enviroment for context
        mov ebx, context
        mov ebp, dword ptr [ebx]        ; context->ebp
        mov esp, dword ptr [ebx + 0x4]  ; context->esp
        ; restore general-purpose registers
        mov eax, dword ptr [ebx + 0xc]  ; context->eax
        mov ecx, dword ptr [ebx + 0x10] ; context->ecx
        mov edx, dword ptr [ebx + 0x14] ; context->edx
        mov esi, dword ptr [ebx + 0x18] ; context->esi
        mov edi, dword ptr [ebx + 0x20] ; context->edi
        push dword ptr [ebx + 0x8]      ; context->eip
        mov ebx, dwrod ptr [ebx + 0x10] ; context->ebx
        ret
        ; should never return here
    }
}
#pragma warning(pop)

#elif defined(__GNUC__)
void fiber_swap_context(fiber_context* oldcontext, fiber_context* newcontext)
{
    assert(oldcontext && newcontext);
    // save the current context in the oldcontext
    asm (
        "pushl %0;"
        "call fiber_get_context;"
        // fixup oldcontext->esp, ignore pushed arguments, since we'll resume from restore
        "pushl %%eax;"
        "movl %0, %%eax;"
        "addl $4, 0x4(%%eax);"
        // fixup return address
        "movl $restore, 0x8(%%eax);"
        "popl %%eax;"
        // switch to newcontext
        "pushl %1;"
        "call fiber_set_context;"
        "restore:;"
        :: "m"(oldcontext), "m"(newcontext)
     );
}

void fiber_set_context(fiber_context* context)
{
    asm (
        // restore the enviroment for newcontext
        "movl          %0, %%ebx;"
        "movl     (%%ebx), %%ebp;" // context->ebp
        "movl  0x4(%%ebx), %%esp;" // context->esp
        "movl  0xc(%%ebx), %%eax;" // context->eax
        "movl 0x10(%%ebx), %%ecx;" // context->ecx
        "movl 0x14(%%ebx), %%edx;" // context->edx
        "movl 0x18(%%ebx), %%esi;" // context->esi
        "movl 0x20(%%ebx), %%edi;" // context->edi
        "pushl 0x8(%%ebx);"        // context->eip
        "movl 0x10(%%ebx), %%ebx;" // context->ebx
        "ret;"
        // should never return here
        :: "m"(context)
    );
}
#else
#  error Unsupported compiler
#endif

#else // !FIBER_X86
#  error Unsupported platform
#endif // FIBER_X86
