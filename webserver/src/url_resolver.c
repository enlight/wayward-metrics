#include "config.h"
#include "url_resolver.h"
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "event2/http.h"

typedef struct _wwm_url_handler_t_
{
    char *url_path;
    evhttp_callback_t cb;
    void *cb_data;
    struct _wwm_url_handler_t_ *next;
} *_wwm_url_handler_t;

struct wwm_url_resolver_t_
{
    _wwm_url_handler_t handler_slist; // singly-linked list of handlers
};

//------------------------------------------------------------------------------
/**
*/
static
_wwm_url_handler_t
_wwm_url_handler_new(const char *url_path, evhttp_callback_t cb, void *cb_data)
{
    _wwm_url_handler_t handler = malloc(sizeof(struct _wwm_url_handler_t_));
    handler->url_path = strdup(url_path);
    handler->cb = cb;
    handler->cb_data = cb_data;
    handler->next = NULL;
    return handler;
}

//------------------------------------------------------------------------------
/**
*/
static
void
_wwm_url_handler_destroy(_wwm_url_handler_t handler)
{
    free(handler->url_path);
    free(handler);
}

//------------------------------------------------------------------------------
/**
*/
wwm_url_resolver_t 
wwm_url_resolver_new()
{
    return calloc(1, sizeof(struct wwm_url_resolver_t_));
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_url_resolver_destroy(wwm_url_resolver_t resolver)
{
    _wwm_url_handler_t handler = resolver->handler_slist;
    while (NULL != handler)
    {
        _wwm_url_handler_t temp = handler;
        handler = handler->next;
        _wwm_url_handler_destroy(temp);
    }
    free(resolver);
}

//------------------------------------------------------------------------------
/**
    Adds a handler for the given url path.
    
    Handlers are checked against the request url in the order they are added.
    This means that if you want two different handlers to handle '/media/css' 
    and '/media' then the handler with the longer path should be added before 
    the handler with the shorter path, otherwise the handler for '/media/css' 
    would never get invoked.

    @param url_path The url path, must begin with a '/', can't end in '/'. 
                    regex/glob or anything fancy like that not supported.
    @param cb The callback to be invoked when a request matching the specified
              url path comes in.
    @param cb_data The data to be passed to the callback.
*/
void 
wwm_url_resolver_add_handler(wwm_url_resolver_t resolver, const char *url_path, 
                             evhttp_callback_t cb, void *cb_data)
{
    _wwm_url_handler_t handler = _wwm_url_handler_new(url_path, cb, cb_data);

    if (NULL == resolver->handler_slist)
    {
        resolver->handler_slist = handler;
    }
    else
    {
        _wwm_url_handler_t tail = resolver->handler_slist;
        while (NULL != tail->next)
        {
            tail = tail->next;
        }
        tail->next = handler;
    }
}

//------------------------------------------------------------------------------
/**
    Forwards the request onto one of the registered handlers, 
    depending on the url path.
*/
void 
wwm_url_resolver_handle_request(evhttp_request_t request, wwm_url_resolver_t resolver)
{
    const char *uri = evhttp_request_get_uri(request);
    if (uri)
    {
        char *decoded_uri = evhttp_decode_uri(uri);
        _wwm_url_handler_t handler = resolver->handler_slist;
        while (NULL != handler)
        {
            if (strncmp(handler->url_path, decoded_uri, strlen(handler->url_path)) == 0)
            {
                handler->cb(request, handler->cb_data);
                break;
            }
            handler = handler->next;
        }
        free(decoded_uri);
    }
}
