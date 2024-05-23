.extern start_kernel

    .section .text.init
    .globl _start
_start:
    # ------------------
    # - your code here -
    # ------------------
    
    # ------------------
    la sp,boot_stack
    
    jal mm_init
    jal task_init  
   

        # set stvec = _traps
    la t0,_traps 
    csrw stvec,t0

    # ------------------

        # set sie[STIE] = 1
    csrr t0,sie
    ori t0,t0,32
    csrw sie,t0

    # ------------------

        # set first time interrupt
    rdtime t1
    li t0,10000000
    add a0,t1,t0
    mv a7,x0
    mv a6,x0
    mv a1,x0
    mv a2,x0
    mv a3,x0
    mv a4,x0
    mv a5,x0
    ecall

    # ------------------

        # set sstatus[SIE] = 1
    csrr t0,sstatus
    ori t0,t0,2                                                                                                                                                                                                                                                                                                                                                                                                                                                       
    csrw sstatus,t0
    
    j  start_kernel 

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096          # <-- change to your stack size

    .globl boot_stack_top
boot_stack_top:

#完成 arch/riscv/kernel/head.S 。
#我们首先为即将运行的第一个 C 函数设置程序栈（栈的大小可以设置为 4KB），
#并将该栈放置在.bss.stack 段。
#接下来我们只需要通过跳转指令，跳转至 main.c 中的 start_kernel 函数即可。