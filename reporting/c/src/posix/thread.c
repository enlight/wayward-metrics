#include "wayward/metrics/thread.h"
#include "wayward/metrics/allocator.h"
#include <pthread.h>
#include <stdio.h>

struct wwm_thread_t_
{
    pthread_t      handle;
    pthread_attr_t attr;
};

uint64_t 
wwm_thread_get_current_id(void)
{
    union { pthread_t thread_val; uint64_t uint64_val; } caster = { };
    caster.thread_val = pthread_self();
    return caster.uint64_val;
}

int 
wwm_thread_key_create(wwm_thread_key_t* key, void (*destructor)(void*))
{
    return pthread_key_create(key, destructor);
}

int 
wwm_thread_setspecific(wwm_thread_key_t key, const void* data)
{
    return pthread_setspecific(key, data);
}

void* 
wwm_thread_getspecific(wwm_thread_key_t key)
{
    return pthread_getspecific(key);
}

int 
wwm_thread_key_delete(wwm_thread_key_t key)
{
    return pthread_key_delete(key);
}

wwm_thread_t
wwm_thread_new(void)
{
    wwm_thread_t thread = (wwm_thread_t)g_wwm_allocator.malloc(sizeof(struct wwm_thread_t_));
    if (NULL != thread)
    {
        (void)pthread_attr_init(&(thread->attr));
    }
    return thread;
}

void 
wwm_thread_destroy(wwm_thread_t thread)
{
    (void)pthread_attr_destroy(&(thread->attr));
    g_wwm_allocator.free(thread);
}

int 
wwm_thread_start(wwm_thread_t thread, WWM_THREADPROC_RETTYPE (*thread_proc)(void*), void* arg)
{
    (void)pthread_attr_setdetachstate(&(thread->attr), PTHREAD_CREATE_JOINABLE);
    return pthread_create(&thread->handle, thread->attr, thread_proc, arg);
}

int 
wwm_thread_join(wwm_thread_t thread, int* status)
{
    void* exit_status = 0;
    int retval = pthread_join(thread->handle, &exit_status);
    if (status)
    {
        *status = __INT(exit_status);
    }
    return retval;
}
