#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t sz = 0;
  for (; *s; s ++) {
    sz ++;
  }
  return sz;
}

char *strcpy(char* dst,const char* src) {
  char *ret;
  ret = dst;
  while((*dst++=*src++)!='\0');
  return ret;
}
char* strncpy(char* dst, const char* src, size_t n){
  char* ret;
  ret=dst;
  while(n-->0){
    *dst++=*src++;
  }
  return ret;
}

char* strcat(char* dst, const char* src){
  char* d=dst;
  while(*++dst!='\0');
  while((*dst++=*src++)!='\0');
  return d;
}
int strcmp(const char* s1, const char* s2){
  while(*s1&&*s1==*s2)s1++,s2++;
  return (int)(*s1-*s2);
}
int strncmp(const char* s1, const char* s2, size_t n){
  while(--n>0&&*s1&&*s1==*s2)s1++,s2++;
  return (int)(*s1-*s2);
}


void* memset(void* v,int c,size_t n){
  c &= 0xff;
  uint32_t c2 = (c << 8) | c;
  uint32_t c4 = (c2 << 16) | c2;
  uint64_t c8 = (((uint64_t)c4 << 16) << 16) | c4;

  char *dst = (char *)v;
  const size_t threshold = 32;

  if (n >= threshold) {
    // first let dst aligned by 8 bytes
    int pad = (uintptr_t)dst % 8;
    n -= pad;
    while (pad --) { *dst ++ = c; }

    // loop unrolling
    uint64_t *dst8 = (void *)dst;
    while (n >= threshold) {
      *dst8 ++ = c8;
      *dst8 ++ = c8;
      *dst8 ++ = c8;
      *dst8 ++ = c8;
      n -= threshold;
    }

    // copy the remaining bytes
    dst = (void *)dst8;
  }

  while (n--) { *dst++ = c; }
  return v;
}

void* memmove(void* dst,const void* src,size_t n){
  const char* s;
  char* d;
  if(src+n>dst&&src<dst){
    s=src+n;
    d=dst+n;
    while(n-->0)*--d=*--s;
  }
  else{
    memcpy(dst, src, n);
  }
  return dst;
}

void* memcpy(void* out, const void* in, size_t n) {
  char *dst = (char *)out;
  char *src = (char *)in;
  const size_t threshold = 32;
  int is_align = ((uintptr_t)(dst - src) % 8 == 0);

  if (n >= threshold && is_align) {
    // first let dst aligned by 8 bytes
    int pad = (uintptr_t)dst % 8;
    n -= pad;
    while (pad --) { *dst ++ = *src ++; }

    // loop unrolling
    uint64_t *dst8 = (void *)dst;
    uint64_t *src8 = (void *)src;
    while (n >= threshold) {
      *dst8 ++ = *src8 ++;
      *dst8 ++ = *src8 ++;
      *dst8 ++ = *src8 ++;
      *dst8 ++ = *src8 ++;
      n -= threshold;
    }

    // copy the remaining bytes
    dst = (void *)dst8;
    src = (void *)src8;
  }

  while (n--) { *dst++ = *src++; }
  return out;
}

int memcmp(const void* s1, const void* s2, size_t n){
  return strncmp(s1,s2,n);
}

#endif
