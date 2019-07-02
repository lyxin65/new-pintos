//
// Created by acmzms on 19-6-28.
//

#ifndef FRAMETABLE_H
#define FRAMETABLE_H

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

void evict(void);

void* create_frame(int pos);

void* create_zero(int pos);

void delete_frame(void* page);

void* visit_frame(int pos, int w_r);



#endif // frametable.h
