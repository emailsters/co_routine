#include "co_routine.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ucontext.h>
#include <limits.h>
#include <map>

namespace testing
{
// need 256M + 1M memory
#define MAX_COROUTINE_COUNT (256)
#define STACK_SIZE (1024 * 1024)
#define COROUTINE_INIT_ID (1)

struct Routine;
struct Schedule
{
    ucontext_t main;
    int max_num;

    // auto increase
    int64_t current_id;
    int64_t running_id;
    std::map<int64_t, Routine*> routines;

    // runtime stack
    char stack[STACK_SIZE];
};

struct Routine
{
    ucontext_t context;

    // self stack
    char stack[STACK_SIZE];
    int status;
    RoutineFunc func;
    void* arg;
};

Schedule* CoroutineOpen()
{
    Schedule* schedule = new Schedule;
    if (schedule)
    {
        schedule->max_num = MAX_COROUTINE_COUNT;
        schedule->current_id = 0;
    }
    return schedule;
}

int CoroutineCreate(Schedule* schedule, RoutineFunc func, void* arg)
{
    if (schedule->routines.size() >= MAX_COROUTINE_COUNT)
    {
        return -1;
    }
    Routine* co = new Routine;
    if (!co)
    {
        return -1;
    }
    memset(co->stack, 0, sizeof(co->stack));
    co->func = func;
    co->arg = arg;
    co->status = CO_READY;
    if (schedule->current_id >= LONG_MAX)
    {
        schedule->current_id = COROUTINE_INIT_ID;
    }
    else
    {
        ++schedule->current_id;
    }
    schedule->routines.insert(std::make_pair(schedule->current_id, co));
    return schedule->current_id;
}

void CoroutineClose(Schedule* schedule)
{
    int co_count = schedule->routines.size();
    printf("%d routines remain\n", co_count);
    if (co_count == 0)
    {
        delete schedule;
        return;
    }
    std::map<int64_t, Routine*>::iterator itor = schedule->routines.begin();
    for (; itor != schedule->routines.end(); ++itor)
    {
        delete itor->second;
    }
    delete schedule;
}

void CoroutineMainFunc(void* arg)
{
    Schedule* schedule = reinterpret_cast<Schedule*>(arg);
    uint64_t co_id = schedule->running_id;
    std::map<int64_t, Routine*>::iterator itor = 
        schedule->routines.find(co_id);
    if (itor == schedule->routines.end() || !(itor->second))
    {
        return;
    }
    Routine* co = itor->second;
    co->func(schedule, co->arg);
    delete co;
    schedule->routines.erase(itor);    
    schedule->running_id = -1;

    // will not decrease current_id
    // --schedule->current_id;
}

int CoroutineResume(Schedule* schedule, int64_t coroutine_id)
{
    std::map<int64_t, Routine*>::iterator itor = 
        schedule->routines.find(coroutine_id);
    if (itor == schedule->routines.end() || !(itor->second))
    {
        printf("coroutine %lld not exist\n", coroutine_id);
        return -1;
    }
    Routine* co = itor->second;
    switch (co->status)
    {
    case CO_READY:
        if (getcontext(&co->context) == -1)
        {
            printf("getcontext error\n");
            return -1;
        }
        co->context.uc_stack.ss_sp = schedule->stack;
        co->context.uc_stack.ss_size = sizeof(schedule->stack);
        co->context.uc_link = &schedule->main;
        co->status = CO_RUNNING;
        schedule->running_id = itor->first;
        makecontext(&co->context, (void (*)(void))CoroutineMainFunc, 1, schedule);
        swapcontext(&schedule->main, &co->context);
        
        // todo: add ret judgement         
        break;
    case CO_SUSPEND:
        co->status = CO_RUNNING;
        schedule->running_id = itor->first;
        memcpy(schedule->stack, co->stack, sizeof(co->stack));
        swapcontext(&schedule->main, &co->context);
        break;
    default:
        break;
    }
    return 0;    
}

void CoroutineYield(Schedule* schedule)
{
    int64_t co_id = schedule->running_id;
    std::map<int64_t, Routine*>::iterator itor = 
        schedule->routines.find(co_id);
    if (itor != schedule->routines.end() && !(itor->second))
    {
        return;
    }
    Routine* co = itor->second;
    co->status = CO_SUSPEND;
    schedule->running_id = -1;
    memcpy(co->stack, schedule->stack, sizeof(schedule->stack));
    swapcontext(&co->context, &schedule->main);
}

int CoroutineStatus(Schedule* schedule, int64_t coroutine_id)
{
    if (coroutine_id < 0) return -1;

    std::map<int64_t, Routine*>::iterator itor = 
        schedule->routines.find(coroutine_id);
    if (itor == schedule->routines.end() || !(itor->second))
    {
        return -1;
    }
    return itor->second->status;
}
}
