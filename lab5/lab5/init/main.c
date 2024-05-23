#include "printk.h"
#include "sbi.h"
#include "defs.h"

extern void test();
extern int schedule();
extern _stext;
extern _srodata;
int start_kernel() {
    printk("[S-MODE] 2022");
    printk(" Hello RISC-V\n");
  
    schedule();
    test(); // DO NOT DELETE !!!

	return 0;
}
