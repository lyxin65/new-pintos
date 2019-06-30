//
// Created by acmzms on 19-6-29.
//

#pragma once
#include"frametable.c"
#define PAGE_NUM = 3 << 18;

struct page
{
    void* pageptr;
    //
};

page[PAGE_NUM] pagetable;

void create_page(int addr)
{
    int pos = addr / PGSIZE;
    page[pos].pageptr = create_frame(pos);
}

void read_page(int addr, void* buffer, unsigned size)
{
    //interface for proj2
}

void write_page(int addr, const void* buffer, unsigned size)
{
    //interface for proj2
}