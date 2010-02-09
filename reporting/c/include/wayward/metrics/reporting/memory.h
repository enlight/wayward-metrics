#ifndef _WAYWARD_METRICS_REPORTING_MEMORY_H_
#define _WAYWARD_METRICS_REPORTING_MEMORY_H_

#include "wayward/metrics/reporting/reporter.h"

extern void wwm_reporter_memory_heap_new(wwm_reporter_t reporter, int32_t heap_id, const char* heap_name);
extern void wwm_reporter_memory_heap_destroy(wwm_reporter_t reporter, int32_t heap_id);

extern void wwm_reporter_memory_malloc(wwm_reporter_t reporter, int32_t heap_id, void* addr, size_t);
extern void wwm_reporter_memory_realloc(wwm_reporter_t reporter, int32_t heap_id, void* old_addr, void* new_addr, size_t new_size);
extern void wwm_reporter_memory_free(wwm_reporter_t reporter, int32_t heap_id, void* addr);

extern void wwm_reporter_memory_gc_start(wwm_reporter_t reporter, int32_t heap_id);
extern void wwm_reporter_memory_gc_end(wwm_reporter_t reporter, int32_t heap_id);
    
#endif

