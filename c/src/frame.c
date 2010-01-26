#include "wayward/metrics/frame.h"

#include <stdlib.h>
#include <string.h>

//------------------------------------------------------------------------------
/**
*/
struct wwm_frame_t_ {
    wwm_frame_t     next;
    int32_t         message_type;
    int32_t         correlation_id;
    wwm_buffer_t    payload;
};

static size_t       _wwm_frame_encoded_size(wwm_frame_t frame);

//------------------------------------------------------------------------------
/**
*/
wwm_frame_t
wwm_frame_new(void)
{
    wwm_frame_t frame = (wwm_frame_t)calloc(1, sizeof(struct wwm_frame_t_));
    if (NULL == frame)
    {
        return NULL;
    }
    return frame;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_frame_destroy(wwm_frame_t frame)
{
    if (NULL != frame->payload)
    {
        wwm_buffer_destroy(frame->payload);
    }
    free(frame);
}

//------------------------------------------------------------------------------
/**
    The frame must have been reset or newly created before this is called.
*/
void
wwm_frame_setup(wwm_frame_t frame, int32_t message_type, int32_t correlation_id, wwm_buffer_t payload)
{
    frame->message_type = message_type;
    frame->correlation_id = correlation_id;
    frame->payload = payload;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_frame_reset(wwm_frame_t frame)
{
    if (NULL != frame->payload)
    {
        wwm_buffer_destroy(frame->payload);
    }
    memset(frame, 0, sizeof(struct wwm_frame_t_));
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_frame_encode(const wwm_frame_t frame, wwm_buffer_t buffer)
{
    size_t space_needed = _wwm_frame_encoded_size(frame);
    buffer = wwm_buffer_ensure(buffer, space_needed);
    buffer = wwm_buffer_append_int32(buffer, space_needed - sizeof(int32_t)); // -4 for space_needed
    buffer = wwm_buffer_append_int32(buffer, frame->message_type);
    buffer = wwm_buffer_append_int32(buffer, frame->correlation_id);
    if (NULL != frame->payload)
    {
        buffer = wwm_buffer_append_bytes(buffer, wwm_buffer_bytes(frame->payload), wwm_buffer_length(frame->payload));
    }
    return buffer;
}

//------------------------------------------------------------------------------
/**
*/
static size_t
_wwm_frame_encoded_size(wwm_frame_t frame)
{
    size_t encoded_size = sizeof(int32_t) + sizeof(frame->message_type) + sizeof(frame->correlation_id);
    if (NULL != frame->payload)
    {
         encoded_size += wwm_buffer_length(frame->payload);
    }
    return encoded_size;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_frame_set_next(wwm_frame_t frame, wwm_frame_t next)
{
    frame->next = next;
}

//------------------------------------------------------------------------------
/**
*/
wwm_frame_t
wwm_frame_get_next(wwm_frame_t frame)
{
    return frame->next;
}


//------------------------------------------------------------------------------
/**
*/
void
wwm_frame_set_payload(wwm_frame_t frame, wwm_buffer_t payload)
{
    frame->payload = payload;
}

//------------------------------------------------------------------------------
/**
*/
const wwm_buffer_t
wwm_frame_get_payload(const wwm_frame_t frame)
{
    return frame->payload;
}

