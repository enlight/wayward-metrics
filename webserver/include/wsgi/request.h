#ifndef _WWM_WSGI_REQUEST_H_
#define _WWM_WSGI_REQUEST_H_

#include "context.h"

typedef struct wsgi_request_t_ * wsgi_request_t;

extern PyTypeObject wsgi_request_type;

wsgi_request_t   wsgi_request_new(wsgi_context_t context);
void             wsgi_request_destroy(wsgi_request_t request);
evhttp_request_t wsgi_request_get_http_request(wsgi_request_t request);
PyObject *       wsgi_request_invoke_app(wsgi_request_t request, PyObject *app);
void             wsgi_request_send_response(wsgi_request_t, PyObject *app_result);

#endif // _WWM_WSGI_REQUEST_H_
