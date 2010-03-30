#include "wayward/metrics/socket.h"
#include "wayward/metrics/types.h"
#include <errno.h>
#include <stddef.h>
#include <WinSock2.h>

//------------------------------------------------------------------------------
/**
*/
int
wwm_open_socket(char const *hostname,
                int portnumber)
{
    int sockfd;
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
        closesocket(sockfd);
        return result;
    }

    return sockfd;
}

