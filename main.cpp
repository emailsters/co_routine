#include "co_routine.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
using namespace testing;

void Func1(Schedule* schedule, void* arg)
{
    printf("this is func1\n");
    for (int i = 0; i < 3; ++i)
    {
        printf("this is func1 %d\n", i);
        CoroutineYield(schedule);
        // do async action
    }
    printf("-----func1 end------\n");
}

void Func2(Schedule* schedule, void* arg)
{
    printf("this is func2\n");
    for (int i = 0; i < 5; ++i)
    {
        printf("this is func2 %d\n", i);
        CoroutineYield(schedule);
        // do async action
    }
    printf("-----func2 end------\n");
}

typedef struct
{
    int a;
    int b;
} Dollor;

void Func3(Schedule* schedule, void* arg)
{
    printf("this is func3\n");
    Dollor* dollor = reinterpret_cast<Dollor*>(arg);
    assert(dollor != NULL);

    printf("Func3 receive dollors {%d, %d}\n", dollor->a, dollor->b);
    for (int i = 0; i < 5; ++i)
    {
        printf("this is func3 %d\n", i);
        CoroutineYield(schedule);
        // do async action
    }
    printf("-----func3 end------\n");
}

int main()
{
    Schedule* schedule = CoroutineOpen();
    assert(schedule != NULL);

    Dollor dollor = {3, 4};
    int64_t co_id = CoroutineCreate(schedule, Func1, NULL);
    int64_t co_id_2 = CoroutineCreate(schedule, Func2, NULL);
    int64_t co_id_3 = CoroutineCreate(schedule, Func3, &dollor);
    while (CoroutineStatus(schedule, co_id) != -1 || 
        CoroutineStatus(schedule, co_id_2) != -1 ||
        CoroutineStatus(schedule, co_id_3) != -1)
    {
        CoroutineResume(schedule, co_id);
        CoroutineResume(schedule, co_id_2);
        CoroutineResume(schedule, co_id_3);
    }
    
    CoroutineClose(schedule);

    /*
    if (getcontext(&g_coroutine_func1.context) == -1)
    {
        printf("%s [%u]\n", "error", __LINE__);
        return -1;
    }
    g_coroutine_func1.context.uc_stack.ss_sp = g_coroutine_func1.stack;
    g_coroutine_func1.context.uc_stack.ss_size = sizeof(g_coroutine_func1.stack);
    g_coroutine_func1.context.uc_link = &g_main_context;
    makecontext(g_coroutine_func1.context, Func1, NULL);
    
    if (getcontext(&g_coroutine_func2.context) == -1)
    {
        printf("%s [%u]\n", "error", __LINE__);
        return -1;
    }
    g_coroutine_func2.context.uc_stack.ss_sp = g_coroutine_func2.stack;
    g_coroutine_func2.context.uc_stack.ss_size = sizeof(g_coroutine_func2.stack);
    g_coroutine_func2.context.uc_link = &g_main_context;

    swapcontext(&g_main_context, &g_coroutine_func1.context);
    */
    printf("main end\n");
}