#include "wayward/metrics/thread.h"
#include <stdio.h>
#include <errno.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>

/* error C2016: C requires that a struct or union has at least one member
struct wwm_thread_t_
{
};
*/

uint64_t 
wwm_thread_get_current_id(void)
{
	union { DWORD thread_val; uint64_t uint64_val; } caster = { 0 };
	caster.thread_val = GetCurrentThreadId(); // FIXME: if this was really going to mirror pthreads then it should call GetCurrentThread()
	return caster.uint64_val;
}

int 
wwm_thread_key_create(wwm_thread_key_t* key, void (*destructor)(void*))
{
	*key = TlsAlloc();
	if (TLS_OUT_OF_INDEXES == *key)
		return EAGAIN;
	else
		return 0; // TODO: associate the destructor with the key
}

int 
wwm_thread_setspecific(wwm_thread_key_t key, void* data)
{
	if (TlsSetValue(key, data))
		return 0;
	return EINVAL;
}

void* 
wwm_thread_getspecific(wwm_thread_key_t key)
{
	return TlsGetValue(key);
}

int 
wwm_thread_key_delete(wwm_thread_key_t key)
{
	if (TlsFree(key))
		return 0;
	else
		return ENOENT;
}

int 
wwm_thread_create(wwm_thread_handle_t* handle, const wwm_thread_attr_t* attr, WWM_THREADPROC_RETTYPE (*thread_proc)(void*), void* arg)
{
	*handle = (HANDLE)_beginthreadex(
		NULL, // security
		0, // stack size (default)
		thread_proc,
		arg,
		0, // run the thread as soon as it is created
		NULL // thread id
	);
	if (NULL == handle)
	{
		int err = 0;
		if (0 == _get_errno(&err))
			if ((EAGAIN == err) || (EINVAL == err))
				return err;
		return EBUSY;
	}
	return 0;
}

int 
wwm_thread_join(wwm_thread_handle_t thread_handle, int* status)
{
	// FIXME: check for errors
	WaitForSingleObject(thread_handle, INFINITE);
	if (status)
	{
		DWORD exit_code = 0;
		GetExitCodeThread(thread_handle, &exit_code);
		*status = exit_code;
	}
	return 0;
}
