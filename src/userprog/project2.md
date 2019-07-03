		     +--------------------------+
       	     |      	CS 318  		|
		     | PROJECT 2: USER PROGRAMS	|
		     | 	   DESIGN DOCUMENT     	|
		     +--------------------------+

---- GROUP ----

>> Fill in the names and email addresses of your group members.

李洛玄
卢宇欣
杨宁
朱茂生

---- PRELIMINARIES ----

系统版本 Ubuntu 16.04 gcc 5.4.0

---- DATA STRUCTURES ----

>> A1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration.  Identify the purpose of each in 25 words or less.

1.  struct pro_entry                于process.c中，用于记录各进程的各数据及维护一个表来记录他们
2.  struct file_descriptor          于syscall.h中，用于记录文件描述符及其对应进程

---- ALGORITHMS ----

>> A2: Briefly describe how you implemented argument parsing.  How do
>> you arrange for the elements of argv[] to be in the right order?
>> How do you avoid overflowing the stack page?

通过strtok_r()来分离文件名和参数，然后使用栈指针寄存器esp，按顺序压入栈中（同时保证对齐）；
在取出时，args[esp + 4]为文件名，之后根据syscall类型加载不同数量的参数。

在压入和取出时均做边界检查，保证不越界。



---- RATIONALE ----

>> A3: Why does Pintos implement strtok_r() but not strtok()?

strtok() 会在提取字符串时使用静态缓冲区，因此其实线程不安全的；
相应地，strtok_r() 为线程安全。为了保证线程的安全性，我们使用strtok_r()。


>> A4: In Pintos, the kernel separates commands into a executable name
>> and arguments.  In Unix-like systems, the shell does this
>> separation.  Identify at least two advantages of the Unix approach.

1.减少内核操作，给予shell更多权力；
2.更容易保证线程安全。


			     SYSTEM CALLS
			     ============

---- DATA STRUCTURES ----

>> B1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration.  Identify the purpose of each in 25 words or less.
见A1

>> B2: Describe how file descriptors are associated with open files.
>> Are file descriptors unique within the entire OS or just within a
>> single process?      single process

---- ALGORITHMS ----

>> B3: Describe your code for reading and writing user data from the
>> kernel.
对读/写：首先判断其对应的文件描述符是否合法，若合法，则首先加锁保证线程安全；
然后进行对应的读写（调用file_read/file_write） 并返回相应的参数，
记录在创建的struct里面，最后释放锁，结束并返回参数。


>> B4: Suppose a system call causes a full page (4,096 bytes) of data
>> to be copied from user space into the kernel.  What is the least
>> and the greatest possible number of inspections of the page table
>> (e.g. calls to pagedir_get_page()) that might result?  What about
>> for a system call that only copies 2 bytes of data?  Is there room
>> for improvement in these numbers, and how much?


>> B5: Briefly describe your implementation of the "wait" system call
>> and how it interacts with process termination.
wait 调用process_wait() 来进行， 利用struct pro_entry寻找相应的children，
并且取得其tid，检测其是否在等待，最后等待其结束。在此过程中，提高子进程优先级
保证线程安全（主进程不会先结束）。

>> B6: Any access to user program memory at a user-specified address
>> can fail due to a bad pointer value.  Such accesses must cause the
>> process to be terminated.  System calls are fraught with such
>> accesses, e.g. a "write" system call requires reading the system
>> call number from the user stack, then each of the call's three
>> arguments, then an arbitrary amount of user memory, and any of
>> these can fail at any point.  This poses a design and
>> error-handling problem: how do you best avoid obscuring the primary
>> function of code in a morass of error-handling?  Furthermore, when
>> an error is detected, how do you ensure that all temporarily
>> allocated resources (locks, buffers, etc.) are freed?  In a few
>> paragraphs, describe the strategy or strategies you adopted for
>> managing these issues.  Give an example.

主要在进程正常退出时进行比较全面的检查，异常退出/page fault时进行相应调用

---- SYNCHRONIZATION ----

>> B7: The "exec" system call returns -1 if loading the new executable
>> fails, so it cannot return before the new executable has completed
>> loading.  How does your code ensure this?  How is the load
>> success/failure status passed back to the thread that calls "exec"?
加锁来保证程序的先后正确性。创建成功thread_create 会返回 tid, 否则返回-1

>> B8: Consider parent process P with child process C.  How do you
>> ensure proper synchronization and avoid race conditions when P
>> calls wait(C) before C exits?  After C exits?  How do you ensure
>> that all resources are freed in each case?  How about when P
>> terminates without waiting, before C exits?  After C exits?  Are
>> there any special cases?
因为使用了pro_entry来记录，所以在wait时可以知道C是否存在，不存在则返回-1；
使用了锁和优先级改变来保证父进程和子进程的同步。

---- RATIONALE ----

>> B9: Why did you choose to implement access to user memory from the
>> kernel in the way that you did?
维护一个表比较方便，好解决上面一系列问题

>> B10: What advantages or disadvantages can you see to your design
>> for file descriptors?
优点：好操作，什么都能记录下来
缺点：占用空间不小。

>> B11: The default tid_t to pid_t mapping is the identity mapping.
>> If you changed it, what advantages are there to your approach?
没有更改他，尽管觉得更改之后不容易出现不同线程之间的混淆，但囿于实际情况放弃。