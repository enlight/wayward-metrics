#include "wayward/metrics/frame.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

wayward_metrics_frame_t wayward_metrics_frame_new(int32_t message_type, int32_t correlation_id, size_t initial_size, unsigned char * payload)
{
    wayward_metrics_frame_t frame = (wayward_metrics_frame_t)malloc(sizeof(struct wayward_metrics_frame_t_) + initial_size * sizeof(unsigned char));
    if (NULL == frame)
    {
        return NULL;
    }
    frame->message_type = message_type;
    frame->correlation_id = correlation_id;
    frame->length = initial_size;
    memcpy(frame->payload, payload, initial_size);
    
    return frame;
}

void wayward_metrics_frame_destroy(wayward_metrics_frame_t frame)
{
    free(frame);
}

