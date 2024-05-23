#include "syscalls.h"
#include "printk.h"
#include "defs.h"

extern struct task_struct* current; 
extern void __ret_from_fork();      
extern void* memcpy(void*,void*,uint64);
extern void* memset(void*,int,uint64);
extern struct task_struct* task[NR_TASKS]; 
extern uint64  swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));
extern int IsMapped(uint64* ptb,uint64 addr);
extern create_mapping_sub(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, uint64 perm);

void syscall(struct pt_regs* regs){
    
    if(regs->reg[17 - 1] == SYS_WRITE){
        uint64_t n = regs->reg[12 - 1];
        if(regs->reg[10 - 1] == 1){ 
         char *buff = (char *)regs->reg[11 - 1];  
         uint64_t i;
         for(i = 0;i < n;i++)     
           printk("%c",buff[i]);         
         regs->reg[10 - 1] = n;
        } else regs->reg[10 - 1] = 0;
        
    }else if(regs->reg[17 - 1] == SYS_GETPID){
        regs->reg[10 - 1] = current->pid;
    }else if(regs->reg[17-1] == SYS_CLONE){
     /*
     1. 参考 task_init 创建一个新的 task，将的 parent task 的整个页复制到新创建的 
        task_struct 页上(这一步复制了哪些东西?）。将 thread.ra 设置为 
        __ret_from_fork，并正确设置 thread.sp
        (仔细想想，这个应该设置成什么值?可以根据 child task 的返回路径来倒推)
     
     2. 利用参数 regs 来计算出 child task 的对应的 pt_regs 的地址，
        并将其中的 a0, sp, sepc 设置成正确的值(为什么还要设置 sp?)

     3. 为 child task 申请 user stack，并将 parent task 的 user stack 
        数据复制到其中。 (既然 user stack 也在 vma 中，这一步也可以直接在 5 中做，无需特殊处理)

     3.1. 同时将子 task 的 user stack 的地址保存在 thread_info->
        user_sp 中，如果你已经去掉了 thread_info，那么无需执行这一步

     4. 为 child task 分配一个根页表，并仿照 setup_vm_final 来创建内核空间的映射

     5. 根据 parent task 的页表和 vma 来分配并拷贝 child task 在用户态会用到的内存

     6. 返回子 task 的 pid
    */
      struct task_struct* child = (struct task_struct*)kalloc();
      memcpy(child,current,PGSIZE);
      child->thread.ra = (uint64)(&__ret_from_fork);
      int i;
      for(i = 1;i < NR_TASKS;i++) if(task[i] == NULL) break;
      child->pid = i;
      task[i] = child;
      uint64 offset = PGOFFSET(((uint64)regs));
      struct pt_regs* child_regs = (struct pt_regs* )(child + offset);
      child->thread.sp = (uint64)child_regs;
      memcpy(child_regs,regs,sizeof(struct pt_regs));
      child_regs->reg[10-1] = 0;
      child_regs->reg[2-1] = (uint64)child_regs;
      child_regs->sepc = regs->sepc + 4;
      uint64 u_stack = kalloc();
      memcpy(u_stack,USER_END - PGSIZE, PGSIZE);
      pagetable_t pgtbl = kalloc();
      memset(pgtbl,0,PGSIZE);
      memcpy(pgtbl,swapper_pg_dir,PGSIZE);
      create_mapping_sub(pgtbl,USER_END - PGSIZE,u_stack - PA2VA_OFFSET, PGSIZE, 0x17);
      uint64_t mode = 8;
      mode = mode << 60;
      child->pgd = mode | ((unsigned long)pgtbl - PA2VA_OFFSET) >> 12;
      int j;
      for(j = 0;j < current->vma_cnt;j++){
        struct vm_area_struct* vma = &(current->vmas[j]);
        uint64 addr = vma->vm_start;
        while ((addr < vma->vm_end))
        {
         uint64* cur_pgtbl = ((uint64)current->pgd << 12) + PA2VA_OFFSET;
         if(IsMapped(cur_pgtbl,addr)){
         uint64 page = kalloc();   
         create_mapping_sub(pgtbl,PGROUNDDOWN(addr),page - PA2VA_OFFSET,PGSIZE,(vma->vm_flags & (~(uint64_t)VM_ANONYM)) | 0x11);
         memcpy(page,PGROUNDDOWN(addr),PGSIZE);  
        }
         addr += PGSIZE;
        }  
      }
      printk("[S-MODE] New task:PID = %d\n",i);
      regs->reg[10-1] = i;
    }

}