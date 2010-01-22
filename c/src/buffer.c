#include "wayward/metrics/buffer.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct wayward_metrics_buffer_t_
{
    size_t          size;           ///< Size of the allocated storage.
    size_t          length;         ///< Length of the data in the storage.
    unsigned char   bytes[0];
};

wayward_metrics_buffer_t wayward_metrics_buffer_new(size_t initial_size)
{
    wayward_metrics_buffer_t buf = (wayward_metrics_buffer_t)malloc(sizeof(struct wayward_metrics_buffer_t_) + initial_size * sizeof(unsigned char));
    if (NULL == buf)
    {
        return NULL;
    }
    buf->size = initial_size;
    buf->length = 0;
    memset(buf->bytes, 0, initial_size);
    return buf;
}

wayward_metrics_buffer_t wayward_metrics_buffer_resize(wayward_metrics_buffer_t buf, size_t new_size)
{
    // XXX: This doesn't bother with shrinking currently.
    if (buf->size < new_size)
    {
        // XXX: This doesn't do any sort of extra growth to avoid extra resizing when appending.
        buf = realloc(buf, sizeof(struct wayward_metrics_buffer_t_) + new_size);
        buf->size = new_size;
        // This doesn't initialize any new memory in the bytes array.
    }
    return buf;
}

void wayward_metrics_buffer_destroy(wayward_metrics_buffer_t buf)
{
    free(buf);
}

const unsigned char * wayward_metrics_buffer_bytes(const wayward_metrics_buffer_t buf)
{
    return buf->bytes;
}

size_t wayward_metrics_buffer_length(const wayward_metrics_buffer_t buf)
{
    return buf->length;
}

