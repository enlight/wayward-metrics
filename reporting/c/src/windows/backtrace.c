#include "wayward/metrics/backtrace.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

int 
wwm_backtrace_gather(void** trace_buffer, int trace_buffer_len)
{
    return CaptureStackBackTrace(0, trace_buffer_len, trace_buffer, NULL);
}
