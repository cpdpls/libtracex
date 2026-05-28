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

    tmp_node = registered_nodes;

    if (tmp_node != NULL)
    {
        do
        {
            /* Node found ! Just remove it */
            if (strcmp(scheme, tmp_node->scheme) == 0)
            {
                /* Remote the pointer from the linked list */
                tmp_node->next->prev = tmp_node->prev;
                tmp_node->prev->next = tmp_node->next;
                tmp_node->next = NULL;
                tmp_node->prev = NULL;

                /* Free memory for the scheme string */
                if (tmp_node->scheme != NULL)
                {
                    free(tmp_node->scheme);
                    tmp_node->scheme = NULL;
                }

                /* Free memory for the node operations */
                if (tmp_node->ops != NULL)
                {
                    free(tmp_node->ops);
                    tmp_node->ops = NULL;
                }

                /* Free the node structure */
                free(tmp_node);

            }

            tmp_node = tmp_node->next;
        } while (tmp_node->next != registered_nodes);
        

    }
}

TRACERet_t trace_io_fs_unregister_all(void)
{

}