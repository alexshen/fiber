#ifndef FIBER_CONTEXT_H
#define FIBER_CONTEXT_H

#include "fiber_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*fiber_entry)(void*);

struct fiber_context
{
#ifdef FIBER_X86
    // registers
    uintptr_t ebp;
    uintptr_t esp;
    uintptr_t eip;
    // general purpose registers
    uintptr_t eax;
    uintptr_t ebx;
    uintptr_t ecx;
    uintptr_t edx;
    uintptr_t esi;
    uintptr_t edi;
#else
#  error Unsupported platform
#endif

    char* stack;
    int   stack_size;
    void* userarg;
};

void fiber_get_context(fiber_context* context);
void fiber_make_context(fiber_context* context, fiber_entry entry, void* arg);
void fiber_swap_context(fiber_context* oldcontext, fiber_context* newcontext);
void fiber_set_context(fiber_context* context);

#ifdef __cplusplus
}
#endif

#endif // FIBER_CONTEXT_H
