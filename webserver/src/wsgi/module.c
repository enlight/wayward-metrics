#include <Python.h>
#include "config.h"
#include "wsgi/server.h"
#include "wsgi/request.h"

static PyMethodDef wsgi_module_methods[] =
{
    { NULL } // sentinel
};

//------------------------------------------------------------------------------
/**
    This doesn't conform to the naming convention for python module
    initialization functions, but that's ok because we only call it internally.
*/
void
wsgi_module_initialize(void)
{
    PyObject *m;

    if (PyType_Ready(&wsgi_request_type) < 0)
    {
        return;
    }

    m = Py_InitModule3("wwm_wsgi", wsgi_module_methods, "Wayward Metrics WSGI");

    if (NULL == m)
    {
        return;
    }

    Py_INCREF(&wsgi_request_type);
    PyModule_AddObject(m, "Request", (PyObject *)&wsgi_request_type);
}
