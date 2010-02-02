#include "wayward/metrics/file.h"

#include "wayward/metrics/allocator.h"

#include <fcntl.h>
#include <sys/stat.h>

//------------------------------------------------------------------------------
/**
*/
struct wwm_file_t_
{
    int         fd;
};

//------------------------------------------------------------------------------
/**
*/
wwm_file_t
wwm_file_new(void)
{
    wwm_file_t file = g_wwm_allocator.calloc(1, sizeof(struct wwm_file_t_));
    file->fd = -1;
    return file;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_file_destroy(wwm_file_t file)
{
    wwm_file_close(file);
    g_wwm_allocator.free(file);
}

//------------------------------------------------------------------------------
/**
*/
int
wwm_file_open(wwm_file_t file, const char *filename, enum wwm_file_mode_e mode)
{
    // Make sure any open file here is closed.
    wwm_file_close(file);

    int mapped_mode = 0;
    switch (mode)
    {
        case WWM_FILE_READ_ONLY:
            mapped_mode = O_RDONLY | O_CREAT;
            break;
        case WWM_FILE_WRITE_ONLY:
            mapped_mode = O_WRONLY | O_CREAT;
            break;
        case WWM_FILE_READ_WRITE:
            mapped_mode = O_RDWR | O_CREAT;
            break;
        case WWM_FILE_APPEND:
            mapped_mode = O_WRONLY | O_APPEND | O_CREAT;
            break;
    }
    file->fd = open(filename, mapped_mode, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (-1 != file->fd)
    {
        return file->fd;
    }
    return 0;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_file_close(wwm_file_t file)
{
    if (-1 != file->fd)
    {
        close(file->fd);
        file->fd = -1;
    }
}

//------------------------------------------------------------------------------
/**
*/
int
wwm_file_send_buffer(wwm_file_t file, const wwm_buffer_t buffer)
{
    if (file->fd != -1)
    {
        return write(file->fd, wwm_buffer_bytes(buffer), wwm_buffer_length(buffer));
    }
    return -1;
}

