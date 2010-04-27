#ifndef _WWM_URL_RESOLVER_H_
#define _WWM_URL_RESOLVER_H_

typedef struct wwm_url_resolver_t_ *wwm_url_resolver_t;

wwm_url_resolver_t wwm_url_resolver_new();
void               wwm_url_resolver_destroy(wwm_url_resolver_t resolver);

void wwm_url_resolver_add_handler(wwm_url_resolver_t resolver, const char *url_path, evhttp_callback_t cb, void *cb_data);
void wwm_url_resolver_handle_request(evhttp_request_t request, wwm_url_resolver_t resolver);

#endif // _WWM_URL_RESOLVER_H_
