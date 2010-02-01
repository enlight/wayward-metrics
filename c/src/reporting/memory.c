#include "wayward/metrics/reporting/memory.h"
#include "wayward/metrics/reporting/constants.h"
#include "wayward/metrics/codec.h"

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_memory_heap_new(wwm_reporter_t reporter, int32_t heap_id, const char* heap_name)
{
    wwm_buffer_t data = wwm_buffer_new(512);
    data = wwm_reporter_populate_base_record_data(reporter, data);
    data = wwm_codec_push_begin_tuple(data, 3);
    data = wwm_codec_push_int32(data, METRICS_EVENT_MEMORY_HEAP_NEW);
    data = wwm_codec_push_int32(data, heap_id);
    data = wwm_codec_push_string(data, heap_name);
    wwm_reporter_record_data(reporter, data);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_memory_heap_destroy(wwm_reporter_t reporter, int32_t heap_id)
{
    wwm_buffer_t data = wwm_buffer_new(512);
    data = wwm_reporter_populate_base_record_data(reporter, data);
    data = wwm_codec_push_begin_tuple(data, 2);
    data = wwm_codec_push_int32(data, METRICS_EVENT_MEMORY_HEAP_DESTROY);
    data = wwm_codec_push_int32(data, heap_id);
    wwm_reporter_record_data(reporter, data);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_memory_malloc(wwm_reporter_t reporter, int32_t heap_id, void* addr, size_t size)
{
    wwm_buffer_t data = wwm_buffer_new(512);
    data = wwm_reporter_populate_base_record_data(reporter, data);
    data = wwm_codec_push_begin_tuple(data, 4);
    data = wwm_codec_push_int32(data, METRICS_EVENT_MEMORY_MALLOC);
    data = wwm_codec_push_int32(data, heap_id);
    data = wwm_codec_push_uint64(data, (uint64_t)(uintptr_t)addr);
    data = wwm_codec_push_uint64(data, size);
    wwm_reporter_record_data(reporter, data);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_memory_realloc(wwm_reporter_t reporter, int32_t heap_id, void* old_addr, void* new_addr, size_t new_size)
{
    wwm_buffer_t data = wwm_buffer_new(512);
    data = wwm_reporter_populate_base_record_data(reporter, data);
    data = wwm_codec_push_begin_tuple(data, 5);
    data = wwm_codec_push_int32(data, METRICS_EVENT_MEMORY_REALLOC);
    data = wwm_codec_push_int32(data, heap_id);
    data = wwm_codec_push_uint64(data, (uint64_t)(uintptr_t)old_addr);
    data = wwm_codec_push_uint64(data, (uint64_t)(uintptr_t)new_addr);
    data = wwm_codec_push_uint64(data, new_size);
    wwm_reporter_record_data(reporter, data);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_memory_free(wwm_reporter_t reporter, int32_t heap_id, void* addr)
{
    wwm_buffer_t data = wwm_buffer_new(512);
    data = wwm_reporter_populate_base_record_data(reporter, data);
    data = wwm_codec_push_begin_tuple(data, 3);
    data = wwm_codec_push_int32(data, METRICS_EVENT_MEMORY_FREE);
    data = wwm_codec_push_int32(data, heap_id);
    data = wwm_codec_push_uint64(data, (uint64_t)(uintptr_t)addr);
    wwm_reporter_record_data(reporter, data);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_memory_gc_start(wwm_reporter_t reporter, int32_t heap_id)
{
    wwm_buffer_t data = wwm_buffer_new(512);
    data = wwm_reporter_populate_base_record_data(reporter, data);
    data = wwm_codec_push_begin_tuple(data, 2);
    data = wwm_codec_push_int32(data, METRICS_EVENT_MEMORY_GC_START);
    data = wwm_codec_push_int32(data, heap_id);
    wwm_reporter_record_data(reporter, data);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_reporter_memory_gc_end(wwm_reporter_t reporter, int32_t heap_id)
{
    wwm_buffer_t data = wwm_buffer_new(512);
    data = wwm_reporter_populate_base_record_data(reporter, data);
    data = wwm_codec_push_begin_tuple(data, 2);
    data = wwm_codec_push_int32(data, METRICS_EVENT_MEMORY_GC_END);
    data = wwm_codec_push_int32(data, heap_id);
    wwm_reporter_record_data(reporter, data);
}

