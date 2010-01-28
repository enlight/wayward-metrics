#ifndef _WAYWARD_METRICS_MESSAGE_QUEUE_H_
#define _WAYWARD_METRICS_MESSAGE_QUEUE_H_

#include "wayward/metrics/connection.h"
#include "wayward/metrics/file.h"
#include "wayward/metrics/frame.h"

typedef struct wwm_message_queue_t_ *wwm_message_queue_t;

wwm_message_queue_t     wwm_message_queue_new(void);
void                    wwm_message_queue_destroy(wwm_message_queue_t queue);

void                    wwm_message_queue_request_shutdown(wwm_message_queue_t queue);

void                    wwm_message_queue_set_connection(wwm_message_queue_t queue, wwm_connection_t conn);
void                    wwm_message_queue_set_file(wwm_message_queue_t queue, wwm_file_t file);

void                    wwm_message_queue_enqueue(wwm_message_queue_t queue, wwm_frame_t frame);

#endif

