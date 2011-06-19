#include "fiber.h"
#include "fiber.hpp"
#include <cassert>

using namespace std;

fiber::fiber(fiber_callback entry, void* arg, std::size_t stack_size /* = FIBER_DEFAULT_STACK_SIZE */)
{
    init(entry, arg, 0, stack_size);
}

fiber::fiber(fiber_callback entry, void* arg, char* stack, std::size_t stack_size /* = FIBER_DEFAULT_STACK_SIZE */)
{
    init(entry, arg, stack, stack_size);
}

fiber::~fiber()
{
    if (m_free_stack)
    {
        delete[] m_stack_bottom;
    }
}

void fiber::switch_to(fiber& new_fiber)
{
    // save the current context
    if (!setjmp(m_context))
    {
        longjmp(new_fiber.m_context, 0);
    }
}

void fiber::fiber_wrapper( fiber* this_fiber )
{
    // save the context and return
    if (!setjmp(this_fiber->m_context))
        return;

    this_fiber->m_entry(this_fiber->m_userarg);
    if (this_fiber->m_chainee)
    {
        this_fiber->switch_to(*this_fiber->m_chainee);
    }

    // since the stack below is not correct, we can only exit the current thread.
    exit_fiber();
}

void fiber::chain(fiber& chainee)
{
    m_chainee = &chainee;
}

fiber* fiber::convert_to_fiber()
{
    return new fiber();
}

void fiber::make_current_fiber( fiber& new_fiber )
{
    // I don't why codes below causes crashes, we don't care about m_context of the temp fiber
    // since it's not used, just used to call switch_to...
    // fiber().switch_to(new_fiber);
    jmp_buf tmp;
    if (!setjmp(tmp))
    {
        longjmp(new_fiber.m_context, 0);
    }
}

// other members are not initialized, because they are not needed for a fiber which is created by convert_to_fiber
fiber::fiber()
    : m_free_stack(false)
    , m_chainee(0)
{
}

void fiber::init(fiber_callback entry, void* arg, char* stack, std::size_t stack_size)
{
    assert(entry);
    m_entry = entry;
    m_userarg = arg;
    m_chainee = 0;

    if (!stack)
    {
        m_stack_bottom = new char[stack_size];
        assert(m_stack_bottom);
        m_free_stack = true;
    }
    else
    {
        m_stack_bottom = stack;
        m_free_stack = false;
    }

    const size_t stack_alignment = 16;
    // round down to the pointer boundary
    m_stack_top = reinterpret_cast<char*>(reinterpret_cast<size_t>(m_stack_bottom + stack_size) & ~(stack_alignment - 1));

    // init the environment of us
    init_env(*this);
}

//----------------------------------------------------------------//
// C API for fiber

namespace 
{
    inline fiber& from_handle(fiber_t handle)
    {
        return *static_cast<fiber*>(handle);
    }
}

extern "C"
{

fiber_t create_fiber( fiber_callback entry, void* arg, unsigned int stack_size )
{
    return new fiber(entry, arg, stack_size);
}

fiber_t create_fiber_user_stack( fiber_callback entry, void* arg, char* stack, unsigned int stack_size )
{
    return new fiber(entry, arg, stack, stack_size);
}

fiber_t convert_to_fiber()
{
    return fiber::convert_to_fiber();
}

void make_current_fiber( fiber_t handle)
{
    fiber::make_current_fiber(from_handle(handle));
}

void delete_fiber( fiber_t curr )
{
    delete &from_handle(curr);
}

void switch_fiber( fiber_t curr, fiber_t next )
{
    assert(curr && next);
    from_handle(curr).switch_to(from_handle(next));
}

void chain_fiber( fiber_t curr, fiber_t next )
{
    assert(curr && next);
    from_handle(curr).chain(from_handle(next));
}

} // extern "C"
