#include "wayward/metrics/reporting/private/per_thread_reporter_data.h"

#include <stdlib.h>

//------------------------------------------------------------------------------
/**
*/
_wwm_reporter_per_thread_data_t
_wwm_reporter_per_thread_data_new(wwm_reporter_t owner)
{
    _wwm_reporter_per_thread_data_t ptdata = (_wwm_reporter_per_thread_data_t)malloc(sizeof(struct _wwm_reporter_per_thread_data_t_));
    ptdata->owner = owner;
    ptdata->next = NULL;
    ptdata->stacktrace_buffer = (void**)malloc(STACKTRACE_BUFFER_LENGTH * sizeof(void*));

    _wwm_reporter_add_per_thread_data(owner, ptdata);

    return ptdata;
}

//------------------------------------------------------------------------------
/**
*/
void
_wwm_reporter_per_thread_data_destroy(_wwm_reporter_per_thread_data_t per_thread_data)
{
    _wwm_reporter_remove_per_thread_data(per_thread_data->owner, per_thread_data);
    free(per_thread_data->stacktrace_buffer);
}

//------------------------------------------------------------------------------
/**
*/
void
_wwm_reporter_per_thread_data_kill(void* per_thread_data)
{
    _wwm_reporter_per_thread_data_destroy((_wwm_reporter_per_thread_data_t)per_thread_data);
}


