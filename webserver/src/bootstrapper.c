#include "config.h"
#include "bootstrapper.h"
#include "jansson.h"
#include "http/server.h"
#include "wsgi/server.h"
#include "url_resolver.h"
#include "event2/event.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>

struct wwm_bootstrapper_t_
{
    char server_full_path[256]; // path to server executable (including filename)
    char config_full_path[256]; // path to server config file (including filename)
    event_base_t base;
    wwm_http_server_t http;
    wwm_url_resolver_t url_resolver;
    wsgi_server_t wsgi;
};

typedef struct _wwm_bootstrapper_config_listen_t_
{
    char *address;
    int port;
} *_wwm_bootstrapper_config_listen_t;

typedef struct _wwm_bootstrapper_config_location_t_
{
    char *url_path;
    char *handler;
} *_wwm_bootstrapper_config_location_t;

typedef struct _wwm_bootstrapper_config_alias_t_
{
    char *url_path;
    char *dir_path;
} *_wwm_bootstrapper_config_alias_t;

typedef struct _wwm_bootstrapper_config_t_
{
    json_t *root;
    _wwm_bootstrapper_config_listen_t listen_points;
    int num_listen_points;
    _wwm_bootstrapper_config_location_t locations;
    int num_locations;
    _wwm_bootstrapper_config_alias_t aliases;
    int num_aliases;
} *_wwm_bootstrapper_config_t;

//------------------------------------------------------------------------------
/**
*/
void 
ws_libevent_log(int severity, const char *msg)
{
    OutputDebugString(msg);
}

//------------------------------------------------------------------------------
/**
*/
wwm_bootstrapper_t 
wwm_boostrapper_new()
{
    return calloc(1, sizeof(struct wwm_bootstrapper_t_));
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_bootstrapper_destroy(wwm_bootstrapper_t bs)
{
    wwm_url_resolver_destroy(bs->url_resolver);
    wsgi_server_destroy(bs->wsgi);
    wwm_http_server_destroy(bs->http);
    event_base_free(bs->base);
    free(bs);
}

//------------------------------------------------------------------------------
/**
*/
bool 
wwm_bootstrapper_parse_cmdline(wwm_bootstrapper_t bs, int argc, char **argv)
{
    char *cur_char;
    const char *config_name = "webserver.conf";
    size_t server_full_path_len = strlen(argv[0]);

    if (server_full_path_len < sizeof(bs->server_full_path))
    {
        strcpy(bs->server_full_path, argv[0]);
    }
    else
    {
        return FALSE;
    }
    
    cur_char = bs->server_full_path + server_full_path_len;
    while (('\\' != *cur_char) && ('/' != *cur_char) && (bs->server_full_path != cur_char))
    {
        --cur_char;
    }
    if (('\\' == *cur_char) || ('/' == *cur_char))
    {
        ++cur_char;
    }
    strncpy(bs->config_full_path, bs->server_full_path, cur_char - bs->server_full_path);
    if ((strlen(bs->config_full_path) + strlen(config_name)) < sizeof(bs->config_full_path))
    {
        strcat(bs->config_full_path, config_name);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
/**
*/
static
json_t *
_read_config_file(const char *file_path)
{
    json_t *root = NULL;
    int fd = _open(file_path, _O_TEXT | _O_RDONLY);
    if (-1 != fd)
    {
        long file_size = _lseek(fd, 0, SEEK_END);
        char *file_data = (char *)malloc(file_size + 1);
        if (NULL != file_data)
        {
            int bytes_read;
            _lseek(fd, 0, SEEK_SET);
            bytes_read = _read(fd, file_data, file_size);
            if (bytes_read > 0)
            {
                json_error_t err;
                char *cur_char = file_data;
                file_data[bytes_read] = 0;

                // convert comments to whitespace so that the json parser
                // doesn't choke
                while (TRUE)
                {
                    // ignore whitespace at the start of a line
                    while (('\t' == *cur_char) || (' ' == *cur_char) || ('\n' == *cur_char))
                    {
                        ++cur_char;
                    }
                    
                    if (0 == *cur_char)
                    {
                        break;
                    }
                    else if ('#' == *cur_char)
                    {
                        // convert entire comment line to whitespace and move
                        // to the next line
                        do
                        {
                            *cur_char = ' ';
                            ++cur_char;
                        } while (('\n' != *cur_char) && (0 != *cur_char));
                    }
                    else
                    {
                        // move to the next line
                        do
                        {
                            ++cur_char;
                        } while (('\n' != *cur_char) && (0 != *cur_char));
                    }
                }

                root = json_loads(file_data, &err);
                if (!root)
                {
                    fprintf(stderr, "error: on line %d: %s\n", err.line, err.text);
                }
            }
            
            free(file_data);
        }
        _close(fd);
    }
    return root;
}

//------------------------------------------------------------------------------
/**
*/
static
_wwm_bootstrapper_config_t
_wwm_bootstrapper_config_new()
{
    return calloc(1, sizeof(struct _wwm_bootstrapper_config_t_));
}

//------------------------------------------------------------------------------
/**
*/
static
void
_wwm_bootstrapper_config_destroy(_wwm_bootstrapper_config_t conf)
{
    if (conf->num_listen_points > 0)
    {
        free(conf->listen_points);
    }
    if (conf->num_locations > 0)
    {
        free(conf->locations);
    }
    if (conf->num_aliases > 0)
    {
        free(conf->aliases);
    }
    json_decref(conf->root);
    free(conf);
}

//------------------------------------------------------------------------------
/**
*/
static
bool
_wwm_bootstrapper_config_load(_wwm_bootstrapper_config_t conf, const char *file_path)
{
    conf->root = _read_config_file(file_path);
    if (NULL != conf->root)
    {
        bool config_valid = TRUE;

        json_t *section = json_object_get(conf->root, "Listen");
        if (NULL != section)
        {
            int i;

            conf->num_listen_points = json_array_size(section);
            if (conf->num_listen_points > 0)
            {
                conf->listen_points = calloc(
                    conf->num_listen_points, 
                    sizeof(struct _wwm_bootstrapper_config_listen_t_)
                );
            }
            else
            {
                config_valid = FALSE;
                // FIXME: log error
            }

            for (i = 0 ; i < conf->num_listen_points; i++)
            {
                json_t *entry = json_array_get(section, i);
                if (!json_is_string(json_array_get(entry, 0)))
                {
                    config_valid = FALSE;
                    // FIXME: log error
                }
                else if (!json_is_integer(json_array_get(entry, 1)))
                {
                    config_valid = FALSE;
                    // FIXME: log error
                }
                else
                {
                    conf->listen_points[i].address = json_string_value(json_array_get(entry, 0));
                    conf->listen_points[i].port = json_integer_value(json_array_get(entry, 1));
                }
            }
        }
        else
        {
            config_valid = FALSE;
            // FIXME: log error
        }
        
        section = json_object_get(conf->root, "Location");
        if (NULL != section)
        {
            int i;

            conf->num_locations = json_array_size(section);
            if (conf->num_locations > 0)
            {
                conf->locations = calloc(
                    conf->num_locations, 
                    sizeof(struct _wwm_bootstrapper_config_location_t_)
                );
            }
            
            for (i = 0; i < conf->num_locations; i++)
            {
                json_t *entry = json_array_get(section, i);
                if (!json_is_string(json_array_get(entry, 0)))
                {
                    config_valid = FALSE;
                    // FIXME: log error
                }
                else if (!json_is_string(json_array_get(entry, 1)))
                {
                    config_valid = FALSE;
                    // FIXME: log error
                }
                else
                {
                    conf->locations[i].url_path = json_string_value(json_array_get(entry, 0));
                    conf->locations[i].handler = json_string_value(json_array_get(entry, 1));
                }
            }
        }

        section = json_object_get(conf->root, "Alias");
        if (NULL != section)
        {
            int i;

            conf->num_aliases = json_array_size(section);
            if (conf->num_aliases > 0)
            {
                conf->aliases = calloc(
                    conf->num_aliases, 
                    sizeof(struct _wwm_bootstrapper_config_alias_t_)
                );
            }

            for (i = 0; i < conf->num_aliases; i++)
            {
                json_t *entry = json_array_get(section, i);
                if (!json_is_string(json_array_get(entry, 0)))
                {
                    config_valid = FALSE;
                    // FIXME: log error
                }
                else if (!json_is_string(json_array_get(entry, 1)))
                {
                    config_valid = FALSE;
                    // FIXME: log error
                }
                else
                {
                    conf->aliases[i].url_path = strdup(json_string_value(json_array_get(entry, 0)));
                    conf->aliases[i].dir_path = strdup(json_string_value(json_array_get(entry, 1)));
                }
            }
        }
        return config_valid;
    }
    return FALSE;
}

//------------------------------------------------------------------------------
/**
*/
bool 
wwm_bootstrapper_configure(wwm_bootstrapper_t bs)
{
    _wwm_bootstrapper_config_t conf = _wwm_bootstrapper_config_new();
    bool config_valid = _wwm_bootstrapper_config_load(conf, bs->config_full_path);
    if (config_valid)
    {
        int i;
        
        event_set_log_callback(ws_libevent_log);

        bs->base = event_base_new();
        bs->http = wwm_http_server_new(bs->base);
        bs->url_resolver = wwm_url_resolver_new();

        for (i = 0; i < conf->num_listen_points; i++)
        {
            if (!wwm_http_server_listen_on(bs->http,
                                           conf->listen_points[i].address, 
                                           conf->listen_points[i].port))
            {
                // FIXME: log error
                config_valid = FALSE;
            }
        }

        for (i = 0; i < conf->num_locations; i++)
        {
            if (stricmp(conf->locations[i].handler, "http") == 0)
            {
                wwm_url_resolver_add_handler(
                    bs->url_resolver, conf->locations[i].url_path, 
                    (evhttp_callback_t)wwm_http_server_handle_request, bs->http
                );
            }
            else if (stricmp(conf->locations[i].handler, "wsgi") == 0)
            {
                if (!bs->wsgi)
                {
                    bs->wsgi = wsgi_server_new();
                    config_valid = wsgi_server_init_python(
                        bs->wsgi, bs->server_full_path, 
                        "run_server", "application"
                    );
                }
                wwm_url_resolver_add_handler(bs->url_resolver, 
                    conf->locations[i].url_path, 
                    (evhttp_callback_t)wsgi_server_handle_request, bs->wsgi
                );
            }
            else
            {
                // FIXME: log error
                config_valid = FALSE;
            }
        }

        wwm_http_server_set_url_resolver(bs->http, bs->url_resolver);

        wwm_http_server_set_num_aliases(bs->http, conf->num_aliases);
        for (i = 0; i < conf->num_aliases; i++)
        {
            wwm_http_server_set_alias(
                bs->http, i, conf->aliases[i].url_path, conf->aliases[i].dir_path
            );
        }
    }
    _wwm_bootstrapper_config_destroy(conf);
    return config_valid;
}

//------------------------------------------------------------------------------
/**
*/
void 
wwm_bootstrapper_run(wwm_bootstrapper_t bs)
{
    event_base_dispatch(bs->base); // loop
}
