#ifndef _WAYWARD_METRICS_FRAME_H_
#define _WAYWARD_METRICS_FRAME_H_

#include <stddef.h>
#include <stdint.h>

#include "wayward/metrics/buffer.h"

typedef struct wwm_frame_t_ *wwm_frame_t;

extern wwm_frame_t  wwm_frame_new(void);
extern void         wwm_frame_destroy(wwm_frame_t frame);

extern void         wwm_frame_setup(wwm_frame_t frame, int32_t message_type, int32_t correlation_id, wwm_buffer_t payload);
extern void         wwm_frame_reset(wwm_frame_t frame);

extern wwm_buffer_t wwm_frame_encode(const wwm_frame_t frame, wwm_buffer_t buffer);

extern void         wwm_frame_set_next(wwm_frame_t frame, wwm_frame_t next);
extern wwm_frame_t  wwm_frame_get_next(wwm_frame_t frame);

extern void                 wwm_frame_set_payload(wwm_frame_t frame, wwm_buffer_t payload);
extern const wwm_buffer_t   wwm_frame_get_payload(const wwm_frame_t frame);

#endif

