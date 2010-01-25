#include "wayward/metrics/frame.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//------------------------------------------------------------------------------
/**
*/
struct wwm_frame_t_ {
    int32_t         message_type;
    int32_t         correlation_id;
    size_t          length;
    unsigned char   payload[0];
};

//------------------------------------------------------------------------------
/**
*/
wwm_frame_t
wwm_frame_new(int32_t message_type, int32_t correlation_id, size_t initial_size, unsigned char * payload)
{
    wwm_frame_t frame = (wwm_frame_t)malloc(sizeof(struct wwm_frame_t_) + initial_size * sizeof(unsigned char));
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

//------------------------------------------------------------------------------
/**
*/
void
wwm_frame_destroy(wwm_frame_t frame)
{
    free(frame);
}

