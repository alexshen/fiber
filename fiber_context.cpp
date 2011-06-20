#include "fiber_context.h"
#include <cassert>
#include <cstring>
using namespace std;

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

}

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4731) // inline assembly modifies ebp
void fiber_swap_context(fiber_context* oldcontext, fiber_context* newcontext)
{
    assert(oldcontext && newcontext);
    // save the current context in the oldcontext
    __asm {
        push ebx
        ; save current stack pointer to oldcontext
        mov ebx, oldcontext
        mov dword ptr [ebx + 0x0], ebp
        mov dword ptr [ebx + 0x4], esp
        ; save return address
        mov dword ptr [ebx + 0x8], offset restore
        ; restore the enviroment for newcontext
        mov ebx, newcontext
        mov ebp, dword ptr [ebx + 0x0] ; newcontext->ebp
        mov esp, dword ptr [ebx + 0x4] ; newcontext->esp
        push dword ptr [ebx + 0x8]     ; newcontext->eip
        ret
restore:
        pop ebx
    }
}

void fiber_set_context(fiber_context* context)
{
    __asm {
        push ebx
        ; restore the enviroment for context
        mov ebx, context
        mov ebp, dword ptr [ebx + 0x0] ; context->ebp
        mov esp, dword ptr [ebx + 0x4] ; context->esp
        push dword ptr [ebx + 0x8]     ; context->eip
        ret
        ; should never return here
        pop ebx
    }
}

#  pragma warning(pop)
#elif defined(__GNUC__)
void fiber_swap_context(fiber_context* oldcontext, fiber_context* newcontext)
{
    assert(oldcontext && newcontext);
    // save the current context in the oldcontext
    asm (
        // save current stack pointer to oldcontext
        "movl %%ebp, (%0);"
        "movl %%esp, 4(%0);"
        // save return address
        "movl $restore, %%eax;"
        "movl %%eax, 8(%0);"
        // restore the enviroment for newcontext
        "movl (%1), %%ebp;"  // newcontext->ebp
        "movl 4(%1), %%esp;" // newcontext->esp
        "pushl 8(%1);"       // newcontext->eip
        "ret;"
        "restore:;"
        :: "r"(oldcontext), "r"(newcontext)
        : "%eax"
     );
}

void fiber_set_context(fiber_context* context)
{
    asm (
        // restore the enviroment for newcontext
        "movl (%0), %%ebp;"  // context->ebp
        "movl 4(%0), %%esp;" // context->esp
        "pushl 8(%0);"       // context->eip
        "ret;"
        // should never return here
        :: "r"(context)
    );
}
#else
#  error Unsupported compiler
#endif
