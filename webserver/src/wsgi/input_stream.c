#include <Python.h>
#include "config.h"
#include "wsgi//input_stream.h"
#include "wsgi/request.h"
#include "wsgi/context.h"
#include "event2/http.h"
#include "event2/buffer.h"

struct wsgi_input_stream_t_
{
    PyObject_HEAD
    PyObject *request;
    // the total size of the stream in bytes as specified by the client in the
    // Content-Length HTTP header (if any), the actual size may be different
    // (though that would be an error)
    int content_len;
    // contains the body of the http request
    evbuffer_t buffer;
    // number of bytes read from the buffer so far
    int bytes_read;
};

//------------------------------------------------------------------------------
/**
    Creates a new Python input stream associated with the given request.
*/
wsgi_input_stream_t 
wsgi_input_stream_new(wsgi_request_t request)
{
    // pass the request into the input stream constructor
    PyObject *args = Py_BuildValue("(O)", request);
    if (NULL != args)
    {
        wsgi_input_stream_t obj = (wsgi_input_stream_t)PyObject_CallObject(
            (PyObject *)&wsgi_input_stream_type, args
        );
        Py_DECREF(args);
        return obj;
    }
    return NULL;
}

//------------------------------------------------------------------------------
/**
    Clears any references the given input stream holds to other python objects.
*/
static
void
_wsgi_input_stream_clear_refs(wsgi_input_stream_t stream)
{
    Py_CLEAR(stream->request);
}

//------------------------------------------------------------------------------
/**
    Destroys a Python input stream object created by wsgi_input_stream_new().
*/
void
wsgi_input_stream_destroy(wsgi_input_stream_t stream)
{
    _wsgi_input_stream_clear_refs(stream);
    Py_DECREF(stream);
}

//------------------------------------------------------------------------------
/**
    Allocates memory for an input stream object.
*/
static
PyObject *
_wsgi_input_stream_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    wsgi_input_stream_t self = (wsgi_input_stream_t)type->tp_alloc(type, 0);
    if (NULL != self)
    {
        self->request = NULL;
        self->content_len = 0;
        self->buffer = NULL;
        self->bytes_read = 0;
    }
    return (PyObject *)self;
}

//------------------------------------------------------------------------------
/**
    Initializes an input stream object.
*/
static
int
_wsgi_input_stream_init(wsgi_input_stream_t self, PyObject *args, PyObject *kwds)
{
    PyObject *wsgi_request;
    evhttp_request_t http_request;
    evkeyvalq_t headers;
    const char *header_val;

    // the input stream constructor takes a request object
    if (!PyArg_ParseTuple(args, "O!", &wsgi_request_type, &wsgi_request))
    {
        return -1;
    }

    http_request = wsgi_request_get_http_request((wsgi_request_t)wsgi_request);
    if (NULL == http_request)
    {
        PyErr_SetString(
            PyExc_AssertionError, 
            "WSGI request is not associated with an HTTP request."
        );
        return -1;
    }

    headers = evhttp_request_get_input_headers(http_request);
    header_val = evhttp_find_header(headers, "Content-Length");
    if (NULL != header_val)
    {
        self->content_len = atoi(header_val);
    }

    self->buffer = evhttp_request_get_input_buffer(http_request);

    Py_INCREF(wsgi_request);
    self->request = wsgi_request;

    return 0;
}

//------------------------------------------------------------------------------
/**
*/
static
void
_wsgi_input_stream_dealloc(wsgi_input_stream_t self)
{
    _wsgi_input_stream_clear_refs(self);
    self->ob_type->tp_free((PyObject *)self);
}

//------------------------------------------------------------------------------
/**
*/
static
PyObject *
_wsgi_input_stream_advance(wsgi_input_stream_t self, int bytes_to_read)
{
    if (0 == bytes_to_read)
    {
        return PyString_FromString("");
    }
    else
    {
        PyObject *str = PyString_FromStringAndSize(NULL, bytes_to_read + 1);
        if (NULL != str)
        {
            char *data = PyString_AS_STRING(str);
            int bytes_read = evbuffer_remove(self->buffer, data, bytes_to_read);
            if (-1 == bytes_read)
            {
                Py_DECREF(str);
                PyErr_SetString(
                    PyExc_AssertionError, "Failed to advance input stream."
                );
                return NULL;
            }
            // presumably Python expects the C-string to be NULL terminated
            data[bytes_to_read] = 0;
            self->bytes_read += bytes_read;
        }
        return str;
    }
}

//------------------------------------------------------------------------------
/**
    Implements InputStream.read([size]).
*/
static
PyObject *
_wsgi_input_stream_read(wsgi_input_stream_t self, PyObject *args)
{
    int size = -1;
    size_t buffer_size;

    // the size argument is optional
    if (!PyArg_ParseTuple(args, "|i:read", &size))
    {
        return NULL;
    }

    buffer_size = evbuffer_get_length(self->buffer);
    if ((size < 0) || (size > buffer_size))
    {
        size = buffer_size;
    }

    // The WSGI server is not required to read past the client specified
    // Content-Length, and the WSGI application SHOULD NOT attempt to read
    // more data than specified by the client.
    if ((self->bytes_read + size) > self->content_len)
    {
        size = self->content_len - self->bytes_read;
    }

    return _wsgi_input_stream_advance(self, size);
}

//------------------------------------------------------------------------------
/**
    Implements InputStream.readline([size]).
*/
static
PyObject *
_wsgi_input_stream_readline(wsgi_input_stream_t self, PyObject *args)
{
    // the optional size argument is not supported, though it may be provided
    int size = -1;
    size_t eol_len = 0;
    evbuffer_ptr_t ptr = evbuffer_search_eol(self->buffer, NULL, &eol_len, EVBUFFER_EOL_CRLF);
    if (ptr.pos < 0) // no EOL was found in the buffer
    {
        size = self->content_len - self->bytes_read;
    }
    else
    {
        size = ptr.pos + eol_len; // include the EOL in the resulting string
    }
    return _wsgi_input_stream_advance(self, size);
}

//------------------------------------------------------------------------------
/**
    Implements InputStream.readlines([sizehint]).
*/
static
PyObject *
_wsgi_input_stream_readlines(wsgi_input_stream_t self, PyObject *args)
{
    int size_hint = -1;
    PyObject *line;

    // the sizehint argument is optional
    if (!PyArg_ParseTuple(args, "|i:readlines", &size_hint))
    {
        return NULL;
    }

    line = _wsgi_input_stream_readline(self, NULL);
    if (NULL != line)
    {
        Py_ssize_t line_len = PyString_GET_SIZE(line);
        PyObject *line_list = PyList_New(0);
        
        if (NULL == line_list)
        {
            Py_DECREF(line);
            return NULL;
        }

        while (line_len > 0)
        {
            if (0 == PyList_Append(line_list, line))
            {
                if (self->bytes_read >= size_hint)
                {
                    break;
                }
            }
            else // error
            {
                Py_DECREF(line);
                Py_DECREF(line_list);
                return NULL;
            }

            Py_DECREF(line);
            line = _wsgi_input_stream_readline(self, NULL);
            if (NULL == line)
            {
                Py_DECREF(line_list);
                return NULL;
            }
            line_len = PyString_GET_SIZE(line);
        }

        Py_DECREF(line);
        return line_list;
    }

    return NULL;
}

//------------------------------------------------------------------------------
/**
*/
static PyMethodDef wsgi_input_stream_methods[] =
{
    {
        "read", (PyCFunction)_wsgi_input_stream_read, METH_VARARGS,
        "Read at most size bytes from the stream. "
        "If size is negative or omitted, read all data in the stream."
    },
    { 
        "readline", (PyCFunction)_wsgi_input_stream_readline, METH_VARARGS,
        "Read one line from the stream."
    },
    {
        "readlines", (PyCFunction)_wsgi_input_stream_readlines, METH_VARARGS,
        "Read the entire stream using readline() and return a list of lines read."
    },
    { NULL } // sentinel
};

//------------------------------------------------------------------------------
/**
*/
PyTypeObject wsgi_input_stream_type = 
{
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "wwm_wsgi.InputStream",              // tp_name
    sizeof(struct wsgi_input_stream_t_), // tp_basicsize
    0,                         /*tp_itemsize*/
    (destructor)_wsgi_input_stream_dealloc, // tp_dealloc
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
    "WSGI Input Stream Class", // tp_doc
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    wsgi_input_stream_methods, // tp_methods
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)_wsgi_input_stream_init, // tp_init
    0,                         /* tp_alloc */
    _wsgi_input_stream_new, // tp_new
};
