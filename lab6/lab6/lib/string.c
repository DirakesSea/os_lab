#include "string.h"

void *memset(void *dst, int c, uint64 n) {
    char *cdst = (char *)dst;
    for (uint64 i = 0; i < n; ++i)
        cdst[i] = c;

    return dst;
}

void *memcpy(void *dst, void *src, uint64 n){
   uint64 i;
   char *d = (char *)dst;
   char *s = (char *)src;
   for(i = 0;i < n;i++)
     d[i] = s[i];
   return dst; 
}
