#ifndef _WAYWARD_METRICS_CONFIG_H_
#define _WAYWARD_METRICS_CONFIG_H_

typedef int bool;
#ifndef TRUE
#   define TRUE 1
#endif
#ifndef FALSE
#   define FALSE 0
#endif

#ifdef WIN32
#define WWM_THREADPROC __stdcall
#define WWM_THREADPROC_RETTYPE unsigned
#else
#define WWM_THREADPROC
#define WWM_THREADPROC_RETTYPE void*
#endif

// I hate typing out struct for anything but the definition!
typedef struct event_base *event_base_t;
typedef struct evhttp *evhttp_t;
typedef struct evhttp_request *evhttp_request_t;
typedef struct evkeyvalq *evkeyvalq_t;
typedef struct evkeyval *evkeyval_t;
typedef struct evbuffer *evbuffer_t;
typedef struct evbuffer_ptr evbuffer_ptr_t;

#endif _WAYWARD_METRICS_CONFIG_H_
