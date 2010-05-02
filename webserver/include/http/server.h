#ifndef _WWM_HTTP_SERVER_H_
#define _WWM_HTTP_SERVER_H_

#include "url_resolver.h"

typedef struct wwm_http_server_t_ *wwm_http_server_t;

wwm_http_server_t wwm_http_server_new(event_base_t base);
void              wwm_http_server_destroy(wwm_http_server_t server);

bool wwm_http_server_listen_on(wwm_http_server_t server, 
                               const char *address, int port);
void wwm_http_server_set_num_aliases(wwm_http_server_t server, int num_aliases);
void wwm_http_server_set_alias(wwm_http_server_t server, int alias_index, 
                               const char *url_path, const char *dir_path);
void wwm_http_server_set_url_resolver(wwm_http_server_t server, 
                                      wwm_url_resolver_t url_resolver);
void wwm_http_server_handle_request(evhttp_request_t request, 
                                    wwm_http_server_t server);

#endif // _WWM_HTTP_SERVER_H_
