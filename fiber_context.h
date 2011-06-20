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
    // registers
    uintptr_t ebp;
    uintptr_t esp;
    uintptr_t eip;

    char* stack;
    int   stack_size;
    void* userarg;
};

void fiber_make_context(fiber_context* context, fiber_entry entry, void* arg);
void fiber_swap_context(fiber_context* oldcontext, fiber_context* newcontext);
void fiber_set_context(fiber_context* context);

#ifdef __cplusplus
}
#endif

#endif // FIBER_CONTEXT_H
