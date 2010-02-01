#include "wayward/metrics/thread.h"

#include <pthread.h>
#include <stdio.h>

struct wwm_thread_t_
{
};

uint64_t wwm_thread_get_current_id(void)
{
    union { pthread_t thread_val; uint64_t uint64_val; } caster = { };
    caster.thread_val = pthread_self();
    printf("ID: %lld\n", caster.uint64_val);
    return caster.uint64_val;
}

