include_directories(
    ../include
    D:/dev/libevent/include
    D:/dev/libevent/compat
    D:/dev/libevent/WIN32-Code
    D:/dev/avro/lang/c/jansson/src
    D:/Dev/avro/lang/c/src
)

set(include_http_src
    ../include/http/server.h
)

set(include_wsgi_src
    ../include/wsgi/context.h
    ../include/wsgi/input_stream.h
    ../include/wsgi/module.h
    ../include/wsgi/request.h
    ../include/wsgi/server.h
)

set(include_src
    ../include/bootstrapper.h
    ../include/config.h
    ../include/mime_types.h
    ../include/url_resolver.h
)

set(src_http_src
    http/server.c
)

set(src_wsgi_src
    wsgi/context.c
    wsgi/input_stream.c
    wsgi/module.c
    wsgi/request.c
    wsgi/server.c
)

set(src_src
    bootstrapper.c
    main.c
    mime_types.c
    url_resolver.c
)

set(webserver_SRCS
    ${include_http_src}
    ${include_wsgi_src}
    ${include_src}
    ${src_http_src}
    ${src_wsgi_src}
    ${src_src}
)

source_group(include FILES ${include_src})
source_group(include\\http FILES ${include_http_src})
source_group(include\\wsgi FILES ${include_wsgi_src})
source_group(src FILES ${src_src})
source_group(src\\http FILES ${src_http_src})
source_group(src\\wsgi FILES ${src_wsgi_src})

add_executable(webserver ${webserver_SRCS})
target_link_libraries(webserver 
    libevent
    avro
    Ws2_32
)
