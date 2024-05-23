//arch/riscv/kernel/proc.c
#include "proc.h"
#include "mm.h"
#include "defs.h"
#include "rand.h"
#include "printk.h"
#include <string.h>
#include "test.h"
#include "test_schedule.h"
#include "elf.h"

// gdb-multiarch ~/myoslab/lab5/vmlinux
//arch/riscv/kernel/proc.c

extern void __dummy();

struct task_struct* idle;           // idle process
struct task_struct* current;        // 指向当前运行线程的 `task_struct`
struct task_struct* task[NR_TASKS]; // 线程数组, 所有的线程都保存在此

/**
 * new content for unit test of 2023 OS lab2
*/
extern uint64 task_test_priority[]; // test_init 后, 用于初始化 task[i].priority 的数组
extern uint64 task_test_counter[];  // test_init 后, 用于初始化 task[i].counter  的数组
extern unsigned long swapper_pg_dir[512];
extern create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, uint64 perm);
extern create_mapping_sub(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, uint64 perm);
extern char _sramdisk[];
extern char _eramdisk[];

static uint64_t load_program(struct task_struct* task) {
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)_sramdisk;

    uint64_t phdr_start = (uint64_t)ehdr + ehdr->e_phoff;
    int phdr_cnt = ehdr->e_phnum;

    Elf64_Phdr* phdr;
    int load_phdr_cnt = 0;

    pagetable_t pgtbl = (pagetable_t)kalloc();
    memcpy((void *)(pgtbl), (void *)(&swapper_pg_dir), PGSIZE);

    for (int i = 0; i < phdr_cnt; i++) {
        phdr = (Elf64_Phdr*)(phdr_start + sizeof(Elf64_Phdr) * i);
        //if (phdr->p_type == PT_LOAD) {
            // alloc space and copy content
            // do mapping
            uint64_t pgnum = (PGOFFSET(phdr->p_vaddr) + phdr->p_memsz + PGSIZE - 1)/PGSIZE;
            //uint64_t pages = alloc_pages(pgnum);
            //uint64_t start_addr = (uint64_t)(_sramdisk + phdr->p_offset);
            //memcpy((void *)(pages),(void *)start_addr,phdr->p_memsz);
            //create_mapping_sub((uint64_t *)pgtbl,PGROUNDDOWN(phdr->p_vaddr),(pages - PA2VA_OFFSET),pgnum*PGSIZE,phdr->p_flags);
            uint64_t flag = 0;
            if(phdr->p_flags & PF_X)  flag |= VM_X_MASK;
            if(phdr->p_flags & PF_W)  flag |= VM_W_MASK; 
            if(phdr->p_flags & PF_R)  flag |= VM_R_MASK;
            do_mmap(task, (uint64_t)(phdr->p_vaddr), pgnum*PGSIZE, flag, phdr->p_offset, phdr->p_filesz);
        //}
    }   

    // allocate user stack and do mapping
    // code...

    // following code has been written for you
    // set user stack 
    //uint64_t va,pa;
    //va = USER_END - PGSIZE;
    //pa = (uint64_t)alloc_page() - PA2VA_OFFSET;
    //create_mapping_sub(pgtbl,va,pa,PGSIZE,23);
    do_mmap(task, USER_END - PGSIZE, PGSIZE, VM_R_MASK | VM_W_MASK | VM_ANONYM, 0, 0);
    uint64_t mode = 8;
    mode = mode << 60;
    task->pgd = mode | ((unsigned long)pgtbl - PA2VA_OFFSET) >> 12;
    //task->pgd = (unsigned long)pgtbl - PA2VA_OFFSET;
    // pc for the user program
    task->thread.sepc = ehdr->e_entry;
    // sstatus bits set
    task->thread.sstatus = (csr_read(sstatus) | 0x40020) & 0xfffffffffffffeff;
    // user stack for user program
    task->thread.sscratch = USER_END;
}


void task_init() {
    test_init(NR_TASKS);
    // 1. 调用 kalloc() 为 idle 分配一个物理页
    // 2. 设置 state 为 TASK_RUNNING;
    // 3. 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
    // 4. 设置 idle 的 pid 为 0
    // 5. 将 current 和 task[0] 指向 idle
    /* YOUR CODE HERE */
    idle = (struct task_struct*)kalloc();
    idle->state = TASK_RUNNING;
    idle->counter = 0;
    idle->priority = 0;
    idle->pid = 0;
    current = idle;
    task[0] = idle;
    idle->thread.sscratch = 0;
    idle->vma_cnt = 0;
    idle->pgd = (pagetable_t)swapper_pg_dir - PA2VA_OFFSET;

    // 1. 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
    // 2. 其中每个线程的 state 为 TASK_RUNNING, 此外，为了单元测试的需要，counter 和 priority 进行如下赋值：
    //      task[i].counter  = task_test_counter[i];
    //      task[i].priority = task_test_priority[i];
    // 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`,
    // 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址,  `sp` 设置为 该线程申请的物理页的高地址
    /* YOUR CODE HERE */
    int i;
    for(i = 1;i < 2;i++)
    {
      task[i] = (struct task_struct*)kalloc();
      task[i]->state = TASK_RUNNING;
      task[i]->counter = task_test_counter[i];
      task[i]->priority = task_test_priority[i];
      task[i]->pid = i;
      task[i]->thread.ra = (uint64)(&__dummy);
      task[i]->thread.sp = (uint64)(task[i]) + PGSIZE - 1;
      task[i]->vma_cnt = 0;
      #ifdef ELF
      load_program(task[i]);
      #endif
      
      #ifdef BIN
      pagetable_t pgtbl = (pagetable_t)kalloc();
      memcpy((void *)(pgtbl), (void *)(&swapper_pg_dir), PGSIZE);
      /* uint64_t va,pa;
      va = USER_START;
      pa = (unsigned long)(_sramdisk)-PA2VA_OFFSET;
      create_mapping_sub(pgtbl, va, pa, (unsigned long)(_eramdisk)-(unsigned long)(_sramdisk), 31);
      va = USER_END - PGSIZE;
      pa = (uint64_t)alloc_page() - PA2VA_OFFSET;
      create_mapping_sub(pgtbl,va,pa,PGSIZE,23); */
      do_mmap(task[i], USER_START, _eramdisk - _sramdisk, VM_R_MASK | VM_W_MASK | VM_X_MASK, 0, _eramdisk - _sramdisk);
      do_mmap(task[i], USER_END - PGSIZE, PGSIZE, VM_R_MASK | VM_W_MASK | VM_ANONYM, 0, PGSIZE);
      uint64_t mode = 8;
      mode = mode << 60;
      task[i]->pgd = mode | ((unsigned long)pgtbl-PA2VA_OFFSET) >> 12;
      task[i]->thread.sepc = USER_START;
      task[i]->thread.sstatus = (csr_read(sstatus) | 0x40020) & 0xfffffffffffffeff;
      task[i]->thread.sscratch = USER_END;
      #endif 

    } 
    for(i = 2;i < NR_TASKS;i++) task[i] = NULL;

    printk("...proc_init done!\n");
}

// arch/riscv/kernel/proc.c
void dummy() {
    schedule_test();
    uint64 MOD = 1000000007;
    uint64 auto_inc_local_var = 0;
    int last_counter = -1;
    while(1) {
        if ((last_counter == -1 || current->counter != last_counter) && current->counter > 0) {
            if(current->counter == 1){
                --(current->counter);   // forced the counter to be zero if this thread is going to be scheduled
            }                           // in case that the new counter is also 1, leading the information not printed.
            last_counter = current->counter;
            auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
            printk("[PID = %d] is running. auto_inc_local_var = %d\n", current->pid, auto_inc_local_var);
        }
    }
}

extern void __switch_to(struct task_struct* prev, struct task_struct* next);


void switch_to(struct task_struct* next) {
    /* YOUR CODE HERE */
    if(current != next){ 
    #ifdef SJF
        printk("[S-MODE] switch to [PID = %d COUNTER = %d]\n", next->pid, next->counter);
    #endif 
    #ifdef PRIORITY
        //printk("PRIORITY schedule!\n");
        printk("switch to [PID = %d PRIORITY = %d COUNTER = %d]\n", next->pid, next->priority, next->counter);
    #endif
    struct task_struct* old = current;
    current = next;
    __switch_to(old,next); 
      }
    return;
}

void do_timer(void) {
    // 1. 如果当前线程是 idle 线程 直接进行调度
    // 2. 如果当前线程不是 idle 对当前线程的运行剩余时间减1 若剩余时间仍然大于0 则直接返回 否则进行调度
    /* YOUR CODE HERE */
    if(current == idle)
    {
       schedule();
    }else{
        current->counter--;
     if(current->counter > 0)  return;
     else{
       schedule();
     }      
    }
}


void schedule(void) {
    //短作业优先调度算法
    /* YOUR CODE HERE */
    #ifdef SJF
     uint64 min = 0xFFFFFFFFFFFFFFFF,min_index = -1;
    for(int i=1;i<NR_TASKS;i++){
        if(task[i] != NULL && task[i]->counter < min && task[i]->counter > 0){
            min = task[i]->counter;
            min_index = i;
        }
    }
    if(min_index == -1 ){
        for(int i=1;i<NR_TASKS;i++){
            if(task[i] != NULL){
                task[i]->counter = rand()%10+3;
                task[i]->priority = rand()%10+1;
            }
        }
        schedule();
    }else{
        switch_to(task[min_index]);
    }
    #endif

    //优先级调度算法
    /* YOUR CODE HERE */
    #ifdef  PRIORITY 
    //printk("PRIORITY schedule!\n");
    int i;
    struct task_struct* next = current;
    while(1)
    { 
      uint64 max = 0;
      i = NR_TASKS;
      while(--i)
      {
        if(!task[i])
            continue;
        if(task[i]->state == TASK_RUNNING && task[i]->counter > max)
            max = task[i]->counter, next = task[i];
      }
      if(max) break;
      for(i = NR_TASKS - 1;i > 0;i--)
      {
        if(task[i]) task[i]->counter = (task[i]->counter >> 1) + task[i]->priority;
      }
    }
    switch_to(next);
    #endif
}

void do_mmap(struct task_struct * _task, uint64_t addr, uint64_t length, uint64_t flags,
   uint64_t vm_content_offset_in_file, uint64_t vm_content_size_in_file){
    _task->vma_cnt++;
    struct vm_area_struct *new_vma = &_task->vmas[_task->vma_cnt - 1];
    new_vma->vm_start = addr;
    new_vma->vm_end = addr + length;
    new_vma->vm_flags = flags;
    new_vma->vm_content_offset_in_file = vm_content_offset_in_file;
    new_vma->vm_content_size_in_file = vm_content_size_in_file;
}

struct vm_area_struct *find_vma(struct task_struct *_task, uint64_t addr){
   uint64_t i = 0;
   for(i = 0;i < _task->vma_cnt;i++){
     if(_task->vmas[i].vm_start <= addr && addr < _task->vmas[i].vm_end) 
      return &(_task->vmas[i]);
   }
  return NULL;
}