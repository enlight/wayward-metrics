#ifndef _WAYWARD_METRICS_THREAD_H_
#define _WAYWARD_METRICS_THREAD_H_

#include "wayward/metrics/config.h"
#include "wayward/metrics/types.h"

typedef struct wwm_thread_t_ *wwm_thread_t;

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

typedef HANDLE wwm_thread_handle_t;
typedef void*  wwm_thread_attr_t;
typedef DWORD  wwm_thread_key_t;

#else // if not WIN32

#include <pthread.h>

typedef pthread_t      wwm_thread_handle_t;
typedef pthread_attr_t wwm_thread_attr_t;
typedef pthread_key_t  wwm_thread_key_t;

#endif // WIN32

extern uint64_t wwm_thread_get_current_id(void);
extern int wwm_thread_key_create(wwm_thread_key_t* key, void (*destructor)(void*));
extern int wwm_thread_setspecific(wwm_thread_key_t key, void* data);
extern void* wwm_thread_getspecific(wwm_thread_key_t key);
extern int wwm_thread_key_delete(wwm_thread_key_t key);
extern int wwm_thread_create(wwm_thread_handle_t* handle, const wwm_thread_attr_t* attr, WWM_THREADPROC_RETTYPE (*thread_proc)(void*), void* arg);
extern int wwm_thread_join(wwm_thread_handle_t handle, int* status);

#endif // _WAYWARD_METRICS_THREAD_H_
