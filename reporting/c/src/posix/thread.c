#include "wayward/metrics/thread.h"

#include <pthread.h>
#include <stdio.h>

struct wwm_thread_t_
{
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

int 
wwm_thread_create(wwm_thread_handle_t* handle, const wwm_thread_attr_t* attr, WWM_THREADPROC_RETTYPE (*thread_proc)(void*), void* arg)
{
	return pthread_create(handle, attr, thread_proc, arg);
}

int 
wwm_thread_join(wwm_thread_handle_t thread_handle, int* status)
{
	void* exit_status = 0;
	int retval = pthread_join(thread_handle, &exit_status);
	if (status)
		*status = __INT(exit_status);
	return retval;
}
