//
// Created by acmzms on 19-6-29.
//

#pragma once

#include "frametable.c"
#define BLOCKS_PER_PAGE = /block.size
int* slot;
void* buffer;

void init(int size)
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
    int bpp = PGSIZE / swap_block->size;// BLOCKS_PER_PAGE
    swap_block = block_get_role (BLOCK_SWAP);
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
        block_write(swap_block, k * bpp + i, buffer);
    }
}

void swap_out(int pos)
{
    struct block *swap_block;
    int bpp = PGSIZE / swap_block->size;// BLOCKS_PER_PAGE
    swap_block = block_get_role (BLOCK_SWAP);
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
        block_read(swap_block, k * bpp + i, buffer);
    }
}

void frame_in(int pos)
{
    void* p = create_frame(pos);
    swap_out(pos);
    memcpy(p, buffer, PGSIZE);
}

void frame_out(void* page, int pos)
{
    memcpy(buffer, page, PGSIZE);
    swap_in(pos);
}

void free(int pos)
{
    for(int i = 0;i < size;i++)
    {
        if(slot[i] == pos) slot[i] = -1;
    }
}
