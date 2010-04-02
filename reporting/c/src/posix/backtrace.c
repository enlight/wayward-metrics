#include "wayward/metrics/stdwwm.h"
#include "wayward/metrics/backtrace.h"
#include <execinfo.h>

int 
wwm_backtrace_gather(void** trace_buffer, int trace_buffer_len)
{
    return backtrace(trace_buffer, trace_buffer_len);
}
