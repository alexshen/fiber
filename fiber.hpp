#ifndef FIBER_HPP
#define FIBER_HPP

#include "fiber.h"
#include <cstddef>

/**
 * Exception must be turned off when using this class. since the exception chain will not be recovered after switching
 * You must not delete a running fiber.
 */

class fiber;
namespace detail
{
    void fiber_trampoline(fiber* this_fiber);
}

class fiber
{
    enum
    {
        fiber_invalid    = 0x0,
        fiber_inited     = 0x1,
        fiber_free_stack = 0x2
    };
public:
    typedef void (*fiber_callback)(void*);

    fiber(fiber_callback entry, void* arg = 0, std::size_t stack_size = FIBER_DEFAULT_STACK_SIZE);
    fiber(fiber_callback entry, void* arg, char* stack, std::size_t stack_size = FIBER_DEFAULT_STACK_SIZE);
    ~fiber();

    void chain(fiber& chainee);

    // convert the current thread to a fiber
    // You must delete the returned object after using
    static fiber* convert_to_fiber();

    void switch_to(fiber& new_fiber);

    // switch to the new fiber without saving the current context
    static void make_current_fiber(fiber& new_fiber);
private:
    fiber();

    void init(fiber_callback entry, void* arg, char* stack, std::size_t stack_size, int state);

    // the wrapper
    friend void detail::fiber_trampoline(fiber* this_fiber);
private:
    // Platform specific variables go into impl
    struct         impl;
    impl*          m_impl;
    static impl* create_impl();
    static void  destroy_impl(impl* p);

    // Platform in-dependent variables
    char*          m_stack;
    std::size_t    m_stack_size;
    int            m_state;

    fiber_callback m_entry;
    void*          m_userarg;
    fiber*         m_chainee;
private:
    // disable copying
    fiber(fiber const&);
    fiber& operator=(fiber const&);
};

#endif // FIBER_HPP
