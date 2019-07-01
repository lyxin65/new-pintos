//
// Created by acmzms on 19-6-29.
//

#pragma once

#include <math.h>
#include"frametable.c"
#include "userprog/syscall.c"
#define PAGE_NUM = 3 << 18;
#define FILE_NUM = 3 << 9;
mapid_t[PAGE_NUM] pagetable;
int[FILE_NUM] filetable; //start pos
int[FILE_NUM] filesize;  //size
int[FILE_NUM] filename;  //fileid
mapid_t FILE_NO = 1;

void* buffer;

mapid_t add_file(int fd, int va, int sz) //va : addr, sz : file size
{
    struct block *file_block;
    swap_block = block_get_role (BLOCK_FILESYS);
    int blksz = file_block->size;// BLOCKS_SIZE
    int num = ceil(sz / blksz);
    sys_read(fd, buffer, sz);
    for(int i = 0;i < num;i++)
    {
        block_write(file_block, va / file_block->size + num, buffer + num * cntsz);
        pagetable[va / file_block->size] = FILE_NO;
    }
    filetable[FILE_NO] = va / file_block->size;
    filesize[FILE_NO] = sz;
    filename[FILE_NO] = fd;
    FILE_NO++;
    return FILE_NO - 1;
}

void rem_file(mapid_t m)
{
    if(filename[m] == 0) return;
    struct block *file_block;
    swap_block = block_get_role (BLOCK_FILESYS);
    int blksz = file_block->size;// BLOCKS_SIZE
    int pos = filetable[m];
    int cnt = 0
    while(pagetable[pos + cnt] == m)
    {
        block_read(file_block, pos + cnt, buffer + cnt * blksz);
        pagetable[pos + cnt] = 0;
        cnt++;
    }
    sys_write(filename[m], buffer, filesize[m]);
    filename[m] = 0;
}