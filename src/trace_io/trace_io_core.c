#include "trace_io_fs.h"

#include <string.h>

struct trace_io_node
{
    const char *scheme;
    struct trace_io_fs_ops *ops;
    struct trace_io_node *next;
    struct trace_io_node *prev;
};

static struct trace_io_node * registered_nodes = NULL;

TRACERet_t trace_io_fs_register(const char *scheme, const struct trace_io_fs_ops *ops)
{
    struct trace_io_node *new_node;
    struct trace_io_node *first_node;

    if (scheme == NULL || ops == NULL)
        return TRACE_BAD_INPUT_PTR;

    new_node = (struct trace_io_node*)malloc(sizeof(struct trace_io_node));
    if (new_node == NULL)
    {
        return TRACE_ALLOC_FAIL;
    }
    new_node->next = NULL;
    new_node->prev = NULL;
    new_node->ops = ops;
    new_node->scheme = (const char*)malloc(sizeof(strlen(scheme)));

    if (new_node->scheme == NULL)
    {
        return TRACE_ALLOC_FAIL;
    }

    if (registered_nodes == NULL)
    {
        registered_nodes = new_node;
        registered_nodes->next = registered_nodes;
        registered_nodes->prev = registered_nodes;
        return TRACE_SUCCESS;
    }
    else
    {
        first_node = registered_nodes->next;

        new_node->next = first_node;
        new_node->prev = registered_nodes;

        first_node->prev = new_node;
        registered_nodes->next = new_node;


    }



}

void trace_io_fs_unregister(const char *scheme)
{
    struct trace_io_node *tmp_node;

    /* Input sanitize */
    if (tmp_node != NULL)
    {
        /* Loop over the cricular linked list until we find the scheme */
        for (tmp_node = registered_nodes; tmp_node->next != registered_nodes; tmp_node = tmp_node->next)
        {
            if (strcmp)

        }

    }
}

TRACERet_t trace_io_fs_unregister_all(void)
{

}