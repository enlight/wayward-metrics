#include "wayward/metrics/stdwwm.h"
#include "wayward/metrics/time.h"
#include <sys/types.h>
#include <sys/timeb.h>

int 
gettimeofday(struct timeval* timestamp, void* tz)
{
    struct _timeb now;
    _ftime(&now);
    // seconds since midnight January 1, 1970 UTC
    timestamp->tv_sec = now.time;
    // convert the fractional part of a second from milliseconds to microseconds
    timestamp->tv_usec = now.millitm * 1000;
    
    return 0;
}
