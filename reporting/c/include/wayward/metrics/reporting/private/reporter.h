#ifndef _WAYWARD_METRICS_REPORTING_PRIVATE_REPORTER_H_
#define _WAYWARD_METRICS_REPORTING_PRIVATE_REPORTER_H_

#include "wayward/metrics/message_queue.h"
#include "wayward/metrics/types.h"
#include "wayward/metrics/thread.h"

#define STACKTRACE_BUFFER_LENGTH 100
#define MAX_CONTEXT_DEPTH 100

typedef struct _wwm_reporter_per_thread_data_t_ *_wwm_reporter_per_thread_data_t;
//------------------------------------------------------------------------------
/**
*/
struct wwm_reporter_t_
{
    wwm_message_queue_t             message_queue;
    wwm_thread_key_t                per_thread_data_key;
    _wwm_reporter_per_thread_data_t per_thread_data_slist;
};

//------------------------------------------------------------------------------
/**
*/
struct _wwm_reporter_context_t_
{
    uint64_t            context_id;
};

//------------------------------------------------------------------------------
/**
*/
struct _wwm_reporter_per_thread_data_t_
{
    wwm_reporter_t                          owner;
    void                                 ** stacktrace_buffer;
    struct _wwm_reporter_context_t_         context_stack[MAX_CONTEXT_DEPTH];
    int32_t                                 current_context_depth;
    _wwm_reporter_per_thread_data_t         next;
};

extern _wwm_reporter_per_thread_data_t _wwm_reporter_get_per_thread_data(wwm_reporter_t reporter);
extern void _wwm_reporter_add_per_thread_data(wwm_reporter_t reporter, _wwm_reporter_per_thread_data_t per_thread_data);
extern void _wwm_reporter_remove_per_thread_data(wwm_reporter_t reporter, _wwm_reporter_per_thread_data_t per_thread_data);

extern _wwm_reporter_per_thread_data_t  _wwm_reporter_per_thread_data_new(wwm_reporter_t owner);
extern void _wwm_reporter_per_thread_data_destroy(_wwm_reporter_per_thread_data_t per_thread_data);
extern void _wwm_reporter_per_thread_data_kill(void* per_thread_data);

extern void _wwm_reporter_per_thread_data_push_context(_wwm_reporter_per_thread_data_t, uint64_t context_id);
extern void _wwm_reporter_per_thread_data_pop_context(_wwm_reporter_per_thread_data_t);

#endif

