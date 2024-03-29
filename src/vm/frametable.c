//
// Created by acmzms on 19-6-28.
//


#include "vm/frametable.h"
#include "threads/palloc.h"
#include "userprog/exception.h"
#include "vm/swapslot.h"
#include "threads/malloc.h"

struct frame *framehead;
struct frame *clockptr;

void frame_init() {
	printf("   enter frame init\n");
	framehead = malloc(sizeof(struct frame));
	printf("   1 step\n");
	clockptr = framehead;
}

void create_frame(int pos, void* tmp) //allocate pages
{
	struct frame* p = framehead;
    while(p->next != NULL)
    {
        p = p->next;
    }
    //void* tmp = palloc_get_page(PAL_USER);
    printf("   after while\n");
    if(tmp != NULL)
    {
    	p->next = malloc(sizeof(struct frame));
		p->next->r = 1;
		p->next->pageptr = tmp;
		p->next->pos = pos;
    }
    else
    {
        evict();
        create_frame(pos, tmp);
    }
}

/*void* create_zero(int pos, page tmp) //allocate pages
{
    struct frame* p = framehead;
    while(p->next != NULL)
    {
        p = p->next;
    }
    //void* tmp = palloc_get_page(PAL_USER | PAL_ZERO);
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
}*/
//please delete this

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
            //palloc_free_page(f->next->pageptr);
            free(f->next);
            f->next = f->next->next;
            break;
        }
        f = f->next;
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
    //struct intr_frame* g;
    //g->eax = -1;
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

