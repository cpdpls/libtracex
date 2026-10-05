#ifndef __TRACEX_LIST_H__
#define __TRACEX_LIST_H__

#include <stdlib.h>

#define tracex_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define tracex_container_of(ptr, type, member) ({				\
  const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - tracex_offsetof(type, member) ); })

#define tracex_list_entry(ptr, type, member) \
	tracex_container_of(ptr, type, member)

#define tracex_list_first_entry(ptr, type, member) \
	tracex_list_entry((ptr)->next, type, member)
    
#define tracex_list_entry_is_head(pos, head, member)				\
	tracex_list_is_head(&pos->member, (head))

#define tracex_list_next_entry(pos, member) \
	tracex_list_entry((pos)->member.next, typeof(*(pos)), member)

#define tracex_list_for_each_entry(pos, head, member)				\
    for (pos = tracex_list_first_entry(head, typeof(*pos), member);	\
        !tracex_list_entry_is_head(pos, head, member);			\
        pos = tracex_list_next_entry(pos, member))

#define tracex_list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = tracex_list_first_entry(head, typeof(*pos), member),	\
		n = tracex_list_next_entry(pos, member);			\
	     !tracex_list_entry_is_head(pos, head, member); 			\
	     pos = n, n = tracex_list_next_entry(n, member))
        
struct tracex_list {
    struct tracex_list *prev;
    struct tracex_list *next;
};

typedef struct tracex_list tracex_node;


static inline int tracex_list_is_head(const struct tracex_list *list, const struct tracex_list *head)
{
	return list == head;
}

static void tracex_list_init(struct tracex_list *head)
{
    if (head != NULL)
    {
        head->next = head;
        head->prev = head;
    }
}
static void tracex_list_insert(struct tracex_list *entry, struct tracex_list *head)
{
    if (entry != NULL || head != NULL)
    {
        head->next->prev = entry;
        entry->next  = head->next;
        entry->prev = head;
        head->next = entry;
    }
    
}
static void tracex_list_delete(struct tracex_list *entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
}

#endif
