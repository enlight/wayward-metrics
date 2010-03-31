include_directories(${WAYWARD_METRICS_SOURCE_DIR}/include)

add_executable(test
    main.c
)

if(WIN32)
    set(test_libs 
        waywardmetrics
        Ws2_32
    )
else()
    set(test_libs
        waywardmetrics
        pthread
    )
endif()

target_link_libraries(test ${test_libs})
