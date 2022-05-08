#include "common.h"
#include "syscall.h"


static uintptr_t sys_none() {
	return 1;
}

void sys_exit(int status) {
	_halt(status);
	
}

static uintptr_t sys_write(int fd, void *buf, size_t count) {
    if (fd == 1 || fd == 2) { // stdout or stderr
      size_t i;
      char c;
      for (i = 0; i < count; i++) {
        memcpy(&c,buf+i,1);
        _putc(c);
      }  
      return count;
    }  
    return -1;
  }

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none: a[0] = sys_none(); break;
    case SYS_exit: sys_exit(a[1]); break;
    case SYS_write: a[0] = sys_write(a[1], (void*)a[2], a[3]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  SYSCALL_ARG1(r) = a[0];
  return NULL;
}
