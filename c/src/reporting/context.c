#include "wayward/metrics/reporting/context.h"
#include "wayward/metrics/reporting/constants.h"
#include "wayward/metrics/codec.h"

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_context_create(wwm_reporter_t reporter, uint64_t context_id, const char *name)
{
    wwm_buffer_t data = wwm_buffer_new(512);
    data = wwm_reporter_populate_base_record_data(reporter, data);
    data = wwm_codec_push_begin_tuple(data, 2);
    data = wwm_codec_push_int32(data, METRICS_EVENT_CONTEXT_CREATE);
    data = wwm_codec_push_uint64(data, context_id);
    data = wwm_codec_push_string(data, name);
    wwm_reporter_record_data(reporter, data);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_context_enter(wwm_reporter_t reporter, uint64_t context_id)
{
    wwm_buffer_t data = wwm_buffer_new(512);
    data = wwm_reporter_populate_base_record_data(reporter, data);
    data = wwm_codec_push_begin_tuple(data, 2);
    data = wwm_codec_push_int32(data, METRICS_EVENT_CONTEXT_ENTER);
    data = wwm_codec_push_uint64(data, context_id);
    wwm_reporter_record_data(reporter, data);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_context_exit(wwm_reporter_t reporter)
{
    wwm_buffer_t data = wwm_buffer_new(512);
    data = wwm_reporter_populate_base_record_data(reporter, data);
    data = wwm_codec_push_begin_tuple(data, 1);
    data = wwm_codec_push_int32(data, METRICS_EVENT_CONTEXT_EXIT);
    wwm_reporter_record_data(reporter, data);
}

