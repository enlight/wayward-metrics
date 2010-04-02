#include "wayward/metrics/message_queue.h"
#include "wayward/metrics/allocator.h"
#include "wayward/metrics/config.h"
#include "wayward/metrics/thread.h"

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
    wwm_thread_key_t        per_thread_queue_key;
    wwm_thread_t            background_thread;
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
    wwm_buffer_t                    head;
    wwm_buffer_t                    tail;
    _wwm_per_thread_queue_t         next;
};

static WWM_THREADPROC_RETTYPE WWM_THREADPROC _wwm_message_queue_start(void* queue);
static void                     _wwm_message_queue_run(wwm_message_queue_t queue);
static void                     _wwm_message_queue_add_per_thread_queue(wwm_message_queue_t queue, _wwm_per_thread_queue_t per_thread_queue);
static void                     _wwm_message_queue_remove_per_thread_queue(wwm_message_queue_t queue, _wwm_per_thread_queue_t per_thread_queue);

static _wwm_per_thread_queue_t  _wwm_per_thread_queue_new(wwm_message_queue_t owner);
static void                     _wwm_per_thread_queue_destroy(_wwm_per_thread_queue_t per_thread_queue);
static void                     _wwm_per_thread_queue_enqueue(_wwm_per_thread_queue_t per_thread_queue, wwm_buffer_t buffer);
static wwm_buffer_t             _wwm_per_thread_queue_dequeue(_wwm_per_thread_queue_t per_thread_queue);

//------------------------------------------------------------------------------
/**
*/
wwm_message_queue_t
wwm_message_queue_new(void)
{
    wwm_message_queue_t queue = (wwm_message_queue_t)g_wwm_allocator.calloc(1, sizeof(struct wwm_message_queue_t_));

    queue->connection = NULL;
    queue->file = NULL;
    queue->send_buffer = wwm_buffer_new(5000);

    (void)wwm_thread_key_create(&(queue->per_thread_queue_key));

    queue->background_thread = wwm_thread_new();
    (void)wwm_thread_start(queue->background_thread, _wwm_message_queue_start, (void*)queue);
    return queue;
}

//------------------------------------------------------------------------------
/**
*/
void
wwm_message_queue_destroy(wwm_message_queue_t queue)
{
    queue->shutdown_requested = TRUE;
    (void)wwm_thread_join(queue->background_thread, NULL);

    wwm_thread_destroy(queue->background_thread);

    (void)wwm_thread_key_delete(queue->per_thread_queue_key);

    if (NULL != queue->connection)
    {
        wwm_connection_destroy(queue->connection);
    }

    if (NULL != queue->file)
    {
        wwm_file_destroy(queue->file);
    }

    wwm_buffer_destroy(queue->send_buffer);

    g_wwm_allocator.free(queue);
}

//------------------------------------------------------------------------------
/**
    If a thread makes use of a wwm_message_queue_t instance it should call this 
    function prior to termination to ensure that any thread-specific resources 
    obtained by the wwm_message_queue_t instance are released.
*/
void
wwm_message_queue_exit_thread(wwm_message_queue_t queue)
{
    void* ptq = wwm_thread_key_get(queue->per_thread_queue_key);
    if (NULL != ptq)
    {
        _wwm_per_thread_queue_destroy((_wwm_per_thread_queue_t)ptq);
    }
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
static WWM_THREADPROC_RETTYPE
WWM_THREADPROC
_wwm_message_queue_start(void* queue)
{
    _wwm_message_queue_run((wwm_message_queue_t)queue);
    return 0; // Implied pthread_exit(NULL)
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
            wwm_buffer_t buffer = _wwm_per_thread_queue_dequeue(ptq);
            while (NULL != buffer)
            {
                queue->send_buffer = wwm_buffer_append_int32(queue->send_buffer, wwm_buffer_length(buffer));
                queue->send_buffer = wwm_buffer_append_buffer(queue->send_buffer, buffer);
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
                buffer = _wwm_per_thread_queue_dequeue(ptq);
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
wwm_message_queue_enqueue(wwm_message_queue_t queue, wwm_buffer_t buffer)
{
    _wwm_per_thread_queue_t ptqueue = wwm_thread_key_get(queue->per_thread_queue_key);
    if (NULL == ptqueue)
    {
        ptqueue = _wwm_per_thread_queue_new(queue);
        wwm_thread_key_set(queue->per_thread_queue_key, ptqueue);
    }
    _wwm_per_thread_queue_enqueue(ptqueue, buffer);
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
    _wwm_per_thread_queue_t ptqueue = (_wwm_per_thread_queue_t)g_wwm_allocator.malloc(sizeof(struct _wwm_per_thread_queue_t_));
    ptqueue->owner = owner;
    ptqueue->head = ptqueue->tail = wwm_buffer_new(0);
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
_wwm_per_thread_queue_enqueue(_wwm_per_thread_queue_t per_thread_queue, wwm_buffer_t buffer)
{
	MemoryBarrier();
    wwm_buffer_set_next(per_thread_queue->tail, buffer);
    per_thread_queue->tail = buffer;
}

//------------------------------------------------------------------------------
/**
*/
static wwm_buffer_t
_wwm_per_thread_queue_dequeue(_wwm_per_thread_queue_t per_thread_queue)
{
    wwm_buffer_t old_head = per_thread_queue->head;
    wwm_buffer_t next_head = wwm_buffer_get_next(old_head);
    if (NULL == next_head)
    {
        // queue was empty.
        return NULL;
    }
    per_thread_queue->head = next_head;
    wwm_buffer_destroy(old_head);
    return next_head;
}

