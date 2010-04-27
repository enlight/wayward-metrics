#include "config.h"
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "http/server.h"
#include "mime_types.h"
#include "event2/http.h"
#include "event2/buffer.h"

//------------------------------------------------------------------------------
/**
    Converts the given time stamp (in local time) to a string that conforms to 
    the RFC 1123 time format as specified by RFC 2616 section 3.3.1.

    e.g. Sun, 06 Nov 1994 08:49:37 GMT

    @param buffer The buffer in which the string should be stored, exactly 30 
                  characters (including the null-terminator) will be stored if 
                  this function succeeds.
    @param buffer_size Size of the buffer in which the string representation
                       should be stored, this must be large enough to store at 
                       least 30 characters (including the null-terminator).
    @return TRUE on success, FALSE on failure.
*/
static
bool
_rfc1123_format_date_time(time_t time_stamp, char *buffer, size_t buffer_size)
{
    // Can't use strftime() because that takes the current locale into account,
    // but according to the RFC 2822 Date and Time Specification - day and 
    // month abbreviations must be in English.
    static const char *days[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
    static const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    struct tm *gmt = gmtime(&time_stamp);
    int retval = evutil_snprintf(
        buffer, buffer_size, 
        "%s, " // 1
        "%02d %s %04d " // 2
        "%02d:%02d:%02d GMT", // 3
        days[gmt->tm_wday], // 1
        gmt->tm_mday, months[gmt->tm_mon], (gmt->tm_year + 1900), // 2
        gmt->tm_hour, gmt->tm_min, gmt->tm_sec // 3
    );
    return (retval >= 0);
}

//------------------------------------------------------------------------------
/**
    @brief Checks if the given time of modification differs from that provided
           by the client.
    @return TRUE if the time of modification has changed and thus the server
            should resend the file to the client. 
            FALSE if the file previously sent to the client is still current.
*/
static
bool
_wwm_http_server_check_file_modified(evhttp_request_t request, 
                                     const char *last_modified)
{
    evkeyvalq_t headers = evhttp_request_get_input_headers(request);
    const char *modified_since = evhttp_find_header(headers, "If-Modified-Since");
    if (NULL != modified_since)
    {
        return (strcmp(modified_since, last_modified) != 0);
    }
    return TRUE;
}

//------------------------------------------------------------------------------
/**
*/
static
void
_wwm_http_server_set_output_headers_for_file(evhttp_request_t request, 
                                             const char *file_path, 
                                             size_t file_size,
                                             const char *last_modified)
{
    evkeyvalq_t output_headers;
    char size[12];
    const char *mime_type;

    output_headers = evhttp_request_get_output_headers(request);
    evhttp_add_header(output_headers, "Last-Modified", last_modified);
    evutil_snprintf(size, sizeof(size), "%ld", (long)file_size);
    evhttp_add_header(output_headers, "Content-Length", size);
    mime_type = wwm_mime_types_guess(file_path);
    if (NULL != mime_type)
    {
        evhttp_add_header(output_headers, "Content-Type", mime_type);
    }
}

//------------------------------------------------------------------------------
/**
*/
static
void
_wwm_http_server_send_file(evhttp_request_t request, const char *file_path)
{
    int fd = _open(file_path, _O_BINARY|_O_RDONLY);
    if (fd < 0)
    {
        evhttp_send_reply(request, HTTP_NOTFOUND, "Not Found", NULL);    
    }
    else
    {
        struct __stat64 file_stat;
        if (_fstat64(fd, &file_stat) < 0)
        {
            _close(fd);
            evhttp_send_reply(request, 500, "Failed to obtain file size.", NULL);
        }
        else
        {
            char last_modified[30];
            _rfc1123_format_date_time(
                file_stat.st_mtime, last_modified, sizeof(last_modified)
            );
            if (_wwm_http_server_check_file_modified(request, last_modified))
            {
                // FIXME: Ugh, don't like defines in the middle of functions,
                // need to write one function for Windows and another for Unix.
#ifdef WIN32
                char chunk[4096];
#endif // WIN32
                size_t bytes_left = file_stat.st_size;
                evbuffer_t buffer = evbuffer_new();
                if (buffer)
                {
                    _wwm_http_server_set_output_headers_for_file(
                        request, file_path, file_stat.st_size, last_modified
                    );
#ifndef WIN32
                    evbuffer_add_file(buffer, fd, 0, file_stat.st_size);
#else
                    // FIXME: large files should be sent in chunks!
                    while (bytes_left > 0)
                    {
                        int bytes_read = _read(fd, chunk, sizeof(chunk));
                        if (bytes_read > 0)
                        {
                            evbuffer_add(buffer, chunk, bytes_read);
                            bytes_left -= bytes_read;
                        }
                        else // FIXME: better error handling!
                        {
                            break;
                        }
                    }
#endif // WIN32
                    evhttp_send_reply(request, HTTP_OK, "OK", buffer);
                    free(buffer);
                }
                else
                {
                    evhttp_send_reply(
                        request, HTTP_SERVUNAVAIL, 
                        "The server is currently unable to handle the request. "
                        "Try again later.", NULL
                    );
                }
            }
            else // file hasn't been modified, no need to resend
            {
                evhttp_send_reply(
                    request, HTTP_NOTMODIFIED, "Not Modified", NULL
                );
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_http_server_handle_request(evhttp_request_t request, evhttp_t server)
{
    const char *uri = evhttp_request_get_uri(request);
    if (uri)
    {
        char *decoded_uri = evhttp_decode_uri(uri);
        // FIXME: make this configurable
        const char *alias_base_path = "D:/Dev/django/django/contrib/admin/media";
        size_t alias_len = strlen("/media");
        if (strncmp("/media", decoded_uri, alias_len) == 0)
        {
            const char *alias_rel_path = decoded_uri + alias_len;
            char *file_path = (char *)malloc(strlen(alias_base_path) + strlen(alias_rel_path) + 1);
            strcpy(file_path, alias_base_path);
            strcat(file_path, alias_rel_path);
            _wwm_http_server_send_file(request, file_path);
            free(file_path);
        }
        free(decoded_uri);
    }
}
