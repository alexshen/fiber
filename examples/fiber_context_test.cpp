#include "fiber_context.h"
#include <cstdio>
#include <cstdlib>

using namespace std;

bool returnFromContext;
fiber_context maincontext1;
fiber_context maincontext2, context;
void context_entry(void* arg)
{
    printf("%d\n", *(int*)arg);
    printf("%s\n", __FUNCTION__);
    fiber_swap_context(&context, &maincontext);
    printf("%s\n", __FUNCTION__);
    returnFromContext = true;
    fiber_set_context(&maincontext1);
}

int main()
{
    int arg = 10;
    context.stack = new char[16 * 1024];
    context.stack_size = 16 * 1024;
    fiber_get_context(&maincontext1);
    if (!returnFromContext)
    {
        fiber_make_context(&context, context_entry, &arg);
        fiber_swap_context(&maincontext2, &context);
        fiber_swap_context(&maincontext2, &context);
    }
    else
    {
        printf("return from context\n");
    }
}
