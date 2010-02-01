#include "wayward/metrics/reporting/reporter.h"

#include "wayward/metrics/buffer.h"
#include "wayward/metrics/codec.h"
#include "wayward/metrics/connection.h"
#include "wayward/metrics/file.h"
#include "wayward/metrics/reporting/constants.h"
#include "wayward/metrics/reporting/private/per_thread_reporter_data.h"
#include "wayward/metrics/reporting/private/reporter.h"
#include "wayward/metrics/thread.h"

#include <execinfo.h>
#include <stdlib.h>

//------------------------------------------------------------------------------
/**
*/
wwm_reporter_t
wwm_reporter_new(void)
{
    wwm_reporter_t reporter = (wwm_reporter_t)calloc(1, sizeof(struct wwm_reporter_t_));

    (void)pthread_key_create(&(reporter->per_thread_data_key), _wwm_reporter_per_thread_data_kill);

    reporter->message_queue = wwm_message_queue_new();
    return reporter;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_destroy(wwm_reporter_t reporter)
{
    wwm_message_queue_destroy(reporter->message_queue);
    free(reporter);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_log_to_network(wwm_reporter_t reporter, const char *hostname, int portnumber)
{
    int sockfd = wwm_open_socket(hostname, portnumber);
    wwm_connection_t conn = wwm_connection_new();
    wwm_connection_set_sockfd(conn, sockfd);
    wwm_message_queue_set_connection(reporter->message_queue, conn);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_log_to_file(wwm_reporter_t reporter, const char *filename)
{
    wwm_file_t file = wwm_file_new();
    wwm_file_open(file, filename, WWM_FILE_APPEND);
    wwm_message_queue_set_file(reporter->message_queue, file);
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_reporter_start_command(wwm_reporter_t reporter, wwm_buffer_t data, int32_t message_type, int32_t correlation_id)
{
    data = wwm_buffer_append_int32(data, message_type);
    data = wwm_buffer_append_int32(data, correlation_id);
    return data;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_start_session(wwm_reporter_t reporter, const char *session_id)
{
    wwm_buffer_t buffer = wwm_buffer_new(255);
    buffer = wwm_reporter_start_command(reporter, buffer, METRICS_METHOD_START_SESSION, 1);
    buffer = wwm_codec_push_begin(buffer);
    buffer = wwm_codec_push_begin_tuple(buffer, 1);
    buffer = wwm_codec_push_string(buffer, session_id);

    wwm_message_queue_enqueue(reporter->message_queue, buffer);
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_reporter_populate_base_record_data(wwm_reporter_t reporter, wwm_buffer_t data)
{
    _wwm_reporter_per_thread_data_t ptdata = _wwm_reporter_get_per_thread_data(reporter);
    int stack_frame_count = backtrace(ptdata->stacktrace_buffer, STACKTRACE_BUFFER_LENGTH);

    struct timeval now;
    (void)gettimeofday(&now, NULL);

    data = wwm_buffer_ensure(data, 512);
    data = wwm_reporter_start_command(reporter, data, METRICS_METHOD_START_SESSION, 1);
    data = wwm_codec_push_begin(data);
    data = wwm_codec_push_begin_tuple(data, 3);
    data = wwm_codec_push_timestamp(data, &now);
    data = wwm_codec_push_uint64(data, wwm_thread_get_current_id());
    data = wwm_codec_push_begin_tuple(data, 0); // Plug in the stack trace
    data = wwm_codec_push_begin_tuple(data, 0); // Plug in the context stack
    return data;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_record_data(wwm_reporter_t reporter, wwm_buffer_t data)
{
    wwm_message_queue_enqueue(reporter->message_queue, data);
}

//------------------------------------------------------------------------------
/**
*/
_wwm_reporter_per_thread_data_t
_wwm_reporter_get_per_thread_data(wwm_reporter_t reporter)
{
    _wwm_reporter_per_thread_data_t ptdata = pthread_getspecific(reporter->per_thread_data_key);
    if (NULL == ptdata)
    {
        ptdata = _wwm_reporter_per_thread_data_new(reporter);
        pthread_setspecific(reporter->per_thread_data_key, ptdata);
    }
    return ptdata;
}

//------------------------------------------------------------------------------
/**
*/
void
_wwm_reporter_add_per_thread_data(wwm_reporter_t reporter, _wwm_reporter_per_thread_data_t per_thread_data)
{
    if (NULL == reporter->per_thread_data_slist)
    {
        reporter->per_thread_data_slist = per_thread_data;
    }
    else
    {
        _wwm_reporter_per_thread_data_t tail = reporter->per_thread_data_slist;
        while (NULL != tail->next)
        {
            tail = tail->next;
        }
        tail->next = per_thread_data;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
_wwm_reporter_remove_per_thread_data(wwm_reporter_t reporter, _wwm_reporter_per_thread_data_t per_thread_data)
{
    if (per_thread_data == reporter->per_thread_data_slist)
    {
        reporter->per_thread_data_slist = per_thread_data->next;
    }
    else
    {
        _wwm_reporter_per_thread_data_t ptd;
        for (ptd = reporter->per_thread_data_slist; NULL != ptd->next; ptd = ptd->next)
        {
            if (per_thread_data == ptd->next)
            {
                ptd->next = per_thread_data->next;
                break;
            }
        }
    }
}

