#include "defs.h"
#include "string.h"
#include "mm.h"
#include "types.h"
#include "printk.h"
// gdb-multiarch ~/myoslab/src/lab3/vmlinux
/* early_pgtbl: 用于 setup_vm 进行 1GB 的 映射。 */
unsigned long  early_pgtbl[512] __attribute__((__aligned__(0x1000)));

void setup_vm(void) {
    /* 
    1. 由于是进行 1GB 的映射 这里不需要使用多级页表 
    2. 将 va 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
        high bit 可以忽略
        中间9 bit 作为 early_pgtbl 的 index
        低 30 bit 作为 页内偏移 这里注意到 30 = 9 + 9 + 12， 即我们只使用根页表， 根页表的每个 entry 都对应 1GB 的区域。 
    3. Page Table Entry 的权限 V | R | W | X 位设置为 1
    */   
   memset(early_pgtbl, 0x0, PGSIZE);
   uint64 pa,va;

   pa = PHY_START;    //0x80000000 ~ 0xafffffff
   va = PHY_START;    //0x80000000 ~ 0xafffffff
   uint64 index;
   index = (va >> 30) & 0x1ff;
   early_pgtbl[index] = ((pa & 0x00ffffffc0000000) >> 2) | 0xf;

   va = VM_START;     //0xffffffe000000000 ~ 0xffffffe03fffffff
   index = (va >> 30) & 0x1ff;
   early_pgtbl[index] = ((pa & 0x00ffffffc0000000) >> 2) | 0xf;
   
   printk("...setup_vm done!\n");
}


/* swapper_pg_dir: kernel pagetable 根目录， 在 setup_vm_final 进行映射。 */
unsigned long   swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));
extern uint64 _stext;
extern uint64 _srodata;
extern uint64 _sdata;

void setup_vm_final(void) {
    memset(swapper_pg_dir, 0x0, PGSIZE);

    // No OpenSBI mapping required
    
    // mapping kernel text X|-|R|V
    create_mapping((uint64 *)swapper_pg_dir, (uint64)&_stext, (uint64)(&_stext) - PA2VA_OFFSET, (uint64)(&_srodata) - (uint64)(&_stext), 0xb);

    // mapping kernel rodata -|-|R|V
    create_mapping((uint64 *)swapper_pg_dir, (uint64)&_srodata, (uint64)(&_srodata) - PA2VA_OFFSET, (uint64)(&_sdata) - (uint64)(&_srodata), 0x3);

    // mapping other memory -|W|R|V
    create_mapping((uint64 *)swapper_pg_dir, (uint64)&_sdata, (uint64)(&_sdata) - PA2VA_OFFSET, PHY_SIZE - ((uint64)(&_sdata) - (uint64)(&_stext)), 0x7);

    // set satp with swapper_pg_dir  
  
    // YOUR CODE HERE
    uint64 pgdir = (uint64)swapper_pg_dir - PA2VA_OFFSET;
    asm volatile("li t0,8\n"
                 "slli t0,t0,60\n"
                 "mv t1,%[pgdir]\n"
                 "srli t1,t1,12\n"
                 "or t0,t0,t1\n"
                 "csrw satp,t0"
                 : 
                 : [pgdir]"r"(pgdir)
                 : "memory");

    // flush TLB
    asm volatile("sfence.vma zero, zero");

    // flush icache
    asm volatile("fence.i");
    printk("...setup_vm_final done!\n");
    return;
}


/**** 创建多级页表映射关系 *****/
/* 不要修改该接口的参数和返回值 */
create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, uint64 perm) {
    /*
    pgtbl 为根页表的基地址
    va, pa 为需要映射的虚拟地址、物理地址
    sz 为映射的大小，单位为字节
    perm 为映射的权限 (即页表项的低 8 位)
    创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
    可以使用 V bit 来判断页表项是否存在
    */
   uint64  vpn2,vpn1,vpn0,pte2,pte1,pte0;
   uint64* tbl1;
   uint64* tbl0;
   uint64 end = va + sz;

   while(va < end)
   {
     vpn2 = (va >> 30) & 0x1ff;
     pte2 = pgtbl[vpn2];
     if((pte2 & 0x1) == 0){
        uint64* k = (uint64* )kalloc();
        pte2 = (((uint64)k - PA2VA_OFFSET) >> 12) << 10 | 1;
        pgtbl[vpn2] = pte2;
     }
     tbl1 = (uint64* )(((pte2 >> 10) << 12));
     vpn1 = (va >> 21) & 0x1ff;
     pte1 = tbl1[vpn1];
     if((pte1 & 0x1) == 0){
        uint64* k = (uint64* )kalloc();
        pte1 = (((uint64)k - PA2VA_OFFSET) >> 12) << 10 | 1;
        tbl1[vpn1] = pte1;
     } 
     tbl0 = (uint64* )(((pte1 >> 10) << 12));
     vpn0 = (va >> 12) & 0x1ff;
     pte0 = ((pa >> 12) << 10) | perm;
     tbl0[vpn0] = pte0; 
     va += PGSIZE;
     pa += PGSIZE;
   }
  return;
}