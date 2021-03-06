#include "wayward/metrics/stdwwm.h"
#include "wayward/metrics/buffer.h"
#include "wayward/metrics/allocator.h"
#include <string.h>

//------------------------------------------------------------------------------
/**
*/
struct wwm_buffer_t_
{
    wwm_buffer_t    next;
    size_t          size;           ///< Size of the allocated storage.
    size_t          length;         ///< Length of the data in the storage.
    unsigned char   bytes[0];
};

// Round 'a' up to the nearest multiple of 'm'. 'm' MUST be a power of 2.
// http://graphics.stanford.edu/~seander/bithacks.html#ModulusDivisionEasy
#define ROUND_UP(a, m)          ((a) + (m) - ((a) & ((m) - 1)))
#define BUFFER_DATA_INCREMENT   512

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_buffer_new(size_t initial_size)
{
    wwm_buffer_t buf = (wwm_buffer_t)g_wwm_allocator.malloc(sizeof(struct wwm_buffer_t_) + initial_size * sizeof(unsigned char));
    if (NULL == buf)
    {
        return NULL;
    }
    buf->next = NULL;
    buf->size = initial_size;
    buf->length = 0;
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_buffer_destroy(wwm_buffer_t buf)
{
    g_wwm_allocator.free(buf);
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_buffer_resize(wwm_buffer_t buf, size_t new_size)
{
    // This doesn't bother with shrinking currently.
    if (buf->size < new_size)
    {
        size_t alloc_size = ROUND_UP(sizeof(struct wwm_buffer_t_) + new_size, BUFFER_DATA_INCREMENT);
        buf = g_wwm_allocator.realloc(buf, alloc_size);
        buf->size = alloc_size - sizeof(struct wwm_buffer_t_);
        // This doesn't initialize any new memory in the bytes array.
    }
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_buffer_ensure(wwm_buffer_t buf, size_t space_needed)
{
    size_t space_available = buf->size - buf->length;
    if (space_available < space_needed)
    {
        buf = wwm_buffer_resize(buf, buf->length + space_needed);
    }
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_buffer_reset(wwm_buffer_t buf)
{
    buf->length = 0;
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

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_buffer_append_int8(wwm_buffer_t buf, int8_t value)
{
    buf = wwm_buffer_ensure(buf, 1);
    memcpy(buf->bytes + buf->length, &value, 1);
    buf->length += 1;
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_buffer_append_int16(wwm_buffer_t buf, int16_t value)
{
    int16_t network_value = htons(value);

    buf = wwm_buffer_ensure(buf, 2);
    memcpy(buf->bytes + buf->length, &network_value, 2);
    buf->length += 2;
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_buffer_append_int32(wwm_buffer_t buf, int32_t value)
{
    int32_t network_value = htonl(value);

    buf = wwm_buffer_ensure(buf, 4);
    memcpy(buf->bytes + buf->length, &network_value, 4);
    buf->length += 4;
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_buffer_append_bytes(wwm_buffer_t buf, const unsigned char * bytes, size_t length)
{
    buf = wwm_buffer_ensure(buf, length);
    memcpy(buf->bytes + buf->length, bytes, length);
    buf->length += length;
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_buffer_append_buffer(wwm_buffer_t buf, const wwm_buffer_t value)
{
    buf = wwm_buffer_ensure(buf, value->length);
    memcpy(buf->bytes + buf->length, value->bytes, value->length);
    return buf;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_buffer_set_next(wwm_buffer_t buf, wwm_buffer_t next)
{
    buf->next = next;
}

//------------------------------------------------------------------------------
/**
*/
wwm_buffer_t
wwm_buffer_get_next(wwm_buffer_t buf)
{
    return buf->next;
}


