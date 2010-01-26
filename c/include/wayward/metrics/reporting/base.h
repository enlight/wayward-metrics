#ifndef _WAYWARD_METRICS_REPORTING_BASE_H_
#define _WAYWARD_METRICS_REPORTING_BASE_H_

#include "wayward/metrics/buffer.h"

typedef struct wwm_reporter_t_ *wwm_reporter_t;

extern wwm_reporter_t   wwm_reporter_new(const char *hostname, int portnumber);
extern void             wwm_reporter_destroy(wwm_reporter_t reporter);

extern void             wwm_reporter_start_session(wwm_reporter_t reporter, const char *session_id);
extern void             wwm_reporter_record_data(wwm_reporter_t reporter, wwm_buffer_t data);

#endif

