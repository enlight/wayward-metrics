#ifndef _WAYWARD_METRICS_BUFFER_H_
#define _WAYWARD_METRICS_BUFFER_H_

#include <stddef.h>
#include <stdint.h>

typedef struct wwm_buffer_t_ * wwm_buffer_t;

extern wwm_buffer_t             wwm_buffer_new(size_t initial_size);
extern void                     wwm_buffer_destroy(wwm_buffer_t buf);

extern wwm_buffer_t             wwm_buffer_resize(wwm_buffer_t buf, size_t new_size);
extern wwm_buffer_t             wwm_buffer_ensure(wwm_buffer_t buf, size_t space_needed);
extern void                     wwm_buffer_reset(wwm_buffer_t buf);

extern const unsigned char *    wwm_buffer_bytes(const wwm_buffer_t buf);
extern size_t                   wwm_buffer_length(const wwm_buffer_t buf);

extern wwm_buffer_t             wwm_buffer_append_int8(wwm_buffer_t buf, int8_t value);
extern wwm_buffer_t             wwm_buffer_append_int16(wwm_buffer_t buf, int16_t value);
extern wwm_buffer_t             wwm_buffer_append_int32(wwm_buffer_t buf, int32_t value);
extern wwm_buffer_t             wwm_buffer_append_bytes(wwm_buffer_t buf, const unsigned char * bytes, size_t length);

#endif

