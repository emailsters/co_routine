#ifndef _CO_ROUTINE_H_
#define _CO_ROUTINE_H_
#include <stdint.h>

namespace testing
{
struct Schedule;
enum CoState
{
    CO_READY,
    CO_RUNNING,
    CO_SUSPEND,
    CO_DEAD
};

typedef void (*RoutineFunc)(Schedule* schedule, void*arg);

Schedule* CoroutineOpen();
int CoroutineCreate(Schedule* schedule, RoutineFunc func, void* arg);
void CoroutineClose(Schedule* schedule);
int CoroutineResume(Schedule* schedule, int coroutine_id);
void CoroutineYield(Schedule* schedule);
int CoroutineStatus(Schedule* schedule, int64_t coroutine_id);

}

#endif
