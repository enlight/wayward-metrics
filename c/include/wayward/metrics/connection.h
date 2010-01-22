#ifndef _WAYWARD_METRICS_CONNECTION_H_
#define _WAYWARD_METRICS_CONNECTION_H_

#include "wayward/metrics/frame.h"

typedef struct wayward_metrics_connection_t_ *wayward_metrics_connection_t;

extern wayward_metrics_connection_t wayward_metrics_connection_new(void);
extern void wayward_metrics_connection_destroy(wayward_metrics_connection_t conn);

extern int wayward_metrics_connection_get_sockfd(wayward_metrics_connection_t conn);
extern void wayward_metrics_connection_set_sockfd(wayward_metrics_connection_t conn, int sockfd);

extern int wayward_metrics_connection_handle_input(wayward_metrics_connection_t conn,
                                                   wayward_metrics_frame_t decoded_frame);
extern int wayward_metrics_connection_send_frame(wayward_metrics_connection_t conn,
                                                 const wayward_metrics_frame_t frame);

#endif

