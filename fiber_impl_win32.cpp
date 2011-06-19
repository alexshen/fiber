#include "fiber.hpp"
#include <csetjmp>
#include <Windows.h>
using namespace std;

struct fiber::impl
{
    jmp_buf context;
};

fiber::impl* fiber::create_impl()
{
    return new fiber::impl;
}

void fiber::destroy_impl(impl* p)
{
    delete p;
}

void detail::fiber_trampoline( fiber* this_fiber )
{
    this_fiber->m_entry(this_fiber->m_userarg);
    if (this_fiber->m_chainee)
    {
        this_fiber->switch_to(*this_fiber->m_chainee);
    }

    // since the stack below is not correct, we have to exit the current thread.
    ExitThread(0);
}

void fiber::switch_to(fiber& new_fiber)
{
    if (!setjmp(m_impl->m_context))
    {
        if (!(new_fiber.m_state & fiber_inited))
        {
            new_fiber.m_state |= fiber_inited;

            const size_t stack_alignment = 16;
            // round down to the pointer boundary
            char* stack_top = reinterpret_cast<char*>(
                    reinterpret_cast<size_t>(new_fiber.m_stack + new_fiber.m_stack_size) & ~(stack_alignment - 1));
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
        else
        {
            longjmp(new_fiber.m_impl->m_context, 0);
        }
    }
}

void fiber::make_current_fiber( fiber& new_fiber )
{
    // I don't why codes below causes crashes, we don't care about m_context of the temp fiber
    // since it's not used, just used to call switch_to...
    // fiber().switch_to(new_fiber);
    longjmp(new_fiber.m_impl->m_context, 0);
}
