#include "sbi.h"
#include "printk.h"
#include "syscalls.h"
#include "defs.h"
#include "string.h"
#include "stdint.h"

extern struct task_struct* current;  
extern create_mapping_sub(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, uint64 perm);
extern char _sramdisk[];

void do_page_fault(struct pt_regs *regs) {
    
     //1. 通过 stval 获得访问出错的虚拟内存地址（Bad Address）
     //2. 通过 find_vma() 查找 Bad Address 是否在某个 vma 中
     //3. 分配一个页，将这个页映射到对应的用户地址空间
     //4. 通过 (vma->vm_flags & VM_ANONYM) 获得当前的 VMA 是否是匿名空间
     //5. 根据 VMA 匿名与否决定将新的页清零或是拷贝 uapp 中的内容
    uint64 bad_addr = regs->stval;  
    struct vm_area_struct *vma = find_vma(current,bad_addr);   
    if(vma != NULL){  
        uint64 new_addr = kalloc();
        unsigned long* pgtbl = (((unsigned long)current->pgd & 0xfffffffffff) << 12) + PA2VA_OFFSET;
        create_mapping_sub(pgtbl,bad_addr,new_addr-PA2VA_OFFSET,PGSIZE,(vma->vm_flags & (~(uint64_t)VM_ANONYM)) | 0x11);
        if( !(vma->vm_flags & VM_ANONYM) ){
            uint64 src_addr = (uint64)(_sramdisk) + vma->vm_content_offset_in_file;
            uint64 offset = (uint64)(bad_addr) - PGROUNDDOWN(bad_addr);
            uint64 sz = (vma->vm_start + vma->vm_content_size_in_file > bad_addr) ? PGSIZE - offset : 0;
            memcpy(new_addr+offset,src_addr,sz);
      } 
    }
}

void trap_handler(uint64_t scause, uint64_t sepc, struct pt_regs *regs) {
    // 通过 `scause` 判断trap类型
    // 如果是interrupt 判断是否是timer interrupt
    // 如果是timer interrupt 则打印输出相关信息, 并通过 `clock_set_next_event()` 设置下一次时钟中断
    // `clock_set_next_event()` 见 4.3.4 节
    // 其他interrupt / exception 可以直接忽略
    if(scause==0x8000000000000005) 
       {
       clock_set_next_event();
       do_timer();
       //printk("[S-MODE] Supervisor Mode Timer Interrupt\n");
       }
    else if(scause == 0x8){
        syscall(regs);
        regs->sepc += 4;
    } else if(scause == 0xc || scause == 0xd || scause == 0xf) {
        do_page_fault(regs);
        printk("[S-MODE] Page Fault Interrupt, ");
        printk("scause: %lx, ", scause);
        printk("stval: %lx, ", regs->stval);
        printk("sepc: %lx\n", regs->sepc);
    } else {
        printk("[S-MODE] Unhandled trap, ");
        printk("scause: %lx, ", scause);
        printk("stval: %lx, ", regs->stval);
        printk("sepc: %lx\n", regs->sepc);
        while (1);
    }   
    // YOUR CODE HERE
}
