#ifndef FIBER_HPP
#define FIBER_HPP

#include "fiber.h"
#include <cstddef>
#include <csetjmp>

/**
 * Exception must be turned off when using this class. since the exception chain will not be recovered after switching
 * You must not delete a running fiber.
 */

class fiber
{
    enum
    {
        fiber_invalid    = 0x1,
        fiber_inited     = 0x2,
        fiber_free_stack = 0x4
    };
public:
    typedef void (*fiber_callback)(void*);

    fiber(fiber_callback entry, void* arg = 0, std::size_t stack_size = FIBER_DEFAULT_STACK_SIZE);
    fiber(fiber_callback entry, void* arg, char* stack, std::size_t stack_size = FIBER_DEFAULT_STACK_SIZE);
    ~fiber();

    void switch_to(fiber& new_fiber);
    void chain(fiber& chainee);

    // convert the current thread to a fiber
    // You must delete the returned object after using
    static fiber* convert_to_fiber();

    // switch to the new fiber without saving the current context
    static void make_current_fiber(fiber& new_fiber);
private:
    fiber();

    void init(fiber_callback entry, void* arg, char* stack, std::size_t stack_size);

    // platform specific init
    static void init_env(fiber& new_fiber);
    static void exit_fiber();

    // the wrapper
    static void fiber_wrapper(fiber* this_fiber);
private:
    std::jmp_buf   m_context;
    // stack is top-down
    char*          m_stack_bottom; // the bottom address of the stack
    char*          m_stack_top;    // the top address of the stack
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
