#ifndef _WAYWARD_METRICS_FRAME_H_
#define _WAYWARD_METRICS_FRAME_H_

#include <stddef.h>
#include <stdint.h>

typedef struct wayward_metrics_frame_t_ *wayward_metrics_frame_t;

extern wayward_metrics_frame_t wayward_metrics_frame_new(int32_t message_type, int32_t correlation_id, size_t initial_size, unsigned char * payload);
extern void wayward_metrics_frame_destroy(wayward_metrics_frame_t frame);

#endif

