#include <Python.h>
#include "config.h"
#include "wsgi/request.h"
#include "wsgi/context.h"
#include "sys/queue.h" // for TAILQ_FOREACH (must be included before the _struct.h headers)
#include "event2/util.h"
#include "event2/event_struct.h"
#include "event2/http.h"
#include "event2/http_struct.h"

struct wsgi_request_t_
{
    PyObject_HEAD
    wsgi_context_t context;
    PyObject *environ;
    bool response_headers_packed;
    PyObject *response_headers;
    PyObject *app_result; // returned by the WSGI application
};

PyObject * _wsgi_request_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
int        _wsgi_request_init(wsgi_request_t self, PyObject *args, PyObject *kwds);
void       _wsgi_request_dealloc(wsgi_request_t self);
PyObject * _wsgi_request_start_response(wsgi_request_t self, PyObject *args);
PyObject * _wsgi_request_write(wsgi_request_t self, PyObject *args);
bool       _wsgi_request_fill_environ(wsgi_request_t self);

//------------------------------------------------------------------------------
/**
    Creates a new Python request object associated with the given context. 
*/
wsgi_request_t
wsgi_request_new(wsgi_context_t context)
{
    // pass the context into the request constructor
    PyObject *py_context = PyCObject_FromVoidPtr(context, NULL);
    if (NULL != py_context)
    {
        PyObject *py_args = Py_BuildValue("(O)", py_context);
        Py_DECREF(py_context);
        if (NULL != py_args)
        {
            wsgi_request_t request = (wsgi_request_t)PyObject_CallObject(
                (PyObject *)&wsgi_request_type, py_args
            );
            Py_DECREF(py_args);
            return request;
        }
    }
    return NULL;
}

//------------------------------------------------------------------------------
/**
    Clears any references the given request object holds to other python objects.
*/
static
void
_wsgi_request_clear_refs(wsgi_request_t request)
{
    Py_CLEAR(request->app_result);
    Py_CLEAR(request->response_headers);
    Py_CLEAR(request->environ);
}

//------------------------------------------------------------------------------
/**
    Destroys a Python request object created by wsgi_request_new().
*/
void
wsgi_request_destroy(wsgi_request_t request)
{
    _wsgi_request_clear_refs(request);
    Py_DECREF(request);
}

//------------------------------------------------------------------------------
/**
    Invokes the given WSGI application passing in the environ dictionary and
    start_response callable associated with the given request.

    @return The result (a new reference) produced by the application.
*/
PyObject *
wsgi_request_invoke_app(wsgi_request_t request, PyObject *app)
{
    PyObject *start_response = PyObject_GetAttrString((PyObject *)request, "start_response");
    if (NULL != start_response)
    {
        PyObject *args = Py_BuildValue("(OO)", request->environ, start_response);
        Py_DECREF(start_response);
        if (NULL != args)
        {
            PyObject *result = PyObject_CallObject(app, args);
            Py_DECREF(args);
            return result;
        }
    }
    return NULL;    
}

//------------------------------------------------------------------------------
/**
    Takes all the HTTP response headers in the Python request and puts them
    into the libevent request.
*/
static
bool
_wsgi_pack_headers(wsgi_request_t request)
{
    Py_ssize_t num_headers;
    Py_ssize_t i;
    evkeyvalq_t headers;
    
    num_headers = PyList_Size(request->response_headers);
    if (PyErr_Occurred())
    {
        return FALSE;
    }

    headers = evhttp_request_get_output_headers(wsgi_context_get_request(request->context));
    for (i = 0; i < num_headers; i++)
    {
        const char *header;
        const char *value;

        PyObject *item = PyList_GetItem(request->response_headers, i);
        if (NULL == item)
        {
            return FALSE;
        }
        if (!PyArg_ParseTuple(item, "ss", &header, &value))
        {
            return FALSE;
        }
        if (0 != evhttp_add_header(headers, header, value))
        {
            return FALSE;
        }
    }

    request->response_headers_packed = TRUE;

    return TRUE;
}

//------------------------------------------------------------------------------
/**
*/
static
bool
_wsgi_send_body(wsgi_request_t request, const char *buffer, size_t buffer_size)
{
    // Implement this and we might actually be able to run this thing.
    return TRUE;
}

//------------------------------------------------------------------------------
/**
*/
bool 
_wsgi_send_response(wsgi_request_t request, PyObject *app_result)
{
    PyObject *it;
    PyObject *data;

    it = PyObject_GetIter(app_result);
    if (NULL == it)
    {
        return FALSE;
    }

    while (data = PyIter_Next(it))
    {
        Py_ssize_t data_size = PyString_Size(data);
        if (PyErr_Occurred())
        {
            Py_DECREF(data);
            break;
        }

        if (data_size > 0)
        {
            const char *buffer = PyString_AsString(data);
            if (NULL == buffer)
            {
                Py_DECREF(data);
                break;
            }
            
            if (!request->response_headers_packed)
            {
                if (!_wsgi_pack_headers(request))
                {
                    Py_DECREF(data);
                    break;
                }
            }

            if (!_wsgi_send_body(request, buffer, data_size))
            {
                Py_DECREF(data);
                break;
            }
        }

        Py_DECREF(data);
    }

    Py_DECREF(it);

    if (PyErr_Occurred())
    {
        return FALSE;
    }

    // if there was no data to send still send the headers
    if (!request->response_headers_packed)
    {
        if (!_wsgi_pack_headers(request))
        {
            return FALSE;
        }
    }

    return TRUE;
}

//------------------------------------------------------------------------------
/**
    Calls the close() method on the given result object.
*/
static
void
_wsgi_result_close(PyObject *result)
{
    // save exception state
    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);

    if (PyObject_HasAttrString(result, "close")) 
    {
        PyObject *method = PyObject_GetAttrString(result, "close");
        if (NULL != method) 
        {
            PyObject *args = PyTuple_New(0);
            if (NULL != args) 
            {
                PyObject *val = PyObject_CallObject(method, args);
                Py_DECREF(args);
                Py_XDECREF(val);
            }
            Py_DECREF(method);
        }
    }

    PyErr_Restore(type, value, traceback);
}

//------------------------------------------------------------------------------
/**
*/
void 
wsgi_request_send_response(wsgi_request_t request, PyObject *app_result)
{
    request->app_result = app_result;
    _wsgi_send_response(request, app_result);
    _wsgi_result_close(app_result);
}

//------------------------------------------------------------------------------
/**
    Allocates memory for a request object.
*/
PyObject *
_wsgi_request_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    wsgi_request_t self = (wsgi_request_t)type->tp_alloc(type, 0);
    if (NULL != self)
    {
        self->environ = PyDict_New();
        if (NULL == self->environ) 
        {
            Py_DECREF(self);
            return NULL;
        }

        self->context = NULL;
        self->response_headers = NULL;
        self->response_headers_packed = FALSE;
        self->app_result = NULL;
    }
    return (PyObject *)self;
}

//------------------------------------------------------------------------------
/**
    Initializes a request object.
*/
int
_wsgi_request_init(wsgi_request_t self, PyObject *args, PyObject *kwds)
{
    // the request constructor takes a context object
    PyObject *py_context;
    if (!PyArg_ParseTuple(args, "O!", &PyCObject_Type, &py_context))
    {
        return -1;
    }

    self->context = (wsgi_context_t)PyCObject_AsVoidPtr(py_context);

    if (!_wsgi_request_fill_environ(self))
    {
        return -1;
    }

    return 0;
}

//------------------------------------------------------------------------------
/**
*/
void
_wsgi_request_dealloc(wsgi_request_t self)
{
    _wsgi_request_clear_refs(self);
    self->ob_type->tp_free((PyObject *)self);
}

//------------------------------------------------------------------------------
/**
*/
PyObject * 
_wsgi_request_start_response(wsgi_request_t self, PyObject *args)
{
    PyObject *status, *response_headers, *exc_info = NULL;
    PyObject *tmp;

    // start_response(status, response_headers, exc_info = None)
    // 
    // The first two parameters are required, the third should only be provided
    // in case the WSGI application encounters an exception.
    if (!PyArg_ParseTuple(args, "SO!|O:start_response", 
                          &status, &PyList_Type, &response_headers, 
                          &exc_info))
    {
        return NULL;
    }

    if ((NULL != exc_info) && (Py_None != exc_info)) 
    {
        // The WSGI application encountered an exception.
        // MUST raise an exception if the headers have already been sent,
        // SHOULD propagate the exception set by the WSGI application.
        if (self->response_headers_packed) 
        {
            PyObject *type, *value, *traceback;
            if (!PyArg_ParseTuple(exc_info, "OOO", &type, &value, &traceback))
            {
                return NULL;
            }
            Py_INCREF(type);
            Py_INCREF(value);
            Py_INCREF(traceback);
            PyErr_Restore(type, value, traceback);
            return NULL;
        }
    }
    else if (NULL != self->response_headers)
    {
        // It is a fatal error to call start_response() without the exc_info
        // argument if start_response() has already been called within the
        // current invocation of the WSGI application.
        PyErr_SetString(PyExc_AssertionError, "start_response() already called.");
        return NULL;
    }

    // TODO: validate status and headers, some validation will be done by libevent

    // We'll need to convert these from Python to C later, 
    // we don't do it now because the WSGI application may still change them.
    tmp = self->response_headers;
    Py_INCREF(response_headers);
    self->response_headers = response_headers;
    Py_XDECREF(tmp);

    return PyObject_GetAttrString((PyObject *)self, "write");
}

//------------------------------------------------------------------------------
/**
*/
PyObject * 
_wsgi_request_write(wsgi_request_t self, PyObject *args)
{
    PyErr_SetString(
        PyExc_NotImplementedError, 
        "Django doesn't use write(), so we haven't bothered to implement it."
    );
    return NULL;
}

//------------------------------------------------------------------------------
/**
*/
static
bool
_set_dict_key_value_str(PyObject *dict, const char *key, const char *value_str)
{
    PyObject *py_value_str = PyString_FromString(value_str);
    if (NULL != py_value_str)
    {
        int retval = PyDict_SetItemString(dict, key, py_value_str);
        Py_DECREF(py_value_str);
        return (0 == retval);
    }
    return FALSE;
}

//------------------------------------------------------------------------------
/**
    Splits the HTTP Host header value into a name and a port.
    
    Returns true only if both name and port could be extracted.
*/
static
bool
_host_header_split(char *header_val, const char **name, const char **port)
{
    *name = strtok(header_val, ":");
    if (NULL != *name)
    {
        *port = strtok(NULL, ":");
        if (NULL != *port)
        {
            return TRUE;
        }
    }
    return FALSE;
}

//------------------------------------------------------------------------------
/**
*/
static
bool
_environ_set_http_headers(PyObject *environ, evkeyvalq_t headers)
{
    char key_name[256];
    const char *prefix = "HTTP_";
    size_t prefix_len = strlen(prefix);
    size_t max_key_len = sizeof(key_name) - prefix_len - 1;
    evkeyval_t header;
    
    TAILQ_FOREACH(header, headers, next)
    {
        size_t key_len;
        size_t i;

        // extract CGI environment vars from the HTTP headers
        if (evutil_ascii_strcasecmp(header->key, "Host") == 0)
        {
            const char *host_name = NULL;
            const char *host_port = NULL;
            char *host = strdup(header->value);

            if (!_host_header_split(host, &host_name, &host_port))
            {
                free(host);
                return FALSE;
            }
            if (!_set_dict_key_value_str(environ, "SERVER_NAME", host_name))
            {
                free(host);
                return FALSE;
            }
            if (!_set_dict_key_value_str(environ, "SERVER_PORT", host_port))
            {
                free(host);
                return FALSE;
            }
            free(host);
        }
        else if (evutil_ascii_strcasecmp(header->key, "Content-Length") == 0)
        {
            if (!_set_dict_key_value_str(environ, "CONTENT_LENGTH", header->value))
            {
                return FALSE;
            }
        }
        else if (evutil_ascii_strcasecmp(header->key, "Content-Type") == 0)
        {
            if (!_set_dict_key_value_str(environ, "CONTENT_TYPE", header->value))
            {
                return FALSE;
            }
        }

        key_len = strlen(header->key);
        if (key_len > max_key_len)
        {
            key_len = max_key_len;
        }
        // prefix HTTP_, convert to uppercase, and replace dashes with underscores
        strcpy(key_name, prefix);
        for (i = 0; i < key_len; i++)
        {
            if ('-' == header->key[i])
            {
                key_name[prefix_len + i] = '_';
            }
            else
            {
                key_name[prefix_len + i] = toupper(header->key[i]);
            }
        }
        key_name[prefix_len + i] = 0;

        if (!_set_dict_key_value_str(environ, key_name, header->value))
        {
            return FALSE;
        }
    }

    return TRUE;
}

//------------------------------------------------------------------------------
/**
*/
static
const char *
_evhttp_request_method(enum evhttp_cmd_type type)
{
    const char *method;

    switch (type) 
    {
    case EVHTTP_REQ_GET:
        method = "GET";
        break;

    case EVHTTP_REQ_POST:
        method = "POST";
        break;

    case EVHTTP_REQ_HEAD:
        method = "HEAD";
        break;

    case EVHTTP_REQ_PUT:
        method = "PUT";
        break;

    case EVHTTP_REQ_DELETE:
        method = "DELETE";
        break;

    default:
        method = NULL;
        break;
    }

    return method;
}

//------------------------------------------------------------------------------
/**
*/
static
void
_split_uri(char *uri, const char **path, const char **query)
{
    *path = strtok(uri, "?");
    if (NULL == *path)
    {
        *path = "";
        *query = "";
    }
    else
    {
        *query = strtok(NULL, "?");
        if (NULL == *query)
        {
            *query = "";
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
static
bool
_environ_set_cgi_vars(PyObject *environ, evhttp_request_t request)
{
    const char *request_method;
    char server_protocol[16];
    const char *uri;
    char *uri_copy;
    const char *uri_path = NULL;
    const char *uri_query = NULL;

    request_method = _evhttp_request_method(request->type);
    if (NULL == request_method)
    {
        return FALSE;
    }
    if (!_set_dict_key_value_str(environ, "REQUEST_METHOD", request_method))
    {
        return FALSE;
    }
    if (!_set_dict_key_value_str(environ, "SCRIPT_NAME", ""))
    {
        return FALSE;
    }

    uri = evhttp_request_get_uri(request);
    if (NULL == uri)
    {
        return FALSE;
    }
    uri_copy = strdup(uri);
    _split_uri(uri_copy, &uri_path, &uri_query);
    if (!_set_dict_key_value_str(environ, "PATH_INFO", uri_path))
    {
        free(uri_copy);
        return FALSE;
    }
    if (!_set_dict_key_value_str(environ, "QUERY_STRING", uri_query))
    {
        free(uri_copy);
        return FALSE;
    }
    free(uri_copy);

    if (-1 == sprintf(server_protocol, "HTTP/%d.%d", request->major, request->minor))
    {
        return FALSE;
    }
    if (!_set_dict_key_value_str(environ, "SERVER_PROTOCOL", server_protocol))
    {
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
/**
*/
static
bool
_environ_set_wsgi_vars(PyObject *environ, wsgi_request_t request)
{
    PyObject *version;
    int retval;
    PyObject *py_stderr;

    version = Py_BuildValue("(ii)", 1, 0);
    if (NULL == version)
    {
        return FALSE;
    }
    retval = PyDict_SetItemString(environ, "wsgi.version", version);
    Py_DECREF(version);
    if (0 != retval)
    {
        return FALSE;
    }
    // FIXME: support HTTPS
    if (!_set_dict_key_value_str(environ, "wsgi.url_scheme", "http"))
    {
        return FALSE;
    }
    // FIXME: implement this required feature
    //if (0 != PyDict_SetItemString(environ, "wsgi.input", request->input))
    //{
    //    return FALSE;
    //}
    py_stderr = wsgi_context_get_stderr(request->context);
    if (0 != PyDict_SetItemString(environ, "wsgi.errors", py_stderr))
    {
        return FALSE;
    }
    // FIXME: set this to true when I add multi-threading
    if (0 != PyDict_SetItemString(environ, "wsgi.multithread", Py_False))
    {
        return FALSE;
    }
    if (0 != PyDict_SetItemString(environ, "wsgi.multiprocess", Py_False))
    {
        return FALSE;
    }
    if (0 != PyDict_SetItemString(environ, "wsgi.run_once", Py_False))
    {
        return FALSE;
    }
    // FIXME: implement this optional feature
    //if (0 != PyDict_SetItemString(environ, "wsgi.file_wrapper", (PyObject *)&file_wrapper_type))
    //{
    //    return FALSE;
    //}

    return TRUE;
}

//------------------------------------------------------------------------------
/**
*/
bool
_wsgi_request_fill_environ(wsgi_request_t self)
{
    evhttp_request_t request = wsgi_context_get_request(self->context);
    PyObject *environ = self->environ;
    evkeyvalq_t input_headers = evhttp_request_get_input_headers(request);

    if (!_environ_set_http_headers(environ, input_headers))
    {
        return FALSE;
    }
    if (!_environ_set_cgi_vars(environ, request))
    {
        return FALSE;
    }
    if (!_environ_set_wsgi_vars(environ, self))
    {
        return FALSE;
    }

    return TRUE;
}

static PyMethodDef wsgi_request_methods[] =
{
    {
        "start_response", (PyCFunction)_wsgi_request_start_response, METH_VARARGS,
        "Callable to be invoked by the application, "
        "the first two positional arguments are required."
    },
    { 
        "write", (PyCFunction)_wsgi_request_write, METH_VARARGS,
        "Callable to be invoked by the application, "
        "the one and only positional argument should be a string to be written "
        "to the HTTP response body."
    },
    { NULL } // sentinel
};

PyTypeObject wsgi_request_type = 
{
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "wwm_wsgi.Request",             // tp_name
    sizeof(struct wsgi_request_t_), // tp_basicsize
    0,                         /*tp_itemsize*/
    (destructor)_wsgi_request_dealloc, // tp_dealloc
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,   // tp_flags
    "WSGI Request Class", // tp_doc
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    wsgi_request_methods, // tp_methods
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)_wsgi_request_init, // tp_init
    0,                         /* tp_alloc */
    _wsgi_request_new, // tp_new
};
