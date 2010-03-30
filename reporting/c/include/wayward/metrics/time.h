#ifndef _WAYWARD_METRICS_TIME_H_
#define _WAYWARD_METRICS_TIME_H_

#ifdef WIN32

#include <WinSock2.h> // for timeval

extern int gettimeofday(struct timeval* timestamp, void* tz);

#else // if not WIN32

#include <sys/time.h>

#endif // WIN32

#endif // _WAYWARD_METRICS_TIME_H_
