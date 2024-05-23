#ifndef __SYS__
#define __SYS__

#include "stdint.h"
#include "proc.h"
#define SYS_WRITE   64
#define SYS_GETPID  172
#define SYS_CLONE   220

void syscall(struct pt_regs* regs);

#endif