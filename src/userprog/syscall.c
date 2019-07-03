#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include "devices/input.h"
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "user/syscall.h"

#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "userprog/pagedir.h"
#include "threads/synch.h"
#include "threads/malloc.h"
#include "lib/kernel/list.h"

#ifdef VM
#include "vm/page.h"
#endif

#ifdef DEBUG
#define __debug(...) printf(__VA_ARGS__)
#else
#define __debug(...) /* nop */
#endif

//what to do :
//wait \exev \boundary check

#ifdef VM
enum fd_search_filter
{
  FD_FILE = 1,
  FD_DIRECTORY = 2
};

static struct mmap_desc* find_mmap_desc(struct thread *, mapid_t fd);

void preload_and_pin_pages(const void *, size_t);
void unpin_preloaded_pages(const void *, size_t);

#endif

struct lock lock_for_fs;
static int get_user(const uint8_t *uaddr);
static bool put_user(uint8_t *udst, uint8_t byte);
static void syscall_handler(struct intr_frame *);

static void fail_invalid_access(void)
{
  if (lock_held_by_current_thread(&lock_for_fs))
    lock_release(&lock_for_fs);

  sys_exit(-1);
  NOT_REACHED();
}
static void boundary_check(const void *uaddr)
{
  struct thread *cur = thread_current();
  if (uaddr == NULL || !is_user_vaddr(uaddr) ||
      (pagedir_get_page(cur->pagedir, uaddr)) == NULL)
    sys_exit(-1);

  // check uaddr range or segfaults
  if (get_user(uaddr) == -1)
    fail_invalid_access();
}

/* modified by YN  begin*/
//file_system_lock      must have only one threads in the file system at any time

void syscall_init(void)
{
  lock_init(&lock_for_fs);
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}
/* modified by YN  end*/

/* modified by YN  begin*/

struct file_descriptor *get_fd(struct thread *, int);

struct file_descriptor *get_fd(struct thread *t, int fd_num)
{
  ASSERT(t != NULL);

  if (fd_num == 0 || fd_num == 1) //stdin and out
    return NULL;

  struct list *fd_list = &t->file_descriptors;
  struct list_elem *e;
  e = list_begin(fd_list);
  while (e != list_end(fd_list))
  {
    struct file_descriptor *fd = list_entry(e, struct file_descriptor, elem);
    if ((int)fd->num == fd_num)
      return fd;
    e = list_next(e);
  }

  return NULL;
}

static int num_of_args(const char *esp, int args[])
{
  int argc;
  boundary_check((const uint8_t *)(esp));
  boundary_check((const uint8_t *)(esp + 1));
  boundary_check((const uint8_t *)(esp + 2));
  boundary_check((const uint8_t *)(esp + 3)); //maybe check all is a must

  int sys_num = *(int *)esp;
  switch (sys_num)
  {
  case SYS_HALT:
    argc = 1;
    break;
  case SYS_EXIT:
  case SYS_EXEC:
  case SYS_WAIT:
  case SYS_REMOVE:
  case SYS_OPEN:
  case SYS_FILESIZE:
  case SYS_TELL:
  case SYS_CLOSE:
  #ifdef VM
  case SYS_MUNMAP:
  #endif
    argc = 2;
    break;
  case SYS_CREATE:
  case SYS_SEEK:
  #ifdef VM
  case SYS_MMAP:
  #endif
    argc = 3;
    break;
  case SYS_READ:
  case SYS_WRITE:
    argc = 4;
    break;
  default:
    ASSERT(false);
  }

  for (int i = 0; i < argc; ++i)
  {
    const int *addr = (const int *)(esp + 4 * i);
    boundary_check((const uint8_t *)(addr));
    boundary_check((const uint8_t *)(addr + 1));
    boundary_check((const uint8_t *)(addr + 2));
    boundary_check((const uint8_t *)(addr + 3));
    args[i] = *addr;
  }
  return argc;
}

//void sys_halt(void) NO_RETURN;
pid_t sys_exec(const char *cmd_line);
int sys_wait(pid_t pid);
bool sys_create(const char *file, unsigned initial_size);
bool sys_remove(const char *file);
int sys_open(const char *file);
int sys_filesize(int fd_num);
int sys_read(int fd_num, void *buffer, unsigned size);
int sys_write(int fd_num, const void *buffer, unsigned size);
void sys_seek(int fd_num, unsigned position);
unsigned sys_tell(int fd_num);
void sys_close(int fd_num);
void sys_exit(int status)
{
  thread_current()->exitcode = status;
  thread_exit();
}
#ifdef VM
mapid_t sys_mmap(int fd, void *addr);
bool sys_munmap(mapid_t mapping);
#endif

static void
syscall_handler(struct intr_frame *f UNUSED)
{
  __debug("calling ");
  //  __debug("name: %s\n", thread_current()->name);
  thread_current()->current_esp = f->esp;
  char *esp = f->esp;

  int args[4] = {0};
  int argc = num_of_args(esp, args);
  ASSERT(argc <= 4);

  int type = args[0];

  switch (type)
  {
  case SYS_HALT:
    __debug("sys_halt\n");
    shutdown_power_off();
    NOT_REACHED(); //no it should not be written , it is a panic
    //dont no what it is, but it is used in lib/user/syscall
  case SYS_EXIT:
    __debug("sys_exit\n");
    sys_exit(args[1]);
    NOT_REACHED();
  case SYS_EXEC:
    __debug("sys_exec\n");
    f->eax = sys_exec((const char *)args[1]);
    return;
  case SYS_WAIT:
    __debug("sys_wait\n");
    f->eax = sys_wait(args[1]);
    return;
  case SYS_CREATE:
    __debug("sys_create %s %d\n", (const char *)args[1], args[2]);
    f->eax = sys_create((const char *)args[1], args[2]);
    return;
  case SYS_REMOVE:
    __debug("sys_remove\n");
    f->eax = sys_remove((const char *)args[1]);
    return;
  case SYS_OPEN:
    __debug("sys_open\n");
    f->eax = sys_open((const char *)args[1]);
    return;
  case SYS_FILESIZE:
    __debug("sys_filesize\n");
    f->eax = sys_filesize(args[1]);
    return;
  case SYS_READ:
    __debug("sys_read\n");
    f->eax = sys_read(args[1], (void *)args[2], args[3]);
    return;
  case SYS_WRITE:
    __debug("sys_write\n");
    f->eax = sys_write(args[1], (const void *)args[2], args[3]);
    return;
  case SYS_SEEK:
    __debug("sys_seek\n");
    sys_seek(args[1], args[2]);
    return;
  case SYS_TELL:
    __debug("sys_tell\n");
    f->eax = sys_tell(args[1]);
    return;
  case SYS_CLOSE:
    __debug("sys_close\n");
    sys_close(args[1]);
    return;
  #ifdef VM
  case SYS_MMAP:
    f->eax = sys_mmap(args[1], args[2]);
    return;
  case SYS_MUNMAP:
    sys_munmap(args[1]);
    return;
  #endif
  default:
    ASSERT(false);
    thread_exit();
  }
}

pid_t sys_exec(const char *cmd_line)
{
  boundary_check((const uint8_t *)(cmd_line));
  boundary_check((const uint8_t *)(cmd_line + 1));
  boundary_check((const uint8_t *)(cmd_line + 2));
  boundary_check((const uint8_t *)(cmd_line + 3));
  __debug("waitting for fs lock\n");
  lock_acquire(&lock_for_fs);
  __debug("..got it!!\n");
  pid_t pid = process_execute(cmd_line);
  lock_release(&lock_for_fs);
  __debug("releasing fs lock!!\n");
  if (pid == TID_ERROR)
  {
    return -1;
  }
  return pid;
}

int sys_wait(pid_t pid)
{
  int id = process_wait(pid);
  return id;
}

bool sys_create(const char *file, unsigned initial_size)
{
  bool success = false;
  boundary_check((const uint8_t *)file);
  lock_acquire(&lock_for_fs);
  success = filesys_create(file, initial_size);
  lock_release(&lock_for_fs);
  return success;
}

bool sys_remove(const char *file)
{
  boundary_check((const uint8_t *)file);
  lock_acquire(&lock_for_fs);
  bool success = filesys_remove(file);
  lock_release(&lock_for_fs);
  return success;
}

int sys_open(const char *file)
{
  boundary_check((const uint8_t *)file);
  struct file *file_open;
  struct file_descriptor *fd = malloc(sizeof(struct file_descriptor)); //it shows can't find the definition ???
  if (!fd)                                                             //open failed
    return -1;

  lock_acquire(&lock_for_fs);
  file_open = filesys_open(file);
  lock_release(&lock_for_fs);
  if (!file_open) //or file_open == NULL ?
  {
    free(fd);
    return -1;
  }
  fd->file = file_open;
  struct list *fd_list = &thread_current()->file_descriptors;
  if (list_empty(fd_list))
    fd->num = 3; //2 for stderr, zwt says
  else
    fd->num = (list_entry(list_back(fd_list), struct file_descriptor, elem)->num) + 1;
  list_push_back(fd_list, &fd->elem);
  return fd->num;
}

int sys_filesize(const int fd_num)
{
  int size;
  struct file_descriptor *fd = get_fd(thread_current(), fd_num);
  if (!fd)
    return -1;
  lock_acquire(&lock_for_fs);
  size = file_length(fd->file);
  lock_release(&lock_for_fs);
  return size;
}

int sys_read(int fd_num, void *buffer, unsigned size)
{
  boundary_check((const uint8_t *)buffer);
  boundary_check((const uint8_t *)buffer + size - 1);
  if (fd_num == STDIN_FILENO) //stdio.h
  {
    uint8_t *udst = (uint8_t *)buffer;
    for (unsigned i = 0; i < size; ++i)
    {
      if (!put_user(udst, input_getc()))
      {
        sys_exit(-1);
      }
      ++udst;
    }
    return size;
  }

  if (fd_num == STDOUT_FILENO)
    return -1;

  int filesize;
  struct file_descriptor *fd = get_fd(thread_current(), fd_num);
  if (!fd)
    return -1;
  lock_acquire(&lock_for_fs);
  
#ifdef VM
      preload_and_pin_pages(buffer, size);  //yi yi bu ming
#endif

  filesize = file_read(fd->file, buffer, size);

#ifdef VM
     unpin_preloaded_pages(buffer, size);
#endif
  lock_release(&lock_for_fs);
  return filesize;
}

int sys_write(int fd_num, const void *buffer, unsigned size)
{
  boundary_check((const uint8_t *)buffer);
  boundary_check((const uint8_t *)buffer + size - 1);
  if (fd_num == STDIN_FILENO)
  {
    return -1;
  }
  if (fd_num == STDOUT_FILENO)
  {
    putbuf((char *)buffer, size); //in fact i dont find this function
                                  //but others are all using it.
    return size;
  }

  int size_success = 0;
  struct file_descriptor *fd = get_fd(thread_current(), fd_num);
  if (!fd)
  {
    return -1;
  }
  lock_acquire(&lock_for_fs);
#ifdef VM
      preload_and_pin_pages(buffer, size);  //yi yi bu ming
#endif

  size_success = file_write(fd->file, buffer, size);
  
#ifdef VM
     unpin_preloaded_pages(buffer, size);
#endif
  lock_release(&lock_for_fs);
  return size_success;
}

void sys_seek(int fd_num, unsigned position)
{
  struct file_descriptor *fd = get_fd(thread_current(), fd_num);
  if (!fd)
    return;
  lock_acquire(&lock_for_fs);
  file_seek(fd->file, position);
  lock_release(&lock_for_fs);
}

unsigned sys_tell(int fd_num)
{
  unsigned position;
  struct file_descriptor *fd = get_fd(thread_current(), fd_num);
  if (!fd)
    return -1; //but the document dont say what should be return
  lock_acquire(&lock_for_fs);
  position = (unsigned)file_tell(fd->file);
  lock_release(&lock_for_fs);
  return position;
}

void sys_close(int fd_num)
{
  struct file_descriptor *fd = get_fd(thread_current(), fd_num);
  if (!fd)
    return;
  lock_acquire(&lock_for_fs);
  file_close(fd->file);
  lock_release(&lock_for_fs);
  list_remove(&fd->elem);
  free(fd);
}

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user(const uint8_t *uaddr)
{
  int result;
  asm("movl $1f, %0; movzbl %1, %0; 1:"
      : "=&a"(result)
      : "m"(*uaddr));
  return result;
}

/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user(uint8_t *udst, uint8_t byte)
{
  int error_code;
  asm("movl $1f, %0; movb %b2, %1; 1:"
      : "=&a"(error_code), "=m"(*udst)
      : "q"(byte));
  return error_code != -1;
}

//VM

#ifdef VM
mapid_t sys_mmap(int fd, void *addr)
{

  if (addr == NULL || pg_ofs(addr) != 0)
    return -1;
  if (fd < 3)
    return -1;
  struct thread *curr = thread_current();

  lock_acquire(&lock_for_fs);

  struct file *f = NULL;
  struct file_descriptor *file_d = get_fd(thread_current(), fd);
  if (file_d && file_d->file)
  {
    // reopen file and  store in the mmap_desc struct at munmap
    f = file_reopen(file_d->file);
  }
  if (f == NULL || file_length(f) == 0)
  {
    lock_release(&lock_for_fs);
    return -1;
  }
  size_t size = file_length(f);

  //all page addr is not exist
  size_t offset;
  for (offset = 0; offset < size; offset += PGSIZE)
  { //PGSIZE 4096
    void *r_addr = addr + offset;
    if (vm_supt_has_entry(curr->supt, r_addr))
    {
      lock_release(&lock_for_fs);
      return -1;
    }
  }

  // then map to the file system
  for (offset = 0; offset < size; offset += PGSIZE)
  {
    void *r_addr = addr + offset;

    size_t read_bytes = (offset + PGSIZE < size ? PGSIZE : size - offset);
    size_t zero_bytes = PGSIZE - read_bytes; //yong 0 tianman

    vm_supt_install_filesys(curr->supt, r_addr,
                            f, offset, read_bytes, zero_bytes, true);
  }

  mapid_t mapping;
  if (!list_empty(&curr->mmap_list))
  {
    mapping = list_entry(list_back(&curr->mmap_list), struct mmap_desc, elem)->id + 1;
  }
  else
    mapping = 1;

  struct mmap_desc *mmap_d = (struct mmap_desc *)malloc(sizeof(struct mmap_desc));
  mmap_d->id = mapping;
  mmap_d->file = f;
  mmap_d->vaddr = addr;
  mmap_d->size = size;
  list_push_back(&curr->mmap_list, &mmap_d->elem);

  lock_release(&lock_for_fs);
  return mapping;

  NOT_REACHED();
}

bool sys_munmap(mapid_t mapping)
{
  struct thread *curr = thread_current();
  struct mmap_desc *mmap_d = find_mmap_desc(curr, mapping);

  if (mmap_d == NULL)
  {
    return false; // or fail_invalid_access() ?
  }

  lock_acquire(&lock_for_fs);
  size_t offset, file_size = mmap_d->size;
  for (offset = 0; offset < file_size; offset += PGSIZE)
  {
    void *addr = mmap_d->vaddr + offset;
    size_t bytes = (offset + PGSIZE < file_size ? PGSIZE : file_size - offset);
    vm_supt_mm_unmap(curr->supt, curr->pagedir, addr, mmap_d->file, offset, bytes);
  }

  // Free resources, and remove from the list
  list_remove(&mmap_d->elem);
  file_close(mmap_d->file);
  free(mmap_d);

  lock_release(&lock_for_fs);

  return true;
}

#endif





#ifdef VM
static struct mmap_desc* find_mmap_desc(struct thread *t, mapid_t mapping)
{
  ASSERT (t != NULL);

  struct list_elem *e;

  if (! list_empty(&t->mmap_list)) {
    for(e = list_begin(&t->mmap_list);
        e != list_end(&t->mmap_list); e = list_next(e))
    {
      struct mmap_desc *desc = list_entry(e, struct mmap_desc, elem);
      if(desc->id == mapping) {
        return desc;
      }
    }
  }

  return NULL; // not found
}


void preload_and_pin_pages(const void *buffer, size_t size)
{
  struct supplemental_page_table *supt = thread_current()->supt;
  uint32_t *pagedir = thread_current()->pagedir;

  void *upage;
  for(upage = pg_round_down(buffer); upage < buffer + size; upage += PGSIZE)
  {
    vm_load_page (supt, pagedir, upage);
    vm_pin_page (supt, upage);
  }
}

void unpin_preloaded_pages(const void *buffer, size_t size)
{
  struct supplemental_page_table *supt = thread_current()->supt;

  void *upage;
  for(upage = pg_round_down(buffer); upage < buffer + size; upage += PGSIZE)
  {
    vm_unpin_page (supt, upage);
  }
}

#endif 



/* modified by YN  end*/

//printf ("system call!\n");
//thread_exit ();
