#include <Python.h>
#include "config.h"
#include "wsgi/context.h"
#include "wsgi/request.h"
#include "event2/http.h"

struct wsgi_context_t_
{
    PyObject *py_stderr; // borrowed reference, server owns it
    evhttp_request_t request;
};

//------------------------------------------------------------------------------
/**
*/
wsgi_context_t
wsgi_context_new(void)
{
    return calloc(1, sizeof(struct wsgi_context_t_));
}

//------------------------------------------------------------------------------
/**
*/
void
wsgi_context_destroy(wsgi_context_t context)
{
    free(context);
}

//------------------------------------------------------------------------------
/**
*/
void 
wsgi_context_set_stderr(wsgi_context_t context, PyObject *py_stderr)
{
    context->py_stderr = py_stderr;
}

//------------------------------------------------------------------------------
/**
*/
PyObject * 
wsgi_context_get_stderr(wsgi_context_t context)
{
    return context->py_stderr;
}

//------------------------------------------------------------------------------
/**
*/
void 
wsgi_context_set_request(wsgi_context_t context, evhttp_request_t request)
{
    context->request = request;
}

//------------------------------------------------------------------------------
/**
*/
evhttp_request_t 
wsgi_context_get_request(wsgi_context_t context)
{
    return context->request;
}

//------------------------------------------------------------------------------
/**
*/
static
void
_invoke_app(wsgi_context_t context, PyObject *app)
{
    wsgi_request_t request = wsgi_request_new(context);
    if (NULL != request)
    {
        PyObject *result = wsgi_request_invoke_app(request, app);
        if (NULL == result)
        {
            if (PyErr_Occurred())
            {
                PyErr_Print();
            }
        }
        else
        {
            wsgi_request_send_response(request, result);
        }
        wsgi_request_destroy(request);
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
wsgi_context_invoke_app(wsgi_context_t context, PyObject *app)
{
    // FIXME: uncomment this when I add multi-threading
    //PyGILState_STATE gil_state = PyGILState_Ensure();
    _invoke_app(context, app);
    //PyGILState_Release(gil_state)
}
