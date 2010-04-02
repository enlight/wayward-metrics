#ifndef _WAYWARD_METRICS_SOCKET_H_
#define _WAYWARD_METRICS_SOCKET_H_

#ifdef WIN32
typedef SOCKET wwm_socket_t;
#else
typedef int wwm_socket_t;
#endif

void wwm_socket_system_initialize(void);
void wwm_socket_system_cleanup(void);

wwm_socket_t wwm_socket_open(char const *hostname, int portnumber);
int          wwm_socket_send(wwm_socket_t sockd, const char* buffer, size_t buffer_len);

#endif

