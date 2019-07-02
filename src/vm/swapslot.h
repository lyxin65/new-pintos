//
// Created by acmzms on 19-6-29.
//

#ifndef SWAPSLOT_H
#define SWAPSLOT_H

#include "vm/frametable.h"
#include "threads/vaddr.h"
#include "devices/block.h"
#include <string.h>
#include "threads/malloc.h"



void swap_init(void);

void swap_in(int pos);

void swap_out(int pos);

void frame_in(int pos);

void frame_out(void* page, int pos);

void frame_free(int pos);

#endif // swapslot.h
