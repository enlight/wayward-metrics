#ifndef _WAYWARD_METRICS_CODEC_H_
#define _WAYWARD_METRICS_CODEC_H_

#include "wayward/metrics/buffer.h"
#include "wayward/metrics/time.h"

extern wwm_buffer_t wwm_codec_push_begin(wwm_buffer_t buf);
extern wwm_buffer_t wwm_codec_push_begin_tuple(wwm_buffer_t buf, int num_items);
extern wwm_buffer_t wwm_codec_push_int32(wwm_buffer_t buf, int32_t value);
extern wwm_buffer_t wwm_codec_push_uint64(wwm_buffer_t buf, uint64_t value);
extern wwm_buffer_t wwm_codec_push_atom(wwm_buffer_t buf, const char * value);
extern wwm_buffer_t wwm_codec_push_string(wwm_buffer_t buf, const char * value);
extern wwm_buffer_t wwm_codec_push_timestamp(wwm_buffer_t buf, const struct timeval *tv);

#endif

