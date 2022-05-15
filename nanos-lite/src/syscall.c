#include "common.h"
#include "syscall.h"


extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern ssize_t fs_write(int fd, const void *buf, size_t len);
extern off_t fs_lseek(int fd, off_t offset, int whence);
extern int fs_close(int fd);



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
      Log("buffer:%s",(char*)buf);
      for (i = 0; i < count; i++) {
        memcpy(&c,buf+i,1);
        _putc(c);
      }  
      return count;
    }  
    if(fd>=3){
      return fs_write(fd,buf,count);
    }
    Log("fd<=0");
    return -1;
}

int sys_brk(int addr){
  
  return 0;
}  




static uintptr_t sys_open(const char *path) {
	return fs_open(path, 0, 0);
}

static uintptr_t sys_read(int fd, void *buf, size_t count) {
	return fs_read(fd, buf, count);
}

static uintptr_t sys_close(int fd) {
	return fs_close(fd);
}

static uintptr_t sys_lseek(int fd, off_t offset, int whence) {
	return fs_lseek(fd, offset, whence);
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
    case SYS_brk:a[0]=sys_brk(a[1]);break;
    case SYS_open:a[0]=sys_open((const char*)a[1]);break;
    case SYS_read:a[0]=sys_read(a[1],(void*)a[2],a[3]);break;
    case SYS_close:a[0]=sys_close(a[1]);break;
    case SYS_lseek:a[0]=sys_lseek(a[1],a[2],a[3]);break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  SYSCALL_ARG1(r) = a[0];
  return NULL;
}
