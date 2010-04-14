#ifndef _WWM_WSGI_CONTEXT_H
#define _WWM_WSGI_CONTEXT_H

typedef struct wsgi_context_t_ * wsgi_context_t;

wsgi_context_t wsgi_context_new(void);
void           wsgi_context_destroy(wsgi_context_t context);

void       wsgi_context_set_stderr(wsgi_context_t context, PyObject *py_stderr);
PyObject * wsgi_context_get_stderr(wsgi_context_t context);

void             wsgi_context_set_request(wsgi_context_t context, evhttp_request_t request);
evhttp_request_t wsgi_context_get_request(wsgi_context_t context);

void wsgi_context_invoke_app(wsgi_context_t context, PyObject *app);

#endif // _WWM_WSGI_CONTEXT_H
