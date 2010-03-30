include_directories(${WAYWARD_METRICS_SOURCE_DIR}/include)

add_executable(test
    main.c
)

target_link_libraries(test
    waywardmetrics
    pthread
)
