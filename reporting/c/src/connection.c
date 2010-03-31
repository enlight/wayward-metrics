#include "wayward/metrics/connection.h"
#include "wayward/metrics/allocator.h"
#include "wayward/metrics/buffer.h"
#ifndef WIN32
#include <unistd.h>
#endif

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
    wwm_connection_t conn = (wwm_connection_t)g_wwm_allocator.calloc(1, sizeof(struct wwm_connection_t_));
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
    g_wwm_allocator.free(conn);
}

//------------------------------------------------------------------------------
/**
*/
int
wwm_connection_handle_input(wwm_connection_t conn,
                            wwm_buffer_t out_buffer) 
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

