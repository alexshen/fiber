#include "fiber.h"
#include "fiber.hpp"
#include <cstring>
#include <cassert>

#ifdef _WIN32
#  include <Windows.h>
#else
#  error Only win32 is supported
#endif

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
    assert(is_convert_from_thread() || get_state() != fs_running);
    if (m_free_stack)
    {
        delete[] m_stack_bottom;
    }
}

void fiber::switch_to(fiber& new_fiber)
{
    // cannot switch to self or a running fiber
    assert(new_fiber.get_state() != fs_running && this != &new_fiber);
    
    if (this == &new_fiber || new_fiber.get_state() == fs_running)
        return;

    // save the current context
    if (!setjmp(m_context))
    {
        set_state(fs_switched_out);
        if (new_fiber.get_state() == fs_not_run)
        {
            new_fiber.set_state(fs_running);
            // since we've saved the context, it's safe to modify ebp and esp
#ifdef _WIN32
#  ifdef _MSC_VER
            // setup the stack for the new fiber
            char* stack_top = new_fiber.m_stack_top;
            __asm {
                mov esp, stack_top
            }
#  else
#    error Only MSVC is supported for now
#  endif
            fiber_wrapper(&new_fiber);
            // never reach here
#  else
#    error Only win32 is supported for now
#endif
        }
        else // if (new_fiber.m_state == fs_switched_out)
        {
            longjmp(new_fiber.m_context, 0);
        }
    }
    else
    {
        set_state(fs_running);
    }
}

void fiber::fiber_wrapper( fiber* this_fiber )
{
    this_fiber->m_entry(this_fiber->m_userarg);
    if (this_fiber->m_chainee)
    {
        this_fiber->switch_to(*this_fiber->m_chainee);
    }

    // since the stack below is not correct, we can only exit the current thread.
#ifdef _WIN32
    ExitThread(0);
#else
#  error Only win32 is supported
#endif
}

void fiber::chain(fiber& chainee)
{
    m_chainee = &chainee;
}

fiber* fiber::convert_to_fiber()
{
    return new fiber();
}

#ifdef ENABLE_MAKE_CURRENT_FIBER
void fiber::make_current_fiber(fiber& new_fiber)
{
    // create a temporary fiber to switch from 
    fiber tmp;
    tmp.switch_to(new_fiber);
}
#endif

// other members are not initialized, because they are not needed for a fiber which is created by convert_to_fiber
fiber::fiber()
    : m_free_stack(false)
    , m_state(fs_convert_from_thread | fs_running)
    , m_chainee(0)
{
}

void fiber::init(fiber_callback entry, void* arg, char* stack, std::size_t stack_size)
{
    assert(entry);
    m_entry = entry;
    m_userarg = arg;

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
    m_stack_size = stack_size;
    m_state = fs_not_run;
}

bool fiber::is_convert_from_thread() const
{
    return (m_state & fs_convert_from_thread) != 0;
}

void fiber::set_convert_from_thread( bool flag )
{
    m_state |= flag ? fs_convert_from_thread : 0;
}

void fiber::set_state( state new_state )
{
    m_state &= ~fs_state_mask;
    m_state |= new_state;
}

fiber::state fiber::get_state() const
{
    return static_cast<state>(m_state & fs_state_mask);
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

#ifdef ENABLE_MAKE_CURRENT_FIBER
void make_current_fiber(fiber_t handle)
{
    fiber::make_current_fiber(from_handle(handle));
}
#endif

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
