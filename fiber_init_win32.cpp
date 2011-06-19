#include "fiber.hpp"
#include <Windows.h>

void fiber::init_env(fiber& new_fiber)
{
    char* stack_top = new_fiber.m_stack_top;
#if defined(_MSC_VER)
    // setup the stack for the fiber
    __asm
    {
        push ebx
        mov ebx, esp
        mov esp, stack_top
    }

    fiber_wrapper(&new_fiber);

    // restore the stack for the old fiber
    __asm
    {
        mov esp, ebx
        pop ebx
    }
#elif defined(__GNUC__)
    // setup the stack for the fiber
    asm (
        "movl %%esp, %%ebx;"
        "movl %0, %%esp;"
        :: "r"(stack_top)
        : "ebx"
        );

    fiber_wrapper(&new_fiber);

    // restore the stack for the old fiber
    asm ("movl %ebx, %esp;");
#else
#  error Unsupported compiler
#endif

}

void fiber::exit_fiber()
{
    ExitThread(0);
}
