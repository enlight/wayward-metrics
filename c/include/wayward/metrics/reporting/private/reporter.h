#ifndef _WAYWARD_METRICS_REPORTING_PRIVATE_REPORTER_H_
#define _WAYWARD_METRICS_REPORTING_PRIVATE_REPORTER_H_

#include "wayward/metrics/message_queue.h"
#include "wayward/metrics/reporting/private/per_thread_reporter_data.h"

#include <pthread.h>

//------------------------------------------------------------------------------
/**
*/
struct wwm_reporter_t_
{
    wwm_message_queue_t             message_queue;
    pthread_key_t                   per_thread_data_key;
    _wwm_reporter_per_thread_data_t per_thread_data_slist;
};

_wwm_reporter_per_thread_data_t _wwm_reporter_get_per_thread_data(wwm_reporter_t reporter);
void _wwm_reporter_add_per_thread_data(wwm_reporter_t reporter, _wwm_reporter_per_thread_data_t per_thread_data);
void _wwm_reporter_remove_per_thread_data(wwm_reporter_t reporter, _wwm_reporter_per_thread_data_t per_thread_data);

#endif

