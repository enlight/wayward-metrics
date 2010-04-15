#ifdef WIN32
#include <WinSock2.h>
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include "event2/event.h"
#include "event2/http.h"
#include "config.h"
#include "wsgi/server.h"

static int ws_socket_system_initialized = FALSE;
event_base_t base = NULL;

void 
ws_socket_system_initialize(void)
{
    if (!ws_socket_system_initialized)
    {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2,2), &wsaData);
        if (0 != result)
        {
            // failed
        }
        else
        {
            ws_socket_system_initialized = TRUE;
        }
    }
}

void 
ws_socket_system_cleanup(void)
{
    if (ws_socket_system_initialized)
    {
        WSACleanup();

        ws_socket_system_initialized = FALSE;
    }
}

BOOL
WINAPI
ctrl_handler(DWORD ctrl_type) 
{ 
    switch (ctrl_type)
    { 
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
            // FIXME: This doesn't actually work as advertised, the application
            // will only terminate after handling the current request, if there
            // is not request then it will never exit at all. This is probably
            // a libevent bug.
            event_base_loopexit(base, NULL);
            return TRUE; // don't call any other handlers

        case CTRL_BREAK_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            return FALSE;

        default: 
            return FALSE; // find someone else to handle it
    } 
}

void 
ws_libevent_log(int severity, const char *msg)
{
    OutputDebugString(msg);
}

int
main(int argc, char **argv)
{
    evhttp_t http;
    wsgi_server_t wsgi;

    event_set_log_callback(ws_libevent_log);
    ws_socket_system_initialize();

    base = event_base_new();
    http = evhttp_new(base);
    wsgi = wsgi_server_new();

    if (wsgi_server_init_python(wsgi, "test", "app"))
    {
        if (0 == evhttp_bind_socket(http, "127.0.0.1", 8080))
        {
            evhttp_set_gencb(http, wsgi_handle_request, wsgi);
            evhttp_set_timeout(http, 60);
        }

        SetConsoleCtrlHandler(ctrl_handler, TRUE);

        event_base_dispatch(base); // loop
    }

    wsgi_server_destroy(wsgi);
    evhttp_free(http);
    event_base_free(base);
    ws_socket_system_cleanup();
}
