//
// Created by acmzms on 19-6-28.
//

#pragma once
#include "threads/palloc.h"
#include "threads/palloc.c"
#include "userprog/exception.c"

struct frame
{
    // info from process
    void* pageptr;
    int pos;
    frame* next;
    int r //for LRU
    int dirty;
    frame()
    {
        pageptr = null;
        next = null;
        dirty = 0;
        r = 1;
    }
};

frame* framehead;
frame* clockptr;

void* create_frame(int pos) //allocate pages
{
    frame* p = framehead;
    while(p->next != null)
    {
        p = p->next;
    }
    void* tmp = palloc_get_page(PAL_USER);
    if(tmp != null)
    {
        p->next = malloc(sizeof(frame));
        p->next->pageptr = tmp;
        p->next->pos = pos;
        return tmp;
    }
    else
    {
        evict();
        return create_frame(pos);
    }
}

void evict()
{
    while(1)
    {
        if(clockptr->next == null)
        {
            clockptr = framehead;
        }
        if(!clockptr->next->r)
        {
            palloc_free_page(clockptr->next->pageptr);
            clockptr->next = clockptr->next->next;
            free(clockptr->next);
            return;
        }
        else
        {
            clockptr->r = 0;
            clockptr = clockptr->next;
        }
    }
}

