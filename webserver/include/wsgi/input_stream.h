#ifndef _WWM_WSGI_INPUT_STREAM_H_
#define _WWM_WSGI_INPUT_STREAM_H_

#include "request.h"

typedef struct wsgi_input_stream_t_ * wsgi_input_stream_t;

extern PyTypeObject wsgi_input_stream_type;

wsgi_input_stream_t wsgi_input_stream_new(wsgi_request_t request);
void                wsgi_input_stream_destroy(wsgi_input_stream_t stream);

#endif // _WWM_WSGI_INPUT_STREAM_H_
