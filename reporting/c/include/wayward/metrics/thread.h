#ifndef _WAYWARD_METRICS_THREAD_H_
#define _WAYWARD_METRICS_THREAD_H_

#include <stdint.h>

extern uint64_t wwm_thread_get_current_id(void);

typedef struct wwm_thread_t_ *wwm_thread_t;

#endif

