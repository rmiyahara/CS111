/*
NAME: Ryan Miyahara
EMAIL: rmiyahara144@gmail.com
ID: 804585999
*/

#include "SortedList.h"
#include <stdio.h>
#include <string.h>
#include <sched.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
    if (!element)
        return;
    if (!list)
        return;
    if (opt_yield && INSERT_YIELD)
        sched_yield();
       
    SortedListElement_t* p = list;
    while (p != list) {
        if (strcmp(element->key, p->key) <= 0)
            break;
        else
            p = p->next;
    }
    element->prev = p->prev;
    element->next = p;
    element->prev->next = element;
    p->prev = element;
    return;

}

int SortedList_delete( SortedListElement_t *element) {
    if (opt_yield && DELETE_YIELD)
        sched_yield();

    if ((element ->prev->next != element) || (element->next->prev != element))
        return -1;
    element->prev->next = element->next;
    element->next->prev = element->prev;
    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
    if (opt_yield && LOOKUP_YIELD)
        sched_yield();
    
    SortedListElement_t* p = list->next;
    while(curr != list) {
        if (strcmp(key, curr->key) == 0)
            return p;
        else
            p = p->next;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list) {
    int len = 0;
    if (opt_yield && LOOKUP_YIELD)
        sched_yield();
    
    SortedListElement_t* p = list->next;
    while (p != list) {
        if ((p->prev->next != p) || (p->next->prev != p))
            return -1;
        len++;
        p = p->next;
    }
    return len;
}