#include "fiber_context.h"
#include <cassert>
#include <cstring>
using namespace std;

void fiber_make_context(fiber_context* context, fiber_entry entry, void* arg)
{
    assert(context && entry);
    // default alignment of the stack
    const int alignment = 16;

    context->esp   = ((int)context->stack + context->stack_size) & ~(alignment - 1);
    context->ebp   = context->esp;
    context->eip   = (int)entry;
    context->userarg = arg;

    // push the argument onto the stack
    char* top = (char*)context->esp;
    memcpy(top, &context->userarg, sizeof(void*));
    // make space for the pushed argument
    context->esp  -= sizeof(void*);

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
        "pushl %%ebx;"
        // save current stack pointer to oldcontext
        "movl %0, %%ebx;"
        "movl %%ebp, (%%ebx);"
        "movl %%esp, 4(%%ebx);"
        // save return address
        "movl restore, %%eax;"
        "movl %%eax, 8(%%ebx);"
        // restore the enviroment for newcontext
        "movl %1, %%ebx;"
        "movl (%%ebx), %%ebp;"   // newcontext->ebp
        "movl 4(%%ebx), %%esp;" // newcontext->esp
        "pushl 8(%%ebx);"       // newcontext->eip
        "ret;"
        "restore:;"
        "popl %%ebx;"
        :: "r"(oldcontext), "r"(newcontext)
        : "%eax"
     );
}

void fiber_set_context(fiber_context* context)
{
    asm (
        "pushl %%ebx;"
        // restore the enviroment for newcontext
        "movl %0, %%ebx;"
        "movl (%%ebx), %%ebp;"   // context->ebp
        "movl 4(%%ebx), %%esp;" // context->esp
        "pushl 8(%%ebx);"       // context->eip
        "ret;"
        // should never return here
        "popl %%ebx;"
        :: "r"(context)
    );
}
#else
#  error Unsupported compiler
#endif
