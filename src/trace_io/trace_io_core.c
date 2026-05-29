#include "trace_io_fs.h"

#include <string.h>

struct trace_io_node
{
    char *scheme;
    const struct trace_io_fs_ops *ops;
    struct trace_io_node *next;
    struct trace_io_node *prev;
};

static struct trace_io_node * registered_nodes = NULL;
static void node_destroy(struct trace_io_node **node);

void trace_io_fs_init(void)
{
    
}

TRACERet_t trace_io_fs_register(const char *scheme, const struct trace_io_fs_ops *ops)
{
    TRACERet_t status;
    struct trace_io_node *new_node;
    struct trace_io_node *first_node;

    /* Input sanitize */
    if (scheme == NULL || ops == NULL)
    {
        status = TRACE_BAD_INPUT_PTR;
        goto return_status;
    }

    /* Check for an invalid driver function pointer */
    if(ops->close == NULL || ops->open == NULL || ops->read == NULL)
    {
        status = TRACE_INVALID_REGISTER;
        goto return_status;
    }

    /* Allocate a new node */
    new_node = (struct trace_io_node*)malloc(sizeof(struct trace_io_node));
    if (new_node == NULL)
    {
        status = TRACE_ALLOC_FAIL;
        goto return_status;
    }

    /* Initialize the internal node pointers */
    new_node->next = NULL;
    new_node->prev = NULL;
    new_node->ops = ops;

    /* Alloc memory for the scheme name */
    new_node->scheme = (char*)malloc(sizeof(strlen(scheme)));

    if (new_node->scheme == NULL)
    {
        status = TRACE_ALLOC_FAIL;
        goto cleanup_failure;
    }

    /* Copy the scheme name inside the new node */
    strcpy(new_node->scheme, scheme);

    /* Check if this is the very first registered fs driver */
    if (registered_nodes == NULL)
    {
        registered_nodes = new_node;
        registered_nodes->next = registered_nodes;
        registered_nodes->prev = registered_nodes;
    }
    else
    {
        first_node = registered_nodes->next;
        new_node->next = first_node;
        new_node->prev = registered_nodes;
        first_node->prev = new_node;
        registered_nodes->next = new_node;
    }

    status = TRACE_SUCCESS;
    goto return_status;

cleanup_failure:
    node_destroy(&new_node);

return_status:
    return status;

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
                /* Remove the pointer from the linked list */
                tmp_node->next->prev = tmp_node->prev;
                tmp_node->prev->next = tmp_node->next;
                
                /* If we are removing the first entry in the linked list, assign the 
                    Global pointer to the next entry 
                */
                if (tmp_node == registered_nodes)
                {
                    registered_nodes = tmp_node->next;
                }
                /* Destroy the node from the memory */
                node_destroy(&tmp_node);
                break;
            }

            tmp_node = tmp_node->next;
        } while (tmp_node != registered_nodes);
        

    }
}

void trace_io_fs_unregister_all(void)
{
    struct trace_io_node *tmp_node;
    struct trace_io_node *next_node;

    if (registered_nodes != NULL)
    {
        tmp_node = registered_nodes;
        do
        {
            next_node = tmp_node->next;
            node_destroy(&tmp_node);
            tmp_node = next_node;
        }while(tmp_node != registered_nodes);

        registered_nodes = NULL;
    }
}

static void node_destroy(struct trace_io_node **node)
{
    if (node != NULL)
    {
        if (*node != NULL)
        {
            if ((*node)->scheme != NULL)
            {
                free((*node)->scheme);
                (*node)->scheme = NULL;
            }
            (*node)->next = NULL;
            (*node)->prev = NULL;
            (*node)->ops = NULL;

            free(*node);
            *node = NULL;
        }
    }
}