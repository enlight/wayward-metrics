#ifndef _WAYWARD_METRICS_REPORTING_PRIVATE_PER_THREAD_REPORTER_DATA_H_
#define _WAYWARD_METRICS_REPORTING_PRIVATE_PER_THREAD_REPORTER_DATA_H_

#include "wayward/metrics/reporting/base.h"

#define STACKTRACE_BUFFER_LENGTH 100

typedef struct _wwm_reporter_per_thread_data_t_ *_wwm_reporter_per_thread_data_t;

extern _wwm_reporter_per_thread_data_t  _wwm_reporter_per_thread_data_new(wwm_reporter_t owner);
extern void _wwm_reporter_per_thread_data_destroy(_wwm_reporter_per_thread_data_t per_thread_data);
extern void _wwm_reporter_per_thread_data_kill(void* per_thread_data);

//------------------------------------------------------------------------------
/**
*/
struct _wwm_reporter_per_thread_data_t_
{
    wwm_reporter_t                          owner;
    void                                 ** stacktrace_buffer;
    _wwm_reporter_per_thread_data_t         next;
};

#endif

