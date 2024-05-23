#include "printk.h"
#include "sbi.h"
#include "defs.h"

extern void test();

int start_kernel() {
    printk("2022");
    printk(" Hello RISC-V\n");
   // csr_read(sstatus);
   // csr_write(sscratch,0x1234);
    test(); // DO NOT DELETE !!!

	return 0;
}
