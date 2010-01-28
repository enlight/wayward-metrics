#include "wayward/metrics/reporting/base.h"

#include "wayward/metrics/bert_encode.h"
#include "wayward/metrics/buffer.h"
#include "wayward/metrics/connection.h"
#include "wayward/metrics/file.h"
#include "wayward/metrics/frame.h"
#include "wayward/metrics/message_queue.h"
#include "wayward/metrics/reporting/constants.h"

#include <stdlib.h>

//------------------------------------------------------------------------------
/**
*/
struct wwm_reporter_t_
{
    wwm_message_queue_t     message_queue;
};

//------------------------------------------------------------------------------
/**
*/
wwm_reporter_t
wwm_reporter_new(void)
{
    wwm_reporter_t reporter = (wwm_reporter_t)calloc(1, sizeof(struct wwm_reporter_t_));

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
void
wwm_reporter_start_session(wwm_reporter_t reporter, const char *session_id)
{
    wwm_buffer_t buffer = wwm_buffer_new(255);
    buffer = wwm_bert_push_begin(buffer);
    buffer = wwm_bert_push_begin_tuple(buffer, 1);
    buffer = wwm_bert_push_string(buffer, session_id);

    wwm_frame_t frame = wwm_frame_new();
    wwm_frame_setup(frame, METRICS_METHOD_START_SESSION, 1, buffer);

    wwm_message_queue_enqueue(reporter->message_queue, frame);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_record_data(wwm_reporter_t reporter, wwm_buffer_t data)
{
    wwm_frame_t frame = wwm_frame_new();
    wwm_frame_setup(frame, METRICS_METHOD_RECORD_DATA, 0, data);
    wwm_message_queue_enqueue(reporter->message_queue, frame);
}

