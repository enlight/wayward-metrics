#ifndef _WAYWARD_METRICS_FILE_H_
#define _WAYWARD_METRICS_FILE_H_

#include "wayward/metrics/buffer.h"

typedef struct wwm_file_t_ *wwm_file_t;

enum wwm_file_mode_e
{
    WWM_FILE_READ_ONLY,
    WWM_FILE_WRITE_ONLY,
    WWM_FILE_READ_WRITE,
    WWM_FILE_APPEND,
};

extern wwm_file_t   wwm_file_new(void);
extern void         wwm_file_destroy(wwm_file_t file);

extern int          wwm_file_open(wwm_file_t file, const char *filename, enum wwm_file_mode_e mode);
extern void         wwm_file_close(wwm_file_t file);

extern int          wwm_file_send_buffer(wwm_file_t file,
                                         const wwm_buffer_t buffer);

#endif


