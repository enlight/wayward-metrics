include_directories(${WAYWARD_METRICS_SOURCE_DIR}/include)

set(include_wayward_src
    ../include/wayward/metrics.h
)
set(include_wayward_metrics_src
    ../include/wayward/metrics/allocator.h
    ../include/wayward/metrics/allocator_system.h
    ../include/wayward/metrics/buffer.h
    ../include/wayward/metrics/codec.h
    ../include/wayward/metrics/config.h
    ../include/wayward/metrics/connection.h
    ../include/wayward/metrics/file.h
    ../include/wayward/metrics/message_queue.h
    ../include/wayward/metrics/socket.h
    ../include/wayward/metrics/stdwwm.h
    ../include/wayward/metrics/backtrace.h
    ../include/wayward/metrics/time.h
    ../include/wayward/metrics/thread.h
    ../include/wayward/metrics/types.h
    ../include/wayward/metrics/version.h
)
set(include_wayward_metrics_reporting_src
    ../include/wayward/metrics/reporting/constants.h
    ../include/wayward/metrics/reporting/context.h
    ../include/wayward/metrics/reporting/memory.h
    ../include/wayward/metrics/reporting/reporter.h
)
set(include_wayward_metrics_reporting_private_src
    ../include/wayward/metrics/reporting/private/reporter.h
)
set(include_wayward_metrics_posix_src
    ../include/wayward/metrics/posix/platform.h
)
set(include_wayward_metrics_windows_src
    ../include/wayward/metrics/windows/platform.h
)
set(src_src
    allocator.c
    allocator_system.c
    buffer.c
    codec.c
    connection.c
    message_queue.c
    version.c
)
set(src_reporting_src
    reporting/context.c
    reporting/memory.c
    reporting/reporter.c
)
set(src_posix_src
    posix/file.c
    posix/socket.c
    posix/backtrace.c
    posix/thread.c
)
set(src_windows_src
    windows/file.c
    windows/socket.c
    windows/backtrace.c
    windows/time.c
    windows/thread.c
)
set(waywardmetrics_SRCS
    ${include_wayward_src}
    ${include_wayward_metrics_src}
    ${include_wayward_metrics_reporting_src}
    ${include_wayward_metrics_reporting_private_src}
    ${src_src}
    ${src_reporting_src}
)
if(WIN32)
    set(waywardmetrics_SRCS
        ${waywardmetrics_SRCS}
        ${include_wayward_metrics_windows_src}
        ${src_windows_src}
    )
else()
    set(waywardmetrics_SRCS
        ${waywardmetrics_SRCS}
        ${include_wayward_metrics_posix_src}
        ${src_posix_src}
    )
endif()

# organize the source in the Visual Studio solution explorer
source_group(include\\wayward FILES ${include_wayward_src})
source_group(include\\wayward\\metrics FILES ${include_wayward_metrics_src})
source_group(include\\wayward\\metrics\\reporting FILES ${include_wayward_metrics_reporting_src})
source_group(include\\wayward\\metrics\\private FILES ${include_wayward_metrics_reporting_private_src})
source_group(include\\wayward\\metrics\\posix FILES ${include_wayward_metrics_posix_src})
source_group(include\\wayward\\metrics\\windows FILES ${include_wayward_metrics_windows_src})
source_group(src FILES ${src_src})
source_group(src\\posix FILES ${src_posix_src})
source_group(src\\reporting FILES ${src_reporting_src})
source_group(src\\windows FILES ${src_windows_src})

add_library(waywardmetrics ${waywardmetrics_SRCS})
