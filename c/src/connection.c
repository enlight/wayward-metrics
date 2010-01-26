#include "wayward/metrics/connection.h"

#include "wayward/metrics/buffer.h"

#include <stdlib.h>
#include <unistd.h>

//------------------------------------------------------------------------------
/**
*/
struct wwm_connection_t_
{
    int         sockfd;
};

//------------------------------------------------------------------------------
/**
*/
wwm_connection_t
wwm_connection_new(void)
{
    wwm_connection_t conn = (wwm_connection_t)calloc(1, sizeof(struct wwm_connection_t_));
    if (NULL == conn)
    {
        return NULL;
    }

    return conn;
}

//------------------------------------------------------------------------------
/**
*/
int
wwm_connection_get_sockfd(wwm_connection_t conn)
{
    return conn->sockfd;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_connection_set_sockfd(wwm_connection_t conn, int sockfd)
{
    conn->sockfd = sockfd;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_connection_destroy(wwm_connection_t conn)
{
    free(conn);
}

//------------------------------------------------------------------------------
/**
*/
int
wwm_connection_handle_input(wwm_connection_t conn,
                            wwm_frame_t decoded_frame)
{
    return 0;
}

//------------------------------------------------------------------------------
/**
*/
int
wwm_connection_send_buffer(wwm_connection_t conn,
                           const wwm_buffer_t buffer)
{
    int bytes_written = write(conn->sockfd, wwm_buffer_bytes(buffer), wwm_buffer_length(buffer));
    // XXX: Check for errors...
    return bytes_written;
}

