#include "sbi.h"
#include "printk.h"
#include "syscall.h"
#include "defs.h"

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
       printk("[S-MODE] Supervisor Mode Timer Interrupt\n");
       }
    else if(scause == 0x8){
        syscall(regs);
        regs->sepc += 4;
    }
    
    // YOUR CODE HERE
}