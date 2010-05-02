#ifdef WIN32
#include <WinSock2.h>
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include "config.h"
#include "bootstrapper.h"
#include "avro.h"

static int ws_socket_system_initialized = FALSE;

static
bool 
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
    return ws_socket_system_initialized;
}

static
void 
ws_socket_system_cleanup(void)
{
    if (ws_socket_system_initialized)
    {
        WSACleanup();

        ws_socket_system_initialized = FALSE;
    }
}

int
main(int argc, char **argv)
{
    wwm_bootstrapper_t bs;

    if (!ws_socket_system_initialize())
    {
        return EXIT_FAILURE;
    }

    avro_init(); // need this to use jansson

    bs = wwm_boostrapper_new();
    
    if (!wwm_bootstrapper_parse_cmdline(bs, argc, argv))
    {
        return EXIT_FAILURE;
    }
    
    if (!wwm_bootstrapper_configure(bs))
    {
        return EXIT_FAILURE;
    }
    
    wwm_bootstrapper_run(bs);
    
    wwm_bootstrapper_destroy(bs);
    
    ws_socket_system_cleanup();

    return EXIT_SUCCESS;
}
