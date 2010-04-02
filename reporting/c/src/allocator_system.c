#include "wayward/metrics/stdwwm.h"
#include "wayward/metrics/allocator_system.h"
#include "wayward/metrics/allocator.h"

#include <stdlib.h>

//------------------------------------------------------------------------------
/**
*/
void
wwm_allocator_system_initialize(void)
{
    g_wwm_allocator.malloc  = wwm_allocator_system_malloc;
    g_wwm_allocator.calloc  = wwm_allocator_system_calloc;
    g_wwm_allocator.realloc = wwm_allocator_system_realloc;
    g_wwm_allocator.free    = wwm_allocator_system_free;
}

//------------------------------------------------------------------------------
/**
*/
void*
wwm_allocator_system_malloc(size_t size)
{
    return malloc(size);
}

//------------------------------------------------------------------------------
/**
*/
void*
wwm_allocator_system_calloc(size_t count, size_t size)
{
    return calloc(count, size);
}

//------------------------------------------------------------------------------
/**
*/
void*
wwm_allocator_system_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_allocator_system_free(void *ptr)
{
    free(ptr);
}

