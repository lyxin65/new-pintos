//
// Created by acmzms on 19-6-29.
//
#include "vm/suplpagetable.h"
#include "vm/frametable.h"
#include "userprog/syscall.h"
#include "devices/block.h"

mapid_t add_file(int fd, int va, int sz) //va : addr, sz : file size
{
    struct block *file_block;
    file_block = block_get_role (BLOCK_FILESYS);
    int blksz = block_size(file_block);// BLOCKS_SIZE
    int num = (sz + blksz - 1) / blksz;
    sys_read(fd, buffer, sz);
    for(int i = 0;i < num;i++)
    {
        block_write(file_block, va / block_size(file_block) + num, buffer + num * blksz);
        pagetable[va / block_size(file_block)] = FILE_NO;
    }
    filetable[FILE_NO] = va / block_size(file_block);
    file_size[FILE_NO] = sz;
    filename[FILE_NO] = fd;
    FILE_NO++;
    return FILE_NO - 1;
}

void rem_file(mapid_t m)
{
    if(filename[m] == 0) return;
    struct block *file_block;
    file_block = block_get_role (BLOCK_FILESYS);
    int blksz = block_size(file_block);// BLOCKS_SIZE
    int pos = filetable[m];
    int cnt = 0;
    while(pagetable[pos + cnt] == m)
    {
        block_read(file_block, pos + cnt, buffer + cnt * blksz);
        pagetable[pos + cnt] = 0;
        cnt++;
    }
    sys_write(filename[m], buffer, file_size[m]);
    filename[m] = 0;
}

void visit_file(mapid_t m, int modify, int dlt)
{
    struct block *file_block;
    file_block = block_get_role (BLOCK_FILESYS);
    int blksz = block_size(file_block);// BLOCKS_SIZE
    int pos = filetable[m] + dlt / block_size(file_block);
    visit_frame(pos , modify);
}
