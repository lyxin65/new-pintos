//
// Created by acmzms on 19-6-29.
//

#include "vm/swapslot.h"
#include "vm/frametable.h"
#include "threads/vaddr.h"
#include "devices/block.h"
#include <string.h>
#include "threads/malloc.h"
#include "threads/palloc.h"

int* slot;
void* buffer;
int size = 3 << 8;

void swap_init()
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
    swap_block = (struct block *)block_get_role (BLOCK_SWAP);
    int bpp = PGSIZE / block_size(swap_block);// BLOCKS_PER_PAGE
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
        block_write(swap_block, k * bpp + i, buffer + i * block_size(swap_block));
    }
}

void swap_out(int pos)
{
    struct block *swap_block;
    swap_block = block_get_role (BLOCK_SWAP);
    int bpp = PGSIZE / block_size(swap_block);// BLOCKS_PER_PAGE
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
        block_read(swap_block, k * bpp + i, buffer + i * block_size(swap_block));
    }
}

void frame_in(int pos)
{
    void *p = palloc_get_page(PAL_USER);
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
