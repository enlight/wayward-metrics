#include "wayward/metrics/message_queue.h"

#include <pthread.h>
#include <stdlib.h>

#include "wayward/metrics/config.h"

// This uses a concurrent queue based on http://www.cs.rochester.edu/u/michael/PODC96.html
// but without the locks since we have a single reader and a single writer
// thread for each queue.
typedef struct _wwm_per_thread_queue_t_ *_wwm_per_thread_queue_t;

//------------------------------------------------------------------------------
/**
*/
struct wwm_message_queue_t_
{
    wwm_connection_t        connection;
    wwm_file_t              file;
    pthread_key_t           per_thread_queue_key;
    pthread_t               background_thread;
    pthread_attr_t          background_thread_attr;
    _wwm_per_thread_queue_t per_thread_queue_slist;
    wwm_buffer_t            send_buffer;
    volatile bool           shutdown_requested;
};

//------------------------------------------------------------------------------
/**
*/
struct _wwm_per_thread_queue_t_
{
    wwm_message_queue_t             owner;
    wwm_frame_t                     head;
    wwm_frame_t                     tail;
    bool                            thread_exited;
    _wwm_per_thread_queue_t         next;
};

static void                   * _wwm_message_queue_start(void* queue);
static void                     _wwm_message_queue_run(wwm_message_queue_t queue);
static void                     _wwm_message_queue_add_per_thread_queue(wwm_message_queue_t queue, _wwm_per_thread_queue_t per_thread_queue);
static void                     _wwm_message_queue_remove_per_thread_queue(wwm_message_queue_t queue, _wwm_per_thread_queue_t per_thread_queue);

static _wwm_per_thread_queue_t  _wwm_per_thread_queue_new(wwm_message_queue_t owner);
static void                     _wwm_per_thread_queue_destroy(_wwm_per_thread_queue_t);
static void                     _wwm_per_thread_queue_kill(void*);
static void                     _wwm_per_thread_queue_enqueue(_wwm_per_thread_queue_t, wwm_frame_t);
static wwm_frame_t              _wwm_per_thread_queue_dequeue(_wwm_per_thread_queue_t per_thread_queue);

//------------------------------------------------------------------------------
/**
*/
wwm_message_queue_t
wwm_message_queue_new(void)
{
    wwm_message_queue_t queue = (wwm_message_queue_t)calloc(1, sizeof(struct wwm_message_queue_t_));

    queue->connection = NULL;
    queue->file = NULL;
    queue->send_buffer = wwm_buffer_new(5000);

    (void)pthread_key_create(&(queue->per_thread_queue_key), _wwm_per_thread_queue_kill);

    (void)pthread_attr_init(&(queue->background_thread_attr));
    (void)pthread_attr_setdetachstate(&(queue->background_thread_attr), PTHREAD_CREATE_JOINABLE);
    (void)pthread_create(&(queue->background_thread), &(queue->background_thread_attr), _wwm_message_queue_start, (void*)queue);
    return queue;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_message_queue_destroy(wwm_message_queue_t queue)
{
    queue->shutdown_requested = TRUE;
    (void)pthread_join(queue->background_thread, NULL);

    (void)pthread_attr_destroy(&(queue->background_thread_attr));

    (void)pthread_key_delete(queue->per_thread_queue_key);

    if (NULL != queue->connection)
    {
        wwm_connection_destroy(queue->connection);
    }

    if (NULL != queue->file)
    {
        wwm_file_destroy(queue->file);
    }

    wwm_buffer_destroy(queue->send_buffer);

    free(queue);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_message_queue_set_connection(wwm_message_queue_t queue, wwm_connection_t conn)
{
    queue->connection = conn;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_message_queue_set_file(wwm_message_queue_t queue, wwm_file_t file)
{
    queue->file = file;
}

//------------------------------------------------------------------------------
/**
*/
static void*
_wwm_message_queue_start(void* queue)
{
    _wwm_message_queue_run((wwm_message_queue_t)queue);
    return NULL; // Implied pthread_exit(NULL)
}

//------------------------------------------------------------------------------
/**
*/
static void
_wwm_message_queue_run(wwm_message_queue_t queue)
{
    while (FALSE == queue->shutdown_requested)
    {
        _wwm_per_thread_queue_t ptq;
        for (ptq = queue->per_thread_queue_slist; NULL != ptq; ptq = ptq->next)
        {
            int messages_handled = 0;
            wwm_frame_t frame = _wwm_per_thread_queue_dequeue(ptq);
            while (NULL != frame)
            {
                queue->send_buffer = wwm_frame_encode(frame, queue->send_buffer);
                messages_handled++;
                // XXX: Make this high water mark configurable:
                if (wwm_buffer_length(queue->send_buffer) > 50000)
                {
                    if (NULL != queue->connection)
                    {
                        wwm_connection_send_buffer(queue->connection, queue->send_buffer);
                    }
                    if (NULL != queue->file)
                    {
                        wwm_file_send_buffer(queue->file, queue->send_buffer);
                    }
                    wwm_buffer_reset(queue->send_buffer);
                }
                frame = _wwm_per_thread_queue_dequeue(ptq);
            }
            if (messages_handled > 0)
            {
                if (NULL != queue->connection)
                {
                    wwm_connection_send_buffer(queue->connection, queue->send_buffer);
                }
                if (NULL != queue->file)
                {
                    wwm_file_send_buffer(queue->file, queue->send_buffer);
                }
                wwm_buffer_reset(queue->send_buffer);
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_message_queue_enqueue(wwm_message_queue_t queue, wwm_frame_t frame)
{
    _wwm_per_thread_queue_t ptqueue = pthread_getspecific(queue->per_thread_queue_key);
    if (NULL == ptqueue)
    {
        ptqueue = _wwm_per_thread_queue_new(queue);
        pthread_setspecific(queue->per_thread_queue_key, ptqueue);
    }
    _wwm_per_thread_queue_enqueue(ptqueue, frame);
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_message_queue_request_shutdown(wwm_message_queue_t queue)
{
    queue->shutdown_requested = TRUE;
}

//------------------------------------------------------------------------------
/**
*/
static void
_wwm_message_queue_add_per_thread_queue(wwm_message_queue_t queue, _wwm_per_thread_queue_t per_thread_queue)
{
    if (NULL == queue->per_thread_queue_slist)
    {
        queue->per_thread_queue_slist = per_thread_queue;
    }
    else
    {
        _wwm_per_thread_queue_t tail = queue->per_thread_queue_slist;
        while (NULL != tail->next)
        {
            tail = tail->next;
        }
        tail->next = per_thread_queue;
    }
}

//------------------------------------------------------------------------------
/**
*/
static void
_wwm_message_queue_remove_per_thread_queue(wwm_message_queue_t queue, _wwm_per_thread_queue_t per_thread_queue)
{
    if (per_thread_queue == queue->per_thread_queue_slist)
    {
        queue->per_thread_queue_slist = per_thread_queue->next;
    }
    else
    {
        _wwm_per_thread_queue_t ptq;
        for (ptq = queue->per_thread_queue_slist; NULL != ptq->next; ptq = ptq->next)
        {
            if (per_thread_queue == ptq->next)
            {
                ptq->next = per_thread_queue->next;
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
static _wwm_per_thread_queue_t
_wwm_per_thread_queue_new(wwm_message_queue_t owner)
{
    _wwm_per_thread_queue_t ptqueue = (_wwm_per_thread_queue_t)malloc(sizeof(struct _wwm_per_thread_queue_t_));
    ptqueue->owner = owner;
    ptqueue->head = ptqueue->tail = wwm_frame_new();
    ptqueue->thread_exited = FALSE;
    ptqueue->next = NULL;

    _wwm_message_queue_add_per_thread_queue(owner, ptqueue);

    return ptqueue;
}

//------------------------------------------------------------------------------
/**
*/
static void
_wwm_per_thread_queue_destroy(_wwm_per_thread_queue_t per_thread_queue)
{
    _wwm_message_queue_remove_per_thread_queue(per_thread_queue->owner, per_thread_queue);

    // XXX: Destroy any remaining items in the queue?
}

//------------------------------------------------------------------------------
/**
*/
static void
_wwm_per_thread_queue_kill(void* per_thread_queue)
{
    ((_wwm_per_thread_queue_t)per_thread_queue)->thread_exited = TRUE;
}

//------------------------------------------------------------------------------
/**
*/
static void
_wwm_per_thread_queue_enqueue(_wwm_per_thread_queue_t per_thread_queue, wwm_frame_t frame)
{
    __sync_synchronize();
    wwm_frame_set_next(per_thread_queue->tail, frame);
    per_thread_queue->tail = frame;
}

//------------------------------------------------------------------------------
/**
*/
static wwm_frame_t
_wwm_per_thread_queue_dequeue(_wwm_per_thread_queue_t per_thread_queue)
{
    wwm_frame_t old_head = per_thread_queue->head;
    wwm_frame_t next_head = wwm_frame_get_next(old_head);
    if (NULL == next_head)
    {
        // queue was empty.
        return NULL;
    }
    per_thread_queue->head = next_head;
    wwm_frame_destroy(old_head);
    return next_head;
}

