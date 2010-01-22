#include "wayward/metrics/connection.h"

#include "wayward/metrics/buffer.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct wayward_metrics_connection_t_
{
    int         sockfd;
};

wayward_metrics_connection_t wayward_metrics_connection_new(void)
{
    wayward_metrics_connection_t conn = (wayward_metrics_connection_t)malloc(sizeof(struct wayward_metrics_connection_t_));
    if (NULL == conn)
    {
        return NULL;
    }
    memset(conn, 0, sizeof(struct wayward_metrics_connection_t_));

    return conn;
}

int wayward_metrics_connection_get_sockfd(wayward_metrics_connection_t conn)
{
    return conn->sockfd;
}

void wayward_metrics_connection_set_sockfd(wayward_metrics_connection_t conn, int sockfd)
{
    conn->sockfd = sockfd;
}

void wayward_metrics_connection_destroy(wayward_metrics_connection_t conn)
{
    free(conn);
}

int wayward_metrics_connection_handle_input(wayward_metrics_connection_t conn,
                                            wayward_metrics_frame_t decoded_frame)
{
    return 0;
}

int wayward_metrics_connection_send_frame(wayward_metrics_connection_t conn,
                                          const wayward_metrics_frame_t frame)
{
    wayward_metrics_buffer_t encoded;
    // XXX: Implement encoding the frame ...
    write(conn->sockfd, wayward_metrics_buffer_bytes(encoded), wayward_metrics_buffer_length(encoded));
    return 0;
}

