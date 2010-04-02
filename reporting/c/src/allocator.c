#include "wayward/metrics/stdwwm.h"
#include "wayward/metrics/allocator.h"

struct wwm_allocator_t_ g_wwm_allocator = {
    NULL,   // malloc
    NULL,   // calloc
    NULL,   // realloc
    NULL    // free
};

