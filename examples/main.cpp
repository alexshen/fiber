#define TEST_MAKE_CURRENT_FIBER 0
#define TEST_EXIT_FIBER         1
#define TEST_SWITCH_TO          2
//#define TEST_MODE TEST_EXIT_FIBER
//#define TEST_MODE TEST_SWITCH_TO
#ifndef TEST_MODE
#  define TEST_MODE TEST_MAKE_CURRENT_FIBER
#endif

#include "fiber.hpp"
#include <cstdio>
#include <cstdlib>
#include "fiber_context.h"

using namespace std;

fiber* main_fiber;
fiber* writer_fiber;
fiber* chainee_fiber;
bool end = false;
FILE* fpRead = 0;
FILE* fpWrite = 0;
char buf[256];

void writer_fiber_callback(void* arg)
{
    fiber* write_fiber = static_cast<fiber*>(arg);
    while (!end)
    {
        printf("%s", buf);
        writer_fiber->switch_to(*main_fiber);
    }
    printf("writer done\n");
}

void chainee_fiber_callback(void*)
{
    printf("I'm chainee\n");
#if TEST_MODE == TEST_MAKE_CURRENT_FIBER
    fiber::make_current_fiber(*main_fiber);
#elif TEST_MODE == TEST_SWITCH_TO
    chainee_fiber->switch_to(*main_fiber);
#else
    printf("chainee done\n");
#endif
}

bool returnFromContext;
fiber_context context1;
fiber_context maincontext, context;
void context_entry(void* arg)
{
    printf("%d\n", *(int*)arg);
    printf("%s\n", __FUNCTION__);
    fiber_swap_context(&context, &maincontext);
    printf("%s\n", __FUNCTION__);
    returnFromContext = true;
    fiber_set_context(&context1);
}

int main()
{
    int arg = 10;
    context.stack = new char[16 * 1024];
    context.stack_size = 16 * 1024;
    fiber_get_context(&context1);
    if (!returnFromContext)
    {
        fiber_make_context(&context, context_entry, &arg);
        fiber_swap_context(&maincontext, &context);
        fiber_swap_context(&maincontext, &context);
    }
    else
    {
        printf("return from context\n");
    }

    fpRead = fopen("main.cpp", "r");
    if (!fpRead) return EXIT_FAILURE;
    fpWrite = fopen("copy.cpp", "w+");
    if (!fpWrite) return EXIT_FAILURE;

    main_fiber = fiber::convert_to_fiber();
    writer_fiber = new fiber(writer_fiber_callback, writer_fiber);
    chainee_fiber = new fiber(chainee_fiber_callback);
    writer_fiber->chain(*chainee_fiber);

    while (!feof(fpRead))
    {
        fgets(buf, sizeof(buf), fpRead);
        main_fiber->switch_to(*writer_fiber);
    }
    end = true;
    main_fiber->switch_to(*writer_fiber);

    delete main_fiber;
    delete writer_fiber;
    delete chainee_fiber;
    fclose(fpWrite);
    fclose(fpRead);
}
