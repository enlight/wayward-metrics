include_directories(
    ../include
    D:/dev/libevent/include
    D:/dev/libevent/compat
    D:/dev/libevent/WIN32-Code
)

set(include_wsgi_src
    ../include/wsgi/context.h
    ../include/wsgi/module.h
    ../include/wsgi/request.h
    ../include/wsgi/server.h
)

set(include_src
    ../include/config.h
)

set(src_wsgi_src
    wsgi/context.c
    wsgi/module.c
    wsgi/request.c
    wsgi/server.c
)

set(src_src
    main.c
)

set(webserver_SRCS
    ${include_wsgi_src}
    ${include_src}
    ${src_wsgi_src}
    ${src_src}
)

source_group(include FILES ${include_src})
source_group(include\\wsgi FILES ${include_wsgi_src})
source_group(src FILES ${src_src})
source_group(src\\wsgi FILES ${src_wsgi_src})

add_executable(webserver ${webserver_SRCS})
target_link_libraries(webserver 
    libevent
    Ws2_32
)
