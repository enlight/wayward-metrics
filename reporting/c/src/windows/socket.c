#include "wayward/metrics/stdwwm.h"
#include "wayward/metrics/socket.h"
#include "wayward/metrics/types.h"
#include <errno.h>
#include <stddef.h>

//------------------------------------------------------------------------------
/**
    Initialize WinSock2, must be done before any sockets are created.
*/
void wwm_socket_system_initialize(void)
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (0 != result)
    {
        // failed
    }
}

//------------------------------------------------------------------------------
/**
    Cleanup WinSock2.
*/
void wwm_socket_system_cleanup(void)
{
    WSACleanup();
}

//------------------------------------------------------------------------------
/**
*/
wwm_socket_t
wwm_socket_open(char const *hostname, int portnumber)
{
    wwm_socket_t sockd;
    struct sockaddr_in addr;
    struct hostent *he;

    he = gethostbyname(hostname);
    if (he == NULL)
    {
        return INVALID_SOCKET;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(portnumber);
    addr.sin_addr.s_addr = * (uint32_t *) he->h_addr_list[0];

    sockd = socket(PF_INET, SOCK_STREAM, 0);
    if (connect(sockd, (struct sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
    {
        closesocket(sockd);
        return INVALID_SOCKET;
    }

    return sockd;
}

//------------------------------------------------------------------------------
/**
    Sends the given data on the given socket and returns the number of bytes
    sent.
*/
int
wwm_socket_send(wwm_socket_t sockd, const char* buffer, size_t buffer_len)
{
    return send(sockd, buffer, buffer_len, 0);
}
