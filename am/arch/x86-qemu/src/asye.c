#include <am-x86.h>
#include <stdarg.h>

static _Context* (*H)(_Event, _Context*) = NULL;

void irq0();
void irq1();
void irq14();
void vec0();
void vec1();
void vec2();
void vec3();
void vec4();
void vec5();
void vec6();
void vec7();
void vec8();
void vec9();
void vec10();
void vec11();
void vec12();
void vec13();
void vec14();
void vecsys();
void irqall();

#define IRQ     T_IRQ0 + 
#define SYSCALL 0x80
#define E_GP    13
#define E_PF    14
#define MSG(m) : ev.msg = m;

void irq_handle(struct TrapFrame *tf) {
  _Context ctx = {
    .eax = tf->eax, .ebx = tf->ebx, .ecx = tf->ecx, .edx = tf->edx,
    .esi = tf->esi, .edi = tf->edi, .ebp = tf->ebp, .esp3 = 0,
    .eip = tf->eip, .eflags = tf->eflags,
    .cs = tf->cs, .ds = tf->ds, .es = tf->es, .ss = 0,
    .ss0 = 0, .esp0 = 0,
  };

  if (tf->cs & DPL_USER) { // interrupt at user code
    ctx.ss = tf->ss;
    ctx.esp3 = tf->esp;
    ctx.ss0 = KSEL(SEG_KDATA);
    ctx.esp0 = (uint32_t)tf + 68;
  } else { // interrupt at kernel code
    ctx.ss0 = KSEL(SEG_KDATA);
    ctx.esp0 = (uint32_t)tf + 60; // the %esp before interrupt
  }

  if (IRQ 0 <= tf->irq && tf->irq < IRQ 32) {
    lapic_eoi();
  }

  _Event ev = {
    .event = _EVENT_NULL,
    .cause = 0, .ref = 0,
    .msg = "(no message)",
  };
  
  switch (tf->irq) {
    case IRQ 0 MSG("timer interrupt (lapic)")
      ev.event = _EVENT_IRQ_TIMER;
      break;
    case IRQ 1 MSG("I/O device IRQ1 (keyboard)")
      ev.event = _EVENT_IRQ_IODEV;
      break;
    case SYSCALL MSG("int $0x80 trap: _yield() or system call")
      if ((int32_t)tf->eax == -1) {
        ev.event = _EVENT_YIELD;
      } else {
        ev.event = _EVENT_SYSCALL;
      }
      break;
    case E_PF MSG("page fault, @cause: _PROT_XXX")
      ev.event = _EVENT_PAGEFAULT;
      if (tf->err & 0x1) ev.cause |= _PROT_NONE;
      if (tf->err & 0x2) ev.cause |= _PROT_WRITE;
      else               ev.cause |= _PROT_READ;
      ev.ref = get_cr2();
      break;
    case E_GP MSG("GP #13, general protection failure")
      ev.event = _EVENT_ERROR; break;
    // TODO: add other exceptions
    default:
      panic("unexpected interrupt/exception");
      break;
  }

  _Context *ret = &ctx;
  if (H) {
    _Context *next = H(ev, &ctx);
    if (next != NULL) {
      ret = next;
    }
  }

  // TODO: move them to assembly
  if (ret->cs & DPL_USER) {
    cpu_setustk(ret->ss0, ret->esp0);
    // return to user
    asm volatile(
      "nop;"
      "movl %0, %%esp;" // move stack
      "push %1;" // SS
      "push %2;" // ESP
      "push %3;" // EFL
      "push %4;" // CS
      "push %5;" // EIP
      "push %6;" // DS
      "push %7;" // ES
      "push %8;" // EAX  - popal
      "push %9;" // ECX
      "push %10;" // EDX
      "push %11;" // EBX
      "push %12;" // ESP
      "push %13;" // EBP
      "push %14;" // ESI
      "push %15;" // EDI -
      "popal;"
      "popl %%es;"
      "popl %%ds;"
      "iret;"
      "nop;"
    : : "m"(ret->esp0),
        "m"(ret->ss),
        "m"(ret->esp3),
        "m"(ret->eflags),
        "m"(ret->cs),
        "m"(ret->eip),
        "m"(ret->ds),
        "m"(ret->es),
        "m"(ret->eax),
        "m"(ret->ecx),
        "m"(ret->edx),
        "m"(ret->ebx),
        "m"(ret->esp0),
        "m"(ret->ebp),
        "m"(ret->esi),
        "m"(ret->edi)
    );

  } else {
    // return to kernel
    asm volatile(
      "nop;"
      "movl %0, %%esp;" // move stack
      "push %1;" // EFL
      "push %2;" // CS
      "push %3;" // EIP
      "push %4;" // DS
      "push %5;" // ES
      "push %6;" // EAX  - popal
      "push %7;" // ECX
      "push %8;" // EDX
      "push %9;" // EBX
      "push %10;" // ESP
      "push %11;" // EBP
      "push %12;" // ESI
      "push %13;" // EDI -
      "popal;"
      "popl %%es;"
      "popl %%ds;"
      "iret;"
      "nop;"
    : : "m"(ret->esp0),
        "m"(ret->eflags),
        "m"(ret->cs),
        "m"(ret->eip),
        "m"(ret->ds),
        "m"(ret->es),
        "m"(ret->eax),
        "m"(ret->ecx),
        "m"(ret->edx),
        "m"(ret->ebx),
        "m"(ret->esp0),
        "m"(ret->ebp),
        "m"(ret->esi),
        "m"(ret->edi)
    );
  }
}

int _asye_init(_Context*(*handler)(_Event, _Context*)) {
  static GateDesc idt[NR_IRQ];
  ioapic_enable(IRQ_KBD, 0);

  // init IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), irqall, DPL_KERN);
  }

  // --------------------- exceptions --------------------------
  idt[0]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec0,   DPL_KERN);
  idt[1]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec1,   DPL_KERN);
  idt[2]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec2,   DPL_KERN);
  idt[3]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec3,   DPL_KERN);
  idt[4]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec4,   DPL_KERN);
  idt[5]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec5,   DPL_KERN);
  idt[6]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec6,   DPL_KERN);
  idt[7]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec7,   DPL_KERN);
  idt[8]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec8,   DPL_KERN);
  idt[9]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec9,   DPL_KERN);
  idt[10]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec10,  DPL_KERN);
  idt[11]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec11,  DPL_KERN);
  idt[12]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec12,  DPL_KERN);
  idt[13]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec13,  DPL_KERN);
  idt[14]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec14,  DPL_KERN);
  // --------------------- interrupts --------------------------
  idt[32]   = GATE(STS_IG32, KSEL(SEG_KCODE), irq0,   DPL_KERN);
  idt[33]   = GATE(STS_IG32, KSEL(SEG_KCODE), irq1,   DPL_KERN);
  idt[46]   = GATE(STS_IG32, KSEL(SEG_KCODE), irq14,  DPL_KERN);
  // -------------------- system call --------------------------
  idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys, DPL_USER);
  set_idt(idt, sizeof(idt));
  H = handler;

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  _Context *ctx = (_Context *)stack.start;
  ctx->eax = ctx->ebx = ctx->ecx = ctx->edx = 0;
  ctx->esi = ctx->edi = ctx->ebp = ctx->esp3 = 0;

  ctx->ss0 = 0; // only used for ring3 procs
  ctx->esp0 = (uint32_t)stack.end;
  ctx->cs = KSEL(SEG_KCODE);
  ctx->ds = ctx->es = ctx->ss = KSEL(SEG_KDATA);
  ctx->eip = (uint32_t)entry;
  ctx->eflags = FL_IF;

  uint32_t **esp = (uint32_t **)&ctx->esp0;
  *(*esp -= 1) = (uint32_t)arg; // argument
  *(*esp -= 1) = 0; // return address
  return ctx;
}

void _yield() {
  asm volatile("int $0x80" : : "a"(-1));
}

int _intr_read() {
  return (get_efl() & FL_IF) != 0;
}

void _intr_write(int enable) {
  if (enable) {
    sti();
  } else {
    cli();
  }
}
