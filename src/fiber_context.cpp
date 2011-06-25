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

    // clear all callee-save general purpose registers
    context->ebx = context->esi = context->edi = 0;
}

#if defined(_MSC_VER)
// We're naked...
__declspec(naked) void fiber_get_context(fiber_context* context)
{
    // TODO: how much space need to reserve for assert ?
    //assert(context);
    // save the current context in `context' and return
    __asm {
        // save current stack pointer to context
        mov ecx, dword ptr [esp + 0x4] ; fixup, point to the argument, ignore return address
        mov dword ptr [ecx], ebp ; context->ebp
        mov eax, esp
        // fixup esp, ignore return address, as the eip is set to the caller's address
        add eax, 0x4
        mov dword ptr [ecx + 0x4], eax ; context->esp
        mov eax, dword ptr [esp]
        mov dword ptr [ecx + 0x8], eax ; context->eip
        // save callee-save general-purpose registers
        mov dword ptr [ecx + 0xc],  ebx; context->ebx
        mov dword ptr [ecx + 0x10], esi; context->esi
        mov dword ptr [ecx + 0x14], edi; context->edi
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
        mov eax, oldcontext
        add dword ptr[eax + 0x4], 0x4
        // fixup return address
        mov dword ptr[eax + 0x8], offset restore
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
        mov eax, context
        mov ebp, dword ptr [eax]        ; context->ebp
        mov esp, dword ptr [eax + 0x4]  ; context->esp
        ; restore callee-save general-purpose registers
        mov ebx, dword ptr [eax + 0xc]  ; context->ebx
        mov esi, dword ptr [eax + 0x10] ; context->esi
        mov edi, dword ptr [eax + 0x14] ; context->edi
        push dword ptr [eax + 0x8]      ; context->eip
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
        "mov %0, %%eax;"
        // fixup oldcontext->esp, ignore pushed arguments, since we'll resume from restore
        "addl $0x4, 0x4(%%eax);"
        // fixup return address
        "movl $restore, 0x8(%%eax);"
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
        "movl     (%0), %%ebp;" // context->ebp
        "movl  0x4(%0), %%esp;" // context->esp
        "movl  0xc(%0), %%ebx;" // context->ebx
        "movl 0x10(%0), %%esi;" // context->esi
        "movl 0x14(%0), %%edi;" // context->edi
        "pushl 0x8(%0);"        // context->eip
        "ret;"
        // should never return here
        :: "r"(context)
    );
}
#else
#  error Unsupported compiler
#endif

#else // !FIBER_X86
#  error Unsupported platform
#endif // FIBER_X86
