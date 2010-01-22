#ifndef _WAYWARD_METRICS_BUFFER_H_
#define _WAYWARD_METRICS_BUFFER_H_

#include <stddef.h>
#include <stdint.h>

typedef struct wwm_buffer_t_ * wwm_buffer_t;

extern wwm_buffer_t             wwm_buffer_new(size_t initial_size);
extern wwm_buffer_t             wwm_buffer_resize(wwm_buffer_t buf, size_t new_size);
extern void                     wwm_buffer_destroy(wwm_buffer_t buf);

extern const unsigned char *    wwm_buffer_bytes(const wwm_buffer_t buf);
extern size_t                   wwm_buffer_length(const wwm_buffer_t buf);

#endif

