#include "klib.h"

size_t strlen(const char *s) {
  assert(s);
  size_t sz = 0;
  for (; *s; s ++) {
    sz ++;
  }
  return sz;
}

char* strcpy(char* dst,const char* src){
  assert(src&&dst);
  char* ret;
  ret=dst;
  while((*dst++=*src++)!='\0');
  return ret;
}
char* strncpy(char* dst, const char* src, size_t n){
  assert(src&&dst);
  char* ret;
  ret=dst;
  while(n-->0){
    *dst++=*src++;
  }
  return ret;
}

char* strcat(char* dst, const char* src){
  assert(dst&&src);
  char* d=dst;
  while(*++dst!='\0');
  while((*dst++=*src++)!='\0');
  return d;
}
int strcmp(const char* s1, const char* s2){
  assert(s1&&s2);
  while(*s1&&*s1==*s2)s1++,s2++;
  return (int)(*s1-*s2);
}
int strncmp(const char* s1, const char* s2, size_t n){
  assert(s1&&s2);
  while(--n>0&&*s1&&*s1==*s2)s1++,s2++;
  return (int)(*s1-*s2);
}


void* memset(void* v,int c,size_t n){
  assert(v);

  void* ori_dst = v;
  c &= 0xff;
  int cc = c | (c << 8);
  int cccc = cc | (cc << 16);

  int res = n & 0x3;
  n >>= 2;
  int i;
  for (i = 0; i < n; i ++) {
    ((int *)v)[i] = cccc;
  }

  i <<= 2;
  v += i;
  switch (res) {
    case 3: ((char *)v)[2] = c;
    case 2: ((char *)v)[1] = c;
    case 1: ((char *)v)[0] = c;
    default: ;
  }

  return ori_dst;
}

void* memmove(void* dst,const void* src,size_t n){
  assert(dst&&src);
  const char* s;
  char* d;
  if(src+n>dst&&src<dst){
    s=src+n;
    d=dst+n;
    while(n-->0)*--d=*--s;
  }
  else{
    s=src;
    d=dst;
    while(n-->0)*d++=*s++;
  }
  return dst;
}
void* memcpy(void* dst, const void* src, size_t n){
  assert(dst&&src);

  void* ori_dst = dst;
  int res = n & 0x3;
  n >>= 2;
  int i;
  for (i = 0; i < n; i ++) {
    ((int *)dst)[i] = ((int *)src)[i];
  }

  i <<= 2;
  dst += i;
  src += i;
  switch (res) {
    case 3: ((char *)dst)[2] = ((char *)src)[2];
    case 2: ((char *)dst)[1] = ((char *)src)[1];
    case 1: ((char *)dst)[0] = ((char *)src)[0];
    default: ;
  }

  return ori_dst;
}
int memcmp(const void* s1, const void* s2, size_t n){
  return strncmp(s1,s2,n);
}

