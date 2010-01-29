#include "wayward/metrics/bert_encode.h"
#include "wayward/metrics/private/bert_constants.h"

#include <string.h>

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_bert_push_begin(wwm_buffer_t buf)
{
    buf = wwm_buffer_append_int8(buf, BERT_PROTOCOL_VERSION);
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_bert_push_begin_tuple(wwm_buffer_t buf, int32_t num_items)
{
    if (num_items < 256)
    {
        buf = wwm_buffer_ensure(buf, 2);
        buf = wwm_buffer_append_int8(buf, BERT_SMALL_TUPLE_EXT);
        buf = wwm_buffer_append_int8(buf, num_items);
    }
    else
    {
        buf = wwm_buffer_ensure(buf, 5);
        buf = wwm_buffer_append_int8(buf, BERT_LARGE_TUPLE_EXT);
        buf = wwm_buffer_append_int32(buf, num_items);
    }
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_bert_push_int32(wwm_buffer_t buf, int32_t value)
{
    if (value < 256)
    {
        buf = wwm_buffer_ensure(buf, 2);
        buf = wwm_buffer_append_int8(buf, BERT_SMALL_INTEGER_EXT);
        buf = wwm_buffer_append_int8(buf, value);
    }
    else
    {
        buf = wwm_buffer_ensure(buf, 5);
        buf = wwm_buffer_append_int8(buf, BERT_INTEGER_EXT);
        buf = wwm_buffer_append_int32(buf, value);
    }
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_bert_push_atom(wwm_buffer_t buf, const char * value)
{
    size_t string_length = strlen(value);

    buf = wwm_buffer_ensure(buf, 1 + 2 + string_length);
    buf = wwm_buffer_append_int8(buf, BERT_ATOM_EXT);
    buf = wwm_buffer_append_int16(buf, string_length);
    buf = wwm_buffer_append_bytes(buf, value, string_length);
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_bert_push_string(wwm_buffer_t buf, const char * value)
{
    size_t string_length = strlen(value);

    buf = wwm_buffer_ensure(buf, 1 + 2 + string_length);
    buf = wwm_buffer_append_int8(buf, BERT_BINARY_EXT);
    buf = wwm_buffer_append_int32(buf, string_length);
    buf = wwm_buffer_append_bytes(buf, value, string_length);
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_bert_push_timestamp(wwm_buffer_t buf, const struct timeval *tv)
{
    unsigned int megaseconds  = tv->tv_sec / 1000000;
    unsigned int seconds      = tv->tv_sec % 1000000;
    unsigned int microseconds = tv->tv_usec;

    buf = wwm_bert_push_begin_tuple(buf, 5);
    buf = wwm_bert_push_atom(buf, "atom");
    buf = wwm_bert_push_atom(buf, "time");
    buf = wwm_bert_push_int32(buf, megaseconds);
    buf = wwm_bert_push_int32(buf, seconds);
    buf = wwm_bert_push_int32(buf, microseconds);
    return buf;
}

