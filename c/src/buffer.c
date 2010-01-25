#include "wayward/metrics/buffer.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//------------------------------------------------------------------------------
/**
*/
struct wwm_buffer_t_
{
    size_t          size;           ///< Size of the allocated storage.
    size_t          length;         ///< Length of the data in the storage.
    unsigned char   bytes[0];
};

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_buffer_new(size_t initial_size)
{
    wwm_buffer_t buf = (wwm_buffer_t)malloc(sizeof(struct wwm_buffer_t_) + initial_size * sizeof(unsigned char));
    if (NULL == buf)
    {
        return NULL;
    }
    buf->size = initial_size;
    buf->length = 0;
    memset(buf->bytes, 0, initial_size);
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_buffer_resize(wwm_buffer_t buf, size_t new_size)
{
    // XXX: This doesn't bother with shrinking currently.
    if (buf->size < new_size)
    {
        // XXX: This doesn't do any sort of extra growth to avoid extra resizing when appending.
        buf = realloc(buf, sizeof(struct wwm_buffer_t_) + new_size);
        buf->size = new_size;
        // This doesn't initialize any new memory in the bytes array.
    }
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_buffer_destroy(wwm_buffer_t buf)
{
    free(buf);
}

//------------------------------------------------------------------------------
/**
*/
const unsigned char*
wwm_buffer_bytes(const wwm_buffer_t buf)
{
    return buf->bytes;
}

//------------------------------------------------------------------------------
/**
*/
size_t
wwm_buffer_length(const wwm_buffer_t buf)
{
    return buf->length;
}

