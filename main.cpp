#define TEST_MAKE_CURRENT_FIBER

#include "fiber.hpp"
#include <cstdio>
#include <cstdlib>
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
#if defined(ENABLE_MAKE_CURRENT_FIBER) && defined(TEST_MAKE_CURRENT_FIBER)
    fiber::make_current_fiber(*main_fiber);
#else
    chainee_fiber->switch_to(*main_fiber);
#endif
}

int main()
{
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