//
// Created by acmzms on 19-6-28.
//

#pragma once
#include "threads/palloc.h"
#include "userprog/exception.h"

struct frame
{
    // info from process
    void* pageptr;
    int pos;
    struct frame* next;
    int r; //for LRU
    int dirty;
    /*
    frame()
    {
        pageptr = NULL;
        next = NULL;
        dirty = 0;
        r = 1;
    }
    */
};

struct frame* framehead;
struct frame* clockptr;

void* create_frame(int pos) //allocate pages
{
    struct frame* p = framehead;
    while(p->next != NULL)
    {
        p = p->next;
    }
    void* tmp = palloc_get_page(PAL_USER);
    if(tmp != NULL)
    {
        p->next = malloc(sizeof(struct frame));
        p->next->r = 1;
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

void* create_zero(int pos) //allocate pages
{
    struct frame* p = framehead;
    while(p->next != NULL)
    {
        p = p->next;
    }
    void* tmp = palloc_get_page(PAL_USER | PAL_ZERO);
    if(tmp != NULL)
    {
        p->next = malloc(sizeof(struct frame));
        p->next->r = 1;
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


void delete_frame(void* page)
{
    struct frame* f = framehead;
    while(f->next != NULL)
    {
        if(f->next->pageptr == page)
        {
            if(f->next == clockptr)
            {
                clockptr = f->next->next;
            }
            palloc_free_page(f->next->pageptr);
            free(f->next);
            f->next = f->next->next;
        }
    }
}

void* visit_frame(int pos, int w_r)
{
    struct frame* f = framehead;
    while(f->next != NULL)
    {
        f = f->next;
        if(f->pos == pos)
        {
            if(w_r) f->dirty = 1;
            return f->pageptr;
        }
    }
    struct intr_frame* g;
    g->eax = -1;
    //page_fault(g);
    return NULL;
}

void evict()
{
    while(1)
    {
        if(clockptr->next == NULL)
        {
            clockptr = framehead;
        }
        if(!clockptr->next->r)
        {
            if(clockptr->next->dirty)
            {
                frame_out(clockptr->next->pageptr, clockptr->next->pos);
            }
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

