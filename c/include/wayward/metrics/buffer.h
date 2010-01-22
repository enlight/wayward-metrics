#ifndef _WAYWARD_METRICS_BUFFER_H_
#define _WAYWARD_METRICS_BUFFER_H_

#include <stddef.h>
#include <stdint.h>

typedef struct wayward_metrics_buffer_t_ * wayward_metrics_buffer_t;

extern wayward_metrics_buffer_t wayward_metrics_buffer_new(size_t initial_size);
extern wayward_metrics_buffer_t wayward_metrics_buffer_resize(wayward_metrics_buffer_t buf, size_t new_size);
extern void                     wayward_metrics_buffer_destroy(wayward_metrics_buffer_t buf);

extern const unsigned char *    wayward_metrics_buffer_bytes(const wayward_metrics_buffer_t buf);
extern size_t                   wayward_metrics_buffer_length(const wayward_metrics_buffer_t buf);

#endif

