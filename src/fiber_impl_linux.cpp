//  @file fiber_impl_linux.cpp
//  Fiber class linux implmentation using context api
//
//  Copyright 2011 Alex Shen. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "fiber.hpp"
#include <ucontext.h>
#include <pthread.h>

namespace 
{
    // XXX: remove this
    fiber* current;
    void context_entry()
    {
        fiber* this_fiber = current;
        current = 0;
        detail::fiber_trampoline(this_fiber);
    }
}

struct fiber::impl
{
    ucontext_t context;
};

fiber::impl* fiber::create_impl()
{
    return new fiber::impl();
}

void fiber::destroy_impl(impl* p)
{
    delete p;
}

void fiber::switch_to(fiber& new_fiber)
{
    if (!(new_fiber.m_state & fiber_inited))
    {
        new_fiber.m_state |= fiber_inited;

        ucontext_t& context = new_fiber.m_impl->context;
        getcontext(&context);

        context.uc_stack.ss_sp   = new_fiber.m_stack;
        context.uc_stack.ss_size = new_fiber.m_stack_size;
        context.uc_flags         = 0;
        context.uc_link          = 0;

        current = &new_fiber;
        makecontext(&context, context_entry, 0);
    }
    swapcontext(&m_impl->context, &new_fiber.m_impl->context);
    //getcontext(&m_impl->context);
    //setcontext(&new_fiber.m_impl->context);
}

void detail::fiber_trampoline(fiber* this_fiber)
{
    this_fiber->m_entry(this_fiber->m_userarg);
    if (this_fiber->m_chainee)
    {
        this_fiber->switch_to(*this_fiber->m_chainee);
    }
    pthread_exit(0);
}

void fiber::make_current_fiber(fiber& new_fiber)
{
    setcontext(&new_fiber.m_impl->context);
}
