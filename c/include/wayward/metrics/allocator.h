#ifndef _WAYWARD_METRICS_ALLOCATOR_H_
#define _WAYWARD_METRICS_ALLOCATOR_H_

#include <stddef.h>

typedef void * (*wwm_malloc_def)(size_t size);
typedef void * (*wwm_calloc_def)(size_t count, size_t size);
typedef void * (*wwm_realloc_def)(void *ptr, size_t size);
typedef void   (*wwm_free_def)(void *ptr);

struct wwm_allocator_t_
{
    wwm_malloc_def          malloc;
    wwm_calloc_def          calloc;
    wwm_realloc_def         realloc;
    wwm_free_def            free;
};

extern struct wwm_allocator_t_ g_wwm_allocator;

#endif

