#ifndef _WAYWARD_METRICS_FRAME_H_
#define _WAYWARD_METRICS_FRAME_H_

#include <stddef.h>
#include <stdint.h>

typedef struct wwm_frame_t_ *wwm_frame_t;

extern wwm_frame_t  wwm_frame_new(int32_t message_type, int32_t correlation_id, size_t initial_size, unsigned char * payload);
extern void         wwm_frame_destroy(wwm_frame_t frame);

#endif

