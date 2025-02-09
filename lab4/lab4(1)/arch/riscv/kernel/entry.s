.section .text.entry
    .align 2
    .global __dummy
__dummy:
    # YOUR CODE HERE
    mv t0,sp
    csrr sp,sscratch
    csrw sscratch,t0

    sret 

    .globl __switch_to
__switch_to:
    # save state to prev process
    # YOUR CODE HERE

    li    t4,  48 
	add   a3, a0, t4
	add   a4, a1, t4
	# Save context into prev->thread
	sd ra, 0(a3)
    sd sp, 8(a3)
    sd s0, 16(a3)
    sd s1, 24(a3)
    sd s2, 32(a3)
    sd s3, 40(a3)
    sd s4, 48(a3)
    sd s5, 56(a3)
    sd s6, 64(a3)
    sd s7, 72(a3)
    sd s8, 80(a3)
    sd s9, 88(a3)
    sd s10, 96(a3)
    sd s11, 104(a3)
    csrr t5, sepc
    sd t5, 112(a3)
    csrr t5, sstatus
    sd t5, 120(a3)
    csrr t5, sscratch
    sd t5, 128(a3)
    csrr t5, satp
    sd t5, 136(a3)
	# Restore context from next->thread
	# your code
	ld ra, 0(a4)
    ld sp, 8(a4)
    ld s0, 16(a4)
    ld s1, 24(a4)
    ld s2, 32(a4)
    ld s3, 40(a4)
    ld s4, 48(a4)
    ld s5, 56(a4)
    ld s6, 64(a4)
    ld s7, 72(a4)
    ld s8, 80(a4)
    ld s9, 88(a4)
    ld s10, 96(a4)
    ld s11, 104(a4)
    ld t5, 112(a4)
    csrw sepc, t5
    ld t5, 120(a4)
    csrw sstatus, t5
    ld t5, 128(a4)
    csrw sscratch, t5
    ld t5, 136(a4)
    csrw satp, t5


    sfence.vma zero, zero

    # flush icache
    fence.i

	ret

    .globl _traps 
_traps:
    # YOUR CODE HERE
    # -----------
    
    csrr t0,sscratch
    beq t0,x0,_traps_switch
    csrw sscratch,sp
    mv sp,t0
_traps_switch:
        # 1. save 32 registers and sepc to stack
    addi sp, sp, -264
    sd x1, 0(sp)
    sd x2, 8(sp)
    sd x3, 16(sp)
    sd x4, 24(sp)
    sd x5, 32(sp)
    sd x6, 40(sp)
    sd x7, 48(sp)
    sd x8, 56(sp)
    sd x9, 64(sp)
    sd x10, 72(sp)
    sd x11, 80(sp)
    sd x12, 88(sp)
    sd x13, 96(sp)
    sd x14, 104(sp)
    sd x15, 112(sp)
    sd x16, 120(sp)
    sd x17, 128(sp)
    sd x18, 136(sp)
    sd x19, 144(sp)
    sd x20, 152(sp)
    sd x21, 160(sp)
    sd x22, 168(sp)
    sd x23, 176(sp)
    sd x24, 184(sp)
    sd x25, 192(sp)
    sd x26, 200(sp)
    sd x27, 208(sp)
    sd x28, 216(sp)
    sd x29, 224(sp)
    sd x30, 232(sp)
    sd x31, 240(sp)
    csrr t0, sepc
    sd t0, 248(sp)
    csrr t0, sstatus
    sd t0, 256(sp)

    # 2. call trap_handler
    csrr a0, scause
    csrr a1, sepc
    mv a2, sp
    call trap_handler

    # 3. restore sepc and 32 registers (x2(sp) should be restore last) from stack
    ld x1, 0(sp)
    ld x2, 8(sp)
    ld x3, 16(sp)
    ld x4, 24(sp)
    ld x5, 32(sp)
    ld x6, 40(sp)
    ld x7, 48(sp)
    ld x8, 56(sp)
    ld x9, 64(sp)
    ld x10, 72(sp)
    ld x11, 80(sp)
    ld x12, 88(sp)
    ld x13, 96(sp)
    ld x14, 104(sp)
    ld x15, 112(sp)
    ld x16, 120(sp)
    ld x17, 128(sp)
    ld x18, 136(sp)
    ld x19, 144(sp)
    ld x20, 152(sp)
    ld x21, 160(sp)
    ld x22, 168(sp)
    ld x23, 176(sp)
    ld x24, 184(sp)
    ld x25, 192(sp)
    ld x26, 200(sp)
    ld x27, 208(sp)
    ld x28, 216(sp)
    ld x29, 224(sp)
    ld x30, 232(sp)
    ld x31, 240(sp)
    ld t0, 248(sp)
    csrw sepc, t0
    ld t0, 256(sp)
    csrw sstatus, t0
    addi sp, sp, 264

    # -----------

        # 4. return from trap
    csrr t0,sscratch
    beq t0,x0,_traps_end
    csrw sscratch,sp
    mv sp,t0
_traps_end:
    sret
    
    # -----------