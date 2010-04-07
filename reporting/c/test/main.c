#include "wayward/metrics.h"
#include <stdlib.h>

#ifdef WIN32
#define wwm_sleep Sleep
#else // not WIN32
#define wwm_sleep sleep
#endif // WIN32

int
main(int argc, char ** argv)
{
    wwm_reporter_t reporter = NULL;
    int i;

    reporter = wwm_reporter_new();
    // wwm_reporter_log_to_network(reporter, "localhost", 8346);
    wwm_reporter_log_to_file(reporter, "temp.dat");
    wwm_reporter_start_session(reporter, "test_session");

    for (i = 0; i < 1000000; i++)
    {
        wwm_buffer_t buffer = wwm_buffer_new(768);
        buffer = wwm_reporter_populate_base_record_data(reporter, buffer);
        buffer = wwm_codec_push_begin_tuple(buffer, 2);
        buffer = wwm_codec_push_int32(buffer, i);
        buffer = wwm_codec_push_string(buffer, "hello!");
        wwm_reporter_record_data(reporter, buffer);
    }

    wwm_sleep(1);

    wwm_reporter_destroy(reporter);

    return 0;
}

