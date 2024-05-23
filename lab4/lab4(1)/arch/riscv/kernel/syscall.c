#include "syscall.h"
#include "printk.h"

extern struct task_struct* current; 

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
    }

}