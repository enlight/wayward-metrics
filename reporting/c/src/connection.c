#include "wayward/metrics/stdwwm.h"
#include "wayward/metrics/connection.h"
#include "wayward/metrics/allocator.h"
#include "wayward/metrics/buffer.h"

//------------------------------------------------------------------------------
/**
*/
struct wwm_connection_t_
{
    wwm_socket_t sockd;
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
wwm_socket_t
wwm_connection_get_socket(wwm_connection_t conn)
{
    return conn->sockd;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_connection_set_socket(wwm_connection_t conn, wwm_socket_t sockd)
{
    conn->sockd = sockd;
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
    int bytes_written = wwm_socket_send(conn->sockd, wwm_buffer_bytes(buffer), wwm_buffer_length(buffer));
    // XXX: Check for errors...
    return bytes_written;
}

