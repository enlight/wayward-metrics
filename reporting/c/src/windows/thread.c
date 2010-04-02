#include "wayward/metrics/thread.h"
#include "wayward/metrics/allocator.h"
#include <stdio.h>
#include <errno.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>

struct wwm_thread_t_
{
    HANDLE handle;
};

uint64_t 
wwm_thread_get_current_id(void)
{
    union { DWORD thread_val; uint64_t uint64_val; } caster = { 0 };
    caster.thread_val = GetCurrentThreadId(); // FIXME: if this was really going to mirror pthreads then it should call GetCurrentThread()
    return caster.uint64_val;
}

int 
wwm_thread_key_create(wwm_thread_key_t* key)
{
    *key = TlsAlloc();
    if (TLS_OUT_OF_INDEXES == *key)
    {
        return EAGAIN;
    }
    else
    {
        return 0;
    }
}

int 
wwm_thread_key_set(wwm_thread_key_t key, void* data)
{
    if (TlsSetValue(key, data))
    {
        return 0;
    }
    return EINVAL;
}

void* 
wwm_thread_key_get(wwm_thread_key_t key)
{
    return TlsGetValue(key);
}

int 
wwm_thread_key_delete(wwm_thread_key_t key)
{
    if (TlsFree(key))
    {
        return 0;
    }
    else
    {
        return ENOENT;
    }
}

wwm_thread_t
wwm_thread_new(void)
{
    wwm_thread_t thread = (wwm_thread_t)g_wwm_allocator.malloc(sizeof(struct wwm_thread_t_));
    if (NULL != thread)
    {
        thread->handle = NULL;
    }
    return thread;
}

void 
wwm_thread_destroy(wwm_thread_t thread)
{
    if (NULL != thread->handle)
    {
        CloseHandle(thread->handle);
        thread->handle = NULL;
    }
    g_wwm_allocator.free(thread);
}

int 
wwm_thread_start(wwm_thread_t thread, WWM_THREADPROC_RETTYPE (*thread_proc)(void*), void* arg)
{
    thread->handle = (HANDLE)_beginthreadex(
        NULL, // security
        0, // stack size (default)
        thread_proc,
        arg,
        0, // run the thread as soon as it is created
        NULL // thread id
    );
    if (NULL == thread->handle)
    {
        int err = 0;
        if (0 == _get_errno(&err))
        {
            if ((EAGAIN == err) || (EINVAL == err))
            {
                return err;
            }
        }
        return EBUSY;
    }
    return 0;
}

int 
wwm_thread_join(wwm_thread_t thread, int* status)
{
    // FIXME: check for errors
    WaitForSingleObject(thread->handle, INFINITE);
    if (status)
    {
        DWORD exit_code = 0;
        GetExitCodeThread(thread->handle, &exit_code);
        *status = exit_code;
    }
    return 0;
}
