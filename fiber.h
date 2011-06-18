#ifndef FIBER_H
#define FIBER_H

#ifdef __cplusplus
extern "C"
{
#endif

//crash
//#define ENABLE_MAKE_CURRENT_FIBER
#define FIBER_DEFAULT_STACK_SIZE (16 * 1024)

// opaque type of fiber
typedef void* fiber_t;

// fiber callback prototype
typedef void (*fiber_callback)(void*);

/**
 * create a fiber
 *
 * @param entry      entry of the fiber
 * @param arg        user-provided argument passed to entry
 * @param stack_size stack size of the fiber, if 0, use FIBER_DEFAULT_STACK_SIZE
 */
fiber_t create_fiber(fiber_callback entry, void* arg, unsigned int stack_size);

/**
 * create a fiber but with user-supplied stack memory
 *
 * @param stack      user-supplied stack memory
 * @param stack_size stack size
 */
fiber_t create_fiber_user_stack(fiber_callback entry, void* arg, char* stack, unsigned int stack_size);

/**
 * convert the current thread to a fiber
 *
 * after using, call delete_fiber with the returned value
 * Note: You should not call this function more than one on a thread.
 */
fiber_t convert_to_fiber();

#ifdef ENABLE_MAKE_CURRENT_FIBER
/**
 * Make a fiber as the current fiber
 *
 * @param handle fiber to make as current
 */
void make_currrent_fiber(fiber_t handle);
#endif

/**
 * Delete a fiber
 *
 * @param fiber to be deleted
 */
void delete_fiber(fiber_t handle);

/**
 * switch to another fiber
 *
 * @param prev current fiber before switching to another fiber
 * @param next new fiber to switch to
 */
void switch_fiber(fiber_t curr, fiber_t next);

/**
 * when fiber returns, if no chaining fiber, current thread is exited
 * otherwise, switching to the chaining fiber
 * NOTE: Only the fiber created by create_fiber* can chain other fibers.
 *
 * @param curr a fiber
 * @param next chaining fiber
 */
void chain_fiber(fiber_t curr, fiber_t next);

#ifdef __cplusplus
}
#endif

#endif // FIBER_H
