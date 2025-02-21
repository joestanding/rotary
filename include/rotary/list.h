/*
 * include/rotary/list.h
 * Circular and Linked List Implementations
 */

#ifndef INC_LIST_H
#define INC_LIST_H

#include <rotary/core.h>

#define INIT_LIST_HEAD(name) { .next = &(name), .prev = &(name) }

#define list_for_each(entry, head, member) \
    for (entry = container_of((head)->next, typeof(*entry), member); \
         &entry->member != NULL; \
         entry = container_of(entry->member.next, typeof(*entry), member))

#define clist_for_each(entry, head, member) \
    for (entry = container_of((head)->next, typeof(*entry), member); \
         &entry->member != (head); \
         entry = container_of(entry->member.next, typeof(*entry), member))

/* Used when entries may be removed, but the list must be continued to be
 * traversed - keeps a backup of the next node in the list */
#define list_for_each_safe(entry, tmp, head, member) \
    for (entry = container_of((head)->next, typeof(*entry), member), \
         tmp = container_of(entry->member.next, typeof(*entry), member); \
         &entry->member != NULL; \
         entry = tmp, tmp = container_of(entry->member.next, typeof(*entry), member))

#define clist_for_each_safe(entry, tmp, head, member) \
    for (entry = container_of((head)->next, typeof(*entry), member), \
         tmp = container_of(entry->member.next, typeof(*entry), member); \
         &entry->member != (head); \
         entry = tmp, tmp = container_of(entry->member.next, typeof(*entry), member))

typedef struct list_node_t {
    struct list_node_t * next;
    struct list_node_t * prev;
} list_node_t, list_head_t;

/* ------------------------------------------------------------------------- */
/* Circular Doubly-Linked List                                               */
/* ------------------------------------------------------------------------- */

static inline void clist_init(list_node_t * first_node) {
    if(first_node == NULL) return;

    first_node->next = first_node;
    first_node->prev = first_node;
}

/* ------------------------------------------------------------------------- */

static inline void clist_add(list_head_t * node_head, list_node_t * node_new) {

    if(node_head->next != NULL)
        node_head->next->prev = node_new;

    node_new->next = node_head->next;

    node_new->prev = node_head;

    node_head->next = node_new;
}

/* ------------------------------------------------------------------------- */

static inline void clist_add_before(list_node_t * node_ref,
                                    list_node_t * node_new) {
    if(node_ref == NULL || node_new == NULL) return;

    node_new->next = node_ref;
    node_new->prev = node_ref->prev;

    node_ref->prev = node_new;

    node_new->prev->next = node_new;
}

/* ------------------------------------------------------------------------- */

static inline void clist_add_after(list_node_t * node_ref,
                                   list_node_t * node_new) {
    if(node_ref == NULL || node_new == NULL) return;

    node_new->next = node_ref->next;
    node_new->prev = node_ref;

    node_ref->next = node_new;

    node_new->next->prev = node_new;
}

/* ------------------------------------------------------------------------- */

static inline void clist_delete_node(list_node_t * node) {
    if(node == NULL) return;

    node->prev->next = node->next;
    node->next->prev = node->prev;
}

/* ------------------------------------------------------------------------- */
/* Linear Doubly-Linked List                                                 */
/* ------------------------------------------------------------------------- */

static inline void llist_init(list_node_t * first_node) {
    if(first_node == NULL) return;

    first_node->next = NULL;
    first_node->prev = NULL;
}

/* ------------------------------------------------------------------------- */

static inline void llist_add(list_head_t * node_head, list_node_t * node_new) {

    if(node_head->next != NULL)
        node_head->next->prev = node_new;

    node_new->next = node_head->next;

    node_new->prev = node_head;

    node_head->next = node_new;
}

/* ------------------------------------------------------------------------- */

static inline void llist_add_before(list_node_t * node_ref,
                                    list_node_t * node_new) {
    if(node_ref == NULL || node_new == NULL) return;

    node_new->next = node_ref;
    node_new->prev = node_ref->prev;

    // If the ref. node had a preceding node, update its 'next' ptr
    if(node_ref->prev != NULL) {
        node_ref->prev->next = node_new;
    }

    node_ref->prev = node_new;

}

/* ------------------------------------------------------------------------- */

static inline void llist_add_after(list_node_t * node_ref,
                                   list_node_t * node_new) {
    if(node_ref == NULL || node_new == NULL) return;

    node_new->next = node_ref->next;
    node_new->prev = node_ref;

    if(node_ref->next != NULL) {
        node_ref->next->prev = node_new;
    }

    node_ref->next = node_new;
}

/* ------------------------------------------------------------------------- */

static inline void llist_delete_node(list_node_t * node) {
    if(node == NULL) return;

    if(node->prev != NULL) {
        node->prev->next = node->next;
    }

    if(node->next != NULL) {
        node->next->prev = node->prev;
    }
}

/* ------------------------------------------------------------------------- */

static inline int32_t llist_length(list_node_t * start_node) {
    uint32_t count = 0;
    list_node_t * current_node = start_node;

    while(current_node != NULL) {
        count++;
        current_node = current_node->next;
    }

    return count;
}

/* ------------------------------------------------------------------------- */

static inline list_node_t * llist_get_last(list_node_t * start_node) {
    list_node_t * current_node = start_node;

    while(current_node != NULL) {
        if(current_node->next == NULL) {
            return current_node;
        }
        current_node = current_node->next;
    }

    return (list_node_t*)E_ERROR;
}

/* ------------------------------------------------------------------------- */

static inline void llist_add_end(list_node_t * node_ref,
                                 list_node_t * node_new) {
    if(node_new == NULL) return;

    if(node_ref == NULL) {
        // If the list is empty, initialize node_new as the first node
        llist_init(node_new);
    } else {
        // Find the last node and add node_new after it
        list_node_t * last_node = llist_get_last(node_ref);
        if(last_node != (list_node_t*)E_ERROR) {
            llist_add_after(last_node, node_new);
        }
    }
}

/* ------------------------------------------------------------------------- */

#endif
