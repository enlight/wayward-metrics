#include "wayward/metrics/stdwwm.h"
#include "wayward/metrics/backtrace.h"

int 
wwm_backtrace_gather(void** trace_buffer, int trace_buffer_len)
{
    return CaptureStackBackTrace(0, trace_buffer_len, trace_buffer, NULL);
}
