#ifndef _WWM_WSGI_SERVER_H_
#define _WWM_WSGI_SERVER_H_

typedef struct wsgi_server_t_ * wsgi_server_t;

wsgi_server_t wsgi_server_new(void);
bool          wsgi_server_init_python(wsgi_server_t server, 
                                      char *exe_name,
                                      const char *mod_name, 
                                      const char *app_name);
void          wsgi_server_destroy(wsgi_server_t server);
void          wsgi_server_handle_request(evhttp_request_t request, wsgi_server_t server);

#endif // _WWM_WSGI_SERVER_H_
