#ifndef FIBER_HPP
#define FIBER_HPP

#include "fiber.h"
#include <cstddef>
#include <csetjmp>

/**
 * Exception must be turned off when using this class. since the exception chain will not be recovered after switching
 * You must not delete a running fiber.
 * If you're using MSCV, you need to turn of DEP in Linking options, otherwise crash.
 */

class fiber
{
    enum state {
        fs_not_run,
        fs_running,
        fs_switched_out,
        fs_dead,
        fs_state_mask          = (1 << fs_dead) - 1,
        fs_convert_from_thread
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

#ifdef ENABLE_MAKE_CURRENT_FIBER
    // switch to the new fiber without saving the current context
    static void make_current_fiber(fiber& curr, fiber& new_fiber);
#endif
private:
    fiber();

    void init(fiber_callback entry, void* arg, char* stack, std::size_t stack_size);

    bool is_convert_from_thread() const;
    void set_convert_from_thread(bool flag);
    void set_state(state new_state);
    state get_state() const;

    // the wrapper
    static void fiber_wrapper(fiber* this_fiber);
private:
    std::jmp_buf   m_context;
    // stack is top-down
    char*          m_stack_bottom; // the bottom address of the stack
    char*          m_stack_top;    // the top address of the stack
    std::size_t    m_stack_size;
    bool           m_free_stack;

    fiber_callback m_entry;
    void*          m_userarg;
    fiber*         m_chainee;
    int            m_state;
private:
    // disable copying
    fiber(fiber const&);
    fiber& operator=(fiber const&);
};

#endif // FIBER_HPP
