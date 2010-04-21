#include <Python.h>
#include "config.h"
#include "wsgi/request.h"
#include "wsgi/context.h"
#include "wsgi/input_stream.h"
#include "sys/queue.h" // for TAILQ_FOREACH (must be included before the _struct.h headers)
#include "event2/util.h"
#include "event2/event_struct.h"
#include "event2/http.h"
#include "event2/http_struct.h"
#include "event2/buffer.h"

struct wsgi_request_t_
{
    PyObject_HEAD
    wsgi_context_t context;
    PyObject *environ;
    wsgi_input_stream_t input; // wsgi.input
    PyObject *response_status;
    bool response_headers_packed;
    PyObject *response_headers;
    PyObject *app_result; // returned by the WSGI application
};

static bool _wsgi_request_fill_environ(wsgi_request_t self);

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
    Py_CLEAR(request->response_status);
    Py_CLEAR(request->input);
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
    Get the underlying http request associated with this wsgi request.
*/
evhttp_request_t 
wsgi_request_get_http_request(wsgi_request_t request)
{
    return request->context ? wsgi_context_get_request(request->context) : NULL;
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
_wsgi_pack_headers(wsgi_request_t request, int *content_len)
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
        if (evutil_ascii_strcasecmp(header, "Content-Length") == 0)
        {
            *content_len = atoi(value);
        }
    }

    return TRUE;
}

//------------------------------------------------------------------------------
/**
*/
static
bool
_wsgi_request_get_response_status(wsgi_request_t self, 
                                  int *status_code, const char **status_reason)
{
    const char *status = PyString_AsString(self->response_status);
    if ((NULL == status) || (strlen(status) < 4))
    {
        return FALSE;
    }
    // According to the WSGI spec the status string provided by the 
    // WSGI application should not have any surrounding whitespace,
    // and should only consist of a status code and a reason phrase
    // separated by a single space.
    *status_code = atoi(status);
    if (0 == status_code)
    {
        return FALSE;
    }
    *status_reason = &status[4]; // status code should always be 3 digits
    return TRUE;
}

//------------------------------------------------------------------------------
/**
*/
static
bool
_wsgi_request_send_body(wsgi_request_t self, const char *body, size_t body_size)
{
    int status_code;
    const char *status_reason;
    evhttp_request_t request;
    evbuffer_t buffer = NULL;

    if (!_wsgi_request_get_response_status(self, &status_code, &status_reason))
    {
        return FALSE;
    }

    request = wsgi_context_get_request(self->context);
    if (body_size > 0)
    {
        buffer = evbuffer_new();
        evbuffer_add(buffer, body, body_size);
        evhttp_send_reply(request, status_code, status_reason, buffer);
        evbuffer_free(buffer);
    }
    else // just send the HTTP headers
    {
        evhttp_send_reply(request, status_code, status_reason, NULL);
    }

    return TRUE;
}

//------------------------------------------------------------------------------
/**
*/
static
bool
_wsgi_request_send_chunked_body_start(wsgi_request_t self)
{
    int status_code;
    const char *status_reason;

    if (_wsgi_request_get_response_status(self, &status_code, &status_reason))
    {
        evhttp_send_reply_start(
            wsgi_context_get_request(self->context), 
            status_code, status_reason
        );
        return TRUE;
    }

    return FALSE;
}

//------------------------------------------------------------------------------
/**
*/
static
void
_wsgi_request_send_body_chunk(wsgi_request_t self, 
                              const char *chunk, size_t chunk_size)
{
    evbuffer_t buffer = evbuffer_new();
    evbuffer_add(buffer, chunk, chunk_size);
    evhttp_send_reply_chunk(wsgi_context_get_request(self->context), buffer);
    evbuffer_free(buffer);
}

//------------------------------------------------------------------------------
/**
*/
static
void
_wsgi_request_send_chunked_body_end(wsgi_request_t self)
{
    evhttp_send_reply_end(wsgi_context_get_request(self->context));
}

//------------------------------------------------------------------------------
/**
*/
static
bool 
_wsgi_request_send_response(wsgi_request_t request)
{
    Py_ssize_t num_items;
    PyObject *it;
    PyObject *data;
    int content_len = -1;
    bool chunked = TRUE;

    // If the WSGI application did not provide a Content-Length HTTP header
    // then libevent will set it to match the size of the data sent out. 
    //
    // If the WSGI application did provide a Content-Length HTTP header but 
    // it doesn't match the size of the data sent out then the client will end
    // up somewhat confused, perhaps we should detect it and report it as an
    // error.
    
    num_items = PySequence_Size(request->app_result);
    // If PySequence_Size() failed we don't want to propagate the error.
    PyErr_Clear();

    it = PyObject_GetIter(request->app_result);
    if (NULL == it)
    {
        return FALSE;
    }

    while (data = PyIter_Next(it))
    {
        Py_ssize_t body_size = PyString_Size(data);
        if (PyErr_Occurred())
        {
            Py_DECREF(data);
            break;
        }

        if (body_size > 0)
        {
            const char *body = PyString_AsString(data);
            if (NULL == body)
            {
                Py_DECREF(data);
                break;
            }

            if (!request->response_headers_packed)
            {
                if (!_wsgi_pack_headers(request, &content_len))
                {
                    Py_DECREF(data);
                    break;
                }

                if (((content_len < 0) && (1 == num_items)) || (content_len == body_size))
                {
                    chunked = FALSE;
                }

                if (chunked && !_wsgi_request_send_chunked_body_start(request))
                {
                    Py_DECREF(data);
                    break;
                }

                request->response_headers_packed = TRUE;
            }

            if (chunked)
            {
                _wsgi_request_send_body_chunk(request, body, body_size);
            }
            else if (!_wsgi_request_send_body(request, body, body_size))
            {
                Py_DECREF(data);
                break;
            }
        }
        Py_DECREF(data);
    }

    if (chunked && request->response_headers_packed)
    {
        _wsgi_request_send_chunked_body_end(request);
    }

    Py_DECREF(it);

    if (PyErr_Occurred())
    {
        return FALSE;
    }

    // if there was no data to send still send the headers
    if (!request->response_headers_packed)
    {
        if (!_wsgi_pack_headers(request, &content_len))
        {
            return FALSE;
        }
        request->response_headers_packed = TRUE;
        _wsgi_request_send_body(request, NULL, 0);
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
    _wsgi_request_send_response(request);
    _wsgi_result_close(app_result);
}

//------------------------------------------------------------------------------
/**
    Allocates memory for a request object.
*/
static
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

        self->input = NULL;
        self->context = NULL;
        self->response_status = NULL;
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
static
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
    self->input = wsgi_input_stream_new(self);
    
    if (!_wsgi_request_fill_environ(self))
    {
        return -1;
    }

    return 0;
}

//------------------------------------------------------------------------------
/**
*/
static
void
_wsgi_request_dealloc(wsgi_request_t self)
{
    _wsgi_request_clear_refs(self);
    self->ob_type->tp_free((PyObject *)self);
}

//------------------------------------------------------------------------------
/**
    This will be called by the WSGI application.
*/
static
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
    tmp = self->response_status;
    Py_INCREF(status);
    self->response_status = status;
    Py_XDECREF(tmp);

    tmp = self->response_headers;
    Py_INCREF(response_headers);
    self->response_headers = response_headers;
    Py_XDECREF(tmp);

    return PyObject_GetAttrString((PyObject *)self, "write");
}

//------------------------------------------------------------------------------
/**
*/
static
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
    if (0 != PyDict_SetItemString(environ, "wsgi.input", (PyObject *)request->input))
    {
        return FALSE;
    }
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
static
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

//------------------------------------------------------------------------------
/**
*/
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

//------------------------------------------------------------------------------
/**
*/
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
