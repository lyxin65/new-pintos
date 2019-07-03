#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <list.h>


// in the user/syscall.h
/* Map region identifier. */
typedef int mapid_t;
#define MAP_FAILED ((mapid_t) -1)
//


extern struct lock lock_for_fs;
//yn
struct file_descriptor
{
    uint64_t num; //0 stdin 1 stdout 2stderror(not asked)
    struct file* file;
    struct list_elem elem;
    /* data */
};


void sys_exit(int);

void syscall_init (void);

//#ifdef VM


struct mmap_desc{
  mapid_t id;
  struct list_elem elem;
  struct file* file;

  void *vaddr;   // user vaddr
  size_t size;  // file size
};
//#endif

#ifdef VM
bool sys_munmap (mapid_t);
#endif

#endif /* userprog/syscall.h */
