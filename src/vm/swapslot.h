//
// Created by acmzms on 19-6-29.
//

#pragma once

#include "frametable.c"
#include "threads/vaddr.h"
#include "devices/block.h"
#define BLOCKS_PER_PAGE = /block.size
int* slot;
void* buffer;
int size = 3 << 8;

void init()
{
    slot = malloc(size * sizeof(int));
    for(int i = 0;i < size;i++)
    {
        slot[i] = -1;
    }
}

void swap_in(int pos)
{
    struct block *swap_block;
    swap_block = block_get_role (BLOCK_SWAP);
    int sssssize = swap_block->size;
    int bpp = PGSIZE / swap_block->size;// BLOCKS_PER_PAGE
    int k;
    for(k = 0;k < size;k++)
    {
        if(slot[k] != -1)
        {
            slot[k] = pos;
            break;
        }
    }
    for(int i = 0;i < bpp;i++)
    {
        block_write(swap_block, k * bpp + i, buffer + i * swap_block->size);
    }
}

void swap_out(int pos)
{
    struct block *swap_block;
    swap_block = block_get_role (BLOCK_SWAP);
    int bpp = PGSIZE / swap_block->size;// BLOCKS_PER_PAGE
    int k;
    for(k = 0;k < size;k++)
    {
        if(slot[k] != -1)
        {
            slot[k] = -1;
            break;
        }
    }
    for(int i = 0;i < bpp;i++)
    {
        block_read(swap_block, k * bpp + i, buffer + i * swap_block->size);
    }
}

void frame_in(int pos)
{
    void *p = create_frame(pos);
    swap_out(pos);
    memcpy(p, buffer, PGSIZE);
}

void frame_out(void* page, int pos)
{
    memcpy(buffer, page, PGSIZE);
    swap_in(pos);
}

void frame_free(int pos)
{
    for(int i = 0;i < size;i++)
    {
        if(slot[i] == pos) slot[i] = -1;
    }
}
