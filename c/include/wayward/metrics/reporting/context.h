#ifndef _WAYWARD_METRICS_REPORTING_CONTEXT_H_
#define _WAYWARD_METRICS_REPORTING_CONTEXT_H_

#include "wayward/metrics/reporting/reporter.h"

extern void wwm_reporter_context_create(wwm_reporter_t reporter, uint64_t context_id, const char *name);
extern void wwm_reporter_context_enter(wwm_reporter_t reporter, uint64_t context_id);
extern void wwm_reporter_context_exit(wwm_reporter_t reporter);

#endif

