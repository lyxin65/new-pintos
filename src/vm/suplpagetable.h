//
// Created by acmzms on 19-6-29.
//

#ifndef SUPLPAGETABLE_H
#define SUPLPAGETABLE_H

#include "vm/frametable.h"
#include "userprog/syscall.h"
#include "devices/block.h"
#include "user/syscall.h"

#define PAGE_NUM (3 << 8)
#define FILE_NUM (3 << 8)
mapid_t pagetable[PAGE_NUM];
int filetable[FILE_NUM]; //start pos
int file_size[FILE_NUM];  //size
int filename[FILE_NUM];  //fileid
mapid_t FILE_NO = 1;

void* buffer;

mapid_t add_file(int fd, int va, int sz);

void rem_file(mapid_t m);

void visit_file(mapid_t m, int modify, int dlt);

#endif // suplpagetable.h
