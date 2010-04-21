#include <Python.h>
#include "config.h"
#include "wsgi/server.h"
#include "wsgi/module.h"
#include "wsgi/context.h"

struct wsgi_server_t_
{
    PyObject *py_stderr; // the Python stderr stream
    PyObject *py_app; // the WSGI application that will handle requests
    PyThreadState *py_thread_state; // the main interpreter thread state
};

//------------------------------------------------------------------------------
/**
*/
wsgi_server_t 
wsgi_server_new(void)
{
    return calloc(1, sizeof(struct wsgi_server_t_));
}

//------------------------------------------------------------------------------
/**
*/
bool 
wsgi_server_init_python(wsgi_server_t server, char *exe_name, 
                        const char *mod_name, const char *app_name)
{
    PyObject *py_str;
    PyObject *py_module;

    Py_SetProgramName(exe_name);
    //PyEval_InitThreads();
    Py_Initialize();
    wsgi_module_initialize();

    // get a reference to the Python stderr stream
    py_str = PyString_FromString("sys");
    if (NULL == py_str)
    {
        return FALSE;
    }
    py_module = PyImport_Import(py_str);
    Py_DECREF(py_str);
    if (NULL == py_module)
    {
        return FALSE;
    }
    server->py_stderr = PyObject_GetAttrString(py_module, "stderr");
    Py_DECREF(py_module);
    if (NULL == server->py_stderr)
    {
        return FALSE;
    }

    // get a reference to the WSGI application
    py_str = PyString_FromString(mod_name);
    if (NULL == py_str)
    {
        return FALSE;
    }
    py_module = PyImport_Import(py_str);
    Py_DECREF(py_str);
    if (NULL == py_module)
    {
        if (PyErr_Occurred())
        {
            PyErr_Print();
        }
        return FALSE;
    }
    server->py_app = PyObject_GetAttrString(py_module, app_name);
    Py_DECREF(py_module);
    if ((NULL == server->py_app) || !PyCallable_Check(server->py_app)) 
    {
        Py_CLEAR(server->py_app);
        return FALSE;
    }

    //server->py_thread_state = PyEval_SaveThread();
    return TRUE;
}

//------------------------------------------------------------------------------
/**
*/
void
wsgi_server_destroy(wsgi_server_t server)
{
    if (NULL != server->py_thread_state)
    {
        PyEval_RestoreThread(server->py_thread_state);
    }
    Py_XDECREF(server->py_app);
    Py_XDECREF(server->py_stderr);
    Py_Finalize();

    if (NULL != server)
    {
        free(server);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
wsgi_server_handle_request(evhttp_request_t request, void *server_ptr)
{
    wsgi_server_t server = (wsgi_server_t)server_ptr;
    // create a new context for the thread
    wsgi_context_t context = wsgi_context_new();
    if (NULL != context)
    {
        wsgi_context_set_stderr(context, server->py_stderr);
        wsgi_context_set_request(context, request);
        wsgi_context_invoke_app(context, server->py_app);
        wsgi_context_destroy(context);
    }   
}
