#include "fiber.hpp"
#include <Windows.h>

void fiber::init_env(fiber& new_fiber)
{
    char* stack_top = new_fiber.m_stack_top;
#if defined(_MSC_VER)
    // setup the stack for the fiber
    __asm
    {
        mov esp, stack_top
    }
#elif defined(__GNUC__)
    // setup the stack for the fiber
    asm (
        "movl %0, %%esp;"
        :: "r"(stack_top)
        );
#else
#  error Unsupported compiler
#endif
    fiber_wrapper(&new_fiber);
}

void fiber::exit_fiber()
{
    ExitThread(0);
}
