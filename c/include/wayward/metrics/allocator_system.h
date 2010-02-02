#ifndef _WAYWARD_METRICS_ALLOCATOR_SYSTEM_H_
#define _WAYWARD_METRICS_ALLOCATOR_SYSTEM_H_

#include <stddef.h>

extern void   wwm_allocator_system_initialize(void);
extern void* wwm_allocator_system_malloc(size_t size);
extern void* wwm_allocator_system_calloc(size_t count, size_t size);
extern void* wwm_allocator_system_realloc(void *ptr, size_t size);
extern void  wwm_allocator_system_free(void *ptr);

#endif

