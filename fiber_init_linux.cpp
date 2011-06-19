#include "fiber.hpp"
#include <pthread.h>

void fiber::init_env(fiber& new_fiber)
{
    char* stack_top = new_fiber.m_stack_top;
#if defined(__GNUC__)
    // setup the stack for the fiber
    asm (
        "movl %0, %%esp;"
        :: "r"(stack_top)
        );
    fiber::fiber_wrapper(&new_fiber);
#else
#  error Unsupported compiler
#endif
}

void fiber::exit_fiber()
{
    pthread_exit(0);
}
