#ifndef _WAYWARD_METRICS_THREAD_H_
#define _WAYWARD_METRICS_THREAD_H_

#include "wayward/metrics/config.h"
#include "wayward/metrics/types.h"

typedef struct wwm_thread_t_* wwm_thread_t;

#ifdef WIN32
typedef DWORD wwm_thread_key_t;
#else
typedef pthread_key_t wwm_thread_key_t;
#endif

extern uint64_t wwm_thread_get_current_id(void);

extern int   wwm_thread_key_create(wwm_thread_key_t* key);
extern int   wwm_thread_key_set(wwm_thread_key_t key, void* data);
extern void* wwm_thread_key_get(wwm_thread_key_t key);
extern int   wwm_thread_key_delete(wwm_thread_key_t key);

extern wwm_thread_t wwm_thread_new(void);
extern void         wwm_thread_destroy(wwm_thread_t thread);
extern int          wwm_thread_start(wwm_thread_t thread, WWM_THREADPROC_RETTYPE (*thread_proc)(void*), void* arg);
extern int          wwm_thread_join(wwm_thread_t thread, int* status);

#endif // _WAYWARD_METRICS_THREAD_H_
