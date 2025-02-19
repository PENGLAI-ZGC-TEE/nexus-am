/*
 * The Nexus Abstract Machine Architecture (AM)
 * A portable abstraction of a bare-metal computer
 */

#ifndef __AM_H__
#define __AM_H__

#include <stdint.h>
#include <stddef.h>
#include ARCH_H // "arch/x86-qemu.h", "arch/native.h", ...

#ifdef __cplusplus
extern "C" {
#endif

// ===================== Constants and Structs =======================

enum {
  _EVENT_NULL = 0,
  _EVENT_ERROR,
  _EVENT_IRQ_SOFT,
  _EVENT_IRQ_TIMER,
  _EVENT_IRQ_IODEV,
  _EVENT_PAGEFAULT,
  _EVENT_SYSCALL,
  _EVENT_YIELD,
};

enum {
  _PROT_NONE  = 0x1, // no access
  _PROT_READ  = 0x2, // can read
  _PROT_WRITE = 0x4, // can write
  _PROT_EXEC  = 0x8, // can execute
};

// Memory area for [@start, @end)
typedef struct _Area {
  void *start, *end;
} _Area;

// An event of type @event, caused by @cause of pointer @ref
typedef struct _Event {
  int event;
  uintptr_t cause, ref;
  const char *msg;
} _Event;

// Arch-dependent processor context
typedef struct _Context _Context;
typedef struct _Context_m _Context_m;

// A protected address space with user memory @area
// and arch-dependent @ptr
typedef struct _AddressSpace {
  size_t pgsize;
  _Area area;
  void *ptr;
} _AddressSpace;

// ====================== Turing Machine (TRM) =======================

extern _Area _heap;
void _putc(char ch);
void _halt(int code) __attribute__((__noreturn__));

// ======================= I/O Extension (IOE) =======================

int    _ioe_init();
size_t _io_read (uint32_t dev, uintptr_t reg, void *buf, size_t size);
size_t _io_write(uint32_t dev, uintptr_t reg, void *buf, size_t size);

// ====================== Context Extension (CTE) ====================

int  _cte_init(_Context *(*handler)(_Event ev, _Context *ctx));
void _yield();
int  _intr_read();
void _intr_write(int enable);
_Context* _kcontext(_Area kstack, void (*entry)(void *), void *arg);

// ================= Virtual Memory Extension (VME) ==================

int  _vme_init(void *(*pgalloc)(size_t size), void (*pgfree)(void *));
int  _vme_init_custom(void *(*pgalloc)(size_t size), void (*pgfree)(void *), _Area * custom_segments, int len);
void _protect(_AddressSpace *as);
void _unprotect(_AddressSpace *as);
void _map(_AddressSpace *as, void *va, void *pa, int prot);
_Context *_ucontext(_AddressSpace *as, _Area kstack, void *entry);

// a fault map for xiangshan testing access fault
void _map_fault(_AddressSpace *as, void *va, void *pa, int prot);

// hugepage map for xiangshan testing
void _map_rv_hugepage(_AddressSpace *as, void *va, void *pa, int prot, int pagetable_level);

// ================= Multi-Processor Extension (MPE) =================

int _mpe_init(void (*entry)());
int _ncpu();
int _cpu();
intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval);

#ifdef __cplusplus
}
#endif

#endif
