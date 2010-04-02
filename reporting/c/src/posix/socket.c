#include "wayward/metrics/stdwwm.h"
#include "wayward/metrics/socket.h"

#include <errno.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

//------------------------------------------------------------------------------
/**
*/
void 
wwm_socket_system_initialize(void)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void 
wwm_socket_system_cleanup(void)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
wwm_socket_t
wwm_socket_open(char const *hostname,
                int portnumber)
{
    wwm_socket_t sockfd;
    struct sockaddr_in addr;
    struct hostent *he;

    he = gethostbyname(hostname);
    if (he == NULL)
    {
        return -ENOENT;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(portnumber);
    addr.sin_addr.s_addr = * (uint32_t *) he->h_addr_list[0];

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        int result = -errno;
        close(sockfd);
        return result;
    }

    return sockfd;
}

//------------------------------------------------------------------------------
/**
    Sends the given data on the given socket and returns the number of bytes
    sent.
*/
int
wwm_socket_send(wwm_socket_t sockd, const char* buffer, size_t buffer_len)
{
    return write(sockd, buffer, buffer_len, 0);
}
