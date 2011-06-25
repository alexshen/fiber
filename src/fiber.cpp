//  @file fiber.cpp
//  Common implementation of the fiber class
//
//  Copyright 2011 Alex Shen. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "fiber.h"
#include "fiber.hpp"
#include <cassert>

using namespace std;

fiber::fiber(fiber_callback entry, void* arg, std::size_t stack_size /* = FIBER_DEFAULT_STACK_SIZE */)
{
    init(entry, arg, 0, stack_size, fiber_free_stack);
}

fiber::fiber(fiber_callback entry, void* arg, char* stack, std::size_t stack_size /* = FIBER_DEFAULT_STACK_SIZE */)
{
    init(entry, arg, stack, stack_size, fiber_invalid);
}

fiber::~fiber()
{
    if (m_state & fiber_free_stack)
    {
        delete[] m_stack;
    }
    destroy_impl(m_impl);
}

fiber* fiber::convert_to_fiber()
{
    return new fiber();
}

fiber::fiber()
{
    init(0, 0, 0, 0, fiber_inited);
}

void fiber::chain(fiber& chainee)
{
    m_chainee = &chainee;
}

void fiber::init(fiber_callback entry, void* arg, char* stack, std::size_t stack_size, int state)
{
    assert(entry && !(state & fiber_inited) || !entry && (state & fiber_inited));
    m_entry = entry;
    m_userarg = arg;
    m_chainee = 0;

    if (!stack)
    {
        m_stack = new char[stack_size];
        assert(m_stack);
    }
    else
    {
        m_stack = stack;
    }
    m_state = state;
    m_stack_size = stack_size;
    m_impl = create_impl();
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
