#ifndef _WAYWARD_METRICS_CONNECTION_H_
#define _WAYWARD_METRICS_CONNECTION_H_

#include "wayward/metrics/buffer.h"
#include "wayward/metrics/frame.h"

typedef struct wwm_connection_t_ *wwm_connection_t;

extern wwm_connection_t     wwm_connection_new(void);
extern void                 wwm_connection_destroy(wwm_connection_t conn);

extern int                  wwm_connection_get_sockfd(wwm_connection_t conn);
extern void                 wwm_connection_set_sockfd(wwm_connection_t conn, int sockfd);

extern int                  wwm_connection_handle_input(wwm_connection_t conn,
                                                        wwm_frame_t decoded_frame);
extern int                  wwm_connection_send_buffer(wwm_connection_t conn,
                                                       const wwm_buffer_t buffer);

#endif

