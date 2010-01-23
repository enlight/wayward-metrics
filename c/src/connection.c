#include "wayward/metrics/connection.h"

#include "wayward/metrics/buffer.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct wwm_connection_t_
{
    int         sockfd;
};

wwm_connection_t wwm_connection_new(void)
{
    wwm_connection_t conn = (wwm_connection_t)malloc(sizeof(struct wwm_connection_t_));
    if (NULL == conn)
    {
        return NULL;
    }
    memset(conn, 0, sizeof(struct wwm_connection_t_));

    return conn;
}

int wwm_connection_get_sockfd(wwm_connection_t conn)
{
    return conn->sockfd;
}

void wwm_connection_set_sockfd(wwm_connection_t conn, int sockfd)
{
    conn->sockfd = sockfd;
}

void wwm_connection_destroy(wwm_connection_t conn)
{
    free(conn);
}

int wwm_connection_handle_input(wwm_connection_t conn,
                                wwm_frame_t decoded_frame)
{
    return 0;
}

int wwm_connection_send_frame(wwm_connection_t conn,
                              const wwm_frame_t frame)
{
    wwm_buffer_t encoded;
    // XXX: Implement encoding the frame ...
    write(conn->sockfd, wwm_buffer_bytes(encoded), wwm_buffer_length(encoded));
    return 0;
}

