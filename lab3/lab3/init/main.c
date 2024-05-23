#include "printk.h"
#include "sbi.h"
#include "defs.h"

extern void test();
extern _stext;
extern _srodata;
int start_kernel() {
    printk("2022");
    printk(" Hello RISC-V\n");
   // csr_read(sstatus);
   // csr_write(sscratch,0x1234);
   // printk("_stext: %x\n",&_stext);
    //printk("_srodata: %x\n",&_srodata);
   // _srodata = 0;
   // printk("_srodata: %x\n",&_srodata);
    test(); // DO NOT DELETE !!!

	return 0;
}
