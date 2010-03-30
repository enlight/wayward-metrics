#ifndef _WAYWARD_METRICS_CONFIG_H_
#define _WAYWARD_METRICS_CONFIG_H_

typedef int bool;
#ifndef TRUE
#   define TRUE 1
#endif
#ifndef FALSE
#   define FALSE 0
#endif

#ifndef WIN32
#ifndef MemoryBarrier
#define MemoryBarrier __sync_synchronize
#endif // MemoryBarrier
#endif // WIN32

#ifdef WIN32
#define WWM_THREADPROC __stdcall
#define WWM_THREADPROC_RETTYPE unsigned
#else
#define WWM_THREADPROC
#define WWM_THREADPROC_RETTYPE void*
#endif

#endif

